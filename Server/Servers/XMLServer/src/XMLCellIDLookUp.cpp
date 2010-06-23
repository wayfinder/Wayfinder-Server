/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"

#include "CellIDLookUp.h"

#ifdef USE_XML
#include "XMLTool.h"
#include "XMLServerElements.h"
#include "XMLNodeIterator.h"
#include "XMLSearchUtility.h"
#include "ParserExternalAuth.h"
#include "ParserExternalAuthHttpSettings.h"
#include "TimedOutSocketLogger.h"
#include "STLStringUtility.h"

#include <util/XMLString.hpp>

using namespace CellIDLookUp;

void 
parse3GPPRequest( const DOMNode* node, CellIDRequest& cellIDReq ) {
   // Extract the cell id request values
   using namespace XMLTool;
   getAttribValue( cellIDReq.m_mcc, "c_mcc", node );
   getAttribValue( cellIDReq.m_mnc, "c_mnc", node );
   getAttribValue( cellIDReq.m_lac, "lac", node );
   getAttribValue( cellIDReq.m_cellID, "cell_id", node );
   getAttribValue( cellIDReq.m_networkType, "network_type", node );
   getAttribValue( cellIDReq.m_signalStrength, "signal_strength", node );

   if ( cellIDReq.m_networkType.empty() ) {
      mc2log << warn << "[XMLCellId]:parse3GPPRequest "
             << "no network type supplied" << endl;
   }

   // Check if cell_id and lac has "0x" in the start. If not add it.
   if ( ! ( cellIDReq.m_cellID.substr( 0, 2 ) == "0x" ) &&
        ! cellIDReq.m_cellID.empty() ) {
      cellIDReq.m_cellID = "0x" + cellIDReq.m_cellID;
   }
   if ( ! ( cellIDReq.m_lac.substr( 0, 2 ) == "0x" ) &&
        ! cellIDReq.m_lac.empty() ) {
      cellIDReq.m_lac = "0x" + cellIDReq.m_lac;
   }
}

bool
XMLParserThread::xmlParseCellIDRequest( DOMNode* cur,
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent ) 
try {
   int indentLevel = 1;
   MC2String errorCode;      // errorcode if one should be sent in reply.
   MC2String errorMessage;   // errorMessage if one should be sent in reply

   // Create the reply element
   DOMElement* cellID_reply =
      XMLUtility::createStandardReply( *reply, *cur, "cell_id_reply" );
   out->appendChild( cellID_reply );

   // <!-- Cell ID request -->
   // <!ELEMENT cell_id_request (3GPP | CDMA | iDEN)>
   // <!ATTLIST cell_id_request
   //   position_system   %position_system_t;  "MC2"
   //   transaction_id    ID                   #REQUIRED >
   // <!ELEMENT TGPP EMPTY>
   // <!ATTLIST TGPP
   //   c_mcc             CDATA #IMPLIED
   //   c_mnc             CDATA #IMPLIED
   //   lac               CDATA #IMPLIED
   //   cell_id           CDATA #IMPLIED
   //   network_type      CDATA #IMPLIED
   //   signal_strength   CDATA #IMPLIED >
   // <!ELEMENT CDMA EMPTY>
   // <!ATTLIST CDMA
   //   sid               CDATA #IMPLIED
   //   nid               CDATA #IMPLIED
   //   bid               CDATA #IMPLIED
   //   network_type      CDATA #IMPLIED
   //   signal_strength   CDATA #IMPLIED >
   // <!ELEMENT iDEN EMPTY>
   // <!ATTLIST iDEN
   //   c_mcc             CDATA #IMPLIED
   //   dnc               CDATA #IMPLIED
   //   sa_id             CDATA #IMPLIED
   //   lla_id            CDATA #IMPLIED
   //   cell_id           CDATA #IMPLIED
   //   signal_strength   CDATA #IMPLIED >
   //
   // <!-- Cell ID reply -->
   //<!ELEMENT cell_id_reply ( ( position_item ) |
   //            ( status_code, status_message, status_code_extended? ) ) >
   // <!ATTLIST cell_id_reply
   //   transaction_id    ID       #REQUIRED
   //   inner_radius      %number; #IMPLIED
   //   outer_radius      %number; #IMPLIED
   //   altitude          %number; #IMPLIED
   //   start_angle       %number; #IMPLIED
   //   end_angle         %number; #IMPLIED >

   using namespace XMLTool;


   // Get the wanted coordinate type from the client
   XMLCommonEntities::coordinateType positionSystem = XMLCommonEntities::MC2;
   getAttrib( positionSystem, "position_system", cur );

   CellIDRequest cellIDReq;
   CellIDReply cellIDReply;

   // decode request data
   ElementConstIterator it( cur->getFirstChild() );
   ElementConstIterator end( NULL );
   for ( ; it != end; ++it ) {
      if ( XMLString::equals( it->getNodeName(), "TGPP" ) ) {
         parse3GPPRequest( *it, cellIDReq );
      } else if ( XMLString::equals( it->getNodeName(), "CDMA" ) ) {
         // Not supported yet
      } else if ( XMLString::equals( it->getNodeName(), "iDEN" ) ) {
         // Not supported yet
      } else {
         mc2log << warn << "[XMLCellID] No valid node." << endl;
      }
   }
   
   mc2log << "[XMLCellID] Network " << cellIDReq.m_networkType 
          << " signalStrength " << cellIDReq.m_signalStrength
          << " mcc " << cellIDReq.m_mcc << " mnc " << cellIDReq.m_mnc
          << " lac " << cellIDReq.m_lac
          << " cellID " << cellIDReq.m_cellID << endl;

   if ( ! cellIDLookUp( cellIDReq, cellIDReply, &*m_group ) ) {
      mc2dbg << "[XMLCellId] Cell id look up failed"  << endl;
      XMLServerUtility::appendStatusNodes( cellID_reply, reply, 1,
                                           false,
                                           "-1", 
                                           "No cell id position found" );
      return true;
   }
      
   // Build the reply xml
   addAttrib( cellID_reply, "inner_radius", cellIDReply.m_innerRadius );
   addAttrib( cellID_reply, "outer_radius", cellIDReply.m_outerRadius );
   addAttrib( cellID_reply, "altitude", cellIDReply.m_altitude );
   addAttrib( cellID_reply, "start_angle", cellIDReply.m_startAngle );
   addAttrib( cellID_reply, "end_angle", cellIDReply.m_endAngle );
   XMLSearchUtility::appendPositionItem( cellIDReply.m_coord.lat,
                                         cellIDReply.m_coord.lon,
                                         MAX_UINT16, // angle, will not be added when > 360
                                         positionSystem,
                                         cellID_reply,
                                         reply,
                                         indentLevel,
                                         indent );

   if ( indent ) {
      XMLUtility::indentPiece( *cellID_reply, indentLevel );
   }

   return true;
   
} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[XMLCellId] " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return true;
}

#endif // USE_XML

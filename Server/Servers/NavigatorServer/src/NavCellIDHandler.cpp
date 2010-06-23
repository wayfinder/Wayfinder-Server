/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavCellIDHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "NavRequestData.h"
#include "CellIDLookUp.h"
#include "ParserExternalAuth.h"
#include "ParserExternalAuthHttpSettings.h"
#include "ServerRegionIDs.h"
#include "TimedOutSocketLogger.h"
#include "STLStringUtility.h"
#include "UserData.h"


NavCellIDHandler::NavCellIDHandler( InterfaceParserThread* thread,
                                    NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
   m_expectations.push_back( ParamExpectation( 6200, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6201, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6202, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6203, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6204, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6205, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6206, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6207, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6208, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6209, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6210, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6211, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6212, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6213, NParam::String ) );
}


NavCellIDHandler::~NavCellIDHandler() {
}


struct futureCellID {
   MC2String m_sid;
   MC2String m_nid;
   MC2String m_bid;
   MC2String m_mcc;
   MC2String m_dnc;
   MC2String m_saID;
   MC2String m_llaID;
   MC2String m_cellID;
   
};

bool
NavCellIDHandler::handleCellIDLookup( NavRequestData& rd )
{
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // Start parameter printing
   mc2log << info << "[NavCellIDHandler]::handleCellIDLookup:";

   // Input data
   CellIDLookUp::CellIDRequest cellIDRequest;

   // Current SignalStrength
   if ( getParam( 6201, cellIDRequest.m_signalStrength, rd.params ) ) {
      mc2log << " signalStrength " << cellIDRequest.m_signalStrength;
   }

   stringstream cellIDData;
   // Network type
   if ( getParam( 6200, cellIDRequest.m_networkType, rd.params ) ) {
      cellIDData << " Network " << cellIDRequest.m_networkType;
   }

   // GSM (TGPP)
   // Current MCC
   if ( getParam( 6202, cellIDRequest.m_mcc, rd.params ) ) {
      cellIDData << " mcc " << cellIDRequest.m_mcc;
   }
   // Current MNC
   if ( getParam( 6203, cellIDRequest.m_mnc, rd.params ) ) {
      cellIDData << " mnc " << cellIDRequest.m_mnc;
   }
   // Current LAC
   if ( getParam( 6204, cellIDRequest.m_lac, rd.params ) ) {
      cellIDData << " lac " << cellIDRequest.m_lac;
   }
   // Current cellID
   if ( getParam( 6205, cellIDRequest.m_cellID, rd.params ) ) {
      cellIDData << " cellID " << cellIDRequest.m_cellID;
   }

   futureCellID fCellID;
   // CDMA
   // CurrentSID
   if ( getParam( 6206, fCellID.m_sid, rd.params ) ) {
      cellIDData << " sid " << fCellID.m_sid;
   }
   // CurrentNID
   if ( getParam( 6207, fCellID.m_nid, rd.params ) ) {
      cellIDData << " nid " << fCellID.m_nid;
   }
   // CurrentBID
   if ( getParam( 6208, fCellID.m_bid, rd.params ) ) {
      cellIDData << " nid " << fCellID.m_bid;
   }

   // iDEN
   // CurrentMCC
   if ( getParam( 6209, fCellID.m_mcc, rd.params ) ) {
      cellIDData << " mcc " << fCellID.m_mcc;
   }
   // CurrentDNC
   if ( getParam( 6210, fCellID.m_dnc, rd.params ) ) {
      cellIDData << " dnc " << fCellID.m_dnc;
   }
   // CurrentSA_ID
   if ( getParam( 6211, fCellID.m_saID, rd.params ) ) {
      cellIDData << " sa_id " << fCellID.m_saID;
   }
   // CurrentLLA_ID
   if ( getParam( 6212, fCellID.m_llaID, rd.params ) ) {
      cellIDData << " lla_id " << fCellID.m_llaID;
   }
   // CurrentCell_ID
   if ( getParam( 6213, fCellID.m_cellID, rd.params ) ) {
      cellIDData << " cell_id " << fCellID.m_cellID;
   }
   mc2log << cellIDData.str();

   // End parameter printing
   mc2log << endl;

   // Top region id
   uint32 topRegion = m_group->getRegionIDs()->getRegionIdFromMCC( 
      cellIDRequest.m_mcc );
   if( topRegion != MAX_UINT32 ) {
      rd.rparams.addParam( NParam( 6306, topRegion ) );
   } else {
      mc2log << warn << "Top region not found for MCC: " << 
         cellIDRequest.m_mcc << endl;
   }

   CellIDLookUp::CellIDReply cellIDReply;
   if ( !CellIDLookUp::cellIDLookUp( cellIDRequest, 
                                     cellIDReply, 
                                     &*m_group ) ) {
      mc2log << "[NavCellIDHandler]::handleCellIDLookup: Cell id look up "
             << "failed" << endl;
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
      ok = false;
   } else {
      mc2log << "[NavCellIDHandler]::handleCellIDLookup: Coord " 
             << cellIDReply.m_coord
             << " radius " << cellIDReply.m_outerRadius 
             << " top region  " << topRegion << endl;
      // Send cellID coordinate and radius
      rd.rparams.addParam( NParam( 6300, cellIDReply.m_coord ) );
      // Altitude (m)
      rd.rparams.addParam( NParam( 6301, cellIDReply.m_altitude ) );
      // Inner radius
      rd.rparams.addParam( NParam( 6302, cellIDReply.m_innerRadius ) );
      // Outer radius
      rd.rparams.addParam( NParam( 6303, cellIDReply.m_outerRadius ) );
      // Start angle
      rd.rparams.addParam( NParam( 6304, cellIDReply.m_startAngle ) );
      // End angle
      rd.rparams.addParam( NParam( 6305, cellIDReply.m_endAngle ) );
   }

   return ok;
}


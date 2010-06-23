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

#include "SearchParserHandler.h"
#include "XMLTool.h"
#include "XMLCommonElements.h"
#include "XMLTopRegionElement.h"

bool
XMLParserThread::xmlParseSearchPositionDescRequest( DOMNode* cur,
                                                    DOMNode* out,
                                                    DOMDocument* reply,
                                                    bool indent )
try {
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur,
                                       "search_position_desc_reply" );
   out->appendChild( root );

   // delay append child until we have checked crc
   //   out->appendChild( root );

   // The request:
   //
   // <!ELEMENT search_position_desc_request position_item >
   // <!ATTLIST search_position_desc_request
   //       transaction_id ID #REQUIRED
   //       language CDATA #REQUIRED >
   //
   // <!ELEMENT position_item ( lat, lon, angle? )>
   // <!ATTLIST position_item
   //       position_system
   //       %position_system_t; #REQUIRED >
   //
   //
   // The reply:
   //
   // <!ELEMENT search_position_desc_reply ( top_region?, search_hit_type* ) >
   // <!ATTLIST search_position_desc_reply
   //       transaction_id ID #REQUIRED 
   //       length %number; #REQUIRED >
   //
   //
   //

   // fetch language and position, required elements.
   SearchParserHandler::RegionRequest request;
   XMLTool::getAttribValue( request.m_language, "language", cur );

   const DOMNode* posElement = XMLTool::findNode( cur, "position_item" );
   
   if ( ! posElement ) {
      throw XMLTool::Exception( "Missing node.", "position_item" );
   }

   MC2String errorCode, errorMsg;
   uint16 angle;
   XMLCommonEntities::coordinateType coordType;
   if ( ! XMLCommonElements::getPositionItemData( posElement,
                                                  request.m_position.lat,
                                                  request.m_position.lon,
                                                  angle, 
                                                  errorCode, 
                                                  errorMsg,
                                                  &coordType ) ) {
      throw XMLTool::Exception( errorCode + ":" + errorMsg, 
                                "position_item" );
   }
   mc2dbg<< "###### XMLSearchPositionDescRequest: " << request.m_position << endl;

   SearchParserHandler::RegionInfo info;
   if ( getSearchHandler().getRegionInfo( request, info ) ) {
      root->
         appendChild( XMLTopRegionElement::
                      makeTopRegionElement( info.m_topMatch, coordType,
                                            request.m_language,
                                            root,
                                            0, false, // indentlevel 
                                            NULL ) ); // no databuffer
   } else {
      XMLTool::addAttrib( root, "length", 0 );
      // the position item might be outside any top region.
      return true; 
   }

   CompactSearchHitTypeVector::const_iterator it = info.m_headings.begin();
   CompactSearchHitTypeVector::const_iterator itEnd = info.m_headings.end();
   for ( ; it != itEnd; ++it ) {
      DOMElement* hitType = XMLTool::addNode( root, "search_hit_type" );

      XMLTool::addAttrib( hitType, "round", (*it).m_round );
      XMLTool::addAttrib( hitType, "heading", (*it).m_heading );
      XMLTool::addNode( hitType, "name", (*it).m_name );

      if ( ! (*it).m_imageName.empty() ) {
         XMLTool::addNode( hitType, "image_name", (*it).m_imageName );
      }
   }

   XMLTool::addAttrib( root, "length", info.m_headings.size() );

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

 } catch ( const XMLTool::Exception& e ) {
   mc2log << warn << "search_desc_request: " << e.what() << endl;
   return false;
}

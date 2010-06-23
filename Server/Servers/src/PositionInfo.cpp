/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PositionInfo.h"

#include "Requester.h"
#include "FixedSizeString.h"
#include "StringTable.h"
#include "LangTypes.h"
#include "MC2Coordinate.h"
#include "ItemTypes.h"
#include "Requester.h"

#include "CoordinateRequest.h"
#include "TopRegionRequest.h"
#include "NameUtility.h"

namespace PositionInfo {

uint32
getTopRegionFromPosition( Requester& requester,
                          const TopRegionRequest* topReq,
                          const MC2Coordinate& position ) {

   CoordinateRequest req( requester.getNextRequestID(), topReq );
   const TopRegionMatch* topMatch = NULL;
   
   if ( ! getInfoFromPosition( requester, req, topReq, position, topMatch ) ) {
        return MAX_UINT32;
   }

   return topMatch->getID();
}


bool 
getInfoFromPosition( Requester& requester,
                     CoordinateRequest& req,
                     const TopRegionRequest* topReq,
                     const MC2Coordinate& position,
                     const TopRegionMatch*& topMatch ) {
   byte allowedTypes[] = { 
      ItemTypes::municipalItem
   };

   req.addCoordinate( position.lat, position.lon, // position
                      0, // outDataTypes, not sure about this value.
                      // max number of allowed types
                      sizeof( allowedTypes ) / sizeof( allowedTypes[ 0 ] ), 
                      allowedTypes ); // item types vector

   requester.putRequest( &req );

   // request ok? and have valid top region?
   if ( req.getStatus() == StringTable::NOTOK &&
        topReq->getStatus() == StringTable::NOTOK ) {
      return false;
   }

   // only interested in the first hit.
   CoordinateReplyPacket* pack = req.getCoordinateReply();
   if ( pack == NULL ) { 
      return false;
   }

   // find out top region id
   topMatch = topReq->getCountryForMapID( pack->getMapID() );
   if ( topMatch == NULL ) {
      return false;
   }

   return true;
}

MC2String
getMunicipalFromPosition( Requester& requester,
                          const MC2Coordinate& position,
                          const LangType& language,
                          const TopRegionRequest* topReq,
                          uint32& topRegionID ) {
   uint32 dist = 0; // not used
      
   // find "where" field from municipal or something.
   CoordinateRequest req( requester.getNextRequestID(),
                          topReq,
                          NameUtility::
                          getBestLanguage( language, dist ) );
   const TopRegionMatch* topMatch = NULL;
   if ( ! PositionInfo::
        getInfoFromPosition( requester, req, topReq, position, topMatch ) ) {
      return "";
   }

   topRegionID = topMatch->getID();

   CoordinateReplyPacket* pack = req.getCoordinateReply();

   mc2log << "SearchParserHandler::getMunicipalFromPosition: top region id = "
          << topRegionID << endl;

   // get municipal name or normal name
   FixedSizeString 
      notused( 256 ), 
      municipal( notused ),
      name( notused );

   pack->getStrings( &notused[0],   // country
                     &municipal[0], // municipal
                     &notused[0],   // city
                     &notused[0],   // district
                     &name[0],   // name
                     notused.getMaxSize() ); // max string size

   // if municipal is empty use the item name
   if ( municipal.empty() ) {
      return name.c_str();
   }

   return municipal.c_str();
}

}

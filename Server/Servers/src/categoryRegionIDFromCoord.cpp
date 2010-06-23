/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "categoryRegionIDFromCoord.h"
#include "CoordinateRequest.h"
#include "MapRequester.h"
#include "ItemTypes.h"

CategoryRegionID categoryRegionIDFromCoord( const MC2Coordinate& coord,
                                            MapRequester* requester ) {
   auto_ptr<CoordinateRequest> coordReq(
      new CoordinateRequest( RequestData( requester->getNextRequestID(),
                                         NULL ),
                             requester->getTopRegionRequest() ) );

   byte SSIType = ItemTypes::streetSegmentItem;

   coordReq->addCoordinate( coord.lat, coord.lon, 
                            0, // type of reply data 0 = all
                            1, // number of items to return
                            &SSIType );

   requester->putRequest( coordReq.get() );

   CoordinateReplyPacket* reply = coordReq->getCoordinateReply();

   StringTable::countryCode invalid( static_cast<StringTable::countryCode>(-1) );
   if ( reply == NULL || reply->getCountryCode() == invalid ) {
      return CategoryRegionID::NO_REGION;
   }
   else {
      return CategoryRegionID( reply->getCountryCode() );
   }
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POSITION_INFO_H
#define POSITION_INFO_H

#include "config.h"
#include "MC2String.h"

class CoordinateRequest;
class TopRegionMatch;
class TopRegionRequest;
class MC2Coordinate;
class Requester;
class LangType;

namespace PositionInfo {

/**
 * Gets municipal name and top region from a position used to
 * determine "where" field for external searches that does not
 * handle coordinate location searches.
 *
 * @param position the position we are interested in
 * @param language of the municipal
 * @param topReq The current top region request.
 * @param topRegionID will get correct top region if search was successfull
 * @return municipal name 
 */
MC2String getMunicipalFromPosition( Requester& requester,
                                    const MC2Coordinate& position,
                                    const LangType& language,
                                    const TopRegionRequest* topReq,
                                    uint32& topRegionID );

/**
 * Fetch information about the top region at the specific position.
 * @param req Coordinate request.
 * @param topReq Top region request.
 * @param position A coordinate in the region to fetch info for.
 * @param topMatch Will contain information about the top region at the
 *                 coordinate.
 * @return true if info about the position fetch was succesfully.
 */
bool getInfoFromPosition( Requester& requester,
                          CoordinateRequest& req,
                          const TopRegionRequest* topReq,
                          const MC2Coordinate& position,
                          const TopRegionMatch*& topMatch );

/**
 * Gets the top region id from a posistion.
 * @param topReq Top region request.
 * @param position A coordinate in the region to fetch top region for.
 * @return The top region id, MAX_UINT32 if not succesfull.
 */
uint32 getTopRegionFromPosition( Requester& requester,
                                 const TopRegionRequest* topReq,
                                 const MC2Coordinate& position );
}

#endif // POSITION_INFO_H


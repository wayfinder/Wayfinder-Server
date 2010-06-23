/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "RouteRequestParams.h"
#include "RouteRequest.h"

#define INITLIST(expandType, language, nbrWantedRoutes,         \
                 noConcatenate, passedRoads)                    \
/* bool m_abbreviate; */                                        \
/* bool m_landmarks;  */                                        \
/* bool m_isStartTime;*/                                        \
/* uint32 m_turnCost; */                                        \
/* uint32 m_time;     */                                        \
   m_expandType(expandType),                                    \
   m_language(language),                                        \
   m_nbrWantedRoutes(nbrWantedRoutes),                          \
   m_allowedMaps(NULL),                                         \
   m_noConcatenate(noConcatenate),                              \
   m_passedRoads(passedRoads),                                  \
   m_avoidTollRoads(false),                                     \
   m_avoidHighways(false),                                      \
   m_removeAheadIfDiff( true ),                                 \
   m_nameChangeAsWP(false),                                     \
   m_compareDisturbanceRoute(false)


RouteRequestParams::RouteRequestParams() :
   INITLIST(0,StringTable::ENGLISH, 1, false, 0)
{
}

RouteRequestParams::RouteRequestParams( uint32 expandType,
                                        StringTable::languageCode language,
                                        bool noConcatenate,
                                        uint32 passedRoads,
                                        uint32 nbrWantedRoutes ) : 
   INITLIST(expandType, language, nbrWantedRoutes,
            noConcatenate, passedRoads)
{
}

RouteRequest* 
RouteRequestParams::createRouteRequest(const RequestUserData& user,
                                       uint16 reqID,
                                       const TopRegionRequest* topReq,
                                       const DisturbanceList* disturbances,
                                       Request* parentRequest) const
{
   return new RouteRequest( user, reqID, *this, topReq, disturbances, 
                            parentRequest );
}

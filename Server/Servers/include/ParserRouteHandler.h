/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSER_ROUTE_HANDLER_H
#define PARSER_ROUTE_HANDLER_H

#include "config.h"

#include "ParserHandler.h"
#include "StringTable.h"

class DisturbanceList;
class DriverPref;
class RequestUserData;
class RouteID;
class RouteRequest;
class RouteRequestParams;
class TopRegionRequest;
class UnexpandedRoute;

/**
 *   Route handling functions for a ParserThread.
 */
class ParserRouteHandler : public ParserHandler {
public:
   /**
    * Standard constructor matching that of ParserHandler.
    * @param thread The ParserThread using this ParserRouteHandler.
    * @param group The ParserThreadGroup containing the owning thread.
    */
   ParserRouteHandler( ParserThread* thread,
                       ParserThreadGroup* group );
   
   /**
    * Non-virtual destructor. The parent object does not have a
    * virtual destructor.
    */
   ~ParserRouteHandler();

   /**
    * Recalculate a previously produced route. The origin and
    * destination are calculated from the old route and then a new
    * route with the same endpoints are requested.
    */
   RouteRequest* redoRoute( const RouteID& oldRouteID,
                            const RouteRequestParams& rrParams,
                            const RequestUserData& user,
                            const TopRegionRequest* topReq,
                            const DisturbanceList* disturbances = NULL );

   UnexpandedRoute* redoUnexpandedRoute( const RouteID& oldRouteID,
                                         const RouteRequestParams& rrParams,
                                         const RequestUserData& user,
                                         const TopRegionRequest* topReq,
                                         const DisturbanceList* disturbances = NULL);

   bool routeChanged( const RouteID& oldRouteID,
                      const RouteRequestParams& rrParams,
                      const RequestUserData& user,
                      const TopRegionRequest* topReq );

   UnexpandedRoute* getStoredUnexpandedRoute( const RouteID& routeID,
                                              const DriverPref& prefs );

};


#endif

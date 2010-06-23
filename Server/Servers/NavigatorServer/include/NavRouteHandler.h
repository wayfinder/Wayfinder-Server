/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVROUTEHANDLER_H
#define NAVROUTEHANDLER_H

#include "config.h"
#include "NavHandler.h"
#include "ItemTypes.h"


class NavRequestPacket;
class NavReplyPacket;
class NavRequestData;
class UserItem;
class NavUserHelp;
class isabBoxSession;


/**
 * Class handling NavServerProt, v10+, routes.
 *
 */
class NavRouteHandler : public NavHandler {
   public:
      /**
       * Constructor.
       */
      NavRouteHandler( InterfaceParserThread* thread,
                       NavParserThreadGroup* group,
                       NavUserHelp* userHelp );


      /**
       * Handles a route packet.
       *
       * @param rd The holder of the request data.
       * @return True if route ok, false if not and then reply's status
       *         is set.
       */
      bool handleRoute( NavRequestData& rd );


      /**
       * Some route specific status codes.
       */
      enum RouteReplyStatus {
         ROUTE_REPLY_NO_ROUTE_FOUND        = 0x81,
         ROUTE_REPLY_TOO_FAR_FOR_VEHICLE   = 0x82,
         ROUTE_REPLY_PROBLEM_WITH_ORIGIN   = 0x83,
         ROUTE_REPLY_PROBLEM_WITH_DEST     = 0x84,
         ROUTE_REPLY_NO_AUTO_DEST          = 0x85,
         ROUTE_REPLY_NO_ORIGIN             = 0x86,
         ROUTE_REPLY_NO_DESTINATION        = 0x87,
         /** This is not an error code but a specific status. */
         ROUTE_REPLY_NO_ROUTE_CHANGE       = 0x88,
      };


   private:
      /**
       * Converts to a mc2 vehicle.
       */
      ItemTypes::vehicle_t mc2Vehicle( byte vehicle ) const;


      /// A NavUserHelp.
      NavUserHelp* m_userHelp;
};


#endif // NAVROUTEHANDLER_H


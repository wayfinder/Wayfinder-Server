/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVSERVERINFOHANDLER_H
#define NAVSERVERINFOHANDLER_H

#include "config.h"
#include "NavHandler.h"


class NavRequestPacket;
class NavReplyPacket;
class UserItem;
class NavUserHelp;
class IsabBoxInterfaceRequest;
class ClientSetting;
class NavRequestData;

/**
 * Class handling NavServerProt, v10+, ServerInfo requests.
 *
 */
class NavServerInfoHandler : public NavHandler {
   public:
      /**
       * Constructor.
       */
      NavServerInfoHandler( InterfaceParserThread* thread,
                            NavParserThreadGroup* group,
                            NavUserHelp* userHelp );


      /**
       * Destructor.
       */
      ~NavServerInfoHandler();


      /**
       * Handles a ServerInfo packet.
       *
       * @param rd The holder of the request data.
       * @return True if ServerInfo ok, false if not and then reply's
       *         status is set.
       */
      bool handleServerInfo( NavRequestData& rd );


      /**
       * Handles a TopRegion packet.
       *
       * @param rd The holder of the request data.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleTopRegion( NavRequestData& rd );


      /**
       * Handles a LatestNews packet.
       *
       * @param userItem The user.
       * @param req The LatestNews request.
       * @param reply Set status of if problem.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleLatestNews( UserItem* userItem, 
                             NavRequestPacket* req,
                             NavReplyPacket* reply );


      /**
       * Handles a Categories packet.
       *
       * @param userItem The user.
       * @param req The Categories request.
       * @param reply Set status of if problem.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleCategories( UserItem* userItem, 
                             NavRequestPacket* req,
                             NavReplyPacket* reply );


      /**
       * Handles a CallcenterList packet.
       *
       * @param userItem The user.
       * @param req The CallcenterList request.
       * @param reply Set status of if problem.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleCallcenterList( UserItem* userItem, 
                                 NavRequestPacket* req,
                                 NavReplyPacket* reply );


      /**
       * Handles a ServerList packet.
       *
       * @param userItem The user.
       * @param req The ServerList request.
       * @param reply Set status of if problem.
       * @param ireq The interface request that the packet came with.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleServerList( UserItem* userItem, 
                             NavRequestPacket* req,
                             NavReplyPacket* reply, 
                             IsabBoxInterfaceRequest* ireq );


      /**
       * Handles a NewPassword packet.
       *
       * @param userItem The user.
       * @param req The NewPassword request.
       * @param reply Set status of if problem.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleNewPassword( UserItem* userItem, 
                              NavRequestPacket* req,
                              NavReplyPacket* reply );


      /**
       * Handles a ServerAuthBob packet.
       *
       * @param userItem The user.
       * @param req The ServerAuthBob request.
       * @param reply Set status of if problem.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleServerAuthBob( UserItem* userItem, 
                                NavRequestPacket* req,
                                NavReplyPacket* reply );


      /**
       * Handles a SearchDesc packet.
       *
       * @param rd The holder of the request data.
       * @param requestVersion The version of the request.
       * @return True if ok, false if not and then reply's status is set.
       */
      bool handleSearchDesc( NavRequestData& rd, byte requestVersion );


   private:
      /**
       * A NavUserHelp.
       */
      NavUserHelp* m_userHelp;
};


#endif // NAVSERVERINFOHANDLER_H


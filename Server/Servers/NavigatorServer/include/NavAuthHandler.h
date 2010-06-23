/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVAUTHHANDLER_H
#define NAVAUTHHANDLER_H

#include "config.h"
#include "NavHandler.h"

class NavRequestPacket;
class NavReplyPacket;
class isabBoxSession;
class NavUserHelp;
class UserItem;
class NavExternalAuth;
class IsabBoxInterfaceRequest;
class ClientSetting;
class NavRequestData;


/**
 * Class handling NavServerProt, v10+, auths.
 *
 */
class NavAuthHandler : public NavHandler {
   public:
      /**
       * Constructor.
       */
      NavAuthHandler( InterfaceParserThread* thread,
                      NavParserThreadGroup* group,
                      NavUserHelp* userHelp );


      /**
       * Destructor.
       */
      virtual ~NavAuthHandler();


      /**
       * Handles auth of a packet.
       *
       * @param rd The holder of the request data, some fields filled in
       *           in this method.
       * @return True if auth is ok, false if not and then reply's status
       *         is set.
       */
      bool handleAuth( NavRequestData& rd );


      /**
       * Handles a WHOAMI_REQ.
       *
       * @param rd The holder of the request data, some fields filled in
       *           in this method.
       * @return True if userItem set ok, false if not and reply status
       *         set.
       */ 
      bool handleWhoAmIReq( NavRequestData& rd );


      /**
       * Handles a UPGRADE_REQ.
       *
       * @param rd The holder of the request data.
       * @return True if upgraded ok, false if not and then reply's status
       *         is set.
       */
      bool handleUpgrade( NavRequestData& rd );


      /**
       * Handles a CHANGED_LICENSE_REQ.
       *
       * @param rd The holder of the request data.
       * @return True if changed ok, false if not and reply status
       *         set.
       */ 
      bool handleChangedLicence( NavRequestData& rd );


   private:
      /**
       * Checks is a user is expired.
       */
      bool checkUserExpired( UserItem* userItem, NavRequestPacket* req, 
                             NavReplyPacket* reply );


      /**
       * Checks a user's transactions.
       */
      bool checkTransactions( UserItem* userItem, NavRequestPacket* req, 
                              NavReplyPacket* reply );


      /**
       * Checks a user's WFST.
       */
      bool checkUserWFST( UserItem* userItem, NavRequestPacket* req, 
                          NavReplyPacket* reply,
                          const ClientSetting* clientSetting );


      /**
       * Checks a trail user's access time.
       */
      bool checkTrialTime( UserItem* userItem, NavRequestPacket* req, 
                           NavReplyPacket* reply,
                           isabBoxSession* session,
                           const ClientSetting* clientSetting );


      /**
       * Checks a user's access Iron or Trial.
       */
      bool checkIronTrialTime( NavRequestData& rd );

      /**
       * Check if a user may use the current client.
       * @param rd 
       * @return True if the user may use the current client version,
       *         false otherwise
       */
      bool checkVersionLock( NavRequestData& rd );

      /**
       * Get user.
       */
      void getUser( UserItem*& userItem, MC2String& idStr, bool& errorSet,
                    NavRequestPacket* req, NavReplyPacket* reply,
                    const char* printName,
                    bool useCache = true,
                    bool wipeFromCache = false );

      /**
       * Check if userID is the owner's userID.
       *
       * @param rd The request data to check request parameters vs. 
       *           UserItem in session.
       * @param clientID Set to the userID of the client.
       * @return True if client has same userID as the server.
       */
      bool checkUserIDMatches( const NavRequestData& rd, 
                               MC2String& clientID ) const;


      /// A NavUserHelp.
      NavUserHelp* m_userHelp;


      /// The NavExternalAuth.
      NavExternalAuth* m_externalAuth;
};


#endif // NAVAUTHHANDLER_H


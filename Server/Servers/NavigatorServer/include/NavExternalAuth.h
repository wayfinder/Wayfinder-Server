/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVEXTERNALAUTH_H
#define NAVEXTERNALAUTH_H

#include "config.h"
#include "NavHandler.h"
#include "LangTypes.h"
#include "NavUserHelp.h"

class NavRequestPacket;
class NavReplyPacket;
class isabBoxSession;
class NavUserHelp;
class UserItem;
class IsabBoxInterfaceRequest;
class ParserExternalAuthHttpSettings;


/**
 * Class handling external authentication.
 *
 */
class NavExternalAuth : public NavHandler {
   public:
      /**
       * Constructor.
       */
      NavExternalAuth( 
         InterfaceParserThread* thread,
         NavParserThreadGroup* group,
         NavUserHelp* userHelp );


      /**
       * Checks if a request is an external auth candidate.
       *
       * @req The request to check.
       * @param ireq The interface request that the packet came with.
       */
      bool isExternalAuth( NavRequestPacket* req,
                           IsabBoxInterfaceRequest* ireq );


      /**
       * Handles auth of a packet.
       *
       * @param rd The holder of the request data.
       * @param reusedConnection Set to true if reused connection.
       * @param externalAuthName Set to the name of the external auth.
       * @return 0 if auth is ok, -1 if not ok and reply's status
       *         is set, -2 if to try normal auth after this.
       */
      int handleExternalAuth( NavRequestData& rd,
                              bool& reusedConnection,
                              MC2String& externalAuthName );


      /**
       * Removes external-auth reserved user names from parameter block.
       *
       * @param params Where to remove reserved usernames from.
       * @return 0 if ok, -1 if error and -2 if timeout.
       */
      int removeExternalAuthUserNames( NParamBlock& params ) const;


   private:
      /**
       * Checks if request looks like an http authenticated user.
       */
      bool isHttpRequest( NavRequestPacket* req ) const;


      /**
       * Handle a request using http-header to identify user and limit
       * access to a number of IP-series.
       *
       * @param rd The holder of the request data.
       * @param reusedConnection Set to true if reused connection.
       * @param settings The settings to use.
       * @return 0 if auth is ok, -1 if not ok and reply's status
       *         is set, -2 if to try normal auth after this.
       */
      int handleHttpHeaderRequest( 
         NavRequestData& rd,
         bool& reusedConnection, 
         const ParserExternalAuthHttpSettings* settings );


      /// A NavUserHelp.
      NavUserHelp* m_userHelp;
};


#endif // NAVEXTERNALAUTH_H


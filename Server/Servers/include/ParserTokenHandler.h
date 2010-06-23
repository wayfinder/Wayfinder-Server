/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERTOKENHANDLER_H
#define PARSERTOKENHANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"

class UserUser;
class UserToken;
class ClientSetting;
class UserItem;


/**
 * Handles tokens in ParserThread.
 *
 */
class ParserTokenHandler : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserTokenHandler( ParserThread* thread,
                          ParserThreadGroup* group );

      /**
       * Makes a new token string.
       */
      MC2String makeTokenStr();

      /**
       * Set the token for a user, deletes all present and adds a new
       * token.
       *
       * @param user The user to change.
       * @param tokenStr The new token to set.
       * @param increaseAge If to increase age, default true.
       * @param sendChange If to send change to db, default true.
       * @return 0 If ok, -1 if error, -2 if timeout.
       */
      int setToken( UserUser* user, const MC2String& tokenStr, 
                    const ClientSetting* clientSetting,
                    bool increaseAge = true, bool sendChange = true );

      /**
       * Get a token from a user.
       * 
       * @param user The user to find token in.
       * @param tokenStr The token to look for.
       * @return The token or NULL if not such token
       */
      UserToken* getToken( UserUser* user, const MC2String& tokenStr,
                           const ClientSetting* clientSetting );

      /**
       * Get a const token from a user.
       * 
       * @param user The user to find token in.
       * @param tokenStr The token to look for.
       * @return The token or NULL if not such token
       */
      const UserToken* getToken( 
         const UserUser* user, const MC2String& tokenStr,
         const ClientSetting* clientSetting ) const;

      /**
       * Checks if a token is expired.
       *
       * @param user The user to check.
       * @param tokenStr The token to check for.
       * @return True if token is expired, false if not.
       */
      bool expiredToken( UserUser* user, const MC2String& tokenStr,
                         const ClientSetting* clientSetting );

      /**
       * Adds a new token.
       *
       * @param user The user to check.
       * @param tokenStr The token to add.
       * @param sendChange If to send change to db, default true.
       * @return 0 If ok, -1 if error, -2 if timeout.
       */
      int addNewToken( UserUser* user, const MC2String& tokenStr, 
                       const ClientSetting* clientSetting,
                       bool sendChange = true );

      /**
       * Removes all tokens but tokenStr.
       *
       * @param user The user to change.
       * @param tokenStr The token to keep.
       * @param sendChange If to send change to db, default true.
       * @param allGroups If to remove from all groups not just the one
       *                  for the ClientSetting.
       * @param deleteHotTokens If to remove tokens even if they are still hot,
       *                        very newly created.
       * @return 0 If ok, -1 if error, -2 if timeout.
       */
      int removeAllButToken( UserUser* user, const MC2String& tokenStr,
                             const ClientSetting* clientSetting,
                             bool sendChange = true,
                             bool allGroups = false,
                             bool deleteHotTokens = false );

      /**
       * Returns the number of tokens the user has.
       *
       * @param allGroups If all groups not just the one
       *                  for the ClientSetting.
       */
      uint32 getNbrTokens( const UserUser* user,
                           const ClientSetting* clientSetting,
                           bool allGroups = false ) const;

      /**
       * Get the group of tokens for a user with a client.
       */
      const char* tokenGroup( 
         const UserUser* user, 
         const ClientSetting* clientSetting = NULL ) const;

   private:
};


#endif // PARSERTOKENHANDLER_H


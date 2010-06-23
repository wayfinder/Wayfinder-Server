/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERACTIVATIONHANDLER_H
#define PARSERACTIVATIONHANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include <memory>

class UserUser;
class ClientSetting;

/**
 * Class handling activation codes.
 *
 */
class ParserActivationHandler : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserActivationHandler( 
         ParserThread* thread,
         ParserThreadGroup* group );


      /**
       * Destructor.
       */
      virtual ~ParserActivationHandler();


      /**
       * Handles activation for a user.
       *
       * @param user The user to activate for. Will be updated if ok.
       * @param clientSetting The client being used.
       * @param activationCode The activation code.
       * @param phoneNumber The phone number. If empty string then 
       *                    phoneNumber is ignored.
       * @param topRegionID The selected top region id.
       * @param ip The ip of the requester.
       * @param userAgent The userAgent of the requester. For logging.
       * @param userInput The userInput of the requester. For logging.
       * @param cellularModel The cellularModel of the requester. Updates
       *                      or sets model for phoneNumber.
       * @param ownerUIN Set to the owner of the AC if return value is -4.
       * @param serverStr Set to the server to redirect to if return
       *                  value is -8.
       * @param allowSpecialCodes If allows special activation codes.
       * @return 0 if ok, -1 on error, -2 on timeout, -3 Wrong phone nbr,
       *         -4 Used activation code, -5 Bad activation code,
       *         -6 Extension not allowed, -7 Creation not allowed(Right
       *         already exists), -8 redirect needed, -9 if code is valid
       *         but may not be used in this type of client. 
       *         -10 if user must choose region before using ac.
       */
      int activateUser( UserUser* user,
                        const ClientSetting* clientSetting,
                        const char* activationCode,
                        const char* phoneNumber,
                        uint32 topRegionID,
                        uint32 ip,
                        const char* userAgent,
                        const char* userInput,
                        const char* cellularModel,
                        uint32& ownerUIN,
                        MC2String& serverStr,
                        bool allowSpecialCodes = false );


      /**
       * Gets activation data for an activation.
       *
       * @param activationCode The activation code.
       * @param rights Set to the rights as string.
       * @param ownerUIN Set to the UIN of the user that owns the 
       *                 activation code, 0 if not used.
       * @param server Set to the server the other instance this code
       *               belongs to, empty means this instance.
       * @param wholeServerStr If to return the whole server string not
       *                       just the server part.
       * @return 0 if ok, -1 on error, -2 on timeout, -5 Bad activation
       *         code.
       */
      int getActivationData( const char* activationCode,
                             MC2String& rights, uint32& ownerUIN,
                             MC2String& server, 
                             bool wholeServerStr = false );


      /**
       * Checks if an activation code is a special code.
       */
      bool isSpecialActivationCode( const MC2String& ac ) const;
   /**
    * Add users rights to user determined by activationCode and rightStr
    * @param phoneNumber the phones number
    * @param cellularModel the cellular model
    * @param topRegionID The selected top region id.
    * @param rightStr user rights string
    * @param activationCode activaction code
    * @param now time "now", start of processing
    * @param newWriteId For adding rights to DB or not.
    * @return 0 on success, negative on failure
    */
   int addUserRights( UserUser* user,
                      const char* phoneNumber,
                      const char* cellularModel,
                      uint32 topRegionID,
                      const MC2String& rightStr,
                      const MC2String& activationCode,
                      uint32 now,
                      uint32 newWriteId = 0 );
private:
   /**
    * sends activation packet
    * @param user the user
    * @param ac Activation code
    * @param ip
    * @param userAgent the user agent string
    * @param userInput user input string
    * @return 0 on success, negative on failure.
    */
   int sendActivation( UserUser* user,
                       const MC2String& ac,
                       uint32 ip, 
                       const char* userAgent, 
                       const char* userInput );
   /**
    * Gets activation code
    * @param clientSettings the client settings
    * @param activation code the activation code to send
    * @return 0 on success, negative on failure.
    */
   int getActivationCode( UserUser* user,
                          const ClientSetting* clientSetting,
                          const char* activationCode,
                          uint32 ip,
                          const char* userAgent,
                          const char* userInput,
                          uint32& ownerUIN,
                          MC2String& serverStr,
                          bool allowSpecialCodes,
                          uint32 now,
                          MC2String& ac, 
                          MC2String& rightStr );


   /**
    * This is a map< MC2String, UserEnums::URType >.
    */
   class RightStrToRightContImpl;


   /**
    * The Map of string to user right.
    */
   auto_ptr<RightStrToRightContImpl> m_rightStrToRightCont;
};


#endif // PARSERACTIVATIONHANDLER_H


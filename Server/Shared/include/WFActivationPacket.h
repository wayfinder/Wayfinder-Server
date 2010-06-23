/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WFACTIVATIONPACKET_H
#define WFACTIVATIONPACKET_H

#include "config.h"
#include "Packet.h"

#include "WFResCodes.h"
#include "WFSubscriptionConstants.h"

class UserRight;


/**
 *   Packet for requesting the activation time
 *   or using up an activation code.
 */
class WFActivationRequestPacket : public RequestPacket {
public:

   /**
    *   Enum describing what to do.
    */
   enum whatToDo_t {
      /// Return the time for the activation
      GET_TIME = 0,
      /// Eat the activation
      USE      = 1,
      /// Update the .. fields
      UPDATE   = 2,
   };
   
   /**
    *   Send this packet to the module to do stuff with
    *   the activation code.
    *
    *   @param activationCode The activation code.
    *   @param ip The ip of the requester.
    *   @param userAgent The userAgent of the requester.
    *   @param userInput The userInput of the requester.
    *   @param userName The userName of the requester.
    *   @param UIN The UIN of the requester.
    *   @param packetID       The packet ID.
    *   @param requestID      The request ID.
    */
   WFActivationRequestPacket( const char* activationCode,
                              whatToDo_t what,
                              uint32 ip, const char* userAgent,
                              const char* userInput,
                              const char* userName,
                              uint32 UIN,
                              const char* server,
                              uint32 packetID  = MAX_UINT32,
                              uint32 requestID = MAX_UINT32 );
   
   /**
    *   Returns the activation code.
    */
   const char* getActivationCode() const;

   /**
    *   Return what to do.
    */
   whatToDo_t getWhatToDo() const;


   /**
    * Return the ip.
    */
   uint32 getIP() const;


   /**
    * Return the UserAgent.
    */
   const char* getUserAgent() const;


   /**
    * Return the UserInput.
    */
   const char* getUserInput() const;


   /**
    * Return the UserName.
    */
   const char* getUserName() const;


   /**
    * Return the server.
    */
   const char* getServer() const;


   /**
    * return the UIN.
    */
   uint32 getUIN() const;

   
private:
   int calcPacketSize( const char* activationCode,
                       const char* userAgent,
                       const char* userInput,
                       const char* userName,
                       const char* server );

   /// Position in packet of what to do
   static const int WHAT_TO_DO_POS = REQUEST_HEADER_SIZE;


   /// Position in packet of UIN.
   static const int UIN_POS = WHAT_TO_DO_POS + 4;


   /// Position in packet of ip.
   static const int IP_POS = UIN_POS + 4;
   
   /// Position of activationCode
   static const int ACTIVATION_CODE_POS = IP_POS + 4;
   
   // And the rest of the strings are after that... 
};

/**
 *   The reply to a WFActivationRequestPacket.
 *  
 */
class WFActivationReplyPacket : public ReplyPacket {
public:
   /**
    * @param req The request.
    * @param resultCode What happened. OK or WRONG_ACTIVATION_CODE.
    * @param rights The rights for this activation as string.
    * @param ownerUIN The UIN of the owner of the activation 0 if not 
    *                 owned.
    * @param server The other instance this code belongs to. 
    */
   WFActivationReplyPacket( const WFActivationRequestPacket* req,
                            WFResCodes::activation_res resultCode,
                            const char* rights, uint32 ownerUIN,
                            const char* server );

   /**
    * Returns the result code.
    */
   WFResCodes::activation_res getResultCode() const;


   /**
    * Get the owner UIN for the activation code, 0 if not used.
    */
   uint32 getOwnerUIN() const;


   /**
    * Get the server string. Empty means no server set.
    */
   const char* getServer() const;


   /**
    * Get the rights for this activation code. As unoarsed string.
    */
   const char* getRights() const;

   
private:

   /// Position in packet for result code
   static const int RESULT_CODE_POS = REPLY_HEADER_SIZE;


   /// Position in packet for owner uin.
   static const int UIN_POS         = RESULT_CODE_POS + 4;


   /// Position in packet for rights string
   static const int RIGHTS_POS  =  UIN_POS + 4;


   /// Position for server string is after the rights string.
};

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSSENDREQUEST_H
#define SMSSENDREQUEST_H


#include "Types.h"
#include "config.h"
#include "Request.h"
#include "SMSPacket.h"


/**
 *    Sends a SMSSendRequestPacket to the SMSModule and waits for the 
 *    SMSSendReplyPacket.
 *
 */
class SMSSendRequest : public Request {
  public:
   /**
     *   The timout-time before resending an SMSSendRequestPacket to 
     *   the SMSModule (in ms).
     */
   static const uint32 SMSSENDREQUESTPACKET_TIMEOUT;
  
   /**
     *   Creates a new empty request. It sends the SMSSendRequestPacket(s) 
     *   and waits for the answer(s).
     */
   SMSSendRequest( uint16 requestID);

   /**
     *   Creates a new request. It sends the SMSSendRequestPacket(s) 
     *   and waits for the answer(s).
     */
   SMSSendRequest( uint16 requestID,
                   SMSSendRequestPacket* p );

   /**
     *   Destructs the request.
     */
   virtual ~SMSSendRequest();

   /**
     *   Add a new SMSSendRequestPacket to this request.
     *   @param   pack  The new SMSSendPacket.
     *   @return  True if the packet added ok, false otherwise 
     *            (pack == NULL).
     */
   bool addNewSMS(SMSSendRequestPacket* pack);


   /**
    * @return the next packet to send or NULL if nothing to send.
    */
	PacketContainer* getNextPacket();
   

   /**
     *   Handle an answer.
     *   @param ans  The answer that should be handled by this request.
     */
	void processPacket(PacketContainer *ans);


   /**
     *   Returns the last SMSSendReplyPacket if all went OK or NULL 
     *   if something went very wrong.
     */
	PacketContainer* getAnswer();


   /**
    *    The number of SMS to send.
    */
   uint32 getNbrSMSs();


   /**
    * Returns true if all SMS has been sent and received.
    */
   bool virtual requestDone();


   /**
    * The index'th sms.
    *
    * @param index The index of the SMS to return.
    * @return The SMSSendRequestPacket at index index or NULL if index
    *         is out of bounds.
    */
   SMSSendRequestPacket* getSMSSendRequestPacket( uint32 index );


   private:
      /**
        *   Special SMSPacketContainer that contains a pointer to the
        *   next SMSPacketContainer.
        */
      class SMSPacketContainer : public PacketContainer {
         public:
            /**
              *   Create a new SMSPacketContainer. The parameters
              *   are given direct to the constructor in the
              *   PacketContainer.
              *   @see  PacketContainer
              *   @param   list  Pointer to the first SMS in the
              *                  list of send packets
              */
            SMSPacketContainer(Packet* thePacket, 
                               uint32 serverTimestamp,
                               uint16 serverResend,
                               moduletype_t type,
                               uint32 resendTimeout = defaultResendTimeoutTime,
                               uint32 nbrResends = defaultResends )
               : PacketContainer(thePacket, serverTimestamp, 
                                 serverResend, type, resendTimeout, 
                                 nbrResends) 
            {
               m_next = NULL;
            }

            /**
              *   @return  The SMSPacketContainer that follows this one.
              */
            SMSPacketContainer* getNext() {
               return (m_next);
            }

            /**
              *   @param   newNext  The SMSPacketContainer that should
              *                     follow this one.
              */
            void setNext(SMSPacketContainer* newNext) {
               m_next = newNext;
            }

         private:
            /**
              *   The SMSPacketContainer that follows this one.
              */
            SMSPacketContainer* m_next;

      };

      /** 
        *   The first SMSPacketContainer to send.
        */
      SMSPacketContainer* m_firstSendPacket;

      /**
        *   Pointer to the current SMSPacketContainer that should
        *   be send. Only increased by the getNextPacket-method.
        */
      SMSPacketContainer* curSMSPacketContainer;
      

      /**
        *   The number of packets to send.
        */
      uint16 m_nbrSendPackets;

      /**
        *   The number of packets received.
        */
      uint16 m_nbrReceivedPackets;

      /**
        *   The answer from this Request.
        */
      SMSPacketContainer* m_answer;

};

#endif // SMSSENDREQUEST_H


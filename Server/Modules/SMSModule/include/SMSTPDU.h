/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSTPDU_H
#define SMSTPDU_H
#include "Types.h"

#define SMS_(x) x

/**
 *    SMSTPDUContainer.
 *
 */
class SMSTPDUContainer {
      /**
       */
      uint8 *pnext;

      /** 
       *    Buffer to hold data while making a message.
       */
      uint8 data[160];

      /** 
       *    @name Actual SMS
       *    @memo These variables _are_ the SMS Message.
       *    @doc  These variables _are_ the SMS Message. All constructors
       *          should set these variables properly
       */
      //@{
         /** 
          *    The data to send in the SMS packet.
          */
         uint8 m_payLoad[160];
         
         /** 
          *    The size of the data that is about to be sent.
          */
         int   m_payLoadSize;
         
         /** 
          *    Address of service center.
          */
         char m_serviceCenter[50];
         /** 
          *    Address of recipient phone.
          */
         char m_recipientPhone[50];
      //@}

   public:
      /**
       */
      enum {
         SMS_(SCA_ADDRESS_TYPE     ) = 0x91,
         SMS_(TP_MESSAGE_TYPE      ) = 0x11,
         SMS_(TP_MESSAGE_TYPE_HI   ) = 0x51,
         SMS_(TP_MESSAGE_REFERENCE ) = 0x00,
         SMS_(DEST_ADDRESS_TYPE    ) = 0x91,
         SMS_(MESSAGE_TP_PID       ) = 0x00,
         SMS_(MESSAGE_TP_DCS        ) = 0x00,
         SMS_(MESSAGE_TP_VP        ) = 0xA7,
         SMS_(INFO_ELEMENT_ID      ) = 0x06,
         SMS_(EL_INSERT_TYPE_INFORMATION ) = 0x03,
      };


      /**
       *    Creates a new SMSTPDU from data.
       *    @pPayload       Pointer to the data.
       *    @nPayloadSize   Length of the data.
       *    @sRxPhone       Null-terminated string with phonenumber.
       *    @sServiceCenter Optional service center address.
       */
      SMSTPDUContainer(uint8 *pPayload,
                       size_t nPayloadSize = 0,
                       const char *sRxPhone = "",
                       const char *sServiceCenter = "")
       {
          strcpy(m_recipientPhone, sRxPhone);
          strcpy(m_serviceCenter, sServiceCenter);
          memcpy(m_payLoad, pPayload, nPayloadSize);
          pnext = data;
          m_payLoadSize = nPayloadSize;
          // Move this following one to getMessageAsHex.
          makeMessage(pPayload, nPayloadSize, sRxPhone, sServiceCenter);
       }

      /**
       *    Create a new SMSTPDUContainer from data in hex-form,
       *    as received from the phone
       *    @param hexData Data from the phone. Nullterminated, please.
       */
      SMSTPDUContainer(char* hexData);

      /**
       *    Put the message as hexadecimal characters into hexData.
       *    This function is used when sending data to the phone.
       *    @param   hexData     
       *    @param   maxLength   
       */
      int getMessageAsHex(char* hexData, size_t maxLength);

      /**
       *    Get the phonenumber of the message.
       *    @return Pointer to the telephone number of the message. Sender
       *            when receiving and recipient when sending.
       *            N.B. Just a pointer into the object. If you want
       *            to use it after the object is destroyed you must copy
       *            it. 
       */
      const char* getPhoneNumber() const {
         return m_recipientPhone;
      }

      /**
       *    Get the phonenumber of the service center.
       *    @return The telephone number of the service center.
       *            N.B. Just a pointer into the object. If you want
       *            to use it after the object is destroyed you must copy
       *            it.
       */
      const char* getServiceCenterNumber() const {
         return m_serviceCenter;
      }

      /**
       *    Get the size of the actual SMS data.
       *    @return The size of the actual SMS data.
       */
      int getPayloadSize() const {
         return m_payLoadSize;
      }

      /**
       *    Get the actual SMS data.
       *    @return The actual SMS data. ( 7-bits only ).
       */
      const uint8* getPayload() const {
         return m_payLoad;
      }
 
  private:
      /**
       *    @param   payload        
       *    @param   len            
       *    @param   rxPhone        
       *    @param   sServiceCenter 
       */
      void makeMessage(uint8 *payload,
                       size_t len = 0,
                       const char *rxPhone = 0,
                       const char *sServiceCenter = 0);

      /**
       *    @param   hexData 
       *    @return  
       */
      int decodeHexMessage(char* hexData);

      /**
       *    @param   outBuffer   
       *    @param   inBuffer    
       *    @param   offset      
       *    @param   len         
       */
      void unpackAddress(char* outBuffer,
                         uint8* inBuffer,
                         int offset,
                         int len);

  protected:
      /**
       */
      void   reset() {
         pnext = data;
      }

  public:
      /**
       */
      size_t count(void) {
         return pnext - data;
      }
      
      /**
       */
      size_t countFree(void) {
         return sizeof(data) - count();
      }

  protected:
      /**
       */
      uint8* position(void) {
         return pnext;
      }

      /**
       */
      SMSTPDUContainer& operator<<(uint8 n) {
         *pnext++ = n;
         // assert(pnext - data < (int)sizeof(data));
         return *this;
      }
     
      /**
       *    @param   sPhoneNo 
       */
      void addAddress(char *sPhoneNo);
};

#endif


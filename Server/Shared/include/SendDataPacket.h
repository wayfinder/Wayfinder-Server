/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SENDDATAPACKET_H
#define SENDDATAPACKET_H

#include "config.h"
#include "Packet.h"
#include "URL.h"

/**
 * Packet to send data over the internet to outside MC2. 
 *
 */
class SendDataRequestPacket : public RequestPacket {
   public:
      /**
       * Type of data.
       */
      enum dataType {
         raw = 0,
         http = 1,
      };

      /**
       * Creates a SendDataRequestPacket.
       * 
       * @param peer The host and port to send data to. Proto ssl/https
       *             gives ssl connection.
       * @param data The bytes to send to the host and port.
       * @param dataLen The number of bytes in data.
       * @param expectedReplyData The regular expression required in the 
       *                          reply from the peer.
       */
      SendDataRequestPacket( const URL& peer,
                             const byte* data, uint32 dataLen,
                             const char* expectedReplyData,
                             dataType type );

      /**
       * Get the peer.
       */
      URL getPeer() const;

      /**
       * Get pointer to the data.
       */
      const byte* getData() const;

      /**
       * Get the length of the data.
       */
      uint32 getDataLength() const;

      /**
       * Get the expected regular expression.
       */
      const char* getExpectedReplyData() const;

      /**
       * Get the dataType.
       */
      dataType getDataType() const;

   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         dataType_POS            = REQUEST_HEADER_SIZE,
         dataLen_POS             = dataType_POS + 4,
         urlLength_POS           = dataLen_POS + 4,
         expectedReplyData_POS   = urlLength_POS + 4,
         
         endStatic_POS           = expectedReplyData_POS + 4
      };
};


/**
 * Reply Packet to send data over the internet to outside MC2 request 
 * packet.
 * Contains only status.
 *
 */
class SendDataReplyPacket : public ReplyPacket {
   public:
      /**
       * Reply to a SendDataRequestPacket.
       */
      SendDataReplyPacket( const SendDataRequestPacket* r,
                           uint32 status );
};

#endif // SENDDATAPACKET_H


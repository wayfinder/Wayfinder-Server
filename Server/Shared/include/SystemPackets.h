/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYSTEMPACKETS_H
#define SYSTEMPACKETS_H


#include "config.h"
#include "Packet.h"

#define ACKTIME_NOMAP 50

/**
 *    An acknowledge that is send from a module back to the server. This
 *    gives the module the opertunity to tell the server about 
 *    the status of the RequestPacket (updated eta. etc.).
 *
 */
class AcknowledgeRequestReplyPacket : public ReplyPacket {
   public:
      /**
       * Creates a AcknowledgeRequestReplyPacket with status and
       * estimated time to arrival.
       * 
       * @param rp      The RequestPacket that this is an answer to.
       * @param status  StringTable::OK means that the request was received
       *                ok but a new timeout is needed, StringTable::NOTOK
       *                means that the request was received but no answer
       *                is forthcoming and the request should be aborted.
       * @param eta     Estimated Time to Arrival of reply in ms.
       */
      AcknowledgeRequestReplyPacket( const RequestPacket* rp, 
                                     uint32 status, 
                                     uint32 eta );


      /**
       *    The new estimated time of arrival.
       *    @return The new timeout time.
       */ 
      inline uint32 getETA() const;

      
      /**
       *    Set a new estimated time of arrival.
       *    @param eta The new estimated time to arrival of reply in ms.
       */
      inline void setETA( uint32 eta );
};


//**********************************************************************
// Inlines
//**********************************************************************


inline uint32 
AcknowledgeRequestReplyPacket::getETA() const {
   return readLong( REPLY_HEADER_SIZE );
}


inline void 
AcknowledgeRequestReplyPacket::setETA( uint32 eta ) {
   writeLong( REPLY_HEADER_SIZE, eta );
}


#endif // SYSTEMPACKETS_H

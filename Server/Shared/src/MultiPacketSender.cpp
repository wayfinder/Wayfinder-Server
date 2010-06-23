/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MultiPacketSender.h"

#include "DataBuffer.h"
#include "Packet.h"
#include "DeleteHelpers.h"

MultiPacketSender::MultiPacketSender( const IPnPort& destination )
   throw (SocketException) :
   BufferSender( destination ),
   m_readyToSend( false ) { // not ready until we get a packet
 
}

MultiPacketSender::~MultiPacketSender() {
   STLUtility::deleteQueueValues( m_buffers );
}

void MultiPacketSender::addPacket( const Packet& packet )
{
   // 
   // Create buffer type from packet and add it to send list
   // 
   BufferType* buff = new BufferType( 4 + packet.getLength() );
   DataBuffer lenBuff( 4 );
   lenBuff.writeNextLong( packet.getLength() );
   memcpy( buff->begin(), lenBuff.getBufferAddress(), 4 );
   memcpy( buff->begin() + 4, packet.getBuf(),
           packet.getLength() );

   m_buffers.push( buff );
   m_readyToSend = true;
}

bool MultiPacketSender::empty() const {
   return m_buffers.empty();
}

void MultiPacketSender::
handleIO( bool readyRead, bool readyWrite ) try {
   mc2dbg4 << "[MultiPacketSender]:handleIO( " 
           << readyRead << ", " << readyWrite << ")" << endl;

   dequeueSendPacket();
   BufferSender::handleIO( readyRead, readyWrite );
   dequeueSendPacket();

   if ( getState() == ERROR ) {
      m_readyToSend = false;
   }

// in case we try to reconnect, catch errors here
} catch ( SocketException socketError ) {
   mc2dbg << "[MPS]: " << socketError.what() << endl;
   setTimeout( 0 );
}

void MultiPacketSender::dequeueSendPacket() {
   // Do not set new buffer in sendder until we
   // have sent the previous.
   if ( getState() != DONE && 
        getState() != READY ) {
      return;
   }

   //
   // If buffer sender is done and we
   // still have buffers to send then
   // set new send buffer
   //
   if ( ! m_buffers.empty() ) {
      auto_ptr<BufferType> buffer( m_buffers.front() );
      m_buffers.pop();
      setBuffer( *buffer );
      setWorking();
      m_readyToSend = true;
   } else {
      mc2dbg2 << "[MultiPacketSender]: all packets sent. "
             << "Waiting for new packets." << endl;
      //
      // Set this selectable in idle state.
      // i.e dont select it for write, instead
      // select it for read to check for errors.
      // see wantRead/wantWrite  
      // Set longer timeout now
      //
      setIdle();
      m_readyToSend = false;
   }

}
bool MultiPacketSender::wantWrite() const {
   return m_readyToSend;
}

bool MultiPacketSender::wantRead() const {
   // use this to check for errors, if we get read ready -> error
   return true;
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETRECEIVESOCKET_H
#define PACKETRECEIVESOCKET_H


#include "config.h"
#include "TCPSocket.h"


/**
 * Socket for receiving Packets over tcp.
 * 
 *
 */
class PacketReceiveSocket : public TCPSocket {
   public:
      /**
       * Create a new socket.
       */
      PacketReceiveSocket();
      /**
       * Constructor that takes an fd as parameter.
       * 
       * @param   sock     The filedescriptor of the socket.
       * @param   backlog  The value of the backlog variable.
       */
      PacketReceiveSocket( SOCKET sock, int backlog = DEFAULT_BACKLOG );


      /**
       * Destructor.
       */
      virtual ~PacketReceiveSocket();

      
      /**
       * Accept a connection. 
       *
       * @return  A PacketReceiveSocket
       *          This socket must be deleted by the caller to accept.
       */
      virtual TCPSocket* accept();


      /**
       * Get the size of the read packet.
       */
      uint32 getSize() const;


      /**
       * Get the buffer.
       *
       * @param steal If to take ownership of the buffer.
       */
      byte* getBuffer( bool steal = false );


      /**
       * Read available bytes to make a Packet buffer.
       */
      void readBytes();


      /**
       * The current state.
       */
      enum read_state_t { 
         /// Reading the packet size (4 bytes)
         reading_size = 0,
         /// Reading the packet data
         reading_packet = 1,
         /// Done
         done = 2,
         /// Error
         error = 3
      };


      /**
       * Gets the current state.
       */
      read_state_t getReadState() const;

      
   private:
      read_state_t m_readState;


      /**
       * The size of the packet.
       */
      uint32 m_size;


      /**
       * The number read bytes so far.
       */
      uint32 m_readSize;


      /**
       * The packet buffer.
       */
      byte* m_buffer;


      /**
       * The packet size buffer.
       */
      uint32 m_sizeBuf;
};


// =======================================================================
//                                 Implementation of the inlined methods =


inline uint32
PacketReceiveSocket::getSize() const {
   return m_size;
}


inline PacketReceiveSocket::read_state_t
PacketReceiveSocket::getReadState() const {
   return m_readState;
}


#endif // PACKETRECEIVESOCKET_H


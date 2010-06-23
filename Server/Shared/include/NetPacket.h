/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NETPACKET_H
#define NETPACKET_H

#include "config.h"
#include "IPnPort.h"
#include <memory>

class Packet;

/**
 * Simple container class for a packet, the packets destination
 * and what protocol the packet should be sent with.
 */
class NetPacket {
public:
   /// packet types
   enum Type { 
      UDP, //< wants to be sent via UDP 
      TCP  //< wants to be sent via TCP
   };

   /**
    * creates a net packet
    * @param packet the packet to send, will be deleted.
    * @param destination the destination ip and port
    * @param type the packet send type
    */
   NetPacket( Packet* packet, 
              const IPnPort& destination,
              Type type );

   ~NetPacket();

   /// @return send packet type
   Type getType() const { return m_type; }
   /// @return packet to send
   const Packet& getPacket() const { return *m_packet; }
   /// @return destination
   const IPnPort& getDestination() const { return m_destination; }

private:
   std::auto_ptr<Packet> m_packet;
   IPnPort m_destination;
   Type m_type;
};

#endif // NETPACKET_H

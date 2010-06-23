/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETNET_H
#define PACKETNET_H

#include "config.h"

class IPnPort;
class Packet;
class TCPSocket;
class URL;

namespace PacketUtils {

/**
 * Connect to destination and send a packet via TCP
 * @param pack the packet to send
 * @param destination target destination
 * @param timeoutUS the timeout in micro seconds before 
 *                  send is considered broken
 * @return true on success
 */
bool sendViaTCP( const Packet& pack,
                 const IPnPort &destination,
                 uint32 timeoutUS = MAX_UINT32 );
/**
 * Sends a packet on a specific socket
 * @param pack the packet to send
 * @param sock the socket to send the packet
 * @param timeoutMicros the timeout in micro seconds 
 *                      before send is considered broken
 * @return true on successfull
 */
bool sendViaTCP( const Packet& pack,
                 TCPSocket& sock,
                 uint32 timeoutMicros = MAX_UINT32 );
/**
 * receive a packet from a socket
 * @param socket the socket to read from
 * @param timeout
 * @return allocated packet on success, else false
 */
Packet* receivePacket( TCPSocket& socket, uint32 timeout = MAX_UINT32 );

/**
 * Save URL into a packet
 * @param packet
 * @param pos Initial position in packet.
 * @param url
 */
void saveURL( Packet& packet, int& pos, const URL& url );

/**
 * Loads an URL from a packet.
 * @param packet
 * @param pos Initial position in packet.
 * @return URL loaded from packet.
 */
URL loadURL( const Packet& packet, int& pos );

}

#endif // PACKETNET_H

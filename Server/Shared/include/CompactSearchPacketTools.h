/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "CompactSearch.h"

class Packet;

/**
 * \namespace Holds packet tools to store and retreive compact search data.
 */
namespace CompactSearchPacketTools {

/**
 * Calculate the packet size needed to store \c container.
 * @param container
 * @return Size of packet in bytes to store \c container.
 */
uint32 calcPacketSize( const CompactSearchHitTypeVector& container );

/**
 * Write all hit types to a packet. Assumes there is enough room for the
 * container to fit in the packet.
 * @param packet Will write data to this packet.
 * @param pos The start position to start write, will return the end position.
 * @param container All hit types to write into packet.
 */
void writeToPacket( Packet& packet, int& pos,
                    const CompactSearchHitTypeVector& container );

/**
 * Read hit types from a packet.
 * @param packet Load from this.
 * @param pos Byte position to start load from.
 * @param container Will be filled with hit types from packet.
 */
void readFromPacket( Packet& packet, int& pos,
                     CompactSearchHitTypeVector& container );

}


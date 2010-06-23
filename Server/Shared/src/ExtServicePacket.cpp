/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExtServicePacket.h"

#include "CompactSearchPacketTools.h"

ExtServiceRequestPacket::
ExtServiceRequestPacket( PacketID packID,
                         RequestID reqID,
                         uint32 crc ):
   RequestPacket( REQUEST_HEADER_SIZE + 4, // header + crc,
                  0, // prio
                  PACKETTYPE_EXTSERVICE_LIST_REQUEST,
                  packID,
                  reqID,
                  Packet::MAX_MAP_ID ) {
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong( pos, crc );
   setLength( REQUEST_HEADER_SIZE );
}


uint32 ExtServiceRequestPacket::getCRC() const {
   return readLong( REQUEST_HEADER_SIZE );
}

ExtServiceReplyPacket::
ExtServiceReplyPacket( const ExtServiceRequestPacket& request,
                       const CompactSearchHitTypeVector& hitTypes,
                       uint32 crc ):
   ReplyPacket( REPLY_HEADER_SIZE +
                // reply data size
                CompactSearchPacketTools::calcPacketSize( hitTypes ) +
                4, // crc size
                PACKETTYPE_EXTSERVICE_LIST_REPLY,
                &request,
                StringTable::OK ) {
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, crc );
   CompactSearchPacketTools::writeToPacket( *this, pos, hitTypes );
   setLength( pos );
}


uint32 ExtServiceReplyPacket::getCRC() const {
   return readLong( REPLY_HEADER_SIZE );
}

void ExtServiceReplyPacket::
getHitTypes( CompactSearchHitTypeVector& hitTypes ) {
   int pos = REPLY_HEADER_SIZE + 4;
   CompactSearchPacketTools::readFromPacket( *this, pos, hitTypes );
}

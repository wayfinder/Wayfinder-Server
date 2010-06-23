/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTSERVICE_PACKET_H
#define EXTSERVICE_PACKET_H

#include "Packet.h"
#include "CompactSearch.h"

/**
 *
 */
class ExtServiceRequestPacket: public RequestPacket {
public:
   /**
    * @param crc Checksum for service.
    */
   ExtServiceRequestPacket( PacketID packID,
                            RequestID reqID,
                            uint32 crc );
   /// @return Checksum.
   uint32 getCRC() const;
};

/**
 * Reply data from a ExtServiceRequest.
 * This packet contains the compact search hit type description.
 * Conceptual similar to the TopRegionReplyPacket
 */
class ExtServiceReplyPacket: public ReplyPacket {
public:
   ExtServiceReplyPacket( const ExtServiceRequestPacket& request,
                          const CompactSearchHitTypeVector& container,
                          uint32 crc );

   /**
    * @return Checksum of the headers.
    */
   uint32 getCRC() const;

   /**
    * @param container
    */
   void getHitTypes( CompactSearchHitTypeVector& container );
};

#endif // EXTSERVICE_PACKET_H

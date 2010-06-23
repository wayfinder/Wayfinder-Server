/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FetchAllDisturbancesPacket.h"
#include "DisturbancePacketUtility.h"
#include "StringTable.h"

FetchAllDisturbancesRequestPacket::
FetchAllDisturbancesRequestPacket( PacketID pid,
                                   RequestID rid,
                                   const MC2String& providerID ):
   RequestPacket( MAX_PACKET_SIZE,
                  0, // prio, not important
                  PACKETTYPE_FETCH_ALL_DISTURBANCES_REQUEST,
                  pid, // packet id
                  rid, // request id
                  Packet::MAX_MAP_ID ) {
   int position = REQUEST_HEADER_SIZE;
   incWriteString( position, providerID );
   setLength( position );
}

MC2String FetchAllDisturbancesRequestPacket::getProviderID() const {
   int position = REQUEST_HEADER_SIZE;
   return incReadString( position );
}


FetchAllDisturbancesReplyPacket::
FetchAllDisturbancesReplyPacket( const RequestPacket* request,
                                 const Disturbances& disturbances ):
   ReplyPacket( DisturbancePacketUtility::
                calcPacketSize( REPLY_HEADER_SIZE,
                                disturbances ),
                PACKETTYPE_FETCH_ALL_DISTURBANCES_REPLY,
                request,
                StringTable::OK ) {
   int position = REPLY_HEADER_SIZE;
   DisturbancePacketUtility::
      writeToPacket( disturbances,
                     this, position, // packet and positioning
                     false, // don't remove disturbances ( not used )
                     false ); // remove all, ( not used )
   setLength( position );
}

void FetchAllDisturbancesReplyPacket::
getDisturbances( Disturbances& dists ) const {
   bool notUsedRemoveDist; // dummy variable, not used
   bool notUsedRemoveAll; // dummy variable, not used.

   int position = REPLY_HEADER_SIZE;
   DisturbancePacketUtility::
      getDisturbances( dists,
                       this, position,
                       notUsedRemoveDist, notUsedRemoveAll );
   
}

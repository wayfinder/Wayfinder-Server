/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandPacket.h"
#include "ExpandRequestData.h"
#include "SearchReplyData.h"
#include "UserData.h"
#include "LangTypes.h"
#include "UserRightsMapInfo.h"
#include "PacketHelpers.h"

int
ExpandRequestPacket::calcPacketSize( const ExpandRequestData& data,
                                     const UserUser* user ) {
   int size = data.getSizeInPacket();
   UserRightsMapInfo rights( data.getIDPair().first, user );
   size += rights.getSizeInPacket() + 4;
   
   return size;
} 
  
ExpandRequestPacket::ExpandRequestPacket( const ExpandRequestData& data,
                                          const UserUser* user )
      : RequestPacket( REQUEST_HEADER_SIZE + calcPacketSize( data, user ), 
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_EXPAND_REQUEST,
                       Packet::NO_PACKET_ID, // Packet ID
                       Packet::MAX_REQUEST_ID,
                       Packet::MAX_MAP_ID )
   
{
   int pos = REQUEST_HEADER_SIZE;
   UserRightsMapInfo rights( data.getIDPair().first, user );
   SaveLengthHelper slh ( this, pos );
   slh.writeDummyLength( pos );
   rights.save( this, pos );
   slh.updateLengthUsingEndPos( pos );
   data.save( this, pos );
   setLength( pos );

   if( data.getIDPair().first == MAX_UINT32 ) {
      setMapID( rights.getFirstAllowedMapID() );
   } else {
      setMapID( data.getIDPair().first );
   }
}

void
ExpandRequestPacket::get( ExpandRequestData& data,
                          UserRightsMapInfo& rights ) const
{
   int pos = REQUEST_HEADER_SIZE;
   LoadLengthHelper llh ( this, pos );
   llh.loadLength( pos );
   rights.load( this, pos );
   llh.skipUnknown( pos );
   data.load( this, pos );
}

ExpandReplyPacket::ExpandReplyPacket( const RequestPacket* req,
                                      const SearchReplyData& res )
      : ReplyPacket( 65536, // FIXME: Calculate this somehow
                     Packet::PACKETTYPE_EXPAND_REPLY,
                     req,
                     StringTable::OK )
{
   int pos = REPLY_HEADER_SIZE;
   SaveLengthHelper slh ( this, pos );
   slh.writeDummyLength( pos );
   res.save( this, pos );
   slh.updateLengthUsingEndPos( pos );
   setLength( pos );
}

void
ExpandReplyPacket::get( SearchReplyData& res ) const
{
   int pos = REPLY_HEADER_SIZE;
   LoadLengthHelper llh ( this, pos );
   llh.loadLength( pos );
   res.load( this, pos );
   llh.skipUnknown( pos );
}
                  

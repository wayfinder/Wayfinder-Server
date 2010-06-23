/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SendDataPacket.h"
#include "PacketNet.h"

SendDataRequestPacket::SendDataRequestPacket( 
   const URL& peer,
   const byte* data, uint32 dataLen,
   const char* expectedReplyData,
   dataType type )
      : RequestPacket( REQUEST_HEADER_SIZE + 4*4 + peer.getSizeAsBytes()
                       + dataLen + strlen( expectedReplyData ) + 1,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_SEND_DATA_REQUEST,
                       0, 0, MAX_UINT32 )
{
   int pos = REQUEST_HEADER_SIZE;

   incWriteLong( pos, type );
   incWriteLong( pos, dataLen );
   incWriteLong( pos, peer.getSizeAsBytes() );
   incWriteLong( pos, strlen( expectedReplyData ) + 1 );
   PacketUtils::saveURL( *this, pos, peer );
   incWriteByteArray( pos, data, dataLen );
   incWriteString( pos, expectedReplyData );

   setLength( pos );
}

URL
SendDataRequestPacket::getPeer() const {
   int pos = endStatic_POS;
   return PacketUtils::loadURL( *this, pos );
}

const byte*
SendDataRequestPacket::getData() const {
   int pos = endStatic_POS;
   pos += readLong( urlLength_POS );
   uint32 dataLen = getDataLength();
   return incReadByteArray( pos, dataLen );
}

uint32
SendDataRequestPacket::getDataLength() const {
   return readLong( dataLen_POS );
}

const char*
SendDataRequestPacket::getExpectedReplyData() const {
   int pos = endStatic_POS;
   pos += readLong( urlLength_POS );
   pos += getDataLength();

   return incReadString( pos );
}

SendDataRequestPacket::dataType
SendDataRequestPacket::getDataType() const {
   return dataType( readLong( dataType_POS ) );
}

SendDataReplyPacket::SendDataReplyPacket( const SendDataRequestPacket* r,
                                          uint32 status )
      :  ReplyPacket( REPLY_HEADER_SIZE,
                      Packet::PACKETTYPE_SEND_DATA_REPLY,
                      r,
                      status )
{
   setLength( REPLY_HEADER_SIZE );
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SignPost.h"
#include "Packet.h"
#include "DataBuffer.h"
#include "BitUtility.h"
#include "PacketDataBuffer.h"

void
SignColors::load( const Packet* p, int& pos ) {
   textColor = GDUtils::Color::imageColor( p->incReadByte( pos ) );
   frontColor = GDUtils::Color::imageColor( p->incReadByte( pos ) );
   backColor = GDUtils::Color::imageColor( p->incReadByte( pos ) );
}

void
SignColors::save( Packet* p, int& pos ) const {
   p->incWriteByte( pos, textColor );
   p->incWriteByte( pos, frontColor );
   p->incWriteByte( pos, backColor );
}

uint32
SignColors::getSizeInPacket() const {
   return 3 + 1/*Possible padding*/;
}


SignPost::SignPost()
      : m_colors(), m_dist( 0 ), m_stringCode( 0 )
{}

SignPost::SignPost( const SignColors& colors,
                    uint32 dist,
                    uint32 stringCode, bool isExit )
      : m_colors( colors ), m_dist( dist ), m_stringCode( stringCode )
{
   BitUtility::setBit( m_dist, 31, isExit );
}

SignPost::~SignPost() {
}

void
SignPost::save( DataBuffer& dataBuffer ) const {
   dataBuffer.writeNextLong( m_stringCode );
   dataBuffer.writeNextLong( m_dist );
   PacketDataBuffer::saveAsInPacket( dataBuffer, m_colors );
}

void
SignPost::load( DataBuffer& dataBuffer ) {
   m_stringCode = dataBuffer.readNextLong();
   m_dist = dataBuffer.readNextLong();
   PacketDataBuffer::loadAsFromPacket( dataBuffer, m_colors );
}

uint32
SignPost::getDist() const {
   return m_dist & 0x7f;
}

bool
SignPost::getExit() const {
   return BitUtility::getBit( m_dist, 31 );
}

void
SignPost::setDist( uint32 dist ) {
   bool isExit = getExit();
   m_dist = dist;
   setExit( isExit );
}

void
SignPost::setExit( bool s ) {
   BitUtility::setBit( m_dist, 31, s );
}

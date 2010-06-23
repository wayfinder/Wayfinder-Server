/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandStringSignPost.h"
#include "Packet.h"

void
ExpandStringSignPost::load( const Packet* p, int& pos ) {
   m_text        = p->incReadString( pos );
   m_textColor   = p->incReadByte( pos );
   m_frontColor  = p->incReadByte( pos );
   m_backColor   = p->incReadByte( pos );
   m_ambiguous   = p->incReadByte( pos ) != 0;
   m_priority    = p->incReadLong( pos );
   m_elementType = p->incReadLong( pos );
   m_dist = p->incReadLong( pos );
   mc2dbg4 << "Read dist " << m_dist << " at " << (pos-4) << endl;
}

void
ExpandStringSignPost::save( Packet* p, int& pos ) const {
   p->incWriteString( pos, m_text );
   p->incWriteByte( pos, m_textColor );
   p->incWriteByte( pos, m_frontColor );
   p->incWriteByte( pos, m_backColor );
   p->incWriteByte( pos, m_ambiguous );
   p->incWriteLong( pos, m_priority );
   p->incWriteLong( pos, m_elementType );
   mc2dbg4 << "Writting dist " << m_dist << " at " << pos << endl;
   p->incWriteLong( pos, m_dist );
}

uint32
ExpandStringSignPost::getSizeInPacket() const {
   return m_text.size() + 1 + 3
      + 1 + 4*3
      +3/*Possible padding*/;
}

void
ExpandStringSignPost::dump( ostream& out ) const {
   out << "Text " << MC2CITE( m_text ) << " TextC " << int( m_textColor )
       << " FrontC " << int( m_frontColor ) << " BackC " << int( m_backColor )
      // << " Ambiguous " << StringUtility::booleanAsString( m_ambiguous )
      /*<< " Priority " << m_priority*/ << " elementType "
       << MC2HEX( m_elementType ) << " Distance " << m_dist;
}


void
ExpandStringSignPosts::load( const Packet* p, int& pos ) {
   uint32 nbr = p->incReadLong( pos );
   resize( nbr );
   mc2dbg4 << "Read size " << size() << "(" << nbr << ")" << " at " << pos << endl;
   for ( uint32 i = 0 ; i < nbr ; ++i ) {
      (*this)[ i ].load( p, pos );
   }
}

void
ExpandStringSignPosts::save( Packet* p, int& pos ) const {
   p->incWriteLong( pos, size() );
   mc2dbg4 << "Wrote size " << size() << " at " << pos << endl;
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      (*this)[ i ].save( p, pos );
   }
}

uint32
ExpandStringSignPosts::getSizeInPacket() const {
   uint32 bytes = 4/*nbrsignPosts*/;
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      bytes += (*this)[ i ].getSizeInPacket();
   }

   return bytes;
}

void
ExpandStringSignPosts::dump( ostream& out ) const {
   bool first = true;
   out << "Signs: ";
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      if ( !first ) {
         out << ", ";
      } else {
         first = false;
      }
      (*this)[ i ].dump( out );
   }
}

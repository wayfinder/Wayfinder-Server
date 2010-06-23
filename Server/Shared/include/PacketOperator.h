/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKET_OPERATOR_H
#define PACKET_OPERATOR_H

#include "Packet.h"
#include "AlignUtility.h"

/**
 * Operates on Packet, a container for current packet and current position
 * in packet.
 * For example:
 * \code
 * // write a long to packet
 * struct WriteLongsToPacket: public PacketOperator {
 *     WriteLongsToPacket( Packet& pack,
 *                         int& startPos ):
 *         PacketOperator( pack, startPos )
 *     {}
 *
 *     void operator()( int value ) {
 *       packet().incWriteLong( position(), value );
 *     }
 * };
 *
 * // ...
 *
 * // Add values from a container
 * Packet somePacket;
 * vector<int> values;
 * int startPos = REQUEST_HEADER_SIZE;
 * for_each( values.begin(), values.end(),
 *           WriteLongsToPacket( somePacket, startPos );
 * \endcode
 */
template < typename PacketType = Packet >
class PacketOperator {
public:
   /**
    * @param packet The packet to be operated on.
    * @param startPosition Start byte-offset in packet.
    */
   PacketOperator( PacketType& packet, int& startPosition ):
      m_packet( packet ),
      m_position( startPosition ) {
   }

   /// @return current packet
   PacketType& packet() {
      return m_packet;
   }
   /// @return current position
   int& position() {
      return m_position;
   }

   /**
    * Specialized read for each type.
    */
   template < typename T >
   inline void read( T& value );

   /**
    * Specialized write for each type.
    */
   template < typename T >
   inline void write( const T& value );


   /// align current position to long
   void alignToLong() {
      AlignUtility::alignLong( position() );
   }

private:
   /// Packet to operate on.
   PacketType& m_packet;
   /// Current byte position in packet.
   int& m_position;
};

template <> template <> inline void 
PacketOperator<>::read< uint32 >( uint32& value ) {
   value = packet().incReadLong( position() );
}

template <> template <> inline void 
PacketOperator< Packet >::read< uint64 >( uint64& value ) {
   value = packet().incReadLongLong( position() );
}

template <> template <> inline void 
PacketOperator< Packet >::read< MC2String >( MC2String& value ) {
   packet().incReadString( position(), value );
}

template <> template <> inline void 
PacketOperator< Packet >::write< uint32 >( const uint32& value ) {
   packet().incWriteLong( position(), value );
}

template <> template <> inline void
PacketOperator< Packet >::write< uint64 >( const uint64& value ) {
   packet().incWriteLongLong( position(), value );
}

template <> template <> inline void 
PacketOperator< Packet >::write< MC2String >( const MC2String& value ) {
   packet().incWriteString( position(), value );
}

#endif // PACKET_OPERATOR_H

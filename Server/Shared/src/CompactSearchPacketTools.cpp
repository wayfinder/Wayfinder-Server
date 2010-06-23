/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CompactSearchPacketTools.h"

#include "Packet.h"
#include "AlignUtility.h"
#include "ForAll.h"
#include "PacketOperator.h"
#include "PacketOperatorCommon.h"
#include "ref_cast.h"

#include <memory>

using STLUtility::ref_cast;

namespace CompactSearchPacketTools {

// always put the string on top, so we do not have to
// align more than once
#define COMPACT_SEARCH_PACKET_OP(values,op)     \
   op( values.m_name );                         \
   op( values.m_imageName );                    \
   op( values.m_type );                         \
   alignToLong();                               \
   op( ref_cast< uint32 >( values.m_nameStringCode ) ); \
   op( ref_cast< uint32 >( values.m_typeStringCode ) ); \
   op( values.m_round );                        \
   op( values.m_serviceID );                    \
   op( values.m_heading );                      \
   op( values.m_searchTypeMask );               \
   op( values.m_topRegionID );                  \
   op( ref_cast< uint32 >( values.m_language ) ); \
   op( values.m_mapRights );                    \
   op( ref_cast< uint32 >( values.m_providerType ) );  \
   op( values.m_invertRights );               

/**
 * Loads a hit type from a packet.
 */
struct LoadFromPacket: public PacketOperator< Packet > {
   LoadFromPacket( Packet& packet, int& startPosition ):
      PacketOperator<Packet>( packet, startPosition ) {
   }

   void operator()( CompactSearchHitTypeVector::value_type& hitType ) {
      COMPACT_SEARCH_PACKET_OP( hitType, read );
   }
};

/**
 * Writes a hit type to a packet.
 */
struct WriteToPacket: private PacketOperator< Packet > {
   WriteToPacket( Packet& packet, int& startPosition ):
      PacketOperator<Packet>( packet, startPosition ) {
   }

   void operator()( const CompactSearchHitTypeVector::value_type& hitType ) {
      COMPACT_SEARCH_PACKET_OP( hitType, write );
   }
};


// Specialization to calculate the size
uint32 calcPacketSize( const CompactSearchHitTypeVector& container ) {
   // Calculate the packet size using accumulate operator
   int size = 4; // for the container size.
   AccumulateSize< CompactSearchHitTypeVector::value_type > countSize( size );
   STLUtility::for_all( container, countSize );
   return countSize.size();
}

void writeToPacket( Packet& packet, int& pos,
                    const CompactSearchHitTypeVector& container ) {
   // write the container to the packet using operator WriteToPacket
   packet.incWriteLong( pos, container.size() );
   STLUtility::for_all( container, WriteToPacket( packet, pos ) );
}

void readFromPacket( Packet& packet, int& pos,
                     CompactSearchHitTypeVector& container ) {
   // Read container from packet using operator LoadFromPacket
   container.resize( packet.incReadLong( pos ) );
   STLUtility::for_all( container, LoadFromPacket( packet, pos ) );
}


} // CompactSearchPacketTools

// Specialization to calculate the size
template <>
void
AccumulateSize< CompactSearchHitTypeVector::value_type >::
operator()( const CompactSearchHitTypeVector::value_type& hitType ) {
   COMPACT_SEARCH_PACKET_OP( hitType, write );
}

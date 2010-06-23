/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCEUTILITY_H
#define DISTURBANCEUTILITY_H

#include "DisturbanceElement.h"
#include "Packet.h"
#include "STLUtility.h"

#include <map>


/**
 */
namespace DisturbancePacketUtility {

void writeToPacket( Packet& packet, int& packetPosition,
                    const DisturbanceElement& currentDist );

template < typename Container, typename Selector >
void writeToPacket( const Container& container,
                    Selector selectValue,
                    Packet* packet,
                    int& packetPosition,
                    bool removeDisturbances,
                    bool removeAll ) {

   packet->incWriteLong( packetPosition, container.size() );

   typename Container::const_iterator it = container.begin();
   for (; it != container.end(); ++it ) {
      writeToPacket( *packet, packetPosition, *selectValue( *it ) );
   }

   byte b = 0;
   if ( removeDisturbances ) {
      b |= 0x01;
   }

   if ( removeAll ) {
      b |= 0x02;
   }

   packet->incWriteByte( packetPosition, b );
   packet->setLength( packetPosition );

}
template <typename Container>
inline void writeToPacket( const Container& container,
                           Packet* packet,
                           int& packetPosition,
                           bool removeDisturbances,
                           bool removeAll ) {
   writeToPacket( container,
                  STLUtility::makeIdentity( container ),
                  packet, packetPosition,
                  removeDisturbances, removeAll );
}

// specialization for map
template <>
inline void writeToPacket<IDToDisturbanceMap>
( const IDToDisturbanceMap& container,
  Packet* packet,
  int& packetPosition,
  bool removeDisturbances,
  bool removeAll ) {
   writeToPacket( container,
                  STLUtility::makeSelect2nd( container ),
                  packet, packetPosition,
                  removeDisturbances, removeAll );
}


/**
 * Calculate the packet size needed to store one disturbance element.
 * @param disturbance The element to use when calculating packet size.
 * @return Number of bytes needed to store a disturbance element.
 */
int calcPacketSize( const DisturbanceElement& disturbance );

/**
 * Calculates the packet size needed for a set of disturbances.
 * @param headerSize The size in bytes for the packet header.
 * @param disturbances The disturbances to calculate packet size for.
 * @param selectValue A value selector, this is used to fetch the value from
 *                    the containers value.
 * @return packet size needed to fit all these disturbances.
 */
template <typename Container, typename Selector>
int calcPacketSize( int headerSize, const Container& disturbances,
                    Selector selectValue )
{
   int size = headerSize;
   size += 4; // Size of distmap

   for ( typename Container::const_iterator it = disturbances.begin(),
            itEnd = disturbances.end(); it != itEnd; ++it ) {
      size += calcPacketSize( *selectValue( *it ) );
   }
   size++; // Remove or add byte
   return size;
}

/// Calls first calcPacketSize with identity selector.
template < typename Container >
inline int calcPacketSize( int headerSize,
                           const Container& dists ) {
   return calcPacketSize( headerSize, dists,
                          STLUtility::makeIdentity( dists ) );
}

/// Map specialization of previous function, calls select2nd on container
/// value.
template <>
inline int calcPacketSize<IDToDisturbanceMap>( int headerSize,
                                               const IDToDisturbanceMap&
                                               distMap ) {
   return calcPacketSize( headerSize, distMap,
                          STLUtility::makeSelect2nd( distMap ) );
}

void getDisturbances( map<uint32, DisturbanceElement*> &distMap,
                      const Packet* packet,
                      int& packetPosition,
                      bool &removeDisturbances,
                      bool &removeAll );

void getDisturbances( vector< DisturbanceElement* > &distMap,
                      const Packet* packet,
                      int& packetPosition,
                      bool& removeDisturbances,
                      bool& removeAll );

} // DisturbancePacketUtility

#endif // DISTURBANCEUTILITY_H


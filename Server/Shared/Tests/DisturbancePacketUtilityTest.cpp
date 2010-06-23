/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbancePacketUtility.h"
#include "MC2UnitTestMain.h"
#include "DeleteHelpers.h"

MC2_UNIT_TEST_FUNCTION( disturbancePacketUtilityTest ) {
   // setup a large set of disturbances and test the packet size.

   // We use max packet size here so we can be sure to
   // go over the max packet size limit
   STLUtility::AutoContainer< vector<DisturbanceElement*> > elements;
   elements.resize( MAX_PACKET_SIZE );

   for ( size_t i = 0, n = elements.size(); i < n; ++i ) {
      elements[ i ] = new DisturbanceElement();
   }

   int headerOffset = REPLY_HEADER_SIZE;
   int packetSize = DisturbancePacketUtility::
      calcPacketSize( headerOffset, // header offset size
                      elements );

   MC2_TEST_REQUIRED( packetSize > 0 );
   // We do not store just one byte per elements in the packet
   MC2_TEST_REQUIRED( (size_t)packetSize > elements.size() - headerOffset );

   Packet packet( packetSize );
   DisturbancePacketUtility::
      writeToPacket( elements, &packet,
                     headerOffset,
                     false, // dont remove dist (not used)
                     false ); // remove all (not used)

   // it is ok if the packet size is larger, but it must NOT
   // be smaller, then we did something wrong in calcPacketSize
   MC2_TEST_CHECK( packet.getLength() >=
                   static_cast<unsigned int>( packetSize ) );
}

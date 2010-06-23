/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SimpleBalancerHelpers.h"

/// Help function for loadMapsFromString
void loadMapFromString( MapSafeVector& mapVector,
                        const char* mapstr ) {
   int map = 0;
   int size = 0;

   istringstream is( mapstr );
   string str;
   if ( getline( is, str, ':' ) ) {
      map = boost::lexical_cast<int>( str );
   }
   if ( getline( is, str, ':' ) ) {
      size = boost::lexical_cast<int>( str );
   }

   mapVector.finishLoadingMap( map, size );
}

void loadMapsFromString( MapSafeVector& mapVector,
                         const char* maps ) {
      istringstream is( maps );
      string map;
      while ( is >> map ) {
         loadMapFromString( mapVector, map.c_str() );
      }
}

void clearPacketSendList( PacketSendList& packetList ) {
   STLUtility::deleteAllSecond( packetList );
}

void updateStatisticsExpectNoPackets( SimpleBalancerTestFixture& fix ) {
   // make sure the balancer knows about its own module's statistics
   PacketSendList packetList;
   fix.balancer->timeout( fix.thisModule->createStatisticsPacket(), 
                          packetList );

   MC2_TEST_CHECK( packetList.size() == 0 );

   // give the balancer statistics packets for each module in the vector
   for ( size_t i = 0; i < fix.availables.size(); ++i ) {
      fix.balancer->updateStats( fix.availables[ i ]->createStatisticsPacket(),
                                 packetList );
      MC2_TEST_CHECK( packetList.size() == 0 );
   }

   clearPacketSendList( packetList );
}

StatisticsPacket* simpleStatisticsPacketNoMaps( const IPnPort& source) {
   MapSafeVector maps;
   return new StatisticsPacket( source, maps, 0, 0, 0, 0 );
}

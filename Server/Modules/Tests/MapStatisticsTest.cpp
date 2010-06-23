/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"
#include "MapStatistics.h"
#include "Packet.h"
#include <algorithm>

/**
 * Tests construction.
 */
MC2_UNIT_TEST_FUNCTION( instantiateMapStatistics ) {
   MapStatistics ms;
   MC2_TEST_CHECK( ms.getNbrOfMaps() == 0 );
}

/**
 * Tests the load and save functions.
 */
MC2_UNIT_TEST_FUNCTION( serializeMapStatistics ) {
   MapStatistics ms;
   vector<pair<uint32, uint32> > maps;

   maps.push_back( make_pair( 1, 10 ) );
   maps.push_back( make_pair( 2, 20 ) );
   maps.push_back( make_pair( 3, 30 ) );
   maps.push_back( make_pair( 4, 40 ) );

   for ( size_t i = 0; i < maps.size(); ++i ) {
      ms.finishLoadingMap( maps[i].first, maps[i].second );
   }

   uint64 maxMem = 200;
   uint64 optMem = 100;

   ms.setMaxMem( maxMem );
   ms.setOptMem( optMem );

   Packet pack( 10000 );
   int pos = 0;
   ms.save( &pack, pos );

   MapStatistics ms2;
   pos = 0;
   ms2.load( &pack, pos );

   MC2_TEST_CHECK( ms2.getNbrOfMaps() == maps.size() );
   set<MapElement> mapset;
   ms2.getAllMapInfo( mapset );
   MC2_TEST_CHECK( mapset.size() == maps.size() );

   vector<pair<uint32, uint32> > maps2;
   
   for ( set<MapElement>::iterator itr = mapset.begin();
         itr != mapset.end();
         ++itr ) {
      MapElement me = *itr;
      MC2_TEST_CHECK( me.getStatus() == MapElement::LOADED );
      maps2.push_back( make_pair( me.getMapID(), me.getMapSize() ) );
   }

   sort( maps.begin(), maps.end() );
   sort( maps2.begin(), maps2.end() );
   
   MC2_TEST_CHECK( maps == maps2 );
   set<uint32> avoid;
   MC2_TEST_CHECK( ms2.getOldestMapAndAge( avoid ).first == 1 );

   MC2_TEST_CHECK( ms2.getMaxMem() == maxMem );
   MC2_TEST_CHECK( ms2.getOptMem() == optMem );
}

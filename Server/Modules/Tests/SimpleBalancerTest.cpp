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
#include "SimpleBalancerHelpers.h"
#include "DeleteMapPacket.h"

ISABThreadInitialize threadInit;

/**
 * Tests instantiation and initial state of SimpleBalancer.
 */
MC2_UNIT_TEST_FUNCTION( instantiateSimpleBalancer ) {
   SimpleBalancerTestFixture fix( false /* no maps */ );

   MC2_TEST_CHECK( fix.balancer->getNbrModules() == 1 );
}

/**
 * Tests the SimpleBalancer::updateStats function.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerUpdateStats ) {
   SimpleBalancerTestFixture fix( false /* no maps */ );

   PacketSendList packetList;
   StatisticsPacket* sp = simpleStatisticsPacketNoMaps( host2 );
   fix.balancer->updateStats( sp,
                              packetList );

   // should now know about host1 and host2
   MC2_TEST_CHECK( fix.balancer->getNbrModules() == 2 );
   MC2_TEST_CHECK( packetList.size() == 0 );

   // send an equivalent packet again
   sp = simpleStatisticsPacketNoMaps( host2 );
   fix.balancer->updateStats( sp,
                              packetList );

   // should still only know about two modules
   MC2_TEST_CHECK( fix.balancer->getNbrModules() == 2 );
   MC2_TEST_CHECK( packetList.size() == 0 );

   // this should clear the module list
   fix.balancer->becomeAvailable();
   MC2_TEST_CHECK( fix.balancer->getNbrModules() == 1 );
}

/**
 * When a request is sent which uses a non-existant map, the
 * SimpleBalancer should send an acknowledge telling the source
 * of the request this, and not send the request further.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerNonExistantMap ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   // request info about an item in map 666 which doesn't exist
   fix.useMap( Assert::MAPNOTFOUND,
               666,
               host2 // pretend host2 made the request
               );
}

/**
 * Tests some simple "load balancing" with just a single
 * module which is the leader.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerOneModule ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   // a request for info about something in map 0, which isn't loaded
   fix.useMap( Assert::MAPNOTLOADED,
               0,
               host1, // origin of the request
               host1  // where we expect the load to be sent
               );

   // send the request again, the balancer should assume the module
   // is loading map 0
   fix.useMap( Assert::MAPLOADEDORLOADING,
               0,
               host1, 
               host1
               );
}

/**
 * Test that the balancer updates its own statistics properly on timeout
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerOneModuleOwnStats ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   // pretend map 0 is loaded in this module
   fix.thisModule->loadMap( 0 );

   PacketSendList packetList;
   fix.balancer->timeout( fix.thisModule->createStatisticsPacket(),
                          packetList );
   
   // shouldn't need to send any packets as a consequence of the timeout
   MC2_TEST_CHECK( packetList.size() == 0 );

   // request info about something in map 0, assert that it's loaded
   fix.useMap( Assert::MAPLOADEDORLOADING,
               0,
               host1,
               host1 
               );
}

/**
 * This test sets up several modules with different maps,
 * sends a request which uses one of the maps, and checks
 * so the right module is sent the request.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerSendToModuleWithMap ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   fix.thisModule->loadMaps( "1 4 7" );
   fix.availables.push_back( new TestModule( host2, "2 5 8" ) );
   fix.availables.push_back( new TestModule( host3, "3 6 9" ) );

   // let the balancer know which maps are loaded in each module
   updateStatisticsExpectNoPackets( fix );

   // request info about something in map 5 (which is loaded in host2)
   fix.useMap( Assert::MAPLOADEDORLOADING,
               5,
               host1, // origin of the request
               host2  // expected destination
               );
}

/**
 * A regression test for an old bug.
 * The reactToMapLoaded function incorrectly accumulated memory sizes.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerReactToMapLoadedCounting ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   // the balancer starts with opt mem = 100, max mem = 200 and
   // this module has no maps loaded, now create an available
   // module with the same settings, but a map loaded of size 10

   fix.availables.push_back( new TestModule( host2, 
                                             "10:10" // a map with size 10
                                             ) );

   // now load a map of size 30 in the leader
   fix.thisModule->loadMap( 11, 30 );

   // make sure the balancer knows about the current situation
   updateStatisticsExpectNoPackets( fix );

   // now load 4 maps of size 1, they should all be loaded in the available
   // module since it has so much less memory used
   for ( int i = 0; i < 4; ++i ) {
      auto_ptr<LoadMapRequestPacket> request(
         fix.useMap( Assert::MAPNOTLOADED,
                     i,
                     host1, // origin of the request
                     host2  // expected destination
                     ) );

      // create a reply packet saying the size of the loaded map was 1
      PacketSendList packetList;
      LoadMapReplyPacket loadMapReply( *request.get(), StringTable::OK, 1 );

      fix.balancer->reactToMapLoaded( packetList, &loadMapReply );

      MC2_TEST_CHECK( packetList.size() == 0 );
   }
}

/**
 * Test what happens when trying to load a map when all modules
 * are loading. The balancer should reply with an ack when this happens
 * and then another ack when a map finished loading.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerRefusedDueToLoading ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   // set up an available
   fix.availables.push_back( new TestModule( host2, "1:10 2:12" ) );

   // load a map in the leader
   fix.thisModule->loadMaps( "3:7" );

   // make sure the balancer knows about the current situation
   updateStatisticsExpectNoPackets( fix );

   vector<LoadMapReplyPacket*> replyPackets;
   // make sure all modules are loading
   for ( int i = 0; i < 2; ++i ) {
      auto_ptr<LoadMapRequestPacket> request(
         fix.useMap( Assert::MAPNOTLOADED,
                     i+10,
                     host1, // origin of the request
                     host1, // expected destination
                     false  // don't check destination
                     ) );

      replyPackets.push_back( 
         new LoadMapReplyPacket( *request.get(), StringTable::OK, 1 ) );
   }

   // request another load
   PacketSendList packetList;
   RequestPacket* request =
      new ItemInfoRequestPacket( IDPair_t( 50, 0 ), NULL );
   fix.balancer->getModulePackets( packetList,
                                   request );

   // only an ack packet now since all modules are loading
   MC2_TEST_CHECK( packetList.size() == 1 );
   MC2_TEST_CHECK( packetList.front().second->getSubType() ==
                   Packet::PACKETTYPE_ACKNOWLEDGE );

   // tell the balancer that the maps finished loading
   // on the first reply an ack should be sent, then nothing
   for ( size_t i = 0; i < replyPackets.size(); ++i ) {
      PacketSendList packetList;
      fix.balancer->reactToMapLoaded( packetList, replyPackets[ i ] );

      if ( i == 0 ) {
         MC2_TEST_CHECK( packetList.size() == 1 );
         MC2_TEST_CHECK( packetList.front().second->getSubType() ==
                         Packet::PACKETTYPE_ACKNOWLEDGE );
      } 
      else {
         MC2_TEST_CHECK( packetList.size() == 0 );
      }

      delete replyPackets[ i ];
      clearPacketSendList( packetList );
   }
}

/**
 * Test that the balancer removes modules from which it hasn't
 * received a statistics packet for a long time.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerRemoveDeadModules ) {
   SimpleBalancerTestFixture fix( false /* no maps */ );
   TimeUtility::startTestTime( 0 );

   // set up an available
   fix.availables.push_back( new TestModule( host2, "" ) );

   // make sure the balancer knows about the module
   updateStatisticsExpectNoPackets( fix );

   MC2_TEST_CHECK( fix.balancer->getNbrModules() == 2 ); // itself and host2

   TimeUtility::testSleep( MAX_TIME_NO_STATISTICS+1 );
   PacketSendList packetList;
   fix.balancer->timeout( fix.thisModule->createStatisticsPacket(),
                          packetList );

   MC2_TEST_CHECK( fix.balancer->getNbrModules() == 1 ); // only itself
}

/**
 * This test checks that loading of a new map is done in the module
 * with the oldest map if all modules have roughly the same memory usage.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerSendToModuleWithOldestMap ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   const char* leaderMaps     = "5:2 6:4 7:3 8:3";
   const char* available1Maps = "1:5 2:6";
   const char* available2Maps = "3:10 4:3";

   // make sure all loaded maps get different age
   TimeUtility::startTestTime( 1 );

   // set up two availables with roughly equal memory usage
   fix.availables.push_back( new TestModule( host2, available1Maps ) );
   fix.availables.push_back( new TestModule( host3, available2Maps ) );

   // similar for the leader
   fix.thisModule->loadMaps( leaderMaps );

   // make sure the balancer knows about the modules
   updateStatisticsExpectNoPackets( fix );
   
   // using a new map should cause a load to be sent to host2 since it has
   // the oldest map (was created first)
   fix.useMap( Assert::MAPNOTLOADED,
               9,
               host1, // origin of request
               host2  // expected destination
               );

   // reload the maps in host2 so host3 has the oldest map
   fix.availables.front()->unloadMap(1);
   fix.availables.front()->unloadMap(2);
   fix.availables.front()->loadMaps( available1Maps );

   // do the same thing again
   updateStatisticsExpectNoPackets( fix );

   // now host3 should get the map instead
   fix.useMap( Assert::MAPNOTLOADED,
               9,
               host1, // origin of request
               host3  // expected destination
               );
}

/**
 * This test checks that maps are deleted in age order.
 */
MC2_UNIT_TEST_FUNCTION( simpleBalancerDeleteOrder ) {
   SimpleBalancerTestFixture fix( true /* uses maps */ );

   // make sure everything happens at different time stamps
   TimeUtility::startTestTime( 1 );

   // load 10 maps of size 10 each, filling the leader
   for ( int i = 0; i < 10; ++i ) {
      fix.useMap( Assert::MAPNOTLOADED,
                  i,
                  host1, // origin of request
                  host1  // expected destination
                  );
      fix.thisModule->loadMap( i, 10 );
      updateStatisticsExpectNoPackets( fix );
   }

   // use the maps in reverse order, making map 9 the oldest
   for ( int i = 9; i >= 0; --i ) {
      fix.useMap( Assert::MAPLOADEDORLOADING,
                  i,
                  host1, // origin of request
                  host1  // expected destination
                  );

      // updates the timestamp
      fix.thisModule->useMap( i );
   }

   // load another 10 maps, the old maps should be removed in reverse order
   for ( int i = 10; i < 20; ++i ) {
      fix.useMap( Assert::MAPNOTLOADED,
                  i,
                  host1,
                  host1
                  );
      fix.thisModule->loadMap( i, 10 );

      // the balancer checks the memory usage in timeout()
      PacketSendList packetList;
      fix.balancer->timeout( fix.thisModule->createStatisticsPacket(),
                             packetList );

      MC2_TEST_REQUIRED( packetList.size() == 1 );
      DeleteMapRequestPacket* deletePacket = 
         dynamic_cast<DeleteMapRequestPacket*>( packetList.front().second );
      MC2_TEST_REQUIRED( deletePacket != NULL );
      MC2_TEST_REQUIRED( deletePacket->getMapID() == 
                         static_cast<uint32>( 19-i ) );

      fix.thisModule->unloadMap( deletePacket->getMapID() );
      clearPacketSendList( packetList );
   }
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SIMPLEBALANCERHELPERS_H__
#define __SIMPLEBALANCERHELPERS_H__

#include "config.h"
#include "SimpleBalancer.h"
#include "MapSafeVector.h"
#include "Properties.h"
#include "StatisticsPacket.h"
#include "ItemInfoPacket.h"
#include "MapNotice.h"
#include "IDPairVector.h"
#include "SystemPackets.h"
#include "LoadMapPacket.h"
#include "DeleteHelpers.h"
#include "MC2UnitTest.h"
#include "StringTable.h"

// some module addresses that are used in the tests
const IPnPort host1(1,0);
const IPnPort host2(2,0);
const IPnPort host3(3,0);

/**
 * Loads maps from a string representation into a MapSafeVector.
 *
 * The maps string should be separated by spaces and each map is
 * either just a map id or a map id and a size, no map size means
 * size = 0, e.g:
 *
 * "1 2:3" - load maps 1 and 2, map 2 should have size 3
 */
void loadMapsFromString( MapSafeVector& mapVector,
                         const char* maps );


/** 
 * Represents a module in our tests for SimpleBalancer.
 */
class TestModule {
public:
   TestModule( const IPnPort& address,
               const char* loadedMaps,
               uint64 optMem = 100,
               uint64 maxMem = 200,
               int queueLength = 0 )
         : m_address( address ),
           m_optMem( optMem ),
           m_maxMem( maxMem ),
           m_queueLength( queueLength ) {
      loadMapsFromString( m_mapVector, loadedMaps );
   }

   StatisticsPacket* createStatisticsPacket() const {
      return new StatisticsPacket( m_address,
                                   m_mapVector,
                                   m_optMem,
                                   m_maxMem,
                                   m_queueLength,
                                   0 );
   }

   void loadMap( uint32 mapID, int size = 0 ) {
      m_mapVector.finishLoadingMap( mapID, size );
   }

   void loadMaps( const char* maps ) {
      loadMapsFromString( m_mapVector, maps );
   }

   void unloadMap( uint32 mapID ) {
      m_mapVector.removeMap( mapID );
   }

   void useMap( uint32 mapID ) {
      m_mapVector.updateLastUse( mapID );
   }

   StatisticsPacket* createStatisticsPacket() {
      return new StatisticsPacket( m_address,
                                   m_mapVector,
                                   m_optMem,
                                   m_maxMem,
                                   m_queueLength,
                                   0 );
                                   
   }

private:
   IPnPort m_address;          ///< The module's address
   MapSafeVector m_mapVector;  ///< The maps in the module
   uint64 m_optMem;            ///< Optimal memory amount
   uint64 m_maxMem;            ///< Maximum memory amount
   int m_queueLength;
};

namespace Assert {

/** 
 * The useMap function works differently depending on the expected
 * status of the map.
 */
enum MapExpectation {
   MAPNOTFOUND,
   MAPNOTLOADED,
   MAPLOADEDORLOADING
};

}

/// Deallocates the packets in the list and makes the list empty
void clearPacketSendList( PacketSendList& packetList );

/**
 * The test fixture for the SimpleBalancer tests.
 */
struct SimpleBalancerTestFixture {

   SimpleBalancerTestFixture( bool usesMaps ) {
      setupStandardProperties();
      thisModule = new TestModule( host1, "", 100, 200, 0 );
                               
      balancer = new SimpleBalancer( host1,
                                     "Mock", 
                                     thisModule->createStatisticsPacket(), 
                                     usesMaps );

      // all tests start with the balancer's module as leader
      balancer->becomeLeader();
      set<MapID> allMaps;
      for ( int i = 0; i < 100; ++i ) {
         allMaps.insert( i );
      }

      balancer->setAllMaps( allMaps );
   }

   ~SimpleBalancerTestFixture() {
      delete thisModule;
      delete balancer;

      STLUtility::deleteValues( availables );
   }

   void setupStandardProperties() {
      Properties::setPropertyFileName( "/dev/null" );
   }

   /**
    * Simulates that someone wants to use a map, and checks various
    * things depending on whether we expect the map to exist, be loaded
    * etc.
    */
   auto_ptr<LoadMapRequestPacket>
   useMap( Assert::MapExpectation exp,
           uint32 mapID, 
           const IPnPort& originHost,
           const IPnPort& destHost = IPnPort(),
           bool testDest = true ) {
      auto_ptr<LoadMapRequestPacket> result;

      // send a request which uses the map to the balancer
      RequestPacket* request =
         new ItemInfoRequestPacket( IDPair_t( mapID, 0 ), NULL );
      request->setOriginAddr( originHost );
      PacketSendList packetList;
      balancer->getModulePackets( packetList, request );

      // check the resulting packet list
      if ( exp == Assert::MAPNOTFOUND ) {
         // packetList should now only contain the acknowledge packet
         MC2_TEST_CHECK( packetList.size() == 1 );
         MC2_TEST_CHECK( packetList.front().first == originHost );
         AcknowledgeRequestReplyPacket* ackPack =
            dynamic_cast<AcknowledgeRequestReplyPacket*>( 
               packetList.front().second );
         MC2_TEST_CHECK( ackPack != NULL );
         MC2_TEST_CHECK( ackPack->getStatus() == StringTable::MAPNOTFOUND );
      } 
      else if ( exp == Assert::MAPNOTLOADED ) {
         // packetList should now contain one packet to load the map, followed
         // by our original request
         MC2_TEST_CHECK( packetList.size() == 2 );
         LoadMapRequestPacket* loadRequest =
            dynamic_cast<LoadMapRequestPacket*>( packetList.front().second );
         MC2_TEST_CHECK( loadRequest != 0 );
         result.reset( loadRequest );
         if ( testDest ) {
            // check that both go to dest
            MC2_TEST_CHECK( packetList.front().first == destHost );
            MC2_TEST_CHECK( packetList.back().first == destHost );
         }
         // check that our request is on the list
         MC2_TEST_CHECK( packetList.back().second == request );
         // remove the load request so it wont be deleted below
         packetList.erase( packetList.begin() );
      } 
      else if ( exp == Assert::MAPLOADEDORLOADING ) {
         // packetList should now contain only the request
         MC2_TEST_CHECK( packetList.size() == 1 );
         if ( testDest ) {
            // check that it's sent to dest
            MC2_TEST_CHECK( packetList.front().first == destHost );
         }
         // is it the packet we sent?
         MC2_TEST_CHECK( packetList.front().second == request );
      } 
      else {
         // invalid expectation
         MC2_ASSERT( false );
      }

      clearPacketSendList( packetList );
      return result;
   }

   TestModule* thisModule;
   vector<TestModule*> availables;

   /** 
    * The balancer that is being tested, it is supposed to be the balancer
    * for thisModule.
    */
   SimpleBalancer *balancer;
};

/**
 * Makes sure the balancer knows about its own module's statistics
 * as well as all the other modules' statistics.
 */
void updateStatisticsExpectNoPackets( SimpleBalancerTestFixture& fix );

/// Creates a pretty much empty statistics packet from a certain source
StatisticsPacket* simpleStatisticsPacketNoMaps( const IPnPort& source);

#endif // __SIMPLEBALANCERHELPERS_H__

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// Tests for JobThread, JobCourier, JobDispatcher
// This test will make sure single and multithreaded
// dispatcher works.
//

#include "SingleJobDispatcher.h"

#include "Processor.h"
#include "MapSafeVector.h"
#include "PacketCache.h"
#include "PacketSender.h"
#include "Packet.h"
#include "IPnPort.h"
#include "PacketQueue.h"
#include "QueuedPacketSender.h"
#include "JobThread.h"
#include "NetPacket.h"
#include "NetUtility.h"
#include "StringTable.h"
#include "Properties.h"
#include "TestPackets.h"
#include "ProcessorFactory.h"
#include "MultiJobDispatcher.h"
#include "ThreadedJobCourier.h"
#include "FIFOJobScheduler.h"
#include "STLUtility.h"

#include "MC2UnitTestMain.h"

#include <unistd.h>

/// A log handler that counts how many times logging is done
class TestLogHandler: public LogHandler {
public:
   TestLogHandler() {
      MC2Logging::getInstance()
         .addHandler( ( MC2Logging::LOGLEVEL_DEBUG |
                        MC2Logging::LOGLEVEL_INFO  |
                        MC2Logging::LOGLEVEL_WARN  |
                        MC2Logging::LOGLEVEL_ERROR |
                        MC2Logging::LOGLEVEL_FATAL ), this );
   }
   virtual ~TestLogHandler() {
   }
   void handleMessage(int level, const char* msg, int msgLen,
                      const char* levelStr,
                      const char* timeStamp ) {
      if ( msg ) {
         ISABSync sync( m_logMutex );
         m_messages.push_back( msg );
      }
   }

   uint32 getNbrLogs() const {
      return m_messages.size();
   }

   /// @return all the messages passed through this instance.
   const vector<MC2String>& getMessages() const {
      return m_messages;
   }
   /// reset number of messages.
   void clearMessages() {
      m_messages.clear();
   }

private:
   vector<MC2String> m_messages;
   ISABMutex m_logMutex;
};

/**
 * A simple test processor for handling packets and returning
 * dummy packets.
 */
class TestProcessor: public Processor {
public:
   TestProcessor( MapSafeVector* vec );
   ~TestProcessor();

   int getCurrentStatus() {
      return 1;
   }
   uint32 getNbrProcessedPackets() const {
      return m_nbrProcessed;
   }
   /// The total number of processed packets of all instances of this.
   static uint32 m_totalNbrProcessed;
protected:
   /// @copydoc Processor::handleRequestPacket(RequestPacket, char)
   Packet* handleRequestPacket( const RequestPacket& pack,
                                char* packetInfo );

private:
   /// Base class processor needs this, but does not own it.
   MapSafeVector* m_vec;
   /// Number of packets that gone through this processor.
   uint32 m_nbrProcessed;
};

uint32 TestProcessor::m_totalNbrProcessed = 0;

// Lock for counting number of real processed packets of m_totalNbrProcessed
ISABMutex g_processorMutex;

TestProcessor::TestProcessor( MapSafeVector* vec ):
   Processor( vec ),
   m_vec( vec ),
   m_nbrProcessed( 0 ) {
}

TestProcessor::~TestProcessor() {

}

Packet* TestProcessor::
handleRequestPacket( const RequestPacket& pack,
                     char* packetInfo ) {
   { // count total number of processed packets
      ISABSync sync( g_processorMutex );
      m_totalNbrProcessed++;
   }
   m_nbrProcessed++;
   // Return any packet...
   ReplyPacket* reply =
      new ReplyPacket( 1024,
                       Packet::PACKETTYPE_GFXFEATUREMAPREPLY,
                       &pack,
                       StringTable::OK );

   return reply;
}


struct MocPacketSender: public PacketSender {
   MocPacketSender();

   bool sendPacket( Packet* pack,
                    const IPnPort& destination );
   uint32 m_nbrSent;
};

MocPacketSender::MocPacketSender():
   m_nbrSent( 0 ) {
}

bool MocPacketSender::
sendPacket( Packet* pack, const IPnPort& destination ) {
   delete pack;
   m_nbrSent++;
   return true;
}

MC2_UNIT_TEST_FUNCTION( dispatchTest ) {
   Properties::setPropertyFileName( "/dev/null" );
   // Disable cache
   Properties::insertProperty( "PACKET_CACHE_MAX_AGE_SEC", "0" );
   Properties::insertProperty( "PACKET_CACHE_MAX_SIZE_BYTES", "0" );

   PacketCache cache;
   MocPacketSender sender;
   MapSafeVector msVect;
   TestProcessor proc( &msVect );
   PacketQueue receiveQueue;

   SingleJobDispatcher dispatcher( cache, sender, proc, receiveQueue );
   RequestPacket* pack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                         0x7F, 0xAB, 1 ); // packetID, requestID, mapID
   // a must have if we want a valid packet time in the log
   pack->setArrivalTime();

   // process the same package twice, thus the first is a clone
   dispatcher.dispatch( pack->getClone() );
   dispatcher.dispatch( pack );

   MC2_TEST_CHECK( sender.m_nbrSent == 2 );
   MC2_TEST_CHECK( proc.getNbrProcessedPackets() == 2 );

   Properties::Destroy();
}

class AddPacketInfoProcessor: public Processor {
public:
   AddPacketInfoProcessor( MapSafeVector* vec ):
      Processor( vec ) {
   }

   /// Set a specific message to be set in the next
   /// round of handleRequestPacket.
   void setMessage( const MC2String& msg ) {
      m_msg = msg;
   }

   int getCurrentStatus() {
      return 1;
   }

protected:

   /// @copydoc Processor::handleRequestPacket(RequestPacket, char)
   Packet* handleRequestPacket( const RequestPacket& pack,
                                char* packetInfo ) {
      sprintf( packetInfo, "[%s]", m_msg.c_str() );
      return NULL;
   }
private:
   MC2String m_msg;

   MapSafeVector* m_vec;
};

/*
 * This test whether the logging works as it should for adding
 * messages through packetInfo.
 */
MC2_UNIT_TEST_FUNCTION( packetInfoTest ) {
   Properties::setPropertyFileName( "/dev/null" );
   // Disable cache
   Properties::insertProperty( "PACKET_CACHE_MAX_AGE_SEC", "0" );
   Properties::insertProperty( "PACKET_CACHE_MAX_SIZE_BYTES", "0" );

   PacketCache cache;
   MocPacketSender sender;
   MapSafeVector msVect;
   AddPacketInfoProcessor proc( &msVect );
   PacketQueue receiveQueue;
   TestLogHandler* logger = new TestLogHandler();

   SingleJobDispatcher dispatcher( cache, sender, proc, receiveQueue );
   RequestPacket* pack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                         0x7F, 0xAB, 1 ); // packetID, requestID, mapID
   pack->setArrivalTime();
   logger->clearMessages();
   proc.setMessage( "special test0" );
   dispatcher.dispatch( pack->getClone() );
   MC2_TEST_REQUIRED( logger->getNbrLogs() == 1 );
   MC2_TEST_CHECK( STLUtility::has( logger->getMessages()[0],
                                    "special test0" ) );
   proc.setMessage( "special msg" );
   dispatcher.dispatch( pack );
   MC2_TEST_REQUIRED( logger->getNbrLogs() == 2 );
   MC2_TEST_CHECK( STLUtility::has( logger->getMessages()[1],
                                    "special msg" ) );

   Properties::Destroy();
}


/**
 * Adds packages to queue and returns number of packages that should be
 * processed and replied. i.e it ignores packages such as "shutdown".
 */
uint32 addPackets( PacketQueue& receive ) {

   // add two packets, one normal and then a shutdown packet so
   // the job thread dies

   RequestPacket* genericPack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                         0x7F, 0xAB, 1 ); // packetID, requestID, mapID
   genericPack->setOriginAddr( IPnPort( NetUtility::getLocalIP(), 12345 ) );
   genericPack->setArrivalTime();
   receive.enqueue( genericPack );

   uint32 returnSize = receive.getStatistics();

   // Now add packages that are not suppose to be in the reply queue
   receive.enqueue( createShutdownPacket() );

   return returnSize;
}

MC2_UNIT_TEST_FUNCTION( jobThreadTest ) {
   Properties::setPropertyFileName( "/dev/null" );

   // Test job thread with the single dispatcher
   // It should handle two packages, a normal and a shutdown package.

   ISABThreadInitialize initThreads;

   PacketCache cache;
   MocPacketSender sender;
   MapSafeVector msVect;
   TestProcessor proc( &msVect );
   PacketQueue receiveQueue;
   QueuedPacketSender::SendQueue sendQueue;

   uint32 numPackages = addPackets( receiveQueue );

   JobThread* worker = new JobThread( &proc,
                                      &receiveQueue,
                                      sendQueue );

   worker->run();
   // should have sent one package, the shutdown package must not be
   // in the package send queue
   MC2_TEST_REQUIRED( sendQueue.getSize() == numPackages );

   // wait for job thread to shutdown
   worker->join();

   Properties::Destroy();
}

MC2_UNIT_TEST_FUNCTION( dispatchDirectTest ) {
   Properties::setPropertyFileName( "/dev/null" );

   // Test dispatchDirect so it does not do logging!

   ISABThreadInitialize initThreads;

   PacketCache cache;
   MocPacketSender sender;
   MapSafeVector msVect;
   TestProcessor proc( &msVect );
   PacketQueue receiveQueue;
   QueuedPacketSender::SendQueue sendQueue;

   receiveQueue.enqueue( createDirectDispatchPacket() );
   receiveQueue.enqueue( createDirectDispatchPacket() );
   receiveQueue.enqueue( createShutdownPacket() );

   TestLogHandler* logger = new TestLogHandler();

   JobThread* worker = new JobThread( &proc,
                                      &receiveQueue,
                                      sendQueue );
   worker->run();

   // Must have empty log, 3 logs is ok, must log shutdown packet.
   MC2_TEST_CHECK_EXT( logger->getNbrLogs() <= 3,
                       boost::lexical_cast< MC2String >( logger->getNbrLogs() )
                       + "# logs. Dispatch direct does logging!" );
   // and no packet processed!
   MC2_TEST_CHECK_EXT( proc.getNbrProcessedPackets() == 2,
                       boost::lexical_cast< MC2String >
                       ( proc.getNbrProcessedPackets() ) );


   worker->join();

   Properties::Destroy();
}

// Test cache
MC2_UNIT_TEST_FUNCTION( cacheTest ) {
   Properties::setPropertyFileName( "/dev/null" );

   Properties::insertProperty( "PACKET_CACHE_MAX_AGE_SEC", "300" );
   Properties::insertProperty( "PACKET_CACHE_MAX_SIZE_BYTES", "20000000" );
   PacketCache cache;
   MocPacketSender sender;
   MapSafeVector msVect;
   TestProcessor proc( &msVect );
   PacketQueue receiveQueue;
   QueuedPacketSender::SendQueue sendQueue;

   SingleJobDispatcher dispatcher( cache, sender, proc, receiveQueue );

   dispatcher.dispatch( createCachablePacket( 1 ) );
   MC2_TEST_CHECK_EXT( cache.getTotalNbrOfPackets() == 1, "Missing cache!" );

   dispatcher.dispatch( createCachablePacket( 2 ) );
   MC2_TEST_CHECK_EXT( cache.getTotalNbrOfPackets() == 2, "Missing cache!" );

   dispatcher.dispatch( createCachablePacket( 1 ) );
   MC2_TEST_CHECK_EXT( cache.getTotalNbrOfPackets() == 2,
                       "Cache is not re-using old cached element!" );

   // Test dispatch direct packet
   dispatcher.dispatchDirect( createDirectDispatchPacket() );
   dispatcher.dispatch( createDirectDispatchPacket() );
   // No cache for this packet
   MC2_TEST_CHECK_EXT( cache.getTotalNbrOfPackets() == 2,
                       "Packets that are dispatched directly should not be "
                       "cachable!" );

   Properties::Destroy();
}

struct TestProcessorFactory: public ProcessorFactory {
   TestProcessorFactory():m_vec( new MapSafeVector() ) {
   }

   virtual ~TestProcessorFactory() {
      delete m_vec;
   }

   Processor* create() {
      return new TestProcessor( m_vec );
   }

   MapSafeVector* m_vec;
};
/**
 * Operator for determining if a string has a specific substring.
 */
struct HaveSubString {
   HaveSubString( const MC2String& subStr ):
      m_subStr( subStr ) {
   }

   /// @return true if str has m_subStr inside it.
   bool operator()( const MC2String& str ) {
      return str.find( m_subStr ) != MC2String::npos;
   }

   /// The substring to search for.
   MC2String m_subStr;
};

/**
 * Test multi dispatch.
 * This will test multiple packets sent to the dispatcher,
 * and the dispatcher should return directly or wait if there is
 * no courier available to process the packet.
 * It will also test the logging of cached packets.
 */
MC2_UNIT_TEST_FUNCTION( multiDispatchTest ) {
   Properties::setPropertyFileName( "/dev/null" );

   Properties::insertProperty( "PACKET_CACHE_MAX_AGE_SEC", "300" );
   Properties::insertProperty( "PACKET_CACHE_MAX_SIZE_BYTES", "20000000" );

   ISABThreadInitialize initThreads;

   TestProcessor::m_totalNbrProcessed = 0;

   PacketCache cache;
   MocPacketSender sender;
   TestProcessorFactory factory;
   PacketQueue receiveQueue;
   const uint32 MAX_NUMBER_OF_THREADS = 5;
   MultiJobDispatcher* dispatcher =
      new MultiJobDispatcher( cache, sender, factory,
                              MAX_NUMBER_OF_THREADS,
                              new FIFOJobScheduler( MAX_NUMBER_OF_THREADS ),
                              receiveQueue );
   sleep( 2 ); // wait for threads to start

   TestLogHandler* logger = new TestLogHandler();
   RequestPacket* pack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                         0x7F, 0xAB, 1 ); // packetID, requestID, mapID
   // a must have if we want a valid packet time in the log
   pack->setArrivalTime();

   //
   // Note: This is not a fully covering test. The test actually needs
   // to be run in a while loop for a long long time.
   //
   const uint32 NBR_PACKETS = 100;
   // process the same package more than once
   for ( uint32 i = 0; i < NBR_PACKETS - 1; ++i ) {
      dispatcher->dispatch( pack->getClone() );
   }

   dispatcher->dispatch( pack );


   // This will wait until all threads are dead.
   delete dispatcher;
   MC2_TEST_CHECK_EXT( sender.m_nbrSent == NBR_PACKETS,
                       sender.m_nbrSent );

   // count number of messages that contained "cache"
   MC2_TEST_CHECK_EXT( count_if( logger->getMessages().begin(),
                                 logger->getMessages().end(),
                                 HaveSubString( "(cached)" ) ) ==
                       (int)(NBR_PACKETS - TestProcessor::m_totalNbrProcessed),
                       "The number of cached packets does not match the number"
                       " of logs that contains \"(cached)\"" );

   // reset processed packets
   TestProcessor::m_totalNbrProcessed = 0;

   Properties::Destroy();
}

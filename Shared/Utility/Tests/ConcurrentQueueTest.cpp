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

#include "ConcurrentQueue.h"
#include "DebugClock.h"
#include "ISABThread.h"

class Feed: public ISABThread {
public:
   Feed( ConcurrentQueue<int>& values,
         uint32 maxValues,
         uint32 index ):
      m_values( values ),
      m_maxValues( maxValues ),
      m_index( index ) {

   }
   void run();

private:
   ConcurrentQueue< int >& m_values;
   const uint32 m_maxValues;
   const uint32 m_index;
};

class Eat: public ISABThread {
public:
   Eat( ConcurrentQueue< int >& values, uint32 index ):
      m_values( values ),
      m_index( index ) {
   }

   void run();
private:
   ConcurrentQueue< int >& m_values;
   uint32 m_index;
};

void Feed::run() {
   for ( uint32 i = 0; i < m_maxValues; ++i ) {
      m_values.push( 10 );
   }
   terminate();
   mc2dbg << ">>> Feeder(" << m_index << ") is done." << endl;
}

void Eat::run() {
   while ( ! terminated ) {
      int value = 0;
      m_values.waitAndPop( value );
      if ( value == 666 ) {
         terminate();
         break;
      } else {
         MC2_ASSERT( value == 10 );
      }
   }
   mc2dbg << "consumer(" << m_index << ") done." << endl;
}

MC2_UNIT_TEST_FUNCTION( concurrentQueueTest ) {
   DebugClock clock;
   ISABThreadInitialize initThreads;
   ConcurrentQueue< int > queue;
   const uint32 MAX_FEEDERS = 10;
   vector<ISABThreadHandle> feeders;
   vector<ISABThreadHandle> consumers;

   // create feeders and consumers
   for ( uint32 i = 0; i < MAX_FEEDERS; ++i ) {
      feeders.push_back( new Feed( queue, 1000, i ) );
      consumers.push_back( new Eat( queue, i ) );
      // start the consumers directly
      consumers.back()->start();
   }
   // start feeders
   for ( uint32 i = 0; i < MAX_FEEDERS; ++i ) {
      feeders[ i ]->start();
   }

   // Wait for all feeders
   mc2dbg << "Waiting for feeders." << endl;
   for ( uint32 i = 0; i < MAX_FEEDERS; ++i ) {
      feeders[ i ]->join();
      mc2dbg << "--------- Done Waiting for feeder(" << i << ")" << endl;
   }

   mc2dbg << "Waiting for consumers." << endl;
   // send stop signal through the queue, one for
   // each consumer.
   for ( uint32 i = 0; i < MAX_FEEDERS; ++i ) {
      mc2dbg << "Adding shutdown." << endl;
      queue.push( 666 );
   }

   initThreads.waitTermination();

   mc2dbg << "Took: " << clock << endl;
}

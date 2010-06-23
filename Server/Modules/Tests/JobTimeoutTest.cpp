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

#include "StandardReader.h"
#include "JobTimeoutHandler.h"
#include "NetPacketQueue.h"
#include "PacketQueue.h"
#include "MapSafeVector.h"
#include "PacketReaderThread.h"
#include "ModulePacketSenderReceiver.h"
#include "NetPacket.h"
#include "multicast.h"

//
// Tests the timeout functionality for Reader.
// The JobThread has a maximum predefined time slot,
// when it uses more than the Reader has to take action.
// The resons for JobThread been working too long can be
// many, but mostly it hangs or waiting for sockets forever.
//

/// counts timeouts
class CountTimeouts: public JobTimeoutHandler {
public:
   CountTimeouts( StandardReader& reader ):
      m_reader( reader ) {
   }

   void timeout();

private:
   StandardReader& m_reader;
};

void CountTimeouts::timeout() {
   // just for the testing record :)
   MC2_TEST_CHECK( true );
   m_reader.terminate();
}

MC2_UNIT_TEST_FUNCTION( timeoutTest ) {

   ISABThreadInitialize initThreads;
   // use an unused module type
   moduletype_t moduleType = MODULE_TYPE_INVALID;
   PacketQueue jobQueue;
   NetPacketQueue sendQueue;
   MapSafeVector loadedMaps;
   // setup a fake sender receiver, but do not start it
   ModulePacketSenderReceiver
      senderReceiver( 3025,
                      IPnPort( MultiCastProperties::getNumericIP( moduleType, true ),
                               MultiCastProperties::getPort( moduleType, true ) ),
                      IPnPort( MultiCastProperties::getNumericIP( moduleType, false ),
                               MultiCastProperties::getPort( moduleType, false ) ) );

   PacketReaderThread packetReader( senderReceiver, false );

   // setup a dummy reader
   StandardReader* reader =
      new StandardReader( moduleType,
                          &jobQueue, sendQueue,
                          &loadedMaps,
                          packetReader,
                          3024, // listen port
                          0, // rank
                          false ); // dont use maps
   ISABThreadHandle readerHandle = reader;

   CountTimeouts *count = new CountTimeouts( *reader );
   reader->setJobTimeoutHandler( count );

   reader->start();
   // fake that the job thread is working
   loadedMaps.setJobThreadStart();
   TimeUtility::startTestTime( 6*1000 );
   sleep( 2 );

   initThreads.waitTermination();
}


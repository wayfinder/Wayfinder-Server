/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "CommunicationModule.h"
#include "CommunicationReader.h"
#include "CommunicationProcessor.h"
#include "ModulePacketSenderReceiver.h"
#include "PacketQueue.h" // For m_senderReceiver->getSendQueue()
#include "PropertyHelper.h"
#include "JobThread.h"
#include "PacketReaderThread.h"


CommunicationModule::CommunicationModule( int argc, char* argv[] )
   : Module( MODULE_TYPE_COMMUNICATION,
             argc, argv, false )
{
}

CommunicationModule::~CommunicationModule()
{
}

int
CommunicationModule::handleCommand( const char* input ) {      
   // no own commands right now, add later
   return Module::handleCommand(input);
}

void
CommunicationModule::init() {
   m_senderReceiver.
      reset( new ModulePacketSenderReceiver( m_preferredPort,
                                             IPnPort( m_leaderIP,
                                                      m_leaderPort ),
                                             IPnPort( m_availIP, 
                                                      m_availPort) ) );

   m_packetReader.reset( new PacketReaderThread(
       static_cast<ModulePacketSenderReceiver&>(*m_senderReceiver )  ) );

   m_queue.reset( new PacketQueue );
   m_processor.reset( new CommunicationProcessor( m_loadedMaps.get() ) );
   m_jobThread = new JobThread( m_processor.get(), m_queue.get(),
                                m_senderReceiver->getSendQueue() );
   m_reader = new CommunicationReader( 
      m_queue.get(),
      m_senderReceiver->getSendQueue(),
      m_loadedMaps.get(),
      m_packetReader.get(),
      m_senderReceiver->getPort(),
      m_rank );
   mc2dbg2 << "CommunicationModule::init() done, going to Module::init()" 
           << endl;

   Module::init();
}



int main( int argc, char *argv[] )
try {
   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;

   CommunicationModule* CM = new CommunicationModule( argc, argv );

   CM->parseCommandLine();

   CM->init();
   CM->start();
   CM->gotoWork( init );

   return 0;
} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initialize CommunicationModule. "
          << ex.what() << endl;
   return 1;
}


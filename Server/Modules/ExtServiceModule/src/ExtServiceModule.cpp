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

#include "ExtServiceModule.h"
#include "ExtServiceProcessor.h"
#include "ExtServiceReader.h"

#include "SearchMatch.h"

#include "ExtServices.h"
#include "SearchRequestParameters.h"
#include "ExternalSearchRequestData.h"

#include "ExternalSearchDesc.h"
#include "ExternalSearchDescGenerator.h"
#include "LangTypes.h"
#include "Properties.h"
#include "XMLInit.h"
#include "PropertyHelper.h"

#include "PacketQueue.h"

#include "PacketSenderReceiver.h"
#include "JobThread.h"
#include "PacketReaderThread.h"
#include "ModulePacketSenderReceiver.h"
#include "ExtServiceProcessorFactory.h"

#include "ProviderPrioScheduler.h"

#include <map>


ExtServiceModule::ExtServiceModule( int argc, char* argv[] ):
   Module( MODULE_TYPE_EXTSERVICE,
           argc, argv, false ),
   m_procFactory( new ExtProcessorFactory( &getLoadedMaps() ) )
{
   
}

ExtServiceModule::~ExtServiceModule() {
}

int
ExtServiceModule::handleCommand( const char* input ) {   
   // This is from Component

   if ( strcmp( input, "shutdown" ) == 0 ) {
      RequestPacket* shutdownPack =
         new RequestPacket( 1024,
                            0, // prio
                            Packet::PACKETTYPE_SHUTDOWN,
                            0xBE, 0xEF, 1 ); // packetID, requestID, mapID
      shutdownPack->setArrivalTime();
      m_queue->enqueue( shutdownPack );
      // This will make sure the threads in processor factory dies before the
      // module starts to wait for unfinished thread.
      m_procFactory.reset( 0 );
   }
   
   // no own commands right now, add later
   return Module::handleCommand(input);
}


void
ExtServiceModule::init() {
  m_senderReceiver.
      reset( new ModulePacketSenderReceiver( m_preferredPort,
                                             IPnPort( m_leaderIP,
                                                      m_leaderPort ),
                                             IPnPort( m_availIP, 
                                                      m_availPort) ) );

   m_packetReader.reset( new PacketReaderThread(
       static_cast<ModulePacketSenderReceiver&>(*m_senderReceiver )  ) );

   m_queue.reset( new PacketQueue );
   //
   // Default is to use 3 processor at the same time.
   //
   // This is if two processors are used with same provider but with one using
   // the yellowpages and the other using whitepages, and if the provider is
   // currently having connection problems, then both of the threads will block
   // and no other provider can be used, thus the extra processor.
   //
   // Although, if all threads are using the same provider....then we have to
   // design a scheduler for the job dispatcher to distribute the jobs better.
   //
   uint32 nbrProcessors = Properties::
      getUint32Property( "EXTSERVICES_NUMBER_PROCESSORS", 3 );

   m_jobThread = new JobThread( *m_procFactory, nbrProcessors,
                                new ProviderPrioScheduler(),
                                m_queue.get(),
                                m_senderReceiver->getSendQueue() );

   m_reader = new ExtServiceReader( m_queue.get(),
                                    m_senderReceiver->getSendQueue(),
                                    m_loadedMaps.get(), m_packetReader.get(),
                                    m_senderReceiver->getPort(),
                                    m_rank );

   mc2dbg2 << "ExtServiceModule::init() done, going to Module::init()" << endl;

   Module::init();
}

int main(int argc, char *argv[]) try {
   // setup thread, xml and property system
   ISABThreadInitialize init;
   XMLTool::XMLInit xmlInit;
   PropertyHelper::PropertyInit propInit;

   // initialize the module
   ExtServiceModule* EM = new ExtServiceModule( argc, argv );
   JTCThreadHandle handle = EM;
   // put the module to work
   EM->parseCommandLine();
   EM->init();
   EM->start();
   EM->gotoWork(init);
   
   return 0;

} catch ( const XMLException& err ) {
   cerr << "Error during Xerces-c Initialization.\n"
        << "  Exception message:"
        << err.getMessage() << endl; 
   return 1;
} catch ( const ComponentException& err ) {
   cerr << err.what() << endl;
   return 2;
}


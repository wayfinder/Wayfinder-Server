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

#include "PacketReaderThread.h"
#include "RouteReader.h"
#include "RouteProcessor.h"
#include "multicast.h"
#include "TCPSocket.h"

#include "ISABThread.h"
#include "CommandlineOptionHandler.h"
#include "Properties.h"
#include "JobThread.h"
#include "DataBuffer.h"
#include "Packet.h"
#include "ModulePacketSenderReceiver.h"

#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "MapSafeVector.h"
#include "PreLoadMapModule.h"
#include "PropertyHelper.h"

#include <memory>
#include <iostream>
#include <vector>
#include <signal.h>

class RouteModule: public PreLoadMapModule {
public:
   RouteModule( CommandlineOptionHandler* handler ):
      PreLoadMapModule( MODULE_TYPE_ROUTE, handler )
   { }

   void createProcessor() {
      if ( m_processor.get() != NULL ) {
         return;
      }

      m_processor.reset( new RouteProcessor( m_loadedMaps.get() ) );
      m_senderReceiver.
         reset( new ModulePacketSenderReceiver( m_preferredPort,
                                                IPnPort( m_leaderIP,
                                                         m_leaderPort ),
                                                IPnPort( m_availIP, 
                                                         m_availPort) ) );
      m_packetReader.
         reset( new PacketReaderThread( static_cast<ModulePacketSenderReceiver&>
                                        (*m_senderReceiver ) ) );
      m_queue.reset( new PacketQueue() );

      m_jobThread = new JobThread( m_processor.get(),
                                   m_queue.get(),
                                   m_senderReceiver->getSendQueue() );
   }

   void init() {

      createProcessor();
                                   
      char filename[256];
      sprintf( filename, "route%d.log", rand() );

      m_reader = new RouteReader( m_queue.get(),
                                  m_senderReceiver->getSendQueue(),
                                  m_loadedMaps.get(),
                                  m_packetReader.get(),
                                  m_senderReceiver->getPort(),
                                  m_rank, filename,
                                  false, // not superLeader
                                  NULL, false );  // no push service

      Module::init();
   }

};

void profile();

int main(int argc, char *argv[])
try {
   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;


   bool   profileCalcRoute = false;

   uint32 mapToLoad = MAX_UINT32;


   auto_ptr<CommandlineOptionHandler> 
      coh( new CommandlineOptionHandler( argc, argv ) );
       
   coh->addOption("-c", "--profile-calcroute",
                  CommandlineOptionHandler::presentVal,
                  1, &profileCalcRoute, "F",
                  "If present the processor will be run in the main "
                  "thread and read packets of the format 4 bytes len, "
                  "len bytes packet from stdin");

   char defVal[10];
   sprintf(defVal, "%u", MAX_UINT32);
   coh->addOption("-m", "--loadmap",
                  CommandlineOptionHandler::uint32Val,
                  1, &mapToLoad, defVal,
                  "Load a map and delete it again and exit");
      
   RouteModule* routeModule = new RouteModule( coh.release() );
   ISABThreadHandle routeHandle = routeModule;

   routeModule->parseCommandLine();

   if ( profileCalcRoute ) {
      profile();
   }
   
   if ( mapToLoad != MAX_UINT32 ) {
      char packInfo[Processor::c_maxPackInfo];
      MapSafeVector loadedMaps;      

      RouteProcessor processor( &loadedMaps );
      LoadMapRequestPacket* lmap = new LoadMapRequestPacket(mapToLoad);     
      DeleteMapRequestPacket* dmap = new DeleteMapRequestPacket(mapToLoad);
      auto_ptr<ReplyPacket> lmapRepl( (ReplyPacket*)
                                      processor.handleRequest(lmap, packInfo));
      if ( lmapRepl->getSubType() != Packet::PACKETTYPE_LOADMAPREPLY ||
           lmapRepl->getStatus() != StringTable::OK ) {
         return 1;
      }
      mc2dbg << "Memory: " << SysUtility::getMemoryInfo() << endl;
      // delete 
      processor.handleRequest(dmap, packInfo);
      Properties::Destroy();

      exit(0);
   }

   routeModule->init();
   routeModule->start();
   routeModule->gotoWork( init );

   return 0;

} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initialize RouteModule. " 
          << ex.what() << endl;
   return 1;
}



void profile() {
   MapSafeVector loadedMaps;
   RouteProcessor processor(&loadedMaps);
   while(!feof(stdin)) {
      char packinfo[Processor::c_maxPackInfo];
      DataBuffer buf(4);
      if ( fread( buf.getBufferAddress(), 4, 1, stdin) != 1 )
         exit(0);
      uint32 packSize = buf.readNextLong();
      RequestPacket* packet = new RequestPacket(packSize+4);
      if ( fread ( packet->getBuf(), packSize, 1, stdin ) != 1 )
         exit(0); 
      processor.handleRequest(packet, packinfo);
   }
   exit(0);
}

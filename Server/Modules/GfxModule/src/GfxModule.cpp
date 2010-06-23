/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxProcessor.h"
#include "ISABThread.h"
#include "CommandlineOptionHandler.h"
#include "multicast.h"
#include "ISABThread.h"
#include "MapSafeVector.h"
#include "DataBuffer.h"
#include "Module.h"
#include "Packet.h"
#include "PropertyHelper.h"
#include "XMLInit.h"

#include <iostream>

class GfxModule: public Module {
public:
   GfxModule( CommandlineOptionHandler* handler ):
      Module( MODULE_TYPE_GFX, handler,
              false ) // dont use maps
   {
    
   }

   void parseCommandLine() throw (ComponentException) {
      char* packetFilename = NULL;
      m_cmdlineOptHandler->
         addOption( "-w", "--save-packets",
                    CommandlineOptionHandler::stringVal,
                    1, &packetFilename, "",
                    "File to save all incoming packets in for later profiling");

      Module::parseCommandLine();

      
      m_packetFilename = packetFilename ? packetFilename : "";

   }
   void init() {
      m_processor.reset( new GfxProcessor( m_loadedMaps.get(),
                                           m_packetFilename.c_str()));

      Module::init();
   }

private:
   MC2String m_packetFilename;
};

int main(int argc, char *argv[])
try {
   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;
   XMLTool::XMLInit initXML;

   auto_ptr<CommandlineOptionHandler> 
      coh( new CommandlineOptionHandler( argc, argv ) );

   bool profileProc = false;
   coh->addOption( "-c", "--profile-processor",
                   CommandlineOptionHandler::presentVal,
                   1, &profileProc, "F",
                   "If present the processor will be run in the main "
                   "thread and read packets of the format 4 bytes len, "
                   "len bytes packet from stdin");

   GfxModule* gfxModule = new GfxModule( coh.release() );
   ISABThreadHandle handle = gfxModule;

   gfxModule->parseCommandLine();

   if ( profileProc ) {
      MapSafeVector loadedMaps;
      GfxProcessor processor(&loadedMaps);
      while(!feof(stdin)) {
         char packinfo[Processor::c_maxPackInfo];
         DataBuffer buf(4);
         if ( fread( buf.getBufferAddress(), 4, 1, stdin) != 1 )
            exit(0);
         uint32 packSize = buf.readNextLong();
         RequestPacket* packet = new RequestPacket(packSize+4);
         if ( fread ( packet->getBuf(), packSize, 1, stdin ) != 1 ) {
            delete packet;
            return 0;
         }

         processor.handleRequest(packet, packinfo);
      }

      return 0;
   }

   gfxModule->init();
   gfxModule->start();
   gfxModule->gotoWork( init );

   return 0;

} catch ( const ComponentException& ex ) {
   mc2log << "Could not initialize GfxModule. " << ex.what() << endl;
   return 1;
}


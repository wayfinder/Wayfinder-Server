/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Packet.h"
#include "DatagramSocket.h"
#include "PacketReceiver.h"
#include "multicast.h"
#include "SupervisorStorage.h"
#include "SupervisorDisplay.h"

#include "CommandlineOptionHandler.h"
#include "SupervisorReceiver.h"
#include "RawSupervisorDisplay.h"

#include<iostream>

#include <signal.h>

bool keeprunning = true;

void ctrl_c_handler(int n)
{
   keeprunning = false;
   signal(SIGINT, SIG_DFL);
}

#define ED "\r\n"
int main(int argc, char **argv)
{
   cout << "Supervisor starting ..." << endl;
   CommandlineOptionHandler coh(argc, argv);
   coh.setSummary( "Supervisor shows what is running." ED
                   "Commands to use when running: " ED
                   "  m - Flips show all maps" ED
                   "  w - Flips alert" ED
                   "  c - Flips collate" ED
                   "  k - Add string to uncollate" ED
                   "  u - Remove string from uncollate, special \"clear\"" 
                   "      removes all uncollates" ED
                   "  a - Flips average to sum for collated infos." ED
                   "Property HOSTNAME_LOOKUP determines if the hostname should"
                   " be looked up, " ED
                   "default value is true." ED
                   );

   bool dont_show_loaded_mapsids = false;
   uint32 allowedMapSets = MAX_UINT32;
   uint32 alertQueue = MAX_UINT32;
   uint32 showQueue = 0;
   bool collate = false;
   const char* separator = ":";
   const char* outfile = "/dev/stdout";
   bool rawOutput = false;
   bool stabilize = false;
   // dont_show_loaded_mapsids
   coh.addOption( "-m", "--dont_show_loaded_mapsids",
                  CommandlineOptionHandler::presentVal,
                  1, &dont_show_loaded_mapsids, "F",
                  "Set if not to show loaded mapsIDs." );

   coh.addOption( "-s", "--mapSet",
                  CommandlineOptionHandler::uint32Val,
                  1, &allowedMapSets, "0xFFFFFFFF",
                  "Which map sets to show. Three bits bitfield." );

   coh.addOption( "-w", "--warnqueuesize",
                  CommandlineOptionHandler::uint32Val,
                  1, &alertQueue, "0xFFFFFFFF",
                  "Queue size when to start alering." );

   coh.addOption( "-n", "--showqueuesize",
                  CommandlineOptionHandler::uint32Val,
                  1, &showQueue, "0",
                  "Queue size when to start showing modules." );

   coh.addOption( "-c", "--collate",
                  CommandlineOptionHandler::presentVal,
                  1, &collate, "F",
                  "Set if collate." );
   coh.addOption( "-r", "--raw",
                  CommandlineOptionHandler::presentVal,
                  0, &rawOutput, "F",
                  "Set to raw output" );
   coh.addOption( "-o", "--output",
                  CommandlineOptionHandler::stringVal,
                  1, &outfile, "/dev/stdout",
                  "Output file for raw output." );
   coh.addOption( "", "--separator",
                  CommandlineOptionHandler::stringVal,
                  1, &separator, ":",
                  "Separator for raw output." );
   coh.addOption( "", "--stabilize",
                  CommandlineOptionHandler::presentVal,
                  0, &stabilize, "F",
                  "If to wait at startup until things have stabilized before "
                  "displaying information." );


   if(!coh.parse()) {
      cerr << "Supervisor: Error on commandline (-h for help)" << endl;
      exit(1);
   }
   if(!Properties::setPropertyFileName(coh.getPropertyFileName())) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;
      exit(1);
   }

   // Turn on sigpipe 
   SysUtility::setChangePipeSignal( true );
   SysUtility::setPipeDefault();

   SupervisorReceiver* receiver = new SupervisorReceiver();
   SupervisorStorage* storage = new SupervisorStorage( 
      dont_show_loaded_mapsids );

   SupervisorDisplay* display = NULL;
   if ( rawOutput ) {
      display = new RawSupervisorDisplay( storage, receiver, 
                                             outfile, separator );
   } else {
      display = new TextDisplay( storage, receiver );
   }

   storage->addObserver(display);
   display->setAlertQueueSize( alertQueue );
   display->setShowQueueSize( showQueue );
   display->setCollate( collate );
   display->setStabilize( stabilize );


   int packetCounter = 0;
   Packet* packet = NULL; // new Packet(65535);
   DatagramReceiver* sock = NULL;

   signal(SIGINT, ctrl_c_handler);
   
   while( keeprunning ) {
      // if( packetReceiver->receive(packet, 1000000) ) { <- deprecated
      if ( (packet = receiver->receiveAndCreatePacket( 1000000, sock ) )
           != NULL ) 
      {
         if ( sock != NULL ) {
            storage->handlePacket( packet, sock );
         } else{
            storage->handlePacket(NULL, NULL);
         }
      } else {
         storage->handlePacket(NULL, NULL);
      }
      delete packet;
      if ( packetCounter++ > 10000 ) {
         // For memleak-checks
//           exit(1);
      }
   }

   cout << "Cleaning up" << endl;

   // Clean up
   delete display;
   delete storage;

   // We should delete receivers and packetReceiver too.
   cout << "Supervisor stopping ..." << endl;
   return 0;
}


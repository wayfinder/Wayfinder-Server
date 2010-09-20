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

#include <vector>
#include <stdlib.h>
#include <iostream>
#include "MC2String.h"
#include "sockets.h" // inet_addr
#include "Types.h"
#include "multicast.h"
#include "SearchProcessor.h"
#include "JobThread.h"
#include "ISABThread.h"
#include "PacketQueue.h"
#include "Properties.h"
#include "CommandlineOptionHandler.h"
#include "CoordinateTransformer.h"
#include "PacketReaderThread.h"
#include "SearchReader.h"
#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "StringUtility.h"
#include "TrieStruct.h"
#include "AbbreviationTable.h"
#include "STLStringUtility.h"
#include "ModulePacketSenderReceiver.h"
#include "PreLoadMapModule.h"
#include "PropertyHelper.h"
#include "SearchPacket.h"
#include "SearchMatch.h"

#include "MapSafeVector.h"

#ifdef profiling
#include "SearchPacket.h"
#endif

class SearchModule: public PreLoadMapModule {
public:
   SearchModule( CommandlineOptionHandler* handler ): 
      PreLoadMapModule( MODULE_TYPE_SEARCH, handler )
   { 
   }

   void createProcessor() {
      if ( m_processor.get() != NULL ) {
         return;
      }

      m_processor.reset( new SearchProcessor( m_loadedMaps.get() ) );
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

                                   
      m_reader = new SearchReader( m_queue.get(),
                                   m_senderReceiver->getSendQueue(),
                                   m_loadedMaps.get(),
                                   m_packetReader.get(),
                                   m_senderReceiver->getPort(),
                                   m_rank );
      Module::init();
   }
};


void profile();
void houseNumbersTest();
void abbrevTest();
void levDistTest();


int main(int argc, char *argv[])
try {

   ISABThreadInitialize init;
   PropertyHelper::PropertyInit propInit;

   errno = 0;
   uint32 mapToLoad = MAX_UINT32;
   bool testHouseNumbers = false;
   bool testLevDist      = false;
   bool testAbbrev       = false;

   auto_ptr<CommandlineOptionHandler> 
      coh( new CommandlineOptionHandler( argc, argv ) );
      
   char defVal[20];
   sprintf(defVal, "%u", MAX_UINT32);
   coh->addOption("-m", "--loadmap",
                 CommandlineOptionHandler::uint32Val,
                 1, &mapToLoad, defVal,
                 "Load a map and delete it again and exit");

   coh->addOption("", "--testHouseNumbers",
                 CommandlineOptionHandler::presentVal,
                 1, &testHouseNumbers, "F",
                 "Reads strings from stdin and prints the housenumbers "
                 "and the strings with the housenumber removed.");
      
   coh->addOption("", "--testAbbrev",
                 CommandlineOptionHandler::presentVal,
                 1, &testAbbrev, "F",
                 "Reads strings from stdin and abbrevates them");
                                        
   coh->addOption("", "--testLevDist",
                 CommandlineOptionHandler::presentVal,
                 1, &testLevDist, "F",
                 "Reads strings from stdin and prints the levenhsteindist");

#ifdef profiling
   profile();
#endif

   SearchModule* searchModule = new SearchModule( coh.release() );
   JTCThreadHandle searchHandle = searchModule;

   searchModule->parseCommandLine();

   if ( testHouseNumbers ) {
      houseNumbersTest();
   }
   
   if ( testAbbrev ) {
      abbrevTest();
   }
   
   if ( testLevDist ) {
      levDistTest();
   }

   if ( mapToLoad != MAX_UINT32 ) {
      char packInfo[Processor::c_maxPackInfo];
      MapSafeVector loadedMaps;
      SearchProcessor processor( &loadedMaps );

      LoadMapRequestPacket* lmap = new LoadMapRequestPacket(mapToLoad);     
      DeleteMapRequestPacket* dmap = new DeleteMapRequestPacket(mapToLoad);
      auto_ptr<ReplyPacket> lmapRepl( (ReplyPacket*)
                                      processor.handleRequest(lmap, packInfo));
      if ( lmapRepl->getSubType() != Packet::PACKETTYPE_LOADMAPREPLY ||
           lmapRepl->getStatus() != StringTable::OK ) {
         return 1;
      }

      mc2dbg << "Memory: " << SysUtility::getMemoryInfo() << endl;

      delete processor.handleRequest(dmap, packInfo);

      return 0;
   }

   searchModule->init();
   searchModule->start();
   searchModule->gotoWork( init );

   return 0;

} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initialize SearchModule. " 
          << ex.what() << endl;
   return 1;
}

void profile() {
#ifdef profiling

   mc2log << warn << "Warning: Running in profiling mode" << endl;
   auto_ptr<SearchProcessor> processor( new SearchProcessor( NULL ) );
   // test where the time is wasted
   // create SearchProcessor
   mc2dbg4 << "SearchProcessor constructor done" << endl;

   const uint32 loadMapID = 13;
   mc2log << info << "Load map number " << loadMapID << endl;
   LoadMapRequestPacket* lmp = new LoadMapRequestPacket(loadMapID, 0, 0);
   processor->handleRequest(static_cast<RequestPacket *>( lmp ));
   mc2log << info << "Map number " << loadMapID << " loaded" << endl;

   mc2log << info << "SearchProcessor constructor done" << endl;
   exit(0);
   
   uint32 *mapID = new uint32;
   mapID[0] = loadMapID;
   mc2dbg2 << "SearchProcessor constructor done" << endl;
   Packet *resPacket = NULL;

   char *searchString = new char[1024];

   const uint32 count = 1000;
   srand(3);
   mc2log << info << count << " requests between checkpoints." << endl;

   time_t tid2_t = 0;
   uint32 loopCount = 0;
   time_t tid_t = TimeUtility::getCurrentTime();
   uint32 totMatches = 0;
   do {
      for ( uint32 i=0; i<count; i++ ) {

         // create the question (normally not done in the module)
         const char *s;
         s = processor->getRandString();
         sprintf( searchString, "%s", s);
         searchString[11] = '\0';
         UserSearchRequestPacket *usr = new UserSearchRequestPacket(0, 0);
         usr->encodeRequest(1, mapID, "", 0, NULL, 0, 0, NULL,
                            0, NULL, NULL, NULL, SearchTypes::AllDB,
                            searchString, 0,
                            SearchTypes::CloseMatch,
                            SearchTypes::Anywhere,
                            SearchTypes::BestMatchesSort, 255, 42, 0);

         // handle the question
         resPacket = processor->handleRequest(usr);
         delete usr;

         // unpack the result (normally not done in the module)
         int nbrMatches;
         int position;
         VanillaMatch *vm1 = static_cast<VanillaSearchReplyPacket *>
            (resPacket)->getFirstMatch( position,
                                        nbrMatches );
         totMatches += nbrMatches;
         VanillaMatch *vm2 = vm1;
         delete vm1;
         for (int i = 0; i < nbrMatches; i++) {
            vm2 = static_cast<VanillaSearchReplyPacket *>
               (resPacket)->getNextMatch( position );
            vm1 = vm2;
            delete vm1;
         }
         
         delete resPacket;
      }
      loopCount++;
      tid2_t = TimeUtility::getCurrentTime();
   } while ((tid2_t-tid_t) < 10000 );
   
   mc2log << info << "Total time " << tid2_t-tid_t << " ms." << endl
          << (double) count * loopCount * 1000/(tid2_t-tid_t)
          << " requests were handled per second." << endl
          << "However, nothing was sent over the network..." << endl
          << count * loopCount << " requests performed" << endl
          << "Average number of matches per reply: "
          << (double) totMatches/(count*loopCount) << endl;
   exit(1);
#endif
}

void houseNumbersTest() {

   while ( cin ) {
      const int rowSize = 1024;
      char row[rowSize];
      // Isn't there a getline for strings?
      cin.getline(row, rowSize);
      int number;
      MC2String strippedString;
      StringSearchUtility::getStreetNumberAndSearchString(MC2String(row),
                                                          number,
                                                          strippedString);
      cout << "\"" << row << "\" -> \"" << strippedString
           << "\" #" << number << endl;
   }
}

void abbrevTest() {
   while ( cin ) {
      const int rowSize = 1024;
      char row[rowSize];
      // Isn't there a getline for strings?
      cin.getline(row, rowSize);
      char dest[1024];
      AbbreviationTable::abbreviate(row, dest, LangTypes::english,
                                    AbbreviationTable::anywhere);

      cout << "\"" << row << "\" -> \"" << dest
           << "\"" << endl;
   }
}

void levDistTest() {
   while ( cin ) {
      const int rowSize = 1024;
      char row1[rowSize];
      char row2[rowSize];
      // Isn't there a getline for strings?
      cin.getline(row1, rowSize);
      cin.getline(row2, rowSize);

      int levDist = Trie::getLevDist(row1, row2, 1000);
         
      cout << "Trie::getLevDist(\"" << row1 << "\", \""
           << row2 << "\") = " << levDist << endl;
         
   }
}

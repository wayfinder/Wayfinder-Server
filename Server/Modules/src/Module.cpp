/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Module.h"

#include "config.h"
#include "MapSafeVector.h"
#include "Processor.h"

#include "ModulePacketSenderReceiver.h"
#include "PacketReaderThread.h"
#include "StandardReader.h"
#include "PacketQueue.h"
#include "JobThread.h"
#include "multicast.h"
#include "ScopedArray.h"
#include "CommandlineOptionHandler.h"

namespace {
MC2String makeModuleName( moduletype_t moduleType )
{
   MC2String res = ModuleTypes::moduleTypeToString( moduleType );
   res += "Module";
   return res;
}

}
Module::Module( moduletype_t moduleType,
                CommandlineOptionHandler* handler,
                bool useMaps ):
   Component( makeModuleName( moduleType ).c_str(), handler ),
   m_loadedMaps( new MapSafeVector() ),
   m_jobThread( NULL ),
   m_reader( NULL ),
   m_type( moduleType ),
   m_useMaps( useMaps ) 
{
   setup();
}

Module::Module(moduletype_t moduleType,
               int argc, char* argv[],
               bool useMaps )
   : Component( makeModuleName(moduleType).c_str(),
                argc, argv),
     m_loadedMaps( new MapSafeVector() ),
     m_jobThread( NULL ),
     m_reader( NULL ),
     m_type( moduleType ),
     m_useMaps( useMaps )
{
   setup();
}

      
Module::~Module() {
}

void Module::setup() {
   m_cmdlineOptHandler->addOption( "-t", "--port",
                                   CommandlineOptionHandler::uint32Val,
                                   1, &m_preferredPort, "0",
                                   "Set the preferred port. This parameter "
                                   "overrides the default value in the property"
                                   " file.");

   m_cmdlineOptHandler->addOption( "-r", "--rank",
                                   CommandlineOptionHandler::uint32Val,
                                   1, &m_rank, "0",
                                   "Set the rank of this module.");

}

void
Module::parseCommandLine() throw ( ComponentException ) {

   uint32 mapSet = MAX_UINT32;
   if ( m_useMaps ) {
      m_cmdlineOptHandler->
         addOption( "-s", "--mapSet",
                    CommandlineOptionHandler::uint32Val,
                    1, &mapSet, "0xFFFFFFFF",
                    "Which map set to use, only valid and required if "
                    " more than one is defined in mc2.prop" );
   }
   m_cmdlineOptHandler->
      addOption( "", "--leader",
                 CommandlineOptionHandler::presentVal,
                 0, &m_startAsLeader, "false",
                 "Start as leader." );
   Component::parseCommandLine();
   
   if ( m_useMaps ) {
      // check map set settings early
      if ( mapSet != MAX_UINT32 ) {
         if ( Properties::getProperty("MAP_SET_COUNT") == NULL ) {
            throw ComponentException( "mapSet parameter given but "
                                      "MAP_SET_COUNT not "
                                      "configured!" );
         }
         if ( mapSet >= Properties::getUint32Property("MAP_SET_COUNT", 0) ) {
            throw ComponentException( "mapSet parameter can't be "
                                      "equal to or higher than "
                                      "MAP_SET_COUNT!" );;
         }
         Properties::setMapSet( mapSet );
      } else {
         if ( Properties::getProperty("MAP_SET_COUNT") != NULL ) {
            throw ComponentException( "mapSet parameter not given "
                                      "but MAP_SET_COUNT "
                                      "configured!" );
         }
      }
   }

   m_leaderIP = MultiCastProperties::getNumericIP( m_type, true );
   m_leaderPort = MultiCastProperties::getPort( m_type, true );
   m_availIP = MultiCastProperties::getNumericIP( m_type, false );
   m_availPort = MultiCastProperties::getPort( m_type, false );

   // set prop prefix to "module type"_MODULE
   m_propPrefix = StringUtility::
      copyUpper( ModuleTypes::moduleTypeToString( m_type ) );
   m_propPrefix += "_MODULE" ;

   if ( m_preferredPort == 0 ) {
      mc2log << info 
             << m_componentName << ": No port number given, using default."
             << endl;
      m_preferredPort = MultiCastProperties::getModulePort( m_type );
   }
}

void
Module::init() 
{
   if ( m_senderReceiver.get() == NULL ) {
      m_senderReceiver.
         reset( new ModulePacketSenderReceiver( m_preferredPort,
                                                IPnPort( m_leaderIP,
                                                         m_leaderPort ),
                                                IPnPort( m_availIP, 
                                                         m_availPort) ) );
   }

   if ( m_packetReader.get() == NULL ) {
      m_packetReader.reset(
          new PacketReaderThread(
       static_cast<ModulePacketSenderReceiver&>(*m_senderReceiver ) ) );
   }

   if (NULL == m_queue.get())
      m_queue.reset( new PacketQueue );

   // Need a processor, and only if we do not have a job thread.
   if ( m_processor.get() == NULL &&
        m_jobThread == NULL ) {
      throw ComponentException( "Module::init(): No Processor was instantiated by "
                                "the derived Module!");
   }

   if (NULL == m_jobThread) {
      m_jobThread = new JobThread( m_processor.get(), 
                                   m_queue.get(), 
                                   m_senderReceiver->getSendQueue() );
   }
   m_jobThreadHandle = m_jobThread;
   if (NULL == m_reader) {
      m_reader = 
         new StandardReader( m_type, m_queue.get(), 
                             m_senderReceiver->getSendQueue(),
                             m_loadedMaps.get(), *m_packetReader,
                             m_senderReceiver->getPort(),
                             m_rank, m_useMaps );
   }

   m_readerHandle = m_reader;
   Component::init();

   if ( m_startAsLeader ) {
      m_reader->setStartAsLeader( true );
   }
}

void
Module::run()
{
   m_jobThread->start();
   m_packetReader->start();
   m_reader->start();
   while ( m_readerHandle->isAlive() ) try {
      m_readerHandle->join();
   } catch ( const JTCException& ) { }

}

int
Module::handleCommand(const char* input)
{
   char* command = StringUtility::newStrDup(input);
   ScopedArray<char> origCommand( command );
   char** cmd = &command;
   char* token = StringUtility::strsep(cmd, " ");
   int retVal = 1;

   if (strcmp(token, "help") == 0) {
      Component::handleCommand(input);
      cout <<         "  set rank     - Set the rank of this module"
           << endl << "  vote         - Initiate a vote"
           << endl << "  queuestat    - Display Module queue status"
           << endl << "  queuedump    - Display Module queue dump"
           << endl << "  mapstatus    - Displays data about the module's maps"
           << endl;
   } else if ((strcmp(token, "quit") == 0)) {
      // code to shutdown module specific things
      retVal = Component::handleCommand(input);
   } else if ((strcmp(token, "shutdown") == 0)) {
      //
      // Start vote on other modules before we shutdown.
      //
      // This is done because the other modules will wait for
      // hearth beat timeout, so we force a vote instead which
      // reduces the wait time.
      m_reader->lastVote();

      // Terminate all threads, the order is important!
      m_reader->terminate();
      m_jobThread->terminate();
      m_queue->terminate();
      m_senderReceiver->stop();

      retVal = Component::handleCommand(input);
   } else if (strcmp(token, "config") == 0) {
      Component::handleCommand(input);
      cout <<         "  Preferred port     : " << m_preferredPort
           << endl << "  Leader IP          : " << prettyPrintIP(m_leaderIP)
           << endl << "  Leader Port        : " << m_leaderPort
           << endl << "  Available IP       : " << prettyPrintIP(m_availIP)
           << endl << "  Available Port     : " << m_availPort
           << endl;
   } else if (strcmp(token, "status") == 0) {
      Component::handleCommand(input);

      cout <<         "  queue length : " << m_queue->getStatistics()
           << endl << "  nbr of reqs  : " << m_reader->getNbrRequests()
           << endl << "  leader/avail : ";

      cout << (m_reader->isLeader() ? "leader" : "available")
           << endl << "  rank         : " << m_reader->getRank()
           << endl << "  nbr of votes : " << m_reader->getNbrVotes()
           << endl;

   } else if ( strcmp( token, "mapstatus" ) == 0 ) {
      printMapStatus();
   } else if ((strcmp(token, "set") == 0)) {
      token = StringUtility::strsep(cmd, " ");
      if (token && (strcmp(token, "rank") == 0)) {
         token = StringUtility::strsep(cmd, " ");
         if (token) {
            uint32 newRank = atol(token);
            m_reader->setRank(newRank);
            cout << "Rank set to " << newRank << endl;
         } else {
            cout << "Missing value for new rank" << endl;
         }
      } else {
         Component::handleCommand(input);
      }

   } else if ((strcmp(token, "vote") == 0)) {
      m_reader->vote();
   } else if (strcmp(token, "queuestat") == 0) {
      cout <<         "  queue length : " << m_queue->getStatistics()
           << endl;
      // statistics for processed packets (nbr for each type, avg process
      // time, max process time, etc
   } else if (strcmp(token, "queuedump") == 0) {
      cout <<         "  queue length : " << m_queue->getStatistics()
           << endl;
      // we want to dump short info for everything in the queue here
   } else {
      retVal = Component::handleCommand(input);
   }

   return retVal;
}

namespace {
/// Converts seconds to an "hh:mm:ss" format
string secondsToHMS( int seconds ) {
   ostringstream os;
   
   int hours = seconds / 3600;
   seconds -= hours;

   int minutes = seconds / 60;
   int secs = seconds % 60;

   os << setw(2) << setfill('0') << hours << ":" 
      << setw(2) << minutes << ":" 
      << setw(2) << secs;
   return os.str();
}
}

void Module::printMapStatus() {
   set<MapElement> maps;
   int size = m_reader->getLoadedMaps()->getAllMapInfo( maps );

   if ( size == 0 ) {
      cout << "No maps loaded" << endl;
      return;
   }

   // the column sizes
   const int idColSize     = 10;
   const int sizeColSize   = 12;
   const int statusColSize = 18;
   const int timeColSize   = 27;

   // print header
   cout << "Map status:" << endl;
   cout << setw(idColSize) << "id"
        << setw(sizeColSize) << "size"
        << setw(statusColSize) << "status"
        << setw(timeColSize) << "time since use (hh:mm:ss)" << endl;
   cout << setfill('-') 
        << setw(idColSize+sizeColSize+statusColSize+timeColSize) 
        << "" << setfill(' ') << endl;

   int totalSize = 0;

   // print the maps
   for ( set<MapElement>::iterator itr = maps.begin();
         itr != maps.end();
         ++itr ) {
      const MapElement& me = *itr;
      
      string lastUse = secondsToHMS( ( TimeUtility::getCurrentTime() - 
                                      me.getLastUse() ) / 1000 );

      cout << hex << setw(idColSize) << me.getMapID() << dec
           << setw(sizeColSize) << me.getSize() 
           << setw(statusColSize) << me.getStatusAsString()
           << setw(timeColSize) << lastUse << endl;

      totalSize += me.getSize();
   }
   cout << endl << "Sum of map sizes: " << totalSize << endl;
}

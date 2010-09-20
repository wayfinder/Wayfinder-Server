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

#include "MapReader.h"
#include "MapProcessor.h"
#include "JobThread.h"
#include "ISABThread.h"
#include "PacketQueue.h"
#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "Properties.h"
#include "CommandlineOptionHandler.h"
#include "Packet.h"
#include "PacketReaderThread.h"
#include "multicast.h"
#include "STLStringUtility.h"
#include "ModulePacketSenderReceiver.h"
#include "TimeUtility.h"
#include "PropertyHelper.h"
#include "PreLoadMapModule.h"
#include "PacketDump.h"

#include "XMLInit.h"
#include "SearchUnitBuilder.h"
#include "GenericMap.h"
#include "SearchUnit.h"
#include "MC2String.h"
#include "ModuleMap.h"
#include "MapPacket.h"
#include "MapGenerator.h"
#include "MapModuleNoticeContainer.h"
#include "MapModuleNotice.h"
#include "MapSafeVector.h"
#include "ProcessorFactory.h"

#include "DataBuffer.h"
#include "FilePtr.h"
#include "MapBits.h"

#include <iostream>
#include <vector>
#include <memory>
#include <stdlib.h>

#ifdef USE_XML
#include <util/PlatformUtils.hpp>
#endif

void mmExit();
int testLoadAll();
bool uint32StringToVector( const char* ids, vector<uint32>& list );
void playBackFromFile( FILE* file, const char* packetFileName );

bool saveModuleMap( uint32 mapID, uint32 mapType, 
                    MapModuleNoticeContainer* indexdb = NULL,
                    GenericMap* theMap = NULL,
                    bool cacheUpdate = false );
bool saveMapList( const char* mapIDs, vector<uint32>& mapTypes, 
                  bool cacheUpdate );
bool saveSearchMap( const char* mapIDs );
bool saveRouteMap( const char* mapIDs );
bool saveMaps( const char* saveMaps, bool cacheUpdate );

struct IsSubStr {
   IsSubStr( const char* value ):m_value( value ) { }
   bool operator()( const MC2String& str ) const {
      return str.find( m_value ) != MC2String::npos;
   }
   const char* m_value;
};

/**
 * Creates MapProcessor(s)
 */
class MapProcessorFactory: public ProcessorFactory {
public:
   MapProcessorFactory( MapSafeVector* loadedMaps,
                        const MC2String& packetFilename ):
      m_loadedMaps( loadedMaps ),
      m_packetFilename( packetFilename ) {
   }

   /// @see ProcessorFactory::create()
   Processor* create() {
      return new MapProcessor( m_loadedMaps,
                               m_packetFilename.c_str() );
   }

private:
   /// Info about which maps are loaded.
   MapSafeVector* m_loadedMaps;
   /// Filename for storing packages to file
   MC2String m_packetFilename;
};

class MapModule: public PreLoadMapModule {
public:   
   MapModule( CommandlineOptionHandler* handler ):
      PreLoadMapModule( MODULE_TYPE_MAP, handler )
   { 
   }

   void parseCommandLine() throw (ComponentException) {

      char* CL_packetFile = NULL;

      CommandlineOptionHandler& coh = *m_cmdlineOptHandler;

      coh.addOption( "", "--save-packets",
                     CommandlineOptionHandler::stringVal,
                     1, &CL_packetFile, "",
                     "File to save all incoming packets in for later"
                     " profiling");

      PreLoadMapModule::parseCommandLine();

      // Check if packets should be saved.

      if ( CL_packetFile != NULL && strlen( CL_packetFile ) ) {
         m_packetFilename = CL_packetFile;
      }

      if ( coh.getTailLength() > 0 ) {
         m_startWithMaps.reset( new Vector(0, coh.getTailLength() ) );
         for(int i = 0; i < coh.getTailLength(); i++) {
            uint32 id = strtoul( coh.getTail(i), NULL, 10 );
            if (id == UINT_MAX) {
               cerr << strerror(errno) << endl;
               exit(1);
            }
            m_startWithMaps->addLast( id );
         }
      } else {
         m_startWithMaps.reset( new Vector(0, 1) );
      }

      if ( m_startWithMaps->getSize() == 0 ) {
         mc2dbg2 << ". Will load all maps" << endl;
      } else {
         mc2dbg2 << ". Will load map(s): ";
         for ( uint32 i = 0; i < m_startWithMaps->getSize() - 1; ++i )
            mc2dbg2 << m_startWithMaps->getElementAt( i ) << ", " ;
         mc2dbg2 << m_startWithMaps->getElementAt( m_startWithMaps->getSize() - 1 )
                 << endl;
      }

   }

   void createProcessor() {
      // the processor factory creates the processor
      m_senderReceiver.
         reset( new ModulePacketSenderReceiver( m_preferredPort,
                                                IPnPort( m_leaderIP,
                                                         m_leaderPort ),
                                                IPnPort( m_availIP, 
                                                         m_availPort) ) );
      m_queue.reset( new PacketQueue() );

      m_procFactory.reset( new MapProcessorFactory( m_loadedMaps.get(),
                                                    m_packetFilename ) );

      // Create a threaded job dispatcher with only one job courier.
      // This way the cache can be handled while the job courier is working
      // on something else.
      m_jobThread = new JobThread( *m_procFactory, 1,
                                   NULL, // use default scheduler
                                   m_queue.get(),
                                   m_senderReceiver->getSendQueue() );
      cout << "MapModule.createProcessor DONE" << endl;
   }

   void init() throw ( ComponentException ) {

      // init processor if we do not have one.
      if ( m_jobThread == NULL ) {
         createProcessor();
      }

      m_packetReader.
         reset( new PacketReaderThread( static_cast<ModulePacketSenderReceiver&>
                                        (*m_senderReceiver ) ) );

      m_reader = new MapReader( m_queue.get(),
                                m_senderReceiver->getSendQueue(),
                                m_loadedMaps.get(),
                                m_packetReader.get(),
                                m_senderReceiver->getPort(),
                                m_rank, 
                                m_startWithMaps.get() );
      Module::init();
   }
   const MC2String& getPacketFilename() const { return m_packetFilename; }
private:
   MC2String m_packetFilename;
   auto_ptr<Vector> m_startWithMaps;
   auto_ptr<MapProcessorFactory> m_procFactory;
};

int main(int argc, char *argv[])
try { 
   atexit( mmExit );

   ISABThreadInitialize init;
   XMLTool::XMLInit xmlInit;
   PropertyHelper::PropertyInit propInit;

   uint32 mapToLoad = MAX_UINT32;
   char* searchMapsToSave = NULL;
   char* routeMapsToSave = NULL;
   char* mapsToSave = NULL;
   
   char* CL_profileFile = NULL;
   bool CL_testLoadAll = false;
   bool CL_cacheUpdate = false;

   
   auto_ptr<CommandlineOptionHandler> 
      coh( new CommandlineOptionHandler( argc, argv ) );

   coh->setTailHelp("[mapid...]");
   coh->setSummary("(This text could have been better). Use the tail "
                   "to specify which map id:s to load. If none present, "
                   "load all maps.");
   


   coh->addOption("", "--profile-processor",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_profileFile, "",
                  "If present the processor will be run in the main "
                  "thread and read packets of the format 4 bytes len, "
                  "len bytes packet from file");

   coh->addOption("", "--saveSearchMap",
                  CommandlineOptionHandler::stringVal,
                  1, &searchMapsToSave, "",              
                  "Save a list of SearchMaps to disk and exit. "
                  "Cache-path must be set "
                  "in mc2.prop for this to work properly.");

   coh->addOption("", "--saveRouteMap",
                  CommandlineOptionHandler::stringVal,
                  1, &routeMapsToSave, "",              
                  "Save a list of RoutingMaps to disk and exit. "
                  "Cache-path must be set "
                  "in mc2.prop for this to work properly.");

   coh->addOption("", "--saveMaps",
                  CommandlineOptionHandler::stringVal,
                  1, &mapsToSave, "",              
                  "Save a list of mapids for a number of map types "
                  "to disk and exit. "
                  "Format: \"mapType(,mapType)*;mapID(,mapID)*\""
                  "Cache-path must be set "
                  "in mc2.prop for this to work properly.");

   coh->addOption("", "--testLoadAll",
                  CommandlineOptionHandler::presentVal,
                  0, &CL_testLoadAll, "F",
                  "Test loading all maps in index.db" );

   char defVal[20];
   sprintf(defVal, "%u", MAX_UINT32);
   coh->addOption("-m", "--loadmap",
                  CommandlineOptionHandler::uint32Val,
                  1, &mapToLoad, defVal,
                  "Load a map and delete it again and exit");


   coh->addOption("", "--cacheUpdate",
                  CommandlineOptionHandler::presentVal,
                  0, &CL_cacheUpdate, "F",
                  "Use together with --saveMaps. Makes caches get written only "
                  "if no cache file exist or the existing cache file is older "
                  "than corresponging map file.");

   MapModule* mapModule = new MapModule( coh.release() );
   JTCThreadHandle moduleHandle = mapModule;

   mapModule->parseCommandLine();

   if ( CL_testLoadAll ) {
      return testLoadAll();
   }
   
   if ( searchMapsToSave != NULL && *searchMapsToSave != '\0' ) {
      return saveSearchMap(searchMapsToSave) ? 0 : 1;
   }

   if ( routeMapsToSave != NULL && *routeMapsToSave != '\0' ) {
      return saveRouteMap(routeMapsToSave) ? 0 : 1;
   }

   if ( mapsToSave != NULL && *mapsToSave != '\0' ) {
      if (CL_cacheUpdate){
         mc2log << info << "Only updates cache maps not up-to-date." << endl;
      }
      return saveMaps( mapsToSave, CL_cacheUpdate ) ? 0 : 1;
   }

   if ( mapToLoad != MAX_UINT32 ) {
      char packInfo[Processor::c_maxPackInfo];
      MapSafeVector loadedMaps;
      MapProcessor processor(&loadedMaps, NULL);
      LoadMapRequestPacket* lmap = new LoadMapRequestPacket(mapToLoad);
      DeleteMapRequestPacket* dmap = new DeleteMapRequestPacket(mapToLoad);
      delete processor.handleRequest(lmap, packInfo);

      cout << SysUtility::getMemoryInfo() << endl;

      processor.handleRequest(dmap, packInfo);
      exit(0);
   }
   
   if ( CL_profileFile ) {
      FileUtils::FilePtr profileFD( fopen( CL_profileFile, "r" ) );
      if ( profileFD.get() == NULL ) {
         mc2dbg << fatal << "[MM]: Can not load profile file: " 
                << CL_profileFile << endl;
         mc2dbg << fatal << " Error: " << strerror( errno ) << endl;
         exit( EXIT_FAILURE );
      }

      playBackFromFile( profileFD.get(), mapModule->getPacketFilename().c_str() );

      exit( 0 );
   }

   mapModule->init();
   mapModule->start();
   mapModule->gotoWork( init );

   return 0;

} catch ( const ComponentException& ex ) {
   mc2log << fatal << "Could not initilize component. " 
          << ex.what() << endl;
   return 1;
}


class mapIDSort {
public:
    bool operator()( uint32 a , uint32 b ) const {
       if ( MapBits::getMapLevel( a ) > MapBits::getMapLevel( b ) ) {
          return true;
       } else if ( MapBits::getMapLevel( a ) < MapBits::getMapLevel( b ) )
       {
          return false;
       } else {
          return a < b;
       }
    }
};


void mmExit() 
{
   mc2log << "[MM]: Exiting." << endl;
}

int testLoadAll()
{
   MapModuleNoticeContainer indexdb;
   indexdb.load( INDEX_NAME );
   for( int i = 0, n = indexdb.getSize(); i < n; ++i ) {
      const MapModuleNotice* mn = indexdb[i];
      auto_ptr<GenericMap> curMap( GenericMap::createMap( mn->getMapID() ) );
      if ( curMap.get() == NULL ) {
         return EXIT_FAILURE;
      }
   }
   return EXIT_SUCCESS;
}

bool
saveModuleMap(uint32 mapID, uint32 mapType, 
              MapModuleNoticeContainer* indexdb,
              GenericMap* theMap,
              bool cacheUpdate )
{
   mc2dbg << "[MapModule]: Will cache map 0x" << hex << mapID << dec
          << endl;

   // Load the map
   uint32 startTime = TimeUtility::getCurrentTime();
   bool deleteMap = false;
   if ( theMap == NULL ) {
      GenericMap::createMap( mapID );
      deleteMap = true;
   }
   
   if ( theMap == NULL ) {
      return false;
   }

   // Create new SearchMapGenerator with fake maphandler etc.
   MapGenerator* mapGen = MapGenerator::createGeneratorForDataBuffer(mapType);

   // Create index.db if necessary.
   bool deleteIndexDB = false;
   // We will have to load index.db to check if the maps are
   // correct since we have had some maps that weren't correct.
   if ( /* mapGen->getIndexDBNeeded() && */ indexdb == NULL ) {
      // Load index.db
      indexdb = new MapModuleNoticeContainer();
      indexdb->load( INDEX_NAME );      
      deleteIndexDB = true;
   }

   const MapModuleNotice* mapNotice = indexdb->getMap( mapID );
   mc2dbg << "mapNotice " << mapNotice->getCreationTime() << " map "
          << theMap->getCreationTime() << endl;
   MC2_ASSERT( mapNotice->getCreationTime() == theMap->getCreationTime() );

   // Get the map.
   DataBuffer* db = mapGen->generateDataBuffer(theMap, indexdb);
   
   bool success = false;
   mc2dbg << "[MapModule]: Will save " << db->getBufferSize()
          << " bytes" << endl;
   if ( ModuleMap::saveCachedMap(*db, theMap,
                                 mapType,
                                 theMap->getCreationTime(),
                                 MapGenerator::generatorVersion,
                                 cacheUpdate)) {
      mc2log << info << "[MapModule]: Cache for map 0x" << hex << mapID << dec
             << " OK" << endl;
      success = true;
   } else {
      mc2log << error << "[MapModule]: Saving failed for map 0x"
             << hex << mapID << dec << endl;
      success = false;
   }

   mc2dbg << "[MapModule]: Map handled in "
          << uint32(TimeUtility::getCurrentTime() - startTime) << endl;

   // Clean up
   mc2dbg << "[MapModule]: Cleaning up" << endl;   
   delete db;
   if ( deleteMap ) {
      delete theMap;
   }
   if ( deleteIndexDB ) {
      delete indexdb;
   }
   delete mapGen;
   
   return success;
}

bool
uint32StringToVector( const char* ids, vector<uint32>& list ) {
   char* tmpPtr = NULL;
   const char* idPos = ids;
   uint32 id = 0;
   bool allOk = true;
   if ( *idPos != '\0' ) {
      do {
         id = strtoul( idPos, &tmpPtr, 0 );
         if ( tmpPtr != idPos && (tmpPtr[ 0 ] == ',' ||
                                  tmpPtr[ 0 ] == '\0' ) )
         {
            list.push_back( id );
            if ( tmpPtr[ 0 ] != '\0' ) {
               idPos = tmpPtr + 1;
            }
         } else {
            // Error in indata!
            mc2log << error << "MM::uint32StringToVector() Bad uint32 in "
                   << MC2CITE( ids ) << " at " << MC2CITE( idPos )
                   << endl;
            tmpPtr = NULL;
         }
      } while ( tmpPtr != NULL && tmpPtr[ 0 ] != '\0' );
   } // End non empty ids
   
   return allOk;
}

bool 
saveMapList( const char* mapIDs, vector<uint32>& mapTypes, 
             bool cacheUpdate )
{
   // Create index.db 
   MapModuleNoticeContainer* indexdb = NULL;
   // Load index.db
   indexdb = new MapModuleNoticeContainer();
   indexdb->load(  INDEX_NAME, Properties::getMapSet() );      
   bool allOk = true;

   vector<uint32> mapIDList;
   if ( StringUtility::strcmp( mapIDs, ".." ) == 0 ) {
      // All maps
      for ( uint32 i = 0 ; i < indexdb->getSize() ; ++i ) {
         mapIDList.push_back( indexdb->getElementAt( i )->getMapID() );
      }
      sort( mapIDList.begin(), mapIDList.end(), mapIDSort() );
   } else {
      allOk = uint32StringToVector( mapIDs, mapIDList );
   }

   // Fixup mapset
   if( Properties::getMapSet() != MAX_UINT32 ) {
      for ( vector<uint32>::iterator it = mapIDList.begin();
            it != mapIDList.end();
            ++it ) {
         *it = MapBits::getMapIDWithMapSet( *it, Properties::getMapSet() );
      }
   }

   if ( allOk ) {
      for ( uint32 i = 0 ; i < mapIDList.size() ; ++i ) {
         GenericMap* theMap = NULL;


         for ( uint32 j = 0 ; j < mapTypes.size() ; ++j ) {
            uint32 mapID = mapIDList[i];
            MC2String mapFileName = GenericMapHeader::getFilenameFromID(mapID);
            if ( cacheUpdate && ModuleMap::cacheIsUpToDate(mapID, 
                                                           mapFileName,
                                                           mapTypes[ j ]) ){
               // Map is up to date, continue.
            }
            else {
               if (theMap == NULL ){
                  theMap = GenericMap::createMap( mapIDList[ i ] );
                  mc2dbg8 << "Map file name" << theMap->getFilename() << endl;
               }
               if ( !saveModuleMap( mapIDList[ i ], mapTypes[ j ], indexdb,
                                    theMap, cacheUpdate ) ){
                  allOk = false;
                  mc2log << warn << "MM::saveMapList() map 0x" << hex
                         << mapIDList[ i ] << dec << " for map type "
                         << mapTypes[ j ] << " failed" << endl;
                  MC2_ASSERT(false);
               }
            }
         }
         delete theMap;
      }
   } else {
      // Error in indata!
      mc2log << error << "MM::saveSearchMap() Bad mapID list in "
             << MC2CITE( mapIDs ) << endl;
      MC2_ASSERT(false);
   }
   
   delete indexdb;
   return allOk;
}

bool
saveSearchMap( const char* mapIDs )
{

   vector<uint32> mapTypes;
   mapTypes.push_back( MapRequestPacket::MAPREQUEST_SEARCH );
   return saveMapList( mapIDs, mapTypes, false );

}


bool
saveRouteMap( const char* mapIDs )
{
   vector<uint32> mapTypes;
   mapTypes.push_back( MapRequestPacket::MAPREQUEST_ROUTE );
   return saveMapList( mapIDs, mapTypes, false );
}


bool
saveMaps( const char* saveMaps, bool cacheUpdate ) {
   bool ok = true;
   const char* semiPos = strchr( saveMaps, ';' );
   if ( semiPos != NULL ) {
      MC2String mapTypesStr( saveMaps, (semiPos-saveMaps) );
      vector<uint32> mapTypes;

      ok = uint32StringToVector( mapTypesStr.c_str(), mapTypes );
      if ( ok ) {
         ok = saveMapList( semiPos + 1, mapTypes, cacheUpdate );
      } else {
         mc2log << error << "MM:saveMaps error in mapTypes list."
                << endl;
      }
   } else {
      mc2log << error << "MM:saveMaps no ';' in argument see help."
             << endl;
      ok = false;
   }

   return ok;
}

void
playBackFromFile(FILE* file, const char* packetFileName)
{
   char packinfo[Processor::c_maxPackInfo];
   MapSafeVector loadedMaps;
   MapProcessor processor(&loadedMaps, packetFileName);
   
   while ( ! feof( file ) ) {      
      RequestPacket* packet = 
         (RequestPacket*)PacketUtils::loadPacketFromFile( file );
      if ( packet == NULL ) {
         return;
      }
      mc2log << info
             << "[MM]: Packet of size " << packet->getLength() << " read " << endl;
      delete processor.handleRequest(packet, packinfo);
   }
}

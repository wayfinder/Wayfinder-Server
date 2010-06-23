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

#include "MapHandler.h"
#include "Properties.h"
#include "MapLockElement.h"

#include "Map.h"
#include "GenericMap.h"
#include "GfxFeatureMapProcessor.h"
#include "GfxTileFeatureMapProcessor.h"
#include "StringTable.h"
#include "STLUtility.h"

MapHandler::mapStorageEntry_t::~mapStorageEntry_t()
{
   delete m_featureProc;
   delete m_tileProc;
   delete m_map;
}

MapHandler::MapHandler()
{
   mc2dbg2 << "MapHandler created" << endl;

   // Set path to the maps.
   const char* propPath = Properties::getProperty("MAP_PATH");  
   if ( propPath == NULL ) {
      //propPath = "/home/is/devel/MC2/Maps/";
      propPath = "./";
      mc2dbg2 << "Property MAP_PATH == NULL, defaulting to " 
              << propPath << endl;
   }
   path = new char[ strlen( propPath ) + 2 ];
   strcpy( path, propPath );
   if ( path[ strlen( path ) - 1 ] != '/' ) {
      // Append '/'
      strcat( path, "/" );
   }
   mc2dbg2 << "Property MAP_PATH == " << path << endl;
   //m_overview = NULL;
}


MapHandler::~MapHandler()
{
   mc2dbg4 << "MapHandler destructed" << endl;

   for( mapStorage_t::iterator it = m_maps.begin();
        it != m_maps.end();
        ++it ) {
      delete it->second;
   }
   
   delete [] path;
   STLUtility::deleteValues (  m_mapLocks );
}

GenericMap*
MapHandler::getMap(uint32 id)
{
   mc2dbg4 << "MapHandler::getMap(" << id << ")" << endl;
   mapStorage_t::iterator it = m_maps.find( id );
   if ( it != m_maps.end() ) {
      return it->second->m_map;
   } else {
      return NULL;
   }
}

GfxTileFeatureMapProcessor*
MapHandler::getTileProcessor( uint32 mapID )
{
  mapStorage_t::iterator it = m_maps.find( mapID );
   if ( it != m_maps.end() ) {
      return it->second->m_tileProc;
   } else {
      return NULL;
   } 
}

GfxFeatureMapProcessor*
MapHandler::getFeatureProcessor( uint32 mapID )
{
  mapStorage_t::iterator it = m_maps.find( mapID );
   if ( it != m_maps.end() ) {
      return it->second->m_featureProc;
   } else {
      return NULL;
   } 
}

GenericMap*
MapHandler::getLockedMap(uint32 id)
{
   if(!readLockMap(id)){
      mc2log << error << "Readlock failed " << endl;
      return NULL;
   }
   
   else
      return getMap(id);
}


StringCode
MapHandler::addMap( uint32 mapID, uint32& mapSize ) 
{
   mapSize = 1; // Changed to something more sensible when the map is 
               // loaded below.
   mc2dbg2 << "MapHandler::addMap(" << mapID << ")" << endl;
   
   GenericMap* tmpMap = getMap( mapID );

   if ( tmpMap == NULL ) {
      // OK, map didn't exist

      // Ordinary map
      tmpMap = GenericMap::createMap( mapID );

      if ( tmpMap != NULL ) {

         // Get the map size from the map.
         mapSize = tmpMap->getApproxMapSize();
         
         MapLockElement* newLock = new MapLockElement( mapID );
         m_mapLocks.push_back( newLock );
         mc2dbg4 << "New mapLock created " << m_mapLocks.size() << endl;

         GfxTileFeatureMapProcessor* theProc =
            new GfxTileFeatureMapProcessor( tmpMap );
         // Share the poiinfo with the tileprocessor.
         GfxFeatureMapProcessor* theFeat =
            new GfxFeatureMapProcessor( *tmpMap, theProc );
         
         m_maps.insert(
            make_pair( mapID, new mapStorageEntry_t( tmpMap,
                                                     theProc,
                                                     theFeat ) ) );
         
         
         return StringTable::OK;
      } else {
         return StringTable::ERROR_LOADING_MAP;
      }
   } else {
      // Set the map size anyway.
      mapSize = tmpMap->getApproxMapSize();
      mc2log << warn << "MapHandler: Map already loaded"
             << "   mapID " << mapID << endl;
      return StringTable::ERROR_MAP_LOADED;
   }

   // Index contains the position of the inserted map, 
   // MAX_UINT32 upon failure
   return StringTable::NOT;
}

bool
MapHandler::removeMap(uint32 id)
{
   mapStorage_t::iterator it = m_maps.find( id );
   
   if ( it != m_maps.end() ) {
      
      // This might be in a separate thread.
      mc2dbg << "Awaiting all readLocks for map : " << id << endl;
      deleteLockMap(id);
      mc2dbg << "Map " << id << " OK to delete " << endl;
      mc2dbg4 << "Removing map from mapVector" << endl;
      if ( true ) {
         mc2dbg4 << "Deleting the actual map" << endl;
         delete it->second;
         m_maps.erase( it );
         mc2dbg2 << "Map deleted" << endl;
      } else {
         mc2dbg2 << "Remove returned false" << endl;
         // Returning the map lock (creating a new one)
         MapLockElement* me = new MapLockElement(id);
         m_mapLocks.push_back( me );
      }
      return (true);
   } else {
      return (false);
   }
}



bool
MapHandler::readLockMap(uint32 mapID)
{
   MapLockElement search(mapID);
   
   MapLockVector::iterator it = find_if( m_mapLocks.begin(),
                                         m_mapLocks.end(),
                                         bind2nd( MapIDEqualCmp(), &search ) );
   
   if(it == m_mapLocks.end() ) {
      mc2log << error << " MapLock Not Found " << endl;
      return false;
   }

   return (*it)->addReaderToMap();
}

bool
MapHandler::deleteLockMap(uint32 mapID)
{
   MapLockElement search(mapID);
   
   MapLockVector::iterator it = find_if( m_mapLocks.begin(),
                                         m_mapLocks.end(),
                                         bind2nd( MapIDEqualCmp(), &search ) );
   
   if( it == m_mapLocks.end() ) {
      return false;
   }

   MapLockElement* mapLock = *it;
   if( mapLock->deleteMap() ) {
      m_mapLocks.erase( it );
      mc2dbg4 << "Nbr locks : " << m_mapLocks.size() <<endl;
      delete mapLock;
      return true;
   }
   else {
      return false;
   }
}

bool
MapHandler::readReleaseMap(uint32 mapID)
{

   MapLockElement search(mapID);
   
   
   MapLockVector::iterator it = find_if( m_mapLocks.begin(),
                                         m_mapLocks.end(),
                                         bind2nd( MapIDEqualCmp(), &search) );
   
   if( it == m_mapLocks.end() ) {
      return false;
   }
   
   (*it)->finishRead();
   return true;
}

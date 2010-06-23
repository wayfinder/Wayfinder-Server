/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPHANDLER_H
#define MAPHANDLER_H

#include "config.h"
#include "ObjVector.h"
#include "MapLockElement.h"

#include <map>

class GenericMap;
class GfxFeatureMapProcessor;
class GfxTileFeatureMapProcessor;
class StringCode;

/**
  *   Container for all the loaded map-objects.
  *
  */
class MapHandler {
   public:
      /**
        *   Creates an empty MapHandler.
        */
      MapHandler();

      /**
        *   Delete this map handler.
        */
      virtual ~MapHandler();

      /**
        *   To get a map-objbect with specified mapID.
        *   @param   id       ID of the map.
        *   @return  Pointer to mapobject with id, NULL if no such
        *            map found.
        */
      GenericMap* getMap(uint32 id);

      /**
       *    Returns the GfxTileFeatureMapProcessor for the map or
       *    NULL.
       */
      GfxTileFeatureMapProcessor* getTileProcessor( uint32 mapID );

      /**
       *    Returns the GfxFeatuereMapProcessor for the map or
       *    NULL.
       */
      GfxFeatureMapProcessor* getFeatureProcessor( uint32 mapID );
      
      GenericMap* getLockedMap(uint32 id);
      
      /**
        *   Adds a map to the internal datastructure from file.
        *   Uses membervariable path and the mapID printed in 9 positions
        *   followed by ".mcm" as filename.
        *   @param mapID   The ID of the map to add.
        *   @param mapSize The size of the map.
        *   @return  True if the map is added, false otherwise.
        */
      StringCode addMap(uint32 mapID,
                        uint32& mapSize);

      /**
        *   Remove one map from this MapHandler (not on disk...).
        *   @param id         The id of the map to remove.
        *   @return  True if the map is removed, false otherwise.
        */
      bool removeMap(uint32 id);

      /**
        *   This function is inlined.
        *   @return  Pointer to the path.
        */
      inline const char* getPath() const;


         bool readLockMap(uint32 mapID);

         bool deleteLockMap(uint32 mapID);

         bool readReleaseMap(uint32 mapID);
         
   private:

     // Compare the mapID:s
      struct MapIDEqualCmp : public binary_function<const MapLockElement*, const MapLockElement*, bool> {
         bool operator() (const MapLockElement* elm, const MapLockElement* elm2) const {
            return (*elm) == (*elm2);
         }
      };

      struct mapStorageEntry_t {        
         mapStorageEntry_t( GenericMap* map,
                            GfxTileFeatureMapProcessor* proc,
                            GfxFeatureMapProcessor* featureProc ) {
            m_map = map;
            m_tileProc = proc;
            m_featureProc = featureProc;
         }

         /// Deletes the map and the tile processor.
         ~mapStorageEntry_t();
         
         /// The map
         GenericMap* m_map;
         /// The tile processor
         GfxTileFeatureMapProcessor* m_tileProc;
         /// The feature map processor
         GfxFeatureMapProcessor* m_featureProc;
      };

      /// The type of storage to put the maps etc. into
      typedef map<uint32, mapStorageEntry_t*> mapStorage_t;

      mapStorage_t m_maps;
      
      /**
        *   The path to the mcm-files, containing the maps.
        */
      char *path;

      typedef std::vector< MapLockElement* > MapLockVector;

      MapLockVector m_mapLocks;
      
};


// =======================================================================
//                                 Implementation of the inlined methods =


inline const char* 
MapHandler::getPath() const
{
   return (path);
}



#endif


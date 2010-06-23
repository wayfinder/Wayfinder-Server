/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Processor.h"

#include "StringTable.h"

class MapSafeVector;

/**
 * A processor that handles map loading and other map related functions.
 * This processor is currently not thread safe, due to MapSafeVector with
 * the beginWork, endWork, and finishRequest.
 */
class MapHandlingProcessor: public Processor {
public:
   explicit MapHandlingProcessor( MapSafeVector* p_loadedMaps );
   virtual ~MapHandlingProcessor();

   /**
    *    Virtual function for loading a map. Should be called
    *    by handleRequest when that function gets a loadMapRequest
    *    and isn't virtual anymore.
    *    @param mapID The map to load.
    *    @param mapSize Outparameter describing the size of the
    *                   map.
    *    @return StringTable::OK if ok or StringTable::ERROR_MAP_LOADED
    *            if map is loaded. Is really ok.
    */
   virtual StringTable::stringCode loadMap(uint32 mapID,
                                           uint32& mapSize) = 0;
   
   /**
    *    Virtual function to be called when a map should
    *    be deleted.
    *    @param mapID The map to be deleted.
    *    @return StringTable::OK if ok.
    */
   virtual StringTable::stringCode deleteMap(uint32 mapID) = 0;
   

protected:
   virtual Packet* handleSpecialPacket( const RequestPacket& p,
                                        char* packetInfo );

   /**
    *   Removes a map from the loadedMaps-vector.
    */
   void removeMap(uint32 mapID);
   /**
    *  Marks a map as being loaded in the safe vector.
    *  @param mapID The Id of the map.
    *  @return True if no map was loading.
    */
   bool startLoadingMap(uint32 mapID);
   
   /**
    *  Removes the map being loaded from the MapSafeVector.
    *  @return True if a map was being loaded.
    */
   bool cancelLoadingMap(uint32 mapID);

   /**
    *  Moves a map from the being loaded field to the loaded vector
    *  in the MapSafeVector.
    *  @param size The size of the map that was loaded.
    *  @return True if a map was loading.
    */
   bool finishLoadingMap(uint32 mapID, uint32 size);

   MapSafeVector &getLoadedMapsVector() { return *m_loadedMaps; }

private:

   /**
    *   Pointer to the SafeVector in the Reader that contains mapID
    *   of all loadedMaps It should be updated after every processing
    *   of LoadMapRequestPackets.
    */
   MapSafeVector* m_loadedMaps;
};


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_STATUISTITSCS_H
#define MAP_STATUISTITSCS_H

#include "config.h"
#include <set>
#include <map>
#include <iostream>
#include "MC2String.h"

class Packet;

/**
 *  Element to put in a MapSafeVector.
 *  Each MapElement describes a loaded map.
 */
class MapElement {
public:
   /**
    *  Create a new MapElement with status == loaded.
    *  @param mapID The ID of the map.
    *  @param size The size of the map.
    */ 
   MapElement(uint32 mapID, uint32 size);
   
   /**
    *  Destructor. Inlined.
    */
   virtual ~MapElement(){};

   /**
    *  Return the size of the map. Inlined.
    *  @return The size of the map.
    */
   uint32 getMapSize() const
      { return m_size;}
   

   /**
    *  Return the ID of the map. Inlined.
    *  @return The ID of the map.
    */
   uint32 getMapID() const
      { return m_mapID;}


   /**
    *  Set the ID of the map. Inlined.
    *  @param mapID The ID of the map.
    */
   void setMapID(uint32 mapID)
      { m_mapID = mapID;}
   

   /**
    *  Set the size of the map. Inlined.
    *  @param size The size of the map.
    */
   void setSize(uint32 size)
      { m_size = size;}

   /**
    *   Returns the size of the map.
    */
   uint32 getSize() const { return m_size; }

   /**
    *   Return true if the map is loaded or loading.
    */
   bool isMapLoadedOrLoading() const;

   /**
    *   Returns true if the map is fully loaded.
    */
   bool isMapLoaded() const;

   /**
    *   Updates last use time to now.
    */
   void updateLastUse();

   /**
    *   Returns last use time in millis.
    */
   uint32 getLastUse() const { return m_lastUse; }
   
   enum status_t {
      /// The leader has told the module to load the map
      TOLD_TO_LOAD   = 0,
      /// The processor is loading the map
      LOADING        = 1,
      /// The map is fully loaded
      LOADED         = 2,
      /// The leader has told the module to delete the map
      TOLD_TO_DELETE = 3,
      /// The processor is deleting the map
      DELETING       = 4,
      /// The processor has deleted the map
      DELETED        = 5
   };

   /**
    *   Returns true if the module is loading a map.
    */
   bool isLoading() const { return m_status == TOLD_TO_LOAD ||
                                   m_status == LOADING; }

   /**
    *   Returns true if the module is deleting
    */
   bool isDeleting() const { return m_status == TOLD_TO_DELETE ||
                                    m_status == DELETING; }

   

   /**
    *   Returns the loading status of the map.
    */
   status_t getStatus() const {
      return m_status;
   }

   /**
    *   Sets the status of the map to the newStatus.
    */
   void setStatus(status_t newStatus) {
      m_status = newStatus;
   }

   /**
    * Returns the status as a string for debug info etc.
    */
   const char* getStatusAsString() const;

   /**
    *   Compares mapID to the other map id.
    *   To be able to put the elements into
    *   a set.
    */
   bool operator<(const MapElement& other) const {
      return m_mapID < other.m_mapID;
   }
   
   /**
    *   Saves the MapElement into the packet.
    *   @param p   Packet to save the element in.
    *   @param pos Position in packet.
    */
   int save(Packet* p, int& pos ) const;

   /**
    *   Saves the MapElement into the packet.
    *   @param p   Packet to save the element in.
    *   @param pos Position in packet.
    */
   int load(const Packet* p, int& pos );

private:
         
   /// The mapID of the MapElement.
   uint32 m_mapID;

   /// The size of the MapElement.
   uint32 m_size;

   /// The last use of the map
   uint32 m_lastUse;
   
   /// The status of the Map
   status_t m_status;
   
};

/**
 *  Module replacement for SafeVector with extra fields to enable 
 *  more information to be sent from Processors to Readers.
 *  
 */
class MapStatistics : protected map<uint32, MapElement> {
public:
   
   /**
    *   Well - consructor.
    */
   MapStatistics();
   
   /**
    *  Detructor.
    */
   virtual ~MapStatistics();
   
   /**
    *    Saves information about the maps in the packet.
    */
   int save(Packet* p, int& pos) const;
   
   /**
    *    Loads information back from the packet.
    */
   int load(const Packet* p, int& pos);
   
   /**
    *    Sets max mem.
    */
   void setMaxMem(uint64 maxMem);
   
   /**
    *    Returns max mem.
    */
   uint64 getMaxMem() const { return m_maxMem; }
   
   /**
    *    Sets opt mem.
    */
   void setOptMem(uint64 optMem);
   
   /**
    *
    */
   uint64 getOptMem() const { return m_optMem; }
   
   /**
    *    Returns the queue length.
    */
   int getQueueLength() const { return m_queueLength; }
   
   /**
    *    Sums up the amount of used memory.
    */
   uint64 calcUsedMem() const;

   /**
    *    Returns true if the module is loading a map.
    */
   bool moduleIsLoading() const;

   /**
    *    Returns true if the module is deleting a map.
    */
   bool moduleIsDeleting() const;

   /**
    *   This function is inlined.
    *   @return  The number of elements in the vector (lastUsed).
    */
   uint32 getNbrOfMaps() {
      return size();
   }
   
   /**
    *    Returns true if the map is loaded or loading.
    *    @param mapID The map id to load.
    *    @return True if the map is on the way.
    */
   bool isMapLoadedOrLoading(uint32 mapID) const;

   /**
    *    Returns true if the map is about to be loaded.
    */
   bool isMapLoading( uint32 mapID ) const;
   
   /**
    *    Returns true if the map is fully loaded.
    */
   bool isMapLoaded(uint32 mapID) const;

   /**
    *  @param mapID The map id.
    *  @param size The size of the map that was loaded.
    *  @return False if the map was not found.
    */
   bool finishLoadingMap(uint32 mapID, uint32 mapSize);

   /**
    *    Sets the size of the map to mapSize.
    */
   bool setMapSize(uint32 mapID, uint32 mapSize);
   
   /**
    *    Fills a set with the currently loaded
    *    maps.
    *    @param maps The set to put the mapIDs in.
    *    @return The number of maps.
    */
   int getLoadedMaps(set<uint32>& maps);
   
   /**
    *    Fills the set with copies of the MapElements
    *    currently in the vector.
    */
   int getAllMapInfo(set<MapElement>& maps);

   /**
    *    Returns the 1 minute load average.
    *    Calced with every statistics sent.
    */
   float getLoad1() const { return m_loadAvg_1; }

   /**
    *    Returns the 5 minute load average.
    *    Calced with every statistics sent.
    */
   float getLoad5() const { return m_loadAvg_5; }

   /**
    *    Returns the 15 minute load average.
    *    Calced with every statistics sent.
    */
   float getLoad15() const { return m_loadAvg_15; }

   
   /**
    *   @param  mapID The ID of the map to remove from the Vector.
    *   @return True if the element is removed, false otherwise.
    */
   bool removeMap(uint32 mapID);
      
   /**
    *  Returns the value of the modules processing time.
    *  @return The value of the modules processing time.
    */
   virtual uint32 getProcessTime() const;

   /**
    *   Returns the username of the sender of the stats.
    */
   const char* getUserName() const;
   
   /**
    *   Returns 1 if the jobthread is working.
    */
   int jobThreadWorking() const {
      return m_jobThreadWorking;
   }
   
   /**
    *    Changes status for the map in question.
    *    If status is loaded, loading etc. a new map will be added.
    *    If status is deleted the map will be removed.
    *    @return False if the map id isn't present.
    */
   virtual bool setStatus(uint32 mapID, MapElement::status_t status);

   /**
    *    Updates last use of the map.
    */
   virtual void updateLastUse( uint32 mapID );

   /**
    *    Prints the statistics on the stream.
    */
   friend ostream& operator<<(ostream& stream, const MapStatistics& stats);
   
   /**
    *    Prints all the maps with the requested status on the stream.
    *    @param allowedStatus The allowed status to print.
    *    @param prefix        Prefix to print before the maps if any.
    *    @param postfix       Stuff to print after the maps if any.
    */
   ostream& printAllWithStatus(ostream& stream,
                               MapElement::status_t allowedStatus,
                               const char* prefix,
                               const char* postfix) const;

   /**
    *    Prints the map id of the map element if the status == allowedStatus.
    * 
    */
   ostream& printWithStatus(ostream& stream,
                            const MapElement& me,
                            const char* prefix,
                            MapElement::status_t allowedStatus,
                            bool& first) const;

   /**
    *    Returns the oldest map and the age of that.
    *    MAX_UINT32, MAX_UINT32 denotes no maps in module.
    *    @param avoid Maps not to count.
    *    @param pretendNew Pretend as if the forbidden maps are really new.
    */
   pair<uint32, uint32> getOldestMapAndAge( set<uint32>& avoid,
                                            bool pretendNew = false ) const;

   /**
    * Operator +=.
    */
   MapStatistics& operator += ( const MapStatistics &o );
      
protected:

   /**
    *  Saves the vector so that it can be loaded as a MapStatistics.
    */
   int save(Packet* p, int& pos,
            uint64 optMem, uint64 maxMem,
            int queueLength,
            int32 rank) const;

   /**
    *   Add a MapElement to the MapSafeVector.
    *   @param mapID The ID of the map.
    *   @param size The size of the map.
    *   @return False if that map already existed in the vector.
    */
   bool addMap(uint32 mapID, uint32 size);
    
   /**
    *   Performs a linearsearch for an MapElement with ID == MapID.
    *   @param mapID The ID of the map searched for.
    *   @return  Pointer to the element. NULL if no such element.
    */
   MapElement* linearSearch(uint32 mapID);
   
protected:
   
   /// The average time it takes to process a packet.
   uint32 m_processTime;
   
   /// Optimal memory
   uint64 m_optMem;
   
   /// Maximum memory
   uint64 m_maxMem;
   
   /// Length of queue
   int m_queueLength;

   /// Indicates that the jobthread is proceccing a request
   int m_jobThreadWorking;

   /// User name or NULL
   MC2String m_userName;

   /// Load average 1 minute
   float32 m_loadAvg_1;
   /// Load average 5 minutes
   float32 m_loadAvg_5;
   /// Load average 15 minutes
   float32 m_loadAvg_15;

   /// Last time we calced the load
   uint32 m_lastLoadTime;

};


#endif

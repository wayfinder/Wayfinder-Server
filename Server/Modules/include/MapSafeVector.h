/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPSAFEVECTOR_H
#define MAPSAFEVECTOR_H

#include "config.h"
#include <set>
#include <vector>
#include <map>

#include "MapStatistics.h"
#include "ISABThread.h"
#include "IPnPort.h"
#include "PointerFifoTemplate.h"

class PushService;
class PushServices;
class SubscriptionResource;
class Packet;

/**
 *    Thread safe and somewhat modified version of MapStatistics.
 *    Used for communication between the JobThread and Reader.
 */
class MapSafeVector : protected MapStatistics {
public:
   typedef PointerFifoTemplate<Packet> ReaderQueue;
   MapSafeVector();

   /**
    *  Saves the vector so that it can be loaded as a MapStatistics.
    */
   int saveAsMapStats(Packet* p, int& pos,
                      uint64 optMem, uint64 maxMem,
                      int queueLength,
                      int32 rank) const;
   
   /**
    *    Changes status for the map in question.
    *    If status is loaded, loading etc. a new map will be added.
    *    If status is deleted the map will be removed.
    *    @return False if the map id isn't present.
    */
   bool setStatus(uint32 mapID, MapElement::status_t status);
   
   /**
    *  @param mapID The map id.
    *  @param size The size of the map that was loaded.
    *  @return False if the map was not found.
    */
   bool finishLoadingMap(uint32 mapID, uint32 mapSize);

   /**
    *    Returns true if the map is marked as loaded.
    *    @param mapID The mapID to check.
    */
   bool isMapLoaded( uint32 mapID );
   
   /**
    *    Returns true if the map is loaded or loading.
    *    @param mapID The map id to load.
    *    @return True if the map is on the way.
    */
   bool isMapLoadedOrLoading(uint32 mapID);

   /**
    *    Updates the last use of the map to now.
    */
   void updateLastUse( uint32 mapID );
   
   /**
    *   Puts the loaded resources into the vector <code>res</code>.
    *   The returned resources should be deleted by the caller.
    *   @param available vector to put the loaded resources into.
    *   @param deleted   vector to put the deleted resources in.
    */
   void getResources(vector<SubscriptionResource*>& available,
                     vector<SubscriptionResource*>& deleted);

   /**
    *   Tells the (Reader) that a push resource was
    *   added or removed.
    *   @return true if there was a reader fifo set.
    */
   bool pushResourcesChanged();

   /**
    *  Mark the jobthread as working and store the current time.
    */
   void setJobThreadStart();
   
   /**
    *  Mark the jobthread as NOT working.
    */
   void setJobThreadEnd();
   
   /**
    *  Returns the current job of the jobthread if the
    *  JobThread has been busy for too long.
    *  @return -1 if the JobThread is ok.
    */ 
   int jobThreadTimeOut();
   
   /**
    *  Returns the current job of the jobthread if the
    *  JobThread has been busy for too long.
    *  @return -1 if the JobThread is ok.
    */ 
   int jobThreadCrashTimeOut();
   
   /**
    *   Sets the fifo of the reader. Ugly way to wake
    *   up the reader from sleeping.
    */
   void setReaderFifo(ReaderQueue* fifo);
   
   /**
    *  Set a new process time in the vector.
    *  @param pTime The new process time.
    */
   void setProcessTime(uint32 pTime)
      { m_processTime = pTime;}

   /**
    *   Returns the process time. Also adds time for the processor.
    */
   uint32 getProcessTime() const;   
   
   /**
    *  Update the jobThread time-out time to the curren.
    *  (Used when jT will have to wait awhile, like when recieving an
    *   ack - from mapModule).
    */
   void jobThreadIsAlive();

   /**
    *   @param  mapID The ID of the map to remove from the Vector.
    *   @return True if the element is removed, false otherwise.
    */
   bool removeMap(uint32 mapID);

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

   /// sets address
   void setAddr(const IPnPort& addr) { 
      m_addr = addr; 
   }
   /// @return address
   const IPnPort& getAddr() const { 
      return m_addr; 
   }
   /// sets leader addr
   void setLeaderAddr( const IPnPort& addr ) { 
      m_leaderAddr = addr; 
   }
   const IPnPort& getLeaderAddr() const { 
      return m_leaderAddr;
   }
private:

   /**
    *   Calculates the load.
    *   @param timeSinceLastLoad The time that has elapsed since the
    *                            last update.
    *   @param nbrMinutes        The number of minutes of the load.
    *   @param lastLoad          Last value of the load.
    *   @param queueLength       The current load.
    *   @return The new average.
    */
   static double calcLoad( double timeSinceLastLoad,
                           uint32 nbrMinutes,
                           double lastLoad,
                           uint32 queueLength );
   /// Updates the load
   void updateLoad( uint32 queueLength );
   
   /**
    * The monitor used for synchronization.
    */
   ISABMonitor m_monitor;

   /// The Readers fifo or NULL.
   ReaderQueue* m_readerFifo;

   /// Set of deleted maps. For use when subscribing
   set<uint32> m_deletedMaps;
      
   /// If jobThread is working this is the time the work started.
   uint32 m_jobStartTime;

   /// Time before the job thread should timeout in milliseconds.
   uint32 m_jobThreadCrashTimeout;

   /// Current job counter. For use when checking the jobthread
   int m_currentJob;

   /// Total time of all jobs
   uint32 m_totalTime;
   IPnPort m_leaderAddr; //< holds leader address

   IPnPort m_addr; //< holds this modules address

};

#endif  // MAPSAFEVECTOR_H

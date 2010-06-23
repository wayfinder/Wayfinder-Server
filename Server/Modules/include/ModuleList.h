/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULELIST_H
#define MODULELIST_H

#include "config.h"
#include "IPnPort.h"

#include <iostream>
#include <set>
#include <vector>

class MapStatistics;
class MapElement;
class StatisticsPacket;    // Forward declaration
class Packet;

/**
  *   Describes statistics information about a module. Also
  *   contains the maps this module has loaded in RAM.
  *   Should inherit from MapStatistics or something.
  */
class ModuleNotice
{
public:
      /**
       *  Creates a new ModuleNotice and initiates the values to
       *  those in the packet send as parameter.
       */
      ModuleNotice(StatisticsPacket* sp);

      /**
       *  Destructor.
       */
      virtual ~ModuleNotice();

      /**
       *   Returns the MapStatistics for the notice.
       */
      const MapStatistics& getStats() const;

      /**
       * Return the IP-address of the corresponding module.
       * @return  The IP-address of the corresponding module.
       */
      uint32 getIP() const;

      /**
       * Return the port the cortresponding module listens at.
       * @return The port the cortresponding module listens at.
       */
      uint16 getPort() const;

      /**
       *   Returns the IPnPort for the ModuleNotice.
       */
      inline const IPnPort& getAddr() const { return m_ipAndPort; };
      
      /**
       *  Returns the time when the statistics was last updated.
       *  @return The time when the statistics was last updated.
       */
      uint32 getTimeForLastStatistics() const;

      /**
       *  Returns the average processingtime of the module.
       *  If the processing time is zero the function will return
       *  one to avoid giving all packets to a module with a hung
       *  processor.
       *  @return The average processingtime.
       */
      uint32 getProcessTime() const;

      uint32 getStatistics() const
         { return getProcessTime(); }

      /**
       *    Returns the number of packets in the queue of the
       *    module from last statistics.
       */
      uint32 getPacketsInQueue() const;

      /**
       *    Returns the number of packets sent to the module since
       *    last statistics.
       */
      uint32 getRequests() const {
         return m_recentNbrOfReq;
      }

      
      /**
       *    Marks the map as being deleted.
       */
      void startDeletingMap(uint32 mapID);
      
      /**
       *  Mark the map with the given ID as being loading.
       */ 
      void markLoadingMap(uint32 mapID);

      /**
       *    Add the map as being completely loaded.
       */
      void addLoadedMap(uint32 mapID, uint32 size);

      /**
       *  Get the amount of memory the module uses in excess of 
       *  the optimal value.
       *  @return Memory used above optimal value (can be negative)
       */
      int64 getXSMem() const;

      /**
       *   Returns the amount of memory used in the module.
       */
      uint64 getUsedMem() const;
      
      /**
       *  Get the amount of memory that this module is alowed to use.
       *  @return Max memory allowed for this module.
       */
      uint64 getMaxMem() const;

      /**
       *  Returns the optimum memory use.
       */
      uint64 getOptMem() const;
      

      /**
       *  The rank this module has. (In case of the leader terminating.)
       *  @return Module rank.
       */
      uint32 getRank() const;

      /**
       *  Returns true if this notice corresponds to the leader-module.
       *  @return True if leader.
       */
      bool isLeader() const;

      /**
       *   Set if the notice is the leader.
       */
      void setLeader(bool leader);

      /**
       *   Returns true if the module is loading a map.
       */
      bool isLoading() const;
      
      /**
        *   Updates the statistics (the stat-value and the loaded maps).
        *   Overridden in SMSModuleNotice.
        */
      virtual void updateData(StatisticsPacket* sp);

      /**
        *  
        *   @return  True if module has map with id mapID loaded, false
        *            otherwise.
        */
      bool isMapLoaded(uint32 mapID) const;

      /**
       *    Returns true if the map is about to be loaded.
       */
      bool isMapLoading( uint32 mapID ) const;
      
      /**
       *    Returns true if the module is loading or has loaded the
       *    map.
       */
      bool isMapLoadedOrLoading(uint32 mapID) const;
      
      /**
        *   Method for debugging.
        */
      void printLoadedMaps(ostream& stream);

      /**
       *  Returns the number of maps loaded by the module.
       *  @return The number of maps loaded by the module.
       */
      uint32 getNbrMaps() const;

      /**
       *   Add a packet to the m_xtraPackets field.
       *   Also update last use of the map.
       *   @param mapID MapID of the map used or MAX_UINT32 if no map.
       */
      void addPacketToQueue( uint32 mapID );

      void resetRequests() {
         m_recentNbrOfReq = 0;
      }

      float getLoad1() const;
      
      /**
       *  Checks which of the provided maps are present in this module.
       *  @param The maps to check against. The maps that was NOT present
       *         will be added to this set.
       *  @param A set with the ids of the found double maps.
       *  @return Nbr of found maps       
       */
      int checkIfMapsPresent(set<uint32> &presentMaps, set<uint32> &doubles);

      /// Returns true if the module is deleting a map.
      bool isDeleting() const;
      
   protected:

      /**
       *    Ip and port of the module.
       */
      IPnPort m_ipAndPort;

      /**
       *    Map statistics from the Modules.
       */
      MapStatistics* m_moduleMapStats;
      
      /**
        *   Time for last update of statistics and loadedMaps.
        */
      uint32 m_timeForStatistics;

      /// Number of request sent to this module since last reset.
      uint32 m_recentNbrOfReq;
      
      /// Total number of request sent to this module:
      uint32 m_totalNbrOfReq;

      /// Used memory in the module.
      uint64 m_usedMem;

      /// True if it is the leader notice.
      bool m_isLeader;
};


/**
  *   A list of all known modules (only used by the leader).
  */
class ModuleList : public vector<ModuleNotice*>
{
public:

   virtual ~ModuleList() {}
  
   /**
    *   Creates a new list.
    */  
   ModuleList();
   
   /**
    *   @param   ip IP-address of the wanted module.
    *   @param   port Port to the wanted module.
    *   @return  Pointer to the ModuleNotice that has the
    *            specified values on IP and port.
    */
   ModuleNotice* findModule(uint32 ip, uint16 port);

   /**
    *   Find the module matching the supplied addresss.
    *   @param addr IP and port of the module to find.
    *   @return Found module or NULL.
    */
   ModuleNotice* findModule(const IPnPort& addr);
   
   /**
    *   Returns true if all modules are loading.
    */
   bool allModulesLoading() const;
   
   /**
    *   Returns a pointer to the best module with the
    *   supplied mapid loaded. Or NULL.
    *   @param mapID Map to look for.
    */
   ModuleNotice* getBestModuleWithMapLoaded(uint32 mapID);
   
   /**
    *   Returns a pointer to the best module currently
    *   loading the map with the supplied id or NULL.
    *   @param mapID Map to look for.
    */
   ModuleNotice* getBestModuleLoadingMap(uint32 mapID);

   /**
    *   Counts the number of modules that has loaded or
    *   are loading the map.
    *   @param mapID Map id to look for.
    *   @return Number of modules having the map.
    */
   int countModulesWithMap( uint32 mapID ) const;

   /**
    *   Returns the ModuleNotice for the module
    *   with the oldest map and the id of the oldest
    *   map.
    *   @param avoid Maps not to count.
    */
   pair<ModuleNotice*, uint32> getOldestMap( set<uint32>& avoid );
   
   /**
    *   Returns a pointer to the module using the least memory.
    */
   ModuleNotice* getModuleUsingLeastMem();
   
   /**
    *   Returns a pointer to the module using the least memory
    *   and prefer then ones that are not loading maps.
    */
   ModuleNotice* getModuleUsingLeastMemPrefNotLoading();
   
   /**
    *   Returns a pointer to the module using the most memory.
    */
   ModuleNotice* getModuleUsingMostMem();

   /**
    *   Returns pointers to all modules. The one using most
    *   memory is first.
    */
   void getModulesMostMemFirst( vector<ModuleNotice*>& mods ) const;
   
   /**
    *   Removes ModuleNotices that hasn't reported statiistics
    *   in MAX_TIME_NO_STATISTICS ms. They are assumed to be
    *   down.
    */
   void checkTimeForStatistics();

   /**
    *  Returns the notice of the module with the lowest response time.
    *  @return The notice of the module with the lowest response time.
    */
   virtual ModuleNotice* getBestModule(Packet *p);
   
   
   /**
    *   Method used for debug-printing.
    *   @param moduleName Name of module for easier identification.
    *   @param maps If this is true (default) the maps loaded by
    *               each module will be listed.
    */
   virtual void dumpList(const char* moduleName,
                         bool maps = true);
   
   
   void resetNumberOfRequests();
   
   /**
    *   Removes the worst modules from a candidate list.
    *   The worst modules are modules with a load of at least 10
    *   that have a load at least 10 times that of the one with
    *   the lowest load.
    *   @param storage Vector that could be used to store the notices.
    *   @param in_notices Vector to get the notices from.
    *   @return Reference to storage or in_notices.
    */   
   const vector<ModuleNotice*>&
      removeWorst( vector<ModuleNotice*>& storage,
                   const vector<ModuleNotice*>& in_notices ) const;

   /**
    *   Sets if the (experimental) map balancing should be used.
    */
   void setUseMapBalancing( bool use ) {
      m_useMapBalancing = use;
      if ( use ) {
         mc2dbg << info << "[ModuleList]: Will use map balancing" << endl;
      }
   }
   
protected:
   /**
    *   Returns the best module of the modules listed in
    *   the vector.
    */
   static ModuleNotice* getBestModule( const vector<ModuleNotice*>& notices );
      
   /// True if map balancing should be tried
   bool m_useMapBalancing;
};

#endif // MODULELIST_H


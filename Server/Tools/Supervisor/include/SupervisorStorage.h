/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUPERVISOR_STORAGE_H
#define SUPERVISOR_STORAGE_H

#include "config.h"

#include <vector>
#include <algorithm>

#include "MC2String.h"
#include "ObjVector.h"
#include "Packet.h"
#include "multicast.h"
#include "IPnPort.h"
#include "Server.h"
#include "MapStatistics.h"
#include "STLStringUtility.h"
#include "STLStrComp.h"
#include "TimeUtility.h"

#define LOADEDMAPS_STRING_SIZE 255

class SupervisorDisplay;
class DatagramReceiver;
class StatisticsPacket;

/**
 *  Information about a running module
 */
class ModuleInfo {
  public:
   /**
    *   Create a new ModuleInfo object.
    *   @param p Packet to use.
    *   @param sock is the socket that recieved the Packet.
    */
   ModuleInfo( Packet* p, DatagramReceiver* sock,
               bool dont_show_loaded_mapsids );

   ModuleInfo();
   
   /** @return The hostname as a string. Inlined function. */
   const char* getHostName() const {
      return m_hostName.c_str();
   }
   
   /** @return A string representing the module type */
   const char* getModuleTypeAsString() const;

   /** @return A string representing id of the loaded maps */
   const char* getLoadedMapsAsString() const;

   /** @return A string representing statistics of the maps */
   const char* getStatisticsAsString() const;

   /** @return The user of the other module */
   const char* getUserName() const;

   /** @return The percentage of allowed memory used */
   int getPercentMemUse() const;


   /**
    * @return The queue length of the module.
    */
   uint32 getQueueLength() const;


   /**
    * @return The process time of the module.
    */
   uint32 getProcessTime() const;


   /**
    * Returns the 1 minute load average.
    */
   float getLoad1() const { return m_mapStats.getLoad1(); }


   /**
    * Returns the 5 minute load average.
    */
   float getLoad5() const { return m_mapStats.getLoad5(); }


   /**
    * Returns the 15 minute load average.
    */
   float getLoad15() const { return m_mapStats.getLoad15(); }


   /** @return The last time this module sent a heartbeat or statistic */
   uint32 getTimeStamp() const {
      return timeStamp;
   }

   /** Timestamp this packet. Inlined.
    *  @param ts Current time in millis.
    */
   void setTimeStamp(uint32 ts) {
      timeStamp = ts;
   }

   /** Timestamp this packet with current time. Inlined. */
   void setTimeStamp() {
      timeStamp = TimeUtility::getCurrentTime();
   }


   /**
    * Get if jobThreadWorking.
    */
   int getJobThreadWorking() const { 
      return m_mapStats.jobThreadWorking(); }


   /** @return True if the packet matches this module. Inlined. */
   bool matchesPacket(Packet *p, DatagramReceiver* sock);

   /** @return True if the module has not been sending out packets
    *          for a while.
    */
   bool tooOld();

   /**
    *    Sets time for last heartbeat.
    */
   void heartbeatRecv();


   /**
    * Sets dont_show_loaded_mapsids
    */
   void set_dont_show_loaded_mapsids( bool val );


   /// Checks if ModuleTypeAsString is same.
   bool operator == ( const ModuleInfo& o ) const;

   /// Checks if ModuleTypeAsString is not same.
   bool operator != ( const ModuleInfo& o ) const;

   /// Adds others info to this.
   ModuleInfo& operator += ( const ModuleInfo& o );


   /// Checks if this matches any of the entries in s.
   typedef set< MC2String, strNoCaseCompareLess > CollateSet;
   bool collateMatches( const CollateSet& s ) const;
   

   void updateStatStrs();

   /// Get the time of creation of notice
   time_t getCreateTime() const;

  protected:

   const char* getModuleTypePrefix() const;
   
   uint32 moduleType;
   uint32 timeStamp;
   uint32 waitTime;
   uint32 m_lastHeartBeatTime;
   time_t m_createTime;

   /// Origin address for the module
   IPnPort m_originAddr;

   MC2String m_statisticsValue;
   MC2String m_hostName;
   MC2String m_loadedMaps;
   MC2String m_moduleTypeAsString;

   uint32 m_percentMemUse;

   void setLoadedMaps(StatisticsPacket* sp);

   uint16 getAvailablePort(uint16 port );
   friend class ModuleSorter;
   /// If dont show loaded mapsids.
   bool m_dont_show_loaded_mapsids;


   /// The extra module type string
   MC2String extraModuleType;


   MapStatistics m_mapStats;
};


class ModuleSorter {
  public:
   bool operator()(const ModuleInfo& a, const ModuleInfo& b) const {
      if ( a.extraModuleType < b.extraModuleType ) {
         return true;
      }
      if ( a.extraModuleType > b.extraModuleType ) {
         return false;
      }
      if ( a.moduleType < b.moduleType ) {
         return true;
      }
      if ( a.moduleType > b.moduleType ) {
         return false;
      }
      return a.m_originAddr < b.m_originAddr;
   }
   
};

/**
 *    This class contains information about the running modules
 *    and sends updates to SupervisoDisplay objects that have
 *    been added with addObserver.
 *    @see SupervisorDisplay.
 */
class SupervisorStorage {
  public:
   /**
    * The module types.
    */
   static const moduletype_t modules[];

   /** Contructs a new Storage with an empty list. */
   SupervisorStorage( bool dont_show_loaded_mapsids );

   /** Deletes the storage. */
   virtual ~SupervisorStorage();

   /**
    *   Add an observer to the storage
    */
   void addObserver(SupervisorDisplay* s);
   
   /**
    *   Tell all observers that something has happened.
    */
   void notify();
   /**
    *   Handle an incoming packet.
    */
   void handlePacket(Packet *p, DatagramReceiver* sock);

   /**
    *   @return A pointer to the list of running modules.
    */
   void getRunningModules(vector<ModuleInfo>& modulesRunning) {
      modulesRunning = m_modulesRunning;
      std::sort( modulesRunning.begin(), modulesRunning.end(),
                 ModuleSorter());      
   }


   /**
    * Flips dont_show_loaded_mapsids.
    */
   void flip_set_dont_show_loaded_mapsids();

   
  private:
   typedef vector < SupervisorDisplay* > SupervisorDisplayVector;
   
   SupervisorDisplayVector m_observers;
   
   /// Storage of modules.
   vector<ModuleInfo> m_modulesRunning;


   /// If dont show loaded mapsids.
   bool m_dont_show_loaded_mapsids;
};

#endif // SUPERVISOR_STORAGE_H


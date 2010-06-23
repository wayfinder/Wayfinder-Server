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

#include "Packet.h"
#include "PushServices.h"
#include "SubscriptionResource.h"
#include "Fifo.h"
#include "Properties.h"
#include "MapSafeVector.h"
#include "StringUtility.h"
#include "TimeUtility.h"

#define JOBTHREAD_HIDE_TO_MS   (2*60*1000)

MapSafeVector::MapSafeVector():
   m_jobThreadCrashTimeout( Properties::
                            getUint32Property( "JOBTHREAD_TIMEOUT",
                                               600 ) * 1000 ),
   m_addr(0, 0)
{
   m_readerFifo       = NULL;
   m_currentJob       = 0;
   m_totalTime        = 0;
   m_jobStartTime     = TimeUtility::getCurrentTime();
   m_userName         = Properties::getProperty( "USER" );
   m_lastLoadTime     = TimeUtility::getCurrentTime();
}

ostream&
MapStatistics::printWithStatus(ostream& stream,
                               const MapElement& me,
                               const char* prefix,
                               MapElement::status_t allowedStatus,
                               bool& first) const
{
   const char* separator = " ";
   if ( me.getStatus() == allowedStatus ) {
      if ( ! first ) {
         stream << separator;
      } else {
         stream << prefix;
      }
      stream << hex << me.getMapID() << dec;
      first = false;
   }
   return stream;
}

ostream&
MapStatistics::printAllWithStatus(ostream& stream,
                                  MapElement::status_t allowedStatus,
                                  const char* prefix,
                                  const char* postfix) const
{   
   bool first = true;
   // Print the loaded maps
   for( MapStatistics::const_iterator it = begin();
        it != end();
        ++it ) {
      printWithStatus(stream, it->second, prefix,
                      allowedStatus, first);
   }
   if ( first == false ) {
      stream << postfix;
   }
   return stream;
}

ostream&
operator<<(ostream& stream, const MapStatistics& stats)
{
   // Print the maps that are loaded
   stats.printAllWithStatus(stream, MapElement::LOADED,
                            "", " ");
    // Print the maps that are TOLD_TO_LOAD
   stats.printAllWithStatus(stream, MapElement::TOLD_TO_LOAD,
                            "(told to ld ", ") ");
   // Print the maps that are loading
   stats.printAllWithStatus(stream, MapElement::LOADING,
                            "(loading ", ") ");
   // Print the maps that are TOLD_TO_DELETE
   stats.printAllWithStatus(stream, MapElement::TOLD_TO_DELETE,
                            "(told to delete ", ") ");
   // Print the maps that are TOLD_TO_DELETING
   stats.printAllWithStatus(stream, MapElement::DELETING,
                            "(deleting ", ")");
   
   
   return stream;
}

// - MapSafeVector ----------------------------------------

bool
MapSafeVector::removeMap(uint32 mapID)
{
   ISABSync sync(m_monitor);

   if ( MapStatistics::removeMap(mapID) ) {
      if ( m_readerFifo != NULL ) {
         m_deletedMaps.insert(mapID);
         pushResourcesChanged();
      }
      return true;
   } else {
      return false;
   }
}

bool
MapSafeVector::finishLoadingMap(uint32 mapID,
                                uint32 size)
{
   ISABSync synchronized(m_monitor);
   if ( MapStatistics::finishLoadingMap(mapID, size) ) {
      updateLastUse( mapID );
      pushResourcesChanged();
      return true;
   } else {
      return false;
   }
}


int
MapSafeVector::jobThreadTimeOut()
{
   ISABSync synchronized(m_monitor);

   if ( ! m_jobThreadWorking ) {
      return -1;
   }
   
   if ( ((TimeUtility::getCurrentTime() - m_jobStartTime) >
         JOBTHREAD_HIDE_TO_MS ) ) {
      return m_currentJob;
   } else {
      return -1;
   }   
}


uint32
MapSafeVector::getProcessTime() const
{
   uint32 procTime = 0;
   // Just divide the total time with the number of jobs.
   if ( m_jobThreadWorking ) {
       procTime =
          ( m_totalTime + (TimeUtility::getCurrentTime() - m_jobStartTime) )
         / (m_currentJob+1);
   } else {      
      procTime = m_totalTime / (m_currentJob+1);
   }
   return procTime;
}


int
MapSafeVector::jobThreadCrashTimeOut()
{
   ISABSync synchronized(m_monitor);
   if ( ! m_jobThreadWorking ) {
      return -1;
   }
   
   if ( ((TimeUtility::getCurrentTime() - m_jobStartTime) >
         m_jobThreadCrashTimeout ) ) {
      return m_currentJob;
   } else {
      return -1;
   }   
}

void
MapSafeVector::setJobThreadStart() 
{
   ISABSync synchronized(m_monitor);
   m_jobThreadWorking = true;
   m_jobStartTime     = TimeUtility::getCurrentTime();
}

void
MapSafeVector::jobThreadIsAlive()
{
   setJobThreadStart();
}
   
void
MapSafeVector::setJobThreadEnd()
{
   ISABSync synchronized(m_monitor);
   m_jobThreadWorking = false;
   ++m_currentJob;
   if ( m_currentJob == -1 ) {
      ++m_currentJob;
   }
   m_totalTime += (TimeUtility::getCurrentTime() - m_jobStartTime);
}


bool
MapSafeVector::isMapLoaded(uint32 mapID)
{
   ISABSync sync(m_monitor);
   return MapStatistics::isMapLoaded(mapID);
}


bool
MapSafeVector::isMapLoadedOrLoading(uint32 mapID)
{
   ISABSync sync(m_monitor);
   return MapStatistics::isMapLoadedOrLoading(mapID);
}

bool
MapSafeVector::setStatus(uint32 mapID, MapElement::status_t status)
{
   ISABSync sync(m_monitor);
   return MapStatistics::setStatus(mapID, status);
}

void
MapSafeVector::setReaderFifo(ReaderQueue* fifo)
{
   ISABSync syncronized(m_monitor);
   m_readerFifo = fifo;
}

bool
MapSafeVector::pushResourcesChanged()
{
   // ISABSync should be created in calling function.
   if ( m_readerFifo == NULL ) {
      return false;
   } else {
      mc2dbg << "[MSV] enqueing null" << endl;
      m_readerFifo->enqueue(NULL);
      return true;
   }
}

void
MapSafeVector::getResources(vector<SubscriptionResource*>& available,
                            vector<SubscriptionResource*>& deleted)
{
   ISABSync synchronized(m_monitor);

   // Add the available maps
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      const MapElement* me = &(it->second);
      if ( me->getStatus() == MapElement::LOADED ) {
         // We're not loading this map
         // We must remove the map from the deleted maps.
         m_deletedMaps.erase( me->getMapID() );
         // Add it to the available maps.
         available.push_back(new MapSubscriptionResource(me->getMapID()));
      }
   }
   
   // Deleted since we checked last time
   for( set<uint32>::iterator it = m_deletedMaps.begin();
        it != m_deletedMaps.end();
        ++it ) {
      deleted.push_back(new MapSubscriptionResource(*it));
   }
   m_deletedMaps.clear();
}

int
MapSafeVector::getLoadedMaps(set<uint32>& maps)
{
   ISABSync sync(m_monitor);
   return MapStatistics::getLoadedMaps(maps);
}

int
MapSafeVector::getAllMapInfo(set<MapElement>& maps)
{
   ISABSync sync(m_monitor);
   return MapStatistics::getAllMapInfo(maps);
}

double
MapSafeVector::calcLoad( double timeSinceLastLoad,
                         uint32 nbrMinutes,
                         double lastLoad,
                         uint32 queueLength )
{
   double exp_val = exp( -timeSinceLastLoad / (nbrMinutes*60.0*1000) );
   return lastLoad * exp_val + queueLength * ( 1.0 - exp_val );
}

void
MapSafeVector::updateLoad( uint32 queueLength )
{
   double timeSinceLastLoad = TimeUtility::getCurrentTime() - m_lastLoadTime;
   m_lastLoadTime = TimeUtility::getCurrentTime();
   
   m_loadAvg_1 = calcLoad( timeSinceLastLoad,
                           1,
                           m_loadAvg_1,
                           queueLength );
   
   m_loadAvg_5 = calcLoad( timeSinceLastLoad,
                           5,
                           m_loadAvg_5,
                           queueLength );

   m_loadAvg_15 = calcLoad( timeSinceLastLoad,
                            15,
                            m_loadAvg_15,
                            queueLength );   
}

int
MapSafeVector::saveAsMapStats(Packet* p, int& pos,
                              uint64 optMem, uint64 maxMem,
                              int queueLength,
                              int32 rank) const
{
   ISABSync sync(m_monitor);
   // Update the load.
   const_cast<MapSafeVector*>(this)->
      updateLoad( queueLength + jobThreadWorking() );
   
   return MapStatistics::save(p, pos, optMem, maxMem, queueLength, rank);
}

void
MapSafeVector::updateLastUse( uint32 mapID )
{
   ISABSync sync( m_monitor );
   MapStatistics::updateLastUse( mapID );
}

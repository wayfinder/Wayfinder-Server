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

#include <set>
#include "Packet.h"
#include "NewStrDup.h"
#include "MapStatistics.h"

//  -----   MapElement -----------------

MapElement::MapElement(uint32 mapID, uint32 size)
{
   setMapID(mapID);
   setSize(size);
   m_status = LOADED;
   updateLastUse();
}


int
MapElement::save(Packet* p, int& pos ) const
{
   mc2dbg8 << "[ME]: m_mapID  = " << m_mapID
           << "      m_size   = " << m_size
           << "      m_status = " << int(m_status) << endl;
   int origPos = pos;
   p->incWriteLong(pos, m_mapID);
   p->incWriteLong(pos, m_size);
   p->incWriteLong(pos, m_status);

   // reserve room for length
   int length_pos = pos;
   p->incWriteLong( pos, 0 );

   uint32 curTime = TimeUtility::getCurrentTime();
   p->incWriteLong( pos,  curTime - m_lastUse );
   mc2dbg8 << "[ME]: MapID = "
           << MC2HEX( m_mapID ) << " last use = "
           << (curTime-m_lastUse) << endl;
   
   int tmp_length_pos = length_pos;
   p->incWriteLong( tmp_length_pos, pos - origPos );

   return pos - origPos;
}

int
MapElement::load(const Packet* p, int& pos )
{
   int origPos = pos;
   p->incReadLong(pos, m_mapID);
   p->incReadLong(pos, m_size);
   p->incReadLong(pos, m_status);

   int length = p->incReadLong( pos );
   p->incReadLong( pos, m_lastUse );
   // Convert to leader time.
   m_lastUse = TimeUtility::getCurrentTime() - m_lastUse;

   pos = origPos + length;
   return length;
}

bool
MapElement::isMapLoadedOrLoading() const
{
   switch ( getStatus() ) {
      case MapElement::TOLD_TO_LOAD:
      case MapElement::LOADING:
      case MapElement::LOADED:
         return true;
      default:
         return false;
   }
}

bool
MapElement::isMapLoaded() const
{
   return getStatus() == MapElement::LOADED;
}

void
MapElement::updateLastUse()
{
   m_lastUse = TimeUtility::getCurrentTime();
}

const char* MapElement::getStatusAsString() const {
   const char* status = NULL;
   
   switch ( getStatus() ) {
      case MapElement::TOLD_TO_LOAD:
         status = "TOLD_TO_LOAD";
         break;
      case MapElement::LOADING:
         status = "LOADING";
         break;
      case MapElement::LOADED:
         status = "LOADED";
         break;
      case MapElement::TOLD_TO_DELETE:
         status = "TOLD_TO_DELETE";
         break;
      case MapElement::DELETING:
         status = "DELETING";
         break;
      case MapElement::DELETED:
         status = "DELETED";
         break;
      default: // should not happen
         status = "UNKNOWN";
   }

   return status;
}

//  -----   MapStatistics -----------

MapStatistics::MapStatistics()
      : map<uint32, MapElement>()
{
   m_processTime      = 1;
   m_jobThreadWorking = 0;
   m_queueLength      = 0;
   m_optMem           = 0;
   m_maxMem           = 0;
   m_loadAvg_1        = 0;
   m_loadAvg_5        = 0;
   m_loadAvg_15       = 0;
}

MapStatistics::~MapStatistics() {
}

int
MapStatistics::save(Packet* p, int& pos,
                    uint64 optMem, uint64 maxMem,
                    int queueLength,
                    int32 rank) const
{
   // Don't forget to update MapStatistics::save
   int origPos = pos;

   p->incWriteLongLong(pos, optMem);
   p->incWriteLongLong(pos, maxMem);
   p->incWriteLong(pos, getProcessTime());
   p->incWriteLong(pos, queueLength);

   uint32 flags = m_jobThreadWorking ? 1 : 0;

   p->incWriteLong(pos, flags );
   p->incWriteString( pos, m_userName.c_str() );
   p->incAlignWriteLong( pos );

   p->incWriteLong(pos, size());
   
   // Write the length
   int length_pos = pos;
   p->incWriteLong( pos, 0 );
   
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      it->second.save(p, pos );
   }

   p->incWriteFloat32( pos, m_loadAvg_1);
   p->incWriteFloat32( pos, m_loadAvg_5);
   p->incWriteFloat32( pos, m_loadAvg_15);
   
   int tmp_length_pos = length_pos;
   p->incWriteLong( tmp_length_pos, pos - origPos );
   
   p->setLength(pos);
   return pos - origPos;
}

int
MapStatistics::save(Packet* p, int& pos) const
{
   return save(p, pos, m_optMem, m_maxMem, m_queueLength, 0);
}

int
MapStatistics::load(const Packet* p, int& pos)
{
   // Empty the map elements.
   clear();
   int origPos = pos;

   m_optMem = p->incReadLongLong( pos );
   m_maxMem = p->incReadLongLong( pos );   
   p->incReadLong(pos, m_processTime);
   p->incReadLong(pos, m_queueLength);
   uint32 wasJobThreadWorking = p->incReadLong(pos);
   // This one is fun. It is used in some calculations.
   m_jobThreadWorking = wasJobThreadWorking & 0x01;
   m_userName = p->incReadString( pos );
   int nbrMaps = p->incReadLong(pos);
   
   int length = 0;
   p->incReadLong(pos, length);

   for( int i = 0; i < nbrMaps; ++i ) {
      MapElement me(0,0);
      me.load(p, pos );
      insert(make_pair(me.getMapID(), me) );
   }
   
   mc2dbg8 << "[MS]: Length of stats = " << length << endl;
   m_loadAvg_1  = p->incReadFloat32( pos );
   m_loadAvg_5  = p->incReadFloat32( pos );
   m_loadAvg_15 = p->incReadFloat32( pos );      
   mc2dbg8 << "[MS]: Loads are "
           << m_loadAvg_1 << " "
           << m_loadAvg_5 << " "
           << m_loadAvg_15 << endl;
   pos = origPos + length;

   return pos - origPos;
}

void
MapStatistics::setMaxMem( uint64 maxMem ) {
   m_maxMem = maxMem;
}

void MapStatistics::setOptMem( uint64 optMem ) {
   m_optMem = optMem;
}

const char*
MapStatistics::getUserName() const {
   return m_userName.c_str();
}


int
MapStatistics::getLoadedMaps(set<uint32>& maps)
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      const MapElement* me = &(it->second);
      if ( me->getStatus() == MapElement::LOADED ) {
         // We're not loading this map
         maps.insert(me->getMapID());
      } 
   }
   return maps.size();
}

int
MapStatistics::getAllMapInfo(set<MapElement>& maps)
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {      
      maps.insert(it->second);
   }
   return size();
}

uint64
MapStatistics::calcUsedMem() const
{
   uint64 usedMem = 0;
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      // Add the size if the map is loaded or loading only.
      if ( it->second.isMapLoadedOrLoading() ) {
         usedMem += it->second.getMapSize();        
      }
   }
   return usedMem;
}

bool
MapStatistics::moduleIsLoading() const
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      if ( it->second.isLoading() ) {
         return true;
      }
   }
   return false;
}


bool
MapStatistics::moduleIsDeleting() const
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      if ( it->second.isDeleting() ) {
         return true;
      }
   }
   return false;
}

bool
MapStatistics::addMap(uint32 mapID, uint32 size)
{
   pair<iterator, bool> res =
      insert(make_pair( mapID, MapElement(mapID, size ) ) );

   if ( res.second ) {
      mc2dbg4 << "[MapStatistics] Adding map " << mapID << endl;
   }
   
   return res.second;
}
  

bool
MapStatistics::finishLoadingMap(uint32 mapID,
                                uint32 size)
{
   // Warning! This function is re-implemented in MapSafeVector
   updateLastUse( mapID );
   if ( ! isMapLoaded(mapID ) ) {
      if ( ! setStatus(mapID, MapElement::LOADED) ) {
         // Could not find the map
         return false;
      }
      return setMapSize( mapID, size );
   } else {
      return false;
   }
}
 

bool
MapStatistics::isMapLoadedOrLoading(uint32 mapID) const
{
   const_iterator it = find( mapID );
   if ( it == end() ) {
      return false;
   } else {
      const MapElement* el = &it->second;
      return el->isMapLoadedOrLoading();
   }
}

bool
MapStatistics::isMapLoading(uint32 mapID) const
{
   const_iterator it = find( mapID );
   if ( it == end() ) {
      return false;
   } else {
      const MapElement* el = &it->second;
      return el->isLoading();
   }
}


bool
MapStatistics::isMapLoaded(uint32 mapID) const
{
   const MapElement* el = NULL;
   const_iterator it = find( mapID );
   if ( it != end() ) {
      el = &it->second;
   }
   if ( el == NULL ) {
      return false;
   }
   switch ( el->getStatus() ) {
      case MapElement::LOADED:
         return true;
      default:
         return false;
   }
}


bool
MapStatistics::removeMap(uint32 mapID)
{
   iterator it = find(mapID);
   if( it != end() ) {
      mc2dbg4 << "[MapStatistics]: Removing map " << mapID << endl;
      erase(it);
      return true;
   } else {
      return false;
   }
}

MapElement*
MapStatistics::linearSearch(uint32 mapID) 
{
   // This is not linear search anymore
   iterator it = find(mapID);
   if ( it != end() ) {
      return &(it->second);
   } else {
      return NULL;
   }
}

bool
MapStatistics::setStatus(uint32 mapID, MapElement::status_t status)
{
   // This function is used by the packet sender and Processor
   // PacketSender sets "TOLD_TO_LOAD" and "TOLD_TO_DELETE"
   // and Processor sets "LOADING", "LOADED", "DELETING" and
   // "DELETED".

   // Note that double load map-packets should not destroy map sizes.
      
   MapElement* el = linearSearch(mapID);
   if ( el == NULL ) {
      switch ( status ) {
         case MapElement::TOLD_TO_LOAD:
         case MapElement::LOADING:
         case MapElement::LOADED:
            // This is ok - create a new element
            addMap( mapID, 0 );
            return setStatus(mapID, status);
            break;
         default:
            return false;
      }
   }

   el->setStatus(status);
   
   if ( status == MapElement::TOLD_TO_LOAD ||
        status == MapElement::LOADING ) {
      // Guess the size of the map. (If it isn't set)
      if ( el->getSize() < 1  ) {
         if ( getNbrOfMaps() == 0 ) {
            setMapSize(mapID, 1);
         } else {
            setMapSize(mapID, calcUsedMem() / getNbrOfMaps());
         }
      }
   }
   if ( status == MapElement::DELETED ) {
      removeMap(mapID);
   }   
   return true;
}

bool
MapStatistics::setMapSize(uint32 mapID, uint32 size)
{
   MapElement* el = linearSearch(mapID);
   if ( el == NULL ) {
      return false;
   }
   el->setSize(size);
   return true;
}


uint32
MapStatistics::getProcessTime() const
{
   return m_processTime;
}

pair<uint32, uint32>
MapStatistics::getOldestMapAndAge( set<uint32>& avoid,
                                   bool pretendNew ) const
{
   pair<uint32,uint32> retVal(MAX_UINT32, MAX_UINT32);
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      // Unremove once.
      if ( it->second.isMapLoaded() ) {
         uint32 lastUse = it->second.getLastUse();
         if ( avoid.erase( it->second.getMapID() ) ) {
            // Pretend that the undeletable maps are very recently used.
            lastUse = TimeUtility::getCurrentTime();
            if ( ! pretendNew ) {
               // No - pretend it is not there.
               continue;
            }
         }
         if ( lastUse < retVal.second ) {
            retVal.first = it->second.getMapID();
            retVal.second = lastUse;
         }
      }
   }
   return retVal;
}


MapStatistics& 
MapStatistics::operator += ( const MapStatistics &o ) {
   for ( const_iterator it = o.begin() ; it != o.end() ; ++it ) {
      insert( *it );
   }
   m_processTime += o.m_processTime;
   m_optMem += o.m_optMem;
   m_maxMem += o.m_maxMem;
   m_queueLength += o.m_queueLength;
   m_jobThreadWorking += o.m_jobThreadWorking;
   if ( m_userName != o.m_userName ) {
      m_userName.append( "/" );
      m_userName += o.m_userName;
   }
   m_loadAvg_1 += o.m_loadAvg_1;
   m_loadAvg_5 += o.m_loadAvg_5;
   m_loadAvg_15 += o.m_loadAvg_15;
   m_lastLoadTime += o.m_lastLoadTime;

   return *this;
}

void
MapStatistics::updateLastUse( uint32 mapID )
{
   if ( mapID == MAX_UINT32 ) {
      return;
   }
   iterator it = find( mapID );
   if ( it != end() ) {
      it->second.updateLastUse();
   }
}

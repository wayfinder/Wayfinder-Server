/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include<algorithm>

#include "DataBuffer.h"
#include "Packet.h"

#include "ItemIDTree.h"
#include "MapBits.h"

bool
ItemIDTree::empty() const
{
   return m_byOverview.empty();
}

void
ItemIDTree::addMapOrItem( uint32 parentMapID,
                          uint32 mapID,
                          uint32 itemID )
{
   m_byOverview.insert (
      make_pair ( parentMapID, IDPair_t( mapID, itemID ) )
      );
}

int
ItemIDTree::getSizeInDataBuffer() const
{
   // This length should be long-aligned, but it already is.
   return m_byOverview.size() * 4 * 3 + 4 + 4;
}

bool
ItemIDTree::save(DataBuffer* buf) const
{
   buf->writeNextLong( getSizeInDataBuffer() );
   buf->writeNextLong(0); // Version
   buf->writeNextLong( m_byOverview.size() ); // Nbr of things.
   for ( overviewKeyed_t::const_iterator it = m_byOverview.begin();
         it != m_byOverview.end();
         ++it ) {
      buf->writeNextLong( it->first );
      buf->writeNextLong( it->second.getMapID() );
      buf->writeNextLong( it->second.getItemID() );      
   }
   return true;
}

bool
ItemIDTree::load(DataBuffer* buf, uint32 mapSet)
{
   int size = buf->readNextLong();
   int version = buf->readNextLong(); // Version.
   switch ( version ) {
      case 0: {
         int nbrItems = buf->readNextLong();
         for ( int i = 0 ; i < nbrItems; ++i ) {
            uint32 parentMapID = buf->readNextLong();
            uint32 mapID    = buf->readNextLong();
            if (mapSet != MAX_UINT32) {
               mc2dbg4 << "ItemIDTree::load(DataBuffer): mapSet: " << mapSet
                       << ", changing mapID from " << prettyMapIDFill(mapID);
               parentMapID = MapBits::getMapIDWithMapSet(parentMapID, mapSet);
               mapID = MapBits::getMapIDWithMapSet(mapID, mapSet);
               mc2dbg4  << " to " << prettyMapIDFill(mapID) << endl;
            }
            uint32 itemID   = buf->readNextLong();
            addMapOrItem( parentMapID, mapID, itemID );
         }
         return true;
      }
      default:
         mc2log << error << "ItemIDTree::load - unknown version "
                << version << " skipping." << endl;
         buf->readPastBytes( size - 4 ); // Version already read
         return false;
   }
   return false;
}

int
ItemIDTree::getSizeInPacket() const
{
   // This length should be long-aligned which it is.
   // NbrItems + nbritems * 4 bytes * 3 
   return 4 + m_byOverview.size() * 4 * 3;
}

bool
ItemIDTree::save( Packet* p,
                  int &pos ) const
{
   int sizeInPacket = getSizeInPacket();
   // Double the packet size if needed.
   p->updateSize( sizeInPacket, p->getBufSize() + sizeInPacket );

   p->incWriteLong(pos, m_byOverview.size() );
   for ( overviewKeyed_t::const_iterator it = m_byOverview.begin();
         it != m_byOverview.end();
         ++it ) {
      p->incWriteLong( pos, it->first );
      p->incWriteLong( pos, it->second.getMapID() );
      p->incWriteLong( pos, it->second.getItemID() );
   }
   return true;
}

bool
ItemIDTree::load( const Packet* p,
                  int &pos)
{
   int nbrItems = p->incReadLong(pos);
   for ( int i = 0 ; i < nbrItems; ++i ) {
      uint32 parentID = p->incReadLong(pos);
      uint32 mapID    = p->incReadLong(pos);
      uint32 itemID   = p->incReadLong(pos);
      addMapOrItem( parentID, mapID, itemID );
   }
   return true;
}


void
ItemIDTree::addMap( uint32 parentMapID, uint32 mapID)
{
   addMapOrItem( parentMapID, mapID, MAX_UINT32 );
}

void
ItemIDTree::addItem( uint32 parentMapID, uint32 itemID )
{
   addMapOrItem ( parentMapID, parentMapID, itemID );
}

void
ItemIDTree::getTopLevelMapIDs( set<uint32>& mapIDs ) const
{
   // Get the iterators for the range of maps with parent
   // id:s == MAX_UINT32
   pair<overviewKeyed_t::const_iterator,
        overviewKeyed_t::const_iterator> range =
      m_byOverview.equal_range( MAX_UINT32 );

   // Loop over them and put the id:s in the result.
   for ( overviewKeyed_t::const_iterator it = range.first;
         it != range.second;
         ++it ) {
      mapIDs.insert( it->second.getMapID() );
   }
}

void
ItemIDTree::getContents( uint32 mapID, set<IDPair_t>& items) const
{
   // Get the range of stuff that belongs to the map
   pair<overviewKeyed_t::const_iterator,
        overviewKeyed_t::const_iterator> range =
      m_byOverview.equal_range( mapID );
   
   for ( overviewKeyed_t::const_iterator it = range.first;
         it != range.second;
         ++it ) {
      items.insert( it->second );
   }
}
   
void 
ItemIDTree::getContents( uint32 mapID, ItemIDTree& idTree ) const
{
   set<IDPair_t> items;
   getContents( mapID, items );
   for ( set<IDPair_t>::const_iterator it = items.begin(); 
         it != items.end(); ++it ) {
      mc2dbg8 << "[ItemIDTree] Adding " << it->first << ", " << it->second 
              <<  " to " << mapID << endl;
      idTree.addMapOrItem( mapID, it->first, it->second );
      if ( it->second != MAX_UINT32 ) {
         // Recursive call
         getContents( it->second, idTree );
      }
   }
}

int
ItemIDTree::getWholeMaps( set<uint32>& wholeMaps ) const
{
   int nbr = 0;

   for ( overviewKeyed_t::const_iterator it = m_byOverview.begin();
         it != m_byOverview.end();
         ++it ) {
      if ( it->second.getMapID() != MAX_UINT32 &&
           it->second.getItemID() == MAX_UINT32 ) {
         // Check so that the map is not used as key for another
         // item or map.
         if ( m_byOverview.find( it->second.getMapID() ) == m_byOverview.end()) {
            wholeMaps.insert( it->second.getMapID() );
            ++nbr;
         }
      }
   }
   return nbr;
}

int
ItemIDTree::getAllItems( set<IDPair_t>& items ) const
{
   int nbr = 0;
   for ( overviewKeyed_t::const_iterator it = m_byOverview.begin();
         it != m_byOverview.end();
         ++it ) {
      // Check if the itemID is not MAX_UINT32 which means
      // that the whole map is added.
      if ( it->second.getMapID() != MAX_UINT32 &&
           it->second.getItemID() != MAX_UINT32 ) {
         items.insert(it->second);
         ++nbr;
      }
   }
   return nbr;
}

bool
ItemIDTree::wholeMap( uint32 mapID ) const
{
   // FIXME: Could be optimized
   set<uint32> wholeMaps;
   getWholeMaps(wholeMaps);
   // Find the map in wholemaps.
   return wholeMaps.find(mapID ) != wholeMaps.end();
      
}

void
ItemIDTree::getLowestLevelMapIDs( set<uint32>& mapIDs ) const
{
   // This is tricky. May have to be rewritten.
   
   for ( overviewKeyed_t::const_iterator it( m_byOverview.begin() );
         it != m_byOverview.end();
         ++it ) {
      // See if the map contains any other things
      uint32 mapID = it->second.getMapID();
      
      pair<overviewKeyed_t::const_iterator,
        overviewKeyed_t::const_iterator> range =
         m_byOverview.equal_range( mapID );

      // If the map does not contain any other map then were done already.
      if ( range.first == range.second ) {
         // Nothing but the map
         mapIDs.insert( mapID );
         // This map is done.
         continue;
      }

      // Check if the map contains items and not maps.
      for( overviewKeyed_t::const_iterator it2 = range.first;
           it2 != range.second;
           ++it2 ) {
         if ( ( it2->second.getMapID() != mapID ) ||
              ( it2->second.getItemID() != MAX_UINT32 ) ) {
            // Do not insert this map. It has maps in it.
            continue;
         }
      }
      // Passed the test. Add the map.
      mapIDs.insert( mapID );      
   }
}

uint32 
ItemIDTree::getHigherLevelMap( uint32 mapID ) const
{
   vector<uint32> allOverviews;
   allOverviews.reserve( 4 );
   uint32 higherMapID = MAX_UINT32;
   if( getOverviewMapsFor( mapID, allOverviews ) && !allOverviews.empty() ) {
      higherMapID = allOverviews.back();
   }
   return higherMapID;
}


bool
ItemIDTree::getOverviewMapsFor(uint32 mapID,
                               vector<uint32>& overviews) const
{
   // This will only work if the mapID only exists in one overview
   // map, which it should.
   bool found = false;
   for( overviewKeyed_t::const_iterator it( m_byOverview.begin() );
        it != m_byOverview.end();
        ++it ) {
      if ( it->second.getMapID() == mapID &&
           it->second.getItemID() == MAX_UINT32 ) {
         if ( it->first != MAX_UINT32 ) {
            // Not top level
            getOverviewMapsFor(it->first, overviews);
            overviews.push_back(it->first);
         }
         found = true;
         break;
      }
   }
   return found;
}


bool 
ItemIDTree::getLowerMapsFor( uint32 topMapID, vector<uint32>& lower ) const
{
   set<uint32> mapIDs;
   getTopLevelMapIDs( mapIDs );
   set<uint32>::const_iterator it = mapIDs.find( topMapID );
   if ( it != mapIDs.end() ) {
      // topMapID exists
      set<IDPair_t> items;
      getContents( topMapID, items );
      for ( set<IDPair_t>::const_iterator it = items.begin() ; 
            it != items.end() ; ++it ) 
      {
         lower.push_back( it->getMapID() );
      }

      return true;
   } else {
      return false;
   }
}

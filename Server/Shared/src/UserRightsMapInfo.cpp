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

#include "UserRightsMapInfo.h"

// is_sorted
#include "NonStdStl.h"

#include "UserData.h"
#include "IDTranslationTable.h"
#include "Packet.h"
#include "MapBits.h"

#include <map>
#include <algorithm>

/// Secret namespace for the comparator used internally.
namespace {
   ///Since the class only cares about the first value in the pair, the
   ///two operator() functions are templated on the second types(s).
   struct MapIDComp {
      ///Used to find a pair<uint32,T> by specifying only a uint32. 
      ///The search pairs are ordered by the first member.
      template<class T>
      bool operator()( const pair<uint32,T>& a, uint32 b) {
         return a.first < b;
      }
      ///Used to order pairs with first_type == uint32.
      template<class T, class U> //two templated type for greater versatility
      bool operator()( const pair<uint32,T>& a, const pair<uint32,U>& b ) {
         return a.first < b.first;
      }
   };
}


UserRightsMapInfo::UserRightsMapInfo( uint32 mapID,
                                      const UserUser* user,
                                      const MapRights& mask )
{
   m_mapID = mapID;
   if ( user != NULL ) {
      // This will have to be removed later.
      UserUser::regionRightMap_t userRights;
      user->getMapRightsForMap( mapID, userRights, mask );
      
      // Put them in our vector. They have been sorted by map.
      m_rights.insert( m_rights.end(),
                       userRights.begin(), userRights.end() );
   } else {
      //mc2log << warn << "[URMI]: User is NULL - all is allowed " << endl;
      m_rights.push_back( make_pair( m_mapID, ~MapRights() ) );
   }
   MC2_ASSERT( is_sorted( m_rights.begin(),
                          m_rights.end(),
                          MapIDComp() ) );
   //mc2dbg << *this;
}

ostream& operator<<( ostream& o, const UserRightsMapInfo& ur )
{
   for ( UserRightsMapInfo::rightsVector_t::const_iterator it =
            ur.m_rights.begin();
         it != ur.m_rights.end();
         ++it ) {
      o << "[URIM]: Map " << MC2HEX(it->first) << " has UR "
        << it->second << endl;
   }
   return o;
}

UserRightsMapInfo::UserRightsMapInfo( const Packet* packet, int& pos )
{
   load( packet, pos );
}

UserRightsMapInfo::UserRightsMapInfo()
{
   m_mapID = MAX_UINT32;   
}

UserRightsMapInfo::UserRightsMapInfo( uint32 mapID,
                                      const MapRights& mapRights )
{
   m_mapID = mapID;
   m_rights.push_back( make_pair( mapID, mapRights ) );
}

void
UserRightsMapInfo::swap( UserRightsMapInfo& other )
{
   std::swap( m_mapID,  other.m_mapID );
   m_rights.swap( other.m_rights );
}

bool
UserRightsMapInfo::empty() const
{
   return m_rights.empty();
}

inline MapRights
UserRightsMapInfo::getURForMap( uint32 mapID ) const
{
   static const MapIDComp mapidcomp = MapIDComp();

   if ( MapBits::isCountryMap( mapID ) ) {
      // Call the function again if the map is a country map.
      return getURForMap( MapBits::countryToOverview( mapID ) );
   }
   
   // Skip the binary search if we only have one element.
   if ( m_rights.size() == 1 &&
        m_rights.front().first == mapID ) {
      return m_rights.front().second;
   }
   rightsVector_t::const_iterator it =
      std::lower_bound( m_rights.begin(),
                        m_rights.end(),
                        mapID,
                        mapidcomp );
   if ( it != m_rights.end() && it->first == mapID ) {
      return it->second;
   } else {
      return MapRights();
   }
}

bool
UserRightsMapInfo::itemAllowed( const MapRights& itemUR,
                                const IDPair_t& itemID,
                                const IDTranslationTable& transtable ) const
{
   // First check the correct map.
   MapRights mapType = getURForMap( itemID.first );
   if ( mapType & itemUR ) {
      return true;
   } else if ( MapBits::isOverviewMap( m_mapID ) ) {
      if ( itemID.first != m_mapID ) {
         uint32 higherID = transtable.translateToHigher( itemID );
         if ( higherID != MAX_UINT32 ) {
            // Try with the new id.
            return itemAllowed( itemUR,
                                IDPair_t(m_mapID, higherID),
                                transtable);
         } 
      }
      // Not on this map.
      return false;
   }
   mc2dbg4 << "[URMI]: Not allowed " << mapType << " & "
           << itemUR << " == 0 " << endl;
   return false;
}

int
UserRightsMapInfo::load( const Packet* packet, int& pos )
{
   m_rights.clear();
   int origPos = pos;
   // Read map id
   packet->incReadLong( pos, m_mapID );
   // Read number of rights
   int nbrRights = packet->incReadLong( pos ); 

   m_rights.resize( nbrRights );
   for ( int i = 0; i < nbrRights; ++i ) {
      packet->incReadLong( pos, m_rights[i].first );
      m_rights[i].second.load( packet, pos );
   }
   
   MC2_ASSERT( is_sorted( m_rights.begin(),
                          m_rights.end(),
                          MapIDComp() ) );
   return pos - origPos;
}

int
UserRightsMapInfo::save( Packet* packet, int& pos ) const
{
   int origPos = pos;
   // Read map id
   packet->incWriteLong( pos, m_mapID );
   // Write number of rights
   packet->incWriteLong( pos, m_rights.size() );
   // Write rights
   int nbrRights = m_rights.size();
   for ( int i = 0; i < nbrRights; ++i ) {
      packet->incWriteLong( pos, m_rights[i].first );
      m_rights[i].second.save( packet, pos );
   }   
   return pos - origPos;
}

int
UserRightsMapInfo::getSizeInPacket() const
{
   uint32 size = 8; // Mapid + nbr rights
   for ( rightsVector_t::const_iterator it = m_rights.begin();
         it != m_rights.end();
         ++it ) {
      size += it->second.getSizeInPacket() + 4; // Right + mapID
   }
   return size;
}

uint32
UserRightsMapInfo::getFirstAllowedMapID() const
{
   if( ! m_rights.empty() ) {
      return m_rights[0].first;
   }
   return 0;
}

void 
UserRightsMapInfo::filterAllRights()
{
   for( rightsVector_t::iterator it = m_rights.begin();
        it != m_rights.end(); ++it) {
      it->second = filterMapRights( it->second );
   }
}


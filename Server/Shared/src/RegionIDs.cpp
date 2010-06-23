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
#include "RegionIDs.h"
#include "STLStringUtility.h"



RegionIDs::~RegionIDs() {
   // Nothing to delete, all stl members has no newed stuff in them.
}


uint32
RegionIDs::addRegionIDsFor( vector<uint32>& regionIDs, 
                            uint32 regionGroupID ) const
{
   regionGroupMap::const_iterator findIt = m_regionGroupMap.find( 
      regionGroupID );
   uint32 nbr = 0;
   if ( findIt != m_regionGroupMap.end() ) {
      nbr = (*findIt).second.first.size();
      regionIDs.reserve( regionIDs.size() + nbr );
      for ( uint32 i = 0 ; i < nbr ; ++i ) {
         regionIDs.push_back( (*findIt).second.first[ i ] );
      }
   }

   return nbr;
}

bool
RegionIDs::isTheRegionInGroup( uint32 regionGroupID,
                               uint32 expandedID ) const
{
   regionGroupMap::const_iterator findIt =
      m_regionGroupMap.find( regionGroupID );
   if ( findIt != m_regionGroupMap.end() ) {
      uint32 nbr = (*findIt).second.first.size();
      for ( uint32 i = 0 ; i < nbr ; ++i ) {
         if ( (*findIt).second.first[ i ] == expandedID ) {
            return true;
         }
      }
   }
   return false;
}


uint32
RegionIDs::addRegionGroupsFor( vector<uint32>& regionGroupIDs, 
                               uint32 regionID ) const
{
   regionIDMap::const_iterator findIt = m_regionIDMap.find( 
      regionID );
   uint32 nbr = 0;
   if ( findIt != m_regionIDMap.end() ) {
      nbr = (*findIt).second.size();
      regionGroupIDs.reserve( regionGroupIDs.size() + nbr );
      for ( uint32 i = 0 ; i < nbr ; ++i ) {
         regionGroupIDs.push_back( (*findIt).second[ i ] );
      }
   }

   return nbr;   
}


uint32
RegionIDs::addAllRegionGroups( vector<uint32>& regionGroupIDs ) const {
   uint32 nbr = 0;
   for ( regionGroupMap::const_iterator it = m_regionGroupMap.begin() ;
         it != m_regionGroupMap.end() ; ++it )
   {
      regionGroupIDs.push_back( (*it).first );
      ++nbr;
   }

   return nbr;
}

uint32 
RegionIDs::addAllRegionGroups( vector<pair<uint32, NameCollection> >& 
                               regionGroupIDs ) const
{
   uint32 nbr = 0;
   for ( regionGroupMap::const_iterator it = m_regionGroupMap.begin() ;
         it != m_regionGroupMap.end() ; ++it )
   {
      regionGroupIDs.push_back( make_pair( (*it).first, 
                                           (*it).second.second ) );
      ++nbr;
   }

   return nbr;
}

const RegionList*
RegionIDs::getRegionList( const MC2String& name ) const {
   regionListMap::const_iterator findIt = m_regionListMap.find( name );
   if ( findIt != m_regionListMap.end() ) {
      return &findIt->second;
   } else {
      return NULL;
   }
}

uint32
RegionIDs::getRegionIdFromMCC( const MC2String& mcc ) const {
   uint32 mccInt = STLStringUtility::strtoul( mcc );
   mccMap::const_iterator it =  m_mccMap.find( mccInt );
   if ( it != m_mccMap.end() ) {
      return it->second;
   } else {
      return MAX_UINT32;
   }
}

uint32
RegionIDs::getRegionIdFromIsoName( const MC2String& isoName ) const {
   isoNameMap::const_iterator it = m_isoNameMap.find( isoName );
   if ( it != m_isoNameMap.end() ) {
      return it->second;
   } else {
      return MAX_UINT32;
   }
}

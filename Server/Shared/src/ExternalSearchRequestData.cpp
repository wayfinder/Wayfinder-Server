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

#include "ExternalSearchRequestData.h"
#include "Packet.h"
#include "SearchRequestParameters.h"
#include "LangTypes.h"

ExternalSearchRequestData::ExternalSearchRequestData():
   m_distance( INVALID_DISTANCE )
{
   m_searchParams = new SearchRequestParameters;
}

ExternalSearchRequestData::
ExternalSearchRequestData( const SearchRequestParameters& params,
                           uint32 service,
                           const stringMap_t& values,
                           int startHitIdx,
                           int nbrHits,
                           ItemInfoEnums::InfoTypeFilter itemInfoFilter,
                           const MC2Coordinate& coord,
                           Distance distance )
      : m_service( service ),
        m_values( values ),
        m_coord( coord ),
        m_distance( distance ),
        m_itemInfoFilter( itemInfoFilter )
{
   m_searchParams = new SearchRequestParameters( params );
   m_startHitIdx = startHitIdx;
   m_endHitIdx   = startHitIdx + nbrHits;
}

ExternalSearchRequestData&
ExternalSearchRequestData::operator=( const ExternalSearchRequestData& other )
{
   if ( this != &other ) {
      delete [] m_searchParams;
      m_service = other.m_service;
      m_coord = other.m_coord;
      m_distance = other.m_distance;
      m_values  = other.m_values;
      m_searchParams = new SearchRequestParameters( *other.m_searchParams );
      m_startHitIdx = other.m_startHitIdx;
      m_endHitIdx   = other.m_endHitIdx;
      m_itemInfoFilter = other.m_itemInfoFilter;
   }
   return *this;
}

ExternalSearchRequestData::
ExternalSearchRequestData( const ExternalSearchRequestData& other )
      : m_service( other.m_service ),
        m_searchParams( new SearchRequestParameters( *other.m_searchParams ) ),
        m_values( other.m_values ),
        m_coord( other.m_coord ),
        m_distance( other.m_distance ),
        m_startHitIdx( other.m_startHitIdx ),
        m_endHitIdx( other.m_endHitIdx ),
        m_itemInfoFilter( other.m_itemInfoFilter )     
{
}

ExternalSearchRequestData::~ExternalSearchRequestData()
{
   delete m_searchParams;
}

uint32
ExternalSearchRequestData::getService() const
{
   return m_service;
}

LangType
ExternalSearchRequestData::getLang() const
{
   return m_searchParams->getRequestedLang();
}

const SearchRequestParameters&
ExternalSearchRequestData::getSearchParams() const
{
   return *m_searchParams;
}

const ExternalSearchRequestData::stringMap_t& 
ExternalSearchRequestData::getValues() const {
   return m_values;
}

const MC2String&
ExternalSearchRequestData::getVal( int key ) const
{
   static MC2String emptyStr;
   stringMap_t::const_iterator it = m_values.find( key );
   if ( it != m_values.end() ) {
      return it->second;
   } else {
      return emptyStr;
   }
}

int
ExternalSearchRequestData::getSizeInPacket() const
{
   int size = 4;
   size += m_searchParams->getSizeInPacket();
   size += 4;
   size += 4;
   size += 4; // Start hit idx
   size += 4; // End hit idx
   size += 4; // latitude
   size += 4; // longitude
   size += 4; // distance
   size += 4; // item info filter
   for ( stringMap_t::const_iterator it = m_values.begin();
         it != m_values.end();
         ++it ) {
      AlignUtility::alignLong( size );
      size += 4; // string length
      size += it->second.size() + 1; // string + \0
   }
   AlignUtility::alignLong( size );
   return size;
}

int
ExternalSearchRequestData::save( Packet* packet, int& pos ) const
{
   int sizeInPacket = getSizeInPacket();
   int startPos = pos;
   packet->incWriteLong( pos, sizeInPacket );
   
   packet->incWriteLong( pos, getService() );
   packet->incWriteLong( pos, m_startHitIdx );
   packet->incWriteLong( pos, m_endHitIdx );
   packet->incWriteLong( pos, m_coord.lat );
   packet->incWriteLong( pos, m_coord.lon );
   packet->incWriteLong( pos, m_distance );
   packet->incWriteLong( pos, m_itemInfoFilter );

   m_searchParams->save( packet, pos );
   
   packet->incWriteLong( pos, m_values.size() );
   for ( stringMap_t::const_iterator it = m_values.begin();
         it != m_values.end();
         ++it ) {
      packet->incWriteLong( pos, it->first );
      packet->incWriteString( pos, it->second );
   }
   packet->incAlignWriteLong( pos );
   MC2_ASSERT( ( pos - startPos ) == sizeInPacket );
   return sizeInPacket;
}

int
ExternalSearchRequestData::load( const Packet* packet, int& pos )
{
   int startPos = pos;
   int sizeInPacket = packet->incReadLong( pos );
   
   packet->incReadLong( pos, m_service );
   packet->incReadLong( pos, m_startHitIdx );
   packet->incReadLong( pos, m_endHitIdx );
   packet->incReadLong( pos, m_coord.lat );
   packet->incReadLong( pos, m_coord.lon );
   packet->incReadLong( pos, m_distance );
   packet->incReadLong( pos, m_itemInfoFilter );

   m_searchParams->load( packet, pos );
   
   int nbrEntries = packet->incReadLong( pos );
   pair<int, MC2String> entry;
   for ( int i = 0; i < nbrEntries; ++i ) {
      packet->incReadLong( pos, entry.first );
      packet->incReadString( pos, entry.second );
      m_values.insert( entry );
   }
   pos = startPos + sizeInPacket;
   return sizeInPacket;
}

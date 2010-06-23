/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SearchRequestParameters.h"
#include "PacketHelpers.h"
#include "Packet.h"
#include "AlignUtility.h"

int
SearchRequestParameters::getSizeInPacket() const
{
   return 4 * 16 + m_categoriesInMatches.size() * 4 
      + m_mapRights.getSizeInPacket() +
      4 +  // number of categories
      AlignUtility::
      alignToLong( sizeof ( Categories::value_type ) * m_categories.size() ) +
      4 + // number of poi types
      sizeof ( POITypeSet::value_type ) * m_poiTypes.size();
}

int
SearchRequestParameters::save(Packet* packet, int& pos) const
{   
   SaveLengthHelper slh( packet, pos );
   
   int startPos = pos;
   packet->incWriteLong(pos, m_matchType);
   packet->incWriteLong(pos, m_stringPart);
   packet->incWriteLong(pos, m_sortingType);
   
   packet->incWriteLong(pos, m_regionsInMatches);
   packet->incWriteLong(pos, m_overviewRegionsInMatches);
   packet->incWriteLong(pos, m_reqLanguage | 0x80000000 );      
   
   packet->incWriteLong(pos, m_nbrHits);
   packet->incWriteLong(pos, m_locationType);
   packet->incWriteLong(pos, m_searchForTypes);
   
   packet->incWriteLong(pos, m_addStreetNameToCompanies);
   packet->incWriteLong(pos, m_endHitIndex);
   packet->incWriteLong(pos, m_uniqueOrFull);

   packet->incWriteLong(pos, m_addSynonymNameToPOIs);
   packet->incWriteLong( pos, m_invertedRight );
   m_mapRights.save( packet, pos );

   // write categories
   {
      packet->incWriteLong( pos, m_categories.size() );
      Categories::const_iterator it = m_categories.begin();
      Categories::const_iterator itEnd = m_categories.end();
      for ( ; it != itEnd; ++it ) {
         packet->incWriteShort( pos, *it );
      }
   }
   AlignUtility::alignLong( pos );

   // write poi types
   packet->incWriteLong( pos, m_poiTypes.size() );
   for ( POITypeSet::const_iterator
            it = m_poiTypes.begin(), itEnd = m_poiTypes.end();
         it != itEnd;
         ++it ) {
      packet->incWriteLong( pos, *it );
   }

   slh.writeDummyLength( pos );
   packet->incWriteLong(pos, m_categoriesInMatches.size());
   for(uint32 i = 0; i < m_categoriesInMatches.size(); i++)
      packet->incWriteLong(pos, m_categoriesInMatches[i]);
   slh.updateLengthUsingEndPos( pos );

   MC2_ASSERT( ( pos - startPos ) == getSizeInPacket() );
   
   return getSizeInPacket();
}

int
SearchRequestParameters::load(const Packet* packet, int& pos)
{
   LoadLengthHelper llh( packet, pos );
   
   packet->incReadLong(pos, m_matchType);
   packet->incReadLong(pos, m_stringPart);
   packet->incReadLong(pos, m_sortingType);
   
   packet->incReadLong(pos, m_regionsInMatches);
   packet->incReadLong(pos, m_overviewRegionsInMatches);
   uint32 n = packet->incReadLong( pos );
   bool isOld = ( (n & 0x80000000) == 0);
   if( ! isOld ) {
      m_reqLanguage = (LangTypes::language_t) (n & 0x7fffffff);
   } else {
      m_reqLanguage = (LangTypes::language_t) n;
   }
   
   packet->incReadLong(pos, m_nbrHits);
   packet->incReadLong(pos, m_locationType);
   packet->incReadLong(pos, m_searchForTypes);
   
   packet->incReadLong(pos, m_addStreetNameToCompanies);
   packet->incReadLong(pos, m_endHitIndex);
   packet->incReadLong(pos, m_uniqueOrFull);

   packet->incReadLong(pos, m_addSynonymNameToPOIs);
   packet->incReadLong( pos, m_invertedRight );
   m_mapRights.load( packet, pos );

   // load categories
   uint32 nbrCategories = packet->incReadLong( pos );
   for ( uint32 cat = 0; cat < nbrCategories; ++cat ) {
      m_categories.insert( packet->incReadShort( pos ) );
   }

   AlignUtility::alignLong( pos );

   m_poiTypes.clear();
   uint32 nbrPOITypes = packet->incReadLong( pos );
   for ( uint32 poiTypeIndex = 0;
         poiTypeIndex < nbrPOITypes;
         ++poiTypeIndex ) {
      m_poiTypes.insert( static_cast< POITypeSet::value_type >
                         ( packet->incReadLong( pos ) ) );
   }


   if( ! isOld ) {
      llh.loadLength( pos );
      uint32 n = packet->incReadLong(pos);
      for(uint32 i = 0; i < n; i++) {
         uint32 itemID = packet->incReadLong(pos);
         m_categoriesInMatches.push_back(itemID);
      }
      llh.skipUnknown( pos );
   }
      
   return getSizeInPacket();
}

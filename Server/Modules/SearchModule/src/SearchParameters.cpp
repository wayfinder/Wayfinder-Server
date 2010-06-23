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

#include<set>

#include "SearchParameters.h"

#include "SearchPacket.h"
#include "SearchRequestParameters.h"
#include "StringUtility.h"


VanillaSearchParameters::~VanillaSearchParameters()
{
}


void
VanillaSearchParameters::setRestriction(uint8 newRest)
{
   m_restriction = newRest;
}

UserSearchParameters::UserSearchParameters(const SearchRequestPacket * packet)
{
   vector<IDPair_t> allowedRegions;
   SearchRequestParameters params;
   
   char* searchString;
   packet->getParamsAndRegions(params, searchString,
                               allowedRegions, m_bboxes,
                               m_rights );
   
   // Trim start and end to get exact matches when they are exact.
   MC2_ASSERT( searchString != NULL );
   m_searchString = StringUtility::trimStartEnd( searchString );

   // Transfer the allowedRegions to the masks
   uint32 nbrRegions = allowedRegions.size();
   m_regionIDs.resize( nbrRegions );

   if ( packet->getSearchType() != SearchRequestPacket::PROXIMITY_SEARCH ) {
      // Copy the allowed areas into the masks
      for( uint32 i = 0; i < nbrRegions; ++i ) {
         m_regionIDs[i] = allowedRegions[i].getItemID();
      }
   } else {
      // When doing proximity search we use the allowedregions as
      // items to expand.
      m_itemsToConvertToMatches = allowedRegions;
      m_regionIDs.clear();
   }
   m_minNbrHits = params.getMinNbrHits();
   m_endHitIndex = params.getEndHitIndex();
   m_matching = params.getMatchType();
   m_stringPart = params.getStringPart();
   m_sorting = params.getSortingType();
   m_restriction = 0;
   m_returnCategories = params.getCategoriesInMatches();
   m_mapRights = params.getMapRights();
   m_invertRights = params.invertRights();
   m_categories = params.getCategories();
   m_poiTypes.clear();
   copy( params.getPOITypes().begin(), params.getPOITypes().end(),
         std::inserter( m_poiTypes, m_poiTypes.begin() ) );

   // Check if to use the parameters for overview search or ordinary.
   if ( packet->getSearchType() != SearchRequestPacket::OVERVIEW_SEARCH ) {
      m_requestedTypes = params.getSearchForTypes();
      m_returnRegionTypes = params.getRegionsInMatches();
   } else {
      // Overview search has its own parameters in the server.
      m_requestedTypes    = params.getSearchForLocationTypes();
      m_returnRegionTypes = params.getRegionsInOverviewMatches();
   }

   m_language = params.getRequestedLang();
   m_editDistanceCutoff = 0;
   m_addPoints = 0;
   m_mapID = packet->getMapID();

   m_addSSIToCompanies = params.getAddStreetNamesToCompanies();
   m_sortOrigin = packet->getSortOrigin();

   // When the old stuff is removed from the GenerateMapServer,
   // why don't we have our own copy of the SearchRequestParameters?
   m_addSynonymNameToPOIs = params.getAddSynonymNameToPOIs();
   
   mc2dbg8 << "[USP]: Sort origin = " << m_sortOrigin << endl;
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SearchHeadingDesc.h"

#include "MapRights.h"
#include "SearchTypes.h"
#include "UserData.h"

SearchHeadingDesc::SearchHeadingDesc( ImageTable::ImageSet imageSet,
                                     const CompactSearchHitTypeVector& headings ) {
   
   // These are the rights not to be shown in normal search only in own heading
   const uint64 mapRights = 0;
   const uint64 fullRights = MAX_UINT64;

   // default descriptions of the service type in english
   const char* ADDRESSES_DEFAULT_DESCRIPTION = "Street addresses";
   const char* POI_DEFAULT_DESCRIPTION = "Interesting places";

   CompactSearchHitType internalTypes[] = {
      //
      // Internal round 0 search headings
      //
      { "Places", 
        StringTable::SEARCH_HEADING_PLACES, 
        POI_DEFAULT_DESCRIPTION,
        StringTable::ESP_DESCR_POI,
        ImageTable::getImage( ImageTable::SEARCH_HEADING_PLACES, imageSet ), 
        0, // Round
        0, // ServiceID
        0, // Heading, Note: don't change this headings nbr, it's a special case
        SEARCH_COMPANIES|SEARCH_MISC, 
        MAX_UINT32, LangTypes::invalidLanguage,
        mapRights, true, CompactSearchHitType::yellow_pages
      },
      { "Addresses", 
        StringTable::SEARCH_HEADING_ADDRESSES, 
        ADDRESSES_DEFAULT_DESCRIPTION,
        StringTable::ESP_DESCR_STREETS,
        ImageTable::getImage( ImageTable::SEARCH_HEADING_ADDRESSES, imageSet ),
        0, // Round
        0, // ServiceID
        1, // Note: do not change this headings number, it's a special case
        SEARCH_STREETS, 
        MAX_UINT32, LangTypes::invalidLanguage,
        mapRights, true, CompactSearchHitType::yellow_pages 
      }
   };

   CompactSearchHitType additionalTypes[] = {
      // Special headings we don't search in, only
      // here for controlling the search view order
      { "Favorites",
        StringTable::FAVOURITES,
        "",
        StringTable::NOSTRING,
        "search_heading_favorites",
        0,   // round
        0,   // service id
        FAVORITES_HEADING, // heading
        0, // no mask
        MAX_UINT32, 
        LangTypes::invalidLanguage,
        fullRights, false, CompactSearchHitType::yellow_pages 
      },
      { "Phonebook",
        StringTable::NOSTRING,
        "",
        StringTable::NOSTRING,
        "search_heading_phonebook",
        0,   // round
        0,   // service id
        PHONEBOOK_HEADING, // heading
        0, // no mask
        MAX_UINT32, 
        LangTypes::invalidLanguage,
        fullRights, false, CompactSearchHitType::yellow_pages 
      },
      { "MergedResults",
        StringTable::NOSTRING,
        "",
        StringTable::NOSTRING,
        "search_heading_merged_results",
        0,   // round
        0,   // service id
        MERGED_RESULTS_HEADING, // heading
        0, // no mask
        MAX_UINT32, 
        LangTypes::invalidLanguage,
        fullRights, false, CompactSearchHitType::yellow_pages 
      }
   };
   
   // Add the internal types
   m_headings = CompactSearchHitTypeVector( internalTypes, internalTypes + 
                                            sizeof( internalTypes ) / 
                                            sizeof( internalTypes[ 0 ] ) );

   // Add external services
   m_headings.insert( m_headings.end(), headings.begin(), headings.end() );


   // Add additional services last
   m_headings.insert( m_headings.end(), additionalTypes, additionalTypes +
                      sizeof( additionalTypes ) / sizeof( additionalTypes[ 0 ] ) );



   // setup external ID map to heading index
   // and heading index to heading ID map
   for ( uint32 i = 0; i < m_headings.size(); ++i ) {
      if ( m_headings[ i ].m_round == 1 ) {
         m_externalToHeadingIndex.
            insert( make_pair( m_headings[ i ].m_serviceID, i ) );
      }
      m_headingIDToHeadingIndex[ m_headings[ i ].m_heading ] = i;
   }

}

CompactSearchHitType 
SearchHeadingDesc::findServiceHeadingType( ServiceID serviceID ) const {
   map<ServiceID, HeadingIndex>::const_iterator it = 
      m_externalToHeadingIndex.find( serviceID );
   if ( it == m_externalToHeadingIndex.end() ) {
      return CompactSearchHitType();
   }

   return m_headings[ (*it).second ];
}

SearchHeadingDesc::HeadingID 
SearchHeadingDesc::findServiceHeading( ServiceID serviceID ) const {
   CompactSearchHitType type = findServiceHeadingType( serviceID );
   return type.isValid() ? type.m_heading : INVALID_HEADING_ID;
}

bool 
SearchHeadingDesc::canSearchHeading( const CompactSearchHitType& heading,
                                     const UserUser& user ) {
   // special headings
   if ( isSpecialHeading( heading.m_heading ) ) {
      return false;
   }

   // all rights
   if ( heading.m_mapRights == MAX_UINT64 ) {
      return true;
   }

   // make sure the client has the rights to search this heading
   // So check heading maprights if it has non inverted rights
   // and compare it to the user rights,
   // if the user does not have the headings map rights then 
   // continue with the next heading.

   if ( ! heading.m_invertRights &&
        ! user.hasAnyRightIn( MapRights( static_cast<MapRights::Masks>
                                         ( heading.m_mapRights ) ) ) ) {
      return false;
   }

   return true;
}

const CompactSearchHitType&
SearchHeadingDesc::
getHeading( HeadingID id ) const throw ( std::out_of_range ) {
   HeadingIDToHeadingIndex::const_iterator it = 
      m_headingIDToHeadingIndex.find( id );
   if ( it == m_headingIDToHeadingIndex.end() ) {
      throw std::out_of_range( "Invalid heading ID" );
   }

   return m_headings.at( it->second );
}

CompactSearchHitType SearchHeadingDesc::
getHitTypeFromHeading( HeadingID heading ) const { 
   // find out which round we should use
   for ( uint32 i = 0; i < m_headings.size(); ++i ) {
      if ( m_headings[i].m_heading == static_cast<uint32>( heading )){
         return m_headings[ i ];
      }
   }
   
   return CompactSearchHitType();
}

bool SearchHeadingDesc::isSpecialHeading( HeadingID heading ) {
   if ( heading == FAVORITES_HEADING || 
        heading == PHONEBOOK_HEADING || 
        heading == MERGED_RESULTS_HEADING ) {
      return true;
   }
   return false;
}

namespace {

/**
 * Translates a string code, if there is no translation for the
 * given language, english will be tried, if there still is no
 * translation a default string will be returned.
 *
 * @param stringCode What to translate.
 * @param language The language to translate to.
 * @param defaultString The default string to return.
 */
MC2String translate( StringTable::stringCode stringCode,
                     LangType::language_t language,
                     const MC2String& defaultString ) {

   const char* translated =  StringTable::getString( stringCode,
                                                     LangType( language ) );

   // try english if translation failed
   if ( translated == NULL ) {
      translated = StringTable::getString( stringCode,
                                           LangType( LangTypes::english ) );
   }

   // if still failed, return the default string
   if ( translated == NULL || strlen( translated ) == 0 ) {
      return defaultString;
   }
   else {
      return translated;
   }
}

}

void SearchHeadingDesc::
translateHitType( CompactSearchHitType& hitType,
                  LangType::language_t language) const {
   // special case for round 1
   if ( hitType.m_round == 1 ) {
      // We dont use name here because the old search needs 
      // the old description strings with "country name + whatever" for name
      // but the new combined/compact search needs a more compact form of
      // description without country name prepended.

          hitType.m_topRegionID = findServiceHeadingType(hitType.m_serviceID).m_topRegionID;
 
   } else {
      // assuming round 0
      // translate the name
      hitType.m_name = ::translate( hitType.m_nameStringCode,
                                    language,
                                    hitType.m_name );
   }

   // translate the string
   hitType.m_type = ::translate( hitType.m_typeStringCode,
                                 language,
                                 hitType.m_type );
}

void SearchHeadingDesc::
getTranslatedHeadings( const UserUser& user,
                       LangType::language_t language,
                       bool includeSpecialHeadings,
                       CompactSearchHitTypeVector& hitTypes ) const {

   // translate each external service name
   // TODO: translate internal service name
   for ( uint32 i = 0; i < m_headings.size(); ++i ) {
      if ( ! isSpecialHeading( m_headings[ i ].m_heading ) &&
           ! canSearchHeading( m_headings[ i ], user ) ) {
         continue;
      }

      if ( ! includeSpecialHeadings && 
           isSpecialHeading( m_headings[ i ].m_heading ) ) {
         // do not include special headings
         continue;
      }

      hitTypes.push_back( m_headings[ i ] );

      CompactSearchHitType& hitType = hitTypes.back();

      translateHitType( hitType, language );

   }

}

void SearchHeadingDesc::
getHeadings( const UserUser& user,
             CompactSearchHitTypeVector& headings ) const {
   for ( uint32 i = 0; i < m_headings.size(); ++i ) {
      // must have the correct rights for this heading
      if ( ! canSearchHeading( m_headings[ i ], user ) ) {
         continue;
      }

      headings.push_back( m_headings[ i ] );
   }
}

CompactSearchHitType
SearchHeadingDesc::findSearchTypeHeadingType( uint32 searchType ) const {
   for ( uint32 i = 0, endNbr = m_headings.size() ; i < endNbr ; ++i ) {
      if ( m_headings[ i ].m_round == 0 &&
           (m_headings[ i ].m_searchTypeMask & searchType) != 0 ) {
         // Match!
         return m_headings[ i ];
      }
   }

   return CompactSearchHitType();
}

SearchHeadingDesc::HeadingID 
SearchHeadingDesc::findSearchTypeHeading( uint32 searchType ) const {
   CompactSearchHitType type = findSearchTypeHeadingType( searchType );
   return type.isValid() ? type.m_heading : INVALID_HEADING_ID;
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExternalSearchHeadingDesc.h"
#include "CompactSearchPacketTools.h"
#include "DataBuffer.h"
#include "Packet.h"
#include "MC2CRC32.h"

#include "MapRights.h"
#include "ExtServices.h"
#include "SearchTypes.h"
#include "PropertyHelper.h"

ExternalSearchHeadingDesc::ExternalSearchHeadingDesc() : m_crc(0) {
   // setup headings

   // Do NOT add an element with empty name value!, some parts of the code in
   // this file uses empty() to validate a hit type
   //
   // The heading number must be unique and not more than 99 as 
   // SearchHeadingDesc starts it's SpecialHeadings at 100.
   // 
   // There are two elements in each hit type that needs to be updated
   // (if needed) with language:
   // * name
   // * top region id
   // these two can not be used directly without translating or getting valid
   // top region id from external search desc.
   //
   // The real names for round 1 is NOT taken from ExternalSearchDescGenerator.cpp
   // The names here for round 1 is the actual names,
   // so in conclusion: If you change name here, please update the names in
   // ExternalSearchDescGenerator.cpp !
   //
   const uint64 fullRights = MAX_UINT64;

   // default descriptions of the service type in english
   // const char* WP_DEFAULT_DESCRIPTION = "People listings";
   const char* YP_DEFAULT_DESCRIPTION = "Business listings";
   const char* POI_DEFAULT_DESCRIPTION = "Interesting places";
   // const char* ERP_DEFAULT_DESCRIPTION = "Electronic road pricing";

   CompactSearchHitType types[] = {
      /* //
      // Internal round 0 search headings
      //
      { "Places",
        StringTable::SEARCH_HEADING_PLACES,
        "",
        StringTable::NOSTRING,
        "search_heading_places",   0,
        0,
        0, // Note: do not change this headings number, it's a special case
        SEARCH_COMPANIES|SEARCH_MISC,
        MAX_UINT32, LangTypes::invalidLanguage,
        mapRights, true
      },
      { "Addresses",
        StringTable::SEARCH_HEADING_ADDRESSES,
        "",
        StringTable::NOSTRING,
        "search_heading_addresses", 0,
        0,
        1, SEARCH_STREETS,
        MAX_UINT32, LangTypes::invalidLanguage,
        mapRights, true
        },*/

      //
      // External round 1 search headings
      //


      /*  
      //
      // Additional services should be last.
      //
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
        fullRights, false
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
        fullRights, false
        },*/
   };

   m_headings = CompactSearchHitTypeVector( types, types +
                                            sizeof( types ) /
                                            sizeof( types[ 0 ] ) );


   // Some heading that can be turned on and off though properties
   // The first heading in optionalTypes depends on the first
   // property in optionalTypesProperties and so on.
   CompactSearchHitType optionalTypes[] = {
      //
      // External round 1 search headings
      //

      // Google Local Search
      //
      { "Google Local Search",
        StringTable::NOSTRING,
        YP_DEFAULT_DESCRIPTION,
        StringTable::ESP_DESCR_BIZ,
        "search_heading_google_local_search",
        1, ExtService::google_local_search, 11, 0, MAX_UINT32, 
        LangTypes::english,
        fullRights, false, CompactSearchHitType::yellow_pages 
      },
      // Qype, provider with reviews, ratings and images
      { "Qype",
        StringTable::NOSTRING,
        POI_DEFAULT_DESCRIPTION,
        StringTable::ESP_DESCR_POI,
        "search_heading_qype",
        1, ExtService::qype, 12, 0, MAX_UINT32, 
        LangTypes::english,
        fullRights, false, CompactSearchHitType::yellow_pages 
      },
   };
   // When adding a Heading above add a property here too
   MC2String optionalTypesProperties[] = {
      "ENABLE_GOOGLE_LOCAL_SEARCH_INTEGRATION",
      "ENABLE_QYPE_INTEGRATION",
   };

   // Check if to add optional headings
   {
      const size_t nbrOptional = sizeof( optionalTypes ) / 
         sizeof( optionalTypes[ 0 ] );
      for ( size_t i = 0 ; i < nbrOptional ; ++i ) {
         if ( PropertyHelper::get( optionalTypesProperties[ i ], false ) ) {
            m_headings.push_back( optionalTypes[ i ] );
         }
      }
   }

}

uint32 
ExternalSearchHeadingDesc::getHeadingsCRC() {

   if( m_crc == 0 ) {
      // This is the first request. Calculate the crc
      uint32 size = CompactSearchPacketTools::calcPacketSize( m_headings );
      DataBuffer buf ( size );
      int pos = 0;
      Packet tmpPack( buf.getCurrentOffsetAddress(), 
                      buf.getNbrBytesLeft(), true );
      CompactSearchPacketTools::writeToPacket( tmpPack, pos, m_headings );
      buf.readPastBytes( pos );
      m_crc = MC2CRC32::crc32 ( buf.getBufferAddress(),
                              buf.getCurrentOffset() );

   }
   
   return m_crc;
}


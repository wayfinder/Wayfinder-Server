/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OneSearchBinaryResult.h"
#include "STLUtility.h"
#include "STLStringUtility.h"
#include "SearchParserHandler.h"
#include "SearchMatch.h"
#include "ItemInfoPacket.h"
#include "NavSearchHandler.h"
#include "ItemDetailEnums.h"

namespace {

/**
 * Class for holding info about a Area
 */
class Area {
public:
   Area() {};
   Area( const MC2String& idStr) : id( idStr) {};
   
   int16 type;
   MC2String name;
   MC2String id;

   bool operator==( const MC2String& other ) const{
      return id == other;
   }

   bool operator<( const Area& other ) const {
      return id < other.id;
   }
};

// Gets the size of the fixed
int32 getMatchTableFixedFieldsSize() {
   return sizeof( int32 ) +  // nameIndex
      sizeof( int32 ) + // idIndex
      sizeof( int32 ) + // locationIndex
      sizeof( int32 ) + // lat
      sizeof( int32 ) + // lon
      sizeof( int8 ) + // type
      sizeof( int16 ) + // subtype
      sizeof( int32 ) + // categoryImageIndex 
      sizeof( int32 ) + // providerImageIndex
      sizeof( int32 ) + // brandImageIndex
      sizeof( int8 ) + // flags
      sizeof( int8 ) + // nbrCategories
      sizeof( int8 ) + // nbrAreas
      sizeof( int8 );  // nbrInfoItems
}

/**
 * Get the name for the match, companies have street name in name.
 */
MC2String getMatchName( const VanillaMatch& match ) {
   MC2String name( match.getName() );
   if ( match.getType() == SEARCH_COMPANIES ) {
      const VanillaCompanyMatch& poi =
         static_cast< const VanillaCompanyMatch& >( match );
      if ( ! poi.getCleanCompanyName().empty() ) {
         name = poi.getCleanCompanyName();
      }
   }
   return name;
}

/**
 * Gets all strings and puts them in the stringMap, setting all indexes to zero.
 * Adds areas to the map and calculates size of item info table and matches 
 * table.
 */
void getStringsAreasAndSizes( map<MC2String, int32>& stringMap,
                              map<Area, int32>& areas,
                              int32& nbrItemInfos,
                              int32& matchTableSize,
                              SearchParserHandler& searchHandler,
                              ResultVector::const_iterator matchesBegin,
                              ResultVector::const_iterator matchesEnd ) {
   
   nbrItemInfos = 0;
   matchTableSize = 0;
   for( ResultVector::const_iterator it = matchesBegin; 
        it != matchesEnd; ++it ) {
      const VanillaMatch* match = *it;
      
      // Add main strings
      stringMap[ getMatchName( *match ) ] = 0;
      stringMap[ match->matchToString() ] = 0;
      stringMap[ match->getLocationName() ] = 0;
      stringMap[ searchHandler.getCategoryImageName( *match ) ] = 0;
      stringMap[ searchHandler.getProviderImageName( *match ) ] = 0;
      stringMap[ searchHandler.getBrandImageName( *match ) ] = 0;
      
      matchTableSize += getMatchTableFixedFieldsSize();
      
      // Get category list size
      if ( match->getType() == SEARCH_COMPANIES ) {
         const VanillaCompanyMatch& poi =
            static_cast< const VanillaCompanyMatch& >( *match );
         matchTableSize += poi.getCategories().size() * 4;         
      }

      // Add string for areas
      for ( uint32 i = 0 ; i < match->getNbrRegions() ; ++i ) {
         VanillaRegionMatch* regMatch = match->getRegion( i );
         if ( regMatch->getType() ) {
            Area a;
            a.type = regMatch->getType();
            a.id = MC2String( regMatch->matchToString() );
            a.name = regMatch->getName();

            stringMap[ a.id ] = 0;
            stringMap[ a.name ] = 0;
            areas[ a ] = 0;
            matchTableSize += 4;
         }
      }
      
      // Add strings for item info
      ItemInfoData::ItemInfoEntryCont::const_iterator it;
      ItemInfoData::ItemInfoEntryCont::const_iterator endIt = 
          match->getItemInfos().end();
      for ( it =  match->getItemInfos().begin() ; it != endIt ; ++it ) {
         stringMap[ it->getKey() ] = 0;
         stringMap[ it->getVal() ] = 0;
         nbrItemInfos++;
         matchTableSize += 4;
      }
   }
}

/**
 * Calculates the size of the string table, given all the strings.
 */
int calcStringTableSize( const map<MC2String, int32>& stringMap ) {

   int totalStringLength = 0;

   // Sum up the sizes of all strings
   for ( map<MC2String, int32>::const_iterator itr = stringMap.begin();
         itr != stringMap.end(); ++itr) {
      totalStringLength += (*itr).first.size();
   }

   // Each string has 3 bytes for length indicator + zero terminator
   return stringMap.size()*3 + totalStringLength;
}

/**
 * Creates the string table and sets the indexes to each string
 * in the string map.
 */
void writeStringTable( map<MC2String, int32>& stringMap,
                       DataBuffer* stringTable ) {

   for ( map<MC2String, int32>::iterator itr = stringMap.begin();
         itr != stringMap.end(); ++itr) {
      // The position should be the position of the string, not of the
      // length indicator
      (*itr).second = stringTable->getCurrentOffset() + sizeof( uint16 );

      // Write length indicator
      stringTable->writeNextBAShort( (*itr).first.size() );

      // Write string content
      stringTable->writeNextString( (*itr).first.c_str() );
   }  
}

/**
 * Given a string, this function returns its index into the string table.
 * Since the stringMap should contain all strings, there's a bug somewhere
 * if the string isn't available, so we assert false.
 */
int32 getStringIndex( const map<MC2String, int32>& stringMap,
                      const MC2String& str ) {
   map<MC2String, int32>::const_iterator itr = stringMap.find( str );
   
   if ( itr == stringMap.end() ) {
      // Should never happen
      MC2_ASSERT( false );
      return 0;
   }
   else {
      return (*itr).second;
   }
}

/**
 * Creates the Area table and sets the indexes to correct values
 */
void writeAreaTable( map<Area, int32>& areaMap,
                     const map<MC2String, int32>& stringMap,
                     DataBuffer* areaTable ) {

   for ( map<Area, int32>::iterator itr = areaMap.begin();
         itr != areaMap.end(); ++itr) {
      
      // Set index pos
      (*itr).second = areaTable->getCurrentOffset();

      // Write type
      areaTable->writeNextBAShort( NavSearchHandler::mc2TypeToNavRegionType(
                                      (*itr).first.type ) );
      
      // Write name index
      areaTable->writeNextBALong( getStringIndex( stringMap,
                                                  (*itr).first.name ) );
      // Write id index
      areaTable->writeNextBALong( getStringIndex( stringMap,
                                                  (*itr).first.id ) );
   }  
}


/**
 * Get index for area with id str
 */
int32 getAreaIndex( const map<Area, int32>& areaMap,
                    const MC2String& str ) {
   map<Area, int32>::const_iterator itr = areaMap.find( str );
   
   if ( itr == areaMap.end() ) {
      // Should never happen
      MC2_ASSERT( false );
      return 0;
   }
   else {
      return (*itr).second;
   }
}

typedef vector<uint16> Categories;


/**
 * Create the matchesTable and itemInfoTable
 */
void writeDataTables( const map<MC2String, int32>& stringMap,
                      const map<Area, int32>& areas,
                      SearchParserHandler& searchHandler,
                      ResultVector::const_iterator  matchesBegin,
                      ResultVector::const_iterator  matchesEnd,
                      OneSearchUtils::BinarySearchResult* format) {
   
   DataBuffer* matchBuffer = format->m_matchesTable.get();
   DataBuffer* itemInfoBuffer = format->m_infoItemTable.get();
   
   for( ResultVector::const_iterator it = matchesBegin; 
        it != matchesEnd; ++it ) {
      const VanillaMatch& match = *(*it);

      // Write name and id indexes
      matchBuffer->writeNextBALong( getStringIndex( stringMap,
                                                    getMatchName( match ) ) );
      matchBuffer->writeNextBALong( getStringIndex( stringMap,
                                               match.matchToString() ) );

      // Write location
      matchBuffer->writeNextBALong( getStringIndex( stringMap,
                                               match.getLocationName() ) );

      matchBuffer->writeNextBALong( match.getCoords().lat );
      matchBuffer->writeNextBALong( match.getCoords().lon );

      // Write match type and sub type
      matchBuffer->writeNextBAByte( 
         NavSearchHandler::mc2TypeToNavItemType( match.getType() ) );

      matchBuffer->writeNextBAShort( 
         NavSearchHandler::mc2ItemTypeToNavSubType( 
            ItemTypes::itemType( match.getItemSubtype() ) ) );

      // Write the icons
      matchBuffer->writeNextBALong( 
         getStringIndex( stringMap, 
                         searchHandler.getCategoryImageName( match ) ) );
      matchBuffer->writeNextBALong( 
         getStringIndex( stringMap, 
                         searchHandler.getProviderImageName( match ) ) );
      matchBuffer->writeNextBALong( 
         getStringIndex( stringMap, 
                         searchHandler.getBrandImageName( match ) ) );

      // Write flags
      // Aditional info is currently the only flag
      matchBuffer->writeNextBAByte( match.getAdditionalInfoExists() );

      // Get this hits categories count
      int32 hitCatCount = 0;
      if ( match.getType() == SEARCH_COMPANIES ) {
         const VanillaCompanyMatch& poi =
            static_cast< const VanillaCompanyMatch& >( match );
         hitCatCount = poi.getCategories().size();
      }

      // Write number of categories
      matchBuffer->writeNextBAByte( hitCatCount );

      // Write number of areas
      matchBuffer->writeNextBAByte( match.getNbrRegions() );
      
      // Write number of info fields
      matchBuffer->writeNextBAByte( match.getItemInfos().size() );
 

      // Write the categories
      if ( match.getType() == SEARCH_COMPANIES ) {
         const VanillaCompanyMatch& poi =
            static_cast< const VanillaCompanyMatch& >( match );
         const Categories& ids = poi.getCategories();
         for( Categories::const_iterator it = ids.begin(); 
              it!= ids.end(); ++it ) {
            matchBuffer->writeNextBALong( *it );
         }
      }

      // Write the area offsets in match buffer
      for ( uint32 i = 0 ; i < match.getNbrRegions() ; ++i ) {
         matchBuffer->writeNextBALong( 
            getAreaIndex( areas, match.getRegion( i )->matchToString() ) );         
      }
      
      // Write the detail infos
      ItemInfoData::ItemInfoEntryCont::const_iterator it;
      ItemInfoData::ItemInfoEntryCont::const_iterator endIt = 
         match.getItemInfos().end();
      for ( it = match.getItemInfos().begin() ; it != endIt ; ++it ) {
         // Write offset into match buffer
         matchBuffer->writeNextBALong( itemInfoBuffer->getCurrentOffset() );

         // Write data into item info buffer
         itemInfoBuffer->writeNextBAShort( 
            ItemDetailEnums::poiInfoToPoiDetail( it->getInfoType() ) );

         itemInfoBuffer->writeNextBAByte( 
            ItemDetailEnums::getContentTypeForPoiInfo( it->getInfoType() ) );

         itemInfoBuffer->writeNextBALong( 
            getStringIndex( stringMap, it->getKey() ) );
         itemInfoBuffer->writeNextBALong( 
            getStringIndex( stringMap, it->getVal() ) );
      }
      
   }
}  
}

namespace OneSearchUtils {

void serializeResults( ResultVector::const_iterator matchesBegin,
                       ResultVector::const_iterator matchesEnd,
                       SearchParserHandler& searchHandler,
                       BinarySearchResult* format) {

   map<MC2String, int32> stringMap;
   map<Area, int32> areas;
   int32 nbrOfItemInfos = 0;
   int32 matchTableSize = 0;
   
   // Get string table and other data
   getStringsAreasAndSizes( stringMap, areas, nbrOfItemInfos, matchTableSize,
                            searchHandler, matchesBegin, matchesEnd );

   // Write down the string table
   int stringTableSize = calcStringTableSize( stringMap );

   format->m_stringTable.reset( new DataBuffer( stringTableSize ) );

   writeStringTable( stringMap, format->m_stringTable.get() );


   // Write the area table
   const uint32 areaTableSize =  areas.size() * 
      ( sizeof( int16 ) +  2 * sizeof( uint32 ) );
   format->m_areaTable.reset( new DataBuffer( areaTableSize, true ) );

   writeAreaTable( areas, stringMap, format->m_areaTable.get() );

   // Write matches and info field tables
   const uint32 infoItemTableSize = nbrOfItemInfos *
      ( sizeof( int16 ) + sizeof( int8 ) + 2 * sizeof( int32 ) );
   format->m_infoItemTable.reset( new DataBuffer( infoItemTableSize, true ) );

   format->m_matchesTable.reset( new DataBuffer( matchTableSize ) );

   writeDataTables( stringMap, areas, searchHandler, matchesBegin, matchesEnd, 
                    format );

   // Reset the buffers
   format->m_stringTable->reset();
   format->m_areaTable->reset();
   format->m_infoItemTable->reset();
   format->m_matchesTable->reset();
}

}

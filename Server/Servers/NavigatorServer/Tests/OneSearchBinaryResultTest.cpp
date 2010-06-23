/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"


#define SEARCH_PARSER_HANDLER_H // Avoid including the real SearchParserHandler

#include "SearchMatch.h"

/**
 * Stub implementation for SearchParser handler. Only handling the 
 * icon lookups
 */
class SearchParserHandler_stub{
public:
   SearchParserHandler_stub() : icon("DummyIcon"){ };
   MC2String getCategoryImageName( const SearchMatch& match ) const {
      return icon + "_cat";
   };
   
   MC2String getBrandImageName( const SearchMatch& match ) const {
      return icon + "_brand";
   };
   
   MC2String getProviderImageName( const SearchMatch& match ) const {
      return icon + "_provider";
   };

   MC2String icon;
};


typedef SearchParserHandler_stub SearchParserHandler;

#include "OneSearchBinaryResult.h"
#include "../src/OneSearchBinaryResult.cpp"

using namespace OneSearchUtils;

namespace {

class InfoItem {
public:
   InfoItem(){};
   
   int16 type;
   int8 content_type;
   MC2String key;
   MC2String value;
};

void
parseAndCheckBinaryFormat( BinarySearchResult* format, 
                           const vector<VanillaMatch*>& matches ) {

   const char* stringBase = (const char*) format->m_stringTable->getBufferAddress();

   map<int32, Area> areaMap;

   // Read area table
   MC2_TEST_REQUIRED( format->m_areaTable->getBufferSize() % 10 == 0 );
   while( format->m_areaTable->getNbrBytesLeft() > 0 ) {
      int32 pos = format->m_areaTable->getCurrentOffset();
      Area a;
      a.type = int16( format->m_areaTable->readNextBAShort() );
      a.name = (const char*) stringBase + format->m_areaTable->readNextBALong();
      a.id = (const char*) stringBase + format->m_areaTable->readNextBALong();
      areaMap[pos] = a;
   }

   // Read into item table1
   MC2_TEST_REQUIRED( format->m_infoItemTable->getBufferSize() % 11 == 0 );
   map<int32, InfoItem> infoItemMap;
   while( format->m_infoItemTable->getNbrBytesLeft() > 0 ) {
      int32 pos = format->m_infoItemTable->getCurrentOffset();
      InfoItem ii;
      ii.type = int16( format->m_infoItemTable->readNextBAShort() );
      ii.content_type = int8( format->m_infoItemTable->readNextBAByte() );
      ii.key = (const char*) stringBase + format->m_infoItemTable->readNextBALong();
      ii.value = (const char*) stringBase + format->m_infoItemTable->readNextBALong();
      infoItemMap[pos] = ii;
   }

   int32 nbrOfMatches = 0;
   while ( format->m_matchesTable->getNbrBytesLeft() > 0 ) {
      
      const char* name = stringBase + format->m_matchesTable->readNextBALong();
      const char* id = stringBase + format->m_matchesTable->readNextBALong();
      const char* location = stringBase + format->m_matchesTable->readNextBALong();
      
      int32 lat =  format->m_matchesTable->readNextBALong();
      int32 lon =  format->m_matchesTable->readNextBALong();
      int8 type =  format->m_matchesTable->readNextBAByte();
      int16 subType = format->m_matchesTable->readNextBAShort();
      
      MC2String catIcon = stringBase + format->m_matchesTable->readNextBALong();
      MC2String providerIcon = stringBase + format->m_matchesTable->readNextBALong();
      MC2String brandIcon = stringBase + format->m_matchesTable->readNextBALong();
      
      int8 flags = format->m_matchesTable->readNextBAByte();

      size_t nbrCat = format->m_matchesTable->readNextBAByte();
      size_t nbrAreas =  format->m_matchesTable->readNextBAByte();
      size_t nbrInfo =  format->m_matchesTable->readNextBAByte();

      const VanillaCompanyMatch* orgMatch = static_cast<VanillaCompanyMatch*>( matches[nbrOfMatches] );

      MC2_TEST_CHECK( ( flags & 0x1 ) == orgMatch->getAdditionalInfoExists() );

      MC2_TEST_CHECK( nbrCat == orgMatch->getCategories().size() );
      MC2_TEST_CHECK( nbrAreas == orgMatch->getNbrRegions() );
      MC2_TEST_CHECK( nbrInfo == orgMatch->getItemInfos().size() );         
      
      // Read categories
      vector<uint16> cats;
      for( uint32 i = 0; i < nbrCat; i++ ) {
         cats.push_back( format->m_matchesTable->readNextBALong() );
      }
      MC2_TEST_CHECK( equal( cats.begin(), cats.end(), orgMatch->getCategories().begin() ) );

      // Read areas
      map<int32, Area> newAreaMap;
      for( uint32 i = 0; i < nbrAreas; i++ ) {
         uint32 areaOff = format->m_matchesTable->readNextBALong();

         Area a = areaMap[areaOff];
         MC2_TEST_CHECK( strcmp(a.name.c_str(), orgMatch->getRegion(i)->getName() ) == 0 );
         MC2_TEST_CHECK( strcmp(a.id.c_str(), orgMatch->getRegion(i)->matchToString().c_str() ) == 0 );
      }

      // Read info items
      for( uint32 i = 0; i < nbrInfo; i++ ) {
         uint32 itemOff = format->m_matchesTable->readNextBALong();

         InfoItem ii = infoItemMap[itemOff];
         MC2_TEST_CHECK( strcmp(ii.key.c_str(), orgMatch->getItemInfos()[i].getKey() ) == 0 );
         MC2_TEST_CHECK( strcmp(ii.value.c_str(), orgMatch->getItemInfos()[i].getVal() ) == 0 );

      }

      // Check the results
      MC2_TEST_CHECK( orgMatch->getName() == MC2String( name ) );
      MC2_TEST_CHECK( orgMatch->matchToString() == MC2String( id ) );
      MC2_TEST_CHECK( orgMatch->getLocationName() == MC2String( location ) );
      MC2_TEST_CHECK( orgMatch->getCoords().lat == lat );
      MC2_TEST_CHECK( orgMatch->getCoords().lon == lon );
      MC2_TEST_CHECK( orgMatch->getType() == uint32(type) );
      MC2_TEST_CHECK( NavSearchHandler::mc2ItemTypeToNavSubType( 
                         ItemTypes::itemType(orgMatch->getItemSubtype()) ) == subType  );
    
      nbrOfMatches++;
   }
   MC2_TEST_CHECK( format->m_matchesTable->getNbrBytesLeft() == 0 );
   MC2_TEST_CHECK( uint32(nbrOfMatches) == matches.size() );
}

}


MC2_UNIT_TEST_FUNCTION( OneSearchBinaryResultTest ) {
   MC2String name( "The Search Match" );
   MC2String locationName( "London City" );

   VanillaCompanyMatch theMatch( IDPair_t(),
                                 name.c_str(), 
                                 locationName.c_str(),/*locationName*/
                                 0/*offset*/, 
                                 0/*streetNumber*/ );

   VanillaCompanyMatch theMatch2( IDPair_t(),
                                  "MCD", 
                                  "Magistratsvagen",/*locationName*/
                                  0/*offset*/, 
                                  23/*streetNumber*/ );

   // Add regions
   MC2String regionStr( "Big London" );
   VanillaRegionMatch region1( regionStr.c_str(), 5,2,33, SEARCH_BUILT_UP_AREAS , SearchMatchPoints(), 1, regionStr.c_str(),
                               44, 3, ItemTypes::municipalItem, 2 );

   MC2String regionStr2( "Small London" );
   VanillaRegionMatch region2( regionStr2.c_str(), 6,1,13,SEARCH_BUILT_UP_AREAS, SearchMatchPoints(), 2, regionStr2.c_str(),
                               45, 4, ItemTypes::municipalItem, 3 );

   theMatch.addRegion( &region1 );
   theMatch.addRegion( &region2 );

   theMatch2.addRegion( &region2 );

   // Add categories
   vector< uint16 > categories;
   categories.push_back( 21 );
   categories.push_back( 13 );
   categories.push_back( 456 );
   categories.push_back( 8 );
   theMatch.setCategories( categories );

   // Add Item infos
   SearchMatch::ItemInfoVector infoVec;
   ItemInfoEntry e1( "KeyNumber1", "Value1", ItemInfoEnums::vis_address );
   ItemInfoEntry e2( "ZipCode", "24755", ItemInfoEnums::vis_zip_code );
   infoVec.push_back( e1 );
   infoVec.push_back( e2 );
   theMatch.swapItemInfos( infoVec );
   theMatch.setAdditionalInfoExists( true );

   // Add matches to vector
   vector<VanillaMatch*> matches;
   matches.push_back( &theMatch );
   matches.push_back( &theMatch2 );
   
   // Serialize
   SearchParserHandler handler;
   BinarySearchResult format;
   serializeResults( matches.begin(), matches.end(), handler, &format );

   // Parse and verify
   parseAndCheckBinaryFormat( &format, matches );
}

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

#include "ItemInfoPacket.h"
#include "SearchMatch.h"
#include "UserData.h"
#include "Packet.h"

namespace {

void addMatches( vector<VanillaMatch*>& matches ) {
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
   VanillaRegionMatch region1( regionStr.c_str(), 5,2,33,4, SearchMatchPoints(), 1, regionStr.c_str(),
                               44, 3, ItemTypes::municipalItem, 2 );

   MC2String regionStr2( "Small London" );
   VanillaRegionMatch region2( regionStr2.c_str(), 6,1,13,2, SearchMatchPoints(), 2, regionStr2.c_str(),
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
   matches.push_back( &theMatch );
   matches.push_back( &theMatch2 );
}
}

MC2_UNIT_TEST_FUNCTION( itemInfoDataTest ) {

   MC2Coordinate coord( 12345678, 12345 );
   ItemInfoData infoData( "TheType", "UnitTestItem1", ItemTypes::pointOfInterestItem,
                          0, coord );

   infoData.setMoreInfoAvailable( true );

   // Check that sizes are correct
   const uint32 calculatedSize = infoData.getSizeInPacket();
   Packet pack( calculatedSize + HEADER_SIZE );
   int pos = 0;
   uint32 acctualSize = infoData.save( &pack, pos );

   MC2_TEST_REQUIRED_EXT( calculatedSize >= acctualSize,
                          "Calculated size less than written size" );

   ItemInfoData loadedData;
   pos = 0;
   uint32 loadedSize = loadedData.load( &pack, pos );

   MC2_TEST_REQUIRED_EXT( loadedSize == acctualSize,
                          "Size of read containers does not match written size");

   // Tests on some data members
   MC2_TEST_CHECK( loadedData.getCoord() == infoData.getCoord() );
   MC2_TEST_CHECK( loadedData.getMoreInfoAvailable() == infoData.getMoreInfoAvailable() );
   MC2_TEST_CHECK( loadedData.getType() == infoData.getType() );
   MC2_TEST_CHECK( loadedData.getItemName() == infoData.getItemName() );
}

MC2_UNIT_TEST_FUNCTION( getItemInfoRequestPacketTest ) {
   UserUser user( 1354135 );
   vector<VanillaMatch*> items;
   addMatches( items );
   
   GetItemInfoRequestPacket reqPack( 654,
                                     LangTypes::italian,
                                     ItemInfoEnums::OneSearch_All,
                                     &user,
                                     items );
   MC2_TEST_CHECK( reqPack.getLanguage() == LangTypes::italian );
   MC2_TEST_CHECK( reqPack.getItemInfoFilter() == ItemInfoEnums::OneSearch_All );
}

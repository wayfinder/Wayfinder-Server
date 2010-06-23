/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoriesData.h"
#include "LocaleUtility.h"
#include "StringUtility.h"
#include "ImageTable.h"
#include "StringTable.h"
#include "ItemTypes.h"
#include "Properties.h"
#include "STLUtility.h"
#include "TimeUtility.h"
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace CategoryTreeUtils;

CategoriesData::CategoriesData( bool clientUsesLatin1 ):
   m_categoriesCRC( 0 ), m_nbrCategories( 0 ),
   m_clientUsesLatin1( clientUsesLatin1 )
{
}


CategoriesData::~CategoriesData() {

}

void
CategoriesData::makeList( const char* clientType,
                          LangTypes::language_t language,
                          const CategoriesDataHolder::CatData& inData ) {
   innerMakeList( clientType, language, inData );
}

uint32
CategoriesData::getCRC() const {
   return m_categoriesCRC;
}


uint32 
CategoriesData::getNbrCategories() const {
   return m_nbrCategories;
}


/**
 * Information about a category, things we will need to know
 * about each category
 */
struct CategoryInfo {
   CategoryInfo( const MC2String& _name,
                 ImageTable::ImageCode _imageName,
                 StringTable::stringCode _stringCode )
         : name(_name), imageName(_imageName), stringCode(_stringCode) {
   }

   MC2String name; ///< internal name for the category, used in old search
   ImageTable::ImageCode imageName; ///< the image for the category
   StringTable::stringCode stringCode; ///< the string code for the category
};

typedef map<CategoryID, CategoryInfo> CategoryInfoMap;
// When adding new cateory to this list add as searchable in SPH too
const CategoryInfoMap categoryInfo = boost::assign::map_list_of
( 18,    CategoryInfo( "Airport", ImageTable::CAT_AIRPORT, StringTable::AIRPORT ) )
( 272,   CategoryInfo( "Banks & ATMs", ImageTable::CAT_BANK, StringTable::SEARCHCAT_BANK_ATM ) )
( 152,   CategoryInfo( "ATM", ImageTable::CAT_ATM, StringTable::ATM ) )
( 151,   CategoryInfo( "Bank", ImageTable::CAT_BANK, StringTable::BANK ) )
( 76,    CategoryInfo( "Car dealer", ImageTable::CAT_CARDEALER, StringTable::CAR_DEALER ) )
( 98,    CategoryInfo( "Cinema", ImageTable::CAT_CINEMA, StringTable::CINEMA ) )
( 266,   CategoryInfo( "City Centre", ImageTable::CAT_CITYCENTRE, StringTable::CITY_CENTRE ) )
( 128,   CategoryInfo( "Ferry terminal", ImageTable::CAT_FERRY_TERMINAL, StringTable::FERRY_TERMINAL ) )
( 48,    CategoryInfo( "Golf course", ImageTable::CAT_GOLF_COURSE, StringTable::GOLF_COURSE ) )
( 118,   CategoryInfo( "Hotel", ImageTable::CAT_HOTEL, StringTable::HOTEL ) )
( 123,   CategoryInfo( "Local rail", ImageTable::CAT_COMMUTER_RAIL_STATION, StringTable::LOCAL_RAIL ) )
( 130,   CategoryInfo( "Monument", ImageTable::CAT_HISTORICAL_MONUMENT, StringTable::HISTORICAL_MONUMENT ) )
( 22,    CategoryInfo( "Museum", ImageTable::CAT_MUSEUM, StringTable::MUSEUM ) )
( 63,    CategoryInfo( "Music store", ImageTable::CAT_MUSIC_STORE, StringTable::CAT_MUSIC_AND_VIDEO ) )
( 6,     CategoryInfo( "Nightlife", ImageTable::CAT_NIGHT_LIFE, StringTable::NIGHTLIFE ) )
( 237,   CategoryInfo( "Open parking area", ImageTable::CAT_OPEN_PARKING_AREA, StringTable::OPEN_PARKING_AREA ) )
( 236,   CategoryInfo( "Parking garage", ImageTable::CAT_PARKING_GARAGE, StringTable::PARKING_GARAGE ) )
( 103,   CategoryInfo( "Petrol station", ImageTable::CAT_PETROL_STATION, StringTable::PETROL_STATION ) )
( 245,   CategoryInfo( "Pharmacy", ImageTable::CAT_PHARMACY, StringTable::PHARMACY ) )
( 108,   CategoryInfo( "Police station", ImageTable::CAT_POLICE_STATION, StringTable::POLICE_STATION ) )
( 107,   CategoryInfo( "Post office", ImageTable::CAT_POST_OFFICE, StringTable::POST_OFFICE ) )
( 15,    CategoryInfo( "Rent a car", ImageTable::CAT_RENTACAR_FACILITY, StringTable::RENT_A_CAR_FACILITY ) )
( 85,    CategoryInfo( "Restaurant", ImageTable::CAT_RESTAURANT, StringTable::RESTAURANT ) )
( 59,    CategoryInfo( "Shopping centre", ImageTable::CAT_SHOPPING_CENTRE, StringTable::SHOPPING_CENTRE ) )
( 9,     CategoryInfo( "Shopping", ImageTable::CAT_SHOPPING_CENTRE, StringTable::CAT_SHOPPING__Z__ ) )
( 40,    CategoryInfo( "Ski resort", ImageTable::CAT_SKI_RESORT, StringTable::SKI_RESORT ) )
( 88,    CategoryInfo( "Theatre", ImageTable::CAT_THEATRE, StringTable::THEATRE ) )
( 19,    CategoryInfo( "Tourist information", ImageTable::CAT_TOURIST_OFFICE, StringTable::TOURIST_OFFICE ) )
( 122,   CategoryInfo( "Train Station", ImageTable::CAT_RAILWAY_STATION, StringTable::RAILWAY_STATION ) )
( 104,   CategoryInfo( "Vehicle repair facility", ImageTable::CAT_CAR_REPAIR, StringTable::VEHICLE_REPAIR_FACILITY ) )
( 100,   CategoryInfo( "Bars", ImageTable::CAT_BARS, StringTable::CAT_BARS ) )
( 86,    CategoryInfo( "Cafe", ImageTable::CAT_CAFE, StringTable::CAFE ) )
( 246,   CategoryInfo( "Hospital", ImageTable::CAT_HOSPITAL, StringTable::HOSPITAL ) )
( 90,    CategoryInfo( "Night club", ImageTable::CAT_NIGHT_CLUB, StringTable::CAT_NIGHT_CLUB ) )
( 267,   CategoryInfo( "Parking", ImageTable::CAT_PARKING, StringTable::PARKING ) )
( 121,   CategoryInfo( "Subway", ImageTable::CAT_SUBWAY_STATION, StringTable::SUBWAY_STATION ) )
( 248,   CategoryInfo( "Doctor", ImageTable::CAT_DOCTOR, StringTable::DOCTOR ) )
( 67,    CategoryInfo( "Grocery store", ImageTable::CAT_GROCERY_STORE, StringTable::GROCERY_STORE ) )
( 5,     CategoryInfo( "Travel & Transport", ImageTable::CAT_COMMUTER_RAIL_STATION, StringTable::SEARCHCAT_TRAVEL ) );


typedef map<uint32, ImageTable::ImageCode> POIToImageCode;
POIToImageCode searchListImageMap = boost::assign::map_list_of
( ItemTypes::airport, ImageTable::CAT_AIRPORT )
( ItemTypes::atm, ImageTable::CAT_ATM )
( ItemTypes::bank, ImageTable::CAT_BANK )
( ItemTypes::cafe, ImageTable::CAT_CAFE )
( ItemTypes::carDealer, ImageTable::CAT_CARDEALER )
( ItemTypes::cinema, ImageTable::CAT_CINEMA )
( ItemTypes::cityCentre, ImageTable::CAT_CITYCENTRE )
( ItemTypes::doctor, ImageTable::CAT_DOCTOR )
( ItemTypes::ferryTerminal, ImageTable::CAT_FERRY_TERMINAL )
( ItemTypes::golfCourse, ImageTable::CAT_GOLF_COURSE )
( ItemTypes::hospital, ImageTable::CAT_HOSPITAL )
( ItemTypes::hotel, ImageTable::CAT_HOTEL )
( ItemTypes::commuterRailStation, ImageTable::CAT_COMMUTER_RAIL_STATION )
( ItemTypes::historicalMonument, ImageTable::CAT_HISTORICAL_MONUMENT )
( ItemTypes::museum, ImageTable::CAT_MUSEUM )
// Not sure we should have musicCentre
// ItemTypes::musicCentre
( ItemTypes::nightlife, ImageTable::CAT_NIGHT_LIFE )
( ItemTypes::openParkingArea, ImageTable::CAT_OPEN_PARKING_AREA )
( ItemTypes::parkingGarage, ImageTable::CAT_PARKING_GARAGE )
( ItemTypes::petrolStation, ImageTable::CAT_PETROL_STATION )
( ItemTypes::pharmacy, ImageTable::CAT_PHARMACY )
( ItemTypes::policeStation, ImageTable::CAT_POLICE_STATION )
( ItemTypes::postOffice, ImageTable::CAT_POST_OFFICE )
( ItemTypes::rentACarFacility, ImageTable::CAT_RENTACAR_FACILITY )
( ItemTypes::restaurant, ImageTable::CAT_RESTAURANT )
( ItemTypes::shoppingCentre, ImageTable::CAT_SHOPPING_CENTRE )
( ItemTypes::skiResort, ImageTable::CAT_SKI_RESORT )
( ItemTypes::taxiStop, ImageTable::CAT_TAXI )
( ItemTypes::theatre, ImageTable::CAT_THEATRE )
( ItemTypes::touristOffice, ImageTable::CAT_TOURIST_OFFICE )
( ItemTypes::railwayStation, ImageTable::CAT_RAILWAY_STATION )
( ItemTypes::vehicleRepairFacility, ImageTable::CAT_CAR_REPAIR )
// additional poi types
( ItemTypes::wlan, ImageTable::CAT_WLAN )
( ItemTypes::busStation, ImageTable::CAT_BUS_STATION )
( ItemTypes::casino, ImageTable::CAT_CASINO )
( ItemTypes::amusementPark, ImageTable::CAT_AMUSEMENTPARK )
( ItemTypes::mosque, ImageTable::CAT_MOSQUE )
( ItemTypes::church, ImageTable::CAT_CHURCH )
( ItemTypes::synagogue, ImageTable::CAT_SYNAGOGUE )
( ItemTypes::touristAttraction, ImageTable::CAT_TOURIST_ATTRACTION )
( ItemTypes::tramStation, ImageTable::CAT_TRAM_STATION )
( ItemTypes::groceryStore, ImageTable::CAT_GROCERY_STORE )
( ItemTypes::shop, ImageTable::CAT_SHOP )
( ItemTypes::subwayStation, ImageTable::CAT_SUBWAY_STATION );


struct MapCatData {
   MapCatData( const MC2String& strID,
               const MC2String& image,
               CategoryID id ):
      stringID( strID ),
      imagename( image ),
      catID( id ) {
   }
   MC2String stringID;
   MC2String imagename;
   CategoryID catID;
};

namespace {

MC2String getNameForCategory( CategoryID id ) {
   CategoryInfoMap::const_iterator itr = categoryInfo.find( id );

   if ( itr == categoryInfo.end() ) {
      return "";
   }
   else {
      return (*itr).second.name;
   }
}

ImageTable::ImageCode getImageForCategory( CategoryID id ) {
   CategoryInfoMap::const_iterator itr = categoryInfo.find( id );

   if ( itr == categoryInfo.end() ) {
      return ImageTable::NOIMAGE;
   }
   else {
      return (*itr).second.imageName;
   }
}

StringTable::stringCode 
getStringCodeForCategory( CategoryID id ) {
   CategoryInfoMap::const_iterator itr = categoryInfo.find( id );

   if ( itr == categoryInfo.end() ) {
      return StringTable::NOSTRING;
   }
   else {
      return (*itr).second.stringCode;
   }
}

}

CategoriesDataHolder::CategoriesDataHolder() 
: m_flushTime( TimeUtility::getCurrentTime() ) {
}


CategoriesDataHolder::~CategoriesDataHolder() {
}

void CategoriesDataHolder::
readCategoryListsFromFile( const MC2String& categorySet ) {
   MC2String path = 
      MC2String( Properties::getProperty( "CATEGORIES_PATH",
                                          "./Categories" ) ) + "/" + 
      categorySet + ".txt";

   ifstream is( path.c_str() );
   if ( is.fail() ) {
      mc2log << error << "Failed to open category file " + path << endl;
      return;
   }

   MC2String line;

   // Parse the file line by line
   while ( getline( is, line ) ) {
      if ( line[0] == '#' ) { // Skip comments
         continue;
      }
         
      istringstream iss( line );
      MC2String firstToken;
      iss >> firstToken;
         
      if ( firstToken.empty() || firstToken[ firstToken.size()-1 ] != ':' ) {
         // Empty or incorrectly formatted line
         continue;
      }
      
      // Remove the colon
      firstToken.erase( firstToken.size()-1 );
      MC2String regionID = 
         ( firstToken == "default" ? 
           CategoryRegionID::NO_REGION.toString() : firstToken );
         
      vector<CategoryID> categoryList;      
      MC2String nextToken;
      while ( iss >> nextToken ) {
         try {
            categoryList.push_back( 
               boost::lexical_cast<CategoryID>( nextToken ) );
         } catch ( const boost::bad_lexical_cast& e ) {
            mc2log << warn << "Failed to parse category from category file "
                   << "(bad token: " << nextToken << ")"
                   << endl;
         }
      }

      m_categoryLists[ categorySet ][ regionID ] = categoryList;
   }
}

bool CategoriesDataHolder::timeToFlushCategoryLists() const {
   uint32 cacheInterval = 
      Properties::getUint32Property( "CATEGORIES_FLUSH_INTERVAL", 
                                     TimeUtility::hours2sec(1) // 1 hour
                                     ) * 1000;                 // to ms

   return ( TimeUtility::getCurrentTime() - m_flushTime ) > cacheInterval;
}

void CategoriesDataHolder::flushCategoryLists() {
   m_categoryLists.clear();
   m_flushTime = TimeUtility::getCurrentTime();
}

void CategoriesDataHolder::
getCategories( const MC2String& categorySet,
               CategoryRegionID regionID,
               vector<CategoryID>& categories ) {

   if ( timeToFlushCategoryLists() ) {
      flushCategoryLists();
   }

   if ( !STLUtility::has( m_categoryLists, categorySet ) ) {
      readCategoryListsFromFile( categorySet );
   }
   
   categories.clear();

   CategoryListMap::iterator itr = m_categoryLists.find( categorySet );

   if ( itr != m_categoryLists.end() ) {
      RegionToCategoryListMap& regionToCategoryList = (*itr).second;

      MC2String regionIDString = regionID.toString();
      MC2String defaultRegionString = CategoryRegionID::NO_REGION.toString();

      if ( STLUtility::has( regionToCategoryList, regionIDString ) ) {
         categories = regionToCategoryList[ regionIDString ];
      }
      else if ( STLUtility::has( regionToCategoryList, defaultRegionString ) ) {
         categories = regionToCategoryList[ defaultRegionString ];
      }
   }
}

CategoriesDataHolder::CatData*
CategoriesDataHolder::getCatData( const MC2String& categorySet,
                                  ImageTable::ImageSet imageSet,
                                  const char* clientType,
                                  LangTypes::language_t language, 
                                  bool latin1,
                                  CategoryRegionID regionID ) {

   vector<CategoryID> categories;

   getCategories( categorySet, regionID, categories );

   if ( categories.empty() ) {
      return NULL;
   }

   typedef map< MC2String, MapCatData, LocaleUtility > catmap;

   LocaleUtility locu( language );
   catmap cats( locu );

   for ( size_t i = 0 ; i < categories.size() ; ++i ) {
      StringTable::stringCode stringCode =
         getStringCodeForCategory( categories[ i ] );
      MC2String str = StringUtility::
         makeFirstCapital( StringTable::
                           getString( stringCode, language ) );

      MC2String name = getNameForCategory( categories[ i ] );
      ImageTable::ImageCode imageCode = getImageForCategory( categories[ i ] );

      cats.insert( make_pair( str,
                              MapCatData( name, 
                                          ImageTable::getImage( 
                                             imageCode,
                                             imageSet ),
                                          categories[ i ]
                                          )));

   }
   // now they are in the correct order
   // insert them into vectors
   vector< MC2String > ids;
   vector< MC2String > names;
   vector< MC2String > filenames;
   vector< CategoryID > catIDs;

   for ( catmap::const_iterator cit = cats.begin() ; 
         cit != cats.end() ; ++cit ) {
      if ( latin1 ) {
         ids.push_back( UTF8Util::mc2ToIso( (*cit).second.stringID ) );
         names.push_back( UTF8Util::mc2ToIso( (*cit).first ) );
      }
      else {
         ids.push_back( UTF8Util::mc2ToUtf8( (*cit).second.stringID ) );
         names.push_back( UTF8Util::mc2ToUtf8( (*cit).first ) );
      }
      filenames.push_back( (*cit).second.imagename );
      catIDs.push_back( (*cit).second.catID );
   }

   return new CatData( ids, names, filenames, catIDs );
}

CategoriesData*
CategoriesDataHolder::makeCategory( const MC2String& categorySet, 
                                    ImageTable::ImageSet imageSet,
                                    const char* clientType,
                                    LangTypes::language_t language, 
                                    bool latin1,
                                    CategoryRegionID regionID ) {
   ISABSync sync( m_mtx );

   auto_ptr<CatData> catData( getCatData( categorySet,
                                          imageSet,
                                          clientType,
                                          language,
                                          latin1,
                                          regionID ) );

   if ( catData.get() == NULL ) {
      return NULL;
   }
   else {
      CategoriesData* data = createCategoriesData( latin1 );
      data->makeList( clientType, language, *catData );
      return data;
   }
}

const char*
CategoriesDataHolder::getSearchListImageName( ImageTable::ImageSet imageSet,
                                              POIType poiType ) const {

   // clients with the default image set currently don't have special
   // icons for the search list but rather use the poi icons or search
   // heading icon
   if ( imageSet == ImageTable::DEFAULT ) {
      return "";
   }

   const char* result = "";
   POIToImageCode::iterator itr = searchListImageMap.find( poiType );
   if ( itr != searchListImageMap.end() ) {
      result = ImageTable::getImage( (*itr).second, imageSet );
   }

   // either couldn't find the image code in the map or the image set
   // didn't have an image for this image code, use the generic 
   // search hit icon
   if ( MC2String( result ) == "" ) {
      result = ImageTable::getImage( ImageTable::CAT_GENERIC, imageSet );
   }

   return result;
}

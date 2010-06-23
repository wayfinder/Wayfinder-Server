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

#include <memory>
#include <map>
#include <vector>
#include "MC2String.h"
#include "UTF8Util.h"
#include "SearchMatch.h"
#include "StringUtility.h"
#include "StringSearchUtility.h"
#include "STLStringUtility.h"
#include "ItemInfoEntry.h"
#include "PacketHelpers.h"
#include "ScopedArray.h"
#include "SearchReplyPacket.h"
#include "ExternalSearchConsts.h"
#include "NationalAddressFormatter.h"
#include "POIReview.h"


// --------------- SearchMatch --------------

map<char, uint32>
SearchMatch::initTypeCharMap()
{
   // Note that capitals are reserved for other stuff.
   // C and K are coordinates that must be mapped to items in MM.
   // X are matches from external sources.
   map<char, uint32> typeCharacterMap;
   typeCharacterMap['s'] = SEARCH_STREETS;
   typeCharacterMap['c'] = SEARCH_COMPANIES;
   typeCharacterMap['t'] = SEARCH_CATEGORIES;
   typeCharacterMap['m'] = SEARCH_MISC;
   typeCharacterMap['a'] = SEARCH_MUNICIPALS;
   typeCharacterMap['b'] = SEARCH_BUILT_UP_AREAS;
   typeCharacterMap['d'] = SEARCH_CITY_PARTS;
   typeCharacterMap['z'] = SEARCH_ZIP_CODES;
   typeCharacterMap['x'] = SEARCH_ZIP_AREAS;
   typeCharacterMap['p'] = SEARCH_PERSONS;
   typeCharacterMap['l'] = SEARCH_COUNTRIES;

   return typeCharacterMap;
}

map<char, uint32>
SearchMatch::c_typeCharacterMap = initTypeCharMap();

map<uint32, char>
SearchMatch::initCharTypeMap()
{
   // Note that capitals are reserved for other stuff.
   // FIXME: Just reverse the other one.
   map<uint32, char> characterTypeMap;
   characterTypeMap[SEARCH_STREETS]        = 's';
   characterTypeMap[SEARCH_COMPANIES]      = 'c';
   characterTypeMap[SEARCH_CATEGORIES]     = 't';
   characterTypeMap[SEARCH_MISC]           = 'm';
   characterTypeMap[SEARCH_MUNICIPALS]     = 'a';
   characterTypeMap[SEARCH_BUILT_UP_AREAS] = 'b';
   characterTypeMap[SEARCH_CITY_PARTS]     = 'd';
   characterTypeMap[SEARCH_ZIP_CODES]      = 'z';
   characterTypeMap[SEARCH_ZIP_AREAS]      = 'x';
   characterTypeMap[SEARCH_PERSONS]        = 'p';
   characterTypeMap[SEARCH_COUNTRIES]      = 'l';
   return characterTypeMap;
}

map<uint32, char>
SearchMatch::c_characterTypeMap = initCharTypeMap();


SearchMatch::SearchMatch(const SearchMatch& other)
      :  m_typeid(other.m_typeid),
         m_id(other.m_id),
         m_itemType(other.m_itemType),
         m_itemSubtype(other.m_itemSubtype),
         m_bbox(other.m_bbox),
         m_coordinates(other.m_coordinates),
         m_distance(other.m_distance),
         m_points(other.m_points),
         m_restrictions(other.m_restrictions),
         m_type(other.m_type),
         m_angle( other.m_angle ),
         m_extID( other.m_extID ),
         m_extSource( other.m_extSource ),
         m_additionalInfoExists( other.m_additionalInfoExists )         
{
   for ( uint32 i = 0 ; i < other.getNbrRegions() ; ++i ) {
      m_regions.push_back( make_pair( static_cast<VanillaRegionMatch*>(
         other.getRegion( i )->clone() ), true ) );
   }

   // Copy the strings.
   m_name = NewStrDup::newStrDup(other.m_name);
   if ( other.m_locationName ) {
      m_locationName = NewStrDup::newStrDup(other.m_locationName);
   } else {
      m_locationName = NewStrDup::newStrDup("");
   }
   m_alphaSortingName = 
      NewStrDup::newStrDup(other.m_alphaSortingName );
   m_deleteStrings = true;
   m_synonymName = NewStrDup::newStrDup( other.getSynonymName() );
}

SearchMatch::SearchMatch( int tpd,
                          uint32 mapID,
                          uint32 itemID,
                          ItemTypes::itemType itemType,
                          uint16 itemSubtype,
                          const SearchMatchPoints& points,
                          uint8 restrictions,
                          uint16,
                          uint32 type,
                          const char* name,
                          const char* alphaSortingName,
                          const char* locationName,
                          uint32 nameInfo,
                          uint32 locationNameInfo,
                          uint32 location) :
      m_typeid(tpd),
      m_id(mapID, itemID),
      m_itemType(itemType),
      m_itemSubtype(itemSubtype),
      m_distance(0),
      m_points(points),
      m_restrictions(restrictions),
      m_type(type),
      m_name(name),
      m_locationName(locationName),
      m_alphaSortingName(alphaSortingName),
      m_synonymName( NULL ),
      m_deleteStrings(false),
      m_nameInfo(nameInfo),
      m_locationNameInfo(locationNameInfo),
      m_angle( MAX_UINT16 ),
      m_extSource( 0 ), // Source is MC2
      m_additionalInfoExists( false )
{
}

SearchMatch::SearchMatch( int tpd,
                          uint32 type,
                          const IDPair_t& id,
                          const char* name,
                          const char* locationName) :
      m_typeid(tpd),
      m_id(id),
      m_itemType(ItemTypes::numberOfItemTypes),
      m_itemSubtype(MAX_UINT16),
      m_distance(0),
      m_restrictions(0),
      m_type(type),
      m_name(NULL),
      m_locationName(NULL),
      m_alphaSortingName(NULL),
      m_synonymName( NULL ),
      m_deleteStrings(true),
      m_nameInfo(0),
      m_locationNameInfo(0),
      m_angle( MAX_UINT16 ),
      m_extSource(0), // Source is MC2
      m_additionalInfoExists( false )
{         
   m_name = NewStrDup::newStrDup(name ? name : "");
   m_locationName =
      NewStrDup::newStrDup(locationName ? locationName : "");
   m_alphaSortingName = NewStrDup::newStrDup( m_name );
}

ostream&
operator<<( ostream& stream,
            const SearchMatch& match)
{
   return stream << SearchMatch::searchTypeToTypeCharacter( match.getType() )
                 << ":" << match.getID() << ":\""
                 << match.getName() << "\"" << endl;
}

uint32
SearchMatch::searchMatchBaseStrLength = 1 + 1 + 10 + 1 + 10 + 1 + 10 + 1 + 10 + 256;

void
SearchMatch::deleteRegions()
{
   for ( uint32 i = 0 ; i < getNbrRegions() ; ++i ) {
      if ( m_regions[ i ].second ) {
         delete m_regions[ i ].first;
      }
   }
   m_regions.clear();
}

SearchMatch::~SearchMatch()
{
   deleteRegions();
   if ( m_deleteStrings ) {
      delete [] const_cast<char*>(m_name);
      delete [] const_cast<char*>(m_locationName);
      delete [] const_cast<char*>(m_alphaSortingName);
   }
   delete [] m_synonymName;
}


void
SearchMatch::setRegions(const RegionMatchVector& regions,
                        bool copy)
{
   deleteRegions();
   if ( copy == false ) {
      // Do not copy the regions.
      for ( uint32 i = 0 ; i < regions.size() ; ++i ) {
         m_regions.push_back(
            make_pair( regions[i].first, false ));
      }
   } else {
      // Copy the regions.
      for ( uint32 i = 0 ; i < regions.size() ; ++i ) {
         m_regions.push_back( make_pair( 
            static_cast<VanillaRegionMatch*>(regions[i].first->clone()),
            true ) );
      }
   }
}

uint32
SearchMatch::typeCharacterToSearchType( char typeChar )
{
   map<char, uint32>::const_iterator it = c_typeCharacterMap.find(typeChar);
   if ( it == c_typeCharacterMap.end() ) {
      return 0;
   } else {
      return it->second;
   }
}


char
SearchMatch::searchTypeToTypeCharacter( uint32 searchType ) {
   map<uint32, char>::const_iterator it = c_characterTypeMap.find(
      searchType );
   if ( it == c_characterTypeMap.end() ) {
      return 0;
   } else {
      return it->second;
   }
}

const char*
SearchMatch::getLocationName() const
{
   if ( m_locationName ) {      
      return m_locationName;
   } else {
      return "";
   }
}

void
SearchMatch::setLocationName(const char* newLocation)
{
   if ( m_deleteStrings ) {
      delete [] m_locationName;
   } else {
      // Now we have to copy all of the strings.
      m_name = NewStrDup::newStrDup(m_name);
      m_alphaSortingName = NewStrDup::newStrDup(m_alphaSortingName);
      m_deleteStrings = true;
   }
   m_locationName = NewStrDup::newStrDup(newLocation);
}

void
SearchMatch::setSynonymName( const char* synonymName )
{
   delete [] m_synonymName;
   m_synonymName = NewStrDup::newStrDup( synonymName );
}

char*
SearchMatch::
getLocationNameFromRegionAndUp(const SearchMatch* startReg,
                               uint32 allowedSearchTypes,
                               vector<const SearchMatch*>* regions) const
{
   // Calculate stringlength
   int lenSum = 0;
   for ( const SearchMatch* curReg = startReg;
         curReg != NULL;
         curReg = curReg->getRegion(0) ) {
      if ( ( curReg->getType() & allowedSearchTypes ) == 0 ) {
         continue;
      }
      lenSum += strlen(curReg->getName()) + 1 + 3; // Commas
   }

   char* tmpName = new char[ lenSum + 1 ];
   tmpName[0] = '\0'; // Zero terminate
   
   const char* lastName = "";
   for ( const SearchMatch* curReg = startReg;
         curReg != NULL;
         curReg = curReg->getRegion(0) ) {
      if ( ( curReg->getType() & allowedSearchTypes ) == 0 ) {
         continue;
      }
      const char* thisName = curReg->getName();
      if ( StringUtility::strcasecmp(thisName, lastName) != 0 ) {
         // Add comma if it isn't the first region.
         if ( lastName[0] != 0 ) {
            strcat(tmpName, ", ");
         }
         strcat(tmpName, thisName);
         if ( regions ) {
            regions->push_back(curReg);
         }
      }
      lastName = thisName;
   }
   return tmpName;
}

void
addRegionsToMatrix(vector<vector<const SearchMatch*> >& regions,
                   int curIdx,
                   const SearchMatch* curMatch,
                   uint32 reqTypes)
{
   const int nbrRegions = curMatch->getNbrRegions();
   int regionCount = 0;
   int lastIdx = 0;
   for( int i = 0; i < nbrRegions; ++i ) {
      if ( reqTypes & curMatch->getRegion(i)->getType() ) {
         ++regionCount;
         lastIdx = i;
      }
   }
   
   // Copy the curVect unless it is the last one
   for( int i = 0; i < nbrRegions; ++i ) {
      if ( reqTypes & curMatch->getRegion(i)->getType() ) {
         // Choose vector
         if ( i != lastIdx ) {
            // Push back a copy of the vector.
            regions.push_back(regions[curIdx]);
            regions.back().push_back(curMatch->getRegion(i));
            addRegionsToMatrix(regions,
                               regions.size() - 1,
                               curMatch->getRegion(i),
                               reqTypes);
         } else {
            // Last index. Use original vector
            regions[curIdx].push_back(curMatch->getRegion(lastIdx));
            addRegionsToMatrix(regions,
                               curIdx,
                               curMatch->getRegion(lastIdx),
                               reqTypes);
         }
      }
   }
}

int
countRegionLevels(const SearchMatch* match,
                  uint32 reqTypes,
                  int sumSoFar = 0)
{
   if ( match->getNbrRegions() == 0 ) {
      return sumSoFar;
   } else {
      int maxVal = sumSoFar;
      for( int i = 0; i < int(match->getNbrRegions()); ++i ) {
         if ( match->getRegion(i)->getType() & reqTypes ) {
            const SearchMatch* curReg = match->getRegion(i);
            int addVal = 1;
//              if ( strstr(curReg->getName(), match->getName() ) ) {
//                 // They contain the same name. No special info
//                 // Don't count the level.
//                 addVal = 0;
//              }
            int tmpVal = countRegionLevels(curReg, reqTypes,
                                           sumSoFar+addVal);
            maxVal = MAX(tmpVal, maxVal);
         }
      }
      return maxVal;
   }
}

char*
SearchMatch::createLocationName(uint32 reqTypes,
                                vector<const SearchMatch*>* printRegs) const
{
   if ( printRegs ) {
      printRegs->clear();
   }
   const int nbrRegions = getNbrRegions();

   // Start by counting
   int regionCount = 0;
   for( int i=0; i < nbrRegions; ++i ) {
      if ( getRegion(i)->getType() & reqTypes ) {
         regionCount++;
      }
   }
   if ( regionCount == 0 ) {
      return NewStrDup::newStrDup("");
   } else if ( regionCount == 1 ) {
      // FIXME: Add region of regions too.
      return getLocationNameFromRegionAndUp(getRegion(0), reqTypes, printRegs);
   }

   // Tricky, now there are more than one region in the bottom.
   mc2dbg8 << "[SM]: Location will be tricky for " << *this
          << endl;

   // Find the best common region, i.e. the common region
   // with the most regions above it.
   // Calculate the number of rows in vector
   vector<vector<const SearchMatch*> > matrix;
   matrix.push_back(vector<const SearchMatch*>());

   addRegionsToMatrix(matrix, 0,
                      this, reqTypes);

   map<IDPair_t, int> countMap;
   map<IDPair_t, const SearchMatch*> regionMap;
   for(vector<vector<const SearchMatch*> >::const_iterator it = matrix.begin();
       it != matrix.end();
       ++it ) {
      for ( vector<const SearchMatch*>::const_iterator jt = it->begin();
            jt != it->end();
            ++jt ) {
         const SearchMatch* curRegion = *jt;
         countMap[curRegion->getID()]++;
         regionMap.insert(make_pair(curRegion->getID(), curRegion));
      }
   }
   mc2dbg8 << "[SMatch]: Size of matrix = " << matrix.size() << endl;
   mc2dbg8 << "[SMatch]: Region counts : " << endl;
   vector<IDPair_t> candidates;
   for(map<IDPair_t,int>::const_iterator it = countMap.begin();
       it != countMap.end();
       ++it ) {
      mc2dbg8 << "[SMatch]:   " << *regionMap[it->first]
              << ":" << it->second << endl;
      // It is in all the regions.
      if ( it->second == int(matrix.size()) ) {
         candidates.push_back(it->first);
      }
   }

   // The candidates are common regions for all the regions
   // Try to find the common region with the most regions
   int maxLevels = -1;
   const SearchMatch* maxRegion = NULL;
   for( vector<IDPair_t>::const_iterator it = candidates.begin();
        it != candidates.end();
        ++it ) {
      const SearchMatch* curRegion = regionMap[*it];
      int nbrLevelsAbove = countRegionLevels(curRegion,
                                             reqTypes);
      
      mc2dbg8 << "[SMatch]: Candidate " << *curRegion
              << " has " << nbrLevelsAbove
              << " levels " << endl;
      if ( nbrLevelsAbove > maxLevels ) {
         maxRegion = curRegion;
         maxLevels = nbrLevelsAbove;
      }
   }

   // This may or may not work always.
   char* tempLocation = getLocationNameFromRegionAndUp(maxRegion,
                                                       reqTypes,
                                                       printRegs);

   if ( tempLocation[0] != '\0' ) {
      // Location name has been set
      return tempLocation; 
   } else {
      delete [] tempLocation;
   }

   mc2dbg8 << "[SM]: Mucho tricko" << endl;

   // Try again
   MC2String locationName;

   set<MC2String> usedNames;
   // Take the bottom (top) regions and print them
   // Only add maxNbr regions to the answer.
   int nbrAdded = 0;
   const int maxNbr = 3;
   for ( vector<vector<const SearchMatch*> >::const_iterator it =
            matrix.begin();
         it != matrix.end();
         ++it ) {
      if ( usedNames.find( it->back()->getName() ) == usedNames.end() ) {
         if ( ! locationName.empty() ) {
            // Separate these with slashes
            locationName += "/";
         }
         if ( printRegs ) {
            printRegs->push_back(it->back());
         }
         locationName += it->back()->getName();
         ++nbrAdded;
      }
      if ( nbrAdded >= maxNbr ) {
         break;
      }
      usedNames.insert( it->back()->getName() );
   }
   return NewStrDup::newStrDup(locationName.c_str());
}
 
void
SearchMatch::updateLocationName(uint32 reqTypes)
{
#if 1
   // General approach for ordinary types.
   char* tmpName = createLocationName(reqTypes & SEARCH_REGION_TYPES, NULL);
   // FIXME: Do something about the zip-codes too.
   
   // A small optimization
   if ( m_deleteStrings ) {
      delete [] m_locationName;
      m_locationName = tmpName;
   } else {
      setLocationName(tmpName);
      delete [] tmpName;
   }

#else
   // For testing the getRegionsToPrint
   vector<const SearchMatch*> printRegs;
   getRegionsToPrint(printRegs);
   MC2String resString;
   for( vector<const SearchMatch*>::const_iterator it = printRegs.begin();
        it != printRegs.end();
        ++it ) {
      if ( it != printRegs.begin() ) {
         resString += ", ";
      }
      resString += (*it)->getName();
   }
   setLocationName(resString.c_str());
#endif
}


void
SearchMatch::getRegionsToPrint( vector<const SearchMatch*>& printRegs ) const
{
   delete [] createLocationName( SEARCH_MUNICIPALS|SEARCH_BUILT_UP_AREAS|
                                 SEARCH_CITY_PARTS, &printRegs);
   return;
}

SearchMatch*
SearchMatch::createMatch(uint32 searchType,
                         const IDPair_t& id,
                         uint16 offset)
{
   SearchMatchPoints points;
   SearchMatch* match = NULL;
   switch ( searchType ) {
      case SEARCH_STREETS:
         match = new VanillaStreetMatch( 
            id, "", "", offset, 0, false, false );
         break;
      case SEARCH_COMPANIES:
         match = new VanillaCompanyMatch( 
            id, "", "", offset, 0 );
         break;
      case SEARCH_CATEGORIES:
         match = new VanillaCategoryMatch(
            "", 0, "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0, 
            ItemTypes::categoryItem, 0 );
         break;
      case SEARCH_MISC:
         match = new VanillaMiscMatch(
            "", 0, "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0, 
            0, 0, ItemTypes::routeableItem, 0 );
         break;
      case SEARCH_MUNICIPALS:
         match = new VanillaMunicipalMatch( 
            "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0 , 0, 
            ItemTypes::municipalItem, 0 );
         break;
      case SEARCH_BUILT_UP_AREAS:
         match = new VanillaBuiltUpAreaMatch( 
            "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0 , 0, 
            ItemTypes::builtUpAreaItem, 0 );
         break;
      case SEARCH_CITY_PARTS:
         match = new VanillaCityPartMatch( 
            "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0 , 0, 
            ItemTypes::cityPartItem, 0 );
         break;
      case SEARCH_ZIP_CODES:
         match = new VanillaZipCodeMatch( 
            "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0 , 0, 
            ItemTypes::zipCodeItem, 0 );
         break;
      case SEARCH_ZIP_AREAS:
         match = new VanillaZipAreaMatch( 
            "", 0, id.getMapID(), id.getItemID(), points, 0, "", 0 , 0, 
            ItemTypes::zipAreaItem, 0 );
         break;
      case SEARCH_PERSONS:
         match = new VanillaCompanyMatch( id, "", "", offset, 0 );
         match->setType ( SEARCH_PERSONS );
         break;
      case SEARCH_COUNTRIES:
         match = new VanillaCountryMatch( "", MAX_UINT32 );
         break;
      default:
         match = NULL;
         break;
   }
   return match;  
}

SearchMatch*
SearchMatch::createMatch( const char* name,
                          const char* locationName,
                          const IDPair_t& id,
                          const MC2Coordinate& coords,
                          ItemTypes::itemType itemType,
                          uint32 itemSubType )
{
   uint32 searchType = ItemTypes::itemTypeToSearchType( itemType );
   if ( searchType == 0 ) {
      searchType = SEARCH_MISC;
   }
   SearchMatch* match = createMatch( searchType, id, 0 );
   if ( match == NULL ) {
      return NULL;
   }
   match->setName( name );
   match->setLocationName( name );
   match->setCoords( coords );
   match->setItemType( itemType );
   match->setItemSubType( itemSubType );
   return match;
}

SearchMatch*
SearchMatch::createExtMatch( const char* inStr )
{

   // Format: X(type):(lat):(lon):(angle):(itemtype):(extid):(extsource hex)
   ++inStr; // Skip the 'X'

   uint32 searchType = typeCharacterToSearchType( inStr[ 0 ] );
   char* pos = StringUtility::strchr( inStr + 1, ':' );
   if ( pos == NULL ) {
      return NULL;
   }
   MC2Coordinate coord;
   // Get latitude
   char* tmpPos = NULL;
   coord.lat = strtoul( pos + 1, &tmpPos, 16 );
   if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
      return NULL;
   }
   // Get longitude
   pos = tmpPos;
   coord.lon = strtoul( pos + 1, &tmpPos, 16 );
   if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
      return NULL;
   }
   // Get angle
   pos = tmpPos;
   uint16 angle = strtoul( pos + 1, &tmpPos, 16 );
   // Angle is increased by one before printed in matchToString.
   angle--;
   if ( tmpPos == NULL || (tmpPos[ 0 ] != ':' && tmpPos[ 0 ] != '\0') )
   {
      return NULL;
   }
   // Get item type
   ItemTypes::itemType itemType = ItemTypes::streetSegmentItem;
   pos = tmpPos;
   itemType = static_cast<ItemTypes::itemType>
      ( strtoul( pos + 1, &tmpPos, 16 ) );
   if ( tmpPos == NULL || (tmpPos[ 0 ] != ':' && tmpPos[ 0 ] != '\0') ) {
      return NULL;
   }

   // Get the external source id ( string )
   pos = tmpPos;
   char* colon = strchr( pos + 1, ':' );
   if ( colon == NULL ) {
      return NULL;
   }
   MC2String extID( pos + 1, colon - ( pos + 1 ) );

   uint32 extSource = 0;
   MC2String name;

   // Get the external source ( hex )
   pos = colon;
   extSource = strtoul( pos + 1, &tmpPos, 16 );
   if ( tmpPos == NULL || (tmpPos[ 0 ] != ':' && tmpPos[ 0 ] != '\0') ) {
      return NULL;
   }

   // Create the match and set all stuff.
   SearchMatch* tmpMatch = createMatch( searchType, IDPair_t() );
   tmpMatch->setCoords( coord );
   tmpMatch->m_extID.swap ( extID );
   tmpMatch->m_extSource = extSource;
   tmpMatch->m_angle = angle;
   tmpMatch->setItemType( itemType );
   tmpMatch->setName( name.c_str() );
   return tmpMatch;
}

SearchMatch*
SearchMatch::createMatch( const MC2String& str )
{
   return createMatch( str.c_str() );
}

SearchMatch*
SearchMatch::createMatch(const char* inStr)
{
   MC2String strTrimed = StringUtility::trimStartEnd( inStr );
   inStr = strTrimed.c_str();
   if ( inStr[0] == 'X' || inStr[0] == 'Y' ) {
      return createExtMatch( inStr );
   }
   uint32 len = strlen( inStr );
   if ( len < 5 ) {
      return NULL;
   }
   if ( (inStr[ 0 ] == 'K' || inStr[ 0 ] == 'C') ) {
      // utf-8:
      // "C:%ld:%ld:%ld:%s",
      // iso:
      // "K:%ld:%ld:%ld:%s",
      char stringType = inStr[ 0 ];
      char* pos = (char*)inStr + 1;
      if ( *pos != ':' ) {
         return NULL;
      }
      // Lat
      char* tmpPos = NULL;
      int32 lat = strtoul( pos + 1, &tmpPos, 0 );
      if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
         return NULL;
      }
      pos = tmpPos;
      // Lon
      int32 lon = strtoul( pos + 1, &tmpPos, 0 );
      if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
         return NULL;
      }
      pos = tmpPos;
      // Closed
      uint32 closed = strtoul( pos + 1, &tmpPos, 0 );
      if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
         return NULL;
      }
      pos = tmpPos;
      // Name
      const char* inName = pos + 1;
      MC2String mc2String;
      if ( stringType == 'C' ) {
         // Utf-8 inName
         mc2String = UTF8Util::utf8ToMc2( inName );
      } else { // iso inName 
         mc2String = UTF8Util::isoToMc2( inName );
      }
      inName = mc2String.c_str();

      uint32 type = SEARCH_STREETS | SEARCH_COMPANIES;
      if ( closed ) {
         type = SEARCH_ALL_REGION_TYPES | SEARCH_MISC;
      }

      // FIXME: This will never work ok, client should send type
      ItemTypes::itemType itype = ItemTypes::pointOfInterestItem;
      if ( closed ) {
         type = ItemTypes::builtUpAreaItem;
      }

      SearchMatch* res = createMatch( SEARCH_STREETS, IDPair_t(), 0 );
      MC2_ASSERT( res != NULL );
      res->setName( inName );      
      res->setCoords( lat, lon );
      res->setItemType( itype );
      res->setType( type );
      return res;
   } else if ( inStr[ 0 ] == 'P' ) {
      // "Persistent" id placeholder
      mc2log << "[SearchMatch]: Persistent ids not handled here!" << endl;
      return NULL;
   } else {
      // s:3004532:0:[offset]:itemtype[:optionalsubclassdata]
      // Type is in first position.
      uint32 searchType = typeCharacterToSearchType( inStr[ 0 ] );
      const char* pos = inStr + 1;
      if ( *pos != ':' ) {
         return NULL;
      }
      char* tmpPos = NULL;
      uint32 itemID = strtoul( pos + 1, &tmpPos, 16 );
      if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
         return NULL;
      }
      pos = tmpPos;
      uint32 mapID = strtoul( pos + 1, &tmpPos, 16 );
      if ( tmpPos == NULL || tmpPos[ 0 ] != ':' ) {
         return NULL;
      }
      pos = tmpPos;
      uint16 offset = strtoul( pos + 1, &tmpPos, 16 );
      if ( tmpPos == NULL || (tmpPos[ 0 ] != ':' && tmpPos[ 0 ] != '\0') )
      {
         return NULL;
      }
      pos = tmpPos;
      ItemTypes::itemType itemType = static_cast<ItemTypes::itemType>
         (strtoul( pos + 1, &tmpPos, 16 ));      
      if ( tmpPos == NULL || (tmpPos[ 0 ] != ':' && tmpPos[ 0 ] != '\0') )
      {
         return NULL;
      }
      
      
      SearchMatch* tmpMatch =
         createMatch(searchType, IDPair_t(mapID,itemID), offset);
      if ( tmpMatch != NULL ) {
         tmpMatch->setItemType( itemType );
      }
      return tmpMatch;
   }
}

SearchMatch*
SearchMatch::createMatch(const Packet* packet, int& pos, bool compactForm)
{

   uint32 typeID     = packet->incReadLong(pos);  
   uint32 searchType = packet->incReadLong(pos);

   // Use high bit to check if old version
   bool oldVersion = ( typeID & 0x80000000 ) == 0;   
   // Remove high bit
   typeID &= 0x7fffffff;

   if ( oldVersion ) {
      mc2dbg8 << "[SM]: Old version of save has been used" << endl;
   }
   
   SearchMatch* theMatch = NULL;

   // Check if it is an overview match or ordinary.
   if ( typeID != VANILLAMATCH ) {
      mc2dbg8 << "[SM]: Will create overview match" << endl;
      mc2dbg8 << "[SM]: type id = " << typeID << endl;
      theMatch = new OverviewMatch( IDPair_t() );
      theMatch->setType(searchType);
   } else {
      theMatch = createMatch(searchType);
   }

   // Try to load it
   if ( theMatch != NULL ) {
      theMatch->load(packet, pos, compactForm);
   } else {
      mc2log << error << "[SearchMatch]: Could not create match from packet."
             << endl;
      mc2log << error << "[SearchMatch]: Type is 0x" << hex << searchType
             << dec << " and typeID = " << typeID << endl;
   }
   return theMatch;
}


int
SearchMatch::load(const Packet* packet, int& pos, bool compactForm)
{

   // Currently the opposit of this function is
   // almost SearchMatch::save. At last.
   // The typeid and type of the match should already
   // be read by createMatch when this function is called.
   mc2dbg8  << "[SM]: Load - type = " << hex << m_type << dec
            << " pos = " << pos << endl;

   int startPos = pos;

   LoadLengthHelper llh( const_cast<Packet* >( packet ), pos );
   llh.loadLength( pos );

   // Update the stuff that is sent in the match.
   char* name;
   packet->incReadString(pos, name);
   char* locationName;
   packet->incReadString(pos, locationName);
   char* alphaSortingName;
   packet->incReadString( pos, alphaSortingName );

   if ( !compactForm ) {

      m_itemType  = ItemTypes::itemType(packet->incReadShort( pos ));
      m_itemSubtype = uint16(packet->incReadLong( pos ));


      m_points.load(packet, pos, false);
      m_points.setTypes(m_type, m_itemType, m_itemSubtype);

      m_distance = packet->incReadLong( pos );

      packet->incReadShort( pos, m_angle );
      packet->incReadShort( pos, m_extSource );
      packet->incReadString( pos, m_extID );

      delete [] m_synonymName;
      m_synonymName = NULL;
      const char* tmpSyn = packet->incReadString( pos );
      if ( tmpSyn[0] != '\0' ) {
         m_synonymName = NewStrDup::newStrDup( tmpSyn );
      }

      // Read number of item infos
      int nbrInfos = packet->incReadLong( pos );
      // Create new vector to put it into.
      ItemInfoVector infos( nbrInfos );
      m_itemInfos.swap( infos );
      // And load it
      for ( ItemInfoVector::iterator it = m_itemInfos.begin();
            it != m_itemInfos.end();
            ++it ) {
         it->load( packet, pos );
      }

      packet->incAlignReadLong( pos );

      mc2dbg8 << "[SM]: Angle = " << m_angle
              << ", m_extSource = " << m_extSource
              << ", m_extID = " << m_extID 
              << ", pos = " << pos << endl;
      

   }
   
   m_mapRights.load( packet, pos );
   m_id.first    = packet->incReadLong( pos );
   m_id.second   = packet->incReadLong( pos );
   
   m_coordinates.lat = packet->incReadLong( pos );
   m_coordinates.lon = packet->incReadLong( pos );

   int nbrRegions = packet->incReadLong( pos );

   for ( int i = 0 ; i < nbrRegions ; ++i ) {
      VanillaRegionMatch* region =
         static_cast<VanillaRegionMatch*>(createMatch(packet, pos, true));
      mc2dbg8  << "[SM]::load type of region is "
               << hex << region->getType() << dec
               << endl;
      m_regions.push_back( make_pair( region, true ) );
   }
   if ( m_deleteStrings ) {
      delete [] m_name;
      delete [] m_alphaSortingName;
      delete [] m_locationName;
   }

   if ( packet->incReadByte( pos ) == 0 ) {
      m_additionalInfoExists = false;
   } else {
      m_additionalInfoExists = true;
   }


#if 0
   m_deleteStrings    = true;
   m_name             = NewStrDup::newStrDup(name);
   m_alphaSortingName = NewStrDup::newStrDup(alphaSortingName);
   m_locationName     = NewStrDup::newStrDup(locationName);
#else
   m_name             = name;
   m_alphaSortingName = alphaSortingName;
   m_locationName     = locationName;
   m_deleteStrings    = false;
#endif

   m_nameInfo = 0;
   m_locationNameInfo = 0;
   
   llh.skipUnknown( pos );
   
   
   return pos - startPos;
}

int
SearchMatch::save(Packet* packet, int& pos, bool compactForm) const
{

   // Reserve more mem if there are less than 2000 bytes left
   if ( pos > int( packet->getBufSize() - VP_MIN_FREE - VP_SOME_LESS_THAN_MIN_FREE ) ) {
      // Reserve at most 3000 more bytes, since that is the
      // number that limits the adding in VanillaSearchReply.
      int newSize = packet->getBufSize() + VP_MIN_FREE;
      mc2dbg8 << "[SearchMatch]: Resizing packet from "
              << packet->getBufSize() << " to "
              << newSize << endl;
      int oldLength = packet->getLength();
      packet->setLength( pos );
      packet->resize( newSize );
      packet->setLength( oldLength );
   }

   // Start writing the variable data now
   int startPos = pos;

   packet->incWriteLong(pos, m_typeid | 0x80000000 );
   // Set the high bit of the type to indicate new version of save
   packet->incWriteLong(pos, m_type );

   SaveLengthHelper slh( packet, pos );
   slh.writeDummyLength( pos );

   packet->incWriteString(pos, getName());
   packet->incWriteString(pos, getLocationName());
   packet->incWriteString(pos, m_alphaSortingName);

   if ( !compactForm ) {

      packet->incWriteShort( pos, m_itemType );
      packet->incWriteLong( pos, m_itemSubtype );
      // we need to write this after itemType and itemSubType
      // since we need to set type in load before loading points
      m_points.save(packet, pos, false);

      packet->incWriteLong( pos, m_distance );
      packet->incWriteShort( pos, m_angle );
      packet->incWriteShort( pos, m_extSource );      
      packet->incWriteString( pos, m_extID );

      packet->incWriteString( pos, getSynonymName() );

      packet->incWriteLong( pos, m_itemInfos.size() );
      for ( ItemInfoVector::const_iterator it = m_itemInfos.begin();
            it != m_itemInfos.end();
            ++it ) {
         it->save( packet, pos );
      }

      packet->incAlignWriteLong( pos ); // Keep this last.
   }

   m_mapRights.save( packet, pos );
   packet->incWriteLong(pos, m_id.first);
   packet->incWriteLong(pos, m_id.second);
 
   packet->incWriteLong(pos, m_coordinates.lat);
   packet->incWriteLong(pos, m_coordinates.lon);

   packet->incWriteLong(pos, getNbrRegions());
   for( uint32 i=0; i < getNbrRegions(); ++i ) {
      mc2dbg8 << "[SM]: Saving region with type " << hex
              << getRegion(i)->getType() << dec << endl;
      getRegion(i)->save(packet, pos, true);
   }

   if ( m_additionalInfoExists ) {
      packet->incWriteByte( pos, 1 );
   } else {
      packet->incWriteByte( pos, 0 );
   }

   slh.updateLengthUsingEndPos( pos );

   return pos - startPos;

}

bool 
SearchMatch::matchToString( const SearchMatch* match,
                            char* str, uint32 len, bool extradata )
{
   if ( len < searchMatchBaseStrLength ) {
      return false;
   }
   
   if ( match->m_extSource == 0 ) { // Means that the match cometh from MC2.
      uint32 mapID = match->getMapID();
      uint32 itemID = match->getItemID();
      sprintf( str, "%c:%X:%X:%X:%X", 
               searchTypeToTypeCharacter( match->getType() ),
               itemID, mapID, match->getOffset(),
               match->getItemType() );
   } else {
      // Invalid angle will be 0 instead of FFFF.
      uint16 writeAngle = match->m_angle + 1;
      snprintf( str, len, "X%c:%X:%X:%X:%X:%s:%X", 
                searchTypeToTypeCharacter( match->getType() ),
                match->m_coordinates.lat,
                match->m_coordinates.lon,
                writeAngle,
                match->getItemType(),
                match->m_extID.c_str(),
                match->m_extSource );
   }
   return true;
}

MC2String
SearchMatch::matchToString(bool extradata) const
{
   const int len = searchMatchBaseStrLength;
   char* tmpStr = new char[len];
   ScopedArray<char> tmpAuto(tmpStr);
   matchToString(this, tmpStr, len, extradata);
   return MC2String(tmpStr);
}

void
SearchMatch::mergeToSaneItemInfos( LangTypes::language_t language, 
                                   uint32 topRegion ) {
   static const uint32 ADDRESSIDX     = 0;
   static const uint32 HOUSENBRIDX    = 1;
   static const uint32 ZIPCODEIDX     = 2;
   static const uint32 COMPZIPIDX     = 3;
   static const uint32 FULLADDRESSIDX = 4;
   static const uint32 ZIPAREAIDX     = 5;
   static const uint32 MAXIDX         = 6;
   
#define copyEntry( idx ) values[idx] = it->getVal(); \
                         it = m_itemInfos.erase( it ); \
                         endIt = m_itemInfos.end();

   MC2String values[MAXIDX];   // Vector for holding values
   
   ItemInfoVector::iterator endIt = m_itemInfos.end();
   for (ItemInfoVector::iterator it = m_itemInfos.begin() ;
        it != endIt ; ) {
      ItemInfoEnums::InfoType type = it->getInfoType();
      if ( type == ItemInfoEnums::vis_address ) {
         copyEntry( ADDRESSIDX );
      } else if ( type == ItemInfoEnums::vis_house_nbr ) {
         copyEntry( HOUSENBRIDX );
      } else if ( type == ItemInfoEnums::vis_zip_code ) {
         copyEntry( ZIPCODEIDX );
      } else if ( type == ItemInfoEnums::vis_complete_zip ) {
         copyEntry( COMPZIPIDX );
      } else if ( type == ItemInfoEnums::vis_full_address ) {
         copyEntry( FULLADDRESSIDX );
      } else if ( type == ItemInfoEnums::Vis_zip_area ) {
         copyEntry( ZIPAREAIDX );
      } else {
         ++it;
      }
   }

   using namespace NationalAddressFormatter;

   MC2String address = values[ADDRESSIDX];
   MC2String houseNbr = values[HOUSENBRIDX];
   
   if ( address.empty() && 
        !values[FULLADDRESSIDX].empty() ) {
      address = values[FULLADDRESSIDX];
   }
   
   if ( values[HOUSENBRIDX].empty() ) {
      // No house number supplied. Try to get it from address
      int streetNbr = 0;
      MC2String streetName;
      StringSearchUtility::simpleGetStreetNumberAndName( address, streetNbr, streetName );
      if ( streetNbr != 0 ) {
         // We got a number. Use the new strings
         address = streetName;
         houseNbr = STLStringUtility::int2str( streetNbr );
      }
   }

   if ( !houseNbr.empty() ) {
      address = formatAddress( topRegion, address, houseNbr );
   }

   values[ADDRESSIDX] = address;
   values[FULLADDRESSIDX] = address;

   MC2String separator = address.empty() ? "" : ", ";
   if ( !values[ZIPAREAIDX].empty() ) {
      if ( !values[ZIPCODEIDX].empty() ) {
         values[FULLADDRESSIDX] += separator;
         values[FULLADDRESSIDX] += formatZip( topRegion, values[ZIPCODEIDX], 
                                              values[ZIPAREAIDX] );
      } else {
         values[FULLADDRESSIDX] += separator;
         values[FULLADDRESSIDX] += values[ZIPAREAIDX];
      }
   }

   // Add new fields
   if ( !values[ FULLADDRESSIDX ].empty() ) {
      MC2String key = 
         ItemInfoEnums:: infoTypeToString( language,
                                           ItemInfoEnums::vis_full_address);
      m_itemInfos.insert( m_itemInfos.begin(),
                          ItemInfoEntry( key, 
                                         values[ FULLADDRESSIDX ],                      
                                         ItemInfoEnums::vis_full_address ) );
   }

   if ( ! values[ ADDRESSIDX ].empty() ) {
      MC2String key = 
         ItemInfoEnums:: infoTypeToString( language,
                                           ItemInfoEnums::vis_address);
      m_itemInfos.insert( m_itemInfos.begin(), 
                          ItemInfoEntry( key, 
                                         values[ ADDRESSIDX ],                          
                                         ItemInfoEnums::vis_address ) );
   }
   
#undef copyEntry
}

// --------------- VanillaMatch ---------------------

VanillaMatch::VanillaMatch( const VanillaMatch& vm )
      :  SearchMatch( vm )
{
}


VanillaMatch::~VanillaMatch() {
   // else
   // The char*s are pointers into the associated packet, and 
   // are deleted by deleting that packet.

   // The array of names are created outside this class, in a
   // Packet, that can not have any member variables. Therefore
   // it is deleted here!
   //         delete m_names;
   //         delete m_alphaSortingName; don't! it points into the packet!
}

void VanillaMatch::addToPacket( VanillaSearchReplyPacket *p,
                                bool increaseCount ) const
{
   p->addMatch(this, increaseCount);
}


// --------------- VanillaRegionMatch ----------------

VanillaRegionMatch::
VanillaRegionMatch( const char *name, uint32 nameInfo,
                    uint32 mapID,
                    uint32 itemID,
                    uint32 type,
                    const SearchMatchPoints& points,
                    uint16 source,
                    const char *alphaSortingName,
                    uint32 location,
                    uint8 restrictions,
                    ItemTypes::itemType itemType,
                    uint16 itemSubtype)
      : VanillaMatch(name, nameInfo, NULL, INVALID_STRING_INFO,
                     mapID, itemID,
                     type,
                     points, source, alphaSortingName, location,
                     restrictions, itemType, itemSubtype)
{
}

int
VanillaRegionMatch::load(const Packet* packet, int& pos, bool compactForm)
{
   int startPos = pos;
   VanillaMatch::load(packet, pos, compactForm );   
   m_restrictions = packet->incReadByte(pos);
   return pos - startPos;
}

int
VanillaRegionMatch::save(Packet* packet, int& pos, bool compactForm) const
{
   int startPos = pos;
   VanillaMatch::save(packet, pos, compactForm);
   packet->incWriteByte(pos, m_restrictions);
   return pos - startPos;
}

// --------------- VanillaMiscMatch -------------------

int
VanillaMiscMatch::load(const Packet* packet, int& pos, bool compactForm)
{
   int startPos = pos;
   VanillaMatch::load(packet, pos, compactForm);
   m_miscType             = packet->incReadLong(pos);
   m_restrictions         = packet->incReadByte(pos);
   return pos - startPos;
}

int
VanillaMiscMatch::save(Packet* packet, int& pos, bool compactForm) const
{
   int startPos = pos;
   VanillaMatch::save(packet, pos, compactForm);
   packet->incWriteLong(pos, m_miscType);
   packet->incWriteByte(pos, m_restrictions);
   return pos - startPos;
}


// --------------- VanillaCategoryMatch ---------------

int
VanillaCategoryMatch::load(const Packet* packet, int& pos, bool compactForm)
{
   int startPos = pos;
   VanillaMatch::load(packet, pos, compactForm);
   m_restrictions         = packet->incReadByte(pos);
   return pos - startPos;
}


int
VanillaCategoryMatch::save(Packet* packet, int& pos, bool compactForm) const
{
   int startPos = pos;
   VanillaMatch::save(packet, pos, compactForm);
   packet->incWriteByte(pos, m_restrictions);
   return pos - startPos;
}

// --------------- VanillaBuiltUpAreaMatch ----------------

VanillaBuiltUpAreaMatch::
VanillaBuiltUpAreaMatch( const char *name, uint32 nameInfo,
                         uint32 mapID,
                         uint32 itemID,
                         const SearchMatchPoints& points,
                         uint16 source,
                         const char *alphaSortingName,
                         uint32 location,
                         uint8 restrictions,
                         ItemTypes::itemType itemType,
                         uint16 itemSubtype)
      : VanillaRegionMatch(name, nameInfo, mapID, itemID,
                           SEARCH_BUILT_UP_AREAS,
                           points, source,
                           alphaSortingName, location,
                           restrictions, itemType, itemSubtype)
{
}

// --------------- VanillaCountryMatch ----------------

VanillaCountryMatch::VanillaCountryMatch( const char *name,
                                          uint32 topRegionID ):
   VanillaRegionMatch( name,
                       0, // name info
                       0, 0, // map and item id
                       SEARCH_COUNTRIES,
                       SearchMatchPoints(),
                       0, // source
                       "", // alphaSortingName
                       0, // location
                       0, // restriction
                       ItemTypes::builtUpAreaItem, // dummy item type
                       0 ), // no subtype
   m_topRegionID( topRegionID ) {
}

int
VanillaCountryMatch::load( const Packet* packet, int& pos, bool compactForm ) {
   int startPos = pos;
   VanillaRegionMatch::load( packet, pos, compactForm );

   m_topRegionID = packet->incReadLong( pos );

   return pos - startPos;
}

int
VanillaCountryMatch::save( Packet* packet, int& pos, bool compactForm ) const {
   int startPos = pos;
   VanillaRegionMatch::save( packet, pos, compactForm );

   packet->incWriteLong( pos, m_topRegionID );

   return pos - startPos;
}

// --------------- VanillaMunicipalMatch ----------------

VanillaMunicipalMatch::VanillaMunicipalMatch( const char *name,
                                              uint32 nameInfo,
                                              uint32 mapID,
                                              uint32 itemID,
                                              const SearchMatchPoints& points,
                                              uint16 source,
                                              const char *alphaSortingName,
                                              uint32 location,
                                              uint8 restrictions,
                                              ItemTypes::itemType itemType,
                                              uint16 itemSubtype)
      : VanillaRegionMatch(name, nameInfo, mapID, itemID,
                           SEARCH_MUNICIPALS,
                           points, source, alphaSortingName, location,
                           restrictions, itemType, itemSubtype)
{
}

// --------------- VanillaCityPartMatch ---------------------

VanillaCityPartMatch::
VanillaCityPartMatch( const char *name, uint32 nameInfo,
                      uint32 mapID,
                      uint32 itemID,
                      const SearchMatchPoints& points,
                      uint16 source,
                      const char *alphaSortingName,
                      uint32 location,
                      uint8 restrictions,
                      ItemTypes::itemType itemType,
                      uint16 itemSubtype)
      : VanillaRegionMatch(name, nameInfo, mapID, itemID,
                           SEARCH_CITY_PARTS,
                           points, source, alphaSortingName, location,
                           restrictions, itemType, itemSubtype)
{
}

// --------------- VanillaZipCodeMatch ---------------------

VanillaZipCodeMatch::
VanillaZipCodeMatch( const char *name, uint32 nameInfo,
                     uint32 mapID,
                     uint32 itemID,
                     const SearchMatchPoints& points,
                     uint16 source,
                     const char *alphaSortingName,
                     uint32 location,
                     uint8 restrictions,
                     ItemTypes::itemType itemType,
                     uint16 itemSubtype)
      : VanillaRegionMatch(name, nameInfo, mapID, itemID,
                           SEARCH_ZIP_CODES,
                           points, source, alphaSortingName, location,
                           restrictions, itemType, itemSubtype)
{
}

// --------------- VanillaZipAreaMatch ---------------------


VanillaZipAreaMatch::
VanillaZipAreaMatch( const char *name, uint32 nameInfo,
                     uint32 mapID,
                     uint32 itemID,
                     const SearchMatchPoints& points,
                     uint16 source,
                     const char *alphaSortingName,
                     uint32 location,
                     uint8 restrictions,
                     ItemTypes::itemType itemType,
                     uint16 itemSubtype)
      : VanillaRegionMatch(name, nameInfo, mapID, itemID,
                           SEARCH_ZIP_AREAS,
                           points, source, alphaSortingName, location,
                           restrictions, itemType, itemSubtype)
{
}

// --------------- VanillaStreetMatch ---------------------

VanillaStreetMatch::VanillaStreetMatch( const char *name,
                                        uint32 nameInfo,
                                        const char *locationName,
                                        uint32 locationNameInfo,
                                        uint32 mapID,
                                        uint32 itemID,
                                        const SearchMatchPoints& points,
                                        uint16 source,
                                        const char *alphaSortingName,
                                        uint32 location,
                                        uint32 streetSegmentItemID,
                                        uint16 streetSegmentOffset,
                                        uint16 streetNumber,
                                        SearchTypes::side_t side,
                                        uint8 restrictions,
                                        ItemTypes::itemType itemType,
                                        uint16 itemSubtype)
      : VanillaMatch(name, nameInfo, locationName, locationNameInfo,
                     mapID, itemID, SEARCH_STREETS,
                     points, source, alphaSortingName, location,
                     restrictions, itemType, itemSubtype),
   m_streetSegmentItemID(streetSegmentItemID),
   m_streetSegmentOffset(streetSegmentOffset),
   m_streetNumber(streetNumber),
   m_side(side)
{
}

VanillaStreetMatch::VanillaStreetMatch(const VanillaStreetMatch& vm)
      : VanillaMatch(vm),
        m_streetSegmentItemID(vm.m_streetSegmentItemID),
        m_streetSegmentOffset(vm.m_streetSegmentOffset),
        m_streetNumber(vm.m_streetNumber),
        m_side(vm.m_side)       
{
}

VanillaStreetMatch::~VanillaStreetMatch() 
{
}

int
VanillaStreetMatch::load(const Packet* packet, int& pos, bool compactForm)
{
   int startPos = pos;
   VanillaMatch::load(packet, pos, compactForm);
   m_streetSegmentItemID  = packet->incReadLong(pos);
   m_streetSegmentOffset  = packet->incReadShort(pos);
   m_streetNumber         = packet->incReadShort(pos);
   m_side = SearchTypes::side_t(packet->incReadByte(pos));
   m_restrictions         = packet->incReadByte(pos);
   byte n;
   n = packet->incReadByte(pos);
   if( n == 0 )
      m_streetNumberFirst = false;
   else
      m_streetNumberFirst = true;
   n = packet->incReadByte(pos);
   if( n == 0 )
      m_streetNumberComma = false;
   else
      m_streetNumberComma = true;
   return pos - startPos;
}


int
VanillaStreetMatch::save(Packet* packet, int& pos, bool compactForm) const
{
   int startPos = pos;
   VanillaMatch::save(packet, pos, compactForm);
   packet->incWriteLong(pos,  m_streetSegmentItemID);
   packet->incWriteShort(pos, m_streetSegmentOffset);
   packet->incWriteShort(pos, m_streetNumber);
   packet->incWriteByte(pos,  m_side);
   packet->incWriteByte(pos,  m_restrictions);
   if( m_streetNumberFirst )
      packet->incWriteByte(pos, 1);
   else
      packet->incWriteByte(pos, 0);
   if( m_streetNumberComma )
      packet->incWriteByte(pos, 1);
   else
      packet->incWriteByte(pos, 0);

   return pos - startPos;
}


// --------------- VanillaCompanyMatch -----------------
int
VanillaCompanyMatch::load(const Packet* packet, int& pos, bool compactForm)
{
   int startPos = pos;
   VanillaMatch::load(packet, pos, compactForm);
   m_streetSegmentID = packet->incReadLong(pos);
   m_streetOffset    = packet->incReadShort(pos);
   m_streetNumber    = packet->incReadShort(pos);
   m_side = SearchTypes::side_t(packet->incReadByte(pos));
   m_restrictions    = packet->incReadByte(pos);
   m_specialImage = packet->incReadString( pos );
   m_companyName = packet->incReadString( pos );
   uint32 numCategories = packet->incReadLong( pos );
   m_categories.resize( numCategories );
   for ( uint32 catIdx = 0; catIdx < numCategories; ++catIdx ) {
      m_categories[ catIdx ] = packet->incReadLong( pos );
   }

   uint32 numImageUrls = packet->incReadLong( pos );
   m_imageUrls.reserve( numImageUrls );
   uint32 numReviews = packet->incReadLong( pos );
   m_reviews.reserve( numReviews );

   for( uint32 i = 0; i < numImageUrls; i++ ) {
      m_imageUrls.push_back( packet->incReadString( pos ) );
   }

   int8 rating;
   MC2String reviewer;
   MC2String date;
   MC2String reviewText;
   for( uint32 j = 0; j < numReviews; j++ ) {
      rating = packet->incReadByte( pos );
      reviewer = packet->incReadString( pos );
      date = packet->incReadString( pos );
      reviewText = packet->incReadString( pos );

      m_reviews.push_back( POIReview( rating, reviewer, date, reviewText ) );
   }
   
   return pos - startPos;
}


int
VanillaCompanyMatch::save(Packet* packet, int& pos, bool compactForm) const
{
   int startPos = pos;
   VanillaMatch::save(packet, pos, compactForm);
   packet->incWriteLong(pos,  m_streetSegmentID);
   packet->incWriteShort(pos, m_streetOffset);
   packet->incWriteShort(pos, m_streetNumber);
   packet->incWriteByte(pos,  m_side);
   packet->incWriteByte(pos,  m_restrictions);
   packet->incWriteString( pos, m_specialImage );
   packet->incWriteString( pos, m_companyName );
   packet->incWriteLong( pos, m_categories.size() );
   for ( Categories::const_iterator catIt = m_categories.begin(),
            catItEnd = m_categories.end();
         catIt != catItEnd;
         ++catIt ) {
      packet->incWriteLong( pos, *catIt );
   }

   packet->incWriteLong( pos, m_imageUrls.size() );
   packet->incWriteLong( pos, m_reviews.size() );
   
   for ( ImageURLs::const_iterator imgIt = m_imageUrls.begin();
         imgIt != m_imageUrls.end(); ++imgIt ) {
      packet->incWriteString( pos, *imgIt );
   }

   for (Reviews::const_iterator revIt = m_reviews.begin();
        revIt != m_reviews.end(); ++revIt ) {
      packet->incWriteByte( pos, revIt->getRating() );
      packet->incWriteString( pos, revIt->getReviewer() );
      packet->incWriteString( pos, revIt->getDate() );
      packet->incWriteString( pos, revIt->getReviewText() );
   }

   return pos - startPos;
}

const MC2String&
VanillaCompanyMatch::getCleanCompanyName() const {
   return m_companyName;
}

void
VanillaCompanyMatch::setCleanCompanyName( const MC2String& str ) {
   m_companyName = str;
}

const VanillaCompanyMatch::ImageURLs& 
VanillaCompanyMatch::getImageURLs() const {
   return m_imageUrls;
}

const VanillaCompanyMatch::Reviews& 
VanillaCompanyMatch::getReviews() const {
   return m_reviews;
}

void 
VanillaCompanyMatch::swapImageURLs( 
   VanillaCompanyMatch::ImageURLs& urls ) {
   m_imageUrls.swap( urls );
}

void 
VanillaCompanyMatch::swapReviews( 
   VanillaCompanyMatch::Reviews& reviews ) {
   m_reviews.swap( reviews );
}


// --------------- OverviewMatch   ---------------------

void
OverviewMatch::addToPacket( OverviewSearchReplyPacket *p )
{
   p->addOverviewMatch( this );
}

OverviewMatch::OverviewMatch( const OverviewMatch& original )
      : SearchMatch( original ), m_overviewID(original.m_overviewID)
{
   setFromUniqueOrFullMatchPacket( original.fromUniqueOrFullMatchPacket());
   setFromFullMatchPacket( original.fromFullMatchPacket() );
   m_radiusMeters = original.getRadiusMeters();
   m_nbrRemovedCharacters = original.getNbrRemovedCharacters();
}

OverviewMatch::OverviewMatch( const SearchMatch& original)
      : SearchMatch ( original )
{
   // Reset some variables.
   m_typeid = OVERVIEWMATCH;
   setFromFullMatchPacket(false);
   setFromUniqueOrFullMatchPacket(false);
   m_radiusMeters = 0;
}


int
OverviewMatch::load(const Packet* packet, int& pos, bool compactForm)
{
   // FIXME: What about full etc. packet?
   int startPos = pos;
   SearchMatch::load(packet, pos, compactForm);
   m_restrictions            = packet->incReadByte(pos);
   m_overviewID.first        = packet->incReadLong(pos);
   m_overviewID.second       = packet->incReadLong(pos);
   m_radiusMeters            = packet->incReadLong(pos);
   m_nbrRemovedCharacters    = packet->incReadByte(pos);
   
   return pos - startPos;
}

int
OverviewMatch::save(Packet* packet, int& pos, bool compactForm) const
{
   int startPos = pos;
   SearchMatch::save(packet, pos, compactForm);
   packet->incWriteByte(pos, m_restrictions);
   packet->incWriteLong(pos, m_overviewID.first);
   packet->incWriteLong(pos, m_overviewID.second);
   packet->incWriteLong(pos, m_radiusMeters);
   packet->incWriteByte(pos, m_nbrRemovedCharacters);
   
   return pos - startPos;
}


OverviewMatch*
OverviewMatch::getSingleMatch( uint32 index ) {
   MC2_ASSERT( index == 0 );
   return new OverviewMatch(*this);
}



// --------------- SearchMatchLink ---------------------

SearchMatchLink::SearchMatchLink( SearchMatch* match ) {
   m_match = match;
   m_next = NULL;
}


SearchMatchLink::~SearchMatchLink() {
}



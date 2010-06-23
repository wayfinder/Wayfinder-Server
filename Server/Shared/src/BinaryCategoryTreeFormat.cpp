/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BinaryCategoryTreeFormat.h"
#include "STLUtility.h"
#include "MC2CRC32.h"
#include "STLStringUtility.h"
#include "LocaleUtility.h"

using namespace CategoryTreeUtils;

typedef vector<CategoryTree::StandaloneNode> Categories;

namespace {

/**
 * Gets all strings (names and icons) from the categories and puts them
 * in the stringMap, setting all indexes to zero.
 */
void getAllStrings( map<MC2String, int32>& stringMap,
                    const Categories& categories ) {

   Categories::const_iterator itr = categories.begin();
   Categories::const_iterator itrEnd = categories.end();
   for ( ; itr != itrEnd; ++itr ) {
      stringMap[ (*itr).getName() ] = 0;
      stringMap[ (*itr).getIcon() ] = 0;
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
 * Used to sort a sequence of CategoryID alphabetically by looking up their
 * name with a map, uses LocaleUtility to sort correctly for a specific 
 * language's locale.
 */
class LocaleCompareCategory {
public:
   LocaleCompareCategory( LangTypes::language_t language,
                          std::map<CategoryID, MC2String>& lookup )
         : m_locale( language ), m_lookup( lookup ) {}

   bool operator()( CategoryID lhs, CategoryID rhs ) const {
      return m_locale.collate( m_lookup[ lhs ], m_lookup[ rhs ] );
   }

private:
   LocaleUtility m_locale;
   std::map<CategoryID, MC2String>& m_lookup;
};

/**
 * Figure out which categories are the top level categories,
 * in other words, the ones that aren't children.
 */
vector<CategoryID> getTopLevelList( const Categories& categories,
                                    LangTypes::language_t language ) {
   // First create a lookup map used for sorting
   std::map<CategoryID, MC2String> lookup;
   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {
      lookup[ (*itr).getID() ] = (*itr).getName();
   }

   // The categories we still think are roots
   set<CategoryID> roots;

   // Assume all are roots at first
   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {
      roots.insert( (*itr).getID() );
   }

   // Go through all children and remove from roots
   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {

      const vector<CategoryID>& children = (*itr).getChildren();

      for ( vector<CategoryID>::const_iterator childItr = children.begin();
            childItr != children.end(); ++childItr ) {
         roots.erase( *childItr );
      }
   }

   vector<CategoryID> result( roots.begin(), roots.end() );

   sort( result.begin(), result.end(), 
         LocaleCompareCategory( language, lookup ) );

   return result;
}

/**
 * Calculates the size of a category in the binary format.
 */
int calcSizeOfCategory( const CategoryTree::StandaloneNode& category ) {
   int size = 0;

   size += sizeof( int32 );  // The category id
   size += sizeof( int32 );  // String index for name
   size += sizeof( int32 );  // String index for icon
   size += sizeof( uint16 ); // Number of sub categories
   // The children
   size_t nbrChildren = category.getChildren().size();
   size += sizeof( int32 ) * nbrChildren;

   return size;
}

/**
 * This function figures out where each category is in the category table,
 * and fills the lookup map with this information.
 */
void createLookupMap( map<CategoryID, int32>& lookupMap,
                      const Categories& categories,
                      const vector<CategoryID>& topLevelList ) {
   int currentOffset = 0;

   // First skip over the list of top level categories.
   // The top level list consists of a uin16 for # of categories, and then
   // one int32 for each top level category.
   currentOffset += sizeof( uint16 ) + topLevelList.size() * sizeof( int32 );

   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {
      lookupMap[ (*itr).getID() ] = currentOffset;
      currentOffset += calcSizeOfCategory( *itr );
   }
}

/**
 * Creates the lookup table.
 */
void writeLookupTable( const map<CategoryID, int32>& lookupMap,
                       DataBuffer* lookupTable ) {
   for ( map<CategoryID, int32>::const_iterator itr = lookupMap.begin();
         itr != lookupMap.end(); ++itr ) {

      lookupTable->writeNextBALong( (*itr).first );
      lookupTable->writeNextBALong( (*itr).second );

   }
}

/**
 * Calculates the size of the whole category table.
 */
int calcCategoryTableSize( const Categories& categories,
                           int nbrTopLevelCategories ) {
   // Start with the list of top level categories
   int sum = sizeof( uint16 ) + nbrTopLevelCategories * sizeof( int32 );

   // Then add all the categories
   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {
      sum += calcSizeOfCategory( *itr );
   }

   return sum;
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
 * Given a category id, this function returns its index in the category
 * table. Since the lookupMap should contain all categories, there's a bug
 * somewhere if the category isn't available, so we assert false.
 */
int32 getCategoryIndex( const map<CategoryID, int32>& lookupMap,
                        CategoryID category ) {
   map<CategoryID, int32>::const_iterator itr = lookupMap.find( category );
   
   if ( itr == lookupMap.end() ) {
      // Should never happen
      MC2_ASSERT( false );
      return 0;
   }
   else {
      return (*itr).second;
   }
}

/**
 * Creates the category table.
 */
void writeCategoryTable( const Categories& categories,
                         const vector<CategoryID>& topLevelList,
                         const map<CategoryID, int32>& lookupMap,
                         const map<MC2String, int32>& stringMap,
                         DataBuffer* categoryTable ) {
   // First write the list of top level categories
   
   categoryTable->writeNextBAShort( topLevelList.size() );
   
   for ( vector<CategoryID>::const_iterator itr = topLevelList.begin();
         itr != topLevelList.end(); ++itr ) {
      categoryTable->writeNextBALong( getCategoryIndex( lookupMap,
                                                        *itr ) );
   }

   // Then write the categories
   
   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {
      categoryTable->writeNextBALong( (*itr).getID() );
      categoryTable->writeNextBALong( getStringIndex( stringMap, 
                                                      (*itr).getName() ) );
      categoryTable->writeNextBALong( getStringIndex( stringMap,
                                                      (*itr).getIcon() ) );

      const vector<CategoryID>& children = (*itr).getChildren();
      categoryTable->writeNextBAShort( children.size() );

      for ( vector<CategoryID>::const_iterator itr = children.begin();
            itr != children.end(); ++itr ) {
         categoryTable->writeNextBALong( getCategoryIndex( lookupMap,
                                                           *itr ) );
      }
   }
}

/**
 * NOTE! Implemented for use in Unit tests
 *
 * Checks that everything in topLevelList is a root node and is actually
 * an id that is used in the tree.
 */
bool checkTopLevelList( const vector<CategoryID>& topLevelList,
                   const Categories& categories ) {
   
   set<CategoryID> roots( topLevelList.begin(),
                          topLevelList.end() );

   set<CategoryID> allCategories;

   for ( Categories::const_iterator itr = categories.begin();
         itr != categories.end(); ++itr ) {

      allCategories.insert( (*itr).getID() );

      const vector<CategoryID>& children = (*itr).getChildren();

      // make sure no child is considered a root
      for ( size_t i = 0; i < children.size(); ++i ) {
         if ( STLUtility::has( roots, children[ i ] ) ) {
            throw MC2Exception( "Child node is considered a root" );
         }
      }
   }

   // make sure all the roots are part of the full list of categories
   for ( set<CategoryID>::iterator itr = roots.begin();
         itr != roots.end(); ++itr ) {
      if ( !STLUtility::has( allCategories, *itr ) ) {
         throw MC2Exception( "Root is not part of the full list of categories" );
      }
   }

   return true;
}

}

namespace CategoryTreeUtils {

void serializeTree( const CategoryTree* tree,
                    LangTypes::language_t language,
                    BinaryCategoryTreeFormat* format) {

   // We should never serialize an empty tree, but rather send an error code.
   MC2_ASSERT( tree->getRootNodes().size() > 0 );

   Categories categories( tree->getStandaloneNodes( language ) );

   // Create string table

   // A map with all the strings and eventually their positions in the
   // string table.
   map<MC2String, int32> stringMap;

   getAllStrings( stringMap, categories );

   int stringTableSize = calcStringTableSize( stringMap );

   format->m_stringTable.reset( new DataBuffer( stringTableSize ) );

   writeStringTable( stringMap, format->m_stringTable.get() );

   vector<CategoryID> topLevelList = getTopLevelList( categories, language );

   // Create lookup table

   map<CategoryID, int32> lookupMap;
   createLookupMap( lookupMap, categories, topLevelList );

   int lookupTableSize = lookupMap.size() * 
      ( sizeof( int32 ) +  // Category ID
        sizeof( int32 ) ); // byte offset into category table

   format->m_lookupTable.reset( new DataBuffer( lookupTableSize ) );

   writeLookupTable( lookupMap, format->m_lookupTable.get() );

   // Create category table

   int categoryTableSize = calcCategoryTableSize( categories,
                                                  topLevelList.size() );

   format->m_categoryTable.reset( new DataBuffer( categoryTableSize ) );

   writeCategoryTable(categories, topLevelList, lookupMap, 
                      stringMap, format->m_categoryTable.get() );

   format->m_stringTable->reset();
   format->m_lookupTable->reset();
   format->m_categoryTable->reset();
}

MC2String getCrcForTree( const BinaryCategoryTreeFormat& catTreeFormat ) {
   uint32 catCrc = MC2CRC32::crc32( 
      catTreeFormat.m_categoryTable->getBufferAddress(), 
      catTreeFormat.m_categoryTable->getBufferSize() );

   uint32 lookupCrc = MC2CRC32::crc32( 
      catTreeFormat.m_lookupTable->getBufferAddress(), 
      catTreeFormat.m_lookupTable->getBufferSize() );

   uint32 stringCrc = MC2CRC32::crc32( 
      catTreeFormat.m_stringTable->getBufferAddress(), 
      catTreeFormat.m_stringTable->getBufferSize() );
                                    
   MC2String crcOut = STLStringUtility::uint2str( catCrc ) +
      STLStringUtility::uint2str( lookupCrc ) +
      STLStringUtility::uint2str( stringCrc );

   return crcOut;
}

/**
 * NOTE! This function is implemented for use in unit test and should be
 * reviewed before use from production code.
 *
 * Parses the binary format into a vector of categories.
 */
void parseBinaryFormat( BinaryCategoryTreeFormat* binaryFormat,
                        Categories& categories ) {
   size_t nbrCategories = binaryFormat->m_lookupTable->getBufferSize() / 8;
   if( binaryFormat->m_lookupTable->getBufferSize() % 8 != 0 ) {
      throw MC2Exception( "binaryFormat->m_lookupTable->getBufferSize() % 8 != 0" );
   }

   // read the top level list
   vector<CategoryID> topLevelList;
   topLevelList.resize( binaryFormat->m_categoryTable->readNextBAShort() );

   for ( size_t i = 0; i < topLevelList.size(); ++i ) {
      int32 categoryIndex = binaryFormat->m_categoryTable->readNextBALong();
      CategoryID catId = 
         binaryFormat->m_categoryTable->readLong( categoryIndex );
      topLevelList[ i ] = catId;
   }

   // read the categories
   for ( size_t i = 0; i < nbrCategories; ++i ) {
      CategoryID catId = binaryFormat->m_categoryTable->readNextBALong();
      int32 nameIndex = binaryFormat->m_categoryTable->readNextBALong();
      int32 iconIndex = binaryFormat->m_categoryTable->readNextBALong();

      char* name = reinterpret_cast<char*>(
         binaryFormat->m_stringTable->getBufferAddress()) + nameIndex;
      char* icon = reinterpret_cast<char*>(
         binaryFormat->m_stringTable->getBufferAddress()) + iconIndex;

      // read the children
      uint16 nbrChildren = binaryFormat->m_categoryTable->readNextBAShort();
      vector<CategoryID> children;
      for ( uint16 j = 0; j < nbrChildren; ++j ) {
         int32 childIndex = binaryFormat->m_categoryTable->readNextBALong();
         CategoryID childId =
            binaryFormat->m_categoryTable->readLong( childIndex );
         children.push_back( childId );
      }

      categories.push_back( 
         CategoryTree::StandaloneNode( catId, children, name, icon ) );
   }

   // make sure we've read everything there is
   if( binaryFormat->m_categoryTable->getCurrentOffset() !=
       binaryFormat->m_categoryTable->getBufferSize() ) {
      throw MC2Exception( "Category table parse did not reach end of buffer!" );
   }

   checkTopLevelList( topLevelList, categories );
}

}

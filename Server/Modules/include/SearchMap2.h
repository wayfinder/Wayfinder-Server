/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHMAP2_H
#define SEARCHMAP2_H

#include "config.h"

#include "IDPairVector.h"
#include "ItemTypes.h"
#include "NodeBits.h"
#include "LangTypes.h"
#include "StringTable.h"

#include <vector>
#include <set>
#include <algorithm>

class DataBuffer;
class IDTranslationTable;
class SearchMapItem;
class SearchMapPOIInfo;
class SearchMapRadiusInfo;
class SearchMapStringTable;
class SearchMapRegionTable;
class UserRightsItemTable;
class UserRightsMapInfo;
class MapRights;

// There are more includes after the classes.

class SearchMap2 {
public:
   /**
    *   Creates a new empty SearchMap.
    */
   SearchMap2();

   /**
    *   Deletes the items in the SearchMap.
    */
   virtual ~SearchMap2();

   /**
    *   Returns the size of the object in a DataBuffer.
    */   
   int getSizeInDataBuffer() const;

   /**
    *   Loads the object from a DataBuffer.
    */
   int load(DataBuffer& dataBuffer);

   /**
    *   Saves the object into a DataBuffer.
    */
   int save(DataBuffer& dataBuffer) const;

   /**
    *   Warning! This function cannot work if more than one
    *   MapModule-map is used in a SearchMap.
    */
   uint32 getMapID() const;
   
   /**
    *   Returns the number of names for the
    *   supplied comboIdx.
    */
   inline int getComboNbrNames(uint32 idx) const;

   /** 
    *   Should only be used by SearchMapItem.
    *   Returns the nameindex for name idx.
    */
   uint32 getComboNameIdx(int nameNbr, uint32 idx) const;

   /**
    *   Returns a pointer to the best name in the item item.
    *   @param item     The item to look for names in.
    *   @param reqLang  The requested language.
    *   @param oldNbr   The number of the name to replace. This is to
    *                   avoid replacing synonym names with other
    *                   synonymnames.
    *   @return The best name found or NULL if no names present.
    */
   inline pair<const char*, LangTypes::language_t>
      getBestName(const SearchMapItem* item,
                  LangTypes::language_t reqLang,
                  int oldNbr = -1) const;

   /**
    *
    */
   inline const char* getBestNameOfType( const SearchMapItem* item,
                                         LangTypes::language_t reqLang,
                                         ItemTypes::name_t type ) const;

   /**
    *   Returns a pointer to the best address in the poiinfo.
    *   @param poiInfo The poiinfo to use.
    *   @param reqLang The requested language.
    *   @return The best name found or NULL if no names present.
    */
   inline pair<const char*, LangTypes::language_t>
      getBestAddress(const SearchMapPOIInfo* poiInfo,
                     LangTypes::language_t reqLang) const;
   
   /**
    *   Returns the name from the StringTable.
    */
   inline const char* getName(uint32 nameIdx) const;

   /**
    *   Returns the index of the SearchMapItem in the map.
    */
   inline uint32 getIndex(const SearchMapItem* item) const;
   
   /**
    *   Iterator to use when iterating over all items.
    */
   typedef SearchMapItem* iterator;

   /**
    *   Const iterator to use when iterating over all items.
    */
   typedef const SearchMapItem* const_iterator;

   /**
    *   Returns iterator to first element in SearchMap2.
    */
   inline iterator begin() const;
   
   /**
    *   Returns iterator after last element in SearchMap2.
    */
   inline iterator end() const;

   /**
    *   Returns the item at position <code>idx</code>
    */
   inline const SearchMapItem* getItemByIndex(uint32 idx) const;
   
   /**
    *    Returns the item with the supplied id or maybe NULL.
    *    @param id ID to look for.
    *    @return Item found or NULL if not found.
    */
   const SearchMapItem* getItemByItemID(const IDPair_t& id) const;

   /**
    *    Returns the poi-info for the supplied item or NULL
    *    if there is none.
    *    @param item The item to use.
    *    @return Found item or NULL.
    */
   const SearchMapPOIInfo* getPOIInfoFor(const SearchMapItem* item) const;

   /**
    * Searches for poi infos with matching categories.
    * @param categories A categories set to search for.
    * @param poiinfos Will contain the poi infos found.
    * @return number of poi infos found
    */
   uint32 getPOIInfoForCategories( const set<uint16>& categories,
                                   vector<const SearchMapPOIInfo*>& 
                                   poiInfos ) const;
   /**
    *    Returns the radius-info for the supplied item or null.
    *    @param item The item to use.
    *    @return Found item or null.
    */
   const SearchMapRadiusInfo* getRadiusInfo(const SearchMapItem* item) const;
     
   /**
    *    Returns the itemtype of item number idx.
    */
   inline uint8 getItemType(uint32 idx) const;

   /**
    *    Translates the supplied overview items into low level and
    *    expands the items if there are more than one underview item
    *    associated with the overview item. (E.g. municipals that are
    *    in more than one map).
    *    @param expandedIDs vector containing index in in-vector in first
    *                       and expanded item in second. Can be more than
    *                       one result for one incoming item.
    *    @param idsToExpand The id:s to lookup.
    *    @return expandedIDs.size().
    */
   int expandIDs(vector<pair<uint32, IDPair_t> >& expandedIDs,
                 const vector<pair<uint32, IDPair_t> >&
                 idsToExpand,
                 bool expand) const;

   /**
    *    Returns the regionTable.
    */
   inline const SearchMapRegionTable* getRegionTable() const;

   /**
    *    Returns true if the item is inside the supplied region.
    *    @param item        Item to check.
    *    @param regionIndex Index of region.
    *    @return True if the item was inside the region or a region that
    *            was inside.
    */
   inline bool isInsideRegion(const SearchMapItem* item,
                              uint32 regionIndex) const;

   /**
    *    Returns true if the item should be found when searching.
    *    @param idx The index of the item.
    */
   bool itemShouldBeFound(uint32 idx) const;

   /**
    *   Translates a lower level map and itemid to higher level.
    *   @param lower The lower level map and itemid.
    *   @return The ID:s on higher level or MAX_UINT32, MAX_UINT32.
    */
   IDPair_t translateToHigher(const IDPair_t& lower) const;

   /**
    *   Translates a higher level map and itemid to lower level.
    *   @param lower The higher level map and itemid.
    *   @return The ID:s on lower level or MAX_UINT32, MAX_UINT32.
    */
   IDPair_t translateToLower(const IDPair_t& lower) const;

   /**
    *   Returns the country code
    */
   inline StringTable::countryCode getCountryCode() const;
   
   /**
    *    Get the native languages.
    */
   inline void getNativeLanguages(
                  set<LangTypes::language_t>& nativeLanguages) const;

   /**
    *    Returns true if the item is allowed by the rights.
    */
   bool itemAllowedByUserRights( const SearchMapItem* item,
                                 const UserRightsMapInfo& rights ) const;
   /**
    * Checks rights for an item
    * @param item Item to chech rights for.
    * @param right Rights to be checked for.
    * @return true if item has the rights, else false
    */
   bool hasRights( const SearchMapItem* item,
                   const MapRights& right) const;

   /**
    * Returns the rights to an item
    * @param item Item to get map rights for.
    * @return The rights of the item
    */
   MapRights getRights( const SearchMapItem* item ) const;

protected:
   void getAllRegionsFor( const SearchMapItem* item,
                          set<const SearchMapItem*>& regions ) const;


   friend class SearchUnitBuilder;
   
   /**
    *   Constructor used by the SearchUnitBuilder
    *   to set all data at once.
    */
   SearchMap2(uint32 nbrAllItems,
              SearchMapItem* allItems,
              uint8* itemTypes,
              uint32 nbrPOIInfos,
              SearchMapPOIInfo* poiInfos,
              SearchMapStringTable* stringTable,
              SearchMapRegionTable* regionTable,
              IDTranslationTable* idtranslationTable,
              uint32 nbrRadiusInfos,
              SearchMapRadiusInfo* radiusInfos,
              StringTable::countryCode countryCode,
              const set<LangTypes::language_t>& nativeLanguages,
              map<uint32, UserRightsItemTable*>& rights );
   
   /**
    *   Deletes all the data. To be used in destructor and load.
    */
   void deleteAllData();
   void createInsideMap();
   uint32 getInsideMapSizeInDataBuffer() const;

   /**
    *   Vector of SearchMapItems sorted by item-id.
    */
   SearchMapItem* m_allItems;

   /**
    *   Vector of item types that would make the
    *   size of the SearchMapItems odd.
    */
   uint8* m_allItemTypes;
   
   /**
    *   The number of items in m_allItems.
    */
   uint32 m_nbrItems;
   
   /**
    *   Vector of SearchMapPOIInfos sorted by item index
    *   in m_allItems.
    */
   SearchMapPOIInfo* m_POIInfos;

   /**
    *   The size of m_poiInfos.
    */
   uint32 m_nbrPOIInfos;

   /**
    *   The stringtable.
    */
   SearchMapStringTable* m_stringTable;

   /**
    *   The region table.
    */
   SearchMapRegionTable* m_regionTable;

   /**
    *   The id translation table.
    */
   IDTranslationTable* m_idTranslationTable;

   /**
    *   The number of items with radiusinfo.
    */
   uint32 m_nbrRadiusInfos;

   /**
    *   The radius infos.
    */
   SearchMapRadiusInfo* m_radiusInfos;
   
   /**
    *   The country code for the map.
    */
   StringTable::countryCode m_countryCode;

   /**
    *    The native languages for the map.
    */
   set<LangTypes::language_t> m_nativeLanguages;

   /// Type for the entries in the user rights tables.
   typedef pair<uint32, UserRightsItemTable*> rightTablesEntry_t;
   
   /// The right tables. Sorted on first which is the mapid
   rightTablesEntry_t* m_rightTables;
   
   /// Number of rights tables. Usually 1.
   int m_nbrRightTables;

private:
   typedef vector< pair<uint32, const SearchMapItem*> > insideMap_t;

   insideMap_t m_insideMap;
   /// maps category id to poi infos
   typedef vector< pair< uint16, uint32 > > poiInfoMap_t;
   poiInfoMap_t m_poiInfoMap;
};

#include "SearchMapStringTable.h"
#include "SearchMapItem.h"

inline int
SearchMap2::getComboNbrNames(uint32 idx) const
{
   return m_stringTable->getComboNbrNames(idx);
}

inline const char*
SearchMap2::getName(uint32 idx) const
{
   return m_stringTable->getName(idx);
}

inline uint32
SearchMap2::getComboNameIdx(int nameNbr, uint32 idx) const
{
   return m_stringTable->getComboNameIdx(nameNbr, idx);
}

inline uint8
SearchMap2::getItemType(uint32 idx) const
{
   return m_allItemTypes[idx];
}

inline
SearchMap2::iterator
SearchMap2::begin() const
{
   return m_allItems;
}

inline
SearchMap2::iterator
SearchMap2::end() const
{
   return &m_allItems[m_nbrItems];
}

inline uint32
SearchMap2::getIndex(const SearchMapItem* item) const
{
   // Pointer arithmetics.
   return item - m_allItems;
}

inline const SearchMapItem*
SearchMap2::getItemByIndex(uint32 idx) const
{
   return &m_allItems[ MapBits::nodeItemID( idx ) ];
}

inline const SearchMapRegionTable*
SearchMap2::getRegionTable() const
{
   return m_regionTable;
}

inline bool
SearchMap2::isInsideRegion(const SearchMapItem* item,
                           uint32 regionIndex) const
{
   for ( SearchMapRegionIterator it = item->getRegionBegin(this);
         it != item->getRegionEnd(this);
         ++it ) {
      if ( *it == regionIndex ) {
         return true;
      } else if ( isInsideRegion(getItemByIndex(*it), regionIndex ) ) {
         return true;
      }
   }
   return false;
}

inline pair<const char*, LangTypes::language_t>
SearchMap2::getBestName(const SearchMapItem* item,
                        LangTypes::language_t reqLang,
                        int oldNameNumber) const
{
   if ( oldNameNumber < 0 ) {
      // Return the best name
      return m_stringTable->getBestNameInCombo(reqLang,
                                               item->m_strIdx);
   } else {
      // Check if the new name was a synonym name
      uint32 idx = m_stringTable->getBestIdxInCombo(reqLang,
                                                    item->m_strIdx);
      if ( GET_STRING_TYPE(idx) == ItemTypes::synonymName ) {
         mc2dbg8 << "[SM2]: Best name is "
                 << ItemTypes::getNameTypeAsString(GET_STRING_TYPE(idx))
                 << endl;
         // Return the old name, since changing the language
         // will only confuse
         return pair<const char*, LangTypes::language_t>
            (item->getName(this, oldNameNumber), LangTypes::invalidLanguage);
      } else {
         // Return the new name.
         return pair<const char*, LangTypes::language_t>
            (m_stringTable->getName(GET_STRING_INDEX(idx)),
             GET_STRING_LANGUAGE(idx));
      }
   }
}

inline const char*
SearchMap2::getBestNameOfType( const SearchMapItem* item,
                               LangTypes::language_t reqLang,
                               ItemTypes::name_t type ) const
{
   uint32 strIdx = m_stringTable->getBestIdxOfTypeInCombo( reqLang,
                                                           type,
                                                           item->m_strIdx );
   
   if ( strIdx != MAX_UINT32 ) {
      if ( type == ItemTypes::synonymName ) {
         // Shortest synonym name is best
         LangTypes::language_t allowedLang = GET_STRING_LANGUAGE( strIdx );
         const int nbrNames = item->getNbrNames( this );
         
         uint32 bestStrIdx = strIdx;
         int bestLength = strlen(
            m_stringTable->getName( GET_STRING_INDEX( strIdx )));
         for( int i = 0; i < nbrNames; ++i ) {
            uint32 curStrInfo = item->getNameInfo(this, i);
            if ( GET_STRING_INDEX(curStrInfo) == GET_STRING_INDEX( strIdx ) ) {
               continue;
            }
            if ( GET_STRING_LANGUAGE( curStrInfo ) == allowedLang &&
                 GET_STRING_TYPE ( curStrInfo ) == type ) {
               const char* curName =
                  m_stringTable->getName( GET_STRING_INDEX( curStrInfo ) );
               int curLength = strlen(curName);
               if ( curLength < bestLength ) {
                  bestLength = curLength;
                  bestStrIdx = curStrInfo;
               }
            }
         }
         strIdx = bestStrIdx;
      }
                                               
      return m_stringTable->getName( GET_STRING_INDEX( strIdx ) );
   } else {
      return NULL;
   }
}


inline pair<const char*, LangTypes::language_t>
SearchMap2::getBestAddress(const SearchMapPOIInfo* poiInfo,
                           LangTypes::language_t reqLang) const
{
   return m_stringTable->getBestNameInCombo(reqLang,
                                            poiInfo->m_addressIdx);
}


inline StringTable::countryCode
SearchMap2::getCountryCode() const
{
   return m_countryCode;
}

inline void
SearchMap2::getNativeLanguages(
               set<LangTypes::language_t>& nativeLanguages) const
{
   nativeLanguages = m_nativeLanguages;
}

#endif

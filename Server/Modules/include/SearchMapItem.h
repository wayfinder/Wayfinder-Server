/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHMAP_ITEM_H
#define SEARCHMAP_ITEM_H

#include "config.h"
#include "MC2Coordinate.h"
#include "IDPairVector.h"
#include "ItemTypes.h"
#include "SearchMapRegionTable.h"
#include "StringUtility.h"

class SearchMap2;
class UserRightsMapInfo;

class SearchMapItem {
public:
   /**
    *   Empty constructor to be used when loading a map.
    */
   SearchMapItem() {}

   SearchMapItem(const IDPair_t& id,
                 const MC2Coordinate& coord,
                 uint32 strIdx,
                 uint32 regionIdx);

   /// Returns the itemid of the item.
   inline const IDPair_t& getID(const SearchMap2* theMap) const;

   /// Returns the number of names
   inline int getNbrNames(const SearchMap2* theMap) const;

   /// Returns the name number nameNbr.
   inline const char* getName(const SearchMap2* theMap,
                              int nameNbr) const;

   /**
    *   Returns one good coordinate for the SearchMapItem.
    *   @param theMap The map as usual.
    */
   inline const MC2Coordinate& getCoordinate(const SearchMap2* theMap) const;

   /**
    *  Returns the type of name.
    *  @param theMap  The map has some data.
    *  @param nameNbr The name number.
    *  @return The type of the name.
    */
   inline ItemTypes::name_t getNameType(const SearchMap2* theMap,
                                        int nameNbr) const;

   
   /**
    *   Returns the best name for the item.
    *   @param theMap     The map is needed of course.
    *   @param lang       The most wanted language.
    *   @param oldNameNbr The number of the name to replace.
    *   @return The best name.
    */
   inline pair<const char*, LangTypes::language_t>
      getBestName(const SearchMap2* theMap,
                  LangTypes::language_t lang,
                  int oldNameNbr = -1) const;

   /**
    *   Returns the best name of the given type. If no name
    *   of the type is found, NULL.
    *   @param theMap The map.
    *   @param lang   The preferred language.
    *   @param type   The name type.
    */
   inline const char* getBestNameOfType( const SearchMap2* theMap,
                                         LangTypes::language_t lang,
                                         ItemTypes::name_t type) const;
                                         

   /**
    *   Returns the best name number of the item.
    *   FIXME: Implement for real in map instead.
    *   @param theMap The map is needed of course.
    *   @param lang   The most wanted language.
    *   @return Index of best name.
    */
   inline int getBestNameNbr(const SearchMap2* theMap,
                             LangTypes::language_t lang) const;

   /**
    *   Returns the index in the stringtable of the name
    *   <code>nameNbr</code> in the item.
    *   @param theMap Some information in only known by the map.
    *   @param nameNbr The name number.
    */
   inline uint32 getNameIdx(const SearchMap2* theMap,
                            int nameNbr) const;

   /**
    *   Returns the complete name info about the item
    *   (containing lang, idx and type).
    *   @param theMap  The SearchMap2.
    *   @param nameNbr The name number.
    */
   inline uint32 getNameInfo(const SearchMap2* theMap,
                             int nameNbr) const;

   /**
    *    Returns true if the item is allowed to be found by the
    *    specified rights.
    */
   inline bool allowedByUserRights( const SearchMap2* theMap,
                                    const UserRightsMapInfo& rights ) const;

   /// Returns the index in the SearchMap of the item
   inline uint32 getIndex(const SearchMap2* theMap) const;

   /// Returns the number of regions of the item
   inline uint32 getNbrRegions(const SearchMap2* theMap) const;

   /// Returns an iterator to the first region
   inline SearchMapRegionIterator
          getRegionBegin(const SearchMap2* theMap) const;
   
   /// Returns an iterator past the last region.
   inline SearchMapRegionIterator getRegionEnd(const SearchMap2* theMap) const;

   /// Returns the type of the item
   inline ItemTypes::itemType getItemType(const SearchMap2* theMap) const;

   /// Returns the searchType of the item
   inline uint32 getSearchType(const SearchMap2* theMap) const;
   
   /// Returns true if the other item has the same names as this one
   inline bool sameNames(const SearchMap2* theMap,
                         const SearchMapItem* otherItem) const;

   /// Returns true if this SearchMapItem has a name in common with the other
   inline bool oneNameSame(const SearchMap2* theMap,
                           const SearchMapItem* other) const;

   /** Returns true if this SearchMapItem has a name in common with the 
    *  other. If names only differ by case, they are considered to be
    *  the same.
    */
   inline bool oneNameSameCase(const SearchMap2* theMap,
                               const SearchMapItem* other) const;

   /**
    *   Returns true if one name in this item is similar to one name
    *   in the other. Currently similar means substring in other.
    *   @param theMap The map is needed for names etc.
    *   @param other  The other item to compare to.
    *   @return True if one name is similar.
    */
   inline bool oneNameSimilar(const SearchMap2* theMap,
                              const SearchMapItem* other) const;

   /// Returns true if the item should be found and is not multimapspecial
   inline bool shouldBeFound(const SearchMap2* theMap) const;

   /// Returns the radius of the item or 0 if none exist
   inline uint32 getRadiusMeters(const SearchMap2* theMap) const;
     
private:
   friend class SearchMap2;
      
   /**
    *   The item id of the item.
    */
   IDPair_t m_id;

   /**
    *   One coordinate of the item.
    */
   MC2Coordinate m_coord;

   /**
    *   Index to the names in SearchMap2StringTable.
    */
   uint32 m_strIdx;

   /**
    *   Index in the regiontable for the regions of the item.
    */
   uint32 m_regionIdx;
   
};


// --- SearchMapPOIInfo

/**
 *   Contains extra information about the companies, 
 *   e.g. adresses.
 */
class SearchMapPOIInfo {
public:
   typedef uint16 CategoryID; ///< category id type
   typedef std::set<CategoryID> Categories; ///< container of categories

   /// Constructor - nothing is set!
   SearchMapPOIInfo() {}

   /**
    *   Constructor that sets variables.
    *
    */
   inline SearchMapPOIInfo(uint32 itemIdx,
                           uint32 addressIdx,
                           uint16 trueNbr,
                           uint16 subType,
                           const Categories& categories,
                           const MC2String& specialImage );

   inline uint32 getItemIdx(const SearchMap2* theMap) const {
      return m_itemIdx;
   }

   inline uint32 getAdressStringComboIdx(const SearchMap2* theMap) const {
      return m_addressIdx;
   }

   /**
    *   Returns the best adress according to the supplied language.
    */
   inline pair<const char*, LangTypes::language_t>
      getBestAddress(const SearchMap2* theMap,
                     LangTypes::language_t language) const;
   
   /**
    *   Returns the number of addresses for the poi.
    */
   inline int getNbrAddresses(const SearchMap2* theMap) const;
   
   inline uint32 getNumberOnStreet(const SearchMap2* theMap) const {
      return m_trueNumberOnStreet;
   }

   /**
    *   Returns the subtype of the match.
    */
   inline uint16 getItemSubType(const SearchMap2* theMap) const {
      return m_subType;
   }
   /// @return category IDs
   inline const Categories& getCategories() const {
      return m_categories;
   }

   const MC2String& getSpecialImage() const {
      return m_specialImage;
   }

private:
   friend class SearchMap2;
   /// Index to the real item in the SearchMap2
   uint32 m_itemIdx;

   /// Index to the names of the addresses
   uint32 m_addressIdx;

   /// Number on street
   uint16 m_trueNumberOnStreet;

   /// Subtype of the match
   uint16 m_subType;
   /// categories of this POI
   Categories m_categories;

   /// Special image
   MC2String m_specialImage;
};

/**
 *   Class for storing the radii of the new extended
 *   bua-items  that consist  of both a region and a
 *   radius. If this kind of item is used another item
 *   is considered inside if it has it as region or is
 *   inside the radius.
 */
class SearchMapRadiusInfo {
public:
   /**
    *   Sets the values of the radiusinfo.
    *   @param itemIdx Index of the item that has a radius.
    *   @param radius  The radius of the item.
    */
   inline void setAll(uint32 itemIdx,
                      uint32 radius);

   /**
    *   Returns the itemindex.
    */
   inline uint32 getItemIdx(const SearchMap2* theMap) const;

   /**
    *   Returns the radius of the radius info,
    */
   inline uint32 getRadiusMeters(const SearchMap2* theMap) const;
   
private:
   /// The itemIdx of the item that this is about
   uint32 m_itemIdx;
   /// The radius of the item.
   uint32 m_radius;
};

inline void
SearchMapRadiusInfo::setAll(uint32 itemIdx,
                            uint32 radius)
{
   m_itemIdx = itemIdx;
   m_radius  = radius;
}

inline uint32
SearchMapRadiusInfo::getItemIdx(const SearchMap2* theMap) const
{
   return m_itemIdx;
}

inline uint32
SearchMapRadiusInfo::getRadiusMeters (const SearchMap2* theMap) const
{
   return m_radius;
}

inline
const MC2Coordinate&
SearchMapItem::getCoordinate(const SearchMap2* theMap) const
{
   return m_coord;
}

#include "SearchMap2.h"

// -- Inlined functions.

inline
SearchMapItem::SearchMapItem(const IDPair_t& id,
                             const MC2Coordinate& coord,
                             uint32 strIdx,
                             uint32 regionIdx)
      : m_id(id), m_coord(coord), m_strIdx(strIdx), m_regionIdx(regionIdx)
{
}

inline const IDPair_t&
SearchMapItem::getID(const SearchMap2* theMap) const
{
   return m_id;
}

inline int
SearchMapItem::getNbrNames(const SearchMap2* theMap) const
{
   return theMap->getComboNbrNames(m_strIdx);
}

inline uint32
SearchMapItem::getNameInfo(const SearchMap2* theMap, int nameNbr) const
{
   return theMap->getComboNameIdx(nameNbr, m_strIdx);
}

inline bool
SearchMapItem::allowedByUserRights( const SearchMap2* theMap,
                                    const UserRightsMapInfo& rights ) const
{
   return theMap->itemAllowedByUserRights( this, rights );
}

inline uint32
SearchMapItem::getNameIdx(const SearchMap2* theMap, int nameNbr) const
{
   return GET_STRING_INDEX( getNameInfo(theMap, nameNbr));
}

inline const char*
SearchMapItem::getName(const SearchMap2* theMap, int nameNbr) const
{
   return theMap->getName( getNameIdx(theMap, nameNbr) );
}

inline ItemTypes::name_t
SearchMapItem::getNameType(const SearchMap2* theMap, int nameNbr) const
{
   return GET_STRING_TYPE( getNameInfo( theMap, nameNbr ) );
}

inline pair<const char*, LangTypes::language_t>
SearchMapItem::getBestName(const SearchMap2* theMap,
                           LangTypes::language_t lang,
                           int oldNameNbr) const
{
   return theMap->getBestName(this, lang, oldNameNbr);
}

inline const char*
SearchMapItem::getBestNameOfType( const SearchMap2* theMap,
                                  LangTypes::language_t lang,
                                  ItemTypes::name_t type) const
{
   return theMap->getBestNameOfType(this, lang, type);
}

inline int
SearchMapItem::getBestNameNbr(const SearchMap2* theMap,
                              LangTypes::language_t lang) const
{
   // FIXME: Implement functions in SearchMap2 and SearchMapStringTable
   //        to avoid going via the string.
   const char* bestName = getBestName(theMap, lang).first;
   const int nbrNames = getNbrNames(theMap);
   for(int i=0; i < nbrNames; ++i ) {
      if ( strcmp(getName(theMap, i), bestName) == 0 ) {
         return i;
      }
   }
   // Should be impossible.
   return 0;
}

inline uint32
SearchMapItem::getIndex(const SearchMap2* theMap) const
{
   return theMap->getIndex(this);
}

inline uint32
SearchMapItem::getNbrRegions(const SearchMap2* theMap) const
{
   return theMap->getRegionTable()->getNbrRegions(m_regionIdx);
}

inline SearchMapRegionIterator
SearchMapItem::getRegionBegin(const SearchMap2* theMap) const
{
   return theMap->getRegionTable()->begin(m_regionIdx);
}
   

inline SearchMapRegionIterator
SearchMapItem::getRegionEnd(const SearchMap2* theMap) const
{
   return theMap->getRegionTable()->end(m_regionIdx);
}

inline ItemTypes::itemType
SearchMapItem::getItemType(const SearchMap2* theMap) const
{
   return ItemTypes::itemType(theMap->getItemType(getIndex(theMap)));
}

inline uint32
SearchMapItem::getSearchType(const SearchMap2* theMap) const
{
   return ItemTypes::itemTypeToSearchType(getItemType(theMap));
}

inline bool
SearchMapItem::sameNames(const SearchMap2* theMap,
                         const SearchMapItem* otherItem) const
{
   // Since every combination of names should be in the table once
   // this comparison should do it.
   return m_strIdx == otherItem->m_strIdx;
}

inline bool
SearchMapItem::oneNameSame(const SearchMap2* theMap,
                           const SearchMapItem* other) const
{
   const int nbrNamesInThis = getNbrNames(theMap);
   const int nbrNamesInOther = other->getNbrNames(theMap);
   for( int i=0; i < nbrNamesInThis; ++i ) {
      const uint32 iNameIdx = getNameIdx(theMap, i); 
      for( int j=0; j < nbrNamesInOther; ++j ) {
         if ( iNameIdx == other->getNameIdx(theMap, j) ) {
            return true;
         }
      }
   }
   return false;
}

inline bool
SearchMapItem::oneNameSameCase(const SearchMap2* theMap,
                               const SearchMapItem* other) const
{
   const int nbrNamesInThis = getNbrNames(theMap);
   const int nbrNamesInOther = other->getNbrNames(theMap);
   for( int i=0; i < nbrNamesInThis; ++i ) {
      const char* nameI = getName(theMap, i);
      for( int j=0; j < nbrNamesInOther; ++j ) {
         const char* nameJ = other->getName(theMap, j);
         // Return true if one of the strings is a substring of the
         // other.
         if ( StringUtility::strcasecmp( nameI, nameJ ) == 0){
            return true;
         }
      }
   }
   return false;
}

inline bool
SearchMapItem::oneNameSimilar(const SearchMap2* theMap,
                              const SearchMapItem* other) const
{
   const int nbrNamesInThis = getNbrNames(theMap);
   const int nbrNamesInOther = other->getNbrNames(theMap);
   for( int i=0; i < nbrNamesInThis; ++i ) {
      const char* nameI = getName(theMap, i);
      for( int j=0; j < nbrNamesInOther; ++j ) {
         const char* nameJ = other->getName(theMap, j);
         // Return true if one of the strings is a substring of the
         // other.
         if ( strstr(nameI, nameJ) || strstr(nameJ, nameI) ) {
            return true;
         }
      }
   }
   return false;
}

inline bool
SearchMapItem::shouldBeFound(const SearchMap2* theMap) const
{
   return theMap->itemShouldBeFound(getIndex(theMap));
}

inline uint32
SearchMapItem::getRadiusMeters(const SearchMap2* theMap) const
{
   const SearchMapRadiusInfo* radiusInfo =
      theMap->getRadiusInfo(this);
   if ( radiusInfo == NULL ) {
      return 0;
   } else {
      return radiusInfo->getRadiusMeters(theMap);
   }
      
}

// -- SearchMapPOIInfo

SearchMapPOIInfo::SearchMapPOIInfo(uint32 itemIdx,
                                   uint32 addressIdx,
                                   uint16 trueNbr,
                                   uint16 subType,
                                   const Categories& categories,
                                   const MC2String& specialImage )
      : m_itemIdx(itemIdx), m_addressIdx(addressIdx),
        m_trueNumberOnStreet(trueNbr), m_subType(subType),
        m_categories( categories ),
        m_specialImage( specialImage )
{
}


inline pair<const char*, LangTypes::language_t>
SearchMapPOIInfo::getBestAddress(const SearchMap2* theMap,
                                 LangTypes::language_t language) const
{
   // FIXME: This is not the best address. Use NameUtility in stringtable
   return theMap->getBestAddress(this, language);
}


inline int
SearchMapPOIInfo::getNbrAddresses(const SearchMap2* theMap) const
{
   return theMap->getComboNbrNames(m_addressIdx);
}


#endif

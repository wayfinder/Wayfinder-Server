/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHUNINT_BUILDER_H
#define SEARCHUNINT_BUILDER_H

#include "config.h"

#include "MC2String.h"
#include "IDPairVector.h"
#include "GetAdditionalPOIInfo.h"

#include <map>
#include <set>

class GenericMap;
class Item;

class SearchUnit;
class SearchMap2;
class TemporarySearchMapItem;
class TemporarySearchMapPOIInfo;
class UserRightsItemTable;
class WriteableSearchMapStringTable;

/**
 *    Use this class for creating SearchUnits from one
 *    or more GenericMaps. Start by adding the GenericMaps
 *    using addMap and then use createSearchUnit() to create
 *    the search unit.
 */
class SearchUnitBuilder {
public:

   /**
    *   Constructor.
    */
   SearchUnitBuilder();
   
   /**
    *   Deletes the temporary data used.
    */
   ~SearchUnitBuilder();
   
   /**
    *   Adds the information from a GenericMap to the
    *   internal data structures.
    */
   bool addMap(const GenericMap& theMap);
   
   /**
    *   Builds the <code>SearchUnit</code> from the
    *   previously added <code>GenericMaps</code>.
    */
   SearchUnit* createSearchUnit();

private:

   friend class TemporarySearchMapPOIInfo;
   friend class TemporarySearchMapItem;

   /// Map of wasp-data per waspID.
   typedef GetAdditionalPOIInfo::poiResultMap_t waspMap_t;
   
   /**
    *   Gets WASP data for all pois in the map.
    */
   static inline uint32 getAllPOIInfo( waspMap_t& poiData,
                                       const GenericMap& theMap,
                                       GetAdditionalPOIInfo& db );
   
   /**
    *   Deletes the temporary items created by the first step.
    */
   void deleteTempItems();

   /**
    *   Adds an item to the SearchMap.
    */
   inline bool addItem(const Item* item,
                       const GenericMap& theMap,
                       set<IDPair_t>& allRegions,
                       const waspMap_t& allWasp );
                       
   /**
    *   Returns true if the item should be added.
    *   @param item   The item.
    *   @param theMap Current map.
    *   @return True if the item should be added.
    */
   inline bool itemShouldBeAdded(const Item* item,
                                 const GenericMap& theMap) const;
   
   /**
    *   Adds a poiinfo to the map.
    */
   inline bool addPOIInfo(const Item* item,
                          const GenericMap& theMap,
                          const waspMap_t& allPoiData);

   /**
    *   Returns a temporary string index to use until
    *   the real table is built.
    */
   uint32 getTemporaryStringIndex(const char* aString);

   /// Typedef for convenience
   typedef map<IDPair_t, TemporarySearchMapItem*> itemMap_t;
   
   /// Collection of temporary items
   itemMap_t m_tempItems;

   /// Typedef of the map containing poiinfos
   typedef map<IDPair_t, TemporarySearchMapPOIInfo*> poiMap_t;
   
   /// Collection of temporary point of interest items
   poiMap_t m_tempPOIs;

   /// Typedef for map containing used strings.
   typedef map<MC2String, uint32> stringMap_t;
   
   /** 
    *   Collection of strings to save some space before the
    *   String in first and temp idx in second. The strings
    *   are stored in the genericmap really.
    */
   stringMap_t m_strings;

   // The country code for the map
   StringTable::countryCode m_countryCode;
   
   // The native languages for the map
   set<LangTypes::language_t> m_nativeLanguages;
   
   /// Map containing overview id:s in first and underview in second
   map<IDPair_t,IDPair_t> m_ovToUndTable;
   
   // -- Second step

   /**
    *   Builds the SearchMap. After this no items can be added.
    */
   void buildSearchMap();
   

   /// StringTable - this is where the strings are put.
   WriteableSearchMapStringTable* m_stringTable;

   /// The SearchMap2.
   SearchMap2* m_searchMap;

   /// GetAdditionalPOIInfo, used to get streetnumber etc for POI:s.
   GetAdditionalPOIInfo m_getPOIInfo;

   /// Type of the map to put the user rights into
   typedef map<uint32, UserRightsItemTable*> userRightsPerMap_t;
   
   /// Map of UserRights for items per map
   userRightsPerMap_t m_userRightsPerMap;
};

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REGION_FAMILY_TYPES_H
#define REGION_FAMILY_TYPES_H

#include "config.h"

#include <map>

class Item;

/**
 *   Class that keeps track of the region families, i.e.
 *   different hierarchies where an item can belong. 
 *   Items from one family should not be able to exist in a
 *   group of another family type. E.g. zip codes cannot exist in
 *   a city part.
 */
class RegionFamilies {
public:
   enum families {
      /// City parts, built up areas, municipals etc
      adminRegions  = 0,
      /// Postal codes and postal areas.
      postalRegions = 1,
      /// Categories.
      categories    = 2,
      /// The number of families.
      endFamily     = 3
   };

   /**
    *   The number of RegionFamlies.
    */
   static const int nbrFamilies = endFamily;

   
   /**
    *   Returns true if the item belongs to the family.
    *   @param familyNumber The number of the family.
    *   @param item         The item to check.
    *   @return True if the item belongs to the family.
    */
   static bool itemBelongsToFamily( int familyNumber,
                                    const Item* item );

   /**
    *   Returns true if the search type belongs to the family.
    *   @param familyNumber The family number to check.
    *   @param searchType   The search type to examine.
    */
   inline static bool searchTypeBelongsToFamily( int familyNumber,
                                                 uint32 searchType );
 
protected:
   
   /**
    *   Returns a search mask for the items in familyNumber.
    *   @param familyNumber Number of family to return mask for.
    *   @return The search types of the items that belong to the family.
    */
   inline static uint32 getSearchTypesForFamily( int familyNumber );

   /**
    *   Class to store the relationship between the
    *   region families and the SearchTypes in.
    */
   class familyToSearchTypeMap_t : public map<int, uint32> {
     public:
      /// Constructor that initializes the map
      familyToSearchTypeMap_t();
   };
      
   /// The map of searchtypes.
   static const familyToSearchTypeMap_t c_familySearchtypes;
   
};

uint32
RegionFamilies::getSearchTypesForFamily( int familyNumber )
{
   familyToSearchTypeMap_t::const_iterator it =
      c_familySearchtypes.find( familyNumber );
   if ( it != c_familySearchtypes.end() ) {
      return it->second;
   } else {
      // No types.
      return 0;
   }
}

bool
RegionFamilies::searchTypeBelongsToFamily( int familyNumber,
                                           uint32 searchType )
{
   return getSearchTypesForFamily( familyNumber ) & searchType;
}

#endif


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

#include "RegionFamilies.h"

#include "SearchTypes.h"
#include "ItemTypes.h"
#include "Item.h"

const RegionFamilies::familyToSearchTypeMap_t
RegionFamilies::c_familySearchtypes;

RegionFamilies::familyToSearchTypeMap_t::familyToSearchTypeMap_t()
{
   // Admin types, buas etc.
   insert( make_pair( adminRegions, SEARCH_MUNICIPALS |
                      SEARCH_BUILT_UP_AREAS |
                      SEARCH_CITY_PARTS ) );
   // Postal types
   insert( make_pair( postalRegions, SEARCH_ZIP_CODES | SEARCH_ZIP_AREAS ) );
   // Categories
   insert( make_pair( categories, SEARCH_CATEGORIES  ) );
}

bool
RegionFamilies::itemBelongsToFamily( int familyNumber,
                                     const Item* item )
{
   ItemTypes::itemType itemType = item->getItemType();
   uint32 searchType = ItemTypes::itemTypeToSearchType( itemType );
   return searchTypeBelongsToFamily( familyNumber, searchType );
}

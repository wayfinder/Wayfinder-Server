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
#include "StringUtility.h"

#include "ServerTileCategories.h"

namespace {

   typedef pair<ServerTileCategories::category_t,
                                      StringTable::stringCode> catPair_t;

   #define Y(x,y) catPair_t(ServerTileCategories::x, StringTable::y)
   
   /// Table of category -> stringTable index
   static const catPair_t catTrans [] = {
      Y ( AIRPORT,           AIRPORT             ),
      Y ( POST_OFFICE,       POST_OFFICE         ),
      Y ( NIGHTLIFE,         NIGHTLIFE           ),
      Y ( PETROL_STATION,    PETROL_STATION      ),
      Y ( PARKING_GARAGE,    PARKING_GARAGE      ),
      Y ( RAILWAY_STATION,   RAILWAY_STATION     ),
      Y ( HOTEL,             HOTEL               ),
      Y ( RENT_A_CAR,        RENT_A_CAR_FACILITY ),
      Y ( MUSEUM,            MUSEUM              ),
      Y ( OPEN_PARKING_AREA, OPEN_PARKING_AREA   ),
      Y ( TOURIST_OFFICE,    TOURIST_OFFICE      ),
      Y ( RESTAURANT,        RESTAURANT          ),
      Y ( FERRY_TERMINAL,    FERRY_TERMINAL      ),
      Y ( ENTERTAINMENT,     ENTERTAINMENT       ),
      Y ( LEISURE,           LEISURE             ),
      Y ( PARKING,           PARKING             ),
      Y ( TOURISM,           TOURISM             ),
      Y ( TRAVEL_TERMINAL,   TRAVEL_TERMINAL     ),
      Y ( WIFI_SPOT,         WIFI_SPOT           ),
      Y ( DISPLAY_OTHERS,    OTHERS              ),
      Y ( OTHER,             NOSTRING            ),
   };

   #undef Y
   
}

const MC2String
ServerTileCategoryEntry::getName( LangTypes::language_t lang ) const 
{
   return StringUtility::makeFirstCapital( 
      StringTable::getString( m_stringCode, lang ) );
}

ServerTileCategories::ServerTileCategories()
{
   CHECK_ARRAY_SIZE( catTrans, NBR_CATEGORIES );

   // Transfer categories from array to map   
   m_categories.insert( catTrans,
                        catTrans + sizeof(catTrans)/sizeof(catTrans[0]) );

   // Update the ids
   for( categoryMap_t::iterator it = m_categories.begin();
        it != m_categories.end();
        ++it ) {
      it->second.setID( it->first );
   }
   
   MC2_ASSERT( m_categories.size() == NBR_CATEGORIES );
}

const ServerTileCategoryEntry&
ServerTileCategories::getCategoryEntry( category_t type)
{
   return m_categories[type];
}

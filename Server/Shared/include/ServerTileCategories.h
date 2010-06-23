/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVERTILECATEGORYNOTICE_H
#define SERVERTILECATEGORYNOTICE_H

#include "config.h"
#include "MC2SimpleString.h"
#include "LangTypes.h"
#include "StringTable.h"
#include <map>

/**
 *    
 */
class  ServerTileCategoryEntry {
public:

   /// Creates an empty ServerTileCategoryEntry
   ServerTileCategoryEntry() : m_key(-1) {}
         
   /// Creates a ServerTileCategoryNotice
   ServerTileCategoryEntry( StringTable::stringCode stringCode,
	 		    int key = -1 ) :
      m_stringCode( stringCode ),
      m_key( key ) {
   }   
   
   /// Returns the name of the category in the specified language.
   const MC2String getName( LangTypes::language_t lang ) const;
   
   void setID( int key ) {
      m_key = key;
   }

   int getID() const {
      return m_key;
   }

   bool enabledByDefault() const {
      return true;
   }
   
   StringTable::stringCode m_stringCode;
   int m_key;
};

class ServerTileCategories {
public:

   /// Creates the notices.
   ServerTileCategories();
   
   /**
    *  Enum of the different categories. The order of these should
    *  be kept to keep the settings of the client from being reset
    *  when new ones are sent. Old, unused categories can be removed.
    */
   enum category_t {
      AIRPORT             =  0,
      POST_OFFICE         =  1,
      NIGHTLIFE           =  2,
      PETROL_STATION      =  3,
      PARKING_GARAGE      =  4,
      RAILWAY_STATION     =  5,
      HOTEL               =  6,
      RENT_A_CAR          =  7,
      MUSEUM              =  8,
      OPEN_PARKING_AREA   =  9,
      TOURIST_OFFICE      = 10,
      RESTAURANT          = 11,
      FERRY_TERMINAL      = 12,
      /// New!
      ENTERTAINMENT       = 13,
      LEISURE             = 14,
      PARKING             = 15,
      TOURISM             = 16,
      TRAVEL_TERMINAL     = 17,
      WIFI_SPOT           = 18,
      /// The others category to show
      DISPLAY_OTHERS      = 19,
      /// NB! Other must always be last, since it should not be displayed
      OTHER,               
      NBR_CATEGORIES = OTHER + 1,
   };

   /// Returns a category notice for the supplied type.
   const ServerTileCategoryEntry& getCategoryEntry( category_t type );

   typedef map<category_t, ServerTileCategoryEntry> categoryMap_t;
   
   categoryMap_t m_categories;
   
};
      
#endif

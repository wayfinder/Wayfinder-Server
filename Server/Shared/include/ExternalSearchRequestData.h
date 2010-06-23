/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNAL_SEARCH_REQUEST_DATA_H
#define EXTERNAL_SEARCH_REQUEST_DATA_H

#include "config.h"

#include "MC2String.h"
#include "MC2Coordinate.h"
#include "ItemInfoEnums.h"
#include <map>

class SearchRequestParameters;
class ExternalSearchDesc;
class Packet;
class LangType;

class ExternalSearchRequestData {
public:
   typedef int32 Distance;
   static const int32 INVALID_DISTANCE = MAX_INT32;

   /// Map.
   typedef map<int, MC2String> stringMap_t;

   /// Creates empty
   ExternalSearchRequestData();

   /**
    *   Creates a new ExternalSearchRequestData.
    *   @param params  Search settings.
    *   @param service Service to use.
    *   @param values  Search input values as per ExternalSearchDesc.
    *   @param startHitIdx Start index of hits to send back.
    *   @param nbrHits     (Max) number of hits to return.
    *   @param coord The input coordinates.
    *   @param itemInfoFilter Filter for item info
    */
   ExternalSearchRequestData( const SearchRequestParameters& params,
                              uint32 service,
                              const stringMap_t& values,
                              int startHitIdx,
                              int nbrHits,
                              ItemInfoEnums::InfoTypeFilter itemInfoFilter = 
                                 ItemInfoEnums::All,
                              const MC2Coordinate& coord =
                              MC2Coordinate::invalidCoordinate,
                              Distance distance = INVALID_DISTANCE
                              );

   /**
    *   Copy-constructor.
    */
   ExternalSearchRequestData( const ExternalSearchRequestData& other );

   /**
    *   Assignment-operator.
    */
   ExternalSearchRequestData&
          operator=( const ExternalSearchRequestData& other);

   /**
    *   Destructor.
    */
   ~ExternalSearchRequestData();

   /**
    *   Returns the search request params.
    */
   const SearchRequestParameters& getSearchParams() const;
   
   inline const MC2Coordinate& getCoordinate() const;

   inline Distance getDistance() const;
   /**
    *   Returns the values.
    */
   const stringMap_t& getValues() const;

   /**
    *   Returns the value for key and empty string if not there.
    */
   const MC2String& getVal( int key ) const;

   /**
    *   Returns the service.
    */
   uint32 getService() const;

   /**
    *   Returns the language.
    */
   LangType getLang() const;

   /**
    *   Saves the request data to a packet.
    */
   int save( Packet* packet, int& pos ) const;

   /**
    *   Loads the data from a packet.
    */
   int load( const Packet* packet, int& pos );

   /**
    *   Returns the size of the data in a packet.
    */
   int getSizeInPacket() const;

   /**
    *   Returns the start hit index.
    */
   int getStartHitIdx() const {
      return m_startHitIdx;
   }
   
   /**
    *   Returns the end hit index
    */
   int getEndHitIdx() const {
      return m_endHitIdx;
   }

   /**
    *   Returns the number of wanted hits
    */
   int getNbrWantedHits() const {
      return m_endHitIdx - m_startHitIdx;
   }

   /**
    * Returns the item info filter level
    */
   ItemInfoEnums::InfoTypeFilter getInfoFilterLevel() const {
      return m_itemInfoFilter;
   }
   

private:
   /// Service
   uint32 m_service;
   /// SearchParameters
   SearchRequestParameters* m_searchParams;
   /// Input values
   stringMap_t m_values;
   /// Input coordinates
   MC2Coordinate m_coord;
   /// Distance from coordinate
   Distance m_distance;

   /// Start hit index
   int m_startHitIdx;
   /// End hit index
   int m_endHitIdx;

   /// ItemInfo filter
   ItemInfoEnums::InfoTypeFilter m_itemInfoFilter;
};

inline
const MC2Coordinate& ExternalSearchRequestData::getCoordinate() const {
   return m_coord;
}

inline ExternalSearchRequestData::Distance
ExternalSearchRequestData::getDistance() const {
   return m_distance;
}

#endif

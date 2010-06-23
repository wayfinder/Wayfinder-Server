/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REVERSE_GEOCODING_H
#define REVERSE_GEOCODING_H

#include "config.h"
#include "MC2String.h"
#include "MC2Coordinate.h"
#include "StringTable.h"
#include <set>

class MapRequester;

/// Functionality for doing reverse geocoding.
namespace ReverseGeocoding {

/// The status of a reverse geocoding.
enum LookupStatus {
   LOOKUP_OK,
   LOOKUP_FAILED,
   OUTSIDE_ALLOWED_AREA,
   OUTSIDE_MAP_COVERAGE,
   TIMEOUT_ERROR
};

/// The result of a reverse geocoding
struct LookupResult {
   LookupResult():
      m_status( LOOKUP_FAILED ),
      m_country(),
      m_municipal(),
      m_city(),
      m_district(),
      m_street(),
      m_mapID( MAX_UINT32 ),
      m_itemID( MAX_UINT32 ),
      m_topRegionID( MAX_UINT32 ),
      m_streetSegmentOffset( 0 )
   { }

   LookupStatus m_status;
   MC2String m_errorString;

   MC2String m_country;
   MC2String m_municipal;
   MC2String m_city;
   MC2String m_district;
   MC2String m_street;

   uint32 m_mapID;
   uint32 m_itemID;

   uint32 m_topRegionID;

   /// The offset on the closest street segment item
   uint16 m_streetSegmentOffset;
};

/**
 * Performs reverse geocoding with help of the map module.
 *
 * @param position The position to lookup.
 * @param angle The angle.
 * @param language Preferred language for the results.
 * @param allowedMaps Use NULL to allow all maps.
 * @param requester The requester to use to communicate with the map module.
 */
LookupResult lookup( const MC2Coordinate& position,
                     uint16 angle,
                     StringTable::languageCode language,
                     const set< uint32 >* allowedMaps,
                     MapRequester* requester );

}

#endif // REVERSE_GEOCODING_H

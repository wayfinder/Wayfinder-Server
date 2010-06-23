/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPBITS_H
#define MAPBITS_H

#include "config.h"

//
// Maximum numbers of (graphical)-zoomlevels allowd in the
// map. log(Number_GFX_ZOOMLEVELS) bits are used in the 32b
// localID that identifies the items on the map.
// -------------------------------------------------------------
#define NUMBER_GFX_ZOOMLEVELS 16
// -------------------------------------------------------------



//
// ID of the first overviewmap. The overview maps will be
// handled in a proper way later (every map must know its
// parent and childs)
// -------------------------------------------------------------
#define FIRST_OVERVIEWMAP_ID  0x80000000
// ------------------------------------------------------------



//
// ID of the first country map. The country maps will be
// handled in a proper way later (every map must know its
// parent and childs)
// -------------------------------------------------------------
#define FIRST_COUNTRYMAP_ID  0x80000001
// ------------------------------------------------------------


//
// ID of the first super overivew map. The country maps will be
// handled in a proper way later (every map must know its
// parent and childs)
// -------------------------------------------------------------
#define FIRST_SUPEROVERVIEWMAP_ID 0x90000000
// ------------------------------------------------------------


//
// Name of map index file.
// Used for storing the information about the maps
// needed by the MapModule leader.
// -------------------------------------------------------------
#define INDEX_NAME "index.db"
// ------------------------------------------------------------

/**
 * The type of a map id, identifies one of the maps.
 */
typedef uint32 MapID;

/**
 * @namespace Functions for identifying map ids
 */
namespace MapBits {

/**
 *   Find out if a map ID is an overview map or not.
 *   Note that a countrymap is not considered an overview map.
 *   @return  True if the mapID is an overview map,
 *            false otherwise.
 */
inline bool isOverviewMap( MapID mapID ) {
   return ((mapID >= FIRST_OVERVIEWMAP_ID) && ((mapID & 1) == 0));
}

/**
 *   Find out if a map ID is a country map or not.
 *   @return  True if the mapID is an overview map,
 *            false otherwise.
 */
inline bool isCountryMap( MapID mapID ) {
   return ((mapID >= FIRST_OVERVIEWMAP_ID) && ((mapID & 1) == 1));
}

/**
 *   Find out if a map ID is an underview map or not.
 *   @return  True if the mapID is an underview map,
 *            false otherwise.
 */
inline bool isUnderviewMap( MapID mapID ) {
   return (mapID < FIRST_OVERVIEWMAP_ID);
}

/**
 *   Find the corresponding country map to an overview map.
 *   If an underview map is supplied MAX_UINT32 is returned.
 *   If an country map is supplied then the country map is
 *   returned.
 *   @return  The corresponding country map to an overview map.
 */
inline MapID overviewToCountry( MapID mapID )  {
   if ( isUnderviewMap( mapID ) ) {
      return (MAX_UINT32);
   } else {
      return ((mapID & 0xfffffffe) + 1);
   }
}

/**
 *   Find the corresponding overview map to an country map.
 *   If an underview map is supplied MAX_UINT32 is returned.
 *   If an overview map is supplied then the overview map is
 *   returned.
 *   @return  The corresponding country map to an overview map.
 */
inline MapID countryToOverview( MapID mapID )  {
   if ( isUnderviewMap( mapID ) ) {
      return (MAX_UINT32);
   } else {
      return (mapID & 0xfffffffe);
   }
}

/**
 *    Returns the map level for the supplied mapID.
 *    @param mapID The id to get the level for.
 *    @return The level of the map id.
 */
inline uint32 getMapLevel( MapID mapID ) {
   if ( isUnderviewMap( mapID ) ) {
      return 0;
   } else {
      return (mapID >> 28) - 7;
   }
}

/**
 *    Get next valid map id of the same type.
 *    @param   mapID The current map id.
 *    @return The next map id.
 */
inline MapID nextMapID( MapID mapID ) {
   if ( isUnderviewMap( mapID ) ) {
      return (mapID + 1);
   } else {
      return (mapID + 2);
   }
}

///  Return mapID with encoded map set ID
inline MapID getMapIDWithMapSet(MapID mapID, uint32 mapSet) {
   return mapID | ((mapSet & 0x07) << 24);
}

///  Return the map set ID using a given map ID
inline uint32 getMapSetFromMapID( MapID mapID ) {
   return ( (mapID >> 24) & 0x07);
}

///  Return map ID stripped from any map set information
inline MapID getMapIDWithoutMapSet( MapID mapID ) {
   return mapID & 0xF8FFFFFF;
}

} // MapBits

#endif // MAPBITS_H

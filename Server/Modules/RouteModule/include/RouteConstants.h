/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTE_CONSTANTS_H
#define ROUTE_CONSTANTS_H

/*
 * Macros used in the RouteModule.
 */
#include "config.h"
#include "OverviewMap.h"
#include "Connection.h"
#include "ItemTypes.h"

// Define this if all connections should be allocated at once
// when loading map. It also means that one has to keep track
// of which connections, that are created when loading the map
// and which connections that are created when e.g. adding a
// disturbance.
#define PREALLOC_CONNECTIONS


#define GET_ZOOM_LEVEL(a) (((a) & 0x7fffffff) >> 27)

#define IS_NODE0(a) ((uint32(a)&0x80000000)==0)

#define SUBROUTE_LOWLEVEL 0x30000000
#define LEVEL_MASK        0x78000000

// Old version of lowerlevel
//#define IS_LOWER_LEVEL(a) ( ( (a) & LEVEL_MASK ) == SUBROUTE_LOWLEVEL)

// Level is now defined in OverviewMap.h
// This macro could be optimized. It's not very optimized by -O3
//   andl $2147483647,%eax
//   shrl $27,%eax
//   cmpl $5,%eax
// The solution would be to preshift the zoomlevel.
#define IS_LOWER_LEVEL(a) ( GET_ZOOM_LEVEL(a) > \
                         (uint32)OverviewMap::maxZoomLevelStreetSegmentItems )

#define IS_UPPER_LEVEL(a) (!IS_LOWER_LEVEL(a))
#if 1
// REAL
#define HAS_NO_RESTRICTIONS(a) ( (a) == ItemTypes::noRestrictions )
#define HAS_NO_THROUGHFARE(a) ( (a) == ItemTypes::noThroughfare )
// END REAL

#else
// BEGIN TESTING
#define HAS_NO_RESTRICTIONS(a) ( ((a) == ItemTypes::noRestrictions) || \
                                 ((a) == ItemTypes::noThroughfare) ) 
// This will not work if the order changes...
// #define HAS_NO_RESTRICTIONS(a) ( (a) < ItemTypes::noEntry )
#define HAS_NO_THROUGHFARE(a) ( false )
#endif
// END TESTING

#define HAS_NO_ENTRY(a) ( (a) == ItemTypes::noEntry )
#define HAS_NO_WAY(a) ( (a) == ItemTypes::noWay )
#define NO_PASSING(a) ( HAS_NO_RESTRICTIONS(a) || HAS_NO_THROUGHFARE(a) )
#define NOT_VALID(a) ( HAS_NO_ENTRY(a) || HAS_NO_WAY(a) )

#define IS_WALKING(a) (( (a) & ItemTypes::pedestrian) != 0)
#define IS_DRIVING_BUS(a) (( (a) & ItemTypes::publicTransportation) != 0)
#define IS_DRIVING(a) (!IS_WALKING(a) && !IS_DRIVING_BUS(a))

#define IS_STATE_ELEMENT(a) ( ( (a) & 0xf0000000 ) == 0xf0000000 )

#define SEC_TO_TIME_COST(a) ( Connection::secToTimeCost(a) )
#define TIME_COST_TO_SEC(a) ( Connection::timeCostToSec(a) )
/**
 * This class contains all the various constants that are used in the
 * RouteModule.
 *
 */
class RouteConstants
{
   public:

      /**
       * The special itemID that indicates the start of a drive path.
       */
      static const uint32 DRIVE_ITEM_ID = 0xf0000001;

      /**
       * The special itemID that indicates the start of a walk path.
       */
      static const uint32 WALK_ITEM_ID = 0xf0000002;

      /**
       *  The special itemID that indicates the start of a bicycle path.
       */
      static const uint32 BIKE_ITEM_ID = 0xf0000003;

      /**
       * The default turnCost. The turnCost in the packet will be
       * multiplied with this constant to determine the final turnCost
       * for a node. This represents ~30 seconds to turn a vehicle.
       */
      static const uint32 TURN_COST_SEC = 120;

      /**
       * The default value of the minimum time to wait at a bus stop.
       * The value is in seconds.
       */
      static const uint32 MIN_WAIT_TIME = 120;

      /**
       * The factor to multiply the cost with for items with zoom level 0.
       */
      static const int TURN_COST_FACTOR_LEVEL_0 = 20;

      /**
       * The factor to multiply the cost with for items with zoom level 1.
       */
      static const int TURN_COST_FACTOR_LEVEL_1 = 12;

      /**
       * The factor to multiply the cost with for items with zoom level 2.
       */
      static const int TURN_COST_FACTOR_LEVEL_2 = 6;

      /**
       * The factor to multiply the cost with for items with zoom level 3.
       */
      static const int TURN_COST_FACTOR_LEVEL_3 = 1;

      /**
       * The factor to multiply the cost with for items with strange zoom
       * level.
       */
      static const int TURN_COST_FACTOR_DEFAULT = 1;

      /**
       * The maximum distance that a walk route can have.
       */
      static const uint32 MAXIMUM_LOWER_LEVEL_ROUTE_DISTANCE = 100000;
      //static const uint32 MAXIMUM_LOWER_LEVEL_ROUTE_DISTANCE = MAX_UINT32;

      /** 
       * The factor between driving a longer distance or walking.
       */
      static const int WALK_FACTOR = 5;

      /**
       * Penalty for moving the car when starting in a street not allowed
       * to.
       */
      static const int NO_WAY_PENALTY = 5;

      /**
       * Penalty for exit a bus.
       */
      static const int EXIT_BUS_PENALTY = 15;

      /**
       *   Walking speed in meters per second.
       */
      static const float WALKING_SPEED_M_S = (5.0 / 3.6);

      /**
       *   Inverse walking speed in seconds per meter.
       */
      static const float INV_WALKING_SPEED_M_S = 1/(5.0 / 3.6);

// ========================================================================
//                                                              Functions =

      /**
       * Check if the given itemID is one of the state elements
       * DRIVE\_ITEM\_ID or WALK\_ITEM\_ID.
       *
       * @param  itemID The ID to check if it is a state ID.
       * @return        True if itemID is a state element, otherwise false.
       */
      static inline bool isStateElement(uint32 itemID);

}; // RouteConstants

// ========================================================================
//                                     Implementation of inline functions =

inline bool
RouteConstants::isStateElement(uint32 itemID)
{
   return GET_TRANSPORTATION_STATE(itemID);
}

#endif //ROUTE_CONSTANTS_H




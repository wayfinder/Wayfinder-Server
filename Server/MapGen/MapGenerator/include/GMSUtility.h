/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSUTILITY_H
#define GMSUTILITY_H

#include "GMSMap.h"

/**
  *   Class containing static methods used during map generation.
  *   
  */
class GMSUtility {

   public:

      /**
        *   Adds external connections to a map.
        *   @param   theMap   The map to add external connections to.
        *   @param   data  Holds info about the external nodes
        *                  from which connections are created.
        *   @return  How many external conenctions that were added to 
        *            the boundry segments in this map. 
        */
      static int addExternalConnection( OldGenericMap* theMap, 
                                        GMSMap::extdata_t data );

      
      /**
        *   Get distance from one point to a given node.
        *   @param   nodeNbr  0 for node zero, otherwise node 1.
        *   @param   gfx      Pointer to the GfxData where nodeNbr
        *                     is located.
        *   @param   lat      Latitude part of the coordinate.
        *   @param   lon      Longitude part of the coordinate.
        *   @return  The distance from the coordinates to
        *            the node.
        */
      static float64 getDistanceToNode( byte nodeNbr, GfxData* gfx, 
                                        int32 lat, int32 lon);


      /**
       *    Filter coordinates in this map, using filtering algorithm
       *    in GfxFilterUtil.
       *
       *    @param theMap The map to filter. Different filtering will be
       *                  applied on different types of maps, e.g
       *                  underview maps and country overview maps.
       */
      static void filterCoordinates(OldGenericMap* theMap);

      /**
       *     Filtering the gfx data of one item.
       *     @param item The item to filter.
       *     @param theMap The map of the item to filter.
       *     @param e1 In/Out parameter. Increased with the error before 
       *               Kolesnikov filter.
       *     @param e2 In/Out parameter. Increased with the error after
       *               Kolesnikov filter.
       *     @param totalCoords In/Out Increased with the number of coords 
       *                        of this item before filtering no matter if it
       *                        is filtered or not.
       *     @param totalCoordsToFilter In/Out Increased with the number of 
       *                        coords of this item before filtering only if it
       *                        has been filtered.
       *     @param unfilteredCoords In/Out Increased with the number of coords
       *                           of this item before it was filtered.
       *     @param filteredCoords In/Out Increased with the number of coords
       *                           of this item after if it was filtered.
       *
       *     @return True if the item was filtered.
       */
      static bool filterItem(OldItem& item, 
                             OldGenericMap& theMap,
                             double& e1, double& e2,
                             uint32& totalCoords, uint32& totalCoordsToFilter,
                             uint32& unfilteredCoords, uint32& filteredCoords);

      /**
       *    Filter items in one map, using filtering algorithm
       *    in GfxFilterUtil, common lines between items in the map are 
       *    filtered individually.
       *    Result is 16 levels of filtering. The level is stored using last 
       *    2 bits of latitude and longitude. Level 0 is 1 meters filtering
       *    to remove coordinates on "a string" and level 1-15 are further
       *    filterings based on level 0.
       *
       *    @param theMap The map to filter.
       */
      static void filterCoordinateLevels(OldGenericMap* theMap);

      /**
       *    Filter country polygon (=map gfx data of country overview map).
       *    Results in 16 levels of filtering. Level 0 is original data,
       *    level 1-15 are filterings upon filterings. If the map already has
       *    a filtered map gfx data, nothing is done.
       *    @param   theMap   The country overview map to filter.
       *    @param   breakPoints A set with coordinates defining common
       *                         lines between neighbouring country polygons.
       *    @return  True if the map gfx data was filtered, false if not
       *             or e.g. if the map was not a country overview map.
       */
      static bool filterCountryPolygonLevels(
                     OldGenericMap* theMap, set<MC2Coordinate> breakPoints );

      /**
       *    Extract break points from the country polygon break points
       *    text file. This file is structured according to
       *    "lat lon description"
       *    e.g.
       *    823915938 245139346 swe-fin-no
       *    785357201 288128788 swe-fin-ocean
       *
       *    @param   breakPointFile The text file with break point coordinates.
       *    @param   breakPoints    Where to store the break points.
       *    @return  True if ok, false e.g. if the text fiel does not exist.
       */
      static bool extractCoPolBreakPoints(
            const char* breakPointFile, set<MC2Coordinate>& breakPoints );

      /**
       *    Create boundry segments geographically.
       *    Coordinates of all nodes that are outside or inside and close
       *    (5 meters) to the map border (map gfx data must be true border) 
       *    are collected.
       *    All maps are looped again and all nodes close to the coordinates
       *    are used for creating virtual items which are added to the 
       *    boundry segments vector of the map.
       */
      static void createBorderBoundrySegments(
               const char* mapPath, uint32 startAtMap);
      
};

#endif

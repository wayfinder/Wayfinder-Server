/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSPOLYUTILITY_H
#define GMSPOLYUTILITY_H

#include "config.h"
#include "ItemTypes.h"
#include <map>
#include <set>

class OldGenericMap;
class GfxData;
class GMSGfxData;
class MC2Coordinate;
class MC2BoundingBox;

/**
  *   Class that holds methods for operations on item gfx data polygons
  *   such as elimination of holes and self-touchings.
  */
class GMSPolyUtility
{
   public:
      
   /**
    *    Remove un-necessary coordinates and polygon defects from
    *    item gfxdata, i.e. any coords that do not contribute to 
    *    the shape of polygons.
    */
   static uint32 rmPolygonDefectsAndUnnecessaryCoords(
                        OldGenericMap* theMap );

   /**
     *   Loop all items in the given map, and eliminate holes
     *   from the item gfx data. This will create self-touching
     *   polygons.
     *   Only processes on gfx data that have closed polygons.
     *   @param   theMap   The map to eliminate holes in.
     *   @param   elimItemType If given, only items of this item type
     *                     will have holes eliminated, else all items.
     */
   static void eliminateHoles( OldGenericMap* theMap, 
         set<MC2Coordinate>& coordsAddedToReverseLine,
         ItemTypes::itemType elimItemType = ItemTypes::numberOfItemTypes );
         
   /**
     *   Loop all items in the given map, and eliminate self-touches
     *   from the item gfx data.
     *   Only processes on gfx data that have closed polygons.
     *   @param   theMap   The map to eliminate self-touch in.
     *   @param   elimItemType If given, only items of this item type
     *                     will have self-touch eliminated, else all items.
     */
   static void eliminateSelfTouch( OldGenericMap* theMap,
         set<MC2Coordinate>& coordsAddedToReverseLine,
         ItemTypes::itemType elimItemType = ItemTypes::numberOfItemTypes );
         
   /**
    *    The id of the item that is processed, used for debug printing
    *    and needed since many of the methods operate on gfx data.
    */
   static uint32 myItemID;
   /**
    *    The id of the map that is processed, used for debug printing
    *    and needed since many of the methods operate on gfx data.
    */
   static uint32 myMapID;

   /**
    *    Merge water items in a country overview map. Large waters are
    *    moved from the underviews to the country overview. We need
    *    to merge waters that covers several underview maps. This method
    *    does it all for the water items: merge items,
    *    merge item polygons, eliminate holes and eliminate self-touch.
    *    It first checks with mergeSameNameCloseItems in NationalProperties
    *    if it is ok to merge waters in the particular co map.
    *    @param   theCOMap   The country overview map to process.
    *    @returns    True if something happened.
    */
   static bool mergeWaterItemsInCOMap( OldGenericMap* theCOMap );

   /**
    *    Merges close items of the same type that have exactly the same
    *    names. No names match no names.
    *    Check with mergeSameNameCloseItems method in NationalProperties
    *    for when to call this method.
    *    @param   theMap      The map in which to merge items.
    *    @param   itemType    The type of items to merge.
    *    @param   processClosedPolygons Set to true if to merge only closed 
    *                polygons, set to false if to merge only line items.
    *    @return Number of items that were involved in the merging
    */
   static uint32 mergeSameNameCloseItems( 
                     OldGenericMap* theMap, ItemTypes::itemType itemType,
                     bool preocessClosedPolygons );
  
   /**
    *    Loop all items in the map, and try to merge polygons within 
    *    the items using mergePolygons method in GMSGfxData.
    *    Merges items with the following item types: waterItem, parkItem, 
    *    forestItem, builtUpAreaItem (for now).
    *    @param   theMap      The map in which to merge item polygons.
    *    @param   mergeItemType If given only items with this type will
    *                         have their polygons merged. Else all item
    *                         types will be processed.
    *    @return  Number of items that had their polygons merged.
    */
   static uint32 mergeItemPolygons( OldGenericMap* theMap, 
         ItemTypes::itemType mergeItemType = ItemTypes::numberOfItemTypes );
  
   private:
   
   typedef set< pair<uint32,uint32> > idxSet_t;
   typedef map< MC2Coordinate, idxSet_t > coordMap_t;
   
   /**
    *    Remove polygon defects from one item gfxdata. Only works on
    *    closed polygons. If the provided gfxdata is not closed the method
    *    returns NULL.
    */
   static GMSGfxData* 
      rmPolygonDefectsAndUnnecessaryCoordsFromClosed( GMSGfxData* gfx );
    
   /**
     *   Eliminate holes from the given gfx data. 
     *   This will create self-touching polygons.
     *   @return A new GMSGfxData if any holes were eliminated,
     *           NULL if there was nothing to eliminate (or failed).
     */
   static GMSGfxData* eliminateHoles( 
         GMSGfxData* gfx, 
         bool& hasProblems,
         set<MC2Coordinate>& coordsAddedToReverseLine );
         
   /**
     *   Eliminate self-touch from the given gfx data. It will eliminate
     *   falukorvs and also split up self-touching positive polys.
     *   @return A new GMSGfxData if any self-touch was eliminated,
     *           NULL if there was nothing to eliminate (or failed).
     */
   static GMSGfxData* eliminateSelfTouch( 
         GMSGfxData* gfx, 
         bool& hasProblems,
         set<MC2Coordinate>& coordsAddedToReverseLine );
         
   /**
    *    Build poly hole hierarchy for a gfxdata, i.e. find out
    *    which positive polygons the holes of the gxf data are located in.
    *    It will return a multimap with polyIdx of the postive polygon
    *    as key and the polyIdx of the hole as value.
    *    Only positive polygons that have holes will be included
    *    in the result.
    *    @param   gfx            The gfxdata to build hierarchy for.
    *    @param   holeHierarchy  The multimap to fill with hierarchy.
    *    @return  True if the gfx data has any holes and the
    *             hierarchy is filled ok. False if the gfx data has no holes 
    *             someting failed during detection (example twisted polygons 
    *             making it impossible to decide orientation).
    */
   static bool getPolyHoleHierarchy( 
      GMSGfxData* gfx, multimap<uint32, uint32>& holeHierarchy,
      bool& hasProblems );

   /**
    *    Get the convex hull of all holes of a specific polygon of
    *    a gfx data. The inparam holeHierarchy (defines
    *    which polygons that are holes of which positive polygons
    *    of this gfx data) is created with getPolyHoleHierarch.
    *    Will not consider the holes defined by excludeTheseHoles.
    *    @return  The convexhull gfxdata, NULL if no convex hull
    *             could be created, e.g. if there were no holes
    *             for the polygon.
    */
   static GMSGfxData* createConvexHullOfHolesOfOnePoly( 
         GMSGfxData* gfx, uint32 poly, 
         multimap<uint32, uint32> holeHierarchy,
         set<uint32> excludeTheseHoles );

   /**
    *    Help method used in eliminateHoles and eliminateSelfTouch.
    *    It will define a line from a coordinate on the convex hull of 
    *    the hole or self-touch-part (depending on main method) 
    *    towards the polygon border.
    *    @param   convHull    The convex hull of the hole/self-touch-part
    *    @param   coordOnHull The coordinate on the convex hull where 
    *                to start drawing the line.
    *    @param   convHullCoordIdx   The index of the coordinate on the
    *                convex hull where to start drawing the line.
    *    @param   polyBbox    The bbox of the polygon where the hole or
    *                self-tocuh-part is located. It will be used to 
    *                make sure that the line is long enough.
    *    @param   endLat      Outparam, the latitude end of the line
    *    @param   endLon      Outparam, the longitude end of the line
    *    @return  True if the line was created and the outparams set,
    *             false otherwise.
    */
   static bool drawLineTowardsPolyBorder(
      GfxData* convHull, MC2Coordinate coordOnHull,
      uint32 convHullCoordIdx, MC2BoundingBox polyBbox,
      int32& endLat, int32& endLon, bool printDebugs = false);

   /**
    *    There is a line from a start coord (which is inside the polygon 
    *    poly of a gfx data) to a end coord which is outside the polygon.
    *    Decide the point on the polygon border which intersects with 
    *    this line.
    *    Via outparams the method returns the intersection point and 
    *    the index of the NEXT coordinate on the poly border.
    *    If the poly border was an old line previously drawn to the
    *    true poly border, there is a reversed line, and the method
    *    also returns the NEXT coordinate on this reversed line = 
    *    a place where you want to insert the interCoord.
    *
    *    @return  True if the intersection point was found and outparams
    *             set, false if not.
    */
   static bool findPolyBorderIntersectionPoint( 
      GMSGfxData* gfx, uint32 poly, 
      MC2Coordinate startCoord, int32 endLat, int32 endLon,
      MC2Coordinate& interCoord, uint32 &nextCoordIdx,
      uint32 &reversedLineNextCoordIdx,
      bool printDebugs = false,
      uint32 polyStartIdx = MAX_UINT32,
      uint32 polyEndIdx = MAX_UINT32 );

   /**
    *    Calls the GfxData::getPointOnConvexHull
    */
   static uint32 getPointOnConvexHull(GMSGfxData* gfx, int polyIndex);

   /**
    *    Check if a gfx data has polygons that are self-touching =
    *    some coordinates within the polygon are shared.
    *    Either checks only one specified polygon, returning the
    *    coordinates that are touching each other. It skips if the
    *    first and the last coordinate of the poly is equal.
    *    The other option is to loop all polygons and analyse all
    *    of them in the same way. NB: It is not comparing poly-vs-poly,
    *    only comparing within each poly.
    *    @param   gfx   The gfxdata to analyse for self-touch
    *    @param   polyNbr  If set, only this poly of the gfxdata will
    *                   be analysed.
    *    @param   selfTouchingCoords   Outparam filled with the
    *                coordinates that are shared within the polygon(s).
    *    @return True if some of the analysed polygon(s) are self-touching,
    *            false if no self-touches are found.
    */
   static bool isSelfTouching( GMSGfxData* gfx, 
      coordMap_t& selfTouchingCoords, uint16 polyNbr = MAX_UINT16 );

   /**
    *    Help method for debug of half-eliminated polygons.
    *    It will create a new mcm map and add one item to it. The
    *    item gfx data is a gfx data that has problems. It will also
    *    print the gfxdata to mif file gfxWithProblems.mif.
    *    @param   gfx   The gfxdata with problems.
    *    @param   mapID The created map will have this id.
    */
   static void createAndSaveAMapFromGfxData( GMSGfxData* gfx, uint32 mapID );

   /**
    *    Check if the holes of a gfx data are touching each other,
    *    i.e. share one or more coordinates.
    *    return True if some hole is touching another hole.
    */
   static bool holesAreTouching( 
         GMSGfxData* gfx, multimap<uint32, uint32> holeHierarchy );
};

// =======================================================================
//                                     Implementation of inlined methods =



#endif


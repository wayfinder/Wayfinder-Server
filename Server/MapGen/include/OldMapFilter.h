/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDMAPFILTER_H
#define OLDMAPFILTER_H

#include "config.h"

#include "ItemTypes.h"
#include <set>
#include <map>
#include <vector>
#include <list>
#include "MC2Coordinate.h"
#include "IDPairVector.h"
#include "Stack.h"
#include "GfxData.h"
#include "GfxFilterUtil.h"
#include "OldGenericMap.h"
#include "GMSGfxData.h"

/**
 * Filters all polygons in a map using given filteringlevels.
 */
class OldMapFilter {

   public:

      OldMapFilter();
      virtual ~OldMapFilter();

      /**
       * Filters all polygons in a map using given filtering levels.
       * If a polygon shares parts of itself with another polygon
       * it will filter that part separately to avoid holes
       * between the polygons. The result of this filter is that the 
       * last "nrOflsbsToBeUsed" bits in all the coords in all the polygons
       * in all the gfxdatas in mapToFilter will be set to represent
       * at what zoom/filter level the coordinate should be used.
       *  
       * To avoid extreme slowdowns the "filterMaxDists" should begin at 
       * low values, say 1 in the normal case and increase with 50-100% for 
       * each zoom/filter level. 
       *
       * The resulting lsb's data can look like this (for nrLsb = 4).
       * If we have a coordinate that is supposed to be shown at filter 
       * level 5 and has the original coords lat: 667797096 
       * lon: 159293744 then it can have the resulting coords 
       * lat: 667797088 lon: 159293745.
       * If we look at the resulting values in hex we will find 
       * lat: 27cdc660 lon: 97ea131 and we can see that the last 4 bits in
       * lat is 0000 and in lon they are 0001. if we put lat in front of 
       * lon we get lat lon -> 0000 0001 -> 00000001 and we have our 
       * zoom/filter level.
       * 
       * @param mapToFilter   The map that we want to filter.
       * @param mapsToConsider A collection of maps used when looking for
       *          shared polygon parts between different items.
       * @param itemTypes     The types of items to filter togheter, i.e. to 
       *                      be tested for shared polygon parts.
       * @param useGfxData    Not useful for anything right now.
       * @param filterMaxDists The filtering level distances used by the filter.
       */
      void filter( 
            OldGenericMap* mapToFilter,
            const set<OldGenericMap*>& mapsToConsider,
            const set<ItemTypes::itemType>& itemTypes,
            bool  useGfxData,
                  vector<int> filterMaxDists );

      /**
       *    Filter the country polygon (= map gfx data) of a country 
       *    overview map. Shared polygon parts between different countries
       *    are defined using set of break point coordinates.
       *
       *    @param   mapToFilter The map to filter country polygon for.
       *    @param   filterMaxDists The filter level distances.
       *    @param   breakPoints    A set with coordinates defining common
       *                            lines between country polygons.
       */
      void filterCountryPolygon(
            OldGenericMap* mapToFilter, vector<int> filterMaxDists,
            set<MC2Coordinate> breakPoints );

      /// Get map gfx data filter distance for a certain filter level (1-15).
      inline int getFiltDistanceForMapGfx( uint32 level );

      /*
       * Get map gfx data filter high level distance for a certain 
       * filter level (1-15).
       *
       */
      inline int getFiltHighLevelDistanceForMapGfx( uint32 level );

      /** 
       *    Get area item filter distances for a certain filter level (1-15).
       *    @param   minAreaFilterDistance   The minimum filter distance
       *                to use for filter level 1, e.g. if a filtering to
       *                remove coords-on-a-string have been done first and
       *                those coordinates removed. Default -1 means that
       *                no min distance should be considered.
       */
      inline int getFiltDistanceForAreaItem(
                  uint32 level, int minAreaFilterDistance = -1 );

      /**
       *    Get line item filter distances for a certain filter level (1-15).
       *    @param   minLineFilterDistance   The minimum filter distance
       *                to use for filter level 1, e.g. if a filtering to
       *                remove coords-on-a-string have been done first and
       *                those coordinates removed. Default -1 means that
       *                no min distance should be considered.
       */
      inline int getFiltDistanceForLineItem(
                  uint32 level, int minLineFilterDistance = -1 );

      /**
       *    Print a filtered country border part to countryBorders.txt.
       *    Used e.g. when saving filtered country border parts for re-using
       *    filtering for neighbouring countries.
       *    The country border part gfx must have true filtered coords, the
       *    identification points (first, last, onTheWay) must be blanked 
       *    (the bits used for storing coordinate filter level are balnked).
       *
       *    @param   firstBreakPoint   The first coord of the border part.
       *    @param   lastBreakPoint    The last coord of the border part.
       *    @param   onTheWayPoint     A coordinate at offset 50% of the
       *                               border part.
       *    @param   countryMapName    Name of the map from which this border 
       *                               part is extracted.
       *    @param   printGfx The gfx data containing the filtered border part.
       *    @param   poly     Which poly of printGfx contains the border part.
       *    @param   startSplitIdx Coord idx in printGfx poly, where the border
       *                      part starts (corresponds to firstBreakPoint).
       *    @param   nbrCoordsInBorderPart   How many coordinates there are
       *                      in the border part.
       */
      static void printCountryBorderToFile(
         MC2Coordinate firstBreakPoint, MC2Coordinate lastBreakPoint,
         MC2Coordinate onTheWayPoint,
         const char* countryMapName, GfxData* printGfx, const uint32 poly,
         int startSplitIdx, int nbrCoordsInBorderPart);

      /**
       *    Method for re-using already filtered country borders
       *    for a new country, in order to avoid gaps between countries.
       *    Reads from a file countryBorders.txt which holds already filtered
       *    country polygon parts, and tries to find a match to the current
       *    polygon part of the new country. If an old filtering matches,
       *    the filtering is added to outGfx and no individual filtering
       *    of the new part must be done.
       *    If an old filtering is found and out country is another compared
       *    to the one for the old filtering, the borderItems.txt file is
       *    writen. It may later be used for creating border items in co-maps.
       *
       *    The method can also be used for ust checking if a country border
       *    part already is written in the countryBorders.txt file or not.
       *    To mark this, set the outGfx NULL when calling the method.
       *
       *    @param   gfxPartToFilter   One part of the border of a country
       *                               polygon to filter, coordinates blanked.
       *    @param   countryMapName    Name of the map from which this border 
       *                               part is extracted.
       *    @param   firstBreakPoint   The first coord in gfxPartToFilter.
       *    @param   lastBreakPoint    The last coord in gfxPartToFilter.
       *    @param   onTheWayPoint     A coordinate at offset 50% of
       *                               gfxPartToFilter.
       *    @param   gfxPartForward Set to true if the gfxPartToFilter is 
       *                      oriented the same way the original gfx part is,
       *                      set to false if the gfxPartToFilter was created 
       *                      backwards.
       *    @param   outGfx   The filtered counterpart of the country polygon.
       *                      Set to NULL if we should only check if the contry
       *                      border is in border file.
       *    @param   polyNbr  Which polygon of the country that is beeing 
       *                      filtered.
       *
       *    @return  True if an old filtering was used, false if not.
       */
      static bool oldBorderFiltering(
            GfxData* gfxPartToFilter, const char* countryMapName,
            MC2Coordinate firstBreakPoint, MC2Coordinate lastBreakPoint,
            MC2Coordinate onTheWayPoint,
            bool gfxPartForward,
            GfxDataFull* outGfx, uint32 polyNbr );

      /**
       *    Help method for e.g. oldBorderFiltering. Find out if a 
       *    coordinate is enough close to a border part. Close enough
       *    is if the distance is less than 50 meters between the coordinate
       *    and the borderPart gfx. But if the borderPart has a length of more
       *    than 100 km 150 meters is close enough.
       *    @param   borderPart  Border part gfx data.
       *    @param   coord       Coord to check distance to.
       *    @�aram   sqDist      Outparam that is set to the distance in meters.
       *    @return  True if the distance between the coord and the border part
       *             is close enough.
       */
      static bool coordCloseToBorderPart( const GfxData* borderPart,
                                          const MC2Coordinate coord,
                                          float64& sqDist );

      /**
       *    Variables used for masking the filter level bits. If changing here
       *    please also change in FilteredCoord class.
       */
      //@{
         /**
          *    Number of bits to use for storing filter level in latitude 
          *    and longitude.
          */
         static const uint32 nrOflsbsToBeUsed;

         /**
          *    Mask for blanking out the bits in coordinates that are
          *    used for storing the filter level information.
          */
         static const int coordBlankMask;
      //@}

   private:
      /**
       * A gigantic container that keeps track of all coordinates in 
       * mapToFilter and in mapsToConsider so that we can test if 
       * a coordinate shares lat and lon with another coordinate
       * in a simple and effective way. Used for detecting shared
       * polygon parts.
       */
      typedef map<MC2Coordinate, set<IDPair_t> > coordMap_t;

      /**
       *    The filter distances to be used when filtering map gfx data,
       *    are items, or line items.
       */
      //@{
         /**
          *    Vector with filter distances to be used when filtering a
          *    map gfx data (e.g. country polygon).
          */
         vector<int> m_mapGfxFilterDistances;

         /**
          *    Vector with filter high level distances to be used when 
          *    filtering a map gfx data (e.g. country polygon).
          */
         vector<int> m_mapGfxFilterHighLevelDistances;

         /*
          * Tells if the vector with map gfx data filter low level 
          * dists is initialised.
          */
         bool m_mapGfxDistDefined;

         /*
          * Tells if the vector with map gfx data filter high level           
          * dists is initialised.
          */
         bool m_mapGfxHighLevelDistDefined;

         /**
          *    Vector with filter distances to be used when filtering
          *    area items (e.g. parks, built-up areas).
          */
         vector<int> m_areaItemFilterDistances;

         /// Tells if the vector with area item filter distances is initialised.
         bool m_areaItemDistDefined;

         /**
          *    Vector with filter distances to be used when filtering
          *    line items (e.g. street segments, ferry items).
          */
         vector<int> m_lineItemFilterDistances;

         /// Tells if the vector with line item filter distances is initialised.
         bool m_lineItemDistDefined;
      //@}

      /**
       *    Init (= fill) the vector with map gfx data filter distances.
       *    Done only once, status given by the m_mapGfxDistDefined.
       */
      inline void initMapGfxDistances();
     
      /**
       *    Init (= fill) the vector with map gfx data filter distances.
       *    Done only once, status given by the m_mapGfxDistDefined.
       */
      inline void initMapGfxHighLevelDistances();
       
      /**
       *    Init (= fill) the vector with area item filter distances. Done
       *    only once, status given by m_areaItemDistDefined.
       *    @param   minAreaFilterDistance   The minimum filter distance
       *                to use for filter level 1. Default -1 means that
       *                no min distance should be considered.
       */
      inline void initAreaItemDistances( int minAreaFilterDistance = -1 );
      
      /**
       *    Init (= fill) the vector with line item filter distances. Done
       *    only once, status given by m_lineItemDistDefined.
       *    @param   minLineFilterDistance   The minimum filter distance
       *                to use for filter level 1. Default -1 means that
       *                no min distance should be considered.
       */
      inline void initLineItemDistances( int minLineFilterDistance = -1 );
      
      /**
       * Fills the coordIntersectMap with item coordinates from
       * mapToFilter and mapsToConsider for given item types.
       *
       * @param theMap The map that we want to filter.
       * @param itemTypes The types of items to be tested for shared
       * polygon parts.
       * @param useGfxData Not useful for anything right now.
       * @param coordIntersectMap see above: typedef map.
       */
      void updateCoordIntersectionMap( 
                  OldGenericMap* theMap,
            const set<ItemTypes::itemType>& itemTypes,
            bool  useGfxData,
                  coordMap_t& coordIntersectMap );

      /**
       *    Fills the coordIntersectMap with map gfx data coordinates
       *    from mapToFilter.
       *
       *    @param   theMap   The map that we want to filter.
       *    @param   coordIntersectMap see above: typedef map.
       */
      void updateCoordIntersectionMapForMapGfx(
               OldGenericMap* theMap,
               coordMap_t& coordIntersectMap );

      /**
       *    Filter all items of given types. Uses information in the
       *    coordIntersectMap to split polygons into smaller part for
       *    filtering shared polygon parts (common lines) individually.
       *    @param   theMap      The map to filter.
       *    @param   itemTypes   The type of items to filter.
       *    @param   useGfxData  Param not used.
       *    @param   coordIntersectMap Contains all interesting coordinates,
       *                         used for detecting shared polygon parts.
       *    @param   filterMaxDists The filter level distances.
       */
      
      void filterUsingInfoInCoordsIntersectionMap( 
                  OldGenericMap* theMap,
            const set<ItemTypes::itemType>& itemTypes,
            bool  useGfxData,
                  coordMap_t& coordIntersectMap,
                  vector<int> filterMaxDists );

      /**
       *    Filter one gfx data, either an item gfx or e.g. the country polygon
       *    (map gfx data of country overview map).
       *    @param   theMap      The map to filter (param not used).
       *    @param   gfx         The gfxData to filter.
       *    @param   newGfx      The gfxData to fill with the filtered version
       *                         of gfx.
       *    @param   useGfxData  Param not used.
       *    @param   coordIntersectMap Contains all interesting coordinates,
       *                         used for detecting shared polygon parts.
       *    @param   filterMaxDists The filter level distances.
       *    @param   countryBorder  Set to true if filtering country polygon.
       *    @param   dumpFilterInfo Set to true for debug-printing, default 
       *                         is false.
       */
      void filterOneGfxUsingInfoInCoordsIntersectionMap( 
                  OldGenericMap* theMap,  // param not really needed
                  const GfxData* gfx,
                  GfxDataFull* newGfx,
                  bool useGfxData,
                  coordMap_t& coordIntersectMap,
                  vector<int> filterMaxDists,
                  bool countryBorder,
                  bool dumpFilterInfo = false );

      /**
       * Takes some indata and filters one part of one polygon, 
       * gives the result in outGfx.
       *
       * Why is this function templated?
       *
       * @param mapToFilter The map that we want to filter (param not needed).
       * @param xyHelper converts from mc2coords to non projected coords. 
       * @param opfBegin index to where we want to start looking in gfx. 
       * @param opfEnd index to where we want to stop looking in gfx
       *               (stops filtering on the coordinate previous to opfEnd).
       * @param filterMaxDists The filtering levels used by the filter.
       * @param p Index to the polygon that we want to filter, 
       *          its inside gfx and we need p to know where in gfx it is.
       * @param gfx The container that holds our polygon.
       * @param outGfx The container that will hold the resulting polygon.
       * @param countryBorder Set to true if filtering a country polygon,
       *                trying to re-use old country border filterings.
       * @param breakPoints   Set to true if the polygon part is split
       *                by break points (the part is shared between items or 
       *                countries).
       */ 
      template<class XYHELPER>
         inline void filterSegment(
                     OldGenericMap* mapToFilter,
               const XYHELPER& xyHelper, 
               const uint32 opfBegin,
               const uint32 opfEnd,
               const vector<int> filterMaxDists,
               const uint32 p,
               const GfxData* gfx,
                     GfxDataFull* outGfx,
                     bool countryBorder,
                     bool breakPoints,
                     bool dumpFilterInfo = false );

      /**
       *    Dump the info in the 2 dim array (paths) that store coord
       *    indeces between filter levels.
       */
      inline void dumpPaths( int** paths, int rows, int cols ) const;

      /**
       *    Check that the vector with filter distances includes valid
       *    and reasonable distance values. There must e.g. not be more
       *    than 15 distances.
       */
      bool validFilterLevelDistances( vector<int> filterMaxDists );

      /**
       *    Write a border part to the border item file borderItems.txt.
       *    If the border part is already in the border file it is
       *    not added again.
       *    @param   borderId    The id of the border part to write 
       *                         (id in countryBorders.txt file).
       *    @param   firstCoord  The first coord of the border part.
       *    @param   lastCoord   The last coord of the border part.
       *    @param   middleCoord A coordinate at offset 50% of the border part.
       */
      static void printBorderItemToFile( uint32 borderId,
         MC2Coordinate& firstCoord, MC2Coordinate& lastCoord,
         MC2Coordinate& middleCoord );

};

// (((=====================================================================
//                                      Implementation of inlined methods =

template<class XYHELPER> 
inline void OldMapFilter::filterSegment(
            OldGenericMap* mapToFilter,
      const XYHELPER& xyHelper,
      const uint32 opfBegin,
      const uint32 opfEnd,
      const vector<int> filterMaxDists,
      const uint32 p,
      const GfxData* gfx,
            GfxDataFull* outGfx,
            bool countryBorder,
            bool breakPoints,
            bool dumpFilterInfo ) {
   
   // Why is this function templated?
 
   const char* countryMapName = mapToFilter->getMapName();

   const uint32 nbrCoordsInGfxPoly = gfx->getNbrCoordinates(p);
   // Get size of the polygon part to filter, from opfBegin to opfEnd.
   // Note that opfBegin might be larger than (or equal to) opfEnd 
   // if filtering a a shared polygon part that extends over the coord 0.
   int tmpFiltSize = opfEnd - opfBegin;
   if ( tmpFiltSize <= 0 ) {
      tmpFiltSize = nbrCoordsInGfxPoly + opfEnd - opfBegin;
   }
   const int filtSize = tmpFiltSize;
   
   const int setLevelMask = 0xffffffff - coordBlankMask + 1;

   if ( dumpFilterInfo ) {
      cout << " filter poly=" << p << " opfBegin=" << opfBegin << " opfEnd="
           << opfEnd << " (" << nbrCoordsInGfxPoly << ")" << endl;
   }
   // If trying to filter just one coordinate,
   // don't filter if the coord is part of a polygon with more coords since
   // the coord will come back in next segment and be filtered then.
   // If just one coord in the polygon, set filter level to max.
   if ( filtSize == 1 ) {
      if ( ( outGfx->getNbrCoordinates(p) == 0 ) &&
           ( nbrCoordsInGfxPoly == 1 ) ) {
      
         uint32 filterLatValue = filterMaxDists.size() / setLevelMask;
         uint32 filterLonValue = filterMaxDists.size() % setLevelMask;
      
         outGfx->addCoordinate(
               (gfx->getLat( p, opfBegin ) & coordBlankMask) + filterLatValue,
               (gfx->getLon( p, opfBegin ) & coordBlankMask) + filterLonValue );
         if ( dumpFilterInfo )
            cout << " Not filtering 1 coord, but adding to outGfx "
                 << p << ":" << opfBegin << endl;
      }
      else if ( dumpFilterInfo ) {
         cout << " Not filtering 1 coord, not adding to outGfx "
              << p << ":" << opfBegin << endl;
      }
      return;
   }
   DEBUG8(
   if ( countryBorder && ((p == 0) || (p == 1)) ) {
      mc2dbg << "   filter " << opfBegin << "->" << opfEnd 
             << " size=" << filtSize << endl;
   });
   
   // Decide if to filter from opfBegin (forwards) or from opfEnd-1 (backwards)
   // always filter from lowest lat/lowest lon to make shared polygon parts
   // be filtered in the same direction
   bool filterForwards = false;
   uint32 endSplitIdx = opfEnd-1;
   if ( endSplitIdx < 0 ) {
      endSplitIdx = nbrCoordsInGfxPoly - 1;
   } else if ( endSplitIdx >= nbrCoordsInGfxPoly ) {
      endSplitIdx = endSplitIdx - nbrCoordsInGfxPoly;
   }
   if ( gfx->getLat(p, opfBegin) < gfx->getLat(p, endSplitIdx) ) {
      filterForwards = true;
   } else if ( gfx->getLat(p, opfBegin) > gfx->getLat(p, endSplitIdx) ) {
      filterForwards = false;
   } else if ( gfx->getLon(p, opfBegin) < gfx->getLon(p, endSplitIdx) ) {
      filterForwards = true;
   }
   // debugprint
   if ( countryBorder && breakPoints /*& ((p == 0) || (p == 1))*/ ) {
      MC2Coordinate from( gfx->getLat(p, opfBegin), gfx->getLon(p, opfBegin) );
      uint32 toIdx = ( (opfBegin+filtSize-1) % nbrCoordsInGfxPoly );
      MC2Coordinate to( gfx->getLat(p, toIdx), gfx->getLon(p, toIdx) );
      mc2dbg1 << "Filter country borderPart poly=" << p 
           << " forwards=" << int(filterForwards)
           << " f=" << opfBegin << "->" << toIdx << " " << from << "->" << to
           << " b=" << toIdx << "->" << opfBegin
           << endl;
   }
   
   // Fill the tempPoly with the original values to init it.
   GfxDataFull* tempPolyGfx = GMSGfxData::createNewGfxData( static_cast<OldGenericMap* >( NULL ), true );
   if ( filterForwards ) {
      for( int l = 0; l < filtSize; l++ ) {
         const uint32 c = ( (l + opfBegin) % nbrCoordsInGfxPoly );
         tempPolyGfx->addCoordinate(
               ( gfx->getLat( p, c ) & coordBlankMask ),
               ( gfx->getLon( p, c ) & coordBlankMask ),
               false );
      }
   } else {
      for( int l = filtSize-1; l >= 0; l-- ) {
         const uint32 c = ( (l + opfBegin) % nbrCoordsInGfxPoly );
         tempPolyGfx->addCoordinate(
               ( gfx->getLat( p, c ) & coordBlankMask ),
               ( gfx->getLon( p, c ) & coordBlankMask ),
               false );
      }
   }

   // If filtering country polygons, check if this border was filtered once
   // before from another country. If so, then don't filter again, but copy
   // the already filtered border to avoid gaps between countries.
   // Only apply to polygonPARTS, those polygons that are split by breakpoints
   // (whole polygons = not shared between countries)
   MC2Coordinate firstBreakPoint, lastBreakPoint, onTheWayPoint;
   if ( countryBorder && breakPoints ) {
      mc2dbg << "Part of country border, size = "
             << tempPolyGfx->getNbrCoordinates(0) << endl;
      firstBreakPoint = MC2Coordinate(
         tempPolyGfx->getLat(0,0), tempPolyGfx->getLon(0,0) );
      lastBreakPoint = MC2Coordinate(
         tempPolyGfx->getLat(0,filtSize-1), tempPolyGfx->getLon(0,filtSize-1) );

      int32 lat, lon;
      uint16 offset = uint16( 0.5 * MAX_UINT16 );
      tempPolyGfx->updateLength();
      if ( tempPolyGfx->getCoordinate( offset, lat, lon ) ) {
         onTheWayPoint = MC2Coordinate( lat, lon );
      } else {
         onTheWayPoint = MC2Coordinate( 0, 0 );
      }
      mc2dbg << "Ident points: first=" << firstBreakPoint << " last="
             << lastBreakPoint << ", onTheWay=" << onTheWayPoint << endl;
      
      bool usedOld = oldBorderFiltering( 
            tempPolyGfx, countryMapName,
            firstBreakPoint, lastBreakPoint, onTheWayPoint,
            filterForwards, outGfx, p );
      
      if ( usedOld ) {
         // Return here, coords were added to outGfx in oldBorderFiltering
         return;
      }
   }
  
   /**
    * paths is a 2 dimensional array that stores the indeces from 
    * every filtering level so that we can backtrack later and set 
    * the lsb's in lat and lon to correct values.
    * 
    */
   int**  paths= new int*[ filterMaxDists.size() + 1 ];
   for ( int i = 0; i < (int)filterMaxDists.size() + 1; ++i ) {
      paths[ i ] = new int[ filtSize ];
   }
   for( int l = 0; l < filtSize; l++ ) {
      paths[ 0 ][ l ] = l;
   }
   for( int i = 1; i < (int)filterMaxDists.size() + 1; i++ ) {
      for( int l = 0; l < filtSize; l++ ) {
         paths[ i ][ l ] = -1;
      }
   }

   /* For all filterlevels up to level 9, filter the polygon part and then 
    * set its lsb's to the values calculated and stored in paths. Backtrack 
    * to find correct values for the lsb's.
    */

   // Filter distance = filterMaxDists[ i ] - filterMaxDists[ i-1 ]
   // since we already have filtered filterMaxDists[ i-1 ] meters..
   uint32 filterDist;

   // The level used for all higher level filterings. If you change
   // you must also change in the getFiltHighLevelDistanceForMapGfx
   // method. Note that tweaking of filter distance will be required
   // in order to guarantee that the same number of coordinates on each
   // level will result from the filtering.
   int newStartLevel = 9;

   int tempPolyGfxSize = filtSize;
   for( int i = 0; i < newStartLevel; i++ ) {
      // Indata to filter from pre-filter
      vector<int> rsPath;
      // Outdata from filter
      vector<int> outIndeces;
      // Outdata from openPolygonFilter
      Stack coordIdx; 

      // Filter distance = filterMaxDists[ i ] - filterMaxDists[ i-1 ]
      // since we already have filtered filterMaxDists[ i-1 ] meters..
      uint32 filterDist = filterMaxDists[ i ];
      if ( i > 0 ) {
         filterDist -= filterMaxDists[ i -1 ];
      }
      
      // Do the pre-filter filter.
      tempPolyGfx->openPolygonFilter( 
            &coordIdx, i, filterDist,
            MAX_UINT32, true,
            0,
            tempPolyGfxSize - 1 );
      
      // Do the filter filter.
      for ( uint32 j = 0; j < coordIdx.getStackSize(); ++j ) {
         rsPath.push_back( coordIdx.getElementAt( j ) );
      }
      // Use the non-filtering iterators.
      GfxData::const_iterator beginIt = tempPolyGfx->polyBegin( i );
      GfxData::const_iterator endIt = tempPolyGfx->polyEnd( i );
       
      GfxFilterUtil::filter( 
            outIndeces,
            rsPath,
            MC2CoordXYHelper(),
            beginIt, endIt );
 
      /**
       * Put the out indeces in paths so that we can use it
       * for backtracking when looking for the correct lsb's
       * later on.
       */
      for( int l = 0; l < (int)( outIndeces.size() ); l++ ) {
         paths[ i + 1 ][ l ] = outIndeces[ l ];  
      }
      // Init the tempPolyGfx for the next filterlevel.
      tempPolyGfx->addPolygon();
      for( int j = 0; j < (int)outIndeces.size(); j++ ) {
         tempPolyGfx->addCoordinate(
               tempPolyGfx->getLat( i, outIndeces[ j ] ),
               tempPolyGfx->getLon( i, outIndeces[ j ] ) );
      }
      tempPolyGfxSize = outIndeces.size();
   }
  
   /* 
    * Next, filter to get levels 15, 14..., 10, using newStartLevel as 
    * starting point. Note that this must be done in reverse 
    * order (15, 14...) since the resulting coordinates from
    * a level M must be included in M-1.
    *
    */
 
   // Initializations
   OldMapFilter mapFilter;

   // We stopped the lower level filtering at level 
   // newStartLevel => tempPolyGfxSize contains newStartLevelSize
   uint32 newStartLevelSize = tempPolyGfxSize;

   vector<int> splitIndeces;
   vector<int> splitFilterIdxs; 
   vector<int> outIndeces;

   // Note: i represents the level that we are creating, 
   // as opposed to the level we are filtering, which is the way it
   // was represented for the lower level filtering above.
   for( int i = 15; i > newStartLevel; i-- ) {
      // Indata to filter from pre-filter
      vector<int> rsPath;
      // Outdata from openPolygonFilter
      Stack coordIdx;

      if( i == 15) {
         // Determine filter distance for level i+1.
         filterDist = mapFilter.getFiltHighLevelDistanceForMapGfx( i );
         
         // Do the pre-filter filter.
         tempPolyGfx->openPolygonFilter( 
               &coordIdx, newStartLevel, filterDist,
               MAX_UINT32, true,
               0,
               newStartLevelSize - 1 );
     
         // Do the filter filter.
         for ( uint32 j = 0; j < coordIdx.getStackSize(); ++j ) {
            rsPath.push_back( coordIdx.getElementAt( j ) );
         }
         // Use the non-filtering iterators.
         GfxData::const_iterator beginIt = 
            tempPolyGfx->polyBegin( newStartLevel );
         GfxData::const_iterator endIt = 
            tempPolyGfx->polyEnd( newStartLevel );
       
         GfxFilterUtil::filter( 
               outIndeces,
               rsPath,
               MC2CoordXYHelper(),
               beginIt, endIt );

         /**
          * Put the out indeces in paths so that we can use it
          * for backtracking when looking for the correct lsb's
          * later on.
          */
         for( int l = 0; l < (int)( outIndeces.size() ); l++ ) { 
            paths[ i ][ l ] = outIndeces[ l ];  
         }
      }
      else {
         // New split indices = indeces of the previous level = outIndeces
         splitIndeces.clear();
         splitIndeces = outIndeces;
         outIndeces.clear();
         
         // Determine filter distance for level i.
         filterDist = mapFilter.getFiltHighLevelDistanceForMapGfx( i );

         // Current pair of split indeces (see below)
         uint16 startSplitIdx;
         uint16 endSplitIdx;

         /*
          * Go through each pair of split indeces, filter 
          * the part 'between them' separately, and then put all
          * filtered parts together.
          *
          */
         for( int j = 0; j < (int)splitIndeces.size() - 1; j++ ) {
            rsPath.clear();
            coordIdx.reset();
            splitFilterIdxs.clear();

            // Determine current pair of split indeces
            startSplitIdx = splitIndeces[ j ];
            endSplitIdx = splitIndeces[ j + 1 ];
            
            // Create temporary gfx data for split part
            GfxDataFull* tempPolyGfxSplit
                = GMSGfxData::createNewGfxData( 
                   static_cast<OldGenericMap* >( NULL ), true );

            for( int l = startSplitIdx; l <= endSplitIdx; l++ ) {
               tempPolyGfxSplit->addCoordinate(
                  ( tempPolyGfx->getLat( newStartLevel, l ) & coordBlankMask ),
                  ( tempPolyGfx->getLon( newStartLevel, l ) & coordBlankMask ),
                  false );
            }
 
            // Do the pre-filter filter.
            tempPolyGfxSplit->openPolygonFilter(
                  &coordIdx, 0 , filterDist,
                  MAX_UINT32, true,
                  0,
                  endSplitIdx - startSplitIdx);

            // Store result from openPolygon in rsPath
            mc2dbg8 << "Result idx from openPolygon: ";
            for ( uint32 j = 0; j < coordIdx.getStackSize(); ++j ) {
               mc2dbg8 << coordIdx.getElementAt( j ) + startSplitIdx << " ";
               rsPath.push_back( coordIdx.getElementAt( j ) );
            }
            mc2dbg8 << endl;

            // Set iterators needed by filter-filter (Kolesnikov)
            GfxData::const_iterator beginIt =
               tempPolyGfxSplit->polyBegin( 0 );
            GfxData::const_iterator endIt =
               tempPolyGfxSplit->polyEnd( 0 );

            // Do the filter filter.            
            GfxFilterUtil::filter(
                  splitFilterIdxs,
                  rsPath,
                  MC2CoordXYHelper(),
                  beginIt, endIt );
           
            delete tempPolyGfxSplit;

            mc2dbg8 << "Result idx from Kolesnikov: ";
             
            /* 
             * Map resulting temp indeces to newStartLevel indeces. Also, 
             * avoid adding split points twice. 
             *
             */
            for( int l = 0; l < (int)( splitFilterIdxs.size() ); l++ ) {
               if( outIndeces.back() != splitFilterIdxs[ l ] + startSplitIdx) {
                  outIndeces.push_back( splitFilterIdxs[ l ] + startSplitIdx );
               }
               mc2dbg8 << splitFilterIdxs[ l ] + startSplitIdx << " ";
            }
            mc2dbg8 << endl;

            mc2dbg8 << info << "Kolesnikov ends" << endl;
         } // end loop of split points
         mc2dbg8 << "Ends filtering using split points from previous level "                    << endl;
         
         /**
          * Put the out indeces in paths so that we can use it
          * for backtracking when looking for the correct lsb's
          * later on.
          */
         mc2dbg8 << "Indeces: ";
         for( int l = 0; l < (int)( outIndeces.size() ); l++ ) {
            mc2dbg8 << outIndeces[ l ] << " ";
            paths[ i ][ l ] = outIndeces[ l ];
         }
         mc2dbg8 << endl;
      } // End creating level i
   } // End higher level filtering
   
   /**
    * Fill outGfx with correct lat and lon values, correct is lat 
    * and lon that have been anded with some nice number to cut of
    * the lbs's.
    */
   // We have perhaps added some coordinates to outGfx poly p before!
   const uint32 nbrCoordsAlreadyAdded = outGfx->getNbrCoordinates(p);
   int startAt = 0;
   if ( nbrCoordsAlreadyAdded > 0 ) {
      startAt = 1;
   }
   for( int j = startAt; j < filtSize; j++ ) {
      uint32 c = ( (j + opfBegin) % nbrCoordsInGfxPoly );
      outGfx->addCoordinate(
            gfx->getLat( p, c ) & coordBlankMask,
            gfx->getLon( p, c ) & coordBlankMask );
   }
   if ( dumpFilterInfo ) {
      cout << " outGfx before=" << nbrCoordsAlreadyAdded << " now="
           << outGfx->getNbrCoordinates(p) << endl;
      dumpPaths( paths, filterMaxDists.size() + 1, filtSize );
   }

   /**
    * Set the lsb's in outGfx to correct values, the correct values
    * will be found in paths using simple backtracking.
    * ( index in each level (i) refers to to the level before (i-1) until
    *   i = 1 where the true coord idxs are stored )
    */
   for( int i = 1; i < (int)filterMaxDists.size() + 1; i++ ) {
      if ( dumpFilterInfo ) {
         cout << "level " << i << ": ";
      }
      int l = 0;
      while( l < filtSize && ( paths[ i ][ l ] != -1 ) ) {
         int index = paths[ i ][ l ];
         // Lower levels i are filtered using level i-1
         if( i <= newStartLevel) {
            for(int k = i - 1; k > 0; k-- ) {
               index = paths[ k ][ index ];
            }
         } else { // Higher levels are filtered using newStartLevel
            for(int k = newStartLevel; k > 0; k-- ) {
               index = paths[ k ][ index ];
            }
         }
         // pay attention to if filtering was done forwards or backwards
         if ( filterForwards ) {
            // pay attention to if this is not the first part of outGfx
            index = (nbrCoordsAlreadyAdded - startAt) + index;
         } else {
            // index counted from the end
            index = filtSize - 1 - index;
            // pay attention to if this is not the first part of outGfx
            index = (nbrCoordsAlreadyAdded - startAt) + index;
         }
         outGfx->setCoordinate( p, index,
            ( outGfx->getLat( p, index ) & coordBlankMask ) + i / setLevelMask,
            ( outGfx->getLon( p, index ) & coordBlankMask ) + i % setLevelMask );
         // if index is 0 and nbrCoordsAlreadyAdded > 0: overwrites the last
         // coord already added to outGfx poly p = ok since they are the one
         // and same break point
         if ( dumpFilterInfo ) {
            cout << index << " ";
         }
         l++;
      }
      if ( dumpFilterInfo ) {
         cout << endl;
      }
   }

   // Debug printing.
   DEBUG4(
   for( int j = 0; j < filtSize; j++ ) {
      mc2dbg4 << "Lat " << 
            (gfx->getLat( p, j ) & coordBlankMask) <<
            " Lon " <<
            (gfx->getLon( p, j ) & coordBlankMask) << endl;
   });
   
   // Clean up after ourselfs.
   for ( int i = 0; i < (int)filterMaxDists.size(); i++ ) {
      delete[] paths[ i ];
   }
   delete[] paths;

   // Print our country border part to file (= the part here added to outGfx)
   if ( countryBorder && breakPoints ) {
      ofstream outfile("countryBorders.txt", ios::app);

      int startAt = 0;
      if (nbrCoordsAlreadyAdded > 0 ) {
         startAt = nbrCoordsAlreadyAdded -1;
      }
      mc2dbg4 << "Part of country border, print to file, " << filtSize 
              << " coords, " << startAt << "->" << (startAt + filtSize -1) 
              << endl;

      printCountryBorderToFile( 
            firstBreakPoint, lastBreakPoint, onTheWayPoint,
            countryMapName, outGfx, p, startAt, filtSize );
   }
}

inline void
OldMapFilter::dumpPaths( int** paths, int rows, int cols ) const
{
   cout << " paths = " << endl;
   for( int r = 0; r < rows; r++ ) {
      for( int c = 0; c < cols; c++ ) {
         cout << paths[ r ][ c ] << "\t";
      }
      cout << endl;
   }
}

inline void
OldMapFilter::initMapGfxDistances()
{
   // filterDist = 1, MAX( (int)(filterDist * 2.5), filterDist + 1 )
   //   1 2 5 12 30 75 187 467 1167 2917 7292 18230 45575 113937 284842
   //
   // filterDist = int( pow(level, 2) * 0.7 * pow(1.6, 0.92 * level) )
   //   1 6 23 63 152 337 707 1424 2777 5284 9853 18069 32678 58400 103308
   //
   // filterDist = int( pow(level, 2) * 0.8 * pow(1.6, 0.89 * level) )
   //   1 7 25 68 161 354 732 1454 2796 5245 9642 17436 31091 54787 95559
   
   int filterDist;
   for( int level = 1; level < 16; level++ ) {
      filterDist = int( pow(double(level), 2) * 0.8 * pow(1.6, 0.89 * level) );
      m_mapGfxFilterDistances.push_back( filterDist );
   }
   m_mapGfxDistDefined = true;
}

inline void
OldMapFilter::initMapGfxHighLevelDistances()
{
   
   // When computing the higher filtering levels a new filtering
   // algorithm is used. These distances are tweaked so that the
   // resulting number of coordinates is as close as possible as 
   // the number of coordinates resulting from the original 
   // algorithm.
   int filterDist;
   for( int level = 1; level < 16; level++ ) {
      filterDist = int( pow(double(level), 2) * 0.8 * pow(1.6, 0.76 * level) );
      if( level == 15 ) {
         m_mapGfxFilterHighLevelDistances.push_back( 36200 );
      } else if( level == 14) {
         m_mapGfxFilterHighLevelDistances.push_back( 22000 );
      } else if( level == 10) {
         m_mapGfxFilterHighLevelDistances.push_back( 2600 );
      } else {
         m_mapGfxFilterHighLevelDistances.push_back( filterDist );
      }
   }
}

inline void
OldMapFilter::initAreaItemDistances( int minAreaFilterDistance )
{
   // filterDist = 1, MAX( (int)(filterDist * 2.5), filterDist + 1 )
   //                 MAX( filterDist, minAreaFilterDistance * level )
   //                 minAreaFilterDistance = 3
   //   3 6 9 12 30 75 187 467 1167 2917 7292 18230 45575 113937 284842
   //
   // filterDist = int( pow(level, 2) * 0.8 * pow(1.6, 0.89 * level) )
   //   1 7 25 68 161 354 732 1454 2796 5245 9642 17436 31091 54787 95559
   
   int filterDist;
   for( int level = 1; level < 16; level++ ) {
      filterDist = int( pow(double(level), 2) * 0.8 * pow(1.6, 0.89 * level) );
      if ( minAreaFilterDistance > -1 ) {
         filterDist = MAX( filterDist, minAreaFilterDistance*level );
      }
      m_areaItemFilterDistances.push_back( filterDist );
   }
   m_areaItemDistDefined = true;
}

inline void
OldMapFilter::initLineItemDistances( int minLineFilterDistance )
{
   // filterDist = 1, MAX( (int)(filterDist * 2.0), filterDist + 1 )
   //                 MAX( filterDist, minLineFilterDistance * level )
   //                 minLineFilterDistance 1 meter, co-map: 50 meters
   //   1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384
   //   50 100 150 200 250 300 350 400 450 512 1024 2048 4096 8192 16384
   //
   // filterDist = int( pow(level, 2) * 0.8 * pow(1.6, 0.89 * level) )
   //   1 7 25 68 161 354 732 1454 2796 5245 9642 17436 31091 54787 95559
   
   int filterDist;
   for( int level = 1; level < 16; level++ ) {
      filterDist = int( pow(double(level), 2) * 0.8 * pow(1.6, 0.89 * level) );
      if ( minLineFilterDistance > -1 ) {
         filterDist = MAX( filterDist, minLineFilterDistance*level );
      }
      m_lineItemFilterDistances.push_back( filterDist );
   }
   m_lineItemDistDefined = true;
}

inline int
OldMapFilter::getFiltDistanceForMapGfx( uint32 level )
{
   if ( level > 15 ) {
      // invalid level
      return 0;
   }

   // make sure that the vector with map gfx data distances
   // m_mapGfxFilterDistances is initialised
   if ( ! m_mapGfxDistDefined ) {
      initMapGfxDistances();
   }

   // filt level 0 is no filtering,
   if ( level <= 0 ) {
      return 0;
   }
 
   // filt level 1 is first filtering, i.e. index 0 in the vector
   uint32 vectorIdx = level - 1;
   return ( m_mapGfxFilterDistances[ vectorIdx ] );
}

inline int
OldMapFilter::getFiltHighLevelDistanceForMapGfx( uint32 level )
{
   if ( level > 15 ) {
      // invalid level
      return 0;
   }

   // make sure that the vector with map gfx data distances
   // m_mapGfxFilterDistances is initialised
   if ( ! m_mapGfxHighLevelDistDefined ) {
      initMapGfxHighLevelDistances();
   }

   // filt level 0 is no filtering,
   if ( level <= 0 ) {
      return 0;
   }

   // filt level 1 is first filtering, i.e. index 0 in the vector
   uint32 vectorIdx = level - 1;
   return ( m_mapGfxFilterHighLevelDistances.at(vectorIdx) );
}


inline int
OldMapFilter::getFiltDistanceForAreaItem(
               uint32 level, int minAreaFilterDistance )
{
   if ( level > 15 ) {
      // invalid level
      return 0;
   }

   // make sure that the vector with area item distances
   // m_areaItemFilterDistances is initialised
   if ( ! m_areaItemDistDefined ) {
      initAreaItemDistances( minAreaFilterDistance );
   }

   // filt level 0 is no filtering,
   if ( level <= 0 ) {
      return 0;
   }
   
   // filt level 1 is first filtering, i.e. index 0 in the vector
   uint32 vectorIdx = level - 1;
   return ( m_areaItemFilterDistances[ vectorIdx ] );
}

inline int
OldMapFilter::getFiltDistanceForLineItem(
               uint32 level, int minLineFilterDistance )
{
   if ( level > 15 ) {
      // invalid level
      return 0;
   }

   // make sure that the vector with line item distances
   // m_lineItemFilterDistances is initialised
   if ( ! m_lineItemDistDefined ) {
      initLineItemDistances( minLineFilterDistance );
   }

   // filt level 0 is no filtering,
   if ( level <= 0 ) {
      return 0;
   }
   
   // filt level 1 is first filtering, i.e. index 0 in the vector
   uint32 vectorIdx = level - 1;
   return ( m_lineItemFilterDistances[ vectorIdx ] );
}

#endif

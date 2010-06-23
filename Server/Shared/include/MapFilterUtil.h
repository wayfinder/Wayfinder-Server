/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPFILTERUTIL_H
#define MAPFILTERUTIL_H

#include "config.h"

#include "MC2Coordinate.h"
#include "MC2String.h"

#include <math.h>
#include <vector>

/**
 * Util methods for filtering all polygons in a map using given
 * filteringlevels.
 * Why is all functionality implemented in .h-file?
 */
class MapFilterUtil {

   public:

      /// Needs to init the bools, really.
      MapFilterUtil() : m_mapGfxDistDefined( false ),
                        m_areaItemDistDefined( false ),
                        m_lineItemDistDefined ( false ) {
      }

      /// Get map gfx data filter distance for a certain filter level (1-15).
      inline int getFiltDistanceForMapGfx( uint32 level );

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
       *    Get an MC2Coordinate from a string '(' <LAT> ',' <LON> ')'.
       *
       *    @param   s     The string with MC2Coordinate string.
       *    @param   coord The coordinat extracted from the string.
       *    @return  True if a coordinate was extracted, false if not.
       */
      static bool coordFromString(const MC2String& s, MC2Coordinate& coord);

      
   private:

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

         /// Tells if the vector with map gfx data filter dists is initialised.
         bool m_mapGfxDistDefined;

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
       *    Check that the vector with filter distances includes valid
       *    and reasonable distance values. There must e.g. not be more
       *    than 15 distances.
       */
      bool validFilterLevelDistances( vector<int> filterMaxDists );

};

// (((=====================================================================
//                                      Implementation of inlined methods =



inline void
MapFilterUtil::initMapGfxDistances()
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
MapFilterUtil::initAreaItemDistances( int minAreaFilterDistance )
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
MapFilterUtil::initLineItemDistances( int minLineFilterDistance )
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
MapFilterUtil::getFiltDistanceForMapGfx( uint32 level )
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
MapFilterUtil::getFiltDistanceForAreaItem(
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
MapFilterUtil::getFiltDistanceForLineItem(
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

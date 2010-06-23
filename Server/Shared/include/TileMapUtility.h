/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPUTILITY_H
#define TILEMAPUTILITY_H

#include "config.h"

#include "DrawSettings.h"

// For POINT!
#include "GfxUtility.h"

// Forward declaration
class MapSettings;
class Item;
class StreetItem;
class GenericMap;
class MC2BoundingBox;
class FilterSettings;

// Forward declaration
class GfxFeatureMap;
class GfxFeature;

class GfxBBox;


/**
 *    Utility methods for deciding when to include GfxFeatures
 *    into GfxMaps that should be used for extracting TileMaps.
 *    
 *    FIXME: Lots of code is copied from MapUtility.
 * 
 */
class TileMapUtility {
   public:
      static int getScaleLevel(const MC2BoundingBox* bbox, 
                               int widthPixel, 
                               int heightPixel);
      static int getScaleLevel(const MC2BoundingBox* bbox, 
                               int widthPixel, 
                               int heightPixel,
                               uint32 factor);

      /**
       *    Find out on wich scalelevel a certain item should be added.
       *    The caller is responsible for that the indata is valid (e.g.
       *    item != NULL and item->getGfxData() != NULL).
       *    
       *    Note that StreetItems should be checked with 
       *    toDrawStreetPolygon for each polygon, since each polygon may
       *    represent a different roadclass.
       *    
       *    TODO: The countryMap parameter should be removed and instead
       *          the default behaviour should be to use streets instead
       *          of streetsegments. 
       *    
       *    @param item The item to check.
       *    @param widthPixel The width in pixels.
       *    @param heightPixel The height in pixels.
       *    
       *    @param countryMap Whether the map is a country map or not.
       *                      Note that when this parameter is set to
       *                      true, useStreets are automatically ignored.
       *                      
       *    @param useStreets Whether to draw streets instead of 
       *                      streetsegments where possible.
       *                      Note that when this this flag is set, the
       *                      scalelevel for each streetpolygon must be
       *                      checked with toDrawStreetPolygon() method.
       *                      Note that when the countryMap flag is
       *                      set to true, then this parameter is 
       *                      automatically ignored.
       *                      
       *    @param theMap     The map that the item belongs to.
       *                      This parameter only needs to be supplied
       *                      when the useStreets flag is set.
       *    @param itemCoord  The coordinate in case of the
       *                      item being a poi. Only needed to be supplied
       *                      when the item is a poi.
       *    @param alwaysIncludedFromCountryMap Optional parameter. Only
       *                                        valid if countryMap is
       *                                        set to true.
       *                                        If specified, only includes
       *                                        the items that always 
       *                                        should be included from the
       *                                        country map. Typically
       *                                        items that are only
       *                                        present in country map
       *                                        and removed from the
       *                                        other underlying maps, 
       *                                        such as certain water 
       *                                        items.
       *    @param   imageName   A special image that should be used
       *                         for the poi.
       *                      
       *    @return The scaleLevel that item belongs too, MAX_UINT32 is
       *            returned if feature never should be added.
       */   
      static uint32 toDrawItem( Item& item, 
                                uint32 minScale, uint32 maxScale,
                                MC2BoundingBox* viewBox,
                                int widthPixel, int heightPixel,
                                bool countryMap = false, 
                                bool useStreets = false,
                                GenericMap* theMap = NULL,
                                const MC2Coordinate&
                                itemCoord = MC2Coordinate(),
                                bool alwaysIncludedFromCountryMap = false,
                                byte extraPOIInfo = MAX_BYTE,
                                const char* imageName = NULL );

      /**
       *    Find out on wich scalelevel a street polygon should be added.
       *    The caller is responsible for that the indata is valid (e.g.
       *    street != NULL, theMap != NULL and polygon value valid ).
       *
       *    Note that StreetItems should be checked with 
       *    this method for each polygon, since each polygon may
       *    represent a different roadclass.
       *
       *    @param street     The street to check.       *    
       *    @param polygon    The index of the polygon.
       *    @return The scaleLevel that item belongs too, MAX_UINT32 is
       *            returned if feature never should be added.
       */   
      static uint32 toDrawStreetPolygon( StreetItem* street, 
                                         uint16 polygon );

      /**
       *    Get information about how an item should be filtered on a
       *    given scale-level.
       *    @param filterSettings Outparamter that is set to the type of
       *                          filtering that should be done on the
       *                          given scale-level.
       *    @param item           The item to get filtersettings for.
       *    @param scale          The maximum scale that the item should 
       *                          be presented at.
       *    @param polygon        The cardinal number of the polygon to
       *                          filter. The first polygon will be used
       *                          if not set.
       *    @return True if the filterSettings are filled in, false 
       *            otherwise.
       */
      static bool getFilterSettings(FilterSettings* filterSettings,
                                    Item* item, int32 scale, 
                                    uint16 polygon = 0);


   private:
      
      /**
       * Adds a filter factor to the meter value supplied as parameter. This 
       * makes it easier to use the same filter factor for many scale levels.
       * @param meterValue The value to multiply by the filter factor.
       * @return The result for multiplying the filter factor.
       */
      static uint32 filterFactor(uint32 meterValue);


      struct scalelevel_t {
         const char* name;
         // Scale in meters/pixel
         uint32 maxScale;
         uint32 minAreaFactor;
      };

      static const scalelevel_t m_scaleLevels[];
      static const uint32 m_nbrScales;
};
      
#endif


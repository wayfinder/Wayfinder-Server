/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPUTILITY_H
#define MAPUTILITY_H

#include "config.h"


#include "DrawSettings.h"

// For POINT!
#include "GfxUtility.h"


class MapSettings;
class Item;
class StreetItem;
class GenericMap;
class MC2BoundingBox;
class FilterSettings;
class GfxFeatureMap;
class GfxFeature;
class POIImageIdentificationTable;

/**
 * 
 */
class MapUtility {
   public:
      #define CONTINENT_LEVEL       0
      #define COUNTRY_LEVEL         1
      #define COUNTY_LEVEL          2
      #define SMALL_COUNTY_LEVEL    3
      #define MUNICIPAL_LEVEL       4
      #define CITY_LEVEL            5
      #define TRAFFIC_INFO_LEVEL CITY_LEVEL
      #define SMALL_CITY_LEVEL      6
      #define DISTRICT_LEVEL        7
      #define BLOCK_LEVEL           8
      #define PART_OF_BLOCK_LEVEL   9
      #define DETAILED_STREET_LEVEL 10

      #define INVALID_SCALE_LEVEL   MAX_UINT32

   static int getScaleLevel( int64 area,
                             int widthPixel, 
                             int heightPixel, 
                             uint32 factor );

   static int getScaleLevel( const MC2BoundingBox& bbox, 
                             int widthPixel, 
                             int heightPixel );
   static int getScaleLevel( const MC2BoundingBox& bbox, 
                             int widthPixel, 
                             int heightPixel,
                             uint32 factor );

      static const char* getScaleLevelName(uint32 scaleLevel);

      /**
       *    Get a value of when to draw the given polygon in the given 
       *    feature. The values that are returned from this method should
       *    be used to compare the polygons in all the features to find
       *    out in what order the polygons should be draw. A typical usage
       *    is to loop over all the features and polygons, get the 
       *    draw-order, sort them according to the draw-order and then
       *    draw the polygons in that order.
       *    @param feature The feature to get the order-value for.
       *    @param poly    The number of the polygon in feature to check.
       *    @param border  If it is the border of the street or the middle 
       *                   of the street.
       */
      static int getDrawOrder(const GfxFeature* feature, uint32 poly,
                              bool border = true);


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
       *    @param lat        The latitude coordinate in case of the
       *                      item being a poi. Only needed to be supplied
       *                      when the item is a poi.
       *    @param lon        The longitude coordinate in case of the
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
       *                      
       *    @return The scaleLevel that item belongs too, MAX_UINT32 is
       *            returned if feature never should be added.
       */   
      static uint32 toDrawItem( Item* item, MC2BoundingBox* bbox,
                                int widthPixel, int heightPixel,
                                bool countryMap = false, 
                                bool useStreets = false,
                                GenericMap* theMap = NULL,
                                int32 lat = MAX_INT32, 
                                int32 lon = MAX_INT32,
                                bool alwaysIncludedFromCountryMap = false,
                                byte extraPOIInfo = MAX_BYTE );

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
      
      /**
       *    Get the draw-settings for a given feature. 
       *     @param type     The type of the feature.
       *    @param scale    The scale-level for the map that will be drawn.
       *    @param settings The object that will be filled with data.
       *    @param feature  The feature to make draw-settings for.
       *    @param border   True if the border of the street should be
       *                    drawn, false if the middle of the street should
       *                    be draw. This parameter is only used if the
       *                    type is STREET_*.
       *    @param poly     The polygon that should be drawn. This might
       *                    contain additional information such as
       *                    roundabout or ramp.
       *    @param featureMap The GfxFeatureMap that is drawn. This might
       *                      contain additional infomation such as
       *                      transportationtype and U-turn.
       *    @param poiStatus  Optional parameter, set if
       *                      the the feature POI/WCPOI represent 
       *                      several other POI/WCPOI:s.
       *    @return True if the settings are set, false otherwise. If false
       *            is returned, the feature should not be drawn!
       */
      static bool getDrawSettings(
          GfxFeature::gfxFeatureType type,
          int scale, 
          MapSettings* mapSettings,
          DrawSettings* settings, 
          const GfxFeature* feature,
          bool border = false,
          GfxPolygon* poly = NULL,
          const GfxFeatureMap* featureMap = NULL,
          DrawSettings::poiStatus_t poiStatus = DrawSettings::singlePOI,
          const POIImageIdentificationTable* imageTable = NULL );


      /**
       *    Get the filter level of the country overview map given
       *    a certain scaleLevel. If no filtering should be used, 
       *    then a value equal to or higher than 
       *    CountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX is returned.
       *    @see CountryOverviewMap
       *    @param scaleLevel The scalelevel for the map.
       *    @return The filter level for the CountryOverviewMap.
       */
      static byte getCountryFilterLevel(int filtScaleLevel);

      
      /**
       * Calculates the DrawItemParameters.
       *
       * @param width The width of the image.
       * @param height The height of the image.
       * @param bbox The boundingbox for the map.
       * @param xFactor Set to the xfactor.
       * @param yFactor Set to the yFactor.
       */
      static void makeDrawItemParameters( uint16 width, uint16 height,
                                          MC2BoundingBox& bbox,
                                          float64& xFactor, 
                                          float64& yFactor );
      
      /**
       * Get the type of GfxFeature that the specified item should be
       * represented as.
       * @param item The item.
       * @param filterSettings Optional parameter, the filter settings. 
       * @return NBR_GFXFEATURES if no suitable gfxFeatureType was found
       *         for the item, otherwise the gfxFeatureType is returned.
       */
      static GfxFeature::gfxFeatureType getFeatureTypeForItem( 
                                   const Item* item, 
                                   const FilterSettings* filterSettings );

      
      /**
       * Get the longitude coordinate from image coordinate.
       * @param x The x-coordinate in the image.
       * @param width The width of the image.
       * @return The longitude value for the given image coordinate.
       */
      //static int32 getLon( int x, MC2BoundingBox& bbox, 
      //                     float64 xFactor, uint16 width );

      /**
       * Get how much a difference in image coordinates is in 
       * longitude coordinates.
       * @param x The difference in x-coordinates in the image.
       * @param xFactor The xFactor.
       * @return The difference in longitude coordinates.
       */
      //static int32 getLonDiff( int x, float64 xFactor );

       /**
       * Get the latitude coordinate from image coordinate.
       * @param y The y-coordinate in the image.
       * @param width The width of the image.
       * @return The latitude value for the given image coordinate.
       */
      //static int32 getLat( int y, MC2BoundingBox& bbox, 
      //                    float64 yFactor, uint16 height );
      
      /**
       * Get how much a difference in image coordinates is in 
       * latitude coordinates.
       * @param y The difference in y-coordinates in the image.
       * @param yFactor The yFactor.
       * @return The difference in latitude coordinates.
       */
// static int32 getLatDiff( int y, float64 yFactor );
 
   /** @return true if the feature type is a large area and should have a
    *          centroid in the gfx feature data
    */
   static bool isLargeAreaType( GfxFeature::gfxFeatureType type );

   private:
      struct scalelevel_t {
         const char* name;
         // Scale in meters/pixel
         uint32 maxScale;
         uint32 minAreaFactor;
      };

      static const scalelevel_t m_scaleLevels[];
      static const uint32 m_nbrScales;

      static bool getDrawSettingsSSI(GfxFeature::gfxFeatureType type,
                                     int scale,
                                     DrawSettings* settings,
                                     bool border,
                                     GfxRoadPolygon* poly);

};

#endif


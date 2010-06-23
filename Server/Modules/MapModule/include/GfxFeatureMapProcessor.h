/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATUREMAPPROCESSOR_H
#define GFXFEATUREMAPPROCESSOR_H

#include "config.h"

#include "MC2BoundingBox.h"
#include "GfxFeature.h"
#include "GfxFeatureMapProcBase.h"

#include <map>

class CountryOverviewMap;
class FilterSettings;
class GenericMap;
class GfxData;
class GfxFeature;
class GfxFeatureMap;
class GfxFeatureMapReplyPacket;
class GfxFeatureMapRequestPacket;
class GfxFeatureMapCityCentreReplyPacket;
class GfxFeatureMapCityCentreRequestPacket;
class Item;
class MapHandler;
class MapSetting;
class MapSettings;
class Stack;

/**
  *   Class that generates gfx-feature maps.
  *
  */
class GfxFeatureMapProcessor : public GfxFeatureMapProcBase {
   public :
      /**
        *   Creates a GfxFeatureMapProcessor.
        *   @param   mh The map.
        *   @param   theOneWithTheCache Other GfxFeatureMapProcBase
        *            with the cached cc display classes.
        */   
      GfxFeatureMapProcessor( GenericMap& m,
                              GfxFeatureMapProcBase* theOneWithTheCache );
                              
      
      /**
        *   Deletes the GfxFeatureMapProcessor and releases the allocated 
        *   memory.
        */
      virtual ~GfxFeatureMapProcessor();

      /**
        *   Generates a GfxFeature-map-
        *   @param   p  The requestpacket.
        */
      GfxFeatureMapReplyPacket* generateGfxFeatureMap(
                                    const GfxFeatureMapRequestPacket* p);

   private:
      
      /**
       *  The maximum (most detailed) scale-level of the map.
       */
      int m_maxScaleLevel;

      /**
       *  The minimum (least detailed) scale-level of the map.
       */
      int m_minScaleLevel;

      /**
       *  The scale-level of the map used when filtering features.
       */
      int m_filtScaleLevel;

      /**
       *  Whether we allow more than one coordinate per pixel on the
       *  screen or not.
       */
      bool m_maxOneCoordPerPixel;
      
      /**
        *   Initializes the members from the request packet.
        *   @param   p  The requestpacket.
        */
      void init(const GfxFeatureMapRequestPacket* p);

      /**
       * Creates an country feature map
       * @see createGfxFeatureMap
       */
      void createCountryFeatureMap( GfxFeatureMap* gfxFeatureMap,
                                    const GfxFeatureMapRequestPacket* p );

      /**
       * Creates an underview feature map
       * @see createGfxFeatureMap
       */
      void createUnderviewFeatureMap( GfxFeatureMap* gfxFeatureMap,
                                      const GfxFeatureMapRequestPacket* p );

      /**
        *   Extract an ordinary gfxFeature-map
        *   @param   gfxFeatureMap Preallocated feature-map which will be 
        *                  filled with features.
        *   @param   p  The requestpacket. This contains data about e.g.
                        what language to use.
        */
      void createGfxFeatureMap(GfxFeatureMap* gfxFeatureMap,
                               const GfxFeatureMapRequestPacket* p);
      
      /**
        *   Extract a feature-map over a route.
        *   @param   p        GfxFeatureMapRequestPacket which contains
        *                     information about the nodes that the route
        *                     consists of.
        *   @param   featureMap Preallocated feature-map which will be 
        *                       filled with features.
        */
      void createRouteGfxFeatureMap(const GfxFeatureMapRequestPacket* p,
                                    GfxFeatureMap* featureMap);
      
      /**
        *   Extract a feature-map containing point of interests.
        *   @param   p           The requestpacket. This contains data
        *                        about e.g. what language to use.
        *   @param   featureMap  Preallocated feature-map which will be 
        *                        filled with features.
        */
      void createPOIGfxFeatureMap(const GfxFeatureMapRequestPacket* p,
                                  GfxFeatureMap* featureMap);

      void createCityCentreGfxFeatureMap(
                            const GfxFeatureMapRequestPacket*,
                            GfxFeatureMap* featureMap,
                            MC2BoundingBox& bbox );
      
      /**
       *    Methods for creating country LAND and BORDER features.
       */
      //@{
      
         /**
          *    Create a LAND-feature without any polygons.
          *    @param cmap       The country map.
          *    @param p          The request packet.
          */
         GfxFeature* createLandFeature(CountryOverviewMap* cmap,
                                       const GfxFeatureMapRequestPacket* p );

         /**
          *    Decide how many polygons from the country map gfx data 
          *    (country polygon) that should be included in the LAND feature.
          */
         uint32 getNumberCountryPolysToSend( CountryOverviewMap* comap );
         
         /**
          *    Add a polygon to a LAND-feature.
          *    @param cmap       The country map.
          *    @param feature    The LAND-feature.
          *    @param polynumber The polygonnumber in the gfx of the country.
          *    @return If the polygon was succesfully added.
          */
         bool addCountryPolygon( CountryOverviewMap* cmap,
                                 GfxFeature* feature, 
                                 int polynumber );
         /**
          *    Create and add country border features to the gfx feature map.
          *    Adds border features only if the map is a country map
          *    and has gfx data filtered in coordinate levels.
          */
         uint32 createAndAddBorders( GfxFeatureMap* gfxFeatureMap );

      //@}
      
      /**
        *   Create an GfxFeature from an item.
        *   @param item     The item with the gfx data that will be used
        *                     to create the polygons in the GfxFeature.
        *   @param filterSettings The filter-parameters, if to filter the
        *                         graphical representation of the item.
        *   @param t              The type of the feature, if this parameter
        *                         not is set, the type of the new feature
        *                         will be calculated from the type of the
        *                         item.
        *   @param coordinateStartingAtNode0 Whether the coordinates 
        *                                    should start at node 0 or not
        *                                    when added. Optional 
        *                                    parameter, set to true if not
        *                                    specified.
        *   @param   lat         Optional parameter. GfxData will be set
        *                        to only one coordinate, and this is
        *                        the latitude part of it. Currently only
        *                        useful when extracting point of interests.
        *   @param   lon         Optional parameter. GfxData will be set
        *                        to only one coordinate, and this is
        *                        the longitude part of it. Currently only
        *                        useful when extracting point of interests.
        *   @param lang          The preferred language when finding the 
        *                        name of the new gfx feature. 
        *                        Default invalidLanguage, then the first 
        *                        name of item is used.
        *                        
        *   @return  The GfxFeature that was created in this method.
        */
      GfxFeature* createGfxFeatureFromItem(
               Item* item, 
               const FilterSettings* filterSettings,
               GfxFeature::gfxFeatureType t = GfxFeature::NBR_GFXFEATURES,
               bool coordinateStartingAtNode0 = true,
               int32 lat = MAX_INT32,
               int32 lon = MAX_INT32,
               LangTypes::language_t lang = LangTypes::invalidLanguage,
               byte extraPOIInfo = MAX_BYTE,
               int startOffset = -1,
               int endOffset = -1,
               const char* imageName = NULL );

      
      /**
       *    Determines whether the specified feature type should be 
       *    included on the map. Note that it does not take into 
       *    consideration if a particular feature should be included,
       *    use MapUtility::toDrawItem(..) in order to determine that.
       *    (Other things are taken into consideration then, such as
       *    scalelevel and so on.)
       *    @param   type     The feature type.
       *    @return  True if the feature type should be included on the
       *             map, false otherwise.
       */
      inline bool includeFeatureType( 
                        GfxFeature::gfxFeatureType type ) const;


      /**
       *    Some methods and members to simplify the adding of 
       *    coordinates.
       *    Usage:
       *    * Two ways of adding coordinates:
       *      1) The coordinates are already clipped against the view
       *         boundingbox and we simply need to add them to GfxFeature:
       *
       *         * Add the first coordinate with 
       *           addClippedFirstCoordinate(..).
       *           
       *         * Add the rest of the coordinates with 
       *           addClippedCoordinate(..).
       *           
       *         * If the polygon is closed, call addClosedCoordinate(..).
       *    
       *      2) The coordinates are NOT clipped against the view 
       *         boundingbox and we don't intend to do that either.
       *         We then need a slightly more complicated way of adding
       *         coordinates. In order to add one coordinate we look at the
       *         previous and next coordinate as well, in order to be able
       *         to skip coordinates that will not be visible outside
       *         the view boundingbox. No clipping is performed:
       *    
       *         * Get the true (lat,lon) that should be added and call
       *           addFirstCoordinate(..). This will initiate the members.
       *           The following coordinate is also supplied to this method.
       *      
       *         * For each coordinate call addPreviousCoordinate(..). 
       *           Supply the next coordinate to add (if not adding the 
       *           last coordinate) as nextLat and nextLon. Note that the 
       *           call to addPreviousCoordinate(..) will add the 
       *           coordinate that was supplied as nextLat/nextLon through 
       *           a previous call to addFirstCoordinate(..) or 
       *           addPreviousCoordinate(..)
       *      
       *         * If the polygon is closed, call addClosedCoordinate(..).
       */
      //@{
         int32 m_prevAddedLat;
         int32 m_prevAddedLon;
         int32 m_startLat;
         int32 m_startLon;

         
         /**
          *    Method used when adding the first clipped coordinate.
          *
          *    @param   lat            The first latitude to add.
          *    @param   lon            The first longitude to add.
          *    @param   feature        The feature to add coordinate to.
          *    @param   approxNbrCoord The approximate number of
          *                            coordinates to add. Default set to
          *                            16.
          *    @param   coordinates16Bits Whether 16 bit relative or
          *                               32 bit absolute coordinates
          *                               should be used for the feature.
          *                               Default is 16 bit relative.
          */
         void addClippedFirstCoordinate(int32 lat, int32 lon,
                                        GfxFeature* feature,
                                        uint32 approxNbrCoord = 16,
                                        bool coordinates16Bits = true);

         /**
          *    Method used when adding non-first coordinates.
          * 
          *    @param   lat            The latitude to add.
          *    @param   lon            The longitude to add.
          *    @param   feature        The feature to add coordinate to.
          *    @param   approxNbrCoord The approximate number of
          *                            coordinates to add. Default set to
          *                            16.
          */
         void addClippedCoordinate(int32 lat, int32 lon,
                                   GfxFeature* feature);
         
         /**
          *    Add the first coordinate to the feature.
          *    The following coordinate is also supplied.
          *    In case there does not exist a following coordinate, 
          *    simply set nextLat = lat and nextLon = lon.
          *    nextLat and nextLon will be added the next time
          *    addPreviousCoordinate() is called.
          *    
          *    @param   lat            The first latitude to add.
          *    @param   lon            The first longitude to add.
          *    @param   nextLat        The next latitude to add.
          *    @param   nextLon        The next longitude to add.
          *    @param   feature        The feature to add coordinate to.
          *    @param   approxNbrCoord The approximate number of
          *                            coordinates to add. Default set to
          *                            16.
          *    @param   coordinates16Bits Whether 16 bit relative or
          *                               32 bit absolute coordinates
          *                               should be used for the feature.
          *                               Default is 16 bit relative.
          */
         void addFirstCoordinate(int32 lat, int32 lon, 
                                 int32 nextLat, int32 nextLon,
                                 GfxFeature* feature,
                                 uint32 approxNbrCoord = 16,
                                 bool coordinates16Bits = true);
         
         /**
          *    The latitude value of the coordinate that will be added
          *    the next time addPreviousCoordinate is called.
          */
         int32 m_currLat;
         
         /**
          *    The longitude value of the coordinate that will be added
          *    the next time addPreviousCoordinate is called.
          */
         int32 m_currLon;
         
         /**
          *    Add the previously stored coordinate to the feature.
          *    The coordinate must have been supplied by a preceding
          *    call to addFirstCoordinate() or addPreviousCoordinate()
          *    (nextLat and nextLon).
          *
          *    Supply the next coordinate to add as nextLat and nextLon.
          *    
          *    @param   nextLat  The next latitude coordinate to add 
          *                      (ie. the coordinate after the one that is
          *                      is being added in this method).
          *    @param   nextLon  The next longitude coordinate to add 
          *                      (ie. the coordinate after the one that is
          *                      is being added in this method).
          *    @param   feature  The feature to add the coordinate to.
          */
         void addPreviousCoordinate( int32 nextLat, 
                                     int32 nextLon, 
                                     GfxFeature* feature );

         /**
          *    Add the previously stored coordinate to the feature.
          *    The coordinate must have been supplied by a preceding
          *    call to addFirstCoordinate() or addPreviousCoordinate()
          *    (nextLat and nextLon).
          *
          *    Use this method to add the last coordinate to a feature
          *    (ie. no next coordinate is supplied).
          *
          *    @param   feature  The feature to add the coordinate to.
          */
         void addPreviousCoordinate( GfxFeature* feature );

         /**
          *    Add closed coordinate.
          *    @param   feature  The feature to add the coordinate to.
          */
         void addClosedCoordinate(GfxFeature* feature);
      //@}


      /**
       *    Checks if the current feature is currently included in
       *    a single pixel that already have a feature of similar type
       *    added to it.
       *    @param   feature  Feature to check.
       *    @return True if another feature is already added
       *            at the pixel.
       */
      bool manyFeaturesOnPixel( GfxFeature* feature );
     

      /**
        *   Adds the coordinates of the gfxdata to the feature. Will
        *   use the given filter-settings to filter the polygon.
        *
        *   @param feature  The feature to add the coordinates to.
        *   @param gfx      The gfxdata with the coordinates.
        *   @param settings The settings to use when filtering the polygon.
        *   @param poly     The cardinal number of the polygon of the 
        *                   gfxdata to add to the feature.
        *   @param coordinateStartingAtNode0 Whether the coordinates 
        *                                    should start at node 0 or not
        *                                    when added. Optional 
        *                                    parameter, set to true if not
        *                                    specified.
        *   @return  True if the coordinates are added, false otherwise.
        */
      bool addGfxDataToFeature(GfxFeature* feature, 
                               const GfxData* gfx,
                               const FilterSettings* settings,
                               uint32 poly,
                               bool coordinateStartingAtNode0 = true);
      
      /**
       *    Help method to addFilteredGfxDataToFeature.
       *    Adds the coordinates from the gfxdata to the point vector.
       *    @param   gfx         The gfxdata.
       *    @param   poly        The polygon index of the gfxdata.
       *    @param   polyStack   NULL if all coords should be added,
       *                         otherwise contains the coord indices to
       *                         add.
       *    @param   vertices    [OUT] The point vector to be set.
       */
      void addCoordsFromGfxDataToPointVector( 
                                    const GfxData* gfx, 
                                    uint16 poly,
                                    const Stack* polyStack,
                                    vector<POINT>& vertices );

      /**
       *    Add coordinates to a feature, by using a filter-stack.
       *    @param feature   The feature to add the coordinates to.
       *    @param gfx       The gfxdata with the coordinates.
       *    @param poly      The cardinal number of the polygon of the 
       *                     gfxdata to add to the feature.
       *    @param polyStack Stack with the indices of the coordinates
       *                     that will be added.
       *    @return  True if the coordinates are added, false otherwise.
       */
      bool addFilteredGfxDataToFeature(GfxFeature* feature, 
                                       const GfxData* gfx,
                                       uint32 poly,
                                       const Stack* polyStack,
                                       bool forward = true);

      /**
       *    Add one coordinate to the given feature. Will add it as relative
       *    coordinates in 16 bits to save space. This means that if the
       *    latDiff or lonDiff is larger than 2^16 several coordinates
       *    will be added to the feature.
       *
       *    @param feature The feature that the coordinate should be added
       *                   to.
       *    @param latDiff The latitude differance.
       *    @param lonDiff The longitude differance.
       */
      void addCoordinateToFeature(GfxFeature* feature, 
                                  int latDiff, int lonDiff);

      byte adjustPOIInfoForCountry( byte extraPOIIfo, 
                            StringTable::countryCode country,
                            ItemTypes::pointOfInterest_t poiType =
                              ItemTypes::nbr_pointOfInterest );
            
      /**
        *   The boundingbox of the gfxmap.
        */
      MC2BoundingBox* m_bbox;

      MC2BoundingBox m_coordIncludeBBox;
      
      /**
       *    Checks if the specified coordinate should be included in the 
       *    map. The coordinate may be moved closer to the map boundingbox.
       *    
       *    @param   lat   Latitude value.
       *    @param   lon   Longitude value.
       *    @return  True if the coordinate should be included in the map,
       *             false otherwise.
       */
      bool includeCoordinate(int32& lat, int32& lon);

      /**
       *    Cohen-Sutherland outcodes for the previous, current and
       *    next coordinate to add.
       *
       *    @see MC2BoundingBox
       */
      byte m_prevCSOutcode;
      byte m_currCSOutcode;
      byte m_nextCSOutcode;
      

      /**
       *    Previous x pixel coordinate.
       */
      int m_prevPixelX;
      
      
      /**
       *    Previous y pixel coordinate.
       */
      int m_prevPixelY;
      
      
      /**
       * Width and height of the screen in pixels.
       */
      uint16 m_screenX, m_screenY;

      
      /**
       * X scale factor. Used when drawing on to the screen.
       * @see MapUtility::getLinearX.
       */
      float64 m_xScaleFactor;

      
      /**
       * Y scale factor. Used when drawing on to the screen.
       * @see MapUtility::getLinearY.
       */
      float64 m_yScaleFactor;

      
      /**
        *   Maximum lateral filtering distance for this slimmmap.
        */
      uint32 m_maxLatFiltDist;

      /**
       * The number of request packets (number of maps)
       * that this slimmap request involves.
       */
      uint16 m_nbrReqPackets;

      /**
       * The type of clipping algorithm used.
       * If true a slower more general algorithm which handles 
       * concave polygons is used.
       * Otherwise a faster (Sutherland-Hodgeman) clipper is used.
       */
      bool m_concaveGfxClipper;

      /**
       * The level of filtering for the country map.
       * The highest level of filtering is 0.
       */
      byte m_filterLevel;
       
      /**
       *    The mapsettings for this map.
       */
      MapSettings* m_mapSettings;

      /** Centroid for overview map, used with Land features.
       * This is used because land features have large amounts of
       * cordinates and are expensive to calculate the centroid for.
       * The centroid is used as the last polygon for land features and
       * sent back to server and then sent to the gfx module.
       * The land polygon sent to the gfx module might be clipped; this will
       * affected the centroid calculation in the gfx module in a bad way, thus
       * the calculation must be done here in map module.
       */
      MC2Coordinate m_landCentroid;
      
      /**
       *    Table containing the item types that are added for each pixel.
       *    Key: ypixel*m_screenX + xpixel.
       *    Value: The GfxFeature types currently added at that pixel.
       */
      multimap<uint32, uint32> m_typePerPixel;
};

#endif


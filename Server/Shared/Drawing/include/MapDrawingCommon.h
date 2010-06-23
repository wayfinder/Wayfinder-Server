/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_DRAWING_COMMON_HH
#define MAP_DRAWING_COMMON_HH

#include "config.h"

#include "GfxFeature.h"
#include "DrawSettings.h"

class MC2Point;
class GfxData;
class GfxFeature;
class GfxFeatureMap;
class DrawingProjection;
class MapSetting;
class MapSettings;
class Mc2BoundingBox;


namespace MapDrawingCommon {

class ObjectBoxes;

struct featurenotice_t {
   int m_drawOrder;
   bool m_border;
   const GfxFeature* m_feature;
   uint32 m_featureIndex;
   uint32 m_polygonIndex;
   const MapSetting* m_setting;         
   DrawSettings::poiStatus_t m_poiStatus;
   bool m_visible;
};

struct LessFeatureNoticeOrder:
      public binary_function<const featurenotice_t&, 
                             const featurenotice_t&, bool> {
   bool operator()(const featurenotice_t& x, 
                   const featurenotice_t& y) {
      return (x.m_drawOrder < y.m_drawOrder);
   }
};

struct LessNoticeFeatureIdOrder:
      public binary_function<const featurenotice_t&, const featurenotice_t&, bool> {
   bool operator()(const featurenotice_t& x, const featurenotice_t& y) {
      return (x.m_featureIndex < y.m_featureIndex);
   }
};


/// @return true if the feature is some kind of a street feature
bool isStreetFeature( const GfxFeature& feature );

/**
 * Determines centroid of feature polygon.
 * @param feature Current feature to be drawn.
 * @param featGfx gfx data for feature.
 * @param projection Current projection.
 * @param toAdd Will return whether or not this feature should be added to the
 *              draw list.
 * @return Center of the featGfx polygon.
 */
MC2Point determineCentroid( const GfxFeature& feature,
                            const GfxData& featGfx,
                            const DrawingProjection& projection,
                            bool& toAdd );
/**
 *    Find out if a street feature is a candidate for 
 *    1: merging in order to draw more names in the image, or
 *    2: drawing the name of the feature.
 *    @param   type        The feature type.
 *    @param   scaleLevel  The scaleLevel of the image.
 *    @param   smallImage  If the image is small (width <= 250 pixels)
 *    @param   merge       Set to true if test is for merging,
 *                         set to false if test is for drawing.
 */
bool mergeAndDrawStreetNames( GfxFeature::gfxFeatureType type,
                              uint32 scaleLevel,
                              bool smallImage, bool merge = false);

/**
 * Adds notices to notices vector in draworder.
 * @param featureMap The GfxFeatureMap to sort.
 * @param mapSettings The MapSettings if any, may be NULL.
 * @param notices Set to contain the sorted notices.
 * @param singleDraw If the notices should contain duplicates
 *        for polygons that should be drawn twice, or if
 *        poi:s should be removed if they are located on the same
 *        spot.
 * @param checkOverlappingObjects   If set to true, overlapping 
 *                                  objects  will not be allowed.
 *                              Make sure the objectBBoxes parameter
 *                              is set in case this param is true.
 * @param includeHiddenFeatures If set to true, hidden features
 *                              will be included.
 * @param poiFiltering  Optional parameter. Set to true the server
 *                      should make sure that not too many poi:s
 *                      are added. This parameter is only valid
 *                      if checkOverlappingObjects are set to true.
 * @param objectBBoxes If specified, set to contain the boundingboxes 
 *                  in mc2 coordinates of all added objects that 
 *                  must not be overlapped
 *                  (POI/WCPOI/TRAFFIC_INFO:s etc).
 *                  Note that this parameter must be specified
 *                  if checkOverlappingObjects is set to true.
 */
void sortAndCreateFeatureNotices( GfxFeatureMap* featureMap,
                                  MapSettings* mapSettings,
                                  vector<featurenotice_t> &notices,
                                  bool singleDraw,
                                  bool checkOverlappingObjects,
                                  bool includeHiddenFeatures,
                                  bool poiFiltering = true,
                                  ObjectBoxes* objectBBoxes = NULL);
         
/**
 *    Find out if a name is a roadSign name: The criterias are:
 *    1: max 3 chars in the name
 *    2: only digits in the name, or the name starts with E, M or A
 *    
 *    A name beginning with E will result in eroadsign with
 *    green colour, the rest will be roadsign with blue colour.
 */
bool roadSignName( const char* name );

} // MapDrawingCommon

#endif // MAP_DRAWING_COMMON_HH

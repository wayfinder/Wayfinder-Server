/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_TEXT_PLACEMENT_H
#define MAP_TEXT_PLACEMENT_H

#include "config.h"
#include <boost/shared_ptr.hpp>

#include "GfxDataTypes.h"

#include <vector>

class GfxFeature;
class MapSetting;
class GfxData;
class GfxDataFull;
class ImageDraw;
class MC2BoundingBox;
class DrawingProjection;
class GfxFeatureMap;
class MapSettings;

namespace MapDrawingCommon {
class ObjectBoxes;
}

namespace MapTextPlacement {

/// Text importance levels determines the sort order.
/// Any change in order of these will affect the collision testing
/// order.
enum textImportanceLevel {
   /// Land
   LAND = 0,
   /// Built up area
   BUILTUP_AREA,
   /// City centre
   CITY_CENTRE,
   /// Builtup area, square symbol
   BUILTUP_AREA_SQUARE,
   /// Builtup area, small symbol
   BUILTUP_AREA_SMALL,
   /// Individual building
   INDIVIDUALBUILDING,
   /// Building
   BUILDING,
   /// Main class road
   STREET_MAIN,
   /// First class road
   STREET_FIRST,
   /// Second class road
   STREET_SECOND,
   /// Third class road
   STREET_THIRD,
   /// Park
   PARK,
   /// Island
   ISLAND,
   /// Water
   WATER,
   /// Water
   WATER_LINE,
   /// Fourth class road
   STREET_FOURTH,
   /// Forest
   FOREST,
   /// Pedestrian area
   PEDESTRIANAREA,
   /// Aircraft road
   AIRCRAFTROAD,
   /// Ferry
   FERRY,

   /// Other gfxFeatureTypes
   OTHER = 256
};
      
/**
 * Text sorting notice
 */
struct featuretextnotice_t {
   ~featuretextnotice_t();
   int m_scaleLevel;
   int m_textImportanceLevel;
   const GfxFeature* m_feature;
   const MapSetting* m_setting;
   float64 m_length;
   vector<GfxDataTypes::textPos> m_curvedTextOut;
   uint32 m_id;         // used when merging street features
   boost::shared_ptr<GfxData> m_gfxData;  // used when merging street features
         
};

typedef std::vector<featuretextnotice_t>::const_iterator 
featureTextNoticeConstIt;

typedef std::vector<featuretextnotice_t>::iterator 
featureTextNoticeIt;

/**
 * Ititializes the notices vector and sets the textattributes in
 * the features of featureMap.
 * @param featureMap The GfxFeatureMap to set text for.
 * @param notices Set to contain the notices used to draw the text.
 * @param objectBBoxes Boundingboxes in mc2 coords 
 *                     of already added objects that must not be
 *                     overlapped with text.
 * @return true if textsetting succedded, false if not.
 */
bool initializeText( GfxFeatureMap* featureMap,
                     MapSettings* mapSettings,
                     vector<featuretextnotice_t> &notices,
                     MapDrawingCommon::ObjectBoxes& objectBBoxes,
                     ImageDraw* image,
                     vector<GfxData*>& gfxTextArray,
                     const DrawingProjection* projection );

/**
 * @param feature
 * @param sortednotices
 * @param currentNoticeIt
 * @param textGfx Returns a text
 * @param scaleLevel scale level of the drawing area.
 * @param screenX Drawing area width in pixels.
 * @param screenY Drawing area height in pixels.
 * @param bbox Bounding box of the current draw area.
 * @param tryNbr Current text placement try number on the same text.
 * @param nbrBuaTextsAdded Number of built up areas added.
 *
 */
bool getTextLocation( GfxFeature* feature, 
                      const vector<featuretextnotice_t>& sortednotices,
                      const vector<featuretextnotice_t>::const_iterator
                      currentNoticeIt,
                      GfxDataFull*& textGfx, 
                      uint32 scaleLevel,
                      uint32 screenX,
                      uint32 screenY,
                      MC2BoundingBox& bbox,
                      int& tryNbr,
                      uint32 nbrBuaTextsAdded,
                      const DrawingProjection* projection,
                      const ImageDraw* image );
      
bool getTextLocationRotated( GfxFeature* feature,
                             featuretextnotice_t& notice,
                             uint32 scaleLevel,
                             uint32 screenX,
                             uint32 screenY,
                             MC2BoundingBox& bbox,
                             ImageDraw* image,
                             MapDrawingCommon::ObjectBoxes& objectBBoxes,
                             vector<GfxData*>& gfxTextArray,
                             vector<featuretextnotice_t>& addednotices,
                             uint32& nbrRoadSigns,
                             const DrawingProjection* projection,
                             uint32 streetWidth );

void placeOtherText( MapSettings* mapSettings,
                     const vector<featuretextnotice_t> &notices,
                     const GfxFeatureMap* featureMap,
                     GfxFeature* feature,
                     const featureTextNoticeIt& f, 
                     const DrawingProjection* projection,
                     ImageDraw* image,
                     vector<GfxData*>& gfxTextArray,
                     MapDrawingCommon::ObjectBoxes& objectBBoxes,
                     vector<featuretextnotice_t>& addednotices,
                     uint32& nbrBuaTextsAdded  );

void placeStreetText( MapSettings* mapSettings,
                      const GfxFeatureMap* featureMap,
                      GfxFeature* feature,
                      featureTextNoticeIt& f, 
                      const DrawingProjection* projection,
                      ImageDraw* image,
                      vector<GfxData*>& gfxTextArray,
                      MapDrawingCommon::ObjectBoxes& objectBBoxes,
                      vector<featuretextnotice_t>& addednotices,
                      uint32& nbrRoadSigns );

/// @return True if the feature has text that must be split into two 
///         strings.
bool shouldSplitText( const GfxFeature& feature );

/// Check if this feature type uses texts also in alt language
/// @param feature The feature to chack
/// @return True if this feature should have also alt lang text.
bool useAltLangText( const GfxFeature& feature );

/// Note: currently no better place for this function.
/// @return true if screen width represents a small image.
bool isSmallImage( uint32 screenWidth );

}

#endif // MAP_TEXT_PLACEMENT_H

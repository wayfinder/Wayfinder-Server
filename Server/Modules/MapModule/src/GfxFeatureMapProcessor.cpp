/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "Stack.h"
#include "GfxFeatureMapProcessor.h"
#include "GfxData.h"
#include "PointOfInterestItem.h"

#include "MapHashTable.h"
#include "AbbreviationTable.h"
#include "MapUtility.h"
#include "StreetSegmentItem.h"
#include "MapSettings.h"
#include "GfxFeatureMapPacket.h"
#include "GfxFeatureMap.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "GfxConstants.h"
#include "CountryOverviewMap.h"
#include "FilterSettings.h"
#include "StreetItem.h"
#include "GfxFeatureMapUtility.h"
#include "CentroidCalculation.h"

#include "GfxData.h"
#include "Properties.h"

#include "Node.h"

#include "MC2Point.h"
#include "DrawingProjection.h"
#include "DebugClock.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "NodeBits.h"
#include "FeatureName.h"

static int nbrsplits = 0;
static int nbrcoords = 0;


GfxFeatureMapProcessor::
GfxFeatureMapProcessor( GenericMap& m,
                        GfxFeatureMapProcBase* theOneWithTheCache)
      : GfxFeatureMapProcBase( m, theOneWithTheCache )
{
   m_bbox = new MC2BoundingBox;
   m_mapSettings = NULL; 
}

GfxFeatureMapProcessor::~GfxFeatureMapProcessor()
{
   delete m_bbox;
   delete m_mapSettings;
}

void
GfxFeatureMapProcessor::init(const GfxFeatureMapRequestPacket* p) 
{

   m_nbrReqPackets = p->getNbrReqPackets();
  
   mc2dbg2 << " SMP::init() : map id = " << m_map->getMapID()
           << ", nbr req packets = " << m_nbrReqPackets << endl;
 
   // Clear m_typePerPixel.
   m_typePerPixel.clear();

   // Fill the members with data from the requestpacket
   p->getScreenSize(m_screenX, m_screenY);
   
   p->getMC2BoundingBox(m_bbox);
   m_maxLatFiltDist = 0;

   mc2dbg2 << " SMP::init() : bbox (lat,lon), (lat,lon) = (" 
          << m_bbox->getMaxLat() << "," << m_bbox->getMinLon() << ") ("
          << m_bbox->getMinLat() << "," << m_bbox->getMaxLon() << ")"
          << endl;

   
   delete m_mapSettings;
   m_mapSettings = new MapSettings();
   p->getMapSettings(m_mapSettings);
   
   // Read from mc2.prop which kind of clipping that should be used.
   m_concaveGfxClipper = 
      Properties::getUint32Property( "CONCAVE_GFX_CLIPPER", 0 );

   // Set the boundingbox for the coordinates that will be included in
   // the area
   // 
   // Calculate the extra distance that shuold be added to each side 
   // of the bbox. The distance corresponds to approximately 10 pixels.
   
   int32 extraDist = 0;
  
   if ( ! m_concaveGfxClipper ) {
      extraDist = int32(float64(m_bbox->getHeight() / m_screenY) * 10);
   }
   
   m_coordIncludeBBox.setMaxLat(m_bbox->getMaxLat() + extraDist); 
   m_coordIncludeBBox.setMinLat(m_bbox->getMinLat() - extraDist); 
   m_coordIncludeBBox.setMaxLon(m_bbox->getMaxLon() + extraDist); 
   m_coordIncludeBBox.setMinLon(m_bbox->getMinLon() - extraDist); 

   // Calculate and set the scalelevel of this map.
   m_maxScaleLevel = p->getMaxScaleLevel();
   m_minScaleLevel = p->getMinScaleLevel();
   m_filtScaleLevel = p->getFiltScaleLevel();

   // DEBUG: Print the current scalelevel.
   mc2dbg4 << "GMP: Current scalelevel is " 
           << MapUtility::getScaleLevel( *m_bbox, m_screenX, m_screenY )
           << endl;
   
   // Set draw item parameters.
   MapUtility::makeDrawItemParameters( m_screenX, m_screenY, *m_bbox,
                                       m_xScaleFactor, m_yScaleFactor );
   
   mc2dbg4 << "ScaleLevels:" << endl << "   max = " << m_maxScaleLevel
           << ", min = " << m_minScaleLevel << ", filt = "
           << m_filtScaleLevel << endl;
   
   mc2dbg4 << "m_bbox:" << endl;
   DEBUG4( m_bbox->dump(); );
   mc2dbg2 << "m_coordIncludeBBox:" << endl;
   DEBUG4( m_coordIncludeBBox.dump(); );

   nbrsplits = 0;
   nbrcoords = 0;

   // Reset the outcodes.
   m_prevCSOutcode = m_currCSOutcode = m_nextCSOutcode = 0;

}

inline bool
GfxFeatureMapProcessor::includeFeatureType( 
                                 GfxFeature::gfxFeatureType type ) const
{
   MapSetting* featureSetting = 
      m_mapSettings->getSettingFor( type, 
                                    m_maxScaleLevel,
                                    false ); // don't copy setting
   if ( (featureSetting == NULL) ||
        ((featureSetting != NULL) && 
         (featureSetting->m_onMap || featureSetting->m_borderOnMap)) ) 
   {
      // Either setting was not found, or the feature should be
      // included on the map according to the setting.
      return (true);
   } else {
      return (false);
   }
}


bool
GfxFeatureMapProcessor::includeCoordinate( int32& lat, int32& lon )
{
   // Check if the coordinate should be included.
   
   if ( (m_prevCSOutcode & m_currCSOutcode & m_nextCSOutcode) == 0 ) {
      // Include coordinate!
      
      return (true);
   } else {
      // Should not be included.
      return (false);
   }
}


GfxFeature*
GfxFeatureMapProcessor::createLandFeature(CountryOverviewMap* cmap,
                                          const GfxFeatureMapRequestPacket* p)
{

   LangType lang = p->getLanguage();

   if ( lang == LangTypes::invalidLanguage &&
        cmap->getNbrNativeLanguages() ) {
      // get the top most native language if the
      // requested language was invalid.
      lang = cmap->getNativeLanguage( 0 );
   }

   const char* countryName = StringTable::
      getStringToDisplayForCountry( m_map->getCountryCode(),
                                    ItemTypes::
                                    getLanguageTypeAsLanguageCode( lang ) );

   const char* countryNameEnglish =
      StringTable::getStringToDisplayForCountry( m_map->getCountryCode(),
                                                 StringTable::ENGLISH );

   if (countryName == NULL) {
      countryName = countryNameEnglish;
   }


   GfxFeature* feature = new GfxFeature(
      GfxFeature::LAND, countryName);
   // The country should always be visible 
   feature->setScaleLevel( CONTINENT_LEVEL );
   feature->setBasename( countryNameEnglish );
   return feature;

}


bool
GfxFeatureMapProcessor::addCountryPolygon( CountryOverviewMap* cmap,
                                           GfxFeature* feature, 
                                           int polynumber )
{
   // Set filterlevel, depending on the scale
   byte countryFilterLevel = 0;
   switch (m_filtScaleLevel) {
      case CONTINENT_LEVEL :
         countryFilterLevel = 0;
         break;
      case COUNTRY_LEVEL :
         countryFilterLevel = 1;
         break;
      case COUNTY_LEVEL :
         countryFilterLevel = 2;
         break;
      case SMALL_COUNTY_LEVEL :
         countryFilterLevel = 3;
         break;
      case MUNICIPAL_LEVEL :
         countryFilterLevel = 4;
         break;
      case CITY_LEVEL :
         countryFilterLevel = 5;
         break;
      case SMALL_CITY_LEVEL :
         countryFilterLevel = 6;
         break;
      case DISTRICT_LEVEL :
         countryFilterLevel = 7;
         break;
      case BLOCK_LEVEL :
         countryFilterLevel = 8;
         break;
      case PART_OF_BLOCK_LEVEL :
         countryFilterLevel = 9;
         break;
      case DETAILED_STREET_LEVEL :
         countryFilterLevel = 10;
         break;
   }

   const GfxData* gfx = cmap->getGfxData();

   
   bool retVal;
   uint32 originalSize;
   
   if (countryFilterLevel >= CountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX) {
      // Add all coordinates to the gfxFeature
      mc2dbg4 << "To add all coordinates in country" << endl;
      mc2dbg4 << "Starting to add country" << endl;
      originalSize = gfx->getNbrCoordinates(polynumber);
      uint32 startTime = TimeUtility::getCurrentMicroTime();
      retVal = addGfxDataToFeature(feature, gfx, NULL, polynumber);         
      mc2dbg4 << "Done. It took " 
              << TimeUtility::getCurrentMicroTime() - startTime << " us." << endl;
      
 
   } else {
      // Get filterstack and add the coordinates in that
      // TODO: Add the other polygons that should be included in this 
      //       filter-level.
      const Stack* countryStack = 
         cmap->getFilterStack(countryFilterLevel,polynumber);
      
      originalSize = countryStack->getStackSize();
      mc2dbg4 << "To add " << originalSize 
              << " coordinates in country" << endl;
      retVal = addFilteredGfxDataToFeature(feature, gfx,  
                                           polynumber, countryStack);
   }

   return (retVal);
}

uint32
GfxFeatureMapProcessor::createAndAddBorders(GfxFeatureMap* gfxFeatureMap)
{
   if ( ! MapBits::isCountryMap(m_map->getMapID()) ) {
      return 0;
   }

   // The map gfx data must be filtered
   // (else there are no border items created together with the co polygon)
   if ( ! m_map->mapGfxDataIsFiltered() ) {
      return 0;
   }
   
   uint32 nbrBorders = 0;
   uint32 startTime = TimeUtility::getCurrentTime();
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<m_map->getNbrItemsWithZoom(z); i++) {
         Item* curItem = m_map->getItem(z, i);
         if ( (curItem != NULL) && 
              (curItem->getGfxData() != NULL) &&
              (curItem->getItemType() == ItemTypes::borderItem) ) {
            
            GfxFeature* borderFeature = 
                  new GfxFeature( GfxFeature::BORDER );
            borderFeature->setScaleLevel( CONTINENT_LEVEL );

            // Get the gfx-data that the feature is based on
            bool add = false;
            const GfxData* gfx = curItem->getGfxData();
            if (gfx != NULL) {
               uint32 nbrPolygons = gfx->getNbrPolygons();
               for (uint32 j=0; j<nbrPolygons; j++) {
                  addGfxDataToFeature(borderFeature, gfx, NULL, j );
               }
               add = true;
            }
            if ( add ) {
               gfxFeatureMap->addFeature( borderFeature );
               nbrBorders++;
            } else {
               delete borderFeature;
            }

         }
      }   
   }
   
   mc2dbg8 << "Added " << nbrBorders << " border features in "
           << (TimeUtility::getCurrentTime()-startTime) 
           << " ms" << endl;
   
   return nbrBorders;
}

   
bool 
GfxFeatureMapProcessor::manyFeaturesOnPixel( GfxFeature* feature )
{
   if ( m_maxOneCoordPerPixel && 
        ( feature->getNbrPolygons() == 1 ) &&
        ( feature->getPolygon( 0 )->getNbrCoordinates() == 1 ) ) {
      int32 lat = feature->getPolygon( 0 )->getLat( 0 );
      int32 lon = feature->getPolygon( 0 )->getLon( 0 );
      MC2Coordinate coord( lat, lon );
      MC2Point point =
         m_mapSettings->getDrawingProjection()->getPoint( coord );
      uint32 pixelX = point.getX();
      uint32 pixelY = point.getY();

      if ( ( pixelX < m_screenX ) && ( pixelY < m_screenY ) ) {
         uint32 pixelIdx = pixelX + pixelY * m_screenX;
         
         typedef multimap<uint32, uint32>::iterator mIt_t;
         pair<mIt_t, mIt_t> range = 
            m_typePerPixel.equal_range( pixelIdx );
         bool found = false;
         mIt_t it = range.first;
         while ( ( ! found ) && ( it != range.second ) ) {
            if ( it->second == (uint32)feature->getType() ) {
               found = true;
            } else {
               ++it;
            }
         }
         
         if ( found ) {
            // Already exists a feature of the same type that was
            // completely inside this pixel. No need to add another
            // feature of the same type to this pixel!
            return true;
         } else {
            // Add this feature type to this pixel.
            m_typePerPixel.insert( 
                  make_pair( pixelIdx, feature->getType() ) );
         }
      }
   }
   return false;
}


GfxFeature*
GfxFeatureMapProcessor::createGfxFeatureFromItem(
        Item* item,
        const FilterSettings* filterSettings,
        GfxFeature::gfxFeatureType type,
        bool coordinateStartingAtNode0, 
        int32 lat, 
        int32 lon,
        LangTypes::language_t lang,
        byte extraPOIInfo,
        int startOffset,
        int endOffset,
        const char* imageName )
{ 
//#define INCLUDE_ID_IN_FEATNAME

   // The name of the feature
#ifndef INCLUDE_ID_IN_FEATNAME
   const char* featName = "";
#else
   char featName[128];
   featName[0] = '\0';
#endif
   // Only add names, if present and we are not extracting crossing map.
   if ( (! m_mapSettings->getNavigatorCrossingMap()) && 
        (item->getNbrNames() > 0) ) {
      
      // Get the best name for item considering requested language.
      // (for ssi: use normal name before roadNumber)
      const char* tmpFeatName = "";

      uint32 rawindex = FeatureName::getItemNameIndex( *m_map, *item, lang );

      if ( rawindex != MAX_UINT32 ) {
         // we have a name to use.
         tmpFeatName = m_map->getRawName( rawindex );
      }

      // Look up the name in the map.
#ifndef INCLUDE_ID_IN_FEATNAME
      featName = tmpFeatName;
#else
      sprintf(featName, "%s (%d)", tmpFeatName, item->getID());
#endif
   }

   mc2dbg4 << "createGfxFeatureFromItem " << featName << endl;
   
   // The feature to return
   GfxFeature* feature = NULL;
   
   // Determine the featuretype if that is not already done.
   if (type == GfxFeature::NBR_GFXFEATURES) {
      type = MapUtility::getFeatureTypeForItem( item, 
                                                filterSettings );
   }
   
   if (type != GfxFeature::NBR_GFXFEATURES) {
      if (includeFeatureType( type )) {
         // Either setting was not found, or the feature should be
         // included on the map according to the setting.
         // Add feature!
         feature = GfxFeature::createNewFeature(type, featName);
      } else {
         // Settings says this feature shouldn't be included.
         return (NULL);
      }
   } else {
      // No appropriate feature type
      return (NULL);
   }

   // Set the country code
   feature->setCountryCode( m_map->getCountryCode() );

   // Check if the gfxdata only should consists of the submitted coordinate
   // (typically a poi)

   if ((lat != MAX_INT32) && (lon != MAX_INT32)) {
      // Only add one coord to feature polygon.
      feature->addNewPolygon( true, 1);
      feature->addCoordinateToLast( lat, lon );
   } else {
      // Get the gfx-data that the feature is based on
      const GfxData* gfx = item->getGfxData();
      if (gfx != NULL) {
         uint32 nbrPolygons = gfx->getNbrPolygons();
         for (uint32 j=0; j<nbrPolygons; j++) {
            // Whether to add this polygon or not.
            bool add = true;
            
            if (item->getItemType() == ItemTypes::streetItem) {
               // Need to check that each polygon of the street item
               // should be added, since different street polygons can
               // have different roadclasses and importance.
               uint32 scaleLevel = MapUtility::toDrawStreetPolygon(
                              static_cast<StreetItem*> (item), j);

               if ( (scaleLevel > uint32(m_maxScaleLevel)) || 
                    (scaleLevel < uint32(m_minScaleLevel)) ) {
                  add = false;
               }
            }
            
            if ( add ) {
               addGfxDataToFeature(feature, gfx, filterSettings, j,
                                   coordinateStartingAtNode0);
            }
         }
         
         // Don't add the feature if it only covers one pixel and there
         // are already a feature of the same type covering that pixel.
         if ( manyFeaturesOnPixel( feature ) ) {
            delete feature;
            return NULL;
         }
      }
   }
  
   // Set the poi type for GfxFeature::POI
   if ((feature->getType() == GfxFeature::POI) && 
       (item->getItemType() == ItemTypes::pointOfInterestItem)) {
      ItemTypes::pointOfInterest_t poiType = 
         (static_cast<PointOfInterestItem*> (item))
            ->getPointOfInterestType();

      GfxPOIFeature* gfxPOI = dynamic_cast<GfxPOIFeature*> (feature);
      gfxPOI->setPOIType( poiType );
      gfxPOI->setExtraInfo( extraPOIInfo );

      if ( imageName != NULL ) {
         gfxPOI->setCustomImageName( imageName );
      }
      
   } 
   else
   // Set the data in the GfxRoadPolygon if street segment item
   if (item->getItemType() == ItemTypes::streetSegmentItem) {
      StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>(item);
      for (uint32 i=0; i<feature->getNbrPolygons(); i++) {
         GfxRoadPolygon* poly = dynamic_cast<GfxRoadPolygon*>
                                            (feature->getPolygon(i));
         if (poly != NULL) {
            mc2dbg8 << "setting parameters for polygon" << endl;
            poly->setParams(ssi->getNode(0)->getSpeedLimit(),
                            ssi->getNode(1)->getSpeedLimit(),
                            ssi->isMultiDigitised(),
                            ssi->isRamp(), 
                            ssi->isRoundabout(),
                            ssi->getNode(0)->getLevel(),
                            ssi->getNode(1)->getLevel(), 
                            ssi->getNode(0)->getEntryRestrictions(),
                            ssi->getNode(1)->getEntryRestrictions());
            DEBUG4(
               if ( (ssi->getNode(0)->getLevel() != 0) ||
                    (ssi->getNode(1)->getLevel() != 0)) {
                  mc2dbg4 << "SSI with level!=0, id=" << ssi->getID() << endl;
                  poly->dump(1);
               }
            );
         }
      }
   }
   
   
   // Return the feature that was created here.
   return (feature);
}

void
GfxFeatureMapProcessor::addCoordsFromGfxDataToPointVector( 
                                    const GfxData* gfx, 
                                    uint16 poly,
                                    const Stack* polyStack,
                                    vector<POINT>& vertices )
{
   POINT p;
   if ( polyStack == NULL ) {
      for ( uint32 i = 0; 
            i < gfx->getNbrCoordinates( poly ); ++i ) {
         p.x = gfx->getLon( poly, i );
         p.y = gfx->getLat( poly, i );
         vertices.push_back( p );
      }
   } else {
      // Add only the vertices present in polyStack.
      for ( uint32 i = 0; i < polyStack->getStackSize(); ++i ) {
         p.x = gfx->getLon( poly, polyStack->getElementAt( i ) );
         p.y = gfx->getLat( poly, polyStack->getElementAt( i ) );
         vertices.push_back( p );
      }
   }
   if ( gfx->getClosed( poly ) ) {
      // Closed polygon.
      if ( vertices.front().x != p.x ||
            vertices.front().y != p.y ) {
         vertices.push_back( vertices.front() );
      }
   }
}

bool
GfxFeatureMapProcessor::addFilteredGfxDataToFeature( 
   GfxFeature* feature, 
   const GfxData* gfx,
   uint32 poly,
   const Stack* polyStack,
   bool forward)
{

   // Check if 32 or 16 bit representation of the coordinates
   // is suitable.
   bool coordinates16Bits = true;
   
   // Note that the length is 0 for most items on the country overviewmap
   // currently. This need to be taken care of!!
   
   //if (gfx->getLength() == 0) {
   //   (const_cast<GfxData*> (gfx))->updateLength();
   //}
   if ( uint32(gfx->getLength(poly) / float64(gfx->getNbrCoordinates(poly))) > 
        650 ) {
      coordinates16Bits = false;
   }
   
   
   // Two cases:
   // 
   // (1) Adding a closed polygon:
   //     First clip the polygon and then add it by using
   //     the simple addClippedFirstCoordinate(..) 
   //     and addClippedCoordinate(..).
   //     
   // (2) Adding an open polygon:
   //     Add it using addFirstCoordinate(..) 
   //     and addPreviousCoordinate(..).  
   
   // NB: 
   // Code is optimized for speed rather than visibility in some places..
   
   if (gfx->getClosed(poly)) {
      // (1) Adding a closed polygon:
      uint16 firstNewPoly = feature->getNbrPolygons();
 
      MC2BoundingBox bb;
      gfx->getMC2BoundingBox(bb);
      if (! bb.inside(m_coordIncludeBBox)) {      
         // Clip the feature since it is only partly inside the bbox.
               
            
            vector< vector<POINT> > clippedPolygons;
            clippedPolygons.push_back( vector<POINT>() );
        
            vector<POINT>& vertices = clippedPolygons.front();
            // Add the coordinates from the gfxdata to the point vector.
            addCoordsFromGfxDataToPointVector( gfx, 
                                               poly, 
                                               polyStack, 
                                               vertices );
            
            if ( m_concaveGfxClipper ) {
               // Greiner Hormann clipper.
               if ( ! GfxUtility::clipToPolygon( m_coordIncludeBBox, 
                                                 clippedPolygons ) ) {
                  return false;
               }
            } else {
               // Sutherland Hodgemann clipper.
               if ( ! GfxUtility::clipPolyToBBoxFast( &m_coordIncludeBBox, 
                                                      vertices ) ) {
                  return false;
               }
            }
            
            for ( vector< vector<POINT> >::iterator pt = 
                     clippedPolygons.begin(); pt != clippedPolygons.end();
                  ++pt ) {
               vector<POINT>& clippedVertices = *pt;
               
               uint32 nbrCoords = clippedVertices.size();
               if (forward)  {
                  // Add in forward direction.
                  vector<POINT>::iterator it = 
                     clippedVertices.begin();
                  addClippedFirstCoordinate(it->y, it->x, 
                                            feature, 
                                            nbrCoords,
                                            coordinates16Bits);
                  ++it;
                  for (;it != clippedVertices.end(); ++it) {
                     addClippedCoordinate(it->y, it->x, feature);
                  }
               } else {
                  // Add in reverse direction
                  vector<POINT>::reverse_iterator it = 
                     clippedVertices.rbegin();
                  addClippedFirstCoordinate(it->y, it->x, 
                                            feature,
                                            nbrCoords,
                                            coordinates16Bits);
                  ++it;
                  for (;it != clippedVertices.rend(); ++it) {
                     addClippedCoordinate(it->y, it->x, feature);
                  }
               }
               addClosedCoordinate( feature );
            }
         
      } else {
         // No need to clip feature, since all of it is inside the bbox.
         uint32 nbrCoords;
      
         if (polyStack != NULL) {
            // Add filtered coordinates
            nbrCoords = polyStack->getStackSize();
            uint32 index;
            if (nbrCoords > 0) {       
               if (forward) {
                  index = polyStack->getElementAt(0);
                  // Add in forward direction.
                  addClippedFirstCoordinate(gfx->getLat(poly, index),
                                            gfx->getLon(poly, index),
                                            feature, 
                                            nbrCoords,
                                            coordinates16Bits);
                  for (uint32 i = 1; i < nbrCoords; i++) {
                     index = polyStack->getElementAt(i);
                     addClippedCoordinate(gfx->getLat(poly, index),
                                          gfx->getLon(poly, index),
                                          feature);
                  }
               } else {                  
                  // Add in reverse direction
                  index = polyStack->getElementAt(nbrCoords - 1);
                  addClippedFirstCoordinate(
                        gfx->getLat(poly, index),
                        gfx->getLon(poly, index), 
                        feature,
                        nbrCoords,
                        coordinates16Bits);
                  for (int32 i = nbrCoords-1; i >= 0; i--) {
                     index = polyStack->getElementAt(i);
                     addClippedCoordinate(gfx->getLat(poly, index),
                                          gfx->getLon(poly, index),
                                          feature);
                  }
               }
               addClosedCoordinate( feature );
            }
            
         } else { // polyStack == NULL
            // Add all coordinates
            nbrCoords = gfx->getNbrCoordinates(poly);
            if (nbrCoords > 0) {       
               if (forward) {
                  // Add in forward direction.
                  addClippedFirstCoordinate(gfx->getLat(poly, 0),
                                            gfx->getLon(poly, 0),
                                            feature,
                                            nbrCoords,
                                            coordinates16Bits);
                  for (uint32 i = 1; i < nbrCoords; i++) {
                     addClippedCoordinate(gfx->getLat(poly, i),
                                          gfx->getLon(poly, i),
                                          feature);
                  }
               } else {
                  // Add in reverse direction
                  addClippedFirstCoordinate(
                        gfx->getLat(poly, nbrCoords-1),
                        gfx->getLon(poly, nbrCoords-1), 
                        feature,
                        nbrCoords,
                        coordinates16Bits);
                  for (int32 i = nbrCoords-1; i >= 0; i--) {
                     addClippedCoordinate(gfx->getLat(poly, i),
                                          gfx->getLon(poly, i),
                                          feature);
                  }
               }
               addClosedCoordinate( feature );
            }
         }
      }         
      // Set the area of the new polygons.
      float64 area = abs( gfx->polygonArea( poly ) *  
                          GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER );
      for ( uint16 j = firstNewPoly; j < feature->getNbrPolygons(); ++j ) {
         feature->getPolygon( j )->setArea( area ); 
      }

   } else {
      // (2) Adding an open polygon:
   
      if (polyStack != NULL) {
         
         uint32 stackSize = polyStack->getStackSize();

         int32 startIndex = 0;
         // The index after start index
         int32 nextIndex = 0;
         // The stop index (is invalid)
         int32 stopIndex = 0;
         bool increase = true;

         if ( forward ) {
            startIndex = 0;
            nextIndex = 1;
            stopIndex = stackSize;
            
            increase = true;
         } else {
            startIndex = stackSize - 1;
            nextIndex = startIndex - 1;
            stopIndex = -1;
            increase = false;
         }

         if (stackSize < 2) {
            // No next coordinate exists. Use the first coordinate instead.
            nextIndex = startIndex; 
         }
         
         // Add the first coordinate to the feature
         uint32 idx = polyStack->getElementAt( startIndex );
         uint32 nextIdx = polyStack->getElementAt( nextIndex );
         addFirstCoordinate( gfx->getLat( poly, idx), 
                             gfx->getLon( poly, idx), 
                             gfx->getLat( poly, nextIdx),
                             gfx->getLon( poly, nextIdx),
                             feature, stackSize,
                             coordinates16Bits );
            
         // Add the rest of the coordinates except for the last one.
         if (stackSize > 2) {
            // Step forward two indices for startIndex. 
            // (It points to next coord)
            for ( int32 i = startIndex + (increase ? 2 : -2) ; 
                 i != stopIndex ; 
                 increase ? i++ : i-- ) 
            {
               idx = polyStack->getElementAt( i );
               addPreviousCoordinate( gfx->getLat( poly, idx), 
                                      gfx->getLon( poly, idx),
                                      feature );
            }
         }

         // Add the last coordinate
         if (stackSize > 1) {
            addPreviousCoordinate( feature );
         }

         cerr << "Added " << stackSize << " coordinates out of " 
              << gfx->getNbrCoordinates( poly ) << endl;

         // Check if polygon closed
         if ( gfx->closed() ) {
            addClosedCoordinate( feature );
         }

      } else { // polyStack == NULL
         // The stack is NULL. Add all coordinates.
         int32 startIndex = 0;
         // The index after start index
         int32 nextIndex = 0;
         // The stop index (is invalid)
         int32 stopIndex = 0;
         
         bool increase = true;

         int32 nbrCoords = gfx->getNbrCoordinates( poly );

         if ( forward ) {
            startIndex = 0;
            stopIndex = nbrCoords;
            nextIndex = 1;
            increase = true;
         } else {
            startIndex = nbrCoords - 1;
            stopIndex = -1;
            nextIndex = startIndex - 1;
            increase = false;
         }
         
         
         if (nbrCoords < 2) {
            // No next coordinate exists. Use the first coordinate instead.
            nextIndex = startIndex; 
         }

         // Add the first coordinate to the feature      
         addFirstCoordinate( gfx->getLat( poly, startIndex), 
                             gfx->getLon( poly, startIndex),
                             gfx->getLat( poly, nextIndex),
                             gfx->getLon( poly, nextIndex), 
                             feature, MIN( gfx->getNbrCoordinates( poly ), 
                                           256 ),
                             coordinates16Bits );
         
         
         // Add the rest of the coordinates except for the last one.
         if (nbrCoords > 2) {
            // Step forward two indices for startIndex. 
            // (It points to next coord)
            for ( int32 i = startIndex + (increase ? 2 : -2); 
                  i != stopIndex ; 
                  increase ? i++ : i-- ) {
               // Note that the coordinate supplied to this method is the
               // coordinate AFTER the one currently being added 
               // (ie. next coordinate).
               addPreviousCoordinate( 
                          gfx->getLat( poly, i ), gfx->getLon( poly, i ),
                          feature );
            }
         }

         // Add the last coordinate
         if (nbrCoords > 1) {
            addPreviousCoordinate( feature );
         }

         // Check if polygon closed
         if ( gfx->closed() ) {
            addClosedCoordinate( feature );
         }
      }
   }
   
   // Successful if there is at least one polygon
   return (feature->getNbrPolygons() > 0);
}


bool
GfxFeatureMapProcessor::addGfxDataToFeature(GfxFeature* feature, 
                                            const GfxData* gfx,
                                            const FilterSettings* settings,
                                            uint32 poly,
                                            bool coordinateStartingAtNode0)
{
   mc2dbg4 << " addGfxDataToFeature " << endl;

   if (settings != NULL) {
      switch (settings->m_filterType) {
         case FilterSettings::OPEN_POLYGON_FILTER :
         case FilterSettings::CLOSED_POLYGON_FILTER : {
            mc2dbg4 << "Filtering GfxData!!!" << endl;
            // Create the stack to filter with
            Stack polyStack(gfx->getNbrCoordinates(poly)/10);
            polyStack.reset();

            // Filter the polygon
            bool stackFilled = false;

            // Filter the polygon
            if (settings->m_filterType == 
                  FilterSettings::CLOSED_POLYGON_FILTER) {
               stackFilled = ((GfxData*) gfx)->openPolygonFilter(
                                          &polyStack, 
                                          poly, 
                                          settings->m_maxLatDist,
                                          settings->m_maxWayDist);
            } else {
               stackFilled = ((GfxData*) gfx)->openPolygonFilter(
                                        &polyStack, 
                                        poly, 
                                        settings->m_maxLatDist,
                                        settings->m_maxWayDist);
            }
            // Add the coordinates
            if (stackFilled) {
               addFilteredGfxDataToFeature(feature, gfx, poly, 
                                           &polyStack, 
                                           coordinateStartingAtNode0);
            }
         } break;
         
         case FilterSettings::SYMBOL_FILTER : {
            // Add one coordinate (middle of bbox)
            int32 lat = gfx->getMinLat() + 
                        gfx->getMaxLat()/2-gfx->getMinLat()/2;
            int32 lon = gfx->getMinLon() + 
                        gfx->getMaxLon()/2-gfx->getMinLon()/2;
            feature->addNewPolygon( true, 1);
            feature->addCoordinateToLast( lat, lon );
         } break;

         case FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER: {
            Stack polyStack;

            if ( gfx->douglasPeuckerPolygonFilter( polyStack,
                                                   poly,
                                                   settings->m_maxLatDist ) ) {
               addFilteredGfxDataToFeature( feature, gfx, poly,
                                            &polyStack,
                                            coordinateStartingAtNode0 );
            }
         } break;
         default :
            // Add all the coordinates
            addFilteredGfxDataToFeature(feature, gfx, poly, NULL, 
                                        coordinateStartingAtNode0);
      }
   } else {
      // Settings == NULL
      addFilteredGfxDataToFeature(feature, gfx, poly, NULL, 
                                  coordinateStartingAtNode0);
   }


   // Successful if there is at least one polygon
   return (feature->getNbrPolygons() > 0);
}

void
GfxFeatureMapProcessor::addClippedFirstCoordinate(int32 lat, int32 lon,
                                                  GfxFeature* feature,
                                                  uint32 approxNbrCoord,
                                                  bool coordinates16Bits)
{
   //Always use 16 bits until the clients support mixed 16/32 coordinates.
   coordinates16Bits = true;
   m_startLat = m_prevAddedLat = lat;
   m_startLon = m_prevAddedLon = lon;

   // Calculate pixel coordinates. Only necessary if we are extracting
   // a "flat" map, ie. it should not be possible to zoom into it.
   if (m_maxOneCoordPerPixel) {
      MC2Coordinate coord( lat, lon );
      MC2Point point =
         m_mapSettings->getDrawingProjection()->getPoint( coord );
      m_prevPixelX = point.getX();
      m_prevPixelY = point.getY();
   }
   
   feature->addNewPolygon(coordinates16Bits, approxNbrCoord);
   feature->addCoordinateToLast(lat, lon);
   mc2dbg4 << "addClippedFirstCoordinate :"
           << " new polygon with start-coord (" 
           << m_prevAddedLat << "," << m_prevAddedLon << ")" << endl;
}

void
GfxFeatureMapProcessor::addClippedCoordinate(int32 lat, int32 lon,
                                             GfxFeature* feature)
{
   // Calculate pixel coordinates. Only necessary if we are extracting
   // a "flat" map, ie. it should not be possible to zoom into it.
   int currX = 0;
   int currY = 0;
   if (m_maxOneCoordPerPixel) {
      MC2Coordinate coord( lat, lon );
      MC2Point point =
         m_mapSettings->getDrawingProjection()->getPoint( coord );
      currX = point.getX();
      currY = point.getY();
   }

   // Always add coordinates if zoomable.
   // Otherwise only add coordinates if this coordinate will end up
   // at a different pixel than the previous one.
   if ( (! m_maxOneCoordPerPixel) || 
        ((m_prevPixelX != currX) || (m_prevPixelY != currY )) ) {
      // Add coordinate.
      feature->addCoordinateToLast( lat, lon, 
                                    m_prevAddedLat, m_prevAddedLon );
      
      // Update prev members
      
      m_prevAddedLat = lat;
      m_prevAddedLon = lon;

      m_prevPixelX = currX;
      m_prevPixelY = currY;
   }   
}

void
GfxFeatureMapProcessor::addFirstCoordinate(int32 lat, int32 lon, 
                                           int32 nextLat, int32 nextLon,
                                           GfxFeature* feature,
                                           uint32 approxNbrCoord,
                                           bool coordinates16Bits)
{
   // Update the outcodes
   m_prevCSOutcode = 
      m_coordIncludeBBox.getCohenSutherlandOutcode( lat, lon ); 
   m_currCSOutcode = 
      m_coordIncludeBBox.getCohenSutherlandOutcode( nextLat, nextLon );
   m_nextCSOutcode = 0;
   
   addClippedFirstCoordinate(lat, lon, feature, 
                             approxNbrCoord, coordinates16Bits);
   
   m_currLat = nextLat;
   m_currLon = nextLon;
   
}

void
GfxFeatureMapProcessor::addCoordinateToFeature(
                                    GfxFeature* feature, 
                                    int latDiff, int lonDiff)
{
   const int MAX_COORD_DIFF = 0x7ffe;

   int fooStartLat = latDiff;
   int fooStartLon = lonDiff;
   int fooLat = 0;
   int fooLon = 0;

   // Make points along the too long line
   while ( (latDiff > MAX_COORD_DIFF) || (latDiff < -MAX_COORD_DIFF) ||
           (lonDiff > MAX_COORD_DIFF) || (lonDiff < -MAX_COORD_DIFF) ) 
   {
      // Move MAX_COORD_DIFF
      int tmpLatDiff = 0;
      int tmpLonDiff = 0;
      if ( abs( latDiff ) > abs( lonDiff ) ) {
         // lat largest
         tmpLatDiff = latDiff < 0 ? (-MAX_COORD_DIFF) : MAX_COORD_DIFF;
         tmpLonDiff = int( rint( 
            tmpLatDiff * float64(lonDiff) / latDiff ) );
      } else {
         // lon largest
         tmpLonDiff = lonDiff < 0 ? (-MAX_COORD_DIFF) : MAX_COORD_DIFF;
         tmpLatDiff = int( rint( 
            tmpLonDiff * float64(latDiff) / lonDiff ) );
      }
      mc2dbg8 << " spiltto  x " << tmpLonDiff 
              << " y " << tmpLatDiff << endl;
      nbrsplits++;
      feature->addCoordinateToLast( tmpLatDiff, tmpLonDiff ); 
      nbrcoords++;
      fooLat += tmpLatDiff;
      fooLon += tmpLonDiff;
      latDiff -= tmpLatDiff;
      lonDiff -= tmpLonDiff;
   }

   fooLat += latDiff;
   fooLon += lonDiff;

   if ( fooStartLon != fooLon || fooStartLat != fooLat ) {
      mc2dbg4 << "      x " << fooStartLon << " y " << fooStartLat << endl;
      mc2dbg4 << "      x " << fooLon << " y " << fooLat << endl;
   }
  
   feature->addCoordinateToLast(latDiff, lonDiff);
   nbrcoords++;
}

void
GfxFeatureMapProcessor::addPreviousCoordinate( int32 nextLat,
                                               int32 nextLon,
                                               GfxFeature* feature )
{
  
   m_nextCSOutcode = 
      m_coordIncludeBBox.getCohenSutherlandOutcode( nextLat, nextLon );
   
   if (includeCoordinate( m_currLat, m_currLon )) {
      // Add coordinate
      addClippedCoordinate(m_currLat, m_currLon, feature);
      // Update the outcodes
      m_prevCSOutcode = m_currCSOutcode;
      m_currCSOutcode = m_nextCSOutcode;
   }

   // Update current coordinate.
   m_currLat = nextLat;
   m_currLon = nextLon;
}

void
GfxFeatureMapProcessor::addPreviousCoordinate( GfxFeature* feature )
{
   // Supply currLat and currLon as next coordinates (even if this is
   // the last coordinate to add).
   addPreviousCoordinate( m_currLat, m_currLon, feature );
}

void
GfxFeatureMapProcessor::addClosedCoordinate(GfxFeature* feature)
{
//   addCoordinateToFeature(feature, m_startLat-m_prevAddedLat, 
//                                   m_startLon-m_prevAddedLon );
     addClippedCoordinate(m_startLat, m_startLon, feature);
}


void
GfxFeatureMapProcessor::createRouteGfxFeatureMap(
                                 const GfxFeatureMapRequestPacket* p,
                                 GfxFeatureMap* gfxFeatureMap)
{
   mc2dbg2 << "GfxFeatureMapProcessor::createRouteGfxFeatureMap() "
           << "from map " << m_map->getMapID() << endl;
   
   // Extract an gfxFeaturemap of a route.
   list<uint32> nodeIDs;
   uint32 nbrNodes = p->getNodeIDs( m_map, nodeIDs );

   uint32 x;
   if ( nodeIDs.front() == 0xf0000002 ||
        nodeIDs.front() == 0xf0000003 ) {
      x = 1;
   }
   else 
      x = 0;
   
   // The RouteModule gives us two ids even when routing on only one
   // segment. Lets remove the extra one.
//   if ( ( nbrNodes == (2+x) ) && (p->getNodeID(x) == p->getNodeID(x+1))) 
//      nbrNodes--;
  
   if ( nbrNodes == (2+x) ) {
      list<uint32>::const_iterator it = nodeIDs.begin();
      for ( uint32 i = 0; i < x; ++i ) {
         ++it;
      }
      list<uint32>::const_iterator nextIt = it;
      ++nextIt;
      if ( *it == *nextIt ) {
         --nbrNodes;
         nodeIDs.pop_back();
      }
   }
   
   mc2dbg4 << "   Number of route nodes " << nbrNodes << endl;
   
   uint32 nbrAdded = 0;
   bool firstFeature = true;

   list<uint32>::const_iterator it = nodeIDs.begin();
   for (uint32 i = 0; i < nbrNodes; i++) {
      uint32 nodeID = *it;
      mc2dbg8 << "nodeID = " << nodeID << endl;

      // Get the corresponding item.
      Item* item = m_map->itemLookup(0x7fffffff & nodeID);

      GfxData* gfx = NULL;
      if ((item != NULL) && ((gfx = item->getGfxData()) != NULL)) {

         bool includeRouteOrigin = 
            (firstFeature && (! p->getIgnoreStartOffset()));
         bool includeRouteDestination =
            (i == (nbrNodes - 1)) && (! p->getIgnoreEndOffset());
         
         // XXX: Quick fix...
         MC2BoundingBox bbox;
         gfx->getMC2BoundingBox(bbox);
         if ( ! bbox.overlaps( m_bbox ) ) {
            // Mark that the first feature has been taken care of.
            if ( includeFeatureType( GfxFeature::EMPTY ) ) {
               // Add empty feature to make numbering work
               if ( includeRouteOrigin ) {
                  // Add an extra empty feature, (ie. route origin)
                  gfxFeatureMap->addFeature( 
                        new GfxFeature( GfxFeature::EMPTY ) );
               }
               if ( includeRouteDestination ) {
                  // Add an extra empty feature, (ie. route destination)
                  gfxFeatureMap->addFeature( 
                        new GfxFeature( GfxFeature::EMPTY ) );
               }
               gfxFeatureMap->addFeature( 
                     new GfxFeature( GfxFeature::EMPTY ) );
            }
            firstFeature = false;

         } else {

            GfxFeature* routeFeature = NULL;
            // Destination must be added last after last ROUTE
            GfxFeature* destFeature = NULL;

            // Find out if coordinates start at node 0 or 1
            byte nodeNbr = 0;
            if ( nodeID & 0x80000000 ) {
               // Means the coordinates should be added in opposite order.
               nodeNbr = 1;
            }

            bool start = false;
            
            // Check if we should handle start/end of route with offsets.
            if ( includeRouteOrigin ) {
               mc2dbg8 << "   Handling start of route" << endl;
               // Create the feature
               routeFeature = new GfxFeature(GfxFeature::ROUTE);
               start = true;
               
               // Get the cardinal number for the coordinate following 
               // (startLat, startLon).
               int32 startLat, startLon;
               int32 startIdx = gfx->getCoordinate( p->getStartOffset(),
                                              startLat, 
                                              startLon );

              
               if (nodeNbr == 0) {               
                  // 0            10              10            1
                  // o------------oo--------------oo------------o
                  //     x-------->
                  
                  // Add first coordinate 
                  addFirstCoordinate(startLat, startLon,    // first
                                     gfx->getLat(0,startIdx), // next
                                     gfx->getLon(0,startIdx), // next
                                     routeFeature);

                  int32 stopIdx = gfx->getNbrCoordinates(0) - 1;
                  
                  // Add the rest of the coordinates
                  for (int32 i=startIdx+1; i<=stopIdx; i++) {
                     addPreviousCoordinate( gfx->getLat(0,i), 
                                            gfx->getLon(0,i), 
                                            routeFeature); 
                  }
                  // Add last coord
                  addPreviousCoordinate( routeFeature ); 
               } else {
                  // 0            10              10            1
                  // o------------oo--------------oo------------o
                  //                               <----x
                  
                  startIdx--;
                  // Add first coordinate 
                  addFirstCoordinate(startLat, startLon,    // first
                                     gfx->getLat(0, startIdx), // next
                                     gfx->getLon(0, startIdx), // next
                                     routeFeature);
                  int32 stopIdx = 0;
                  for (int32 i=startIdx-1; i>=stopIdx; i--) {
                     addPreviousCoordinate( gfx->getLat(0, i), 
                                            gfx->getLon(0, i), 
                                            routeFeature); 
                  }
                  // Add last coord
                  addPreviousCoordinate( routeFeature ); 
               }
               
               // Add ROUTE_ORIGIN feature.
               mc2dbg8 << "   To add ROUTE_ORIGIN-feature" << endl;
               GfxFeature* originFeature = 
                  new GfxFeature(GfxFeature::ROUTE_ORIGIN);
               originFeature->addNewPolygon(true, 1);
               originFeature->addCoordinateToLast(startLat, startLon);
               gfxFeatureMap->addFeature(originFeature);
               
               // Starting angle towards node 1
               float64 angle = gfx->getAngle( p->getStartOffset() );
               if ( nodeNbr ) {
                  // other direction
                  angle = int(rint(angle + 180.0)) % 360;
               }
               gfxFeatureMap->setStartingAngle( uint32(angle * 256 / 360) );

               // initialUTurn
               // TODO: Get the initial uturn from the route and set
               //       start if so.
            }

            // Check if to handle end of route
            if ( includeRouteDestination ) {
               mc2dbg8 << "   Handling end of route" << endl;
               // Create the feature
               delete routeFeature;
               
               routeFeature = new GfxFeature(GfxFeature::ROUTE);
               
               // Get the cardinal number for the coordinate following 
               // (endLat, endLon).
               int32 endLat, endLon;
               int32 stopIdx = gfx->getCoordinate( p->getEndOffset(),
                                                   endLat, 
                                                   endLon );
               
               if (!start) { 
                  // Routing on more than one segment. The normal case.
                  if (nodeNbr == 0) {
                     // 0            10              10            1
                     // o------------oo--------------oo------------o
                     //                               -------->x
                     
                     // Add first coordinate
                     int32 startIdx = 0;
                     
                     // The second coordinate to add
                     int32 nextLat, nextLon;
                     if (stopIdx > 1) {
                        nextLat = gfx->getLat(0, 1);
                        nextLon = gfx->getLon(0, 1);
                     } else {
                        nextLat = endLat;
                        nextLon = endLon;
                     }
                     
                     addFirstCoordinate(gfx->getLat(0,startIdx), // first
                                        gfx->getLon(0,startIdx), // first
                                        nextLat, // next
                                        nextLon, // next
                                        routeFeature);
                     
                     startIdx += 2;
                     stopIdx--;
                     for (int32 i=startIdx; i<=stopIdx; i++) 
                        addPreviousCoordinate( gfx->getLat(0, i), // next
                                               gfx->getLon(0, i), // next
                                               routeFeature );
                     // Add last coord
                     addPreviousCoordinate(endLat, endLon, // next
                                           routeFeature);
                     addPreviousCoordinate(routeFeature);
                  }  else { 
                     // 0            10              10            1
                     // o------------oo--------------oo------------o
                     //      x<-------
                     
                     // Add first coordinate
                     int32 startIdx = gfx->getNbrCoordinates(0) - 1;
                     
                     // The second coordinate to add.
                     int32 nextLat, nextLon;
                     if (stopIdx == startIdx) {
                        nextLat = endLat;
                        nextLon = endLon;
                     } else {
                        nextLat = gfx->getLat(0,startIdx - 1);
                        nextLon = gfx->getLon(0,startIdx - 1);
                     }
                     
                     addFirstCoordinate(gfx->getLat(0,startIdx), // first
                                        gfx->getLon(0,startIdx), // first
                                        nextLat, // next
                                        nextLon, // next
                                        routeFeature);
                     startIdx--;
                     for (int32 i=startIdx; i>=stopIdx; i--) {
                        addPreviousCoordinate(gfx->getLat(0,i), // next
                                              gfx->getLon(0,i), // next
                                              routeFeature); 
                     }
                     // Add last coord
                     addPreviousCoordinate(endLat, endLon, // next
                                           routeFeature);
                     addPreviousCoordinate(routeFeature);
                     } 
               } else {
                  // Routing on only one segment.
                  // We need some extra checks here since the routemodule
                  // does not set the correct nodeNbr unfortunately
                  // (it's always 0 regardless of the direction of the 
                  // route).
                  
                  int32 startLat, startLon;
                  
                 
                  int32 startIdx = gfx->getCoordinate( p->getStartOffset(),
                                                       startLat, 
                                                       startLon );
                               
                  // Add coordinates in the correct order!
                  if (nodeNbr == 0) {
                     
                     int32 nextLat, nextLon;
                     if (startIdx == stopIdx) {
                        // Only two coordinates.
                        nextLat = endLat;
                        nextLon = endLon;
                     } else {
                        nextLat = gfx->getLat(0,startIdx);
                        nextLon = gfx->getLon(0,startIdx);
                     }
                     
                     addFirstCoordinate(startLat,startLon, // first
                                        nextLat, nextLon,  // next
                                        routeFeature);
                     
                     startIdx++;
                     stopIdx--;
                     
                     for (int32 i = startIdx; i <= stopIdx; i++) 
                        addPreviousCoordinate(gfx->getLat(0,i), // next 
                                              gfx->getLon(0,i), // next
                                              routeFeature);              
                  } else {
                     
                     int32 nextLat, nextLon;
                     if (startIdx == stopIdx) {
                        // Only two coordinates.
                        nextLat = endLat;
                        nextLon = endLon;
                     } else {
                        startIdx--;
                        nextLat = gfx->getLat(0,startIdx);
                        nextLon = gfx->getLon(0,startIdx);
                     }
                     
                     addFirstCoordinate(startLat,startLon, // first
                                        nextLat, nextLon,  // next
                                        routeFeature);
                     startIdx--;
                     
                     for (int32 i = startIdx; i >= stopIdx; i--) 
                        addPreviousCoordinate(gfx->getLat(0,i), // next
                                              gfx->getLon(0,i), // next
                                              routeFeature);
                  }
                  // Last coord
                  addPreviousCoordinate(endLat, endLon,  // next
                                        routeFeature);
                  addPreviousCoordinate(routeFeature);
               }
               
               // Add ROUTE_DESTINATION feature.
               mc2dbg8 << "   To add ROUTE_DESTINATION-feature" << endl;
               destFeature = 
                  new GfxFeature(GfxFeature::ROUTE_DESTINATION);
               destFeature->addNewPolygon(true, 1);
               destFeature->addCoordinateToLast(endLat, endLon);
            } 

            if (routeFeature == NULL) {
               // Not the start or end of a route. 
               // Create the features as usual.
              
               // language does not matter, it is not used..
               routeFeature = createGfxFeatureFromItem(item, 
                                                       NULL,
                                                       GfxFeature::ROUTE,
                                                       (nodeNbr == 0) );
            }

            if (routeFeature != NULL) {
               mc2dbg4 << "   To add route-feature to map" << endl;
               gfxFeatureMap->addFeature(routeFeature);
               nbrAdded++;
            } else {
               mc2dbg2 << "WARN: Failed to add route feature..." << endl;
            }
            if ( destFeature != NULL ) {
               gfxFeatureMap->addFeature(destFeature);
               destFeature = NULL;
            }

            // This is not the first feature any more.
            firstFeature = false;
         }
      } else {
         // An item cold not be retreived from the node id.
         // The node id represents a change of transportation type.
         ItemTypes::transportation_t transpType =
            GET_TRANSPORTATION_STATE(nodeID);
 
         if ( firstFeature ) {
            // Set the transportation type.
            gfxFeatureMap->setTransportationType( transpType );

            // drivingOnRightSide
            gfxFeatureMap->setDrivingOnRightSide( 
               m_map->driveOnRightSide() );
         } else if ( transpType == ItemTypes::walk ) {
            // Send a PARK_CAR feature.
            // Get the coordinate from the previous route node.
            list<uint32>::const_iterator prevIt = it;
            --prevIt;
            uint32 prevNodeID = *prevIt;
            Item* prevItem = 
               m_map->itemLookup( prevNodeID & ITEMID_MASK );
            GfxData* prevGfx = NULL;
            if ( prevItem != NULL &&    // Shouldn't happen but ...
                 (prevGfx = prevItem->getGfxData()) != NULL ) { 
               uint32 coordIdx = 0; 
               if ( MapBits::isNode0( prevNodeID ) ) {
                  coordIdx = prevGfx->getNbrCoordinates(0) - 1;
               }
               int32 lat = prevGfx->getLat( 0,coordIdx );
               int32 lon = prevGfx->getLon( 0,coordIdx );
               
               GfxFeature* routeFeature = new GfxFeature( 
                  GfxFeature::PARK_CAR, // PARK_AND_WALK is better name
                  m_map->getItemName( prevItem,
                                      p->getLanguage(),
                                      ItemTypes::invalidName ),
                  false );
               
               routeFeature->addNewPolygon( true, 1 );
               routeFeature->addCoordinateToLast( lat, lon );
               
               gfxFeatureMap->addFeature( routeFeature );
            }
         } else { // Unknown
            mc2log << warn << here << " Strange node transpType " 
                   << int(transpType) << endl;
         }
      }
      // Update the node iterator.
      ++it;
   }
     
   mc2dbg4 << "Added " << nbrAdded << " route features." << endl;
   
}

void 
GfxFeatureMapProcessor::createPOIGfxFeatureMap(
                                       const GfxFeatureMapRequestPacket* p,
                                       GfxFeatureMap* featureMap)
{
   mc2dbg2 << "GfxFeatureMapProcessor::createPOIGfxFeatMap() from map " 
           << m_map->getMapID() << " lang " 
           << (int)p->getLanguage() << endl;
   
   uint32 nbrBefore = featureMap->getNbrFeatures();
   featureMap->setScaleLevel( m_maxScaleLevel );

   uint32 startTime = TimeUtility::getCurrentMicroTime();
   FilterSettings filterSettings;

   set<uint32> items;
   set<ItemTypes::itemType> onlyTheseItems;
   onlyTheseItems.insert( ItemTypes::pointOfInterestItem );
   
   m_map->getIDsWithinBBox( items,
                            *m_bbox,
                            onlyTheseItems );

   set<uint32>::const_iterator it = items.begin();
   set<uint32>::const_iterator itEnd = items.end();
   for (; it != itEnd; ++it ) {

      Item* curItem = m_map->itemLookup( *it ); 

      if ( curItem == NULL ||
           GET_ZOOMLEVEL( curItem->getID() ) != ItemTypes::poiiZoomLevel ) {
         continue;
      }

      byte extraPOIInfo = MAX_BYTE;
      PointOfInterestItem* poi = NULL;
      // Whether this poi type should be added to the map or not.
      bool poiTypeOK = false;
      if (curItem->getItemType() == ItemTypes::pointOfInterestItem) {
         poi = static_cast< PointOfInterestItem* > ( curItem );
         if ( m_mapSettings->getPOI( poi->getPointOfInterestType() )) {
            // This poi type should be added.
            poiTypeOK = GfxFeatureMapUtility::
               checkPOIExtra( *poi, *m_map, *m_mapSettings,
                              true ); // include pois that has no specific right
            
            if ( poiTypeOK && poi->isPointOfInterestType( ItemTypes::cityCentre ) 
                 && m_mapSettings->getShowCityCentres() ) {
               // Avoid duplicated city centres
               poiTypeOK = false;
            }
         }
      }
         
      // Make sure that this POI type should be added to the map.
      if ( ! poiTypeOK ) {
         continue;
      }
      
      // Get POI coordinates
      int32 lat, lon;
      if ( curItem->getGfxData() == NULL ) {
         m_map->getItemCoordinates( curItem->getID(), 0, lat, lon );
      } else {
         lat = curItem->getGfxData()->getLat(0,0);
         lon = curItem->getGfxData()->getLon(0,0);
      }
      // The poi is located in the map.

      // fetch special image name

      MC2String imageName;
      if ( poi ) {
         GfxFeatureMapUtility::
            getExtraInfoForPOI( *poi, *m_map, 
                                extraPOIInfo, imageName );
      }
            
      // Get the scalelevel
      uint32 scaleLevel = MapUtility::toDrawItem( curItem, m_bbox,
                                                  m_screenX,
                                                  m_screenY,
                                                  false, // countrymap
                                                  false, // usestreets
                                                  NULL,  // the map
                                                  lat,
                                                  lon,
                                                  false, // alwaysIncludedFromCountryMap
                                                  extraPOIInfo );

      // Special for turkish hospitals - change type
      if ( poi &&
           ( poi->getPointOfInterestType() == ItemTypes::hospital ) &&
           ( m_map->getCountryCode() == StringTable::TURKEY_CC ) ) {
         extraPOIInfo = 1;
      }


      // Make sure the poi should be included 
      if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
           (scaleLevel >= uint32(m_minScaleLevel)) ) {
         // XXX: Post office and cafe should only be shown
         //  if they have a special image.
         //  The configuration of this needs to be fixed in a better way!
         if ( poi &&
              ( poi->getPointOfInterestType() == ItemTypes::postOffice ||
                poi->getPointOfInterestType() == ItemTypes::cafe ) &&
              imageName.empty() ) {
            // skip this poi
            continue;
         }

         MapUtility::getFilterSettings(&filterSettings, curItem,
                                       m_filtScaleLevel);
         GfxFeature* feature = 
            createGfxFeatureFromItem(curItem, 
                                     &filterSettings,
                                     GfxFeature::NBR_GFXFEATURES,
                                     true,
                                     lat,
                                     lon,
                                     p->getLanguage(),
                                     extraPOIInfo,
                                     -1, -1,
                                     imageName.empty() ? 
                                     NULL : imageName.c_str() );
         if ( feature != NULL ) {
            feature->setScaleLevel( scaleLevel );
            featureMap->addFeature(feature);
            DEBUG8(feature->dump(10));
         }
      }
   }
   
   mc2dbg4 << featureMap->getNbrFeatures() - nbrBefore 
           << " items selected and added in " 
           << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0 << " ms" 
           << endl
           << "Number splits are " << nbrsplits << endl
           << "Number coords are " << nbrcoords << endl
           << "Size of GfxFeatureMap = " << featureMap->getMapSize()
           << endl;

}

void
GfxFeatureMapProcessor::createCityCentreGfxFeatureMap(
   const GfxFeatureMapRequestPacket* packet,
   GfxFeatureMap* featureMap,
   MC2BoundingBox& bbox)
{
   mc2dbg2 << "[GFMP]: createCityCentreGfxFeatureMap" << endl;
   mc2dbg8 << " map=" << m_map->getMapID() << " requested lang="
           << LangTypes::getLanguageAsString(
              packet->getLanguage(), true) 
           << endl;
   featureMap->setScaleLevel( m_maxScaleLevel );
   FilterSettings filterSettings;
   
   // If ordinary map or only to draw the countryoverview map(s).
   if ( (MapBits::isUnderviewMap(m_map->getMapID())) || 
        (packet->getDrawOverviewContents() ) ) {
      // City centres are by default on zoom 14 poiiZoomLevel
      // but in co maps, seom city centres are also on zoom 1
      set<uint32> zoomlevels;
      zoomlevels.insert(ItemTypes::poiiZoomLevel);
      if ( MapBits::isCountryMap(m_map->getMapID()) ) {
         zoomlevels.insert(1);
      }
      for ( set<uint32>::const_iterator it = zoomlevels.begin();
            it != zoomlevels.end(); it++) {
         // Add city centres
         for (uint32 i = 0; 
              i < m_map->getNbrItemsWithZoom( *it ); 
              i++) {
            Item* curItem = m_map->getItem(*it, i);
            if (curItem == NULL) {
               continue;
            }
                              
            // Add the city centres
            PointOfInterestItem* poiItem =
               static_cast<PointOfInterestItem*>( curItem );

            if( poiItem->getPointOfInterestType() != ItemTypes::cityCentre )
            {
               continue;
            }

            uint32 waspID = MAX_UINT32;
            if ( m_mapSettings->getPOI( poiItem->getPointOfInterestType() ))
            {
               // This poi type should be added.
               if ( poiItem->isPointOfInterestType(
                       ItemTypes::cityCentre ) ) {
                  waspID = poiItem->getWASPID();
               }
            } else {
               continue;
            }
            
            // Get POI coordinates
            int32 lat, lon;
            if (curItem->getGfxData() == NULL) {
               m_map->getItemCoordinates(curItem->getID(), 0,
                                         lat, lon);
            } else {
               lat = curItem->getGfxData()->getLat(0,0);
               lon = curItem->getGfxData()->getLon(0,0);
            }
            // The poi is located in the map.
            
            // Get the display class for citycentres.
            byte extraPOIInfo = MAX_BYTE;
            if ( waspID != MAX_UINT32 ) {
               getDisplayClassForCC( waspID, extraPOIInfo );
               extraPOIInfo = adjustPOIInfoForCountry(
                  extraPOIInfo,
                  m_map->getCountryCode(),
                  ItemTypes::cityCentre );
            }

            // Get the scalelevel
            uint32 scaleLevel = 
               MapUtility::toDrawItem( curItem, &bbox,
                                       m_screenX,
                                       m_screenY,
                                       false, // countrymap
                                       false, // usestreets
                                       NULL,  // the map
                                       lat,
                                       lon,
                                       false, // alwaysIncludedFromCountryMap
                                       extraPOIInfo );
                    
            // Make sure the poi should be included 
            if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
                 (scaleLevel >= uint32(m_minScaleLevel)) ) {
    
               MapUtility::getFilterSettings(&filterSettings,
                                             curItem,
                                             m_filtScaleLevel);
               GfxFeature* feature = 
                  createGfxFeatureFromItem( curItem, 
                                            &filterSettings,
                                            GfxFeature::NBR_GFXFEATURES,
                                            true,
                                            lat,
                                            lon,
                                            packet->getLanguage(),
                                            extraPOIInfo );
               if ( feature != NULL ) {
                  feature->setScaleLevel( scaleLevel );
                  featureMap->addFeature(feature);
                  DEBUG8(feature->dump(10));
               }
            }
         }
      }
   }
}

uint32 
GfxFeatureMapProcessor::getNumberCountryPolysToSend( CountryOverviewMap* comap )
{
   const GfxData* mapGfx = comap->getGfxData();
   uint32 nbrToSend = mapGfx->getNbrPolygons();
   
   // The polygons being sorted on nbrCoordinates, we can accept 
   // to skip(send) 5 small polys with many coords, in order to 
   // send larger polygons with fewer coordinates.
   // Code copied from GfxCountryMapGenerator
   uint32 minPolyLength = 100000;
   if (m_maxScaleLevel <= 2)  // county
      minPolyLength = 70000;
   else
      minPolyLength = 35000;
   uint32 nbrPolygons = 0;
   uint32 maxNbrSkipPolys = 5;
   uint32 skipPolys[maxNbrSkipPolys];
   for (uint32 i = 0; i < maxNbrSkipPolys;i++) {
      skipPolys[i] = MAX_UINT32;
   }
   
   bool cont = true;
   uint32 nbrSkip = 0;
   while (cont && (nbrPolygons < mapGfx->getNbrPolygons())) {
      mc2dbg8 << "poly " << nbrPolygons << " nbrC=" 
              << mapGfx->getNbrCoordinates(nbrPolygons)
              << " length=" << mapGfx->getLength(nbrPolygons);
      if (mapGfx->getLength(nbrPolygons) < minPolyLength) {
         skipPolys[nbrSkip] = nbrPolygons;
         mc2dbg8 << "  - \"skipping\" ";
         nbrSkip++;
         if (nbrSkip >= maxNbrSkipPolys) {
            cont = false;
         }
      }
      mc2dbg8 << endl;
      nbrPolygons++;
   }
   // Don't send if any of the small polygons are the last ones..
   uint32 n = maxNbrSkipPolys;
   while ( (n > 0) && (skipPolys[n-1] >= nbrPolygons-1)) {
      if (skipPolys[n-1] == nbrPolygons-1) {
         mc2dbg8 << " skip poly in the end of nbrPolygons" << endl;
         nbrPolygons--;
      }
      n--;
   }
   
   nbrToSend = MAX(nbrPolygons, 10);
   nbrToSend = MIN(nbrToSend, mapGfx->getNbrPolygons());
   mc2dbg4 << "Sending " << nbrToSend << " polygons of "
           << mapGfx->getNbrPolygons() << " for " 
           << comap->getMapName() << endl;

   return nbrToSend;
}

void
GfxFeatureMapProcessor::
createCountryFeatureMap( GfxFeatureMap* gfxFeatureMap,
                         const GfxFeatureMapRequestPacket* p ) {
   FilterSettings filterSettings;
   // Country overview map.
   // Check if we are to include LAND features and if the country polygon 
   // of this country should be included from this country map
   // Add not more than 10 land polygons. NO! add more.

   if ( includeFeatureType( GfxFeature::LAND )  &&
        p->getIncludeCountryPolygon() ) {

      DebugClock clock;

      CountryOverviewMap* comap = 
         static_cast<CountryOverviewMap*>(m_map);
         
      // LAND feature
      // 1. create feature (name and type)
      GfxFeature* landFeature = createLandFeature( comap, p );
      // 2. find out how many polys to include
      uint32 nbrToSend = getNumberCountryPolysToSend( comap );
      // 3. add polys to feature
      for(uint16 i=0;
          i < comap->getGfxData()->getNbrPolygons() && i < nbrToSend;
          i++) {
         addCountryPolygon( comap, landFeature, i );
      }
      // now we add the centroid to the last feature
      if ( nbrToSend >= 1 &&
           comap->getGfxData()->getNbrPolygons() >= 1 ) {

         if ( ! m_landCentroid.isValid() ) {
            // Get land centroid from a table, if we have any for this feature.
            MC2Coordinate landCentroid =
               CentroidCalculation::getCentroidForLandFeature( *landFeature );
            if ( ! landCentroid.isValid() ) {
               // select the first polygon here,
               // TODO: maybe this should select the largest poly instead.
               comap->getGfxData()->getPolygonCentroid( 0, m_landCentroid );
            } else {
               m_landCentroid = landCentroid;
            }
         } 
         landFeature->addNewPolygon( true, 1 );
         landFeature->addCoordinateToLast( m_landCentroid );
      }

      mc2dbg4 << "LAND-feature " << landFeature->getName()
              << " created in " <<  clock << endl;
               
      // Add the LAND-feature to the GfxFeatureMap
      gfxFeatureMap->addFeature(landFeature);

      // Add borderItems = BORDER-features
      uint32 nbrBorders = createAndAddBorders( gfxFeatureMap );
      mc2dbg4 << "Added " << nbrBorders << " BORDER features for co map "
              << m_map->getMapName() << endl;
   }

   // Add water items. Present at zoomlevel 1.
   for (uint32 i = 0; i < m_map->getNbrItemsWithZoom(1); i++) {
      Item* curItem = m_map->getItem(1, i);
      if ( (curItem != NULL) && 
           (curItem->getGfxData() != NULL) ) {
         uint32 scaleLevel = 
            MapUtility::toDrawItem( curItem, m_bbox,
                                    m_screenX, m_screenY,
                                    true,          // countryMap
                                    false,         // useStreets (default)
                                    NULL,          // theMap (default)
                                    MAX_INT32, MAX_INT32,     // lat, lon
                                    true );        // always included from
                                                   // country map.
            
         if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
              (scaleLevel >= uint32(m_minScaleLevel)) ) {
            MapUtility::getFilterSettings(&filterSettings, curItem,
                                          m_filtScaleLevel);
            GfxFeature* feature = 
               createGfxFeatureFromItem(curItem, &filterSettings);
            if ( feature != NULL )  {
               feature->setScaleLevel( scaleLevel );
               gfxFeatureMap->addFeature(feature);
            }
         }
      }
   }

}

void
GfxFeatureMapProcessor::
createUnderviewFeatureMap( GfxFeatureMap* gfxFeatureMap,
                           const GfxFeatureMapRequestPacket* p ) {

   FilterSettings filterSettings;
   // Get the items to include on the map

   set<uint32> items;
   m_map->getIDsWithinBBox( items, 
                            *m_bbox,
                            set<ItemTypes::itemType>() );// all items are allowed

   // now we should have everything we need
   set<uint32>::const_iterator it = items.begin();
   set<uint32>::const_iterator itEnd = items.end();
   for (; it != itEnd; ++it ) {
      Item* curItem = m_map->itemLookup( *it ); 
      if ( curItem == NULL || curItem->getGfxData() == NULL ) {
         continue;
      }

      // We don't want to extract poi:s in this method.
      if ( GET_ZOOMLEVEL( curItem->getID() ) == ItemTypes::poiiZoomLevel ) {

         continue;
      }
      uint32 scaleLevel = 
         MapUtility::toDrawItem( curItem, m_bbox,
                                 m_screenX, m_screenY,
                                 MapBits::isCountryMap(m_map->getMapID()));

      if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
           (scaleLevel >= uint32(m_minScaleLevel)) ) {

         MapUtility::getFilterSettings(&filterSettings, curItem,
                                       m_filtScaleLevel);
         GfxFeature* feature = 
            createGfxFeatureFromItem( curItem, &filterSettings,
                                      GfxFeature::NBR_GFXFEATURES,
                                      true, MAX_INT32, MAX_INT32,
                                      p->getLanguage() );



         // would like language_t instead of languageCode
         // if possible..
         if ( feature != NULL )  {
            // setup centroid point for large areas
            CentroidCalculation::addCentroidForAreas( *feature, *curItem );
            feature->setScaleLevel( scaleLevel );
            gfxFeatureMap->addFeature(feature);
         }
      }          
   }


}

void
GfxFeatureMapProcessor::createGfxFeatureMap(GfxFeatureMap* gfxFeatureMap,
                                            const GfxFeatureMapRequestPacket* p)
{
   mc2dbg8 << " map=" << m_map->getMapID() << " requested lang="
        << LangTypes::getLanguageAsString( p->getLanguage(), true ) 
        << endl;

   uint32 nbrBefore = gfxFeatureMap->getNbrFeatures();
   DebugClock clock;

   gfxFeatureMap->setScaleLevel( m_maxScaleLevel );
   
   // Check if this is the countryoverviewmap or an ordinary map.
   if (MapBits::isCountryMap(m_map->getMapID())) {
      createCountryFeatureMap( gfxFeatureMap, p );
   }
   
   // If ordinary map or only to draw the countryoverview map(s).
   if ( (MapBits::isUnderviewMap(m_map->getMapID())) || 
        (p->getDrawOverviewContents() ) ) {
      createUnderviewFeatureMap( gfxFeatureMap, p );
   }

   mc2dbg4 << gfxFeatureMap->getNbrFeatures() - nbrBefore 
           << " items selected and added in " 
           << clock << endl
           << "Number splits are " << nbrsplits << endl
           << "Number coords are " << nbrcoords << endl
           << "Size of GfxFeatureMap = " << gfxFeatureMap->getMapSize()
           << endl;

  
}


GfxFeatureMapReplyPacket* 
GfxFeatureMapProcessor::generateGfxFeatureMap(const GfxFeatureMapRequestPacket* p)
{
   DebugClock functionClock;

   mc2dbg2 << "GfxFeatureMapProcessor::generateGfxFeatureMap()" << endl;
   
   GfxFeatureMapReplyPacket* reply = new GfxFeatureMapReplyPacket(p);
   reply->setMapID( p->getMapID() );
      

   if (m_map == NULL) {
      mc2log << warn << here << " m_map == NULL" << endl;
      reply->setStatus(StringTable::MAPNOTFOUND);
      return (reply);
   }
   
   // Copyright
   reply->setCopyright( m_map->getCopyrightString() );

   // Init the other members with info from the packet.
   init(p);

   // Create the map that will be filled with data
   GfxFeatureMap gfxFeatureMap;
   
   MC2BoundingBox bbox;
   p->getMC2BoundingBox(&bbox);
     
   gfxFeatureMap.setMC2BoundingBox(&bbox);
   // p->getScreenSize( m_screenX, m_screenY ); // Done in init(p)
   gfxFeatureMap.setScreenSize( m_screenX, m_screenY );

   bool extractMap = m_mapSettings->getShowMap();
   bool extractRoute = m_mapSettings->getShowRoute() && p->containsRoute();
   bool extractPOI = m_mapSettings->getShowPOI();
   bool extractCityCentres = m_mapSettings->getShowCityCentres();
   
   // Extract map?
   if (extractMap) {
      // Extract map.
      m_maxOneCoordPerPixel = 
         m_mapSettings->getMaxOneCoordPerPixelForMap();
      mc2dbg2 << "  Extracting map" << endl
              << "  m_maxOneCoordPerPixel = " << m_maxOneCoordPerPixel 
              << endl;
      createGfxFeatureMap(&gfxFeatureMap, p);
   }
   
   // Extract route?
   if (extractRoute) {
      // Extract route
      m_maxOneCoordPerPixel = 
         m_mapSettings->getMaxOneCoordPerPixelForRoute();
      mc2dbg2 << "  Extracting route" << endl
              << "  m_maxOneCoordPerPixel = " << m_maxOneCoordPerPixel 
              << endl;
      createRouteGfxFeatureMap(p, &gfxFeatureMap);
   } 
   
   // Extract poi:s
   if( extractPOI ) {
      m_maxOneCoordPerPixel = false;
      mc2dbg2 << "  Extracting poi:s" << endl;
      createPOIGfxFeatureMap( p, &gfxFeatureMap );
   }

   // Extract city centres
   if( extractCityCentres ) {
      m_maxOneCoordPerPixel = false;
      mc2dbg2 << "  Extracting city centres" << endl;
      createCityCentreGfxFeatureMap( p, &gfxFeatureMap, bbox );
   }
   
   // Store the gfxFeaturemap into the replypacket.
   DataBuffer buf( gfxFeatureMap.getMapSize() );
   gfxFeatureMap.save( &buf );
   //gfxFeatureMap->dump(0);
   //gfxFeatureMap->printStatistics(1);
   
   const uint32 EXTRALENGTH = 200;
   if ( (gfxFeatureMap.getMapSize()+EXTRALENGTH) > reply->getBufSize()) {
      reply->resize(gfxFeatureMap.getMapSize() + 
                    REPLY_HEADER_SIZE + EXTRALENGTH);
   }
   reply->setGfxFeatureMapData( buf.getCurrentOffset(), &buf );


   mc2dbg8 << here << " Dumping GfxFeature map that will be sent:" << endl;
   DEBUG8( gfxFeatureMap.dump(2); );
   
   mc2dbg << "Took " << functionClock << " to "
          << "generate the gfxFeature map." << endl;
   
   reply->setStatus( StringTable::OK );

   return reply;
}

byte
GfxFeatureMapProcessor::adjustPOIInfoForCountry(
                     byte extraPOIIfo, StringTable::countryCode country,
                     ItemTypes::pointOfInterest_t poiType)
{
   // Increase importance of city centres in countries where no big
   // cities exists.
   // Change the middle-important cities wit displayclass 7 and 8 to be 5 and 7
   // Don't change minor cities.

   byte retVal = extraPOIIfo;

   if ( (poiType == ItemTypes::cityCentre ) &&
        (
         (country == StringTable::SWEDEN_CC) ||
         (country == StringTable::DENMARK_CC) ||
         (country == StringTable::NORWAY_CC) ||
         (country == StringTable::FINLAND_CC) )) {
      if (extraPOIIfo == 7)
         retVal = 5;
      else if (extraPOIIfo == 8)
         retVal = 7;
   }

   return retVal;
}




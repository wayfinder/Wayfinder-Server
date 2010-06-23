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
#include "GfxTileFeatureMapProcessor.h"

#include "AbbreviationTable.h"
#include "MapUtility.h"
#include "TileMapUtility.h"
#include "TileMapFormatDesc.h"
#include "TileMapProperties.h"
#include "StreetSegmentItem.h"
#include "MapSettings.h"
#include "GfxFeatureMapPacket.h"
#include "GfxFeatureMap.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "CountryOverviewMap.h"
#include "FilterSettings.h"
#include "PointOfInterestItem.h"
#include "GfxFeatureMapUtility.h"

#include "GfxData.h"
#include "GfxDataFull.h"
#include "Properties.h"
#include "WaterItem.h"
#include "MapFilterUtil.h"
#include "Node.h"
#include "StreetItem.h"
#include "GzipUtil.h"
#include "GfxConstants.h"
#include "DrawingProjection.h"
#include "DebugClock.h"
#include "MapBits.h"
#include "NodeBits.h"
#include "FeatureName.h"

static int nbrsplits = 0;
static int nbrcoords = 0;

GfxTileFeatureMapProcessor::GfxTileFeatureMapProcessor(GenericMap* theMap)
   : GfxFeatureMapProcBase( *theMap )
{
   m_bbox = new MC2BoundingBox;
   m_mapSettings = NULL;
   m_mapFilterUtil = new MapFilterUtil;
}

GfxTileFeatureMapProcessor::~GfxTileFeatureMapProcessor()
{
   delete m_bbox;
   delete m_mapSettings;
   delete m_mapFilterUtil;
}

void
GfxTileFeatureMapProcessor::init(const GfxFeatureMapRequestPacket* p) 
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
  
   //if ( ! m_concaveGfxClipper ) {
   if ( true || ! m_concaveGfxClipper ) {
      extraDist = int32(float64(m_bbox->getHeight() / m_screenY) * 10);
   }
   //extraDist = -extraDist;
   extraDist = 0;
  
   m_coordIncludeBBox.setMaxLat(m_bbox->getMaxLat() + extraDist); 
   m_coordIncludeBBox.setMinLat(m_bbox->getMinLat() - extraDist); 
   m_coordIncludeBBox.setMaxLon(m_bbox->getMaxLon() + extraDist); 
   m_coordIncludeBBox.setMinLon(m_bbox->getMinLon() - extraDist); 

   // Calculate and set the scalelevel of this map.
   m_maxScaleLevel = p->getMaxScaleLevel();
   m_minScaleLevel = p->getMinScaleLevel();
   m_filtScaleLevel = p->getFiltScaleLevel();

   // DEBUG: Print the current scalelevel.
   mc2dbg2 << "GMP: Current scalelevel is " 
           << m_maxScaleLevel
           << endl;
   
   // Set draw item parameters.
   MapUtility::makeDrawItemParameters( m_screenX, m_screenY, *m_bbox,
                                       m_xScaleFactor, m_yScaleFactor );
   
   mc2dbg4 << "ScaleLevels: max = " << m_maxScaleLevel
           << ", min = " << m_minScaleLevel << ", filt = "
           << m_filtScaleLevel << endl;
   
   DEBUG4(
   mc2dbg4 << "m_bbox:" << endl;
   m_bbox->dump(););
   DEBUG2(
   mc2dbg2 << "m_coordIncludeBBox:" << endl;
   m_coordIncludeBBox.dump(););

   nbrsplits = 0;
   nbrcoords = 0;

   // Reset the outcodes.
   m_prevCSOutcode = m_currCSOutcode = m_nextCSOutcode = 0;
}

inline bool
GfxTileFeatureMapProcessor::includeFeatureType(
                                  GfxFeature::gfxFeatureType type ) const
{
   // The setting is from MapSettings::createDefaultTileMapSettings
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
GfxTileFeatureMapProcessor::includeCoordinate( int32& lat, int32& lon )
{
   // Always include the coordinate for now.
   return true;
   // Check if the coordinate should be included.
   
   if ( (m_prevCSOutcode & m_currCSOutcode & m_nextCSOutcode) == 0 ) {
      // Include coordinate!
      
      return (true);
   } else {
      // Should not be included.
      return (false);
   }
}

uint32 
GfxTileFeatureMapProcessor::getNumberCountryPolysToSend( CountryOverviewMap* comap )
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
   mc2dbg << "Sending " << nbrToSend << " polygons of "
          << mapGfx->getNbrPolygons() << " for " 
          << comap->getMapName() << endl;

   return nbrToSend;
}

GfxFeature*
GfxTileFeatureMapProcessor::createCountryBorder(
                                            const GfxFeatureMapRequestPacket* p,
                                            CountryOverviewMap* cmap )
{
   const char* countryName = StringTable::getStringToDisplayForCountry(
      m_map->getCountryCode(), 
      ItemTypes::getLanguageTypeAsLanguageCode( p->getLanguage() ) );
   if (countryName == NULL) {
      countryName = StringTable::getStringToDisplayForCountry(
                        m_map->getCountryCode(), StringTable::ENGLISH );
      mc2dbg4 << "countryName == NULL, using " << countryName 
              << ", language=" << uint32(p->getLanguage()) << endl;
   }
   
   GfxFeature* feature = new GfxFeature(
      GfxFeature::LAND, countryName);
   // The country should always be visible 
   feature->setScaleLevel( CONTINENT_LEVEL );

   return feature;
}

float64
GfxTileFeatureMapProcessor::getTileScale()
{
   // The tile scale is a constant. One tile is drawn as long as the map 
   // scale is less than tileScale (e.g. a tile with scale 18.9 is
   // used until we zoom out more than map scale 19)
   // (bbox = one tile, screen = map display)
   //
   // Examples made for screenY = 200 - use as constant
   //
   // detail level 0 = tile scale 6.3
   // detail level 1 = tile scale 18.9
   // detail level 2 = tile scale 56.7
   // detail level 3 = tile scale 170.1
   // detail level 4 = tile scale 510.3
   // detail level 5 = tile scale 1530.9
   // detail level 6 = tile scale 4592.7
   // detail level 7 = tile scale 13778.2
   // detail level 8 = tile scale 41333.x
   
   float64 meters = m_bbox->getHeight() * GfxConstants::MC2SCALE_TO_METER;
   float64 tileScale = TileMapFormatDesc::getScale(meters, 200 /*m_screenY*/);
   
   return tileScale;
}

uint32
GfxTileFeatureMapProcessor::getCoPolFilterLevel( float64 tileScale )
{
   uint32 coPolFilterLevel = 0;
   if (tileScale < 12) {         // map scale to 12
      //coPolFilterLevel = 1;
      coPolFilterLevel = 2;
   }
   else if (tileScale < 30) {    // map scale to 30 (det 1)
      coPolFilterLevel = 4;
   }
   else if (tileScale < 100) {   // map scale to 100 (det 2)
      coPolFilterLevel = 6;
   }
   else if (tileScale < 300) {   // map scale to 300 (det 3)
      //coPolFilterLevel = 8;
      coPolFilterLevel = 7;
   }
   else if (tileScale < 800) {   // map scale to 800 (det 4)
      //coPolFilterLevel = 9;
      coPolFilterLevel = 8;
   }
   else if (tileScale < 1600) {  // map scale to 1600 (det 5)
      //coPolFilterLevel = 10;
      coPolFilterLevel = 9;
   }
   else if (tileScale < 2800) {  // map scale to 2800
      coPolFilterLevel = 11;  // ? ingen tile detail level
   }
   else if (tileScale < 5000) { // det 6
      //coPolFilterLevel = 12; 
      coPolFilterLevel = 11;
   }
   else if (tileScale < 15000) {
      coPolFilterLevel = 14;
   }
   else {
      //coPolFilterLevel = 15;
      coPolFilterLevel = 14;
   }

   if ( isHighEndDevice() && coPolFilterLevel > 1 ) {
      // For high end devices show a more detailed polygon
      if ( coPolFilterLevel >=4 && coPolFilterLevel < 14 ) {
         coPolFilterLevel -= 2;
      } else {
         coPolFilterLevel--;
      }
   }

   return coPolFilterLevel;
}

bool
GfxTileFeatureMapProcessor::addCountryPolygon( const CountryOverviewMap& cmap,
                                               GfxFeature* feature, 
                                               int polynumber )
{
   // The map gfx data was not filtered in levels...
   // use the pre-stored gfx filterings of the co map country polygon
   // Now crash
   MC2_ASSERT( cmap.mapGfxDataIsFiltered() );
   
   
   // If the map gfx data of this country map was filtered in levels,
   // get the gfxdata for the land feature from a good filter level
   //
   const GfxData& gfx = *cmap.getGfxData();
   
   // Decide country filter level, depends on the tile scale
   float64 tileScale = getTileScale();
   
   uint32 coPolFilterLevel = getCoPolFilterLevel( tileScale );
   
   // Build gfx data for the land feature from the chosen filter level
   // Small polys from the country are not interesting to show..
   // Always add poly 0!
   // Polygons are included by createGfxFeatureMap if length of
   // poly is > 35000 for m_maxScaleLevel > 2 (county), and if 
   // length > 70000 for maxScale <= 2 (county)
   bool includePolygon = true;
   if ( (polynumber != 0) && (tileScale > 4500) ) { // detail level 6++
      int filterDist = 
         m_mapFilterUtil->getFiltDistanceForMapGfx(coPolFilterLevel);
      // get rid of some small islands by increasing the distance...
      filterDist *= 2;
      if ( gfx.getLength(polynumber) < filterDist ) {
         // don't add
         includePolygon = false;
      }
   }

   if ( includePolygon ) {
      auto_ptr<GfxDataFull> 
         landGfx( GfxData::createNewGfxData( NULL, true ) );
      // reserve worst case number of coordinates
      landGfx->reserveCoordinates( landGfx->getNbrPolygons() - 1, 
                                   gfx.getNbrCoordinates( polynumber ) );

      landGfx->addCoordinates( gfx.beginFilteredPoly( polynumber, 
                                                       coPolFilterLevel ),
                               gfx.endFilteredPoly( polynumber, 
                                                     coPolFilterLevel ) );
      landGfx->setClosed( 0, gfx.getClosed( polynumber ) );
      // The filtered polygon may contain only 1 or 2 coords, then don't add
      // ?? Small countries will not be drawn ok...
      if ( landGfx->getNbrCoordinates(0) >= 3 ) {
         addGfxDataToFeature(feature, landGfx.get(), NULL, 0);
      }
      DEBUG8(
             if ( polynumber == 0 ) {
                mc2dbg << "country polygon level=" << coPolFilterLevel 
                       << " nbrCoords="
                       << landGfx->getNbrCoordinates(0) << " of "
                       << gfx.getNbrCoordinates(0) << endl;
             });
   }
   
   // Return if any polygons are present in the feature.
   return ( feature->getNbrPolygons() > 0 );

}

bool
GfxTileFeatureMapProcessor::addGfxToBorderFeature(
                     GfxFeature* borderFeature, GfxData* gfx)
{
   // Decide filter level, depends on the tile scale
   float64 tileScale = getTileScale();
   uint32 coPolFilterLevel = getCoPolFilterLevel( tileScale );
   
   GfxDataFull* newGfx = GfxData::createNewGfxData( NULL, true );
   
   newGfx->addCoordinates( gfx->beginFilteredPoly( 0, coPolFilterLevel ),
                           gfx->endFilteredPoly( 0, coPolFilterLevel ) );
                           
   newGfx->setClosed( 0, gfx->getClosed( 0 ) );
   if ( newGfx->getNbrCoordinates(0) >= 2 ) {
      addGfxDataToFeature(borderFeature, newGfx, NULL, 0);
   }

   delete newGfx;
   
   return ( borderFeature->getNbrPolygons() > 0 );
}

uint32
GfxTileFeatureMapProcessor::createAndAddBorders(GfxFeatureMap* gfxFeatureMap)
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
   DebugClock clock;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<m_map->getNbrItemsWithZoom(z); i++) {
         Item* curItem = m_map->getItem(z, i);
         if ( (curItem != NULL) && 
              (curItem->getGfxData() != NULL) &&
              (curItem->getItemType() == ItemTypes::borderItem) ) {
            
            GfxFeature* borderFeature = 
                  new GfxFeature( GfxFeature::BORDER );
            borderFeature->setScaleLevel( CONTINENT_LEVEL );

            if ( addGfxToBorderFeature( 
                     borderFeature, curItem->getGfxData()) ) {
               addFeature( gfxFeatureMap, borderFeature );
               nbrBorders++;
            } else {
               delete borderFeature;
            }

         }
      }   
   }
   
   mc2dbg8 << "Added " << nbrBorders << " border features in "
           << clock << endl;
   
   return nbrBorders;
}

bool 
GfxTileFeatureMapProcessor::manyFeaturesOnPixel( GfxFeature* feature )
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
GfxTileFeatureMapProcessor::createGfxFeatureFromItem(
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
   char featNameBuf[1024];
   featNameBuf[0] = '\0';

   // Default is to use the buffer.
   const char* featNameToUse = featNameBuf;
   const char* basename = "";

   bool nameIsAbbreviated = false;
   LangTypes::language_t featNameLang = LangTypes::invalidLanguage;
   
   // Only add names, if present and we are not extracting crossing map.
   if ( (! m_mapSettings->getNavigatorCrossingMap()) && 
        (item->getNbrNames() > 0) ) {
      
      // Get the best name for item considering requested language.
      // (for ssi: use normal name before roadNumber)
      const char* tmpFeatName = "";

      uint32 rawindex = FeatureName::getItemNameIndex( *m_map, *item, lang );

      if (rawindex != MAX_UINT32) {
         // we have a name to use.
         tmpFeatName = m_map->getRawName(rawindex);
         featNameLang = GET_STRING_LANGUAGE( rawindex );
      }

      // Set basename for gfx data.
      // ( G-tiles, or T-tiles )
      // This extra name is used for a correct merge order for streets in the
      // tile module.
      // This is because some streets might have mismatching names between
      // languages. For example in taiwan there were two street items 
      // that had same name in english but two different names in chinese which
      // caused CRC-mismatch between T- and G-tiles.
      uint32 baseIndex = FeatureName::getItemNameIndex( *m_map, *item,
                                                        LangTypes::invalidLanguage);


      if ( baseIndex != MAX_UINT32 ) {
         basename = m_map->getRawName( baseIndex );
      } else {
         basename = NULL;
      }

      if ( basename == NULL ) {
         basename = tmpFeatName;
      }

      // Look up the name in the map.
#ifndef INCLUDE_ID_IN_FEATNAME
      if ( item->getItemType() == ItemTypes::streetSegmentItem ) {
#if 0         
         // A ssi, abbreviate the name.
         AbbreviationTable::abbreviate( tmpFeatName, featNameBuf,
                                        featNameLang,
                                        AbbreviationTable::beginningAndEnd );
         nameIsAbbreviated = true;
#else
         // Let the TileModule abbreviate the name instead.
         featNameToUse = tmpFeatName;
#endif
      } else {
         // Not a street. Don't abbreviate.
         featNameToUse = tmpFeatName;
         //strcpy( featNameBuf, tmpFeatName );
      }
#else
      sprintf(featNameBuf, "%s (%d)", tmpFeatName, item->getID());
#endif
   }

   mc2dbg4 << "createGfxFeatureFromItem " << featNameToUse << endl;
   
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
         feature = GfxFeature::createNewFeature(type, featNameToUse);
      } else {
         // Settings says this feature shouldn't be included.
         return (NULL);
      }
   } else {
      // No appropriate feature type
      return (NULL);
   } 
     
   feature->setLangType( featNameLang, nameIsAbbreviated );
   feature->setBasename( basename );

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
         

         // Calculate untouchable coordinates to be used as indata to 
         // filtering:
         //
         // first polygon index
         // second coordinate index
         multimap< uint32,uint32 > allSelfTouchCoords;
         // Perhaps you should remove identical coords first.
         if (gfx->getMultiCoords(allSelfTouchCoords)){
            mc2dbg8 << "Got multi coords" << endl;
         }
         mc2dbg8 << "allSelfTouchCoords:" << allSelfTouchCoords.size() 
                 << endl;
            

         for (uint32 poly=0; poly<nbrPolygons; poly++) {
            
            // Whether to add this polygon or not.
            bool add = true;

            if (item->getItemType() == ItemTypes::streetItem) {
               // Need to check that each polygon of the street item
               // should be added, since different street polygons can
               // have different roadclasses and importance.
               uint32 scaleLevel = TileMapUtility::toDrawStreetPolygon(
                              static_cast<StreetItem*> (item), poly);

               if ( (scaleLevel > uint32(m_maxScaleLevel)) || 
                    (scaleLevel < uint32(m_minScaleLevel)) ) {
                  add = false;
               }
            }
            
            if ( add ) {
               // Extract the self touch coords for this polygon.
               set<uint32> selfTouchCoords;
               multimap< uint32,uint32 >::const_iterator polyIt = 
                  allSelfTouchCoords.lower_bound( poly );
               multimap< uint32,uint32 >::const_iterator polyItEnd =
                  allSelfTouchCoords.upper_bound( poly );
               for ( ; polyIt != polyItEnd; ++polyIt ) {
                  selfTouchCoords.insert( polyIt->second );
               }

               addGfxDataToFeature(feature, gfx, filterSettings, poly,
                                   coordinateStartingAtNode0,
                                   startOffset, endOffset,
                                   &selfTouchCoords);
            }
         }
         
         // Don't add the feature if it only covers one pixel and there
         // are already a feature of the same type covering that pixel.
         // Don't add if it's not containing any polygons.
         if ( manyFeaturesOnPixel( feature ) || 
              feature->getNbrPolygons() == 0 ) {
            delete feature;
            return NULL;
         }
      }
   }
 
   // Boost the area for rivers.
   if ( ( item->getItemType() == ItemTypes::waterItem ) &&
        ( (static_cast<WaterItem*> ( item ))->getWaterType() ==
          ItemTypes::river ) ) {
      // River! Boost the area by a factor of 4.
      for ( uint32 i = 0; i < feature->getNbrPolygons(); ++i ) {
         GfxPolygon* poly = feature->getPolygon( i );
         poly->setArea( poly->getArea() * 4 );
      }
   }
 
   // Set the poi type for GfxFeature::POI
   if ((feature->getType() == GfxFeature::POI) && 
       (item->getItemType() == ItemTypes::pointOfInterestItem)) {
      ItemTypes::pointOfInterest_t poiType = 
         (static_cast<PointOfInterestItem*> (item))
            ->getPointOfInterestType();

      GfxPOIFeature* gfxPOI = static_cast<GfxPOIFeature*> (feature);
      gfxPOI->setPOIType( poiType );
      gfxPOI->setExtraInfo( extraPOIInfo );        
      // setup categories
      GenericMap::Categories categories;
      m_map->getCategories( *item, categories );
      gfxPOI->swapCategories( categories );

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
GfxTileFeatureMapProcessor::
addCoordsFromGfxDataToPointVector( const GfxData& gfx, 
                                   uint16 poly,
                                   const Stack* polyStack,
                                   vector<POINT>& vertices,
                                   bool node0,
                                   int startOffset,
                                   int endOffset )
{

   if ( ! node0 ) {
//      if ( startOffset == 0 ) {
//         startOffset = MAX_UINT16;
//      }
//      if ( endOffset == MAX_UINT16 ) {
//         endOffset = 0;
//      }
      std::swap( startOffset, endOffset );
   }   
   
   POINT p;
   if ( polyStack == NULL ) {

      mc2dbg8 << "[GTFMP]: startOffset = " << startOffset
              << ", endOffset = " << endOffset << endl;
      
      int startIndex = 0;   
      POINT startPoint;     
      
      if ( startOffset != -1 ) {
         MC2_ASSERT( poly == 0 );
         startIndex = gfx.getCoordinate( startOffset,
                                         startPoint.y, 
                                         startPoint.x );
         vertices.push_back( startPoint );
      }

      int endIndex = gfx.getNbrCoordinates( poly );
      
      POINT endPoint;
      if ( endOffset != -1 ) {
         MC2_ASSERT( poly == 0 );
         endIndex = gfx.getCoordinate( endOffset, 
                                       endPoint.y, 
                                       endPoint.x );
      }

      if ( endIndex < startIndex ) {
         // This is really an error - the above checks for node 0
         // should have taken care of that. Something seems to be
         // strange on one node routes.
         mc2log << error << "[GTFMP]: endIndex < startIndex - swapping "
                << endl;
         mc2dbg << "[GTFMP]: startOffset = " << startOffset << endl;
         mc2dbg << "[GTFMP]: endOffset = " << endOffset << endl;
         std::swap( endIndex, startIndex );
      }
      
      vertices.reserve( vertices.size() + endIndex - startIndex + 2 );
      {
         GfxData::const_iterator begin = gfx.polyBegin( poly ) + startIndex;
         GfxData::const_iterator end   = gfx.polyBegin( poly ) + endIndex;
         for ( GfxData::const_iterator it = begin;
               it != end;
               ++it ) {
            p.x = it->lon;
            p.y = it->lat;
            vertices.push_back( p );
         }
      }

      if ( endOffset != -1 ) {
         vertices.push_back( endPoint );
      }
      
   } else {
      // Add only the vertices present in polyStack.
      vertices.reserve( vertices.size() + polyStack->getStackSize() + 1 );
      for ( uint32 i = 0; i < polyStack->getStackSize(); ++i ) {
         p.x = gfx.getLon( poly, polyStack->getElementAt( i ) );
         p.y = gfx.getLat( poly, polyStack->getElementAt( i ) );
         vertices.push_back( p );
      }
   }
   if ( gfx.getClosed( poly ) ) {
      // Closed polygon.
      if ( vertices.front().x != p.x ||
            vertices.front().y != p.y ) {
         vertices.push_back( vertices.front() );
      }
   }
}

bool
GfxTileFeatureMapProcessor::
addFilteredGfxDataToFeature( GfxFeature* feature, 
                             const GfxData* gfx,
                             uint32 poly,
                             const Stack* polyStack,
                             bool forward,
                             int startOffset,
                             int endOffset )
{

   MC2BoundingBox bb;
   gfx->getMC2BoundingBox(bb);

   if ( ! bb.overlaps( m_coordIncludeBBox ) ) {
      return false;
   }
   
   vector< vector<POINT> > clippedPolygons(1);
   vector<POINT>& vertices = clippedPolygons.front();
   
   // Add the coordinates from the gfxdata to the point vector.
   addCoordsFromGfxDataToPointVector( *gfx,
                                      poly, 
                                      polyStack, 
                                      vertices,
                                      forward,
                                      startOffset,
                                      endOffset );
   if ( vertices.empty() ) {
      return false;
   }

   bool closedPolygon = gfx->getClosed( poly );
   bool allCoordsInside = bb.inside( m_coordIncludeBBox );   

   if ( ! allCoordsInside ) {
      if ( closedPolygon ) {
         // Closed polygon.
         
         // Select appropriate clipper according to m_concaveGfxClipper.
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
 
      } else {
         // Open polygon.
         GfxUtility::clipPolylineToBBox( m_coordIncludeBBox, 
                                         clippedPolygons );

      }
   }

   for ( vector< vector<POINT> >::iterator pt = 
         clippedPolygons.begin(); pt != clippedPolygons.end();
         ++pt ) {
      // Unique the polygons
      vector<POINT>::iterator newEnd = std::unique( pt->begin(),
                                                    pt->end() );
      if ( newEnd != pt->end() ) {
         mc2dbg8 << "[GTFMP]: Removed non-unique coords" << endl;
         pt->resize( std::distance( pt->begin(), newEnd ) );
      }
      // Also remove the last coordinate if it is the same as the first
      // for closed polygons since it will be re-added later.
      if ( closedPolygon && pt->size() > 1 && pt->front() == pt->back() ) {
         mc2dbg8 << "[GTFMP]: Removed last == first which will be readded"
                << endl;
         pt->resize( pt->size() - 1 );
      }
   }

   
   // Use 32 bit coordinates.
   bool coordinates16Bits = false;   
   uint16 firstNewPoly = feature->getNbrPolygons();
   // Add the coordinates.
   for ( vector< vector<POINT> >::iterator pt = 
         clippedPolygons.begin(); pt != clippedPolygons.end();
         ++pt ) {
      vector<POINT>& clippedVertices = *pt;
      uint32 nbrCoords = clippedVertices.size();
      if ( nbrCoords == 0 ) {
         continue;
      }
      
      if (forward)  {
         // Add in forward direction.
         vector<POINT>::iterator it = 
            clippedVertices.begin();
         addClippedFirstCoordinate( it->y, it->x, 
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
         addClippedFirstCoordinate( it->y, it->x, 
                                    feature,
                                    nbrCoords,
                                    coordinates16Bits);
         ++it;
         for (;it != clippedVertices.rend(); ++it) {
            addClippedCoordinate(it->y, it->x, feature);
         }
      }
      if ( closedPolygon ) {
         // Add the closed coordinate.
         addClosedCoordinate( feature );
      }
   }

   if ( closedPolygon ) {
      // Set the area of the new polygons.
      float64 area = fabs( gfx->polygonArea( poly ) *  
            GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER );

      for ( uint16 j = firstNewPoly; j < feature->getNbrPolygons(); ++j ) {
         feature->getPolygon( j )->setArea( area );
      }
   }
   
   // Successful if there is at least one polygon
   return (feature->getNbrPolygons() > 0);
}

bool
GfxTileFeatureMapProcessor::addGfxDataToFeature(GfxFeature* feature, 
                                                const GfxData* gfx,
                                                const FilterSettings* settings,
                                                uint32 poly,
                                                bool coordinateStartingAtNode0,
                                                int startOffset,
                                                int endOffset,
                                                const 
                                                set< uint32 > *
                                                selfTouchCoords)
{
   mc2dbg4 << " addGfxDataToFeature " << endl;

   if (settings != NULL) {
      switch ( settings->m_filterType) {
         case FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER:
         case FilterSettings::OPEN_POLYGON_FILTER :
         case FilterSettings::CLOSED_POLYGON_FILTER : {
            mc2dbg8 << "Filtering GfxData for feature of type " 
                    << feature->getType() << endl;
            // Create the stack to filter with
            Stack polyStack(gfx->getNbrCoordinates(poly)/10);
            polyStack.reset();

            // Filter the polygon
            bool stackFilled = false;

            // Filter the polygon
            if (settings->m_filterType == 
                  FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER) {
               stackFilled = 
                  gfx->douglasPeuckerPolygonFilter( polyStack, 
                                                    poly, 
                                                    settings->m_maxLatDist );
            } else {
               stackFilled = ((GfxData*) gfx)->openPolygonFilter(
                                        &polyStack, 
                                        poly, 
                                        settings->m_maxLatDist,
                                        settings->m_maxWayDist, true);
            }
            // Add the coordinates
            if (stackFilled) {

               Stack* polyStackPointer = &polyStack;

               // Keep also self touching coords.
               Stack polyStack2;
               if ( selfTouchCoords != NULL ){
                  set<uint32> coordsToKeep;
                  coordsToKeep = *selfTouchCoords;
                  for ( uint32 i=0; i<polyStack.getStackSize(); i++){
                     coordsToKeep.insert(polyStack.getElementAt(i));
                  }
                  for ( set<uint32>::const_iterator coordIdxIt = 
                           coordsToKeep.begin(); 
                        coordIdxIt != coordsToKeep.end(); coordIdxIt++ ){
                     polyStack2.push(*coordIdxIt);
                  }
                  polyStackPointer = &polyStack2;
               }

               addFilteredGfxDataToFeature(feature, gfx, poly, 
                                           polyStackPointer, 
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

         default :
            // Add all the coordinates
            addFilteredGfxDataToFeature(feature, gfx, poly, NULL, 
                                        coordinateStartingAtNode0,
                                        startOffset, endOffset );
      }
   } else {
      // Settings == NULL
      addFilteredGfxDataToFeature(feature, gfx, poly, NULL, 
                                  coordinateStartingAtNode0,
                                  startOffset, endOffset );
   }


   // Successful if there is at least one polygon
   return (feature->getNbrPolygons() > 0);
}

void
GfxTileFeatureMapProcessor::addClippedFirstCoordinate(int32 lat, int32 lon,
                                                      GfxFeature* feature,
                                                      uint32 approxNbrCoord,
                                                      bool coordinates16Bits)
{
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
GfxTileFeatureMapProcessor::addClippedCoordinate(int32 lat, int32 lon,
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
GfxTileFeatureMapProcessor::
addCoordinateToFeature( GfxFeature* feature, 
                        int latDiff, int lonDiff )
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
GfxTileFeatureMapProcessor::addPreviousCoordinate( int32 nextLat,
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
GfxTileFeatureMapProcessor::addPreviousCoordinate( GfxFeature* feature )
{
   // Supply currLat and currLon as next coordinates (even if this is
   // the last coordinate to add).
   addPreviousCoordinate( m_currLat, m_currLon, feature );
}

void
GfxTileFeatureMapProcessor::addClosedCoordinate(GfxFeature* feature)
{
//   addCoordinateToFeature(feature, m_startLat-m_prevAddedLat, 
//                                   m_startLon-m_prevAddedLon );
     addClippedCoordinate(m_startLat, m_startLon, feature);
}


void
GfxTileFeatureMapProcessor::createRouteGfxFeatureMap( const GfxFeatureMapRequestPacket* p,
                                                      GfxFeatureMap* gfxFeatureMap )
{
   mc2dbg2 << "GfxTileFeatureMapProcessor::createRouteGfxFeatureMap() "
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
   
   
   LangTypes::language_t lang = p->getLanguage();

   list<uint32>::const_iterator it = nodeIDs.begin();
   bool firstFeature = true;

   for (uint32 i = 0; i < nbrNodes; i++) {
      uint32 nodeID = *it;
      mc2dbg8 << "nodeID = " << nodeID << endl;

      // Get the corresponding item.
      Item* item = m_map->itemLookup(0x7fffffff & nodeID);

      if ( item == NULL ) {
         mc2dbg << "[GTFMP]: Item " << MC2HEX( nodeID )
                << " not found" << endl;
      }

      GfxData* gfx = NULL;
      if ((item != NULL) && ((gfx = item->getGfxData()) != NULL)) {

         bool includeRouteOrigin = 
            (firstFeature && (! p->getIgnoreStartOffset()));
         bool includeRouteDestination =
            (i == (nbrNodes - 1)) && (! p->getIgnoreEndOffset());
         
         MC2BoundingBox bbox;
         gfx->getMC2BoundingBox(bbox);
         if ( ! bbox.overlaps( m_bbox ) ) {
            // Mark that the first feature has been taken care of.
            firstFeature = false;

         } else {

            // Find out if coordinates start at node 0 or 1
            byte nodeNbr = 0;
            if ( nodeID & 0x80000000 ) {
               // Means the coordinates should be added in opposite order.
               nodeNbr = 1;
            }
            
            // Get the offsets.
            int startOffset = -1;
            if ( includeRouteOrigin ) {
               startOffset = p->getStartOffset();
            }
            
            int endOffset = -1;
            if ( includeRouteDestination ) {
               endOffset = p->getEndOffset();
            }
           
            
            // The origin.
            if ( includeRouteOrigin ) {
               int32 startLat, startLon;
               gfx->getCoordinate( p->getStartOffset(),
                                   startLat, 
                                   startLon );

               if ( m_coordIncludeBBox.inside( startLat, startLon ) ) {
                  // Add ROUTE_ORIGIN feature.
                  mc2dbg8 << "   To add ROUTE_ORIGIN-feature" << endl;
                  GfxFeature* originFeature = 
                     new GfxFeature(GfxFeature::ROUTE_ORIGIN);
                  originFeature->addNewPolygon(true, 1);
                  originFeature->addCoordinateToLast(startLat,  
                                                     startLon);

                  // Starting angle towards node 1
                  float64 angle = gfx->getAngle( p->getStartOffset() );
                  if ( nodeNbr ) {
                     // other direction
                     angle = int(rint(angle + 180.0)) % 360;
                  }
                  gfxFeatureMap->setStartingAngle( 
                        uint32(angle * 256 / 360) );
                  
                  // Add to map.
                  gfxFeatureMap->addFeature(originFeature);
               }
            }
            
            // Language does matter, it is used..
            GfxFeature* routeFeature = createGfxFeatureFromItem(
                                           item, 
                                           NULL,
                                           GfxFeature::ROUTE,
                                           (nodeNbr == 0),
                                           MAX_INT32,
                                           MAX_INT32,
                                           lang,
                                           MAX_BYTE,
                                           startOffset, 
                                           endOffset );
            
            if ( routeFeature != NULL ) {
               
               bool add = true;
               
               if ( routeFeature->getNbrPolygons() == 1 ) { 
                  GfxPolygon* polygon = routeFeature->getPolygon( 0 );
                  if ( polygon->getNbrCoordinates() < 2 ) {
                     add = false;
                  } else if ( polygon->getNbrCoordinates() == 2 ) {
                     // Don't add the feature if both coordinates are the same.
                     // It will mess up the route for the java client.
                     if ( ( polygon->getCoord( 0 ) ==
                            polygon->getCoord( 1 ) ) ) {
                        add = false;
                     }
                  }
               }
                 
               if ( add ) {
                  // Add to map.
                  addFeature( gfxFeatureMap, routeFeature );
               } else {
                  delete routeFeature;
                  routeFeature = NULL;
               }
            }

            // The destination.
            if ( includeRouteDestination ) {
               
               int32 endLat, endLon;
               gfx->getCoordinate( p->getEndOffset(),
                                   endLat, 
                                   endLon );

               if ( m_coordIncludeBBox.inside( endLat, endLon ) ) {
               
                  // Add ROUTE_DESTINATION feature.
                  mc2dbg8 << "   To add ROUTE_DESTINATION-feature" 
                     << endl;
                  GfxFeature* routeDestination = 
                     new GfxFeature(GfxFeature::ROUTE_DESTINATION);
                  routeDestination->addNewPolygon(true, 1);
                  routeDestination->addCoordinateToLast(endLat, endLon);
                  
                  // Add to map.
                  gfxFeatureMap->addFeature( routeDestination );
               }
            }

            // This is not the first feature any more.
            firstFeature = false;
         }
      } else {
         // An item could not be retreived from the node id.
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
            if ( prevItem == NULL ) {
               mc2log << warn 
                      << "[GTFMP] Prev Item: " << MC2HEX( prevNodeID )
                      << " not found. " << endl;
            }

            GfxData* prevGfx = NULL;
            if ( prevItem != NULL &&
                 ( prevGfx = prevItem->getGfxData() ) != NULL ) {
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
                                      ItemTypes::invalidName),
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
}

void
GfxTileFeatureMapProcessor::addConnectingItems( Node* node, 
                                            set<uint32>& handledNodeIDs,
                                            GfxFeatureMap* featureMap,
                                            uint32 totalLength,
                                            uint32 maxLength,
                                            LangTypes::language_t langType,
                                            bool add )
{
   mc2dbg8 << "[GFMP]::addConnectingItems. Node ID = "
          << node->getNodeID() 
          << " Total length = " << totalLength
          << ", max length = " << maxLength << endl;
   if ( add ) {
   
      Item* curItem = 
         m_map->itemLookup( MapBits::nodeItemID( node->getNodeID() ) );
      if ( curItem == NULL ) {
         mc2dbg << error <<  "[GFMP] could not find item to connect." << endl;
         mc2dbg << error << "[GFMP] ItemID: "
                << hex << MapBits::nodeItemID( node->getNodeID() )
                << " Node id: "
                << node->getNodeID()
                << dec
                << endl;
         return;
      }

      uint32 curLength = uint32( curItem->getGfxData()->getLength( 0 ) );
#if 0
      if ( totalLength + curLength > maxLength ) {
         mc2dbg << "[GFMP]::addConnectingItems. "
                << "Skipping node since " << totalLength + curLength
                << " > " << maxLength << endl;
         return;
      }
#endif
      totalLength += curLength;
      
      FilterSettings filterSettings;
      TileMapUtility::getFilterSettings(&filterSettings, curItem,
                                        m_filtScaleLevel);
      
      GfxFeature* feature = 
         createGfxFeatureFromItem( curItem, &filterSettings,
               GfxFeature::NBR_GFXFEATURES,
//                     GfxFeature::ROUTE,
               true, MAX_INT32, MAX_INT32,
               langType );
      // would like language_t instead of languageCode
      // if possible..
      if ( feature != NULL )  {
         mc2dbg8 << "[GFMP]::addConnectingItems. Adding connecting route " 
                << curItem->getID() << endl;
         feature->setScaleLevel( CONTINENT_LEVEL );
         addFeature( featureMap, feature );
      }
      handledNodeIDs.insert( node->getNodeID() );
#if 1
      if ( totalLength  > maxLength ) {
         mc2dbg8 << "[GFMP]::addConnectingItems. "
                << "Skipping node since " << totalLength
                << " > " << maxLength << endl;
         return;
      }
#endif      
   }
      
   for ( uint32 i = 0; i < node->getNbrConnections(); ++i ) {
      Connection* con = node->getEntryConnection( i );
      uint32 nodeID = con->getConnectFromNode();
      mc2dbg8 << "[GFMP]::addConnectingItems. Processing node " 
              << node->getNodeID() << " child node "
              << nodeID << endl; 
      if ( handledNodeIDs.find( nodeID ) != handledNodeIDs.end() ) {
         // Found a node already handled.
         mc2dbg8 << "[GFMP]::addConnectingItems. Skipping" << endl;
         continue;
      }

      addConnectingItems( m_map->nodeLookup( nodeID ),
                          handledNodeIDs,
                          featureMap,
                          totalLength,
                          maxLength,
                          langType,
                          true ); // Always add the item.
   }
   
}

void
GfxTileFeatureMapProcessor::addCloseToRouteItems( Node* node, 
                                            set<uint32>& handledNodeIDs,
                                            GfxFeatureMap* featureMap,
                                            uint32 length,
                                            LangTypes::language_t langType )
{
   mc2dbg4 << "[GFMP]::addCloseToRouteItems. Node ID = "
          << node->getNodeID() << endl;

   Item* curItem = 
      m_map->itemLookup( MapBits::nodeItemID( node->getNodeID() ) );
   if ( curItem == NULL ) {
      mc2dbg << error << "[GFMP] addCloseToRouteItems: Not a valid item id: "
             << hex << MapBits::nodeItemID( node->getNodeID() ) << dec << endl;
      return;
   }

   FilterSettings filterSettings;
   TileMapUtility::getFilterSettings(&filterSettings, curItem,
                                     m_filtScaleLevel);
   
   // Get the GfxData for the item
   const GfxData* gfxData = curItem->getGfxData();
   
   for(uint32 i = 0; i < gfxData->getNbrPolygons(); i++) {
      GfxData::const_iterator begin = gfxData->polyBegin( i );
      GfxData::const_iterator end   = gfxData->polyEnd( i );
      for ( GfxData::const_iterator it = begin; it != end;
            ++it ) {
         
         POINT p1, p2, p3, p4;
         // First coordinate
         p1.y = it->lat + int32(
            length * GfxConstants::METER_TO_MC2SCALE );
         p1.x = it->lon + int32(
            length * GfxConstants::METER_TO_MC2SCALE /
            GfxUtility::getCoslat( it->lat, it->lat ) );
         
         // Second coordinate
         p2.y = it->lat - int32(
            length * GfxConstants::METER_TO_MC2SCALE );
         p2.x = it->lon + int32(
            length * GfxConstants::METER_TO_MC2SCALE /
            GfxUtility::getCoslat( it->lat, it->lat ) );
         
         // Third coordinate
         p3.y = it->lat + int32(
            length * GfxConstants::METER_TO_MC2SCALE );
         p3.x = it->lon - int32(
            length * GfxConstants::METER_TO_MC2SCALE  /
            GfxUtility::getCoslat( it->lat, it->lat ) );
         
         // Fourth coordinate
         p4.y = it->lat - int32(
            length * GfxConstants::METER_TO_MC2SCALE );
         p4.x = it->lon - int32(
            length * GfxConstants::METER_TO_MC2SCALE /
            GfxUtility::getCoslat( it->lat, it->lat ) );         
         
         MC2BoundingBox bbox( MAX(p1.y, MAX(p2.y, MAX(p3.y, p4.y) ) ),
                              MIN(p1.x, MIN(p2.x, MIN(p3.x, p4.x ) ) ),
                              MIN(p1.y, MIN(p2.y, MIN(p3.y, p4.y ) ) ),
                              MAX(p1.x, MAX(p2.x, MAX(p3.x, p4.x ) ) ) );
         
         set<Item*> resultItems; 
         set<ItemTypes::itemType>  allowedTypes;
         allowedTypes.insert( ItemTypes::streetSegmentItem );
         
         m_map->getItemsWithinBBox( resultItems,
                                    bbox,
                                    allowedTypes );
         
         set<Item*>::iterator it;
         for( it = resultItems.begin(); it != resultItems.end(); ++it) {
            Item* curItem = *it;
            if( (handledNodeIDs.find( curItem->getID() | 0x80000000) ==
                 handledNodeIDs.end()) &&
                (handledNodeIDs.find( curItem->getID() & 0x7fffffff) ==
                 handledNodeIDs.end()) )
            {
               GfxFeature* feature = 
                  createGfxFeatureFromItem( curItem, &filterSettings,
                                            GfxFeature::NBR_GFXFEATURES,
                                            true, MAX_INT32, MAX_INT32,
                                            langType );
               
               if ( feature != NULL )  {
                  mc2dbg8 << "[GFMP]::addCloseToRouteItems. Adding connecting route " 
                          << curItem->getID() << endl;
                  feature->setScaleLevel( CONTINENT_LEVEL );
                  addFeature( featureMap, feature );
               }
               handledNodeIDs.insert( curItem->getID() | 0x80000000 );
               handledNodeIDs.insert( curItem->getID() & 0x7fffffff );
            }
         }
      }
   }   
}

void
GfxTileFeatureMapProcessor::createConnectingRouteGfxMap(
                           const GfxFeatureMapRequestPacket* p,
                           GfxFeatureMap* featureMap,
                           const RedLineSettings::speedVect_t& speedVect,
                           set<uint32>& handledNodeIDs )
{
   // Extract an gfxFeaturemap of a route.
   list<uint32> routeIDs;
   p->getNodeIDs( m_map, routeIDs );
      
   handledNodeIDs.insert( routeIDs.begin(), routeIDs.end() );
     
   LangTypes::language_t langType = p->getLanguage();
   
   for ( list<uint32>::const_iterator it = routeIDs.begin();
         it != routeIDs.end(); ++it ) {
      
      Node* node = m_map->nodeLookup( *it );
     
      if ( node == NULL ) {
         continue;
      }
     
      // Find the max length for this speed. 
      uint32 maxLength = 0;
      for ( uint32 i = 0; i < speedVect.size(); ++i ) {
         if ( speedVect[ i ].first <= node->getSpeedLimit() ) {
            maxLength = speedVect[ i ].second;
         } else {
            break;
         }
      }

      mc2dbg8 << "[GFMP]:createConnectingRouteGfxMap. Handling route node: " 
              << node->getNodeID() 
              << ", speed is " << (int) node->getSpeedLimit() 
              << ", and maxLength is " << maxLength << endl;
      addConnectingItems( node, handledNodeIDs, featureMap, 
                          0, maxLength, langType,
                          false ); // Don't add this route item.
   }
}

void
GfxTileFeatureMapProcessor::createCloseToRouteGfxMap(
                          const GfxFeatureMapRequestPacket* p,
                          GfxFeatureMap* featureMap,
                          const RedLineSettings::speedVect_t& speedVect,
                          set<uint32>& handledNodeIDs ) 
{
   // Extract a gfxFeaturemap of a route.
   list<uint32> routeIDs;
   p->getNodeIDs( m_map, routeIDs );

   handledNodeIDs.insert( routeIDs.begin(), routeIDs.end() );
   
   LangTypes::language_t langType = p->getLanguage();
   
   for ( list<uint32>::const_iterator it = routeIDs.begin();
         it != routeIDs.end(); ++it ) {
      
      Node* node = m_map->nodeLookup( *it );
      
      if ( node == NULL ) {
         continue;
      }
     
      // Find the length for this speed. 
      uint32 length = 0;
      for ( uint32 i = 0; i < speedVect.size(); ++i ) {
         if ( speedVect[ i ].first <= node->getSpeedLimit() ) {
            length = speedVect[ i ].second;
         } else {
            break;
         }
      }      
      
      mc2dbg8 << "[GFMP]:createCloseToRouteGfxMap. Handling route node: " 
              << node->getNodeID() 
              << ", speed is " << (int) node->getSpeedLimit() 
              << ", and length is " << length << endl;
      addCloseToRouteItems( node, handledNodeIDs, featureMap, 
                            length, langType );
   }
}
                         
void 
GfxTileFeatureMapProcessor::createPOIGfxFeatureMap(
                                       const GfxFeatureMapRequestPacket* p,
                                       GfxFeatureMap* featureMap )
{
   mc2dbg2 << "GfxTileFeatureMapProcessor::createPOIGfxFeatMap() from map " 
           << m_map->getMapID() << " lang " 
           << (int)p->getLanguage() << endl;

   uint32 nbrBefore = featureMap->getNbrFeatures();
   featureMap->setScaleLevel( m_maxScaleLevel );


   DebugClock clock;
   FilterSettings filterSettings;
   // Ordinary POI

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

      if ( curItem == NULL ) {
         continue;
      }
      bool keepZoom = (
            (GET_ZOOMLEVEL(  curItem->getID() ) == ItemTypes::poiiZoomLevel) ||
            ( MapBits::isCountryMap(m_map->getMapID()) &&
                 (GET_ZOOMLEVEL( curItem->getID() ) == 1)) );
      if ( ! keepZoom ) {
         continue;
      }
            
      PointOfInterestItem* poi = NULL;
      // Whether this poi type should be added to the map or not.
      bool poiTypeOK = false;
      uint32 waspID = MAX_UINT32;
      if (curItem->getItemType() == ItemTypes::pointOfInterestItem) {
         poi = static_cast<PointOfInterestItem*> (curItem);
         if ( m_mapSettings->getPOI( poi->getPointOfInterestType() )) {
            // This poi type should maybe be added.
            poiTypeOK = GfxFeatureMapUtility::
               checkPOIExtra( *poi, *m_map, *m_mapSettings, 
                              false ); // do full map right check
            if ( poi->isPointOfInterestType( ItemTypes::cityCentre ) ) {
               waspID = poi->getWASPID();
            }
         }
      }

      // Make sure that this POI type should be added to the map.
      if ( ! poiTypeOK ) {
         continue;
      }
      mc2dbg8 << "ItemName: " << m_map->
         getItemName( curItem,
                      LangTypes::swedish,
                      ItemTypes::officialName )
             << endl;

      // Get POI coordinates
      MC2Coordinate itemCoord;
      if (curItem->getGfxData() == NULL) {
         // SLOW!
         m_map->getItemCoordinates( curItem->getID(), 0,
                                    itemCoord.lat, itemCoord.lon );
      } else {
         itemCoord = curItem->getGfxData()->getCoordinate( 0, 0 );
      }
      
      // The poi is located in the map.
      
      // Get the display class for citycentres.
      byte extraPOIInfo = MAX_BYTE;
      if ( waspID != MAX_UINT32 ) {   
         getDisplayClassForCC( waspID, extraPOIInfo );
         extraPOIInfo = adjustPOIInfoForCountry( extraPOIInfo,
                                                 m_map->getCountryCode(),
                                                 ItemTypes::cityCentre);
      }
      
      MC2String imageName;
      if ( poi ) {
         GfxFeatureMapUtility::
            getExtraInfoForPOI( *poi, *m_map, 
                                extraPOIInfo, imageName );
      }

      // Get the scalelevel
      uint32 scaleLevel =
         TileMapUtility::toDrawItem( *curItem,
                                     m_minScaleLevel, m_maxScaleLevel,
                                     m_bbox,
                                     m_screenX,
                                     m_screenY,
                                     false, // countrymap
                                     false, // usestreets
                                     NULL,  // the map
                                     itemCoord,
                                     false, // alwaysIncludedFromCountryMap
                                     extraPOIInfo,
                                     imageName.empty() ? 
                                     NULL : imageName.c_str() );

      // Special for turkish hospitals - change type
      if ( poi &&
           poi->getPointOfInterestType() == ItemTypes::hospital &&
           m_map->getCountryCode() == StringTable::TURKEY_CC ) {
         extraPOIInfo = 1;
      }

      // Make sure the poi should be included 
      if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
           (scaleLevel >= uint32(m_minScaleLevel)) ) {
         // XXX: Post office and cafe should only be shown
         //  if they have a special image.
         //  The configuration of this needs to be fixed in a better way!
         if ( poi &&
              poi->getPointOfInterestType() == ItemTypes::postOffice &&
              imageName.empty() ) {
            // skip this poi
            continue;
         }

         TileMapUtility::getFilterSettings(&filterSettings, curItem,
                                           m_filtScaleLevel);
         
         GfxFeature* feature = 
            createGfxFeatureFromItem(
               curItem, 
               &filterSettings,
               GfxFeature::NBR_GFXFEATURES,
               true,
               itemCoord.lat, itemCoord.lon,
               p->getLanguage(),
               extraPOIInfo,
               -1, -1,
               imageName.empty() ? NULL : imageName.c_str() );
         
         if ( feature != NULL ) {
            feature->setScaleLevel( scaleLevel );
            addFeature(featureMap, feature);
            DEBUG8(feature->dump(10));
         }
      }
   }
   
   mc2dbg4 << featureMap->getNbrFeatures() - nbrBefore 
           << " items selected and added in " 
           << clock
           << endl
           << "Number splits are " << nbrsplits << endl
           << "Number coords are " << nbrcoords << endl
           << "Size of GfxFeatureMap = " << featureMap->getMapSize()
           << endl;

}



void
GfxTileFeatureMapProcessor::
createGfxFeatureMap( const GfxFeatureMapRequestPacket* p ,
                     GfxFeatureMap* gfxFeatureMap )
{  
   gfxFeatureMap->setScaleLevel( m_maxScaleLevel );
   FilterSettings filterSettings;
   
   // Check if this is the countryoverviewmap or an ordinary map.
   if (MapBits::isCountryMap(m_map->getMapID())) {
      mc2dbg8 << "Looking in country overivew" << endl;
      // Country overview map.
      // Check if we are to include LAND features and if the country polygon 
      // of this country should be included from this country map
      DebugClock clock;
      if ( includeFeatureType( GfxFeature::LAND )  &&
           p->getIncludeCountryPolygon() ) {
         
         CountryOverviewMap* comap = static_cast<CountryOverviewMap*>(m_map);

         // LAND feature
         // 1. create feature (name and type)
         GfxFeature* landFeature = createCountryBorder( p, comap );
         // 2. find out how many polys to include
         uint32 nbrToSend = getNumberCountryPolysToSend( comap );
         // 3. add polys to feature (consider co map gfx data filtering)
         bool anyPolygonAdded = false;
         for(uint16 i=0;
             i < comap->getGfxData()->getNbrPolygons() && i < nbrToSend;
             i++) {
            anyPolygonAdded = addCountryPolygon( *comap, landFeature, i );
         }
         mc2dbg4 << "LAND-feature " << landFeature->getName() << " created in " 
                 << clock << endl;
               
         if ( anyPolygonAdded ) {
            // Add the LAND-feature to the GfxFeatureMap
            addFeature( gfxFeatureMap, landFeature);
         } else {
            delete landFeature;
         }

         // BORDER features
         uint32 nbrBorders = createAndAddBorders( gfxFeatureMap );
         mc2dbg4 << "Added " << nbrBorders << " BORDER features for co map "
                 << m_map->getMapName() << endl;
      }

      // Only add water items on some levels.
      mc2dbg8 << "m_maxScaleLevel:" << m_maxScaleLevel << endl;
      if ( m_maxScaleLevel >= TileMapProperties::areaGfxThreshold ) {
         
         // Add water items. Present at zoomlevel 1.
         const uint32 nbrItemsWithZoom = m_map->getNbrItemsWithZoom(1);
         for (uint32 i = 0; i < nbrItemsWithZoom; i++) {
            Item* curItem = m_map->getItem(1, i);
            if ( curItem == NULL || 
                 curItem->getGfxData() == NULL ) {
               continue;
            }

            uint32 scaleLevel = 
               TileMapUtility::toDrawItem( *curItem,
                                           m_minScaleLevel, m_maxScaleLevel,
                                           m_bbox,
                                           m_screenX, m_screenY,
                                           true,          // countryMap
                                           false,         // useStreets (default)
                                           NULL,          // theMap (default)
                                           MC2Coordinate(), // (default)
                                           true );        // always included from
                                                          // country map.
               
            if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
                 (scaleLevel >= uint32(m_minScaleLevel)) ) {
               TileMapUtility::getFilterSettings( &filterSettings, curItem,
                                                  m_filtScaleLevel ); 
               GfxFeature* feature = 
                  createGfxFeatureFromItem( curItem, &filterSettings,
                                            GfxFeature::NBR_GFXFEATURES,
                                            true, MAX_INT32, MAX_INT32,
                                            p->getLanguage() );

               if ( feature != NULL )  {
                  feature->setScaleLevel( scaleLevel );
                  addFeature( gfxFeatureMap, feature );
               }
            }
         } 
      }
   }
   
   // If ordinary map or only to draw the countryoverview map(s).
   if ( (MapBits::isUnderviewMap(m_map->getMapID())) || 
        (p->getDrawOverviewContents() ) ) {

      
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
         if ( curItem == NULL || curItem->getGfxData() == NULL )
            continue;

         // We don't want to extract poi:s in this method.
         if ( GET_ZOOMLEVEL( curItem->getID() ) == ItemTypes::poiiZoomLevel ) {
            continue;
         }

         // Water/area items added above
         if ( (curItem->getItemType() == ItemTypes::waterItem ||
               curItem->getItemType() == ItemTypes::builtUpAreaItem) &&
              m_maxScaleLevel < TileMapProperties::areaGfxThreshold ) {
            continue;
         }
         // border items already included.
         if ( curItem->getItemType() == ItemTypes::borderItem ) {
            continue;
         }

         uint32 scaleLevel = 
            TileMapUtility::toDrawItem( *curItem, 
                                        m_minScaleLevel, m_maxScaleLevel,
                                        m_bbox,
                                        m_screenX,
                                        m_screenY,
                                        MapBits::
                                        isCountryMap(m_map->getMapID()) );

         if ( (scaleLevel <= uint32(m_maxScaleLevel)) && 
              (scaleLevel >= uint32(m_minScaleLevel)) ) {

            TileMapUtility::getFilterSettings( &filterSettings, curItem,
                                               m_filtScaleLevel );
            GfxFeature* feature = 
               createGfxFeatureFromItem(curItem, 
                                        &filterSettings,
                                        GfxFeature::NBR_GFXFEATURES,
                                        true, MAX_INT32, MAX_INT32,
                                        p->getLanguage() );
            // would like language_t instead of languageCode
            // if possible..
            if ( feature != NULL )  {

               feature->setScaleLevel( scaleLevel );
               addFeature( gfxFeatureMap, feature );
            }
         }
      }
   }
}



GfxFeatureMapReplyPacket* 
GfxTileFeatureMapProcessor::
generateGfxFeatureMap( const GfxFeatureMapRequestPacket* p,
                       MapSettings** mapSettings )
{
   MC2_ASSERT( p->getMapID() == m_map->getMapID() );
 
   DebugClock functionClock;

    mc2dbg2 << "GfxTileFeatureMapProcessor::generateGfxFeatureMap()" << endl;
   
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

   if ( mapSettings != NULL ) {
      *mapSettings = m_mapSettings;
   }

   // Create the map that will be filled with data
   GfxFeatureMap* gfxFeatureMap = new GfxFeatureMap;
   
   MC2BoundingBox bbox;
   p->getMC2BoundingBox(&bbox);
   gfxFeatureMap->setMC2BoundingBox(&bbox);
   // p->getScreenSize( m_screenX, m_screenY ); // Done in init(p)
   gfxFeatureMap->setScreenSize( m_screenX, m_screenY );

   bool extractMap = m_mapSettings->getShowMap();
   // We cannot extract the route if p doesn't contain route, but
   // if none is set all will be set, so see below.
   bool extractRoute = m_mapSettings->getShowRoute() || p->containsRoute();
   bool extractPOI = m_mapSettings->getShowPOI();
   bool extractCityCenters = m_mapSettings->getShowCityCentres();

   if ( (!extractMap) && (!extractRoute) && (!extractPOI) ) {
      // If none of these are set, that means all should be valid.
      mc2dbg2 << "  Using default map extraction: map, poi and route" << endl;
      extractMap = extractPOI = extractRoute = true;
   }

   // Extract map?
   if (extractMap) {
      // Extract map.
      m_maxOneCoordPerPixel = 
         m_mapSettings->getMaxOneCoordPerPixelForMap();
      mc2dbg2 << "  Extracting map" << endl
              << "  m_maxOneCoordPerPixel = " << m_maxOneCoordPerPixel 
              << endl;
      createGfxFeatureMap(p, gfxFeatureMap);
   }
   
   // Extract route? and has route?
   if (extractRoute && p->containsRoute() ) {
      // Extract route
      m_maxOneCoordPerPixel = 
         m_mapSettings->getMaxOneCoordPerPixelForRoute();
      mc2dbg2 << "  Extracting route" << endl
              << "  m_maxOneCoordPerPixel = " << m_maxOneCoordPerPixel 
              << endl;
      createRouteGfxFeatureMap(p, gfxFeatureMap);
   } 

   const RedLineSettings& redlineSettings = 
      m_mapSettings->getRedLineSettings();

   set<uint32> handledNodeIDs;
   if ( redlineSettings.getIncludeConnectingRoads() ) {
      mc2dbg << "[GTFM]: redline " << redlineSettings << endl;
      RedLineSettings::speedVect_t speedVect = 
         redlineSettings.getSpeedVect();
      createConnectingRouteGfxMap( p,
                                   gfxFeatureMap,
                                   speedVect,
                                   handledNodeIDs );
   } 
   if( ! redlineSettings.getBBoxSpeedVect().empty() ) {
      RedLineSettings::speedVect_t bboxSpeedVect =
         redlineSettings.getBBoxSpeedVect();
      createCloseToRouteGfxMap( p,
                                gfxFeatureMap, 
                                bboxSpeedVect,
                                handledNodeIDs);
   }
   
   // Extract poi:s or cityCenters?
   if ( extractPOI || extractCityCenters ) {
      m_maxOneCoordPerPixel = false;
      mc2dbg2 << "  Extracting poi:s" << endl;
      createPOIGfxFeatureMap( p, gfxFeatureMap );
   }      
 
   // Store the gfxFeaturemap into the replypacket.
   DataBuffer* buf = new DataBuffer( gfxFeatureMap->getMapSize() );
   gfxFeatureMap->save(buf);
   int bufLen = buf->getCurrentOffset();
   //gfxFeatureMap->dump(10);
   //gfxFeatureMap->printStatistics(1);
  
   bool zipped = false;

   // Zipping is disabled since it seems to take more time to zip than
   // to send the data.
#if 0   
   // Check if the saved buf is smaller once zipped.
   DataBuffer* zippedBuf = new DataBuffer( buf->getBufferSize() * 2 + 1024 );
   DebugClock clock;
   int zippedLen = GzipUtil::gzip( zippedBuf->getBufferAddress(),
                                   zippedBuf->getBufferSize(),
                                   buf->getBufferAddress(),
                                   bufLen, 9 );
   if ( ( zippedLen > 0 ) && ( zippedLen < bufLen ) ) {
      // The zipped buffer was smaller, so let's use it instead.
      mc2dbg << "[GTFMP]: Zipped buffer reduced size from " 
             << bufLen << " to " << zippedLen << endl;
      mc2dbg << "[GTFMP]: Zipping took "
             << clock << endl;
      zippedBuf->readPastBytes( zippedLen );
      delete buf;
      buf = zippedBuf;
      bufLen = zippedLen;
      zipped = true;
   } else {
      // The zipped data became larger than the unzipped! Bad.
      delete zippedBuf;
   }
#endif  
   const uint32 EXTRALENGTH = 200;
   if ( ( gfxFeatureMap->getMapSize() + EXTRALENGTH) > reply->getBufSize()) {
      reply->resize(gfxFeatureMap->getMapSize() + 
                    REPLY_HEADER_SIZE + EXTRALENGTH);
   }
   reply->setGfxFeatureMapData( bufLen, buf, zipped );

   DEBUG8(
      mc2dbg << here << " Dumping GfxFeature map that will be sent:" << endl;
      gfxFeatureMap->dump(2);
   );

   mc2dbg << "[GfxTFMP] nbr generated " << gfxFeatureMap->getNbrFeatures() 
          << " for map id: " << hex << "0x" << m_map->getMapID() << dec << endl;

   delete gfxFeatureMap;
   delete buf; 
   
   mc2dbg2 << "[GfxTFMP] generateGfxFeatureMap took " << functionClock << endl;
   
   reply->setStatus(StringTable::OK);
   return reply;
}

bool 
GfxTileFeatureMapProcessor::singlePolygonFeature( 
                              GfxFeature::gfxFeatureType type ) 
{
   switch ( type ) {
      case ( GfxFeature::STREET_MAIN ) :
      case ( GfxFeature::STREET_FIRST ) :
      case ( GfxFeature::STREET_SECOND ) :
      case ( GfxFeature::STREET_THIRD ) :
      case ( GfxFeature::STREET_FOURTH ) :
      case ( GfxFeature::FERRY ) : 
      case ( GfxFeature::ROUTE ) : 
      case ( GfxFeature::BORDER ) : 
         return true;
      default :
         return false;
   }
}

void
GfxTileFeatureMapProcessor::addFeature( GfxFeatureMap* gfxFeatureMap,
                                        GfxFeature* feature )
{
   // We only support street and ferry features with one polygon.
   // If several polygons are found then a new features is created
   // for each polygon.
   if ( ( feature->getNbrPolygons() > 1 ) && 
        singlePolygonFeature( feature->getType() ) ) {      
      // Create a new feature for each one.
      for ( uint16 p = 0; p < feature->getNbrPolygons(); ++p ) {
         gfxFeatureMap->addFeature( feature->clonePolygon( p ) ); 
      }

      // Delete the feature and set it to NULL.
      delete feature;
      feature = NULL;
   } else {
      gfxFeatureMap->addFeature( feature );
   }

}
   
byte
GfxTileFeatureMapProcessor::adjustPOIInfoForCountry(
                     byte extraPOIIfo, StringTable::countryCode country,
                     ItemTypes::pointOfInterest_t poiType)
{
   // Increase importance of city centres in countries where no big
   // cities exists.
   // Change the middle-important cities wit displayclass 7 and 8 to be 5 and 7
   // Don't change minor cities.

   byte retVal = extraPOIIfo;

   if ( (poiType == ItemTypes::cityCentre) &&
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


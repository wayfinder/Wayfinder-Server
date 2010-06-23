/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapDrawerExport.h"

#include "MapTextPlacement.h"
#include "MapDrawingCommon.h"

#include "GfxFeatureMap.h"
#include "GfxFeature.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "GfxData.h"
#include "DataBuffer.h"
#include "MC2BoundingBox.h"
#include "ObjectBoxes.h"

#include "STLUtility.h"

namespace MapDrawerExport {

using MapDrawingCommon::featurenotice_t;
using MapTextPlacement::featuretextnotice_t;

typedef vector<featurenotice_t>::const_iterator featureNoticeIt;

unsigned int m_totalBufSizeHeader_POS = 0;

/**
 * Calculates the export map size for a GfxFeatureMap.
 * @param featureMap The GfxFeatureMap to calculate size for.
 * @param notices Notices to the GfxPolygons to calculate size 
 *        for.
 * @return The size for the GfxFeatureMap in export format.
 */
uint32
exportMapSize( GfxFeatureMap* featureMap,
               vector<featurenotice_t>& notices ) {
   uint32 size = 0;

   // EXPORT GFX FEATURE MAP VERSION
   size += 2+2;
   // EXPORT GFX FEATURE MAP HEADER
   size += 4+4+4+4+2+2+4+4+4;

   // EXPORT GFX FEATUREs
   const GfxFeature* feature = NULL;
   for ( uint32 i = 0 ; i < featureMap->getNbrFeatures() ; i++ ) {
      feature = featureMap->getFeature( i );
      size += 4+4+4+4+4+4+4+4 + strlen( feature->getName() ) +1+3;
      if ( dynamic_cast< const GfxPOIFeature* > (feature) ) {
         // POI
         size += 1+3;
      } else if ( dynamic_cast< const GfxTrafficInfoFeature* > (feature) ) {
         // TRAFFIC_INFO
         size += 4+4+1+1+1+1;
      }
   }

   // EXPORT GFX POLYGONs
   for ( featureNoticeIt f = notices.begin() ; f < notices.end() ; f++ ) {
      size += 4+4+4+4+4;
      GfxPolygon* poly = f->m_feature->getPolygon( f->m_polygonIndex );

      if ( dynamic_cast< GfxRoadPolygon* > ( poly ) != NULL )
      {
         size += 4;
      } else if ( (f->m_feature->getType() == GfxFeature::POI) ||
                  (f->m_feature->getType() == GfxFeature::TRAFFIC_INFO) )
      {
         size += 4;
      }

      int coordPairSize;
      if ( poly->usesCoordinates16Bits() ) {
         coordPairSize = 4;
      } else {
         coordPairSize = 8;
      }

      // Include all coords except one, since that one was include
      // as starting lat and lon.
      size += coordPairSize*( poly->getNbrCoordinates() - 1);
   }

   return size;
}

/**
 * Calculates the export scalable  map size for a GfxFeatureMap.
 * @param featureMap The GfxFeatureMap to calculate size for.
 * @param notices Notices to the GfxPolygons to calculate size 
 *        for.
 * @return The size for the GfxFeatureMap in export scalable format.
 */
uint32 exportScalableMapSize( GfxFeatureMap* featureMap,
                              vector<featurenotice_t>& notices )
{
   uint32 size = 0;
   // EXPORT SCALABLE GFX FEATURE MAP VERSION
   size += 2+2;

   // EXPORT SCALABLE GFX FEATURE MAP HEADER
   // This size is slightly too large since we don't care to count
   // how many GfxFeature types that are actually used, instead
   // we approximate this by saying all GfxFeature types are used.
   size +=  4+4+4+4+4+2+2+4+4+4+4 + (4+4)*GfxFeature::NBR_GFXFEATURES;


   // EXPORT GFX FEATUREs
   const GfxFeature* feature = NULL;
   for ( uint32 i = 0 ; i < featureMap->getNbrFeatures() ; i++ ) {
      feature = featureMap->getFeature( i );
      size += 2+1+2+ strlen( feature->getName() ) +1+3;
      size += 1 +3;
   }

   // EXPORT GFX POLYGONs
   for ( featureNoticeIt f = notices.begin() ; f < notices.end() ; f++ ) {
      size += 4+4+4+4;
      if ( dynamic_cast< GfxRoadPolygon* > ( f->m_feature->getPolygon(
         f->m_polygonIndex ) ) != NULL )
      {
         size += 4;
      }

      size += 4*( f->m_feature->getPolygon(
         f->m_polygonIndex )->getNbrCoordinates() );
   }

   return size;
}

/**
 * Writes an EXPORT GFX FEATURE MAP VERSION to exportMap.
 * @param exportMap Databuffer to write to.
 * @param featureMap The featureMap to write header for.
 */
void writeExportGfxFeatureMapVersion( DataBuffer* exportMap,
                                      GfxFeatureMap* featureMap )
{
   //      <majorversion ( 2 bytes )>
   exportMap->writeNextShort( 0 );
   //      <minorversion ( 2 bytes )>
   exportMap->writeNextShort( 0 );
}

/**
 * Writes an EXPORT GFX FEATURE MAP HEADER to exportMap.
 * @param exportMap Databuffer to write to.
 * @param featureMap The featureMap to write header for.
 * @param nbrPolygons The number of polygons in the export map.
 */
void writeExportGfxFeatureMapHeader( DataBuffer* exportMap,
                                     GfxFeatureMap* featureMap,
                                     uint32 nbrPolygons )
{
   MC2BoundingBox bbox;
   featureMap->getMC2BoundingBox( &bbox );
   //        <maxLat ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMaxLat() );
   //        <minLon ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMinLon() );
   //        <minLat ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMinLat() );
   //        <maxLon ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMaxLon() );
   //        <screenX ( 2 bytes )>
   exportMap->writeNextShort( featureMap->getScreenX() );
   //        <screenY ( 2 bytes )>
   exportMap->writeNextShort( featureMap->getScreenY() );
   //        <scale level ( 4 bytes )>
   exportMap->writeNextLong( featureMap->getScaleLevel() );
   //        <nbrFeatures ( 4 bytes )>
   exportMap->writeNextLong( featureMap->getNbrFeatures() );
   //        <totalNbrPolygons ( 4 bytes)>
   exportMap->writeNextLong( nbrPolygons );
}

/**
 * Writes an EXPORT SCALABLE GFX FEATURE MAP HEADER to exportMap.
 * @param exportMap Databuffer to write to.
 * @param featureMap The featureMap to write header for.
 * @param nbrPolygons The number of polygons in the export map.
 */
void writeExportScalableGfxFeatureMapHeader( DataBuffer* exportMap,
                                             GfxFeatureMap* featureMap,
                                             uint32 nbrPolygons )
{
   //       <mapSizeWithHeader ( 4 bytes )> placeholder, is set later when all the features have been added.
   exportMap->writeNextLong( 0 );
   //If the position for the mapSizeWithHeader is altered the m_totalBufSizeHeader_POS must be altered accordingly.
   m_totalBufSizeHeader_POS = 2+2;

   MC2BoundingBox bbox;
   featureMap->getMC2BoundingBox( &bbox );
   //        <maxLat ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMaxLat() );
   //        <minLon ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMinLon() );
   //        <minLat ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMinLat() );
   //        <maxLon ( 4 bytes )>
   exportMap->writeNextLong( bbox.getMaxLon() );
   //        <screenX ( 2 bytes )>
   exportMap->writeNextShort( featureMap->getScreenX() );
   //        <screenY ( 2 bytes )>
   exportMap->writeNextShort( featureMap->getScreenY() );
   //        <scale level ( 4 bytes )>
   exportMap->writeNextLong( featureMap->getScaleLevel() );
   //        <nbrFeatures ( 4 bytes )>
   exportMap->writeNextLong( featureMap->getNbrFeatures() );
   //        <totalNbrPolygons ( 4 bytes)>
   exportMap->writeNextLong( nbrPolygons );

   // Count the number of  features for each feature type.
   int nbrFeaturesPerFeatureType[GfxFeature::NBR_GFXFEATURES];
   for (int i=0; i < GfxFeature::NBR_GFXFEATURES; i++){
      nbrFeaturesPerFeatureType[i] = 0;
   }
   for (uint32 i=0; i < featureMap->getNbrFeatures(); i++ ){
      MC2_ASSERT( featureMap->getFeature(i)->getType() <
                  GfxFeature::NBR_GFXFEATURES );
      nbrFeaturesPerFeatureType[ featureMap->getFeature(i)->getType() ]++;
   }

   // Count the used feature types.
   int usedNbrFeatureTypes = 0;
   for (int i=0; i < GfxFeature::NBR_GFXFEATURES; i++){
     if (nbrFeaturesPerFeatureType[i] != 0){
       usedNbrFeatureTypes++;
     }
   }

   // Write the number or used featrue types.
   //        <usedNbrFeatureTypes ( 4 bytes)>
   exportMap->writeNextLong( usedNbrFeatureTypes );

   // Write the number of  features for each used  feature type.
   for (int i=0; i < GfxFeature::NBR_GFXFEATURES; i++){
      if ( nbrFeaturesPerFeatureType[i] != 0){
         //        <the feature type i ( 4 bytes)>
         exportMap->writeNextLong( i );
         //        <nbrFeatures for feature type i ( 4 bytes)>
         exportMap->writeNextLong( nbrFeaturesPerFeatureType[i] );

         mc2dbg1 << "Feature type: " << i << " has "  <<  nbrFeaturesPerFeatureType[i] << " features." << endl;
      }
   }
}

/**
 * Writes total size of exportMap to the exportMap header.
 * @param exportMap Databuffer to write to.
 */
void
writeExportScalableGfxFeatureMapSize( DataBuffer* exportMap )
{
   uint32 theSize = exportMap->getCurrentOffset();
   if ( theSize > (  m_totalBufSizeHeader_POS + 4 ) ){
      exportMap->writeLong( theSize,  m_totalBufSizeHeader_POS);
      exportMap->reset();
      exportMap->readPastBytes( theSize );
   }
   else {
      mc2dbg4 << "MapDrawer::writeExportScalableGfxFeatureMapSize(), "
              << "to small buffer. "  << endl;
   }
}


/**
 * Writes an EXPORT GFX FEATURE to exportMap.
 * @param exportMap Databuffer to write to.
 * @param feature The GfxFeature to write.
 */
void
writeExportGfxFeature( DataBuffer* exportMap,
                       GfxFeature* feature )
{
   //        <type ( 4 bytes )>
   exportMap->writeNextLong( feature->getType() );
   //        <scale level ( 4 bytes )>
   exportMap->writeNextLong( feature->getScaleLevel() );
   //        <name ( string )>
   exportMap->writeNextString( feature->getName() );
   //        <textLat ( 4 bytes )>
   exportMap->writeNextLong( feature->getTextLat() );
   //        <textLon ( 4 bytes )>
   exportMap->writeNextLong( feature->getTextLon() );
   //        <fontSize ( 4 bytes )>
   exportMap->writeNextLong( feature->getFontSize() );
   //        <drawTextStart ( 4 bytes )>
   exportMap->writeNextLong( feature->getDrawTextStart() );
   //        <displayText ( 4 bytes bool)>
   exportMap->writeNextLong( feature->getDisplayText() );
   //        <nbrPolygons ( 4 bytes )>
   exportMap->writeNextLong( feature->getNbrPolygons() );

   // Check if to write a GfxPOIFeature.
   if ( dynamic_cast< GfxPOIFeature* > ( feature ) != NULL ) {
      GfxPOIFeature* poiFeature = static_cast< GfxPOIFeature* > ( feature );
      //        <poi type ( 1 byte )>
      exportMap->writeNextByte( byte(poiFeature->getPOIType()) );
   }
   else
   // Check if to write a GfxTrafficInfoFeature
   if ( dynamic_cast< GfxTrafficInfoFeature* > ( feature ) != NULL ) {
      GfxTrafficInfoFeature* trafficInfoFeature =
         static_cast< GfxTrafficInfoFeature* > ( feature );
      //        <start time ( 4 bytes )>
      exportMap->writeNextLong( trafficInfoFeature->getStartTime() );
      //        <end time ( 4 bytes )>
      exportMap->writeNextLong( trafficInfoFeature->getEndTime() );
      //        <traffic info type ( 1 bytes )>
      exportMap->writeNextByte(
            byte(trafficInfoFeature->getTrafficInfoType()) );
      //        <angle ( 1 bytes )>
      exportMap->writeNextByte(
            byte(trafficInfoFeature->getAngle()/360.0*255.0) );
      //        <end time ( 1 bytes )>
      exportMap->writeNextBool(
            trafficInfoFeature->isValidInBothDirections() );
   }

}

/**
 * Writes an EXPORT GFX FEATURE to exportScalableMap.
 * @param exportMap Databuffer to write to.
 * @param feature The GfxFeature to write.
 */
void
writeExportScalableGfxFeature( DataBuffer* exportMap,
                               GfxFeature* feature )
{
   //        <type ( 4 bytes )>
   /*Could perhaps be a byte if the types where trimmed*/
   exportMap->writeNextShort( feature->getType() );
   //exportMap->writeNextLong( feature->getType() );
   //        <scale level ( 4 bytes )>
   exportMap->writeNextByte( feature->getScaleLevel() );
   //exportMap->writeNextLong( feature->getScaleLevel() );
   //        <name ( string )>
   exportMap->writeNextString( feature->getName() );
   //        <textLat ( 4 bytes )>
   //exportMap->writeNextLong( feature->getTextLat() );
   //        <textLon ( 4 bytes )>
   //exportMap->writeNextLong( feature->getTextLon() );
   //        <fontSize ( 4 bytes )>
   //exportMap->writeNextShort( feature->getFontSize() );
   //exportMap->writeNextLong( feature->getFontSize() );
   //        <drawTextStart ( 4 bytes )>
   //exportMap->writeNextLong( feature->getDrawTextStart() );
   //        <displayText ( 4 bytes bool)>
   //exportMap->writeNextLong( feature->getDisplayText() );
   //        <nbrPolygons ( 4 bytes )>
   exportMap->writeNextShort( feature->getNbrPolygons() );
   //exportMap->writeNextLong( feature->getNbrPolygons() );

   mc2dbg8 << feature->getName() << ": type =  "
           << (int)( feature->getType());
   mc2dbg8 << " scaleLevel = " << (int)( feature->getScaleLevel());
   mc2dbg8 << " nbrPolygons =" << (int)feature->getNbrPolygons();


   // Check if to write a GfxPOIFeature.
   if ( dynamic_cast< GfxPOIFeature* > ( feature ) != NULL ) {
      GfxPOIFeature* poiFeature =
         static_cast< GfxPOIFeature* > ( feature );
      //        <poi type ( 1 byte )>
      exportMap->writeNextByte( byte(poiFeature->getPOIType()) );
      mc2dbg8 << feature->getName() << ": type =  "
              << (int)( feature->getType());
      mc2dbg8 << " POItype =" << (int)poiFeature->getPOIType() << endl;
   }
}

/**
 * Writes an EXPORT GFX POLYGON or an EXPORT GFX ROAD POLYGON 
 * header to exportMap.
 * @param exportMap Databuffer to write to.
 * @param f Notice to the GfxPolygonto write.
 */
void
writeExportGfxPolygonHeader( DataBuffer* exportMap,
                             featureNoticeIt& f )
{
   GfxPolygon* poly = f->m_feature->getPolygon( f->m_polygonIndex );
   //        <featureIdx ( 4 bytes )>
   exportMap->writeNextLong( f->m_featureIndex );
   //        <16 bit coord representation ( 1 bool )>
   exportMap->writeNextBool(poly->usesCoordinates16Bits());
   // Pad 3 bytes
   exportMap->writeNextByte(0);
   exportMap->writeNextByte(0);
   exportMap->writeNextByte(0);

   uint32 nbrCoordinates = poly->getNbrCoordinates();

   if (poly->usesCoordinates16Bits()) {
      //        <startLat ( 4 bytes )>
      exportMap->writeNextLong( poly->getLat( 0 ) );
      //        <startLon ( 4 bytes )>
      exportMap->writeNextLong( poly->getLon( 0 ) );
      // Reduce nbr coords to follow since we already wrote
      // one coordinate (pair)
      nbrCoordinates--;
   }
   //       <Nbr coordinates to follow ( 4 bytes )>
   exportMap->writeNextLong( nbrCoordinates );

   if ( dynamic_cast< GfxRoadPolygon* > ( poly ) != NULL ) {
      GfxRoadPolygon* roadPoly = static_cast< GfxRoadPolygon* > ( poly );
      //   EXPORT GFX ROAD POLYGON PARAMS
      //   FIXME: Only posSpeed and level0 is sent!!!
      uint32 params = ( ((uint32(roadPoly->getPosSpeedlimit()))<<24) |
                        ((uint32(roadPoly->isMultidigitialized()))<<23) |
                        ((uint32(roadPoly->isRamp()))<<22) |
                        ((uint32(roadPoly->isRoundabout()))<<21) |
                        ((uint32(roadPoly->getLevel0()))<<17));
      exportMap->writeNextLong( params );
   } else if ( (f->m_feature->getType() == GfxFeature::POI) ||
               (f->m_feature->getType() == GfxFeature::TRAFFIC_INFO) ) {
      exportMap->writeNextBool( f->m_visible );
      if ( f->m_visible ) {
         byte poiStatus = 0;
         switch (f->m_poiStatus) {
            case (DrawSettings::singlePOI):
               poiStatus = 0;
               break;
            case (DrawSettings::multiSamePOI):
               poiStatus = 1;
               break;
            case (DrawSettings::multiDifferentPOI):
               poiStatus = 2;
               break;
         }
         exportMap->writeNextByte( poiStatus );
      }
   }
}

/**
 * Writes an EXPORT SCALABLE GFX POLYGON or an EXPORT GFX SCALABLE ROAD POLYGON 
 * header to exportMap.
 * @param exportMap Databuffer to write to.
 * @param f Notice to the GfxPolygonto write.
 */
void
writeExportScalableGfxPolygonHeader( DataBuffer* exportMap,
                                     featureNoticeIt& f )
{
   GfxPolygon* poly = f->m_feature->getPolygon( f->m_polygonIndex );
   //        <featureIdx ( 4 bytes )>
   //exportMap->writeNextLong( f->m_featureIndex );
   //        <startLat ( 4 bytes )>
   exportMap->writeNextLong( poly->getLat( 0 ) );
   //        <startLon ( 4 bytes )>
   exportMap->writeNextLong( poly->getLon( 0 ) );
   //        <nbrCoords ( 4 bytes )>
   exportMap->writeNextLong( poly->getNbrCoordinates() - 1 );

   if ( dynamic_cast< GfxRoadPolygon* > ( poly ) != NULL ) {
      GfxRoadPolygon* roadPoly = static_cast< GfxRoadPolygon* > ( poly );
      //   EXPORT GFX ROAD POLYGON PARAMS
      //   FIXME: Only posSpeed and level0 is sent!!!
      uint32 params = ( ((uint32(roadPoly->getPosSpeedlimit()))<<24) |
                        ((uint32(roadPoly->isMultidigitialized()))<<23) |
                        ((uint32(roadPoly->isRamp()))<<22) |
                        ((uint32(roadPoly->isRoundabout()))<<21) |
                        ((uint32(roadPoly->getLevel0()))<<17));
      exportMap->writeNextLong( params );
      mc2dbg8 << " roadParams = " << params;

      mc2dbg8 << " nbrCoordinates =" << (int)poly->getNbrCoordinates();
      mc2dbg8 << " startLat = " << poly->getLat( 0 )
              << " startLon = " << poly->getLon( 0 );;
   }
}

/**
 * Writes an EXPORT GFX POLYGON BODY to exportMap.
 * @param exportMap Databuffer to write to.
 * @param f Notice to the GfxPolygonto write.
 */
void
writeExportGfxPolygonBody( DataBuffer* exportMap,
                           featureNoticeIt& f )
{
   GfxPolygon* poly = f->m_feature->getPolygon( f->m_polygonIndex );

   if (poly->usesCoordinates16Bits()) {
      // Write 16 bit relative coordinates.
      // Note that the start lat, lon has already been written.
      for ( uint32 i = 1 ; i < poly->getNbrCoordinates(); i++ ) {
         // FIXME:
         // Hack to get lat/lon diff by setting prevLon/prevLat to
         // 0 in getLat/getLon method.

         //      <latDiff ( 2 bytes )>
         exportMap->writeNextShort( poly->getLat( i, 0 ) );
         //      <lonDiff ( 2 bytes )>
         exportMap->writeNextShort( poly->getLon( i, 0 ) );
         mc2dbg8 << " Lat diff =" << (int) poly->getLat( i, 0 )
                 << " Lon diff = " << (int) poly->getLon( i, 0 );
      }
      mc2dbg8 << endl;
   } else {
      // Write 32 bit absolute coordinates
      for ( uint32 i = 0; i < poly->getNbrCoordinates(); i++ ) {
         //      <lat ( 4 bytes )>
         exportMap->writeNextLong( poly->getLat( i ) );
         //      <lon ( 4 bytes )>
         exportMap->writeNextLong( poly->getLon( i ) );
         mc2dbg8 << " Lat =" << (int) poly->getLat( i )
                 << " Lon = " << (int) poly->getLon( i );
      }
      mc2dbg8 << endl;
   }
}

DataBuffer*
makeExportGfxFeatureMap( GfxFeatureMap* map,
                         MapSettings* mapSettings,
                         bool singleDraw,
                         const DrawingProjection* projection,
                         bool initText ) {
   if ( map == NULL ) {
      mc2log << warn << "MapDrawer::makeExportGfxFeatureMap "
             << "input map is NULL. " << endl;
      return NULL;
   }
   vector<featurenotice_t> notices;
   // Vector of boundingboxes of objects that must not be overlapped.
   MapDrawingCommon::ObjectBoxes objectBBoxes;
   STLUtility::AutoContainer< vector<GfxData*> > gfxTextArray;
   MapDrawingCommon::
      sortAndCreateFeatureNotices( map, mapSettings,
                                   notices, singleDraw,
                                   true, // Check for overlapping objects
                                   false, // Include hidden featues.
                                   true, // Filter pois
                                   &objectBBoxes );
   if ( initText ) {
      ImageDraw* image = NULL;
      vector<featuretextnotice_t> textnotices;
      MapTextPlacement::
         initializeText( map, mapSettings, textnotices, objectBBoxes,
                         image, gfxTextArray, projection );
   }
   // Create export map
   auto_ptr<DataBuffer>
      exportMap( new DataBuffer( exportMapSize( map, notices ) ) );

   // Version
   writeExportGfxFeatureMapVersion( exportMap.get(), map );

   // Header
   writeExportGfxFeatureMapHeader( exportMap.get(), map, notices.size() );

   // Features
   for ( uint32 i = 0 ; i < map->getNbrFeatures() ; i++ ) {
      writeExportGfxFeature( exportMap.get(), const_cast< GfxFeature* > (
         map->getFeature( i ) ) );
   }
   // Polygons
   for ( featureNoticeIt f = notices.begin() ; f < notices.end() ; f++ ) {
      writeExportGfxPolygonHeader( exportMap.get(), f );
      writeExportGfxPolygonBody( exportMap.get(), f );
   }

   return exportMap.release();
}

DataBuffer*
makeExportScalableGfxFeatureMap( GfxFeatureMap* map,
                                 MapSettings* mapSettings,
                                 bool singleDraw,
                                 bool initText )
{
   mc2dbg2 << "MapDrawer creates ExportScalableGfxFeatureMap" << endl;


   if ( map == NULL ) {
      mc2log << warn << "MapDrawer::makeExportScalableGfxFeatureMap "
             << "input map is NULL. " << endl;
      return NULL;
   }

   vector<featurenotice_t> notices;
   MapDrawingCommon::
      sortAndCreateFeatureNotices( map, mapSettings,
                                   notices, singleDraw, false, false);

   // Create export map
   DataBuffer* exportMap = new DataBuffer(
      exportScalableMapSize( map, notices ) );

   // Version
   writeExportGfxFeatureMapVersion( exportMap, map );

   // Header
   writeExportScalableGfxFeatureMapHeader(exportMap, map, notices.size() );

   // Sort the notices in feature id order instead of drawing order.
   sort( notices.begin(), notices.end(),
         MapDrawingCommon::LessNoticeFeatureIdOrder() );

   // Features and polygons
   featureNoticeIt f = notices.begin();
   mc2dbg2 << "Number of Features in map: "
           << map->getNbrFeatures() << endl;
   for ( uint32 i = 0 ; i < map->getNbrFeatures() ; i++ ) {

      // NB. Count the nbrs of features in each featuretype here.

      writeExportScalableGfxFeature( exportMap,
                                     const_cast< GfxFeature* >
                                     ( map->getFeature( i ) ) );

      //Write polygons, which have the same id as the feature.
      mc2dbg8 << "Feature id: " << i << endl;
      for (uint32 j = 0; j <  map->getFeature( i )->getNbrPolygons(); j++ )
      {
         //while ( f->m_featureIndex == i){
         mc2dbg8 << "Polygon new id: " <<   f->m_polygonIndex
                 << ", Fid:  " <<  f->m_featureIndex << "; ";

         writeExportScalableGfxPolygonHeader( exportMap, f );
         writeExportGfxPolygonBody( exportMap, f );
         f++;
      }
      mc2dbg8 << endl;
   }

   // Map size
   writeExportScalableGfxFeatureMapSize( exportMap );

   return exportMap;
}

} // MapDrawerExport

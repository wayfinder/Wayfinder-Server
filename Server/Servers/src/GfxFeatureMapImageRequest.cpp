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

#include <algorithm>
#include <set>

#include "GfxFeatureMapImageRequest.h"

#include "MapUtility.h"
#include "MapSettings.h"

#include "RoutePacket.h"
#include "GfxFeatureMapPacket.h"
#include "GfxFeatureMapImagePacket.h"

#include "PacketContainer.h"
#include "TopRegionRequest.h"
#include "Properties.h"
#include "GfxFeatureMap.h"

#include "GFMDataHolder.h"
#include "BBoxPacket.h"
#include "GfxConstants.h"

#include "STLUtility.h"
#include "MC2String.h"

#include "UserData.h"

#include "DrawingProjection.h"
#include "MapBits.h"

// Request tags for sorting the packets are as follows
// 4 bits of initial request count ( bits 31-28 )
// 2 bits of route/info module tag ( bits 27-26 )
// 26 bitar packet count that will be or:ed 

#define GFMIR_MAKE_BBOX_TAG(c) ((c) << 28)
#define GFMIR_TAG_INFO  ( 1U << 26 )
#define GFMIR_TAG_ROUTE ( 1U << 27 )

// -----------------------------------------------------------------------
// ----------------------------------------- GfxFeatureMapImageRequest ---

void
GfxFeatureMapImageRequest::ownMapSettings()
{
   m_mapSettingsToDelete = m_mapSettings;
}

StringTable::stringCode
GfxFeatureMapImageRequest::getStatus() const
{
   // I'm just plucking peppercorns: 1,2,3,4,5...
   return m_status;
}

GfxFeatureMapImageRequest::GfxFeatureMapImageRequest (
   const RequestData& idOrParent,
   const MC2BoundingBox* bboxLatLon,
   uint16 screenX,
   uint16 screenY,
   LangTypes::language_t language,
   const RouteReplyPacket* p,
   bool image,
   bool exportFormat,
   bool drawCopyRight,
   ImageDrawConfig::imageFormat format,
   uint16 sizeParam,
   MapSettings* mapSettings,
   const TopRegionRequest* topReq,
   const MC2BoundingBox& ccBBox,
   uint32 maxScale, // INVALID_SCALE_LEVEL
   uint32 minScale, // CONTINENT_LEVEL
   uint32 filtScale, // INVALID_SCALE_LEVEL
   bool extractForTileMaps, // = false 
   uint32 zoom ) // MAX_UINT32
   : RequestWithStatus(idOrParent),
     m_screenSize( screenX, screenY ),
     m_useEvents( false )
{
   m_mapSettingsToDelete = NULL;
   m_topReq = topReq;
   // "SERVER_ALLOWED_GFX_REGIONS"
   m_specialAllowedMaps = getAllowedGfxMapIDs();
   m_onlyRoute = false;
   // Set some members
   m_upperLeftLat = bboxLatLon->getMaxLat();
   m_upperLeftLon = bboxLatLon->getMinLon();
   m_lowerRightLat = bboxLatLon->getMinLat();
   m_lowerRightLon = bboxLatLon->getMaxLon();   

   if( ccBBox.isValid() ) {
      m_ccBBox = ccBBox;
   }
   

   m_sizeParam = sizeParam;
   m_mapSettings = mapSettings;

   if( m_mapSettings->getDrawingProjection() == NULL ) {
      // Create default drawing projection
      DrawingProjection* projection =
         new CosLatProjection( *bboxLatLon,
                               screenY, screenX );
      m_mapSettings->setDrawingProjection( projection );
   } 
   
   m_arrowType = RouteArrowType::NOT_USED;
   m_beforeTurn = MAX_UINT32;
   m_afterTurn = MAX_UINT32;
   m_mapRotation = 0;
   m_symbolMap = new GfxFeatureMap();
   m_extractForTileMaps = extractForTileMaps;
   m_gfmDataHolder = NULL;
   m_status = StringTable::TIMEOUT_ERROR;
   m_user = NULL;

   if (maxScale == INVALID_SCALE_LEVEL) {
      m_scalable = false;
      m_maxScaleLevel =
         MapUtility::getScaleLevel(*bboxLatLon, screenX, screenY);
   } else {
      m_maxScaleLevel = maxScale;
      m_scalable = true;
   }
   m_minScaleLevel = minScale;
   
   if (filtScale == INVALID_SCALE_LEVEL) {
      m_filtScaleLevel = m_maxScaleLevel;
   } else {
      m_filtScaleLevel = filtScale;
   }

   if ( (mapSettings->getShowRoute()) && 
        (! mapSettings->getShowMap()) &&
        (! mapSettings->getShowPOI()) &&
        (! mapSettings->getShowCityCentres()) ) {
      m_onlyRoute = true;
   }
   

   if ( zoom != MAX_UINT32 ) {
      // If we have a specific zoom level, then we should
      // draw overview content if we are level 7 or lower,
      // That was at least what was expected before by
      // the "algorithm" in the maxLengthThreshold-statement below.
      //  (lower zoom means further away from "earth")
      if ( zoom < 7 ) {
         m_drawOverviewContents = true;
      } else {
         m_drawOverviewContents = false;
      }
   } else {
      // Used to be in MapReader.
      // This is just odd...
      const int32 mapLengthThreshold =
         int32(100000 * GfxConstants::METER_TO_MC2SCALE);
      if ( MAX(bboxLatLon->getWidth(), bboxLatLon->getHeight()) <
           mapLengthThreshold ) {
         m_drawOverviewContents = false;
      } else {
         m_drawOverviewContents = true;
      }
   }
   // Modify the maxOneCoordPerPixel attributes in MapSettings.
   // Better if those attributes should already be set correctly when
   // entering this method...
   if (m_scalable) {
      // Means that we should be able to zoom into the map.
      mapSettings->setMaxOneCoordPerPixelForMap(false);
      mapSettings->setMaxOneCoordPerPixelForRoute(false);
   } else if ( m_onlyRoute ) {
      mapSettings->setMaxOneCoordPerPixelForRoute(false);
   }

   m_includeTrafficInfo = mapSettings->getShowTraffic();
   if ( ! m_extractForTileMaps && m_maxScaleLevel < TRAFFIC_INFO_LEVEL ) {
      // Don't show any traffic for non vector maps that are
      // zoomed out more than TRAFFIC_INFO_LEVEL
      // This is done to avoid unnecessary loading of overview maps.
      m_includeTrafficInfo = false;
   }

   m_useEvents = mapSettings->getShowEvents();

   m_language = language;
   m_answer = NULL;
   m_done = false;
   m_routeReplyPacket = p;
   m_drawImage = image;
   m_exportFormat = exportFormat;
   m_drawCopyRight = drawCopyRight;
   m_imageFormat = format;
   
   m_allowedMaps = NULL;
   
   if ( ! m_onlyRoute ) {
      // send first map requests
      makeInitialMapRequests();
   }
   // Send route map packets
   if ( m_mapSettings->getShowRoute() ) {
      mc2dbg2 << "[GFMIR]: Route Packet = " << MC2HEX(uintptr_t(p)) 
              << endl;
   }
   if (p != NULL && m_mapSettings->getShowRoute() ) {
      makeGfxFeatureMapRequestPackets((RouteReplyPacket*) p);
   }   
} 


GfxFeatureMapImageRequest::~GfxFeatureMapImageRequest()
{
   // Delete everything here...
   
   delete m_symbolMap;

   // Delete any remaining packets in m_mapReplies (concatenateMapAnswers
   // normaly deletes them but if timeout we do it here).
   STLUtility::deleteValues( m_mapReplies );

   delete m_gfmDataHolder;

   delete m_mapSettingsToDelete;
   
   delete m_answer;
   delete m_specialAllowedMaps;
}


void
GfxFeatureMapImageRequest::makeInitialMapRequests( const MC2BoundingBox& box,
                                                   uint32& count )
{
   uint32 mapSetCount = Properties::getUint32Property("MAP_SET_COUNT", 
                                                      MAX_UINT32);
   if (mapSetCount != MAX_UINT32)
      mapSetCount--;

   while (mapSetCount != (MAX_UINT32 - 1)) {
      bool underview = ! m_drawOverviewContents;
      bool country = true;
      BBoxReqPacketData reqData( box, underview, country );
      mc2dbg8 << reqData << endl;
      BBoxRequestPacket* req = new BBoxRequestPacket( reqData );
      updateIDs( req );
      req->setRequestTag( GFMIR_MAKE_BBOX_TAG( ++count ) );
      mc2dbg8 << "[GFMIR]: BBox sent tag "
              << MC2HEX(req->getRequestTag()) << endl;
      enqueuePacketContainer(
         new PacketContainer( 
            req, 0, 0, MODULE_TYPE_MAP,
            PacketContainer::defaultResendTimeoutTime,
            PacketContainer::defaultResends, 
            mapSetCount ));
           
      if (mapSetCount == 0 || mapSetCount == MAX_UINT32)
         mapSetCount = MAX_UINT32 - 1;
      else
         mapSetCount--;
   }
}


void
GfxFeatureMapImageRequest::makeInitialMapRequests()
{
   MC2BoundingBox box( m_upperLeftLat, m_upperLeftLon,
                       m_lowerRightLat, m_lowerRightLon );
   
   // Avoid the backside of the earth (|)
   vector<MC2BoundingBox> bboxes;   
   if ( box.getMinLon() > box.getMaxLon() ) {
      bboxes.resize( 2, box );
      bboxes.front().setMaxLon( MAX_INT32 );
      bboxes.back().setMinLon( MIN_INT32 );
   } else {
      bboxes.push_back( box );
   }
   
   // Avoid the backside of the earth (|)
   // City centres
   vector<MC2BoundingBox> cc_bboxes;
   
   if( m_mapSettings->getShowCityCentres() && m_ccBBox.isValid() ) {
      if ( m_ccBBox.getMinLon() > m_ccBBox.getMaxLon() ) {
         cc_bboxes.resize( 2, m_ccBBox );
         cc_bboxes.front().setMaxLon( MAX_INT32 );
         cc_bboxes.back().setMinLon( MIN_INT32 );
      } else {
         cc_bboxes.push_back( m_ccBBox );
      }
   }
   
   uint32 count = 0;
   for ( vector<MC2BoundingBox>::const_iterator it = bboxes.begin();
         it != bboxes.end();
         ++it ) {
      makeInitialMapRequests( *it, count ); // Will increase count
   }
   // City centres

   for ( vector<MC2BoundingBox>::const_iterator it = cc_bboxes.begin();
         it != cc_bboxes.end();
         ++it ) {
      mc2dbg8 << "[GFMIR]: Requesting box " << *it << endl;
      makeInitialMapRequests( *it, count ); // Will increase count
   }
}

void
GfxFeatureMapImageRequest::handleBBoxReply( const BBoxReplyPacket* packet )
{
   // Get the packet data.
   MC2BoundingBox bbox;
   vector<uint32> mapIDs;
   packet->get( bbox, mapIDs );

   // If we are going to use events, then enqueue some event packets
   // instead of map packets
   if ( m_useEvents ) {
      if ( mapIDs.empty() ) {
         // nothing to do if we did not cover any map IDs.
         // For example; the bounding box was outside the map set.
         return;
      }
      makeRequestPacketsForEvents( mapIDs, bbox,
                                   packet->getRequestTag() );
      return;
   }

   uint32 count = 0;
   // Send packets
   for ( vector<uint32>::const_iterator it = mapIDs.begin();
         it != mapIDs.end();
         ++it, ++count ) {
      uint32 mapID = *it;
      if ( ! checkAllowedMapID( mapID ) ) {
         continue;
      }
      // Send to MapModule
      if ( m_mapSettings->getShowPOI() ||
           m_mapSettings->getShowMap() ||
           m_mapSettings->getShowCityCentres() ) {
         // Include country polygon for now.
         mc2dbg8 << "[GFMIR]: mapID = " << MC2HEX( mapID )
                 << " bbox = " << bbox << endl;
         uint32 reqTagToUse = packet->getRequestTag() | count;
         makeGfxFeatureMapRequestPacket( mapID, bbox, true,
                                         reqTagToUse );
      }
   }
   // Send to infomodule
   if ( m_includeTrafficInfo ) {
      makeGfxFeatureMapRequestPacketsForInfoModule( mapIDs, bbox,
                                                    packet->getRequestTag() );
   }
}

void
GfxFeatureMapImageRequest::processPacket(PacketContainer* ans)
{
   if ( ans ) {
      mc2dbg8 << "[GFMIR]: processPacket() req tag = "
              << MC2HEX( ans->getPacket()->getRequestTag() ) << endl;
   }
   mc2dbg4
      << "[GFMIR] processPacket() top, current nbr of outstanding packets: "
      << getNbrOutstandingPackets() << endl;
   ReplyPacket* reply = static_cast<ReplyPacket*>(ans->getPacket());

   switch (reply->getSubType()) {

      case Packet::PACKETTYPE_GFXFEATUREMAPREPLY: {
         mc2dbg4
            << "[GFMIR] top of PACKETTYPE_GFXFEATUREMAPREPLY handling" << endl;
         // check if it points out more mapids or contains data
         GfxFeatureMapReplyPacket* p = static_cast<
            GfxFeatureMapReplyPacket*> ( ans->getPacket() );
         
         // check packet
         // is a reply with data, put in vector?
         if ( checkAllowedMapID( p->getMapID() )
              && (p->getStatus() != StringTable::MAPNOTFOUND) ) {
            m_mapReplies.push_back(ans);
            mc2dbg4 << "[GFMIR] saved reply in m_mapReplies, status: "
                    << p->getStatus() << endl;
         } else {
            mc2dbg4 << "[GFMIR] processPacket: status was MAPNOTFOUND, "
               "throwing it away" << endl;
            delete ans;
         }
         break;
      }

      case Packet::PACKETTYPE_BBOXREPLY:
         handleBBoxReply( static_cast<BBoxReplyPacket*>( reply ) );
         delete ans;
         break;
      
      case Packet::PACKETTYPE_GFXFEATUREMAP_IMAGE_REPLY: {
         mc2dbg4
            << "[GFMIR] top of PACKETTYPE_GFXFEATUREMAP_IMAGE_REPLY handling"
            << endl;
         m_answer = ans;
         m_done = true;
         // Update the m_status flag.
         // This is from isabBoxMapReply originally
         if ( m_answer == NULL || 
              StringTable::stringCode( 
                 static_cast<ReplyPacket*>(
                    m_answer->getPacket() )->getStatus() ) != 
              StringTable::OK )
         {
            // Error
            if ( m_answer == NULL ) {
               m_status = StringTable::TIMEOUT_ERROR;
            } else {
               m_status = StringTable::stringCode( static_cast<ReplyPacket*>(
                  m_answer->getPacket() )->getStatus());
            }
         } else {
            m_status = StringTable::OK;
         }
            break;
         }
      default:
         mc2log << warn << "[GFMIR] Unexpected packet received: " 
                << reply->getSubTypeAsString() << endl;
      break;
   } // switch

   mc2dbg4 << "[GFMIR] processPacket() current nbr of outstanding packets: "
          << getNbrOutstandingPackets() << endl;
   if (getNbrOutstandingPackets() == 0 && m_answer == NULL) {
      concatenateMapAnswers();
   }
}

PacketContainer*
GfxFeatureMapImageRequest::getAnswer()
{
   PacketContainer* answer = m_answer;
   m_answer = NULL;
   return answer;
}
      
GFMDataHolder* 
GfxFeatureMapImageRequest::getGFMDataHolder() const
{
   return m_gfmDataHolder;
}


void 
GfxFeatureMapImageRequest::setRouteTurn( uint32 beforeTurn, 
                                         uint32 afterTurn,
                                         RouteArrowType::arrowType arrow)
{
   m_arrowType = arrow;
   m_beforeTurn = beforeTurn;
   m_afterTurn = afterTurn;
}     

void
GfxFeatureMapImageRequest::setMapRotation( int16 rotationAngle )
{
  m_mapRotation = rotationAngle;
}

void 
GfxFeatureMapImageRequest::addSymbolToMap( 
   int32 lat, int32 lon,
   const char* name,
   GfxSymbolFeature::symbols symbol,
   const char* symbolImage )
{
   GfxSymbolFeature* feat = new GfxSymbolFeature( GfxFeature::SYMBOL,
                                                  name,
                                                  symbol,
                                                  symbolImage );
   feat->addNewPolygon( true, 1 );
   feat->addCoordinateToLast( lat, lon );
   
   m_symbolMap->addFeature( feat );
}


void 
GfxFeatureMapImageRequest::addSymbolMap( GfxFeatureMap* map ) {
   m_symbolMap->mergeInto( map ); 
}


void
GfxFeatureMapImageRequest::
makeGfxFeatureMapRequestPacket(uint32 mapID,
                               const MC2BoundingBox& bbox,
                               bool includeCountryPolygon,
                               uint32 reqTagToUse )
{
   bool showMap = m_mapSettings->getShowMap();
   bool showTopoMap = m_mapSettings->getShowTopographMap();
   bool showPOI = m_mapSettings->getShowPOI();
   bool showRoute = m_mapSettings->getShowRoute();
   bool showCityCentres = m_mapSettings->getShowCityCentres();

   if( bbox == m_ccBBox )
   {
      m_mapSettings->setMapContent( false, false, false, false );
      m_mapSettings->setShowCityCentres( true );
   }

   MapRights origMapRights = m_mapSettings->getMapRights();

   if ( m_user != NULL ) {
      m_mapSettings->setMapRights( m_user->getUser()->getMapRightsForMap( 
                                      mapID ) );
   }
   
   // Create the packet
   GfxFeatureMapRequestPacket* req = 
      new GfxFeatureMapRequestPacket( mapID,
                                      getUser(),
                                      getID(),
                                      getNextPacketID(),
                                      bbox.getMaxLat(),
                                      bbox.getMinLon(),
                                      bbox.getMinLat(),
                                      bbox.getMaxLon(),
                                      m_screenSize.getWidth(),
                                      m_screenSize.getHeight(),
                                      m_maxScaleLevel,
                                      m_minScaleLevel,
                                      m_filtScaleLevel,
                                      m_language,
                                      m_mapSettings,
                                      true, // ignoreStartOffset
                                      true, // ignoreEndOffset
                                      0,    // startOffset
                                      0,    // endOffset
                                      false,// drawOverviewContents
                                      m_extractForTileMaps );
 
   req->setNbrReqPackets( 0 );
   req->setDrawOverviewContents(m_drawOverviewContents);
   req->setIncludeCountryPolygon(includeCountryPolygon);
   req->setRequestTag( reqTagToUse );
   
   // Timeout in ms, resend(s)
   enqueuePacketContainer( new PacketContainer(req, 0, 0, MODULE_TYPE_MAP, 
                                               5000, 3));
   
   m_mapSettings->setMapContent( showMap, showTopoMap,
                                 showPOI, showRoute );
   m_mapSettings->setShowCityCentres( showCityCentres );
   m_mapSettings->setMapRights( origMapRights );
}
namespace {
auto_ptr<ReplyPacket>
createEmptyGFMReplyPacket( const GfxFeatureMapRequestPacket* req,
                           const ScreenSize& screenSize,
                           const MC2BoundingBox& bbox,
                           uint32 maxScaleLevel ) {


 // Create timeout answer
   GfxFeatureMap gfxFeatureMap;

   gfxFeatureMap.setMC2BoundingBox( &bbox );
   gfxFeatureMap.setScreenSize( screenSize );
   gfxFeatureMap.setScaleLevel( maxScaleLevel );

   DataBuffer buf( gfxFeatureMap.getMapSize() );
   gfxFeatureMap.save( &buf );

   // Create the reply
   GfxFeatureMapReplyPacket* reply = new GfxFeatureMapReplyPacket( req );
   auto_ptr<ReplyPacket> replyP( reply );

   reply->setGfxFeatureMapData( buf.getCurrentOffset(), &buf );
   reply->setStatus( StringTable::OK );

   return replyP;
}

}
void
GfxFeatureMapImageRequest::
makeRequestPacketsForEvents( const vector<uint32>& mapIDs,
                             const MC2BoundingBox& bbox,
                             uint32 origReqTag ) {
   // Create the packet
   GfxFeatureMapRequestPacket* req =
      new GfxFeatureMapRequestPacket( MAX_UINT32, // map id is not important here
                                      getUser(),
                                      getID(),
                                      getNextPacketID(),
                                      bbox.getMaxLat(),
                                      bbox.getMinLon(),
                                      bbox.getMinLat(),
                                      bbox.getMaxLon(),
                                      m_screenSize.getWidth(),
                                      m_screenSize.getHeight(),
                                      m_maxScaleLevel,
                                      m_minScaleLevel,
                                      m_filtScaleLevel,
                                      m_language,
                                      m_mapSettings,
                                      true, // ignoreStartOffset
                                      true, // ignoreEndOffset
                                      0,    // startOffset
                                      0,    // endOffset
                                      false,// drawOverviewContents
                                      m_extractForTileMaps );
   req->setNbrReqPackets( 0 );
   req->setDrawOverviewContents(m_drawOverviewContents);
   req->setRequestTag( origReqTag );

   enqueuePacketContainer( new PacketContainer( req, 0, 0,
                                                MODULE_TYPE_EXTSERVICE ) );
}

void
GfxFeatureMapImageRequest::
makeGfxFeatureMapRequestPacketsForInfoModule( const vector<uint32>& mapIDs,
                                              const MC2BoundingBox& bbox,
                                              uint32 origReqTag )
{   
   // Check if any underview maps are present.
   bool containsUnderview = false;
   for ( vector<uint32>::const_iterator i = mapIDs.begin();
         i != mapIDs.end();
         i++) {

      if ( MapBits::isUnderviewMap( *i ) ) {
         containsUnderview = true;
         break;
      }
   }

   uint32 count = 0;
   // send one for each map in m_mapsHandled
   for ( vector<uint32>::const_iterator i = mapIDs.begin();
         i != mapIDs.end();
         i++, ++count ) {

      uint32 mapID = *i;

      if ( MapBits::isCountryMap( mapID ) ) {
         if ( containsUnderview ) {
            // Skip country maps in case underview maps are present.
            continue;
         } else {
         
            // Country map found when no underviews are present.
            // Convert the country map to overview map.
            mapID = MapBits::countryToOverview( mapID ); 
         }
      }

      // Create the packet
      GfxFeatureMapRequestPacket* req = 
         new GfxFeatureMapRequestPacket( mapID,
                                         getUser(),
                                         getID(),
                                         getNextPacketID(),
                                         bbox.getMaxLat(),
                                         bbox.getMinLon(),
                                         bbox.getMinLat(),
                                         bbox.getMaxLon(),
                                         m_screenSize.getWidth(),
                                         m_screenSize.getHeight(),
                                         m_maxScaleLevel,
                                         m_minScaleLevel,
                                         m_filtScaleLevel,
                                         m_language,
                                         m_mapSettings,
                                         true, // ignoreStartOffset
                                         true, // ignoreEndOffset
                                         0,    // startOffset
                                         0,    // endOffset
                                         false,// drawOverviewContents
                                         m_extractForTileMaps );

      // this is apparently never used, not even in MapModule
      req->setNbrReqPackets(0);  
      req->setRequestTag( count | GFMIR_TAG_INFO | origReqTag );
     
      auto_ptr<ReplyPacket>
         reply( createEmptyGFMReplyPacket( req,
                                           m_screenSize, bbox,
                                           m_maxScaleLevel ) );

      PacketContainer* toSend =
         new PacketContainer( req, 0, 0, MODULE_TYPE_TRAFFIC,
                              5000, 2); // Timeout in ms, resend(s)
      // Add default timeout packet with empty map.
      toSend->putTimeoutPacket( reply.release() );

      mc2dbg8 << "[GFMIR]: Enqueueing to infomod" << endl;
      enqueuePacketContainer( toSend );
   }
   m_mapsHandled.clear();
}

void
GfxFeatureMapImageRequest::makeGfxFeatureMapRequestPackets(RouteReplyPacket* p)
{
   mc2dbg4 << "[GFMIR]: makeGfxFeatureMapRequestPackets(RRP* p)" << endl;
   // Only route on these maps
   bool showMap = m_mapSettings->getShowMap();
   bool showTopoMap = m_mapSettings->getShowTopographMap();
   bool showPOI = m_mapSettings->getShowPOI();
   bool showRoute = m_mapSettings->getShowRoute();
   bool showCityCentres = m_mapSettings->getShowCityCentres();

   m_mapSettings->setMapContent( false, false, false, true );
   m_mapSettings->setShowCityCentres( false );
   
   // The nodeIDs index in the RouteReplyPacket;
   uint32 nodeIndex = 0;

   // Number of nodes in the RouteReplyPacket.
   uint32 nbrNodes = p->getNbrItems();
   
   uint32 mapID = 0, nodeID = 0;
   uint32 prevMapID = MAX_UINT32;
   // If we skipped last mapid
   bool skippedLastMapID = false;
   bool firstRoute = true;
   
   GfxFeatureMapRequestPacket* curRequest = NULL;

   while (nodeIndex < nbrNodes) {
      
      p->getRouteItem(nodeIndex, mapID, nodeID);

      if ( checkAllowedMapID( mapID ) ) {
         if ( mapID != prevMapID ) {
            skippedLastMapID = false;
            // Keep track of the packet ids.
            uint16 packetID = getNextPacketID();
            // Create a new GfxFeatureMapRequestPacket
            curRequest = new GfxFeatureMapRequestPacket(
               mapID,
               getUser(),
               getID(),
               packetID,
               m_upperLeftLat,
               m_upperLeftLon,
               m_lowerRightLat,
               m_lowerRightLon,
               m_screenSize.getWidth(),
               m_screenSize.getHeight(),
               10, // max scaleLevel
               0,  // min scaleLevel
               10, // filtering scaleLevel
               m_language,
               m_mapSettings,
               true, // ignoreStartOffset
               true, // ignoreEndOffset
               0,    // startOffset
               0,    // endOffset
               false,// drawOverviewContents
               m_extractForTileMaps );
         
            // set the tag, use the nodeIndex since we
            // only care about the order
            curRequest->setRequestTag(nodeIndex | GFMIR_TAG_ROUTE);
            // If first part of route, add offset
            if ( firstRoute && !skippedLastMapID ) {
               curRequest->setIgnoreStartOffset( false );
               curRequest->setStartOffset( p->getStartOffset() );
               firstRoute = false;
            }
         
            // Set ids
            curRequest->setMapID( mapID );
         
            // Create a new PacketContainer with the packet and add to the
            // queue
            enqueuePacketContainer(new PacketContainer( curRequest, 0, 0, 
                                                        MODULE_TYPE_MAP ));
            prevMapID = mapID;
         }
      
         // Add the node id.
         curRequest->updateSize( 4, curRequest->getBufSize() * 2 );
         curRequest->addNodeID( nodeID );
      } else { // Else do nothing for route outside allowed maps
         skippedLastMapID = true;
         mc2dbg << "[GFMIR] route packets: map not allowed: "
                << prettyMapIDFill(mapID) << endl;
      }
      
      // Increase the index
      nodeIndex++;
   }

   // Add end offset.
   if ( curRequest != NULL && !skippedLastMapID ) {
      curRequest->setIgnoreEndOffset( false );
      curRequest->setEndOffset( p->getEndOffset() );
   }

   m_mapSettings->setMapContent( showMap, showTopoMap,
                                 showPOI, showRoute );
   m_mapSettings->setShowCityCentres( showCityCentres );
}


void
GfxFeatureMapImageRequest::makeGfxFeatureMapImageRequestPacket(
   uint32 size, DataBuffer* data,
   const char* copyright )
{  
   GfxFeatureMapImageRequestPacket* req = 
      new GfxFeatureMapImageRequestPacket( getID(),
                                           getNextPacketID(),
                                           m_imageFormat,
                                           m_exportFormat && !m_drawImage,
                                           m_scalable,
                                           m_drawCopyRight,
                                           size,
                                           m_sizeParam,
                                           m_minScaleLevel,
                                           m_maxScaleLevel,
                                           m_mapSettings->getSize(),
                                           copyright );
   req->setGfxFeatureMapData( size, data, m_mapSettings );
   req->setMapRotation( m_mapRotation );

   if ( m_arrowType == RouteArrowType::TURN_ARROW ) {
      // set route turn
     req->setRouteTurn( m_beforeTurn, m_afterTurn );
   }
   else if (m_arrowType == RouteArrowType::ROUTE_AS_ARROW ){
     // make the route be drawn as an arrow using the turn to focus on.
     req->setRouteAsArrow( m_afterTurn );
   }

   enqueuePacketContainer(new PacketContainer( req, 0, 0,
                                               MODULE_TYPE_GFX,
                                               5000, // Timeout in ms
                                               2 /* resend(s) */ ));
   
}

/**
 *   First all non-route/-info replies, then info replies and last route
 *   replies
 */
class PacketSorter {
  public:
    bool operator()( PacketContainer* a, PacketContainer* b) {
      uint32 aTag = a->getPacket()->getRequestTag();
      uint32 bTag = b->getPacket()->getRequestTag();
      uint32 aType = (aTag & (GFMIR_TAG_ROUTE | GFMIR_TAG_INFO));
      uint32 bType = (bTag & (GFMIR_TAG_ROUTE | GFMIR_TAG_INFO));
      if (aType != bType) // 1) GFX vs ROUTE vs INFO
         return aType < bType;
      return aTag < bTag;
    }
};

void 
GfxFeatureMapImageRequest::concatenateMapAnswers()
{
   // Check if all packets are ok
   bool allPacketsOk = true;
   StringTable::stringCode errorCode = StringTable::OK;
   for ( vector<PacketContainer*>::iterator i = m_mapReplies.begin() ; 
         i != m_mapReplies.end() && allPacketsOk ; i++ ) {
      uint32 status = static_cast<ReplyPacket*>((*i)->getPacket())->getStatus();
      if ( status != StringTable::OK &&
           ( status != StringTable::MAPNOTFOUND )) {  // should never be MAPNOTFOUND
         errorCode = StringTable::stringCode( status );
         allPacketsOk = false;
      }
   }
   if ( !allPacketsOk ) {
      
      GfxFeatureMapReplyPacket* reply = new GfxFeatureMapReplyPacket( 0U );
      reply->setStatus( errorCode );
      
      mc2log << warn << "[GFMIR] concat: not all packets were OK, returning " 
             << errorCode << endl;
      m_answer = new PacketContainer( reply, 0, 0, MODULE_TYPE_INVALID );
      m_done = true;
      
      m_status = errorCode;
      return;
   }


   // Must sort the replies so that TileMaps will work. (want the same checksum each time)
   std::sort( m_mapReplies.begin(), m_mapReplies.end(), PacketSorter() );
   
   // The map replies must be unique
   typedef set<PacketContainer*, PacketSorter> a_set;
   a_set t_set( m_mapReplies.begin(), m_mapReplies.end() );
   if ( t_set.size() != m_mapReplies.size() ) {
      mc2log << warn << "GFMIR: concatenateMapAnswers duplicates found: " 
             << endl;
      mc2log << hex << "m_mapReplies ";
      for ( uint32 i = 0 ; i < m_mapReplies.size() ; ++i ) {
         mc2log << m_mapReplies[ i ]->getPacket()->getRequestTag() << " ";
      }
      mc2log << endl;
      mc2log << "t_set        ";
      for ( a_set::const_iterator it = t_set.begin() ; it != t_set.end() ;
            ++it )
      {
         mc2log << (*it)->getPacket()->getRequestTag() << " ";
      }
      mc2log << dec << endl;
   }
   // Client sometimes sends same tile twise (G+DOH+U-Y+++7hh2iU5y 
   // G+DOG+U-Y+++7hh2iU5y) also when requesting T- and G-route tiles
   // the same request tags are duplicated sometimes. 
   // MC2_ASSERT( t_set.size() == m_mapReplies.size() );

   MC2BoundingBox bbox( m_upperLeftLat, m_upperLeftLon, 
                        m_lowerRightLat, m_lowerRightLon );

   delete m_gfmDataHolder;
   m_gfmDataHolder = new GFMDataHolder( bbox,
                                        m_screenSize.getWidth(),
                                        m_screenSize.getHeight() );

   DataBuffer* mapData = NULL;
   for ( vector<PacketContainer*>::iterator i = m_mapReplies.begin(); 
                     i != m_mapReplies.end(); ++i ) {
      GfxFeatureMapReplyPacket* packet = 
         static_cast<GfxFeatureMapReplyPacket*>((*i)->getPacket());
      bool zipped = false;
      mapData = packet->getGfxFeatureMapData( zipped );
      if ( mapData != NULL ) {
         m_gfmDataHolder->addGFMData( *i,    // Packet container.
                                      zipped,
                                      mapData,
                                      packet->getRequestTag(),
                                      // use specific copyright?
                                      m_copyright.empty() ? 
                                      packet->getCopyright() :
                                      m_copyright.c_str() );
      }
   }
   // The GFMDataHolder will delete the packetcontainers, so just 
   // clear the vector here.
   m_mapReplies.clear();
   
   // Add the symbol map.
   mapData = new DataBuffer( m_symbolMap->getMapSize() );
   m_symbolMap->save( mapData );
   m_gfmDataHolder->addGFMData( NULL,  // Packet container.
                                false, // zipped
                                mapData );


//   Packet testPacket( 50000000 );
//   int pos = 0;
//   m_gfmDataHolder->save( &testPacket, pos );
//
//   GFMDataHolder gfmHolder2;
//   pos = 0;
//   gfmHolder2.load( &testPacket, pos );
   
   
   // If you want a GfxExportFeatureMap or an Image in return
   if( m_exportFormat || m_drawImage ) {
      // FIXME, let the GfxModule merge the gfxfeaturemaps.
      DataBuffer* buf = m_gfmDataHolder->getBuffer();
      const char* copyright = m_gfmDataHolder->getCopyright().c_str();
      
      // Create the GfxFeatureMapImageRequestPacket
      makeGfxFeatureMapImageRequestPacket( buf->getCurrentOffset(), 
                                           buf, copyright );

      // If you want to get a GfxFeatureMap in return.
   } else {
   
      m_done = true;
      m_answer = NULL;
      m_status = StringTable::OK;
   } 

}

set<uint32>*
GfxFeatureMapImageRequest::getAllowedGfxMapIDs() const
{
   // Access control disabled for your convenience
   const char* prop = "SERVER_ALLOWED_GFX_REGIONS";
   const char* val = Properties::getProperty( prop );
   if ( val == NULL ) {
      return NULL;
   }

   // Parse the comma separated names of the regions.
   vector<MC2String> names;
   StringUtility::tokenListToVector( names,
                                     val,
                                     ';',
                                     true );
   
   set<uint32> allowedMaps;
   for ( vector<MC2String>::const_iterator it = names.begin();
         it != names.end();
         ++it ) {      
      const char* name = it->c_str();
      mc2dbg << "[Gfmir]: name = " << name << endl;
      const TopRegionMatch* match = m_topReq->getTopRegionByName( name );
      if ( match == NULL ) {
         continue;
      }
      mc2dbg << "TopLevelRegion : " 
             << match->getNames()->getBestName( LangTypes::swedish )->getName()
             << endl;
      const ItemIDTree& idTree = match->getItemIDTree();
      set<uint32> mapIDs;
      mc2dbg << " consists of the following items:" << endl;
      idTree.getTopLevelMapIDs( mapIDs );
      for ( set<uint32>::const_iterator jt = mapIDs.begin();
            jt != mapIDs.end(); ++jt ) {
         allowedMaps.insert(*jt);
         if ( MapBits::isOverviewMap(*jt) ) {
            allowedMaps.insert( MapBits::overviewToCountry(*jt) );
         }
         set<IDPair_t> items;
         idTree.getContents( *jt, items );
         for ( set<IDPair_t>::const_iterator kt = items.begin();
               kt != items.end(); ++kt ) {
            allowedMaps.insert( kt->first );
         }
      }
   }

   mc2dbg << STLUtility::co_dump( allowedMaps ) << endl;
         
   set<uint32>* ret = new set<uint32>;
   ret->swap( allowedMaps );
   return ret;
}

bool
GfxFeatureMapImageRequest::checkAllowedMapID( uint32 mapID ) const
{
   if ( m_specialAllowedMaps != NULL ) {
      return m_specialAllowedMaps->find( mapID ) !=
                   m_specialAllowedMaps->end();
   }
   return true;
#if 0
   if ( m_allowedMaps == NULL || m_allowedMaps->find( mapID ) != 
        m_allowedMaps->end() || 
        ( MapBits::isCountryMap( mapID ) && m_allowedMaps->find( 
           MapBits::countryToOverview( mapID ) ) != m_allowedMaps->end() ))
   {
      return true;
   } else {
      return false;
   }
#endif
}

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
#include <iomanip>
#include <sys/wait.h>
#include "TimeUtility.h"
#include "MEMapArea.h"
#include "MEGdkColors.h"  
#include "OldMapHashTable.h"
#include "MEStreetSegmentItemLayer.h" 

#include "GDFRef.h"
#include "OldNode.h"
#include "SignPostTable.h"

MEMapArea::MEMapArea(OldGenericMap* map, uint8 filterLevel) 
{ 
    
   set_events (Gdk::BUTTON_PRESS_MASK |
               Gdk::PROPERTY_CHANGE_MASK);
   // The minimum size of the window
   
   set_size_request(600, 600);

   m_map = map;
   m_filterLevel = filterLevel;

   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      m_layer[i] = NULL;
   }

   // Defalut background color.
   MEGdkColors* colors = MEGdkColors::getColors(); 
   m_bgColor = colors->m_lightYellow2; 
 
   m_initialized = false;
   m_gdfRef = NULL;
   m_keepHighlightsOnSelect = false;
}

bool
MEMapArea::itemAmongShownLayers(uint32 id)
{
   bool retVal = false;
   OldItem* item = m_map->itemLookup(id);

   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if( m_layer[i] != NULL) {
         if ((item != NULL) && (uint32(item->getItemType()) == i)) {
            retVal = true;
         }
      }
   }

   return retVal;
}

bool
MEMapArea::highlightItem(uint32 id, bool clearFirst, bool locate, 
                       bool redrawScreen, MEGdkColors::color_t color) 
{
   // Initiate the returnvalue (if the id is present among the 
   // shown layers).
   OldItem* item = m_map->itemLookup(id);
   bool returnValue = false;

   // Handle color for point symbol.
   MEGdkColors* cols = MEGdkColors::getColors();
   Gdk::Color pointColor = cols->m_black;
   if ( color != MEGdkColors::invalidColor ){
      pointColor = cols->getColor(color);
   }

   MEGdkColors::color_t highlightColor = color;
   if ( color == MEGdkColors::invalidColor ) {
      highlightColor = MEGdkColors::red;
   }
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if( m_layer[i] != NULL) {
         if (clearFirst) {
            m_layer[i]->clearAllHighlight();
         }

         // Handle symbol for POIs.
         OldPointOfInterestItem* poi = 
            dynamic_cast<OldPointOfInterestItem*>(item);
         if ( poi != NULL && !locate ){
            GfxData* gfx = poi->getGfxData();
            if ( gfx != NULL ){
               if ( gfx->getNbrPolygons() > 0 && 
                    gfx->getNbrCoordinates(0) > 0 ){
                  highlightCoordinate(gfx->getLat(0,0), gfx->getLon(0,0),
                                      pointColor, 
                                      MEItemLayer::diamondSymbol, 
                                      2, // lineWidth
                                      false );
               }
            }
         }
            

         m_layer[i]->addHighlightID(id, highlightColor, 3, locate);
         if ((item != NULL) && (uint32(item->getItemType()) == i)) {
            returnValue = true;
         }
      }
   }
   if (returnValue && redrawScreen){
      redraw();
   }
   return (returnValue);
}

bool 
MEMapArea::highlightConnection(uint32 toNodeID, uint32 fromNodeID) 
{

   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      MERouteableItemLayer* rl =
         dynamic_cast<MERouteableItemLayer*>(m_layer[i]);
      if( rl != NULL) {
         rl->highlightConnection(toNodeID, fromNodeID);
      }
   }

   redraw();
   return true;
}

bool 
MEMapArea::highlightCoordinate(int32 lat, int32 lon, Gdk::Color color,
                             MEItemLayer::symbol_t symbol,
                             int lineWidth, 
                             bool redrawScreen)
{
   MEItemLayer* layer = m_layer[int(ItemTypes::nullItem)];
   if(layer != NULL) {
      layer->addHighlightCoordinate(lat, lon, color, lineWidth, symbol);
      if (redrawScreen) {
         //redraw();
         redraw_added_highlight();
      }
      return (true);
   }
   return (false);
}

bool 
MEMapArea::highlightCoordinates(GfxData* gfxData)
{
   if (gfxData == NULL) {
      mc2dbg1 << "Selected item has no gfxData" << endl;
      return false;
   }

   bool ok = true;
   MEGdkColors* cols = MEGdkColors::getColors();
   clearCoordinateHighlight(false);
   uint32 nbrCoordsHighlighted = 0;
   
   Gdk::Color color = cols->m_blue;
   for( uint32 p = 0; p < gfxData->getNbrPolygons(); ++p ) {
      GfxData::const_filter_iterator polyEnd =
         gfxData->endFilteredPoly(p, m_filterLevel);
      for( GfxData::const_filter_iterator it =
              gfxData->beginFilteredPoly(p, m_filterLevel);
           it != polyEnd; ++it) {
         MC2Coordinate currCoord = *it;
         
         ok = highlightCoordinate(currCoord.lat, currCoord.lon,
                                  color, MEItemLayer::crossSymbol,
                                  2, false);
         if ( color == cols->m_blue )
            color = cols->m_orange;
         else if ( color == cols->m_orange )
            color = cols->m_black;
         if( !ok )
            goto exit;
         nbrCoordsHighlighted++;
      }
   }
  exit:
   
   // Redraw secreen if it is updated.
   if (ok)
      redraw();

   if ( m_filterLevel > 0 ) {
      mc2dbg4 << "Filter level " << int(m_filterLevel) << ": "
              << nbrCoordsHighlighted << " coords highlighted" << endl;
   }
   return (ok);
}

void 
MEMapArea::clearCoordinateHighlight(bool redrawScreen) {
   if( m_layer[ItemTypes::nullItem] != NULL) {
      m_layer[int(ItemTypes::nullItem)]->clearCoordinateHighlight();
      if (redrawScreen) {
         redraw();
      }
   }
}

void 
MEMapArea::clearAllHighlight() {
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if( m_layer[i] != NULL) {
         m_layer[i]->clearAllHighlight();
      }
   }
   redraw();
}

MEStreetSegmentItemLayer* 
MEMapArea::getStreetSegmentItemLayer() 
{
   return dynamic_cast<MEStreetSegmentItemLayer*>
                      (m_layer[uint32(ItemTypes::streetSegmentItem)]);
}

bool 
MEMapArea::getConnectionCosts(OldConnection* conn, OldNode* toNode,
                            uint32& costA, uint32& costB,
                            uint32& costC, uint32& costD,
                            bool externalConnection) 
{
   return (m_map->getConnectionCost(conn, toNode, costA, costB,
                                    costC, costD, externalConnection));
}

const vector<GMSLane>
MEMapArea::getLanes(const OldNode& node) const
{
   return node.getLanes( *m_map );
   
} // getLanes

uint32
MEMapArea::getConnectingLanes(const OldNode& toNode, 
                              const OldConnection& connection) const
{
   return toNode.getConnectedLanes( *m_map, connection.getConnectFromNode() );
   
} // getLanes

const vector<GMSSignPost*>&
MEMapArea::getSignPosts(const OldNode& toNode, 
                        const OldConnection& connection) const
{
   const SignPostTable& signPostTable = m_map->getSignPostTable();
   return signPostTable.getSignPosts( connection.getFromNode(), 
                                      toNode.getNodeID() );
   
} // getLanes


void 
MEMapArea::printSignPosts(const OldNode& toNode, 
                    const OldConnection& 
                    connection) const
{
   const vector<GMSSignPost*>& signPosts = 
      getSignPosts(toNode, connection);
   for (uint32 i=0; i<signPosts.size(); i++){
      mc2log << "Sign post:" << i << endl;
      signPosts[i]->debugPrint(mc2log, m_map);
      mc2log << endl;
   }
}

void
MEMapArea::adjustBBox(bool keepCenter)
{
   m_bboxLatLon.updateCosLat();
   // Adjust to fit with the current size of this window
   float64 factor = (float64(m_bboxLatLon.getWidth()) / float64(get_width())) /
                    (float64(m_bboxLatLon.getHeight()) / float64(get_height()));
   mc2dbg4  << "Factor before: " << factor << endl;

   if (keepCenter) {
      int32 cLat, cLon;
      m_bboxLatLon.getCenter(cLat, cLon);
      if (factor > 1.0) {
         // Use the width to set m_bboxLatLon
         mc2dbg4 << "To increase the height " << factor << " times..." << endl;
         uint32 newH = uint32(float64(m_bboxLatLon.getHeight()) * factor);
         m_bboxLatLon.setMaxLat(cLat + newH/2);
         m_bboxLatLon.setMinLat(cLat - newH/2);
      } else {
         // Use the height to set m_bboxLatLon
         mc2dbg4 << "To increase the width " << 1.0 / factor << " times..." 
                 << endl;
         uint32 newW = uint32(m_bboxLatLon.getLonDiff() / factor);
         m_bboxLatLon.setMaxLon(cLon + newW/2);
         m_bboxLatLon.setMinLon(cLon - newW/2);
      }
   } else {
      if (factor > 1.0) {
         // Use the width to set m_bboxLatLon
         mc2dbg4 << " 1 factor=" << factor << endl;
         uint32 newW = uint32(m_bboxLatLon.getLonDiff() / factor);
         m_bboxLatLon.setMaxLon(m_bboxLatLon.getMinLon() + newW);
      } else {
         // Use the height to set m_bboxLatLon
         mc2dbg4 << " 2 factor=" << factor << endl;
         uint32 newH = uint32(float64(m_bboxLatLon.getHeight()) * factor);
         m_bboxLatLon.setMinLat(m_bboxLatLon.getMaxLat() - newH);
      }
   }
   m_bboxLatLon.updateCosLat();
   factor = (float64(m_bboxLatLon.getWidth()) / float64(get_width())) /
            (float64(m_bboxLatLon.getHeight()) / float64(get_height()));
   mc2dbg4 << "Factor afterwards: " << factor << " bbox.height()=" 
           << m_bboxLatLon.getHeight() << ", bbox.width()=" 
           << m_bboxLatLon.getWidth() << endl;
   // reset background
   Glib::RefPtr<Gdk::Pixmap> pixmap(0);
   m_window = get_window();
   m_window->set_back_pixmap(pixmap, false);
   m_window->set_background(m_bgColor);
   m_window->clear();
}

void 
MEMapArea::on_realize() 
{
   // we need to do the default realize
   Gtk::DrawingArea::on_realize();
   
   // Initiate the bounding-box to the whole map
   m_map->getGfxData()->getMC2BoundingBox(m_bboxLatLon);
   if ( (m_bboxLatLon.getMaxLat() == MAX_INT32) ||
        (m_bboxLatLon.getMinLat() == MAX_INT32) ) {
      m_bboxLatLon.update(664700000, 157350063);
      m_bboxLatLon.update(664900000, 157350063);
   }
   
   // Now we can allocate any additional resources we need
   m_window = get_window();

   // set background
   m_window->set_background(m_bgColor);
   m_window->clear();
   adjustBBox();

   setDefaultDrawing();

   // Redraw
   recalculateAndRedraw();
   m_initialized = true;
}

void
MEMapArea::setDefaultDrawing(){
   // Add the default layer(s)
   mc2dbg4 << here << " Adding default item-layers to maparea" << endl;
   addItemLayer(ItemTypes::streetSegmentItem, false, MEGdkColors::black);
   addItemLayer(ItemTypes::ferryItem, false, MEGdkColors::blue);
   addItemLayer(ItemTypes::nullItem, false, MEGdkColors::black);
   // This is actually the map boudary
   addItemLayer(ItemTypes::routeableItem, false, MEGdkColors::orange);
   // Add these layers now (not drawn|selectable but possible to search 
   // for items without having to activate the layers via Drawsettings..)
   addItemLayer(ItemTypes::streetItem, false,  MEGdkColors::orange);

   m_layer[int(ItemTypes::streetSegmentItem)]->m_highlightedOnly =
      false;
}

void
MEMapArea::addItemLayer(ItemTypes::itemType t, 
                      bool recalc,
                      MEGdkColors::color_t defaultColor,
                      MEGdkColors::color_t defaultHighlightColor,
                      bool filled)
{
   m_window = get_window();

   // Make sure that the layer is empty.
   if (m_layer[uint32(t)] == NULL) {
      if (t == ItemTypes::streetSegmentItem) {
         // Handle SSI's special
         mc2dbg4 << "Adding SSI-layer" << endl;
         m_layer[uint32(ItemTypes::streetSegmentItem)] = 
            new MEStreetSegmentItemLayer(m_window,
                                       MEGdkColors::black, 
                                       MEGdkColors::red,
                                       m_filterLevel);
      } else if (t == ItemTypes::ferryItem) {
         mc2dbg4 << "Adding ferry-layer" << endl;
         m_layer[uint32(ItemTypes::ferryItem)] = 
            new MERouteableItemLayer(m_window,
                                   ItemTypes::ferryItem,
                                   MEGdkColors::blue, 
                                   MEGdkColors::red,
                                   m_filterLevel);
      } else {
         mc2dbg2 << "Adding layer with type=" << int(t) << endl;
         m_layer[uint32(t)] = new MEItemLayer(m_window, t, 
                                            defaultColor, 
                                            defaultHighlightColor,
                                            m_filterLevel);
      }
      setItemLayerParameters(t, filled, defaultColor);
      if (recalc)
         recalculateAndRedraw();
   } else {
      mc2dbg4 << "Items alreay shown, t=" << int(t) << endl;
   }
}

void 
MEMapArea::removeItemLayer(ItemTypes::itemType t, bool recalc)
{
   // The nullItem-layer is used when highlighting coordinates...
   if (t != ItemTypes::nullItem) {
      // Make sure that the layer is empty.
      delete m_layer[uint32(t)];
      m_layer[uint32(t)] = NULL;
      if (recalc)
         recalculateAndRedraw();
   }
}

void 
MEMapArea::setItemLayerParameters(ItemTypes::itemType t, bool filled, 
                                MEGdkColors::color_t col)
{
   // Make sure that the layer is empty.
   if (m_layer[uint32(t)] != NULL) {
      m_layer[uint32(t)]->setParameters(filled, col);
      mc2dbg4 << "To set parameters for layer m_layer[" << uint32(t) 
             << "]" << endl;
   }
}

bool 
MEMapArea::isLayerActive(ItemTypes::itemType t)
{
   return (m_layer[uint32(t)] != NULL);
}

bool  
MEMapArea::isLayerFilled(ItemTypes::itemType t)
{
   if (m_layer[uint32(t)] != NULL) {
      return m_layer[uint32(t)]->isFilled();
   }
   return false;
}

MEGdkColors::color_t
MEMapArea::getDrawColor(ItemTypes::itemType t)
{
   if (m_layer[uint32(t)] != NULL) {
      return m_layer[uint32(t)]->getDrawColor();
   }
   return MEGdkColors::invalidColor;
}

bool
MEMapArea::on_expose_event(GdkEventExpose* e)
{ 
   // Hack to make sure we don't redraw too often...
   static uint32 lastRedrawTime = 0; 
   uint32 now = TimeUtility::getCurrentTime(); 

   mc2dbg4 << "e->width=" << e->area.width << ", width()=" << get_width() 
           << ", e->height=" << e->area.height << ", height()=" << get_height() 
           << endl; 
   if ( (e->count == 0) && (true /*(now-lastRedrawTime) > 500*/) ) { 
      lastRedrawTime = now; 
       //Redraw!
      mc2dbg4 << "expose_event Redrawing, e->count=" << e->count << endl; 
      m_window = get_window(); 
      
      // Keep upper-left 
      adjustBBox(false);     // Only necessary if/when size changed! 
      redraw();
   } else {
      mc2dbg4 << here << " Did not redraw, e->count=" << e->count
              << ", timeDiff=" << (now-lastRedrawTime) << endl;
   }

   return true;
}

bool
MEMapArea::on_button_press_event (GdkEventButton* e)
{
   m_window = get_window();

   // Calculate the position of the mouse-click in (lat,lon)
   int32 latDiff = m_bboxLatLon.getHeight();
   int32 latClick = m_bboxLatLon.getMaxLat() - 
                    int32(rint(float64(latDiff) * float64(e->y) / 
                               float64(get_height())));
   int32 lonDiff = m_bboxLatLon.getLonDiff();
   int32 lonClick = m_bboxLatLon.getMinLon() + 
                    int32(rint(float64(lonDiff) * float64(e->x) / 
                               float64(get_width())));
   
   mc2dbg4 << "button_press at (" << e->x << "," << e->y << ") = "
           << "(" << latClick << "," << lonClick << ")" << endl;
   
   // Check what mouse-button that was clicked
   if ( (e->button == 1) || (e->button == 3)) {
      // Left == 1 == zoom in  or right == 3 == zoom out

      float64 zoomFactor = 1.0;
      if (e->button == 1) {
         // Zoom in
         zoomFactor = 0.4;
      } else {
         // Zoom out
         zoomFactor = 1.6;
      }

      // Calucalte max and min latitude
      const int32 deltaLat = int32(rint(latDiff * zoomFactor)) / 2;
      m_bboxLatLon.setMaxLat(latClick + deltaLat);
      m_bboxLatLon.setMinLat(latClick - deltaLat);

      const int32 deltaLon = uint32(rint(lonDiff * zoomFactor)) / 2;
      m_bboxLatLon.setMaxLon(lonClick + deltaLon);
      m_bboxLatLon.setMinLon(lonClick - deltaLon);
      adjustBBox();
      recalculateAndRedraw();

   } else if (e->button == 2) {
      mc2dbg4 << "MIDDLE (getItem)" << endl;
   
      OldMapHashTable* mapHashTable = m_map->getHashTable();
      if (mapHashTable != NULL) {
         // Add allowed itemtypes  ( == shown layers) 
         mapHashTable->clearAllowedItemTypes();
         for (uint32 i=0; i<int(ItemTypes::numberOfItemTypes); i++) {
            if (m_layer[i] != NULL) 
               mapHashTable->addAllowedItemType(ItemTypes::itemType(i));
         }

         // Get the closest item
         uint64 dist = 0;
         uint32 closestID = mapHashTable->getClosest(lonClick, latClick, dist);
         float64 distMeter = (sqrt(dist) * GfxConstants::MC2SCALE_TO_METER);

         set<uint32> closeIDs;
         mapHashTable->getAllWithinRadius(closeIDs,lonClick, latClick, 50);
         set<uint32>::const_iterator idIt = closeIDs.begin();
         mc2dbg << "Close IDs: ";
         while (idIt != closeIDs.end()){
            MC2String name = "NULL";
            if (m_map->getBestItemName(*idIt) != NULL) 
               name = m_map->getBestItemName(*idIt);
            mc2dbg << *idIt << " \"" << name << "\" ";
            ++idIt;
         }
         mc2dbg << endl;

         // Add highlighted ID to all layers
         for (uint32 i=0; i<int(ItemTypes::numberOfItemTypes); i++) {
            if (m_layer[i] != NULL) {
               if ( ! m_keepHighlightsOnSelect ) {
                  m_layer[i]->clearAllHighlight();
               }
               m_layer[i]->addHighlightID(closestID);
            }
         }
         // Redraw the screen.
         redraw();
         
         OldItem* item = m_map->itemLookup(closestID);
         if (item != NULL) {
            const char* itemName = "-no name-";
            if ( item->getNbrNames() > 0 ) {
               itemName = StringUtility::newStrDup(m_map->getFirstItemName(item));
            }
            mc2log << info << "Found \"" << itemName << "\" (" 
                   << StringTable::getString(
                        ItemTypes::getItemTypeSC(item->getItemType()),
                        StringTable::ENGLISH) << " with ID=" 
                   << closestID << "), sqDistMC2=" << dist
                   << " distMeter=" << distMeter << endl;
            if (item->getGfxData() != NULL) {
               mc2log << "nbr polys=" << item->getGfxData()->getNbrPolygons()
                      << " total nbr coords="
                      << item->getGfxData()->getTotalNbrCoordinates() 
                      << " closed=" << int(item->getGfxData()->getClosed(0))
                      << endl;
            }

            
            // Show item-info, SSI is a bit special...
            MEItemInfoDialog* itemInfoDialog =
               MEItemInfoDialog::instance(this);
            itemInfoDialog->setItemToShow(item, m_map);
         } else {
            MEItemInfoDialog* itemInfoDialog =
               MEItemInfoDialog::instance(this);
            itemInfoDialog->hide();
         }
      }
   } else {
      mc2dbg << "button=" << e->button << " unhandled" << endl;
   }

   return 0;
}

bool
MEMapArea::on_configure_event (GdkEventConfigure* p0) 
{
   if (m_initialized) {
      recalculate();
   }
   return (0);
}

void 
MEMapArea::zoomToBBox(int32 northLat, int32 westLon, 
                    int32 southLat, int32 eastLon)
{

   m_bboxLatLon.setMaxLat(northLat);
   m_bboxLatLon.setMinLat(southLat);
   m_bboxLatLon.setMaxLon(eastLon);
   m_bboxLatLon.setMinLon(westLon);
   //m_bboxLatLon.dump();
   // Adjust to fit with the current size of this window
   adjustBBox();
   //m_bboxLatLon.dump();

   
   recalculateAndRedraw();
}


void 
MEMapArea::recalculateAndRedraw() 
{
   recalculate();
   redraw();
}

void 
MEMapArea::recalculate() 
{
   mc2dbg4 << here << " Recalculating all items..." << endl;
   uint32 start = TimeUtility::getCurrentMicroTime();
   // Recreate all layers
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if( m_layer[i] != NULL) {
         m_layer[i]->setFilterLevel(m_filterLevel);
         m_layer[i]->createItems(m_map, 
                                 m_bboxLatLon, 
                                 get_width(), 
                                 get_height());
      }
   }
   uint32 end = TimeUtility::getCurrentMicroTime();
   mc2dbg4 << "All items created in " << (end-start)/1000.0 << "ms" << endl;
}

uint32 MEMapArea::m_layerDrawingOrder[] = {
   ItemTypes::municipalItem,
   ItemTypes::builtUpAreaItem,
   ItemTypes::cityPartItem,
   ItemTypes::zipCodeItem,
   ItemTypes::waterItem,
   ItemTypes::parkItem,
   ItemTypes::forestItem,
   ItemTypes::pedestrianAreaItem,
   ItemTypes::militaryBaseItem,
   ItemTypes::airportItem,
   ItemTypes::buildingItem,
   ItemTypes::individualBuildingItem,
   ItemTypes::aircraftRoadItem,
   ItemTypes::railwayItem,
   ItemTypes::islandItem,
   ItemTypes::streetSegmentItem,
   ItemTypes::streetItem,
   ItemTypes::nullItem,
   ItemTypes::pointOfInterestItem,
   ItemTypes::categoryItem,
   ItemTypes::busRouteItem,
   ItemTypes::ferryItem,
   ItemTypes::subwayLineItem,
   ItemTypes::routeableItem,  // map boundry
   ItemTypes::borderItem,
   ItemTypes::cartographicItem
};

void 
MEMapArea::redraw_added_highlight() 
{
   mc2dbg4 << here << " Redrawing all highlight..." << endl;
   
   // Clear the window and draw all layers
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if( m_layer[m_layerDrawingOrder[i]] != NULL) {
         m_layer[m_layerDrawingOrder[i]]->drawAllHighlight();
      }
   }
}

void 
MEMapArea::redraw() 
{
   mc2dbg4 << here << " Redrawing all items..." << endl;
   uint32 start = TimeUtility::getCurrentMicroTime();
 
   // Clear the window and draw all layers
   
   m_window = get_window();
   m_window->clear();
   
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if( m_layer[m_layerDrawingOrder[i]] != NULL) {
         m_layer[m_layerDrawingOrder[i]]->drawItems();
      }
   }
   uint32 end = TimeUtility::getCurrentMicroTime();
   mc2dbg4 << "All items redrawn in " << (end-start)/1000.0 << "ms" << endl;
}

void
MEMapArea::verifyMaxMin(int32& maxL, int32& minL, int32 delta,
                      int32 maxAllowedL, int32 minAllowedL)
{
   if ((maxL > maxAllowedL) && (minL < minAllowedL)) {
      // Both maxL and minL invalid
      mc2dbg4 << "L case 1" << endl;
      maxL = maxAllowedL;
      minL = minAllowedL;
   } else if (maxL > maxAllowedL) {
      // maxL invalid
      mc2dbg4 << "L case 2" << endl;
      maxL = maxAllowedL;
      minL = maxL - delta;
   } else if (minL < minAllowedL) {
      // minL invalid
      mc2dbg4 << "L case 3" << endl;
      minL = minAllowedL;
      maxL = minL + delta;
   }
   maxL = MIN(maxL, maxAllowedL);
   minL = MAX(minL, minAllowedL);
}

const char*
MEMapArea::getMapCountryName()
{
   const char* countryName = "";
   if (m_map != NULL) {
      StringTable::countryCode country = m_map->getCountryCode();
      if (country < StringTable::NBR_COUNTRY_CODES) {
         countryName = StringTable::getString(
                     StringTable::getCountryStringCode(country),
                     StringTable::ENGLISH);
         // Replace any space-chars with "_"
         char* newCountryStr = StringUtility::replaceString(
                  countryName, " ", "_");
         if ( newCountryStr != NULL ) {
            return newCountryStr;
         } else {
            delete newCountryStr;
            return countryName;
         }
      }
   }
   return countryName;
}

void
MEMapArea::setGdfRef(GDFRef* gdfRef)
{
   m_gdfRef = gdfRef;
}

GDFID
MEMapArea::getGdfID(const OldItem* item)
{
   if ( m_gdfRef != NULL ){
      mc2dbg8 << "m_gdfRef set" << endl;
      return m_gdfRef->getGdfID(item);
   }
   else {
      mc2dbg8 << "m_gdfRef not set" << endl;
      GDFID gdfID; // invalid
      return gdfID; 
   }
}

bool
MEMapArea::loadedGdfRef(){
   if ( m_gdfRef != NULL ){
      return true;
   }
   else {
      return false;
   }
}

uint32
MEMapArea::highlightFromGdfID(uint32 gdfFeatID){
   set<uint32> itemIDs = m_gdfRef->getAllMcmIDs(gdfFeatID);
   set<uint32>::const_iterator itemIt = itemIDs.begin();
   while ( itemIt != itemIDs.end() ){
      mc2dbg << "GDF ID highlight item ID: " << *itemIt << endl;
      highlightItem(*itemIt,
                    true, // clearFirst
                    false, // locate
                    false // redrawScreen
                    );
      ++itemIt;
   }
   redraw();

   if (itemIDs.size() == 1){
      return *itemIDs.begin();
   }
   else {
      return MAX_UINT32;
   }
}

void
MEMapArea::setBgColor(MEGdkColors::color_t color){
   MEGdkColors* cols = MEGdkColors::getColors();
   m_bgColor = cols->getColor(color);
   m_window->set_background(m_bgColor);
}

void
MEMapArea::setKeepHighlightsOnSelect(bool keep)
{
   m_keepHighlightsOnSelect = keep;
}

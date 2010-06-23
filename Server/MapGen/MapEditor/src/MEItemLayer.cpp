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
#include "MEItemLayer.h"
#include "MEGdkColors.h"
#include "MC2BoundingBox.h"
#include "GfxData.h"
#include "MapBits.h"


// ===================================================================
//                                                    MEDrawableItem =

MEItemLayer::MEDrawableItem::MEDrawableItem(MEItemLayer* itemLayer,
                                      uint32 nbrCoords,
                                      OldItem* item,
                                      bool closed)
   : m_item(item),
     m_nbrPoints(0),
     m_closed(closed),
     m_itemLayer(itemLayer)
{
   if (nbrCoords > 0)
   m_points.reserve(nbrCoords);
   else 
     m_points.reserve(0);
}

MEItemLayer::MEDrawableItem::~MEDrawableItem() 
{
}

void
MEItemLayer::MEDrawableItem::addCoordinate(gint x, gint y)
{
   // Extremely dangerous... Assume that not to many coordinates are added
   // and that they only are added once.
   m_points.push_back( Gdk::Point( x, y ) );
   ++m_nbrPoints;
}

void 
MEItemLayer::MEDrawableItem::draw(Glib::RefPtr<Gdk::GC>& gc, 
				  Glib::RefPtr<Gdk::Window>& w) 
{ 
   if (allCoordsSame()) {
      w->draw_point(gc, m_points[0].get_x(), m_points[0].get_y());
   } 
   else if (m_closed) {
     w->draw_polygon(gc, m_itemLayer->m_filled, m_points);
   } else {
     w->draw_lines(gc, m_points);
   }
}

void
MEItemLayer::MEDrawableItem::locate(Glib::RefPtr<Gdk::GC>& gc, Gdk::Window& w)
{
   // Get the bounding box
   gint maxX, minX, maxY, minY;
   getBoundingBox(maxX, minX, maxY, minY);

   int r = MAX(maxX-minX+25, 100);
   r = MAX(maxY-minY+25, r);
   mc2dbg8 << "   r = " << r << endl;
   
   int x = m_points[0].get_x()-r;
   int y = m_points[0].get_y()-r;
   w.draw_rectangle(gc, false, x, y, 2*r, 2*r);

   mc2dbg << "MEItemLayer::MEDrawableItem::locate (x,y)=(" 
          << x << "," << y << ")" << endl;
}


void 
MEItemLayer::MEDrawableItem::highlightCoord(Glib::RefPtr<Gdk::GC>& gc, 
					    Glib::RefPtr<Gdk::Window>& w, 
					    uint32 i) 
{
   if (i < m_nbrPoints) {
      // Valid i-value
      int x = m_points[i].get_x()-HIGHLIGHT_SQ_SIZE/2;
      int y = m_points[i].get_y()-HIGHLIGHT_SQ_SIZE/2;
      w->draw_rectangle(gc, true, x, y,
                       HIGHLIGHT_SQ_SIZE, HIGHLIGHT_SQ_SIZE);
   }
}

void 
MEItemLayer::MEDrawableItem::highlightFirstCoord(Glib::RefPtr<Gdk::GC>& gc, 
						 Glib::RefPtr<Gdk::Window>& w) 
{
   highlightCoord(gc, w, 0);
}

void 
MEItemLayer::MEDrawableItem::highlightLastCoord(Glib::RefPtr<Gdk::GC>& gc, 
						Glib::RefPtr<Gdk::Window>& w) 
{
   highlightCoord(gc, w, m_nbrPoints-1);
}

void
MEItemLayer::MEDrawableItem::getBoundingBox(gint& maxX, gint& minX,
                                        gint& maxY, gint& minY)
{
   maxX = maxY = 0;
   minX = minY = 65000;

   for (uint32 i=0; i<m_nbrPoints; i++) {
      gint x = m_points[i].get_x() /* - HIGHLIGHT_SQ_SIZE/2*/;
      gint y = m_points[i].get_y() /* - HIGHLIGHT_SQ_SIZE/2*/;
      if (maxX < x)
         maxX = x;
      if (minX > x)
         minX = x;
      if (maxY < y)
         maxY = y;
      if (minY > y)
         minY = y;
   }
}

void
MEItemLayer::MEDrawableItem::getBoundingBox(MC2BoundingBox& bb)
{
   bb.reset();

   for (uint32 i=0; i<m_nbrPoints; i++) {
      bb.update(m_points[i].get_x(), m_points[i].get_y(), false);
   }
   // Do not update cos-lat, since not valid anyway (square!).
}


bool 
MEItemLayer::MEDrawableItem::allCoordsSame()
{
   if (m_nbrPoints < 1){
      return false;
   }
   gint firstX = m_points[0].get_x();
   gint firstY = m_points[0].get_y();
   
   bool differs = false;
   uint32 i=0;
   while ( (i< m_nbrPoints) and (!differs) ){
      differs = (m_points[i].get_x() != firstX);
      if (!differs){
         differs = (m_points[i].get_y() != firstY);
      }
      i++;
   }
   return !differs;
}

MEItemLayer::MEItemLayer(Glib::RefPtr<Gdk::Window>& window, 
                     ItemTypes::itemType type,
                     MEGdkColors::color_t defaultColor, 
                     MEGdkColors::color_t defaultHighlightColor,
                     uint8 filterLevel)
   : m_window(window), m_itemType(type)
{
   MEGdkColors* colors = MEGdkColors::getColors();
   m_highlightedOnly = false;
   m_filled = true;
   m_drawColor = defaultColor;
   m_filterLevel = filterLevel;
  
   m_mainGC = Gdk::GC::create(window);
   m_mainGC->set_foreground(colors->getColor(defaultColor));
   m_mainGC->set_fill(Gdk::SOLID);


   m_highlightGC=Gdk::GC::create(window);
   m_highlightGC->set_line_attributes(3, 
				      Gdk::LINE_SOLID,
				      Gdk::CAP_NOT_LAST,
				      Gdk::JOIN_BEVEL);
   m_highlightGC->set_foreground(colors->getColor(defaultHighlightColor));
}

MEItemLayer::~MEItemLayer() {
   mc2log << error << here << " MUST delete allocated memory!" << endl;
}

void
MEItemLayer::setParameters(bool filled, MEGdkColors::color_t col)
{
   m_filled = filled;
   MEGdkColors* colors = MEGdkColors::getColors();
   m_mainGC->set_foreground(colors->getColor(col));
   m_drawColor = col;
}

void 
MEItemLayer::drawItems() 
{
   mc2dbg4 << "MEItemLayer::drawItems To draw " << m_allItems.size() 
           << " items" << endl;

   // Draw all items
   multimap<uint32, MEDrawableItem*>::iterator p = m_allItems.begin();
   while (p != m_allItems.end()) {
      mc2dbg8 << "Drawing item..." << endl;
      p->second->draw(m_mainGC, m_window);
      p++;
   }

   // Draw highlighted items afterwards to make sure that they are on top
   drawAllHighlight();
}


void
MEItemLayer::createItems(const OldGenericMap* genericMap,
                       MC2BoundingBox& bboxLatLon,
                       gint16 hPixels, gint16 vPixels)
{
   mc2dbg4 << "Creating items" << endl;
   
   // Delete all items in the m_allItems
   if (!m_allItems.empty()) {
      mc2dbg4 << "m_allItems.size()=" << m_allItems.size() << endl;
      multimap<uint32, MEDrawableItem*>::iterator p = m_allItems.begin();
      while (p != m_allItems.end()) {
         mc2dbg8 << "Deleting item..." << endl;
         delete (p->second);
         p++;
      }
      m_allItems.clear();
   }
   
   // Calculate the scale-factors
   m_latScaleFactor = float64(vPixels) / bboxLatLon.getHeight();
   m_lonScaleFactor = float64(hPixels) / bboxLatLon.getLonDiff();
   float64 factor = 
      float64(bboxLatLon.getHeight()) / float64(bboxLatLon.getWidth()) *
      float64(hPixels) / float64(vPixels);
   mc2dbg8 << here << " factor == " << factor << endl;
   if (factor > 1) {
      // Compensate for that the image is wider than it is high
      m_latScaleFactor *= factor;
   } else {
      // Compensate for that the image is higher than it is wide
      m_lonScaleFactor /= factor;
   }
   
   m_latSubFactor = bboxLatLon.getMaxLat();
   m_lonSubFactor = bboxLatLon.getMinLon();

   // Add new Gdk_Points-object to the list
   MC2BoundingBox bb;
   if (m_itemType != ItemTypes::routeableItem) {
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<genericMap->getNbrItemsWithZoom(z); i++) {
            OldItem* curItem = genericMap->getItem(z, i);
            if ( (curItem != NULL) && 
                 (curItem->getGfxData() != NULL) &&
                 (curItem->getItemType() == m_itemType) ) {
               mc2dbg4 << "Checking item with ID=0x" << hex << curItem->getID() 
                       << dec << endl;

               GfxData* gfx = curItem->getGfxData();
               // Check that the bboxes overlaps
               gfx->getMC2BoundingBox(bb);
               if (bb.overlaps(bboxLatLon))
                  createDrawableFromGfxData(gfx, curItem);
            }
         }
      }
   } else {
      // Show the map-border when we are at the routeableItem
      GfxData* gfx = (GfxData*) genericMap->getGfxData();

      // Check that the bboxes overlaps
      MC2BoundingBox bb;
      gfx->getMC2BoundingBox(bb);
      if (bb.overlaps(bboxLatLon)) {
         createDrawableFromGfxData( gfx, NULL,
            MapBits::isCountryMap( genericMap->getMapID()) );
      }
   }
}

void
MEItemLayer::createDrawableFromGfxData(GfxData* gfx, OldItem* item, bool print )
{
   // Loop over all polygons
   for (uint32 p=0; p<gfx->getNbrPolygons(); p++) {
      uint32 nbrCoord = gfx->getNbrCoordinates(p);
 
      MEDrawableItem* drawable = new MEDrawableItem(this, nbrCoord, 
                                                item, gfx->closed());
      uint32 nbrC = 0;
      GfxData::const_filter_iterator polyEnd =
         gfx->endFilteredPoly(p, m_filterLevel);
      for(GfxData::const_filter_iterator it =
             gfx->beginFilteredPoly(p, m_filterLevel);
          
          it != polyEnd; ++it) {
         MC2Coordinate currCoord = *it;
         drawable->addCoordinate(getGtkX(currCoord.lon),
                                 getGtkY(currCoord.lat));
         nbrC++;
      }
      if ( print && (p == 0) && (m_filterLevel > 0) ) {
         mc2dbg4 << "country pol poly 0, filt level " << int(m_filterLevel)
              << " nbrCoord = " << nbrC << " of " 
              << gfx->getNbrCoordinates(p) << endl;
      }
      
      // Create the Gdk_Points-object and insert that into m_allItems
      uint32 itemID = MAX_UINT32;
      if (item != NULL) 
         itemID = item->getID();
      m_allItems.insert( pair<uint32, MEDrawableItem*>(itemID, drawable) );

    }
}


bool 
MEItemLayer::highlightItemWithIDs(Vector& itemIDs,
                                MEGdkColors::color_t color,
                                int lineWidth,
                                bool locate)
{
   clearAllHighlight();

   for (uint32 i=0; i<itemIDs.getSize(); i++) {
      addHighlightID(itemIDs[i], color, lineWidth, locate);
   }
   
   return (true);
}

void 
MEItemLayer::drawAllHighlight()
{
   drawHighlightedItems();
   drawHighlightedCoordinates();
}

void
MEItemLayer::addHighlightID(uint32 itemID, MEGdkColors::color_t color, 
                          int lineWidth, bool locate)
{
   // Add to the vector
   MEGdkColors* colorObj = MEGdkColors::getColors();
   m_highlightedItems.push_back(
      (struct highlightitem_t) { itemID, lineWidth, 
                                 colorObj->getColor(color), locate} );

   mc2dbg8 << here << " To highlight " << m_highlightedItems.size() 
           << " items" << endl;
}



void
MEItemLayer::addHighlightCoordinate(int32 lat, int32 lon,
                                  Gdk::Color color,
                                  int lineWidth, symbol_t symbol)
{
   // Add to the highlightCoordinates-vector.
   m_highlightedCoordinates.push_back( 
         (struct highlightcoord_t) {lat, lon, symbol, lineWidth, color} );
}

void 
MEItemLayer::clearAllHighlight() 
{
   clearCoordinateHighlight() ;
   clearItemHighlight() ;
}

void 
MEItemLayer::clearCoordinateHighlight() 
{
   m_highlightedCoordinates.erase(m_highlightedCoordinates.begin(),
                                  m_highlightedCoordinates.end());
}

void 
MEItemLayer::clearItemHighlight() 
{
   mc2dbg8 << here << " clearItemHighlight called" << endl;
   m_highlightedItems.erase(m_highlightedItems.begin(),
                            m_highlightedItems.end());
}


bool 
MEItemLayer::drawItemWithID(uint32 id, Glib::RefPtr<Gdk::GC>& gc, 
                          bool highlightNode, bool locateItem) 
{
   multimap<uint32, MEDrawableItem*>::iterator
      p = m_allItems.lower_bound(id & 0x7fffffff);
   while ( (p != m_allItems.end()) && 
           (p != m_allItems.upper_bound(id & 0x7fffffff))) {
      p->second->draw(gc, m_window);
      mc2dbg4 << here << " Drawing item with id=" << id<< endl;
      if (highlightNode) {
         if ( (id & 0x80000000) == 0x80000000) {
            p->second->highlightLastCoord(gc, m_window);
         } else {
            p->second->highlightFirstCoord(gc, m_window);
         }
      }
      p++;
   }

   if (locateItem) {
      Vector ids(1);
      ids.addLast(id);
      locateItemsWithIDs(&ids);
   }

   return (m_allItems.count(id & 0x7fffffff) > 0);
}

void
MEItemLayer::locateItemsWithIDs(Vector* ids)
{
   int maxX = MIN_INT32;
   int minX = MAX_INT32;
   int maxY = MIN_INT32;
   int minY = MAX_INT32;

   bool updated = false;

   for (uint32 i=0; i<ids->getSize(); i++) {
      uint32 id = ids->getElementAt(i) & 0x7fffffff;
      multimap<uint32, MEDrawableItem*>::iterator p =
                           m_allItems.lower_bound(id);
      while ( (p != m_allItems.end()) && (p != m_allItems.upper_bound(id))) {
         gint tMaxX, tMinX, tMaxY, tMinY;
         p->second->getBoundingBox(tMaxX, tMinX, tMaxY, tMinY);
         if (tMaxX > maxX) maxX = tMaxX;
         if (tMinX < minX) minX = tMinX;
         if (tMaxY > maxY) maxY = tMaxY;
         if (tMinY < minY) minY = tMinY;
         updated = true;
         p++;
      }
   }

   if (updated) {
      // Caclulate where to draw the rectangle
      int deltaX = MAX(maxX-minX+25, 100);
      int deltaY = MAX(maxY-minY+25, 100);
      int x = minX + (maxX-minX)/2 - deltaX/2;
      int y = minY + (maxY-minY)/2 - deltaY/2;
      m_window->draw_rectangle(m_highlightGC, false, x, y, deltaX, deltaY);
      mc2dbg4 << here << " located, (x,y)=(" << x<< "," <<y 
              << "), deltaX=" << deltaX << ", deltaY=" << deltaY << endl;
   }
}


void
MEItemLayer::drawHighlightedItems() {
   // The graphics context that will be used when drawing the items
   Glib::RefPtr<Gdk::GC> gc;
   gc = Gdk::GC::create(m_window);   

   mc2dbg8 << here << " To highlight " << m_highlightedItems.size() 
           << " items" << endl;

   // Use the new vector with more information
   vector<highlightitem_t>::const_iterator it;
   for (it = m_highlightedItems.begin(); 
        it != m_highlightedItems.end(); it++) {
      // Update the gc
     
     //gc->set_line_width(it->lineWidth);
     gc->set_line_attributes(3, 
			     Gdk::LINE_SOLID,
			     Gdk::CAP_NOT_LAST,
			     Gdk::JOIN_BEVEL); 
     gc->set_foreground(it->color);
      drawItemWithID(it->itemID, gc, 
                     false,      // Highlight node (MSB in itemID)
                     it->locate);
   }
}



void
MEItemLayer::drawHighlightedCoordinates() {
   // The graphics context that will be used when drawing the coords.
   Glib::RefPtr<Gdk::GC> gc;
   gc = Gdk::GC::create(m_window);

   // Use the new vector with more information
   vector<highlightcoord_t>::const_iterator it;
   for (it = m_highlightedCoordinates.begin(); 
        it != m_highlightedCoordinates.end(); it++) {
      // Update the gc
      //gc->set_line_width(it->lineWidth);
     gc->set_line_attributes(it->lineWidth, 
			     Gdk::LINE_SOLID,
			     Gdk::CAP_NOT_LAST,
			     Gdk::JOIN_BEVEL); 
     gc->set_foreground(it->color);

      // Calculate screen-coordinate to draw at
      gint16 x = getGtkX(it->lon);
      gint16 y = getGtkY(it->lat);

      switch (it->symbol) {
         case squareSymbol: {
            mc2dbg4 << "   To draw squareSymbol at (" << x << "," 
                    << y << ")" << endl;
            // Compensate for that (x,y) in draw_rectangle is upper left
            x -= HIGHLIGHT_SQ_SIZE/2;
            y -= HIGHLIGHT_SQ_SIZE/2;
            m_window->draw_rectangle(gc, true, x, y,
                                    HIGHLIGHT_SQ_SIZE, HIGHLIGHT_SQ_SIZE);
            break;
         }
         case crossSymbol: {
            mc2dbg4 << "   To draw crossSymbol at (" << x << "," 
                    << y << ")" << endl;
            gint r = HIGHLIGHT_SQ_SIZE;
            m_window->draw_line(gc, x-r, y-r, x+r, y+r);
            m_window->draw_line(gc, x-r, y+r, x+r, y-r);
            break;
         }
         case diamondSymbol: {
            mc2dbg4 << "   To draw diamondSymbol at (" << x << "," 
                    << y << ")" << endl;
            int r = HIGHLIGHT_SQ_SIZE;
            int r2 = r/2;
            m_window->draw_line(gc, x-r2, y, x, y-r2);
            m_window->draw_line(gc, x, y-r2, x+r2+1, y+1);
            m_window->draw_line(gc, x+r2, y, x, y+r2);
            m_window->draw_line(gc, x, y+r2, x-r2, y);


            break;
         }
      }
   }


}

void
MEItemLayer::setFilterLevel(uint8 filterLevel)
{
   m_filterLevel = filterLevel;
}

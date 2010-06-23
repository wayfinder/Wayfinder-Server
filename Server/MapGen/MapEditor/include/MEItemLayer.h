/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEITEMLAYER_H
#define MEITEMLAYER_H

#include "config.h"
#include "OldGenericMap.h"

#include <map>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include "MEGdkColors.h"

#define HIGHLIGHT_SQ_SIZE 10

/**
 *    Describe all items of one specific layer.
 *
 */
class MEItemLayer {
   public:
      /**
       *    Create a new layer with items.
       *    @param window        The window where the items will be drawn.
       *    @param type          The type of the items in this layer.
       *    @param defaultColor  The color to use when drawing the items.
       *    @param defaultHighlightCol  The default color to use when 
       *                         highlighting the items.
       */
      MEItemLayer(Glib::RefPtr<Gdk::Window>& window, ItemTypes::itemType type,
                  MEGdkColors::color_t deafultColor, 
                  MEGdkColors::color_t defaultHighlightColor,
                  uint8 filterLevel);

      /**
       *    Delete this item layer, including all the items.
       */
      virtual ~MEItemLayer();

      /**
       *
       */
      void setParameters(bool filled, MEGdkColors::color_t col);
      
      /**
       *    Draw the items in this layer.
       */
      virtual void drawItems();

      /**
       *    Create the items for this layer from the given map, over the
       *    given area.
       *    @param map        The map where to get the items from.
       *    @param bboxLatLon The latitude and longitude that describes
       *                      the bounding box that will be shown.
       *    @param hPixels    The number of horizontal pixels that will be
       *                      shown on the screen.
       *    @param vPixels    The number of vertical pixels that will be
       *                      shown on the screen.
       */
      void createItems(const OldGenericMap* map,
                       MC2BoundingBox& bboxLatLon,
                       gint16 hPixels, gint16 vPixels);

      /**
       *    Highlight the items with given IDs.
       *    @param itemIDs    A vector with the ID for the items to highlight.
       *    @param color      The color to use when highlighting the items.
       *    @param lineWidth  The linewidth to use when drawing the 
       *                      highlighted items.
       *    
       *    @return  Currently always returns true.
       */
      bool highlightItemWithIDs(Vector& itemIDs, 
                                MEGdkColors::color_t color = MEGdkColors::red, 
                                int lineWidth = 3,
                                bool locate = false);

      /**
       *    Add one item to highlight.
       *    @param id         The ID of the item to highlight.
       *    @param color      The color to use when highlighting the item.
       *    @param lineWidth  The linewidth to use when drawing the 
       *                      highlighted item.
       */
      void addHighlightID(uint32 id, 
                          MEGdkColors::color_t color = MEGdkColors::red,
                          int lineWidth = 3, bool locate = false);

      /**
       *    Clear all highlight in the map.
       */
      virtual void clearAllHighlight();
      virtual void clearCoordinateHighlight();
      virtual void clearItemHighlight();

      /**
       *    Highlight everything that should be highlighted on the map.
       */
      virtual void drawAllHighlight();

      /**
       *    The symbols that are possible to use when highlighting
       *    coordinates.
       */
      enum symbol_t {
         squareSymbol,
         crossSymbol,
         diamondSymbol
      };

      /**
       *    Add a coordinate that should be highlighted.
       *    @param lat  The latitude part of the coordinate that should 
       *                be highlighted.
       *    @param lon  The longitude part of the coordinate that should 
       *                be highlighted.
       */
      void addHighlightCoordinate(int32 lat, int32 lon, 
                                  Gdk::Color color,
                                  int lineWidth = 2,
                                  symbol_t symbol = squareSymbol);
      
      bool isFilled() { return m_filled;};
      MEGdkColors::color_t getDrawColor() { return m_drawColor;};
      void locateItemsWithIDs(Vector* ids);
      bool m_highlightedOnly;

      void setFilterLevel(uint8 filterLevel);
   protected:

      void createDrawableFromGfxData(GfxData* gfx, OldItem* item,
                                     bool print = false );


      
      /**
       *    Class that describes one drawable on the map. Have methods 
       *    that draws the item in a window and to highlight it in
       *    different ways.
       */
      class MEDrawableItem {
         public:
            /**
             *    Create a new drawable.
             *    @param thePoints  The points that this drawable consists 
             *                      of.
             *    @param nbrCoord   The number of coordinates in 
             *                      thePoints-array.
             *    @param closed     Boolean that determine if this drawable
             *                      should be drawn as an open or closed
             *                      polygon.
             */
            MEDrawableItem(MEItemLayer* itemLayer, /*GdkPoint* thePoints, */
                         uint32 nbrCoords, OldItem* item, bool closed);
               
            /**
             *    Delete this drawable.
             */
            ~MEDrawableItem();

            void addCoordinate(gint x, gint y);

            /**
             *    Draw this drawable in one window, using a specified
             *    graphics context.
             *    @param   gc The graphics context that will be used to
             *                draw this drawable.
             *    @param   w  The window where this drawable will be 
             *                draw.
             */
            void draw(Glib::RefPtr<Gdk::GC>& gc, Glib::RefPtr<Gdk::Window>& w);

            /**
             *    Locate this item by drawing e.g. a big square or circle
             *    around it.
             *    @param   gc The graphics context that will be used to
             *                draw this drawable.
             *    @param   w  The window where this drawable will be 
             *                draw.
             *    @param   r  The radius or side, in pixels, of the circle 
             *                or square to draw (optional parameter).
             */
            void locate(Glib::RefPtr<Gdk::GC>& gc, Gdk::Window& w);

            /**
             *    Highlight one of the coordinates in this drawable.
             *    @param   gc The graphics context that will be used to
             *                highlight the node.
             *    @param   w  The window where this drawable will be 
             *                draw.
             *    @param   i  The index of the coordinate to highlight.
             *                Valid values are 0 <= i < m_nbrPoints.
             */
            void highlightCoord(Glib::RefPtr<Gdk::GC>& gc, 
				Glib::RefPtr<Gdk::Window>& w, 
				uint32 i);

            /**
             *    Highlight the first coordinate.
             *    @param   gc The graphics context that will be used to
             *                highlight the node.
             *    @param   w  The window where this drawable will be 
             *                draw.
             */
            void highlightFirstCoord(Glib::RefPtr<Gdk::GC>& gc, 
				     Glib::RefPtr<Gdk::Window>& w);

            /**
             *    Highlight the last coordinate.
             *    @param   gc The graphics context that will be used to
             *                highlight the node.
             *    @param   w  The window where this drawable will be 
             *                draw.
             */
            void highlightLastCoord(Glib::RefPtr<Gdk::GC>& gc, 
				    Glib::RefPtr<Gdk::Window>& w);

            OldItem* m_item;
            
            /**
             *    Get the bounding box of this drawable.
             *    @param maxX Outparameter that is set to the maximum X-value.
             *    @param minX Outparameter that is set to the minimum X-value.
             *    @param maxY Outparameter that is set to the maximum Y-value.
             *    @param minY Outparameter that is set to the minimum y-value.
             */
            void getBoundingBox(gint& maxX, gint& minX,
                                gint& maxY, gint& minY);

            void getBoundingBox(MC2BoundingBox& bb);

         private:
            /**
             *    An array with the points that describes this drawable.
             *    The size of this array is m_nbrPoints.
             */
            std::vector<Gdk::Point> m_points;

            /**
             *    The size of the m_points-array.
             */
            uint32 m_nbrPoints;

            /**
             *    Describes if the polygon isclosed or not.
             */
            gint m_closed;

            /**
             *    The itemlayer where this drawable is located. To make
             *    is possible to access members in that class when 
             *    drawing the item, e.g. m_filled.
             */
            MEItemLayer* m_itemLayer;

            /**
             * @return True if all coordinates are the same, i.e located
             *         on the same spot.
             */
            bool allCoordsSame();
      };

      /**
       *    Give class MEDrawableItem access to the members. It will use 
       *    e.g. m_filled when drawing.
       */
      friend class MEDrawableItem;

      /**
       *    Set to true if the closed items shoud be filled or not.
       */
      bool m_filled;
      MEGdkColors::color_t m_drawColor;

      /**
       *    The graphics context that will be used for the items if 
       *    nothing else is done in the subclasses.
       */
      Glib::RefPtr<Gdk::GC> m_mainGC;

      /**
       *    The graphics context that will be used for the highlighted
       *    drawables if nothing else is done in the subclasses.
       */
      Glib::RefPtr<Gdk::GC> m_highlightGC;
      
      /**
       *    The window where the drawables will be drawn.
       */
      Glib::RefPtr<Gdk::Window> m_window;

      /**
       *    The type of the items in this item-layer.
       */
      ItemTypes::itemType m_itemType;

      /**
       *    All the drawables in this layer that should be displayed at
       *    the current zoom-level. Have the ID of the items as key, and
       *    a pointer to a MEDrawableItem as value.
       */
      multimap<uint32, MEDrawableItem*> m_allItems;

      /**
       *    @name Highlighted items
       *    Types, members and methods used when drawing the highlighted 
       *    items on the map.
       */
      //@{
         /**
          *    Struct that describes one item that should be highlighted. 
          *    Contains information about the id of the item, the color and
          *    the line width that should be used when drawing the item on 
          *    the map.
          */
         struct highlightitem_t {
            /// The id of the item to highlight
            uint32 itemID;

            /// The width of the line that will draw this item
            int lineWidth;

            /// The color that will be used when drawing item
            Gdk::Color color;

            /** 
             *    True if the item will be located (a big square around it), 
             *    false otherwise.
             */
            bool locate;
         };

         /**
          *    A vector with the items to highlight.
          */
         vector<highlightitem_t> m_highlightedItems;

         /**
          *    Highlight the items on the map that should be highlighted.
          */
         void drawHighlightedItems();
      //@}

      /**
       *    @name Highlighted coordinates
       *    Types, members and methods used when highlighting selected
       *    coordinates on the map.
       */
      //@{
         /**
          *    Struct that describes one coordinate that should be 
          *    highlighted. Contains information about the coordinate,
          *    the symbol and the color that should be used when drawing
          *    the coordinate on the map.
          */
         struct highlightcoord_t{
            int32 lat;
            int32 lon;
            symbol_t symbol;
            int lineWidth;
            Gdk::Color color;
         };

         /**
          *    A vector with the coordinates to highlight.
          */
         vector<highlightcoord_t> m_highlightedCoordinates;

         /**
          *    Highlight the coordinates on the map that should be 
          *    highlighted.
          */
         void drawHighlightedCoordinates();
      //@}

      /**
       *    Draw one item with given ID on one given GC. Is also useable
       *    when highlighting one of the node if the item is a street
       *    segment item. In this case, the id will be used to get the
       *    node to highlight (the MSB is set if node 1, unset if node 
       *    0).
       *    @param id            The ID of the item to draw on the screen 
       *                         (in window m_window).
       *    @param gc            The graphics context that should be used
       *                         when drawing the item.
       *    @param highlightNode True if to highlight the node described
       *                         by the MSB in id, false otherwise.
       *    @param locateItem    True if the item should be located (draw
       *                         a big square around it), false otherwise.
       */
      bool drawItemWithID(uint32 id, Glib::RefPtr<Gdk::GC>& gc, 
			  bool highlightNode = false,
                          bool locate = false);
      
      /**
       *    @name mc2-coordinates -> screen coordinates.
       *    Members and methods that are used to convert from true 
       *    coordinates (lat,lon) to screen-coordinates.
       */
      //@{
         /// The factor that is multiplied with the latitude
         float64 m_latScaleFactor;

         /// The factor that is multiplied with the longitude
         float64 m_lonScaleFactor;

         /// The factor that is substracted from the latitude
         float64 m_latSubFactor;

         /// The factor that is substracted from the longitude
         float64 m_lonSubFactor;

         // The filter level
         uint8 m_filterLevel;

         /**
          *    Get the x-value from a given longitude on the current map.
          *    @param lon  The longitude to calculate the x-coordinate for.
          *    @return  The x-coordinate for the given longitude.
          */
         inline gint16 getGtkX(int32 lon);

         /**
          *    Get the y-value from a given latitude on the current map.
          *    @param lat  The latitude to calculate the x-coordinate for.
          *    @return  The y-coordinate for the given latitude.
          */
         inline gint16 getGtkY(int32 lat);
      //@}
};

// ======================================================================
//                                    Implementation of inlined methods =

inline gint16 
MEItemLayer::getGtkX(int32 lon) {
   return int16( rint((lon - m_lonSubFactor) * m_lonScaleFactor));
}

inline gint16 
MEItemLayer::getGtkY(int32 lat) {
   return int16( rint((m_latSubFactor - lat) * m_latScaleFactor));
}



#endif



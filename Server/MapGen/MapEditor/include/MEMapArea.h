/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEMAPAREA_H
#define MEMAPAREA_H

#include "config.h"
#include "MEItemLayer.h"
#include "MEStreetSegmentItemLayer.h"
#include "OldGenericMap.h"
#include "MEItemInfoDialog.h"
#include "MC2BoundingBox.h"

#include <gtkmm/main.h>
#include <gtkmm/widget.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>

#include "GDFID.h"


class OldNode;
class GMSSignPost;
class GDFRef;

/**
 *    Objects of this class describes the DrawingArea where the map is drawn.
 *    The objects will also handle mouse-click etc. in the map.
 *
 */
class MEMapArea : public Gtk::DrawingArea {
   public:
      /**
       *    Create a new MEMapArea for a given map.
       *    @param map           The map that contains all information
       *                         (items etc.).
       *    @param filterLevel   The filter level to use when drawing the map.
       *                         Default is 0 (draw all coordinates).
       */
      MEMapArea(OldGenericMap* map, uint8 filterLevel = 0);

      /// Find out if an item (id) is present among the shown layers
      bool itemAmongShownLayers(uint32 id);

      /**
       *    Highlight one item on the map. If the clearFirst-parameter is
       *    set to false this id will be added to the possible other
       *    items to highlight.
       *    @param id         The ID of the item to highlight.
       *    @param clearFirst Before the given item is added, all other
       *                      highlights are removed.
       *    @return True if the given id was found in the map among the 
       *            existing layers and added to. This does not mean 
       *            that the highlighted item is displayed with the 
       *            current zoom.
       */
      bool highlightItem(uint32 id, bool clearFirst = true, 
                         bool locate = true, bool redrawScreen = true, 
                         MEGdkColors::color_t color = 
                         MEGdkColors::invalidColor);
      /**
       *    Highlight one connection in the map.
       *    @param toNodeID   The ID of the node that it is possible to
       *                      drive to.
       *    @param fromNodeID The ID of the node that it is possible to
       *                      drive from.
       *    @return  True if the connection is highlighted, false otherwise.
       */
      bool highlightConnection(uint32 toNodeID, uint32 fromNodeID);

      /**
       *    Highlight one coordinate on the map, using the given values
       *    when highlighting.
       *    @param lat        The latitude to highlight.
       *    @param lon        The longitude to highlight.
       *    @param color      The color to draw with.
       *    @param symbol     The symbol to draw at the given coordinate.
       *    @param lineWidth  The linewidth to use when drawing the symbol
       *                      (used when appropriate for the selected 
       *                      symbol).
       *    @param redraw     True if the map should be redraw, false 
       *                      otherwise.
       *    @return True if the coordinate is highlighted, false otherwise
       *            (e.g. the coordinate outside the current map).
       */
      bool highlightCoordinate(int32 lat, int32 lon, Gdk::Color color,
                               MEItemLayer::symbol_t symbol =
                                 MEItemLayer::squareSymbol,
                               int lineWidth = 2, 
                               bool redrawScreen = true);
      /**
       *    Highlight all the coordinates in one GfxData.
       *
       *    @param gfxData The GfxData that will be highlighted.
       *    @return True if the coordinates are highlighted, false 
       *            otherwise.
       */
      bool highlightCoordinates(GfxData* gfxData);

      void clearCoordinateHighlight(bool redrawScreen); 

      
      /**
       *    Clear all highlighted items and coordinates.
       */
      void clearAllHighlight(); 
      
      /**
       *    Get the layer with the street segments.
       *    @return  A pointer to the street segment item layer. Will 
       *             return NULL if this layer not is active or upon other
       *             errors.
       */
      MEStreetSegmentItemLayer* getStreetSegmentItemLayer();

      /** 
       *    Redraw all layers on the map.
       */
      void redraw();

      void redraw_added_highlight();
      
      
      /**
       *    Recalculate the polygons that are shown on the map and redraw 
       *    the map.
       */
      void recalculateAndRedraw();

      /**
       *    Get the costs of one connection in the map.
       *    @param conn    The connection to get the costs for.
       *    @param toNode  The node where the connection is stored.
       *    @param costA   Outparameter that is set to costA of the 
       *                   connection.
       *    @param costB   Outparameter that is set to costB of the 
       *                   connection.
       *    @param costC   Outparameter that is set to costC of the 
       *                   connection.
       *    @param costD   Outparameter that is set to costD of the 
       *                   connection.
       *    @param externalConnection True if the connection is an external
       *                   connection (to/from an other map), false 
       *                   otherwise.
       */
      bool getConnectionCosts(OldConnection* conn, OldNode* toNode,
                              uint32& costA, uint32& costB,
                              uint32& costC, uint32& costD,
                              bool externalConnection = false);
      /**
       *    @name Information about the shown map.
       *    Methods to get the map or information about the map.
       */
      //@{
         /**
          *    Get a pointer to the shown map.
          *    @return A pointer to the map.
          */
         inline OldGenericMap* getMap(); 

         /**
          *    Get the country name of the shown map. Used for writing
          *    country in the comment field of extra data map correction
          *    records.
          *    @return  The country name of the map in english. All
          *             space-chars are replaced with "_".
          */
         const char* getMapCountryName();

         /// Get the map id of the shown map.
         inline uint32 getMapId();
      //@}

      /**
       *    Get the bounding box of the currently shown area.
       *    @param bbox The boudningbox that will be set to the currently 
       *                shown area.
       */
      inline void setBoundingBox(MC2BoundingBox& bbox);

      /**
       *    @name Handle the shown layers.
       *    Methods to add, remove and change parameters of the shown 
       *    item-layers.
       */
      //@{
         /**
          *    Add one item-layer that will be shown. Nothing will happen
          *    if this layer already is shown.
          *    @param t The type of the layer that will be shown.
          */
         void addItemLayer(ItemTypes::itemType t,
                           bool recalc,
                           MEGdkColors::color_t defaultColor 
                               = MEGdkColors::black,
                           MEGdkColors::color_t defaultHighlightColor
                               = MEGdkColors::red,
                           bool filled = false);

         /**
          *    Remove one of the item-layers. Nothing will happen
          *    if this layer not already is shown.
          *    @param t The type of the layer that will be removed.
          */
         void removeItemLayer(ItemTypes::itemType t, bool recalc);

         /**
          *    Set some parameters that will be used when drawing all 
          *    items in the selected layer.
          *    @param t       The type of the items that will be affected
          *                   of the parameters.
          *    @param filled  True if all closed items will be filled or 
          *                   not.
          */
         
         void setItemLayerParameters(ItemTypes::itemType t, bool filled,
                                     MEGdkColors::color_t col);
        
         bool isLayerActive(ItemTypes::itemType t);
         bool isLayerFilled(ItemTypes::itemType t);
         MEGdkColors::color_t getDrawColor(ItemTypes::itemType t);
      //@}
      
      void zoomToBBox(int32 northLat, int32 westLon, 
                      int32 southLat, int32 eastLon);

      inline void setFilterLevel(uint8 filterLevel);
      
      inline uint8 getFilterLevel() const;
      
      void setGdfRef(GDFRef* gdfRef);
     
      GDFID getGdfID(const OldItem* item); 
      bool loadedGdfRef();
      /**
       * @return Id of matched mcm ID if only one found, otherwise 
       *         MAX_UINT32
       */
      uint32 highlightFromGdfID(uint32 gdfFeatID); 
      
      /** Set the background color of the main window.
       */
      void setBgColor(MEGdkColors::color_t color); 

      /// Set the valud of the m_keepHighlightsOnSelect
      void setKeepHighlightsOnSelect( bool keep ); 

      /// Get all lanes of a node.
      const vector<GMSLane> getLanes(const OldNode& node) const; 

      /// Get all connecting lanes of a connection.
      uint32 getConnectingLanes(const OldNode& toNode, 
                                const OldConnection& connection) const;

      /// Get the sing post of a connection.
      const vector<GMSSignPost*>& getSignPosts(const OldNode& toNode, 
                                               const OldConnection& 
                                               connection) const;
      /// Debug print of sing post info.
      void printSignPosts(const OldNode& toNode, 
                          const OldConnection& 
                          connection) const;

    protected:
      /**
       *    The implementation of the realize-method. This is called when the
       *    window is created.
       */
      virtual void on_realize(); 

      /**
       *     Restores default drawing settings.
       */
      void setDefaultDrawing();

      /**
       *    This method draws the map in the window. Called when the window
       *    needs to be redrawn.
       *    @param  e The event with expose information (not used).
       *    @return ???
       */
      virtual bool on_expose_event(GdkEventExpose* e);

      /**
       *    This method handels mouse-clicks.
       *    @param  e   The event with information about the mouse-click 
       *                (screen coordinates, time etc.).
       *    @return ???
       */
      virtual bool on_button_press_event (GdkEventButton* e);

      /** 
       *    This method is called when the window is resized.
       *    @param   e Event with information about the changes.
       *    @return ???
       */
      virtual bool on_configure_event (GdkEventConfigure* e);

   private:
      /**
       *    Recalculate all the polygons that should be shown on the map 
       *    (for all item layers).
       */
      void recalculate();
      
      /**
       *    Update the maximum and minimum values of the bounding box. Used
       *    when to calculate new drawable items in the item layers.
       *    @param maxL        In/out-parameter that is set to the maximum, 
       *                       valid value of the latitude coordinate.
       *    @param minL        In/out-parameter that is set to the minimum
       *                       valid value of the latitude coordinate.
       *    @param maxAllowedL The maximum allowed latitude.
       *    @param minAllowedL The minimum allowed latitude.
       */
      void verifyMaxMin(int32& maxL, int32& minL, int32 delta,
                        int32 maxAllowedL, int32 minAllowedL);
      /** 
       *    The layers with items on the map. Each layer handles one 
       *    ItemType.
       */
      MEItemLayer* m_layer[uint32(ItemTypes::numberOfItemTypes)];

      /** 
       *    The order in which to draw the layers
       */
      static uint32 m_layerDrawingOrder[uint32(ItemTypes::numberOfItemTypes)]; 

      /** 
       *    The window where to draw.
       */
      Glib::RefPtr<Gdk::Window> m_window;

      /**
       *    The map with all the items and other information.
       */
      OldGenericMap* m_map;

      void adjustBBox(bool keepCenter = true);

      /**
       *    The bounding box of what is currently shown.
       */
      MC2BoundingBox m_bboxLatLon;

      /**
       *    A simple semaphore that is used to make sure that the window not
       *    is redrawn until the item-layers are created.
       */
      bool m_initialized;

      /**
       *    The filter level used when displaying the map.
       */  
      uint8 m_filterLevel;

      /// Used for finding GDF IDs corresponding to the items of the map.
      GDFRef* m_gdfRef;

      Gdk::Color m_bgColor;

      /// Whether to keep highlights when selecting next item or not.
      bool m_keepHighlightsOnSelect;
};


inline OldGenericMap* 
MEMapArea::getMap() 
{
   return (m_map);
}

inline void 
MEMapArea::setBoundingBox(MC2BoundingBox& bbox) 
{
   bbox.setMaxLat(m_bboxLatLon.getMaxLat());
   bbox.setMinLat(m_bboxLatLon.getMinLat());
   bbox.setMaxLon(m_bboxLatLon.getMaxLon());
   bbox.setMinLon(m_bboxLatLon.getMinLon());
}

inline uint32
MEMapArea::getMapId()
{
   if (m_map != NULL)
      return m_map->getMapID();
   return MAX_UINT32;
}

inline void
MEMapArea::setFilterLevel(uint8 filterLevel)
{
   m_filterLevel = filterLevel;
}

inline uint8
MEMapArea::getFilterLevel() const
{
   return m_filterLevel;
}

#endif


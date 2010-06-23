/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEMAPEDITORWINDOW_H
#define MEMAPEDITORWINDOW_H

#include "config.h"
#include "OldGenericMap.h"
#include "StringUtility.h"
#include "MEGdkColors.h"
#include "MEDrawSettingsDialog.h"
#include <gtkmm/base.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/combo.h>
#include <gtkmm/entry.h>
#include <gtkmm/separator.h>

class GDFRef;

/**
 *    The main window in the MapEditor. Has a map-area and the main manu.
 * 
 */
class MEMapEditorWindow : public Gtk::Window {
   public:
      /**
       *    Create a new MEMapEditorWindow for a given map. This window
       *    contains a map-area (that is created in this constructor)
       *    to the left and a menu to the right.
       *    @param map  The map that will be shown in the MEMapArea.
       *    @param gdfRefFile Explicit directory of gdf ref file to load. 
       *           Otherwise first found gdf ref is loaded.
       */
      MEMapEditorWindow(OldGenericMap* map, MC2String gdfRefDir);

      /**
       *    Deconstructor
       */
      ~MEMapEditorWindow();

      /**
       *    Method to handle when entrys go out of focus, to
       *    make sure that the text remains selected.
       */
      bool focus_out_event(GdkEventFocus*);

      /**
       *    Method that is called when to change the draw settings.
       */
      void drawSettingsPressed();

      /**
       *    Method that is called when to show nodes from a text-file.
       *    The text file was created e.g. with MapTool --compturns option
       *    to report turn descriptions that differ when comparing two 
       *    otherwise identical mcm maps.
       */
      void showNodesPressed();

      /**
       *    Method called when showing extra data files using the
       *    ShowExtradataWindow. Useful when checking map error
       *    corrections for a new map release.
       */
      void showExtradataPressed();

      /**
       *    Method that is called when searching for an item in the
       *    map with given itemID or waspID or ...
       */
      void searchIDPressed();

      /**
       *    Method that is called when the ID paste button or 
       *    paste lat or paste lon is pressed.
       */
      void pastePressed( Gtk::Entry* entry );

      /**
       *    Method that is called when the text from clipboard is
       *    received
       */
      void on_clipboardTextReceived( const Glib::ustring& text, 
                                     Gtk::Entry* entry);

      
      /**
       *    Method that is called when searching for an item with given 
       *    itemID. Called from searchIDPressed.
       */
      void searchItemIDPressed();

      /**
       *    Method that is called when searching for a poi with given 
       *    GDF ID. Called from searchIDPressed.
       */
      void searchPoiWaspIDPressed(); 

      /**
       *    Method that is called when searching for an item with given 
       *    WASP ID. Called from searchIDPressed.
       */
      void searchGdfIDIDPressed();


      /**
       *    Method that is called when searching for an item with given 
       *    name. Called from searchIDPressed.
       */
      void searchItemNamePressed();

      /**
       *    Method that is called when pressing zoom-to for itemID, 
       *    waspID, itemName, ... 
       */
      void zoomToIDPressed();

      /**
       *    Method that is called when zooming to a searched item.
       *    Called from zoomToIDPressed.
       */
      void zoomToItemPressed();

      /**
       *    Method that is called when zooming to a searched item.
       *    Called from zoomToIDPressed.
       */
      void zoomToPoiPressed();

      /**
       *    Method that is called when zooming to a searched item.
       *    Called from zoomToIDPressed.
       */
      void zoomToGdfIdPressed(); 

      /**
       *    Zoom to item with item ID itemID.
       */
      void zoomToItem(uint32 itemID);
      /**
       *    Method that is called when searching for coordinates.
       */
      void searchCoordinatePressed();

      /**
       *    Method that is called when zooming to a searched coordinate.
       */
      void zoomToCoordinatePressed();

      /**
       *    Clear all highlight on the map.
       */
      void clearHighlightPressed();

      /**
       *    Method which is called when the new filter level is used.
       */
      void filterLevelPressed();

      /**
       *    Highlights the items in the vector. Also saves the items
       *    in a private m_loadedItemIds vector for later access.
       *
       *    @param items Vector with item IDs of items to highlight.
       */
      void highlightItems( vector< pair<uint32, MEGdkColors::color_t> >& 
                           items );
      
      /**
       *    Highlights the mc2 coordinates in the vector. Also saves 
       *    the coords in a private m_loadedCoords vector for later access.
       *
       *    @param items Vector with mc2 coordinates to highlight.
       */
      void highlightCoords( vector<MC2Coordinate>& coords );

      /**
       * Handles draw settings appropriate for showing POIs with colors.
       */
  
      void initiateShowPOIs(); 

      #ifdef MAP_EDITABLE 
         /**
          *    Get the name of the logfile.
          *    @return A pointer to the name of the logfile that is used
          *            to store map changes.
          */
         const char* getLogFileName();
         
         /**
          *    Method that is called when the save-button is pressed.
          */
         void savePressed();
      #endif 

      void setGdfRef(GDFRef* gdfRef);

   protected:
   
      /**
       *    Handles key presses, short commands for misc actions
       *    @param p0 The key event
       */
      virtual bool on_key_press_event(GdkEventKey* p0);


   private:
      Gtk::Box* m_rightMenu;
      Gtk::ToggleButton* m_hideBtn;
      void on_hideBtnClicked();
       
      /**
       *    The map area with the map that is currently displayed (m_map).
       */
      MEMapArea* m_mapArea;

      /**
       *    @name Various entries.
       *    Entry boxes for different searches and filenames.
       */
      //@{
         /**
          *   Entry where to write the id type to search for
          */
         Gtk::Entry* m_searchIDType; 

         /**
          *    Combo-box with the possible values of id types to search for 
          *    (itemid, waspid, etc).
          */
         Gtk::Combo* m_searchIDTypeCombo; 

         /**
          *    Entry where to write the ID of the item to search for.
          *    POI WASP id or item id.
          */
         Gtk::Entry* m_searchIDEntry;

         /**
          *    Entry where to write the latitude of the coordinate to
          *    show on the map.
          */
         Gtk::Entry* m_searchLatEntry;

         /**
          *    Entry where to write the longitude of the coordinate to
          *    show on the map.
          */
         Gtk::Entry* m_searchLonEntry;

         /**
          *    Entry where to write the filter level to display
          */
         Gtk::Entry* m_filterLevelIn;
         
      //@}

      /**
       *    Font selection
       */
      //Gtk::FontSelection* m_fontSelection; // Fixme: not yet in use
      
      /**
       *    The map that is currently shown.
       */
      OldGenericMap* m_map;

      /**
       *    The filter level currently shown in the map.
       */
      uint8 m_filterLevel;

      /**
       *    A vector with items ids that were loaded for higlight.
       */
      vector<pair< uint32, MEGdkColors::color_t > > m_loadedItemIds;

      /**
       *    A vector with coordinates that were loaded for higlight.
       */
      vector<MC2Coordinate> m_loadedCoords; 

      /**
       *    Method that highlights all items in the m_loadedItemIds vector.
       */
      void highlightLoadedItemIDs();
      
      /**
       *    Method that highlights all coords in the m_loadedCoords vector.
       */
      void highlightLoadedCoords();

      /**
       *    Highlights all data initially loaded from file.
       */
      void highlightLoadedData();

      #ifdef MAP_EDITABLE 
         /**
          *    Entry for the name of the logfile where all map-changes
          *    (extra data records) are saved.
          */
         Gtk::Entry* m_logFilenameEntry;
      #endif

      /**
       *    Method that takes the coordinate strings and converts them 
       *    to mc2 coordinates. Used for searchCoordinatePressed and 
       *    zoomToCoordinatePressed, so they can take both mc2 and
       *    wgs84 degrees coordinates.
       *    @return  True if the mc2 coordinate out params are set,
       *             false if not.
       */
      bool getMc2Coordinates( char* latString, char* lonString, 
                              int32 &mc2lat, int32 &mc2lon );  
      
      /// Button for deciding whether to keep highlights on selecting or not
      Gtk::ToggleButton* m_keepHighlightsOnSelectBtn;
      
      /**
       *    If the keep-highlights-on-select button is pressed, update
       *    the bool variable in MapArea
       */
      void on_keepHighlightsOnSelectBtnClicked();
};

/**
 *    A global pointer to the MEMapEditorWindow.
 */
extern MEMapEditorWindow* g_mapEditorWindow; 

/**
 *    A global variable. If it is true, strings in the loaded map
 *    are coded as UTF-8. Otherwise the strings are coded 
 *    as 8 bit ASCII iso-8859-1.
 */
extern bool g_utf8Strings;

#endif


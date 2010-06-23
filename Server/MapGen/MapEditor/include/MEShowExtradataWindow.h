/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESHOWEXTRADATA_H
#define MESHOWEXTRADATA_H

#include "config.h"
#include <gtkmm.h>
#include <gtkmm/window.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/frame.h>
#include <vector>
#include <list>
#include "MEMapArea.h"
#include "OldExtraDataUtility.h"
#include <map>
#include <vector>
#include "MC2String.h"


/**
 *    A window for displaying the coordinates of extra data records
 *    in the MapEditor map. Useful when checking and updating 
 *    extra data for new map releases.
 *    
 *    An extra data file is loaded and all records that fits the map
 *    are displayed in a list. When selecting one ed record in the list,
 *    the coordinates of that record are displyed in a second list.
 *    The coordinates can be higlighted, all of them or one-by-one to 
 *    show the location of the ed record in the map.
 *
 */
class MEShowExtradataWindow : public Gtk::Window {
   public:
      /**
       *    Get the one and only instance of this class.
       *    @return  The one and only MEShowExtradataWindow.
       */
      static MEShowExtradataWindow* instance(MEMapArea* mapArea);

      /**
       *    Free the memory of the one instance of this class.
       */
      static void deleteInstance();

      /**
       *    Delete this window and everything that is allocated here.
       */
      virtual ~MEShowExtradataWindow();


   private:
      /**
       *    Create a window that should be used for loading and 
       *    inspecting an extra data file. Declared private to 
       *    make sure we only have one instance of this class.
       */
      MEShowExtradataWindow(MEMapArea* mapArea);

      /**
       *    The map-area where to draw the items.
       */
      MEMapArea* m_mapArea;

      /**
       *    The one and only instance of this class.
       */
      static MEShowExtradataWindow* m_redWindow;
      //static MEShowExtradataWindow* _staticInstance;

     
      /// File selector that is used for selecting extra data file to show
      Gtk::FileSelection* m_fileSelector;
      /// Called when the OK button of the file selector is pressed.
      void on_fileSelOK();
      /// Called when the Cancel button of the file selector is pressed.
      void on_fileSelCancel();

      /// Entry box for extra data file name.
      Gtk::Entry* m_fileNameEntry;
      /// Called when browsing for extra data file, opens the file selector.
      void on_browseForEDFile();
      /**
       *    Called when a extra data file has been selected. Will fill the
       *    extra data list with records from the file that fits the current
       *    map.
       */
      void on_loadEDFile();
    
      /**
       *    Map holding all extra data records that fits the
       *    currently viewed map. The key is extra data record id,
       *    the content is the record represented as a vector of 
       *    MC2Strings (the ED recordAsStrings).
       */
      std::map<uint32, vector<MC2String> > m_edRecords;
      
      /// Method called when one extra data record is selected from the list.
      void ed_selected();
     
      /// List with all coordinates from one selcted extra data record.
      Gtk::TreeView* m_edCoordsList;
      /// Check that one coord is selected and highlight it.
      void highlightOneCoordPressed();
      /// Highlight all coordinates of the selected extra data record.
      void highlightAllCoordsPressed();
      /// Clear all coordinates highlight.
      void clearHighlightPressed();
      /// Zoom to highlighted coordinates.
      void zoomPressed();

      /// Entry box for ed record id string (to make it possible to copy)
      Gtk::Entry* m_idVal;
      Gtk::Entry* m_latVal;
      Gtk::Entry* m_lonVal;
      Gtk::Entry* m_refVal;
      
      /// Clear this window from all data.
      void clearPressed();
      /// Clear this window from all data and hide.
      void closePressed();

      /**
       *    Get the value which is to be set from an extra data record.
       *    For e.g. an updateName-record the newName is returned, and
       *    for a setVehicleRestr-record the new vr (eg 0x80) is returned.
       *    An empty string ("") is returned for those records that are
       *    not checked between map-releases.
       *    @param   recType  The type of extra data record.
       *    @param   edRecord The record.
       */
      string getValueFromRecord(vector<MC2String> edRecord,
            OldExtraDataUtility::record_t recType = 
                     OldExtraDataUtility::NUMBER_OF_RECORD_TYPES);

      /**
       *    Get number of coordinates in an extra data record.
       *    Also returns in which position of the extra data record 
       *    the first coordinate is located. 
       *    E.g. an setEntryRectr-record has 2 coordinates, the first 
       *    is located in position 1 (counted as 0,1,2,...).
       *    @param   edRecord       The extra data record to check.
       *    @param   firstCoordPos  Position in the record of the 
       *                            first coordinate (out param).
       *    @return  The number of coordinates in the extra data record.
       */
      uint32 getCoordsForRecord(vector<MC2String> edRecord,
                                uint32 &firstCoordPos);

      /**
       *    Find out if a record fits the currently displayed map and 
       *    will be included in the extra data list. Used for all
       *    maps (unverview, overview, country overview etc).
       *    The method takes the first coordinate in the record, and 
       *    searches for close items (ssi, ferry or island, for overviews 
       *    also municipals). If any item is within 500 meters, the record 
       *    is considered to fit the map.
       *    @param   edRecord    The record to check.
       *    @return  True if the record fits the map, false otherwise.
       */
      bool recordFitsThisMap(vector<MC2String> edRecord);

      /**
       *    Get an MC2String with wgs84 coordinates from an MC2String with 
       *    m_mc2 coordinates.
       *    Extracts m_mc2 coordinates from a string, converts the 
       *    coordinates to wgs84 (expressed in decimal degrees) and 
       *    prints them to a string.
       *    @param   m_mc2CoordString String with m_mc2 coordinates according
       *                            to the format in extra data records.
       *    @return  An MC2String with wgs84 coordinates. Empty string if
       *             the conversion was not successful.
       */
      MC2String getWGS84stringFromMC2CoordString(MC2String m_mc2CoordString);

      /*
       * Various TreeViewer structures for extradata
       */

      /// TreeViewer to visualize the extradata
      Gtk::TreeView m_treeViewED;

      /// Structure for handling m_treeViewED selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_selectionED;

      /// Underlying data structure that contains the extradata
      Glib::RefPtr<Gtk::ListStore> m_listStoreED;

      /// Internal class for extradata columns:
      class ColumnsED : public Gtk::TreeModel::ColumnRecord
      {
      public:

         ColumnsED()
         { add(m_recId); add(m_recType); 
           add(m_value); add(m_orig_value);
           add(m_TArefId);}

         Gtk::TreeModelColumn<Glib::ustring> m_recId;
         Gtk::TreeModelColumn<Glib::ustring> m_recType;
         Gtk::TreeModelColumn<Glib::ustring> m_value;
         Gtk::TreeModelColumn<Glib::ustring> m_orig_value;
         Gtk::TreeModelColumn<Glib::ustring> m_TArefId;
      };

      /// Extradata columns
      ColumnsED m_columnsED;

      /*
       * Various TreeViewer structures for extradata coordinates
       */

      /// TreeViewer to visualize the extradata
      Gtk::TreeView m_treeViewEDCoords;

      /// Structure for handling m_treeViewEDCoords selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_selectionEDCoords;

      /// Underlying data structure that contains the extradata
      Glib::RefPtr<Gtk::ListStore> m_listStoreEDCoords;

      /// Internal class for extradata columns:
      class ColumnsEDCoords : public Gtk::TreeModel::ColumnRecord
      {
      public:

         ColumnsEDCoords()
         { add(m_mc2); add(m_wgs84);}

         Gtk::TreeModelColumn<MC2String> m_mc2;
         Gtk::TreeModelColumn<Glib::ustring> m_wgs84;
      };

      /// Extradata columns
      ColumnsEDCoords m_columnsEDCoords;
};

#endif


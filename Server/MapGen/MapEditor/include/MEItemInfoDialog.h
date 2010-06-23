/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEITEMINFODIALOG_H
#define MEITEMINFODIALOG_H

#include "config.h"
#include "OldGenericMap.h"
#include "OldItem.h"
#include "ItemTypes.h"
#include "StringTable.h"
#include "MEMapArea.h"
#include "MEItemInfoWidget.h"
#include "MERouteableItemInfoWidget.h"
#include "MEGroupItemInfoWidget.h"
#include "MEStreetSegmentItemInfoWidget.h"
#include "MEFerryItemInfoWidget.h"
#include "MEWaterItemInfoWidget.h"
#include "MEParkItemInfoWidget.h"
#include "MEPoiItemInfoWidget.h"
#include "MEBuildingItemInfoWidget.h"
#include "MECartographicItemInfoWidget.h"
#include "MEIndividualBuildingItemInfoWidget.h"
#include <gtkmm/dialog.h>
#include <gtkmm/togglebutton.h>
#include "GfxData.h"

#include <iostream>



// Forward declaration
class MEMapArea;

#ifdef MAP_EDITABLE
   #include "MELogCommentInterfaceBox.h"
#endif


#define XALIGN (-0.5)
#define YALIGN (0.0)

#define FIXED_OPT (GTK_FILL)
#define EXPAND_OPT ( GTK_FILL | GTK_EXPAND )


/**
 *
 *
 */
class MEItemInfoDialog : public Gtk::Window {
   public:
      
      /**
       *    Create a ItemInfo dialog. Done via this static method to make
       *    sure that only one ItemInfo dialog is created.
       *    @param mapArea The MEMapArea where the street segment is 
       *                   displayed.
       *    @return An info dialog.
       */
      static MEItemInfoDialog* instance(MEMapArea* mapArea);
  
      /**
       *    Free the memory of the one instance of this class.
       */
      static void deleteInstance();
 
      /**
       *    Set the item to show in the item-dialog.
       *    @param item The item to show in the item dialog.
       *    @param map  The map wher the item is located.  
       */
      void setItemToShow(OldItem* item, OldGenericMap* map);

      /**
       *    The language that are used when displaying information about
       *    the item. This might be moved to a more global place...
       */
      static const StringTable::languageCode m_language = 
         StringTable::ENGLISH;

   protected:
      /**
       *    Constructor private to make sure not instanciated from 
       *    outside.
       */
      MEItemInfoDialog(MEMapArea* mapArea);

      // don't close the window, hide it
      virtual gint delete_event_impl(GdkEventAny* p0);

      static MEItemInfoDialog* _instance;

      /**
       *    Get the map-area where the map is drawn. To be able to highlight
       *    the selected item and connection.
       */
      MEMapArea* getMapArea();
      
      OldItem* m_item;
      OldGenericMap* m_map;

      /**
       *    The canvas where the map is drawn. All map-drawing are handled
       *    by this object.
       */
      MEMapArea* m_mapArea;

      /**
       *    The button that is called when the shows-coordinates button 
       *    is clicked.
       */
      void showCoordinatesClicked();

      /// Print item to a midmif file, fileName is "item.mid" and "item.mif"
      void midmifPressed();

      void cancelPressed() {
         hide();
      }

      void okPressed();

      void deletePressed();

      #ifdef MAP_EDITABLE
         /**
          *    A box with labels and edit-boxes where to write source and
          *    comment.
          */
         MELogCommentInterfaceBox* m_logCommentBox;
      #endif

   private:
      // Should probably be stored in a vector...
      MEItemInfoWidget* m_itemInfo;
      MERouteableItemInfoWidget* m_routeableItemInfo;
      MEGroupItemInfoWidget* m_groupItemInfo;
      MEStreetSegmentItemInfoWidget* m_streetSegmentItemInfo;
      MEWaterItemInfoWidget* m_waterItemInfo;
      MEParkItemInfoWidget* m_parkItemInfo;
      MEPoiItemInfoWidget* m_poiItemInfo;
      MEBuildingItemInfoWidget* m_buildingItemInfo;
      MEFerryItemInfoWidget* m_ferryItemInfo;
      MECartographicItemInfoWidget* m_cartographicItemInfo;
      MEIndividualBuildingItemInfoWidget* m_individualBuildingItemInfo;

      Gtk::ToggleButton* m_coordinateButton;
};

#endif


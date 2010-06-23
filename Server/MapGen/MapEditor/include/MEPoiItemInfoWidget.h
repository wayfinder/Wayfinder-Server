/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEPOIITEMINFOWIDGET_H
#define MEPOIITEMINFOWIDGET_H

#include "config.h"
#include "MEAbstractItemInfoWidget.h"
#include "OldPointOfInterestItem.h"
#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm.h>


/**
 *    Widget that shows point of interest spcific information.
 *
 */
class MEPoiItemInfoWidget : public MEAbstractItemInfoWidget {
   public:

      /**
       *    Create the poi widget.
       */
      MEPoiItemInfoWidget();

      /**
       *    Delete the poi widget.
       */
      virtual ~MEPoiItemInfoWidget();

      /**
       *    Activate the poi widget, fill it with information about
       *    a selected poi.
       *    @param mapArea The mapArea where the poi is located.
       *    @param poi     The poi for which to extract info.
       */
      void activate(MEMapArea* mapArea, OldPointOfInterestItem* poi);
      
#ifdef MAP_EDITABLE
      virtual void saveChanges(MELogCommentInterfaceBox* logCommentBox);
#endif // MAP_EDITABLE

   private:
      /// The wasp id of this poi
      Gtk::Entry* m_waspIdVal;
      
      /// The poi type of this poi
      Gtk::Entry* m_poiTypeVal;
      
      /// The item id of the street segment to which this poi is connected
      Gtk::Entry* m_ssiIdVal;
      
      /// This poi's offset on the street segment to which it is connected
      Gtk::Entry* m_ssiOffsetVal;

      /*
       * Various TreeViewer structures for poi categories
       */

      /// TreeViewer to visualize the poi categories
      Gtk::TreeView m_treeView;

      /// Structure for handling TreeView selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_selection;

      /// Underlying data structure
      Glib::RefPtr<Gtk::ListStore> m_listStore;

      /// Internal class for extradata columns:
      class Columns : public Gtk::TreeModel::ColumnRecord
      {
      public:

         Columns()
         { add(m_category); }
         // m_textColor column used by cellRenderer only
         Gtk::TreeModelColumn<Glib::ustring> m_category;
      };

      /// Extradata columns
      Columns m_columns;

};

#endif


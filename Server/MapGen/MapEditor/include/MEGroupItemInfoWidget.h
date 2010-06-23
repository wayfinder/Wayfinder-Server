/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEGROUPITEMINFOWIDGET_H
#define MEGROUPITEMINFOWIDGET_H

#include "config.h"
#include "MEAbstractItemInfoWidget.h"
#include "OldGroupItem.h"
#include <gtkmm/treeview.h>
#include <gtkmm.h>

/**
 *    Widget that shows information about a GroupItem.
 *
 */
class MEGroupItemInfoWidget : public MEAbstractItemInfoWidget {
   public:
      MEGroupItemInfoWidget();
      virtual ~MEGroupItemInfoWidget();
      void activate(MEMapArea* mapArea, OldGroupItem* item);
      
   private:
      void highlightRecursivePressed();
      void highlightPressed();
      void doHighlight(bool recursive);
      void selectItemInGroup();
      MC2String getNameStrForItemInGroup( uint32 id );
      MC2String getItemTypeIDForItemInGroup( uint32 id );

      /*
       * Various TreeViewer structures for groups info
       */

      /// TreeViewer to visualize the extradata
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
         { add(m_pos); add(m_tpe); 
           add(m_ID); add(m_name); }

         Gtk::TreeModelColumn<MC2String> m_pos;
         Gtk::TreeModelColumn<MC2String> m_tpe;
         Gtk::TreeModelColumn<MC2String> m_ID;
         Gtk::TreeModelColumn<MC2String> m_name;
      };

      /// Extradata columns
      Columns m_columns;
};

#endif


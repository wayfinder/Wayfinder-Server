/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEITEMINFOWIDGET_H
#define MEITEMINFOWIDGET_H

#include "config.h"
#include "MEAbstractItemInfoWidget.h"
#include <gtkmm/treeview.h>
#include <gtkmm/entry.h>
#include <gtkmm.h>
/**
 *    Widget that shows information that is common for all Items. That is
 *    information that is stored in the Item class.
 *
 */
class MEItemInfoWidget : public MEAbstractItemInfoWidget {
   public:
      MEItemInfoWidget();
      virtual ~MEItemInfoWidget();
      void activate(MEMapArea* mapArea, OldItem* item);
      
      #ifdef MAP_EDITABLE
         void editNamePressed();
      #endif
         void groupClicked();
         
   private:
      void redrawNames(const OldItem* item, const OldGenericMap* theMap);

      Gtk::TreeView* m_nameList;
      Gtk::TreeView* m_groupList;
      Gtk::Entry* m_itemIDVal;
      Gtk::Entry* m_itemTypeVal;

   friend class MEEditNameDialog; 

      /** 
      * Various datastructures for viewing of groups.
      */

      /// TreeViewer to visualize the groups data
      Gtk::TreeView m_treeViewGroups;

      /// Underlying data structure that contains the groups data
      Glib::RefPtr<Gtk::ListStore> m_listStoreGroups;

      /// Structure for handling m_TreeView selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_selectionGroups;

      /// Internal class for groups columns:
      class ColumnsGroups : public Gtk::TreeModel::ColumnRecord
      {
      public:
         //Pos", "ID", "Type", "Name
         ColumnsGroups()
         { add(m_pos); add(m_ID);
           add(m_type); add(m_name);}

         Gtk::TreeModelColumn<int> m_pos;
         Gtk::TreeModelColumn<Glib::ustring> m_ID;
         Gtk::TreeModelColumn<Glib::ustring> m_type;
         Gtk::TreeModelColumn<Glib::ustring> m_name;
      };

      /// Groups columns
      ColumnsGroups m_columnsGroups;



      /** 
       * Various data structures for viewing of names.
      */

      /// TreeViewer to visualize the names data
      Gtk::TreeView m_treeViewNames;

      /// Underlying data structure that contains the names data
      Glib::RefPtr<Gtk::ListStore> m_listStoreNames;

      /// Structure for handling m_TreeView selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_selectionNames;
      
      /// Internal class for names columns:
      class ColumnsNames : public Gtk::TreeModel::ColumnRecord
      {
      public:
         //Pos", "ID", "Type", "Name
         ColumnsNames()
         { add(m_name); add(m_index); }

         Gtk::TreeModelColumn<Glib::ustring> m_name;
         Gtk::TreeModelColumn<int> m_index;
      };

      /// groups columns
      ColumnsNames m_ColumnsNames;
};

#endif


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESIGNPOSTINFODIALOG_H
#define MESIGNPOSTINFODIALOG_H

#include "config.h"
#include "OldConnection.h"
#include <gtkmm/dialog.h>
#include <gtkmm/togglebutton.h>
#include "MEMapArea.h"
#include "MELogCommentInterfaceBox.h"
#include "MEEditSignPostDialog.h"

/**
 *    A dialog for displaying info about sign post names of a certain 
 *    connection. An edit-dialog is available for editing the sign posts.
 *
 */
class MESignPostInfoDialog : public Gtk::Dialog {
   public:
      /**
       *    Get the one and only instance of this class.
       *    @param conn       The connection for which to display sign posts.
       *    @param toNodeID   The node TO which the connection leads to.
       *    @param mapArea    The map area where the connection is located.
       *                      If no map area is given, the sign posts can
       *                      only be displayed, not edited.
       *    @return The one and only MESignPostInfoDialog.
       */
      static MESignPostInfoDialog* instance(OldConnection* conn, 
                                            uint32 toNodeID,
                                            MEMapArea* mapArea = NULL);

      /**
       *    Delete this dialog.
       */
      virtual ~MESignPostInfoDialog();

      /**
       *    Show the dialog.
       */
      void show();

   private:
      /**
       *    Constructor, private to make sure not called from outside.
       *    @param conn       The connection for which to display sign posts.
       *    @param toNodeID   The node TO which the connection leads to.
       *    @param mapArea    The map area where the connection is located.
       */
      MESignPostInfoDialog( OldConnection* conn,
                           uint32 toNodeID, MEMapArea* mapArea);

      /**
       *    Static member to hold the one and only instance of this class.
       */
      static MESignPostInfoDialog* _staticInstance;

      /**
       *    Fill the sign post info dialog with the sign post names from
       *    the chosen connection.
       */
      void redrawSignPostNames();

      /**
       *    Method taht is called when the window is destroyed.
       *    The window should not be closed, only hidden.
       */
      virtual gint delete_event_impl(GdkEventAny* p0);

      /**
       *    Method that is called when the edit-button is pressed.
       *    This will open the edit-signpost-dialog with the 
       *    selected sign post name for editing.
       */
      void editSpPressed();

      /**
       *    Method that is called when the close-button is pressed.
       *    This will close the sign post info dialog, and the 
       *    edit-signpost-dialog if open.
       */
      void closePressed();

      /** The map area where these sign posts are located.*/
      MEMapArea* m_mapArea;

      /** The currently displayed connection.*/
      OldConnection* m_connection;

      /** Where the displayed connection leads to.*/
      uint32 m_toNodeID;

      /** List with all sign post names in m_connection.*/
      Gtk::TreeView* m_signpostList;
      
      /**
       *    The edit-signpost-dialog that is opened by this dialog,
       *    used for closing the edit-dialog upon close of this dialog.
       */
      MEEditSignPostDialog* m_editSpDialog;
 
      
      /**
       * Various TreeViewer structures for signpost info
       */

      /// TreeViewer to visualize signpost
      Gtk::TreeView m_treeView;

      /// Structure for handling m_TreeView selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_selection;

      /// Underlying data structure that contains the extradata
      Glib::RefPtr<Gtk::ListStore> m_listStore;

      /// Internal class for extradata columns:
      class Columns : public Gtk::TreeModel::ColumnRecord
      {
      public:

         Columns()
         { add(m_info); add(m_index);}

         Gtk::TreeModelColumn<MC2String> m_info;
         Gtk::TreeModelColumn<uint32> m_index;
      };

      /// Extradata columns
      Columns m_columns;

   friend class MEEditSignPostDialog;
};

#endif


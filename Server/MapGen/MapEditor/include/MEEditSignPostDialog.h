/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEEDITSIGNPOSTDIALOG_H
#define MEEDITSIGNPOSTDIALOG_H

#include "config.h"
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/combo.h>
#include "OldGenericMap.h"
#include "OldConnection.h"
#include "MEMapArea.h"
#include "MELogCommentInterfaceBox.h"

class MESignPostInfoDialog;


/**
 *    A dialog to edit the sign posts of a certain connection. It is
 *    possible to remove sign posts from the connection or add new ones.
 *    It is not (yet) possible to update (change) the name of a sign
 *    post, since this functionality is not implemented in extra data
 *    handling.
 *    The dialog is very similar to the MEEditNameDialog.
 *    
 */
class MEEditSignPostDialog : public Gtk::Dialog {
   public:
      /**
       *    Get the one and only instance of this class.
       *    @param mapArea The map-area where the sign posts are located.
       *    @param parent  The dialog with the information about the
       *                   selected sign post - To make sure it 
       *                   is redrawn when this dialog is closed.
       *    @return The one and only MEEditSignPostDialog.
       */
      static MEEditSignPostDialog* instance(MEMapArea* mapArea, 
                                      MESignPostInfoDialog* parent);

      /**
       *    Set the sign post name that should be edited/added. If the 
       *    spIndex is greater than the number of sign posts for the 
       *    given connection, a new sign post is added.
       *
       *    @param spIndex    The cardinal number of the sign post to edit. 
       *                      If more than the number of sign posts for 
       *                      conn, a new sign post is added.
       *    @param conn       The connection to edit/add sign post for.
       *    @param toNodeID   Where the connections leads TO.
       */
      void setName(int spIndex, OldConnection* conn, uint32 toNodeID);

   protected:
      /**
       *    Constructor, protected to make sure not called from outside.
       *    @param mapArea The map-area where the sign posts are located.
       *    @param parent  The dialog with info about the selected sign post.
       */
      MEEditSignPostDialog(MEMapArea* mapArea, MESignPostInfoDialog* parent);

      /**
       *    Don't close this window, hide it.
       */
      virtual gint delete_event_impl(GdkEventAny* p0);
      
      /**
       *    Method that is called when the cancel-button is pressed. This
       *    will cancel any changes that are done to the sign post name.
       */
      void cancelPressed();

      /**
       *    Method that is called when the removeName-button is pressed.
       *    This will remove the selected sign post name.
       */
      void removeNamePressed();

      /**
       *    Method that is called when the ok-button is pressed. This
       *    will commit all changes that are done to the sign post name 
       *    = add a new sign post or update the name of an existing. 
       *    Note that updateSignpost is not implemented.
       */
      void okPressed();

      /**
       *    Close the dialog and reset the map. That is: redraw the names
       *    in the info dialog.
       */
      void internalHide();
     

      int32 getNbrSignPosts(uint32 fromNodeID, uint32 toNodeID) const;

      /** The one and only instance of this class. */
      static MEEditSignPostDialog* _staticInstance;

      /**
       *    The sign post info dialog that created this edit dialog.
       *    To be able to update the sign post list of the info dialog
       *    when a name has been removed/added/changed.
       */
      MESignPostInfoDialog* m_parent;

      /** The map-area where to edit the sign posts. */
      MEMapArea* m_mapArea;

      /** The connection to edit sign post for. */
      OldConnection* m_connection;

      /** The node where the connection leads to. */
      OldNode* m_toNode;

      /**
       *    The sign post index in m_connection.
       *    Valid value if to edit/remove a sign post name 
       *    is 0 <= m_spIndex < m_conection->getNbrSignPost(). 
       *    If the value is outside this range, a new sign post 
       *    will be added to m_connection.
       */
      int m_spIndex;

      /**
       *    @name The sign post name
       *    Some data about the sign post name that will be edited.
       */
      //@{
         /// String index of the original string that is displayed
         uint32 m_stringIndex;
         /// The original type of the name
         ItemTypes::name_t m_nameType;
         /// The original language of the name
         LangTypes::language_t m_nameLanguage;

         /// Connection coordinates string to write in extra data record.
         MC2String m_connAsString;
      //@}

      /**
       *    The entry- and combo-boxes with the values for this name.
       */
      //@{
         /// The name.
         Gtk::Entry* m_nameVal;

         /// The language of the name.
         Gtk::Entry* m_languageVal;

         /// Combo-box with the possible languages.
         Gtk::Combo* m_languageCombo;

         /// The type of the name.
         Gtk::Entry* m_nameTypeVal;

         /// Combo-box with the possible name types.
         Gtk::Combo* m_nameTypeCombo;
      //@}

      /** Box with labels and edit-boxes where to write the log-comments. */
      MELogCommentInterfaceBox* m_logCommentBox;
      
};


#endif


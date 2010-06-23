/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEEDITNAMEDIALOG_H
#define MEEDITNAMEDIALOG_H

#include "config.h"
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/combo.h>
#include "OldGenericMap.h"
#include "OldItem.h"
#include "ItemTypes.h"
#include "MEMapArea.h"
#include "MELogCommentInterfaceBox.h"

class MEItemInfoWidget;


/**
 *    A dialog to change the name of an item. 
 *    
 */
class MEEditNameDialog : public Gtk::Dialog {
   public:
      /**
       *    Get the one and only instance of this class.
       *    @param mapArea       The map-area where to highlight the items.
       *    @param parentDialog  The dialog with the information about the
       *                         selected item- To make sure it is redrawn 
       *                         when this dialog is closed.
       *    @return The one and only MEEditNameDialog.
       */
      static MEEditNameDialog* instance(
               MEMapArea* mapArea, MEItemInfoWidget* parentWidget);

 
      /**
       *    Free the memory of the one instance of this class.
       */
      static void deleteInstance();

      /**
       *    Set the name that should be edited/added. If the nameIndex
       *    is greater than the number of names for the given item, a new
       *    name could be added.
       *
       *    @param nameIndex  The cardinal number of the name to edit. If
       *                      more than the number of names for item, a
       *                      new name is added.
       *    @param item       The item to edit/add name for.
       *    @param theMap     The map where the item and the names are 
       *                      located.
       */
      void setName(int nameIndex, OldItem* item, OldGenericMap* theMap);

   protected:
      /**
       *    Constructor, protected to make sure that it not is called from
       *    outside.
       *    @param mapArea The map-area where to highlight the items.
       */
      MEEditNameDialog(MEMapArea* mapArea, MEItemInfoWidget* parent);

      /**
       *    Don't close this window, hide it.
       */
      virtual gint delete_event_impl(GdkEventAny* p0);
      
      /**
       *    Method that is called when the cancel-button is pressed. This
       *    will cancel all changes that are done to the name.
       */
      void cancelPressed();

      /**
       *    Method that is called when the removeName-button is pressed.
       *    This will remove the selected name.
       */
      void removeNamePressed();

      /**
       *    For remove or update name of street items, the ed records
       *    should be printed for the individual street segments.
       *    This method collects item identification strings for the
       *    street segments of street m_item.
       *    @param   ssiIdentStrings   Outparam where to store the ident
       *                               strings of the street segments.
       *    @param   nameLang          Language of the street name to edit.
       *    @param   nameStr           The street name to edit.
       */
      bool collectIdentStringsForStreetSegments(
                        vector<MC2String>& ssiIdentStrings,
                        LangTypes::language_t nameLang,
                        MC2String nameStr );

      /**
       *    Method that is called when the ok-button is pressed. This
       *    will commit all changes that are done to the name.
       *    Add and update.
       */
      void okPressed();

      /**
       *    Print all the items that will change name. Highlights the
       *    items, whose ids are in the m_changeItems vector.
       */
      void displayItems();

      /**
       *    Close the dialog and reset the map. That is: clear all highlight
       *    except from m_item.
       */
      void internalHide();
      
      /**
       *    The one and only instance of this class.
       */
      static MEEditNameDialog* _staticInstance;

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
      
      /**
       *    The map where to change.
       */
      OldGenericMap* m_map;

      /**
       *    The item to change name for.
       */
      OldItem* m_item;

      /**
       *    The offset of the name in m_item. Valid values if to change name
       *    are 0 <= m_nameOffset < m_item->getNbrNames(). If the value is
       *    outside this range, a new name will be added to m_item.
       */
      int m_nameOffset;

      /**
       *    @name The item-name
       *    Some data about the name of the item that will be edited.
       */
      //@{
         /// String index of the original string that is displayed
         uint32 m_stringIndex;
         /// The original type of the name
         ItemTypes::name_t m_nameType;
         /// The original language of the name
         LangTypes::language_t m_nameLanguage;

         MC2String m_itemAsString;
      //@}

      /**
       *    The ID of the items to change name on.
       */
      Vector m_changeItems;

      /**
       *    The map-area where to draw the items.
       */
      MEMapArea* m_mapArea;

      /// The item info widget that created this MEEditNameDialog.
      MEItemInfoWidget* m_parent;

      /**
       *    A box with labels and edit-boxes where to write source and
       *    comment.
       */
      MELogCommentInterfaceBox* m_logCommentBox;

};


#endif


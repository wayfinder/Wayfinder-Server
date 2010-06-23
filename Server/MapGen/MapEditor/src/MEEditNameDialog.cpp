/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MEEditNameDialog.h"
#include "config.h"
#include "StringUtility.h"
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <list>
#include "MEMapEditorWindow.h"
#include "MEItemInfoWidget.h"
#include "MEFeatureIdentification.h"
#include "OldExtraDataUtility.h"
#include "MELogFilePrinter.h"
#include "CharEncoding.h"
#include "OldStreetItem.h"

MEEditNameDialog*
MEEditNameDialog::instance(MEMapArea* mapArea, MEItemInfoWidget* parent)
{
   // If no instance, create one!
   if (_staticInstance == 0) {
      _staticInstance = new MEEditNameDialog(mapArea, parent);
   }

   // Return the only instance
   return _staticInstance;
}

void
MEEditNameDialog::deleteInstance()
{
   if (_staticInstance != 0) {
      delete _staticInstance;
   }
}


void
MEEditNameDialog::setName(int nameOffset, OldItem* item, OldGenericMap* theMap)
{
   MC2_ASSERT(item != NULL);
   MC2_ASSERT(theMap != NULL);

   if ( item->getItemType() == ItemTypes::pointOfInterestItem ) {
      // Allowed?
      // Yes, if we want quick fix and save the map with MapEditor
      // it is necessary to be able to do this on POIs
      mc2dbg << "Use WASP web to edit names of POIs!!" << endl;
   }

   // Save some information in the membervariables
   m_map = theMap;
   m_item = item;
   m_nameOffset = nameOffset;
   m_stringIndex = m_item->getStringIndex(m_nameOffset);
   m_nameType = m_item->getNameType(m_nameOffset);
   m_nameLanguage = m_item->getNameLanguage(m_nameOffset);

   // Get a string with coordinates etc that identifies this item
   bool identOK = MEFeatureIdentification::getItemIdentificationString(
                  m_itemAsString, m_map, m_item, m_nameOffset);
   if ( ! identOK ) {
      mc2log << warn << " Failed to get item identification string, \"" 
             << m_itemAsString << "\"" << endl;
      mc2log << "Cannot edit the name of this item!" << endl;
      return;
      // if street, it is ok to fail?
      // the identStrings will be on the individual street segments anyway
   }
   mc2dbg8 << "MEEditNameDialog: Got item identification string, \"" 
           << m_itemAsString << "\"" << endl;

   // Set the information in the m_nameVal, m_languageVal and m_nameTypeVal
   m_changeItems.reset();
   mc2dbg8 << "MEEditNameDialog::setName nameOffset=" << nameOffset 
           << " nbrNames=" << int(item->getNbrNames())
           << " m_stringIndex=" << m_stringIndex 
           << " name=" << theMap->getName(m_stringIndex) << endl;
   if (nameOffset < item->getNbrNames()) {
      // Add text to the dialog
      m_nameVal->set_text(theMap->getName(m_stringIndex));
      m_languageVal->set_text(LangTypes::getLanguageAsString(m_nameLanguage));
      m_nameTypeVal->set_text(ItemTypes::getNameTypeAsString(m_nameType));

      // Get the items that will be affected
      m_changeItems.push_back(item->getID());
      // if street: collect the street segments
      // NO, it takes too long time to do the highlighting
      /*
      if ( m_item->getItemType() == ItemTypes::streetItem ) {
         OldStreetItem* street = static_cast<OldStreetItem*>(m_item);
         for ( uint32 g = 0; g < street->getNbrItemsInGroup(); g++ ) {
            m_changeItems.push_back( street->getItemNumber(g) );
         }
         mc2dbg4 << "Edit name on street, but the ed records will be"
                 << " written for the individual street segments" << endl;
      }*/


   } else {
      // No valid string index; to add name. Pre-set default type and lang
      if (m_item->getItemType() == ItemTypes::streetItem) {
         mc2log << error << "Not possible to add names to street items"
                << endl;
         return;
      }
      m_nameVal->set_text("");
      LangTypes::language_t defLang = theMap->getNativeLanguage(0);
      m_languageVal->set_text(LangTypes::getLanguageAsString(defLang));
      m_nameTypeVal->set_text(ItemTypes::getNameTypeAsString(
               ItemTypes::officialName));
   }


   // Light all the items scheduled for change on the map.
   displayItems();

   show_all();
}

MEEditNameDialog::MEEditNameDialog(
         MEMapArea* mapArea, MEItemInfoWidget* parent)
   : Dialog()
{
   // Initiate member
   m_mapArea = mapArea;
   m_parent = parent;
   
   // Some variables used in this method
   Gtk::HBox* hbox;
   Gtk::Label* label;

   set_title("Edit name");
   
   // Instruction string
   label = manage(new Gtk::Label("Update/add names in latin-1"));
   hbox = manage(new Gtk::HBox());
   hbox->pack_start(*label); 
   get_vbox()->pack_start(*hbox);
   
   // Create name-field
   m_nameVal = manage(new Gtk::Entry());
   hbox = manage(new Gtk::HBox());
   label = manage(new Gtk::Label("Name: "));
   hbox->pack_start(*label); 
   hbox->pack_start(*m_nameVal);
   get_vbox()->pack_start(*hbox);

   // Create language-field
   m_languageCombo = manage(new Gtk::Combo());
   m_languageVal = m_languageCombo->get_entry();
   hbox = manage(new Gtk::HBox());
   label = manage(new Gtk::Label("Language: "));
   hbox->pack_start(*label); 
   hbox->pack_start(*m_languageCombo); 
   get_vbox()->pack_start(*hbox);
   // Add all strings to m_languageCombo
   list<string> languageList;
   for (int i=0; i<int(LangTypes::nbrLanguages); i++) {
      languageList.push_back(LangTypes::getLanguageAsString(
               LangTypes::language_t(i)));
   }
   m_languageCombo->set_popdown_strings(languageList);


   // Create type-field
   m_nameTypeCombo = manage(new Gtk::Combo());
   m_nameTypeVal = m_nameTypeCombo->get_entry();
   hbox = manage(new Gtk::HBox());
   label = manage(new Gtk::Label("Type: "));
   hbox->pack_start(*label); 
   hbox->pack_start(*m_nameTypeCombo); 
   get_vbox()->pack_start(*hbox);
   // Add all strings to m_nameCombo
   list<string> nameTypeList;
   for (int i=0; i<int(ItemTypes::maxDefinedName); i++) {
      nameTypeList.push_back(ItemTypes::getNameTypeAsString(
            ItemTypes::name_t(i)));
   }
   m_nameTypeCombo->set_popdown_strings(nameTypeList);

   // Make sure everything is editable
   m_nameVal->set_editable(true);

   // Labels and edit-boxes with comment and source
   m_logCommentBox = manage(new MELogCommentInterfaceBox(
                      m_mapArea->getMapCountryName(), m_mapArea->getMapId()));
   get_vbox()->pack_start(*m_logCommentBox);

   // Create the OK and CANCEL and REMOVE NAME buttons
   Gtk::Button* cancelButton = manage(new Gtk::Button("Cancel"));
   Gtk::Button* removeNameButton = manage(new Gtk::Button("Remove"));
   Gtk::Button* okButton = manage(new Gtk::Button("OK"));
   cancelButton->signal_clicked().connect(sigc::mem_fun(*this, 
					       &MEEditNameDialog::cancelPressed));
   removeNameButton->signal_clicked().connect(
               sigc::mem_fun(*this, &MEEditNameDialog::removeNamePressed));
   okButton->signal_clicked().connect(
               sigc::mem_fun(*this, &MEEditNameDialog::okPressed));
   hbox = manage(new Gtk::HBox());
   hbox->pack_start(*cancelButton);
   hbox->pack_start(*removeNameButton);
   hbox->pack_start(*okButton);
   get_action_area()->pack_start(*hbox);
}

gint
MEEditNameDialog::delete_event_impl(GdkEventAny* p0)
{
   internalHide();
   return 1;
}


void 
MEEditNameDialog::cancelPressed()
{
   mc2dbg8 << "MEEditNameDialog::cancelPressed()" << endl;

   internalHide();
}

void 
MEEditNameDialog::removeNamePressed()
{
   mc2dbg8 << "MEEditNameDialog::removeNamePressed()" << endl;

   // Hide this dialog if name was removed, else keep it open
   bool toHide = false;
   
   if (m_nameOffset < m_item->getNbrNames()) {
      // we have a name to remove
   
      // Get the strings.
      MC2String nameStr = 
         StringUtility::newStrDup(m_nameVal->get_text().c_str());
      const char* languageStr = 
         StringUtility::newStrDup(m_languageVal->get_text().c_str());
      const char* typeStr = 
         StringUtility::newStrDup(m_nameTypeVal->get_text().c_str());
      
      // If we are removing name on street item, the removeName records
      // must be written for the individual street segments
      // Collect info for the street segment items here please
      vector<MC2String> ssiIdentStrings;
      if ( m_item->getItemType() == ItemTypes::streetItem ) {
         collectIdentStringsForStreetSegments( ssiIdentStrings,
               LangTypes::getStringAsLanguage(languageStr), nameStr );
         mc2dbg2 << "Got ident string for " << ssiIdentStrings.size() 
                 << " street segments of street" << endl;
      }
      
      // Make update in map (remove name)
      vector<uint32> foo;
      int counter = OldExtraDataUtility::removeNameFromItem(
            m_map, m_item, m_item->getItemType(),
            nameStr, ItemTypes::getStringAsNameType(typeStr),
            LangTypes::getStringAsLanguage(languageStr), foo);
     
      if (counter > 0) {
         // name was removed
            
         // Create log comment string
         vector<MC2String> logStrings = m_logCommentBox->getLogComments();

         // Empty new value vector
         vector<MC2String> newValStrings;
         
         // Print to log file
         if ( m_item->getItemType() != ItemTypes::streetItem ) {
            MELogFilePrinter::print(
               logStrings, "removeNameFromItem",
               m_itemAsString, newValStrings );
         } else {
            vector<MC2String>::const_iterator identIt;
            for ( identIt = ssiIdentStrings.begin();
                  identIt != ssiIdentStrings.end(); identIt++ ) {
               MELogFilePrinter::print(
                  logStrings, "removeNameFromItem", *identIt, newValStrings);
            }
         }
         mc2dbg << "Name removed: " << nameStr << " from "
                << counter << " items" << endl;
         toHide = true;


         // if it was a street, and it now has no names left, remove it
         if ( m_item->getItemType() == ItemTypes::streetItem ) {
            if ( m_item->getNbrNames() == 0 ) {
               mc2dbg << "Removed all names from the street " 
                      << m_item->getID() << " - removing the street" << endl;
               m_map->removeItem(m_item->getID());
            }
         }
      
      } else {
         mc2log << error << "No such name to remove " << nameStr 
                << ":" << languageStr << ":" << typeStr << endl;
      }
      
   } else {
      mc2log << "Must select an existing name in the name list for removal" 
             << endl;
   }
   
   if (toHide)
      internalHide();
}

void 
MEEditNameDialog::okPressed()
{
   mc2dbg8 << "MEEditNameDialog::okPressed()" << endl;

   // Get the strings. Copy them to avoid errors in the gtk-methods.
   const char* nameStr = 
      StringUtility::newStrDup(m_nameVal->get_text().c_str());
   const char* languageStr = 
      StringUtility::newStrDup(m_languageVal->get_text().c_str());
   const char* typeStr = 
      StringUtility::newStrDup(m_nameTypeVal->get_text().c_str());
   mc2dbg << "new name=" << nameStr << ", language=" << languageStr 
          << ", type=" << typeStr << endl;

   // Make sure that the length of the strings are resonable
   if ( (strlen(nameStr) == 0) || (strlen(languageStr) == 0) || 
        (strlen(typeStr) == 0)) {
      mc2log << warn << "Invalid strings (zero-length): \"" << nameStr 
             << "\", \"" << languageStr << "\", \"" << typeStr << "\"" 
             << endl;
      return;
   }
   
   // Hide this dialog if something is changed, else keep it open
   bool toHide = false;
   
   // Extract the name-type and the language-type. Make sure they are valid.
   ItemTypes::name_t newNameType = ItemTypes::getStringAsNameType(typeStr);
   LangTypes::language_t newLanguage = 
      LangTypes::getStringAsLanguage(languageStr);
   
   // invalidName and invalidLanguage are valid values!
   // but lang should be invalidLang for roadNbr etc
   if ( (newNameType == ItemTypes::roadNumber) &&
        (newLanguage != LangTypes::invalidLanguage) ) {
      mc2log << error << "EditName: Must enter invalidLanguage for"
             << " nameType=roadNumber - name not updated/added" << endl;
   } else if ( (newLanguage == LangTypes::invalidLanguage) &&
               (newNameType == ItemTypes::officialName ) ) {
      mc2log << error << "EditName: Can't use invalidLanguage for nameType:"
                      << ItemTypes::getNameTypeAsString(newNameType) << endl;
   }
   

   // UPDATE_NAME
   else if (m_nameOffset < m_item->getNbrNames()) {
      // Update the name for all names in m_changeNames
      
      // Check if something has changed
      char* oldName = StringUtility::newStrDup(m_map->getName(m_stringIndex));
      bool typeChanged = (newNameType != m_nameType);
      bool langChanged = (newLanguage != m_nameLanguage);
      bool nameChanged = StringUtility::strcmp(nameStr, oldName);
      
      if ( typeChanged || langChanged) {
         mc2log << warn << "UpdateName: type and/or language changed. "
                << "This is NOT supported by the UPDATE_NAME-record" << endl;

      } else if (nameChanged) {
         // The string has changed
      
         // Make update in map
         // update name correct encoded.. (input is latin-1)
         MC2String encodedNewName = nameStr;
#ifdef MC2_UTF8
   // Running on centos, input is UTF8 already
   // If running on some machine interpretting input as latin-1 use these rows!
//         CharEncoding* charEncoder = new CharEncoding(
//            CharEncodingType::iso8859_1, CharEncodingType::UTF8, true );
//         if ( charEncoder != NULL ) {
//            charEncoder->convert(encodedNewName, encodedNewName); // src,dest
//         }
#endif
         
         // If we update name on street item, the updateName records
         // must be written for the individual street segments
         // Collect info for the street segment items here please
         vector<MC2String> ssiIdentStrings;
         if ( m_item->getItemType() == ItemTypes::streetItem ) {
            collectIdentStringsForStreetSegments(
               ssiIdentStrings, m_nameLanguage, oldName );
            mc2dbg2 << "Got ident string for " << ssiIdentStrings.size() 
                    << " street segments of street" << endl;
         }
         
         vector<uint32> foo;
         int counter = OldExtraDataUtility::updateName(
               m_map, m_item, m_item->getItemType(),
               oldName, newNameType, newLanguage, encodedNewName, foo);
         
         if ( counter > 0 ) {
            // name was updated
            mc2dbg << " was updated" << endl;

            // Create log comment string
            vector<MC2String> logStrings = 
                     m_logCommentBox->getLogComments(oldName);
         
            // Create new val string
            vector<MC2String> newValStrings;
            newValStrings.push_back( nameStr );

            // Print to log file
            if ( m_item->getItemType() != ItemTypes::streetItem ) {
               MELogFilePrinter::print(
                  logStrings, "UpdateName", m_itemAsString, newValStrings );
            } else {
               vector<MC2String>::const_iterator identIt;
               for ( identIt = ssiIdentStrings.begin();
                     identIt != ssiIdentStrings.end(); identIt++ ) {
                  MELogFilePrinter::print(
                     logStrings, "UpdateName", *identIt, newValStrings );
               }
            }

            mc2dbg << "Name updated: " << oldName << " -> " << nameStr 
                   << " for " << counter << " items" << endl;
            toHide = true;
            
         } else {
            mc2log << error << "Failed to update name" << endl;
         }
         
      } else {
         mc2dbg << "EditName: Nothing has changed!!!" << endl;
      }
   } // update name

   // ADD_NAME_TO_ITEM
   else {
      
      // don't add an officialName with a language that already exists
      uint32 checkStrIdx = m_item->getNameWithType(newNameType, newLanguage);
      if ((checkStrIdx != MAX_UINT32) && 
          (newNameType == ItemTypes::officialName)) {
         mc2log << error << "Item already has an officialName with language '"
                << LangTypes::getLanguageAsString(newLanguage)
                << "' - new name not added" << endl;
      } else if ((checkStrIdx != MAX_UINT32) &&
                 (StringUtility::strcasecmp(m_map->getName(checkStrIdx),
                                            nameStr) == 0 )) {
         mc2log << error << "Item already has this exact name" << endl;
      }
      
      else if (m_item->getItemType() == ItemTypes::streetItem) {
         mc2log << error << "Cannot add names to street items" << endl;
      }
      else {
         
         // Make update in map (add name)
         // add name correct encoded.. (input is latin-1)
         MC2String encodedNewName = nameStr;
#ifdef MC2_UTF8
   // Running on centos, input is UTF8 already
   // If running on some machine interpretting input as latin-1 use these rows!
//         CharEncoding* charEncoder = new CharEncoding(
//            CharEncodingType::iso8859_1, CharEncodingType::UTF8, true );
//         if ( charEncoder != NULL ) {
//            charEncoder->convert(encodedNewName, encodedNewName); // src,dest
//         }
#endif
         vector<uint32> foo;
         int counter = OldExtraDataUtility::addNameToItem(
                  m_map, m_item, m_item->getItemType(),
                  encodedNewName, newNameType, newLanguage, foo);
         
         if ( counter > 0 ) {

            // Create log comment string
            vector<MC2String> logStrings = m_logCommentBox->getLogComments();

            // Create new val string
            vector<MC2String> newValStrings;
            newValStrings.push_back( nameStr );
            newValStrings.push_back(
                  ItemTypes::getNameTypeAsString(newNameType) );
            newValStrings.push_back(
                  LangTypes::getLanguageAsString(newLanguage) );
            
            // Print to log file (will do char conversion)
            MELogFilePrinter::print(
                  logStrings, "AddNameToItem", m_itemAsString, newValStrings );
            
            mc2dbg << "Name added: "
                   << OldExtraDataUtility::createEDFileString(newValStrings)
                   << " to " << counter << " items" << endl;
            toHide = true;
         
         } else {
            mc2log << error << "AddName: Failed to add name "
                   << nameStr << " to item" << endl;
         }
      
      }
   } // addName

   delete nameStr;
   delete languageStr;
   delete typeStr;

   if (toHide)
      internalHide();
}

void 
MEEditNameDialog::internalHide()
{
   // Clear all highlight except from m_item
   m_mapArea->highlightItem(m_item->getID(), true);
   m_parent->redrawNames(m_item, m_map);
   
   hide();
}

void
MEEditNameDialog::displayItems()
{
   for (uint32 i=0; i<m_changeItems.getSize(); i++) {
      mc2dbg4 << "Added highlight to item with id " << m_changeItems[i] 
              << endl;
      m_mapArea->highlightItem(m_changeItems[i], false);
   }

   DEBUG4(
      // Print the name of the items to the terminal
      mc2log << info << "To change names for " << m_changeItems.getSize()
             << " items" << endl;
      for (uint32 i=0; i<m_changeItems.getSize(); i++) {
         mc2log << info << i << " \"" << m_map->getItemName(m_changeItems[i]) 
                << "\" " << m_changeItems.getElementAt(i) << endl;
      }

      mc2log << info << "-------------------------------------------------" 
             << endl;
   );
}

bool
MEEditNameDialog::collectIdentStringsForStreetSegments(
      vector<MC2String>& ssiIdentStrings,
      LangTypes::language_t nameLang, MC2String nameStr )
{

   OldStreetItem* street = static_cast<OldStreetItem*>(m_item);
   for ( uint32 g = 0; g < street->getNbrItemsInGroup(); g++ ) {
      OldItem* ssi = m_map->itemLookup( street->getItemNumber(g) );
      // Get the name offset for the name of this ssi
      // The name type may differ, but the lang must be equal
      bool found = false; int nos = 0;
      LangTypes::language_t ssiLang; ItemTypes::name_t ssiType;
      uint32 ssiIdx;
      while ( !found &&
              ( nos < ssi->getNbrNames() ) ) {
         if (ssi->getNameAndType(nos, ssiLang, ssiType, ssiIdx)) {
            if ( (ssiLang == nameLang) &&
                 (strcmp( m_map->getName(ssiIdx),
                          nameStr.c_str()) == 0) ) {
               found = true;
            }
         }
         if ( !found ) {
            nos++;
         }
      }
      if ( found ) {
         // create a identString for this ssi and this nameOffset
         MC2String identString;
         bool identOK = 
            MEFeatureIdentification::getItemIdentificationString(
                        identString, m_map, ssi, nos);
         if ( identOK ) {
            ssiIdentStrings.push_back( identString );
            mc2dbg8 << "got ident string for ssi " << g << " " 
                    << ssi->getID() << " of street" << endl;
         } else {
            mc2log << error << " can not get item identification for "
                 << "ssi " << ssi->getID() << " of street" << endl;
         }
      }
   }

   return ( ssiIdentStrings.size() == street->getNbrItemsInGroup() );
}


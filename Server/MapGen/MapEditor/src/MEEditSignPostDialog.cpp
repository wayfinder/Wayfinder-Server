/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MEEditSignPostDialog.h"
#include "config.h"
#include "StringUtility.h"
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <list>
#include "MEMapEditorWindow.h"
#include "MESignPostInfoDialog.h"
#include "MEFeatureIdentification.h"
#include "OldExtraDataUtility.h"
#include "MELogFilePrinter.h"
#include "OldNode.h"

MEEditSignPostDialog*
MEEditSignPostDialog::_staticInstance;

MEEditSignPostDialog*
MEEditSignPostDialog::instance(MEMapArea* mapArea, MESignPostInfoDialog* parent)
{
   // If no instance, create one!
   if (_staticInstance == 0) {
      _staticInstance = manage(new MEEditSignPostDialog(mapArea, parent));
   }

   // Return the only instance
   return _staticInstance;
}

void
MEEditSignPostDialog::setName(
      int spIndex, OldConnection* conn, uint32 toNodeID)
{
   MC2_ASSERT(conn != NULL);
   MC2_ASSERT(m_mapArea != NULL);
   OldGenericMap* theMap = m_mapArea->getMap();
   OldNode* toNode = theMap->nodeLookup(toNodeID);
   MC2_ASSERT(toNode != NULL);

   // Save some information in the membervariables
   m_connection = conn;
   m_toNode = toNode;
   m_spIndex = spIndex;
   
   m_nameVal->set_text("");

   uint32 fromNodeID = m_connection->getConnectFromNode();
   const SignPostTable& spTable = theMap->getSignPostTable();
   const vector<GMSSignPost*>& signPosts = 
      spTable.getSignPosts(fromNodeID, toNode->getNodeID());
   int32 curIndex = 0;
   for (uint32 i=0; i<signPosts.size(); i++){
      const vector<GMSSignPostSet>& spSets = signPosts[i]->getSignPostSets();
      for (uint32 j=0; j<spSets.size(); j++){
         const vector<GMSSignPostElm>& spElms = 
            spSets[j].getSignPostElements();
         for (uint32 k=0; k<spElms.size(); k++){
            if ( curIndex == spIndex ){
               m_nameVal->set_text(spElms[k].getTextString(*theMap));
            }
            curIndex++;
         }
      }
   }

   // Get a string with coordinates etc that identifies this connection
   // The string is written in extra data format
   bool identOK = 
      MEFeatureIdentification::getConnectionIdentificationString(
                     m_connAsString, theMap, m_connection, m_toNode );
   if ( identOK ) {
      mc2dbg4 << here << " Got conn identification string, \"" 
              << m_connAsString << "\"" << endl;
   } else {
      mc2log << warn << " Failed to get conn identification string, \"" 
             << m_connAsString << "\"" << endl;
      return;
   }

   LangTypes::language_t defaultLang = theMap->getNativeLanguage(0);
   m_languageVal->set_text(LangTypes::getLanguageAsString(defaultLang));
   m_nameTypeVal->set_text(ItemTypes::getNameTypeAsString(
               ItemTypes::officialName));
   
   show_all();
}

MEEditSignPostDialog::MEEditSignPostDialog(MEMapArea* mapArea,
                                           MESignPostInfoDialog* parent)
   : Dialog()
{
   // Initiate member
   m_mapArea = mapArea;
   m_parent = parent;
   
   set_title("Edit signpost");
   
   // Instruction string
   Gtk::Label* label = manage(new Gtk::Label("Add signpost names in latin-1"));
   Gtk::HBox* hbox = manage(new Gtk::HBox());
   hbox->pack_start(*label); 
   get_vbox()->pack_start(*hbox);
   
   // Create name-field
   m_nameVal = manage(new Gtk::Entry());
   hbox = manage(new Gtk::HBox());
   label = manage(new Gtk::Label("Sign post name: "));
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
   m_languageVal->set_editable(false);

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
   m_nameTypeVal->set_editable(false);

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
   cancelButton->signal_clicked().connect(
               sigc::mem_fun(*this, &MEEditSignPostDialog::cancelPressed));
   removeNameButton->signal_clicked().connect(
               sigc::mem_fun(*this, &MEEditSignPostDialog::removeNamePressed));
   okButton->signal_clicked().connect(
               sigc::mem_fun(*this, &MEEditSignPostDialog::okPressed));
   hbox = manage(new Gtk::HBox());
   hbox->pack_start(*cancelButton);
   hbox->pack_start(*removeNameButton);
   hbox->pack_start(*okButton);
   get_action_area()->pack_start(*hbox);
}

gint
MEEditSignPostDialog::delete_event_impl(GdkEventAny* p0)
{
   internalHide();
   return 1;
}


void 
MEEditSignPostDialog::cancelPressed()
{
   mc2dbg4 << "MEEditSignPostDialog::cancelPressed()" << endl;

   internalHide();
}

void 
MEEditSignPostDialog::removeNamePressed()
{
   mc2dbg4 << "MEEditSignPostDialog::removeNamePressed()" << endl;

   bool toHide = false; 
   if (m_spIndex < getNbrSignPosts(m_connection->getConnectFromNode(), 
                                   m_toNode->getNodeID()) ){
      // we have a name to remove
   
      // Get the strings.
      MC2String nameStr = 
         StringUtility::newStrDup(m_nameVal->get_text().c_str());
      const char* languageStr = 
         StringUtility::newStrDup(m_languageVal->get_text().c_str());
      const char* typeStr = 
         StringUtility::newStrDup(m_nameTypeVal->get_text().c_str());
      // Create signpostName str (name of the signpost to remove)
      vector<MC2String> identNameStrings;
      identNameStrings.push_back( nameStr );
      identNameStrings.push_back( languageStr );
      identNameStrings.push_back( typeStr );
      MC2String identNameStr = 
            OldExtraDataUtility::createEDFileString(identNameStrings);
    
      OldGenericMap* theMap = m_mapArea->getMap();
      uint32 fromNodeID = m_connection->getConnectFromNode();
      bool removedWholeSingPost = 
         m_toNode->removeSignPost( *theMap, fromNodeID, nameStr);

      // Create log comment string
      vector<MC2String> logStrings = m_logCommentBox->getLogComments();
      
      // Both m_connAsString and identNameStr should belong to identString
      // printed with MELogFilePrinter (to handle map char encoding)
      // There is no new value to print.
      MC2String identString = m_connAsString + identNameStr;
         
      // Empty new value vector
      vector<MC2String> newValStrings;
      
      // Print to log file
      MELogFilePrinter::print(
               logStrings, "removeSignpost", identString, newValStrings );
      mc2dbg << "Signpost removed: " << nameStr << endl;
      toHide = true;
      
      if (removedWholeSingPost){
         mc2log << error << "The whole sign post was removed: "
                << identNameStr << endl;
      }
      
   } else {
      mc2log << "Must select an existing sign post name "
             << "in the name list for removal" << endl;
   }
   
   if (toHide)
      internalHide();
   
}

void 
MEEditSignPostDialog::okPressed()
{
   mc2dbg4 << "MEEditSignPostDialog::okPressed()" << endl;

   // Get the strings. Copy them to avoid errors in the gtk-methods.
   const char* nameStr = 
      StringUtility::newStrDup(m_nameVal->get_text().c_str());
   const char* languageStr = 
      StringUtility::newStrDup(m_languageVal->get_text().c_str());
   const char* typeStr = 
      StringUtility::newStrDup(m_nameTypeVal->get_text().c_str());
   // Create signpostName str (name of the signpost to edit)
   vector<MC2String> identNameStrings;
   identNameStrings.push_back( nameStr );
   identNameStrings.push_back( languageStr );
   identNameStrings.push_back( typeStr );
   MC2String identNameStr = 
         OldExtraDataUtility::createEDFileString(identNameStrings);
   mc2dbg4 << "new name=" << identNameStr << endl;

   // Make sure that the length of the strings are resonable
   if ( (strlen(nameStr) == 0) || (strlen(languageStr) == 0) || 
        (strlen(typeStr) == 0)) {
      mc2log << warn << "Invalid strings (zero-length): \"" << nameStr 
             << "\", \"" << languageStr << "\", \"" << typeStr << "\"" 
             << endl;
      return;
   }
   
   // Extract the name-type and the language-type. Make sure that they 
   // are valid.
   bool toHide = false;
   ItemTypes::name_t newNameType = ItemTypes::getStringAsNameType(typeStr);
   LangTypes::language_t newLanguage = 
      LangTypes::getStringAsLanguage(languageStr);
   // invalidName and invalidLanguage are valid values!
   // but lang should be invalidLang for roadNbr etc
   if ( (newNameType == ItemTypes::roadNumber) &&
        (newLanguage != LangTypes::invalidLanguage) ) {
      mc2log << error << "Must enter invalidLanguage for"
             << " nameType=roadNumber - name not updated/added" << endl;

   } else if ( (newLanguage == LangTypes::invalidLanguage) &&
               (newNameType == ItemTypes::officialName ) ) {
      mc2log << error << "Can't use invalidLanguage for nameType:"
                      << ItemTypes::getNameTypeAsString(newNameType) << endl;
      
   } else if (m_spIndex < getNbrSignPosts(m_connection->getConnectFromNode(),
                                          m_toNode->getNodeID())) {
      // Update sign post name not implemented
      // no extradata record exists for this feature.
      
      mc2dbg << "updateSignpost is not implemented!" << endl
             << " - try removeSignpost + addSignpost instead." << endl;

   } else {
      // Add new sign post
      
      //uint32 newSP = m_mapArea->getMap()->addSignPostName(
      //m_connection, nameStr, newLanguage, newNameType);

      uint32 fromNodeID = m_connection->getConnectFromNode();
      uint32 toNodeID = m_toNode->getNodeID();
      
      OldGenericMap* theMap = m_mapArea->getMap();
      OldConnection* con;
      theMap->getConnection(fromNodeID, toNodeID, con);
      if (con != NULL) {
         // Make sure that a connection exists between the nodes.
         
         // Compose a sign post element.
         GMSSignPostElm signPostElm;
         uint32 nameStringCode = theMap->addName(nameStr,
                                                 newLanguage,
                                                 ItemTypes::officialName);
         signPostElm.setTextStringCode(nameStringCode);
         signPostElm.setType(GMSSignPostElm::placeName);
         
         bool result = 
            theMap->getNonConstSignPostTable().addSingleElmSignPost(fromNodeID,
                                                                    toNodeID,
                                                                    signPostElm);
         if (!result){
            //if ( newSP == MAX_UINT32 ) {
            mc2log << error << "AddSignpost: Failed to add signpost "
                   << identNameStr << endl;
            
         } else {
            // sign post was added
            
            // Create log comment string
            vector<MC2String> logStrings = m_logCommentBox->getLogComments();
            
            // Print to log-file
            MELogFilePrinter::print(
               logStrings, "addSignpost", m_connAsString, identNameStrings );
            mc2dbg << "Signpost added: " << identNameStr << endl;
            toHide = true;
         }
      }
   }
   delete nameStr;
   delete languageStr;
   delete typeStr;


   if (toHide)
      internalHide();
}

int32
MEEditSignPostDialog::getNbrSignPosts(uint32 fromNodeID, uint32 toNodeID) const
{
   OldGenericMap* theMap = m_mapArea->getMap();
   const SignPostTable& spTable = theMap->getSignPostTable();
   const vector<GMSSignPost*>& signPosts = 
      spTable.getSignPosts(fromNodeID, toNodeID);

   int32 nbrSignPosts = 0;
   for (uint32 i=0; i<signPosts.size(); i++){
      const vector<GMSSignPostSet>& spSets = signPosts[i]->getSignPostSets();
      for (uint32 j=0; j<spSets.size(); j++){
         const vector<GMSSignPostElm>& spElms = 
            spSets[j].getSignPostElements();
         for (uint32 k=0; k<spElms.size(); k++){
            nbrSignPosts++;
         }
      }
   }
   return nbrSignPosts;
} // getNbrSignPosts



void 
MEEditSignPostDialog::internalHide()
{
   m_parent->redrawSignPostNames();
   
   hide();
}


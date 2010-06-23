/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "MELogCommentInterfaceBox.h"
#include <gtkmm/label.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include "StringUtility.h"
#include <list>


MELogCommentInterfaceBox::MELogCommentInterfaceBox(
                           const char* countryName, uint32 mapId)
{
   // set members
   m_countryName = countryName;
   m_mapId = mapId;
   
   // Create the logbox, which will include all boxes for
   // source, comment, orig value and ref ID
   Gtk::VBox* logBox = manage(new Gtk::VBox());

   // Instruction string
   Gtk::Label* label = manage(new Gtk::Label("Add log comments in latin-1"));
   Gtk::HBox* instructBox = manage(new Gtk::HBox());
   instructBox->pack_start(*label); 
   
   // Create a hbox with the source and comment (label + edit)
   Gtk::HBox* sourceAndCommentBox = manage(new Gtk::HBox());
   // vbox with the source (label + edit)
   Gtk::VBox* tmpVbox = manage(new Gtk::VBox());
   label = new Gtk::Label("source");
   m_mapEditSource = new Gtk::Entry();
   tmpVbox->pack_start(*label);
   tmpVbox->pack_start(*m_mapEditSource);
   // Add to the hbox
   sourceAndCommentBox->pack_start(*tmpVbox);
   // vbox with the comment (label + edit)
   tmpVbox = manage(new Gtk::VBox());
   label = new Gtk::Label("comment");
   m_mapEditComment = new Gtk::Entry();
   tmpVbox->pack_start(*label);
   tmpVbox->pack_start(*m_mapEditComment);
   // Add to the hbox
   sourceAndCommentBox->pack_start(*tmpVbox);

   // Create a hbox with ed insert type and group
   Gtk::HBox* typeAndGroupBox = manage(new Gtk::HBox());
   // Create the insert type box
   tmpVbox = manage(new Gtk::VBox());
   label = new Gtk::Label("ed insert type");
   m_mapEditEDInsertTypeCombo = manage(new Gtk::Combo());
   m_mapEditEDInsertType = m_mapEditEDInsertTypeCombo->get_entry();
   tmpVbox->pack_start(*label);
   tmpVbox->pack_start(*m_mapEditEDInsertTypeCombo);
   // add strings to the combo
   list<string> insertTypeList;
   insertTypeList.push_back("");
   insertTypeList.push_back("beforeInternalConnections");
   insertTypeList.push_back("beforeGenerateStreets");
   insertTypeList.push_back("beforeGenerateTurndescriptions");
   insertTypeList.push_back("afterGenerateTurndescriptions");
   insertTypeList.push_back("dynamicExtradata");
   m_mapEditEDInsertTypeCombo->set_popdown_strings(insertTypeList);
   m_mapEditEDInsertType->set_text(""); // default is no type
   // Add to the hbox
   typeAndGroupBox->pack_start(*tmpVbox);
   // Create the group box
   tmpVbox = manage(new Gtk::VBox());
   label = new Gtk::Label("ed group");
   m_mapEditEDGroup = new Gtk::Entry();
   tmpVbox->pack_start(*label);
   tmpVbox->pack_start(*m_mapEditEDGroup);
   // Add to the hbox
   typeAndGroupBox->pack_start(*tmpVbox);

   // Create a hbox with ref ID and empty (to enable symetric logboxes)
   Gtk::HBox* refIdBox = manage(new Gtk::HBox());
   // Create the refID box
   tmpVbox = manage(new Gtk::VBox());
   label = new Gtk::Label("mapsupplier ref ID");
   m_mapEditRefID = new Gtk::Entry();
   tmpVbox->pack_start(*label);
   tmpVbox->pack_start(*m_mapEditRefID);
   // Add to the hbox
   refIdBox->pack_start(*tmpVbox);
   // Create the empty box
   tmpVbox = manage(new Gtk::VBox());
   label = new Gtk::Label("(empty)");
   m_mapEditEmpty = new Gtk::Entry();
   tmpVbox->pack_start(*label);
   tmpVbox->pack_start(*m_mapEditEmpty);
   // Add to the hbox
   refIdBox->pack_start(*tmpVbox);

   logBox->pack_start(*instructBox);
   logBox->pack_start(*sourceAndCommentBox);
   logBox->pack_start(*typeAndGroupBox);
   logBox->pack_start(*refIdBox);
   pack_start(*logBox);

   m_mapEditEmpty->set_state(Gtk::STATE_INSENSITIVE);
}

vector<MC2String> 
MELogCommentInterfaceBox::getLogComments( const char* origValStr )
{
   // Don't change the order in the log comment! Add new ones at the end.
   
   vector<MC2String> logStrings;
   char tmpStr[64];
   
   // 0 Add date
   time_t currentTime = time(NULL);
   sprintf(tmpStr, "%c# %s", '\n', asctime(localtime(&currentTime)));
   tmpStr[strlen(tmpStr)-1] = '\0'; // remove \n at end of tmpStr
   logStrings.push_back( tmpStr );
   
   // 1 Add username
   struct passwd* pass = getpwuid(getuid());
   if (pass != NULL) {
      sprintf(tmpStr, "%s", pass->pw_name);
   } else {
      sprintf(tmpStr, "unknown");
   }
   logStrings.push_back( tmpStr );

   // 2 Add source
   logStrings.push_back( m_mapEditSource->get_text().c_str() );
   
   // 3 Add comment
   logStrings.push_back( m_mapEditComment->get_text().c_str() );

   // 4 Add original value
   logStrings.push_back( origValStr );

   // 5 Add ref id e.g. TA CARE
   logStrings.push_back( m_mapEditRefID->get_text().c_str() );

   // 6 Add room for extra data record id (not possible to add in MapEditor)
   logStrings.push_back( "" );

   // 7 Add extra data record insert type
   logStrings.push_back( m_mapEditEDInsertType->get_text().c_str() );

   // 8 Add map release (empty string for now)
   logStrings.push_back( "" );

   // 9 Add extra data group
   StringUtility::strlcpy(tmpStr, m_mapEditEDGroup->get_text().c_str(), 64);
   if (strlen(tmpStr) == 0) {
      // default is 0 (= no group)
      sprintf(tmpStr, "%d", 0);
   }
   logStrings.push_back( tmpStr );
   
   // 10 Add country string (from the map)
   logStrings.push_back( m_countryName );
   
   // 11 Add map id (from the map)
   sprintf(tmpStr, "%d", m_mapId);
   logStrings.push_back( tmpStr );
   
   // 12 Add "EndOfRecord;"
   logStrings.push_back( "EndOfRecord" );

   DEBUG4(
   mc2dbg4 << "LogStrings: \"";
   for ( vector<MC2String>::iterator it = logStrings.begin();
         it != logStrings.end(); it++ ) {
      mc2dbg4 << *it << MC2MapGenUtil::poiWaspFieldSep;
   }
   mc2dbg4 << "\"" << endl;
   );

   return logStrings;
}


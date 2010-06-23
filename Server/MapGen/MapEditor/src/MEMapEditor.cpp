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
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>

#include "OldGenericMap.h"
#include "MEMapEditorWindow.h" 
#include "MEEditNameDialog.h" 
#include "MEDrawSettingsDialog.h" 
#include "METurnRestrictionsDialog.h" 
#include "CommandlineOptionHandler.h"
#include "GDFRef.h"
#include "MC2Coordinate.h"
#include "MEGdkColors.h"

MEMapEditorWindow* g_mapEditorWindow;
bool g_utf8Strings;

// Quit app when user closes main window:
gint 
on_win_delete_event(GdkEventAny* event)
{
   Gtk::Main::quit();
   return FALSE;
};


MEItemInfoDialog* MEItemInfoDialog::_instance(0); 
MEEditNameDialog* MEEditNameDialog::_staticInstance(0);
MEDrawSettingsDialog* MEDrawSettingsDialog::_staticInstance(0);
METurnRestrictionsDialog* METurnRestrictionsDialog::_staticInstance(0); 



int main(int argc, char** argv)
{
   CommandlineOptionHandler coh(argc, argv, 1);
   coh.setTailHelp("map");
   coh.setSummary("Load a mcm map to inspect.\n"
                  "The map is either the full path of a map including directory, "
                  "or the hex id of a map in the current directory.");

   // add options to coh here
   
   // ------------------------------------------------------- highlight items
   char* o_highlightCoords = NULL;
   coh.addOption("", "--highlightCoords",
                 CommandlineOptionHandler::stringVal,
                 1, &o_highlightCoords, "\0",
                 "Load a text file with mc2 coordinates, space separated "
                 "lat lon per row, and highlight them. Example of file:\n"
                 "664592528 159265161\n"
                 "707464888 215690097\n");

   char* o_highlightIDs = NULL;
   coh.addOption("", "--highlightIDs",
                 CommandlineOptionHandler::stringVal,
                 1, &o_highlightIDs, "\0",
                 "Load a text file with item IDs, one item ID per row, "
                 "and highlight the items.");


   char* o_gdfRefDir = NULL;
   coh.addOption("", "--gdfRefDir",
                 CommandlineOptionHandler::stringVal,
                 1, &o_gdfRefDir, "\0",
                 "Load a text file with GDF IDs from the directory given "
                 "in this option." );

   bool o_gdfRef = false;
   coh.addOption("-g", "--gdfRef",
                 CommandlineOptionHandler::presentVal,
                 0, &o_gdfRef, "F",
                 "Load a text file with GDF IDs. Assumes it to be in the "
                 "current dir or in a subdir called temp. If the GDF ref "
                 "file is stored in any other place, point to the dir with "
                 "the --gdfRefDir option.");

   // Parse command-line
   if(!coh.parse()) {
      cerr << argv[0] << ": Error on commandline! (-h for help)" << endl;
      exit(1);
   }

   // Make sure that there is only ONE map file in tail
   if ( coh.getTailLength() != 1 ) {
      mc2log << error << argv[0] << ": Specify only one map in tail!" << endl;
      exit(1);
   }

   // Initialize the GTK-system
   Gtk::Main kit(argc, argv);

   // Create the map given as argument in tail
   const char* mapName = coh.getTail(0);
   OldGenericMap* theMap = OldGenericMap::createMap(mapName);
    
   if (theMap == NULL) {
      mc2log << fatal << " Failed to load map \"" << mapName << "\" "
             << "Terminating" << endl;
      exit(0);
   } else {
      mc2log << info << "Map \"" << mapName << "\" loaded" << endl;

   }

   // Create the windows
   MC2String gdfRefDir;
   if ( o_gdfRefDir != NULL ){
      gdfRefDir = o_gdfRefDir;
   }
   else if (o_gdfRef){
      gdfRefDir = "_FIND_";
   }
   MEMapEditorWindow* win = new MEMapEditorWindow(theMap, 
                                                  gdfRefDir);
  
   g_mapEditorWindow = win;

   // Utf-8 or not?
   g_utf8Strings = theMap->stringsCodedInUTF8();
   if ( g_utf8Strings )
      cout << "    map string are utf-8" << endl;
   else
      cout << "    map strings are not utf-8" << endl;
#ifdef MC2_UTF8
   cout << "    mc2 server code is utf-8" << endl;
#else
   cout << "    mc2 server code is not utf-8" << endl;
#endif
   
   win->signal_delete_event().connect(sigc::ptr_fun(&on_win_delete_event)); 

   win->show_all();
  
   // Read coords from highlight file.
   if ( o_highlightCoords != NULL ){
      mc2log << info << "Highligt coords file: " << o_highlightCoords << endl;
      ifstream file( o_highlightCoords, ios::in );
      if ( ! file ) {
         mc2log << error << "Could not open highlight file"  <<endl;
         mc2log << error << "File name: " << o_highlightCoords << endl;
         exit(1);
      }
      
      vector<MC2Coordinate> highligthCoords;
      int32 tmpLat = MAX_INT32;
      int32 tmpLon = MAX_INT32;
      file >> tmpLat;
      file >> tmpLon;
      while ( ! file.eof() ) {
         if ( (tmpLat == MAX_INT32) || (tmpLon == MAX_INT32) ){
            mc2log << error << "Something wrong in highlight file." 
                   << endl;
            exit(1);
         }
         highligthCoords.push_back(MC2Coordinate(tmpLat, tmpLon));
         file >> tmpLat;
         file >> tmpLon;
      }
      file.close();
      win->highlightCoords(highligthCoords);
   }


   // Read IDs from highlight file.
   if ( o_highlightIDs != NULL ){
      mc2log << info << "Highligt IDs file: " << o_highlightIDs << endl;
      ifstream file( o_highlightIDs, ios::in );
      if ( ! file ) {
         mc2log << error << "Could not open highlight file"  <<endl;
         mc2log << error << "File name: " << o_highlightIDs << endl;
         exit(1);
      }
      
      // Used for storing item IDs and colors to draw the items in.
      vector< pair <uint32, MEGdkColors::color_t > > highligthIDs; 

      // Read the first ID
      uint32 tmpID = MAX_UINT32;
      file >> tmpID;

      // Check if we have a color value, in that case read it.
      char testChar = file.peek();
      MC2String colorOrComment ="";
      if ( testChar != '\n' && !file.eof() ){
         file >> colorOrComment;
      }

      // We had at least one color value. Initiate the special draw settings.
      if ( colorOrComment != "" ){
         //win->initiateShowPOIs(); 
      }

      while ( ! file.eof() ) {
         if ( tmpID == MAX_UINT32 ){
            mc2log << error << "Something wrong in highlight file." 
                   << endl;
            exit(1);
         }
         
         // Handle color value.
         MEGdkColors::color_t color = 
            MEGdkColors::getColorFromString(colorOrComment.c_str()); 
         highligthIDs.push_back(make_pair(tmpID, color)); 
         
         // Read next row.
         file >> tmpID;
         char testChar = file.peek();
         colorOrComment ="";
         if ( testChar != '\n' && !file.eof() ){
            file >> colorOrComment;
         }
      }
      file.close();
      win->highlightItems(highligthIDs);
      mc2dbg1 << "Loaded " << highligthIDs.size() << " ids from file"
              << endl;
   }

   // Start GTK
   kit.run(); 

   // Clean up and return
   delete win;
   return 0;
}



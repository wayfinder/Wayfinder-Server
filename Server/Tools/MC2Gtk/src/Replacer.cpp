/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Replacer.h"

#include "XMLUIParser.h"

#include <iostream>
#include <fstream>
#include <algorithm>

namespace MC2Gtk {


Replacer::Replacer( const char* fileToReplace, 
                    const char* uiFilename ) throw (XMLUI::Exception):
   m_srcFilename( fileToReplace ) {
try {

   using namespace XMLUI;

   {// setup system
      UIParser::WidgetVector windows = UIParser::parseFile( uiFilename );
      for_each( windows.begin(), windows.end(),
             ptr_fun( gtk_widget_show_all ) );
   }

   // get all widget ids and find those that end with "_replace" 
   // which should be entries 
   typedef vector<MC2String> Strings;
   Strings ids = UIParser::getWidgetIDs();
   Strings::const_iterator it = ids.begin();
   Strings::const_iterator itEnd = ids.end();
   for ( ; it != itEnd; ++it ) {
      size_t strIt = (*it).find( "_replace" );
      if ( strIt == MC2String::npos ) {
         continue;
      }
      // remove _replace at the end and add to our replace str<->GtkWidget map
      MC2String replaceStr = (*it);
      replaceStr.erase( strIt );
      m_replaceEntries.insert( make_pair( replaceStr, 
                                          UIParser::getWidget( (*it) ) ) );

   }
   
} catch ( const MC2Gtk::XMLUI::Exception& e ) {
   throw MC2Gtk::XMLUI::Exception( MC2String("Replacer:" ) +
                                   e.what() );
}

}

void Replacer::replace(const MC2String& destFilename ) 
   throw (FileUtils::FileException ) {
   
   mc2log << "Replace: " << m_srcFilename << endl;

   using FileUtils::FileException;

   // open in/output file 

   ifstream infile( m_srcFilename.c_str() );
   if ( ! infile ) {
      throw FileException( MC2String( "Could not open input file:" ) + 
                           m_srcFilename );
   }

   ofstream outfile( destFilename.c_str() );
   if ( ! outfile  ) {
      throw FileException( MC2String( "Could not open output file: " ) + 
                           destFilename );
   }

   while ( ! infile.eof() ) {

      std::string line;
      getline( infile, line );
      line += "\n"; // readd new line

      using namespace XMLUI;

      // replace each line
      EntryMap::const_iterator it = m_replaceEntries.begin();
      EntryMap::const_iterator itEnd = m_replaceEntries.end();
      for ( ; it != itEnd; ++it ) {
         // string to be replace must be in the format ${thevalue}
         MC2String replaceString("${");
         replaceString += (*it).first + "}";

         size_t replacePos = line.find( replaceString );
         if ( replacePos == MC2String::npos ) {
            continue;
         }

         line.replace( replacePos, replaceString.size(), 
                       gtk_entry_get_text( GTK_ENTRY( (*it).second ) ) );
      }
      outfile << line;
   }
}


}

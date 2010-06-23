/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2GTK_REPLACER_H
#define MC2GTK_REPLACER_H

#include "XMLUIParser.h"
#include "FileException.h"

namespace MC2Gtk {
/**
 * Loads and creates a gui but uses special id tags to replace
 * text in a file. all ids ending with _replace will be used with their
 * text values to replace ${variable} in source file.
 * Example:  <entry id="server_replace" value="entryvalue"/> 
 * and the source file has:
 * "something ${server} somethingelse"  
 * Then the destination file will get
 * (unless the entry value was changed during runtime):
 * "something entryvalue somethingelse"
 *
 */
class Replacer {
public:
   /**
    * @param fileToReplace the source file
    * @param uiFilename the user interface file
    */
   Replacer( const char* fileToReplace,
             const char* uiFilename ) throw (XMLUI::Exception);
   /**
    * Replaces variables in src file to destination file, throw file exception 
    * if source or destination file had errors.
    * @param destFile destination file 
    */
   void replace( const MC2String& destFilename ) throw (FileUtils::FileException);

private:

   MC2String m_srcFilename; //< source filename
   
   /// entries that contain text that should replaced in the src file
   typedef map<MC2String, GtkWidget*> EntryMap;
   EntryMap m_replaceEntries;

};

}

#endif 

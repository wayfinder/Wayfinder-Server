/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpFileHandler.h"

#include "Properties.h"

#include "Utility.h"

#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



HttpFileHandler::HttpFileHandler(){
   mc2dbg8 << "HttpFileHandler getting root" << endl;
   const char* root = Properties::getProperty("HTML_ROOT");
   mc2dbg8 << "root is " << root << endl;
   if ( root == NULL ) root = "./";
   m_htmlRoot = new MC2String(root);
   MC2String::size_type pos = 0;

   while ( pos < m_htmlRoot->size() ) {
#ifdef _WIN32
      pos = m_htmlRoot->find('/', pos);
      //mc2dbg8 << "found / at: " << pos << endl;
      if ( pos != MC2String::npos )
         (*m_htmlRoot)[pos] = '\\'; 
#else
      pos = m_htmlRoot->find('\\', pos);
      //mc2dbg8 << "found \\ at: " << pos << endl;
      if ( pos != MC2String::npos )
         (*m_htmlRoot)[pos] = '/';
#endif
   }
}


HttpFileHandler::~HttpFileHandler(){
   delete m_htmlRoot;
}


byte*
HttpFileHandler::getFile(const MC2String& fileString, 
			 int& length, 
			 struct stat* status) {
   MC2String fileName; 
   byte* inBuff = NULL;
   int file;
   uint32 fSize;
   size_t pos;
   // Convert / and \ to correct values for filesystem.
   pos = 0;
   fileName = fileString;
   
#ifdef __linux
   /*
   while ( pos < fileName.size() ) {
     pos = fileName.find('\\', pos);
      //  mc2dbg8 << "found \\ at: " << pos << endl;
      if ( pos != MC2String::npos ) 
	 fileName.replace(pos, 1, 1, '/');
   }
   */
#endif
#ifdef _WIN32
   while ( pos < fileName.size() ) {
      pos = fileName.find('/', pos);
      if ( pos != MC2String::npos )
         fileName.replace(pos, 1, 1, '\\'); 
   }
#endif   

   
   // Test if above the root
   if ( (fileString.find("../") != MC2String::npos) ||
        (fileString.find("..\\") != MC2String::npos) ) { // Hacker!
      return NULL;
   }
   fileName.insert(0, m_htmlRoot->c_str()); // Put path first
   
   if (fileString[0] == '/') { // Ok rootdir first    
      stat(fileName.c_str(), status);
      
      /* mc2dbg4 << "getFile " <<
         fileName << " status: " << endl << 
             "Size=" << status->st_size << endl <<
	     "Modified=" << status->st_mtime << endl;*/
      length = fSize = status->st_size;
      file = open(fileName.c_str(), O_RDONLY);
      if (file == -1) { // Not found
         mc2dbg4 << "Unable to open file: " << fileName << endl;
         return NULL;
      }
      // Test type of file
      if ( ! (S_ISLNK(status->st_mode)  || S_ISREG(status->st_mode) || 
              S_ISFIFO(status->st_mode) || S_ISBLK(status->st_mode)) ) 
      {
         mc2dbg << "Not a permitted filetype: " << fileName << endl;
         if ( (S_ISLNK(status->st_mode)) ) {
            mc2dbg4 << "Is a LINK" << endl;
         }
         if ( S_ISREG(status->st_mode) ) {
            mc2dbg4 << "Is a REGULARFILE" << endl;
         } 
         if ( S_ISFIFO(file) ) {
            mc2dbg4 << "Is a FIFOFile" << endl;
         }
         if ( S_ISBLK(file) ) {
            mc2dbg4 << "Is a BLOCKDEVFILE" << endl;
         } 
         if ( S_ISDIR(status->st_mode) ) {
            mc2dbg4 << "Is a DIRECTORY" << endl;
         }
         if ( S_ISCHR(status->st_mode) ) {
            mc2dbg4 << "Is a Character device!" << endl;
         }
         if ( S_ISSOCK(status->st_mode) ) {
            mc2dbg4 << "Is a SOCKET!" << endl;
         }
         return NULL;
      }
      inBuff = new byte[fSize];
      if ( !Utility::read(file, inBuff, fSize) ) {
         DEBUG1(mc2log << error << "getFile failed to read all of: " <<
                fileString << endl);
         length = 0;
      }
      close(file);
   } else {
      DEBUG1(mc2log << warn << "getFile odd filename " <<
             fileString << endl;);
   }

   // INFO: HACK TO REMOVE ROUTEFILES, they are generated and are not on disc
   mc2dbg8 << "Trying file " << fileString << endl;
   if( ( fileString.find(".dat", pos) != MC2String::npos ) &&
       ( fileString.find("routefile", pos) != MC2String::npos ) ){
      mc2dbg8 << "Removing file " <<  fileName.c_str() <<endl;
      remove( fileName.c_str() );
   }
   
   return inBuff;
}


const char*
HttpFileHandler::getFileType(const MC2String& ext, byte* file) {
   const char* type;
   if ( (ext == "htm") || (ext == "html") )
      type = "text/html";
   else if (ext == "gif")
      type = "image/gif";
   else if ( (ext == "jpg") || (ext == "jpeg") )
      type = "image/jpeg";
   else if ( ext == "png" )
      type = "image/png";
   else if ( ext == "mng" )
      type = "image/mng";
   else if ( ext == "class" ) 
      type = "application/x-java-applet;version=1.1.6";
   else if ( ext == "jar" ){
      type = "application/java-archive";
   }
   else if ( ext == "wml" )   // WAP - Wireless Markup Langugage
      type = "text/vnd.wap.wml";
   else if ( ext == "wmls" )  // WAP - Wireless Markup Langugage Script
      type = "text/vnd.wap.wmlscript";
   else if ( ext == "wbmp" )  // WAP - Wireless BitMaP
      type = "image/vnd.wap.wbmp; type=0";
   else if ( ext == "wmlc" )  // WAP - Compiled WML-code
      type = "application/vnd.wap.wml";
   else if ( ext == "wmlsc" ) // WAP - Compiled WMLS-code
      if ( file != NULL ) {
         if ( file[0] == 0x00 ) // WAP 1.0
            type = "application/x-wap.wmlscriptc";
         else if ( file[0] == 0x01 ) // WAP 1.1
            type = "application/vnd.wap.wmlscriptc";
         else // Default WAP 1.1 and future?
            type = "application/vnd.wap.wmlscriptc"; 
      } else
         type = "application/vnd.wap.wmlscriptc";
   else if ( ext == "bin" || ext == "dat" ) // Binary content
      type = "application/octet-stream";
   else if ( ext == "css" ) // Cascading Style Sheets
      type = "text/css";
   else if ( ext == "xml" ) // Extensible Markup Language
      type = "text/xml";
   else
      type = "";

   return type;
}

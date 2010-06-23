/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FileLineReader.h"
#include "File.h"

FileLineReader::FileLineReader() {
}

FileLineReader::~FileLineReader() {
}

int
FileLineReader::readFile( const MC2String& file ) {
   int res = 0;

   // Remove any old data
   clear();

   vector<byte> buff;
   int nbrBytes = File::readFile( file.c_str(), buff );
   if ( nbrBytes > 0 ) {
      // Parse the bytes
      char c = '\0';
      size_t pos = 0;
      MC2String curLine;
      while ( pos < buff.size() ) {
         c = buff[ pos ];
         // A char
         if ( c == '\r' ) {
            // Ignore
         } else if ( c == '\n' ) {
            // End of line
            push_back( curLine );
            curLine.clear();
         } else {
            // A normal char
            curLine.push_back( c );
         }
         ++pos;
      }
      // New line last?
      if ( !curLine.empty() ) {
         mc2log << "FileLineReader no new line at end of file: " << file 
                << endl;
         res = -1;
      }
   } else {
      mc2log << "FileLineReader failed to read file: " << file << endl;
      res = -1;
   }

   return res;
}

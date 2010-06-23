/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FileUtils.h"

#include "FilePtr.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace FileUtils {

int32 getFileSize( const char* filename,
                   uint32& fileSize ) {
   struct stat status;
   int res = ::stat( filename, &status ); 
   if ( res == 0  ) {
      fileSize = status.st_size;
   }

   return res;
}

bool getFileContent( const char* filename, char*& content,
                     uint32* fileSizeReturn ) {
   // code originaly from XMLServer.cpp and NavigatorServer.cpp
   if ( filename == NULL ) {
      return false;
   }
   // Read file
   bool ok = false;

   uint32 fileSize;
   int res = getFileSize( filename, fileSize );
   if ( res == 0 ) {

      if ( fileSizeReturn != NULL ) {
         *fileSizeReturn = fileSize;
      }

      FilePtr f( fopen( filename, "rb" ) );
      if ( f.get() != NULL ) {

         uint32 buffLen = fileSize;

         byte* buff = new byte[ buffLen + 1 ];
         res = fread( buff, 1, buffLen, f.get() );

         if ( res == int32( buffLen ) ) {
            buff[ buffLen ] = '\0';
            buffLen += 1; // The extra terminator
            content = (char*)buff;
            ok = true;
         } else {
            delete[] buff;
            mc2log << warn
                   << "Failed to read all of file \"" << filename 
                   << "\" read " << res << " of " << buffLen << endl;
         }
      } else {
         mc2log << warn
                << "Failed to open file \"" << filename << "\" error: " 
                << strerror( errno ) << endl; 
      }
   } else {
      mc2log << warn
             << "Failed to stat file \"" << filename << "\" error: " 
             << strerror( errno ) << endl;
   }

   return ok;

}

}

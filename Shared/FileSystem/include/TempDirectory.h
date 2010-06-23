/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILEUTILS_TEMPDIRECTORY_H
#define FILEUTILS_TEMPDIRECTORY_H

#include "config.h"
#include "FileException.h"
#include "NotCopyable.h"

namespace FileUtils {
/**
 * Creates and destroys a temporary directory.
 */
class TempDirectory: private NotCopyable {
public:
   /**
    * Creates a temporary directory with directory name = pathXXXXXX/extraPath.
    * throws FileException on error
    * @param path the path that will be appended XXXXXX, where X 
    *        is a random char
    * @param extraPath the final path will have path/extraPath
    * 
    */
   explicit TempDirectory( const MC2String& path, 
                           const MC2String& extraPath = MC2String() ) 
      throw (FileException);

   ~TempDirectory();
   /// @return path name of the temporary directory
   const MC2String& getPath() const { return m_path; }

private:
   /// removes the directory
   void removeDirectory();

   MC2String m_path; //< the temporary path name
};

}
#endif

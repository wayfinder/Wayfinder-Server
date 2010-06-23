/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILEUTILS_DIRECTORY_H
#define FILEUTILS_DIRECTORY_H

#include "MC2String.h"
#include "NotCopyable.h"
#include "FileException.h"

#include <vector>

namespace FileUtils {

/**
 * Opens a handle to a directory.
 * TODO: add a scanDirectory function that takes a functor.
 *
 */
class Directory: private NotCopyable {
public:

   typedef std::vector<MC2String> Filenames;

   /// Filter interface for scan directory.
   class Filter {
   public:
      /**
       * Determine if a file should be included.
       * @param filename The name of the file to be tested.
       * @return true If the file should be included.
       */
      virtual bool shouldInclude( const char* filename ) const = 0;
   };

   /// @param directory The directory to open, will throw if fail to open.
   explicit Directory( const MC2String& directory ) throw ( FileException );
   Directory();
   ~Directory();

   /**
    * Scan directory for filenames and filter out files.
    * @param filenames Returns the set of files not filtered out.
    * @param filter Determines which files to include in the result.
    */
   void scan( Filenames& filenames, const Filter& filter ) const;

   /**
    * @return next filename in directory and advance file pointer
    *         to next filename.
    *         If filename is empty then directory reached
    *         end.
    */
   MC2String getNextFilename();

   /// Close the current directory.
   void close();

   /**
    * Open a new directory, this will close the current directory.
    * @param directory The name of the directory to open.
    * @return true on success and current directory name is changed, on failure
    *         it will return false and the currently opened directory will stay
    *         open.
    */
   bool open( const MC2String& directory );

   /// @return full directory name including last '/'.
   const MC2String& getName() const;

   /**
    * Rewind file pointer to begining of directory
    */
   void rewind();

private:
   class Impl;
   Impl* m_impl;
};

} // FileUtils

#endif // FILEUTILS_DIRECTORY_H

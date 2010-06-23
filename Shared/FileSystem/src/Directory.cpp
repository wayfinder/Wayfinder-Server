/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Directory.h"

#include <dirent.h>

namespace FileUtils {

class Directory::Impl {
public:
   Impl():m_dir( NULL ) {
   }
   inline Impl( const MC2String& directory );
   inline ~Impl();

   /**
    * Scan directory for filenames and filter out files.
    * @param filenames Returns the set of files not filtered out.
    */
   inline void scan( Filenames& filenames, const Filter& filter );

   /// @return next filename in directory and advance file pointer
   ///         to next filename.
   inline const char* getNextFilename();

   /**
    * Open a new directory, this will close the current directory.
    * @param directory The name of the directory to open.
    */
   inline bool open( const MC2String& directory );

   /// Close the directory handle.
   inline void close();

   inline void rewind();

   bool isValid() const {
      return m_dir != NULL;
   }

   const MC2String& getName() const {
      return m_name;
   }

   void setName( const MC2String& name ) {
      m_name = name;
      if ( m_name[ m_name.size() - 1 ] != '/' ) {
         m_name += '/';
      }
   }

private:
   /// @same as getNextFilename but without any checks
   const char* nextFilename();

   DIR* m_dir;
   MC2String m_name;
};


Directory::Directory( const MC2String& directory ) throw ( FileException ):
   m_impl( new Impl( directory ) ){

}

Directory::Directory():
   m_impl( new Impl() ) {
}

Directory::~Directory() {
   delete m_impl;
}

bool Directory::open( const MC2String& directory ) {
   return m_impl->open( directory );
}

void Directory::close() {
   m_impl->close();
}

MC2String Directory::getNextFilename() {
   const char* filename = m_impl->getNextFilename();
   if ( filename == NULL ) {
      return "";
   }
   return filename;
}

void Directory::rewind() {
   m_impl->rewind();
}

void Directory::scan( Filenames& filenames, const Filter& filter ) const {
   const_cast<Directory::Impl*>( m_impl )->scan( filenames, filter );
}

const MC2String& Directory::getName() const {
   return m_impl->getName();
}

//
// Implementation details begins here...
//

inline
Directory::Impl::Impl( const MC2String& directory ):
   m_dir( NULL ) {

   if ( ! open( directory ) ) {
      throw FileException( MC2String("Failed to open directory: \"" ) +
                           directory + "\"" );
   }
}

inline
Directory::Impl::~Impl() {
   close();
}

inline
bool Directory::Impl::open( const MC2String& directory ) {

   DIR* newDir = opendir( directory.c_str() );
   if ( newDir == NULL ) {
      // Failed to open new directory, keep old and
      // return failure.
      return false;
   }

   // close old directory and set new
   close();

   m_dir = newDir;
   setName( directory );

   return true;
}

inline
void Directory::Impl::close() {
   if ( isValid() ) {
      closedir( m_dir );
   }
}

inline
const char* Directory::Impl::getNextFilename() {
   if ( ! isValid()) {
      return NULL;
   }

   return nextFilename();
}
inline
const char* Directory::Impl::nextFilename() {
   struct dirent* entry = readdir( m_dir );
   if ( entry == NULL ) {
      return NULL;
   }

   // return valid name
   return entry->d_name;
}

inline
void Directory::Impl::scan( Filenames& filenames, const Filter& filter ) {
   if ( ! isValid() ) {
      return;
   }

   // save old directory position
   long pos = telldir( m_dir );

   const char* curFilename = NULL;
   while ( ( curFilename = nextFilename() ) != NULL ) {
      if ( filter.shouldInclude( curFilename ) ) {
         filenames.push_back( getName() + curFilename );
      }
   }

   // restore old directory position
   seekdir( m_dir, pos );
}


inline
void Directory::Impl::rewind() {
   if ( isValid() ) {
      rewinddir( m_dir );
   }
}

} // namespace FileUtils

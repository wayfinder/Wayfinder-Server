/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FOBC.h"
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include "File.h"
#include "STLStringUtility.h"
// stat includes

#ifndef WIN32
   // Assume unix-like
   #include <sys/types.h>
   #include <sys/stat.h>
#else
   #include <stat.h>
   // I think in WIN32
#endif

FOBC::FOBC() {
}


FOBC::~FOBC() {
   // Hmm, all FOBs have smart pointers so nothing to delete!
}


void
FOBC::clear() {
   m_files.clear();
}


aFOB
FOBC::innerGetFile( const MC2String& path, bool forceReRead ) {
   FOBMap::iterator fIt = m_files.find( path );
   if ( fIt != m_files.end() ) {
      if ( forceReRead ) {
         m_files.erase( fIt );
      } else {
         mc2dbg8 << "[FOBC]: Cached file Filename " 
                 << fIt->second->m_path << endl;
         return fIt->second;
      }
   }

   // Get file
   aFOB r;
   struct stat status;
   int res = lstat( path.c_str(), &status );
   if ( res == 0 ) {
      MC2String fName( path );
#ifdef __linux__
      const uint32 MAX_FILE_NAME_SIZE = 2048;
      char tmpFileName[ MAX_FILE_NAME_SIZE ];

      if ( S_ISLNK( status.st_mode ) ) {
         int lnkres = readlink( fName.c_str(), tmpFileName, 
                                MAX_FILE_NAME_SIZE );
         if ( lnkres > 0 ) {
            tmpFileName[ lnkres ] = 0;
            mc2dbg8 << "[FOBC]: " << fName << " links to "
                    << tmpFileName << endl;
            if ( tmpFileName[ 0 ] != '/' ) {
               MC2String dir = STLStringUtility::dirname( fName );
               fName = dir + "/" + tmpFileName;
            } else {
               fName = tmpFileName;
            }
            res = stat( fName.c_str(), &status );
            mc2dbg8 << "[FOBC]: New Filename " << fName << endl;
            // Check for this in cache
            FOBMap::iterator fIt = m_files.find( fName );
            if ( fIt != m_files.end() ) {
               if ( forceReRead ) {
                  m_files.erase( fIt );
               } else {
                  mc2dbg8 << "[FOBC]: Cached linked file Filename " 
                          << fIt->second->m_path << endl;
                  // Add link to cache
                  m_files.insert( make_pair( path, fIt->second ) );
                  return fIt->second;
               }
            }
         } else {
            mc2log << "[FOBC]: Failed to read link contents " 
                   << MC2CITE( fName ) << " res " << lnkres << " error: "
                   << strerror( errno ) << endl;
         }
      }
#endif

      if ( res == 0 ) {
         FOB* f = new FOB( fName );
         int fres = File::readFile( fName.c_str(), f->m_file );
         if ( fres < 0 ) {
            delete f;
         } else {
            mc2dbg8 << "[FOBC]: New file Filename " << fName << endl;
            r = (*m_files.insert( make_pair( fName, aFOB( f ) ) ).first).
               second;
            if ( fName != path ) {
               // Add link to cache too
               m_files.insert( make_pair( path, r ) );
            }
         }
      }
   } else {
      mc2dbg2 << "[FOBC] failed to stat file \"" << path 
              << "\" error: " << strerror( errno ) << endl;
   }

   return r;
}


#if 0
      /**
       * Removes a file from the cache.
       */
      void removeFile( const MC2String& path );


void
FOBC::removeFile( const MC2String& path ) {
   m_mutex.lock();
   FOBMap::iterator fIt = m_files.find( path );
   if ( fIt != m_files.end() ) {
      mc2dbg8 << "[FOBC]: Removing Cached file Filename " 
                  << fIt->second->m_path << " for path " << path << endl;
      m_files.erase( fIt );
   }
   m_mutex.unlock();
}
#endif


#if 0
      /**
       * Cache a directory's contents. Only one level.
       */
      void cacheDir( const MC2String& path );

#include <dirent.h>

void
FOBC::cacheDir( const MC2String& path ) {
   DIR* d = opendir( path.c_str() );
   if ( d != NULL ) {
      struct dirent* f = readdir( d );
      while ( f != NULL ) {
         getFile( path + "/" + f->d_name );
         f = readdir( d );
      }

      closedir( d );
   } else {
      mc2log << warn << "[FOBC] failed to open dir \"" << path 
             << "\" error: " << strerror( errno ) << endl;
   }
}
#endif

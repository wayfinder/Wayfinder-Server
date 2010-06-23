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

#include "File.h"

#include "MC2String.h"
#include <sys/stat.h>
#include <sys/types.h>


// Initiate the errorcodes
const int File::END_OF_FILE = -1;
const int File::FILE_ERROR = -2;

File::File(const char* filename)
{
   if ( filename != NULL ) {
      m_filename = new char[strlen(filename)+1];
      strcpy(m_filename, filename);
   } else {
      m_filename = NULL;
   }
   m_file = NULL;
}

File::~File()
{
   delete [] m_filename;
   close();
}

bool
File::openProgram(const MC2String& program, const char* mode)
{
   m_file = popen(program.c_str(), mode);
   return m_file;
}


bool
File::open(const char* mode)
{
   if (m_file != NULL) {
      fclose(m_file);
   }
   
   m_file = fopen(m_filename, mode);

   if ( strchr(mode, 'w' ) == NULL &&
        strchr(mode, 'a' ) == NULL &&
        strchr(mode, '+' ) == NULL ) {
      // Only do this for reading.
      if ( m_file == NULL ) {
         mc2dbg << "[File]: Open "
                << MC2CITE(m_filename) << " failed - trying gunzip"
                << endl;             
         openProgram(MC2String("gunzip -c ") + m_filename + ".gz", "r");
      }
      
      if ( m_file == NULL ) {
         mc2dbg << "[File]: Open "
                << MC2CITE(m_filename) << " failed - trying bunzip"
                << endl;             
         openProgram(MC2String("bunzip2 -c ") + m_filename + ".bz2", "r");
      }
   }
   
   return (m_file != NULL);
}


bool
File::close()
{
   if (m_file != NULL) {
      int retVal = fclose(m_file);
      m_file = NULL;
      return (retVal == 0);
   }
   return (true);
}

bool
File::setPos( size_t pos ) {
   return fseek( m_file, pos, SEEK_SET ) == 0;
}

bool
File::read( vector<byte>& buff, size_t size ) {
   size_t startSize = buff.size();
   buff.resize( startSize + size );
   size_t res = fread( &buff.front() + startSize, 1, size, m_file );

   return res == size;
}

bool File::fileExist( const MC2String& filename ) { 
   struct stat fStat;
   if ( stat( filename.c_str(), &fStat ) != 0 ) {
      return false;
   } 

   return  (fStat.st_mode & ( S_IFLNK | S_IFREG ) );
}

int File::mkdir_p( const MC2String& inDir )
{
   static const mode_t dirMode = S_IRUSR | S_IWUSR | S_IXUSR | 
                                 S_IRGRP | S_IXGRP |
                                 S_IROTH | S_IXOTH;
   
   int res = mkdir( inDir.c_str(), dirMode );
   
   if ( res != 0 ) {
      if ( errno != EEXIST ) {
         MC2String dir( inDir );
         int pos = dir.length() -1;
         while ( pos > 0 ) {
            if ( dir[ pos ] == '/' ) {
               dir[ pos ] = '\0';
               break;
            }
            pos--;
         }
         if ( pos == 0 ) {
            res = -1;
         } else {
            res = mkdir_p( dir );
         }
         if ( res == 0 ) {
            res = mkdir( inDir.c_str(), dirMode );
         }
      } else {
         res = 0;
      }
   }
   return res;
}


int 
File::readFile( const char* filename, vector<byte>& buff ) {
   struct stat status;
   int res = stat( filename, &status );
   if ( res == 0 ) {
      FILE* f = fopen( filename, "rb" );
      if ( f != NULL ) {
         uint32 buffLen = status.st_size;
         uint32 nbrRead = 0;
         byte rb[4096];
         do {
            res = fread( rb, 1, 4096, f );
            if ( res > 0 ) {
               buff.insert( buff.end(), rb, rb + res );
               nbrRead += res;
            }
         } while ( res > 0 && nbrRead < buffLen );
         fclose( f );
         if ( nbrRead == buffLen ) {
            res = nbrRead;
         } else {
           mc2log << warn << "File::readFile failed to read "
                  << "all of file \"" << filename << "\" read " << nbrRead
                  << " of " << buffLen << endl;
           res = -1;
         }
      } else {
         res = -1;
         mc2log << warn << "File::readFile failed to open "
                << "file \"" << filename << "\" error: " 
                << strerror( errno ) << endl; 
      }
   } else {
      mc2log << warn << "File::readFile failed to stat "
             << "file \"" << filename << "\" error: " 
             << strerror( errno ) << endl;
   }

   return res;
}

int 
File::writeFile( const char* filename, const byte* buff, uint32 buffLen ) {
   FILE* f = fopen( filename, "wb" );
   int res = -1;
   if ( f != NULL ) {
      uint32 nbrWritten = 0;
      do {
         res = fwrite( buff + nbrWritten, 1, 
                       buffLen - nbrWritten, f );
         if ( res > 0 ) {
            nbrWritten += res;
         }
      } while ( res > 0 && nbrWritten < buffLen );
      fclose( f );
      if ( nbrWritten == buffLen ) {
         res = nbrWritten;
      } else {
         mc2log << warn << "File::writeFile failed to write "
                << "all of file \"" << filename << "\" written " 
                << nbrWritten << " of " << buffLen << endl;
         res = -1;
      }
   } else {
      res = -1;
      mc2log << warn << "File::writeFile failed to open "
             << "file \"" << filename << "\" error: " 
             << strerror( errno ) << endl; 
   }

   return res;
}

int 
File::writeFile( const char* filename, const vector<byte>& buff ) {
   return writeFile( filename, &buff.front(), buff.size() );
}

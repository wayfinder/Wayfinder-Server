/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FileDBufRequester.h"

#include "MC2SimpleString.h"
#include "BitBuffer.h"

#include <stdio.h>

#ifndef __SYMBIAN32__
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_LENGTH 1
#else
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_LENGTH 1
#endif

#ifdef MC2_SYSTEM
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#   include "File.h"
#   include "MC2String.h"
#endif

FileDBufRequester::FileDBufRequester(DBufRequester* parent,
                                     const char* path)
      : DBufRequester(parent),
        m_path(path)
{

}


FileDBufRequester::~FileDBufRequester()
{
}

// Some kind of unix

MC2SimpleString
FileDBufRequester::getFileName(const MC2SimpleString& descr)
{
   char* fullFileName = new char[m_path.length() + descr.length() +
#ifdef MC2_SYSTEM
                                 100 +
#endif
                                PATH_SEPARATOR_LENGTH + 3];
   strcpy(fullFileName, m_path.c_str());
   strcat(fullFileName, PATH_SEPARATOR);
#ifdef MC2_SYSTEM
   // Separate into dirs
   if ( descr.length() >= 1 ) { 
      // First use the first letter as top dir.
      char topDir = descr[ 0 ];
      strncat( fullFileName, &topDir, 1 );
      strcat( fullFileName, PATH_SEPARATOR );
      char dir[ 50 ];
      // Make hash
      uint32 hashet = 0;
      int len = descr.length();
      static const int nbrRotate = 8;
      for ( int i = 0; i < len; ++i ) {
         hashet = ( hashet >> (32 - nbrRotate) ) | (hashet << nbrRotate);
         hashet ^= descr[i];
      }
      // Then use ental for digit one
      static const uint32 divisor = 10;
      uint32 intA = hashet % divisor;
      hashet /= divisor;
      uint32 intB = hashet % divisor;
      hashet /= divisor;
      uint32 intC = hashet % divisor;
      hashet /= divisor;
      
      sprintf( dir, "%u%s%u%s%u", intA, PATH_SEPARATOR, 
               intB, PATH_SEPARATOR, intC );
      strcat( fullFileName, dir );
      strcat( fullFileName, PATH_SEPARATOR );
   }
#endif
   strcat(fullFileName, descr.c_str());
   MC2SimpleString retVal(fullFileName);
   delete [] fullFileName;
   return retVal;
}

BitBuffer*
FileDBufRequester::readFromFile(const MC2SimpleString& descr)
{
   MC2SimpleString fullFileName(getFileName(descr));
   FILE* file = fopen(fullFileName.c_str(), "r");

   if ( file == NULL ) {
      // No file yet.
      return NULL;
   }

   // File is opened.
   BitBuffer* retBuf = NULL;
   
   // Seek to the end of file
   int seekRes = fseek(file, 0, SEEK_END);
   if ( seekRes < 0 ) {
      fclose(file);
      return NULL;
   }
   
   long fileSize = ftell(file);
   if ( fileSize <= 0 ) {
      mc2dbg << "[FDBR]: File size is " << fileSize << endl;
   }
   // Set back the position.
   seekRes = fseek(file, 0, SEEK_SET);
   retBuf = new BitBuffer(fileSize);
   // FIXME: How about some error checks?
   if ( long (
      fread(retBuf->getBufferAddress(), 1, fileSize, file) ) != fileSize ) {
      mc2dbg << "[FDBR]: Failed to read file " << MC2CITE( fullFileName )
             << endl;
      delete retBuf;
      retBuf = NULL;
   }
   fclose(file);
   
   return retBuf;
}

bool
FileDBufRequester::writeToFile(const MC2SimpleString& descr,
                               BitBuffer* buffer)
{
   MC2SimpleString fullFileName(getFileName(descr));

   {
      if ( buffer->getBufferSize() != 0 ) {
         FILE* readFile = fopen(fullFileName.c_str(), "r");
         if ( readFile != NULL ) {
            // Already there
            mc2dbg8 << "[FDBR]: " << descr << " already on disk" << endl;
            fclose(readFile);
            return true;
         }
      } else {
         // Buffer size is zero. Remove the file
         mc2dbg << "[FDBR]: Size is zero - removing file "
                << MC2CITE( fullFileName ) << endl;
         return unlink( fullFileName.c_str() ) != -1;
      }
   }
#if defined (__unix__) || defined (__MACH__)
#define USE_TEMP_FILE
#endif
   
#ifdef MC2_SYSTEM
   // Make sure that the dir exists
   MC2String dir( fullFileName.c_str() );
   // Terminate at last PATH_SEPARATOR
   int pos = dir.length() -1;
   while ( pos > 0 ) {
      if ( dir[ pos ] == PATH_SEPARATOR[ 0 ] ) {
         dir[ pos ] = '\0';
         break;
      }
      pos--;
   }
   errno = 0;
   if ( File::mkdir_p( dir.c_str() ) != 0 ) {
      if ( errno != EEXIST ) {
         mc2log << warn << "Failed to create dir: " << dir << " file "
                << fullFileName << endl;
         perror( "Last system error" );
         return false;
      }
   }
#endif

#ifdef USE_TEMP_FILE
   // Make temporary file in the same dir. (For e.g. server
   // to avoid two threads writing to the same file).
   char tempTemplate[1024];   
   sprintf( tempTemplate, "%s%stilecachXXXXXX", m_path.c_str(), 
            PATH_SEPARATOR );
   int tmpDesc = mkstemp(tempTemplate);

   if ( tmpDesc == -1 ) {
      mc2dbg << "[FDBR]: Failed creating temp file. Syserr: " 
             << strerror(errno) << endl;
      return false;
   }

   MC2SimpleString tempName(tempTemplate);
   FILE* file = fdopen(tmpDesc, "w");
#else
   // Write directly to the file
   FILE* file = fopen(fullFileName.c_str(), "w");
#endif
      
   if ( file && (fwrite(buffer->getBufferAddress(),
                        1, buffer->getBufferSize(), file) ==
                 buffer->getBufferSize() ) ) {
      mc2dbg8 << "[FDBR]: Wrote " << descr << " to disk" << endl;
      // Important to close the file and thereby flushing it
      // before renaming it.
      int closeRes = fclose(file);
      if ( closeRes != 0 ) {
         mc2log << "[FDBR]: Close failed" << endl;
      }
      file = NULL;
#ifdef USE_TEMP_FILE
      // Rename the file to the correct name and hope it works.
      if ( closeRes == 0 ) {
         rename(tempName.c_str(), fullFileName.c_str());
      } else {
         mc2dbg << "[FDBR]: Failed closing " << tempName << " ("
                << descr << ") Syserr: " << strerror(errno) << endl;
         unlink( tempName.c_str() );
      }
#endif
      return true;
   } else {
      mc2dbg << "[FDBR]: Failed writing " << tempName << " ("
             << descr << ") Syserr: " << strerror(errno) << endl;
      unlink( tempName.c_str() );
   }
   if ( file ) {
      fclose(file);
   }
   return false;
}

BitBuffer*
FileDBufRequester::requestCached(const MC2SimpleString& descr)
{
   return readFromFile(descr);
}


void
FileDBufRequester::release(const MC2SimpleString& descr,
                           BitBuffer* buffer )
{
   if ( buffer != NULL ) {
      writeToFile(descr, buffer);
   }  
   // Since we create all our buffers from file we can
   // send up the stuff to the parent.
   DBufRequester::release(descr, buffer);
}

void
FileDBufRequester::releaseCached( const MC2SimpleString& descr,
                                  BitBuffer* buffer )
{
   // Since we create all our buffers from file we can
   // send up the stuff to the parent.
   DBufRequester::releaseCached(descr, buffer);
}

void
FileDBufRequester::internalRemove( const MC2SimpleString& descr ) {
   // Remove the Tile completely
   MC2SimpleString fullFileName(getFileName(descr));

   // Do we care about result?
   int res = unlink( fullFileName.c_str() );
   if ( res == 0 ) {
      mc2dbg << "[FDBR]: Removed " << MC2CITE( descr ) << endl;
   }
}


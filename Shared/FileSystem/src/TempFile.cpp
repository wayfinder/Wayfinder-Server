/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TempFile.h"

#include "File.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

namespace {

MC2String addSlash( const MC2String& path ) {
   if ( ! path.empty() &&
        path[ path.size() - 1 ] != '/' ) {
      return path + '/';
   }

   return path;
}

void createPath(const MC2String& path ) {
   if ( path.empty() )
      return;

   if ( File::mkdir_p( path ) == -1 ) {
      mc2dbg << "[TempFile]: Failed to make directory " 
             << MC2CITE( path ) 
             << ". Error: " << strerror( errno ) << endl;
   }
}

}

TempFile::TempFile( const MC2String& tempPrefixFilename,
                    const MC2String& tempPath,
                    const MC2String& realFilename,
                    const MC2String& realPath ):
   m_realFilename( addSlash( realPath ) + realFilename ),
   m_fd( -1 ),
   m_failed( false )
{
   createPath( addSlash( tempPath ) );
   createPath( addSlash( realPath ) );

   m_tempFilename = addSlash( tempPath ) + tempPrefixFilename + "XXXXXX";

   m_fd = mkstemp( &m_tempFilename[0] );
   if ( m_fd == -1 ) {
      mc2dbg << "[TempFile]: Failed to create tempfile "
             << MC2CITE( m_tempFilename ) << ". " 
             << "error: " << strerror( errno ) << endl;
   }
} 

TempFile::~TempFile() 
{
   if ( !ok() )
      return;

   syncAndClose();


   // if the real filename is empty we should just unlink it

   if ( ! m_failed && ! m_realFilename.empty() ) {
      
      mc2dbg << "[TempFile]: renaming " 
             << MC2CITE( m_tempFilename ) 
             << " to " 
             << MC2CITE( m_realFilename ) 
             << endl;
            

      if ( rename( m_tempFilename.c_str(), 
                   m_realFilename.c_str() ) == -1 ) {

         mc2dbg << "[TempFile]: rename from " 
                << MC2CITE( m_tempFilename ) 
                << " to " 
                << MC2CITE( m_realFilename ) 
                << " failed. error: " 
                <<  strerror( errno ) << endl;
      }
   } else if ( unlink() == -1 ) {
      mc2dbg << "[TempFile]: unlink of " 
             << MC2CITE( m_tempFilename ) 
             << " failed. error: " 
             << strerror( errno ) << endl;
   }

}

uint32 TempFile::getCurrentOffset() const 
{
   int ret = lseek( getFD(), 0, SEEK_CUR );
   if ( ret == -1 ) {
      mc2dbg << "[TempFile]: getOffset() error: " << strerror( errno ) << endl;
      return 0;
   }
   return (uint32)ret;
}

uint32 TempFile::getSize() const 
{
   uint32 oldPos = getCurrentOffset();
   int lastPos = lseek( getFD(), 0, SEEK_END );
   if ( lastPos == -1 ) {
      mc2dbg << "[TempFile]: getSize() error: " << strerror( errno ) << endl;
      return 0;
   }
   // restore old position
   if ( lseek( getFD(), oldPos, SEEK_SET ) == -1 ) {
      mc2dbg << "[TempFile]: getSize() failed to restore old position. " 
             << endl;
      mc2dbg << "[TempFile]: error: " << strerror( errno ) << endl;
   }

   return (uint32)lastPos;
}

int TempFile::write( const char* buff, uint32 size ) 
{
   return ::write( getFD(), buff, size );
}


int TempFile::read( char* buff, uint32 size ) 
{
   return ::read( getFD(), buff, size );
}

int TempFile::unlink() {

   syncAndClose();

   m_failed = false;

   return ::unlink( m_tempFilename.c_str() );
}

int TempFile::seek( int offset, int whence ) {
   return ::lseek( getFD(), offset, whence );
}

int TempFile::sync() {
   return ::fsync( getFD() );
}

int TempFile::close() {
   return ::close( getFD() );
}

void TempFile::syncAndClose() {
   if ( ! ok() ) {
      return;
   }

   // sync before close
   if ( sync() == -1 ) {
      mc2dbg << "[TempFile]: Failed to sync to disc. " 
             << "error: " << strerror( errno ) << endl;
   }

   if (  close( ) == -1 ) {
      mc2dbg << "[TempFile]: Failed to close file. " 
             << "error: " << strerror( errno ) << endl;

   } else {
      // only set this if close was successfull,
      // we can try to close it later
      m_fd = -1;
   }

}

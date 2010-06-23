/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef _WIN32
#error IMPLEMENT pipe for win32!.
#endif

#include "Pipe.h"
#include "Selectable.h"
#include "FileException.h"

#include <unistd.h>
#include <fcntl.h>

class SelectableFD: public Selectable { 
public:
   explicit SelectableFD( int fd = 0):m_fd( fd ) { }
   virtual ~SelectableFD() { 
      if ( m_fd != 0 && m_fd != -1 ) {
         close( m_fd ); 
      }
   }

   void setFD( int fd ) { m_fd = fd; }
   selectable getSelectable() const { return m_fd; }
private:
   int m_fd;
};

namespace {
MC2String getSystemError( int errValue ) {
   char buff[ 1024 ];
   memset( buff, 0, 1024 );
   strerror_r( errValue, buff, 1024 );
   return buff;

}
}

Pipe::Pipe() throw (FileUtils::FileException) {
   int pipefd[2];
   if ( pipe( pipefd ) == -1 ) {
      MC2String error( "Pipe: can not create pipe. " );
      error += ::getSystemError( errno );
      throw FileUtils::FileException( error );
   }
   
   m_fd[0].reset( new SelectableFD( pipefd[ 0 ] ) );
   m_fd[1].reset( new SelectableFD( pipefd[ 1 ] ) );
   
   for ( int i=0; i < 2; ++i ) {
      int res = fcntl( pipefd[ i ], F_SETFL, O_NONBLOCK );
      if ( res < 0 ) {
         MC2String error( "Pipe: could not set nonblock. " );
         error += ::getSystemError( errno );
         throw FileUtils::FileException( error );
      }
   }
}

Pipe::~Pipe() {
}

int Pipe::read( const char* buff, size_t count ) 
{
   return ::read( m_fd[ 0 ]->getSelectable(),
                  (void*)buff, count );
}

int Pipe::write( const char* buff, size_t count )
{
   return ::write( m_fd[ 1 ]->getSelectable(),
                   (void*)buff, count );
}

const Selectable& Pipe::getWriteSelectable() const
{
   return *m_fd[ 1 ];
}

const Selectable& Pipe::getReadSelectable() const
{
   return *m_fd[ 0 ];
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "URL.h"
#include "StringUtility.h"
#include "NewStrDup.h"

#include "FixedSizeString.h"

URL::URL(const char* spec)
{
   createFromString( spec );
}

URL::URL( const MC2String& spec )
{
   createFromString( spec.c_str() );
}

URL::URL( const URL& other ) : m_hostname( NULL ),
                               m_proto( NULL ),
                               m_path( NULL ),
                               m_file( NULL ),
                               m_spec( NULL )
{
   *this = other;
}

#define DUP(x) ( (x) ? NewStrDup::newStrDup(x) : (x) )

URL&
URL::operator=( const URL& other )
{
   if ( this == &other ) {
      return *this;
   }
   deleteData();
   m_proto    = DUP( other.m_proto );
   m_hostname = DUP( other.m_hostname );
   m_path     = DUP( other.m_path );
   m_file     = DUP( other.m_file );
   m_spec     = DUP( other.m_spec );
   m_port     = other.m_port;
   m_valid    = other.m_valid;
   return *this;
}

URL::~URL()
{
   deleteData();
}

void
URL::deleteData()
{
   delete [] m_proto;
   delete [] m_hostname;
   delete [] m_path;
   delete [] m_file;
   delete [] m_spec;
}

void
URL::createFromString( const char* spec )
{
   // deleteData? To be safe?
   m_hostname = NULL;
   m_proto    = NULL;
   m_path     = NULL;
   m_file     = NULL;
   m_spec     = NULL;
   m_port = -1;
   m_valid = true; // Hope for the best.

   if ( spec == NULL || strlen(spec) == 0 ) {
      m_valid = false;
      return;
   }

   // ok, lets check for file protocol first and then tokenize the string.
   const uint32 FILE_PROT_STR_LEN = strlen( "file://");
   if ( strncmp( spec, "file://", FILE_PROT_STR_LEN ) == 0 ) {
      m_proto = StringUtility::newStrDup( "file" );
      m_hostname = StringUtility::newStrDup("");
      // copy everything after "file://"
      m_path = StringUtility::newStrDup( spec + FILE_PROT_STR_LEN );
      m_file = createFileFromPath( m_path );
      createSpec();
      return;
   }

   // Parse the string. Yuck.
   char* urlCopy = StringUtility::newStrDup(spec);

   vector< char* > tokenized;
   StringUtility::tokenListToVector( tokenized,
                                     urlCopy,
                                     '/',
                                     false );

   const uint32 pathStart = 3;
   if ( tokenized.size() < pathStart ) {
      m_valid = false;
   } else {
      // Find the protocol. It should be in the first element.
      m_proto = StringUtility::newStrDup( tokenized[ 0 ] );
      if ( strlen(m_proto) > 0 )
         m_proto[strlen(m_proto) - 1] = 0; // Remove the ":"

      // The hostname and maybe port
      m_hostname = StringUtility::newStrDup( tokenized[ 2 ] );

      char* colon = strchr(m_hostname, ':');
      if ( colon != NULL ) {
         // There is a ":", i.e. a port
         m_port = atoi(colon + 1);
         *colon = '\0'; // Terminate the hostname
      }

      int pathSize = 100;
      // The path.
      uint32 i;
      for(i=pathStart; i < tokenized.size(); i++) {
         pathSize += strlen( tokenized[ i ] ) + 2;
      }
      char* tmpPath = new char[pathSize];
      tmpPath[0] = '\0';
      // The path will always start with "/". If we haven't entered
      // the for loop we will set the first character to "/" anyway,
      // so we add another terminating zero.
      tmpPath[1] = '\0';
      for(i=pathStart; i < tokenized.size(); i++) {
         strcat(tmpPath, "/");
         strcat(tmpPath, tokenized[ i ]);
      }
      // Set the path to at least "/"
      tmpPath[0] = '/';
      m_path = StringUtility::newStrDup( tmpPath );
      delete [] tmpPath;

      m_file = createFileFromPath( m_path );
   }

   bool fileProt = m_valid && strcmp( m_proto, "file" ) == 0;

   if ( m_port < 1 && m_valid &&
        ! fileProt ) {
      // Find out the port from the protocol.
      if ( strcmp(m_proto, "http") == 0 ) {
         m_port = 80;
      } else if ( strcmp(m_proto, "https") == 0 ) {
         m_port = 443;
      } else {
         // Unknown protocol
         const char* errorText = "Unknown protocol in URL: ";
         char* completeError =
           new char[strlen(errorText) + 1 + strlen(m_proto) + 100];
         sprintf(completeError, "%s%s", errorText, m_proto);
         mc2log << error << completeError << endl;
         m_valid = false;
         delete [] completeError;
      }
   }

   if ( ! fileProt &&
        ( m_hostname == NULL || strlen(m_hostname) == 0 ) ) {
      m_valid = false;
   }

   // Check the port. (IPv4)
   if ( ! fileProt && m_port > 65535 ) {
      m_valid = false;
   }

   delete [] urlCopy;
   createSpec(); // Fill in the string form of the URL
}

URL::URL(const char* protocol,
         const char* hostname,
         int port,
         const char* path)
{
   if ( protocol != NULL && hostname != NULL && path != NULL &&
        port > 0 ) {
      m_proto = StringUtility::newStrDup(protocol);
      m_hostname = StringUtility::newStrDup(hostname);
      m_port = port;
      m_path = StringUtility::newStrDup(path);
      m_file = createFileFromPath( path );
      m_valid = true;
   } else {
      m_proto = NULL;
      m_hostname = NULL;
      m_port = -1;
      m_path = NULL;
      m_file =  NULL;
      m_valid = false;
   }
   createSpec();
}

void
URL::createSpec()
{
   if ( m_valid ) {
      // Create temporary space for the spec
      // Too much really, but we won't use it for very long.
      const uint32 STRING_SIZE =
         strlen(m_proto)*2 + 1 +
         strlen(m_hostname)*2 + 1 +
         strlen(m_path) + 1024;

      FixedSizeString tmpSpec( STRING_SIZE );

      if ( strcmp( m_proto, "file" ) == 0 ) {
         sprintf( tmpSpec, "file://%s", m_path );
      } else {
         sprintf( tmpSpec, "%s://%s:%d%s",
                  m_proto, m_hostname, m_port, m_path);
      }
      m_spec = StringUtility::newStrDup( tmpSpec.c_str() );
   } else {
      m_spec = StringUtility::newStrDup("INVALID URL");
   }
}

char*
URL::createFileFromPath( const char* path ) {
   // Make file from path from first slash and before first ?
   const char* startPos = strchr( path, '/' );
   if ( startPos == NULL ) {
      startPos = path;
   }
   const char* endPos   = strchr( startPos, '?' );
   if ( endPos == NULL ) {
      endPos            = strchr( startPos, '#' );
   }
   if ( endPos == NULL ) {
      endPos = startPos + strlen( startPos ) + 1;
   }
   const uint32 fileLen = endPos - startPos;
   char* file = new char[ fileLen + 1 ];
   strncpy( file, startPos, fileLen );
   file[ fileLen ] = '\0';

   return file;
}

uint32
URL::getSizeAsBytes() const {
   if ( m_spec != NULL ) {
      return strlen( m_spec ) + 1;
   } else {
      return 0;
   }
}

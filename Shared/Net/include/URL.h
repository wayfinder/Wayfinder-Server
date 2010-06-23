/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef URL_H
#define URL_H

#include "config.h"
#include "MC2String.h"

/**
 *   Class encapsulating an URL.
 *   Mostly for translating a string into
 *   protocol/port/file.
 *   Handles file://, http:// and https:// protocols.
 *   Needs Ref - anchor handling.
 *         Method to get path without all but directories and
 *         method to get query part.
 *
 */
class URL {

public:

   /**
    *   Copy-constructor
    */
   URL( const URL& other );

   /**
    *   Assignment operator!
    */
   URL& operator=( const URL& other );
   
   /**
    *   Creates a new URL with the spec.
    *   @param spec The URL as a proper string.
    */
   URL(const char* spec);

   /**
    *   Creates a new URL with the spec.
    *   @param spec The URL as a proper string.
    */
   URL( const MC2String& spec );

   /**
    *   Creates a new URL with the provided data.
    *   @param protocol The protocol to use (eg. http).
    *   @param hostname The hostname for the URL.
    *   @param port     The port to connect to.
    *   @param path     The path.
    */
   URL(const char* protocol,
       const char* hostname,
       int port,
       const char* path );

   /**
    *   The destructor.
    */
   ~URL();
   
   /**
    *   @return The protocol for the URL.
    */
   inline const char* getProto() const;

   /**
    *   @return The hostname part of the URL.
    */
   inline const char* getHost() const;

   /**
    *   @return The port of the URL.
    */
   inline int getPort() const;

   /**
    *   @return Everything after "proto://host:port".
    */
   inline const char* getPath() const;

   /**
    *   @return The file in url, may be empty,  
    *   "proto://host:port/dir/dir2/file.ext?g=2" gives /dir/dir2/file.ext.
    */
   inline const char* getFile() const;

   /**
    *   @return The text format of the URL.
    */
   inline const char* getSpec() const;
   
   /**
    *   Returns true if the URL is valid. One or many of the
    *   other get-methods can return null if valid is false.
    *   @return True if the URL is valid.
    */
   inline bool isValid() const;

   /**
    * The size when saved into a Packet.
    */
   uint32 getSizeAsBytes() const;
   
private:

   /**
    *   Called by destructor and operator=.
    */
   void deleteData();
   
   /**
    *   Inits the url from a string.
    */
   void createFromString( const char* str );
   
   /**
    *   Creates the text version of the URL.
    */
   void createSpec();

   char* createFileFromPath( const char* path );
   
   /** The hostname */
   char* m_hostname;

   /** The protocol */
   char* m_proto;

   /** The path, including all */
   char* m_path;

   /**
    * The file of the url.
    */
   char* m_file;

   /** The port */
   int m_port;

   /** True if the URL is valid */
   bool m_valid;

   /** The URL as a string */
   char* m_spec;
};

inline ostream& operator<<( ostream& o, const URL& url )
{
   return o << url.getSpec() << endl;
}

inline const char*
URL::getProto() const
{
   return m_proto;
}

inline const char*
URL::getHost() const
{
   return m_hostname;
}

inline int
URL::getPort() const
{
   return m_port;
}

inline const char*
URL::getPath() const
{
   return m_path;
}

inline const char*
URL::getFile() const {
   return m_file;
}

inline bool
URL::isValid() const
{
   return m_valid;
}

inline const char*
URL::getSpec() const
{
   return m_spec;
}

#endif

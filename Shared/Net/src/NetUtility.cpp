/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NetUtility.h"
#include "ScopedArray.h"

#include "sockets.h"
#include "SimpleArray.h"

#include <cstdio>

namespace NetUtility {

bool gethostbyname( const char* hostname, 
                    struct in_addr& sin_addr ) {
    SimpleArray<char> hostAuxData( 1024 );
    struct hostent hostbuf;
    struct hostent* hostEntry = NULL;
    int herr = 0;
    if ( ::gethostbyname_r( hostname,
                            &hostbuf, 
                            hostAuxData.data(), hostAuxData.size(),
                            &hostEntry,
                            &herr ) == 0  && hostEntry != NULL) {
       memcpy(&sin_addr, hostEntry->h_addr, hostEntry->h_length);
       return true;
    } 

    // Note: hstrerror is obsolete and might not work on some systems.
    // and there is no alternative
    mc2log << error 
           << "[NetUtility::gethostbyname] " << hstrerror( herr ) << endl;
    return false;
}

MC2String getHostName( uint32 IP, bool dnsLookup ) 
{
   ScopedArray<char> name( new char[1024] );
   struct in_addr address; // Unneccesary really. uint32 would do.
   struct hostent* he = NULL;
   address.s_addr = htonl(IP);
   if ( dnsLookup && 
        (he = gethostbyaddr( (const char *)(&address.s_addr), 
                             sizeof(uint32), AF_INET)) != NULL )
   {
      strcpy( name.get(), he->h_name );
   } else {
      sprintf( name.get(), "%u.%u.%u.%u", 
               uint32((IP & 0xff000000) >> 24),
               uint32((IP & 0x00ff0000) >> 16), 
               uint32((IP & 0x0000ff00) >>  8), 
               uint32(IP & 0x000000ff) );
   }
   return name.get();
}

MC2String getLocalHostName() 
{
   return getHostName( getLocalIP() );
}

uint32 getLocalIP() 
{
   static bool inited = false;
   static uint32 result;
   if ( ! inited ) { // Only call these complicated functions once.
#ifndef SINGLE_VERSION
      char hostName[ MAXHOSTNAMELEN + 1];
      if ( gethostname( hostName, MAXHOSTNAMELEN ) == 0 ) {
         struct in_addr sin_addr;
         if ( NetUtility::gethostbyname( hostName, sin_addr ) ) {
            result = ntohl(sin_addr.s_addr);
            inited = true;
         } else {
            result = 2130706433; // 127.0.0.1
            MC2ERROR(
              "Utility::getLocalIP gethostbyname() failed, using 127.0.0.1" );
         }
      } else {
         result = 2130706433; // 127.0.0.1
         PERROR;
         MC2ERROR(
           "Utility::getLocalIP gethostname() failed, using 127.0.0.1" );
      }
#else 
      result = 2130706433; // 127.0.0.1
      inited = true;
#endif

   }
   return result;
}

MC2String ip2str( uint32 IP ) {
   char str[ 256 ];
   snprintf( str, 256, "%u.%u.%u.%u",
             int((IP & 0xff000000) >> 24),
             int((IP & 0x00ff0000) >> 16),
             int((IP & 0x0000ff00) >>  8),
             int (IP & 0x000000ff) );
   return str;
}

uint32 iptoh( const char *ip_address ) {
   struct sockaddr_in sin;

   memset( &sin, 0, sizeof( sin ) );
   sin.sin_family = AF_INET;

   if ( ! gethostbyname( ip_address, sin.sin_addr ) ) {
      sin.sin_addr.s_addr = inet_addr( ip_address );
   }

   return ntohl( sin.sin_addr.s_addr );
}

uint32 aton( const MC2String& ipWithDots ) {
   struct in_addr inp;
   if ( inet_aton( ipWithDots.c_str(), &inp ) != 0 ) {
      return ntohl( inp.s_addr );
   }
   return MAX_UINT32;
}

}

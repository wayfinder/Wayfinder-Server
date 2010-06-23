/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NETUTILITY_H
#define NETUTILITY_H

#include "config.h"
#include "MC2String.h"

struct in_addr;
/**
 * Contains useful utilities for network stuff
 */
namespace NetUtility {

/// Maximum number of tries when iterating for finding free port.
const uint32 MAX_NBR_TRIES_PORTFIND = 100;

/**
 * reentrant version of libc gethostbyname (using gethostbyname_r).
 * @param hostname the hostname to lookup
 * @param sin_addr will be filled in with hostname
 * @return true on success and with sin_addr filled in
 */
bool gethostbyname( const char* hostname, struct in_addr& sin_addr );

/// @return host byte order ip address on success, else MAX_UINT32 on failure
uint32 aton( const MC2String& ipWithDots );

/// @return ip with dots string from host byte order ip number
MC2String ip2str( uint32 ip );

/**
 *   IP to hex lookup.
 *   @param ip_address  The IP to convert.
 *   @return   The ip in 4 bytes.
 */
uint32 iptoh( const char *ip_address );

/**
 * Creates and returns a string to hostname
 * @param ip the ip to hostname
 * @param dnsLookup whether the ip should be looked up
 * @return string to hostname
 */
MC2String getHostName( uint32 IP, bool dnsLookup = true );

/// @return local hostname string
MC2String getLocalHostName();

/// @return local ip
uint32 getLocalIP();

inline bool isMulticast( uint32 IP ) 
{
   // first four bits in ip: ( 1 1 1 0 ) = multicast
   return ( IP & 0xF0000000 ) == 0xE0000000;
}

}

#endif // NETUTILITY_H

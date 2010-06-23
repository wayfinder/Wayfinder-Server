/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_SOCKETS_H
#define MC2_SOCKETS_H

#undef USE_UNIX_HEADERS


#if defined(__CYGWIN32__) || defined(__FreeBSD__) || defined ( __linux )
#define USE_UNIX_HEADERS
#elif defined (__SVR4) || defined (__MACH__)
#define USE_UNIX_HEADERS
#endif

#ifdef USE_UNIX_HEADERS
   // Some of these are not strictly needed in linux, but
   // needed in the other os:es. E.g. linux includes
   // sys/types.h and limits.h in sys/param.h which is not
   // done in FreeBSD and solaris.
   #include <limits.h>
   #include <sys/types.h>
   #include <sys/param.h>
   #include <sys/socket.h>
   #include <sys/time.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <unistd.h>

   #include <fcntl.h>
   #include <signal.h>

   typedef  int   SOCKET;
   #ifndef SOCKET_ERROR
      #define SOCKET_ERROR -1
   #endif
#endif

// This part is a bit strange, since it would mean that
// winsock.h will be included in Cygwin, or?

#ifdef _WIN32
   #include <winsock.h>
   #define socklen_t int 
   #define ssize_t int
   #ifndef SOCKET_ERROR
      #define SOCKET_ERROR INVALID_SOCKET
   #endif
   // unsigned int
#endif

// Define the maximum length of a host name
#ifndef MAXHOSTNAMELEN
   #define MAXHOSTNAMELEN 64
#endif



#endif


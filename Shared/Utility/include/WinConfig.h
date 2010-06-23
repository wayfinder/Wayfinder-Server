/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef  WINCONFIG_H
#  define   WINCONFIG_H

#  ifdef    _MSC_VER          // We have Visual C++
#     ifndef   STRICT
#        define   STRICT   1
#     endif

#     include  <windows.h>
#     include  <wininet.h>
#     include  <vector>

#     define   SIZEOF_CHAR    1
#     define   SIZEOF_SHORT   2
#     define   SIZEOF_INT     4
#     define   SIZEOF_LONG    4
#     define   SIZEOF_FLOAT   4
#     define   SIZEOF_DOUBLE  8

#     define   HAVE_IO_H
#     define   HAVE_WINSOCK_H

#     ifdef    HAVE_IOSTREAM
#        define   HAVE_STRSTREAM
#        define   HAVE_FSTREAM
#     else
#        define   HAVE_STRSTREA
#     endif

//#     define   HAVE_SYS_TYPES_H

#     define   HAVE__UNLINK
#     define   HAVE_WIN32_THREADS
// #     define   HAVE_EXCEPTION

// The maxium size of the host name
#ifndef MAXHOSTNAMELEN
#  define MAXHOSTNAMELEN 256
#endif

// Define PI for windows
#define M_PI   3.141592653589793
#define random() rand()
#define atanh(X) (log((1 + X)/(1 - X))/2)

//#     pragma warning( disable : 4660 )
//#     pragma warning( disable : 4290 )
//#     pragma warning( disable : 4250 )
//#     pragma warning( disable : 4355 )
#        pragma warning( disable : 4786 )

#     if _MSC_VER <= 1020                 // VC++ 4.2
#        define   SIZEOF_BOOL    0
#        define   HAVE_VCPLUSPLUS_BUGS
#        define   HAVE_NO_MUTABLE
#        define   HAVE_NO_TYPENAME

#        pragma warning( disable : 4237 )
#     else                                // >= VC++ 5.0
#        define   SIZEOF_BOOL    1
#        define   HAVE_NO_TYPENAME
#        define   HAVE_VCPLUSPLUS_BUGS

#        ifdef HAVE_IOSTREAM
#           define   HAVE_STD_NAMESPACE
#        endif
#     endif

#     ifdef _DEBUG
         // get a little more help from the mem leak checking of visual.
         // The following lines prints the file and line where the allocation was made in addition to the normal
         // length of block and first 16 bytes of data.
#        define _CRTDBG_MAP_ALLOC
#        include <stdlib.h>
#        include <malloc.h>
#        include <crtdbg.h>
#     endif

#else
#pragma message("Unknowm compiler")
#     error    Unknown compiler!

#  endif    // _MSC_VER

#endif   // WINCONFIG_H


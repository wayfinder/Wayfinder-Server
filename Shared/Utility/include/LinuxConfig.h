/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef  LINUXCONFIG_H
#  define  LINUXCONFIG_H

#  ifdef    __GNUG__       // We have g++

//#     define   SIZEOF_BOOL    1
//#     define   SIZEOF_CHAR    1
//#     define   SIZEOF_SHORT   2
//#     define   SIZEOF_INT     4
//#     define   SIZEOF_LONG    4
//#     define   SIZEOF_FLOAT   4
//#     define   SIZEOF_DOUBLE  8

//#     define   HAVE_STRERROR
//#     define   HAVE_EXCEPTION
//#     define   HAVE_FSTREAM
//#     define   HAVE_IOSTREAM
//#     define   HAVE_SCHED_H
//#     define   HAVE_STDDEF_H
//#     define   HAVE_STDLIB_H
//#     define   HAVE_STRINGS_H
//#     define   HAVE_STRSTREAM
//#     define   HAVE_SYS_SELECT_H
//#     define   HAVE_SYS_TIME_H
//#     define   HAVE_SYS_TYPES_H
//#     define   HAVE_TERMIO_H
//#     define   HAVE_TERMIOS_H
//#     define   HAVE_UNISTD_H

//#     define   HAVE_NO_NAMESPACE

//#     define   HAVE_POSIX_THREADS
//#     define   HAVE_SCHED_YIELD

#  else

#     error    Unknown compiler!

#  endif    // __GNUG__

#endif   // LINUXCONFIG_H


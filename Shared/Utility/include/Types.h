/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __TYPES_H
#define __TYPES_H
#include "config.h"

// DBL_MAX
#include <values.h>


#ifdef HAVE_STD_NAMESPACE
// To make doc++ happier
namespace std;

using namespace std;
#endif

// Some integer types for convenience
typedef  unsigned char      uint8;
typedef  signed char        int8;
typedef  unsigned short     uint16;
typedef  signed short       int16;
typedef  unsigned int       uint32;
typedef  signed int         int32;

#ifdef _MSC_VER
   typedef  unsigned __int64   uint64;
   typedef  __int64            int64;
#else
   typedef  unsigned long long uint64;
   typedef  long long          int64;
#endif

typedef  uint8              byte;

typedef  float    float32;
typedef  double   float64;

#define MAX_UINT8    uint8(0xff)
#define MAX_INT8     int8(0x7f)
#define MAX_UINT16   uint16(0xffffU)
#define MAX_INT16    int16(0x7fff)
#define MAX_UINT32   uint32(0xffffffffU)
#define MAX_INT32    int32(0x7fffffff)
#define MIN_INT8     int8(0x80)
#define MIN_INT16    int16(0x8000)
#define MIN_INT32    int32(0x80000000)
#define MAX_FLOAT64  DBL_MAX

#define REMOVE_UINT32_MSB(a) (uint32(a)&0x7fffffff)
#define GET_UINT32_MSB(a) (uint32(a)&0x80000000)
#define TOGGLE_UINT32_MSB(a) (uint32(a)^0x80000000)

#ifdef _WIN32
   #define MAX_UINT64   0xffffffffffffffff
   #define MAX_INT64    0x7fffffffffffffff
#else
   #define MAX_UINT64   ((uint64) -(uint64)1)
   #define MAX_INT64    int64(((uint64) -(uint64)1) >> 1)
   #define MIN_INT64    (int64(1)<<63)
#endif

#define MAX_BYTE     MAX_UINT8

typedef const char* const_char_p;

#endif // TYPES_H



/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_CONFIG_H
#  define MC2_CONFIG_H

#  ifdef   __sparc
#     define   WORDS_BIGENDIAN
#  endif

#  ifdef   __i386
#     define   WORDS_LITTLEENDIAN
#  endif

#  ifdef _MSC_VER
#     include  "WinConfig.h"
#  endif

#  ifdef __GNUC__
#    ifdef _WIN32 
#       include  "GnuWinConfig.h"
#    endif

#    ifdef   __linux
#       include  "LinuxConfig.h"
#    endif
#  endif

#  ifdef   __SVR4
#     include  "SolarisConfig.h"
#  endif

#  if defined(__unix__) || defined(__MACH__)
// To avoid problems with MAX,MIN
#     include  <sys/param.h>

// Hope that we have inttypes.h
// Get the format macros from inttypes.
#     define __STDC_FORMAT_MACROS
#     include  <inttypes.h>
#  endif



#include <stdio.h>



// It MC2_UTF8 is defined, utf8 is used in mc2
#define MC2_UTF8


// Set default stl allocator to use malloc
//#define __USE_MALLOC

// 
// This is an _U_G_L_Y_ way of crashing programs on
// errors. Any usage of this macro should be avoided
// and soon replaced with cleaner error-handling.
// Only for debugging!
//--------------------------------------------------------------
#ifndef _MSC_VER
#  define   PANIC(A,B)  { mc2log << fatal << A << B << endl; perror("PANIC! Last system error: "); *(char*)(NULL) = 1; exit(1); }
#else
#  define   PANIC(A,B)  { mc2log << fatal << A << B << endl; }
#endif
//--------------------------------------------------------------

// 
// Print the last system error on standard error
// -------------------------------------------------------------
#ifndef _MSC_VER
#  define   PERROR perror("Last system error")
#else
#  define   PERROR  { ; }
#endif
// -------------------------------------------------------------




//
// Used by ModuleList.h
//
// Maximum time (in ms) between statistics packets from module to 
// leader.
// The module is assumed to be dead after this time.
//--------------------------------------------------------------
#  define   MAX_TIME_NO_STATISTICS 3000
//--------------------------------------------------------------
//



// 
//
//--------------------------------------------------------------
// The DEBUG_VERBOSE() macro is deprecated and should
// not be used! Replace with the different DEGUG
// levels below!
//--------------------------------------------------------------
#  define   DEBUG_VERBOSE(a)
//--------------------------------------------------------------
//
//

#  ifdef DEBUG_LEVEL_8
#     define   DEBUG8(a)   a
#  elif DEBUG_LEVEL_8_NUM
#     define   DEBUG8(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#  else
#     define   DEBUG8(a)
#  endif

#  ifdef DEBUG_LEVEL_4
#     define   DEBUG4(a)    a
#  elif DEBUG_LEVEL_4_NUM
#     define   DEBUG4(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#  else
#     define   DEBUG4(a)
#  endif

#  ifdef DEBUG_LEVEL_2
#     define   DEBUG2(a)   a
#  elif DEBUG_LEVEL_2_NUM
#     define   DEBUG2(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#  else
#     define   DEBUG2(a)
#  endif

#  ifdef DEBUG_LEVEL_1
#     define   DEBUG1(a)   a
#  elif DEBUG_LEVEL_1_NUM
#     define   DEBUG1(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#  else
#     define   DEBUG1(a)
#  endif

// This is for debugging the database
#  ifdef DEBUG_LEVEL_DB
#     define   DEBUG_DB(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#  else
#     define   DEBUG_DB(a)
#  endif

// This is for debugging the destructors
#  ifdef DEBUG_LEVEL_DEL
#     define   DEBUG_DEL(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#  else
#     define   DEBUG_DEL(a)
#  endif

// This is for debugging the GDF extraction
#  ifdef DEBUG_LEVEL_GDF
#     define   DEBUG_GDF(a)   a
#  else
#     define   DEBUG_GDF(a)
#  endif

/**
 * Works more or less like assert.
 * @param testResult If false, something will happen, such as a line may be executed on which you should have placed a breakpoint.
 * @return dummy nothing.
 */
inline int almostAssert(bool testResult);
// implementation
inline int almostAssert(bool testResult) {
   int kalle = 3;
#if defined(_DEBUG) || defined(DEBUG_LEVEL_2)
   if (!testResult) {
#     ifndef _MSC_VER
//      MC2_ASSERT(false);
#     else
//      TRACE(_T("almostAssertion failed!"));
#     endif
      // place breakpoint here!
      kalle++;
   }
#endif
   return kalle;
}

// Error prints date time message to stderr, but doesn't exit
#ifndef _MSC_VER

// @param infoType The string to begin the message with,
//                 e.g., ERROR, WARNING, INFO.
#define MC2DEBUGPRINT( infoType, str ) { \
   char mc2debugprint_buff[1024]; \
   int mc2debugprint_pos = 0; \
   time_t mc2debugprint_t; \
	struct tm *mc2debugprint_tm; \
\
   ::time(&mc2debugprint_t); \
   struct tm mc2debugprint_result; \
   mc2debugprint_tm = localtime_r( &mc2debugprint_t, &mc2debugprint_result ); \
\
   mc2debugprint_pos += sprintf( \
      mc2debugprint_buff + mc2debugprint_pos, \
      "%04d-%02d-%02d %02d:%02d:%02d %s: ", \
      mc2debugprint_tm->tm_year + 1900, mc2debugprint_tm->tm_mon + 1, \
      mc2debugprint_tm->tm_mday, \
      mc2debugprint_tm->tm_hour, mc2debugprint_tm->tm_min, \
      mc2debugprint_tm->tm_sec, infoType ); \
   /* 1024 - 34 = 991 !!!includes length of string!!! */ \
   strncpy( mc2debugprint_buff + mc2debugprint_pos, str, 991 ); \
   mc2debugprint_buff[1023] = '\0'; \
   cerr << mc2debugprint_buff+2 << endl; \
}

#define MC2ERROR( str )   MC2DEBUGPRINT( "ERROR", str )
#define MC2WARNING( str ) MC2DEBUGPRINT( "WARN ", str )
#define MC2INFO( str )    MC2DEBUGPRINT( "INFO ", str )
#define MC2ERROR2( str, A )   { MC2ERROR(   str ) A }
#define MC2WARNING2( str, A ) { MC2WARNING( str ) A }
#define MC2INFO2( str, A )    { MC2INFO(    str ) A }
#define MC2ERROR_STMT( A ) A
#define MC2WARNING_STMT( A ) A
#define MC2INFO_STMT( A ) A

#else // if _MSC_VER is defined to something.

#define MC2ERROR( str )       almostAssert(false);
#define MC2WARNING( str )     almostAssert(false);
#define MC2ERROR2( str, A )   almostAssert(false);
#define MC2WARNING2( str, A ) almostAssert(false);
#define MC2INFO( str )
#define MC2INFO2( str, A )
#endif

// Defines to help the compiler optimizing branches.
// Example where size usually is != 0 
// if ( MC2_LIKELY( size() != 0 ) ) {
//   // Do stuff
// }
#if defined (__GNUC__) 
#   define MC2_LIKELY(expression) (__builtin_expect(!!(expression), 1))
#   define MC2_UNLIKELY(expression) (__builtin_expect(!!(expression), 0))
#   define ATTRIB_NOTHROW   __attribute__((nothrow))
#   define ATTRIB_PURE      __attribute__((pure))
#   define ATTRIB_CONST     __attribute__((const))
#   if (__GNUC__) > 2 
#     define ATTRIB_NORETURN  __attribute__((noreturn))
#   else
      //  Doesn't seem to work on older gccs.
#     define ATTRIB_NORETURN
#   endif
#else
#define MC2_LIKELY(expression) (expression)
#define MC2_UNLIKELY(expression) (expression)
#define ATTRIB_NOTHROW
#define ATTRIB_PURE
#define ATTRIB_CONST
#define ATTRIB_NORETURN
#endif


//----------------------------------------------------------
//
// Assert-macro to be used in the mc2-source.
// 
//----------------------------------------------------------
#ifdef NDEBUG
   #define MC2_ASSERT(f) do { if ( false && (f) ) {} } while (0)
#else

   #ifdef _MSC_VER
      #ifdef ASSERT
         #define MC2_ASSERT(f) ASSERT(f)
      #else
         #define MC2_ASSERT(f) { almostAssert(f); strlen(NULL); }
      #endif
   #else
      /**
       *    This method is called in the MC2_ASSERT-macro. Will flush
       *    both standard error and standard out, print the current file 
       *    and the line-number to standard error and exit the prgram
       *    with the abort()-method. This will also generate a core-file.
       *    The implementation of handleAssert is located in Utility.cpp.
       *    @param l    The name of the file.
       *    @param n    The linenumber in l.
       */
      void handleAssert(const char* l, int n, const char* e) ATTRIB_NORETURN;

      #define MC2_ASSERT_STRINGIFY(x) #x
      #define MC2_ASSERT(f)                     \
         if ( MC2_LIKELY(f) ) { ;                             \
         } else                                 \
            handleAssert(__FILE__, __LINE__, MC2_ASSERT_STRINGIFY(f) )
   #endif
#endif



//
//
//----------------------------------------------------------
// NEW NAMES THAT DO NOT CONFLICT 
// WITH STANDARD TYPES IN VC STUDIO.
//
// Replace the TRACE macros with the debug options above!
//----------------------------------------------------------
#ifndef _MSC_VER
   #define TRACE  printf
//#define TRACE if (false) printf
#endif

#  define   TRACE_2(a)
#  define   TRACE_4(a)
#  define   TRACE_8(a)
//----------------------------------------------------------
//
//


//__________________________________________________________

#ifdef _MSC_VER
      // STL include file
#  include <yvals.h>              // warning numbers get enabled in yvals.h 

#  pragma warning(disable: 4786)  // identifier was truncated to 'number' characters in 
                                  // the debug information
#endif


#  include "Types.h"

#include "MC2Logging.h"

#ifndef _MSC_VER
#  include <errno.h>
#  include <time.h>
#  include <string.h>
#  if defined (__CYGWIN32__)
#     undef __declspec
#  endif
#  define __declspec(a)
#endif

/// Macro to get the number of items in an array. Use with care
#define NBR_ITEMS(x) ( sizeof(x)/sizeof(x[0]) )
/// Macro that fails to compile if the number of elements in an array is wrong
#define CHECK_ARRAY_SIZE( x, y ) if ( true ) ; else { \
               uint32 check_array[\
               ( (y) == NBR_ITEMS( x ) ) ? 1 : -1 ] = { 0 };\
               check_array[0] = 8;\
               }


#endif //   MC2_CONFIG_H

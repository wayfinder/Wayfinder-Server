/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TimeUtility.h"

#include <sys/time.h>

namespace TimeUtility {

// Variables for testing.
//
// These are implemented with functions to make sure they are 
// initialized before use.

/// Whether testing has been activated.
bool& isTestingTime() {
   static bool testing = false;
   return testing;
}

/// How much to auto increase the test time.
uint32& autoIncrease() {
   static uint32 ai = 0;
   return ai;
}

/// Test simulated time.
uint32& testTime() {
   static uint32 tt = 0;
   return tt;
}

uint32 getCurrentTime() 
{
   if ( isTestingTime() ) {
      uint32 result = testTime();
      testTime() += autoIncrease();
      return result;
   }
#ifndef _MSC_VER
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (tv.tv_usec/1000 + tv.tv_sec * 1000);
#else
   static struct _SYSTEMTIME tv;
   GetSystemTime(&tv);
   return(tv.wMilliseconds + tv.wSecond * 1000 + tv.wMinute * 60 * 1000 );
#endif
}

uint32 getCurrentMicroTime() 
{
   if ( isTestingTime() ) {
      return getCurrentTime()*1000;
   }
#ifndef _MSC_VER
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (tv.tv_usec + tv.tv_sec * 1000000);
#else
   struct _SYSTEMTIME tv;
   GetSystemTime(&tv);
   return(tv.wMilliseconds*1000 + tv.wSecond * 1000000 + tv.wMinute * 60 * 1000000 );
#endif
}



uint32 getRealTime()
{
   if ( isTestingTime() ) {
      return getCurrentTime()/1000;
   }
#ifndef _MSC_VER
   return (time(NULL));
#else
   struct _SYSTEMTIME tv;
   GetSystemTime(&tv);
   return(tv.wSecond);
#endif
}

void microSleep( uint32 microseconds ) {
   struct timeval tv;
   tv.tv_sec  = microseconds / 1000000;
   tv.tv_usec = microseconds % 1000000;

   if ( select( 0, NULL, NULL, NULL, &tv ) < 0 ) {
      perror( "Utility::sleep select error: " );
   }
}

uint32 convertDate( uint32 year, uint32 month, uint32 day ) {
   struct tm tmstruct;

   tmstruct.tm_year=((int32)year)-1900;
   tmstruct.tm_mon=((int32)month)-1;
   tmstruct.tm_sec = 0;
   tmstruct.tm_min = 0;
   tmstruct.tm_hour = -::timezone/3600;
   tmstruct.tm_isdst = 0;
   tmstruct.tm_mday = ((int32)day);

   return mktime( &tmstruct );
}

void startTestTime( uint32 autoIncreaseTime ) {
   isTestingTime() = true;
   autoIncrease() = autoIncreaseTime;
   testTime() = 0;
}

void stopTestTime() {
   isTestingTime() = false;
}

void testSleep( uint32 ms ) {
   testTime() += ms;
}

}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

# define __USE_XOPEN   //dev-1
#define _SVID_SOURCE   //dev-1
#define _XOPEN_SOURCE  //dev-3
#include <time.h>
#undef __USE_XOPEN     //dev-1
#undef _SVID_SOURCE    //dev-1
#undef _XOPEN_SOURCE   //dev-3
#include "config.h"
#include "ISO8601TimeParser.h"

time_t ISO8601Time::getTime(const MC2String& time)
{
   struct tm parse_tm = { 0 };
#if 0
   int parsed = 0;
   int conv = sscanf(time, "%d-%d-%dT%d:%d:%d%n", &(parse_tm.tm_year), 
                     &(parse_tm.tm_mon), &(parse_tm.tm_mday), 
                     &(parse_tm.tm_hour), &(parse_tm.tm_min), 
                     &(parse_tm.tm_sec), &parsed);
   char* end = (conv == 6) ? time + parsed : NULL;
#else
   char* end = strptime( time.c_str(), "%Y-%m-%dT%H:%M:%S", &parse_tm );
#endif
   if(end != NULL && *end != '\0'){
      while(*end && isspace(*end)){
         ++end;
      }
      if(*end == 'Z'){
         //UTC, do nothing
      } else {
         int tz_offset_hours = 0;
         int tz_offset_minutes = 0;
         int chars = 0;
         if( 2 == sscanf(end, "%d:%d%n", &tz_offset_hours, 
                         &tz_offset_minutes, &chars) && chars == 6){
            //[+-]hh:mm
            int sign = tz_offset_hours >= 0 ? +1 : -1;
            tz_offset_hours  *= sign;
            parse_tm.tm_hour += -1 * sign * tz_offset_hours;
            parse_tm.tm_min  += -1 * sign * tz_offset_minutes;
         } else if ( 1 == sscanf(end, "%d%n", &tz_offset_hours, &chars) &&
                     (chars == 5 || chars == 3)) {
            int sign = tz_offset_hours >= 0 ? +1 : -1;
            tz_offset_hours *= sign;
            if(chars == 5){
               //[+-]hhmm
               int tmp = tz_offset_hours / 100;
               tz_offset_minutes = tz_offset_hours - (tmp * 100);
               tz_offset_hours = tmp * 100;
            } else {
               //[+-]hh
            }
            parse_tm.tm_hour += -1 * sign * tz_offset_hours;
            parse_tm.tm_min  += -1 * sign * tz_offset_minutes;            
         } else {

         }
      }
   } else {
      // TODO: This is for local time only from the machine, is this correct?
      parse_tm.tm_hour += ::timezone / 3600 - ::daylight; 
   }
   return timegm(&parse_tm);
}

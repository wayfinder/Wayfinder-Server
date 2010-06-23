/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIMEUTILITY_H
#define TIMEUTILITY_H

#include "config.h"

/// Contains useful functions for handling of time
namespace TimeUtility {

/**
 *   Get the current time in milliseconds. The time wraps every 
 *   1193 minute (=49.7 days) so no timedifferance may be longer 
 *   than that.
 *   @return Relative time in milliseconds.
 */
uint32 getCurrentTime();

/**
 *   Get the current time in microseconds. This time wraps quite 
 *   often so only short intervals can be measured!
 *   @return Current Time in microseconds.
 */
uint32 getCurrentMicroTime();

/**
 *    Get the current real time in seconds since 00:00:00 
 *    1 January 1970, Coordinated Universal Time (UTC).
 *    Use gmtime, localtime and strftime to get 
 *    more human times.
 *    @return The number of elapsed seconds since 00:00:00 1 Jan 1970.
 */
uint32 getRealTime();

/**
 *  Sleeps the specified time (using select).
 *  @param microseconds The number of microseconds to sleep.
 */
void microSleep( uint32 microseconds );

/**
 * Calculates how many seconds has passed since 1 Jan 1970
 */
uint32 convertDate( uint32 year, uint32 month, uint32 day );

/**
 * Converts minuts into seconds.
 * @return number of seconds.
 */
inline uint32 minuts2sec( uint32 minuts ) {
   return minuts * 60;
}

/**
 * Converts hours into seconds.
 * @return number of seconds.
 */
inline uint32 hours2sec( uint32 hours ) {
   return hours * minuts2sec( 60 );
}

/**
 * Converts days into seconds.
 * @return number of seconds.
 */
inline uint32 days2sec( uint32 days ) {
   return days * hours2sec( 24 );
}

/**
 * Causes the functions in TimeUtility to follow simulated time instead
 * of using the system clock. This is meant to be used in testing.
 * 
 * @param autoIncreaseTime  Each read of time will cause the test time to
 *                          increase by this many ms.
 */
void startTestTime( uint32 autoIncreaseTime = 0 );

/**
 * Stops using simulated time.
 */
void stopTestTime();

/**
 * Increases the simulated time by a number of ms.
 * 
 * @ms   Number of ms to sleep.
 */
void testSleep( uint32 ms );

}

#endif // TIMEUTILITY_H

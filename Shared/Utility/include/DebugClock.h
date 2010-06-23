/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DEBUG_CLOCK_H
#define DEBUG_CLOCK_H

#include "config.h"

#include "TimeUtility.h"
#include <iosfwd>

/// uses milli seconds 
class DebugClock {
public:
   inline DebugClock() : m_startTime( TimeUtility::getCurrentTime() ) {}
   /// @return time in msec
   inline uint32 getTime() const {
      return TimeUtility::getCurrentTime() - m_startTime;
   }
private:
   uint32 m_startTime;
};

/// uses micro seconds 
class DebugClockMicro {
public:
   DebugClockMicro(): m_startTime( TimeUtility::getCurrentMicroTime() ) { }
   /// @return time in usec
   uint32 getTime() const {
      return TimeUtility::getCurrentMicroTime() - m_startTime;
   }
private:
   uint32 m_startTime;
};

/// Uses seconds.
class DebugClockSeconds {
public:
   DebugClockSeconds(): m_startTime( TimeUtility::getRealTime() ) { }
   /// @return elapsed time in seconds.
   uint32 getTime() const {
      return TimeUtility::getRealTime() - m_startTime;
   }
private:
   /// Start time in seconds since 00:00:00 1970-01-01 UTC.
   uint32 m_startTime;
};

/// write debug clock to ostream
inline ostream& operator<<( ostream& ost, const DebugClock& clock) {
   ost << clock.getTime() << " ms";
   return ost;
}

/// write debug clock micro to ostream
inline ostream& operator << ( ostream& ost, const DebugClockMicro& clock ) {
   ost << clock.getTime() << " us";
   return ost;
}

inline ostream& operator << ( ostream& ost, const DebugClockSeconds& clock ) {
   ost << clock.getTime() << " s";
   return ost;
}

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ItemTypes.h"
#include "RMDriverPref.h"

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 30200
#include <strstream>
#else
#include <sstream>
#endif

RMDriverPref::RMDriverPref()
{
   m_cost         = 0x00010000;
   m_vehicleParam = ItemTypes::passengerCar;
   m_time         = 0;
   m_minWaitTime  = 0;
   m_isStartTime  = true;
   m_useUturn     = false;
}

void RMDriverPref::dump()
{
   mc2log << info << " cost " << hex << m_cost
          << " vehicle " << m_vehicleParam << dec
          << " time " << m_time
          << " minWaitTime " << m_minWaitTime
          << " isStartTime " << m_isStartTime 
          << " useUturn " << m_useUturn << endl;
}

ostream&
operator<<(ostream& stream,
           const RMDriverPref& pref)
{
#if GCC_VERSION < 30200
   strstream strstr;
#else
   stringstream strstr;
#endif
   strstr << '[' << uint32(pref.getCostA())
          << ',' << uint32(pref.getCostB())
          << ',' << uint32(pref.getCostC())
          << ',' << uint32(pref.getCostD())
          << ',';
   if ( pref.m_vehicleParam == ItemTypes::passengerCar ) {
      strstr << "\"car\"";
   } else if ( pref.m_vehicleParam == ItemTypes::pedestrian ) {
      strstr << "\"ped\"";
   } else {
      strstr << "veh: 0x" << hex << uint32(pref.m_vehicleParam) << dec;
   }
   strstr << ']' << ends;
   stream << strstr.str();
#if GCC_VERSION < 30200
   strstr.freeze(0);
#endif
   return stream;
}

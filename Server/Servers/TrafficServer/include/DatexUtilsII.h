/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATEX_UTILS_II_H
#define DATEX_UTILS_II_H

#include "config.h"
#include "TrafficDataTypes.h"
#include "MC2String.h"

#include <map>

class DatexUtilsII
{
public:

   /// Map for translating from message code to phrase
   static map<MC2String, TrafficDataTypes::phrase> c_phraseMap;

   /// Map for translation from message code to type
   static map<MC2String, TrafficDataTypes::disturbanceType> c_typeMap;

   /// Map for translating from phrase to severity
   static map<TrafficDataTypes::phrase, TrafficDataTypes::severity>
      c_phraseToSeverity;

   /// Map for geting estimated end time from disturbance type
   static map< TrafficDataTypes::disturbanceType, uint32 > c_estEndTimeMap;

   /// Method for initializing lookup tables.
   static bool initializeTables();

   /// Dummy bool for initialization
   static bool c_initialized;

private:
};

#endif // DATEX_UTILS_II_H

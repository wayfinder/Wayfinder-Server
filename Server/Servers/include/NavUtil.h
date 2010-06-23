/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAV_UTIL_H
#define NAV_UTIL_H

#include "config.h"
#include "StringTable.h"
#include "ItemTypes.h"
#include "RouteTypes.h"


/**
 * Class for holding some utility functions for NavMessages.
 */
class NavUtil {
public:
      /**
    * Converts a Nav language to a MC2 languageCode.
    * Returns StringTable::SMSISH_ENG if invalid value.
    */
   static StringTable::languageCode mc2LanguageCode( uint32 language );

   
   /**
    * Converts a Nav vehicle to a ItemTypes::vehicle_t.
    *
    * @param vehicle The Nav vehicle.
    * @returns The ItemTypes::vehicle_t for the Nav vehicle.
    */
   static ItemTypes::vehicle_t mc2Vehicle( uint8 vehicle );


   /**
    * Converts a Nav route cost to a mc2 route cost.
    * 
    * @param routeCost The Nav route cost.
    * @return The mc2 route cost for the nav routeCost.
    */
   static RouteTypes::routeCostType mc2RouteCost( uint8 routeCost );


private:
   /// Private constructor to avoid objects.
   NavUtil();

};

#endif // NAV_UTIL_H

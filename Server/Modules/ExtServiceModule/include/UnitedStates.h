/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UNITEDSTATES_H
#define UNITEDSTATES_H

#include "config.h"

namespace UnitedStates {

/// State top regions
enum States {
   ALASKA = 10,
   ARIZONA = 11,
   ARKANSAS = 12,
   CALIFORNIA = 13,
   COLORADO = 14,
   CONNECTICUT = 15,
   DELAWARE = 16,
   DISTRICT_OF_COLUMBIA = 17,
   FLORIDA = 18,
   GEORGIA = 19,
   HAWAII = 20,
   IDAHO = 21,
   ILLINOIS = 22,
   INDIANA = 23,
   IOWA = 24,
   KANSAS = 25,
   KENTUCKY = 26,
   LOUISIANA = 27,
   MAINE = 28,
   MARYLAND = 29,
   MASSACHUSETTS = 30,
   MICHIGAN = 31,
   MINNESOTA = 32,
   MISSISSIPPI = 33,
   MISSOURI = 34,
   MONTANA = 35,
   NEBRASKA = 36,
   NEVADA = 37,
   NEW_HAMPSHIRE = 38,
   NEW_JERSEY = 39,
   NEW_MEXICO = 40,
   NEW_YORK = 41,
   NORTH_CAROLINA = 42,
   NORTH_DAKOTA = 43,
   OHIO = 44,
   OKLAHOMA = 45,
   OREGON = 46,
   PENNSYLVANIA = 47,
   RHODE_ISLAND = 48,
   SOUTH_CAROLINA = 49,
   SOUTH_DAKOTA = 50,
   TENNESSEE = 51,
   TEXAS = 52,
   UTAH = 53,
   VERMONT = 54,
   VIRGINIA = 55,
   WASHINGTON = 56,
   WEST_VIRGINIA = 57,
   WISCONSIN = 58,
   WYOMING = 59,
   ALABAMA = 60,
};

/// Return state from top region.
inline const char* topRegionToState( uint32 topRegion ) {
   switch ( (States)( topRegion ) ) {
   case ALASKA:
      return "AK";
   case ARIZONA:
      return "AZ";
   case ARKANSAS:
      return "AR";
   case CALIFORNIA:
      return "CA";
   case COLORADO:
      return "CO";
   case CONNECTICUT:
      return "CT";
   case DELAWARE:
      return "DE";
   case DISTRICT_OF_COLUMBIA:
      return "DC";
   case FLORIDA:
      return "FL";
   case GEORGIA:
      return "GA";
   case HAWAII:
      return "HI";
   case IDAHO:
      return "ID";
   case ILLINOIS:
      return "IL";
   case INDIANA:
      return "IN";
   case IOWA:
      return "IA";
   case KANSAS:
      return "KS";
   case KENTUCKY:
      return "KY";
   case LOUISIANA:
      return "LA";
   case MAINE:
      return "ME";
   case MARYLAND:
      return "MD";
   case MASSACHUSETTS:
      return "MA";
   case MICHIGAN:
      return "MI";
   case MINNESOTA:
      return "MN";
   case MISSISSIPPI:
      return "MS";
   case MISSOURI:
      return "MO";
   case MONTANA:
      return "MT";
   case NEBRASKA:
      return "NE";
   case NEVADA:
      return "NV";
   case NEW_HAMPSHIRE:
      return "NH";
   case NEW_JERSEY:
      return "NJ";
   case NEW_MEXICO:
      return "NM";
   case NEW_YORK:
      return "NY";
   case NORTH_CAROLINA:
      return "NC";
   case NORTH_DAKOTA:
      return "ND";
   case OHIO:
      return "OH";
   case OKLAHOMA:
      return "OK";
   case OREGON:
      return "OR";
   case PENNSYLVANIA:
      return "PA";
   case RHODE_ISLAND:
      return "RI";
   case SOUTH_CAROLINA:
      return "SC";
   case SOUTH_DAKOTA:
      return "SD";
   case TENNESSEE:
      return "TN";
   case TEXAS:
      return "TX";
   case UTAH:
      return "UT";
   case VERMONT:
      return "VT";
   case VIRGINIA:
      return "VA";
   case WASHINGTON:
      return "WA";
   case WEST_VIRGINIA:
      return "WV";
   case WISCONSIN:
      return "WI";
   case WYOMING:
      return "WY";
   case ALABAMA:
      return "AL";
   }

   return "";
}

}
#endif // UNITEDSTATES_H

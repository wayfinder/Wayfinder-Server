/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StringTableUtility.h"
#include "StringUtility.h"

#include <stdio.h>

namespace StringTableUtility {

int32 printDistance( char* buffer,
                     int meters,
                     StringTable::languageCode lc,
                     distanceFormat distFormat,
                     distanceUnit distUnit ) {
   int feet  = (int)(meters/ 0.3048);
   int yards = (int)(meters/ 0.9144);
   int tenthmiles = (int)(meters / 160.9344);
   
   switch( distFormat ) {
      case( EXACT ) :
         if(distUnit == METERS){
            return sprintf(buffer, "%i %s", meters,
                           StringTable::getString( StringTable::METERS, lc ));
         } else if(distUnit == YARDS){
            return sprintf(buffer, "%i %s", yards,
                           StringTable::getString( StringTable::IMPERIAL_YARDS,
                                                   lc));
         } else if(distUnit == FEET){
            return sprintf(buffer, "%i %s", feet,
                           StringTable::getString( StringTable::IMPERIAL_FEET,
                                                   lc ));
         } 
         
         break;
         
      case ( NORMAL ):
      case ( COMPACT ) :
         const char* space = "";
         if ( distFormat == NORMAL ) {
            space = " ";
         }
         if(distUnit == METERS){
            // + 5/50/500 to round instead of truncate
            if ( meters < 10){
               return sprintf(buffer, "%i%s%s",meters , space,
                              StringTable::getString( StringTable::METERS, lc ));
            } else if ( meters < 100 ) {
               uint32 printDist = ( ( meters + 2 ) / 5 ) * 5;
               return sprintf(buffer, "%i%s%s",printDist , space,
                              StringTable::getString( StringTable::METERS, lc ));
            } else if (meters + 5 < 1000 ) {
               uint32 printDist = ( ( meters + 5 ) / 10 ) * 10;
               return sprintf(buffer, "%i%s%s", printDist , space,
                              StringTable::getString( StringTable::METERS, lc ));
            } else if (meters + 50 < 10000 ) {
               uint32 kms = ( meters + 50 ) / 1000; // Round the kilometers
               uint32 afterdot = ( ( meters - kms * 1000 ) + 50 ) / 100;
               // Afterdot should now contain one digit.
               return sprintf(buffer, "%i.%u%s%s", kms, afterdot, space,
                              StringTable::getString(
                                 StringTable::KILOMETERS, lc ));
            } else {
               // Print integer part of the kilometers 
               return sprintf(buffer, "%i%s%s", ( meters + 500 ) / 1000, space,
                              StringTable::getString(
                                 StringTable::KILOMETERS, lc ));
            }
         } else if(distUnit == YARDS){
             // + 5/50/500 to round instead of truncate
            if ( yards < 10){
               return sprintf(buffer, "%i%s%s",yards , space,
                              StringTable::getString( StringTable::IMPERIAL_YARDS,
                                                      lc ));
            } else if ( yards < 100 ) {
               uint32 printDist = ( ( yards + 2 ) / 5 ) * 5;
               return sprintf(buffer, "%i%s%s",printDist , space,
                              StringTable::getString( StringTable::IMPERIAL_YARDS,
                                                      lc ));
            } else if (yards + 5 < 1000 ) {
               uint32 printDist = ( ( yards + 5 ) / 10 ) * 10;
               return sprintf(buffer, "%i%s%s", printDist , space,
                              StringTable::getString( StringTable::IMPERIAL_YARDS,
                                                      lc ));
            } else if (tenthmiles < 100 ) {
               uint32 miles = ( tenthmiles ) / 10; // Round the kilometers
               uint32 afterdot = (  tenthmiles - miles * 10 );
               // Afterdot should now contain one digit.
               return sprintf(buffer, "%i.%u%s%s", miles, afterdot, space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_MILES, lc ));
            } else {
               // Print integer part of the kilometers
               return sprintf(buffer, "%i%s%s", ( tenthmiles + 5 ) / 10, space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_MILES, lc ));
            }
           
         } else if(distUnit == FEET){
            // + 5/50/500 to round instead of truncate
            if ( feet < 10){
               return sprintf(buffer, "%i%s%s",feet , space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_FEET, lc ));
            } else if ( feet < 100 ) {
               uint32 printDist = ( ( feet + 2 ) / 5 ) * 5;
               return sprintf(buffer, "%i%s%s",printDist , space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_FEET, lc ));
            } else if (feet + 5 < 1000 ) {
               uint32 printDist = ( ( feet + 5 ) / 10 ) * 10;
               return sprintf(buffer, "%i%s%s", printDist , space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_FEET, lc ));
            } else if (tenthmiles < 100 ) {
               uint32 miles = ( tenthmiles ) / 10; // Round the kilometers
               uint32 afterdot = (  tenthmiles - miles * 10 );
               // Afterdot should now contain one digit.
               return sprintf(buffer, "%i.%u%s%s", miles, afterdot, space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_MILES, lc ));
            } else {
               // Print integer part of the kilometers
               return sprintf(buffer, "%i%s%s", ( tenthmiles + 5 ) / 10, space,
                              StringTable::getString(
                                 StringTable::IMPERIAL_MILES, lc ));
            }
           
         } 
         break;
   }
   //Something is in error
   return -1;
}

char* setDefaultCountryCode( char* phoneNumber,
                             StringTable::countryCode defaultCountryCode ) {

   // The default values. Might become static members of this class later...
   const char* REPLACE_BEGINNING = "0";
   const char* defaultCCString = StringTable::getCountryPhoneCode(defaultCountryCode);

   // Remove all nondigits in the string (e.g. '+' or '-')
//    int phoneLength = strlen(phoneNumber);
//    char* tmpPhoneNumber = new char[strlen(phoneNumber) + 1];
//    int writePos = 0;
//    for (int pos=0; pos < phoneLength; pos++) {
//       if (isdigit(phoneNumber[pos]) != 0) {
//          tmpPhoneNumber[writePos] = phoneNumber[pos];
//          writePos++;
//       }
//    }
//    tmpPhoneNumber[writePos] = '\0';
   char* tmpPhoneNumber = StringUtility::removeAllButDigits( phoneNumber );
   delete [] phoneNumber;
   phoneNumber = tmpPhoneNumber;

   // The char* to return
   char* retVal = phoneNumber;

   // Check if to do any replacement
   int replaceLength = strlen(REPLACE_BEGINNING);
   int phoneLength = strlen( phoneNumber );
   if ( (replaceLength < phoneLength) &&
        (strncmp(phoneNumber, REPLACE_BEGINNING, replaceLength) == 0)) {
      // Found REPLACE_BEGINNING in the beginning of phoneNumber, 
      // replace with DEFAULT_COUNTRY_CODE
      retVal = new char[replaceLength + phoneLength + 1];
      strcpy(retVal, defaultCCString);
      strcat(retVal, &phoneNumber[replaceLength]);
      DEBUG2(cerr << "retVal = " << retVal << ", phoneNumber = " 
                  << phoneNumber << endl);
      delete [] phoneNumber;
   }

   // Return
   return (retVal);
}

const char* turnToString( StringTable::stringCode turn ) {
   if ( turn == StringTable::LEFT_TURN ) {
      return "left";
   } else if ( turn == StringTable::RIGHT_TURN ) {
      return "right";
   } else if ( turn == StringTable::AHEAD_TURN ) {
      return "ahead";
   } else if ( turn == StringTable::U_TURN ) {
      return "u_turn";
   } else if ( turn == StringTable::FOLLOWROAD_TURN ) {
      return "followroad";
   } else if ( turn == StringTable::ENTER_ROUNDABOUT_TURN ) {
      return "enter_roundabout";
   } else if ( turn == StringTable::EXIT_ROUNDABOUT_TURN ) {
      return "exit_roundabout";
   } else if ( turn == StringTable::AHEAD_ROUNDABOUT_TURN ) {
      return "ahead_roundabout";
   } else if ( turn == StringTable::RIGHT_ROUNDABOUT_TURN ) {
      return "right_roundabout";
   } else if ( turn == StringTable::LEFT_ROUNDABOUT_TURN ) {
      return "left_roundabout";
   } else if ( turn == StringTable::OFF_RAMP_TURN ) {
      return "off_ramp";
   } else if ( turn == StringTable::LEFT_OFF_RAMP_TURN ) {
      return "left_off_ramp";
   } else if ( turn == StringTable::RIGHT_OFF_RAMP_TURN ) {
      return "right_off_ramp";
   } else if ( turn == StringTable::ON_RAMP_TURN ) {
      return "on_ramp";
   } else if ( turn == StringTable::ENTER_BUS_TURN ) {
      return "enter_bus";
   } else if ( turn == StringTable::EXIT_BUS_TURN ) {
      return "exit_bus";
   } else if ( turn == StringTable::CHANGE_BUS_TURN ) {
      return "change_bus";
   } else if ( turn == StringTable::PARK_CAR ) {
      return "park_car";
   } else if ( turn == StringTable::DRIVE_FINALLY ) {
      return "finally";
   } else if ( turn == StringTable::DRIVE_START ) {
      return "start";
   } else if ( turn == StringTable::EXIT ) {
      return "exit";
   } else if ( turn == StringTable::KEEP_LEFT ) {
      return "keep_left";
   } else if ( turn == StringTable::KEEP_RIGHT ) {
      return "keep_right";
   } else if ( turn == StringTable::ENTER_FERRY_TURN ) {
      return "enter_ferry";
   } else if ( turn == StringTable::EXIT_FERRY_TURN ) {
      return "exit_ferry";
   } else if ( turn == StringTable::CHANGE_FERRY_TURN ) {
      return "change_ferry";
   } else if ( turn == StringTable::DRIVE_START_WITH_UTURN ) {
      return "start_with_u_turn";
   } else if ( turn == StringTable::U_TURN_ROUNDABOUT_TURN ) {
      return "u_turn_roundabout";
   } else if ( turn == StringTable::ENDOFROAD_LEFT_TURN ) {
      return "endofroad_left_turn";
   } else if ( turn == StringTable::ENDOFROAD_RIGHT_TURN ) {
      return "endofroad_right_turn";
   } else {
      return "";
   }
}

} // StringTableUtility


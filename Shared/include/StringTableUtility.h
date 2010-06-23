/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGTABLEUTILITY_H
#define STRINGTABLEUTILITY_H

#include "StringTable.h"

/**
 * @namespace Various functions using StringTable and strings,
 *            moved from StringUtility.
 * 
 */
namespace StringTableUtility {

/**
 *   Formattypes for distances.
 */
enum distanceFormat {
   EXACT   = 0,
   NORMAL  = 1,
   COMPACT = 2
};

/**
 *   Units for distances.
 */
enum distanceUnit {
   METERS = 0,
   YARDS  = 1,   // Miles & Yards
   FEET   = 2    // Miles & Feet 
};

/**
 * Print a distance to a buffer. The result's format is
 * depending on the distance.
 * @remark distUnit is currently IGNORED.
 *
 * @param buffer     The buffer where to write the distance.
 * @param meters     The number of meters to write.
 * @param lc         Languagecode for the unit's text.
 * @param distFormat Format of output (EXACT, NORMAL, COMPACT).
 * @param distUnit   What kind of unit to use (METER, YARDS).
 * @return The number of characters written or -1 if unsuccessful.
 */
int32 printDistance( char* buffer,
                     int meters,
                     StringTable::languageCode lc,
                     distanceFormat distFormat,
                     distanceUnit distUnit //= METERS
                     );

 /**
  *    Set the default country code for the given phonenumber.
  *    This operation is performed if the given phonenumber 
  *    starts with REPLACE_BEGINNING (declaered inside this method).
  *    A new, temporary string is created inside this method, so
  *    correct usage of this method is 
  *    #phonenbr = StringUtility::setDefaultCountryCode(phonenbr);#.
  *    This also means that the length of the returned string not
  *    necessery is the same as the one given as parameter.
  *
  *    @param   phoneNumber The phone number to change (if 
  *                         necessery).
  *    @param   defaultCountryCode The country code to insert in
  *                                place of a leading zero.
  *    @return  A pointer to the converted phonenumber (or the same).
  *             @remark See usage for how to use this method to avoid 
  *                     memory leaks etc.}
  * REMOVE
  */
char* setDefaultCountryCode(char* phoneNumber,
                            StringTable::countryCode defaultCountryCode);

/**
 * Returns a string representing the turn stringcode in parameter.
 * 
 * @param turn The stringCode for the turn.
 * @return String representing the turn if turn isn't a turn
 *         stringCode then "" is returned..
 */
const char* turnToString( StringTable::stringCode turn );


} // StringTableUtility

#endif // STRINGTABLEUTILITY_H

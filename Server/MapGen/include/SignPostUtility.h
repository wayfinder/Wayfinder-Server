/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIGNPOSTUTILITYS_H
#define SIGNPOSTUTILITYS_H

#include "config.h"
#include "StringTable.h"
#include "SignPost.h"
#include "GMSSignPostElm.h"


class GMSSignPostSet;
class GenericMap;
class OldGenericMap;
class LangType;

namespace SignPostUtility {

static const uint32 DEFAULT_PRIO = 10;
static const uint32 INVALID_PRIO = MAX_UINT32;

/**
 * Make a SignPost and return priority for it.
 *
 * @param sign Data is set in this.
 * @param inData The sign to make SignPost for.
 * @param map The GenericMap to have the resulting string in.
 * @param newSigns Whether or not it should use the new sign table.
 * @return The prio for the sign, MAX_UINT32 if sign shouldn't be used.
 */
uint32 makeSignPost( SignPost& sign, MC2String& str,
                     LangType& strLang,
                     const GMSSignPostSet& inData,
                     const GenericMap& map, 
                     const OldGenericMap& oldMap,
                     bool newSigns );



void convertSignPost( const OldGenericMap& map,
                      GMSSignPostSet& inData );

/**
 * Get the colors of the sign.
 *
 * @return True if suitable color found, else default color is set.
 */

bool getColorForNew( const GMSSignPostSet& signs,
                     StringTable::countryCode country,
                     SignColors& colors,
                     uint32& prio,
                     bool& exit );
/**
 * Get the colors of the sign.
 *
 * @return True if suitable color found, else default color is set.
 */
bool getColorFor( const GMSSignPostSet& signs,
                  StringTable::countryCode country,
                  SignColors& colors,
                  uint32& prio,
                  bool& exit );

} // SignPostUtility


#endif // SIGNPOSTUTILITY_H


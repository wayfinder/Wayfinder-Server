/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COUNTRYCODESTRINGTABLE_H
#define COUNTRYCODESTRINGTABLE_H

#include "config.h"
#include "StringTable.h"
#include "LangTypes.h"
#include <set>

class CountryCodeStringTable {
   public :

   /**
    *   Returns true if the street number is going to be first in the
    *   address..
    */
   static bool streetNumberFirst(StringTable::countryCode cc,
                                 LangTypes::language_t lang);
   
   /**
    *   Returns true if there is going to be a comma between the
    *   street number and the street name.
    */
   static bool streetNumberComma(StringTable::countryCode cc,
                                 LangTypes::language_t lang);
   
   // Inits the map m_countriesFirst.
   static void init();

  private:
   // True if the map m_countriesFirst is initiated.
   static bool m_initiated;

   // A map with all the counties is supposed to have the street number
   // first.
   static set<StringTable::countryCode> m_streetNumberFirstCountry;
   
   // A set with all the countries that is supposed to have a comma between
   // the street number and the street name.
   static set<StringTable::countryCode> m_streetNumberCommaCountry;

   // A set with all the languages that is supposed to have the street number
   // first.
   static set<LangTypes::language_t> m_streetNumberFirstLanguage;
   
   // A set with all the languages that is supposed to have a comma between
   // the street number and the street name.
   static set<LangTypes::language_t> m_streetNumberCommaLanguage;

};

#endif

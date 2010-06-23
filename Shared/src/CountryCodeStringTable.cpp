/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CountryCodeStringTable.h"

bool
CountryCodeStringTable::streetNumberFirst(StringTable::countryCode cc,
                                          LangTypes::language_t lang)
{
   init();

   if ( lang != LangTypes::invalidLanguage ) {
      return m_streetNumberFirstLanguage.count( lang ) > 0;
   }
   else {
      return m_streetNumberFirstCountry.count( cc ) > 0;
   }
}

bool
CountryCodeStringTable::streetNumberComma(StringTable::countryCode cc,
                                          LangTypes::language_t lang) 
{
   init();

   return m_streetNumberCommaLanguage.count( lang ) > 0;
}


bool CountryCodeStringTable::m_initiated = false;
set<StringTable::countryCode> CountryCodeStringTable::m_streetNumberFirstCountry;
set<StringTable::countryCode> CountryCodeStringTable::m_streetNumberCommaCountry;
set<LangTypes::language_t> CountryCodeStringTable::m_streetNumberFirstLanguage;
set<LangTypes::language_t> CountryCodeStringTable::m_streetNumberCommaLanguage;

void
CountryCodeStringTable::init()
{
   if (! m_initiated) {
      StringTable::countryCode ccarr[] = {
         StringTable::AUSTRALIA_CC,
         StringTable::CANADA_CC,
         StringTable::FRANCE_CC,
         StringTable::ENGLAND_CC,
         StringTable::HONG_KONG_CC,
         StringTable::ISRAEL_CC,
         StringTable::IRELAND_CC,
         StringTable::LUXEMBOURG_CC,
         StringTable::NEW_ZEALAND_CC,
         StringTable::CHINA_CC,
         StringTable::SOUTH_AFRICA_CC,
         StringTable::SOUTH_KOREA_CC,
         StringTable::TAIWAN_CC,
         StringTable::USA_CC,
         StringTable::SINGAPORE_CC,
         StringTable::INDIA_CC
      };

      int nbrCountries = sizeof( ccarr ) / sizeof( ccarr[0] );

      for ( int i = 0; i < nbrCountries; ++i ) {
         m_streetNumberFirstCountry.insert( ccarr[ i ] );
      }

      m_streetNumberFirstLanguage.insert(LangTypes::english);
      m_streetNumberFirstLanguage.insert(LangTypes::french);      
   }
   m_initiated = true;
}

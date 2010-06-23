/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NameUtility.h"
#include <stdlib.h>

const uint32 NameUtility::maxLangPoints = 100;


inline NameUtility::bestLanguageCodeMap initBestLanguageCodeMap() {
   NameUtility::bestLanguageCodeMap theMap;
   
   for ( uint32 lang_t = LangTypes::english ; 
         lang_t < LangTypes::nbrLanguages; ++lang_t )
   {
      uint32 bestDist = 0;
      StringTable::languageCode bestLang = StringTable::ENGLISH;
      LangTypes::language_t lang_tInLang_t = 
         LangTypes::language_t( lang_t );

      // Get closest lang
      for ( uint32 lang = StringTable::ENGLISH ; 
            lang != StringTable::SMSISH_ENG ; ++lang )
      {
         uint32 dist = NameUtility::getLangPoints( 
            ItemTypes::getLanguageCodeAsLanguageType( 
               StringTable::languageCode( lang ) ), lang_tInLang_t );
         if ( dist > bestDist ) {
            bestDist = dist;
            bestLang = StringTable::languageCode( lang );
         }
      }
      theMap.insert( make_pair( lang_tInLang_t, 
                                make_pair( bestLang, bestDist ) ) );
   }
   return theMap;
}


NameUtility::bestLanguageCodeMap NameUtility::m_bestLanguageCodeMap = 
initBestLanguageCodeMap();

inline void addPrefLang( NameUtility::PreferedLanguageCont& cont, 
                  LangTypes::language_t lang, 
                  const LangTypes::language_t* zht,
                  size_t nbr ) {
   NameUtility::PreferedLanguageCont::mapped_type m;
   for ( size_t i = 0 ; i < nbr ; ++i ) {
      // More points is better
      m.insert( make_pair( zht[ i ], NameUtility::maxLangPoints - i ) );
   }
   // To avoid selecting english before any prefered language put it
   // in the preferd list, last with lower points than prefered.
   m.insert( make_pair( LangTypes::english, NameUtility::maxLangPoints - nbr ) );
   cont.insert( make_pair( lang, m ) );
}

inline NameUtility::PreferedLanguageCont initPrefredLanguage() {
   NameUtility::PreferedLanguageCont cont;
   typedef LangTypes LT;

   // Traditional Chinese
   const LT::language_t zht[] = { LT::chineseTradHongKong, LT::chinese };
   addPrefLang( cont, LT::chineseTraditional, zht, NBR_ITEMS( zht ) );

   // Traditional Chinese Hong Kong
   const LT::language_t zhh[] = { LT::chineseTraditional, LT::chinese };
   addPrefLang( cont, LT::chineseTradHongKong, zhh, NBR_ITEMS( zhh ) );

   // Chinese (simplified)
   const LT::language_t zh[] = { LT::chineseTraditional };
   addPrefLang( cont, LT::chinese, zh, NBR_ITEMS( zh ) );

   // French
   const LT::language_t fr[] = { LT::italian, LT::spanish, LT::portuguese };
   addPrefLang( cont, LT::french, fr, NBR_ITEMS( fr ) );

   // Italian
   const LT::language_t it[] = { LT::spanish, LT::french, LT::portuguese };
   addPrefLang( cont, LT::italian, it, NBR_ITEMS( it ) );

   // Spanish
   const LT::language_t sp[] = { LT::portuguese, LT::italian, LT::french };
   addPrefLang( cont, LT::spanish, sp, NBR_ITEMS( sp ) );

   // Portuguese
   const LT::language_t pt[] = { LT::spanish, LT::italian, LT::french };
   addPrefLang( cont, LT::portuguese, pt, NBR_ITEMS( pt ) );

   // WARNING: English is special see addPrefLang before adding it here

   return cont;
}

NameUtility::PreferedLanguageCont NameUtility::m_preferedLanguage =
initPrefredLanguage();


StringTable::languageCode 
NameUtility::getBestLanguage( 
   LangTypes::language_t lang, uint32& dist )
{
   bestLanguageCodeMap::const_iterator findIt = m_bestLanguageCodeMap.find(
      lang );
   if ( findIt == m_bestLanguageCodeMap.end() ) {
      // This will hopefully not happen, but to make sure
      // we just set the english language.
      // TODO: Check calls to this function and possible use exception here.
      dist = MAX_UINT32;
      return StringTable::ENGLISH;
   }

   dist = findIt->second.second;

   return findIt->second.first;
}

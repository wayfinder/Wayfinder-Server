/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"

#include "LangTypes.h"
#include "StringConvert.h"
#include "LangUtility.h"

#ifndef __SYMBIAN32__

#include <map>

// specialized templates for string conversion
// used in value conversion in nodes and attributes

namespace StringConvert {
template <>
LangTypes::language_t 
convert<LangTypes::language_t>( const MC2String& str )
throw ( StringConvert::ConvertException ) {
   
   LangTypes::language_t lang = LangUtility::getStringAsLanguage( str.c_str() );

   // ok, this will apprently not happen, since the default is "english"
   // but we keep it here for the future.
   if ( lang == LangTypes::invalidLanguage ) {
      throw StringConvert::
         ConvertException( MC2String( LangTypes::
                                      getLanguageAsString( lang ) ) + " : " + str);
   }

   return lang;
}
} // StringConvert

LangTypes::strMap_t LangTypes::c_stringAsLanguageShort = LangTypes::strMap_t();

LangTypes::strMap_t LangTypes::c_stringAsLanguageFull  = LangTypes::strMap_t();

/**
 * Initialize the map of strings as language. Done once.
 */
bool 
LangTypes::stringAsLanguageInitialized = LangTypes::initStringAsLanguage();

bool
LangTypes::initStringAsLanguage()
{   
   // Fill in the maps using the array.
   for (uint32 i = 0; i < LangTypes::nbrLanguages; i++) {
         c_stringAsLanguageShort.insert(
            make_pair(languageAsString[i][0], LangTypes::language_t(i)));
         c_stringAsLanguageFull.insert(
            make_pair(languageAsString[i][1], LangTypes::language_t(i)));
         
   }
   return true;
}

#endif

#if defined (__GNUC__) && __GNUC__ > 2
# define USE_NEW_LOCALES
#else
# undef  USE_NEW_LOCALES
#endif

// Col 1: short name
// Col 2: long name
// Col 3: ISO 639-1 code
// Col 4: ISO 639-3 code 
// see http://www.sil.org/iso639-3/
// Col 5: preferred UTF-8 locale 
// Col 6: ISO 639-3 code and Optional country dialect in ISO 3166.
// (like ENG_USA or por_bra).
//
// The locale may be found by using the 'locale -a' command in linux.
// Which locales are available differ depending on the version of the
// GNU toolchain, see the USE_NEW_LOCALES macro. The locale is the
// ISO_639-1 code of the language followed by an underscore and the
// ISO 3166-1 alpha 2
// (http://en.wikipedia.org/wiki/ISO_3166-1_alpha-2) code for the
// country. This should be followed by '.utf8'. There are exceptions
// to this pattern. Since we do not consider the language as different
// depending on where it is spoken, we have to pick a likely country
// and hope that the localization patterns are the same.
//
// The locale is only used for collation at the moment, but that might
// change.
//
// Note that the order must be the same as in LangTypes::language_t
// and that nbrLanguages must be last in the array.
// Only ASCII-characters are allowed. No åäö:s etc.
const char* const LangTypes::languageAsString[][6] =
   { {"eng", "english",                     "en", "eng", "en_GB.utf8", "eng"},
     {"swe", "swedish",                     "sv", "swe", "sv_SE.utf8", "swe"},
     {"ger", "german",                      "de", "ger", "de_DE.utf8", "ger"},
     {"dan", "danish",                      "da", "dan", "da_DK.utf8", "dan"},
     {"ita", "italian",                     "it", "ita", "it_IT.utf8", "ita"},
     {"dut", "dutch",                       "nl", "dut", "nl_NL.utf8", "dut"},
     {"spa", "spanish",                     "es", "spa", "es_ES.utf8", "spa"},
     {"fre", "french",                      "fr", "fre", "fr_FR.utf8", "fre"},
#ifdef USE_NEW_LOCALES
     {"wel", "welch",                       "cy", "wel", "cy_GB.utf8", "wel"},
#else
     {"wel", "welch",                       "cy", "wel", "en_GB.utf8", "wel"},//(11)
#endif
     {"fin", "finnish",                     "fi", "fin", "fi_FI.utf8", "fin"},
     {"nor", "norwegian",                   "no", "nor", "no_NO.utf8", "nor"},//(1)
     {"por", "portuguese",                  "pt", "por", "pt_PT.utf8", "por"},
     {"ame", "english (us)",                "us", "eng", "en_US.utf8", "eng_usa"},//(13) (14)
     {"cze", "czech",                       "cs", "cze", "cs_CZ.utf8", "cze"},
     {"alb", "albanian",                    "sq", "alb", "sq_AL.utf8", "alb"},
     {"baq", "basque",                      "eu", "baq", "eu_ES.utf8", "baq"},
     {"cat", "catalan",                     "ca", "cat", "ca_ES.utf8", "cat"},
     {"fry", "frisian",                     "fy", "fry", "nl_NL.utf8", "fry"},//(3)
     {"gle", "irish",                       "ga", "gai", "ga_IE.utf8", "gle"},
     {"glg", "galician",                    "gl", "glg", "gl_ES.utf8", "glg"},
     {"ltz", "letzeburgesch",               "lb", "ltz", "de_LU.utf8", "ltz"},//(4)
     {"roh", "raeto romance",               "rm", "roh", "it_CH.utf8", "roh"},//(5)
#ifdef USE_NEW_LOCALES
     {"scr", "serbo croatian",              "sh", "scr", "sr_CS.utf8", "scr"},//(7)
#else
     {"scr", "serbo croatian",              "sh", "scr", "sr_YU.utf8", "scr"},//(7)
#endif
     {"slv", "slovenian",                   "sl", "slv", "sl_SI.utf8", "slv"},
     {"val", "valencian",                   "",   "val", "es_ES.utf8", "val"},//(6)//(13)
     {"hun", "hungarian",                   "hu", "hun", "hu_HU.utf8", "hun"},
     {"gre", "greek",                       "el", "gre", "el_GR.utf8", "gre"},
     {"pol", "polish",                      "pl", "pol", "pl_PL.utf8", "pol"},
     {"slo", "slovak",                      "sk", "slo", "sk_SK.utf8", "slo"},
     {"rus", "russian",                     "ru", "rus", "en_GB.utf8", "rus"},
     {"grl", "greek latin syntax",          "",   "grl", "el_GR.utf8", "grl"},//(8)//(13)
     {"invalidLanguage", "invalidLanguage", "xx", "xxx", "en_GB.utf8", "xxx"},//(2)
     {"rul", "russian latin syntax",        "",   "rul", "en_GB.utf8", "rul"},//(8)//(13)
     {"tur", "turkish",                     "tr", "tur", "tr_TR.utf8", "tur"},
     {"ara", "arabic",                      "ar", "ara", "ar_AE.utf8", "ara"},//(9)
     {"chi", "chinese",                     "zh", "chi", "wf_zh_CN.utf8", "chi"},//(10)
     {"chl", "chinese latin syntax",        "",   "chl", "en_GB.utf8", "chl"},//(8)//(13)
     {"est", "estonian",                    "et", "est", "et_EE.utf8", "est"},
     {"lav", "latvian",                     "lv", "lav", "lv_LV.utf8", "lav"},
     {"lit", "lithuanian",                  "lt", "lit", "lt_LT.utf8", "lit"},
     {"tha", "thai",                        "th", "tha", "th_TH.utf8", "tha"},
     {"bul", "bulgarian",                   "bg", "bul", "bg_BG.utf8", "bul"},
     {"cyt", "cyrillic transcript",         "",   "cyt", "en_GB.utf8", "cyt"},// Don't use this one.
     {"ind", "indonesian",                  "id", "ind", "id_ID.utf8", "ind"},
     {"may", "malay",                       "ms", "may", "ms_MY.utf8", "may"},
     {"isl", "icelandic",                   "is", "isl", "is_IS.utf8", "isl"},
     {"jpn", "japanese",                    "ja", "jpn", "ja_JP.utf8", "jpa"},
     {"amh", "amharic",                     "am", "amh", "am_ET.utf8", "amh"},
     {"hye", "armenian",                    "hy", "hye", "hy_AM.utf8", "hye"},
     {"tgl", "tagalog",                     "tl", "tgl", "tl_PH.utf8", "tgl"},
     {"bel", "belarusian",                  "be", "bel", "be_BY.utf8", "bel"},
     {"ben", "bengali",                     "bn", "ben", "bn_BD.utf8", "ben"},
     {"mya", "burmese",                     "my", "mya", "my_MM.utf8", "mya"},
     {"hrv", "croatian",                    "hr", "hrv", "hr_HR.utf8", "hrv"},
     {"fas", "farsi",                       "fa", "fas", "fa_IR.utf8", "fas"},
     {"gla", "gaelic",                      "gd", "gla", "gd_UK.utf8", "gla"},
     {"kat", "georgian",                    "ka", "kat", "ka_GE.utf8", "kat"},
     {"guj", "gujarati",                    "gu", "guj", "gu_IN.utf8", "guj"},
     {"heb", "hebrew",                      "he", "heb", "he_IL.utf8", "heb"},
     {"hin", "hindi",                       "hi", "hin", "hi_IN.utf8", "hin"},
     {"kan", "kannada",                     "kn", "kan", "kn_IN.utf8", "kan"},
     {"kaz", "kazakh",                      "kk", "kaz", "kk_KZ.utf8", "kaz"},
     {"khm", "khmer",                       "km", "khm", "km_MM.utf8", "khm"},
     {"kor", "korean",                      "ko", "kor", "ko_KP.utf8", "kor"},
     {"lao", "lao",                         "lo", "lao", "lo_LA.utf8", "lao"},
     {"mkd", "macedonian",                  "mk", "mkd", "mk_MK.utf8", "mkd"},
     {"mal", "malayalam",                   "ml", "mal", "ml_IN.utf8", "mal"},
     {"mar", "marathi",                     "mr", "mar", "mr_IN.utf8", "mar"},
     {"mol", "moldavian",                   "mo", "mol", "mo_MD.utf8", "mol"},
     {"mon", "mongolian",                   "mn", "mon", "mn_MN.utf8", "mon"},
     {"pan", "punjabi",                     "pa", "pan", "pa_PK.utf8", "pan"},
     {"ron", "romanian",                    "ro", "ron", "ro_RO.utf8", "ron"},
     {"srp", "serbian",                     "sr", "srp", "sr_RS.utf8", "srp"},
     {"sin", "sinhalese",                   "si", "sin", "si_LK.utf8", "sin"},
     {"som", "somali",                      "so", "som", "so_SO.utf8", "som"},
     {"swa", "swahili",                     "sw", "swa", "sw_TZ.utf8", "swa"},
     {"tam", "tamil",                       "ta", "tam", "ta_IN.utf8", "tam"},
     {"tel", "telugu",                      "te", "tel", "te_IN.utf8", "tel"},
     {"bod", "tibetan",                     "bo", "bod", "bo_CN.utf8", "bod"},
     {"tir", "tigrinya",                    "ti", "tir", "ti_ER.utf8", "tir"},
     {"tuk", "turkmen",                     "tk", "tuk", "tk_TM.utf8", "tuk"},
     {"ukr", "ukrainian",                   "uk", "ukr", "uk_UA.utf8", "ukr"},
     {"urd", "urdu",                        "ur", "urd", "ur_PK.utf8", "urd"},
     {"vie", "vietnamese",                  "vi", "vie", "vi_VN.utf8", "vie"},
     {"zul", "zulu",                        "zu", "zul", "zu_ZA.utf8", "zul"},
     {"sot", "sesotho",                     "st", "sot", "st_ZA.utf8", "sot"},
     {"bun", "bulgarian latin syntax",      "",   "bun", "en_GB.utf8", "bun"},
     {"bos", "bosnian",                     "",   "bos", "en_GB.utf8", "bos"},
     {"sla", "slavic",                      "",   "sla", "en_GB.utf8", "sla"},
     {"bet", "belarusianLatinStx",          "",   "bet", "en_GB.utf8", "bet"},
     {"mat", "macedonianLatinStx",          "",   "mat", "en_GB.utf8", "mat"},
     {"scc", "serbianLatinStx",             "",   "scc", "en_GB.utf8", "scc"},
     {"ukl", "ukrainianLatinStx",           "",   "ukl", "en_GB.utf8", "ukl"},
     {"mlt", "maltese",                     "",   "mlt", "en_GB.utf8", "mlt"},
     {"zht", "chinese traditional",    "zh-hant", "zht", "wf_zh_TW.utf8", "zht"},//15
     {"zhh", "chinese trad. hong kong","zh-hant-hk","zhh","zh_HK.utf8","zhh"},//15
     {"thl", "thaiLatinStx",                "",   "thl", "en_GB.utf8", "thl"},

     // Add new languages here, also update
     // - POIObject.pm:  m_nameLang, set in the sub getNameLang
     // - WASP database: POINameLanguages
     // - ItemTypes.cpp: ItemTypes::getLanguageTypeAsLanguageCode

     {"nbrLanguages", "nbrLanguages",       "yy", "yyy", "en_GB.utf8"},//(2)

     // Non ISO 639-2 codes present above: grl, rul, chl, cyt.

     //(1) Bokmål. Use nn_No.utf8 for nynorsk.
     //(2) even invalid languages may have a valid locale.
     //(3) 'fy' refers to west frisian, a language used in northern
     //    Netherlands
     //(4) related to german, spoken in Luxembourg.
     //(5) Related to italian, poken in Switzerland.
     //(6) According to wikipedia Valencian is a Catalan dialect. It
     //    also lists the iso codes as "ca" and "cat". We use the
     //    Catalan locale.
     //(7) Serbo-croatian is, while still listed as the official
     //    language of Serbia-Montenegro, now a non language. Serbs
     //    speak serbian and Croatians croatian. The locale is set as
     //    sr_CS which is serbian in Serbia-Montenegro. Since
     //    Montenegro want's to separate, who knows what will happen.
     //    The Redhat 7.3 system has sr_YU.utf-8 instead. It's an
     //    ooold system.
     //(8) We guess that the same locale as the ordinary syntax is ok.
     //(9) We use the Arabic Emirate (AE) as the country, even though
     //    there are several locales for arabic listing different
     //    countries.
     //(10) We chose CN (Peoples Republic of China) over TW (Taiwan),
     //     SG (Singapore), and HK (Hong Kong), sorts according to pinyin.
     //(11) No utf welch locale on redhat 7.3
     //(13) WARNING: Not in iso-639-2!!!
     //(14) WARNING: Not in iso-639-1!!!
     //(15) Chinese traditional mostly for Taiwan. Uses zhuyin for sorting,
     //     this is our own locale built from a table from 
     //     Mimer Information Technology AB.
     //     Hong kong uses own sorting.
   };


const char*
LangTypes::getLanguageAsString(LangTypes::language_t langType, 
                               bool fullName)
{
   // Check the array size and add 1 since nbrLanguages is also in list.
   CHECK_ARRAY_SIZE( languageAsString, LangTypes::nbrLanguages + 1);
   if (langType < LangTypes::nbrLanguages) {
      return languageAsString[langType][fullName];
   } else {
      return languageAsString[LangTypes::invalidLanguage][fullName];
   }
}


const char* 
LangTypes::getLanguageAsISO639( LangTypes::language_t langType, bool two )
{
   if ( langType > LangTypes::nbrLanguages ) {
      langType = LangTypes::invalidLanguage;
   }
   return languageAsString[ langType ][ 2 + (two?0:1) ];
}

LangTypes::language_t
LangTypes::getISO639AsLanguage( const char* langType )
{
   int nbrLangs = LangTypes::nbrLanguages;

   int nbrChars = strlen( langType ); 
   
   // Only allow two or three letters.
   if ( nbrChars != 2 && nbrChars != 3 ) {
      return LangTypes::invalidLanguage;  
   }
   
   // The offset is the same as nbr of characters. 
   int offset = nbrChars;
  
   for( int i = 0; i < nbrLangs; ++i ) {
      if ( strcasecmp(languageAsString[i][offset], langType) == 0 ) {
         // Found a match - return
         return LangTypes::language_t(i);
      }
   }
   return LangTypes::invalidLanguage;  
}

LangTypes::language_t
LangTypes::getISO639AsLanguage( const char* isoStr, bool dashOne ) {
   int offset = dashOne ? 2 : 3;
  
   for ( int i = 0 ; i < LangTypes::nbrLanguages ; ++i ) {
      if ( strcasecmp( languageAsString[ i ][ offset ], isoStr ) == 0 ) {
         // Found a match - return
         return LangTypes::language_t( i );
      }
   }
   return LangTypes::invalidLanguage;  
}

const char* 
LangTypes::getLanguageAsISO639AndDialect( LangTypes::language_t langType )
{
   if ( langType > LangTypes::nbrLanguages ) {
      langType = LangTypes::invalidLanguage;
   }
   return languageAsString[ langType ][ 5 ];
}

LangTypes::language_t
LangTypes::getISO639AndDialectAsLanguage( const char* langType )
{
   for( int i = 0 ; i < LangTypes::nbrLanguages ; ++i ) {
      if ( strcasecmp( languageAsString[ i ][ 5 ], langType ) == 0 ) {
         // Found a match - return
         return LangTypes::language_t( i );
      }
   }
   return LangTypes::invalidLanguage;  
}


LangTypes::language_t
LangTypes::getStringAsLanguage(const char* langType, bool fullName)
{
#ifndef __SYMBIAN32__
   const char* cmpString = langType;

   // Get the right map for the job
   const strMap_t& stringAsLanguage
      = fullName ? c_stringAsLanguageFull : c_stringAsLanguageShort;

   // Look in the map
   strMap_t::const_iterator it =
      stringAsLanguage.find(cmpString);
   
   if (it != stringAsLanguage.end()) {
      return it->second;
   } else {
      return LangTypes::invalidLanguage;
   }
#else
   int nbrLangs = LangTypes::nbrLanguages;
   // Use offset 1 for fullname and 0 for short name.
   int offset = fullName ? 1 : 0;
   for( int i = 0; i < nbrLangs; ++i ) {
      if ( strcasecmp(languageAsString[i][offset], langType) == 0 ) {
         // Found a match - return
         return LangTypes::language_t(i);
      }
   }
   return LangTypes::invalidLanguage;
#endif
}


LangTypes::language_t 
LangTypes::getNavLangAsLanguage( uint32 language ) {
   switch( language ) {
      case 0x00:
         return english;
      case 0x01:
         return swedish;
      case 0x02:
         return german;
      case 0x03:
         return danish;
      case 0x04:
         return finnish;
      case 0x05:
         return norwegian;
      case 0x06:
         return italian;
      case 0x07:
         return dutch;
      case 0x08:
         return spanish;
      case 0x09:
         return french;
      case 0x0a:
         return welch;
      case 0x0b:
         return portuguese;
      case 0x0c:
         return czech;
      case 0x0d:
         return american;
      case 0x0e:
         return hungarian;
      case 0x0f:
         return greek;
      case 0x10:
         return polish;
      case 0x11:
         return slovak;
      case 0x12:
         return russian;
      case 0x13:
         return slovenian;
      case 0x14:
         return turkish;
      case 0x15:
         return arabic;

      case 0x16 : // SWISS_FRENCH
         return french;
      case 0x17 : // SWISS_GERMAN
         return german;
      case 0x18 : // ICELANDIC
         return icelandic;
      case 0x19 : // BELGIAN_FLEMISH
         return dutch;   // FIXME: Update when lang avail.
      case 0x1a : // AUSTRALIAN_ENGLISH
         return english;
      case 0x1b : // BELGIAN_FRENCH
         return french;
      case 0x1c : // AUSTRIAN_GERMAN
         return german;
      case 0x1d : // NEW_ZEALAND_ENGLISH
         return english;
      case 0x1e : // CHINESE_TAIWAN
         return chineseTraditional;
      case 0x1f : // CHINESE_HONG_KONG
         return chineseTradHongKong;
      case 0x20 : //CHINESE_PRC (People's Republic of China)
         return chinese;
      case 0x21 : // JAPANESE
         return japanese;
      case 0x22 : // THAI
         return thai;
      case 0x23 : // AFRIKAANS 
         return dutch;   // FIXME: Update when lang avail.
      case 0x24 : // ALBANIAN
         return albanian;
      case 0x25 : // AMHARIC
         return amharic;
      case 0x26 : // ARMENIAN
         return armenian;
      case 0x27 : // TAGALOG
         return tagalog;
      case 0x28 : // BELARUSIAN
         return belarusian;
      case 0x29 : // BENGALI
         return bengali;
      case 0x2a : // BULGARIAN
         return bulgarian;
      case 0x2b : // BURMESE
         return burmese;
      case 0x2c : // CATALAN
         return catalan;
      case 0x2d : // CROATIAN
         return croatian;
      case 0x2e : // CANADIAN_ENGLISH
         return english;
      case 0x2f : // SOUTH_AFRICAN_ENGLISH
         return english;
      case 0x30 : // ESTONIAN
         return estonian;
      case 0x31 : // FARSI
         return farsi;
      case 0x32 : // CANADIAN_FRENCH
         return french;
      case 0x33 : // GAELIC
         return gaelic;
      case 0x34 : // GEORGIAN
         return georgian;
      case 0x35 : // GREEK_CYPRUS
         return greek;
      case 0x36 : // GUJARATI
         return gujarati;
      case 0x37 : // HEBREW
         return hebrew;
      case 0x38 : // HINDI
         return hindi;
      case 0x39 : // INDONESIAN
         return indonesian;
      case 0x3a : // IRISH
         return irish;
      case 0x3b : // SWISS_ITALIAN
         return italian;
      case 0x3c : // KANNADA (Indian language)
         return kannada;
      case 0x3d : // KAZAKH
         return kazakh;
      case 0x3e : // KHMER
         return khmer;
      case 0x3f : // KOREAN
         return korean;
      case 0x40 : // LAO
         return lao;
      case 0x41 : // LATVIAN
         return latvian;
      case 0x42 : // LITHUANIAN
         return lithuanian;
      case 0x43 : // MACEDONIAN
         return macedonian;
      case 0x44 : // MALAY
         return malay;
      case 0x45 : // MALAYALAM
         return malayalam;
      case 0x46 : // MARATHI
         return marathi;
      case 0x47 : // MOLDAVIAN
         return moldavian;
      case 0x48 : // MONGOLIAN
         return mongolian;
      case 0x49 : // NYNORSK
         return norwegian; // FIXME: Update when lang avail.
      case 0x4a : // BRAZILIAN_PORTUGUESE
         return portuguese; // FIXME: Update when lang avail.
      case 0x4b : // PUNJABI
         return punjabi;
      case 0x4c : // ROMANIAN
         return romanian;
      case 0x4d : // SERBIAN
         return serbian;
      case 0x4e : // SINHALESE
         return sinhalese;
      case 0x4f : // SOMALI
         return somali;
      case 0x50 : // LATIN_AMERICAN_SPANISH
         return spanish; // FIXME: Update when lang avail.
      case 0x51 : // SWAHILI
         return swahili;
      case 0x52 : // FINNISH_SWEDISH
         return swedish;
      case 0x53 : // TAMIL
         return tamil;
      case 0x54 : // TELUGU
         return telugu;
      case 0x55 : // TIBETAN
         return tibetan;
      case 0x56 : // TIGRINYA
         return tigrinya;
      case 0x57 : // CYPRUS_TURKISH
         return turkish;
      case 0x58 : // TURKMEN
         return turkmen;
      case 0x59 : // UKRAINIAN
         return ukrainian;
      case 0x5a : // URDU
         return urdu;
      case 0x5b : // VIETNAMESE
         return vietnamese;
      case 0x5c : // ZULU
         return zulu;
      case 0x5d : // SESOTHO
         return sesotho;
      case 0x5e : // BASQUE
         return basque;
      case 0x5f : // GALICIAN
         return galician;
      case 0x60 : // ASIA_PACIFIC_ENGLISH
         return english; // FIXME: Update when lang avail.
      case 0x61 : // TAIWAN_ENGLISH
         return english; // FIXME: Update when lang avail.
      case 0x62 : // HONG_KONG_ENGLISH
         return english; // FIXME: Update when lang avail.
      case 0x63 : // CHINA_ENGLISH
         return english; // FIXME: Update when lang avail.
      case 0x64 : // JAPAN_ENGLISH
         return english; // FIXME: Update when lang avail.
      case 0x65 : // THAI_ENGLISH
         return english; // FIXME: Update when lang avail.
      case 0x66 : // ASIA_PACIFIC_MALAY
         return malay; // FIXME: Update when lang avail.
      case 0x67 : // BOSNIAN
         return bosnian;
      case 0x68 : // MALTESE
         return maltese;
      case 0x69 : // CHINESE_TRADITIONAL
         return chineseTraditional;

      default:
         mc2log << warn << "LangTypes::getNavLangAsLanguage unknwon "
                << "language " << int(language) << " using english."
                << endl;
         return english;
   }

}

const char* LangTypes::getUtf8LocaleAsString(LangTypes::language_t langType)
{
   if ( langType > LangTypes::nbrLanguages ) {
      langType = LangTypes::invalidLanguage;
   }
   return languageAsString[ langType ][ 4 ];   
}




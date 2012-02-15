/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "StringTable.h"
#include "StringUtility.h"
#include "Utility.h"
#include "UTF8Util.h"
#include "NameUtility.h"
#include "LangTypes.h"


#ifndef MC2_UTF8

vector<vector<MC2String> >
StringTable::latinStrings;

bool
StringTable::initLatinStrings()
{
   latinStrings.resize( nbrStrings );
   for(uint32 i = 0; i < nbrStrings; ++i ) {
      latinStrings[i].resize( nbrLanguages * 2 );
   }
   for(uint32 i = 0; i < nbrStrings; ++i ) {
      for(uint32 j = 0; j < nbrLanguages; ++j ) {
         if( strings[i][j] != NULL ) {
            latinStrings[i][j] =
               UTF8Util::utf8ToIso( strings[i][j] );
         } else {
            latinStrings[i][j] = "";
         }
      }
   }
   
   return true;
}

bool
StringTable::initiated = initLatinStrings();

#endif

const char*
StringTable::getString(stringCode sc, languageCode lc) {
#ifdef MC2_UTF8
   // Check arguments and crash
   MC2_ASSERT( sc < StringTable::NBR_STRINGS );
   MC2_ASSERT( lc < languageCode(nbrLanguages + nbrPseudoLanguages) );
   
   // Try correct language.
   const char* result = strings[sc][lc];
   
   // Found something
   if ( result ) return result;
   
   if( lc > SMSISH_ENG ) {
      // Try real language
      result = strings[sc][lc - SMSISH_ENG];
      // Found something
      if ( result ) return result;
      // Try SMSish english
      result = strings[sc][SMSISH_ENG];
      // Found something
      if ( result ) return result;         
   }
   
   result = strings[sc][ENGLISH]; // English must be there
   
   MC2_ASSERT( result != NULL );
   
   return result;
#else
   if ( (sc < StringTable::NBR_STRINGS) &&
        (lc < languageCode(nbrLanguages + nbrPseudoLanguages)) )
   {
      MC2String* result = &latinStrings[sc][lc];
      if((result->size() == 0) && (lc > SMSISH_ENG))
         result = &latinStrings[sc][SMSISH_ENG];
      else if((result->size() == 0) && (lc > ENGLISH) && (lc <= SMSISH_ENG))
         result = &latinStrings[sc][ENGLISH];
      almostAssert(result->size() != 0);
      return result->c_str();
   } else {
      almostAssert(false); // someone supplied incorrect arguments
      return (NULL);
   }
#endif
}

const char*
StringTable::getString( stringCode code,
                        const LangType& lang,
                        bool smsish )
{
   uint32 langPoints;
   StringTable::languageCode strLang =
      NameUtility::getBestLanguage( lang,
                                    langPoints);
   if ( smsish ) {
      strLang = StringTable::languageCode( strLang + nbrLanguages );
   }
   return StringTable::getString( code, strLang );
}


const StringTable::languageCode* 
StringTable::getLanguages() {
   static StringTable::languageCode languages[] = 
   {StringTable::ENGLISH,
    StringTable::SWEDISH,
    StringTable::GERMAN,
    StringTable::DANISH,
    StringTable::FINNISH,
    StringTable::NORWEGIAN,
    StringTable::FRENCH,
    StringTable::SPANISH,
    StringTable::ITALIAN,
    StringTable::DUTCH,
    StringTable::PORTUGUESE,
    StringTable::AMERICAN_ENGLISH,
    StringTable::HUNGARIAN,
    StringTable::CZECH,
    StringTable::GREEK,
    StringTable::POLISH,
    StringTable::SLOVAK,
    StringTable::RUSSIAN,
    StringTable::SLOVENIAN,
    StringTable::TURKISH,
    StringTable::CHINESE,
    StringTable::CHINESE_TRADITIONAL,
   };

   CHECK_ARRAY_SIZE( languages, nbrLanguages );
   
   return languages;
}


const char**
StringTable::getLanguagesAsString() {
   static const char* langugageStrings[] = {
      strings[ StringTable::ENGLISH_STRING  ][ StringTable::ENGLISH  ], 
      strings[ StringTable::SWEDISH_STRING  ][ StringTable::SWEDISH  ],
      strings[ StringTable::GERMAN_STRING   ][ StringTable::GERMAN   ],
      strings[ StringTable::DANISH_STRING   ][ StringTable::DANISH   ], 
      strings[ StringTable::FINNISH_STRING  ][ StringTable::FINNISH  ],
      strings[ StringTable::NORWEGIAN_STRING][ StringTable::NORWEGIAN],
      strings[ StringTable::FRENCH_STRING   ][ StringTable::FRENCH  ],
      strings[ StringTable::SPANISH_STRING  ][ StringTable::SPANISH  ],
      strings[ StringTable::ITALIAN_STRING  ][ StringTable::ITALIAN  ],
      strings[ StringTable::DUTCH_STRING    ][ StringTable::DUTCH  ],
      strings[ StringTable::PORTUGUESE_STRING ][ StringTable::PORTUGUESE ],
      strings[StringTable::AMERICAN_STRING][StringTable::AMERICAN_ENGLISH],
      strings[ StringTable::HUNGARIAN_STRING  ][ StringTable::HUNGARIAN],
      strings[ StringTable::CZECH_STRING  ][ StringTable::CZECH ],
      strings[ StringTable::GREEK_STRING    ][ StringTable::GREEK ],
      strings[ StringTable::POLISH_STRING ][ StringTable::POLISH ],
      strings[ StringTable::SLOVAK_STRING  ][ StringTable::SLOVAK],
      strings[ StringTable::RUSSIAN_STRING  ][ StringTable::RUSSIAN],
      strings[ StringTable::SLOVENIAN_STRING  ][ StringTable::SLOVENIAN],
      strings[ StringTable::TURKISH_STRING  ][ StringTable::TURKISH],
      strings[ StringTable::CHINESE_STRING  ][ StringTable::CHINESE],
      strings[ StringTable::CHINESE_STRING  ][ StringTable::CHINESE_TRADITIONAL ],
   };

   CHECK_ARRAY_SIZE( langugageStrings, nbrLanguages );
   
   return langugageStrings;
}

StringTable::stringCode
StringTable::getLanguageAsStringCode(StringTable::languageCode lc) 
{
   switch (lc) {
      case StringTable::ENGLISH :
      case StringTable::SMSISH_ENG :
         return StringTable::ENGLISH_STRING;
      case StringTable::SWEDISH :
      case StringTable::SMSISH_SWE :
         return StringTable::SWEDISH_STRING;
      case StringTable::GERMAN :
      case StringTable::SMSISH_GER :
         return StringTable::GERMAN_STRING;
      case StringTable::DANISH :
      case StringTable::SMSISH_DAN :
         return StringTable::DANISH_STRING;
      case StringTable::FINNISH :
      case StringTable::SMSISH_FIN :
         return StringTable::FINNISH_STRING;
      case StringTable::NORWEGIAN :
      case StringTable::SMSISH_NOR :
         return StringTable::NORWEGIAN_STRING;
      case StringTable::FRENCH :
      case StringTable::SMSISH_FRA :
         return StringTable::FRENCH_STRING;
      case StringTable::SPANISH :
      case StringTable::SMSISH_SPA :
         return StringTable::SPANISH_STRING;
      case StringTable::ITALIAN :
      case StringTable::SMSISH_ITA :
         return StringTable::ITALIAN_STRING;
      case StringTable::DUTCH :
      case StringTable::SMSISH_DUT :
         return StringTable::DUTCH_STRING;
      case StringTable::PORTUGUESE:
      case StringTable::SMSISH_POR :
         return StringTable::PORTUGUESE_STRING;
      case StringTable::AMERICAN_ENGLISH :
      case StringTable::SMSISH_AME :
         return StringTable::AMERICAN_STRING;
      case StringTable::HUNGARIAN :
      case StringTable::SMSISH_HUN :
         return StringTable::HUNGARIAN_STRING;
      case StringTable::CZECH :
      case StringTable::SMSISH_CZE :
         return StringTable::CZECH_STRING;
      case StringTable::GREEK : 
      case StringTable::SMSISH_GRE : 
         return StringTable::GREEK_STRING;
      case StringTable::POLISH :
      case StringTable::SMSISH_POL :
         return StringTable::POLISH_STRING;
      case StringTable::SLOVAK :
      case StringTable::SMSISH_SVK :
         return StringTable::SLOVAK_STRING;
      case StringTable::RUSSIAN :
      case StringTable::SMSISH_RUS :
         return StringTable::RUSSIAN_STRING;
      case StringTable::SLOVENIAN :
      case StringTable::SMSISH_SLV :
         return StringTable::SLOVENIAN_STRING;
      case StringTable::TURKISH :
      case StringTable::SMSISH_TUR :
         return StringTable::TURKISH_STRING;
      case StringTable::CHINESE :
      case StringTable::SMSISH_CHI :
         return StringTable::CHINESE_STRING;
      case StringTable::CHINESE_TRADITIONAL :
      case StringTable::SMSISH_ZHT :
         return StringTable::CHINESE_STRING;
   }
   // We don't get here - at least not with the next compiler
   return StringTable::UNKNOWN;
}


bool 
StringTable::isLanguage( StringTable::languageCode lc ) 
{
   bool is = false;

   for ( uint32 i = 0 ; i < getNbrLanguages() ; i++ ) {
      if ( getLanguages()[i] == lc ) {
         is = true;
         break;
      }
   }
   return is;
}


const StringTable::stringCode*
StringTable::getLanguagesAsStringCode() {
   static StringTable::stringCode languages[] = 
   {StringTable::ENGLISH_STRING,
    StringTable::SWEDISH_STRING,
    StringTable::GERMAN_STRING,
    StringTable::DANISH_STRING,
    StringTable::FINNISH_STRING,
    StringTable::NORWEGIAN_STRING,
    StringTable::FRENCH_STRING,
    StringTable::SPANISH_STRING,
    StringTable::ITALIAN_STRING,
    StringTable::DUTCH_STRING,
    StringTable::PORTUGUESE_STRING,
    StringTable::AMERICAN_STRING,
    StringTable::HUNGARIAN_STRING,
    StringTable::CZECH_STRING,
    StringTable::GREEK_STRING,
    StringTable::POLISH_STRING,
    StringTable::SLOVAK_STRING,
    StringTable::RUSSIAN_STRING,
    StringTable::SLOVENIAN_STRING,
    StringTable::TURKISH_STRING,
    StringTable::CHINESE_STRING,
    StringTable::CHINESE_STRING,
   };

   CHECK_ARRAY_SIZE( languages, nbrLanguages );
     
   return languages;
}


const StringTable::languageCode* 
StringTable::getPseudoLanguages() {
   static StringTable::languageCode pseudoLanguages[] =
   {StringTable::SMSISH_ENG,
    StringTable::SMSISH_SWE,
    StringTable::SMSISH_GER,
    StringTable::SMSISH_DAN,
    StringTable::SMSISH_FIN,
    StringTable::SMSISH_NOR,
    StringTable::SMSISH_FRA,
    StringTable::SMSISH_SPA,
    StringTable::SMSISH_ITA,
    StringTable::SMSISH_DUT,
    StringTable::SMSISH_POR,
    StringTable::SMSISH_AME,
    StringTable::SMSISH_HUN,
    StringTable::SMSISH_CZE,
    StringTable::SMSISH_GRE,
    StringTable::SMSISH_POL,
    StringTable::SMSISH_SVK,
    StringTable::SMSISH_RUS,
    StringTable::SMSISH_SLV,
    StringTable::SMSISH_TUR,
    StringTable::SMSISH_CHI,
    StringTable::SMSISH_ZHT,
   };
   
   CHECK_ARRAY_SIZE( pseudoLanguages, nbrPseudoLanguages );

   return pseudoLanguages;
}


bool 
StringTable::isShortLangugage( StringTable::languageCode lc ) {
   bool is = false;

   for ( uint32 i = 0 ; i < getNbrPseudoLanguages() ; i++ ) {
      if ( getPseudoLanguages()[i] == lc ) {
         is = true;
         break;
      }
   }
   return is;
}


const StringTable::languageCode 
StringTable::getLanguageCode( const char* language,
                              StringTable::languageCode defaultCode )
{
   StringTable::languageCode languageCode = defaultCode;

   const StringTable::languageCode* langs = StringTable::getLanguages();
   const StringTable::stringCode* l_strings = 
      StringTable::getLanguagesAsStringCode();

   MC2String langStr = StringUtility::trimStartEnd( language );
   
   int nbrLanguages = StringTable::getNbrLanguages();

   for ( int32 i = 0 ; i < nbrLanguages ; i++ ) {
      for ( int32 j = 0 ; j < nbrLanguages ; j++ ) {
         const char* tmpLang = StringTable::strings[l_strings[i]][ langs[j] ];
         if ( tmpLang == NULL ) {           
            continue;
         }
         if ( StringUtility::strcasecmp( langStr, tmpLang) == 0 ) {
            languageCode = langs[i];
            return languageCode;
         }
      }
   }
   return languageCode;
}

uint32 
StringTable::getRouteDescription(stringCode sc, 
                                 languageCode lc, 
                                 uint32 exitCount,
                                 char* dest, 
                                 int maxSize,
                                 bool hasName,
                                 bool endOfRoad)
{
   // Check the maxSize
   if (maxSize < 2) {
      return (0);
   }
   // Check if the sc is LEFT or RIGHT turn
   if (((sc != LEFT_TURN) && 
        (sc != RIGHT_TURN) && 
        (sc != EXIT_ROUNDABOUT_TURN)) ||
       (exitCount == 0)) {
      
      // Special case when starting.
      // Either: "Start north AT Baravägen" (if roadname present)
      // Or:     "Start north" (if no roadname present), ie. add no text.
      if (sc == DRIVE_START) {
         if (hasName) {
            strncpy(dest, getString(DIR_AT, lc), maxSize - 1);
            return (strlen(dest));
         } else {
            strncpy(dest, "", maxSize - 1);
         }
         return (strlen(dest));
      }
      // Add route phrase
      if(endOfRoad){
         // End of road turn  (The turn is Left or Right).
         // Use ENDOFROAD_LEFT_TURN, or ENDOFROAD_RIGHT_TURN
         if(sc == LEFT_TURN){
            strncpy(dest, getString(ENDOFROAD_LEFT_TURN, lc), 
                    maxSize - 1);
         } else {
            strncpy(dest, getString(ENDOFROAD_RIGHT_TURN, lc), 
                    maxSize - 1);
         }
         exitCount = 0; // if not made before.
      } else{
         // Normal case.
         const char* src = getString(sc, lc);
         strncpy(dest, src, maxSize - 1);
      }
      
      
      if (hasName) {
         // A road name will follow, which means the proper
         // preposition must be added. "then turn left INTO baravägen"
         if ((sc == OFF_RAMP_TURN) || (sc == LEFT_OFF_RAMP_TURN) ||
             (sc == RIGHT_OFF_RAMP_TURN)){
            strncat(dest, getString(PREPOSITION_POST_OFF_RAMP, lc),
               maxSize - 1 - strlen(dest));
         } else if (sc == PARK_CAR) {
            strncat(dest, getString(PREPOSITION_POST_PARK_CAR, lc), 
               maxSize - 1 - strlen(dest));
         } else if (sc == PARK_BIKE) {
            strncat(dest, getString(PREPOSITION_POST_PARK_BIKE, lc),
               maxSize - 1 - strlen(dest));
         } else if (sc == DRIVE_START ) {
            // "Start AT Baravägen"
            strncat(dest, getString(DIR_AT, lc), 
               maxSize - 1 - strlen(dest));
         } else if (sc == DRIVE_FINALLY) {
            // "fram till"
            strncat(dest, getString(PREPOSITION_POST_DRIVE_FINALLY, lc),
               maxSize - 1 - strlen(dest));
         } else if (sc == DRIVE_START_WITH_UTURN) {
            strncat(dest, getString(PREPOSITION_U_TURN, lc),
                    maxSize - 1 - strlen(dest));
         } else if ((sc != ENTER_FERRY_TURN) && (sc != CHANGE_FERRY_TURN)){
            // Use standard turn preposition
            strncat(dest, getString(PREPOSITION_POST_TURN, lc), 
               maxSize - 1 - strlen(dest));
         }
      } else if ((sc == OFF_RAMP_TURN) || (sc == LEFT_OFF_RAMP_TURN) ||
                 (sc == RIGHT_OFF_RAMP_TURN)) {
         // No roadname of the ramp.
         // "then drive into ramp"
         // strncat(dest, getString(PREPOSITION_POST_OFF_RAMP, lc),
         //    maxSize - 1 - strlen(dest));
         //strncat(dest, getString(RAMP, lc), maxSize - 1 - strlen(dest));
      }

      return (strlen(dest));
   }
   
   stringCode pre = stringCode(sc+1);
   stringCode post = stringCode(sc+2);

   int nbrWrittenBytes = 0;
   
   // The string that is before "X:th"
   if (getString(pre, lc) > 0) {
      strncpy(dest, getString(pre, lc), maxSize-1); 
      nbrWrittenBytes = MIN(strlen(getString(sc, lc)), (uint32) maxSize);
      if (nbrWrittenBytes > maxSize-2) {
         dest[nbrWrittenBytes] = '\0';
         return(nbrWrittenBytes);
      }
   }

   // The serial number
   char tmpStr[25];
   tmpStr[ 0 ] = 0;
   int serialSize = 0;
   if ( sc == EXIT_ROUNDABOUT_TURN && isShortLangugage( lc ) ) {
      serialSize = getSerialNumber( exitCount, 
                                    getNormalLanguageCode( lc ), 
                                    tmpStr );
   } else {
      serialSize = getSerialNumber(exitCount, lc, tmpStr);
   }
   if (serialSize + nbrWrittenBytes >= maxSize) {
      // The serial number does not fit into the destination string!
      dest[nbrWrittenBytes] = '\0';
      return(nbrWrittenBytes);
   }
   strcat(dest, tmpStr);
   nbrWrittenBytes += serialSize;
   
   // The string that is after the "X:th"
   if (getString(post, lc) > 0) {
      strncat(dest, getString(post, lc), maxSize-nbrWrittenBytes); 
      if (nbrWrittenBytes >= maxSize) {
         dest[maxSize] = '\0';
         return(maxSize);
      }
   }
   nbrWrittenBytes = strlen(dest);

   // In case of exit roundabout add preposition if the roadname is known
   // For instance: "then take the 1st exit from roundabout INTO Baravägen"
   if ((sc == EXIT_ROUNDABOUT_TURN) && (hasName)) {
      strncat(dest, getString(PREPOSITION_POST_TURN, lc),
         maxSize-nbrWrittenBytes); 
      if (nbrWrittenBytes >= maxSize) {
         dest[maxSize] = '\0';
         return(maxSize);
      }
   }
   
   return (strlen(dest));
}

uint32
StringTable::getSerialNumber(uint32 i, languageCode lc, char* dest)
{
   stringCode sc;
   switch (i) {
      case (1) :
         sc = SERIAL_EXTENSION_1;
      break;
      case (2) :
         sc = SERIAL_EXTENSION_2;
      break;
      case (3) :
         sc = SERIAL_EXTENSION_3;
      break;
      default :
         if(lc == NORWEGIAN){
            if((i == 4) || (( i > 6) && (i < 11))){
               sc = SERIAL_EXTENSION_ALT;
            }
            else {
               sc = SERIAL_EXTENSION_DEFAULT;
            }
         } else {
            sc = SERIAL_EXTENSION_DEFAULT;
         }
         
   };
   sprintf(dest, "%i%s", i, getString(sc, lc));
   return (strlen(dest));
}


const StringTable::languageCode 
StringTable::getShortLanguageCode( StringTable::languageCode code ) {
   if ( code < static_cast<int32>(nbrLanguages) )
      return StringTable::languageCode( code + nbrLanguages );
   else 
      return StringTable::SMSISH_ENG;
}


const StringTable::languageCode 
StringTable::getNormalLanguageCode( StringTable::languageCode code ) {
   if ( StringTable::languageCode(code - nbrPseudoLanguages) > 0 )
      return StringTable::languageCode( code - nbrPseudoLanguages );
   else 
      return code; 
}


const char*
StringTable::getMonthString( uint32 monthIndex, languageCode lc)
{
   if( (monthIndex > 0) && (monthIndex <= 12) ) {
      return getString( stringCode(monthIndex + uint32(MONTH_1) - 1), lc );
   }
   else {
      return getString( UNKNOWN, lc );
   }
}



const char*
StringTable::getWeekdayString( uint32 weekdayIndex, languageCode lc)
{
   if( (weekdayIndex >= 0) && (weekdayIndex <= 6) ) {
      return getString( stringCode(weekdayIndex + uint32(WEEKDAY_1)), lc );
   }
   else {
      return getString( UNKNOWN, lc );
   }
}


StringTable::languageCode 
StringTable::getLanguageFromPhonemumber( 
   const char* phoneNumber,
   StringTable::languageCode defaultLanguage )
{
   StringTable::languageCode lang = defaultLanguage;

   if ( strlen( phoneNumber ) > 2 ) {
      if ( strncmp(
         phoneNumber,
         StringTable::getCountryPhoneCode(StringTable::SWEDEN_CC),
         2 ) == 0 ) { // Sweden
         lang = StringTable::SWEDISH;
      } else if ( strncmp(
         phoneNumber,
         StringTable::getCountryPhoneCode(StringTable::GERMANY_CC),
         2 ) == 0 ) { // Germany
         lang = StringTable::GERMAN;
      }  else if ( strncmp(
         phoneNumber,
         StringTable::getCountryPhoneCode(StringTable::AUSTRIA_CC),
         2 ) == 0 ) { // Austria
         lang = StringTable::GERMAN;
      }
   }
   
   return lang;
}

const char*
StringTable::getCountryPhoneCode(StringTable::countryCode cc)
{
   const char* result = "46"; // default.
   if (cc < StringTable::NBR_COUNTRY_CODES) {
      result = StringTable::countryCodes[cc][0];
   } else {
      mc2log << error
             << "Error in getcountryphonecode, cc too large. cc=" << int(cc)
             << endl;
   }
   return result;
}

StringTable::stringCode
StringTable::getCountryStringCode(StringTable::countryCode cc)
{
   // The string code that will be returned
   StringTable::stringCode retCode = StringTable::UNKNOWN;

   // Switch on the country code to set the string code
   switch (cc) {
      case ENGLAND_CC :
         retCode = StringTable::ENGLAND;
         break;
      case SWEDEN_CC :
         retCode = StringTable::SWEDEN;
         break;
      case GERMANY_CC :
         retCode = StringTable::GERMANY;
         break;
      case DENMARK_CC :
         retCode = StringTable::DENMARK;
         break;
      case FINLAND_CC :
         retCode = StringTable::FINLAND;
         break;
      case NORWAY_CC :
         retCode = StringTable::NORWAY;
         break;
      case BELGIUM_CC : 
         retCode = StringTable::BELGIUM;
         break;
      case NETHERLANDS_CC :
         retCode = StringTable::NETHERLANDS;
         break;
      case LUXEMBOURG_CC :
         retCode = StringTable::LUXEMBOURG;
         break;
      case USA_CC :
         retCode = StringTable::USA;
         break;
      case SWITZERLAND_CC :
         retCode = StringTable::SWITZERLAND;         
         break;
      case AUSTRIA_CC :
         retCode = StringTable::AUSTRIA;         
         break;         
      case FRANCE_CC :
         retCode = StringTable::FRANCE;         
         break;         
      case SPAIN_CC :
         retCode = StringTable::SPAIN;         
         break;
      case ANDORRA_CC :
         retCode = StringTable::ANDORRA;         
         break;  
      case LIECHTENSTEIN_CC :
         retCode = StringTable::LIECHTENSTEIN;         
         break;           
      case ITALY_CC :
         retCode = StringTable::ITALY;         
         break;      
      case MONACO_CC :
         retCode = StringTable::MONACO;         
         break;
      case IRELAND_CC :
         retCode = StringTable::IRELAND;         
         break;
      case PORTUGAL_CC :
         retCode = StringTable::PORTUGAL;         
         break;
      case CANADA_CC :
         retCode = StringTable::CANADA;         
         break;
      case HUNGARY_CC :
         retCode = StringTable::HUNGARY;
         break;
      case CZECH_REPUBLIC_CC :
         retCode = StringTable::CZECH_REPUBLIC;
         break;
      case POLAND_CC :
         retCode = StringTable::POLAND;         
         break;
      case GREECE_CC :
         retCode = StringTable::GREECE;
         break;
      case ISRAEL_CC :
         retCode = StringTable::ISRAEL;
         break;
      case BRAZIL_CC :
         retCode = StringTable::BRAZIL;
         break;
      case SLOVAKIA_CC :
         retCode = StringTable::SLOVAKIA;
         break;
      case RUSSIA_CC :
         retCode = StringTable::RUSSIA;
         break;
      case TURKEY_CC :
         retCode = StringTable::TURKEY;
         break;
      case SLOVENIA_CC :
         retCode = StringTable::SLOVENIA;
         break;
      case BULGARIA_CC :
         retCode = StringTable::BULGARIA;
         break;
      case ROMANIA_CC :
         retCode = StringTable::ROMANIA;
         break;
      case UKRAINE_CC :
         retCode = StringTable::UKRAINE;
         break;
      case SERBIA_CC :
         retCode = StringTable::SERBIA_MONTENEGRO;
         break;
      case CROATIA_CC :
         retCode = StringTable::CROATIA;
         break;
      case BOSNIA_CC :
         retCode = StringTable::BOSNIA;
         break;
      case MOLDOVA_CC :
         retCode = StringTable::MOLDOVA;
         break;
      case MACEDONIA_CC :
         retCode = StringTable::MACEDONIA;
         break;
      case ESTONIA_CC :
         retCode = StringTable::ESTONIA;
         break;
      case LATVIA_CC :
         retCode = StringTable::LATVIA;
         break;
      case LITHUANIA_CC :
      retCode = StringTable::LITHUANIA;
         break;
      case BELARUS_CC :
         retCode = StringTable::BELARUS;
         break;
      case MALTA_CC :
         retCode = StringTable::MALTA;
         break;
      case CYPRUS_CC :
         retCode = StringTable::CYPRUS;
         break;
      case ICELAND_CC :
         retCode = StringTable::ICELAND;
         break;
      case HONG_KONG_CC :
         retCode = StringTable::HONG_KONG;
         break;
      case SINGAPORE_CC :
         retCode = StringTable::SINGAPORE;
         break;
      case AUSTRALIA_CC :
         retCode = StringTable::AUSTRALIA;
         break;
      case UAE_CC :
         retCode = StringTable::UNITED_ARAB_EMIRATES;
         break;
      case BAHRAIN_CC :
         retCode = StringTable::BAHRAIN;
         break;
      case AFGHANISTAN_CC :
         retCode = StringTable::AFGHANISTAN;
         break;
      case ALBANIA_CC :
         retCode = StringTable::ALBANIA;
         break;
      case ALGERIA_CC :
         retCode = StringTable::ALGERIA;
         break;
      case AMERICAN_SAMOA_CC :
         retCode = StringTable::AMERICAN_SAMOA;
         break;
      case ANGOLA_CC :
         retCode = StringTable::ANGOLA;
         break;
      case ANGUILLA_CC :
         retCode = StringTable::ANGUILLA;
         break;
      case ANTARCTICA_CC :
         retCode = StringTable::ANTARCTICA;
         break;
      case ANTIGUA_AND_BARBUDA_CC :
         retCode = StringTable::ANTIGUA_AND_BARBUDA;
         break;
      case ARGENTINA_CC :
         retCode = StringTable::ARGENTINA;
         break;
      case ARMENIA_CC :
         retCode = StringTable::ARMENIA;
         break;
      case ARUBA_CC :
         retCode = StringTable::ARUBA;
         break;
      case AZERBAIJAN_CC :
         retCode = StringTable::AZERBAIJAN;
         break;
      case BAHAMAS_CC :
         retCode = StringTable::BAHAMAS;
         break;
      case BANGLADESH_CC :
         retCode = StringTable::BANGLADESH;
         break;
      case BARBADOS_CC :
         retCode = StringTable::BARBADOS;
         break;
      case BELIZE_CC :
         retCode = StringTable::BELIZE;
         break;
      case BENIN_CC :
         retCode = StringTable::BENIN;
         break;
      case BERMUDA_CC :
         retCode = StringTable::BERMUDA;
         break;
      case BHUTAN_CC :
         retCode = StringTable::BHUTAN;
         break;
      case BOLIVIA_CC :
         retCode = StringTable::BOLIVIA;
         break;
      case BOTSWANA_CC :
         retCode = StringTable::BOTSWANA;
         break;
      case BRITISH_VIRGIN_ISLANDS_CC :
         retCode = StringTable::BRITISH_VIRGIN_ISLANDS;
         break;
      case BRUNEI_DARUSSALAM_CC :
         retCode = StringTable::BRUNEI_DARUSSALAM;
         break;
      case BURKINA_FASO_CC :
         retCode = StringTable::BURKINA_FASO;
         break;
      case BURUNDI_CC :
         retCode = StringTable::BURUNDI;
         break;
      case CAMBODIA_CC :
         retCode = StringTable::CAMBODIA;
         break;
      case CAMEROON_CC :
         retCode = StringTable::CAMEROON;
         break;
      case CAPE_VERDE_CC :
         retCode = StringTable::CAPE_VERDE;
         break;
      case CAYMAN_ISLANDS_CC :
         retCode = StringTable::CAYMAN_ISLANDS;
         break;
      case CENTRAL_AFRICAN_REPUBLIC_CC :
         retCode = StringTable::CENTRAL_AFRICAN_REPUBLIC;
         break;
      case CHAD_CC :
         retCode = StringTable::CHAD;
         break;
      case CHILE_CC :
         retCode = StringTable::CHILE;
         break;
      case CHINA_CC :
         retCode = StringTable::CHINA;
         break;
      case COLOMBIA_CC :
         retCode = StringTable::COLOMBIA;
         break;
      case COMOROS_CC :
         retCode = StringTable::COMOROS;
         break;
      case CONGO_CC :
         retCode = StringTable::CONGO;
         break;
      case COOK_ISLANDS_CC :
         retCode = StringTable::COOK_ISLANDS;
         break;
      case COSTA_RICA_CC :
         retCode = StringTable::COSTA_RICA;
         break;
      case CUBA_CC :
         retCode = StringTable::CUBA;
         break;
      case DJIBOUTI_CC :
         retCode = StringTable::DJIBOUTI;
         break;
      case DOMINICA_CC :
         retCode = StringTable::DOMINICA;
         break;
      case DOMINICAN_REPUBLIC_CC :
         retCode = StringTable::DOMINICAN_REPUBLIC;
         break;
      case DR_CONGO_CC :
         retCode = StringTable::DR_CONGO;
         break;
      case ECUADOR_CC :
         retCode = StringTable::ECUADOR;
         break;
      case EGYPT_CC :
         retCode = StringTable::EGYPT;
         break;
      case EL_SALVADOR_CC :
         retCode = StringTable::EL_SALVADOR;
         break;
      case EQUATORIAL_GUINEA_CC :
         retCode = StringTable::EQUATORIAL_GUINEA;
         break;
      case ERITREA_CC :
         retCode = StringTable::ERITREA;
         break;
      case ETHIOPIA_CC :
         retCode = StringTable::ETHIOPIA;
         break;
      case FAEROE_ISLANDS_CC :
         retCode = StringTable::FAEROE_ISLANDS;
         break;
      case FALKLAND_ISLANDS_CC :
         retCode = StringTable::FALKLAND_ISLANDS;
         break;
      case FIJI_CC :
         retCode = StringTable::FIJI;
         break;
      case FRENCH_GUIANA_CC :
         retCode = StringTable::FRENCH_GUIANA;
         break;
      case FRENCH_POLYNESIA_CC :
         retCode = StringTable::FRENCH_POLYNESIA;
         break;
      case GABON_CC :
         retCode = StringTable::GABON;
         break;
      case GAMBIA_CC :
         retCode = StringTable::GAMBIA;
         break;
      case GEORGIA_CC :
         retCode = StringTable::GEORGIA_COUNTRY;
         break;
      case GHANA_CC :
         retCode = StringTable::GHANA;
         break;
      case GREENLAND_CC :
         retCode = StringTable::GREENLAND;
         break;
      case GRENADA_CC :
         retCode = StringTable::GRENADA;
         break;
      case GUADELOUPE_CC :
         retCode = StringTable::GUADELOUPE;
         break;
      case GUAM_CC :
         retCode = StringTable::GUAM;
         break;
      case GUATEMALA_CC :
         retCode = StringTable::GUATEMALA;
         break;
      case GUINEA_CC :
         retCode = StringTable::GUINEA;
         break;
      case GUINEA_BISSAU_CC :
         retCode = StringTable::GUINEA_BISSAU;
         break;
      case GUYANA_CC :
         retCode = StringTable::GUYANA;
         break;
      case HAITI_CC :
         retCode = StringTable::HAITI;
         break;
      case HONDURAS_CC :
         retCode = StringTable::HONDURAS;
         break;
      case INDIA_CC :
         retCode = StringTable::INDIA;
         break;
      case INDONESIA_CC :
         retCode = StringTable::INDONESIA;
         break;
      case IRAN_CC :
         retCode = StringTable::IRAN;
         break;
      case IRAQ_CC :
         retCode = StringTable::IRAQ;
         break;
      case IVORY_COAST_CC :
         retCode = StringTable::IVORY_COAST;
         break;
      case JAMAICA_CC :
         retCode = StringTable::JAMAICA;
         break;
      case JAPAN_CC :
         retCode = StringTable::JAPAN;
         break;
      case JORDAN_CC :
         retCode = StringTable::JORDAN;
         break;
      case KAZAKHSTAN_CC :
         retCode = StringTable::KAZAKHSTAN;
         break;
      case KENYA_CC :
         retCode = StringTable::KENYA;
         break;
      case KIRIBATI_CC :
         retCode = StringTable::KIRIBATI;
         break;
      case KUWAIT_CC :
         retCode = StringTable::KUWAIT;
         break;
      case KYRGYZSTAN_CC :
         retCode = StringTable::KYRGYZSTAN;
         break;
      case LAOS_CC :
         retCode = StringTable::LAOS;
         break;
      case LEBANON_CC :
         retCode = StringTable::LEBANON;
         break;
      case LESOTHO_CC :
         retCode = StringTable::LESOTHO;
         break;
      case LIBERIA_CC :
         retCode = StringTable::LIBERIA;
         break;
      case LIBYA_CC :
         retCode = StringTable::LIBYA;
         break;
      case MACAO_CC :
         retCode = StringTable::MACAO;
         break;
      case MADAGASCAR_CC :
         retCode = StringTable::MADAGASCAR;
         break;
      case MALAWI_CC :
         retCode = StringTable::MALAWI;
         break;
      case MALAYSIA_CC :
         retCode = StringTable::MALAYSIA;
         break;
      case MALDIVES_CC :
         retCode = StringTable::MALDIVES;
         break;
      case MALI_CC :
         retCode = StringTable::MALI;
         break;
      case MARSHALL_ISLANDS_CC :
         retCode = StringTable::MARSHALL_ISLANDS;
         break;
      case MARTINIQUE_CC :
         retCode = StringTable::MARTINIQUE;
         break;
      case MAURITANIA_CC :
         retCode = StringTable::MAURITANIA;
         break;
      case MAURITIUS_CC :
         retCode = StringTable::MAURITIUS;
         break;
      case MAYOTTE_CC :
         retCode = StringTable::MAYOTTE;
         break;
      case MEXICO_CC :
         retCode = StringTable::MEXICO;
         break;
      case MICRONESIA_CC :
         retCode = StringTable::MICRONESIA;
         break;
      case MONGOLIA_CC :
         retCode = StringTable::MONGOLIA;
         break;
      case MONTSERRAT_CC :
         retCode = StringTable::MONTSERRAT;
         break;
      case MOROCCO_CC :
         retCode = StringTable::MOROCCO;
         break;
      case MOZAMBIQUE_CC :
         retCode = StringTable::MOZAMBIQUE;
         break;
      case MYANMAR_CC :
         retCode = StringTable::MYANMAR;
         break;
      case NAMIBIA_CC :
         retCode = StringTable::NAMIBIA;
         break;
      case NAURU_CC :
         retCode = StringTable::NAURU;
         break;
      case NEPAL_CC :
         retCode = StringTable::NEPAL;
         break;
      case NETHERLANDS_ANTILLES_CC :
         retCode = StringTable::NETHERLANDS_ANTILLES;
         break;
      case NEW_CALEDONIA_CC :
         retCode = StringTable::NEW_CALEDONIA;
         break;
      case NEW_ZEALAND_CC :
         retCode = StringTable::NEW_ZEALAND;
         break;
      case NICARAGUA_CC :
         retCode = StringTable::NICARAGUA;
         break;
      case NIGER_CC :
         retCode = StringTable::NIGER;
         break;
      case NIGERIA_CC :
         retCode = StringTable::NIGERIA;
         break;
      case NIUE_CC :
         retCode = StringTable::NIUE;
         break;
      case NORTHERN_MARIANA_ISLANDS_CC :
         retCode = StringTable::NORTHERN_MARIANA_ISLANDS;
         break;
      case NORTH_KOREA_CC :
         retCode = StringTable::NORTH_KOREA;
         break;
      case OCCUPIED_PALESTINIAN_TERRITORY_CC :
         retCode = StringTable::OCCUPIED_PALESTINIAN_TERRITORY;
         break;
      case OMAN_CC :
         retCode = StringTable::OMAN;
         break;
      case PAKISTAN_CC :
         retCode = StringTable::PAKISTAN;
         break;
      case PALAU_CC :
         retCode = StringTable::PALAU;
         break;
      case PANAMA_CC :
         retCode = StringTable::PANAMA;
         break;
      case PAPUA_NEW_GUINEA_CC :
         retCode = StringTable::PAPUA_NEW_GUINEA;
         break;
      case PARAGUAY_CC :
         retCode = StringTable::PARAGUAY;
         break;
      case PERU_CC :
         retCode = StringTable::PERU;
         break;
      case PHILIPPINES_CC :
         retCode = StringTable::PHILIPPINES;
         break;
      case PITCAIRN_CC :
         retCode = StringTable::PITCAIRN;
         break;
      case QATAR_CC :
         retCode = StringTable::QATAR;
         break;
      case REUNION_CC :
         retCode = StringTable::REUNION;
         break;
      case RWANDA_CC :
         retCode = StringTable::RWANDA;
         break;
      case SAINT_HELENA_CC :
         retCode = StringTable::SAINT_HELENA;
         break;
      case SAINT_KITTS_AND_NEVIS_CC :
         retCode = StringTable::SAINT_KITTS_AND_NEVIS;
         break;
      case SAINT_LUCIA_CC :
         retCode = StringTable::SAINT_LUCIA;
         break;
      case SAINT_PIERRE_AND_MIQUELON_CC :
         retCode = StringTable::SAINT_PIERRE_AND_MIQUELON;
         break;
      case SAINT_VINCENT_AND_THE_GRENADINES_CC :
         retCode = StringTable::SAINT_VINCENT_AND_THE_GRENADINES;
         break;
      case SAMOA_CC :
         retCode = StringTable::SAMOA;
         break;
      case SAO_TOME_AND_PRINCIPE_CC :
         retCode = StringTable::SAO_TOME_AND_PRINCIPE;
         break;
      case SAUDI_ARABIA_CC :
         retCode = StringTable::SAUDI_ARABIA;
         break;
      case SENEGAL_CC :
         retCode = StringTable::SENEGAL;
         break;
      case SEYCHELLES_CC :
         retCode = StringTable::SEYCHELLES;
         break;
      case SIERRA_LEONE_CC :
         retCode = StringTable::SIERRA_LEONE;
         break;
      case SOLOMON_ISLANDS_CC :
         retCode = StringTable::SOLOMON_ISLANDS;
         break;
      case SOMALIA_CC :
         retCode = StringTable::SOMALIA;
         break;
      case SOUTH_AFRICA_CC :
         retCode = StringTable::SOUTH_AFRICA;
         break;
      case SOUTH_KOREA_CC :
         retCode = StringTable::SOUTH_KOREA;
         break;
      case SRI_LANKA_CC :
         retCode = StringTable::SRI_LANKA;
         break;
      case SUDAN_CC :
         retCode = StringTable::SUDAN;
         break;
      case SURINAME_CC :
         retCode = StringTable::SURINAME;
         break;
      case SVALBARD_AND_JAN_MAYEN_CC :
         retCode = StringTable::SVALBARD_AND_JAN_MAYEN;
         break;
      case SWAZILAND_CC :
         retCode = StringTable::SWAZILAND;
         break;
      case SYRIA_CC :
         retCode = StringTable::SYRIA;
         break;
      case TAIWAN_CC :
         retCode = StringTable::TAIWAN;
         break;
      case TAJIKISTAN_CC :
         retCode = StringTable::TAJIKISTAN;
         break;
      case TANZANIA_CC :
         retCode = StringTable::TANZANIA;
         break;
      case THAILAND_CC :
         retCode = StringTable::THAILAND;
         break;
      case TIMOR_LESTE_CC :
         retCode = StringTable::TIMOR_LESTE;
         break;
      case TOGO_CC :
         retCode = StringTable::TOGO;
         break;
      case TOKELAU_CC :
         retCode = StringTable::TOKELAU;
         break;
      case TONGA_CC :
         retCode = StringTable::TONGA;
         break;
      case TRINIDAD_AND_TOBAGO_CC :
         retCode = StringTable::TRINIDAD_AND_TOBAGO;
         break;
      case TUNISIA_CC :
         retCode = StringTable::TUNISIA;
         break;
      case TURKMENISTAN_CC :
         retCode = StringTable::TURKMENISTAN;
         break;
      case TURKS_AND_CAICOS_ISLANDS_CC :
         retCode = StringTable::TURKS_AND_CAICOS_ISLANDS;
         break;
      case TUVALU_CC :
         retCode = StringTable::TUVALU;
         break;
      case UGANDA_CC :
         retCode = StringTable::UGANDA;
         break;
      case UNITED_STATES_MINOR_OUTLYING_ISLANDS_CC :
         retCode = StringTable::UNITED_STATES_MINOR_OUTLYING_ISLANDS;
         break;
      case UNITED_STATES_VIRGIN_ISLANDS_CC :
         retCode = StringTable::UNITED_STATES_VIRGIN_ISLANDS;
         break;
      case URUGUAY_CC :
         retCode = StringTable::URUGUAY;
         break;
      case UZBEKISTAN_CC :
         retCode = StringTable::UZBEKISTAN;
         break;
      case VANUATU_CC :
         retCode = StringTable::VANUATU;
         break;
      case VENEZUELA_CC :
         retCode = StringTable::VENEZUELA;
         break;
      case VIETNAM_CC :
         retCode = StringTable::VIETNAM;
         break;
      case WALLIS_AND_FUTUNA_ISLANDS_CC :
         retCode = StringTable::WALLIS_AND_FUTUNA_ISLANDS;
         break;
      case WESTERN_SAHARA_CC :
         retCode = StringTable::WESTERN_SAHARA;
         break;
      case YEMEN_CC :
         retCode = StringTable::YEMEN;
         break;
      case ZAMBIA_CC :
         retCode = StringTable::ZAMBIA;
         break;
      case ZIMBABWE_CC :
         retCode = StringTable::ZIMBABWE;
         break;
      case SAN_MARINO_CC :
         retCode = StringTable::SAN_MARINO;
         break;

      default:
         break;
   }

   // Return the string code
   DEBUG2(mc2dbg << "Country code " << int(cc) << " has string code " 
               << int(retCode) << endl);
   return (retCode);
}

const char*
StringTable::getStringToDisplayForCountry( countryCode cc, languageCode lc )
{
   
   // The string code that will be returned
   StringTable::stringCode retCode = getCountryStringCode( cc );
   
   if ( retCode == ENGLAND ) {
      retCode = UNITED_KINGDOM;
   }

   return ( getString( retCode, lc) );
}

bool
StringTable::sanityCheck()
{
   // Note that this function does not check everything.
   // Please update when needed.
   for ( int i = 0, n = NBR_STRINGS; i < n; ++i ) {
      if ( StringTable::strings[StringTable::stringCode(i)][ENGLISH] ) {
         // OK
      } else {
         cerr << "[StringTable]: String nbr "
              << i << " does not have an Engrish translation" << endl;
         MC2_ASSERT( false );
      }
   }


   for ( int i = 0, n = nbrLanguages + nbrPseudoLanguages; i < n; ++i ) {
      if ( getLanguageAsStringCode( StringTable::languageCode(i) ) ==
           StringTable::UNKNOWN ) {
         cerr << "[StringTable]: getLanguageAsStringCode(" << i << ")"
              << " does not work" << endl;
      }                                     
   }
   
   return true;
}

const bool
StringTable::m_sanityCheckRun = sanityCheck();

// FIXME: Check the ones that begin with the same number
const char* StringTable::countryCodes[][2] = {
   // first same order as those in country code table
   {"44", "United Kingdom"},
   {"46", "Sweden"},
   {"49", "Germany"},
   {"45", "Denmark"},
   {"358", "Finland"},
   {"47", "Norway"},
   {"32", "Belgium"},
   {"31", "Netherlands"},
   {"352", "Luxembourg"},
   {"1", "United States"},
   {"41", "Switzerland"},
   {"43", "Austria"},
   {"33", "France"},
   {"34", "Spain"},
   {"376","Andorra"},
   {"423", "Liechtenstein"},
   {"39", "Italy"},
   {"377", "Monaco"},
   {"353", "Ireland"},
   {"351", "Portugal"},
   {"1", "Canada"},
   {"36", "Hungary"},
   {"420", "Czech Republic"},
   {"48", "Poland"},
   {"30", "Greece"},
   {"972", "Israel"},
   {"55", "Brazil"},
   {"421", "Slovakia"},
   {"7", "Russian Federation"},
   {"90", "Turkey"},
   {"386", "Slovenia"},
   {"359", "Bulgaria"},
   {"40", "Romania"},
   {"380", "Ukraine"},
   {"381", "Serbia and Montenegro"},

   {"385", "Croatia"},
   {"387", "Bosnia and Herzegovina"},
   {"373", "Moldova"},
   {"389", "Macedonia"},
   {"372", "Estonia"},
   {"371", "Latvia"},
   {"370", "Lithuania"},
   {"375", "Belarus"},
   {"356", "Malta"},
   {"357", "Cyprus"},
   {"354", "Iceland"},
   {"852", "Hong Kong"},
   {"65", "Singapore"},
   {"61", "Australia"},
   {"971", "United Arab Emirates"},
   {"973", "Bahrain"},

   // AND world countries here
   {"93", "Afghanistan"},
   {"355", "Albania"},
   {"213", "Algeria"},
   {"684", "American Samoa"},
   {"244", "Angola"},
   {"1264", "Anguilla"},
   {"672", "Antarctica"},
   {"1268", "Antigua and Barbuda"},
   {"54", "Argentina"},
   {"374", "Armenia"},
   {"297", "Aruba"},
   {"994", "Azerbaijan"},
   {"1242", "Bahamas"},
   {"880", "Bangladesh"},
   {"1246", "Barbados"},
   {"501", "Belize"},
   {"229", "Benin"},
   {"1441", "Bermuda"},
   {"975", "Bhutan"},
   {"591", "Bolivia"},
   {"267", "Botswana"},
   {"1284", "Virgin Islands (British)"},
   {"673", "Brunei Darussalam"},
   {"226", "Burkina Faso"},
   {"257", "Burundi"},
   {"855", "Cambodia"},
   {"237", "Cameroon"},
   {"238", "Cape Verde"},
   {"1345", "Cayman Islands"},
   {"236", "Central African Republic"},
   {"235", "Chad"},
   {"56", "Chile"},
   {"86", "China"},
   {"57", "Colombia"},
   {"269", "Comoros"},
   {"242", "Congo"},
   {"682", "Cook Islands"},
   {"506", "Costa Rica"},
   {"53", "Cuba"},
   {"253", "Djibouti"},
   {"1767", "Dominica"},
   {"1809", "Dominican Republic"},
   {"243", "Congo, Democratic Republic"}, // Former Zaire
   {"593", "Ecuador"},
   {"20", "Egypt"},
   {"503", "El Salvador"},
   {"240", "Equatorial Guinea"},
   {"291", "Eritrea"},
   {"251", "Ethiopia"},
   {"298", "Faroe Islands"},
   {"500", "Falkland Islands (Malvinas)"},
   {"679", "Fiji"},
   {"594", "French Guiana"},
   {"689", "French Polynesia"},
   {"241", "Gabon"},
   {"220", "Gambia"},
   {"995", "Georgia"},
   {"233", "Ghana"},
   {"299", "Greenland"},
   {"1473", "Grenada"},
   {"590", "Guadeloupe"},
   {"1671", "Guam"},
   {"502", "Guatemala"},
   {"224", "Guinea"},
   {"245", "Guinea-Bissau"},
   {"592", "Guyana"},
   {"509", "Haiti"},
   {"504", "Honduras"},
   {"91", "India"},
   {"62", "Indonesia"},
   {"98", "Iran, Islamic Republic of"},
   {"964", "Iraq"},
   {"225", "Cote D'Ivoire"}, // Ivory Coast
   {"1809", "Jamaica"},
   {"81", "Japan"},
   {"962", "Jordan"},
   {"7", "Kazakhstan"},
   {"254", "Kenya"},
   {"686", "Kiribati"},
   {"965", "Kuwait"},
   {"996", "Kyrgyzstan"},
   {"856", "Laos"},
   {"961", "Lebanon"},
   {"266", "Lesotho"},
   {"231", "Liberia"},
   {"218", "Libya"},
   {"853", "Macau"},
   {"261", "Madagascar"},
   {"265", "Malawi"},
   {"60", "Malaysia"},
   {"960", "Maldives"},
   {"223", "Mali"},
   {"692", "Marshall Islands"},
   {"596", "Martinique"},
   {"222", "Mauritania"},
   {"230", "Mauritius"},
   {"269", "Mayotte"},
   {"52", "Mexico"},
   {"691", "Micronesia"},
   {"976", "Mongolia"},
   {"1664", "Montserrat"},
   {"212", "Morocco"},
   {"258", "Mozambique"},
   {"95", "Myanmar"},
   {"264", "Namibia"},
   {"674", "Nauru"},
   {"977", "Nepal"},
   {"599", "Netherlands Antilles"},
   {"687", "New Caledonia"},
   {"64", "New Zealand"},
   {"505", "Nicaragua"},
   {"227", "Niger"},
   {"234", "Nigeria"},
   {"683", "Niue"},
   {"1670", "Northern Mariana Islands"},
   {"850", "Korea, People's Republic"},   // North Korea
   {"970", "Palestinian Settlements"},
   {"968", "Oman"},
   {"92", "Pakistan"},
   {"680", "Palau"},
   {"507", "Panama"},
   {"675", "Papua New Guinea"},
   {"595", "Paraguay"},
   {"51", "Peru"},
   {"63", "Philippines"},
   {"872", "Pitcairn"},
   {"974", "Qatar"},
   {"262", "Reunion"},
   {"250", "Rwanda"},
   {"290", "St. Helena"},
   {"1869", "Saint Kitts and Nevis"},
   {"1758", "Saint Lucia"},
   {"508", "St. Pierre And Miquelon"},
   {"1784", "St. Vincent and Grenadines"},
   {"685", "Samoa"},
   {"239", "Sao Tome and Principe"},
   {"966", "Saudi Arabia"},
   {"221", "Senegal"},
   {"248", "Seychelles"},
   {"232", "Sierra Leone"},
   {"677", "Solomon Islands"},
   {"252", "Somalia"},
   {"27", "South Africa"},
   {"82", "Korea, Republic"}, // South Korea
   {"94", "Sri Lanka"},
   {"249", "Sudan"},
   {"597", "Suriname"},
   {"47", "Svalbard and Jan Mayen"},
   {"268", "Swaziland"},
   {"963", "Syrian Arab Republic"},
   {"886", "Taiwan,Province of China"},
   {"992", "Tajikistan"},
   {"255", "Tanzania"},
   {"66", "Thailand"},
   {"670", "Timor-Leste"},
   {"228", "Togo"},
   {"690", "Tokelau"},
   {"676", "Tonga"},
   {"1868", "Trinidad and Tobago"},
   {"216", "Tunisia"},
   {"993", "Turkmenistan"},
   {"1649", "Turks and Caicos Islands"},
   {"688", "Tuvalu"},
   {"256", "Uganda"},
   {"1", "US Minor Outlying Islands"},
   {"1340", "Virgin Islands (U.S.)"},
   {"598", "Uruguay"},
   {"998", "Uzbekistan"},
   {"678", "Vanuatu"},
   {"58", "Venezuela"},
   {"84", "Vietnam"},
   {"681", "Wallis and Futuna Islands"},
   {"212", "Western Sahara"},
   {"967", "Yemen"},
   {"260", "Zambia"},
   {"263", "Zimbabwe"},
   {"378", "San Marino"},
   
   {"XX", "EndOfCountryEnum"},

   // then the others.
   {"350", "Gibraltar"},
   {"6723", "Norfolk Island"},
   {"1787", "Puerto Rico"},


   { NULL, NULL}
};
//


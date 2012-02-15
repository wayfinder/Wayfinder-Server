/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGTABLE_H
#define STRINGTABLE_H

#include "config.h"
#include "MC2String.h"

#include "StringTableUTF8.h"

class LangType;

/**
  *   This class contains all string that might be written in the
  *   MapCentral system. All functions are static.
  *
  */
class StringTable : public StringTableUTF8 {
public :
      /**
        *   @name Countries.
        *   The countries that are available. Used for storage in the
        *   maps. To get a (printable) string with the name of the
        *   country, methods in this class must be used! The extension
        *   "_CC" is for CountryCode.
        */
      enum countryCode {
         /// England
         ENGLAND_CC = 0,

         /// Sweden
         SWEDEN_CC = 1,

         /// German
         GERMANY_CC = 2,

         /// Denmark
         DENMARK_CC = 3,

         /// Finland
         FINLAND_CC = 4,

         /// Norway
         NORWAY_CC = 5,

         /// Belgium
         BELGIUM_CC = 6,

         /// Netherlands
         NETHERLANDS_CC = 7,

         /// Luxembourg
         LUXEMBOURG_CC = 8,

         /// USA
         USA_CC = 9,

         /// SWITZERLAND
         SWITZERLAND_CC = 10,

         /// AUSTRIA
         AUSTRIA_CC = 11,

         /// FRANCE
         FRANCE_CC = 12,

         /// SPAIN
         SPAIN_CC = 13,

         /// ANDORRA
         ANDORRA_CC = 14,

         /// LIECHTENSTEIN
         LIECHTENSTEIN_CC = 15,

         /// ITALY
         ITALY_CC = 16,

         /// MONACO
         MONACO_CC = 17,

         /// IRELAND
         IRELAND_CC = 18,

         /// PORTUGAL
         PORTUGAL_CC = 19,

         /// CANADA
         CANADA_CC = 20,

         /// HUNGARY
         HUNGARY_CC = 21,

         /// CZECH REPUBLIC
         CZECH_REPUBLIC_CC = 22,

         /// POLAND
         POLAND_CC = 23,

         /// GREECE
         GREECE_CC = 24,

         /// ISRAEL
         ISRAEL_CC = 25,

         /// BRAZIL
         BRAZIL_CC = 26,

         /// SLOVAKIA
         SLOVAKIA_CC = 27,

         /// RUSSIA
         RUSSIA_CC = 28,

         /// TURKEY
         TURKEY_CC = 29,

         /// SLOVENIA
         SLOVENIA_CC = 30,

         /// BULGARIA
         BULGARIA_CC = 31,

         /// ROMANIA
         ROMANIA_CC = 32,

         /// UKRAINE
         UKRAINE_CC = 33,

         /// SERBIA & MONTENEGRO
         SERBIA_CC = 34,

         /// CROATIA
         CROATIA_CC = 35,

         /// BOSNIA and Herzegovina
         BOSNIA_CC = 36,

         /// MOLDOVA
         MOLDOVA_CC = 37,

         /// MACEDONIA, The Former Yugoslav Rep. Of
         MACEDONIA_CC = 38,

         /// ESTONIA
         ESTONIA_CC = 39,

         /// LATVIA
         LATVIA_CC = 40,

         /// LITHUANIA
         LITHUANIA_CC = 41,

         /// BELARUS
         BELARUS_CC = 42,

         /// MALTA
         MALTA_CC = 43,
         
         /// CYPRUS,
         CYPRUS_CC = 44,
         
         /// ICELAND,
         ICELAND_CC = 45,
         
         /// HONG KONG,
         HONG_KONG_CC = 46,
         
         /// SINGAPORE,
         SINGAPORE_CC = 47,
         
         /// AUSTRALIA,
         AUSTRALIA_CC = 48,
         
         /// UNITED ARAB EMIRATES,
         UAE_CC = 49,
         
         /// BAHRAIN,
         BAHRAIN_CC = 50,
         
         /// AFGHANISTAN,
         AFGHANISTAN_CC = 51,

         /// ALBANIA,
         ALBANIA_CC = 52,

         /// ALGERIA,
         ALGERIA_CC = 53,

         /// AMERICAN_SAMOA,
         AMERICAN_SAMOA_CC = 54,

         /// ANGOLA,
         ANGOLA_CC = 55,

         /// ANGUILLA,
         ANGUILLA_CC = 56,

         /// ANTARCTICA,
         ANTARCTICA_CC = 57,

         /// ANTIGUA_AND_BARBUDA,
         ANTIGUA_AND_BARBUDA_CC = 58,

         /// ARGENTINA,
         ARGENTINA_CC = 59,

         /// ARMENIA,
         ARMENIA_CC = 60,

         /// ARUBA,
         ARUBA_CC = 61,

         /// AZERBAIJAN,
         AZERBAIJAN_CC = 62,

         /// BAHAMAS,
         BAHAMAS_CC = 63,

         /// BANGLADESH,
         BANGLADESH_CC = 64,

         /// BARBADOS,
         BARBADOS_CC = 65,

         /// BELIZE,
         BELIZE_CC = 66,

         /// BENIN,
         BENIN_CC = 67,

         /// BERMUDA,
         BERMUDA_CC = 68,

         /// BHUTAN,
         BHUTAN_CC = 69,

         /// BOLIVIA,
         BOLIVIA_CC = 70,

         /// BOTSWANA,
         BOTSWANA_CC = 71,

         /// BRITISH_VIRGIN_ISLANDS,
         BRITISH_VIRGIN_ISLANDS_CC = 72,

         /// BRUNEI_DARUSSALAM,
         BRUNEI_DARUSSALAM_CC = 73,

         /// BURKINA_FASO,
         BURKINA_FASO_CC = 74,

         /// BURUNDI,
         BURUNDI_CC = 75,

         /// CAMBODIA,
         CAMBODIA_CC = 76,

         /// CAMEROON,
         CAMEROON_CC = 77,

         /// CAPE_VERDE,
         CAPE_VERDE_CC = 78,

         /// CAYMAN_ISLANDS,
         CAYMAN_ISLANDS_CC = 79,

         /// CENTRAL_AFRICAN_REPUBLIC,
         CENTRAL_AFRICAN_REPUBLIC_CC = 80,

         /// CHAD,
         CHAD_CC = 81,

         /// CHILE,
         CHILE_CC = 82,

         /// CHINA,
         CHINA_CC = 83,

         /// COLOMBIA,
         COLOMBIA_CC = 84,

         /// COMOROS,
         COMOROS_CC = 85,

         /// CONGO,
         CONGO_CC = 86,

         /// COOK_ISLANDS,
         COOK_ISLANDS_CC = 87,

         /// COSTA_RICA,
         COSTA_RICA_CC = 88,

         /// CUBA,
         CUBA_CC = 89,

         /// DJIBOUTI,
         DJIBOUTI_CC = 90,

         /// DOMINICA,
         DOMINICA_CC = 91,

         /// DOMINICAN_REPUBLIC,
         DOMINICAN_REPUBLIC_CC = 92,

         /// DR_CONGO,
         DR_CONGO_CC = 93,

         /// ECUADOR,
         ECUADOR_CC = 94,

         /// EGYPT,
         EGYPT_CC = 95,

         /// EL_SALVADOR,
         EL_SALVADOR_CC = 96,

         /// EQUATORIAL_GUINEA,
         EQUATORIAL_GUINEA_CC = 97,

         /// ERITREA,
         ERITREA_CC = 98,

         /// ETHIOPIA,
         ETHIOPIA_CC = 99,

         /// FAEROE_ISLANDS,
         FAEROE_ISLANDS_CC = 100,

         /// FALKLAND_ISLANDS,
         FALKLAND_ISLANDS_CC = 101,

         /// FIJI,
         FIJI_CC = 102,

         /// FRENCH_GUIANA,
         FRENCH_GUIANA_CC = 103,

         /// FRENCH_POLYNESIA,
         FRENCH_POLYNESIA_CC = 104,

         /// GABON,
         GABON_CC = 105,

         /// GAMBIA,
         GAMBIA_CC = 106,

         /// GEORGIA,
         GEORGIA_CC = 107,

         /// GHANA,
         GHANA_CC = 108,

         /// GREENLAND,
         GREENLAND_CC = 109,

         /// GRENADA,
         GRENADA_CC = 110,

         /// GUADELOUPE,
         GUADELOUPE_CC = 111,

         /// GUAM,
         GUAM_CC = 112,

         /// GUATEMALA,
         GUATEMALA_CC = 113,

         /// GUINEA,
         GUINEA_CC = 114,

         /// GUINEA_BISSAU,
         GUINEA_BISSAU_CC = 115,

         /// GUYANA,
         GUYANA_CC = 116,

         /// HAITI,
         HAITI_CC = 117,

         /// HONDURAS,
         HONDURAS_CC = 118,

         /// INDIA,
         INDIA_CC = 119,

         /// INDONESIA,
         INDONESIA_CC = 120,

         /// IRAN,
         IRAN_CC = 121,

         /// IRAQ,
         IRAQ_CC = 122,

         /// IVORY_COAST,
         IVORY_COAST_CC = 123,

         /// JAMAICA,
         JAMAICA_CC = 124,

         /// JAPAN,
         JAPAN_CC = 125,

         /// JORDAN,
         JORDAN_CC = 126,

         /// KAZAKHSTAN,
         KAZAKHSTAN_CC = 127,

         /// KENYA,
         KENYA_CC = 128,

         /// KIRIBATI,
         KIRIBATI_CC = 129,

         /// KUWAIT,
         KUWAIT_CC = 130,

         /// KYRGYZSTAN,
         KYRGYZSTAN_CC = 131,

         /// LAOS,
         LAOS_CC = 132,

         /// LEBANON,
         LEBANON_CC = 133,

         /// LESOTHO,
         LESOTHO_CC = 134,

         /// LIBERIA,
         LIBERIA_CC = 135,

         /// LIBYA,
         LIBYA_CC = 136,

         /// MACAO,
         MACAO_CC = 137,

         /// MADAGASCAR,
         MADAGASCAR_CC = 138,

         /// MALAWI,
         MALAWI_CC = 139,

         /// MALAYSIA,
         MALAYSIA_CC = 140,

         /// MALDIVES,
         MALDIVES_CC = 141,

         /// MALI,
         MALI_CC = 142,

         /// MARSHALL_ISLANDS,
         MARSHALL_ISLANDS_CC = 143,

         /// MARTINIQUE,
         MARTINIQUE_CC = 144,

         /// MAURITANIA,
         MAURITANIA_CC = 145,

         /// MAURITIUS,
         MAURITIUS_CC = 146,

         /// MAYOTTE,
         MAYOTTE_CC = 147,

         /// MEXICO,
         MEXICO_CC = 148,

         /// MICRONESIA,
         MICRONESIA_CC = 149,

         /// MONGOLIA,
         MONGOLIA_CC = 150,

         /// MONTSERRAT,
         MONTSERRAT_CC = 151,

         /// MOROCCO,
         MOROCCO_CC = 152,

         /// MOZAMBIQUE,
         MOZAMBIQUE_CC = 153,

         /// MYANMAR,
         MYANMAR_CC = 154,

         /// NAMIBIA,
         NAMIBIA_CC = 155,

         /// NAURU,
         NAURU_CC = 156,

         /// NEPAL,
         NEPAL_CC = 157,

         /// NETHERLANDS_ANTILLES,
         NETHERLANDS_ANTILLES_CC = 158,

         /// NEW_CALEDONIA,
         NEW_CALEDONIA_CC = 159,

         /// NEW_ZEALAND,
         NEW_ZEALAND_CC = 160,

         /// NICARAGUA,
         NICARAGUA_CC = 161,

         /// NIGER,
         NIGER_CC = 162,

         /// NIGERIA,
         NIGERIA_CC = 163,

         /// NIUE,
         NIUE_CC = 164,

         /// NORTHERN_MARIANA_ISLANDS,
         NORTHERN_MARIANA_ISLANDS_CC = 165,

         /// NORTH_KOREA,
         NORTH_KOREA_CC = 166,

         /// OCCUPIED_PALESTINIAN_TERRITORY,
         OCCUPIED_PALESTINIAN_TERRITORY_CC = 167,

         /// OMAN,
         OMAN_CC = 168,

         /// PAKISTAN,
         PAKISTAN_CC = 169,

         /// PALAU,
         PALAU_CC = 170,

         /// PANAMA,
         PANAMA_CC = 171,

         /// PAPUA_NEW_GUINEA,
         PAPUA_NEW_GUINEA_CC = 172,

         /// PARAGUAY,
         PARAGUAY_CC = 173,

         /// PERU,
         PERU_CC = 174,

         /// PHILIPPINES,
         PHILIPPINES_CC = 175,

         /// PITCAIRN,
         PITCAIRN_CC = 176,

         /// QATAR,
         QATAR_CC = 177,

         /// REUNION,
         REUNION_CC = 178,

         /// RWANDA,
         RWANDA_CC = 179,

         /// SAINT_HELENA,
         SAINT_HELENA_CC = 180,

         /// SAINT_KITTS_AND_NEVIS,
         SAINT_KITTS_AND_NEVIS_CC = 181,

         /// SAINT_LUCIA,
         SAINT_LUCIA_CC = 182,

         /// SAINT_PIERRE_AND_MIQUELON,
         SAINT_PIERRE_AND_MIQUELON_CC = 183,

         /// SAINT_VINCENT_AND_THE_GRENADINES,
         SAINT_VINCENT_AND_THE_GRENADINES_CC = 184,

         /// SAMOA,
         SAMOA_CC = 185,

         /// SAO_TOME_AND_PRINCIPE,
         SAO_TOME_AND_PRINCIPE_CC = 186,

         /// SAUDI_ARABIA,
         SAUDI_ARABIA_CC = 187,

         /// SENEGAL,
         SENEGAL_CC = 188,

         /// SEYCHELLES,
         SEYCHELLES_CC = 189,

         /// SIERRA_LEONE,
         SIERRA_LEONE_CC = 190,

         /// SOLOMON_ISLANDS,
         SOLOMON_ISLANDS_CC = 191,

         /// SOMALIA,
         SOMALIA_CC = 192,

         /// SOUTH_AFRICA,
         SOUTH_AFRICA_CC = 193,

         /// SOUTH_KOREA,
         SOUTH_KOREA_CC = 194,

         /// SRI_LANKA,
         SRI_LANKA_CC = 195,

         /// SUDAN,
         SUDAN_CC = 196,

         /// SURINAME,
         SURINAME_CC = 197,

         /// SVALBARD_AND_JAN_MAYEN,
         SVALBARD_AND_JAN_MAYEN_CC = 198,

         /// SWAZILAND,
         SWAZILAND_CC = 199,

         /// SYRIA,
         SYRIA_CC = 200,

         /// TAIWAN,
         TAIWAN_CC = 201,

         /// TAJIKISTAN,
         TAJIKISTAN_CC = 202,

         /// TANZANIA,
         TANZANIA_CC = 203,

         /// THAILAND,
         THAILAND_CC = 204,

         /// TIMOR_LESTE,
         TIMOR_LESTE_CC = 205,

         /// TOGO,
         TOGO_CC = 206,

         /// TOKELAU,
         TOKELAU_CC = 207,

         /// TONGA,
         TONGA_CC = 208,

         /// TRINIDAD_AND_TOBAGO,
         TRINIDAD_AND_TOBAGO_CC = 209,

         /// TUNISIA,
         TUNISIA_CC = 210,

         /// TURKMENISTAN,
         TURKMENISTAN_CC = 211,

         /// TURKS_AND_CAICOS_ISLANDS,
         TURKS_AND_CAICOS_ISLANDS_CC = 212,

         /// TUVALU,
         TUVALU_CC = 213,

         /// UGANDA,
         UGANDA_CC = 214,

         /// UNITED_STATES_MINOR_OUTLYING_ISLANDS,
         UNITED_STATES_MINOR_OUTLYING_ISLANDS_CC = 215,

         /// UNITED_STATES_VIRGIN_ISLANDS,
         UNITED_STATES_VIRGIN_ISLANDS_CC = 216,

         /// URUGUAY,
         URUGUAY_CC = 217,

         /// UZBEKISTAN,
         UZBEKISTAN_CC = 218,

         /// VANUATU,
         VANUATU_CC = 219,

         /// VENEZUELA,
         VENEZUELA_CC = 220,

         /// VIETNAM,
         VIETNAM_CC = 221,

         /// WALLIS_AND_FUTUNA_ISLANDS,
         WALLIS_AND_FUTUNA_ISLANDS_CC = 222,

         /// WESTERN_SAHARA,
         WESTERN_SAHARA_CC = 223,

         /// YEMEN,
         YEMEN_CC = 224,

         /// ZAMBIA,
         ZAMBIA_CC = 225,

         /// ZIMBABWE,
         ZIMBABWE_CC = 226,

         /// ZIMBABWE,
         SAN_MARINO_CC = 227,

         /// Number of country codes
         NBR_COUNTRY_CODES
            
      };

  public:
      /**
       *  All country codes. (Phone number country prefixes)
       */
      static const char* countryCodes[][2];

   private :
   
      /**
       *    The strings in latin-1.
       */
#ifndef MC2_UTF8
      static vector<vector<MC2String> > latinStrings;
      
      /**
       *    True if the latin strings has been initiated.
       */
      static bool initiated;
      
      /**
       *   Initiation of the latin strings.
       */
      static bool initLatinStrings();
#endif
      
   public:

      
      
      /**
        *   @param   stringCode     The wanted string.
        *   @param   languageCode   In what language we want the string.
        *   @return  pointer to the string, NULL if invalid parameters.
        */
      static const char *getString(stringCode sc, languageCode lc);
      
      /**
        *   @param   stringCode     The wanted string.
        *   @param   languageCode   In what language we want the string.
        *   @return  pointer to the string, NULL if invalid parameters.
        */
      static const char *getString(stringCode sc, const LangType& langType,
                                   bool smsish = false );

      

      /**
        *   Method to be used when printing routedescription that
        *   contains the exit count. E.g. "Take the 2:nd street to 
        *   the left".
        *
        *   @param   sc    The stringcode that describes the turn.
        *   @param   cc    In what language we want the string.
        *   @param   exitC The number of the exit, if 0 the number is not
        *                  printed.
        *   @param   destStr  Pointer to an allocated buffer where
        *                     the resulting string is written.
        *   @param   maxSize  The maximum number of bytes written
        *                     into dest.
        *   @param   endOfRoad If this is true the result becomes
        *                     "Turn ???? at the end of the street"
        *   @param   hasName  Set this to true if the road in the turn
        *                     description has a name, false otherwise.
        *   @return  The number of characters written into dest.
        */
      static uint32 getRouteDescription(stringCode sc, languageCode lc, 
                                        uint32 exitCount,
                                        char* dest, int maxSize,
                                        bool hasName = true,
                                        bool endOfRoad = false);

      /**
        *
        */
      static uint32 getSerialNumber(uint32 i, languageCode lc, char* dest);

      /**
        *   All the possible languagecodes. Is nbrLanguages long.
        *   @return  A vector with all supported languagecodes. 
        *            Don't delete!
        */
      static const languageCode* getLanguages();


      /**
       * Checks if a languageCode is a normal langugage.
       * @param lc is the languageCode to check.
       * @return true if lc is a normal language.
       */
      static bool isLanguage( languageCode lc );


      /**
        *   All the possible languages as strings codes. Same order as in 
        *   getLanguage.
        *   @return  A vector with all supported language strings.
        *            Don't delete!
        */
      static const stringCode* getLanguagesAsStringCode();

      static stringCode getLanguageAsStringCode(languageCode lc);


      
      /**
        *   All the possible languages as strings. Same order as in 
        *   getLanguage.
        *   @return  A vector with all supported language strings.
        *            Don't delete!
        */
      static const char** getLanguagesAsString();

      
      /**
        *   @return  The number of languages.
        */
      static uint32 getNbrLanguages() {   
         return nbrLanguages; 
      }


      /**
        *   @return  The number of pseudolanguages.
        */
      static uint32 getNbrPseudoLanguages() { 
         return nbrPseudoLanguages; 
      }


      /**
        *   All pseudolanguages as languagecodes. Is 
        *   nbrPseudoLanguages long.
        *   @return  A static vector with all pseudoLanguages. 
        */
      static const languageCode* getPseudoLanguages();


      /**
       * Checks if a languageCode is a short langugage.
       * @param lc is the languageCode to check.
       * @return true if lc is a short language.
       */
      static bool isShortLangugage( languageCode lc );


      /**
       *  Tries to match a string with a languageString and return its 
       *  languageCode.
       * @param language is the string with the langugage name.
       * @param defaultCode is the defualt value to return if no
       *        languageString matching language is found.
       */
      static const languageCode getLanguageCode( const char* language,
                                                 languageCode defaultCode
                                                 = ENGLISH );


      /**
       *  Tries to match a languageCode and return its 
       *  short language (SMSISH) languageCode.
       * @param language is the langugage to convert.
       */
      static const languageCode getShortLanguageCode( languageCode code );


      /**
       *  Tries to match a languageCode and return its 
       *  Normal language languageCode.
       * @param language is the langugage to convert.
       */
      static const languageCode getNormalLanguageCode( languageCode code );


      /**
       * Get the name of a month.
       *
       * @param monthIndex which month (1-12).
       * @param language the language to use.
       * @return name of the month or "UNKNOWN" if index is out of bounds.
       */
      static const char* getMonthString( uint32 monthIndex, languageCode lc);


      /**
       * Get the name of a weekday.
       *
       * @param weekdayIndex which weekday (0-6).
       * @param language the language to use.
       * @return name of the weekday or "UNKNOWN" if index is out of bounds.
       */
      static const char* getWeekdayString( uint32 weekdayIndex,
                                           languageCode lc);


      /**
       * Tries to get the language from the phonenumbers country code.
       *
       * @param phoneNumber The phonenumber to check.
       * @param defaultLanguage The language to return if nothing better
       *                        can be found.
       * @return The language of the phone or defaultLanguage.
       */
      static languageCode getLanguageFromPhonemumber( 
         const char* phoneNumber,
         languageCode defaultLanguage = ENGLISH );

      /**
       *    Get the string code for a country with given country code.
       *    Usefull when printing the name of the country.
       *
       *    @param  cc   The country code of the country.
       *    @return The string code for the country with country code cc.
       */
      static stringCode getCountryStringCode(countryCode cc);

      /**
       *    Get a printable string for a country with given country code.
       *    To be used when printing the name of the country in maps an
       *    map images.
       *    It is an improvement of the getCountryStringCode, in the way
       *    that it translates the bad country stringCodes to better ones
       *    (e.g. United Kingdom is used instead of England).
       *
       *    @param  cc   The country code of the country.
       *    @param  lc   In which language the string is wanted.
       *    @return The string for the country with country code cc.
       */
      static const char* getStringToDisplayForCountry(
                              countryCode cc, languageCode lc);

      /**
       * Gets the phone number country code prefix.
       * For example, sweden is 46.
       * @param cc The country.
       * @return The country code prefix.
       */
      static const char* getCountryPhoneCode(StringTable::countryCode cc);

      /**
       *   Crashes the program if there is a string that is missing
       *   in English.
       */
      static bool sanityCheck();

      /**
       *   Set to true by calling sanityCheck.
       */
      static const bool m_sanityCheckRun;
      
};

/**
 *   Class holding a string code.
 *   Has the advantage that it can be forward declared.
 */
class StringCode {
public:
   typedef StringTable::stringCode stringCode_t;

   /// Creates a new StringCode that defaults to NOSTRING.
   inline StringCode( ) : m_stringCode( StringTable::NOSTRING ) {}
   
   /// Creates new service.
   inline StringCode( stringCode_t stringCode ) : m_stringCode( stringCode ) {}

   /// Returns the service as a service_t.
   inline operator stringCode_t() const { return m_stringCode; }
   
private:
   /// The service of this ExtService.
   stringCode_t m_stringCode;
};

/**
 * Placeholder for StringTable::languageCode. Use this
 * when you need to forward declare language code.
 */
class LanguageCode {
public:
   inline LanguageCode():
      m_langCode( StringTable::ENGLISH ) { }

   inline LanguageCode( StringTable::languageCode code ):
      m_langCode( code ) { }

   /// "Convert" to language code
   inline operator StringTable::languageCode() const { return m_langCode; }
private:
   StringTable::languageCode m_langCode;
};

/**
 * Placeholder for StringTable::countryCode. Use this
 * when you need to forward declare country code.
 */
class CountryCode {
public:
   CountryCode():
      m_country( StringTable::NBR_COUNTRY_CODES ) {
   }

   CountryCode( StringTable::countryCode code ):
      m_country( code ) {
   }

   /// "Convert" to country code.
   inline operator StringTable::countryCode() const { return m_country; }

private:
   StringTable::countryCode m_country;
};

#endif // STRINGTABLE_H

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LANG_TYPES_H
#define LANG_TYPES_H

#include "config.h"

#include <string.h>

#include <map>

/**   
 *   Contains type for languages in maps (items).
 */
class LangTypes {
  public:
      // ===============================================================
      //                                                 LanguageTypes =
      // ===============================================================
      
      /**
        *   The possible languages of the names. 
        *   @see name_t   The type of thet name is specified by the name_t.
        *   @see StringTable::languageCode! Maintain the same order.
        *   When a new language is added here, languageAsString[][6]
        *   must be updated. Maintain the same order!
        *
        *   Smaller languages in Tele Atlas countries.
        *   andorra:     CAT
        *   austria:     SLV
        *   ireland:     GLE
        *   italy:       ALB
        *   italy:       SCR
        *   italy:       SLV
        *   luxembourg:  LTZ
        *   netherlands: FRY
        *   spain:       BAQ
        *   spain:       CAT
        *   spain:       GLG
        *   spain:       VAL
        *   switzerland: ROH
        */

      enum language_t {
         /** English */
         english = 0, // Maintain the order.

         /** Swedish */
         swedish = 1, // Maintain the order.

         /** German */
         german = 2, // Maintain the order.

         /** Danish */
         danish = 3, // Maintain the order.

         /** Italian */
         italian = 4, // Maintain the order.

         /** Dutch */
         dutch = 5, // Maintain the order.

         /** Spanish */
         spanish = 6, // Maintain the order.

         /** French */
         french = 7, // Maintain the order.

         /** Welch */
         welch = 8, // Maintain the order.

         /** Finnish */
         finnish = 9, // Maintain the order.
         
         /** Norwegian */
         norwegian = 10, // Maintain the order.
         
         /** Portuguese */
         portuguese = 11, // Maintain the order.

         /** American English */
         american = 12, // Maintain the order.
         
         /** Czech */
         czech = 13, // Maintain the order.

         /** ALB Albanian */
         albanian = 14, // Maintain the order.
         
         /** BAQ Basque */
         basque = 15, // Maintain the order.
         
         /** CAT Catalan */
         catalan = 16, // Maintain the order.
         
         /** FRY Frisian */
         frisian = 17, // Maintain the order.
         
         /** GLE Irish */
         irish = 18, // Maintain the order.
         
         /** GLG Galician */
         galician = 19, // Maintain the order.
         
         /** LTZ Letzeburgesch */
         letzeburgesch = 20, // Maintain the order.
         
         /** ROH Raeto-Romance */
         raetoRomance = 21, // Maintain the order.
         
         /** SCR Serbo-Croatian (Roman) */
         serboCroatian = 22, // Maintain the order.
         
         /** SLV Slovenian */
         slovenian = 23, // Maintain the order.
         
         /** VAL Valencian */
         valencian = 24, // Maintain the order.

         /** HUN Hungarian */
         hungarian = 25, // Maintain the order.

         /** GRE Greek, Modern */
         greek = 26, // Maintain the order.
         
         /** POL Polish  */
         polish = 27, // Maintain the order.

         /** SLO Slovak */
         slovak = 28,  // Maintain the order.

         /** RUS Russian */
         russian = 29,  // Maintain the order.

         /** GRL Greek, Modern Latin Syntax */
         greekLatinStx = 30,

         /** Invalid language 
           * NB" Do not use this one for finding out how many languages 
           * there are.
           *
           * Names in languages not supported by mc2 gets this language 
           * type.
           */
         invalidLanguage = 31, // Maintain the order.

         /** RUL Russian Latin Syntax */
         russianLatinStx = 32, // Maintain the order.

         /** TUR Turkish */
         turkish = 33, // Maintain the order.

         /** ARA Arabic */
         arabic = 34, // Maintain the order.

         /** CHI */
         chinese = 35,

         /** CHL */
         chineseLatinStx = 36,

         /** EST Estonian */
         estonian = 37,

         /** LAV Latvian*/
         latvian = 38,

         /** LIT Lithuanian*/
         lithuanian = 39,

         /** THA Thai language*/
         thai = 40,

         /** BUL Bulgarian language*/
         bulgarian = 41,

         /** CYT Cyrillic transcript. Used by AND when printing cyrillic
             names with latin characters.
         */
         cyrillicTranscript = 42,

         /** IND Indonesian language */
         indonesian = 43,

         /** MAY Malay language */
         malay = 44,

         /// Icelandic
         icelandic = 45,

         /// Japanese
         japanese = 46,

         /// Amharic
         amharic = 47,

         /// Armenian
         armenian = 48,

         /// Tagalog
         tagalog = 49,

         /// Belarusian (cyrillic)
         belarusian = 50,

         /// Bengali
         bengali = 51,

         /// Burmese
         burmese = 52,

         /// Croatian
         croatian = 53,

         /// Farsi
         farsi = 54,

         /// Gaelic
         gaelic = 55,

         /// Georgian
         georgian = 56,

         /// Gujarati
         gujarati = 57,

         /// Hebrew
         hebrew = 58,

         /// Hindi
         hindi = 59,

         /// Kannada (Language in India)
         kannada = 60,

         /// Kazakh
         kazakh = 61,

         /// Khmer
         khmer = 62,

         /// Korean
         korean = 63,

         /// Lao
         lao = 64,

         /// Macedonian (cyrillic)
         macedonian = 65,

         /// Malayalam
         malayalam = 66,

         /// Marathi
         marathi = 67,

         /// moldavian
         moldavian = 68,

         /// Mongolian
         mongolian = 69,

         /// Punjabi
         punjabi = 70,

         /// Romanian
         romanian = 71,

         /// Serbian
         serbian = 72,

         /// Sinhalese
         sinhalese = 73,

         /// Somali
         somali = 74,

         /// Swahili
         swahili = 75,

         /// Tamil
         tamil = 76,

         /// Telugu
         telugu = 77,

         /// Tibetan
         tibetan = 78,

         /// Tigrinya
         tigrinya = 79,

         /// Turkmen
         turkmen = 80,

         /// Ukrainian
         ukrainian = 81,

         /// Urdu
         urdu = 82,

         /// Vietnamese
         vietnamese = 83,

         /// Zulu
         zulu = 84,

         /// Sesotho
         sesotho = 85,

         /// Bulgarian latin syntax 
         bulgarianLatinStx = 86,
         
         /// Bosnian 
         bosnian = 87,
         
         /// Slavic 
         slavic = 88,
         
         /// Belarusian Latin
         belarusianLatinStx = 89,

         /// Macedonian Latin
         macedonianLatinStx = 90,

         /// Serbian Latin
         serbianLatinStx = 91,

         /// Ukrainian Latin
         ukrainianLatinStx = 92,
         
         /// Maltese
         maltese = 93,

         /// Chinese, traditional
         chineseTraditional = 94,

         /// Chinese traditional for Hong Kong
         chineseTradHongKong = 95,

         /// Thai Latin
         thaiLatinStx = 96,
         

         // Add new languages here.

         // Make sure to update LangTypes::languageAsString as well.

         
         /** Number of languages */
         nbrLanguages // Keep last.

      };
      
   /**
    *   Get a string that describes this language-type. The string
    *   returned is short or full, eg swedish will return "swe" or
    *   "swedish".
    *   @param langType  The language type to return a string for.
    *   @param fullName  Default false. If the name of the language
    *                    should be the full name.
    *   @return The string describing the language type according 
    *           to the parameters.
    */   
   static const char* getLanguageAsString(LangTypes::language_t langType,
                                          bool fullName = false );

   /**
    *   Get the language described with a string ignoring case,
    *   eg "swe" or "SWE" (only mc2, otherwise it is case sensitive
    *   ). If <code>fullName</code> is true, the
    *   full name will be matched instead. The language as string
    *   should always be in English.
    *   @param    langType    The string.
    *   @param    fullName    Use e.g. "swedish" instead of "swe"
    *   @return   The language type, 
    *             if no match invalidLanguage is returned.
    */
   static LangTypes::language_t getStringAsLanguage(const char* langType,
                                                    bool fullName = false);
   
   /**
    *   Get the iso639 string for this language-type.
    *
    *   @param langType The language type to return a string for.
    *   @param two If two letter or three letter abbreviation.
    *   @return The iso639 string for the language type.
    */
   static const char* getLanguageAsISO639( LangTypes::language_t langType, 
                                           bool two = true );

   /**
    *   Returns the language_t for the specified iso639 string.   
    *   The string can consist of two or three letters and must be
    *   NULL terminated.
    *   @param isoStr The string, containing two or three letters.
    *   @return invalidLanguage if not found, or invalid length of 
    *           the string.
    */
   static language_t getISO639AsLanguage( const char* isoStr );

   /**
    *   Returns the language_t for the specified iso639 string.   
    *
    *   @param isoStr The string to find in the selected iso list.
    *   @param dashOne If to look in ISO 639-1 codes or in ISO 639-3.
    *   @return invalidLanguage if not found, or invalid length of 
    *           the string.
    */
   static language_t getISO639AsLanguage( const char* isoStr, bool dashOne );

   /**
    * Get the iso639 string and optional coutry dialect, in ISO 3166-1 
    * alpha-3 for this language-type.
    *
    * @param langType The language type to return a string for.
    * @return The string for the language type.
    */
   static const char* getLanguageAsISO639AndDialect( 
      LangTypes::language_t langType );

   /**
    * Returns the language_t for the specified iso639-2 string with 
    * optional country dialect in ISO 3166-1 alpha-3.
    * 
    * @param str The string.
    * @return invalidLanguage if not found, or invalid length of 
    *         the string.
    */
   static language_t getISO639AndDialectAsLanguage( const char* isoStr );


   /**
    * Get the Nav language as language_t.
    */
   static language_t getNavLangAsLanguage( uint32 language );


   static const char* getUtf8LocaleAsString(LangTypes::language_t langType);
   /**
    *    Struct used for comparing character strings.
    *    Same as in Utility.h, but added again to avoid
    *    dependancies.
    */
   struct ltstrnocase {
      bool operator()( const char* s1, const char* s2 ) const
         {
            return strcasecmp(s1, s2) < 0;
         }
   };


   /**
    *   Type of map to keep the translations to from strings in.
    */
   typedef map<const char*,
               language_t, ltstrnocase> strMap_t;
   
   
private:

#ifndef __SYMBIAN32__
   /// Table from short string to language
   static strMap_t c_stringAsLanguageShort;

   /// Table from full string to language
   static strMap_t c_stringAsLanguageFull;
#endif
   
   /**
    * @name Static data structures and members used for
    *       getLanguageAsString() and getStringAsLanguage().
    */
   //@{
      /**
       * Table consisting of short name, long name, ISO 639-1 code,
       * ISO 639-2 code, and preferred UTF-8 locale of the
       * languages. The order of language must be the same as in
       * language_t.
       */
       static const char* const languageAsString[][6];

#ifndef __SYMBIAN32__
      /**
       *   Method initializing stringAsLanguage. Run once only.
       *   Also checks if the number of itemTypes is 25.
       */
      static bool initStringAsLanguage();

      /**
       *   Dummy variable used when calling initStringAsLanguage().
       *   Needed by the compilator.
       */
      static bool stringAsLanguageInitialized;
#endif
   //@}


};

#ifdef MC2_SYSTEM
class LangType {
public:
   typedef LangTypes::language_t language_t;

   /// Creates a new LangType that defaults to invalidLanguage
   inline LangType( ) : m_lang ( LangTypes::invalidLanguage ) {}
   
   /// Creates new LangType
   inline LangType( LangTypes::language_t lang ) : m_lang(lang) {}

   /// Returns the service as a service_t.
   inline operator language_t() const { return m_lang; }
   
private:
   /// The service of this LangTyp
   language_t m_lang;
};

#endif

#endif


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ABBREVIATIONTABLE_H
#define ABBREVIATIONTABLE_H

#include "config.h"
#include "StringTable.h"
#include "LangTypes.h"

#include <vector>
#include "MC2String.h"

/**
  *   This class contains all abbreviation strings that are used for the
  *   (dynamic) strings in the MapCentral system. E.g (in Swedish) 
  *   "GATAN" is abbreviated as "G.". Here is also methods to expand a 
  *   string with abbreviations to its full name, e.g. (in Swedish)
  *   "Ö VALLG." is expanded to "ÖSTRA VALLGATAN".
  *
  *   All functions are declaered static.
  *
  *   <I><B>NB!</B> Currently a linear-search is done among all the
  *   strings. As long as the number of abbreviations are small this
  *   does not matter, but if this increase this must be optimized.
  *   For example by building a better/faster datastructure runtime.</I>
  *
  */
class AbbreviationTable {
   public :
      /**
        *   The allowed position of the abbreviation/expansion in the string.
        *   Used in the static abbreviation-table.
        */
      enum pos_t {
         /** 
           *   The full name of the abbreviation must come first in the
           *   source string.
           */
         beginning = 0,

         /** 
           *   The full name of the abbreviation must come last in the
           *   source string.
           */
         end = 1,

         /**
          *    The full name of the abbreviation can come either first
          *    or last in the source string.
          */
         beginningAndEnd = 2,

         /** 
          *   The full name of the abbreviation can come everywhere in
          *   in the string, except first and last in the string..
          */
         centre = 3,

         /**
          *   The full name of the abbreviation can come everywhere in
          *   the string.
          */
         anywhere = 4,
         
         /** 
           *   The full name of the abbreviation must be a full word 
           *   (with whitspaces on both sides) in the source string.
           */
         word = 5       
      };

   
      /**
        *   Abbreviate a given string with a given language. The 
        *   destination string must be at least as big as the source
        *   string. <I><B>NB!</B> No check that the result fits into
        *   the destination string is performed, but it is guarenteed
        *   that the result not is longer than the source.</I>
        *
        *   @param   src   The source string that contains the string
        *                  that should be abbreviated.
        *   @param   dest  A preallocated buffer where the result is
        *                  written.
        *   @param   lang  The language of the source string.
        *   @param   pos   Which part of the string that is going to be
        *                  abbreviated.
        *   @param   country  In which country the source string is located,
        *                  detecting country specific abbreviation.
        *                  Default is NBR_COUNTRY_CODES = no specific country.
        *   @return  A pointer to the destination string.
        */
      static char* abbreviate(const char* src, char* dest,
                              LangTypes::language_t lang,
                              AbbreviationTable::pos_t pos,
                              StringTable::countryCode country = 
                                       StringTable::NBR_COUNTRY_CODES);

      /**
        *   Expand the abbreviations in a string and print the result 
        *   into a new string.
        *
        *   @param   src      The source string containing the abbreviations.
        *   @param   dest     A preallocated buffer where the result
        *                     is written.
        *   @param   maxLen   The maximum number of charactes that will
        *                     be written into dest. 
        *   @param   lang     The language of the source string.
        *   @param   pos      Which part of the string that is going to be
        *                     expanded.
        *   @param   country  In which country the source string is located,
        *                     detecting country specific abbreviation.
        *                     Default is NBR_COUNTRY_CODES =no specific country.
        *   @param manageAddress Default set to true, the string to expand is 
        *                     an address (m_expansions table will be used). 
        *                     Set false if the string is an area (e.g. city 
        *                     or municipal) and the area expansions table 
        *                     should be used instead.
        *   @return  A pointer to the destination string. NULL is returned
        *            if the result did not fit inte dest (the resulting
        *            string is (would have been) longer than maxLen bytes).
        */
      static char* expand(const char* src, char* dest, size_t maxLen,
                          LangTypes::language_t lang,
                          AbbreviationTable::pos_t pos,
                          StringTable::countryCode country = 
                                    StringTable::NBR_COUNTRY_CODES,
                          bool manageAddress = true);

      /**
       *    Old expand method. Will be removed since there is
       *    another better expand method. For param documentation,
       *    see the expand method.
       */
      static char* oldExpand(const char* src, char* dest, size_t maxLen,
                          LangTypes::language_t lang,
                          AbbreviationTable::pos_t pos,
                          bool &expanded);
      
      /**
       *    Expands to all possible expansion alternatives, returning a vector.
       *    The source string is split in single words and each word
       *    expanded to all possibilities considering language and country.
       *    The expanded single words are joined to several strings with
       *    all possible expansion alternatives.
       *    
       *    @param   src      The source string to expand.
       *    @param   lang     The language of the source string.
       *    @param   country  The country in which to expand. Default is
       *                      numberCountryCodes meaning no country.
       *    @param manageAddress Default set to true, the string to expand is 
       *                      an address (m_expansions table will be used). 
       *                      Set false if the string is an area (e.g. city 
       *                      or municipal) and the area expansions table 
       *                      should be used instead.
       *    @return  A vector of strings containing the possible expansions
       *             of the source string. Only strings not equal to the 
       *             src string are added to this vector.
       */
      static vector<MC2String> fullExpand(
                              const char* src, LangTypes::language_t lang,
                              StringTable::countryCode country = 
                                             StringTable::NBR_COUNTRY_CODES,
                              bool manageAddress = true);
      
  private:
      /**
        *   Record used when shortening a string. 
        *   The static abbreviationtable and expansiontable 
        *   consists of structs like this.
        */
      struct abbreviation_t {
         /** 
           *   The language of this abbreviation.
           */
         LangTypes::language_t lang;
         
         /** 
           *   The country where this abbreviation is valid.
           *   If valid in all countries default is NBR_COUNTRY_CODES.
           */
         StringTable::countryCode country;

         /**
           *   The full string of the abbreviation, e.g. (in Swedish) 
           *   "GATAN".
           */
         const char* fullString;

         /** 
           *   The string that should be replace the fullString. This 
           *   string must be shorter than the fullString, e.g. (in 
           *   Swedish "G").
           */
         const char* abbreviation;

         /**
           *   The allowed position of the fullString.
           *   @see  pos_t
           */
         pos_t pos;
      };

      /**
        *   Record used when shortening a string. 
        *   The static abbreviationtable and expansiontable 
        *   consists of structs like this.
        */
         struct abbreviation_tommi_t {

         const abbreviation_tommi_t& operator=( const abbreviation_t& abb ) {
            lang         = abb.lang;
            country      = abb.country;
            fullString   = abb.fullString;
            abbreviation = abb.abbreviation;
            pos          = abb.pos;
            return *this;
         }
            
         
         /** 
           *   The language of this abbreviation.
           */
         LangTypes::language_t lang;
         
         /** 
           *   The country where this abbreviation is valid.
           *   If valid in all countries default is NBR_COUNTRY_CODES.
           */
         StringTable::countryCode country;

         /**
           *   The full string of the abbreviation, e.g. (in Swedish) 
           *   "GATAN".
           */
         MC2String fullString;

         /** 
           *   The string that should be replace the fullString. This 
           *   string must be shorter than the fullString, e.g. (in 
           *   Swedish "G").
           */
         MC2String abbreviation;

         /**
           *   The allowed position of the fullString.
           *   @see  pos_t
           */
         pos_t pos;
      };

      /**
        *   @name Abbreviation tabular.
        *   The abbreviations for addresses that are used in this class.
        */
      //@{
         /**
           *   The abbreviations, the values in the array are static in the
           *   cpp-file.
           */
         static const abbreviation_t m_abbreviations_orig[];

         /**
           *   A constant describing the number of abbreviations in the 
           *   m_abbreviations-array.
           */
         static const uint32 m_nbrAbbreviations;

         /**
          *    Vector consisting of the abbreviations converted
          *    to MC2Strings.
          */
         static const vector<abbreviation_tommi_t> m_abbreviations;
          
         
      //@}

      /**
        *   @name Expansion tabular.
        *   The expansions for addresses that are used in this class.
        */
      //@{
         /**
           *   The expansions, the values in the array are static in the
           *   cpp-file.
           */
         static const abbreviation_t m_expansions_orig[];

         /**
           *   A constant describing the number of expansion in the 
           *   m_expansions-array.
           */
         static const uint32 m_nbrExpansions;
        
         /**
          *    Vector consisting of the expansions converted
          *    to MC2Strings.
          */
         static const vector<abbreviation_tommi_t> m_expansions;

         /**
           *   The character that is used as "any non alpha numeric 
           *   character" in m_expansions-array.
           */
         static const char m_nonAlnumChar;
      //@}

      /**
        *   @name Area expansion tabular.
        *   The expansions for areas (cities) that are used in this class.
        */
      //@{
         /**
           *   The expansions, the values in the array are static in the
           *   cpp-file.
           */
         static const abbreviation_t m_areaExpansions[];

         /**
           *   A constant describing the number of expansion in the 
           *   m_areaExpansions-array.
           */
         static const uint32 m_nbrAreaExpansions;
      //@}

      /**
       *    Returns true if the abbrevation/expansion matches.
       *    @param origStr     String to look in.
       *    @param foundPos    Pointer to the place where long/short
       *                       string was found.
       *    @param abbrevPos   The position where the abbrev is allowed, from
       *                       the m_abbreviations/m_expansions table.
       *    @param abbrevElement The element from the 
       *                         m_abbreviations/m_expansions table to check.
       *    @param toAbbreviate Tells whether to check matching for 
       *                        abbreviation or for expansion of the string.
       *    @param singleWordExpand Set to True if the origString is one 
       *                        single word. Default value is False, the
       *                        origStr must not be a single word.
       *    @param manageAddress Default is true, the string to manage is
       *                        address and m_abbreviations or m_expansions
       *                        tables will be used. Set false if the string
       *                        to check is an area (e.g. city or municipal)
       *                        and the area abbrev and expand tables should
       *                        be used instead.
       *    @return True if the checks were ok.
       */
      static bool abbrevMatches(const char* origStr,
                                const char* foundPos,
                                const char* foundAbbrev,
                                pos_t abbrevPos,
                                uint32 abbrevNbrInTable,
                                bool toAbbreviate,
                                bool singelWordExpand = false,
                                bool manageAddress = true);

      /**
       *    Returns true if the inPos allows abbrevations/expansions 
       *    of type tablePos.
       *    @return true (for now).
       */
      static bool abbrevAllowed(pos_t inPos, pos_t tablePos,
                                bool toAbbreviate);
      
      /**
        *   Get the position of the abbreviation to expand in a string.
        *
        *   @param   src   The unexpanded string.
        *   @param   i     The index of the abbreviation to check.
        *                  Valid values are 0..m_nbrExpansions-1.
        *   @return  The position of abbreviation number i in src. A 
        *            negative value is returned if abbreviation number i
        *            don not fit in src.
        */
      static int getExpandPosition(const char* src, uint32 i,
                                   AbbreviationTable::pos_t pos);

      /**
        *   Replacement of the strncmp-method that compares the first
        *   n characters in two strings, but also use the m_nonAlnumChar.
        *   If any of the parameters are NULL, 0 will be returned!
        *
        *   @param   s1 One of the strings that will be compared.
        *   @param   s2 One of the strings that will be compared.
        *   @param   n  The number of characters in s1 and s2 that 
        *               (maximum) will be compared.
        *   @return  A negative value of s1 is less than s2, zero if
        *            s1 == s2 and a positive value of s1 is greater 
        *            than s2.
        */
      static int strncmp_alnum(const char* s1, const char* s2, size_t n);
      
      /**
       *    Help method for expanding. Serves both the normal expand,
       *    and the fullExpand funtion.
       *    @param   src      The source string containing the abbreviations.
       *    @param   dest     A preallocated buffer where the result
       *                      is written.
       *    @param   maxLen   The maximum number of charactes that will
       *                      be written into dest. 
       *    @param   lang     The language of the source string.
       *    @param   country  In which country the source string is located,
       *                      detecting country specific abbreviation. If no
       *                      specific country give NBR_COUNTRY_CODES.
       *    @param   pos      Which part of the string that is going to be
       *                      expanded.
       *    @param   singleWordExpand  Tells if the origString is one 
       *                      single word (true), or not.
       *    @param  expandedStrings    A vector of strings that will contain
       *                      the possible expansions of the source string.
       *    @param manageAddress Set to true if the string to expand is an
       *                      address (m_expansions table will be used). 
       *                      Set false if the string is an area (e.g. city 
       *                      or municipal) and the area expansions table 
       *                      should be used instead.
       *
       *    @return  A pointer to the destination string. NULL is returned
       *             if the result did not fit inte dest (the resulting
       *             string is (would have been) longer than maxLen bytes).
       */
      static char* helpExpand(const char* src, char* dest, size_t maxLen,
                          LangTypes::language_t lang,
                          StringTable::countryCode country,
                          AbbreviationTable::pos_t pos,
                          bool singleWordExpand,
                          vector<MC2String>& expandedStrings,
                          bool manageAddress);

      /**
       *    Converts the entries in a table to the ones using MC2String.
       */
      static vector<abbreviation_tommi_t>
         convAbbrev(const abbreviation_t* table, int nbrAbb);

};

#endif


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGSEARCHUTILITY_H
#define STRINGSEARCHUTILITY_H

// Includes
#include "config.h"

#include <vector>
#include <set>

#include "Types.h"
#include "ItemTypes.h"
#include "StringUtility.h"
#include "TextConversionTables.h"
#include "TextIterator.h"
#include "UTF8Util.h"

class LangType;

/**
  *   Some useful string functions mainly used by searchmodule.
  *
  */
class StringSearchUtility {
public:
   /**
    *    Functor to use for sorting strings in descending
    *    length order.
    */
   class DescendingStringLength {
   public:
      inline bool operator()(const char* a,
                             const char* b) {
         return strlen(a) > strlen(b);
      }
   };
   
   /**
    *  Gets the almost soundex encoded string from a
    *  normal string.
    *  @param source The string to translate.
    *  @param len The lenght of the source string.
    *  @param language The language to use while encoding.
    *  @param resultLen The length of the resulting string.
    *  @return The new encoded string. Don't forget to delete.
    */
   static int getSoundexString(char* buffer, int buffSize,
                               const char* source,
                               const LangType& language,
                               uint32& resultLen);

   /**
    *  Compares a soundexencoded string to a normal string,
    *  using soundex comparison.
    *  @param s1 The normal string.
    *  @param soundexStr The soundex string.
    *  @param soundexStrLen The length of the soundex string.
    *  @return True if same up to the length of the soundex string,
    *          false if not.
    */
   static bool compareSoundexString(char* buffer, int buffSize,
                                    const char* s1,
                                    const char* soundexBuffer,
                                    uint32 soundexStrLen,
                                    const LangType& language);


   /**
    *   Returns the uk postal code with one group removed.
    *   N35A 6QW -> N35A 6 -> N35A -> N
    *   N3 5AQ -> N3 5 -> N3 -> N
    */
   static MC2String removeUKPostalCodeGroup( const MC2String& code );

   /**
    *   @param code The postal code to calculate number of groups of.
    *   @return Number of levels in the postal code supplied in code.
    *           E.g. "N35A 6QW" -> 4, "N3 5" -> 3, "N" -> 1
    */
   static uint32 getUKPostalCodeNbrGroups( const MC2String& code );

   /**
    * Calculates the Levenstein edit distance.
    * The code was originally published by Gertjan van Noord.
    * Only minor modifications here.
    * @param a String 1.
    * @param b String 2.
    * @param maxDistance The cutoff distance.
    * @return The distance.
    */
   static int calculateEditDistance(const char* a,
                                    const char* b,
                                    uint32 maxDistance);
   
   /**
    * Checks language group.
    * @param requestedLanguage The reqed lang.
    * @param language The lang.
    * @return Some measure of closeness.
    */
   static uint32 sameLanguageGroup(uint32 requestedLanguage,
                                   uint32 language);
   
   /**
    * Calcs score for a string language and string type.
    * @param language The language.
    * @param requestedLanguage The requested language.
    * @param strType The type if the string.
    * @return a score
    */
   static uint32 getStringTypeScore(uint32 language,
                                    uint32 requestedLanguage,
                                    uint32 strType);

   /**
    *    Returns true if the character c is one that
    *    divides strings into words.
    */
   static inline bool isWordSeparator(uint32 ucs);
   
   /**
    *   Converts the name string to exact ident for use in
    *   SearchModule.
    *   @param name String to convert.
    *   @return New string for use as exact ident.
    */
   static inline MC2String convertIdentToExact( const mc2TextIterator& name );
   
   /**
    *   Converts the name string into close ident for use in
    *   SearchModule.
    *   @param name String to convert.
    *   @param keepWordSeps True if word separators should be kept for
    *                       word-matching.
    *   @return New string for use as exact ident.
    */ 
   static inline MC2String convertIdentToClose( mc2TextIterator name,
                                                bool keepWordSeps = false);
   /**
    *   Copies all normal characters to a new string, and returns it.
    *   @param allChars     The string with strange characters to be 
    *                       removed.
    *   @param keepWordSeps Keep word separators in string.
    *   @return  Pointer to a new string with all normal characters
    *            from allChars.
    */
   static inline MC2String stripStrangeChars(const MC2String& allChars,
                                             bool keepWordSeps = false);
   
   /**
    *   Returns true if there is a street number in the beginning of the
    *   string.
    */
   static bool isFrontStreetNumber(const char* word);
   
   /**
    *   Returns true if there is a street number at the end of the string.
    */
   static bool isBackStreetNumber(const char* word);
   
   
   /**
    *   Gets the streetNumber and the strippedSearchString in
    *   the searchString.
    *   @param searchString The searchString.
    *   @param streetNumber A reference to the streetNumber.
    *   @param strippedSearchString A reference to the strippedSearchString.
    */      
   static void getStreetNumberAndSearchString(const MC2String& searchString,
                                              int& streetNumber,
                                              MC2String& strippedSearchString);

   /**
    *   A house number is considered simple if it contains only digits
    *   or if it contains only digits and an ending letter.
    *
    *   @param str     The string to check
    *   @param number  The parsed number
    */
   static bool isSimpleHouseNumber( const MC2String& str, int& number );

   /**
    *   Works like getStreetNumberAndSearchString but only splits simple
    *   strings so that we can be sure that the result looks good if we
    *   glue the stuff together again (regardless of whether the number
    *   should be first or last), also this function doesn't remove odd
    *   characters or converts accented to non-accented characters.
    *
    *   @param searchString The string to split.
    *   @param streetNumber A reference to the street number.
    *   @param streetName A reference to the street name.
    */
   static void simpleGetStreetNumberAndName( const MC2String& searchString,
                                             int& streetNumber,
                                             MC2String& streetName );

   /**
    *   Splits the incoming string into words using
    *   isWordSeparator.
    *   @param words A vector of pointers into the <code>str</code>.
    *   @param str   The string to split. Can be const char* or MC2String.
    */
   static void splitIntoWords(vector<MC2String>& words,
                              mc2TextIterator str );
   
   /**
    * Removes street number and anything else that might follow
    * the first digit.
    * If the first character is a digit, the leading digits are
    * skipped and the rest of the string is returned.
    *
    * @param searchString the searchstring e.g. 'tunav39b656'
    * @return the streetname (or whatever it is) without
    * numbers e.g. 'tunav'.
    */
   static inline MC2String stripSearchString( const MC2String& searchString );

   /**
    *   Makes a uppercase copy of str and returns it. Supports ISO8859-1.
    *   Also converts german sharp S into two ordinary S:s. Will probably
    *   allocate too much memory for the string every time, hence the
    *   number2.
    *   @param str is the string to copy and to upercase.
    *   @returns the new uppercase string.
    */
   static inline MC2String copyUpper2( mc2TextIterator it );

   /**
    *    Calculates the distance between two strings, if the distance
    *    is within a certain limit.
    *    Examples: $(a,b)$ = (Baravägen, Braavägen)  => distance = 2
    *                         
    *              $(a,b)$ = (Baravägen, Bravgen)    => distance = 2
    *                                                 
    *              $(a,b)$ = (Baravägen, Baravägenn) => distance = 1
    *                                                
    *    @param a,b       Words to calculate the distance between
    *    @param maxDist   Maximum allowed distance
    *
    *    @param return    The distance if it is less than or equal to maxDist,
    *                     -1 otherwise
    */
   static int getLevDist(const char* a, const char* b, uint16 maxDist);
   
   /**
    *   Same as the other getLevDist, but with precalculated lengths.
    *   @param alen The length of string a.
    *   @param blen The length of string b.
    */
   static int getLevDist(const char* a, int alen,
                         const char* b, int blen,
                         uint16 maxDist);
   
   /**
    *    Returns true if all the words in words are in itemName.
    *    @param words    The words. Longest word first.
    *    @param itemName The itemName.
    */
   static inline bool allWordsMatch(const vector<char*>& words,
                                    const char* itemName);

   /**
    *    Returns true if any of the words in fistName is an exact match 
    *    to any word in secondName.
    *
    *    @param firstName  The string including words to compare to the 
    *                      words in secondName.
    *    @param secondName The string including words to compare to the 
    *                      words in firstName.
    */
   static inline bool anyWordMatch( const MC2String& firstName,
                                    const MC2String& secondName );

   static inline const char* wordMatches( const char* word,
                                          const char* itemName,
                                          set<const char*>& positions );

   /**
    *    Returns true if the word is an exact match to one or more than one
    *    of the words in itemName.
    *
    *    @param word The word to look for in itemName.
    *    @param itemName The string to look for word in.
    */
   static inline bool wholeWordMatchOneWord( const MC2String& word,
                                             const MC2String& itemName);
                                             

   /**
    *    Returns true if the words in query match the
    *    words in itemName in correct order. (closeString)
    *    @param words The words to search for (matching already checked).
    *    @param itemName The name of the item.
    *    @param firstWordBegins True if the first word is in the beginning
    *                           of the string.
    */
   static inline bool wordsCorrectOrder(const vector<const char*> words,
                                        const char* itemName,
                                        bool* firstWordBegins = NULL);
      
  private:
   /** 
    *   Private constructor to avoid objects of this class.
    */
   StringSearchUtility();
};


inline MC2String
StringSearchUtility::copyUpper2( mc2TextIterator it )
{
   MC2String retVal;
   while( *it != 0 ) {
      uint32 ucs = *it;
      TextConversionTables::convTableFindRes result =
         TextConversionTables::findInTable(TextConversionTables::c_upperTable,
                                           ucs);
      if( result.resString != 0 ) {
         retVal += UTF8Util::utf8ToMc2(MC2String(result.resString));
      } else {
         retVal += UTF8Util::ucsToMc2(ucs);
      }
      ++it;
   }
   return retVal;
}

inline MC2String
StringSearchUtility::convertIdentToExact( const mc2TextIterator& it )
{
   return copyUpper2( it );
}


inline MC2String
StringSearchUtility::convertIdentToClose( mc2TextIterator it,
                                          bool keepWordSeps )
{
   MC2String exact = convertIdentToExact( it );
   return stripStrangeChars( exact, keepWordSeps );
}

         
inline MC2String
StringSearchUtility::stripStrangeChars(const MC2String& allChars, 
                                       bool keepWordSeps)
{
   MC2String retVal;
   mc2TextIterator it(allChars.c_str());
   while( *it != 0 ) {
      uint32 ucs = *it;
      if( keepWordSeps || !isWordSeparator(ucs) ) {
         TextConversionTables::convTableFindRes result =
            TextConversionTables::findInTable(
                                       TextConversionTables::c_removeStrange,
                                       ucs);
         if( result.resString != NULL ) {
            retVal += UTF8Util::utf8ToMc2(MC2String(result.resString));
         } else {
            retVal += UTF8Util::ucsToMc2(ucs);
         }
      }
      ++it;
   }
   return retVal;
}


inline bool
StringSearchUtility::isWordSeparator(uint32 ucs)
{
   return (!StringUtility::isAlNum(ucs));
}

MC2String
StringSearchUtility::stripSearchString( const MC2String& searchString )
{
   int streetNumber;
   MC2String strippedSearchString;
   StringSearchUtility::getStreetNumberAndSearchString(searchString,
                                                       streetNumber,
                                                       strippedSearchString);

   return strippedSearchString;
}

inline const char*
StringSearchUtility::wordMatches( const char* word,
                                  const char* itemName,
                                  set<const char*>& positions )
{
   const char* res = itemName;
   while ( res != NULL ) {
      res = strstr(res, word);
      if ( res == NULL ) {
         // We can return here, since the words cannot
         // be found.
         return NULL;
      }
      // Check if found before.
      if ( positions.find(res) == positions.end() ) {
         // Possible wordmatch candidate.
         const bool wordMatch =
            ( res == itemName ) || // First letter
            ( StringSearchUtility::isWordSeparator( *( res - 1) ) ); // A word
         if ( ! wordMatch ) {
            // Nothing
         } else {
            // New wordmatch that hasn't been found before.
            positions.insert(res);
            return res;
         }
      }
      // Next letter.
      ++res;
   }
   return NULL;
}


inline bool
StringSearchUtility::allWordsMatch(const vector<char*>& words,
                                   const char* itemName)
{
   // Note that the words must be sorted with descending length...
   // The words must also be cluc.
   int nbrWords = words.size();
   // Should work with pointers since they point into the same string.
   set<const char*> positions;
   
   // Check the words
   for( int i = 0; i < nbrWords; ++i ) {
      if ( wordMatches( words[i], itemName, positions) == NULL ) {
         return false;
      }
   }
   return true;
}

inline bool
StringSearchUtility::wordsCorrectOrder(const vector<const char*> words,
                                       const char* itemName,
                                       bool* firstWordBegins)
{
   return true;
   
   if ( firstWordBegins != NULL ) {
      *firstWordBegins = false;
   }
   
   MC2String closeItemName = 
      StringSearchUtility::convertIdentToClose(itemName);
   char* closeItemNameCopy = 
      StringUtility::newStrDup(closeItemName.c_str());
   
   char* lastPos = closeItemNameCopy - 1;
   for( vector<const char*>::const_iterator it = words.begin();
        it != words.end();
        ++it ) {
      char* foundPos = strstr(closeItemNameCopy, *it);
      if ( foundPos < lastPos ) {
         // Wrong direction return false
         delete [] closeItemNameCopy;
         return false;
      } else {
         // Check if the first word found is first in the string.
         if ( firstWordBegins != NULL &&
              it == words.begin() && foundPos == closeItemName ) {
            *firstWordBegins = true;
         }
      } 
      lastPos = foundPos;
   }
   delete [] closeItemNameCopy;
   
   return true;   
}

inline bool
StringSearchUtility::wholeWordMatchOneWord( const MC2String& word,
                                            const MC2String& itemName)
{
   vector<MC2String> words;
   splitIntoWords( words, itemName );
   
   uint32 i=0;
   bool match=false;
   while ( (i< words.size()) && !match ){
      match = 
         (StringUtility::strcmp( words[i].c_str(), word.c_str() ) == 0 );
      i++;
   }
   return match;
}

inline bool
StringSearchUtility::anyWordMatch( const MC2String& firstName,
                                   const MC2String& secondName )
{
   vector<MC2String> words1;
   splitIntoWords( words1, firstName );
   
   uint32 i=0;
   bool match=false;
   while ( (i< words1.size()) && !match ){   
      match = wholeWordMatchOneWord(words1[i], secondName);
      i++;
   }

   vector<MC2String> words2;
   splitIntoWords( words2, secondName );

   i=0;
   while ( (i< words2.size()) && !match ){   
      match = wholeWordMatchOneWord(words2[i], firstName);
      i++;
   }   
   return match;
}



#endif // STRINGSEARCHUTULITY_H 

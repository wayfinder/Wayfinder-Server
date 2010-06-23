/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Inludes
#include "config.h"

#include "StringSearchUtility.h"
#include "StringUtility.h"
#include "NewStrDup.h"
#include "LangTypes.h"

#include <iterator>
#include <boost/lexical_cast.hpp>


// 2 = just two languages for now...
const uint8 soundexTable[2][256] = {
   // english and everything else
   {0, // zero
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,2,3,0,1, 2,0,0,2,2,4,5,5,0,1, // 80
    2,6,2,3,0,1,0,2,0,2, 0,0,0,0,0,0,0,0,0,0, // 100
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0 // 255
   },
   // swedish
   {0, // zero
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,2,3,0,1, 2,0,0,2,2,4,5,5,0,1, // 80
    2,6,2,3,0,1,1,2,0,2, 0,0,0,0,0,0,0,0,0,0, // 100
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0 // 255
   }
};

int
StringSearchUtility::getSoundexString(char* buffer,
                                      int bufferSize,
                                      const char* source,
                                      const LangType& inLanguage,
                                      uint32& resultLen)
{
   char* bufferP = buffer;
   int length = 0;
   LangType language = inLanguage;

   if (language == LangTypes::swedish) {
      // special case for swedish:
      // first character is not special,
      // and w v both have soundex code 1.
      // use special soundex table for swedish
      language = LangTypes::swedish; // true already, but written for clarity.
      if (soundexTable[language][uint8(*source)] == 0) {
         *(bufferP++) = *source++; // first character is kept.
         length++;
      } // unless it has a soundex code.
   } else {
      // other language than swedish
      language = LangTypes::language_t(0); // FIXME: Support other languages
      //
      *(bufferP++) = *source++; // first character is kept.
      length++;
   }
   
   uint8 lastCode = 0;
   // checking against lenght in release version is not needed. bufsize should be much larger than any string.
   while ( (*source != '\0') && (length < (bufferSize-1)) ) {
      uint8 code = soundexTable[language][uint8(*source++)];
      if ((code != 0) && (code != lastCode)) {
         lastCode = code;
         *(bufferP++) = code;
         length++;
      }
   }
   *bufferP = '\255'; // termination character
   resultLen = bufferP-buffer; // excluding termination character
   
   return 0;
}

bool
StringSearchUtility::compareSoundexString(char* buffer,
                                          int buffSize,
                                          const char* s1,
                                          const char* soundexBuffer,
                                          uint32 soundexStrLen,
                                          const LangType& language)
{
   //
   bool result = true; // default
   // quick and dirty way
   uint32 ss1len = 0;
   getSoundexString(buffer, buffSize, s1, language, ss1len);
   for (uint32 i = 0; i < soundexStrLen; i++) {
      if (buffer[i] != soundexBuffer[i]) {
         result = false;
         break; // for
      }
   } // for
   return result;
}

const uint32 MAXLINE = 100; // no longer strings are allowed.

/*
  original code published by Gertjan van Noord
*/
int
StringSearchUtility::calculateEditDistance(const char* W, // database string
                                           const char* P, // query string
                                           uint32 maxDistance)
{
   /* possible todos (?)
    *  transposed characters. this will be even slower.
    *  optimize time. once users say ok.
    *  optimize scoring. for this we need some kind of
    *     real user misspelled - correct dictionary.
    *  weighting of scores based on phonetic similarity.
    *  introduce simple cutoff.
    *  introduce similarity matrix for
    *     character replacement penalty - bonuses. hm.
    *     e.g., to replace an m by an n should cost less than by an r.
    *     y-i
    *     å-o-u
    */
//   const uint32 transposedChars = 5; // this one isn't implemented
   const uint32 queryErase = 10;
   const uint32 queryInsert = 5;
   const uint32 queryReplace = 7;
   const uint32 queryBeginningPenalty = 6;
   
   int arr[MAXLINE][MAXLINE]; 
   
   // opt: no need to calculate query string length (every time too),
   // since we know that already.
   uint32 slb = strlen(P);
   uint32 sla = MIN(slb + maxDistance,
                    strlen(W));

   // W w[j]
   // P p[i]

   // opt: no need to init the matrix every time.
   // use the same frame every time.
   
   // init matrix
   uint32 j;
   uint32 i;
   for (j = 0; j <= sla; j++) { 
      arr[0][j] = j * queryInsert; 
   } 
   for (i = 0; i <= slb; i++) { 
      arr[i][0] = i * queryErase; 
   } 

   uint32 add;
   for (j = 1; j <= sla; j++) { 
      for (i = 1; i <= slb; i++) { 
         if (W[j-1] == P[i-1]) {
//            arr[i][j] = arr[i-1][j-1];
//            continue; // for. no need to go on if characters are the same.
            add = 0;
         } else {
            // See which character is replaced by which.
            // This will probably ruin the inlining.
            add = queryReplace;
            uint8 ws = soundexTable[0][(uint8)W[j-1]]; // TODO: use language
            uint8 ps = soundexTable[0][(uint8)P[i-1]]; // TODO: use language
            if (ws != 0) {
               if (ws == ps) {
                  // same soundex group
                  add = queryReplace - 2;
               }
            } else {
               // no soundexcode
               const int nbrVowels = 9;
               const char* vowels = "AEIOUYÅÄÖ";
               DEBUG8({
                  if (nbrVowels != int(strlen(vowels))) {
                     MC2ERROR("nbrVowels != strlen(vowels)");
                  }
               });
               const char* wVowel = strchr(vowels, W[j-1]);
               const char* pVowel = strchr(vowels, P[i-1]);
               int vowelDistanceTable[nbrVowels][nbrVowels] = {
                  {0, 1, 2, 2, 2, 2, 1, 1, 1},
                  {1, 0, 1, 2, 2, 1, 2, 1, 1},
                  {2, 1, 0, 2, 2, 1, 2, 2, 1},
                  {2, 2, 2, 0, 1, 1, 1, 2, 1},
                  {2, 2, 2, 1, 0, 1, 2, 2, 1},
                  {2, 1, 1, 1, 1, 0, 2, 2, 1},
                  {1, 2, 2, 1, 2, 2, 0, 1, 1},
                  {1, 1, 2, 2, 2, 2, 1, 0, 1},
                  {1, 1, 1, 1, 1, 1, 1, 1, 0}
               };
               if ((wVowel != NULL) &&
                   (pVowel != NULL))
               {
                  int wi = wVowel - vowels;
                  int pi = pVowel - vowels;
                  // both are vowels.
                  add = queryReplace - 3 + vowelDistanceTable[wi][pi];
                  /* The following distance table is used:
                   * i   y   u
                   * e   ö   o
                   * a   å   ä
                   * with some modifications e.g. a is close to both ä&å.
                   */
               }
            }
         }
         uint32 m = queryErase  + arr[i-1][j]; 
         uint32 l = queryInsert + arr[i]  [j-1];
         // insert operations at the end of the query string should be less
         // expensive.
         if (i == slb) {
            // add 1, remove queryInsert penalty.
            l += int32(1 - queryInsert);
            // this means that the normal queryInsert penalty is
            // reduced to 1 per character for characters at the end
            // of the word
         }
         uint32 n = add + arr[i-1][j-1];
         // errors in the beginning get a higher penalty than
         // errors later in the query.
         uint32 beginPy = add != 0 ?
            MAX(int(queryBeginningPenalty - i), 0) : 0;
         // the above means:
         // If there is an error in the first position,
         // there is an extra penalty of queryBeginningPenalty.
         // For each position later, the extra penalty is
         // reduced by 1
         /*
          *           erase cheaper than insert
          *                    erase cheaper than replace
          *                            erase
          *                                replace
          *                                     insert cheaper than replace
          *                                             insert
          *                                                 replace
          */
         arr[i][j] = beginPy +
            (m < l ? (m < n ? m : n): (l < n ? l : n));
      } // for j
   } // for i

   return arr[slb][sla];
}

/*
  original code, as published by Gertjan van Noord
#define MAXLINE 100 

int lev(char a[],char b[]) { 
  int arr[MAXLINE][MAXLINE]; 
  int i,j,l,m,n,add; 
  
  for (i=0;i<=strlen(a);i++) { 
    arr[0][i]=i; 
    } 
  for (j=0;j<=strlen(b);j++) { 
    arr[j][0]=j; 
  } 

  for (j=1;j<=strlen(b);j++) { 
    for (i=1;i<=strlen(a);i++) { 
      if (a[i-1] == b[j-1]) 
      { add=0; } else { add=1; } 
      m = 1+arr[j-1][i]; 
      l = 1+arr[j][i-1]; 
      n = add+arr[j-1][i-1]; 
      arr[j][i] = (m < l ? (m < n ? m : n): (l < n ? l : n)); 
    } 
  } 
  
  return arr[strlen(b)][strlen(a)]; 

}
*/

uint32
StringSearchUtility::sameLanguageGroup(uint32 requestedLanguage,
                                       uint32 language)
{
   // NB! At the moment this method does not seem to work!

   uint32 closeness = 0;
   // scale is 0 to 10.

   uint8 table[LangTypes::nbrLanguages][LangTypes::nbrLanguages] =
   {{10,  8,  9,  8,  7,  9,  7,  6,  9},
    { 2, 10,  3,  9,  3,  6,  3,  3,  2},
    { 7,  7, 10,  7,  6,  8,  6,  5,  8},
    { 3,  9,  4, 10,  4,  7,  4,  4,  3},
    { 5,  3,  6,  3, 10,  4,  8,  8,  5},
    { 8,  5,  8,  5,  5, 10,  5,  7,  7},
    { 4,  4,  5,  4,  8,  3, 10,  9,  4},
    { 6,  6,  7,  6,  9,  5,  9, 10,  6},
    { 9,  2,  2,  2,  2,  2,  2,  2, 10}};

   if ((language < LangTypes::nbrLanguages) &&
       (requestedLanguage < LangTypes::nbrLanguages))
   {
      closeness = table[language][requestedLanguage];
   } else {      
      if (language == LangTypes::nbrLanguages) {
         closeness = 1; // perhaps better than nothing?
      }
   }
   // supposed to be: swedish in same group as danish,
   // german<->dutch, italian<->spanish<->french<->portugese(hm?)
   return closeness;
}

uint32
StringSearchUtility::getStringTypeScore(uint32 language,
                                        uint32 requestedLanguage,
                                        uint32 strType)
{
   // ** lower scores are better. **

   // somehow this method should take into account what country you are in.
   if ( (language > LangTypes::nbrLanguages) &&
        (strType != ItemTypes::roadNumber) ) {
      mc2log << error << "[StringSearchUtil]: "
         "Language is worse than invalid : Language = "
             << language 
             << ", requestedLanguage = " << requestedLanguage << endl;
   }
   // pretty slow for now...
   uint32 score = MAX_UINT16;
   uint32 frac = 1;
   // language:
   uint32 languageFactor = 0;
   if (language == requestedLanguage) {
      languageFactor = 10;
//      score -= 256*10;
   } else if ((frac = sameLanguageGroup(requestedLanguage, language))) {
      languageFactor = frac;
//      score -= 256*frac;
//      score = MAX_UINT16/4;
   } else if (language == LangTypes::english) {
      languageFactor = 1;
//      score -= 256;
   } else {
      // ... nothing
   }
   if (strType == ItemTypes::synonymName) {
      score -= 256*(languageFactor >> 8);
   } else {
      score -= 256*languageFactor;
   }
   // type:
   uint32 base = 1000;
   switch ((ItemTypes::name_t) strType) {
      case ItemTypes::uniqueName:       score -= base    ; break;
      case ItemTypes::officialName:     score -= base-20 ; break;
      case ItemTypes::alternativeName:  score -= base-40 ; break;
      case ItemTypes::abbreviationName: score -= base-60 ; break;
      case ItemTypes::synonymName:      score -= base-120; break;
      case ItemTypes::roadNumber:       score -= base-140; break;
      case ItemTypes::exitNumber:       score -= base-160; break;
      case ItemTypes::invalidName:      score -= 1       ; break;
      case ItemTypes::maxDefinedName:   score -= 0       ; break;
         // no default!
   } // switch
   return score;
}


bool
StringSearchUtility::isFrontStreetNumber(const char* word)
{
   // Check if there is any street number in the beginning of the string.
   // The numbers in the beginning of the word is used, if there are not
   // followed by st, nd, rd or th. For example 1st Street.
   MC2String number;
   MC2String chars;

   mc2TextIterator it(word);
   bool addNumber = true;
   while( *it != 0 ) {
      uint32 ucs = *it;
      if( ucs < 256 ) {
         char c = char(ucs);
         if( isdigit(c) && addNumber ) {
            number += c;
         } else {
            chars += UTF8Util::ucsToUtf8(ucs);
            addNumber = false;
         }
      } else {
         chars += UTF8Util::ucsToUtf8(ucs);
         addNumber = false;
      }
      ++it;
   }
   if( chars.size() == 0 ) {
      if( number.size() > 0 )
         return true;
      else {
         return false;
      }
   } else {
      if( (number.size() > 0) && (chars != "st") &&
          (chars != "nd") && (chars != "rd") && (chars != "th") &&
          (!isWordSeparator(chars[0]) || (chars[0] == '-') ||
           (chars[0] == ',')) )
      {
         return true;
      } else 
         return false;      
   }
}

bool
StringSearchUtility::isBackStreetNumber(const char* word)
{
   // Check if there is any street number at the end of the string.
   // The numbers in the beginning of the word are used as street number.
   if( word[0] == '\0' ) {
      return false;
   } else {
      // Maybe this string should be used for something else than
      // just checking if it is empty? Is it because it only has
      // every other digit in it?
      MC2String number;
      mc2TextIterator it(word);
      while( true ) {
         uint32 firstChar = *it;
         ++it;
         uint32 secondChar = *it;
         if( firstChar < 256 ) {
            char f = char(firstChar);
            if( secondChar < 256 ) {
               char s = char(secondChar);
               if( isdigit(f) && isdigit(s) ) {
                  number += firstChar;
               } else if( isdigit(f) && (secondChar == 0) ) {
                  number += firstChar;
               } else {
                  // This seems to mean that if the second char is not
                  // a digit and not the end of the string, then stop
                  // searching.
                  if ( isdigit(f) ) {
                     // Added this becase it always needed two digits
                     // to detect a number.
                     number += firstChar;
                  }
                  break;
               }
            } else {
               break;
            }
         } else {
            break;
         }
         if( secondChar == 0 ) {
            break;
         } else {
            ++it;
         }
      }
      if( ! number.empty() ) {
         mc2dbg8 << "[SSU]: Found number " << MC2CITE( number ) << endl;
         return true;
      } else {         
         return false;
      }
   }
}


void
StringSearchUtility::
getStreetNumberAndSearchString(const MC2String& insearchString,
                               int& streetNumber,
                               MC2String& strippedSearchString)
{
   // Trim start and end.
   MC2String searchString = StringUtility::trimStart( insearchString.c_str() );
   char* tmpStr = NewStrDup::newStrDup( searchString );
   StringUtility::trimEnd( tmpStr );
   searchString = tmpStr;
   delete [] tmpStr;
      
   MC2String newStr =
      StringSearchUtility::stripStrangeChars(searchString, true);
   vector<MC2String> words;
   splitIntoWords( words, newStr );
   uint32 nbrWords = words.size();
   if( nbrWords > 1 ) {
      bool frontNumber = isFrontStreetNumber(words[0].c_str());
      if( frontNumber ) {
         MC2String number;
         mc2TextIterator it( words[0] );
         while( *it != 0 ) {
            uint32 ucs = *it;
            if( ucs < 256 ) {
               char c = char(ucs);
               if( isdigit(c) ) {
                  number += c;
               } else {
                  break;
               }
            } else {
               break;
            }
            ++it;
         }
         streetNumber = atoi(number.c_str());
         MC2String str;
         for(uint32 i = 1; i < nbrWords; i++) {
            str += words[i];
            if( i != (nbrWords-1) )
               str += " ";
         }
         strippedSearchString = str;
         mc2dbg8 << "[SSU]: streetNumber 1 = " << streetNumber << endl;
         return;
      }
      bool backNumber = isBackStreetNumber(words[nbrWords-1].c_str());
      if( backNumber ) {
         MC2String number;
         mc2TextIterator it( words[nbrWords-1] );
         while( *it != 0 ) {
            uint32 ucs = *it;
            if( ucs < 256 ) {
               char c = char(ucs);
               if( isdigit(c) )
                  number += c;
               else
                  break;
            } else {
               break;
            }
            ++it;
         }
         streetNumber = atoi(number.c_str());
         MC2String str;
         for(uint32 i = 0; i < nbrWords-1; i++) {
            str += words[i];
            if( i != (nbrWords-2) )
               str += " ";
         }
         strippedSearchString = str;
         mc2dbg8 << "[SSU]: streetNumber 2 = " << streetNumber << endl;
         return;
      }
   }
   streetNumber = 0;
   strippedSearchString = searchString;
   mc2dbg8 << "[SSU]: streetNumber 3 = " << streetNumber << endl;
}

bool
StringSearchUtility::isSimpleHouseNumber( const MC2String& str, 
                                          int& number ) {
   MC2String numericPart("");
   number = 0;

   mc2TextIterator it( str );
   while( *it != 0 ) {
      uint32 ucs = *it;
      if( ucs < 256 ) {
         char c = static_cast<char>( ucs );
         if ( isdigit(c) ) {
            numericPart += c;
         }
         else {
            if ( isalpha(c) && *(++it) == 0 ) {
               break;
            }
            return false;
         }
      }
      ++it;
   }
   if ( numericPart.size() > 0 ) {
      try {
         number = boost::lexical_cast<int>( numericPart );
         return true;
      } catch( const boost::bad_lexical_cast& e ) {
         number = 0;
         return false;
      }
   }
   else {
      return false;
   }
}

void 
StringSearchUtility::
simpleGetStreetNumberAndName( const MC2String& insearchString,
                              int& streetNumber,
                              MC2String& streetName ) {
   // Trim start and end.
   MC2String searchString = StringUtility::trimStart( insearchString.c_str() );
   char* tmpStr = NewStrDup::newStrDup( searchString );
   StringUtility::trimEnd( tmpStr );
   searchString = tmpStr;
   delete [] tmpStr;

   // Split the searchString into tokens
   vector<MC2String> words;
   istringstream is( searchString );
   copy( istream_iterator<MC2String>( is ), 
         istream_iterator<MC2String>(), 
         back_inserter( words ) );
   int nbrWords = words.size();

   if ( nbrWords <= 1 ) {
      streetNumber = 0;
      streetName = searchString;
      return;
   }

   MC2String rest; // the string without the house number
   if ( isSimpleHouseNumber( words[ 0 ], streetNumber ) ) {
      for ( int i = 1; i < nbrWords; ++i ) {
         rest += words[ i ];
         if ( i != ( nbrWords - 1 ) ) {
            rest += " ";
         }
      }
   }
   else if ( isSimpleHouseNumber( words[ nbrWords-1 ], streetNumber ) ) {
      for ( int i = 0; i < nbrWords-1; ++i ) {
         rest += words[i];
         if ( i != ( nbrWords - 2 ) ) {
            rest += " ";
         }
      }
   }
   else {
      streetNumber = 0;
      rest = searchString;
   }

   // if rest contains digit, don't accept,
   // set streetNumber to 0 and streetName to searchString
   mc2TextIterator it( rest );
   while( *it != 0 ) {
      uint32 ucs = *it;
      if( ucs < 256 ) {
         char c = static_cast<char>( ucs );
         if( isdigit(c) ) {
            // digits in more than one place, not a simple string
            streetNumber = 0;
            streetName = searchString;
            return;
         }
      }
      ++it;
   }
   
   //  set streetName to rest
   streetName = rest;
}

   
int
StringSearchUtility::getLevDist(const char* a, int alen,
                                const char* b, int blen,
                                uint16 maxDist)
{
   // Modified the old code from StringSearchUtility.h
   if (abs(int(alen - blen)) > maxDist) {
      mc2dbg8 << "[StringSearchUtility]: alen = " << alen << ", blen = "
              << blen << endl;
      return(-2);
   }
   const uint32 MAXLINE = 100;
   int arr[MAXLINE][MAXLINE];
   
   for(int i=0;i<=int(alen);i++) { 
      arr[0][i]=i;
   }
   int j;
   for(j=0;j<=int(blen);j++) { 
      arr[j][0]=j;
   }
   for(j=1;j<=int(blen);j++) {
      int minRowVal = 255;
      for (int i=1; i<=int(alen);i++) {
         int add;
         if ( a[i-1] == b[j-1] ) {
            add = 0;
         } else {
            add = 1;
         }
         int m = 1+arr[j-1][i];
         int l = 1+arr[j][i-1];
         int n = add+arr[j-1][i-1];
         // Take the minimum of m n l ?
         arr[j][i] = (m < l ? (m < n ? m : n): (l < n ? l : n));
         if (arr[j][i] < minRowVal)
            minRowVal = arr[j][i];
      }
      // The distance has to be greater than maxDis
      if (minRowVal > maxDist) 
         return(-1) ;     
   }
   return(arr[blen][alen]);
}

int
StringSearchUtility::getLevDist(const char* a, const char* b, uint16 maxDist)
{
   return getLevDist(a, strlen(a),
                     b, strlen(b),
                     maxDist);
}


void
StringSearchUtility::splitIntoWords(vector<MC2String>& words,
                                    mc2TextIterator it )
{
   MC2String tmp;
   while( *it != 0 ) {
      uint32 ucs = *it;
      if( StringSearchUtility::isWordSeparator(ucs) ) {
         words.push_back(tmp);
         tmp = "";
      } else {
         tmp += UTF8Util::ucsToMc2(ucs);
      }
      ++it;
   }
   words.push_back(tmp);
}


MC2String
StringSearchUtility::removeUKPostalCodeGroup( const MC2String& in_code )
{
   MC2String code = StringUtility::trimStartEnd( in_code );
   // There are four groups
   // e.g N35A 6QW contains N, 35A, 6 and QW
   // or  N3 5AQ contains N, 3, 5 and AQ
   int code_length = code.length();

   // 1. If code contains space and number followed by letters - remove
   //    the last letters.
   MC2String::size_type space_pos = code.find( ' ' );
   if ( space_pos != MC2String::npos ) {
      bool digitFound = false;      
      for ( int i = space_pos; i < code_length; ++i ) {
         digitFound = digitFound || isdigit( code[i] );
         if ( isalpha( code[i] ) ) {
            if ( digitFound ) {
               mc2dbg8 << "[StringSearchUtility]: 1" << endl;
               return MC2String( code, 0, i );
            }
         }
      }
   }
   // 2. If code contains space - remove everything upto the space
   if ( space_pos != MC2String::npos ) {
      for( int i = 0; i < code_length; ++i ) {
         if ( code[i] == ' ' ) {
            mc2dbg8 << "[StringSearchUtility]: 2" << endl;
            return MC2String( code, 0, i );
         }
      }
   }
   // 3. If code contains letter followed by number or number+letters
   //    remove from the number forward.
   {
      for ( int i = 0; i < code_length; ++i ) {
         if ( isdigit( code[i] ) ) {
            mc2dbg8 << "[StringSearchUtility]: 3" << endl;
            return MC2String( code, 0, i );
         }
      }
   }

   // 4. If code contains only letter or does not match return empty
   return "";
}

uint32
StringSearchUtility:: getUKPostalCodeNbrGroups( const MC2String& in_code )
{
   MC2String code = in_code;
   uint32 result = 0;
   while (code != ""){
      code = removeUKPostalCodeGroup(code);
      result++;
   }
   return result;

   /*
   MC2String code = StringUtility::trimStartEnd( in_code );
   // There are four groups
   // e.g N35A 6QW contains N, 35A, 6 and QW
   // or  N3 5AQ contains N, 3, 5 and AQ
   int code_length = code.length();

   // 1. If code contains space and number followed by letters - 
   //    This is a full 4 level zip.
   uint32 space_pos = code.find( ' ' );
   if ( space_pos != MC2String::npos ) {
      bool digitFound = false;      
      for ( int i = space_pos; i < code_length; ++i ) {
         digitFound = digitFound || isdigit( code[i] );
         if ( isalpha( code[i] ) ) {
            if ( digitFound ) {
               return 4;
            }
         }
      }
   }
   // 2. If code contains space - 
   //    This is a 3 level zip.
   //    FIXME: this one returns valid result for invalid UK zips.
   if ( space_pos != MC2String::npos ) {
      for( int i = 0; i < code_length; ++i ) {
         if ( code[i] == ' ' ) {
            mc2dbg8 << "[StringSearchUtility]: 2" << endl;
            return 3;
         }
      }
   }
   // 3. If code contains letter followed by number or number+letters
   //    remove from the number forward.
   {
      for ( int i = 0; i < code_length; ++i ) {
         if ( isdigit( code[i] ) ) {
            mc2dbg8 << "[StringSearchUtility]: 3" << endl;
            //return MC2String( code, 0, i );
         }
      }
   }

   // 4. If code contains only letter or does not match return empty
   return 0;
   */



} // getUKPostalCodeNbrGroups

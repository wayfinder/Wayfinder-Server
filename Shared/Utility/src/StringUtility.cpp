/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StringUtility.h"

#include "NewStrDup.h"
#include "StackOrHeap.h"
#include "STLStringUtility.h"
#include "StringConvert.h"
#include "TimeUtility.h"

// Regular expression for phonenumber matching
#include <regex.h>

#include <memory>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include<boost/tokenizer.hpp>

MC2String
StringUtility::makeFirstCapital( const MC2String& str ) 
{
   MC2String retVal;
   bool first = true;
   mc2TextIterator it(str.c_str());
   while( *it != 0 ) {
      uint32 ucs = *it;
      if( first ) {
         retVal += UTF8Util::utf8ToMc2(StringUtility::upper(ucs));
         first = false;
      } else {
         retVal += UTF8Util::ucsToMc2(ucs);
      }
      ++it;
   }
   return retVal;
}

int
StringUtility::isRomanNumeral(const char* str, uint32 n) 
{
   // Check if we have a roman number between lastSpace and i
   // Currently handle all numbers between 1 and 20
   int retVal = 0;
   switch (n) {
      case 2:
         // Check the possible numbers with two letters
         if ( (strncasecmp(str, "II", 2) == 0) ||
              (strncasecmp(str, "IV", 2) == 0) ||
              (strncasecmp(str, "IX", 2) == 0) ||
              (strncasecmp(str, "VI", 2) == 0) ||
              (strncasecmp(str, "VX", 2) == 0) ||
              (strncasecmp(str, "XI", 2) == 0) ||
              (strncasecmp(str, "XV", 2) == 0) ) {
            retVal = 2;
         } break;
      case 3:
         // Check the possible numbers with three letters
         if ( (strncasecmp(str, "III", 3) == 0) ||
              (strncasecmp(str, "VII", 3) == 0) ||
              (strncasecmp(str, "XII", 3) == 0) ||
              (strncasecmp(str, "XIV", 3) == 0) ||
              (strncasecmp(str, "XVI", 3) == 0) ||
              (strncasecmp(str, "XIX", 3) == 0) ) {
            retVal = 3;
         } break;
      case 4:
         // Check the possible numbers with four letters
         if ( (strncasecmp(str, "VIII", 4) == 0) ||
              (strncasecmp(str, "XIII", 4) == 0) ||
              (strncasecmp(str, "XVII", 4) == 0) ) {
            retVal = 4;
         } break;
      case 5:
         // Check the possible numbers with five letters
         if ( (strncasecmp(str, "XVIII", 5) == 0) ) {
            retVal = 5;
         } break;
   } // Switch

   return retVal;
}

void
StringUtility::makeRomanNumeralsUpper( char* str )
{
   uint32 lastPos = 0;
   uint32 len = strlen(str);
   
   // Find first space/non-alnum
   while ( (lastPos < len) && 
           ( (str[lastPos] != ' ') || (isAlNum(str[lastPos]))) ) {
      ++lastPos;
   }
   
   for (uint32 curPos=lastPos+1; curPos<len; ++curPos) {
      if ( (str[curPos] == ' ') || (!isAlNum(str[curPos]))) {

         int r = isRomanNumeral(&(str[lastPos+1]), curPos-lastPos-1);

         for (uint32 j=lastPos+1; j<lastPos+1+r; j++) {
            str[j] = upper(str[j])[0];
         }
         lastPos = curPos;
      }
   }
   
   // Check last word
   if (len-lastPos > 0) {
      int r = isRomanNumeral(&(str[lastPos+1]), len-lastPos-1);

      for (uint32 j=lastPos+1; j<lastPos+1+r; j++) {
         str[j] = upper(str[j])[0];
      }
  }
}


MC2String
StringUtility::makeFirstInWordCapital( const MC2String& str,
                                       bool makeRomanNumbersUpper )
{
   MC2String result;
   result.reserve( str.length() + 10 );
   bool capitalize = true;
   mc2TextIterator it(str.c_str());
   while( *it != 0 ) {
      uint32 ucs = *it;
      // FIXME: isspace might not work under Windows.
      if ( (StringUtility::isSpace(ucs) == 0) && (ucs != '(' ) &&
           ( ucs != '/' ) &&
           ( ucs != '-' ) && ( ucs != ',') && ( ucs != '\'' ) &&
           ( ucs != '\'' ) ) {
         if (capitalize) {
            // Not space and capitalize
            result += UTF8Util::utf8ToMc2(StringUtility::upper(ucs));
            capitalize = false;
         } else {
            // Not space and not capitalize
            result += UTF8Util::utf8ToMc2(StringUtility::lower(ucs));
         }
      } else {
         // Found space
         capitalize = true;
         result += UTF8Util::ucsToMc2(ucs);
      }
      ++it;
   }
   
   // The makeRomanNumeralsUpper is not really utf-8 compliant
   // but should work.
   if (makeRomanNumbersUpper) {
      char* tmp = NewStrDup::newStrDup( result );
      StringUtility::makeRomanNumeralsUpper( tmp );
      result = tmp;
      delete [] tmp;
   }

   return result;
   
}

bool
StringUtility::splitSeconds(uint32 deltaTime, uint32& hour, 
                                 uint32& minute, uint32&second)
{
   hour = deltaTime/3600;
   uint32 hs = hour*3600;
   minute = (deltaTime-hs)/60;
   second = deltaTime-hs-minute*60;
   return (true);
}

char*
StringUtility::splitSeconds(uint32 deltaTime, char* str)
{
   uint32 h,m,s;
   StringUtility::splitSeconds(deltaTime, h, m, s);
   sprintf(str, "%d:%02d:%02d%c", h, m, s, '\0');
   return (str);
}

char* StringUtility::strsep(char** stringp,
                            const char* delim)
{
   char* retPointer;

   // are we done?
   if (*stringp == NULL)
     return NULL;

   // no, next token:
   retPointer = *stringp;
   *stringp = strpbrk(*stringp, delim);

   // if not the last one, terminate it
   if (*stringp != NULL)
      (*stringp)++[0] = '\0';

   return retPointer; 
}

MC2String
StringUtility::trimStartEnd( const char* s, const char* additional )
{
   char* tmpStr = newStrDup( trimStart( s, additional ) );
   trimEnd( tmpStr, additional );
   // Will be deleted by MC2SimpleStringNoCopy
   return MC2SimpleStringNoCopy( tmpStr ).c_str();
}

MC2String
StringUtility::trimStartEnd( const MC2String& s, const char* additional )
{
   return trimStartEnd( s.c_str(), additional );
}

void
StringUtility::tokenListToVector( vector<char*>& out,
                                  char* list,
                                  char sep,
                                  bool skipEmptyTokens,
                                  bool trimStart)
{
   char* pos;
   char* findPos;
   char* end;
   pos = findPos = list;
   end = list + strlen( list ); // points to the ending nullbyte.
   
   while ( *pos != '\0' ) {
      if (trimStart) {
         pos = StringUtility::trimStart( pos );
      }
      findPos = strchr( pos, sep );
      if ( findPos == NULL ) { // no more tokens
         findPos = end;
      } else {
         *findPos = '\0'; // replace separator by nullbyte
      }
      if ( ( findPos > pos ) ||
           (!skipEmptyTokens &&
            ( findPos == pos )) )
      {
         out.push_back( pos );
      }
      if ( findPos != end ) {
         pos = findPos + 1; // skip the separator char
      }
      else 
         pos = findPos; // point to the ending nullbyte
   } // while
   if ( !skipEmptyTokens ) {
      if ( (*pos == '\0') && pos != list &&
           (*(pos - 1) == '\0') )
      {
         // Empty token last
         out.push_back( pos );
      }
   } else if ( !skipEmptyTokens &&
               ( pos == list ) )
   {
      // Empty list
      out.push_back( pos );
   }

}

void
StringUtility::tokenListToVector( vector<MC2String>& splitStr,
                                  const MC2String& toSplit,
                                  char sep,
                                  bool skipEmptyTokens,
                                  bool trimStart )
{
   // Copy the instring as it will be written to by the other
   // tokenListToVector
   int len = toSplit.length() + 1;
   StackOrHeap<256, char> buf( len );
   strcpy( buf, toSplit.c_str() );

   // Do the other tokenListToVector
   vector<char*> tokens;
   tokenListToVector( tokens, buf, sep,
                      skipEmptyTokens, trimStart );

   // Transfer the strings
   splitStr.reserve( tokens.size() );
   for ( int i = 0, n = tokens.size(); i < n; ++i ) {
      splitStr.push_back( tokens[ i ] );
   }   
}

void 
StringUtility::splitToVector( const MC2String& toSplit,
                              char separator,
                              vector< MC2String >& splitStr,
                              bool skipEmptyTookens) {
   using namespace boost;
   
   tokenizer<escaped_list_separator<char> > tok(
      toSplit, escaped_list_separator<char>('\\', separator, '\"'));
   
   for(tokenizer<escaped_list_separator<char> >::iterator it = tok.begin(); 
       it != tok.end(); ++it){
      if ( ! ( skipEmptyTookens && (*it) == "" ) ) {
         splitStr.push_back( *it );      
      }
   }   
}


bool 
StringUtility::onlyDigitsInString( const char* str,
                                   bool ignoreSpace ) 
{
   bool found = false;
   uint32 length = strlen(str);
   uint32 i=0;

   if ( ignoreSpace ) {
      // Skipp initial whitespace
      while ( i < length && isspace( str[ i ] ) ) {
         i++;
      }
   }
   uint32 startIndex = i;

   // Loop until found or end of string
   while ((!found) && (i < length)) {
      if (isdigit(str[i]) == 0) {
         found = true;
      } else {
         i++;
      }
   }

   if ( ignoreSpace ) {
      // Check if there was whitespace last and number before
      if ( i < length && i > startIndex && isspace( str[ i ] ) ) {
         // Skipp trailing whitespace
         while ( i < length && isspace( str[ i ] ) ) {
            i++;
         }
         // See if there was only whitespace until the end
         found = ( i == length );
      }
   }

   return (!found);
}

bool
StringUtility::containsDigit( const char* str )
{
   while ( *str ) {
      if ( isdigit(*str) ) {
         return true;
      }
      ++str;
   }
   return false;
}

uint32
StringUtility::makeDate( const char* date ) {
   char divider = '\0';
   const char* pos = NULL;
   
   if ( strchr( date, '-') != NULL ) {
      divider = '-';
   } else {
      divider = '/';
   }
   pos = date;
   uint32 firstField = strtol(pos, NULL, 10);
   pos = strchr( date, divider );
   if ( pos == NULL ) {
      return 0;
   }
   uint32 lengthOne = pos - date;
   pos ++;
   uint32 secondField = strtol(pos, NULL, 10);
   pos = strchr( pos, divider );
   if ( pos == NULL ) {
      return 0;
   }
   uint32 lengthTwo = pos - date - lengthOne -1;
   pos ++;
   uint32 thirdField = strtol(pos, NULL, 10);

   pos = strchr( pos, '\0');
   uint32 lengthThree = pos - date - lengthOne - lengthTwo - 2;
   uint32 time = 0;

   if ( lengthOne == 4 ) {
      time = convertDate(firstField, secondField, thirdField);
   } else if ( lengthThree == 4 ){
      time = convertDate( thirdField, firstField, secondField );
   } else {
      time = 0;
   }

   return time;
}


int 
StringUtility::makeDateStr( uint32 time, char* dateStr, char* timeStr ) {
   struct tm *tm_struct;
   int size = 0;
   
   time_t rtime = time;
   struct tm result;
   tm_struct = gmtime_r( &rtime, &result );

   sprintf( dateStr, "%.4d-%.2d-%.2d", 
            tm_struct->tm_year + 1900, tm_struct->tm_mon + 1,
            tm_struct->tm_mday );
   size += sprintf( timeStr, "%.2d:%.2d:%.2d", tm_struct->tm_hour, 
                    tm_struct->tm_min, tm_struct->tm_sec );
   return size;
}

size_t
StringUtility::strftime( char* s, size_t max, const char* format,
                         const struct tm *tm )
{
   return ::strftime( s, max, format, tm );
}

uint32
StringUtility::convertDate(uint32 year, uint32 month, uint32 day) {
   return TimeUtility::convertDate( year, month, day );
}
 

int
StringUtility::replaceChar( char *str,
                            char searchFor, char replaceWith,
                            int stopAt )
{
   int nbrReplaced = 0;
   if (str != NULL) {
      int pos = 0;
      while ((pos < stopAt) &&
             (str[pos] != '\0'))
      {
         if (str[pos] == searchFor) {
            str[pos] = replaceWith;
            nbrReplaced++;
         }
         pos++;
      }
   } // if
   return nbrReplaced;
}

char*
StringUtility::replaceString( const char* source,
                              const char* searchFor,
                              const char* replaceWith,
                              int maxReplace)
{
   MC2_ASSERT(source != NULL);
   MC2_ASSERT(searchFor != NULL);
   int ocurrences = 0;
   if(strstr(source, searchFor) != NULL) {
      // count the ocurrences
      char* s = (char*)source;
      while((s = strstr(s, searchFor)) != NULL) {
         ++ocurrences;
         ++s;
      }

      // check maximum replace count
      if ((maxReplace > 0) && (ocurrences > maxReplace))
         ocurrences = maxReplace;

      // check replacement string
      const char* repl = "";
      if (replaceWith != NULL) {
         repl = replaceWith;
      }

      // allocate new string
      uint32 newLength = strlen(source) + ocurrences * strlen(repl);
      char* newString = new char[newLength];
      newString[0] = '\0';

      // build the new string from source and repl, using ocurrences as the max
      char* found = (char*)source;
      char* last = (char*)source;
      int searchLen = strlen(searchFor);
      while ( (ocurrences > 0) && ((found = strstr(last, searchFor)) != NULL)) {
         strncat(newString, last, found - last);
         strcat(newString, repl);
         found += searchLen;
         last = found;
         --ocurrences;
      }
      // add the last part
      if (last < source + strlen(source))
         strcat(newString, last);

      return newString;
   } else {
      return NULL;
   }
}


size_t
StringUtility::utf8lcpy( char *dest, const char *src, size_t n )
{
   MC2_ASSERT( src != NULL );
   
   if ( n == 0 ) {
      return strlen(src);
   }
   
   MC2_ASSERT( dest != NULL );
   // Position in out-buffer.
   uint32 outpos = 0;
   // Save room for the zero term
   --n;
   for ( mc2TextIterator it = src; *it != 0; ++it ) {
      // Number of utf-8 bytes for the current unicode character
      uint32 curNbr = it.nbrBytesForCurrentChar();
      if ( curNbr <= n ) {
         n -= curNbr;
         // Copy the bytes to the destination
         uint32 nbrWritten = UTF8Util::unicodeToUtf8( *it, &dest[outpos] );
         MC2_ASSERT( nbrWritten == curNbr );
         outpos += curNbr;
      } else {
         // No more space.
         break;
      }
   }
   /// Zero terminate
   dest[outpos] = 0;
   return strlen(src);
}

char* 
StringUtility::removeAllButDigits( const char* str ) {
   // Remove all nondigits in the string
   int length = strlen( str );
   char* tmpStr = new char[ length + 1];
   int writePos = 0;
   for ( int pos=0 ; pos < length ; pos++ ) {
      if ( isdigit( str[ pos ] ) != 0 ) {
         tmpStr[ writePos ] = str[ pos ];
         writePos++;
      }
   }
   tmpStr[ writePos ] = '\0';
   return tmpStr;
}

void 
StringUtility::randStr( char* target, uint32 nbr ) {
   for ( uint32  i = 0 ; i < nbr ; i++ ) {
      if ( (uint32)(2.0*rand()/(RAND_MAX + 1.0) ) ) {
         // 65-90
         char ch = 65 + (uint32)(25.0*rand()/(RAND_MAX + 1.0));
         target[ i ] = ch;
      } else {
         // 97-122
         char ch = 97 + (uint32)(25.0*rand()/(RAND_MAX + 1.0));
         target[ i ] = ch;
      }
   }
   target[ nbr ] = '\0';
}

uint32 
StringUtility::wapStr( char* target, const char* str ) {
   mc2TextIterator pos = mc2TextIterator( str );
   char* write = target;
   
   if ( str == NULL ) {
      return 0;
   }
   
   while ( *pos != '\0' ) {
      switch( *pos ) {
         case '&' : 
            strcpy( write, "&amp;" );
            write += 5; // strlen( "&amp;" )
            break;
         case '"' :
            strcpy( write, "&quot;" );
            write += 6; // strlen( "&quot;" )
            break;
         case '\'': 
            strcpy( write, "&apos;");
            write += 6; // strlen( "&apos;" )
            break;
         case '<' : 
            strcpy( write, "&lt;" );
            write += 4; // strlen( "&lt;" )
            break;
         case '>' : 
            strcpy( write, "&gt;" ); 
            write += 4; // strlen( "&gt;" )
            break;
         case '$' : 
            strcpy( write, "$$" );
            write += 2; // strlen( "$$" )
            break;
         default: { 
               uint32 c = *pos;
               if (c < 128) { // Ok
                  *write = *pos;
                  write++;
               }
               else { // Special care
                  write += sprintf( write, "&#x%X;", c );
               }
            }
            break;
      } // / switch( *pos )
      ++pos;
   }
   *write = '\0';

   return write - target;
}

void 
StringUtility::swe2eng( char* pszTarget, const char* pszSource )
{
   char c;
   while( *pszSource != '\0' ) {
      switch( (unsigned char)*pszSource ) {
         default:
            c = *pszSource;
            break;
         case 0xE5 :    // 'å'
         case 0xE4 :    // 'ä'
            c = 'a';
            break;
         case 0xF6 :    // 'ö'
            c = 'o';
            break;
         case 0xC5 :    // 'Å'
         case 0xC4 :    // 'Ä'
            c = 'A';
            break;
         case 0xD6 :    // 'Ö'
            c = 'O';
            break;
      }
      *pszTarget = c;

      pszTarget++;
      pszSource++;
   }
   *pszTarget = '\0';
}


bool 
StringUtility::base64Encode( const char* input, char* output,
                             uint32 maxLineLength ) 
{
   return base64Encode( (const byte*)input, strlen( input ), output,
                        maxLineLength );
}

MC2String
StringUtility::base64Encode( const MC2String& instr,
                             uint32 maxLineLength )
{
   // Try to calculate the length of the string
   // Each byte will be 8/6 bytes
   // Each newline will be 2 characters
   // Sometimes there will be 3 extra = at the end.
   int length = (instr.length() + 1 ) * 8 / 6;
   if ( maxLineLength != MAX_UINT32 ) {
      length += 2*instr.length() / maxLineLength;
   }
   length += 3 + 3 + 1;
   char* output = new char[ length ];
   // Encode the complete string 
   base64Encode( reinterpret_cast<const byte*>(instr.c_str()),
                 instr.length(),
                 output,
                 maxLineLength );
   // Copy the string and return it.
   return MC2SimpleStringNoCopy( output ).c_str();
}



bool 
StringUtility::base64Encode( const byte* input, uint32 inputLength,
                             char* output, uint32 maxLineLength )
{
   int i; 
   const byte* eofPos = input + inputLength;
   byte dtable[256];           /* Encode table */
   uint32 lineLength = 0;
   const char eol[3] = "\r\n";
   const uint32 eolLength = 2;
   
   /*  Fill dtable with character encodings.  */
   for (i = 0; i < 26; i++) {
      dtable[i] = 'A' + i;
      dtable[26 + i] = 'a' + i;
   }
   for (i = 0; i < 10; i++) {
      dtable[52 + i] = '0' + i;
   }
   dtable[62] = '+';
   dtable[63] = '/';
   
   while ( input < eofPos ) {
      byte igroup[3], ogroup[4];
      int c=0, n;
      
      igroup[0] = igroup[1] = igroup[2] = 0;
      for (n = 0; n < 3; n++) {
         if ( input == eofPos ) {
            break;
         }
         c = *input;
         input++;
         igroup[n] = (byte) c;
      }
      if (n > 0) {
         ogroup[0] = dtable[igroup[0] >> 2];
         ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
         ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
         ogroup[3] = dtable[igroup[2] & 0x3F];
         
         /* Replace characters in output stream with "=" pad
            characters if fewer than three characters were
            read from the end of the input string. */
         
         if (n < 3) {
            ogroup[3] = '=';
            if (n < 2) {
               ogroup[2] = '=';
            }
         }
         for (i = 0; i < 4; i++) {
            *output = ogroup[i];
            output++;
            lineLength++;
            if ( lineLength >= maxLineLength && input < eofPos ) {
               for ( uint32 i = 0; i < eolLength ; i++ ) {
                  *output = eol[ i ];
                  output++;
               }
               lineLength = 0;
            }
         }
      }
   }
  
   *output = '\0';
  
   return true;
}


int
StringUtility::base64Decode( const char* input, byte* output )
{
   int i;
   byte dtable[256];           /* Decode table */
   bool errcheck = true; 

   for (i = 0; i < 256; i++) {
      dtable[i] = 0x80;
   }
   for (i = 'A'; i <= 'Z'; i++) {
      dtable[i] = 0 + (i - 'A');
   }
   for (i = 'a'; i <= 'z'; i++) {
                   dtable[i] = 26 + (i - 'a');
   }
   for (i = '0'; i <= '9'; i++) {
      dtable[i] = 52 + (i - '0');
   }
   dtable[uint8('+')] = 62;
   dtable[uint8('/')] = 63;
   dtable[uint8('=')] = 0;

   byte* origOutput = output;

   /*CONSTANTCONDITION*/
   while (true) {
      byte a[4], b[4], o[3];
      
      for (i = 0; i < 4; i++) {
         int c = *input;
         input++;

         if ( c == '\0' ) {
            if (errcheck && (i > 0)) {
               mc2log << warn << "StringUtility::base64Decode "
                         "Input file incomplete. " << endl;
            }
            return -1;
         }
         if (dtable[c] & 0x80) {
            if (errcheck && c != '\r' && c!= '\n') {
               mc2log << "Illegal character '" << c 
                      << "' in input file." << endl;
            }
            /* Ignoring errors: discard invalid character. */
            i--;
            continue;
         }
         a[i] = (byte) c;
         b[i] = (byte) dtable[c];
      }
      o[0] = (b[0] << 2) | (b[1] >> 4);
      o[1] = (b[1] << 4) | (b[2] >> 2);
      o[2] = (b[2] << 6) | b[3];
      i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);

      int j;
      for (j = 0; j < i; j++) {
         *output = o[j];
         output++;
      }

      if (i < 3 || *input == '\0' ) {
         *output = '\0';
         return output - origOutput;
      }
   }
}

char*
StringUtility::SQLEscape(const char* src, char* dest, uint32 max, uint32 len)
{
#define SQLESCAPE_CHAR_CHECK src[i] == '\'' || src[i] == '\0' || \
                             src[i] == '\n' || src[i] == '\r' || \
                             src[i] == '\\' || src[i] == '"'

   uint32 i;
   uint32 j = 0;
   if (len > 0) {
      for (i = 0; i < len && j < max; /*i++*/) {
         if (SQLESCAPE_CHAR_CHECK)
            dest[j++] = '\\';
         dest[j++] = src[i++];
      }
   } else {
      i = 0;
      while (src[i] != '\0' && j < max) {
         if (SQLESCAPE_CHAR_CHECK)
            dest[j++] = '\\';
         dest[j++] = src[i++];
      }
   }

   dest[j] = '\0';
   return dest;
}

char*
StringUtility::SQLEscapeSecure(const char* src, char* dest, uint32 max,
                               uint32 len)
{
#define SQLESCAPESEC_CHAR_CHECK src[i] == '\'' || src[i] == '\0' || \
                                src[i] == '\n' || src[i] == '\r' || \
                                src[i] == '\\' || src[i] == '"'  || \
                                src[i] == '%'  || src[i] == '_'

   uint32 i;
   uint32 j = 0;
   if (len > 0) {
      for (i = 0; i < len && j < max; /*i++*/) {
         if (SQLESCAPESEC_CHAR_CHECK)
            dest[j++] = '\\';
         dest[j++] = src[i++];
      }
   } else {
      i = 0;
      while (src[i] != '\0' && j < max) {
         if (SQLESCAPESEC_CHAR_CHECK)
            dest[j++] = '\\';
         dest[j++] = src[i++];
      }
   }

   dest[j] = '\0';
   return dest;
}

MC2String
StringUtility::SQLEscape( const MC2String& src )
{
   char* dest = new char[ src.length() * 2 + 1 ];
   return MC2SimpleStringNoCopy(
      SQLEscape( src.data(), dest,
                 MAX_UINT32, src.length() ) ).c_str();
}

MC2String
StringUtility::SQLEscapeSecure( const MC2String& src )
{
   char* dest = new char[ src.length() * 2 + 1 ];
   return MC2SimpleStringNoCopy(
      SQLEscapeSecure( src.data(), dest,
                       MAX_UINT32, src.length() ) ).c_str();
}

   

int
StringUtility::URLEncode(char* outbuf,
                         const char* inbuf)
{
   int len = URLEncode(outbuf, inbuf, strlen(inbuf));
   outbuf[len] = '\0';
   return len;
}

int
StringUtility::URLEncode(char* outbuf,
                         const char* inbuf,
                         int inlength)
{
   uint32 pos = 0;
   uint32 outpos = 0;
   char cstr[44];
   byte ch;   

   while ( inlength -- ) {
      ch = inbuf[pos++];
      if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
           (ch >= '0' && ch <= '9') || (ch >= '-' && ch <= '.') ||
           ( ch == '_' )) {
         // Ordinary character
         // Append to outbuf
         outbuf[outpos++] = (char)ch;
      } else if ( ch == 32 ) { // ASCII 32 == ' '
         // Space - special
         outbuf[outpos++] = '+';
      } else {
         // Even more special.
         sprintf(cstr, "%%%02X", ch);
         int cstrlen = 3; // strlen(cstr); // Should be constant really.
         memcpy(&outbuf[outpos], cstr, cstrlen);
         outpos += cstrlen;
      }
   }   
   return outpos;
}

MC2String
StringUtility::URLEncode( const MC2String& instr )
{
   int len = (instr.length() + 1) * 3;
   StackOrHeap<256, char> buf( len );
   
   int outlen = URLEncode( buf, instr.c_str(), instr.length() );
   buf[outlen] = '\0';

   return MC2String(buf);
}

int 
StringUtility::URLDecode(byte* outbuf,
                         const char* inbuf)
{
   uint32 pos = 0;
   uint32 outPos = 0;
   uint32 result = 0;   

   while ( inbuf[pos] != '\0' ) {
      if ( inbuf[pos] == '%' ) {
         switch ( inbuf[pos + 1] ) {
            case 'A' : result = 10; break;
            case 'B' : result = 11; break;
            case 'C' : result = 12; break;
            case 'D' : result = 13; break;
            case 'E' : result = 14; break;
            case 'F' : result = 15; break;
            case 'a' : result = 10; break;
            case 'b' : result = 11; break;
            case 'c' : result = 12; break;
            case 'd' : result = 13; break;
            case 'e' : result = 14; break;
            case 'f' : result = 15; break;
            default: result = inbuf[pos + 1] - 48;
         }
         result = result*16;
         switch ( inbuf[pos + 2] ) { 
            case 'A' : result += 10; break;
            case 'B' : result += 11; break;
            case 'C' : result += 12; break;
            case 'D' : result += 13; break;
            case 'E' : result += 14; break;
            case 'F' : result += 15; break;
            case 'a' : result += 10; break;
            case 'b' : result += 11; break;
            case 'c' : result += 12; break;
            case 'd' : result += 13; break;
            case 'e' : result += 14; break;
            case 'f' : result += 15; break;
            default: result = result + inbuf[pos + 2] - 48;
         }
         outbuf[outPos++] = (char)result; // Works 0-255.
         // '%' and first number, last number handled normaly at end
         pos += 2;
      }
      else if ( inbuf[pos] == '+' ) {
         outbuf[outPos++] = ' ';
      } else {
         outbuf[outPos++] = inbuf[pos];
      }
      pos++;
   }

   return outPos;
}
    

bool 
StringUtility::checkBoolean( const char* str, bool defaultValue ) {
   if ( StringUtility::strcasecmp( str, "false" ) == 0 ) {
      return false;
   } else if ( StringUtility::strcasecmp( str, "true" ) == 0 ) {
      return true;
   } else {
      return defaultValue;
   }
}


const char* 
StringUtility::booleanAsString( bool value ) {
   if ( value ) {
      return "true";
   } else {
      return "false";
   }
}

bool 
StringUtility::validEmailAddress( const char* emailAddress ) {
   // This regular expression is also in mc2php-util.php::isEmailValid
   // in mc2web. Keep them uptodate.
   const char* reqStr = 
      "^[-!#$%&'*+./0-9=?A-Z^_`a-z{|}~]+"      // Before at
      "@"                                     // at
      "[A-Za-z0-9]"                          // First let-dig
      "[-A-Za-z0-9]*\\."                    // let-dig-hyp dot
      "[-A-Za-z0-9.]+$";                   // let-dig-hyp-dot
   regex_t p;
   int c = (REG_EXTENDED|REG_NOSUB);
   int res = regcomp( &p, reqStr, c );
   if ( res != 0 ) {
      mc2log << error << "StrU::validEmailAddress regcomp failed for \"" 
             << reqStr << "\"" << endl;
      return false;
   }

   // Split on ',' and do check for each part (without ','!)
   vector<MC2String> es = STLStringUtility::explode( ",", emailAddress );

   for ( uint32 i = 0 ; i < es.size() && res == 0 ; ++i ) {
      res = regexec( &p, es[ i ].c_str(), 0, NULL, 0 );
   }

   regfree( &p );

   if ( res == 0 ) {
      return true;
   } else {
      return false;
   }
}


bool 
StringUtility::endsWithStr( const char* str, 
                            const char* endStr, 
                            bool caseSensitive )
{
   int strLen = strlen( str );
   int endStrLen = strlen( endStr );
   
   int pos = strLen - endStrLen;

   if ( pos < 0 )
      return false;
   
   if ( caseSensitive )
      return (strcmp( str + pos, endStr ) == 0);
   else 
      return (strcasecmp( str + pos, endStr ) == 0);
   
}


uint32 
StringUtility::addMonths( uint32 now, uint32 months ) {
   struct tm tm_real;
   struct tm *tmstruct = &tm_real;
   time_t rtime = now;
   tmstruct = localtime_r( &rtime, tmstruct );
   tmstruct->tm_year += int32( floor( months / 12 + 0.01 ) );
   tmstruct->tm_mon  += months % 12;
   if ( tmstruct->tm_mon > 11 ) {//Flipped year (mktime should fix this)
      tmstruct->tm_year++;
      tmstruct->tm_mon -= 12;
   }

   return mktime( tmstruct );
}


uint32 
StringUtility::addYears( uint32 now, uint32 years ) {
   struct tm tm_real;
   struct tm *tmstruct = &tm_real;
   time_t rtime = now;
   tmstruct = localtime_r( &rtime, tmstruct );
   tmstruct->tm_year += years;

   return mktime( tmstruct );
}

const set<const char*> 
StringUtility::stringVectorToCharSet( const vector<char*>& strVector )
{
   set<const char*> result;

   for ( uint32 i=0; i < strVector.size(); ++i ) {
       result.insert( strVector[ i ] );
   }

   return result;
}

int
StringUtility::parseCommaSepInts(vector<uint32>& result,
                                 const char* inStr)
{
   if ( inStr == NULL ) {
      return 0; // OK
   }
   if ( strlen(inStr) == 0 ) {
      return 0; // OK
   }

   char* strCopy = StringUtility::newStrDup(inStr);

   vector< char * > vect;
   StringUtility::tokenListToVector( vect, strCopy, ',');
   for ( uint32 i = 0; i < vect.size(); ++i ) {
      char* endPtr = NULL;
      uint32 tempVal = strtoul( vect[ i ], &endPtr, 0 );
      if ( endPtr == NULL ) {
         delete [] strCopy;
         return -1; // ERROR
      } else {
         result.push_back(tempVal);
      }
   }

   delete [] strCopy;
   return vect.size();
}


bool
StringUtility::validPhonenumber( const char* nbr ) {
   regex_t p;
   int c = (REG_EXTENDED|REG_NOSUB);
   bool ok = false;

   const char* cnbr[] = { 
      "^93[1-9][0-9]{6,16}$",       // Afghanistan
      "^355[1-9][0-9]{6,16}$",      // Albania
      "^213[1-9][0-9]{6,16}$",      // Algeria
      "^376[1-9][0-9]{5,14}$",      // Andorra
      "^244[1-9][0-9]{6,16}$",      // Angola
      "^672[1-9][0-9]{6,16}$",      // Australian External Territories 
         // (Antarctica)
      "^54[1-9][0-9]{6,16}$",       // Argentina
      "^374[1-9][0-9]{6,16}$",      // Armenia
      "^297[1-9][0-9]{6,16}$",      // Aruba
      "^247[1-9][0-9]{6,16}$",      // Ascension
      "^61[1-9][0-9]{6,16}$",       // Australia (incl Cocos-Keeling IslandS)
      "^43[1-9][0-9]{5,14}$",       // Austria
      "^994[1-9][0-9]{6,16}$",      // Azerbaijan
      "^973[1-9][0-9]{6,16}$",      // Bahrain
      "^880[1-9][0-9]{6,16}$",      // Bangladesh
      "^375[1-9][0-9]{6,16}$",      // Belarus
      "^32[1-9][0-9]{5,14}$",       // Belgium
      "^501[1-9][0-9]{6,16}$",      // Belize
      "^229[1-9][0-9]{6,16}$",      // Benin
      "^975[1-9][0-9]{6,16}$",      // Bhutan
      "^591[1-9][0-9]{6,16}$",      // Bolivia
      "^387[1-9][0-9]{6,16}$",      // Bosnia & Herzegovina
      "^267[1-9][0-9]{6,16}$",      // Botswana
      "^55[1-9][0-9]{6,16}$",       // Brazil
      "^673[1-9][0-9]{6,16}$",      // Brunei Darussalam
      "^359[1-9][0-9]{6,16}$",      // Bulgaria
      "^226[1-9][0-9]{6,16}$",      // Burkina Faso
      "^257[1-9][0-9]{6,16}$",      // Burundi
      "^855[1-9][0-9]{6,16}$",      // Cambodia
      "^237[1-9][0-9]{6,16}$",      // Cameroon
      "^238[1-9][0-9]{6,16}$",      // Cape Verde Islands
      "^236[1-9][0-9]{6,16}$",      // Central African Republic
      "^235[1-9][0-9]{6,16}$",      // Chad
      "^64[1-9][0-9]{6,16}$",       // Chatham Island (New Zealand)
      "^56[1-9][0-9]{6,16}$",       // Chile (Easter Island)
      "^86[1-9][0-9]{6,16}$",       // China (PRC)
      "^57[1-9][0-9]{6,16}$",       // Colombia
      "^269[1-9][0-9]{6,16}$",      // Comoros / Mayotte Island
      "^243[1-9][0-9]{6,16}$",      // Congo, Dem. Rep. of (Zaire)
      "^682[1-9][0-9]{6,16}$",      // Cook Islands
      "^506[1-9][0-9]{6,16}$",      // Costa Rica
      "^225[1-9][0-9]{6,16}$",      // Côte d'Ivoire (Ivory Coast)
      "^385[1-9][0-9]{6,16}$",      // Croatia
      "^53[1-9][0-9]{6,16}$",       // Cuba (Christmas Island, Guantanamo Bay)
      "^599[1-9][0-9]{6,16}$",      // Curaçao / Netherlands Antilles
      "^357[1-9][0-9]{6,16}$",      // Cyprus
      "^420[1-9][0-9]{5,14}$",      // Czech Republic
      "^45[1-9][0-9]{7,12}$",       // Denmark
      "^246[1-9][0-9]{6,16}$",      // Diego Garcia
      "^253[1-9][0-9]{6,16}$",      // Djibouti
      "^670[1-9][0-9]{6,16}$",      // East Timor / Timor Leste
      "^593[1-9][0-9]{6,16}$",      // Ecuador
      "^20[1-9][0-9]{6,16}$",       // Egypt
      "^503[1-9][0-9]{6,16}$",      // El Salvador
      "^88213[0-9]{4,16}$",         // EMSAT (Mobile Satellite service)
      "^240[1-9][0-9]{6,16}$",      // Equatorial Guinea
      "^291[1-9][0-9]{6,16}$",      // Eritrea
      "^372[1-9][0-9]{6,16}$",      // Estonia
      "^251[1-9][0-9]{6,16}$",      // Ethiopia
      "^500[1-9][0-9]{6,16}$",      // Falkland Islands (Malvinas)
      "^298[1-9][0-9]{5,14}$",      // Faroe islands
      "^679[1-9][0-9]{6,16}$",      // Fiji Islands
      "^358[1-9][0-9]{5,12}$",      // Finland
      "^33[1-9][0-9]{8,9}$",        // France , more strict since italians
                                    //  tend to forget their country code
      "^596[1-9][0-9]{6,16}$",      // French Antilles / Martinique
      "^594[1-9][0-9]{6,16}$",      // French Guiana
      "^689[1-9][0-9]{6,16}$",      // French Polynesia
      "^241[1-9][0-9]{6,16}$",      // Gabonese Republic
      "^220[1-9][0-9]{6,16}$",      // Gambia
      "^995[1-9][0-9]{6,16}$",      // Georgia
      "^995[1-9][0-9]{6,16}$",      // Georgia
      "^49[1-9][0-9]{5,14}$",       // Germany
      "^233[1-9][0-9]{6,16}$",      // Ghana
      "^350[1-9][0-9]{6,16}$",      // Gibraltar
      "^350[1-9][0-9]{6,16}$",      // Gibraltar
      "^881[1-9][0-9]{6,16}$",      // Global Mobile Satellite System (GMSS)
         // Globalstar 8818,8819
      "^30[1-9][0-9]{6,16}$",       // Greece
      "^299[1-9][0-9]{6,16}$",      // Greenland
      "^590[1-9][0-9]{6,16}$",      // Guadeloupe
      "^502[1-9][0-9]{6,16}$",      // Guatemala
      "^245[1-9][0-9]{6,16}$",      // Guinea-Bissau
      "^224[1-9][0-9]{6,16}$",      // Guinea
      "^592[1-9][0-9]{6,16}$",      // Guyana
      "^509[1-9][0-9]{6,16}$",      // Haiti
      "^504[1-9][0-9]{6,16}$",      // Honduras
      "^852[1-9][0-9]{6,16}$",      // Hong Kong
      "^36[1-9][0-9]{6,16}$",       // Hungary
      "^354[1-9][0-9]{6,16}$",      // Iceland
      "^91[1-9][0-9]{6,16}$",       // India
      "^62[1-9][0-9]{6,16}$",       // Indonesia
      "^871[1-9][0-9]{6,16}$",      // Inmarsat (Atlantic Ocean - East)
      "^874[1-9][0-9]{6,16}$",      // Inmarsat (Atlantic Ocean - West)
      "^873[1-9][0-9]{6,16}$",      // Inmarsat (Indian Ocean)
      "^872[1-9][0-9]{6,16}$",      // Inmarsat (Pacific Ocean)
      "^800[1-9][0-9]{6,16}$",      // International Freephone Service
      "^808[1-9][0-9]{6,16}$",      // International Shared Cost Service (ISCS)
         // Wake Island
      "^98[1-9][0-9]{6,16}$",       // Iran
      "^964[1-9][0-9]{6,16}$",      // Iraq
      "^353[1-9][0-9]{5,14}$",      // Ireland
      "^972[1-9][0-9]{6,16}$",      // Israel
      "^39[1-9][0-9]{5,14}$",       // Italy
      "^81[1-9][0-9]{6,16}$",       // Japan
      "^962[1-9][0-9]{6,16}$",      // Jordan
      "^254[1-9][0-9]{6,16}$",      // Kenya
      "^686[1-9][0-9]{6,16}$",      // Kiribati
      "^850[1-9][0-9]{6,16}$",      // Korea (North)
      "^82[1-9][0-9]{6,16}$",       // Korea (South)
      "^965[1-9][0-9]{6,16}$",      // Kuwait
      "^996[1-9][0-9]{6,16}$",      // Kyrgyz Republic
      "^856[1-9][0-9]{6,16}$",      // Laos
      "^371[1-9][0-9]{6,16}$",      // Latvia
      "^961[1-9][0-9]{6,16}$",      // Lebanon
      "^266[1-9][0-9]{6,16}$",      // Lesotho
      "^231[1-9][0-9]{6,16}$",      // Liberia
      "^218[1-9][0-9]{6,16}$",      // Libya
      "^423[1-9][0-9]{3,14}$",      // Liechtenstein
      "^370[1-9][0-9]{6,16}$",      // Lithuania
      "^352[0-9][0-9]{3,14}$",      // Luxembourg
      "^853[1-9][0-9]{6,16}$",      // Macao
      "^389[1-9][0-9]{6,16}$",      // Macedonia (Former Yugoslav Rep of.)
      "^261[1-9][0-9]{6,16}$",      // Madagascar
      "^265[1-9][0-9]{6,16}$",      // Malawi
      "^60[1-9][0-9]{6,16}$",       // Malaysia
      "^960[1-9][0-9]{6,16}$",      // Maldives
      "^223[1-9][0-9]{6,16}$",      // Mali Republic
      "^356[1-9][0-9]{6,16}$",      // Malta
      "^692[1-9][0-9]{6,16}$",      // Marshall Islands
      "^222[1-9][0-9]{6,16}$",      // Mauritania
      "^230[1-9][0-9]{6,16}$",      // Mauritius
      "^52[1-9][0-9]{6,16}$",       // Mexico
      "^691[1-9][0-9]{6,16}$",      // Micronesia, (Federal States of)
      "^373[1-9][0-9]{6,16}$",      // Moldova
      "^377[1-9][0-9]{5,14}$",      // Monaco
      "^976[1-9][0-9]{6,16}$",      // Mongolia
      "^382[1-9][0-9]{6,16}$",      // Montenegro
      "^212[1-9][0-9]{6,16}$",      // Morocco
      "^258[1-9][0-9]{6,16}$",      // Mozambique
      "^95[1-9][0-9]{6,16}$",       // Myanmar
      "^264[1-9][0-9]{6,16}$",      // Namibia
      "^674[1-9][0-9]{6,16}$",      // Nauru
      "^977[1-9][0-9]{6,16}$",      // Nepal
      "^31[1-9][0-9]{5,14}$",       // Netherlands
      "^687[1-9][0-9]{6,16}$",      // New Caledonia
      "^505[1-9][0-9]{6,16}$",      // Nicaragua
      "^227[1-9][0-9]{6,16}$",      // Niger
      "^234[1-9][0-9]{6,16}$",      // Nigeria
      "^683[1-9][0-9]{6,16}$",      // Niue
      "^672[1-9][0-9]{6,16}$",      // Norfolk Island
      "^47[1-9][0-9]{7,12}$",       // Norway
      "^968[1-9][0-9]{6,16}$",      // Oman
      "^92[1-9][0-9]{6,16}$",       // Pakistan
      "^680[1-9][0-9]{6,16}$",      // Palau
      "^970[1-9][0-9]{6,16}$",      // Palestinian Settlements
      "^507[1-9][0-9]{6,16}$",      // Panama
      "^675[1-9][0-9]{6,16}$",      // Papua New Guinea
      "^595[1-9][0-9]{6,16}$",      // Paraguay
      "^51[1-9][0-9]{6,16}$",       // Peru
      "^63[1-9][0-9]{6,16}$",       // Philippines
      "^48[1-9][0-9]{6,16}$",       // Poland
      "^351[1-9][0-9]{5,14}$",      // Portugal (incl Azores & Madeira)
      "^974[1-9][0-9]{6,16}$",      // Qatar
      "^262[1-9][0-9]{5,14}$",      // Réunion (Le) Island
      "^40[1-9][0-9]{6,16}$",       // Romania
      "^7[0-9]{6,16}$",             // Russia and Kazakhstan
      "^250[1-9][0-9]{6,16}$",      // Rwandese Republic
      "^290[1-9][0-9]{6,16}$",      // St. Helena
      "^508[1-9][0-9]{6,16}$",      // St. Pierre & Miquelon
      "^685[1-9][0-9]{6,16}$",      // Samoa
      "^378[1-9][0-9]{6,16}$",      // San Marino
      "^239[1-9][0-9]{6,16}$",      // São Tomé and Principe
      "^966[1-9][0-9]{6,16}$",      // Saudi Arabia
      "^221[1-9][0-9]{6,16}$",      // Senegal
      "^381[1-9][0-9]{6,16}$",      // Serbia (Yugoslavia)
      "^248[1-9][0-9]{6,16}$",      // Seychelles Republic
      "^232[1-9][0-9]{6,16}$",      // Sierra Leone
      "^65[1-9][0-9]{6,16}$",       // Singapore
      "^421[1-9][0-9]{6,16}$",      // Slovak Republic
      "^386[1-9][0-9]{6,16}$",      // Slovenia
      "^677[1-9][0-9]{6,16}$",      // Solomon Islands
      "^252[1-9][0-9]{6,16}$",      // Somali Democratic Republic
      "^27[1-9][0-9]{6,16}$",       // South Africa
      "^34[1-9][0-9]{5,14}$",       // Spain (incl Canary & Balearic Is.)
      "^94[1-9][0-9]{6,16}$",       // Sri Lanka
      "^249[1-9][0-9]{6,16}$",      // Sudan
      "^597[1-9][0-9]{6,16}$",      // Suriname
      "^268[1-9][0-9]{6,16}$",      // Swaziland
      "^467[0-9]{8,9}$",            // Sweden
      "^41[1-9][0-9]{5,14}$",       // Switzerland (Vatican City 418)
      "^963[1-9][0-9]{6,16}$",      // Syria
      "^886[1-9][0-9]{6,16}$",      // Taiwan
      "^992[1-9][0-9]{6,16}$",      // Tajikistan
      "^255[1-9][0-9]{6,16}$",      // Tanzania (Zanzibar)
      "^66[1-9][0-9]{6,16}$",       // Thailand
      "^88216[1-9][0-9]{6,16}$",    // Thuraya (Mobile Satellite service)
      "^228[1-9][0-9]{6,16}$",      // Togolese Republic
      "^690[1-9][0-9]{6,16}$",      // Tokelau
      "^676[1-9][0-9]{6,16}$",      // Tonga Islands
      "^216[1-9][0-9]{6,16}$",      // Tunisia
      "^90[1-9][0-9]{6,16}$",       // Turkey
      "^993[1-9][0-9]{6,16}$",      // Turkmenistan
      "^688[1-9][0-9]{6,16}$",      // Tuvalu
      "^256[1-9][0-9]{6,16}$",      // Uganda
      "^380[1-9][0-9]{6,16}$",      // Ukraine
      "^971[1-9][0-9]{6,16}$",      // United Arab Emirates
      "^44[1-9][0-9]{3,14}$",       // United Kingdom
      "^1[1-9][0-9]{6,16}$",        // United States of America and Canada
      // (American Samoa 1684, Antigua and Anguilla 1264, Barbuda 1268, 
      // Bahamas 1242, Barbados 1246, Bermuda 1441, British Virgin Is. 1284, 
      // Cayman Islands 1345, Dominica 1767, Dominican Republic 1829,
      // Ellipso (Mobile Satellite service) 17625, Globalstar 17637,
      // Grenada 1473, Guam 1671, ICO Global 17621, Iridium 17633, Jamaica 1876
      // Midway Island 1808, Montserrat 1664, Nevis/St. Kitts 1869, 
      // Northern Marianas Islands 1670, Puerto Rico 1787, St. Lucia 1758,
      // St. Vincent & Grenadines 1784, Trinidad & Tobago 1868,
      // Turks and Caicos Islands 1649, US Virgin Islands 1340 )
      "^878[1-9][0-9]{6,16}$",   // Universal Personal Telecommunications (UPT)
      "^598[1-9][0-9]{6,16}$",      // Uruguay
      "^998[1-9][0-9]{6,16}$",      // Uzbekistan
      "^678[1-9][0-9]{6,16}$",      // Vanuatu
      "^58[1-9][0-9]{6,16}$",       // Venezuela
      "^84[1-9][0-9]{6,16}$",       // Vietnam
      "^681[1-9][0-9]{6,16}$",      // Wallis and Futuna Islands
      "^967[1-9][0-9]{6,16}$",      // Yemen
      "^260[1-9][0-9]{6,16}$",      // Zambia
      "^263[1-9][0-9]{6,16}$",      // Zimbabwe
   };

   
   for ( uint32 i = 0 ; i < (sizeof( cnbr ) / sizeof( *cnbr )) ; ++i ) {
      int res = regcomp( &p, cnbr[ i ], c );
      if ( res != 0 ) {
         mc2log << error << "StrU::validPhonenumber regcomp failed for \"" 
                << cnbr[ i ] << "\"" << endl;
         continue;
      }
      res = regexec( &p, nbr, 0, NULL, 0 );
      regfree( &p );

      if ( res == 0 ) {
         // Found match!
         ok = true;
         break;
      }
   }

   return ok;
}


char*
StringUtility::cleanPhonenumber( const char* nbr ) {
   // Remove leading double zero
   nbr = trimStart( nbr );
   if ( nbr [ 0 ] == '0' && nbr [ 1 ] == '0' ) {
      nbr = nbr + 2;
   }
   
   // Remove 
   return removeAllButDigits( nbr );
}

bool
StringUtility::regexp( const char *regex, const char *string,
                       int cflags, int eflags )
{
   regex_t p;
   cflags |= REG_NOSUB;
   int res = regcomp( &p, regex, cflags );
   if ( res == 0 ) {
      res = regexec( &p, string, 0, NULL, eflags );
      regfree( &p );
   } else {
      const uint32 errorBuffSize = 256;
      char errorBuff[ errorBuffSize ];
      regerror( res, &p, errorBuff, errorBuffSize );
      mc2log << warn << "[StU] regcomp failed error: " << res 
             << " " << MC2CITE( errorBuff ) << endl;
   }

   return res == 0;
}

bool
StringUtility::regexp( const MC2String& regex, const MC2String& string,
                       int cflags, int eflags )
{
   return regexp( regex.c_str(), string.c_str(), cflags, eflags );
}

bool
StringUtility::regexp( const char *regex, const char *str,
                       uint32 nbrMatches,
                       vector< MC2String >& matches,
                       int cflags, int eflags )
{
   regex_t p;
   int res = regcomp( &p, regex, cflags );
   regmatch_t pmatch[ nbrMatches + 1 ];
   if ( res == 0 ) {
      res = regexec( &p, str, nbrMatches + 1, pmatch, eflags );
      if ( res == 0 ) {
         // Move pmatches to matches
         for ( uint32 i = 0 ; i < nbrMatches ; ++i ) {
            matches.push_back( MC2String( str + pmatch[ i + 1 ].rm_so, 
                                          pmatch[ i + 1 ].rm_eo - 
                                          pmatch[ i + 1 ].rm_so ) );
         }
      }
      regfree( &p );
   } else {
      const uint32 errorBuffSize = 256;
      char errorBuff[ errorBuffSize ];
      regerror( res, &p, errorBuff, errorBuffSize );
      mc2log << warn << "[StU]:regexp regcomp failed error: " << res 
             << " " << MC2CITE( errorBuff ) << endl;
   }

   return res == 0;
}

bool
StringUtility::regexp( const MC2String& regex, const MC2String& string,
                       uint32 nbrMatches,
                       vector< MC2String >& matches,
                       int cflags, int eflags )
{
   return regexp( regex.c_str(), string.c_str(), nbrMatches, matches,
                  cflags, eflags );
}

bool
StringUtility::regexpWholeBracket( const char *regex, const char *str,
                                   int cflags, int eflags )
{
   regex_t p;
   int res = regcomp( &p, regex, cflags );
   regmatch_t pmatch[ 2 ];
   if ( res == 0 ) {
      res = regexec( &p, str, 2, pmatch, eflags );
      if ( res == 0 ) {
         // Check if whole string is matched.
         if (  pmatch[ 1 ].rm_so == 0 &&
               pmatch[ 1 ].rm_eo == int(strlen( str )) ) {
            // Matches whole
         } else {
            res = -1;
         }
      }
      regfree( &p );
   } else {
      const uint32 errorBuffSize = 256;
      char errorBuff[ errorBuffSize ];
      regerror( res, &p, errorBuff, errorBuffSize );
      mc2log << warn << "[StU]:regexp regcomp failed error: " << res 
             << " " << MC2CITE( errorBuff ) << endl;
   }

   return res == 0;
}

bool
StringUtility::regexpWholeBracket( 
   const MC2String& regex, const MC2String& string,
   int cflags, int eflags )
{
   return regexpWholeBracket( regex.c_str(), string.c_str(), cflags, eflags );
}

bool
StringUtility::regexpCheck( const char *regex, 
                            MC2String& error, int cflags ) {
   regex_t p;
   int res = regcomp( &p, regex, cflags );
   if ( res != 0 ) {
      const uint32 errorBuffSize = 256;
      char errorBuff[ errorBuffSize ];
      regerror( res, &p, errorBuff, errorBuffSize );
      error = errorBuff;
   }
   regfree( &p );

   return res == 0;
}


bool
StringUtility::regexpCheck( const MC2String& regex, 
                            MC2String& error, int cflags ) {
   return regexpCheck( regex.c_str(), error, cflags );
}


MC2String 
StringUtility::decodeHtmlString( const MC2String& str ) {
   //
   // Search for exp: &#.*;
   // and try to convert the value between "&#" and ";"
   // to an integer.
   //

   // get start position
   MC2String::size_type startPos = str.find( "&#" );
   if ( startPos == MC2String::npos ) {
      // didnt find anything, lets return unmodified
      return str;
   }

   MC2String returnString;

   // now, if we found one, lets copy
   // the data between the start and the position
   // of the first &#-hit
   if ( startPos != 0 ) {
      returnString += str.substr( 0, startPos );
   }

   MC2String::size_type endPos = MC2String::npos;
   while ( startPos != MC2String::npos ) { 

      // now if previous endPos was valid, copy the
      // string between.
      if ( endPos != MC2String::npos ) {
         returnString += str.substr( endPos + 1, startPos - endPos - 1);
      }

      //
      // find the end point;
      // Instead of actually searching for the ';' 
      // a "non-number" search is done instead, this is to catch
      // cases with missing ';' for example:
      //  " string string &#237; missing &#237 valid : &#237; "
      // which will be translated to:
      //  " string string í missing &#237 valid : í "
      //
      MC2String::size_type oldEndPos = endPos;
      endPos = str.find_first_not_of( "0123456789", startPos + 2 );
      if ( endPos == MC2String::npos ||
           str[ endPos ] != ';' ) {
         // did not find any end point, lets finish
         //         returnString += str.data() + startPos;
         // restore old end position
         endPos = oldEndPos;
         // next position please
         startPos = str.find( "&#", startPos + 2 );
         continue;
      }

      // try to convert the number between start end end position
      try {
         uint32 code = StringConvert::
            convert<uint32>( str.substr( startPos + 2, endPos - startPos - 2));
         returnString += UTF8Util::ucsToUtf8( code );
      } catch ( const StringConvert::ConvertException& what ) {
         // add invalid html number thingy, this should not actually happen
         // since we are looking for a non-number value as end point....
         // but just in case;
         returnString += str.substr( startPos, startPos - endPos );
      }
      // next position please
      startPos = str.find( "&#", endPos + 1 );
   }

   // if there was a valid end position, then we have
   // some more text to copy
   if ( endPos != MC2String::npos ) {
      returnString += str.data() + endPos + 1;
   }

   return returnString;
}


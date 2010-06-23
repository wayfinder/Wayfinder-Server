/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGUTILITY_H
#define STRINGUTILITY_H

#include "config.h"
#include "MC2String.h"
#include "TextIterator.h"
#include "TextConversionTables.h"
#include "UTF8Util.h"
#include <set>
#include <regex.h>

#ifdef __linux
   #include <sys/time.h>
#elif _MSC_VER
   #include <time.h>
   #ifdef _DEBUG
      #pragma warning( disable : 4786 )
   #endif
#endif

inline const char* BP( bool B ) {
   return B ? "True" : "False";
}

/**
  *   Some useful string functions.
  *
  */
class StringUtility {
public:
      /**
       *   Converts the name string into close ident for use in
       *   SearchModule.
       *   @param name String to convert.
       *   @param keepSpaces True if spaces should be kept for
       *                     word-matching.
       *   @return New string for use as exact ident.
       */ 
      static MC2String convertIdentToClose(const MC2String& name,
                                           bool keepSpaces = false);

      static inline MC2String upper(uint32 ucs);
      
      static inline MC2String lower(uint32 ucs);
      
      /**
       * Makes a uppercase copy of str and returns it.
       * The iterator is automatically created from e.g. const char*
       * or MC2String, so please do not do any casts or temporary
       * variables.
       * @param str is the string to copy and to upercase.
       * @returns the new uppercase string.
       */
      static inline MC2String copyUpper(mc2TextIterator str);

      /**
       * Makes a lowercase copy of str and returns it. 
       * The iterator is automatically created from e.g. const char*
       * or MC2String, so please do not do any casts or temporary
       * variables.
       * @param str is the string to copy and to lowercase.
       * @returns the new lowercase string.
       */
      static inline MC2String copyLower(mc2TextIterator str);
      
      /**
        *   Compare two strings. Works like the system call strcmp (this
        *   method currently only pass the call to the system).
        *
        *   @param   strA  The first string to compare.
        *   @param   strB  The second string to compare.
        *   @return  An integer less than 0 if strA < strB, 0 if 
        *            strA == strB and an integer > 0 if strA > strB.
        */
      static inline int strcmp(const char* strA, const char* strB);

      /**
        *   Compare two strings ignoring case. Works like the system call
        *   strcasecmp, but this also works with swedish characters!
        *   Sorts, does not collate!
        *
        *   @param   strA  The first string to compare.
        *   @param   strB  The second string to compare.
        *   @return  An integer less than 0 if strA < strB, 0 if 
        *            strA == strB and an integer > 0 if strA > strB.
        */
      static inline int strcasecmp(const mc2TextIterator& strA,
                                   const mc2TextIterator& strB);

      /**
        *   Compare the n first letters in two strings, ignoring case. 
        *   Works like the system call strncasecmp, but this also works 
        *   with swedish characters!
        *
        *   @param   strA  The first string to compare.
        *   @param   strB  The second string to compare.
        *   @param   n     The maximum number of characters to compare.
        *   @return  An integer 
        *            - less than 0 if (the first n letters of) strA < strB
        *            - equal to 0  if strA == strB
        *            - greater than 0 if strA > strB.
        */
      static inline int strncasecmp(const mc2TextIterator& strA,
                                    const mc2TextIterator& strB,
                                    int n);

      /**
       *    Locate a substring.
       *    Works like the system call strstr.
       *    @param   haystack Haystack.
       *    @param   needle   Needle.
       *    @return  The first occurance of the substring needle in the
       *             string haystack.
       */
      static inline char* strstr( const char* haystack, 
                                  const char* needle );

      /**
       * Locate a character in a string.
       * Works like the system call strchr.
       *
       * @param s The string to search.
       * @param c The character to search for in s.
       * @return The positino of the first c in s or NULL if no c in s.
       */
      static char* strchr( const char *s, int c );

      /**
       * Locate the last occurrence character in a string.
       * Works like the system call strrchr.
       *
       * @param s The string to search.
       * @param c The character to search for in s.
       * @return The positino of the first c in s or NULL if no c in s.
       */
      static char* strrchr( const char *s, int c );
      
      /**
        *   Removes whitespace at the end of the string given as parameter,
        *   can also remove other characters at the end.
        *   @param   s           The string that should be trimmed.
        *   @param   additional  Optional additional characters to remove.
        */
      static inline void trimEnd(char* s, const char* additional = "");

      /**
        *   Search for the first character in a string that is not
        *   whitespace (or optionally additional characters).
        *   @param   s          The string that should be trimmed.
        *   @param   additional Optional additional characters to skip over.
        *   @return  A pointer to the first non-whitespace character
        *            in s.
        */
      static inline char* trimStart(const char* s, 
                                    const char* additional = "");

      /**
       *    Trims start and end of the string.
       *    @param s           The string to trim.
       *    @param additional  Optional additional characters to trim.
       */
      static MC2String trimStartEnd( const char* s,
                                     const char* additional = "");

      /**
       *    Trims start and end of the string.
       *    @param s           The string to trim.
       *    @param additional  Optional additional characters to trim.
       */
      static MC2String trimStartEnd( const MC2String& orig,
                                     const char* additional = "");
     

   /**
    * Trims whitespace from the beginning of a string. 
    * Whitespace are all characters identified by the isspace function:
    * space, form-feed ('\f'), newline ('\n'), carriage return ('\r'),
    * horizontal tab ('\t'), and vertical tab ('\v').
    * @param str The string to trim. Note that the argument is a
    *            reference and is modified by the function.
    * @return The str argument. 
    */
   static MC2String& trimStartInPlace(MC2String& str);
   /**
    * Trims whitespace from the end of a string. 
    * Whitespace are all characters identified by the isspace function:
    * space, form-feed ('\f'), newline ('\n'), carriage return ('\r'),
    * horizontal tab ('\t'), and vertical tab ('\v').
    * @param str The string to trim. Note that the argument is a
    *            reference and is modified by the function.
    * @return The str argument. 
    */
   static MC2String& trimEndInPlace(MC2String& str);
   /**
    * Trims whitespace from both the beginning and the end of a string. 
    * Whitespace are all characters identified by the isspace function:
    * space, form-feed ('\f'), newline ('\n'), carriage return ('\r'),
    * horizontal tab ('\t'), and vertical tab ('\v').
    * @param str The string to trim. Note that the argument is a
    *            reference and is modified by the function.
    * @return The str argument. 
    */
   static MC2String& trimInPlace(MC2String& str);


      /**
       * Makes the first char in str uppercase.
       * @param str is the string in which to set the first letter to uppercase.
       * @return a pointer to str.
       */
      static MC2String makeFirstCapital( const MC2String& str );

      /**
        *   Make the first character in every word uppercase, and all
        *   other characters lowercase.
        *   WARNING! Is not really UTF-8 aware. Only works because 
        *            X, I and V are ascii.
        *   @param str   The string where to change case.
        *   @param makeRomanNumbersUpper
        *                True if roman numerals (e.g. "XI" or "IV") should
        *                be written with upper case letters.
        *   @return  A pointer to str.
        */
      static MC2String makeFirstInWordCapital(const MC2String& str, 
                                              bool makeRomanNumbersUpper = false);
      
      
      static int isRomanNumeral(const char* str, uint32 n);

      static void makeRomanNumeralsUpper( char* str );

      /**
        *   Allocates memory for, and copies a string using new
        *   instead of malloc (that the normal strdup uses).
        *   The copied string may be deleted using delete instead of free.
        *   @param   s  The string to copy.
        *   @return  A copy of s.
        */
      static inline char* newStrDup(const char* s);

      /**
        *   @name Timeformatting methods
        */
      //@{
         /**
           *   Translate a timedifferance in seconds to hours, minutes and
           *   seconds.
           *
           *   @param   deltaTime   The time differance in seconds.
           *   @param   hour        Outparamter that is set to the number
           *                        of hours in deltatime.
           *   @param   minute      Outparamter that is set to the number
           *                        of minutes (0-59) in deltatime.
           *   @param   second      Outparamter that is set to the number
           *                        of seconds (0-59) in deltatime.
           *   @return  True if caluculation when alright, false otherwise.
           */
         static bool splitSeconds(  uint32 deltaTime, uint32& hour, 
                                    uint32& minute, uint32& second);

         /**
           *   Prints one delta time (in seconds) into a preallocated 
           *   string. Calls splitSeconds internally.
           *   @param   deltaTime   The time difference to print (in s).
           *   @param   str         The string where the result should
           *                        be written.
           *   @return  A pointer to str.
           */
         static char* splitSeconds(uint32 deltaTime, char* str);
      //@}
      
      /**
       *    Does the exact same thing as the strsep() that can be found
       *    in for instance Linux, but is missing in Solaris.
       *    It returns the next token from the string stringp which is
       *    delimited by a character in delim. The token is terminated
       *    with a '\0' character and stringp is updated to point past
       *    the token.
       *    @param stringp Pointer to a pointer the string to tokenize
       *    @param delim   Pointer to string containing the delimiting
       *                   character(s)
       *    @return Pointer to the next token or NULL if delim isn't found.
       */
      static char* strsep(char** stringp,
                          const char* delim);
      
      /**
       * Turns a token separated string into a vector of strings.
       * Only the vector should be deleted. (not any of the elements.)
       * The list is altered to have '\0's where there where seps.
       * The list should be deleted after deleting the vector.
       * @param out A vector that will be filled with tokens.
       * @param list is the list of items with sep separated words.
       * @param sep is the separator.
       * @param skipEmptyTokens if false, the empty string is a valid token
       * @param trimStart If true, the tokens will be trimmed.
       * @return a char vector pointing into list.
       */
   static void tokenListToVector( vector< char* >& out,
                                  char* list, 
                                  char sep,
                                  bool skipEmptyTokens = true,
                                  bool trimStart = true);

      /**
       *   @see tokenListToVector
       *   Probably not very efficient.
       */
   static void tokenListToVector( vector<MC2String>& splitStr,
                                  const MC2String& toSplit,
                                  char sep,
                                  bool skipEmptyTokens = true,
                                  bool trimStart = true );

   /**
    * Turns a token separated string into a vector of strings.
    * Uses boost::escaped_list_separator
    * Escape character \ and quote character " can be used.
    * http://www.boost.org/doc/libs/1_32_0/libs/tokenizer/escaped_list_separator.htm
    *
    * @param toSplit The string to split
    * @param separator The separator used when splitting
    * @param splitStr Vector containing the splitted strings
    * @param skipEmptyTokens if false, the empty string is a valid token
    */
   static void splitToVector( const MC2String& toSplit,
                              char separator,
                              vector< MC2String >& splitStr,
                              bool skipEmptyTookens = true );
   
      /**
       * Sort of a MC2String and STL version of strtok, only without most
       * of strtoks problems.  Most importantly it is reentrant and
       * doesn't modify its first argument.  It still has the drawback
       * that you loose the identity of the delimiting character.
       *
       * If s is the empty string, no tokens are generated. 
       *
       * If s contains two adjacent delimiter characters an empty string
       * token will be generated for the string between them.
       *
       * If s has delimiters at the beginning or end, empty string tokens
       * will be generated.
       *
       * As a result of the above rules, if s contains only a single
       * delimiter character, two empty string tokens will be generated.
       *
       * @param s The string to tokenize.
       * @param delim Lists the delimiters.
       * @param out Output iterator for storing the tokens. Use
       *            back_inserter to insert elements into containers.
       */
      template<class OutIt>
      static void strtok(const MC2String& s, const MC2String& delim, OutIt out)
      {
         MC2String::size_type endpos = 0;
         MC2String::size_type startpos = 0;
         while(MC2String::npos != (endpos = s.find_first_of(delim, startpos))){
            *out++ = s.substr(startpos, endpos - startpos);
            startpos = endpos + 1;
         }
         *out++ = s.substr(startpos, endpos - startpos);
      }
      
      /**
        *   Check if a given string only contains digits.
        *   @param   str   The string to check.
        *   @param   ignoreSpace If leading and trailing space should
        *            be ignored.
        *   @return  True if the given string only contains digits,
        *            false otherwise.
        */
      static bool onlyDigitsInString( const char* str, 
                                      bool ignoreSpace = false );

      /**
       *    Returns true if the string contains one or more digits.
       *    @param str String to look in.
       *    @return True if there was a digit in the string.
       */
      static bool containsDigit( const char* str );

      /**
       * Replaces chars by other chars.
       * @param str the string to change
       * @param searchFor the char to replace
       * @param replaceWith the char to replace the searchFor chars with.
       * @param stopAt The number of characters to check.
       */
      static int replaceChar(char *str,
                             char searchFor,
                             char replaceWith,
                             int stopAt = MAX_INT32 );

      /**
       *  Replaces strings in strings. A new string or NULL is returned,
       *  the original string is not deleted, the caller has to delete
       *  the new string when done.
       *  @param source      The original string
       *  @param searchFor   The string to replace
       *  @param replaceWith The string to replace with. Can be NULL or just
       *                     "".
       *  @param maxReplace  The maximum number of strings to replace, 0 means
       *                     replace all ocurrences (also default).
       *  @return Pointer to a new string with all replacements done, NULL if no
       *          replacements were made;
       *
       *  NOTE: This method is used in both MC2 and Nav2.                              *        Please change both if bugs are found.
       */
      static char* replaceString( const char* source,
                                  const char* searchFor,
                                  const char* replaceWith,
                                  int maxReplace = 0 );

      /**
       *    Checks if a character is alphanumerical (all characters
       *    above 127 are considered alphanumerical).
       *    @param   ch The character (as byte).
       *    @return True If the character is alphanumerical.
       */
      static inline bool isAlNum(uint32 ucs);

      static inline bool isSpace(uint32 ucs);
      
      /**
       * Converts a SQLdate to Unix time.
       * This function ONLY works here in Sweden at this point
       */
      static uint32 makeDate( const char* date );

      /**
       * Makes a date and time string from a time value.
       * dateStr must be of a length of 11 and timeStr 9.
       *
       * @param time The time in UTC.
       * @param dateStr The string to print date into.
       * @param timeStr The string to print time into.
       * @return The number of chars written to timeStr.
       */
      static int makeDateStr( uint32 time, char* dateStr, char* timeStr );

      /**
       * Format a time string according to fmt, see man strftime for all
       * format strings.
       *
       * @param s The string to write time into.
       * @param max The maximum size of chars written to s.
       * @param format The format string for the time to write.
       * @param tm The time struct with the date.
       */
      static size_t strftime( char* s, size_t max, const char* format,
                              const struct tm *tm );

      /**
       * Obsolete! Use TimeUtility::convertDate
       */
      static uint32 convertDate(uint32 year, uint32 month, uint32 day);

      /** Safer method of using strncat, inspired by Secure-Programs-HOWTO.
       *  @param dest Where to append the source.
       *  @param src  The source to copy from.
       *  @param n    The number of characters to copy.
       *  Compared to the strncat function, the resulting string will always be
       *  NULL-terminated. (at least it's supposed to be)
       */
      static inline char *strlcat(char *dest, const char *src, size_t n);

      /**
       *    USE utf8lcpy IF UTF-8 is used.
       *    Safer method of using strncpy, inspired by Secure-Programs-
       *    HOWTO/BSD. Compared to the strncat function, the resulting 
       *    string will always be NULL-terminated. (at least it's 
       *    supposed to be).
       *
       *    @param dest Where to append the source.
       *    @param src  The source to copy from.
       *    @param n    The number of characters to copy.
       */
      static inline char *strlcpy(char *dest, const char *src, size_t n);


      /**
       *    strlcpy for utf-8. Will only copy complete utf-8 characters.
       *    Will zero-terminate destination if n != 0.
       *    @param dest Destination address.
       *    @param src  Source address.
       *    @param n    Maximum number of bytes to copy. One byte for the
       *                zero term.
       *    @return Number of bytes in src.
       */
      static size_t utf8lcpy( char *dest, const char *src, size_t n );
   
      /**
       * Removes all non digit chars from str and returns a copy 
       * with only the digits.
       *
       * @param str The string to extract digits from.
       * @return A new string with the digits from str, caller must
       *         delete it.
       */
      static char* removeAllButDigits( const char* str );

      /**
       * Prints random chars into target for nbr chars.
       */
      static void randStr( char* target, uint32 nbr );

      /**
       * WAPifies str into target.
       *
       * @param target String to write WAP safe version of str into.
       * @param str String to WAPify.
       * @return Number chars wrtitten to target.
       */
      static uint32 wapStr( char* target, const char* str );

      /**
       * Converts a string with swedish characters to english by removing
       * the "dots" and "rings". This is handy when creating files where
       * the filename has swedish characters in it.
       *
       * @param pszTarget The resulting string without swedish chars.
       * @param pszSource The string containing swedish chars.
       */
      static void swe2eng( char* pszTarget, const char* pszSource );


      /**
       * Base 64 encodes a string.
       * Is NOT optimized.
       *
       * @param input The string to encode.
       * @param output The string to write the encoded chars to.
       *               Base64encoding takes 4/3 + 4 more chars
       *               than there are chars in the input string.
       *               If you use maxLineLength then there is
       *               (inputLength * 4/3 / maxLineLength) * 2 more chars.
       * @param maxLineLength The maximun length before breaking a line
       *                      with carrige return and line feed.
       * @return True, there are no faulty strings that can be detected
       *         by this method.
       */
      static bool base64Encode( const char* input, char* output,
                                uint32 maxLineLength = MAX_UINT32 );

      /**
       *   Base 64 encodes a string.
       *   Is NOT optimized.
       *   @param instr The input string
       *   @return The input string base64 encoded.
       */
      static MC2String base64Encode( const MC2String& instr,
                                     uint32 maxLineLength = MAX_UINT32 );

      /**
       * Base 64 encodes a buffer.
       * Is NOT optimized.
       *
       * @param input The buffer to encode.
       * @param inputLength The size of the input buffer.
       * @param output The string to write the encoded chars to.
       *               Base64encoding takes 1/3 + 4 more chars
       *               than there are chars in the input string.
       *               If you use maxLineLength then there is
       *               (inputLength / maxLineLength) * 2 more chars.
       * @param maxLineLength The maximun length before breaking a line
       *                      with carrige return and line feed.
       * @return True, there are no faulty inputs that can be detected
       *         by this method.
       */
      static bool base64Encode( const byte* input, uint32 inputLength,
                                char* output, 
                                uint32 maxLineLength = MAX_UINT32 );
      
      
      /**
       * Base 64 decode a string.
       * Is NOT optimized.
       *
       * @param input The string to decode.
       * @param output The bytearray to write the decoded chars to.
       *               Base64decoding takes 1/4 less chars
       *               than there are chars in the input string.
       *               The bytearray is nullterminated.
       * @return The length if the string decoded ok,
       *         -1 if faulty Base64 input.
       */
      static int base64Decode( const char* input, byte* output );

      /**
       *  URLEncodes the zero-terminated inbuf into outbuf,
       *  which will also be zero-terminated. Each input char
       *  can result in three output chars.
       *  @param outbuf The result. Should be 3*strlen(inbuf)+1 long.
       *  @param inbuf  The buffer to encode.
       *  @return The length of the written string.
       *
       *  NOTE: This method is used in both MC2 and Nav2.                 
       *        Please change both if bugs are found.
       */
      static int URLEncode(char* outbuf,
                           const char* inbuf);

      /**
       *  URLEncodes the inbuf into outbuf. Outbuf will not be 
       *  zero terminated.
       *  @param outbuf   The result. Should be 3*inlength long.
       *  @param inbuf    The buffer to encode.
       *  @param inlength The length of the inbuffer.
       *  @return Number of bytes written to outbuf.
       *
       *  NOTE: This method is used in both MC2 and Nav2.                              
       *        Please change both if bugs are found.
       */
      static int URLEncode(char* outbuf,
                           const char* inbuf,
                           int inlength);

      /**
       *   URLEncodes the string.
       */
      static MC2String URLEncode( const MC2String& src );
      
      
      /**
       * URLDecodes the inbuf onto outbuf. Outbuf will not be 
       * zero terminated.
       *  @param outbuf   The result. Should be inlength long.
       *  @param inbuf    The buffer to decode.
       *  @return Number of bytes written to outbuf.
       *
       *  NOTE: This method is used in both MC2 and Nav2.                              *        Please change both if bugs are found.
       */
      static int URLDecode(byte* outbuf,
                           const char* inbuf);


      /**
       *  "SQL" escapes a string, adds the result to a string.
       *  Escaped characters: ' " \n \r \0 \ 
       *  @param sre    Pointer to string to add/escape
       *  @param dest   Pointer to the destination, the escaped string is added
       *                and then NULL terminated. Destination needs enough space
       *                to fit strlen(source) * 2 + 1 (worst case)
       *  @param max    Maximum characters to add/escape, regardless of the
       *                length, default is no limit.
       *  @param len    Length of string, any \0 is then handled as part of the
       *                string. Set to 0 to use the null termination in source
       *                instead.
       *  @return       Pointer to the dest string.
       */
      static char* SQLEscape(const char* src, char* dest,
                             uint32 max = MAX_UINT32, uint32 len = 0);

      /**
       *  "SQL" escapes a string, like SQLEscape, but escapes additional
       *  potentially "dangerous charactes. 
       *  The full set escaped: ' " \n \r \0 \ % _
       *  @param src    Pointer to string to add/escape
       *  @param dest   Pointer to the destination, the escaped string is added
       *                and then NULL terminated. Destination needs enough space
       *                to fit strlen(source) * 2 + 1 (worst case)
       *  @param max    Maximum characters to add/escape, regardless of the
       *                length, default is no limit.
       *  @param len    Length of string, any \0 is then handled as part of the
       *                string. Set to 0 to use the null termination in source
       *                instead.
       *  @return       Pointer to the dest string.
       */
      static char* SQLEscapeSecure(const char* src, char* dest, 
                                   uint32 max = MAX_UINT32, uint32 len = 0);

      /**
       *    @see SQLEscape(char*)
       */
      static MC2String SQLEscape( const MC2String& src );

      /**
       *    @see SQLEscapeSecure(char*)
       */
      static MC2String SQLEscapeSecure( const MC2String& src);

      /**
       *  Compares two strings, one or both of which may be NULL.
       *  Using StringUtility::strcasecmp. (case insensitive)
       *  @param string1 The first string.
       *  @param string2 The second string.
       *  @return The result of strcasecmp, or similar if strings are NULL.
       */
      static inline int null_strcmp(const char* string1, 
                                    const char* string2);

      /**
       * Checks if a string is "true" or "false" and retuns true or false.
       * If string isn't "true" or "false" then default is returned.
       * 
       * @param str The string to check for boolean.
       * @param defaultValue The default value if str is not recognized as
       *        a boolean. Default false.
       */
      static bool checkBoolean( const char* str, 
                                bool defaultValue = false );

      /**
       * Returns string "true" or "false" depending on parameter.
       * 
       * @param value The bool to return as string.
       * @return String representing value.
       */
      static const char* booleanAsString( bool value );

      /**
       * Checks if a string is a valid email-address.
       * The check is to see if the string looks like [string]@[string].
       * @param emailAddress The string to check.
       * @return True if emailAddress is a valid email-address, 
       *         false if not.
       */
      static bool validEmailAddress( const char* emailAddress );


      /**
       * Checks if a string ends with a certain string.
       * All strings must be null-terminated.
       * @param   str            The string to check.
       * @param   endStr         The end-string to check for.
       * @param   caseSensitive  Whether the comparison should be case
       *                         sensitive or not. Default not case
       *                         sensitive.
       *  @return True if str ends with endStr, otherwise false.
       */
      static bool endsWithStr( const char* str, 
                               const char* endStr, 
                               bool caseSensitive = false );


      /**
       * Adds a number of months to a time.
       *
       * @param now The time to start at.
       * @param months The number of months to add to now.
       * @returns now + months in seconds (UTC).
       */
      static uint32 addMonths( uint32 now, uint32 months );

      /**
       * Adds a number of years to a time.
       *
       * @param now The time to start at.
       * @param years The number of years to add to now.
       * @returns now + years in seconds (UTC).
       */
      static uint32 addYears( uint32 now, uint32 years );


      /**
       * @name STL converting methods
       */
      //@{
      /**
       * Converts a StringVector to a set of const char*.
       */    
      static const set<const char*>
      stringVectorToCharSet( const vector< char* >& strVector );
      //@}                                                  

   static int parseCommaSepInts(vector<uint32>& result,
                                const char* inStr);


      /**
       * Checks if a string is a valid phonenumber.
       *
       * @param nbr The phonenumber to check.
       * @return True if nbr is a valid phonenumber, false if not.
       */
      static bool validPhonenumber( const char* nbr );


      /**
       * Cleans phonenumber string from non numbers and leading double 
       * zero.
       *
       * @param nbr The phonenumber to clean.
       * @return The cleaned phonenumber, newed must be deleted by caller.
       */
      static char* cleanPhonenumber( const char* nbr );

      /**
       * Match a string using a regular expression.
       * See man regexec for cflags and eflags. REG_NOSUB is always set in
       * cflags.
       *
       * @param regex The regular expression.
       * @param string The string to match.
       * @param cflags See man regcomp.
       * @param eflags See man regexec.
       * @return True if matching, false if not.
       */
      static bool regexp( const char *regex, const char *string,
                          int cflags = REG_EXTENDED, int eflags = 0 );
      static bool regexp( const MC2String& regex, const MC2String& string,
                          int cflags = REG_EXTENDED, int eflags = 0 );

      /**
       * Match a string using a regular expression with substrings.
       * See man regexec for cflags and eflags.
       *
       * @param regex The regular expression.
       * @param string The string to match.
       * @param nbrMatches The number of substring in regex.
       * @param matches The substrings are added to this.
       * @param cflags See man regcomp.
       * @param eflags See man regexec.
       * @return True if matching, false if not.
       */
      static bool regexp( const char *regex, const char *string,
                          uint32 nbrMatches,
                          vector< MC2String >& matches,
                          int cflags = REG_EXTENDED, int eflags = 0 );
      static bool regexp( const MC2String& regex, const MC2String& string,
                          uint32 nbrMatches,
                          std::vector< std::string >& matches,
                          int cflags = REG_EXTENDED, int eflags = 0 );


      /**
       * Match a string using a regular expression with a bracket expression
       * around it and checks if the whole string was matched.
       * Like "([a]+)" Matches any non empty string with only 'a's in it.
       * See man regexec for cflags and eflags.
       *
       * @param regex The regular expression, surounded by ().
       * @param string The string to match.
       * @param cflags See man regcomp.
       * @param eflags See man regexec.
       * @return True if matching, false if not.
       */
      static bool regexpWholeBracket( 
         const char *regex, const char *string,
         int cflags = REG_EXTENDED, int eflags = 0 );
      static bool regexpWholeBracket( 
         const MC2String& regex, const MC2String& string,
         int cflags = REG_EXTENDED, int eflags = 0 );

      /**
       * Check if a string is a valid regular expression.
       *
       * @param regex The regular expression to check.
       * @param error Set to the error if not ok.
       * @return True if valid expression, false if not.
       */
      static bool regexpCheck( const char *regex,
                               MC2String& error, int cflags = REG_EXTENDED );
      static bool regexpCheck( const MC2String& regex,
                               MC2String& error, int cflags = REG_EXTENDED );

   /**
    *
    * Decodes number in format "&#<number>;" ( without the <> ) to an utf-8
    * code on the string.
    * @return decoded html string
    */
   static MC2String decodeHtmlString( const MC2String& str );

  private:
      /**
       *   Returns a pointer to a string containing the ucs character
       *   in upper case. Do not trust that the string is copied into
       *   maybe_used.
       */
      static inline const char* upperNoAlloc( uint32 ucs, char* maybe_used );

      /** 
        *   Private constructor to avoid objects of this class.
        */
      StringUtility();

};


// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

int
StringUtility::null_strcmp(const char* string1, const char* string2)
{
   if ( string1 == NULL && string2 != NULL )
      return -1;
   if ( string1 != NULL && string2 == NULL )
      return 1;
   if ( string1 == NULL && string2 == NULL )
      return 0;
   return StringUtility::strcasecmp(string1, string2);
}

inline char *
StringUtility::strlcat(char *dest, const char *src, size_t n)
{
   // inefficient implementation,
   // to be replaced by standard library function strlcat
   size_t destLen = strlen(dest);
   strncat(dest, src, n);  // overwrites the terminating null of dest.
   if (destLen > 0) {
      dest[destLen+n] = '\0';
   }
   return dest;
}

inline char *
StringUtility::strlcpy(char *dest, const char *src, size_t n)
{
   // inefficient implementation,
   // to be replaced by standard library function strlcpy
   strncpy(dest, src, n);
   if (n > 0) {
      dest[n-1] = '\0';
   }
   return dest;
}


inline int 
StringUtility::strcmp(const char* strA, const char* strB)
{
   return ::strcmp(strA, strB);
}

inline const char*
StringUtility::upperNoAlloc( uint32 ucs, char* maybe_used )
{
   TextConversionTables::convTableFindRes result =
      TextConversionTables::findInTable(TextConversionTables::c_upperTable,
                                        ucs);
   if ( result.resString != NULL ) {
      // This one points into the table
      return result.resString;
   } else {
      // Put the original character into the buffer.
      UTF8Util::unicodeToUtf8( ucs, maybe_used );
      return maybe_used;
   }
}

inline int
StringUtility::strcasecmp( const mc2TextIterator& strA,
                           const mc2TextIterator& strB )
{
   // ===============================================================
   // This is copied to the strncasecmp-method!                     =
   // ===============================================================

   // Initiate the return value
   int res = 0;
   char utf8a[8]; // I guess 7 would do here.
   char utf8b[8];
   mc2TextIterator itA(strA);
   mc2TextIterator itB(strB);
   uint32 unicodeA = *itA;
   uint32 unicodeB = *itB;

   while(((res = strcmp(upperNoAlloc(unicodeA, utf8a),
                        upperNoAlloc(unicodeB, utf8b) ) ) == 0)  &&
         (unicodeA != 0) && (unicodeB != 0) )
   {
      ++itA;
      ++itB;
      unicodeA = *itA;
      unicodeB = *itB;
   }
   
   // Return the value
   return res;
}

inline int
StringUtility::strncasecmp(const mc2TextIterator& strA,
                           const mc2TextIterator& strB,
                           int n)
{
   // ===============================================================
   // This is more or less a plain copy of the strcasecmp-method,   =
   // the reason to copy is to not make that method slower...       =
   // ===============================================================
   
// Initiate the return value
   int res = 0;
   int pos = 1;
   char utf8a[8]; // I guess 7 would do here.
   char utf8b[8];
   mc2TextIterator itA(strA);
   mc2TextIterator itB(strB);
   uint32 unicodeA = *itA;
   uint32 unicodeB = *itB;

   while(((res = strcmp(upperNoAlloc(unicodeA, utf8a),
                        upperNoAlloc(unicodeB, utf8b) ) ) == 0)  &&
         (unicodeA != 0) && (unicodeB != 0) && (n > pos++) )
   {
      ++itA;
      ++itB;
      unicodeA = *itA;
      unicodeB = *itB;
   } 

   // Return the value
   return res;
}

inline char* 
StringUtility::strstr( const char* haystack, const char* needle ) 
{
   return const_cast<char*>(::strstr( haystack, needle ));   
}

inline char*
StringUtility::strchr( const char *s, int c ) {
   return const_cast<char*>(::strchr( s, c ));
}

inline char*
StringUtility::strrchr( const char *s, int c ) {
   return const_cast<char*>(::strrchr( s, c ));
}

inline void 
StringUtility::trimEnd(char* s, const char* additional) 
{
   if ( s != NULL && s[ 0 ] != '\0' ) {
      uint32 tmp = strlen(s)-1;
      while ( ( isspace(s[tmp]) || strchr(additional, s[tmp]) != NULL ) && 
              (tmp > 0)) {
         s[tmp] = '\0';
         tmp--;
      }
   }
}


inline char* 
StringUtility::trimStart(const char* s, const char* additional) 
{
   if (s != NULL) {
      uint32 tmp = 0;
      while ( (s[tmp] != '\0') && 
              (isspace(s[tmp]) || strchr(additional, s[tmp]) != NULL) ) {
         tmp++;
      }
      return ( (char*) &(s[tmp]));
   } else {
      return (NULL);
   }
}


inline MC2String& 
StringUtility::trimStartInPlace(MC2String& str)
{
   static const MC2String remove(" \t\n\r\f\v");
   if( str.empty() ){
      return str;
   }

   MC2String::size_type first = str.find_first_not_of( remove );
   if( first != MC2String::npos ){
      str.erase( 0, first );
   } else if( remove.find( str[0] ) != MC2String::npos ){
      str.clear();
   }

   return str;
}

inline MC2String& 
StringUtility::trimEndInPlace(MC2String& str)
{
   //same characters as isspace()
   static const MC2String remove(" \t\n\r\f\v");
   if( str.empty() ){
      return str;
   }

   MC2String::size_type last = str.find_last_not_of( remove );

   if( last != MC2String::npos ){
      str.erase( last + 1, MC2String::npos );
   } else if( remove.find( str[0] ) != MC2String::npos ){
      str.clear();
   }
   return str;
}

inline MC2String& 
StringUtility::trimInPlace(MC2String& str)
{
   return trimEndInPlace( trimStartInPlace( str ) );
}

inline bool
StringUtility::isAlNum(uint32 ucs)
{
   if( ucs < 256 ) {
      byte ch = byte(ucs);
      if ( ch == byte('§') )
         return false;
      return ((ch >  '/') &&
              !((ch >= ':') &&
                (ch <= '@')) &&
              !((ch >= '[') &&
                (ch <= '`')) &&
              !((ch >= '{') &&
                (ch <= 126) &&
                (ch != 127)) &&
              !((ch >= 128) &&
                (ch <= 191)) &&
              ! (ch == ((byte) '¤') ));
   } else {
      TextConversionTables::convTableFindRes result =
         TextConversionTables::findInTable(
                                    TextConversionTables::c_removeStrange,
                                    ucs);
      if( result.resString != NULL )
         return true;
      else {
         return false;
      }
   }
}

inline bool
StringUtility::isSpace(uint32 ucs)
{
   if( ucs < 256 ) {
      return isspace(char(ucs));
   } else {
      return false;
   }
}

inline MC2String
StringUtility::upper(uint32 ucs) {
   TextConversionTables::convTableFindRes result =
      TextConversionTables::findInTable(TextConversionTables::c_upperTable,
                                        ucs);
   if( result.resString != NULL ) {
      return UTF8Util::utf8ToMc2( result.resString );
   } else {
      return UTF8Util::ucsToMc2(ucs);
   }
}

inline MC2String
StringUtility::lower(uint32 ucs) {   
   TextConversionTables::convTableFindRes result =
      TextConversionTables::findInTable(TextConversionTables::c_lowerTable,
                                        ucs);
   if( result.resString != NULL ){
      return UTF8Util::utf8ToMc2( result.resString );
   } else {
      return UTF8Util::ucsToMc2(ucs);
   }      
}

inline MC2String
StringUtility::copyUpper(mc2TextIterator it)
{
   MC2String retVal;
   while( *it != 0 ) {
      uint32 ucs = *it;
      retVal += upper(ucs);
      ++it;
   }
   return retVal;
}

inline MC2String
StringUtility::copyLower(mc2TextIterator it)
{
   MC2String retVal;
   while( *it != 0 ) {
      uint32 ucs = *it;
      retVal += lower(ucs);
      ++it;
   }
   return retVal;
}

inline char*
StringUtility::newStrDup(const char* s)
{
   char* theDupe = new char[strlen(s) + 1];
   strcpy( theDupe, s );
   return theDupe;
}


#endif // STRINGUTULITY_H 

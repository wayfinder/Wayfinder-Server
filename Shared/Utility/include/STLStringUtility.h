/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STLSTRINGUTILITY_H
#define STLSTRINGUTILITY_H

#include "config.h"
#include "MC2String.h"


/**
  *   Some useful STL string functions.
  *
  */
class STLStringUtility {
public:
   
   /**
    * basename - strip directory and suffix from filenames
    * 
    * @param dirSep  The directory seperator. Default '/'.
    * @return name with any leading directory components removed.
    *          If specified, also remove a trailing SUFFIX.
    */
   static MC2String basename( const MC2String& name,
                              const MC2String& suffix = "",
                              char dirSep = '/' );

   /**
    *   Adds a path component to the src_path, not adding
    *   the slash if it is already present.
    */
   static MC2String addPathComponent( const MC2String& path,
                                      const MC2String& toadd,
                                      const char* dirSep = "/" );
   
   
   /**
    * Strips non-directory part from filename.
    * The trailing '/' (dirSep) is not included in the result.
    * If no directory part is found, . is returned.
    * 
    * @param dirSep  The directory seperator. Default '/'.
    * @return  The non-directory part of the filename.
    */
   static MC2String dirname( const MC2String& str, char dirSep = '/' );

   /**
    *   Returns the dos-type file extension in lower case for a string
    *   (if tolower is not false).
    *   @param str E.g. "filename.TXT"
    *   @param tolower If true convert the extension to lower case.
    *   @return "txt" in the above case if tolower = true;
    */
   static MC2String fileExtension( const MC2String& str, bool tolower = false);


   /**
    * Breaks a line at blanks or if too wide word the word is broken
    * after lineWidth chars.
    *
    * @param line The line to break.
    * @param lineWidth The maximum line width.
    * @param breakChars The chars to break line at.
    * @param eolStr The char(s) that is end of line.
    * @return A broken string :).
    */
   static MC2String breakLine( const MC2String line, uint32 lineWidth,
                               const MC2String breakChars = 
                               MC2String( " \t-,&.:;+-=()" ),
                               const MC2String eolStr = 
                               MC2String( "\n" ) );


   /**
    * Appends a uint32, as decimal numbers, to a string.
    *
    * @param ui The uint32 to append.
    * @param s The string to append to.
    * @retrun The string.
    */
   static MC2String& uint2str( uint32 ui, MC2String& s );


   /**
    * Make a string from a uint32, as decimal numbers.
    *
    * @param ui The uint32.
    * @retrun The string.
    */
   static MC2String uint2str( uint32 ui );


   /**
    * Appends a int32, as decimal numbers, to a string.
    *
    * @param ui The int32 to append.
    * @param s The string to append to.
    * @retrun The string.
    */
   static MC2String& int2str( int32 ui, MC2String& s );


   /**
    * Make a string from a int32, as decimal numbers.
    *
    * @param i The int32.
    * @retrun The string.
    */
   static MC2String int2str( int32 i );


   /**
    * Appends a uint64, as decimal numbers, to a string.
    *
    * @param ui The int32 to append.
    * @param s The string to append to.
    * @retrun The string.
    */
   static MC2String& int2str( uint64 ui, MC2String& s );


   /**
    * Appends a uint32, as hexa decimal numbers, to a string.
    *
    * @param ui The uint32 to append.
    * @param s The string to append to.
    * @retrun The string.
    */
   static MC2String& uint2strAsHex(  uint32 ui, MC2String& s );


   /**
    * Appends a float64 to a string.
    *
    * @param f The float64 to append.
    * @param s The string to append to.
    * @retrun The string.
    */
   static MC2String& float2Str(  float64 f, MC2String& s );

   /** 
    *  Converts initial part of the string str to a long integer value 
    *  according to the given base. Calls strtol or strtoul of the stdlib.h 
    *  internally.
    *
    * Note that this function suffers from the same lack of error
    * indication as atoi.
    *
    *  @param str  The string to convert to a long. May include leading
    *              white space.
    *  @param base The base to use, must be a value between 2 and 36 or the
    *              special value 0. When base 0 is used leading "0x" in the
    *              given string leads to the use of base 16, leading 0 
    *              leads to base 8. Otherwise base 10.
    *
    *  @return A long given by the first interpretable characters in the
    *          string str. Unsigned for strtoul and signed for strtol.
    */
   //@{
   static uint32 strtol ( const MC2String& str, int base = 0 );
   static uint32 strtoul ( const MC2String& str, int base = 0 );
   static uint64 strtoll ( const MC2String& str, int base = 0 );
   static uint64 strtoull ( const MC2String& str, int base = 0 );
   //@}

   /** 
    *  Converts initial part of the string str to a float64 value. 
    *  Calls strtod  of the stdlib.h internally.
    *
    * Note that this function suffers from the same lack of error
    * indication as atoi.
    *
    *  @param str  The string to convert to a float64. May include leading
    *              white space.
    *
    *  @return A float64 given by the first interpretable characters in the
    *          string str.
    */
   static float64 strtod ( const MC2String& str );

   /** 
    *  Converts initial part of the string str to a float64 value. 
    *  Checks if the string is numerical.
    *  Calls strtod of the stdlib.h internally.
    *
    *  @param str  The string to convert to a float64. May include leading
    *              white space.
    *  @param val  The float64 value we want.
    *
    *  @return True if the string is numerical, false otherwise.
    */
   static bool strtod ( const MC2String& str, float64& val );
   
   /**
    * Works essentially like strtoul above but with error handling.
    * 
    * @param str  The string to convert to an uint32. May include leading
    *             white space.
    * @param val  The uint32 we want.
    * 
    * @return True of the string is numerical.
    */
   static bool strtoul( const MC2String& str, uint32& val, int base = 0 );

   /**
    * Split a string by string.
    *
    * @param separator The string that separates str.
    * @param str The string to separate.
    * @return Array of strings.
    */
   static vector<MC2String> explode( const MC2String& separator, 
                                     const MC2String& str );
   
   /**
    * Joins the strings in a vector into one string.
    * For instance, if the vector contains "Hello" and "World!", 
    * join( strings ) returns "Hello World!".
    * 
    * @param strings             The strings to join
    * @param removeEmptyStrings  Should empty strings be ignored?
    * @param separator           The separator with which to separate the 
    *                            strings
    */
   static MC2String join( const vector<MC2String>& strings, 
                          bool removeEmptyStrings = false, 
                          const char* separator = " " );

   /**
    * Replaces a string with another one
    * @param str the string 
    * @param what the thing to replace in str
    * @param withThis the string thats going to be inserted
    * @return true if the string was replaced
    */
   static bool replaceString( MC2String& str, 
                              const MC2String& what,
                              const MC2String& withThis );

   /**
    * Replaces all occurances of a string with another one
    *
    * @param str the string 
    * @param what the thing to replace in str
    * @param withThis the string thats going to be inserted
    * @return The number of strings that was replaced.
    */
   static uint32 replaceAllStrings( MC2String& str, 
                                    const MC2String& what,
                                    const MC2String& withThis );

   /**
    *   Calls StringUtility::splitSeconds but returns an MC2String.
    */
   static MC2String splitSeconds(uint32 nbrSec);

   /**
    *    Struct used for comparing strings in STL.
    */
   struct ltstr {
      bool operator()( const char* s1, const char* s2 ) const
      {
         return strcmp(s1, s2) < 0;
      }
   };   
private:

   /**
    *   Removes any leading directory components from name.
    */
   static MC2String basename_no_suffix( const MC2String& name,
                                        char dirSep = '/' );
   
};


#endif

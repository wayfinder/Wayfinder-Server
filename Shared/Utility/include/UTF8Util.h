/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTF8UTIL_H
#define UTF8UTIL_H

#include "config.h"

#include "MC2String.h"
#include "MC2SimpleString.h"

class UTF8Util {
  public:

   /**
    *    Returns the number of characters as opposed to the number
    *    of bytes in the string.
    */
   static int nbrChars( const char* str );
   
   /**
    *    Returns the number of characters as opposed to the number
    *    of bytes in the string.
    */
   static int nbrChars( const MC2SimpleString& str );

   /**
    *    Returns the number of characters as opposed to the number
    *    of bytes in the string.
    */
   static int nbrChars( const MC2String& str );

   /**
    *    Writes utf-8 encoded string into res from str ISO encoding.
    *    @param str is the iso encoded string
    *    @param outlen The length of the encoded string excluding zero
    *                  termination. Optional.
    *    @param inlen  The length of the input string. Optional.
    *    @return New char* encoded in UTF8. Will be twice as long
    *            as the original.
    */
   static char* isoToUtf8( const char* str,
                           int* outlen = NULL,
                           int* inlen  = NULL );
   
   /**
    *   Writes utf-8 encoded string into already allocated string.
    *   The destination must be twice as large as the strlen of src
    *   plus 1.
    *   @param destMustBeTwiceAsBigAsSrcPlusZeroTerm Destination.
    *   @param src Source.
    *   @param outlen If not NULL the number of characters written will
    *                 be placed here.
    *   @param inlen  If not NULL this number will be used instead of running
    *                 strlen.
    *   @return Ptr to destMustBeTwiceAsBigAsSrcPlusZeroTerm.
    */
   static char* isoToUtf8( char* destMustBeTwiceAsBigAsSrcPlusZeroTerm,
                           const char* src,
                           int* outlen = NULL,
                           int* inlen  = NULL );
                           
   
   /**
    *   Converts a string from ISO-8859-1 to UTF8
    *   @param str The ISO-8859-1 string to be converted.
    *   @return The UTF8 string.
    */
   static MC2String isoToUtf8( const MC2String& str );

   /**
    *   Converts a string from UTF8 to ISO-8859-1.
    *   @param str The UTF8 string to be converted.
    *   @return The ISO-8859-1 string.
    */
   static MC2String utf8ToIso( const MC2String& str );

   /**
    *   Converts from utf8 to ucs, one character.
    */
   static uint32 utf8ToUcs( const MC2String& str) ;
   
   /**
    *   Converts one ucs-character to UTF8.
    *   @param ucs The vector with ucs-integers to be converted.
    *   @return The UTF8 string.
    */
   static MC2String ucsToUtf8( uint32 ucs );

   /**
    *   Converts a vector with ucs-integers to ISO-8859-1.
    *   @param ucs The vector with ucs-integers to be converted.
    *   @return The ISO-88591-1 string.
    */
   static MC2String ucsToIso( const uint32 ucs);

   /**
    *   Converts a ISO-8859-1 string to a vector with ucs-integers.
    *   @param str The ISO-8859-1 string to be converted.
    *   @return The vector with ucs-integers.
    */
   static uint32 isoToUcs( const MC2String& str);

   /**
    *   Converts a mc2-string to a UTF8.
    *   @param str The mc2-string to be converted.
    *   @return The UTF8 string.
    */
#ifdef MC2_UTF8
   inline static const MC2String& mc2ToUtf8( const MC2String& str );
#else
   inline static MC2String mc2ToUtf8( const MC2String& str );
#endif

   /**
    *   Converts a UTF8 string to mc2.
    *   @param str The UTF8 string to be converted.
    *   @return The mc2-string.
    */
#ifdef MC2_UTF8
   inline static const MC2String& utf8ToMc2( const MC2String& str );
#else
   inline static MC2String utf8ToMc2( const MC2String& str );
#endif

   /**
    *   Converts a mc2-string to ISO-8859-1.
    *   @param str The mc2-string to be converted.
    *   @return The ISO-8859-1 string.
    */
   static MC2String mc2ToIso( const MC2String& str );

   /**
    *   Converts a ISO-8859-1 string to mc2.
    *   @param str The ISO-8859-1 string to be converted.
    *   @return The mc2-string.
    */
   static MC2String isoToMc2( const MC2String& str );

   /**
    *   Converts a mc2-string to a vector with ucs-integers.
    *   @param str The mc2-string to be converted.
    *   @return The vector with ucs-integers.
    */
   static uint32 mc2ToUcs( const MC2String& str );

   /**
    *   Converts a vector with ucs-integers to mc2.
    *   @param ucs The vector with ucs-integers to be converted.
    *   @return The mc2-string.
    */
   static inline MC2String ucsToMc2( uint32 ucs );

   
   /**
    *  Converts a unicode character to a utf-8 string.
    *  The utf-8 string must be at least 7 characters 
    *  long.
    *  @param unicode The unicode character.
    *  @param utf8    Destination string.
    *  @return Number of bytes added to <code>utf8</code> excl zero term.
    */
   static int unicodeToUtf8(uint32 unicode,
                            char* utf8);

   /**
    *   Converts a string that can be utf-8 or latin-1 into
    *   utf-8. Does this by assuming that all invalid combinations
    *   are latin-1.
    *
    *   @param reportConversions Set this to true to get a print of each
    *                            invalid UTF-8 sign.
    */
   static MC2String cleanUtf8( const MC2String& instr, 
                               bool reportConversions=false );

   /**
    *   Returns true if the string is valid UTF-8.
    *
    *   @param reportConversions Set this to true to get a print of each
    *                            invalid UTF-8 sign.
    */
   static bool isValidUTF8( const char* str,
                            bool reportConversions=false );

   /**
    *   Returns true if the string is valid UTF-8.
    */
   static bool isValidUTF8( const MC2String& str );
   
};

inline MC2String
UTF8Util::ucsToUtf8( uint32 ucs )
{
   char tmp[8];
   unicodeToUtf8( ucs, tmp );
   return tmp;
}

inline MC2String
UTF8Util::ucsToMc2( uint32 ucs )
{
#ifdef MC2_UTF8
   return UTF8Util::ucsToUtf8( ucs );
#else
   return UTF8Util::ucsToIso( ucs );
#endif
}

#ifdef MC2_UTF8
inline const MC2String&
#else
inline MC2String
#endif
UTF8Util::utf8ToMc2( const MC2String& str )
{
#ifdef MC2_UTF8
   return str;
#else
   return UTF8Util::utf8ToIso( str );
#endif
}


#ifdef MC2_UTF8
inline const MC2String&
#else
inline MC2String
#endif
UTF8Util::mc2ToUtf8( const MC2String& str )
{
#ifdef MC2_UTF8
   return str;
#else
   int inlen = str.length();
   int outlen = 0;
   // The MC2SimpleStringNoCopy will delete [] the convStr when we return
   MC2SimpleStringNoCopy convStr( isoToUtf8( str.c_str(), &outlen, &inlen ) );
   return convStr.c_str();
#endif
}

#endif

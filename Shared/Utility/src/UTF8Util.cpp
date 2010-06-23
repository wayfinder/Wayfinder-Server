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

#include "UTF8Util.h"
#include "CharEncoding.h"
#include "TextIterator.h"

int
UTF8Util::nbrChars( const char* str )
{
   int len = 0;
   for( mc2TextIterator it = mc2TextIterator(str);
        *it != 0;
        ++it ) {
      ++len;
   }
   return len;
}

int
UTF8Util::nbrChars( const MC2String& str )
{
   int len = 0;
   for( mc2TextIterator it = mc2TextIterator(str);
        *it != 0;
        ++it ) {
      ++len;
   }
   return len;
}

int
UTF8Util::nbrChars( const MC2SimpleString& str )
{
   int len = 0;
   for( mc2TextIterator it = mc2TextIterator(str.c_str());
        *it != 0;
        ++it ) {
      ++len;
   }
   return len;
}

char*
UTF8Util::isoToUtf8( const char* str,
                     int* outlen,
                     int* inlen )
{
   int len = inlen ? *inlen : strlen(str);
   
   char* res = new char[ (len << 1) + 1 ];
   return isoToUtf8( res, str, outlen, inlen );
}

char*
UTF8Util::isoToUtf8( char* destMustBeTwiceAsBigAsSrcPlusZeroTerm,
                     const char* src,
                     int* outlen,
                     int* inlen )
{
   int len = inlen ? *inlen : strlen(src);
   
   char* res = destMustBeTwiceAsBigAsSrcPlusZeroTerm;
   int inpos = 0;
   
   int outpos = 0;
   
   while( inpos < len ) {
      uint8 ch = uint8( src[ inpos ] );
      if ( (ch & 0x80) == 0 ) { // Single byte, ch < 0x80
         res[outpos++] = ch;
      } else { // Multibyte utf-8 char
         // Since input is ISO-8859-1 ch is always < 0x800
         // So only two bytes are needed
         res[outpos++] = char( ((ch >> 6) | 0xC0) );
         res[outpos++] = char( ((ch & 0x3F) | 0x80) );
      }
      ++inpos;
   }

   // Zero terminate
   res[outpos] = '\0';
   
   if ( outlen ) {
      *outlen = outpos;
   }
   return res;
}

MC2String
UTF8Util::isoToUtf8( const MC2String& str )
{
   int inlen = str.size();
   int outlen;
   char* resultTmp = UTF8Util::isoToUtf8(str.c_str(), &outlen, &inlen);
   MC2String result = resultTmp;
   delete [] resultTmp;

   return result;
}

MC2String
UTF8Util::utf8ToIso( const MC2String& str )
{
   MC2String result;
   for(utf8TextIterator it(str.c_str()); *it != 0; ++it) {
      if( *it < 256 ) {
         result += *it;
      } else {
         uint32 tmpRes = CharEncoding::unicodeToLatin1(*it);
         if( tmpRes != 0 ) {
            result += tmpRes;
         }
      }
   }
   return result;
}   

uint32
UTF8Util::utf8ToUcs(const MC2String& str )
{
   mc2TextIterator it(str.c_str());
   return *it;
}

int
UTF8Util::unicodeToUtf8(uint32 unicode,
                        char* utf8)
{
   // FIXME: Remove the redundant comparisions and
   //        use shift/and/or instead of + * % /
   int idx = 0;
   if( (unicode >= 0x00000000 ) && (unicode <= 0x0000007f) ) {
      utf8[idx++] = unicode;
   } else if( (unicode >= 0x00000080) && (unicode <= 0x000007ff) ) {
      // First char
      unsigned char firstChar = 192 + (unicode / 64);
      utf8[idx++] = ( (char ) firstChar);
      // Second char
      unsigned char secondChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char ) secondChar);
   } else if( (unicode >= 0x00000800) && (unicode <= 0x0000ffff) ) {
      // First char
      unsigned char firstChar = 224 + (unicode / 4096);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) thirdChar);
   } else if( (unicode >= 0x00010000) && (unicode <= 0x001fffff) ) {
      // First char
      unsigned char firstChar = 240 + (unicode / 262144);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 4096) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) thirdChar);
      // Fourth char
      unsigned char fourthChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) fourthChar);
   } else if( (unicode >= 0x00200000) && (unicode <= 0x03ffffff) ) {
      // First char
      unsigned char firstChar = 248 + (unicode / 16777216);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 262144) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 28 + ((unicode / 4096) % 64);
      utf8[idx++] = ( (char) thirdChar);
      // Fourth char
      unsigned char fourthChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) fourthChar);
      // Fifth char
      unsigned char fifthChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) fifthChar);
   } else if( (unicode >= 0x04000000) && (unicode <= 0x7fffffff) ) {
      // First char
      unsigned char firstChar = 252 + (unicode / 1073741824);
      utf8[idx++] = ( (char) firstChar);
      // Second char
      unsigned char secondChar = 128 + ((unicode / 16777216) % 64);
      utf8[idx++] = ( (char) secondChar);
      // Third char
      unsigned char thirdChar = 128 + ((unicode / 262144) % 64);
      utf8[idx++] = ( (char) thirdChar);
      // Fourth char
      unsigned char fourthChar = 128 + ((unicode / 4096) % 64);
      utf8[idx++] = ( (char) fourthChar);
      // Fifth char
      unsigned char fifthChar = 128 + ((unicode / 64) % 64);
      utf8[idx++] = ( (char) fifthChar);
      // Sixth char
      unsigned char sixthChar = 128 + (unicode % 64);
      utf8[idx++] = ( (char) sixthChar);
   }

   // Zero terminate.
   utf8[idx] = '\0';
   return idx;
}
      
   
MC2String
UTF8Util::ucsToIso( const uint32 ucs)
{
   char result[2];
   result[1] = 0;
   if( ucs < 256 ) {
      result[0] = (char)ucs;
   } else {
      uint32 tmpRes = CharEncoding::unicodeToLatin1(ucs);
      if( tmpRes != 0 ) {
         result[0] = tmpRes;
      } else {
         result[0] = ' ';
      }
   }
   
   return result;
}

uint32
UTF8Util::isoToUcs( const MC2String& str)
{
   isoTextIterator it(str.c_str());
   return *it;
}

MC2String
UTF8Util::mc2ToIso( const MC2String& str )
{
#ifdef MC2_UTF8
   return UTF8Util::utf8ToIso( str );
#else
   return str;
#endif
}

uint32
UTF8Util::mc2ToUcs( const MC2String& str )
{
#ifdef MC2_UTF8
   return UTF8Util::utf8ToUcs( str );
#else
   return UTF8Util::isoToUcs( str );
#endif
}

MC2String
UTF8Util::isoToMc2( const MC2String& str )
{
#ifdef MC2_UTF8

   int inpos = 0;
   int len = str.length();
   
   MC2String res;
   // Will max be twice as long 
   res.reserve( 2 * len + 1 );
   while( inpos < len ) {
      uint8 ch = uint8( str[ inpos ] );
      if ( (ch & 0x80) == 0 ) { // Single byte, ch < 0x80
         res += ch;
      } else { // Multibyte utf-8 char
         // Since input is ISO-8859-1 ch is always < 0x800
         // So only two bytes are needed
         res += char( ((ch >> 6) | 0xC0) );
         res += char( ((ch & 0x3F) | 0x80) );
      }
      ++inpos;
   }
   
   return res;
#else
   return str;
#endif 
}


MC2String
UTF8Util::cleanUtf8( const MC2String& instr, bool reportConversions ) 
{
   MC2String res;
   int count = 0;
   for ( uint32 pos = 0; pos < instr.length(); pos += count ) {
      count = utf8TextIterator::nbrBytesForUtf8Char( instr.c_str(), pos );

      if ( pos + count > instr.length() ) {
         // Illegal. Assume latin-1
         char tmp[8];
         unicodeToUtf8( (byte)instr[pos], tmp );
         res += tmp;
         count = 1;
         continue;
      }
      
      // Check for valid utf-8 in other ways
      bool valid = true;
      switch ( count ) {
         case 1:
            // High bit must not be set. Ascii...
            valid = ( instr[pos] & 0x80 ) == 0;
            break;
         case 2:
            valid = ( ( instr[pos] & 0xe0 ) == 0xc0 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 );
            break;
         case 3:
            valid = ( ( instr[pos] & 0xf0 ) == 0xe0 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 );
            break;
         case 4:
            valid = ( ( instr[pos] & 0xf8 ) == 0xf0 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+3] & 0xc0 ) == 0x80 );
            break;
         case 5:
            valid = ( ( instr[pos] & 0xfc ) == 0xf8 ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+3] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+4] & 0xc0 ) == 0x80 );
            break;
         case 6:
            valid = ( ( instr[pos] & 0xfe ) == 0xfc ) &&
               ( ( instr[pos+1] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+2] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+3] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+4] & 0xc0 ) == 0x80 ) &&
               ( ( instr[pos+5] & 0xc0 ) == 0x80 );
            break;            
         default:
            valid = false;
      }

      if ( valid ) {
         res += instr.substr( pos, count );
      } else {
         char tmp[8];
         unicodeToUtf8( (byte)instr[pos], tmp );
         if ( reportConversions ){
            uint8 ord = (byte)instr[pos];
            mc2dbg << "Str[" << pos << "]: " << (byte)instr[pos] << "(0x" 
                   << hex << (uint32)ord << dec
                   << ")" << endl;
         }
         res += tmp;
         count = 1;
      }
   }
   return res;
}

bool
UTF8Util::isValidUTF8( const char* str, bool reportConversions )
{
   // FIXME: Optimize by extracting parts of cleanUTF8 to
   // own method.
   if ( str == NULL ) {
      return true;
   }
   MC2String tmp = cleanUtf8( str, 
                              reportConversions
                              );
   return tmp == str;
}

bool
UTF8Util::isValidUTF8( const MC2String& str )
{
   MC2String tmp = cleanUtf8( str );
   return tmp == str;
}

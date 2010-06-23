/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpParserThreadUtility.h"
#include "UTF8Util.h"


void
HttpParserThreadUtility::isoToUtf8( const MC2String& str, MC2String& res )
{
   uint32 pos = 0;
   uint32 ch = 0;

   while( pos < str.size() ) {
      ch = uint8( str[ pos ] );
      
      if ( (ch & 0x80) == 0 ) { // Single byte, ch < 0x80
         res += ch;
      } else { // Multibyte utf-8 char
         // Since input is ISO-8859-1 ch is always < 0x800
         // So only two bytes are needed
         res += char( ((ch >> 6) | 0xC0) );
         res += char( ((ch & 0x3F) | 0x80) );
      }
      pos++;
   }
}


void
HttpParserThreadUtility::charsetToMC2( MC2String& str, 
                                       const char* charset ) 
{
   MC2String res;

   // Less reallocations of the string
   res.reserve( str.size() );
   
   if ( strncmp( charset, "utf-8", 5 ) == 0 ) {
      res = UTF8Util::utf8ToMc2( str );         
   } else { // ISO and default
      res = UTF8Util::cleanUtf8( str );
   }

   str = res;
}


void
HttpParserThreadUtility::utf8ToIso( const MC2String& str, MC2String& res )
{
   res = UTF8Util::utf8ToIso( str );
}


MC2String*
HttpParserThreadUtility::URLEncode(const MC2String& str) {
   uint32 pos = 0;
   MC2String* res = new MC2String();
   char cstr[44];
   byte ch;
   
   res->reserve( str.size() );

   while (pos < str.size()) {
      ch = str[pos];
      if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
           (ch >= '0' && ch <= '9') || (ch >= '-' && ch <= '.') )
      {
         // Ordinary character
         res->append( 1, static_cast<char>(ch) );
      } /* //Some browsers (new R380) doesn't think '+' is url-encoded 
           // so it encodes it again! resulting in a '+' isof. ' ' 
        else if ( ch == 32 ) { // ASCII 32 == ' '
         res->append( 1, '+' );
         } */ else {
         sprintf(cstr, "%%%2X", ch);
         res->append( cstr );
      }
      pos++;
   }
   
   return res;
}


MC2String* 
HttpParserThreadUtility::unURLEncode(const MC2String& str) {
   uint32 pos = 0;
   MC2String* res = new MC2String();
   uint32 result;

   while (pos < str.size()) {
      if (str[pos] == '%') {
         switch (str[pos + 1]) {
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
            default: result = str[pos + 1] - 48;
         }
         result = result*16;
         switch (str[pos + 2]) { 
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
            default: result = result + str[pos + 2] - 48;
         }
         (*res) += (char)result; // Works 0-255.
         // '%' and first number, last number handled normaly at end
         pos += 2;
      }
      else if (str[pos] == '+') {
         (*res) += ' ';
      } else {
         (*res) += str[pos];
      }
      pos++;
   }
   return res;
}

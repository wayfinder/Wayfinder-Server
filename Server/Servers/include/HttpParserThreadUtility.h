/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPPARSERTHREADUTILITY_H
#define HTTPPARSERTHREADUTILITY_H

#include "config.h"
#include "MC2String.h"


/**
 * Some generic functions used by HttpParserThread and possibly others.
 *
 */
class HttpParserThreadUtility {
   public:
      /**
       * Writes utf-8 encoded string into res from str ISO encoding.
       * @param str is the iso encoded string
       * @param target is the string to append the utf-8 chars to.
       */
      static void isoToUtf8( const MC2String& str, MC2String& res );


      /**
       * Transforms str to MC2.
       * Supports UTF-8. ISO-8859-1, ASCII first 256 chars in UNINCODE is 
       * the same so they are all ok too.
       */
      static void charsetToMC2( MC2String& str, const char* charset );

   
      /**
       * Parses the utf-8 encoded string str into target with ISO encoded.
       * @param str is the utf-8 encoded string
       * @param target is the string to append the iso chars to.
       */
      static void utf8ToIso( const MC2String& str, MC2String& res );


      /** 
       * URL-echapes a string and returns a new string
       * @param str is the string to URL-encode.
       */
      static MC2String* URLEncode(const MC2String& str);
      
      
      /** 
       * Un URL-echapes a string and returns a new string
       * @param str is the URL-encoded string.
       */
      static MC2String* unURLEncode(const MC2String& str);


   private:
      /**
       * Private constructor to avoid usage.
       */
      HttpParserThreadUtility();
};


#endif // HTTPPARSERTHREADUTILITY_H


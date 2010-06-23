/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StringTalkerUtility.h"
#include "STLStringUtility.h"

namespace TalkerUtility {
MC2String eraseAllBetween( const MC2String& inStr,
                           char startChar, char endChar ) {
   MC2String str = inStr;
   MC2String::size_type startPos = 0;
   while ( (startPos = str.find_first_of( startChar, startPos ) ) != MC2String::npos ) {
      MC2String::size_type endPos = str.find_first_of( endChar, startPos );
      if ( endPos != MC2String::npos ) {
         // + 1 to remove endChar too
         str.erase( startPos, endPos - startPos + 1 );
      } else {
         break;
      }
   }

   return str;
}

MC2String clearHtmlTags( const MC2String& str ) {
   MC2String ret = eraseAllBetween( str, '<', '>' );
   // replace any &amp; with normal & so xerces can replace them correctly
   STLStringUtility::replaceAllStrings( ret, "&amp;", "&" );
   return ret;
}

}

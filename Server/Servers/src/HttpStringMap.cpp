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

#include "HttpStringMap.h"
#include "LangTypes.h"

#include <errno.h>
#include <stdlib.h>

const MC2String&
stringMap::getStrParamThrw( const MC2String& name ) const throw ( MC2String )
{
   const_iterator it = find( name );
   if ( it == end() ) {
      throw "Unfound param " + name;
   } else if ( it->second == NULL ) {
      throw "it->second == NULL for " + name;
   }
   return *(it->second);
}

MC2String
stringMap::getStrParam( const MC2String& name,
                        const MC2String& defval ) const
{
   try {
      return getStrParamThrw( name );
   } catch ( MC2String& ) {      
   }
   return defval;
}

long int
stringMap::getIntParamThrw( const MC2String& paramName ) const
   throw ( MC2String )
{
   const MC2String& tmp = getStrParamThrw( paramName );
   errno = 0;
   long int tmp_int = strtol( tmp.c_str(), NULL, 0 );
   if ( errno ) {
      throw "Cannot convert " + paramName + "=" + tmp + " to integer";
   }
   return tmp_int;
}

long int
stringMap::getIntParam( const MC2String& paramName,
                        long int defval ) const
{
   try {
      return getIntParamThrw( paramName );
   } catch ( MC2String& ) {
   }
   return defval;
}

LangType
stringMap::getISO639Thrw( const MC2String& name ) const throw (MC2String)
{
   const MC2String& val = getStrParamThrw( name );
   LangTypes::language_t lang = LangTypes::getISO639AsLanguage( val.c_str() );
   if ( lang == LangTypes::invalidLanguage ) {
      lang = LangTypes::getISO639AndDialectAsLanguage( val.c_str() );
   }
   if ( lang == LangTypes::invalidLanguage ) {
      throw MC2String( "Invalid language in param ") + name + "=" + val;
   }
   return lang;
}

LangType
stringMap::getISO639( const MC2String& name, const LangType& def ) const
{
   try {
      return getISO639Thrw( name );
   } catch ( MC2String& ) {
   }
   return def;
}

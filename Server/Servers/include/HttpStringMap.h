/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPSTRINGMAP_H
#define HTTPSTRINGMAP_H

#include "config.h"

#include "MC2String.h"

#include <map>

class LangType;

/**
 *   Map for passing http-params.
 */
class stringMap : public map<MC2String, MC2String *> {
public:
   /**
    *   Returns a string param or throws if not found.
    *   @warning Throws
    */
   const MC2String&
      getStrParamThrw( const MC2String& name ) const throw (MC2String);

   /**
    *   Returns a string param or the default value if not found.
    */
   MC2String getStrParam( const MC2String& name,
                          const MC2String& defval ) const;

   /**
    *    Returns a long int parameter or throws an error if bad int.
    *    @warning Throws
    */
   long int getIntParamThrw( const MC2String& name ) const throw (MC2String);

   /**
    *    Returns a long int parameter or returns the default val
    *    if not found or invalid integer.
    */
   long int getIntParam( const MC2String& name, long int defVal ) const;

   /**
    *    Returns a language param or throws if it is unkown.
    */
   LangType getISO639Thrw( const MC2String& name ) const throw (MC2String);

   /**
    *    Returns a language with a default value if error.
    */
   LangType getISO639( const MC2String& name, const LangType& def ) const;
   
};

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef URL_PARAMS_H
#define URL_PARAMS_H

#include "config.h"
#include "MC2String.h"

#include <sstream>

/**
 * A class for adding url parameters.
 * \code
 * URLParams params;
 * params.add( "uin", 10 );
 * params.add( "user_name", "Chuck Norris" );
 * MC2String urlParams = params;
 * // urlParams will now be: "?uin=10&user_name=Chuck%20Norris"
 * \endcode
 */
class URLParams {
public:

   URLParams();

   ~URLParams() {}

   /**
    * For adding a parameter to a url feed.
    * @param param The parameter to add.
    * @param value The value of the parameter.
    */
   template < typename T >
   void add( const char* param, const T& value ) {
      if ( m_addedOne ) {
         m_urlParams << "&";
      } else {
         m_urlParams << "?";
         m_addedOne = true;
      }
      m_urlParams << param << "=" << value;
   }
   /// Special case for strings, must be url encoded
   void add( const char* param, const char* value );

   /// Convert to a MC2String.
   operator MC2String() { return m_urlParams.str(); }

   /// Reset the params.
   void reset() {
      m_addedOne = false;
      m_urlParams.str( "" );
   }

private:
   /// To keep track if a param is added.
   bool m_addedOne;

   /// The string stream we add params to.
   stringstream m_urlParams;

};

/// Specialization of MC2String, because we need to URL encode it.
template <>
void URLParams::add< MC2String > ( const char* param, const MC2String& value );

#endif // URL_PARAMS_H

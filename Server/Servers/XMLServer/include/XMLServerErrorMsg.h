/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLSERVERERRORMSG_H
#define XMLSERVERERRORMSG_H

#include "config.h"

#include "MC2String.h"
#include <stdexcept>

class StringCode;

class XMLServerErrorMsg: public std::exception {
public:
   /// Creates a new XMLServerErrorMsg
   template<class STRING1, class STRING2>
   XMLServerErrorMsg( const STRING1& code,
                      const STRING2& msg ) throw() 
         : m_code( code ),
           m_msg( msg ),
           m_uri( "" ) {}
   template<class STRING1, class STRING2, class STRING3>
   XMLServerErrorMsg( const STRING1& code,
                      const STRING2& msg,
                      const STRING3& uri ) throw() 
         : m_code( code ),
           m_msg( msg ),
           m_uri( uri ) {}
   /**
    *   Creates default error message for the sent in string code.
    */
   XMLServerErrorMsg( StringCode error ) throw();
   ~XMLServerErrorMsg() throw() { }
   const MC2String& getCode() const {
      return m_code;
   }
   const MC2String& getMsg() const {
      return m_msg;
   }
   const MC2String& getURI() const {
      return m_uri;
   }
   const char* what() const throw() {
      return m_msg.c_str();
   }
private:
   MC2String m_code;
   MC2String m_msg;
   MC2String m_uri;
};


#endif

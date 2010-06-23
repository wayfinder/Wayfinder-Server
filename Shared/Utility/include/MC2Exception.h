/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2EXCEPTION_H
#define MC2EXCEPTION_H

#include "MC2String.h"
#include <stdexcept>
#include "SysUtility.h"
#include <vector>

/// Common exception class.
class MC2Exception : public std::exception {
public:

   /// @param what the exception message
   MC2Exception( const MC2String& what ):
      m_what( "[Exc]: " + what ) {
      SysUtility::getBacktrace( m_bt );
   }
   
   /// Constructor with prefix and exception message. 
   MC2Exception( const MC2String& prefix, const MC2String& what ):
      m_what( "[" + prefix + "]: " + what ) {
      SysUtility::getBacktrace( m_bt );
   }
   
   ~MC2Exception() throw() {
   }

   /// @return string to exception message
   const char *what() const throw() { return m_what.c_str(); }

   /// For debugprint
   friend ostream& operator<<( ostream& o, const MC2Exception& e ) {
      o << e.m_what << endl;
      for ( uint32 i = 0; i < e.m_bt.size(); ++i ) {
         o << "[bt][" << i << "]: " << e.m_bt[ i ] << endl;
      }
      return o;
   }
   
private:
   
   MC2String m_what; //< contains the exception message

   /// Contains back trace.
   vector<MC2String> m_bt;
};


#endif

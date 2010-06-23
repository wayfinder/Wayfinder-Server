/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2String.h"
#if GCC_VERSION < 30000

const uint32 MC2String::npos = MAX_UINT32;


istream&
operator >> ( istream& is, MC2String& s ) {
   int w = is.width( 0 );
   if ( is.ipfx0() ) {
      register streambuf *sb = is.rdbuf();
      s.resize( 0 );
      s.push_back( '\0' );
      while ( true ) {
         int ch = sb->sbumpc ();
         if ( ch == EOF ) {
            is.setstate( ios::eofbit );
            break;
         } else if ( isspace( ch ) ) {
            sb->sungetc ();
            break;
         }
         s += static_cast< char >( ch );
         if ( --w ==  1 ) {
            break;
         }
      }
   }

   is.isfx();
   if ( s.length () == 0 ) {
      is.setstate( ios::failbit );
   }
   
   return is;
}


istream&
getline ( istream& is, MC2String& s, char delim ) {
   if ( is.ipfx1 () ) {
      _IO_size_t __count = 0;
      streambuf *sb = is.rdbuf();
      s.resize( 0 );
      s.push_back( '\0' );
      
      while ( true ) {
         int ch = sb->sbumpc ();
         if ( ch == EOF ) {
            is.setstate ( __count == 0
                          ? (ios::failbit|ios::eofbit)
                          : ios::eofbit );
            break;
         }

         ++__count;

         if ( ch == delim ) {
            break;
         }

         s += static_cast< char >( ch );
         
         if ( s.length () == s.npos - 1 ) {
            is.setstate (ios::failbit);
            break;
         }
      }
   }

   // We need to be friends with istream to do this.
   // is._gcount = __count;
   is.isfx ();

   return is;   
}
#endif

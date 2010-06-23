/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FIXEDSIZESTRING_H
#define FIXEDSIZESTRING_H

#include "ScopedArray.h"

#include "MC2String.h"
#include "StackOrHeap.h"

#include <stdarg.h>
#include <string.h>
#include <iostream>

/** 
 * Holds a string that can not be resized over a 
 * fixed constant size. Usefull replacement for places where normal
 * c-style char buffers[fixedSize] are used. 
 * All copy and assignments operators copies at most
 * max size of the string and terminates the string with null.
 *
 */
class FixedSizeString {
public:
   typedef char value_type;
   typedef size_t size_type;
   typedef char* iterator;
   typedef const char* const_iterator;

   /// copies max size and the other string
   explicit FixedSizeString( const FixedSizeString& str ):
      m_array( str.getMaxSize() ),
      m_maxSize( str.getMaxSize() ) {
      copy( str );
   }

   /// creates a string with max size 
   explicit FixedSizeString( size_t size ):
      m_array( size ),
      m_maxSize( size ) {
      memset( m_array, 0, size );
   }

   FixedSizeString& operator = ( const char* src ) {
      copy( src );
      return *this;
   }

   FixedSizeString& operator = ( const FixedSizeString& str ) {
      copy( str );
      return *this;
   }

   // copies other string, but maximum getMaxSize() items 
   void copy( const char* src ) {
      strncpy( begin(), src, getMaxSize() );
      begin()[ getMaxSize() - 1 ] = 0;
   }

   /// copies other string, but maximum this->getMaxSize() items.
   void copy( const FixedSizeString& str ) {
      copy( str.begin() );
   }

   /// @return start of string
   iterator begin() { 
      return const_cast<iterator>
         ( const_cast<const FixedSizeString*>
           (this)->begin() ); 
   }
   /// @return end of string + 1
   iterator end() { return const_cast<iterator>
                       ( const_cast<const FixedSizeString*>(this)->end() ); }

   /// @return start of string
   const_iterator begin() const { 
      return m_array; 
   }

   /// @return end of string + 1
   const_iterator end() const { return begin() + size(); }

   /// @return value at position index
   char& operator[] ( size_type index ) { 
      return const_cast<char&>
         ( const_cast<const FixedSizeString&>(*this)[ index ] );
   }

   /// @return value at position index
   const char& operator [] ( size_type index ) const {
      return begin()[ index ];
   }

   /// @return c string
   const char* c_str() const { return begin(); }

   /// @return string length
   size_type size() const { return strlen( begin() ); }

   /// @return true if there is no string, else false.
   bool empty() const { return begin()[0] == 0; }

   /// @return maximum string size this instance can hold
   size_type getMaxSize() const { return m_maxSize; }
                
private:
   StackOrHeap<256, char> m_array;
   const size_t m_maxSize;
};

/**
 * Same as snprintf but for FixedSizeString
 * @return the same return value as snprintf, see snprintf manual
 */
int sprintf( FixedSizeString& str, const char* formatStr, ... )
   __attribute__ ((format (printf, 2, 3)));

inline int sprintf( FixedSizeString& str, const char* formatStr, ... )
{
   va_list args;
   va_start( args, formatStr );

   int nbrPrinted = vsnprintf( str.begin(), str.getMaxSize(),
                               formatStr, args );

   // we set end null.
   // nbrPrinted could be -1 for old glibc.
   // See manual for snprintf.
   str[ str.getMaxSize() - 1 ] = 0;

   va_end( args );
   return nbrPrinted;
}

/// adds fixed string to ostream             
inline ostream& operator << ( ostream& ostr, const FixedSizeString& str ) {
   ostr << str.c_str();
   return ostr;
}

/// adds fixed string to MC2String
inline MC2String& operator += ( MC2String& dest, const FixedSizeString& src ) {
   dest += src.c_str();
   return dest;
}

/// adds two strings
inline MC2String operator + ( const MC2String& strA, const FixedSizeString& strB ) {
   return strA + strB.c_str();
}

/// compares two strings
inline bool operator == ( const FixedSizeString& strA, const char* str ) {
  return strncmp( strA.c_str(), str, strA.getMaxSize() ) == 0;
}

/// compares two strings
inline bool operator == ( const FixedSizeString& first, 
                          const FixedSizeString& second ) {
   return 
      strncmp( first.c_str(), second.c_str(), 
               min( first.getMaxSize(),
                    second.getMaxSize() ) ) == 0;
}


#endif

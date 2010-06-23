/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2STRING_H
#define MC2STRING_H

#include "config.h"
#include <vector>
//#include "StringUtility.h"

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.2.0 */
#if GCC_VERSION > 30200
#include <string>
#define MC2String std::string
#else

#include <algorithm>

/**
 * An MC2 string that is thread safe.
 * Typedef to string when string works! (Or define!)
 * typedef string MC2String;
 * Destructor is missing by purpose.
 * Copy constructor using default vector copy constructor.
 * Internaly the string is allways null terminated.
 *
 */
class MC2String : public vector< char > {
   private:
      /**
       * Terminate the string.
       */
      void terminate() {
         if ( (*this)[ vector<char>::size() -1 ] != '\0' ) {
            push_back( '\0' );
         }
      }


      /**
       * Internal function used by others, find in str.
       */
      inline uint32 find( const char* str, char c, uint32 i, 
                          uint32 len ) const {

         for ( ; i < len ; ++i ) {
            if ( str[ i ] == c ) {
               return i;
            }
         }
         return npos;
      }
      

   public:
      /// Not possible index.
      static const uint32 npos;


      /// Type of size
      typedef uint32 size_type;


      /**
       * Empty string constructor.
       */
      explicit MC2String() : vector<char>(1, '\0') {
      }

      /**
       * Copy range of other string constructor.
       */
      MC2String( const MC2String& str, uint32 pos, uint32 n = npos ) {
         push_back( '\0' );
         replace( 0, npos, str, pos, n );
      }


      /**
       * Copy a part of a char array.
       */
      MC2String( const char* s, uint32 n ) {
         vector<char>::insert( end(), s, 
                               s + (n == npos ? strlen( s ) : n ) );
         push_back( '\0' );
      }


      /**
       * Copy a char array.
       */
      MC2String( const char* str )
            : vector<char>( str,
                            str + strlen( str ) + 1 ) {
      }


      /**
       * Create string with n copies of c.
       */
      MC2String( uint32 n, char c ) 
         : vector<char>( n ,c )
      {
         push_back( '\0' );
      }


      /**
       * Create string from a std-string
       */
      MC2String( const string& s )
            : vector<char>( s.c_str(), 
                            s.c_str() + s.size() + 1 ) {
      }


      /**
       * Create string from a range.
       */
      template<class InputIterator>
      MC2String( InputIterator begin, InputIterator end )
            : vector<char>( begin, 
                            end ) {
         push_back( '\0' );
      }


      /**
       * Override.
       */
      size_type size() const {
         return vector<char>::size() -1;
      }


      /**
       * Override.
       */
      size_type length() const {
         return vector<char>::size() -1;
      }

      /**
       * Overdide.
       */
      bool empty() const {
         return ( size() == 0 );
      }


      /**
       * Override.
       */
      void clear() {
         vector<char>::clear();
         push_back( '\0' );
      }


      /**
       * Terminate string and return pointer to it.
       */
      const char* c_str() const {
         return &(*begin());
      }


      /**
       * Return pointer to string.
       */
      const char* data() const {
         return &(*begin());
      }


      /**
       * Assign a char array.
       */
      MC2String& operator= ( const char* str ) {
         vector<char>::clear();
         vector<char>::insert( end(), str, str + strlen( str ) + 1 );
         return *this;
      }


      /**
       * Assign a char.
       */
      MC2String& operator= ( char c ) {
         vector<char>::clear();
         push_back( c );
         push_back( '\0' );
         return *this;
      }


      /**
       * Assign a std-string.
       */
      MC2String& operator= ( const string& s ) {
         vector<char>::clear();
         append( s );
         push_back( '\0' );
         return *this;
      }


      MC2String& insert( uint32 pos, const char* s ) {
         if ( pos >= length() ) {
            append( s );
         } else { // Not at end
            vector<char>::insert( begin() + pos, s, s + strlen( s ) ); 
         }
         return *this;
      }


      MC2String& insert( uint32 pos, const MC2String& s ) {
         if ( pos >= length() ) {
            append( s );
         } else { // Not at end
            vector<char>::insert( begin() + pos, s.begin(), s.end()-1 ); 
         }
         return *this;
      }


      MC2String& insert( uint32 pos, const char* s, uint32 n ) {
         if ( pos >= length() ) {
            append( s, n );
         } else { // Not at end
            vector<char>::insert( begin() + pos, s, s + n ); 
         }
         return *this;
      }


      MC2String& append( const MC2String& s) {
         vector<char>::insert( end()-1, s.begin(), s.end()-1 );
         return *this;
      }


      MC2String& append( const MC2String& s, uint32 pos, uint32 n ) {
         replace( size(), 0, s, pos, n );
         return *this;
      }


      MC2String& append( const char* s ) {
         vector<char>::insert( end()-1, s, s + strlen( s ) );
         return *this;
      }


      MC2String& append( const char* s, uint32 n ) {
         vector<char>::insert( end()-1, s, s + n );  
         return *this;
      }


      MC2String& append( uint32 n, char c ) {
         vector<char>::insert( end()-1, n, c );
         return *this;
      }


      MC2String& append( const_iterator first, const_iterator last ) {
         vector<char>::insert( end()-1, first, last );
         return *this;
      }
      

      MC2String& operator+= ( const MC2String& s) {
         append( s ); 
         return *this;
      }


      MC2String& operator+= ( const char* s ) {
         append( s );
         return *this;
      }


      MC2String& operator+= ( char c ) {
         vector<char>::insert( end()-1, c );
         return *this;
      }


      MC2String& erase( uint32 pos = 0, uint32 n = npos ) {
         if ( pos >= size() ) {
            pos = size() -1;
         }
         if ( (pos + n >= size()) || (pos + n < n) ) {
            n = size() - pos;
         }
         vector<char>::erase( begin() + pos, begin() + pos + n );
         return *this;
      }


      inline friend ostream& operator << ( ostream& os, 
                                           const MC2String& s ) 
      {
         return (os << s.c_str());
      }


      /**
       * Create a string from MC2String.
       */
      operator string() const { return c_str(); }


      uint32 find( const MC2String& str, uint32 pos = 0 ) const {
         return find( str.data(), pos, str.size() ); 
      }


      uint32 find( const char* s, uint32 pos, uint32 n ) const {
         uint32 i = pos;
         for ( ; i + n <= size() ; ++i )
            if ( data() [ i ] == *s &&
                 /*StringUtility::*/strncmp( data() + i, s, n ) == 0 )
            {
               return i;
            }
         return npos;
      }


      uint32 find ( const char* s, uint32 pos = 0 ) const {
         return find( s, pos, strlen( s ) ); 
      }


      uint32 find( char c, uint32 pos = 0 ) const {
         return find( data(), c, pos, size() );
      }


      uint32 rfind( const MC2String& str, uint32 pos = npos ) const {
         return rfind( str.data(), pos, str.size() );
      }


      uint32 rfind( const char* s, uint32 pos, uint32 n ) const {
         if ( n > size() ) {
            return npos;
         }

         uint32 i = size() - n;
         if ( i > pos ) {
            i = pos;
         }

         for ( ++i ; i-- > 0; ) {
            if ( data() [ i ] == *s &&
                 /*StringUtility::*/strncmp( data() + i, s, n ) == 0 )
            {
               return i;
            }
         }
         return npos;
      }


      uint32 rfind( const char* s, uint32 pos = npos ) const {
         return rfind( s, pos, strlen( s ) ); 
      }


      uint32 rfind( char c, uint32 pos = npos ) const {
         if ( size() < 1 ) {
            return npos;
         }

         uint32 i = size() - 1;
         if ( i > pos ) {
            i = pos;
         }

         for ( ++i ; i-- > 0; )
            if ( data ()[ i ] == c ) {
               return i;
            }
         return npos;
      }

      uint32 find_first_not_of( char c, uint32 pos = 0 ) const {
         for ( uint32 i = pos; i < size(); ++i ) {
            if ( data()[ i ] != c )
               return i;
         }
         return npos;
      }

      size_type find_first_not_of(const char *ptr, size_type off = 0) const
      {
         return find_first_not_of(ptr, off, strlen(ptr));
      }
   
      size_type find_first_not_of(const char *ptr, 
                                  size_type off, size_type count) const
      {
         const char* const end_of = ptr + count; //end of characters to skip.
         vector<char>::const_iterator it = begin() + off; //jump offset
         for( ; it != end(); ++it){
            if( end_of == std::find(ptr, end_of, *it) ) {
               break; //found a character not in ptr. Search no more.
            }
         }
   
         return (it != end()) ? (it - begin()) : npos;
      }
   
      size_type find_first_not_of(const MC2String& right, size_type off = 0) const
      {
         return find_first_of(right.c_str(), off);
      }
   
      uint32 find_first_of( const MC2String& str, uint32 pos = 0) const {
         return find_first_of( str.data(), pos, str.size() ); 
      }


      uint32 find_first_of( const char* s, uint32 pos, uint32 n ) const {
         uint32 i = pos;
         for ( ; i < size() ; ++i ) {
            if ( find( s, data()[ i ], 0, n ) != npos ) {
               return i;
            }
         }
         return npos;
      }


      uint32 find_first_of( const char* s, uint32 pos = 0) const {
         return find_first_of( s, pos, strlen( s ) ); 
      }


      uint32 find_first_of( char c, uint32 pos = 0 ) const { 
         return find ( c, pos ); 
      }


      uint32 find_last_of( const MC2String& str, uint32 pos = npos ) const{
         return find_last_of( str.data(), pos, str.length() ); 
      }


      uint32 find_last_of( const char* s, uint32 pos, uint32 n ) const {
         if ( size() == 0 ) {
            return npos;
         }
         uint32 i = size() - 1;
         if ( i > pos ) {
            i = pos;
         }
         for ( ++i ; i-- > 0 ; ) {
            if ( find( s, data()[ i ], 0, n ) != npos ) {
               return i;
            }
         }
         return npos;
      }


      uint32 find_last_of( const char* s, uint32 pos = npos ) const {
         return find_last_of( s, pos, strlen( s ) );
      }


      uint32 find_last_of( char c, uint32 pos = npos ) const {
         return rfind( c, pos ); 
      }




      uint32 find_last_not_of( const char* s, uint32 pos, uint32 n ) const {

         if ( size() == 0){
            return npos;
         }

         uint32 i = size() - 1;
         if (i > pos){
            i = pos;
         }

         for ( ++i; i-- > 0;){
            if ( find (s, data () [i], 0, n) == npos){
               return i;
            }
         }
         
         return npos;
      }

      uint32 find_last_not_of( const char* s, uint32 pos = npos ) const {
         return find_last_not_of( s, pos, strlen( s ) );
      }


      uint32 find_last_not_of( const MC2String& str, uint32 pos = npos) const {
         return find_last_not_of( str.data(), pos, str.size() ); 
      }

      uint32 find_last_not_of( const char c, uint32 pos = npos) const {

         if ( size() == 0){
            return npos;
         }

         uint32 i = size() - 1;
         if (i > pos){
            i = pos;
         }

         for ( ++i; i-- > 0;){
            if ( data () [i] != c ){
               return i;
            }
         }
         
         return npos;
      }
      



      MC2String substr( uint32 pos = 0, uint32 n = npos ) const {
         if ( n != npos ) {
            if ( pos + n >= size() ) {
               n = size() - pos;
            }
         }
         return MC2String( data()+pos, n ); 
      }


      int compare( const MC2String& str, uint32 pos = 0, 
                   uint32 n = npos ) const 
      {
         if ( pos > size() ) {
            return npos;
         }

         uint32 rlen = size() - pos;
         if ( rlen > n ) {
            rlen = n;
         }
         if ( rlen > str.size() ) {
            rlen = str.size();
         }
         int r = /*StringUtility::*/strncmp( 
            data() + pos, str.data (), rlen );
         if ( r != 0 ) {
            return r;
         }
         if ( rlen == n ) {
            return 0;
         }
         return (size() - pos) - str.size();
      }


      int compare( const char* s, uint32 pos, uint32 n ) const {
         if ( pos > size() ) {
            return npos;
         }
         uint32 rlen = size() - pos;
         if ( rlen > n ) {
            rlen = n;
         }
         int r = /*StringUtility::*/strncmp( 
            data() + pos, s, rlen );
         if ( r != 0 ) {
            return r;
         }
         return (size() - pos) - n;
      }


      int compare ( const char* s, uint32 pos = 0) const { 
         return compare( s, pos, strlen( s ) ); 
      }


      MC2String& replace( uint32 pos1, uint32 n1, const MC2String& str,
                          uint32 pos2 = 0, uint32 n2 = npos )
      {
         const size_t len2 = str.length();

         if ( pos1 == 0 && n1 >= length() && pos2 == 0 && n2 >= len2 ) {
            return operator=( str );
         }

         MC2_ASSERT( pos2 <= len2 );

         if ( n2 > len2 - pos2 ) {
            n2 = len2 - pos2;
         }

         return replace (pos1, n1, str.data() + pos2, n2);
      }


      MC2String& replace( uint32 pos, uint32 n1, const char* s, uint32 n2 )
      {
         uint32 len = size();
         MC2_ASSERT( pos <= len );
         if ( n1 > len - pos ) {
            n1 = len - pos;
         }
         MC2_ASSERT( len - n1 <= max_size() - n2 );
         iterator insPos = vector<char>::erase( begin() + pos, 
                                                begin() + pos + n1 );
         bool addEol = false;
         if ( insPos == end() ) {
            addEol = true;
         }
         vector<char>::insert( insPos, s, s + n2 );
         if ( addEol ) {
            push_back( '\0' );
         }
         return *this;
      }


      MC2String& replace( uint32 pos, uint32 n1, const char* s ) { 
         return replace( pos, n1, s, strlen( s ) ); 
      }


      MC2String& assign( const char* s, uint32 n = npos ) {
         vector<char>::clear();
         vector<char>::insert( end(), s, 
                               s + (n == npos ? strlen( s ) : n ) );
         push_back( '\0' );
         return *this;
      }


      MC2String& assign( const MC2String& str, uint32 pos = 0, 
                         uint32 n = npos ) 
      {
         replace( 0, npos, str, pos, n );
         return *this;
      }

};


inline MC2String operator+ ( const MC2String& lhs, const MC2String& rhs ) {
   MC2String str( lhs );
   str.append( rhs );
   return str;
}


inline MC2String operator+ ( const char* lhs, const MC2String& rhs ) {
   MC2String str( lhs );
   str.append( rhs );
   return str;
}


inline MC2String operator+ ( char lhs, MC2String& rhs ) {
   MC2String str( 1, lhs );
   str.append( rhs );
   return str;
}


inline MC2String operator+ ( const MC2String& lhs, const char* rhs ) {
   MC2String str( lhs );
   str.append( rhs );
   return str;
}


inline MC2String operator+ ( const MC2String& lhs, char rhs ) {
   MC2String str( lhs );
   str.append( 1, rhs );
   return str;
}


inline bool
operator== ( const MC2String& lhs, const MC2String& rhs ) {
   return (lhs.compare( rhs ) == 0);
}


inline bool
operator== ( const char* lhs, const MC2String& rhs ) {
  return ( rhs.compare( lhs ) == 0 );
}


inline bool
operator== ( const MC2String& lhs, const char* rhs ) {
   return ( lhs.compare( rhs ) == 0 );
}


inline bool
operator!= ( const char* lhs, const MC2String& rhs ) {
  return ( rhs.compare( lhs ) != 0 );
}


inline bool
operator!= ( const MC2String& lhs, const char* rhs ) {
  return ( lhs.compare( rhs ) != 0 );
}


inline bool
operator< ( const MC2String& lhs, const MC2String& rhs ) {
  return ( lhs.compare( rhs ) < 0 );
}


inline bool
operator< ( const char* lhs, const MC2String& rhs ) {
  return ( rhs.compare( lhs ) > 0) ;
}


inline bool
operator< ( const MC2String& lhs, const char* rhs ) {
  return ( lhs.compare( rhs ) < 0 );
}


inline bool
operator> ( const char* lhs, const MC2String& rhs ) {
  return ( rhs.compare( lhs ) < 0 );
}


inline bool
operator> ( const MC2String& lhs, const char* rhs ) {
  return ( lhs.compare( rhs ) > 0 );
}


inline bool
operator<= ( const char* lhs, const MC2String& rhs ) {
  return ( rhs.compare( lhs ) >= 0 );
}


inline bool
operator<= ( const MC2String& lhs, const char* rhs ) {
  return ( lhs.compare( rhs ) <= 0 );
}


inline bool
operator>= ( const char* lhs, const MC2String& rhs ) {
  return ( rhs.compare( lhs ) <= 0 );
}


inline bool
operator>= ( const MC2String& lhs, const char* rhs ) {
  return ( lhs.compare( rhs ) >= 0 );
}


inline bool
operator!= ( const MC2String& lhs, const MC2String& rhs ) {
   return ( lhs.compare( rhs ) != 0 );
}


inline bool
operator> ( const MC2String& lhs, const MC2String& rhs ) {
  return ( lhs.compare( rhs ) > 0 );
}


inline bool
operator<= ( const MC2String& lhs, const MC2String& rhs ) {
  return ( lhs.compare( rhs ) <= 0 );
}


inline bool
operator>= ( const MC2String& lhs, const MC2String& rhs ) {
  return ( lhs.compare( rhs ) >= 0 );
}


istream& operator >> ( istream& is, MC2String& s );


istream& getline ( istream& is, MC2String& s, char delim = '\n' );

#endif
#endif // MC2STRING_H


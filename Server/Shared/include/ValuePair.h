/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STLUTILITY_VALUEPAIR_H
#define STLUTILITY_VALUEPAIR_H

#include "config.h"

namespace STLUtility {

/**
 * A pair that sorts on the first value.
 * And has a key and value to get the first and second.
 *
 */
template < typename A, typename B >
class ValuePair {
public:
   typedef A key_type;
   typedef B value_type;

   A key;
   B value;

   ValuePair() : key(), value() {}
   ValuePair( const A& a, const B& b ) : key( a ), value( b ) {}
   ValuePair( const A& a ) : key( a ), value() {}

   template< typename Aa, typename Bb >
   ValuePair( const ValuePair< Aa, Bb > &p ) : key( p.key ), value( p.value ){}
};

/// Two pairs of the same type are equal if their key are equal.
template< class A, class B >
inline bool
operator == ( const ValuePair< A, B>& a, const ValuePair< A, B>& b ) { 
   return a.key == b.key;
}

/// <http://gcc.gnu.org/onlinedocs/libstdc++/20_util/howto.html#pairlt>
template< class A, class B >
inline bool
operator < ( const ValuePair< A, B >& a, const ValuePair< A, B >& b ) {
   return a.key < b.key;
}

/// Uses @c operator== to find the result.
template< class A, class B >
inline bool
operator != ( const ValuePair< A, B >& a, const ValuePair< A, B >& b ) {
   return !(a == b); 
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator > ( const ValuePair< A, B >& a, const ValuePair<A, B >& b ) { 
   return b < a; 
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator <= ( const ValuePair< A, B >& a, const ValuePair< A, B >& b ) {
   return !(b < a);
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator >= ( const ValuePair< A, B >& a, const ValuePair< A, B >& b ) {
   return !(a < b); 
}

/// Two pairs of the same type are equal if their key are equal.
template< class A, class B >
inline bool
operator == ( const ValuePair< A, B>& a, const A& b ) { 
   return a.key == b;
}

/// <http://gcc.gnu.org/onlinedocs/libstdc++/20_util/howto.html#pairlt>
template< class A, class B >
inline bool
operator < ( const ValuePair< A, B >& a, const A& b ) {
   return a.key < b;
}

/// Uses @c operator== to find the result.
template< class A, class B >
inline bool
operator != ( const ValuePair< A, B >& a, const A& b ) {
   return !(a == b); 
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator > ( const ValuePair< A, B >& a, const A& b ) { 
   return b < a; 
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator <= ( const ValuePair< A, B >& a, const A& b ) {
   return !(b < a);
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator >= ( const ValuePair< A, B >& a, const A& b ) {
   return !(a < b); 
}

/// Two pairs of the same type are equal if their key are equal.
template< class A, class B >
inline bool
operator == ( const A& a, const ValuePair< A, B>& b ) { 
   return a == b.key;
}

/// <http://gcc.gnu.org/onlinedocs/libstdc++/20_util/howto.html#pairlt>
template< class A, class B >
inline bool
operator < ( const A& a, const ValuePair< A, B >& b ) {
   return a < b.key;
}

/// Uses @c operator== to find the result.
template< class A, class B >
inline bool
operator != ( const A& a, const ValuePair< A, B >& b ) {
   return !(a == b); 
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator > ( const A& a, const ValuePair<A, B >& b ) { 
   return b < a; 
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator <= ( const A& a, const ValuePair< A, B >& b ) {
   return !(b < a);
}

/// Uses @c operator< to find the result.
template< class A, class B >
inline bool
operator >= ( const A& a, const ValuePair< A, B >& b ) {
   return !(a < b); 
}


/**
 *  @brief A convenience wrapper for creating a pair from two objects.
 *  @param  x  The first object.
 *  @param  y  The second object.
 *  @return   A newly-constructed pair<> object of the appropriate type.
 *
 *  The standard requires that the objects be passed by reference-to-const,
 *  but LWG issue #181 says they should be passed by const value.  We follow
 *  the LWG by default.
 */
template< typename A, typename B >
inline ValuePair< A, B >
make_vpair( A a, B b) {
   return ValuePair< A, B >( a, b );
}



} // End namespace STLUtility

#endif  // STLUTILITY_VALUEPAIR_H

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STLUTILITY_SORT1ST_H
#define STLUTILITY_SORT1ST_H

#include <functional>

namespace STLUtility {

/**
 * Binary function for sorting to first type in a pair.
 * @see makeSort1st for usage.
 */
template <typename Pair>
struct Sort1st: public binary_function<Pair, typename Pair::first_type, bool> {
   bool operator () ( const Pair& a, const Pair& b ) const {
      return a.first < b.first;
   }
   bool operator () ( const Pair& a, 
                      const typename Pair::first_type& b ) const {
      return a.first < b ;
   }
   bool operator () ( const typename Pair::first_type& a, 
                      const Pair& b ) const {
      return a < b.first;
   }
};

/**
 * Helper for Sort1st, usage 
 * \code
 * makeSort1st( container );
 * \endcode
 *
 * This will help determine the type instead of explicitly specifying it
 * which helps when the container type or value type changes.
 * For example:
 * \code
 * vector< pair< int, char > > somevector;
 * for_each( somevector.begin(), somevector.end(),
 *           makeSort1st( somevector ) );
 * \endcode
 * Instead of:
 * \code
 * for_each( somevector.begin(), somevector.end(),
 *           Sort1st<pair< int, char > >() );
 * \endcode
 *
 * @param not_used Is only there to determine the type.
 * @return a Sort1st of type T::value_type
 */
template < typename T >
Sort1st<typename T::value_type> makeSort1st( T& not_used ) {
   return Sort1st<typename T::value_type>();
}

}

#endif // STLUTILITY_SORT1ST_H

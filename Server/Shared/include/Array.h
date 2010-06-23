/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STLUTILITY_ARRAY_H
#define STLUTILITY_ARRAY_H

#include "config.h"

namespace STLUtility {

/**
 * A simple vector container that does not destroy the data. Think of it as a
 * std::vector but it does not delete the data.
 * Example usage @see GenericMap where it uses the container as helper to point
 * to a small array within an even larger array. Or in other words; a subset array.
 *
 */
template < typename T >
class Array {
public:
   typedef T& reference;
   typedef const T& const_reference;
   typedef T* iterator;
   typedef const T* const_iterator;
   typedef uint32 size_type;
   typedef T value_type;
   typedef T* pointer;
   typedef const T* const_pointer;

   /// Empty array
   Array(): m_data( 0 ), m_size( 0 ) { 
   }

   /**
    * @param data the data to hold.
    * @param size size of the data.
    */
   Array( pointer data, size_type size ):
      m_data( data ), m_size( size ) {
   }

   /// @return start of this array
   iterator begin() {
      return m_data;
   }

   /// @return end of this array
   iterator end() {
      return m_data + m_size;
   }

   /// @return start of this array
   const_iterator begin() const {
      return const_cast<const_iterator>(const_cast<Array<T>*>(this)->begin());
   }
   
   /// @return end of this array
   const_iterator end() const {
      return const_cast<const_iterator>(const_cast<Array<T>*>(this)->end());
   }

   /// @return referens to object as position pos
   reference operator []( size_type pos ) {
      return *( begin() +  pos );
   }
   /// @return const referens to object as position pos
   const_reference operator []( size_type pos ) const {
      return *( begin() + pos );
   }

   /// @return size of this array
   size_type size() const {
      return m_size;
   }
   /// @return true if container is empty
   bool empty() const {
      return size() == 0;
   }
private:
   pointer m_data; ///< the data.
   size_type m_size; ///< size of m_data.
};

/**
 * Array creator.
 * @param data the data to put in to the array
 * @param size number of elements in the array
 */
template <typename T>
Array<T> make_array( typename Array< T >::pointer data,
                     typename Array< T >::size_type size ) {
   return Array< T >( data, size );
}

}

#endif  // STLUTILITY_ARRAY_H

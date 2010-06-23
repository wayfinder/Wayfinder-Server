/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIMPLEARRAY_H
#define SIMPLEARRAY_H

#include "config.h"
#include <algorithm>

/**
 * Simple array template that creates a T * array
 *
 */
template <typename T>
class SimpleArray {
public:
   typedef unsigned int size_type;
   typedef T* iterator;
   typedef const T* const_iterator;

   SimpleArray(): m_data( 0 ), m_size( 0 ) { }
   /**
    * Creates an array with specified size
    */
   explicit SimpleArray( size_type size ):m_data(0), m_size(0) {
      allocate( size );
   }

   SimpleArray( const SimpleArray<T>& copyMe ):m_data(0), m_size(0) {
      copy( copyMe );
   }

   virtual ~SimpleArray() {
      delete [] m_data;
   }

   SimpleArray<T>& operator = ( const SimpleArray<T>& rhs ) {
      copy( rhs );
   }

   void copy( const SimpleArray<T>& copyMe ) {
      if ( this == &copyMe ) 
         return;

      allocate( copyMe.size() );
      for (size_type i = 0; i < size(); ++i ) {
         m_data[ i ] = copyMe.m_data[ i ];
      }
   }

   /// allocates array with specified size, will not copy old data
   void allocate( size_type size ) {
      delete [] m_data;
      m_data = new T[size];
      m_size = size;
   }

   void swap( SimpleArray<T>& other ) {
      std::swap( other.m_data, m_data );
      std::swap( other.m_size, m_size );
   }

   /// @return start of array
   const_iterator begin() const { return data(); }
   /// @return start of array
   iterator begin() { return data(); }
   /// @return end of array, out side buffer
   const_iterator end() const { return data() + size(); }
   /// @return end of array, out side buffer
   iterator end() { return data() + size(); }

   /// @return size of array
   size_type size() const { return m_size; }
   /// @return item in array at position index
   T& operator []( size_type index ) { return m_data[index]; }
   /// @return item in array at position index
   const T& operator [] ( size_type index ) const { return m_data[index]; }
   
   T& get( size_type index ) { return m_data[ index ]; }
   const T& get( size_type index ) const { return m_data[ index ]; }

   /// @return pointer to array
   T* data() { return m_data; }
   /// @return pointer to array
   const T* data() const { return m_data; }
   /**
    * Release data pointer
    * Internal data will be cleared.
    * @return  pointer to data
    */
   T* release() { 
      T* returnData = m_data;
      m_data = NULL;
      m_size = 0;
      return returnData;
   }
private:

   T* m_data;
   size_type m_size;
};

#endif 

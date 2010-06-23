/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SCOPEDARRAY_H
#define SCOPEDARRAY_H

#include "NotCopyable.h"

/// same as auto ptr but will call delete [] on the pointer
template < typename T >
class ScopedArray: private NotCopyable {
public:
   explicit ScopedArray( T* array = 0 ):
      m_ptr( array ) { 
   }

   ~ScopedArray() {
      destroy();
   }

   /// destoy old array and assign new
   void reset( T* array ) {
      destroy();
      m_ptr = array;
   }

   /// @return reference to index
   T& operator[]( unsigned int pos ) {
      return get()[ pos ];
   }
   /// @return reference to index
   const T& operator[]( unsigned int pos ) const {
      return get()[ pos ];
   }

   /// @return ptr to array
   const T* get() const {
      return m_ptr;
   }

   /// @return ptr to array
   T* get() {
      return m_ptr;
   }

   /// stop owning this array ptr
   T* release() {
      T* oldPtr = m_ptr;
      m_ptr = 0;
      return oldPtr;
   }

   /// destroy array
   void destroy() {
      delete [] release();
   }
   
private:
   T* m_ptr; //< holds ptr to array
};

#endif // SCOPEDARRAY_H

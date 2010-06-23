/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AUTOPTR_H
#define AUTOPTR_H

/**
 * Class very similar to std::auto_ptr except that it has void destroy()
 * that has to be specialized. 
 * Use this class if the object must be destroyed in another way
 * other than "delete obj" 
 * @see FilePtr.h for example.
 */
template <typename T>
class AutoPtr {
public:
   explicit AutoPtr( T* ptr = 0 ):
      m_ptr( ptr ) {
   }

   ~AutoPtr() {
      destroy();
   }
   /// release ptr 
   T* release() {
      T* ptr = m_ptr;
      m_ptr = 0;
      return ptr;
   }

   /// @return ptr
   T* get() {
      return m_ptr;
   }
   /// @return ptr
   const T* get() const {
      return m_ptr;
   }

   /// destroy current ptr and set new
   void reset( T* ptr ) {
      destroy();
      m_ptr = ptr;
   }
   T& operator *() {
      return *get();
   }
   /// @return ptr
   const T& operator *() const {
      return *get();
   }
   T* operator->() {
      return get();
   }

   const T* operator->() const {
      return get();
   }

   /// destroy ptr
   void destroy();

private:
   T* m_ptr;
};

#endif // AUTOPTR_H

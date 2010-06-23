/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DELETEHELPERS_H
#define DELETEHELPERS_H

#include "config.h"
#include <algorithm>
#include <queue>
#include <list>

namespace STLUtility {

/// deletes all values in a std::queue
template < typename T>
void deleteQueueValues( queue<T*>& values ) {
   while ( ! values.empty() ) {
      delete values.front();
      values.pop();
   }
}

/// deletes all values in a std::list
template < typename T>
void deleteListValues( std::list<T*>& values ) {
   while ( ! values.empty() ) {
      delete values.front();
      values.pop_front();
   }
}


// destroys all items passed to operator()(T)
struct DeleteObject {
   template <typename A>
   A operator () (A item) {
      delete item;
      return NULL;
   }
};

// destroys all items passed to operator()(T)
struct DeleteArrayObject {
   template <typename A>
   A operator () (A item) {
      delete [] item;
      return NULL;
   }
};

// de-allocates with the old C function free()
struct FreeDeleter {
   template<typename T>
   void operator()(T* ptr) {
      free( ptr );
   }
};

/// destroys the a value in a map
struct DeleteMapValue {
   template <typename argument_type>
   void operator ()( argument_type& p ) const {
      delete p.second;
   }
};
/// deletes all second values in a map type container
template <typename T>
void deleteAllSecond( T& val ) {
   for_each( val.begin(), val.end(),
             DeleteMapValue() );
   val.clear();
}

/// delete all values in container and clear it.
template <typename T>
void deleteValues( T& val ) {
   for_each( val.begin(), val.end(),
             DeleteObject() );
   val.clear();
}

/// delete all values in container using array delete and clear it.
template <typename T>
void deleteArrayValues( T& val ) {
   for_each( val.begin(), val.end(),
             DeleteArrayObject() );
   val.clear();
}

/**
 * deletes all values in a container when it goes out of scope
 * Usage: AutoContainer< vector<SomeClass*> > myVector;
 * This will delete all pointers in myVector when it goes out of scope.
 */
template <typename T>
class AutoContainer: public T {
public:
   ~AutoContainer() {
      deleteValues( *this );
   }

private:
   /// Dangerous to do
   operator T&();
};

template <typename T>
class AutoContainerMap: public T {
public:
   AutoContainerMap() { }
   explicit AutoContainerMap( const T& src ):T( src ) { };
   ~AutoContainerMap() {
      deleteAllSecond( *this );
   }
   T& operator=(const T& o) {
      deleteAllSecond( *this );
      return T::operator=( o );
   }
private:
   /// Dangerous to do
   operator T&();   
};

}

#endif 

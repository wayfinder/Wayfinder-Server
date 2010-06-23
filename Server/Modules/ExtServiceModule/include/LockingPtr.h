/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOCKINGPTR_H
#define LOCKINGPTR_H

#include "ISABThread.h"
#include "NotCopyable.h"

/**
 * Thread mutex for volatile objects.
 * This must be used for volatile objects that has to be locked.
 */
template <typename T, typename Mutex = ISABMutex>
class LockingPtr: private NotCopyable {
public:
   typedef T* Ptr;
   typedef T& Ref;

   /**
    * @param lockObject The object to lock for the thread.
    * @param mutex The mutex to use to lock the object down.
    */
   LockingPtr( volatile T& lockObject, Mutex& mutex ):
      m_lockObject( const_cast<T*>( &lockObject ) ),
      m_sync( mutex ) {
   }

   ~LockingPtr() {
   }

   Ref operator * () {
      return *m_lockObject;
   }

   Ptr operator -> () {
      return m_lockObject;
   }

private:
   Ptr m_lockObject; ///< object to lock
   ISABSync m_sync;
};

template < typename T, typename Mutex >
LockingPtr<T, Mutex>* createLock( volatile const T& object, Mutex& mutex ) {
   return new LockingPtr<T, Mutex>( const_cast<T&>( object ), mutex );
}

#endif // LOCKINGPTR_H

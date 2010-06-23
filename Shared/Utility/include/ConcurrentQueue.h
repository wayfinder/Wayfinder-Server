/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include "PMonitor.h"
#include "NotCopyable.h"

#include <queue>

/**
 * A thread safe FIFO queue.
 * Implemented using PThreads.
 *
 */
template <typename T>
class ConcurrentQueue: private NotCopyable {
public:
   /**
    * Wait for the queue to be non empty and pop it.
    * The function was intentionaly designed not to return a value,
    * because if it should return and the assignment failed with exception
    * the queue will not have the old value and the state of the queue after
    * the exception will be wrong. So instead the assignment is done inside
    * waitAndPop, if it fails the queue will not pop and it will be in a
    * correct state after the failure.
    * @param value Assign queue value to this.
    */
   void waitAndPop( T& value ) {
      PThread::Monitor::Sync sync( m_lock );
      while ( m_queue.empty() ) {
         m_lock.wait();
      }
      value = m_queue.front();
      m_queue.pop();
   }

   /**
    * Push a value to the queue.
    * @param value to be pushed.
    */
   void push( const T& value ) {
      PThread::Monitor::Sync sync( m_lock );
      bool wasEmpty = m_queue.empty();
      m_queue.push( value );
      if ( wasEmpty ) {
         // Only notify if the queue was empty
         m_lock.notifyAll();
      }
   }

private:
   /// Internal queue handling.
   std::queue< T > m_queue;
   /// Lock for internal queue.
   PThread::Monitor m_lock;
};

#endif // CONCURRENT_QUEUE_H

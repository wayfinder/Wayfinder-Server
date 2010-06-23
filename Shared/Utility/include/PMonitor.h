/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_PTHREAD_MONITOR_H
#define MC2_PTHREAD_MONITOR_H
#include "NotCopyable.h"

#include <pthread.h>

namespace PThread {

/**
 * Monitor implemented using pthreads.
 */
class Monitor: private NotCopyable {
public:
   /// Synchronizes Monitor
   class Sync {
   public:
      /// Lock monitor \c mon .
      explicit Sync( Monitor& mon ):
         m_mon( mon ) {
         m_mon.lock();
      }

      /// Unlock monitor.
      ~Sync() {
         m_mon.unlock();
      }

   private:
      Monitor& m_mon;
   };

   /// Sync must do some fancy stuff
   friend class Monitor::Sync;

   Monitor() {
      pthread_mutex_init( &m_lock, NULL );
      pthread_cond_init( &m_cond, NULL );
   }

   ~Monitor() {
      pthread_cond_destroy( &m_cond );
      pthread_mutex_destroy( &m_lock );
   }

   /// Wait until notified.
   void wait() {
      pthread_cond_wait( &m_cond, &m_lock );
   }

   /// Notify one waiter.
   void notify() {
      pthread_cond_signal( &m_cond );
   }

   /// Notify everyone that waits on this monitor.
   void notifyAll() {
      pthread_cond_broadcast( &m_cond );
   }

private:
   /// Lock monitor
   void lock() {
      pthread_mutex_lock( &m_lock );
   }
   /// Unlock monitor
   void unlock() {
      pthread_mutex_unlock( &m_lock );
   }

   /// condition for lock.
   pthread_cond_t m_cond;
   /// mutex lock.
   pthread_mutex_t m_lock;
};

} // PThread

#endif // MC2_PTHREAD_MONITOR_H

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_THREAD_H
#define MC2_THREAD_H

#include <pthread.h>
#include <memory>

#include <iostream>
using namespace std;
#include <errno.h>

namespace MC2 {

/**
 * Base functor for callbacks with thread.
 */
struct ThreadProxyBase {
   virtual void operator() () = 0;
};

/** 
 * ThreadProxy functor...this should be in some kind of
 * binding functor set instead. ( boost::bind )
 */
template <typename Receiver, typename ReturnType=void>
class ThreadProxy: public ThreadProxyBase {
public:
   typedef ReturnType (Receiver::* Action)();
   ThreadProxy( Receiver& r, Action a ): 
      m_receiver( r ), m_action( a ) { }
   void operator () () {
      (m_receiver.*m_action)();
   }
private:
   Receiver &m_receiver;
   Action m_action;
};

/**
 * Exception thrown by class Thread.
 */
class ThreadException: public std::exception {
public:
   explicit ThreadException( const char* what ) throw():
      m_what( what ) {
   }

   ~ThreadException() throw () {
   }

   const char* what() const throw() {
      return m_what;
   }

private:
   const char* m_what;
};

/**
 * Basic thread functionality for test suit.
 * @code
 * struct Basic {
 *     void doStuff() {
 *     }
 * };
 * Basic functor;
 *
 * Thread somethread( threadCall( functor, &Basic::doStuff ) );
 * somethread.join();
 *
 * @endcode
 *
 */
class Thread {
public:
   typedef std::auto_ptr< ThreadProxyBase > Callback;

   /// Creates and starts a new thread.
   explicit Thread( Callback callback ) throw ( ThreadException ):
      m_thread_id( 0 ),
      m_callback( callback ) {
      start();
   }

   /// wait for thread to finish
   void join();

private:
   // start thread
   void start() throw ( ThreadException ) ;
   
   static void* run( void *data ) {
      Callback& b = static_cast<Thread*>( data )->m_callback;
      (*b)();
      pthread_exit( NULL );
   }
   pthread_t m_thread_id;
   Callback m_callback;
};

void Thread::start() throw ( ThreadException ) {
   int ret = pthread_create( &m_thread_id, NULL, &Thread::run, this );
   if ( ret == -1 ) {
      char buff[ 1024 ];
      strerror_r( errno, buff, 1024 );
      cerr << buff << endl;
   }
}

void Thread::join() {
   pthread_join( m_thread_id, NULL );
}

/// Helper function to create a callback functor for Thread.
template <typename Receiver, typename Action >
Thread::Callback threadCall( Receiver& r, Action a ) {
   return Thread::Callback( new ThreadProxy< Receiver >( r, a ) );
}

}

#endif // MC2_THREAD_H

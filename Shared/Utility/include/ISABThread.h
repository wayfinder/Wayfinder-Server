/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABTHREAD_H
#define ISABTHREAD_H

#include "config.h"
#include <stdlib.h>
#include "JTC/JTC.h"
#include <errno.h>
#include "SysUtility.h"


// Forward declaration
class ISABThreadGroup;
class ISABThread;

typedef JTCHandleT<ISABThreadGroup> ISABThreadGroupHandle;
typedef JTCHandleT<ISABThread> ISABThreadHandle;


/**
 * This class encapsulates the functionality of a thread.
 */
class ISABThread : public JTCThread
{
  public:
   /**
    * Constructs a new Thread with default ThreadGroup if none specified.
    */
   ISABThread( ISABThreadGroup* group = NULL, 
               const char* name = "ISABThread" );

   /**
    * Destructor.
    */
   virtual ~ISABThread();


   /**
    * Start the thread.
    */
   void start();


   /**
    * Override this method to provide functionality to your thread.
    */
   virtual void run();

   
   /**
    * Determine if the thread is currently running.
    */
   bool isAlive() const;
      

   /**
    * Wait for this thread to terminate.
    */
   void join();
   

   /**
    * Wait at most millis for the thread to terminate.
    *
    * @param millis is the number of milliseconds to wait for
    *        termination.
    * @param nanos is the number of nanoseconds to wait for
    *        termination.
    */
   void join(long millis, int nanos = 0);


   /**
    * Set the priority of this thread.
    *
    * @param newPri is the new priority of the thread.
    */
   void setPriority(int newPri);
   

   /**
    * Get the priority of this thread.
    */
   int getPriority() const;
   
   
   /**
    * Terminate the thread cleanly by setting the protected
    * boolean 'terminated'.
    */
   void terminate();

   /**
    * Sleep for millis milliseconds, and nano nanoseconds.
    * @param millis The milliseconds.
    * @param nano The nanoseconds.
    */
    static void sleep(long millis, int nano = 0);

   /**
    * Sleep for some millis.
    *
    * @param ms The nbr of milliseconds to sleep.
    */
   virtual void milliSleep( int ms );

   
  protected:
   /// Thread running.
   bool terminated;


  private:
   /// The ISAB system thread group, used if no group is specified.
   ISABThreadGroup* defaultThreadGroup;


   // Handle for above or constructor - group.
   JTCThreadGroupHandle* threadGroupHandle;
};


/**
 * Before using the ISABThread package, instantiate this class!
 */
class ISABThreadInitialize : public JTCInitialize
{
  public:
   /**
    * Constructs a ThreadInitializer that handles the thread handling.
    */
   inline ISABThreadInitialize();

   
   /**
    * Destructor
    */
   inline virtual ~ISABThreadInitialize();

   
   /**
    * Wait for all running threads to terminate.
    */
   inline void waitTermination();
};


/**
 * A group of threads handler.
 */
class ISABThreadGroup : public JTCThreadGroup
{
  public:
   /**
    * Constructs a new ThreadGroup.
    */
   inline ISABThreadGroup(const char* name = "ISABThreadGroup");


   /**
    * Constructs a new ThreadGroup with parent as parent ThreadGroup.
    */
   inline ISABThreadGroup(ISABThreadGroup* parent, 
                          const char* name = "ISABThreadGroup");


   /**
    * Destructor.
    */
   inline virtual ~ISABThreadGroup();


   /** 
    * Called on an uncaught exception.
    * @param t the threadhande of the thread with uncaught exception.
    * @param e the exception.
    */
   inline virtual void uncaughtException(JTCThreadHandle t, 
                                         const JTCException& e);


   /** 
    * Called on an uncaught exception non JTC exception.
    * @param t the threadhande of the thread with uncaught exception.
    */
   inline virtual void uncaughtException(JTCThreadHandle t);   

   /**
    * The parent of this ThreadGroup.
    */
   inline ISABThreadGroupHandle getParent() const;

   /**
    * The number of active threads in this group, and in child 
    * ThreadGroups.
    */
   inline int activeCount() const;
};


/**
 * Monitor superclass
 */
class ISABMonitor : public JTCMonitor
{
  public:
   /**
    * Constructs a new monitor with no locks.
    */
   inline ISABMonitor();

   
   /**
    * Destructor.
    */
   inline virtual ~ISABMonitor();


   /**
    * Wait for notification.
    */
   inline void wait();


   /**
    * Wait for notification, or timeout milliseconds.
    */
   inline void wait(long timeout);


   /**
    * Wake one waiting object.
    */
   inline void notify();


   /**
    * Wake all waiting objects.
    */
   inline void notifyAll();
};

/**
 * Critical section class.
 * This class can NOT handle recursive locks, use ISABRecursiveMutex
 * if you want to do that.
 *
 */
class ISABMutex : public JTCMutex {
   public:
      /**
       * Locks the mutex. If the mutex is already locked, the calling
       * thread blocks until the mutex is unlocked. If the current
       * owner of the mutex's lock attempts to relock the mutex a 
       * deadlock may result.
       */
      inline void lock() const;


      /**
       * Unlocks the mutex. This function is called by the owner of the
       * mutex's lock to release it. If the caller doesn't have the
       * mutex's lock then the behaviour is undefined.
       */
      inline void unlock() const;
};

/**
 * Monitor synchronization class
 */
class ISABSync : public JTCSynchronized
{
public:
   /**
    *   A new lock for monitor mon.
    *   @param mon is the monitor lock and then unlock upon desruction.
    */
   ISABSync(const ISABMonitor & mon) : JTCSynchronized( mon ) {}
   
   /**
    *   A new lock for mutex mut.
    *   @param mon is the mutex to lock and then unlock upon desruction.
    */
   ISABSync(const ISABMutex & mut) : JTCSynchronized( mut ) {}

   /**
    *   Releases the lock.
    */
   virtual ~ISABSync() {}

private:
   ///  Not implemented to prevent use.
   ISABSync( const ISABSync& other );
   ///  Not implemented to prevent use.
   ISABSync& operator=( const ISABSync& other );
};


/**
 * Critical section class.
 * This class CAN handle recursive locks.
 *
 */
class ISABRecursiveMutex : public JTCRecursiveMutex {
   public:
      /**
       * Locks the mutex. If the mutex is already locked, the calling
       * thread blocks until the mutex is unlocked.
       */
      inline void lock() const;


      /**
       * Unlocks the mutex. This function is called by the owner of the
       * mutex's lock to release it.
       */
      inline void unlock() const;
};

/**
 * A mutex which can be used before ISABThreadInitialize is instatiated.
 *
 * This mutex is NOT recursive.
 *
 * This class should be obsolete when changing to a thread library better
 * than JTC.
 */
class ISABMutexBeforeInit {
public:
   ISABMutexBeforeInit();

   ~ISABMutexBeforeInit();

   /**
    * Locks the mutex. If the mutex is already locked, the calling
    * thread blocks until the mutex is unlocked.
    */
   void lock();

   /**
    * Unlocks the mutex. This function is called by the owner of the
    * mutex's lock to release it.
    */
   void unlock();

private:
   /** 
    * This class is implemented with pthreads since JTC makes no guarantees
    * about behaviour before JTCInitialize has been constructed.
    */
   pthread_mutex_t m_mutex;
};

/**
 * This class works like ISABSync but for ISABMutexBeforeInit.
 *
 * This class should be obsolete when changing to a thread library better
 * than JTC.
 */
class ISABSyncBeforeInit {
public:
   ISABSyncBeforeInit( ISABMutexBeforeInit& mutex );

   ~ISABSyncBeforeInit();

private:
   ISABMutexBeforeInit& m_mutex;
};

/**
 * Used together with ISABCallOnce.
 */
typedef pthread_once_t ISABOnceFlag;

/**
 * Used to initialize a ISABOnceFlag.
 */
#define ISAB_ONCE_INIT PTHREAD_ONCE_INIT

/**
 * Used to call a function once in a thread safe manner.
 * This is useful especially for thread safe initialization,
 * see boost::call_once or man pthread_once for rationale.
 *
 * @todo Implement with boost or whatever thread library we 
 *       choose to use after JTC, currently implemented with
 *       pthread.
 */
void ISABCallOnce( void (*func)(), ISABOnceFlag& flag );

/**
 * Thread specific storage.
 *
 * Implemented with pthread at the moment since it should work
 * before a JTCInitialize object has been created.
 */
class ISABTSS {
public:

   /**
    * The key type. 
    * Keys are used to access thread specific storage.
    */
   typedef pthread_key_t TSSKey;

   /**
    * Creates a key for thread specific storage.
    */
   static TSSKey createKey( void (*cleanupFunc)(void*) );

   /**
    * Deletes a key.
    */
   static void deleteKey( TSSKey key );
      
   /**
    * Gets the value for a key for the currently executing thread. 
    * The value will be NULL if not set.
    */
   static void* get( TSSKey key );

   /**
    * Sets the value for a key for the currently executing thread.
    */
   static void set( TSSKey key, const void* value );
};

// inlines

////////////////////////////////////////////////////////////////////////
// ISABThreadInitialize
////////////////////////////////////////////////////////////////////////


ISABThreadInitialize::
ISABThreadInitialize()
{
}


ISABThreadInitialize::
~ISABThreadInitialize()
{
}

 
void
ISABThreadInitialize::
waitTermination()
{
   JTCInitialize::waitTermination();
}



////////////////////////////////////////////////////////////////////////
// ISABMonitor
////////////////////////////////////////////////////////////////////////


ISABMonitor::
ISABMonitor()
{
}

ISABMonitor::
~ISABMonitor()
{
}

void
ISABMonitor::
wait()
{
   while( true ){
      try{
         JTCMonitor::wait();
         return;
      }
      catch( const JTCException& e ){
         mc2log << warn << "ISABMonitor::wait " << endl << e << endl;
      }
   }
}

void
ISABMonitor::
wait(long timeout)
{
   while( true ){
      try{
         JTCMonitor::wait(timeout);
         return;
      }
      catch( const JTCException& e ){
         mc2log << warn << "ISABMonitor::wait(long) " << endl << e << endl; 
      }
   }
}

void
ISABMonitor::
notify()
{
   JTCMonitor::notify();
}

void
ISABMonitor::
notifyAll()
{
   JTCMonitor::notifyAll();
}


////////////////////////////////////////////////////////////////////////
// ISABSync
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// ISABThreadGroup
////////////////////////////////////////////////////////////////////////

ISABThreadGroup::ISABThreadGroup(const char* name) 
      : JTCThreadGroup( name )
{
}


ISABThreadGroup::ISABThreadGroup(ISABThreadGroup* parent, 
                                 const char* name )
      : JTCThreadGroup( parent, name )
{
}


ISABThreadGroup::~ISABThreadGroup() {
}


void
ISABThreadGroup::uncaughtException(JTCThreadHandle t, 
                                   const JTCException& e)
{
   mc2log << fatal << "ISABThreadGroup::uncaughtJTCException " << e << endl;
   SysUtility::dumpProcessInfo();
   exit(2);
}


void
ISABThreadGroup::uncaughtException(JTCThreadHandle t) {
   mc2log << fatal << "ISABThreadGroup::uncaughtException " << strerror(errno) 
          << endl;
   SysUtility::dumpProcessInfo();
   exit(2);
}

inline ISABThreadGroupHandle
ISABThreadGroup::getParent() const {
   return ISABThreadGroupHandle( 
      static_cast< ISABThreadGroup* > ( JTCThreadGroup::getParent().get() 
                                        ) );
}

inline int
ISABThreadGroup::activeCount() const {
   return JTCThreadGroup::activeCount();
}

////////////////////////////////////////////////////////////////////////
// ISABMutex
////////////////////////////////////////////////////////////////////////


void
ISABMutex::lock() const {
   JTCMutex::lock();
}


void
ISABMutex::unlock() const {
   JTCMutex::unlock();
}


////////////////////////////////////////////////////////////////////////
// ISABRecursiveMutex
////////////////////////////////////////////////////////////////////////


void
ISABRecursiveMutex::lock() const {
   JTCRecursiveMutex::lock();
}


void
ISABRecursiveMutex::unlock() const {
   JTCRecursiveMutex::unlock();
}


#endif // ISABTHREAD_H

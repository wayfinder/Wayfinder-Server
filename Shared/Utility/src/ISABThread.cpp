/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ISABThread.h"
#include "MC2Exception.h"
#include "TimeUtility.h"

////////////////////////////////////////////////////////////////////////
// ISABThread
////////////////////////////////////////////////////////////////////////

void runHook( JTCThread* thread )
{
   try{
      thread->run();
   }
   catch(const JTCException& e ){
      mc2log << fatal << "ISABThread::run unknown exception " << endl;
      throw(e);
   }
   catch ( const MC2Exception& e ) {
      mc2log << fatal << "ISABThread::run uncaught exception " 
             << e << endl;
      throw(e);
   }

}

ISABThread::
//ISABThread(JTCThreadGroupHandle& group, const char* name) 
ISABThread( ISABThreadGroup* group, const char* name) 
      : JTCThread( *( group ? 
                      (defaultThreadGroup = NULL),
                      (threadGroupHandle = 
                       new JTCThreadGroupHandle(group)) 
                      : 
                      (threadGroupHandle = 
                       new JTCThreadGroupHandle( (defaultThreadGroup = 
                                                  new ISABThreadGroup())) )
                      ) )
{
	terminated = false;   
   setRunHook( runHook );
}


ISABThread::
~ISABThread()
{
   delete threadGroupHandle; // defaultThreadGroup is deleted automagi
}


void
ISABThread::
start()
{
   try{
      JTCThread::start();
   }
   catch(const JTCException& e ) {
      if ( dynamic_cast<const JTCOutOfMemoryError*>(&e) != NULL ) {
         mc2log << fatal << "[ISABThread]: No mem "
                << e << endl;
         SysUtility::dumpProcessInfo();
         *((byte*)(NULL)) = 1;
         exit(1);
      } else {
         mc2log << warn << "ISABThread::start unknown exception "
                << e << endl;
      }
   }
}


void
ISABThread::
run()
{
   JTCThread::run();
}


bool
ISABThread::
isAlive() const
{
   return JTCThread::isAlive();
}


void
ISABThread::
join()
{
   try{
      JTCThread::join();
   }
   catch(const JTCException& e ){
      mc2log << warn << "ISABThread::join unknown exception "
             << e << endl;
   }
}


void
ISABThread::
join(long millis, int nanos )
{
   try{
      JTCThread::join(millis, nanos);
   }
   catch(const JTCException& e ){
       mc2log << warn << "ISABThread::join(millis) uknown exception "
              << e << endl;
   }
}


void
ISABThread::
setPriority(int newPri)
{
   try{
      JTCThread::setPriority(newPri);
   }
   catch(const JTCException& e ){
      mc2log << warn << "ISABThread::setPriority uknown exception "
             << e << endl;
   }
}


int
ISABThread::
getPriority() const
{
   try{
      return JTCThread::getPriority();
   }
   catch(const JTCException& e ){
      mc2log << warn << "ISABThread::getPriority uknown exception "
             << e << endl;      
      return -1;
   }
}


void
ISABThread::
terminate()
{
   terminated = true;
}

void
ISABThread::
sleep(long millis, int nano )
{
   // Make sure that nano doesn't contain millis.
   millis += nano/1000000;
   // if we have millis ignore the nano seconds
   if( millis > 0 ){
      uint32 start = TimeUtility::getCurrentTime();
      uint32 now = start;
      while ( long(now - start) < millis ) {
         try{
            JTCThread::sleep( millis - (now - start) ); 
            return;
         } 
         catch( const JTCInterruptedException& e ) {
            now = TimeUtility::getCurrentTime();
         }
         catch( const JTCException& e ){
            mc2log << warn << "ISABThread::sleep uknown exception (milli)"
                   << e << endl;
            return;
         }
      }
   }
   else{ // use the nano seconds
      uint32 micros = nano / 1000;
      uint32 start = TimeUtility::getCurrentMicroTime();
      uint32 now = start;
      while ( (now - start) < micros ) {
         try{
            JTCThread::sleep( 0, micros - (now - start) ); 
            return;
         } 
         catch( const JTCInterruptedException& e ) {
            now = TimeUtility::getCurrentMicroTime();
         }
         catch( const JTCException& e ){
            mc2log << warn << "ISABThread::sleep uknown exception (micro)"
                   << e << endl;
            return;
         }
      }
   }

}

void
ISABThread::milliSleep( int ms ) 
{
   sleep( ms );
}

////////////////////////////////////////////////////////////////////////
// ISABMutexBeforeInit
////////////////////////////////////////////////////////////////////////

ISABMutexBeforeInit::ISABMutexBeforeInit() {
   pthread_mutex_init( &m_mutex, NULL );
}

ISABMutexBeforeInit::~ISABMutexBeforeInit() {
   pthread_mutex_destroy( &m_mutex );
}

void ISABMutexBeforeInit::lock() {
   pthread_mutex_lock( &m_mutex );
}

void ISABMutexBeforeInit::unlock() {
   pthread_mutex_unlock( &m_mutex );
}

////////////////////////////////////////////////////////////////////////
// ISABSyncBeforeInit
////////////////////////////////////////////////////////////////////////

ISABSyncBeforeInit::ISABSyncBeforeInit( ISABMutexBeforeInit& mutex )
      : m_mutex( mutex ) {
   mutex.lock();
}
ISABSyncBeforeInit::~ISABSyncBeforeInit() {
   m_mutex.unlock();
}

void ISABCallOnce( void (*func)(), ISABOnceFlag& flag ) {
   pthread_once( &flag, func );
}


ISABTSS::TSSKey ISABTSS::createKey( void (*cleanupFunc)(void*) ) {
   TSSKey result;
   pthread_key_create( &result, cleanupFunc );
   return result;
}

void ISABTSS::deleteKey( TSSKey key ) {
   pthread_key_delete( key );
}

void* ISABTSS::get( TSSKey key ) {
   return pthread_getspecific( key );
}

void ISABTSS::set( TSSKey key, const void* value ) {
   pthread_setspecific( key, value );
}

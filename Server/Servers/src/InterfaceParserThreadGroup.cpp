/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InterfaceParserThreadGroup.h"
#include "ParserThreadGroup.h"
#include "ParserThread.h"
#include "InterfaceIO.h"
#include "InterfaceRequest.h"
#include "InterfaceFactory.h"
#include "ThreadRequestHandler.h"
#include "TimedOutSocketLogger.h"

InterfaceParserThreadGroup::
InterfaceParserThreadGroup( ServerTypes::servertype_t serverType,
                            const char* threadGroupName ,
                            uint32 minNbrThreads,
                            uint32 maxNbrThreads,
                            uint32 queueFullFactor,
                            uint32 queueOverFullFactor )
      : ParserThreadGroup( serverType, threadGroupName ),
        m_minNbrThreads( minNbrThreads ), m_maxNbrThreads( maxNbrThreads ),
        m_queueFullFactor( queueFullFactor ), m_queueFullNbr( 0 ),
        m_queueOverFullFactor( queueOverFullFactor ), 
        m_queueOverFullNbr( 0 ), m_nbrIdleThreads( 0 ),
        m_shutdownPipe( NULL )
{
   updateQueueNbr(); // Sets m_queueFullNbr and m_queueOverFullNbr
}


InterfaceParserThreadGroup::~InterfaceParserThreadGroup() {
   // WARNING: Don't ever delete threads! Is always NULL here. Leaving code for fun
   delete m_preCacheTileMapHandler.get();

   // Shouldn't be anything to delete in queue etc...

}


void
InterfaceParserThreadGroup::
start( ThreadRequestHandler* handler,
       InterfaceIO* interfaceIO,
       set< InterfaceFactory* > interfaceFactories,
       TimedOutSocketLogger* socketLogger) {

   setRequestHandler( handler );

   m_interfaceIO = interfaceIO;
   m_timedOutSocketLogger = socketLogger;
   m_interfaceFactories = interfaceFactories;

   // Create the minimum number of ParserThreads.
   for ( uint32 i = 0 ; i < m_minNbrThreads ; i++ ) {
      m_threads.push_back( spawnThread() );
   }
   updateQueueNbr(); // Sets m_queueFullNbr and m_queueOverFullNbr

   // Start the threads
   for ( threadHandleStructure_t::iterator it = m_threads.begin() ;
         it != m_threads.end() ; ++it ) {
      it->get()->start();
   }
}

void
InterfaceParserThreadGroup::shutDown() {
   ISABSync sync( m_irequestsMonitor );
   if ( !m_running ) {
      return;
   }

   m_running = false;

   // Stop input(s)
   mc2dbg << "[InterfaceParserThreadGroup] stop factories." << endl;
   for_each( m_interfaceFactories.begin(),
             m_interfaceFactories.end(),
             mem_fun( &InterfaceFactory::terminate ) );
   mc2dbg << "[InterfaceParserThreadGroup] shutdown interface io." << endl;     
   // Tell IO to kick idle IO
   m_interfaceIO->shutdownStarts();
   m_timedOutSocketLogger->shutdownStarts();
   mc2dbg << "[InterfaceParserThreadGroup] terminate thread request handler." << endl;     
   // Stop Push
   m_handler->terminate();

   mc2dbg << "[InterfaceParserThreadGroup] notify irequest monitor." << endl;     
   // Make sure all threads are awake
   m_irequestsMonitor.notifyAll();
   mc2dbg << "[InterfaceParserThreadGroup] terminate precache tile map handler." << endl;     
   m_preCacheTileMapHandler->terminate();
   m_preCacheRouteQueue.terminate();
   // See preCacheDone about setting m_preCacheTileMapHandler = NULL;
   m_shutdownPipe->notify();
}


bool 
InterfaceParserThreadGroup::returnAndGetInterfaceRequest( 
   InterfaceRequest* ireply,
   InterfaceRequest*& ireq, 
   ParserThreadHandle me )
{
   ISABSync sync( m_irequestsMonitor );
//    mc2dbg << "InterfaceParserThreadGroup::returnAndGetInterfaceRequest "
//           << ireply;
//    if ( ireply != NULL ) {
//       mc2dbg << " state " << InterfaceRequest::getStateAsString( 
//          ireply->getState() );
//    }
//    mc2dbg << " m_irequests.size() " << m_irequests.size() 
//           << " m_threads.size() " << m_threads.size() << endl;

   if ( ireply != NULL ) {
      switch ( ireply->getState() ) {
         case InterfaceRequest::Ready_To_IO_Reply:
            // Put into interFaceIO
            if ( !m_running ) {
               ireply->terminate();
            }
            m_interfaceIO->putInterfaceRequest( ireply );
            break;
         case InterfaceRequest::Done:
            handleDoneInterfaceRequest( ireply );
            break;

         case InterfaceRequest::Ready_To_IO_Request:
         case InterfaceRequest::Ready_To_Process:
         case InterfaceRequest::Error:
         case InterfaceRequest::Timeout_request:
         case InterfaceRequest::Timeout_reply:
         case InterfaceRequest::Uninitalized:
            mc2dbg2 << "InterfaceParserThreadGroup strange state "
                   << "of returned InterfaceRequest: "
                   << InterfaceRequest::getStateAsString( 
                      ireply->getState() ) << endl;
            handleDoneInterfaceRequest( ireply );
            ireq = NULL;
            break;
      }
   }

   // Get a InterfaceRequest to return.
   ireq = NULL;
   bool run = m_running;
   if ( m_running ) {
      if ( m_irequests.empty() ) {
         // Check if surplus thread
         ISABSync* msync = new ISABSync( m_monitor );
         if ( m_threads.size() > m_minNbrThreads ) {
            // Remove surplus thread
            run = false;
            removeParserThread( me );
            // Wake up a thread to check if it is surplus too
            m_irequestsMonitor.notify();
            updateQueueNbr(); // Sets m_queueFullNbr and m_queueOverFullNbr
            delete msync;
         } else {
            // Unlock m_monitor
            delete msync;
            m_nbrIdleThreads++;
            while ( m_irequests.empty() && m_running ) {
               // Wait for a wakeup
               m_irequestsMonitor.wait();
            }
            m_nbrIdleThreads--;
            if ( !m_irequests.empty() ) {
               ireq = m_irequests.front();
               m_irequests.pop_front();
            }
         }
      } else {
         ireq = m_irequests.front();
         m_irequests.pop_front();
      }
   } else {
      if ( !m_irequests.empty() ) {
         ireq = m_irequests.front();
         m_irequests.pop_front();
         run = true;
      } else {
         if ( m_interfaceIO->empty() ) {
            // Remove thread
            run = false;
            ISABSync* msync = new ISABSync( m_monitor );
            removeParserThread( me );
            if ( m_threads.empty() ) {
               mc2dbg << "[InterfaceParserThreadGroup] no more threads." << endl;
               mc2dbg << "[InterfaceParserThreadGroup] destroying handler and interface io." << endl;
               // All threads done and nothing more coming from IO
               // delete the last threads alive in m_handler
               // so we can continue in main after waitTermination.
               // Check if preCacheTileMapHandler is still running
               if ( m_preCacheTileMapHandler.get() == NULL ) {
                  delete m_handler;
               }
               // Delete IO thread(s) too
               delete m_interfaceIO;
               // And the logger as well
               delete m_timedOutSocketLogger;
            }
            updateQueueNbr(); // Sets m_queueFullNbr and m_queueOverFullNbr
            delete msync;
         } else { 
            // else wait for the remaining in m_interfaceIO
            ISABThread::yield();
            run = true;
         }
      }
   }
   // mc2log << "[InterfaceParserThreadGroup] " << __FUNCTION__ << " end." << endl;

//   ISABSync sync( m_irequestsMonitor );
//    m_irequestsMonitor.notify(); // Why wake another thread?
   
   return run;
}


int 
InterfaceParserThreadGroup::putInterfaceRequest( InterfaceRequest*& ireq ){
   ISABSync sync( m_irequestsMonitor );
   ISABSync* msync = new ISABSync( m_monitor );
   int reqiFull = getNbrOverQueueFull();
   int reqiOverFull = getNbrOverQueueOverFull();
   bool mustProcessNow = false;
//    mc2dbg << "InterfaceParserThreadGroup::putInterfaceRequest " << ireq
//           << " state " << InterfaceRequest::getStateAsString( 
//              ireq->getState() )
//           << " m_irequests.size() " << m_irequests.size() 
//           << " reqiFull " << reqiFull << " reqiOverFull " << reqiOverFull
//           << " m_threads.size() " << m_threads.size() << endl;
   delete msync;

   if ( ireq != NULL ) {
      switch ( ireq->getState() ) {
         case InterfaceRequest::Ready_To_IO_Reply:
         case InterfaceRequest::Ready_To_IO_Request:
            // Put into interFaceIO
            if ( !m_running ) {
               ireq->terminate();
            }
            m_interfaceIO->putInterfaceRequest( ireq );
            ireq = NULL;
            break;

         case InterfaceRequest::Done:
            handleDoneInterfaceRequest( ireq );
            ireq = NULL;
            break;

         case InterfaceRequest::Ready_To_Process:
            mustProcessNow = ireq->getPriority( this ) < 0;// High priority
            if ( reqiOverFull < 0 || mustProcessNow ) {
               ireq->startQueueTime();
               if ( mustProcessNow ) {
                  m_irequests.push_front( ireq );
               } else {
                  m_irequests.push_back( ireq );
               }
               ireq = NULL;
               // Wake up a thread to process it
               m_irequestsMonitor.notify();
            }
            break;

         case InterfaceRequest::Error:
         case InterfaceRequest::Timeout_request:
         case InterfaceRequest::Timeout_reply:
         case InterfaceRequest::Uninitalized:
            mc2dbg2 << "InterfaceParserThreadGroup strange state "
                   << "of put InterfaceRequest: "
                   << InterfaceRequest::getStateAsString( 
                      ireq->getState() ) << endl;
            handleDoneInterfaceRequest( ireq );
            ireq = NULL;
            break;
      }
   } // End if ireq != NULL

   msync = new ISABSync( m_monitor );
   if ( (reqiFull > 0 || m_nbrIdleThreads == 0) && m_irequests.size() > 0&&
        (m_threads.size() < m_maxNbrThreads || mustProcessNow) ) 
   {
      // Aha, full and not all threads started
      m_threads.push_back( spawnThread() );
      m_threads.back()->start();
      updateQueueNbr(); // Sets m_queueFullNbr and m_queueOverFullNbr
   }
   delete msync;

   return reqiFull;
}


void 
InterfaceParserThreadGroup::handleDoneInterfaceRequest( 
   InterfaceRequest* ireply )
{
   delete ireply;
}


uint32 
InterfaceParserThreadGroup::getMinNbrThreads() {
   ISABSync sync( m_monitor );
   return m_minNbrThreads;
}


uint32 
InterfaceParserThreadGroup::getMaxNbrThreads() {
   ISABSync sync( m_monitor );
   return m_maxNbrThreads;
}


void 
InterfaceParserThreadGroup::setNbrThreads( 
   uint32 minNbrThreads, uint32 maxNbrThreads )
{
   ISABSync sync( m_monitor );
   m_minNbrThreads = minNbrThreads;
   m_maxNbrThreads = maxNbrThreads;
}


void 
InterfaceParserThreadGroup::setQueueFactors( uint32 queueFullFactor, 
                                             uint32 queueOverFullFactor )
{
   ISABSync sync( m_monitor );
   m_queueFullFactor = queueFullFactor;
   m_queueOverFullFactor = queueOverFullFactor;
   updateQueueNbr();
}


InterfaceFactory* 
InterfaceParserThreadGroup::getFirstInterfaceFactory() {
   if ( m_interfaceFactories.empty() ) {
      return NULL;
   } else {
      return *m_interfaceFactories.begin();
   }
}


void 
InterfaceParserThreadGroup::updateQueueNbr() {
   m_queueFullNbr = m_threads.size() * m_queueFullFactor;
   m_queueOverFullNbr = (m_threads.size() + 1) * m_queueFullFactor * 
      m_queueOverFullFactor;
}


int
InterfaceParserThreadGroup::getNbrOverQueueFull() const {
   return int32(m_irequests.size()) - int32(m_queueFullNbr);
}


int 
InterfaceParserThreadGroup::getNbrOverQueueOverFull() const {
   return int32(m_irequests.size()) - int32(m_queueOverFullNbr);  
}

void
InterfaceParserThreadGroup::preCacheDone( ParserThreadHandle preCahce ) {
   ISABSync msync( m_monitor );
   if ( m_threads.empty() ) {
      // preCacheTileMapHandler is last so it is time to delete m_handler
      // and release the last threads.
      delete m_handler;
   }
   m_preCacheTileMapHandler = NULL;
}

JTCRecursiveMutex* getMutex( ISABMonitor& mon ) {
   // This is good and robust code ;-)
   // Assuming that the JTCRecursiveMutex is last in JTCMonitor
   return (JTCRecursiveMutex*)(((byte*)&mon) + sizeof( ISABMonitor ) - 
                               sizeof( JTCRecursiveMutex ) );
}

ostream& printMonitorStatus( ostream& s, ISABMonitor& mon, 
                             const MC2String& name ) {
   JTCRecursiveMutex* mut = getMutex( mon );
   bool locked = mut->get_owner() != JTCThreadId();
   s << " " << name << " is " << ( locked ? "locked" : "unlocked" );
   if ( locked ) {
      s << " by " << mut->get_owner();
   }
   return s;
}

void
InterfaceParserThreadGroup::printStatus( ostream& s ) {
   {
      printMonitorStatus( s, m_irequestsMonitor, "m_irequestsMonitor" );
      printMonitorStatus( s, m_monitor, "m_monitor" );
      s << endl;
   }

   ISABSync syncI( m_irequestsMonitor );
   ISABSync syncM( m_monitor );

   uint32 cacheRNbr = 0;
   uint32 cacheRSize = getCache()->getCurrentRemovedSize( &cacheRNbr );
   
   s << " cache " << getCache()->getCurrentElementcount() << " " 
     << getCache()->getCurrentSize() << " bytes"
     << " removed in cache " << cacheRNbr << " " << cacheRSize << " bytes"
     << " nbr threads " << m_threads.size() << " idle " << m_nbrIdleThreads
     << " nbr irequests " << m_irequests.size()
     << " nbr requests in io " << m_interfaceIO->size()
     << " edgenodesData size " << getEdgenodesDataSize()
     << " UserSessionCache size " << getUserSessionCacheSize()
     << " UserLoginCache size " << getUserLoginCacheSize()
     << " RouteStorage size " << getRouteStorageSize()
     << " NewsMap size " << getNewsMapSize()
       ;
}

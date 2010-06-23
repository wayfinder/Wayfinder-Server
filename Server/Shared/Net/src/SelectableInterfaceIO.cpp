/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SelectableInterfaceIO.h"
#include "SelectableSelector.h"
#include "SelectableInterfaceRequest.h"
#include "InterfaceHandleIO.h"
#include "DeleteHelpers.h"
#include "TimeUtility.h"

SelectableInterfaceIO::SelectableInterfaceIO( 
   InterfaceHandleIO* group )
      : InterfaceIO( group ),
        m_selector( new SelectableSelector() )
{
   m_nbrRequests = 0;
   m_shutdown = false;
   m_thread = new SelectableInterfaceIOThread( m_selector.get(), this );
   m_thread->start();
}


SelectableInterfaceIO::~SelectableInterfaceIO() {
   // Shut down the SelectableInterfaceIOThread
   m_thread->terminate();
   m_selector->notify();
   while ( m_thread->isAlive() ) {
      ISABThread::yield();
   }
   // Delete all remaining in m_addedSelectable (should not be any)
   STLUtility::deleteValues( m_addedSelectable );
   mc2dbg << "[SelectableInterfaceIO] dead" << endl;
}


void
SelectableInterfaceIO::putInterfaceRequest( InterfaceRequest* ireq ) {
   MC2_ASSERT( dynamic_cast< SelectableInterfaceRequest* > ( ireq ) );
   SelectableInterfaceRequest* irequest = 
      static_cast< SelectableInterfaceRequest* > ( ireq );
   if ( irequest->wantRead() || irequest->wantWrite() ) {
      m_mutex.lock();
//       mc2dbg << "SelectableInterfaceIO::putInterfaceRequest " << ireq << endl;
      m_addedSelectable.insert( irequest );
      ++m_nbrRequests;
      m_selector->notify();
      m_mutex.unlock();
   } else {
      mc2log << warn << "SelectableInterfaceIO::putInterfaceRequest "
             << "ireq doesn't want any IO returning it to done in group "
             << "ireq in state " << InterfaceRequest::getStateAsString( 
                irequest->getState() ) << endl;
      m_group->handleDoneInterfaceRequest( irequest );
   }
}


bool
SelectableInterfaceIO::empty() const {
   m_mutex.lock();
   bool res = (m_nbrRequests == 0);
   m_mutex.unlock();
   return res;
}

uint32
SelectableInterfaceIO::size() const {
   m_mutex.lock();
   uint32 res = m_nbrRequests;
   m_mutex.unlock();
   return res;
}

void
SelectableInterfaceIO::shutdownStarts() {
   m_mutex.lock();
   m_shutdown = true;
   // Wakeup
   m_selector->notify();
   m_mutex.unlock();
}


void
SelectableInterfaceIO::putReady( selSet& readySelectable,
                                 selSet& addedSelectable,
                                 bool& shutdown )
{
   for ( selSet::iterator it = readySelectable.begin() ; 
         it != readySelectable.end() ; ++it )
   {
      InterfaceRequest* ireq = *it;
//        mc2dbg << "SelectableInterfaceIO::putReady returning "
//               << ireq << endl;
      int overLoad = m_group->putInterfaceRequest( ireq );
      if ( ireq != NULL ) {
         // Overloaded
         (*it)->handleOverloaded( overLoad );
         if ( (*it)->getState() != InterfaceRequest::Ready_To_IO_Reply ) {
            m_group->handleDoneInterfaceRequest( (*it) );
         } else {
            m_mutex.lock();
            m_addedSelectable.insert( (*it) );
            ++m_nbrRequests;
            m_mutex.unlock();
         }
      }
   }

   m_mutex.lock();
   m_nbrRequests -= readySelectable.size();
   // Return the added Requests
   addedSelectable.swap( m_addedSelectable );
   m_addedSelectable.clear();
   shutdown = m_shutdown;
   m_mutex.unlock();
}



SelectableInterfaceIO::SelectableInterfaceIOThread::
SelectableInterfaceIOThread( SelectableSelector* selector,
                             SelectableInterfaceIO* io )
      : ISABThread( NULL, "SelectableInterfaceIOThread" ),
        m_selector( selector ), m_io( io )
{}


SelectableInterfaceIO::SelectableInterfaceIOThread::
~SelectableInterfaceIOThread()
{
   // Delete all remaining in m_selectable (should not be any)
   for ( selReqSet::iterator it = m_selectable.begin() ; 
         it != m_selectable.end() ; ++it )
   {
      delete *it;
   }
}


void 
SelectableInterfaceIO::SelectableInterfaceIOThread::run() {
   selSet readySelectable;
   selSet addedSelectable;
   const uint32 maxTimeout = MAX_UINT32 / 1000;
   uint32 timeout = maxTimeout;
   int res = 0;
   set<Selectable*> readReady;
   set<Selectable*> writeReady;
   bool shutdown = false;

   while ( !terminated ) {
      // Return ready get new
      m_io->putReady( readySelectable, addedSelectable, shutdown );
      readySelectable.clear();
      if ( shutdown ) {
         // Find idle requests and return them
         for ( selReqSet::iterator it = m_selectable.begin() ; 
               it != m_selectable.end() ; /*In code*/ )
         {
            if ( (*it)->isIdle() ) {
               (*it)->timedout();
               m_selector->removeSelectable( (*it)->getSelectable(),
                                             true, true );
               readySelectable.insert( *it );
               m_selectable.erase( it++ );
            } else {
               ++it;
            }
         }
         selSet addedSelectableTmp;
         m_io->putReady( readySelectable, addedSelectableTmp, shutdown );
         readySelectable.clear();
         addedSelectable.insert( addedSelectableTmp.begin(), 
                                 addedSelectableTmp.end() );
      }
      uint32 now = TimeUtility::getRealTime();
      for ( selSet::iterator it = addedSelectable.begin() ; 
            it != addedSelectable.end() ; ++it )
      {
         (*it)->setIOStartTime( now );
         (*it)->setTotalUsedIOTime( 0 );
         m_selector->addSelectable( (*it)->getSelectable(), 
                                    (*it)->wantRead(), (*it)->wantWrite());
         m_selectable.insert( *it );
      }
      addedSelectable.clear();

      // Calculate timeout
      timeout = maxTimeout;
      selReqSet::iterator firstIt = m_selectable.begin();
      if ( firstIt != m_selectable.end() ) {
         timeout = MIN( (*firstIt)->getTimeout() - 
                        (*firstIt)->getUsedIOTime(), maxTimeout );
      }

      // Do the select
      uint32 startTime = TimeUtility::getCurrentTime();
//      mc2dbg << "m_selectable.size " << m_selectable.size() << endl;
      res = m_selector->select( timeout*1000 /*us*/, 
                                readReady, writeReady );
      uint32 endTime = TimeUtility::getCurrentTime();
      uint32 timeUsed = endTime - startTime;
      // Update now after select
      now = TimeUtility::getRealTime();

      // Check all irequests
      set< SelectableInterfaceRequest* > reorderSet;
      for ( selReqSet::iterator it = m_selectable.begin() ; 
            it != m_selectable.end() ; /* In code */ )
      {
         bool remove = false;
         bool reorder = false; // If not remove then perhaps reorder
                              //  erase and insert to get in right place
         // Add used time
//          mc2dbg << " Setting used time " << ((*it)->getUsedIOTime() + 
//                                              timeUsed)
//                 << " timeout " << (*it)->getTimeout() << endl;
         (*it)->setUsedIOTime( (*it)->getUsedIOTime() + timeUsed );
         // Check read & write
         if ( (*it)->wantRead() || (*it)->wantWrite() ) {
            bool readyRead = false;
            bool readyWrite = false;
            set<Selectable*>::iterator findIt;
            if ( (*it)->wantRead() &&
                 (findIt = readReady.find( (*it)->getSelectable() ) ) != 
                 readReady.end() ) 
            {
               readyRead = true;
               // Handles below remove from ready
               readReady.erase( findIt );
               // Restart IO timeout
               (*it)->setTotalUsedIOTime( (*it)->getTotalUsedIOTime() +
                                          (*it)->getUsedIOTime() );
               (*it)->setUsedIOTime( 0 );
            }
            if ( (*it)->wantWrite() &&
                 (findIt = writeReady.find( (*it)->getSelectable() ) ) != 
                 writeReady.end() ) 
            {
               readyWrite = true;
               // Handles below remove from ready
               writeReady.erase( findIt );
               // Restart IO timeout
               (*it)->setTotalUsedIOTime( (*it)->getTotalUsedIOTime() +
                                          (*it)->getUsedIOTime() );
               (*it)->setUsedIOTime( 0 );
            }
            if ( readyRead || readyWrite ) {
               (*it)->handleIO( readyRead, readyWrite );
               reorder = true;
               // Reinsert into selector
               m_selector->addSelectable( 
                  (*it)->getSelectable(), 
                  (*it)->wantRead(), 
                  (*it)->wantWrite() );
            }
         }

         // Check for state change
         if ( (*it)->getState() == InterfaceRequest::Ready_To_IO_Request ||
              (*it)->getState() == InterfaceRequest::Ready_To_IO_Reply )
         {
            // Keep it, but check timeout
            if ( (*it)->getUsedIOTime() > (*it)->getTimeout() ||
                 now - (*it)->getIOStartTime() > (*it)->getTotalTimeout() )
            {
               // Timeout!
               (*it)->timedout();
               remove = true;
            }
         } else { // Return it
            remove = true;
         }

         if ( remove || reorder ) {
            if ( remove ) {
               // Remove from selector too
               m_selector->removeSelectable( (*it)->getSelectable(),
                                             true, true );
               readySelectable.insert( *it );
            } else {
               reorderSet.insert( *it );
            }
            m_selectable.erase( it++ );
         } else {
            ++it;
         }
      }

      // Re-insert reorderSet into main set
      if ( ! reorderSet.empty() ) {
         m_selectable.insert( reorderSet.begin(), reorderSet.end() );
         reorderSet.clear();
      }

      // Check for leftovers in readReady and writeReady
      // Hmm, they are not needed by any of the irequests, just forget
      // about them
      readReady.clear();
      writeReady.clear();
   }

   mc2dbg << "[SelectableInterfaceIO]::SelectableInterfaceIOThread::run ends"
          << endl;
}

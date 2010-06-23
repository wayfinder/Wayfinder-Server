/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SelectableSelector.h"
#include "sockets.h"

#include "NotifyPipe.h"

namespace {
typedef map<Selectable*, DebugClock> TimeMap;
typedef SelectableSelector::selSet selSet;

// remove from set
void removeSelectable( Selectable* sel, 
                       selSet& theSet ) {
   selSet::iterator findIt = theSet.find( sel );
   if ( findIt != theSet.end() ) {
      // Found remove
      theSet.erase( findIt );
   }
}

void removeSelectable( Selectable::selectable sel,
                       selSet& theSet ) {
   selSet::iterator it = theSet.begin();
   selSet::iterator itEnd = theSet.end();
   for (; it != itEnd; ++it ) {
      if ( (*it)->getSelectable() == sel ) {
         theSet.erase( it );
         break;
      }
   }
}

}

SelectableSelector::SelectableSelector():
   m_notifyPipe( new NotifyPipe() ) {
   m_terminated = false;

   // Initialize the system sets
   clearAndSet();
}


SelectableSelector::~SelectableSelector() {
}


SelectableSelector::SelectStatus
SelectableSelector::select( int32 timeout, 
                            selSet& readReady, 
                            selSet& writeReady )
{
   int res = -1;
   m_mutex.lock();
   // Set up the fd-sets
#ifdef COUNT_HIGHEST_SEL
   Selectable::selectable maxSel = 
#endif
      clearAndSet();
   m_mutex.unlock();

   // Set timeout and select
#ifndef _WIN32
   int nfds = 
#   ifdef COUNT_HIGHEST_SEL
      maxSel + 1;
#   else
   getdtablesize();
#   endif
#else
   int nfds = 0; // Ignored in WIN32
#endif

   if ( timeout < 0 ) {
      // Without timeout
      res = ::select( nfds, &m_readfds, &m_writefds, NULL, NULL );
   } else {
      // With timeout
      struct timeval tv;
      tv.tv_sec = timeout / 1000000;
      tv.tv_usec = timeout % 1000000;
      res = ::select( nfds, &m_readfds, &m_writefds, NULL, &tv );
   }

   m_mutex.lock();
   // Check status of select and:
   if ( res < 0 ) {
      //  If error clearAndSet and return error
      mc2log << warn << "SelectableSelector::select select failed "
             << strerror( errno ) << endl;
      clearAndSet();
      res = ERROR;
   } else if ( res == 0 ) {
      //  If timeout return timeout
      if ( timeout < 0 ) {
         mc2log << warn << "SelectableSelector::select no timeout select "
                << "and select returns 0?" << endl;
      }
      res = TIMEOUT;
   } else if ( res > 0 ) {
      //  If selected then find and move Selectables to readReady and 
      //     writeReady also clr in fd_sets
      int nbrFound = 0; // Not more than res ready selectables

      for ( selSet::iterator it = m_readSelectables.begin() ;
            it != m_readSelectables.end() && nbrFound < res ; /*In code*/ )
      {
         if ( FD_ISSET( (*it)->getSelectable(), &m_readfds ) ) {
            // Found ready selectable
            nbrFound++;
            readReady.insert( *it );
            m_readSelectables.erase( it++ );
         } else {
            ++it;
         }
      }
      for ( selSet::iterator it = m_writeSelectables.begin() ;
            it != m_writeSelectables.end() && nbrFound < res ; /*In code*/)
      {
         if ( FD_ISSET( (*it)->getSelectable(), &m_writefds ) ) {
            // Found ready selectable
            nbrFound++;
            writeReady.insert( *it );
            m_writeSelectables.erase( it++ );
         } else {
            ++it;
         }
      }

      // Notify pipe
      if ( FD_ISSET( m_notifyPipe->getSelectable(), &m_readfds ) ) {
         // Notified!: clear pipe
         m_notifyPipe->consumeAll();
      }

      res = OK;

   }

   m_mutex.unlock();   

   return static_cast<SelectStatus>( res );
}
                            

void
SelectableSelector::terminate() {
   m_terminated = true;
   notify();
}


void
SelectableSelector::addSelectable( Selectable* sel, 
                                   bool readSelect, bool writeSelect,
                                   bool doNotify )
{
   m_mutex.lock();
   if ( readSelect ) {
      m_readSelectables.insert( sel );
#ifdef DEBUG_2
      // so we dont add more than one selectable
      selSet::const_iterator it = m_readSelectables.begin();
      selSet::const_iterator itEnd = m_readSelectables.end();
      for (; it != itEnd; ++it ) {
         selSet::const_iterator otherIt = m_readSelectables.begin();
         for ( ; otherIt != itEnd; ++otherIt ) {
            if ( it != otherIt ) {
               MC2_ASSERT((*it)->getSelectable() != (*otherIt)->getSelectable() );
            }
         }
      }
#endif
   }
   if ( writeSelect ) {
      m_writeSelectables.insert( sel );
#ifdef DEBUG_2
      // so we dont add more than one selectable
      selSet::const_iterator it = m_writeSelectables.begin();
      selSet::const_iterator itEnd = m_writeSelectables.end();
      for (; it != itEnd; ++it ) {
         selSet::const_iterator otherIt = m_writeSelectables.begin();
         for (; otherIt != itEnd; ++otherIt ) {
            if ( it != otherIt ) {
               MC2_ASSERT((*it)->getSelectable() != (*otherIt)->getSelectable() );
            }
         }
      }
#endif
   }
   if ( doNotify ) {
      notify();
   }
   m_mutex.unlock();
}


void 
SelectableSelector::removeSelectable( Selectable::selectable sel,
                                      bool readSelect, bool writeSelect,
                                      bool doNotify )
{
   m_mutex.lock();
   if ( readSelect ) {
      ::removeSelectable( sel, m_readSelectables );
   }

   if ( writeSelect ) {
      ::removeSelectable( sel, m_writeSelectables );
   }
   if ( doNotify ) {
      notify();
   }
   m_mutex.unlock();
}

void
SelectableSelector::removeSelectable( Selectable* sel,
                                      bool readSelect, bool writeSelect,
                                      bool doNotify )
{
   m_mutex.lock();
   if ( readSelect ) {
      ::removeSelectable( sel, m_readSelectables );
   }

   if ( writeSelect ) {
      ::removeSelectable( sel, m_writeSelectables );
   }
   if ( doNotify ) {
      notify();
   }
   m_mutex.unlock();
}


void 
SelectableSelector::notify() {
   // write to notifyPipe, pipe is nonblocking so no way this should block
   if ( ! m_notifyPipe->notify() ) {
      mc2log << warn
             << "[SelectableSelector]: notification pipe write error: "
             << strerror( errno ) << endl;
   } 
}


bool 
SelectableSelector::empty() {
   m_mutex.lock();
   bool res = m_readSelectables.empty() && m_writeSelectables.empty();
   m_mutex.unlock();
   return res;
}


// ----------- Internal methods --------------------


#ifdef COUNT_HIGHEST_SEL
Selectable::selectable
#else
void
#endif
SelectableSelector::clearAndSet() {
   FD_ZERO( &m_readfds );
   FD_ZERO( &m_writefds );
#ifdef COUNT_HIGHEST_SEL
   Selectable::selectable maxSel = m_notifyPipe->getSelectable();
#endif

   FD_SET( m_notifyPipe->getSelectable(), &m_readfds );
   for ( selSet::iterator it = m_readSelectables.begin() ;
         it != m_readSelectables.end() ; ++it )
   {
//      mc2dbg << "adding read-fd " << (*it)->getSelectable() << endl;
      FD_SET( (*it)->getSelectable(), &m_readfds );
#ifdef COUNT_HIGHEST_SEL
//       if ( (*it)->getSelectable() > maxSel ) {
//          maxSel = (*it)->getSelectable();
//       }
      maxSel |= (*it)->getSelectable();
#endif
   }
   for ( selSet::iterator it = m_writeSelectables.begin() ;
         it != m_writeSelectables.end() ; ++it )
   {
//      mc2dbg << "adding write-fd " << (*it)->getSelectable() << endl;
      FD_SET( (*it)->getSelectable(), &m_writefds ); 
#ifdef COUNT_HIGHEST_SEL
//       if ( (*it)->getSelectable() > maxSel ) {
//          maxSel = (*it)->getSelectable();
//       }
      maxSel |= (*it)->getSelectable();
#endif
   }
#ifdef COUNT_HIGHEST_SEL
   return maxSel;
#endif
}

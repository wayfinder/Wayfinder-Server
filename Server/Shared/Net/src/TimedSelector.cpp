/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TimedSelector.h"

#include <typeinfo>

TimedSelector::TimedSelector()
{
}

TimedSelector::~TimedSelector()
{

}

void TimedSelector::select( SelectableSet& timeoutSet )
{

   if ( m_selector.empty() ) {
      mc2dbg << "[TimedSelector]: selector empty!" << endl;
      MC2_ASSERT( false );
      //
      // If selector is empty, we wait for a notification to m_selector
      //
      SelectableSelector::selSet notUsed;
      m_selector.select( -1, notUsed, notUsed );
      return;
   }

   MC2_ASSERT( ! m_timeoutMap.empty() );

   // get minimum timeout from the set,
   // which would be the first item in the set due
   // to the timeout sort.
   int32 timeout = (*m_timeoutMap.begin()).second->getTimeLeft();

   // if we have a timeout (in milliseconds), 
   // convert it to microseconds
   if ( timeout != -1 ) {
      timeout *= 1000; // convert to microseconds
   }

   SelectableSelector::selSet readReady, writeReady;

   mc2dbg4 << "[TimedSelector]: timeout: " << timeout << endl;

   SelectableSelector::SelectStatus
      selectStatus = m_selector.select( (int32)timeout,
                                        readReady, writeReady );

   switch ( selectStatus ) {
   case SelectableSelector::OK:
      checkReadAndWrites( readReady, writeReady, timeoutSet );
      break;
   case SelectableSelector::TIMEOUT: {
      mc2dbg2 << "[TimedSelector]: select timed out" << endl;
      //
      // A selector timed out, check timeout items
      // and call handleTimeout.

      int32 curTime = TimeUtility::getCurrentTime();
      TimeoutMap::iterator it = m_timeoutMap.begin();
      TimeoutMap::iterator itEnd = m_timeoutMap.upper_bound( curTime );
      for (; it != itEnd; ++it ) {
         timeoutSet.insert( (*it).second );
      }
   }
      break;
   case SelectableSelector::ERROR:
      mc2dbg2 << fatal << "[TimedSelector]: error from select!" << endl;
      // 
      // Any error from select is bad!
      //
      MC2_ASSERT( selectStatus != SelectableSelector::ERROR );
      break;
   }

   //
   // If we had any timeout in a selectable;
   // call handleTimeout and remove it from select
   //
   SelectableSet::iterator it = timeoutSet.begin();
   SelectableSet::iterator itEnd = timeoutSet.end();
   for (; it != itEnd; ++it ) {
      (*it)->handleTimeout();
      removeSelectable( *it );
   }
}

void TimedSelector::addSelectable( TimedSelectable* select )
{
   MC2_ASSERT( select );

   m_selector.addSelectable( select,
                             select->wantRead(), select->wantWrite(),
                             false ); // no notify
   m_timeoutMap.insert( make_pair( select->getAbsoluteTime(), select ) );
}

void TimedSelector::removeSelectable( TimedSelectable* select )
{
   MC2_ASSERT( select );

   m_selector.removeSelectable( select, true, true,
                                false ); // no notify
   
   TimeoutMap::iterator it = m_timeoutMap.begin();
   TimeoutMap::iterator itEnd = m_timeoutMap.end();
   for (; it != itEnd; ++it ) {
      if ( (*it).second == select ) {
         m_timeoutMap.erase( it );
         break;
      }
   }
}

void TimedSelector::removeSelectable( Selectable::selectable sel ) 
{
   m_selector.removeSelectable( sel, true, true,
                                false ); // no notify

   TimeoutMap::iterator it = m_timeoutMap.begin();
   TimeoutMap::iterator itEnd = m_timeoutMap.end();
   for (; it != itEnd; ++it ) {
      if ( (*it).second->getSelectable() == sel ) {
         m_timeoutMap.erase( it );
         break;
      }
   }
}

void TimedSelector::updateTimeout( TimedSelectable& selectable ) 
{
   TimeoutMap::iterator it = m_timeoutMap.begin();
   TimeoutMap::iterator itEnd = m_timeoutMap.end();
   for (; it != itEnd; ++it ) {
      if ( (*it).second == &selectable ) {
         m_timeoutMap.erase( it );
         break;
      }
   }
   // was the selectable found in the timeout map?
   // then readd it 
   if ( it != itEnd ) {
      addSelectable( &selectable );
   }
}

void TimedSelector::checkReadAndWrites( SelectableSelector::selSet& readReady,
                                        SelectableSelector::selSet& writeReady,
                                        SelectableSet& timeoutSet ) 
{
   // this case would be very strange...
   if ( readReady.empty() && writeReady.empty() ) {
      return;
   }
   
   typedef std::vector< TimedSelectable* > SelVector;
   // these selectables should be readded 
   // to selectables again with updated timeout 
   SelVector readdSelectables;

   //
   // Check each timeout selectable and see if
   // it is ready for read or write.
   //
   TimeoutMap::iterator it = m_timeoutMap.begin();
   TimeoutMap::iterator itEnd = m_timeoutMap.end();
   for (; it != itEnd ; ++it ) {

      TimedSelectable* selectable = (*it).second;

      //
      // Determine read or write ready and call handler
      //
      bool read = readReady.find( selectable ) != readReady.end();
      bool write = writeReady.find( selectable ) != writeReady.end();
      if ( read || write ) {
         mc2dbg2 << "selectable ready("<< selectable->getSelectable()
                << "): " << typeid( *selectable ).name() << endl;
         selectable->handleIO( read, write );
      }

      //
      // If the selectable timed out; 
      // handleTimeout and add to timeout list 
      // else add selectable to selector again.
      //
      if ( selectable->timedOut() ) {
         timeoutSet.insert( selectable );
      } else {
         if ( read || write ) {
            // this selector needs to update its timeout 
            // so we add it to our readd vector here and
            // update time after the loop
            readdSelectables.push_back( selectable );
         } else {
            m_selector.addSelectable( selectable,
                                      selectable->wantRead(),
                                      selectable->wantWrite(), 
                                      false ); // no notify
         }         
      }
    
   }

   //
   // Readd selectables by:
   // Removing it from timeoutMap and then readd it.
   // This way the absolute time gets updated in the timeoutMap
   //
   SelVector::iterator selIt = readdSelectables.begin();
   SelVector::iterator selItEnd = readdSelectables.end();
   for (; selIt != selItEnd; ++selIt ) {
      removeSelectable( *selIt );
      addSelectable( *selIt );
   }

}

void TimedSelector::getSelectables( SelectableSet& selectables )
{
   TimeoutMap::iterator it = m_timeoutMap.begin();
   TimeoutMap::iterator itEnd = m_timeoutMap.end();
   for (; it != itEnd; ++it ) {
      selectables.insert( (*it).second );
   }
}

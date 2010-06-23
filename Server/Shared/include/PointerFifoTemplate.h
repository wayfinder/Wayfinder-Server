/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POINTERFIFOTEMPLATE_H
#define POINTERFIFOTEMPLATE_H

#include "config.h"

#include "ISABThread.h"
#include "SelectableSelector.h"

#include <queue>
#include <vector>

/**
 *    An implementation of a queue that uses the
 *    first-in-first-out algorithm. The methods are protected with
 *    a monitor to make them safe. The template uses pointers to
 *    the specified class to be able to return NULL.
 *    <br>
 */
template<class ElementT, typename Notifier = SelectableSelector> 
class PointerFifoTemplate: private queue<ElementT*> {
public:
   
   typedef typename queue<ElementT*>::size_type SizeType;
   typedef ElementT* value_type;

   /**
    *    Constructor. Sets some internal variables to their
    *    start values.
    *    @param selector A selector that should be
    *                          woken up when something is enqueued.
    */
   PointerFifoTemplate(Notifier* selector = NULL) {
      m_wakeUp     = false;
      m_terminated = false;
      m_selector = selector;
   }
   
   /**
    *    Delete contents of the queue and what it points to.
    */
   virtual ~PointerFifoTemplate() {
      ISABSync synchronized(m_monitor);
      // Delete the contents of the queue.
      while ( !queue<ElementT*>::empty() ) {
         ElementT* element = queue<ElementT*>::front();
         delete element;
         queue<ElementT*>::pop();
      }
   }

   /**
    *    Sets a socketreceiver to alert when new stuff comes into
    *    the queue.
    */
   void setSelectNotify(Notifier* selector) {
      ISABSync synchronized(m_monitor);
      m_selector = selector;
   }
   
   /**
    *    Returns the number of elements in the queue.
    *    @return The number of elements in the queue.
    */
   SizeType getSize() const {
      return queue<ElementT*>::size();
   }
   
   /**
    *    Insert one element into the queue. The  is not copied,
    *    so it must not be deleted by the caller.
    *    @param   p  The element to insert into the queue.
    *    @return  True if the element is inserted, false otherwise.
    */
   bool enqueue(ElementT* p) {
      // Make sure no one else is changing the list
      ISABSync synchronized(m_monitor);
      
      // Insert the packet into the queue if not NULL
      if (p != NULL) {
         queue<ElementT*>::push(p);
         m_monitor.notifyAll();
         if ( m_selector != NULL ) {
            m_selector->notify();
         }
         return true;
      } else {
         // NULL means that we should not wait for long now.
         m_wakeUp = true;         
         m_monitor.notifyAll();
         if ( m_selector != NULL ) {
            m_selector->notify();
         }
         return false;
      }
   }
      
   /**
    *    Get one packet from the queue. If the queue is empty this
    *    method will hang until there is a packet to return, but
    *    maximum maxWaitTime. If the timeout is reached or if 
    *    terminate() is called, NULL is returned.
    *    
    *    @param   maxWaitTime The maximum time to wait for packets
    *                         in the queue. In milliseconds.
    *    @return  One packet in the queue.
    */
   ElementT* dequeue(uint32 maxWaitTime) {
      // Make sure no one else is changing the list
      ISABSync synchronized(m_monitor);
      
      if (!m_wakeUp && queue<ElementT*>::empty() && !m_terminated) {
         try {
            if ( maxWaitTime != MAX_UINT32 ) {
               m_monitor.wait(maxWaitTime);
            } else {
               m_monitor.wait();
            }
         } catch ( const JTCInterruptedException & ) {
            mc2log << warn 
                   << "PointerFifoTemp::dequeue wait( int ) interrupted!"
                   << endl;
         }
      }
      
      if ( m_wakeUp ) {
         m_wakeUp = false;
      }
   
      // If we have a packet to return, we return it here
      if (!queue<ElementT*>::empty() && !m_terminated) {
         ElementT* p = queue<ElementT*>::front();
         queue<ElementT*>::pop();
         return p;
      } else {
         return NULL;
      }
   }

   /**
    *    Get one element from the queue. If the queue is empty this
    *    method will hang until there is a packet to return or until
    *    the queue is woken up. In that case NULL can be returned.
    *    @return  One element in the queue.
    */
   ElementT* dequeue() {
      return dequeue(MAX_UINT32);
   }

   /**
    *    Get one element from the queue. If the queue is empty
    *    NULL will be returned.
    *    @return One element from the queue or NULL if the queue is
    *            empty.
    */
   ElementT* dequeueNonBlocking() {
      ISABSync synchronized(m_monitor);
      if ( !queue<ElementT*>::empty() ) {
         ElementT* p = queue<ElementT*>::front();
         queue<ElementT*>::pop(); return p;         
      } else {
         return NULL;
      }
   }
   
   void dequeue( vector<ElementT*>& elements ) {
      ISABSync sync( m_monitor );
      SizeType size = queue<ElementT*>::size();
      elements.resize( size );
      for (SizeType i = 0; i < size; ++i ) {
         elements[ i ] = queue<ElementT*>::front();
         queue<ElementT*>::pop();
      }
   }
   /**
    *    Releases the monitor by notifying all. NOTE! This will make 
    *    dequeue() return NULL.
    */
   void terminate() {
      ISABSync synchronized(m_monitor);
      m_terminated = true;
      m_monitor.notifyAll();
   }
   
private:
   /**
    *    The monitor used to protect the enqueue- and dequeue-methods.
    */
   ISABMonitor m_monitor;
   
   /**
    *    True if the queue is terminated, false otherwise.
    */
   bool m_terminated;
   
   /**
    *    True if the queue should wake up.
    */
   bool m_wakeUp;

   /**
    *    Socketreceiver to wake up when someting happens.
    */
   Notifier* m_selector;
   
   /**
    *    Private copy constructor to avoid use.
    *    Not implemented to further avoid use.
    */
   PointerFifoTemplate(const PointerFifoTemplate& rhs);

};

#endif


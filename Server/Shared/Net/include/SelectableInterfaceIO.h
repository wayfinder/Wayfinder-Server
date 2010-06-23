/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SELECTABLEINTERFACEIO_H
#define SELECTABLEINTERFACEIO_H

#include "config.h"
#include "InterfaceIO.h"
#include "ISABThread.h"
#include "SelectableInterfaceRequest.h"
#include <set>
#include <memory>


// Forwards
class SelectableSelector;


/**
 * Abstract super class for handling InterfaceRequests IO.
 *
 */
class SelectableInterfaceIO : public InterfaceIO {
   public:
      /**
       * Creates a new InterfaceIO.
       *
       * @param group The InterfaceHandleIO to send
       *              InterfaceRequests ready to be processed to.
       */
      SelectableInterfaceIO( InterfaceHandleIO* group );


      /**
       * Destructor.
       */
      virtual ~SelectableInterfaceIO();


      /**
       * A new InterfaceRequest to do IO for.
       * Mutex method.
       */
      virtual void putInterfaceRequest( InterfaceRequest* ireq );


      /**
       * If no InterfaceRequests in this.
       * Mutex method.
       */
      virtual bool empty() const;

      /**
       * The number of InterfaceRequest in this.
       */
      virtual uint32 size() const;

      /**
       * Shutdown, stop idle IOs.
       */
      virtual void shutdownStarts();


   protected:


      typedef set< SelectableInterfaceRequest* > selSet;


      typedef multiset< SelectableInterfaceRequest*, 
         SelectableInterfaceRequestCmpLess > selReqSet;


      /**
       * Put some ready selectables by SelectableInterfaceIOThread.
       * Mutex method.
       *
       * @param readReady The Selectables that are done with IO.
       * @param addedSelectable Added selectacbles are swapped in into this
       *                        set.
       * @param shutdown Set to true if shuting down, stop idle IOs.
       */
      void putReady( selSet& readySelectable,
                     selSet& addedSelectable,
                     bool& shutdown );


      /**
       * The SelectableSelector that selects among the selectables.
       */
      auto_ptr< SelectableSelector > m_selector;


      /**
       * The set of SelectableInterfaceRequests that has been added.
       */
      selSet m_addedSelectable;


      /**
       * The number of SelectableInterfaceRequests that is currently 
       * being processed.
       */
      int32 m_nbrRequests;


      /**
       * If shuting down, no idleling.
       */
      bool m_shutdown;


      /**
       * The mutex protecting m_selectable.
       */
      ISABMutex m_mutex;


      /**
       * The thread that calls select.
       * @return The smallest timeout of all Selectables, in ms, MAX_UINT32
       *         if no Selectable.
       */
      class SelectableInterfaceIOThread : public ISABThread {
         public:
            /**
             * @param selector The selector.
             */
            SelectableInterfaceIOThread( SelectableSelector* selector,
                                         SelectableInterfaceIO* io );


            /**
             * Destructor
             */
            ~SelectableInterfaceIOThread();


            /**
             * The thread method.
             */
            void run();


         private:
            /// The selector
            SelectableSelector* m_selector;


            /// The selectables in timeout order
            selReqSet m_selectable;


            /// The IO
            SelectableInterfaceIO* m_io;
      };


      /**
       * The thread that calls select.
       */
      ISABThreadHandle m_thread;
};


#endif // SELECTABLEINTERFACEIO_H



/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SELECTABLESELECTOR_H
#define SELECTABLESELECTOR_H

#include "config.h"
#include "Selectable.h"
#include "ISABThread.h"
#include "DebugClock.h"

#include <map>
#include <set>
#include <vector>
#include <memory>

class NotifyPipe;

/**
 * Selects among a set of selectables.
 *
 */
class SelectableSelector {
   public:
   /// a set of selectables
   typedef set< Selectable* > selSet;

   /// status returned by select
   enum SelectStatus {
      TIMEOUT = -2, ///< a timeout occurred
      ERROR = -1, ///< something went wrong
      OK = 0 ///< everything executed ok
   };

      /**
       * Create a new SelectableSelector.
       */
      SelectableSelector();


      /**
       * Destructor.
       */
      virtual ~SelectableSelector();


      /**
       * Select among the selectables with a timeout.
       * Mutex method at beginning and end of method but not when 
       * actually selecting.
       *
       * @param timeout Timeout for select in microseconds, waits forever
       *                if negative.
       * @param readReady Selectables ready for read are added to this.
       * @param writeReady Selectables ready for write are added to this.
       * @return 0 on select ok, -1 on error and -2 on timeout.
       */
      SelectStatus select( int32 timeout, 
                           selSet& readReady, 
                           selSet& writeReady );


      /**
       * Set to terminate, select returns immediately.
       * Calls notify.
       */
      void terminate();


      /**
       * Adds a selectable.
       * Adds the Selectable to read-select or write-select or both
       * or neighter (but that is a nop). If sel is added to both
       * read- and write-select then it must be removed from both.
       * Mutex method.
       *
       * @param sel The Selectable to add.
       * @param readSelect If to select for read.
       * @param writeSelect If to select for write.
       */
      void addSelectable( Selectable* sel, 
                          bool readSelect, bool writeSelect,
                          bool doNotify = true );


      /**
       * Removes a selectable by pointer.
       * Mutex method.
       *
       * @param sel The Selectable to remove.
       * @param readSelect If to remove from read-select.
       * @param writeSelect If to remove from write-select.
       */
      void removeSelectable( Selectable* sel,
                             bool readSelect, bool writeSelect,
                             bool doNotify = true);

      /**
       * Removes a selectable by Selectable::selectable, slow method.
       * Mutex method.
       *
       * @param sel The selectable to remove.
       * @param readSelect If to remove from read-select.
       * @param writeSelect If to remove from write-select.
       */
      void removeSelectable( Selectable::selectable sel,
                             bool readSelect, bool writeSelect,
                             bool doNotify = true );

      /**
       * Notify anyone in select.
       */
      void notify();


      /**
       * If empty.
       * Mutex method.
       */
      bool empty();

private:
      /**
       * Clears the fd_sets and add the notify pipe and all in 
       * m_readSelectables and m_writeSelectables.
       *
       * @return The highest selectable.
       */
//#define COUNT_HIGHEST_SEL
#ifdef COUNT_HIGHEST_SEL
       Selectable::selectable
#else
       void
#endif
          clearAndSet();



      /**
       * Mutex for m_readSelectables and m_writeSelectables.
       */
      ISABMutex m_mutex;

      /**
       * The set of selectables for read.
       */
      selSet m_readSelectables;


      /**
       * The set of selectables for write.
       */
      selSet m_writeSelectables;


      /**
       * If terminated.
       */
      bool m_terminated;


      /// The set of system read selectables.
      fd_set m_readfds;
      
   
      /// The set of system write selectables.    
      fd_set m_writefds;

   /// The notify pipe.
   std::auto_ptr<NotifyPipe> m_notifyPipe;

};

#endif // SELECTABLESELECTOR_H


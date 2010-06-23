/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIMEDSELECTOR_H
#define TIMEDSELECTOR_H

#include "SelectableSelector.h"
#include "TimedSelectable.h"

#include <set>
#include <map>
/**
 * Timed version of SelectableSelector.
 * Handles TimedSelectables.
 */
class TimedSelector {
public:
   typedef set<TimedSelectable*> SelectableSet;

   TimedSelector();
   ~TimedSelector();

   /**
    * Select on timed selectables and calls handleIO when
    * there is a read or write ready.
    * @param timedOut this set will be filled with selectables that timed out
    */
   void select( SelectableSet& timedOut );

   /**
    * Add selectable with specific io type.
    *
    * @param select
    * @param type
    */
   void addSelectable( TimedSelectable* select );

   /**
    * Remove from select
    *
    * @param select 
    */
   void removeSelectable( TimedSelectable* select );

   /**
    * Remove from select
    *
    * @param select
    */
   void removeSelectable( Selectable::selectable select );

   /// @return selector
   SelectableSelector& getSelector() { return m_selector; }
   /*
    * Fills in selectable set with all the selectables, used for a proper
    * shutdown (delete of selectables)
    * @param selectables this set will be filled in with all the selectables
    *  contained in this selector
    */ 

   void getSelectables( SelectableSet& selectables );
   /**
    * Update timeout map for a specific selectable
    * @param selectable the item to update the timeout from
    */
   void updateTimeout( TimedSelectable& selectable );
private:
   void checkReadAndWrites( SelectableSelector::selSet& readReady,
                            SelectableSelector::selSet& writeReady,
                            SelectableSet& timeoutSet );

   typedef multimap< uint32, TimedSelectable* > TimeoutMap;

   TimeoutMap m_timeoutMap;
   SelectableSelector m_selector;
};

#endif

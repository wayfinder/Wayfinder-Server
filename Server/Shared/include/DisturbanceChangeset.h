/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCE_CHANGESET_H
#define DISTURBANCE_CHANGESET_H

#include "config.h"
#include "NotCopyable.h"

#include <vector>

class DisturbanceElement;

/**
 * A changset of elements.
 * Holds update/new and removed set.
 * This will not be copyable, use swap to swap changes. This is to
 * preserve a consistent ownership of the elements.
 */
class DisturbanceChangeset: private NotCopyable {
public:

   typedef std::vector<DisturbanceElement*> Elements;

   DisturbanceChangeset() { }

   /// Swaps and takes ownership of the elements.
   DisturbanceChangeset( Elements& updateSet,
                         Elements& removeSet ) {
      m_updateSet.swap( updateSet );
      m_removeSet.swap( removeSet );
   }
   ~DisturbanceChangeset();

   /// @return The set to be added or updated
   const Elements& getUpdateSet() const { return m_updateSet; }
   /// @return The set to be removed
   const Elements& getRemoveSet() const { return m_removeSet; }

   /// Swap update set with \c other.
   void swapUpdateSet( Elements& other ) { other.swap( m_updateSet ); }

   /// Swap remove set with \c other.
   void swapRemoveSet( Elements& other ) { other.swap( m_removeSet ); }

   /// swap changes with another changes
   void swap( DisturbanceChangeset& other ) {
      swapUpdateSet( other.m_updateSet );
      swapRemoveSet( other.m_removeSet );
   }

private:
   Elements m_updateSet; ///< The set of elements to be added or updated.
   Elements m_removeSet; ///< The set of elements to be removed.
};

#endif // DISTURBANCE_CHANGESET_H

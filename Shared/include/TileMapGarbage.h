/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPHARAGAAHGE_H
#define TILEMAPHARAGAAHGE_H

#include "config.h"
#include <vector>

#include "TileMapToolkit.h"

template <class G> class TileMapGarbage : public TileMapIdleListener {
private:
   enum {
      /// Minimum free memory
      m_minFreeMem  = 40000,
      /// Minimum size of biggest block.
      m_minBlockSize = 8096,
   };
   /// The id of the requested idle. 0 if not requested.
   uint32 m_idleID;

   /// List of garbage to delete.
   vector<G*> m_garbage;

   /// The tilemap toolkit is needed to request the 
   TileMapToolkit* m_toolkit;

   /// True if the object is active
   int m_isActive;

public:
   /**
    *   Creates a new collection of garbage.
    *   Is active at startup, i.e. will start requesting idles
    *   when garbage is added.
    */
   TileMapGarbage( TileMapToolkit* toolkit ) {
      m_toolkit = toolkit;
      m_idleID  = 0;      
      activate();
   }

   /// Deletes all remaining garbage
   virtual ~TileMapGarbage() {
      deActivate();
      m_toolkit->cancelIdle( this, m_idleID );
      
      while ( deleteSomeGarbage() ) {
         // Deleting
      }      
   }
   
   /**
    *   Deletes garbage if needed.
    */
   void checkGarbage() {
      // Delete garbage until there is some available memory.
      uint32 biggestBlock;
      while ( ( ( m_toolkit->availableMemory(biggestBlock) < m_minFreeMem ) ||
                ( biggestBlock < m_minBlockSize ) ) &&
              deleteSomeGarbage() ) {
      }
   }
   
   /// Add an object for later destruction.
   inline void addGarbage( G* garb ) {
      m_garbage.push_back( garb );
      // Check if there is too much garbage already.
      checkGarbage();
      // Start the idle if this object is active.
      startIdle();
   }

   /**
    *   Deletes one or many garbage objects depending on memory
    *   use and the number of elements in the list.
    *   @return Not zero if something was deleted.
    */
   int deleteSomeGarbage() {
      if ( ! m_garbage.empty() ) {
         delete m_garbage.back();
         m_garbage.pop_back();
      }
      return ! m_garbage.empty();
   }
   
   /**
    *   Activates the idle object.
    */
   void activate() { m_isActive = true; }

   /**
    *   Stops the idle object.
    */
   void deActivate() { m_isActive = false; }
      
   
private:
   /// Idle listener needs to be able to call runIdleTask
   friend class TileMapGarbageIdleListener;
   
   /**
    *   Runs checkGarbage.
    */
   void runIdleTask( uint32 id ) {
      if ( id == m_idleID ) {
         mc2dbg8 << "[TileMapGarbage]: runIdleTask" << endl;
         m_idleID = 0;
         if ( deleteSomeGarbage() ) {
            startIdle();
         }
      }
   }

   /**
    *   Starts the idle ojbect.
    */
   int startIdle() {
      if ( m_isActive && m_idleID == 0 && ( ! m_garbage.empty() )) {
         m_idleID = m_toolkit->requestIdle( this );
         return true;
      } else {
         return false;
      }
   }
      
};

#endif

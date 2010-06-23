/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETCONTAINERTREE_H
#define PACKETCONTAINERTREE_H

#include "config.h"
#include "PacketContainer.h"

#include <map>


/**
 *   Class for holding a sorted tree of PacketContainers.
 *
 */
class PacketContainerTree {
public:
      /**
       * Destructor.
       */
      ~PacketContainerTree();
      
      /**
       * Adds a new PacketContaner to the tree.
       *
       * @param cont The PacketContaner to add.
       */
      inline void add( PacketContainer* cont );
   

      /**
       * Removes a PacketContaner from the tree.
       *
       * @param cont The PacketContaner to remove.
       */
      inline void remove( PacketContainer* cont );      
      
      /**
       *   Get the packetcontainer with the lowest absolute
       *   timeout time.
       *
       *   @return The minimun PacketContainer.
       */
      inline PacketContainer* getMin();


      /**
       *   Get the packetcontainer with the lowest absolute
       *   timeout time.
       *   @return The minimun PacketContainer.       
       */
      inline PacketContainer* extractMin();

      /**
       *   Returns packet container with the supplied ids or NULL.
       */
      PacketContainer* linearGetWithRequestAndPackID( uint32 reqID,
                                                      uint32 packID );

      /**
       * The number of PacketContainers in tree.
       * 
       * @return The number of PacketContainers.
       */
      inline uint32 getCardinal() const;

      /**
       *   Returns true if the tree is empty.
       */
      inline bool empty() const;
private:

   /// Type of map of packetcontainers.
   typedef multimap<uint32, PacketContainer*> packetMap_t;

   /// Packet containers are kept here.
   packetMap_t m_packets;
};


// =======================================================================
//                                 Implementation of the inlined methods =


void 
PacketContainerTree::add( PacketContainer* cont )
{
   m_packets.insert( make_pair( cont->getTimeoutAbsTime(),
                                cont ) );                                
}


void 
PacketContainerTree::remove( PacketContainer* cont )
{
   // You could possible find the start it with lower_bound
   // on the time, but not if the time has changed in the cont.
   for( packetMap_t::iterator it = m_packets.begin();
        it != m_packets.end();
        ++it ) {
      if ( it->second == cont ) {
         m_packets.erase(it);
         return;
      }
   }
}


PacketContainer*
PacketContainerTree::getMin()
{
   if ( ! m_packets.empty() ) {
      return m_packets.begin()->second;
   } else {
      return NULL;
   }      
}

PacketContainer*
PacketContainerTree::extractMin()
{
   PacketContainer* tmp = getMin();
   if ( tmp != NULL ) {
      remove(tmp);
   }
   return tmp;
}


uint32 
PacketContainerTree::getCardinal() const
{
   return m_packets.size();
}

bool
PacketContainerTree::empty() const
{
   return m_packets.empty();
}

#endif // PACKETCONTAINERTREE_H

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULEPUSHQUEUE_H
#define MODULEPUSHQUEUE_H

#include "config.h"
#include <deque>
#include "ISABThread.h"

class TCPSocket;
class Packet;

class ModulePushList : private deque<pair< Packet*, TCPSocket*> > {
public:
   /**
    *   Adds a new packet and socket to the fifo.
    *   @param packet The packet that was received on the socket.
    *                 NULL if the socket was broken.
    *   @param socket The socket where the packet was received.
    */
   inline void insert( Packet* packet, TCPSocket* socket);

   /**
    *   Gets and removes a packet and socket from the fifo.
    *   @return Pointer to a pair of Packet and TCPSocket. To
    *           be deleted by the caller.
    */
   inline pair<Packet*, TCPSocket*>* remove();

   inline bool has( const TCPSocket* socket ) const;
private:
   
   /**
    *   Returns true if the Fifo is empty.
    */
   inline bool unsafeEmpty() const;

   /**
    *   Mutex to protect the 
    */
   ISABMutex m_mutex;
   
};

inline bool
ModulePushList::unsafeEmpty() const
{
   bool result = deque< pair<Packet*, TCPSocket*> >::empty();
   return result;
}

inline void
ModulePushList::insert( Packet* packet,
                        TCPSocket* socket)
{
   m_mutex.lock();
   push_back( make_pair(packet, socket) );
   m_mutex.unlock();
}

inline pair<Packet*, TCPSocket*>*
ModulePushList::remove()
{
   m_mutex.lock();
   if ( ! unsafeEmpty() ) {
      // Copy
      pair<Packet*, TCPSocket*>*retVal =
         new pair<Packet*, TCPSocket*>(front());
      pop_front();
      m_mutex.unlock(); // Important
      return retVal;
   } 
   // List is empty
   m_mutex.unlock(); // Important
   return NULL;
}

inline
bool ModulePushList::
has( const TCPSocket* socket ) const  
{
   m_mutex.lock();
   const_iterator it = begin();
   const_iterator itEnd = end();
   for (; it != itEnd; ++it ) {
      if ( (*it).second == socket ) {
         break;
      }
   }
   m_mutex.unlock();

   if ( it == itEnd ) {
      return false;
   }

   return true;

}
#endif

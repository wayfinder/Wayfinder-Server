/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEMORYDATABUFFERREQUESTER_H
#define MEMORYDATABUFFERREQUESTER_H

#include "config.h"
#include "DBufRequester.h"
#include "MC2SimpleString.h"

#include<map>
#include<list>

/**
 *   Class requesting databuffers from memory and a parent
 *   requester.
 */
class MemoryDBufRequester : public DBufRequester {
private:
   /// The maps and descrptions are stored here. Stored in access order.
   typedef list<pair<MC2SimpleString, BitBuffer*> > storage_t;
public:
   
   /// Pair of string and buffer
   typedef storage_t::value_type stringBufPair_t;

   /**
    *   Creates a new MemoryDBufRequester.
    *   @param parent Who to ask if the buffer isn't in memory.
    *   @param maxMem Approx max mem in bytes. TODO Implement.
    */
   MemoryDBufRequester(DBufRequester* parent,
                       uint32 maxMem);

   /**
    *   Deletes all stuff.
    */
   virtual ~MemoryDBufRequester();
   
   /**
    *   Makes it ok for the Requester to delete the BitBuffer
    *   or to put it in the cache. Requesters which use other
    *   requesters should hand the objects back to their parents
    *   and requesters which are top-requesters should delete them.
    *   It is important that the descr is the correct one.
    */ 
   virtual void release(const MC2SimpleString& descr,
                        BitBuffer* obj);

   /**
    *   If the DBufRequester already has the map in cache it
    *   should return it here or NULL. The BitBuffer should be
    *   returned in release as usual.
    *   @param descr Key for databuffer.
    *   @return Cached BitBuffer or NULL.
    */
   BitBuffer* requestCached(const MC2SimpleString& descr);

   /// Sets maximum storage size.
   void setMaxSize(uint32 size);
   /// Clears cache
   void clearCache();

   
   // -- For use as a queue.

   /**
    *   Returns a reference to the oldest map.
    */
   stringBufPair_t& front();

   /**
    *   Removes the oldest map. Note that the buffer must
    *   be deleted by the caller.
    */
   void pop_front();

   /**
    *   Returns true if the queue is empty.
    */
   bool empty() const {
      return m_buffers.empty();
   }
 
private:   
   
   /**
    *  Type of map for buffer lookup. Uses list iterators since they
    *  are valid until deleted
    */
   typedef map<const MC2SimpleString*,
               storage_t::iterator,
               MC2SimpleString::StrCmp> bufferMap_t;
   
    /// Inserts the element in both the list and the map
   bool private_insert(const MC2SimpleString& descr,
                       BitBuffer* buffer);

   /// Removes the map from the storage
   void internalRemove( const MC2SimpleString& descr );
   
   /// erases the element both from the map and the list.
   void erase(bufferMap_t::iterator it);

   /// erases the element both from the map and the list.
   void erase(storage_t::iterator it);

   /// Checks memory usage and erases buffers until it is below max
   void deleteOldMapsIfNecessary();

   /// Returns the memory useage for the supplied pair
   uint32 getMemUse(const stringBufPair_t& entry) const;
   
   /// Storage of buffers.
   storage_t m_buffers;
   /// Buffer lookup
   bufferMap_t m_bufferLookup;

   /// Amount of used memory
   uint32 m_usedMem;
   /// Maximum amount of used memory.
   uint32 m_maxMem;
   
};


#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MemoryDBufRequester.h"


#include "BitBuffer.h"

#undef WRITE_TO_FILE
#ifdef WRITE_TO_FILE
#  include <stdio.h>
#endif

MemoryDBufRequester::MemoryDBufRequester(DBufRequester* parent,
                                         uint32 maxMem)
      : DBufRequester(parent)
{
   m_maxMem = maxMem;
   m_usedMem = 0;
}

MemoryDBufRequester::~MemoryDBufRequester()
{

#ifdef WRITE_TO_FILE
   FILE* file = fopen("MapFile.dump", "w");
   // Write size of maps little endian. Oh.
   uint32 nbrMaps = m_buffers.size();
   fwrite( &nbrMaps, 4, 1, file );
#endif
   
   // delete the saved buffers.
   for( storage_t::iterator it = m_buffers.begin();
        it != m_buffers.end();
        ++it ) {
#ifdef WRITE_TO_FILE
      const MC2SimpleString& curDesc = it->first;
      const BitBuffer* curBuf        = it->second;
      BitBuffer tempBuf( 4 + curDesc.length() +
                         4 + curBuf->getBufferSize() );
      tempBuf.writeNextBALong( curDesc.length() );
      tempBuf.writeNextByteArray( (const byte*)curDesc.c_str(),
                                  curDesc.length() );
      tempBuf.writeNextBALong( curBuf->getBufferSize() );
      tempBuf.writeNextByteArray( curBuf->getBufferAddress(),
                                  curBuf->getBufferSize() );
      fwrite( tempBuf.getBufferAddress(),
              tempBuf.getBufferSize(), 1, file );
#endif
      DBufRequester::release(it->first, it->second);
   }
#ifdef WRITE_TO_FILE
   fclose(file);
#endif
   m_buffers.clear();
   m_bufferLookup.clear();
}

uint32
MemoryDBufRequester::getMemUse(const stringBufPair_t& entry) const
{
   // A less optimistic size calculation than the old one.
   return 8 + 8 + 4 + entry.first.length() +
      entry.second->getBufferSize();
}

void
MemoryDBufRequester::setMaxSize( uint32 size )
{
   m_maxMem = size;
}

void
MemoryDBufRequester::clearCache()
{
   uint32 oldMem = m_maxMem;
   m_maxMem = 0;
   deleteOldMapsIfNecessary();
   m_maxMem = oldMem;
}

void
MemoryDBufRequester::internalRemove( const MC2SimpleString& descr )
{
   bufferMap_t::iterator findit = m_bufferLookup.find( &descr );
   if ( findit == m_bufferLookup.end() ) {
      mc2log << "[MDBR]: Removing " << descr << " from storage" << endl;
      erase( findit );
   }
}

bool
MemoryDBufRequester::private_insert(const MC2SimpleString& descr,
                                    BitBuffer* buffer)
{
   bufferMap_t::iterator findit = m_bufferLookup.find( &descr );
   if ( findit == m_bufferLookup.end() ) {
      // Not there - insert right away
      // Add to the list
      storage_t::iterator listit =
         m_buffers.insert(m_buffers.end(), make_pair(descr, buffer));
      // Add the pointer to the string and the iterator in the list
      // to the lookup map.
      m_bufferLookup.insert(make_pair(&(listit->first),
                                      listit) );
      m_usedMem += getMemUse(*listit);
      return true;
   } else {
      // Erase the buffer and insert it.
      storage_t::iterator listit = findit->second;
      stringBufPair_t pair( *listit );         
      erase(findit);
      // Release the buffer to the next requester.
      DBufRequester::release( pair.first, pair.second );
      return private_insert(descr, buffer);      
   }
   // Cannot happen
   return false; 
}

void
MemoryDBufRequester::erase(bufferMap_t::iterator it)
{
   storage_t::iterator lit = it->second;
   m_usedMem -= getMemUse(*lit);
   m_bufferLookup.erase(it);
   m_buffers.erase(lit);
}

void
MemoryDBufRequester::erase(storage_t::iterator it)
{
   // If not found in map, then it will be problems.
   erase(m_bufferLookup.find(&(it->first)));
}

BitBuffer*
MemoryDBufRequester::requestCached(const MC2SimpleString& descr)
{   
   bufferMap_t::iterator it = m_bufferLookup.find( &descr );
   if ( it != m_bufferLookup.end() ) {
      // Use the iterator in the map to find the buffer.
      BitBuffer* buf = it->second->second;
      buf->reset();
      // Remove it from our storage.
      erase( it );
      return buf;
   } else {
      return DBufRequester::requestCached(descr);
   }
}

void
MemoryDBufRequester::deleteOldMapsIfNecessary()
{
   while ( m_usedMem > m_maxMem && !m_buffers.empty() ) {
      // Delete first in list (oldest)
      storage_t::iterator lit = m_buffers.begin();
      // Copy the pair since the flow of the program can lead us
      // back into insert.
      stringBufPair_t bufPair(*lit);
      erase( lit );
      DBufRequester::release(bufPair.first, bufPair.second);
   }
#ifdef MC2_SYSTEM
   // Check that the tracking of sizes work when running mc2.
   uint32 sum = 0;
   for( storage_t::iterator it = m_buffers.begin();
        it != m_buffers.end();
        ++it ) {
      sum += getMemUse(*it);
   }
   MC2_ASSERT( sum == m_usedMem );
#endif
}

MemoryDBufRequester::stringBufPair_t&
MemoryDBufRequester::front()
{
   return *(m_buffers.begin());
}

void
MemoryDBufRequester::pop_front()
{
   // Delete first in list (oldest)
   storage_t::iterator lit = m_buffers.begin();  
   erase( lit );
}

void
MemoryDBufRequester::release(const MC2SimpleString& descr,
                             BitBuffer* buffer)
{
   // Insert the buffer into the storage
   if ( buffer != NULL ) {
      private_insert(descr, buffer);
   }
   // Keep track of mem use
   deleteOldMapsIfNecessary();
}

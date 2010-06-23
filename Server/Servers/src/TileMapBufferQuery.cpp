/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "TileMapBufferQuery.h"

#include "SharedBuffer.h"

#include "MC2SimpleString.h"
#include "DeleteHelpers.h"

void
TileMapBufferQuery::init()
{
   m_multiBuf  = NULL;  
   // Start with 10 k
   m_buf.reserve( 10 * 1024 );
   m_timeLimit = 2000;
}

TileMapBufferQuery::TileMapBufferQuery( const MC2SimpleString& tileMapSpec )
      : TileMapQuery( paramVect_t( 1, tileMapSpec ) )
{
   m_maxSize = MAX_UINT32;
   init();
}

TileMapBufferQuery::TileMapBufferQuery( const paramVect_t& specs,
                                        uint32 maxSize )
      : TileMapQuery( specs ),
        m_maxSize( maxSize )
{
   init();
}

TileMapBufferQuery::~TileMapBufferQuery()
{
   delete m_multiBuf;
   STLUtility::deleteAllSecond( m_singleBufs );
}

static void addString( vector<uint8>& buf,
                       const MC2SimpleString& str )
{
   const uint8* strBegin =
      reinterpret_cast<const uint8*>(str.c_str());
   const uint8* strEnd   = strBegin + str.length() + 1;
   // Write the str into the big buffer
   buf.insert( buf.end(), strBegin, strEnd );
}

static void addLength( vector<uint8>& buf,
                       const SharedBuffer* inbuf )   
{
   uint32 length = 0;
   if ( inbuf ) {
      length = inbuf->getBufferSize();
   }
   uint8 lengthBytes[8];   
   SharedBuffer lengthBuf( lengthBytes, 4 );
   lengthBuf.writeNextBALong( length );
   buf.insert( buf.end(), lengthBytes, lengthBytes + 4 );
}

static void addBuffer( vector<uint8>& outbuf,
                       const SharedBuffer* inbuf )
{
   addLength( outbuf, inbuf );
   if ( inbuf != NULL ) {
      outbuf.insert( outbuf.end(),
                     inbuf->getBufferAddress(),
                     inbuf->getBufferAddress() + inbuf->getBufferSize() );
   }
}

int
TileMapBufferQuery::internalAddBuffers( const bufVect_t& bufs )
{
   int nbrAdded = 0;
   for ( bufVect_t::const_iterator it = bufs.begin();
         it != bufs.end();
         ++it ) {

      // FIXME: Add small buffers even if size exceeded.
      if ( m_buf.size() > m_maxSize ) {
         break;
      }

      mc2dbg8 << "[TMBQ]: internalAddBuffers " << it->first << endl;
            
      // Write the param string
      addString( m_buf, it->first );
      // Write the buffer len + buffer
      addBuffer( m_buf, it->second );
      
      ++nbrAdded;
   }
   return nbrAdded;
}

void
TileMapBufferQuery::createMultiBuffer()
{
   m_multiBuf = new SharedBuffer( & ( m_buf.front() ),
                                  m_buf.size() );
   // Also position the offset at the end of the buffer.
   m_multiBuf->readPastBytes( m_buf.size() );
}

const SharedBuffer*
TileMapBufferQuery::getMultiBuffer() const
{
   mc2dbg4 << "TileMapBufferQuery: " << __FUNCTION__ << endl;
   if ( m_multiBuf == NULL ) {
      const_cast<TileMapBufferQuery*>(this)->createMultiBuffer();      
   }
   return m_multiBuf;
}

const SharedBuffer*
TileMapBufferQuery::createSingleBuf( const MC2SimpleString& param )
{
   const SharedBuffer* multiBuf = getMultiBuffer();
   if ( multiBuf == NULL ) {
      return NULL;
   }
   // Read the single buffer from the multibuffer.
   SharedBuffer rb( *multiBuf );
   rb.reset();
   while ( rb.getCurrentOffset() < rb.getBufferSize() ) {
      MC2SimpleString desc = rb.readNextString();
      int buflen = rb.readNextBALong();
      if ( buflen != 0 ) {
         SharedBuffer mapBuf(
            const_cast<uint8*>(rb.readNextByteArray( buflen )),
            buflen );
         mapBuf.readPastBytes( buflen );
         if ( param == desc ) {
            // Found it!
            m_singleBufs.insert( make_pair( desc,
                                            new SharedBuffer( mapBuf ) ) );
            return m_singleBufs[desc];
         }
      }
   }
   // Not found
   return NULL;
}

const SharedBuffer*
TileMapBufferQuery::getSingleBuffer( const MC2SimpleString& param ) const
{
   map<MC2SimpleString, SharedBuffer*>::const_iterator findit =
      m_singleBufs.find( param );
   if ( findit != m_singleBufs.end() ) {
      return findit->second;
   } else {
      return const_cast<TileMapBufferQuery*>(this)->createSingleBuf( param );
   }
}

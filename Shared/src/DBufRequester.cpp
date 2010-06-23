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

#include "DBufRequester.h"

#include "BitBuffer.h"

DBufRequester::DBufRequester(DBufRequester* parentRequester)
{
   m_parentRequester = parentRequester;
}

BitBuffer*
DBufRequester::requestCached(const MC2SimpleString& descr)
{
   if ( m_parentRequester != NULL ) {
      return m_parentRequester->requestCached(descr);
   } else {
      return NULL;
   }
}

void
DBufRequester::request(const MC2SimpleString& descr,
                       DBufRequestListener* caller)
{
   // Look in the cache.
   BitBuffer* tmpBuf = requestCached(descr);
   
   // Look in parent's cache.
   if ( ( tmpBuf == NULL ) && ( m_parentRequester != NULL ) ) {
      tmpBuf = m_parentRequester->requestCached(descr);
   }
   
   if ( tmpBuf ) {
      // Found a buffer. Send it back.
      caller->requestReceived(descr, tmpBuf);
   } else if ( m_parentRequester != NULL ) {
      // Request it from the parent if there is one.
      m_parentRequester->request(descr, caller);
   } else {
      // Request it from an external source
      // We are probably the bottom requester.
      requestFromExternal(descr, caller);
   }
}

void
DBufRequester::release(const MC2SimpleString& descr,
                       BitBuffer* obj)
   
{
   if ( m_parentRequester != NULL ) {
      m_parentRequester->release(descr, obj);
   } else {
      delete obj;
   }
}

void
DBufRequester::releaseCached( const MC2SimpleString& descr,
                              BitBuffer* obj )
   
{
   DBufRequester::release( descr, obj );
}

void
DBufRequester::cancelAll()
{
   if ( m_parentRequester != NULL ) {
      m_parentRequester->cancelAll();
   }
}

void
DBufRequester::requestFromExternal(const MC2SimpleString& descr,
                                   DBufRequestListener* caller)
{
   
}

void
DBufRequester::removeBuffer( const MC2SimpleString& descr )
{
   // First remove the buffer here
   internalRemove( descr );
   // Then remove it from the parent.
   if ( m_parentRequester ) {
      m_parentRequester->removeBuffer( descr );
   }
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_MEMCACHED

#include "Memcached.h"
#include "AutoPtr.h"
#include "DeleteHelpers.h"
#include "Properties.h"
#include <libmemcached/memcached.h>

typedef AutoPtr<memcached_st> MemcachedAutoPtr;
template<>
void AutoPtr<memcached_st>::destroy() {
   if ( get() != NULL ) {
      memcached_free( get() );
   }
}

namespace Memcached {

MemcachedException::MemcachedException( const MC2String& what )
   : MC2Exception( "Memcached", what ) {
}

struct Cache::Implementation {
   Implementation()
         : memc( NULL ) {
   }

   MemcachedAutoPtr memc;
};

/**
 * Help function for throwing a helpful exception when something
 * has failed in libmemcached.
 */
void throwOnFail( memcached_return rc, 
                  memcached_st* memc,
                  const MC2String& description ) {

   if ( rc != MEMCACHED_SUCCESS ) {
      throw MemcachedException( description + ": " +
                                memcached_strerror( memc, rc ) );
   }
}

Cache::Cache( const MC2String& hosts )
      : m_pimpl( new Implementation ) {
   m_pimpl->memc.reset( memcached_create( NULL ) );

   if ( m_pimpl->memc.get() == NULL ) {
      throw MemcachedException( "Couldn't create memcached_st structure" );
   }

   // Enable asynchronous IO mode
   memcached_return rc = 
      memcached_behavior_set( m_pimpl->memc.get(),
                              MEMCACHED_BEHAVIOR_NO_BLOCK,
                              1 /* 1 = enable */ );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't set asynchronous IO mode" );

   uint32 timeout = Properties::getUint32Property( "MEMCACHED_TIMEOUT",
                                                   1000 // default, ms
                                                   );
   // Set poll timeout
   rc = memcached_behavior_set( m_pimpl->memc.get(),
                                MEMCACHED_BEHAVIOR_POLL_TIMEOUT,
                                timeout );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't set poll timeout" );

   // Set connect timeout
   rc = memcached_behavior_set( m_pimpl->memc.get(),
                                MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT,
                                timeout );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't set connect timeout" );

   // work around the fact that memcached_servers_parse 
   // doesn't take a const char*
   char* hostsDup = strdup( hosts.c_str() );
   memcached_server_st* serverList = memcached_servers_parse( hostsDup );
   free( hostsDup );

   if ( serverList == NULL ) {
      throw MemcachedException( "Couldn't parse server string" );
   }

   rc = memcached_server_push( m_pimpl->memc.get(), 
                               serverList );
   memcached_server_list_free( serverList );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't add servers" );
}

Cache::~Cache() {
}

void Cache::add( const MC2String& key,
                 const void* data,
                 size_t dataLength,
                 time_t expiration /*= 0*/,
                 uint32_t flags /*= 0*/ ) {
   SysUtility::IgnorePipe ignorePipe;

   memcached_return rc = memcached_add( m_pimpl->memc.get(),
                                        key.c_str(),
                                        strlen( key.c_str() ),
                                        (const char*)data,
                                        dataLength,
                                        expiration,
                                        flags );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't add a value" );
}

void Cache::set( const MC2String& key, 
                 const void* data, 
                 size_t dataLength,
                 time_t expiration /*= 0*/,
                 uint32_t flags /*= 0*/ ) {
   SysUtility::IgnorePipe ignorePipe;

   memcached_return rc = memcached_set( m_pimpl->memc.get(),
                                        key.c_str(),
                                        strlen( key.c_str() ),
                                        (const char*)data,
                                        dataLength,
                                        expiration,
                                        flags );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't set a value" );
}

std::pair<boost::shared_ptr<void>, size_t> 
Cache::get( const MC2String& key, 
            uint32_t* flags /*= NULL*/ ) {
   SysUtility::IgnorePipe ignorePipe;
   size_t valueLength;
   memcached_return rc;
   uint32_t flagsLocal;

   boost::shared_ptr<void> storedValue( memcached_get( m_pimpl->memc.get(),
                                                       key.c_str(),
                                                       strlen( key.c_str() ),
                                                       &valueLength,
                                                       &flagsLocal,
                                                       &rc ),
                                        STLUtility::FreeDeleter() );

   if ( storedValue != NULL ) {
      std::pair<boost::shared_ptr<void>, size_t> result( storedValue , 
                                                         valueLength );

      if ( flags != NULL ) {
         *flags = flagsLocal;
      }

      return result;
   } else {
      throw MemcachedException( MC2String( "Couldn't get a value: " ) +
                                memcached_strerror( m_pimpl->memc.get(), rc ) );
   }
}

void Cache::remove( const MC2String& key ) {
   SysUtility::IgnorePipe ignorePipe;

   memcached_return rc = memcached_delete( m_pimpl->memc.get(),
                                           key.c_str(),
                                           strlen( key.c_str() ),
                                           0 );
   
   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't delete a value" );
}

void Cache::flush() {
   SysUtility::IgnorePipe ignorePipe;

   memcached_return rc = memcached_flush( m_pimpl->memc.get(), 0 );

   throwOnFail( rc, m_pimpl->memc.get(), "Couldn't flush" );
}

}

#endif // HAVE_MEMCACHED

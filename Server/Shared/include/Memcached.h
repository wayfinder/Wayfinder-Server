/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __MEMCACHED_CPPWRAP_H__
#define __MEMCACHED_CPPWRAP_H__

#ifdef HAVE_MEMCACHED

#include "config.h"

#include <boost/shared_ptr.hpp>
#include <DataBuffer.h>
#include <MC2Exception.h>

/// Namespace for classes related to memcached
namespace Memcached {

/// This exception is thrown on errors in this namespace
class MemcachedException : public MC2Exception {
public:
   MemcachedException( const MC2String& what );
};

/** 
 * A cache using memcached.
 * 
 * This is a thin wrapper around the most important parts of libmemcached.
 * A note about race conditions: add and set are designed to prevent race
 * conditions if used properly. Use add to add new things and set to update
 * existing things. add will throw exception if the key already exists whereas
 * set will unconditionally change the value (potentially create the 
 * key/value-pair).
 * 
 */
class Cache {
public:
   /** Create a cache using a list of hosts.
    * @param hosts   A list of hosts like "localhost, foo:500, foo, bar"
    *                All hosts except foo:555 will be set to the default port,
    *                while that host will have a port of 555.
    * @throws  MemcachedException.
    */
   Cache( const MC2String& hosts );
   
   virtual ~Cache();
   
   /** Adds a key/value pair
    * @param key        The key.
    * @param data       The value.
    * @param dataLength The length of value data.
    * @param expiration An optional expiration time for the value (seconds).
    * @param flags      Optional flags to associate with the value.
    * @throws MemcachedException.
    */
   void add( const MC2String& key,
             const void* data,
             size_t dataLength,
             time_t expiration = 0,
             uint32_t flags = 0 );
   
   /** Sets a value for a specified key.
    * @param key        The key.
    * @param data       The value.
    * @param dataLength The length of value data.
    * @param expiration An optional expiration time for the value (seconds).
    * @param flags      Optional flags to associate with the value.
    * @throws MemcachedException.
    */
   void set( const MC2String& key, 
             const void* data, 
             size_t dataLength,
             time_t expiration = 0,
             uint32_t flags = 0 );

   /** Gets a value associated with a key.
    * @param   key     The key.
    * @param   flags   Optionally get flags associated with the value.
    * @returns A pointer to the value and the size of the value in a pair.
    * @throws MemcachedException.
    */
   std::pair<boost::shared_ptr<void>, size_t> get( const MC2String& key, 
                                                   uint32_t* flags = NULL );

   /** Removes a key/value pair from the cache.
    * @param key  The key.
    * @throws MemcachedException.
    */
   void remove( const MC2String& key );
   
   /** Flushes the cache (removes all content).
    * @throws MemcachedException.
    */
   void flush();

private:
   struct Implementation;
   std::auto_ptr<Implementation> m_pimpl;
};

/**
 * A simple to use cache which doesn't throw exceptions on error from
 * memcached. If a server is down for instance when you do a get, this
 * class will behave simply as if the key/value looked for doesn't
 * exist in the cache.
 * 
 * The values put into the cache should be of type T, which must have
 * methods to serialize and load from a DataBuffer.
 */
template<typename T>
class SimpleNoThrowCache {
public:
   
   /**
    * Create a cache.
    * 
    * @param hosts   See description of format in Cache class.
    * @param prefix  If supplied this string will be used as a prefix
    *                for all keys so that several caches can share a
    *                memcached cluster easily.
    */
   SimpleNoThrowCache( const MC2String& hosts,
                       const MC2String& prefix = MC2String() ) 
   : m_cache( NULL ), m_prefix( prefix ), m_needsToFlush( false ) {
      try {
         m_cache.reset( new Cache( hosts ) );
      } catch ( const MemcachedException& e ) {
         mc2log << error << "Failed to create Memcached::Cache object" << endl;
         mc2log << error << e.what() << endl;
      }
   }
   
   virtual ~SimpleNoThrowCache() {}
   
   /**
    * Adds a key/value-pair to the cache.
    * 
    * @param key           The key.
    * @param value         The value.
    * @param expiration    Optional expiration time (seconds).
    * @param flags         Optional flags associated with the value.
    */
   void add( const MC2String& key,
             const T& value,
             time_t expiration = 0,
             uint32_t flags = 0 ) {
      flushIfNeeded();
      if ( !m_needsToFlush && m_cache.get() != NULL ) {
         DataBuffer buffer( value.getDataBuffer() );
         
         try {
            m_cache->add( m_prefix+key, 
                          buffer.getBufferAddress(), 
                          buffer.getBufferSize(), 
                          expiration, 
                          flags );
         } catch ( const MemcachedException& e ) {
            // silently ignore
         }
      }
   }
   
   /**
    * Sets the value for a key.
    * 
    * @param key        The key.
    * @param value      The value.
    * @param expiration Optional expiration time (seconds).
    * @param flags      Optional flags associated with the value.
    */
   void set( const MC2String& key, 
             const T& value,
             time_t expiration = 0,
             uint32_t flags = 0 ) {
      flushIfNeeded();
      if ( !m_needsToFlush && m_cache.get() != NULL ) {
         DataBuffer buffer( value.getDataBuffer() );
         
         try {
            m_cache->set( m_prefix+key, 
                          buffer.getBufferAddress(), 
                          buffer.getBufferSize(), 
                          expiration, 
                          flags );
         } catch ( const MemcachedException& e ) {
            // silently ignore
         }
      }
   }
   
   /**
    * Get the value for a key.
    * 
    * @param key     The key.
    * @param value   The value.
    * @param flags   Optionally get the flags associated with the value.
    * @return Wether the key/value-pair existed.
    */
   bool get( const MC2String& key,
             T& value,
             uint32_t* flags = NULL ) {
      flushIfNeeded();
      if ( !m_needsToFlush && m_cache.get() != NULL ) {
         try {
            std::pair<boost::shared_ptr<void>, size_t> result = 
               m_cache->get( m_prefix+key, flags );

            DataBuffer buffer( (byte*)result.first.get(), result.second );
            value.load( buffer );
            return true;
         } catch ( MemcachedException& e ) {
            // treat as if there's no such element
         }
      }
      return false;
   }
   
   /**
    * Remove a key/value pair.
    * 
    * @param key  The key.
    */
   void remove( const MC2String& key ) {
      flushIfNeeded();
      if ( !m_needsToFlush && m_cache.get() != NULL ) {
         try {
            m_cache->remove( m_prefix+key );
         } catch ( MemcachedException& e ) {
            // silently ignore
         }
      }
   }
   
   /**
    * Removes all entries in the cache.
    */
   void flush() {
      if ( m_cache.get() != NULL ) {
         try {
            m_cache->flush();
            m_needsToFlush = false;
         } catch ( MemcachedException& e ) {
            // Can't trust the contents of the cache until
            // it has been flushed...
            m_needsToFlush = true;

            mc2log << error << "Failed to flush memcached" << endl;
            mc2log << error << e.what() << endl;
         }
      }
   }
   
private:

   void flushIfNeeded() {
      if ( m_needsToFlush ) {
         flush();
      }
   }

   std::auto_ptr<Cache> m_cache;  ///< The memcached Cache
   MC2String m_prefix;            ///< An optional prefix
   bool m_needsToFlush;           ///< Set to true if a flush has failed
};

}

#endif // HAVE_MEMCACHED

#endif // __MEMCACHED_CPPWRAP_H__

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MONITORCACHE_H
#define MONITORCACHE_H

// Includes
#include "config.h"
#include "Cache.h"
// ISABMonitor
#include "ISABThread.h"


/**
 *    MonitorCache, holds a cache of CacheElements.
 *    Is multithread safe, atleast for ISABThreads.
 *    
 */
class MonitorCache : public Cache, public ISABMonitor {
 public:
   /**
    * @param maxCacheSize is the maximum size of the cache in chars(bytes).
    * @param maxElements is the maximum number of elements in the cache.
    * @param startSize is the initial number of possible entries in 
    *        the cache.
    * @param incrementSize is the size of with the cache grows.
    */
   MonitorCache( CacheElement::CACHE_TYPE type,
                 uint32 maxCacheSize = 1048576,
                 uint32 maxElements = 1000 );

   
   /**
    * Destructor
    */
   virtual ~MonitorCache();


   /**
    * Finds a match.
    * Updates the last used time and adds a lock to the element.
    * @param ID is the ID of the element.
    * @param type is the type of CacheElement to search for.
    * @return the found element, null if none found.
    */
   CacheElement* find(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type);

   
   /**
    * Finds a match.
    * Updates the last used time and adds a lock to the element.
    * @param keyElement the CacheElement to use == on to find the match.
    * @return the first CacheElement in the cache with the same key as 
    *         keyElement
    */
   CacheElement* find(CacheElement* keyElement);


   /**
    * Checks if there is a CacheElement mathing keyElement.
    * Doesn't lock the element.
    * @return true if there is a match, false otherwise.
    */
   bool exists(CacheElement* keyElement);

   
   /**
    * Checks if there is a CacheElement mathing keyElement.
    * Doesn't lock the element.
    * @param ID is the ID of the element.
    * @param type is the type of CacheElement to search for.
    * @return true if there is a match, false otherwise.
    */
   bool exists(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type);


   /**
    * Adds a element to the cache.
    * Removes some if too much space.
    * @param element is the CacheElement to add to the cache.
    */
   void add(CacheElement* element);


   /**
    * Adds e element to the cache and locks it.
    * Removes some old contents if needed.
    * @param element is the element to and and lock.
    * @return the locked element.
    */
   CacheElement* addAndLock(CacheElement* element);


   /**
    * Removes a element from the cache.
    * Checks if the element is locked.
    * @param element is the element to remove.
    * @return true if the element was removed false otherwise.
    */
   bool remove(CacheElement* element);

   
   /**
    * Frees a locked element.
    */
   void releaseCacheElement(CacheElement* element);


   /**
    * Gets the maxsize of the cache.
    */
   uint32 getMaxSize();
   
   /**
    * Sets the maxsize of the cache.
    */
   void setMaxSize(uint32 newMax);


   /**
    * Gets the currentSize od the cache.
    */
   uint32 getCurrentSize();


   /**
    * Gets the maximun of cacheelements.
    */
   uint32 getMaxElements();


   /**
    * Sets the maximun number of elements.
    */
   void setMaxElements(uint32 newMax);


   /**
    * Gets the number of elements in the cache
    */
   uint32 getCurrentElementcount();

   /**
    *   Get the current size and elementcount for removed but not
    *   yet released elements.
    *   Note that this calculates the sizes each call!
    *   Use with care.
    */
   uint32 getCurrentRemovedSize( uint32* nbr = NULL ) const;

 private:

};

#endif // MONITORCACHE

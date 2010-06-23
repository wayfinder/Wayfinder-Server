/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CACHE_H
#define CACHE_H

// Includes
#include "config.h"
#include "CacheElement.h"
#include "NotCopyable.h"

#include <set>


// Defines
// How much used space after clean
#define CLEAN_QUOTA 0.50


/**
 *   Cache, holds a cache of CacheElements.
 *
 */
class Cache: private NotCopyable {
   public:
      /**
       *   Create a new cache with possibility to set some initial 
       *   values.
       *
       *   @param   type           The type of this Cache, client or 
       *                           server.
       *   @param   maxCacheSize   The maximum size of this cache in 
       *                           chars (bytes).
       *   @param   maxElements    The maximum number of elements in the 
       *                           cache.
       */
      Cache( CacheElement::CACHE_TYPE type,
             uint32 maxCacheSize = 1048576,
             uint32 maxElements = 1000 );
      

      /**
       *   Delete this cache and all the elements in it.
       */
      virtual ~Cache();


      /**
       *   Find a match. Updates the last used time and adds a lock to 
       *   the element.
       *
       *   @param ID   Tthe ID of the element.
       *   @param type The type of CacheElement to search for.
       *   @return The found element, NULL if none found.
       */
      CacheElement* find(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type);
      

      /**
       *   Find a match. Updates the last used time and adds a lock to 
       *   the element.
       *
       *   @param keyElement The CacheElement to use == on to find the 
       *                     match.
       *   @return The first CacheElement in the cache with the same key
       *           as keyElement
       */
      CacheElement* find(CacheElement* keyElement);


      /**
       *   Check if there is a CacheElement mathing keyElement.
       *   Doesn't lock the element.
       *
       *   @param keyElement is the element to match.
       *   @return True if there is a match, false otherwise.
       */
      bool exists(CacheElement* keyElement) const;
      

      /**
       *   Check if there is a CacheElement mathing keyElement.
       *   Doesn't lock the element.
       *
       *   @param ID   The ID of the element.
       *   @param type The type of CacheElement to search for.
       *   @return True if there is a match, false otherwise.
       */
      bool exists(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type) const;
      
      
      /**
       *   Add a element to the cache. Removes some if too much space.
       *   @param element The CacheElement to add to the cache.
       */
      void add(CacheElement* element);
      
      
      /**
       *   Add a element to the cache and locks it. Removes some old 
       *   contents if needed.
       *
       *   @param element The element to and and lock.
       *   @return The locked element.
       */
      CacheElement* addAndLock(CacheElement* element);
      

      /**
       *   Remove one element from the cache. Checks if the element is 
       *   locked. Deletes the element eventually, after all locks are 
       *   released.
       *
       *   @param element The element to remove.
       *   @return True if the element was removed false otherwise.
       */
      bool remove(CacheElement* element);
      
      
      /**
       *   Free a locked element.
       *   @param element is the locked element to release.
       *   @param checkCapacity If true the capacity will be checked
       *          and the cache may be cleaned.
       */
      void releaseCacheElement( CacheElement* element, 
                                bool checkCapacity = true );
      
      
      /**
       *   Get the maxsize of the cache.
       *   @return  The maximum size of this cache.
       */
      inline uint32 getMaxSize() const;
      
      
      /**
       *   Set the maxsize of the cache in bytes.
       *   @param   newMax   The new maximum value of the number of 
       *                     bytes in this cache.
       */
      void setMaxSize(uint32 newMax);
      
      
      /**
       *   Get the currentSize of the cache in bytes.
       */
      uint32 getCurrentSize() const;


      /**
       *   Get the maximun number of cacheelements.
       */
      inline uint32 getMaxElements() const;
      
      
      /**
       *   Set the maximun number of elements.
       *   @param   newMax   The new maximum number of elements in this 
       *                     cache.
       */
      void setMaxElements(uint32 newMax);
      
      
      /**
       *   Get the number of elements in the cache.
       *   @return  The current number of elements in this cache.
       */
      uint32 getCurrentElementcount() const;
      
      
      /**
       *   Get the type of this cache.
       *   @return The type of cache.
       */
      inline CacheElement::CACHE_TYPE getType() const;


      /**
       *   Clean up the cache if nessesary.
       *   @return True if the cache was cleaned, false otherwise.
       */
      bool clean();

      /**
       *   Get the current size and elementcount for removed but not
       *   yet released elements.
       *   Note that this calculates the sizes each call!
       *   Use with care.
       */
      uint32 getCurrentRemovedSize( uint32* nbr = NULL ) const;
      
   private:
      typedef multiset< CacheElement*, CacheElementCmpLess > CacheSet;

      /**
       * Class for sorting pairs of uint32 and CacheSet::iterator.
       */
      class sortedVectLess {
         public:
         inline bool operator() ( 
            const pair< uint32, CacheSet::iterator >& a,
            const pair< uint32, CacheSet::iterator >& b ) const 
         {
            return a.first < b.first;
         }
      };


      /// The CacheElements.
      CacheSet* m_vector;
      
      
      /// Temporary vector for removed elements still in use.
      CacheSet* m_tmpVector;
      
      
      /**
       *   @name Cache-sizes.
       *   Members to hold some size-information about the cache.
       */
      //@{
         /// The current size of this cache.
         uint32 m_currentSize;
         
         /// The maximum size of this cache.
         uint32 m_maxSize;
         
         /// The current number of elements in this cache.
         uint32 m_currentElements;

         /// The maximum number of elements allowed in this cache.
         uint32 m_maxElements;
      //@}


      /// The element used to search within the CacheSet
      CacheElement* searchElement;
      
      
      /// The type of this cache.
      CacheElement::CACHE_TYPE m_type;
      
      
      /**
       *   Checks all sizes.
       *   @return True if the cache needs to be cleaned, false otherwise.
       */
      bool checkCapacity();
            
      /**
       * The internal next cacheElementID. Used to differ between
       * a removed but not yet released element from a addAndLockeded 
       * element with same id as the removed. Both have same id so
       * which to release?
       */
      uint32 m_currentElementID;


      /**
       * The next elementID. All added elements gets a unique elementID.
       */
      inline uint32 getNextElementID();
};


// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -


inline uint32 
Cache::getMaxElements() const { 
   return m_maxElements; 
}


inline uint32 
Cache::getMaxSize() const { 
   return m_maxSize; 
}


inline CacheElement::CACHE_TYPE 
Cache::getType() const { 
   return m_type; 
}


uint32 
Cache::getNextElementID() {
   if ( m_currentElementID == 0 ) {
      m_currentElementID++;
   }
   return m_currentElementID++;
}


#endif // CACHE

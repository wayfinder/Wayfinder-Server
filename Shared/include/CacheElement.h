/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CACHEELEMENT_H
#define CACHEELEMENT_H

#include "config.h"

/**
 *    CacheElement. Class with no actual content.
 *
 */
class CacheElement {
   public:
      /** 
       *    Type of CacheElement. Define the possible elementtypes, 
       *    don't forget to update NBR_CACHE_ELEMENT_TYPES as it is 
       *    used in a vector in Cache.
       */
      enum CACHE_ELEMENT_TYPE{GFX_ELEMENT_TYPE         = 0,
                              STRINGTABLE_ELEMENT_TYPE = 1,
                              USER_ELEMENT_TYPE        = 2,
                              ISAGFX_ELEMENT_TYPE      = 3,
                              USER_DATA_ELEMENT_TYPE   = 4,
                              OVERVIEW_ELEMENT_TYPE    = 5,
                              CELLULAR_MODELS_ELEMENT_TYPE = 6,
                              NBR_CACHEELEMENT_TYPES };
                               

      /// Cache in server or client
      enum CACHE_TYPE{ CLIENT_TYPE = 0,
                       SERVER_TYPE = 1};

      /**
       * Constructor, sets use-time to now.
       */
      CacheElement(uint32 ID, 
                   CACHE_ELEMENT_TYPE element_type, 
                   CACHE_TYPE cache_type);

      /**
       * Destructor. Does nothing.
       */
      inline virtual ~CacheElement();

      /** 
       *    @name Operators
       *    Operators, to search the elements.
       */
      //@{
         /// equal
         bool operator == (const CacheElement& el) const {
            return m_ID == el.m_ID && m_type == el.m_type; 
         }

         /// not equal
         bool operator != (const CacheElement& el) const {
            return m_ID != el.m_ID || m_type != el.m_type; 
         }

         /// greater than
         bool operator > (const CacheElement& el) const {
            return m_ID > el.m_ID;
         }

         /// less than
         bool operator < (const CacheElement& el) const {
            return m_ID < el.m_ID;
         }
      //@}
      

      /**
       * @return the size of the element.
       */
      inline virtual uint32 getSize() const;


      /**
       * @return ID
       */
      uint32 getID() const { return m_ID; }
      
      
      /**
       * @return type
       */
      CACHE_ELEMENT_TYPE getType() const { return m_type; }
      
      
      /**
       * @return the type of cache.
       */
      CACHE_TYPE getCacheType() const { return m_cacheType; }


      /**
       * Add a lock to the element
       */
      inline void addLock();


      /**
       * Remove a lock on the element.
       */
      inline void removeLock();


      /**
       * Returns the number of locks on the element.
       */
      inline uint32 getNumberLocks() const;

      
      /**
       * @return true if cache item is valid
       */
      bool isValid() const { return m_isValid; }
      

      /**
       * @param isValid tells if cache item is valid or not
       */
      void setValid( bool isValid ) { m_isValid = isValid; }


   protected: 
      /// The id of the element
      uint32 m_ID;
      
      
      /// Used for invlaidate cacheelement
      bool m_isValid;
   
   
      /// The type of element
      CACHE_ELEMENT_TYPE m_type;

   private:
      /// The last time the element was used
      uint32 m_lastUseTime;


      /// Used for cleaning the cache
      bool m_isChecked;
      
      
      /// The type of cache this element is in.
      CACHE_TYPE m_cacheType;
      
   
      /// The number of user of this element
      uint32 m_locks;
      
      /**
       * Sets the last used time.
       * @param newTime is the new last used time.
       */
      inline void setLastUsedTime(uint32 newTime);
      
      /**
       * Returns the last use time of the element.
       * @return last use time.
       */
      inline uint32 getLastUsedTime() const;
      
      
      /**
       * Sets the ID of the element.
       * @param ID the new ID.
       */
      inline void setID(uint32 ID) { m_ID = ID; }
      
      
      /**
       * Sets the type of the element.
       * @param type is the new type.
       */
      void setType(CACHE_ELEMENT_TYPE type) { m_type = type; }
      

      /**
       * The elementID of this CacheElement.
       */
      uint32 m_elementID;


      /**
       * The elementID of this CacheElement.
       */
      inline uint32 getElementID() const;


      /**
       * Set the elementID of this CacheElement.
       */
      inline void setElementID( uint32 elID );
      

      // Cache handles CacheElements
      friend class Cache;
};


inline 
CacheElement::~CacheElement() 
{
}


inline void
CacheElement::setLastUsedTime(uint32 newTime) 
{
   m_lastUseTime = newTime;
}


inline uint32
CacheElement::getLastUsedTime() const 
{
   return m_lastUseTime;
}


inline uint32
CacheElement::getSize() const 
{
   return 0;
}


inline void 
CacheElement::addLock() 
{
   m_locks++;
}


inline void 
CacheElement::removeLock() 
{
   m_locks--;
}


inline uint32 
CacheElement::getNumberLocks() const 
{
   return m_locks;
}


inline uint32 
CacheElement::getElementID() const 
{
   return m_elementID;
}


inline void 
CacheElement::setElementID( uint32 elID ) 
{
   m_elementID = elID;
}


/**
 * Class for comparing if CacheElements are less.
 */
class CacheElementCmpLess {
   public:
      bool operator()(const CacheElement* a, const CacheElement* b ) const 
      {
         return *a < *b;
      }
      bool operator()(const CacheElement& a, const CacheElement& b ) const
      {
         return a < b;
      }
};

#endif // CACHEELEMENT_H

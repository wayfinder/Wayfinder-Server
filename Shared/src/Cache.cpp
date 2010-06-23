/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
#   ifndef DEBUG_LEVEL_1
#      define DEBUG_LEVEL_1
#      define undef_level_1
#   endif
#   ifndef DEBUG_LEVEL_2
#      define DEBUG_LEVEL_2
#      define undef_level_2
#   endif
#   ifndef DEBUG_LEVEL_4
#      define DEBUG_LEVEL_4
#      define undef_level_4
#   endif
#   ifndef DEBUG_LEVEL_8
#      define DEBUG_LEVEL_8
#      define undef_level_8
#   endif
*/

#include "Cache.h"

#include "TimeUtility.h"
#include <vector>
#include <algorithm>

 
Cache::Cache(CacheElement::CACHE_TYPE type,
             uint32 maxCacheSize, 
             uint32 maxElements)
{
   m_type            = type;
   m_maxSize         = maxCacheSize;
   m_currentSize     = 0;
   m_maxElements     = maxElements;
   m_currentElements = 0;
   m_currentElementID = 1;
  
   m_vector = new CacheSet[ CacheElement::NBR_CACHEELEMENT_TYPES ];
   m_tmpVector = new CacheSet[CacheElement::NBR_CACHEELEMENT_TYPES];

   searchElement = new CacheElement( 0, CacheElement::GFX_ELEMENT_TYPE, 
                                    m_type );
}


Cache::~Cache() {
   mc2dbg4 << "Deleting elements in Cache." << endl;
   for ( int k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++) {
      for ( CacheSet::iterator it = m_vector[ k ].begin() ;
            it != m_vector[ k ].end() ; ++it )
      {
         delete *it;
      }
   }
   delete [] m_vector;
   delete searchElement;

   for ( int l = 0 ; l < CacheElement::NBR_CACHEELEMENT_TYPES ; l++) {
      for ( CacheSet::iterator it = m_tmpVector[ l ].begin() ;
            it != m_tmpVector[ l ].end() ; ++it )
      {
         delete *it;
      }
   }
   delete [] m_tmpVector;

}


CacheElement* Cache::find(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type)
{
   searchElement->setID(ID);
   searchElement->setType(type);
   
   return find( searchElement );
}


CacheElement*
Cache::find(CacheElement* keyElement) {
   CacheElement::CACHE_ELEMENT_TYPE type = keyElement->getType();
   
   CacheSet::iterator el = m_vector[ type ].find( keyElement );
   if ( el != m_vector[ type ].end() ) {
      (*el)->setLastUsedTime( TimeUtility::getRealTime() );
      (*el)->addLock();
      mc2dbg4 << "Find locking " << (*el)->getID() << endl;
      return *el;
   } else {
      return NULL;
   }
}


bool
Cache::exists(uint32 ID, CacheElement::CACHE_ELEMENT_TYPE type) const {
   searchElement->setID(ID);
   searchElement->setType(type);

   return exists( searchElement );
}


bool
Cache::exists(CacheElement* keyElement) const {
   CacheElement::CACHE_ELEMENT_TYPE type = keyElement->getType();
   
   CacheSet::iterator el = m_vector[ type ].find( keyElement );
   if ( el != m_vector[ type ].end() ) {
      return true;
   } else {
      return false;
   }
}


void
Cache::add(CacheElement* element) {
   mc2dbg8 << "Adding element: " << element->getID()<< endl;
   if ( element->isValid() ) {
      element->setElementID( getNextElementID() );
      m_vector[ element->getType() ].insert( element );
      m_currentSize += element->getSize();
      m_currentElements++;
      if ( (m_currentSize > m_maxSize) || 
           (m_currentElements > m_maxElements) ) 
      {
         mc2dbg8 << "Too much in cache" << endl;
         if ( checkCapacity() ) { // Recheck if something has changed
            mc2dbg8 << "Still too much in cache, cleaning" << endl;
            clean();
         }
      }
   }
}


CacheElement*
Cache::addAndLock(CacheElement* element) {
   mc2dbg8 << "Addandlock element: " << element->getID()<< endl;
   if ( element->isValid() ) {
      element->setElementID( getNextElementID() );
      m_vector[ element->getType() ].insert( element );
      element->addLock();
      m_currentSize += element->getSize();
      m_currentElements++;
      if ( (m_currentSize > m_maxSize) || 
           (m_currentElements > m_maxElements) ) 
      {
         mc2dbg8 << "Too much in cache2" << endl;
         if ( checkCapacity() ) { // Recheck if something has changed
            mc2dbg8 << "Still too much in cache, cleaning" << endl;
            clean();
         }
      }
   }
   return element;
}


bool 
Cache::remove(CacheElement* element) {
   mc2dbg8 << "Removing Element " << element->getID() << endl;
   CacheSet::iterator el = m_vector[ element->getType() ].find( element );

   if ( el != m_vector[ element->getType() ].end() ) {
      CacheElement* cel = *el;
      mc2dbg8 << "Element found, locks = " << (*el)->getNumberLocks()
              << endl;
      if ( cel->getNumberLocks() == 0 ) {
         mc2dbg8 << "Element removeable" << endl;
         m_vector[ cel->getType() ].erase( el );
         m_currentSize -= cel->getSize();
         m_currentElements--;
         mc2dbg8 << "Element removed" << endl;
         delete cel;
         return true;
      } else { // Move to tmp-vector and wait for release
         mc2dbg8 << "Element moved to tmp-vector avaiting deletion."
                 << " type " << cel->getType() << endl;
         m_vector[ cel->getType() ].erase( el );
         m_currentSize -= cel->getSize();
         m_currentElements--;
         m_tmpVector[ cel->getType() ].insert( cel );
         return true;
      }
   } else {
      return false;
   }
}


bool 
Cache::checkCapacity() {
   m_currentSize = 0;
   m_currentElements = 0;
   for ( int k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++) {
      for ( CacheSet::iterator it = m_vector[ k ].begin() ;
            it != m_vector[ k ].end() ; ++it )
      {
         m_currentSize += (*it)->getSize();
      }
      m_currentElements += m_vector[ k ].size();
   }
   return (m_currentSize > m_maxSize) || 
      (m_currentElements > m_maxElements);
}


bool
Cache::clean() {
   if ( (m_currentSize > m_maxSize) || 
        (m_currentElements > m_maxElements)) {
      mc2dbg4 << "Cleaning..." << endl;
      m_currentSize = 0;
      m_currentElements = 0;
      for ( int k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++) {
         for ( CacheSet::iterator it = m_vector[ k ].begin() ;
               it != m_vector[ k ].end() ; ++it )
         {
            m_currentSize += (*it)->getSize();
         }
         m_currentElements += m_vector[ k ].size();
      }
      mc2dbg4 << "Current size = " << m_currentSize << " count = " 
              << m_currentElements << endl;
      // Cleanup, remove oldest half
      mc2dbg4 << "Cache cleanup starts " << endl;
      uint32 rstartTime = TimeUtility::getCurrentMicroTime();
      uint32 startTime = rstartTime;

      typedef vector< pair< uint32, CacheSet::iterator > > SortedVect;
      SortedVect sortedVect[ CacheElement::NBR_CACHEELEMENT_TYPES ];

      for ( int k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++) {
         startTime = TimeUtility::getCurrentMicroTime();
         for ( CacheSet::iterator it = m_vector[ k ].begin() ;
               it != m_vector[ k ].end() ; ++it )
         {
            sortedVect[ k ].push_back( make_pair( (*it)->getLastUsedTime(),
                                                  it ) );
         }
         uint32 endTime = TimeUtility::getCurrentMicroTime();
         mc2dbg8 << "Time for sortedVect " << k << " creation is " \
                 << (endTime-startTime) << " us"  << endl;
         startTime = TimeUtility::getCurrentMicroTime();
         std::sort( sortedVect[ k ].begin(), sortedVect[ k ].end(),
                    sortedVectLess() );
         endTime = TimeUtility::getCurrentMicroTime();
         mc2dbg8 << "Time to sort sortedVect " << k << " is " \
                 << (endTime-startTime) << " us"  << endl;
      }

      bool more = true;
      vector< SortedVect::iterator > frontIts;
      for ( int k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++) {
         frontIts.push_back( sortedVect[ k ].begin() );
      }

      while ( (more) && 
              ((m_currentSize/(double)m_maxSize > CLEAN_QUOTA) ||
               (m_currentElements/double(m_maxElements) > CLEAN_QUOTA ) ) )
      {
         uint32 oldestTime = MAX_UINT32;
         uint32 deleteAbleVector = MAX_UINT32;
         // Get the oldest of the first of each vector in sortedVect 
         for ( int k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++ )
         {
            if ( frontIts[ k ] != sortedVect[ k ].end() &&
                 (*frontIts[ k ]->second)->getLastUsedTime() < oldestTime&&
                 (*frontIts[ k ]->second)->getNumberLocks() == 0 )
            {
               deleteAbleVector = k;
               oldestTime = 
                  (*frontIts[ k ]->second)->getLastUsedTime();
            }
         }

         mc2dbg8 << "Found oldest in " << deleteAbleVector << endl;
         
         if ( deleteAbleVector != MAX_UINT32 ) {
            m_currentSize -= 
               (*frontIts[ deleteAbleVector ]->second)->getSize();
            m_currentElements--;
            CacheElement* delEl = *frontIts[ deleteAbleVector ]->second;
            m_vector[ deleteAbleVector ].erase( 
               *frontIts[ deleteAbleVector ]->second );
            frontIts[ deleteAbleVector ]++;
            delete delEl;
         } else {
            // No more elements!
            more = false;
         }
      }
      
      uint32 endTime = TimeUtility::getCurrentMicroTime();
      mc2dbg4 << "Time for Cache cleanup is " \
              << (endTime-rstartTime) << " us size " << m_currentSize
              << " count " << m_currentElements << endl;

      return true;
   } else
      return false;
}


void
Cache::releaseCacheElement(CacheElement* element, bool checkCapacity) {
   mc2dbg8 << "releaseCacheElement " << element->getID() << endl;
   CacheElement::CACHE_ELEMENT_TYPE type = element->getType();
   CacheSet::iterator el = m_vector[ type ].find( element );
   CacheSet::iterator tel = m_tmpVector[ type ].end();
   bool foundInVector = false;
   bool foundInDeleted = false;

   if ( el != m_vector[ type ].end() && 
        (*el)->getElementID() == element->getElementID() )
   {
      mc2dbg8 << "Found in Vector!" << endl;
      foundInVector = true; 
   } else {
      tel = m_tmpVector[ type ].lower_bound( element );
      mc2dbg8 << "Searching in tmpVector start " << endl;
      while ( tel != m_tmpVector[ type ].end() ) {
         if ( *(*tel) == *element ) {
            if ( (*tel)->getElementID() == element->getElementID() ) {
               // Found match
               mc2dbg8 << "Found in tmpVector" << endl;
               foundInDeleted = true;
               break;
            }
         } else {
            // End of elements that are identical
            break;
         }
         tel++;
      }

      if ( ! foundInDeleted ) {
         if ( el != m_vector[ type ].end() ) {
            mc2dbg8 << "Found in Vector, but not right elementID" << endl;
            foundInVector = true; 
         } else if ( tel != m_tmpVector[ type ].end() ) {
            mc2dbg8 << "Found in tmpVector, but not right elementID" 
                    << endl;
            tel = m_tmpVector[ type ].lower_bound( element );
            foundInDeleted = true;
         }
      }
   }

   if ( foundInVector ) {
      mc2dbg8 << "Removing lock on " << (*el)->getID() << " in Vector"
              << endl;
      (*el)->removeLock();
      if ( checkCapacity && Cache::checkCapacity() ) clean();
   } else if ( foundInDeleted ) { // Check in tmp-vector
      mc2dbg8 << "Removing lock on " << (*tel)->getID() << " in tmpVector"
              << endl;
      (*tel)->removeLock();
      if ( (*tel)->getNumberLocks() == 0 ) {
         CacheElement* tmpEl = (*tel);
         m_tmpVector[ element->getType() ].erase( tel );
         delete tmpEl;
      }
   }
   
   // Debug
   DEBUG8( uint32 nbr = 0; 
           for ( uint32 k = 0 ; 
                 k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++ ) 
           {
              for ( CacheSet::iterator it = m_vector[ k ].begin() ;
                    it != m_vector[ k ].end() ; ++it )
              {
                 if ( (*it)->getNumberLocks() != 0 ) {
                    nbr++;
                 }
                 mc2dbg8 << "Type: " << k << " " 
                         << "ElementID: " << (*it)->getID() 
                         << " size " << (*it)->getSize() 
                         << " is locked " 
                         << (*it)->getNumberLocks() << " times" << endl;
              }
           }
           mc2dbg8 << "A total of " << nbr << " cacheelements are locked." 
                   << " of " << m_currentElements << endl;
           nbr = 0;
           for ( uint32 k = 0 ; 
                 k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++ ) 
           {
              for ( CacheSet::iterator it = m_tmpVector[ k ].begin() ;
                    it != m_tmpVector[ k ].end() ; ++it )
              {
                 if ( (*it) != NULL ) {
                    nbr++;
                    mc2dbg8 << "ElementID: " << (*it)->getID() 
                            << " removed but alive, locks " 
                            << (*it)->getNumberLocks() << endl;
                 }
              }
           }
           mc2dbg8 << "A total of " << nbr 
                   << " cacheelements removed but not released." << endl;
           mc2dbg8 << "Size,Count= " << m_currentSize << "," 
                   << m_currentElements << endl;
           );
}


void
Cache::setMaxSize(uint32 newMax) {
   if ( (newMax < m_maxSize) || 
        (m_currentSize > m_maxSize) || 
        (m_currentElements > m_maxElements) ) 
   {
      m_maxSize = newMax;
      clean();
   } else {
      m_maxSize = newMax;
   }
}


uint32
Cache::getCurrentSize() const {
   return m_currentSize;
}


void
Cache::setMaxElements(uint32 newMax) {
   if ( newMax < m_maxElements ||
        (m_currentSize > m_maxSize) || 
        (m_currentElements > m_maxElements) ) 
   {
      m_maxElements = newMax == 0 ? 1 : newMax;
      clean();
   } else {
      m_maxElements = newMax;
   }
}


uint32
Cache::getCurrentElementcount() const {
   return m_currentElements;
}

uint32
Cache::getCurrentRemovedSize( uint32* nbr ) const {
   uint32 size = 0;
   uint32 nbrEl = 0;

   for ( uint32 k = 0 ; k < CacheElement::NBR_CACHEELEMENT_TYPES ; k++ ) {
      for ( CacheSet::iterator it = m_tmpVector[ k ].begin() ;
            it != m_tmpVector[ k ].end() ; ++it ) {
         if ( (*it) != NULL ) {
            nbrEl++;
            size += (*it)->getSize();
         }
      }
   }

   if ( nbr != NULL ) {
      *nbr = nbrEl;
   }

   return size;
}


/*
#ifdef undef_level_1
#   undef DEBUG_LEVEL_1
#endif
#ifdef undef_level_2
#   undef DEBUG_LEVEL_2
#endif
#ifdef undef_level_4
#   undef DEBUG_LEVEL_4
#endif
#ifdef undef_level_8
#   undef DEBUG_LEVEL_8
#endif
*/

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALLOCATORTEMPLATES_H
#define ALLOCATORTEMPLATES_H

#include "DeleteHelpers.h"
#include "STLUtility.h"
#include "ArrayTools.h"

#if defined (__GNUC__) && __GNUC__ > 2
#include <ext/slist>
using namespace __gnu_cxx;
#else
#include <slist>
#endif

#include <map>

class GenericMap;

class AbstractMC2Allocator {
public:

   virtual ~AbstractMC2Allocator() {};
   virtual void reallocate(int n) = 0;
   virtual uint32 getBlockSize() const = 0;
   virtual uint32 getTotalNbrElements() const = 0;
};

/**
 *    Allocator for arrays.
 */
template<class T> class MC2ArrayAllocator : public AbstractMC2Allocator {
public:
   
   typedef map<T*, int> singeAllocMap_t;

   MC2ArrayAllocator( int size = 0 ) {
      m_warnSingleAlloc = false;
      m_bigArray = NULL;
      reallocate( size );
   }

   ~MC2ArrayAllocator() {
      reallocate(0);
   }

   uint32 getBlockSize() const {
      return m_nbrElementsUsed;
   }

   /**
    *    Returns the total number of elements, i.e. the total
    *    size that should be allocated to avoid using single alloc.
    *    Function only works if no addElements have been run on the
    *    block allocated.
    */
   uint32 getTotalNbrElements() const {
      uint32 sum = getBlockSize();
      for ( typename singeAllocMap_t::const_iterator it = m_singleAlloc.begin();
            it != m_singleAlloc.end();
            ++it ) {
         sum += it->second;
      }
      return sum;
   }
   
   /**
    *    Sets new size. Should only be called once, since
    *    it destroys all data inside.
    */
   void reallocate( int n ) {
      // Destroy the single alloc.
      for ( typename singeAllocMap_t::iterator it = m_singleAlloc.begin();
            it != m_singleAlloc.end();
            ++it ) {
         delete [] it->first;
      }
      m_singleAlloc.clear();

      // Clear and recreate the bigarray
      m_nbrElementsUsed = 0;
      delete [] m_bigArray;
      if ( n ) {
         m_bigArray = new T[n];
      } else {
         m_bigArray = NULL;
      }
      m_bigArraySize = n;
   }

   /// Set if warnings should be emitted for single alloc.
   void setWarnSingleAlloc( bool warnOrNot ) {
      m_warnSingleAlloc = warnOrNot;
   }
   
   /**
    *    Returns a pointer to an array with room for
    *    <code>size</code>
    *    entries.
    */
   T* getNewArray( int size ) {
      if ( size ==  0 ) {
         return NULL;
      }
      // Check if there is room in the big array.
      if ( ( m_nbrElementsUsed + size ) <= m_bigArraySize ) {
         mc2dbg8 << "[MC2ARRAY]: Using bigbuf (" << size << ")" << endl;
         T* oldPtr = m_bigArray + m_nbrElementsUsed;
         m_nbrElementsUsed += size;
         return oldPtr;
      } else {
         if ( m_warnSingleAlloc ) {
            mc2log << warn
                   << "[MC2ARRAY]: Single alloc(" << size << ")" << endl;
         }
         // Have to alloc a new one.
         T* newArray = new T[size];
         m_singleAlloc.insert( make_pair( newArray, size ) );
         return newArray;
      }
   }
   
   /**
    *    Adds a new element to an array. The array will be
    *    in the set of single alloc.
    */
   template<class S> T* addElement( T* array,
                                    const T& elem, S& nbrElems ) {
      if ( m_warnSingleAlloc ) {
         mc2log << warn << "[MC2ARRAY]: addElement called" << endl;
      }
      if ( inBigArray( array ) ) {
         // Get a new or old array
         T* tmpArray = getNewArray( nbrElems + 1 );
         // Copy old elements.
         for ( int i = 0, n=nbrElems; i < n; ++i ) {
            tmpArray[i] = array[i];            
         }
         // And add the last one.
         tmpArray[ nbrElems++ ] = elem;
         array = tmpArray;
      } else {
         // Not in the big array - remove it from the single alloc list
         // and realloc it.
         m_singleAlloc.erase( array );
         array = ArrayTool::addElement( array, elem, nbrElems );
         m_singleAlloc.insert( make_pair( array, nbrElems ) );
      }
      return array;
   }

private:

   bool inBigArray( const T* ptr ) {
      if ( ptr == NULL ) {
         return false;
      } else {
         return (ptr >= m_bigArray) && (ptr < (m_bigArray + m_bigArraySize ));
      }
   }

   singeAllocMap_t m_singleAlloc;
   T* m_bigArray;
   uint32 m_bigArraySize;
   uint32 m_nbrElementsUsed;
   bool m_warnSingleAlloc;
};

/**
 *    Used to make it possible to allocate lots of objects of one kind 
 *    at the same time. The objects are stored in an array to make it
 *    faster to allocate them and to make them use less memory. Called
 *    MC2Allocator to avoid mix with e.g. the Allocator in STL.
 * 
 */
template<class T> class MC2Allocator : public AbstractMC2Allocator
{
public:
   typedef STLUtility::DualForwardIterator<T, typename slist<T *>::iterator > iterator;
   typedef STLUtility::DualForwardIterator<const T, typename slist< T* >::const_iterator > const_iterator;

   /**
    *    Alloctea n objects with type T.
    *    @param n The number of elements to allocate.
    */
   MC2Allocator(int n = 0) {
      // Create an array of objects (_not_ of pointers to objects).
      MINFO("MC2Allocator(n) enter");
      //mc2dbg << "[MC2Allocator]: sizeof(T) == " << sizeof(T) << endl;
      m_block = NULL;
      initiate(n);
      MINFO("MC2Allocator(n) leave");
   };

   /**
    *    Delete this allocator an all memory allocated here.
    */
   virtual ~MC2Allocator() {
      MINFO("~MC2Allocator() enter");
      mc2dbg8 << "Deallocate allocator with " << m_blockAllocated 
              << " objects" << endl;
      // Will delete everything
      initiate(0);
      MINFO("~MC2Allocator() leave");
   };

   void initiate(int n) {
      delete [] m_block;
      if (n > 0)
         m_block = new T[n];
      else
         m_block = NULL;

      // Initiate the members
      m_blockAllocated = n;
      m_curBlockIndex = 0;

      // Delete all objects in m_singleAllocated
      for ( typename slist<T*>::iterator it = m_singleAllocated.begin();
            it != m_singleAllocated.end(); ++it) {
         delete *it;
      }

      m_singleAllocated.clear();
   }

   virtual void reallocate(int n) {
      delete [] m_block;
      if (n > 0)
         m_block = new T[n];
      else
         m_block = NULL;

      // Initiate the members
      m_blockAllocated = n;
      m_curBlockIndex = 0;

      STLUtility::deleteValues( m_singleAllocated );
   }

   virtual uint32 getBlockSize() const {
      return m_blockAllocated;
   }

   virtual uint32 getTotalNbrElements() const {
      return m_curBlockIndex + m_singleAllocated.size();
   }

   /**
    *    Get the next allocated object.
    *    @return An empty, allocated T-object.
    */
   T* getNextObject() {
      MINFO("   MC2Allocator()::getNextObject() enter");
      T* retVal = NULL;

      if (m_curBlockIndex < m_blockAllocated) {
         // OK, return pointer to object number m_curBlockIndex in m_block
         mc2dbg4 << "Allocator returned pointer to object" << endl;
         retVal = &(m_block[m_curBlockIndex]);
         ++m_curBlockIndex;
      } else {
         // m_block is full, allocate new, put into m_singleAllocated 
         // and return
         retVal = new T();
         mc2dbg4 << "Allocator returned new object" << endl;
         m_singleAllocated.push_front(retVal);
      }
         
      MINFO("   MC2Allocator()::getNextObject() leave");
      // Return retVal
      return retVal;
   };

   /**
    *    Hand over an object to the allocator. This mean that the
    *    destruction of the object will be handled by this allocator.
    *    If the object already is allocated here, nothing will happen
    *    (the pointers are checked) so this method should be safe to 
    *    call. Since the allocator not is optimized for this operation,
    *    a call to this method might be a bit expensive (O(n) in worst 
    *    case).
    *    @param t The object to "give" to this allocator.
    *    @return  True if the object already was present in this 
    *             allocator, false otherwise (the destruction of the 
    *             object will be handeled by this allocator, no matter 
    *             what is returned).
    */
   bool handOverObject(T* t) {
      // Compare pointers to find out if t is in m_block
      if ( (t >= m_block) && 
           (t < (m_block+m_curBlockIndex*sizeof(T))) ) {
         // OK, t is inside m_block
         mc2dbg8 << "Found object in block in allocator" << endl;
         return true;
      }
      // Check all elements in m_singleAllocated
      typename slist<T *>::const_iterator it = m_singleAllocated.begin();
      while ( (it != m_singleAllocated.end()) && (*it != t)) {
         ++it;
      }

      // Add to m_singleAllocated if not found
      if (it == m_singleAllocated.end()) {
         m_singleAllocated.push_front(t);
         mc2dbg1 << "Found object that was not in allocator" << endl;
         return false;
      }
      mc2dbg8 << "Found object in list in allocator" << endl;
      return true;
   };

   
   iterator begin() {
      return iterator( m_block, 
                       m_block + m_blockAllocated,
                       m_singleAllocated.begin() );
   }

   iterator end() {
      return iterator( m_block + m_blockAllocated, 
                       m_block + m_blockAllocated,
                       m_singleAllocated.end() );
   }

   const_iterator begin() const {
      return const_iterator( m_block, m_block + m_blockAllocated,
                             m_singleAllocated.begin() );
   }
   const_iterator end() const {
      return const_iterator( m_block + m_blockAllocated, 
                             m_block + m_blockAllocated,
                             m_singleAllocated.end() );
   }
   /// assumes the pointer exist inside
   /// @return position of the pointer
   uint32 getPosition( const T* p ) const {
      if ( ! m_singleAllocated.empty() ) {
         const_iterator it = begin();
         const_iterator it_end = end();
         for (uint32 distance = 0; it != it_end; ++it, ++distance ) {
            if ( p == &(*it) )
               return distance;
         }
      } 

      MC2_ASSERT( p >= m_block && p < m_block + m_blockAllocated );

      return p - m_block;
   }
protected:
   uint32 getSingleElementsSize() const {
      return m_singleAllocated.size();
   }
private:
   /**
    *    The block with objects of type <tt>T</tt>.
    */
   T* m_block;

   /**
    *    The number of allocated elements in m_block.
    */
   int m_blockAllocated;

   /**
    *    The index of the last free object in m_block.
    */
   int m_curBlockIndex;
      
   /**
    *    A list with the items that are allocated after m_block 
    *    is filled.
    */
   slist<T *> m_singleAllocated;
};

#endif


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ARRAYTOOLS_H
#define ARRAYTOOLS_H

#include "config.h"

#include <string.h>


namespace ArrayTool {

// Nice macros to have when sending arrays around.
#define BEGIN_ARRAY(x) (x)
#define END_ARRAY(x) ( (x) + (sizeof(x)/sizeof((x)[0]) ) )

/**
 *    @name Template functions for dynamic allocation of arrays.
 *
 *    Example usage 1:
 *    
 *    uint32 allocSize = 10;
 *    uint32 size = 0;
 *    byte* buf = new buff[allocSize];
 *   
 *    buff = ArrayTool::addElement(buf, 7, size, allocSize);
 *    ...
 *    // Reallocs to actual size.
 *    buff = Utility::realloc(buf, size, size, allocSize);
 *    
 *    // Finished with adding elements. Store buffer and
 *    // buffer size.
 *    m_importantBuffer = buf;
 *    m_importantSize = size;
 *    
 *    Example usage 2:
 *    
 *    Old member variables in class:
 *    Vector m_numbers;
 *
 *    Old method, called seldomly in running system:
 *    void addNumber(uint32 number) {
 *       m_numbers.addLast(number);
 *    }
 *
 *    New member variables in class:
 *    uint32 m_nbrNumbers;
 *    uint16* m_numbers;
 *
 *    New member function:
 *    void addNumber(uint32 number) {
 *       m_numbers = ArrayTool::addElement(m_numbers, number,
 *                                       m_nbrNumbers);
 *    }
 *    
 */
//@{
/**
 *    Reallocates array. If the array will be shrunk, the
 *    destructor for the removed elements will be called.
 *    @param buf The array to reallocate.
 *    @param nbrElements The number of valid elements in the
 *           buf.
 *    @param newAllocSize The new wanted allocation size.
 *    @param allocSize The old allocationsize. Will be 
 *                     changed to newAllocSize.
 *    @return The new buffer.
 */
template<class T, class S> 
T* reallocate( T* buf, uint32 nbrElems, uint32 newAllocSize, 
               S& allocSize) {
   T* newBuf = NULL;
   if ( newAllocSize > 0 )
      newBuf = new T[newAllocSize];
   if ( buf != NULL && newBuf != NULL ) {
      uint32 nbr =  std::min(nbrElems, newAllocSize);
      for(uint32 i = 0; i < nbr; ++i ) {
         newBuf[i] = buf[i];
      }
   }
   // Call destructor for the leftover elements.
   delete [] buf;
   allocSize = newAllocSize;
               
   return newBuf;
}

/**
 *    Add element last to array.
 *    @param buf The array to add element to.
 *    @param elem The element to add.
 *    @param nbrElems The number of valid elements in buf.
 *                    Will be increased by one.
 *    @param allocSize The number of allocated elements in buf.
 *                     Can change if the array must be 
 *                     reallocated.
 *    @return The array or a new one that has been reallocated.
 */
template<class T, class S>
T* addElement(T* buf, const T& elem, S& nbrElems, 
                     S& allocSize) {
   if ( nbrElems >= allocSize ) {
      if ( allocSize > 0 ) 
         buf = reallocate(buf, nbrElems, allocSize * 2, allocSize);
      else
         buf = reallocate(buf, nbrElems, 1, allocSize);
   }
               
   buf[nbrElems++] = elem;
   return buf;
}
            
/**
 *    Add element last to array. Assumed that allocSize of
 *    the array is equal to the number of elements.
 *    Always reallocates the buffer to nbrElems + 1.
 *    @param buf The array to add the element to.
 *    @param elem The element to add.
 *    @param nbrElements The allocated size and the number
 *                       of valid elements in <code>buf</code>.
 *    @return A new array with the old elements in it. Plus
 *            the new one.
 */
template<class T, class S>
T* addElement(T* buf, const T& elem, S& nbrElems) {
   uint32 allocSize = nbrElems;
   buf = reallocate(buf, nbrElems, allocSize + 1, allocSize);
   buf[nbrElems++] = elem;

   return buf;
}
            

/**
 *    Remove element in array.
 *    @param buf The array.
 *    @param idx The index to remove the element from.
 *    @param size The number of valid elements in buf.
 */
template<class T, class S>
void removeElement(T* buf, uint32 idx, S& size) {
   if (idx != uint32(size - 1)) {
      // Copy the element to be removed to a new buffer.
      byte* removedElemBuf = new byte[sizeof(T)];
      memcpy(removedElemBuf, buf + idx, sizeof(T));
      // Move all elements after the removed element
      // one step forward.
      memmove(buf + idx, 
              buf + idx + 1, 
              (size - idx - 1) * sizeof(T));
      // Copy the removed element to the last position in
      // the array.
      memcpy(buf + size - 1, removedElemBuf, sizeof(T));
      delete[] removedElemBuf;
   }
   size--;
}
//@}

} // ArrayTool namespace

#endif


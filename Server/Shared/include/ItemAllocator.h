/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMALLOCATOR_H
#define ITEMALLOCATOR_H

#include "AllocatorTemplate.h"

#include "DataBuffer.h"
#include "GenericMap.h"
#include "STLUtility.h"

/**
 * Load/Save interface for ItemAllocator
 */
struct DataBufferMapObject {
   virtual ~DataBufferMapObject() {}
   virtual void load( DataBuffer& buff, GenericMap& map ) = 0;
   virtual void save( DataBuffer& buff, const GenericMap& map ) const = 0;
   virtual uint32 getMemoryUsage() const = 0;
};

/**
 * Allocator for items. Can laod and save.
 */
template <typename T>
class ItemAllocator: public MC2Allocator<T>, public DataBufferMapObject {
public:

   virtual ~ItemAllocator() { }

   void load( DataBuffer& buff, GenericMap& map ) {
      // load size
      MC2Allocator<T>::reallocate( buff.readNextLong() );
      // load items
      typename MC2Allocator<T>::iterator it = MC2Allocator<T>::begin();
      typename MC2Allocator<T>::iterator it_end = MC2Allocator<T>::end();
      for (; it != it_end; ++it ) {
         (*it).load( buff, map );
         map.updateZoom( static_cast< Item& >( *it ) );
         buff.alignToLong();
      }
   }

   void save( DataBuffer& buff, const GenericMap& map ) const {
      // write size
      buff.writeNextLong( MC2Allocator<T>::getBlockSize() + 
                          MC2Allocator<T>::getSingleElementsSize() );
      // save all items
      typename MC2Allocator<T>::const_iterator it = MC2Allocator<T>::begin();
      typename MC2Allocator<T>::const_iterator it_end = MC2Allocator<T>::end();
      for (; it != it_end; ++it ) {
         (*it).save( buff, map );
         buff.alignToLong();
      }
   }

   uint32 getMemoryUsage() const {
      return sizeof(T) * ( MC2Allocator<T>::getBlockSize() + 
                           MC2Allocator<T>::getSingleElementsSize() );
   }
};

#endif 

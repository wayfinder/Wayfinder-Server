/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMCOMBOLOADER_H
#define ITEMCOMBOLOADER_H

#include "config.h"
#include "STLUtility.h"
#include "DataBufferRW.h"
#include "PacketRW.h"
#include "PacketDataBuffer.h"

/// Loader for ints. Reads and writes uint32:s.
template<class INTEGER_TYPE> 
class ItemComboLoader {
public:
   /// For the ItemDataTable
   template<class X>
   static void writeValue( DataBuffer& buf, const X& value ) {
      buf.writeNextLong( value );
   }

   /// For the ItemDataTable
   template<class X>
   static void readValue( DataBuffer& buf, X& value ) {
      value = buf.readNextLong();
   }
   
   /// For the ItemDataTable
   template<class X>
   static uint32 getValSizeInDataBuffer( const X& value ) {
      return 4;
   }   
};

/// Specialization for DataBufferRW
template <> class ItemComboLoader<class DataBufferRW> {
public:
   template <typename X>
   static void writeValue( DataBuffer& buf, const X& value ) {
      value.save( buf );    
   }
   template <typename X>
   static void readValue( DataBuffer& buf, X& value ) {
      value.load( buf );
   }
   template <typename X>
   static uint32 getValSizeInDataBuffer( const X& value ) {
      return value.getSizeInDataBuffer();
   }
};

///Specialization for PacketRW
template<> class ItemComboLoader<class PacketRW> {
public:
   template<class X>
   static void writeValue( DataBuffer& buf, const X& val ){
      PacketDataBuffer::saveAsInPacket( buf, val );
   }
   template<class X>
   static void readValue( DataBuffer& buf, X& val ){
      PacketDataBuffer::loadAsFromPacket( buf, val );
   }
   template<class X>
   static uint32 getValSizeInDataBuffer( const X& val ) {
      return val.getSizeInPacket();
   }
};




/// Specialisation for 8-bits
template<> class ItemComboLoader<uint8> {
public:
   /// For the ItemDataTable
   template<class X> static void writeValue( DataBuffer& buf,
                                             const X& value ) {
      buf.writeNextByte( value );
   }

   /// For the ItemDataTable
   template<class X> static void readValue( DataBuffer& buf, X& value ) {
      value = buf.readNextByte();
   }
   
   /// For the ItemDataTable
   template<class X> static uint32 getValSizeInDataBuffer( const X& value ) {
      return 1;
   }       
};

/// Specialisation for 16-bits
template<> class ItemComboLoader<uint16> {
public:
   /// For the ItemDataTable
   template<class X>
   static void writeValue( DataBuffer& buf, const X& value ) {
      buf.writeNextShort( value );
   }

   /// For the ItemDataTable
   template<class X>
   static void readValue( DataBuffer& buf, X& value ) {
      value = buf.readNextShort();
   }
   
   /// For the ItemDataTable
   template<class X>
   static uint32 getValSizeInDataBuffer( const X& value ) {
      return 2;
   }       
   
};

/// Class that selects the correct loader/saver for values.
template<class T>
class LoaderTypeSelector {
public:
   ///See if the type T is DataBufferRW and/or PacketRW
   enum { 
      IsDataBufferRW = IsDataBufferRWT<T>::Yes, ///T is derived from DataBufferRW 
      IsPacketRW     = IsPacketRWT<T>::Yes, ///T is derived from PacketRW
      IsNeither      = ! (IsDataBufferRW & IsPacketRW),
   };

   ///Find the loader type. If the type T is derived from
   ///DataBufferRW, we should select the DataBufferRW class as
   ///loaderType, if T is derived from PacketRW the PacketRW type
   ///should be selected. If T is derived form neither of these types,
   ///T itself should be selected.
   typedef ItemComboLoader< IF_THEN_ELSE( IsDataBufferRW, DataBufferRW,
                                          IF_THEN_ELSE( IsPacketRW, PacketRW,
                                                        T)) > loaderType;
   

};

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "GfxFeatureMapImagePacket.h"
#include "MapSettings.h"
#include "DataBuffer.h"

// ------------------------------------------------------------------------
// -------------------------------------- GfxFeatureMapImageRequestPacket -

GfxFeatureMapImageRequestPacket::GfxFeatureMapImageRequestPacket(  
                                     uint16 reqID, uint16 packetID,
                                     ImageDrawConfig::imageFormat format,
                                     bool exportFormat,
                                     bool scalableExportFormat,
                                     bool drawCopyRight,
                                     uint32 size,
                                     uint16 sizeParam,
                                     uint8 minScaleLevel,
                                     uint8 maxScaleLevel,
                                     uint32 mapSettingSize,
                                     const char* copyright)
  : RequestPacket( REQUEST_HEADER_SIZE + size + (copyright_POS - REQUEST_HEADER_SIZE) + mapSettingSize + strlen( copyright ) + 1 + 3 + 4,
                       GFXFEATUREMAP_IMAGE_REQUEST_PRIO,
                       PACKETTYPE_GFXFEATUREMAP_IMAGE_REQUEST,
                       packetID,
                       reqID, 
                       MAX_UINT32 )
{
   int position = imageFormat_POS;

   incWriteLong( position, format );
   incWriteLong( position, size );
   incWriteShort( position, sizeParam );

   byte b = 0;
   if ( !exportFormat )
      b &= 0x7f;
   else
      b |= 0x80;
   if ( !scalableExportFormat )
      b &= 0xbf;
   else
      b |= 0x40;
   if ( !drawCopyRight ) {
      b &= 0xdf;
   } else {
      b |= 0x20;
   }

   // b & 0x10 == aligned map data - will be sent in new version later.
   // Now it is the new version
   b |= 0x10; // It will be aligned.
   
   incWriteByte(position, b);
   
   incWriteByte( position, minScaleLevel );
   incWriteByte( position, maxScaleLevel );

   // Route turn byte
   incWriteByte(position, RouteArrowType::NOT_USED );

   // Map rotation
   incWriteShort(position, 0 );

   
   // beforeTurn
   incWriteLong( position, MAX_UINT32 );

   // afterTurn
   incWriteLong( position, MAX_UINT32 );


   incWriteLong( position, mapSettingSize );
   incWriteLong( position, strlen( copyright ) + 1 );
     
   setCopyright( copyright );
   position += strlen( copyright ) + 1;

   setLength( position );
   
}

ImageDrawConfig::imageFormat
GfxFeatureMapImageRequestPacket::getImageFormat() const
{
   return ImageDrawConfig::imageFormat( readLong( imageFormat_POS ) );
}

void
GfxFeatureMapImageRequestPacket::setImageFormat(
   ImageDrawConfig::imageFormat format)
{
   writeLong(imageFormat_POS, format);
}


uint16
GfxFeatureMapImageRequestPacket::getSizeParameter()  const
{
   int position = sizePar_POS;
   return readShort(position);
}


bool
GfxFeatureMapImageRequestPacket::getExportFormat() const
{
   int position = exportFormat_POS;
   byte b = readByte(position);
   b &= 0x80;
   return( b == 0x80 );
}

bool
GfxFeatureMapImageRequestPacket::alignedMapData() const
{
   int position = exportFormat_POS;
   byte b = readByte(position);
   b &= 0x10;
   return  b == 0x10;
}


bool
GfxFeatureMapImageRequestPacket::getScalableExportFormat() const
{
   int position = exportFormat_POS;
   byte b = readByte(position);
   b &= 0x40;
   return( b == 0x40 );
}


bool
GfxFeatureMapImageRequestPacket::getDrawCopyRight() const {
   int position = exportFormat_POS;
   byte b = readByte(position);
   b &= 0x20;
   return( b == 0x20 );
}


uint8 
GfxFeatureMapImageRequestPacket::getMinScaleLevel() const
{
   int position =  minScaleLevel_POS;
   return readByte(position);
}

uint8 
GfxFeatureMapImageRequestPacket::getMaxScaleLevel() const
{
   int position =  maxScaleLevel_POS;
   return readByte(position);
}

DataBuffer* 
GfxFeatureMapImageRequestPacket::getGfxFeatureMapData( 
   MapSettings* setting ) const
{
   int position = copyright_POS + readLong( copyrightSize_POS );
   
   // MapSettings
   uint32 size = readLong(position);
   if ( size >= 0 && setting != NULL ) {
      position = copyright_POS + readLong( copyrightSize_POS );
      setting->loadFromPacket( this, position );
   } else {
      return NULL;
   }

   // GfxFeatureMap
   // Read size
   size = readLong(size_POS);
   
   if (size > 0) {

      if ( alignedMapData() ) {
         AlignUtility::alignLong( position );
      }
      
      // Get the byte pointer from the packet.
      const byte* bytes = incReadByteArray(position, size);

      if ( alignedMapData() ) {
         // Don't copy the data.
         DataBuffer* data = new DataBuffer( const_cast<byte*> (bytes), size );
         // Read past the bytes.
         data->readPastBytes( size );
         return data;
      } else {
         // Copy the data - should not happen when both US and EU
         // clusters are updated.
         mc2dbg << "[GFMIRP]: unaligned -> copying databuffer" << endl;
         DataBuffer* data = new DataBuffer( size );
         data->writeNextByteArray( bytes, size );
         return data;
      }      
   } else {
      return (NULL);
   }
}

void 
GfxFeatureMapImageRequestPacket::setGfxFeatureMapData(
   uint32 size,
   DataBuffer* data,
   const MapSettings* setting )
{
   int position = copyright_POS + readLong( copyrightSize_POS );

   // MapSettings
   setting->saveToPacket( this, position );   
   
   // GfxFeatureMap

   // Reset offsets
   data->reset();
   
   // Get the bytearray pointer from the DataBuffer.
   const byte* bytes = data->readNextByteArray(size);
   // Align the packet position
   incAlignWriteLong( position );
   setLength ( position );
   if ( updateSize( size, getBufSize() ) ) {
      mc2dbg << "[GFMIRP]: Resizing buffer to " << getBufSize() << endl;
   }
   // Copy it into the packet.   
   incWriteByteArray(position, bytes, size);
   
   mc2dbg8 << "position " << position<< endl;
   setLength(position);
   
}

void 
GfxFeatureMapImageRequestPacket::setRouteTurn( uint32 beforeTurn, 
                                               uint32 afterTurn ) 
{
   writeByte( arrowType_POS, RouteArrowType::TURN_ARROW );
   writeLong( beforeTurn_POS, beforeTurn ); 
   writeLong( afterTurn_POS, afterTurn ); 
}

void
GfxFeatureMapImageRequestPacket::setRouteAsArrow( uint32 afterTurn )
{
   writeByte( arrowType_POS, RouteArrowType::ROUTE_AS_ARROW );
   writeLong( afterTurn_POS, afterTurn );
}

RouteArrowType::arrowType
GfxFeatureMapImageRequestPacket::getArrowType() const {
   return static_cast<RouteArrowType::arrowType>( readByte( arrowType_POS ) );
}

uint32 
GfxFeatureMapImageRequestPacket::getBeforeturn() const {
   return readLong( beforeTurn_POS );
}

uint32 
GfxFeatureMapImageRequestPacket::getAfterturn() const {
   return readLong( afterTurn_POS );
}

void 
GfxFeatureMapImageRequestPacket::setMapRotation(int16 angle)
{
   writeShort( mapRotation_POS, angle );
}

int16 
GfxFeatureMapImageRequestPacket::getMapRotation() const
{
   return readShort( mapRotation_POS );
}

const char* 
GfxFeatureMapImageRequestPacket::getCopyright() const {
   char* tmp = NULL;
   int pos = copyright_POS;
   incReadString( pos, tmp );
   return tmp;
}

void 
GfxFeatureMapImageRequestPacket::setCopyright( const char* copyright ) {
   int pos = copyright_POS;
   incWriteString( pos, copyright );
   writeLong( copyrightSize_POS, pos - copyright_POS );
}

// ------------------------------------------------------------------------
// -----------------------------------------GfxFeatureMapImageReplyPacket -


GfxFeatureMapImageReplyPacket::GfxFeatureMapImageReplyPacket(
                                  uint32 size,  
                                  const GfxFeatureMapImageRequestPacket* p,
                                  bool imageData)
      : ReplyPacket( REPLY_HEADER_SIZE + size + 8,
                     PACKETTYPE_GFXFEATUREMAP_IMAGE_REPLY,
                     (RequestPacket*) p,
                     StringTable::OK)
{
   int position = imageData_POS;

   byte b = readByte(position);
   
   if( !imageData ) {
      b &= 0x7f;
      incWriteByte(position, b);
   } else {
      b |= 0x80;
      incWriteByte(position, b);
   }

   incWriteLong(position, size);
   
   setLength(position);
}


bool
GfxFeatureMapImageReplyPacket::isImageData() const
{
   int position = imageData_POS;

   byte b = readByte(position);
   b &= 0x80;
   if( b == 0x00 )
      return false;
   else
      return true;
}


byte*
GfxFeatureMapImageReplyPacket::getImageData() const
{
   int position = imageData_POS;

   byte b = incReadByte(position);
   b &= 0x80;
   
   if( b == 0x80 ) {
      // Read the size
      uint32 size = incReadLong(position);
      byte* data = new byte[size];

      // Copy the data from the packet to the bytearray.
      incReadByteArray(position, data, size);
      
      mc2dbg8 << "position " << position<< endl;
      return (data);
   } else {
      return (NULL);
   }
}


void
GfxFeatureMapImageReplyPacket::setImageData(uint32 size,
                                            byte* data)
{
   int position = size_POS;
   
   // Set the size
   incWriteLong(position, size);
   // Set the bytes
   incWriteByteArray(position, data, size);
   
   setLength(position);
}


byte*
GfxFeatureMapImageReplyPacket::getGfxExportFeatureMapData() const
{
   int position = imageData_POS;

   byte b = incReadByte(position);
   b &= 0x80;
   
   if( b == 0x00 ) {   
      //Read the size
      uint32 size = incReadLong(position);
      
      if (size > 0) {
         byte* data = new byte[size];
         
         // Copy the data from the packet to the bytearray.
         incReadByteArray(position, data, size);
         
         mc2dbg8 << "position " << position<< endl;
         return (data);

      } else {
         return (NULL);
      }
   } else {
      return (NULL);
   }
}

void
GfxFeatureMapImageReplyPacket::setGfxExportFeatureMapData(uint32 size,
                                                          DataBuffer* data)
{                                         
   int position = size_POS;
   
   incWriteLong(position, size);
   
   // Reset offsets
   data->reset();
   
   // Get the bytearray pointer from the DataBuffer.
   const byte* bytes = data->readNextByteArray(size);
   // Copy it into the packet.
   incWriteByteArray(position, bytes, size);

   
   mc2dbg8 << "position " << position<< endl;
   setLength(position);
}

byte*
GfxFeatureMapImageReplyPacket::getGfxExportScalableFeatureMapData() const
{
   int position = imageData_POS;

   byte b = incReadByte(position);
   //Makes b == 0x7F if the first two bytes are 0 and 1.
   b |= 0x3F;
      
   if( b == 0x7F ) {   
      //Read the size
      uint32 size = incReadLong(position);
      
      if (size > 0) {
         byte* data = new byte[size];
         
         // Copy the data from the packet to the bytearray.
         incReadByteArray(position, data, size);
         
         mc2dbg8 << "position " << position<< endl;
         return (data);

      } else {
         return (NULL);
      }
   } else {
      return (NULL);
   }
}

void
GfxFeatureMapImageReplyPacket::setGfxExportScalableFeatureMapData(uint32 size,
                                                          DataBuffer* data)
{                                         
  int position = imageData_POS;
  
  byte b = 0;
  //Set bit indicating scalable.
  b |= 0x40;
  
  incWriteByte(position, b);

  position = size_POS;
   
   incWriteLong(position, size);
   
   // Reset offsets
   data->reset();
   
   // Get the bytearray pointer from the DataBuffer.
   const byte* bytes = data->readNextByteArray(size);
   // Copy it into the packet.
   incWriteByteArray(position, bytes, size);

   
   mc2dbg8 << "position " << position<< endl;
   setLength(position);
}


uint32
GfxFeatureMapImageReplyPacket::getSize() const
{
   return readLong(size_POS);
}



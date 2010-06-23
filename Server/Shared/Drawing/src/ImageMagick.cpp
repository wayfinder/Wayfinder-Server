/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ImageMagick.h"

namespace GSystem {

namespace Magick {
Image::Image( uint32 width, uint32 height,
              const char* map, 
              StorageType storage,
              const void* pixels ) throw (GSystem::Exception):
   m_image( NULL ) {

   // initialize exception
   ExceptionInfo exception;
   GetExceptionInfo( &exception );

   m_image = ConstituteImage( width, height, map, storage,
                              pixels, &exception );
   if ( m_image == NULL ) {
      MC2String exceptionString = "Reason: ";
      exceptionString += exception.reason;
      exceptionString += "Description: ";
      exceptionString += exception.description;

      DestroyExceptionInfo( &exception );

      throw GSystem::Exception( "[ImageMagick]" + exceptionString );
   }
   DestroyExceptionInfo( &exception );

   GetQuantizeInfo(&m_quantizeInfo);
}

Image::~Image() {
   DestroyImage( m_image );
}

void Image::quantizeColorSpace( ColorspaceType colorSpace ) {
   m_quantizeInfo.colorspace = colorSpace;
}

void Image::quantizeColors( uint32 numColors ) {
   m_quantizeInfo.number_colors = numColors;
}

bool Image::quantize() {
   return QuantizeImage( &m_quantizeInfo, m_image );
}

void Image::magick( const char* magickStr ) {
   strcpy( m_image->magick, magickStr );
}

pair< unsigned char*, int >
Image::getBuffer() {
   size_t size = 0;
   ::ImageInfo info;
   GetImageInfo( &info );
   ExceptionInfo exception;
   GetExceptionInfo( &exception );

   unsigned char* blob = ImageToBlob( &info, m_image, &size,
                                      &exception );
   if ( blob == NULL ) {
      MC2String exceptionString = "Reason: ";
      exceptionString += exception.reason;
      exceptionString += "Description: ";
      exceptionString += exception.description;

      DestroyExceptionInfo( &exception );

      throw GSystem::Exception( "[ImageMagick]" + exceptionString );
   }

   DestroyExceptionInfo( &exception );

   return make_pair( blob, size );
}

bool Image::write( const MC2String& filename ) {
   strcpy( m_image->filename, filename.c_str() );
   ::ImageInfo info;
   GetImageInfo( &info );

   return WriteImage( &info, m_image );
}

void Image::setImageType( ImageType imageType ) {
   SetImageType( m_image, imageType );
}

}

}

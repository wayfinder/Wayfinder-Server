/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GSystem.h"

#include <magick/api.h>

namespace GSystem {

/// Image Magick helper classes
namespace Magick {

/// simple wrapper class for ImageMagick (instead of Magick++)
class Image {
public:
   /**
    * @param width the width in pixels 
    * @param height the height in pixels
    * @param map original data map (i.e "RGB", "RGBA"..)
    * @param storage the storage type
    * @param pixels original image data
    */
   Image( uint32 width, uint32 height,
          const char* map, 
          StorageType storage,
          const void* pixels ) throw (GSystem::Exception);
   ~Image();

   /// set color space for quantization
   void quantizeColorSpace( ColorspaceType colorSpace );
   /// set maximum number of colors to quantizes
   void quantizeColors( uint32 numColors );
   /// quantizes image
   bool quantize();
   /// set magick string ( i.e "GIF", "PNG" etc )
   void magick( const char* magickStr );
   /// write image to file
   bool write( const MC2String& filename );
   void setImageType( ImageType type );

   /**
    * Get the image as a buffer and the size of the buffer.
    * @return pair of buffer data and the data size.
    */
   pair< unsigned char*, int > getBuffer();
private:
   ::Image* m_image; ///< internal ImageMagick pointer
   ::QuantizeInfo m_quantizeInfo; ///< quantization information
};

};

}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IMAGEDRAWCONFIG_H
#define IMAGEDRAWCONFIG_H

#include "config.h"


/**
 * Holds image cofiguration constants.
 */
class ImageDrawConfig {
   public:
     /**
      * The types of image formats.
      */ 
      enum imageFormat {
         /// PNG Portable Network Graphics
         PNG = 0,
         /// Wireless BitMaP
         WBMP,
         /// Joint Pictures Expert Group
         JPEG,
         /// Graphics Interchange Format (You MUST pay royalties to unisys)
         GIF,

         // The number of image formats, must be last!
         NBR_IMAGE_FORMATS
      };


      /**
       * Tries to match format to an imageFormat, return defaultFormat 
       * if no match.
       * @param format The imageFormat string.
       * @param defaultFormat The default format to return if no format
       *                      matches format string, default PNG.
       * @return ImageFormat that matches format string, returns 
       *         defaultFormat if not match.
       */
      static ImageDrawConfig::imageFormat imageFormatFromString( 
         const char* format,
         ImageDrawConfig::imageFormat defaultFormat = 
         ImageDrawConfig::PNG );


      /**
       * The image formats as magick strings, is NBR_IMAGE_FORMATS long.
       */
      static const char* imageFormatMagick[];


      /**
       * Get the imageformat as string.
       */
      static const char* getImageFormatAsString( imageFormat format );


   private:
      /**
       * Prvate constructor to avoid usage.
       * NB! Not implemnted.
       */
      ImageDrawConfig();
};

#endif // IMAGEDRAWCONFIG_H



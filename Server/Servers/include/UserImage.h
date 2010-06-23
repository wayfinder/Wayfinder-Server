/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USER_IMAGE_H
#define USER_IMAGE_H

#include "config.h"
#include "MC2String.h"
#include <memory>

class BitBuffer;

namespace UserImage {

/**
 * This will create a PNG from a SVG, the size is determined by the filename
 * format.
 *
 * The format is:
 * imagename_%wx%h.png
 * Where %w is the request width in pixels and %h the height in pixels, and the
 * SVG image loaded to convert this image will be called imagename.svg.
 *
 * The requested size will not be affected by cropping.
 *
 * @param imagePath Base path of images without the last '/'.
 * @param filename filename without the special prefix, e.g Q.
 * @param cropped Whether all side transparency should be cropped,
 *                e.g no offset transparency on either side.
 *                The requested size will not be affected by this.
 * @return allocated bitbuffer for custom PNG image.
 */
std::auto_ptr< BitBuffer >
createCustomImageSize( const MC2String& imagePath,
                       const MC2String& filename,
                       bool cropped );

/**
 * Convert input filename to real filename and size, that can be converted
 * to a valid size later.
 * @param height will contain height in pixels.
 * @return final filename, if empty then the convert failed.
 */
MC2String getFilename( const MC2String& filename,
                       int32& width, int32& height );

/**
 * Try to get image width and height from a png file.
 *
 * @param filePath The patch to the png file.
 * @param width Set to the width of the image.
 * @param height Set to the height of the image.
 * @return True if width and height are set.
 */
bool getPNGImageSize( const MC2String& filePath,
                       int32& width, int32& height );


/**
 * Magnify \c origname file to \c destName by a factor.
 * First tries to use svg then calls ImageTools::magnify.
 * 
 * @param origName Original filename.
 * @param destName Destination filename.
 * @param factor The magnification factor, 2 for double original size.
 * @param cropped If to remove transparent areas of the image.
 * @return true on success.
 */
bool magnify( const MC2String& origName, const MC2String& destName,
              uint32 factor, bool cropped );


} // UserImage

#endif // USER_IMAGE_H

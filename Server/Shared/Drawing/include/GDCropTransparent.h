/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GDCROPTRANSPARENT_H
#define GDCROPTRANSPARENT_H

#include <gd.h>

#include "MC2String.h"
#include "MC2Point.h"
#include "GDImagePtr.h"

class SharedBuffer;
class DataBuffer;

namespace GDUtils {

/// first is top left cut point, second is bottom right cut point
class CutPoints {
public:
   CutPoints():m_topLeft( 0, 0 ), m_bottomRight( 0, 0 ) { }
   CutPoints( const MC2Point& topLeft, const MC2Point& bottomRight ):
      m_topLeft( topLeft ), m_bottomRight( bottomRight ) { }
   const MC2Point& getTopLeft() const { return m_topLeft; }
   const MC2Point& getBottomRight() const { return m_bottomRight; }

   /// @return width between cut points in pixels.
   uint32 getWidth() const {
      return getBottomRight().getX() - getTopLeft().getX();
   }

   /// @return height between cut points in pixels.
   uint32 getHeight() const {
      return getTopLeft().getY() - getBottomRight().getY();
   }

private:
   MC2Point m_topLeft;
   MC2Point m_bottomRight;
};

/**
 * Crops a image transparent parts and calculates hot spot
 * @param srcFilename loads png from this file
 * @param destination memmapped data buffer destination
 * @param hotspot the original cetrum of the src image relative to the new image
 * @return true if crop was successful
 */
bool cropTransparentHotspot( const MC2String& srcFilename,
                             DataBuffer& destination, MC2Point& hotspot );

/**
 * Crops a image transparent parts and calculates the offset based on the two images
 * center. 
 * @param srcFilename loads png from this file
 * @param offset the original center of the src image relative to the new center
 * @param cutPoints the points for the new image
 * @return true if crop was successful
 */
bool cropTransparentOffset( const MC2String srcFilename,
                            MC2Point& offset,
                            CutPoints* cutPoints = NULL );

/**
 * crops the image's transparent part
 * @param image gd image pointer
 * @param destFilename destination filename
 * @param cutPoints returns the top left  and bottom right cutpoints
 * @return true if the crop was successfull
 */
bool cropTransparent( gdImagePtr image, 
                      const MC2String& destFilename,
                      CutPoints* cutPoints = NULL );

/**
 * Crop transparent PNG from a buffer to a PNG buffer.
 * @param pngBuffer Raw data buffer from a PNG file.
 * @return cropped PNG buffer.
 */
std::auto_ptr< SharedBuffer > cropTransparent( const SharedBuffer& pngBuffer );

/**
 * crops the image's transparent part
 * @param srcFilename load png from this file and convert it and save to destFilename
 * @param destFilename destination filename
 * @param cutPoints returns the top left  and bottom right cutpoints
 * @return true if the crop was successfull
 */
bool cropTransparent( const MC2String& srcFilename, 
                      const MC2String& destFilename,
                      CutPoints* cutPoints = NULL );

/**
 * crops the image's transparent part   
 * @param image gd image pointer
 * @param cutPoints returns the top left  and bottom right cutpoints
 * @return ImagePtr for the new image
 */
gdImagePtr cropTransparent( gdImagePtr image,
                          CutPoints* cutPoints = NULL );

/**
 * Determine cropping points for an image.
 * @param pngBuffer buffer containing raw data of a PNG image.
 * @return start and end points for cropping.
 */
CutPoints findCroppingPoints( const SharedBuffer& pngBuffer );

}

#endif // GDCROPTRANSPARENT_H

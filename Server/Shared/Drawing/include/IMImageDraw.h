/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IMIMAGEDRAW_H
#define IMIMAGEDRAW_H


#include "config.h"
#include "ImageDraw.h"
#include <vector>
#include "GfxUtility.h"
// ImageMagick the graphics library for image creation
//#include "magick/api.h"


/**
 * Class for drawing images in memory and saving it into memory in
 * known image formats.
 *
 * ImageMagick copyright.
 * This class uses ImageMagick to draw images.
"Copyright (C) 2001 ImageMagick Studio, a non-profit organization dedicated
to making software imaging solutions freely available.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files ("ImageMagick"),
to deal in ImageMagick without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of ImageMagick, and to permit persons to whom the
ImageMagick is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of ImageMagick.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.  In no event shall
ImageMagick Studio be liable for any claim, damages or other liability,
whether in an action of contract, tort or otherwise, arising from, out of
or in connection with ImageMagick or the use or other dealings in
ImageMagick.

Except as contained in this notice, the name of the ImageMagick Studio
shall not be used in advertising or otherwise to promote the sale, use or
other dealings in ImageMagick without prior written authorization from the
ImageMagick Studio.

ImageMagick is available as

  ftp://ftp.simplesystems.org/pub/ImageMagick/

The official ImageMagick WWW page is

  http://www.simplesystems.org/cristy/ImageMagick.html

The author is magick@wizards.dupont.com."
 *
 */
class IMImageDraw : public ImageDraw {
   public:
      /**
       * Constructs a new Image with the desired dimensions.
       *
       * @param xSize The number of pixels in the x-dimension.
       * @param ySize The number of pixels in the y-dimension.
       */
      IMImageDraw( uint32 xSize, uint32 ySize );


      /**
       * Destructor, deallocates image.
       */
      ~IMImageDraw();


      /**
       * Returns the image as a buffer of the right format.
       *
       * @param size Set to the length of the retuned buffer.
       * @param format The type of encoding of the buffer.
       */
      virtual byte* getImageAsBuffer( uint32& size, 
                                      ImageDrawConfig::imageFormat format
                                      = ImageDrawConfig::PNG );


      /**
       * Draws an arc, usually a positioned area.
       * NOT IMPLEMENTED!
       *
       * @param cx Center of arc x-value, may be negative of larger than
       *           images x-size.
       * @param cx Center of arc y-value, may be negative of larger than
       *           images y-size.
       * @param startAngle The start angle in degrees.
       * @param stopAngle The stop angle in degrees.
       * @param outerRaduis The outer radius.
       * @param innerRaduis The inner radius, may be 0.
       * @param color The color to draw with.
       * @param lineWidth The linewidth to draw, default 1. 
       */
      virtual void drawArc( int cx, int cy,
                            int startAngle, int stopAngle, 
                            int innerRadius, int outerRadius,
                            GDUtils::Color::CoolColor color,
                            int lineWidth = 1 );
      
      
      /**
       * Draw a spline turn arrow using four spline controlpoints and
       * and an arrow. The arrow is drawn either using the three
       * arrow points or the arrowAngle and arrowLength parameters.
       * It is up to the subclass to deside.
       * @param x0 The x value of the first controlpoint.
       * @param y0 The y value of the first controlpoint.
       * @param x1 The x value of the second controlpoint.
       * @param y1 The y value of the second controlpoint.
       * @param x2 The x value of the third controlpoint.
       * @param y2 The y value of the third controlpoint.
       * @param x3 The x value of the fourth controlpoint.
       * @param y3 The y value of the fourth controlpoint.
       * @param arrowX0 The x value of the left arrowpoint.
       * @param arrowY0 The y value of the left arrowpoint.
       * @param arrowX1 The x value of the center arrowpoint.
       * @param arrowY1 The y value of the center arrowpoint.
       * @param arrowX2 The x value of the right arrowpoint.
       * @param arrowY2 The y value of the right arrowpoint.
       * @param arrowAngle The alternative to using the arrow points
       *                   is to use this as the angle of the arrow.
       * @param arrowLength The alternative to using the arrow points
       *                    is to use this as the length of the arrow.
       * @param col The color to draw the turn arrow with.
       * @param arrowWidth The width of the arrows line. Default 1.
       */
      virtual void drawTurnArrow( int x0, int y0, int x1, int y1, 
                                  int x2, int y2, int x3, int y3,
                                  int arrowX0, int arrowY0,
                                  int arrowX1, int arrowY1,
                                  int arrowX2, int arrowY2,
                                  int32 arrowAngle, int32 arrowLength,
                                  GDUtils::Color::CoolColor col,
                                  int arrowWidth = 1 );
          
      /**
       * Draw the route as an arrow. The arrow head is drawn at the 
       * final point in route. All coordinates should be screen/image
       * coordinates.
       *
       * @param route Vector containing the points forming the route.
       *              These points should end at a distance from the 
       *              edge making it possible to draw the arrow head.
       * @param arrowHead Vector containing the points forming the arrow
       *                  head. Exactly three points should be contained
       *                  in the vector, ordered: left corner, top, 
       *                  right corner.
       * @param color The color used for drawing the whole arrow, 
       *              including both the route and the arrow head.
       * @param routeWidth The width used when drawing the route part of 
       *                   the arrow.
       */
      virtual void drawRouteAsArrow(vector<POINT> route,
                                    vector<POINT> arrowHead,
                                    GDUtils::Color::CoolColor color,
                                    uint32 routeWidth);
      
   protected:
      /**
       * Returns the RGB value of a pixel.
       * @param x The x-coordinate of the pixel to get.
       * @param y The y-coordinate of the pixel to get.
       * @param red Set to the red part of the pixel color.
       * @param green Set to the green part of the pixel color.
       * @param blue Set to the blue part of the pixel color.
       */
      virtual void getPixel( uint32 x, uint32 y, 
                             unsigned char& red, 
                             unsigned char& green, 
                             unsigned char& blue );


   private:


      /**
       * The image to draw on.
       */
      void* m_image;

      
      /**
       * The information about the image.
       */
      void* m_imageInfo;
};


#endif // IMIMAGEDRAW_H

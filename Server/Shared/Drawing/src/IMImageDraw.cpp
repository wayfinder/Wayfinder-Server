/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "IMImageDraw.h"
// ImageMagick the graphics library for image creation
#include "magick/api.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "GDColor.h"
#include "GSystem.h"

// The new version of ImageMagick has a Symbian-type enum instead of bool.
#if MagickLibVersion >= 0x607
#define MB(x) ( (x) ? MagickTrue : MagickFalse )
#else
#define MB(x) (x)
#endif

/**
 * WARNING: Moved here to avoid IM stuff in header (It breaks stuff...)
 * Get a color.
 * @param color The color requested.
 * @param setColor Set to the color requested.
 * @param quantumShift The number of bits more in ImageMagick than 8.
 */
void getColor( GDUtils::Color::CoolColor color,
               PixelPacket& setColor, int quantumShift );


inline void
getColor( GDUtils::Color::CoolColor color, 
          PixelPacket& setColor, 
          int quantumShift ) 
{
   using namespace GDUtils;
   GSystem::Color col( static_cast<uint32>( color ) );
   setColor.red = col.getRed() << quantumShift;
   setColor.green = col.getGreen() << quantumShift;
   setColor.blue = col.getBlue() << quantumShift;

   setColor.opacity = color; // If PseudoClass this is index
}





IMImageDraw::IMImageDraw( uint32 xSize, uint32 ySize ) 
      : ImageDraw( xSize, ySize )
{
   DEBUG8(char cStr[512]; clock_t m_startTime = clock(); );
   ImageInfo* imImageInfo = CloneImageInfo( NULL ); // Create new
   GetImageInfo( imImageInfo ); // Initialize it

   // Set image width and height
   char* imImageSize = static_cast< char* > (
      AcquireMemory( 22 ) ); // 10 + 1 + 10 + 1
   sprintf( imImageSize, "%ux%u", m_width, m_height );
   imImageInfo->size = imImageSize;

   imImageInfo->colorspace = RGBColorspace;
   
   Image* image = AllocateImage( imImageInfo );
   // Allocate all supported colors
   AllocateImageColormap( image, GDUtils::Color::NBR_COLORS);

   PixelPacket colorPixel;
   for ( uint32 i = 0 ; i < image->colors ; i++ ) {
      getColor( GDUtils::Color::CoolColor( i ), 
                colorPixel, 
                image->depth );

      image->colormap[ i ] = colorPixel;

      mc2dbg8 << "imImage col: " << i << " RGB " << (int)colorPixel.red
              << "," << (int)colorPixel.green << "," << (int)colorPixel.blue 
              << endl;
   }

   // Fill it with white
   PixelPacket color;
   PixelPacket borderColor;
   DrawInfo drawInfo;
   GetDrawInfo( imImageInfo, &drawInfo ); // Initialize it
   getColor( GDUtils::Color::makeColor( GDUtils::Color::WHITE ),
             color, image->depth );
   getColor( GDUtils::Color::makeColor( GDUtils::Color::RED ),
             borderColor,  // Not in image
             image->depth );
   drawInfo.fill = color;

   if ( !ColorFloodfillImage( image, &drawInfo, borderColor, 
                              m_width/2, m_height/2, FillToBorderMethod ) )
   {
      mc2log << warn << "IMImageDraw::IMImageDraw failed to reset image!"
             << endl; 
   }

   DEBUG8(sprintf(cStr, "IMImageDraw::IMImageDraw Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);
   m_image = image;
   m_imageInfo = imImageInfo;
}


IMImageDraw::~IMImageDraw() {
   DestroyImage( static_cast<Image*>( m_image ) );
   DestroyImageInfo( static_cast< ImageInfo* > ( m_imageInfo ) );
}


byte*
IMImageDraw::getImageAsBuffer( uint32& size, 
                               ImageDrawConfig::imageFormat format ) 
{
   byte* out = NULL;

   Image* image = static_cast< Image* >( m_image );
   ImageInfo* imageInfo = static_cast< ImageInfo* >( m_imageInfo );
   // Set format
   strcpy( image->magick, ImageDrawConfig::imageFormatMagick[ format ] );
   DEBUG8(char cStr[512]; clock_t m_startTime = clock(); );
/*
   // Quantize to get used colors to reduce number colors
   QuantizeInfo quantize_info;
   GetQuantizeInfo( &quantize_info );
   // Don't require less colors just remove unused
   quantize_info.number_colors = image->colors; // More if antialiasing!
//GetNumberColors( image, NULL );
   quantize_info.tree_depth = 8;
   QuantizeImage( &quantize_info, image );
   DEBUG8(sprintf(cStr, "Quantize Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);

   DEBUG8(mc2dbg << "GetNumberColors " << GetNumberColors( image, NULL ) 
          << " ~= " << image->colors << endl
          << endl;
          for ( uint32 i = 0 ; i < image->colors ; i++ ) {
             PixelPacket testpixel = image->colormap[ i ];
            mc2dbg  << "Image color: " << i << " RGB "
                  << (int)testpixel.red << "," 
                  << (int)testpixel.green << "," << (int)testpixel.blue 
                  << endl;
          } );
*/

   // Make out
   DEBUG8(m_startTime = clock(););
   ExceptionInfo err;
   size = 0;
   size_t imageSize = 0;
   void* res = ImageToBlob( imageInfo, image, &imageSize, &err );
   size = static_cast<uint32>( imageSize );
   
   if ( res == NULL ) {
      mc2log << error << "IMImageDraw::getImageAsBuffer error: " 
                 << err.description << " why: " << err.reason << endl;
   }
   out = (byte*)memcpy( new byte[ size ], res, size );
   LiberateMemory( &res );
   DEBUG8(sprintf(cStr, "ImageToBlob Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);

   return out;
}


void 
IMImageDraw::drawArc( int cx, int cy,
                      int startAngle, int stopAngle, 
                      int innerRadius, int outerRadius,
                      GDUtils::Color::CoolColor color,
                      int lineWidth )
{
   // FIXME: Should use not arc but ellipse is broken can't do 315->225
   // as it start at 315 and adds 1 until 225!
   mc2dbg4 << "IMImageDraw::drawArc( " << cx << "," << cy << ","
           << startAngle << "," << stopAngle << ","
           << innerRadius << "," << outerRadius << " )" << endl;

   Image* image = static_cast< Image* >( m_image );
   ImageInfo* imageInfo = static_cast< ImageInfo* >( m_imageInfo );
   DrawInfo drawInfo;
   GetDrawInfo( imageInfo, &drawInfo ); // Initialize it
   PixelPacket col;
   int startAng = ( startAngle - 90 + 360 ) % 360;
   int stopAng = ( stopAngle - 90 + 360 ) % 360;
   char primitive [ MaxTextExtent ]; // The thing to draw

   getColor( color, col, image->depth );
   drawInfo.stroke = col;
   drawInfo.stroke_width = double( lineWidth );
   drawInfo.stroke_antialias = MB ( true ); // Smother

   if ( startAng == stopAng ) {
      // Full circle
      startAng = 0; 
      stopAng = 360;
   }

   float64 sinStart = sin( startAngle * GfxConstants::degreeToRadianFactor );
   float64 cosStart = -cos( startAngle * GfxConstants::degreeToRadianFactor);
   float64 sinStop = sin( stopAngle * GfxConstants::degreeToRadianFactor );
   float64 cosStop = -cos( stopAngle * GfxConstants::degreeToRadianFactor);
   
   // Inner arc start point
   int isx = cx + int( rint( sinStart*innerRadius ) );
   int isy = cy + int( rint( cosStart*innerRadius ) );

   // Outer arc start point
   int osx = cx + int( rint( sinStart*outerRadius ) );
   int osy = cy + int( rint( cosStart*outerRadius ) );

   // Inner arc end point
   int iex = cx + int( rint( sinStop*innerRadius ) );
   int iey = cy + int( rint( cosStop*innerRadius ) );

   // Outer arc end point
   int oex = cx + int( rint( sinStop*outerRadius ) );
   int oey = cy + int( rint( cosStop*outerRadius ) );

   // Inner arc
   if ( innerRadius > 0 ) {   
      double iangle = GfxConstants::degreeToRadianFactor * 
         (( 360 - ((( stopAngle - startAngle ) / 2 ) + startAngle ) + 90 )
          % 360 );
      int iarcx = cx + int32( rint( cos( iangle ) * innerRadius ) ) - isx;
      int iarcy = cy + int32( -rint( sin( iangle ) * innerRadius ) ) - isy;

      sprintf( primitive, "arc %hu,%hu %hu,%hu %hd,%hd",
               isx, isy, 
               iex, iey, 
               iarcx, iarcy );
      
      drawInfo.primitive = primitive;
      if ( !DrawImage( image, &drawInfo ) ) {
         mc2log << warn << "IMImageDraw::drawArc "
                << "failed to draw inner arc!" << endl << "primitive "
                << primitive << endl;
      }
   }

   // Lines between innner and outer arc, not if full circle
   if ( !(startAng == 0 && stopAng == 360 ) ) {
      // Start line
      sprintf( primitive, "polyline %hu,%hu,%hu,%hu",
               isx, isy, osx, osy );
      drawInfo.primitive = primitive;
      if ( !DrawImage( image, &drawInfo ) ) {
         mc2log << warn << "IMImageDraw::drawArc "
                << "failed to draw start line!" << endl
                << "primitive " << primitive << endl;
      }

      // Stop line
      sprintf( primitive, "polyline %hu,%hu,%hu,%hu",
               iex, iey, oex, oey );
      drawInfo.primitive = primitive;
      if ( !DrawImage( image, &drawInfo ) ) {
         mc2log << warn << "IMImageDraw::drawArc "
                << "failed to draw stop line!" << endl << "primitive "
                << primitive << endl;
      }
   }

   // OuterArc
   double oangle = GfxConstants::degreeToRadianFactor * 
      (( 360 - ((( stopAngle - startAngle ) / 2 ) + startAngle ) + 90 )
       % 360 );
   int oarcx = cx + int32( rint( cos( oangle ) * outerRadius ) ) - osx;
   int oarcy = cy + int32( -rint( sin( oangle ) * outerRadius ) ) - osy;

   sprintf( primitive, "arc %hu,%hu %hu,%hu %hd,%hd",
            osx, osy, 
            oex, oey, 
            oarcx, oarcy  );
      
   drawInfo.primitive = primitive;
   if ( !DrawImage( image, &drawInfo ) ) {
      mc2log << warn << "IMImageDraw::drawArc " << "failed to draw outer arc!"
             << endl << "primitive " << primitive << endl;
   }
}


void 
IMImageDraw::drawTurnArrow( int x0, int y0, int x1, int y1, 
                            int x2, int y2, int x3, int y3,
                            int arrowX0, int arrowY0,
                            int arrowX1, int arrowY1,
                            int arrowX2, int arrowY2,
                            int32 arrowAngle, int32 arrowLength,
                            GDUtils::Color::CoolColor col,
                            int arrowWidth )
{
   mc2dbg4 << "IMImageDraw::drawTurnArrow( " 
           << x0 << "," << y0 << ", "
           << x1 << "," << y1 << ", "
           << x2 << "," << y2 << ", "
           << x3 << "," << y3 << endl
           << arrowX0 << "," << arrowY0 << ", "
           << arrowX1 << "," << arrowY1 << ", "
           << arrowX2 << "," << arrowY2 << " )" << endl;

   vector<int> resX;
   vector<int> resY;
   Image* image = static_cast< Image* >( m_image );
   ImageInfo* imageInfo = static_cast< ImageInfo* >( m_imageInfo );
   DrawInfo drawInfo;
   GetDrawInfo( imageInfo, &drawInfo ); // Initialize it
   PixelPacket color;
   char primitive [ MaxTextExtent ]; // The thing to draw
   int lineWidth = arrowWidth;
   uint32 pos = 0;

   getColor( GDUtils::Color::CoolColor( col ), color, image->depth );
   drawInfo.stroke = color;
   drawInfo.stroke_width = double( lineWidth );
   drawInfo.stroke_antialias = MB( true ); // Smother

   GfxUtility::makeSpline( x0, y0, x1, y1, 
                           x2, y2, x3, y3,
                           resX, resY, 0.05 );

   // Draw spline line
   int x = 0;
   int y = 0;
   
   if ( resX.size() > 0 ) {
      strcpy( primitive, "polyline " );
      pos = strlen( primitive );

      for ( uint32 i = 0 ; i < resX.size() - 1 ; i++ ) {
         x = resX[ i ];
         y = resY[ i ];
         
         pos += sprintf( primitive + pos, "%hu,%hu,", 
                         x, y );
      }
      // Add last coordinate without "," after last number
      pos += sprintf( primitive + pos, "%hu,%hu", 
                      resX[ resX.size() - 1 ],
                      resY[ resX.size() - 1 ] );

      drawInfo.primitive = primitive;
      if ( !DrawImage( image, &drawInfo ) ) {
         mc2log << warn << "IMImageDraw::drawTurnArrow "
                << "failed to draw spline!" << endl 
                << "primitive " << primitive << endl;
      }
   }

   // Draw Arrow
   drawInfo.fill = color;
   strcpy( primitive, "polyline " );
   pos = strlen( primitive );

   pos += sprintf( primitive + pos, "%hu,%hu,%hu,%hu,%hu,%hu", 
                   arrowX0, arrowY0,
                   arrowX1, arrowY1,
                   arrowX2, arrowY2 );

   drawInfo.primitive = primitive;
   if ( !DrawImage( image, &drawInfo ) ) {
      mc2log << warn << "IMImageDraw::drawTurnArrow "
             << "failed to draw arrow!" << endl << "primitive " << primitive
             << endl;
   }
}

void
IMImageDraw:: drawRouteAsArrow(vector<POINT> route,
                               vector<POINT> arrowHead,
                               GDUtils::Color::CoolColor color,
                               uint32 routeWidth)
{
  // TODO: Implement when needed.
}

void 
IMImageDraw::getPixel( uint32 x, uint32 y, 
                       unsigned char& red, 
                       unsigned char& green, 
                       unsigned char& blue )
{
   PANIC( "IMImageDraw::getPixel ", "not implemented!" );
}

/*
#undef DEBUG1(a)
#undef DEBUG2(a)
#undef DEBUG4(a)
#undef DEBUG8(a)
#define DEBUG1(a) oldD1(a)
#define DEBUG2(a) oldD2(a)
#define DEBUG4(a) oldD4(a)
#define DEBUG8(a) oldD8(a)
*/

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GDCropTransparent.h"

#include "FilePtr.h"
#include "TempFile.h"
#include "DataBuffer.h"
namespace GDUtils {

/// functor to determine if point is 100% transparent 
struct IsTransparentOP { 

   explicit IsTransparentOP( gdImagePtr image ):
      m_image( image ),      
      m_transparent( gdImageGetTransparent( image ) )
   { }

   /// @return true if color at ( x, y ) is transparent
   bool operator ()( int x, int y ) {
      int color = gdImageGetPixel( m_image, x, y );
      if ( ( m_transparent != -1 && m_transparent == color ) ||
           gdImageAlpha( m_image, color ) == gdAlphaTransparent ) {
         return true;
      }
      return false;
   }

   gdImagePtr m_image;
   int m_transparent;
};

/// returns true if point is not 100% transparent
struct IsOpaqueOP {
   IsOpaqueOP( gdImagePtr image ):m_transp( image ) { }
   bool operator ()( int x, int y ) { return ! m_transp( x, y ); }
   int getTransparentColor() const { return m_transp.m_transparent; }

   IsTransparentOP m_transp;
};

/// copies pixel from src to dest using src position + (x,y)
struct CopyIndexedOP {
   CopyIndexedOP( gdImagePtr dest, gdImagePtr src,
                  const MC2Point& srcPos ):
      m_dest( dest ), m_src( src ), m_srcPos( srcPos ) { }

   void operator() ( int x, int y ) {
      int colorInOrig = 
         gdImageGetPixel( m_src, 
                          m_srcPos.getX() + x, m_srcPos.getY() + y);

      int colorInDest =
            gdImageColorResolveAlpha( m_dest,
                                      gdImageRed( m_src, colorInOrig ),
                                      gdImageGreen( m_src, colorInOrig ),
                                      gdImageBlue( m_src, colorInOrig ),
                                      gdImageAlpha( m_src, colorInOrig ) );
         gdImageSetPixel( m_dest, x, y, colorInDest );
   }

   gdImagePtr m_dest, m_src;
   MC2Point m_srcPos;
};

bool cropTransparentHotspot( const MC2String& filename, 
                             DataBuffer& buff, MC2Point& hotspot ) {
   FileUtils::FilePtr file( fopen( filename.c_str(), "rb" ) );
   if ( file.get() == NULL ) {
      mc2dbg8 << "[GDCrop] Could not open file: " 
              << filename << endl;
      return false;
   }

   ImagePtr image( gdImageCreateFromPng( file.get() ) );

   if ( image.get() == NULL ) {
      return false;
   }

   CutPoints cut;
   TempFile destFile( "cropTransparent-png", "/tmp/" );
   MC2String destFilename = destFile.getTempFilename();
   if ( ! cropTransparent( image.get(), destFilename, &cut ) ) {
      return false;
   }
   
   const MC2Point srcCenter( gdImageSX( image.get() ) / 2,
                             gdImageSY( image.get() ) / 2 );
   hotspot = srcCenter - cut.getTopLeft();

   buff.memMapFile( destFilename.c_str() );

   return true;
}

bool cropTransparentOffset( const MC2String srcFilename,
                            MC2Point& offset,
                            CutPoints* cutPoints )
{
   FileUtils::FilePtr file( fopen( srcFilename.c_str(), "rb" ) );
   if ( file.get() == NULL ) {
      mc2dbg << "[GDCrop] Could not open file: " 
             << srcFilename << endl;
      return false;
   }

   ImagePtr image( gdImageCreateFromPng( file.get() ) );

   if ( image.get() == NULL ) {
      return false;
   }

   ImagePtr otherImage ( cropTransparent( image.get(), cutPoints ) );
   if ( !otherImage.get() ) {
      return false;
   }

   // Calc the center point of the visible rect, described by the cut points.
   MC2Point new_c( ( ( cutPoints->getBottomRight().getX() - 
                       cutPoints->getTopLeft().getX() ) / 2 ) + cutPoints->getTopLeft().getX(), 
                   ( ( cutPoints->getBottomRight().getY() - 
                       cutPoints->getTopLeft().getY() ) / 2 ) + cutPoints->getTopLeft().getY() );

   // Calc the center point of the image.
   MC2Point old_c( gdImageSX( image ) / 2,
                   gdImageSY( image ) / 2 );

   // Offset point is center point of image - center point of the visible rect (cut points)
   offset = old_c - new_c;
   return true;
}


bool cropTransparent( gdImagePtr image,
                      const MC2String& destFilename,
                      CutPoints* cutPoints ) {

   // make sure we can create and write destination file first.
   FileUtils::FilePtr pngfile( fopen( destFilename.c_str(), "wb") );

   if ( pngfile.get() == NULL ) {
      return false;
   }

   ImagePtr otherImage ( cropTransparent( image, cutPoints ) );
   if ( !otherImage.get() ) {
      return false;
   }
   // save image to file
   gdImagePng( otherImage.get(), pngfile.get() );

   // return cut points 
   if ( cutPoints == NULL ) {
      return false;
   }

   return true;
}

bool cropTransparent( const MC2String& filename,
                      const MC2String& destFilename,
                      CutPoints* cutPoints ) {

   FileUtils::FilePtr file( fopen( filename.c_str(), "rb" ) );
   if ( file.get() == NULL ) {
      mc2dbg<<"[GDCrop] Could not open file: "
            << filename << endl;
      return false;
   }

   ImagePtr image( gdImageCreateFromPng( file.get() ) );

   if ( image.get() == NULL ) {
      return false;
   }

   return cropTransparent( image.get(), destFilename, cutPoints );
}

CutPoints findCroppingPoints( gdImagePtr image ) {
   // functor to determine if point is opaque
   IsOpaqueOP isOpaque( image );

   const int width = gdImageSX( image );
   const int height = gdImageSY( image );
   // find first side
   // for each row find the first non-transparent pixel
   int minX = width;
   int maxX = 0;
   for ( int y = 0; y < height; ++y ) {
      for ( int x = 0; x < width; ++x ) {
         if ( isOpaque( x, y ) ) {
            minX = min( minX, x );
            break;
         }
      }
      for ( int x = width - 1; x > 0; --x ) {
         if ( isOpaque( x, y ) ){
            maxX = max( maxX, x );
            break; // next row
         }
      }
   }


   // find bottom/top transparent clip:
   // note: starting on min X to max X.
   int maxY = 0;
   int minY = height;
   for ( int x = minX; x < maxX; ++x ) {
      for ( int y = 0; y < height; ++y ) {
         if ( isOpaque( x, y ) ){
            minY = min( minY, y );
            break;
         }
      }

      for ( int y = height - 1; y > 0; --y ) {
         if ( isOpaque( x, y ) ) {
            maxY = max( maxY, y );
            break; // next column
         }
      }
   }
   return CutPoints( MC2Point( minX, maxY ),
                     MC2Point( maxX, minY ) );
}

CutPoints findCroppingPoints( const SharedBuffer& buffer ) {
   ImagePtr image( gdImageCreateFromPngPtr( buffer.getBufferSize(),
                                            buffer.getBufferAddress() ) );
   return findCroppingPoints( image.get() );
}

gdImagePtr cropTransparent( gdImagePtr image,
                            CutPoints* cutPoints )
{
   // get Image properties
   const int width = gdImageSX( image );
   const int height = gdImageSY( image );

   int minX = width;
   int minY = height;
   int maxX = 0;
   int maxY = 0;
   {
      CutPoints cropPoints = findCroppingPoints( image );
      minX = cropPoints.getTopLeft().getX();
      maxY = cropPoints.getTopLeft().getY();
      maxX = cropPoints.getBottomRight().getX();
      minY = cropPoints.getBottomRight().getY();
   }

   // make the cutting area large by adding one pixel
   // on right and bottom side. This is because we want to cut
   // in the transparent part, not the visible part.

   //   -----------------------   minY
   //   |                     | <---
   //   |        ********     |
   //   |        ********     |
   //   |        ********     |
   //   |        ********     |
   //   |        ********     |   maxY
   //   |        ********     | <---
   //   |                     |
   //   |                     |
   //   |                     |
   //   -----------------------
   //           ^        ^
   //           | minX   | maxX
   //           |        |
   //           |        |
   //

   // add one pixel if maxX != width
   maxX = ( maxX == width ? maxX : maxX + 1 );
   // add one pixel if maxY != height
   maxY = ( maxY == height ? maxY : maxY + 1 );

   // now we have our minX and maxY, time to start crop
   const int otherWidth = maxX - minX;
   const int otherHeight = maxY - minY;

   ImagePtr otherImage( gdImageCreate( otherWidth, otherHeight ) );
   if ( otherImage.get() == NULL ) {
      return NULL;
   }

   // copy the cut region of the src image region to the destination image
   CopyIndexedOP copyIndexed( otherImage.get(), image,
                              MC2Point( minX, minY ) );
   for ( int y = 0; y < otherHeight; ++y ) {
      for ( int x = 0; x < otherWidth; ++x ) {
         copyIndexed( x, y );
      }
   }

   // return cut points
   if ( cutPoints != NULL ) {
      *cutPoints = CutPoints( MC2Point( minX, minY ),
                              MC2Point( maxX, maxY ) );
   }

   return otherImage.release();
}



int bufferSinkCallback( void* context, const char* buffer, int len ) {
   vector< uint8 >& data = *reinterpret_cast< vector< uint8 >* >( context );
   data.insert( data.end(), (uint8*)buffer, (uint8*)( buffer + len ) );
   return len;
}

std::auto_ptr< SharedBuffer >
cropTransparent( const SharedBuffer& pngBuffer ) {
   ImagePtr image( gdImageCreateFromPngPtr( pngBuffer.getBufferSize(),
                                            pngBuffer.getBufferAddress() ) );
   if ( image.get() == NULL ) {
      return std::auto_ptr< SharedBuffer >();
   }

   ImagePtr croppedImage( cropTransparent( image.get() ) );
   vector< uint8 > outputdata;
   gdSink sink;
   sink.context = &outputdata;
   sink.sink = bufferSinkCallback;

   gdImagePngToSink( croppedImage.get(), &sink );
   SharedBuffer outputDataBuffer( &outputdata[ 0 ], outputdata.size() );
   return std::auto_ptr< SharedBuffer >
      ( new SharedBuffer( outputDataBuffer ) );
}

} // GDUtils

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserImage.h"

#include "SVGImage.h"

#include "BitBuffer.h"
#include "DataBufferUtil.h"
#include "GDCropTransparent.h"
#include "ImageTools.h"
#include "GDImageDraw.h"
#include "File.h"
#include "FilePtr.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <math.h>
#include "STLStringUtility.h"

namespace {

/**
 * @param pngBuffer The buffer created from
 * @return cropped PNG file in buffer 
 */
std::auto_ptr< BitBuffer > cropIt( const BitBuffer& pngBuffer ) {

   std::auto_ptr< SharedBuffer >
      cropped( GDUtils::cropTransparent( pngBuffer ) );

   // convert SharedBuffer to BitBuffer and
   // take ownership of the data
   uint32 size = cropped->getBufferSize();
   std::auto_ptr< BitBuffer > retBuff( new BitBuffer( cropped->release(),
                                                      size ) );
   retBuff->setSelfAlloc( true );

   
   return retBuff;
}

}
namespace UserImage {

std::auto_ptr< BitBuffer >
createCustomImageSize( const MC2String& imagePath,
                       const MC2String& inFilename,
                       bool cropped ) {

   int32 width = -1;
   int32 height = -1;
   MC2String filename = getFilename( inFilename, width, height );

   if ( filename.empty() ) {
      return std::auto_ptr< BitBuffer >();
   }
   filename += ".svg";

   MC2String fullFilename = imagePath + "/" + filename;

   std::auto_ptr< DataBuffer > buff( ImageTools::
                                     createPNGFromSVG( fullFilename,
                                                       width, height ) );

   // If the image is suppose to be cropped, then we need to find out
   // where it is going to crop it and make the image even larger to
   // match the requested size
   if ( cropped && buff.get() != NULL ) {
      GDUtils::CutPoints croppingPoints =
         GDUtils::findCroppingPoints( *buff );

      // determine new image size by current crop percentage
      float32 widthDiff = 
         1.0 / ((croppingPoints.getWidth() + 1) / (float32)width);
      float32 heightDiff = 
         1.0 / ((croppingPoints.getHeight()+ 1) / (float32)height);

      // if size differs, request new scaled image
      if ( croppingPoints.getWidth() != 0 ||
           croppingPoints.getHeight() != 0 ) {
         // rescale the image so we get correct image once we crop it.
         buff = ImageTools::
            createPNGFromSVG( fullFilename,
                              static_cast< uint32 >
                              ( round( width * widthDiff ) ),
                              static_cast< uint32 >
                              ( round( height * heightDiff ) ) );
      }
   }

   if ( buff.get() == NULL ) {
      return auto_ptr< BitBuffer >();
   }


   std::auto_ptr< BitBuffer > retBuffer = DataBufferUtil::convertToBitBuffer( *buff );

   if ( cropped ) {
      retBuffer = ::cropIt( *retBuffer );
   }

   return retBuffer;
}

MC2String getFilename( const MC2String& filename,
                       int32& width, int32& height ) try {
   width = -1;
   height = -1;
   boost::regex reg( "^(.*)_([0-9]*)x([0-9]*)\\.png$" );

   boost::smatch matches;
   if ( boost::regex_search( filename, matches, reg ) ) {
      if ( matches[ 1 ].matched &&
           matches[ 2 ].matched &&
           matches[ 3 ].matched ) {
         width = boost::lexical_cast< int32 >( matches[ 2 ].str() );
         height = boost::lexical_cast< int32 >( matches[ 3 ].str() );
         return matches[ 1 ];
      }
   }

   return MC2String();
} catch ( const boost::bad_lexical_cast& bc ) {
   // width might be set during lexical_cast
   width = -1;
   height = -1;
   return MC2String();
}


bool getPNGImageSize( const MC2String& filePath,
                       int32& width, int32& height )
{
   FileUtils::FilePtr file( fopen( filePath.c_str(), "rb" ) );
   GDUtils::ImagePtr image( NULL );
   if ( file.get() != NULL ) {
      // Read
      image.reset( gdImageCreateFromPng( file.get() ) );
      if ( image.get() == NULL ) {
         mc2log << warn << "[UserImage] Failed to load png file: " 
                << filePath << endl;
         return false;
      }
   }

   if ( image.get() != NULL ) {
      width = gdImageSX( image );
      height = gdImageSY( image );
      return true;
   } else {
      return false;
   }
}

bool magnify( const MC2String& origName, const MC2String& destName,
              uint32 factor, bool cropped )
{
   // Try from svg
   int32 width = 0;
   int32 height = 0;
   if ( UserImage::getPNGImageSize( origName, width, height ) ) {
      MC2String imagePath( STLStringUtility::dirname( origName ) );
      MC2String fileName( STLStringUtility::basename( origName ) );
      char customImageStr[ fileName.size() + 20*2 + 10 ];
      sprintf( customImageStr, "%s_%hux%hu.png", 
               STLStringUtility::basename( fileName, ".png" ).c_str(),
               width * factor, height * factor );
      mc2dbg << "Magnifying " << origName << " using " << customImageStr
             << endl;
      auto_ptr< BitBuffer > buff( 
         UserImage::createCustomImageSize( imagePath, customImageStr, 
                                           cropped ) );
      if ( buff.get() != NULL ) {
         // Save as destName
         int32 buffLen = buff->getBufferSize();
         if ( File::writeFile( destName.c_str(), buff->getBufferAddress(),
                               buffLen ) == buffLen ) {
            // Written ok
            return true;
         } else {
            // Try ImageTools::magnify below
            remove( destName.c_str() ); // Just in case it is left
         }
      }
   } // End if we could get the original image width and height
   
   return ImageTools::magnify( origName, destName, factor );
      
}

} // UserImage

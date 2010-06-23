/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"

#include "SVGImage.h"

#include "Cairo.h"
#include "TempFile.h"
#include <librsvg/rsvg.h>

#include <fstream>

/// Tests the so the image size is.
/// @param buff Contains image data.
/// @param width The expected width in pixels.
/// @param height The expected height in pixels.
void checkImageSize( const DataBuffer& buff,
                     uint32 width, uint32 height ) {
   // write image data to file and then load it
   TempFile tmpImageFile( "unittest", "/tmp" );
   MC2_TEST_REQUIRED( tmpImageFile.ok() );
   {
      ofstream outfile( tmpImageFile.getTempFilename().c_str() );
      outfile.write( (const char*)buff.getBufferAddress(),
                     buff.getBufferSize() );
   }

   std::auto_ptr< GSystem::Surface >
      surface( GSystem::Cairo::
               loadPNG( tmpImageFile.getTempFilename() ) );
   MC2_TEST_REQUIRED( surface.get() );
   MC2_TEST_CHECK_EXT( surface->getWidth() == width, width );
   MC2_TEST_CHECK_EXT( surface->getHeight() == height, height );

}

// Tests SVG to PNG conversion.
MC2_UNIT_TEST_FUNCTION( svgImageTest ) {

   // Test a fail loading
   std::auto_ptr< DataBuffer >
      imagedata( ImageTools::createPNGFromSVG( "",
                                               -1, -1 ) );
   MC2_TEST_CHECK( imagedata.get() == NULL );

   // Test a successful loading
   imagedata = ImageTools::createPNGFromSVG( "data/image.svg",
                                             -1, -1 );

   MC2_TEST_REQUIRED( imagedata.get() != NULL );

   checkImageSize( *imagedata, 40, 40 );

   // Test a successful loading
   imagedata = ImageTools::createPNGFromSVG( "data/image.svg",
                                             320, 480 );

   MC2_TEST_REQUIRED( imagedata.get() != NULL );
   checkImageSize( *imagedata, 320, 480 );

   // we should check the size here...
}

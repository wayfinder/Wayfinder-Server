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

#include "UserImage.h"

#include "DataBuffer.h"
#include "BitBuffer.h"

#include "TempFile.h"
#include "Cairo.h"


#include <fstream>
#include <memory>

/// Tests the so the image size is correct.
/// @param buff Contains image data.
/// @param width The expected width in pixels.
/// @param height The expected height in pixels.
void checkImageSize( const SharedBuffer& buff,
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

MC2_UNIT_TEST_FUNCTION( userImageTest ) {
   using namespace UserImage;
   // test fail.
   std::auto_ptr< BitBuffer > buff;
   buff = createCustomImageSize( "", "", false );
   MC2_TEST_CHECK( buff.get() == NULL );

   // test success and an image size of 320x480
   // this should load customimage.svg in data directory
   buff = createCustomImageSize( "data",
                                 "customimage_320x480.png", false );

   MC2_TEST_REQUIRED( buff.get() );
   checkImageSize( *buff, 320, 480 );

   // test success and image size of 50x50 and with cropping enabled.
   buff = createCustomImageSize( "data",
                                 "customimage_50x50.png", true );
   MC2_TEST_REQUIRED( buff.get() );
   checkImageSize( *buff, 50, 49/*Cropping all transparent rows gives this*/ );

}

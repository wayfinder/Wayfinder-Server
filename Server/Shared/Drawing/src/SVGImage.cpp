/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SVGImage.h"
#include "GdkPixbufPtr.h"

#include <librsvg/rsvg.h>

namespace ImageTools {

std::auto_ptr< DataBuffer > createPNGFromSVG( const MC2String& filename,
                                              int32 width, int32 height ) {

   typedef std::auto_ptr< DataBuffer > ReturnValue;

   // Must initialize gobject subsystems first
   // Initialize this several times is no problem,
   // although not sure about thread issues
   g_type_init();

   // Load SVG to pixbug
   GError *gerror = NULL;
   GdkPixbufPtr imagedata( rsvg_pixbuf_from_file_at_size( filename.c_str(),
                                                          width, height,
                                                          &gerror ) );

   if ( imagedata.get() == NULL ) {
      if ( gerror ) {
         mc2log << error << "[createPNGFromSVG]: "
                << gerror->message << " for file " << filename << endl;
         g_error_free( gerror );
      }
      return ReturnValue();
   }

   // Save pixbuf to buffer as PNG format
   gerror = NULL;
   gchar *buffer = NULL;
   gsize buffer_size = 0;
   if ( gdk_pixbuf_save_to_buffer( imagedata.get(),
                                   &buffer, &buffer_size,
                                   "png",
                                   &gerror, NULL ) == FALSE ) {

      mc2log << error << "[createPNGFromSVG]: "
             << gerror->message << endl;
      g_error_free( gerror );
      return ReturnValue();
   }

   // meh, we have to copy it so we can use consistent delete []
   // instead of free()
   ReturnValue retbuff( new DataBuffer( static_cast< uint32 >
                                        ( buffer_size ) ) );
   memcpy( retbuff->getBufferAddress(), buffer, buffer_size );
   g_free( buffer );

   return retbuff;
}

} // GSystem

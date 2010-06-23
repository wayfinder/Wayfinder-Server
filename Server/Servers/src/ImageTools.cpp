/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ImageTools.h"

#include "TempFile.h"
#include "GDCropTransparent.h"
#include "STLStringUtility.h"
#include "MC2Point.h"
#include "DataBuffer.h"
#include "BitBuffer.h"
#include "stat_inc.h"
#include <unistd.h>
#include <stdlib.h>

namespace ImageTools {

bool convert( const MC2String& origname, const MC2String& convertFromExt ) {

   MC2String origDirName( STLStringUtility::dirname( origname ) );
#if 0
   if ( origDirName[0] != '/' ) {
      // Prepend the current dir.
      if ( origDirName[0] == '.' ) {
         // Remove dot
         origDirName.erase(0);
      }
      char curDir[1024];
      char* cwd = getcwd(curDir, sizeof(curDir));
      if ( cwd == NULL ) {
         return false;
      } else {
         origDirName = MC2String(cwd) + origDirName;
      }
   }
#endif
   MC2String origExt( STLStringUtility::fileExtension( origname, true ) );
   MC2String origBaseName( STLStringUtility::basename( origname, origExt ) );
   MC2String pngName = origDirName + '/' + origBaseName + convertFromExt;

   // Do not convert to mif since convert will convert to png
   if ( origExt == "mif" ) {
      mc2dbg << "[ImageTools::convert]: Not converting .mif file since that is "
             << "impossible" << endl;
      return 0;
   }

   // OrigExt seems to be the destination ext
   if ( origExt == "apa" ) {
      mc2dbg << "[ImageTools::convert]: Apa-file requested" << endl;
      DataBuffer buf;
      MC2Point hotspot( 0, 0 );
      if ( ! GDUtils::cropTransparentHotspot( pngName, buf, hotspot ) ) {
         return 1;
      }
      mc2dbg << "[ImageTools::convert]: Hotspot for " << origname << " is "
             << hotspot << endl;
      // Use temporary file to avoid trouble
      TempFile fil( origBaseName + ".xox",
                    origDirName,
                    origBaseName + "apa",
                    origDirName );
      int expected1 = buf.getBufferSize();
      if ( fil.write( buf.getBufferAddress(), expected1 )
           != expected1 ) {
         mc2dbg << "[ImageTools::convert]: Error when writing png part of apa" << endl;
         return 1;
      }
      // Now add the hotspot
      DataBuffer hotSpotBuf( 8 );
      hotSpotBuf.writeNextShort( hotspot.getX() );
      hotSpotBuf.writeNextShort( hotspot.getY() );
      int expected = hotSpotBuf.getCurrentOffset();
      if ( fil.write( hotSpotBuf.getBufferAddress(), expected ) != expected ) {
         mc2dbg << "[ImageTools::convert]: Error when writing hotspot" << endl;
         return 1;
      }
      return 0;
   }
 
   MC2String convertBinary( "convert" );
   mc2dbg << "[ImageTools::convert]: Would convert from "
          << pngName << " to " << origname << endl;
   pid_t childPid = fork();
   if ( childPid ) {
      int status;
      struct rusage rusage;
      pid_t res = wait3(&status, 0, &rusage);
      mc2dbg << "[ImageTools::convert]: Convert has exited - status is " << status
             << endl;
      return res > 0 && status == 0;
   } else {      
      int res = execlp(convertBinary.c_str(), "convert run by XMLServer",
             pngName.c_str(), origname.c_str(), NULL);
      // End child process forked above, return the result code.
      exit( res );
      // Should not get here.
      return 0;
   }
}


bool magnify( const MC2String& origName, const MC2String& destName,
              uint32 factor ) {
   mc2dbg << "[ImageTools::magnify]: Would convert from "
          << origName << " to " << destName << " Magnifying " << factor
          << " times." << endl;

   MC2String convertBinary( "convert" );
   pid_t childPid = fork();
   if ( childPid ) {
      int status;
      struct rusage rusage;
      pid_t res = wait3(&status, 0, &rusage);
      mc2dbg << "[ImageTools::magnify]: magnify has exited - status is " 
             << status << endl;
      return res > 0 && status == 0;
   } else {
      // -magnify <factor> This does not exist in our version of ImageMagick
      // -resize %200 (alias for -geometry %200)
      const char* magnifyStr = "-resize";
      char factorStr[20];
      sprintf( factorStr, "%%%u00", factor );
      int res = execlp( convertBinary.c_str(), 
                        "convert -magnify run by MC2 Server", magnifyStr,
                        factorStr, origName.c_str(), destName.c_str(), NULL );
      // End child process forked above, return the result code.
      exit( res );
      // Should not get here.
      return 0;
   }
}

} // End namespace ImageTools

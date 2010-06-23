/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GDRGB_H
#define GDRGB_H

#include "config.h"

class Packet;

namespace GDUtils {

/**
 * Color utilities for GDUtils
 */
namespace Color {

/// holds red, green and blue values
struct RGB {
   int red;
   int green;
   int blue;

   /// Empty RGB.
   RGB() {}

   /// RGB from values.
   RGB( int r, int g, int b ) 
         : red( r ), green( g ), blue( b ) {}

   /**
    * Load from packet.
    */
   void load( const Packet* p, int& pos );

   /**
    * Save to packet.
    */
   void save( Packet* p, int& pos ) const;

   /**
    * @return Size in packet. 
    */
   uint32 getSizeInPacket() const;

   /**
    * Print on ostream.
    *
    * @param stream The stream to print on.
    * @param rgb The RGB to print.
    * @return The stream.
    */
   friend ostream& operator<< ( ostream& stream,
                                const RGB& rgb )
   {
      return stream << "(" << rgb.red << "," << rgb.green << "," 
                    << rgb.blue << ")";
   }

   bool operator < ( const RGB& o ) const {
      if ( red != o.red ) {
         return red < o.red;
      } else if ( green != o.green ) {
         return green < o.green;
      } else {
         return blue < o.blue;
      }
   }

   bool operator == ( const RGB& o ) const {
      return !(*this < o || o < *this);
   }
};


} // namespace Color

} // namespace GDUtils


#endif // GDRGB_H


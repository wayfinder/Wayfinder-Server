/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_PLOTTERTYPES_H
#define MAP_PLOTTERTYPES_H

#include "MapPlotterConfig.h"

namespace isab {
   /**
    *   The different types of bitmaps that need to
    *   be supported. The server will not send a bitmap
    *   of the wrong type to a client which does not support it.
    */
   enum bitMapType {
      png  = 0,
      gif  = 1,
      wbmp = 2
   };
   
   /**
    *   The style of the line endings.
    */
   enum capStyle {
      /// The lines end and their end coordinate.
      flatCap   = 0,
      /// The lines are extended by half their width.
      squareCap = 1,
      /// The lines are ended by a circle with the width as diameter
      roundCap  = 2,
   };
   
   /**
    *   The different dash styles of the pen.
    */
   enum dashStyle {
      /// The pen will not draw
      nullDash = 0,
      /// The pen is solid
      solidDash = 1,
      /// A dotted line
      dottedDash = 2,
      /// A dashed line <pre>- - - -</pre>
      dashedDash = 3,
      /// A dot-dashed line <pre>. _ . _ .</pre>
   };

   /**
    *   Rectangle.
    */
   class Rectangle {
   public:
      /**
       *   Creates a new Rectangle with the supplied width
       *   and height and a corner in x,y.
       */
      Rectangle( int x = 0, int y = 0,
                 unsigned int width = 0, unsigned int height = 0) :
         m_x(x), m_y(y), m_width(width), m_height(height) {}

      /**
       *   Returns the x coordinate.
       */
      int getX() const { return m_x; }

      /**
       *   Returns the y coordinate.
       */
      int getY() const { return m_y; }

      /**
       *   Returns the height of the rectangle.
       */
      unsigned int getHeight() const { return m_height; }

      /**
       *   Returns the width of the rectangle.
       */
      unsigned int getWidth() const { return m_width; }

   private:
      
      int m_x;
      int m_y;      
      unsigned int m_width;
      unsigned int m_height;      
   };


}

#endif

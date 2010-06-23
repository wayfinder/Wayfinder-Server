/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PIXELBOX
#define PIXELBOX

#include "config.h"
#include "MC2BoundingBox.h"
#include "MC2Point.h"
#include "MapPlotterTypes.h"

#ifdef __SYMBIAN32__
// This is to be able to use TRect
#   include<e32std.h>
#endif

/**
 *    Boundingbox with pixels.
 */
class PixelBox : public MC2BoundingBox 
{   
public:
   /**
    *    Constructor. Creates an invalid pixelbox.
    */
   inline PixelBox();
   
   /**
    *    Constructor.
    *    @param   aCorner     A corner.
    *    @param   otherCorner Other corner.
    */
   inline PixelBox( const MC2Point& aCorner,
                    const MC2Point& otherCorner );
   
   /**
    *    Constructor.
    *    @param rectangle Rectangle to create the box from.
    */
   inline PixelBox( const isab::Rectangle& rectangle );

#ifdef __SYMBIAN32__
   /**
    *    Construct a PixelBox from a TRect.
    */
   inline PixelBox( const TRect& trect ) {
      minLat = trect.iTl.iY;
      maxLat = trect.iBr.iY;
      maxLon = trect.iBr.iX;
      minLon = trect.iTl.iX;
      cos_lat = 0;
   }
#endif

   /**
    *    Updates the bounding box to cover the specified point.
    */
   inline void update(const MC2Point& point);

   ///   Updates the box to cover itself plus the other box.
   inline void update(const PixelBox& otherBox);

   ///   Moves the pixelbox by the x and y values in the point. Returns the box
   inline PixelBox& move( const MC2Point& deltas );
   
   /**
    *    Returns true if the points are inside.
    */
   inline int pointInside( const MC2Point& point ) const;
   
   /**
    *    Sets the corners of the PixelBox.
    */
   inline void set(const MC2Point& aCorner, const MC2Point& oppositCorner);

   /**
    *    Returns the center point of the pixbox.
    */
   inline MC2Point getCenterPoint() const;

   ///    @return The top left corner
   inline MC2Point getTopLeft() const;
   ///    @return The bottom left corner
   inline MC2Point getBottomLeft() const;
   ///    @return The bottom right corner
   inline MC2Point getBottomRight() const;
   ///    @return The top right corner
   inline MC2Point getTopRight() const;
   ///    @return Top-left, bottom-left, bottom-right or top-right corner
   inline MC2Point getCorner(int nbr) const;

   ///    Moves the specified corner of the box to the specified point
   inline PixelBox& moveTopLeftTo(const MC2Point& topLeft);
   ///    Moves the specified corner of the box to the specified point
   inline PixelBox& moveBottomLeftTo(const MC2Point& bottomLeft);
   ///    Moves the specified corner of the box to the specified point
   inline PixelBox& moveBottomRightTo(const MC2Point& bottomRight);
   ///    Moves the specified corner of the box to the specified point
   inline PixelBox& moveTopRightTo(const MC2Point& topRight);
   ///    Moves the center to to the specified point.
   inline PixelBox& moveCenterTo(const MC2Point& newCenter);

   /**
    *    Returns a rectangle representing this bounding box.
    */
   inline operator isab::Rectangle() const;

#ifdef __SYMBIAN32__
   /**
    *    Creates a TRect from this pixelbox.
    */
   inline operator TRect() const;
#endif
   
   /**
    *    Snaps otherBox so that it is inside this box.
    *    Note that otherBox must fit inside the box for this
    *    method to work.
    *    @return How much the box was offset.
    */
   inline MC2Point snapToBox( PixelBox& otherBox ) const;
};

// --- Implementation of inlined methods ---


inline int
PixelBox::pointInside( const MC2Point& point ) const
{
   return inside( point.getY(), point.getX() );
}

inline void
PixelBox::set(const MC2Point& aCorner, const MC2Point& oppositCorner)
{
   maxLat = MAX( aCorner.getY(), oppositCorner.getY() );
   minLat = MIN( aCorner.getY(), oppositCorner.getY() );
   maxLon = MAX( aCorner.getX(), oppositCorner.getX() );
   minLon = MIN( aCorner.getX(), oppositCorner.getX() );
}

inline
PixelBox::PixelBox() : MC2BoundingBox()
{
   cos_lat = 1.0;
}

inline 
PixelBox::PixelBox( const MC2Point& aCorner, const MC2Point& otherCorner )
  : MC2BoundingBox(false, false)
{   
   cos_lat = 1.0;
   set(aCorner, otherCorner);
}

inline
PixelBox::PixelBox( const isab::Rectangle& rectangle )
  : MC2BoundingBox( false, false )
{
   cos_lat = 1.0;
   minLat = rectangle.getY();
   minLon = rectangle.getX();
   maxLat = minLat + rectangle.getHeight();
   maxLon = minLon + rectangle.getWidth();
}

inline MC2Point
PixelBox::getCenterPoint() const
{
   return MC2Point( minLon + ( (maxLon - minLon) >> 1 ),
                    minLat + ( (maxLat - minLat) >> 1 ) );
}

inline MC2Point
PixelBox::getTopLeft() const
{
   return MC2Point( getMinLon(), getMinLat() );
}
inline MC2Point
PixelBox::getBottomLeft() const
{
   return MC2Point( getMinLon(), getMaxLat() );
}

inline MC2Point
PixelBox::getBottomRight() const
{
   return MC2Point( getMaxLon(), getMaxLat() );
}

inline MC2Point
PixelBox::getTopRight() const
{
   return MC2Point( getMaxLon(), getMinLat() );
}

inline MC2Point
PixelBox::getCorner(int nbr) const
{
   switch ( nbr & 3 ) {
      case 0:
         return getTopLeft();
      case 1:
         return getBottomLeft();
      case 2:
         return getBottomRight();
      default:
         return getTopRight();
   }
}

inline void
PixelBox::update(const MC2Point& point)
{
   MC2BoundingBox::update( point.getY(), point.getX(), false );
}

inline void
PixelBox::update(const PixelBox& other)
{
   update( other.getTopRight() );
   update( other.getBottomLeft() );
}

inline PixelBox&
PixelBox::move( const MC2Point& deltas )
{
   minLon += deltas.getX();
   maxLon += deltas.getX();
   minLat += deltas.getY();
   maxLat += deltas.getY();
   return *this;
}

inline PixelBox::operator isab::Rectangle() const
{
   return isab::Rectangle( minLon, minLat, getLonDiff(), getHeight() );
}

#ifdef __SYMBIAN32__
inline PixelBox::operator TRect() const
{
   return TRect( getTopLeft(), getBottomRight() );
}
#endif // __SYMBIAN32__

inline PixelBox&
PixelBox::moveTopLeftTo(const MC2Point& topLeft)
{
   return move( topLeft - getTopLeft() );
}

inline PixelBox&
PixelBox::moveBottomLeftTo(const MC2Point& bottomLeft)
{
   return move( bottomLeft - getBottomLeft() );
}

inline PixelBox&
PixelBox::moveBottomRightTo(const MC2Point& bottomRight)
{
   return move( bottomRight - getBottomRight() );
}

inline PixelBox&
PixelBox::moveTopRightTo(const MC2Point& topRight)
{
   return move( topRight - getTopRight() );
}

inline PixelBox&
PixelBox::moveCenterTo(const MC2Point& newCenter)
{
   return move( newCenter - getCenterPoint() );
}

inline MC2Point 
PixelBox::snapToBox( PixelBox& otherBox ) const
{
   // Not that this method won't work if otherBox does not
   // fit inside this box.

   MC2Point p( 0, 0 );

   // Check left boundary.
   int delta = otherBox.getMinLon() - getMinLon();
   if ( delta < 0 ) {
      p.getX() = delta;
   } else {
      // Check right.
      delta = otherBox.getMaxLon() - getMaxLon();
      if ( delta > 0 ) {
         p.getX() = delta;
      }
   }

   // Check top boundary.
   delta = otherBox.getMinLat() - getMinLat();
   if ( delta < 0 ) {
      p.getY() = delta;
   } else {
      // Check bottom.
      delta = otherBox.getMaxLat() - getMaxLat();
      if ( delta > 0 ) {
         p.getY() = delta;
      }
   }

   // Now move to the correct position.
   otherBox.move( MC2Point( -p.getX(), -p.getY() ) );

   return p;
}

#endif

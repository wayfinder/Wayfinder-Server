/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_PROJECTION_H
#define MAP_PROJECTION_H

// This is due to the fact that TileMapConfig contains the right
// ifdefs
#include "TileMapConfig.h"
#include "TransformMatrix.h"
#include "MC2BoundingBox.h"
#include "MC2Point.h"

/**
 *   Class that describes a projection of a map onto a screen.
 *   Handles zooming, rotation etc.
 */
class MapProjection : public TransformMatrix {
public:

   /**
    *   Creates new projection in Lund.
    */
   MapProjection();
   
   /**
    *   Creates a new projection.
    */
   MapProjection(const MC2Coordinate& centerCoord,
                 int screenX, int screenY, double scale,
                 double angle = 0);

   /**
    *   Sets the screen size.
    */
   void setScreenSize(const MC2Point& size);
   
   /**
    *   Zooms the display. Value larger than one means zoom in.
    */
   double zoom(double factor);

   /**
    *   Zooms the display to the supplied corners.
    *   Does not work for angles other than 0 for the moment.
    */
   void setPixelBox( const MC2Point& oneCorner,
                     const MC2Point& otherCorner );

   /**
    *   Sets scale in meters per pixel.
    *   @param scale New scale.
    *   @return The scale set.
    */
   double setScale(double scale);

   /**
    *   Returns the current scale in meters per pixel.
    */
   double getScale() const;

   /**
    *   Sets the center coordinate to newCenter.
    */
   void setCenter(const MC2Coordinate& newCenter);

   /**
    *   Sets the specified point on the screen to the
    *   specified coordinate.
    *   @param newCoord    The new coordinate to move the specified
    *                      point to.
    *   @param screenPoint The point on the screen to set to the
    *                      specified coordinate.
    */
   void setPoint(const MC2Coordinate& newCoord,
                 const MC2Point& screenPoint );
   
   /**
    *   Moves the display 
    */
   void move(int deltaX, int deltaY);

   /**
    *   Sets the angle to the number of degrees in the
    *   parameter. Rotates around the center.
    */
   void setAngle(double angleDegrees);

   /**
    *   Sets the angle to the number of degrees in the
    *   parameter.
    *   @param angleDegrees  Angle in degrees.
    *   @param rotationPoint Point to rotate around.
    */
   void setAngle( double angleDegrees,
                  const MC2Point& rotationPoint );

   /**
    *    Returns the current angle.
    */
   double getAngle() const;
   
   /**
    *   Returns the screensize of the projection.
    */
   inline const MC2Point& getScreenSize() const;

   /**
    *   Returns a reference to the center coordinate.
    */
   inline const MC2Coordinate& getCenter() const;
   
   /**
    *   Sets new boundingbox.
    *   @param bbox The new bounding box.
    *   @return The bounding box that was really set.
    */
   MC2BoundingBox setBoundingBox(const MC2BoundingBox& bbox);

   /**
    *   Calculates the bounding box for the projection.
    */
   MC2BoundingBox getBoundingBox() const;   
   
   /**
    *   Returns the scale to be used with the given screen size.
    */
   static double getScaleFromBBoxAndSize( int screenXSize, 
                                          int screenYSize,
                                          MC2BoundingBox& bbox );

   /**
    *   Returns the lower left coordinate. Warning!! Calculated.
    *   Should use center instead.
    */
   MC2Coordinate calcLowerLeft() const;
   
protected:
   /**
    *   Updates the matrix.
    */
   void updateTransformMatrix();

private:
   /**
    *   Center coordinate.
    */
   MC2Coordinate m_centerCoord;

   /**
    *   Last screen size.
    */
   MC2Point m_screenSize;
   
   /**
    *   Current scale.
    */
   double m_scale;

   /**
    *   Current angle in degrees.
    */
   double m_angle;
   
};

inline const MC2Point&
MapProjection::getScreenSize() const
{
   return m_screenSize;
}

inline const MC2Coordinate& 
MapProjection::getCenter() const
{
   return m_centerCoord;
}

#endif

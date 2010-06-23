/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapProjection.h"
#include "PixelBox.h"

#define TOP_LAT 912909609
#define BOTTOM_LAT -912909609

MapProjection::
MapProjection() : TransformMatrix(),
                  m_screenSize(100,100)
{
   m_angle = 0.0;
   MC2Coordinate lower( 664609150, 157263143 );
   MC2Coordinate upper( 664689150, 157405144 );
   MC2BoundingBox bbox(lower, upper);
   
   setBoundingBox(bbox);
}

MapProjection::
MapProjection(const MC2Coordinate& centerCoord,
              int screenX, int screenY, double scale,
              double angle) : TransformMatrix(), m_centerCoord(centerCoord),
                              m_screenSize(screenX, screenY),
                              m_scale(scale),
                              m_angle(angle)
{
}

void
MapProjection::updateTransformMatrix()
{
   // Constant forever
   static const double mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   const double invScale = 1.0/m_scale;
   const double mc2scale = mc2scaletometer * invScale;

   TransformMatrix::updateMatrix( m_angle, mc2scale, m_centerCoord,
                                  m_screenSize.getX(),
                                  m_screenSize.getY() );
}

float64
MapProjection::getScaleFromBBoxAndSize( int screenXSize, 
                                        int screenYSize,
                                        MC2BoundingBox& bbox )
{
   static const double mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   // Strech out the bbox to fit the screen size.

   // This is copied from GfxUtility. I don't want that dependency   
   // so I have copied it here. I might want to change it though.

   int32 minLat = bbox.getMinLat();
   int32 minLon = bbox.getMinLon();
   int32 maxLat = bbox.getMaxLat();
   int32 maxLon = bbox.getMaxLon();
   
   int width = screenXSize;
   int height = screenYSize;
   // width and height should have same proportions as 
   // bbox.width and bbox.height
   float64 bboxHeight = bbox.getHeight();
   float64 bboxWidth = bbox.getWidth();
   if ( bboxHeight == 0.0 ) {
      bboxHeight = 1.0;
   }
   if ( bboxWidth == 0.0 ) {
      bboxWidth = 1.0;
   }
   float64 factor = bboxHeight / bboxWidth * width / height;
   if ( factor < 1 ) {
      // Compensate for that the display is higher than the bbox
//      height = uint16( height * factor );
      int32 extraHeight = 
         int32( rint( ( (bboxHeight / factor ) - 
                        bboxHeight ) / 2 ) );
      minLat -= extraHeight;
      maxLat += extraHeight;
      bbox.setMinLat( minLat );
      bbox.setMaxLat( maxLat );
   } else {
      // Compensate for that the display is wider than the bbox
//      width = uint16( width / factor );
      uint32 lonDiff = bbox.getLonDiff();
      if ( lonDiff == 0 ) {
         lonDiff = 1;
      }
      int32 extraWidth = 
         int32( rint( ( (lonDiff * factor ) - 
                        lonDiff ) / 2 ) );
      minLon -= extraWidth;
      maxLon += extraWidth;
      bbox.setMinLon( minLon );
      bbox.setMaxLon( maxLon );
   }

   bbox.setMinLat( minLat );
   bbox.setMaxLat( maxLat );
   bbox.setMinLon( minLon );
   bbox.setMaxLon( maxLon );
   
   float64 scale = 
      double(bbox.getHeight() * mc2scaletometer) /
      screenYSize; // unit meters map / pixel
   
   return scale;   
}

MC2BoundingBox
MapProjection::setBoundingBox(const MC2BoundingBox& inbbox)
{
   MC2BoundingBox bbox(inbbox);
   // Set the scale
   m_scale = getScaleFromBBoxAndSize( m_screenSize.getX(),
                                      m_screenSize.getY(),
                                      bbox );
   // Save the corner
   setCenter( bbox.getCenter() );
   
   return bbox;
}

MC2BoundingBox
MapProjection::getBoundingBox() const
{   
   MC2Coordinate coord1;
   inverseTranformCosLatSupplied( coord1,
                                  m_screenSize.getX(),
                                  0,
                                  getCosLat(getCenter().lat));
   MC2Coordinate coord2;
   inverseTranformCosLatSupplied( coord2,
                                  0,
                                  m_screenSize.getY(),
                                  getCosLat(getCenter().lat));
   MC2Coordinate coord3;
   inverseTranformCosLatSupplied( coord3,
                                  m_screenSize.getX(),
                                  m_screenSize.getY(),
                                  getCosLat(getCenter().lat));
   MC2Coordinate coord4;
   inverseTranformCosLatSupplied( coord4,
                                  0,
                                  0,
                                  getCosLat(getCenter().lat) );
   MC2BoundingBox bbox(coord1, coord2);
   bbox.update(coord3, false);
   bbox.update(coord4);

#ifdef __unix__
   mc2dbg8 << "[TMH]: m_centerCoord = " << m_centerCoord
           << "                                      and center of bbox = "
           << bbox.getCenter() << endl;
   MC2Point corner1(0,0);
   MC2Point corner2(corner1);
   MC2Point corner3(corner2);
   MC2Point corner4(corner3);

   transformPointCosLatSupplied(corner1, coord1, getCosLat(getCenter().lat));
   transformPointCosLatSupplied(corner2, coord2, getCosLat(getCenter().lat));
   transformPointCosLatSupplied(corner3, coord3, getCosLat(getCenter().lat));
   transformPointCosLatSupplied(corner4, coord4, getCosLat(getCenter().lat));
   
   mc2dbg << "[TMH]: Corners are "
          << corner1.getX() << "," << corner1.getY() << " + "
          << corner2.getX() << "," << corner2.getY() << " + "
          << corner3.getX() << "," << corner3.getY() << " + "
          << corner4.getX() << "," << corner4.getY() << " + "
          << endl;
#endif
   return bbox;
}

double
MapProjection::setScale(double scale)
{
   m_scale = scale;
   const double minScale = 0.1;
   const double maxScale = 24000.0;
   if ( m_scale < minScale ) {
      m_scale = minScale;
   } else if ( m_scale > maxScale ) {
      m_scale = maxScale;
   }
   updateTransformMatrix();
   return m_scale;
}

double
MapProjection::getScale() const
{
   return m_scale;
}


double
MapProjection::zoom(double factor)
{
   setScale( factor * getScale() );
   return m_scale;
}

void
MapProjection::setPixelBox( const MC2Point& oneCorner,
                            const MC2Point& otherCorner )
{
   PixelBox pixBox( oneCorner, otherCorner );
   MC2BoundingBox bbox;
   for( int i = 0; i < 4; ++i ) {
      MC2Coordinate tmpCoord;
      inverseTranformUsingCosLat( tmpCoord, pixBox.getCorner(i) );
      bbox.update( tmpCoord );
   }
   double oldangle = m_angle;
   setAngle(0);
   setBoundingBox( bbox );
   setAngle( oldangle );
}

void
MapProjection::move(int deltaX,
                    int deltaY )
{
#if 0
   // Only move one pixel.
   if ( deltaX ) {
      deltaX = deltaX/abs<int>(deltaX);
   }
   if ( deltaY ) {
      deltaY = deltaY/abs<int>(deltaY);
   }
#endif
   // Translate the screen coordinates into lat/lon.
   MC2Coordinate center;
   inverseTranformCosLatSupplied(
      center,
      deltaX + (m_screenSize.getX() >> 1),
      deltaY + (m_screenSize.getY() >> 1),
      getCosLat( m_centerCoord.lat ) );
   setCenter( center );
}

void
MapProjection::setCenter(const MC2Coordinate& newCenter)
{
   m_centerCoord = newCenter;
   if ( m_centerCoord.lat > TOP_LAT ) {
      m_centerCoord.lat = TOP_LAT;
   } else if ( m_centerCoord.lat < BOTTOM_LAT ) {
      m_centerCoord.lat = BOTTOM_LAT;
   }
   updateTransformMatrix();
}

void
MapProjection::setPoint(const MC2Coordinate& newCoord,
                        const MC2Point& screenPoint )
{
   // Translate the center to a screen coord.
   MC2Point centerPoint(0,0);
   transformPointInternalCosLat( centerPoint,
                                 m_centerCoord );

   int deltaX = centerPoint.getX() - screenPoint.getX();
   int deltaY = centerPoint.getY() - screenPoint.getY();

   // Set center
   setCenter( newCoord );
   // Move to the new point.
   move(deltaX, deltaY);  
}

void
MapProjection::setAngle(double angleDeg)
{   
   m_angle = angleDeg;
   updateTransformMatrix(); 
}

void
MapProjection::setAngle(double angleDeg,
                        const MC2Point& rotationPoint )
{
   // Translate the center to a screen coord.
   MC2Point centerPoint(0,0);
   transformPointInternalCosLat( centerPoint,
                                 m_centerCoord );

   int deltaX = centerPoint.getX() - rotationPoint.getX();
   int deltaY = centerPoint.getY() - rotationPoint.getY();

   move(-deltaX, -deltaY);
   setAngle( angleDeg );
   move(deltaX, deltaY);
}

double
MapProjection::getAngle() const
{
   return m_angle;
}

void
MapProjection::setScreenSize(const MC2Point& size)
{
   m_screenSize = size;
}

MC2Coordinate
MapProjection::calcLowerLeft() const
{
   // Constant forever
   static const double mc2scaletometer = 6378137.0*2.0*
      3.14159265358979323846 / 4294967296.0;
   const double invScale = 1.0/m_scale;
   const double mc2scale = mc2scaletometer * invScale;

   const int screenWidth  = m_screenSize.getX();
   const int screenHeight = m_screenSize.getY();
   
   return MC2Coordinate( int32(double(m_centerCoord.lat) -
                               (1/mc2scale * screenHeight * 0.5)),
                         int32(double(m_centerCoord.lon) -
                               (1/mc2scale/getCosLat(m_centerCoord.lat) *
                                screenWidth * 0.5 ) ) );
}

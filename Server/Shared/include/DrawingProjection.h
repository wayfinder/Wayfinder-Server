/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DRAWING_PROJECTION_H
#define DRAWING_PROJECTION_H

#include "config.h"

#include <vector>
#include <set>

#include "MC2BoundingBox.h"
#include "PixelBox.h"
#include "Point.h"
#include "StringTableUTF8.h"

class MC2Point;
class MC2Coordinate;
class Packet;

// Super class for different projections for drawing
class DrawingProjection {

  public:
   
   enum projection_t {
      cosLatProjection = 0,
      braunProjection = 1,
      mercatorProjection = 2,

      nbrProjections
   };

   // Constructor
   DrawingProjection( DrawingProjection::projection_t type,
                      const MC2Point& size );

   //DrawingProjection() { m_projectionType = nbrProjections; }

   // Destructor
   virtual ~DrawingProjection() {}

   // Inits the class
   virtual void init() = 0;

   // Saves the data to a packet   
   virtual void save( Packet* p, int& pos ) const = 0;

   // Reads the data from a packet and creates the object
   static DrawingProjection* create( const Packet* p, int& pos );
   
   // Converts a MC2-coordinate to a point
   virtual MC2Point getPoint( const MC2Coordinate& coord ) const = 0;

   // Converts a global MC2-coordinate to a point
   virtual MC2Point getGlobalPoint( const MC2Coordinate& coord ) const = 0;

   // Gets the pixelbox where the projection is valid
   const PixelBox& getPixelBox() const {
      return m_pixelBox;
   }

   // Converts a MC2-coordinate to a POINT
   POINT getPointFromCoordinate( const MC2Coordinate& coord ) const;
   
   // Converts a point to a MC2-coordinate
   virtual MC2Coordinate getCoordinate( const MC2Point& point ) const = 0;

   // Converts a global point to a MC2-coordinate
   virtual MC2Coordinate
          getGlobalCoordinate( const MC2Point& point ) const = 0;
   
   // Converts a POINT to a MC2-coordinate
   MC2Coordinate getCoordinateFromPoint( const POINT& point ) const;
   
   virtual int32 getLatDiff( int32 y ) const = 0;
   
   virtual int32 getLonDiff( int32 x ) const = 0;

   // Returns the projection type
   DrawingProjection::projection_t getProjectionType() const;

   // Returns the size of the data in bytes
   virtual uint32 getSizeInPacket() const = 0;

   // Returns the bounding box
   virtual const MC2BoundingBox& getBoundingBox() const = 0;

   virtual const MC2BoundingBox& getLargerBoundingBox() const = 0;

   // Returns the scale level
   virtual uint32 getScaleLevel() const = 0;

   // Returns the status
   virtual StringTableUTF8::stringCode getStatus() const;

   // The status
   StringTableUTF8::stringCode m_status;

   bool isJ2meZoom( int zoom ) const {
      return m_j2me_zooms.find( zoom ) != m_j2me_zooms.end();
   }
   
  private:
   // Pixelbox
   PixelBox m_pixelBox;
   // The projection type
   DrawingProjection::projection_t m_projectionType;
protected:
   // Zoom levels for j2me-client
   set<uint32> m_j2me_zooms;
};



// Sub class for Coslat projection
class CosLatProjection : public DrawingProjection {
   
  public:
   // Constructor
   CosLatProjection( const MC2BoundingBox& bbox,
                     int32 height,
                     int32 width,
                     int16 rotationAngle = 0);

   // Destructor
   ~CosLatProjection() {}

   // Init
   void init();
   
   // Saves the data to a packet
   void save( Packet* p, int& pos ) const;

   // Converts a MC2-coordinate to a point
   MC2Point getPoint( const MC2Coordinate& coord ) const;

   // Converts a global MC2-coordinate to a point
   MC2Point getGlobalPoint( const MC2Coordinate& coord ) const;

   // Converts a point to a MC2-coordinate
   MC2Coordinate getCoordinate( const MC2Point& point ) const;

   // Converts a global point to a MC2-coordinate
   MC2Coordinate getGlobalCoordinate( const MC2Point& point ) const;
   
   int32 getLatDiff( int32 y ) const;

   int32 getLonDiff( int32 x ) const;

   // Returns the size of the data in bytes
   uint32 getSizeInPacket() const;

   // Returns the bounding box
   const MC2BoundingBox& getBoundingBox() const;

   const MC2BoundingBox& getLargerBoundingBox() const;

   // Returns the scale level
   uint32 getScaleLevel() const;

   
   
  protected:

   // The bounding box
   MC2BoundingBox m_bbox;

   // The x scale
   float64 m_xscale;

   // The y scale
   float64 m_yscale;

   // The height of the frame
   int32 m_height;

   // The width of the frame
   int32 m_width;

   // The angle for rotation
   int16 m_rotationAngle;

};

// Cylindrical projection

class CylindricalProjection : public DrawingProjection {
   
  public:
   /// The default number of pixels
   static const int NBR_PIXELS;

   // The number of zoom levels
   static const int NBR_ZOOM_LEVELS;
   
   // Constructor
   CylindricalProjection( int xPixel,
                          int yPixel,
                          int zoomLevel,
                          int nbrPixels,
                          DrawingProjection::projection_t type );

   // Destructor
   ~CylindricalProjection() { }

   // Returns the number of zoom levels
   int getNbrZoomlevels() const;
   
   //  Returns the matrix with the nbr of squares for each zoom level
   vector< pair<int, int> > getSquareSizes() const;

   // Returns the bounding box
   const MC2BoundingBox& getBoundingBox() const;

   const MC2BoundingBox& getLargerBoundingBox() const;

   /**
    * @return true if the \c pixelSize is valid.
    */
   static bool isValidPixelSize( uint32 pixelSize );

  protected:

   // Create the square
   void createSquare();
   
   // Create the bounding box
   void createBoundingBox();

   // Matrix with sizes for the squares
   vector< pair<int, int> > m_squareSizes;
   
   // nbr of squares in latitude
   int m_latSquares;

   // nbr of squares in longitude
   int m_lonSquares;

   // The x coordinate for the requested square
   int m_xPixel;

   // The y coordinate for the requested square
   int m_yPixel;

   // The zoom level
   int m_zoomLevel;

   // The nbr of pixels for the picture
   int m_nbrPixels;

   // The radius
   double m_radius;

   // The bounding box
   MC2BoundingBox m_bbox;

   MC2BoundingBox m_largerBBox;
};


// Brauns's cylindrical projection
class BraunProjection : public CylindricalProjection {
  public:
   // Constructor
   BraunProjection( int xPixel,
                    int yPixel,
                    int zoomLevel,
                    int nbrPixels );

   // Destructor
   ~BraunProjection() {}

   // Init
   void init();
   
   // Saves the data to a packet
   void save( Packet* p, int& pos ) const;

   // Converts a MC2Coordinate to a point
   MC2Point getPoint( const MC2Coordinate& coord ) const;

   // Converts a global MC2-coordinate to a point
   MC2Point getGlobalPoint( const MC2Coordinate& coord ) const;

   // Converts a point to a MC2-coordinate
   MC2Coordinate getCoordinate( const MC2Point& point ) const;

   // Converts a global point to a MC2-coordinate
   MC2Coordinate getGlobalCoordinate( const MC2Point& point ) const;

   int32 getLatDiff( int32 y ) const;

   int32 getLonDiff( int32 x ) const;

   // Returns the size of the data in bytes
   uint32 getSizeInPacket() const;

   // Returns the scale level
   uint32 getScaleLevel() const;
};

// Mercator projection
class MercatorProjection : public CylindricalProjection {

  public:
   // Constructor
   MercatorProjection( int xPixel,
                       int yPixel,
                       int zoomLevel,
                       int nbrPixels );

   // Destructor
   ~MercatorProjection() {}

   // Init
   void init();

   // Saves the data to a packet
   void save( Packet* p, int& pos ) const;

   // Converts a MC2Coordinate to a point
   MC2Point getPoint( const MC2Coordinate& coord ) const;

   // Converts a global MC2Coordinate to a point
   MC2Point getGlobalPoint( const MC2Coordinate& coord ) const;
   
   // Converts a point to a MC2-coordinate
   MC2Coordinate getCoordinate( const MC2Point& point ) const;

   // Converts a global point to a MC2-coordinate
   MC2Coordinate getGlobalCoordinate( const MC2Point& point ) const;

   int32 getLatDiff( int y ) const;

   int32 getLonDiff( int x ) const;

   // Returns the size of the data in bytes
   uint32 getSizeInPacket() const;

   // Returns the scale level
   uint32 getScaleLevel() const;
};

#endif

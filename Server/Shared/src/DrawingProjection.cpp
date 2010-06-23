/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DrawingProjection.h"
#include "MC2Point.h"
#include "MC2Coordinate.h"
#include "Packet.h"
#include "GfxConstants.h"
#include "CoordinateTransformer.h"
#include "MapUtility.h"

// Drawing projection
DrawingProjection::DrawingProjection( DrawingProjection::projection_t type,
                                      const MC2Point& size )
      : m_pixelBox( MC2Point(0,0), size ),
        m_projectionType(type)
        
{
}

DrawingProjection*
DrawingProjection::create( const Packet* p, int& pos )
{
   DrawingProjection* result = NULL;
   
   DrawingProjection::projection_t type =
      (DrawingProjection::projection_t) p->incReadLong( pos );
   switch( type ) {
      case DrawingProjection::cosLatProjection : {
         MC2BoundingBox bbox;
         p->incReadBBox( pos, bbox );
         int32 height = p->incReadLong( pos );
         int32 width = p->incReadLong( pos );
         int16 rotationAngle = p->incReadShort( pos );

         result = new CosLatProjection( bbox,
                                        height,
                                        width,
                                        rotationAngle );
         result->init();
      }
         break;
      case DrawingProjection::braunProjection : {
         MC2BoundingBox bbox;
         p->incReadBBox( pos, bbox );
         int32 latIndex = p->incReadLong( pos );
         int32 lonIndex = p->incReadLong( pos );
         int32 zoomLevel = p->incReadLong( pos );
         int32 nbrPixels = p->incReadLong( pos );
         
         result = new BraunProjection( latIndex,
                                       lonIndex,
                                       zoomLevel,
                                       nbrPixels );
         result->init();
      }
         break;
      case DrawingProjection::mercatorProjection : {
         MC2BoundingBox bbox;
         p->incReadBBox( pos, bbox );
         int32 latIndex = p->incReadLong( pos );
         int32 lonIndex = p->incReadLong( pos );
         int32 zoomLevel = p->incReadLong( pos );
         int32 nbrPixels = p->incReadLong( pos );

         result = new MercatorProjection( latIndex,
                                          lonIndex,
                                          zoomLevel,
                                          nbrPixels );
         result->init();
      }
         break;
      default:
         mc2log << warn << "DrawingProjection::create() " 
                << "Unknown projection type." << endl;
         exit(1);
         
         break;
   }
   return result;
}

POINT
DrawingProjection::getPointFromCoordinate( const MC2Coordinate& coord ) const
{
   MC2Point p = getPoint( coord );
   POINT returnPoint = { p.getX(), p.getY()};

   return returnPoint;
}

MC2Coordinate
DrawingProjection::getCoordinateFromPoint( const POINT& point ) const
{
   MC2Point p = MC2Point( point.x, point.y );
   return getCoordinate( p );
}

DrawingProjection::projection_t
DrawingProjection::getProjectionType() const
{
   return m_projectionType;
}

StringTableUTF8::stringCode
DrawingProjection::getStatus() const
{
   return m_status;
}


// CosLat projection
CosLatProjection::CosLatProjection( const MC2BoundingBox& bbox,
                                    int32 height,
                                    int32 width,
                                    int16 rotationAngle )
      : DrawingProjection( DrawingProjection::cosLatProjection,
                           MC2Point( width, height ) )
{
   m_bbox.setMaxLat( bbox.getMaxLat() );
   m_bbox.setMaxLon( bbox.getMaxLon() );
   m_bbox.setMinLat( bbox.getMinLat() );
   m_bbox.setMinLon( bbox.getMinLon() );
   m_height = height;
   m_width = width;
   m_rotationAngle = rotationAngle;
   m_bbox.updateCosLat();
   init();
}

void
CosLatProjection::init()
{
   m_xscale = float64(m_width - 1) / m_bbox.getLonDiff();
   m_yscale = float64(m_height - 1) / m_bbox.getHeight();   
   
   // Make sure that the image will not have strange proportions.
   float64 factor = float64(m_bbox.getHeight()) / m_bbox.getWidth()
      * (m_width - 1) / (m_height - 1);
   
   if (factor < 1) {
      //  Compensate for that the image is wider than it is high
      m_yscale *= factor;
   } else {
      // Compensate for that the image is higher than it is wide
      m_xscale /= factor;
   }
   m_status = StringTableUTF8::OK;
}

void
CosLatProjection::save( Packet* p, int& pos ) const
{
   // Write the projection type
   p->incWriteLong( pos, DrawingProjection::cosLatProjection );

   // Write the bounding box
   p->incWriteBBox( pos, m_bbox );
   
   // Write the height and width
   p->incWriteLong( pos, m_height );
   p->incWriteLong( pos, m_width );
   
   // Write the map rotation
   p->incWriteShort( pos, m_rotationAngle );
}


MC2Point
CosLatProjection::getPoint( const MC2Coordinate& coord ) const
{
   int32 unrotated_xcoord, unrotated_ycoord;
   // xcoord
   unrotated_xcoord =
      int32(rint((coord.lon - m_bbox.getMinLon()) * m_xscale));
   if (unrotated_xcoord > (m_width*2))
      unrotated_xcoord = m_width*2;
   else if (unrotated_xcoord < (-m_width*2))
      unrotated_xcoord = -m_width*2 ;      
   // ycoord
   unrotated_ycoord = int32(m_height) - 1 - 
      int32(rint((coord.lat - m_bbox.getMinLat()) * m_yscale));
   if (unrotated_ycoord > (m_height*2))
      unrotated_ycoord = m_height*2;
   else if (unrotated_ycoord < (-m_height*2)) 
      unrotated_ycoord = -m_height*2;     
   
   int32 xcoord, ycoord;
   if( m_rotationAngle == 0 ) {
      if ( coord.lat == m_bbox.getMinLat() ) {
         ycoord = m_height;
      } else {
         ycoord = unrotated_ycoord;
      }
      if ( coord.lon == m_bbox.getMaxLon() ) {
         xcoord = m_width;
      } else {
         xcoord = unrotated_xcoord;
      }

   } else {
      float64 angleRad = (m_rotationAngle / 180.0) * M_PI;
      
      int32 halfImageWidth = m_width / 2;
      int32 halfImageHeight = m_height / 2;
      
      // Translate origo to the center.
      int32 translatedImageX = unrotated_xcoord - halfImageWidth;
      int32 translatedImageY = unrotated_ycoord - halfImageHeight;

      float64 sinAngle = sin(angleRad);
      float64 cosAngle = cos(angleRad);
      
      // Rotate around origo. (clockwise, i.e. negative direction)
      float64 rotatedImageX =
        cosAngle * translatedImageX
        + sinAngle * translatedImageY;
      
      float64 rotatedImageY = 
        - sinAngle * translatedImageX
        + cosAngle * translatedImageY;
      
   
      // Translate back.
      xcoord = static_cast<int32>(floor(0.5
                                        + rotatedImageX
                                        + halfImageWidth ));
      ycoord = static_cast<int32>(floor(0.5
                                        + rotatedImageY
                                        + halfImageHeight ));
   }
   return MC2Point( xcoord, ycoord );
}

MC2Point
CosLatProjection::getGlobalPoint( const MC2Coordinate& coord ) const
{
   // See formula in getGlobalCoordinate

   int32 xcoord, ycoord;

   if( m_rotationAngle == 0 ) {
      // xcoord
      xcoord = int32(rint(coord.lon * m_xscale));

      // ycoord
      ycoord = int32( rint( m_height - 1 - coord.lat * m_yscale ) );
   } else {
      xcoord = 0;
      ycoord = 0;
   }
   return MC2Point( xcoord, ycoord );
}

MC2Coordinate
CosLatProjection::getGlobalCoordinate( const MC2Point& point ) const
{
   /*
    * The following formula is used to convert from point to lat/lon and vice
    * versa.
    * For lat <-> y: 
    *    lat = (height - 1 - Y ) / yscale)
    *    y = height - 1 - lat * yscale
    * For lon <-> x:
    *    lon = x / m_xscale
    *    x = lon * m_xscale
    */
   int32 lat, lon;

   lat = int32( rint(( ((m_height - 1 - point.getY()) / m_yscale) ) ) );
   lon = int32( rint( ( (point.getX() / m_xscale) ) ) );

   return MC2Coordinate( lat, lon );
}

MC2Coordinate
CosLatProjection::getCoordinate( const MC2Point& point ) const
{
   int32 lat, lon;

   lat = int32( rint(
            ( ((m_height - 1 - point.getY()) / m_yscale) +
              m_bbox.getMinLat() ) ) );
   lon = int32( rint( ( (point.getX() / m_xscale) + m_bbox.getMinLon() ) ) );

   return MC2Coordinate( lat, lon );
}



int32
CosLatProjection::getLatDiff( int32 y ) const
{
   return int32( rint( y / m_yscale ) );
}

int32
CosLatProjection::getLonDiff( int32 x ) const
{
   return int32( rint( x / m_xscale ) );
}

uint32
CosLatProjection::getSizeInPacket() const
{
   return 4 + 16 + 16 + 4 + 4 + 4;
}

const MC2BoundingBox&
CosLatProjection::getBoundingBox() const
{
   return m_bbox;
}

const MC2BoundingBox&
CosLatProjection::getLargerBoundingBox() const
{
   return m_bbox;
}

uint32
CosLatProjection::getScaleLevel() const
{
   return MapUtility::getScaleLevel( m_bbox,
                                     m_width,
                                     m_height );
}

// Cylindrical projection

const int CylindricalProjection::NBR_PIXELS = 180;
const int CylindricalProjection::NBR_ZOOM_LEVELS = 15;

CylindricalProjection::CylindricalProjection(
   int xPixel,
   int yPixel,
   int zoomLevel,
   int nbrPixels,
   DrawingProjection::projection_t type )
   : DrawingProjection( type, MC2Point(nbrPixels,nbrPixels) ),
     m_latSquares( 0 ),
     m_lonSquares( 0 ),
     m_xPixel( xPixel ),
     m_yPixel( yPixel ),
     // set range 0 to max zoom levels
     m_zoomLevel( zoomLevel ),
     m_nbrPixels( nbrPixels ),
     m_radius( 0.0 )
{

   // Set nbr squares in latitude and longitude
   const int factor = 10;
   for (int i = 0; i < NBR_ZOOM_LEVELS; ++i) {
      int n = int(factor * pow( double(2), i ) );
      m_squareSizes.push_back( pair<int, int>( n, n ) );
   }
}

int
CylindricalProjection::getNbrZoomlevels() const
{
   return m_squareSizes.size();
}

vector< pair<int, int> >
CylindricalProjection::getSquareSizes() const
{
   return m_squareSizes;
}

void
CylindricalProjection::createSquare() 
{
   // Set the status
   
   if( m_zoomLevel < 1 || m_zoomLevel > NBR_ZOOM_LEVELS ) {
      m_status = StringTableUTF8::NOTOK;
   } else { 
      m_latSquares = m_squareSizes[m_zoomLevel-1].first;
      m_lonSquares = m_squareSizes[m_zoomLevel-1].second;
      m_radius = double(m_lonSquares * m_nbrPixels) / 2 / M_PI;

      if( (m_xPixel % m_nbrPixels) != 0 ) {
         m_status = StringTableUTF8::NOTOK;
      } else if( (m_yPixel % m_nbrPixels) != 0 ) {
         m_status = StringTableUTF8::NOTOK;
      } else if( m_xPixel < (-m_lonSquares*m_nbrPixels/2) ) {
         m_status = StringTableUTF8::NOTOK;
      } else if( m_xPixel > (m_lonSquares*m_nbrPixels/2-m_nbrPixels) ) {
         m_status = StringTableUTF8::NOTOK;
      } else if( m_yPixel < (-m_latSquares*m_nbrPixels/2) ) {
         m_status = StringTableUTF8::NOTOK;
      } else if( m_yPixel > (m_latSquares*m_nbrPixels/2-m_nbrPixels) ) {
         m_status = StringTableUTF8::NOTOK;
      } else {
         m_status = StringTableUTF8::OK;
      }
   }
}

bool CylindricalProjection::isValidPixelSize( uint32 pixelSize ) {
   // Even and larger than 31
   if ( pixelSize % 2 == 0 &&
        pixelSize >= 32 ) {
      return true;
   }
   return false;
}

void
CylindricalProjection::createBoundingBox()
{
   int xIndex =
      int( ( m_xPixel + m_lonSquares * m_nbrPixels / 2 )
           / m_nbrPixels );
   int yIndex =
      int( ( -(m_yPixel+m_nbrPixels)
             + m_latSquares * m_nbrPixels / 2 )  / m_nbrPixels);
   
   // Lower left coordinate
   int lowerXPixel =
      ( -m_lonSquares * m_nbrPixels / 2 ) + m_nbrPixels * xIndex;
   int lowerYPixel =
      int( m_latSquares * m_nbrPixels / 2 ) - m_nbrPixels * (yIndex+1);
   MC2Point lowerPoint( lowerXPixel, lowerYPixel );
   MC2Coordinate lowerCoord = getGlobalCoordinate( lowerPoint );
               
   // Upper right coordinate
   int upperXPixel =
      ( -m_lonSquares * m_nbrPixels / 2 ) + m_nbrPixels * ( xIndex + 1 );
   int upperYPixel =
      int( m_latSquares * m_nbrPixels / 2 ) - m_nbrPixels * yIndex;

   MC2Point upperPoint( upperXPixel, upperYPixel );
   MC2Coordinate upperCoord = getGlobalCoordinate( upperPoint );

   m_bbox.setMaxLat( upperCoord.lat );
   m_bbox.setMaxLon( upperCoord.lon );
   m_bbox.setMinLat( lowerCoord.lat );
   m_bbox.setMinLon( lowerCoord.lon );
   
   m_bbox.updateCosLat();

   m_largerBBox.setMaxLat(
      upperCoord.lat + getLatDiff( m_nbrPixels / 2 ) );
   m_largerBBox.setMaxLon(
      upperCoord.lon + getLonDiff( m_nbrPixels / 2 ) );
   m_largerBBox.setMinLat(
      lowerCoord.lat - getLatDiff( m_nbrPixels / 2 ) );
   m_largerBBox.setMinLon(
      lowerCoord.lon - getLonDiff( m_nbrPixels / 2 ) );

   m_largerBBox.updateCosLat();
}

const MC2BoundingBox&
CylindricalProjection::getBoundingBox() const
{
   return m_bbox;
}

const MC2BoundingBox&
CylindricalProjection::getLargerBoundingBox() const
{
   return m_largerBBox;
}

// Braun's cylindrical projection

BraunProjection::BraunProjection( int xPixel,
                                  int yPixel,
                                  int zoomLevel,
                                  int nbrPixels )
      : CylindricalProjection( xPixel,
                               yPixel,
                               zoomLevel,
                               nbrPixels,
                               DrawingProjection::braunProjection )
{
   init();
}

void
BraunProjection::init()
{
   createSquare();
   createBoundingBox();   
}

void
BraunProjection::save( Packet* p, int& pos ) const
{
   // Write the projection type
   p->incWriteLong( pos, DrawingProjection::braunProjection );

   // Write the bounding box
   p->incWriteBBox( pos, m_bbox );

   // Write the x pixel
   p->incWriteLong( pos, m_xPixel );

   // Write the y pixel
   p->incWriteLong( pos, m_yPixel );

   // Write the zoom level
   p->incWriteLong( pos, m_zoomLevel );

   // Write the nbr of pixels
   p->incWriteLong( pos, m_nbrPixels );
}

MC2Point
BraunProjection::getPoint( const MC2Coordinate& coord ) const
{
   double lat = double( coord.lat * GfxConstants::invRadianFactor );
   double lon = double( coord.lon * GfxConstants::invRadianFactor );

   double x = double( lon * m_radius );
   double y = double( 2.0 * m_radius * tan( double( lat / 2 ) ) );
   
   double minLat = double(m_bbox.getMinLat() * GfxConstants::invRadianFactor);
   double minLon = double(m_bbox.getMinLon() * GfxConstants::invRadianFactor);
      
   double minX = minLon * m_radius;
   double minY = 2.0 * m_radius * tan( double( minLat / 2 ) );

   int32 xResult = int32( rint( x - minX ) );
   int32 yResult = int32( rint( y - minY ) );
   
   return MC2Point( xResult, m_nbrPixels - yResult );
}

MC2Point
BraunProjection::getGlobalPoint( const MC2Coordinate& coord ) const
{
   double lat = double( coord.lat * GfxConstants::invRadianFactor );
   double lon = double( coord.lon * GfxConstants::invRadianFactor );
   
   double x = lon * m_radius;
   double y = 2.0 * m_radius * tan( double( lat / 2 ) );

   int32 xResult = int32( rint( x ) );
   int32 yResult = int32( rint( y ) );
   
   return MC2Point( xResult, yResult );
}

MC2Coordinate
BraunProjection::getCoordinate( const MC2Point& point ) const
{
   double lat = 2 * atan( double(point.getY()) / 2 / m_radius );
   double lon = double( point.getX() ) / m_radius;

   double latMC2 = double( lat * GfxConstants::radianFactor );
   double lonMC2 = double( lon * GfxConstants::radianFactor );

   int32 latResult = int32( rint( latMC2 - double( m_bbox.getMinLat() ) ) );
   int32 lonResult = int32( rint( lonMC2 - double( m_bbox.getMinLon() ) ) );
   
   return MC2Coordinate( latResult, lonResult );
}

MC2Coordinate
BraunProjection::getGlobalCoordinate( const MC2Point& point ) const
{
   double lat = 2 * atan( double(point.getY()) / 2 / m_radius );
   double lon = double( point.getX() ) / m_radius;

   int32 latMC2 = int32( rint( lat * GfxConstants::radianFactor ) );
   int32 lonMC2 = int32( rint( lon * GfxConstants::radianFactor ) );
   
   return MC2Coordinate( latMC2, lonMC2 );
}

int32
BraunProjection::getLatDiff( int32 y ) const
{
   int diffMax = 0;
   MC2Coordinate minCoord = MC2Coordinate( m_bbox.getMinLat(),
                                           m_bbox.getMinLon() );
   MC2Point minPoint = getGlobalPoint( minCoord );
   MC2Point newMinPoint = MC2Point( minPoint.getX() - y,
                                    minPoint.getY() - y );
   MC2Coordinate newMinCoord = getGlobalCoordinate( newMinPoint );

   MC2Coordinate maxCoord = MC2Coordinate( m_bbox.getMaxLat(),
                                           m_bbox.getMaxLon() );
   MC2Point maxPoint = getGlobalPoint( maxCoord );
   MC2Point newMaxPoint = MC2Point( maxPoint.getX() + y,
                                    maxPoint.getY() + y );
   MC2Coordinate newMaxCoord = getGlobalCoordinate( newMaxPoint );
   
   if( (minCoord.lat - newMinCoord.lat) > diffMax ) {
      diffMax = minCoord.lat - newMinCoord.lat;
   }  else if( (newMaxCoord.lat - maxCoord.lat) > diffMax ) {
      diffMax = newMaxCoord.lat - maxCoord.lat;
   } 
  
   return diffMax;
}

int32
BraunProjection::getLonDiff( int32 x ) const
{
   int diffMax = 0;
   
   MC2Coordinate minCoord = MC2Coordinate( m_bbox.getMinLat(),
                                           m_bbox.getMinLon() );
   MC2Point minPoint = getGlobalPoint( minCoord );
   MC2Point newMinPoint = MC2Point( minPoint.getX() - x,
                                    minPoint.getY() - x );
   MC2Coordinate newMinCoord = getGlobalCoordinate( newMinPoint );

   MC2Coordinate maxCoord = MC2Coordinate( m_bbox.getMaxLat(),
                                           m_bbox.getMaxLon() );
   MC2Point maxPoint = getGlobalPoint( maxCoord );
   MC2Point newMaxPoint = MC2Point( maxPoint.getX() + x,
                                    maxPoint.getY() + x );
   MC2Coordinate newMaxCoord = getGlobalCoordinate( newMaxPoint );
   
   if( (minCoord.lon - newMinCoord.lon) > diffMax ) {
      diffMax = minCoord.lon - newMinCoord.lon;
   }  else if( (newMaxCoord.lon - maxCoord.lon) > diffMax ) {
      diffMax = newMaxCoord.lon - maxCoord.lon;
   }
  
   return diffMax;
}

uint32 
BraunProjection::getSizeInPacket() const
{
   return 4 + 16 + 4 + 4 + 4 + 4;
}

uint32
BraunProjection::getScaleLevel() const
{
   switch( m_zoomLevel ) {
      case 1:
         return 0;
      break;
      case 2:
         return 1;
      break;
      case 3:
         return 1;
      break;
      case 4:
         return 2;
      break;
      case 5:
         return 2;
      break;
      case 6:
         return 2;
      break;
      case 7:
         return 2;
      break;
      case 8:
         return 3;
      break;
      case 9:
         return 3;
      break;
      case 10:
         return 4;
      break;
      case 11:
         return 5;
      break;
      case 12:
         return 6;
      break;
      case 13:
         return 7;
      break;
      case 14:
         return 8;
      break;
      case 15:
         return 9;
      break;
      default:
         return 10;
         break;
   }
   
   // Should not happen
   return MAX_UINT32;
}
   
// Mercator projection
MercatorProjection::MercatorProjection( int xPixel,
                                        int yPixel,
                                        int zoomLevel,
                                        int nbrPixels )
      : CylindricalProjection( xPixel,
                               yPixel,
                               zoomLevel,
                               nbrPixels,
                               DrawingProjection::mercatorProjection )
{
   init();
}

void
MercatorProjection::init()
{
   createSquare();
   createBoundingBox();
   // Init the zoom levels used by j2me-client.
   uint32 j2meZooms [] = { 4, 7, 9, 11, 13, 15 };
   m_j2me_zooms.insert( &j2meZooms[0],
                        &j2meZooms[0] +
                        sizeof(j2meZooms)/sizeof(j2meZooms[0]));
}

void
MercatorProjection::save( Packet* p, int& pos ) const
{
   // Write the projection type
   p->incWriteLong( pos, DrawingProjection::mercatorProjection );

   // Write the bounding box
   p->incWriteBBox( pos, m_bbox );

   // Write the x pixel
   p->incWriteLong( pos, m_xPixel );

   // Write the y pixel
   p->incWriteLong( pos, m_yPixel );
   
   // Write the zoom level
   p->incWriteLong( pos, m_zoomLevel );

   // Write the nbr of pixels
   p->incWriteLong( pos, m_nbrPixels );
}

MC2Point
MercatorProjection::getPoint( const MC2Coordinate& coord ) const
{
   double lat = double(coord.lat * GfxConstants::invRadianFactor);
   double lon = double(coord.lon * GfxConstants::invRadianFactor);
   
   double x = m_radius * lon;
   double y =
      double(m_radius) * log( tan( lat ) + double( 1.0 / cos( lat ) ) );
  
   double minLat = double(m_bbox.getMinLat() * GfxConstants::invRadianFactor);
   double minLon = double(m_bbox.getMinLon() * GfxConstants::invRadianFactor);

   double minX = m_radius * minLon;
   double minY =
      m_radius * log( tan( minLat ) + double( 1.0 / cos( minLat ) ) );
   
   int32 xResult = int32( rint( x - minX ) );
   int32 yResult = int32( rint( y - minY ) );
   
   return MC2Point( xResult, m_nbrPixels - yResult );
}

MC2Point
MercatorProjection::getGlobalPoint( const MC2Coordinate& coord ) const
{
   double lat = double(coord.lat * GfxConstants::invRadianFactor);
   double lon = double(coord.lon * GfxConstants::invRadianFactor);

   double x = m_radius * lon;
   double y = m_radius * log( tan( lat ) + double( 1.0 / cos( lat ) ) );

   int32 xResult = int32( rint( x ) );
   int32 yResult = int32( rint( y ) );
     
   return MC2Point( xResult, yResult );
}

MC2Coordinate
MercatorProjection::getCoordinate( const MC2Point& point ) const
{
   double lat = atan( sinh( double(point.getY() / m_radius ) ) );
   double lon = double( point.getX() ) / m_radius;

   double latMC2 = double( lat * GfxConstants::radianFactor );
   double lonMC2 = double( lon * GfxConstants::radianFactor );
   
   int32 latResult = int32( rint( latMC2 - double(m_bbox.getMinLat() ) ) );
   int32 lonResult = int32( rint( lonMC2 - double(m_bbox.getMinLon() ) ) );
   
   return MC2Coordinate( latResult, lonResult );
}

MC2Coordinate
MercatorProjection::getGlobalCoordinate( const MC2Point& point ) const
{
   double lat = atan( sinh( double(point.getY() / m_radius ) ) );
   double lon = double( point.getX() ) / m_radius;

   int32 latMC2, lonMC2;
   latMC2 = int32( rint( lat * GfxConstants::radianFactor ) );
   lonMC2 = int32( rint( lon * GfxConstants::radianFactor ) );
      
   return MC2Coordinate( latMC2, lonMC2 );   
}

int32
MercatorProjection::getLatDiff( int32 y ) const
{
   int diffMax = 0;
   MC2Coordinate minCoord = MC2Coordinate( m_bbox.getMinLat(),
                                           m_bbox.getMinLon() );
   MC2Point minPoint = getGlobalPoint( minCoord );
   MC2Point newMinPoint = MC2Point( minPoint.getX() - y,
                                    minPoint.getY() - y );
   MC2Coordinate newMinCoord = getGlobalCoordinate( newMinPoint );

   MC2Coordinate maxCoord = MC2Coordinate( m_bbox.getMaxLat(),
                                           m_bbox.getMaxLon() );
   MC2Point maxPoint = getGlobalPoint( maxCoord );
   MC2Point newMaxPoint = MC2Point( maxPoint.getX() + y,
                                    maxPoint.getY() + y );
   MC2Coordinate newMaxCoord = getGlobalCoordinate( newMaxPoint );
   
   if( (minCoord.lat - newMinCoord.lat) > diffMax ) {
      diffMax = minCoord.lat - newMinCoord.lat;
   }  else if( (newMaxCoord.lat - maxCoord.lat) > diffMax ) {
      diffMax = newMaxCoord.lat - maxCoord.lat;
   } 
  
   return diffMax;
}

int32
MercatorProjection::getLonDiff( int32 x ) const
{
   int diffMax = 0;
   
   MC2Coordinate minCoord = MC2Coordinate( m_bbox.getMinLat(),
                                           m_bbox.getMinLon() );
   MC2Point minPoint = getGlobalPoint( minCoord );
   MC2Point newMinPoint = MC2Point( minPoint.getX() - x,
                                    minPoint.getY() - x );
   MC2Coordinate newMinCoord = getGlobalCoordinate( newMinPoint );

   MC2Coordinate maxCoord = MC2Coordinate( m_bbox.getMaxLat(),
                                           m_bbox.getMaxLon() );
   MC2Point maxPoint = getGlobalPoint( maxCoord );
   MC2Point newMaxPoint = MC2Point( maxPoint.getX() + x,
                                    maxPoint.getY() + x );
   MC2Coordinate newMaxCoord = getGlobalCoordinate( newMaxPoint );
   
   if( (minCoord.lon - newMinCoord.lon) > diffMax ) {
      diffMax = minCoord.lon - newMinCoord.lon;
   }  else if( (newMaxCoord.lon - maxCoord.lon) > diffMax ) {
      diffMax = newMaxCoord.lon - maxCoord.lon;
   }
  
   return diffMax;
}

uint32 
MercatorProjection::getSizeInPacket() const
{
   return 4 + 16 + 4 + 4 + 4 + 4;
}

uint32
MercatorProjection::getScaleLevel() const
{
   switch( m_zoomLevel ) {
      case 1:
         return 0;
      break;
      case 2:
         return 0;
      break;
      case 3:
         return 1;
      break;
      case 4:
         return 2;
      break;
      case 5:
         return 2;
      break;
      case 6:
         return 3;
      break;
      case 7:
         return 3;
      break;
      case 8:
         return 4;
      break;
      case 9:
         return 5;
      break;
      case 10:
         return 5;
      break;
      case 11:
         return 6;
      break;
      case 12:
         return 7;
      break;
      case 13:
         return 8;
      break;
      case 14:
         return 9;
      break;
      case 15:
         return 10;
      break;
      default:
         return 10;
         break;
   }
      
   // Should not happen
   return MAX_UINT32;
}

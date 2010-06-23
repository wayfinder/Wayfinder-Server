/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2COORDINATE_H
#define MC2COORDINATE_H

#include "config.h"

// Forwards
class MC2Coordinate;
class WGS84Coordinate;

/**
 *    The coordinates used in nav2.
 *    Convert to MC2Coordinate if you want to do something useful.
 */
class Nav2Coordinate {
  public:
   Nav2Coordinate(int32 nav2LatVal, int32 nav2LonVal) :
      nav2lat(nav2LatVal),
      nav2lon(nav2LonVal) {}

   explicit Nav2Coordinate( const MC2Coordinate& realCoord );
   
   int32 nav2lat;
   int32 nav2lon;      
};

/**
 *    An MC2 coordinate containing a latitude and longitude.
 */
class MC2Coordinate {
public:
   /**
    *   Empty constructor sets lat and lon to MAX_INT32
    */
   MC2Coordinate()
         : lat(MAX_INT32), lon(MAX_INT32) {
   }

   explicit MC2Coordinate(const Nav2Coordinate& nav2coord );

   /**
    * Create an MC2Coordinate from a WGS84Coordinate.
    */
   explicit MC2Coordinate( const WGS84Coordinate& wgsCoord );

   /**
    * Constructor sets lat and lon.
    *
    * @param latValue The value of latitude that this MC2Coordinate should
    *                 have.
    * @param lonValue The value of longitude that this MC2Coordinate should
    *                 have.
    */
   MC2Coordinate( int32 latValue, int32 lonValue )
         : lat(latValue), lon(lonValue) {
   }

   /**
    * The latitude of the coordinate.
    */
   int32 lat;

   /**
    * The longitude of the coordinate.
    */
   int32 lon;

   /**
    * If the coordinate is valid.
    *
    * @return True if the coordinate is valid.
    */
   bool isValid() const {
      return ( lat <= MAX_INT32/2 && lat >= int32(MIN_INT32)/2 );
   }

   /**
    * @name Operators.
    * @memo Comparison operators.
    * @doc  To make it possible to sort and search MC2Coordinates.
    */
   //@{
      /** 
       * Equal operator.
       *
       * @param other The MC2Coordinate to compare too.
       * @return If *this and other are equal.
       */
      bool operator== ( const MC2Coordinate& other ) const {
         return (lat == other.lat && lon == other.lon);
      }

      /** 
       * Not equal operator.
       *
       * @param other The MC2Coordinate to compare too.
       * @return If *this and other are not equal.
       */
      bool operator!= ( const MC2Coordinate& other ) const {
         return (lat != other.lat || lon != other.lon);
      }

      /** 
       * Greater than operator.
       *
       * @param other The MC2Coordinate to compare too.
       * @return If *this is greater than other.
       */
      bool operator> ( const MC2Coordinate& other ) const {
         if ( lat > other.lat ) {
            return true;
         } else if ( lat == other.lat ) {
            return (lon > other.lon);
         } else {
            return false;
         }
      }

      /** 
       * Less than operator.
       *
       * @param other The MC2Coordinate to compare too.
       * @return If *this is less than other.
       */
      bool operator< ( const MC2Coordinate& other ) const {
         if ( lat < other.lat ) {
            return true;
         } else if ( lat == other.lat ) {
            return (lon < other.lon);
         } else {
            return false;
         }
      }
   //@}

   /**
    * Print on ostream.
    *
    * @param stream The stream to print on.
    * @param coord  The coordinate to print.
    * @return The stream.
    */
   friend ostream& operator<< ( ostream& stream,
                                const MC2Coordinate& coord )
   {
      return stream << "(" << coord.lat << "," << coord.lon << ")";
   }

   /**
    * The invalid coordinate. NB! not all invalid coordinates are exactly
    * equal to this, operator== might return false even if the other 
    * coordinate is invalid too.
    */
   static const MC2Coordinate invalidCoordinate;
   
public:
   
   /**
    *   Class to be used when putting MC2Coordinates in e.g.
    *   ClipUtil.h, InsideUtil etc.
    */
   class XYHelper {
   public:
      /// Returns the x value for the coordinate
      int32 getX( const MC2Coordinate& coord ) const {
         return coord.lon;
      }
      /// Returns the y value for the coordinate
      int32 getY( const MC2Coordinate& coord ) const {
         return coord.lat;
      }
      /// Returns the x value for the coordinate
      int32 getX( const MC2Coordinate* coord ) const {
         return coord->lon;
      }
      /// Returns the y value for the coordinate
      int32 getY( const MC2Coordinate* coord ) const {
         return coord->lat;
      }
   };
protected:
   /// Constructor that does nothing. Used in subclass TileMapCoord.
   MC2Coordinate( bool ) {}
};

// More XYHelpers
namespace GfxUtil {
// For MC2Coordinate
inline int32 getCoordX( const MC2Coordinate& coord ) {
   return coord.lon;
}

inline int32 getCoordY( const MC2Coordinate& coord ) {
   return coord.lat;
}

}

#endif // MC2COORDINATE_H


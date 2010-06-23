/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXPOLYGON_H
#define GFXPOLYGON_H

#include "config.h"
#include <vector>
#include "GfxCoordinates.h"
#include "MC2Coordinate.h"

class DataBuffer;

/**
 *    A polygon that describes one feature in the vector map.
 *
 */
class GfxPolygon {
public:   
      /** 
       * Constructor
       */
      GfxPolygon( bool coordinates16Bits, uint32 startSize = 16);


      /** 
       * Constructor, used when reading from databuffer.
       */
      GfxPolygon();


      /**
       * Destructor
       */
      virtual ~GfxPolygon();

      /**
       *   Sets new coords. Will be 32 bits.
       *   @newCoords Vector of new coordinates
       */
      void setCoords( const vector<MC2Coordinate>& newCoords );
   
      /**
       * Get the i:th latitude. O(1) complexity regardless of the
       * internal representation of the coordinates.
       * Prefer this method to getLat( uint32 i ).
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLat  The latitude at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The latitude.
       */
      inline int32 getLat( uint32 i, int32 prevLat ) const;
      
      /**
       * Get the i:th longitude. O(1) complexity regardless of the 
       * internal representation of the coordinates.
       * Prefer this method to getLon( uint32 i ).
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLon  The longitudde at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The longitude.
       */
      inline int32 getLon( uint32 i, int32 prevLon ) const;

      
      /**
       * Get the i:th latitude without specifying the previous latitude.
       * O(1) complexity if the coordinates are represented by
       * absolute 32 bit coordinates (a GfxCoordinates32 object), 
       * O(n) complexity if the coordinates are represented by
       * relative 16 bit coordinates (a GfxCoordinates16 object), where
       * n is the number of coordinates added so far.
       * Use with care! Rather use getLat( uint32 i, int32 prevLat )
       * if possible.
       * 
       * @param   i        Coordinate number to get.
       * @return  The latitude.
       */
      inline virtual int32 getLat( uint32 i ) const;

      
      /**
       * Get the i:th longitude without specifying the previous longitude.
       * O(1) complexity if the coordinates are represented by
       * absolute 32 bit coordinates (a GfxCoordinates32 object), 
       * O(n) complexity if the coordinates are represented by
       * relative 16 bit coordinates (a GfxCoordinates16 object), where
       * n is the number of coordinates added so far.
       * Use with care! Rather use getLon( uint32 i, int32 prevLon )
       * if possible.
       * 
       * @param   i        Coordinate number to get.
       * @return  The longitude.
       */
      inline virtual int32 getLon( uint32 i ) const;

      
      /**
       * @return  The number of coordinates.
       */
      inline uint32 getNbrCoordinates() const;

      /**
       *    Get the length of this polygon (meters)
       *    @return The length, in meters, of the polygon.
       */
      inline float64 getLength() const;

      /**
       *    Set the area in sq meters of the true feature that 
       *    this polygon  either completely or partly covers.
       *    @param   area  The area in m2.
       */
      inline void setArea( float64 area );
      
      /**
       *    Get the area in sq meters of the true feature that 
       *    this polygon  either completely or partly covers.
       *    @return The area in m2.
       */
      inline float64 getArea() const;

      /**
       *    Get a coordinate on the polygon, that is located length
       *    meters from one of the nodes (which one descided by the value
       *    of the fromFirst-parameter).
       */
      inline bool getPointOnPolygon(float64 length, int& nbrPassedCoords,
                                    int32& lat, int32 &lon, 
                                    bool fromFirst = true) const;

      
      /**
       * @return  True if 16 bit relative coordinate representation is 
       *          used. False if 32 bit absolute coordinate representation
       *          is used.
       */
      inline virtual bool usesCoordinates16Bits() const;

      
      /**
       * Add new coordinates. O(1) complexity regardless of internal
       * represenation of the coordinates.
       * Prefer this method to addCoordinate( int32 lat, int32 lon ).
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       * @param   prevLat  Previous latitude. Ignored if no previous
       *                   coordinates exist.
       * @param   prevLon  Previous longitude. Ignored if no previous
       *                   coordinates exist.
       */
      inline virtual void addCoordinate( int32 lat, int32 lon, 
                                         int32 prevLat, int32 prevLon );
      
      
      /**
       * Add new coordinates without specifying the previous ones.
       * O(1) complexity if the coordinates are represented by
       * absolute 32 bit coordinates (a GfxCoordinates32 object), 
       * O(n) complexity if the coordinates are represented by
       * relative 16 bit coordinates (a GfxCoordinates16 object), where
       * n is the number of coordinates added so far.
       * Use with care! Rather use the other version of addCoordinate().
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       */
      inline virtual void addCoordinate( int32 lat, int32 lon );


      inline virtual bool removeLastCoordinate();

      
      /**
       * Set the first latitude value.
       * @param   startLat The first latitude value.
       */
      inline void setStartLat( int32 startLat );

      
      /**
       * Set the first longitude value.
       * @param   startLat The first longitude value.
       */
      inline void setStartLon( int32 startLon );

      /**
       * The size of the GfxPolygon when put into a DataBuffer, in bytes.
       */
      virtual uint32 getSize() const;


      /**
       * Dump information about the polygon to stdout.
       */
      virtual void dump(int verboseLevel = 1) const;


      /**
       * Load the polygon from a DataBuffer.
       *
       * @param data DataBuffer with the GfxPolygon data to read.
       */
      bool createFromDataBuffer( DataBuffer* data );


      /**
       * Put the polygon into a DataBuffer. Should write getSize bytes.
       */
      virtual bool save( DataBuffer* data ) const;

      /**
       * Get the coordinate at position i.
       */
      inline MC2Coordinate getCoord( uint32 i ) const;

   class const_iterator :
      public std::iterator<std::forward_iterator_tag,
                           MC2Coordinate >
   {
   public:
      const_iterator( const GfxPolygon& poly,
                      int pos ) : m_poly( &poly ), m_pos( pos ),
                                  m_lastUpdatedPos( -1 ) {
         
      }
      
      const_iterator& operator++() {
         // Must update the old coord
         updateCoord();
         ++m_pos;
         return *this;
      }

      void updateCoord() {
         if ( m_pos != m_lastUpdatedPos ) {
            MC2Coordinate lastCoord ( m_coord );
            m_coord.lat = m_poly->getLat( m_pos, lastCoord.lat );
            m_coord.lon = m_poly->getLon( m_pos, lastCoord.lon );
            m_lastUpdatedPos = m_pos;
         } 
      }
            
      const MC2Coordinate& operator*() const {
         // Heh
         const_cast<const_iterator*>(this)->updateCoord();
         return m_coord;
      }
      
      const MC2Coordinate* operator->() const {
         // Update coordinate.
         this->operator*();
         return &m_coord;
      }
      
      bool operator!=( const const_iterator& other ) const {
         return ( m_pos != other.m_pos ) || ( m_poly != other.m_poly );
      }
      
      bool operator==( const const_iterator& other ) const {
         return !(*this != other );
      }
   private:
      const GfxPolygon* m_poly;
      int m_pos;
      int m_lastUpdatedPos;
      MC2Coordinate m_coord;
   };

   const_iterator begin() const {
      return const_iterator( *this, 0 );
   }

   const_iterator end() const {
      return const_iterator( *this, getNbrCoordinates() );
   }

   protected:
      /**
       * Read header from databuffer.
       */
      virtual void readHeader( DataBuffer* data );

      
      /**
       * Reads coordinates from databuffer.
       */
      void readCoords( DataBuffer* data );


      /**
       * Writes the header into Databuffer.
       */
      virtual void writeHeader( DataBuffer* data ) const;

      
      /**
       * Writes the coordinates into Databuffer.
       */
      void writeCoords( DataBuffer* data ) const;


   private:

      GfxCoordinates* m_coords;

      /**
       *    The area in square meters of the full feature.
       */
      float64 m_area;
      
};


// =======================================================================
//                                     Implementation of inlined methods =


inline int32 
GfxPolygon::getLat( uint32 i, int32 prevLat ) const
{
   return (m_coords->getLat(i, prevLat));
}


inline int32 
GfxPolygon::getLon( uint32 i, int32 prevLon ) const
{
   return (m_coords->getLon(i, prevLon));
}

inline int32 
GfxPolygon::getLat( uint32 i ) const
{
   return (m_coords->getLat(i));
}


inline int32 
GfxPolygon::getLon( uint32 i ) const
{
   return (m_coords->getLon(i));
}


inline uint32 
GfxPolygon::getNbrCoordinates() const
{
   return (m_coords->getNbrCoordinates());
}

inline float64 
GfxPolygon::getLength() const
{
   return m_coords->getLength();
}


inline void 
GfxPolygon::setArea( float64 area )
{
   m_area = area;
}


inline float64 
GfxPolygon::getArea() const
{
   return m_area;
}


inline bool 
GfxPolygon::getPointOnPolygon(float64 length, int& nbrPassedCoords,
                              int32& lat, int32 &lon, 
                              bool fromFirst) const
{
   return m_coords->getPointOnPolygon(length, nbrPassedCoords,
                                      lat, lon, fromFirst);
}


inline bool 
GfxPolygon::usesCoordinates16Bits() const
{
   return (m_coords->usesCoordinates16Bits());
}


inline void
GfxPolygon::addCoordinate( int32 lat, int32 lon )
{
   m_coords->addCoordinate(lat, lon);
}


inline void
GfxPolygon::addCoordinate( int32 lat, int32 lon, 
                           int32 prevLat, int32 prevLon )
{
   m_coords->addCoordinate(lat, lon, prevLat, prevLon);
}


inline bool 
GfxPolygon::removeLastCoordinate()
{
   return m_coords->removeLastCoordinate();
}


inline void
GfxPolygon::setStartLat( int32 startLat )
{
   m_coords->setStartLat( startLat );
}


inline void
GfxPolygon::setStartLon( int32 startLon )
{
   m_coords->setStartLon( startLon );
}

inline MC2Coordinate 
GfxPolygon::getCoord( uint32 i ) const 
{
   return MC2Coordinate( getLat( i ), getLon( i ) );
}

#endif // GFXPOLYGON_H


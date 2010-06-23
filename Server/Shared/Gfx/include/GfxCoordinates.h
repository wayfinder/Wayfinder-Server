/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXCOORDINATES_H
#define GFXCOORDINATES_H

#include "config.h"
#include <vector>
#include "GfxConstants.h"
#include "MC2Coordinate.h"
#include "TileMapCoord.h"

class DataBuffer;

/**
 *    Abstract class representing a collection of coordinates.
 *
 */
class GfxCoordinates {
   public:
      /** 
       * Constructor
       * @param   startSize   Allocate the number of coordinates 
       *                      to this value.
       */
      GfxCoordinates( uint32 startSize );


      /** 
       * Constructor, used when reading from databuffer.
       */
      GfxCoordinates();


      /**
       * Destructor
       */
      virtual ~GfxCoordinates();


      /**
       * Static method. Creates a new GfxCoordinates object with the
       * correct dynamic type from a databuffer.
       * @param   data  The databuffer.
       * @return  A new GfxCoordinates object with the correct dynamic type.
       */
      static GfxCoordinates* createNewGfxCoordinates( DataBuffer* data );
      
      
      /**
       * Static method. Creates a new GfxCoordinates object with the
       * correct dynamic based on the parameters.
       * @param   coordinates16Bits Whether 16 bit relative or 32 bit
       *                            absolute coordinate representation
       *                            should be used.
       * @param   startSize         Allocate the number of coordinates 
       *                            to this value.
       * @return  A new GfxCoordinates object with the correct dynamic type.
       */
      static GfxCoordinates* createNewGfxCoordinates( bool coordinates16Bit,
                                                   uint32 startSize = 16);
      
      
      /**
       * Abstract method.
       * The size of the GfxCoordinates when put into a DataBuffer, 
       * in bytes.
       * @return  The size.
       */
      virtual uint32 getSize() const = 0;


      /**
       * Dump information about the polygon to stdout.
       * @param   verboseLevel   The verboseLevel.
       */
      virtual void dump(int verboseLevel = 1) const;


      /**
       * Abstract method.
       * Load the coordinates from a DataBuffer.
       *
       * @param data DataBuffer with the GfxCoordinates data to read.
       * @return  True if the operation was succesful, false otherwise.
       */
      virtual bool createFromDataBuffer( DataBuffer* data ) = 0;


      /**
       * Abstract method.
       * Put the coordinates into a DataBuffer. Should write getSize bytes.
       * @param   data  Preallocated databuffer.
       * @return  True if the operation was succesful, false otherwise.
       */
      virtual bool save( DataBuffer* data ) const = 0;

      
      /**
       * Abstract method.
       * Get the i:th latitude. O(1) complexity regardless of the dynamic
       * type of the GfxCoordinates object.
       * Prefer this method to getLat( uint32 i ).
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLat  The latitude at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The latitude.
       */
      virtual int32 getLat( uint32 i, int32 prevLat ) const = 0;

      
      /**
       * Abstract method.
       * Get the i:th longitude. O(1) complexity regardless of the dynamic
       * type of the GfxCoordinates object.
       * Prefer this method to getLon( uint32 i ).
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLon  The longitudde at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The longitude.
       */
      virtual int32 getLon( uint32 i, int32 prevLon ) const = 0;

      
      /**
       * Abstract method.
       * Get the i:th latitude without specifying the previous latitude.
       * O(1) complexity if this is a GfxCoordinates32 object, 
       * O(i) complexity if this is a GfxCoordinates16 object.
       * Use with care! Rather use getLat( uint32 i, int32 prevLat )
       * if possible.
       * 
       * @param   i        Coordinate number to get.
       * @return  The latitude.
       */
      virtual int32 getLat( uint32 i ) const = 0;

      
      /**
       * Abstract method.
       * Get the i:th longitude without specifying the previous longitude.
       * O(1) complexity if this is a GfxCoordinates32 object, 
       * O(i) complexity if this is a GfxCoordinates16 object.
       * Use with care! Rather use getLon( uint32 i, int32 prevLon )
       * if possible.
       * 
       * @param   i        Coordinate number to get.
       * @return  The longitude.
       */
      virtual int32 getLon( uint32 i ) const = 0;

      
      /**
       * Abstract method.
       * @return  The number of coordinates.
       */
      virtual uint32 getNbrCoordinates() const = 0;

      
      /**
       * Abstract method.
       * @return  True if 16 bit relative coordinate representation is 
       *          used. False if 32 bit absolute coordinate representation
       *          is used.
       */
      virtual bool usesCoordinates16Bits() const = 0;

      
      /**
       * Abstract method.
       * Add new coordinates. O(1) complexity regardless of the dynamic
       * type of the GfxCoordinates object.
       * Prefer this method to addCoordinate( int32 lat, int32 lon ).
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       * @param   prevLat  Previous latitude. Ignored if no previous
       *                   coordinates exist.
       * @param   prevLon  Previous longitude. Ignored if no previous
       *                   coordinates exist.
       */
      virtual void addCoordinate( int32 lat, int32 lon,
                                  int32 prevlat, int32 prevLon ) = 0;
      
      
      /**
       * Abstract method.
       * Add new coordinates without specifying the previous ones.
       * O(1) complexity if this is a GfxCoordinates32 object, 
       * O(n) complexity if this is a GfxCoordinates16 object where
       * n is the number of coordinates added so far.
       * Use with care! Rather use the other version of addCoordinate().
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       */
      virtual void addCoordinate( int32 lat, int32 lon ) = 0;


      virtual bool removeLastCoordinate() = 0;
      
      /**
       * Abstract method.
       * Set the first latitude value.
       * @param   startLat The first latitude value.
       */
      virtual void setStartLat( int32 startLat ) = 0;

      
      /**
       * Abstract method.
       * Set the first longitude value.
       * @param   startLat The first longitude value.
       */
      virtual void setStartLon( int32 startLon ) = 0;

      /**
       *    Get the length of this polygon in meters.
       *    @return The length of this polygon, in meters.
       */
      virtual float64 getLength() const;

      /**
       *    Abstract method.
       *    Get a coordinate on the polygon that is located length meters
       *    from either the first or the last coordinate.
       *    @param length  The number of meters from the first (or last) 
       *                   coordinate to the coordinate to return. Valid
       *                   values are 0 <= length < getLength().
       *    @param nbrPassedCoords Outparameter that is set to the number 
       *                   of coordinates that are read past to get 
       *                   (lat, lon).
       *    @param lat     Outparamter that is set to the latitude part
       *                   of the coordinate that is located on the polygon,
       *                   length meters from the first or last coordinate.
       *    @param lon     Outparamter that is set to the longitude part
       *                   of the coordinate that is located on the polygon,
       *                   length meters from the first or last coordinate.
       *    @param fromFirst Optional parameter that indicated if the
       *                   returned coordinate should be located length
       *                   meters from the first (this parameter set to
       *                   true) or from the last (this parameter set to
       *                   false) coordinate.
       *    @return True if the outparameters are set correctly, false
       *            otherwise. If set to false, nothing could be said
       *            about the outparameters.
       */
      virtual bool getPointOnPolygon(float64 length, int& nbrPassedCoords,
                                     int32& lat, int32 &lon,
                                     bool fromFirst = true) const;
};

/**
 *    16 bit relative representation of a collection of coordinates.
 *
 */
class GfxCoordinates16 : public GfxCoordinates {
   public:
      
      /** 
       * Constructor
       * @param   startSize   Allocate the number of coordinates 
       *                      to this value.
       */
      GfxCoordinates16( uint32 startSize );


      /** 
       * Constructor, used when reading from databuffer.
       */
      GfxCoordinates16();


      /**
       * Destructor
       */
      virtual ~GfxCoordinates16();

      /**
       * The size of the GfxCoordinates when put into a DataBuffer, 
       * in bytes.
       * @return  The size.
       */
      uint32 getSize() const;


      /**
       * Dump information about the polygon to stdout.
       * @param   verboseLevel   The verboseLevel.
       */
      void dump(int verboseLevel = 1) const;

      
      /**
       * Put the polygon into a DataBuffer. Should write getSize bytes.
       * @param   data  Preallocated databuffer.
       * @return  True if the operation was succesful, false otherwise.
       */
      bool save( DataBuffer* data ) const;


      /**
       * Load the polygon from a DataBuffer.
       *
       * @param data DataBuffer with the GfxCoordinates data to read.
       * @return  True if the operation was succesful, false otherwise.
       */
      bool createFromDataBuffer( DataBuffer* data );

      
      /**
       * Get the i:th latitude. O(1) complexity.
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLat  The latitude at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The latitude.
       */
      inline int32 getLat( uint32 i, int32 prevLat ) const;
      
      
      /**
       * Get the i:th longitude. O(1) complexity.
       * Prefer this method to getLat( uint32 i ).
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLon  The longitudde at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The longitude.
       */
      inline int32 getLon( uint32 i, int32 prevLon ) const;

      
      /**
       * Get the i:th latitude without specifying the previous latitude.
       * O(i) complexity, so use with care!
       * Use with care! Rather use getLat( uint32 i, int32 prevLat )
       * if possible.
       * 
       * @param   i        Coordinate number to get.
       * @return  The latitude.
       */
      inline int32 getLat( uint32 i ) const;

      
      /**
       * Get the i:th longitude without specifying the previous longitude.
       * O(i) complexity, so avoid usage!
       * Use with care! Rather use getLon( uint32 i, int32 prevLon )
       * if possible.
       * 
       * @param   i        Coordinate number to get.
       * @return  The longitude.
       */
      inline int32 getLon( uint32 i ) const;
      
      
      /**
       * @return  The number of coordinates.
       */
      inline uint32 getNbrCoordinates() const;
      
      
      /**
       * @return  True.
       */
      inline bool usesCoordinates16Bits() const;
      
      
      /**
       * Add new coordinates. O(1) complexity.
       * Prefer this method to addCoordinate( int32 lat, int32 lon ).
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       * @param   prevLat  Previous latitude. Ignored if no previous
       *                   coordinates exist.
       * @param   prevLon  Previous longitude. Ignored if no previous
       *                   coordinates exist.
       */
      inline void addCoordinate( int32 lat, int32 lon,
                                         int32 prevlat, int32 prevLon );
      
      
      /**
       * Add new coordinates without specifying the previous ones.
       * O(n) complexity, so avoid usage! 
       * Use with care! Rather use the other version of addCoordinate().
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       */
      inline void addCoordinate( int32 lat, int32 lon );
      

      inline bool removeLastCoordinate();

      
      /**
       * Add coordinate difference from previous coordinate.
       * @param   latDiff  Lat difference from previous lat.
       * @param   lonDiff  Lon difference from previous lon.
       */
      void addCoordinateDiff( int latDiff, int lonDiff );      
      
      
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
      
   private:
      /**
       * Read header from databuffer.
       * @param   data  Databuffer.
       * @retrun Number of coordinates.
       */
      uint32 readHeader( DataBuffer* data );

      
      /**
       * Reads coordinates from databuffer.
       * @param   data           Databuffer.
       * @param   nbrCoordinates Nbr coordinates to read.
       */
      void readCoords( DataBuffer* data, uint32 nbrCoordinates );


      /**
       * Writes the header into Databuffer.
       * @param   data  Databuffer.
       */
      void writeHeader( DataBuffer* data ) const;

      
      /**
       * Writes the coordinates into Databuffer.
       * @param   data  Databuffer.
       */
      void writeCoords( DataBuffer* data ) const;
      
      
      /**
       * The starting latitude. 32 bits.
       */
      int32 m_startLat;

      
      /**
       * The starting longitude. 32 bits.
       */
      int32 m_startLon;

      
      /**
       * Vector of relative 16 bit latitude coordinates.
       */
      vector<int16> m_lat;

      
      /**
       * Vector relative 16 bit longitude coordinates.
       */
      vector<int16> m_lon;
};

/**
 * 32 bit representation of coordinates.
 */
class GfxCoordinates32 : public GfxCoordinates {
   public:
      /** 
       * Constructor
       * @param   startSize   Allocate the number of coordinates 
       *                      to this value.
       */
      GfxCoordinates32( uint32 startSize );

      /**
       *   @param coords The coordinates.
       */
      GfxCoordinates32( const vector<MC2Coordinate>& coords );
   
      /** 
       * Constructor, used when reading from databuffer.
       */
      GfxCoordinates32();


      /**
       * Destructor
       */
      virtual ~GfxCoordinates32();


      /**
       * The size of the GfxCoordinates when put into a DataBuffer, 
       * in bytes.
       * @return  The size.
       */
      uint32 getSize() const;


      /**
       * Dump information about the polygon to stdout.
       * @param   verboseLevel   The verboseLevel.
       */
      void dump(int verboseLevel = 1) const;

      
      /**
       * Put the polygon into a DataBuffer. Should write getSize bytes.
       * @param   data  Preallocated databuffer.
       * @return  True if the operation was succesful, false otherwise.
       */
      bool save( DataBuffer* data ) const;


      /**
       * Load the polygon from a DataBuffer.
       *
       * @param data DataBuffer with the GfxCoordinates data to read.
       * @return  True if the operation was succesful, false otherwise.
       */
      bool createFromDataBuffer( DataBuffer* data );

      
      /**
       * Get the i:th latitude. O(1) complexity.
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLat  The latitude at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The latitude.
       */      
      inline int32 getLat( uint32 i, int32 prevLat ) const;
      
      
      /**
       * Get the i:th longitude. O(1) complexity.
       * 
       * @param   i        Coordinate number to get.
       * @param   prevLon  The longitudde at position i-1. If i == 0
       *                   then this parameter is ignored.
       * @return  The longitude.
       */      
      inline int32 getLon( uint32 i, int32 prevLon ) const;

      
      /**
       * Abstract method.
       * Get the i:th latitude without specifying the previous latitude.
       * O(1) complexity.
       * 
       * @param   i        Coordinate number to get.
       * @return  The latitude.
       */      
      inline int32 getLat( uint32 i ) const;
      
      
      /**
       * Get the i:th longitude without specifying the previous longitude.
       * O(1) complexity.
       * 
       * @param   i        Coordinate number to get.
       * @return  The longitude.
       */      
      inline int32 getLon( uint32 i ) const;
      
      
      /**
       * @return  The number of coordinates.
       */
      inline uint32 getNbrCoordinates() const;

      
      /**
       * @return False.
       */
      inline bool usesCoordinates16Bits() const;

      
      /**
       * Add new coordinates. O(1) complexity.
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       * @param   prevLat  Previous latitude. Ignored if no previous
       *                   coordinates exist.
       * @param   prevLon  Previous longitude. Ignored if no previous
       *                   coordinates exist.
       */
      inline void addCoordinate( int32 lat, int32 lon,
                                         int32 prevlat, int32 prevLon );
      
      /**
       * Abstract method.
       * Add new coordinates without specifying the previous ones.
       * O(1) complexity.
       *
       * @param   lat      Latitude to add.
       * @param   lon      Longitude to add.
       */      
      inline void addCoordinate( int32 lat, int32 lon );
      
      inline bool removeLastCoordinate();
     
      /**
       * Add coordinate difference from previous coordinate.
       * @param   latDiff  Lat difference from previous lat.
       * @param   lonDiff  Lon difference from previous lon.
       */
      void addCoordinateDiff( int latDiff, int lonDiff );      
      
      
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
      
      
   private:
      /**
       * Read header from databuffer.
       * @param   data  Databuffer.
       * @return  Number of coordinates.
       */
      uint32 readHeader( DataBuffer* data );

      
      /**
       * Reads coordinates from databuffer.
       * @param   data  Databuffer.
       * @param   nbrCoordinates Nbr coordinates to read.
       */
      void readCoords( DataBuffer* data, uint32 nbrCoordinates );


      /**
       * Writes the header into Databuffer.
       * @param   data  Databuffer.
       */
      void writeHeader( DataBuffer* data ) const;

      
      /**
       * Writes the coordinates into Databuffer.
       * @param   data  Databuffer.
       */
      void writeCoords( DataBuffer* data ) const;
      
      
      /**
       * Vector of absolute 32 bit latitude coordinates.
       */
      vector<TileMapCoord> m_coords;      
};



// =======================================================================
//                                     Implementation of inlined methods =



////////////////////////////////////////////////////////////////////////

inline int32 
GfxCoordinates16::getLat( uint32 i, int32 prevLat ) const
{
   if (i == 0) {
      return (m_startLat);
   } else {
      return (m_lat[i - 1] + prevLat);
   }
}


inline int32 
GfxCoordinates16::getLon( uint32 i, int32 prevLon ) const
{
   if (i == 0) {
      return (m_startLon);
   } else {
      return (m_lon[i - 1] + prevLon);
   }

}


inline int32 
GfxCoordinates16::getLat( uint32 i ) const
{
   // Cost O(i)
   uint32 lat = m_startLat;
   for (uint32 j = 0; j < i; j++) {
      lat += m_lat[j];
   }
   
   return (lat);
}


inline int32 
GfxCoordinates16::getLon( uint32 i ) const
{
   // Cost O(i)
   uint32 lon = m_startLon;
   for (uint32 j = 0; j < i; j++) {
      lon += m_lon[j];
   }
   
   return (lon);
}


inline uint32 
GfxCoordinates16::getNbrCoordinates() const
{
   uint32 size = m_lon.size();
   if ( (size > 0) || 
        ((m_startLat != GfxConstants::IMPOSSIBLE) &&
         (m_startLon != GfxConstants::IMPOSSIBLE)) ) {
      size++;
   }
   return (size);
}


inline bool 
GfxCoordinates16::usesCoordinates16Bits() const
{
   return (true);
}


inline void
GfxCoordinates16::addCoordinate( int32 lat, int32 lon )
{
   int32 nbrCoords = getNbrCoordinates();
   if (nbrCoords > 0) {
      return addCoordinate(lat, lon, 
                           getLat(nbrCoords - 1), getLon(nbrCoords - 1));
   } else {
      return addCoordinate(lat, lon, 
                           GfxConstants::IMPOSSIBLE, GfxConstants::IMPOSSIBLE);
   }

}
   

inline void
GfxCoordinates16::addCoordinate( int32 lat, int32 lon,
                                 int32 prevLat, int32 prevLon )
{
   if ((m_startLat == GfxConstants::IMPOSSIBLE) && 
       (m_startLon == GfxConstants::IMPOSSIBLE)) {
      // Invalid start coordinates.
      mc2dbg4 << "Setting (m_startLat,m_startLon)=(" << m_startLat << ","
              << m_startLon << ")" << endl;
      m_startLat = lat;
      m_startLon = lon;
   } else {
      addCoordinateDiff( lat - prevLat, lon - prevLon );
   }
}

inline bool 
GfxCoordinates16::removeLastCoordinate()
{
   if (m_lon.size() > 0) {
      m_lon.pop_back();
      m_lat.pop_back();
      return true;
   } else if ( (m_startLat != GfxConstants::IMPOSSIBLE) && 
               (m_startLon != GfxConstants::IMPOSSIBLE)) {
      m_startLat = GfxConstants::IMPOSSIBLE;
      m_startLon = GfxConstants::IMPOSSIBLE;
      return true;
   }
   return false;
}


inline void 
GfxCoordinates16::setStartLat( int32 startLat )
{
   m_startLat = startLat;
}

inline void 
GfxCoordinates16::setStartLon( int32 startLon )
{
   m_startLon = startLon;
}

////////////////////////////////////////////////////////////////////////

inline int32 
GfxCoordinates32::getLat( uint32 i ) const
{
   return m_coords[i].lat;
}


inline int32 
GfxCoordinates32::getLon( uint32 i ) const
{
   return m_coords[i].lon;
}

inline int32 
GfxCoordinates32::getLat( uint32 i, int32 prevLat ) const
{
   return getLat(i);
}


inline int32 
GfxCoordinates32::getLon( uint32 i, int32 prevLon ) const
{
   return getLon(i);
}

inline uint32 
GfxCoordinates32::getNbrCoordinates() const
{
   return m_coords.size();
}


inline bool 
GfxCoordinates32::usesCoordinates16Bits() const
{
   return false;
}

inline void
GfxCoordinates32::addCoordinate( int32 lat, int32 lon )
{
   m_coords.push_back( TileMapCoord( lat, lon ) );
}

inline void
GfxCoordinates32::addCoordinate( int32 lat, int32 lon, 
                                 int32 prevlat, int32 prevLon )
{
   addCoordinate( lat, lon );
}

inline bool 
GfxCoordinates32::removeLastCoordinate()
{
   if ( ! m_coords.empty() ) {
      m_coords.pop_back();
      return true;
   }
   return false;
}



inline void
GfxCoordinates32::addCoordinateDiff( int latDiff, int lonDiff )
{
   addCoordinate( m_coords.back().lat + latDiff,
                  m_coords.back().lon + lonDiff );
}

inline void 
GfxCoordinates32::setStartLat( int32 startLat )
{
   if ( ! m_coords.empty() ) {
      m_coords[0].lat = startLat;
   } 
}

inline void
GfxCoordinates32::setStartLon( int32 startLon )
{
   if ( ! m_coords.empty() ) {
      m_coords[0].lon = startLon;
   }
}

#endif // GFXCOORDINATES_H


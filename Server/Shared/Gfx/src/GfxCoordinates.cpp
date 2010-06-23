/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "DataBuffer.h"

#include "GfxCoordinates.h"
#include "GfxUtility.h"

#include <math.h>
#include <stdlib.h>

GfxCoordinates::GfxCoordinates( uint32 startSize ) 
{

}


GfxCoordinates::GfxCoordinates() 
{

}


GfxCoordinates::~GfxCoordinates() 
{
   // I hope vector works
}


GfxCoordinates* 
GfxCoordinates::createNewGfxCoordinates( DataBuffer* data ) 
{
   bool coordinates16Bits = data->readNextBool();
   GfxCoordinates* coords = 
      createNewGfxCoordinates( coordinates16Bits );
   coords->createFromDataBuffer( data );
   return (coords);
}

   
GfxCoordinates* 
GfxCoordinates::createNewGfxCoordinates( bool coordinates16Bits, 
                                         uint32 startSize ) 
{
   GfxCoordinates* coords;
   if (coordinates16Bits) {
      coords = new GfxCoordinates16( startSize ); 
   } else {
      coords = new GfxCoordinates32( startSize ); 
   }

   return (coords);
}
      
uint32 
GfxCoordinates::getSize() const {
   // 16bitcoords (1 bool) + 3 byte padding
   uint32 size = 1 + 3;
   return size;
}


void
GfxCoordinates::dump(int verboseLevel) const 
{

}


bool
GfxCoordinates::createFromDataBuffer( DataBuffer* data ) 
{
   return true;
}


bool
GfxCoordinates::save( DataBuffer* data ) const 
{
   data->writeNextBool( usesCoordinates16Bits() );
   return true;
}

float64
GfxCoordinates::getLength() const
{
   const uint32 n = getNbrCoordinates();
   float64 length = 0;
   if (n > 0) {
      int32 lat = getLat(0);
      int32 lon = getLon(0);
      for (uint32 i=1; i<getNbrCoordinates(); ++i) {
         length += sqrt(GfxUtility::squareP2Pdistance_linear(
                                 lat, lon, getLat(i), getLon(i)));
         lat = getLat(i);
         lon = getLon(i);
      }
   }
   mc2dbg4 << "getLength() returning " << length << endl;
   return length;
}


bool 
GfxCoordinates::getPointOnPolygon(float64 length, int& nbrPassedCoords,
                                  int32& lat, int32 &lon,
                                  bool fromFirst) const
{
   const int n = getNbrCoordinates();
   if (n < 2) {
      mc2log << warn << "GfxCoordinates::getPointOnPolygon only " << n 
             << " coordinates." << endl;
      return false;
   }

   // Currently does not handle "fromFirst"
   if (!fromFirst) {
      mc2log << error << here << " Does not handle fromFirst == false" 
             << endl;
      return false;
   }

   // If the request was exactly zero meters
   // from the first coordinate, then use the first
   // coordinate.
   if ( length == 0 ) {
      lat = getLat( 0 );
      lon = getLon( 0 );
      nbrPassedCoords = 1;
      return true;
   }

   float64 ackLength = 0;
   float64 lastDist = 0;
   int32 curLat = getLat(0);
   int32 curLon = getLon(0);
   nbrPassedCoords = 1;

   // Let nbrPassedCoords point to the coordinate _after_ the last to include
   while ( (nbrPassedCoords < n) && (ackLength < length)) {
      mc2dbg8 << "LOOP: nbrPassed=" << nbrPassedCoords << ", n=" << n
              << ", ackLength=" << ackLength << ", length=" << length 
              << endl;
      lastDist = sqrt(GfxUtility::squareP2Pdistance_linear(
                     curLat, curLon, 
                     getLat(nbrPassedCoords), getLon(nbrPassedCoords)));
      ackLength += lastDist;
      curLat = getLat(nbrPassedCoords);
      curLon = getLon(nbrPassedCoords);
      ++nbrPassedCoords;
   }

   if (ackLength >= length) {
      --nbrPassedCoords;
      // Caculate (lat,lon) that is located between 
      // (getLat(nbrPassedCoords-1), getLon(nbrPassedCoords-1)) and 
      // (getLat(nbrPassedCoords), getLon(nbrPassedCoords))
      GfxUtility::getPointOnLine(
         getLat(nbrPassedCoords-1), getLon(nbrPassedCoords-1),
         getLat(nbrPassedCoords), getLon(nbrPassedCoords),
         length-(ackLength-lastDist), lat, lon);
      return true;
   }
   return false;
}



///////////////////////////////////////////////////////////////////////
GfxCoordinates16::GfxCoordinates16( uint32 startSize ) 
{
   m_startLat = GfxConstants::IMPOSSIBLE;
   m_startLon = GfxConstants::IMPOSSIBLE;
}


GfxCoordinates16::GfxCoordinates16() 
{
   m_startLat = GfxConstants::IMPOSSIBLE;
   m_startLon = GfxConstants::IMPOSSIBLE;
}


GfxCoordinates16::~GfxCoordinates16() 
{
   // I hope vector works
}




uint32 
GfxCoordinates16::getSize() const 
{
   uint32 size = GfxCoordinates::getSize();
   // (startLat+startLon) + nbrCoordinates + 
   // nbrCoordinates * 2
   size += 2*4 + 4 + getNbrCoordinates()*2*2;

   return size;
}


void
GfxCoordinates16::dump(int verboseLevel) const 
{
   if ( verboseLevel > 0 ) {
      cout << "      GfxCoordinates16 " << endl
           << "         Header: " << endl
           << "            startLat:       " << getLat(0) << endl
           << "            startLon:       " << getLon(0) << endl
           << "            nbrCoordinates: " << getNbrCoordinates() 
           << endl;
      if ( verboseLevel > 9 ) {
         int32 lat = getLat(0);
         int32 lon = getLon(0);
         for ( uint32 i = 1 ; i < getNbrCoordinates() ; i ++ ) {
            lat = getLat(i, lat);
            lon = getLon(i, lon);
            cout << "               " << i 
                 << " latDiff " << m_lat[i-1] 
                 << " lonDiff " << m_lon[i-1]
                 << " (" << lat << "," << lon << ")" << endl;
         }
      }
   }
}


bool
GfxCoordinates16::createFromDataBuffer( DataBuffer* data ) 
{
   GfxCoordinates::createFromDataBuffer( data );
   uint32 nbrCoordinates = readHeader( data );
   readCoords( data, nbrCoordinates );
   
   return true;
}


bool
GfxCoordinates16::save( DataBuffer* data ) const 
{
   GfxCoordinates::save( data );
   writeHeader( data );
   writeCoords( data );

   return true;
}


uint32
GfxCoordinates16::readHeader( DataBuffer* data ) 
{
   mc2dbg8 << "GfxCoordinates16::readHeader" << endl;
   m_startLat = data->readNextLong();
   m_startLon = data->readNextLong();
   // Return nbr coords
   return ( data->readNextLong() );
}

void
GfxCoordinates16::readCoords( DataBuffer* data, uint32 nbrCoords ) 
{
   for (uint32 i = 0; i < nbrCoords; i++) {
      m_lat.push_back( data->readNextShort() );
      m_lon.push_back( data->readNextShort() );
   }
}


void
GfxCoordinates16::writeHeader( DataBuffer* data ) const 
{
   data->writeNextLong( m_startLat );
   data->writeNextLong( m_startLon );
   data->writeNextLong( m_lat.size() );
}


void
GfxCoordinates16::writeCoords( DataBuffer* data ) const 
{
   for (uint32 i = 0; i < m_lat.size(); i++) {
      data->writeNextShort( m_lat[i] );
      data->writeNextShort( m_lon[i] );
   }
}


void
GfxCoordinates16::addCoordinateDiff( int latDiff, int lonDiff )
{
   const int MAX_COORD_DIFF = 0x7ffe;

   int fooStartLat = latDiff;
   int fooStartLon = lonDiff;
   int fooLat = 0;
   int fooLon = 0;

   // Make points along the too long line
   while ( (latDiff > MAX_COORD_DIFF) || (latDiff < -MAX_COORD_DIFF) ||
           (lonDiff > MAX_COORD_DIFF) || (lonDiff < -MAX_COORD_DIFF) ) 
   {
      // Move MAX_COORD_DIFF
      int tmpLatDiff = 0;
      int tmpLonDiff = 0;
      if ( abs( latDiff ) > abs( lonDiff ) ) {
         // lat largest
         tmpLatDiff = latDiff < 0 ? (-MAX_COORD_DIFF) : MAX_COORD_DIFF;
         tmpLonDiff = int( rint( 
            tmpLatDiff * float64(lonDiff) / latDiff ) );
      } else {
         // lon largest
         tmpLonDiff = lonDiff < 0 ? (-MAX_COORD_DIFF) : MAX_COORD_DIFF;
         tmpLatDiff = int( rint( 
            tmpLonDiff * float64(latDiff) / lonDiff ) );
      }
      mc2dbg8 << " spiltto  x " << tmpLonDiff 
              << " y " << tmpLatDiff << endl;
      m_lat.push_back( tmpLatDiff );
      m_lon.push_back( tmpLonDiff );
      fooLat += tmpLatDiff;
      fooLon += tmpLonDiff;
      latDiff -= tmpLatDiff;
      lonDiff -= tmpLonDiff;
   }

   fooLat += latDiff;
   fooLon += lonDiff;

   if ( fooStartLon != fooLon || fooStartLat != fooLat ) {
      mc2dbg4 << "      x " << fooStartLon << " y " << fooStartLat << endl
              << "      x " << fooLon << " y " << fooLat << endl;
   }
  
   m_lat.push_back( latDiff );
   m_lon.push_back( lonDiff );
}


///////////////////////////////////////////////////////////////////////
GfxCoordinates32::GfxCoordinates32( uint32 startSize ) 
{

}

GfxCoordinates32::GfxCoordinates32( const vector<MC2Coordinate>& coords )
{
   m_coords.insert( m_coords.end(), coords.begin(), coords.end() );
}


GfxCoordinates32::GfxCoordinates32() 
{

}


GfxCoordinates32::~GfxCoordinates32() 
{
   // I hope vector works
}

uint32 
GfxCoordinates32::getSize() const 
{
   uint32 size = GfxCoordinates::getSize();
   // nbrCoordinates + 
   // nbrCoordinates *2
   size += 4 + getNbrCoordinates()*2*4;

   return size;
}


void
GfxCoordinates32::dump(int verboseLevel) const 
{
   if ( verboseLevel > 0 ) {
      cout << "      GfxCoordinates32 " << endl
           << "         Header: " << endl
           << "            nbrCoordinates: " << getNbrCoordinates() 
           << endl 
           << "            startLat:       " << getLat(0) << endl
           << "            startLon:       " << getLon(0) << endl;
      if ( verboseLevel > 9 ) {
         int32 lat = getLat(0);
         int32 lon = getLon(0);
         for ( uint32 i = 1 ; i < getNbrCoordinates() ; i ++ ) {
            lat = getLat(i, lat);
            lon = getLon(i, lon);
            cout << "               " << i 
                 << " (" << lat << "," << lon << ")" << endl;
         }
      }
   }
}


bool
GfxCoordinates32::createFromDataBuffer( DataBuffer* data ) 
{
   GfxCoordinates::createFromDataBuffer( data );
   uint32 nbrCoordinates = readHeader( data );
   readCoords( data, nbrCoordinates );
   
   return true;
}


bool
GfxCoordinates32::save( DataBuffer* data ) const 
{
   GfxCoordinates::save( data );
   writeHeader( data );
   writeCoords( data );

   return true;
}


uint32
GfxCoordinates32::readHeader( DataBuffer* data ) 
{
   mc2dbg8 << "GfxCoordinates32::readHeader" << endl;
   // Return nbr coords
   return ( data->readNextLong() );
}

void
GfxCoordinates32::readCoords( DataBuffer* data, uint32 nbrCoords ) 
{
   for (uint32 i = 0; i < nbrCoords; i++) {
      const int32 lat = data->readNextLong();
      const int32 lon = data->readNextLongAligned();
      m_coords.push_back( TileMapCoord( lat, lon ) );
   }
}


void
GfxCoordinates32::writeHeader( DataBuffer* data ) const 
{
   data->writeNextLong( m_coords.size() );
}


void
GfxCoordinates32::writeCoords( DataBuffer* data ) const 
{
   for ( vector<TileMapCoord>::const_iterator it = m_coords.begin();
         it != m_coords.end();
         ++it ) {
      data->writeNextLong( it->lat );
      data->writeNextLong( it->lon );
   }
}


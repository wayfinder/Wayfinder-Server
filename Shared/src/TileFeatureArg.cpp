/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"
#include "TileFeatureArg.h"
#include "MathUtility.h"
#include "TileMap.h"

// ------------------------------- TileFeatureArg -------------------------

#ifdef MC2_SYSTEM
void 
TileFeatureArg::dump( ostream& stream ) const 
{
   stream << "Name " << m_name << endl;   
}

void
TileFeatureArg::saveFullArg( BitBuffer& buf ) const
{
   tileFeatureArg_t type = this->getArgType();
   buf.writeNextBits( type, 3 );
  
   buf.writeNextBits( m_name, 8 );
   
   switch ( type ) {
      case ( simpleArg ) : { 
         buf.writeNextBits( simpleArgCast( this )->getSize(), 5 );
      } break; 
      
      default:
         break;
   }
   TileMap tileMap;
   this->save( buf, tileMap, NULL );
}
#endif

TileFeatureArg* 
TileFeatureArg::loadFullArg( BitBuffer& buf )
{
   TileArgNames::tileArgName_t name;
   tileFeatureArg_t type;
   
   buf.readNextBits(type, 3);

   buf.readNextBits(name, 8);
   


   TileFeatureArg* arg = NULL;
   switch ( type ) {
      case ( simpleArg ) : { 
         byte size = buf.readNextBits( 5 );
         arg = new SimpleArg( name, size );
      } break; 
      
      case ( coordArg ) : {
         arg = new CoordArg( name );
      } break;
         
      case ( coordsArg ) : {
         // Cannot create default coords arg, since they need a map.
         arg = new CoordsArg( NULL, name );
      } break;
      
      case ( stringArg ) : {
         arg = new StringArg( name );
      } break;
      
      default :
         mc2log << fatal << "TileFeatureArg::loadFullArg invalid type"
                << endl;
         return NULL;
   }
   TileMap tmpTileMap;
   arg->load( buf, tmpTileMap, NULL );
   return arg;
}

// ------------------------------- SimpleArg ------------------------------
   
void 
SimpleArg::setValue( uint32 value ) 
{
   m_valueByScaleIdx.clear();
   m_valueByScaleIdx.push_back( value );
}

void 
SimpleArg::setValue( const vector<uint32>& valueByScaleIdx ) 
{
   m_valueByScaleIdx = valueByScaleIdx;
}

uint32 
SimpleArg::getValue( int scaleIdx ) const 
{
   if ( m_valueByScaleIdx.size() == 1 ) {
      scaleIdx = 0;
   }
   MC2_ASSERT( scaleIdx < (int) m_valueByScaleIdx.size() );   
   return m_valueByScaleIdx[ scaleIdx ];
}

#ifdef MC2_SYSTEM

void 
SimpleArg::dump( ostream& stream ) const 
{
   stream << "SimpleArg (" << (int)m_size << ")" << endl;
   TileFeatureArg::dump( stream );
   stream << "value " << getValue() << endl;
}

bool 
SimpleArg::save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const 
{
   int sameAsPrevious = 0;
   if ( prevArg != NULL ) {
      const SimpleArg* prevSimpleArg = 
         static_cast<const SimpleArg*> ( prevArg );
      if ( m_valueByScaleIdx == prevSimpleArg->m_valueByScaleIdx ) {
         // Same value as previous one.
         sameAsPrevious = 1;
      }
   }
   buf.writeNextBits( sameAsPrevious, 1 );
   if ( sameAsPrevious == 0 ) {
      // Not same as previous.
      // Write if more than one values exists.
      bool multi = ( m_valueByScaleIdx.size() > 1 );
      buf.writeNextBits( multi, 1 );
      if ( multi ) {
         // Assume 32 scale indexes are max.
         MC2_ASSERT( m_valueByScaleIdx.size() < 32 );
         buf.writeNextBits( m_valueByScaleIdx.size(), 5 );
         for ( uint32 i = 0; i < m_valueByScaleIdx.size(); ++i ) {
            buf.writeNextBits( m_valueByScaleIdx[ i ], m_size );
         }
      } else {
         // Only one value.
         MC2_ASSERT( ! m_valueByScaleIdx.empty() );
         buf.writeNextBits( m_valueByScaleIdx[ 0 ], m_size );
      }
   }
   return true;      
}
#endif

bool 
SimpleArg::load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg ) 
{
   bool sameAsPrevious = buf.readNextBits( 1 ) != 0;
   if ( sameAsPrevious ) {
      // Use previous argument value.
      const SimpleArg* prevSimpleArg = 
         static_cast<const SimpleArg*> ( prevArg );
      setValue( prevSimpleArg->getValue() );
   } else {
      // Not same as previous.
      // Read if multiple values.
      bool multi = buf.readNextBits( 1 ) != 0;
      if ( multi ) {
         // Multiple values.
         int nbrValues = buf.readNextBits( 5 );
         m_valueByScaleIdx.resize( nbrValues );
         for ( int i = 0; i < nbrValues; ++i ) {
            m_valueByScaleIdx[ i ] = buf.readNextBits( m_size );
         }
      } else {
         // Only one value.
         m_valueByScaleIdx.push_back( buf.readNextBits( m_size ) );
      }
   }
         
   return true;
}

TileFeatureArg* 
SimpleArg::clone()
{
   return new SimpleArg( m_name, m_size );
}

// ------------------------------- StringArg ------------------------------
   
void 
StringArg::setValue( const MC2SimpleString& str ) 
{
   m_stringByScaleIdx.clear();
   m_stringByScaleIdx.push_back( str );
}

void 
StringArg::setValue( const vector<MC2SimpleString>& strByScaleIdx ) 
{
   m_stringByScaleIdx = strByScaleIdx;
}

const MC2SimpleString&
StringArg::getValue( int scaleIdx ) const 
{
   if ( m_stringByScaleIdx.size() == 1 ) {
      scaleIdx = 0;
   }
   MC2_ASSERT( scaleIdx < (int)m_stringByScaleIdx.size() );
   return m_stringByScaleIdx[ scaleIdx ];
}

#ifdef MC2_SYSTEM

void 
StringArg::dump( ostream& stream ) const 
{
   stream << "StringArg" << endl;
   TileFeatureArg::dump( stream );
   stream << "string " << getValue() << endl;
}

bool 
StringArg::save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const 
{
   // Write if more than one values exists.
   bool multi = ( m_stringByScaleIdx.size() > 1 );
   buf.writeNextBits( multi, 1 );
   if ( multi ) {
      // Assume 32 scale indexes are max.
      MC2_ASSERT( m_stringByScaleIdx.size() < 32 );
      buf.writeNextBits( m_stringByScaleIdx.size(), 5 );
      buf.alignToByte();
      for ( uint32 i = 0; i < m_stringByScaleIdx.size(); ++i ) {
         buf.writeNextString( m_stringByScaleIdx[ i ].c_str() );
      }
   } else {
      // Only one value.
      buf.alignToByte();
      MC2_ASSERT( ! m_stringByScaleIdx.empty() );
      buf.writeNextString( m_stringByScaleIdx[ 0 ].c_str() );
   }
   return true;      
}
#endif

bool 
StringArg::load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg ) 
{
   // Read if more than one values exists.
   bool multi = buf.readNextBits( 1 ) != 0;
   if ( multi ) {
      // Multiple values.
      int nbrValues = buf.readNextBits( 5 );
      m_stringByScaleIdx.resize( nbrValues );
      buf.alignToByte();
      for ( int i = 0; i < nbrValues; ++i ) {
         m_stringByScaleIdx[ i ] = buf.readNextString();
      }
   } else {
      // Only one value.
      buf.alignToByte();
      m_stringByScaleIdx.push_back( buf.readNextString() );
   }
   return true;
}

TileFeatureArg* 
StringArg::clone()
{
   return new StringArg( m_name );
}

// ------------------------------- CoordArg -------------------------------

#ifdef MC2_SYSTEM
void 
CoordArg::setCoord( int32 lat, int32 lon, const TileMap& tileMap ) 
{
   TileMapCoord coord( lat, lon );
   // Snap the coordinate to a pixel in the tile map.
   tileMap.snapCoordToPixel( coord );
   
   m_coord = coord;
}

void 
CoordArg::dump( ostream& stream ) const 
{
   stream << "CoordArg " << endl;
   TileFeatureArg::dump( stream );
   stream << m_coord.getLat() << ", " << m_coord.getLon() << endl;
}

bool 
CoordArg::save( BitBuffer& buf, const TileMap& tileMap,
                const TileFeatureArg* prevArg ) const 
{
   // TODO: Doesn't use relative coordinates yet.
   int16 lat = ( m_coord.getLat() - 
         tileMap.getReferenceCoord().getLat() ) / 
      tileMap.getMC2Scale();
   int16 lon = ( m_coord.getLon() - 
         tileMap.getReferenceCoord().getLon() ) / 
      tileMap.getMC2Scale();
   buf.alignToByte();
   buf.writeNextBAShort( lat );
   buf.writeNextBAShort( lon );
   return true;
}
#endif

bool 
CoordArg::load( BitBuffer& buf, TileMap& tileMap,
                const TileFeatureArg* prevArg ) 
{
   // TODO: Doesn't use relative coordinates yet.
   buf.alignToByte();
   int16 diffLat = buf.readNextBAShort();
   int16 diffLon = buf.readNextBAShort();
   int32 lat = diffLat * tileMap.getMC2Scale() + 
      tileMap.getReferenceCoord().getLat();
   int32 lon = diffLon * tileMap.getMC2Scale() + 
      tileMap.getReferenceCoord().getLon();
   m_coord.setCoord( lat, lon );
   return true;
}

TileFeatureArg* 
CoordArg::clone() 
{
   return new CoordArg( m_name );
}

// ------------------------------- CoordsArg ------------------------------

#ifdef MC2_SYSTEM

inline void
CoordsArg::push_back( const TileMapCoord& coord )
{
   m_allCoords->push_back(coord);
   m_endCoordIdx = m_allCoords->size();
}

void 
CoordsArg::addCoord( int32 lat, int32 lon, TileMap& tileMap ) 
{
   if ( m_startCoordIdx == MAX_UINT32 ) {
      m_allCoords = &tileMap.coordsRef();
      m_startCoordIdx = m_allCoords->size();
      m_endCoordIdx = m_allCoords->size();
   }

   TileMapCoord coord( lat, lon );
   // Snap the coordinate to a pixel in the tile map.
   tileMap.snapCoordToPixel( coord );

   // Check that we're not adding duplicate coordinates.
   if ( size() > 0 ) {
      const TileMapCoord& lastCoord = back();
      if ( lastCoord != coord ) {
         // Not duplicated. Add.
         push_back( coord );
      }
   } else {
      // First coordinate. Add.
      push_back( coord );
   } 
}

bool 
CoordsArg::save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const 
{
   const TileMapCoord* referenceCoord = &(tileMap.getReferenceCoord());
   
   // Check if the previous coords last coordinate can be used as
   // reference.
   if ( prevArg != NULL ) {
      // Use previous feature's last coordinate as reference. 
      const CoordsArg* prevCoordsArg = 
         static_cast<const CoordsArg*> ( prevArg );
      MC2_ASSERT( prevCoordsArg->size() > 0 );
      referenceCoord = &(prevCoordsArg->back());
   }
  
   // Write the number of coordinates.
   int nbrBitsCoordSize = MathUtility::getNbrBits( size() );
   MC2_ASSERT( nbrBitsCoordSize < 16 );
   buf.writeNextBits( nbrBitsCoordSize, 4 );
   buf.writeNextBits( size(), nbrBitsCoordSize );

   if (  size() == 0 ) {
      return true;
   }
     
   mc2dbg8 << "CoordsArg::save" << endl;
   mc2dbg8 << "referenceCoord = "
      << referenceCoord->getLon()
      << ", " << 
      referenceCoord->getLat()
      << ", mc2scale = "
      << tileMap.getMC2Scale() 
      << endl;
    
   TileMapCoords::const_iterator it = begin();

   // Calculate the starting coordinate.
   int16 startLatDiff = ( it->getLat() - 
                 referenceCoord->getLat() ) / 
      tileMap.getMC2Scale();
   int16 startLonDiff = ( it->getLon() - 
                 referenceCoord->getLon() ) / 
      tileMap.getMC2Scale();
  
   // vector of lat/lon pairs.
   vector< pair< int16, int16 > > coordDiffs;
   coordDiffs.resize( size() - 1 );
   vector< pair< int16, int16 > >::iterator coordDiffIt = 
      coordDiffs.begin();

   TileMapCoords::const_iterator nextIt = it;
   ++nextIt;
   int16 maxLatDiff = 0;
   int16 maxLonDiff = 0;
   while ( nextIt != end() ) {
      // Calculate coordinate diffs.
      int16 latDiff = ( nextIt->getLat() - it->getLat() ) / 
            tileMap.getMC2Scale();
      int16 lonDiff = ( nextIt->getLon() - it->getLon() ) / 
            tileMap.getMC2Scale();

      // Check if latdiff is largest diff so far.
      int tmpDiff = latDiff;
      if ( latDiff >= 0 ) {
         tmpDiff = latDiff + 1;
      } else {
         tmpDiff = -latDiff;
      }

      if ( tmpDiff > maxLatDiff ) {
         maxLatDiff = tmpDiff;
      }
      
      // Check if londiff is largest diff so far.
      tmpDiff = lonDiff;
      if ( lonDiff >= 0 ) {
         tmpDiff = lonDiff + 1;
      } else {
         tmpDiff = -lonDiff;
      }

      if ( tmpDiff > maxLonDiff ) {
         maxLonDiff = tmpDiff;
      }

      // Add to the vector.
      coordDiffIt->first = latDiff;
      coordDiffIt->second = lonDiff;
      ++coordDiffIt;
      // Update the iterators.
      it = nextIt;
      ++nextIt;
   }

   // Nbr of bits needed for startdiff. +1 for the sign.
   int nbrBitsStartDiff = MathUtility::getNbrBitsSigned( 
         MAX( abs( startLatDiff ), abs( startLonDiff ) ) );
   MC2_ASSERT( nbrBitsStartDiff < 16 );
   buf.writeNextBits( nbrBitsStartDiff, 4 );
   // Write start lat/lon diff.
   buf.writeNextBits( startLatDiff, nbrBitsStartDiff );
   buf.writeNextBits( startLonDiff, nbrBitsStartDiff );
   
   mc2dbg8 << " nbrBitsStartDiff = " << nbrBitsStartDiff
      << ", startLatDiff = " << startLatDiff << ", startLonDiff = "
      << startLonDiff 
      << ", MAX( abs( startLatDiff ), abs( startLonDiff ) ) = "
      <<  MAX( abs( startLatDiff ), abs( startLonDiff ) ) << endl;
   
   // Calculate how many bits that are necessary for representing
   // the maximum coord diff.
   int bitsPerLatDiff = MathUtility::getNbrBitsSigned( maxLatDiff );

   int bitsPerLonDiff = MathUtility::getNbrBitsSigned( maxLonDiff );

   MC2_ASSERT( bitsPerLatDiff < 16 );
   MC2_ASSERT( bitsPerLonDiff < 16 );
   mc2dbg8 << "bitsPerLatDiff = " << bitsPerLatDiff << " ("
      << maxLatDiff << ")" << endl;
   mc2dbg8 << "bitsPerLonDiff = " << bitsPerLonDiff << " ("
      << maxLonDiff << ")" << endl;
   // Everything is calculated now. 
   // Write to the buffer.

   // The number of bits needed for lat diff
   buf.writeNextBits( bitsPerLatDiff, 4 );
   // The number of bits needed for lon diff
   buf.writeNextBits( bitsPerLonDiff, 4 );
   
   // Write all the other coord diffs.
   for ( vector< pair< int16, int16 > >::iterator it =
         coordDiffs.begin(); it != coordDiffs.end(); ++it ) {
      // lat diff
      buf.writeNextBits( it->first, bitsPerLatDiff );
      // lon diff
      buf.writeNextBits( it->second, bitsPerLonDiff );
      mc2dbg8 << "diff " << it->first << ", " << it->second << endl;
   }
   return true;
}

void 
CoordsArg::dump( ostream& stream ) const 
{
   stream << "CoordsArg " << endl;
   TileFeatureArg::dump( stream );
   stream << "Nbr coords " << size() << endl;
   for ( TileMapCoords::const_iterator it = begin(); 
         it != end(); ++it ) {
      stream << it->getLat() << ", " << it->getLon() << endl;
   }
}

#endif

static inline int sizeTrick(int size)
{
   static const int allowedSizes[] = { 128 };
   for ( int i = 0;
         i < int(sizeof(allowedSizes)/sizeof(allowedSizes[0]));
         ++i ) {
      if ( size < allowedSizes[i] ) {
         return allowedSizes[i];
      }
   }
   // Try the same again with half the size.
   return sizeTrick(size >> 1) << 1;
}

bool 
CoordsArg::load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg ) 
{
   const TileMapCoord* referenceCoord = &(tileMap.getReferenceCoord());

   // Check if the previous coords last coordinate can be used as
   // reference.
 
   if ( prevArg != NULL ) {
      // Use previous feature's last coordinate as reference. 
      const CoordsArg* prevCoordsArg = 
         static_cast<const CoordsArg*> ( prevArg );
      MC2_ASSERT( prevCoordsArg->size() > 0 );
      referenceCoord = &(prevCoordsArg->back());
   }

   // Write the number of coordinates.
   int nbrBitsCoordSize = buf.readNextBits( 4 );
   uint16 nbrCoords = buf.readNextBits( nbrBitsCoordSize );

   if ( nbrCoords == 0 ) {
      return true;
   }

   mc2dbg8 << "CoordsArg::load" << endl;
   mc2dbg8 << "referenceCoord = "
      << referenceCoord->getLon()
      << ", " << 
      referenceCoord->getLat()
      << ", mc2scale = "
      << tileMap.getMC2Scale() 
      << endl;

   // Nbr bits needed for start diff.
   int nbrStartDiffBits = buf.readNextBits( 4 );
   // Read startLatDiff.
   int16 startLatDiff = buf.readNextSignedBits( nbrStartDiffBits );
   // Read startLonDiff.
   int16 startLonDiff = buf.readNextSignedBits( nbrStartDiffBits );
   
   mc2dbg8 << "nbrStartDiffBits = " << nbrStartDiffBits << endl;
   mc2dbg8 << " startLatDiff = " << startLatDiff << ", startLonDiff = "
      << startLonDiff << endl;
   
   // Read bitsPerLatDiff.
   int bitsPerLatDiff = buf.readNextBits( 4 );
   // Read bitsPerLonDiff.
   int bitsPerLonDiff = buf.readNextBits( 4 );

   // Set the coordinates
   m_allCoords = &(tileMap.coordsRef());
   m_startCoordIdx = m_allCoords->size();
   m_endCoordIdx = m_allCoords->size() + nbrCoords;
   
   // Calculate the starting coordinate.
   int32 prevLat = startLatDiff * tileMap.getMC2Scale() +
      referenceCoord->getLat();
   int32 prevLon = startLonDiff * tileMap.getMC2Scale() +
      referenceCoord->getLon();
  
   m_allCoords->push_back( TileMapCoord( prevLat, prevLon ) );
   
   // Read the rest of the coords.
   for ( int i = 1; i < nbrCoords; ++i ) {
      // Lat
      int16 latDiff = buf.readNextSignedBits( bitsPerLatDiff );
      int32 curLat = latDiff * tileMap.getMC2Scale() + prevLat;
      // Lon
      int16 lonDiff = buf.readNextSignedBits( bitsPerLonDiff ); 
      int32 curLon = lonDiff * tileMap.getMC2Scale() + prevLon;
      mc2dbg8 << "diff " << latDiff << ", " << lonDiff << endl;
      mc2dbg8 << "coord " << curLat << ", " << curLon << endl;
      // Update prev coordinates.
      prevLat = curLat;
      prevLon = curLon;
      // Set the coord.
      m_allCoords->push_back( TileMapCoord( curLat, curLon ) );
   }

   // Oopendate de bundingboex.
   for( uint32 h = m_startCoordIdx; h < m_endCoordIdx; ++h ) {
      m_bbox.update( (*m_allCoords)[h].getLat(),
                     (*m_allCoords)[h].getLon(), false );
   }
   
   return true;
}

TileFeatureArg* 
CoordsArg::clone()
{
   return new CoordsArg( m_allCoords, m_name );
}

// -------------------------------- ltArgPtr ------------------------------

bool 
ltArgPtr::operator()( const TileFeatureArg* arg1, 
                      const TileFeatureArg* arg2 ) const
{
   if ( arg1->getArgType() < arg2->getArgType() ) {
      return true;
   } else if ( arg1->getArgType() > arg2->getArgType() ) {
      return false;
   } else {
      // Same type. 
      // Check the name.
      if ( arg1->getName() < arg2->getName() ) {
         return true;
      } else if ( arg1->getName() > arg2->getName() ) {
         return false;
      }
      
      // Same name.
      
      // Need to cast.
      switch ( arg1->getArgType() ) {
         case ( TileFeatureArg::simpleArg ) : {
            const SimpleArg* sarg1 = TileFeatureArg::simpleArgCast( arg1 );
            const SimpleArg* sarg2 = TileFeatureArg::simpleArgCast( arg2 );
            // Check size.
            if ( sarg1->getSize() < sarg2->getSize() ) {
               return true;
            } else if ( sarg1->getSize() > sarg2->getSize() ) {
               return false;
            } 
            // Same size. Check value.
            return sarg1->m_valueByScaleIdx < sarg2->m_valueByScaleIdx;
         } break;
         
         case ( TileFeatureArg::stringArg ) : {
            const StringArg* sarg1 = TileFeatureArg::stringArgCast( arg1 );
            const StringArg* sarg2 = TileFeatureArg::stringArgCast( arg2 );
            return sarg1->m_stringByScaleIdx < sarg2->m_stringByScaleIdx;
         } break;

         default :
            mc2log << fatal 
               << "ltArgPtr::operator() - Unsupported arg type "
               << int( arg1->getArgType() ) << endl;
            MC2_ASSERT( false ); 
            break;
      }
   }
   return false;
}

// ------------------------ TileArgContainer ------------------------------

TileArgContainer::~TileArgContainer()
{
   // We own the args in this container so delete them.
   for ( map< TileFeatureArg*, int >::iterator it = m_indexByArg.begin();
         it != m_indexByArg.end(); ++it ) {
      delete (*it).first;
   }
}

#ifdef MC2_SYSTEM
void 
TileArgContainer::save( BitBuffer& buf ) const
{
   buf.alignToByte();
   buf.writeNextBAShort( m_argByIndex.size() );
   for ( vector<TileFeatureArg*>::const_iterator it = m_argByIndex.begin();
         it != m_argByIndex.end(); ++it ) {
      (*it)->saveFullArg( buf ); 
   }
}
#endif

void 
TileArgContainer::load( BitBuffer& buf )
{
   buf.alignToByte();
   uint32 size = buf.readNextBAShort();
   m_argByIndex.resize( size );
   for ( uint32 i = 0; i < size; ++i ) {
      TileFeatureArg* arg = TileFeatureArg::loadFullArg( buf );
      m_argByIndex[ i ] = arg;
      m_indexByArg.insert( make_pair( arg, i ) );
   }
}
 
int 
TileArgContainer::addArg( TileFeatureArg* arg )
{
   // Check if the arg is already present or not.
   indexByArg_t::iterator findIt = m_indexByArg.find( arg );
   
   if ( findIt != m_indexByArg.end() ) {
      // Already present. 
      // Delete the arg and return the index.
      delete arg;
      return (*findIt).second;
   } else {
      // Not present. Add!
      int index = m_argByIndex.size();
      m_indexByArg.insert( make_pair( arg, index ) );
      m_argByIndex.push_back( arg );
      return index;
   }
}


TileFeatureArg* 
TileArgContainer::getArg( int index )
{
   MC2_ASSERT( index < (int) m_argByIndex.size() );
   return m_argByIndex[ index ];
}

int 
TileArgContainer::getArgIndex( TileFeatureArg* arg ) const
{
   indexByArg_t::const_iterator findIt = m_indexByArg.find( arg );
   
   if ( findIt != m_indexByArg.end() ) {
      return (*findIt).second;
   } else {
      // Not found.
      MC2_ASSERT( false );
      return -1;
   }
}



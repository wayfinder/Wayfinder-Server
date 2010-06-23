/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileImportanceTable.h"
#include "BitBuffer.h"

TileImportanceNotice::TileImportanceNotice( float64 detailLevel, 
                                            uint16 maxScale,
                                            uint16 type,
                                            uint32 threshold ) :
      m_detailLevel( detailLevel ),
      m_maxScale( maxScale ),
      m_type( type ), 
      m_threshold( threshold )
{
   
}

void 
TileImportanceNotice::load( BitBuffer& buf )
{   
   m_threshold = buf.readNextBALong();
   m_maxScale = buf.readNextBAShort();
   m_type = buf.readNextBAShort();
   m_detailLevel = buf.readNextBAByte() / 20.0 ;
}

void 
TileImportanceNotice::save( BitBuffer& buf ) const
{
   buf.writeNextBALong( m_threshold );
   buf.writeNextBAShort( m_maxScale );
   buf.writeNextBAShort( m_type );
   buf.writeNextBAByte( uint32( m_detailLevel * 20 ) );
}

void
TileImportanceNotice::dump( ostream& stream ) const
{
   stream << "TileImportanceNotice: detailLevel = " << m_detailLevel
          << ", type = " << m_type << ", threshold = " << m_threshold
          << ", maxScale = " << m_maxScale 
          << endl;
}

// ----------------------- TileImportanceTable ----------------------------

TileImportanceTable::~TileImportanceTable()
{
   // Delete the stuff in the matrix.
   for ( uint32 i = 0; i < m_importanceMatrix.size(); ++i ) {
      delete m_importanceMatrix[ i ];
   }
}

void 
TileImportanceTable::load( BitBuffer& buf )
{
   clear();
   uint16 nbrElems = buf.readNextBAShort();
   for ( uint16 i = 0; i < nbrElems; ++i ) {
      TileImportanceNotice notice;
      notice.load( buf );
      insert( make_pair( notice.getMaxScale(), notice ) );
   }
   // Build the matrix.
   buildMatrix();
}

void 
TileImportanceTable::save( BitBuffer& buf ) const
{
   buf.writeNextBAShort( size() );
   for ( const_iterator it = begin(); it != end(); ++it ) {
      it->second.save( buf );
   }
}

void
TileImportanceTable::getInterestingScales( vector<uint32>& scales ) const
{
   for ( const_iterator it = begin(); it != end(); ++it ) {
      scales.push_back( it->first );
   }
}


int
TileImportanceTable::getNbrImportanceNbrs( uint16 scale, 
                                           int detailLevel ) const
{
   int nbrImportance = 0;
   for ( const_reverse_iterator it = rbegin(); it != rend(); ++it ) {
      if ( (*it).first >= scale ) {
         if ( (*it).second.getDetailLevel() == detailLevel ||
              (*it).second.getThreshold() == MAX_UINT32 ) {
            ++nbrImportance;
         }
      } else {
         return nbrImportance;
      }
   }
   return nbrImportance;
}

const TileImportanceNotice*
TileImportanceTable::getImportanceNbrSlow( int importanceNbr,
                                       int detailLevel ) const
{
   const_reverse_iterator it = rbegin();
   int count = -1;
   while ( it != rend() ) {
      if ( ( (*it).second.getDetailLevel() == detailLevel ) ||
           ( (*it).second.getThreshold() == MAX_UINT32 ) ) {
         ++count;
         if ( count == importanceNbr ) {
            return &((*it).second);
         }
      }
      ++it;
   }

   // If we get here, we didn't find the importance notice.
   return NULL;
}      

const TileImportanceNotice*
TileImportanceTable::getImportanceNbr( int importanceNbr,
                                       int detailLevel ) const
{
   // Indata must not be incorrect now!
   return (*(m_importanceMatrix[ detailLevel ]))[ importanceNbr ];
}

void
TileImportanceTable::buildMatrix()
{

   if ( empty() ) {
      return;
   }
   
   int nbrDetailLevels = int((*rbegin()).second.getDetailLevel()) + 1;

   m_importanceMatrix.resize( nbrDetailLevels );
   for ( int i = 0; i < nbrDetailLevels; ++i ) {

      int nbrImportances = getNbrImportanceNbrs( 0, i );
      
      
      vector<const TileImportanceNotice*>* vecPtr = 
         new vector<const TileImportanceNotice*>();

      vecPtr->resize( nbrImportances );

      m_importanceMatrix[ i ] = vecPtr;
      for ( int j = 0; j < nbrImportances; ++j ) {
         // Use the slow method.
         (*vecPtr)[ j ] = getImportanceNbrSlow( j, i ); 
      }
   }
}

const TileImportanceNotice* 
TileImportanceTable::getFirstOfType( uint16 type ) const
{
   const_reverse_iterator it = rbegin();
   while ( it != rend() ) {
      if ( (*it).second.getType() == type ) {
         return &((*it).second);
      }
      ++it;
   }

   // If we get here, we didn't find the importance notice.
   return NULL;
}

void
TileImportanceTable::dump( ostream& stream ) const
{
   stream << "TileImportanceTable::dump" << endl;
   for ( TileImportanceTable::const_iterator it = begin(); it != end();
         ++it ) {
      stream << "ScaleLevel = " << (*it).first << ", ";
      (*it).second.dump( stream );
   }
}

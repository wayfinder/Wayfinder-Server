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

#include "BitBuffer.h"
#include "SFDMultiBufferWriter.h"
#include "TileMapParams.h"
#include "STLUtility.h"

#include <set>

// - - - - - - - - - SFDMultiBufferWriter - - - - - - - - -
namespace {
void addBuffer( vector<uint8>& dest,
                const SharedBuffer* buf )
{
   if ( buf != NULL ) {
      dest.insert( dest.end(),
                   buf->getBufferAddress(),
                   buf->getBufferAddress() + buf->getBufferSize() );
   }
                         
}

void addUint32( vector<uint8>& dest,
                uint32 val )
{
   BitBuffer tmp(4);
   tmp.writeNextBALong( val );
   addBuffer( dest, &tmp );
}

void addBufferAndLength( vector<uint8>& dest,
                         const SharedBuffer* buf )
{
   int bufLen = 0;
   if ( buf != NULL ) {
      bufLen = buf->getBufferSize();
   }
   addUint32( dest, bufLen );
   addBuffer( dest, buf );
}

void addString( vector<uint8>& dest,
                const MC2SimpleString& str )
{
   dest.insert( dest.end(),
                reinterpret_cast<const uint8*>(str.c_str()),
                reinterpret_cast<const uint8*>(str.c_str()) + str.length() + 1 );
               
}

uint32
addLayerHeader( std::vector<byte>& bytes,
                int layerID )
{
   bytes.push_back( layerID );   
   // Position for the imp-bits.
   uint32 retVal = bytes.size();
   // Room for the imp-bits.
   bytes.push_back( 0 );
   bytes.push_back( 0 );
   return retVal;
}

uint32
addImp( std::vector<byte>& bytes,
        uint32 imp_bits_offset, int imp )
{
   SharedBuffer buf( &bytes[imp_bits_offset], 2 );
   uint16 nbr = buf.readNextBAShort();
   buf.reset();
   // Now add the imp.
   buf.writeNextBAShort( nbr | ( 1 << imp ) );
   return nbr;   
}

class SFDMBSorter {
public:
   typedef pair<TileMapParams,BitBuffer*> pair_t;
   bool operator()( const pair_t& a,
                    const pair_t& b ) const {
      //
      // !! OBS !! 
      // Any change in this order must be reflected in the
      // TileCollectionIterator.
      // !! OBS !!
      //
      if ( a.first.getLayer() != b.first.getLayer() ) {
         return a.first.getLayer() < b.first.getLayer();
      }
      if ( a.first.getImportanceNbr() != b.first.getImportanceNbr() ) {
         return a.first.getImportanceNbr() < b.first.getImportanceNbr();
      }
      return a.first.getTileMapType() < b.first.getTileMapType();
   }
};


int
getNbrLayersPresent( const SFDMultiBufferWriter::toAddVect_t& maps )
{
   set<int> layers;
   SFDMultiBufferWriter::toAddVect_t::const_iterator it = maps.begin();
   SFDMultiBufferWriter::toAddVect_t::const_iterator it_end = maps.end();
   for ( ; it != it_end;  ++it ) {
      layers.insert( it->first.getLayer() );
   }
   return layers.size();
}

}

SFDMultiBufferWriter::SFDMultiBufferWriter( uint32 startOffset )
{
   m_startOffset = startOffset;
   m_buffer = NULL;
   m_curNbrOffset = MAX_UINT32;
}

                                       
void 
SFDMultiBufferWriter::writeParam( std::vector<byte>& bytes,
                                  const TileMapParams& dataParam, 
                                  const SharedBuffer* dataBuff,
                                  const TileMapParams& stringParam, 
                                  const SharedBuffer* stringBuff,
                                  int& lastLayer,
                                  uint32& impBitsOffset,
                                  bool addDebug ) 
{
   if ( stringBuff == NULL || 
        dataBuff == NULL ) {
      return;
   }

   MC2_ASSERT( stringParam.getTileMapType() == TileMapTypes::tileMapStrings );
   MC2_ASSERT( dataParam.getTileMapType() == TileMapTypes::tileMapData );

   int curLayer = stringParam.getLayer();
   if ( curLayer != lastLayer ) {
      impBitsOffset = addLayerHeader( bytes, curLayer );
      lastLayer = curLayer;
   }

   addImp( bytes, impBitsOffset, stringParam.getImportanceNbr() );


   addBufferAndLength( bytes, dataBuff );
   if ( addDebug ) {
      addString( bytes, dataParam.getAsString() );
   }

   addBufferAndLength( bytes, stringBuff );
   if ( addDebug ) {
      addString( bytes, stringParam.getAsString() );
   }

}

void SFDMultiBufferWriter::writeHeader( std::vector<byte>& bytes, int nbrLayers ) {
   // Write the number of layers.
   bytes.push_back( nbrLayers );
}

uint32
SFDMultiBufferWriter::writeAdded( bool addDebug ) {
   // Must be an even number of maps (1 data + 1 string)*n
   MC2_ASSERT( ! m_toAdd.empty() );
   MC2_ASSERT( ( m_toAdd.size() & 1 ) == 0 );
   
   // Format is:
   // Number of layers : 1 byte
   // For each layer:
   //    Layer id : 1 byte
   //    Present importances : 16 bits, imp 0 = bit 0
   //    For each imp set to one: 1 G-map and 1 T-map.
   
   // Sort the buffervector.
   std::sort( m_toAdd.begin(), m_toAdd.end(), SFDMBSorter() );

   // Start writing

   // Nbr layers
   int nbrLayers = getNbrLayersPresent( m_toAdd );

   writeHeader( m_bytes, nbrLayers );

   uint32 impBitsOffset = MAX_UINT32;

   int lastLayer = -1;
   for ( uint32 curMapIdx = 0; curMapIdx < m_toAdd.size(); curMapIdx += 2 ) {
      writeParam( m_bytes,
                  m_toAdd[curMapIdx].first,
                  m_toAdd[curMapIdx].second,
                  m_toAdd[curMapIdx + 1].first,
                  m_toAdd[curMapIdx + 1].second,
                  lastLayer,
                  impBitsOffset,
                  addDebug );
   }

   for_each( m_toAdd.begin(), m_toAdd.end(),
             STLUtility::DeleteMapValue() );

   m_toAdd.clear();
   
   return m_startOffset + m_bytes.size();
}

SFDMultiBufferWriter::~SFDMultiBufferWriter()
{
   delete m_buffer;
}

void
SFDMultiBufferWriter::addMap( const TileMapParams& params,
                              BitBuffer* buf )
{
   MC2_ASSERT( m_buffer == NULL );

   if ( buf != NULL ) {
      m_toAdd.push_back(
         toAddVect_t::value_type( params, buf ) );
   }
}

const SharedBuffer*
SFDMultiBufferWriter::createBuffer()
{
   m_buffer = new SharedBuffer( &m_bytes[0], m_bytes.size() );
   return m_buffer;
}

const SharedBuffer*
SFDMultiBufferWriter::getBuffer() const
{
   if ( m_buffer == NULL ) {
      return const_cast<SFDMultiBufferWriter*>(this)->createBuffer();
   } else {
      return m_buffer;
   }
}




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

#include "SimplePoiDesc.h"

#include "ServerTileMapFormatDesc.h"
#include "POIImageIdentificationTable.h"
#include "BitBuffer.h"

// -- SimplePoiDescTypeEntry

SimplePoiDescTypeEntry::
SimplePoiDescTypeEntry( uint32 type,
                        const MC2String& bitmap_name,
                        const MC2String& centered_bitmap_name )
      : m_type( type ),
        m_bitmapName( bitmap_name ),
        m_centeredBitmapName( centered_bitmap_name )
{
}

// -- SimplePoiDesc
const uint16 SimplePoiDesc::SPECIAL_FEATURE_OFFSET = 1024;

SimplePoiDesc::SimplePoiDesc( const ServerTileMapFormatDesc& tmfd,
                              const MC2String& urlbase,
                              const POIImageIdentificationTable& imageTable )

{
   m_urlBase = urlbase;

   // Make entries already
   typedef ServerTileMapFormatDesc::imageByType_t imageByType_t;
   const imageByType_t& imagesByType = tmfd.getUsedPoiNamesByType();
   for ( imageByType_t::const_iterator it = imagesByType.begin();
         it != imagesByType.end();
         ++it ) {
      MC2String bitmapName = it->second.c_str();
      m_typeEntries.push_back(
         SimplePoiDescTypeEntry( it->first,
                                 MC2String("b") + bitmapName,
                                 MC2String("B") + bitmapName ) );
   }

   // add special feature images
   vector<MC2String> codes;
   imageTable.getCodeTable( codes );
   for ( uint32 i = 0; i < codes.size(); ++i ) {
      m_typeEntries.
         push_back( SimplePoiDescTypeEntry( SPECIAL_FEATURE_OFFSET + i, 
                                            MC2String( "b" ) + codes[ i ],
                                            MC2String( "B" ) + codes[ i ] ) );
   }
   // add additional images from custom poi image table

}

SimplePoiDesc::~SimplePoiDesc()
{
}

void
SimplePoiDesc::save( BitBuffer& dest ) const
{
   // Magic
   const uint8 magic[] = { 'P', 'X', 'X', 'X' };
   dest.writeNextByteArray( magic, 4 );
   
   // Total size placeholder.
   BitBuffer lenBuf( dest.getCurrentOffsetAddress(), 4 );
   dest.writeNextBALong( 0 );

   // URL base for images
   dest.writeNextString( m_urlBase.c_str() );
   int nbrBytesPerType = sizeof ( uint16 ); // See SPECIAL_FEATURE_OFFSET
   // Number of bytes per type
   dest.writeNextByte( nbrBytesPerType );
   // Number of types   
   dest.writeNextBALong( m_typeEntries.size() );
   // Write the strings
   for ( typeEntryVector_t::const_iterator it = m_typeEntries.begin();
         it != m_typeEntries.end();
         ++it ) {
      // Type
      if ( nbrBytesPerType == 1 ) {
         dest.writeNextByte( it->getType() );
      } else {
         dest.writeNextBAShort( it->getType() );
      }

      // Write the bitmap name
      dest.writeNextString( it->getBitmapName().c_str() );
      // Write the alternative bitmap name (wrong center)
      dest.writeNextString( it->getCenteredBitmapName().c_str() );
   }

   // Update size
   lenBuf.writeNextBALong( dest.getCurrentOffset() );

   // Decrease the size to the same as the offset.
   dest.setSizeToOffset();

}

SharedBuffer*
SimplePoiDesc::getAsNewBytes()
{
   BitBuffer* buf = new BitBuffer( 10*1024*1024 );
   save( *buf );
   return buf;
}

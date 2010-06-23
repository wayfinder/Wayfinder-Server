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

#include "SFDSavableHeader.h"
#include "SharedBuffer.h"
#include "RouteID.h"
#include "TileMapParamTypes.h"
#include "TileCollectionNotice.h"
#include "TimeUtility.h"

#include <algorithm>


SFDSavableHeader::SFDSavableHeader( const MC2SimpleString& name, bool debug ) 
   : SFDHeader(),
     m_multiBufferOffsetStartOffset( 0 ),
     m_multiBufferStartOffset( 0 )
{

   m_name = name;
   m_readDebugParams = debug;
   // Include the debug param strings until we know for sure it's
   // working.
   //m_readDebugParams = true;
}

SFDSavableHeader::~SFDSavableHeader() 
{

}
   
int 
SFDSavableHeader::save( SharedBuffer& buf )
{
   /* Format is as follows:
    * STRING     - magic string (13 bytes)
    * BYTE       - version
    * BYTE       - encryption type
    * LONG       - header size
    * LONG       - file size
    * STRING     - name
    * LONG       - creation time
    * BYTE       - null terminated strings
    * BYTE       - maximum string size
    * BYTE       - initial characters size
    * BYTE array - initial chars 
    * BYTE       - nbr route ids
    * Array { 
    *  LONG      - route id
    *  LONG      - creation time
    * }
    * LONG       - number of bits for string index
    * LONG       - string index start offset
    * LONG       - number of strings
    * LONG       - start position of string data 
    * LONG       - start position of buffer size index 
    * LONG       - start position for buffer data
    * BYTE       - read debug params
    * SHORT      - tile collection size
    * Array {
    *   TileCollectionNotice 
    * }
    *
    */

   // Magic string.
   buf.writeNextString( "storkafinger" );

   // Version
   buf.writeNextBAByte( m_version );
  
   // Encryption type. FIXME
//   buf.writeNextBAByte( (byte) m_encryptionType );
   buf.writeNextBAByte( 0 ); // No encryption.
  
   // Header size. 
   int headerPos = buf.getCurrentOffset();
   buf.writeNextBALong( 0 ); // Fill in later.

   // File size.
   buf.writeNextBALong( m_fileSize );
   
   // The name.
   buf.writeNextString( m_name );  

   // Creation time.
   m_creationTime = TimeUtility::getRealTime();
   buf.writeNextBALong( m_creationTime );

   // Null terminated strings?
   buf.writeNextBAByte( m_stringsAreNullTerminated );

   // Longest length of string.
   buf.writeNextBAByte( m_maxStringSize );

   // Nbr initial chars.
   buf.writeNextBAByte( m_initialCharacters.size() );
   // Initial chars.
   {for ( byte b = 0; b < m_initialCharacters.size(); ++b ) {
      buf.writeNextBAByte( m_initialCharacters[ b ] );
   }}
   
   // Nbr route ids.
   buf.writeNextBAByte( m_routeIDs.size() );
   {for ( byte b = 0; b < m_routeIDs.size(); ++b ) {
      buf.writeNextBALong( m_routeIDs[ b ].getRouteIDNbr() );
      buf.writeNextBALong( m_routeIDs[ b ].getCreationTime() );
   }}

   // Number of bits for the string index.
   buf.writeNextBALong( m_strIdxEntrySizeBits );

   // Position for the start of strings index.
   buf.writeNextBALong( m_strIdxStartOffset );

   // Number strings.
   buf.writeNextBALong( m_nbrStrings );
   
   // Position for the start of string data.
   buf.writeNextBALong( m_strDataStartOffset );

   // Position for the start of the buffer size index.
   buf.writeNextBALong( m_bufferIdxStartOffset );

   // Position for when the buffer data starts.
   buf.writeNextBALong( m_bufferDataStartOffset );

   // This is the end of the binary search stuff.

   // If to read debug param strings for the multi buffers.
   buf.writeNextBAByte( m_readDebugParams );
    
   // The tile collections.
   buf.writeNextBAShort( m_tileCollection.size() );
   uint32 offset = m_multiBufferOffsetStartOffset;
   mc2dbg << "[SFDSH]: save - mb startOffsetOffset = " << offset << endl;
   for ( uint32 i = 0; i < m_tileCollection.size(); ++i ) {
      // Update the offsets in m_tileCollection.
      offset = m_tileCollection[ i ].updateOffset( offset );
      m_tileCollection[ i ].save( buf );
      mc2dbg << "[SFDSH]: save - next mb startOffsetOffset = " 
             << offset << endl;
   }
   
   // Store the offset for the multibuffer offset.
   m_multiBufferStartOffset = offset;
   
   // Now set the header size. 
   int curPos = buf.getCurrentOffset();
   buf.reset();
   buf.readPastBytes( headerPos );
   const_cast<SFDSavableHeader*>(this)->m_headerSize = curPos - headerPos;
   buf.writeNextBALong( m_headerSize );

   // Go back to the end of the buffer.
   buf.reset();
   buf.readPastBytes( curPos );

   return curPos;
}

void
SFDSavableHeader::updateMetaData( const MC2SimpleString& str )
{
   
   // Also include the NULL terminator.
   if ( str.length() + 1 > (uint32) m_maxStringSize ) {
      m_maxStringSize = str.length() + 1;
   }

   if ( str.length() == 0 ) {
      MC2_ASSERT( false );
      return;
   }
   
   // Update m_initialCharacters
   char ch = str[ 0 ];

   if ( std::find( m_initialCharacters.begin(),
                   m_initialCharacters.end(),
                   ch ) == m_initialCharacters.end() ) {
      // Initial character not found in vector. Add it.
      m_initialCharacters.push_back( ch );
   }

   if ( ! TileMapParamTypes::isMap( str.c_str() ) ) {
      // Nothing more to update for non tilemaps.
      return;
   }
   
   // Create params from the string.
   TileMapParams param( str );

   // Check route ids.
   if ( param.getRouteID() != NULL ) {
      if ( std::find( m_routeIDs.begin(), m_routeIDs.end(), 
                      *param.getRouteID() ) == m_routeIDs.end() ) {
         // Route id not found. Add.
         m_routeIDs.push_back( *param.getRouteID() );
      }
   }
}


void
SFDSavableHeader::writeFileSize( uint32 fileSize, SharedBuffer& buf ) {
   uint32 startPos = buf.getCurrentOffset();
   // Move to file pos
   buf.reset();
   buf.readPastBytes( 13 + 1 + 1 + 4 /*storkafinger,version,encryption,headerSize*/ );
   buf.writeNextBALong( fileSize );

   // Set position back to start pos
   buf.reset();
   buf.readPastBytes( startPos );
}

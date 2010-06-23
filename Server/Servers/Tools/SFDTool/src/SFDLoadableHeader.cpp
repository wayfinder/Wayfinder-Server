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

#include "SFDLoadableHeader.h"
#include "SharedBuffer.h"
#include "RouteID.h"
#include "TileCollectionNotice.h"

#define INITIAL_HEADER_SIZE 19


SFDLoadableHeader::SFDLoadableHeader( const MC2SimpleString& name, bool debug )
      : SFDSavableHeader( name, debug )
{
}

SFDLoadableHeader::~SFDLoadableHeader() 
{
}

bool
SFDLoadableHeader::load( SharedBuffer& buf )
{
   bool res = loadInitialHeader( buf );
   if ( res ) {
      res = loadRemainingHeader( buf );
   }

   return res;
}

bool
SFDLoadableHeader::loadInitialHeader( SharedBuffer& buf )
{
   // Load initial header.
   bool res = true;
   // Check size
   if ( buf.getBufferSize() < INITIAL_HEADER_SIZE ) {
      mc2log << error << "[SFDLH] Buffer size too small " 
             << buf.getBufferSize() << endl;
      res = false;
   }
   if ( res ) {
      const char* str = buf.readNextString();
      if ( strcmp( str, "storkafinger" ) != 0 ) {
         mc2log << error << "[SFDLH] not a SFD file!" 
                << buf.getBufferSize() << endl;
         res = false;
      }
   }

   if ( res ) {
      // Version
      m_version = buf.readNextBAByte();
  
      // Encryption type.
      m_encryptionType = encryption_t( buf.readNextBAByte() );
      MC2_ASSERT( m_encryptionType == no_encryption );

      // Header size.
      m_headerSize = buf.readNextBALong();
   }
   return res;
}

bool
SFDLoadableHeader::loadRemainingHeader( SharedBuffer& buf )
{
   bool res = true;
   // File size.
   m_fileSize = buf.readNextBALong();

   // The name.
   m_name = buf.readNextString();

   mc2dbg << "[SFDLH] m_name = " << m_name << endl;

   // Check the file size.
   if ( buf.getBufferSize() != uint32(m_fileSize) ) {
      mc2log << error << "[SFDLH] Buffer size not same as file size " 
             << buf.getBufferSize() << " != " << m_fileSize << endl;
      res = false;
   } else {
      mc2dbg << "[SFDLH] fileSize " << m_fileSize << endl;
   }
   
   if ( res ) {
      // Creation time.
      m_creationTime = buf.readNextBALong();

      // Null terminated strings?
      m_stringsAreNullTerminated = buf.readNextBAByte();

      // Longest length of string.
      m_maxStringSize = buf.readNextBAByte();

      // Nbr initial chars.
      byte nbrInitialChars = buf.readNextBAByte();
      m_initialCharacters.resize( nbrInitialChars );
      // Initial chars.
      {for ( byte b = 0; b < nbrInitialChars; ++b ) {
         m_initialCharacters[ b ] = buf.readNextBAByte();
      }}
      
      // Nbr route ids.
      byte nbrRouteIDs = buf.readNextBAByte();
      m_routeIDs.reserve( nbrRouteIDs );
      {for ( byte b = 0; b < nbrRouteIDs; ++b ) {
         uint32 id = buf.readNextBALong();
         uint32 creationTime = buf.readNextBALong(); 
         m_routeIDs.push_back( RouteID( id, creationTime ) );
      }}

      // Number of bits for the string index.
      m_strIdxEntrySizeBits = buf.readNextBALong();

      // Position for the start of strings index.
      m_strIdxStartOffset = buf.readNextBALong();

      // Number strings.
      m_nbrStrings = buf.readNextBALong();
   
      // Position for the start of string data.
      m_strDataStartOffset = buf.readNextBALong();

      // Position for the start of the buffers index.
      m_bufferIdxStartOffset = buf.readNextBALong();

      // Position for the start of the buffer data.
      m_bufferDataStartOffset = buf.readNextBALong();
  
      // If to read debug param strings for the multi buffers.
      m_readDebugParams = buf.readNextBAByte();
      mc2dbg << "[SFDLH] debug is " << m_readDebugParams << endl;
   
      // The tile collections.
      uint32 nbrCollections = buf.readNextBAShort();
      mc2dbg << "[SFDLH] tile collection size " << nbrCollections << endl;

      m_tileCollection.resize( nbrCollections );
      {for ( uint32 i = 0; i < nbrCollections; ++i ) {
         m_tileCollection[ i ].load( buf );
      }}
  
      // All is now loaded.
   }

   return res;
}

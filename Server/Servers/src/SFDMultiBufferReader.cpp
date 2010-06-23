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
#include "SFDMultiBufferReader.h"
#include "TileMapParams.h"
#include "TileCollectionNotice.h"
#include "SFDHeader.h"
#include "TileMapParamTypes.h"

// - - - - - - - - - SFDMultiBufferReader - - - - - - - - -

SFDMultiBufferReader::SFDMultiBufferReader( BitBuffer* biggie_smalls,
                                            const TileMapParams& param,
                                            const SFDHeader* header )
{
   m_bigBuf = biggie_smalls;
   
   m_nbrLayers = m_bigBuf->readNextBAByte();
   
   m_curLayer = 0;
   m_prototypeParam = param;
 
   m_impRange = NULL;
   m_header = header;

   m_hasNext = readNextMetaIfNeeded();
   mc2dbg2 << "[SFDMBR] bigBuf size " << m_bigBuf->getBufferSize() << endl;
}

bool
SFDMultiBufferReader::readNextMetaIfNeeded()
{
   mc2dbg2 << "[SFDMBR] readNextMetaIfNeeded curLayer " << m_curLayer
           << " nbrLayers " << m_nbrLayers << endl;
   if ( m_curLayer < m_nbrLayers ) {
      m_layerID = m_bigBuf->readNextBAByte();
      m_existingImps = m_bigBuf->readNextBAShort();
      
      // Get the importance range for this layer.
      TileMapParams tmpParam( m_prototypeParam );
      tmpParam.setLayer( m_layerID );
      m_impRange = m_header->getImportanceRange( tmpParam );
      MC2_ASSERT( m_impRange != NULL ); 

      m_curImp = m_impRange->m_firstImp;
      
      m_strings = false;
      
      return true; 
   } // else
   return false;
}


SFDMultiBufferReader::~SFDMultiBufferReader()
{
   // In server we have a buffer and it is not owned by reader
   // delete m_bigBuf;
}

bool
SFDMultiBufferReader::hasNext() const 
{
   return m_hasNext;
}

bool
SFDMultiBufferReader::moveToNext()
{
   // Toggle between strings or data map.
   m_strings = !m_strings;

   if ( m_strings ) {
      return true;
   } // Else move to next importance, or possibly layer.

   // Move to next importance.
   ++m_curImp;
   if ( m_curImp > m_impRange->m_lastImp ) {
      // The importance was out of range.
      // Move to next layer.
      ++m_curLayer;
      return readNextMetaIfNeeded();
   }
   return true;
}

SFDMultiBufferReader::bufPair_t&
SFDMultiBufferReader::getCurrent()
{
   return m_current;
}

// Method checks that the read param string is what was expected.
static bool checkParamStr( const char* readDebugParamStr, 
                           const TileMapParams& expectedParam )
{
   if ( ! TileMapParamTypes::isMap( readDebugParamStr ) ) {
      // Should have been a map.
      return false;   
   }
   MC2SimpleString tmp( readDebugParamStr );
   TileMapParams debugParam( tmp );

   // The language can differ for string tilemaps.
   if ( debugParam.getTileMapType() == TileMapTypes::tileMapStrings ) {
      debugParam.setLanguageType( expectedParam.getLanguageType() );
   }
   // Now check the strings.
   mc2dbg2 << "checkParamStr " << debugParam.getAsString().c_str()
          << " =? " << expectedParam.getAsString().c_str() << endl;
   return ( strcmp( debugParam.getAsString().c_str(), 
                    expectedParam.getAsString().c_str() ) == 0 );
}

SFDMultiBufferReader::bufPair_t&
SFDMultiBufferReader::readNext( LangTypes::language_t lang )
{

   TileMapTypes::tileMap_t ttype = m_strings ? TileMapTypes::tileMapStrings :
                                      TileMapTypes::tileMapData;

   m_current.first = TileMapParams( 9,      // Server prefix
                                    m_prototypeParam.useGZip(),
                                    m_layerID,
                                    ttype,
                                    m_curImp,
                                    lang,
                                    m_prototypeParam.getTileIndexLat(),
                                    m_prototypeParam.getTileIndexLon(),
                                    m_prototypeParam.getDetailLevel() );
   
   // Check if existing.
   bool existing = (m_existingImps >> m_curImp) & 0x1;
  
   BitBuffer* sean_combs = NULL;
   
   if ( existing ) {
      
      // Length of buffer
      uint32 bufLen = m_bigBuf->readNextBALong();
      mc2dbg2 << "[SFDMBR] length of buffer is " << bufLen << endl;

      // Read the map buffer.
      if ( bufLen > 0 ) {
         sean_combs = new BitBuffer( bufLen );
         mc2dbg4 << "[SFDMBR]:readNext: about to read " << bufLen << endl;
         m_bigBuf->readNextByteArray( sean_combs->getBufferAddress(),
                                      bufLen );
         mc2dbg4 << "[SFDMBR]:readNext: Read " << bufLen << endl;
      }

      if ( m_header->readDebugParams() ) {
         mc2dbg4 << "[SFDMBR]:readNext: about to read debug param. size " 
                << m_bigBuf->getBufferSize() << " pos " 
                << m_bigBuf->getCurrentOffset() << endl;
         const char* paramStr = m_bigBuf->readNextString();
         mc2dbg4 << "[SFDMBR]:readNext: Read " << paramStr << endl;
         MC2_ASSERT( checkParamStr( paramStr, m_current.first ) );
         paramStr = ""; // Make compiler happy? In compiles without assert and dbg as paramStr is not used then
      }
   }

   mc2dbg4 << "Moving to next" << endl;
   m_hasNext = moveToNext();   

   mc2dbg4 << "[SFDMBR]:readNext: Expected param " << m_current.first 
           << ", existing = " << existing << endl;

   m_current.second = sean_combs;

   return m_current;

}



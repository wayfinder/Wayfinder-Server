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

#include "TileMapClientSFDQuery.h"

#include "MemoryDBufRequester.h"
#include "BitBuffer.h"
#include "MathUtility.h"
#include "SFDSavableHeader.h"
#include "TileMapParamTypes.h"
#include "ServerTileMapFormatDesc.h"
#include "SFDMultiBufferWriter.h"
#include "TileCollectionNotice.h"
#include "TileCollectionIterator.h"
#include "TileMapParams.h"

#include "TempFile.h"
#include "File.h"

#include "Properties.h"

#include "STLUtility.h"
#include "DataBuffer.h"
#include "DataBufferUtil.h"

namespace {

/// creates a temp path string 
MC2String getTempPath() {
   const char* path = Properties::getProperty("SFDQUERY_TEMP_PATH");

   if ( path == NULL ) {
      const char* fallbackPropName = "TILE_MAP_CACHE_PATH";

      mc2dbg << "[TMCSFDQuery]: " 
             << "Using " << fallbackPropName 
             << " as temp path for temp file. " 
             << endl;

      path = Properties::getProperty( fallbackPropName );
      if ( path == NULL ) {
         mc2dbg << "[TMCSFDQuery]: fallback failed!, using /tmp/" << endl;
         path = "/tmp/";
      }
   }

   return path;
}

typedef std::pair< MC2SimpleString, TileMapQuery::bufVect_t > StringBuffPair;
typedef std::pair< StringBuffPair, StringBuffPair > BufferPair;
typedef std::list< BufferPair > BufferList;

/// @return true if pair is complete
inline bool isCompletePair( const BufferPair& pair ) {
  return ! ( pair.first.second.empty() || pair.second.second.empty() );
}

/// @return start of a complete pair
BufferList::iterator beginCompletePair( BufferList& blist ) {
   if ( blist.empty() )
      return blist.end();

   if ( isCompletePair( *blist.begin() ) )
      return blist.begin();

   return blist.end();
}

void copyBufVect( TileMapQuery::bufVect_t& other, 
                  const TileMapQuery::bufVect_t& buffers )
{
   if ( ! other.empty() ) {
      STLUtility::deleteAllSecond( other );
      other.clear();
   }

   other.reserve( buffers.size() );

   TileMapQuery::bufVect_t::const_iterator it = buffers.begin();
   TileMapQuery::bufVect_t::const_iterator it_end = buffers.end();
   for (; it != it_end; ++it ) {
      other.push_back( TileMapQuery::
                       bufVect_t::
                       value_type( it->first, 
                                   new BitBuffer( *it->second ) ) );
   }
   
}

/// Will sort the parameters based on TileCollection offsets.
struct ParamOffsetSorter
{
   ParamOffsetSorter( const TileCollectionNotice& collection ) 
            : m_collection( &collection ) {}

   bool operator() ( const TileMapParams& p1, 
                     const TileMapParams& p2 ) const 
   {
      int offset1 = m_collection->getOffset( p1 );
      int offset2 = m_collection->getOffset( p2 );
      if ( offset1 != offset2 ) {
         return offset1 < offset2;
      } // else
      
      if ( p1.getLayer() != p2.getLayer() ) {
         return p1.getLayer() < p2.getLayer();
      } // else 

      if ( p1.getTileMapType() != p2.getTileMapType() ) {
         return p1.getTileMapType() < p2.getTileMapType();
      } // else
      
      return p1.getImportanceNbr() < p2.getImportanceNbr();
   }
         
   const TileCollectionNotice* m_collection;
};

}



         
struct TileMapClientSFDQueryPriv {

   explicit TileMapClientSFDQueryPriv( const MC2SimpleString& name, 
                                       bool debug ):
      m_tileCollectionIt( NULL ),
      m_tileCollectionItEnd( NULL ),
      m_startSearchIt( NULL ),
      m_endSearchIt( NULL ),
      m_header( name, debug ),
      m_nbrWanted( 0 ),
      m_nbrDone( 0 ),
      m_lastLayer( -1 ),
      m_impBitsOffset( MAX_UINT32 ),
      m_prevOffset( -1 ),
      m_tempFile( NULL ),
      m_offsetBuffer( NULL ),
      m_headerSetup( false ),
      m_usingOtherParams( true ) { // iterating others first, always

      const MC2String path( getTempPath() );
      mc2dbg << "[TMCSFDQuery]: temp path is : " << path << endl;

      if ( File::mkdir_p( path ) == -1 ) {
         mc2log << error << 
            "[TMCSFDQuery]: Failed to create path: " << path << endl;
      }

      m_tempFile = new TempFile( "tmapBuffer", path );

      if ( ! m_tempFile->ok() ) {
         mc2log << error << "[TMCLSFDQuery]: Failed to create temp file. " 
                << " Error: " << strerror( errno ) << endl;
         MC2_ASSERT( false );
      }
   }

   ~TileMapClientSFDQueryPriv() {

      delete m_offsetBuffer;
      delete m_tempFile;
      delete m_tileCollectionIt;
      delete m_tileCollectionItEnd;
      delete m_startSearchIt;
      delete m_endSearchIt;
   }

   /// @return Current params from iterator
   TileMapQuery::paramVect_t::value_type getParam() const {
      if ( m_otherParamsIt != m_otherParams.end() ) {
         return *m_otherParamsIt;
      }

      return **m_tileCollectionIt;
   }

   /// increase the iterators one step
   void step() {
      // increase otherParam iterator first
      if ( m_otherParamsIt != m_otherParams.end() ) {
         ++m_otherParamsIt;  
      } else {
         ++(*m_tileCollectionIt);
      }
   }
   /// @return true if the iterators are at the end
   bool finished() {
      return m_otherParamsIt == m_otherParams.end() &&
         *m_tileCollectionIt == *m_tileCollectionItEnd;
   }

   /// flushes bytes to temp file
   void flush( int startOffset, std::vector<byte>& bytes ) {
     
      // TODO: This is expensive! Start by adding a 0 first and then set it here?
      bytes.insert( bytes.begin(), m_layers.size() );

      int bytesWritten = m_tempFile->
         write( &bytes.front(), bytes.size() );

      if ( bytesWritten == - 1 ) {
         mc2dbg << "[TMCSFDQuery]: failed to write to file: " 
                << m_tempFile->getTempFilename() 
                << ". Error: " << strerror( errno ) << endl;
         MC2_ASSERT( false );
      }

      uint32 dataOffset = startOffset + m_tempFile->getSize();
      
      m_offsetBuffer->writeNextBALong( dataOffset );

      bytes.clear();
      m_impBitsOffset = MAX_UINT32;
      m_lastLayer = -1;
      m_layers.clear();
   }

   /// collection of notices
   TileCollectionNotice m_tileCollection;

   /// current position in tile collection
   TileCollectionIterator* m_tileCollectionIt;
   /// end position in tile collection
   TileCollectionIterator* m_tileCollectionItEnd;
   /// start of search iterator when looking for bufers to add
   TileCollectionIterator* m_startSearchIt;
   /// end of search iterator when looking for buffers to add
   TileCollectionIterator* m_endSearchIt;

   /// holds other params (non maps) position
   TileMapQuery::paramSet_t::const_iterator m_otherParamsIt;
   /// holds other params
   TileMapQuery::paramSet_t m_otherParams;

   SFDSavableHeader m_header;

   uint32 m_nbrWanted; //< nbr of still wanted params
   uint32 m_nbrDone; //< number of params done

   set<int> m_layers; //< layer set to count nbr layers in each appendBuffer
   int m_lastLayer;
   uint32 m_impBitsOffset;

   int m_prevOffset; //< previous offset 

   TempFile* m_tempFile; //< the file to store G and T buffer datas
   SharedBuffer* m_offsetBuffer; //< holds offsets
   bool m_headerSetup; //< whether or not the header was setup
   std::vector<byte> m_bytes; //< holds data between flushes
   bool m_usingOtherParams; //< whether we are currently iterating otherParamsvector

   /**
    * Holding buffers until we get a pair that have buffers
    */
   ::BufferList m_holdingBuffers; 
   
};


DBufRequester&
TileMapClientSFDQuery::createCache()
{
   m_realCache = new MemoryDBufRequester( NULL, MAX_UINT32 );
   return *m_realCache;
}

TileMapClientSFDQuery::
TileMapClientSFDQuery( const ServerTileMapFormatDesc* mapDesc, 
                       const MC2SimpleString& name,
                       bool debug,
                       const MC2BoundingBox& bbox,
                       const set<int>& layers,
                       const set<MC2SimpleString>& extraParams,
                       bool useGzip,
                       LangTypes::language_t lang,
                       uint32 scale ):
   TileMapClientPrecacheQuery( paramSet_t(),
                               createCache(),
                               mapDesc ),
   m_priv( new TileMapClientSFDQueryPriv( name, debug ) )
{
   m_statusPrints = true;
   m_resultBuf = NULL;
   m_name = name;

   mapDesc->getAllNotices( m_priv->m_tileCollection,
                           m_priv->m_otherParams,
                           bbox,
                           layers,
                           useGzip,
                           lang,
                           scale,
                           &extraParams );

   m_priv->m_otherParamsIt = m_priv->m_otherParams.begin();
   m_priv->m_tileCollection.makeCompactDetail();
   m_priv->m_tileCollection.updateOffset( 0 );

   m_priv->m_tileCollectionIt = 
      new TileCollectionIterator( m_priv->m_tileCollection,
                                  m_mapDesc->getServerPrefix(),
                                  useGzip, lang,
                                  true );  

   m_priv->m_tileCollectionItEnd = 
      new TileCollectionIterator( *m_priv->m_tileCollectionIt );
   m_priv->m_tileCollectionItEnd->goToEnd();

   m_priv->m_startSearchIt = 
      new TileCollectionIterator( *m_priv->m_tileCollectionIt );
   m_priv->m_endSearchIt =
      new TileCollectionIterator( *m_priv->m_tileCollectionIt );
      
   m_priv->m_prevOffset = 0;

   {
      // calculate number of wanted params
      TileCollectionIterator it( *m_priv->m_tileCollectionIt, false );
      uint32 nbrWanted = m_priv->m_otherParams.size();
      for ( ; it != *m_priv->m_tileCollectionItEnd; 
            ++it, ++nbrWanted ) {
      }
      m_priv->m_nbrWanted = nbrWanted;
      mc2dbg << "[TMCSFDQuery]: nbr of wanted = " << m_priv->m_nbrWanted << endl;
   }

   m_priv->m_nbrDone = 0;

}

TileMapClientSFDQuery::~TileMapClientSFDQuery()
{
   delete m_priv;
   delete m_realCache;
   delete m_resultBuf;
}


static void addString( vector<uint8>& dest,
                       const MC2SimpleString& str,
                       bool addZero = true )
{
   int extra = addZero ? 1 : 0;
   dest.insert( dest.end(),
                reinterpret_cast<const uint8*>(str.c_str()),
                reinterpret_cast<const uint8*>(str.c_str()) +
                str.length() + extra );
}

static void addBuffer( vector<uint8>& dest,
                       const SharedBuffer& buf,
                       uint32 size = MAX_UINT32 )
{
   if ( size == MAX_UINT32 ) {
      size = buf.getBufferSize();
   }
   dest.insert( dest.end(),
                buf.getBufferAddress(),
                buf.getBufferAddress() + size );
}

static void addBuffer( vector<uint8>& dest,
                       const vector<uint8>& src )
{
   dest.insert( dest.end(), src.begin(), src.end() );
}

static void writeOffsets( BitBuffer& buf,
                          map<MC2SimpleString, uint32> posMap,
                          uint32 endOffset )
{
   // Create string offset buffer.
   for ( map<MC2SimpleString, uint32>::const_iterator it = posMap.begin();
         it != posMap.end();
         ++it ) {
      buf.writeNextBALong( it->second );
   }
   // Length is calculated by comparing two adjacent values
   // so there has to be one last in the file.
   buf.writeNextBALong( endOffset );
}


void
TileMapClientSFDQuery::
createBinarySearchBufferAndUpdateHeader( SFDSavableHeader& header,
                                         const paramSet_t& params )
{

   // 1 if we should write the terminator.
   int writeTerminator = 1;
   typedef map<MC2SimpleString, uint32> stringPosMap_t;
   stringPosMap_t stringPosMap;

   stringPosMap_t bufPosMap;
   
   vector<uint8> mapBuf;
   vector<uint8> strBuf;

   uint32 maxStringSize = 0;
   for ( paramSet_t::const_iterator it = params.begin();
         it != params.end();
         ++it ) {
      BitBuffer* curBuf = m_realCache->requestCached( *it );
      if ( curBuf == NULL ) {
         mc2dbg4 << "[TMCSFDQ]: Adding NULL buffer for " << *it
                 << endl;
      } else {
         mc2dbg4 << "[TMCSFDQ]: Adding buffer for " << *it
                 << endl;
      }
      if ( curBuf ) {
         // Save the startpos and add the string to the strbuffer.
         stringPosMap.insert( make_pair( *it, strBuf.size() ) );
         addString( strBuf, *it, writeTerminator );
         
         header.updateMetaData( *it );
        
         // Save the startpos and add the real buffer to the bufbuffer.
         bufPosMap.insert( make_pair( *it, mapBuf.size() ) );
         addBuffer( mapBuf, *curBuf );

         maxStringSize = MAX( it->length(), maxStringSize );
      }
      // Delete buffer. Should not be used anymore.
      // If the cache is a memory cache, it is now destroyed.
      delete curBuf;
   }

   mc2dbg << "[TMCSFDQ]: Max string offset = "
          << strBuf.size() << endl;
   mc2dbg << "[TMCSFDQ]: Max buffer offset = "
          << mapBuf.size() << endl;
   
   
   // Create string offset buffer.
   BitBuffer str_offset_buf( stringPosMap.size() * 4 + 4 );
   writeOffsets( str_offset_buf, stringPosMap, strBuf.size() );
   
   // Create buffer offset buffer.
   BitBuffer buf_offset_buf( bufPosMap.size() * 4 + 4 );
   writeOffsets( buf_offset_buf, bufPosMap, mapBuf.size() );

   // FIXME: Create the header too.

   // Write it all to our storage buffer.
   m_storage.clear();
   m_storage.reserve( str_offset_buf.getCurrentOffset() +
                      strBuf.size() + 
                      buf_offset_buf.getCurrentOffset() +
                      mapBuf.size() );

   SharedBuffer header_buf(10*1024);

   // FIXME: Add variable data before finding out the size.
   
   // First find out the size of the header.
   uint32 headerSize = header.save( header_buf );
   header_buf.reset();

   header.m_stringsAreNullTerminated = writeTerminator;
   header.m_strIdxEntrySizeBits = 4*8;
   header.m_strIdxStartOffset = headerSize;
   header.m_nbrStrings = stringPosMap.size();
   header.m_strDataStartOffset = header.m_strIdxStartOffset +
      str_offset_buf.getCurrentOffset();
   header.m_bufferIdxStartOffset = header.m_strDataStartOffset +
      strBuf.size();
   header.m_bufferDataStartOffset = header.m_bufferIdxStartOffset +
      buf_offset_buf.getCurrentOffset();
   header.m_maxStringSize = maxStringSize + writeTerminator;

   // This is not binary stuff any more.
   header.m_multiBufferOffsetStartOffset = 
      header.m_bufferDataStartOffset + mapBuf.size();

   // Update buf_offset_buf with m_bufferDataStartOffset.
   for ( stringPosMap_t::iterator it = bufPosMap.begin();
         it != bufPosMap.end(); ++it ) {
      it->second += header.m_bufferDataStartOffset;
   }
   buf_offset_buf.reset();
   writeOffsets( buf_offset_buf, bufPosMap, 
                 header.m_bufferDataStartOffset + mapBuf.size() );
   
   // header.m_fileSize is not yet set.

   mc2dbg << "[TMCSFDQ]: 1 Header size = " << header.m_headerSize << endl;
   mc2dbg << "[TMCSFDQ]: 1 File size = " << header.m_fileSize << endl;
   
   uint32 newHeaderSize = header.save( header_buf );
   
   mc2dbg << "[TMCSFDQ]: 2 Header size = " << header.m_headerSize << endl;
   mc2dbg << "[TMCSFDQ]: 2 File size = " << header.m_fileSize << endl;

   MC2_ASSERT( newHeaderSize == headerSize );
   
   // FIXME:
   // Now we use some memory...
   addBuffer( m_storage, header_buf, header_buf.getCurrentOffset() );
   addBuffer( m_storage, str_offset_buf, str_offset_buf.getCurrentOffset() );
   addBuffer( m_storage, strBuf );
   addBuffer( m_storage, buf_offset_buf, buf_offset_buf.getCurrentOffset() );
   addBuffer( m_storage, mapBuf );
   
}


int
TileMapClientSFDQuery::getNextParams( paramVect_t& params, int maxNbr )
{
   if ( m_priv == NULL ) {
      return TileMapClientPrecacheQuery::getNextParams( params, maxNbr );
   } else if ( isDone() ) {
      mc2dbg << "[TMCSFDQuery]: tileCollectionIt reached end." << endl;
      return 0;
   }

   // assuming params empty
   MC2_ASSERT( params.empty() );

   if ( m_priv->m_otherParamsIt == m_priv->m_otherParams.end() ) {
      m_priv->m_usingOtherParams = false;

      if ( ! m_priv->m_holdingBuffers.empty() ) {

         // we still have some old buffers that needs to be
         // filled so we add them to our params again

         ::BufferList::iterator it = m_priv->m_holdingBuffers.begin();
         ::BufferList::iterator itEnd = m_priv->m_holdingBuffers.end();
         for (; it != itEnd; ++it ) {
            if ( (*it).first.second.empty() ) {
               params.push_back( (*it).first.first );
               maxNbr--;
            }

            if ( (*it).second.second.empty() ) {
               params.push_back( (*it).second.first );
               maxNbr--;
            }
         }

      } else {
         // only update start and end iterator if we have 
         // no lingering buffers 
         *m_priv->m_startSearchIt = 
            TileCollectionIterator( *m_priv->m_tileCollectionIt, false );

         // End iterator should be at the beginning of the next
         // importance cycle
         *m_priv->m_endSearchIt =
            TileCollectionIterator( *m_priv->m_startSearchIt, true );
         ++(*m_priv->m_endSearchIt);
      }

      if ( maxNbr < 0 ) {
         maxNbr = 0;
      }

      // advance end iterator to end of our search window which is
      // used in appendBuffers
      for (int i = 0; i < maxNbr; ++i, ++(*m_priv->m_endSearchIt) ) {
         if ( *m_priv->m_endSearchIt == *m_priv->m_tileCollectionItEnd ) {
            break;
         }
      }

      // get even number of params
      maxNbr = maxNbr / 2 * 2;
      for ( int i = 0; i < maxNbr && ! m_priv->finished() ; i += 2 ) {

         MC2SimpleString dataParam( m_priv->getParam() );
         m_priv->step();
         
         MC2SimpleString stringParam( m_priv->getParam() );
         m_priv->step();

         params.push_back( dataParam );
         params.push_back( stringParam );
         
         m_priv->m_holdingBuffers.
            push_back( make_pair( make_pair( dataParam, bufVect_t() ),
                                  make_pair( stringParam, bufVect_t() ) ) );

      }

   } else {
      params.push_back( m_priv->getParam() );
      m_priv->step();
      m_priv->m_usingOtherParams = true;
   }

   return params.size();

}

const SharedBuffer*
TileMapClientSFDQuery::createBuffer()
{

   MC2_ASSERT( m_priv->m_holdingBuffers.empty() );

   // flush the last pieces of data
   m_priv->flush( m_priv->m_header.m_multiBufferStartOffset,
                  m_priv->m_bytes );

   // write the finishing bytes to file

   MC2_ASSERT( m_priv->m_offsetBuffer->bufferFilled() );

   // First store offsetBuffer in "output"
   addBuffer( m_storage, *m_priv->m_offsetBuffer );
    
   // create a temp fil to store the result in
   TempFile tmpFile("sfdbuffer", ::getTempPath() );
   if ( ! tmpFile.ok() ) {
      mc2log << fatal << "[TMCSFDQuery]: could not create temp file: " 
             << tmpFile.getTempFilename() 
             << ". Error: " << strerror( errno ) << endl;
      MC2_ASSERT( false );
   }

   //
   // write header
   //
   {
      // Make new buffer with current "output"
      SharedBuffer tmpBuff( &m_storage.front(), m_storage.size() );

      m_priv->m_header.m_fileSize = m_storage.size() +
         m_priv->m_tempFile->getSize();
   
      m_priv->m_header.save( tmpBuff );

      // move the internal position of the buffer to the end
      tmpBuff.reset();
      tmpBuff.readNextByteArray( tmpBuff.getBufferSize() );

      DataBufferUtil::saveBuffer( tmpBuff, tmpFile.getFD() );
   
   }

   //
   // append data/string buffers to the complete sfd buffer
   //
   {
      // mem map buffers temp file and append it to the other temp file
      DataBuffer tmpBuff;
      tmpBuff.memMapFile( m_priv->m_tempFile->getTempFilename().c_str() );
      // move the internal position to the end
      tmpBuff.readNextByteArray( m_priv->m_tempFile->getSize() );

      DataBufferUtil::saveBuffer( tmpBuff, tmpFile.getFD() );
   }
   
   // mem map the temp file to result buffer
  
   DataBuffer* dbuf = new DataBuffer();
   dbuf->memMapFile( tmpFile.getTempFilename().c_str() );
   delete m_resultBuf;
   m_resultBuf = dbuf;
   dbuf = NULL;

   m_resultBuf->reset();

   return m_resultBuf;
   
}

#if 0
// This method is not used! See createBuffer() above.
template < typename T >
const SharedBuffer*
TileMapClientSFDQuery::createBuffer( TileCollectionNotice& collection,
                                     const paramSet_t& otherParams,
                                     T paramsBegin,
                                     T paramsEnd ) {
   mc2dbg << "[TMCSFDQ]: createBuffer" << endl;
      
   // The params are now in the order that they should be in the buffer.

   // Create the header and add the tilecollection.
   SFDSavableHeader header( m_name );
   header.m_tileCollection.push_back( collection );

   // Create the binary search buffer and update the header.
   createBinarySearchBufferAndUpdateHeader( header, otherParams );   
   
   mc2dbg4 << "[TMCSFDQ]: Binary search buffer created. Nbr params = " 
          << otherParams.size() << endl;
  
   // Save the header once more, now since the binary search
   // data has been added to the header.
   SharedBuffer tmpBuf( 10*1024 );
   header.save( tmpBuf );
 
   mc2dbg4 << "[TMCSFDQ]: header.m_multiBufferStartOffset = " 
          << header.m_multiBufferStartOffset << endl;
   mc2dbg4 << "[TMCSFDQ]: header.m_multiBufferOffsetStartOffset = "
          << header.m_multiBufferOffsetStartOffset << endl;
   uint32 dataStartOffset = header.m_multiBufferStartOffset;

   // Contains the offsets to the multibuffers.
   SharedBuffer offsetBuf( header.m_multiBufferStartOffset - 
                           header.m_multiBufferOffsetStartOffset );

   // Contains the multi buffers.
   SFDMultiBufferWriter multiBuf( dataStartOffset );

   offsetBuf.writeNextBALong( dataStartOffset );
   // Must be -1 which is used below.
   int prevOffset = -1;

   for (T it = paramsBegin ; it != paramsEnd; ++it ) {
      const TileMapParams param( *it );

      // Request the tilemaps.
      BitBuffer* curBuf = m_realCache->requestCached( param );
      if ( curBuf == NULL ) {
         mc2dbg4 << "[TMCSFDQ]: Adding NULL buffer for " << param.getAsString()
                 << endl;
      } else {
         mc2dbg4 << "[TMCSFDQ]: Adding buffer for " << param.getAsString()
                 << endl;
      }
      
      // Get the offset.
      int curOffset = collection.getOffset( param );
      MC2_ASSERT( curOffset >= 0 );
      mc2dbg4 << "[TMCSFDQ]: collection.getOffset( "
             << param.getAsString() << " ) = "
             << curOffset << endl;
      
      MC2_ASSERT( prevOffset <= curOffset );
      if ( curOffset != prevOffset ) {
         // Offset has changed. Means that there will be a new multibuffer for
         // this offset. Add to the offset buffer.
         // But don't add it the first time the offset changes.
         if ( prevOffset != -1 ) {
            uint32 dataOffset = 
               multiBuf.writeAdded( header.readDebugParams() );
            offsetBuf.writeNextBALong( dataOffset );
         }
         // Update offset.
         prevOffset = curOffset;
      }
      
      // curBuf will be deleted by addMap.
      multiBuf.addMap( param, curBuf );
   }

   // Write the last offset. Needed since current and next offset is used to
   // calculate the size.
   {
      uint32 lastOffset = multiBuf.writeAdded( header.readDebugParams() );
      offsetBuf.writeNextBALong( lastOffset );
      mc2dbg << "[TMCSFDQ]: Last data offset " << lastOffset << endl;
   }
      
   MC2_ASSERT( offsetBuf.bufferFilled() );
   mc2dbg << "[TMCSFDQuery]: offsetBuffer size " 
          << offsetBuf.getBufferSize() << endl;

   // Add to storage.
   addBuffer( m_storage, offsetBuf );
   addBuffer( m_storage, *multiBuf.getBuffer() );
  
   // Update the file size.
   header.m_fileSize = m_storage.size();

   delete m_resultBuf;
   m_resultBuf = new SharedBuffer( &m_storage.front(), m_storage.size() );
  
   // Save the header for the last time!
   header.save( *m_resultBuf );
   m_resultBuf->reset();
   
   return m_resultBuf;
}
#endif

const SharedBuffer*
TileMapClientSFDQuery::getResult() const
{
   if ( m_resultBuf ) {
      return m_resultBuf;
   } else {
      return const_cast<TileMapClientSFDQuery*>(this)->createBuffer();
   }
}

bool 
TileMapClientSFDQuery::isDone() const {
   if ( m_priv == NULL ) {
      return TileMapClientPrecacheQuery::isDone();
   }

   return m_priv->m_holdingBuffers.empty() && m_priv->finished();
}

int TileMapClientSFDQuery::addBuffers( const bufVect_t& buffs ) {

   if ( m_priv == NULL ) {
      return TileMapClientPrecacheQuery::addBuffers( buffs );
   }

   //
   // Using previous iterator, since the current one is already
   // in the "next" importance cycle.
   //
   const TileCollectionIterator startIt( *m_priv->m_startSearchIt,
                                         false ); // use all importances
   if ( startIt == *m_priv->m_tileCollectionItEnd ) {
      mc2dbg << "[TMCSFDQuery]: not adding anything more." << endl;
      return 0;
   }

   // this is the real buffer we are going to add
   bufVect_t toAdd;


   //
   // Find buffers in generated params and add those 
   // that we find to "toAdd" buffer
   //


   bufVect_t::const_iterator bufIt = buffs.begin();
   bufVect_t::const_iterator bufItEnd = buffs.end();
   for (; bufIt != bufItEnd; ++bufIt ) {
         
      // add non-maps
      if ( ! TileMapParamTypes::isMap( bufIt->first.c_str() ) ) {
         toAdd.push_back( *bufIt );
         continue;
      }

      bool added = false; // whether a param was added
      // Special case for "other" param.
      //
      // if usingOtherParams it and the other paramsvector 
      // is not empty then we are still creating new params 
      // with the m_otherParams in getNextParams(). 
      // We need to go through the other params vector and
      // add the buffers.
      //
      if ( ! m_priv->m_otherParams.empty() &&
           m_priv->m_usingOtherParams ) {
         paramSet_t::const_iterator otherIt = m_priv->m_otherParams.begin();
         paramSet_t::const_iterator otherItEnd = m_priv->m_otherParams.end();
         for (; otherIt != otherItEnd; ++otherIt ) {
            if ( *otherIt == bufIt->first ) {
               toAdd.push_back( *bufIt );
               added = true;
               break;
            }
         }
      }

      // if it was not added in the other params, then check the other range
      if ( ! added ) {
         // search until we reach end search iterator or
         // find a matching param
         TileCollectionIterator it( startIt );
         for (; it != *m_priv->m_endSearchIt; ++it ) {
            if ( (*it).getAsString() == bufIt->first ) {
               toAdd.push_back( *bufIt );
               break;
            }
         }
      }

   }

 
   // if we added buffers, increase the iterator
   if ( ! buffs.empty() ) {
      if ( toAdd.empty() ) {
         mc2dbg << "[TMCSFDQuery]: toAdd empty, try with the same params"
                << " again. " << endl;
      }
   }

   m_priv->m_nbrDone += toAdd.size();

   if ( ! toAdd.empty() && 
         m_priv->m_usingOtherParams ) {
      return internalAddBuffers( toAdd );
   }

   // not using other params
   appendBuffers( toAdd );
   printStatus();

   return toAdd.size();

}

uint32 TileMapClientSFDQuery::getNbrDone() const {
   if ( m_priv == NULL ) {
      return TileMapClientPrecacheQuery::getNbrDone();
   }

   return m_priv->m_nbrDone;
}

uint32 TileMapClientSFDQuery::getNbrStillWanted() const {
   if ( m_priv == NULL ) {
      return TileMapClientPrecacheQuery::getNbrStillWanted();
   }

   return m_priv->m_nbrWanted - m_priv->m_nbrDone;
}

void TileMapClientSFDQuery::createHeader() {
   MC2_ASSERT( m_priv );

   mc2dbg4 << "[TMCSFDQuery]: createHeader" << endl;

   // The params are now in the order that they should be in the buffer.
   
   // Create the header and add the tilecollection.
   m_priv->m_header.m_tileCollection.push_back( m_priv->m_tileCollection );
   
   createBinarySearchBufferAndUpdateHeader( m_priv->m_header,
                                            m_priv->m_otherParams );
   
   // create a dummy buffer so we update offset and stuff in SFDSavableHeader
   SharedBuffer tmpBuf( 10 * 1024 );
   m_priv->m_header.save( tmpBuf );

   int dataStartOffset = m_priv->m_header.m_multiBufferStartOffset; 
   m_priv->m_offsetBuffer = 
      new SharedBuffer( dataStartOffset - 
                        m_priv->m_header.m_multiBufferOffsetStartOffset );

   m_priv->m_offsetBuffer->writeNextBALong( dataStartOffset );

   m_priv->m_headerSetup = true;
}

void TileMapClientSFDQuery::appendBuffers( const bufVect_t& buffers ) {
   // nothing to do here
   if ( buffers.empty() )
      return;

   // search through the holding buffers and find matching string/data
   ::BufferList::iterator it = m_priv->m_holdingBuffers.begin();
   const ::BufferList::iterator it_end = m_priv->m_holdingBuffers.end();
   for (; it != it_end; ++it ) {
      if ( buffers[ 0 ].first == (*it).first.first ) {
         ::copyBufVect( (*it).first.second, buffers );
         break;
      } else if ( buffers[ 0 ].first == (*it).second.first ) {
         ::copyBufVect( (*it).second.second, buffers );
         break;
      }
   }

   if ( m_priv->m_holdingBuffers.empty() )
      return;

   // now see if we have any buffer pairs that are ready to 
   // be written to disk
   it = ::beginCompletePair( m_priv->m_holdingBuffers );
   ::BufferList::iterator lastIt = it_end;
   int distance = 0;
   for (; it != it_end; ++it ) {
      if ( ::isCompletePair( *it ) ) {
         appendBuffers( (*it).first.second, 
                        (*it).second.second );
         lastIt = it;
         distance++;

      } else {
         // we must have complete pair in order
         break;
      }
   }

   // advance search index if we added buffers
   advance( *m_priv->m_startSearchIt, distance );

   // now we destroy all the complete pair in the lists
   if ( lastIt != it_end ) {
      ++lastIt;
      m_priv->m_holdingBuffers.erase( m_priv->m_holdingBuffers.begin(),
                                      lastIt );
      
   }
}

void TileMapClientSFDQuery::appendBuffers( bufVect_t& dataBuffer,
                                           bufVect_t& stringBuffer ) {

   // cannot be empty nor different size
   MC2_ASSERT( !dataBuffer.empty() );
   MC2_ASSERT( !stringBuffer.empty() );
   if ( stringBuffer.size() != dataBuffer.size() ) {
      mc2dbg << "[TMCSFDQ] string buffer (" << stringBuffer.size()
             << "): " << endl;
      for ( size_t i = 0; i < stringBuffer.size(); ++i ) {
         mc2dbg << stringBuffer[ i ].first << ", ";
      }
      mc2dbg << endl;
      mc2dbg << "[TMCSFDQ] data buffer(" << dataBuffer.size()
             <<"): " << endl;
      for ( size_t i = 0; i < dataBuffer.size(); ++i ) {
         mc2dbg << dataBuffer[ i ].first << ", ";
      }
      mc2dbg << endl;
      MC2_ASSERT( stringBuffer.size() == dataBuffer.size() );
   }


   MC2_ASSERT( m_priv );


   // setup header once we got otherParams buffers
   if ( ! m_priv->m_headerSetup ) {
      createHeader();
   }

   MC2_ASSERT( m_priv->m_offsetBuffer );

   mc2dbg4 << "Writing to file: " << dataBuffer[ 0 ].first 
           << "/" << stringBuffer[ 0 ].first << endl;


   std::vector<byte>& bytes = m_priv->m_bytes;

   if ( bytes.empty() ) {
      m_priv->m_impBitsOffset = MAX_UINT32;
   }

   for ( uint32 i = 0; i < stringBuffer.size(); ++i ) {

      if ( ! shouldBeAdded( dataBuffer[ i ] ) ) {
         delete dataBuffer[ i ].second;
         dataBuffer[ i ].second = NULL;
         delete stringBuffer[ i ].second;
         stringBuffer[ i ].second = NULL;
         continue;
      }

      TileMapParams dataParam( dataBuffer[ i ].first );


      int curOffset = m_priv->
         m_tileCollection.getOffset( dataParam );

      // the new offset can not be lower than previous offset
      MC2_ASSERT( m_priv->m_prevOffset <= curOffset );

      // flush buffer if offset changed
      if ( curOffset != m_priv->m_prevOffset ) {
         //
         // do not flush on the first round.
         //
         if ( m_priv->m_prevOffset != -1 ) {
            //
            // flush buffer to file
            //
            m_priv->flush( m_priv->m_header.m_multiBufferStartOffset, 
                           bytes );
         }
         m_priv->m_prevOffset = curOffset;
      }

      // add layer to we know how many layers we use in each round
      m_priv->m_layers.insert( dataParam.getLayer() );

      SFDMultiBufferWriter::
         writeParam( bytes,
                     dataParam, dataBuffer[ i ].second,
                     TileMapParams( stringBuffer[ i ].first ),
                     stringBuffer[ i ].second,
                     m_priv->m_lastLayer, 
                     m_priv->m_impBitsOffset, 
                     m_priv->m_header.readDebugParams() );

      // we dont need the data buffers anymore

      delete stringBuffer[ i ].second;
      stringBuffer[ i ].second = NULL;

      delete dataBuffer[ i ].second;
      dataBuffer[ i ].second = NULL;
            
   }

   stringBuffer.clear();
   dataBuffer.clear();
}

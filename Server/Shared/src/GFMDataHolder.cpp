/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GFMDataHolder.h"
#include "DataBuffer.h"
#include "PacketContainer.h"
#include "Packet.h"
#include "GfxFeatureMap.h"
#include "GunzipUtil.h"

GFMDataHolder::GFMDataHolder() 
{
   m_gfxMap = NULL;
   m_gfxMapBuffer = NULL;
   m_copyright = "";
}

GFMDataHolder::GFMDataHolder( const MC2BoundingBox& bbox, 
                              int screenSizeX, 
                              int screenSizeY )
{
   m_bbox = bbox;
   m_screenSizeX = screenSizeX;
   m_screenSizeY = screenSizeY;
   m_gfxMap = NULL;
   m_gfxMapBuffer = NULL;
   m_copyright = "";
}

GFMDataHolder::~GFMDataHolder()
{
   // Delete.
   for ( uint32 i = 0; i < m_data.size(); ++i ) {
      delete m_data[ i ].m_garbage;
      delete m_data[ i ].m_data;
   }

   delete m_gfxMap;
   delete m_gfxMapBuffer;
}

void
GFMDataHolder::addGFMData( PacketContainer* pc,
                           bool zipped,
                           DataBuffer* data,
                           uint32 requestTag,
                           const char* copyright )
{
   delete m_gfxMap;
   m_gfxMap = NULL;
   delete m_gfxMapBuffer;
   m_gfxMapBuffer = NULL;
   m_data.push_back( GFMData( pc, zipped, data, 
                              requestTag, copyright ) );
}


int
GFMDataHolder::save( Packet* packet, int& pos ) const
{
   int startPos = pos;
   
   packet->incWriteLong( pos, 0 ); // Length, filled in later.
   
   packet->incWriteBBox( pos, m_bbox );
   packet->incWriteLong( pos, m_screenSizeX );
   packet->incWriteLong( pos, m_screenSizeY );
   packet->incWriteLong( pos, m_data.size() );

   // Approximate the size needed in the packet.
   uint32 approxSize = 100 * m_data.size();
   for ( uint32 i = 0; i < m_data.size(); ++i ) {
      approxSize += m_data[ i ].m_data->getCurrentOffset();
   }
   // updateSize requires that the length of the packet is set.
   packet->setLength( pos );
   packet->updateSize( approxSize, approxSize );

   for ( uint32 i = 0; i < m_data.size(); ++i ) {
      const GFMData& gfmData = m_data[ i ];
      packet->incWriteLong( pos, gfmData.m_requestTag );
      packet->incWriteLong( pos, gfmData.m_data->getCurrentOffset() );
      packet->incWriteByteArray( pos, 
                                 gfmData.m_data->getBufferAddress(),
                                 gfmData.m_data->getCurrentOffset() );
      packet->incWriteString( pos, gfmData.m_copyright.c_str() );
      packet->incWriteByte( pos, gfmData.m_zipped );
      AlignUtility::alignLong( pos );
   }
   int nbrBytes = pos - startPos;
   packet->incWriteLong( startPos, nbrBytes );
   
   return nbrBytes;
}
      
int 
GFMDataHolder::load( const Packet* packet, int& pos )
{
   int startPos = pos;
   int sizeInPacket = packet->incReadLong( pos );
  
   packet->incReadBBox( pos, m_bbox );
   m_screenSizeX = packet->incReadLong( pos );
   m_screenSizeY = packet->incReadLong( pos );
   uint32 count = packet->incReadLong( pos );
   
   for ( uint32 i = 0; i < count; ++i ) {
      GFMData gfmData;
      
      gfmData.m_requestTag = packet->incReadLong( pos );
      uint32 nbrBytes = packet->incReadLong( pos );
      const byte* data = packet->incReadByteArray( pos, nbrBytes );
      gfmData.m_data = new DataBuffer( const_cast<byte*> (data), nbrBytes ); 
      gfmData.m_copyright = packet->incReadString( pos );
      gfmData.m_zipped = packet->incReadByte( pos );
      AlignUtility::alignLong( pos );
      gfmData.m_garbage = NULL;
      m_data.push_back( gfmData );
   }
   pos = startPos + sizeInPacket;
   return sizeInPacket;
}

void
GFMDataHolder::mergeIfNecessary()
{
   if ( m_gfxMapBuffer != NULL ) {
      return;
   }

   // Unzip any zipped buffers.
   for ( uint32 i = 0; i < m_data.size(); ++i ) {
      GFMData& gfmData = m_data[ i ];
      
      if ( gfmData.m_zipped ) {
         uint32 startTime = TimeUtility::getCurrentTime();
         // Check unzipped length.
         int unzippedLength = 
            GunzipUtil::origLength( gfmData.m_data->getBufferAddress(),
                                    gfmData.m_data->getBufferSize() );
         
         // Unzip.
         unsigned char* unzippedBuf = 
            new unsigned char[ unzippedLength ];
         int retVal = GunzipUtil::gunzip( unzippedBuf,
                                          unzippedLength, 
                                          gfmData.m_data->getBufferAddress(),
                                          gfmData.m_data->getBufferSize() );
         if ( retVal < 0 ) {
            delete[] unzippedBuf;
            return;
         }
         
         mc2dbg << "[GFMDH]: Unzipping took " 
                << TimeUtility::getCurrentTime() - startTime << " ms." << endl;
         startTime = 0;
         
         delete gfmData.m_data;
         gfmData.m_data = new DataBuffer( unzippedBuf, unzippedLength );
         // Better read past the bytes just to make sure..
         gfmData.m_data->readPastBytes( unzippedLength );
         gfmData.m_zipped = false;
      }
   }

   GfxFeatureMap* answerMap = new GfxFeatureMap();
   vector< byte > answerBuffer;
   uint32 answerNbrFeatures = 0;
   const char* copyright = "";
   MC2String bestCR = "";
   GfxFeatureMap tempMap;

   // loop through all the replies
   map<MC2String, uint32> nbrFeaturesPerCopyright;
   map<MC2String, uint32>::iterator crIt;
   DataBuffer* mapData = NULL;
   bool firstMapData = true;
   for ( uint32 i = 0; i < m_data.size(); ++i ) {
      const GFMData& gfmData = m_data[ i ];
      mapData = gfmData.m_data;
      
      // FIXME: Is mapData really allowed to be NULL???
      if ( mapData != NULL ) {
         
         uint32 nbrFeatures = 0;
         if ( firstMapData ) {
            firstMapData = false;
            nbrFeatures = answerMap->loadHeader( mapData );
         } else
            nbrFeatures = tempMap.loadHeader( mapData );
         answerNbrFeatures += nbrFeatures;
         answerBuffer.insert( answerBuffer.end(), 
                              (4 - answerBuffer.size()) & 0x3, 0);
         answerBuffer.insert( answerBuffer.end(), 
                              mapData->getCurrentOffsetAddress(), 
                              mapData->getCurrentOffsetAddress() + 
                              (mapData->getBufferSize() - 
                              mapData->getCurrentOffset()) );

#define GFMIR_TAG_ROUTE 0x4000000
         if ( gfmData.m_requestTag == GFMIR_TAG_ROUTE)  // first route packet
            answerMap->mergeRouteSettings( &tempMap );

         // Only count copyright strings that are not empty.
         if ( gfmData.m_copyright[ 0 ] != 0 ) {
            // Count number features added per copyright
            MC2String tmpCR = gfmData.m_copyright.c_str();
            crIt = nbrFeaturesPerCopyright.find( tmpCR );
            if ( crIt != nbrFeaturesPerCopyright.end() ) {
               crIt->second += nbrFeatures;
            } else {
               nbrFeaturesPerCopyright.insert( make_pair(tmpCR, nbrFeatures) );
            }
         }
      } else {
         // This should not happen.
         MC2_ASSERT( false );
      }
   }
      
   // Select the best copyright string (the one that most features have)
   // Don't use empty string (IM sends empty copyright)
   uint32 nbrFeaturesWithCR = 0;
   for ( crIt = nbrFeaturesPerCopyright.begin();
         crIt != nbrFeaturesPerCopyright.end(); crIt++) {
      if ( (crIt->first.size() > 0) &&
           (crIt->second > nbrFeaturesWithCR) ) {
         nbrFeaturesWithCR = crIt->second;
         bestCR = crIt->first;
      }
   }
   if ( bestCR.size() > 0 ) {
      copyright = bestCR.c_str();
   }
    
   // Set the bbox and screen size in case out of map coverage.
   answerMap->setMC2BoundingBox( &m_bbox );
   answerMap->setScreenSize( m_screenSizeX, m_screenSizeY );

   // Save the answermap into a DataBuffer
   DataBuffer featData( &answerBuffer.front(), answerBuffer.size() );
   uint32 answerSize = answerMap->getMapSize() + answerBuffer.size();

   m_copyright = copyright;
   m_gfxMapBuffer = new DataBuffer( answerSize );
   answerMap->save( m_gfxMapBuffer, &featData, answerNbrFeatures );
   
   // Delete memory allocated here
   delete answerMap;
}



const GfxFeatureMap* 
GFMDataHolder::getGfxFeatureMap()
{
   mergeIfNecessary();
   if ( m_gfxMap == NULL ) {
      m_gfxMap = new GfxFeatureMap();
      m_gfxMap->load( m_gfxMapBuffer );
   }
   return m_gfxMap;
}

DataBuffer*
GFMDataHolder::getBuffer()
{
   mergeIfNecessary();
   return m_gfxMapBuffer;
}

const MC2SimpleString&
GFMDataHolder::getCopyright()
{
   mergeIfNecessary();
   return m_copyright; 
}


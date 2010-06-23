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
#include "MapSettings.h"
#include "GenericMap.h"
#include "UserRightsMapInfo.h"

#include "GfxFeatureMapPacket.h"
#include "GunzipUtil.h"

#include "StringUtility.h"

#define MAX_GFXFEATUREMAPREPLY_HEADER_SIZE (REPLY_HEADER_SIZE+12*4)

// ---------------------------------------------------------------------
// ---------------------------------------- GfxFeatureMapRequestPacket -


GfxFeatureMapRequestPacket::GfxFeatureMapRequestPacket(
   uint32 mapID,
   const UserUser* user,
   uint16 reqID,
   uint16 packetID,
   int32  upperLeftLat,
   int32  upperLeftLon,
   int32  lowerRightLat,
   int32  lowerRightLon,
   uint16 screenSizeX,
   uint16 screenSizeY,
   uint32 maxScaleLevel,
   uint32 minScaleLevel,
   uint32 filtScaleLevel,
   LangTypes::language_t language,
   const MapSettings* mapSettings,
   bool   ignoreStartOffset,
   bool   ignoreEndOffset,
   uint16 startOffset,
   uint16 endOffset,
   bool drawOverviewContents,
   bool extractForTileMaps )
      : RequestPacket( MAX_PACKET_SIZE,
                       GFXFEATUREMAP_REQUEST_PRIO,
                       PACKETTYPE_GFXFEATUREMAPREQUEST,
                       packetID,
                       reqID, 
                       MAX_UINT32 )
{
   setMapID( mapID );
   MC2_ASSERT(mapSettings != NULL);
   writeLong( upperLeftLat_POS, upperLeftLat );
   writeLong( upperLeftLon_POS, upperLeftLon );
   writeLong( lowerRightLat_POS, lowerRightLat );
   writeLong( lowerRightLon_POS, lowerRightLon );
   writeShort( screenSizeX_POS, screenSizeX );
   writeShort( screenSizeY_POS, screenSizeY );
   writeLong( maxScaleLevel_POS, maxScaleLevel );
   writeLong( minScaleLevel_POS, minScaleLevel );
   writeLong( filtScaleLevel_POS, filtScaleLevel );
   writeLong( language_POS, language );
   writeShort( startOffset_POS, startOffset );
   writeShort( endOffset_POS, endOffset );
   // Write some bits.
   setIgnoreStartOffset(ignoreStartOffset);
   setIgnoreEndOffset(ignoreEndOffset);
   setDrawOverviewContents(drawOverviewContents);
   writeBit(isStartAndEnd_POS, EXTRACT_FOR_TILEMAPS_BIT_POS, 
            extractForTileMaps);
   writeByte( nbrReqPackets_POS, 0); // Nbr req packets
   writeShort( nbrNodeIDs_POS, 0 ); // Nbr node ids.
   int pos = endStatic_POS;
   mapSettings->saveToPacket( this, pos );
   setLength( pos );

   UserRightsMapInfo rights( mapID, user, ~MapRights() );
   rights.save( this, pos );

   // Save the real length of the mapSettings + userRights
   writeLong( mapSettingsLen_POS, pos - endStatic_POS );

   setLength( pos );
}

void
GfxFeatureMapRequestPacket::getBoundingCoordinates(int32&  upperLeftLat,
                                                   int32&  upperLeftLon,
                                                   int32&  lowerRightLat,
                                                   int32&  lowerRightLon) const
{
   int position = upperLeftLat_POS;
   upperLeftLat = incReadLong(position);
   upperLeftLon = incReadLong(position);
   lowerRightLat = incReadLong(position);
   lowerRightLon = incReadLong(position);
}


void 
GfxFeatureMapRequestPacket::getMC2BoundingBox(MC2BoundingBox* bbox) const
{
   int32 ulLat, ulLon, lrLat, lrLon;
   getBoundingCoordinates(ulLat, ulLon, lrLat, lrLon);
   bbox->reset();
   
   bbox->update(ulLat, ulLon);
   bbox->update(lrLat, lrLon);
}


void 
GfxFeatureMapRequestPacket::setBoundingCoordinates(int32  upperLeftLat,
                                             int32  upperLeftLon,
                                             int32  lowerRightLat,
                                             int32  lowerRightLon)
{
   int position = upperLeftLat_POS;
   incWriteLong(position, upperLeftLat);
   incWriteLong(position, upperLeftLon);
   incWriteLong(position, lowerRightLat);
   incWriteLong(position, lowerRightLon);
}

void 
GfxFeatureMapRequestPacket::getScreenSize(uint16& screenSizeX, 
                                    uint16& screenSizeY) const
{
   int position = screenSizeX_POS;
   screenSizeX = incReadShort(position);
   screenSizeY = readShort(position);
}

ScreenSize
GfxFeatureMapRequestPacket::getScreenSize() const {
   ScreenSize::ValueType x, y;
   getScreenSize( x, y );
   return ScreenSize( x, y );
}

void 
GfxFeatureMapRequestPacket::setScreenSize(uint16 screenSizeX, uint16 screenSizeY)
{
   int position = screenSizeX_POS;
   incWriteShort(position, screenSizeX);
   incWriteShort(position, screenSizeY);
}

uint32 
GfxFeatureMapRequestPacket::getMaxScaleLevel() const
{
   return (readLong(maxScaleLevel_POS));
}

void 
GfxFeatureMapRequestPacket::setMaxScaleLevel(uint32 scaleLevel)
{
   int position = maxScaleLevel_POS;
   incWriteLong(position, scaleLevel);
}

uint32 
GfxFeatureMapRequestPacket::getMinScaleLevel() const
{
   return (readLong(minScaleLevel_POS));
}

void 
GfxFeatureMapRequestPacket::setMinScaleLevel(uint32 scaleLevel)
{
   int position = minScaleLevel_POS;
   incWriteLong(position, scaleLevel);
}

uint32 
GfxFeatureMapRequestPacket::getFiltScaleLevel() const
{
   return (readLong(filtScaleLevel_POS));
}

void 
GfxFeatureMapRequestPacket::setFiltScaleLevel(uint32 scaleLevel)
{
   int position = filtScaleLevel_POS;
   incWriteLong(position, scaleLevel);
}


LangTypes::language_t
GfxFeatureMapRequestPacket::getLanguage() const
{
   return LangTypes::language_t( readLong( language_POS ) );
}


void 
GfxFeatureMapRequestPacket::setLanguage( 
   LangTypes::language_t language )
{
   writeLong( language_POS, language );
}


void 
GfxFeatureMapRequestPacket::getMapSettings( MapSettings* mapSettings ) const
{
   MC2_ASSERT( mapSettings != NULL );
   if ( readLong( mapSettingsLen_POS ) > 0 ) {
      int pos = endStatic_POS;   
      mapSettings->loadFromPacket( this, pos );
   }
}

void 
GfxFeatureMapRequestPacket::getMapSettingsAndRights( MapSettings& mapSettings,
                                                     UserRightsMapInfo& rights) const
{   
   if ( readLong( mapSettingsLen_POS ) > 0 ) {
      int pos = endStatic_POS;   
      mapSettings.loadFromPacket( this, pos );
      if ( pos < (int)getLength() ) {
         rights.load( this, pos );
      } else {
         UserRightsMapInfo all( getMapID(), ~MapRights() );
         rights.swap( all );
      }
   }
}


bool
GfxFeatureMapRequestPacket::extractForTileMaps() const
{
   return readBit(isStartAndEnd_POS, EXTRACT_FOR_TILEMAPS_BIT_POS );
}

bool
GfxFeatureMapRequestPacket::getIgnoreStartOffset() const
{
   // NB! Inverted in the packet.
   return ! readBit( isStartAndEnd_POS, IGN_START_BIT_POS);
}


void
GfxFeatureMapRequestPacket::setIgnoreStartOffset(bool ignore)
{
   // NB! Inverted in the packet.
   writeBit( isStartAndEnd_POS, IGN_START_BIT_POS, ! ignore);
}


bool
GfxFeatureMapRequestPacket::getIgnoreEndOffset() const
{
   // NB! Inverted in the packet.
   return ! readBit(isStartAndEnd_POS, IGN_END_BIT_POS);
}


void
GfxFeatureMapRequestPacket::setIgnoreEndOffset(bool ignore)
{
   // NB! Inverted in the packet.
   writeBit(isStartAndEnd_POS, IGN_END_BIT_POS, ! ignore);
}

bool
GfxFeatureMapRequestPacket::getDrawOverviewContents() const
{
   return readBit(isStartAndEnd_POS, DRAW_OVERVIEW_BIT_POS);
}

void
GfxFeatureMapRequestPacket::setDrawOverviewContents(bool drawOverview)
{
   writeBit(isStartAndEnd_POS, DRAW_OVERVIEW_BIT_POS, drawOverview);
}

bool
GfxFeatureMapRequestPacket::getIncludeCountryPolygon() const
{
   return readBit(isStartAndEnd_POS, DRAW_COUNTRYPOLYGON_BIT_POS);
}

void
GfxFeatureMapRequestPacket::setIncludeCountryPolygon(
                                          bool includeCountryPolygon)
{
   writeBit(isStartAndEnd_POS, DRAW_COUNTRYPOLYGON_BIT_POS, 
            includeCountryPolygon);
}

uint16
GfxFeatureMapRequestPacket::getStartOffset() const
{
   int pos = startOffset_POS;

   return ( readShort( pos ) );
}


void
GfxFeatureMapRequestPacket::setStartOffset(uint16 offset)
{
   int pos = startOffset_POS;

   writeShort( pos, offset );
}


uint16
GfxFeatureMapRequestPacket::getEndOffset() const
{
   int pos = endOffset_POS;

   return ( readShort( pos ) );
}


void
GfxFeatureMapRequestPacket::setEndOffset(uint16 offset)
{
   writeShort( endOffset_POS, offset );
}


uint16
GfxFeatureMapRequestPacket::getNbrNodeIDs() const
{
   return ( readShort( nbrNodeIDs_POS) );
}


void
GfxFeatureMapRequestPacket::addNodeID( uint32 nodeID )
{
   int pos = incNbrNodeIDs();
   incWriteLong( pos, nodeID );
   setLength( pos );
}


uint32 
GfxFeatureMapRequestPacket::getNodeIDs( const GenericMap* theMap,
                                        list<uint32>& nodeIDs ) const
{
   uint32 nbrNodeIDs = getNbrNodeIDs();
   int pos = readLong( mapSettingsLen_POS ) + endStatic_POS;
   for ( uint32 i = 0; i < nbrNodeIDs; ++i ) {
      nodeIDs.push_back( incReadLong( pos ) );
   }
   // Expand node ids.
   return theMap->expandNodeIDs( nodeIDs );
}

byte
GfxFeatureMapRequestPacket::getNbrReqPackets() const
{
   int pos = nbrReqPackets_POS;

   return ( readByte(pos) );
}

void
GfxFeatureMapRequestPacket::setNbrReqPackets(byte nbr)
{
   int pos = nbrReqPackets_POS;

   writeByte( pos, nbr );
}


void
GfxFeatureMapRequestPacket::printPacket() const
{
   cout << "GfxFeatureMapRequestPacket::printPacket() for map " 
        <<  getMapID() << endl;
   cout << "Bounding box for the GfxFeatureMapRequestPacket:" << endl;
   MC2BoundingBox bbox;
   getMC2BoundingBox(&bbox);
   bbox.dump();
   uint16 x, y;
   getScreenSize(x, y);
   cout << "Screen size (x = " << x << ", y = " 
        << y << ")" << endl;
   cout << "max scaleLevel = " << getMaxScaleLevel() << endl;
   cout << "min scaleLevel = " << getMinScaleLevel() << endl;
   cout << "filt scaleLevel = " << getFiltScaleLevel() << endl;
   cout << "language = " << (int)getLanguage() << endl;
   cout << "nbrNodeIDs " << getNbrNodeIDs() << endl;
   cout << "  isStartAndEnd byte 0x" << hex 
        << int( readByte(isStartAndEnd_POS)) << dec << endl;
   cout << "IgnoreStartOffset " << BP(getIgnoreStartOffset()) << endl;
   cout << "  isStartAndEnd byte 0x" << hex 
        << int(readByte(isStartAndEnd_POS)) << dec << endl;
   cout << "IgnoreEndOffset " << BP(getIgnoreEndOffset()) << endl;
   cout << "StartOffset " << getStartOffset() << endl;
   cout << "EndOffset " << getEndOffset() << endl;
   cout << "NbrReqPackets " << int(getNbrReqPackets()) << endl;
   cout << "DrawOverviewContents " << BP(getDrawOverviewContents()) 
        << endl;
}


// ---------------------------------------------------------------------
// ------------------------------------------------ GfxFeatureMapReplyPacket -

void
GfxFeatureMapReplyPacket::init()
{
   // Initialize the flag field to zero.
   writeLong( FLAGS_POS, 0 );
   
   // Set "don't draw the roads of overviewmaps"
   setDrawOverviewContents(false);
   // Set zero requests to resend
   int position = NBR_REQUESTS_POS;
   incWriteLong(position, 0);
   // Init mapID
   setMapID( MAX_UINT32 );

   // Print empty copyright
   setCopyright( "" );
   position += readShort( LENGTH_COPYRIGHT_POS ) + 2;
   
   // Set zero bytes of GfxFeatureMap data
   incWriteLong(position, 0);

   setLength(position);
}


GfxFeatureMapReplyPacket::GfxFeatureMapReplyPacket(uint32 size)
   : ReplyPacket( size + MAX_GFXFEATUREMAPREPLY_HEADER_SIZE,
                  PACKETTYPE_GFXFEATUREMAPREPLY)
                  
{
   init();
}


GfxFeatureMapReplyPacket::GfxFeatureMapReplyPacket(const GfxFeatureMapRequestPacket* p)
   : ReplyPacket( MAX_PACKET_SIZE,
                  PACKETTYPE_GFXFEATUREMAPREPLY,
                  (RequestPacket*) p,
                  StringTable::OK)
{
   init();
   setDrawOverviewContents(p->getDrawOverviewContents());
}


void 
GfxFeatureMapReplyPacket::setCopyright( const char* copyright ) {
   int pos = COPYRIGHT_POS;
   incWriteString( pos, copyright );
   writeShort( LENGTH_COPYRIGHT_POS, pos - COPYRIGHT_POS );
   setLength( pos );
}


const char* 
GfxFeatureMapReplyPacket::getCopyright() const {
   char* tmp = NULL;
   int pos = COPYRIGHT_POS;
   incReadString( pos, tmp );
   return tmp;
}


void 
GfxFeatureMapReplyPacket::addMapIDForResending(uint32 mapID, 
                                               bool includeCountryPolygon)
{
   // Until Gfx is added this is the place to put mapids.
   int position = getGfxStartPos(); 
   incWriteLong(position, mapID);
   incWriteLong(position, includeCountryPolygon);
   // zero bytes of GfxFeatureMap data
   incWriteLong(position, 0);
   setLength(position);
   incNbrRequestsToResend();
}

uint32 
GfxFeatureMapReplyPacket::getMapIDToResend(uint32 i) const
{
   if (i < getNbrRequestsToResend()) {
      // get the map id for i:th map
      return readLong( MAP_ID_START_POS + (i * 8) + 
                       readShort( LENGTH_COPYRIGHT_POS ) );
   } else {
      return MAX_UINT32;
   }
}

bool 
GfxFeatureMapReplyPacket::getIncludeCountryPolygonForMapToResend(uint32 i) const
{
   if (i < getNbrRequestsToResend()) {
      // get countrymap bool for i:th map
      return (readLong( MAP_ID_START_POS + (i * 8) + 4 + 
                       readShort( LENGTH_COPYRIGHT_POS )) != 0);
   } else {
      return false;
   }
}

void 
GfxFeatureMapReplyPacket::setGfxFeatureMapData(uint32 size, 
                                               DataBuffer* data,
                                               bool zipped )
{
   // Store the zipped flag.
   setZippedGfxFeatureMapData( zipped );
   
   // Get position
   int position = getGfxStartPos();

   // Check size of buffer
   int bufSize  = getBufSize();
   int required = position + size + 1024;
   if ( required > bufSize ) {
      mc2dbg8 << "GfxFeatureMapReplyPacket : Resizing to " << required 
              << endl;
      setLength(position); // Needed for resize,
      resize(required);
   }
   
   // Write the size
   incWriteLong(position, size);

   // Reset offsets
   data->reset();
   
   // Get the bytearray pointer from the DataBuffer.
   const byte* bytes = data->readNextByteArray(size);
   // Copy it into the packet.
   incWriteByteArray(position, bytes, size);
   
   mc2dbg8 << "position " << position<< endl;
   setLength(position);
}

DataBuffer* 
GfxFeatureMapReplyPacket::getGfxFeatureMapData( bool& zipped )
{
   // Get if the data is zipped.
   zipped = getZippedGfxFeatureMapData();

   int position = getGfxStartPos();
   
   // Read size
   uint32 size = incReadLong(position);

   if (size > 0) {

      // Get the byte pointer from the packet.
      const byte* bytes = incReadByteArray(position, size);

      // Don't copy the data.
      DataBuffer* data = new DataBuffer( const_cast<byte*> (bytes), size );
      // Read past the bytes.
      data->readPastBytes( size );
      
      mc2dbg8 << "position " << position<< endl;
      return (data);
   } else {
      return (NULL);
   }
}

DataBuffer* 
GfxFeatureMapReplyPacket::getGfxFeatureMapData()
{
   bool zipped = false;
   DataBuffer* maybeZippedBuf = getGfxFeatureMapData( zipped );
   
   if ( zipped ) {
      // Zipped, means that we should unzip it.
      
      // Check unzipped length.
      int unzippedLength = 
         GunzipUtil::origLength( maybeZippedBuf->getBufferAddress(),
                                 maybeZippedBuf->getBufferSize() );
      
      // Unzip.
      DataBuffer* unzippedBuf = new DataBuffer( unzippedLength );
      int retVal = GunzipUtil::gunzip( unzippedBuf->getBufferAddress(),
                                       unzippedBuf->getBufferSize(), 
                                       maybeZippedBuf->getBufferAddress(),
                                       maybeZippedBuf->getBufferSize() );
      delete maybeZippedBuf;
      if ( retVal < 0 ) {
         delete unzippedBuf;
         return NULL;
      }
      // Better read past the bytes just to make sure..
      unzippedBuf->readPastBytes( unzippedLength );
      return unzippedBuf;
   } else {
      // Was already unzipped.
      return maybeZippedBuf;
   }
}

bool
GfxFeatureMapReplyPacket::getDrawOverviewContents() const
{
   return readBit(FLAGS_POS, DRAW_OVERVIEW_BIT_POS);
}

void
GfxFeatureMapReplyPacket::setDrawOverviewContents(bool drawOverview)
{
   writeBit(FLAGS_POS, DRAW_OVERVIEW_BIT_POS, drawOverview);
}

bool
GfxFeatureMapReplyPacket::getIncludeCountryPolygon() const
{
   return readBit(FLAGS_POS, DRAW_COUNTRYPOLYGON_BIT_POS);
}

void
GfxFeatureMapReplyPacket::setZippedGfxFeatureMapData(bool zipped)
{
   writeBit(FLAGS_POS, ZIPPED_GFXFEATUREMAPDATA_POS, zipped);
}

bool
GfxFeatureMapReplyPacket::getZippedGfxFeatureMapData() const
{
   return readBit(FLAGS_POS, ZIPPED_GFXFEATUREMAPDATA_POS);
}

void
GfxFeatureMapReplyPacket::setIncludeCountryPolygon(bool includeCountryPolygon)
{
   writeBit(FLAGS_POS, DRAW_COUNTRYPOLYGON_BIT_POS, includeCountryPolygon);
}

void
GfxFeatureMapReplyPacket::printPacket()
{
   cout << "GfxFeatureMapReplyPacket::printPacket()" << endl;
   cout << "  nbr requests to resend : " << getNbrRequestsToResend()
        << " [";
   uint32 i;
   for (i = 0; i < getNbrRequestsToResend(); i++)
      cout << " " << getMapIDToResend(i);
   cout << " ]" << endl;
}


void 
GfxFeatureMapReplyPacket::setMapID( uint32 mapID ) {
   writeLong( MAPID_POS, mapID );
}


uint32 
GfxFeatureMapReplyPacket::getMapID() const {
   return readLong( MAPID_POS );
}

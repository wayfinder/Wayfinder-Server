/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapPacket.h"
#include "StringTable.h"

// ******************************************************************
//                                                   MapRequestPacket

void
MapRequestPacket::init(byte mapType,
                       byte zoomLevel,
                       uint32 mapVersion,
                       uint32 mapGeneratorVersion)
{
   int pos = MAP_TYPE_POS;
   incWriteByte(pos, mapType);
   incWriteByte(pos, zoomLevel);
   incWriteShort(pos, 0); // Should really pad by it self.
   incWriteLong(pos, mapVersion);
   incWriteLong(pos, mapGeneratorVersion);
   incWriteLong(pos, 0); // Nbr of overviewMaps
   incWriteLong(pos, MAX_UINT32); // Place to put overview map, level 1
   incWriteLong(pos, MAX_UINT32); // Place to put overview map, level 2
   incWriteLong(pos, MAX_UINT32); // Place to put overview map, level 3
   setLength(pos);
}

MapRequestPacket::MapRequestPacket( uint16 requestId,
                                    byte mapType,
				                        uint32 mapID,
                                    byte zoomlevel,
                                    uint32 mapVersion,
                                    uint32 mapGeneratorVersion)
   : RequestPacket( MAX_MAP_REQUEST_LENGTH, 
                    MAP_REQUEST_PRIO, 
                    Packet::PACKETTYPE_MAPREQUEST, 
                    0, // packetID
                    requestId, 
                    mapID )
{
   init(mapType, zoomlevel, mapVersion, mapGeneratorVersion);
}

MapRequestPacket::MapRequestPacket( uint16 requestId,
                                    uint16 packetId,
                                    byte mapType,
                                    uint32 mapID,
                                    byte zoomlevel,
                                    uint32 mapVersion,
                                    uint32 mapGeneratorVersion)
   : RequestPacket( MAX_MAP_REQUEST_LENGTH, 
                    MAP_REQUEST_PRIO, 
                    Packet::PACKETTYPE_MAPREQUEST, 
                    packetId,
                    requestId, 
                    mapID )
{
   init(mapType, zoomlevel, mapVersion, mapGeneratorVersion);
}


byte
MapRequestPacket::getMapType() const
{
   return readByte( MAP_TYPE_POS );
}

const char*
MapRequestPacket::getMapTypeAsString() const
{
   MapRequestPacket::MapType maptype =
      MapRequestPacket::MapType(getMapType());
   switch ( maptype ) {
      case MAPREQUEST_SEARCH:
         return "MAPREQUEST_SEARCH";
      case MAPREQUEST_ROUTE:
         return "MAPREQUEST_ROUTE";
      case MAPREQUEST_GFX:
         return "MAPREQUEST_GFX";
      case MAPREQUEST_STRINGTABLE:
         return "MAPREQUEST_STRINGTABLE";
      case MAPREQUEST_GFXCOUNTRY:
         return "MAPREQUEST_GFXCOUNTRY";
      case MAPREQUEST_INFO:
         return "MAPREQUEST_INFO";
   }
   // Shouldn't happen - compiler should
   // complain if a case is missing. Unfortunately it
   // complains if there is no return here too.
   return "MAPREQUEST_IMPOSSIBLE"; 
}

void
MapRequestPacket::setMapType(MapType newType)
{
   writeByte( MAP_TYPE_POS, newType );
}

byte
MapRequestPacket::getZoomlevel() const
{
   return readByte( ZOOM_LEVEL_POS );
}

void 
MapRequestPacket::setZoomlevel( byte zoomlevel )
{
   writeByte( ZOOM_LEVEL_POS, zoomlevel );
}

int
MapRequestPacket::setOverviewMaps(const vector<uint32>& maps)
{
   int pos = NBR_OVERVIEW_POS;
   incWriteLong(pos, maps.size());
   for( vector<uint32>::const_iterator it(maps.begin());
        it != maps.end();
        ++it) {
      incWriteLong(pos, *it);
   }
   setLength(pos);
   return maps.size();
}

int
MapRequestPacket::getOverviewMaps(vector<uint32>& maps) const
{
   int pos = NBR_OVERVIEW_POS;
   int nbrOv = incReadLong(pos);
   for(int i=0; i < nbrOv; ++i ) {
      uint32 mapID = incReadLong(pos);
      maps.push_back(mapID);
   }
   return nbrOv;
}

uint32
MapRequestPacket::getMapVersion() const
{
   return readLong( MAP_VERSION_POS );
}

uint32
MapRequestPacket::getMapGeneratorVersion() const
{
   return readLong( MAP_GENERATOR_VER_POS );
}

bool
MapRequestPacket::newMapNeeded(uint32 myVersion,
                               uint32 myGeneratorVersion) const
{
   uint32 packetVersion = getMapVersion();
   uint32 packetGenVer  = getMapGeneratorVersion();

   // If the packetVersions are MAX_UINT32 the sender does not
   // know about versions. Load map.
   if ( (packetVersion == MAX_UINT32) || (packetGenVer == MAX_UINT32) )
      return true;

   // Check versions
   return (myVersion != packetVersion) ||
          (myGeneratorVersion != packetGenVer );
}


// ******************************************************************
//                                                     MapReplyPacket

MapReplyPacket::MapReplyPacket(const MapRequestPacket* requestPacket,
                               uint32 mapIP,
                               uint16 mapPort,
                               uint32 mapVersion,
                               uint32 generatorVersion)
 :  ReplyPacket( MAX_MAP_REPLY_LENGTH,
                 Packet::PACKETTYPE_MAPREPLY,
                 requestPacket,
                 StringTable::OK )
{
   int position = REPLY_HEADER_SIZE;
   incWriteLong(position, mapIP);
   incWriteLong(position, mapPort);
   incWriteLong(position, requestPacket->getMapID());
   incWriteLong(position, mapVersion);
   incWriteLong(position, generatorVersion);
   setLength(position);   
}

uint32 
MapReplyPacket::getReplyIP() const
{
   return readLong( REPLY_IP_POS );
}

uint16
MapReplyPacket::getReplyPort() const
{
   return readLong( REPLY_PORT_POS );
}

uint32
MapReplyPacket::getMapID() const
{
   return readLong( MAP_ID_POS );
}


uint32
MapReplyPacket::getMapVersion() const
{
   return readLong( MAP_VERSION_POS );
}

uint32
MapReplyPacket::getGeneratorVersion() const
{
   return readLong( MAP_GENERATOR_POS );
}

bool
MapReplyPacket::newMapNeeded(uint32 myVersion,
                             uint32 myGeneratorVersion) const
{
   if ( getMapVersion() == MAX_UINT32 ||
        getGeneratorVersion() == MAX_UINT32 ) {
      // Sender does not use the versions.
      return true;
   }
   
   return (myVersion != getMapVersion()) ||
          (myGeneratorVersion != getGeneratorVersion() );
}

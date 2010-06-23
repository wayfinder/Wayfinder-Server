/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_PACKET_H
#define MAP_PACKET_H

#include "config.h"
#include <vector>

#include "Packet.h"

#define MAX_MAP_REQUEST_LENGTH (REQUEST_HEADER_SIZE + 40)
#define MAX_MAP_REPLY_LENGTH   (REPLY_HEADER_SIZE   + 40)

#define MAP_REPLY_PRIO   DEFAULT_PACKET_PRIO
#define MAP_REQUEST_PRIO DEFAULT_PACKET_PRIO


 /**
   *  Packet used for requesting a map from the map modules
   *
   *  Until the maps contain information about the overview maps
   *  the reader should write the overview map id into the
   *  pixel width field if the request is a routemap request.
   *
   *  After the normal header this packet contains
   *  @packetdesc
   *     @row HEADER_SIZE @sep 4 bytes @sep
   *                  map ID (included in the RequestPacket) @endrow
   *     @row +4  @sep 1 byte  @sep maptype                  @endrow
   *     @row +5  @sep 1 byte  @sep zoomlevel                @endrow
   *     @row +8  @sep 4 byte  @sep mapVersion               @endrow
   *     @row +12 @sep 4 byte  @sep mapGeneratorVersion      @endrow
   *     @row +16 @sep 4 byte  @sep number of overview maps (If route) @endrow
   *     @row +20 @sep 4 byte  @sep first overview map (if route) @endrow
   *  @endpacketdesc
   *  
   */
class MapRequestPacket : public RequestPacket 
{
public:
  
   /** Subtypes for PACKETTYPE_MAPREQUEST */
      
   enum MapType {
      MAPREQUEST_SEARCH       = 0,
      MAPREQUEST_ROUTE        = 1,
      MAPREQUEST_GFX          = 2,
      MAPREQUEST_STRINGTABLE  = 3,
      MAPREQUEST_GFXCOUNTRY   = 4,
      MAPREQUEST_INFO         = 5
   };

   /** 
    *  @param requestId the request of this packet.
    *  @param mapType Type of map {search, route, gfx}.
    *  @param mapID The ID of the map that we want.
    *  @param zoomlevel Requested zoomlevel.
    *  @param mapVersion The mapversion already cached by the requestor.
    */
   MapRequestPacket(uint16 requestId,
                    byte mapType, 
                    uint32 mapID,
                    byte zoomlevel = 0,
                    uint32 mapVersion          = MAX_UINT32,
                    uint32 mapGeneratorVersion = MAX_UINT32 );

   /**
    *  @param requestId the request of this packet.
    *  @param packetId the ID of this packet.
    *  @param mapType Type of map {search, route, gfx}.
    *  @param mapID The ID of the map that we want.
    *  @param zoomlevel Requested zoomlevel.
    *  @param mapVersion The already cached version of the map at the callers.
    */
   MapRequestPacket(uint16 requestId,
                    uint16 packetId,
                    byte mapType, 
                    uint32 mapID,
                    byte zoomlevel = 0,
                    uint32 mapVersion = MAX_UINT32,
                    uint32 mapGeneratorVersion = MAX_UINT32 );

   /**
    *   Initializes the packet. Used by the constructors.
    *   Also makes some room for the overview maps.
    *   @param mapType The map type to write to the packet.
    *   @param zoomLevel The zoomlevel to write to the packet.
    *   @param mapVersion The version of the map.
    *   @param generatorVersion The version of the map generator.
    */
   void init(byte mapType,
             byte zoomLevel,
             uint32 mapVersion,
             uint32 generatorVersion);
   
   /** @return The map type of the request */
   byte getMapType() const;

   /** @return The map type of the request as a string */
   const char* getMapTypeAsString() const;

   /** Set the Type of the MapRequestPacket to newType */
   void setMapType(MapType newType);

   /** Get requested zoomlevel */
   byte getZoomlevel() const;

   /** Get requested zoomlevel */
   void setZoomlevel( byte zoomlevel );

   /** Write overview maps to the packet, sorted by level, lowest first */
   int setOverviewMaps(const vector<uint32>& maps);

   /** Get the overview maps from the packet, sorted by level, lowest first */
   int getOverviewMaps(vector<uint32>& maps) const;

   /**
    *   Returns the map version already cached by the sender of the pack.
    */
   uint32 getMapVersion() const;

   /**
    *   Returns the version of the map generator used to get the
    *   cached version of the map.
    */
   uint32 getMapGeneratorVersion() const;

   /**
    *   Returns true if a new map is needed.
    *   @param myVersion The version of the map in index.db.
    *   @param myGeneratorVersion The current version of the
    *                             generator of the right type.
    */
   bool newMapNeeded(uint32 myVersion,
                     uint32 myGeneratorVersion) const;
   
private:

   /** Position of map type byte */
   static const int MAP_TYPE_POS = REQUEST_HEADER_SIZE;

   /** Position of zoom level byte */
   static const int ZOOM_LEVEL_POS = MAP_TYPE_POS + 1;

   /** Position of map version */
   static const int MAP_VERSION_POS = ZOOM_LEVEL_POS + 1 + 2; // Has padding

   /** Position of map generator version */
   static const int MAP_GENERATOR_VER_POS = MAP_VERSION_POS + 4;

   /** Position of the number of overview maps */
   static const int NBR_OVERVIEW_POS = MAP_GENERATOR_VER_POS + 4;

   /** Position of pixel height */
   static const int FIRST_OVERVIEW_MAP_POS = NBR_OVERVIEW_POS + 4;
};


/**
   *  Packet that is sent as a reply to a MapRequestPacket
   *
   *  After the normal header this packet contains
   *  @packetdesc
   *     @row REPLY_HEADER_SIZE  @sep 4 bytes @sep IP from where the map can 
   *                                    be retrieved @endrow
   *     @row +4                 @sep 2 bytes @sep port to use @endrow
   *     @row +6                 @sep 2 bytes @sep padding. @endrow
   *     @row +8                 @sep 4 bytes @sep Map ID @endrow
   *     @row+16                 @sep 4 bytes @sep Map version @endrow
   *     @row+20                 @sep 4 bytes @sep Map generator ver @endrow
   *  @endpacketdesc
   *
   */
class MapReplyPacket : public ReplyPacket
{
 public:
      /**
        *   <b>Use this constructor!</b>
        *   Creates a  MapReplyPacket by using the corresponding question
        *   (a aMapRequestPacket).
        *
        *   @param   requestPacket is the request for this reply.
        *   @param   mapIP The IP from where the map will be sent. 
        *   @param   mapPort The port from where the map will be sent.
        *   @param   mapVersion The version of the map according to
        *                       index.db.
        */
      MapReplyPacket(const MapRequestPacket* requestPacket,
                     uint32 mapIP,
                     uint16 mapPort,
                     uint32 mapVersion       = MAX_UINT32,
                     uint32 generatorVersion = MAX_UINT32 );

      /** 
        *   @return The IP from where the map can be gotten. 
        */
      uint32 getReplyIP() const;
      
      /** 
        *   @return The port to connect to. 
        */
      uint16 getReplyPort() const;

      /**
       *    Get the ID of the map.
       *    @return The id of the map.
       */
      uint32 getMapID() const;

      /**
       *    Returns the version of the map.
       *    @return The version of the map.
       */
      uint32 getMapVersion() const;

      /**
       *    Returns the version of the generator of the map.
       */
      uint32 getGeneratorVersion() const;

      /**
       *     Returns true if a new map is needed.
       *     @param myVersion The currently cached map version.
       *     @param myGeneratorVersion The currently 
       */
      bool newMapNeeded(uint32 myVersion, uint32 myGeneratorVersion) const;
   
private:

   /// Position of the ip to connect to
   static const int REPLY_IP_POS      = REPLY_HEADER_SIZE;

   /// Position of the port to connect to
   static const int REPLY_PORT_POS    = REPLY_IP_POS    + 4;

   /// Position of the map ID
   static const int MAP_ID_POS        = REPLY_PORT_POS  + 4;

   /// Position of the map version.
   static const int MAP_VERSION_POS   = MAP_ID_POS      + 4;

   /// Position of the generator version
   static const int MAP_GENERATOR_POS = MAP_VERSION_POS + 4;
   
};

#endif


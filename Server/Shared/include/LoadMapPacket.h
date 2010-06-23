/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOADMAPPACKET_H
#define LOADMAPPACKET_H

#define LOAD_MAP_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define LOAD_MAP_REPLY_PRIO 0

#include "config.h"
#include "Packet.h"

/**
 *   Packet that is send from the Leader to the Available to make 
 *   them load a new map from the MapModule (or, if it's a MapModule
 *   that should load a new map, from disc).
 *   <br />
 *   
 */
class LoadMapRequestPacket : public RequestPacket {
public:
   /**
    *   Constructor.
    *   @param   mapID ID of the map the receiver should load.
    *   @param   originIP IP of the sender.      
    *   @param   originPort Port of the sender.      
    *   @param destIP   The ip that the request was sent to.
    *                   (JT does not know this).
    *   @param destPort The port of the receiver.
    */
   LoadMapRequestPacket( uint32 mapID,
                         uint32 originIP = 0,
                         uint16 originPort = 0,
                         uint32 destIP = 0,
                         uint16 destPort = 0);
   /**
    *   Returns the destination IP of the packet.
    */
   inline uint32 getMapLoaderIP() const;

   /**
    *   Returns the destination port of the packet.
    */
   inline uint32 getMapLoaderPort() const;
   
private:
   /** Position of destIP ( 4 bytes ) */
   static const int DEST_IP_POS = REQUEST_HEADER_SIZE;
   /** Position of destPort ( 4 bytes ) */
   static const int DEST_PORT_POS = DEST_IP_POS + 4;
   
};

/**
 *   Packet send as an ack. to the leader when the Module is 
 *   finished loading the map.
 *
 */
class LoadMapReplyPacket : public ReplyPacket {
   public:
   /**
    * Constructor.
    * @param req     The RequestPacket this reply is send as an
    *                answer/ack to.
    * @param status  The status of the maploading.
    * @param mapSize The size of the map. Used by Reader to
    *                balance memory use.
    *  
    */
   LoadMapReplyPacket( const LoadMapRequestPacket& req,
                       uint32 status,
                       uint32 mapSize );
   
   /**
    *   Set the mapID of this packet.
    *
    *   @param mapID The mapID to set.
    */
   inline void setMapID(uint32 mapID);
   
   /**
    *   Get the mapID of this packet.
    *
    *   @return The mapID of this packet.
    */
   inline uint32 getMapID();

   /**
    *   Returns the IP of the module that should load the map.
    */
   inline uint32 getMapLoaderIP() const;

   /**
    *   Returns the IP of the module that should load the map.
    */
   inline uint32 getMapLoaderPort() const;

   /**
    *   Returns the size of the loaded map.
    */
   inline uint32 getLoadedMapSize() const;

private:      
   /** The position in the packet of the mapID. */
   static const int MAP_ID_POS          = REPLY_HEADER_SIZE;
   /** The position of the maploader's ip */
   static const int DEST_IP_POS         = MAP_ID_POS + 4;
   /** The position of the maploader's port */
   static const int DEST_PORT_POS       = DEST_IP_POS + 4;
   /** The size of the loaded map */
   static const int LOADED_MAP_SIZE_POS = DEST_PORT_POS + 4;
      
}; // LoadMapReplyPacket

// ==========================================================================
//                                      Implementation of inlined functions =

// === LoadMapRequest
inline uint32
LoadMapRequestPacket::getMapLoaderIP() const
{
   return readLong(DEST_IP_POS);
}

inline uint32
LoadMapRequestPacket::getMapLoaderPort() const
{
   return readLong(DEST_PORT_POS);
}


// === LoadMapReply

inline void
LoadMapReplyPacket::setMapID(uint32 mapID)
{
   writeLong(MAP_ID_POS, mapID);
}


inline uint32
LoadMapReplyPacket::getMapID()
{
   return readLong(MAP_ID_POS);
}

inline uint32
LoadMapReplyPacket::getMapLoaderIP() const
{
   return readLong(DEST_IP_POS);
}

inline uint32
LoadMapReplyPacket::getMapLoaderPort() const
{
   return readLong(DEST_PORT_POS);
}

inline uint32
LoadMapReplyPacket::getLoadedMapSize() const
{
   return readLong(LOADED_MAP_SIZE_POS);
}

#endif // LOADMAPPACKET_H


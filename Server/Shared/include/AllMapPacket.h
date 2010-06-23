/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALLMAPPACKET_H
#define ALLMAPPACKET_H

#include "config.h"
#include "Packet.h"
#include "StringTable.h"
#include "MC2BoundingBox.h"

#define ALLMAP_REPLY_PRIO 0
#define ALLMAP_REQUEST_PRIO 0

/**
  *   Packet used to get information from the map module about all
  *   excisting maps in the system.
  *
  *   The different types of requests are:
  *   <ul>
  *      <li>Get the mapIDs of all known maps.
  *      <li>Get both mapIDs and boundingboxes for all known maps.
  *      <li>Get the mapIDs of all loaded maps.
  *   </ul>
  *
  *   The format of the AllMapRequestPacket after the header in the
  *   RequestPacket is:
  *   @packetdesc
  *      @row REQUEST_HEADER_SIZE @sep 4 bytes @sep The type of information 
  *                                                 wanted in reply. @endrow
  *   @endpacketdesc
  *
  */
class AllMapRequestPacket : public Packet
{
   public:
      /**
        *   What information to send for each map.
        */
      enum allmap_t { 
         /**
           *   Only send a list with the mapID's.
           */
         ONLY_MAPID  = 0,

         /**
           *   Send the boundingbox for each map.
           */
         BOUNDINGBOX = 1,

         /**
           *   Only send a list with the mapID's of the loaded maps.
           */
         ONLY_LOADED_MAPID  = 2,

         /**
           *   Send the bounding box and the version-number for each map.
           */
         BOUNDINGBOX_VER  = 3
      };

      /**
        *   Creates one request for all maps with a specified type.
        *   @param   type  The type of requested answer.
        */
      AllMapRequestPacket(allmap_t type);

      /**
        *   Creates a AllMapRequestPacket with specified originIP
        *   and port.
        *
        *   @param   aIP   IP of the sender.
        *   @param   aPort Portnumber that the sender whant the answer
        *                  send to.
        */      
      AllMapRequestPacket(uint32 aIP, uint16 aPort);

      /**
        *   Creates a AllMapRequestPacket with specified originIP
        *   and port.
        *
        *   @param   aIP   IP of the sender.
        *   @param   aPort Portnumber that the sender whant the answer
        *                  send to.
        *   @param   type  How much information want in the reply.
        */
      AllMapRequestPacket(uint16 reqID, uint16 packetID, allmap_t type);

      /**
        *   Deletes the object and releases the memory.
        */
      virtual ~AllMapRequestPacket();

      /**
        *   Used to get the type of the answer.
        *   @return  What kind of answer wanted.
        */
      allmap_t getType() const;
};


/**
  *   The reply to a all map request packet. Contains information
  *   about the excisting maps.
  *   @packetdesc
  *      @row HEADER_SIZE   @sep 4 bytes @sep nbrMaps @endrow
  *      @row HEADER_SIZE+4 @sep 4 bytes @sep stringCode @endrow
  *      @row HEADER_SIZE+8 @sep 4 bytes @sep replyType@endrow
  *   @endpacketdes
  *
  */
class AllMapReplyPacket : public Packet
{

   public:
      /**
        *   Creates an empty AllMapReplyPacket
        */
      AllMapReplyPacket();

      /**
        *   Set the common part of the request and the answer.
        */
      AllMapReplyPacket(const AllMapRequestPacket* p);

      /**
        *   Delete this packet, and release the memory.
        */
      virtual ~AllMapReplyPacket();

      /**
        *   Get the type of the provided information.
        *   @return  The type of the information in this reply.
        */
      AllMapRequestPacket::allmap_t getType();

      /**
        *   Get a map ID from this packet.
        *   @param   i  The map number ( 0 <= i < nbrMaps).
        *   @return  id of map number i.
        */
      uint32 getMapID(uint32 i);

      /**
        *   Get a map version from this packet.
        *   @param   i  The map number ( 0 <= i < nbrMaps).
        *   @return  version of map number i, creation time in UNIX time
        *            set to 0 if there's no version info in this packet
        */
      uint32 getMapVersion(uint32 i);

      /**
        *   Method to set boundingbox (if type == BOUNDINGBOX) number i.
        *   @param   i     The index of the boundingbox to set.
        *   @param   bbox  The bounding box number i.
        */
      void setMC2BoundingBox(uint32 i, MC2BoundingBox* bbox);

      /**
        *   Method to set boundingbox (if type == BOUNDINGBOX_VER) 
        *   number i.
        *   @param   i        The index of the boundingbox to set.
        *   @param   bbox     The bounding box number i.
        *   @param   version  Outparameter that is set to the version of
        *                     this map. The version is the creation time
        *                     in UNIX-time.
        */
      void setMC2BoundingBox(uint32 i, MC2BoundingBox* bbox, uint32& version);

      /**
        *   Get the number of maps in this packet.
        *   @return  The number of mapID's in this packet.
        */
      uint32 getNbrMaps();

      /**
        *   Add one map to this packet.
        *   @param   mapID MapID of the map to add to the packet.
        */
      void addLast(uint32 mapID);

      /**
        *   Add one map, including it's boundingbox, to this packet.
        *   @param   mapID    MapID of the map to add to the packet.
        *   @param   maxLat   The maximum latitude of this map.
        *   @param   minLat   The minimum latitude of this map.
        *   @param   maxLon   The maximum longitude of this map.
        *   @param   minLon   The minimum longitude of this map.
        */
      void addLast(uint32 mapID, int32 maxLat, int32 minLat, 
                                 int32 maxLon, int32 minLon);

      /**
       *    AddLast but with bounding box.
       */
      void addLast( uint32 mapID, const MC2BoundingBox& bbox );

      /**
        *   Add one map, including the version and boundingbox, to 
        *   this packet.
        *   @param   mapID    MapID of the map to add to the packet.
        *   @param   version  The time when this map was created.
        *   @param   maxLat   The maximum latitude of this map.
        *   @param   minLat   The minimum latitude of this map.
        *   @param   maxLon   The maximum longitude of this map.
        *   @param   minLon   The minimum longitude of this map.
        */
      void addLast(uint32 mapID, uint32 version,
                   int32 maxLat, int32 minLat, 
                   int32 maxLon, int32 minLon);

      /// AddLast with bounding box and version.
      void addLast( uint32 mapID,
                    uint32 version,
                    const MC2BoundingBox& bbox );

      /**
        *   Get the status of this reply.
        *   @return  StatusCode of the reply.
        */
      StringTable::stringCode getStatusCode();

      /**
        *   Set the status of the reply.
        *   @param   code  The new value of the status.
        */
      void setStatusCode(StringTable::stringCode code);
};

#endif



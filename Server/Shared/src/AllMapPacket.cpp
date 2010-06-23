/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AllMapPacket.h"

// ******************************************************************
//                                                AllMapRequestPacket
AllMapRequestPacket::AllMapRequestPacket(uint32 aIP, uint16 aPort)
  : Packet( HEADER_SIZE+4,
            ALLMAP_REQUEST_PRIO, 
            PACKETTYPE_ALLMAPREQUEST, 
            aIP,
            aPort,
            0, // packetID
            0, // reqID
            0 /* deb */ ) 
{
   writeLong(HEADER_SIZE, 0);    // 0 = ONLY_MAPID
   setLength(HEADER_SIZE+4);
}

AllMapRequestPacket::AllMapRequestPacket( uint16 reqID, 
                                          uint16 packID, 
                                          allmap_t type)
  : Packet( HEADER_SIZE+4, 
            ALLMAP_REQUEST_PRIO,
            PACKETTYPE_ALLMAPREQUEST,
            0, // originIP
            0, // originPort
            reqID, // packetID
            packID, // reqID
            0 /* deb */ ) 
{
   writeLong(HEADER_SIZE, type);
   setLength(HEADER_SIZE+4);
}

AllMapRequestPacket::AllMapRequestPacket( allmap_t type )
  : Packet( HEADER_SIZE+4,
            ALLMAP_REQUEST_PRIO,
            PACKETTYPE_ALLMAPREQUEST,
            0, // originIP
            0, // originPort
            0, // packetID
            0, // reqID
            0 /* deb */ ) 
{
   writeLong(HEADER_SIZE, type);
   setLength(HEADER_SIZE+4);
}

AllMapRequestPacket::~AllMapRequestPacket()
{
}

AllMapRequestPacket::allmap_t
AllMapRequestPacket::getType() const {
   return (allmap_t(readLong(HEADER_SIZE)));
}



// ******************************************************************
//                                                  AllMapReplyPacket
AllMapReplyPacket::AllMapReplyPacket() 
         : Packet( 65535,
                   0,
                   0,
                   0,
                   0,
                   0,
                   0,
                   0)
{
   setLength(HEADER_SIZE);
}


AllMapReplyPacket::AllMapReplyPacket( const AllMapRequestPacket *p)
         : Packet( 65535,
            ALLMAP_REPLY_PRIO,
            PACKETTYPE_ALLMAPREPLY, 
            p->getOriginIP(),
            p->getOriginPort(),
            p->getPacketID(),
            p->getRequestID(),
            p->getDebInfo())
{
   int position = HEADER_SIZE;
   incWriteLong(position, 0);             // Initialize nbr mapID's
   incWriteLong(position, 0);             // Initialize stringCode
   incWriteLong(position, p->getType());  // Set type of reply
   setLength(position);
}


AllMapReplyPacket::~AllMapReplyPacket()
{

}

uint32
AllMapReplyPacket::getMapID(uint32 i)
{
   if (i < getNbrMaps() ) {
      switch( getType() ) {
         case AllMapRequestPacket::ONLY_MAPID :
         case AllMapRequestPacket::ONLY_LOADED_MAPID :
            return (readLong(HEADER_SIZE+12 + i*4));

         case AllMapRequestPacket::BOUNDINGBOX :
            return (readLong(HEADER_SIZE+12 + (i)*20));

         case AllMapRequestPacket::BOUNDINGBOX_VER :
            return (readLong(HEADER_SIZE+12 + (i)*24));

         default:
            return (MAX_UINT32);
      }
   } 
   else {
      return (MAX_UINT32);
   }
}

uint32
AllMapReplyPacket::getMapVersion(uint32 i)
{
   uint32 version;

   version = 0;

   if (getType() == AllMapRequestPacket::BOUNDINGBOX_VER)
      version = readLong(HEADER_SIZE+12+i*24+4);

   return version;
}

uint32
AllMapReplyPacket::getNbrMaps()
{
   return (readLong(HEADER_SIZE));
}

void
AllMapReplyPacket::addLast(uint32 mapID)
{
   int position = getLength();
   incWriteLong(position, mapID);
   writeLong(HEADER_SIZE, getNbrMaps()+1);
   setLength(position);
}

void
AllMapReplyPacket::addLast(uint32 mapID,  int32 maxLat, int32 minLat, 
                                          int32 maxLon, int32 minLon)
{
   int position = getLength();
   incWriteLong(position, mapID);
   incWriteLong(position, maxLat);
   incWriteLong(position, minLat);
   incWriteLong(position, maxLon);
   incWriteLong(position, minLon);
   writeLong(HEADER_SIZE, getNbrMaps()+1);
   setLength(position);
}

void
AllMapReplyPacket::addLast( uint32 mapID, const MC2BoundingBox& bbox )
{
   addLast( mapID,
            bbox.getMaxLat(), bbox.getMinLat(),
            bbox.getMaxLon(), bbox.getMinLon() );
            
}

void
AllMapReplyPacket::addLast(uint32 mapID, uint32 version,
                           int32 maxLat, int32 minLat, 
                           int32 maxLon, int32 minLon)
{
   int position = getLength();
   incWriteLong(position, mapID);
   incWriteLong(position, version);
   incWriteLong(position, maxLat);
   incWriteLong(position, minLat);
   incWriteLong(position, maxLon);
   incWriteLong(position, minLon);
   writeLong(HEADER_SIZE, getNbrMaps()+1);
   setLength(position);
}

void
AllMapReplyPacket::addLast( uint32 mapID,
                            uint32 version,
                            const MC2BoundingBox& bbox )
{
   addLast( mapID, version,
            bbox.getMaxLat(), bbox.getMinLat(),
            bbox.getMaxLon(), bbox.getMinLon() );
            
}


StringTable::stringCode
AllMapReplyPacket::getStatusCode()
{
   return (StringTable::stringCode(readLong(HEADER_SIZE+4)));
}

void
AllMapReplyPacket::setStatusCode(StringTable::stringCode code)
{
   writeLong(HEADER_SIZE+4, code);
}

AllMapRequestPacket::allmap_t
AllMapReplyPacket::getType()
{
   return ( AllMapRequestPacket::allmap_t(readLong(HEADER_SIZE+8)));
}

void
AllMapReplyPacket::setMC2BoundingBox(uint32 i, MC2BoundingBox* bbox)
{
   if (getType() == AllMapRequestPacket::BOUNDINGBOX) {
      int position = HEADER_SIZE+12+i*20+4;
      bbox->setMaxLat( (int32) incReadLong(position) );
      bbox->setMinLat( (int32) incReadLong(position) );
      bbox->setMaxLon( (int32) incReadLong(position) );
      bbox->setMinLon( (int32) incReadLong(position) );
   } else {
      bbox->setMaxLat(0);
      bbox->setMinLat(0);
      bbox->setMaxLon(0);
      bbox->setMinLon(0);
   }
   bbox->updateCosLat();
}

void
AllMapReplyPacket::setMC2BoundingBox(uint32 i, 
                                     MC2BoundingBox* bbox,
                                     uint32& version)
{
   if (getType() == AllMapRequestPacket::BOUNDINGBOX_VER) {
      int position = HEADER_SIZE+12+i*24+4;
      version = incReadLong(position);
      bbox->setMaxLat( (int32) incReadLong(position) );
      bbox->setMinLat( (int32) incReadLong(position) );
      bbox->setMaxLon( (int32) incReadLong(position) );
      bbox->setMinLon( (int32) incReadLong(position) );
   } else {
      version = MAX_UINT32;
      bbox->setMaxLat(0);
      bbox->setMinLat(0);
      bbox->setMaxLon(0);
      bbox->setMinLon(0);
   }
   bbox->updateCosLat();
}


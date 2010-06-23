/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INFO_COORDINATE_PACKET_H
#define INFO_COORDINATE_PACKET_H

#include "config.h"
#include "MC2Coordinate.h"
#include "Packet.h"

/**
 * Represents a single coordinate-angle pair included in a
 * InfoCoordinateRequestPacket.
 */
class InfoCoordinate 
{
public:
   /** @name Constructors */
   //@{
   /**
    * Default constructor. Sets the member variable coord to the
    * default value for a MC2Coordinate, and the angle member variable
    * to MAX_INT32.
    */
   InfoCoordinate() :coord(), angle(MAX_INT32)
   {}
   /**
    * Constructor.
    * @param pCoord The coordinate value. 
    * @param pAngle The angle value. 
    */
   InfoCoordinate(const MC2Coordinate& pCoord, uint32 pAngle) : 
      coord(pCoord), angle(pAngle)
   {}
   //@}

   /**
    * @name Reading and writing to a packet. 
    *  4 bytes lat
    *  4 bytes lon
    *  4 bytes angle
    */
   //@{
   /**
    * Write itself into a packet. 
    * @param packet The packet to write into.
    * @param pos The position of the packet to start writing at. Will
    *            be updated to point to after the written data.
    */
   void save(Packet* packet, int& pos) const;
   /**
    * Update itself with data read from a packet.
    * @param packet The packet to read from
    * @param pos The position of the packet to start reading from. Will
    *            be updated to point to after the read data.
    */
   void load( const Packet* packet, int& pos) ;
   //@}

   /**@name Member data */
   //@{
   /** The coordinate data */
   MC2Coordinate coord;
   /** The angle */
   uint32 angle;
   //@}
};

/**
 * Requests information regarding coordinate-angle pairs. 
 */
class InfoCoordinateRequestPacket: public RequestPacket
{
public:
   /** The container for coordinate-angle pairs. */
   typedef std::vector<InfoCoordinate> InfoCoordCont;
   /** @name Constructors. */
   //@{
   /**
    * Constructor for a packet requesting information about a single
    * coordinate-angle pair.
    * @param The map this packet pertains.
    * @param coord The coordinate.
    * @param angle The angle. Defaults to MAX_INT16.
    */
   InfoCoordinateRequestPacket(uint32 mapID,
                               const MC2Coordinate& coord,
                               uint32 angle = MAX_INT16);

   /**
    * Constructor for a packet requesting information about several
    * coordinate-angle pair.
    * @param The map this packet pertains.
    * @param coords The coordinate-angle pairs. 
    */   
   InfoCoordinateRequestPacket(uint32 mapID,
                               const InfoCoordCont& coords);

   //@}
   /**
    * Reads the data encoded in the packet into a InfoCoordsCont. 
    * @param coords The container that the read InfoCoordinates will be stored.
    * @return The number of InfoCoordinate objects that was read. 
    */
   InfoCoordCont::size_type 
   readData(InfoCoordCont& coords) const;
   
private:
   /**
    * Writes the InfoCoordinates from a InfoCoordCont into this
    * packet.
    * @param coords The InfoCoordCont. 
    */
   void encodeRequest(const InfoCoordCont& coords);
};

/**
 * Objects of this class are stored in the InfoCoordinateReplyPacket.
 */
class InfoCoordinateReplyData
{
public:
   /** @name Constuctors. */
   //@{
   /**
    * Default constructor. Sets all member variables to MAX_UINT32,
    * which represents an invalid object.
    */ 
   InfoCoordinateReplyData() :
      mapID(MAX_UINT32), nodeID0(MAX_UINT32), nodeID1(MAX_UINT32), 
      distance(MAX_UINT32), offset(MAX_UINT32)
   {}

   /**
    * Constructor. 
    */
   InfoCoordinateReplyData(uint32 mapID, uint32 nodeID0, uint32 nodeID1, 
                           uint32 distance, uint32 offset) : 
      mapID(mapID), nodeID0(nodeID0), nodeID1(nodeID1), distance(distance), 
      offset(offset)
   {}

   /**
    * Stores this object in a packet.
    * @param packet The Packet to write into.
    * @param pos The position to start writing at. Will be updated to
    *            point past the written data.
    */
   void save(Packet* packet, int& pos) const; 
   /**
    * Updates this object from a packet.
    * @param packet The Packet to read from.
    * @param pos The position to start reading from. Will be updated to
    *            point past the read data.
    */
   void load(Packet* packet, int& pos);
   /**
    * Compares this object with the argument, and updates this object
    * with the best of the two. 
    * If the distance member variable is lower in the parameter object
    * the data from that object will be copied into this object.
    * @param data The object to compare to and maybe copy from. 
    */
   void merge( const InfoCoordinateReplyData& data );
   /** The id of the map this info is from */
   uint32 mapID;
   /** The first node ID */
   uint32 nodeID0;
   /** The second node ID */
   uint32 nodeID1;
   /** The distance to the segment?*/
   uint32 distance;
   /** The offset into the segment?*/
   uint32 offset;
};

ostream& operator<<(ostream& stream, const InfoCoordinateReplyData& icrd);

/**
 * The reply packet for InfoCoordinateRequestPacket.
 */
class InfoCoordinateReplyPacket: public ReplyPacket
{
public:
   /** 
    * The container type used to collect the InfoCoordinateReplyData
    * objects contained in the packet. 
    */
   typedef std::vector<InfoCoordinateReplyData> InfoReplyCont;
   /**
    * Constructor. 
    * @param request The packet the new object is a reply to.
    * @param status The status of the packet. 
    * @param data The objects to store in the packet. 
    */
   InfoCoordinateReplyPacket( const InfoCoordinateRequestPacket* request,
                              uint32 status, 
                              const InfoReplyCont& data );
   /**
    * Merge the InfoCoordinateReplyData objects in this packet with
    * the ones stored in the InfoReplyCont parameter.  The merge is
    * done by replacing any invalid objects in the parameter with
    * valid from this packet, or if this packet contains better
    * matches they will replace the ones in the parameter see
    * InfoCoordinateReplyData::merge.
    * @param data The container to merge into. 
    */
   void mergeData(InfoReplyCont& data);
private:
   /**
    * Write a set of InfoCoordinateReplyData objects into this packet.
    * @param data The objects to write.
    */
   void encodeReply(const InfoReplyCont& data);
};

#endif

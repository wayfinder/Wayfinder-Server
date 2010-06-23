/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EDGENODESPACKET_H
#define EDGENODESPACKET_H

#define EDGENODES_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define EDGENODES_REPLY_PRIO   DEFAULT_PACKET_PRIO

#define EDGENODES_REQUEST_MAX_LENGTH 256
#define EDGENODES_REPLY_MAX_LENGTH   65536

#include "config.h"
#include <set>
#include <vector>
#include <map>

#include "Packet.h"

class OrigDestInfoList;
class OrigDestInfo;

/**
 *   Packet used when getting the nodes on a map edge
 *   which also exist on the supplied level.
 *   After the normal RequestPacket header the packet
 *   contains (X is REQUEST_HEADER_SIZE):
 *   @packetdesc
 *      @row X     @sep 4 bytes @sep Level to check @endrow
 *      @row X + 4 @sep 4 bytes @sep User defined data.
 *                                   Should be copied to the reply. @endrow
 *      @row X + 8 @sep 4 bytes @sep Vehicle to check for allowance @endrow
 *   @endpacketdesc
 */
class EdgeNodesRequestPacket : public RequestPacket {

public:
   
   /**
    *    Creates a new EdgeNodesRequestPacket for the
    *    specified <code>mapID</code> and <code>level</code>.
    *    Also implies that the RouteModule should try to avoid
    *    nodes that cannot be used by the supplied vehicle.
    *    @param packetID  The packetID of the packet.
    *    @param requestID The requestID of the packet.
    *    @param mapID     The map to check.
    *    @param level     The absolute level where the nodes must exist.
    *    @param dests     The destinations (if any).
    *    @param vehicles  The vi
    *    @param userDef   User defined data. Should be copied to the reply
    *                     by the module.
    */
   EdgeNodesRequestPacket(uint32 packetID,
                          uint32 requestID,
                          uint32 mapID,
                          uint32 level,
                          const OrigDestInfoList& dests,
                          uint32 userDef = MAX_UINT32,
                          uint32 vehicles = MAX_UINT32);
   /**
    *    Returns the level return external nodes for.
    */
   uint32 getLevel() const;

   /**
    *    Returns the user defined data.
    */
   uint32 getUserDefinedData() const;

   /**
    *    Returns the vehicle mask of the user.
    */
   uint32 getVehicle() const;

   /**
    *    Adds the destinations to the list <code>dests</code>.
    *    @param dests List to add the destinations to.
    *    @return nbr of destinations.
    */
   int getDestinations(OrigDestInfoList& dests) const;
   
private:

   /**
    *    Adds an OrigDestInfo to the packet at position pos.
    */
   int writeOrigDestInfo(int& pos, const OrigDestInfo& info);

   /**
    *    Returns an OrigDestInfo from the Packet at position pos.
    *    Note that the OrigDestInfo is not complete.
    */
   OrigDestInfo* readOrigDestInfo(int& pos) const;
   
   /**   The position of the level variable */
   static const int LEVEL_POS = REQUEST_HEADER_SIZE;

   /** The position of the user defined data */
   static const int USER_DEFINED_POS = LEVEL_POS + 4;

   /** The position of the vehicle */
   static const int VEHICLE_POS = USER_DEFINED_POS + 4;

   /** The position of the number of OrigDestInfos */
   static const int DEST_NBR_POS = VEHICLE_POS + 4;

   /** The position of the start of the OrigDestInfos */
   static const int DEST_START_POS = DEST_NBR_POS + 4;
   
};

// -----------------------------------------------------------------
//  Implementation of inlined methods for EdgeNodesRequestPacket
// -----------------------------------------------------------------
inline uint32
EdgeNodesRequestPacket::getLevel() const
{
   return readLong(LEVEL_POS);
}

inline uint32
EdgeNodesRequestPacket::getUserDefinedData() const
{
   return readLong(USER_DEFINED_POS);
}

inline uint32
EdgeNodesRequestPacket::getVehicle() const
{
   return readLong(VEHICLE_POS);
}


/**
 *   Packet containing information about the edgenodes in
 *   a RoutingMap. Packet contents:
 *   @packetdesc
 *      @row X       @sep 4 bytes @sep MapID.                    @endrow
 *      @row X +  4  @sep 4 bytes @sep Requested level.          @endrow
 *      @row X +  8  @sep 4 bytes @sep User defined data.        @endrow
 *      @row X + 12  @sep 4 bytes @sep 0x0000.                   @endrow
 *      @row X + 16  @sep 4 bytes @sep 0xffff.                   @endrow
 *      @row X + 20  @sep 4 bytes @sep MapID of overview map.    @endrow
 *      @row X + 24  @sep 4 bytes @sep Number of bordering maps. @endrow
 *      @row X + 32  @sep 4 bytes @sep Number of external nodes on
 *                                     requested level.          @endrow
 *      @row X + 40  @sep 4 bytes @sep Total number of external nodes.
 *                                                               @endrow
 *      @row X + 48  @sep 4 bytes * number of bordering maps
 *                   @sep MapID:s for bordering maps             @endrow
 *      @row X + 4*number of bordering maps
 *                   @sep (3 * 4 bytes) * number of external nodes
 *                   @sep ItemID, lat and lon for external nodes @endrow
 *                      
 *   @endpacketdesc
 */
class EdgeNodesReplyPacket : public ReplyPacket {

public:

   /**
    *   Creates a new EgdenodesReplyPacket from a requestpacket
    *   and more data.
    *   @param req The EdgeNodesRequestPacket that caused the reply.
    *   @param status The status of the reply.
    *   @param overviewMapID The ID of the overview map to this map.
    *   @param borderMaps    The maps to which this map has connections.
    *   @param externalNodes The external node ID:s on this map.
    *   @param lats          The latitudes of the nodes.
    *   @param lons          The longitudes of the nodes.
    *   @param distances     Map containing map id in first and minimum
    *                        distance from a destination in second.
    *   @param totalNbrExt   The total number of external nodes for the
    *                        map on level 0.
    */
   EdgeNodesReplyPacket(const EdgeNodesRequestPacket* req,
                        uint32 status,
                        uint32 overviewMapID,
                        const set<uint32>& borderMaps,
                        const vector<uint32>& externalNodes,
                        const vector<int32>& lats,
                        const vector<int32>& lons,
                        const map<uint32, uint32>& distances,
                        int totalNbrExt);

   /**
    *   Creates a new EdgeNodesReplyPacket, probably with 
    *   an error code.
    *   @param req The requestPacket.
    *   @param status The status of the reply.
    */
   EdgeNodesReplyPacket(const EdgeNodesRequestPacket* req,
                        uint32 status);

   /**
    *   Fills the supplied OrigDestInfoList with the edgenodes
    *   for the current level. The list is not emptied before
    *   adding. The Vehicles of the nodes will be <code>NULL</code>.
    *   @param nodeList  List to put the nodes into.
    *   @param distances Map to put the distances to other maps into.
    *   @param flipIDs   True if the node-id:s should be flipped (for
    *                    destination nodes).
    *   @return The number of nodes added.
    */
   int getEdgeNodes(OrigDestInfoList& nodeList,
                    map<uint32,uint32>* distances = NULL,
                    bool flipIDs = false) const;

   /**
    *   Returns the overview map id.
    */
   inline uint32 getOverviewMapID() const;

   /** Returns the map ID */
   inline uint32 getMapID() const;
   
   /**
    *   Fills the supplied set with the ID:s of the borderMaps.
    *   @param borderMaps The neighbours to this map.
    *   @param distances  The smallest distances from the destinations
    *                     to one of the edgenodes. MapID in first and
    *                     distance in m in second.
    *   @return Number of bordering maps.
    */
   int getBorderMaps(set<uint32>& borderMaps) const;

   /**
    *    Returns the user defined data.
    */
   uint32 getUserDefinedData() const;
   
private:

   /** Position for the Map id */
   static const int MAP_ID_POS            = REPLY_HEADER_SIZE;
   /** Position for level */
   static const int LEVEL_POS             = MAP_ID_POS          + 4;
   /** Position of user defined data */
   static const int USER_DEFINED_POS      = LEVEL_POS           + 4;
   /** Position of four bytes of zeroes */
   static const int LONG_ZERO_POS         = USER_DEFINED_POS    + 4;
   /** Position of four bytes of all ones */
   static const int LONG_FF_POS           = LONG_ZERO_POS       + 4;
   /** Position for the overview map id */
   static const int OVERVIEW_MAP_ID_POS   = LONG_FF_POS         + 4;
   /** Position of number of bordering maps */
   static const int NBR_BORDER_MAPS_POS   = OVERVIEW_MAP_ID_POS + 4;
   /** Position of number of external nodes */
   static const int NBR_EXT_NODES_POS     = NBR_BORDER_MAPS_POS + 4;
   /** Position of the total number of external nodes on all levels */
   static const int TOTAL_NBR_EXT_POS     = NBR_EXT_NODES_POS   + 4;
   /** Position of the start of the list of border maps */
   static const int BORDER_MAPS_START_POS = TOTAL_NBR_EXT_POS   + 4;

   /** The size of one bordermap */
   static const int BORDER_MAP_SIZE = 4;

   /** Returns the number of bordering maps */
   inline int getNbrBorderMaps() const;

   /**
    *  Writes the start of the packet.
    *  @param pos The position after the start of the data will be here.
    *  @param req The request to get data from.
    */
   inline void writePacketStart(int& pos,
                                const EdgeNodesRequestPacket* req);
   

   /** Returns the number of edgenodes */
   inline int getNbrEdgeNodes() const;
   
   /** Returns the position where the nodes start */
   inline int getNodesStartPos() const;
   
   /** Writes the bordermapIDs to the packet */
   inline void writeBorderMaps(int& pos,
                               const set<uint32>& borderMaps);

   /** Writes the external nodes and lat/lons to the packet */
   inline void writeExternalNodes(int& pos,
                                  const vector<uint32>& externalNodes,
                                  const vector<int32>& lats,
                                  const vector<int32>& lons);

   /** Writes the clostest distances to the other maps */
   inline void writeDistances(int& pos,
                              const map<uint32,uint32>& distances);
   
};

// -----------------------------------------------------------------
//  Implementation of inlined methods for EdgeNodesReplyPacket
// -----------------------------------------------------------------

inline uint32
EdgeNodesReplyPacket::getMapID() const
{
   return readLong( MAP_ID_POS );
}

inline uint32
EdgeNodesReplyPacket::getUserDefinedData() const
{
   return readLong( USER_DEFINED_POS );
}


inline uint32
EdgeNodesReplyPacket::getOverviewMapID() const
{
   return readLong( OVERVIEW_MAP_ID_POS);
}

inline int
EdgeNodesReplyPacket::getNbrBorderMaps() const
{
   return readLong( NBR_BORDER_MAPS_POS );
}

inline int
EdgeNodesReplyPacket::getNbrEdgeNodes() const
{
   return readLong( NBR_EXT_NODES_POS );
}


inline int
EdgeNodesReplyPacket::getNodesStartPos() const
{
   return BORDER_MAPS_START_POS + getNbrBorderMaps() * BORDER_MAP_SIZE;
}


#endif

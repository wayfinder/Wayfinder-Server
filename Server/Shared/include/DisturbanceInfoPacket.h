/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCEINFOPACKET_H
#define DISTURBANCEINFOPACKET_H

#define DISTURBANCE_REQUEST_PRIO DEFAULT_PACKET_PRIO

#include "Packet.h"
#include "DisturbanceElement.h"
#include "DisturbancePacketUtility.h"
#include "IDPairVector.h"
#include "TrafficDataTypes.h"

#include <map>
#include <vector>

class UserUser;
class UserRightsMapInfo;
class DisturbanceDescription;

/**
  *   Packet to request disturbance information for specified nodes
  *  
  *
  *   Table begins at REQUEST_HEADER_SIZE.
  *   @packetdesc
  *      @row 0     @sep 2 bytes @sep  Number of nodes in the request @endrow
  *      @row 2     @sep 2 bytes @sep  Current Map pos
  *      @row 4     @sep 2 bytes @sep  Current Node pos
  *      @row 4     @sep 4 bytes @sep  NumberOfMaps
  *      @row 8     @sep 4 bytes @sep  FirstMapID
  *      @row 12    @sep 4 bytes @sep  NumberOfNodes on map
  *      @row 16    @sep 4 bytes @sep  NodeID
  *
  *
  *
  *
  *
 */
class DisturbanceInfoRequestPacket : public RequestPacket
{
  static const int NUMBER_OF_NODES    = REQUEST_HEADER_SIZE;
  static const int START_OF_NODES_POS = REQUEST_HEADER_SIZE + 4;
  static const int USER_RIGHTS_POS    = REQUEST_HEADER_SIZE + 8;
  public:  

  struct detour_t{
     IDPair_t splitPoint;
     IDPair_t distPoint;
     IDPair_t joinPoint;
  };
  
  
  
   /**
    *
    */
  DisturbanceInfoRequestPacket(const UserUser* user,
                               uint32 mapID,
                               uint32 originIP   = 0,
                               uint16 originPort = 0);
   
   ~DisturbanceInfoRequestPacket();
   
   
      
   /**
    *   Add a node to ask for disturbances on.
    *   @param mapID
    *   @param nodeID
    */
   void addNode(uint32 mapID, uint32 nodeID);
   
   void addDisturbanceDescription(const DisturbanceDescription& dist);
   
   void addDisturbanceVector(const vector<DisturbanceDescription>& distVect);
   
   /**
    *   Add all the nodes in a IDPair vector.
    *   @param mapID
    *   @param nodeID
    */
   void addNodes(vector<IDPair_t> nodes);
   
   /**
    *   Add all the nodes in a IDPair vector.
    *   @param mapID
    *   @param nodeID
    */
   // void addDetourNodes(vector<detour_t> nodes);
   
   int getRights( UserRightsMapInfo& rights ) const;
   
   /**
    *
    *   @return The total number of nodes in this packet
    */
   uint32 getNumberOfNodes() const;

   
   /**
    *
    *   @return The total number of nodes in this packet
    */
   uint32 getFirstNodePosition() const;
   
   /**
    *   Get the nodeID and mapID of a node in the packet
    *   @param index the index of this node in the packet
    *   @return 
    */
   IDPair_t getMapAndNode(uint32 index) const;

   /**
    *   Get the number of nodes on a given map
    *   @param
    *   @return
    */
   uint32 getNumberOfNodesOnMap(uint32 mapID) const;

   /**
    *   Get a node of the given map.
    *   @param
    *   @return
    */
   uint32 getNodeOnMap(uint32 mapId, uint32 index) const;
   
};



/// ReplyPacket

/**
 *   
 *
 */
class DisturbanceInfoReplyPacket : public ReplyPacket
{
   static const int NUMBER_OF_DIST = REPLY_HEADER_SIZE;
   static const int START_OF_DIST  = REPLY_HEADER_SIZE + 4;

  public:

   /**
    *   Returns a a map with the disturbances on a map.
    *   @param p The DisturbanceRequestPacket.
    *   @param distMap The map with the disturbances.
    *   @param removeDisturbances True if the disturbances are going to
    *   be removed.
    */
   DisturbanceInfoReplyPacket(const DisturbanceInfoRequestPacket* p);
   
   /**
    *  Destructor
    *
    */
   virtual ~DisturbanceInfoReplyPacket();

   
   /**
    *  Add a information of a disturbance to the packet.
    *
    */
   void addDisturbance(uint32 mapID, uint32 nodeID,
                       TrafficDataTypes::disturbanceType type,
                       uint32 distID,
                       MC2String comment);
   

   /**
    *
    *   @return The total number of disturbances in this packet
    */
   uint32 getNumberOfDist() const;

   void setNumberOfDist(uint32 nbr);
   
   /**
    *   NB: Not safe to use anymore !!!   
    *   Get the the disturbance with a given index
    *   @param
    *   @return
    */
   // bool getDisturbance(uint32 index,
   //                     uint32 &mapID, uint32 &nodeID,
   //                     TrafficDataTypes::disturbanceType &type,
   //                     uint32 &distID,
   //                     MC2String &comment);

   /**
    *   NB: Not safe to use anymore !!!   
    *   Get the next disturbance. If no pos value is given this will
    *   be the first disturbance. The return value is used as next pos.
    *   @param
    *   @return
    */
   // int getNextDisturbance( uint32 &mapID, uint32 &nodeID,
    //                       TrafficDataTypes::disturbanceType &type,
    //                       uint32 &distID,
    //                       MC2String &comment, int pos = START_OF_DIST);

   /**
    *   Get the next disturbance. If no pos value is given this will
    *   be the first disturbance. The return value is used as next pos.
    *   @param
    *   @return
    */
   int getNextDistID( uint32 &mapID, uint32 &nodeID, uint32 &distID,
                      int pos = START_OF_DIST);

   /**
    *   Get the next disturbance Info. If no pos value is given this will
    *   be the first disturbance. The return value is used as next pos.
    *   @param
    *   @return
    */
   int getNextDistInfo(TrafficDataTypes::disturbanceType &type,
                       MC2String &comment, int pos);

   

   /**
    *   Add the return info to the vector. If the distPoint equals
    *   the mapID.nodeID of the returned description that element is
    *   updated. If that element already has been updated (multiple
    *   disturbances) a new element is added.
    *
    *   @param vector The vector with disturbances and detours.
    *   @return -1 if failed other wise the number of updated elements.
    */
   int fillDisturbanceVector(vector<DisturbanceDescription> &distVector);


   set<uint32>  m_storedDisturbanceIDs;

   
};


#endif // DISTURBANCEINFOPACKET_H
















/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LEVELTRANSITOBJECT_H
#define LEVELTRANSITOBJECT_H

#include "config.h"

#include <set>
#include <map>

#include "StringTable.h"
#include "PacketContainerTree.h"
#include "OrigDestInfo.h"
#include "IDPairVector.h"

class EdgeNodesReplyPacket;     // forward decl
class IDTranslationReplyPacket; // forward decl
class PacketContainer;          // forward decl
class Request;                  // forward decl
class DriverPref;               // forward decl
class SubRoute;                 // forward decl

/**
 *    This class does all the tasks involved in going up and down
 *    a level in the routing. That includes finding origina and
 *    destination nodes on higher level, and
 *    keeping track of which maps to route on after routing on higher level.
 *
 */

class LevelTransitObject {
public:

   /**
    *    Main constructor of the class
    */
   LevelTransitObject( Request* request,
                       OrigDestInfoList* origList,
                       const OrigDestInfoList* destList,
                       uint32 level,
                       const DriverPref* driverPrefs);

   /**
    *    Delete this LevelTransitObject, and release allocated memory.
    */
   virtual ~LevelTransitObject();

   /**
    *    Handles an incoming answer containing a Packet 
    *    rebuilds the outgoing list, alternatively
    *    constructing the finished output.
    *    @param p The PacketContainer to handle.
    */
   void processPacket( const PacketContainer* pc );
   
   /**
    *    Returns the top RequestPacket on m_outgoingQueue.
    *    @return  Container with request packet
    */
   PacketContainer* getNextPacket();

   /**
    *    Returns true if specified mapID is in m_destMapSet
    *    @return  true if destination mapID is OK, false otherwise
    */
   bool allowedMapID( uint32 mapID );

   /**
    *    Returns an OrigDestInfo from m_matchHighLowMap specified
    *    by mapID and nodeID.
    *    @return  Node on low level that matches a high level node.
    */
   OrigDestInfo* getLowerLevelDest( uint32 nodeID,
                                    uint32 mapID );

   /**
    *    @return  m_higherLevelOrigList
    */
   inline OrigDestInfoList* getHigherLevelOrigList();

   /**
    *    @return  m_higherLevelDestList
    */   
   inline OrigDestInfoList* getHigherLevelDestList();

   /**
    *    Returns the current status of the object
    *    @return  The current status of this object
    */
   inline StringTable::stringCode getStatus() const;

   /**
    *    Returns true if there is nothing more for this object to do
    *    @return  true if there is nothing more for this object to do
    */
   inline bool requestDone();

   /**
    *    Returns true if the supplied SubRoute is a destination.
    *    @param subRoute The SubRoute to check.
    *    @return True if the SubRoute is in the list of destinations.
    */
   bool isDest(const SubRoute* subRoute) const;
   
private:

   /**
    *    Creating all the EdgeNodesPackets for origins and dests
    *    before changing the state to FINDING_NODES
    */
   void gotoFindingNodes( );
   
   /**
    *    Called by processPacket when in state FINDING_NODES
    *    and packet type is EDGENODESREPLY. In UserDefinedData
    *    is info about this packet containing origins.
    */
   void processFindingOrigs( EdgeNodesReplyPacket* packet );
   
   /**
    *    Called by processPacket when in state FINDING_NODES
    *    and packet type is EDGENODESREPLY. In UserDefinedData
    *    is info about this packet containing dests.
    */
   void processFindingDests( EdgeNodesReplyPacket* packet );
   
   /**
    *    Called by processPacket when in state FINDING_NODES
    *    and packet type is IDTRANSLATIONREPLY. In UserDefinedData
    *    is info about this packet containing origins.
    */
   void processOrigTranslation( IDTranslationReplyPacket* packet );

   /**
    *    Called by processPacket when in state FINDING_NODES
    *    and packet type is IDTRANSLATIONREPLY. In UserDefinedData
    *    is info about this packet containing destinations.
    */
   void processDestTranslation( IDTranslationReplyPacket* packet );

   enum state_t { 
      /** 
       *    The state that is entered at the start.
       *    In this state the maps to route to are requested.
       */
      FINDING_MAPS,

      /**
       *    When in this state, the edge nodes are requested
       *    as well as the overview mapID.
       */
      FINDING_NODES,       

      /**
       *    Map the nodes up one level.
       */
      TRANSLATING_NODES,       
      
      /**
       *    We have finished the finding.
       */
      DONE
      
   };

   /**
    *    Current state of this object according to state_t above
    */
   state_t m_state;

   /**
    *    The map level at which routing is currently done
    */
   uint32 m_level; 

   /**
    *    STL Set containing the map IDs that the origins are on
    */
   set<uint32> m_origMapSet;

   /**
    *    STL Set containing map IDs that are allowed to route to
    *    when going down to lower level after a higher level routing.
    */
   set<uint32> m_destMapSet;

   /**
    *    STL Map containing a serial number to match higher level
    *    origins when mergin after higher level routing
    */
   map<uint32, OrigDestInfoList*> m_lowerOrigMap;
   
   /**
    *    STL Map containing a serial number to match higher level
    *    origins when mergin after higher level routing
    */    
   map<uint32, OrigDestInfoList*> m_lowerDestMap;

   /**
    *    Mapping higher level node ID and map ID to lower level
    *    OrigDestInfo, to continue routing on this map level
    *    after routing on higher level.
    */
   multimap< uint32, pair<uint32, OrigDestInfo*> > m_matchHighLowIDMap;

   /**
    *    The current status of the object
    */
   StringTable::stringCode m_status;
   
   /**
    *    Container for keeping packets on the way to other modules
    */
   PacketContainerTree m_outgoingQueue;

   /**
    *    Temporary container that stores ID RequestPackets until all
    *    EdgeNodeRequestPackets are processed
    */
   PacketContainerTree m_tempQueue;

   /**
    *    The mother ship of the sender. To get packetid:s from.
    */
   Request* m_request;

   /**
    *    Counter keeping track of how many packets are in other modules.
    *    This is to determine when we are finished
    */
   uint32 m_nbrOutstanding;

   /**
    *    The origins on which to start routing on this level.
    */
   OrigDestInfoList* m_lowerLevelOrigList;

   /**
    *    The origins to be sent to higher level routing
    */
   OrigDestInfoList m_higherLevelOrigList;

   /**
    *    The destinations to reach on this level
    */
   OrigDestInfoList m_lowerLevelDestList;

   /**
    *    The destinations to be sent to higher level routing
    */
   OrigDestInfoList m_higherLevelDestList;

   /**
    *    Driver preferences to be used when sending edgenodes request.
    */
   const DriverPref* m_driverPrefs;

   /**
    *    The maps bordering to the destination map.
    */
   set<uint32> m_destBorderMaps;

   /**
    *    The destination nodes.
    */
   set<IDPair_t> m_lowLevelDestNodes;

   /**
    *    The original destinations.
    */
   OrigDestInfoList m_originalDests;

   /**
    *    Counter to keep track of destination requests.
    */
   uint32 m_destPacketCounter;

   /**
    *    The minimum distance from a destination to the neigbour
    *    map. If a distance is less than this, the neighbour map
    *    will also be considered.
    */
   int m_minDistFromDestToEdge;
   
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline OrigDestInfoList*
LevelTransitObject::getHigherLevelOrigList(){
   return &m_higherLevelOrigList;
}

inline OrigDestInfoList*
LevelTransitObject::getHigherLevelDestList(){
   return &m_higherLevelDestList;
}

inline StringTable::stringCode
LevelTransitObject::getStatus() const {
   return m_status;
}

inline bool
LevelTransitObject::requestDone()
{
   return ( m_state == DONE );
}

#endif

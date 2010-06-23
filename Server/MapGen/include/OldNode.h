/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDNODE_H
#define OLDNODE_H

#include "config.h"
#include "ItemTypes.h"
#include "GMSLane.h"
#include "GMSSignPostElm.h"

/**
 *    The maximum number of connections in one node. Depends on the 
 *    number of bits for m_nbrEntryConnections (2^6).
 */
#define MAX_NBR_ENTRYCONNECTIONS 64

class OldConnection;
class DataBuffer;
class OldGenericMap;

// Stupid that the OldNode can use streams.
#include <fstream>

/**
  *   Describes one node on one street segment. Including a list with all 
  *   connections that leads from this node. A node is one of the endpoints
  *   of a OldStreetSegmentItem (is the street segment not i a stright line
  *   from node 0 to node 1 it might have graphical points in between, but
  *   these are <b>not</b> nodes and described by the GfxData for the 
  *   OldStreetSegmentItem). The connections that are used when traversing
  *   the map is always between two OldNodes.
  *
  *   A node have membervariables that is valid for the streetsegment when
  *   entering the road at that node (e.g. entry restrictions or level).
  *
  */
class OldNode {
   public:
      /**
        *   Create an empty OldNode.
        */
      inline OldNode() { MINFO("   OldNode()"); }; 

      /**
        *   Create one node with specified ID.
        *
        *   @param   The id of this node (first bit indicates if
        *            this is node 0 or 1).
        */
      OldNode(uint32 nodeID);

      /**
        *   Create one node with data read from the params.
        *
        *   @param   Pointer to start of datafield.
        *   @param   The id of this node (first bit indicates if
        *            this is node 0 or 1).
        */
      OldNode(DataBuffer *dataBuffer, uint32 nodeID, bool fullCreate = true);

      /**
        *   Destroy this OldNode.
        */
      inline virtual ~OldNode() { 
         // Do not delete the OldConnections (allocated in an Allocator),
         // but the array where they are stored!
         // Do not delete the connection array. It is also in an allocator.
         //delete [] m_entryConnections;
         MINFO("   ~OldNode()"); 
      };

      /**
        *   Write all data of the node.
        *   @param   dataBuffer Where the data is written.
        */
      bool save(DataBuffer *dataBuffer) const;

      /**
        *   Get the number of OldConnections that leads to this node.
        *   @return  The nuber of entryConnections. 
        */
      inline uint16 getNbrConnections() const;

      /**
        *   Get the ID of this node. The ID is the same as the ID for
        *   the Street Segment where this node is located except the
        *   MSB (that is 1 if this is node 1, 0 otherwise).
        *   @return  The id of this node
        */
      inline uint32 getNodeID() const;

      /**
        *   Set the ID of this node to a new value.
        *   NB! Wrong usage of this method leads to a corrupt database!
        *
        *   @return  The id of this node
        */
      inline void setNodeID(uint32 newNodeID);

      /**
       *    Find out if this is node 0 or 1.
       *    @return  true if this is node0, false otherwise.
       */
      inline bool isNode0() const;

      /**
       *    Returns the ID of the node at the other end of a routable 
       *    item.
       */
      inline uint32 getOppositeNodeID() const;
      
      /**
       *    Get one of the connections that leads to this node.
       *    @param   j  The index of the OldConnection to return. Valid
       *                values are 0 <= j < getNbrConnections().
       *    @return  Entry connection number j.
       */
      inline OldConnection* getEntryConnection(uint32 j) const;
      
      /**
       *    Get the connection that leads to this node coming
       *    from the specified node id. If no such connection is
       *    found, NULL is returned.
       *    
       *    @param   fromNode    The node id that the connection is
       *                         coming from.
       *    @return  The connection coming from fromNode if such
       *             a connection is present. Otherwise NULL.
       */
      OldConnection* getEntryConnectionFrom( uint32 fromNode ) const;

      /**
       *    Get the entryrestrictions for this node.
       *    @return  The entryrestrictions for this node.
       */
      inline ItemTypes::entryrestriction_t getEntryRestrictions() const;

      /**
        *   Set the entry restrictions for this node to a new value.
        *   @param entry   The new entry restrictions for this node.
        */
      inline void setEntryRestrictions(ItemTypes::entryrestriction_t entry);

      /**
        *   Get the speed limit when passing this node. 
        *   @return The speed limit of the link leading from this node.
        *
        */
      inline uint8 getSpeedLimit() const;
      
      /**
        *   Set the speed limit to a specified value.
        *   @param   limit The new speedlimit.
        */
      inline void setSpeedLimit(byte limit);

      /**
        *   Get the type of junktion where this node is located. 
        *   @return  Junction type of this node.
        */
      inline ItemTypes::junction_t getJunctionType() const;

      /**
        *   Get the type of junktion where this node is located to a
        *   new value. 
        *   @param   type  The new type of junction for this node.
        */
      inline void setJunctionType( ItemTypes::junction_t type);

      /**
        *   Add one connection to this node.
        *   @param   newCon   The new connection that will be added.
        *                     <i><b>NB!</B>No copying is made, so the
        *                     new connection must not be deleted.</I>
        *   @return  True if the connection is added, false otherwise
        *            (the connection from the specified node does already
        *            exist).
        */
      bool addConnection(OldConnection* newCon,
                         OldGenericMap& theMap );

      /**
        *   Get the level of this node (relative the ground).
        *   @see     level
        *   @return  The level of this node relative the ground.
        */
      inline int8 getLevel() const;

      /**
        *   Set the level of this node (relative the ground).
        *   @see     level
        *   @param   level The level of this node relative the ground.
        */
      inline void setLevel( int8 level );

      /**
        *   Find out if this road is a major road or not.
        *   @return  True if road is a major road.
        */
      inline bool isMajorRoad() const;

      /**
        *   Set the major-road membervaribale to a new value.
        *   @param major   True if road is a major road, false otherwise.
        */
      inline void setMajorRoad( bool major);

      /**
       *    Get the maximum weight of the vehicles that might pass 
       *    this node.
       *    @return  The maximum weight on this road.
       */
      inline uint8 getMaximumWeight() const;

      /**
       *    Set the maximum weight of the vehicles that might pass 
       *    this node.
       *    @param maxWeight the maximum weight on this road.
       */
      inline void setMaximumWeight( uint8 maxWeight );

      /**
       *    Get the maximum height of the vehicles that might pass 
       *    this node.
       *    @return The maximum height on this road.
       */
      inline uint8 getMaximumHeight() const;

      /**
       *    Set the maximum height of the vehicles that might pass 
       *    this node.
       *    @param maxHeight the maximum height on this road.
       */
      inline void setMaximumHeight( uint8 maxHeight );

      /**   
       *    Get the number of lanes of this road.
       *    @return  The number of lanes.
       */
      uint32 getNbrLanes(const OldGenericMap& theMap) const;

      /**
       * @return Returns the lanes of this node.
       */
      vector<GMSLane> getLanes(const OldGenericMap& theMap) const;

      /**
       * Method used by GDF extraction. Can only be called once for a lane.
       * Reorders lanes so they are indexed from left to right etc.
       */
      void completeLaneInfoFromGdf( OldGenericMap& theMap);

      /**
       * Set the lane with index laneIndex of this node.
       */
      void setLane(const GMSLane& lane, uint32 laneIndex, 
                   OldGenericMap& theMap);

      /**
       * Add another connecting lane to the ones stored for the connection from
       * fromNodeID to this node. This does not remove any existing connecting 
       * lanes, and if the fromLaneIndex is already set as a connecting lane, 
       * the data is not changed.
       */
      void addLaneConnectivity( OldGenericMap& theMap, uint32 fromNodeID, 
                                uint32 fromLaneIndex );

      /**
       *  @return Returns a bit field with the lanes of the from node that
       *          connects to this node. Lowest bit is representing the left
       *          most lane. If no data about this connection
       *          exists, MAX_UINT32 is returned.
       */ 
      uint32 getConnectedLanes( const OldGenericMap& theMap, 
                                uint32 fromNodeID ) const;
   

      /** Adds a sign post part valid for the connection from fromNodeID.
       *  If no connection between this node and fromNodeID, no sign post
       *  data is added. If this sign post set is already set, it is not
       *  set again.
       * 
       *  @param theMap The map to add the sign post data to
       *  @param fromNodeID The node from which the connection of this sign
       *                    post data comes.
       *  @param signPostIndex The index of this sign post. There may be many
       *                       sign posts stored for the same connection.
       *  @param singPostSetIdx The sign post set to add the sign post data to.
       *  @param signPostPartIdx The sign post part within the set to add the
       *                         signpost data to.
       *  @param singPostElm The sign post data to add.

       *  @return Returns false of no sign post data was added.
       */
      bool addSignPostPart( OldGenericMap& theMap, uint32 fromNodeID,
                            uint32 signPostIndex, 
                            uint32 singPostSetIdx, 
                            uint32 signPostPartIdx,
                            GMSSignPostElm& singPostElm );

      /**
       *  @return Returns number of sign posts of the connection from 
       *          fromNodeID.
       */
      uint32 getNbrSignPosts(const OldGenericMap& theMap, 
                             uint32 fromNodeID) const;

      /**
       * @return Returns true if a sign post containing only signPostElm 
       *         exists for the connection from fromNodeID.
       */
      bool signPostExists( const OldGenericMap& theMap, uint32 fromNodeID, 
                           const GMSSignPostElm& signPostElm ) const;

      /**
       * @return Returns true if a sing post element was removed from the
       *         connection from fromNodeID.
       */
      bool removeSignPost( OldGenericMap& theMap, uint32 fromNodeID,
                           MC2String signPostText );

      
      /**
       *    Find out if this road has a road toll when entering the
       *    street segment via this node.
       *    @return  True if there is a road toll.
       */
      inline bool hasRoadToll() const;

      /**
       *    Set if this road has a road toll when entering the
       *    street segment via this node.
       *    @param roadToll is true if there is a road toll.
       */
      inline void setRoadToll( bool roadToll );

      /**
        *   Delete the connections that comes from a given node.
        *   @param   nodeID   The ID of the node.
        *   @return  True if a connection from nodeID was found and that
        *            was deleted, false otherwise.
        */
      bool deleteConnectionsFrom(uint32 nodeID, OldGenericMap& theMap);
   
      /**
        *   Delete all connections.
        */
      void deleteAllConnections(OldGenericMap& theMap);

      /**
        *   Deletes an entry at connection index.
        *   @param   index The index of the connection that will be
        *                  removed.
        */
      void deleteConnection( uint32 index, OldGenericMap& theMap );

      /**
        *   Writes node specific header to mif-file.
        *   @param   mifFile
        */
      static void printNodeMifHeader(ofstream& mifFile, uint32 n);
      
      /**
        *    Returns the ID of the node at the other end of a routable 
        *    item.
        *
        *    @param nodeID The ID of the node to get the ID of the node at
        *                  the other side of a routable item of.
        */
      static inline uint32 oppositeNodeID( uint32 nodeID );
      
      /**
        *   Writes all data in the nodes to a midFile.
        *   @param   midFile
        */
      void printNodeMidData(ofstream& midFile) const;

      /**
        *   Print the data (in textformat) about this node to a 
        *   preallocated buffer.
        *   @param   buf      The buffer where the information should
        *                     be written.
        *   @param   maxLen   The maximum length (in bytes) of the data 
        *                     that is written to buf.
        *   @return  A pointer to buf. NULL is returned upon error.
        */
      char* toString(char* buf, size_t maxLen);

      /**
       *    Get the size of this object in bytes in the memory.
       *    @return The size of the object in bytes.
       */
      uint32 getMemoryUsage(void) const;

      /**
       *    Method that updates all attributes of a node to the values 
       *    of another node. This includes e.g. entry restrictions, speed
       *    limits and major road attributes. If the nodes are originating
       *    from the same map (same node) also the attributes of the 
       *    connections to the node are updated to values from otherNode.
       *    For more documentation see the updateAttributesFromItem
       *    method in OldItem.h.
       *
       *    @param otherNode  The node from which to get update values.
       *    @param nodeNbr    The node nbr (0 or 1) of this node.
       *    @param sameMap    Default true meaning that the nodes are
       *                      one and the same (same map), set to false if
       *                      the they originates from different maps (e.g.
       *                      underview and overview map)
       *    @return  True if some attribute was updated for the connection,
       *             false if not.
       */
      bool updateNodeAttributesFromNode(OldNode* otherNode, uint32 nodeNbr,
                                        bool sameMap = true);
      

   protected:
      /**
        *   Create this OldConnection from a data buffer. The protocoll
        *   used in the data buffer is defined in a document called
        *   {\tt grammars.tex}
        *   @param   databuffer  The data buffer from where this object
        *                        will be created.
        */
      void createFromDataBuffer(DataBuffer* databuffer, 
                                OldGenericMap* theMap);

      /**
        *   The id of this node. The first bit indicates if this is
        *   node 0 or 1 in the OldStreetSegmentItem with id 
        *   <TT>nodeID & 0x7FFFFFFF</TT>.
        */
      uint32 m_nodeID;

      /**
        *   Array with all entry-connections to this node. The member
        *   m_nbrEntryConnections contains the size of this array.
        */
      OldConnection** m_entryConnections;

      /**
        *   The maximum speed in this direction in km/h.
        */
      uint8 m_speedLimit : 8;

      /**
       *    The number of OldConnections in m_entryConnections.
       */
      uint8 m_nbrEntryConnections : 6;

      /**
        *   Level of node relative groundlevel.
        *   <ol>
        *      <li><b>...;</b> below ground</li>
        *      <li><b>-1;</b> below ground</li>
        *      <li><b>0;</b> on ground</li>
        *      <li><b>1;</b> above ground</li>
        *      <li><b>...;</b> above ground</li>
        *   </ol>
        */
      int8 m_level : 5;

      /**
        *   The maximum height of the vehicles passing this node.
        */
      uint8 m_maximumHeight : 2;

      /**
        *   The maximum weight of the vehicles passing this node
        *   in tons.
        */
      uint8 m_maximumWeight : 3;

      /**
        *   Restrictions on this node. Stored in one byte on disk,
        *   but used as enumeration when loaded.
        */
      byte m_entryRestrictions : 2;

      /**
        *   Type of junction
        */
      byte m_junctionType : 2;

      /**
        *   Major road. What in Sweden is called "huvudled".
        */
      bool m_majorRoad : 1;

      /**
        *   Are there roadToll at this node?
        */
      bool m_roadToll : 1;

      friend class OldRouteableItem;

};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint16 
OldNode::getNbrConnections() const
{
   return m_nbrEntryConnections;
}

inline uint32 
OldNode::getNodeID() const
{
   return (m_nodeID);
}

inline void  
OldNode::setNodeID(uint32 newNodeID) 
{
   m_nodeID = newNodeID;
}

inline bool  
OldNode::isNode0() const
{
   return ( (m_nodeID & 0x80000000) != 0x80000000);
}

inline uint32 
OldNode::oppositeNodeID( uint32 nodeID ){
   // At this moment, the line below is located directly in the code at many
   // places.
   return nodeID ^ 0x80000000;
}

inline uint32
OldNode::getOppositeNodeID() const
{
   return oppositeNodeID(m_nodeID);
}

inline OldConnection*  
OldNode::getEntryConnection(uint32 j) const
{
   if (j < getNbrConnections()) {
      return m_entryConnections[j];
   } else {
      return (NULL);
   }
}

inline ItemTypes::entryrestriction_t  
OldNode::getEntryRestrictions() const
{
   return ( (ItemTypes::entryrestriction_t) m_entryRestrictions);
}

inline void  
OldNode::setEntryRestrictions( ItemTypes::entryrestriction_t entry) 
{
   m_entryRestrictions = entry;
}

inline uint8  
OldNode::getSpeedLimit() const
{
   return m_speedLimit;
}

inline void  
OldNode::setSpeedLimit(byte limit) 
{
   m_speedLimit = limit;
}

inline ItemTypes::junction_t  
OldNode::getJunctionType() const
{
   return ( (ItemTypes::junction_t) m_junctionType);
}

inline void  
OldNode::setJunctionType( ItemTypes::junction_t type) 
{
   m_junctionType = type;
}

inline int8
OldNode::getLevel() const
{
   return m_level;
}

inline void  
OldNode::setLevel( int8 level )
{
   m_level = level;
}

inline bool  
OldNode::isMajorRoad() const
{
   return m_majorRoad;
}

inline void  
OldNode::setMajorRoad( bool major)
{
   m_majorRoad = major;
}

inline uint8  
OldNode::getMaximumWeight() const
{
   return m_maximumWeight;
}

inline void  
OldNode::setMaximumWeight( uint8 maxWeight )
{
   m_maximumWeight = maxWeight;
}

inline uint8  
OldNode::getMaximumHeight() const
{
   return m_maximumHeight;
}

inline void  
OldNode::setMaximumHeight( uint8 maxHeight )
{
   m_maximumHeight = maxHeight;
}


inline bool  
OldNode::hasRoadToll() const
{
   return m_roadToll;
}

inline void  
OldNode::setRoadToll( bool roadToll )
{
   m_roadToll = roadToll;
}

#endif


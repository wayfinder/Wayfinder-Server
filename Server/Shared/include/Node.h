/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NODE_H
#define NODE_H

#include "config.h"
#include "ItemTypes.h"
#include "Connection.h"

/**
 *    The maximum number of connections in one node. Depends on the 
 *    number of bits for m_nbrEntryConnections (2^6).
 */
#define MAX_NBR_ENTRYCONNECTIONS 64

class Connection;
class DataBuffer;
class GenericMap;

/**
  *   Describes one node on one street segment. Including a list with all 
  *   connections that leads from this node. A node is one of the endpoints
  *   of a StreetSegmentItem (is the street segment not i a stright line
  *   from node 0 to node 1 it might have graphical points in between, but
  *   these are <b>not</b> nodes and described by the GfxData for the 
  *   StreetSegmentItem). The connections that are used when traversing
  *   the map is always between two Nodes.
  *
  *   A node have membervariables that is valid for the streetsegment when
  *   entering the road at that node (e.g. entry restrictions or level).
  *
  */
class Node {
public:
   /**
    *   Create an empty Node.
    */
   inline Node() { MINFO("   Node()"); }; 

   /**
    *   Create one node with specified ID.
    *
    *   @param   The id of this node (first bit indicates if
    *            this is node 0 or 1).
    */
   explicit Node( uint32 nodeID );

   /**
    *   Destroy this Node.
    */
   inline ~Node() { 
      // Do not delete the Connections (allocated in an Allocator),
      // but the array where they are stored!
      // Do not delete the connection array. It is also in an allocator.
      //delete [] m_entryConnections;
      MINFO("   ~Node()"); 
   };

   /**
    *   Write all data of the node.
    *   @param   dataBuffer Where the data is written.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *   Get the number of Connections that leads to this node.
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
    *    Get one of the connections that leads to this node.
    *    @param   j  The index of the Connection to return. Valid
    *                values are 0 <= j < getNbrConnections().
    *    @return  Entry connection number j.
    */
   inline Connection* getEntryConnection(uint32 j) const;
      
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
   Connection* getEntryConnectionFrom( uint32 fromNode ) const;

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
   inline uint8 getNbrLanes() const;

   /**
    *    Update the number of lanes of this road.
    *   @param  nbrLanes The number of lanes for this road.
    */
   inline void setNbrLanes( uint8 nbrLanes );

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
    *    Get the size of this object in bytes in the memory.
    *    @return The size of the object in bytes.
    */
   uint32 getMemoryUsage() const;

      
protected:
   friend class M3Creator;
   /**
    *   Create this Connection from a data buffer. The protocoll
    *   used in the data buffer is defined in a document called
    *   {\tt grammars.tex}
    *   @param   databuffer  The data buffer from where this object
    *                        will be created.
    */
   void load( DataBuffer& databuffer, GenericMap& theMap );

   /**
    *   The id of this node. The first bit indicates if this is
    *   node 0 or 1 in the StreetSegmentItem with id 
    *   <TT>nodeID & 0x7FFFFFFF</TT>.
    */
   uint32 m_nodeID;

   /**
    *   Array with all entry-connections to this node. The member
    *   m_nbrEntryConnections contains the size of this array.
    */
   Connection* m_entryConnections;

   /**
    *   The maximum speed in this direction in km/h.
    */
   uint8 m_speedLimit : 8;

   /**
    *    The number of Connections in m_entryConnections.
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
    *   The number of lanes in this direction of the street.
    */
   uint8 m_nbrLanes : 2;

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

   friend class RouteableItem;
};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint16 
Node::getNbrConnections() const
{
   return m_nbrEntryConnections;
}

inline uint32 
Node::getNodeID() const
{
   return (m_nodeID);
}

inline void  
Node::setNodeID(uint32 newNodeID) 
{
   m_nodeID = newNodeID;
}

inline bool  
Node::isNode0() const
{
   return ( (m_nodeID & 0x80000000) != 0x80000000);
}

inline Connection*  
Node::getEntryConnection(uint32 j) const
{
   if (j < getNbrConnections()) {
      return &m_entryConnections[j];
   } else {
      return (NULL);
   }
}

inline ItemTypes::entryrestriction_t  
Node::getEntryRestrictions() const
{
   return ( (ItemTypes::entryrestriction_t) m_entryRestrictions);
}

inline void  
Node::setEntryRestrictions( ItemTypes::entryrestriction_t entry) 
{
   m_entryRestrictions = entry;
}

inline uint8  
Node::getSpeedLimit() const
{
   return m_speedLimit;
}

inline void  
Node::setSpeedLimit(byte limit) 
{
   m_speedLimit = limit;
}

inline ItemTypes::junction_t  
Node::getJunctionType() const
{
   return ( (ItemTypes::junction_t) m_junctionType);
}

inline void  
Node::setJunctionType( ItemTypes::junction_t type) 
{
   m_junctionType = type;
}

inline int8
Node::getLevel() const
{
   return m_level;
}

inline void  
Node::setLevel( int8 level )
{
   m_level = level;
}

inline bool  
Node::isMajorRoad() const
{
   return m_majorRoad;
}

inline void  
Node::setMajorRoad( bool major)
{
   m_majorRoad = major;
}

inline uint8  
Node::getMaximumWeight() const
{
   return m_maximumWeight;
}

inline void  
Node::setMaximumWeight( uint8 maxWeight )
{
   m_maximumWeight = maxWeight;
}

inline uint8  
Node::getMaximumHeight() const
{
   return m_maximumHeight;
}

inline void  
Node::setMaximumHeight( uint8 maxHeight )
{
   m_maximumHeight = maxHeight;
}

inline uint8  
Node::getNbrLanes() const
{
   return m_nbrLanes;
}

inline void  
Node::setNbrLanes( uint8 nbrLanes )
{
   m_nbrLanes = nbrLanes;
}

inline bool  
Node::hasRoadToll() const
{
   return m_roadToll;
}

inline void  
Node::setRoadToll( bool roadToll )
{
   m_roadToll = roadToll;
}

#endif


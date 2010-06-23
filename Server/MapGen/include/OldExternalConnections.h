/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDEXTERNALCONNECTIONS_H
#define OLDEXTERNALCONNECTIONS_H

#include "config.h"
#include "OldGenericMap.h"
#include "ObjVector.h"
#include "OldConnection.h"
#include <vector>

#include "DataBuffer.h"

/**
  *   Describes a segment on the boundry. A segment on the boundry is
  *   a segment (routeable item, i.e. street segent or ferry item) 
  *   that might have connections from other maps.
  *
  */
class OldBoundrySegment : public VectorElement {
   public:

      /**
        *   The different cases of how nodes are close to the boundry.
        */
      enum closeNode_t {
         /// Node 0 is close to the boundry.
         node0close = 0,
         /// Node 1 is close to the boundry.
         node1close = 1,
         /// Both node 0 and node 1 is close to the boundry.
         //bothNodesClose,
         // No node is close (default value).
         noNodeClose
      };
      
      /**
        *   Default constructor.
        */
      OldBoundrySegment();

      /**
        *   Create a boundrySegment with the smallest possible
        *   information.
        *   @param   routeableItemID  ID of the corresponding
        *                             routeable item {\bf on this map}.
        *   @param   closeNodeValue   Which node(s) are close to the boundry,
        *                             0=node0, 1=node1, 2=bothNodes.
        */
      OldBoundrySegment(uint32 routeableItemID,
                     closeNode_t closeNodeValue);

      /**
        *   Create a OldBoundrySegment with data from a databuffer.
        *   @param   dataBuffer  The buffer that contains the data for
        *                        this boundry segment.
        */
      OldBoundrySegment(DataBuffer* dataBuffer, OldGenericMap* theMap);

      /**
        *   Destructor that deletes this OldBoundrySegment.
        */
      virtual ~OldBoundrySegment();

      /**
        *   @name Operators
        *   These operators are needed for sorting and searching among 
        *   the OldBoundrySegments.
        */
      //@{
         /// equal
         bool operator == (const VectorElement& elm) const {
            return ( m_routeableItemID == (dynamic_cast
                     <const OldBoundrySegment*> (&elm))->m_routeableItemID);
         };
         /// not equal
         bool operator != (const VectorElement& elm) const {
            return ( m_routeableItemID != (dynamic_cast
                     <const OldBoundrySegment*> (&elm))->m_routeableItemID);
         };

         /// greater
         bool operator > (const VectorElement& elm) const {
            return ( m_routeableItemID > (dynamic_cast
                     <const OldBoundrySegment*> (&elm))->m_routeableItemID);
         };

         /// less
         bool operator < (const VectorElement& elm) const {
            return ( m_routeableItemID < (dynamic_cast
                    <const OldBoundrySegment*> (&elm))->m_routeableItemID);
         };
      //@}

      /**
        *   Use this method to get the number of external connections
        *   to one of the nodes.
        *
        *   @param   i  The node (0 or 1).
        *   @return  The number of external connections to node i. If
        *            i == 0 then the number of connections to node 0
        *            is returned, otherwise the number of connections
        *            to node 1.
        */
      inline uint32 getNbrConnectionsToNode(byte i) const;
      
      /**
        *   Get one of the connections that leads to node n.
        *   @param   n  The node (o or 1).
        *   @param   i  The wanted connection.
        *   @return  The i:th connection that leads to node n of
        *            this segment. NULL is returned upon error.
        */
      inline OldConnection* getConnectToNode(byte n, uint32 i) const;
      inline uint32 getFromMapIDToNode(byte n, uint32 i) const;

      /**
        *   Get one of the connections that leads to node 0.
        *
        *   @param   i  What connection wanted. Valid values are
        *               0 < getNbrConnectionsToNode(0).
        *   @return  The i:th connection that leads to node0 of this 
        *            routeable item. NULL is returned upon error.
        */
      inline OldConnection* getConnectToNode0(uint32 i) const;

      inline uint32 getFromMapIDToNode0(uint32 i) const;

      /**
        *   Get one of the connections that leads to node 1.
        *
        *   @param   i  What connection wanted. Valid values are
        *               0 < getNbrConnectionsToNode(1).
        *   @return  The i:th connection that leads to node1 of this 
        *            routeable item. NULL is returned upon error.
        */
      inline OldConnection* getConnectToNode1(uint32 i) const;

      inline uint32 getFromMapIDToNode1(uint32 i) const;

      /**
        *   Set the value of the attribute "closeNode"
        *   to a new value.
        *   @param   value The new value of the attribute.
        */
      inline void setCloseNodeValue(closeNode_t value);

      /**
        *   Find out which node(s) are closest to the boundry.
        *   @return  The value of the "closeNode" attribute, 0=node0,
        *            1=node1, 2=both nodes.
        */
      inline closeNode_t getCloseNodeValue() const;

      /**
        *   Add one connection to this boundry segment.
        *
        *   @param con        The connection to be added.
        *   @param fromMapID  The ID of the map where this connection
        *                     come from.
        *   @param closestToBoundry  Should the connection be added from
        *            the node closest to the boundry or not? (the node of 
        *            the ssi on the other map).
        *   @return  True if the connection added alright, false
        *            otherwise.
        */
      bool addConnection(OldConnection* con, 
                         uint32 fromMapID,
                         bool closestToBoundry);

      /**
        *   Add one connection.
        *
        *   @param   con   The connection to be added.
        *   @param fromMapID  The ID of the map where this connection
        *                     come from.
        *   @param   toID  The id of the connectToNode (this is included
        *                  to make it possible to add the connection
        *                  to the correct node).
        *   @return  True if the connection added alright, false
        *            otherwise.
        */
      bool addConnection(OldConnection* con, uint32 fromMapID, uint32 toID);

      /**
        *   Get the ID of the routeable item (on the local map) that this
        *   object represent.
        *   @return The id of the routeable item the OldConnection leads to.
        */
      inline uint32 getConnectRouteableItemID() const;

      /**
       *    Save this OldBoundrySegment into a data buffer.
       *    @param   dataBuffer  The data buffer where this OldBoundrySegment
       *                         will be saved.
       */
      void save(DataBuffer* dataBuffer);

      /**
       *    Delete all connections that leads to this boundry segment.
       */
      void deleteAllConnections();

      /**
       *    Find out the memory usage for this object.
       *    @return The memory usage in bytes for this object.
       */
      virtual uint32 getMemoryUsage() const;

      uint32 getExternalConnectionIdx(byte node, 
                                      uint32 fromMapID, 
                                      uint32 fromItemID) const;

   /**
    * Map ID and connection.
    */
   typedef pair<uint32, OldConnection*> externalConnection_t;
   typedef vector< externalConnection_t > ExternalConnectionVector;
   const ExternalConnectionVector& getConnectionsToNode0() const { return m_connectionsToNode0; }
   const ExternalConnectionVector& getConnectionsToNode1() const { return m_connectionsToNode1; }
   
   uint32 getRouteableItemID() const { return m_routeableItemID; }
   closeNode_t getCloseNode() const { return m_closeNode; }

   private:


      typedef vector<externalConnection_t>::const_iterator 
         externalConnectionIt;
      
      /**
        *   The connection that leads to node0.
        */
      vector<externalConnection_t> m_connectionsToNode0;

      /**
        *   The connection that leads to node1.
        */
      vector<externalConnection_t> m_connectionsToNode1;

      /**
        *   The connections lead to the nodes of this routeable 
        *   item, i.e. street segment or ferry item (on this map).
        */
      uint32 m_routeableItemID;

      /**
        *   Which node(s) of this segment is/are close to the boundry. 
        *   0 means node 0 is close, 1 means node 1 is close and 2 
        *   means both nodes are close.
        */
      closeNode_t m_closeNode;
};


/**
  *   A vector with the segments on the boundry. A segment on the boundry
  *   is a segment that might have connections from other maps.
  *
  */
class OldBoundrySegmentsVector : public ObjVector {
   public:
      /**
        *   Creates an empty array with boundry segments.
        */
      OldBoundrySegmentsVector();

      /**
        *   Creates an array of boundry segments with data from
        *   the dataBuffer. Uses the same format as saved in the 
        *   save-method.
        */
      OldBoundrySegmentsVector(DataBuffer* dataBuffer, OldGenericMap* theMap);

      /**
        *   Deletes this array with boundry segments.
        */
      virtual ~OldBoundrySegmentsVector();

      /**
        *   Saves the boundry segments in this array to the
        *   dataBuffer.
        *   @param   dataBuffer  The databuffer where the boundrysegments
        *                        array is saved.
        */
      void save(DataBuffer* dataBuffer);

      /**
       *    Get a boundry segment with a specified item ID.
       *    @param   itemID   The ID of the boundry segment to return.
       *    @return  Pointer to boundry segment with routeableItemID 
       *             == itemID. NULL is returned if no such boundry
       *             segment.
       */
      inline OldBoundrySegment* getBoundrySegment(uint32 itemID) const;

      /**
        *   Adds a (empty) ExternalConnection with routeableItemID. If the 
        *   item already is present in the vector, it will not be added.
        *   
        *   @param routeableItemID  The ID the the routeable item that
        *                           is located close to the map 
        *                           boundry.
        *   @param closeNodeValue   Which node(s) are close to the boundry.
        *   @return  True if a new OldBoundrySegment is added, false 
        *            otherwise.
        */
      bool addBoundrySegment(uint32 routeableItemID, 
                             OldBoundrySegment::closeNode_t closeNodeValue);
      
      /**
       *    Add a connection that leads from a routeable item on 
       *    another map, to a routeable item on this map.
       *    @warning  Since one OldBoundrySegment only can have connections 
       *              from one map, we have an error if connections from 
       *              different maps leads to the same boundry segment.
       *
       *    @param fromMapID  The ID of the map where fromNodeID is located.
       *    @param fromNodeID The ID of the node on the other map, from
       *                      where it is possible to drive to #toNodeID#.
       *    @param toNodeID   The ID of a node on this map to where it
       *                      is possible to drive from #fromNodeID#.
       */
      void addConnection(uint32 fromMapID, uint32 fromNodeID, uint32 toNodeID);

      /**
        *   Get the connection that leads from a certain item on
        *   a certain map.
        *
        *   @param  itemID ID of the item that the connection leads from.
        *   @param  mapID  ID of the map that the connection leads from.
        *   @return A connection that leads from the item and map
        *           mapID and ItemID. NULL is returned if no such
        *           connection is found.
        */
      OldConnection* getBoundryConnection(uint32 itemID, uint32 mapID) const;

      /**
        *   Get the boundry segments that have connections from 
        *   a certain node on a certain map. One bs is only added once.
        *
        *   @param   mapID    ID of the map the connection leads from.
        *   @param   nodeID   ID of the node the connection leads from.
        *   @param   result   The vector to store the boundry segments in.
        */
      void getBoundrySegments(uint32 mapID, uint32 nodeID,
                              ObjVector* result) const;

      /**
        *   Prints all external connections to standard out.
        *   @param   toMapID  ID of the map where the connections leads
        *                     from. If not specified, connections from
        *                     all maps are printed.
        */
      void dump(uint32 toMapID = MAX_UINT32);

      /**
       *    Get the total number of connections that leads to this map.
       *    @return  The total number of connections in the 
       *             OldBoundrySegments in this array.
       */
      uint32 getTotNbrConnections();

      /**
        *   Delete all the connections in all the boundry segments.
        */
      void deleteAllConnections();
};


// =======================================================================
//                                     Implementation of inlined methods =


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                        OldBoundrySegment -

inline uint32  
OldBoundrySegment::getNbrConnectionsToNode(byte i) const 
{
   if (i == 0)
      return (m_connectionsToNode0.size());
   else 
      return (m_connectionsToNode1.size());
}

inline OldConnection* 
OldBoundrySegment::getConnectToNode(byte n, uint32 i) const
{
   MC2_ASSERT( (n == 0) || (n == 1));
   if (n == 0) {
      return (m_connectionsToNode0[i].second);
   } else {
      return (m_connectionsToNode1[i].second);
   }
}

inline uint32 
OldBoundrySegment::getFromMapIDToNode(byte n, uint32 i) const
{
   MC2_ASSERT( (n == 0) || (n == 1));
   if (n == 0) {
      return (m_connectionsToNode0[i].first);
   } else {
      return (m_connectionsToNode1[i].first);
   }
}



inline OldConnection*  
OldBoundrySegment::getConnectToNode0(uint32 i) const 
{
   return (m_connectionsToNode0[i].second);
}

inline uint32 
OldBoundrySegment::getFromMapIDToNode0(uint32 i) const
{
   return (m_connectionsToNode0[i].first);
}

inline OldConnection*  
OldBoundrySegment::getConnectToNode1(uint32 i) const 
{
   return (m_connectionsToNode1[i].second);
}

inline uint32 
OldBoundrySegment::getFromMapIDToNode1(uint32 i) const
{
   return (m_connectionsToNode1[i].first);
}

inline void
OldBoundrySegment::setCloseNodeValue(closeNode_t value)
{
   m_closeNode = value;
}

inline OldBoundrySegment::closeNode_t
OldBoundrySegment::getCloseNodeValue() const
{
   return (m_closeNode);
}

inline uint32 
OldBoundrySegment::getConnectRouteableItemID() const 
{
   return (m_routeableItemID);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                 OldBoundrySegmentsVector -

inline OldBoundrySegment*
OldBoundrySegmentsVector::getBoundrySegment(uint32 itemID) const
{
   OldBoundrySegment searchEC(itemID & ITEMID_MASK, OldBoundrySegment::noNodeClose);
   return ( static_cast<OldBoundrySegment*>(binaryGet(&searchEC)));
}



#endif


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDROUTEABLEITEM_H
#define OLDROUTEABLEITEM_H

#include "config.h"
#include "OldItem.h"

class OldGenericMap;
class OldNode;

/**
  *   Superclass of all routeable items. Consists of
  *   two nodes, one in each end of the segment.
  *
  */
class OldRouteableItem : public OldItem {
   public:
      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldRouteableItem() { MINFO("OldRouteableItem");};
      
      /**
        *   Creates an item containing information about a 
        *   OldRouteableItem.
        *
        *   @param   type  The itemtype of this item.
        *   @param   id    The localID of this item.
        */
      OldRouteableItem(ItemTypes::itemType type, uint32 id);

      /**
        *   Destroys the OldRouteableItem.
        */
      virtual ~OldRouteableItem();

      /**
        *   Writes the item into the dataBuffer.
        *   @param   dataBuffer Where to print the information.
        *   @return  True if no errors occurred, false otherwise.
        */
      virtual bool save(DataBuffer* dataBuffer) const;
     
      /**
        *   Get one of the nodes.
        *   @param   i  The node to return. Valid values are 0 for
        *               noe 0 and 1 for node 1.
        *   @return  Pointer to the node asked for (0 or 1).
        */
      inline OldNode* getNode(int i);
      inline const OldNode* getNode(int i) const;

      /**
        *   Get one of the nodes when the node ID is supplied. If 
        *   the most significant bit is set, node 1 will be returned,
        *   otherwise node 0.
        *   @param   id The ID of the node to return. 
        *   @return  Pointer to the node indicated by the most
        *            significant bit in the id.
        */
      inline OldNode* getNodeFromID(uint32 id);

      /**
       *    Print (debug) information about the object into a
       *    static string in the class. Do not delete the string.
       *    @return The OldRouteableItem as a string
       */
      virtual char* toString();

      /**
        *   Get the distance from the last point of this segment to
        *   an other.
        *   @return  Squared distance from last point in this street
        *            segment to nearest endpoint in the other one. 
        *            Only work when Earth can be linearized.
        *            Returns -1 if gfxData was NULL.
        */
      double sqrDistanceFromLast(OldRouteableItem *routeableItem);

      /**
        *   Get the distance from the first point of this segment to
        *   an other.
        *   @return  Squared distance from first point in this street
        *            segment to last end-point in the other one. Does 
        *            not work where Earth cannot be approximated with 
        *            a plane. Returns -1 if gfxData was NULL.
        */
      double sqrDistanceFromFirst(OldRouteableItem *routeableItem);

      /**
        *   Get the distance between the first and the last point of 
        *   this steet segment.
        *   @return The squared distance between first and last point.
        */
      double sqrFlightDistance();

      /**
        *   Find out if this routeable item is connected to an other.
        *   @param other    The segment to look for connections with.
        *   @param nodeNbr  The node to check. If nodeNbr != 0 and nodeNbr 
        *                    != 1 it will check both nodes.
        *   @return True if this routeable item is connected to the other. 
        */
      bool isConnectedTo(OldRouteableItem* other, int nodeNbr = 2);
      
      /**
        *   Get the length of this routeable item. The length is 
        *   returned in meters.
        *   @return  The length (in meters) of this OldRouteableItem.
        *            MAX_UINT32 is returned if an error occurred.
        */
      //inline uint32 getLength() const;

      /**
        *   Delete connections that leads from a specified node.
        *   @param   nodeID   ID of the node.
        *   @return  True if any connections are deleted.
        */
      bool deleteConnectionsFrom(uint32 nodeID, OldGenericMap& theMap);

      /**
        *   Get the class of this road.
        *   To be overridden by sub classes supporting road class.
        *   If not overridden, mainRoad is returned.
        *   @return The road class (i.e. the level)
        */
      inline virtual byte getRoadClass() const;
      
      /**
        *   Get the number of bytes this object uses of the memory.
        *   @return How many bytes this object uses.
        */
      uint32 getMemoryUsage() const;

      /**
       * Deletes what is allocated when you don't have a map but not
       * deleted in destructor. 
       * NB! Don't call if you are unsure. Call only before destructor.
       */
      //void deleteMapData(OldGenericMap& theMap);

      /**
       *    Virtual method that updates all attributes of item and,
       *    if any, its nodes and connections to the values of the 
       *    other item. This includes e.g. names, groups, entry 
       *    restrictions, sign posts, speed limit and gfxdata.
       *    For more documentation see OldItem.h.
       *
       *    @param otherItem  The item from which to get update values.
       *    @param sameMap    Default true meaning that the items are
       *                      one and the same (same map), set to false if
       *                      the items originates from different maps (e.g.
       *                      underview and overview map)
       *    @return  True if some attribute was updated for the item,
       *             false if not.
       */
      virtual bool updateAttributesFromItem(OldItem* otherItem,
                                            bool sameMap = true);

   protected:
      /**
        *   Create this routeable item from a data buffer.
        *   @param   dataBuffer  The databuffer containing the data
        *                        for this street segment.
        */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);
        
      /**
        *   Create a new node. The reason for this method is to
        *   make it possible for the subclasses to create subclasses
        *   to OldNode.
        *
        *   @warning Since the normal way of getting a "new" node is
        *            to ask the map for a pre-allocated one the members
        *            will @b not be deleted. This means that using
        *            this method to create the members will cause
        *            memory leak!
        *
        *   @param   dataBuffer  The data buffer containing the data
        *                        for the new node.
        *   @param   nodeID      The ID of the new node.
        *   @return  The created node.
        */
      virtual OldNode* createNewNode(DataBuffer* dataBuffer, uint32 nodeID);

      /**
       *    Get a pre-allocated node from the map.
       *    @param theMap  Pointer to the map where the nodes are 
       *                   pre-allocated.
       *    @return  A "new" node, from the map.
       */
      virtual OldNode* getNewNode(OldGenericMap* theMap);

      /** 
        *   The first node of this routeable item.
        */
      OldNode* m_node0;

      /**
        *   The second node of this routeable item.
        */
      OldNode* m_node1;

      /**
       *    Declaire OldItem as a friend, to make it possible to
       *    call the createFromDataBuffer-methd.
       */
      friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
      friend class GMSItem;

};

// ========================================================================
//                                      Implementation of inlined methods =

inline OldNode* 
OldRouteableItem::getNode(int i) 
{
   if (i == 0) {
      return (m_node0);
   } else if (i == 1) {
      return (m_node1);
   } else {
      return (NULL);
   }
}

inline const OldNode* 
OldRouteableItem::getNode(int i) const
{
   if ( i == 0 )
      return m_node0;
   else if ( i == 1 )
      return m_node1;

   return NULL;
}

inline OldNode* 
OldRouteableItem::getNodeFromID(uint32 id) 
{
   if ( (id & 0x80000000) == 0x80000000) {
      return (m_node1);
   } else {
      return (m_node0);
   }
}

inline byte 
OldRouteableItem::getRoadClass() const 
{
   // Should be overridden by subclasses having roadclass.
   mc2log << warn << here << " OldRouteableItem::getRoadClass() called." 
          << endl;
   return (ItemTypes::mainRoad);
}

#endif


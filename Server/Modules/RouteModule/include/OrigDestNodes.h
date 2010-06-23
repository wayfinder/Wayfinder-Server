/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ORIG_DEST_NODES_H
#define ORIG_DEST_NODES_H

#include "config.h"
#include "CCSet.h"
#include "RoutingNode.h"

/**
 * Class that stores all incoming origins and destinations in
 * leader/availables.
 *
 */
class OrigDestNode : public RoutingNode, public Link
{
   // The constructors may not be used by anyone but RoutingMap.
private:
   friend class RoutingMap;

   void init() {
      m_cost          = MAX_UINT32;
      m_estimatedCost = MAX_UINT32;
      m_gradient      = NULL;
      m_dest          = false;
      m_isVisited     = false;
      m_lat           = MAX_INT32;
      m_lon           = MAX_INT32;
      m_forwardConn   = NULL;
      m_backwardConn  = NULL;
      m_restriction   = ItemTypes::noRestrictions;
      m_costASum      = 0;
      m_costBSum      = 0;
      m_costCSum      = 0;     
      m_turnCost      = 0;
      m_nodeIndex     = MAX_UINT32;
   }
   
   /**
    * Constructs a new OrigdestNode.
    *
    * @param index         The nodeID to be processed.
    * @param mapID         Is the mapID of the node. 
    * @param offset        The offset into the segment.
    * @param lat           The latitude of the node.
    * @param lon           The longitude of the node.
    * @param cost          The real cost from origin to this node.
    * @param estimatedCost The estimated cost from origin to
    *                      destination that passes this node.
    * @param turnCost      The cost to turn to this node. This
    *                      parameter is optional.
    */
   OrigDestNode(uint32 index,
                uint32 mapID,
                uint32 offset,
                int32 lat,
                int32 lon,
                int32 cost,
                int32 estimatedCost,
                uint32 costASum,
                uint32 costBSum,
                uint32 costCSum,
                uint32 turnCost = 0)
      : RoutingNode(true) {
      init();
      m_itemID        = MAX_UINT32;
      m_nodeIndex     = index;
      m_mapID         = mapID;
      m_offset        = offset;
      m_lat           = lat;
      m_lon           = lon;
      m_cost          = cost;
      m_estimatedCost = estimatedCost;
      m_turnCost      = turnCost;

      
      m_costASum      = costASum;
      m_costBSum      = costBSum;
      m_costCSum      = costCSum;
   }
      
   /**
    * Constructs a new OrigDestNode. Used by leader to check if origin
    * and destination lies within the same segment.
    *
    * @param uint32 itemID The itemID of this node.
    * @param uint32 mapID  The mapID that the node belongs to.
    * @param uint32 offset The offset in the segment of the node.
    */
   OrigDestNode( uint32 itemID, uint32 mapID, uint32 offset )
      : RoutingNode(true){
      init();
      m_itemID = itemID;
      m_mapID  = mapID;
      m_offset = offset;
      
      // and the rest are set to defaults
      m_cost          = 0;
      m_estimatedCost = 0;
   }
      
   /**
    * Default constructor.
    */
   OrigDestNode() : RoutingNode(true) {
      init();
      m_itemID        = MAX_UINT32;
      m_nodeIndex     = MAX_UINT32;
      m_offset        = MAX_UINT16;
      m_lat           = MAX_UINT32;
      m_lon           = MAX_UINT32;
      m_mapID         = MAX_UINT32;
      m_cost          = 0;
      m_estimatedCost = 0;
      m_turnCost      = 0;
      m_forwardConn   = NULL;
      m_backwardConn  = NULL;
      m_gradient      = NULL;
      m_restriction   = ItemTypes::noRestrictions;
      m_costASum      = 0;
      m_costBSum      = 0;
      m_costCSum      = 0;     
   }
   
public:   
   /**
    * Get the offset of this node (from node 0 in segment). Offset is in
    * per cent normalized to 2^16-1.
    *
    * @return  The offset from node 0.
    */
   inline uint16 getOffset();
   
   /**
    * Get the mapID of this node.
    *
    * @return The mapID.
    */
   inline uint32 getMapID() const;
   
   /**
    *    Setts the starting direction. Is set to true if driving towards 
    *    node 1, false if driving towards node 0.
    *
    *    @param startDir The starting direction, true if driving towards
    *                    node 1, false if driving towards node 0.
    */
   inline void setStartDirection( bool startDir );
   
   /**
    *    Get the starting direction. Will return true if driving 
    *    towards node 1 (positive direction), false if driving towards 
    *    node 0 (negative direction).
    *
    *    @return True if driving towards node 1, false if driving 
    *            towards node 0.
    */
   inline bool getStartDirection();
   
   /**
    * Get the cost to turn to this node.
    *
    * @return The turnCost of this node.
    */
   inline uint32 getTurnCost();
   
   /**
    * Set the cost to turn to this node.
    *
    * @param turnCost The cost to turn to this node.
    */
   inline void setTurnCost(uint32 turnCost);
   
   /**
    * Dumps the data to mc2log.
    */
   inline void dump();

   /**
    *   Returns the cost sum for cost A.
    */
   inline uint32 getCostASum() const;

   /**
    *   Returns the cost sum for cost B.
    */
   inline uint32 getCostBSum() const;

   /**
    *   Returns the cost sum for cost C.
    */
   inline uint32 getCostCSum() const;

   /**
    *   Returns the next node in the list.
    */
   inline OrigDestNode* next() const;

   /**
    *   Returns the previous node in the list.
    */
   inline OrigDestNode* prev() const;
   
private:
   
   /**
    * Offset to node zero from the origin/destination.
    */
   uint16 m_offset;
   
   /**
    * The mapID of this node.
    */
   uint32 m_mapID;
   
   /**
    * This is the cost to turn to this node. It will be added to the
    * connection costs.
    */
   uint32 m_turnCost;
   
   /** 
    * The start direction, only valid if this is an origin.
    */
   bool m_startDir;

   /**   
    *   The sum of costs A.
    */
   uint32 m_costASum;
   
   /**   
    *   The sum of costs B.
    */
   uint32 m_costBSum;
   
   /**   
    *   The sum of costs C.
    */
   uint32 m_costCSum;
   
}; // OrigDestNode


////////////////////////////////////////////////////////////////
// Inlined methods for OrigDestNode
////////////////////////////////////////////////////////////////

inline uint16 OrigDestNode::getOffset()
{
   return m_offset;
}

inline void OrigDestNode::setStartDirection( bool startDir )
{
   m_startDir = startDir;
}

inline bool OrigDestNode::getStartDirection()
{
   return m_startDir;
}

inline uint32 OrigDestNode::getTurnCost()
{
   return m_turnCost;
}

inline void
OrigDestNode::setTurnCost(uint32 turnCost)
{
   m_turnCost = turnCost;
}

inline uint32
OrigDestNode::getMapID() const
{
   return m_mapID;
}

inline void 
OrigDestNode::dump()
{
   mc2log << info  << " mapID " << m_mapID << " itemID " << hex
          << m_itemID << dec << " offset " << m_offset << endl
          << " index " << m_nodeIndex << " lat " << m_lat 
          << " lon " << m_lon << " cost " << m_cost 
          << " estim " << m_estimatedCost 
          << " turn " << m_turnCost 
          << " startDir " << m_startDir << endl;
}


inline uint32
OrigDestNode::getCostASum() const
{
   return m_costASum;
}

inline uint32
OrigDestNode::getCostBSum() const
{
   return m_costBSum;
}

inline uint32
OrigDestNode::getCostCSum() const
{
   return m_costCSum;
}

inline OrigDestNode*
OrigDestNode::next() const
{
   return static_cast<OrigDestNode*>(this->suc());
}

inline OrigDestNode*
OrigDestNode::prev() const
{
   return static_cast<OrigDestNode*>(this->pred());
}


#endif

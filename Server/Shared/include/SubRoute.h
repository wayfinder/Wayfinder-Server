/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBROUTE_H
#define SUBROUTE_H

#include <vector>
#include "Types.h"
#include "OrigDestInfo.h"

/**
 *    The information in this class is used in the RouteModule for routing,
 *    and in the server to keep track of the routes requested and finished.
 *    It contains all nodes between start and end within a certain map,
 *    plus other information about the route. Note that if a subroute ends
 *    in a node that has several external nodes, it is divided into several
 *    SubRoutes.
 *
 */
class SubRoute : public vector<uint32> {
public:

   /**
    *    Main constructor taking orig and dest.
    */
   SubRoute(const OrigDestInfo& OrigInfo,
            const OrigDestInfo& DestInfo);
  
   /**
    *    Adds a node ID to the subroute.
    *
    *    @param nodeID The ID of the node to add.
    */
   inline void addNodeID(uint32 nodeID);

   /**
    *    Gets the node at position 'index'.
    *    @param index Index of the node to return.
    *    @return The nodeID at index 'index'.
    */
   inline uint32 getNodeID(uint32 index) const;
   
   /**
    *    @return  The ID of the map that the SUbRoute is in.
    */
   inline uint32 getThisMapID() const;

   /**
    *    This function is used by RouteSender to put all nodes
    *    in one SubRoute on the same map. This saves some time
    *    when mapping to higher level, since not all neighbour
    *    maps have to be loaded. Also setDestNodeID below.   
    *    @param  The new ID of the map that this SubRoute leads to.
    */
   inline void setNextMapID( uint32 mapID );
   
   /**
    *    @return  The ID of the map that this SubRoute leads to
    */
   inline uint32 getNextMapID() const;

   /**
    *    @return  The ID of the node corresponding to the origInfo
    */
   inline uint32 getOrigNodeID() const;

   /**
    *    @return  The complete id of the destination node.
    */
   inline const IDPair_t& getOrigID() const;

   /**
    *    @param  The new ID of the dest that this SubRoute leads to.
    */   
   inline void setDestNodeID( uint32 nodeID );
   
   /**
    *    @return  The ID of the node corresponding to the destInfo
    */
   inline uint32 getDestNodeID() const;

   /**
    *    @return  The complete id of the destination node.
    */
   inline const IDPair_t& getDestID() const;

   /**
    *    @param  The new EstimatedCost of the dest.
    */   
   inline void setEstimatedCost( uint32 cost );
   
   /**
    *    @return  The estimated cost for the destInfo
    */
   inline uint32 getEstimatedCost() const;

   /**
    *    @return  Cost for the destInfo
    */
   inline uint32 getCost() const;

   /**
    *    Sets a new cost for the destInfo.
    */
   inline void setCost(uint32 newCost);

   /**
    *    @return  Lat for the destInfo
    */
   inline int32 getDestLat() const;

   /**
    *    @return  Lon for the destInfo
    */
   inline int32 getDestLon() const;

   /**
    *    Set the ID of this SubRoute.
    *    This is the same as prevSubRoute for the destInfo
    *
    *    @param  ID  The ID of the SubRoute
    */
   inline void setSubRouteID(uint32 ID);

   /**
    *    @return  The ID of this SubRoute
    *             (prevSubRoute for destInfo)
    */
   inline uint32 getSubRouteID() const;

   /**
    *    Set the ID of the previous SubRoute.
    *    This is the same as prevSubRoute for the origInfo
    *
    *    @param  ID  The ID of the previous SubRoute
    */
   inline void setPrevSubRouteID(uint32 ID);

   /**
    *    @return  The ID of the previous SubRoute
    *             (prevSubRoute for origInfo)
    */
   inline uint32 getPrevSubRouteID() const;

   /**
    *    Gets the offset for the DestInfo.
    *    @return  Offset for the destination.
    */
   inline float getDestOffset() const;

   /**
    *    Sets the offset for the DestInfo.
    *    @param  New offset for the destination.
    */
   inline void setDestOffset( float offset );
   
   /**
    *    Gets the offset for the OrigInfo.
    *    @return Offset for the origin.
    */
   inline float getOrigOffset() const;

   /**
    *    Sets the offset for the OrigInfo.
    *    @param New offset for the origin.
    */
   inline void setOrigOffset( float offset );

   /**
    *    Gets the starting vehicle for this SubRoute.
    *    @return The starting vehicle for this SubRoute.
    */
   inline const Vehicle* getOrigVehicle() const;
   
   /**
    *    @return  True if origInfo and destInfo in same map
    */
   inline bool hasOneMap() const;

   /**
    *    Gets the destination info
    *    @return  A pointer to the destination info.
    */
   inline const OrigDestInfo* getDestInfo() const;

   /**
    *    Setss the destination info
    *    @return  A pointer to the destination info.
    */
   inline void setDestInfo( const OrigDestInfo& info );   

   /**
    *    Returns the costA sum for the route.
    */
   inline uint32 getCostASum() const;

   /**
    *    Returns the costB sum for the route.
    */
   inline uint32 getCostBSum() const;

   /**
    *    Returns the sum of costs C up to this point.
    */
   inline uint32 getCostCSum() const;

   /**
    *
    */
   inline void setCostASum(uint32 costSum);
   
   /**
    *
    */
   inline void setCostBSum(uint32 costSum);
   
   /**
    *
    */
   inline void setCostCSum(uint32 costSum);
   
private:

   /**
    *    information about the start of the SubRoute
    */
   OrigDestInfo m_origInfo;

   /**
    *    information about the end of the SubRoute
    */
   OrigDestInfo m_destInfo;
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline void
SubRoute::addNodeID(uint32 nodeID)
{
   push_back(nodeID);
}

inline uint32
SubRoute::getNodeID(uint32 index) const
{
   return (*this)[index];
}

inline uint32
SubRoute::getThisMapID() const
{
   return m_origInfo.getMapID();
}

inline void
SubRoute::setNextMapID( uint32 mapID )
{
   m_destInfo.setMapID( mapID );
}

inline uint32
SubRoute::getNextMapID() const
{
   return m_destInfo.getMapID();
}

inline uint32
SubRoute::getOrigNodeID() const
{
   return m_origInfo.getNodeID();
}

inline const IDPair_t&
SubRoute::getOrigID() const
{
   return m_origInfo.getID();
}

inline const IDPair_t&
SubRoute::getDestID() const
{
   return m_destInfo.getID();
}

inline void
SubRoute::setDestNodeID( uint32 nodeID )
{
   m_destInfo.setNodeID( nodeID );
}

inline uint32
SubRoute::getDestNodeID() const
{
   return m_destInfo.getNodeID();
}

inline void
SubRoute::setEstimatedCost( uint32 cost )
{
   m_destInfo.setEstimatedCost( cost );
}

inline uint32
SubRoute::getEstimatedCost() const
{
   return m_destInfo.getEstimatedCost();
}

inline uint32
SubRoute::getCost() const
{
   return m_destInfo.getCost();
}

inline void
SubRoute::setCost(uint32 newCost)
{
   m_destInfo.setCost(newCost);
}

inline int32
SubRoute::getDestLat() const
{
   return m_destInfo.getLat();
}

inline int32
SubRoute::getDestLon() const
{
   return m_destInfo.getLon();
}

inline void
SubRoute::setSubRouteID(uint32 ID)
{
   m_destInfo.setPrevSubRouteID(ID);
}

inline uint32
SubRoute::getSubRouteID() const
{
   return m_destInfo.getPrevSubRouteID();
}

inline void
SubRoute::setPrevSubRouteID(uint32 ID)
{
   m_origInfo.setPrevSubRouteID(ID);
}

inline uint32
SubRoute::getPrevSubRouteID() const
{
   return m_origInfo.getPrevSubRouteID();
}

inline float
SubRoute::getDestOffset() const
{
   return m_destInfo.getOffset();
}

inline void
SubRoute::setDestOffset( float offset )
{
   m_destInfo.setOffset( offset );
}

inline float
SubRoute::getOrigOffset() const
{
   return m_origInfo.getOffset();
}

inline void
SubRoute::setOrigOffset( float offset )
{
   m_origInfo.setOffset( offset );
}

inline const Vehicle*
SubRoute::getOrigVehicle() const
{
   return m_origInfo.getVehicle();
}

inline bool
SubRoute::hasOneMap() const
{
   return ( m_origInfo.getMapID() == m_destInfo.getMapID() );
}

inline const OrigDestInfo*
SubRoute::getDestInfo() const
{
   return &m_destInfo;
}

inline void
SubRoute::setDestInfo( const OrigDestInfo& info )
{
   m_destInfo = info;
}

inline uint32
SubRoute::getCostASum() const
{
   return m_destInfo.getCostASum();
}

inline uint32
SubRoute::getCostBSum() const
{
   return m_destInfo.getCostBSum();
}

inline uint32
SubRoute::getCostCSum() const
{
   return m_destInfo.getCostCSum();
}

inline void
SubRoute::setCostASum(uint32 costSum)
{
   return m_destInfo.setCostASum(costSum);
}

inline void
SubRoute::setCostBSum(uint32 costSum)
{
   return m_destInfo.setCostBSum(costSum);
}

inline void
SubRoute::setCostCSum(uint32 costSum)
{
   return m_destInfo.setCostCSum(costSum);
}

#endif






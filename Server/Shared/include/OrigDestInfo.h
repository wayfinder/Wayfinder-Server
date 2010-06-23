/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ORIGDESTINFO_H
#define ORIGDESTINFO_H

#include "config.h"

#include <vector>
#include <list>
#include "Types.h"
#include "IDPairVector.h"

class Vehicle; // forward decl
class DriverPref; // forward decl

class SearchMatch;

/**
 *    This class hold information about origin and destination for each
 *    subroute. The information is used both for routing and expanding.
 *
 */
class OrigDestInfo {
public:

   /**
    *    Empty constructor. Creates an invalid OrigDestInfo to be used
    *    as starting node in the starting SubRouteContainer.
    *    @param prevIndex  To keep track of where the route started
    *                      on lower level.
    */
   OrigDestInfo( uint32 prevIndex = MAX_UINT32 );

   /**
    *    Main constructor.
    */
   OrigDestInfo(const Vehicle* pVehicle,
                uint32 mapID,
                uint32 nodeID,
                uint32 prevSubRouteID = MAX_UINT32,
                uint32 cost           = 0,
                uint32 estimatedCost  = 0,
                int32  lat            = MAX_INT32,
                int32  lon            = MAX_INT32,
                uint16 angle          = MAX_UINT16,
                uint32 costASum       = 0,
                uint32 costBSum       = 0,
                uint32 costCSum       = 0);

   /**
    *    Constructor for adding OrigDestInfo to list going in to RouteSender.
    *    Note that object will get pref->getBestVehicle.
    */
   OrigDestInfo(const DriverPref* pref,
                uint32 mapID,
                uint32 nodeID,
                int32  lat,
                int32  lon,
                float  offset,
                uint32 turnCost = 0);
   
   /**
    *    Copy constructor.
    */
   inline OrigDestInfo(const OrigDestInfo& info);

   /**
    *    Constructor for using when adding searchMatches to RouteObject.
    */
   OrigDestInfo(const SearchMatch& match);

   /**
    *    Returns the id of the node.
    */
   inline const IDPair_t& getID() const;

   /**
    *    Sets the id of the node.
    *    @param newID The new id to set.
    */
   inline void setID(const IDPair_t& newID);

   /**
    *    Returns the ID of the routing node this info refers to.
    */
   inline uint32 getNodeID() const;

   /**
    *    Sets the ID of the routing node this info refers to.
    */
   inline void setNodeID(uint32 ID);

   /**
    *    Returns the MC2 latitude.
    */
   inline int32 getLat() const;

   /**
    *    Sets the MC2 latitude.
    */
   inline void setLat(int32 lat);

   /**
    *    Returns the MC2 longtude.
    */
   inline int32 getLon() const;

   /**
    *    Sets the MC2 longtude.
    */
   inline void setLon(int32 lon);

   /**
    *    Returns the vehicle angle in degrees.
    */
   inline uint16 getAngle() const;

   /**
    *    Sets the vehicle angle in degrees. 0-359.
    *    @param angle The angle in degrees.
    */
   inline void setAngle(uint16 angle);

   /**
    *    Returns the ID of the map this node is in.
    */
   inline uint32 getMapID() const;

   /**
    *    Sets the ID of the map this node is in.
    */
   inline void setMapID(uint32 ID);

   /**
    *    Returns the ID of the SubRoute that leads to this node.
    */
   inline uint32 getPrevSubRouteID() const;

   /**
    *    Sets the ID of the SubRoute that leads to this node.
    */
   inline void setPrevSubRouteID(uint32 ID);

   /**
    *    Rerurns the cost of getting to this node
    */
   inline uint32 getCost() const;

   /**
    *    Sets the cost of getting to this node
    */
   inline void setCost(uint32 cost);

   /**
    *    Rerurns the estimated cost of this node
    */
   inline uint32 getEstimatedCost() const;

   /**
    *    Sets the estimated cost of this node
    */
   inline void setEstimatedCost(uint32 cost);

   /**
    *    Returns how far in (fraction) to the segment the start or dest is.
    */
   inline float getOffset() const;

   /**
    *    Sets how far in (fraction) to the segment the start or dest is.
    */
   inline void setOffset(float offset);

   /**
    *    Returns the cost of going in the direction the vehicle
    *    does not start in.
    */
   inline uint32 getTurnCost() const;

   /**
    *    Sets the cost of going in the direction the vehicle
    *    does not start in.
    */
   inline void setTurnCost(uint32 turnCost);

   /**
    *    Returns a pointer to the vehicle that the user has
    *    when arriving to this node.
    */
   inline const Vehicle* getVehicle() const;

   /**
    *    Sets the vehicle that the user has when arriving to this node.
    */
   inline void setVehicle(const Vehicle* pVehicle);

   /**
    *    Returns the costA sum for the route up to this point.
    */
   inline uint32 getCostASum() const;

   /**
    *    Returns the costB sum for the route up to this point.
    */
   inline uint32 getCostBSum() const;

   /**
    *    Returns the sum of costs C up to this point.
    */
   inline uint32 getCostCSum() const;

   /**   Sets the costASum (for use with sortdist). */
   inline void setCostASum(uint32 costASum);

   /**   Sets the costBSum (for use with sortdist). */
   inline void setCostBSum(uint32 costBSum);
   
   /**   Sets the costCSum (for use with sortdist). */
   inline void setCostCSum(uint32 costCSum);

protected:
   
   /**
    *    The map and item id of the node.
    */
   IDPair_t m_id;
private:
   
   /**
    *    MC2 latitude of this node.
    */
   int32 m_lat;

   /**
    *    MC2 longitude of this node.
    */
   int32 m_lon;

   /**
    *    Starting angle of vehicle in degrees.
    */
   uint16 m_angle;
   

   /**
    *    ID of the SubRoute that leads to this node.
    */
   uint32 m_prevSubRouteID;

   /**
    *    Cost of getting to this node
    */
   uint32 m_cost;

   /**
    *    The estimated cost of getting from origin to the destination
    *    via this node.
    */
   uint32 m_estimatedCost;

   /**
    *    How far in (fraction) to the segment the start or dest is
    */
   float m_offset;

   /**
    *    The cost of going in the direction the vehicle does not start in
    */
   uint32 m_turnCost;

   /**
    *    The vehicle that the user has when arriving to this node.
    */
   const Vehicle* m_vehicle;

   /**
    *    The sum of the costs A for the routes up to this point.
    */
   uint32 m_costASum;

   /**
    *    The sum of cost B for the routes up to this point.
    */
   uint32 m_costBSum;

   /**
    *    The sum of cost C up to this point.
    */
   uint32 m_costCSum;
};

class OrigDestInfoList : public list<OrigDestInfo> {

public:

   inline void addOrigDestInfo(const OrigDestInfo& info);
   
};

class OrigDestInfoListList : public list<OrigDestInfoList*> {};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline
OrigDestInfo::OrigDestInfo(const OrigDestInfo& info)
      :    m_id(info.m_id),
           m_lat(info.m_lat),
           m_lon(info.m_lon),
           m_angle(info.m_angle),
           m_prevSubRouteID(info.m_prevSubRouteID),
           m_cost(info.m_cost),
           m_estimatedCost(info.m_estimatedCost),
           m_offset(info.m_offset),
           m_turnCost(info.m_turnCost),
           m_vehicle(info.m_vehicle),
           m_costASum(info.m_costASum),
           m_costBSum(info.m_costBSum),
           m_costCSum(info.m_costCSum)
{
}


inline const IDPair_t&
OrigDestInfo::getID() const
{
   return m_id;
}

inline void
OrigDestInfo::setID(const IDPair_t& newID)
{
   m_id = newID;
}

inline uint32
OrigDestInfo::getNodeID() const
{
   return m_id.second;
}

inline void
OrigDestInfo::setNodeID(uint32 ID)
{
   m_id.second = ID;
}

inline int32
OrigDestInfo::getLat() const
{
   return m_lat;
}

inline void
OrigDestInfo::setLat(int32 lat)
{
   m_lat = lat;
}

inline int32
OrigDestInfo::getLon() const
{
   return m_lon;
}

inline void
OrigDestInfo::setLon(int32 lon) {
   m_lon = lon;
}

inline uint16
OrigDestInfo::getAngle() const {
   return m_angle;
}

inline void
OrigDestInfo::setAngle(uint16 angle) {
   m_angle = angle;
}

inline uint32
OrigDestInfo::getMapID() const {
   return m_id.getMapID();
}

inline void
OrigDestInfo::setMapID(uint32 ID) {
   m_id.first = ID;
}

inline uint32
OrigDestInfo::getPrevSubRouteID() const {
   return m_prevSubRouteID;
}

inline void
OrigDestInfo::setPrevSubRouteID(uint32 ID) {
   m_prevSubRouteID = ID;
}

inline uint32
OrigDestInfo::getCost() const {
   return m_cost;
}

inline void
OrigDestInfo::setCost(uint32 cost) {
   m_cost = cost;
}

inline uint32
OrigDestInfo::getEstimatedCost() const {
   return m_estimatedCost;
}

inline void
OrigDestInfo::setEstimatedCost(uint32 cost) {
   m_estimatedCost = cost;
}

inline float
OrigDestInfo::getOffset() const {
   return m_offset;
}

inline void
OrigDestInfo::setOffset(float offset) {
   m_offset = offset;
}

inline uint32
OrigDestInfo::getTurnCost() const {
   return m_turnCost;
}

inline void
OrigDestInfo::setTurnCost(uint32 turnCost) {
   m_turnCost = turnCost;
}

inline const Vehicle*
OrigDestInfo::getVehicle() const {
   return m_vehicle;
}

inline void
OrigDestInfo::setVehicle(const Vehicle* pVehicle) {
   m_vehicle = pVehicle;
}

inline void
OrigDestInfoList::addOrigDestInfo( const OrigDestInfo& info )
{
   push_back( info );
}

inline uint32
OrigDestInfo::getCostASum() const
{
   return m_costASum;
}

inline uint32
OrigDestInfo::getCostBSum() const
{
   return m_costBSum;
}

inline uint32
OrigDestInfo::getCostCSum() const
{
   return m_costCSum;
}

inline void
OrigDestInfo::setCostASum(uint32 costASum) 
{
   m_costASum = costASum;
}

inline void
OrigDestInfo::setCostBSum(uint32 costBSum)
{
   m_costBSum = costBSum;
}

inline void
OrigDestInfo::setCostCSum(uint32 costCSum)
{
   m_costCSum = costCSum;
}

#endif





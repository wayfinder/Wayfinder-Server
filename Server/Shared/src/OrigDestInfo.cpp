/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "OrigDestInfo.h"
#include "GfxConstants.h"
#include "DriverPref.h"
#include "SearchMatch.h"

OrigDestInfo::OrigDestInfo( uint32 prevIndex)
      : m_id(MAX_UINT32, MAX_UINT32)
{
   m_vehicle = NULL;   
   m_prevSubRouteID = prevIndex;
   m_cost = MAX_UINT32;
   m_estimatedCost = MAX_UINT32;
   m_offset = 0;
   m_turnCost = 0;
   m_lat = GfxConstants::IMPOSSIBLE;
   m_costASum = 0;
   m_costBSum = 0;
   m_costCSum = 0;
}

OrigDestInfo::OrigDestInfo(const Vehicle* pVehicle,
                           uint32 mapID,
                           uint32 nodeID,
                           uint32 prevSubRouteID,
                           uint32 cost,
                           uint32 estimatedCost,
                           int32  lat,
                           int32  lon,
                           uint16 angle,
                           uint32 costASum,
                           uint32 costBSum,
                           uint32 costCSum)
{
   m_vehicle = pVehicle;
   m_id.first = mapID;
   m_id.second = nodeID;
   m_prevSubRouteID = prevSubRouteID;
   m_cost = cost;
   m_estimatedCost = estimatedCost;
   m_offset = 0;
   m_turnCost = 0;
   if ( lat == MAX_INT32 ) {
      m_lat = GfxConstants::IMPOSSIBLE;
      m_lon = MAX_INT32;
   } else {
      m_lat = lat;
      m_lon = lon;
   }
   m_angle = angle;
   m_costASum = costASum;
   m_costBSum = costBSum;
   m_costCSum = costCSum;
}   

OrigDestInfo::OrigDestInfo(const DriverPref* pref,
                           uint32 mapID,
                           uint32 nodeID,
                           int32  lat,
                           int32  lon,
                           float  offset,
                           uint32 turnCost)
      : m_id(mapID, nodeID)
{
   m_vehicle = pref->getBestVehicle();
   m_prevSubRouteID = MAX_UINT32;
   m_cost = 0;
   m_estimatedCost = 0;
   m_offset = offset;
   m_turnCost = turnCost;
   if ( lat == MAX_INT32 ) {
      m_lat = GfxConstants::IMPOSSIBLE;
      m_lon = MAX_INT32;
   } else {
      m_lat = lat;
      m_lon = lon;
   }      
   m_angle = MAX_UINT16;
   m_costASum = 0;
   m_costBSum = 0;
   m_costCSum = 0;
}
   
OrigDestInfo::OrigDestInfo(const SearchMatch& match)
{
   m_vehicle = NULL;
   m_id      = match.getID();
   m_offset  = 0.5;
   m_lat     = GfxConstants::IMPOSSIBLE;
   m_lon     = MAX_INT32;
}

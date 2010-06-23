/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INFO_PROCESSOR_H
#define INFO_PROCESSOR_H

#include "MapHandlingProcessor.h"

#include <map>
#include <memory>

class InfoSQL;

class GfxFeatureMapRequestPacket;
class GfxFeatureMapReplyPacket;
class DisturbanceRequestPacket;
class DisturbanceReplyPacket;
class RouteTrafficCostRequestPacket;
class RouteTrafficCostReplyPacket;
class GetTMCCoordinateRequestPacket;
class GetTMCCoordinateReplyPacket;
class DisturbanceInfoRequestPacket;
class DisturbanceInfoReplyPacket;
class DisturbanceChangesetReplyPacket;
class DisturbanceChangesetRequestPacket;
class FetchAllDisturbancesRequestPacket;
class FetchAllDisturbancesReplyPacket;
class TrafficElementDatabase;
class TrafficMapInfo;
class UpdateDisturbanceRequestPacket;
class UpdateDisturbanceReplyPacket;
class TrafficAccessRequestPacket;
class TrafficAccessReplyPacket;
class DisturbanceChangeset;
class UserRightsMapInfo;
class DisturbanceElement;

/**
  *   Processes requestpackets that come to the InfoModule.
  *
  *   Some sort of updateDatabase will be needed to update the start
  *   and end time of the periodical disturbances in the database.
  *   If not they will need several updates each when they are loaded
  *   which will take time if there are many. 
  * 
  */
class InfoProcessor : public MapHandlingProcessor {
public :

   /**
    *   Creates a new InfoProcessor that uses the loadedMaps to
    *   communicate with the reader.
    *
    *   @param   loadedMaps  Vector used to tell the Reader about.
    */
   InfoProcessor(MapSafeVector* loadedMaps);

   /**
    *   Deletes the InfoProcessor and releases the allocated 
    *   memory.
    */
   virtual ~InfoProcessor();

   /**
    *   Returns the TrafficMapInfo with this mapID.
    *   @param mapID  The mapID.
    *   @return The TrafficMapInfo with this mapID.
    */
   TrafficMapInfo* getTrafficMapInfo(const uint32 mapID);


   /**
    *   Used to return the current status (load) of this processor.
    *   This is currently not used.
    *
    *   @return  The status (load) of this processor.
    */
   int getCurrentStatus();

   /**
    *    Virtual function for loading a map. Should be called
    *    by handleRequest when that function gets a loadMapRequest
    *    and isn't virtual anymore.
    *    @param mapID The map to load.
    *    @param mapSize Outparameter describing the size of the
    *                   map.
    *    @return StringTable::OK if ok.
    */
   virtual StringTable::stringCode loadMap(uint32 mapID,
                                           uint32& mapSize);


   /**
    *    Virtual function to be called when a map should
    *    be deleted.
    *    @param mapID The map to be deleted.
    *    @return StringTable::OK if ok.
    */
   virtual StringTable::stringCode deleteMap(uint32 mapID);

protected:
   /**
    *   The ``main method'' that the JobThread calls to get answers
    *   to the request packets.
    *
    *   @param   p  The request packet to handle.
    *   @return  A packet that is a reply of the request given as
    *            parameter.
    */
   Packet* handleRequestPacket( const RequestPacket& p,
         char* packetInfo);

private:

   /**
    *   Creates a feature map reply packet containing the
    *   disturbances for the supplied bounding box and
    *   so on.
    *   @param p Request packet.
    *   @param packetInfo JobThread info to print.
    */
   inline GfxFeatureMapReplyPacket*
      handleGfxFeatureMapRequest(const GfxFeatureMapRequestPacket* p,
                                 char* packetInfo );

   /**
    *    Fills the RouteTrafficCostReplyPacket with disturbances from
    *    this map.
    */
   inline RouteTrafficCostReplyPacket*
      handleRouteTrafficCostRequest(const RouteTrafficCostRequestPacket* r,
                                    char* packetInfo );

   /**
    *   Creates a multimap containing all the active disturbances
    *   in the time interval from the DisturbanceRequestPacket located
    *   on the mapID from the DisturbanceRequestPacket.
    */
   inline DisturbanceReplyPacket*
      handleDisturbanceRequest(const DisturbanceRequestPacket* p);

   inline TrafficAccessReplyPacket*
      handleTrafficAccessRequest(const TrafficAccessRequestPacket* p);

   inline DisturbanceInfoReplyPacket*
      handleDisturbanceInfoRequest(const DisturbanceInfoRequestPacket* p);

   inline UpdateDisturbanceReplyPacket*
      handleUpdateDisturbanceRequest( const UpdateDisturbanceRequestPacket& udrp );

   inline GetTMCCoordinateReplyPacket*
      handleGetTmcCoordRequest(const GetTMCCoordinateRequestPacket& gtcrp);

   DisturbanceChangesetReplyPacket*
   handleDisturbanceChangesetRequest( const DisturbanceChangesetRequestPacket&
                                      packet );

   /// Handle Fetch all disturbances request
   FetchAllDisturbancesReplyPacket*
   handleFetchAllDisturbances( const FetchAllDisturbancesRequestPacket& pack );

   /// 
   /// @param changes The current disturbance changes
   void updateTrafficUnits( DisturbanceChangeset& changes );

   /**
    *    Removes redundant rights for this user on this map.
    *    @param userRights The rights to update
    *    @return Number of rights removed
    */
   inline void updateUserRights(UserRightsMapInfo& userRights);

   typedef map<uint32, TrafficMapInfo*> TrafficMap;
   /// A map that stores TrafficMapInfo.
   TrafficMap m_handleUnitMap;

   /// A SQL database that stores disturbances.
   auto_ptr<InfoSQL> m_database;

   /// Handles traffic element request/replies
   auto_ptr<TrafficElementDatabase> m_trafficDatabase;
};

#endif


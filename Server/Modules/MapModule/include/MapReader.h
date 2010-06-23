/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPREADER_H
#define MAPREADER_H

#include "config.h"

#include <sys/types.h>
#include <dirent.h>

#include "StandardReader.h"

#include "StringTable.h"
#include "ItemTypes.h"
#include "MapModuleNoticeContainer.h"

class Vector;

// Forward
class TopRegionReplyPacket;
class SmallestRoutingCostRequestPacket;
class SmallestRoutingCostReplyPacket;
class MMRoutingCostTable;
class MapReplyPacket;
class RoutingInfoRequestPacket;
class RoutingInfoReplyPacket;
class MC2Coordinate;
class BBoxRequestPacket;
class BBoxReplyPacket;
class AllMapRequestPacket;
class AllMapReplyPacket;
class CopyrightBoxRequestPacket;
class CopyrightBoxReplyPacket;

/**
  *   Class that handles some packets to the MapModule in a special way.
  *
  */
class MapReader : public StandardReader {
   /**
     *   The name of the file used to store the information about the maps
     *   needed by the leader.
     */
   #define INDEX_NAME "index.db"

   public :
       /**
        *   Creates an MapReader according to the specifications.
        *   
        *   @param   q              The queue where to insert the request 
        *                           to be  processed by the processor.
        *   @param   loadedMaps     Where to look for the currently loaded
        *                           maps.
        *   @param   preferedPort   The port to use if possible
        *   @param   leaderIP       IP-address of the leader multicast address.
        *   @param   leaderPort     Port for packets to the leader.
        *   @param   availableIP    IP-address of the available multicast 
        *                           address.
        *   @param   availablePort  Port for packets to the available's.
        *   @param   definedRank    Value used when voting for leader. High
        *                           value indicated that the system 
        *                           administrator want this Module to be
        *                           leader.
        *   @param   startWithMaps  ID of the maps to load. If NULL all maps
        *                           are loaded.
        */
      MapReader(  Queue *q,
                  NetPacketQueue& sendQueue,
                  MapSafeVector* loadedMaps,
                  PacketReaderThread* packetReader, uint16 port,
                  uint32 definedRank,
                  Vector* startWithMaps);

      /**
        *   Deletes the MapReader and releases the allocated memory.
        */
      virtual ~MapReader();

      /**
        *   Take care of some of the special controllpackets that might
        *   be received by the MapModule-leader. Calls the 
        *   leaderProcessCtrlPacket in the super class to take care of 
        *   the ``ordinary'' control packet.
        *
        *   @param   p  The incomming control packet.
        *   @return  True if the packet was handled by this method or
        *            by the method in the superclass, false otherwise.
        */
      bool leaderProcessCtrlPacket(Packet *p);

      /**
        *   Loads the MapModuleObjVector when become leader. Calls the
        *   initLeader() in the superclass to ``do the ordinary things''.
        */  
      void initLeader(set<MapID>& allMaps);

      /**
        *   Makes Generic map to print a mid mif version of an Items
        *   attributes and gfx-data.
        *   @param  mapId  The identity of the map
        *   @param  type   The item type to create.
        *   @retruns  true if map is created, false if otherwise.
        */
      bool printMidMif(uint32 mapId, ItemTypes::itemType type);

      
private:
      /**
       *    Adds info about the overview maps to the MapRequestPacket
       *    if the packet is requesting a routemap. Uses the country
       *    information to get the country map with the same country
       *    code as the requested map and then converts the country map
       *    id to an overview map id.
       *    <br />
       *    STORKA: This should be removed when the maps have this info
       *            in them.
       *    @param p The MapRequestPacket
       */
      inline MapReplyPacket* handleMapRequestPacket(Packet* p);
      
      /**
       *    Adds all top regions to the reply packet.
       *    @param   p  TopRegionRequestPacket.
       *    @return  A new TopRegionReplyPacket containing all the
       *             top regions.
       */
      inline TopRegionReplyPacket* handleTopRegionRequestPacket( 
                                                         Packet* p ) const;

      /**
       *    Fills a reply with information needed by the RouteSender.
       *    Currently number of map levels and distances to use when
       *    routing on them.
       *    @param req The request.
       *    @return A reply with the information.
       */
      inline RoutingInfoReplyPacket* handleRoutingInfoRequest(
         const RoutingInfoRequestPacket* req) const;


      /**
       *    Uses Properties <b>ROUTE_DISTANCE_LEVEL_n</b>
       *    to find the distance to be used 
       *    to decide when to start routing on higher level
       *    for the supplied level. If there is no such property
       *    the defaults are used.
       *    @param level The level to move from.
       *    @return The level should be left when the distance is
       *            larger than the value returned here.
       */
      inline uint32 getRouteDistanceForLevel(uint32 level) const;
      
      /**
       *    Takes care of a smallestRoutingCostRequestPacket and returns
       *    a reply. 
       *    {\bf NB!} The reply is created inside this method and both
       *    the request and the reply {\bf must} be deleted by the caller!
       */
      inline SmallestRoutingCostReplyPacket* 
         handleSmallestRoutingCostRequestPacket(
            const SmallestRoutingCostRequestPacket* p ) const;

      /**
        *   Used internal to get the map where a point is located.\\
        *   {\it  Currently this method uses linear search among the 
        *         maps!}
        *   
        *   @param   lat   The latitude for the point.
        *   @param   lon   The longitude for the point.
        *   @param   mapDist Set to the signed square distance from the
        *                    point to the map if not NULL. Default NULL.
        *   @return  Id of the map where point is located.
        */
      uint32 findMapFromCoordinate( int32 lat, int32 lon, float64* 
                                    mapDist = NULL );

      void findMapsAtCoordinate(int32 lat, int32 lon, Vector* mapIDs); 
      
      /**
        *   Used internally to get the maps that are covered by
        *   a boundingbox. Note that this method may include maps that
        *   really are not covered, since it uses the boundingboxes of
        *   the maps to compare with. This on the other hand a 
        *   much faster way to do it.
        *   {\it  Currently this method uses linear search among the 
        *         maps!}
        *   
        *   @param   bbox     The boundingbox.
        *   @param   mapIDs   Preallocated empty vector which will 
        *                     contain the mapids of the maps covered
        *                     by the boundingbox.
        *   @param   onlyAddCountryMaps   Optional parameter, 
        *                                 default false. Indicates
        *                                 if only country maps should
        *                                 be included or not.
        */
      void findMapsInMC2BoundingBox(const MC2BoundingBox* bbox,
                                    Vector* mapIDs,
                                    bool onlyAddCountryMaps = false);

      /**
       *    Finds the maps covered by a circle with center 
       *    in <code>center</code> and with a radius <code>radius</code>.
       *    @param mapIDs Vector to put the mapIDs in.
       *    @param center Center of the circle.
       *    @param radiusMeters Radius in verktum.
       *    @param addUnderviewMaps True if underview maps are interesting.
       *    @param addCountryMaps   True if country maps are interesting.
       */
      void findMapsWithinRadius(Vector& mapIDs,
                                const MC2Coordinate& center,
                                uint32 radiusMeters,
                                bool addUnderviewMaps       = true,
                                bool addCountryMaps         = false);

      /**
       *    Handles a bborequest packet.
       */
      BBoxReplyPacket* handleBBoxRequest( const BBoxRequestPacket* req );

      /**
       *    Handles the AllMap packet.
       */
      AllMapReplyPacket* handleAllMapRequest( const AllMapRequestPacket* req );
   
   CopyrightBoxReplyPacket* 
   handleCopyrightBoxRequest( const CopyrightBoxRequestPacket* packet ) const;

      /**
        *   Vector containing ID of the maps to start loading.
        */
      Vector* m_startWithMaps;
      
      /**
       *    Container with information about all maps.
       *    Has replaced the MapModuleObjVector.
       */
      MapModuleNoticeContainer m_indexDB;

      /**
       *    A table containing the smallest routing costs
       *    from map to map.
       */
      MMRoutingCostTable* m_routingCostTable;

};


// =======================================================================
//                                 Implementation of the inlined methods =

#endif


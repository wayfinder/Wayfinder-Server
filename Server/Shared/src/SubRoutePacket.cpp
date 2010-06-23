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

#include "Types.h"
#include "multicast.h"

#include "SubRouteVector.h"
#include "OrigDestInfo.h"
#include "DriverPref.h"
#include "Vehicle.h"
#include "SubRoute.h"

#include "DisturbanceList.h"
#include "ItemTypes.h"

#include "SubRoutePacket.h"
#include "MapBits.h"

#include <vector>

#define IS_STATE_ELEMENT(a) ( GET_TRANSPORTATION_STATE(a) )

#define ORIG_DEST_SIZE            (20+12)
#define SUB_ROUTE_SIZE            (20+12)
#define SUB_ROUTE_CONNECTION_SIZE 32
#define DISTURBANCE_SIZE          (4+4+4+4+4)

SubRouteRequestPacket::SubRouteRequestPacket(const DriverPref* driverPref,
                                             const SubRouteVector* origs,
                                             const OrigDestInfoList* dests,
                                             const OrigDestInfoList* allDests,
                                             bool routeToOneDest,
                                             uint32 cutOff,
                                             int levelDelta,
                                             const DisturbanceVector*
                                             disturbances,
                                             bool calcCostSums,
                                             bool dontSendSubRoutes,
                                             const DisturbanceVector*
                                             infoModuleDists )
{   
   mc2dbg4 << "SubRouteRequestPacket::SubRouteRequestPacket" << endl;
   setPriority(DEFAULT_PACKET_PRIO);
   // Calculate size of request
   // Origins and destinations
  
   // Do something about the listtype
   if ( driverPref->getVehicleRestriction() != ItemTypes::pedestrian ) {
      setListType(SubRouteListTypes::LOWER_LEVEL);
   } else {
      // Pedestrian
      setListType(SubRouteListTypes::LOWER_LEVEL_WALK); // Lower level walk
   }

   uint32 reqSize = SUBROUTE_REQUEST_HEADER_SIZE;

   // The number of subroutes
   int nbrSubRoutes = origs->getSize() +
      // Add the destinations for HIGHER_LEVEL too
    ((getListType() == SubRouteListTypes::HIGHER_LEVEL) ? dests->size() : 0);

   
   // Origin. Will probably be removed
   if ( origs != NULL &&
        getListType() == SubRouteListTypes::HIGHER_LEVEL )
      reqSize += origs->size() * ORIG_DEST_SIZE;
   // Destinations 
   if ( dests != NULL )
      reqSize += dests->size() * ORIG_DEST_SIZE;
   // Subroutes.
   for(uint32 i=0; i < origs->size() ; i++ ) {
      reqSize += SUB_ROUTE_SIZE +
         SUB_ROUTE_CONNECTION_SIZE * 1;
   }

   // Compensate for disturbances
   // Length
   reqSize += 4;
   if ( disturbances != NULL ) {
      reqSize += DISTURBANCE_SIZE*disturbances->size();
   }
   reqSize += 4;
   if ( infoModuleDists != NULL ) {
      reqSize += 8 * infoModuleDists->size();
   }

   if ( reqSize > getBufSize() ) {
      mc2dbg2 << "SubRouteRequestPacket - buffer too small reallocing"
              << endl;
      byte* temp = MAKE_UINT32_ALIGNED_BYTE_BUFFER( reqSize * 2);
      memcpy(temp, this->buffer, SUBROUTE_REQUEST_HEADER_SIZE);      
      delete [] this->buffer;
      this->buffer = temp;
      this->bufSize = reqSize * 2;
   }

   // First request? Won't exactly work all the time...
   bool original = origs->front()->getPrevSubRouteID() == MAX_UINT32;

   if ( levelDelta == 1 ) {
      // Higher level routing - do some tricks
      setMapID( FIRST_OVERVIEWMAP_ID  );
      setListType(SubRouteListTypes::HIGHER_LEVEL);
   }

   //setOriginIP(leaderIP);
   //setOriginPort(leaderPort);
   setSubType(Packet::PACKETTYPE_SUBROUTEREQUEST);
   // Get mapid from one of the origins
   setMapID(origs->front()->getNextMapID() );
   // Setting the extra vehicle to avoid toll roads.
   uint32 extraVehicle = 0;
   if ( driverPref->avoidTollRoads() ) {
      extraVehicle |= ItemTypes::avoidTollRoad;
   }
   if ( driverPref->avoidHighways() ) {
      extraVehicle |= ItemTypes::avoidHighway;
   }
   setVehicleRestrictions( driverPref->getVehicleRestriction() |
                           extraVehicle );
                                                  
   setRoutingCosts(driverPref->getRoutingCosts());
   setUseTurnCost(driverPref->useUturn());
   setTime(driverPref->getTime());
   setMinWaitTime(driverPref->getMinWaitTime());

   // Some flags.
   setRouteToAll(! routeToOneDest );
   setDontSendSubRoutes( dontSendSubRoutes );
   setCalcCostSums( calcCostSums );
      
   setRouteID(0);  // Don't know what it should be. Seems to be the same
                   // as the request id, but for the RouteReader.
   setCutOff(cutOff);
   
   setIsOriginalRequest(original);
   
   // Add the origins that belongs to this mapID and only the first time.
   setNbrOrigins(0);
   int pos = SUBROUTE_REQUEST_HEADER_SIZE;

   if ( original ) {
      for(uint32 i=0; i < origs->getSize(); i++) {
         const SubRoute* curSubRoute = origs->getSubRouteAt(i);
         // Destinations are origins in next route.
         const OrigDestInfo* orig = curSubRoute->getDestInfo();
         addOrigin(*orig, pos);
//           addOrigin(curSubRoute->getNextMapID(),
//                     curSubRoute->getDestNodeID(),
//                     curSubRoute->getDestOffset(),
//                     MAX_UINT32,
//                     MAX_UINT32,
//                     pos);
      }
   }
   
   // Add all destinations ( not allDests, they aren't used yet)
   setNbrDestinations(0);
   OrigDestInfoList::const_iterator it;
   it = dests->begin();
   while ( it != dests->end() ) {
      addDestination(*it, pos);
      it++;
   }

   setNbrSubRoutes( nbrSubRoutes );
   for (uint32 i = 0; i < origs->getSize(); i++) {
      SubRoute* subRoute = (*origs)[i];
      if (subRoute != NULL) {
         addSubRoute(subRoute, pos);
      }
   }

   if ( getListType() == SubRouteListTypes::HIGHER_LEVEL ) {
      OrigDestInfoList::const_iterator it(dests->begin());
      for(uint32 i=0; i < dests->size() ; ++i ) {
         SubRoute subRoute(*it, *it);
         ++it;
         // Add backward subroute for each destination.
         // The routemodule uses them as destinations later.
         addSubRoute(&subRoute, pos, false);
      }
   }
   
   addDisturbances(pos, disturbances);
   // Add info module disturbances using the more compact format.
   addInfoModDisturbances( pos, infoModuleDists );
   setLength(pos);
   
   if ( true || pos > (int)reqSize || (reqSize - pos) > 65536 ) {
      mc2dbg2 << "SubRouteRequestPacket - calcSize = " << reqSize 
           << "real size = " << pos << endl;
   }
} // Constructor

void SubRouteRequestPacket::addDisturbances(int& pos,
                                            const DisturbanceVector* vect)
{
   if ( vect == NULL ) {
      incWriteLong(pos, 0);
      return;
   }
   int vectSize = vect->size();
   incWriteLong(pos, vectSize);
   for(int i=0; i < vectSize; ++i) {
      const DisturbanceListElement* el = (*vect)[i];
      incWriteLong(pos, el->getMapID());
      incWriteLong(pos, el->getNodeID());
      incWriteLong(pos, el->getRawFactor());
      incWriteLong(pos, el->getUseFactor());
      incWriteLong(pos, el->getTime());
   }
}


void SubRouteRequestPacket::
addInfoModDisturbances(int& pos,
                       const DisturbanceVector* vect)
{
   if ( vect == NULL ) {
      incWriteLong(pos, 0);
      return;
   }
   int vectSize = vect->size();
   incWriteLong(pos, vectSize);
   for(int i=0; i < vectSize; ++i) {
      const DisturbanceListElement* el = (*vect)[i];
      MC2_ASSERT( el->getUseFactor() );
      incWriteLong(pos, el->getNodeID());
      incWriteLong(pos, el->getRawFactor());
   }
}


void SubRouteRequestPacket::addOrigin( const OrigDestInfo& origin,
                                       int32& pos )
{
   incWriteLong( pos, origin.getMapID() );
   incWriteLong( pos, origin.getNodeID() );
   
   incWriteShort( pos, uint16(origin.getOffset() * float(MAX_UINT16)) );
   // If the node is node 0 it has turncost if we're driving towards 0
   // This may be wrong though.
   const bool hasTurnCost = origin.getTurnCost();
   bool drivingTowardsZero = !MapBits::isNode0(origin.getNodeID());
   drivingTowardsZero ^= hasTurnCost;
   incWriteByte( pos, !drivingTowardsZero );
   incWriteByte( pos, 0 );  // pad byte
   
   incWriteLong( pos, origin.getLat() );
   incWriteLong( pos, origin.getLon() );
   setNbrOrigins( getNbrOrigins()+1 );
}

void SubRouteRequestPacket::addOrigin( uint32 mapID,
                                       uint32 nodeID,
                                       float  offset,
                                       uint32 lat,
                                       uint32 lon,
                                       int32& pos)
{
   incWriteLong( pos, mapID );
   incWriteLong( pos, nodeID );
   incWriteShort( pos, (uint16)(offset * float(MAX_UINT16)));
   // If the node is node 0 we are driving towards node 1.
   incWriteByte( pos, MapBits::isNode0( nodeID ));
   incWriteByte( pos, 0 );  // pad byte
   
   incWriteLong( pos, lat );
   incWriteLong( pos, lon );
   setNbrOrigins( getNbrOrigins()+1 );
}
   
void SubRouteRequestPacket::addDestination( const OrigDestInfo& dest,
                                            int32& pos )
{
   incWriteLong( pos, dest.getMapID() );
   incWriteLong( pos, dest.getNodeID() );

   incWriteShort( pos, uint16(dest.getOffset() * float(MAX_UINT16)) );
   incWriteShort( pos, 0 );                  // pad bytes

   incWriteLong( pos, dest.getLat() );
   incWriteLong( pos, dest.getLon() );
   setNbrDestinations( getNbrDestinations()+1 );
}

void SubRouteRequestPacket::addSubRoute( const SubRoute* subRoute,
                                         int32& pos,
                                         bool forward )
{
   incWriteLong( pos, subRoute->getSubRouteID() );
   incWriteLong( pos, subRoute->getPrevSubRouteID() );
   incWriteByte( pos, uint8(false) ); // Route complete
   incWriteByte( pos, uint8(forward) ); // Forward

   // We only have one external connection per subroute now.
   uint32 nbrConnections = 1;
   incWriteShort( pos, nbrConnections );
   for (uint32 j = 0; j < nbrConnections; j++) {
      incWriteLong(pos, subRoute->getNextMapID() );
      incWriteLong(pos, subRoute->getDestNodeID() );
      incWriteLong(pos, subRoute->getCost() );
      incWriteLong(pos, subRoute->getEstimatedCost() );
      mc2dbg8 << "Writing cost A sum " << subRoute->getCostASum() << endl;
      incWriteLong(pos, subRoute->getCostASum() );
      mc2dbg8 << "Writing cost B sum " << subRoute->getCostBSum() << endl;
      incWriteLong(pos, subRoute->getCostBSum() );
      mc2dbg8 << "Writing cost C sum " << subRoute->getCostCSum() << endl;
      incWriteLong(pos, subRoute->getCostCSum() );
   }
}

// ========================================================================
//                                                    SubRouteReplyPacket =

const Vehicle*
SubRouteReplyPacket::stateElementToVehicle(const DriverPref* driverPref,
                                           uint32 stateElement)
{
   uint32 vehicleToFind = ItemTypes::passengerCar;
   switch ( stateElement ) {
      case 0xf0000002:
         vehicleToFind = ItemTypes::pedestrian;
         break;
      case 0xf0000003:
         vehicleToFind = ItemTypes::bicycle;
         break;
   }
   
   for(int i=0; i < driverPref->getNbrVehicles(); ++i ) {
      const Vehicle* curVeh = driverPref->getVehicle(i);
      if ( curVeh->getVehicleMask() == vehicleToFind )
         return curVeh;
   }
   mc2log << warn << here << " Couldn't find suitable vehicle for "
          << "state element " << hex << stateElement << dec << endl;
   return driverPref->getBestVehicle();
}


void SubRouteReplyPacket::getSubRouteVector( const DriverPref* driverPref,
                                             SubRouteVector& subRouteVector )
{
   int pos = SUBROUTE_REPLY_PACKET_SIZE;

   const int maxNbrCostSums = 3;
   // Temporary structure to hold the data.
   struct extern_conn_t {
      uint32 mapID;
      uint32 nodeID;
      uint32 cost;
      uint32 estCost;
      int32 lat;
      int32 lon;
      uint16 offset;
      int nbrCosts;
      uint32 costSums[maxNbrCostSums]; // For sum of costs A-C
   };
   
   // Get some data. Wait till later with what to do with them.
   getListType();
   getRouteID();
   getCutOff();
   
   const uint32 nbrSubRoutes = getNbrSubRoutes();
   
   for (uint32 i = 0; i < nbrSubRoutes; i++) {
      
      // Get a vehicle that we can use when creating OrigDestInfos.
      // Use the best vehicle in the driverPref.
      const Vehicle* vehicle = driverPref->getBestVehicle();

      // Must read some stuff before creating the SubRoute.

      // SubRoute Id should not be used, I think. A new ID will be
      // assigned when the route is inserted into the ServerSubRouteVector.
      // Will set it to MAX_UINT32 when creating the origin.
      const uint32 subRouteID     = incReadLong(pos);
      mc2dbg8 << "[SubRouteReplyPacket]: I am using " << subRouteID << endl;
      // How can the RouteModule know the ID of the route?
      const uint32 prevSubRouteID = incReadLong(pos);
      incReadByte(pos); // Complete - not used
      incReadByte(pos); // Forward - Not used
      incReadShort( pos ); // Skip something
      const uint32 mapID          = incReadLong(pos);
      
      const int nbrConnections = incReadLong(pos);
      const int nbrNodes = incReadLong(pos);      

      struct extern_conn_t* extConns = new extern_conn_t[nbrConnections];
      
      for (int j = 0; j < nbrConnections; j++) {
         // Zero the cost-sums
         for(int x=0; x < maxNbrCostSums; ++x )
            extConns[j].costSums[x] = 0;
         
         extConns[j].mapID    = incReadLong(pos);
         extConns[j].nodeID   = incReadLong(pos);
         extConns[j].cost     = incReadLong(pos);
         extConns[j].estCost  = incReadLong(pos);
         extConns[j].lat      = incReadLong(pos);
         extConns[j].lon      = incReadLong(pos);
         extConns[j].offset   = incReadShort(pos);
         extConns[j].nbrCosts = incReadShort(pos);
         mc2dbg8 << "Number of costs = " << extConns[j].nbrCosts << endl;
         // The number of costs (probably a,b,c)
         for(int x=0; x < extConns[j].nbrCosts; ++x ) {
            if ( x < maxNbrCostSums ) {
               extConns[j].costSums[x] = incReadLong(pos);               
               mc2dbg4 << "Cost[" << x << "] = " << extConns[j].costSums[x]
                      << endl;
            } else {
               mc2log << warn << "Skipping unknown cost[" << x << "]" << endl;
               // Don't know what to do with it yet.
               incReadLong(pos);
            }
         }        
      }
      
      uint32* endNodes = new uint32[nbrNodes];
      for (int j = 0; j < nbrNodes; j++) {
         endNodes[j] = incReadLong(pos);
      }

      // So now we have the external connections in extConns[] and
      // the nodes in endNodes[]. Try to create a SubRoute for every
      // extConn and add the nodes.
      
      // We start with the first node as origin if it isn't a state
      // element.
      uint32 startNode = 0; 
      if ( IS_STATE_ELEMENT( endNodes[0] ) ) {
         vehicle = stateElementToVehicle( driverPref, endNodes[0] );
         // Skip this element later.
         startNode = 1;
      }

      // Seems like routes to an external node and routes to a destination
      // are handled the same. I.e. an external connection is added even if
      // we reach the destination, but this external connection has the
      // same map id as the origin map.
      subRouteVector.reserve(nbrConnections);
      for( int j=0; j < nbrConnections; ++j ) {
         // Create origin and destination
         OrigDestInfo orig(vehicle,
                           mapID,
                           endNodes[startNode],
                           prevSubRouteID,
                           0,  // estCost, hope it works
                           0); // cost,    hope it works
         
         // Dest will have the wrong vehicle if the route contains
         // a state element somewhere else than in the beginning.
         OrigDestInfo dest(vehicle,
                           extConns[j].mapID,
                           extConns[j].nodeID,
                           MAX_UINT32, // subRouteID,
                           extConns[j].cost,
                           extConns[j].estCost,
                           extConns[j].lat,
                           extConns[j].lon,
                           uint16(MAX_UINT16), // angle
                           extConns[j].costSums[0],
                           extConns[j].costSums[1],
                           extConns[j].costSums[2]);

         mc2dbg8 << "Cost[" << 0 << "] = " << dest.getCostASum()
                 << endl;
         mc2dbg8 << "Cost[" << 1 << "] = " << dest.getCostBSum()
                 << endl;
         mc2dbg8 << "Cost[" << 2 << "] = " << dest.getCostCSum()
                 << endl;
         
         // Set offset too.
         dest.setOffset(float(extConns[j].offset) / float(MAX_UINT16));
         // Create the subroute.
         SubRoute* subRoute = new SubRoute(orig, dest);
         subRoute->reserve(nbrNodes);
         int nbrNodesToAdd = nbrNodes;
         if ( subRoute->hasOneMap() ) {
            // Seems like the destination is added twice in the dest-route.
            --nbrNodesToAdd;
         }
         for( int nodeNbr = startNode + 1;
              nodeNbr < nbrNodesToAdd;
              ++nodeNbr) {
            uint32 nodeToAdd = endNodes[nodeNbr];
            subRoute->addNodeID(nodeToAdd);
         }

         mc2dbg8 << "Cost[" << 0 << "] = " << subRoute->getCostASum()
                 << endl;
         mc2dbg8 << "Cost[" << 1 << "] = " << subRoute->getCostBSum()
                 << endl;
         mc2dbg8 << "Cost[" << 2 << "] = " << subRoute->getCostCSum()
                 << endl;
         // Add the SubRoute to the SubRouteVector
         subRouteVector.insertSubRoute(subRoute);
      }
      delete [] endNodes;
      delete [] extConns;
   }
} // getSubRouteList

void
SubRouteReplyPacket::dumpRoutePacket()
{
//     cout << "SubRouteReplyPacket: " << endl;
//     cout << "....................." << endl;
//     uint16 listType = getListType();
//     cout << "listType             " << listType << endl;
//     cout << "routeID              " << getRouteID() << endl;
//     cout << "cutOff               " << getCutOff() << endl;
//     uint32 nbrSubRoutes = getNbrSubRoutes();
//     cout << "nbrSubRoutes         " << nbrSubRoutes << endl;
//     cout << "....................." << endl;

//     int pos = SUBROUTE_REPLY_PACKET_SIZE;
//     for (uint32 j = 0; j < nbrSubRoutes; j++) {
//        cout << "subRoute " << j << ":" << endl; 
//        cout << "subRouteID     " << incReadLong(pos) << endl;
//        cout << "prevSubRouteID " << incReadLong(pos) << endl;
//        cout << "routeComplete  " << BP(incReadByte(pos)) << endl;
//        cout << "forward        " << BP(incReadByte(pos)) << endl;
//        pos +=2;    // padbytes
//        cout << "comesFromMapID " << incReadLong(pos) << endl;
//        uint32 nbrConnections = incReadLong(pos);
//        cout << "nbrConnections " << nbrConnections << endl;
//        uint32 nbrNodes = incReadLong(pos);
//        cout << "nbrNodes       " << nbrNodes << endl;
//        cout << "..............." << endl;
      
//        for (uint32 k = 0; k < nbrConnections; k++) {
//           cout << "connection     " << k << ":" << endl;
//           cout << "mapID          " << incReadLong(pos) << endl;
//           cout << "nodeID         " << hex << "0x" << incReadLong(pos)
//                << dec << endl;
//           cout << "cost           " << incReadLong(pos) << endl;
//           cout << "estimated cost " << incReadLong(pos) << endl;
//        }
//        cout << "..............." << endl;
      
//        for (uint32 l = 0; l < nbrNodes; l++) {
//           uint32 nodeID = incReadLong(pos);
//           cout << "0x" << hex << nodeID << dec; 
//  //           if (listType == SubRouteList::PROXIMITY_REQUEST) {
//  //              cout << ", offset " << incReadShort(pos) << " and cost "
//  //                   << incReadLong(pos) << endl;
//  //           }
//  //         else {
//              cout << endl;
//  //         }
//        }
//   }      
} // dumpRoutePacket

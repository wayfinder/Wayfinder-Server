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

#include "ServerSubRouteVector.h"
#include "RouteSender.h"
#include "OrigDestInfo.h"
#include "SubRoutePacket.h"
#include "Request.h"
#include "LevelTransitObject.h"
#include "GfxUtility.h"
#include "DriverPref.h"
#include "Connection.h"
#include "Properties.h"
#include "RouteTypes.h"

#include "SmallestRoutingCostTable.h"
#include "SmallestRoutingCostPacket.h"
#include "RouteTrafficCostPacket.h"
#include "RequestUserData.h"
#include "UserData.h"

#include "OverviewMap.h"

#include "RoutingInfoPacket.h"
#include "MapBits.h"
#include "NodeBits.h"
#include "Math.h"

#include <algorithm>
#include <set>
#include <sstream>

/* Critera (in meters) to go up one level */
/* Not used. RoutingInfo is used instead. */
//const uint32 UP_CRITERIA[]   = {1000, 2000, MAX_UINT32};

/* Radius (in meters) in which to find maps to go down one level */
//const uint32 DOWN_CRITERIA[] = {MAX_UINT32, 20*1000, 40*1000};

#define RSU "[" << (m_user.getUser() ? m_user.getUser()->getLogonID() : "NA") << "]"

class NewStartSubRoute{
public:
   NewStartSubRoute( RouteSender* thisRouteSender ) {
      m_routeSender = thisRouteSender;
   }
   void operator() ( const OrigDestInfo& insertInfo ) {
      m_routeSender->insertOrigDestInfo( insertInfo );
   }
private:
   RouteSender* m_routeSender;
};

const char*
RouteSender::stateToString( state_t state )
{
   switch (state) {
      case START_ROUTING:
         return "START_ROUTING";
      case TRANSIT_FINDING:
         return "TRANSIT_FINDING";
      case HIGHER_ROUTING:
         return "HIGHER_ROUTING";
      case CUTOFF_END_ROUTING:
         return "CUTOFF_END_ROUTING";
      case CUTOFF_HIGHER_ROUTING:
         return "CUTOFF_HIGHER_ROUTING";
      case END_ROUTING:
         return "END_ROUTING";
      case DONE:
         return "DONE";
   }
   // Should not happen. Compiler complains, though.
   return "QUE";
}

bool
RouteSender::isDestRoute(const SubRoute* subRoute) const
{
   if ( m_destItems.find(IDPair_t(subRoute->getNextMapID(),
                                  subRoute->getDestNodeID())) !=
        m_destItems.end()) {
      return true;
   } else {
      return false;
   }
}

bool
RouteSender::createAndEnqueueSubRouteRequest()
{
   // Route on all SubRoutes on the same map as the best map.
   SubRouteVector requestVector;

   uint32 mapID = m_requestSubRouteContainer.dequeueSubRoutesFromBestMap(
      requestVector, m_levelTransitObject, m_level);

   mc2dbg8 << RSU << "[RS]: Next map to route on = " << mapID << endl;
   
   if ( requestVector.getSize() > 0 ) {

      // Also insert the ones from the m_tooFarSubRouteContainer
      // to the same map. Since we already have decided to route
      // here it will not hurt since we will go to higher level
      // anyway if we visit too many maps.
      // It will hurt if we forget the routes
      // though. So we have to take them sooner or later.
      if ( getCutOff() != MAX_UINT32 ) {
         m_tooFarSubRouteContainer.updateCutOff( getCutOff() );
         m_tooFarSubRouteContainer.updateContainer(
            &m_finishedServerSubRouteVector);
      }

#if 1
      // We will take the subroutes leading to the same map from
      // the toofar container.
      int nbrTooFar =
         m_tooFarSubRouteContainer.dequeueSubRoutesFromMap(requestVector,
                                                           mapID);
      
      mc2dbg2 << RSU << "[RS]: Nbr routes added from too far container = "
             << nbrTooFar << endl;
#endif
      
      // Go through requestVector and insert each in
      // finishedServerSubRouteVector
      mc2dbg2 << RSU << "[RS]: Origs in request: "
             << requestVector.getSize()
             << " dests: "
             << m_destInfoList->size()
             << " cutoff: "
         //<< m_requestSubRouteContainer.getCutOff()
             << m_finishedServerSubRouteVector.getCutOff()
             << endl;
      for ( uint32 i = 0; i < requestVector.getSize(); i++ ) {
         m_finishedServerSubRouteVector.insertSubRoute(
            requestVector.getSubRouteAt( i ) );
      }


            
#if 1
      // ReActivated again.
      // Testing to send the old subroutes too. Will probably be
      // bad when using maps where there are lonely roads that are far
      // outside the rest of the roads in the map.
      
      map<uint32, SubRoute*> finishedSubRoutes;


      // Does not remove the most expensive ones.
      m_finishedServerSubRouteVector.getSubRoutesToMap(mapID,
                                                       finishedSubRoutes);

      mc2dbg2 << RSU << "[RS]: finished size BEFORE " << finishedSubRoutes.size()
             << endl;

      // Remove the ones that are already in the requestvector.
      for(SubRouteVector::iterator it = requestVector.begin();
          it != requestVector.end();
          /**/ ) {
         map<uint32, SubRoute*>::iterator found =
            finishedSubRoutes.find( (*it)->getDestNodeID() );
         // Should always be found since we have inserted them
         // above.
         if ( found != finishedSubRoutes.end() ) {
            // Same in old vector and outgoing vector
            if ( found->second->getCost() >= (*it)->getCost() ) {
               // Finished is more expensive
               finishedSubRoutes.erase((*it)->getDestNodeID());
               ++it;
            } else {
               // The one we are about to send is more expensive
               it = requestVector.erase(it);
            }
         } else {
            // Just in case
            ++it;
         }
      }

      mc2dbg2 << RSU << "[RS]: finished size EFTUR " << finishedSubRoutes.size()
             << endl;

      // Add the finished routes to the list
      for(map<uint32, SubRoute*>::const_iterator it(finishedSubRoutes.begin());
          it != finishedSubRoutes.end();
          ++it ) {
         // Add the route
         requestVector.push_back( it->second );
      }
#endif


      // For statistics and to avoid too many visits.
      m_usedMaps[mapID]++;

      // Set the new vector
      m_nextMapAndSubRouteVector.first = mapID;
      m_nextMapAndSubRouteVector.second.swap( requestVector );

      if ( trafficNeeded( mapID ) ) {
         mc2dbg2 << RSU << "[RS]: Traffic needed for map " << MC2HEX( mapID )
                << endl;
         // Delay the enqueuing of the packet until traffic info has
         // been received.
         prepareGettingTraffic( mapID );
      } else {
         mc2dbg2 << RSU << "[RS]: No traffic needed for map " << MC2HEX( mapID )
                << endl;
         // Ok - enqueue it right away.
         enqueueSubRouteRequest();
      }

      return true;
   } else {
      mc2dbg2 << RSU << "[RS]: No more SubRoutes to route on"
             << endl;
      return false;
   }
}   

bool
RouteSender::trafficNeeded( uint32 mapID ) const
{
   return m_useTrafficIfC && 
      m_mapsWithTrafficInfo.find(mapID) == m_mapsWithTrafficInfo.end();
}

const DisturbanceVector*
RouteSender::getDistVectorForPackets(uint32 mapID) const
{
   return m_distListForPackets->getDisturbances(mapID);
}

bool
RouteSender::enqueueSubRouteRequest()
{
   const uint32 mapID = m_nextMapAndSubRouteVector.first;
   mc2dbg2 << RSU << "[RS]: enqueueSubRouteRequest for mapID "
          << MC2HEX(mapID)
          << endl;


   // Get all disturbances if it is an overview map and let the
   // RouteModule figure out if the low level node is on that map.
   const DisturbanceVector* distsForUserAndMapCoverage =
      getDistVectorForPackets(
         MapBits::isOverviewMap(mapID) ? MAX_UINT32 : mapID);

   // Get only the ones for the correct map when taking the ones
   // from the InfoModule
   const DisturbanceVector* infoModuleDists =
      m_infoModuleDists->getDisturbances( mapID );
   int nifo = infoModuleDists ? infoModuleDists->size() : 0;
   mc2dbg2 << RSU << "[RS]: Added " << nifo << " dists from infomodule" << endl;
      
   SubRouteRequestPacket* pack =
      new SubRouteRequestPacket( m_driverPref,
                                 & (m_nextMapAndSubRouteVector.second),
                                 m_destInfoList,
                                 m_allDestInfoList,
                                 m_isEnd,
                                 m_finishedServerSubRouteVector.getCutOff(),
                                 //m_requestSubRouteContainer.getCutOff(),
                                 0, // Always using RM CalcCost on low level
                                 distsForUserAndMapCoverage,
                                 m_calcCostSums,
                                 m_dontSendSubRoutes,
                                 infoModuleDists );

   // Don't delete the subroutes when the vector is deleted.
   m_nextMapAndSubRouteVector.second.resetAll();
   pack->setRequestID( m_request->getID() );
   pack->setPacketID( m_request->getNextPacketID() );
   PacketContainer* packContainer =
      new PacketContainer( pack, 0, 0, MODULE_TYPE_ROUTE);
   
   m_outgoingQueue.add( packContainer );
   return true;
}

int
RouteSender::calcNbrDests(const OrigDestInfoList* allDestInfoList,
                          const OrigDestInfoList* destInfoList,
                          uint32 nbrBestDests)
{
   if ( nbrBestDests == MAX_UINT32 ) {
      // Old behaviour
      return ((allDestInfoList == NULL) ? 1 : destInfoList->size());
   } else {
      return nbrBestDests;
   }                                     
}

RouteSender::RouteSender(const RequestUserData& user,
                         Request* request,
                         const OrigDestInfoList* origInfoList,
                         const OrigDestInfoList* destInfoList,
                         const OrigDestInfoList* allDestInfoList,
                         const DriverPref* pref,
                         const RoutingInfo* routingInfo,
                         uint32 distFromStartToFinish,
                         const DisturbanceList* disturbances,
                         uint32 nbrBestDests,
                         int level,
                         uint32 externalCutOff /* = MAX_UINT32 */,
                         bool calcCostSums /* = false */,
                         bool dontSendSubRoutes,/* = false */
                         set<uint32>* lowerLevelDestMaps, /* = NULL */
                         RouteAllowedMap* allowedMaps /* = NULL */ )
      : m_requestSubRouteContainer(destInfoList),
      m_tooFarSubRouteContainer(),
      // Create subrouteVector with one
      // destination if allDestInfoList == NULL
      // else use the size of the destInfoList.
      // FIXME: Add calcNbrDests here.
      m_finishedServerSubRouteVector(calcNbrDests(allDestInfoList,
                                                  destInfoList,
                                                  nbrBestDests)),
//        m_finishedServerSubRouteVector(
//           allDestInfoList == NULL ? 1 : destInfoList->size() ),
      m_driverPref(pref),
      m_destInfoList(destInfoList),
      m_nbrOutstanding(0),
      m_outgoingQueue(),
      m_status(StringTable::OK),
      m_request(request),
      m_disturbances(disturbances),
      m_packetCounter( 0 ),
      m_routingInfo(routingInfo),
      m_minDistFromOrigToDest( distFromStartToFinish ),
      m_user( user ),
      m_useTrafficIfC( true )
{
   mc2dbg << RSU << "[RouteSender::RouteSender]: cost (A B C D) = ("
          << uint32(pref->getCostA()) << " "
          << uint32(pref->getCostB()) << " "
          << uint32(pref->getCostC()) << " "
          << uint32(pref->getCostD()) << ")" << endl;
   m_infoModuleDists = new DisturbanceList;
   m_distListForPackets = NULL;
   m_higherLevelRouteSender = NULL;
   m_higherOrigList = NULL;
   m_levelTransitObject = NULL;
   m_allowedMaps = allowedMaps;

   m_requestSubRouteContainer.setAllowedMaps( m_allowedMaps );
   m_tooFarSubRouteContainer.setAllowedMaps( m_allowedMaps );

   m_level = level;
   m_state = START_ROUTING;

   m_finishedServerSubRouteVector.setExternalCutOff( externalCutOff );

   // Create the starting SubRoutes.

   // Check if all origins in allowed maps
   for ( OrigDestInfoList::const_iterator it = origInfoList->begin();
         it != origInfoList->end() ; ++it )
   {
      if ( m_allowedMaps != NULL && m_allowedMaps->find( 
              it->getMapID() ) == m_allowedMaps->end() ) 
      {
         // This origin is not allowed
         if ( m_level == 0 ) {
            m_status = StringTable::ORIGIN_OUTSIDE_ALLOWED_AREA;
            m_state = DONE;
         }
         mc2dbg2 << RSU << "ORIGIN_OUTSIDE_ALLOWED_AREA " << hex << it->getMapID()
                 << "," << it->getNodeID() << dec << endl;
      } else {
         insertOrigDestInfo( *it );
      }
   }
   
   if ( allDestInfoList == NULL ) {
      // Only one route to one of the dests should be returned
      m_allDestInfoList = destInfoList;
      if ( nbrBestDests == MAX_UINT32 || nbrBestDests == 1 )
         m_isEnd = true; // Old way, one destination returned
      else
         m_isEnd = false;
   } else {
      // All routes to the dests should be returned
      m_allDestInfoList = allDestInfoList;
      m_isEnd = false;
   }

   // Calc cost sums if we should calculate the cost to more than
   // one destination. (part of a dist sort)
   // Also don't send subroutes
   m_calcCostSums = calcCostSums;
   m_dontSendSubRoutes = dontSendSubRoutes;
   if ( nbrBestDests != MAX_UINT32 ) {
      m_calcCostSums      = true;
      m_dontSendSubRoutes = true;
   }

   // Initialize the allowedMapsStuff
   initAllowedMaps();
   

   mc2dbg2 << RSU << "[RS]: Leaving RouteSender::RouteSender on level "
           << m_level
           << endl
           << "Size of Container is "
           << m_requestSubRouteContainer.getSize()
           << ", of Vector is "
           << m_finishedServerSubRouteVector.getSize()
           << ", of TooFarContainer is "
           << m_tooFarSubRouteContainer.getSize()
           << endl;

   // Create the set of destinations.
   for(OrigDestInfoList::const_iterator it = m_destInfoList->begin();
       it != m_destInfoList->end();
       ++it ) {
      if ( m_allowedMaps != NULL && m_allowedMaps->find( 
              it->getMapID() ) == m_allowedMaps->end() ) 
      {
         // This destination is not allowed
         if ( m_level == 0 ) {
            m_status = StringTable::DESTINATION_OUTSIDE_ALLOWED_AREA;
            m_state = DONE;
         }
         mc2dbg2 << RSU << "DESTINATION_OUTSIDE_ALLOWED_AREA " << hex 
                 << it->getMapID() << "," << it->getNodeID() << dec 
                 << endl;
      } else {
         m_destItems.insert(IDPair_t(it->getMapID(), it->getNodeID()));
         m_destMaps.insert(it->getMapID());
      }
   }

   // Check if we need to transfer the lowerlevel destmaps.
   if ( lowerLevelDestMaps != NULL ) {
      // We want to use the destinations on low level when
      // we calculate cut off.
      m_lowerLevelDestMaps = *lowerLevelDestMaps;
   } else {
      // We are already on lowest level
      m_lowerLevelDestMaps  = m_destMaps;
   }
   
   // FIXME: Add globally later. With mutexes and so on.
   //        Like in get this from ParserThreadGroup, by keeping it there.
   m_routingCostTable = new SmallestRoutingCostTable;

   m_maxLevel = m_routingInfo->getUpCriteria().size() - 1;

   mc2dbg2 << RSU << "[RS:RS]: m_maxLevel = " << m_maxLevel << endl;
   
   m_minNbrHighLevelNodes =
      Properties::getUint32Property("ROUTE_MIN_NBR_HIGHLEVELNODES", 16);

   createAndEnqueueSubRouteRequest();
}

RouteSender::~RouteSender() 
{
   delete m_infoModuleDists;
   delete m_higherLevelRouteSender;
   delete m_higherOrigList;
   delete m_levelTransitObject;
   delete m_routingCostTable;

   m_nextMapAndSubRouteVector.second.resetAll();

   if ( ! m_subRoutesWaitingForSmallestCost.empty() ) {
      mc2dbg2 << RSU << "[~RS]: Number of Subroutes waiting for costs: "
             << m_subRoutesWaitingForSmallestCost.size() << endl;

      // Delete them. This means that something has gone
      // wrong, but we should not leak memory.
      for( map<uint32, SubRouteVector*>::iterator
           it(m_subRoutesWaitingForSmallestCost.begin());
           it != m_subRoutesWaitingForSmallestCost.end();
           ++it ) {
         delete it->second;
      }      
   }

   delete m_distListForPackets;
}

uint32
RouteSender::calcSmallestCost( uint32 originMap,
                               const set<uint32>& destMaps,
                               const DriverPref& driverPref) const
{
   uint32 minCost = MAX_UINT32;
   for( set<uint32>::const_iterator it = destMaps.begin();
        it != destMaps.end();
        ++it ) {
      const RoutingCosts* rc = m_routingCostTable->getCosts(originMap,
                                                            *it);
      if ( rc == NULL ) {
         // Nothing to do. We don't have it.
         mc2log << error << RSU 
                << "[RS]: Could not find cost in table - wrong" << endl;
         return 0; // EARLY RETURN HERE
      }
      
      // Calculate mincost. Use cost B for cost C
      uint32 routeCostA = Connection::metersToDistCost(rc->getCostA());
      uint32 routeCostB = Connection::secToTimeCost(rc->getCostB());
      uint32 curCost = routeCostA * driverPref.getCostA() +
                       routeCostB * driverPref.getCostB() +
                       routeCostB * driverPref.getCostC();
      minCost = MIN(curCost, minCost);
   }
   return minCost;
}

uint32
RouteSender::calcSmallestCost( const SubRoute* subRoute,
                               const set<uint32>& destMaps,
                               const DriverPref& driverPref) const
{
   // Calc the smallest cost to get from the end of the route to the dest.
   uint32 destMapID = subRoute->getNextMapID();
   uint32 cost = calcSmallestCost( destMapID, destMaps, driverPref);
   return cost;
}

void
RouteSender::processStartRouting( SubRouteVector* srv )
{

   // I hope that this will not copy the vector. 
   SubRouteVector& incomingSubRouteVector = *srv;

   // Don't get the vector from the packet anymore
   // packet->getSubRouteVector( m_driverPref, incomingSubRouteVector );
   
   // Put each incoming routes in one of
   //    a) finishedServerSubRouteVector
   //    b) requestSubRouteContainer
   //    c) tooFarSubRouteContainer
   mc2dbg2 << RSU << "[RS]: SubRoutes in reply: "
          << incomingSubRouteVector.getSize()
          << endl;

   set<uint32> tooFar;
   set<uint32> notTooFar;
   set<uint32> toMaps;
   map<uint32, int> nbrHighLevelNodes;
   map<uint32, int> nbrNodes;
   int totalNbrNodes = 0;
   int totalNbrOnHighLevel = 0;
   int nbrDests = 0;
         
   for ( uint32 subRouteCounter = 0;
         subRouteCounter < incomingSubRouteVector.getSize();
         subRouteCounter++ ) {

      mc2dbg8 << RSU << "[RS]: Got SubRoute to "
              << incomingSubRouteVector[subRouteCounter]->getNextMapID()
              << ":" << hex
              << incomingSubRouteVector[subRouteCounter]->getDestNodeID()
              << dec << endl;
      mc2dbg8 << RSU << "[RS]: Cost is = "
              << incomingSubRouteVector[subRouteCounter]->getCost()
              << endl;
      
      // Keep this
      bool isDest = isDestRoute(incomingSubRouteVector[subRouteCounter]);

      uint32 nextMapID =
         incomingSubRouteVector[subRouteCounter]->getNextMapID();
      toMaps.insert(nextMapID);

      uint32 noD = incomingSubRouteVector[subRouteCounter]->getDestNodeID();
      if ( !OverviewMap::isLowerLevel( noD ) ) {
         ++nbrHighLevelNodes[nextMapID];
         ++totalNbrOnHighLevel;
      }
      ++nbrNodes[nextMapID];
      ++totalNbrNodes;
            
      // It used to say m_level > 0 here, but we don't know
      // if we're done until we have tried all maps.
      if ( isDest ) {
         // Finished routes go into the vector.
         // Also sets the subRouteID of the route.
         m_finishedServerSubRouteVector.insertSubRoute(
            incomingSubRouteVector[subRouteCounter], true);
         // Keep the incomingSubRouteVector from deleting the subroute
         incomingSubRouteVector.resetIndex( subRouteCounter );
         ++nbrDests;
      } else if ( incomingSubRouteVector[subRouteCounter]->hasOneMap() ) {
         // Same origin and destination map - should not continue routing
         // just insert the route.
         mc2dbg8 << RSU << "[RS]: Same orig and destmap" << endl;
         m_finishedServerSubRouteVector.insertSubRoute(
            incomingSubRouteVector[subRouteCounter], false);
         incomingSubRouteVector.resetIndex( subRouteCounter );
      } else {
         // Might need more routing - put in in a container.
         // Container will check duplicates itself.

         // Calculate the distance to the closest destination and
         // determine if we are within criterion away from a dest.

         // FIXME: Make the criteriummap member
         CriteriumMap critMap = m_routingInfo->getUpCriteria();
         uint32 criterion = critMap[m_level];
         uint32 minDist = MAX_UINT32;
         OrigDestInfoList::const_iterator it = m_destInfoList->begin();
         bool allFoundOnThisLevel = true;

         // Only one destination wanted or we already have a cutoff.
         if ( m_isEnd ||
              (m_finishedServerSubRouteVector.getCutOff() != MAX_UINT32) ) {
            // Why must minDist be larger than criterion?
            // Hmm. I see - there is no use in checking if it's less
            // because we only want to see if one destination is inside.
            // But shouldn't we use the smallest value for estimation?
            while ( (true || (minDist > criterion))
                    && (it != m_destInfoList->end()) ) {
               float64 thisDist =
                  ( sqrt( float64( GfxUtility::squareP2Pdistance_linear(
                     incomingSubRouteVector[subRouteCounter]->getDestLat(),
                     incomingSubRouteVector[subRouteCounter]->getDestLon(),
                     it->getLat(),
                     it->getLon() ) ) ) );
               mc2dbg4 << RSU << "[RS]: DestLat(a) = " << it->getLat() << endl
                       << "      DestLon(a) = " << it->getLon() << endl;
               if ( thisDist < MAX_UINT32 ) {
                  if ( (uint32)thisDist < minDist ) {
                     minDist = (uint32)thisDist;
                  }
               }
               ++it;
            }
         } else {
            mc2dbg4 << RSU << "[RS]: Counting" << endl;
            // We want routes to all destinations and all dests are
            // not found yet
            int nbrLowLevel = 0;
            while( it != m_destInfoList->end() ) {
               float64 thisDist =
                  ( sqrt( float64( GfxUtility::squareP2Pdistance_linear(
                     incomingSubRouteVector[subRouteCounter]->getDestLat(),
                     incomingSubRouteVector[subRouteCounter]->getDestLon(),
                     it->getLat(),
                     it->getLon() ) ) ) );
               mc2dbg8 << RSU << "[RS]: DestLat(b) = " << it->getLat() << endl
                       << "      DestLon(b) = " << it->getLon() << endl;
               if ( thisDist < MAX_UINT32 ) {
                  if ( (uint32)thisDist < minDist ) {
                     minDist = (uint32)thisDist;
                  }
               }
               if ( thisDist <= criterion ) {
                  // Low level
                  ++nbrLowLevel;
               }
               ++it;
            }
            // TODO: This could be optimized...
            uint32* destIndexArray =
               m_finishedServerSubRouteVector.getDestIndexArray();
            int indexArraySize = m_finishedServerSubRouteVector.getNbrDest();
            int i = 0;
            for(i=0; i < indexArraySize; ++i ) {
               if ( destIndexArray[i] == MAX_UINT32 ) {
                  // First not valid one. There really should be one or
                  // else the cutoff wouldn't be MAX_UINT32 which we
                  // check before entering this function.
                  break;
               }
            }
            mc2dbg8 << RSU << "[RS]: NbrDests still on this level"
                    << nbrLowLevel << endl;
            mc2dbg8 << RSU << "[RS]: Nbr dests found " << i << endl;
            if ( i < nbrLowLevel )
               allFoundOnThisLevel = false;
         }

         mc2dbg8 << RSU<< "[RS]: MINDIST IS = " << minDist << endl;
         mc2dbg8 << RSU << "[RS]: CRITER  IS = " << criterion << endl;
         
         // costA is distance cost
         uint32 estCostA = Connection::metersToDistCost(minDist);
         // costB is time cost
         // where we should use 110km/h since this is highest speed in
         // Sweden.
         // We use 90 km/h instead, to get bigger costs
         double timeSec = minDist / ( Math::KMH_TO_MS * 90.0 );
         uint32 estCostB   = uint32( Connection::secToTimeCost(timeSec) );

         // Checking if coordinates are OK for higher level..
         if ( m_level > 0 ) {
            //mc2dbg2 << RSU << "[RS]: Setting mindist to zero" << endl;
            //minDist = costA = costB = 0;
         }

         uint32 estCostSum = estCostA * m_driverPref->getCostA() +
                estCostB * m_driverPref->getCostB() +
                estCostB * m_driverPref->getCostC();
         

         // Check the precalculated smallest cost too.
         uint32 smallCostSum = calcSmallestCost(
            incomingSubRouteVector[subRouteCounter], m_lowerLevelDestMaps,
            *m_driverPref);

         mc2dbg8 << RSU << "[RS]: The crow flies " << estCostSum
                 << " and smallCostSum flies "
                 << smallCostSum << "[" << subRouteCounter << "]" << endl;
         // Use the maximum
         uint32 curEstCost = MAX(smallCostSum, estCostSum);

         if ( curEstCost == smallCostSum ) {
            mc2dbg4 << RSU << "[RS]: Using curEstCost = " << curEstCost 
                    << endl;
            mc2dbg4 << RSU << "[RS]: Map id          = "
                    << incomingSubRouteVector[subRouteCounter]->getNextMapID()
                    << endl;
         } else {
            mc2dbg4 << RSU << "[RS]: Using crow       = " << curEstCost 
                    << endl;
            mc2dbg4 << RSU << "[RS]: smallCostSum was = " << smallCostSum 
                    << endl;
            mc2dbg4 << RSU << "[RS]: Map id           = "
                    << incomingSubRouteVector[subRouteCounter]->getNextMapID()
                    << endl;
         }

         incomingSubRouteVector[subRouteCounter]->setEstimatedCost(
            incomingSubRouteVector[subRouteCounter]->getCost() +
            curEstCost);

         mc2dbg8 << RSU << "[RS]: Set estimated cost to "
                 << incomingSubRouteVector[subRouteCounter]->getEstimatedCost()
                 << endl;
         
         // If we are too far away, insert in tooFarSRC
         // otherwise in requestSRC
         if ( minDist > criterion ) {
            tooFar.insert(subRouteCounter);
         } else {
            notTooFar.insert(subRouteCounter);
         }
      }
   }

   // If we have visited the destination map more than twice
   // we will go to higher level.

   const int maxVisitsOnDestMap = 2;
   uint32 curDestMap = MAX_UINT32;
   bool okToInsertAllInTooFar = false;
   
   if ( m_level < m_maxLevel ) {
      for( set<uint32>::const_iterator it = m_destMaps.begin();
           it != m_destMaps.end();
           ++it ) {
         map<uint32, int>::const_iterator search = m_usedMaps.find( *it );
         if ( search != m_usedMaps.end()
              && search->second > maxVisitsOnDestMap) {
            curDestMap = *it;
            okToInsertAllInTooFar = true;         
            break;
         }
      }
   }
   
#if 0
   if ( m_tooFarSubRouteContainer.getSize() >
        m_requestSubRouteContainer.getSize() ) {
      mc2dbg2 << RSU << "[RS]: Lots of routes in toofar. Going to higher "
             << "m_requestSubRouteContainer.getSize() = "
             << m_requestSubRouteContainer.getSize()
             << ", m_tooFarSubRouteContainer.getSize() = "
             << m_tooFarSubRouteContainer.getSize() << endl; 
      okToInsertAllInTooFar = true;
      m_requestSubRouteContainer.putAllInto( m_tooFarSubRouteContainer,
                                             &m_finishedServerSubRouteVector);
   }
#endif
   CriteriumMap critMap = m_routingInfo->getUpCriteria();
   uint32 criterion = critMap[m_level];

   if ( m_level < m_maxLevel ) {
      if ( (uint32)m_minDistFromOrigToDest > criterion ) {
         mc2dbg2 << RSU << "[RS]: Dist from orig to dest is "
                << m_minDistFromOrigToDest << " and crit is "
                << criterion << " - going to higher level " << endl;
         okToInsertAllInTooFar = true;
         m_requestSubRouteContainer.
            putAllInto( m_tooFarSubRouteContainer,
                        &m_finishedServerSubRouteVector);
      }
   }
   
   if ( okToInsertAllInTooFar && ( m_level < m_maxLevel ) ) {
      mc2dbg2 << RSU << "[RS]: Destination map " << curDestMap
             << " visited more than " << maxVisitsOnDestMap << " times"
             << " going to higher level" << endl;
      m_requestSubRouteContainer.putAllInto( m_tooFarSubRouteContainer,
                                             &m_finishedServerSubRouteVector);
      
   }
   

   int nbrKept = 0;
   // Moved the insertion here. The destination are already inserted.
   // FIXME: Iterate over the sets instead. 
   for( uint32 subRouteCounter = 0;
        subRouteCounter < incomingSubRouteVector.getSize();
        ++subRouteCounter ) {
      if ( incomingSubRouteVector[subRouteCounter] == NULL ) {
         // Destinations are already taken care of.
         continue;
      }
      
      const uint32 nextMapID =
         incomingSubRouteVector[subRouteCounter]->getNextMapID();

      if ( nbrHighLevelNodes.find(nextMapID) == nbrHighLevelNodes.end()) {
         nbrHighLevelNodes[nextMapID] = 0;
      }

      // WARNING: Can be used on all levels but the highest.
      // Avoid routing on maps that has too many external connections      
      const bool okToInsertInTooFar = okToInsertAllInTooFar;
            
      if ( okToInsertInTooFar ||
           tooFar.find( subRouteCounter ) != tooFar.end() ) {
         if ( incomingSubRouteVector[subRouteCounter]->size() > 0 ) {
            // Moved the setting of id:s to moveAllSubRoutes
            // since we need the correct stuff when we send routes
            // from too far on low level.
         } else {
            mc2dbg8 << RSU << "[RS]: Incoming is too small" << endl;
         }
         mc2dbg8 << RSU << "[RS]: Size of incoming is "
                 << incomingSubRouteVector[subRouteCounter]->size()
                 << endl;
         // There is something strange when checking using the
         // finishedServerSubRouteContainer.
         if ( m_tooFarSubRouteContainer.checkAndInsertSubRoute(
            incomingSubRouteVector[subRouteCounter],
            &m_finishedServerSubRouteVector ) ) {
            ++nbrKept;
         }
         //NULL);         
      } else if ( notTooFar.find ( subRouteCounter ) != notTooFar.end() ) {
         mc2dbg8 << RSU << "[RS]: Will call checkandinsert" << endl;
         if ( m_requestSubRouteContainer.checkAndInsertSubRoute(
            incomingSubRouteVector[subRouteCounter],
            &m_finishedServerSubRouteVector ) ) {
            ++nbrKept;
         }
      } else {
         delete incomingSubRouteVector[subRouteCounter];
      }
      // Keep the incoming vector from deleting the SubRoute.
      incomingSubRouteVector.resetIndex( subRouteCounter );
   }

   mc2dbg2 << RSU << "[RS]: Nbr of routes inserted " << nbrKept << endl;
   mc2dbg2 << RSU << "[RS]: Nbr of dests " << nbrDests << endl;
   
   uint32 finishedCutOff = m_finishedServerSubRouteVector.getCutOff();
   
   // Set new cutoff if it changed to a smaller value.
   m_requestSubRouteContainer.updateCutOff( finishedCutOff );
   // Remove SubRoutes with cost > cutoff.
   m_requestSubRouteContainer.updateContainer(&m_finishedServerSubRouteVector);

   // Check that there are enough routes to the higher level
   if ( m_requestSubRouteContainer.empty() &&
        ! m_tooFarSubRouteContainer.empty() &&
        finishedCutOff == MAX_UINT32 ) {
      // Have to decide if there are enough routes that lead to the
      // higher level.
      int nbrOnHigherLevel =
         m_tooFarSubRouteContainer.countNodesOnLevel(m_level + 1);
      int nbrOnHigherLevelInVector =
         m_finishedServerSubRouteVector.countNodesOnLevel(m_level + 1);
      int nbrTotal = m_tooFarSubRouteContainer.getSize();
      mc2dbg2 << RSU << "[RS]: m_tooFar contains " << nbrOnHigherLevel
             << " of " << nbrTotal << " nodes on level "
             << (m_level+1) << endl;
      mc2dbg2 << RSU << "[RS]: m_finished contains " << nbrOnHigherLevelInVector
             << " of " << m_finishedServerSubRouteVector.getSize()
             << " nodes on level "
             << (m_level+1) << endl;

      // Move back the outstanding routes to lower level.
      if ( nbrTotal != 0 && nbrOnHigherLevel < m_minNbrHighLevelNodes ) {
         m_tooFarSubRouteContainer.
            copyAllInto( m_requestSubRouteContainer,
                         &m_finishedServerSubRouteVector);
         mc2dbg2 << RSU << "[RS]: Not enough high level nodes total = "
                << nbrTotal << ", nbrOnHigherLevel = " << nbrOnHigherLevel
                << ", m_minNbrHighLevelNodes = " << m_minNbrHighLevelNodes
                << endl;
      } else {
         mc2dbg2 << RSU << "[RS]: Enough high level nodes" << endl;
      }
   }  

   
   // Keep routing if there are subroutes in request container,
   // otherwise move on to the next step
   if ( m_requestSubRouteContainer.getSize() > 0 ) {
      createAndEnqueueSubRouteRequest();
      
   } else if ( m_nbrOutstanding == 0 ) {
      // Check and use cutoff in too far container.           
      m_tooFarSubRouteContainer.updateCutOff(
         m_finishedServerSubRouteVector.getCutOff() );

      mc2dbg2 << RSU << "[RS]: m_finishedServerSubRouteVector has cutoff = "
             << m_finishedServerSubRouteVector.getCutOff() << endl;

      mc2dbg2 << RSU << "[RS]: m_tooFarSubRouteContainer.size() == "
             << m_tooFarSubRouteContainer.getSize() << endl;
      m_tooFarSubRouteContainer.updateContainer(
         &m_finishedServerSubRouteVector);
      mc2dbg2 << RSU << "[RS]: m_tooFarSubRouteContainer.size() == "
              << m_tooFarSubRouteContainer.getSize() << endl;
      
      // If there are still SubRoutes to route on, we have to prepare
      // to create a child RouteSender, m_higherLevelRouteSender.
      // Otherwise routing on this level is finished.
      // Used to test for no cutoff too, but there could be routes
      // in the toofar subroute container that weren't used on low level
      // but should be.
      if ( m_tooFarSubRouteContainer.getSize() > 0 ) {
         
         // Move all the SubRoutes that remains routing on.
         SubRouteVector requestVector;
         m_tooFarSubRouteContainer.
            moveAllSubRoutes( requestVector,
                              m_finishedServerSubRouteVector);

         // Go through requestVector and insert each
         // in finishedServerSubRouteVector.
         m_higherOrigList = new OrigDestInfoList;

         for ( uint32 i = 0; i < requestVector.getSize(); ++i ) {
            m_finishedServerSubRouteVector.insertSubRoute(
               requestVector.getSubRouteAt(i) );
            m_higherOrigList->addOrigDestInfo(
               *( requestVector.getSubRouteAt(i)->getDestInfo() ) );
         }
         requestVector.resetAll();

         // The LevelTransitObject keeps track of origs and dests on
         // this level and on the higher level.
         // In this state all we need is the dests to route to
         // on the higher level.
         // The driverPrefs are sent so that the RouteModule can
         // remove the ones it thinks are unreachable.
         mc2dbg2 << RSU << "[RS]: Will use leveltransit" << endl;
         m_levelTransitObject = new LevelTransitObject( m_request,
                                                        m_higherOrigList,
                                                        m_destInfoList,
                                                        m_level + 1,
                                                        m_driverPref);

         // Get as many packets as possinble from levelTransitObject,
         // then move on to the next state.
         PacketContainer* pc = m_levelTransitObject->getNextPacket();
         while ( pc != NULL ) {
            m_outgoingQueue.add( pc );
            pc = m_levelTransitObject->getNextPacket( );
         }
         m_state = TRANSIT_FINDING;

      } else {
         m_state = DONE;
      }
   }
   // packetContainer is deleted in processPacket.
}
   
void
RouteSender::processTransitFinding( const PacketContainer* p )
{
   // First send the packet on to the LevelTransitObject
   m_levelTransitObject->processPacket( p );

   // If LevelTransitObject has finished, move on to higher level.
   // Else keep sending packets

   if ( m_levelTransitObject->requestDone() == true ) {
      if ( m_levelTransitObject->getStatus() == StringTable::OK ) {
         if ( m_levelTransitObject->getHigherLevelDestList()->size() > 0){
            m_higherLevelRouteSender =
               new RouteSender( m_user,
                                m_request,
                                m_levelTransitObject->getHigherLevelOrigList(),
                                m_levelTransitObject->getHigherLevelDestList(),
                                m_allDestInfoList, // ??? higherleveldest?
                                m_driverPref,
                                m_routingInfo,
                                m_minDistFromOrigToDest,
                                m_disturbances,
                                MAX_UINT32,
                                m_level + 1,
                                m_finishedServerSubRouteVector.getCutOff(),
                                m_calcCostSums,       // Transfer this 
                                m_dontSendSubRoutes,  // Transfer this 
                                &m_lowerLevelDestMaps,// For SmallestCost
                                m_allowedMaps ); // For allowed routing
            
            // Get as many packets as possible from RouteSender,
            // then move on to the next state.
            PacketContainer* pc = m_higherLevelRouteSender->getNextPacket();
            while ( pc != NULL ) {
               m_outgoingQueue.add( pc );
               pc = m_higherLevelRouteSender->getNextPacket( );
            }
            
            m_state = HIGHER_ROUTING;
         } else {
            m_state = DONE;
         }
      } else {
         m_status = m_levelTransitObject->getStatus();
         m_state = DONE;
      }
   } else {
      // Get new request packets from the LevelTransitObject         
      PacketContainer* pc = m_levelTransitObject->getNextPacket();
      while (pc != NULL) {
         m_outgoingQueue.add( pc );
         pc = m_levelTransitObject->getNextPacket( );
      }
   }
}   

uint32
RouteSender::getCutOff() const
{
   // FIXME: What happens if there is an external cutoff?
   return m_finishedServerSubRouteVector.getCutOff();
}

void
RouteSender::processHigherRouting( const PacketContainer* p )
{

   // First send the packet on to the LevelTransitObject
   m_higherLevelRouteSender->processPacket( p );
   
   // If LevelTransitObject has finished, move on to higher level.
   // Else keep sending packets

   // Check state and if the higherlevel sender has a cutoff.
   // If we are in HIGHER_ROUTING we want to check the
   // route to the destination if there is a cutoff in the
   // higher level. This means that we can get a new, better
   // cutoff. Hopefully we can stop routing on higher level
   // directly after setting the new cutoff that we will get
   // on lower level.

   // Be careful! What happens if there are oustanding packets?


   // Get higher vector
   ServerSubRouteVector& higherVector =
      m_higherLevelRouteSender->m_finishedServerSubRouteVector;

   // Find out if the dests are found.
   bool destFoundOnHigherLevel = 
      ( higherVector.getDestIndexArray()[higherVector.getNbrDest()-1]
      != MAX_UINT32 );

   bool oneDestFoundOnHigherLevel =
      ( higherVector.getDestIndexArray()[0]
      != MAX_UINT32 );

   mc2dbg8 << RSU << "[RS]: destFoundOnHigherLevel = " << destFoundOnHigherLevel
           << endl; 
   mc2dbg8 << RSU << "[RS]: oneDestFoundOnHigherLevel = " 
           << oneDestFoundOnHigherLevel << endl; 
   
   bool stopAndSmellTheRoses = (m_state == HIGHER_ROUTING ) &&
      ( m_higherLevelRouteSender->m_state == START_ROUTING ) &&
      /* ( m_higherLevelRouteSender->m_nbrOutstanding == 0 ) && */
      /* ( m_higherLevelRouteSender->getCutOff() != MAX_UINT32) && */
      /* destFoundOnHigherLevel */
      oneDestFoundOnHigherLevel;

   //stopAndSmellTheRoses = false;
   
   if ( stopAndSmellTheRoses ) {
      mc2dbg2 << RSU << "[RS]: Will route on lower \"too early\"" << endl;
   }
   
   if ( m_higherLevelRouteSender->requestDone() == true ||
        // Testing to read the route when there is a cutoff.
        // FIXME: Go to END_ROUTING and then back to higher routing
        stopAndSmellTheRoses ) {
      if ( m_higherLevelRouteSender->getStatus() == StringTable::OK ) {
         ServerSubRouteVectorVector* resultVector =
            m_higherLevelRouteSender->getRoute();
         
         mc2dbg2 << RSU << "[RS]: Size of result " << resultVector->size() << endl;

         for ( uint32 indexCounter = 0;
               indexCounter < resultVector->size();
               ++indexCounter ) {
            
            // Moving all the SubRoutes from higher level to this level.
            // Presumably the RouteModule returns a subroute
            // with the last node on this level.
            SubRoute* containerSubRoute =
               m_finishedServerSubRouteVector.merge(
                  (*resultVector)[indexCounter] );
            (*resultVector)[indexCounter]->resetAll();
            if ( containerSubRoute != NULL ) {
               OrigDestInfo* tmpInfo = m_levelTransitObject->getLowerLevelDest(
                  containerSubRoute->getDestNodeID(),
                  containerSubRoute->getNextMapID() );
               if ( tmpInfo == NULL ) {
                  mc2dbg2 << RSU << "[RS]: Higherlevel destnode = "
                         << containerSubRoute->getNextMapID()
                         << ":" << hex << containerSubRoute->getDestNodeID()
                         << dec << endl;
                  delete containerSubRoute;
                  continue; // To avoid crashing. Does not work
               }
               // Set costs
               // FIXME: Copy containerSubRoute->getDestInfo och
               //        change destitem and destmap instead.
               OrigDestInfo info(*tmpInfo);
               info.setCost( containerSubRoute->getCost() );
               info.setEstimatedCost( containerSubRoute->getEstimatedCost() );
               // Set costsums              
               info.setCostASum( containerSubRoute->getCostASum() );
               info.setCostBSum( containerSubRoute->getCostBSum() );
               info.setCostCSum( containerSubRoute->getCostCSum() );
               info.setVehicle( containerSubRoute->
                                getDestInfo()->getVehicle());
               mc2dbg8 << RSU << "[RS]: RouteSender costs (0)"
                       <<  info.getCostASum() << ':'
                       <<  info.getCostBSum() << ':'
                       <<  info.getCostCSum() << endl;
               // Get the higher level id:s before setting new dest.
               OrigDestInfo higherOrig(info);
               higherOrig.setID( containerSubRoute->getDestID() );
               // Change the destination into lower level.
               containerSubRoute->setDestInfo( info );               
               m_requestSubRouteContainer.insertSubRoute( containerSubRoute );

               // Check if this is a destination.
               if ( m_destItems.find( info.getID() ) !=
                    m_destItems.end() ) {
                  // FIXME: Is it possible to have  just the
                  // copy as destination instead?
                  // Make a copy which we can refer to for the short route.
                  SubRoute* routeCopy = new SubRoute( *containerSubRoute);
                  // Insert SubRoute to get ID.
                  m_finishedServerSubRouteVector.insertSubRoute( routeCopy,
                                                                 true );
                  // When the above is tested and works the following
                  // can be removed / 2002-12-02
                  // Try to insert the subroute as finished so that we can
                  // go straight down if needed.
                  //info.setPrevSubRouteID( routeCopy->getSubRouteID() );
                  //m_finishedServerSubRouteVector.insertSubRoute(
                  //   new SubRoute(info, info), true);
                  mc2dbg2 << RSU << "[RS]: Inserting vertical route" << endl;
               } else {
                  mc2dbg8 << RSU << "[RS]: Copy is not a destination" << endl;
               }
            }
         }
         delete resultVector;
         createAndEnqueueSubRouteRequest();
         if ( m_state == HIGHER_ROUTING && stopAndSmellTheRoses ) {            
            m_state = CUTOFF_END_ROUTING;
            // Speed hack. Should really be CUTOFF_END_ROUTING, but
            // there are packets remaining in the higherlevel sender
            // that should be checked for cutoff.
            //m_state = END_ROUTING;
         } else {
            m_state = END_ROUTING;
         }
      } else {
         m_status = m_higherLevelRouteSender->getStatus();
         m_state = DONE;
      }
   } else {
      // Get new request packets from the RouteSender         
      PacketContainer* pc = m_higherLevelRouteSender->getNextPacket();
      while (pc != NULL) {
         m_outgoingQueue.add( pc );
         pc = m_higherLevelRouteSender->getNextPacket( );
      }
   }
}
   
void
RouteSender::processEndRouting( SubRouteReplyPacket* packet )
{

   SubRouteVector incomingSubRouteVector;
   packet->getSubRouteVector( m_driverPref, incomingSubRouteVector );

   mc2dbg2 << RSU << "[RS]: Size of incomingSubRouteVector: "
          << incomingSubRouteVector.getSize()
          << endl;

   int nbrDests = 0;
   // Put each incoming routes in one of
   //    a) finishedServerSubRouteVector
   //    b) requestSubRouteContainer
   for ( uint32 subRouteCounter = 0;
         subRouteCounter < incomingSubRouteVector.getSize();
         subRouteCounter++ ) {

      bool isDest =
         isDestRoute(incomingSubRouteVector[subRouteCounter]);

      if ( isDest ) {
         // Finished routes go into the vector.
         // Also sets the subRouteID of the route.
         
         ++nbrDests;
         
         m_finishedServerSubRouteVector.insertSubRoute(
            incomingSubRouteVector[subRouteCounter], true);
         
      } else {
         // Might need more routing - put in in the container.
         // Container will check duplicates itself.
         m_requestSubRouteContainer.checkAndInsertSubRoute(
            incomingSubRouteVector[subRouteCounter],
            &m_finishedServerSubRouteVector );
         
      }
      // Keep the incoming vector from deleting the SubRoute.
      incomingSubRouteVector.resetIndex( subRouteCounter );
      
   }

   mc2dbg2 << RSU << "[RS]: " << stateToString(m_state)
          << ": " << nbrDests << " destinations"
          << endl;

   
   // Set new cutoff if it changed to a smaller value.
   m_requestSubRouteContainer.updateCutOff(
      m_finishedServerSubRouteVector.getCutOff() );
   // Remove SubRoutes with cost > cutoff.
   m_requestSubRouteContainer.updateContainer(&m_finishedServerSubRouteVector);            

   // FIXME: Add option to createAndEnqueueSubRouteRequest instead
   //m_isEnd = true;
   
   // Keep routing if there are subroutes in request container,
   // otherwise move on to the next step
   if ( m_requestSubRouteContainer.getSize() > 0 ) {
      if ( !createAndEnqueueSubRouteRequest() &&
           m_nbrOutstanding == 0 ) {
         if ( m_state == END_ROUTING ) {
            m_state = DONE;
         } else {
            m_state = CUTOFF_HIGHER_ROUTING;
         }
      }
   } else if ( m_nbrOutstanding == 0 ) {
      if ( m_state == END_ROUTING ) {
         m_state = DONE;
      } else {
         m_state = CUTOFF_HIGHER_ROUTING;
      }
   }
   // packetContainer is deleted in processPacket.

   if ( m_state == CUTOFF_HIGHER_ROUTING ) {
      mc2dbg2 << RSU << "[RS]: Setting cutoff to " << getCutOff() 
             << " was " << m_higherLevelRouteSender->getCutOff()
             << endl;
      m_higherLevelRouteSender->
         m_finishedServerSubRouteVector.setExternalCutOff( getCutOff());

      m_requestSubRouteContainer.updateCutOff(getCutOff());
      m_requestSubRouteContainer.updateContainer(&m_finishedServerSubRouteVector);
      
      // INFO: This will almost work. The highlevel sender has already
      //       decided where to route. We should divide the function
      //       for handling subroutepackets into two.
      //       One for receiving and one for checking stuff and removing
      //       routes once the cutoff is set.
      
      // Get new request packets from the RouteSender         
      PacketContainer* pc = m_higherLevelRouteSender->getNextPacket();
      int nbrPacks = 0;
      while (pc != NULL) {
         ++nbrPacks;
         if ( pc->getPacket()->getSubType() ==
              Packet::PACKETTYPE_SUBROUTEREQUEST ) {
            // Change the cutoff to the new value.
            // 
            SubRouteRequestPacket* p =
               static_cast<SubRouteRequestPacket*>(pc->getPacket());
            p->setCutOff(getCutOff());
         }
         m_outgoingQueue.add( pc );
         pc = m_higherLevelRouteSender->getNextPacket( );
      }
      mc2dbg2 << RSU << "[RS]: Higherlevel route sender had "
             << nbrPacks << " packets in queueue" << endl;
         
   }
   
}

SubRouteVector*
RouteSender::checkCostsAvailable( SubRouteReplyPacket* packet,
                                  MapPairVector& neededMaps)
{
   SubRouteVector* incomingSubRouteVector = new SubRouteVector;
   packet->getSubRouteVector( m_driverPref, *incomingSubRouteVector);
   int size = incomingSubRouteVector->getSize();

   // Insert all the fromMaps in the set
   set<uint32> fromMaps;
   for( int i = 0; i < size; ++i ) {
      fromMaps.insert((*incomingSubRouteVector)[i]->getNextMapID());
   }

   // The to-maps are in m_lowerLevelDestMaps
   for( set<uint32>::const_iterator to = m_lowerLevelDestMaps.begin();
        to != m_lowerLevelDestMaps.end();
        ++to ) {
      for( set<uint32>::const_iterator from = fromMaps.begin();
           from != fromMaps.end();
           ++from) {
         if ( m_routingCostTable->getCosts(*from, *to) == NULL ) {
            mc2dbg2 << RSU << "[RS]: Need cost from " << *from << " to "
                   << *to << endl;
            //cout << "M " << *from << " " << *to << endl;
            neededMaps.push_back(MapPair(*from, *to));
         }
      }
   }
   return incomingSubRouteVector;
}

bool
RouteSender::prepareGettingRoutingCosts( MapPairVector& neededMaps,
                                         SubRouteVector* srv)
{
   mc2dbg2 << RSU << "[RS]: Will create SmallestRoutingCostPacket" << endl;
   // Create the packet
   SmallestRoutingCostRequestPacket* smrcp =
      new SmallestRoutingCostRequestPacket(neededMaps, ++m_packetCounter);
   
   // Register the SubRouteVector
   m_subRoutesWaitingForSmallestCost[m_packetCounter] = srv;

   // Set ID:s
   smrcp->setRequestID( m_request->getID() );
   smrcp->setPacketID ( m_request->getNextPacketID() );
   
   // Make container
   PacketContainer* pc = new PacketContainer(smrcp,
                                             0,
                                             0,
                                             MODULE_TYPE_MAP);
   // Add it to the queueue.
   m_outgoingQueue.add(pc);
   
   return true;
}

void
RouteSender::prepareGettingTraffic( uint32 mapID )
{   

   MC2_ASSERT( m_mapsWithTrafficInfo.end() ==
               m_mapsWithTrafficInfo.find( mapID ) );
   
   mc2dbg2 << RSU << "[RS]: Will create RouteTrafficCostPacket" << endl;

      
   RouteTrafficCostRequestPacket* rtcrp =
      new RouteTrafficCostRequestPacket( m_user.getUser(),
                                         mapID, // map id
                                         ++m_packetCounter );
   
   // Set ID:s
   rtcrp->setRequestID( m_request->getID() );
   rtcrp->setPacketID ( m_request->getNextPacketID() );
   
   // Make container
   PacketContainer* pc = new PacketContainer( rtcrp,
                                              0,
                                              0,
                                              MODULE_TYPE_TRAFFIC,
                                              1000,
                                              1 );
   
   // Add a packet to use if the InfoModule times out
   static const vector<pair<uint32,uint32> > no_dists;
   pc->putTimeoutPacket(
      new RouteTrafficCostReplyPacket( rtcrp, StringTable::TIMEOUT_ERROR,
                                       no_dists ) );
   // Add it to the queueue.
   m_outgoingQueue.add(pc);

   mc2dbg2 << RSU << "[RS]: Traffic packet for map "
          << MC2HEX( mapID ) << " enqueued" << endl;
}

void
RouteSender::processTrafficReplyPacket( const Packet* packet )
{
   const RouteTrafficCostReplyPacket* rtcrp =
      static_cast<const RouteTrafficCostReplyPacket*>(packet);
   
   mc2dbg2 << RSU << "[RS]: processTrafficReply, mapID = "
          << MC2HEX( rtcrp->getMapID() ) << endl;
   
   const uint32 mapID = rtcrp->getMapID();
   
   if ( rtcrp->getStatus() == StringTable::TIMEOUT_ERROR ) {
      mc2log << warn << RSU 
             << "[RS]: Looks like the INFOMODULE didn't respond in time "
             << " - no more traffic "
             << "(mapID= " << MC2HEX( rtcrp->getMapID() ) << ")" << endl;
      m_useTrafficIfC = false;
   }

   MC2_ASSERT( mapID == m_nextMapAndSubRouteVector.first );

   // Never request info on this map again.
   m_mapsWithTrafficInfo.insert( mapID );
   
   // FIXME: Apply the disturbances too.
   DisturbanceVector tmpDists;
   rtcrp->getTraffic( tmpDists, m_driverPref->getCostC() );
   mc2dbg2 << RSU << "[RS]: " << tmpDists.size() 
           << " dists read from traffic packet" << endl;

   // Add the disturbances to our list of info module disturbances.
   m_infoModuleDists->takeDisturbances( tmpDists, mapID );
   MC2_ASSERT( tmpDists.empty() );
   
   // Enqueue the SubRoutePacket now
   enqueueSubRouteRequest();
   
}

SubRouteVector*
RouteSender::processSmallestRoutingCostReply( Packet* packet )
{
   mc2dbg2 << RSU << "[RS]: SmallestRoutingCostReplyPacket received " << endl;
   SmallestRoutingCostReplyPacket* reply =
      static_cast<SmallestRoutingCostReplyPacket*>(packet);

   // Extract the routing costs.
   SmallestRoutingCostReplyPacket::costMap_t costsOfPacket;

   reply->getRoutingCosts(costsOfPacket);
   for( SmallestRoutingCostReplyPacket::costMap_t::const_iterator it =
           costsOfPacket.begin();
        it != costsOfPacket.end();
        ++it ) {
      uint32 fromMapID = it->first.first;
      uint32 toMapID   = it->first.second;
      const RoutingCosts& costs = it->second;

      mc2dbg2 << RSU << "[RS]: SmallestRoutingCost from map "
             << fromMapID << " to " << toMapID
             << " are " << costs.getCostA() << ", "
             << costs.getCostB() << endl;

      // Put the costs in the table.
      m_routingCostTable->setCosts(fromMapID, toMapID,
                                   costs);                                   
   }

   // Get the SubRouteVector from the storage.
   // Should work. Otherwise we are toast anyway. Can't route without
   // SubRouteVectors.
   SubRouteVector* retVal =
      m_subRoutesWaitingForSmallestCost[reply->getUserDefinedData()];

   // Remove it so we can delete what's left in the destructor
   // if we do not get here every time (MM is dead).
   m_subRoutesWaitingForSmallestCost.erase(reply->getUserDefinedData());
   
   return retVal;
}

void
RouteSender::processPacket( const PacketContainer* p)
{
   if ( p == NULL ) {
      // PacketContainer == NULL indicates a timeout.
      m_status = StringTable::TIMEOUT_ERROR;
      return;
   }
   
   m_nbrOutstanding--;

   // Check the state, and distribute the packet accordingly
   switch ( m_state ) {

      case START_ROUTING: {
         mc2dbg2 << RSU << "[RS]: In START_ROUTING level = "
                << m_level
                << endl;
         // Packet has to be SUBROUTEREPLY or SMALLESTROUTINGCOST
         // or ROUTETRAFFICCOSTREPLY
         Packet* packet = p->getPacket();
         if ( packet->getSubType() == Packet::PACKETTYPE_SUBROUTEREPLY ) {
            SubRouteReplyPacket* subRoutePacket =
               static_cast<SubRouteReplyPacket*>(packet);
            // Set the status of the RouteSender if
            // the packet we got is not OK.
            if ( subRoutePacket->getStatus() != StringTable::OK ) {
               m_status =
                  StringTable::stringCode( subRoutePacket->getStatus() );
            } else {
               // Extract the needed maps to get costs from.
               MapPairVector neededMaps;
               SubRouteVector* srv =
                  checkCostsAvailable( subRoutePacket, neededMaps );
               if ( neededMaps.empty() ) {
                  // No maps needed - stay in this state.
                  processStartRouting( srv );
                  // srv has been used now.
                  delete srv;
               } else {
                  // This means that we stay in the state too.
                  prepareGettingRoutingCosts( neededMaps, srv );
               }
            }
         } else if ( packet->getSubType() ==
                     Packet::PACKETTYPE_SMALLESTROUTINGCOSTREPLY ) {
            // Handle smallest routing cost reply and get the right vector
            SubRouteVector* srv = processSmallestRoutingCostReply(packet);
            // Process the SubRouteVector.
            processStartRouting(srv);
            // Also delete the vector.
            delete srv;
         } else if ( packet->getSubType() ==
                     Packet::PACKETTYPE_ROUTETRAFFICCOSTREPLY ) {
            // Handle route traffic cost packet.
            // Means that the delayed SubRouteRequestPacket will
            // be created and sent.
            processTrafficReplyPacket( packet );
         } else {
            mc2log << error << RSU 
                   << "[RS]: In state 'START_ROUTING' RouteSender"
                      " got packet of type "
                   << packet->getSubTypeAsString()
                   << endl;
            // We cannot recover from this.
            m_status = StringTable::INTERNAL_SERVER_ERROR;
         }
         break;
      }
      case TRANSIT_FINDING:
         mc2dbg2 << RSU << "[RS]: In TRANSIT_FINDING level = " << m_level
                << endl;
         // LevelTransitObject takes care of checking packet type
         processTransitFinding( p );
         break;
      case HIGHER_ROUTING:
      case CUTOFF_HIGHER_ROUTING:
         mc2dbg2 << RSU << "[RS]: In " << stateToString(m_state)
                << " level = " << m_level
                << endl;
         // child RouteSender takes care of checking packet type
         processHigherRouting( p );         
         break;
      case END_ROUTING:
      case CUTOFF_END_ROUTING:
      {         
         mc2dbg2 << RSU << "[RS]: " << stateToString(m_state)
                << " level = " << m_level
                << endl;
         // Packet has to be SUBROUTEREPLY OR ROUTETRAFFICCOSTREPLYPACKET
         Packet* packet = p->getPacket();
         if ( packet->getSubType() == Packet::PACKETTYPE_SUBROUTEREPLY ) {
            SubRouteReplyPacket* subRoutePacket =
               static_cast<SubRouteReplyPacket*>(packet);
            // Set the status of the RouteSender if the packet we
            // got is not OK.
            if ( subRoutePacket->getStatus() != StringTable::OK ) {
               m_status =
                  StringTable::stringCode( subRoutePacket->getStatus() );
            } else {
               processEndRouting( subRoutePacket );
            }
         } else if ( packet->getSubType() ==
                     Packet::PACKETTYPE_ROUTETRAFFICCOSTREPLY ) {
            // Handle route traffic cost packet.
            // Means that the delayed SubRouteRequestPacket will
            // be created and sent.
            processTrafficReplyPacket( packet );
         } else {
            mc2log << error << RSU 
                   << "[RS]: In state 'END_ROUTING' RouteSender "
                      "got packet of type "
                   << packet->getSubTypeAsString()
                   << endl;
            // We cannot recover from this.
            m_status = StringTable::INTERNAL_SERVER_ERROR;
         }
         break;
      }
      default:
         // I.e we are in DONE state
         // No packets should arrive here
         mc2log << error << RSU 
                << "[RS]: In state 'DONE' RouteSender got packet of type "
                << p->getPacket()->getSubTypeAsString()
                << endl;
         // We cannot recover from this.
         m_status = StringTable::INTERNAL_SERVER_ERROR;
         break;
   }      
   
   mc2dbg2 << RSU << "[RS]: Leaving RouteSender::processPacket on level "
           << m_level
           << endl
           << "Size of Container is "
           << m_requestSubRouteContainer.getSize()
           << ", of Vector is "
           << m_finishedServerSubRouteVector.getSize()
           << ", of TooFarContainer is "
           << m_tooFarSubRouteContainer.getSize()
           << endl;
   // The packet p is deleted in the Request, i.e. RouteRequest in
   // this case.
}


PacketContainer*
RouteSender::getNextPacket()
{
   if (m_outgoingQueue.getMin() == NULL) {
      return NULL;
   } else {
      m_nbrOutstanding++;
      return m_outgoingQueue.extractMin();
   }
}

ServerSubRouteVectorVector*
RouteSender::getRoute()
{
   ServerSubRouteVectorVector* returnVector = new ServerSubRouteVectorVector;

   // Make room for the routes.
   returnVector->reserve( m_finishedServerSubRouteVector.getNbrDest() );

   // Get the finished routes.
   for ( uint32 indexCounter = 0;
         indexCounter < m_finishedServerSubRouteVector.getNbrDest();
         indexCounter++ ) {

      ServerSubRouteVector* v =
         m_finishedServerSubRouteVector.getResultVector( indexCounter );

      mc2dbg8 << RSU << "[RS]: Size of SSRV is "
              << ((v != NULL) ? v->getSize() : 0) << endl;
      
      if ( (v != NULL) && (v->size() != 0) ) {
         returnVector->push_back(v);
      } else {
         mc2dbg2 << RSU << "[RS]: Missing route in vector." << endl;
         delete v;
      }
         
   }

   // Print visited maps.
   if ( true ) {
      mc2dbg << RSU << "[RS]: Visited Maps : ";
      
      int nbrVisits = 0;
      for(map<uint32, int>::const_iterator it = m_usedMaps.begin();
          it != m_usedMaps.end();
          ++it ) {
         mc2dbg << " " << MC2HEX(it->first) << "(" << it->second << ")";
         nbrVisits += it->second;
      }
      mc2dbg << " : Totally " << nbrVisits << " visits";
   
      // Print destination maps
      mc2dbg << " : Destination maps: ";
      for( set<uint32>::const_iterator it( m_destMaps.begin() );
           it != m_destMaps.end();
           ++it ) {
         mc2dbg << MC2HEX(*it) << ' ';
      }
      mc2dbg << endl;
   }
   
   return returnVector;
}

ServerSubRouteVectorVector*
RouteSender::getSortedRoute()
{
   ServerSubRouteVectorVector* returnVector = getRoute();
   // Sort the routes. Cheapest first.
   if ( returnVector->size() > 1 ) 
      sort(returnVector->begin(), returnVector->end(),
           SubRouteVector::compareCosts);
   return returnVector;
}


void
RouteSender::initAllowedMaps()
{
   // FIXME: Add original disturbances and delete this
   DisturbanceList* distList = new DisturbanceList;

   // Copy the old disturbances.
   if ( m_disturbances != NULL ) {
      const DisturbanceVector* vect = m_disturbances->getDisturbances();
      for( DisturbanceVector::const_iterator it = vect->begin();
           it != vect->end();
           ++it ) {
         distList->addDisturbance(new DisturbanceListElement(**it));
      }
   }

   // Add blocked stuff for all maps.
   if ( m_allowedMaps != NULL ) {
      // Add blocked disturbances
      for ( RouteAllowedMap::const_iterator blockedMap(m_allowedMaps->begin());
            blockedMap != m_allowedMaps->end();
            ++blockedMap ) {
         uint32 mapID = blockedMap->first;
         for ( vector< uint32 >::const_iterator it =
                  blockedMap->second.begin() ;
               it != blockedMap->second.end() ; ++it ) {
            // avoidNode should be used.
            distList->avoidNode( mapID, *it );
            // Other node too, might not be needed but jic
            uint32 otherNode = *it;
            if ( MapBits::isNode0( *it ) ) {
               // Set high bit
               otherNode = (otherNode | 0x80000000);
            } else {
               // Unset high bit
               otherNode = (otherNode & 0x7FFFFFFF);
            }
            // avoidNode should be used.
            distList->avoidNode( mapID, otherNode );           
         }                  
      }
   }
   m_distListForPackets = distList;
}

/*
#ifdef undef_level_1
#   undef DEBUG_LEVEL_1
#endif
#ifdef undef_level_2
#   undef DEBUG_LEVEL_2
#endif
#ifdef undef_level_4
#   undef DEBUG_LEVEL_4
#endif
#ifdef undef_level_8
#   undef DEBUG_LEVEL_8
#endif
*/

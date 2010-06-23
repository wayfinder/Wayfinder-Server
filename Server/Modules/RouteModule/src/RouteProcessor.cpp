/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RouteProcessor.h"
#include "RoutePacket.h"
#include "StringTable.h"
#include "CCSet.h"
#include "OrigDestNodes.h"
#include "RouteConstants.h"
#include "SubRouteListTypes.h"
#include "SubRouteList.h"

// Some packets
#include "EdgeNodesPacket.h"
#include "IDTranslationPacket.h"
#include "DisturbancePushPacket.h"
#include "RMSubRoutePacket.h"
#include "UpdateTrafficCostPacket.h"
#include "SystemPackets.h"

#include <vector>
#include "OrigDestInfo.h"
#include "DisturbanceList.h"
#include "NewStrDup.h"
#include "STLUtility.h"

#include "CalcRoute.h"

RouteProcessor::RouteProcessor(MapSafeVector* loadedMaps,
                               const char* packetFile)
      : MapHandlingProcessor(loadedMaps)
{
   m_calcRouteVector = new CalcRouteVector;
   if ( packetFile == NULL ) {
      m_packetFileName = NULL;
   } else {
      m_packetFileName = NewStrDup::newStrDup(packetFile);
   }
}


RouteProcessor::~RouteProcessor()
{
   STLUtility::deleteValues ( *m_calcRouteVector );
   delete m_calcRouteVector;
}


int
RouteProcessor::getCurrentStatus()
{
   return 0;
}


void
RouteProcessor::handleMapNotFound(uint32 mapID, const RequestPacket* p)
{
   mc2dbg2 << "mapID " <<  mapID << " is not loaded." << endl;
   mc2dbg2 << "The packet that caused this output was of type "
        << p->getSubTypeAsString() << endl;
   mc2dbg2 << "The maps in this module are " << endl;
   
   CalcRoute* tempCalc = NULL;
   for (uint32 i = 0; i < m_calcRouteVector->size(); i++) {
      tempCalc = (*m_calcRouteVector)[ i ];
      if (mapID == tempCalc->getMapID()) {
         mc2dbg2 << "Hm, this is map" << tempCalc->getMapID() << endl;
      }
      mc2dbg2 << "#" << i << ":\t" << tempCalc->getMapID() << endl;
   }
   mc2dbg2 << "<end of list>" << endl;
}

/////////////////////////////////////////////////////////////////////
// Helper functions
/////////////////////////////////////////////////////////////////////
uint32
RouteProcessor::getCalcRouteIndex( uint32 mapID ) const
{
   for (uint32 i = 0; i < m_calcRouteVector->size(); i++) {
      CalcRoute* tempCalc = (*m_calcRouteVector)[ i ];
      if (mapID == tempCalc->getMapID())
         return i;      
   }
   // Not found.
   return MAX_UINT32;
}

CalcRoute* 
RouteProcessor::getCalcRoute( uint32 mapID ) const
{
   CalcRoute* tempCalc = NULL;
   for (uint32 i = 0; i < m_calcRouteVector->size(); i++) {
      tempCalc = (*m_calcRouteVector)[ i ];
      if (mapID == tempCalc->getMapID())
         return tempCalc;      
   }
   return NULL;
}

void
RouteProcessor::updateIndices(CalcRoute* calc,
                              Head* nodeList)
{
   RoutingMap* routingMap = calc->getMap();
   OrigDestNode* tempNode = static_cast<OrigDestNode*>(nodeList->first());
   while (tempNode != NULL) {
      uint32 index = routingMap->binarySearch(tempNode->getItemID());
      tempNode->setIndex(index);
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }
}

void
RouteProcessor::updateUturns(const RMDriverPref* driverParam,
                             Head* origin)
{
   // NB It is presumed that both node 0 and node 1 of an item
   // is included in this list!!
   OrigDestNode* tempNode = static_cast<OrigDestNode*>(origin->first());
   while( tempNode != NULL ) {
      int zoomLevel = GET_ZOOM_LEVEL(tempNode->getItemID());
#if 0
      int turnCostFactor = 0;
      switch( zoomLevel ) {
         case 0 :
            turnCostFactor = RouteConstants::TURN_COST_FACTOR_LEVEL_0;
         break;
         case 1 : 
            turnCostFactor = RouteConstants::TURN_COST_FACTOR_LEVEL_1;
         break;
         case 2 : 
            turnCostFactor = RouteConstants::TURN_COST_FACTOR_LEVEL_2;
         break;
         case 3 : 
            turnCostFactor = RouteConstants::TURN_COST_FACTOR_LEVEL_3;
         break;
         default : 
            turnCostFactor = RouteConstants::TURN_COST_FACTOR_DEFAULT;
         break;
      }
      
      uint32 turnCost =
         Connection::secToTimeCost(
            RouteConstants::TURN_COST_SEC * turnCostFactor);
#else
      uint32 turnCostSeconds = Connection::STANDSTILLTIME_U_TURN_MINOR_ROAD;
      if ( zoomLevel <= 2 ) {
         turnCostSeconds = Connection::STANDSTILLTIME_U_TURN_MAJOR_ROAD;
      }
      uint32 turnCost = Connection::secToTimeCost( turnCostSeconds );
#endif

      // Multiply by the things sent as costs.
      turnCost = turnCost * driverParam->getCostA() +
         turnCost * driverParam->getCostB() +
         turnCost * driverParam->getCostC() +
         turnCost * driverParam->getCostD();


      
      if ( (tempNode->getStartDirection()) &&
           (!IS_NODE0(tempNode->getItemID())) ){
         // Driving towards node 1, penalize if node 1
         uint32 oldCost = tempNode->getTurnCost();
         tempNode->setTurnCost( turnCost );
         mc2dbg/*4*/ << "Setting starting-u-turncost in node1 " 
                << MC2HEX( tempNode->getItemID() ) 
                << " to " << turnCost << " (" << turnCostSeconds << "s)"
                << " old cost " << oldCost<< endl;
      } else if ( (!tempNode->getStartDirection()) &&
                  (IS_NODE0(tempNode->getItemID()) ))  {
         // driving towards node 0, penalize if node 0
         uint32 oldCost = tempNode->getTurnCost();
         tempNode->setTurnCost( turnCost );
         mc2dbg/*4*/ << "Setting starting-u-turncost in node0 "
                << MC2HEX( tempNode->getItemID() )
                << " to " << turnCost << " (" << turnCostSeconds << "s)"
                << " old cost " << oldCost << endl;
      }
         
      tempNode = static_cast<OrigDestNode*>( tempNode->suc() ); 
   }
} // updateUTurns


inline void
RouteProcessor::copySubroutesToOrigin(SubRouteList* subList,
                                      Head* origin,
                                      RoutingMap* routingMap)
{
   // Add all origins from subList
   if (subList->getNbrSubRoutes() == 0) {
      mc2dbg << "subList->getNbrSubRoutes(1) == 0" << endl;
   }

   for (uint32 i = 0; i < subList->getNbrSubRoutes(); i++) {
      RMSubRoute* sub = subList->getSubRoute(i);
      for (uint32 j = 0; j < sub->getNbrConnections(); j++) {
         
         uint32 mapID;
         uint32 nodeID;
         uint32 cost;
         uint32 estimatedCost;
         uint16 offset;
         int32 lat,lon;
         uint32 costASum;
         uint32 costBSum;
         uint32 costCSum;

         sub->getExternal(j, mapID, nodeID, cost, estimatedCost,
                          lat, lon, costASum, costBSum, costCSum);

         mc2dbg8 << "RP: copySubRoutesToOrigin: From packet costs "
                 << costASum << ','
                 << costBSum << ','
                 << costCSum << endl;

         
         if (routingMap->getMapID() == mapID) {
            RoutingNode* routingNode =
               routingMap->getNodeFromTrueNodeNumber(nodeID);
            if ( routingNode != NULL ) {
               // XXX Check this
               if (IS_NODE0(nodeID)) {
                  offset = 0;
               }
               else {
                  offset = 0; // Should always be zero
               }

               mc2dbg8 << "[RP] - new OrigDestNode with lat = "
                       << routingNode->getLat() << " and lon = "
                       << routingNode->getLong() << endl;
               mc2dbg8 << "[RP] - server thinks         lat = "
                       << lat << " and lon = " << lon << endl;

               
               
               OrigDestNode* node 
                  = routingMap->newOrigDestNode(routingNode->getIndex(),
                                                mapID,
                                                offset,
                                                routingNode->getLat(),
                                                routingNode->getLong(),
                                                cost,
                                                estimatedCost,
                                                costASum,
                                                costBSum,
                                                costCSum);
               node->setItemID(nodeID); 
               node->into(origin);
            } else {
               mc2log << error
                      << "RP: Strange node " << hex
                      << nodeID << dec << " not fould in map "
                      << mapID << endl;
            }
         }
      }
   }
} // copySubRoutesToOrigin


void
RouteProcessor::copySubroutes(SubRouteList* subList,
                              Head* origin,
                              Head* dest,
                              RoutingMap* map)
{
   DEBUG4( cerr << "RouteProcessor::copySubroutes" << endl );

   uint32 mapID, nodeID, cost, estimatedCost;
   uint16 offset;
   OrigDestNode* node;
   if( subList->getNbrSubRoutes() == 0 )
      cerr << "subList->getNbrSubRoutes(2) == 0" << endl;

   for( uint32 i=0; i < subList->getNbrSubRoutes(); i++){
      RMSubRoute* sub = subList->getSubRoute(i);
      
      for( uint32 j=0; j<sub->getNbrConnections(); j++){
         int32 lat, lon;
         uint32 costASum;
         uint32 costBSum;
         uint32 costCSum;

         sub->getExternal( j, mapID, nodeID, cost, estimatedCost, lat, lon,
                           costASum, costBSum, costCSum);

         mc2dbg8 << "RP: copySubRoutes: From packet costs "
                 << costASum << ','
                 << costBSum << ','
                 << costCSum << endl;
         
         if( ( GET_UINT32_MSB( nodeID ) ) != 0 )
            offset = 0;
         else
            offset = 0;
         // Node id should probably mean index???
         node = map->newOrigDestNode( nodeID,
                                      mapID,
                                      offset,
                                      0,
                                      0,
                                      cost,
                                      estimatedCost,
                                      costASum,
                                      costBSum,
                                      costCSum);
         if(sub->isForward() ) {
            node->into(origin);
         } else {
            node->into(dest);
         }
      }
   }
   if( origin->cardinal() == 0 ){
      cerr << "origin->cardinal() == 0" << endl;
   }
}

RMSubRouteReplyPacket*
RouteProcessor::getSubRouteReplyPacket(const RMSubRouteRequestPacket*
                                       requestPacket,
                                       SubRouteList* resultList,
                                       uint32 status)
{
   DEBUG4(cerr << "RouteProcessor::getSubRouteReplyPacket" << endl);
   
   RMSubRouteReplyPacket* replyPacket = NULL;
   if (status == StringTable::OK) {
      // Everything is OK, so far.
      DEBUG4(cerr << "status == StringTable::OK" << endl);
      uint32 totSize = resultList->getTotalSize();
      replyPacket = new RMSubRouteReplyPacket(requestPacket,
                                              totSize,
                                              status);
      replyPacket->addSubRouteList(resultList);
   }
   else {
      // Something went wrong when routing. ListType sent to leader will be
      // NOT_VALID, and reply will be sent immediately to server when this
      // reaches leader.
      DEBUG1(cerr << "status != StringTable::OK, status is  " <<
              StringTable::getString((StringTable::stringCode)status,
                                     StringTable::ENGLISH)
              << endl);
      replyPacket = new RMSubRouteReplyPacket(requestPacket,
                                              MAX_PACKET_SIZE,
                                              status);
      // XXX MAX_PACKET_SIZE is probably unnecessary.
      replyPacket->setRouteID(requestPacket->getRouteID());
      DEBUG1(cout << "replyPacket routeID " << replyPacket->getRouteID()
              << endl);
   }
   
   return replyPacket;
} // getSubRouteReplyPacket

/////////////////////////////////////////////////////////////////
// Packet processing functions
/////////////////////////////////////////////////////////////////

StringTable::stringCode
RouteProcessor::loadMap(uint32 mapID,
                        uint32& mapSize)
{
   mc2dbg << "LOADING MAP" << endl;
   mapSize = 1; // For now at least.
   CalcRoute* calc = getCalcRoute(mapID);
   if (calc == NULL) {
      char debugString[1024];
      sprintf(debugString, "[RP] Trying to load map %08x", mapID);
      MC2INFO(debugString);
      RoutingMap* newMap = new RoutingMap(mapID);
      uint32 startTime = TimeUtility::getCurrentTime();
      if ( !newMap->load(mapSize, &getLoadedMapsVector()) ) {
         mc2log << error << "RouteProcessor::loadMap - failed to load map"
                << endl;
         delete newMap;
         return StringTable::ERROR_LOADING_MAP;
      } else {
         // The map now belongs to the CalcRoute and will be deleted
         // with it when calc is deleted.
         calc = new CalcRoute(newMap);
         m_calcRouteVector->push_back(calc);
         uint32 endTime = TimeUtility::getCurrentTime();
         float seconds = ( float(endTime) - float(startTime)) / 1000.0;
         sprintf(debugString, "[RP] Map %08x loaded in %.2f seconds",
                 mapID, seconds);
         mc2log << info << debugString << endl;
         return StringTable::OK;
      }
   } else {
      // Map already loaded
      char debugString[1024];
      sprintf(debugString, "[RP] Map %08x is already loaded", mapID);
      mc2log << info << debugString << endl;
      return StringTable::ERROR_MAP_LOADED;
   }
}


StringTable::stringCode
RouteProcessor::deleteMap(uint32 mapID)
{
   uint32 calcRouteIndex = getCalcRouteIndex( mapID );
   CalcRoute* calc = getCalcRoute( mapID );
   StringTable::stringCode status = StringTable::OK;
   if (calc == NULL) {
      MC2WARNING("Map not present in this RouteModule!");
      return StringTable::MAPNOTFOUND;
   } else {
      char debugString[1024];
      sprintf(debugString, "Deleting map %08x at index %d in vector",
              mapID, calcRouteIndex);
      MC2INFO(debugString);
      status = StringTable::OK;
      m_calcRouteVector->erase( m_calcRouteVector->begin() + calcRouteIndex );
      delete calc;
      return StringTable::OK;
   }
}


inline UpdateTrafficCostReplyPacket*
RouteProcessor::processUpdateTrafficCostRequestPacket(
   const UpdateTrafficCostRequestPacket* updateCostsPacket)
{
   mc2dbg2 << "RouteProcessor::processUpdateTrafficCostRequestPacket"
           << endl;
   
   // Temporary trick
   UpdateTrafficCostReplyPacket* reply = 
         new UpdateTrafficCostReplyPacket( updateCostsPacket );
   reply->setStatus(StringTable::OK);
   return reply;
   
   UpdateTrafficCostReplyPacket* replyPacket = NULL;

   if (updateCostsPacket != NULL) {
      
      uint32 mapID = updateCostsPacket->getMapID();
      
      DEBUG8(updateCostsPacket->dump());         
      mc2dbg << "Updating map " << mapID << endl;

      // Get the CalcRoute for the map.
      CalcRoute* calcRoute = getCalcRoute(mapID);
      StringTable::stringCode status = StringTable::OK;
      
      if (calcRoute != NULL) {
         RoutingMap* mapToUpdate = calcRoute->getMap();
            
         uint32 nbrNodesToUpdate = updateCostsPacket->getNbrCosts();
         for (uint32 j = 0; j < nbrNodesToUpdate; j++) {
            
            uint32 nodeID = MAX_UINT32;
            uint32 cost   = MAX_UINT32;
            
            if (updateCostsPacket->getCost(nodeID, cost, j)) {
               
               RoutingNode* node =
                  mapToUpdate->getNodeFromTrueNodeNumber(nodeID);
               
               if (node != NULL) {
                  RoutingConnection* connection =
                     node->getFirstConnection();
                  
                  while (connection != NULL) {
                     RoutingConnectionData* connData =
                        connection->getData();
//                       RoutingNode* toNode =
//                          mapToUpdate->getNode(connection->getIndex());
                     RoutingNode* toNode = connection->getNode();
                     if ( toNode == NULL ) {
                        mc2dbg << "Coulnd't find connection from "
                               << hex << nodeID << " to node with index "
                               << connection << endl;
                        continue;
                     }
                     uint32 costC = connData->getCostB(0) + cost;
                     mapToUpdate->changeConnectionCosts(
                        node,
                        connection,
                        false,
                        connData->getCostA(0),
                        connData->getCostB(0),
                        costC,
                        connData->getVehicleRestriction(false));
                     connection = connection->getNext();
                  }
               }
               else {
                  MC2WARNING("Node not found");
                  cerr << "map " << mapID << "   nodeID: " << hex << nodeID
                       << dec << " (" << nodeID << ")" << endl;
                  status = StringTable::NOTFOUND;
               }
            }
            else {
               cerr << "Could not get cost " << j << endl;
            }
         }
      }
      else {
         handleMapNotFound(mapID, updateCostsPacket);
         status = StringTable::MAPNOTFOUND;
      }
 
      replyPacket = new UpdateTrafficCostReplyPacket(updateCostsPacket);
      replyPacket->setStatus(status);
   } // end if (calcRoute != NULL)
      
   return replyPacket;
} // updateTrafficCostRequestPacket


inline RMSubRouteReplyPacket*
RouteProcessor::processSubRouteRequestPacket(const RMSubRouteRequestPacket*
                                             subRouteRequestPacket)
{
   RMSubRouteReplyPacket* replyPacket = NULL;
   
   if (subRouteRequestPacket != NULL) {
      
      CalcRoute* calc = getCalcRoute(subRouteRequestPacket->getMapID());
      RoutingMap* curMap = calc->getMap();
      
      // Extract data from the packet
      RMDriverPref* driverPref   = new RMDriverPref;
      Head* origin               = new Head;
      Head* dest                 = new Head;
      Head* allDest              = new Head;         
      SubRouteList* incomingList = new SubRouteList;
      bool originalRequest = subRouteRequestPacket->isOriginalRequest();
      
      subRouteRequestPacket->getDriverPreferences(driverPref);
      subRouteRequestPacket->getOrigins(curMap, origin);
      subRouteRequestPacket->getDestinations(curMap, dest);
      subRouteRequestPacket->getAllDestinations(curMap, allDest);

      DisturbanceVector disturbances;
      subRouteRequestPacket->getSubRouteList(incomingList, &disturbances);

      mc2dbg << "Size of disturbancevector " << disturbances.size() << endl;
      
      mc2dbg << "routeID " << incomingList->getRouteID() << endl;
      mc2dbg << "DriverPrefs : " << *driverPref << endl;


      
      SubRouteList* resultList = new SubRouteList;
      resultList->copyListHeader(*incomingList);
      
      uint32 status;               
      if (calc != NULL) {
         // Always update the destination indices          
         if (incomingList->getListType() == SubRouteListTypes::HIGHER_LEVEL) {
            copySubroutes(incomingList, origin, dest, calc->getMap());
         } else {
            updateIndices(calc, allDest);
            updateIndices(calc, dest);
            
            mc2dbg << "nbrDest    " << dest->cardinal() << endl;
            mc2dbg << "nbrAllDest " << allDest->cardinal() << endl;
            
            if (originalRequest) {
               if (driverPref->useUturn()) {
                  updateUturns(driverPref, origin);
               }
               // Only the first time
               updateIndices(calc, origin);
            } else {
               // XXX This is tricky...
               if (incomingList->getListType() ==
                   SubRouteListTypes::HIGHER_LEVEL_BACKWARD) {
                  copySubroutesToOrigin(incomingList,
                                        dest,
                                        calc->getMap());
               } else {
                  copySubroutesToOrigin(incomingList,
                                        origin,
                                        calc->getMap());
               }
            }
         }
         mc2dbg << "nbrOrigins " << origin->cardinal() << endl;
         // Do the actual routing
         status = calc->route(origin,
                              dest,
                              allDest,
                              incomingList,
                              resultList,
                              driverPref,
                              originalRequest,
                              subRouteRequestPacket->getRouteToAll(),
                              &disturbances,
                              subRouteRequestPacket->getCalcSums(),
                              !subRouteRequestPacket->getDontSendSubRoutes());
         
      }
      else {
         char debugString[1024];
         sprintf(debugString,
                 "This module has not loaded map %u (yet)",
                 subRouteRequestPacket->getMapID());
         mc2log << warn << debugString << endl;
         status = StringTable::MAPNOTFOUND;
         handleMapNotFound(subRouteRequestPacket->getMapID(),
                           subRouteRequestPacket);
      }
      
      replyPacket = getSubRouteReplyPacket(subRouteRequestPacket,
                                           resultList,
                                           status);
      
      // Delete all allocated memory.
      incomingList->deleteAllSubRoutes();      
      delete incomingList;
      resultList->deleteAllSubRoutes();
      delete resultList;
      delete origin;
      delete dest;
      delete allDest;
      delete driverPref;

      int nbrDist = disturbances.size();
      for(int i=0; i < nbrDist; ++i ) {
         delete disturbances[i];
      }
   }
   
   return replyPacket;
} // processSubRouteRequestPacket

EdgeNodesReplyPacket*
RouteProcessor::processEdgeNodesRequestPacket(const EdgeNodesRequestPacket*
                                              reqPacket)
{
   // Get some data from the packet.
   uint32 mapID = reqPacket->getMapID();
   int level = reqPacket->getLevel();

   CalcRoute* calc = getCalcRoute(mapID);
   if ( calc == NULL ) {
      // We don't have the map for the packet (yet/anymore?).
      return
         new EdgeNodesReplyPacket(reqPacket, StringTable::MAPNOTFOUND);
   }

   // Now everything should be ok.
   // Get the external nodes at level level
   vector<uint32> extNodes;
   vector<int32>  extLats;
   vector<int32>  extLons;
   OrigDestInfoList destinations;

   reqPacket->getDestinations(destinations);
   
   RMDriverPref dpref;
   dpref.setVehicleRestriction(reqPacket->getVehicle());

   map<uint32,uint32> distances;
   calc->getExternalNodesToLevel(level, extNodes, extLats, extLons,
                                 distances,
                                 destinations,
                                 dpref);
   // Get the overview map on level level.
   uint32 higherLevelMapID = calc->getHigherLevelMapID(level);
   // Get the total number of edgenodes on the level of this map.
   uint32 totalNumberExternal = calc->getMap()->getNumExternalNodes();
   // Get the neighbour maps for this map on the level of this map.
   set<uint32> neighbourMapIDs;
   calc->getNeighbourMapIDs(neighbourMapIDs);

   // Create the reply.
   EdgeNodesReplyPacket* reply =
      new EdgeNodesReplyPacket(reqPacket,
                               StringTable::OK,
                               higherLevelMapID,
                               neighbourMapIDs,
                               extNodes,
                               extLats,
                               extLons,
                               distances,
                               totalNumberExternal);
   
   return reply;
   // The request is deleted outside
}

inline IDTranslationReplyPacket*
RouteProcessor::
processIDTranslationRequestPacket(const IDTranslationRequestPacket* p)
{
   uint32 mapID = p->getMapID();

   CalcRoute* calc = getCalcRoute(mapID);
   if ( calc == NULL ) {
      // Map wasn't found
      mc2log << warn  << "processIDTranslationRequestPacket mapID " 
             << prettyMapID( mapID ) << " not loaded" << endl;
      return new IDTranslationReplyPacket(p,
                                             StringTable::MAPNOTFOUND);
                                             
   }
   
   IDPairVector_t innodes;
   IDPairVector_t outnodes;
   p->getAllNodes(innodes);

   if ( p->getTranslateToLower() ) {
      calc->lookupNodesHigherToLower(outnodes, innodes);
   } else {
      calc->lookupNodesLowerToHigher(outnodes, innodes);
   }
   return new IDTranslationReplyPacket(p, StringTable::OK,
                                          outnodes);
}

inline int
RouteProcessor::processDisturbancePushPacket(const DisturbancePushPacket* dp)
{
   mc2dbg << "PROCESSING DISTURBANCE_PUSH_PACKET" << endl;
   CalcRoute* calc = getCalcRoute(dp->getMapID());
   if ( calc == NULL ) {
      mc2dbg << "[RP]: Got disturbance push on map "
             << dp->getMapID() << " but I cannot find it" << endl;
      return 0;
   }

   // I don't think this will change
   uint32 mapID = dp->getMapID();
   
   // Variables to put the result into.
   bool remove = false;
   bool removeAll = false;
   map<uint32, DisturbanceElement*> distMap;
   dp->getDisturbances(distMap, remove, removeAll);

   if( removeAll ) {
      mc2dbg << "REMOVE ALL" << endl;
      RoutingMap* theMap = calc->getMap();
      theMap->rollBack(false);
   }
   int nbrDist = 0;
   // Do stuff with the disturbances on the map
   for( map<uint32, DisturbanceElement*>::iterator it(distMap.begin());
        it != distMap.end();
        ++it) {
      ++nbrDist;
      // Get the disturbance element for convenience
      DisturbanceElement* curEl = it->second;
      if ( it == distMap.begin() ) {
         if ( ! remove ) {
            mc2dbg << "[RP]: Adding disturbance \""
                   << curEl->getText() << '"' << endl;
         } else {
            mc2dbg << "[RP]: Removing disturbance \""
                   << curEl->getText() << '"' << endl;
         }
         mc2dbg << "[RP]: Disturbance ID " << curEl->getDisturbanceID()
                << endl;
      }
      
      // Check mapID
      uint32 curMapID = curEl->getMapID();
      if ( curMapID != mapID ) {
         mc2dbg << "[RP]: Blaargh new map id " << curMapID
                << " when packet said " << mapID << endl;
         CalcRoute* curCalc = getCalcRoute(curMapID);
         if ( curCalc != NULL ) {
            mapID = curMapID;
            calc  = curCalc;
         } else {
            mc2dbg << "[RP]: Could not find map " << curMapID << endl;
            continue;
         }
      }

      RoutingMap* mapToUpdate = calc->getMap();
      
      map<uint32, uint32> nodeID = curEl->getNodeID();      
      vector<uint32> indexVector = curEl->getRouteIndex();
      uint32 imFactor = curEl->getCostFactor();
      vector<uint32>::iterator it;
      map<uint32, uint32>::iterator mapIt;
      for(it = indexVector.begin(); it != indexVector.end(); it++) {
         uint32 routeIndex = *it;
         mapIt = nodeID.find(routeIndex);
         uint32 currentNodeID = mapIt->second;
         RoutingNode* node =
            mapToUpdate->getNodeFromTrueNodeNumber(currentNodeID);
         
         if ( imFactor != 1000 ) {
            mc2dbg << "[RProc]: factor in packet is "
                   << imFactor << endl;
         }
                 
         if ( node != NULL ) {
            // See if something needs to be done
            if ( ( !remove ) && ( imFactor != 0 ) ) {
               // Update the cost
               mapToUpdate->multiplyNodeCost( node, imFactor );
            } else if ( remove ) {
               mapToUpdate->restoreConnections( node );
            }
         } else {
            char* debstr = new char[1024];
            const char* action = remove ? "remove" : "add";
            sprintf(debstr, "[RProc]: Cannot %s factor = %d for node 0x%x "
                    "- not found in map %d\n",
                    action, imFactor, currentNodeID, curMapID);
            mc2log << error << debstr;
            delete [] debstr;
         }
      }
   }
   
   // Delete the disturbances
   for( multimap<uint32, DisturbanceElement*>::iterator it(distMap.begin());
        it != distMap.end();
        ++it) {
      // The disturbance is in second.
      delete it->second;
   }
   return nbrDist;
}

////////////////////////////////////////////////////////////////////
// Debug
////////////////////////////////////////////////////////////////////

void
RouteProcessor::printDebugData(bool original, Head* origin, Head* dest)
{
   if (original) {
      OrigDestNode* tempNode =
         static_cast<OrigDestNode*>(origin->first());
      cerr << "====== ORIGIN ======" << endl;
      uint32 lastItemID = MAX_UINT32;
      uint32 itemID = MAX_UINT32;
      while ((tempNode != NULL) &&
             ((itemID = tempNode->getItemID()) != lastItemID)) {
         itemID = tempNode->getItemID();
         
         cerr << tempNode-> getMapID() << ":" << hex 
              << itemID << " (" << dec << itemID 
              << ") with offset " << tempNode->getOffset() 
              <<  ", turnCost " << tempNode->getTurnCost() << endl;
         lastItemID = itemID;
         tempNode = static_cast<OrigDestNode*>(tempNode->suc());
      }
      
      cerr << "==== DESTINATION ====" << endl;
      tempNode = static_cast<OrigDestNode*>(dest->first());
      while (tempNode != NULL) { 
         cerr << tempNode-> getMapID() << ":" << hex 
              << tempNode->getItemID() << " (" << dec 
              << tempNode->getItemID() << ") with offset " 
              << tempNode->getOffset() << endl;
         tempNode = static_cast<OrigDestNode*>(tempNode->suc());
      }
      cerr << "=====================" << endl;
   }
} // printDebugData


////////////////////////////////////////////////////////////
//  handleRequest - inherited from Processor.
////////////////////////////////////////////////////////////

Packet*
RouteProcessor::handleRequestPacket( const RequestPacket& requestPacket,
                                     char* packetInfo )
{
   DEBUG1( cerr << "***" << endl);
   DEBUG4( cerr << "RouteProcessor::handleRequest" << endl );
   Packet* replyPacket = NULL;

   uint16 subType = requestPacket.getSubType();
      
   if ( m_packetFileName != NULL ) {
      // Use this code to write the packets to disk for later profiling
      FILE* packetFile = fopen("routePacks", "ab");
      uint32 packLen = ntohl(requestPacket.getLength());
      fwrite(&packLen, 4, 1, packetFile);
      fwrite(requestPacket.getBuf(),
             requestPacket.getLength(), 1, packetFile);
      fclose(packetFile);
   }
      
   switch (subType) {
   case Packet::PACKETTYPE_UPDATETRAFFICCOSTREQUEST : {
      DEBUG1(
             MC2INFO("RouteProcessor got UpdatetrafficCostRequestPacket"));
      const UpdateTrafficCostRequestPacket* updateTrafficCostPacket =
         static_cast<const UpdateTrafficCostRequestPacket*>(&requestPacket);
      replyPacket =
         processUpdateTrafficCostRequestPacket(updateTrafficCostPacket);
   }
      break;
         
   case Packet::PACKETTYPE_SUBROUTEREQUEST : {
      mc2dbg << "RouteProcessor got SubRouteRequestPacket - size = "
             << requestPacket.getLength() << endl;
      const RMSubRouteRequestPacket* subRoutePacket =
         static_cast<const RMSubRouteRequestPacket*>(&requestPacket);
      replyPacket = processSubRouteRequestPacket(subRoutePacket);
   }
      break;
         
   case Packet::PACKETTYPE_EDGENODESREQUEST: {
      mc2dbg << "RouteProcessor got EdgeNodesRequestPacket" << endl;
      const EdgeNodesRequestPacket* edgeNodesReq =
         static_cast<const EdgeNodesRequestPacket*>(&requestPacket);
      replyPacket = processEdgeNodesRequestPacket(edgeNodesReq);
   }
      break;
         
   case Packet::PACKETTYPE_IDTRANSLATIONREQUEST: {
      mc2dbg4 << "RouteProcessor got IDTranslationRequestPacket" << endl;
      const IDTranslationRequestPacket* levelReq =
         static_cast<const IDTranslationRequestPacket*>(&requestPacket);
      mc2dbg8 << "Incoming packet:" << endl;
      DEBUG8(
             HEXDUMP(mc2dbg8, levelReq->getBuf(), levelReq->getLength(),""));
      replyPacket = processIDTranslationRequestPacket(levelReq);
      mc2dbg8 << "Outgoing packet:" << endl;
      DEBUG8(HEXDUMP(mc2dbg8, replyPacket->getBuf(),
                     replyPacket->getLength(), ""));
   }
      break;
         
   case Packet::PACKETTYPE_DISTURBANCEPUSH: {
      mc2dbg << "RouteProcessor got DisturbancePushPacket " << endl;
      const DisturbancePushPacket* distPushReq =
         static_cast<const DisturbancePushPacket*>( &requestPacket );
      int nbrDist = processDisturbancePushPacket( distPushReq );
      replyPacket = NULL; // We cannot reply to this.
      // Add info about the number of disturbances.
      sprintf(packetInfo, "ndist = %d", nbrDist);
   }
      break;
   default : {
      mc2log << warn
             << "RouteProcessor received packet with unknown type = "
             << requestPacket.getSubTypeAsString()
             << " ip=" << requestPacket.getOriginIP()
             << ", port=" << requestPacket.getOriginPort() << endl;
      replyPacket = NULL;
   }
   }// end switch

   // If a mapload failed packets in the queue might lack the right map.
   // and need to be sent again
   if((dynamic_cast<ReplyPacket*>(replyPacket) != NULL) &&
      (static_cast<ReplyPacket*>(replyPacket)->getStatus() ==
       StringTable::MAPNOTFOUND)) {
      delete replyPacket;
      replyPacket = new AcknowledgeRequestReplyPacket(&requestPacket,
                                                      StringTable::OK,
                                                      ACKTIME_NOMAP);
   }      


   DEBUG1(cout << endl);
   
   return replyPacket;
} // handleRequest


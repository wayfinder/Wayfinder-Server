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

#include "CalcRoute.h"
#include "ItemTypes.h"
// BucketHeap only needed if main prioqueue is BucketHeap
#ifdef MAIN_PRIO_QUEUE_IN_RM_IS_BUCKET_HEAP
#include "BucketHeap.h"
#endif
// Redblack is always needed.
#include "RedBlackTree.h"
#include "StringTable.h"
#include "OrigDestNodes.h"
#include "RMSubRoute.h"
#include "SubRouteList.h"
#include "IDPairVector.h"

#include "GfxUtility.h"
#include "GfxConstants.h"
#include "DebugRM.h"
#include "RouteConstants.h"

#include "MC2BoundingBox.h"

#include "RoutingNode.h"

#include "FileDebugCalcRoute.h"

#include "OrigDestInfo.h"

#include "RoutingMap.h"
#include "Properties.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "Math.h"

#include "NodeBits.h"

#include <set>
#include <sstream>

// TODO: (When time permits or climate changes)
// * Better division of maps. (Divide where there are few external conns)
// * Fix maps so that less stuff has to be checked for each node/connection
// * Making RoutingMap smaller in memory will probably increase the speed.
// * Tune the heap for current map sizes. Hasn't been done in years.
// * Make this file more readable
// * Better transitions from car->walk, car->no_throughfare_car->walk,
//   starting_on_invalid_segment_car -> car etc. (See Vehicle)

#define FLOAT_OFFSET(a) (float32(a)/float32(MAX_UINT16))
#define FLOAT_ANTI_OFFSET(a) (float32(MAX_UINT16-(a))/float32(MAX_UINT16))

#ifdef USE_RESET_THREAD_IN_CALCROUTE
class ResetThread : public ISABThread {
public:
   ResetThread(CalcRoute* calcRoute)
         : ISABThread(), m_calcRoute(calcRoute) {
      setName("CalcRouteResetThread");
      //setPriority(JTC_MIN_PRIORITY); // Not supported.
   }

   void run() {
      m_calcRoute->resetToStartState();
      m_calcRoute->resetThreadHasFinished();
   }

   CalcRoute* m_calcRoute;
};
#endif


uint32
CalcRoute::getMapID()
{
   return m_map->getMapID();
}


RoutingMap*
CalcRoute::getMap()
{
   return m_map;
}


bool
CalcRoute::printVersion()
{
   mc2log << info
          << endl;
   return true;
}

bool
CalcRoute::versionPrinted = CalcRoute::printVersion();

#ifdef USE_RESET_THREAD_IN_CALCROUTE

void
CalcRoute::startResetThread()
{
   ISABSync sync(m_resetMonitor);
   m_resetThreadRunning = true;
   ResetThread* resetThread = new ResetThread(this);
   resetThread->start();
}

void
CalcRoute::waitForResetThread()
{
   ISABSync sync(m_resetMonitor);
   if ( m_resetThreadRunning ) {
      mc2dbg << "[CR]: Reset thread still running" << endl;
      while ( m_resetThreadRunning ) {
         m_resetMonitor.wait();
         mc2dbg << "[CR]: Waited - checking again" << endl;
      }
   } else {
      mc2dbg << "[CR]: No wait necessary" << endl;
   }
}

void
CalcRoute::resetThreadHasFinished()
{
   ISABSync sync(m_resetMonitor);
   m_resetThreadRunning = false;
   m_resetMonitor.notify();
}

#endif

CalcRoute::CalcRoute(RoutingMap* map)
{
#ifdef MAIN_PRIO_QUEUE_IN_RM_IS_BUCKET_HEAP
   m_priorityQueue            = new BucketHeap(map);
#else
   m_priorityQueue            = new RedBlackTree(map);
#endif
   m_throughfarePriorityQueue = new RedBlackTree(map);
   m_notValidPriorityQueue    = new RedBlackTree(map);
   m_normalPriorityQueue      = new RedBlackTree(map);
   m_map = map;
   m_lowerLevelBBox           = new MC2BoundingBox;
   m_outerLowerLevelBBox      = new MC2BoundingBox;
   m_outsideHeap              = new RedBlackTree(map);
#ifdef USE_RESET_THREAD_IN_CALCROUTE
   startResetThread();
#endif
}


CalcRoute::~CalcRoute()
{
#ifdef USE_RESET_THREAD_IN_CALCROUTE
   // Make sure that the reset-thread has terminated before
   // we steal the map from it.
   waitForResetThread();
#endif
   delete m_priorityQueue;
   delete m_throughfarePriorityQueue;
   delete m_notValidPriorityQueue;
   delete m_normalPriorityQueue;
   delete m_map;
   delete m_lowerLevelBBox;
   delete m_outerLowerLevelBBox;
   delete m_outsideHeap;
}


uint32
CalcRoute::estimateDistToDest( RoutingNode* node,
                               Head* dest,
                               const RMDriverPref* driverPref)
{
   return estimateDistToDest( node, dest,
                              driverPref->getCostA(),
                              driverPref->getCostB(),
                              driverPref->getCostC(),
                              driverPref->getCostD() );
}


// Use this function to calculate connectioncosts
inline uint32
CalcRoute::calcConnectionCost(dpCost_t costA,
                              dpCost_t costB,
                              dpCost_t costC,
                              dpCost_t costD,
                              uint32 vehicleRest,
                              const RoutingConnectionData *data)
{
   // Don't know if this is actually faster than multiplying
   // We'll have to time it some day.
#if 0
   uint32 retVal = 0;
   if ( costA )
      retVal += costA * data->getCostA(vehicleRest);
   if ( costB )
      retVal += costB * data->getCostB(vehicleRest);
   if ( costC )
      retVal += costC * data->getCostC(vehicleRest);
   if ( costD )
      retVal += costD * data->getCostD();
   return uint32(retVal);
#else
   return costA * data->getCostA(vehicleRest) +
          costB * data->getCostB(vehicleRest) +
          costC * data->getCostC(vehicleRest) + costD * data->getCostD();
     //return data->getCostB();
#endif
}

inline uint32
CalcRoute::calcConnectionCost(const RMDriverPref* driverParam,
                              const RoutingConnectionData* data)
{
   return calcConnectionCost(driverParam->getCostA(),
                             driverParam->getCostB(),
                             driverParam->getCostC(),
                             driverParam->getCostD(),
                             driverParam->getVehicleRestriction(),
                             data);
}

inline uint32
CalcRoute::getWalkingCostBFromA( uint32 costA )
{
   double distMeters = Connection::distCostToMeters(costA);
   double timeSeconds = distMeters * RouteConstants::INV_WALKING_SPEED_M_S;
   uint32 timeCost = uint32(Connection::secToTimeCost(timeSeconds));
   return timeCost;
}


inline uint32
CalcRoute::calcConnectionCostWalk(dpCost_t costA,
                                  dpCost_t costB,
                                  dpCost_t costC,
                                  dpCost_t costD,
                                  uint32 vehicleRest,
                                  const RoutingConnectionData* data)
{
   return costA * data->getCostA(vehicleRest) * RouteConstants::WALK_FACTOR +
      (costB + costC ) * getWalkingCostBFromA(data->getCostA(vehicleRest));
}

inline uint32
CalcRoute::calcConnectionCostWalk(const RMDriverPref* driverParam,
                                  const RoutingConnectionData* data)
{
   return calcConnectionCostWalk(driverParam->getCostA(),
                                 driverParam->getCostB(),
                                 driverParam->getCostC(),
                                 driverParam->getCostD(),
                                 driverParam->getVehicleRestriction(),
                                 data);
}


inline bool
CalcRoute::canDriveOnSegment(const RoutingNode* node,
                             const RMDriverPref* pref,
                             bool forward,
                             bool onlyFrom)
{
   const bool usingCostC = pref->getCostC() != 0;
   // The purpose of the function is to find out if it seems to
   // be allowed to drive on the segment following the node.
   // It may have been forbidden to enter the node, but that is
   // not the same.
   const bool walking = pref->getVehicleRestriction() & ItemTypes::pedestrian;

   // If we are not walking we have to check the entry restriction.
   if ( ! walking ) {
      if ( HAS_NO_WAY( node->getRestriction() ) ) {
         return false;
      }
   } 

   // Check all the connections from this node and see if there is one
   // that we can drive on. If there is, we assume that it is allowed
   // to drive on the segment too.
   for ( RoutingConnection* curConn = node->getFirstConnection(forward);
         curConn != NULL;
         curConn = curConn->getNext() ) {
      if ( curConn->getData()->getVehicleRestriction(usingCostC) &
           pref->getVehicleRestriction() ) {
         // Also check the entry restriction of the next node.
         // Seems like map people sometimes use NO_ENTRY when
         // it should be NO_WAY.
         if ( ! walking ) {
            if ( !NOT_VALID(curConn->getNode()->getRestriction() ) ) {
               return true;
            } 
         } else {
            return true;
         }
      }
   }

   if ( ! onlyFrom ) {
      // Check the entry connections too. If it is possible to drive in
      // it should be possible to drive on the segment.
      const bool backwards = !forward;
      // If the node has NO_ENTRY it cannot be possible to drive in      
      if ( !walking && HAS_NO_ENTRY ( node->getRestriction() ) ) {
         return false;
      }
      for ( RoutingConnection* curConn = node->getFirstConnection(backwards);
            curConn != NULL;
            curConn = curConn->getNext() ) {
         if ( curConn->getData()->getVehicleRestriction(usingCostC) &
              pref->getVehicleRestriction() ) {
            // One connection lets us get into the segment.
            return true;
         }
      }
   }
   
   // Did not find a connection which we could drive on
   return false;
}

inline bool
CalcRoute::couldDriveIntoSegment(const RoutingNode* node,
                                 const RMDriverPref* pref,
                                 bool forward)
{
   const bool usingCostC = pref->getCostC() != 0;
   // The purpose of this function is to check if we are
   // starting on an unallowed segment
   const bool walking = pref->getVehicleRestriction() & ItemTypes::pedestrian;

   // The other node on the segment.
   RoutingNode* otherSide = m_map->getNodeFromTrueNodeNumber(
      TOGGLE_UINT32_MSB( node->getItemID() ) );
   
   // If we are walking we don't have to chk the entry restriction
   if ( ! walking ) {
      if ( (HAS_NO_WAY( node->getRestriction()) ||
            HAS_NO_ENTRY( node->getRestriction())) &&
           (HAS_NO_WAY( otherSide->getRestriction() ||
                        HAS_NO_ENTRY(otherSide->getRestriction() ) ) ) ) {
         return false; // We were not allowed to get into this segment.
         // Not to get into the segment and turn the car around either
      }
   }

   // Check the entry connections too.
   const bool backwards = !forward;
   for ( RoutingConnection* curConn = node->getFirstConnection(backwards);
         curConn != NULL;
         curConn = curConn->getNext() ) {
      if ( curConn->getData()->getVehicleRestriction(usingCostC) &
           pref->getVehicleRestriction() ) {
         // One connection lets us get into the segment.
         return true;
      }
   }
   for ( RoutingConnection* curConn = node->getFirstConnection(forward);
         curConn != NULL;
         curConn = curConn->getNext() ) {
      if ( curConn->getData()->getVehicleRestriction(usingCostC) &
           pref->getVehicleRestriction() ) {
         // One connection lets us get out of the segment.
         // That is, it was possible to drive into the segment
         // and turn the car around.
         return true;
      }
   }
   // No good connections were found.
   return false;
}
                                 

inline void
CalcRoute::updateLowerLevelBBox(Head* origins)
{
   // The minimum half size of the height and width of the bbox.
   static const int incSizeMeters = 1200;

   // The same for the outer bbox
   static const int outerIncSizeMeters = 5000;
   
   m_lowerLevelBBox->reset();
   m_outerLowerLevelBBox->reset();
   OrigDestNode* curOrig = static_cast<OrigDestNode*>(origins->first());
   while ( curOrig != NULL ) {
      RoutingNode* realNode =
         m_map->getNodeFromTrueNodeNumber(curOrig->getItemID());
      MC2_ASSERT( realNode != NULL) ;

      m_lowerLevelBBox->update(realNode->getLat(), realNode->getLong());
      m_outerLowerLevelBBox->update(realNode->getLat(), realNode->getLong());

      curOrig = static_cast<OrigDestNode*>(curOrig->suc());
   }

   // Add some distance to the bbox
   m_lowerLevelBBox->increaseMeters(uint16(incSizeMeters));
   m_outerLowerLevelBBox->increaseMeters(uint16(outerIncSizeMeters));
   mc2dbg << "The size of the inner bbox is (h/m, w/m) ("
          << (m_lowerLevelBBox->getHeight() * GfxConstants::MC2SCALE_TO_METER)
          << ","
          << (m_lowerLevelBBox->getWidth() * GfxConstants::MC2SCALE_TO_METER)
          << ')' << endl;
      mc2dbg << "The size of the outer bbox is (h/m, w/m) ("
          << (m_outerLowerLevelBBox->getHeight()
              * GfxConstants::MC2SCALE_TO_METER)
          << ","
          << (m_outerLowerLevelBBox->getWidth()
              * GfxConstants::MC2SCALE_TO_METER)
          << ')' << endl;
}

inline bool
CalcRoute::shouldIncludeNode(const RoutingNode* nextNode,
                             bool curNodeOnLowerLevel,
                             bool routeToHigher)
{
   // We're not routing to higher. Let's return true.
   if ( ! routeToHigher )
      return true;

   // These are the rules
   // 1. Everything is allowed inside the inner bbox
   // 2. If we are outside the inner bbox, but inside the outer
   //    bbox then we are not allowed to route from nodes on higher
   //    level to nodes on lower level.
   // 3. If we are outside both boxes, it is only allowed to route
   //    to nodes on higher level.
   

   // Always allow everything inside the bounding box. Unlikely, since
   // it should only be like that in the beginning.
   if ( MC2_UNLIKELY(
      m_lowerLevelBBox->inside(nextNode->getLat(), nextNode->getLong() ) ) ) {
      return true;
   }

   // Now we must be outside the inner bounding box.
   // We're allowed to route to higher level nodes
   if ( IS_UPPER_LEVEL( nextNode->getItemID() ) ) {
      return true;
   }

   // The next node is not inside the inner bbox.
   if ( m_outerLowerLevelBBox->inside(nextNode->getLat(),
                                      nextNode->getLong()) ) {
      // If the next node is on lower level, this node must
      // also be on lower level.
      return curNodeOnLowerLevel;
   } else {
      // Outside the outer we are not allowed to
      // continue on lower level either.
      return false;
   }
}

void
CalcRoute::resetToStartState()
{
   // Timer
   uint32 resetStartTime = TimeUtility::getCurrentTime();
   trickTheOptimizer(resetStartTime);
   // Here is the resetting
   m_map->reset();
   uint32 heapResetStartTime = TimeUtility::getCurrentTime();
   trickTheOptimizer(heapResetStartTime);
   m_priorityQueue->reset();
   m_normalPriorityQueue->reset();
   m_invalidNodeVector.clear();
   m_notValidPriorityQueue->reset();
   m_throughfarePriorityQueue->reset();
   uint32 heapResetStopTime = TimeUtility::getCurrentTime();
   trickTheOptimizer(heapResetStopTime);
   // Print the time
   uint32 resetTime = TimeUtility::getCurrentTime() - resetStartTime;
   trickTheOptimizer(resetTime);
   char debugString[1024];
   sprintf(debugString, "Map %08x + pq reset in %u millis, pq in %u", getMapID(),
           resetTime, (heapResetStopTime-heapResetStartTime));
   mc2dbg << debugString << endl;
}

bool 
CalcRoute::checkVehicleRestrictions(RoutingNode* curNode,
                                    const RMDriverPref* driverParam,
                                    bool forward)
{
   const bool usingCostC = driverParam->getCostC() != 0;
   const bool walking =
      driverParam->getVehicleRestriction() == ItemTypes::pedestrian;
   
   if ( !forward && !walking ) {
      if ( NOT_VALID(curNode->getRestriction()) )
           return false;
   }
   
   RoutingConnection* connection = curNode->getFirstConnection(forward);
   
   while( connection != NULL ) {
      RoutingConnectionData* data = connection->getData();

      //RoutingNode* nextNode = m_map->getNode(connection->getIndex());
      RoutingNode* nextNode = connection->getNode();
      bool nextNodeValid = !NOT_VALID(nextNode->getRestriction())
         || !forward || walking;
      
      // Check if it is possible to drive from this node
      if( data->getVehicleRestriction(usingCostC) &
          driverParam->getVehicleRestriction() && nextNodeValid) {
         // Possible to drive through at least one connection
         return true;
      }
      connection = connection->getNext();
   }
   
   // The only way to get out from this node is to walk
   return false;
} // checkVehicleRestrictions


inline uint32
CalcRoute::checkOrigDests(Head* origins,
                          Head* dests)
{
   // Go through the lists and see if there are any that
   // aren't in the map.
   for (int i = 0; i < 2; ++i) {
      // Select dests or origins.
      Head* curList = (i == 0) ? origins : dests;
      OrigDestNode* tempNode 
         = static_cast<OrigDestNode*>( curList->first() );
      
      while( tempNode != NULL ) {
         if ( tempNode->getIndex() == MAX_UINT32 ) {
            MC2WARNING("CalcRoute: Node has index MAX_UINT32");
         }
         if ( curList == origins ) {
            mc2dbg << "[CR]: Origin at " << hex << tempNode->getItemID()
                   << dec << endl;
         }
         RoutingNode* curNode = m_map->getNode( tempNode->getIndex() );
         // XXX: Is this correct??
         tempNode->setItemID(curNode->getItemID());
         if ( curNode == NULL ) {
            char debugString[1024];
            sprintf(debugString, "Couldn't find index %d in the map",
                    tempNode->getItemID());
            MC2ERROR(debugString);
            if ( curList == origins ) {
               // EARLY RETURN HERE
               return StringTable::ONE_OR_MORE_INVALID_ORIGS;
            } else {
               // EARLY RETURN HERE
               return StringTable::ONE_OR_MORE_INVALID_DESTS;
            }
         }
         tempNode = static_cast<OrigDestNode*>(tempNode->suc());
      }
   }
   // All nodes were found on the map.
   return StringTable::OK;
}

int
CalcRoute::removeDups(Head* theList, bool isDest)
{
   // Timer
   uint32 startTime = TimeUtility::getCurrentTime();
   
   int nbrRemoved = 0;
   //  nodeid  origdestnode
   map<uint32, OrigDestNode*> origDestList;
   OrigDestNode* curNode = static_cast<OrigDestNode*>(theList->first());
   
   while ( curNode != NULL ) {
      OrigDestNode* nextNode = static_cast<OrigDestNode*>(curNode->suc());
      map<uint32, OrigDestNode*>::iterator it =
         origDestList.find(curNode->getItemID());
      if ( it == origDestList.end() ) {
         origDestList.insert(
            pair<uint32,OrigDestNode*>(curNode->getItemID(), curNode));
      } else {
         ++nbrRemoved;
         OrigDestNode* listNode = it->second;
         // Compare the offsets.
         bool condition = curNode->getOffset() > listNode->getOffset();
         // For destination we want small offsets.
         if ( isDest ) {
            condition = ! condition;
         }
         if ( condition ) {
            // The one in the map has greater offset.
            // Remove it.
            listNode->out();
            origDestList.erase(it);
            delete listNode;
            // Insert our node
            origDestList[curNode->getItemID()] = curNode;
         } else {
            curNode->out();
            delete curNode;
         }
      }
      curNode = nextNode;
      
   }
   uint32 time = TimeUtility::getCurrentTime() - startTime;
   char debugString[1024];
   sprintf(debugString, "Dups removed in %u millis", time);
   mc2dbg << debugString << endl;
   return nbrRemoved;
}

Head*
CalcRoute::removeUnreachable(Head* origDest,
                             const RMDriverPref* prefs,
                             bool forward)
{
   Head* retList = new Head();
   int nbrRemoved = 0;
   // For each node
   OrigDestNode* curNode = static_cast<OrigDestNode*>(origDest->first());
   while ( curNode != NULL ) {
      OrigDestNode* nextNode = static_cast<OrigDestNode*>(curNode->suc());
      RoutingNode* realNode =
         m_map->getNodeFromTrueNodeNumber(curNode->getItemID());
      if ( !checkVehicleRestrictions(realNode, prefs, forward) ||
           !checkVehicleRestrictions(realNode, prefs, !forward) ||
           ! HAS_NO_RESTRICTIONS(realNode->getRestriction()) ||
           realNode->getFirstConnection(forward) == NULL ||
           realNode->getFirstConnection(!forward) == NULL) {
         // Can only walk from this node.
         ++nbrRemoved;
         curNode->out();
         curNode->into(retList);
      }
      curNode = nextNode;
   }
   return retList;
}

int
CalcRoute::routeOnOneSegment(Head* origs,
                             Head* dests,
                             const RMDriverPref* pref,
                             bool forward,
                             SubRouteList* incoming,
                             SubRouteList* result,
                             bool routeToAll,
                             bool calcCostSums)
{
   // No one-segment routes on overviewmaps, please.
//     if ( m_map->getMapID() >= 0x8000000 )
//        return 0;
   // For costSums
   const bool usingCostC = pref->getCostC() != 0;
   RMDriverPref driverPrefA;
   driverPrefA.setRoutingCosts(0x01000000);
   RMDriverPref driverPrefB;
   driverPrefB.setRoutingCosts(0x00010000);
   RMDriverPref driverPrefC;
   driverPrefC.setRoutingCosts(0x00000100);

   map<uint32, OrigDestNode*> origMap;
   for( OrigDestNode* curNode =
           static_cast<OrigDestNode*>(origs->first());
        curNode != NULL;
        curNode = curNode->next() ) {
      origMap.insert(make_pair(curNode->getItemID(), curNode));
   }

   RoutingNode* externalNode = m_map->getExternalNodes();
   for (uint32 i = 0; i < m_map->getNumExternalNodes(); i++) {
      RoutingNode* tempNode = 
         m_map->getNodeFromTrueNodeNumber(externalNode[i].getItemID());
      // Add the external nodes as routes with the same cost as original
      // since all external connections have length == 0
      map<uint32, OrigDestNode*>::const_iterator it =
         origMap.find( externalNode[i].getItemID() );
      if ( it != origMap.end() ) {
         mc2dbg << "[CR]: Orig is external" << endl;
      
         // Add the result to the result list.
         CompleteRMSubRoute* subRoute =
            new CompleteRMSubRoute(m_map->getMapID());
         subRoute->setPrevSubRouteID(
            incoming->findExternalNode( externalNode[i].getItemID() ) );
         subRoute->addEndNode( it->second->getItemID() );
         subRoute->setEndOffset( it->second->getOffset() );
         subRoute->setStartOffset( it->second->getOffset() );
         for ( ExternalRoutingConnection* extCon =
                  (ExternalRoutingConnection*)
                  externalNode[i].getFirstConnection(true);
               extCon != NULL;
               extCon = (ExternalRoutingConnection*)
                  (extCon->getNext()) ) {
            subRoute->addExternal( extCon->getMapID(),
                                   extCon->getNodeID(),
                                   it->second->getRealCost(m_map),
                                   it->second->getRealCost(m_map),
                                   tempNode->getLat(),
                                   tempNode->getLong(),
                                   it->second->getCostASum(),
                                   it->second->getCostBSum(),
                                   it->second->getCostCSum());
         }
         if ( result->addSubRoute( subRoute ) == MAX_UINT32 ) {
            // Cheaper exists
            delete subRoute;
         } else {

         }
      }
   }

   
   int nbrSameSeg = 0;
   OrigDestNode* curOrig = static_cast<OrigDestNode*>(origs->first());
   while ( curOrig ) {
      OrigDestNode* curDest = static_cast<OrigDestNode*>(dests->first());
      while ( curDest ) {
         if ( curDest->getItemID() == curOrig->getItemID() &&
              curDest->getMapID() == curOrig->getMapID()) {
            // They are the same (node)
            RoutingNode* rNode =
                  m_map->getNode(curDest->getIndex());

            bool forbidden = NOT_VALID(rNode->getRestriction());

            bool walkForbidden = true;

            // Check if it is possible to walk
            RoutingConnection* curConn =
               rNode->getFirstConnection(!forward);
            while ( curConn != NULL ) {
               RoutingConnectionData* data = curConn->getData();
               if (data->getVehicleRestriction(usingCostC) &
                   ItemTypes::pedestrian) {
                  walkForbidden = false;
                  break;
               }
               curConn = curConn->getNext(); 
            }

            bool reallyForbidden = walkForbidden && forbidden;
            
            if ( !reallyForbidden &&
                 curDest->getOffset() >= curOrig->getOffset() ) {

               uint32 penaltyFactor = 1;
               if ( forbidden )
                  penaltyFactor = RouteConstants::WALK_FACTOR;
            
               
               // Only calc this way since we will find the other node
               // later.
               float offsetFactor;
               if ( curDest->getOffset() > curOrig->getOffset() ) {
                  offsetFactor = FLOAT_OFFSET(curDest->getOffset())-
                                 FLOAT_OFFSET(curOrig->getOffset());
               } else {
                  offsetFactor = FLOAT_OFFSET(curOrig->getOffset())-
                                 FLOAT_OFFSET(curDest->getOffset());
               }

               uint32 minCost = getMinCost(rNode, pref, true);
               uint32 cost = uint32(minCost * offsetFactor);
               if ( curOrig->getTurnCost() != 0 )
                  mc2dbg2 << "Setting turncost = " << curOrig->getTurnCost()
                          << endl;
               cost += curOrig->getTurnCost();
               cost += curOrig->getRealCost(m_map);              

               cost *= penaltyFactor;
               
               mc2dbg4 << here << " itemID = " << hex
                       << curDest->getItemID() << dec << endl;
               mc2dbg4 << " destoffset = " << curDest->getOffset() << endl;
               mc2dbg4 << " origoffset = " << curOrig->getOffset() << endl;
               mc2dbg4 << " factor = " << offsetFactor << endl;
               mc2dbg4 << here << " cost = " << cost << endl;
               // Add the result to the result list.
               CompleteRMSubRoute* subRoute =
                  new CompleteRMSubRoute(m_map->getMapID());
               subRoute->setEndOffset( curDest->getOffset() );
               subRoute->setStartOffset( curOrig->getOffset() );
               subRoute->setForward(forward);
               subRoute->setPrevSubRouteID(
                  incoming->findExternalNode( curDest->getItemID() ) );
               if ( forbidden ) {
                  mc2dbg << "forbidden..." << endl;
                  subRoute->addLast(RouteConstants::WALK_ITEM_ID);
               }
               subRoute->addEndNode( curDest->getItemID() );
               
               // Calc the cost sums for sortdist request.
               uint32 costASum;
               uint32 costBSum;
               uint32 costCSum;
               if ( calcCostSums ) {
                  costASum = uint32 ( curOrig->getCostASum()
                     + getMinCost(rNode, &driverPrefA, true) * offsetFactor);
                  costBSum = uint32 ( curOrig->getCostBSum() 
                     + getMinCost(rNode, &driverPrefB, true) * offsetFactor);
                  costCSum = uint32 ( curOrig->getCostCSum()
                     + getMinCost(rNode, &driverPrefC, true) * offsetFactor);
               } else {
                  costASum = 0;
                  costBSum = 0;
                  costCSum = 0;
               }
               
               subRoute->addExternal( m_map->getMapID(),
                                      curDest->getItemID(),
                                      cost,
                                      cost,
                                      rNode->getLat(),
                                      rNode->getLong(),
                                      costASum,
                                      costBSum,
                                      costCSum);
               subRoute->setRouteComplete(true);
               subRoute->setVisited(true);

               // Also check if the subroute leads to another map
#if 1
               RoutingNode* externalNode = m_map->getExternalNodes();
               // Is there a better way than to loop?
               for (uint32 i = 0; i < m_map->getNumExternalNodes(); i++) {
                  const uint32 curID = externalNode[i].getItemID();
                  
                  if ( curID == rNode->getItemID() ) {                     
                     ExternalRoutingConnection* extCon =
                        (ExternalRoutingConnection*)
                        externalNode[i].getFirstConnection(true);
                     while ( extCon != NULL ) {
                        mc2dbg << "[CR]: Onenode to external "
                               << IDPair_t(extCon->getMapID(),
                                           extCon->getNodeID()) << endl;
                        
                        subRoute->addExternal( extCon->getMapID(),
                                               extCon->getNodeID(),
                                               cost,
                                               cost,
                                               rNode->getLat(),
                                               rNode->getLong(),
                                               costASum,
                                               costBSum,
                                               costCSum);
                        extCon = (ExternalRoutingConnection*)
                                        (extCon->getNext());
                     }
                  }
               }
#endif
               
               if ( ! routeToAll ) {
                  if ( cost <= m_cutOff ) {
                     mc2dbg << "[CR]: RouteOnOne setting cutoff" << endl;
                     m_cutOff = cost;
                  }
               }

               if ( cost < rNode->getRealCost(m_map) ) {
//                    rNode->setEstCost(m_map,minCost *
//                                   FLOAT_ANTI_OFFSET(curOrig->getOffset()));
//                    rNode->setRealCost(m_map,rNode->getCost());
//                    m_priorityQueue->enqueue(rNode);
               }
               
               if ( result->addSubRoute( subRoute ) == MAX_UINT32 ) {
                  // Cheaper exists
                  delete subRoute;
               } else {
                  nbrSameSeg++;
               }
            }
         }
         curDest = static_cast<OrigDestNode*>(curDest->suc());           
      }
      curOrig = static_cast<OrigDestNode*>(curOrig->suc());
   }
   return nbrSameSeg;
}

/////////////////////////////////////////////////////////////////////
// Functions for initialize the routing
/////////////////////////////////////////////////////////////////////

uint32 
CalcRoute::initRoute( bool originalRequest,
                      Head* origin,
                      Head* destination,
                      const RMDriverPref* driverParam,
                      bool routeToHigherLevel,
                      bool forward ){
   mc2dbg2 << "CalcRoute::initRoute(1)" << endl;
   FileDebugCalcRoute::writeComment("initRoute(1)");

   Head* orig;
   Head* dest;

   // If we are routing backwards then swap the places.
   if( forward ) {
      orig = origin;
      dest = destination;
   } else {
      orig = destination;
      dest = origin;
   }

   DEBUG8(
      mc2dbg<< "origins : " << endl;
      for(OrigDestNode* cur = static_cast<OrigDestNode*>(orig->first());
          cur != NULL;
          cur = static_cast<OrigDestNode*>(cur->suc()) ) {
         ((OrigDestNode*)cur)->dump();
      }
      mc2dbg << "destinations : " << endl;
      for(OrigDestNode* cur = static_cast<OrigDestNode*>(dest->first());
          cur != NULL; 
          cur = static_cast<OrigDestNode*>(cur->suc())) {
         ((OrigDestNode*)cur)->dump();
      }
   );
   
   uint32 status;
   
   // Always set original request.
   // Can be trouble with external nodes otherwise
   // (e.g. Stockholm->Gotland)
   // 2003-07-09
   //originalRequest = true;
   // 2003-07-17 
   // New version of initroute will take care of the Gotland case.
   if( originalRequest ) {
      status = initRouteFirstTime( orig,
                                   dest,
                                   driverParam,
                                   routeToHigherLevel,
                                   forward );
   } else {
      status = initRoute( orig,
                          dest,
                          driverParam,
                          routeToHigherLevel,
                          forward );
   }
   return status;
}

uint32 
CalcRoute::initRoute( Head* startRoutingNodeList,
                      Head* endRoutingNodeList,
                      const RMDriverPref* driverParam,
                      bool routeToHigherLevel,
                      bool forward )
{

   mc2dbg4 << "CalcRoute::initRoute(2)" << endl;
   FileDebugCalcRoute::writeComment("initRoute(2)");

   // Find the minimum cost and initialize the bucketheap if necessary
   uint32 minCost = MAX_UINT32;
   {
      OrigDestNode* tempNode =
         static_cast<OrigDestNode*>(startRoutingNodeList->first());
      
      if( tempNode == NULL ) {
         mc2dbg << "tempNode == NULL " << endl;
         mc2dbg << "Direction is forward " << BP(forward) << endl;
      }
      
      while( tempNode != NULL ){
         if (tempNode->getRealCost(m_map) < minCost) {
            minCost = tempNode->getRealCost(m_map);
         }
         tempNode = static_cast<OrigDestNode*>(tempNode->suc());
      }
      
      if (minCost != MAX_UINT32) {
         m_priorityQueue->updateStartIndex(minCost);
      }
   }

#if 0
   {
      OrigDestNode* tempNode =
         static_cast<OrigDestNode*>(startRoutingNodeList->first());
      while( tempNode != NULL ){
         
         RoutingNode* curNode = m_map->getNode( tempNode->getIndex() );
         if( ( curNode != NULL ) &&
             HAS_NO_RESTRICTIONS( curNode->getRestriction() ) ) {
            
            // Check cost first... It may avoid the infinite loops.
            if ( curNode->getRealCost(m_map) > tempNode->getRealCost(m_map) ) {
               // Update this nodes cost to avoid infinite loops
               curNode->setRealCost(m_map, tempNode->getRealCost(m_map) );
               curNode->setEstCost(m_map,  tempNode->getRealCost(m_map) );
               // XXX: This should be right, but may be wrong
               //      Have to take care of it in readresult
               //      (The same ID will be there twice)
               curNode->setGradient(m_map, tempNode );
               
               // What is this ?
#           if 0
               RoutingNode* temp =
                  m_map->getNodeFromTrueNodeNumber(
                     TOGGLE_UINT32_MSB( curNode->getItemID() ) );
               temp->setVisited(m_map, true);
#           endif
               m_priorityQueue->enqueue( curNode );
            }
         } else {
            if ( curNode == NULL ) {
               mc2log << warn << "[CR]: Route from inexistant node "
                      << IDPair_t(m_map->getMapID(), tempNode->getItemID())
                      << endl;
            } else if ( !HAS_NO_RESTRICTIONS(curNode->getRestriction() ) ) {
               mc2dbg << "[CR]: External route starts at node with rest "
                      << IDPair_t(m_map->getMapID(), tempNode->getItemID())
                      << endl;
            }
         }
         tempNode = tempNode->next();
      }
   }
#else
   const bool usingCostC = driverParam->getCostC() != 0;
   for( OrigDestNode* curOrigNode =
           static_cast<OrigDestNode*>(startRoutingNodeList->first());
        curOrigNode != NULL;
        curOrigNode = curOrigNode->next() ) {
      // Get real node
      RoutingNode* curNode = m_map->getNode( curOrigNode->getIndex());
      if ( curNode == NULL) {
         mc2log << error << "[CR]: Found node is NULL in ir" << endl;
         continue;
      }

      for( const RoutingConnection* curConn =
              curNode->getFirstConnection(forward);
           curConn != NULL;
           curConn = curConn->getNext() ) {
         // Get the connection data.
         const RoutingConnectionData* connData = curConn->getData();
         // Check what to do.
         // Get the node from the connection
         RoutingNode* nextNode = curConn->getNode();
         if ( ! HAS_NO_RESTRICTIONS( nextNode->getRestriction() ) ) {
            // Only consider the ones that have no restrictions.
            continue;
         }
         const bool vehicleAllowed =
            connData->getVehicleRestriction(usingCostC) &
            driverParam->getVehicleRestriction();
         if ( ! vehicleAllowed ) {
            continue;
         }
       
         // All is ok - enqueue the new node.
         uint32 cost = curOrigNode->getRealCost(m_map) +
            calcConnectionCost(driverParam, connData);
         if ( cost < curNode->getRealCost(m_map) ) {
            nextNode->setRealCost(m_map, cost);
            nextNode->setEstCost(m_map, cost);
            nextNode->setGradient(m_map, curOrigNode);
            m_priorityQueue->enqueue(nextNode);
         }
      }
   }
#endif
   
   // Mark destination nodes if there are'nt any we will route
   // to the external nodes
   OrigDestNode* tempNode =
      static_cast<OrigDestNode*>(endRoutingNodeList->first());
   while (tempNode != NULL) {
      RoutingNode* curNode = m_map->getNode( tempNode->getIndex() );
      curNode->setDest(m_map,true);
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());      
   }
   return StringTable::OK;

}


uint32
CalcRoute::initRouteFirstTime( Head* startRoutingNodeList,
                               Head* endRoutingNodeList,
                               const RMDriverPref* driverParam,
                               bool routeToHigherLevel,
                               bool forward,
                               bool secondTime )
{
   const bool usingCostC = driverParam->getCostC() != 0;
   mc2dbg << "CalcRoute::initRouteFirstTime" << endl; 
   FileDebugCalcRoute::writeComment("initRouteFirstTime");

   uint32 status = checkOrigDests(startRoutingNodeList,
                                  endRoutingNodeList);
   if ( status != StringTable::OK ) {
      // Something was wrong with the origins or destinations.
      return status;
   }

   // Mark destination nodes. If there aren't any we will route
   // to the external nodes
   for( OrigDestNode* destNode =
           static_cast<OrigDestNode*>(endRoutingNodeList->first());
        destNode != NULL;
        destNode = destNode->next() ) {
      // Get real node and mark it as destination.
      RoutingNode* curNode = m_map->getNode(destNode->getIndex());
      curNode->setDest(m_map,true);      
   }
   
   
   // Parse all the starting nodes to get valid starting nodes
   OrigDestNode* curStartNode 
      = static_cast<OrigDestNode*>( startRoutingNodeList->first() );

   set<RoutingNode*> allowedDests;
   
   while( curStartNode != NULL ) {      
      if ( curStartNode->getIndex() == MAX_UINT32 ) {
         mc2log << warn << "CalcRoute: Node has index MAX_UINT32" << endl;
         // Take the next node
         curStartNode = curStartNode->next();
         continue;
      }

      // Get real node from map.
      RoutingNode* curNode = m_map->getNode( curStartNode->getIndex() );

      // Check if it was possible to get here.
      const bool couldDriveHere = couldDriveIntoSegment(curNode,
                                                        driverParam,
                                                        forward);

      mc2dbg << "[CR]: Could drive into node " << hex
             << curNode->getItemID() << dec << " = " << BP(couldDriveHere)
             << endl;
      
      if (curNode != NULL) {

         float32 dOffset = FLOAT_OFFSET( curStartNode->getOffset() );

         // Calculate the smallest cost to get from this node.
         // Use it as the "length" of this segment.        
         uint32 turnFreeCost = getMinCost(curNode, driverParam);
         // Same, but when we are "walking" (e.g. driving where
         // we cannot).
         uint32 turnFreeCostWalk = getMinCost(curNode, driverParam);
         
         // The cost for this node. Cannot be set, since current
         // node may be visited from another curNode.
         // uint32 curNodeCost = MAX_UINT32;
         //uint32 curNodeCost = uint32((1-dOffset)*turnFreeCost);
         
         mc2dbg2 << "[CR]: irft: turnfreeCost=" << turnFreeCost << endl;
         
         // Calculate how much of this cost that should be removed
         // from the connection cost.
         uint32 lesserCost = uint32(turnFreeCost * dOffset);
         uint32 lesserCostWalk = uint32(turnFreeCostWalk * dOffset);

         mc2dbg2 << "[CR]: irft: lesserCost=" << lesserCost << endl;

         // New strategy introduced here 2002-12-11
         // Enqueue the non-valid nodes in non-valid queue.
         // Enqueue the valid nodes in normal queue.
         // Enquque the nt-nodes into throughfare queue.
         // Afterwards the expansion of non-valid nodes will 
         // continue until there are no nodes in the non-valid queue
         // or a destination is reached.
         
         // The main difference between this code and the old one is that
         // the old one checked all connections from a node and if there
         // were one valid connection, the other nodes were not enqueued
         // in nonvalid.
         
         for(const RoutingConnection* curConn =
                curNode->getFirstConnection(forward);
             curConn != NULL;
             curConn = curConn->getNext()) {
            
            // Get the connection data.
            const RoutingConnectionData* connData = curConn->getData();

            // Get the node from the connection
            RoutingNode* nextNode = curConn->getNode();
           
            const bool walking = driverParam->getVehicleRestriction() & 
               ItemTypes::pedestrian;
            
            // Check what to do.
            bool vehicleAllowed = connData->getVehicleRestriction(usingCostC) &
                                  driverParam->getVehicleRestriction();

            const bool allAllowed = vehicleAllowed && ( (
               ( !HAS_NO_WAY(curNode->getRestriction() ) &&
                 HAS_NO_RESTRICTIONS(nextNode->getRestriction() ) ) ) ||
                  walking );
            const bool throughfare = vehicleAllowed &&
               ( HAS_NO_THROUGHFARE(nextNode->getRestriction() )
                 && HAS_NO_THROUGHFARE(curNode->getRestriction() ) );

            const bool throughfareLater = vehicleAllowed &&
               ( HAS_NO_THROUGHFARE(nextNode->getRestriction()) &&
                 ( !HAS_NO_THROUGHFARE(curNode->getRestriction())));
            
            const bool walkAllowed =
               ItemTypes::pedestrian &
               connData->getVehicleRestriction(usingCostC);

            // Check if we should punish.
            uint32 cost; 
            uint32 removeCost; 
            if ( canDriveOnSegment(curNode, driverParam, forward) ) {
               // Same cost for allAllowed and throughfare
               cost = curStartNode->getRealCost(m_map) +
                  calcConnectionCost( driverParam, connData );
               removeCost = lesserCost;
            } else {
               // Use walking cost if not allowed.
               mc2dbg << "[CR]: irft - using walking cost from node 0x"
                      << hex << curNode->getItemID() << dec
                      << endl;
               cost = curStartNode->getRealCost(m_map) +
                  calcConnectionCostWalk( driverParam, connData );
               removeCost = lesserCostWalk;
            }

            // Add the turncost
            if ( forward ) {
               if ( curStartNode->getTurnCost() ) {
                  mc2dbg << "[CR]: Adding turncost "
                         << endl;
                  cost += curStartNode->getTurnCost();
               }
            }
            
            // Remove the offset cost.
            if ( cost >= removeCost ) {
               cost -= removeCost;
            } else {
               mc2log << warn << here
                      << " cost became 0" << endl;
               cost = 0;
            }
            
            // Dijkstra
            if ( cost < nextNode->getRealCost(m_map) ) {
               // Set the costs.
               nextNode->setRealCost(m_map, cost );
               nextNode->setEstCost(m_map, cost );
               // Points back to the OrigDestNode.
               nextNode->setGradient(m_map, curStartNode ); 

               const char* queueName = "";
               // Choose the right queue and enqueue the node
               // Check if we can drive from the segment.
               if ( allAllowed || ( canDriveOnSegment( nextNode, driverParam,
                                                       forward, true) &&
                                    !couldDriveHere ) ) {
                  // Everything is ok
                  if ( couldDriveHere && allAllowed ) {
                     // We want to expand all the valid nodes
                     m_priorityQueue->enqueue( nextNode );
                     nextNode->setVisited(m_map, true);
                     queueName = "main";
                  } else {
                     if ( HAS_NO_THROUGHFARE(nextNode->getRestriction() ) ) {
                        m_throughfarePriorityQueue->enqueue( nextNode );
                        queueName = "throughfare [b]";
                        // New 2003-07-31
                        m_invalidNodeVector.
                           push_back(make_pair(curConn, nextNode));
                      } else {
                        // We only want to expand the cheapest valid node.
                        m_normalPriorityQueue->enqueue( nextNode );
                        queueName = "normal";
                        // New 2003-08-01
                        m_invalidNodeVector.
                           push_back(make_pair(curConn, nextNode));
                        
                      }
                  }
                  allowedDests.insert(nextNode);

               } else if ( throughfare ) {
                  // No throughfare node
                  m_throughfarePriorityQueue->enqueue( nextNode );
                  allowedDests.insert(nextNode);
                  queueName = "throughfare";
                  // New 2003-08-01
                  m_invalidNodeVector.
                     push_back(make_pair(curConn, nextNode));
               } else if ( throughfareLater ) {
                  nextNode->setVisited(m_map, false);
                  m_invalidNodeVector.
                     push_back(make_pair(curConn, nextNode));
                  if ( nextNode->isDest(m_map) ) {
                     // Set cutoff - a little high but better than
                     // nothing.
                     m_cutOff = nextNode->getRealCost(m_map) +
                        getMinCost(nextNode, driverParam);
                  }
                  queueName = "throughfare later";

               } else {
                  // We cannot drive here.
                  if ( nextNode->isDest(m_map) ) {
                     // We want to find the destination when expanding
                     // later instead.
                     mc2dbg << "[CR]: irft - Node 0x" << hex
                            << nextNode->getItemID() << dec
                            << " is a dest" << endl;
                     if ( false ) {
                        nextNode->reset(m_map);
                        nextNode->setDest(m_map,true);
                     }                     
                  } else {                     
                     // so we can use it as a real segment.
                     if ( canDriveOnSegment( nextNode, driverParam, forward) ){
                        mc2dbg << "[CR]: Can drive on segment "
                               << hex << nextNode->getItemID() << dec
                               << endl;
                        // Also insert the node in the m_normalPriorityQueue
                        m_normalPriorityQueue->enqueue( nextNode );
                        // New 2003-08-01
                        m_invalidNodeVector.push_back(make_pair(curConn,
                                                                nextNode));
                        queueName = "normal";
                     } else {
                        queueName = "non-valid";
                        if ( walkAllowed  ) {
                           m_notValidPriorityQueue->enqueue( nextNode );
                           nextNode->setVisited(m_map, false);
                           m_invalidNodeVector.push_back(make_pair(curConn,
                                                                   nextNode));
                           allowedDests.insert(nextNode);
                        }
                        
                     }  
                  }
               }
               mc2dbg << "[CR]: irft - putting node "
                      << hex << nextNode->getItemID() << dec
                      << " with cost " << nextNode->getRealCost(m_map)
                      << " into " << queueName << endl;
            }
         }
      }
      curStartNode = curStartNode->next();
   }
   
   if (m_notValidPriorityQueue->isEmpty()) {
      mc2dbg2 << "m_notValidPriorityQueue is empty" << endl;
   }

   if (m_throughfarePriorityQueue->isEmpty()) {
      mc2dbg2 << "m_throughfarePriorityQueue is empty" << endl;
   }

   // Return here if it is the second time around.
   if ( secondTime ) {
      // Empty the queues
      while ( !m_notValidPriorityQueue->isEmpty() ) {
         m_notValidPriorityQueue->dequeue();         
      }
      while ( !m_throughfarePriorityQueue->isEmpty() ) {
         m_throughfarePriorityQueue->dequeue();         
      }
      return StringTable::OK;
   }

   // Now start expanding the non valid priority queue if needed
   // The nodes in m_priorityQueue are nodes that are fully OK
   // to drive to, i.e. that it was possible to drive into the origin
   // node and then to the nodes in m_priorityQueue. The ones in
   // m_normalPriorityQueue should be nodes that can be reached from
   // an origin that is invalid, but are valid themselves.   
   bool normalQueueWasEmpty = m_priorityQueue->isEmpty();
   
   if ( m_priorityQueue->isEmpty() ) {
      mc2dbg << "[CR]: Prioqueue is empty - expanding nonvalid nodes"
             << endl;
      expandNonValidNodes( driverParam, forward );
   } else {
      // Reset all in nonvalid etc.
      while ( !m_notValidPriorityQueue->isEmpty() ) {
         RoutingNode* node = m_notValidPriorityQueue->dequeue();
         mc2dbg << "[CR]: irft - dequeuing node [3] 0x"
                << hex << node->getItemID() << dec << endl;
         //node->reset(m_map);
         // Put the nodes into vector for expansion later...         
         m_invalidNodeVector.push_back(
            make_pair((RoutingConnection*)NULL, node));
         
      }
   }

   RoutingNode* cheapestValid =
      m_normalPriorityQueue->isEmpty() ? NULL :
      m_normalPriorityQueue->dequeue();

   if ( ! m_priorityQueue->isEmpty() ) {
      cheapestValid = NULL;
   }

   if ( m_priorityQueue->isEmpty() &&
        (!m_throughfarePriorityQueue->isEmpty() ) ) {
      // Expand the no througfare nodes
      expandThroughfareNodes( driverParam, forward );
      mc2dbg << "[CR]: Moving throughfare nodes into m_priorityQueue"
             << endl;
      cheapestValid = NULL;
   }
   
   if ( cheapestValid != NULL ) {
      // Check the destinations...
      for ( OrigDestNode* curDest =
               static_cast<OrigDestNode*>(endRoutingNodeList->first());
            curDest != NULL;
            curDest = curDest->next() ) {     
         RoutingNode* realNode = m_map->getNode( curDest->getIndex() );
         if ( realNode->getGradient(m_map) != NULL ) {            
            uint32 extraCost = calcOffsetCostWalk( endRoutingNodeList,
                                                   realNode,
                                                   driverParam,
                                                   forward);
            if ( realNode->getRealCost(m_map) + extraCost <
                 cheapestValid->getRealCost(m_map) ) {
               // It is cheaper to drive to the destination than to the
               // edge of the invalid area.
               mc2dbg << "[CR]: irft - cheaper to drive to goal than out"
                      << endl;
               m_cutOff = realNode->getRealCost(m_map) + extraCost;
            }
         }
      }
   } else {
      // Reset all destinations, but not the cheapest valid.
      for ( OrigDestNode* curDest =
               static_cast<OrigDestNode*>(endRoutingNodeList->first());
            curDest != NULL;
            curDest = curDest->next() ) {
         RoutingNode* realNode = m_map->getNode( curDest->getIndex() );
         // Don't reset the cheapest valid.
         if ( realNode != cheapestValid ) {
            //mc2dbg << "[CR]: irft resetting node [1] "
            //       << hex << curDest->getItemID() << dec << endl;
            //realNode->reset(m_map);
            realNode->setDest(m_map,true);
         }
      }
   }

   // Put the cheapest valid nodes into the real queue.
   // If the normal queue wasn't empty before, all valid nodes are
   // put into the queue.
   // FIXME: Put the other ones that have been reached from the same
   // node into the queue too.
   if ( cheapestValid ) {
      uint32 cheapestValidCost = cheapestValid->getRealCost(m_map);
      RoutingNode* cheapestValidGradient = cheapestValid->getGradient(m_map);
      while ( cheapestValid ) {
         if ( (cheapestValid->getRealCost(m_map) == cheapestValidCost) ||
              (cheapestValid->getGradient(m_map) == cheapestValidGradient) ) {
            mc2dbg << "[CR]: irft putting node 0x" 
                   << hex << cheapestValid->getItemID() << dec
                   << " with cost " << cheapestValid->getRealCost(m_map)
                   << " into m_priorityQueue" << endl;
            m_priorityQueue->enqueue(cheapestValid);
            // Protect the route from calccost dijkstra
            for ( RoutingNode* gradient = cheapestValid;
                  gradient != NULL;
                  gradient = gradient->getGradient(m_map) ) {
               gradient->setVisited(m_map, true);
            }
            //cheapestValid->setVisited(m_map, true);
            if ( cheapestValid->isDest(m_map) ) {
               mc2dbg << "[CR]: irft Cheapestvalid is destination" << endl;
            }
         }
         if ( ! m_normalPriorityQueue->isEmpty() ) {
            cheapestValid = m_normalPriorityQueue->dequeue();
            if ( ! normalQueueWasEmpty ) {
               // All nodes should be enqueued               
               cheapestValidCost = cheapestValid->getRealCost(m_map);
            }
         } else {
            cheapestValid = NULL;
         }
      }
   }

   // Put the origins in the originSet
   set<uint32> originSet;
   for( OrigDestNode* curOrig =
           static_cast<OrigDestNode*>(startRoutingNodeList->first());
        curOrig != NULL;
        curOrig = curOrig->next()) {
      originSet.insert(curOrig->getItemID());
   }
   
   // Clear out and reset the other nodes.
   while ( ! m_normalPriorityQueue->isEmpty() ) {
      RoutingNode* node = m_normalPriorityQueue->dequeue();
      // FIXME: This does not always work. If a node is reset
      //        there can still be nodes that have this node
      //        as gradient. I think the purpose was not to expand
      //        the nodes.
      bool dest = node->isDest(m_map); // Should not be...
      if ( true || node->getGradient(m_map) == NULL ||
           (originSet.find(node->getGradient(m_map)->getItemID())
            == originSet.end()))
      {
         // Testing to use the nodes for expansion later.
         mc2dbg << "[CR]: irft NOT resetting node [2] "
                << hex << node->getItemID() << dec << endl;
      }
      node->setDest(m_map,dest);
   }
   
   // And finally expand the no througfare nodes
   expandThroughfareNodes( driverParam, forward );
   
   // We may have found a destination already so update the cutoff value.
   OrigDestNode* curDestNode =
      static_cast<OrigDestNode*>(endRoutingNodeList->first());
   
   while( curDestNode != NULL ) {
      RoutingNode* curNode = m_map->getNode(curDestNode->getIndex());
      // FIXME: The allowed destinations here should be the ones
      // which it is possible to walk to.
      if ( allowedDests.find(curNode) != allowedDests.end() ) {
         if( curNode->getEstCost(m_map) < m_cutOff ) {
            m_cutOff = curNode->getEstCost(m_map);            
            mc2dbg << "[CR]: irft setting cutoff to " << m_cutOff << endl;
         }
      }
      // Reset the node so that we can find it again.
      //curNode->reset(m_map);
      curNode->setDest(m_map,true);
      curDestNode = curDestNode->next();
   }
   
   if (m_priorityQueue->isEmpty()) {
      if (forward) {
         mc2log << error << "CalcRoute: No valid start routing node" << endl;
         return StringTable::ERROR_NO_VALID_START_ROUTING_NODE;
      }
      else {
         mc2log << error << "CalcRoute: No valid end routing node" << endl;
         return StringTable::ERROR_NO_VALID_END_ROUTING_NODE;
      }
   } else {
      return StringTable::OK;
   }
} // initRouteFirstTime


uint32 
CalcRoute::initRouteBus( Head* startRoutingNodeList,
                         Head* endRoutingNodeList,
                         const RMDriverPref* driverPref,
                         bool forward )
{
   const bool usingCostC = driverPref->getCostC() != 0;
   mc2dbg4 << "CalcRoute::initRouteBus" << endl; 
   m_priorityQueue->updateStartIndex( driverPref->getTime() );
   uint32 startTime = driverPref->getTime();
   uint32 time;

   OrigDestNode* tempNode 
      = static_cast<OrigDestNode*>(startRoutingNodeList->first());
   RoutingNode* curNode;

   while( tempNode != NULL ){
      curNode = m_map->getNode( tempNode->getIndex() );
      if ( (curNode != NULL) && ( curNode->getLineID() == 0 ) ) {
         // Have to start walking

         curNode->setEstCost(m_map, startTime ); 
         curNode->setRealCost(m_map, startTime );

         // Update the connecting nodes to get the right offset
         RoutingConnection* conn = curNode->getFirstConnection(forward);
         RoutingConnectionData* connData;
         float32 dOffset = FLOAT_ANTI_OFFSET(tempNode->getOffset());

         while( conn != NULL ){            
            connData = conn->getData();
            if( ( driverPref->getVehicleRestriction() &
                  connData->getVehicleRestriction(usingCostC) ) != 0 ){

               //RoutingNode* nextNode = m_map->getNode( conn->getIndex() );
               RoutingNode* nextNode = conn->getNode();
               if( nextNode != NULL ){
                  // Shift the cost 14 steps to make the cost in seconds.
                  if( nextNode->getLineID() == 0 ){ // We are still walking
                     time = startTime +
                        (uint32)( ( (driverPref->getCostD()*connData->getCostD()) >> 14 )*dOffset);
                  }
                  else{ // Enter a bus
                     time = startTime + (uint32)(( (driverPref->getCostD()*connData->getCostD()) >> 14 )*dOffset);
                     // Lookup in a table when the next bus leaves.                     
                     time = m_map->getNextDepartureTime( time, nextNode->getItemID(), nextNode->getLineID());
                  }
                  nextNode->setEstCost(m_map, time );
                  nextNode->setRealCost(m_map, time );
                  nextNode->setGradient(m_map, curNode );
                  m_priorityQueue->enqueue( nextNode );
               }
            }
            conn = conn->getNext();
         }
      }
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());      
   }

   if( m_priorityQueue->isEmpty() ) {
      return StringTable::ERROR_NO_VALID_START_ROUTING_NODE;
   }

   // Mark destination nodes if there aren't any we will route
   // to the external nodes
   tempNode = static_cast<OrigDestNode*>(endRoutingNodeList->first());
   while( tempNode != NULL ){
      // Setting a destination
      curNode = m_map->getNode(tempNode->getIndex());
      curNode->setDest(m_map,true);
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }      
   return StringTable::OK;
}

uint32
CalcRoute::initRouteWalking( Head* origin,
                             Head* dest,
                             const RMDriverPref* driverParam,
                             bool forward )
{
   const bool usingCostC = driverParam->getCostC() != 0;
   const uint32 vehRes = driverParam->getVehicleRestriction();
   mc2dbg << "CalcRoute::initRouteWalking" << endl; 

   DEBUG8(
      mc2dbg8 << "origins : " << endl;
      for(OrigDestNode* cur = static_cast<OrigDestNode*>(origin->first());
          cur != NULL;
          cur = static_cast<OrigDestNode*>(cur->suc()) ) {
         ((OrigDestNode*)cur)->dump();
      }
      mc2dbg8 << "destinations : " << endl;
      for(OrigDestNode* cur = static_cast<OrigDestNode*>(dest->first());
          cur != NULL; 
          cur = static_cast<OrigDestNode*>(cur->suc())) {
         ((OrigDestNode*)cur)->dump();
      }
      );

   OrigDestNode* tempNode =
      static_cast<OrigDestNode*>(origin->first());
   while( tempNode != NULL ){
      RoutingNode* curNode = 
         m_map->getNode( tempNode->getIndex() );

      if( curNode != NULL ){
         RoutingConnection* conn = curNode->getFirstConnection( forward );
         float dOffset = FLOAT_ANTI_OFFSET(tempNode->getOffset());

         curNode->setEstCost(m_map,tempNode->getRealCost(m_map));
         curNode->setRealCost(m_map,tempNode->getRealCost(m_map));

         if (conn == NULL) {
            mc2dbg << "node has no connections " << endl;
         }

         while( conn != NULL ){
            RoutingConnectionData* connData = conn->getData();
            
            if( ( driverParam->getVehicleRestriction() &
                  connData->getVehicleRestriction(usingCostC) ) != 0 ){

               uint32 cost = tempNode->getRealCost(m_map) +
                  (uint32)((int32)connData->getCostA(vehRes)*dOffset);
               //RoutingNode* nextNode = m_map->getNode(conn->getIndex());
               RoutingNode* nextNode = conn->getNode();
               
               if (cost < nextNode->getRealCost(m_map)) {
                  nextNode->setRealCost(m_map,cost);
                  nextNode->setEstCost(m_map,cost);
                  /* Should point back to the origdestnode */
                  nextNode->setGradient(m_map,tempNode);
                  m_priorityQueue->enqueue( nextNode );
               }                        
            }
            conn = conn->getNext();
         }
      }

      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }
   
   if( m_priorityQueue->isEmpty() ) {
      mc2log << warn << "No valid startnode found for WALK_ROUTE!!!" << endl;
//      return StringTable::ERROR_NO_VALID_START_ROUTING_NODE;
   }
   
   // Mark destination nodes if there aren't any we will route to the
   // external nodes
   tempNode = static_cast<OrigDestNode*>(dest->first());
   while( tempNode != NULL ){
      RoutingNode* curNode = m_map->getNode( tempNode->getIndex() );      
      curNode->setDest(m_map,true);
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }
   return StringTable::OK;   
}

int
CalcRoute::lookupNodesLowerToHigher( IDPairVector_t& out,
                                     const IDPairVector_t& in)
{
   const int inSize = in.size();
   out.reserve(inSize);
   
   for(int i=0; i < inSize; ++i ) {
      out.push_back(m_map->translateToHigher( in[i] ) );
   }
   return inSize;
}
                                   
int
CalcRoute::lookupNodesHigherToLower( IDPairVector_t& lower,
                                     const IDPairVector_t& higher)
{
   const int inSize = higher.size();
   lower.reserve(inSize);
   
   for(int i=0; i < inSize; ++i ) {
      lower.push_back(m_map->translateToLower(higher[i]));
   }
   return inSize;
}

uint32 
CalcRoute::initRouteOnHigherLevel( Head* startRoutingNodeList,
                                   Head* endRoutingNodeList,
                                   const RMDriverPref* driverParam ){
   // This function should not be used if I'm correct
   mc2dbg4 << "CalcRoute::initRouteOnHigherLevel" << endl; 
   OrigDestNode* tempNode = 
      static_cast<OrigDestNode*>(startRoutingNodeList->first() );

   // Update the priority queue if necessary
   uint32 minCost = MAX_UINT32;
   while( tempNode != NULL ){
      if( tempNode->getRealCost(m_map) < minCost )
         minCost = tempNode->getRealCost(m_map);
      tempNode = (OrigDestNode*)tempNode->suc(); 
   }
   if( minCost != MAX_UINT32 )
      m_priorityQueue->updateStartIndex(minCost);

   tempNode = static_cast<OrigDestNode*>(startRoutingNodeList->first());
   while( tempNode != NULL ){

      // Translate to higher level.
      
      // Index here seems to be the item id of the tempNode.
      uint32 higherID = m_map->translateToHigher( tempNode->getMapID(),
                                                  tempNode->getIndex() );
      
      if( higherID != MAX_UINT32 ) {
      
         RoutingNode* curNode =
            m_map->getNodeFromTrueNodeNumber(higherID);
         
         if (curNode != NULL) {
            // XXX Check if this is correct
            curNode->setEstCost(m_map, tempNode->getRealCost(m_map) ); 
            curNode->setRealCost(m_map, tempNode->getRealCost(m_map) );
            m_priorityQueue->enqueue( curNode );
         }
      }
      else {
         mc2dbg << "[CR]: Node with id " << tempNode->getMapID()
                << ":" << hex << tempNode->getIndex() << dec
                << " not found on map " << getMapID() << endl;
      }

      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }
   
#define TEST_DESTS_ON_HIGHER_LEVEL
#ifdef  TEST_DESTS_ON_HIGHER_LEVEL
   // If TEST_DESTS_ON_HIGHER_LEVEL is defined we mark the destination
   // nodes as destinations and stop calcing dijkstra when all
   // destinations are reached. 
   
   /// Check this if it is correct.... Seems to work.
   
   // Mark destination nodes
   tempNode = (OrigDestNode*) endRoutingNodeList->first();
   while( tempNode != NULL ){

      uint32 higherID = m_map->translateToHigher( tempNode->getMapID(),
                                                  tempNode->getIndex() );
      
      
      mc2dbg2 << "Looking up " << tempNode->getMapID() << ":"
              << hex << tempNode->getIndex()
              << dec << " as dest..." << flush;
      
      if( higherID != MAX_UINT32 ) {
         mc2dbg8 << "found "
                 << getMapID() << hex << ":"
                 << higherID << dec << endl;
         mc2dbg8 << " cost  " << tempNode->getRealCost(m_map) << endl;
         
         RoutingNode* curNode =
            m_map->getNodeFromTrueNodeNumber( higherID );
         
         if( curNode != NULL ){
            curNode->setDest(m_map,true);
            //curNode->setRealCost(m_map, tempNode->getEstCost(m_map) );
         }
      }
      else {
//         mc2dbg2 << "NULL" << endl;
      }
      
      tempNode = (OrigDestNode*)tempNode->suc();      
   }
#endif
   return StringTable::OK;
}

////////////////////////////////////////////////////////////
// Help methods for initialization 
////////////////////////////////////////////////////////////

void 
CalcRoute::expandNonValidNodes( const RMDriverPref* driverParam,
                                bool forward )
{
   const bool usingCostC = driverParam->getCostC() != 0;
   mc2dbg4 << "CalcRoute::expandNonValidNodes" << endl;
   FileDebugCalcRoute::writeComment("expandNonValidNodes");
   // Expand the non valid heap until no items are left in it.
   uint32 localCutOff = MAX_UINT32;
   
   if ( !m_normalPriorityQueue->isEmpty() ) {
      RoutingNode* node = m_normalPriorityQueue->dequeue();
      localCutOff = node->getRealCost(m_map);
      m_normalPriorityQueue->enqueue(node);      
   }

   mc2dbg << "[CR]: Localcutoff = " << localCutOff << endl;
   

   while( !m_notValidPriorityQueue->isEmpty() ) {
      uint32 nextLocalCutOff = localCutOff;
      RoutingNode* curNode = 
         m_notValidPriorityQueue->dequeue();
     
      if( curNode != NULL ) {
         if( curNode->isDest(m_map) ) {            
            mc2dbg << "[CR]: Found destination in expandNonVal: "
                   << hex << curNode->getItemID() << dec
                   << " cost = " << curNode->getRealCost(m_map)
                   << endl;
            continue;
         }
         RoutingConnection* curConnection = 
            curNode->getFirstConnection( forward );
         while( curConnection != NULL ) {
            RoutingConnectionData* curConnData = 
               curConnection->getData();
            
            // Penalize the moving of the car
            uint32 cost = curNode->getRealCost(m_map) +
               calcConnectionCostWalk(driverParam, curConnData);

            RoutingNode* nextNode = curConnection->getNode();
            
            if( cost < nextNode->getRealCost(m_map) &&
                cost <= localCutOff ) {  
               nextNode->setEstCost(m_map, cost );
               nextNode->setRealCost(m_map, cost );
               nextNode->setGradient(m_map, curNode );
               const bool canWalk =
                  curConnData->getVehicleRestriction(usingCostC) &
                  ItemTypes::pedestrian;

               const bool vehicleOK = driverParam->getVehicleRestriction()
                  & curConnData->getVehicleRestriction(usingCostC);
               const bool nextNodeValid =
                  !NOT_VALID(nextNode->getRestriction());
               
               // if it is possible to drive to the node update
               // the normal or the throughfare heap.
               if( ( vehicleOK && nextNodeValid )  ||
                   canDriveOnSegment( nextNode, driverParam,
                                      forward, true) ) {
                  if( HAS_NO_THROUGHFARE( nextNode->getRestriction() ) ){
                     mc2dbg2 << "Adding a node m_throughfarePriorityQueue "
                            << hex << nextNode->getItemID() << dec << endl;
                     m_throughfarePriorityQueue->enqueue( nextNode );
                  } else {
                     mc2dbg2 << "Adding a node m_normalPriorityQueue "
                            << hex << nextNode->getItemID() << dec << endl;
                     m_normalPriorityQueue->enqueue(nextNode);
                     nextNode->setVisited(m_map, true);
                  }
                  // Set the local cutoff to use for next node.
                  nextLocalCutOff =
                     nextNode->getRealCost(m_map);
               } else { // keep on expanding the heap.
                  mc2dbg2 << "Adding a node m_notValidPriorityQueue "
                          << hex << nextNode->getItemID() << dec << endl;
                  // Only expand if it is possible to walk.
                  if ( canWalk ) {
                     m_notValidPriorityQueue->enqueue( nextNode );
                  }
               }
            }
            curConnection = curConnection->getNext();
         }
      }
      // Set new local cutoff when all connections from current node
      // are done.
      localCutOff = nextLocalCutOff;
   }
} // expandNonValidNodes


void 
CalcRoute::expandThroughfareNodes( const RMDriverPref* driverParam,
                                   bool forward )
{
   const bool usingCostC = driverParam->getCostC() != 0;
   mc2dbg << "CalcRoute::expandThroughfareNodes" << endl;
   FileDebugCalcRoute::writeComment("expandThroughfareNodes");
   while( !m_throughfarePriorityQueue->isEmpty() ){
      RoutingNode* curNode = 
         m_throughfarePriorityQueue->dequeue();
      curNode->setVisited(m_map, true);
      if (curNode->isDest(m_map)) {
         mc2dbg << "[CR]: Found destination in expandThroughfareNodes"
                << endl;
         // Setting the cutoff a bit too high...
         m_cutOff = curNode->getRealCost(m_map) + getMinCost(curNode, driverParam);
         continue;
      }
      
      RoutingConnection* curConnection = 
         curNode->getFirstConnection( forward );

      while( curConnection != NULL ) {
         RoutingConnectionData* curConnData = 
            curConnection->getData();

         bool validNode = false;
         
         uint32 cost = curNode->getRealCost(m_map) +
            calcConnectionCost( driverParam, curConnData);
         RoutingNode* nextNode = curConnection->getNode();
            
         if ((driverParam->getVehicleRestriction() & 
              curConnData->getVehicleRestriction(usingCostC)) != 0) {
            if ( ! NOT_VALID( nextNode->getRestriction() ) ) {
               if( cost < nextNode->getRealCost(m_map) ||
                   ! nextNode->isVisited(m_map) ){
                  nextNode->setRealCost(m_map, cost );
                  nextNode->setEstCost(m_map, cost );
                  nextNode->setGradient(m_map, curNode );
                  nextNode->setVisited(m_map, true);
                  
                  if( HAS_NO_THROUGHFARE( nextNode->getRestriction() ) ) {
                     // We're still inside
                     m_throughfarePriorityQueue->enqueue( nextNode );
                     validNode = true;
                  } else {
                     // We're outside.
                     m_priorityQueue->enqueue( nextNode );
                     validNode = true;
                  }                  
               }
            } 
         }
      
         if ( ! validNode ) {
            if ( cost < nextNode->getRealCost(m_map) &&
                 ! nextNode->isVisited(m_map) ) {
               nextNode->setRealCost(m_map, cost );
               nextNode->setEstCost(m_map, cost );
               nextNode->setGradient(m_map, curNode );
               // Visited == false means that is isn't valid.
               nextNode->setVisited (m_map, false );
               if ( nextNode->getIndex() != MAX_UINT32 ) {
                  m_invalidNodeVector.push_back(
                     make_pair(curConnection, nextNode));
               }
            }
         }
  
         curConnection = curConnection->getNext();
      }
   }
   m_throughfarePriorityQueue->reset();
}  // expandThroughfareNodes


////////////////////////////////////////////////////////
// Code for calculating the routes
////////////////////////////////////////////////////////

// The order of these have changed to avoid long compile times.

inline void
CalcRoute::calcCostDijkstra(const RMDriverPref* driverParam,
                            Head* destination,
                            bool forward,
                            bool routeToAll)
{
   uint32 startTime = TimeUtility::getCurrentTime();
   // Do nothing with the time, but force update
   trickTheOptimizer(startTime);
   
   uint32 costA = driverParam->getCostA();
   uint32 costB = driverParam->getCostB();
   uint32 costC = driverParam->getCostC();

   if ( costA && !costB && !costC ) {
      if ( costA == 1 ) {
         calcCostDijkstra(driverParam, 1, 0, 0, destination,
                          forward, routeToAll);
      } else {
         calcCostDijkstra(driverParam, costA, 0, 0, destination,
                          forward, routeToAll);
      }
   } else if ( !costA && costB && !costC ) {
      if ( costB == 1 ) {
         calcCostDijkstra(driverParam, 0, 1, 0, destination,
                          forward, routeToAll);
      } else {
         calcCostDijkstra(driverParam, 0, costB, 0, destination,
                          forward, routeToAll);
      }
   } else if ( (!costA) && (!costB) && costC ) {
      if ( costC == 1 ) {
         calcCostDijkstra(driverParam, 0, 0, 1, destination,
                          forward, routeToAll);
      } else {
         calcCostDijkstra(driverParam, 0, 0, costC, destination,
                          forward, routeToAll);
      }
   } else {
      calcCostDijkstra(driverParam, costA, costB, costC,
                       destination, forward, routeToAll);
   }
   uint32 stopTime = TimeUtility::getCurrentTime();
   trickTheOptimizer(stopTime);
   mc2dbg << "CR:ccd dijkstratime " <<
      (stopTime - startTime) << endl;
}


inline void
CalcRoute::calcCostDijkstra(const RMDriverPref* driverParam,
                            dpCost_t costA,
                            dpCost_t costB,
                            dpCost_t costC,
                            Head* destination,
                            bool forward,
                            bool routeToAll)
{
   const bool estimate = destination->cardinal() < 3;
   const uint32 restriction = driverParam->getVehicleRestriction();
   const bool walking  = IS_WALKING(restriction);
   if ( estimate ) {
      if ( walking ) {
         calcCostDijkstra(driverParam,
                          costA, costB, costC, destination,
                          forward, routeToAll,
                          true, true);
      } else {
         calcCostDijkstra(driverParam,
                          costA, costB, costC, destination,
                          forward, routeToAll,
                          true, false);
      }
   } else {
      if ( walking ) {
         calcCostDijkstra(driverParam,
                          costA, costB, costC, destination,
                          forward, routeToAll,
                          false, true);
      } else {
         calcCostDijkstra(driverParam,
                          costA, costB, costC, destination,
                          forward, routeToAll,
                          false, false);
      }
   }
}


inline void
CalcRoute::calcCostDijkstra(const RMDriverPref* driverParam,
                            dpCost_t costA,
                            dpCost_t costB,
                            dpCost_t costC,
                            Head* destination,
                            bool forward,
                            bool routeToAll,
                            bool estimate,
                            bool walking,
                            bool throughfareOK)
{ 
   const bool underviewMap = !MapBits::isOverviewMap( m_map->getMapID() );
   const bool usingCostC = costC != 0;
   mc2dbg4 << "calcCostDijkstra, map " << m_map->getMapID() 
          << ", forward " << BP(forward) << endl;
   FileDebugCalcRoute::writeComment("calcCostDijkstra");
   
   int destNbr = 0; // For debug
   int nbrDestsLeft = 1;
   int nbrOriginalDests = nbrDestsLeft;
   uint32 cutOff = m_cutOff;
   if ( routeToAll ) {
      // Make routes to all destinations, please.
      nbrDestsLeft = destination->cardinal();
      nbrOriginalDests = nbrDestsLeft;
      if ( MapBits::isOverviewMap(m_map->getMapID()) ) {
         // See if we can speed things up by looking for half.

      }
      // Cannot use the cutoff here
      // FIXME: We can use cutoff when routing higher level.
      if ( m_map->getMapID() < 0x80000000 )
         cutOff = MAX_UINT32;
   }
   mc2dbg << "CALCCOSTDIJKSTRA: Cut off = " << cutOff << endl;
   
   // mask with restriction data
   const uint32 restriction = driverParam->getVehicleRestriction();

   // Creation of nodes should only be possible in the map.
   RoutingNode* destNodes = m_map->createDestNodes( destination->cardinal() );

   int nbrDest = 0;
   uint32 tmpCutOff = MAX_UINT32;
   DEBUG1(int nbrDequeued = 0);
   DEBUG1(int nbrConnections = 0);

   if ( estimate ) {
      mc2dbg << "[CR]: ccd - estimation _will_ be used" << endl;
   } else {
      mc2dbg << "[CR]: ccd - estimation will NOT be used" << endl;
   }
   
   while ( ! m_priorityQueue->isEmpty() ) {
      RoutingNode* curNode = m_priorityQueue->dequeue();
      // For tricks later
      curNode->setVisited(m_map, true); // (Means that we can drive here)
      DEBUG1(++nbrDequeued);

      // Works for the lifo that sorts if the heap contains a destination.
      
      if ( MC2_LIKELY( !curNode->isDest(m_map) ) ) {
      } else {
         mc2dbg8 << "calcCostDijkstra found destination " << hex
                << curNode->getItemID() << dec << endl;
         if (curNode->getIndex() == MAX_UINT32) { // Its a strange node
            mc2dbg8 << "...again!" << endl;
            if ( nbrDestsLeft == 1 && destNbr == 0 ) {
               tmpCutOff = MIN(tmpCutOff, curNode->getRealCost(m_map));
            }
            // Stop if we have found all destinations.
            ++destNbr;
            mc2dbg8 << "Found destination " <<  destNbr << " of "
                   << destination->cardinal() << endl;
            
            if ( --nbrDestsLeft == 0 ) {
               mc2dbg << "calcCostDijkstra ends after finding "
                      << destNbr << " destinations" << endl;
               if ( destNbr == 1 ) {
                  mc2dbg << "Found destination "
                         << hex << curNode->getItemID() << dec
                         << " has cost "
                         << curNode->getRealCost(m_map) << endl;
               }
               DEBUG1(mc2dbg << "nbrDequeued = " << nbrDequeued << endl;);
               DEBUG1(mc2dbg << "nbrConnections = " << nbrConnections
                             << endl;);
               m_cutOff = tmpCutOff;
               delete[] destNodes;
               return;
            }
         } else { // Normal node
            RoutingNode* tempNode = NULL;
            for( int i=0;i<nbrDest;i++ ){
               if( destNodes[i].getItemID() == curNode->getItemID() ){
                  tempNode = &destNodes[i];
                  break;
               }
            }
          
            uint32 extraCost = calcOffsetCost( destination,
                                               curNode,
                                               driverParam,
                                               forward );
            mc2dbg8 << "extracost = " << extraCost << endl;
            
            if( extraCost != MAX_UINT32 ){
               
               if (tempNode != NULL) { // Found a node
                  if (curNode->getRealCost(m_map) + extraCost <
                      tempNode->getRealCost(m_map)) {
                     tempNode->setEstCost(m_map, curNode->getEstCost(m_map) +
                                                 extraCost );
                     // Added 2002-10-30
                     tempNode->setRealCost(m_map, curNode->getRealCost(m_map) +
                                                  extraCost);
                     mc2dbg2 << curNode->getEstCost(m_map) + extraCost<< endl;
                     m_priorityQueue->enqueue(tempNode);
                  }
               } else { // Have to add a new one.
                  tempNode = &destNodes[nbrDest++];
                  tempNode->setIndex( MAX_UINT32);
                  tempNode->setItemID( curNode->getItemID() );
                  
                  // Added 2002-10-30
                  tempNode->setRealCost(m_map,
                                        curNode->getRealCost(m_map) +
                                        extraCost);
                  tempNode->setEstCost(m_map,
                                       curNode->getEstCost(m_map) +
                                       extraCost );
                  tempNode->setDest(m_map,true);
                  m_priorityQueue->enqueue(tempNode);
               }
            } else {
               mc2log << error
                      << "CalcRoute::calcOffsetCost returned MAX_UINT32"
                      << endl;
            }
         }
      }

      // I think that the following if should be removed soon.
      const uint32 curCost = curNode->getRealCost(m_map);
//        if ( MC2_UNLIKELY( curCost > cutOff ) ) {
//           // Next node, please. If we know that the heap is really
//           // ordered, we can exit the function here.
//           continue;
//        }
      
      /* Testing to enqueue the node again */
      /*else*/  {
         const RoutingConnection* tmpConnection =
            curNode->getFirstConnection(forward);

         while( tmpConnection != NULL ) {
            const RoutingConnectionData* const tmpConnectionData =
               tmpConnection->getData();

            if (restriction &
                tmpConnectionData->getVehicleRestriction(usingCostC)) {
               
               RoutingNode* nextNode = tmpConnection->getNode();

               // Check if the next node has ok restrictions or
               // we are walking.
               // If no throughfare is OK only nodes with no throughfare
               // are ok.              
               if(
                  ((!throughfareOK) &&
                   HAS_NO_RESTRICTIONS(nextNode->getRestriction() ) ) ||
                  ( throughfareOK &&
                    HAS_NO_THROUGHFARE(nextNode->getRestriction() ) ) ||
                   walking ) {
                  // Calculate a new cost with driver preferences
                  // FIXME: Try to find a way to avoid multiplications
                  //        and additions if one or many of the costs
                  //        are zero.
                  uint32 tmpCost =  
                     curCost + calcConnectionCost(costA, costB, costC, 0,
                                                  restriction,
                                                  tmpConnectionData);

                  DEBUG1(++nbrConnections);
                  
                  uint32 est = 0;

                  if( ( ( tmpCost < nextNode->getRealCost(m_map) ) ||
                        (!nextNode->isVisited(m_map) ) )  &&
                        ( tmpCost <= cutOff ) ) {
                     nextNode->setRealCost(m_map, tmpCost );
                     // Do not estimate if there are too many destinations.,
                     if ( estimate ) {
                        est = estimateDistToDest(nextNode,
                                                 destination,
                                                 costA,
                                                 costB,
                                                 costC,
                                                 0);                  
                     }
                     nextNode->setGradient(m_map, curNode );
                     nextNode->setEstCost(m_map, tmpCost + est);
                     nextNode->setVisited(m_map,  true );
                     m_priorityQueue->enqueue( nextNode );
                  }
               } else {
                  goto invalidnode;
               }
            } else {               
              invalidnode:
               if ( underviewMap ) {
                  // Do not punish here.
                  // It might be possible not to set the costs etc.
                  // and store them in the vector instead.
                  uint32 tmpCost =
                     curCost + calcConnectionCost(costA, costB, costC, 0,
                                                  restriction,
                                                  tmpConnectionData);
                  
                  RoutingNode* nextNode = tmpConnection->getNode();
                  
                  if( ( tmpCost < nextNode->getRealCost(m_map) ) &&
                      ( tmpCost < cutOff ) &&
                      !nextNode->isVisited(m_map) ) {
                     nextNode->setRealCost(m_map, tmpCost );
                     nextNode->setEstCost(m_map, tmpCost );
                     nextNode->setGradient(m_map, curNode );
                     // Visited == false means that is isn't valid.
                     nextNode->setVisited (m_map, false );
                     if ( nextNode->getIndex() != MAX_UINT32 ) {
                        m_invalidNodeVector.push_back(
                           make_pair(tmpConnection, nextNode));
                     }
                  }
               }
            }
            tmpConnection = tmpConnection->getNext();
         } // end while (tmpConnection != NULL)
      } // end else
   } // end while (!m_priorityQueue->isEmpty())
   delete [] destNodes;
   mc2dbg << "calcCostDijkstra ends - heap empty" << endl;
   mc2dbg << "[CR]: Nbr dests left: " << nbrDestsLeft << " of "
          << nbrOriginalDests << endl;
   DEBUG1(mc2dbg << "nbrDequeued = " << nbrDequeued << endl);
   DEBUG1(mc2dbg << "nbrConnections = " << nbrConnections << endl);
   mc2dbg << "Map size = " << m_map->getNbrNodes() << endl;
} // calcCostDijkstra

                            
inline void 
CalcRoute::calcCostExternalDijkstra(const RMDriverPref* driverParam,
                                    dpCost_t costA,
                                    dpCost_t costB,
                                    dpCost_t costC,
                                    bool routeToHigherLevel,
                                    bool forward,
                                    bool estimate,
                                    Head* allDestinations)   
{
   const bool usingCostC = driverParam->getCostC() != 0;

   DEBUG1(int nbrDequeued = 0);
   mc2dbg << "calcCostExternalDijkstra, map " << m_map->getMapID()
          << ", forward " << BP(forward) << endl;
   
   if ( routeToHigherLevel)
      mc2dbg << "[CR] cced will route to higher level only " << endl;
   FileDebugCalcRoute::writeComment("calcCostExternalDijkstra");

   mc2dbg << "CALCCOSTEXTERNALDIJKSTRA: Cut off = " << m_cutOff
          << endl;
   
   // Get the restrictions
   uint32 restriction = driverParam->getVehicleRestriction();

   if ( estimate ) {
      mc2dbg << "[CR]: cced - estimation _will_ be used" << endl;
   } else {
      mc2dbg << "[CR]: cced - estimation will NOT be used" << endl;
   }

   while ( !m_priorityQueue->isEmpty() ){
      RoutingNode* curNode = m_priorityQueue->dequeue();
      DEBUG1(++nbrDequeued);
      const uint32 curCost = curNode->getRealCost(m_map);
      DEBUG8(mc2dbg << "[CR]: cced - dequeued node 0x"
             << hex << curNode->getItemID() << dec
             << " with cost "
             << curCost << endl);
     
      const RoutingConnection* tmpConnection =
         curNode->getFirstConnection( forward );

      while( tmpConnection != NULL ) {         

         const RoutingConnectionData* tmpConnectionData =
            tmpConnection->getData();

         if (restriction &
             tmpConnectionData->getVehicleRestriction(usingCostC) ) {

            RoutingNode* newNode = tmpConnection->getNode();
            
            const uint32 newNodeCost = newNode->getRealCost(m_map);
            
            if( (curCost < newNodeCost) &&
                HAS_NO_RESTRICTIONS( newNode->getRestriction() ) ) {
               const bool curNodeOnLowLevel =
                  IS_LOWER_LEVEL( curNode->getItemID() );
               if ( shouldIncludeNode( newNode, curNodeOnLowLevel,
                                       routeToHigherLevel) ) {

                  // Calculate a new cost with driver preferences
                  uint32 tmpCost =  curCost + 
                     costA * tmpConnectionData->getCostA(restriction) +
                     costB * tmpConnectionData->getCostB(restriction) +
                     costC * tmpConnectionData->getCostC(restriction);

                  uint32 est = 0;
                  if ( estimate ) {
                     est = estimateDistToDest(newNode,
                                              allDestinations,
                                              costA,
                                              costB,
                                              costC,
                                              0);                  
                  }
                  
                  if( ( tmpCost < newNodeCost ) &&
                      ( (tmpCost + est) <= m_cutOff ) ) {

                     newNode->setRealCost(m_map, tmpCost );
                     // Set the real cost to the same as est.
                     // We will route the whole map anyway.
                     newNode->setEstCost(m_map, tmpCost );
                     m_priorityQueue->enqueue( newNode );
                     newNode->setGradient(m_map, curNode );
                  }
               } else {
                  // Save till later - it is too slow.
                  //m_outsideHeap->enqueue(curNode);
               }
            }
         } 
         tmpConnection = tmpConnection->getNext();         
      }
   }
   DEBUG1(mc2dbg << "[CR]: calcCostExternalDijkstra - nbrDequeued = "
          << nbrDequeued << endl);
  
} // calcCostExternalDijkstra


inline void
CalcRoute::calcCostExternalDijkstra(const RMDriverPref* driverParam,
                                    dpCost_t costA,
                                    dpCost_t costB,
                                    dpCost_t costC,
                                    bool routeToHigherLevel,
                                    bool forward,
                                    Head* allDestinations)
{
   // Only use estimation when there are few destinations 
   // and there is a cutoff.
   const bool estimate = (allDestinations->cardinal() < 5) && 
      (m_cutOff != MAX_UINT32);
   if ( estimate ) {
      calcCostExternalDijkstra(driverParam, 
                               costA,
                               costB,
                               costC,
                               routeToHigherLevel,
                               forward, true, allDestinations);
   } else {
      calcCostExternalDijkstra(driverParam, 
                               costA,
                               costB,
                               costC,
                               routeToHigherLevel,
                               forward, false, allDestinations);
   }
}

void
CalcRoute::calcCostExternalDijkstra(const RMDriverPref* driverParam,
                                    bool routeToHigherLevel,
                                    bool forward,
                                    Head* allDestinations)
{
   uint32 startTime = TimeUtility::getCurrentTime();
   trickTheOptimizer(startTime);
   uint32 costA = driverParam->getCostA();
   uint32 costB = driverParam->getCostB();
   uint32 costC = driverParam->getCostC();
  
   if ( costA && !costB && !costC ) {
      if ( costA == 1 ) {
         calcCostExternalDijkstra(driverParam, 1, 0, 0,
                                  routeToHigherLevel, forward,
                                  allDestinations);
      } else {
         calcCostExternalDijkstra(driverParam, costA, 0, 0, 
                                  routeToHigherLevel, forward,
                                  allDestinations);
      }
   } else if ( !costA && costB && !costC ) {
      if ( costB == 1 ) {
         calcCostExternalDijkstra(driverParam, 0, 1, 0, 
                                  routeToHigherLevel, forward,
                                  allDestinations);
      } else {
         calcCostExternalDijkstra(driverParam, 0, costB, 0, 
                                  routeToHigherLevel, forward,
                                  allDestinations);
      }
   } else if ( (!costA) && (!costB) && costC ) {
      if ( costC == 1 ) {
         calcCostExternalDijkstra(driverParam, 0, 0, 1,
                                  routeToHigherLevel, forward,
                                  allDestinations);
      } else {
         calcCostExternalDijkstra(driverParam, 0, 0, costC, 
                                  routeToHigherLevel, forward,
                                  allDestinations);
      }
   } else {
      calcCostExternalDijkstra(driverParam, costA, costB, costC,
                               routeToHigherLevel, forward,
                               allDestinations);
   }
   uint32 stopTime = TimeUtility::getCurrentTime();
   trickTheOptimizer(stopTime);
   mc2dbg << "RM:CR:calcCostExternalDijkstratime = " << (stopTime-startTime)
          << endl;
}

///////////////////////////////////////////////////////
// Help method for the calccost routines
////////////////////////////////////////////////////////

OrigDestNode*
CalcRoute::getOrigDestNode( Head* origDests,
                            uint32 nodeID )
{
   OrigDestNode* foundNode = NULL;
   for( OrigDestNode* curNode =
           static_cast<OrigDestNode*>(origDests->first());
        curNode != NULL;
        curNode = static_cast<OrigDestNode*>(curNode->suc())) {
      if ( curNode->getItemID() == nodeID ) {
         foundNode = curNode;
         break;
      }       
   }
   return foundNode;
}

OrigDestNode*
CalcRoute::getOrigDestNode( Head* origDests,
                            const RoutingNode* realNode )
{
   return getOrigDestNode(origDests, realNode->getItemID());
}

uint32
CalcRoute::calcOffsetCost( Head* destination,
                           RoutingNode* node,
                           const RMDriverPref* driverParam,
                           bool forward )
{
   // This differs from getOrigDestNode because the index is used
   // and then looked up in the map. Must check if that is necessary.
   // The lookup makes it possible to put an OrigDestNode into the
   // node field though.
   OrigDestNode* destNode = NULL;
   OrigDestNode* tempNode =
      static_cast<OrigDestNode*>( destination->first() );
   while( tempNode != NULL ) {
      RoutingNode* curNode = m_map->getNode( tempNode->getIndex() );
      if ( curNode->getItemID() == node->getItemID() ) {
         destNode = tempNode;
         break;
      }        
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }

   uint32 extraCost = MAX_UINT32;
   
   if (destNode != NULL) {
      
      RoutingNode* curNode = m_map->getNode( destNode->getIndex() );
      extraCost = getMinCost( curNode, driverParam, forward ); 
      if( extraCost != MAX_UINT32 ){
         if( !forward ) {
            extraCost =
               uint32(extraCost*FLOAT_ANTI_OFFSET(destNode->getOffset()));
         } else {
            extraCost = uint32(extraCost*
                               FLOAT_OFFSET(destNode->getOffset()));
         }
      }
   }
   return extraCost;
} // calcOffsetCost


uint32
CalcRoute::calcOffsetCostWalk( Head* destinations,
                               RoutingNode* node,
                               const RMDriverPref* driverParam,
                               bool forward)
{
   // Get the OrigDestNode
   OrigDestNode* destNode = getOrigDestNode(destinations, node);
   uint32 costSum = 0;
   // Cost A is always needed.
   const uint32 costA = getMinCost( node, driverParam->getVehicleRestriction(),
                                    1, 0, 0 );
   
   if ( costA == MAX_UINT32 ) {
      return MAX_UINT32;
   }
   
   if ( driverParam->getCostA() ) {
      // Add costA with penalty
      costSum += uint32(costA *
                        RouteConstants::WALK_FACTOR *
                        driverParam->getCostA());
   }
   if ( driverParam->getCostB() || driverParam->getCostC() ) {
      // Add costB and costC with walking speed
      costSum += (driverParam->getCostB() + driverParam->getCostC()) *
         getWalkingCostBFromA(costA);
   }

   // FIXME: Check if this is ok when we route backwards.
   if ( forward ) {
      return uint32(costSum*FLOAT_OFFSET(destNode->getOffset()));
   } else {
      return uint32(costSum*FLOAT_ANTI_OFFSET(destNode->getOffset()));
   }
}


////////////////////////////////////////////////////////
// Read the result
////////////////////////////////////////////////////////


uint32 
CalcRoute::readResult(Head* origin,
                      Head* destination,
                      SubRouteList* incomingList,
                      SubRouteList* resultList,
                      const RMDriverPref* driverParam,
                      bool forward,
                      bool calcCostSums,
                      bool readToExternal)
{
   const bool usingCostC = driverParam->getCostC() != 0;
   DEBUG2( mc2dbg2 << "CalcRoute::readResult" << endl );
   FileDebugCalcRoute::writeComment("readResult");
   // CalcCost from origin i.e. read result from destination
   Head* useDests;
   Head* useOrigins;
   OrigDestNode* startNode;   
   if ( forward ) {
      useDests = destination;
      useOrigins = origin;
      startNode = static_cast<OrigDestNode*>(destination->first());
   } else {
      useDests = origin;
      useOrigins = destination;
      startNode = static_cast<OrigDestNode*>(origin->first());
   }

   // Don't use the lists - segfault instead.
   destination = NULL;
   origin = NULL;

   // Don't check for no throughfare on overview maps. We cannot
   // stop there, I think.
   if ( !MapBits::isOverviewMap( m_map->getMapID() ) ) {
      // FIXME: Expand the throughfare nodes if all destinations are not found.
      set<uint32> itemIDs;
      for ( OrigDestNode* tempOrigDest = startNode;
            tempOrigDest != NULL;
            tempOrigDest = tempOrigDest->next() ) {
         RoutingNode* realNode = m_map->getNode(tempOrigDest->getIndex() );
         if ( realNode->getRealCost(m_map) != MAX_UINT32 &&
              realNode->isVisited(m_map) ) {
            itemIDs.insert(MapBits::nodeItemID(realNode->getItemID() ) );
         }
      }
      
      mc2dbg << "[CR]: Nbr itemids found = " << itemIDs.size()
             << " of " << (useDests->cardinal() >> 1) << endl;
      int nbrNT = 0;
      m_priorityQueue->reset();
      if ( itemIDs.size() < uint32(useDests->cardinal() >> 1 ) ) {
         // Find out which nodes were rejected because of NT.
         for ( uint32 i = 0; i < m_invalidNodeVector.size(); ++i ) {
            const RoutingConnection* connection =
               m_invalidNodeVector[i].first;
            RoutingNode* node = m_invalidNodeVector[i].second;
            if ( node->isVisited(m_map) ) {
               // Not invalid anymore.
               continue;
            }
            if ( connection == NULL ) {
               // Invalid connection
               continue;
            }
            if ( HAS_NO_THROUGHFARE(node->getRestriction()) ) {
               // Has no throughfare. Check the connection
               RoutingConnectionData* data = connection->getData();
               if ( data->getVehicleRestriction(usingCostC) &
                    driverParam->getVehicleRestriction()) {
                  // ok
                  node->setVisited(m_map, true);
                  m_priorityQueue->enqueue(node);
                  nbrNT++;
               }
            }
         }
      }      
      if ( ! m_priorityQueue->isEmpty() ) {
         mc2dbg << "[CR]: Expanding " << nbrNT
                << " no throughfare nodes" << endl;
         calcCostDijkstra(driverParam,
                          driverParam->getCostA(),
                          driverParam->getCostB(),
                          driverParam->getCostC(),
                          useDests,
                          forward,
                          false, // Routetoall, fixme
                          false, // Don't estimate, fixme.
                          false, // Cannot be walking
                          true); // Troughfare is ok.
      } else {
         mc2dbg << "[CR]: No no-throughfare nodes" << endl;
      }
   }
  
   
   // Read result first. There may be valid routes...
   // Change this into reading all the routes if there should be many.
   set<uint32> validDestinationsRead;
   if ( startNode != NULL ) {
      uint32 minCost = MAX_UINT32;
      RoutingNode* bestNode = NULL;
      OrigDestNode* bestOrigDestNode = NULL;
      for ( OrigDestNode* tempNodeItem = startNode;
            tempNodeItem != NULL;
            tempNodeItem = tempNodeItem->next()) {
         
         RoutingNode* curNode =
            m_map->getNode(tempNodeItem->getIndex());

         mc2dbg8 << "[CR]: Node " << hex
                << curNode->getItemID() << dec << " has cost "
                << curNode->getRealCost(m_map) << endl;
         // Use the offset.
         uint32 extraCost = calcOffsetCost( useDests,
                                            curNode,
                                            driverParam,
                                            forward );
         if ( curNode->getRealCost(m_map) != MAX_UINT32 &&
              curNode->isVisited(m_map) ) {
            validDestinationsRead.insert(curNode->getItemID());
            if (curNode->getRealCost(m_map) + extraCost < minCost) {
               bestNode = curNode;
               bestOrigDestNode = tempNodeItem;
               minCost = curNode->getRealCost(m_map) + extraCost;
               mc2dbg8 << "[CR]: Found a bestnode " << endl;           
            }
         }
      }
      
      // Found a destination on this map
      if (bestNode != NULL) {
         // Update the cutOff value
         if (minCost < m_cutOff) {
            m_cutOff = minCost;
         }
         
         // Fills this subroute to the resultlist
         CompleteRMSubRoute* subRoute =
            readResultFromDestination(incomingList,
                                      resultList,
                                      bestNode,
                                      NULL,
                                      driverParam,
                                      forward,
                                      bestOrigDestNode->getOffset(),
                                      calcCostSums);
         
         // Have to update start/end-offset and
         if ( ! MapBits::isOverviewMap(m_map->getMapID()) ) {
            if (subRoute != NULL) {
               insertStateElement(subRoute,
                                  driverParam,
                                  forward);
            }
         }
      }
   }

   // I think we're done now if it is an overview map.
   if ( MapBits::isOverviewMap( m_map->getMapID() ) ) {
      return StringTable::OK;
   }
   
   OrigDestNode* tempNodeItem = startNode;
   // Map for the destinations that have not been found already.
   map<uint32, RoutingNode*> walkDests; // Sorted by ID.   
   // Map for the destination that have been found.
   map<uint32, RoutingNode*> carDests;  // Sorted by ID.
   
   
   tempNodeItem = startNode;
   while (tempNodeItem != NULL) {
      RoutingNode* curNode = m_map->getNode(tempNodeItem->getIndex());
      // Enqueue all not visited nodes.
      if (! curNode->isVisited(m_map) ) {
         walkDests.insert(pair<uint32,RoutingNode*>(curNode->getItemID(),
                                                    curNode));
      } else {
         carDests.insert(pair<uint32,RoutingNode*>(curNode->getItemID(),
                                                   curNode));
      }
      tempNodeItem = static_cast<OrigDestNode*>(tempNodeItem->suc());
   }

   // Do not do tricks if it is an overview map.
   if ( ! MapBits::isOverviewMap(m_map->getMapID() ) ) {
      set<uint32> itemIDs;
      for ( OrigDestNode* tempOrigDest = startNode;
            tempOrigDest != NULL;
            tempOrigDest = tempOrigDest->next() ) {
         RoutingNode* realNode = m_map->getNode(tempOrigDest->getIndex() );
         if ( realNode->getRealCost(m_map) != MAX_UINT32 &&
              realNode->isVisited(m_map) ) {
            itemIDs.insert(MapBits::nodeItemID(realNode->getItemID() ) );
         }
      }
      
      mc2dbg << "[CR]: Nbr itemids found = " << itemIDs.size()
             << " of " << (useDests->cardinal() >> 1) << endl;
      if ( (int)itemIDs.size() < (useDests->cardinal() >> 1) ) {
         // Preprocessing to calculate destinations from
         // not valid and throughfare nodes
         mc2dbg << "[CR]: Size of m_invalidNodeVector = "
                <<  m_invalidNodeVector.size() << endl;
         
         
         for ( uint32 i = 0; i < m_invalidNodeVector.size(); ++i ) {
            if ( ! m_invalidNodeVector[i].second->isVisited(m_map) ) {
               RoutingNode* node = m_invalidNodeVector[i].second;
               // Check if there is a car-path to the node
               const bool node0found =
                  carDests.find(node->getItemID()) != carDests.end();
               // Check if there is a car-path to the other node.
               const bool node1found =
                  carDests.find(TOGGLE_UINT32_MSB(node->getItemID()))
                  != carDests.end();
               if ( (! node0found) && (! node1found) ) {
                  m_normalPriorityQueue->enqueue(node);
               }
            }
         }
         m_invalidNodeVector.clear();
         expandNodesResult(driverParam, forward);
      } else {
         mc2dbg << "[CR]: All destinations found validly" << endl;       
      }
   } // endif overview map
   
   
   // All destinations should now have a path.
   // Update the offset.
   tempNodeItem = startNode;
   
   while (tempNodeItem != NULL) {
      RoutingNode* curNode = m_map->getNode(tempNodeItem->getIndex());
      
      float32 dOffset = FLOAT_OFFSET( tempNodeItem->getOffset() );

      if (curNode->getEstCost(m_map) == MAX_UINT32) {
         mc2dbg4 << "Not a valid destination" << endl;
      } else {
         uint32 minCost = getMinCost(curNode, driverParam);
         if (minCost != MAX_UINT32) {
            minCost = int32(minCost*dOffset);
            mc2dbg4 << "End offset " << dOffset << endl;
            mc2dbg4 << "Updating node " << hex << curNode->getItemID()
                   << dec << endl;
            DEBUG4(tracePath( curNode, driverParam ));
            // Check how we got here...
            RoutingNode* gradient = curNode->getGradient(m_map);
            if ( gradient != NULL ) {
               RoutingConnection* conn =
                  curNode->getConnection(gradient, !forward);
               if ( conn != NULL ) {
                  if ( NOT_VALID(curNode->getRestriction()) ||
                       ((conn->getData()->getVehicleRestriction(usingCostC) &
                        driverParam->getVehicleRestriction()) == 0) ) {
                     mc2dbg << "Using penalty for node "
                            << hex << curNode->getItemID() << dec
                            << endl;
                     minCost =
                        uint32(getMinCostWalk(curNode, driverParam) * dOffset);
                  }
               }
            }

            curNode->setEstCost(m_map,curNode->getEstCost(m_map) + minCost);
            // Set real cost to the same.
            curNode->setRealCost(m_map,curNode->getRealCost(m_map) + minCost);
         } else {
            mc2dbg << "MinCost == MAX_UINT32" << endl;
         }
      }
      tempNodeItem = static_cast<OrigDestNode*>(tempNodeItem->suc());
   }   
   
   // Now all possible destinations should have a path with a cost ->
   // Find the best destination
   tempNodeItem = startNode;
   RoutingNode* bestNode = NULL;
   OrigDestNode* bestOrigDestNode = NULL;
   uint32 minCost = MAX_UINT32;

   while (tempNodeItem != NULL) {
      RoutingNode* curNode = m_map->getNode(tempNodeItem->getIndex());

      // Avoid reading destinations found validly.
      if ( validDestinationsRead.find( curNode->getItemID() ) ==
                                       validDestinationsRead.end() &&
           validDestinationsRead.find(
              TOGGLE_UINT32_MSB(curNode->getItemID() ) ) ==
           validDestinationsRead.end()) {
         if (curNode->getEstCost(m_map) < minCost &&
             (carDests.find(curNode->getItemID()) != carDests.end() ) )  {
            bestNode = curNode;
            curNode->setVisited(m_map, true);
            bestOrigDestNode = tempNodeItem;
            minCost = curNode->getEstCost(m_map);
            mc2dbg << "Found bestNode among cardests" << endl;
         }
      }
      tempNodeItem = static_cast<OrigDestNode*>(tempNodeItem->suc());
   }

   if ( bestNode == NULL ) {
      tempNodeItem = startNode;
      while (tempNodeItem != NULL) {         
         RoutingNode* curNode = m_map->getNode(tempNodeItem->getIndex());

         if ( validDestinationsRead.find( curNode->getItemID() ) ==
                                          validDestinationsRead.end() &&
              validDestinationsRead.find(
                 TOGGLE_UINT32_MSB(curNode->getItemID() ) ) ==
              validDestinationsRead.end() ) {
            if (curNode->getEstCost(m_map) < minCost ) {
               bestNode = curNode;
               bestOrigDestNode = tempNodeItem;
               minCost = curNode->getEstCost(m_map);
               mc2dbg << "Found bestNode among all destinatiins" << endl;
            }
         }
         tempNodeItem = static_cast<OrigDestNode*>(tempNodeItem->suc());
      }
   }
      
   mc2dbg << "[CR]: ReadResult: Bestnode = " << bestNode << endl;
   // Found a destination on this map
   if (bestNode != NULL) {
//      tracePath( bestNode, driverParam );      
//      mc2dbg << "MinCost " << minCost << endl;
      // Update the cutOff value
      if (minCost < m_cutOff) {
         m_cutOff = minCost;
      }

      // Fills this subroute to the resultlist
      CompleteRMSubRoute* subRoute =
         readResultFromDestination(incomingList,
                                   resultList,
                                   bestNode,
                                   NULL,
                                   driverParam,
                                   forward,
                                   bestOrigDestNode->getOffset(),
                                   calcCostSums);

      {
         RoutingNode* externalNode = m_map->getExternalNodes();
         // Is there a better way than to loop?
         for (uint32 i = 0; i < m_map->getNumExternalNodes(); i++) {
            const uint32 curID = externalNode[i].getItemID();
            
            if ( curID == bestNode->getItemID() ) {                     
               ExternalRoutingConnection* extCon =
                  (ExternalRoutingConnection*)
                  externalNode[i].getFirstConnection(true);
               while ( extCon != NULL ) {
                  mc2dbg << "[CR]: result extradest to external "
                         << IDPair_t(extCon->getMapID(),
                                     extCon->getNodeID()) << endl;
                  
                  subRoute->addExternal( extCon->getMapID(),
                                         extCon->getNodeID(),
                                         bestNode->getRealCost(m_map),
                                         bestNode->getRealCost(m_map),
                                         bestNode->getLat(),
                                         bestNode->getLong(),
                                         0,
                                         0,
                                         0);
                  extCon = (ExternalRoutingConnection*)
                     (extCon->getNext());
               }
            }
         }
      }     

      
      
      // Have to update start/end-offset and
      if ( ! MapBits::isOverviewMap(m_map->getMapID()) ) {
         if (subRoute != NULL) {
            insertStateElement(subRoute,
                               driverParam,
                               forward);
         }
      }
   }

   if ( readToExternal ) {

   
      
      // Read all the routes from the external nodes
      // XXX use readResultToExternalConnection instead????
      RoutingNode* externalNode = m_map->getExternalNodes();
      for (uint32 i = 0; i < m_map->getNumExternalNodes(); i++) {
         RoutingNode* tempNode = 
            m_map->getNodeFromTrueNodeNumber(externalNode[i].getItemID());
         
         if (tempNode != NULL) {         
            // XXX: Only check the cost to cover the case
            // when starting node is on the map edge.
            if ( tempNode->getEstCost(m_map) != MAX_UINT32 ) {
               readResultFromDestination(incomingList,
                                         resultList,
                                         tempNode,
                                         &externalNode[i],
                                         driverParam,
                                         forward,
                                         0,  // offset
                                         calcCostSums);
            }
         }
      }
   }
   // Borde kolla resultlist om det finns items annars returnera en felkod.
   return StringTable::OK;
} // readResult

uint32 
CalcRoute::readResultToAll(Head* origin,
                           Head* destination,
                           SubRouteList* incomingList,
                           SubRouteList* resultList,
                           const RMDriverPref* driverParam,
                           bool forward,
                           bool calcCostSums)
{
   // FIXME: Fix readresult so that it can read the result to all instead
   // non-valid routes may affect the valid ones.
   mc2dbg4 << "CalcRoute::readResultToAll" << endl;
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 status = StringTable::NOTFOUND;
   OrigDestNode* curDest = static_cast<OrigDestNode*>(destination->first());
   while ( curDest != NULL ) {
      // Do a small trick and send in each destination in its own list.
      OrigDestNode* destCopy =
         m_map->newOrigDestNode(curDest->getIndex(),
                                curDest->getMapID(),
                                curDest->getOffset(),
                                curDest->getLat(),
                                curDest->getLong(),
                                curDest->getRealCost(m_map),
                                curDest->getEstCost(m_map),
                                curDest->getCostASum(),
                                curDest->getCostBSum(),
                                curDest->getCostCSum()
                                );
       Head* oneDestList = new Head;
       destCopy->into(oneDestList);
       if ( readResult(origin,
                       oneDestList,
                       incomingList,
                       resultList,
                       driverParam,
                       forward,
                       calcCostSums,
                       curDest->suc() == NULL) // Read external if last one
            == StringTable::OK) {
          // One is enough
          status = StringTable::OK;
       }
       destCopy->out();
       delete destCopy;
       delete oneDestList;
       curDest = static_cast<OrigDestNode*>(curDest->suc());
   }
   uint32 stopTime = TimeUtility::getCurrentTime();
   mc2dbg << "Number of subRoutes in resultList = "
          << resultList->getNbrSubRoutes() << " Time:" 
          << (stopTime-startTime) << " millis" << endl;
   return status;
}

uint32
CalcRoute::readResultWalk( Head* origin,
                           Head* destination,
                           SubRouteList* incomingList,
                           SubRouteList* resultList,
                           const RMDriverPref* driverParam,
                           bool forward,
                           bool calcCostSums)
{
   mc2dbg2 << "CalcRoute::readResultWalk" << endl;

   OrigDestNode* startNode;   
   if (forward) { // CalcCost from origin i.e. read result from destination
      startNode = static_cast<OrigDestNode*>(destination->first());
   }
   else {
      startNode = static_cast<OrigDestNode*>(origin->first());
   }

   OrigDestNode* tempNodeItem = startNode;
   // Update the offset first.
   while( tempNodeItem != NULL ){
      RoutingNode* tempNode = m_map->getNode( tempNodeItem->getIndex() );
      float32 dOffset = FLOAT_OFFSET( tempNodeItem->getOffset() );

      if ( tempNode->getEstCost(m_map) == MAX_UINT32 ) {
         mc2dbg4 << "Not a valid destination (walk)" << endl;
      } else {
         uint32 minCost = getMinCost(tempNode, driverParam, forward);
         if ( minCost != MAX_UINT32 ) {
            minCost = uint32(minCost * dOffset);
            mc2dbg4 << "End offset for walker " << dOffset << endl;
            tempNode->setEstCost(m_map,tempNode->getEstCost(m_map) + minCost);
            tempNode->setRealCost(m_map,tempNode->getEstCost(m_map));
         } else {
            mc2dbg4 << "MinCost == MAX_UINT32" << endl;            
         }
      }
      tempNodeItem = static_cast<OrigDestNode*>(tempNodeItem->suc());
   }

   tempNodeItem = startNode;
   RoutingNode* bestNode = NULL;
   OrigDestNode* bestOrigDestNode = NULL;
   uint32 maxCost = MAX_UINT32;

   while ( tempNodeItem != NULL ) {
      RoutingNode* tempNode = m_map->getNode( tempNodeItem->getIndex() );
      if( tempNode->getRealCost(m_map) < maxCost ){
         bestNode = tempNode;
         bestOrigDestNode = tempNodeItem;
         maxCost = tempNode->getRealCost(m_map);
      }
      tempNodeItem = static_cast<OrigDestNode*>(tempNodeItem->suc());
   }
   
   // Found a destination on this map
   if( bestNode != NULL ) {
      // Update the cutOff value
      if( maxCost < m_cutOff )
         m_cutOff = maxCost;

      // Fills this subroute to the resultlist
      readResultFromDestination(incomingList,
                                resultList,
                                bestNode,
                                NULL,
                                driverParam,
                                forward,
                                bestOrigDestNode->getOffset(),
                                calcCostSums);

      // Have to update start/end-offset and   <--- ????
      /*    if (subRoute != NULL) {
          CompleteRMSubRoute* completeSubRoute =
              static_cast<CompleteRMSubRoute*>(subRoute);
          Vector* nodeIDs = completeSubRoute->getNodeIDs();         
          nodeIDs->insertElementAt(0, RouteConstants::WALK_ITEM_ID);
          } */ 
   }
   
   // Read all the routes from the external nodes
   // XXX use readResultToExternalConnection instead????
   RoutingNode* externalNode = m_map->getExternalNodes();
   for( uint32 i=0;i<m_map->getNumExternalNodes();i++ ){
      RoutingNode* tempNode = m_map->getNodeFromTrueNodeNumber(
         externalNode[i].getItemID() );
      if( tempNode != NULL ){

         if( ( tempNode->getGradient(m_map) != NULL ) &&
             ( tempNode->getEstCost(m_map)     != MAX_UINT32 ) &&
             ( !tempNode->isVisited(m_map) )){
            readResultFromDestination( incomingList,
                                       resultList,
                                       tempNode,
                                       &externalNode[i],
                                       driverParam,
                                       forward,
                                       0, // offset
                                       calcCostSums);           
         }
      }
   }
                                 
   // XXX Should check the result list

   return StringTable::OK;
}

uint32 CalcRoute::readResultBus( Head* origin,
                                 Head* destination,
                                 SubRouteList* incomingList,
                                 SubRouteList* resultList,
                                 const RMDriverPref* driverParam,
                                 bool forward )
{
   return StringTable::NOTOK;
}


uint32 CalcRoute::readResultToExternalConnection(SubRouteList* incoming,
                                                 SubRouteList* result,
                                                 const RMDriverPref*
                                                             driverParam,
                                                 Head* orig,
                                                 Head* dest,
                                                 bool forward,
                                                 bool calcCostSums){
   mc2dbg << "CalcRoute::readResultToExternalConnection" << endl;
   FileDebugCalcRoute::writeComment("readResultToExternalConnection");
   // Test variable for debug file printout.
   bool neverEnteredReadResult = true;
   // Read all the routes from the external nodes
   RoutingNode* externalNode = m_map->getExternalNodes();

   const int nbrExternal = m_map->getNumExternalNodes();
   mc2dbg << "[CR]: Number of external nodes in map = "
          << nbrExternal << endl;
    
   for (int i = 0; i < nbrExternal; i++) {
      RoutingNode* curNode = 
         m_map->getNodeFromTrueNodeNumber( externalNode[i].getItemID() );

      static const bool onlyExternalOrigin = false;
      static const bool onlyExternal = false;
      
      if( curNode != NULL ) {
         if( ( curNode->getGradient(m_map) != NULL || onlyExternalOrigin ) && 
             ( curNode->getEstCost(m_map) != MAX_UINT32 ) ){
            if ( onlyExternal ) 
               mc2dbg8 << "OnlyExternal = " << onlyExternal << endl;
            
            curNode->setRealCost(m_map, curNode->getEstCost(m_map) );

            if ( dest != NULL ) {
#undef  USE_ESTIMATION_IN_EXTERNAL_NODES
#ifdef  USE_ESTIMATION_IN_EXTERNAL_NODES
               // FIXME: Use driverpref when estimating.
               uint32 estimated  =
                  estimateDistToDest( curNode,
                                      dest, 
                                      driverParam );
               curNode->setEstCost(m_map, curNode->getRealCost(m_map) +
                                   estimated );
#else
               curNode->setEstCost(m_map,curNode->getRealCost(m_map));
#endif
            }
            
            if( curNode->getEstCost(m_map) <= m_cutOff ){
               readResultFromDestination( incoming,
                                          result,
                                          curNode,
                                          &externalNode[i],
                                          driverParam,
                                          forward,
                                          0, // offset
                                          calcCostSums);
               neverEnteredReadResult = false;
            }
         }
      }
   }
   if ( neverEnteredReadResult )
      FileDebugCalcRoute::writeComment("Never entered readResultToDest from "
                                       " readResultExternal");
   mc2dbg << "Exit CalcRoute::readResultToExternalConnection" << endl;
   return StringTable::OK;
} // readResultToExternalConnection


inline
uint32 CalcRoute::checkAdditionalCosts(RoutingNode* fromNode,
                                       RoutingNode* toNode)
{
   // Get the connection                                 // forward
   RoutingConnection* conn = fromNode->getConnection(toNode, true);
   if ( conn == NULL ) {
      // Shouldn't happen, please
      mc2log << warn << "Route is connectionless between "
             << hex << fromNode->getItemID() << " and "
             << toNode->getItemID() << dec << endl;
      return 0;
   }
   RoutingConnectionData* data = conn->getData();
   // Zero vehicle is ok since we compare the diff
   int32 costDiff = data->getCostC(0) - data->getCostB(0);
   if ( costDiff ) {
      if ( costDiff < 0 ) {
         mc2dbg << "CostC - CostB is negative = "
                << costDiff << endl;
         costDiff =
            Connection::secToTimeCost(uint32(3600));
                          // FIXME: Not quite correct
                          // Sets it to an hour for now, to make it work in
                          // the ExpandRouteProcessor. Gets negative if the
                          // costC is set to MAX_UINT32.
         
      } else {
         uint32 retCost = Connection::timeCostToSec(uint32(costDiff));
         if(retCost == 0){
            return 1;
            mc2dbg4 << "Additional cost was 0, setting 1 instead" << endl;
         }
         else{
            return retCost;
         }
         
      }
      
   } /* else {
      // No difference in cost here
      // Check temporary diff
      uint32 costA;
      uint32 costB;
      uint32 costC;
      uint32 costD;
      if ( m_map->checkTempDisturbances(fromNode->getItemID(),
                                        toNode->getItemID(),
                                        costA,
                                        costB,
                                        costC,
                                        costD)) {
         cerr  << "Costdiff was " << costDiff << endl;
         if(data->getCostC(0) >= costC)
            costDiff = data->getCostC(0) - costC;
         else
            costDiff = costC - data->getCostC(0);
         
      }
      }*/
   return Connection::timeCostToSec(uint32(costDiff));
   
}

void
CalcRoute::sumCostsForConnection(RoutingNode* fromNode,
                                 RoutingNode* toNode,
                                 uint32& costASum,
                                 uint32& costBSum,
                                 uint32& costCSum)
{
   // Get the connection                                 // forward
   RoutingConnection* conn = fromNode->getConnection(toNode, true);
   if ( conn == NULL ) {
      // Shouldn't happen, please
      mc2log << warn << "Route is connectionless between "
             << hex << fromNode->getItemID() << " and "
             << toNode->getItemID() << dec << endl;
      return;
   }
   RoutingConnectionData* data = conn->getData();
   costASum += data->getCostA(0);
   costBSum += data->getCostB(0);
   costCSum += data->getCostC(0);
}

inline
CompleteRMSubRoute*
CalcRoute::fillSubRouteFromDestination(RoutingNode* dest,
                                       bool forward,
                                       uint32& extID,
                                       bool calcCostSums,
                                       uint32& costASum,
                                       uint32& costBSum,
                                       uint32& costCSum)
{
   // Cost sums for sortdist request
   costASum = 0;
   costBSum = 0;
   costCSum = 0;
      
   RoutingNode* gradient = dest->getGradient(m_map);
   RoutingNode* tempNode = dest;
   
   mc2dbg4 << "Destination has cost : " << dest->getRealCost(m_map) << endl;
   
   // Subroute from origin to dest inside the current map not including
   // the connecting node on other map
   CompleteRMSubRoute* subRoute  = new CompleteRMSubRoute(m_map->getMapID());
   CompleteRMSubRoute* tempRoute = new CompleteRMSubRoute(m_map->getMapID());
   // Allocate a lot of room for the subroute.
   tempRoute->getNodeIDs()->setAllocSize(
      MAX(4096,
          tempRoute->getNodeIDs()->getAllocSize()));
   // Add the subroute and check if we have
   // to walk somewhere along the route
   
   int nbrNodesAdded = 0;
   while( gradient != NULL ) {
      mc2dbg8 << "Adding node " << hex << tempNode->getItemID() << dec << endl;
      tempRoute->addEndNode(tempNode->getItemID());
      // Check if costB != costC for the connection
      if ( gradient != NULL ) {
         RoutingNode* grToCheck = gradient;
         // FIXME: Test this outside the loop
         if ( gradient->getGradient(m_map) == NULL ) {
            // OrigDestNode !! Has no connections            
            // Get the real node instead - for reading route.
            mc2dbg2 << "GRADIENT = NULL" << endl;
            grToCheck = m_map->getNode(gradient->getIndex());
         }
         if ( grToCheck->getItemID() == tempNode->getItemID() ) {
            // Same node twice. Should be the origin when we come
            // from another map.
            // Skip the checks and go down.
         } else { 
            uint32 extraCostSec = checkAdditionalCosts( grToCheck, tempNode );
            if ( extraCostSec ) {
               mc2dbg8 << "Adding extra cost of "
                       << extraCostSec << " seconds "
                       << "between " << hex << gradient->getItemID() << " and "
                       << tempNode->getItemID() << dec << endl;
               mc2dbg8 << "Will set nodeid to " << hex
                       << (ADD_COST_STATE_MASK | extraCostSec)
                       << dec << endl;
               tempRoute->addEndNode( ADD_COST_STATE_MASK | extraCostSec );
            }
            // FIXME: Are all costs summed up this way?
            // FIXME: Do this outside the loop.
            if ( calcCostSums ) {
               sumCostsForConnection( grToCheck, tempNode, costASum,
                                      costBSum, costCSum);
            }
         }
      }
      tempNode = gradient;
      gradient = gradient->getGradient(m_map);
       if ( (nbrNodesAdded & 0xffff) == 0xffff ) {
          mc2log << warn << "Added " << nbrNodesAdded << " to the result"
                 << endl;
          set<uint32> checkNodes;
          Vector* nodeIDs = tempRoute->getNodeIDs();
          for(int i=nodeIDs->getSize() - 1; i >= 0; --i ) {
             uint32 curNodeID = (*nodeIDs)[i];
             if ( GET_ADDITIONAL_COST_STATE( curNodeID) == 0 ) {
                if ( checkNodes.find( curNodeID ) != checkNodes.end() ) {
                   // Print error msg
                   stringstream loop;
                   loop << "[CalcRoute]: Loop (backwards) :";
                   for( int j = nodeIDs->getSize() - 1; j >= i; -- j) {
                      loop << hex << (*nodeIDs)[j] << dec << " ";
                   }
                   loop << endl << ends;
                   mc2log << error << loop.str();
                   // Get out.
                   delete tempRoute;
                   delete subRoute;
                   return NULL;
                } else {
                   checkNodes.insert(curNodeID);
                }
             }
          }
          if ( nbrNodesAdded > int(m_map->getNbrNodes()) ) {
             mc2log << error << "More nodes in the route than in the map"
                    << "-probably a loop" << endl;
             // Very long route. Probably a loop.
             delete tempRoute;
             delete subRoute;
             return NULL;
          }
       }
       ++nbrNodesAdded;
   }
   mc2dbg4 << "Adding last node "
          << hex << tempNode->getItemID() << dec << endl;
   extID = tempNode->getItemID();
   tempRoute->addEndNode(extID);

   // FIXME: Is this correct? Found out today that there
   //        are routes that only consist of one node.
   //        (gradient is null from the start)
   //        Should they be handled here or in "oneSegment"?
   if ( subRoute->getNodeIDs()->getSize() == 1 ) {
      // Only endnode added!!!
      mc2dbg << "Size of tempRoute "
             << tempRoute->getNodeIDs()->getSize() << endl;
      delete subRoute;
      return NULL;
   }

   
   if (forward) {
      // Reverse the order
      Vector* nodeIDs = tempRoute->getNodeIDs();
      //uint32 size = nodeIDs->getSize();
      std::reverse(nodeIDs->begin(), nodeIDs->end() );
      // Make sure that the array is big enough
      //subRoute->setAllocSize(size);
      //for (int32 i = size - 1; i >= 0; --i) {
      //   subRoute->addLast(nodeIDs->getElementAt(i));
      //}
      //delete tempRoute;
      delete subRoute;
      subRoute = tempRoute;
   } else {
      delete subRoute;
      subRoute = tempRoute;
   }

   if ( false ) {
      // Print the costs for all nodes in the route
      Vector* nodeIDs = subRoute->getNodeIDs();
      mc2dbg << "[CR]: Route : " << endl;
      for( int i = 0; i < (int)nodeIDs->size(); ++i ) {
         RoutingNode* node = m_map->getNodeFromTrueNodeNumber((*nodeIDs)[i]);
         mc2dbg << "[CR]: Node " << hex << (*nodeIDs)[i] << dec
                << " has real cost " << node->getRealCost(m_map)
                << " and est cost " << node->getEstCost(m_map) << endl;
      }
   }
   
   subRoute->setForward(forward);
   
   return subRoute;
}

uint32 CalcRoute::readResultToBestOnHigherLevel( Head* origin,
                                                 Head* destination,
                                                 SubRouteList* incomingList,
                                                 SubRouteList* resultList,
                                             const RMDriverPref* driverParam,
                                                 bool calcCostSums)
{
   // Mark destination nodes and update the costs
   OrigDestNode* tempNode =
      static_cast<OrigDestNode*>(destination->first());

   RoutingNode* bestNode = NULL;
   OrigDestNode* bestRealNode;
   uint32 minCost = MAX_UINT32;
   while( tempNode != NULL ) {

      uint32 higherNodeID = m_map->translateToHigher( tempNode->getMapID(),
                                                      tempNode->getIndex() );
      
      if( higherNodeID != MAX_UINT32 ) {

         RoutingNode* curNode = 
            m_map->getNodeFromTrueNodeNumber(higherNodeID); 
        
         if( curNode != NULL ) {
            if( curNode->getEstCost(m_map) < MAX_UINT32 ) {
               if( (curNode->getEstCost(m_map) + tempNode->getRealCost(m_map)) < minCost ){
                  minCost = curNode->getEstCost(m_map) + tempNode->getRealCost(m_map);
                  bestNode = curNode;
                  bestRealNode = tempNode;
               }
            }
         }
      }
      else {
         mc2dbg8 << "found .. NULL" << endl;
      }
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }

   if( bestNode != NULL ) {

      uint32 extID;
      uint32 costASum;
      uint32 costBSum;
      uint32 costCSum;
      CompleteRMSubRoute* subRoute = fillSubRouteFromDestination(bestNode,
                                                                 true,
                                                                 extID,
                                                                 calcCostSums,
                                                                 costASum,
                                                                 costBSum,
                                                                 costCSum);
      subRoute->setForward(true);

      // Find the previous history
      uint32 mapID, nodeID, cost, estimatedCost;
      RMSubRoute* tempSubRoute;
      for( uint32 i = 0; i < incomingList->getNbrSubRoutes(); i++ ) {
         tempSubRoute = static_cast<RMSubRoute*>
            (incomingList->getSubRoute(i));
         for (uint32 j = 0; j < tempSubRoute->getNbrConnections(); j++) {
            int lat, lon; // Not really used.
            uint32 dummy;
            tempSubRoute->getExternal(j,
                                      mapID,
                                      nodeID,
                                      cost,
                                      estimatedCost,
                                      lat,
                                      lon,
                                      dummy,
                                      dummy,
                                      dummy);
                         

            uint32 higherNodeID = m_map->translateToHigher( mapID, nodeID );
            
            if( higherNodeID != MAX_UINT32 ) {
               if( tempNode->getItemID() == higherNodeID ) {
                  subRoute->setPrevSubRouteID(
                     tempSubRoute->getSubRouteID());
//                  mc2dbg << "subRoute " << subRoute->getSubRouteID() <<
//                     " has prevSubRoute " << subRoute->getPrevSubRouteID()
//                       << " (readResultOnHigherLevel)" << endl;
               } // if
            } // if
         } // for
      } // for
      
      RoutingConnection* tmpConnection;
      RoutingNode* newNode;
      tmpConnection = bestNode->getFirstConnection(true);
      while( tmpConnection != NULL ) {
         newNode = tmpConnection->getNode();

         IDPair_t lowerLevelIDs(
            m_map->translateToLower( newNode->getItemID() ) );
         
         uint32 lowerMapID  = lowerLevelIDs.first;
         uint32 lowerNodeID = lowerLevelIDs.second;
                 
         if( lowerMapID != MAX_UINT32 ) {
            
            subRoute->setRouteComplete(true); // NB This was outcommented!!
            subRoute->setVisited(true);
            subRoute->addExternal( lowerMapID,
                                   lowerNodeID,
                                   bestNode->getRealCost(m_map),
                                   bestNode->getEstCost(m_map),
                                   bestNode->getLat(),
                                   bestNode->getLong(),
                                   costASum,
                                   costBSum,
                                   costCSum);
            mc2dbg4 << "LAT1 : " << bestNode->getLat()
                    << " LON1 : " << bestNode->getLong() << endl;
         }
         else {
            mc2dbg8 << "NULL" << endl;
         }
         
         tmpConnection = (RoutingConnection*)tmpConnection->getNext();
      }
      uint32 index = resultList->addSubRoute( subRoute );
      if( index == MAX_UINT32 )
         delete subRoute;
   }
   else {
      mc2log << warn << "readResultOnHigherLevel did not find a best node"
             << endl;
      return StringTable::OK;
   }
   return StringTable::OK;
} // readResultOnHigherLevel

uint32
CalcRoute::readResultOnHigherLevel( Head* origin,
                                    Head* destination,
                                    SubRouteList* incomingList,
                                    SubRouteList* resultList,
                                    const RMDriverPref* driverParam,
                                    bool calcCostSums)
{
   const bool readResultToAll = false;
   if ( readResultToAll ) {
      uint32 status = StringTable::NOT;
      OrigDestNode* curDest = static_cast<OrigDestNode*>(destination->first());
      while ( curDest != NULL ) {
         OrigDestNode* destCopy =
            m_map->newOrigDestNode(curDest->getIndex(),
                                   curDest->getMapID(),
                                   curDest->getOffset(),
                                   curDest->getLat(),
                                   curDest->getLong(),
                                   curDest->getRealCost(m_map),
                                   curDest->getEstCost(m_map),
                                   curDest->getCostASum(),
                                   curDest->getCostBSum(),
                                   curDest->getCostCSum()
                                   );
         Head* oneDestList = new Head;
         destCopy->into(oneDestList);
         if ( readResultToBestOnHigherLevel(origin,
                                            oneDestList,
                                            incomingList,
                                            resultList,
                                            driverParam,
                                            calcCostSums) == StringTable::OK) {
            // One is enough
            status = StringTable::OK;
         }
            
         destCopy->out();
         delete destCopy;
         delete oneDestList;
         curDest = static_cast<OrigDestNode*>(curDest->suc());
      }
      mc2dbg << "Number of subRoutes in resultList = "
             << resultList->getNbrSubRoutes() << endl;
      return status;
   } else {
      return readResultToBestOnHigherLevel(origin,
                                           destination,
                                           incomingList,
                                           resultList,
                                           driverParam,
                                           calcCostSums);
   }
}
  

CompleteRMSubRoute*
CalcRoute::readResultFromDestination(SubRouteList* incoming,
                                     SubRouteList* result,
                                     RoutingNode* dest,
                                     RoutingNode* external,
                                     const RMDriverPref* driverParam,
                                     bool forward,
                                     uint16 endOffset,
                                     bool calcCostSums)
                         
{
   
   mc2dbg4 << "CalcRoute::readResultFromDestination" << endl;

   uint32 extID;
   uint32 costASum = 0;
   uint32 costBSum = 0;
   uint32 costCSum = 0;
   CompleteRMSubRoute* subRoute = fillSubRouteFromDestination(dest,
                                                              forward,
                                                              extID,
                                                              calcCostSums,
                                                              costASum,
                                                              costBSum,
                                                              costCSum);

   
   if ( subRoute == NULL )
      return NULL;

   mc2dbg8 << "CR.rrfd, costs = "
           << costASum << ':'
           << costBSum << ':'
           << costCSum << endl;
      
   
   subRoute->setEndOffset( endOffset );
   subRoute->setForward( forward );

   mc2dbg8 << "[CR]: Looking for " << hex << extID << dec << endl;
   RMSubRoute* prevSubRoute = incoming->findExternalNodeSubRoute( extID
                                                     /* use offset too? */
                                                                  );

   if (prevSubRoute != NULL ) {      
      subRoute->setPrevSubRouteID(prevSubRoute->getSubRouteID());
      
      // Also add costASums - costCSums
      // Must look at the previous SubRoute to get costs a-c
      if ( calcCostSums ) {
         
         uint32 prevCostASum;
         uint32 prevCostBSum;
         uint32 prevCostCSum;
         // It will probably be enough with the first one, since all costs
         // should be equal at this point
         prevSubRoute->getCostSums(0, prevCostASum,
                                   prevCostBSum, prevCostCSum);

         mc2dbg8 << "CR:rrfd PrevSubRoute has "
                 << prevCostASum << ':'
                 << prevCostBSum << ':'
                 << prevCostCSum << endl;
         
         costASum += prevCostASum;
         costBSum += prevCostBSum;
         costCSum += prevCostCSum;
      }
      
      mc2dbg8 << "subRoute " << subRoute->getSubRouteID() 
              << " has prevSubRouteID " << subRoute->getPrevSubRouteID() 
              << " (readResultFromDestination)" << endl;
   } else {
      // This means that the gradient is missing for a node and should
      // not happen. 
      mc2log << error << "[CR]: prevSubRoute == NULL for destnode " << hex
             << dest->getItemID() << dec << " orig node " << hex
             << extID << dec << " could not be found"
             << endl;
      delete subRoute;
      return NULL;
   }
   
   bool bUpdate = false;

   if (external != NULL) {
      //Add the connecting node(s) on the other map to the bordernode
      ExternalRoutingConnection* tempCon =
         (ExternalRoutingConnection*)external->getFirstConnection(true); 
      RoutingConnectionData* connData;
      uint32 costA = driverParam->getCostA();
      uint32 costB = driverParam->getCostB();
      uint32 costC = driverParam->getCostC();
      uint32 costD = driverParam->getCostD();
      uint32 estimated = 0;


#ifdef USE_ESTIMATION_IN_EXTERNAL_NODES
      // XXX: Testing estimation
      // Seems to go slower with estimation. Don't know why.
      // It shouldn't if the estimated cost is larger with larger
      // distance.
      // Turned out that the estimation didn't return the values
      // in the right units for the costs.
      // It is possible that the estimation works a little now, but
      // I haven't checked it very much.
      // Now I have checked it even more and figured out that the
      // driverpref costs must be in the estimation.
      
      if ( dest->getEstCost(m_map) > dest->getRealCost(m_map) ) {
         estimated = dest->getEstCost(m_map) - dest->getRealCost(m_map);
         //mc2dbg << "Using estimated cost!! = " << estimated << endl;
      }
#endif

      // Add special external route for the node on this map.
//          subRoute->addExternal( m_map->getMapID(),
//                                 dest->getItemID(),
//                                 dest->getRealCost(m_map),
//                                 dest->getEstCost(m_map),
//                                 dest->getLat(),
//                                 dest->getLong(),
//                                 costASum,
//                                 costBSum,
//                                 costCSum);
//        bUpdate = true;
      
      while( tempCon != NULL ) {
         // Cost is always 0 here
         connData = tempCon->getData();
         uint32 cost = dest->getRealCost(m_map) +
            costA * connData->getCostA(0) +
            costB * connData->getCostB(0) +
            costC * connData->getCostC(0) +
            costD * connData->getCostD();
         
         if( (estimated+cost) <= m_cutOff ) { 
            if( !( IS_UPPER_LEVEL( external->getItemID() ) &&
                   IS_LOWER_LEVEL( tempCon->getNodeID()   ) &&  
     ( ( incoming->getListType() == SubRouteListTypes::HIGHER_LEVEL_FORWARD) ||
     (incoming->getListType() == SubRouteListTypes::HIGHER_LEVEL_BACKWARD)))) {
               
               bUpdate = true;
               // index is really the itemID!!
               subRoute->addExternal( tempCon->getMapID(),
                                      tempCon->getNodeID(), 
                                      cost,
                                      estimated + cost,
                                      dest->getLat(),  // Check this
                                      dest->getLong(),  // Check this
                                      costASum,
                                      costBSum,
                                      costCSum); 
               mc2dbg4 << "LAT2 : " << dest->getLat()
                       << " LON2 : " << dest->getLong() << endl;
            } else {
               mc2dbg << "[CR]: Not adding route to lower" << endl;
            }
         }
         tempCon =  (ExternalRoutingConnection*)tempCon->getNext();
      }
   } else {
      bUpdate = true;
      mc2dbg4 << "LAT3 : " << dest->getLat()
              << " LON3 : " << dest->getLong() << endl;
      subRoute->addExternal( m_map->getMapID(),
                             dest->getItemID(), 
                             dest->getRealCost(m_map),
                             dest->getEstCost(m_map),
                             dest->getLat(),
                             dest->getLong(),
                             costASum,
                             costBSum,
                             costCSum);
      mc2dbg4 << "Added external node " << hex
              << dest->getItemID() << dec << " with cost "
              << dest->getRealCost(m_map) << endl;
      subRoute->setRouteComplete( true );
      subRoute->setVisited(true);
   }
   
   if( bUpdate ) {
      uint32 index = result->addSubRoute( subRoute );         
      if( index == MAX_UINT32 ) {
         delete subRoute;
         subRoute = NULL;
      }
   } else {
      mc2dbg8 << "The subRoute is NULL" << endl;
      delete subRoute;
      subRoute = NULL;
   }
   return subRoute;
} // ReadResultFromDestination

////////////////////////////////////////////////////////
// Help methods for reading the result
////////////////////////////////////////////////////////


uint32 
CalcRoute::getMinCost( RoutingNode* curNode,
                       uint32 vehRes,
                       dpCost_t costA,
                       dpCost_t costB,
                       dpCost_t costC)
{
   // Backward has no use here since we are trying to find the
   // length of the segment, not the minimum length "other segment"
   const bool forward = true;
   RoutingConnection* conn = curNode->getFirstConnection( forward );
   uint32 minCost = MAX_UINT32; 
   while( conn != NULL ){
      RoutingConnectionData* connData = conn->getData();
      uint32 cost = costA * connData->getCostA(vehRes)+
                    costB * connData->getCostB(vehRes)+
                    costC * connData->getCostC(vehRes);

      if( cost < minCost )
         minCost = cost;

      conn = conn->getNext();
   }
   // Check the other side 
   curNode = m_map->getNodeFromTrueNodeNumber(
      TOGGLE_UINT32_MSB( curNode->getItemID()) );
   conn = curNode->getFirstConnection( forward );
   while( conn != NULL ){
      RoutingConnectionData* connData = conn->getData();
      uint32 cost =  
         costA * connData->getCostA(vehRes)+
         costB * connData->getCostB(vehRes)+
         costC * connData->getCostC(vehRes);

      if( cost < minCost )
         minCost = cost;

      conn = conn->getNext();
   }
   return minCost;
}

uint32 
CalcRoute::getMinCost( RoutingNode* curNode,
                       const RMDriverPref* driverParam,
                       bool forward)
{
   uint32 vehRes = driverParam->getVehicleRestriction();
   return getMinCost( curNode,
                      vehRes,
                      driverParam->getCostA(),
                      driverParam->getCostB(),
                      driverParam->getCostC());

}

uint32
CalcRoute::getMinCostWalk( RoutingNode* curNode,
                           uint32 vehRes,
                           dpCost_t costA,
                           dpCost_t costB,
                           dpCost_t costC)
{
   const uint32 minCostA = getMinCost( curNode, vehRes, 1, 0, 0 );
   return costA * minCostA * RouteConstants::WALK_FACTOR +
      ( costB + costC ) * getWalkingCostBFromA(minCostA);
}

uint32
CalcRoute::getMinCostWalk( RoutingNode* curNode,
                           const RMDriverPref* driverParam)
{
   return getMinCostWalk( curNode,
                          driverParam->getVehicleRestriction(),
                          driverParam->getCostA(),
                          driverParam->getCostB(),
                          driverParam->getCostC());
}

void
CalcRoute::expandNonValidNodesResult( RedBlackTree* resetHeapNonValid,
                                      RedBlackTree* resetHeapThroughfare,
                                      const RMDriverPref* driverParam,
                                      bool forward  )
{
   const bool usingCostC = driverParam->getCostC() != 0;
   mc2dbg << "CalcRoute::expandNonValidNodesResult" << endl;
   FileDebugCalcRoute::writeComment("expandNonValidNodesResult");
   // First traverse all those nodes with MAX_UINT32 as real cost as they
   // haven't been visited previously. Put the edge nodes on a new stack

   // Set of destinations that have been used for getting out.
   set<RoutingNode*> usedDestinations;
   
   while( ! m_notValidPriorityQueue->isEmpty() ) {
      RoutingNode* curNode = 
         m_notValidPriorityQueue->dequeue();

      if ( curNode->getEstCost(m_map) > m_cutOff ) {
         // If we have been routing backwards more than the cutoff
         // it will be to expensive in the other direction too.
         mc2dbg << "[CR]: envnr: Node "
                << hex
                << curNode->getItemID()
                << dec << " has cost > m_cutOff" << endl;
         continue;
      }
      
      // Route in the opposit direction.
      RoutingConnection* curConnection = 
         curNode->getFirstConnection( ! forward );

      // For all the connections in the node.
      // FIXME: Should it be like this. Maybe we should only continue
      //        with the ones that don't have any valid connections?
      while( curConnection != NULL ) {
         RoutingConnectionData* curConnData = 
            curConnection->getData();

         RoutingNode* prevNode = curConnection->getNode();
         
         RoutingNode* otherSide =
            m_map->getNodeFromTrueNodeNumber(
               TOGGLE_UINT32_MSB( prevNode->getItemID()));

         // testTest is true if there is no way out of this.
         // XXX: Not true. There can be a way from this segment
         // but the next segment can have no way.
         // Maybe testTest is unnecessary.
         bool testTest = ! ( checkVehicleRestrictions( prevNode,
                                                       driverParam,
                                                       forward) ||
                             checkVehicleRestrictions( otherSide,
                                                       driverParam,
                                                       forward));
         // Don't use testTest for now.
         testTest = true;

         bool cannotDrive = !(driverParam->getVehicleRestriction() &
            curConnData->getVehicleRestriction(usingCostC));
         bool nodeInvalid = NOT_VALID(curNode->getRestriction());
         bool canWalk = curConnData->getVehicleRestriction(usingCostC) &
                        ItemTypes::pedestrian;
         
         if( ( cannotDrive || nodeInvalid) && canWalk &&
             ( prevNode->getRealCost(m_map) == MAX_UINT32 ) && testTest ) {
            // Not valid
            uint32 cost = curNode->getEstCost(m_map) +
                          calcConnectionCostWalk(driverParam,
                                                 curConnData);            
            
            if( cost < prevNode->getEstCost(m_map) ) {
               // Only set est cost. Real cost is used to detect visited
               // by calcCostDijkstra
               prevNode->setEstCost(m_map, cost );
               mc2dbg8 << "Adding a node to m_notValidPriorityQueue "
                       << hex << prevNode->getItemID()
                       << dec << endl;
               FileDebugCalcRoute::writeComment("Adding to notVal");
               prevNode->setGradient(m_map, curNode );
               m_notValidPriorityQueue->enqueue( prevNode );
               resetHeapNonValid->enqueue( prevNode );
            }
         } else { // A visited node or a throughfare
            if( HAS_NO_THROUGHFARE( prevNode->getRestriction() ) && 
                ( prevNode->getRealCost(m_map) == MAX_UINT32 ) ) {
               // Only set est cost. Real cost is used to detect visited
               // by calcCostDijkstra
               prevNode->setEstCost(m_map,0);          
               m_throughfarePriorityQueue->enqueue( prevNode );   
               mc2dbg8 << "Adding a node to m_throughfarePriorityQueue(2) "
                       << endl;
               resetHeapThroughfare->enqueue( prevNode );
               prevNode->setGradient(m_map, curNode );
            } else if( prevNode->getRealCost(m_map) != MAX_UINT32 ) {
               mc2dbg8 << "Adding a node to m_normalPriorityQueue "
                      << endl;
#ifdef FILE_DEBUGGING
               prevNode->setEstCost(m_map,prevNode->getEstCost(m_map));
#endif
               // Trace back to destination.
               RoutingNode* extraNode;
               for ( extraNode = curNode;
                     extraNode->getGradient(m_map) != NULL;
                     extraNode = extraNode->getGradient(m_map) ) {
                  
               }
               mc2dbg << "[CR]: extranode = 0x"
                      << hex << extraNode->getItemID() << dec << endl;
               // Check if we already found a cheaper non-valid way
               // from the destination.
               if ( usedDestinations.find( extraNode ) ==
                    usedDestinations.end() ) {
                  // Not found. Add the new one.
                  m_normalPriorityQueue->enqueue( prevNode );
                  FileDebugCalcRoute::writeComment("Adding to normal");
                  if ( otherSide->getRealCost(m_map) != MAX_UINT32 ) {
                     m_normalPriorityQueue->enqueue( otherSide );
                  }
                  usedDestinations.insert(extraNode);
               } else {

               }
            } else {
               // It is possible to drive to this node but
               // it hasn't been visited.
               // I don't know if this one should be added then
               // I have commented out the ones that are only commented
               // out once.
               uint32 cost = curNode->getEstCost(m_map) +
                  calcConnectionCostWalk(driverParam, curConnData);
               if( cost < prevNode->getEstCost(m_map) ){
                  // Set cost should probably be here.
                  prevNode->setEstCost(m_map, cost );
                  mc2dbg8 << "Not adding a node to "
                          << "m_notValidPriorityQueue(2) "
                          << hex << prevNode->getItemID() << dec << endl ;
                  FileDebugCalcRoute::writeComment("not Adding to notVal(2)");
                  //m_notValidPriorityQueue->enqueue( prevNode );
                  resetHeapNonValid->enqueue( prevNode );
               }              
            }
         }
         curConnection = curConnection->getNext();
      }         
   }

   // The normal and throughfare heaps now contains a
   // list with all the edge nodes.
   // First reset the nodes
   FileDebugCalcRoute::writeComment
      ("Resetting nodes is resetHeapNonValid");
   while( !resetHeapNonValid->isEmpty() ){
      RoutingNode* curNode = 
         resetHeapNonValid->dequeue();
      curNode->reset(m_map);
   }   
   m_notValidPriorityQueue->reset();
}

void
CalcRoute::expandThrougfareNodesResult( RedBlackTree* resetHeapThroughfare,
                                        const RMDriverPref* driverParam,
                                        bool forward )
{
   const uint32 vehRes = driverParam->getVehicleRestriction();
   const bool usingCostC = driverParam->getCostC() != 0;
   mc2dbg4 << "CalcRoute::expandThrougfareNodesResult" << endl;
   FileDebugCalcRoute::writeComment("expandThrougfareNodesResult");
   while( !m_throughfarePriorityQueue->isEmpty() ){
      RoutingNode* curNode = 
         m_throughfarePriorityQueue->dequeue();

      RoutingConnection* curConnection = 
         curNode->getFirstConnection( !forward );

      while( curConnection != NULL ) {
         RoutingConnectionData* curConnData = 
            curConnection->getData();

//           RoutingNode* prevNode = 
//                 m_map->getNode( curConnection->getIndex() );

         RoutingNode* prevNode = curConnection->getNode();
         
         if( ( driverParam->getVehicleRestriction() &
               curConnData->getVehicleRestriction(usingCostC) ) != 0 ) {
            if( HAS_NO_THROUGHFARE( prevNode->getRestriction() ) ) {
               uint32 cost = curNode->getEstCost(m_map) +
                     driverParam->getCostA() * curConnData->getCostA(vehRes) +
                     driverParam->getCostB() * curConnData->getCostB(vehRes) +
                     driverParam->getCostC() * curConnData->getCostC(vehRes);

               if( cost < prevNode->getEstCost(m_map) ) {
                  prevNode->setEstCost(m_map, cost );
                  prevNode->setGradient(m_map, curNode );
                  m_throughfarePriorityQueue->enqueue( prevNode );
                  mc2dbg8 << "Adding a node to m_throughfarePriorityQueue(3) "
                          << hex << curNode->getItemID() << dec << endl;
                  resetHeapThroughfare->enqueue( prevNode );
               }
            } else if( prevNode->getEstCost(m_map) != MAX_UINT32 ) {
               mc2dbg8 << "Adding a node to m_normalPriorityQueue "
                       << endl ;
               m_normalPriorityQueue->enqueue( prevNode );
            } else {
               // This is for a special case that the item
               //  is not possible to come from
               mc2dbg << "Node not added " << endl;
            }
         }
         curConnection = curConnection->getNext();
      }         
   }

   // First reset the nodes
   FileDebugCalcRoute::writeComment
      ("Resetting nodes in resetHeapThroughfare");
   while( !resetHeapThroughfare->isEmpty() ){
      RoutingNode* curNode = resetHeapThroughfare->dequeue();
      curNode->reset(m_map);
   }
   m_throughfarePriorityQueue->reset();
}

void
CalcRoute::expandNodesResult( const RMDriverPref* driverParam,
                              bool forward  )
{
   const bool usingCostC = driverParam->getCostC() != 0;
   mc2dbg << "CalcRoute::expandNodesResult" << endl;
   FileDebugCalcRoute::writeComment("expandNodesResult");

   DEBUG1(int nbrDequeued = 0);
   
   while( !m_normalPriorityQueue->isEmpty() ){
      DEBUG1(++nbrDequeued);
      RoutingNode* curNode = 
         m_normalPriorityQueue->dequeue();
      
      if( curNode->isDest(m_map) ) {
         mc2dbg << "[CR]: enr found dest "
                << hex << curNode->getItemID() << dec << endl;
         DEBUG4(tracePath(curNode, driverParam));
      } /* else */
      // We should probably continue routing even if the node was
      // a destination
      {
         RoutingConnection* curConnection =
            curNode->getFirstConnection( forward ); 
         
         while( curConnection != NULL ) {
            RoutingConnectionData* curConnData = curConnection->getData();

            RoutingNode* nextNode = curConnection->getNode();
            
            uint32 tempCost = calcConnectionCostWalk(driverParam,
                                                     curConnData);

            if ( tempCost > m_cutOff ) {
               // Next, please
               curConnection = curConnection->getNext();
               continue;
            }

            const bool canWalk = ItemTypes::pedestrian &
                               curConnData->getVehicleRestriction(usingCostC);
            const bool canDrive = driverParam->getVehicleRestriction() &
                               curConnData->getVehicleRestriction(usingCostC);
            const bool restrictionOK = NOT_VALID(nextNode->getRestriction());

            // There is really no difference between the non-valid
            // nodes and the other ones. If we are here it means that
            // we could not drive to the destination and that all nodes
            // should be calculated as walking.

            // Don't know how the visited flag is supposed to work
            // in this case. In the first part of the routing it is
            // used to mark the nodes that are reached validly.

            if ( ! nextNode->isVisited(m_map) ) {
               
               if ( ( canWalk && ( (!canDrive) || (!restrictionOK) ) ) ||
                    ( curNode->isVisited(m_map) ) ) {
                  
                  uint32 cost = curNode->getEstCost(m_map) + tempCost;
                  
                  if( cost < nextNode->getRealCost(m_map) ) {
                     nextNode->setRealCost(m_map, cost );
                     nextNode->setEstCost(m_map, cost );
                     nextNode->setGradient(m_map, curNode );
                     // Use the visited flag to ensure that we dont 
                     // pass in and out of a non valid zone
                     //nextNode->setVisited(m_map,  true );
                     m_normalPriorityQueue->enqueue( nextNode );
                  }
               } else if( HAS_NO_THROUGHFARE( nextNode->getRestriction() ) ||
                          (canDrive && restrictionOK && canWalk ) ) {
                  uint32 cost = curNode->getEstCost(m_map) + tempCost;
                  if( cost < nextNode->getRealCost(m_map) ){
                     nextNode->setRealCost(m_map, cost );
                     nextNode->setEstCost(m_map, cost );
                     nextNode->setGradient(m_map, curNode );
                     // Use the visited flag to ensure that we dont 
                     // pass in and out of a non valid zone
                     nextNode->setVisited(m_map,  false );
                     
                     m_normalPriorityQueue->enqueue( nextNode );
                  }
               }
            }
            curConnection = curConnection->getNext();
         } 
      }
   }
   DEBUG1(mc2dbg << "NbrDequeueued = " << nbrDequeued << endl);
}

void
CalcRoute::insertStateElement( CompleteRMSubRoute* completeSubRoute,
                               const RMDriverPref* driverParam,
                               bool forward )
{  
   const bool usingCostC = driverParam->getCostC() != 0;
   // FIXME: Handle the case of two segments....
   mc2dbg4 << "CalcRoute::insertStateElement" << endl;
   FileDebugCalcRoute::writeComment("insertStateElement");
   if( completeSubRoute != NULL ){
      Vector* nodeIDs = completeSubRoute->getNodeIDs();

      uint32 currentState = MAX_UINT32;
      if( IS_DRIVING( driverParam->getVehicleRestriction() ) ){
         currentState = RouteConstants::DRIVE_ITEM_ID;
      } else {
         currentState = RouteConstants::WALK_ITEM_ID;
      }
      // XXX This should be done!!
      if ( currentState != RouteConstants::DRIVE_ITEM_ID) {
         nodeIDs->insertElementAt( 0, currentState );
      }

      if ( currentState == RouteConstants::WALK_ITEM_ID )
         return;
    
      // Check if we need to park and walk, or go by bus

      // We check two at a time so..
      int nbrNodes = int(nodeIDs->getSize()) - 1;
      int newIndex = -1;
      int oldIndex = -2;
      
      for( int i = nbrNodes-1; i >= 0 && newIndex != oldIndex; --i ) {
         mc2dbg8 << "[CR]: insert state i = " << i << endl;
         oldIndex = newIndex;
         RoutingNode* node1 =
            m_map->getNodeFromTrueNodeNumber((*nodeIDs)[i] );
         RoutingNode* node2 =
            m_map->getNodeFromTrueNodeNumber((*nodeIDs)[i+1] );
         
         if ( node1 == NULL || node2 == NULL )
            continue;
         RoutingConnection* conn = node1->getConnection( node2, forward );
         if ( conn == NULL ) {
            if ( i ) {
               mc2log << warn << "conn is NULL in insertStateElement i = "
                      << i
                      << endl;
            }
            continue;
         }

         // Check node restriction
         if ( NOT_VALID( node2->getRestriction() ) ) {
            mc2dbg4 << "Inserting node2 state element at " << i+1 << endl;
            currentState = RouteConstants::WALK_ITEM_ID;
            newIndex = i+1;
         }

//           if ( ! HAS_NO_RESTRICTIONS(node1->getRestriction() ) ) {
//              mc2dbg << "Inserting node1 state element at " << i << endl;
//              currentState = RouteConstants::WALK_ITEM_ID;
//              index = i;
//           }
         
         RoutingConnectionData* connData = conn->getData();
         // Check vehicle restrictions
         if ( ( driverParam->getVehicleRestriction() &
                connData->getVehicleRestriction(usingCostC) ) == 0 ) {
            mc2dbg4 << "Inserting veh state element at " << i+1 << endl;
            newIndex = i+1;
         }

         if ( ! node2->isVisited(m_map) ) {
            newIndex = i+1;
         }
      }
   
      if ( newIndex > 0 ) {
         // Insert the state element at 0 instead...
         if ( newIndex == 1 )
            newIndex = 0;
         mc2dbg << "Inserting state element at " << newIndex << endl;
         nodeIDs->insertElementAt(newIndex, RouteConstants::WALK_ITEM_ID);
      }
   }
}



////////////////////////////////////////////////////////
// Debug
////////////////////////////////////////////////////////

void CalcRoute::tracePath( RoutingNode* node,
                           const RMDriverPref* driverParam )
{
   mc2log << info << "Tracing path " << endl
          << "costA " << driverParam->getCostA() << " costB "
          << driverParam->getCostB() << endl;
   while( node != NULL ){
      mc2log << "ID " << hex << node->getItemID() << dec
             << " cost " << node->getEstCost(m_map) << endl;

      node = node->getGradient(m_map); 
   }
   mc2log << endl << "Done tracing path" << endl;
}
   
void
CalcRoute::printNbrExternalNodesVisited()
{
   mc2log << info << "printNbrExternalNodesVisited" << endl;
   
   RoutingNode* externalNodesArray = m_map->getExternalNodes();
   if (externalNodesArray != NULL) {
      int nbrVisitedExternalNodes = 0;
      
      for (uint32 i = 0; i < m_map->getNumExternalNodes(); i++) {
         RoutingNode* realNode =
            m_map->getNodeFromTrueNodeNumber(
               externalNodesArray[i].getItemID());
         if (realNode != NULL)
            if (realNode->getEstCost(m_map) != MAX_UINT32)
               nbrVisitedExternalNodes++;
      }
      
      mc2log << "Number of visited external nodes of map " <<
         m_map->getMapID() << " is " << nbrVisitedExternalNodes <<
         " out of " << m_map->getNumExternalNodes() << endl;
   }   
   else {
      mc2log << "This map has no external nodes, thus none visited" << endl;
      if (m_map->getNumExternalNodes() > 0) {
         mc2log << error <<"Something is very wrong, m_nbrExternalNodes is"
                << m_map->getNumExternalNodes() << endl; 
      }
   }
} // printNbrExternalNodesVisited


inline bool
CalcRoute::isOnLevel(uint32 level,
                     const RoutingNode& nodeToCheck)
{
   // FIXME: This function always returns true.
   if ( level == 0 ) {
      return true;      
   } else {
      // Return true for now since the translation will remove the unwanted
      // ones
      return true;
      // This could be unnecessary since the translation requests
      // will remove the ones that are invalid.
      const uint32 zoomLevel = GET_ZOOM_LEVEL(nodeToCheck.getItemID());
      return zoomLevel <= (uint32)OverviewMap::maxZoomLevelForMapLevel[level];
   }
}

//////////////////////////////////////////////////////////////////
// Public functions.
//////////////////////////////////////////////////////////////////

void
CalcRoute::getExternalNodesToLevel(int level,
                                   vector<uint32> &externalNodesOut,
                                   vector<int32>& lats,
                                   vector<int32>& lons,
                                   map<uint32,uint32>& distances,
                                   const OrigDestInfoList& dests,
                                   const RMDriverPref& driverPrefs)
{
   // Get the external nodes from the map.
   RoutingNode* externalNode = m_map->getExternalNodes();
   int nbrExternal = m_map->getNumExternalNodes();

   // Reserve some extra space in the vector
   externalNodesOut.reserve(nbrExternal);
   lats.reserve(nbrExternal);
   lons.reserve(nbrExternal);

   if ( driverPrefs.getVehicleRestriction() == MAX_UINT32 ) {
      // Return all nodes.  
      for(int i=0; i < nbrExternal; ++i ) {
         if ( isOnLevel( level, externalNode[i] ) ) {
            externalNodesOut.push_back(externalNode[i].getItemID());
            lats.push_back(externalNode[i].getLat());
            lons.push_back(externalNode[i].getLong());
         }
      }
   } else {
      // Make OrigDestNodes
      Head* destList = new Head;
      for( int i = 0; i < nbrExternal; ++i ) {
         if ( isOnLevel( level, externalNode[i] ) ) {
            OrigDestNode* newNode =
               m_map->newOrigDestNode( IDPair_t(m_map->getMapID(),
                                                externalNode[i].getItemID()),
                                       0 ); // Offset
            newNode->into(destList);
         }
      }
      // Remove OrigDestNodes.
      // Does not work for some unknown reason.....
      //int nbrRemoved = removeUnreachable(destList, &driverPrefs, !true);

      // Add all nodes for now.
      int nbrRemoved = 0;
      mc2dbg4 << "[CR]: Removed " << nbrRemoved << " of "
              << nbrExternal
              << "in edgenode request"
              << endl;

      OrigDestNode* nextNode = NULL;
      // Put the rest into the answer and remove the listnodes
      for( OrigDestNode* curNode =
              static_cast<OrigDestNode*>(destList->first());
           curNode != NULL;
           curNode = nextNode) {
         RoutingNode* realNode =
            m_map->getNodeFromTrueNodeNumber(curNode->getItemID());
         externalNodesOut.push_back(curNode->getItemID());
         lats.push_back(realNode->getLat());
         lons.push_back(realNode->getLong());
         // Save the next node and delete the current one.
         nextNode = static_cast<OrigDestNode*>(curNode->suc());
         curNode->out();
         delete curNode;
      }
   }

   if ( !dests.empty() ) {
      // The distance that the server will compare the distances to.
      // Not sent but will not be smaller than 0. (squared)
      const uint32 serverDistance = 0*0;

      // Map containing the distances.
      map<uint32, double> distMap;
      // Make sure that we have something in the map.
      for ( set<uint32>::const_iterator it =
               m_map->getNeighbourMapIDs()->begin();
            it != m_map->getNeighbourMapIDs()->end();
            ++it) {
         distMap.insert(make_pair(*it, MAX_FLOAT64));
      }
      
      // Calculate the distances to the closest maps.
      for( OrigDestInfoList::const_iterator it = dests.begin();
           it != dests.end();
           ++it ) {
         for ( int i=0; i < nbrExternal; ++i ) {
            double dist = -1;

            // For all connections...
            for ( ExternalRoutingConnection* curConn =
                     (ExternalRoutingConnection*)
                     externalNode[i].getFirstConnection(true);
                  curConn != NULL;
                  curConn = (ExternalRoutingConnection*)curConn->getNext()) {
               // Check if the distance to the current map is too small
               // or not.
               if ( distMap[curConn->getMapID()] > serverDistance ) {
                  // The distance to this map is still too large.
                  if ( dist < 0 ) {
                  // The distance is needed. Calculate it.
                     dist = 
                        GfxUtility::squareP2Pdistance_linear(it->getLat(),
                                                             it->getLon(),
                                                    externalNode[i].getLat(),
                                                    externalNode[i].getLong());
                  }
                  if ( dist < distMap[curConn->getMapID()] ) {
                     distMap[curConn->getMapID()] = dist;
                  }
               } else {
                  // Skip all external nodes to the map. It is already done.
               }
            }
         }
      }
      
      for(map<uint32, double>::const_iterator it = distMap.begin();
          it != distMap.end();
          ++it ) {
         double realDist = ::sqrt(it->second);
         mc2dbg << "[CR]: Closest distance to map " 
                << prettyMapIDFill(it->first) << " is " << realDist << endl;
         if ( realDist < MAX_UINT32 ) {
            distances.insert(make_pair(it->first, uint32(realDist)) );
         } else {
            // Can this happen?
            distances.insert(make_pair(it->first, MAX_UINT32) );
         }
      }
   }
}

uint32
CalcRoute::getHigherLevelMapID(int higherlevel) const
{
   return m_map->getMapAtLevel(higherlevel);
}

void
CalcRoute::getNeighbourMapIDs(set<uint32> &neighbourMapIDs)
{
   const set<uint32>* mapIDs = m_map->getNeighbourMapIDs();
   neighbourMapIDs = *mapIDs;
}

inline uint32
CalcRoute::realRoute(Head* origin, 
                     Head* destination,
                     Head* allDestinations,
                     SubRouteList* incomingList,
                     SubRouteList* resultList,
                     const RMDriverPref* driverParam,
                     bool originalRequest,
                     bool routeToAll,
                     bool calcCostSums,
                     bool sendSubRoutes)
{
   mc2dbg << "RouteToAll has value " << BP(routeToAll) << endl;
   mc2dbg << "SendSubRoutes has value " << BP(sendSubRoutes) << endl;
   uint32 status = StringTable::OK;
   
   //bool routeToHigherLevel = incomingList->getRouteOnHigherLevel();
   bool forward            = incomingList->isForwardRouting();
   m_cutOff                = incomingList->getCutOff();

   // Depending on previous result we should do different things
   switch(incomingList->getListType() ) {

      case SubRouteListTypes::PROXIMITY_REQUEST : {
         // Will not handle proximity requests.
         status = StringTable::NOTSUPPORTED;         
      }
      break;
      case SubRouteListTypes::HIGHER_LEVEL_BACKWARD: // This fall through is OK
      case SubRouteListTypes::HIGHER_LEVEL_FORWARD: {
         status = StringTable::NOTSUPPORTED;
      }
      break;

      case SubRouteListTypes::HIGHER_LEVEL:{
         status = StringTable::NOTSUPPORTED;
      }      
      break;
      
      case SubRouteListTypes::LOWER_LEVEL : {

         // Check if there are routes on one segment
         // Add the routes to the result.
//           if ( destination->cardinal() == 0 ) {
//              RoutingNode* externalNode = m_map->getExternalNodes();
//              RoutingNode* realNode = m_map->getNodeFromTrueNodeNumber(
//                 externalNode->getItemID() );
//              for( uint32 i=0; i < m_map->getNumExternalNodes(); ++i ) {
//                 OrigDestNode* dest =
//                    m_map->newOrigDestNode(realNode->getIndex(),
//                                           m_map->getMapID(),
//                                           0,
//                                           realNode->getLat(),
//                                           realNode->getLong(),
//                                           realNode->getRealCost(m_map),
//                                           realNode->getEstCost(m_map),
//                                           0, 0,0);
//                 dest->setItemID( realNode->getItemID());
//                 dest->into(destination);
//              }
//           }
         int nbrSameSeg = routeOnOneSegment(origin,
                                            destination,
                                            driverParam,
                                            forward,
                                            incomingList,
                                            resultList,
                                            routeToAll,
                                            calcCostSums);

         if ( nbrSameSeg )
            mc2dbg << "NbrSameSeg = " << nbrSameSeg << endl;
         
         // Remove the duplicate itemID:s
         // Keep the ones with the lowest offset if
         // dup was found.
         int origDups = removeDups(origin, false);
         int destDups = removeDups(destination, true);

         if ( origDups )
            mc2dbg << "NbrOrigDups = " << origDups << endl;
         if ( destDups )
            mc2dbg << "NbrDestDups = " << destDups << endl;
                  
         Head* unreachList = NULL;
         if ( MapBits::isOverviewMap(m_map->getMapID() ) ) {
            // Remove the destinations that cannot be reached.
            unreachList = 
               removeUnreachable(destination,
                                 driverParam,
                                 !forward);
            mc2dbg << "NbrUnreach = " << unreachList->cardinal() << endl;
         }
         
         mc2dbg << "[CR]: NbrDest " << destination->cardinal() << endl;
         
         status = initRoute(originalRequest,
                            origin,
                            destination,
                            driverParam,
                            false,  // Don't route to higher level
                            forward);
         
         if (status != StringTable::OK ) {
            return status;
         }

         // Calculate the route
         if ( destination->cardinal() > 0 ) {
            // Origin and destination lies in the same map

            // FIXME: Find a better way to test if it should be
            //        all destinations or not.
            if ( routeToAll == false && m_map->getMapID() < 0x80000000) {
               calcCostDijkstra(driverParam,
                                destination,
                                forward,
                                routeToAll); // One or many origins?
            } else {
               mc2dbg << "Routing to many many" << endl;
               // FIXME: Don't do it this way
               // FIXME: That is - figure out a better way to decide
               // FIXME: when to route to the edges and when not to.
               calcCostDijkstra(driverParam,
                                destination,
                                forward,
                                routeToAll);
            }

            // Add the unreachable again.
            if ( unreachList != NULL ) {
               for( OrigDestNode* curDest =
                       static_cast<OrigDestNode*>(unreachList->first());
                    curDest != NULL;
                    curDest = static_cast<OrigDestNode*>(curDest->suc())) {
                  curDest->out();
                  curDest->into(destination);
               }
               delete unreachList;
            }
            
            if ( routeToAll == false ) {
               status = readResult(origin,
                                   destination,
                                   incomingList,
                                   resultList,
                                   driverParam,
                                   forward,
                                   calcCostSums);
            } else {
               status = readResultToAll(origin,
                                        destination,
                                        incomingList,
                                        resultList,
                                        driverParam,
                                        forward,
                                        calcCostSums);
               // Blaargh! Read all results.
               m_cutOff = MAX_UINT32;
               readResultToExternalConnection( incomingList,
                                               resultList,
                                               driverParam,
                                               origin,
                                               allDestinations,
                                               forward,
                                               calcCostSums);
#if 0
               for( OrigDestNode* curDest =
                       static_cast<OrigDestNode*>(destination->first());
                    curDest != NULL;
                    curDest = static_cast<OrigDestNode*>(curDest->suc())) {
                  RoutingNode* realNode =
                     m_map->getNodeFromTrueNodeNumber(curDest->getItemID());
                  if ( realNode->getRealCost(m_map) == MAX_UINT32 ) {
                     mc2dbg8 << "[CR]: Did not reach " << hex
                             << curDest->getItemID() << dec << endl;
                  } else {
                     mc2dbg8 << "[CR]: Reached " << hex
                             << curDest->getItemID() << dec << endl;
                  }
               }
#endif          
               if ( sendSubRoutes == false ) {
                  mc2dbg << "Removing nodes from answer" << endl;
                  int resSize = resultList->getNbrSubRoutes();
                  for(int i=0;i < resSize ; ++i ) {
                     CompleteRMSubRoute* sr =
                        static_cast<CompleteRMSubRoute*>
                        (resultList->getSubRoute(i));
                     Vector* nodeIDs = sr->getNodeIDs();
                     // Does not seem to work with server!!
                     // nodeIDs->reset();
                     mc2dbg8 << nodeIDs << endl;
                  }
               }
            }
         } else {
            // Origin and destination does not lie within the same map
            // Check if we should route to higher level

            if ( m_map->getNumExternalNodes() == 0 ) {
               mc2log << info << "[CR]: No external nodes in this map"
                      << endl;
               break;
            }
            
            Head* origList = forward ? ( origin ) : ( destination );
            Head* destList = allDestinations; // Will this work backwards?
            OrigDestNode* curOrigDest =
               static_cast<OrigDestNode*>(origList->first());
            uint32 minCost = MAX_UINT32;
            while ( curOrigDest != NULL ) {               
               uint32 distCost = estimateDistToDest( curOrigDest,
                                                     destList,
                                                     1,
                                                     0,
                                                     0,
                                                     0);
               minCost = MIN(distCost, minCost);
               curOrigDest = static_cast<OrigDestNode*>(curOrigDest->suc());
            }

            // FIXME: Use constant for distance.
            static const uint32 compareCost =
               Connection::metersToDistCost(uint32(80*1000));
            
            if ( originalRequest && (minCost > compareCost) ) {
               m_outsideHeap->reset();
               updateLowerLevelBBox(origList);
               calcCostExternalDijkstra( driverParam,
                                         false, // Never route to higher
                                         forward,
                                         allDestinations);

               RoutingNode* externalNode = m_map->getExternalNodes();
               int nbrExtRoutes = 0;
               for (uint32 i = 0;
                    i < m_map->getNumExternalNodes() && nbrExtRoutes < 3;
                    ++i) {
                  RoutingNode* curNode = 
                     m_map->getNodeFromTrueNodeNumber(
                        externalNode[i].getItemID() );
                  if ( curNode->getGradient(m_map) != NULL ) {
                     ++nbrExtRoutes;
                  }
               }
               if ( nbrExtRoutes > 1 ) {
                  uint32 startTime = TimeUtility::getCurrentTime();
                  readResultToExternalConnection( incomingList,
                                                  resultList,
                                                  driverParam,
                                                  origin,
                                                  allDestinations,
                                                  forward,
                                                  calcCostSums);
                  uint32 stopTime = TimeUtility::getCurrentTime();
                  mc2dbg << "Number of subRoutes in extresultList [2] = "
                         << resultList->getNbrSubRoutes() << " Time:" 
                         << (stopTime-startTime) << " millis" << endl;
               } else {
                  // Blargh! No external route found, enqueue
                  // again
                  mc2dbg << "[CR] Enqueueing nodes again and will consider"
                            " all nodes" << endl;
                  while ( ! m_outsideHeap->isEmpty() ) {
                     m_priorityQueue->enqueue( m_outsideHeap->dequeue());
                  }

                  // Re-route
                  calcCostExternalDijkstra( driverParam,
                                            false, // Don't route to higher
                                            forward,
                                            allDestinations);
                  
                  readResultToExternalConnection( incomingList,
                                                  resultList,
                                                  driverParam,
                                                  origin,
                                                  allDestinations,
                                                  forward,
                                                  calcCostSums);
                  mc2dbg << "[CR] The result list now contains "
                         << resultList->getNbrSubRoutes() << " subroutes"
                         << endl;
               }
            } else {
               mc2dbg << "[CR]: Routing on all nodes" << endl;               
               
               calcCostExternalDijkstra( driverParam,
                                         false, // Don't route to higher
                                         forward,
                                         allDestinations);

               uint32 startTime = TimeUtility::getCurrentTime();
               readResultToExternalConnection( incomingList,
                                               resultList,
                                               driverParam,
                                               origin,
                                               allDestinations,
                                               forward,
                                               calcCostSums);
               uint32 stopTime = TimeUtility::getCurrentTime();
               mc2dbg << "Number of subRoutes in extresultList [1] = "
                      << resultList->getNbrSubRoutes() << " Time:" 
                      << (stopTime-startTime) << " millis" << endl;
            }

         }
         mc2dbg << "[CR] Size of resultList = "
                << resultList->getNbrSubRoutes()
                << endl;
      }
      break;
      
      case SubRouteListTypes::LOWER_LEVEL_WALK: {
          status = initRoute(originalRequest,
                             origin,
                             destination,
                             driverParam,
                             false,  // Don't route to higher level
                             forward);
//         status = initRouteWalking( origin, 
//                                    destination, 
//                                    driverParam, 
//                                    forward );
         
         // Should the status be not ok, nothing further will be done.
         if (status != StringTable::OK) {
            return status;
         }
         
         // Calculate the route
         // Use calcCostDijkstra if at least one destination lie within
         // the map of this CalcRoute object.
         if (destination->cardinal() > 0) {
            calcCostDijkstra( driverParam, destination, forward );
         }
         else { // no dest in this map, route to external in low level
            calcCostExternalDijkstra(driverParam, false, forward,
                                     allDestinations);
         }
         
         status = readResultWalk( origin,
                                  destination,
                                  incomingList,
                                  resultList,
                                  driverParam,
                                  forward,
                                  calcCostSums);         
      }
      break;
      
      case SubRouteListTypes::PUBLIC_TRANSPORTATION : {
         status = StringTable::NOTSUPPORTED;
      }
      break;
      
      default: {
         MC2WARNING("CalcRoute::route: no subroute type found");
      }
      break;
   }
   
   resultList->setCutOff( m_cutOff );

   return status;
} // realRoute

uint32
CalcRoute::route(Head* origin, 
                 Head* destination,
                 Head* allDestinations,
                 SubRouteList* incomingList,
                 SubRouteList* resultList,
                 const RMDriverPref* driverParam,
                 bool originalRequest,
                 bool routeToAll,
                 const DisturbanceVector* disturbances,
                 bool calcCostSums,
                 bool sendSubRoutes)
{
   // Ugly. Set the default penalties for toll roads
   Connection::tollRoadTimeDefaultPenalty_s =
      Properties::getUint32Property(
         "ROUTE_TOLL_ROAD_TIME_PENALTY_S",
         Connection::tollRoadTimeDefaultPenalty_s);
   
   Connection::tollRoadDistanceDefaultPenalty_m =
      Properties::getUint32Property(
         "ROUTE_TOLL_ROAD_DIST_PENALTY_M",         
         Connection::tollRoadDistanceDefaultPenalty_m);

   Connection::highwayDefaultPenaltyFactor =
      Properties::getUint32Property(
         "ROUTE_HIGHWAY_PENALTY_FACTOR_PERCENT",
         uint32(Connection::highwayDefaultPenaltyFactor * 100)) / 100.0;
   
   mc2dbg2 << "DriverPref costs :"
           << int(driverParam->getCostA()) << ":"
           << int(driverParam->getCostB()) << ":"
           << int(driverParam->getCostC()) << ":"
           << int(driverParam->getCostD()) << endl;
   
   FileDebugCalcRoute::openFile();
   
#ifdef USE_RESET_THREAD_IN_CALCROUTE
   mc2dbg << "route waiting for resethread" << endl;
   waitForResetThread();
#else
   // Reset the map before routing
   resetToStartState();
#endif
   
   // Add disturbances if any
   if ( disturbances != NULL ) {
      m_map->addDisturbances(disturbances);
   }

   // Important! Set the infinity of all the destinations to the inf
   // of the newly reset map.
   for( OrigDestNode* curDest =
           static_cast<OrigDestNode*>(destination->first());
        curDest != NULL;
        curDest = static_cast<OrigDestNode*>(curDest->suc())) {
      curDest->setInfinity(m_map->getInfinity());
      // Print 
      mc2dbg8 << "[CR]: Destination at 0x"
              << hex << curDest->getItemID() << endl;
   }

   // Important! Set the infinity of all the origins to the inf
   // of the newly reset map.
   for( OrigDestNode* curOrig = static_cast<OrigDestNode*>(origin->first());
        curOrig != NULL;
        curOrig = static_cast<OrigDestNode*>(curOrig->suc())) {
      curOrig->setInfinity(m_map->getInfinity());
      // Print origs
      mc2dbg8 << "[CR]: Origin at 0x"
              << hex << curOrig->getItemID() << dec
              << " cost = " << curOrig->getEstCost(m_map) << endl;
   }
  
   uint32 result = realRoute(origin, destination, allDestinations,
                             incomingList, resultList, driverParam,
                             originalRequest, routeToAll, calcCostSums,
                             sendSubRoutes);

   if ( disturbances != NULL ) {
      m_map->rollBack(true);
   }
   
#ifdef USE_RESET_THREAD_IN_CALCROUTE
   // We're done with the map - start the thread
   mc2dbg << "CalcRoute will start the resetthread" << endl;
   startResetThread();
#endif
   
   // Close the file (if file debugging is on);
   FileDebugCalcRoute::closeFile();
   return result;
}

void
CalcRoute::trickTheOptimizer(uint32&)
{   
}

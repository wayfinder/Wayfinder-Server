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

#include "RouteTypes.h"
#include "ServerSubRouteVector.h"
#include "SubRoute.h"
#include "OrigDestInfo.h"
#include "LevelTransitObject.h"
#include "OverviewMap.h"

#include "SubRouteContainer.h"

#include "Connection.h"
#include "MapBits.h"

#include <sstream>

SubRouteContainer::SubRouteContainer( )
      : multimap<uint32, SubRoute*>(),
        m_allowedMaps( NULL )
{
   setCutOff(MAX_UINT32);
}   

SubRouteContainer::SubRouteContainer(const OrigDestInfoList* destInfoList)
      : multimap<uint32, SubRoute*>(),
        m_allowedMaps( NULL )
{
   setCutOff(MAX_UINT32);
   
   OrigDestInfoList::const_iterator it = destInfoList->begin();
   while ( it != destInfoList->end() ) {
      m_destMaps.insert( it->getMapID() );
      it++;
   }
}

SubRouteContainer::~SubRouteContainer()
{
   iterator it;

   it = begin();
   while ( it != end() ) {
      deleteSubRoute( it );
   }
}

void
SubRouteContainer::moveAllSubRoutes(
   SubRouteVector& resultVector,
   ServerSubRouteVector& finishedSubRouteVector)
{
   iterator it = begin();   
   while ( it != end() ) {
#if 1
      bool doMove = true;
      SubRoute* curSubRoute = it->second;
      if ( curSubRoute->getNextMapID() == curSubRoute->getThisMapID() ) {
         mc2dbg2 << "[SRC]: Already same map - skipping" << endl;
         doMove = false;
      }
      if ( MapBits::getMapLevel(curSubRoute->getNextMapID()) !=
           MapBits::getMapLevel(curSubRoute->getThisMapID() ) ) {
         mc2dbg2 << "[SRC]: Diffing levels - skipping" << endl;
         doMove = false;
      }
      
//        if ( MapBits::getMapLevel(curSubRoute->getNextMapID() ) < 1 ) {
//           mc2dbg2 << "[SRC]: Level is too low" << endl;
//           doMove = false;
//        }
         

      // If size of vector is zero, we can not subract the last element
      // and thus unfortunately need to send EdgeNodesRequestPacket
      // on next map as well.
      // A different way of doing this could be to use the
      // previous SubRoute instead, and throw this one away. This
      // would require some more investigation, and would still not
      // solve the problem where we start on the edge of a map.

      if ( doMove ) {
         finishedSubRouteVector.insertSubRoute( new SubRoute(*it->second),
                                                false );
         int back_idx = curSubRoute->size() - 1;
         while ( back_idx >= 0 &&
                 GET_TRANSPORTATION_STATE((*curSubRoute)[back_idx] ) ) {
            --back_idx;
            mc2dbg2 << "[SRC]: 1. Not using node " << hex
                    << curSubRoute->back() << dec << endl;
            curSubRoute->pop_back();
         }
         
         if ( back_idx >= 0 ) {
            MC2_ASSERT( !(GET_TRANSPORTATION_STATE((*curSubRoute)[back_idx])));
            MC2_ASSERT( (*curSubRoute)[back_idx] == curSubRoute->back() );
            mc2dbg8 << "[SRC]: 2. Not using node " << hex
                    << IDPair_t(curSubRoute->getNextMapID(),
                                (*curSubRoute)[back_idx]) << dec << endl;
            
            curSubRoute->setDestNodeID((*curSubRoute)[back_idx]);
            curSubRoute->setNextMapID(curSubRoute->getThisMapID() );
            
            // FIXME: Pop more if back_idx != last
            //curSubRoute->pop_back();
         } else {
            mc2dbg << "[SRC]: Too few nodes setting start=dest"
                   << endl;
            curSubRoute->setNextMapID( curSubRoute->getThisMapID() );
            curSubRoute->setDestNodeID( curSubRoute->getOrigNodeID() );
         }
      }

      bool insertOK = true;
      for( SubRouteVector::iterator inn = resultVector.begin();
           inn != resultVector.end();
           /**/ ) {
         if ( (*inn)->getDestID() == curSubRoute->getDestID() ) {
            // Check which one to keep
            if ( (*inn)->getCost() < curSubRoute->getCost() ) {
               mc2dbg2 << "[SRC]: Already exists cheaper route " << endl;
               insertOK = false;
               ++inn;
               continue;
            } else {
               // Delete?!
               delete *inn;
               inn = resultVector.erase(inn);
               continue;
            }
         }
         ++inn;
      }
#endif
      resultVector.insertSubRoute( it->second );      
      erase( it++ );
   }
}


uint32
SubRouteContainer::dequeueSubRoutesFromBestMap(
   SubRouteVector& resultVector,
   LevelTransitObject* levelTransitObject,
   int level)
{
   if ( empty() ) {
      // This shouldn't really happen, but we should avoid crashing
      // if it does.
      mc2dbg << "SRC: No map loaded, container was empty from the start."
                " Level is "
             << level
             << endl;
      return MAX_UINT32;
   }

   // Remove maps that are not allowed according to the LevelTransitObject
   uint32 dequeueMapID = begin()->second->getNextMapID();
   // Remove all the unallowed ones. They can be used further down.
   while ( levelTransitObject != NULL &&
           /* !levelTransitObject->allowedMapID( dequeueMapID ) && */
           !empty() ) {
      // The current first map in this container was not allowed
      // Remove all routes with that map as destination.      
      iterator it = begin();   
      while ( it != end() ) {
         if ( !levelTransitObject->allowedMapID(it->second->getNextMapID()) ) {
            delete it->second;
            erase( it++ );
         } else {
            ++it;
         }
      }
      if ( !empty() ) {
         dequeueMapID = begin()->second->getNextMapID();
         break;
      } else {
         // We have removed everything!
         mc2dbg << "SRC: No map loaded, no allowed map in leveltransit."
                   " Level is "
                << level
                << endl;
         return MAX_UINT32;
      }
   }

   // Try some tricks, but only if we haven't got a cut off yet.
   bool foundDestMap = false;
   if ( m_cutOff == MAX_UINT32 ) {
      const char* cause = ""; // For debug

      // Try to take neighbor map if it is a destination map.
      // (We will probably get a cut off in that case)
      const_iterator it = begin();
      while ( it != end() && 
              ( m_destMaps.find(
                 it->second->getNextMapID() ) == m_destMaps.end() ) ) {
         ++it;
      }
      
      // Testing to choose map based on distance left to goal if
      // no destination map is neigbour.
      if ( it == end() ) {
         const_iterator it2 = begin();
         
         uint32 minDist = MAX_UINT32;
         while ( it2 != end() ) {
            if ( // Check that we haven't used it yet.
               m_mapsUsedForCutOff.find( it2->second->getNextMapID())
               == m_mapsUsedForCutOff.end() ) {
               uint32 distance = it2->second->getEstimatedCost() -
                  it2->second->getCost();
               
               mc2dbg8 << "Costdiff = " << distance << endl;
               
               if ( distance < minDist  ) {
                  minDist = distance;
                  it = it2;
               }
            }
            ++it2;
         }
         if ( it != end() ) {
            mc2dbg2 << "SRC: Distance " << minDist << " used" << endl;
            mc2dbg2 << "SRC: Est cost " << it->second->getEstimatedCost()
                   << endl;
            mc2dbg2 << "SRC: Cost     " << it->second->getCost()
                   << endl;
            cause = " because it's closer to the destmap";
         }
      } else {
         foundDestMap = true;
         cause = " because it is a destmap";
      }

      if ( it != end() ) {
         dequeueMapID = it->second->getNextMapID();
         if ( dequeueMapID != begin()->second->getNextMapID()) {
            mc2dbg2 << "SRC: Different mapID loaded: "
                   << hex << "0x" << dequeueMapID << dec
                   << " instead of 0x"
                   << hex << begin()->second->getNextMapID() << dec
                   << cause
                   << endl;
         } else {
            mc2dbg2 << "SRC: mapID loaded: 0x"
                   << hex << dequeueMapID << dec
                   << endl;
         }
      } else if ( empty() ) {
         mc2dbg2 << "SRC: No map loaded, container was empty. Level is "
                << level
                << endl;
      } else {
         mc2dbg2 << "SRC: mapID loaded: 0x"
                << hex << dequeueMapID << dec
                << endl;
      }            
   } else {
      mc2dbg2 << "SRC: Cutoff exists. mapID loaded: 0x"
             << hex << dequeueMapID << dec
             << endl;
   }         

   
   
   // FIXME: Don't dequeue the maps for cutoff more than once.
   if ( m_cutOff == MAX_UINT32 ) {
      m_mapsUsedForCutOff.insert(dequeueMapID);
   }
   
   // Finally put the maps with the selected map id into the
   // result vector.
   iterator it = begin();   
   while ( it != end() ) {
      if ( it->second->getNextMapID() == dequeueMapID ) {
         resultVector.insertSubRoute( it->second );
         erase( it++ );
      } else {
         ++it;
      }
   }
   return dequeueMapID;
}

uint32
SubRouteContainer::dequeueSubRoutesFromMap( SubRouteVector& resultVector,
                                            uint32 mapID)
{
   int startSize = resultVector.size();
   // Map of itemid and cost
   map<uint32, uint32> costMap;
   // Only make the map if there are routes to the mapID.
   bool costMapInited = false;
  
   iterator it = begin();   
   while ( it != end() ) {
      if ( it->second->getNextMapID() == mapID ) {
         // MapID found - check if costs are OK.
         if ( ! costMapInited ) {
            // Initialize the cost map
            int nbrInRes = startSize;
            for( int i=0; i < nbrInRes; ++i ) {
               costMap.insert( make_pair( resultVector[i]->getDestNodeID(),
                                          resultVector[i]->getCost() ) );
            }
            costMapInited = true;
         }

         // Look for the same node id in the map.
         map<uint32,uint32>::const_iterator found =
            costMap.find(it->second->getDestNodeID());
         
         if ( found == costMap.end() ) {
            // Not found. Just insert
            resultVector.insertSubRoute( it->second );
         } else {
            // Check if the cost is less.
            if ( found->second > it->second->getCost() ) {
               resultVector.insertSubRoute( it->second );
            } else {
               delete it->second;
            }
         }
         erase( it++ );
      } else {
         ++it;
      }
   }
   
   return resultVector.size() - startSize;
}

void
SubRouteContainer::insertSubRoute(SubRoute* pSubRoute)
{
   if ( m_allowedMaps != NULL && m_allowedMaps->find( 
           pSubRoute->getNextMapID() ) == m_allowedMaps->end() )
   {
      // Out of area, don't add
      mc2log << info 
             << "insertSubRoute subroute out of allowed map, mapid 0x"
             << hex << pSubRoute->getNextMapID() << dec << endl;
      delete pSubRoute;
   } else {
      insert( SubRoutePair( pSubRoute->getEstimatedCost(),
                            pSubRoute ) );
   }
}

bool
SubRouteContainer::checkAndInsertSubRoute(SubRoute* pSubRoute,
                                          ServerSubRouteVector* pVector )
{
   if ( m_allowedMaps != NULL && m_allowedMaps->find( 
           pSubRoute->getNextMapID() ) == m_allowedMaps->end() )
   {
      // Out of area, don't add
      mc2dbg2
             << "checkAndInsertSubRoute skipping subroute to map 0x"
             << hex << pSubRoute->getNextMapID() << dec << endl;
      delete pSubRoute;
      return false;
   }

   if ( GET_TRANSPORTATION_STATE(pSubRoute->getDestNodeID() ) ) {
      mc2log << error << "[SRC]: Cannot route to state element" << endl;
      exit(1);
   }
   
   // End test
   if ( pVector && pVector->containsCheaperThan( pSubRoute ) ) {
#ifdef DEBUG_LEVEL_4
      stringstream strstr;
      strstr << "[SRC]: Cheaper route to "
             << pSubRoute->getNextMapID()
             << ":" << hex << pSubRoute->getDestNodeID() << dec 
             << " exists in vector" << endl << ends;
      mc2dbg4 << strstr.str();
#endif
      delete pSubRoute;
      return false;
   } 

   iterator it = begin();

   bool inserted = false;

   int costDiff = 0;
   while ( it != end() && !inserted ) {

      if ( pSubRoute->getDestNodeID() == it->second->getDestNodeID() &&
           pSubRoute->getNextMapID() == it->second->getNextMapID() ) {
         
         // Found a route to the same node.         
         if ( pSubRoute->getCost() < it->second->getCost() ) {

            costDiff = it->second->getCost() - pSubRoute->getCost();
            mc2dbg8 << "[SRC]: New route is "
                    << Connection::timeCostToSec( uint32(costDiff) )
                    << " seconds cheaper" << endl;
            mc2dbg8 << "[SRC]: New cost is "
                  << Connection::timeCostToSec(it->second->getEstimatedCost())
                    << endl;
            // Delete and remove the one in the container.
            delete it->second;
            erase( it );
            inserted = true;
         } else {          
            mc2dbg4 << "[SRC]: Cheaper route to "
                    << pSubRoute->getNextMapID()
                    << ":" << hex << pSubRoute->getDestNodeID() << dec 
                    << " exists in container" << endl;
            delete pSubRoute;
            inserted = false;
         }
         // We're done and can return.
         if ( inserted ) {
            insertSubRoute( pSubRoute );
            
            // Experiment: Find all the routes that has this route as
            // starting point and lower their cost with the diff.
            const uint32 mapID  = pSubRoute->getDestNodeID();
            const uint32 nodeID = pSubRoute->getNextMapID();
            for ( iterator jt = begin(); jt != end(); ++jt ) {
               SubRoute* listRoute = jt->second;
               if ( listRoute->getThisMapID() == mapID &&
                    listRoute->getOrigNodeID() == nodeID ) {
                  // Lower the cost for that node with the
                  // difference
                  // FIXME: Look in the subroutelist too.
                  // FIXME: Rehash
                  // FIXME: Do it again.
                  mc2dbg << "[SRC]: Cost is spreading" << endl;
                  listRoute->setCost(listRoute->getCost() - costDiff);
               }
            }
         }
         return inserted;
      }
      ++it;
   }
   
   // Did not find an old route to the same node.
   mc2dbg8 << "[SRC]: New subroute to "
           << IDPair_t(pSubRoute->getNextMapID(), pSubRoute->getDestNodeID())
           << " found" << endl;
   mc2dbg8 << "[SRC]: EstCost is " << pSubRoute->getEstimatedCost() << endl;
   insertSubRoute( pSubRoute );
   return true;
}

void
SubRouteContainer::insertOrigDestInfo(const OrigDestInfo& insertInfo)
{
   if ( m_allowedMaps != NULL && m_allowedMaps->find( 
      insertInfo.getMapID() ) == m_allowedMaps->end() ) {
      
      mc2dbg2
             << "insertOrigDestInfo info out of allowed map, mapid 0x"
             << hex << insertInfo.getMapID() << dec << endl;
   } else {
      insert( SubRoutePair( insertInfo.getEstimatedCost(),
                            new SubRoute(
                               OrigDestInfo(
                                  insertInfo.getPrevSubRouteID() ),
                               insertInfo ) ) );
   }
}

void
SubRouteContainer::updateContainer(ServerSubRouteVector* srVect)
{
   iterator it = lower_bound( MAX(m_cutOff, m_cutOff + 1) );

   while ( it != end() ) {
      //deleteSubRoute( it );
      // Insert the subroute among the finished ones.
      srVect->insertSubRoute(it->second, false);
      erase(it++);
   }
}   

void
SubRouteContainer::putAllInto(SubRouteContainer& other,
                              ServerSubRouteVector* srVect)
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      if ( ! other.checkAndInsertSubRoute(new SubRoute(*it->second),
                                          srVect) ) {
         mc2dbg2 << "[SRC]: Cheaper route in SRVector" << endl;         
         // Look for it in the srVect instead
         const SubRoute* listSubRoute =
            srVect->containsCheaperThan(it->second);
         if ( listSubRoute ) {
            other.insertSubRoute(new SubRoute(*listSubRoute));
         }
      }/* else {*/
      // We clear below and copy above perhaps delete it to avoid memleak?
      delete it->second;
//      }
   }
   // This one should be cleared.
   clear();
}

void
SubRouteContainer::copyAllInto(SubRouteContainer& other,
                               ServerSubRouteVector* srVect)
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      if ( !other.checkAndInsertSubRoute(
         new SubRoute(*(it->second)), srVect) ) {
         // Look for it in the srVect instead
         const SubRoute* listSubRoute =
            srVect->containsCheaperThan(it->second);
         if ( listSubRoute ) {
            other.insertSubRoute(new SubRoute(*listSubRoute));
         }
      }
   }
}

int
SubRouteContainer::countNodesOnLevel(int level) const
{
   int nbr = 0;
   for ( const_iterator it = begin();
         it != end();
         ++it ) {
      if ( OverviewMap::existsOnLevel(level, it->second->getDestNodeID())) {
         ++nbr;
      }
   }
   return nbr;
}







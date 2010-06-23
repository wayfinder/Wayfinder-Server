/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TrafficHandler.h"

#include "TrafficIPC.h"
#include "TrafficSituation.h"
#include "TrafficSituationElement.h"
#include "TimeUtility.h"
#include "TrafficDataTypes.h"
#include "DisturbanceElement.h"
#include "DeleteHelpers.h"
#include "IDPairVector.h"

#include <algorithm>
#include <iterator>

using namespace std;

template <typename Container>
struct StoreStrippedElementsT {
   StoreStrippedElementsT( Container& cont ):m_cont( cont ) { }

   void operator() ( DisturbanceElement* disturbance ) {
      disturbance->stripData();
      m_cont.push_back( disturbance );
   }

   Container& m_cont;
};

template < typename T >
StoreStrippedElementsT< T > StoreStrippedElements( T& container ) {
   return StoreStrippedElementsT< T >( container );
}

TrafficHandler::TrafficHandler( const MC2String& provider,
                                TrafficIPC* communicator ):
   m_provider( provider ),
   m_storedElements(),
   m_communicator( communicator ) {
}

TrafficHandler::~TrafficHandler() {
   STLUtility::deleteValues( m_storedElements );
   delete m_communicator;
}

struct LessSituationRef {
   template < typename A, typename B >
   bool operator() ( const A* lhs, const B* rhs ) const {
      //TODO: We need to fix the case where we have no situation id from the
      //feed, aka look for crc and compare that.
      return lhs->getSituationReference() < rhs->getSituationReference();
   }
};

void TrafficHandler::setup() {
   if ( ! fetchSituationsFromModule() ) {
      mc2log << warn
             << "[TrafficHandler] failed to fetch situations from module!" << endl;
   }
   sort( m_storedElements.begin(), m_storedElements.end(),
         LessSituationRef() );
}

void createTrafficSituations( const TrafficHandler::Disturbances& disturbances,
                              TrafficHandler::SitCont& trafficSits ) {
   // for convenience
   typedef TrafficHandler::Disturbances Disturbances;
   typedef TrafficHandler::SitCont SitCont;
   typedef map<uint32, int32> RouteIndex2CoordMap;
   typedef vector< pair<int32, int32> > CoordCont;

   for ( Disturbances::const_iterator it = disturbances.begin();
         it != disturbances.end(); ++it ) {
      // create a new situation
      auto_ptr< TrafficSituation > trafficSit( new TrafficSituation() );

      const RouteIndex2CoordMap& latMap = (*it)->getLatMap();
      const RouteIndex2CoordMap& lonMap = (*it)->getLonMap();
      // first and last coordinate is enough since we will only use those two
      // and route between them anyway in the case if we have no TMC locations
      CoordCont coords;
      // the first coordinate
      coords.push_back( make_pair( latMap.begin()->second,
                                   lonMap.begin()->second ) );
      // the last coordinate
      coords.push_back( make_pair( latMap.rbegin()->second,
                                   lonMap.rbegin()->second ) );

      trafficSit->setSituationReference( (*it)->getSituationReference() );
      trafficSit->addSituationElement( 
            new TrafficSituationElement( "", // the elements sitRef.. 
                                         (*it)->getStartTime(),
                                         (*it)->getEndTime(),
                                         (*it)->getCreationTime(),
                                         (*it)->getType(),
                                         (*it)->getPhrase(),
                                         (*it)->getSeverity(),
                                         (*it)->getText(),
                                         (*it)->getFirstLocation(),
                                         (*it)->getSecondLocation(),
                                         (*it)->getEventCode(),
                                         (*it)->getExtent(),
                                         (*it)->getDirection(),
                                         (*it)->getQueueLength(),
                                         coords ) );
      // we got what we want now go on..
      trafficSits.push_back( trafficSit.release() );
   }
}

bool TrafficHandler::fetchSituationsFromModule() {
   Disturbances disturbances;
   if ( ! m_communicator->getAllDisturbances( m_provider, disturbances ) ) {
      return false;
   }

   // create temporary traffic situations 
   SitCont trafficSits;
   createTrafficSituations( disturbances, trafficSits );
   STLUtility::deleteValues( disturbances );

   // create one disturbance per map from the traffic situations
   Disturbances storedElements;
   composeNewElements( trafficSits, storedElements );
   updateStoredElements( storedElements, Disturbances() );

   STLUtility::deleteValues( trafficSits );

   return true;
}

bool isValid( const TrafficSituation& sit ) {
   if ( sit.getNbrElements() == 0 ) {
      // No elements to check the time on so it is valid
      return true;
   }
   uint32 timeNow = TimeUtility::getRealTime();
   vector< TrafficSituationElement* > tseVect = sit.getSituationElements();
   return timeNow < tseVect.front()->getExpiryTime();
}

void TrafficHandler::
createUniqueSet( const SitCont& parsedSitCont,
                 Disturbances& removedElements,
                 SitCont& newSituations ) {

   removedElements = m_storedElements;
   SitCont::const_iterator pIt = parsedSitCont.begin();
   for ( ; pIt != parsedSitCont.end(); ++pIt ) {

      pair< Disturbances::iterator, Disturbances::iterator > result;
      result = equal_range( removedElements.begin(),
                            removedElements.end(),
                            *pIt,
                            LessSituationRef() );
      
      if ( result.first != removedElements.end() &&
           (*result.first)->getSituationReference() ==
           (*pIt)->getSituationReference() ) {
         removedElements.erase( result.first, result.second );
      } else {
         // new situation not in m_storedElements, add it.
         // check if the time has expired. If it has, skip it.
         if ( isValid( **pIt ) ) {
            newSituations.push_back( *pIt );
         }
      }
   }
}


bool TrafficHandler::processSituations( const SitCont& trafficSits ) {

   Disturbances removedElements;
   SitCont newSituations;
   mc2dbg4 << "[TrafficHandler] trafficSits: " << trafficSits.size() << endl;
   mc2dbg4 << "[TrafficHandler] m_storedElements: " << m_storedElements.size() << endl;

   createUniqueSet( trafficSits, removedElements, newSituations );
   
   mc2dbg4 << "[TrafficHandler] Number of new situations: " << newSituations.size() << endl;
   mc2dbg4 << "[TrafficHandler] Number of removed elements: " << removedElements.size() << endl;

   // Create new DisturbanceElement's
   Disturbances newElements;
   composeNewElements( newSituations, newElements );

   if ( sendChangeset( newElements, removedElements ) ) {
      updateStoredElements( newElements, removedElements );
   } else {
      // the changeset failed, we need to resync with the database again
      mc2log << "[TrafficHandler] Changeset failed, resyncing with database..."
             << endl;
      setup();
      // even if only a part of the changeset was successfull, the only time we can
      // consider this function successful is when all the traffic situations
      // were processed succesfully.
      return false;
   }

   return true;
}

struct LessFirst {
   bool operator() ( const TrafficIPC::IDPairsToCoords::value_type& lhs,
                     const TrafficIPC::IDPairsToCoords::value_type& rhs ) {
      return lhs.first.first < rhs.first.first;
   }
};

void TrafficHandler::composeNewElements( const SitCont& newSituations,
                                         Disturbances& newDisturbances ) {
   SitCont::const_iterator cSitIt = newSituations.begin();
   for ( cSitIt = newSituations.begin(); 
         cSitIt != newSituations.end(); ++cSitIt ) {

      const TrafficSituation& traffSit( **cSitIt );
      vector< TrafficSituationElement* > tse = traffSit.getSituationElements();

      TrafficSituationElement* trafficElem( tse.front() );
      // This is needed since some parsers have a severity factor
      TrafficDataTypes::severity severity = trafficElem->getSeverity();
      uint32 costFactor = TrafficDataTypes::
         getCostFactorFromSeverity( severity );

      if ( costFactor != MAX_UINT32 ) {
         TrafficDataTypes::severity_factor severityFactor = 
            trafficElem->getSeverityFactor();
         costFactor = 
            (uint32)( (float)costFactor * 
                      TrafficDataTypes::
                      getCostFactorFromSeverityFactor( severityFactor ) );
      }
      
      TrafficIPC::IDPairsToCoords idPairsToCoords;
      m_communicator->getMapIDsNodeIDsCoords( traffSit, idPairsToCoords );

      typedef TrafficIPC::IDPairsToCoords::const_iterator IDsToCoordsIt;
      for ( IDsToCoordsIt it = idPairsToCoords.begin(); 
            it != idPairsToCoords.end(); ) {
         // one element per map..
         auto_ptr< DisturbanceElement > disturbance( 
               new DisturbanceElement( MAX_UINT32, // disturbanceID
                                       traffSit.getSituationReference(),
                                       trafficElem->getType(),
                                       trafficElem->getPhrase(),
                                       trafficElem->getEventCode(),
                                       trafficElem->getStartTime(),
                                       trafficElem->getExpiryTime(),
                                       trafficElem->getCreationTime(),
                                       severity,
                                       trafficElem->getDirection(),
                                       trafficElem->getFirstLocationCode(),
                                       trafficElem->getSecondLocationCode(),
                                       trafficElem->getExtent(),
                                       costFactor,
                                       trafficElem->getText(),
                                       trafficElem->getQueueLength() ) );

         disturbance->setMapID( it->first.first );

         // add all coordinates for this map
         for ( uint32 mapID = it->first.first; 
               it->first.first == mapID && it != idPairsToCoords.end(); ++it ) {
            disturbance->addCoordinate( it->first.second, // nodeID
                                        it->second.first.lat, // lat
                                        it->second.first.lon, // lon
                                        0, // angle
                                        it->second.second ); // routeIndex
         }

         newDisturbances.push_back( disturbance.release() );
      }
   }
}

void 
TrafficHandler::updateStoredElements( const Disturbances& newElements,
                                      const Disturbances& removedElements ) {
   // remove elements
   for ( Disturbances::const_iterator it = removedElements.begin(); 
         it != removedElements.end(); ++it ) {
      Disturbances::iterator storedIt = lower_bound( m_storedElements.begin(),
                                                     m_storedElements.end(),
                                                     *it,
                                                     LessSituationRef() );
      if ( storedIt != m_storedElements.end() &&
           (*storedIt)->getSituationReference() ==
           (*it)->getSituationReference() ) {
         delete *storedIt;
         m_storedElements.erase( storedIt );
      }
   }

   // add new elements
   for_each( newElements.begin(), newElements.end(),
             StoreStrippedElements( m_storedElements ) );

   // sort the elements
   sort( m_storedElements.begin(), m_storedElements.end(), LessSituationRef() );

   mc2dbg4 << "[TrafficHandler] stored elements "
           << m_storedElements.size() << endl;
   mc2dbg4 << "[TrafficHandler] new elements " << newElements.size() << endl;
   mc2dbg4 << "[TrafficHandler] removed elements "
           << removedElements.size() << endl;
}


typedef map< uint32, TrafficHandler::Disturbances > MapIDToDisturbances;

/**
 * Map disturbances to map ids.
 * @param idmap Maps MapID to Disturbances.
 * @param ids All Map IDs that were mapped.
 * @param disturbances All the disturbances to be mapped.
 */
void mapMapIDToDisturbances( MapIDToDisturbances& idmap,
                             set< MapIDToDisturbances::key_type >& ids,
                             TrafficHandler::Disturbances& disturbances ) {
   for ( TrafficHandler::Disturbances::iterator
            it = disturbances.begin(), itEnd = disturbances.end();
         it != itEnd; ++it ) {
      idmap[ (*it)->getMapID() ].push_back( *it );
      ids.insert( (*it)->getMapID() );
   }
}

bool TrafficHandler::sendChangeset( Disturbances& newElements,
                                    Disturbances& removedElements ) {

   // Setup map id to disturbance maps
   MapIDToDisturbances mapNewElements;
   MapIDToDisturbances mapRemovedElements;
   set< MapIDToDisturbances::key_type > mapIDs;
   mapMapIDToDisturbances( mapNewElements, mapIDs, newElements );
   mapMapIDToDisturbances( mapRemovedElements, mapIDs, removedElements );

   bool status = true;

   // Send one changeset per map id.
   for ( set< MapIDToDisturbances::key_type >::const_iterator
            it = mapIDs.begin(), itEnd = mapIDs.end();
         it != itEnd; ++it ) {
      bool currentStatus = m_communicator->sendChangeset( mapNewElements[ *it ],
                                                          mapRemovedElements[ *it ] );
      status &= currentStatus;
      if ( ! currentStatus ) {
         mc2log << warn << "[TrafficHandler] MapID " << hex << *it << dec
                << " failed to send changeset." << endl;
      }
   }

   return status;
}


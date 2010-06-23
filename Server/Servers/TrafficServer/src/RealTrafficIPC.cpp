/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RealTrafficIPC.h"

#include "MapRequester.h"
#include "RouteRequest.h"
#include "RoutePacket.h"
#include "ExpandedRoute.h"
#include "StreetSegmentItemPacket.h"
#include "GetTMCCoordinatePacket.h"
#include "FetchAllDisturbancesPacket.h"
#include "IDTranslationPacket.h"
#include "TopRegionRequest.h"
#include "ItemIDTree.h"
#include "DisturbanceChangeset.h"
#include "DisturbanceChangesetPacket.h"
#include "TrafficSituation.h"
#include "TrafficSituationElement.h"
#include "TrafficDataTypes.h"
#include "DisturbanceElement.h"
#include "DeleteHelpers.h"

#include "MapBits.h"

RealTrafficIPC::RealTrafficIPC( MapRequester& moduleCom ):
   m_moduleCom( moduleCom )
{}

RealTrafficIPC::~RealTrafficIPC() {

}

namespace {
template <typename PacketType>
bool getValidPacket( PacketType*& answer,
                     PacketContainer* cont ) {
   if ( cont ) {
      answer = static_cast<PacketType*>( cont->getPacket() );
      if( answer && answer->getStatus() == StringTable::OK ) {
         return true;
      }
   }

   return false;
}
}

struct Clone {
   DisturbanceElement* operator()( const DisturbanceElement* other ) {
      return new DisturbanceElement( *other );
   }
};
inline void clone( const TrafficIPC::Disturbances& cloneThese,
                   TrafficIPC::Disturbances& toMe ) {
   std::transform( cloneThese.begin(), cloneThese.end(),
                   std::back_inserter( toMe ), Clone() );
}

bool
RealTrafficIPC::sendChangeset( const Disturbances& newElements,
                               const Disturbances& removedElements ) {
   Disturbances newClonedElements;
   Disturbances removedClonedElements;
   clone( newElements, newClonedElements );
   clone( removedElements, removedClonedElements );

   DisturbanceChangeset changeset( newClonedElements, removedClonedElements );
   // create the packet
   DisturbanceChangesetRequestPacket* packet = 
      new DisturbanceChangesetRequestPacket( Packet::PacketID( 0 ),
                                             m_moduleCom.getNextRequestID(),
                                             changeset );
   uint32 mapID = MAX_UINT32;
   if ( ! newElements.empty() ) {
      mapID = newElements.front()->getMapID();
   } else if ( ! removedElements.empty() ) {
      mapID = removedElements.front()->getMapID();
   }

   packet->setMapID( mapID );
   // send the packet and get the packet container as a return.
   auto_ptr<PacketContainer> cont( m_moduleCom.
                                   putRequest( packet, MODULE_TYPE_TRAFFIC ) );

   DisturbanceChangesetReplyPacket* answer;
   if ( getValidPacket( answer, cont.get() ) ) {
      if( answer && answer->getStatus() == StringTable::OK ) {
         mc2dbg4 << "[RTIPC] send change set OK" << endl;
         return true;
      } else {

         if ( answer->getUpdateStatus() == StringTable::NOTOK ) {
            mc2log << warn
                   << "[RTIPC] Failed to update new DisturbanceElements."
                   << endl;
         } 

         if ( answer->getRemoveStatus() == StringTable::NOTOK ) {
            mc2log << warn
                   << "[RTIPC] Failed to remove old DisturbanceElements."
                   << endl;
         }
      }
   }
   return false;
}

bool
RealTrafficIPC::sendPacketContainers( PacketContainers& pcs ) {
   m_moduleCom.putRequest( pcs );
   return true;
}

bool RealTrafficIPC::getAllDisturbances( const MC2String& provider,
                                         Disturbances& disturbances ) {
   FetchAllDisturbancesRequestPacket* packet =
      new FetchAllDisturbancesRequestPacket( Packet::PacketID( 0 ),
                                             m_moduleCom.getNextRequestID(),
                                             provider );

   // create the packet container
   auto_ptr<PacketContainer> cont( m_moduleCom.
                                   putRequest( packet, MODULE_TYPE_TRAFFIC ) );

   FetchAllDisturbancesReplyPacket* answer;
   if ( getValidPacket( answer, cont.get() ) ) {
      answer->getDisturbances( disturbances );
      return true;
   }

   return false;
}

bool 
RealTrafficIPC::getTMCCoordinates( const TrafficSituationElement& tse,
                                   MC2Coordinates& firstCoords,
                                   MC2Coordinates& secondCoords ) {
   // Create the packet
   GetTMCCoordinateRequestPacket* packet = new GetTMCCoordinateRequestPacket();
   packet->addPoints( tse.getFirstLocationCode(),
                      tse.getSecondLocationCode(),
                      tse.getExtent(),
                      tse.getDirection() );

   // create the packet container
   auto_ptr<PacketContainer> cont( m_moduleCom.
                                   putRequest( packet, MODULE_TYPE_TRAFFIC ) );

   GetTMCCoordinateReplyPacket* answer;
   if ( getValidPacket( answer, cont.get() ) ) {
      answer->getMC2Coordinates( firstCoords, secondCoords );
      return true;
   }

   return false;
}

bool 
RealTrafficIPC::getCoordinates( const TrafficSituation& traffSit, 
                                MC2Coordinates& firstCoords, 
                                MC2Coordinates& secondCoords ) {
   vector< TrafficSituationElement* > tse = traffSit.getSituationElements();
   if ( tse.size() == 0 ) {
      // invlaid TrafficSituation no TMC location or coordinates provided..
      // This should not happen since its required to have at least one
      // TrafficSituationElement..
      return false;
   } else if ( !tse.front()->getFirstLocationCode().empty() ) {
      // Get the coordinates for the TMC location/locations
      return getTMCCoordinates( *tse.front(), firstCoords, secondCoords );
   } else {
      // No TMC location, get coordinates instead.
      MC2Coordinates coordinates = tse.front()->getMC2Coordinates();
      if ( coordinates.size() == 0 ) {
         // No coordinates either, once again this should not happen, but..
         return false;
      } else {
         // at least on coordinate, this will become our start point
         firstCoords.push_back( coordinates.front() );
         if ( coordinates.size() > 1 ) {
            // great! we got an end point too!
            secondCoords.push_back( coordinates.back() );
         }
         return true;
      }
   }
}

typedef std::multimap< uint32, PacketContainer* > StreetReplies;
void 
processStreetReplies( const StreetReplies& streetReplies,
                      TrafficIPC::IDPairsToCoords& idPairsToCoords ) {
   uint32 mapID = 0;
   int32 lat = 0;
   int32 lon = 0;
   uint32 firstNodeID = 0;
   uint32 secondNodeID = 0;
   TrafficIPC::RouteIndex routeIndex = 0;

   typedef StreetReplies::const_iterator StreetRepIt;
   // for each reply packet check the packet index, for those that have the
   // same packet index. Which means that the reply packet didnt have a unique
   // map id to it so packets where made to ask for the nodeID on all the maps
   // it was on. It will then go on and pick out the packet with the smallest 
   // distance from the point we askes for. And store  and use that data.
   for( StreetRepIt it = streetReplies.begin(); 
        it != streetReplies.end(); ++it ) {
      if ( streetReplies.count( it->first ) > 0 ) {
         // get the range of the index, typically just one element
         std::pair< StreetRepIt, StreetRepIt > range = 
            streetReplies.equal_range( it->first );
         uint32 minDistance = MAX_UINT32;
         for ( StreetRepIt sIt = range.first; sIt != range.second; ++sIt ) {
            PacketContainer* packet = sIt->second;
            StreetSegmentItemReplyPacket* currPacket =
               static_cast< StreetSegmentItemReplyPacket* >
               ( packet->getPacket() );

            uint32 distance = currPacket->getDistance();
            if ( distance < minDistance) {
               mapID = currPacket->getMapID();
               lat = currPacket->getLatitude();
               lon = currPacket->getLongitude();
               firstNodeID = currPacket->getFirstNodeID();
               secondNodeID = currPacket->getSecondNodeID();
               minDistance = distance;
            }
            it = sIt;
         }
      }

      if ( firstNodeID != MAX_UINT32 ) {
         MC2Coordinate coord( lat, lon );
         idPairsToCoords.insert( make_pair( IDPair_t( mapID, firstNodeID ), 
                                            make_pair( coord, routeIndex ) ) );
         ++routeIndex;
      }

      if ( secondNodeID != MAX_UINT32 ) {
         MC2Coordinate coord( lat, lon );
         idPairsToCoords.insert( make_pair( IDPair_t( mapID, secondNodeID ), 
                                            make_pair( coord, routeIndex ) ) );
         ++routeIndex;
      } 
   }
}

struct LessFirst {
   bool operator() ( const TrafficIPC::IDPairsToCoords::value_type& lhs,
                     const TrafficIPC::IDPairsToCoords::value_type& rhs ) {
      return lhs.first.first < rhs.first.first;
   }
};

void RealTrafficIPC::
createIDTranslationPackets( const IDPairsToCoords& idPairsToCoords,
                            PacketContainers& idTransReqPackets,
                            IDPairVectors& lowLevelIDPairs ) {
   
   const ItemIDTree& mapIDTree = 
      m_moduleCom.getTopRegionRequest()->getWholeItemIDTree();

   typedef IDPairsToCoords::const_iterator IDsToCoordsIt;

   for ( IDsToCoordsIt it = idPairsToCoords.begin(); 
         it != idPairsToCoords.end(); ++it ) {
      // foreach unique mapID create a IDTranslationRequestPacket with all the 
      // nodes for the map above.
      uint32 overviewMapID = mapIDTree.getHigherLevelMap( it->first.first );
      if ( overviewMapID == MAX_UINT32 || 
           overviewMapID >= FIRST_SUPEROVERVIEWMAP_ID ) {
         continue;
      }
      // find the range for the map we are working on now
      std::pair< IDsToCoordsIt, IDsToCoordsIt > range;
      range = equal_range( idPairsToCoords.begin(), 
                           idPairsToCoords.end(), 
                           *it, 
                           LessFirst() );

      IDPairVector_t idPairs;
      // for all the IDPair_t for the map store them
      for ( IDsToCoordsIt rIt = range.first; rIt != range.second; ++rIt ) {
         idPairs.push_back( rIt->first );
         it = rIt;
      }

      //create the packet that gets the node ids on the map one level up
      auto_ptr< IDTranslationRequestPacket > 
         idPacket( new IDTranslationRequestPacket( Packet::PacketID( 0 ),
                                                   m_moduleCom.getNextRequestID(),
                                                   overviewMapID,
                                                   false,
                                                   idPairs ) );
      // create the packet container
      idTransReqPackets.
         push_back( new PacketContainer( idPacket.release(), 
                                         0, 
                                         0, 
                                         MODULE_TYPE_MAP ) );
      lowLevelIDPairs.push_back( idPairs );

      mc2dbg4 << "[RTIPC] lowLevelIDPairs size: " << lowLevelIDPairs.size() << endl;
     
   }
}

void RealTrafficIPC::
getIDTranslationData( IDPairsToCoords& idPairsToCoords,
                      const PacketContainers& idTransReqPackets,
                      const IDPairVectors& lowLevelIDPairs ) {

   typedef PacketContainers::const_iterator PCsIT;
   typedef IDPairVectors::const_iterator IDPairsIT;
   IDPairsToCoords tmpIDPairsToCoords;

   IDPairsIT idIt = lowLevelIDPairs.begin();
   for ( PCsIT it = idTransReqPackets.begin();
         it != idTransReqPackets.end(); ++it, ++idIt ) {
      
      IDTranslationReplyPacket* replyPacket =
         static_cast<IDTranslationReplyPacket*>( (*it)->getPacket() );
      if ( replyPacket == NULL ) {
         continue;
      }

      //fetch the translated nodes into highVector
      IDPairVector_t highVector;
      replyPacket->getTranslatedNodes( 0, highVector );
      const IDPairVector_t lowVector = *idIt;
        
      MC2_ASSERT( lowVector.size() == highVector.size() );

      // go through all the high level pairs we got in return
      for ( uint32 index = 0; index < lowVector.size(); ++index ) {
         if ( highVector[ index ].isValid() ) {
            // we got a valid id pair in return so use the corresponding low
            // level coordinates for the id pair. Since its one level up in
            // the maps.
            CoordPair coordPair = idPairsToCoords[ lowVector[ index ] ];
            tmpIDPairsToCoords.insert( make_pair( highVector[ index ],
                                                  coordPair ) );
         }
      }
       
   }

   // this will stop the recursion if there is no more new id pairs
   if ( !tmpIDPairsToCoords.empty() ) {
      // get the higher level nodes for those if there is some.
      getHigerLevelNodes( tmpIDPairsToCoords );
      // insert the new id pairs to coords
      idPairsToCoords.insert( tmpIDPairsToCoords.begin(), 
                              tmpIDPairsToCoords.end() );
   }
}

void RealTrafficIPC::
getHigerLevelNodes( IDPairsToCoords& idPairsToCoords ) {
   PacketContainers idTransReqPackets;
   IDPairVectors lowLevelIDPairs;

   createIDTranslationPackets( idPairsToCoords, 
                               idTransReqPackets, 
                               lowLevelIDPairs );

   sendPacketContainers( idTransReqPackets );

   getIDTranslationData( idPairsToCoords, idTransReqPackets, lowLevelIDPairs );

   STLUtility::deleteValues( idTransReqPackets );

}

void 
RealTrafficIPC::getMapIDsNodeIDsCoords( const TrafficSituation& traffSit,
                                        IDPairsToCoords& idPairsToCoords ) {

   const vector< TrafficSituationElement* >& traffSitElement = 
      traffSit.getSituationElements();

   MC2Coordinates firstCoords;
   MC2Coordinates secondCoords;
   getCoordinates( traffSit, firstCoords, secondCoords );

   // Get the direction of the situation
   TrafficDataTypes::direction direction = 
      traffSitElement.front()->getDirection();

   // if we got tmc location do this
   if ( !firstCoords.empty() && !secondCoords.empty() ) {
      RouteRequests rrs;
      // create route
      if ( direction == TrafficDataTypes::Positive ) {
         createRouteRequest( secondCoords, firstCoords, rrs );
      } else if ( direction == TrafficDataTypes::Negative ) {
         createRouteRequest( firstCoords, secondCoords, rrs );
      } else {
         // Create a route in both directions
         createRouteRequest( secondCoords, firstCoords, rrs );
         createRouteRequest( firstCoords,  secondCoords, rrs );
      }

      mc2dbg4 << "[RTIPC] Created a route now handling it" << endl;

      // get the data we want from the route requests
      handleRouteRequests( rrs, idPairsToCoords );
      for ( RouteRequests::iterator it = rrs.begin(); it != rrs.end(); ++it ) {
         delete (*it)->getAnswer();
         delete *it;
      }

   } else if ( !firstCoords.empty() && secondCoords.empty() ) {


      mc2dbg4 << "[RTIPC] We got a single point.. get the nodeIDs etc" << endl;
      mc2dbg4 << "[RTIPC] Number of coords: " << firstCoords.size() << endl;

      PacketContainers ssiReqPC;
      // we got a single tmc point, create SSI req
      createSSIRequestPackets( firstCoords, ssiReqPC );

      sendPacketContainers( ssiReqPC );

      StreetReplies streetReplies;
      handleSSIReplies( ssiReqPC, firstCoords, streetReplies );

      // we should now have all street segments so lets extract the data
      processStreetReplies( streetReplies, idPairsToCoords );
      STLUtility::deleteAllSecond( streetReplies );
   }
   mc2dbg4 << "[RTIPC] Getting higher level nodes" << endl;
   mc2dbg4 << "[RTIPC] idPairsToCoords size:" << idPairsToCoords.size() << endl;
   
   // get the higher level nodes..
   getHigerLevelNodes( idPairsToCoords );

}

void RealTrafficIPC::
handleSSIReplies( const PacketContainers& ssiPC,
                  const MC2Coordinates& firstCoord,
                  StreetReplies& streetReplies ) {
   PacketContainers::const_iterator replyIt;
   for ( replyIt = ssiPC.begin(); replyIt != ssiPC.end(); ++replyIt ) {
      StreetSegmentItemReplyPacket* replyPacket =
         static_cast<StreetSegmentItemReplyPacket*>( (*replyIt)->getPacket() );
      if ( replyPacket == NULL ) {
         delete *replyIt;
         // skip this packet or resend?
      } else if ( replyPacket->getStatus() == StringTable::OK ) {
         // packet ok so keep this and handle it.
         streetReplies.insert( make_pair( replyPacket->getIndex(), *replyIt ) );
      } else if ( replyPacket->getStatus() == StringTable::NOT_UNIQUE ) {
         // The mapID is not unique so we need to create new packets for each
         // map. This shouldnt happen to often but still needs to be covered.
         Index index = replyPacket->getIndex();
         uint32 nbrMaps = replyPacket->getNbrMapIDs();
         PacketContainers newPackets;
         for ( uint32 i = 0; i < nbrMaps; ++i ) {
            uint32 currentMapID = replyPacket->getMapID( i );
            // create a packet for each map
            auto_ptr< StreetSegmentItemRequestPacket > newPacket(
                  new StreetSegmentItemRequestPacket( 
                     0, // packetID
                     m_moduleCom.getNextRequestID(),
                     index,
                     firstCoord[ index ].lat,  // latitude 
                     firstCoord[ index ].lon, // longitude
                     TrafficDataTypes::NoDirection ) );

            newPacket->setMapID( currentMapID );
            // create the packet container
            newPackets.push_back( 
                  new PacketContainer( newPacket.release(), 
                                       0, 
                                       0, 
                                       MODULE_TYPE_MAP ) );
         }
         // send them and call this function again
         sendPacketContainers( newPackets );
         handleSSIReplies( newPackets, firstCoord, streetReplies );
         delete *replyIt;
      }
   }
}

void
RealTrafficIPC::createRouteRequest( const MC2Coordinates& startCoords,
                                    const MC2Coordinates& endCoords,
                                    RouteRequests& rrs ) {
   const uint32 expandType = ( ROUTE_TYPE_STRING |
                               ROUTE_TYPE_GFX |
                               ROUTE_TYPE_GFX_TRISS |
                               ROUTE_TYPE_ITEM_STRING );

   auto_ptr< RouteRequest > rr( new RouteRequest( NULL,
                                m_moduleCom.getNextRequestID(),
                                expandType,
                                StringTable::SWEDISH,
                                false,
                                0,
                                m_moduleCom.getTopRegionRequest() ) );
   // Set the route parameters.
   rr->setRouteParameters( false,
                           RouteTypes::TIME,
                           ItemTypes::passengerCar,
                           0,
                           0 );

   // Add origin coordinates
   MC2Coordinates::const_iterator it = startCoords.begin();
   for ( ; it != startCoords.end(); ++it ) {
      rr->addOriginCoord( it->lat, it->lon );
   }

   // Add destination coordinates
   for ( it = endCoords.begin(); it != endCoords.end(); ++it ) {
      rr->addDestinationCoord( it->lat, it->lon );
   }

   rrs.push_back( rr.release() );
}

void RealTrafficIPC::
handleRouteRequests( RouteRequests& rrs,
                     IDPairsToCoords& idPairsToCoords ) {
   
   RouteIndex routeIndex = 0;
   for ( RouteRequests::iterator it = rrs.begin(); it != rrs.end(); ++it ) {
      RouteRequest* rr = *it;
      // send the data and handle the reply
      m_moduleCom.putRequest( rr );
      
      if ( rr->getStatus() != StringTable::OK ) {
         continue;
      }

      const ExpandedRoute* er = rr->getExpandedRoute();
      const uint32 nbrOfItems = er->getNbrExpandedRouteItems();
      for ( uint32 i = 0; i < nbrOfItems; ++i ) {
         const ExpandedRouteItem* eri = er->getExpandedRouteItem( i );
         const uint32 nbrOfRoadItems = eri->getNbrExpandedRouteRoadItems();
         for ( uint32 j = 0; j < nbrOfRoadItems; ++j ) {
            const ExpandedRouteRoadItem* erri = 
               eri->getExpandedRouteRoadItem( j );
            MC2Coordinate coord = erri->getCoordinate( 0 );
            uint32 mapID = erri->getMapID();
            uint32 nodeID = erri->getNodeID();
            pair< IDPairsToCoords::iterator, bool > result =
               idPairsToCoords.
               insert( make_pair( IDPair_t( mapID, nodeID ), 
                                  make_pair( coord, routeIndex ) ) );

            // inserted a new id pair so increase the route index
            if ( result.second ) {
               ++routeIndex;
            }
         }
      }
   }
}


void RealTrafficIPC::
createSSIRequestPackets( const MC2Coordinates& coord, PacketContainers& pcs ) {
   MC2Coordinates::const_iterator cIt = coord.begin();
   for ( Index index = 0; cIt != coord.end(); ++cIt, ++index ) {
      // create the packet
      auto_ptr< StreetSegmentItemRequestPacket > packet(
         new StreetSegmentItemRequestPacket( 0, // packetID
                                             m_moduleCom.getNextRequestID(),
                                             index,
                                             cIt->lat,  // latitude 
                                             cIt->lon, // longitude
                                             TrafficDataTypes::NoDirection ) );
      // create the packet container
      pcs.push_back( new PacketContainer( packet.release(), 
                                          0, 
                                          0, 
                                          MODULE_TYPE_MAP ) );
   }
}

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
#include "MapReader.h"

#include <time.h>
#include <vector>
#include <fstream>
#include "Queue.h"
#include "Fifo.h"
#include "MC2BoundingBox.h"
#include "MapModuleNotice.h"
#include "Properties.h"
#include "Stack.h"

#include "DebugClock.h"

#include "ExternalConnections.h"
#include "MC2MapGenUtil.h"

// Packets
#include "ServerProximityPackets.h"
#include "AllCountryPacket.h"
#include "LoadMapPacket.h"
#include "CoordinatePacket.h"
#include "StreetSegmentItemPacket.h"
#include "AllMapPacket.h"
#include "GfxFeatureMapPacket.h"
#include "CopyrightBoxPacket.h"
#include "CopyrightHandler.h"
#include "BBoxPacket.h"

#include "DatagramSocket.h"
#include <iomanip>

#include "DataBuffer.h"
#include "MapPacket.h"

// Items
#include "StreetSegmentItem.h"
#include "StreetItem.h"
#include "PointOfInterestItem.h"
#include "CategoryItem.h"

#include "TopRegionPacket.h"

#include "DataBufferCreator.h"
#include "File.h"

// Smallest routingcost stuff.
#include "SmallestRoutingCostPacket.h"
#include "MMRoutingCostTable.h"

// RoutingInfo packet - information about the levels.
#include "RoutingInfoPacket.h"

// For generator version
#include "MapGenerator.h"

#include "MapBits.h"
#include "Math.h"

MapReader::MapReader(Queue *q,
                     NetPacketQueue& sendQueue,
                     MapSafeVector* loadedMaps,
                     PacketReaderThread* packetReader,
                     uint16 port,
                     uint32 definedRank,
                     Vector* startWithMaps) 
      :  StandardReader(  MODULE_TYPE_MAP, 
                          q, sendQueue,
                          loadedMaps, 
                          *packetReader, port,                  
                          definedRank)
{


   this->m_startWithMaps = startWithMaps;
   // Create the table 
   m_routingCostTable = new MMRoutingCostTable();
}


MapReader::~MapReader()
{
   delete m_routingCostTable;
}

inline MapReplyPacket*
MapReader::handleMapRequestPacket(Packet* p)
{
   MapRequestPacket* mrp = (MapRequestPacket*)p;

   // Get the requested map id.
   uint32 mapID = mrp->getMapID();

   /// FIXME: Send ack if wrong map.
   
   const MapModuleNotice* notice = m_indexDB.getMap(mapID);
   if(notice == NULL){
      MapReplyPacket* noMap = new MapReplyPacket(mrp, 0, 0, 0,0);
      noMap->setStatus(51);
      return noMap;
   }
   
   
   uint32 myMapVersion = notice->getCreationTime();
   // Here we should really get the version of the generator for
   // the map, but we can bump the version number for allt generators
   // when changing one.
   uint32 myGeneratorVersion = MapGenerator::generatorVersion;

   mc2dbg << "[MapReader]: MapReqPack: My version "
          << hex << myMapVersion
          << " and packet " << mrp->getMapVersion() << dec
          << " type = " << uint32( mrp->getMapType() ) << endl;
   
   // If the versions are the same - return packet here.
   if ( ! mrp->newMapNeeded(myMapVersion, myGeneratorVersion) ) {
      mc2dbg << "[MapReader]: Packet has same version - replying" << endl;
      return new MapReplyPacket(mrp, 0, 0, myMapVersion, myGeneratorVersion);
   }
   
   // If it is not a route request, dont do anything more
   if ( mrp->getMapType() != MapRequestPacket::MAPREQUEST_ROUTE )
      return NULL;
      
   vector<uint32> overviews;
   // Get the maps from the container
   if ( m_indexDB.getMapsAbove( mapID, overviews ) ) {
      // ok
   } else {
      // Not found.
      mc2log << error << "[MR]: handleMapRequestPacket mapid "
             << mapID << " was not found." << endl;
   }
   
   // We should really have found everything by now
   mrp->setOverviewMaps(overviews);
   return NULL;
}

inline TopRegionReplyPacket*
MapReader::handleTopRegionRequestPacket( Packet* p ) const
{
   ConstTopRegionMatchesVector topRegions;

   m_indexDB.getTopRegions( topRegions );
   
   TopRegionReplyPacket* reply = new TopRegionReplyPacket( 
                  static_cast<TopRegionRequestPacket*> (p),
                  m_indexDB.getMapTree(),
                  topRegions);
   return reply;
}

inline SmallestRoutingCostReplyPacket*
MapReader::handleSmallestRoutingCostRequestPacket(
      const SmallestRoutingCostRequestPacket* req ) const
{
   // Get the map pairs from the request.
   MapPairVector vec;
   req->getMapPairVector(vec);

   // Get the routing costs for the map pairs from a 
   // SmallestRoutingCostTable
   // (currectly no table, just use cost = (0, 0)
   SmallestRoutingCostReplyPacket::costMap_t costMap;
   RoutingCosts zeroCosts(0,0);
   for (uint32 i = 0; i < vec.size(); i++) {
      const RoutingCosts* packetCost =
         m_routingCostTable->getCostsUsingTree(
            m_indexDB.getMapTree(),
            vec[i].first, vec[i].second);
      if ( packetCost == NULL ) {
         // Send zero if we do not have the costs.
         packetCost = &zeroCosts;
      }
      costMap.insert(make_pair(vec[i], *packetCost));
   }

   SmallestRoutingCostReplyPacket* reply =
      new SmallestRoutingCostReplyPacket(req, costMap);

   return reply;
}

inline uint32
MapReader::getRouteDistanceForLevel(uint32 level) const
{
   // Default up criteria in meters.
   static const uint32 defaultUpCrit [] = { 80*10000, 200*10000, MAX_UINT32,
                                         MAX_UINT32, MAX_UINT32, MAX_UINT32};
   
   char propString[256];
   sprintf(propString, "ROUTE_DISTANCE_LEVEL_%d", level);
   uint32 distance =
      Properties::getUint32Property(propString, defaultUpCrit[level]);
   mc2dbg << "[MapReader]: Distance for level " << level
          << " is " << distance << endl;
   return distance;
      
}

inline RoutingInfoReplyPacket*
MapReader::handleRoutingInfoRequest(const RoutingInfoRequestPacket* req) const
{
   
   int maxMapLevel = 0;
   for (uint32 i=0; i<m_indexDB.getSize(); i++) {
      const MapModuleNotice* mn = m_indexDB[i];
      int mapLevel = MapBits::getMapLevel(mn->getMapID());
      maxMapLevel = MAX(mapLevel, maxMapLevel);
   }
   
   CriteriumMap upCriteria;
   CriteriumMap downCriteria;

//     // Only route on low level!!
//     upCriteria.insert(make_pair(0, MAX_UINT32));
//     return new RoutingInfoReplyPacket(req, RoutingInfo(upCriteria,
//                                                        downCriteria));
   
   // We will only fill in the upCriteria, since the other ones are
   // not used yet
   for( int i = 0; i < maxMapLevel; ++i ) {
      upCriteria.insert(make_pair(i, getRouteDistanceForLevel(i)));
   }
   
   // Last one must be MAX_UINT32. Cannot go higher there.
   upCriteria[maxMapLevel] = MAX_UINT32;

   // Our work is done here.
   return new RoutingInfoReplyPacket(req, RoutingInfo(upCriteria,
                                                      downCriteria));
   
}

AllMapReplyPacket*
MapReader::handleAllMapRequest( const AllMapRequestPacket* req )
{
   mc2dbg1 << "AllMapRequestPacket recv" << endl;
   
   AllMapReplyPacket *replyPack = new AllMapReplyPacket( req );
   
   AllMapRequestPacket::allmap_t type =  req->getType();
   
   uint32 nbrMaps = m_indexDB.getSize();
   
   bool useStartWithMaps = ( (m_startWithMaps != NULL) && 
                             (m_startWithMaps->getSize() > 0));
   
   for (uint32 i=0; i < nbrMaps; i++) {
      const MapModuleNotice* mn = m_indexDB[i];
      if ((mn != NULL) && 
          ( ( (useStartWithMaps) &&
              (m_startWithMaps->linearSearch(mn->getMapID()) < 
               MAX_UINT32)
              ) || (!useStartWithMaps)
            )) {
         if (type == AllMapRequestPacket::BOUNDINGBOX) {
            replyPack->addLast(mn->getMapID(), mn->getBBox() );            
         } else if (type == AllMapRequestPacket::BOUNDINGBOX_VER) {
            uint32 creationTime = mn->getCreationTime();
            mc2dbg2 << "Sending creation time: " << creationTime 
                    << endl;
            replyPack->addLast( mn->getMapID(), creationTime,
                                mn->getBBox() );
         } else {
            replyPack->addLast( mn->getMapID() );
            mc2dbg2 << "added mapID " << mn->getMapID() << endl;
         }
      } // else this should not be included!
   }
   
   replyPack->setStatusCode(StringTable::OK);
   mc2dbg1 << "   Sending AllMapReply to " << req->getOriginAddr()
           << " (ReqID = " << replyPack->getRequestID() 
           << ", packID=" << replyPack->getPacketID() << ")" 
           << endl;
   mc2dbg1 << "Added " << replyPack->getNbrMaps() << " maps"
           << endl;
   mc2dbg4 << "-------------------------" << endl;

   return replyPack;
}

bool
MapReader::leaderProcessCtrlPacket(Packet*p)
{
   MC2_ASSERT( isLeader() == true );
   uint32 startTime = TimeUtility::getCurrentTime();
   // Must take care of the "ordinary" leader packets
   if (! StandardReader::leaderProcessCtrlPacket(p)) {
      
      mc2dbg2 << "MapReader::leaderProcessCtrlPacket" << endl;
      switch( (int)p->getSubType() ) {
         case Packet::PACKETTYPE_ALLCOUNTRYREQUEST : {
            mc2dbg1 << "   AllCountryRequestPacket recv" << endl;
            
            AllCountryReplyPacket* replyPack = new AllCountryReplyPacket(
               static_cast<AllCountryRequestPacket*>(p));
            
            // Check all the notices and find the ones that are
            // country maps.
            for (uint32 i=0; i<m_indexDB.getSize(); i++) {
               const MapModuleNotice* mn = m_indexDB[i];
               if (MapBits::isCountryMap(mn->getMapID())) {
                  const GfxData* gfx = mn->getGfxData();
                  MC2BoundingBox bbox;
                  if (gfx != NULL) {
                     gfx->getMC2BoundingBox(bbox);
                  }                  
                  Vector countryNames(1);
                  countryNames.addLast(
                        StringTable::getCountryStringCode(
                           mn->getCountryCode()));
                  mc2dbg << "addCountry(" << mn->getMapID()
                         << ", " << mn->getCreationTime()
                         << ", bbox, " << countryNames.getElementAt(0) << endl;
                  bbox.dump();
                  replyPack->addCountry(mn->getMapID(), 
                                        mn->getCreationTime(), 
                                        bbox, 
                                        countryNames);
               }
            }
      
            sendAndDeletePacket(replyPack,
                                p->getOriginIP(),
                                p->getOriginPort());
            delete p;
            return (true);
         } break;
         
         
         case Packet::PACKETTYPE_ALLMAPREQUEST : {
            AllMapRequestPacket* req =
               static_cast<AllMapRequestPacket*>( p );
            
            sendAndDeletePacket( handleAllMapRequest( req ),
                                 p->getOriginAddr() );
            delete p;
            return true;
         }
         break;
         
         case Packet::PACKETTYPE_COORDINATEREQUEST : {
            CoordinateRequestPacket* inPacket = (CoordinateRequestPacket*) p;
            
            mc2dbg << "[MapReader]: CoordinateRequestPacket recv mapid = "
                   << inPacket->getMapID() << endl;
            /*
             *    Determin which map that the coordinates are located in
             *    and retransmit the request to the correct module.
             */

            // Check if the incomming request packet has a mapID. If that 
            // is the case resent the request to a module with that map!
            if (inPacket->getMapID() < MAX_UINT32) {
               mc2dbg2
                  << "Got a CoordinateRequestPacket with a MapID<MAX_UINT32" 
                  << endl;
               inPacket->setMapID(inPacket->getMapID());
               
               sendRequestPacket( inPacket );
               mc2dbg4 << "   CoordinateRequestPacket send to module "
                       << "with map" << inPacket->getMapID() << endl;
               // p should _not_ be deleted here since it's either deleted
               // or inserted into queue in sendRequestPacket
               
            } else {
               // Find what map to use
               
               int32 lat = inPacket->getLatitude();
               int32 lon = inPacket->getLongitude();
              
               // Find out if more than one map i located at (lat,lon)
               Vector mapIDs(8);
               findMapsAtCoordinate(lat, lon, &mapIDs);
               if (mapIDs.getSize() >= 1) {
                  // ANY MAP COVERED
                  // We will return the packet even if only one map
                  // is covered. This way we can both reuse the packet
                  // for other purposes and also add user rights for the
                  // correct map in the server.
                  CoordinateReplyPacket* replyPack = 
                     new CoordinateReplyPacket( inPacket,
                                                StringTable::NOT_UNIQUE,
                                                MAX_UINT32,
                                                MAX_UINT32,
                                                0, 0);
                  for (uint32 i=0; i<mapIDs.getSize(); i++) {
                     mc2dbg2 << "adding possible map with ID=" 
                             << mapIDs.getElementAt(i) << endl;
                     replyPack->addPossibleMap(mapIDs.getElementAt(i));
                  }
                  
                  replyPack->setDebInfo(TimeUtility::getCurrentTime()-startTime);
                  // Return the reply packet to the caller
                  sendAndDeletePacket( replyPack, 
                                       p->getOriginIP(), 
                                       p->getOriginPort());
                  delete p;
               } else if ( false && mapIDs.getSize() == 1) {
                  // We will not do this. User rights depend on the
                  // correct map id.
                  // EXACTLY ONE map at (lat,lon), return that
                  inPacket->setMapID(mapIDs.getElementAt(0));
                  
                  sendRequestPacket( inPacket );
                  mc2dbg4 << "   CoordinateRequestPacket resend to module" 
                          << endl;
                  // p should _not_ be deleted here since it's either deleted
                  // or inserted into queue in sendRequestPacket
               } else {
                  // NO map at (lat,lon), find the closest one
                  float64 mapDist = 0.0;
                  uint32 mapID = findMapFromCoordinate( lat, lon, 
                                                        &mapDist );

                  static const double maxDist = 1000.0;
                  if ( mapID == MAX_UINT32 || 
                       fabs( mapDist ) > SQUARE( maxDist ) ) 
                  {
                     mc2dbg << "[MapReader]: Too far to closest map : "
                            << ::sqrt(mapDist) << " > "
                            << maxDist << endl;
                     CoordinateReplyPacket* replyPack = 
                        new CoordinateReplyPacket( inPacket,
                                                   StringTable::MAPNOTFOUND,
                                                   MAX_UINT32,
                                                   MAX_UINT32,
                                                   0, 0);
                     replyPack->setDebInfo(TimeUtility::getCurrentTime() -
                                           startTime);
                     
                     sendAndDeletePacket( replyPack, 
                                          p->getOriginIP(), 
                                          p->getOriginPort());
                     delete p;
                  } else {
                     inPacket->setMapID(mapID);
                     
                     sendRequestPacket( inPacket );
                     mc2dbg4 << "   CoordinateRequestPacket resend "
                             << "to module" << endl;
                     // p should _not_ be deleted here since it's either 
                     // deleted or inserted into queue in sendRequestPacket
                  }
               }
            }
            return (true);
         }
         break;
         
         case Packet::PACKETTYPE_STREET_SEGMENT_ITEMREQUEST : {
            mc2dbg1 << "StreetSegmentItemRequestPacket recv" << endl;
            /*
             *    Determin which map that the coordinates are located in
             *    and retransmit the request to the correct module.
             */
            
            StreetSegmentItemRequestPacket* inPacket =
               (StreetSegmentItemRequestPacket *) p;
            
            // Check if the incoming request packet has a mapID. If that 
            // is the case resent the request to a module with that map!
            if (inPacket->getMapID() < MAX_UINT32) {
               mc2dbg
                  << "Got a SSIRequestPacket with a MapID<MAX_UINT32" 
                  << endl;
               inPacket->setMapID(inPacket->getMapID());
               
               sendRequestPacket( inPacket );
               mc2dbg4 << "   StreetSegmentItemRequestPacket send to module "
                       << "with map" << inPacket->getMapID() << endl;
               // p should _not_ be deleted here since it's either deleted
               // or inserted into queue in sendRequestPacket
               
            } else {
               // Find what map to use
               int32 lat = inPacket->getLatitude();
               int32 lon = inPacket->getLongitude();
              
               // Find out if more than one map i located at (lat,lon)
               Vector mapIDs(8);
               findMapsAtCoordinate(lat, lon, &mapIDs);
               if (mapIDs.getSize() > 1) {
                  mc2dbg << "More than one mapID!" << endl;
                  // MORE THAN ONE map at (lat,lon), return ID of all maps
                  StreetSegmentItemReplyPacket* replyPack = 
                     new StreetSegmentItemReplyPacket( inPacket,
                                                       StringTable::NOT_UNIQUE,
                                                       inPacket->getIndex(),
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_INT32,
                                                       MAX_INT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32);
                  for (uint32 i=0; i<mapIDs.getSize(); i++) {
                     mc2dbg4 << "adding possible map with ID=" 
                             << mapIDs.getElementAt(i) << endl;
                     replyPack->addMapID(mapIDs.getElementAt(i));
                  }
                  
                  // Return the reply packet to the caller
                  sendAndDeletePacket( replyPack, 
                                       p->getOriginIP(), 
                                       p->getOriginPort());
                  delete p;
                  
               } else if (mapIDs.getSize() == 1) {
                  mc2dbg << "One mapID!" << endl;
                  // EXACTLY ONE map at (lat,lon), return that
                  inPacket->setMapID(mapIDs.getElementAt(0));
                  
                  sendRequestPacket( inPacket );
                  mc2dbg4 << "   SSIRequestPacket resend to module" 
                          << endl;
                  // p should _not_ be deleted here since it's either deleted
                  // or inserted into queue in sendRequestPacket
               } else {
                  mc2dbg << "Lat = " << lat << endl;
                  mc2dbg << "Lon = " << lon << endl;
                  mc2dbg << "No map found" << endl;
                  // NO map found at (lat,lon)
                  StreetSegmentItemReplyPacket* replyPack = 
                     new StreetSegmentItemReplyPacket( inPacket,
                                                       StringTable::MAPNOTFOUND,
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_INT32,
                                                       MAX_INT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32,
                                                       MAX_UINT32);
                  sendAndDeletePacket( replyPack, 
                                       p->getOriginIP(), 
                                       p->getOriginPort());
                  delete p;
               } 
            }
            return (true);
         }
         break;

         case Packet::PACKETTYPE_COVEREDIDSREQUESTPACKET : {
            mc2dbg1 << "CoveredIDsRequestPacket recv" << endl;
            /*
             *    Determine which maps that the coordinates are located in
             *    and retransmit the request to the correct module or
             *    send it back.
             */            
            CoveredIDsRequestPacket * inPacket = (CoveredIDsRequestPacket *) p;
            uint32 mapID = inPacket->getMapID();

            // The packet already has an ID - send it to the available.
            if ( mapID != MAX_UINT32 ) {
               // Skip the strange stuff below.
               sendRequestPacket( inPacket );
               return true;
               break;
            }

            Vector mapIDVector;
            if ( mapID == MAX_UINT32 ) {
               int32 lat = inPacket->getLat();
               int32 lon = inPacket->getLon();
               uint32 outerRadiusMeters = inPacket->getOuterRadius();
               
               findMapsWithinRadius(mapIDVector,
                                    MC2Coordinate(lat, lon),
                                    outerRadiusMeters);

               
               
               if ( mapIDVector.size() != 0 ) {                 
                  mapID = mapIDVector[0];
               }
            } else {
               mapIDVector.push_back(mapID);
            }
                        
            if ( mapID == MAX_UINT32 ) {
               mc2dbg << "[MR]: CoveredID map id == MAX_UINT32" << endl;
               CoveredIDsReplyPacket* replyPack = 
                  new CoveredIDsReplyPacket( inPacket);
               replyPack->setMapID(MAX_UINT32);
               replyPack->setStatus(StringTable::MAPNOTFOUND);
               sendAndDeletePacket( replyPack, 
                                    p->getOriginIP(), 
                                    p->getOriginPort());
               delete p;
            } else if ( !inPacket->isLatLon() ) {
               // Only one map covered - send the packet on to the
               // right module.
               // This can only be done if the mapID is set from the
               // beginning, since the UserRights will be wrong otherwise.
               inPacket->setMapID(mapID);
               
               sendRequestPacket( inPacket );
               mc2dbg << "[MR]: CoveredIDsRequestPacket resent"
                  " to module with map " << mapID << endl;
               // p should _not_ be deleted here since it's either deleted
               // or inserted into queue in sendRequestPacket
            } else {
               // Many maps covered. Set the mapID to MAX_UINT32
               // (I think that it should be that way) and add the
               // mapIDs instead of the itemids.
               mc2dbg << "[MR]: " << mapIDVector.size() 
                      << " maps covered " << endl;
               CoveredIDsReplyPacket* replyPack = 
                  new CoveredIDsReplyPacket( inPacket,
                                             CoveredIDsReplyPacket::
                                         calcPacketSize(mapIDVector.size()));
               replyPack->setMapID(MAX_UINT32);                      
               replyPack->setStatus(StringTable::OK);

               for( uint32 i=0; i < mapIDVector.size(); ++i ) {
                  // Add the mapid as invalid type.
                  replyPack->addID(mapIDVector[i],
                                   ItemTypes::numberOfItemTypes);
               }
               
               sendAndDeletePacket( replyPack, 
                                    p->getOriginIP(), 
                                    p->getOriginPort());
               delete p;
            }
            return (true);
         }
         break;


         case Packet::PACKETTYPE_BBOXREQUEST: {
            DebugClock klockan;
            BBoxRequestPacket* req =
               static_cast<BBoxRequestPacket*>( p );
            
            sendAndDeletePacket( handleBBoxRequest( req ),
                                 p->getOriginAddr() );
            delete p;
            mc2dbg8 << "[MapReader]: BBox in " << klockan << endl;
            return true;
         }
         break;
        

         case Packet::PACKETTYPE_MAPREQUEST: {
            // Will add overview map info if it is a routemap request.
            // Will also check the map versions and return a reply
            // if there cached version of the sender's map is ok.
            MapReplyPacket* reply = handleMapRequestPacket(p); 
            // Don't delete the packet.
            // Send the request for this map to an appropriate module
            if ( reply == NULL ) {
               sendRequestPacket((RequestPacket*)p);
            } else {
               sendAndDeletePacket( reply,
                                    p->getOriginIP(),
                                    p->getOriginPort());
               delete p;
            }
            
            return true;
         }
         break;
         case Packet::PACKETTYPE_TOPREGIONREQUEST: {
            TopRegionReplyPacket* reply = handleTopRegionRequestPacket(p); 
            
            sendAndDeletePacket( reply,
                                 p->getOriginIP(),
                                 p->getOriginPort());
            delete p;
            return true;
         }
         break;
         case Packet::PACKETTYPE_SMALLESTROUTINGCOSTREQUEST : {
            SmallestRoutingCostReplyPacket* reply =
               handleSmallestRoutingCostRequestPacket(
               static_cast<SmallestRoutingCostRequestPacket*> (p));
            sendAndDeletePacket( reply,
                                 p->getOriginIP(),
                                 p->getOriginPort());
            delete p;
            // We have processed the packet.
            return true;
         }
         break;
         case Packet::PACKETTYPE_ROUTINGINFOREQUEST : {
            RoutingInfoReplyPacket* reply =
               handleRoutingInfoRequest(static_cast<RoutingInfoRequestPacket*>
                                        (p));
            sendAndDeletePacket( reply,
                                 p->getOriginIP(),
                                 p->getOriginPort());
            delete p;
            // We have processed the packet.
            return true;
         }
         break;

      case Packet::PACKETTYPE_COPYRIGHTBOX_REQUEST:
         CopyrightBoxReplyPacket* reply = 
            handleCopyrightBoxRequest(
                   static_cast<const CopyrightBoxRequestPacket*>( p ) );
         sendAndDeletePacket( reply, p->getOriginAddr() );
         return true;
         break;
         
      }  // end switch
      return (false);
   } else {
      // The packet was handled by the method in the superclass
      return (true); 
   }
}

/**
 * Makes sure each map that "index.db" referes to are located
 * on the disk.
 * It will report each map that are missing and force exit of program
 * if any map is missing.
 */
void verifyMaps( const MapModuleNoticeContainer& indexdb ) {
   mc2dbg << "[MapReader] verifying maps..." << endl;
   bool missingMaps = false;
   for ( uint32 i = 0; i < indexdb.getSize(); ++i ) {
      const MapModuleNotice* map = indexdb[ i ];
      MC2String filename = GenericMapHeader::
         getFilenameFromID( indexdb.getMapID( map->getMapName() ) );
      filename = DataBufferCreator::getMapOrIdxPath( filename,
                                                     Properties::getMapSet() );

      if ( ! File::fileExist( filename ) &&
           ! File::fileExist( filename + ".gz" ) &&
           ! File::fileExist( filename + ".bz2" ) ) {
         mc2dbg << warn << "[MapReader] Missing file: " << filename << endl;
         missingMaps = true;
      }
   }
   if ( missingMaps ) {
      mc2dbg << error << "Missing maps! See above messages for more info."
             << endl;
      MC2_ASSERT( false );
   }

   mc2dbg << "[MapReader] verified maps, all maps are ok." << endl;
}

void
MapReader::initLeader(set<MapID>& allMaps) 
{
// Should _not_ call Reader::initLeader(), since that contacts the MM...
   
   mc2dbg1 << "MapReader::initLeader" << endl;

   if ( m_indexDB.getSize() == 0  &&
        m_indexDB.load("index.db", Properties::getMapSet()) ) {

      verifyMaps( m_indexDB );

      mc2dbg1 << "MapReader::initLeader() Indexfile, " 
              << "index.db" << ", loaded OK" << endl;

      const MapModuleNotice* mn = NULL;

      // Testing.
      if ( false ) {
         WriteableMapModuleNoticeContainer w;
         w.load("index.db");
         w.save("index2.db");
      }
      // Check if all the map should be loaded or some specified 
      // on the commandline
      if ((m_startWithMaps != NULL) && (m_startWithMaps->getSize() > 0)) {
         // The user have specified some maps to load
         mc2dbg2 << "To create " << m_startWithMaps->getSize() 
                 << " LoadMapRequestPacket(s)" << endl;
         for (uint32 i=0; i < m_startWithMaps->getSize(); i++) {
            mc2dbg4 << "Inserts LoadMapRequest(" << mn->getMapID() 
                    << ", 0, 0) into local queue" << endl;
            // By setting originIP and port to 0 we don't get any ack...
            allMaps.insert(m_startWithMaps->getElementAt(i));

            LoadMapRequestPacket* pack = 
               new LoadMapRequestPacket(m_startWithMaps->getElementAt(i),
                                        0,
                                        0);
            sendAndDeletePacket(pack, m_ownAddr);
            
         }
      } else {
         mc2dbg2 << "To insert " << m_indexDB.getSize() 
            << " maps into the allMap array" << endl;
         for (uint32 i=0; i < m_indexDB.getSize(); i++)
         {
            if ( (mn = m_indexDB[i]) != NULL){
               uint32 mapID = mn->getMapID();
               mc2dbg4 << "Inserts map ID " << mapID 
                       << " inte the allMaps array" << endl;
               allMaps.insert( mapID );
            } else {
               mc2log << error << "ERROR: dynamic_cast<MapModuleNotice*> "
                      << "(m_indexDB[" 
                      << i << "])) returns NULL. Exit program!" << endl;
               exit(2);
            }
         }
      }

   } else {
      if ( m_indexDB.getSize() == 0 ) {
         mc2log << fatal
                << "MapReader::initLeader(). Error loading indexfile \"" 
                << "index.db"  << "\", terminating" << endl;
         exit(1);
      }
   }

}

uint32
MapReader::findMapFromCoordinate(int32 lat, int32 lon, float64* mapDist) 
{
   if (m_indexDB.getSize() > 0) {
      mc2dbg2 << "FindMapFromCoordinate(" << lat <<", " << lon 
              << ")" << endl;

      uint32 closestID = MAX_UINT32;
      float64 closestDist = MAX_FLOAT64;
      uint32 i=0;
      while ((i < m_indexDB.getSize()) && closestDist > 0) { 
         if ( MapBits::isUnderviewMap(
                  (m_indexDB[i] )->getMapID()) ) {
            float64 tmpDist = (m_indexDB[i])
               ->getGfxData()->signedSquareDistTo(lat, lon);
            mc2dbg2 << "   tmpDist = " << tmpDist 
                    << ", closestDist = " << closestDist << endl;
            if (tmpDist < closestDist) {
               closestDist = tmpDist;
               closestID = m_indexDB[i]->getMapID();
            }
         }
         i++;
      }
      mc2dbg2 << "   Square-distance to closest map (" << closestID 
                  << ") is " << closestDist << endl;
      if ( mapDist != NULL ) {
         *mapDist = closestDist;
      }
      return (closestID);
   } else {
      return (MAX_UINT32);
   }
}

void
MapReader::findMapsAtCoordinate(int32 lat, int32 lon, Vector* mapIDs) 
{
  mc2dbg4 << "FindMapsAtCooridinate()" << endl;
   
   for (uint32 i = 0; i < m_indexDB.getSize(); i++) {
      // Add if orinary map and (lat,lon) inside that
      const MapModuleNotice* mn = m_indexDB[i];
      if ( (MapBits::isUnderviewMap(mn->getMapID())) &&
           (mn->getGfxData()->insidePolygon(lat, lon) > 0)) {
         mc2dbg4 << "   Coordinate inside map " << mn->getMapID() 
                 << endl;
         mapIDs->addLast(mn->getMapID());
      }
   }
}

BBoxReplyPacket*
MapReader::handleBBoxRequest( const BBoxRequestPacket* req )
{
   BBoxReqPacketData data;
   req->get( data );

   Vector mapIDs;
   bool onlyAddCountryMaps = data.country() &&
                            !data.underview() &&
                            !data.overview();
   
   findMapsInMC2BoundingBox( &data.getBBox(),
                             &mapIDs,
                             onlyAddCountryMaps );

   vector<uint32> resMaps;
   // Now add the maps back to include them in the reply
   for ( Vector::iterator it = mapIDs.begin();
         it != mapIDs.end();
         ++it ) {

      if ( MapBits::isUnderviewMap( *it ) && data.underview() ) {
         resMaps.push_back( *it );
      }
      if ( MapBits::isCountryMap( *it ) && data.country() ) {
         resMaps.push_back( *it );
      }
   }

   return new BBoxReplyPacket( req, resMaps );
   
}

void
MapReader::findMapsInMC2BoundingBox(const MC2BoundingBox* bbox,
                                    Vector* mapIDs, 
                                    bool onlyAddCountryMaps ) 
{
   mc2dbg4 << "FindMapsInMC2BoundingBox()" << endl;
   
   MC2BoundingBox mapBBox;
   
   for (uint32 i = 0; i < m_indexDB.getSize(); i++) {      
      const MapModuleNotice* mn = m_indexDB[i];
         // Either only countrymaps or non-overviewmaps.
         if ( ( onlyAddCountryMaps && 
                MapBits::isCountryMap( mn->getMapID() ) ) ||
              ( ! onlyAddCountryMaps && 
                ! MapBits::isOverviewMap( mn->getMapID() ) ) ) {

         // Get the bounding box for this map.
         mn->getBBox(mapBBox);

         if ( bbox->overlaps(mapBBox)) {
            mc2dbg8 << "[MR]: Map " << mn->getMapID()
                    << " has bbox " << mapBBox 
                    << " and overlaps map bounding box " << *bbox
                    << endl;
            mapIDs->addLast(mn->getMapID());
         } else {
            mc2dbg8 << "[MR]: Map " << MC2HEX(mn->getMapID())
                    << " with bbox " << mapBBox 
                    << " does not overlap " << *bbox << endl;
               
         }
      }
   }
}  


void
MapReader::findMapsWithinRadius(Vector& mapIDVector,
                                const MC2Coordinate& center,
                                uint32 radiusMeters,
                                bool addUnderviewMaps,
                                bool addCountryMaps)
{
   // Start by taking the maps that cover the bbox of the circle.
   int32 lat = center.lat;
   int32 lon = center.lon;
   MC2BoundingBox bbox(lat, lon, lat, lon);
   
   double cosLat = ::cos(GfxConstants::invRadianFactor *
                         double(lat));
   
   int32 radiusMC2 =
      int32(radiusMeters * GfxConstants::METER_TO_MC2SCALE);
   
   mc2dbg << "[MR]: Radius is " << radiusMeters << endl;
   mc2dbg << "[MR]: BBox is center is" << bbox << endl;
   // I can never keep track of the order of the parameters
   // so I update the bbox.
   bbox.update(lat+radiusMC2, int32(lon + radiusMC2*cosLat));
   bbox.update(lat-radiusMC2, int32(lon - radiusMC2*cosLat));
   mc2dbg2 << "[MR]: BBox is " << bbox << endl;

   // Start by finding the maps with overlapping bboxes.
   findMapsInMC2BoundingBox( &bbox, &mapIDVector);
   // Remove underview maps or country maps.
   if ( (!addCountryMaps) || (!addUnderviewMaps) ) {
      for ( Vector::iterator it = mapIDVector.begin();
            it != mapIDVector.end();
            /* Don't do anything */ ) {
         if ( (MapBits::isUnderviewMap(*it) && addUnderviewMaps) ||
              (MapBits::isCountryMap(*it) && addCountryMaps) ) {
            ++it;
         } else {
            it = mapIDVector.erase(it);
         }
      }
   }

   int64 squareRadius = radiusMeters * radiusMeters;
   // Check if the bounding boxes are inside the maps
   // or the distance is less than the radius.
   MC2BoundingBox mapBBox;
   for( Vector::iterator it = mapIDVector.begin();
        it != mapIDVector.end(); /**/ ) {
      bool keep = false;
      const MapModuleNotice* mn = m_indexDB.getMap(*it);
      // Check if the center is inside
      mn->getBBox(mapBBox);
      if ( mapBBox.inside( bbox ) ) {
         keep = true;
         mc2dbg << "[MR]: Keeping map. " << *it
                << " - Map is inside bbox " << endl;         
      } else if ( mn->getGfxData()->nbrCornersInsidePolygon(bbox, 1) != 0 ) {
         // A corner of the bounding box is inside.
         keep = true;
         mc2dbg << "[MR]: Keeping map. " << *it
                << " - One corner is inside" << endl;
      } else {
         int64 dist = mn->getGfxData()->signedSquareDistTo(center.lat,
                                                           center.lon);
         if ( dist <= squareRadius ) {
            // Calculate the minimum distance from the center to
            // the map in question.
            // FIXME: We do not have to check all the points in the
            // gfxData, only until distance is less than radius.
            keep = true;       
         }
         const char* nott = "";
         if ( !keep ) {
            nott = "NOT";
         }
         mc2dbg << "[MR]: " << nott << " Keeping map. " << *it
                << " - Distance is " << nott << " ok: "
                << (::sqrt(fabs(float(dist)))*(dist/fabs(float(dist))))
                << endl;
      }

      // I think that should cover most cases except for very
      // very strange maps.
      
      if ( keep ) {
         ++it;
      } else {
         it = mapIDVector.erase(it);
      }
   }      
}

CopyrightBoxReplyPacket* MapReader::
handleCopyrightBoxRequest( const CopyrightBoxRequestPacket* packet ) const {


   CopyrightHandler copyrights( m_indexDB, packet->getLanguage() );

   CopyrightBoxReplyPacket* reply = 
      new CopyrightBoxReplyPacket( packet, copyrights.getCopyrightHolder() );

   return reply;
}

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

#include "InfoProcessor.h"


#include "DisturbanceInfoPacket.h"
#include "DisturbanceList.h"
#include "GfxFeatureMap.h"
#include "MC2BoundingBox.h"
#include "MapSettings.h"
#include "MapUtility.h"
#include "UserRightsMapInfo.h"
#include "DeleteHelpers.h"
#include "InfoDatexUtils.h"
#include "IDTranslationTable.h"
#include "InfoSQL.h"

#include "GetTMCCoordinatePacket.h"
#include "DisturbancePacket.h"
#include "GfxFeatureMapPacket.h"
#include "RouteTrafficCostPacket.h"
#include "UpdateDisturbancePacket.h"
#include "DisturbanceChangesetPacket.h"
#include "FetchAllDisturbancesPacket.h"

#include "SQLTrafficElementDatabase.h"
#include "DisturbanceChangeset.h"

#include "BitUtility.h"
#include "TrafficMapInfo.h"

// TODO: remove
#include "TrafficSituationElement.h"

#define INFOP "[INFOP:" << __LINE__ << "] "

InfoProcessor::InfoProcessor( MapSafeVector* loadedMaps ):
   MapHandlingProcessor( loadedMaps ),
   m_database( new InfoSQL() ),
   m_trafficDatabase( new SQLTrafficElementDatabase() )
{
   if ( ! m_database->deleteOldDisturbances() ) {
      mc2log << error
             << "[InfoProcessor] Deletion of old disturbances failed!" << endl;
   }

}


InfoProcessor::~InfoProcessor() {

}


TrafficMapInfo*
InfoProcessor::getTrafficMapInfo( uint32 mapID ) {
   map<uint32, TrafficMapInfo*>::iterator it;
   it = m_handleUnitMap.find(mapID);

   if( it != m_handleUnitMap.end() ) {
      return it->second;
   } else {
      return NULL;
   }
}

StringTable::stringCode
InfoProcessor::loadMap( uint32 mapID, uint32 &mapSize) {
   TrafficMapInfo* foundUnit = getTrafficMapInfo(mapID);

   if ( foundUnit != NULL ) {
      mc2log << warn << "[InfoProcessor] map "
             << mapID << " already loaded" << endl;
      return StringTable::ERROR_MAP_LOADED;
   } else {

      TrafficMapInfo* newUnit = new TrafficMapInfo( mapID );

      // load from InfoSQL
      bool status = newUnit->load( *m_database );

      // Allready added to safevector outside the function
      if ( status ) {
         m_handleUnitMap.insert( make_pair( mapID, newUnit ) );
         mapSize = newUnit->mapSize();
         return StringTable::OK;
      } else {
         delete newUnit;
         mapSize = 0;
         return StringTable::NOTOK;
      }
   }
}


StringTable::stringCode
InfoProcessor::deleteMap(uint32 mapID)
{

   TrafficMap::iterator tMap = m_handleUnitMap.find(mapID);
   if(tMap != m_handleUnitMap.end()){
      TrafficMapInfo* unit = tMap->second;
      m_handleUnitMap.erase(tMap);
      delete unit;
      return StringTable::OK;
   } else {
      return StringTable::MAPNOTFOUND;
   }
}

MC2String getDistText( const DisturbanceElement* dist,
                       StringTable::languageCode langCode ) {
   // Here special tricks can be done to get a better string, especially
   // if dist->getText() is empty.
   MC2String text;
   text = dist->getText();
   mc2dbg4 << INFOP << "Disturbance text: " << text << endl;
   return text;
}


inline GfxFeatureMapReplyPacket*
InfoProcessor::handleGfxFeatureMapRequest(const GfxFeatureMapRequestPacket* p,
                                          char* packetInfo )
{
   mc2dbg4 << "InfoProcessor::handleGfxFeatureMapRequest" << endl;

   uint32 mapID = p->getMapID();

   TrafficMapInfo* tu = getTrafficMapInfo( mapID );

   if ( tu == NULL ) {
      // ERROR - Maybe ackpack here instead?
      GfxFeatureMapReplyPacket* reply = new GfxFeatureMapReplyPacket(p);
      reply->setStatus(StringTable::MAPNOTFOUND);
      return reply;
   }

   MC2BoundingBox bbox;

   p->getMC2BoundingBox(&bbox);
   ScreenSize screenSize = p->getScreenSize();

   MapSettings mapSettings;
   UserRightsMapInfo rights;
   p->getMapSettingsAndRights( mapSettings, rights );
   updateUserRights(rights);

   // Set the boundingbox for the coordinates that will be included in
   // the area
   //
   // Calculate the extra distance that shuold be added to each side
   // of the bbox. The distance corresponds to approximately 30 pixels.

   // FIXME: Add filtering.
   //        int filtScaleLevel = p->getFiltScaleLevel();
   int minScaleLevel  = p->getMinScaleLevel();
   int maxScaleLevel  = p->getMaxScaleLevel();


   // Create and init map
   GfxFeatureMap gfxFeatureMap;

   gfxFeatureMap.setMC2BoundingBox(&bbox);
   gfxFeatureMap.setScreenSize( screenSize );
   gfxFeatureMap.setScaleLevel( maxScaleLevel );

   const map<uint32, const DisturbanceElement*>& distList = tu->getDisturbanceList();
   map<uint32, const DisturbanceElement*>::const_iterator it;

   // FIXME: Add real IDTranslationTable. Not really needed since
   // we never use 0x90000000 for drawing.
   IDTranslationTable transTable;
   mc2dbg4 << "[InfoP] maprights for user: \n" << rights << "\n[InfoP] distList: "
           << STLUtility::co_dump(distList, ",") << endl;
   for(it = distList.begin(); it != distList.end(); ++it) {
      // Check user rights
      {
         /// Get one node and check the provider id
         const set<uint32>& nodeIDs = it->second->getNodeIDSet();
         IDPair_t idToCheck( p->getMapID(), *( nodeIDs.begin() ) );
         if ( ! rights.itemAllowed( it->second->getNeededRights(),
                                    idToCheck,
                                    transTable ) ) {
            mc2dbg4 << "[InfoP] item " << (it->second) << " needs rights: "
                    << it->second->getNeededRights() << endl;
            // Not allowed - skip
            continue;
         }
      }

      // Only add disturbances if the we are at
      // or a more detalied level (see MapUtility)
      // Same constant is used in the GfxFeatureMapImageRequest
      // to avoid sending packets.
      if ( maxScaleLevel >= TRAFFIC_INFO_LEVEL ) {
         const DisturbanceElement* dist = it->second;

         MC2String distText = getDistText(
            dist,
            ItemTypes::getLanguageTypeAsLanguageCode( p->getLanguage() ) );
         // create a new GfxTrafficInfoFeature this will in the end set the
         // angle to 0...
         GfxFeature* feature =
            GfxFeature::createNewFeature( GfxFeature::TRAFFIC_INFO,
                                          distText.c_str() );
         GfxTrafficInfoFeature* trafficInfo =
            static_cast<GfxTrafficInfoFeature*> (feature);
         trafficInfo->setTrafficInfoType(dist->getType());
         trafficInfo->setStartTime( dist->getStartTime() );
         trafficInfo->setEndTime( dist->getEndTime() );
         // setAngle?? its set to zero when created... 
         // what do we use the angle for?

         // Add the single coordinate to the polygon.
         feature->addNewPolygon(true, 1);
         map<uint32, int32>::const_iterator it = dist->getLatMap().begin();
         int32 latitude = it->second;
         it = dist->getLonMap().begin();
         int32 longitude = it->second;
         feature->addCoordinateToLast(latitude, longitude);

         feature->setScaleLevel(minScaleLevel);
         // Check insideness
         if ( bbox.contains( MC2Coordinate( latitude, longitude ) ) ) {
            gfxFeatureMap.addFeature(feature);
         } else {
            delete feature;
         }
      }
   }
   // Create the reply
   GfxFeatureMapReplyPacket* reply = new GfxFeatureMapReplyPacket(p);

   DataBuffer buf(gfxFeatureMap.getMapSize());
   gfxFeatureMap.save(&buf);

   // Copied from GfxFeatureMapProcessor...
   const uint32 EXTRALENGTH = 200;
   if ( (gfxFeatureMap.getMapSize()+EXTRALENGTH) > reply->getBufSize()) {
      reply->resize(gfxFeatureMap.getMapSize() +
                    REPLY_HEADER_SIZE + EXTRALENGTH);
   }
   reply->setGfxFeatureMapData(buf.getCurrentOffset(), &buf);
   reply->setStatus( StringTable::OK );

   {
      // Add some info to the JT-printout (number of features added )
      char tmpInfo[1024];
      sprintf( tmpInfo, "f=%d", gfxFeatureMap.getNbrFeatures() );
      strcat( packetInfo, tmpInfo );
   }

   return reply;
}


inline
DisturbanceReplyPacket*
InfoProcessor::handleDisturbanceRequest(const DisturbanceRequestPacket* p)
{
   uint32 mapID = p->getRequestedMapID();
   map<uint32, const DisturbanceElement*> distMap;

   bool removeDisturbances = false;

   TrafficMapInfo* currentUnit = getTrafficMapInfo(mapID);

   if( currentUnit != NULL ) {
      // Return all the disturbances
      //currentUnit->getDisturbances(distMap, 0, MAX_UINT32);
      distMap = currentUnit->getDisturbanceList();
   }

   return new DisturbanceReplyPacket(p,
                                     distMap,
                                     removeDisturbances);

}


inline
DisturbanceInfoReplyPacket*
InfoProcessor::handleDisturbanceInfoRequest(const DisturbanceInfoRequestPacket* p)
{
   mc2dbg4 << "InfoProcessor: Processing DisturbanceInfoRequestPacket"
          << endl;
   DisturbanceInfoReplyPacket* replyPacket =
      new DisturbanceInfoReplyPacket(p);

   UserRightsMapInfo userRights;

   p->getRights(userRights);
   updateUserRights(userRights);


   set<uint32> mapIDs;
   multimap<uint32, uint32> nodeMap;

   uint32 nbrNodes = p->getNumberOfNodes();
   for(uint32 i = 0; i < nbrNodes; i++) {
      IDPair_t idPair = p->getMapAndNode(i);
      uint32 mapID = idPair.first;
      uint32 nodeID = idPair.second;
      mapIDs.insert(mapID);
      nodeMap.insert(pair<uint32,uint32>(mapID,nodeID));
   }

   set<uint32>::iterator mi;
   multimap<uint32, uint32>::iterator ni;
   multimap<uint32, uint32>::iterator lower;
   multimap<uint32, uint32>::iterator upper;
   for(mi = mapIDs.begin(); mi != mapIDs.end(); ++mi) {
      uint32 mapID = *mi;
      lower = nodeMap.lower_bound(mapID);
      upper = nodeMap.upper_bound(mapID);
      set<uint32> nodeIDs;
      for( ni = lower; ni != upper; ++ni) {
         uint32 nodeID = ni->second;
         nodeIDs.insert(nodeID);
      }

      TrafficMapInfo* thu = getTrafficMapInfo(mapID);
      if ( thu != NULL ) {
         
         TrafficMapInfo::NodeIDsToDisturbances distElems =
            thu->findNodeID( nodeIDs, userRights );
         TrafficMapInfo::NodeIDsToDisturbances::iterator it;
         for( it = distElems.begin(); it != distElems.end(); it++) {
            // Add disturbance to packet
            TrafficMapInfo::NodeIDsToDisturbances::value_type thePair = *it;
            TrafficMapInfo::NodeIDsToDisturbances::value_type::second_type
               distElem = thePair.second;

            replyPacket->addDisturbance( mapID,
                                         thePair.first,
                                         distElem->getType(),
                                         distElem->getDisturbanceID(),
                                         distElem->getText() );

         }
      } else {
         mc2dbg << "InfoProcessor: TrafficMapInfo == NULL!"
                << ", map " << mapID <<endl;
      }
   }

   return replyPacket;
}

inline
RouteTrafficCostReplyPacket*
InfoProcessor::
handleRouteTrafficCostRequest( const RouteTrafficCostRequestPacket* rtcrp,
                               char* packetInfo )
{
   TrafficMapInfo::NodesWithDelays vect;
   // FIXME: Fill that vector with disturbances.
   uint32 mapID = rtcrp->getMapID();


   TrafficMapInfo* thu = getTrafficMapInfo(mapID);

   int nodeCounter  = 0;
   if( thu != NULL ) {
      UserRightsMapInfo rights;
      rtcrp->getRights(rights);
      mc2dbg2  << "[IP]: Packet rights " << rights << endl;
      updateUserRights(rights);

      nodeCounter = thu->getNodesWithDelays(rights, vect);

     {
        // Add some info to the JT-printout
        char tmpInfo[1024];
        sprintf( tmpInfo, "n=%d", nodeCounter );
        strcat( packetInfo, tmpInfo );
     }

   } else {
      return new RouteTrafficCostReplyPacket( rtcrp,
                                              StringTable::MAPNOTFOUND,
                                              vect );
   }
   mc2dbg2 << "added costs to " << nodeCounter
          << " nodes on map " << mapID << "." << endl;
   return new RouteTrafficCostReplyPacket( rtcrp,
                                           StringTable::OK,
                                           vect );
}

GetTMCCoordinateReplyPacket*
InfoProcessor::
handleGetTmcCoordRequest( const GetTMCCoordinateRequestPacket& gtcrp )
{
   TrafficSituationElement::CoordCont firstCoords;
   TrafficSituationElement::CoordCont secondCoords;
   MC2String firstLocation;
   MC2String secondLocation;
   int32 extent;
   TrafficDataTypes::direction direction;

   gtcrp.getPoints( firstLocation, secondLocation, extent, direction );

   const bool ok = m_database->getTMCCoords( firstLocation, secondLocation,
                                             extent, direction,
                                             firstCoords, secondCoords );
   mc2dbg << INFOP << "Fetching TMC '" << firstLocation << "' and '"
          << secondLocation << "' with extent " << extent << " and direction "
          << direction << " gave " << firstCoords.size() << "("
          << secondCoords.size() << ") coordinates" << endl;


   GetTMCCoordinateReplyPacket* answer = NULL;
   if ( ok ) {
      const uint32 size = ( firstCoords.size() + secondCoords.size() ) * 8;
      answer = new GetTMCCoordinateReplyPacket(&gtcrp, StringTable::OK, size);
      answer->addCoordinatesToPacket(firstCoords, secondCoords);
   } else {
      answer = new GetTMCCoordinateReplyPacket(&gtcrp, StringTable::NOTOK, 0);
   }
   return answer;
}

void InfoProcessor::
updateTrafficUnits( DisturbanceChangeset& changes ) {
   uint32 mapID = MAX_UINT32;
   if ( ! changes.getUpdateSet().empty() ) {
      mapID = changes.getUpdateSet().front()->getMapID();
   } else if ( ! changes.getRemoveSet().empty() ) {
      mapID = changes.getRemoveSet().front()->getMapID();
   } else {
      // nothing we can do without map id
      return;
   }

   TrafficMapInfo* unit = getTrafficMapInfo( mapID );

   if ( unit == NULL ) {
      // If the map was not loaded, then just ignore traffic unit update.
      // The unit will be loaded next time it is needed.
      return;
   }
   // send the updated and removed set to the unit,
   // the unit will take ownership of the updated disturbances but not
   // the removed disturbances.
   DisturbanceChangeset::Elements updated;
   changes.swapUpdateSet( updated );
   unit->updateDisturbances( updated, changes.getRemoveSet() );
}

DisturbanceChangesetReplyPacket*
InfoProcessor::
handleDisturbanceChangesetRequest( const DisturbanceChangesetRequestPacket&
                                   packet ) {
   

   // get changset from packet and update the database
   DisturbanceChangeset changes;
   packet.getChangeset( changes );
   int status = m_trafficDatabase->updateChangeset( changes );

   // get separate status bits
   using BitUtility::getBit;
   bool updateStat = getBit( (uint32)status,
                             TrafficElementDatabase::UPDATE_FAILED );
   bool removeStat = getBit( (uint32)status,
                             TrafficElementDatabase::REMOVE_FAILED );

   // if we updated the database, we also need to update traffic handler units
   // that were affected.
   if ( status == TrafficElementDatabase::OK ) {
      // Find and reload any traffic units for each map that was affected,
      // so they get the new data.
      updateTrafficUnits( changes );
   } else if ( updateStat && ! removeStat ) {
      // only update traffic units for updated elements.
      DisturbanceChangeset::Elements empty;
      DisturbanceChangeset::Elements updates;
      changes.swapUpdateSet( updates );
      DisturbanceChangeset updateChanges( updates, empty );
      updateTrafficUnits( updateChanges );
   } else if ( ! updateStat && removeStat ) {
      // only update traffic units for removed elements.
      DisturbanceChangeset::Elements empty;
      DisturbanceChangeset::Elements removed;
      changes.swapRemoveSet( removed );
      DisturbanceChangeset removeChanges( empty, removed );
      updateTrafficUnits( removeChanges );
   }

   return new DisturbanceChangesetReplyPacket( &packet,
                                               updateStat, removeStat );

}

FetchAllDisturbancesReplyPacket*
InfoProcessor::
handleFetchAllDisturbances( const FetchAllDisturbancesRequestPacket& pack ) {
   TrafficElementDatabase::TrafficElements disturbances;
   bool ok = m_trafficDatabase->fetchAllDisturbances( pack.getProviderID(),
                                                      disturbances );

   FetchAllDisturbancesReplyPacket* reply = 
      new FetchAllDisturbancesReplyPacket( &pack, disturbances );
   if ( ! ok ) {
      reply->setStatus( StringTable::NOTOK );
   } else {
      reply->setStatus( StringTable::OK );
   }

   return reply;
}

Packet*
InfoProcessor::handleRequestPacket( const RequestPacket& pack,
                                    char* packetInfo )
{
   uint16 subType = pack.getSubType();
   mc2dbg2 << "InfoProcessor recieved packet with subtype = "
           << subType << endl;

   Packet* answerPacket = NULL;

   switch (subType) {
      case Packet::PACKETTYPE_GETTMCCOORREQUEST : {
         answerPacket = handleGetTmcCoordRequest(
            static_cast<const GetTMCCoordinateRequestPacket&>( pack ) );
      }
      break;
      case Packet::PACKETTYPE_GFXFEATUREMAPREQUEST: {
         answerPacket =
            handleGfxFeatureMapRequest(
               static_cast<const GfxFeatureMapRequestPacket*>(&pack), packetInfo );
         break;
      }

      case Packet::PACKETTYPE_DISTURBANCEREQUEST:
      {
         answerPacket =
            handleDisturbanceRequest(
               static_cast<const DisturbanceRequestPacket*>(&pack));
         break;
      }
      case Packet::PACKETTYPE_DISTURBANCEINFOREQUEST:
      {
         answerPacket =
            handleDisturbanceInfoRequest(
               static_cast<const DisturbanceInfoRequestPacket*>(&pack));
         break;
      }

      case Packet::PACKETTYPE_ROUTETRAFFICCOSTREQUEST: {
         answerPacket = handleRouteTrafficCostRequest(
            static_cast<const RouteTrafficCostRequestPacket*>(&pack), packetInfo );
      }
      break;

      case Packet::PACKETTYPE_FETCH_ALL_DISTURBANCES_REQUEST:
         answerPacket = 
            handleFetchAllDisturbances( static_cast
                                        <const FetchAllDisturbancesRequestPacket&>( pack ) );
         break;
      case Packet::PACKETTYPE_DISTURBANCE_CHANGESET_REQUEST:
         answerPacket =
            handleDisturbanceChangesetRequest( static_cast
                                               < const DisturbanceChangesetRequestPacket& >( pack ) );
         break;
      default : {
         mc2log << warn << "InfoProcessor: Packet with subtype = " <<
            (uint32) subType << " not supported."
                << endl;
      }
      break;
   }

   return (answerPacket);
}


int
InfoProcessor::getCurrentStatus()
{
   return (1);
}

void
InfoProcessor::updateUserRights(UserRightsMapInfo& userRights)
{
   userRights.filterAllRights();
}








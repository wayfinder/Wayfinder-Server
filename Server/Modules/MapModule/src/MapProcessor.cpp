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

#include "MapProcessor.h"
#include "MapHashTable.h"
#include "StringUtility.h"
#include "ExpandRoutePacket.h"

#include "MapGenerator.h"

#include "ItemNamesPacket.h"
#include "ItemInfoPacket.h"
#include "GfxFeatureMapPacket.h"
#include "CoordinateOnItemPacket.h"
#include "LoadMapPacket.h"

#include "Item.h"
#include "Node.h"
#include "StreetSegmentItem.h"
#include "CategoryItem.h"
#include "BuiltUpAreaItem.h"
#include "StreetItem.h"
#include "ItemInfoFilter.h"

#include "MapPacket.h"
#include "ServerProximityPackets.h"
#include "CoordinatePacket.h"
#include "StreetSegmentItemPacket.h"
#include "DeleteMapPacket.h"
#include "ExpandItemPacket.h"

#include "RouteExpandItemPacket.h"
#include "IDTranslationPacket.h"
#include "MatchInfoPacket.h"
#include "TrafficDataTypes.h"

#include "Processor.h"
#include "MapHandler.h"
#include "Math.h"   
#include "ExpandRouteProcessor.h"
#include "GfxFeatureMapProcessor.h"
#include "GfxTileFeatureMapProcessor.h"
#include "GfxConstants.h"
#include "GfxUtility.h"
#include "GfxData.h"

#include "Packet.h"
#include "LoadMapPacket.h"

#include "QueueStartPacket.h"
#include "ExpandPacket.h"
#include "ExpandRequestData.h"
#include "SearchReplyData.h"
#include "InfoCoordinatePacket.h"

#include "GenericMap.h"
#include "OverviewMap.h"

#include "Properties.h"
#include "ExternalConnections.h"

#include "UserRightsMapInfo.h"

#include "NetUtility.h"

#include "MapSettings.h"
#include "Component.h"
#include "CategoryTranslationLoader.h"
#include "MapBits.h"

#include "DeleteHelpers.h"
#include "NodeBits.h"

#include <memory>

struct ssiNotice_t {
   Item* m_item;
   uint32 m_score;
   uint64 m_dist;
   uint16 m_offset;
   uint16 m_angle;
   int16 m_angleDiff;
};

struct LessNotice : 
   public binary_function<ssiNotice_t, ssiNotice_t, bool> {
      bool operator()(ssiNotice_t x, ssiNotice_t y) { 
         return (x.m_score < y.m_score); 
      }
};
namespace {
class TranslationListLoader: public CategoryTranslationLoader {
public:
   TranslationListLoader( MapProcessor::CategoryTranslationMap& translations ):
      m_translations( translations ) {
   }

protected:
   void newTranslationValue( uint32 catID,
                             LangTypes::language_t lang, 
                             const MC2String& stringValue ) {
      m_translations[ lang ][ catID ] = stringValue;
   }

   MapProcessor::CategoryTranslationMap& m_translations;
};

}

MapProcessor::MapProcessor(MapSafeVector* loadedMaps,
                           const char* packetFile)
      : MapHandlingProcessor(loadedMaps),
        m_getPOIInformation(Properties::getProperty("POI_SQL_DATABASE"),
                            Properties::getProperty("POI_SQL_HOST"),
                            Properties::getProperty("POI_SQL_USER"),
                            Properties::getProperty("POI_SQL_PASSWORD"))
{
   MC2String categoryFilename = Properties::getProperty( "CATEGORY_TREE_FILE",
                                                         "" );

   if ( ! ::TranslationListLoader( m_poiCategories ).
        loadTranslations( categoryFilename )) {
      mc2log << warn << "MapProcessor::MapProcessor, could not load category file" << endl;
      throw 
         ComponentException( MC2String("Could not load poi categories from: ") +
                             categoryFilename );
   }

   setPacketFilename( packetFile ? packetFile : "" );
   m_mapHandler.reset( new MapHandler() );
   m_routeExpander.reset( new ExpandRouteProcessor() );
   
   m_nextFreePort = FIRST_PORT_NUMBER;
}

MapProcessor::~MapProcessor()
{
}

Packet*
MapProcessor::handleRequestPacket( const RequestPacket& pack,
                                   char* packetInfo)
{

   // The Processor should have checked this and the vectors
   // should be in synch. If the map is not loaded, an ackpacket
   // should be in answerPacket
   MC2_ASSERT( ( MAX_UINT32 == pack.getMapID() ) ||
               ( NULL != m_mapHandler->getMap( pack.getMapID() ) ) );
   

   Packet *answerPacket = NULL;

   uint16 subType = pack.getSubType();
   mc2dbg2 << "MapProcessor recieved packet with subtype = " 
           << (uint32) subType << endl ;
   DEBUG8(p->dump());
      
   switch (subType) {
   case Packet::PACKETTYPE_MAPREQUEST : {
      const MapRequestPacket* mp = static_cast<const MapRequestPacket*>( &pack );
      mc2dbg2 << "MapProcessor, processing MapRequestPacket ("
              << mp->getMapTypeAsString()
              << ")" << endl;
      uint32 mapID = mp->getMapID();
      MapGenerator* mapGenerator = NULL;
      uint16 portNumber = 0;
            
      // First check that we have loaded the map with given ID
      StringTable::stringCode packetStatus = StringTable::OK;
      if (m_mapHandler->getLockedMap(mapID) == NULL) {
         packetStatus = StringTable::MAPNOTFOUND;
      } else {              
         vector<uint32> overviewMaps;
         mp->getOverviewMaps(overviewMaps);
         mapGenerator = MapGenerator::createGenerator( this,
                                                       m_mapHandler.get(),
                                                       mp->getMapType(),
                                                       mapID,
                                                       m_nextFreePort,
                                                       mp->getZoomlevel(),
                                                       overviewMaps );
               
         portNumber = mapGenerator->getPortInUse();
               
         m_nextFreePort = portNumber;
         mapGenerator->start();
      }
            
      GenericMap* curMap = m_mapHandler->getMap(mapID);
      uint32 mapVersion = curMap->getCreationTime();
      // Use the same version for all generators.
      uint32 generatorVersion = MapGenerator::generatorVersion;
            
      answerPacket = new MapReplyPacket( mp,
                                         NetUtility::getLocalIP(),
                                         portNumber,
                                         mapVersion,
                                         generatorVersion);
      (static_cast<ReplyPacket*>(answerPacket))
         ->setStatus(packetStatus);
            
      mc2dbg2 << "sending MapReplyPacket to " 
              << answerPacket->getOriginIP()
              << "." << answerPacket->getOriginPort() 
              << ", listening on port " << portNumber 
              << " packet status = " 
              << StringTable::getString(packetStatus, 
                                        StringTable::ENGLISH) << endl;
            
      if (m_nextFreePort < LAST_PORT_NUMBER) 
         m_nextFreePort++;   // Try with the next port nextTime.
      else
         m_nextFreePort = FIRST_PORT_NUMBER;
   }
      break;
        
   case Packet::PACKETTYPE_ITEM_NAMES_REQUEST :
      {
         const ItemNamesRequestPacket* rp = static_cast<const ItemNamesRequestPacket*>
            (&pack);
         uint32 mapID = rp->getMapID();
         GenericMap* theMap = m_mapHandler->getMap(mapID);
         mc2dbg2 << "   mapID=" << mapID << endl;
         answerPacket = new ItemNamesReplyPacket(rp);
         if (theMap == NULL) {
            static_cast<ItemNamesReplyPacket*>
               (answerPacket)->setStatus(StringTable::NOTFOUND);
         } else {
            mc2dbg4 << "   To expand " << rp->getNbrItems()
                    << " items" << endl;
            for (uint32 i=0; i<rp->getNbrItems(); i++) {
               uint32 curItemID = rp->getItem(i);
               //               const char* itemName = theMap->getItemName(curItemID);
               const char* itemName =
                  theMap->getBestItemName(curItemID & 0x7fffffff,
                                          rp->getPreferedLanguage());
               if (itemName != NULL) {
                  mc2dbg4 << "      Adding name " << itemName << " to ID="
                          << hex << curItemID << dec << endl;
                  static_cast<ItemNamesReplyPacket*>
                     (answerPacket)->addName(curItemID, itemName);
               } else {
                  mc2dbg4 << "      Adding name \"\" to ID="
                          << hex << curItemID << dec
                          << endl;
                  static_cast<ItemNamesReplyPacket*>
                     (answerPacket)->addName(curItemID, "");
               }
               mc2dbg4 << "   All names added" << endl;
            }
         }
         const char* langString =
            LangTypes::getLanguageAsString(rp->getPreferedLanguage(),
                                           false);
         sprintf(packetInfo, "[%s]", langString);
      }
   break;
            
   case Packet::PACKETTYPE_ITEMINFO_REQUEST : {
      answerPacket = processItemInfoPacket(
                    static_cast<const ItemInfoRequestPacket*>( &pack ), packetInfo );
   } break;

   case Packet::PACKETTYPE_GET_ITEM_INFO_REQUEST : {
      answerPacket = processGetItemInfoPacket(
         static_cast<const GetItemInfoRequestPacket*>( &pack ), packetInfo );
   } break;
         
   case Packet::PACKETTYPE_COORDINATEREQUEST : {
      answerPacket = processCoordinateRequestPacket(
                    static_cast<const CoordinateRequestPacket*>(&pack));

      mc2dbg2 << "   MapProcessor COORDINATEREPLY-packet created" 
              << "with status = "
              << StringTable::getString(
                                        StringTable::stringCode(
                         static_cast<const ReplyPacket*>(answerPacket)->getStatus()),
                                        StringTable::ENGLISH) << endl;
   }
      break;

   case Packet::PACKETTYPE_STREET_SEGMENT_ITEMREQUEST : {
      answerPacket = processStreetSegmentItemRequestPacket(
                      static_cast< const StreetSegmentItemRequestPacket*>(&pack));

      mc2dbg4 << "   MapProcessor STREETSEGMENTITEMREPLY-packet created" 
              << "with status = "
              << StringTable::getString(
                                        StringTable::stringCode(
                        static_cast<const ReplyPacket*>(answerPacket)->getStatus()),
                                        StringTable::ENGLISH) << endl;
   }
      break;

   case Packet::PACKETTYPE_EXPANDITEMREQUEST : {
      answerPacket = processExpandItemRequestPacket(
                     static_cast<const ExpandItemRequestPacket*>( &pack ));
   }
      break;

   case Packet::PACKETTYPE_ROUTEEXPANDITEMREQUEST : {
      answerPacket = processRouteExpandItemRequestPacket(
                     static_cast<const RouteExpandItemRequestPacket*>( &pack ));
   }
      break;

   case Packet::PACKETTYPE_COORDINATEONITEMREQUEST : {
      answerPacket = processCoordinateOnItemRequestPacket(
                     static_cast<const CoordinateOnItemRequestPacket*>( &pack ));
      if (answerPacket == NULL) {
         MC2ERROR("   answerPacket == NULL");
      }
   }
      break;

   case Packet::PACKETTYPE_EXPANDROUTEREQUEST : {
      // Expand the route in the request to  
      // 1) StringCodes + strings
      // 2) several gfxRoutes (itemID:s)         
      answerPacket = m_routeExpander->processExpandRouteRequestPacket(
                     static_cast<const ExpandRouteRequestPacket*>( &pack ),
                     m_mapHandler->getMap( pack.getMapID() ));
   }
      break;

   case Packet::PACKETTYPE_COVEREDIDSREQUESTPACKET : {
      answerPacket = processCoveredIDsRequestPacket( 
                     static_cast<const CoveredIDsRequestPacket*>( &pack ) );

      mc2dbg4 << "    CoveredIDsReply created" << endl;
   }
      break;
      
   case Packet::PACKETTYPE_TRAFFICPOINTREQUESTPACKET : {
      answerPacket = processTrafficPointRequestPacket( 
                     static_cast<const TrafficPointRequestPacket*>( &pack ) );

      mc2dbg4 << "    TrafficPointReply created" << endl;
   }
      break;
      
   case Packet::PACKETTYPE_GFXFEATUREMAPREQUEST : {
      const GfxFeatureMapRequestPacket* gfxPack = 
         static_cast< const GfxFeatureMapRequestPacket*>( &pack );

      // Add language too
      LangTypes::language_t langType = gfxPack->getLanguage();
      const char* langString =
         LangTypes::getLanguageAsString(langType,
                                        false);
      packetInfo += sprintf(packetInfo, "[%s]", langString);
      uint32 theMapID = gfxPack->getMapID();
      if ( gfxPack->extractForTileMaps() ) {
         if ( packetInfo != NULL ) {
            // Add a printout so that we know what it is.
            int res = sprintf(packetInfo, "%s", "{tile}");            
            packetInfo += res;

         }


         GfxTileFeatureMapProcessor* proc =
            m_mapHandler->getTileProcessor( theMapID );

         if ( proc != NULL ) {
            MapSettings *settings = NULL;
            answerPacket = proc->generateGfxFeatureMap( gfxPack, &settings );

            if ( packetInfo != NULL ) {
               packetInfo += sprintf( packetInfo, "%s", 
                                      settings->getTileMapParamStr().c_str() );
            }
         }

      } else {
         GfxFeatureMapProcessor* proc =
            m_mapHandler->getFeatureProcessor( theMapID );
         if ( proc != NULL ) {
            answerPacket = proc->generateGfxFeatureMap( gfxPack );
         }
      }
      mc2dbg4 << "    gfxFeatureMapReply created" << endl;
   }
      break;
         
   case Packet::PACKETTYPE_IDTRANSLATIONREQUEST : {
      answerPacket = processIDTranslationRequestPacket(
                       static_cast<const IDTranslationRequestPacket*>( &pack ));
      mc2dbg4 << "    IDTranslationReply created" << endl;
   }
      break;

   case Packet::PACKETTYPE_MATCHINFOREQUEST:
      answerPacket = processMatchInfoRequest(
                       static_cast<const MatchInfoRequestPacket*>( &pack ));
      break;

   case Packet::PACKETTYPE_QUEUESTARTREQUEST:
      answerPacket = processQueueStartPacket(
                      static_cast<const QueueStartRequestPacket*>( &pack ));
      mc2dbg4 << "    QueueStartReply created" << endl;
      break;
	 
   case Packet::PACKETTYPE_EXPAND_REQUEST:
      answerPacket = processExpandRequest(
                      static_cast<const ExpandRequestPacket*>( &pack ));
      break;

   case Packet::PACKETTYPE_INFOCOORDINATEREQUEST:
      answerPacket = processInfoCoordinateRequest(
                       static_cast<const InfoCoordinateRequestPacket*>( &pack ));
      break;

   default : {
      mc2log << warn << "MapProcessor: Packet type not supported!" << endl
             << "   Packettype = " << (uint32) subType 
             << ", originIP=" << pack.getOriginIP()
             << ", originPort=" << pack.getOriginPort()
             << endl; 
   }
      break;
   }

   mc2dbg2 << "MapProcessor: Processing done" << endl;
   return answerPacket;
}

int
MapProcessor::getCurrentStatus()
{
   return (1);
}

void
MapProcessor::addCategories( ItemInfoData& reply, 
                             const Item& item,
                             const GenericMap& curMap,
                             LangTypes::language_t lang ) const {
   // find string map
   CategoryTranslationMap::const_iterator transIt = 
      m_poiCategories.find( lang );
   if ( transIt == m_poiCategories.end() ) {
      // ok, lets try with english
      transIt = m_poiCategories.find( LangTypes::english );
      if ( transIt == m_poiCategories.end() ) {
         // this was strange, no english translation either? 
         // nothing to add...
         mc2log << "No translation" << endl;
         return;
      }
   }
   const CategoryStrings& strings = transIt->second;

   // fetch categories
   GenericMap::Categories categories;
   curMap.getCategories( item, categories );
   if ( categories.empty() ) {
      mc2log << "Categories are empty for this item." << endl;
      // no categories for this item.
      return;
   }

   // set all categories
   const char* str = 
      StringTable::getString( StringTable::CATEGORY, lang );

   GenericMap::Categories::const_iterator catIt = categories.begin();
   GenericMap::Categories::const_iterator catItEnd = categories.end();
   for ( ; catIt != catItEnd; ++catIt ) {

      CategoryStrings::const_iterator stringIt = strings.find( *catIt );
      if ( stringIt == strings.end() ) {
         // no string to add, this is strange though.
         // try next category id.
         mc2dbg << "no strings to add." << endl;
         continue;
      }
      mc2dbg4 << "Adding category name: " << stringIt->second << endl;
      // add string
      reply.addInfoEntry( str, stringIt->second.c_str() );
   }
}

void
MapProcessor::addOneItemInfo( ItemInfoDataCont& reply,
                              const Item* infoItem,
                              LangTypes::language_t langType,
                              const GenericMap& curMap,
                              const UserRightsMapInfo& rights,
                              ItemInfoEnums::InfoTypeFilter infoFilterLevel )
{
   // Check the rights. Is used further down.
   const bool allowedByRights =
      curMap.itemAllowedByUserRights( infoItem->getID(), rights );
   
   if ( ! allowedByRights ) {
      mc2dbg << "[MP]: Item "
             << MC2CITE( curMap.getBestItemName( infoItem,
                                                 LangTypes::english ) )
             << " not allowed by user rights " << endl;
   }

   // Extract languages
   StringTable::languageCode langCode = 
      ItemTypes::getLanguageTypeAsLanguageCode( langType );
   
   // Set to ENGLISH if invalid language
   if ( langCode == StringTable::SMSISH_ENG ) {
      langCode = StringTable::ENGLISH;
   }
   
   // Get the "best" name to add to the packet
   const char* bestItemName = curMap.getBestItemName( infoItem, langType );
   
   mc2dbg4 << "bestItemName=" << bestItemName << endl;
   
   // Handle the different item-types.
   switch ( infoItem->getItemType() ) {
      case ItemTypes::pointOfInterestItem : {
         const PointOfInterestItem* poi = 
            static_cast<const PointOfInterestItem*>(infoItem);
         // Calculate coordinates of this poi
         int32 poiLatitude, poiLongitude;
         if (!curMap.getItemCoordinates( poi->getStreetSegmentItemID(), 
                                         poi->getOffsetOnStreet(),
                                         poiLatitude, 
                                         poiLongitude)) {
            poiLatitude = MAX_INT32;
            poiLongitude = MAX_INT32;
         }
         
         // Add data to packet
         StringTable::languageCode tmpLang = langCode;
         GenericMap::Categories categories;
         curMap.getCategories( *poi, categories );

         const char* typeName = StringTable::getString(
            ItemTypes::getPOIStringCode(
               poi->getPointOfInterestType()),
            tmpLang);
         reply.push_back( ItemInfoData( typeName,
                                        bestItemName,
                                        poi->getItemType(),
                                        poi->getPointOfInterestType(),
                                        MC2Coordinate( poiLatitude,
                                                       poiLongitude ),
                                        categories ) );
         mc2dbg4 << "New POI added, \"" << bestItemName 
                 << "\", type=" << typeName << endl;

         Item* poiSSI =
            curMap.itemLookup( poi->getStreetSegmentItemID() );
               
         const char* addrString = NULL;
         if ( poiSSI != NULL && poiSSI->getNbrNames() > 0 ) {
            addrString = curMap.getItemName( poiSSI,
                                             langType,
                                             ItemTypes::invalidName);
         }

         if ( allowedByRights ) {
            //const uint32 poiSQLID = 644352;  // Itinerary
            const uint32 poiID = poi->getWASPID();
            mc2dbg4 << "Getting info data for POI " << poiID 
                    << endl;
            m_getPOIInformation.addToPacket( curMap,
                                             poi, 
                                             reply.back(),
                                             langType,
                                             addrString,
                                             infoFilterLevel );

            if ( ItemInfoFilter::includeCategoryItem( infoFilterLevel ) ) {
               // add "Category:" field.
               addCategories( reply.back(), *poi, curMap, langType );
            }
               
         } else {
            MC2String neededRight =
               curMap.getNameOfNeededRight( infoItem->getID(),
                                            langType );
            const bool norights = rights.empty();
            if ( neededRight != "" || norights ) {
               const char* rights_key = StringTable::getString(
                  StringTable::RIGHTS_AVAILABILITY_KEY, langCode );
               const char* rights_prefix = StringTable::getString(
                  StringTable::RIGHTS_AVAILABILITY_PREFIX, langCode );
               const char* rights_postfix = StringTable::getString(
                  StringTable::RIGHTS_AVAILABILITY_POSTFIX, langCode );
               const char* unallowed_map = StringTable::getString(
                  StringTable::OUTSIDE_ALLOWED_AREA, langCode);
               if ( rights_key && norights && unallowed_map) {
                  reply.back().addInfoEntry( rights_key,
                                            unallowed_map );
               } else if ( rights_key && rights_prefix && rights_postfix ) {
                  MC2String toPrint = MC2String("") + rights_prefix +
                     neededRight + rights_postfix;
                  reply.back().addInfoEntry( rights_key,
                                            toPrint.c_str() );
               } else {
                  mc2log << error
                         << "[MP]: Couldn't get RIGHTS_AVAILABILITY "
                         << "string from StringTable" << endl;
               }
            }
         }

      } break;

      case ItemTypes::streetSegmentItem:
      case ItemTypes::streetItem:
      { 
         int32 lat = MAX_INT32;
         int32 lon = MAX_INT32;
         if ( !curMap.getItemCoordinates(
                 infoItem->getID(),
                 0x7fff, lat, lon ) ) 
         {
            lat = MAX_INT32;
            lon = MAX_INT32;  
         }
         // Add data to packet
         const char* typeName = StringTable::getString(
            ItemTypes::getItemTypeSC( 
               infoItem->getItemType() ), langCode );
         
         reply.push_back( ItemInfoData( typeName, bestItemName, 
                                        infoItem->getItemType(),
                                        0, MC2Coordinate( lat, lon ) ) );
      } break;

      default :
         // Nothing extra to add for this item...
         const char* typeName = StringTable::getString(
            ItemTypes::getItemTypeSC(infoItem->getItemType()),
            langCode);
         MC2Coordinate coord = curMap.getOneGoodCoordinate( infoItem );

         reply.push_back( ItemInfoData( typeName, bestItemName,
                                        infoItem->getItemType(),
                                        0, coord ) );
         break;
   } // End switch

   if ( ItemInfoFilter::includeNameItems( infoFilterLevel ) ) {
      // Only add one official name!
      const char* onName = curMap.getBestItemName(
         infoItem, ItemTypes::officialName, langType);
      if ( (onName != NULL) && (onName != bestItemName) ) {
         const char* str = StringTable::getString(
            StringTable::NAME, langCode);
         reply.back().addInfoEntry( str, onName );
      }
      
      // Only add one alternative name!
      const char* anName = curMap.getBestItemName(
         infoItem, ItemTypes::alternativeName, langType);
      if ( (anName != NULL) && (anName != bestItemName) ) {
         const char* str = StringTable::getString(
            StringTable::NAME, langCode);
         reply.back().addInfoEntry( str, anName );
      }
      
      // Add all names to infoItem (except from bestItemName)
      for (uint32 i=0; i<infoItem->getNbrNames(); ++i) {
         
         const char* curName =
            curMap.getName(infoItem->getStringIndex(i));
         ItemTypes::name_t nameType = infoItem->getNameType( i );
         
         // add name but exclude synonym, alternative and official
         // because they have already been added on the lines above.
         if ( curName != bestItemName &&
              nameType != ItemTypes::synonymName &&
              nameType != ItemTypes::alternativeName &&
              nameType != ItemTypes::officialName ) {
            // Add to packet
            
            // compose nameValue string as: "name (language)"
            MC2String nameValue( curName );
            // add languages for all items except
            // for road number and invalid names
            LangTypes::language_t lang = infoItem->getNameLanguage( i );
            if ( lang != LangTypes::invalidLanguage ) {
               const char* langStr = LangTypes::getLanguageAsString( lang ); 
               nameValue += MC2String(" (") + langStr + ")";
            }
            
            // exitNumber for SubWayLines means "line" (color or 
            // number)
            
            if ( nameType == ItemTypes::exitNumber &&
                 infoItem->getItemType() == ItemTypes::pointOfInterestItem &&
                 (static_cast<const PointOfInterestItem*>(infoItem)
                  ->getPointOfInterestType () ==
                  ItemTypes::commuterRailStation) ) {
               const char* str = StringTable::
                  getString( StringTable::SUBWAYLINE, langCode);
               reply.back().addInfoEntry( str, nameValue.c_str() );
            } else {
               const char* str = StringTable::
                  getString( StringTable::NAME, langCode);
               reply.back().addInfoEntry( str, nameValue.c_str() );
            }
            
            mc2dbg << "[MapProcessor] Added name to item:" << nameValue << endl;
         }
      }
   }
}

namespace {

/**
 * Fetch information about a poi close to a street.
 * @param curMap
 * @param curItem street or street item.
 * @param poiCoord Coordinate of the POI.
 * @param poiName Name of the POI.
 * @param language Language to use for info items.
 * @param infoItems Will be filled with infos about the poi.
 */
void fetchPOIInfo( const GenericMap* curMap, const Item* curItem,
                   const MC2Coordinate& poiCoord,
                   const char* poiName,
                   const LangTypes::language_t language,
                   Vector& infoItems ) {
   ItemTypes::itemType curType = curItem->getItemType();
   uint32 radius = 5;
   if ( poiName[ 0 ] != '\0' ) {
      // Have name to match can afford bigger radius
      radius = 500;
   }

   // Get the ID of the SSI's that should be used.
   set<uint32> ssiIDs;
   if (curType == ItemTypes::streetSegmentItem) {
      ssiIDs.insert(curItem->getID());
   } else {
      const StreetItem* si = static_cast< const StreetItem*>(curItem);
      for (uint32 i=0; i < si->getNbrItemsInGroup(); i++) {
         const Item* tmpItem = curMap->itemLookup( si->getItemNumber(i));
         if ( (tmpItem != NULL) &&
              (tmpItem->getItemType() ==
               ItemTypes::streetSegmentItem))
            ssiIDs.insert(tmpItem->getID());
      }
   }

   // Calculate offset, if coordinates in packet are set

   if ( poiCoord.lat != MAX_INT32 ) {
      // Add all SSI's that are really close to (lat,lon)
      // to handle POIs on the nodes
      set<ItemTypes::itemType> allowedTypes;
      allowedTypes.insert(ItemTypes::streetSegmentItem);
      set<uint32> itemSet;
      curMap->getIDsWithinRadiusMeter(itemSet,
                                      poiCoord, radius,
                                      allowedTypes);
      for (set<uint32>::const_iterator it = itemSet.begin();
           it != itemSet.end();
           ++it ) {
         ssiIDs.insert((*it));
      }
   }


   // Get the POI's on the SSI's in ssiIDs
   const uint32 z = uint32(ItemTypes::pointOfInterestItem);
   for (uint32 i=0; i<curMap->getNbrItemsWithZoom(z); i++) {
      PointOfInterestItem* poi = item_cast
         <PointOfInterestItem*>(curMap->getItem(z, i));
      if ( poi == NULL ) {
         continue;
      }
      set<uint32>::const_iterator findIt =
         ssiIDs.find( poi->getStreetSegmentItemID() );
      if ( findIt != ssiIDs.end() ) {
         uint32 curSSI = *findIt;

         // Make sure that the coord is within
         // the specified radius.
         MC2Coordinate ssiOffsetCoord;
         bool coordOK = true;
         if ( poiCoord.lat != MAX_INT32 &&
              curMap->getItemCoordinates( curSSI,
                                          poi->getOffsetOnStreet(),
                                          ssiOffsetCoord.lat,
                                          ssiOffsetCoord.lon ) ) {
            coordOK =
               GfxUtility::squareP2Pdistance_linear( poiCoord,
                                                     ssiOffsetCoord ) <=
               float64(radius*radius);
         }

         if ( coordOK ) {
            bool add = poiName[ 0 ] == '\0';
            for ( uint32 j = 0 ; j < poi->getNbrNames() &&
                     !add; ++j ) {
               if ( poi->getNameType( j ) ==
                    ItemTypes::synonymName ) {
                  continue;
               }
               const char* itemName =
                  curMap->getName( poi->getStringIndex( j ) );
               if ( itemName == NULL ) {
                  continue;
               }
               if ( StringUtility::
                    strcasecmp( poiName, itemName ) == 0 ) {
                  // Found poi, add to infoItems
                  add = true;
               }
            }
            if ( add ) {
               infoItems.addLast( poi->getID() );
            }
         }
      }
   } // End for all pois in map

   // Look for POI:s with GfxData, that are located close
   // to the given coordinate
   if ( poiCoord.lat != MAX_INT32) {
      MapHashTable* ht = curMap->getHashTable();
      ht->clearAllowedItemTypes();
      ht->addAllowedItemType(ItemTypes::pointOfInterestItem);
      bool shouldKill = false;
      Vector* closeIDs = ht->getAllWithinRadius_meter( poiCoord.lon,
                                                       poiCoord.lat,
                                                       10, shouldKill);
      if (closeIDs != NULL) {
         for (uint32 i=0; i<closeIDs->getSize(); i++) {
            const char* itemName =
               curMap->getItemName( closeIDs->getElementAt( i ),
                                    language, ItemTypes::invalidName);
            if ( poiName[ 0 ] == '\0' ||
                 (itemName == NULL ||
                  StringUtility::strcasecmp( poiName, itemName ) == 0) ) {
               infoItems.addLastIfUnique( closeIDs->getElementAt( i ) );
            }
         }

         if ( shouldKill ) {
            delete closeIDs;
         }
      }
   }

}

/**
 * Fetches street info near a poi item.
 * @param curMap
 * @param curItem POI item.
 * @param reqType street or street segment type.
 * @param infoItems Will be filled with info about a street.
 */
void fetchStreetInfo( const GenericMap* curMap, const Item* curItem,
                      const ItemTypes::itemType reqType,
                      Vector& infoItems ) {
   const StreetSegmentItem* ssi = item_cast<const StreetSegmentItem*>
      (curMap->itemLookup(static_cast<const PointOfInterestItem*>
                          (curItem)->getStreetSegmentItemID()));

   // Add to infoItems
   if (ssi != NULL) {
      if (reqType == ItemTypes::streetSegmentItem) {
         infoItems.addLast(ssi->getID());
      } else if (ssi->partOfStreet()) {
         for (uint32 k=0; k<ssi->getNbrGroups(); k++) {
            Item* tmpItem = curMap->itemLookup(ssi->getGroup(k));
            if ( (tmpItem != NULL) && (tmpItem->getItemType()
                                       == ItemTypes::streetItem))
               infoItems.addLast(tmpItem->getID());
         }
      }
   }

}

} // anonymous namespace 


/**
 * Get additional information about an item.
 *
 * @return The number of items added, the poi's on the street.
 */
uint32 
MapProcessor::getItemInfo( ItemTypes::itemType reqType, Item* curItem,
                           uint32 curItemID,
                           GenericMap* curMap, const MC2Coordinate& poiCoord, 
                           const char* poiName, LangTypes::language_t language,
                           const UserRightsMapInfo& rights,
                           ItemInfoEnums::InfoTypeFilter infoFilterLevel,
                           ItemInfoDataCont& reply ) {
      
   // A vector where to store the ID's of the items to return 
   // information about.
   Vector infoItems(64);
   
   // Check what data should be returned
   ItemTypes::itemType curType = curItem->getItemType();
   mc2dbg4 << "To send iteminfo for " << curItemID
           << ", reqType="
           << StringTable::getString(
              ItemTypes::getItemTypeSC(reqType), 
              StringTable::ENGLISH) << ", curType="
           << StringTable::getString(
              ItemTypes::getItemTypeSC(curType), 
              StringTable::ENGLISH) << "" << endl;
   
   if ( ( (curType == ItemTypes::streetSegmentItem) ||
          (curType == ItemTypes::streetItem) ) &&
        (reqType == ItemTypes::pointOfInterestItem) ) {
      // Got a street/SSI and to return information about POI
      ::fetchPOIInfo( curMap, curItem,
                      poiCoord,
                      poiName,
                      language,
                      infoItems );

   } else if ( (curType == ItemTypes::pointOfInterestItem) &&
               ( (reqType == ItemTypes::streetSegmentItem) ||
                 (reqType == ItemTypes::streetItem) ) ) {
      // Got POI, return information about street/SSI
      ::fetchStreetInfo( curMap, curItem, reqType,
                         infoItems );
   } else if (curType == reqType) {
      // Got the same type that to return information about.
      const char* itemName = curMap->getItemName( 
         curItem->getID(), language, ItemTypes::invalidName );
      if ( poiName[ 0 ] == '\0' ||
           StringUtility::strcasecmp( poiName, itemName ) == 0 ) {
         infoItems.addLast( curItem->getID() );
      }
   } else {
      // Strange, unhandled case
      mc2log << warn << here << " Unhandled mix of reqType ("
             << StringTable::getString( ItemTypes::getItemTypeSC(reqType),
                                        StringTable::ENGLISH)
             << ") and curType ("
             << StringTable::getString( ItemTypes::getItemTypeSC(curType),
                                        StringTable::ENGLISH) << ")" << endl;
      infoItems.addLast(curItem->getID());
   }

   // Sort the info items by distance from the point.
   // This is also where the non-allowed items are removed.
   multimap<float64, uint32> infoItemsByDistance;
   for ( uint32 i = 0; i < infoItems.getSize(); ++i ) {
      uint32 itemID = infoItems.getElementAt( i );
 
      const Item* item = curMap->itemLookup( itemID );
      MC2_ASSERT( item != NULL );
      MC2Coordinate coord;
      curMap->getOneGoodCoordinate( coord, item );
      float64 sqDistance = 
         GfxUtility::squareP2Pdistance_linear( coord.lat, coord.lon,
                                               poiCoord.lat, poiCoord.lon ); 
      infoItemsByDistance.insert( make_pair( sqDistance, itemID ) );
   }


   // Add data about the items in infoItems to the packet
   for ( multimap<float64, uint32>::const_iterator it = 
            infoItemsByDistance.begin(); it != infoItemsByDistance.end();
         ++it ) {
     
      Item* infoItem = 
         curMap->itemLookup( (*it).second );
            
      MC2_ASSERT(infoItem != NULL);

      // Add info to the packet.
      addOneItemInfo( reply, infoItem, language, *curMap, rights, infoFilterLevel );

   } // End for all infoItems
         

   return infoItems.size();
}


ItemInfoReplyPacket*
MapProcessor::processItemInfoPacket( const ItemInfoRequestPacket* inPacket,
                                     char*& packetInfo )
{
   // It seems like the processing is like this:
   // If there is a streetsegment or street in the packet
   // the id of that will be added to the vector of items
   // to be expanded.
   // If there is a coordinate as well, all ss-items within a radius
   // are added too. If there is a name, the radius will be bigger.
   // The function should really use the methods in GenericMap
   // instead of using the hashtable.   

   uint32 startTime = TimeUtility::getCurrentTime();
   // Cast the in-packet and create the reply   
   ItemInfoReplyPacket* replyPacket =
      new ItemInfoReplyPacket(inPacket);
   replyPacket->setStatus(StringTable::NOTOK);
   ItemInfoDataCont replyData;

   // There seems to be some confusion about the
   // type of the language - so I will print it here.
   {
      LangTypes::language_t langType = 
         inPacket->getPreferredLanguage();
      const char* langString =
         LangTypes::getLanguageAsString(langType,
                                        false);
      sprintf(packetInfo, "[%s]", langString);
   }
   
   // Get some data from the packet
   uint32 mapID = inPacket->getMapID();
   uint32 curItemID = inPacket->getItemID();
   UserRightsMapInfo rights;
   const char* poiName = inPacket->getPOINameAndRights( rights );
   LangTypes::language_t language = inPacket->getPreferredLanguage();
   
   GenericMap* curMap = m_mapHandler->getMap(mapID);
   
   if (curMap == NULL) {
      // at least add the coordinates
      replyData.push_back( ItemInfoData( "", "", 
                                         ItemTypes::nullItem,
                                         0,
                                         MC2Coordinate( 
                                            inPacket->getPOILat(),
                                            inPacket->getPOILon() ) ) );
      replyPacket->setInfo( replyData );
      replyPacket->setStatus( StringTable::MAPNOTFOUND );
      return replyPacket;
   }
   
   // curMap OK
   Item* curItem = curMap->itemLookup(curItemID);
   if (curItem == NULL) {
      // at least add the coordinates
      replyData.push_back( ItemInfoData( "", "", 
                                         ItemTypes::nullItem,
                                         0,
                                         MC2Coordinate( 
                                            inPacket->getPOILat(),
                                            inPacket->getPOILon() ) ) );
      replyPacket->setInfo( replyData );
      replyPacket->setStatus( StringTable::NOTFOUND );
      return replyPacket;
   }
    
   // curItem OK
   
   // Get information
   uint32 nbrInfoItems = getItemInfo( 
      inPacket->getItemType(), curItem, curItemID, curMap,
      MC2Coordinate( inPacket->getPOILat(), inPacket->getPOILon() ),
      poiName, language, rights, ItemInfoEnums::All, replyData );

   // Set the status of the packet
   if ( nbrInfoItems > 0 ) {
      replyPacket->setStatus( StringTable::OK );
   }

   if ( replyData.size() == 0 ) {
      // Nothing extra to add for this item...
      const char* typeName = 
         StringTable::
         getString( ItemTypes::
                    getItemTypeSC( curItem->getItemType() ),
                    ItemTypes::
                    getLanguageTypeAsLanguageCode(inPacket->
                                                  getPreferredLanguage() ));
      replyData.push_back( ItemInfoData( typeName, "", 
                                         curItem->getItemType(),
                                         0,
                                         MC2Coordinate( 
                                            inPacket->getPOILat(),
                                            inPacket->getPOILon() ) ) );
      
   }
   // Fill the packet with the results
   replyPacket->setInfo( replyData );

   mc2dbg << "[MP] ItemInfo calculated in "
          << (TimeUtility::getCurrentTime()-startTime) << "ms"
          << endl;
   
   return replyPacket;
}

ItemInfoReplyPacket* 
MapProcessor::processGetItemInfoPacket( 
   const GetItemInfoRequestPacket* p, char*& packetInfo ) {
   ItemInfoReplyPacket* replyPacket = new ItemInfoReplyPacket( p );
   ItemInfoDataCont replyData;

   uint32 mapID = p->getMapID();
   LangTypes::language_t language = p->getLanguage();
   UserRightsMapInfo rights;
   p->getUserRightsMapInfo( rights );
   
   GenericMap* curMap = m_mapHandler->getMap( mapID );
   
   if ( curMap == NULL ) {
      replyPacket->setStatus( StringTable::MAPNOTFOUND );
      return replyPacket;
   }
   // curMap OK

   // For all itemIDs
   GetItemInfoRequestPacket::itemIDVector itemIDs;
   p->getItemIDs( itemIDs );

   for ( size_t i = 0, end = itemIDs.size() ; i < end ; ++i ) {
      uint32 curItemID = itemIDs[ i ];
      Item* curItem = curMap->itemLookup( curItemID );
      if ( curItem == NULL ) {
         replyPacket->setStatus( StringTable::NOTFOUND );
         return replyPacket;
      }

      /*uint32 nbrInfoItems =*/ getItemInfo( 
         curItem->getItemType(), curItem, curItemID, curMap,
         MC2Coordinate()/*poiCoord*/,
         ""/*poiName*/, language, 
         rights, p->getItemInfoFilter(), replyData );
      
   }
   // Fill the packet with the results
   replyPacket->setInfo( replyData );

   return replyPacket;
}

inline void
MapProcessor::findRouteableItemsNear(Item* expandedItem,
                                     RouteExpandItemReplyPacket* p,
                                     GenericMap* theMap)
{
   // Means that we won't check more than 2*maxCoordinates, I think
   const int maxCoordinates = 50;
   
   mc2dbg4 << "findRouteablesItemsNear" << endl;
   
   GfxData* gfx = expandedItem->getGfxData();

   // Force the use of getOneGood
   bool useOneGood = false;
   switch ( expandedItem->getItemType() ) {
      case ItemTypes::builtUpAreaItem:
      case ItemTypes::municipalItem:
         useOneGood = theMap->getOneGoodCoordinate( expandedItem ).isValid();
         break;
      default:
         // Default here
         break;
   }
   
   if ( gfx == NULL || useOneGood ) {
      if ( gfx == NULL ) {
         mc2dbg << "[MP]: findRouteablesItemsNear: Gfxdata == NULL "
                << " for " << IDPair_t(theMap->getMapID(),
                                       expandedItem->getID() ) << endl;
      }
      MC2Coordinate coord = theMap->getOneGoodCoordinate( expandedItem );
      if ( !coord.isValid() ) {
         mc2dbg << "[MP]: findRouteablesItemsNear: getOneGood also failed"
                << endl;
      } else {
         mc2dbg << "[MP]: Using one good" << endl;
         // Get the closest street
         uint64 dist;
         set<ItemTypes::itemType> allowedTypes;
         allowedTypes.insert( ItemTypes::streetSegmentItem );
         uint32 itemID = theMap->getClosestItemID( coord, dist, allowedTypes );
         Item* ssi = theMap->itemLookup( itemID );
         if ( ssi != NULL ) {
            coord = theMap->getOneGoodCoordinate( ssi );
            // FIXME: Get real offset.
            uint16 offset = MAX_UINT16;
            p->addItem(1, 
                       expandedItem->getID(),
                       expandedItem->getItemType(),
                       &itemID,
                       &offset,
                       &coord.lat, 
                       &coord.lon,                    
                       &itemID );
         } else {
            mc2dbg << "[MP]: Item was NULL when looking up nearest" << endl;
         }
      }
      return;
   }

   typedef pair<StreetSegmentItem*, uint16> ssiPair;
   typedef pair<uint32, ssiPair> ssiTriple;
   // Map to put the pair of nodeID:s and pair of segments and offsets in
   //
   typedef map<uint32, ssiPair> itemMap_t;
   itemMap_t itemMap;
      
   int gfxSize = gfx->getNbrCoordinates(0);

   int coordStep = 1;
   // Check this later.
   if ( gfxSize > maxCoordinates ) {
      coordStep = gfxSize / maxCoordinates;
   }

   // Set up the hashtable
   MapHashTable* hashTable = theMap->getHashTable();
   hashTable->clearAllowedItemTypes();
   hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);

   // Use some of the coordinates and find the closest ssi:s.
   for(int i=0; i < gfxSize; i += coordStep) {
      mc2dbg4 << "i=" << i << endl;
      int32 curLat = gfx->getLat(0,i);
      int32 curLon = gfx->getLon(0,i);
      mc2dbg4 << "lat[" << i << "] = " << curLat << endl;
      mc2dbg4 << "lon[" << i << "] ="  << curLon << endl;
      uint64 closestDist; // Unused for now
      // BLaargh! The hashtable uses lon, lat not lat, lon!!!!!
      // It took me quite some time to figure out.
      uint32 closestID = hashTable->getClosest( curLon,
                                                curLat,
                                                closestDist);

      mc2dbg4 << "closestdist = "
              << (::sqrt(closestDist)*GfxConstants::MC2SCALE_TO_METER)
              << endl;
      
      Item* closestItem = theMap->itemLookup(closestID);
      if ( closestItem != NULL &&
           closestItem->getItemType() == ItemTypes::streetSegmentItem) {
         mc2dbg4 << "ItemID is " << hex << closestItem->getID() << dec
                << endl;

         // Find the closest offset. Can be strange sometimes when
         // a road follows the miscitem in question.

         int32 closestOffset = theMap->getItemOffset(closestItem,
                                                     curLat,
                                                     curLon);
         if ( closestOffset < 0 ) {
            mc2log << warn << "closestOffset failed for item "
                   << hex << closestItem->getID() << dec << endl;            
            // We won't add this one. There's something fishy about it
         } else {

            // Check if the ssi should be added or not.
            // We only need to add the ones with the
            // greatest offset and the smallest offset,
            // because we can't reach the other ones without
            // passing them.
            // We could add all of them but it's better to sort
            // them out here so we won't have to send them to RM

            // See if there are streetsegments with the same ID.
            uint32 idToAdd = closestItem->getID();
            bool shouldAdd = false;

            itemMap_t::const_iterator it(itemMap.find(idToAdd));
            if ( it != itemMap.end() ) {
               // Check node 0
               if ( closestOffset <= it->second.second ) {
                  shouldAdd = closestOffset != it->second.second;
               } else {
                  // Check node 1
                  idToAdd = idToAdd | 0x80000000;
                  closestOffset = MAX_UINT16 - closestOffset;
                  it = itemMap.find(idToAdd);
                  if ( it == itemMap.end() ||
                       closestOffset < it->second.second ) {
                     // Didn't find it or offset less than the
                     // last one.
                     shouldAdd = true;
                  }
               }
            } else {
               // Didn't find it. Add it
               shouldAdd = true;
            }
            
            
            // Add if offset smaller or bigger, depending on nodeID
            if ( shouldAdd ) {
               StreetSegmentItem* s =
                  static_cast<StreetSegmentItem*>(closestItem);            
               itemMap.insert(ssiTriple(idToAdd,
                                        ssiPair(s,closestOffset)));
            }
         }
      } else {
         mc2dbg4 << "Closest item is NULL or not ssi. ID is "
                 << hex << closestID << dec << " (" << curLat << ','
                 << curLon << ')' << endl;
      }
   }
   
   mc2dbg4 << "Found " << itemMap.size() << " streetsegments " << endl;

   uint32 expandedID = expandedItem->getID();
  
   Vector ssis(itemMap.size());
   Vector lats(itemMap.size());
   Vector lons(itemMap.size());
   Vector16 offsets(itemMap.size());
   Vector parentItemIDs(itemMap.size());
   
   // Loop over the items in the map and add them.
   itemMap_t::const_iterator it(itemMap.begin());

   for( ; it != itemMap.end(); ++it) {
      StreetSegmentItem* ssi = it->second.first;
      
      // Adds one item at a time         
      uint32 ssiID  = ssi->getID(); // Take thr real id, not nodeid.
      // Add the offset
      uint16 offset = it->second.second;
      if ( ! MapBits::isNode0( it->first ) ) {
         // Convert from node offset to segment offset.
         offset = MAX_UINT16 - offset;
      }
      int32  curLat;
      int32  curLon;
      
      if ( theMap->getItemCoordinates(ssiID, offset, 
                                      curLat, curLon) ) {
         mc2dbg4 << "Adding item " << hex << ssiID << " to expanded item "
                << expandedID << dec << endl;
         // Seems like the expanded id:s should be added all at once.
         ssis.addLast(ssiID);
         lats.addLast(curLat);
         lons.addLast(curLon);
         offsets.addLast(offset);
         parentItemIDs.addLast(expandedID);
         
      } else {
         mc2dbg2 << "No coordinates for item " << ssiID << endl;
      }      
   }

   // Add to the reply packet
   // Copied from below... a little messy
   if ( ssis.getSize() > 0 ) {
      p->addItem((uint16)ssis.getSize(), 
                 expandedID,
                 expandedItem->getItemType(),
                 ssis.getBuffer(),
                 offsets.getBuffer(),
                 (int32*) lats.getBuffer(), 
                 (int32*) lons.getBuffer(),
                 parentItemIDs.getBuffer());
   } else {
      // Like when expanding StreetItems...
      mc2log << warn  << "Failed to expand item " << hex
             << expandedItem->getID() << dec << endl;
      int foo = MAX_INT32;
      uint32 uintFoo = MAX_UINT32;
      uint16 foo16 = 0;
      p->addItem(0, expandedID, 
                 expandedItem->getItemType(),
                 &expandedID, &foo16, 
                 &foo, &foo, &uintFoo);               
   }

}

RouteExpandItemReplyPacket*
MapProcessor::processRouteExpandItemRequestPacket(
                              const RouteExpandItemRequestPacket* p)
{
   uint32 mapID = p->getMapID();
   GenericMap* theMap = m_mapHandler->getMap(mapID);

   if (theMap == NULL) {
      mc2log << warn << "processRouteExpandItemRequestPacket: theMap == NULL"
             << endl;
      mc2dbg1 << "   MapID = " << mapID << endl;
      return (NULL);
   }

   RouteExpandItemReplyPacket* replyPacket = 
      new RouteExpandItemReplyPacket(p);
   mc2dbg4 << "   ReplyPacket IP: " << replyPacket->getOriginIP() 
           << endl;
   mc2dbg4 << "             port: " << replyPacket->getOriginPort() 
           << endl;
   DEBUG8(replyPacket->dump());
   
   mc2dbg1 << "processRouteExpandItemRequestPacket: nbrItems = " 
           << p->getNbrItems() << endl;
   
   // The status for the replypacket
   StringTable::stringCode status = StringTable::OK;
   
   // Check all the items in the requestpacket
   for (uint32 i=0; i<p->getNbrItems(); i++) {
      // Get the data from the request packet
      uint32 itemID;
      uint16 offset;
      p->getData(i, itemID, offset);
      itemID = itemID & 0x7fffffff;
      mc2dbg4 << "   Got data (" << i << "), ID=" << itemID 
              << " offset=" << offset << endl; 
      Item* item = theMap->itemLookup(itemID);
      if (item != NULL) {

      // Switch on the item type
      switch (item->getItemType()) {
         // = = = = = = = = = = = = = = = = = = = = = = = streetSegmentItem
         //case ItemTypes::ferryItem: // Only for pi
         case (ItemTypes::streetSegmentItem) : {
            uint32 uintFoo = MAX_UINT32;
            int32 curLat, curLon;

            if (theMap->getItemCoordinates(itemID, offset,
                                           curLat, curLon))
            {
               mc2dbg4 << "processRouteExpandItemRequestPacket: "
                       << "Adding streetSegment with ID " 
                       << itemID << " with coordinates (" 
                       << curLat << ", " << curLon << ")" << endl;
               replyPacket->addItem(1, itemID, 
                                    ItemTypes::streetSegmentItem,
                                    &itemID, &offset, 
                                    &curLat, &curLon, &uintFoo);
            } else {
               mc2log << warn <<"   ExpandRouteItem (SSI) failed to get "
                      << "coordinates" << "   item ID=" << itemID << endl;
               int foo = MAX_INT32;
               uint16 foo16 = 0;
               replyPacket->addItem(0, itemID, 
                                    ItemTypes::streetSegmentItem,
                                    &itemID, &foo16, 
                                    &foo, &foo, &uintFoo);
            }
         }
         break;

         // = = = = = = = = = = = = = = = = = = = = = = = = = = streetItem
         case (ItemTypes::streetItem) : {
            StreetItem* si = static_cast<StreetItem*>(item);
            Vector ssi(16);
            Vector parentItemID(16);
            Vector16 offset(16);
            Vector lat(16);
            Vector lon(16);
            mc2dbg2 << "    Expanding ID " << item->getID() << endl;
            for (uint32 i=0; i< si->getNbrItemsInGroup(); i++) {
               StreetSegmentItem* ssiitem = item_cast<StreetSegmentItem*>
                                  (theMap->itemLookup(si->getItemNumber(i)));
               if (ssiitem != NULL) {
                  int32 curLat, curLon;
                  if (theMap->getItemCoordinates(ssiitem->getID(), 0, 
                                                curLat, curLon)) {
                     mc2dbg4 << "       SSI-ID " << ssiitem->getID() 
                             << endl;
                     ssi.addLast(ssiitem->getID());
                     offset.addLast(0); // the first node.
                     lat.addLast(curLat);
                     lon.addLast(curLon);
                     parentItemID.addLast(itemID);
                  } else {
                     mc2log << warn <<"   ExpandRouteItem (SI) failed to get " 
                            << "1st coordinates  item ID=" << itemID << endl;
                  }
               }
               if (ssiitem != NULL) {
                  int32 curLat, curLon;
                  if (theMap->getItemCoordinates(ssiitem->getID(),MAX_UINT16, 
                                                curLat, curLon)) {
                     mc2dbg4 << "       SSI-ID " << ssiitem->getID() 
                             << endl;
                     ssi.addLast(ssiitem->getID());
                     offset.addLast(MAX_UINT16); // Nodus secundus.
                     lat.addLast(curLat);
                     lon.addLast(curLon);
                     parentItemID.addLast(itemID);
                  } else {
                     mc2log << warn <<"   ExpandRouteItem (SI) failed to get " 
                            << " 2nd coordinates item ID=" << itemID << endl;
                  }
               }
            }

            // Add to the reply packet
            if (ssi.getSize() > 0) {
               replyPacket->addItem((uint16)ssi.getSize(), 
                                    itemID,
                                    ItemTypes::streetItem,
                                    ssi.getBuffer(),
                                    offset.getBuffer(),
                                    (int32*) lat.getBuffer(), 
                                    (int32*) lon.getBuffer(),
                                    parentItemID.getBuffer());
            } else {
               mc2log << error
                      << "   FAILED to expand street, item->getNbrGroups()=" 
                      << int(item->getNbrGroups()) << ", ID=" 
                      << item->getID() << endl;
               int foo = MAX_INT32;
               uint32 uintFoo = MAX_UINT32;
               uint16 foo16 = 0;
               replyPacket->addItem(0, itemID, 
                                    ItemTypes::streetItem,
                                    &itemID, &foo16, 
                                    &foo, &foo, &uintFoo);
            }
         }
         break;

         // = = = = = = = = = = = = = = = = = = = = = = pointOfInterestItem
         case (ItemTypes::pointOfInterestItem) : {
            // Currently the company only have one SSI, 
            // so we expand that!
            uint32 ssiID = static_cast<PointOfInterestItem*>(item)
                           ->getStreetSegmentItemID();
            uint16 offset = static_cast<PointOfInterestItem*>(item)
                           ->getOffsetOnStreet();
            int32 curLat, curLon;

            if (theMap->getItemCoordinates(ssiID, offset, curLat, curLon)) {
               mc2dbg4 << "processItemLatLongPacket: Adding itemID" 
                       << itemID << " with coordinates (" 
                       << curLat << ", " << curLon << ")" << endl;
               replyPacket->addItem(1, itemID, 
                                    ItemTypes::pointOfInterestItem,
                                    &ssiID, &offset, 
                                    &curLat, &curLon, &itemID);
            } else {
               mc2log << warn <<"   ExpandRouteItem (POI), failed to get "
                      << "coordinates   item ID=" 
                      << itemID << endl;
               int foo = MAX_INT32;
               uint16 foo16 = 0;
               replyPacket->addItem(0, itemID, 
                                    ItemTypes::pointOfInterestItem,
                                    &itemID, &foo16, 
                                    &foo, &foo, &itemID);
            }
            
         }
         break;
                                          
         // = = = = = = = = = = = = = = = = = = = = = = = = = categoryItem
         case (ItemTypes::categoryItem) : {
            // Expand a category-item into its items
            CategoryItem* cat = static_cast<CategoryItem*>(item);
            uint32 nbrPOI = cat->getNbrItemsInGroup();
            
            // Allocate temporary arrays for the companies
            uint32 ssiID[nbrPOI];
            uint16 offset[nbrPOI];
            int32 lat[nbrPOI];
            int32 lon[nbrPOI];
            uint32 parentItemID[nbrPOI];

            // Add all the companies to the reply packet
            uint32 nbrAdded = 0;
            for (uint32 i=0; i<nbrPOI; i++) {
               PointOfInterestItem* poi = item_cast<PointOfInterestItem*>
                  (theMap->itemLookup(cat->getItemNumber(i)));
               if (poi != NULL) {
                  ssiID[nbrAdded] = poi->getStreetSegmentItemID();
                  parentItemID[nbrAdded] = poi->getID();
                  uint16 curOffset = offset[nbrAdded] 
                                   = poi->getOffsetOnStreet();
                  if (theMap->getItemCoordinates(poi->getID(), curOffset,
                                                 lat[nbrAdded], 
                                                 lon[nbrAdded])) {
                     nbrAdded++;
                  }
               }
            }

            mc2dbg4 << "   To add " << nbrAdded << " POIs for category "
                    << theMap->getFirstItemName(cat) << endl;
            replyPacket->addItem(nbrAdded, itemID, 
                                 ItemTypes::categoryItem,
                                 ssiID, offset, 
                                 lat, lon, parentItemID);
         }
         break;
         
         // = = = = = = = = = = = = = = = = = = = = = = = = Lakes and stuff
         case ItemTypes::waterItem:
         case ItemTypes::forestItem:
         case ItemTypes::parkItem:
         case ItemTypes::individualBuildingItem:
         default:
            // Expand these types into the nearest items.
            findRouteableItemsNear(item,
                                   replyPacket,
                                   theMap);

         break;
            
         
         // = = = = = = = = = = = = = = = = = = = = = = = = unsupported item
         /*
         default : {
            // 
            mc2log << warn <<"   ExpandRouteItem, unsupported itemtype"
                   << "   itemID=" << itemID << ", " << 
               StringTable::getString(
                  ItemTypes::getItemTypeSC(item->getItemType()), 
                  StringTable::ENGLISH) << endl;
            status = StringTable::NOTFOUND;
         }
         */

      } // switch
      } // if
   }

   // Set the status of the reply packet
   replyPacket->setStatus(status);
   
   mc2dbg4 << "   Returning replyPacket" << endl;
   DEBUG8(replyPacket->dump());
   mc2dbg8 << "========================" << endl;
   return (replyPacket);

}

// Quick fix for the broken version that seems to create a new
// packet for each coordinate.
class CoordPackResult {
public:
   CoordPackResult( uint32 itemID, int32 lat, int32 lon,
                    const MC2BoundingBox& bbox)
         : m_itemID(itemID), m_lat(lat), m_lon(lon), m_bbox(bbox) {};
   
   uint32         m_itemID;
   int32          m_lat;
   int32          m_lon;
   MC2BoundingBox m_bbox;
   
};

CoordinateOnItemReplyPacket*
MapProcessor::processCoordinateOnItemRequestPacket(
                              const CoordinateOnItemRequestPacket* p)
{
   if (p == NULL) {
      mc2log << warn << "processCoordinateOnItemRequestPacket: p == NULL"
             << endl;
      return (NULL);
   }

   uint32 mapID = p->getMapID();
   GenericMap* theMap = m_mapHandler->getMap(mapID);
   
   if (theMap == NULL) {
      mc2log << warn <<"processCoordinateOnItemRequestPacket: theMap == NULL"
             << endl;
      mc2dbg1 << "   MapID = " << mapID << endl;
      return (NULL);
   }
   
   // Check if we should send boundingboxes too
   bool bboxWanted = p->getBBoxWanted();
   // Check if all the coordinates is going to be added
   bool allCoordsWanted = p->getAllCoordinatesWanted();

   mc2dbg2 << "processCoordinateOnItemPacket: nbrItems = " 
           << p->getNbrItems() << endl;

   vector<CoordPackResult> resVect;

   static const MC2Coordinate invalidCoord;
   static const MC2BoundingBox invalidBBox;
   
   for (uint32 i=0; i<p->getNbrItems(); i++) {
      uint32 offset;
      uint32 itemID;      
      p->getData(i, itemID, offset);
      
      Item* theItem = theMap->itemLookup(itemID);

      // Check for NULL once
      if ( theItem == NULL ) {
         resVect.push_back( CoordPackResult( itemID, invalidCoord.lat,
                                             invalidCoord.lon,
                                             invalidBBox) );
         mc2log << error << "MapProcessor::"
            "processCoordinateOnItemRequestPacket"
            " Failed to get item " << IDPair_t(mapID, itemID) << endl;
         continue;
      }

      // From here on the item cannot be NULL.
      MC2BoundingBox bbox;
      if ( bboxWanted ) {               
         theMap->getItemBoundingBox(bbox, theItem);
      }
      
      if( ! allCoordsWanted ) {
         MC2Coordinate coord;
         // Check if we should use the "best coordinate"
         if ( offset > MAX_UINT16 ) {
            // MAX_UINT32 means "best offset".
            theMap->getOneGoodCoordinate(coord, theItem);            
         } else {
            // Offset should be used.            
            if (theMap->getItemCoordinates(itemID, offset,
                                           coord.lat, coord.lon)) {
               mc2dbg4 << "processItemLatLongPacket: Adding itemID" 
                       << itemID << " with coordinates "
                       << coord << endl;
               
            } else {
               mc2log << error << "MapProcessor::"
                  "processCoordinateOnItemRequestPacket"
                  " Failed to get coordinates [1] for item "
                      << IDPair_t(mapID, itemID) << endl;
               
            }
         }
         resVect.push_back( CoordPackResult( itemID,
                                             coord.lat,
                                             coord.lon, bbox) );
      } else {
         // All coordinates should be added.
         
         // Add all the coordinates
         GfxData* gfxData = theItem->getGfxData();
         if( gfxData != NULL ) {
            for(uint32 j = 0; j < gfxData->getNbrPolygons(); j++) {
               for(uint32 k = 0; k < gfxData->getNbrCoordinates(j); k++) {
                  int32 lat = gfxData->getLat(j, k);
                  int32 lon = gfxData->getLon(j, k);
                  resVect.push_back(
                     CoordPackResult( itemID, lat, lon, bbox) );
               }
            }
         } else {
            mc2log << error << "MapProcessor::"
               "processCoordinateOnItemRequestPacket"
               " Failed to get gfxData for item "
                   << IDPair_t(mapID, itemID) << endl;
            resVect.push_back(
               CoordPackResult(itemID, MAX_INT32, MAX_INT32,
                               MC2BoundingBox()));
         }
      }
   }
   
   CoordinateOnItemReplyPacket* replyPacket =
      new CoordinateOnItemReplyPacket(p, resVect.size());
   for ( uint32 i = 0 ; i < resVect.size(); ++i ) {
      CoordPackResult& curRes = resVect[i];
      if ( curRes.m_bbox.isValid() ) {
         replyPacket->add(curRes.m_itemID, curRes.m_lat, curRes.m_lon,
                          &curRes.m_bbox);
      } else {
         replyPacket->add(curRes.m_itemID, curRes.m_lat, curRes.m_lon, NULL);
      }
   }
   
   mc2dbg4 << "   Returning replyPacket" << endl;
   DEBUG8(replyPacket->dump());
   mc2dbg8 << "========================" << endl;
   return (replyPacket);
}


CoveredIDsReplyPacket*
MapProcessor::processCoveredIDsRequestPacket( const CoveredIDsRequestPacket* p ) 
{  
   const uint32 mapID = p->getMapID();
   GenericMap *curMap = m_mapHandler->getMap(mapID);
   // Map shouldn't be NULL, that has been checked in handleRequest
   MC2_ASSERT( NULL != curMap );
   
   uint32 status = StringTable::OK;
   
   mc2dbg4 << "   mapID = " << mapID << endl;

   CoveredIDsReplyPacket* returnPacket = NULL;

   if (curMap == NULL) {
      //curMap == NULL
      returnPacket = new CoveredIDsReplyPacket(p);
      // Why is the mapid set to MAX_UINT32 - 1? Now the sender
      // doesn't know which map this was about in a simple way.
      returnPacket->setMapID(MAX_UINT32-1);
      mc2dbg4 << "   curMap == NULL" << endl;
      status = StringTable::MAPNOTFOUND;

   } else {
      // OK
      // Get the allowed item types and user rights from the packet
      UserRightsMapInfo rights;
      set<ItemTypes::itemType> itemTypes;
      p->getItemTypesAndRights(itemTypes, rights);
      mc2dbg << rights << endl;
      if ( itemTypes.empty() ) {
         // Old sender of packet. Add streetSegmentItems
         itemTypes.insert(ItemTypes::streetSegmentItem);
         mc2dbg << "[MProc]: CovID - No types - assuming ssi" << endl;
      }
      
      // Get the data about the area
      mc2dbg << "[MProc]: p->outerRadius=" << p->getOuterRadius() 
             << "m, p->innerRadius()=" << p->getInnerRadius()
             << "m" << endl;
      int32 radius = (int32) (p->getOuterRadius() * 
                              GfxConstants::METER_TO_MC2SCALE);
      int32 innerRadius = (int32) ( p->getInnerRadius() * 
                                    GfxConstants::METER_TO_MC2SCALE);
      int32 startAngle = p->getStartAngle();
      int32 stopAngle = p->getStopAngle();
      int32 lat = MAX_INT32;
      int32 lon = MAX_INT32;
      if (p->isItemID()) {
         mc2dbg4 << "Indata is itemID = " << p->getItemID() << endl;
         if (!curMap->getItemCoordinates(p->getItemID(), p->getOffset(), 
                                        lat, lon)) {
            mc2log << warn
                   << here << " MapProcessor:: Error setting lat, lon)"
                   << endl;
            lat = MAX_INT32;
            lon = MAX_INT32;
         }
      } else {
         lat = p->getLat();
         lon = p->getLon();
      }
      mc2dbg4 << "   Indata (lat, lon, r_o, r_i, a1, a2) = (" 
              << lat  << ", " << lon << ", " << radius << ", "
              << innerRadius << ", " << startAngle << ", "
              << stopAngle << "), radius in MC2-scale" << endl;

      bool boundingBoxArea = startAngle == MAX_UINT16 && 
         stopAngle == MAX_UINT16;

      // If the area not is cirle we create an approximation of it here
      GfxData* areaGfxData = NULL;
      if ( startAngle != 0 && stopAngle != 360 && !boundingBoxArea ) {
         areaGfxData = GfxData::getArcApproximation(
            lat, lon, startAngle, stopAngle, radius, innerRadius);
      }
      
      
      // Get the items that are inside the given area
      if ((lat != MAX_INT32) && (lon != MAX_INT32)) {
         
         // Use the function where we get a set.
         set<Item*> allowedItems;
         if ( !boundingBoxArea && ( areaGfxData == NULL ) ) {
            // Use the radius.
            mc2dbg << "[MP]: Using getItemsWithinRadiusMeter" << endl;
            curMap->getItemsWithinRadiusMeter( 
               allowedItems, MC2Coordinate( lat, lon ), 
               p->getOuterRadius(), itemTypes, &rights );
         } else if ( areaGfxData == NULL ) {
            mc2dbg << "[MP]: Using getItemsWithinBBox" << endl;
            // Use the bounding box.
            MC2BoundingBox bbox( MC2Coordinate( lat, lon ), 
                                  p->getOuterRadius() );
            curMap->getItemsWithinBBox( allowedItems, bbox, itemTypes,
                                        &rights );
         } else {
            mc2dbg << "[MP]: Using getItemsWithinGfxData" << endl;
            // Use the area gfxData
            curMap->getItemsWithinGfxData( allowedItems, areaGfxData,
                                           itemTypes, &rights);
         }

         delete areaGfxData;
         areaGfxData = NULL;
         
         mc2dbg4 << "   To add " << allowedItems.size() << " items " 
                 << endl;
         // Calculate size of packet and create it.
         returnPacket =
            new CoveredIDsReplyPacket(
               p,
               CoveredIDsReplyPacket::calcPacketSize(allowedItems.size()));
         

         // Add the ID of the items that are inside to the packet
         for (set<Item*>::const_iterator it = allowedItems.begin();
              it != allowedItems.end();
              ++it ) {
            const Item* item = *it;
            // No checks should be necessary, just add to the packet.
            returnPacket->addID(item->getID(),
                                item->getItemType());
         }
         mc2dbg << "[MProc]:   Added " << returnPacket->getNumberIDs()
                << " of " << allowedItems.size() << " items to covpacket"
                << endl;
      } else {
         mc2dbg << warn <<  "MapProcessor:: Error in coordinates or item ID"
                <<  "   ID=" << p->getItemID() << ", lat="
                << lat << ", lon=" << lon << endl;
         status = StringTable::NOTFOUND;
      }
   }

   if ( returnPacket == NULL ) {
      DEBUG1(if ( status == StringTable::OK ) {
               mc2dbg << here
                      << " Error: Status is ok, but packet is NULL" << endl;
      });
      // No list added or first if-clause hit
      returnPacket = new CoveredIDsReplyPacket(p);
      returnPacket->setStatus(status);
   }

   return (returnPacket);
}

TrafficPointReplyPacket*
MapProcessor::processTrafficPointRequestPacket(
   const TrafficPointRequestPacket* p ) 
{  
   const uint32 mapID = p->getMapID();
   GenericMap *curMap = m_mapHandler->getMap(mapID);
   // Map shouldn't be NULL, that has been checked in handleRequest
   MC2_ASSERT( NULL != curMap );
   
   uint32 status = StringTable::OK;
   
   mc2dbg4 << "   mapID = " << mapID << endl;

   TrafficPointReplyPacket* returnPacket = NULL;
   if (curMap == NULL) {
      //curMap == NULL
      returnPacket = new TrafficPointReplyPacket(p);
      // Why is the mapid set to MAX_UINT32 - 1? Now the sender
      // doesn't know which map this was about in a simple way.
      returnPacket->setMapID(MAX_UINT32-1);
      mc2dbg4 << "   curMap == NULL" << endl;
      status = StringTable::MAPNOTFOUND;

   } else {
      // OK
      // Get the allowed item types and user rights from the packet
      UserRightsMapInfo rights;
      set<ItemTypes::itemType> itemTypes;
      p->getUserRights(rights);
      uint8 maxRoadClass = p->getMaxRoadClass();
      
      mc2dbg << rights << endl;
      itemTypes.insert(ItemTypes::streetSegmentItem);
      
      // Get the data about the area
      //int32 radius = (int32) (p->getOuterRadius() * 
      //                        GfxConstants::METER_TO_MC2SCALE);
      //int32 innerRadius = (int32) ( p->getInnerRadius() * 
      //                              GfxConstants::METER_TO_MC2SCALE);
      //uint32 radius = p->getMaxRadius();
      //int32 angle = p->getAngle();
      int32 lat = MAX_INT32;
      int32 lon = MAX_INT32;
      lat = p->getLat();
      lon = p->getLon();
      
      
      
      // Get the items that are inside the given area
      if ((lat != MAX_INT32) && (lon != MAX_INT32)) {
          // Use the function where we get a set.
         set<Item*> allowedItems;
         uint32 radius = p->getMaxRadius();
         mc2dbg << "[MP]: Using getItemsWithinRadiusMeter" << endl;
         curMap->getItemsWithinRadiusMeter(allowedItems,
                                           MC2Coordinate( lat, lon ), 
                                           radius,
                                           itemTypes,
                                           &rights );
         
         mc2dbg4 << "   Found " << allowedItems.size() << " items " 
                 << endl;

         // This code is shamelessly stolen from getBestStreetSegment()
         // We need some mods however.
         // * Items in the wrong direction must be penalized
         // * Small angle differences are not relevant
         // * check for roadclass
         // * check for roadname
         mc2dbg4 << "Center coordinate (" << lat << "," << lon << "), radius "
              << radius << " m, angle " << p->getAngle() << " deg " << endl;

         // Get a new "direction point" from the angle & radius.
         // Lets say at rad*3
         double f_angle = double(p->getAngle())*M_PI/180;
         float64 mc2_distance =
            GfxConstants::METER_TO_MC2SCALE *3*radius;
         
         int32 dirLat = lat + int32(mc2_distance*cos(f_angle));
         int32 dirLon = lon + int32(mc2_distance*sin(f_angle)*2);
         mc2dbg8 << " DIRCoord (" << dirLat << "," << dirLon << ")" << endl;
         mc2dbg4 << "p->getTrafficDirection() = "
              << (int) p->getTrafficDirection() << endl;

         vector<ssiNotice_t> notices;
         vector<ssiNotice_t> backnotices;
         notices.reserve( allowedItems.size() );
         for (set<Item*>::const_iterator it = allowedItems.begin();
              it != allowedItems.end(); ++it ) {
            Item* item = *it;
            if (item != NULL) {
               ssiNotice_t notice;
               notice.m_item = item;
               notice.m_score = 0;
               const GfxData* gfx = item->getGfxData();
               notice.m_dist =
                  uint32(::sqrt(gfx->squareDistToLine(lat, lon)));
               int tmpOffset = curMap->getItemOffset(item, lat, lon);
               if (tmpOffset >= 0) {
                  notice.m_offset = uint16(tmpOffset);
               } else {
                  mc2log << warn
                         << "MapProcessor::getTrafficPoint INVALID offset" 
                    << endl;
                  notice.m_offset = 0;
               }
               int32 noticeLat = MAX_INT32;
               int32 noticeLon = MAX_INT32;
               gfx->getCoordinate(notice.m_offset, noticeLat, noticeLon);
               
               // The angle when driving in positive direction
               notice.m_angle = uint16(gfx->getAngle(notice.m_offset));
               
               notice.m_angleDiff = notice.m_angle - p->getAngle();
               mc2dbg4 << notice.m_angle << " - " << p->getAngle() << " = "
                    << notice.m_angleDiff << endl;
               if (notice.m_angleDiff < -180) {
                  notice.m_angleDiff += 360;
               } else if (notice.m_angleDiff > 180) {
                  notice.m_angleDiff -= 360;
               }
               
               // Get entry restrictions
               ItemTypes::entryrestriction_t restNode0 = 
                  static_cast<StreetSegmentItem*>(item)
                  ->getNode(0)->getEntryRestrictions();
               ItemTypes::entryrestriction_t restNode1 = 
                  static_cast<StreetSegmentItem*>(item)
                  ->getNode(1)->getEntryRestrictions();

               if((notice.m_angleDiff > 90) ||
                  (notice.m_angleDiff < -90)){
                  
                  // We'd might as well go the other way.
                  if(notice.m_angleDiff > 90){
                     notice.m_angleDiff -= 180;
                     mc2dbg4 << "2 large angle, flippn'" << endl;
                  } else {
                     notice.m_angleDiff += 180;
                     mc2dbg4 << "2 small angle, flippn'" << endl;
                  }
                  

                  // But now we have to switch the nodes
                  restNode0 = 
                  static_cast<StreetSegmentItem*>(item)
                  ->getNode(1)->getEntryRestrictions();
                  restNode1 = 
                  static_cast<StreetSegmentItem*>(item)
                  ->getNode(0)->getEntryRestrictions();
               }
               

               
               uint8 roadClass =  static_cast<StreetSegmentItem*>(item)
                  ->getRoadClass();
               
               // Calculate the score
               notice.m_score = notice.m_dist;
               mc2dbg4 << " - distance score : " << notice.m_score << endl;

               // Check road class
               if(roadClass > maxRoadClass){
                  mc2dbg4 << "To high roadclass  " << roadClass << ">"
                       << maxRoadClass << endl;
                  notice.m_score += 200;
               }
               
               // Check the direction.
               

               // Check the distance to this.
               uint32 dirDist =
                  uint32(::sqrt(gfx->squareDistToLine(dirLat, dirLon)));
               // Add score according to distance.
               notice.m_score += dirDist/6;
               mc2dbg4 << " - direction score : " << dirDist/6 << endl;

               
               ssiNotice_t notice2 = notice;

               if(p->getTrafficDirection() != TrafficDataTypes::Negative){
                  // Check angle.
                  if ( (abs(notice.m_angleDiff) > 135) &&
                       (restNode1 != ItemTypes::noRestrictions) && 
                       (restNode1 != ItemTypes::noThroughfare) ) {
                     // Heading more or less backwards that not is allowed
                     //notice.m_score *= 4;
                     notice.m_score += abs(notice.m_angleDiff)*2;
                     mc2dbg4 << " - angle score " << abs(notice.m_angleDiff)*2
                             << endl;
                     /// 2;
                     mc2dbg2 << "   Backwards, not allowed, angleDiff=" 
                             << notice.m_angleDiff << endl;
                  } else if ( (abs(notice.m_angleDiff) < 45) &&
                              (restNode0 != ItemTypes::noRestrictions) && 
                              (restNode0 != ItemTypes::noThroughfare) ) {
                     // Heading more or less forward that not is allowed
                     //notice.m_score *= 4;
                     notice.m_score += 2*(180 - abs(notice.m_angleDiff));
                     mc2dbg4 << " - angle Score "
                          << 2*(180 - abs(notice.m_angleDiff))<< endl;
                     /// 2;
                     mc2dbg2 << "   Forwards, not allowed, angleDiff=" 
                             << notice.m_angleDiff << endl;
                  } else if ( (abs(notice.m_angleDiff) >= 45) && 
                              (abs(notice.m_angleDiff) <= 135)) {
                     // Heading more or less left/right
                     //notice.m_score *= 2;
                     notice.m_score += abs(notice.m_angleDiff)*2;
                     mc2dbg4 << " angle score " << abs(notice.m_angleDiff)*2
                          << endl;
                     /// 2;
                     mc2dbg2 << "   About 90 deg wrong, angleDiff=" 
                             << notice.m_angleDiff << endl;
                  } else {
                     
                     notice.m_score += abs(notice.m_angleDiff);
                     mc2dbg4 << " - angle score " << abs(notice.m_angleDiff)
                          << endl;
                  }
                  
                  mc2dbg4 << "Score set to = " << notice.m_score << " : " 
                      << item->getID() << endl;
                  mc2dbg4 << "Coord = (" << noticeLat << "," << noticeLon
                       << ")" << endl;
               
                  notices.push_back(notice);
               }
               if(p->getTrafficDirection() != TrafficDataTypes::Positive){

                  if(notice2.m_angle > 0 )
                     notice2.m_angle -= 180;
                  else
                     notice2.m_angle += 180;
                  
                  notice2.m_angleDiff = notice2.m_angle - p->getAngle();
                  restNode0 = 
                     static_cast<StreetSegmentItem*>(item)
                     ->getNode(0)->getEntryRestrictions();
                  restNode1 = 
                     static_cast<StreetSegmentItem*>(item)
                     ->getNode(1)->getEntryRestrictions();
                  

                  if((notice2.m_angleDiff > 90) ||
                     (notice2.m_angleDiff < -90)){
                     
                    
                     if(notice2.m_angleDiff > 90)
                        notice2.m_angleDiff -= 180;
                     else
                        notice2.m_angleDiff += 180;

                     
                     // But now we have to switch the nodes
                     restNode0 = 
                        static_cast<StreetSegmentItem*>(item)
                        ->getNode(1)->getEntryRestrictions();
                     restNode1 = 
                        static_cast<StreetSegmentItem*>(item)
                        ->getNode(0)->getEntryRestrictions(); 
                        }

                  // Check angle.
                  if ( (abs(notice2.m_angleDiff) > 135) &&
                       (restNode1 != ItemTypes::noRestrictions) && 
                       (restNode1 != ItemTypes::noThroughfare) ) {
                     // Heading more or less backwards that not is allowed
                     //notice.m_score *= 4;
                     notice2.m_score += abs(notice2.m_angleDiff)*2;
                     mc2dbg4 << " - angle score " << abs(notice2.m_angleDiff)
                          << endl;
                     /// 2;
                     mc2dbg2 << "   Backwards, not allowed, angleDiff=" 
                             << notice2.m_angleDiff << endl;
                  } else if ( (abs(notice2.m_angleDiff) < 45) &&
                              (restNode0 != ItemTypes::noRestrictions) && 
                              (restNode0 != ItemTypes::noThroughfare) ) {
                     // Heading more or less forward that not is allowed
                     //notice.m_score *= 4;
                     notice2.m_score += 2*(180 - abs(notice2.m_angleDiff));
                     mc2dbg4 << " - angle score "
                          << 2*(180 - abs(notice2.m_angleDiff))
                          << endl;
                     /// 2;
                     mc2dbg2 << "   Forwards, not allowed, angleDiff=" 
                             << notice2.m_angleDiff << endl;
                  } else if ( (abs(notice2.m_angleDiff) >= 45) && 
                              (abs(notice2.m_angleDiff) <= 135)) {
                     // Heading more or less left/right
                     //notice.m_score *= 2;
                     notice2.m_score += abs(notice2.m_angleDiff)*2;
                     mc2dbg4 << " angle score " << abs(notice2.m_angleDiff)
                          << endl;
                     /// 2;
                     mc2dbg2 << "   About 90 deg wrong, angleDiff=" 
                             << notice2.m_angleDiff << endl;
                  } else {
                     
                     notice2.m_score += abs(notice2.m_angleDiff);
                     mc2dbg4 << " - angle score " << abs(notice2.m_angleDiff)
                          << endl;
                  }
                  
                  mc2dbg2 << "Back score set to = " << notice2.m_score
                          << " : " << item->getID() << endl;
                  mc2dbg4 << "Coord = (" << noticeLat << "," << noticeLon
                       << ")" << endl;
               
                  backnotices.push_back(notice2);
               }
            }
         }
         // Sort the notice-array
         sort(notices.begin(), notices.end(), LessNotice());
         sort(backnotices.begin(), backnotices.end(), LessNotice());
            
         // Calculate size of packet and create it.
         returnPacket =
            new TrafficPointReplyPacket(p);

         int nbrOfAdded = 0;
         int wantedNbrHits = p->getNbrOfHits();
         mc2dbg2 << "wantedNbrHits : " << wantedNbrHits << endl;
         vector<ssiNotice_t>::const_iterator it = notices.begin();
         while (nbrOfAdded < wantedNbrHits &&
                it != notices.end()) {
            returnPacket->addID((*it).m_item->getID());
            nbrOfAdded++;
            it++;
         }
         
         it = backnotices.begin();
         nbrOfAdded = 0;
         while (nbrOfAdded < wantedNbrHits &&
                it != backnotices.end()) {
            returnPacket->addID((*it).m_item->getID());
            nbrOfAdded++;
            it++;
         }


         mc2dbg  << "[MProc]:   Added " << returnPacket->getNumberIDs()
                << " of " << allowedItems.size() << " items to covpacket"
                << endl;
      } else {
         mc2dbg << warn <<  "MapProcessor:: Error in coordinates or item ID"
                <<  ", lat="
                << lat << ", lon=" << lon << endl;
         status = StringTable::NOTFOUND;
      }
   }

   if ( returnPacket == NULL ) {
      DEBUG1(if ( status == StringTable::OK ) {
               mc2dbg << here
                      << " Error: Status is ok, but packet is NULL" << endl;
      });
      // No list added or first if-clause hit
      returnPacket = new TrafficPointReplyPacket(p);
      returnPacket->setStatus(status);
   }
   return (returnPacket);

}

StringTable::stringCode
MapProcessor::loadMap(uint32 mapID, uint32& mapSize)
{
   StringTable::stringCode status = m_mapHandler->addMap(mapID, mapSize);
   if ( status == StringTable::OK ) {
      mc2log << info  << "MapProcessor, map with ID: " << prettyMapIDFill(mapID)
             << " loaded successfully" << endl;
           
      // START DEBUGGING EXTERNAL CONNECTIONS
      DEBUG8(
         if (MapBits::isUnderviewMap(mapID)) {
            BoundrySegmentsVector* extConns = 
               m_mapHandler->getMap(mapID)->getBoundrySegments();
            
            mc2dbg << "External connections of map " << mapID 
                   << "=====" << hex << endl;
            extConns->dump();
            mc2dbg << "==================================" << dec << endl;
            
            if (extConns != NULL) {
               uint32 nbrSegWithCon = 0;
               for (uint32 i=0; i<extConns->getSize(); i++) {
                     BoundrySegment* curSeg = dynamic_cast<BoundrySegment*>
                        (extConns->getElementAt(i));
                     if ( (curSeg != NULL) && 
                          ( (curSeg->getNbrConnectionsToNode(0) > 0) || 
                            (curSeg->getNbrConnectionsToNode(1) > 0))) {
                        nbrSegWithCon++;
                     }
               }
               mc2dbg << nbrSegWithCon << "  connections to other map(s)"
                    << endl;
            }
         }
         );
      // END DEBUGGING EXTERNAL CONNECTIONS
   } else {
      if ( status != StringTable::ERROR_MAP_LOADED ) {
         mc2log << error << "MapProcessor Error loading map with ID: " 
                << prettyMapIDFill(mapID) << endl;
      }
   }
   // Status already set...
   return status;
}

StringTable::stringCode
MapProcessor::deleteMap(uint32 mapID)
{
   if ( m_mapHandler->removeMap(mapID) ) {
      mc2log << info << "MapProcessor, map with ID: " << prettyMapIDFill(mapID)
             << " deleted successfully" << endl;      
      return StringTable::OK;
   } else {
      mc2log << error << "MapProcessor Error deleting map with ID: " 
             << prettyMapIDFill(mapID) << endl;
      return StringTable::MAPNOTFOUND;
   }
}

ExpandItemReplyPacket*
MapProcessor::processExpandItemRequestPacket(const ExpandItemRequestPacket* req)
{
   // Make sure that the inparameter is valid
   if (req == NULL)
      return (NULL);
   
   // The replyPacket
   ExpandItemReplyPacket* replyPacket = new ExpandItemReplyPacket(req);

   // Get the item, return NULL upon error
   GenericMap* theMap = m_mapHandler->getMap(req->getMapID());
   Item* itemToExpand = NULL;
   if ( (theMap != NULL) && 
        ((itemToExpand = theMap->itemLookup(req->getItemID())) != NULL)) {
      switch (itemToExpand->getItemType()) {
         case (ItemTypes::builtUpAreaItem) : {
            
            // Get the municipal and bua where the item is located
            Item* item = theMap->getRegion(itemToExpand, 
                                             ItemTypes::municipalItem);
            uint32 itemToExpandMunicipal = MAX_UINT32;
            if (item != NULL)
               itemToExpandMunicipal = item->getID();
            item = theMap->getRegion(itemToExpand, 
                                       ItemTypes::builtUpAreaItem);
            uint32 itemToExpandBUA = MAX_UINT32;
            if (item != NULL)
               itemToExpandBUA = item->getID();

            if (req->getExpansionType() == 
                  ExpandItemRequestPacket::bestMatch) {
               // Return the best match for the given BUA
               
               // First check the pois to find the center
               bool found = false;
               uint32 currentID = 0;
               Item* checkItem = NULL;
               while ( (!found) &&
                       (currentID < theMap->getNbrItemsWithZoom(
                                       ItemTypes::pointOfInterestItem))
                       ) {
                  checkItem = item_cast<PointOfInterestItem*>
                     (theMap->getItem(ItemTypes::pointOfInterestItem, 
                                      currentID));
                  if ( (checkItem != NULL) &&
                       (checkItem->memberOfGroup(itemToExpandMunicipal)) &&
                       (checkItem->memberOfGroup(itemToExpandBUA)) ) {
                     checkItem = theMap->itemLookup(
                                    static_cast<PointOfInterestItem*>(
                                    checkItem)->getStreetSegmentItemID());
                     found = true;
                  }
                  currentID++;
               }
               
               // If we didn't found any city centre above, use "a big
               // street in the midle".
               uint32 currentZoom = 0;
               while ( (currentZoom < NUMBER_GFX_ZOOMLEVELS) &&
                       (!found)) {
                  currentID = 0;
                  while ( (currentID < 
                           theMap->getNbrItemsWithZoom(currentZoom)) && 
                        (!found)) {
                     checkItem = theMap->getItem(currentZoom, currentID);
                     if ( (checkItem != NULL) && 
                          (checkItem->getItemType() == 
                              ItemTypes::streetSegmentItem) &&
                          (checkItem->memberOfGroup(itemToExpandMunicipal)) &&
                          (checkItem->memberOfGroup(itemToExpandBUA)) ) {
                        // One item found!
                        found = true;
                     }
                     currentID++;
                  }
                  currentZoom++;
               }
           
               // Add the found (?) item to the replyPacket
               if ((found) && (checkItem != NULL)) {
                  replyPacket->addItem(checkItem->getID());
                  replyPacket->setStatus(StringTable::OK);
                  mc2dbg2 << "processExpandItemRequestPacket:: status "
                          << "== OK" << endl;
               } else {
                  replyPacket->setStatus(StringTable::NOTFOUND);
                  mc2dbg1 << "processExpandItemRequestPacket:: Did not"
                              << " found any SSI" << endl;
               }
            }
         } break;

         case (ItemTypes::streetItem) : 
         case (ItemTypes::categoryItem) : {
            // Expand a goup-item into its items
            GroupItem* group = static_cast<GroupItem*>(itemToExpand);
            for (uint32 i=0; i<group->getNbrItemsInGroup(); i++) {
               replyPacket->addItem(group->getItemNumber(i));
            }

            // Set status of reply packet
            if (group->getNbrItemsInGroup() > 0) {
               replyPacket->setStatus(StringTable::OK);
            } else {
               replyPacket->setStatus(StringTable::NOTFOUND);
            }

         } break;

         default :
            mc2log << warn  << "Does not handle item with type " 
                   << (uint32) itemToExpand->getItemType() << endl;
            replyPacket->setStatus(StringTable::NOTSUPPORTED);
      }
   } else {
      // Either theMap or item is NULL
      replyPacket->setStatus(StringTable::NOTOK);
      mc2log << warn  << "processExpandItemRequestPacket:: theMap == NULL" 
             << " or item == NULL" << endl;
   }

   // Return the locally created packet
   return (replyPacket);
}

CoordinateReplyPacket*
MapProcessor::processCoordinateRequestPacket(const CoordinateRequestPacket* inPacket)
{
   uint32 startTime = TimeUtility::getCurrentMicroTime();
   uint32 mapID = inPacket->getMapID();
   uint32 closestItemID = MAX_UINT32;
   uint32 status = StringTable::OK;
   uint16 offset = 0x8000; // Default value
   uint64 dist = MAX_UINT64;
   uint16 posAngle = MAX_UINT16;
   uint16 negAngle = MAX_UINT16;

   // All is allowed for now. Will be trouble with ItemInfo otherwise.
   UserRightsMapInfo rights( mapID, ~MapRights() );
   
   mc2dbg4<< "   mapID = " << mapID << endl;
   
   GenericMap *curMap = m_mapHandler->getMap(mapID);

   int32 lat = inPacket->getLatitude();
   int32 lon = inPacket->getLongitude();
   uint32 outDataTypes = inPacket->getOutDataType();
   mc2dbg4 << "   outDataTypes = " << outDataTypes << endl;

   Item* curItem = NULL;
   if (curMap == NULL) {
      //curMap == NULL
      mc2dbg4 << "   curMap == NULL" << endl;
      status = StringTable::MAPNOTFOUND;
      
   } else if ( lat != MAX_INT32 ) {
      // OK
      mc2dbg4 << "   lat = " << lat << ", lon = " << lon << endl;
      
      DEBUG8(hashTable->dump());
      
      // Get the closest item
      int16 angle = inPacket->getAngle();
      mc2dbg4 << "MapProcessor: angle(in) = " << angle << endl;
 
      mc2dbg4 << "ANGLE = " << angle << ", getNbrItemTypes=" 
              << uint32(inPacket->getNbrItemTypes()) << ", type(0)=" 
              << uint32(inPacket->getItemType(0)) << endl;
      if ( (inPacket->getNbrItemTypes() == 1) &&
           (inPacket->getItemType(0) == ItemTypes::streetSegmentItem) &&
           (angle >= 0) && (angle <= 360)) {
         // Search only among SSI's --> use the angle 
         mc2dbg4 << "Using getBestStreetSegmentItem" << endl;
         uint32 tmpDist = 0;
         const Item* closestItem = getBestStreetSegmentItem(curMap,
                                                            lat, 
                                                            lon, 
                                                            angle,
                                                            tmpDist);
         if (closestItem != NULL) {
            dist = tmpDist;
            closestItemID = closestItem->getID();
         }
         mc2dbg << "TIME: getBestStreetSegmentItem in " 
                   << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0 
                   << " ms" << endl;
      }
      
      if (closestItemID == MAX_UINT32) {
         mc2dbg4 << "Using getClosestItem in hashTable" << endl;
         set<ItemTypes::itemType> allowedTypes;
         for (byte i=0; i<inPacket->getNbrItemTypes(); i++) {
           allowedTypes.insert( 
              ItemTypes::itemType(inPacket->getItemType(i)));
         }
         closestItemID = curMap->getClosestItemID(
            MC2Coordinate(lat, lon), dist,
            allowedTypes );
         // Convert dist into meters (from Squared mc2-scale)
         dist = uint64(::sqrt(dist) * GfxConstants::MC2SCALE_TO_METER);
         mc2dbg << "TIME: getClosestItemID in " 
                   << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0 
                   << " ms" << endl;
      }
      
      
      mc2dbg4 << "   closestItemID = " << mapID << "."
              << closestItemID 
              << ", dist = " << dist << endl;
      
      if ( (closestItemID == MAX_UINT32) || 
           ( (curItem = curMap->itemLookup(closestItemID)) == NULL )){
         status = StringTable::NOTFOUND;
      } else {
         // Calculate the offset
         mc2dbg4 << "   status = OK (" << status << ")" << endl;
         int tmp = curMap->getItemOffset(curItem, lat, lon);
         if (tmp >= 0) {
            offset = uint16(tmp);
            mc2dbg4 << "Offset set to " << offset << endl;
            
            // Calculate the angle
            if ((outDataTypes == 0) || (
               (outDataTypes & COORDINATE_PACKET_RESULT_ANGLE) != 0)) {
               GfxData* gfx = curItem->getGfxData();
               if (gfx != NULL) {
                  float64 angle = gfx->getAngle(offset);
                  mc2dbg4 << "Map: angle = " << angle << endl;
                  if (angle > 0) {
                     posAngle = (uint16) angle;
                     mc2dbg4 << "MapProcessor: posAngle = " << posAngle
                             << endl;
                     negAngle = (posAngle + 180) % 360;
                     mc2dbg4 << "MapProcessor: negAngle = " << negAngle
                             << endl;
                     mc2dbg2 << "   angles set to " << posAngle
                             << ", " << negAngle << endl;
                  }
               }
            }

         } else {
            mc2log << warn << "Error setting offset" << endl;
         }
         
      }
   } else if (lat == MAX_INT32) {
      // invalid latitude 
      mc2log << error << "MapProcessor, invalid latitude"<< endl;
      status = StringTable::UNKNOWN_POSITION;
   } else {
      // hashTable == NULL
      mc2log << error << "MapProcessor, hashTable == NULL"<< endl;
      status = StringTable::INTERNAL_ERROR_IN_MAP;
   }

   // The packet to return
   CoordinateReplyPacket* replyPacket = NULL;

   if ( status == StringTable::OK) {
   
      // Get the ID of the country, builtuparea and municipal
      StringTable::countryCode countryCode = curMap->getCountryCode();
      uint32 cityPartID    = MAX_UINT32;
      uint32 municipalID   = MAX_UINT32;
      uint32 buaID         = MAX_UINT32;
      
      // Save the Items, they can be used when getting the strings.
      Item* munItem = NULL;
      Item* buaItem = NULL;
      Item* districtItem = NULL;

      // Add the things if curItem != NULL and it is needed
      // by the outdatatypes.
      // If the outDataTypes wants strings it gets the ids too
      // since they are needed anyway.
      if ( curItem != NULL &&
           ( ( outDataTypes == 0 ) ||
             ( outDataTypes & COORDINATE_PACKET_RESULT_STRINGS ) ||
             ( outDataTypes & COORDINATE_PACKET_RESULT_IDS) ) ) {

         // Check if it has a index area
         if ( curMap->getIndexAreaFor( curItem ) == NULL ) {
            // Municipal
            munItem = curMap->getRegion(curItem, ItemTypes::municipalItem);
            if ( munItem != NULL )
               municipalID = munItem->getID();
            // BUA
            BuiltUpAreaItem* cityItem = item_cast<BuiltUpAreaItem*>
               ( curMap->getRegion( curItem, ItemTypes::builtUpAreaItem ) );
            if ( cityItem != NULL ) {
               buaItem = curMap->getRegion( curItem, ItemTypes::builtUpAreaItem );
               buaID = cityItem->getID();
               // District ( city part )
               uint32 i = 0;
               while ( ( districtItem == NULL ) &&
                       ( i < cityItem->getNbrItemsInGroup() ) ) {
                  Item* item =
                     curMap->itemLookup( cityItem->getItemNumber(i) );
                  if ( ( item != NULL ) &&
                       ( item->getItemType() == ItemTypes::cityPartItem ) &&
                       ( item->getGfxData() != NULL) &&
                       ( item->getGfxData()->insidePolygon(lat, lon) > 0) ) {
                     districtItem = item;
                  }
                  i++;
               }
            }
            if (districtItem != NULL) {
               cityPartID = districtItem->getID();
            }
         } else {
            Item* indexItem( curMap->getIndexAreaFor( curItem ) );
            while ( indexItem != NULL ) {
               switch( curMap->getIndexAreaOrder( indexItem->getID() ) ) {
                  case 7 : { // Post County
                     munItem = indexItem;
                     municipalID = munItem->getID();
                  }
                  break;
                  case 8 : { // Post Town
                     buaItem = indexItem;
                     buaID = buaItem->getID();
                  }
                  break;
                  case 9 : { // Post Loacality
                     districtItem = indexItem;
                     cityPartID = districtItem->getID();
                  }
                  break;
                  case 10 : { // Unambiguous special case something, Not used
                  }
                  break;
                  default : {
                  }
               }
               indexItem = curMap->getIndexAreaFor( indexItem );
            } // End while
            // If town(order 8) have same name as county(order 7) then
            // there is only order 7 no order 8.
            // Detecting this if there is an lower (9) order and higher (7)
            // but no town (8)
            if ( districtItem != NULL && buaItem == NULL && munItem != NULL ) {
               // Fill in from municipal
               buaItem = munItem;
               buaID = buaItem->getID();
               // Unset munItem? Will duplicate otherwise
               munItem = NULL;
               municipalID = MAX_UINT32;
            }
         }
      }
      
      replyPacket = new CoordinateReplyPacket(inPacket,
                                              status,
                                              mapID,
                                              closestItemID,
                                              offset,
                                              uint32(dist),
                                              posAngle,
                                              negAngle,
                                              countryCode,
                                              municipalID,
                                              buaID,
                                              cityPartID);
      
      LangTypes::language_t languageItem( LangTypes::
                                          language_t( inPacket->
                                                      getLanguage() ) );
      StringTable::languageCode languageCountry =
         ItemTypes::getLanguageTypeAsLanguageCode( languageItem );

      // Add strings if curItem != NULL
      if ((curItem != NULL) &&
          ( (outDataTypes == 0) || (
             (outDataTypes & COORDINATE_PACKET_RESULT_STRINGS) != 0))) {
         // Country
         const char* country = 
            StringTable::getStringToDisplayForCountry(
               countryCode,
               languageCountry);
        
         // Municipal
         const char* municipal = NULL;
         if (munItem != NULL) {
            municipal = curMap->getBestItemName(munItem, 
                                                languageItem);
         }
         
         // City
         const char* city= NULL;
         const char* district = NULL;
         if (buaItem != NULL) {
            city = curMap->getBestItemName(buaItem, 
                                           languageItem);
            
            // District (city part)
            if (districtItem != NULL) {
               district = curMap->getBestItemName(districtItem, 
                                                  languageItem);
            }
         }  
         
         // Name
         const char* name = curMap->getBestItemName(curItem, 
                                                    languageItem);
      
         // Add names to packet
         replyPacket->writeStrings(country, municipal, city, district, name);
         
         mc2dbg2 << "   Written into packet: " << country << ", " 
                 << municipal << ", " << city << ", " << district 
                 << ", " << name << endl;
      }
      // DEBUG
      DEBUG2(
         const int maxLen = 64;
         char cou[maxLen];
         char mun[maxLen];
         char bua[maxLen];
         char dis[maxLen];
         char nam[maxLen];
         replyPacket->getStrings(cou, mun, bua, dis, nam, maxLen);
         mc2dbg << "   Read from packet: " << cou << ", " << mun 
         << ", " << bua << ", " << dis << ", " << nam << endl;
         );
      } else {
         // status != StringTable::OK
         replyPacket = new CoordinateReplyPacket(inPacket,
                                              status,
                                              mapID,
                                              closestItemID,
                                              offset,
                                              uint32(dist));
   }

   mc2dbg2 << "   MapProcessor COORDINATEREPLY-packet created" 
           << "with status = "
           << StringTable::getString( ( StringTable::stringCode )status,
                                      StringTable::ENGLISH )
           << endl;

   return (replyPacket);
}

InfoCoordinateReplyPacket* 
MapProcessor::
processInfoCoordinateRequest(const InfoCoordinateRequestPacket* req ) 
{
   std::vector<InfoCoordinate> coordData;
   req->readData(coordData);
   std::vector<InfoCoordinateReplyData> replyData;
   for(std::vector<InfoCoordinate>::iterator it = coordData.begin();
       it != coordData.end(); ++it){
      uint32 itemID, distance, offset;
      bool dirFromZero;
      const bool result = 
         processInfoCoordinateRequest(req->getMapID(),
                                      it->coord.lat, it->coord.lon, it->angle,
                                      COORDINATE_PACKET_RESULT_ANGLE,
                                      itemID, distance, dirFromZero, offset);


      if ( result && it->angle < 361 ) {
         if ( !dirFromZero ){
            itemID |= 0x80000000;
            // offset = ?;
         }
         replyData.push_back(InfoCoordinateReplyData(req->getMapID(), itemID, 
                                                     MAX_UINT32, distance, 
                                                     offset));
      } else if( result && (it->angle == 32767) ) {
         replyData.push_back(InfoCoordinateReplyData(req->getMapID(),
                                                     itemID & 0x7fffffff,
                                                     itemID | 0x80000000,
                                                     distance, offset));
      } else {
         replyData.push_back(InfoCoordinateReplyData( MAX_UINT32, MAX_UINT32,
                                                      MAX_UINT32, MAX_UINT32,
                                                      MAX_UINT32));
      }
   }
   InfoCoordinateReplyPacket* answer = 
      new InfoCoordinateReplyPacket(req, StringTable::OK, 
                                    replyData);
   return answer;
}


bool
MapProcessor::processOmniInfoCoordinateRequest(uint32 mapID,
                                               int32 lat,
                                               int32 lon,
                                               uint32 angle,
                                               uint32 outDataTypes,
                                               uint32& itemID_0,
                                               uint32& itemID_1,
                                               uint32& dist,
                                               bool& dirFromZero,
                                               uint32& offset)
{
   uint32 closestItemID = MAX_UINT32;
   Item* closestItem = NULL;
   // All is allowed for now. Will be trouble with ItemInfo otherwise.
   UserRightsMapInfo rights( mapID, ~MapRights() );

   mc2dbg4 << "   mapID = " << mapID << endl;
   
   GenericMap *curMap = m_mapHandler->getMap(mapID);

   mc2dbg4 << "   outDataTypes = " << outDataTypes << endl;

   ItemTypes::entryrestriction_t restNode0 = ItemTypes::noRestrictions;
   ItemTypes::entryrestriction_t restNode1 = ItemTypes::noRestrictions;

   if (curMap == NULL) {
      //curMap == NULL
      mc2dbg4 << "   curMap == NULL" << endl;
      return false;

   } else if ( lat != MAX_INT32 ) {
      // OK
      mc2dbg4 << "   lat = " << lat << ", lon = " << lon << endl;

      DEBUG8(hashTable->dump());
      
      // Get the closest item
      mc2dbg4 << "MapProcessor: angle(in) = " << angle << endl;

      if ( angle == 32767 ) { //should always happen in this function
         // Search only among SSI's --> use the angle 
         uint32 tmpDist = 0;
         closestItem = getBestStreetSegmentItem(curMap,
                                                lat, 
                                                lon, 
                                                angle,
                                                tmpDist);
         if (closestItem != NULL) {
            dist = tmpDist;
            closestItemID = closestItem->getID();
            itemID_0 = closestItemID;
            
            // Get entry restrictions
            restNode0 = 
               static_cast<StreetSegmentItem*>(closestItem)
               ->getNode(0)->getEntryRestrictions();
            restNode1 = 
               static_cast<StreetSegmentItem*>(closestItem)
               ->getNode(1)->getEntryRestrictions();

            if ( (restNode0 != ItemTypes::noRestrictions) && 
                 (restNode0 != ItemTypes::noThroughfare) ) {
               itemID_0 |= 0x80000000;
            } else {
               itemID_0 &= 0x7fffffff;
            }

            mc2dbg2 << "1st Found item " << MC2HEX(itemID_0) << " dist " << dist << endl;

         }
      }
      if ( closestItemID == MAX_UINT32 ) {
         return false;
      } else {
         // Get offset & angle.
         int tmp = curMap->getItemOffset(closestItemID, lat, lon);
         GfxData* gfx = closestItem->getGfxData();
         
         if (tmp >= 0 && gfx != NULL) {
            offset = uint32(tmp);
            uint32 item_angle = (uint32)(gfx->getAngle(offset));
            int32 diff = item_angle - angle;
            mc2dbg8 << "item_angle: " << item_angle << endl;
            mc2dbg8 << "in_angle: " << angle << endl;
            mc2dbg8 << "diff: " << diff << endl;
            if(diff <= 90  &&  diff >= -90){
               dirFromZero = true;
            } else {
               dirFromZero = false;
               // we better change the offset to the other node.
               offset = closestItem->getLength() - offset;
            }

            if ( restNode0 == ItemTypes::noRestrictions && 
                 restNode1 == ItemTypes::noRestrictions ) {
               // No restrictions on segment
               itemID_1 = itemID_0;
               mc2dbg2 << "2nd Found SAME item " << MC2HEX(itemID_1) << " dist " << dist << endl;
            } else {
               uint32 rev_angle = item_angle;

               if ( (restNode0 != ItemTypes::noRestrictions) && 
                    (restNode0 != ItemTypes::noThroughfare) ) {
                  mc2dbg2 << "Found restriction on 1st segment node0." << endl;
                  rev_angle = item_angle;
               } else {
                  if (item_angle > 180) {
                     rev_angle -= 180;
                  } else {
                     rev_angle += 180;
                  }
               }

               mc2dbg4 << "rev_angle: " << rev_angle << endl;


               uint32 tmpDist = 0;
               closestItem = getBestStreetSegmentItem(curMap,
                                                      lat, 
                                                      lon, 
                                                      rev_angle,
                                                      tmpDist);
               if (closestItem != NULL) {
                  dist = tmpDist;
                  closestItemID = closestItem->getID();
                  itemID_1 = closestItemID;

                  // Get entry restrictions
                  restNode0 = 
                     static_cast<StreetSegmentItem*>(closestItem)
                     ->getNode(0)->getEntryRestrictions();
                  
                  if ( (restNode0 != ItemTypes::noRestrictions) && 
                       (restNode0 != ItemTypes::noThroughfare) ) {
                     itemID_1 |= 0x80000000;
                  } else {
                     itemID_1 &= 0x7fffffff;
                  }
                  
                  mc2dbg2 << "2nd Found item " << MC2HEX(itemID_1) << " dist " << dist << endl;

               }
            }
            
         } else {
             mc2log << warn
                    << "[MProc]Error checking node from angle " << endl;
         }
      }
   }
   return true;
}


bool
MapProcessor::processInfoCoordinateRequest(uint32 mapID,
                                           int32 lat,
                                           int32 lon,
                                           uint32 angle,
                                           uint32 outDataTypes,
                                           uint32& itemID,
                                           uint32& dist,
                                           bool& dirFromZero,
                                           uint32& offset)
{
   uint32 closestItemID = MAX_UINT32;
   const Item* closestItem = NULL;
   // All is allowed for now. Will be trouble with ItemInfo otherwise.
   UserRightsMapInfo rights( mapID, ~MapRights() );

   mc2dbg4 << "   mapID = " << mapID << endl;
   
   GenericMap *curMap = m_mapHandler->getMap(mapID);

   mc2dbg4 << "   outDataTypes = " << outDataTypes << endl;

   if (curMap == NULL) {
      //curMap == NULL
      mc2dbg4 << "   curMap == NULL" << endl;
      return false;
      
   } else if ( lat != MAX_INT32 ) {
      // OK
      mc2dbg4 << "   lat = " << lat << ", lon = " << lon << endl;
      
      DEBUG8(hashTable->dump());
      
      // Get the closest item
      mc2dbg4 << "MapProcessor: angle(in) = " << angle << endl;
 
      if ( ((angle >= 0) && (angle <= 360)) || angle == 32767 ) {
         // Search only among SSI's --> use the angle 
         uint32 tmpDist = 0;
         closestItem = getBestStreetSegmentItem(curMap,
                                                lat, 
                                                lon, 
                                                angle,
                                                tmpDist);
         if (closestItem != NULL) {
            dist = tmpDist;
            closestItemID = closestItem->getID();
            itemID = closestItemID;
            mc2dbg4 << "Found item " << closestItemID << " dist " << dist << endl;
         }
      }
      if ( closestItemID == MAX_UINT32 ) {
         return false;
      } else {
         // Get offset & angle.
         int tmp = curMap->getItemOffset(closestItemID, lat, lon);
         GfxData* gfx = closestItem->getGfxData();
         
         if (tmp >= 0 && gfx != NULL) {
            offset = uint32(tmp);
            uint32 item_angle = (uint32)(gfx->getAngle(offset));
            int32 diff = item_angle - angle;
            if(diff <= 90  &&  diff >= -90){
               dirFromZero = true;
            } else {
               dirFromZero = false;
               // we better change the offset to the other node.
               offset = closestItem->getLength() - offset;
            }
            
            
         } else {
             mc2log << warn
                    << "[MProc]Error checking node from angle " << endl;
         }
      }
   }
   return true;
}

StreetSegmentItemReplyPacket*
MapProcessor::processStreetSegmentItemRequestPacket(
                                 const StreetSegmentItemRequestPacket* packet)
{
   uint32 mapID = packet->getMapID();
   GenericMap* theMap = m_mapHandler->getMap(mapID);
   if( theMap != NULL ) {
      uint32 index = packet->getIndex();
      int32 latitude = packet->getLatitude();
      int32 longitude = packet->getLongitude();
      TrafficDataTypes::direction direction = packet->getDirection();
      uint64 distance = MAX_UINT64;

      set<ItemTypes::itemType> allowedTypes;
      allowedTypes.insert(ItemTypes::streetSegmentItem);
      MC2Coordinate mc2Coord(latitude, longitude);
      
      uint32 closestItemID =
         theMap->getClosestItemID(mc2Coord,
                                  distance,
                                  allowedTypes);
      Item* item = theMap->itemLookup(closestItemID);
      MC2_ASSERT( item );    // Added to trace crash
      MC2_ASSERT( item_cast<const RouteableItem*>(item) );
      GfxData* gfxData = item->getGfxData();
      MC2_ASSERT( gfxData ); // Added to trace crash
      
      int32 lat, lon;
      uint16 firstAngle, secondAngle;
      uint32 firstNodeID, secondNodeID;
      if( direction != TrafficDataTypes::NoDirection ) {
         gfxData->getCoordinate(uint16(MAX_UINT16/2),
                                lat, lon);

         Node* node0 = ((RouteableItem*) item)->getNode(0);
         MC2_ASSERT( node0 ); // Added to trace crash
         uint32 nodeID0 = node0->getNodeID();
         int32 lat0, lon0;
         theMap->getNodeCoordinates(nodeID0, lat0, lon0);
         Node* node1 = ((RouteableItem*) item)->getNode(1);
         MC2_ASSERT( node1 );  // Added to trace crash
         uint32 nodeID1 = node1->getNodeID();
         int32 lat1, lon1;
         theMap->getNodeCoordinates(nodeID1, lat1, lon1);

         Node* firstNode = NULL;
         uint16 addToAngle = 0;
         if( lat0 > lat1 ) {
            if( direction == TrafficDataTypes::Positive ) {
               firstNode = ((RouteableItem*) item)->getNode(1);
               addToAngle = 180;
            } else {
               firstNode = ((RouteableItem*) item)->getNode(0);
            }
         } else if( lat1 > lat0 ) {
            if( direction == TrafficDataTypes::Positive ) {
               firstNode = ((RouteableItem*) item)->getNode(0);
            } else {
               firstNode = ((RouteableItem*) item)->getNode(1);
               addToAngle = 180;
            }
         } else if( lon1 > lon0 ) {
            if( direction ==  TrafficDataTypes::Positive ) {
               firstNode = ((RouteableItem*) item)->getNode(0);
            } else {
               firstNode = ((RouteableItem*) item)->getNode(1);
               addToAngle = 180;
            }
         } else if( lon0 > lon1 ) {
            if( direction == TrafficDataTypes::Positive ) {
               firstNode = ((RouteableItem*) item)->getNode(1);
               addToAngle = 180;
            } else {
               firstNode = ((RouteableItem*) item)->getNode(0);
            }
         } else {
            firstNode = ((RouteableItem*) item)->getNode(0);
         }
         firstNodeID = firstNode->getNodeID();
         float64 angle = gfxData->getAngle(MAX_UINT16/2);
         firstAngle = (uint16(angle) + addToAngle) % 360;
         secondNodeID = MAX_UINT32;
         secondAngle = MAX_UINT16;
      } else {
         Node* firstNode = ((RouteableItem*) item)->getNode(0);
         firstNodeID = firstNode->getNodeID();
         
         Node* secondNode = ((RouteableItem*) item)->getNode(1);
         secondNodeID = secondNode->getNodeID();
         
         float64 angle = gfxData->getAngle(MAX_UINT16/2);
         firstAngle = uint16(angle);
         secondAngle = (firstAngle+180) % 360;
         
         gfxData->getCoordinate(uint16(MAX_UINT16/2),
                                lat, lon);
      }
      StreetSegmentItemReplyPacket* replyPacket =
         new StreetSegmentItemReplyPacket(packet,
                                          StringTable::OK,
                                          index,
                                          distance,
                                          mapID,
                                          lat,
                                          lon,
                                          firstNodeID,
                                          secondNodeID,
                                          firstAngle,
                                          secondAngle);
      return replyPacket;
   } else {
      StreetSegmentItemReplyPacket* replyPacket =
         new StreetSegmentItemReplyPacket(packet,
                                          StringTable::NOTOK,
                                          MAX_UINT32,
                                          MAX_UINT32,
                                          MAX_UINT32,
                                          MAX_INT32,
                                          MAX_INT32,
                                          MAX_UINT32,
                                          MAX_UINT32,
                                          MAX_UINT32,
                                          MAX_UINT32);
      return replyPacket;                                          
   }
}



Item*
MapProcessor::getBestStreetSegmentItem(GenericMap* map,
                                       int32 lat, int32 lon, 
                                       uint16 angle,
                                       uint32 &dist)
{
   // Constants used in this method
   const uint32 SEARCH_RADIUS = 50;    // meters

   // Find SSI:s in the map
   set<Item*> candidateItems;
   {
      set<ItemTypes::itemType> allowedTypes;
      allowedTypes.insert( ItemTypes::streetSegmentItem );
      map->getItemsWithinBBox( candidateItems,
                               MC2BoundingBox(MC2Coordinate(lat, lon),
                                              SEARCH_RADIUS),
                               allowedTypes );
      if ( candidateItems.empty() ) {
         return NULL;
      }
   }
#if 0
   // Old code
   // Set the hashtable do that only SSI's are found
   MapHashTable* hashTable = map->getHashTable();
   hashTable->clearAllowedItemTypes();
   hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);

   // Get all items within SEARCH_RADIUS
   bool shouldKill = false;
   Vector* candidateIDs = hashTable->getAllWithinRadius_meter(
                                                lon, 
                                                lat, 
                                                SEARCH_RADIUS,
                                                shouldKill,
                                                rights);
   
   // Check if something was found.
   if ( candidateIDs == NULL )
      return NULL;

   int candidateSize = candidateIDs->getSize();
   
   for (int i = 0; i< candidateSize; ++i) {
#endif
   // Add the candidates to an array with ssiNotice_t's
   vector<ssiNotice_t> notices;
   notices.reserve( candidateItems.size() );
   for ( set<Item*>::const_iterator it = candidateItems.begin();
         it != candidateItems.end();
         ++it ) {
      //Item* item = map->itemLookup(candidateIDs->getElementAt(i));
      Item* item = *it;
      if (item != NULL) {
         ssiNotice_t notice;
         notice.m_item = item;
         notice.m_score = 0;
         const GfxData* gfx = item->getGfxData();
         notice.m_dist = uint32(::sqrt(gfx->squareDistToLine(lat, lon)));
         int tmpOffset = map->getItemOffset(item, lat, lon);
         if (tmpOffset >= 0) {
            notice.m_offset = uint16(tmpOffset);
         } else {
            mc2log << warn
                   << "MapProcessor::getBestStreetSegmentItem INVALID offset" 
                    << endl;
            notice.m_offset = 0;
         }
         // The angle when driving in positive direction
         notice.m_angle = uint16(gfx->getAngle(notice.m_offset));

         notice.m_angleDiff = notice.m_angle - angle;
         if (notice.m_angleDiff < -180) {
            notice.m_angleDiff += 360;
         } else if (notice.m_angleDiff > 180) {
            notice.m_angleDiff -= 360;
         }

         // Get entry restrictions
         ItemTypes::entryrestriction_t restNode0 = 
            static_cast<StreetSegmentItem*>(item)
               ->getNode(0)->getEntryRestrictions();
         ItemTypes::entryrestriction_t restNode1 = 
            static_cast<StreetSegmentItem*>(item)
               ->getNode(1)->getEntryRestrictions();
         
         // Calculate the score
         notice.m_score = notice.m_dist;
         if ( angle == 32767 ) {
            mc2dbg4 << "OmniAngle Node: " 
                    << notice.m_angle << " : " 
                    << notice.m_dist << " : " 
                    << notice.m_angleDiff << " : " 
                    << MC2HEX(item->getID()) << endl;

            notice.m_score = notice.m_dist;
         } else if ( (abs(notice.m_angleDiff) > 135) &&
              (restNode1 != ItemTypes::noRestrictions) && 
	      (restNode1 != ItemTypes::noThroughfare) ) {
            // Heading more or less backwards that not is allowed
            //notice.m_score *= 4;
            notice.m_score += abs(notice.m_angleDiff) / 2;
            mc2dbg4 << "   Backwards, not allowed, angleDiff=" 
                    << notice.m_angleDiff << endl;
         } else if ( (abs(notice.m_angleDiff) < 45) &&
                     (restNode0 != ItemTypes::noRestrictions) && 
		     (restNode0 != ItemTypes::noThroughfare) ) {
            // Heading more or less forward that not is allowed
            //notice.m_score *= 4;
            notice.m_score += (180 - abs(notice.m_angleDiff)) / 2;
            mc2dbg4 << "   Forwards, not allowed, angleDiff=" 
                    << notice.m_angleDiff << endl;
         } else if ( (abs(notice.m_angleDiff) >= 45) && 
                     (abs(notice.m_angleDiff) <= 135)) {
            // Heading more or less left/right
            //notice.m_score *= 2;
            notice.m_score += abs(notice.m_angleDiff) / 2;
            mc2dbg4 << "   About 90 deg wrong, angleDiff=" 
                    << notice.m_angleDiff << endl;
         }
         mc2dbg4 << "score set to = " << notice.m_score << " : " 
                 << item->getID() << endl;
         
         notices.push_back(notice);
      }
   }

#if 0
   // Old code
   // Delete the array with IDs
   if (shouldKill) {
      delete candidateIDs;
   }
   candidateIDs = NULL;
#endif

   // Sort the notice-array
   sort(notices.begin(), notices.end(), LessNotice());

   if ( false ) {
      // Dump the value of the notices
      uint32 cardinal = 0;
      for (vector<ssiNotice_t>::const_iterator i=notices.begin(); 
           i<notices.end(); i++) {
         mc2dbg << cardinal++ << " score=" << i->m_score << ", ID=" 
                << i->m_item->getID() << "." << i->m_offset 
                << ", angle=" << i->m_angle << ", dAngle=" 
                << i->m_angleDiff << ", dist=" << i->m_dist << ", "
                << map->getFirstItemName(i->m_item) << endl;
      }
   }

   // Return the first item in the notice-array.
   if (notices.begin() != notices.end()) {
      dist = notices.begin()->m_dist;
      return (notices.begin()->m_item);
   } else {
      return (NULL);
   }
   // We never get here but g++ complians so here is an extra return
   return NULL;
}


IDTranslationReplyPacket* 
MapProcessor::processIDTranslationRequestPacket( 
                        const IDTranslationRequestPacket* p )
{
   uint32 mapID = p->getMapID();
   bool ok = true;
   if ( ! MapBits::isOverviewMap( mapID ) ) {
      ok = false;
   }

   OverviewMap* theMap = 
      static_cast<OverviewMap*> ( m_mapHandler->getMap( mapID ) );

   if ( (! ok) || ( theMap == NULL ) ) {
      mc2log << warn 
             << "processIDTranslationRequestPacket: "
             << "Could not get overview map"
             << endl;
      mc2dbg1 << "   MapID = " << mapID << endl;
      return ( new IDTranslationReplyPacket( 
                                       p, StringTable::NOTFOUND ) );
   }

   IDPairVector_t innodes;
   IDPairVector_t outnodes;
   p->getAllNodes(innodes);
   outnodes.reserve( innodes.size() );

   uint32 lookedUpMapID, lookedUpNodeID;
   
   if ( p->getTranslateToLower() ) {
      // Translate to lower
      for ( IDPairVector_t::const_iterator it = innodes.begin(); 
            it != innodes.end(); ++it ) {
         
         if ( ! theMap->lookupNodeID( it->second, lookedUpMapID, 
                                      lookedUpNodeID ) ) {
            lookedUpMapID = lookedUpNodeID = MAX_UINT32;
         }
         outnodes.push_back( IDPair_t( lookedUpMapID, lookedUpNodeID ) ); 
      }
      
   } else {
      // Translate to higher
      for ( IDPairVector_t::const_iterator it = innodes.begin(); 
            it != innodes.end(); ++it ) {
         
         lookedUpNodeID = 
            theMap->reverseLookupNodeID( it->first, it->second );
         if ( lookedUpNodeID == MAX_UINT32 ) {
            lookedUpMapID = MAX_UINT32;
         } else {
            lookedUpMapID = mapID;
         }
         outnodes.push_back( IDPair_t( lookedUpMapID, lookedUpNodeID ) ); 
      }

   }

   return new IDTranslationReplyPacket( p, StringTable::OK, outnodes );

}

// - Maybe the searchMatch functions should be moved out of MapProcessor?

inline VanillaRegionMatch*
MapProcessor::createRegionMatch(uint32 itemID,
                                const GenericMap& theMap,
                                LangTypes::language_t lang) const
{
   const Item* item = theMap.itemLookup(itemID);
   if ( item == NULL ) {
      return NULL;
   }

   // FIXME: Remove this (SearchMatchPoints) from constuctor
   const SearchMatchPoints points;
   
   VanillaRegionMatch* regionMatch =
      new VanillaRegionMatch(theMap.getBestItemName(item, lang),
                             0, // nameinfo
                             theMap.getMapID(),
                             item->getID(),
                      ItemTypes::itemTypeToSearchType(item->getItemType()),
                             points, // Not important, really.
                             0, // Source
                             theMap.getBestItemName(item, lang),
                             0, // Location
                             0, // Restrictions
                             item->getItemType(),
                             0); // ItemSubtype - FIXME.
   
   return regionMatch;
}

inline void
MapProcessor::addRegionsToMatch(SearchMatch* match,
                                uint32 idToAddFrom,
                                const GenericMap& theMap,
                                LangTypes::language_t reqLang,
                                uint32 allowedSearchTypes,
                                int level) const
{
   // Example
   // Our item is in A, B, C and D
   // A and B are in C and C is in D
   // Foreach region
   //   if region is inside other region -> remove _other_ region
   //   or if no region is inside a region -> keep region
   // After this: call this function for every remaining region.

   // FIXME: Move this functionality to the SearchUnitBuilder.
   // WARNING: There is code similar to this in SearchUnitBuilder. Change it
   //          too if you do something useful here.
 

   const Item* item = theMap.itemLookup(idToAddFrom);
   if ( item == NULL ) {
      // Error?
      return;
   }

   set<uint32> regionIDs;
   
   uint32 nbrGroups = item->getNbrGroups();
   for ( uint32 i = 0; i < nbrGroups; ++i ) {
      const Item* region = theMap.itemLookup(item->getGroup(i));
      if ( region == NULL ) {
         continue;
      }

      switch ( region->getItemType() ) {
         // ItemTypes to ignore.
         case ItemTypes::streetItem:
         case ItemTypes::categoryItem:
            break;
         default:
            regionIDs.insert(item->getGroup(i));
            break;
      }
   }

   for ( set<uint32>::iterator it = regionIDs.begin();
         it != regionIDs.end();
         /* inc later */ ) {
      const Item* item = theMap.itemLookup(*it);
      // I hope the map doesn't lie.
      MC2_ASSERT(item != NULL);
      
      if ( ItemTypes::itemTypeToSearchType(item->getItemType()) &
           allowedSearchTypes ) {
         // ok
         ++it;
      } else {
         // wrong type
         mc2dbg2 << "[MProc]: Removing region of type "
                << hex << ItemTypes::itemTypeToSearchType(item->getItemType())
                << dec << endl;
         regionIDs.erase(it++);
      }
   }
   regionIDs.erase( item->getID() );
   
   mc2dbg2 << "[MProc]: Number of regions for "
          << theMap.getBestItemName(item, reqLang)
          << " = " << regionIDs.size()
          << endl;
   
   for ( set<uint32>::const_iterator it = regionIDs.begin();
         it != regionIDs.end();
         ++it ) {
      bool noRegionInside = true;
      for ( set<uint32>::const_iterator jt = regionIDs.begin();
            jt != regionIDs.end();
            ++jt ) {
         if ( it == jt ) continue;

         const Item* item_j = theMap.itemLookup(*jt);

         if ( item_j == NULL ) {
            continue;
         }

         // Check if j is inside i
         bool j_is_inside_i = item_j->memberOfGroup(*it);

         // Cut when location is removed       
         j_is_inside_i = j_is_inside_i ||
            ( theMap.getRegionID( item_j, ItemTypes::municipalItem ) == 
              *it) ||
            ( theMap.getRegionID( item_j, ItemTypes::builtUpAreaItem ) == 
              *it ) ||
            ( theMap.getCityPartID((Item*)item_j) == *it );
         // end cut
                  
         if ( j_is_inside_i ) {
            // A region was inside i
            noRegionInside = false;
            break;
         }
      } // for jt

      if ( noRegionInside ) {
         // No other region was inside this region, means lowest level.
         VanillaRegionMatch* regionMatch = createRegionMatch(*it,
                                                             theMap,
                                                             reqLang);
         if ( regionMatch != NULL ) {
            mc2dbg2 << "[MapProc]: Created region " << regionMatch->getName()
                    << " at level = " << level << endl;
         
            // Add the regions to that match too.
            addRegionsToMatch(regionMatch, regionMatch->getItemID(),
                              theMap, reqLang, allowedSearchTypes, level + 1);
         
            // Add our region to the original match.
            match->addRegion(regionMatch, true);
         }
      } else {
         mc2dbg2 << "[MapProc]: Will not add "
                << theMap.getBestItemName(theMap.itemLookup(*it), reqLang)
                << endl;
      }
   }
}


/**
 *   Tests if the nbr is inside the interval [limitA, limitB].
 *   @param nbr     The number to test.
 *   @param limitA  One of the limits.
 *   @param limitB  The other limit.
 *   @return True if inside, false if outside.
 */
static inline bool inInterval( uint32 nbr,
                               uint32 limitA,
                               uint32 limitB )
{
   bool result = false;
   if (((nbr>=limitA) && (nbr<=limitB)) ||
       ((nbr>=limitB) && (nbr<=limitA)))
   {
      result = true;
   }
   return result;
}



/**
 *   Check if a given housenumber is located on a street.
 *   @param streetNumber  The streetnumber to attempt to match.
 *   @param start         The lower streetnumber of the side 
 *                        to test.
 *   @param stop          The higher streetnumber of the side 
 *                        to test
 *   @param closestNumber The closest number found of the 
 *                        available ones.
 *   @param closestError  In/out parameter. The error in the 
 *                        closest number found so far.
 *   @param closestStart  Outparameter that is updated if 
 *                        closestnumber is closer to streetnumber 
 *                        than any of the previous ones,
 *                        -> closestStart = start.
 *   @param closestEnd    Outparameter that is updated if 
 *                        closestnumber is closer to streetnumber 
 *                        than any of the previous ones, closestEnd 
 *                        = end.
 *   @return True if a number was matched.
 */
static inline bool checkNumber( uint32 streetNumber,
                                uint32 start,
                                uint32 stop,
                                uint32 &closestNumber,
                                uint32 &closestError,
                                uint32 &closestStart,
                                uint32 &closestEnd )
{
   bool result = false;
   DEBUG8(
      cerr << streetNumber << " streetNumber\n"
      << start << " start\n"
      << stop << " stop\n"
      << closestNumber << " closestNumber\n"
      << closestError << " closestError\n"
      << closestStart << " closestStart\n"
      << closestEnd << " closestEnd\n" << endl;
      );
   if ( inInterval(streetNumber,
                   start,
                   stop) )
   {
      // match found.
      closestNumber = streetNumber;
      closestError = 0;
      closestStart = start;
      closestEnd   = stop;
      result = true;
   } else {
      if
         ((uint32) abs(int(streetNumber)-int(start)) < closestError)
      {
         closestNumber = start;
         closestError = abs(int(streetNumber) -
                            int(start));
         closestStart = start;
         closestEnd   = stop;
         result = true;
      }
      if ((uint32) abs(int(streetNumber)-int(stop)) < closestError)
      {
         closestNumber = stop;
         closestError = abs(int(streetNumber) -
                            int(stop));
         closestStart = start;
         closestEnd   = stop;
         result = true;
      }
   }
   // otherwise: don't change the VAR-parameters.
   return result;
}


/**
 *   Calculates the offset to a street/house number on a street 
 *   segment.
 *
 *   @param number  The number of which the offset is to be 
 *                  calculated. If the number is outside the 
 *                  limits of the street the offset will be 0 
 *                  or MAX_UINT16.
 *   @param closestNumber 
 *                  The number closest to the $number$ that 
 *                  existed on the street. Outparam only 
 *                  (in-value ignored).
 *   @param leftSideStart 
 *                  The closest number on your left side when 
 *                  you stand at offset 0 facing the maximum 
 *                  offset.
 *   @param leftSideEnd   
 *                  The number farthest away on your left side.
 *   @param rightSideStart 
 *                  The closest number on your right side
 *   @param rightSideEnd   
 *                  The number furthest away on your right side.
 *   @param streetNumberType 
 *                  The type of the numbering on the street as 
 *                  defined in ItemTypes.
 *   @param side The side of the number found.
 *   @return The offset as uint16.
 */

static inline uint16 calculateOffset( uint32 number,
                                      uint32& closestNumber,
                                      uint32 leftSideStart,
                                      uint32 leftSideEnd,
                                      uint32 rightSideStart,
                                      uint32 rightSideEnd,
                                      uint32 streetNumberType,
                                      SearchTypes::side_t& closestSide)
{
   // figure out which sides we should check
   bool leftSide = false;
   bool rightSide = false;
   int stepLength = 2;
   switch (streetNumberType) {
      // this ordering of the types is supposed to give the fewest
      // average number of comparisons.
      case ItemTypes::leftEvenStreetNumbers:
         if ( Math::odd( number) ) {
            rightSide = true;
         } else {
            leftSide = true;
         }
         stepLength = 2;
         break;
      case ItemTypes::leftOddStreetNumbers:
         if ( Math::odd( number) ) {
            leftSide = true;
         } else {
            rightSide = true;
         }
         stepLength = 2;
         break;
      case ItemTypes::noStreetNumbers:
/*         // for some reason, this may not be true.
         if ((leftSideStart != 0) && (leftSideEnd != 0)) {
            leftSide = true;
         }
         if ((rightSideStart != 0) && (rightSideEnd != 0)) {
            rightSide = true;
         }
         if ((!leftSide) && (!rightSide)) {*/
            closestNumber = 0; // 0xfffffffe
//         }
         break;
      case ItemTypes::mixedStreetNumbers:
         leftSide = true;
         rightSide = true;
         stepLength = 1;
         break;
      default:
         DEBUG1(MC2WARNING2("Unsupported streetNumberType: ", {
            cerr << (int) streetNumberType << endl;
         }););
         break;
   }
   // now we know which sides to check.
   // Does not assume streetnumbers are 'sorted', i.e. that start<end...
   
   uint32 closestStart = 0;
   uint32 closestEnd = 0;
   uint32 closestError = MAX_UINT32;
   if (leftSide) {
      if ( checkNumber( number,
                        leftSideStart,
                        leftSideEnd,
                        closestNumber, closestError,
                        closestStart, closestEnd ) )
      {
         closestSide = SearchTypes::left_side;
      }
   }
   if (rightSide) {
      if ( checkNumber( number,
                        rightSideStart,
                        rightSideEnd,
                        closestNumber, closestError,
                        closestStart, closestEnd ) )
      {
         closestSide = SearchTypes::right_side;
      }
   }
   
   // return answer
   // interpolate number position:
   int sign = 1;
   if (closestStart > closestEnd) { sign = -1; }
   
   uint16 segmentOffset = 0x7fff;
   if (closestEnd != closestStart) {
      segmentOffset =
         ( (MAX_UINT16 * ((int)closestNumber - (int)closestStart + sign)) /
           ( 2*((int)closestEnd - (int)closestStart) / (int)stepLength +
             (int)stepLength*(int)sign ));
			// The street segment is assumed to be l units long.
			// Place the houses (all assumed to be of the same size)
			// evenly along the segment, with the house number locations
			// in the middle of the houses. This means that the first
			// house number is a short distance from one end of the segment,
			// and the last house number is a short distance from the other
			// end of the segment.
			// The segment contains (closestEnd - closestStart)/stepLength + 1
			// numbers (stepLength = 2 if there are only even or only odd
			// numbers on the current side of the street segment, otherwise
			// stepLength = 1).
			// (house number - closestStart + 1) /
			// (2*the amount of street numbers)
			// gives the position of the house number on the street segment
			// as a fraction of the length of the street segment.
			// If this fraction is multiplied by the length of the street segment
			// l (currently defined to be =0xffff), the result is the
			// segmentoffset.
   }

   if (segmentOffset == 0) {
      MC2WARNING2("Segment offset was 0!",
                  cerr
                  << closestNumber << " closestNumber\n"
                  << closestStart << " closestStart\n"
                  << closestEnd << " closestEnd\n"
                  << sign << " sign\n"
                  << stepLength << " stepLength\n"
                  << endl; );
   }
   
   return segmentOffset;
}
 

inline void
MapProcessor::setBestHouseNbr(VanillaStreetMatch* streetMatch,
                              const GenericMap& theMap,
                              LangTypes::language_t reqLang,
                              uint32 allowedSearchTypes) const
{
   // Find the correct streetsegment
   // The SearchModule will have stored the number used
   // when searching in the match.
   int streetNumber = streetMatch->getStreetNbr();

   if ( streetNumber == 0 ) {
      mc2log << warn << "[MapProc]: setBestHouseNbr: Streets with number 0 "
         "should not be sent here" << endl;
      return;
   }

   // Get the real street.
   const StreetItem* theStreet =
      item_cast<const StreetItem*>
      (theMap.itemLookup(streetMatch->getItemID()));

   if ( theStreet == NULL ) {
      mc2log << error << "[MapProc]: setBestHouseNbr - street "
             << streetMatch->getID() << " not found" << endl;
      return;
   }
   
   // The following is translated from StringSearchUnit.
   
   // Find the street segment that has a
   // street number closest to the requested street number(if it
   // is requested).
   uint32 closestNumber            = 0;
   uint32 closestError             = MAX_UINT32;
   uint16 closestOffset            = MAX_INT16;
   uint32 streetSegmentItemID      = MAX_UINT32;
   SearchTypes::side_t closestSide = SearchTypes::undefined_side;
   const StreetSegmentItem* closestSegment = NULL;
   const StreetSegmentItem* bestStreetSegment = NULL;
   
   // Loop over the segments
   int nbrItemsInGroup = theStreet->getNbrItemsInGroup();
   for( int i = 0; i < nbrItemsInGroup; ++i ) {
      // Get the segment
      const StreetSegmentItem* streetSegment =
         item_cast<StreetSegmentItem*>
         (theMap.itemLookup(theStreet->getItemNumber(i)));
      // Check the segment
      if ( streetSegment == NULL ) {
         mc2dbg << "[MProc]: Not ssi in street "
                << streetMatch->getID() << endl;
         continue;
      }

      // No numbers.
      if ( streetSegment->getStreetNumberType() ==
              ItemTypes::noStreetNumbers) {
         continue;
      }

      uint32 thisNumber = 0;
      SearchTypes::side_t thisSide = SearchTypes::undefined_side;
      uint16 thisOffset = calculateOffset(
         streetNumber,
         thisNumber,
         theMap.getStreetLeftSideNbrStart( *streetSegment ),
         theMap.getStreetLeftSideNbrEnd( *streetSegment ),
         theMap.getStreetRightSideNbrStart( *streetSegment ),
         theMap.getStreetRightSideNbrEnd( *streetSegment ),
         streetSegment->getStreetNumberType(),
         thisSide);

      uint32 thisError = abs(int(thisNumber - streetNumber));
      /*         
      mc2dbg4 << "[MapProc]: Left side:  "
              << streetSegment->getLeftSideNbrStart() << " - "
              << streetSegment->getLeftSideNbrEnd() << endl
              << "[MapProc]: Right side: "
              << streetSegment->getRightSideNbrStart() << " - "
              << streetSegment->getRightSideNbrEnd() << endl
              << "[MapProc]: StreetNumberType: "
              << (int) streetSegment->getStreetNumberType() << endl
              << "[MapProc]: StreetSegmentID 0x" << hex
              << streetSegment->getID()
              << dec
              << endl
              << "[MapProc]: " << thisNumber << " thisNumber\n"
              << thisOffset << " thisOffset\n"
              << thisError << " this error\n"
              << (uint16) thisSide << " thisSide"
              << endl;
   
      */
      if ( thisError < closestError ) {
         closestNumber = thisNumber;
         closestError = thisError;
         closestSegment = streetSegment;
         closestOffset = thisOffset;
         closestSide = thisSide;
         mc2dbg4 << "[MapProc]: closestNumber: " << closestNumber
                 << "\n[MapProc]: closestError: " << closestError
                 << "\n[MapProc]: closestSegment: " << closestSegment
                 << "\n[MapProc]: closestOffset: " << closestOffset
                 << "\n[MapProc]: closestSide: " << uint16(closestSide)
                 << endl;
         streetSegmentItemID = streetSegment->getID();
         bestStreetSegment = streetSegment;
      }
   }      

   // Now we should have found the best number.
   if ( bestStreetSegment != NULL ) {
      streetMatch->setStreetSegmentID( streetSegmentItemID );
      streetMatch->setOffset(closestOffset);
      streetMatch->setSide(closestSide);
      streetMatch->setStreetNbr(closestNumber);
      MC2Coordinate coordOnStreet;
      const GfxData* gfxData = bestStreetSegment->getGfxData();
      if ( gfxData != NULL ) {
         gfxData->getCoordinate(closestOffset,
                                coordOnStreet.lat,
                                coordOnStreet.lon);
      }
      streetMatch->setCoords(coordOnStreet);
   } else {
      // Set invalid values.
      streetMatch->setStreetSegmentID( MAX_UINT32 );
      streetMatch->setOffset(closestOffset);
      streetMatch->setSide(closestSide);
      streetMatch->setStreetNbr(0);
   }
   
   // Change the regions of the street to the ones of the
   // segment. Jubbit.
   if ( streetMatch->getStreetSegmentID() != MAX_UINT32 ) {
      addRegionsToMatch(streetMatch, streetMatch->getStreetSegmentID(),
                        theMap, reqLang, allowedSearchTypes);
   }
}

MatchInfoReplyPacket*
MapProcessor::processMatchInfoRequest(const MatchInfoRequestPacket* req) const
{
   vector<VanillaStreetMatch*> streets;
   req->getStreetMatches(streets);
   LangTypes::language_t reqLang = req->getRequestedLanguage();
   const uint32 allowedRegionTypes = req->getRegionSearchTypes();

   mc2dbg2 << "[MapProc]: processMatchInfoRequest req reg = "
          << hex << allowedRegionTypes << dec << endl;
   
   const GenericMap* curMap = m_mapHandler->getMap(req->getMapID());
   // The Map should never be NULL. Other parts of the processor
   // should handle that.
   MC2_ASSERT( curMap );
   
   for( vector<VanillaStreetMatch*>::iterator it = streets.begin();
        it != streets.end();
        ++it ) {
      // Set house numbers and regions
      setBestHouseNbr(*it, *curMap, reqLang, allowedRegionTypes);
      (*it)->updateLocationName(allowedRegionTypes);
   }

   /* Put the streets that have been filled in into the reply */
   MatchInfoReplyPacket* replyPacket =
      new MatchInfoReplyPacket(req, streets);

   // Clean up
   STLUtility::deleteValues( streets );
   // Done
   return replyPacket;
}

QueueStartReplyPacket*
MapProcessor::processQueueStartPacket(const QueueStartRequestPacket* req) const
{
   QueueStartReplyPacket* replyPacket = new QueueStartReplyPacket(req);
   const GenericMap* curMap = m_mapHandler->getMap(req->getMapID());
   MC2_ASSERT( curMap );
   
   BoundrySegmentsVector* bSegV = curMap->getBoundrySegments();
   
   uint32 nodeID     = req->getFirstNodeID();
   uint32 nextMapID  = MAX_UINT32;
   uint32 targetDist = req->getDistance();
   bool notDone  =  true;
   bool failed   = false;
   uint32 myDist =     0;
   int nbrLinks  = 0;
   
   // Is the first node on a boundry segment we want to continue into
   // the map.
   BoundrySegment* bSegment = bSegV->getBoundrySegment( nodeID & ITEMID_MASK);
   
   bool leaveBoundry = (bSegment != NULL);
   StreetSegmentItem* currSSI = item_cast<StreetSegmentItem*>
         (curMap->itemLookup(nodeID & ITEMID_MASK));

   // To find my way in a rbt.
   byte rbtExitC = 0;
   int passedRbtEntries = 0;
   
   while((nodeID != MAX_UINT32) && notDone){
      Node* currNode = curMap->nodeLookup(nodeID);
      bool passedRbtEntry = false;
      nbrLinks++;
      if(currSSI != NULL){
         // Find the connections to this node.
         Connection* bestConn = NULL;
         int bestConValue =  0;
         int bestIndex    = -1;
         uint16 nbrConn;
         byte nodeNbr=0;   // Used on BoundrySegments
         if(bSegment != NULL){
            if((nodeID & 0x80000000) != 0x80000000)
               nodeNbr = 0;
            else
               nodeNbr = 1;               
            nbrConn = bSegment->getNbrConnectionsToNode(nodeNbr);
            
         } else {
            nbrConn = currNode->getNbrConnections();
         }
         
         for(uint16 i = 0; i < nbrConn ; i++){
            Connection* thisCon;
            
            
            if(bSegment != NULL){
               // Use the BoundrySegment to find connection.
               thisCon = bSegment->getConnectToNode(nodeNbr, i);
            } else {
               thisCon = currNode->getEntryConnection(i);
            }
            if(curMap->isVehicleAllowed(*thisCon, ItemTypes::passengerCar)){
               int connValue = 0;
               switch(thisCon->getTurnDirection()){
                  case ItemTypes::FOLLOWROAD :
                     // The best thing
                     connValue = 4;
                     break;
                  case ItemTypes::AHEAD :
                     // Good
                     connValue = 3;
                     break;
                  case ItemTypes::KEEP_LEFT :
                     connValue = 2;
                     break;
                  case ItemTypes::KEEP_RIGHT :
                     connValue = 2;
                     break;
                  case ItemTypes::ENTER_ROUNDABOUT :{
                     // Check exit nbr and exit count.
                     passedRbtEntry = true;
                     byte entryC = thisCon->getExitCount();
                     if(// 4-way, 2nd exit or exitC entryC
                        ((thisCon->getCrossingKind() ==
                          ItemTypes::CROSSING_4ROUNDABOUT) &&
                         (passedRbtEntries == 1)) ||
                        ((rbtExitC != 0) && (entryC != 0) &&
                         ((rbtExitC+2 == entryC) ||(rbtExitC == entryC+2))
                         )){
                        connValue = 4;
                     } else {
                        
                        connValue = 1;
                     }
                  }
                     break;
                  case ItemTypes::EXIT_ROUNDABOUT:
                     // Check exit
                     connValue = 1;
                     break;
                  case ItemTypes::RIGHT_ROUNDABOUT :
                     // Possible but not likely
                     connValue = 0;
                     break;
                  case ItemTypes::LEFT_ROUNDABOUT :
                     // Possible but not likely
                     connValue = 0;
                     break;
                  case ItemTypes::AHEAD_ROUNDABOUT:
                     // Possible but not likely
                     connValue = 0;
                     break;
                  default:
                     connValue = -1;
                     break;      
               }
               if(connValue > bestConValue){
                  bestConValue = connValue;
                  bestConn     = thisCon;
                  bestIndex    = i;
               }
               else if (connValue == bestConValue){
                  // More check. Compare the connecting nodes
                  bestConn  = NULL;
                  bestIndex = -1;
               }
            } // thisCon->isVehicleAllowed
            
         } // for(uint16 i = 0; i < nbrConn 

         if(bestConn == NULL){
            // We are stymied - no good exit.
            // Lets return the packet.
            notDone = false;
            
         } else {
            // Go to the next node
          
            
            if(leaveBoundry || (bSegment == NULL)) {
               leaveBoundry = false;  // only leave boundry once.
               nodeID = bestConn->getConnectFromNode();
               StreetSegmentItem* currSSI = item_cast<StreetSegmentItem*>
                  (curMap->itemLookup(nodeID & ITEMID_MASK));
               if(currSSI == NULL){
                  // Give up here.
                  notDone = false;
                  failed = true;
               } else {
                  // update distance
                  // Done?
                  if(myDist+currSSI->getLength() >= targetDist){
                     notDone = false;
                  }
                  
                  // update distance
                  myDist += currSSI->getLength();

                  // update rbt info.
                  if(bestConn->getTurnDirection() ==
                     ItemTypes::EXIT_ROUNDABOUT){
                     rbtExitC = bestConn->getExitCount();
                     passedRbtEntries = 0;
                  } else if(passedRbtEntry) {
                     passedRbtEntries++;
                     if(passedRbtEntries > 9){
                        notDone = false;
                        failed = true;
                     }
                     
                     
                  }
                  passedRbtEntry = false;
                  
               }
               
            } else {
               // We have to leave the map.
               if(nodeNbr ==0){
                  nextMapID  = bSegment->getFromMapIDToNode0(bestIndex);
                  
               } else {
                  nextMapID = bSegment->getFromMapIDToNode1(bestIndex);
               }
               nodeID = bestConn->getConnectFromNode();
               notDone = false;
            }
         }
      } else {//if(currSSI != NULL){
         notDone = false;
         failed = true;
      }
      
      
   } // while(
   
   if(failed){
      mc2log << warn << "[MM]processQueueStartPacket - gave up searching for "
             << "queue begining." << endl;
      mc2dbg2 << "Targe distance";
      mc2dbg2 << " on this map (" << req->getMapID() << ")" << endl;
      mc2dbg2 << ": " << targetDist << "m,  gave up at : "
              << myDist << "m." << endl;
   } else if(nextMapID == MAX_UINT32){
      mc2dbg << "[MM]processQueueStartPacket - done searching for queue"
         "begining."<< endl;
      mc2dbg2 << "Targe distance";
      mc2dbg2 << " on this map (" << req->getMapID() << ")" << endl;
      mc2dbg2 << ": " << targetDist << "m,  gave up at : "
              << myDist << "m." << endl;
   } else {
      mc2dbg << "[MM]processQueueStartPacket - searching for queue"
         "begining. Leaving for map = " << nextMapID << endl;
      mc2dbg2 << "Targe distance";
      mc2dbg2 << " on this map (" << req->getMapID() << ")" << endl;
      mc2dbg2 << ": " << targetDist << "m,  gave up at : "
              << myDist << "m." << endl;
   }
   
   replyPacket->setNodeID(nodeID);
   replyPacket->setDistance(myDist);
   replyPacket->setNextMapID(nextMapID);

   return replyPacket;
}

VanillaMatch*
MapProcessor::createMatchFromItem( Item* item,
                                   GenericMap* theMap,
                                   LangTypes::language_t lang ) const
{
   uint32 searchType = ItemTypes::itemTypeToSearchType( item->getItemType() );
   auto_ptr<SearchMatch> match ( SearchMatch::createMatch( searchType ) );
   if( match.get() != NULL ) {
      const char* name = theMap->getBestItemName( item, lang );
      MC2Coordinate coords = theMap->getOneGoodCoordinate( item );
      
      if( name != NULL ) {
         match->setName( name );
      } else {
         return NULL;
      }
      match->setLocationName( "" );
      match->setCoords( coords );
      match->setItemType( item->getItemType() );
      
      return static_cast<VanillaMatch*>(match.release());
   }
   return NULL;
}

ExpandReplyPacket*
MapProcessor::processExpandRequest(const ExpandRequestPacket* req) const 
{
   // The data that is sent back to the server
   SearchReplyData* replyData = new SearchReplyData();
   
   // The request data
   ExpandRequestData reqData;
   UserRightsMapInfo rights;
   req->get( reqData, rights );

   // Read mapID and itemID from request data
   IDPair_t id = reqData.getIDPair();
   uint32 mapID;
   if( id.first == MAX_UINT32 ) {
      mapID = req->getMapID();
   } else {
      mapID = id.first;
   }
   uint32 itemID = id.second;

   // Read language from request data
   LangTypes::language_t lang = reqData.getLanguage();

   // Read the wanted item types from the request data
   set<ItemTypes::itemType> itemTypes = reqData.getItemTypes();

   // Get the map
   GenericMap* currMap = m_mapHandler->getMap( mapID );
   
   // Vector for the matches
   vector<VanillaMatch*> matches;
   for( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for(uint32 i = 0; i < currMap->getNbrItemsWithZoom(z); i++) {
         Item* item = currMap->getItem( z, i );
         if( item == NULL ) {
            continue;
         }
         if( ! itemTypes.empty() ) {
            // Check if the item type is in the set.
            if( ! ( itemTypes.find( item->getItemType() ) !=
                    itemTypes.end() ) )
            {
               continue;
            }
         } else if( item->getItemType() != ItemTypes::categoryItem ) {
            continue;
         }
         // If itemID != MAX_UINT32 you only add the match if item is a
         // member of the group itemID belongs to
         if( itemID != MAX_UINT32 ) {
            if( ! item->memberOfGroup( itemID ) ) {
               continue;
            }
         } else if( item->getNbrGroups() != 0 ) {
            // If itemID == MAX_UINT32 you only want the top regions or
            // root categories.
            continue;
         }
         const bool allowedByRights =
            currMap->itemAllowedByUserRights( item->getID(), rights );
         // Check if the user has the correct rights for the item.
         if( ! allowedByRights ) {
            continue;
         }
         // Create a search match and add it to the vector if 
         VanillaMatch* match = createMatchFromItem( item,
                                                    currMap,
                                                    lang );
         if( match != NULL )
            matches.push_back( match );
      }
   }
   replyData->getMatchVector().swap( matches );

   ExpandReplyPacket* replyPacket =
      new ExpandReplyPacket( req, *replyData );
   
   delete replyData;
   return replyPacket;
}

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

#include "M3Creator.h"
#include "PropertyHelper.h"
#include "StringUtility.h"
#include "AllocatorTemplate.h"
#include "Stack.h"
#include "GetAdditionalPOIInfo.h"
#include "Properties.h"
#include "TempFile.h"
#include "STLStringUtility.h"

#include "UserRightsMapInfo.h"
#include "Sort1st.h"
#include "ValuePair.h"

// maps
#include "Map.h"
#include "OldMap.h"
#include "OverviewMap.h"
#include "OldOverviewMap.h"
#include "CountryOverviewMap.h"
#include "OldCountryOverviewMap.h"
#include "GenericMap.h"
#include "OldGenericMap.h"

#include "GMSGfxData.h"
#include "GfxData.h"

#include "ItemTypes.h"

#include "OldConnection.h"
#include "Connection.h"

#include "OldExternalConnections.h"
#include "ExternalConnections.h"

#include "OldNode.h"
#include "Node.h"

#include "StreetNbr.h"

// old items (mcm map format)
#include "OldAircraftRoadItem.h"
#include "OldAirportItem.h"
#include "OldBuildingItem.h"
#include "OldBuiltUpAreaItem.h"
#include "OldBusRouteItem.h"
#include "OldCategoryItem.h"
#include "OldCartographicItem.h"
#include "OldCityPartItem.h"
#include "OldFerryItem.h"
#include "OldForestItem.h"
#include "OldGroupItem.h"
#include "OldIndividualBuildingItem.h"
#include "OldIslandItem.h"
#include "OldItem.h"
#include "OldMilitaryBaseItem.h"
#include "OldMunicipalItem.h"
#include "OldNullItem.h"
#include "OldParkItem.h"
#include "OldPedestrianAreaItem.h"
#include "OldPointOfInterestItem.h"
#include "OldRailwayItem.h"
#include "OldRouteableItem.h"
#include "OldStreetItem.h"
#include "OldStreetSegmentItem.h"
#include "OldSubwayLineItem.h"
#include "OldWaterItem.h"
#include "OldZipAreaItem.h"
#include "OldZipCodeItem.h"

// new items (m3 map format)
#include "AircraftRoadItem.h"
#include "AirportItem.h"
#include "BuildingItem.h"
#include "BuiltUpAreaItem.h"
#include "BusRouteItem.h"
#include "CategoryItem.h"
#include "CartographicItem.h"
#include "CityPartItem.h"
#include "FerryItem.h"
#include "ForestItem.h"
#include "GroupItem.h"
#include "IndividualBuildingItem.h"
#include "IslandItem.h"
#include "Item.h"
#include "MunicipalItem.h"
#include "NullItem.h"
#include "ParkItem.h"
#include "PedestrianAreaItem.h"
#include "PointOfInterestItem.h"
#include "RailwayItem.h"
#include "RouteableItem.h"
#include "StreetItem.h"
#include "StreetSegmentItem.h"
#include "SubwayLineItem.h"
#include "WaterItem.h"
#include "ZipAreaItem.h"
#include "ZipCodeItem.h"
#include "MapHashTableBuildable.h"
#include "SignPost.h"

#include "config.h"
#include "ItemAllocator.h"

#include <exception>
#include <typeinfo>

#include "ItemComboTable.h"
#include "MapHashTable.h"
#include "SignPostUtility.h"

#include "MapBits.h"
#include "AlignUtility.h"

#include <assert.h>

using namespace STLUtility;


namespace {

typedef std::set< std::vector<uint32> > Uint32VectorSet;


typedef std::map<std::vector<uint32>, uint32>  OffsetMap;

/*
 * This will create a new array with all groups aligned in
 * one group array.
 * offset will have [vector, offset in array] so it can be used
 * to find offset in array.
 */
void copyAndCalculateArray( const Uint32VectorSet& groupSet, 
                            OffsetMap& offset, SimpleArray<uint32>& array ) {
   Uint32VectorSet::const_iterator group_it = groupSet.begin();
   Uint32VectorSet::const_iterator group_it_end = groupSet.end();

   // calculate group vector size      
   uint32 nbrGroups = 0;
   for (; group_it != group_it_end ; ++group_it) {
      nbrGroups += (*group_it).size();
   }
      
   array.allocate( nbrGroups );

   group_it = groupSet.begin();
   for ( uint32 i = 0; group_it != group_it_end;  ++group_it ) {
      copy( (*group_it).begin(), (*group_it).end(),
            array.begin() + i );

      offset[ *group_it ] = i;

      i += (*group_it).size();
   }

}

MapHashTableBuildable* createMapHashTable( GenericMap& map ) {
   const GfxData* gfx = map.getGfxData();
   if ( gfx == NULL ) {
      return NULL;
   }

   MC2BoundingBox bb;
   gfx->getMC2BoundingBox( bb );
   uint32 nbrCells = 100;

   if ( MapBits::getMapLevel( map.getMapID() ) >= 2 ) {
      // Super overview map. We need more cells for fast searches.
      nbrCells = 300;  // We try a factor 9. 
   }

   return new MapHashTableBuildable( bb.getMinLon(), bb.getMaxLon(), 
                                     bb.getMinLat(), bb.getMaxLat(),
                                     nbrCells, nbrCells, &map );
}
/**
 * Builds a MapHashTable from GenericMap.
 * @return allocated hash table
 */
MapHashTable* buildHashTable( GenericMap& map ) { 
   if ( map.getGfxData() == NULL ) {
      return NULL;
   }
   auto_ptr<MapHashTableBuildable> 
      hashTable( ::createMapHashTable( map ) );

   MC2_ASSERT( hashTable.get() );

   for ( int z=0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      const uint32 nbrItemsWithZoom = map.getNbrItemsWithZoom( z );
      for ( uint32 i = 0; i < nbrItemsWithZoom; ++i ) { 
         // The hashtable decides what do do with POI:s.
         hashTable->addItem( map.getItem( z , i ) );
      }
   }

   set< MapHashTableBuildable::CellType > groupSet;

   const MapHashTableBuildable::CellMatrix& cells = hashTable->getCells();
   const uint32 nbrVertical = hashTable->getNbrVerticalCells();
   const uint32 nbrHorizontal = hashTable->getNbrHorizontalCells();
   
   for ( uint32 v = 0; v < nbrVertical; ++v ) {
      for ( uint32 h = 0; h < nbrHorizontal; ++h ) {
         groupSet.insert( cells[ v ][ h ] );
      }
   }

   OffsetMap groupOffset;
   SimpleArray<uint32> groupArray;
   
   copyAndCalculateArray( groupSet, groupOffset, groupArray );
   hashTable->setItemIDs( groupArray, groupOffset );

   return hashTable.release();
}

void testConnection( const OldGenericMap& map, OldItem& item ) {
   OldRouteableItem* ri = dynamic_cast<OldRouteableItem*>( &item );
   if ( ri == NULL ) {
      return;
   }

   for ( int nod = 0; nod < 2; ++nod ) {
      OldNode* node = ri->getNode(nod);
      for ( int connNbr = 0, m = node->getNbrConnections();
            connNbr < m; ++connNbr ) {
         OldConnection* conn = node->getEntryConnection( connNbr );
         if ( conn->isMultiConnection() ) {
            continue;
         }
        

         OldGenericMap& cmap = const_cast<OldGenericMap&>(map);
         // meh, const cast it
         OldConnection* oppconn = 
            cmap.
            getOpposingConnection( conn, node->getNodeID() );

         if ( oppconn == NULL ) {
            mc2log << error <<  "[M3Creator]: Opposing connection missing: "
                   << " nodeID = " << MC2HEX(node->getNodeID())
                   << " connNbr = " << connNbr << endl;
            MC2_ASSERT( cmap.getOpposingConnection( conn, node->getNodeID() ) );
         }

      }
   }
}

}
void M3Creator::copyExternalConnectionVector( GenericMap &map,
          std::vector< std::pair<uint32, Connection* > > &dest, 
          const OldBoundrySegment::ExternalConnectionVector &src ) const {
   OldBoundrySegment::ExternalConnectionVector::const_iterator it =  
      src.begin();
   OldBoundrySegment::ExternalConnectionVector::const_iterator it_end = 
      src.end();

   for ( ; it != it_end; ++it ){
      Connection *con = 
         map.getConnectionAllocator().getNextObject();
      copyConnection( map, *con, *(*it).second );
      dest.push_back( make_pair((*it).first, con ) );
   }
}

void M3Creator::copyGroupItem( Item& newItem, const OldItem& oldItem ) const {
   //   MC2_ASSERT( Item::groupItem( &newItem ) );
   Item::groupItem( &newItem )->m_nbrItemsInGroup =
      dynamic_cast< const OldGroupItem& >( oldItem ).getNbrItemsInGroup();
}

void M3Creator::copyConnection( GenericMap& map,
                                Connection& con, 
                                const OldConnection& oldCon) const {

   Connection::tollRoadTimeDefaultPenalty_s = 
      OldConnection::tollRoadTimeDefaultPenalty_s;
   Connection::tollRoadDistanceDefaultPenalty_m = 
      OldConnection::tollRoadDistanceDefaultPenalty_m;
   Connection::highwayDefaultPenaltyFactor = 
      OldConnection::highwayDefaultPenaltyFactor;

   con.setFromNode( oldCon.getFromNode() );
   con.setExitCount( oldCon.getExitCount() );
   con.setTurnDirection( oldCon.getTurnDirection() );
   con.m_crossingKind = oldCon.getCrossingKind();
   
   // find offset in restriction vector
   GenericMap::VehicleRestrictionArray::iterator veh_it = 
      find( map.m_vehicleRestrictions.begin(),
            map.m_vehicleRestrictions.end(),
            oldCon.getVehicleRestrictions() );
   if ( veh_it != map.m_vehicleRestrictions.end() ) {
      con.m_vehicleRestrictionIdx = veh_it - map.m_vehicleRestrictions.begin();
   } else {
      // no restrictions
      con.m_vehicleRestrictionIdx = MAX_UINT8;
   }
}

void M3Creator::copyNode( GenericMap& map, Node& node, 
                          const OldGenericMap& oldMap, 
                          const OldNode& oldNode,
                          const RouteableItem& route ) const {


   node.m_nbrEntryConnections = oldNode.getNbrConnections();
   if ( map.getConnectionAllocator().getBlockSize() < 
        map.getConnectionAllocator().getTotalNbrElements() ) {
      mc2dbg << " allocator block size < total nbr elements!" << endl;
      mc2dbg << " block size = "
             << map.getConnectionAllocator().getBlockSize() << endl;
      mc2dbg << " total size = "
             << map.getConnectionAllocator().getTotalNbrElements() << endl;
      MC2_ASSERT( false );
   }

   uint32 nodeConnections = oldNode.getNbrConnections();

   if ( nodeConnections == 0 ) {
      node.m_entryConnections = NULL;
   } else {
      node.m_entryConnections = 
         (Connection *)( map.getConnectionAllocator().getNextObject() );
   }

   for ( uint32 conID = 0;  conID < nodeConnections; ++conID ) {
      if ( conID != 0 ) {
         map.getConnectionAllocator().getNextObject();
      }

      copyConnection( map, node.m_entryConnections[ conID ],
                      *oldNode.getEntryConnection( conID ) );

      // Fill m_connectionLaneIdx with connection index to lane index
      uint32 realConID = map.getConnectionID( 
         node.m_entryConnections[ conID ] );
      uint32 fromNodeId = oldNode.getEntryConnection( conID )->getFromNode();
      uint32 laneIdx = oldNode.getConnectedLanes( oldMap, fromNodeId  );
      if ( laneIdx != MAX_UINT32 && laneIdx != 0 ) {
         map.m_connectionLaneIdx.push_back( make_vpair( realConID, laneIdx ) );
         mc2dbg4 << "M3Creator adding lane connection " << realConID 
                << " laneIdx " << MC2HEX( laneIdx ) << " from node "
                << fromNodeId << " cur node " << oldNode.getNodeID() << endl;
      }

      // Signposts
      // Move to own method:
      //copySignPosts( node, node.m_entryConnections[ conID ], ... );
      // From: connection's fromnode To: oldNode.getNodeID()
      const vector<GMSSignPost*>& signPosts = 
         oldNode.getEntryConnection( conID )->getSignPosts( 
            oldMap, oldNode.getNodeID() );
      typedef multimap< int, SignPost > SignMap;
      SignMap signs;
      typedef set< SignPost > SignSet;
      SignSet compareSigns;
      for ( uint32 i = 0 ; i < signPosts.size() ; ++i ) {
         for ( GMSSignPost::const_iterator it = signPosts[ i ]->begin() ;
               it != signPosts[ i ]->end() ; ++it ) {
            // Make and order signpost sets by prio
            SignPost sign;
            MC2String signStr;
            LangType signLang;

            // fetch map version
            MapGenUtil::mapSupAndVerPair supAndVer =
               MapGenUtil::getMapSupAndVer( oldMap.getMapOrigin() );
            MapGenEnums::mapSupplier mapSupplier = supAndVer.first;
            MapGenEnums::mapVersion mapVersion = supAndVer.second;

            bool newSigns = false;
            // If Tele Atlas GDF 2008.10 ff, use new table
            if ( (mapSupplier == MapGenEnums::TeleAtlas) &&
                 (mapVersion >= MapGenEnums::TA_2008_10) ) {
               // new table
               newSigns = true;
            }

            uint32 prio = SignPostUtility::
               makeSignPost( sign, signStr, signLang,
                             *it, map, oldMap, newSigns );

            if ( prio != SignPostUtility::INVALID_PRIO ) {
               // Make the stringCode
               uint32 stringCode = map.m_itemNames->addString( 
                  signStr.c_str() );
               sign.setText( stringCode );
               // Road class 0 2000m else 1000m if enough length
               uint32 dist = 1000;
               if ( oldNode.isMajorRoad() ) {
                  dist = 2000;
               }
               sign.setDist( MIN( route.getLength(), dist ) );
               // Check for double among already added
               if ( compareSigns.find( sign ) == compareSigns.end() ) {
                  signs.insert( make_pair( prio, sign ) );
                  compareSigns.insert( sign );
               }
            }
         }
      }
      if ( !signs.empty() ) {
         // Insert Signs into the maps SignPosts
         SignPost* signArray = 
            map.m_signPostAllocator->getNewArray( signs.size() );
         uint32 s = 0;
         for ( SignMap::const_iterator it = signs.begin() ;
               it != signs.end() ; ++it, ++s ) {
            signArray[ s ] = it->second;
         }
         map.m_connectionSignPost.push_back( 
            make_vpair( realConID, GenericMap::ConnectionSignPostMapArray( 
                           signArray, signs.size() ) ) );
      }
   }

   node.setNodeID( oldNode.getNodeID() );
   node.setEntryRestrictions( oldNode.getEntryRestrictions() );
   node.setSpeedLimit( oldNode.getSpeedLimit() );
   node.setJunctionType( oldNode.getJunctionType() );
   
   node.setLevel( oldNode.getLevel() );
   node.setMajorRoad( oldNode.isMajorRoad() );
   node.setMaximumWeight( oldNode.getMaximumWeight() );
   node.setMaximumHeight( oldNode.getMaximumHeight() );
   node.setNbrLanes( oldNode.getNbrLanes(oldMap) );
   node.setRoadToll( oldNode.hasRoadToll() );
   
   // Add a lane node map 
   // A lane has directions(10 bits), not allowed to drive in with car (1 bit),
   // So an array of uint16s for nodes with lanes.
   vector<GMSLane> lanes = oldNode.getLanes( oldMap );
   if ( lanes.size() > 0 ) {
      ExpandStringLane* laneArray = map.m_laneAllocator->getNewArray( 
         lanes.size() );
      mc2dbg4 << "M3Creator adding lane to node: " << oldNode.getNodeID();
      for ( byte l = 0 ; l < lanes.size() ; ++l ) {
         laneArray[ l ] = ExpandStringLane( 
            lanes[ l ].getExpandStringLaneBits(),
            false/*preferredLane*/,
            false/*notCarLane should be set by getExpandStringLaneBits*/ );
         mc2dbg4 << " " << laneArray[ l ];
      }
      mc2dbg4 << endl;
      map.m_nodeLane.push_back( 
         make_vpair( 
            oldNode.getNodeID(), 
            GenericMap::NodeLaneMapArray( laneArray, lanes.size() ) ) );
   }

   
}

void M3Creator::copyRouteable( GenericMap& map, Item& newItem, 
                               const OldGenericMap& oldMap,
                               const OldItem& oldItem ) const {
   try {
      MC2_ASSERT( Item::routeableItem( &newItem ) );
      RouteableItem &route = *Item::routeableItem( &newItem );
      const OldRouteableItem &oldRoute = 
         dynamic_cast< const OldRouteableItem& >( oldItem );
      // copy nodes, there are only two nodes...
      for (int i=0; i < 2; ++i) {
         route.m_node[i].setNodeID(oldRoute.getNode( i )->getNodeID() );
         copyNode( map, route.m_node[i], oldMap, *oldRoute.getNode( i ),
                   route );
      }

   } catch (std::bad_cast bad) {
      mc2dbg << "Can not cast (Old)Item to (Old)RouteableItem: "<< 
         bad.what() << endl;
   }
}

void M3Creator::copyItem( GenericMap& map, Item& newItem, 
                          const OldGenericMap& oldMap, 
                          const OldItem& oldItem ) const {

   GroupItem* groupItem = Item::groupItem( &newItem );
   if ( groupItem != NULL ) {
      *groupItem = GroupItem( newItem.getItemType(), newItem.getID() );
   }
   // copy item base

   newItem.setGfxData( oldItem.getGfxData()  ? 
                       GfxData::createNewGfxData( &map, oldItem.getGfxData() )
                       : NULL );

   newItem.setID( oldItem.getID() );

   // copy groups ( copied in the main loop )
   newItem.m_groups = NULL;
   newItem.m_nbrGroups = oldItem.getNbrGroups();

   newItem.m_nbrNames = oldItem.getNbrNames();

   newItem.setSource( oldItem.getSource() );
   newItem.m_type = oldItem.getItemType();

   // copy type specific stuff

   switch ( newItem.m_type ) {
   case ItemTypes::aircraftRoadItem:
      break;
   case ItemTypes::airportItem:
      break;
   case ItemTypes::buildingItem:
      item_cast< BuildingItem* >( &newItem )->
         setBuildingType( ItemTypes::unknownType ); // don't use building type.
      break;

   case ItemTypes::builtUpAreaItem: {
      copyGroupItem( newItem, oldItem );
      item_cast< BuiltUpAreaItem* >( &newItem )->
         setNbrInhabitants( 
            dynamic_cast< const OldBuiltUpAreaItem& >( oldItem).
            getNbrInhabitants() );
      
      BuiltUpAreaItem* bua = static_cast<BuiltUpAreaItem*>( &newItem );
      bua->setDisplayClass( 
         oldMap.getAreaFeatureDrawDisplayClass( oldItem.getID() ) );
   } break;

   case ItemTypes::busRouteItem: {
      // do not copy routeable here, will be copied later
      // see note before copy boundry stuff

      BusRouteItem& bus = *item_cast< BusRouteItem* >( &newItem );
      const OldBusRouteItem& oldBus = 
         dynamic_cast< const OldBusRouteItem& >( oldItem );
      bus.setBusRouteID( oldBus.getBusRouteID() );
      bus.setOffsetInClosestStreet( oldBus.getOffsetInClosestStreet() );
   } break;

   case ItemTypes::categoryItem:
      copyGroupItem( newItem, oldItem );
      break;
   case ItemTypes::cartographicItem:
      item_cast< CartographicItem* >( &newItem )->
         setCartographicType( dynamic_cast<const OldCartographicItem&>
                              ( oldItem ).getCartographicType() );
      break;
   case ItemTypes::cityPartItem:
      break;

   case ItemTypes::ferryItem: {
      // do not copy routeable here, will be copied later
      // see note before copy boundry stuff

      FerryItem& ferry = *item_cast< FerryItem* >( &newItem );
      const OldFerryItem& oldFerry = 
         dynamic_cast< const OldFerryItem& >( oldItem );
      ferry.setFerryType( oldFerry.getFerryType() );
      ferry.setRoadClass( oldFerry.getRoadClass() );
   } break;

   case ItemTypes::forestItem:
      break;

   case ItemTypes::individualBuildingItem:
      item_cast< IndividualBuildingItem* >( &newItem )->
         setBuildingType( 
             dynamic_cast< const OldIndividualBuildingItem& >( oldItem ).
                          getBuildingType() );
      break;

   case ItemTypes::islandItem: {
      IslandItem* island = static_cast<IslandItem*>( &newItem );
      island->setDisplayClass( 
         oldMap.getAreaFeatureDrawDisplayClass( oldItem.getID() ) );
   } break;

   case ItemTypes::militaryBaseItem:
      break;

   case ItemTypes::municipalItem:
      item_cast< MunicipalItem* >( &newItem )->
         setNbrInhabitants( 
                         dynamic_cast< const OldMunicipalItem& >( oldItem ).
                         getNbrInhabitants() );
      break;

   case ItemTypes::nullItem:
      break;

   case ItemTypes::parkItem:
      item_cast< ParkItem* >( &newItem )->
         setParkType( dynamic_cast< const OldParkItem& >( oldItem ).
                      getParkType() );
      break;

   case ItemTypes::pedestrianAreaItem:
      break;

   case ItemTypes::pointOfInterestItem: {
      PointOfInterestItem& poi = 
         *item_cast< PointOfInterestItem* >( &newItem );
      const OldPointOfInterestItem& oldPOI = 
         dynamic_cast< const OldPointOfInterestItem& >( oldItem );

      poi.setStreetSegmentItemID( oldPOI.getStreetSegmentItemID() );
      poi.setOffsetOnStreet( oldPOI.getOffsetOnStreet() );

      poi.setPointOfInterestType( oldPOI.getPointOfInterestType() );
      poi.setWASPID( oldPOI.getWASPID() );
      poi.setSide( oldPOI.getSide() );
   } break;

   case ItemTypes::railwayItem:
      break;

   case ItemTypes::routeableItem:      
      // do not copy routeable here, will be copied later
      // see note before copy boundry stuff

      break;

   case ItemTypes::streetItem: {
      copyGroupItem( newItem, oldItem );

      StreetItem& street = *item_cast< StreetItem* >( &newItem );
      const OldStreetItem& oldStreet = 
         dynamic_cast< const OldStreetItem& >( oldItem );

      if ( oldStreet.getGfxData() != NULL ) {
         // copy polygons
         street.m_itemOffsetForPolygon = 
            new uint16[ oldStreet.getGfxData()->getNbrPolygons() ];
         memcpy( street.m_itemOffsetForPolygon, 
                 oldStreet.getItemOffsetForPolygon(),
             sizeof( uint16 ) * oldStreet.getGfxData()->getNbrPolygons() );

      } else {
         street.m_itemOffsetForPolygon = NULL;
      }

   } break;

   case ItemTypes::streetSegmentItem: {
      // do not copy routeable here, will be copied later
      // see note before copy boundry stuff

      StreetSegmentItem& street = 
         *item_cast< StreetSegmentItem* >( &newItem );
      const OldStreetSegmentItem& oldStreet = 
         dynamic_cast< const OldStreetSegmentItem& >( oldItem );

      street.setRoadClass( oldStreet.getRoadClass() );

      street.m_width = oldStreet.getWidth();
      street.m_streetNumberType = oldStreet.getStreetNumberType();
      street.setRoadClass( oldStreet.getRoadClass() );
      street.m_condition = oldStreet.getRoadCondition();
      street.setRoundaboutValue( oldStreet.isRoundabout() );
      street.setRoundaboutishValue( oldStreet.isRoundaboutish() );
      street.setRampValue( oldStreet.isRamp() );
      street.setDividedValue( oldStreet.isDivided() );
      street.setMultiDigitisedValue( oldStreet.isMultiDigitised() );
      street.setControlledAccessValue( oldStreet.isControlledAccess() );
      street.setDisplayClass( oldMap.getRoadDisplayClass( oldItem.getID() ) );
      
   } break;

   case ItemTypes::subwayLineItem:      
      break;

   case ItemTypes::waterItem: {
      WaterItem* water = static_cast<WaterItem*>( &newItem );
      water->setWaterType( dynamic_cast< const OldWaterItem& >( oldItem ).
                           getWaterType() );
      water->setDisplayClass( 
         oldMap.getAreaFeatureDrawDisplayClass( oldItem.getID() ) );
   } break;

   case ItemTypes::zipAreaItem:
      copyGroupItem( newItem, oldItem );
      break;
   case ItemTypes::zipCodeItem:
      copyGroupItem( newItem, oldItem );
      break;
   } 

}


GenericMap* M3Creator::createM3Map( const OldGenericMap& gmsMap ) const
{

   GenericMap* map = 0;
   if ( MapBits::isUnderviewMap( gmsMap.getMapID() ) ) {
      map = new Map( gmsMap.getMapID() );
      copyUnderviewMap( static_cast< Map& >( *map ), 
                        static_cast< const OldMap&  >( gmsMap ) );
   } else if ( MapBits::isOverviewMap( gmsMap.getMapID() ) ) {
      map = new OverviewMap( gmsMap.getMapID() );
      copyOverviewMap( static_cast< OverviewMap& >( *map ),
                       static_cast< const OldOverviewMap& >( gmsMap ) );
   } else {
      map = new CountryOverviewMap( gmsMap.getMapID() );
      copyCountryOverviewMap( static_cast< CountryOverviewMap& >( *map ),
                              static_cast< const OldCountryOverviewMap& >(
                                                     gmsMap ) );
   }


   //
   // Copy OldGenericMapHeader
   //
   map->createAllocators();
   map->createBoundrySegmentsVector();

   map->setMapName( gmsMap.getMapName() );
   map->setCopyrightString( gmsMap.getCopyrightString() );
   map->setMapOrigin( gmsMap.getMapOrigin() );
   map->setCountryCode( gmsMap.getCountryCode() );
   map->setDrivingSide( gmsMap.driveOnRightSide() ); 

   // for each native language add...
   for ( uint32 lang = 0; 
         lang < gmsMap.getNbrNativeLanguages(); ++lang ) {
      map->addNativeLanguage( gmsMap.getNativeLanguage( lang ) );
   }
   // for each currency add...
   for ( uint32 currency = 0;
         currency < gmsMap.getNbrCurrencies(); ++currency ) {
      map->addCurrency( gmsMap.getCurrency( currency ) );
   }
   

   map->m_creationTime = gmsMap.getCreationTime();
   map->setTrueCreationTime( gmsMap.getTrueCreationTime() );
   map->setWaspTime( gmsMap.getWaspTime() );
   map->setDynamicExtradataTime( gmsMap.getDynamicExtradataTime() );

   map->m_utf8Strings = gmsMap.stringsCodedInUTF8();

   map->setMapFiltered( gmsMap.mapIsFiltered() );
   map->setMapGfxDataFiltered( gmsMap.mapGfxDataIsFiltered() );

   map->m_totalMapSize = gmsMap.getTotalMapSize();

   map->m_nbrAllocators = gmsMap.getNbrAllocators();

   map->setFilename( gmsMap.getFilename() );

   if (map->m_pathname != NULL)
      delete [] map->m_pathname;

   map->m_pathname = StringUtility::newStrDup( gmsMap.getPathName() );
 
   map->m_groupsInLocationNameOrder = gmsMap.groupsAreInLocationNameOrder();
   map->m_loadedVersion = gmsMap.getLoadedVersion();
   map->m_mapCountryDir = 
      StringUtility::newStrDup( gmsMap.getMapCountryDir() );
   
   //
   // copy OldGenericMaps stuff
   //

   // copy all zoom sizes
   memcpy( map->m_itemsZoomSize, gmsMap.getItemsZoomSize(), 
           sizeof( uint32 ) * NUMBER_GFX_ZOOMLEVELS );

   // copy m_itemZoomAllocated
   memcpy( map->m_itemsZoomAllocated, gmsMap.getItemsZoomAllocated(),
           sizeof( uint32 ) * NUMBER_GFX_ZOOMLEVELS );

   // copy m_itemNames
   map->m_itemNames = new ItemNames;
   const OldItemNames &names = *gmsMap.getItemNames();
   for ( uint32 i = 0; i < names.getNbrStrings(); ++i ) {
      if ( ! UTF8Util::isValidUTF8( names.getString( i ) ) ) {
         mc2log << error << "String: " << MC2CITE( names.getString(i) )
                << " is not valid UTF-8" << endl;
      }
      map->m_itemNames->addString( names.getString( i ) );
   }

   map->m_adminAreaCentres.reset(
      new GenericMap::adminAreaCentre_t[ gmsMap.m_nbrAdminAreaCentres ] );
   for (uint32 i = 0; i < gmsMap.m_nbrAdminAreaCentres; ++i) {
      map->m_adminAreaCentres[ i ].itemID  = 
         gmsMap.m_adminAreaCentres[ i ].itemID;
      map->m_adminAreaCentres[ i ].centre  = 
         gmsMap.m_adminAreaCentres[ i ].centre;
   }

   map->m_nbrAdminAreaCentres = gmsMap.m_nbrAdminAreaCentres;

   delete map->m_userRightsTable;
   map->m_userRightsTable = new UserRightsItemTable( gmsMap.getRightsTable() );

   if ( gmsMap.m_allRight != 0)
      *map->m_allRight = *gmsMap.m_allRight;

   map->m_idTranslationTable = gmsMap.m_idTranslationTable;
   map->m_idTranslationTable.sortElements();
   // oldVersion gets copied so we restore it to 0
   map->m_idTranslationTable.m_version = 0; 

   MC2_ASSERT( map->m_idTranslationTable.getNbrElements() == 
               gmsMap.m_idTranslationTable.getNbrElements() );
   for (uint32 i = 0; i < map->m_idTranslationTable.getNbrElements(); ++i ) {
      uint32 new_itemID, new_mapID, new_overItemID;      
      uint32 old_itemID, old_mapID, old_overItemID;
      gmsMap.m_idTranslationTable.getElement( i, old_itemID, old_mapID,
                                              old_overItemID );
      map->m_idTranslationTable.getElement( i, new_itemID, new_mapID,
                                            new_overItemID );
      MC2_ASSERT( old_itemID == new_itemID );
      MC2_ASSERT( old_mapID == new_mapID );
      MC2_ASSERT( old_overItemID == new_overItemID );
   }

   map->m_approxMapSize = gmsMap.getApproxMapSize();

   map->m_gfxData = GfxData::createNewGfxData( map, gmsMap.getGfxData() );

   map->getLandmarkTable() = gmsMap.getLandmarkTable();

   map->m_nodeExpansionTable = *gmsMap.getNodeExpansionTable();

   assert(map->m_nodeExpansionTable.size() == 
          gmsMap.getNodeExpansionTable()->size());

   Uint32VectorSet 
      groupSet, 
      nameSet,
      itemsInGroupSet;

   OffsetMap 
      groupOffsetMap, 
      nameOffsetMap, 
      itemsInGroupOffsetMap;

   { // scope for some map/set counters 
      
      // number of coordinates/2 in GfxDataSingleSmallPoly
      uint32 nbrCoordinates = 0;
      if ( map->getGfxData() && 
           map->getGfxData()->getGfxDataType() == 
           GfxData::gfxDataSingleSmallPoly ) {
         nbrCoordinates = map->getGfxData()->getTotalNbrCoordinates();
      }

      std::map<uint32, StreetNbr> sideNbrMap;
      StreetNbr nbr;
      long nbrConnections = 0;
      std::map<uint32, uint32> itemCounter;
      std::set<uint32> vehicleRestrictions;

      // for each m_itemsZoom add sideNbrMap and count number of groups
      for ( uint32 zoom = 0; zoom < NUMBER_GFX_ZOOMLEVELS; ++zoom ) {

         const uint32 nbrItems = gmsMap.getNbrItemsWithZoom( zoom );


         for ( uint32 item = 0; item < nbrItems; ++item ) {
            OldItem *oldItem = gmsMap.getItem( zoom, item );

            // special case for SSI, create StreetNbr table
            if ( oldItem == NULL ) {
               continue;
            } 

            if ( ! MapBits::isCountryMap(gmsMap.getMapID()) ){
               testConnection( gmsMap, *oldItem );
            }

            // 
            // count nbr of items for each type
            itemCounter[oldItem->getItemType()]++; 
            
            for ( uint32 g = 0; g < oldItem->getNbrGroups(); ++g ) {
               uint32 groupID = oldItem->getGroup( g );
               OldItem *groupedItem = gmsMap.itemLookup( groupID );
               MC2_ASSERT( groupedItem != NULL );
               MC2_ASSERT( groupedItem->getItemType() != ItemTypes::nullItem );
            }

            // copy groups
            if ( oldItem->getNbrGroups() > 0 ) { 
               Uint32VectorSet::value_type groupVec( oldItem->getGroups(),
                            oldItem->getGroups() + oldItem->getNbrGroups() );
               groupSet.insert( groupVec );
            }

            // copy name
            if ( oldItem->getNbrNames() > 0 ) {
               Uint32VectorSet::value_type nameVec( oldItem->getNames(),
                               oldItem->getNames() + oldItem->getNbrNames() );
               nameSet.insert( nameVec );
            }

            // if group item then copy items in group vector
            OldGroupItem *oldGroupItem = 
               dynamic_cast< OldGroupItem* >( oldItem );            
            if ( oldGroupItem != NULL  && 
                 oldGroupItem->getNbrItemsInGroup() > 0 ) {
               vector<uint32> nums;
               for ( uint32 i = 0; 
                     i < oldGroupItem->getNbrItemsInGroup(); ++i) {
                  nums.push_back( oldGroupItem->getItemNumber( i ) );
               }
               itemsInGroupSet.insert( nums );
            }

            // check gfx daa
            if ( oldItem->getGfxData() ) {

               GfxData *data = oldItem->getGfxData();

               // we need polys else whats the idea... X-)
               MC2_ASSERT( data->getNbrPolygons() != 0 );

               // we need at least one coordinate in the poly
               for ( uint32 p = 0; p < data->getNbrPolygons(); ++p ) {
                  MC2_ASSERT( data->getNbrCoordinates( p ) != 0 );
               }
               
               if ( oldItem->getGfxData()->getGfxDataType() == 
                    GfxData::gfxDataSingleSmallPoly ) {
                  nbrCoordinates = oldItem->getGfxData()->getTotalNbrCoordinates();
               }
                 
            }

            // if routeable then copy vehicle connections
            if ( oldItem->getItemType() == ItemTypes::streetSegmentItem ||
                 oldItem->getItemType() == ItemTypes::ferryItem ||
                 oldItem->getItemType() == ItemTypes::busRouteItem ) {
               
               OldRouteableItem *route = 
                  static_cast<OldRouteableItem* >( oldItem );
               // for each node in routeable count nbr connections
               // and add vehicle restrions to our set
               for (int n = 0; n < 2; ++n ) { // Node0 and Node1
                  OldNode *node = route->getNode( n );
                  nbrConnections += node->getNbrConnections();
                  for ( int c = 0; c < node->getNbrConnections(); ++c ) {
                     vehicleRestrictions.insert( 
                       node->getEntryConnection( c )->getVehicleRestrictions() );
                  }
               }
            }
            // do street segment item stuff

            if ( oldItem->getItemType() != ItemTypes::streetSegmentItem ) {
               continue;
            }

            try {
               OldStreetSegmentItem &ssi = 
                  dynamic_cast<OldStreetSegmentItem &>( *oldItem );
               
               nbr.m_leftSideNbrStart = ssi.getLeftSideNbrStart();
               nbr.m_leftSideNbrEnd = ssi.getLeftSideNbrEnd();
               nbr.m_rightSideNbrStart = ssi.getRightSideNbrStart();
               nbr.m_rightSideNbrEnd = ssi.getRightSideNbrEnd();
               sideNbrMap[ ssi.getID() ]  = nbr;

            } catch (std::bad_cast ignore) {

            }
         }
      }         


      // create groups, fill it in in the next round of item loop
      copyAndCalculateArray( groupSet, groupOffsetMap, map->m_groups );
      copyAndCalculateArray( nameSet, nameOffsetMap, map->m_names );
      copyAndCalculateArray( itemsInGroupSet, itemsInGroupOffsetMap, 
                             map->m_itemsInGroup );

      // must set streetSideTable before we copy items since it calculates
      // side index table

      // the table calculates both best number of bits
      // to use for index and the best default value
      map->m_streetSideTable.setValues( sideNbrMap );

      if ( gmsMap.getSegmentsOnTheBoundry() != NULL ) {
         const OldBoundrySegmentsVector &segments = *gmsMap.getSegmentsOnTheBoundry();
         for ( uint32 i = 0; i < segments.size(); ++i ) {
            if ( segments.getElementAt( i ) == NULL ) 
               continue;
            const OldBoundrySegment &boundry = 
               *static_cast< OldBoundrySegment* >( segments.getElementAt( i ) );

            // node 0 and 1 copy vehicle restriction and count nbr connections
            for (int n = 0; n < 2; ++n ) {
               nbrConnections += boundry.getNbrConnectionsToNode( n );
               for ( uint32 c = 0; c < boundry.getNbrConnectionsToNode( n );
                     ++c ) {
                  OldConnection *con = boundry.getConnectToNode( n, c );
                  vehicleRestrictions.insert( con->getVehicleRestrictions() );
               }
            }
         }
      }

      // create vehicle restriction array and copy the old set
      map->m_vehicleRestrictions.allocate( vehicleRestrictions.size() );
      copy( vehicleRestrictions.begin(), vehicleRestrictions.end(), 
            map->m_vehicleRestrictions.begin() );            
      
      // this would've been painfull without sed program (without using macro)
      map->m_streetSegmentItemAllocator->reallocate( itemCounter[ ItemTypes::streetSegmentItem ] );
      map->m_municipalItemAllocator->reallocate( itemCounter[ ItemTypes::municipalItem ] );
      map->m_waterItemAllocator->reallocate( itemCounter[ ItemTypes::waterItem ] );
      map->m_parkItemAllocator->reallocate( itemCounter[ ItemTypes::parkItem ] );
      map->m_forestItemAllocator->reallocate( itemCounter[ ItemTypes::forestItem ] );
      map->m_buildingItemAllocator->reallocate( itemCounter[ ItemTypes::buildingItem ] );
      map->m_railwayItemAllocator->reallocate( itemCounter[ ItemTypes::railwayItem ] );
      map->m_islandItemAllocator->reallocate( itemCounter[ ItemTypes::islandItem ] );
      map->m_nullItemAllocator->reallocate( itemCounter[ ItemTypes::nullItem ] );
      map->m_zipCodeItemAllocator->reallocate( itemCounter[ ItemTypes::zipCodeItem ] );
      map->m_builtUpAreaItemAllocator->reallocate( itemCounter[ ItemTypes::builtUpAreaItem ] );
      map->m_cityPartItemAllocator->reallocate( itemCounter[ ItemTypes::cityPartItem ] );
      map->m_zipAreaItemAllocator->reallocate( itemCounter[ ItemTypes::zipAreaItem ] );
      map->m_pointOfInterestItemAllocator->reallocate( itemCounter[ ItemTypes::pointOfInterestItem ] );
      map->m_categoryItemAllocator->reallocate( itemCounter[ ItemTypes::categoryItem ] );
      map->m_busRouteItemAllocator->reallocate( itemCounter[ ItemTypes::busRouteItem ] );      
      map->m_ferryItemAllocator->reallocate( itemCounter[ ItemTypes::ferryItem ] );
      map->m_airportItemAllocator->reallocate( itemCounter[ ItemTypes::airportItem ] );
      map->m_aircraftRoadItemAllocator->reallocate( itemCounter[ ItemTypes::aircraftRoadItem ] );
      map->m_pedestrianAreaItemAllocator->reallocate( itemCounter[ ItemTypes::pedestrianAreaItem ] );
      map->m_militaryBaseItemAllocator->reallocate( itemCounter[ ItemTypes::militaryBaseItem ] );
      map->m_individualBuildingItemAllocator->reallocate( itemCounter[ ItemTypes::individualBuildingItem ] );
      map->m_subwayLineItemAllocator->reallocate( itemCounter[ ItemTypes::subwayLineItem ] );
      map->m_streetItemAllocator->reallocate( itemCounter[ ItemTypes::streetItem ] );
      map->m_cartographicItemAllocator->reallocate( itemCounter[ ItemTypes::cartographicItem ] );
      map->getConnectionAllocator().reallocate( nbrConnections  );
      map->getCoordinateAllocator().reallocate( nbrCoordinates );

   } // end sideNbrMap scope
   
   // copy categories
   {
      OldGenericMap::CategoryMap::const_iterator catIt =
         gmsMap.m_itemCategories.begin();
      OldGenericMap::CategoryMap::const_iterator catItEnd =
         gmsMap.m_itemCategories.end();
      vector<uint32> testItems;
      const uint32 maxTestItems = 100;
      for (; catIt != catItEnd; ++catIt ) {
         // test items
         uint32 id = (*catIt).first;
         const OldGenericMap::CategoryMap::mapped_type& oldValues = 
            (*catIt).second;


         GenericMap::CategoryMap::value_type::value_type values = 
            GenericMap::CategoryArray( map->m_categoryAllocator->
                                       getNewArray( oldValues.size() ),
                                       oldValues.size() );
         if ( testItems.size() < maxTestItems ) {
            testItems.push_back( id  );
         }

         std::copy( oldValues.begin(), oldValues.end(), values.begin() );

         map->m_categoryIds.push_back( make_vpair( id, values ) );
      }
      MC2_ASSERT( map->m_categoryIds.size() == gmsMap.m_itemCategories.size());

      // sort em!
      std::sort( map->m_categoryIds.begin(), map->m_categoryIds.end() );
      // just some testing
      mc2dbg << "Testing categories." << endl;

      for ( uint32 i = 0; i < testItems.size(); ++i ) {

         OldGenericMap::CategoryMap::const_iterator 
            oldIt = gmsMap.m_itemCategories.find( testItems[ i ] );
         MC2_ASSERT( oldIt != gmsMap.m_itemCategories.end() );

         GenericMap::Categories categories;
         // fake an item, we are only interested in the ID here
         Item fakeItem( (ItemTypes::itemType)0, testItems[ i ] );
         map->getCategories( fakeItem, categories );
         if ( categories.size() != (*oldIt).second.size() ) {
            mc2log << fatal << "Old #values: " << (*oldIt).second.size() << endl;
            mc2log << fatal << "New #values: " << categories.size() << endl;
            MC2_ASSERT( categories.size() == (*oldIt).second.size() );
         }

         MC2_ASSERT( std::equal( categories.begin(), categories.end(),
                                 (*oldIt).second.begin() ) );
      }

   }
 
   // for each m_itemsZoom copy to our m_itemsZoom
   for ( uint32 zoom = 0; zoom < NUMBER_GFX_ZOOMLEVELS; ++zoom ) {

      const uint32 nbrItems = gmsMap.getNbrItemsWithZoom( zoom );
      map->m_itemsZoom[ zoom ] = new Item*[ nbrItems ];

      for ( uint32 item = 0; item < nbrItems; ++item ) {
         OldItem *oldItem = gmsMap.getItem( zoom, item );

         if ( oldItem == NULL ) {
            map->m_itemsZoom[ zoom ][ item ] = NULL;
            continue;
         }


         ItemTypes::itemType type = oldItem->getItemType();
         
         Item *newItem = Item::createItemFromType( type, 
                                                   *map, oldItem->getID() );
         
         MC2_ASSERT( newItem != NULL );

         newItem->init( oldItem->getItemType() );
         
         // fill in the new item
         map->m_itemsZoom[ zoom ][ item ] = newItem;
         
         try {

            copyItem( *map, *newItem, 
                      gmsMap, *oldItem );

            MC2_ASSERT( newItem->getNbrGroups() == oldItem->getNbrGroups() );
            // copy group to maps group vector
            if ( oldItem->getNbrGroups() != 0 ) {

               std::vector<uint32> oldGroups( oldItem->getGroups(),
                             oldItem->getGroups() + oldItem->getNbrGroups());

               newItem->setGroup(const_cast<uint32*>(map->getItemGroups() ) + 
                                  groupOffsetMap[ oldGroups ] );
               /*
               for ( int g = 0; g < newItem->getNbrGroups(); ++g ) {
                  MC2_ASSERT( newItem->getGroups()[ g ] == 
                              oldItem->getGroups()[ g ] );
               }
               */
               
            }

            //    MC2_ASSERT( newItem->getNbrNames() == oldItem->getNbrNames() );
            if ( oldItem->getNbrNames() != 0 ) {
               std::vector<uint32> oldNames( oldItem->getNames(),
                              oldItem->getNames() + oldItem->getNbrNames() );

               newItem->setNames(const_cast<uint32*>(map->getItemNameIdx() ) +
                                  nameOffsetMap[ oldNames ] );
               /*
               for ( int n = 0; n < newItem->getNbrNames(); ++n ) {
                  MC2_ASSERT( newItem->getNames()[ n ] ==
                              oldItem->getNames()[ n ] );
               }
               */
            }

            // set offset into items in group in GenericMap
            OldGroupItem *oldGroupItem = 
                                     dynamic_cast< OldGroupItem* >( oldItem );

            if ( oldGroupItem != NULL  && 
                 oldGroupItem->getNbrItemsInGroup() > 0 ) {

               vector<uint32> nums;
               for ( uint32 i = 0; 
                     i < oldGroupItem->getNbrItemsInGroup(); ++i) {
                  nums.push_back( oldGroupItem->getItemNumber( i ) );
               }

               Item::groupItem( newItem )->setItemsInGroup( 
                             const_cast< uint32* >( map->getItemsInGroup() ) +
                             itemsInGroupOffsetMap[ nums ] );
            }
            /*
            // test the street side table for SSI items
            if ( oldItem->getItemType() != ItemTypes::streetSegmentItem ) {
               continue;
            }

            const OldStreetSegmentItem& oldSSI = 
               dynamic_cast< const OldStreetSegmentItem& >( *oldItem );
            const StreetSegmentItem& ssi = 
               *item_cast< const StreetSegmentItem* >( newItem );

            MC2_ASSERT( oldSSI.getLeftSideNbrStart() ==
                        map->getStreetLeftSideNbrStart( ssi ) );
            MC2_ASSERT( oldSSI.getLeftSideNbrEnd() == 
                        map->getStreetLeftSideNbrEnd( ssi ) );
            MC2_ASSERT( oldSSI.getRightSideNbrStart() ==
                        map->getStreetRightSideNbrStart( ssi ) );
            MC2_ASSERT( oldSSI.getRightSideNbrEnd() == 
                        map->getStreetRightSideNbrEnd( ssi ) );
            */
         } catch (std::bad_cast bad) {
            mc2dbg << "Can not cast from Item type to item (" << 
               newItem->getItemType() << ")" << endl;
         }

#if 1
         // Test to see if having index areas as Municipals are a solution
         // to both not drawing them and still finding them from coordinate.
         if ( type == ItemTypes::builtUpAreaItem &&
              gmsMap.isIndexArea( oldItem->getID() ) ) {
            newItem->setItemType( ItemTypes::municipalItem );
            mc2dbg << "Done with item 0x" << hex << oldItem->getID() << dec 
                   << " is now " << newItem->getItemTypeAsString()
                   << endl;
         }
#endif
      }
   }

   // copy routeables, must do this here so the connection IDs
   // gets aligned correctly at save, do not move this loop
   for ( int type_it = 0; type_it < 3; ++type_it ) {
      for ( uint32 zoom = 0; zoom < NUMBER_GFX_ZOOMLEVELS; ++zoom ) {

         const uint32 nbrItems = gmsMap.getNbrItemsWithZoom( zoom );
         for ( uint32 item = 0; item < nbrItems; ++item ) {
            OldItem *oldItem = gmsMap.getItem( zoom, item );
            
            if ( oldItem == NULL ) {
               continue;
            }
            if ( ( type_it == 0 && 
                 oldItem->getItemType() == ItemTypes::streetSegmentItem ) ||
                 ( type_it == 1 && 
                   oldItem->getItemType() == ItemTypes::busRouteItem ) ||
                 ( type_it == 2 && 
                   oldItem->getItemType() == ItemTypes::ferryItem ) ) {
               copyRouteable( *map, *map->getItem( zoom, item ),
                              gmsMap, *oldItem );
            }
            
         }
      }
   }
   // Sort the lane copied in copyRouteable
   std::sort( map->m_nodeLane.begin(), map->m_nodeLane.end() );
   std::sort( map->m_connectionLaneIdx.begin(), 
              map->m_connectionLaneIdx.end() );

   // Sort the SignPost 
   std::sort( map->m_connectionSignPost.begin(), 
              map->m_connectionSignPost.end() );

   // copy boundry stuff...
   if (gmsMap.getSegmentsOnTheBoundry() != NULL) {

      const OldBoundrySegmentsVector &segments = 
         *gmsMap.getSegmentsOnTheBoundry();

      for ( uint32 i = 0; i < segments.size(); ++i) {
         if ( segments.getElementAt(i) == NULL )
            continue;

         const OldBoundrySegment &oldSegment = 
         *static_cast< const OldBoundrySegment* >(segments.getElementAt( i ) );

         BoundrySegment *b = new BoundrySegment();
         map->getSegmentsOnTheBoundry()->push_back( b );
      
         copyExternalConnectionVector( *map,
                                       b->m_connectionsToNode0,
                                       oldSegment.getConnectionsToNode0() );
         copyExternalConnectionVector( *map,
                                       b->m_connectionsToNode1,
                                       oldSegment.getConnectionsToNode1() );
         b->m_routeableItemID = oldSegment.getRouteableItemID();
         switch ( oldSegment.getCloseNode() ) {
         case OldBoundrySegment::node0close:
            b->m_closeNode = BoundrySegment::node0close;
            break;
         case OldBoundrySegment::node1close:
            b->m_closeNode = BoundrySegment::node1close;
            break;
         case OldBoundrySegment::noNodeClose:
            b->m_closeNode = BoundrySegment::noNodeClose;
            break;
         }

      }  
   } else {
      delete map->m_segmentsOnTheBoundry;
      map->m_segmentsOnTheBoundry = NULL;
   }


   // Copy index area table
   {
      map->m_indexAreaOrderMap.reserve( gmsMap.m_indexAreasOrder.size() );
      for ( ItemMap<uint32>::const_iterator it = 
               gmsMap.m_indexAreasOrder.begin(), 
               end = gmsMap.m_indexAreasOrder.end() ; it != end ; ++it ) {
         map->m_indexAreaOrderMap.push_back( 
            make_vpair( it->first, it->second ) );
      }
      // Make sure it is sorted (Should be as it was copied from a std::map)
      std::sort( map->m_indexAreaOrderMap.begin(),
                 map->m_indexAreaOrderMap.end() );
   }


   //   map->buildHashTable();
   map->m_hashTable.reset( ::buildHashTable( *map ) );

   if ( *map->getHashTable() != *gmsMap.getHashTable() ) {
      mc2log << fatal << "Hash tables differs." << endl;
      MC2_ASSERT( false );
   }
   return map;
   
}

void M3Creator::copyOverviewMap( OverviewMap &map, 
                                 const OldOverviewMap &oldMap ) const { 
   // copy containing maps
   map.getContainingMaps() = oldMap.getContainingMaps();
}

void M3Creator::copyUnderviewMap( Map &map, 
                                  const OldMap &oldMap ) const {
   // nothing to do makes jack a dull boy
   // but we never know what the future looks like...unless...

   // Todo: make this code work. I can not find the bug.
   /*
      TimeMachine m("destination: Future"); 
      m.run();
   */
}


void M3Creator::copyCountryOverviewMap( CountryOverviewMap &map, 
                                 const OldCountryOverviewMap &oldMap ) const {


   {
      // must be filtered
      MC2_ASSERT( oldMap.mapGfxDataIsFiltered() );

      OldCountryOverviewMap::OriginalIDsMap::const_iterator it = 
         oldMap.getOriginalIDs().begin();
      OldCountryOverviewMap::OriginalIDsMap::const_iterator it_end = 
         oldMap.getOriginalIDs().end();

      map.getOriginalIDs().allocate( oldMap.getOriginalIDs().size() );
      // add ids
      for (uint32 i = 0; it != it_end; ++it, ++i ){
         map.getOriginalIDs()[ i ].itemID = (*it).first;
         map.getOriginalIDs()[ i ].origItemID = (*it).second.origItemID;
         map.getOriginalIDs()[ i ].origMapID = (*it).second.origMapID;
      }
      // sort ids
      sort( map.getOriginalIDs().begin(),
            map.getOriginalIDs().end(),
            CountryOverviewMap::SortIDs() );

      // debug
      for (uint32 i = 0;  i < map.getOriginalIDs().size(); ++i ) {
         uint32 origMapID, origItemID;
         MC2_ASSERT( const_cast<OldCountryOverviewMap&>(oldMap).getOriginalIDs( 
                                            map.getOriginalIDs()[ i ].itemID,
                                            origMapID, origItemID ) );
         MC2_ASSERT( map.getOriginalIDs()[ i ].origMapID == origMapID );
         MC2_ASSERT( map.getOriginalIDs()[ i ].origItemID == origItemID );
         
      } 
   }
   {
      OldCountryOverviewMap::MapNoticeVector::const_iterator it = 
         oldMap.getMapsInCountry().begin();
      OldCountryOverviewMap::MapNoticeVector::const_iterator it_end = 
         oldMap.getMapsInCountry().end();
      for (; it != it_end; ++it ) {
         CountryOverviewMap::mapnotice_t mn;
         mn.mapID = (*it).mapID;
         mn.creationTime = (*it).creationTime;
         mn.maxLat = (*it).maxLat;
         mn.minLon = (*it).minLon;
         mn.minLat = (*it).minLat;
         mn.maxLon = (*it).maxLon;
         map.getMapsInCountry().push_back( mn );
      }
   }
   map.setNbrGfxPolygons( oldMap.getNbrGfxPolygons() );

   map.m_simplifiedGfxStack = new Stack**[ map.NBR_SIMPLIFIED_COUNTRY_GFX ];
   for ( uint32 level = 0; level < map.NBR_SIMPLIFIED_COUNTRY_GFX; ++level ) {

      map.m_simplifiedGfxStack[ level ] = 
         new Stack*[ oldMap.getNbrGfxPolygons() ];

      for ( uint32 poly = 0; poly < oldMap.getNbrGfxPolygons(); ++poly ) {
         Stack *stack = new Stack;
         map.m_simplifiedGfxStack[ level ][ poly ] = stack;
         *map.m_simplifiedGfxStack[ level ][ poly ] = 
            *oldMap.getStack()[ level ][ poly ];
      }
   }
}

bool operator == ( const MapHashTable& first,
                   const OldMapHashTable& second ) {
#define _NEQ( x ) if ( first.x != second.x ) { \
  cout << "MapHashTable item: " << #x << " differs. " \
       << " new/old: " << static_cast<int32>( first.x ) << "/" \
       << static_cast<uint32>( second.x ) << endl; \
  return false; \
} else {\
  cout << #x << " = " << static_cast<int32>( first.x ) << ";"<<endl; \
}

 _NEQ( m_map->getMapID() );
 _NEQ( m_topVertical );
 _NEQ( m_bottomVertical );
 _NEQ( m_rightHorizontal );
 _NEQ( m_leftHorizontal );
 _NEQ( m_nbrVerticalCells );
 _NEQ( m_nbrHorizontalCells );
 _NEQ( m_verticalShift );
 _NEQ( m_horizontalShift );


#undef _NEQ
   // redefine for hash cell
#define _NEQ( x ) if ( newCell.x != oldCell.x ) { \
cout << "MapHashCell[" << v << "][" << h << "] item: " << #x << " differs. " \
<< " new/old: " << newCell.x << "/" << oldCell.x << endl; \
return false; \
}

   for ( uint32 v = 0; v < first.m_nbrVerticalCells; ++v ) {
      for ( uint32 h = 0; h < first.m_nbrHorizontalCells; ++h ) {
         MapHashCell& newCell = 
            dynamic_cast<MapHashCell&>(*first.m_hashCell[ v ][ h ]);
         OldMapHashCell& oldCell =
            dynamic_cast<OldMapHashCell&>(*second.m_hashCell[ v ][ h ]);

         _NEQ( m_topVertical );
         _NEQ( m_bottomVertical );
         _NEQ( m_rightHorizontal );
         _NEQ( m_leftHorizontal );

         _NEQ( getNbrElements() );

         for ( uint32 i = 0; i < newCell.getNbrElements(); ++i ) {
            if ( newCell.getItemID( i ) !=
                 oldCell.m_allItemID->getElementAt( i ) ) {
               cout << "allItemID at position: " << i  << "differs. "
                    << "old/new" << newCell.getItemID( i ) << "/"
                    <<  oldCell.m_allItemID->getElementAt( i ) << endl;
               return false;
            }
         }
      }
   }

   return true;
}


namespace {

// for debug
void printResults( const GetAdditionalPOIInfo::poiResultMap_t& results ) {

   std::map<uint32, GetAdditionalPOIInfo::resultMap_t>::
      const_iterator poiIt = results.begin();

   for (; poiIt != results.end(); ++poiIt ) {
      mc2dbg << "--------- id:" << (*poiIt).first << endl;

      GetAdditionalPOIInfo::resultMap_t::const_iterator 
         dataIt = (*poiIt).second.begin();

      for ( ; dataIt != (*poiIt).second.end(); ++dataIt ) {
         mc2dbg << (*dataIt).first << " : " << (*dataIt).second << endl;
      }
      mc2dbg << endl;
   }
}

/// @return size in buffer
uint32 appendToStream( ostream& ostr,
                       uint32 staticID,
                       bool dynamicInfo,
                       const GetAdditionalPOIInfo::resultMap_t& result ) {

   //
   // first calculate size in buffer
   //

   // size of number of infos, static id and dynamic 
   uint32 blockSize = 3*sizeof( uint32 ); 
   GetAdditionalPOIInfo::resultMap_t::const_iterator it = 
      result.begin(), itEnd = result.end();

   for ( ; it != itEnd; ++it ) {
      blockSize += 2 * sizeof( uint32 ) +  // language + type size
         (*it).second.first.size() + 1; // string size + ending null
      AlignUtility::alignLong( blockSize );
   }

   //
   // Write data to buffer
   //
     
   DataBuffer buff( blockSize );
   buff.fillWithZeros();
   buff.writeNextLong( staticID ); // static ID
   buff.writeNextLong( dynamicInfo ); // dynamic information
   buff.writeNextLong( result.size() ); // number of infos

   it = result.begin();
   for ( ; it != itEnd; ++it ) {
      buff.alignToLong();
      buff.writeNextLong( (*it).first ); // type
      buff.writeNextLong( (*it).second.second ); // language
      buff.writeNextString( (*it).second.first.c_str() ); // string value
   }
   buff.alignToLong();   
   // must be filled at this point, else buffer size
   // calculation is wrong.
   // actually, skip this. No need to optimize the extra
   // memory we use at this point.
   //   MC2_ASSERT( buff.bufferFilled() );

   ostr.write( (char*)( buff.getBufferAddress() ), 
               buff.getBufferSize() );
   return buff.getBufferSize();
}

/**
 * append all items to the stream                 
 * @param ostr the stream to append items to
 * @param sqlIDs IDs for each result, used to maintain the order in which
 *        they were requested in, since the results is a map and can reorder
 *        them.
 * @param offsets will be filled in with the offsets into the file where the
 *        information about each item is.
 */
void appendToStream( ostream& ostr, 
                     map<uint32, uint32>& sqlIDMap,
                     const vector<uint32>& staticIDs,
                     const vector<uint32>& sqlIDs,
                     const set<uint32>& dynamicSQLIDs,
                     const GetAdditionalPOIInfo::poiResultMap_t& results,
                     vector<uint32>& offsets ) {
   uint32 lastOffset = 0;
   
   for ( uint32 i = 0; i < sqlIDs.size(); ++i ) {
      // make sure we actually have information about this POI in the db.
      GetAdditionalPOIInfo::poiResultMap_t::const_iterator poiIt =
         results.find( sqlIDs[ i ] );

      offsets.push_back( lastOffset );
      // check for dynamic information
      bool dynamicInfo = ( dynamicSQLIDs.find( sqlIDs[ i ] ) != 
                           dynamicSQLIDs.end() );

      if ( poiIt == results.end() ) {
         // add empty information
         lastOffset += appendToStream( ostr, staticIDs[ i ], 
                                       dynamicInfo,
                                       GetAdditionalPOIInfo::resultMap_t() );
      } else {
         // else add all information
         lastOffset += appendToStream( ostr, staticIDs[ i ], 
                                       dynamicInfo,
                                       (*poiIt).second );
      }
   }
}

}

void M3Creator::generatePOIInfo( const OldGenericMap& oldMap,
                                 GenericMap& map,
                                 const MC2String& filename ) 
   const throw (FileUtils::FileException) {


   ofstream file( filename.c_str(), ios::app );
   if ( ! file  ) {
      throw FileUtils::FileException( "Could not open file " + filename );
   }

   mc2dbg << "[M3Creator]: startOffset = " << file.tellp() << endl;

   //   const uint32 nbrItems = oldMap.getNbrItemsWithZoom( ItemTypes::poiiZoomLevel );         
   // for each item; find POI item in in POI item layer 
   // and grab information from POI database; Append the information
   // to the file
   vector<uint32> sqlIDs;
   std::map<uint32, uint32> sqlIDMap;
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      uint32 n = oldMap.getNbrItemsWithZoom( z );
      for ( uint32 itemIndex = 0; itemIndex < n; ++itemIndex) { 
         const OldItem* poi = oldMap.getItem( z, itemIndex );
         if ( poi == NULL || 
              poi->getItemType() != ItemTypes::pointOfInterestItem ) {
            continue;
         }
         
         // got a POI item, add database id.
         sqlIDs.push_back( static_cast<const OldPointOfInterestItem*>(poi)->
                           getWASPID() );
         sqlIDMap[ static_cast<const OldPointOfInterestItem*>(poi)->
                   getWASPID() ] = poi->getID();
      }
   }


   GetAdditionalPOIInfo poiDB( PropertyHelper::get<MC2String>("POI_SQL_DATABASE").c_str(),
                               PropertyHelper::get<MC2String>("POI_SQL_HOST").c_str(),
                               PropertyHelper::get<MC2String>("POI_SQL_USER").c_str(),
                               PropertyHelper::get<MC2String>("POI_SQL_PASSWORD").c_str());

   // fetch normal POI information
   GetAdditionalPOIInfo::poiResultMap_t results;
   if ( poiDB.precache( &*sqlIDs.begin(), &*sqlIDs.end(),
                        // interested in all key IDs
                        set<GetAdditionalPOIInfo::keyID_t>(), 
                        results ) == -1 ) {
      mc2dbg << fatal << "Failed to get poi information. " << endl;
      MC2_ASSERT( false );
   }

   // get static ids
   vector<uint32> staticIDs;
   if ( poiDB.getStaticIDs( &*sqlIDs.begin(), &*sqlIDs.end(),
                            staticIDs ) == -1 ) {
      mc2dbg << fatal << "Failed to get static POI IDs" << endl;
      MC2_ASSERT( false );
   }

   set<uint32> dynamicIDs;
   poiDB.getDynamicIDs( dynamicIDs );
   mc2dbg << "[M3Creator] Nbr dynamic ids: " << dynamicIDs.size() << endl;

   //         ::printResults( results );

   // prepare offset vector
   vector<uint32> offsets; // offsets in POI file
   offsets.reserve( sqlIDs.size() );      
   ::appendToStream( file, sqlIDMap, 
                     staticIDs, sqlIDs, dynamicIDs,
                     results, offsets );
   

   // set poi offsets 
   // note: this is not "real" POI offset, 
   // since its the poi offset in the temporary file not the real map file.
   // this offset table will be updated when the file is loaded in GenericMap
   map.m_poiOffsetTable.allocate( offsets.size() );
   copy( offsets.begin(), offsets.end(),
         map.m_poiOffsetTable.begin() );

   mc2dbg << "[M3Creator] Nbr POIs really added"
          << " (using offset count) " << offsets.size() << endl;
}

void M3Creator::saveM3Map( const OldGenericMap& oldMap,
                           const MC2String& filename ) 
const throw (FileUtils::FileException) {

   auto_ptr<GenericMap> map( createM3Map( oldMap ) );

   MC2_ASSERT( map->getCreationTime() == oldMap.getCreationTime() );

   // create temporary file to hold POI information which will be
   // appended to the map file. Temp file will be in the same directory
   // as the map filename.

   TempFile poiInfoFile( "poi_info", STLStringUtility::dirname( filename ) );

   generatePOIInfo( oldMap, *map, poiInfoFile.getTempFilename() );

   // now we can save the map with valid POI Information table
   if ( ! map->save( filename.c_str(), false ) ) {
      throw FileUtils::FileException( MC2String("Failed to save file: ") + 
                                      filename );
   }

   // append poi information from POI temp file to the map file
   ofstream mapFile( filename.c_str(), ios::app );
   uint32 currentSize = mapFile.tellp();
   uint32 alignedSize = currentSize;
   AlignUtility::alignLong( alignedSize );
   if ( currentSize != alignedSize ) {
      // write some empty bytes until currentSize = alignedSize
      const uint32 emptySize = alignedSize - currentSize;
      char buffer[ emptySize ];
      memset( buffer, 0, emptySize );
      mapFile.write( buffer, emptySize );
      // now it should be aligned to long
   }
   mc2log << "[M3Creator] Writing to offset: " << alignedSize << endl;
   ifstream poiFile( poiInfoFile.getTempFilename().c_str() );
   mapFile << poiFile.rdbuf();
}



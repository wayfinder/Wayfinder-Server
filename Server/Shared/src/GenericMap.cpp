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
#include "GenericMap.h"

#include "NonStdStl.h"
#include "MapHashTable.h"
#include "GfxUtility.h"

#include "BinarySearchTree.h"
#include "ItemTypes.h"
#include "ItemAllocator.h"

#include "AircraftRoadItem.h"
#include "AirportItem.h"
#include "BuildingItem.h"
#include "BuiltUpAreaItem.h"
#include "BusRouteItem.h"
#include "CategoryItem.h"
#include "CityPartItem.h"
#include "FerryItem.h"
#include "ForestItem.h"
#include "GroupItem.h"
#include "IndividualBuildingItem.h"
#include "IslandItem.h"
#include "MunicipalItem.h"
#include "Node.h"
#include "NullItem.h"
#include "ParkItem.h"
#include "PedestrianAreaItem.h"
#include "PointOfInterestItem.h"
#include "RailwayItem.h"
#include "RouteableItem.h"
#include "StreetItem.h"
#include "StreetNbr.h"
#include "StreetSegmentItem.h"
#include "StringSearchUtility.h"
#include "StringUtility.h"
#include "SubwayLineItem.h"
#include "WaterItem.h"
#include "ZipAreaItem.h"
#include "ZipCodeItem.h"
#include "CartographicItem.h"

#include "GfxDataMainFactory.h"

#include "ItemComboTable.h"

#include "OverviewMap.h"

#include "CountryOverviewMap.h"

#include "POIInfo.h"

#include "GfxDataFull.h"

#include "GfxUtility.h"

#include "ItemIdentifier.h"
#include "ExternalConnections.h"

#include "DataBuffer.h"

#include "AllocatorTemplate.h"
#include "MC2MapAllocator.h"

#include "STLStringUtility.h"
#include "UserRightsItemTable.h"
#include "UserRightsMapInfo.h"

#include "DataBufferUtil.h"
#include "STLUtility.h"

#include "DebugClock.h"
#include "ScopedClock.h"
#include "FunctionClock.h"

#include "Properties.h"
#include "DataBufferCreator.h"

#include "Packet.h"
#include "MapBits.h"
#include "Math.h"

// Define this to use the diff between
// process size before and after loading as the map size
#undef  USE_PROCESSSIZE_FOR_MAP_SIZE

#ifdef USE_PROCESSSIZE_FOR_MAP_SIZE
#include "SysUtility.h"
#endif

#include <fstream>
#include <iterator>
#include <memory>
#include <utime.h>

using namespace STLUtility;

namespace {

   /**
    *    Comparator for the table of good coordinates for
    *    items.
    */
   class AdminAreaCentreCmp {
   public:
      /// For is_sorted
      bool operator()(const GenericMap::adminAreaCentre_t& a,
                      const GenericMap::adminAreaCentre_t& b) const {
         return a.itemID < b.itemID;
      }
      /// For lower_bound
      bool operator()(const GenericMap::adminAreaCentre_t& a,
                      uint32 itemID) const {
         return a.itemID < itemID;
      }
   };
   
}

GenericMap*
GenericMap::createMap(uint32 id, const char* path)
{
#ifdef USE_PROCESSSIZE_FOR_MAP_SIZE
   uint32 sizeBefore = SysUtility::getProcessTotalSize();
#endif
   // Variables to return/get the map status
   GenericMap* theNewMap = NULL;

   // Create a map with correct type
   if (MapBits::isUnderviewMap(id)) {
      theNewMap = new Map(id, path);
   } else if (MapBits::isOverviewMap(id)) {
      theNewMap = new OverviewMap(id, path);
   } else {
      theNewMap = new CountryOverviewMap(id, path);
   }

   // Create the allocators in the new map
   theNewMap->createAllocators();

   // Fill the new map with data
   bool newMapOK = theNewMap->load();

   // Find out if the map is ok and set the status of the creation.
   // If it's not OK, it will be deleted!
   if (!newMapOK) {
      delete theNewMap;
      theNewMap = NULL;      
   }

#ifdef USE_PROCESSSIZE_FOR_MAP_SIZE
   // This will not work if we do too much in other threads while loading.
   if ( theNewMap ) {
      if ( SysUtility::getProcessTotalSize() != 0 ) {
         theNewMap->m_approxMapSize =
            SysUtility::getProcessTotalSize() - sizeBefore;
      }
   }
#endif
   
   return (theNewMap);
}

GenericMap*
GenericMap::createMap( const char* mcmName )
{
   // Extract the ID and path from mcmName
   char path[256];
   path[0] = '\0';
   uint32 mapID, mapVersion;
   bool ok = false;

   // Find the last '/'
   const char* slashPtr = strrchr( mcmName, '/' );

   if ( slashPtr == NULL ) {
      // No slash, see if id + version + ".m3" is provided
      if ( sscanf( mcmName, "%x-%d.m3", &mapID, &mapVersion ) == 2 ) {
         strcat( path, "./" );
         ok = true;
      }
   } else {
      // Got a slash, get mapID
      if ( sscanf( slashPtr, "/%x-%d.m3", &mapID, &mapVersion ) == 2 ) {
         // Got mapID, get path
         int n = slashPtr - mcmName + 1;
         MC2_ASSERT( n < 256 );
         if ( strncpy( path, mcmName, n ) != NULL ) {
            path[ n ] = '\0';
            ok = true;
         }
      }
   }
   
   mc2dbg2 << "   mcmName=" << mcmName << endl;
   mc2dbg2 << "   path=" << path << ", mapID=" << mapID << endl;

   return ok ? createMap( mapID, path ) : NULL;
}

#define GENERICMAP_STL_INIT \
   m_stlAllocator( 50000 ), \
   m_nodeLane( m_stlAllocator ), \
   m_connectionLaneIdx( m_stlAllocator ), \
   m_categoryIds( m_stlAllocator ), \
   m_indexAreaOrderMap( m_stlAllocator )
GenericMap::GenericMap()
      : GENERICMAP_STL_INIT
{
   initEmptyMap(MAX_UINT32);
}

GenericMap::GenericMap(uint32 id)
      : GENERICMAP_STL_INIT
{
   initEmptyMap(id);
}

uint32
GenericMap::getApproxMapSize() const
{
   return m_approxMapSize;
}

void
GenericMap::initEmptyMap(uint32 id) 
{
   // NOTE! if you change the order of these the map format will change
#define ADDA(x,y) x = new ItemAllocator<y>; m_itemAllocators.push_back(x)
   ADDA( m_streetSegmentItemAllocator, StreetSegmentItem );
   ADDA( m_streetItemAllocator, StreetItem );
   ADDA( m_municipalItemAllocator, MunicipalItem );
   ADDA( m_cityPartItemAllocator, CityPartItem );
   ADDA( m_waterItemAllocator, WaterItem );
   ADDA( m_parkItemAllocator, ParkItem );
   ADDA( m_forestItemAllocator, ForestItem );
   ADDA( m_buildingItemAllocator, BuildingItem );
   ADDA( m_railwayItemAllocator, RailwayItem );
   ADDA( m_islandItemAllocator, IslandItem );
   ADDA( m_zipCodeItemAllocator, ZipCodeItem );
   ADDA( m_zipAreaItemAllocator, ZipAreaItem );
   ADDA( m_pointOfInterestItemAllocator, PointOfInterestItem );
   ADDA( m_categoryItemAllocator, CategoryItem );
   ADDA( m_builtUpAreaItemAllocator, BuiltUpAreaItem );
   ADDA( m_busRouteItemAllocator, BusRouteItem );
   ADDA( m_airportItemAllocator, AirportItem );
   ADDA( m_aircraftRoadItemAllocator, AircraftRoadItem );
   ADDA( m_pedestrianAreaItemAllocator, PedestrianAreaItem );
   ADDA( m_militaryBaseItemAllocator, Item );
   ADDA( m_individualBuildingItemAllocator, IndividualBuildingItem );
   ADDA( m_subwayLineItemAllocator, SubwayLineItem );
   ADDA( m_nullItemAllocator, NullItem );
   ADDA( m_ferryItemAllocator, FerryItem );
   ADDA( m_simpleItemAllocator, Item );
   ADDA( m_cartographicItemAllocator, CartographicItem );
#undef ADDA

   m_approxMapSize = 0;
   m_mapID = id;
   m_segmentsOnTheBoundry = NULL;
   m_itemNames = NULL;
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      m_itemsZoom[i] = NULL;
      m_itemsZoomSize[i] = 0;
      m_itemsZoomAllocated[i] = 0; 
   }
   m_gfxData = NULL;


   // Set all allocators to NULL to be able to delete them

   m_connectionAllocator = NULL;
   m_coordinateAllocator = NULL;
   m_laneAllocator = NULL;
   m_categoryAllocator = NULL;
   m_signPostAllocator = NULL;

   /// Create new UserRightsItemTable. It is empty
   m_userRightsTable = new UserRightsItemTable( ~MapRights() );
   /// Default argument to functions taking UserRightsMapInfo
   m_allRight = new UserRightsMapInfo( m_mapID, ~MapRights() );

   // The adminAreaCentre table
   m_nbrAdminAreaCentres = 0;
}

void
GenericMap::createAllocators()
{
   m_gfxDataFactory.reset( new GfxDataMainFactory( 0, 0, 0, 0, 0 ) );
   m_connectionAllocator = new MC2Allocator<Connection>(0);
   m_coordinateAllocator = new MC2Allocator<TileMapCoord>(0);
   m_laneAllocator = new MC2ArrayAllocator< class ExpandStringLane >( 0 );
   m_categoryAllocator = new CategoryAllocator( 0 );
   m_signPostAllocator = new MC2ArrayAllocator< class SignPost >( 0 );
}

void
GenericMap::initNonItemAllocators( uint32 nbrGfxDatas, 
                                   uint32 nbrGfxDataSingleSmallPoly,
                                   uint32 nbrGfxDataSingleLine, 
                                   uint32 nbrGfxDataSinglePoint,
                                   uint32 nbrGfxDataMultiplePoints, 
                                   uint32 nbrNodes, 
                                   uint32 nbrConnections,
                                   uint32 nbrCoordinates,
                                   uint32 nbrLanes,
                                   uint32 nbrCategories,
                                   uint32 nbrSignPosts ) 
{
   uint32 useGfxDataFullOnly = Properties::
      getUint32Property( "USE_GFXDATAFULL_ONLY", 0 );

   if ( useGfxDataFullOnly == 0 ) {
      mc2dbg << "GenericMap: Using all GfxData types." << endl;
      m_gfxDataFactory.reset( new 
                              GfxDataMainFactory( nbrGfxDatas,
                                                  nbrGfxDataSingleSmallPoly,
                                                  nbrGfxDataSingleLine,
                                                  nbrGfxDataSinglePoint,
                                                  nbrGfxDataMultiplePoints
                                                  ) );
   } else {
      mc2dbg << "GenericMap:: Using GfxDataFull Only. " << endl;
      m_gfxDataFactory.reset( new 
                              GfxDataFullFactory( nbrGfxDatas +
                                                  nbrGfxDataSingleSmallPoly + 
                                                  nbrGfxDataSingleLine +
                                                  nbrGfxDataSinglePoint +
                                                  nbrGfxDataMultiplePoints
                                                  ) );
   }

   m_connectionAllocator->reallocate(nbrConnections);
   m_coordinateAllocator->reallocate( nbrCoordinates );
   m_laneAllocator->reallocate( nbrLanes );
   m_categoryAllocator->reallocate( nbrCategories );
   m_signPostAllocator->reallocate( nbrSignPosts );
}


GenericMap::GenericMap(uint32 mapID, const char *path)
      : GenericMapHeader(mapID, path)
{
   mc2dbg4 << "GenericMap " << mapID << " to be created" << endl;

   initEmptyMap(mapID);

   // Set the filename of the map
   setFilename(path);

}
MC2Allocator<Connection>& 
GenericMap::getConnectionAllocator() {
   return *m_connectionAllocator;
}

const MC2Allocator<Connection>& 
GenericMap::getConnectionAllocator() const {
   return *m_connectionAllocator;
}

MC2Allocator<TileMapCoord>& 
GenericMap::getCoordinateAllocator() 
{
   return *m_coordinateAllocator;
}

void
GenericMap::createBoundrySegmentsVector()
{
   if ( m_segmentsOnTheBoundry == NULL ) {
      m_segmentsOnTheBoundry = new BoundrySegmentsVector();
   }
}


GenericMap::~GenericMap()
{
   mc2dbg2 << "~GenericMap enter" << endl;

   DebugClock clock;

   delete m_itemNames;
   DEBUG_DEL(mc2dbg << "~GenericMap itemNames destr." << endl) ;

   // FIXME: Do NOT delete the items since they are created inside an Allocator
   if (m_itemsZoomSize != NULL) {
      for (uint32 i=0; i < NUMBER_GFX_ZOOMLEVELS; i++) {
         delete [] m_itemsZoom[i];
      }
   }

   DEBUG_DEL(mc2dbg << "GenericMap::~GenericMap All items destr." << endl);


   delete m_connectionAllocator;
   delete m_coordinateAllocator;
   delete m_laneAllocator;
   delete m_categoryAllocator;

   delete m_segmentsOnTheBoundry;
   DEBUG_DEL(mc2dbg << "~GenericMap segmentsOnTheBoundry destr." << endl) ;
   mc2dbg4 <<"~GenericMap after segments on boundry dtor"<< endl;

   //delete m_gfxData;
   // Created in the allocators!!!
   DEBUG_DEL(mc2dbg << "GenericMap::~GenericMap gfxData destr." << endl) ;
   mc2dbg4 <<"~GenericMap after gfxData dtor"<< endl;

   delete m_currency;
   delete m_userRightsTable;
   delete m_allRight;

   for_each( m_itemAllocators.begin(), m_itemAllocators.end(),
             STLUtility::DeleteObject() );

//    for_each( m_signPostMap.begin(), m_signPostMap.end(),
//              STLUtility::DeleteMapValue<SignPostMap>() );

   mc2dbg4 << "~GenericMap exit" << endl;

   mc2dbg4 << "Map " << getMapID() << " deleted in " 
           << clock << endl;
}

bool
GenericMap::internalLoad(DataBuffer& dataBuffer)
{
   if ( ! GenericMapHeader::internalLoad(dataBuffer) ) {
      return false;
   }
   
   // Seems like the size of the map in mem compared to disk ranges
   // from 1.7 to 2.5
   m_approxMapSize = uint32( dataBuffer.getBufferSize() * 2.3);

   mc2dbg << "GenericMap::internalLoad" << endl;

 
   if ( m_loadedVersion > 5 ){
      // Loading whith variable map body size.

      uint32 startOffset = dataBuffer.getCurrentOffset();
      DataBuffer tmpBuf( dataBuffer.getBufferAddress(),
                         dataBuffer.getBufferSize() );
      tmpBuf.readPastBytes(startOffset);
      uint32 mapBodySize = tmpBuf.readNextLong();
      mc2dbg << "GenericMap body size: " << mapBodySize << " read." 
      << endl;
      uint32 endOffset = startOffset + mapBodySize;

      
      // Static generic map body loading.
      if (!loadFirstBodyBlock(tmpBuf)){
         return false;
      }

      // Variable size generic map body loading.

      // Load user rights table
      if ( tmpBuf.getCurrentOffset() < endOffset ){
         mc2dbg << "Loading m_userRightsTable" << endl;
         m_userRightsTable->load( tmpBuf );
      }

      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading street side table" << endl;
         m_streetSideTable.load( tmpBuf );
      }
      mc2dbg << "Bits in index: " << m_streetSideTable.getNbrOfBitsInIndex() << endl;
      // Load m_adminAreaCentres table (m_loadedVersion >= 7)
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         m_nbrAdminAreaCentres = tmpBuf.readNextLong();
         mc2dbg << "Loading m_adminAreaCentres table with "
                << m_nbrAdminAreaCentres << " records" << endl;

         m_adminAreaCentres.reset( NULL );

         if ( m_nbrAdminAreaCentres > 0 ) {
            m_adminAreaCentres.reset( new
                                adminAreaCentre_t[m_nbrAdminAreaCentres] );
            for ( uint32 i = 0; i < m_nbrAdminAreaCentres; i++ ) {
               adminAreaCentre_t elem;
               elem.itemID = tmpBuf.readNextLong();
               int32 lat = tmpBuf.readNextLong();
               int32 lon = tmpBuf.readNextLong();
               elem.centre = MC2Coordinate(lat, lon);
               m_adminAreaCentres[i] = elem;
            }
            // Check sorting.
            MC2_ASSERT (
               is_sorted( m_adminAreaCentres.get(),
                          m_adminAreaCentres.get() + m_nbrAdminAreaCentres,
                          AdminAreaCentreCmp() ) );
         }
      }

      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         loadRoadDisplayClasses( tmpBuf );
      }

      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         loadAreaDisplayClasses( tmpBuf );
      }

      // Test if everything has been read.
      if ( tmpBuf.getCurrentOffset() < endOffset ){
         mc2log << warn << "Something in map body could not be loaded by"
                << " this version of the code." << endl;
         mc2log << info << "Loaded map version: " << (int)m_loadedVersion 
                << endl;
         mc2dbg8 <<  "Offset:     " << tmpBuf.getCurrentOffset() << endl;
         mc2dbg8 <<  "End Offset: " << endOffset << endl;
      }

      if ( ! MapBits::isOverviewMap( getMapID() ) ) {
         // copy poi info data from databuffer
         // Note: for overview maps, this buffer is loaded in their
         //       respective internalLoad
         copyPOIInfoData( tmpBuf );
      }

      dataBuffer.readPastBytes(mapBodySize);

      doMapTricks(); //Do testing changes in this function.
   }
   else {
      // Loading with static map body size.
      
      mc2dbg << "Only loading static map body" << endl;
      if (!loadFirstBodyBlock(dataBuffer)){
         return false;
      }
   }
   return true;
}

void
GenericMap::doMapTricks()
{
#if 0
   // Testing
   // Fill the UserRightsItemTable
   // Everything with a name that contains a "b" or "B" is forbidden.
   UserRightsItemTable::itemMap_t urMap;
   m_userRightsTable->getItems( urMap );
   mc2dbg << "[GenericMap:" << __LINE__ << "] " << *m_userRightsTable << endl;
   for ( uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         if(Item* item = getItem(z, i)){
            // Do this only for the POI:s, since they are the important ones.
            if ( item->getItemType() == ItemTypes::pointOfInterestItem ) {
#if 0
               bool b = false;
               for ( byte n = 0; n < item->getNbrNames(); ++n ) {
                  if( strchr( m_itemNames->getString(
                                                     item->getStringIndex(n)), 'b'  ) ||
                      strchr( m_itemNames->getString(
                                                     item->getStringIndex(n)), 'B' )  ) {
                     b = true;
                     break;
                  }
               }
               if ( b ) {
                  urMap[ item->getID() ] = MapRights::FREE_TRAFFIC;
               } else {
                  urMap[ item->getID() ] = ~MapRights();
               }
                  
#else
               PointOfInterestItem* poi = item_cast<PointOfInterestItem*>(item);
               if( poi->getPointOfInterestType() == ItemTypes::wlan ){
                  urMap[ item->getID() ] = MapRights::FREE_TRAFFIC;
                  mc2log << "[GenericMap] setting poi " << hex << item->getID() 
                         << " rights to " << dec
                         << urMap[item->getID()] << endl;
               } else {
                  //                  urMap[ item->getID() ] = ~MapRights();
               }
#endif
            }
         }
      }
   }
   UserRightsItemTable newOne( ~MapRights(), urMap );
   // Set the new one
   m_userRightsTable->swap( newOne );
   mc2dbg << "[GenericMap:" << __LINE__ << "] " << *m_userRightsTable << endl;
#endif
}

bool
GenericMap::loadFirstBodyBlock(DataBuffer& dataBuffer)
{
   mc2dbg4 << "loadFirstBodyBlock enter" << endl;
   DebugClock clock;

   FunctionClock( functionClock );

   // The allocators
   uint32 nbrItemsPerType[GenericMapHeader::numberInitialItemTypeAllocators];
   fill_n( nbrItemsPerType, GenericMapHeader::numberInitialItemTypeAllocators,
           0 );

   for ( uint32 t=0;
         t < GenericMapHeader::numberInitialItemTypeAllocators; ++t) {
      uint32 itemType = dataBuffer.readNextLong();
      nbrItemsPerType[ itemType ] = dataBuffer.readNextLong();
      mc2dbg2 << "Load: " << nbrItemsPerType[ itemType ] 
              << " items with type " << itemType << endl;
   }
   // Make sure we don't have any nullItems
   nbrItemsPerType[ uint32(ItemTypes::nullItem) ] = 0;
  
   // The "extra" allocators
   uint32 nbrGfxDatas = dataBuffer.readNextLong();
   uint32 nbrNodes = dataBuffer.readNextLong();
   uint32 nbrConnections = dataBuffer.readNextLong();
   uint32 nbrCoordinates = dataBuffer.readNextLong();
   uint32 nbrGfxDataSingleSmallPoly = 0;
   uint32 nbrGfxDataSingleLine = 0;
   uint32 nbrGfxDataSinglePoint = 0;
   uint32 nbrGfxDataMultiplePoints = 0;
   uint32 nbrSimpleItems = 0;
   uint32 nbrLanes = 0;
   uint32 nbrCategories = 0;
   uint32 nbrSignPosts = 0;
   // Check how many extra allocators there are left to read excluding the
   // already read ones.
   int nbrAllocatorsLeft = m_nbrAllocators - 
         GenericMapHeader::numberInitialItemTypeAllocators - 4;
   
   if ( nbrAllocatorsLeft > 0 ) {
      nbrGfxDataSingleSmallPoly = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrGfxDataSingleLine = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrGfxDataSinglePoint = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrGfxDataMultiplePoints = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrSimpleItems = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrLanes = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrCategories = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      nbrSignPosts = dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }

   // Read the number of new allocators that we don't know about yet.
   while ( nbrAllocatorsLeft > 0 ) {
      dataBuffer.readNextLong();
      --nbrAllocatorsLeft;
   }

   MINFO("internalLoad_t before GfxData");

   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Allocator info loaded in "
                        << clock << endl;
                 clock = DebugClock());

   initNonItemAllocators( nbrGfxDatas, nbrGfxDataSingleSmallPoly,
                          nbrGfxDataSingleLine, nbrGfxDataSinglePoint,
                          nbrGfxDataMultiplePoints, nbrNodes, 
                          nbrConnections, nbrCoordinates, nbrLanes, 
                          nbrCategories, nbrSignPosts );
   // print number of allocators
#define _VALUE(x) ", " << #x << "=" << x
   mc2dbg << "[" << prettyMapIDFill( m_mapID ) 
          << "] #gfxDataFull=" << nbrGfxDatas 
          << _VALUE( nbrGfxDataSingleSmallPoly )
          << _VALUE( nbrGfxDataSingleLine )
          << _VALUE( nbrGfxDataSinglePoint )
          << _VALUE( nbrGfxDataMultiplePoints )
          << _VALUE( nbrCoordinates )
          << _VALUE( nbrNodes )
          << _VALUE( nbrConnections )
          << _VALUE( nbrSimpleItems ) 
          << _VALUE( nbrLanes )
          << _VALUE( nbrCategories )
          << _VALUE( nbrSignPosts )
          << "\n # pois = "
          << (nbrItemsPerType[int(ItemTypes::pointOfInterestItem)] )
          << "\n #ssis = "
          << nbrItemsPerType[int(ItemTypes::streetSegmentItem)]
          << endl;
#undef _VALUE

   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Allocators allocated in "
                        << clock << endl;
                 clock = DebugClock());


   // ***************************************************************
   //                                                  Create GfxData
   // ***************************************************************
   // Please notice that the allocators must be created before the
   // GfxData for the map is created!

   DEBUG_DB(mc2dbg << "load gfxData for the entire map" << endl;)

   // Read past length
   uint32 gfxLength = dataBuffer.readNextLong();
   
   mc2dbg << "GfxDataLength is " << gfxLength << endl;
   
   m_gfxData = createNewGfxData( &dataBuffer );
      
   mc2dbg4 << "internalLoad_t after GfxData" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                 << "] GfxData created in "
                 << clock << endl;
                 clock = DebugClock());
   // ***************************************************************
   //                                            Read all the strings
   // ***************************************************************
   if ( m_itemNames == NULL ) {
      m_itemNames = new ItemNames;
   }

   {
      ScopedClock clock(" itemNamesClock  ");
      m_itemNames->internalLoad( dataBuffer, stringsCodedInUTF8() );
   }

   mc2dbg4 << "internalLoad_t after ItemNames" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] ItemNames created in "
                        << clock << endl;
                 clock = DebugClock());


   m_groups.load( dataBuffer );
   m_names.load( dataBuffer );
   m_itemsInGroup.load( dataBuffer );
   m_vehicleRestrictions.load( dataBuffer );
   m_poiOffsetTable.load( dataBuffer );

   CLOCK_MAPLOAD( mc2log << "[GenericMap] groups, names, itemsIngroup and "
                  << "vehicleRestrictions loaded in " << clock  << endl;
                  clock = DebugClock() );

   {
      ScopedClock itemClock(" itemCreateClock ");
      // setup zoom levels
      for (uint32 i=0; i< NUMBER_GFX_ZOOMLEVELS; ++i ) {
         m_itemsZoomAllocated[ i ] = m_itemsZoomSize[ i ] = dataBuffer.readNextLong();
         delete [] m_itemsZoom[ i ];
         m_itemsZoom[ i ] = new Item*[ m_itemsZoomSize[ i ] ];
         fill_n( m_itemsZoom[i], m_itemsZoomSize[ i ], (Item*)(NULL));
      }
   }

   // ***************************************************************
   // Initiate allocators and load items
   // ***************************************************************
   {
      ScopedClock itemClock(" itemLoadClock ");
      // load items
      ItemAllocatorContainer::iterator it = m_itemAllocators.begin();
      ItemAllocatorContainer::iterator itEnd = m_itemAllocators.end();
      for (; it != itEnd; ++it ) {
         // dont check time now
         //ScopedClock specificClock( typeid( **it ).name() );
         (*it)->load( dataBuffer, *this );
      }
   }
   CLOCK_MAPLOAD( mc2log << "Items loaded in " << clock << endl;
                  clock = DebugClock() );

   // ******************************************************************
   //                                          Read external connections
   // ******************************************************************
   uint32 externalSize = dataBuffer.readNextLong();

   mc2dbg << "Size of boundry segments:" << externalSize << endl;

   if ( externalSize > 0 ) {
      //      FIXME mapset
      m_segmentsOnTheBoundry = new BoundrySegmentsVector( &dataBuffer, this );
   }


   mc2dbg4 << "internalLoad_t after seg. on boundry" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Seg. on boundry created in "
                        << clock << endl;
                 clock = DebugClock());

   // ***************************************************************
   //                                              Fill the hashtable
   // ***************************************************************
   {
      ScopedClock hashClock( " hashTableClock " );
      m_hashTable.reset( new MapHashTable( *this ) );
      m_hashTable->load( dataBuffer );
   }

      
   mc2dbg4 << "internalLoad_t after hashtable" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Hash table created in "
                        << clock << endl;
                 clock = DebugClock());

   // ******************************************************************
   //                                            Read the landmark table
   // ******************************************************************
   mc2dbg2 << "To load the landmark table" << endl;
   uint32 landmarkSize = dataBuffer.readNextLong();
   if (landmarkSize > 0) {
      for (uint32 i = 0; i < landmarkSize; i++) {
         uint32 fromNodeID = dataBuffer.readNextLong();
         uint32 toNodeID = dataBuffer.readNextLong();
         ItemTypes::lmdescription_t description;
            description.itemID = dataBuffer.readNextLong();
            description.importance = dataBuffer.readNextByte();
            description.side = 
               SearchTypes::side_t(dataBuffer.readNextByte());
            description.location = 
               ItemTypes::landmarklocation_t(dataBuffer.readNextByte());
            description.type = 
               ItemTypes::landmark_t(dataBuffer.readNextByte());
            
            uint64 key = (uint64(fromNodeID) << 32) | uint64(toNodeID);
            m_landmarkTable.insert(
                  pair<uint64, ItemTypes::lmdescription_t>(key, description));
      }
   }
   
   mc2dbg4 << "internalLoad_t after landmark table" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Landmark table loaded in "
                        << clock << endl;
                 clock = DebugClock());

   // ******************************************************************
   //                                      Read the node expansion table
   // ******************************************************************

   mc2dbg2 << "To load the node expansion table" << endl;
   {
      ScopedClock expansionClock(" expansionClock ");
   if ( dataBuffer.getCurrentOffset() < dataBuffer.getBufferSize() ) {
      uint32 nodeExpSize = dataBuffer.readNextLong();
      if (nodeExpSize > 0) {
         for (uint32 i = 0; i < nodeExpSize; i++) {
            uint32 fromNodeID = dataBuffer.readNextLong();
            uint32 toNodeID = dataBuffer.readNextLong();
            // Add to the table.
            m_nodeExpansionTable[ make_pair( fromNodeID, toNodeID ) ] =
               expandedNodes_t();
               
            expandedNodes_t& expandedNodes(
                     m_nodeExpansionTable[ make_pair( fromNodeID, toNodeID ) ]);

            uint32 nbrExpandedNodes = dataBuffer.readNextLong();
            for ( uint32 i = 0; i < nbrExpandedNodes; ++i ) {
               expandedNodes.push_back( dataBuffer.readNextLong() );
               mc2dbg8 << "  exp node : " << expandedNodes.back() << endl;
            }

               
         }
      }
   } else {
      mc2log << warn << "No node expansion table found in the map."
             << endl;
   }
   }        

   mc2dbg4 << "internalLoad_t after node expansion table" << endl;

   loadSignPosts( dataBuffer );
   loadCategoryIds( dataBuffer );
   loadLanes( dataBuffer );

   loadIndexAreaOrders( dataBuffer );

//     if ( getMapID() == 258 ) {
//        m_nodeExpansionTable.clear();
//        {
//           expandedNodes_t expandedNodes;
//           expandedNodes.push_back(0x30005cb4);
//           m_nodeExpansionTable[make_pair(0x280020d2, 0x28001ded)] =
//              expandedNodes;
//        }
//        {
//           // Other node.-
//           expandedNodes_t expandedNodes;
//           expandedNodes.push_back(0xb0005cb4);
//           m_nodeExpansionTable[make_pair(0x280020d2,0x28001ded)] =
//              expandedNodes;
//        }
      
//     }

   
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID)
                        << "] Node expansion table loaded in "
                        << clock << endl;
                 clock = DebugClock());
   

   // Print the native languages of this map.
   mc2dbg << "Native languages of map 0x" << hex << getMapID() 
          << dec << "(" << getMapID() << ") nbr langs:" 
          << getNbrNativeLanguages()  << endl;
   for ( uint32 l=0; l<getNbrNativeLanguages(); l++){
      mc2dbg << "Lang " << l << ": " 
             << LangTypes::getLanguageAsString( getNativeLanguage(l) ) 
             << endl;
   }



   return true;
}

bool
GenericMap::internalSave(int outfile) try
{
   if ( ! GenericMapHeader::internalSave(outfile) ) {
      return false;
   }

   // Check connections - move this code later.
   if ( !MapBits::isCountryMap(this->getMapID()) ){
      mc2dbg << "[GenericMap::internalSave]: Checking connections" << endl;
      for ( int z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
         for ( int i = 0, n = getNbrItemsWithZoom(z); i < n; ++i ) {
            RouteableItem* ri = item_cast<RouteableItem*>(getItem(z,i));
            if ( ri == NULL ) {
               continue;
            }
            for ( int nod = 0; nod != 1; ++nod ) {
               Node* node = ri->getNode(nod);
               for ( int connNbr = 0, m = node->getNbrConnections();
                     connNbr < m; ++connNbr ) {
                  Connection* conn = node->getEntryConnection(connNbr);
                  if ( conn->isMultiConnection() ) {
                     continue;
                  }
                  Connection* oppconn = getOpposingConnection(conn, 
                                                              node->getNodeID()
                                                              );
                  if ( oppconn == NULL ) {
                     mc2dbg << "[GenericMap::internalSave]: "
                            << "Opposing connection missing: "
                            << " nodeID = " << MC2HEX(node->getNodeID())
                            << " connNbr = " << connNbr << endl;
                     MC2_ASSERT( getOpposingConnection( conn, 
                                                        node->getNodeID() ) );
                  }
               }
            }
         }
      }
      mc2dbg << "[GenericMap::internalSave]: DONE checking connections" 
             << endl;
   }
   else {
      mc2dbg << "[GenericMap::internalSave]: Not checking connections of " 
             << "country overview maps." << endl;
   }
   
   DebugClock clock;

   dbgPrintFilePos("Starting GenericMap::internalSave", outfile);

   // Store file position and write marker for size.
   off_t origFilePos = lseek(outfile, 0, SEEK_CUR);
   if ( origFilePos == (off_t)-1 ){
      mc2log << error << "GenericMap::internalSave falied to get file pos" 
             << endl;
      return false;
   }
   uint32 sizeMarker = 0;
   if ( write(outfile, &sizeMarker, 4) == -1 ) {
      throw MC2String("Error while writing size marker ");
   }

   
   auto_ptr<DataBuffer> dataBuffer( new DataBuffer(4 + 8*m_nbrAllocators + 256) );
   dataBuffer->fillWithZeros();

   // Data about the allocators
   // Count all itemTypes
   uint32 nbrConnections = 0;
   uint32 nbrNodes = 0;
   uint32 nbrGfxDatas = 0;
   uint32 nbrGfxDataSingleSmallPoly = 0;
   uint32 nbrGfxDataSingleLine = 0;
   uint32 nbrGfxDataSinglePoint = 0;
   uint32 nbrGfxDataMultiplePoints = 0;
   uint32 nbrSimpleItems = 0;
   uint32 nbrItemsWithType[ GenericMapHeader::numberInitialItemTypeAllocators ];
   uint32 nbrCoordinates = 0;
   uint32 nbrLanes = 0;
   uint32 nbrSignPosts = 0;

   for ( NodeLaneMap::const_iterator it = m_nodeLane.begin() ; 
         it != m_nodeLane.end() ; ++it ) {
      nbrLanes += (*it).value.size();
   }

   for ( ConnectionSignPostMap::const_iterator it =
            m_connectionSignPost.begin() ; 
         it != m_connectionSignPost.end() ; ++it ) {
      nbrSignPosts += (*it).value.size();
   }

   for ( uint32 k=0;
         k < GenericMapHeader::numberInitialItemTypeAllocators; ++k) {
      nbrItemsWithType[k] = 0;
   }

   if (m_gfxData != NULL) {
      switch (m_gfxData->getGfxDataType()) {
         case GfxData::gfxDataFull :
            ++nbrGfxDatas;
            break;
         case GfxData::gfxDataSingleSmallPoly :
            nbrCoordinates += m_gfxData->getTotalNbrCoordinates();
            ++nbrGfxDataSingleSmallPoly;
            break;
         case GfxData::gfxDataSingleLine :
            ++nbrGfxDataSingleLine;
            break;
         case GfxData::gfxDataSinglePoint :
            ++nbrGfxDataSinglePoint;
            break;
         case GfxData::gfxDataMultiplePoints :
            ++nbrGfxDataMultiplePoints;
            break;
      }
   }

   for (int z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         Item* item = getItem(z, i);
         if (item != NULL) {
            ItemTypes::itemType type = item->getItemType();
            if ( uint32(type) < 
                     GenericMapHeader::numberInitialItemTypeAllocators ) {
               ++(nbrItemsWithType[int(type)]);
            } else {
               if ( type == ItemTypes::borderItem ) {
                  nbrSimpleItems++;
               } else {
                  mc2log << error << here << "Item type " << int(type)
                         << " not handled" << endl;
               }
            }
            if (item->getGfxData() != NULL) {
               switch (item->getGfxData()->getGfxDataType()) {
                  case GfxData::gfxDataFull :
                     ++nbrGfxDatas;
                     break;
                  case GfxData::gfxDataSingleSmallPoly :
                     nbrCoordinates += item->getGfxData()->getTotalNbrCoordinates();
                     ++nbrGfxDataSingleSmallPoly;
                     break;
                  case GfxData::gfxDataSingleLine :
                     ++nbrGfxDataSingleLine;
                     break;
                  case GfxData::gfxDataSinglePoint :
                     ++nbrGfxDataSinglePoint;
                     break;
                  case GfxData::gfxDataMultiplePoints :
                     ++nbrGfxDataMultiplePoints;
                     break;
               }
            }
            RouteableItem* ri = Item::routeableItem( item );
            if (ri != NULL) {
               nbrNodes += 2;
               for (int j=0; j<2; j++) {
                     nbrConnections += ri->getNode(j)->getNbrConnections();
               }
            }         
         }
      }
   }

   // Add the external connections
   if (m_segmentsOnTheBoundry != NULL)
      nbrConnections += m_segmentsOnTheBoundry->getTotNbrConnections();

   // Save the number of item types for each type (for allocators)
   
   for ( int t=0;
         t < int(GenericMapHeader::numberInitialItemTypeAllocators); ++t) {
      dataBuffer->writeNextLong( t );
      dataBuffer->writeNextLong(nbrItemsWithType[ t ]);
      mc2dbg << "Save: " << nbrItemsWithType[ t ] 
             << " items with type " << t << endl;
   }

   dataBuffer->writeNextLong(nbrGfxDatas);               // extra 1
   dataBuffer->writeNextLong(nbrNodes);                  // extra 2
   dataBuffer->writeNextLong(nbrConnections);            // extra 3
   dataBuffer->writeNextLong( nbrCoordinates );
   dataBuffer->writeNextLong(nbrGfxDataSingleSmallPoly); // extra 5
   dataBuffer->writeNextLong(nbrGfxDataSingleLine);      // extra 6

   mc2dbg << "#gfxDataFull=" << nbrGfxDatas << ", #gfxDataSingleSmallPoly=" 
          << nbrGfxDataSingleSmallPoly << ", #gfxDataSingleLine="
          << nbrGfxDataSingleLine <<", #Coordinates = " << nbrCoordinates << endl;
   
   // New. Check the number of allocators (from GenericMapHeader)
   // and save these allocators if allowed.
   int nbrAllocatorsLeft = m_nbrAllocators -
         GenericMapHeader::numberInitialItemTypeAllocators - 4;
   if ( nbrAllocatorsLeft > 1 ) {
      mc2dbg << "#gfxDataSinglePoint=" << nbrGfxDataSinglePoint << endl;
      dataBuffer->writeNextLong(nbrGfxDataSinglePoint);     // extra 7
      nbrAllocatorsLeft--;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#gfxDataMultiPoint=" << nbrGfxDataMultiplePoints << endl;
      dataBuffer->writeNextLong(nbrGfxDataMultiplePoints);  // extra 8
      nbrAllocatorsLeft--;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#simpleItems=" << nbrSimpleItems << endl;
      dataBuffer->writeNextLong( nbrSimpleItems ); // extra 9
      nbrAllocatorsLeft--;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#lanes=" << nbrLanes << endl;
      dataBuffer->writeNextLong( nbrLanes ); // extra 10
      nbrAllocatorsLeft--;
   }

   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#nbrCategories=" << m_categoryAllocator->getTotalNbrElements() << endl;
      dataBuffer->writeNextLong( m_categoryAllocator->getTotalNbrElements() ); // extra 11
      nbrAllocatorsLeft--;
   }

   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#signPosts=" << nbrSignPosts << endl;
      dataBuffer->writeNextLong( nbrSignPosts ); // extra 12
      nbrAllocatorsLeft--;
   }

   
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << nbrAllocatorsLeft << " more allocators to write!!!" << endl;
   }
   

   mc2dbg << "Saved " << nbrGfxDatas << " gfx, " << nbrNodes << " nodes, "
          << nbrConnections << " connections" << endl;

   using namespace DataBufferUtil;

   saveBuffer( *dataBuffer.get(), outfile );

   dbgPrintFilePos("After allocators sizes", outfile);

   // Save the gfx-data for the entire map.
   if (m_gfxData != NULL) {

      mc2dbg << "Saving gfxData for the map " <<endl;

      dataBuffer.reset(new DataBuffer(m_gfxData->getSizeInDataBuffer() + 4 ));
      dataBuffer->writeNextLong( 0 );    // Length of gfxData, filled in below
      m_gfxData->save( *dataBuffer.get() );

      // write length
      dataBuffer->writeLong(dataBuffer->getCurrentOffset() - 4, 0);

      mc2dbg << "   Saving gfxData for the map. Length="
             << dataBuffer->getCurrentOffset()-4 <<endl;
      MC2_ASSERT( dataBuffer->getCurrentOffset() - 4 == 
                  m_gfxData->getSizeInDataBuffer() );

      saveBuffer( *dataBuffer.get(), outfile );

      dbgPrintFilePos("After map gfx data", outfile);

   } else {
      mc2log << fatal <<"GenericMap::save(), gfxData == NULL" << endl;
      return false;
   }


   // ***************************************************************
   //                                            Save all the strings
   // ***************************************************************
   m_itemNames->save(outfile);


   // ***************************************************************
   // Save DataBufferObjects
   // ***************************************************************
   saveObject( m_groups, outfile );
   saveObject( m_names, outfile );
   saveObject( m_itemsInGroup, outfile );
   saveObject( m_vehicleRestrictions, outfile );
   mc2dbg << "[GenericMap] Saving poi offset table with size: " 
          << m_poiOffsetTable.size() << endl;
   saveObject( m_poiOffsetTable, outfile );

   // ***************************************************************
   // Save nbr of items in each zoom level
   // ***************************************************************

   dataBuffer.reset( new DataBuffer( 4 * NUMBER_GFX_ZOOMLEVELS ) );
   
   for_each( m_itemsZoomSize, m_itemsZoomSize + NUMBER_GFX_ZOOMLEVELS,
             bind1st(mem_fun(&DataBuffer::writeNextLong), dataBuffer.get()) );

   saveBuffer( *dataBuffer.get(), outfile );


   // ***************************************************************
   // Save allocators (which also saves items)
   // ***************************************************************
   {
      ItemAllocatorContainer::iterator it = m_itemAllocators.begin();
      ItemAllocatorContainer::iterator it_end = m_itemAllocators.end();
      for (; it != it_end; ++it ){
         // TODO: In case we are saving a 2nd level overview map or greater,
         // increase the max size of the zoomlevel on disk.
         if( MapBits::getMapLevel( getMapID() ) >= 2 ) {
            dataBuffer.reset(new DataBuffer(MAX_BUFFER_SIZE_SUPER_OVERVIEW));
         } else {
            dataBuffer.reset( new DataBuffer( MAX_BUFFER_SIZE ) );
         }

         (*it)->save( *dataBuffer.get(), *this );

         saveBuffer( *dataBuffer.get(), outfile );
      }
   }

   // ***************************************************************
   // Save external connections
   // ***************************************************************

   mc2dbg2 << "saveSegmentsOnBoundry" << endl;
   dataBuffer.reset( new DataBuffer(10000000) ); // FIXME: Hardcoded size
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(0);          // size set later
   if (m_segmentsOnTheBoundry != NULL) {
      m_segmentsOnTheBoundry->save( *dataBuffer.get(), *this );
      //set size
      dataBuffer->writeLong(dataBuffer->getCurrentOffset()-4, 0);
      mc2log << "m_segmentsOnTheBoundry saved, size=" 
             << dataBuffer->getCurrentOffset()-4 << "bytes, nbrElm=" 
             << m_segmentsOnTheBoundry->getSize() << endl;
   } else {
      mc2log << warn << here << " Failed to save boundry segments" << endl;
      mc2dbg << "m_segmentsOnTheBoundry == NULL" << endl;
   }

   saveBuffer( *dataBuffer.get(), outfile );

   dbgPrintFilePos("Segments on the boundary", outfile);

   // ***************************************************************
   // Save hash table
   // ***************************************************************
   {
      MC2_ASSERT( m_hashTable.get() );

      DataBuffer buff( m_hashTable->getSizeInDataBuffer() );
      m_hashTable->save( buff );
      DataBufferUtil::saveBuffer( buff, outfile );
   }
   // ***************************************************************
   //                                         Save the landmark table
   // ***************************************************************

   mc2dbg2 << "saveLandmarkTable" << endl;
   typedef landmarkTable_t::const_iterator LI;
   uint32 landmarkSize = m_landmarkTable.size();

   dataBuffer.reset( new DataBuffer(10000000) ); // FIXME: Hardcoded size
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(landmarkSize);
   for (LI it = m_landmarkTable.begin(); it != m_landmarkTable.end(); it++) {
      uint32 fromNodeID = uint32 (((*it).first >> 32) & 0x00000000ffffffff);
      uint32 toNodeID = uint32 ((*it).first & 0x00000000ffffffff);
      
      dataBuffer->writeNextLong(fromNodeID);
      dataBuffer->writeNextLong(toNodeID);
      dataBuffer->writeNextLong((*it).second.itemID);
      dataBuffer->writeNextByte((byte) (*it).second.importance);
      dataBuffer->writeNextByte((*it).second.side);
      dataBuffer->writeNextByte((*it).second.location);
      dataBuffer->writeNextByte((*it).second.type);
   }

   saveBuffer( *dataBuffer.get(), outfile );

   dbgPrintFilePos("After landmarks table", outfile);
   mc2dbg1 << "Saved " << landmarkSize << " landmark descriptions" << endl; 

   // ****************************************************************
   //                                    Save the node expansion table
   // ****************************************************************

   mc2dbg2 << "saveNodeExpansionTable" << endl;
   uint32 nodeExpSize = m_nodeExpansionTable.size();

   // Calculate size of the databuffer.
   // bufSize = nodeExpSize + nodeExpSize * ( from + to + nbrExpandedNodes )
   uint32 bufSize = 4 + nodeExpSize * ( 4 + 4 + 4 );
   for ( map<multiNodes_t, expandedNodes_t>::const_iterator it =
            m_nodeExpansionTable.begin(); it != m_nodeExpansionTable.end();
         ++it ) {
      // bufSize += Nbr expanded nodes.
      bufSize += it->second.size() * 4;
   }
   
   dataBuffer.reset( new DataBuffer( bufSize ) ); 
   dataBuffer->fillWithZeros();

   DataBufferChecker dbc( *dataBuffer, "GenericMap::save expanded nodes." );
   
   dataBuffer->writeNextLong( nodeExpSize );
   
   for ( map<multiNodes_t, expandedNodes_t>::const_iterator it =
            m_nodeExpansionTable.begin(); it != m_nodeExpansionTable.end();
         ++it ) {
      mc2dbg8 << "from node : " << it->first.first
              << "to node : " << it->first.second 
              << endl;
      // From node
      dataBuffer->writeNextLong( it->first.first );
      // To node
      dataBuffer->writeNextLong( it->first.second );
      
      // Expanded nodes
      const expandedNodes_t& expandedNodes = it->second;
      // Nbr expanded nodes.
      uint32 nbrExpandedNodes = it->second.size();
      dataBuffer->writeNextLong( nbrExpandedNodes );
      for ( expandedNodes_t::const_iterator it = expandedNodes.begin();
            it != expandedNodes.end(); ++it ) {
         // Expanded node
         dataBuffer->writeNextLong( *it );
         mc2dbg8 << "  exp node : " << *it << endl;
      }
   }
   

   // Check that the size was calculated correctly:
   dbc.assertPosition( bufSize );

   saveBuffer( *dataBuffer.get(), outfile );

   dbgPrintFilePos("After node expansion table", outfile);
   mc2dbg1 << "Saved " << nodeExpSize << " expanded nodes" << endl; 

   saveSignPosts( outfile );
   saveCategoryIds( outfile );
   saveLanes( outfile );

   // Index area order level table
   saveIndexAreaOrders( outfile );


   saveObject( *m_userRightsTable, outfile );

   dbgPrintFilePos("After user rights table", outfile);


   saveObject( m_streetSideTable, outfile );

   mc2dbg1 << "Saved street side table" <<endl;
   dbgPrintFilePos( "After street side table", outfile );

   // Save m_adminAreaCentres table
   // size(m_nbrAdminAreaCentres) + 
   //       m_nbrAdminAreaCentres * ( itemId + lat + lon )
   bufSize = 4 + m_nbrAdminAreaCentres * ( 4 + 4 + 4 );
   dataBuffer.reset( new DataBuffer( bufSize ) );
   DataBufferChecker dbc2( *dataBuffer.get(),
                           "GenericMap::save admin area centres table." );
   dataBuffer->writeNextLong( m_nbrAdminAreaCentres );
   for ( uint32 i = 0; i < m_nbrAdminAreaCentres; i++ ) {
      adminAreaCentre_t elem = m_adminAreaCentres[i];
      dataBuffer->writeNextLong( elem.itemID );
      dataBuffer->writeNextLong( elem.centre.lat );
      dataBuffer->writeNextLong( elem.centre.lon );
   }
   dbc2.assertPosition( bufSize );

   saveBuffer( *dataBuffer.get(), outfile );

   mc2dbg1 << "Saved " << m_nbrAdminAreaCentres << " admin area centres" 
           << endl; 
   dbgPrintFilePos("After admin area centre table", outfile);

   saveRoadDisplayClasses( outfile );

   saveAreaDisplayClasses( outfile );

   // Write the size written
   off_t curFilePos = lseek(outfile, 0, SEEK_CUR);
   if ( curFilePos == (off_t)-1 ){
      mc2log << error << "GenericMap::internalSave falied to get file pos" 
             << endl;
      return false;
   }
   off_t result = lseek(outfile, origFilePos, SEEK_SET);
   if ( result == (off_t)-1 ){
      mc2log << error << "GenericMap::internalSave falied to set file pos" 
             << " 1" << endl;
      return false;
   }
   off_t sizeWritten = curFilePos - origFilePos;
   DataBuffer dbuf(4);                      // Reading with data buffer, 
   dbuf.writeNextLong(sizeWritten);         // therefore writing with it.
   saveBuffer( dbuf, outfile );

   result = lseek(outfile, curFilePos, SEEK_SET);
   if ( result == (off_t)-1 ){
      mc2log << error << "GenericMap::internalSave falied to set file pos" 
             << " 2" << endl;
      return false;
   }
   mc2dbg << "GenericMap body size: " << sizeWritten << " written." 
          << endl;



   mc2log << info << "GenericMap::save(), Map: 0x" << hex 
          << this->getMapID() << dec << "(" << this->getMapID()
          << ") saved in "
          << clock << "." << endl;
   return (true);

} catch (MC2String err) {
   mc2dbg << error << "Exception in internalSave: " << err << endl;
   mc2dbg << error << strerror(errno) << endl;
   return false;
} catch (...) {
   mc2dbg << error << "Exception in internalSave!" << endl;
   mc2dbg << error << strerror(errno) << endl;
   return false;
}

bool
GenericMap::getItemCoordinates(uint32 itemID, uint16 offset,
                               int32 &lat, int32 &lon) const
{
   const Item* i = itemLookup(itemID);
   if (i == NULL) {
      mc2dbg2 << "getItemCoordinates returns false 1" << endl;
      return (false);
   }

   const GfxData* gfx = i->getGfxData();
   // Incase of a point of interest item,
   // get the streetsegmentitem gfxData and offset.
   if ( ( i->getItemType() == ItemTypes::pointOfInterestItem ) &&
        ( gfx == NULL ) ) {
      const PointOfInterestItem* poi =
         static_cast<const PointOfInterestItem*> (i);
      Item* street = itemLookup(poi->getStreetSegmentItemID());
      if ( street ) {
         gfx = street->getGfxData();
      }
      offset = poi->getOffsetOnStreet();
   } else if (i->getItemType() == ItemTypes::streetItem) {
       
      // Special treatment for street items since they don't have any
      // gfxdata, but consists of a number of street segments with gfxdata.
      // Note that offset is not defined for a street so therefore the 
      // middle of the street is returned.
      const StreetItem* streetItem = static_cast<const StreetItem*> ( i );
     
      // The highest road class of the street.
      byte roadClass = MAX_BYTE;
      
      // Get the boundingbox of all constituent street segments.
      MC2BoundingBox bbox;
      for ( uint32 j = 0; j < streetItem->getNbrItemsInGroup(); ++j ) {
         StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>
            ( itemLookup( streetItem->getItemNumber( j ) ) );
         if ( ssi == NULL ) {
            // err, something is wrong with this item!
            mc2dbg << error << "[GenericMap] street item: "
                   << hex << streetItem->getID() << dec
                   << " has some bad item in group." << endl;
            continue;
         }

         if ( ssi->getRoadClass() < roadClass ) {
            roadClass = ssi->getRoadClass();
         }
               
         gfx = ssi->getGfxData();
         if ( gfx != NULL ) {
            MC2BoundingBox bb;
            gfx->getMC2BoundingBox(bb);
            bbox.update( bb );
         }         
      }
      
      // Approximate center of the street to be the center of the 
      // boundingbox.
      int32 clat, clon;
      bbox.getCenter( clat, clon );

      // Loop through all gfx datas again and find the ssi gfxdata that
      // is closest to the center coordinate.
      int64 minDist = MAX_INT64;
      const GfxData* closestGfx = NULL;
      for ( uint32 j = 0; j < streetItem->getNbrItemsInGroup(); ++j ) {
         StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>
            ( itemLookup( streetItem->getItemNumber( j ) ) );
         if ( ssi && ssi->getRoadClass() == roadClass ) { 
            gfx = ssi->getGfxData();
            if ( gfx != NULL ) {
               // First check if the boundingbox is close enough to the
               // center coordinate.
               MC2BoundingBox bb;
               gfx->getMC2BoundingBox(bb);
               if ( bb.squareDistTo( clat, clon ) < minDist ) {
                  // Ok, the min distance to the boundingbox is closer than
                  // minDist. Now make a real (expensive) check:

                  int64 dist = gfx->squareDistToLine( clat, clon );
                  if ( dist < minDist ) {
                     minDist = dist;
                     closestGfx = gfx;
                  }
               }
            }
         }
      }

      if ( closestGfx != NULL ) {
         // Calculate the coordinate on the streetsegment.
         closestGfx->getOffset( clat, clon, lat, lon );
         return true;
      }

   } else {
      gfx = i->getGfxData();
   }
   
   if (gfx == NULL) {
      mc2dbg2 << "getItemCoordinates returns false 2" << endl;
      return (false);
   }

   return (gfx->getCoordinate(offset, lat, lon));
}

static inline
MC2Coordinate
getCentreFromAdminTable( const GenericMap::adminAreaCentre_t* table,
                        int size,
                        uint32 itemID )
{
   if ( table == NULL ) {
      return MC2Coordinate();
   }
   const GenericMap::adminAreaCentre_t* end = table + size;
   const GenericMap::adminAreaCentre_t* findit =
      lower_bound( table, end, itemID, AdminAreaCentreCmp() );
   if ( findit != end && findit->itemID == itemID ) {
      return findit->centre;
   } else {
      return MC2Coordinate();
   }
}

MC2Coordinate
GenericMap::getCentreFromAdminTable(uint32 itemID) const
{
   return ::getCentreFromAdminTable( m_adminAreaCentres.get(),
                                     m_nbrAdminAreaCentres,
                                     itemID );
}

void
GenericMap::getOneGoodCoordinate(MC2Coordinate& coord, 
                                 const Item* item) const
{
   const GfxData* gfxData = item->getGfxData();

   const bool closed = ( (gfxData != NULL) && (gfxData->closed()) );
   const uint32 itemID = item->getID();

   MC2Coordinate tableCoord = 
      ::getCentreFromAdminTable( m_adminAreaCentres.get(),
                                 m_nbrAdminAreaCentres,
                                 itemID );
   if ( tableCoord.isValid() ) {
      mc2dbg4 << "[GMap]: Found coord " << tableCoord << " for item "
             << IDPair_t( getMapID(), itemID ) << " in m_adminAreaCentres"
             << endl;
      coord = tableCoord;
      return;
   }
   
   if ( ! closed ) {
      // Not closed - take the middle (as in mapprocessor)
      // It could also be that the item does not have any
      // gfxData. In that case the getItemCoordinates function
      // will handle it. (StreetItem).
      getItemCoordinates(itemID, MAX_UINT16 >> 1, coord.lat, coord.lon);
   } else {
      // Closed - try the centroid
      gfxData->getPolygonCentroid(0, coord.lat, coord.lon);
      // Some islands and misc can be outside the GfxData of the map
      // since it is only expanded using streetSegmentItems. This also
      // means that the coordinate cannot be looked up 
      switch ( item->getItemType() ) {
         case ItemTypes::builtUpAreaItem:
         case ItemTypes::municipalItem :
         case ItemTypes::parkItem:
         case ItemTypes::forestItem:
         case ItemTypes::islandItem:
         case ItemTypes::aircraftRoadItem:
         case ItemTypes::individualBuildingItem: {
            // Check if the item is inside the map (convex hull)
            float64 tmpDist = getGfxData()->signedSquareDistTo(coord.lat,
                                                               coord.lon);
            // This distance happens to be the same as in MapReader.
            if ( tmpDist < SQUARE(1000.0) ) {
               // The coordinate is ok
               return;
            }
            mc2dbg << "[GM]: This item is not even inside the map: "
                   << IDPair_t(getMapID(), item->getID())
                   << endl;
            set<ItemTypes::itemType> allowedTypes;
            allowedTypes.insert( ItemTypes::streetSegmentItem);
            uint64 dist;
            uint32 closestSSI =
               ((GenericMap*)(this))->getClosestItemID(coord,
                                                       dist,
                                                       allowedTypes);
            if ( closestSSI == MAX_UINT32 ) {
               // no ssi in this map, get one coordinate on this item
               coord.lat = gfxData->getLat(0,0);
               coord.lon = gfxData->getLon(0,0);
               return;
            }
            // Here we should use the closest coordinate.
            getOneGoodCoordinate(coord, itemLookup(closestSSI) );
            return; // RETURNS HERE
         }
         break;
         default:
            /* Item is always ok */
            break;
      }
   }
   
  
}

MC2Coordinate
GenericMap::getOneGoodCoordinate(const Item* item) const
{
   MC2Coordinate retVal;
   getOneGoodCoordinate(retVal, item);
   return retVal;
}

void
GenericMap::getItemBoundingBox(MC2BoundingBox& bbox,
                               const Item* item) const
{
   if ( item->getGfxData() != NULL ) {
      item->getGfxData()->getMC2BoundingBox(bbox);
   } else {
      // Make boundingbox from coordinate and radius 0
      // -> one coordinate in the bbox.
      bbox = MC2BoundingBox(getOneGoodCoordinate(item), 0);
   }
}

MC2BoundingBox
GenericMap::getItemBoundingBox(const Item* item) const
{
   MC2BoundingBox retVal;
   getItemBoundingBox(retVal, item);
   return retVal;
}

int32
GenericMap::getItemOffset(uint32 itemID, int32 lat, int32 lon) const
{
   // If the item with itemID not is found, this is handled in the
   // other version of the getItemOffset-mathod...
   return (getItemOffset(itemLookup(itemID), lat, lon));
}
   
int32
GenericMap::getItemOffset(Item* item, int32 lat, int32 lon) const
{
   if (item == NULL) {
      return (-1);
   }

   GfxData* gfx = item->getGfxData();
   if (gfx == NULL) {
      return (-1);
   }
 
   int32 latOnPoly, lonOnPoly;
   uint16 offset = gfx->getOffset(lat, lon, 
                   latOnPoly, lonOnPoly);
   mc2dbg4 << "GenericMap:: offset=" << offset 
           << ", (latOnPoly, lonOnPoly)=" << latOnPoly << "," 
           << lonOnPoly << ")" << endl;

   return (offset);
                              
}

bool
GenericMap::modifyItemNumberName(Item& item){
   if(item.getItemType() != ItemTypes::streetSegmentItem)
      return false;

   int nbrOfNames = item.getNbrNames();
   for ( int i = 0; i < nbrOfNames ; i++ ){
      if ( item.getNameType(i) == ItemTypes::roadNumber ) {
         if ( GET_STRING_LANGUAGE( item.getRawStringIndex( i ) ) !=
              LangTypes::invalidLanguage ) {
            // allready set, abort.
            return false;
         }
         char* name = m_itemNames->getString( item.getStringIndex( i ) );
         if ( name != NULL ) {
            LangTypes::language_t
               newLang = NameUtility::getNumberNameValue( name );
            item.setNameLanguage( i, newLang );
         }
      }
   }
   return true;
}


void 
GenericMap::getMapBoundingBox(MC2BoundingBox& bbox) const
{
   MC2_ASSERT(m_gfxData != NULL);
   
   // Initialize the bbox to the map-boundry
   m_gfxData->getMC2BoundingBox(bbox);

   // Loop over all the water-items and update bbox
   for ( int z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      uint32 nbrItems = getNbrItemsWithZoom(z);
      for (uint32 i=0; i<nbrItems; i++) {
         // Check water items
         Item* item = getItem(z, i);
         //MC2_ASSERT(item != NULL);
         if ( (item != NULL) &&
              (item->getItemType() == ItemTypes::waterItem)) {
            // Not valid for a waterItem to not have graphical repr.
            MC2_ASSERT(item->getGfxData() != NULL);

            MC2BoundingBox bb;
            item->getGfxData()->getMC2BoundingBox(bb);
            bbox.update( bb );

         }
      }
   }

   // Recalculate the cos-lat factor
   bbox.updateCosLat();
   
}


void
GenericMap::
getItemsWithinRadiusMeter( set< Item* >& resultItems,
                           const MC2Coordinate& center,
                           int radiusMeters,
                           const set< ItemTypes::itemType >& allowedTypes,
                           const UserRightsMapInfo* rights ) {
   //
   // This "algorithm" has been optimized for municipal and built up areas.
   // This is because the normal location hash table contains all items,
   // and traversing through it takes much longer than necessary, so
   // we take out zoom level 0 where these area features resides and
   // do a linear search.
   // The old search for municipals and buas took about 100-200 times longer
   // in London than this search does.
   //
   // The municipals and buas comes from covered ids request in the
   // SearchRequest when searching with position.
   // A good search example would be:
   // Category search: Hotel
   // position: 614468478, -1530255
   // radius: about 10km
   //
   if ( allowedTypes.size() == 2 &&
        STLUtility::has( allowedTypes, ItemTypes::municipalItem ) &&
        STLUtility::has( allowedTypes, ItemTypes::builtUpAreaItem ) ) {

      // only add items that are municipals and buas within
      // the radius.

      uint64 squareRadius = SQUARE( static_cast< uint64 >( radiusMeters ) );
      for ( uint32 itemIndex = 0; itemIndex < getNbrItemsWithZoom( 0 );
            ++itemIndex ) {
         Item* item = getItem( 0, itemIndex );
         if ( item != NULL &&
              ( item->getItemType() == ItemTypes::municipalItem ||
                item->getItemType() == ItemTypes::builtUpAreaItem ) ) {
            const GfxData* gfx = item->getGfxData();
            if ( gfx != NULL ) {
               // Check if the items bounding box is inside the radius
               MC2BoundingBox queryBox;
               gfx->getMC2BoundingBox( queryBox );
               uint64 maxDist = queryBox.maxSquareDistTo( center.lat, center.lon );
               if ( maxDist <= squareRadius ) {
                  // The whole bounding box is inside the search area, add it
                  resultItems.insert( item );
               } else {
                  // If the mininum distance to the bounding box is less then
                  // the radius, take a closer look and add the item if the
                  // distance to the item is less then the radius.
                  uint64 minDist = queryBox.squareDistTo( center.lat,
                                                          center.lon );
                  if ( minDist <= squareRadius ) {

                     int64 distance = gfx->signedSquareDistTo( center.lat,
                                                               center.lon );
                     // If the distance is negative that means that the
                     // coordinates are inside the "item" ( polygon ) and in
                     // that case we the distance to 0 since we need to compare
                     // it with squareRadius, which is a uint64
                     distance = std::max( int64( 0 ), distance );

                     if ( static_cast< uint64 >( distance ) <= squareRadius ) {
                        // the item is inside as well, add it
                        resultItems.insert( item );
                     }
                  }
               }
            }
         }
      }
   } else {
      m_hashTable->setAllowedItemTypes( allowedTypes, rights );

      set<uint32> idsInside;
      int32 radiusMC2 = int32(radiusMeters*GfxConstants::METER_TO_MC2SCALE);
      m_hashTable->getAllWithinRadius(idsInside, center.lon, center.lat,
                                   radiusMC2);

      // We need to lookup the id:s
      for( set<uint32>::const_iterator it = idsInside.begin();
           it != idsInside.end();
           ++it ) {
         resultItems.insert( itemLookup(*it) );
      }
  }
}

void
GenericMap::getItemsWithinBBox(set<Item*>& resultItems,
                               const MC2BoundingBox& bbox,
                               const set<ItemTypes::itemType>& allowedTypes,
                               const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes( allowedTypes, rights );

   set<uint32> idsInside;
   DebugClock clock;

   m_hashTable->getAllWithinMC2BoundingBox( idsInside, bbox );
   mc2dbg2 << "TIME: getAllWithinMC2BoundingBox  "
                << clock  << "." <<  endl;  
   // We need to lookup the id:s
   for( set<uint32>::const_iterator it = idsInside.begin();
        it != idsInside.end();
        ++it ) {
      resultItems.insert( itemLookup(*it) );
   }
}

void
GenericMap::getIDsWithinRadiusMeter(set<uint32>& resultIDs,
                                    const MC2Coordinate& center,
                                    int radiusMeters,
                                    const set<ItemTypes::itemType>&
                                    allowedTypes,
                                    const UserRightsMapInfo* rights) const
{
   m_hashTable->setAllowedItemTypes( allowedTypes, rights );
   
   int32 radiusMC2 = int32(radiusMeters*GfxConstants::METER_TO_MC2SCALE);
   m_hashTable->getAllWithinRadius(resultIDs, center.lon, center.lat,
                                   radiusMC2);
}

void
GenericMap::getIDsWithinBBox(set<uint32>& resultIDs,
                             const MC2BoundingBox& bbox,
                             const set<ItemTypes::itemType>& allowedTypes,
                             const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes( allowedTypes, rights );

   set<uint32> tempRes;
   m_hashTable->getAllWithinMC2BoundingBox( tempRes,
                                            bbox );
   if ( resultIDs.empty() ) {
      resultIDs.swap( tempRes );
   } else {
      resultIDs.insert( tempRes.begin(), tempRes.end() );
   }
}

void
GenericMap::getItemsWithinGfxData(set<Item*>& resultItems,
                                  const GfxData* gfxData,
                                  const set<ItemTypes::itemType>& allowedTypes,
                                  const UserRightsMapInfo* rights)
{
   if ( gfxData == NULL ) {
      return;
   }
   MC2BoundingBox bbox;
   set<Item*> tempRes;

   // Get all the items within all the bounding boxes.
   int nbrPolygons = gfxData->getNbrPolygons();
   for( int polygon = 0; polygon < nbrPolygons; ++polygon ) {
      gfxData->getMC2BoundingBox(bbox, polygon);
      getItemsWithinBBox(tempRes, bbox, allowedTypes, rights);
   }

   // Remove the items that were outside.
   for( set<Item*>::iterator it = tempRes.begin();
        it != tempRes.end() ; ) {
      Item* item = *it;
      GfxData* itemGfx = item->getGfxData();
      if ( itemGfx != NULL ) {
         // Check if gfx is inside other gfx
         if ( gfxData->minSquareDistTo(itemGfx) > 0 ) {
            resultItems.erase(it++);
            continue;
         }
      } else {
         // Probably poi - get a coordinate
         int32 lat;
         int32 lon;
         if ( getItemCoordinates(item->getID(), MAX_UINT16/2,
                                 lat, lon) ) {
            // Check if inside
            if ( gfxData->insidePolygon(lat, lon) == 0 ) {
               resultItems.erase(it++);
               continue;
            }
         } else {
            // Could not get coordinate. Strange, really.
            resultItems.erase(it++);
            continue;
         }
      }
      ++it;
   }
                                   
   // Copy items to real result set.
   resultItems.insert( tempRes.begin(), tempRes.end() );
}

void
GenericMap::getIDsWithinGfxData(set<uint32>& resultIDs,
                                const GfxData* gfxData,
                                const set<ItemTypes::itemType>& allowedTypes,
                                const UserRightsMapInfo* rights)
{
   set<Item*> resultItems;
   getItemsWithinGfxData(resultItems, gfxData, allowedTypes, rights);
   // Blargh - copy the ids into set
   for(set<Item*>::const_iterator it = resultItems.begin();
       it != resultItems.end();
       ++it ) {
      resultIDs.insert((*it)->getID());
   }
}

uint32
GenericMap::getClosestItemID( const MC2Coordinate& coord,
                              uint64& dist,
                              const set<ItemTypes::itemType>& allowedTypes,
                              const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes(allowedTypes, rights);
   return m_hashTable->getClosest(coord.lon, coord.lat, dist);   
}


bool
GenericMap::getAllItemsWithStringIndex(uint32 stringIndex, Vector* result)
{
   // Check the inparameters
   if (result == NULL) {
      mc2log << error
             << "GenericMap::getAllItemsWithStringIndex, result == NULL"
             << endl;
      return (false);
   }     
   // Get_STRING_INDEX is defined in Item.h
   stringIndex = GET_STRING_INDEX(stringIndex);
   result->reset();

   // Loop over all the items in the map and check the items that
   // have the name stringIndex
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z) {
      uint32 nbrIWZ = getNbrItemsWithZoom(z);
      for(uint32 i = 0; i < nbrIWZ; ++i) {
         Item* item = getItem(z, i);
         if (item != NULL) {
            uint32 nbrNames = item->getNbrNames();
            for (uint32 j = 0; j < nbrNames; ++j) {
               if (item->getStringIndex(j) == stringIndex) { 
                  // Found
                  result->addLast(item->getID());
               }
            }
         }
      }
   }

   // Return
   return (true);
}

bool 
GenericMap::isVehicleAllowed(const Connection& con, 
                             ItemTypes::vehicle_t vehicle ) const 
{
   return (getVehicleRestrictions( con ) & (uint32)(vehicle) ) != 0;
}

uint32 
GenericMap::getVehicleRestrictions( const Connection& con ) const
{
   return m_vehicleRestrictions[ con.getVehicleRestrictionIdx() ];
}


GfxData*
GenericMap::createNewGfxData(DataBuffer* dataBuffer) 
{
   return m_gfxDataFactory->create( *dataBuffer, *this );
}


Connection* 
GenericMap::createNewConnection(DataBuffer* dataBuffer)
{
   return (Connection::createNewConnection( *dataBuffer, *this ) );
}

Item*
GenericMap::createNewItem(DataBuffer* dataBuffer)
{
   return Item::createNewItem( *dataBuffer, *this );
}

const Item* 
GenericMap::getItemFromItemIdentifier( const ItemIdentifier& ident ) const
{
   set<const Item*> itemCandidates;

   // Get all items of the correct type and name.
   getItemsWithName( ident.getName(), 
                     itemCandidates, 
                     ident.getItemType() );

   const Item* result = NULL;
   
   // Find out if the item is identified by insideName or coordinates.
   if ( ident.getInsideName() != NULL ) {
      // Identified by being inside an item named ident.getInsideName().
   
      set<const Item*> areaItems;
      getItemsWithName( ident.getInsideName(), areaItems );

      set<const Item*>::const_iterator it = itemCandidates.begin();
      bool multipleHits = false;
      
      while ( ( it != itemCandidates.end() ) && ( ! multipleHits ) ) {
         
         set<const Item*>::const_iterator jt = areaItems.begin();
         while ( ( jt != areaItems.end() ) && ( ! multipleHits ) ) {
            
            // Check if *it logically is inside *jt.
            const GroupItem* groupItem = Item::groupItem( const_cast<Item*>(*jt) );
            if ( ( groupItem != NULL ) && 
                 ( groupItem->containsItem( (*it)->getID() ) ) ) {
               if ( result == NULL ) {
                  result = *it;
               } else if ( result != *it ) {
                  multipleHits = true; // Non unique hit.
                  result = NULL;
               }
            } else {
               GfxData* areaGfx = (*jt)->getGfxData();
               GfxData* itemGfx = (*it)->getGfxData();
               // Check if it is geometrically inside.
               if ( ( areaGfx != NULL ) && 
                    ( areaGfx->closed() ) && 
                    ( itemGfx != NULL  ) &&
                    ( itemGfx->getNbrCoordinates(0) > 0 ) ) {
                  MC2BoundingBox areaGfxBB;
                  MC2BoundingBox itemGfxBB;
                  areaGfx->getMC2BoundingBox(areaGfxBB);
                  itemGfx->getMC2BoundingBox(itemGfxBB);

                  if ( ( areaGfxBB.overlaps(itemGfxBB)) &&
                       ( areaGfx->insidePolygon(  
                                    itemGfx->getLat(0, 0 ),
                                    itemGfx->getLon(0, 0 ) ) > 0 )) {
                     // Inside or on the boundary.
                     if ( result == NULL ) {
                        result = *it;
                     } else if ( result != *it ) {
                        multipleHits = true; // Non unique hit.
                        result = NULL;
                     }
                  }
               }
            }
            ++jt;
         }
         ++it;
      }

      return result;

   } else {
      // Identified by coordinates.
  
      const Item* closestLinearItem = NULL;
      uint32 nbrCloseLinearItems = 0;
      uint64 closestSqDist = MAX_UINT64;

      int32 lat = ident.getLat();
      int32 lon = ident.getLon();
     
      bool multipleHits = false;
      set<const Item*>::const_iterator it = itemCandidates.begin();
      while ( ( it != itemCandidates.end() ) && ( ! multipleHits ) ) {
         GfxData* gfx = (*it)->getGfxData();

         if ( gfx != NULL ) {
            
            if ( gfx->closed() ) {
               if ( gfx->insidePolygon( lat, lon ) == 2 ) {
                  // Coordinate is inside the closed polygon.
                  if ( result == NULL ) {
                     result = *it;
                  } else {
                     multipleHits = true;
                  }
               }
            } else {
               
               // Make sure the coordinate is close enough to the open
               // polygon.
               MC2BoundingBox bbox;
               gfx->getMC2BoundingBox( bbox );
               // Make sure the bbox is at least 20 x 20 meters.
               bbox.increaseMeters( uint32( 20 )); 
                                              
               if ( bbox.contains( lat, lon ) ) {
                  uint64 dist = gfx->squareDistToLine( lat, lon );
                  if ( dist < closestSqDist ) {
                     closestSqDist = dist;
                     closestLinearItem = *it;
                     nbrCloseLinearItems = 1;
                  } else  if ( dist == closestSqDist ) {
                     nbrCloseLinearItems++;
                  }
               }
            }
         }
         ++it;
      }
      
      if ( ( result != NULL ) || multipleHits ) {
         return result;
      } else if ( ( nbrCloseLinearItems == 1 ) &&
                  ( closestSqDist <= 
               SQUARE( ItemIdentifier::maxDistToOpenPolygonInMeters ) ) ) { 
         // Found a unique item (unclosed polygon) that is close enough
         // to the specified coordinate.
         return closestLinearItem;
      }       
   }

   // If we have gotten this far, no unique item could be found.
   return NULL;
}


bool 
GenericMap::getItemIdentifierForItem( const Item* item, 
                                      ItemIdentifier& ident ) const
{
   // Note that this method may not always be able to set the
   // item identifier to uniquely identify the item.
    
   const char* itemName;
   if ( item->getNbrNames() > 0 ) {
      itemName = getName( item->getStringIndex( 0 ) );
   } else {
      // We must have a name for the item.
      return false;
   }
 
   bool retVal = false;
   
   GfxData* gfx = item->getGfxData();
      
   int32 lat;
   int32 lon;
   
   if ( gfx != NULL ) {

      if ( gfx->closed() ){
         // Get a coordinate inside the gfxdata.
         if ( gfx->getRandomCoordinateInside( lat, lon ) ) {
            retVal = true; 
         }
      } else {
         // Get a coordinate halfways into the gfxdata.
         if ( gfx->getCoordinate( MAX_UINT16 / 2, lat, lon ) != 0 ) {
            retVal = true;
         }
      }
      
      if ( retVal ) {
         ident.setParameters( itemName, item->getItemType(), lat, lon );
      }      
   } else {
      
      // No gfx-data available.
      // Use location instead.

      const char* areaName = NULL;

      Item* areaItem = getRegion(item, ItemTypes::builtUpAreaItem);
      if ( areaItem == NULL ) {
         areaItem = getRegion(item, ItemTypes::municipalItem);
      }

      if ( ( areaItem != NULL ) && ( areaItem->getNbrNames() > 0 ) ) {
         areaName = getName( areaItem->getStringIndex( 0 ) );
         retVal = true;
      }

      if ( retVal ) {
         ident.setParameters( itemName, item->getItemType(),
                              areaName ? areaName : "" );
      }
   }

   if ( retVal ) {
      // Make sure the item identifier is ok.
      const Item* result = getItemFromItemIdentifier( ident );
      if ( ( result == NULL ) || ( result->getID() != item->getID() ) ) {
         retVal = false;
      }
   }

   return retVal;
}

uint32
GenericMap::updateOfficialNames()
{
   uint32 nbrChanged = 0;
   set<LangTypes::language_t> langs;
   for ( uint32 i = 0; i < getNbrNativeLanguages(); ++i ) {
      langs.insert(getNativeLanguage(i));
   }
   
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++ ) {
         Item* item = getItem( z, i );
         if ( item != NULL ) {
            for ( uint32 name = 0; name < item->getNbrNames(); ++name ) {
               if ( item->getNameType( name ) == 
                    ItemTypes::officialName ) {
                  LangTypes::language_t lang = 
                     item->getNameLanguage( name );
                  if ( langs.find( lang ) == langs.end() ) {
                     // Not really native.
                     const char* langAsString =
                        LangTypes::getLanguageAsString(
                           LangTypes::language_t( lang ),
                           true);
                     mc2dbg << "[GMap]: Official itemname \""
                            << getName(item->getStringIndex(name))
                            << "\" in " << langAsString << " not native "
                        "changing to alternative " << endl;
                     item->setNameType(name, ItemTypes::alternativeName);
                     ++nbrChanged;
                  } // END: if find
               } // END: if getNameType
            } // END: getNbrNames
         } // END: if
      } // END: getNbrItemsWithZoom
   } // END: for
   return nbrChanged;
}

uint32 
GenericMap::updateNativeLanguages()
{
   // Copied from OldGenericMap
   // Keep the rules for deciding what is a native language in sync with
   // the rules in OldGenericMap.
   
   // Key is native langauge, value is nbr occasions of the langauge
   typedef map<LangTypes::language_t, uint32> langMap_t;
   typedef langMap_t::iterator langMapIt_t;
   langMap_t nativeLanguages;

   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++ ) {
         Item* item = getItem( z, i );
         if ( item != NULL ) {
            for ( uint32 name = 0; name < item->getNbrNames(); ++name ) {
               if ( item->getNameType( name ) == 
                    ItemTypes::officialName ) {
                  LangTypes::language_t lang = 
                     item->getNameLanguage( name );
                  if ( lang != LangTypes::invalidLanguage ) {
                     langMapIt_t langMapIt = nativeLanguages.find( lang );
                     if ( langMapIt == nativeLanguages.end() ) {
                        // Not found before. Insert and set to 1.
                        nativeLanguages[ lang ] = 1;
                     } else {
                        // Already present. Increment nbr of occasions.
                        langMapIt->second++;
                     }
                  } // END: if
               } // END: getNameType
            } // END: getNbrNames
         } // END: if
      } // END: getNbrItemsWithZoom
   } // END: for
   
   // Sort the native languages by order of occasions.
   map<uint32, LangTypes::language_t> langByNbrOccasions;
   uint32 totalNbrNames = 0;
   for ( langMapIt_t langMapIt = nativeLanguages.begin(); 
         langMapIt != nativeLanguages.end(); ++langMapIt ) {
      langByNbrOccasions[ langMapIt->second ] = langMapIt->first;
      totalNbrNames += langMapIt->second;
   }

   uint32 nbrNativeLangs = 0;
   clearNativeLanguages();
   // Add to m_nativeLanguages
   // Go through langByNbrOccasions backwards so that the most common
   // language will be first.
   for ( map<uint32, LangTypes::language_t>::reverse_iterator it = 
            langByNbrOccasions.rbegin(); it != langByNbrOccasions.rend();
         ++it ) {
      // Only add the language as native language if at least 5%, or more 
      // than 500, of the names are in that language.
      if ( (it->first > uint32(totalNbrNames * 0.05)) ||
           (it->first > 200) ) {
         addNativeLanguage( it->second );
         ++nbrNativeLangs;
         mc2log << info << "[GMap]: Adding native language "
                 << LangTypes::getLanguageAsString( it->second, true )
                 << " (nbr occasions = " << it->first << ")." << endl;
      }
      else {
         mc2log << info << "[GMap]: Not adding native language "
                 << LangTypes::getLanguageAsString( it->second, true )
                 << " (nbr occasions = " << it->first << ")." << endl;
      }
   }
   return nbrNativeLangs;
}


uint16 
GenericMap::getStreetLeftSideNbrStart( const StreetSegmentItem& item ) const
{
   return m_streetSideTable.getData( item.getID() ).getLeftSideNbrStart();
}

uint16 
GenericMap::getStreetLeftSideNbrEnd( const StreetSegmentItem& item ) const
{
   return m_streetSideTable.getData( item.getID() ).getLeftSideNbrEnd();
}

uint16 
GenericMap::getStreetRightSideNbrStart( const StreetSegmentItem& item ) const 
{
   return m_streetSideTable.getData( item.getID() ).getRightSideNbrStart();
}

uint16 
GenericMap::getStreetRightSideNbrEnd( const StreetSegmentItem& item ) const
{
   return m_streetSideTable.getData( item.getID() ).getRightSideNbrEnd();
}

void
GenericMap::getCategories( const Item& item, Categories& categories ) const {

   typedef const CategoryMap::value_type* RangeType;
   pair< RangeType, RangeType >
   range = std::equal_range( &m_categoryIds[ 0 ], 
                             &m_categoryIds[ m_categoryIds.size() ], 
                             CategoryMap::value_type( item.getID() ));

   if ( range.second - range.first == 0 ) {
      // no hits
      return;
   }

   if ( range.second - range.first > 1 ) {
      mc2log << error 
             << "[GenericMap] More than one item hit when searching for"
             << " categories for item. Item id: " << hex << item.getID()
             << " map id: " << getMapID() << endl;
      mc2log << error << "[GenericMap] will use the first item." << endl;
   }

   categories.insert( categories.end(),
                      range.first->value.begin(),
                      range.first->value.end() );
}

uint32
GenericMap::expandNodeIDs( list<uint32>& nodeIDs ) const
{
   list<uint32>::iterator curIt = nodeIDs.begin();
   if ( curIt == nodeIDs.end() ) {
      return 0;
   }

   list<uint32>::iterator nextIt = curIt;
   ++nextIt;

   map<multiNodes_t, expandedNodes_t>::const_iterator expandIt;
   
   while ( nextIt != nodeIDs.end() ) {
      
      expandIt = m_nodeExpansionTable.find( make_pair( *curIt, *nextIt ) );
      if ( expandIt != m_nodeExpansionTable.end() ) {
         
         // Insert the list of expanded nodes before nextIt.
         nodeIDs.insert( nextIt, 
                         expandIt->second.begin(),
                         expandIt->second.end() );
         
      } 
         
      // Continue.
      curIt = nextIt;
      ++nextIt;
   }
   return nodeIDs.size();
}

void
GenericMap::getItemsWithName( const char* name, 
                              set<const Item*>& items,
                              ItemTypes::itemType itemType ) const
                              
{
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++ ) {
         
         Item* item = getItem( z, i );
         if ( ( item != NULL ) && 
              ( ( itemType == ItemTypes::numberOfItemTypes ) || 
                ( item->getItemType() == itemType ) ) ) {
            
            // Check if the names are the same.
            bool sameName = false;
            uint32 j = 0;

            while ( ( j < item->getNbrNames() ) && ( !sameName ) ) {
               if ( strcasecmp( getName( item->getStringIndex( j ) ),
                                name ) == 0 ) {
                  sameName = true;
               } else {
                  j++;
               }
            }

            if ( sameName ) {
               items.insert( item );
            }

         }
         
      }
   }
}


float64
GenericMap::getDistanceToNode(byte nodeNbr, GfxData* gfx, int32 lat, int32 lon)
{
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   uint32 n = 0;
   if (nodeNbr > 0) {
      n = gfx->getNbrCoordinates(0) - 1;
   }
   int32 otherLat = gfx->getLat(0,n);
   int32 otherLon = gfx->getLon(0,n);

   return (  GfxUtility::squareP2Pdistance_linear( otherLat, otherLon,
                                                   lat, lon));
}


uint32 GenericMap::getMemoryUsage(void) const {
   // our size
   uint32 sum = sizeof(GenericMap);

   // Calculate sizes of items
   sum += 
      accumulate( m_itemAllocators.begin(),
                  m_itemAllocators.end(), 0,
                  STLUtility::accumulate_cb(
                  mem_fun(&DataBufferMapObject::getMemoryUsage)));

   if ( m_gfxData != NULL )
      sum += m_gfxData->getMemoryUsage();
   if ( m_hashTable.get() != NULL )
      sum += m_hashTable->getMemoryUsage();
   if ( m_itemNames != NULL )
      sum += m_itemNames->getMemoryUsage();      
   if ( m_name != NULL )
      sum += strlen(m_name) + 1;
   if ( m_origin != NULL )
      sum += strlen(m_origin) + 1;
   if ( m_filename != NULL )
      sum += strlen(m_filename) + 1;
   if ( m_pathname != NULL )
      sum += strlen(m_pathname) + 1;

   if ( getBoundrySegments() != NULL ) {
      sum += getBoundrySegments()->getMemoryUsage();
      for( uint32 i=0; i < getBoundrySegments()->getSize(); i++) {
         sum += getBoundrySegments()->getElementAt(i)->getMemoryUsage();
      }
   }
   // sum all item counts
   sum += accumulate( m_itemsZoomAllocated,  
                      m_itemsZoomAllocated + NUMBER_GFX_ZOOMLEVELS, 0 ) 
          * sizeof(uint32);

   sum += m_nativeLanguages.size() * 4;

   if ( m_currency != NULL )
      sum += m_currency->getMemoryUsage();

   return sum;
}

#if 0
bool 
GenericMap::findEmptySlot(uint32 zoomlevel, uint32 &cnt) 
{
   // Increase cnt until an empty slot is found
   Item* curItem = NULL;
   while ( (cnt < m_itemsZoomSize[zoomlevel]) &&
           ((curItem = m_itemsZoom[zoomlevel][cnt]) != NULL) &&
           (curItem->getItemType() != ItemTypes::nullItem)
   ) {
      cnt++;
   }
   // Return true if cnt points to an empty slot, false otherwise
   return ( (cnt < m_itemsZoomSize[zoomlevel]) &&
            (curItem != NULL) );
}

bool 
GenericMap::swapItems(uint32 aID, uint32 bID) 
{
   // NB! READ THE NOTE IN THE DOCUMENTAION (GenericMap.h)!!!

   // Find the items
   Item* aItem = itemLookup(aID);
   Item* bItem = itemLookup(bID);

   if ((aItem != NULL) && (bItem != NULL)) {
      // Swap the items
      m_itemsZoom[(aID & 0x78000000) >> 27]
                 [aID & 0x07ffffff] = bItem;
      m_itemsZoom[(bID & 0x78000000) >> 27]
                 [bID & 0x07ffffff] = aItem;

      // Update ID of both the items
      aItem->setID(bID);
      bItem->setID(aID);

      // Everything seems OK
      return (true);
   } else {
      // One of both of the items where NULL
      return (false);
   }
}
#endif

uint32
GenericMap::getConnection( uint32 fromItemID, uint32 toItemID,
                           Connection*& conn ) const
{
   RouteableItem* toRI = Item::routeableItem( itemLookup(toItemID) );
   if (toRI == NULL) {
      // Wrong ssi id specified
      conn = NULL;
      return (MAX_UINT32);
   }
   
   // Check connection from both nodes
   for (uint16 i = 0; i < 2; i++) {
      Node* toNode = toRI->getNode(i);
      // Check all connections from each node
      for (uint32 j = 0; j < toNode->getNbrConnections(); j++) {
         conn = toNode->getEntryConnection(j);
         // Check if the connection is coming from fromItemID
         if ((conn->getConnectFromNode() & 0x7fffffff) ==  fromItemID) {
            return ( toNode->getNodeID() );
         }
      }
   }

   // If we have come this far, no connection was found
   conn = NULL;
   return (MAX_UINT32);
}
   
uint32
GenericMap::getTotalNbrItemNames() const
{
   return (m_itemNames->getNbrStrings());
}


bool
GenericMap::getConnectionData(Connection* conn, Node* toNode,
                              float64& time, float64& sst, 
                              float64& length, 
                              bool externalConnection ) const
{
   if ( conn->isMultiConnection() ) {
      list<uint32> nodeIDs;
      nodeIDs.push_back( conn->getFromNode() );
      nodeIDs.push_back( toNode->getNodeID() );
      if ( expandNodeIDs( nodeIDs ) > 2 ) {
         // Need at least 3 nodes for a multi connection.
         list<uint32>::const_iterator it = nodeIDs.begin();
         list<uint32>::const_iterator nextIt = it;
         ++nextIt;
         length = time = sst = 0;
         float64 curLength = 0;
         float64 curTime = 0;
         float64 curSST = 0;
         while ( nextIt != nodeIDs.end() ) {
            Node* curNode = nodeLookup( *nextIt );
            Connection* curConn = curNode->getEntryConnectionFrom( *it );
            // FIXME: This check should not be necessary.
            // Remove once the maps are OK again.
            if ( curConn != NULL ) {
               getConnectionData( curConn, curNode, 
                                  curTime, curSST, curLength,
                                  externalConnection );
               length += curLength;
               time += curTime;
               sst += curSST;
            }
            it = nextIt;
            ++nextIt;
         }
         return true;
      }
   }
   
   getSingleConnectionLength( conn, toNode, length, externalConnection );
   getSingleConnectionTime( conn, toNode, time, sst, 
                            length, externalConnection);

   return true;
}


bool
GenericMap::getSingleConnectionLength(Connection* conn, Node* toNode,
                                      float64& length, 
                                      bool externalConnection) const
{
   // Get to and from segments.
   
   RouteableItem* toSegment = static_cast<RouteableItem*> 
      (itemLookup(toNode->getNodeID() & 0x7FFFFFFF));
   
    RouteableItem* fromSegment;
   if (!externalConnection) { 
      fromSegment = static_cast<RouteableItem*> 
         (itemLookup(conn->getConnectFromNode() & 0x7FFFFFFF));
   } else {
      // External connection. 
      // The length of the external connection is 0 since
      // there is a 0-length segment on each side of the map boundary.
      length = 0;
      return true;
   }

   if ( fromSegment == NULL ||
        toSegment == NULL ) {
      return false;
   }

   if ( fromSegment->getGfxData() == NULL ||
        toSegment->getGfxData() == NULL ) {
      return false;
   }

   // Calculate the actual length of the connection.
   // Note that this depends on whether it is a connection involving
   // buses and/or streetsegments.
   
   MC2_ASSERT(fromSegment->getGfxData()->getNbrPolygons() == 1);
   MC2_ASSERT(toSegment->getGfxData()->getNbrPolygons() == 1);

   
   if ((fromSegment->getItemType() == ItemTypes::streetSegmentItem) &&
       (toSegment->getItemType() == ItemTypes::streetSegmentItem)) {
      
      // ssi -> ssi
      length = fromSegment->getGfxData()->getLength(0);
      
   } 
   else if ((fromSegment->getItemType() == ItemTypes::streetSegmentItem) &&
            (toSegment->getItemType() == ItemTypes::busRouteItem)) {
      
      // ssi -> bus
      BusRouteItem* bri = static_cast<BusRouteItem*> (toSegment);
      length = calcBusSSILength(bri, 
            fromSegment->getGfxData()->getLength(0),
                                ( conn->getConnectFromNode() & 0x80000000 ) == 0);
      
   }
   else if ((fromSegment->getItemType() == ItemTypes::busRouteItem) &&
            (toSegment->getItemType() == ItemTypes::streetSegmentItem)) {
         
         // bus -> ssi
      BusRouteItem* bri = static_cast<BusRouteItem*> (fromSegment);
      length = calcBusSSILength(bri, toSegment->getGfxData()->getLength(0), 
                                toNode->isNode0());
   
   }   
   else if ((fromSegment->getItemType() == ItemTypes::busRouteItem) &&
            (toSegment->getItemType() == ItemTypes::busRouteItem)) {
      
      // bus -> bus
      BusRouteItem* fromBRI = static_cast<BusRouteItem*> (fromSegment);
      BusRouteItem* toBRI = static_cast<BusRouteItem*> (toSegment);
      
      // Check that it is an actual bus change.
      if (fromBRI->getBusRouteID() != toBRI->getBusRouteID()) {
         
         // Find the connecting streetsegment.
         uint16 i = 0;
         StreetSegmentItem* connectingSSI = NULL;

         while (  (i < fromSegment->getNode(0)->getNbrConnections()) &&
                  (connectingSSI == NULL) ) {
            uint32 itemID = 
               fromSegment->getNode(0)->getEntryConnection(i)
                  ->getConnectFromNode() & 0x7FFFFFFF;
            connectingSSI = item_cast<StreetSegmentItem*>( itemLookup(itemID) );
            i++;
         }
         
         if (connectingSSI == NULL) {
            // Something very strange has happened!
            mc2log << error
                   << "Unable to find connecting ssi for two different"
                   <<  " bus routes." << endl;
            return (false);
         } else {
            // Calculate the distance
            float64 offsetFactor = 
               abs(fromBRI->getOffsetInClosestStreet() -
                   toBRI->getOffsetInClosestStreet()) / 
               float64(MAX_UINT16);
            length = 
               connectingSSI->getGfxData()->getLength(0) * offsetFactor;
         }
      
      } else {
         // Not a change of buses. Set length to the length of the 
         // fromSegment.
         length = fromSegment->getGfxData()->getLength(0);
      }
      
   } else {
      
      // ?? -> ??
      // Set length to the length of the fromSegment.
      length = fromSegment->getGfxData()->getLength(0);
   } 
   
   return (true);
}
  
bool
GenericMap::getSingleConnectionTime(Connection* conn, Node* toNode,
                                    float64& time, float64& standStillTime,
                                    float64 length,
                                    bool externalConnection ) const
{
   RouteableItem* toRI = Item::routeableItem(
      (itemLookup(toNode->getNodeID() & 0x7FFFFFFF)));
   
   RouteableItem* fromRI;
   Node* fromNode;
   if (!externalConnection) { 
      fromRI = Item::routeableItem(
         (itemLookup(conn->getConnectFromNode() & 0x7FFFFFFF)));
      fromNode = nodeLookup(conn->getConnectFromNode());
   } else {
      // External connection.
      // The connection time of the external connection is 0 since
      // there is a 0-length segment on each side of the map boundary.
      time = standStillTime = 0;
      return true;
   }

   // Note that the time is really only valid for streetsegment 
   // to streetsegment connections.
   
   if ((fromRI == NULL) || (toRI == NULL)) {
      return (false);
   }

   byte speedLimit = fromNode->getSpeedLimit(); 
   
   if (speedLimit > 0) {
      // The speedlimit is valid, set the stand still time
      // depending on the turndescription.
      switch (conn->getTurnDirection()) {
         case (ItemTypes::LEFT) :
            // If the left turn is made on large roads, 
            // the standstill time will be higher.
            // Always expensive when large road involved!
            if ((fromRI->getRoadClass() <= 2) ||
                (toRI->getRoadClass() <= 2)) {
               standStillTime = 
                  Connection::STANDSTILLTIME_LEFTTURN_MAJOR_ROAD;
               mc2dbg4 << "StandStillTime = LEFT (MAJOR)" << endl;
            } else {
               standStillTime = 
                  Connection::STANDSTILLTIME_LEFTTURN_MINOR_ROAD;
               mc2dbg4 << "StandStillTime = LEFT (MINOR)" << endl;
            }
            break;
          case (ItemTypes::UTURN) :
            // If the u-turn is made on large roads, 
            // the standstill time will be higher.
            if ((fromRI->getRoadClass() <= 2) || 
                (toRI->getRoadClass() <= 2)) {
               standStillTime = 
                  Connection::STANDSTILLTIME_U_TURN_MAJOR_ROAD;
               mc2dbg4 << "StandStillTime = UTURN (MAJOR)" << endl;
            } else {
               standStillTime = 
                  Connection::STANDSTILLTIME_U_TURN_MINOR_ROAD;
               mc2dbg4 << "StandStillTime = UTURN (MINOR)" << endl;
            }
            break;
        case (ItemTypes::RIGHT) :
	    // If the left turn is made on large roads,
	    // the standstill time will be higher.
	    if ((fromRI->getRoadClass() <= 2) &&
	        (toRI->getRoadClass() <= 2)) {
               standStillTime = 
                  Connection::STANDSTILLTIME_RIGHTTURN_MAJOR_ROAD;;
               mc2dbg4 << "StandStillTime = RIGHT (MAJOR)" << endl;
	    } else {
	       standStillTime =
	          Connection::STANDSTILLTIME_RIGHTTURN_MINOR_ROAD;
	       mc2dbg4 << "StandStillTime = RIGHT (MINOR)" << endl;
	    }
            break;
         case (ItemTypes::ENTER_ROUNDABOUT) :
            standStillTime = 
               Connection::STANDSTILLTIME_ENTER_ROUNDABOUT;
            mc2dbg4 << "StandStillTime = ENTER ROUNDABOUT" << endl;
            break;
         case (ItemTypes::ENTER_FERRY) :
            standStillTime = 
               Connection::STANDSTILLTIME_ENTER_FERRY;
            mc2dbg4 << "StandStillTime = ENTER FERRY" << endl;
            break;
         case (ItemTypes::EXIT_FERRY) :
            standStillTime = 
               Connection::STANDSTILLTIME_EXIT_FERRY;
            mc2dbg4 << "StandStillTime = EXIT FERRY" << endl;
            break;
         case (ItemTypes::CHANGE_FERRY) :
            standStillTime = 
               Connection::STANDSTILLTIME_CHANGE_FERRY;
            mc2dbg4 << "StandStillTime = CHANGE FERRY" << endl;
            break;
         default:
            standStillTime = 0;
            break;
      }
   }

   time = length / float64(Math::KMH_TO_MS * speedLimit);
   
   StreetSegmentItem* fromSSI = item_cast<StreetSegmentItem*>( fromRI );
   if (fromSSI != NULL) {
      time = time * conn->getPenaltyFactor(fromSSI->getRoadClass(),
                                           fromSSI->isRamp(),
                                           speedLimit);
   }
   
   return (true);

}
   

bool
GenericMap::getConnectionCost(Connection* conn, Node* toNode,
                              uint32& costA, uint32& costB,
                              uint32& costC, uint32& costD,
                              bool externalConnection) const
{
   float64 t, sst, length;
   if (! getConnectionData( conn, toNode, t, sst, length, 
                            externalConnection ) ) {
      return false;
   }

   costB = costC = uint32( rint( Connection::secToTimeCost( t + sst ) ) );
   
   costA = uint32( rint( Connection::metersToDistCost( length ) ) );
   costD = 0;
   
   return (true);
}

uint32 GenericMap::getConnectionID( const Connection& con ) const 
{
   // we assume the Connection exist in the allocator
   return m_connectionAllocator->getPosition( &con );
}


float64
GenericMap::calcBusSSILength(BusRouteItem* bri, float64 dist,
                             bool isSSINode0) const
{
   
   float64 offsetFactor = 
      bri->getOffsetInClosestStreet() / float64(MAX_UINT16);
   
   if (!isSSINode0) {
      // Correct for that the BusRoute offset is calculated from
      // ssi node 0.
      offsetFactor = 1 - offsetFactor;
   }

   return (dist*offsetFactor);
   
}


bool
GenericMap::itemHasNameAndType(Item* item,
                               LangTypes::language_t strLang,
                               ItemTypes::name_t strType,
                               const char* nameStr,
                               uint32& strIdx) const
{
   uint32 i = 0;
   strIdx = MAX_UINT32;
   bool retVal = false;
   bool found = false;

   while ((i<item->getNbrNames()) && (!found)) {
      ItemTypes::name_t curType;
      LangTypes::language_t curLang;
      uint32 curIndex;

      if (item->getNameAndType(i, curLang, curType, curIndex)) {
         // has at lest one name with lang and type
         if ( ((strType == curType) ||
               (strType == ItemTypes::invalidName)) &&
              ((strLang == curLang) ||
               (strLang == LangTypes::invalidLanguage)) &&
              (StringUtility::strcasecmp(nameStr,
                          getName(curIndex)) == 0) ) {
            // same lang, type, and name str
            found = true;
            strIdx = curIndex;
         } else {
            i++;
         }
      } else {
         // no name with lang and type
         found = true; // to break the loop
         strIdx = MAX_UINT32;
      }
   }
   if (found && (strIdx != MAX_UINT32)) {
      retVal = true;
   }

   return (retVal);
}



vector<Item*>
GenericMap::getRegions(const Item* item, ItemTypes::itemType type) const
{
   vector<Item*>items;
   for (uint32 n=0; n<item->getNbrGroups(); ++n) {
      Item* group = itemLookup(item->getGroup(n));
      MC2_ASSERT(group != NULL);
      if (group != NULL){
         if (group->getItemType() == type) {
            items.push_back(group);
         }
      }
   }
   return items;
}
#if 0
uint32
GenericMap:: polygonHasItemsInside(Item* polyItem,
                                     ItemTypes::itemType 
                                     typeOfItemsToLookFor
                                      /* = ItemTypes::numberOfItemTypes*/ )
{
   uint32 result = 0; // No coordinate inside or adjecent to the polygon
                // 1     At least one coordinate adjecent to the polygon
                // 2     At least one coordinate inside the polygon.
   
   GfxData* polyGfx = polyItem->getGfxData();
   if ( polyGfx == NULL){
      // Can't check polygons with no gfx data.
      return result;
   }
   // Get bbox of polyItem
   MC2BoundingBox polyBbox;
   polyGfx->getMC2BoundingBox(polyBbox);

   // Check if any items are within this polyItem.
   for ( int currentZoom=0; 
         currentZoom<NUMBER_GFX_ZOOMLEVELS; 
         currentZoom++) {

      for (uint32 i=0; i<m_itemsZoomSize[currentZoom]; i++) {
         Item* item = m_itemsZoom[currentZoom][i];

         if (item  == NULL) {
            // Can't check items, which are NULL.
            continue;
         }
         if ( ( typeOfItemsToLookFor != ItemTypes::numberOfItemTypes) &&
              ( typeOfItemsToLookFor != item->getItemType() ) )
         {
            // We should not check items of this item type.
            continue;
         }
         GfxData* itemGfx = item->getGfxData();
         if ( itemGfx == NULL ) {
            // Can't check items with no gfx data.
            continue;
         }
         MC2BoundingBox itemBbox;
         itemGfx->getMC2BoundingBox(itemBbox);

         // Check this item.
         if (polyBbox.overlaps(itemBbox)) {
            uint16 nbrPolys = itemGfx->getNbrPolygons();
            uint16 polyIdx = 0;
            while (polyIdx < nbrPolys ){
               uint16 nbrCoords = itemGfx->getNbrCoordinates(polyIdx);
               uint16 coordIdx = 0;
               while  ( coordIdx < nbrCoords ){
                  GfxData::coordinate_type lat =
                     itemGfx->getLat( polyIdx, coordIdx );
                  GfxData::coordinate_type lon = 
                     itemGfx->getLon( polyIdx, coordIdx );
                  result = MAX( static_cast<uint32>
                                ( polyGfx->insidePolygon(lat, lon) ),
                                result );
                  if ( result == 2 ){
                     return result;
                  }
                  ++coordIdx;
               }
               ++polyIdx;
            }
         }
      } // for m_itemsZoomSIze[currentZoom]     
   } // for NUMBER_GFX_ZOOMLEVELS
   
         
   return result;
}


bool 
GenericMap::isUniqueCityCentre( Item* item )
{
   // Respons don't like city centers, always return false.
   // return false;

   PointOfInterestItem* cc = item_cast<PointOfInterestItem*> ( item );
   if ( ( cc != NULL ) && 
        ( cc->isPointOfInterestType( ItemTypes::cityCentre ) ) &&
        ( cc->getNbrNamesWithType( ItemTypes::officialName ) > 0 ) ) {
      uint32 buaID = getRegionID( cc, ItemTypes::builtUpAreaItem ); 
      Item* bua = itemLookup( buaID );

      if ( bua != NULL ) {
         return !oneNameSimilarCase( bua, cc );
      } 
      else {

         // Use the bua of the street.
         uint32 ssiID = cc->getStreetSegmentItemID();
         //      StreetSegmentItem* ssi = item_cast<StreetSegmentItem*> 
         //                         ( itemLookup(ssiID) );
         Item* ssi = itemLookup(ssiID);
         if ( ssi != NULL ){
            vector<Item*>buaRegions = 
               getRegions(ssi, ItemTypes::builtUpAreaItem);
            for(uint32 i=0; i<buaRegions.size(); i++){
               if ( oneNameSimilarCase( buaRegions[i], cc ) ){
                  return false;
               }
            }
            /*
            if (nbrBuas == 1){
               // Only use this bua if it is a single one.
               buaID = getRegionID( cc, ItemTypes::builtUpAreaItem );
               bua = itemLookup( buaID );
               if ( bua != NULL ){
                  return !oneNameSimilarCase( bua, cc );               
               }
               }*/
         }
            


         // No bua when using location. Try geometry.
         uint32 i = 0;
         while ( i < getNbrItemsWithZoom( 0 ) ) {
            BuiltUpAreaItem* tmpBua = 
               item_cast<BuiltUpAreaItem*> ( getItem( 0, i ) );
            if ( tmpBua != NULL ) {
               if ( oneNameSimilarCase( tmpBua, cc ) ) {
                  // Check distance.
                  if ( tmpBua->getGfxData() != NULL ) {
                     MC2Coordinate buaCoord;
                     getOneGoodCoordinate( buaCoord, cc );
                     int64 dist = tmpBua->getGfxData()
                        ->signedSquareDistTo( buaCoord.lat, 
                                              buaCoord.lon ); 
                     if ( dist < SQUARE( 200 ) ) {
                        // Close enough to count as being inside the bua.
                        return false;
                     }
                  } 
               }
            }
            ++i;
         }
      }
      
      return true;
   } else {
      // Not even a city centre with a real name at all. 
      return false;
   }
}
#endif

const MapRights&
GenericMap::getRights( uint32 itemID ) const
{
   return m_userRightsTable->getRights( itemID );
}

bool 
GenericMap::getNodeCoordinates(uint32 nodeID, int32 &lat, int32 &lon) const
{
   Item* i = itemLookup(nodeID);
   MC2_ASSERT(i != NULL);
   if (i == NULL)
      return (false);

   GfxData* gfx = i->getGfxData();
   if ( gfx == NULL ) {
      return false;
   }

   MC2_ASSERT(gfx->getNbrPolygons() == 1);

   if ((gfx == NULL) && gfx->getNbrCoordinates(0) == 0)
      return (false);



   if ((nodeID & 0x80000000) == 0x80000000) {
      // Node 1
      lat = gfx->getLastLat(0);
      lon = gfx->getLastLon(0);
      return (true);
   } else {
      // Node 0
      lat = gfx->getLat(0,0);
      lon = gfx->getLon(0,0);
      return (true);
   }

   return (false);
}

bool
GenericMap::oneNameSimilarCase( const Item* item, 
                                const Item* otherItem ) const
{
   for ( uint32 i = 0; i < item->getNbrNames(); ++i ) {
      const char* name = getName( item->getStringIndex( i ) );
      if ( ( item->getNameType( i ) != ItemTypes::synonymName ) &&
           ( name != NULL ) ) {
         MC2String upperName = 
            StringUtility::copyUpper( name );
         for ( uint32 j = 0; j < otherItem->getNbrNames(); ++j ) {
            const char* otherName = 
               getName( otherItem->getStringIndex( j ) );
            if ( ( otherItem->getNameType( j ) != ItemTypes::synonymName ) &&
                 ( otherName != NULL ) ) {
               
               MC2String upperOtherName =
                  StringUtility::copyUpper( otherName );
               if ( StringSearchUtility::anyWordMatch( upperName.c_str(),
                                                       upperOtherName.c_str()))
               {
                  return true;
               }  
            }
         }
      }
   }
   return false; 
}

bool 
GenericMap::oneNameSameCase( const Item* item, 
                             const Item* otherItem ) const
{
   for ( uint32 i = 0; i < item->getNbrNames(); ++i ) {
      const char* name = getName( item->getStringIndex( i ) );
      if ( ( item->getNameType( i ) != ItemTypes::synonymName ) &&
           ( name != NULL ) ) {
         for ( uint32 j = 0; j < otherItem->getNbrNames(); ++j ) {
            const char* otherName = 
               getName( otherItem->getStringIndex( j ) );
            if ( ( otherItem->getNameType( j ) != ItemTypes::synonymName ) &&
                 ( otherName != NULL ) ) {
               if ( StringUtility::strcasecmp( name, otherName ) == 0 ) {
                  return true;
               }
            }
         }

      }
   }
   return false; 
}

const GfxData*
GenericMap::getItemGfx( const Item* item, GfxDataFull& gfx ) const
{
   {
      const GfxData* itemgfx = item->getGfxData();
      if ( itemgfx ) {
         return itemgfx;
      }
   }
   if ( item->getItemType() == ItemTypes::pointOfInterestItem ) {
      
      const PointOfInterestItem* poi =
         static_cast<const PointOfInterestItem*>(item);
      
      MC2Coordinate ssiOffsetCoord;      
      getItemCoordinates( poi->getStreetSegmentItemID(),
                          poi->getOffsetOnStreet(),
                          ssiOffsetCoord.lat,
                          ssiOffsetCoord.lon );
      
      gfx.setToSinglePoint( ssiOffsetCoord );
      return &gfx;
   }
   // What to do?
   return NULL;
}

bool
GenericMap::itemAllowedByUserRights( uint32 id,
                                     const UserRightsMapInfo& rights ) const
{
   return rights.itemAllowed( m_userRightsTable->getRights( id ),
                              IDPair_t( getMapID(), id ),
                              m_idTranslationTable );
}

bool
GenericMap::itemAllowedByUserRights( const Item& item,
                                     const UserRightsMapInfo& rights ) const
{
   return itemAllowedByUserRights( item.getID(), rights );
}

MC2String
GenericMap::getNameOfNeededRight( uint32 id,
                                  LangTypes::language_t lang ) const
{   
   return m_userRightsTable->getRights( id ).getName( lang );
}

MC2String
GenericMap::getNameOfNeededRight( const Item& item,
                                  LangTypes::language_t lang ) const
{
   return getNameOfNeededRight( item.getID(), lang );
}

bool
GenericMap::itemUsedAsGroup(Item* group) const
{
   bool found = false;
   uint32 z = 0;
   while ( !found && ( z <  NUMBER_GFX_ZOOMLEVELS ) ){

      uint32 i=0;
      while ( !found && ( i < getNbrItemsWithZoom(z) ) ) {

         Item* item = getItem(z, i);
         if ( item != NULL ){
            if ( item->memberOfGroup(group->getID()) ){
               found = true;
               mc2dbg8 << "   GenericMap::itemUsedAsGroup: used for:" 
                       << item->getID() << endl;
            }
         }

         i++;
      }
      z++;
   }
   return found;
}


uint32
GenericMap::getLaneConnectionIdx( uint32 conId ) const {
   ConnectionLaneMap::const_iterator findIt = 
      lower_bound( m_connectionLaneIdx.begin(), m_connectionLaneIdx.end(),
                   conId );
   if ( findIt != m_connectionLaneIdx.end() &&
        findIt->key == conId ) 
   {
      return findIt->value;
   } else {
      return 0;
   }
}

uint32
GenericMap::getLaneConnectionIdx( const Connection& con ) const {
   return getLaneConnectionIdx( getConnectionID( con ) );
}

const GenericMap::NodeLaneMapArray*
GenericMap::getNodeLanes( uint32 nodeId ) const {
   NodeLaneMap::const_iterator findIt = 
      lower_bound( m_nodeLane.begin(), m_nodeLane.end(),
                   nodeId );
   if ( findIt != m_nodeLane.end() &&
        findIt->key == nodeId ) {
      return &findIt->value;
   } else {
      return NULL;
   }
}

const GenericMap::ConnectionSignPostMapArray*
GenericMap::getConnectionSigns( uint32 conID ) const {
   ConnectionSignPostMap::const_iterator findIt =
      lower_bound( m_connectionSignPost.begin(), m_connectionSignPost.end(),
                   conID );
   if ( findIt != m_connectionSignPost.end() &&
        findIt->key == conID ) {
      return &findIt->value;
   } else {
      return NULL;
   }  
}

bool
GenericMap::getConnectToNodeIDs(uint32 fromNodeID, Vector* toNodeIDVec)
{
   // Opposing node means node 0 -> node 1 and vice versa.
   
   // The opposite node of fromNode
   Node* oppositeNode = nodeLookup(fromNodeID ^ 0x80000000);
   if (oppositeNode == NULL) {
      // Invalid fromNodeID
      return (false);
   }
   
   // Go through all nodes connecting to oppositeNode
   for (uint32 i = 0; i < oppositeNode->getNbrConnections(); i++) {
      if ( ! oppositeNode->getEntryConnection(i)->isMultiConnection() ) {
         // Use that all opposing nodes connecting to 
         // oppositeNode must have a connection from toNode.
         toNodeIDVec->addLast(oppositeNode->getEntryConnection(i)
                                         ->getConnectFromNode() 
                                         ^ 0x80000000);
      }
   }
   // Everything OK
   return (true);
}

Connection*
GenericMap::getOpposingConnection(Connection* conn, uint32 toNodeID)
{
   if ( conn->isMultiConnection() ) {
      return NULL;
   }

   Node* fromNode = nodeLookup(conn->getConnectFromNode() ^ 0x80000000);
   uint32 i = 0;
   while (i < fromNode->getNbrConnections()) {
      if (fromNode->getEntryConnection(i)->getConnectFromNode() ==
          (toNodeID ^ 0x80000000)) {
         return (fromNode->getEntryConnection(i));
      } else {
         // Try next connection
         i++;
      }
   }
   
   mc2dbg2 << "Couldn't find opposite connection!" << endl;
   return (NULL);
}

Node* 
GenericMap::nodeLookup(uint32 nodeID) const
{
   Item* i = itemLookup(nodeID); 
   RouteableItem* ri = Item::routeableItem( i );
   if (ri != NULL)
      return ri->getNodeFromID(nodeID);
   else
      return (NULL);
}


bool
GenericMap::getOtherConnections( Connection* conn, 
                           uint32 toNodeID,
                           vector<pair<Connection*, Node*> >& otherConn )
{
   Node* toNode = nodeLookup( toNodeID );
   if ( ( toNode == NULL ) || ( conn == NULL ) || 
        ( conn->isMultiConnection() ) ) {
      return false;
   }
   
   Connection* oppConn = getOpposingConnection(conn, toNodeID);
   uint32 oppToNodeID = conn->getConnectFromNode() ^ 0x80000000;
   //cout << "OppToNode: " << oppToNodeID << " OppFromNode: "
   //     << oppConn->getConnectFromNode() << endl;
   Node* oppToNode = nodeLookup( oppToNodeID );
   //cout << "Nbr conn: " << oppToNode->getNbrConnections() << endl;
   for ( uint32 i = 0; i < oppToNode->getNbrConnections(); ++i ) {
      Connection* oppToNodeConn = oppToNode->getEntryConnection(i);
      //cout << "i:" << i << " FromNode:"
      //     << oppToNodeConn->getConnectFromNode() << endl;
      if ( ( ! oppToNodeConn->isMultiConnection() ) && 
           ( oppToNodeConn->getConnectFromNode() !=
             oppConn->getConnectFromNode() ) ) {
         //cout << "if-sats ok" << endl;
         Connection* returnConn = getOpposingConnection
                              ( oppToNodeConn,
                                oppToNodeID);
         otherConn.push_back( make_pair(returnConn, 
               nodeLookup( oppToNodeConn
                  ->getConnectFromNode() ^ 0x80000000 ) ) );
      }
   }
   
   return true;

}

bool
GenericMap::isPartOfStreet(const Item* item) const
{  
   uint32 nbrGroups = item->getNbrGroups();
   uint32 i = 0;
   bool partOfStreet = false;
   
   while ((i < nbrGroups) && (!partOfStreet)) {
      // Check if the group item is a street.
      StreetItem* si = item_cast<StreetItem*> 
         (itemLookup(item->getGroup(i)));
      if (si != NULL) {
         partOfStreet = true;
      } else {
         i++;
      }
   }
   
   return (partOfStreet);
   
}

bool
GenericMap::isPartOfGfxStreet(const Item* item) const
{  
   uint32 nbrGroups = item->getNbrGroups();
   uint32 i = 0;
   bool partOfStreet = false;
   
   while ((i < nbrGroups) && (!partOfStreet)) {
      // Check if the group item is a street.
      StreetItem* si = item_cast<StreetItem*> 
         (itemLookup(item->getGroup(i)));
      if ( (si != NULL) && (si->getGfxData() != NULL) ) {
         partOfStreet = true;
      } else {
         i++;
      }
   }
   
   return (partOfStreet);
   
}

uint32
GenericMap::partOfNbrStreets(const Item* item) const
{
   uint32 count = 0;

   for (uint32 i = 0; i < item->getNbrGroups(); i++) {
      if ( item_cast<StreetItem*> 
            (itemLookup(item->getGroup(i))) != NULL ) {
         count++;
      }
   }
  
   return (count);
}

uint32
GenericMap::getCityPartID(const Item* item) const 
{
   Item* cp = getRegion(item, ItemTypes::cityPartItem);
   if (cp != NULL)
      return cp->getID();
   return MAX_UINT32;
}

uint32
GenericMap::getRegionID(const Item* item, ItemTypes::itemType type) const
{
   Item* groupItem = getRegion(item, type);
   if (groupItem != NULL)
      return groupItem->getID();
   return MAX_UINT32;
}

Item* 
GenericMap::getRegion(const Item* item, ItemTypes::itemType type) const
{
   uint32 n=0;
   while (n<item->getNbrGroups()) {
      //      mc2log << info << "GroupID: " << item->getGroup(n) << endl;

      //      if ( ! MapBits::isCountryMap(getMapID())){
         // We can't expect to find groups with the right ID in 
         // country overview maps.

         Item* group = itemLookup(item->getGroup(n));
         MC2_ASSERT(group != NULL);
         if (group->getItemType() == type) {
            return group;
         }
         //      }
      ++n;
   }
   return NULL;
}

byte
GenericMap::getNbrRegions(const Item* item, ItemTypes::itemType type) const
{
   byte nbrFound=0;
   for (uint32 n=0; n<item->getNbrGroups(); ++n) {
      Item* group = itemLookup(item->getGroup(n));
      MC2_ASSERT(group != NULL);
      if (group != NULL){
         if (group->getItemType() == type) {
            ++nbrFound;
         }
      }
   }
   return nbrFound;
}

uint32 
GenericMap::getRegionIDIndex(const Item* item, uint32 i,
                             ItemTypes::itemType type) const
{
   Item* loc = getRegionIndex(item, i, type);
   if (loc != NULL)
      return loc->getID();
   return MAX_UINT32;
}

Item*
GenericMap::getRegionIndex(const Item* item, uint32 i,
                             ItemTypes::itemType type) const
{
   uint32 n=0, nbrFound=0;
   while (n<item->getNbrGroups()) {
      Item* group = itemLookup(item->getGroup(n));
      MC2_ASSERT(group != NULL);
      if (group->getItemType() == type) {
         if (i == nbrFound)
            return group;
         ++nbrFound;
      }
      ++n;
   }
   return NULL;
}
         
bool 
GenericMap::containsRegion( const Item* item, 
                            const Item* region ) const
{
   MC2_ASSERT( item != NULL );
   MC2_ASSERT( region != NULL );
   ItemTypes::itemType type = region->getItemType();
   uint32 nbrRegions = getNbrRegions( item, type );
   for ( uint32 i = 0; i < nbrRegions; ++i ) {
      if ( getRegionIDIndex( item, i, type ) == region->getID() ) {
         return true;
      }
   }
   // The region wasn't found.
   return false;
}

Item*
GenericMap::getIndexAreaFor( const Item* item ) const {
   // Get the index area with the highest order
   Item* res = NULL;
   uint32 highestOrder = 0;
   for ( uint32 i = 0, end = item->getNbrGroups() ; i < end ; ++i ) {
      if ( isIndexArea( item->getGroup( i ) ) ) {
         if ( getIndexAreaOrder( item->getGroup( i ) ) > highestOrder ) {
            highestOrder = getIndexAreaOrder( item->getGroup( i ) );
            res = itemLookup( item->getGroup( i ) );
         }
      }
   }
   return res;
}

void
GenericMap::setUserRights(UserRightsItemTable::itemMap_t& itemMap)
{
   //UserRightsItemTable itemMap_t oldURItems = 
   // m_userRightsTable->getItems();
   //
   // Add the old to the new ones and set them to the new URIT.
   
   MapRights defaultVal = ~MapRights();
   UserRightsItemTable newURIT( defaultVal, itemMap );
   m_userRightsTable->swap(newURIT);

} // GenericMap::setUserRight

int
GenericMap::getUserRights(UserRightsItemTable::itemMap_t& itemMap)
{
   return m_userRightsTable->getItems(itemMap);

} // GenericMap::getUserRight


bool
GenericMap::isIndexArea( uint32 itemID ) const {
   return getIndexAreaOrder( itemID ) != MAX_UINT32;
}

uint32
GenericMap::getIndexAreaOrder( uint32 itemID ) const {
   IndexAreaOrderMap::const_iterator findIt = 
      lower_bound( m_indexAreaOrderMap.begin(), m_indexAreaOrderMap.end(),
                   itemID );
   if ( findIt != m_indexAreaOrderMap.end() &&
        findIt->key == itemID ) {
      return findIt->value;
   } else {
      return MAX_UINT32;
   }
}

void
GenericMap::dbgPrintFilePos(MC2String message, 
                            int outfile, 
                            DataBuffer* dbuf)
{
   off_t filePos = lseek(outfile, 0, SEEK_CUR);
   if ( filePos == (off_t)-1 ){
      mc2dbg << "FilePos: Could not get position in file: "
             << outfile << endl;
   }
   else {
      uint32 pos = filePos;
      if ( dbuf!=NULL ){
         pos+=dbuf->getCurrentOffset();
      }
      mc2dbg << "FilePos: 0x" << hex << pos << dec << ": " << message 
             << endl;
   }
} // dbgPrintFilePos

GenericMap::adminAreaCentreTable_t
GenericMap::createCentreCoordinatesForAdminAreas(
      ItemTypes::itemType adminType )
{

   uint32 mapId = getMapID();
   mc2dbg4 << "cCCFAA starts, for adminType="
           << ItemTypes::getItemTypeAsString(adminType)
           << ", creating from map " << mapId << endl;

   // Count how many admin area items there are so we can compare at the end.
   uint32 nbrAdminItems = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         Item* item = getItem(z, i);
         if ( (item != NULL) && (item->getItemType() == adminType) ) {
            nbrAdminItems++;
         }
      }
   }

   // Find all POI city centres in this map. For each city centre, check
   // if it located in an admin area item (adminType) with the same name
   // as the city centre. If so, store info about this in a map.

   // A map with admin area id connected to a city centre mc2 coordinate
   // Do we need a multimap??
   adminAreaCentreTable_t adminCentres;
   adminAreaCentreTable_t::const_iterator it;

   for ( uint32 i = 0; i < getNbrItemsWithZoom( ItemTypes::poiiZoomLevel );
         i++ ) {
      PointOfInterestItem* poi = item_cast<PointOfInterestItem*>(
                           getItem( ItemTypes::poiiZoomLevel, i ) );
      if ( poi == NULL ) {
         continue;
      }
      if ( ! poi->isPointOfInterestType(ItemTypes::cityCentre) ) {
         continue;
      }

      // we have a city centre poi, get admin area items for this cc
      uint32 ssiId = poi->getStreetSegmentItemID();
      Item* ssi = itemLookup( ssiId );
      if ( ssi == NULL ) {
         continue;
      }
      vector<Item*> adminItems = getRegions(ssi, adminType);

      for ( vector<Item*>::iterator a = adminItems.begin();
            a != adminItems.end(); a++ ) {
         // ok check if the city centre share a name with this admin area
         // this oneNameSameCase method does not compare synonymNames
         if ( oneNameSameCase((*a), poi) ) {

            // FIXME: only want to check officialnames?
            // Some city centres have the bua names as alternative names
            // (e.g. minor city centres i Malmö)
            
            uint32 adminId = (*a)->getID();
            mc2dbg8 << "cCCFAA: same name" << " admin: " << adminId << " "
                 << getFirstItemName( *a )
                 << " cc: " << poi->getID() << " (waspID " << poi->getWASPID()
                 << ") " << getFirstItemName( poi )
                 << endl;
            
            // Get a MC2Coordinate for the cc
            MC2Coordinate poiCoord;
            if ( poi->getGfxData() != NULL ) {
               poiCoord = MC2Coordinate( poi->getGfxData()->getLat(0,0),
                                         poi->getGfxData()->getLon(0,0));
            } else {
               // get coordinate from offset on ssi
               uint16 offset = poi->getOffsetOnStreet();
               int32 lat, lon;
               ssi->getGfxData()->getCoordinate(offset, lat, lon);
               poiCoord = MC2Coordinate( lat, lon );
            }
            
            // Add to adminCentres
            // If this admin area already has a coord - ERROR???
            it = adminCentres.find( adminId );
            if ( it != adminCentres.end() ) {
               mc2log << error 
                      << "cCCFAA: ERROR admin area " << mapId << ":"
                      << adminId << " " << getFirstItemName( *a )
                      << " already has a coordinate " << it->second
                      << ", also matching poi " << poi->getID() << " (waspID=" 
                      << poi->getWASPID() << ")"
                      << " coord " << poiCoord << endl;
            } else {
               mc2dbg4 << "cCCFAA: admin area " << mapId << ":" << adminId 
                    << " " << getFirstItemName( *a )
                    << " coord " << poiCoord
                    << " from waspID=" << poi->getWASPID() << endl;
               
               adminCentres.insert( make_pair(adminId, poiCoord) );
            }
         }
      }
   }

   // Check if any admin area did not get a centre coordinate
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         Item* item = getItem(z, i);
         if ( (item != NULL) && (item->getItemType() == adminType) ) {
            if ( adminCentres.find(item->getID()) == adminCentres.end() ) {
               mc2dbg2 << "cCCFAA: no centre coordinate for admin area "
                    << mapId << ":" << item->getID() << " " 
                    << getFirstItemName( item ) << " ("
                    << ItemTypes::getItemTypeAsString(adminType) << ")"
                    << endl;
            }
         }
      }
   }
   mc2log << info << "cCCFAA: created " << adminCentres.size()
          << " admin area centre coords for " << nbrAdminItems << " "
          << ItemTypes::getItemTypeAsString(adminType)
          << "s in map " << mapId << endl;

   return adminCentres;
}


void
GenericMap::setAminAreaCentres ( const adminAreaCentreTable_t& adminCentres )
{
   // loop the map with centres and put in m_adminAreaCentres

   m_nbrAdminAreaCentres = 0;

   // then make new, and fill
   m_nbrAdminAreaCentres = adminCentres.size();
   m_adminAreaCentres.reset( new adminAreaCentre_t[m_nbrAdminAreaCentres] );
   uint32 i = 0;
   adminAreaCentreTable_t::const_iterator it;
   for ( it = adminCentres.begin(); it != adminCentres.end(); it++ ) {
      adminAreaCentre_t elem;
      elem.itemID = it->first;
      elem.centre = it->second;

      m_adminAreaCentres[i] = elem;
      i++;
   }
   
}

GenericMap::adminAreaCentreTable_t
GenericMap::getAdminAreaCentres() const
{
   adminAreaCentreTable_t adminCentres;
   for ( uint32 i = 0; i < m_nbrAdminAreaCentres; i++ ) {
      adminAreaCentre_t elem = m_adminAreaCentres[i];

      adminCentres.insert( make_pair(elem.itemID, elem.centre) );
   }
   return adminCentres;
}

vector<ItemTypes::itemType>
GenericMap::getItemTypesToCreateAdminCentres()
{
   vector<ItemTypes::itemType> adminTypes;
   adminTypes.push_back(ItemTypes::municipalItem);
   adminTypes.push_back(ItemTypes::builtUpAreaItem);

   return adminTypes;
}

uint32
GenericMap::createAndSetCentreCoordinatesForAdminAreas()
{
   if (! MapBits::isUnderviewMap( getMapID()) ) {
      mc2log << error 
             << "GenericMap::createAndSetCentreCoordinatesForAdminAreas: "
             << " only use this method for underviews" << endl;
      return 0;
   }

   // Collect any zip centre coordinates already stored in this map
   // since they cannot be created here (from information in this map).
   adminAreaCentreTable_t myAdminAreaCentres = getAdminAreaCentres();
   adminAreaCentreTable_t zipCentres;
   adminAreaCentreTable_t::const_iterator it;
   for ( it = myAdminAreaCentres.begin();
         it != myAdminAreaCentres.end(); it++ ) {
      uint32 id = it->first;
      ZipCodeItem* zip = item_cast<ZipCodeItem*>( itemLookup(id) );
      if ( zip != NULL ) {
         zipCentres.insert( make_pair(it->first, it->second) );
      }
   }
   if ( zipCentres.size() > 0 ) {
      mc2dbg << "GenericMap::createAndSetCentreCoordinatesForAdminAreas: "
             << "saved centres coordinates for " << zipCentres.size()
             << " zip codes" << endl;
   }
   
   
   // Create and collect centres for all admin area item types
   vector<ItemTypes::itemType> adminTypes = getItemTypesToCreateAdminCentres();
   adminAreaCentreTable_t adminCentres;
   for ( vector<ItemTypes::itemType>::const_iterator type = adminTypes.begin();
         type != adminTypes.end(); type++ ) {
      
      adminAreaCentreTable_t centres = 
         createCentreCoordinatesForAdminAreas( (*type) );
      
      // add the centres to the big map
      for ( it = centres.begin(); it != centres.end(); it++ ) {
         adminCentres.insert( make_pair(it->first, it->second) );
      }
      
   }
   // Re-add the zip centres
   if ( zipCentres.size() > 0 ) {
      for ( it = zipCentres.begin(); it != zipCentres.end(); it++ ) {
         adminCentres.insert( make_pair(it->first, it->second) );
      }
   }

   // Replace the admin centres in this map
   setAminAreaCentres( adminCentres );

   mc2log << info << "Filled admin area centres table with totally "
          << adminCentres.size() << " centre coordinates" << endl;
   return ( adminCentres.size() );
}

bool
GenericMap::updateAdminAreaCentres()
{
   bool updated = false;
   if (! MapBits::isUnderviewMap( getMapID()) ) {
      mc2log << error << "GenericMap::updateAdminAreaCentres: "
             << " only use this method for underviews" << endl;
      return false;
   }
   mc2dbg2 << "GMap::updateAdminAreaCentres: map " << getMapID() << endl;
   
   // Check if the admin area centres need to be updated
   // Please save the zipCode centre coordinates, because they cannot
   // be created from the map content.
   // 1. Get the admin area centres from the table of this map
   // 2. Create/calculate admin area centres in this map
   // 3. Compare the content, re-set the table if they differ

   // 1
   adminAreaCentreTable_t myAdminAreaCentres = getAdminAreaCentres();
   adminAreaCentreTable_t zipCentres;
   adminAreaCentreTable_t::const_iterator it;
   for ( it = myAdminAreaCentres.begin();
         it != myAdminAreaCentres.end(); it++ ) {
      uint32 id = it->first;
      ZipCodeItem* zip = item_cast<ZipCodeItem*>(itemLookup(id));
      if ( zip != NULL ) {
         zipCentres.insert( make_pair(it->first, it->second) );
      }
   }
   mc2dbg << "GenericMap::updateAdminAreaCentres: "
          << "saved centres coordinates for " << zipCentres.size()
          << " zip codes" << endl;

   // 2
   adminAreaCentreTable_t newAdminAreaCentres;
   vector<ItemTypes::itemType> adminTypes = getItemTypesToCreateAdminCentres();

   for ( vector<ItemTypes::itemType>::const_iterator type = adminTypes.begin();
         type != adminTypes.end(); type++ ) {
      
      adminAreaCentreTable_t centres = 
         createCentreCoordinatesForAdminAreas( (*type) );
      
      for ( it = centres.begin(); it != centres.end(); it++ ) {
         newAdminAreaCentres.insert( make_pair(it->first, it->second) );
      }
   }
   // also include the zip centres
   if ( zipCentres.size() > 0 ) {
      for ( it = zipCentres.begin(); it != zipCentres.end(); it++ ) {
         newAdminAreaCentres.insert( make_pair(it->first, it->second) );
      }
   }

   // Dynamic wasping might be done on filtered maps, filtering 1 meter.
   // The poi coord from offset on ssi will not be exactly the same,
   // it will differ some mc2 units. We don't want this to result in
   // updated admin area centre coordinates.
   // So if the coordinate moved < 2 meters don't update.

   // 3
   if ( myAdminAreaCentres != newAdminAreaCentres ) {
      // count how many have changed
      uint32 nbrDiffers = 0;
      uint32 nbrNew = 0;

      // for each coord in the new admin area centres, check how much they
      // differ compared to the original coordinates. They must move > 2 m
      // to be replaced.
      adminAreaCentreTable_t updatedAdminAreaCentres;
      adminAreaCentreTable_t::const_iterator jt;
      for ( it = newAdminAreaCentres.begin();
            it != newAdminAreaCentres.end(); it++ ) {
         uint32 id = it->first;
         MC2Coordinate newCoord = it->second;

         bool useNew = true;
         jt = myAdminAreaCentres.find(id);
         if ( jt != myAdminAreaCentres.end() ) {
            if ( newCoord != jt->second ) {
               nbrDiffers++;
               float64 sqDist = GfxUtility::squareP2Pdistance_linear(
                  newCoord.lat, newCoord.lon, 
                  jt->second.lat, jt->second.lon );
               mc2dbg2 << " GMap::uAAC: coord changed for admin id "
                    << id << " " << jt->second << " -> "
                    << newCoord << ", sqDist=" << sqDist << endl;
               if ( sqDist < 4 ) { // 2 meters squared
                  // the admin centre coord differs less than 1 meters
                  // don't update it
                  useNew = false;
               }
            } else {
               // the admin centre coordinate is equal
               useNew = false;
            }
         }
         if ( useNew ) {
            nbrNew++;
            updatedAdminAreaCentres.insert( make_pair(id, newCoord) );
            mc2dbg << "GMap::updateAdminAreaCentres: "
                   << "use new coord for id " << id << endl;
         } else {
            updatedAdminAreaCentres.insert( make_pair(id, jt->second) );
         }
      }
      mc2dbg << "GMap::updateAdminAreaCentres: "
             << "using " << nbrNew << " admin coords from update ("
             << nbrDiffers << " coords changed in the update)" << endl;
      // size of newAAC and updatedAAC must be the same
      if ( updatedAdminAreaCentres.size() != myAdminAreaCentres.size() ) {
         mc2dbg << "GenericMap::updateAdminAreaCentres: "
                << "ERROR size of newAAC and updatedAAC not the same" << endl;
      }
      
      if ( nbrNew > 0 ) { // updatedAdminAreaCentres != myAdminAreaCentres
         setAminAreaCentres( updatedAdminAreaCentres );
         updated = true;
      }
   }

   mc2log << info << "GMap: Admin area centres ";
   if ( !updated ) {
      mc2log << "NOT ";
   }
   mc2log << "updated in map " << getMapID() << ", was: " 
          << myAdminAreaCentres.size()
          << " now: " << newAdminAreaCentres.size() << endl;
          
   return updated;
   
}

vector<uint32>
GenericMap::getItemsInGroup(uint32 groupID) const{
   vector<uint32>result;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         Item* item = getItem(z, i);
         if ( item == NULL) {
            continue;
         }
         if (item->memberOfGroup(groupID)){
            result.push_back(item->getID());
         }
      }
   }
   return result;
}

void GenericMap::saveSignPosts( int outfile ) {

   using namespace DataBufferUtil;

   // New signposts
   std::auto_ptr<DataBuffer> dataBuffer;

   // Write size of map
   dataBuffer.reset( new DataBuffer( 4 ) );
   dataBuffer->writeNextLong( m_connectionSignPost.size() );
   DataBufferUtil::saveBuffer( *dataBuffer.get(), outfile );

   mc2dbg2 << "saveSignPosts nbr connections with signposts: " 
           << m_connectionSignPost.size() << endl;
   for ( ConnectionSignPostMap::const_iterator it =
            m_connectionSignPost.begin() ;
         it != m_connectionSignPost.end() ; ++it ) {
      // ConnectionID, nbr Signs
      DataBuffer buff( 4+4 + (*it).value.size()*12 );
      buff.writeNextLong( (*it).key );
      buff.writeNextLong( (*it).value.size() );

      for ( uint32 l = 0 ; l < (*it).value.size() ; ++l ) {
         // nbr Signs sign data
         (*it).value[ l ].save( buff );
      }
      buff.alignToLongAndClear();
      DataBufferUtil::saveBuffer( buff, outfile );
   }
}

void GenericMap::loadSignPosts( DataBuffer& dataBuffer ) {
   // at this point we have all connections allocated
   // so we know how to map them to our sign post map

   // New signposts
   uint32 nbr = dataBuffer.readNextLong();
   m_connectionSignPost.resize( nbr );
   for ( uint32 i = 0; i < nbr; ++i ) {
      // ConnectionID, nbr Signs
      uint32 connID = dataBuffer.readNextLong();
      uint32 nbrSigns = dataBuffer.readNextLong();
      SignPost* signArray = m_signPostAllocator->getNewArray( nbrSigns );
      for ( uint32 l = 0 ; l < nbrSigns ; ++l ) {
         signArray[ l ].load( dataBuffer );
      }
      m_connectionSignPost[ i ] = make_vpair( 
         connID, ConnectionSignPostMapArray( signArray, nbrSigns ) );
   }
   mc2dbg2 << "loadSignPosts nbr connections with signposts: " 
           << m_connectionSignPost.size() << endl;
}

void
GenericMap::loadLanes( DataBuffer& buff ) {
   // Segment
   uint32 nbr = buff.readNextLong();
   mc2dbg1 << "loadLanes nbr " << nbr << endl;
   m_nodeLane.resize( nbr );
   for ( uint32 i = 0; i < nbr; ++i ) {
      // SegmentId, nbr lanes
      uint32 segId = buff.readNextLong();
      byte nbrLanes = buff.readNextByte();
      buff.readNextByte(); // Padd byte
      ExpandStringLane* laneArray = m_laneAllocator->getNewArray( nbrLanes );
      mc2dbg2 << "  lanegroup " << segId << " nbr lanes " << int(nbrLanes);
      for ( byte l = 0 ; l < nbrLanes ; ++l ) {
         // nbr lanes Lane data
         PacketDataBuffer::loadAsFromPacket( buff, laneArray[ l ] );
         mc2dbg2 << " " << laneArray[ l ];
      }
      mc2dbg2 << endl;
             
      m_nodeLane[ i ] = make_vpair( 
         segId, NodeLaneMapArray( laneArray, nbrLanes ) );
   }
   MC2_ASSERT( is_sorted( m_nodeLane.begin(), m_nodeLane.end() ) );

   // Connection
   nbr = buff.readNextLong();
   mc2dbg1 << "loadLanes connections " << nbr << endl;
   m_connectionLaneIdx.resize( nbr );
   for ( uint32 i = 0; i < nbr; ++i ) {
      // Connection id, nbr lane ids
      uint32 conId = buff.readNextLong();
      uint32 laneIdx = buff.readNextLong();
      mc2dbg2 << " con " << conId << " ix " << MC2HEX( laneIdx ) << endl;
      m_connectionLaneIdx[ i ] = make_vpair( conId, laneIdx );
   }
   MC2_ASSERT( is_sorted( m_connectionLaneIdx.begin(), 
                          m_connectionLaneIdx.end() ) );
}

void
GenericMap::saveLanes( int outfile ) {
   std::auto_ptr<DataBuffer> dataBuffer;

   // Write size of segment lanes map
   dataBuffer.reset( new DataBuffer( 4 ) );
   dataBuffer->writeNextLong( m_nodeLane.size() );
   mc2dbg1 << "saveLanes nbr " << m_nodeLane.size() << endl;
   DataBufferUtil::saveBuffer( *dataBuffer.get(), outfile );
   for ( NodeLaneMap::const_iterator it = m_nodeLane.begin() ; 
         it != m_nodeLane.end() ; ++it ) {
      // SegmentId, nbr lanes
      DataBuffer buff( 4+1+ 1/*padd*/+ (*it).value.size()*2 + 3/*align*/);
      buff.writeNextLong( (*it).key );
      buff.writeNextByte( (*it).value.size() );
      buff.writeNextByte( 0 ); // Padd byte
      
      mc2dbg2 << "  lanegroup " << (*it).key << " nbr lanes " 
             << int((*it).value.size());
      for ( byte l = 0 ; l < (*it).value.size() ; ++l ) {
         // nbr lanes Lane data
         PacketDataBuffer::saveAsInPacket( buff, (*it).value[ l ] );
         mc2dbg2 << " " << (*it).value[ l ];
      }
      mc2dbg2 << endl;
      buff.alignToLongAndClear();
      DataBufferUtil::saveBuffer( buff, outfile );
   }

   // Connection lanes indexes map
   dataBuffer.reset( new DataBuffer( 4 ) );
   dataBuffer->writeNextLong( m_connectionLaneIdx.size() );
   mc2dbg1 << "saveLanes connections " << m_connectionLaneIdx.size() << endl;
   DataBufferUtil::saveBuffer( *dataBuffer.get(), outfile );
   for ( ConnectionLaneMap::const_iterator it = m_connectionLaneIdx.begin() ; 
         it != m_connectionLaneIdx.end() ; ++it ) {
      // Connection id, nbr lane ids
      DataBuffer buff( 8 );
      buff.writeNextLong( (*it).key );
      buff.writeNextLong( (*it).value  );
      mc2dbg2 << " con " << (*it).key << " ix " << MC2HEX( (*it).value )
             << endl;
      DataBufferUtil::saveBuffer( buff, outfile );
   }
}

void
GenericMap::loadCategoryIds( DataBuffer& buff ) {

   // load it...
   uint32 nbrOfKeys = buff.readNextLong();
   m_categoryIds.resize( nbrOfKeys );
   mc2dbg << "[GenericMap] nbr of category keys: " << nbrOfKeys << endl;
   for ( uint32 key = 0; key < nbrOfKeys; ++key ) {

      uint32 keyValue = buff.readNextLong();      
      uint32 nbrCategories = buff.readNextLong();
      CategoryArray value( m_categoryAllocator->getNewArray( nbrCategories ),
                           nbrCategories );
      CategoryArray::iterator catIt = value.begin();
      for ( ; catIt != value.end(); ++catIt ) {
         *catIt = buff.readNextLong();
      }
      m_categoryIds[ key ] = make_vpair( keyValue, value );
   }
}

void
GenericMap::saveCategoryIds( int outfile ) const {
   // write map size
   {
      DataBuffer longBuff( 4 );
      longBuff.writeNextLong( m_categoryIds.size() );
      DataBufferUtil::saveBuffer( longBuff, outfile );
   }
   mc2dbg << "[GenericMap] nbr of keys: " << m_categoryIds.size() << endl;
   // write category ids
   CategoryMap::const_iterator it = m_categoryIds.begin();
   CategoryMap::const_iterator itEnd = m_categoryIds.end();
   DataBuffer longlongBuff( 8 );
   for ( ; it != itEnd; ++it ) {
      // write key
      longlongBuff.writeNextLong( (*it).key );
      // write nbr categories
      mc2dbg2 << "[GenericMap] nbr categories: " << (*it).value.size() << endl;
      longlongBuff.writeNextLong( (*it).value.size() );
      DataBufferUtil::saveBuffer( longlongBuff, outfile );
      longlongBuff.reset();

      DataBuffer buff( (*it).value.size() * 4 );
      for_each( (*it).value.begin(), (*it).value.end(),
                bind1st( mem_fun( &DataBuffer::writeNextLong ), &buff ) );

      DataBufferUtil::saveBuffer( buff, outfile );
   }
}

void GenericMap::loadRoadDisplayClasses( DataBuffer& buff ) {
   uint32 nbrOfPairs = buff.readNextBALong();

   for ( uint32 i = 0; i < nbrOfPairs; ++i ) {
      uint32 itemId = buff.readNextBALong();
      ItemTypes::roadDisplayClass_t displayClass = 
         static_cast<ItemTypes::roadDisplayClass_t>( buff.readNextByte() );

      Item* item = itemLookup( itemId );

      if ( item == NULL ||
           item->getItemType() != ItemTypes::streetSegmentItem ) {
         mc2log << warn << "Invalid road display table, item " 
                << MC2HEX( itemId ) << " is not a valid StreetSegmentItem"
                << endl;
      }
      else {
         StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>( item );
         ssi->setDisplayClass( displayClass );
      }
   }
}

void GenericMap::saveRoadDisplayClasses( int outfile ) const {
   vector< pair< uint32, ItemTypes::roadDisplayClass_t > > displayClasses;

   // Find all Street Segment Items with a special display class
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i ) {
         Item* item = getItem( z, i );

         if ( item != NULL && 
              item->getItemType() == ItemTypes::streetSegmentItem ) {
            StreetSegmentItem* ssi = static_cast<StreetSegmentItem*>( item );
            ItemTypes::roadDisplayClass_t displayClass = ssi->getDisplayClass();
            if ( displayClass != ItemTypes::nbrRoadDisplayClasses ) {
               displayClasses.push_back( make_pair( item->getID(), 
                                                    displayClass ) );
            }
         }
      }
   }

   // save the display classes to a databuffer

   const int SizeOfPair = sizeof( uint32 ) + sizeof( byte );
   DataBuffer buff( sizeof( uint32 ) + displayClasses.size() * SizeOfPair );

   // first save number of pairs
   buff.writeNextBALong( displayClasses.size() );

   // then all the pairs
   for ( size_t i = 0; i < displayClasses.size(); ++i ) {
      buff.writeNextBALong( displayClasses[ i ].first );
      buff.writeNextByte( displayClasses[ i ].second );
   }

   DataBufferUtil::saveBuffer( buff, outfile );
}

void GenericMap::loadAreaDisplayClasses( DataBuffer& buff ) {
   uint32 nbrOfPairs = buff.readNextBALong();

   for ( uint32 i = 0; i < nbrOfPairs; ++i ) {
      uint32 itemId = buff.readNextBALong();
      ItemTypes::areaFeatureDrawDisplayClass_t displayClass = 
         static_cast<ItemTypes::areaFeatureDrawDisplayClass_t>( 
            buff.readNextByte() );

      Item* item = itemLookup( itemId );

      if ( item != NULL ) {
         if ( item->getItemType() == ItemTypes::waterItem ) {
            WaterItem* water = static_cast<WaterItem*>( item );
            water->setDisplayClass( displayClass );
         } else if ( item->getItemType() == ItemTypes::islandItem ) {
            IslandItem* island = static_cast<IslandItem*>( item );
            island->setDisplayClass( displayClass );
         } else if ( item->getItemType() == ItemTypes::builtUpAreaItem ) {
            BuiltUpAreaItem* bua = static_cast<BuiltUpAreaItem*>( item );
            bua->setDisplayClass( displayClass );
         }
      } else {
         mc2log << warn << "Invalid area feature draw display table, item "
                << MC2HEX( itemId ) << " is not valid for display class"
                << endl;
      }
   }
}

void GenericMap::saveAreaDisplayClasses( int outfile ) const {
   vector< pair< uint32, ItemTypes::areaFeatureDrawDisplayClass_t > > 
      displayClasses;

   // Find all area items with a special display class
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i ) {
         Item* item = getItem( z, i );

         if( item != NULL) {
            if ( item->getItemType() == ItemTypes::waterItem ) {
               WaterItem* water = static_cast<WaterItem*>( item );
               ItemTypes::areaFeatureDrawDisplayClass_t displayClass = 
                  water->getDisplayClass();
               if ( displayClass != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
                  displayClasses.push_back( make_pair( item->getID(), 
                                                       displayClass ) );
               }
            }
            else if ( item->getItemType() == ItemTypes::islandItem ) {
               IslandItem* island = static_cast<IslandItem*>( item );
               ItemTypes::areaFeatureDrawDisplayClass_t displayClass = 
                  island->getDisplayClass();
               if ( displayClass != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
                  displayClasses.push_back( make_pair( item->getID(), 
                                                       displayClass ) );
               }
            }
            else if ( item->getItemType() == ItemTypes::builtUpAreaItem ) {
               BuiltUpAreaItem* bua = static_cast<BuiltUpAreaItem*>( item );
               ItemTypes::areaFeatureDrawDisplayClass_t displayClass = 
                  bua->getDisplayClass();
               if ( displayClass != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
                  displayClasses.push_back( make_pair( item->getID(), 
                                                       displayClass ) );
               }
            }  
         }

      }
   }

   // save the display classes to a databuffer

   const int SizeOfPair = sizeof( uint32 ) + sizeof( byte );
   DataBuffer buff( sizeof( uint32 ) + displayClasses.size() * SizeOfPair );

   // first save number of pairs
   buff.writeNextBALong( displayClasses.size() );

   // then all the pairs
   for ( size_t i = 0; i < displayClasses.size(); ++i ) {
      buff.writeNextBALong( displayClasses[ i ].first );
      buff.writeNextByte( displayClasses[ i ].second );
   }

   DataBufferUtil::saveBuffer( buff, outfile );
}

void
GenericMap::loadIndexAreaOrders( DataBuffer& buff ) {
   // Nbr tuples
   uint32 nbr = buff.readNextLong();
   m_indexAreaOrderMap.reserve( nbr );
   // Load the tuples
   for ( uint32 i = 0, end = nbr ; i != end ; ++i ) {
      uint32 key = buff.readNextLong();
      uint32 value = buff.readNextLong();
      m_indexAreaOrderMap.push_back( make_vpair( key, value ) );
   } 
}

void
GenericMap::saveIndexAreaOrders( int outfile ) {
   std::auto_ptr<DataBuffer> dataBuffer;
   dataBuffer.reset( new DataBuffer( 
                        4 + sizeof( IndexAreaOrderMap::value_type )
                        *m_indexAreaOrderMap.size() ) );
   // Nbr tuples
   dataBuffer->writeNextLong( m_indexAreaOrderMap.size() );
   // Save the tuples
   for ( IndexAreaOrderMap::const_iterator it = m_indexAreaOrderMap.begin(),
            end = m_indexAreaOrderMap.end(); it != end ; ++it ) {
      dataBuffer->writeNextLong( it->key );
      dataBuffer->writeNextLong( it->value );
   }
   // Write result to file
   DataBufferUtil::saveBuffer( *dataBuffer, outfile );
}


auto_ptr<POIInfo> GenericMap::getPOIInfo( const PointOfInterestItem* poi ) const {
   if ( poi == NULL ) {
      return auto_ptr<POIInfo>();
   }

   mc2dbg8 << "[GenericMap::getPOIInfo]: poi " << poi->getID() << endl;
   mc2dbg8 << "[GenericMap::getPOIInfo]: poi.WASPID = " << poi->getWASPID() 
           << endl;

   // calculate offset in poi data
   uint32 offset = 
      m_poiOffsetTable[ m_pointOfInterestItemAllocator->getPosition( poi ) ];

   mc2dbg8 << "[GenericMap::getPOIInfo]: Offset table: " << offset << endl;

   if ( m_poiInfoData.get() ) {
      // load directly from poi info data in memory
      auto_ptr<POIInfo> poiInfo( new POIInfo() );
      DataBuffer buff( m_poiInfoData->getBufferAddress() + offset,
                       m_poiInfoData->getBufferSize() - offset );

      poiInfo->load( buff );

      return poiInfo;
   } else {

      // add start offset in file
      offset += m_poiInfoStartOffset;
   
      mc2dbg8 << "[GenericMap::getPOIInfo]: offset = " << offset << "(" 
              << MC2HEX( offset ) << ")" << endl;

      // determine path + filename
      MC2String fullFilename( m_filename );
      if ( m_pathname == NULL || strlen( m_pathname ) == 0 ) {
         fullFilename = DataBufferCreator::getMapOrIdxPath( m_filename );
      }

      mc2dbg8 << "[GenericMap::getPOIInfo]: Loading poi data from file: " 
              << fullFilename << endl;
      
      // load poi data from file
      auto_ptr<POIInfo> poiInfo( new POIInfo() );
      poiInfo->load( fullFilename, offset );

      return poiInfo;
   }


}

map<uint32, POIInfo*> GenericMap::getPOIInfos( vector<Item*>::const_iterator begin,
                                               vector<Item*>::const_iterator end,
                                               const set<uint32>& keyTypes ) const {
   mc2dbg << "GenericMap::getPOIInfos" << endl;
   map<uint32, POIInfo*> result;

   // for each item, determine if its a poi and then get the poi info
   for ( vector<Item*>::const_iterator itemIt = begin;
         itemIt != end; ++itemIt ) {
      PointOfInterestItem* poi = item_cast<PointOfInterestItem*>( *itemIt );

      if ( poi == NULL ) {
         continue;
      }

      // get info from file
      auto_ptr<POIInfo> info( getPOIInfo( poi ) );
      if ( ! info.get() ) { 
         continue;
      }

      // now lets clone the data the client actually are interested in
      // which is in the set keyTypes.
      auto_ptr<POIInfo> filteredInfo( new POIInfo( *info, keyTypes ) );
      if ( result.insert( make_pair( poi->getWASPID(), 
                                     filteredInfo.get() ) ).second ){
         // release once we know it was successfully inserter into the map
         filteredInfo.release();
      }
   }

   return result;
}

void GenericMap::copyPOIInfoData( const DataBuffer& dataBuffer ) {
   m_poiInfoStartOffset = dataBuffer.getCurrentOffset();
   AlignUtility::alignLong( m_poiInfoStartOffset );

   mc2dbg << "[GenericMap] m_poiInfoStartOffset = " << m_poiInfoStartOffset 
          << endl;

   // Have to use the second argument ( "false" ) for copy constructor here, 
   // Otherwise it doesn't call the correct constructor. gcc bug ?
   m_poiInfoData.
      reset( new DataBuffer( DataBuffer( dataBuffer.getBufferAddress() + 
                                         m_poiInfoStartOffset, 
                                         dataBuffer.getBufferSize() -
                                         m_poiInfoStartOffset ), false ) );
}

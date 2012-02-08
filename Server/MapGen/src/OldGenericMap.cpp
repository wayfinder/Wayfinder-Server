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
#include "OldGenericMap.h"

#include "NonStdStl.h"
#include "OldMapHashTable.h"
#include "GfxUtility.h"

#include "BinarySearchTree.h"
#include "ItemTypes.h"

#include "OldItem.h"
#include "OldAirportItem.h"
#include "OldAircraftRoadItem.h"
#include "OldBuildingItem.h"
#include "OldBuiltUpAreaItem.h"
#include "OldBusRouteItem.h"
#include "OldCategoryItem.h"
#include "OldCityPartItem.h"
#include "OldFerryItem.h"
#include "OldForestItem.h"
#include "OldGroupItem.h"
#include "OldIndividualBuildingItem.h"
#include "OldIslandItem.h"
#include "OldMilitaryBaseItem.h"
#include "OldMunicipalItem.h"
#include "OldNode.h"
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
#include "OldZipCodeItem.h"
#include "OldZipAreaItem.h"
#include "OldCartographicItem.h"

#include "StringUtility.h"
#include "StringSearchUtility.h"

#include "OldOverviewMap.h"

#include "OldCountryOverviewMap.h"


#include "GfxDataFull.h"

#include "GfxUtility.h"
#include "GMSGfxData.h"

#include "ItemIdentifier.h"
#include "OldExternalConnections.h"

#include "DataBuffer.h"

#include "AllocatorTemplate.h"

#include "STLStringUtility.h"
#include "Stack.h"
#include "UserRightsItemTable.h"
#include "UserRightsMapInfo.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "AlignUtility.h"
#include "Math.h"

#include "NodeBits.h"

// Define this to use the diff between
// process size before and after loading as the map size
#undef  USE_PROCESSSIZE_FOR_MAP_SIZE

#ifdef USE_PROCESSSIZE_FOR_MAP_SIZE
#include "SysUtility.h"
#endif

#include <fstream>
#include <iterator>
#include <utime.h>

namespace {

   /**
    *    Comparator for the table of good coordinates for
    *    items.
    */
   class AdminAreaCentreCmp {
   public:
      /// For is_sorted
      bool operator()(const OldGenericMap::adminAreaCentre_t& a,
                      const OldGenericMap::adminAreaCentre_t& b) const {
         return a.itemID < b.itemID;
      }
      /// For lower_bound
      bool operator()(const OldGenericMap::adminAreaCentre_t& a,
                      uint32 itemID) const {
         return a.itemID < itemID;
      }
   };
   
}

OldGenericMap*
OldGenericMap::createMap(uint32 id, const char* path)
{
#ifdef USE_PROCESSSIZE_FOR_MAP_SIZE
   uint32 sizeBefore = SysUtility::getProcessTotalSize();
#endif
   // Variables to return/get the map status
   OldGenericMap* theNewMap = NULL;

   // Create a map with correct type
   if (MapBits::isUnderviewMap(id)) {
      theNewMap = new OldMap(id, path);
   } else if (MapBits::isOverviewMap(id)) {
      theNewMap = new OldOverviewMap(id, path);
   } else {
      theNewMap = new OldCountryOverviewMap(id, path);
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

OldGenericMap*
OldGenericMap::createMap(const char* mcmName)
{
   // Extract the ID and path from mcmName
   char path[256];
   path[0] = '\0';
   uint32 mapID;
   bool ok = false;

   // Find the last '/'
   char* slashPtr = StringUtility::strrchr(mcmName, '/');
   if (slashPtr == NULL) {
      // No slash, se if id + ".mcm" is provided
      if (sscanf(mcmName, "%x.mcm", &mapID) == 1) {
         strcat(path, "./");
         ok = true;
      }
   } else {
      // Got a slash, get mapID
      if (sscanf(slashPtr, "/%x.mcm", &mapID) == 1) {
         // Got mapID, get path
         int n = slashPtr-mcmName+1;
         if (strncpy(path, mcmName, n) != NULL) {
            path[n] = '\0';
            ok = true;
         }
      }
   }
   
   mc2dbg2 << "   mcmName=" << mcmName << endl;
   mc2dbg2 << "   path=" << path << ", mapID=" << mapID << endl;

   if (ok) {
      // Create from other createMap-method
      return createMap(mapID, path);
   } else {
      return NULL;
   }
}


OldGenericMap::OldGenericMap():
   m_idTranslationTable( 1 ) // use old load/save
{
   initEmptyMap(MAX_UINT32);
}

OldGenericMap::OldGenericMap(uint32 id):
   m_idTranslationTable( 1 ) // use old load/save
{
   initEmptyMap(id);
}

uint32
OldGenericMap::getApproxMapSize() const
{
   return m_approxMapSize;
}

void
OldGenericMap::initEmptyMap(uint32 id) 
{
   m_approxMapSize = 0;
   m_mapID = id;
   m_hashTable = NULL;
   //m_segmentsOnTheBoundry = NULL; don't init with NULL
   m_segmentsOnTheBoundry = new OldBoundrySegmentsVector();
   m_itemNames = NULL;
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      m_itemsZoom[i] = NULL;
      m_itemsZoomSize[i] = 0;
      m_itemsZoomAllocated[i] = 0; 
   }
   m_gfxData = NULL;


   // Set all allocators to NULL to be able to delete them
   m_streetSegmentItemAllocator = NULL;
   m_streetItemAllocator = NULL;
   m_municipalItemAllocator = NULL;
   m_cityPartItemAllocator = NULL;
   m_waterItemAllocator = NULL;
   m_parkItemAllocator = NULL;
   m_forestItemAllocator = NULL;
   m_buildingItemAllocator = NULL;
   m_railwayItemAllocator = NULL;
   m_islandItemAllocator = NULL;
   m_zipCodeItemAllocator = NULL;
   m_zipAreaItemAllocator = NULL;
   m_pointOfInterestItemAllocator = NULL;
   m_categoryItemAllocator = NULL;
   m_builtUpAreaItemAllocator = NULL;
   m_busRouteItemAllocator = NULL;
   m_airportItemAllocator = NULL;
   m_aircraftRoadItemAllocator = NULL;
   m_pedestrianAreaItemAllocator = NULL;
   m_militaryBaseItemAllocator = NULL;
   m_individualBuildingItemAllocator = NULL;
   m_subwayLineItemAllocator = NULL;
   m_nullItemAllocator = NULL;
   m_ferryItemAllocator = NULL;
   m_gfxDataAllocator = NULL;
   m_gfxDataSingleSmallPolyAllocator = NULL;
   m_gfxDataSingleLineAllocator = NULL;
   m_gfxDataSinglePointAllocator = NULL;
   m_gfxDataMultiplePointsAllocator = NULL;
   m_nodeAllocator = NULL;
   m_connectionAllocator = NULL;
   m_simpleItemAllocator = NULL;
   m_cartographicItemAllocator = NULL;

   /// Create new UserRightsItemTable. It is empty
   m_userRightsTable = new UserRightsItemTable( ~MapRights() );
   /// Default argument to functions taking UserRightsMapInfo
   m_allRight = new UserRightsMapInfo( m_mapID, ~MapRights() );

   // The adminAreaCentre table
   m_adminAreaCentres = NULL;
   m_nbrAdminAreaCentres = 0;

}

void
OldGenericMap::createAllocators()
{
   m_streetSegmentItemAllocator = new MC2Allocator<OldStreetSegmentItem>(0);
   m_streetItemAllocator = new MC2Allocator<OldStreetItem>(0);
   m_municipalItemAllocator = new MC2Allocator<OldMunicipalItem>(0);
   m_cityPartItemAllocator = new MC2Allocator<OldCityPartItem>(0);
   m_waterItemAllocator = new MC2Allocator<OldWaterItem>(0);
   m_parkItemAllocator = new MC2Allocator<OldParkItem>(0);
   m_forestItemAllocator = new MC2Allocator<OldForestItem>(0);
   m_buildingItemAllocator = new MC2Allocator<OldBuildingItem>(0);
   m_railwayItemAllocator = new MC2Allocator<OldRailwayItem>(0);
   m_islandItemAllocator = new MC2Allocator<OldIslandItem>(0);
   m_zipCodeItemAllocator = new MC2Allocator<OldZipCodeItem>(0);
   m_zipAreaItemAllocator = new MC2Allocator<OldZipAreaItem>(0);
   m_pointOfInterestItemAllocator = new MC2Allocator<OldPointOfInterestItem> (0);
   m_categoryItemAllocator = new MC2Allocator<OldCategoryItem>(0);
   m_builtUpAreaItemAllocator = new MC2Allocator<OldBuiltUpAreaItem> (0);
   m_busRouteItemAllocator = new MC2Allocator<OldBusRouteItem>(0);
   m_airportItemAllocator = new MC2Allocator<OldAirportItem>(0);
   m_aircraftRoadItemAllocator = new MC2Allocator<OldAircraftRoadItem> (0);
   m_pedestrianAreaItemAllocator = new MC2Allocator<OldPedestrianAreaItem> (0);
   m_militaryBaseItemAllocator = new MC2Allocator<OldMilitaryBaseItem> (0);
   m_individualBuildingItemAllocator = 
      new MC2Allocator<OldIndividualBuildingItem> (0);
   m_subwayLineItemAllocator = new MC2Allocator<OldSubwayLineItem> (0);
   m_nullItemAllocator = new MC2Allocator<OldNullItem> (0);
   m_ferryItemAllocator = new MC2Allocator<OldFerryItem> (0);
   m_gfxDataAllocator = new MC2Allocator<GMSGfxData>(0);
   m_gfxDataSingleSmallPolyAllocator =
      new MC2Allocator<GMSGfxData>(0);
   m_gfxDataSingleLineAllocator = new MC2Allocator<GMSGfxData>(0);
   m_gfxDataSinglePointAllocator = new MC2Allocator<GMSGfxData>(0);
   m_gfxDataMultiplePointsAllocator = new MC2Allocator<GMSGfxData>(0);
   m_nodeAllocator = new MC2Allocator<OldNode>(0);
   m_connectionAllocator = new MC2Allocator<OldConnection>(0);
   m_connectionArrayAllocator = new MC2ArrayAllocator<OldConnection*>(0);
   m_connectionArrayAllocator->setWarnSingleAlloc( false );
   m_simpleItemAllocator = new MC2Allocator<OldItem>(0);
   m_cartographicItemAllocator = new MC2Allocator<OldCartographicItem>(0);


   // this method is re-implemented in GMSMap.
}

MC2ArrayAllocator<OldConnection*>& 
OldGenericMap::getConnectionAllocator() {
   return *m_connectionArrayAllocator;
   //return *static_cast< MC2ArrayAllocator<OldConnection*> *> 
   //         (m_connectionArrayAllocator);
}

void
OldGenericMap::initNonItemAllocators( uint32 nbrGfxDatas, 
                                   uint32 nbrGfxDataSingleSmallPoly,
                                   uint32 nbrGfxDataSingleLine, 
                                   uint32 nbrGfxDataSinglePoint,
                                   uint32 nbrGfxDataMultiplePoints, 
                                   uint32 nbrNodes, 
                                   uint32 nbrConnections ) 
{
   // In GMSMap, all GfxDatas are GMSGfxDatas. 
   nbrGfxDatas += nbrGfxDataSingleSmallPoly + 
                  nbrGfxDataSingleLine +
                  nbrGfxDataSinglePoint + 
                  nbrGfxDataMultiplePoints;
   m_gfxDataAllocator->reallocate(nbrGfxDatas);

   // Ordinary
   m_nodeAllocator->reallocate(nbrNodes);
   m_connectionAllocator->reallocate(nbrConnections);
   m_connectionArrayAllocator->reallocate(nbrConnections);
}


OldGenericMap::OldGenericMap(uint32 mapID, const char *path)
   : OldGenericMapHeader(mapID, path),
     m_idTranslationTable( 1 ) // use Old load/save
{
   mc2dbg4 << "OldGenericMap " << mapID << " to be created" << endl;

   initEmptyMap(mapID);

   // Set the filename of the map
   setFilename(path);

}

void
OldGenericMap::createOldBoundrySegmentsVector()
{
   if ( m_segmentsOnTheBoundry == NULL ) {
      m_segmentsOnTheBoundry = new OldBoundrySegmentsVector();
   }
}


OldGenericMap::~OldGenericMap()
{
   mc2dbg2 << "~OldGenericMap enter" << endl;
   uint32 startTime = TimeUtility::getCurrentMicroTime();

   delete m_itemNames;
   DEBUG_DEL(mc2dbg << "~OldGenericMap itemNames destr." << endl) ;

   // Do NOT delete the items since they are created inside an Allocator
   if (m_itemsZoomSize != NULL) {
      for (uint32 i=0; i < NUMBER_GFX_ZOOMLEVELS; i++) {
      
         // WHY DEBUG2 ???
         
         DEBUG2(
                mc2log << "Handing over objects to allocators" << endl;
            // Make sure that all items are in the allocators
            for (uint32 j=0; j < m_itemsZoomSize[i]; j++) {
               //delete m_itemsZoom[i][j];
               OldItem* item = m_itemsZoom[i][j];
               if (item != NULL) {
                  // Hand over object to correct allocator to make sure the 
                  // object is deleted.
                  switch (item->getItemType()) {
                     case ItemTypes::streetSegmentItem : {
                        OldStreetSegmentItem* ssi = 
                           static_cast<OldStreetSegmentItem*>(item);
                        static_cast< MC2Allocator<OldStreetSegmentItem>* >
                           (m_streetSegmentItemAllocator)->handOverObject(ssi);
                        for (int k=0; k<2; ++k) {
                           static_cast< MC2Allocator<OldNode>* >
                              (m_nodeAllocator)->handOverObject(
                                 ssi->getNode(k));
                           for (uint32 l=0; 
                                l<ssi->getNode(k)->getNbrConnections(); 
                                ++l) {
                              static_cast< MC2Allocator<OldConnection>* >
                                 (m_connectionAllocator)->handOverObject(ssi->
                                    getNode(k)->getEntryConnection(l)); 
                           }
                        }
                        } break;
                     case ItemTypes::streetItem :
                        static_cast< MC2Allocator<OldStreetItem>* >
                           (m_streetItemAllocator)->handOverObject(
                              static_cast<OldStreetItem*>(item));
                        break;
                     case ItemTypes::municipalItem :
                        static_cast< MC2Allocator<OldMunicipalItem>* >
                           (m_municipalItemAllocator)->handOverObject(
                              static_cast<OldMunicipalItem*>(item));
                        break;
                     case ItemTypes::cityPartItem :
                        static_cast< MC2Allocator<OldCityPartItem>* >
                           (m_cityPartItemAllocator)->handOverObject(
                              static_cast<OldCityPartItem*>(item));
                        break;
                     case ItemTypes::waterItem :
                        static_cast< MC2Allocator<OldWaterItem>* >
                           (m_waterItemAllocator)->handOverObject(
                              static_cast<OldWaterItem*>(item));
                        break;
                     case ItemTypes::parkItem :
                        static_cast< MC2Allocator<OldParkItem>* >
                           (m_parkItemAllocator)->handOverObject(
                              static_cast<OldParkItem*>(item));
                        break;
                     case ItemTypes::forestItem :
                        static_cast< MC2Allocator<OldForestItem>* >
                           (m_forestItemAllocator)->handOverObject(
                              static_cast<OldForestItem*>(item));
                        break;
                     case ItemTypes::buildingItem :
                        static_cast< MC2Allocator<OldBuildingItem>* >
                           (m_buildingItemAllocator)->handOverObject(
                              static_cast<OldBuildingItem*>(item));
                        break;
                     case ItemTypes::railwayItem :
                        static_cast< MC2Allocator<OldRailwayItem>* >
                           (m_railwayItemAllocator)->handOverObject(
                              static_cast<OldRailwayItem*>(item));
                        break;
                     case ItemTypes::islandItem :
                        static_cast< MC2Allocator<OldIslandItem>* >
                           (m_islandItemAllocator)->handOverObject(
                              static_cast<OldIslandItem*>(item));
                        break;
                     case ItemTypes::zipCodeItem :
                        static_cast< MC2Allocator<OldZipCodeItem>* >
                           (m_zipCodeItemAllocator)->handOverObject(
                              static_cast<OldZipCodeItem*>(item));
                        break;
                     case ItemTypes::zipAreaItem :
                        static_cast< MC2Allocator<OldZipAreaItem>* >
                           (m_zipAreaItemAllocator)->handOverObject(
                              static_cast<OldZipAreaItem*>(item));
                        break;
                     case ItemTypes::pointOfInterestItem :
                        static_cast< MC2Allocator<OldPointOfInterestItem>* >
                           (m_pointOfInterestItemAllocator)->handOverObject(
                              static_cast<OldPointOfInterestItem*>(item));
                        break;
                     case ItemTypes::categoryItem :
                        static_cast< MC2Allocator<OldCategoryItem>* >
                           (m_categoryItemAllocator)->handOverObject(
                              static_cast<OldCategoryItem*>(item));
                        break;
                     case ItemTypes::builtUpAreaItem :
                        static_cast< MC2Allocator<OldBuiltUpAreaItem>* >
                           (m_builtUpAreaItemAllocator)->handOverObject(
                              static_cast<OldBuiltUpAreaItem*>(item));
                        break;
                     case ItemTypes::busRouteItem :
                        static_cast< MC2Allocator<OldBusRouteItem>* >
                           (m_busRouteItemAllocator)->handOverObject(
                              static_cast<OldBusRouteItem*>(item));
                        break;
                     case ItemTypes::airportItem :
                        static_cast< MC2Allocator<OldAirportItem>* >
                           (m_airportItemAllocator)->handOverObject(
                              static_cast<OldAirportItem*>(item));
                        break;
                     case ItemTypes::aircraftRoadItem :
                        static_cast< MC2Allocator<OldAircraftRoadItem>* >
                           (m_aircraftRoadItemAllocator)->handOverObject(
                              static_cast<OldAircraftRoadItem*>(item));
                        break;
                     case ItemTypes::pedestrianAreaItem :
                        static_cast< MC2Allocator<OldPedestrianAreaItem>* >
                           (m_pedestrianAreaItemAllocator)->handOverObject(
                              static_cast<OldPedestrianAreaItem*>(item));
                        break;
                     case ItemTypes::militaryBaseItem :
                        static_cast< MC2Allocator<OldMilitaryBaseItem>* >
                           (m_militaryBaseItemAllocator)->handOverObject(
                              static_cast<OldMilitaryBaseItem*>(item));
                        break;
                     case ItemTypes::individualBuildingItem :
                        static_cast< MC2Allocator<OldIndividualBuildingItem>* >
                           (m_individualBuildingItemAllocator)->handOverObject(
                              static_cast<OldIndividualBuildingItem*>(item));
                        break;
                     case ItemTypes::subwayLineItem :
                        static_cast< MC2Allocator<OldSubwayLineItem>* >
                           (m_subwayLineItemAllocator)->handOverObject(
                              static_cast<OldSubwayLineItem*>(item));
                        break;
                     case ItemTypes::nullItem :
                        static_cast< MC2Allocator<OldNullItem>* >
                           (m_nullItemAllocator)->handOverObject(
                              static_cast<OldNullItem*>(item));
                        break;
                     case ItemTypes::ferryItem :
                        static_cast< MC2Allocator<OldFerryItem>* >
                           (m_ferryItemAllocator)->handOverObject(
                              static_cast<OldFerryItem*>(item));
                        break;
                     case ItemTypes::borderItem :
                        static_cast< MC2Allocator<OldItem>* >
                           (m_simpleItemAllocator)->handOverObject( item );
                        break;
                     case ItemTypes::cartographicItem :
                        static_cast< MC2Allocator<OldItem>* >
                           (m_cartographicItemAllocator)->handOverObject(item);
                        break;
                     case ItemTypes::routeableItem :
                     case ItemTypes::numberOfItemTypes :
                        break;
                  } // End switch

                  /*
                  if (item->getGfxData() != NULL) {
                     static_cast< MC2Allocator<GfxData>* >
                        (m_gfxDataAllocator)->handOverObject(
                           item->getGfxData());
                  }
                  */
               }
            }
            );
         delete [] m_itemsZoom[i];
      }
   }
   DEBUG_DEL(mc2dbg << "OldGenericMap::~OldGenericMap All items destr." << endl);

   // Delete all the allocators...
   mc2dbg4 << "~OldGenericMap before allocator dtor" << endl;
   delete m_streetSegmentItemAllocator;
   delete m_streetItemAllocator;
   delete m_municipalItemAllocator;
   delete m_cityPartItemAllocator;
   delete m_waterItemAllocator;
   delete m_parkItemAllocator;
   delete m_forestItemAllocator;
   delete m_buildingItemAllocator;
   delete m_railwayItemAllocator;
   delete m_islandItemAllocator;
   delete m_zipCodeItemAllocator;
   delete m_zipAreaItemAllocator;
   delete m_pointOfInterestItemAllocator;
   delete m_categoryItemAllocator;
   delete m_builtUpAreaItemAllocator;
   delete m_busRouteItemAllocator;
   delete m_airportItemAllocator;
   delete m_aircraftRoadItemAllocator;
   delete m_pedestrianAreaItemAllocator;
   delete m_militaryBaseItemAllocator;
   delete m_individualBuildingItemAllocator;
   delete m_subwayLineItemAllocator;
   if (m_nullItemAllocator != NULL) {
      mc2dbg4 << "OldNullItem-allocator contains " 
           << m_nullItemAllocator->getBlockSize() << " items" << endl;
   }
   delete m_nullItemAllocator;
   delete m_ferryItemAllocator;
   delete m_gfxDataAllocator;
   delete m_gfxDataSingleSmallPolyAllocator;
   delete m_gfxDataSingleLineAllocator;
   // These were commented out before. Don't know why.
   delete m_gfxDataSinglePointAllocator;
   delete m_gfxDataMultiplePointsAllocator;
   delete m_nodeAllocator;
   delete m_connectionAllocator;
   delete m_connectionArrayAllocator;
   delete m_simpleItemAllocator;
   delete m_cartographicItemAllocator;
   mc2dbg4 << "~OldGenericMap after allocator dtor" << endl;

   delete m_segmentsOnTheBoundry;
   DEBUG_DEL(mc2dbg << "~OldGenericMap segmentsOnTheBoundry destr." << endl) ;
   mc2dbg4 <<"~OldGenericMap after segments on boundry dtor"<< endl;

   //delete m_gfxData;
   // Created in the allocators!!!
   DEBUG_DEL(mc2dbg << "OldGenericMap::~OldGenericMap gfxData destr." << endl) ;
   mc2dbg4 <<"~OldGenericMap after gfxData dtor"<< endl;

   delete m_hashTable;
   DEBUG_DEL(mc2dbg << "OldGenericMap::~OldGenericMap hashTable destr." << endl) ;
   mc2dbg4 <<"~OldGenericMap after hashtable dtor"<< endl;

   delete m_nativeLanguages;
   delete m_currency;
   delete m_userRightsTable;
   delete m_allRight;

   delete[] m_adminAreaCentres;

   mc2dbg4 << "~OldGenericMap exit" << endl;
   uint32 endTime = TimeUtility::getCurrentMicroTime();
   mc2dbg4 << "OldMap " << getMapID() << " deleted in " 
           << (endTime-startTime)/1000.0 << " ms" << endl;
}

uint32 
OldGenericMap::addItem(OldItem *item, uint32 zoomLevel)
{
   uint32 extraSpace = 128;
   if ((zoomLevel == uint32(ItemTypes::pointOfInterestItem)))
      extraSpace = 16384;
    
   if(item != NULL) { 
      if(m_itemsZoomAllocated[zoomLevel] <= m_itemsZoomSize[zoomLevel]) {
         // We have to allocate more memory
         OldItem** tempVector = new OldItem*[m_itemsZoomAllocated[zoomLevel] 
                              + extraSpace];
         if (tempVector == NULL)
            return MAX_UINT32;
         mc2dbg2 << "OldGenericMap::addItem Allocated " << extraSpace 
                 << " more entries for " << "zoomLevel = " 
                 << zoomLevel << endl;
         m_itemsZoomAllocated[zoomLevel] += extraSpace;
         for(uint32 i = 0; i < m_itemsZoomSize[zoomLevel]; i++)
            tempVector[i] = m_itemsZoom[zoomLevel][i];

         /* Replaced with the two lines below
         OldItem** swapVector = tempVector;
         tempVector = m_itemsZoom[zoomLevel];           
         m_itemsZoom[zoomLevel] = swapVector;
         delete [] tempVector;
         */

         delete [] m_itemsZoom[zoomLevel];
         m_itemsZoom[zoomLevel] = tempVector;

         mc2dbg2 << "OldGenericMap::addItem Copied the old vector " << endl; 
      }
      // Give the item an ID in the map.
      
      uint32 posInZoomLevel = m_itemsZoomSize[zoomLevel];
      
      // Make sure municipal items are stored in the beginning of 
      // zoomlevel 0, since they are only represented by a byte in 
      // location.
      if ( ( zoomLevel == 0 ) && 
           ( item->getItemType() == ItemTypes::municipalItem ) ) {
         // Try to find the first NULL / nullItem and recycle that
         // position in the zoomlevel.
         uint32 i = 0;
         bool cont = true;
         while ( cont && 
                 ( i < m_itemsZoomSize[ zoomLevel ] ) ) { 
            if ( ( m_itemsZoom[zoomLevel][ i ] == NULL ) ||
                 ( m_itemsZoom[zoomLevel][ i ]->getItemType() == 
                   ItemTypes::nullItem ) ) {
               // Found a NULL item or nullitem. Recycle!
               //delete m_itemsZoom[ zoomLevel ][ i ]; don't delete since
               // this item is probably in an allocator. 
               cont = false;
               posInZoomLevel = i;
            } else {
               ++i;
            }
         }
      }
      
      // Calculate new id.
      uint32 newID = zoomLevel << 27 | posInZoomLevel;
      item->setID( newID );
      
      // Add item.
      m_itemsZoom[ zoomLevel ][ posInZoomLevel & 0x07ffffff ] = item;
      
      // Increment nbr of items in zoomlevel if necessary.
      if ( posInZoomLevel == m_itemsZoomSize[zoomLevel] ) {
         ++m_itemsZoomSize[zoomLevel];
      }
      
      // Update node id's if adding a routeable item.
      if ((item->getItemType() == ItemTypes::streetSegmentItem) ||
          (item->getItemType() == ItemTypes::busRouteItem) ||
          (item->getItemType() == ItemTypes::ferryItem)) {
         OldRouteableItem* ri = static_cast<OldRouteableItem*> (item);
         ri->getNode(0)->setNodeID(ri->getID() & 0x7FFFFFFF);
         ri->getNode(1)->setNodeID(ri->getID() | 0x80000000);
      }
      return(newID);
   }
   else
      return(MAX_UINT32);
}


bool
OldGenericMap::removeItems(const set<uint32>& itemIDs){
   bool result = true;
   for (set<uint32>::const_iterator itemIt = itemIDs.begin(); 
        itemIt != itemIDs.end(); ++itemIt ){
      // Result is set to false if one removal fails.
      result = result && removeItem(*itemIt,
                                    false, // Not updating hash table
                                    false  // Not updating admin area centers
                                    );
   }
   removeAdminCenters(itemIDs);
   buildHashTable();
   return result;
}

bool 
OldGenericMap::removeItem(uint32 localID, 
                          bool updateHashTable /* = true */,
                          bool updateAdminCenters /* = true */,
                          bool unusedUkZips /* = false */)
{
   mc2dbg4 << "removeItem(" << localID << ")" << endl;
   OldItem* tmpItem = itemLookup(localID);
   if (tmpItem == NULL) {
      mc2dbg << "Item to remove " << localID << " was not found." << endl;
      return (false);
   }
   mc2dbg8 << "Will remove item with ID:" << localID << endl;
      
   
   switch (tmpItem->getItemType()) {
      
      // Do NOT delete anything, since it is allocated by allocators!
      case (ItemTypes::streetSegmentItem) :
      case (ItemTypes::ferryItem) : {
         // Delete the connections that leads from this item.
         // Also remove any connecting virtual segments present in the
         // boundry segments vector.
         vector<uint32> bsToRemove;

         // Check the connections to this routeable item,
         // and remove the opposing ones
         OldRouteableItem* ri = (OldRouteableItem*) tmpItem;
         for (int k=0; k<2; k++) {
            OldNode* node = ri->getNode(k);
            for (uint32 j=0; j<node->getNbrConnections(); j++) {
               uint32 id = node->getEntryConnection(j)->
                                 getConnectFromNode();
               OldRouteableItem* tmpRi = dynamic_cast<OldRouteableItem*>
                                           (itemLookup(id));
               if (tmpRi != NULL) {
                  if ( tmpItem->getItemType() != tmpRi->getItemType() ) {
                     mc2dbg << " conns between different item types "
                            << localID << " <-> " << id << endl;
                  }
                  tmpRi->deleteConnectionsFrom(localID, *this);
                  tmpRi->deleteConnectionsFrom(localID | 0x80000000, *this);
                  
                  // If tmpRi is a boundry segment it belongs to
                  // the routeable and should also be removed
                  uint32 tmpriId = tmpRi->getID();
                  OldBoundrySegment* bs = m_segmentsOnTheBoundry->
                        getBoundrySegment(tmpriId);
                  if (bs != NULL) {
                     bsToRemove.push_back(tmpriId);
                  }

               }
            }
         }

         // To make absolutely sure, check all nodes close to this one.
         const int mc2ScaleRadius = 1000; // \aprox 10 m
         m_hashTable->clearAllowedItemTypes();
         bool killResVect = false;
         uint32 nodeID;
         for (uint32 i=0; i<2; i++) {
            if (i == 1)
               nodeID = localID | 0x80000000;
            else
               nodeID = localID;
            int lat, lon;
            getNodeCoordinates(nodeID, lat, lon);
            Vector* resVect = m_hashTable->getAllWithinRadius(
                                                lon, lat, 
                                                mc2ScaleRadius, 
                                                killResVect);
            for (uint32 j=0; j<resVect->getSize(); j++) {
               OldRouteableItem* curItem = dynamic_cast<OldRouteableItem*>
                                 (itemLookup(resVect->getElementAt(j)));
               if ((curItem != NULL) && (curItem != tmpItem)) {
                  curItem->deleteConnectionsFrom(nodeID, *this);
               }
            }

            if (killResVect)
               delete resVect;
         }

         // Remove any boundry segments that were connected to this routeable
         if (bsToRemove.size() > 0) {
            mc2dbg8 << "For " << localID << " remove " << bsToRemove.size() 
                   << " connected boundry segments" << endl;
            for (vector<uint32>::iterator it = bsToRemove.begin();
                 it != bsToRemove.end(); it++) {
               if (removeItem(*it)) {
                  mc2dbg8 << "Removed boundry segment item " << *it << endl;
               }
            }
         }
         
         // If this segment is a boundry segment, remove it from the
         // segmentsOnBoundry vector
         OldBoundrySegment* bs =
            m_segmentsOnTheBoundry->getBoundrySegment(localID);
         if (bs != NULL) {
            // find the index in the bs vector
            uint32 i = 0;
            bool found = false;
            while (!found && (i < m_segmentsOnTheBoundry->getSize())) {
               OldBoundrySegment* tmpbs = (OldBoundrySegment*)
                  m_segmentsOnTheBoundry->getElementAt(i);
               if (tmpbs->getConnectRouteableItemID() == localID)
                  found = true;
               else
                  i++;
            }
            if (found) {
               m_segmentsOnTheBoundry->removeElementAt(i);
               mc2dbg8 << "Removed boundry segment " << localID 
                       << " from bs vector" << endl;
            } else {
               mc2log << warn << here << "Did not find boundry segment "
                      << localID << " in bs vector" << endl;
            }
         }

         // pois that refer to this ssi must be removed.
         if ( tmpItem->getItemType() == ItemTypes::streetSegmentItem ) {
            for (uint32 i = 0;
                 i < getNbrItemsWithZoom(ItemTypes::poiiZoomLevel); i++) {
               OldPointOfInterestItem* poi =
                  static_cast<OldPointOfInterestItem*>
                  (getItem(ItemTypes::poiiZoomLevel, i));
               if ((poi != NULL) && 
                   (poi->getStreetSegmentItemID() == localID)) {
                  // remove the poi, no need to update hash table.
                  removeItem(poi->getID(), false);
               }
            }
         }


         // Remove lane and sign post information
         for (uint32 n=0; n<2; n++){
            OldNode* node = ri->getNode(n);
            // Remove lane info stored by node ID.
            ItemMap< vector<GMSLane> >::iterator rmIt = 
               m_nodeLaneVectors.find(node->getNodeID());
            if (rmIt != m_nodeLaneVectors.end()){
               m_nodeLaneVectors.erase(rmIt);
            }
            // Remove connecting lane and sing post info with this node as to 
            // node.
            for (uint32 c=0; c<node->getNbrConnections(); c++){
               OldConnection* conn = node->getEntryConnection(c);
               uint32 fromNodeID = conn->getConnectFromNode();
               // Lane info
               m_connectingLanesTable.
                  removeConnectingLanes(fromNodeID, 
                                        node->getNodeID() // toNodeID
                                        );
               // Sing post info
               m_signPostTable.removeSignPosts(fromNodeID,
                                               node->getNodeID() // toNodeID
                                               );

            }
            // Remove connecting lane info with this node as from node.
            Vector toNodeIDs;
            getConnectToNodeIDs(node->getNodeID(), // fromNodeID
                                &toNodeIDs);
            for (uint32 c=0; c<toNodeIDs.size(); c++){
               // Lane info
               m_connectingLanesTable.
                  removeConnectingLanes(node->getNodeID(), // fromNodeID 
                                        toNodeIDs[c] // toNodeID
                                        );
               // Sing post info
                m_signPostTable.removeSignPosts(node->getNodeID(), //fromNodeID
                                                toNodeIDs[c]       //toNodeID
                                                );
            }
         }  // End of remove lane information    
         
      }
      break;   // OldStreetSegmentItem

      case (ItemTypes::municipalItem) :
      case (ItemTypes::builtUpAreaItem) :
      case (ItemTypes::zipCodeItem) : {
         // Remove this location from the items in the map.
         if ( unusedUkZips && 
              (tmpItem->getItemType() == ItemTypes::zipCodeItem) ){
            // Not removing as group in this case.
         }
         else {
            mc2dbg4 << "Removes item with ID: " << localID << endl;
            for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
               for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
                  OldItem* item = getItem(z, i);
                  if ((item != NULL) && (item->memberOfGroup(localID))) {
                     mc2dbg4 << "   Removes item as group from: " 
                             << item->getID() << endl;           
                     item->removeGroupWithID(localID);
                  }
               }
            }
         }
      }
      break;   // municipalItem and builtUpAreaItem
      

      case (ItemTypes::streetItem) :
      case (ItemTypes::categoryItem) : {
         // Remove the references to this group
         mc2dbg4 << "Removing the references to this group (" 
              << localID << ")" << endl;
         OldGroupItem* groupItem = (OldGroupItem*) tmpItem;
         for (uint32 i=0; i<groupItem->getNbrItemsInGroup(); i++) {
            OldItem* item = itemLookup(groupItem->getItemNumber(i));
            mc2dbg4 << "   Removing in " << item->getID() << endl;
            item->removeGroupWithID(groupItem->getID());
         }
      }
      break;   // OldCategoryItem and OldStreetItem
      default : {
         // Nothing to do for the other itemtypes...
      }
   }

   // Remove the groups
   mc2dbg4 << "Member of " << (uint32) tmpItem->getNbrGroups() 
           << " groups." << endl;
   for (uint32 i=0; i<tmpItem->getNbrGroups(); i++) {
      uint32 id = tmpItem->getGroup(i);
      OldGroupItem* group = dynamic_cast<OldGroupItem*>(itemLookup(id));
      if (group != NULL) {
         mc2dbg4 << "Removing item with id " << localID << " in group " 
              << group->getID() << endl;
         group->removeItemWithID(localID);
      }
   }

   // "Remove" the item = replace it with a null-item.
   m_itemsZoom[(localID & 0x78000000) >> 27][localID & 0x07ffffff] = 
      new OldNullItem(localID);
   
   if ( updateHashTable ) {   
      // Re-build the hashtable
      buildHashTable();
   }


   // Remove the itemID from misc containers with items ids.
   // The same containers must be updated in 
   // - swapItems
   // - changeZoomLevel
   
   // remove from adminAreaCentres table
   if (updateAdminCenters){
      ItemTypes::itemType type = tmpItem->getItemType();
      if ( (type == ItemTypes::builtUpAreaItem) ||
           (type == ItemTypes::municipalItem) ||
           (type == ItemTypes::zipCodeItem) ){
         adminAreaCentreTable_t adminCentres = getAdminAreaCentres();
         adminAreaCentreTable_t::iterator it = adminCentres.find(localID);
         if ( it != adminCentres.end() ) {
            adminCentres.erase( it );
            setAminAreaCentres( adminCentres );
         }
      }
   }

   // remove item from item category table.
   ItemMap< set<uint16> >::iterator catIt = m_itemCategories.find(localID);
   if ( catIt != m_itemCategories.end() ){
      m_itemCategories.erase(catIt);
   }

   // remove item from index area order table
   ItemMap< uint32 >::iterator indexAreaIt = m_indexAreasOrder.find(localID);
   if ( indexAreaIt != m_indexAreasOrder.end() ){
      m_indexAreasOrder.erase(indexAreaIt);
   }

   // remove item from non searchable items table.
   ItemMap< bool >::iterator nonSearchIt = 
      m_nonSearchableItems.find(localID);
   if ( nonSearchIt != m_nonSearchableItems.end() ){
      m_nonSearchableItems.erase(nonSearchIt);
   }

   // remove item from road displayClass table
   ItemMap< uint32 >::iterator dispIt = m_roadDisplayClass.find(localID);
   if ( dispIt != m_roadDisplayClass.end() ){
      m_roadDisplayClass.erase(dispIt);
   }

   // remove item from area feature draw displayClass table
   dispIt = m_areaFeatureDrawDisplayClass.find(localID);
   if ( dispIt != m_areaFeatureDrawDisplayClass.end() ){
      m_areaFeatureDrawDisplayClass.erase(dispIt);
   }

   
   return true;
} // removeItem

void 
OldGenericMap::removeAdminCenters(const set<uint32>& itemIDs){
   // remove from adminAreaCentres table
   adminAreaCentreTable_t adminCentres = getAdminAreaCentres();
   
   set<uint32>::const_iterator itemIt = itemIDs.begin();
   while ( itemIt != itemIDs.end() ){
      adminAreaCentreTable_t::iterator admIt = 
         adminCentres.find(*itemIt);
      if ( admIt != adminCentres.end() ) {
         adminCentres.erase( admIt );
      }
      ++itemIt;
   }
   setAminAreaCentres(adminCentres);
   
} // removeAdminCenters


void 
OldGenericMap::addAdminCenters(adminAreaCentreTable_t& areaCenters){
   adminAreaCentreTable_t mapAreaCenters = getAdminAreaCentres();
   
   adminAreaCentreTable_t::const_iterator centerIt = areaCenters.begin();
   while ( centerIt != areaCenters.end() ){
      mapAreaCenters.insert(*centerIt);
      ++centerIt;
   }
   setAminAreaCentres(mapAreaCenters);
   
} // addAdminCenters



void
OldGenericMap::removeAllIdenticalCoordinates()
{
   uint32 n = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         OldItem* item = getItem(z, i);
         if ((item != NULL) && (item->getGfxData() != NULL)) {
            uint32 nbrBefore = item->getGfxData()->getTotalNbrCoordinates();
            GMSGfxData *gfxData = dynamic_cast<GMSGfxData*>( item->getGfxData() );
            MC2_ASSERT( gfxData );
            gfxData->removeIdenticalCoordinates();
            n += nbrBefore - item->getGfxData()->getTotalNbrCoordinates();
         }
      }
   }
   mc2log << info << "Totaly removed " << n << " identical coordinates "
          << "before save" << endl;
}


bool
OldGenericMap::internalLoad(DataBuffer& dataBuffer)
{
   if ( ! OldGenericMapHeader::internalLoad(dataBuffer) ) {
      return false;
   }
   
   // Seems like the size of the map in mem compared to disk ranges
   // from 1.7 to 2.5
   m_approxMapSize = uint32( dataBuffer.getBufferSize() * 2.3);

   mc2dbg << "OldGenericMap::internalLoad" << endl;

 
   if ( m_loadedVersion > 5 ){
      // Loading whith variable map body size.

      uint32 startOffset = dataBuffer.getCurrentOffset();
      DataBuffer tmpBuf( dataBuffer.getBufferAddress(),
                         dataBuffer.getBufferSize() );
      tmpBuf.readPastBytes(startOffset);
      uint32 mapBodySize = tmpBuf.readNextLong();
      mc2dbg << "OldGenericMap body size: " << mapBodySize << " read." 
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
         m_userRightsTable->load(tmpBuf);
      }

      // Load m_adminAreaCentres table (m_loadedVersion >= 7)
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         m_nbrAdminAreaCentres = tmpBuf.readNextLong();
         mc2dbg << "Loading m_adminAreaCentres table with "
                << m_nbrAdminAreaCentres << " records" << endl;
         if ( m_nbrAdminAreaCentres > 0 ) {
            m_adminAreaCentres = new adminAreaCentre_t[m_nbrAdminAreaCentres];
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
               is_sorted( m_adminAreaCentres,
                          m_adminAreaCentres + m_nbrAdminAreaCentres,
                          AdminAreaCentreCmp() ) );
         } else {
            m_adminAreaCentres = NULL;
         }
      }

      // Load m_itemOfficialCodes item map (m_loadedVersion >= 7)
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_itemOfficialCodes" << endl;
         m_itemOfficialCodes.load(tmpBuf);
         mc2dbg << "Loaded m_itemOfficialCodes with " 
                << m_itemOfficialCodes.size() << " items." << endl;
      }
      

      // Map version 8 below.

      // Load m_itemCategories
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_itemCategories" << endl;
         m_itemCategories.load(tmpBuf);
         mc2dbg << "Loaded m_itemCategories with " 
                << m_itemCategories.size() << " items." << endl;
      }
      
      // Load m_nodeLaneVectors
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_nodeLaneVectors" << endl;
         m_nodeLaneVectors.load(tmpBuf);
         mc2dbg << "Loaded m_nodeLaneVectors with " 
                << m_nodeLaneVectors.size() << " nodes." << endl;
      }

      // Load m_connectingLanesTable
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_connectingLanesTable" << endl;
         m_connectingLanesTable.load(tmpBuf);
         mc2dbg << "Loaded m_connectingLanesTable with " 
                << m_connectingLanesTable.size() << " posts." 
                << endl;
      }

      // Load m_signPostTable
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_signPostTable" << endl;
         m_signPostTable.load(tmpBuf);
         mc2dbg << "Loaded m_signPostTable with " 
                << m_signPostTable.size() << " posts." 
                << endl;
      }

      // Load m_indexAreasOrder
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_indexAreasOrder" << endl;
         m_indexAreasOrder.load(tmpBuf);
         mc2dbg << "Loaded m_indexAreasOrder with " 
                << m_indexAreasOrder.size() << " posts." 
                << endl;
      }
      

      // Load m_nonSearchableItems
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_nonSearchableItems" << endl;
         m_nonSearchableItems.load(tmpBuf);
         mc2dbg << "Loaded m_nonSearchableItems with " 
                << m_nonSearchableItems.size() << " posts." 
                << endl;
      }

      // Load m_roadDisplayClass
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_roadDisplayClass" << endl;
         m_roadDisplayClass.load(tmpBuf);
         mc2dbg << "Loaded m_roadDisplayClass with " 
                << m_roadDisplayClass.size() << " posts."
                << endl;
      }
      
      // Load m_areaFeatureDrawDisplayClass
      if ( tmpBuf.getCurrentOffset() < endOffset ) {
         mc2dbg << "Loading m_areaFeatureDrawDisplayClass" << endl;
         m_areaFeatureDrawDisplayClass.load(tmpBuf);
         mc2dbg << "Loaded m_areaFeatureDrawDisplayClass with " 
                << m_areaFeatureDrawDisplayClass.size() << " posts." << endl;
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


      dataBuffer.readPastBytes(mapBodySize);
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

bool
OldGenericMap::loadFirstBodyBlock(DataBuffer& dataBuffer)
{
   mc2dbg4 << "loadFirstBodyBlock enter" << endl;
   CLOCK_MAPLOAD(uint32 startTime = TimeUtility::getCurrentMicroTime());


   // The allocators
   uint32 nbrItemsPerType[OldGenericMapHeader::numberInitialItemTypeAllocators];
   for ( uint32 i=0;
         i < OldGenericMapHeader::numberInitialItemTypeAllocators; ++i) {
      nbrItemsPerType[i] = 0;
   }
   for ( uint32 t=0;
         t < OldGenericMapHeader::numberInitialItemTypeAllocators; ++t) {
      uint32 itemType = dataBuffer.readNextLong();
      nbrItemsPerType[ itemType ] = dataBuffer.readNextLong();
      mc2dbg << "Load: " << nbrItemsPerType[ itemType ] 
              << " items with type " << itemType << endl;
   }
   // Make sure we don't have any nullItems
   nbrItemsPerType[ uint32(ItemTypes::nullItem) ] = 0;
  
   // The "extra" allocators
   uint32 nbrGfxDatas = dataBuffer.readNextLong();
   uint32 nbrNodes = dataBuffer.readNextLong();
   uint32 nbrConnections = dataBuffer.readNextLong();
   uint32 nbrGfxDataSingleSmallPoly = 0;
   uint32 nbrGfxDataSingleLine = 0;
   uint32 nbrGfxDataSinglePoint = 0;
   uint32 nbrGfxDataMultiplePoints = 0;
   uint32 nbrSimpleItems = 0;
   uint32 nbrCartoItems = 0;
   // Check how many extra allocators there are left to read excluding the
   // already read ones.
   int nbrAllocatorsLeft = m_nbrAllocators - 
         OldGenericMapHeader::numberInitialItemTypeAllocators - 3;
   
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
      nbrCartoItems = dataBuffer.readNextLong();
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
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());


   // ***************************************************************
   //                                             Initiate allocators
   // ***************************************************************
   m_streetSegmentItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::streetSegmentItem)]);
   m_streetItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::streetItem)]);
   m_municipalItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::municipalItem)]);
   m_cityPartItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::cityPartItem)]);
   m_waterItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::waterItem)]);
   m_parkItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::parkItem)]);
   m_forestItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::forestItem)]);
   m_buildingItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::buildingItem)]);
   m_railwayItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::railwayItem)]);
   m_islandItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::islandItem)]);
   m_zipCodeItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::zipCodeItem)]);
   m_zipAreaItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::zipAreaItem)]);
   m_pointOfInterestItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::pointOfInterestItem)]);
   m_categoryItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::categoryItem)]);
   m_builtUpAreaItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::builtUpAreaItem)]);
   m_busRouteItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::busRouteItem)]);
   m_airportItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::airportItem)]);
   m_aircraftRoadItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::aircraftRoadItem)]);
   m_pedestrianAreaItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::pedestrianAreaItem)]);
   m_militaryBaseItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::militaryBaseItem)]);
   m_individualBuildingItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::individualBuildingItem)]);
   m_subwayLineItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::subwayLineItem)]);
   m_nullItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::nullItem)]);
   m_ferryItemAllocator
      ->reallocate(nbrItemsPerType[int(ItemTypes::ferryItem)]);

   initNonItemAllocators( nbrGfxDatas, nbrGfxDataSingleSmallPoly,
                          nbrGfxDataSingleLine, nbrGfxDataSinglePoint,
                          nbrGfxDataMultiplePoints, nbrNodes, 
                          nbrConnections );
   
   m_simpleItemAllocator->reallocate( nbrSimpleItems );
   m_cartographicItemAllocator->reallocate( nbrCartoItems );

   mc2dbg4 << "internalLoad_t after allocators ctor" << endl;
   mc2dbg << "[" << prettyMapIDFill(m_mapID) << "] #gfxDataFull="
          << nbrGfxDatas 
          << ", #gfxDataSingleSmallPoly=" << nbrGfxDataSingleSmallPoly
          << ", #gfxDataSingleLine=" << nbrGfxDataSingleLine
          << ", #gfxDataSinglePoint=" << nbrGfxDataSinglePoint
          << ", #gfxDataMultiplePoints=" << nbrGfxDataMultiplePoints
          << "\n # pois = "
          << (nbrItemsPerType[int(ItemTypes::pointOfInterestItem)])
          << "\n #ssis = "
          << nbrItemsPerType[int(ItemTypes::streetSegmentItem)]
          << ", #nodes = " << nbrNodes
          << ", #connections = " << nbrConnections
          << ", #simpleItems=" << nbrSimpleItems
          << ", #cartographicItems=" << nbrCartoItems << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Allocators allocated in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());


   // ***************************************************************
   //                                                  Create GfxData
   // ***************************************************************
   // Please notice that the allocators must be created before the
   // GfxData for the map is created!
   DEBUG_DB(mc2dbg << "load gfxData for the entire map" << endl;)
   // Read past length
   dataBuffer.readNextLong();

   m_gfxData = createNewGfxData(&dataBuffer);
   //m_gfxData = GMSGfxData::createNewGfxData(&dataBuffer, this);
   mc2dbg4 << "internalLoad_t after GfxData" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] GfxData created in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());

   // ***************************************************************
   //                                            Read all the strings
   // ***************************************************************
   if (m_itemNames == NULL)
      m_itemNames = new OldItemNames;
   
   m_itemNames->internalLoad( dataBuffer, stringsCodedInUTF8() );

   mc2dbg4 << "internalLoad_t after OldItemNames" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] OldItemNames created in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());

   // ***************************************************************
   //                                              Read all the items
   // ***************************************************************
   uint32 currentZoom = 0;
   bool keepOnRunning = true;
   while ( (currentZoom < NUMBER_GFX_ZOOMLEVELS) && (keepOnRunning)) {
      DEBUG_DB(mc2dbg << "Processing zoomlevel: " << currentZoom << endl);

      // Read length and number of items with currentZoom
      uint32 lengthOfItems = dataBuffer.readNextLong();
      uint32 nbrItems = dataBuffer.readNextLong();
      if (nbrItems == 0) {
         m_itemsZoomSize[currentZoom] = 0;
         m_itemsZoomAllocated[currentZoom] = 0;
         m_itemsZoom[currentZoom] = NULL;
      } else {
         // Allocate space
         m_itemsZoomSize[currentZoom] = nbrItems;
         m_itemsZoomAllocated[currentZoom] = nbrItems;
         m_itemsZoom[currentZoom] = new OldItem*[nbrItems];
         mc2dbg8 << "nbrItems=" << nbrItems << ", lengthOfItems=" 
                 << lengthOfItems << endl;
         
         // Read all the items
         for (uint32 i=0; i<nbrItems; i++) {
            // Make sure that the new position is clear
            m_itemsZoom[currentZoom][i] = NULL;
            dataBuffer.alignToLong();
            uint32 id = addItemToDatabase(&dataBuffer);
            // Make sure (?) that the item is added at the correct 
            // position with the correct id
            if (id != MAX_UINT32) {
               MC2_ASSERT(id == CREATE_ITEMID(currentZoom, i));
               MC2_ASSERT(id == m_itemsZoom[currentZoom][i]->getID());
               modifyItemNumberName(m_itemsZoom[currentZoom][i]);
            } else {
               mc2dbg8 << "item z=" << currentZoom << ", i=" << i 
                       << " == NULL" << endl;
               MC2_ASSERT(NULL == m_itemsZoom[currentZoom][i]);
            }
         }
      }
      CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) << "] " 
                           << nbrItems << " items (" << lengthOfItems/1024.0 
                           << "kb), zoom " << currentZoom << " created in "
                           << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                           << " ms" << endl;
                    startTime = TimeUtility::getCurrentMicroTime());
         currentZoom++;
   }
   mc2dbg4 << "internalLoad_t after all items" << endl;

   // ******************************************************************
   //                                          Read external connections
   // ******************************************************************
   if(keepOnRunning) {
      mc2dbg4 << "To load external connections" << endl;
      uint32 externalSize = dataBuffer.readNextLong();
      mc2dbg << "Size of boundry segments:" << externalSize << endl;
      if (externalSize > 0) {
         delete m_segmentsOnTheBoundry;
         m_segmentsOnTheBoundry = new OldBoundrySegmentsVector(&dataBuffer, 
                                                            this);
      }
   }
   mc2dbg4 << "internalLoad_t after seg. on boundry" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Seg. on boundry created in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());

   // ***************************************************************
   //                                              Fill the hashtable
   // ***************************************************************
   if (keepOnRunning) {
      buildHashTable();
   }
   mc2dbg4 << "internalLoad_t after hashtable" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Hash table created in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());

   // ******************************************************************
   //                                            Read the landmark table
   // ******************************************************************
   if(keepOnRunning) {
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
   }
   mc2dbg4 << "internalLoad_t after landmark table" << endl;
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Landmark table loaded in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());
#if 1
   // ******************************************************************
   //                                      Read the node expansion table
   // ******************************************************************
   if(keepOnRunning) {
      mc2dbg2 << "To load the node expansion table" << endl;
      if ( dataBuffer.getCurrentOffset() < dataBuffer.getBufferSize() ) {
         uint32 nodeExpSize = dataBuffer.readNextLong();
         if (nodeExpSize > 0) {
            for (uint32 i = 0; i < nodeExpSize; i++) {
               uint32 fromNodeID = dataBuffer.readNextLong();
               uint32 toNodeID = dataBuffer.readNextLong();
               mc2dbg8 << "from node : " << fromNodeID << " to node : "
                       << toNodeID << endl;
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


#endif   
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID)
                        << "] OldNode expansion table loaded in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());
   

   // Print the native languages of this map.
   mc2dbg << "Native languages of map 0x" << hex << getMapID() 
          << dec << "(" << getMapID() << ") nbr langs:" 
          << getNbrNativeLanguages()  << endl;
   for ( uint32 l=0; l<getNbrNativeLanguages(); l++){
      mc2dbg << "Lang " << l << ": " 
             << LangTypes::getLanguageAsString( getNativeLanguage(l) ) 
             << endl;
   }


   // From all items, remove all groups pointing at NULL or NULL items
   // Why this is needed:
   // If the server code is of older version than the map is, the map can
   // contain item of item types the code doesn't know about. Then the code
   // creates a OldNullItem to read past the data..
   // Any references to/from these unknown OldNullItems are removed here.
   uint32 nbrRemoved = 0;
   for ( uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item != NULL) {
            uint32 nbrGroups = item->getNbrGroups();
            for ( int g = nbrGroups-1; g >= 0; g-- ) {
               uint32 groupID = item->getGroup(g);
               OldItem* gItem = itemLookup(groupID);
               if ( (gItem == NULL) ||
                    (gItem->getItemType() == ItemTypes::nullItem) ) {
                  mc2dbg8 << "Removing null group " << g 
                          << " with id " << groupID 
                          << " from item " << item->getID() << endl;
                  item->removeGroup(g);
                  nbrRemoved++;
               }
            }
         }
      }
   }
   if ( nbrRemoved > 0 ) {
      mc2dbg << "Removed " << nbrRemoved 
             << " group IDs pointing at null" << endl;
   }

#if 0
   // Testing
   // Fill the UserRightsItemTable
   // Everything with a name that contains a "b" or "B" is forbidden.
   UserRightsItemTable::itemMap_t urMap;
   for ( uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item == NULL) continue;
         //if ( z == 14 ) mc2dbg << "[GM]: " << item->toString() << endl;
         bool b = false;
         // Do thisd only for the POI:s, since they are the important ones.
         if ( item->getItemType() == ItemTypes::pointOfInterestItem ) {
            for ( byte n = 0; n < item->getNbrNames(); ++n ) {
               if( strchr( m_itemNames->getString(
                              item->getStringIndex(n)), 'b'  ) ||
                   strchr( m_itemNames->getString(
                              item->getStringIndex(n)), 'B' )  ) {
                  b = true;
                  break;
               }
            }
         }
         if ( b ) {
            UserEnums::URType type;
            type |= UserEnums::UR_FREE_TRAFFIC;
            urMap[ item->getID() ] = UserEnums::URType(  type );
                                   
         } else {
            urMap[ item->getID() ] = UserEnums::allMask;
         }
      }
   }
   UserRightsItemTable newOne( UserEnums::allMask,
                               urMap );
   // Set the new one
   m_userRightsTable->swap( newOne );
#endif
   
   return keepOnRunning;
}

bool
OldGenericMap::internalSave(int outfile)
{
   if ( ! OldGenericMapHeader::internalSave(outfile) ) {
      return false;
   }
   uint32 startTime = TimeUtility::getCurrentMicroTime();
   dbgPrintFilePos("Starting OldGenericMap::internalSave", outfile);

   // Store file position and write marker for size.
   off_t origFilePos = lseek(outfile, 0, SEEK_CUR);
   if ( origFilePos == (off_t)-1 ){
      mc2log << error << "OldGenericMap::internalSave falied to get file pos" 
             << endl;
      exit(1);
   }
   uint32 sizeMarker = 0;
   write(outfile, &sizeMarker, 4);

   
   DataBuffer *dataBuffer = new DataBuffer(4 + 8*m_nbrAllocators + 256);
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
   uint32 nbrCartoItems = 0;
   uint32 nbrItemsWithType[ OldGenericMapHeader::numberInitialItemTypeAllocators ];

   for ( uint32 k=0;
         k < OldGenericMapHeader::numberInitialItemTypeAllocators; ++k) {
      nbrItemsWithType[k] = 0;
   }

   if (m_gfxData != NULL) {
      switch (m_gfxData->getGfxDataType()) {
         case GfxData::gfxDataFull :
            ++nbrGfxDatas;
            break;
         case GfxData::gfxDataSingleSmallPoly :
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
         OldItem* item = getItem(z, i);
         if (item != NULL) {
            ItemTypes::itemType type = item->getItemType();
            if ( uint32(type) < 
                     OldGenericMapHeader::numberInitialItemTypeAllocators ) {
               ++(nbrItemsWithType[int(type)]);
            } else {
               if ( type == ItemTypes::borderItem ) {
                  nbrSimpleItems++;
               }
               else if (type == ItemTypes::cartographicItem ){
                  nbrCartoItems++;
               } else {
                  mc2log << error << here << "OldItem type " << int(type)
                         << " not handled" << endl;
               }
            }
            if (item->getGfxData() != NULL) {
               switch (item->getGfxData()->getGfxDataType()) {
                  case GfxData::gfxDataFull :
                     ++nbrGfxDatas;
                     break;
                  case GfxData::gfxDataSingleSmallPoly :
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
            OldRouteableItem* ri = dynamic_cast<OldRouteableItem*>(item);
            if (ri != NULL) {
               for (int j=0; j<2; j++) {
                  if (ri->getNode(j) != NULL) {
                     ++nbrNodes;
                     nbrConnections += ri->getNode(j)->getNbrConnections();
                  }
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
         t < int(OldGenericMapHeader::numberInitialItemTypeAllocators); ++t) {
      dataBuffer->writeNextLong( t );
      dataBuffer->writeNextLong(nbrItemsWithType[ t ]);
      mc2dbg << "Save: " << nbrItemsWithType[ t ] 
              << " items with type " << t << endl;
   }
   dataBuffer->writeNextLong(nbrGfxDatas);               // extra 1
   dataBuffer->writeNextLong(nbrNodes);                  // extra 2
   dataBuffer->writeNextLong(nbrConnections);            // extra 3
   dataBuffer->writeNextLong(nbrGfxDataSingleSmallPoly); // extra 4
   dataBuffer->writeNextLong(nbrGfxDataSingleLine);      // extra 5

   mc2dbg << "#gfxDataFull=" << nbrGfxDatas << ", #gfxDataSingleSmallPoly=" 
          << nbrGfxDataSingleSmallPoly << ", #gfxDataSingleLine="
          << nbrGfxDataSingleLine << endl;
   
   // New. Check the number of allocators (from OldGenericMapHeader)
   // and save these allocators if allowed.
   int nbrAllocatorsLeft = m_nbrAllocators -
         OldGenericMapHeader::numberInitialItemTypeAllocators - 5;
   if ( nbrAllocatorsLeft > 1 ) {
      mc2dbg << "#gfxDataSinglePoint=" << nbrGfxDataSinglePoint << endl;
      dataBuffer->writeNextLong(nbrGfxDataSinglePoint);     // extra 6
      nbrAllocatorsLeft--;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#gfxDataMultiPoint=" << nbrGfxDataMultiplePoints << endl;
      dataBuffer->writeNextLong(nbrGfxDataMultiplePoints);  // extra 7
      nbrAllocatorsLeft--;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#simpleItems=" << nbrSimpleItems << endl;
      dataBuffer->writeNextLong( nbrSimpleItems ); // extra 8
      nbrAllocatorsLeft--;
   }
   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << "#cartographicItems=" << nbrCartoItems << endl;
      dataBuffer->writeNextLong( nbrCartoItems ); // extra 9
      nbrAllocatorsLeft--;
   }

   if ( nbrAllocatorsLeft > 0 ) {
      mc2dbg << nbrAllocatorsLeft << " more allocators to write!!!" << endl;
   }
   
   write(outfile, 
         dataBuffer->getBufferAddress(), 
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After allocators sizes", outfile);

  delete (dataBuffer);

   // Save the gfx-data for the entire map.
   if (m_gfxData != NULL) {
      dataBuffer = new DataBuffer( m_gfxData->getSizeInDataBuffer() + 4 );
      mc2dbg << "Saving gfxData for the map " <<endl;
      dataBuffer->writeNextLong(0);    // Length of gfxData, filled in below
      m_gfxData->save( *dataBuffer );
      // write length
      dataBuffer->writeLong(dataBuffer->getCurrentOffset()-4, 0);
      mc2dbg << "   Saving gfxData for the map. Length="
             << dataBuffer->getCurrentOffset()-4 <<endl;
      MC2_ASSERT( dataBuffer->getCurrentOffset() - 4 == 
                  m_gfxData->getSizeInDataBuffer() );
      write(outfile, 
            dataBuffer->getBufferAddress(), 
            dataBuffer->getCurrentOffset());
      dbgPrintFilePos("After map gfx data", outfile);

      delete dataBuffer;
   } else {
      mc2log << fatal <<"OldGenericMap::save(), gfxData == NULL" << endl;
      exit(1);
   }


   // ***************************************************************
   //                                            Save all the strings
   // ***************************************************************
   mc2dbg << "Saving item names" << endl;
   m_itemNames->save(outfile);
   dbgPrintFilePos("After item names", outfile);
   // ***************************************************************
   //                                              Save all the items
   // ***************************************************************
   mc2dbg << "Saving items" << endl;
   for ( int currentZoom=0; 
         currentZoom<NUMBER_GFX_ZOOMLEVELS; 
         currentZoom++) 
   {
      DEBUG_DB(mc2dbg << "   Processing zoomlevel: " << currentZoom << endl;)
      
      mc2dbg << "   Processing zoomlevel: " << currentZoom << endl;
      // In case we are saving a 2nd level overview map or greater,
      // increase the max size of the zoomlevel on disk.
      uint32 maxSize = MAX_BUFFER_SIZE;
      if( MapBits::getMapLevel( getMapID() ) >= 2 ) {
         maxSize = MAX_BUFFER_SIZE_SUPER_OVERVIEW;
      }
	      
      dataBuffer = new DataBuffer( maxSize );
      dataBuffer->fillWithZeros();

      dataBuffer->writeNextLong(0); // This value is set before writing to disk
      dataBuffer->writeNextLong(m_itemsZoomSize[currentZoom]);
      DEBUG1(
      if (m_itemsZoomSize[currentZoom] > 0)
         mc2dbg1 << "Saving " << m_itemsZoomSize[currentZoom] 
                 << " items in zoomlevel " << currentZoom << endl;
      );

      // Ask every item to save himself to disk
      for (uint32 i=0; i<m_itemsZoomSize[currentZoom]; i++) {
         if (m_itemsZoom[currentZoom][i] != NULL) {
            modifyItemNumberName(m_itemsZoom[currentZoom][i]);
            saveOneItem(dataBuffer, m_itemsZoom[currentZoom][i]);
         } else {
            OldNullItem nullItem(currentZoom << 27 | i);
            saveOneItem(dataBuffer, &nullItem);
         }
         dataBuffer->alignToLongAndClear();
      }

      DEBUG_DB (mc2dbg << "dataBuffer for zoomlevel " << currentZoom << endl;
                dataBuffer->dump();)
      
      // Make sure the zoomlevel is aligned on disk. Pad until that is 
      // the case.
      dataBuffer->alignToLongAndClear();
      
      // Fill in the length of the items with this zoomlevel;
      mc2dbg4 << "LengthOfItems = " 
              << (dataBuffer->getCurrentOffset()-8) << endl;
      dataBuffer->writeLong(dataBuffer->getCurrentOffset()-8, 0);
      write(outfile, 
            dataBuffer->getBufferAddress(), 
            dataBuffer->getCurrentOffset());
      MC2String mess = "After items zoom: ";
      STLStringUtility::int2str(currentZoom, mess);
      dbgPrintFilePos(mess, outfile);
      mc2dbg << "Save:" << " Size of data of zoomlevel " << currentZoom
             << ":" << dataBuffer->getCurrentOffset() << endl;

      delete dataBuffer;
      dataBuffer = NULL;
   }

   // ***************************************************************
   //                                       Save external connections
   // ***************************************************************

   mc2dbg2 << "saveSegmentsOnBoundry" << endl;
   dataBuffer = new DataBuffer(100000000); // Size hardcoded
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(0);          // size set later
   if (m_segmentsOnTheBoundry != NULL) {
      m_segmentsOnTheBoundry->save(dataBuffer);
      //set size
      dataBuffer->writeLong(dataBuffer->getCurrentOffset()-4, 0);
      mc2log << "m_segmentsOnTheBoundry saved, size=" 
             << dataBuffer->getCurrentOffset()-4 << "bytes, nbrElm=" 
             << m_segmentsOnTheBoundry->getSize() << endl;
   } else {
      mc2log << warn << here << " Failed to save boundry segments" << endl;
      if (m_segmentsOnTheBoundry == NULL)
         mc2dbg << "m_segmentsOnTheBoundry == NULL" << endl;
      else
         mc2dbg << "m_segmentsOnTheBoundry->save() returned false" << endl;
   }
   write(outfile, 
         dataBuffer->getBufferAddress(), 
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("Segments on the boundary", outfile);
   delete dataBuffer;

   // ***************************************************************
   //                                         Save the landmark table
   // ***************************************************************

   mc2dbg2 << "saveLandmarkTable" << endl;
   typedef landmarkTable_t::iterator LI;
   uint32 landmarkSize = m_landmarkTable.size();

   dataBuffer = new DataBuffer(100000000); // Size hardcoded
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
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After landmarks table", outfile);
   mc2dbg1 << "Saved " << landmarkSize << " landmark descriptions" << endl; 
   delete dataBuffer;

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
   
   dataBuffer = new DataBuffer( bufSize ); 
   dataBuffer->fillWithZeros();

   DataBufferChecker dbc( *dataBuffer, "OldGenericMap::save expanded nodes." );
   
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
   
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After node expansion table", outfile);
   mc2dbg1 << "Saved " << nodeExpSize << " expanded nodes" << endl; 
   delete dataBuffer;
   dataBuffer = NULL;

   // Save m_userRightsTable
   bufSize = m_userRightsTable->getSizeInDataBuffer();
   dataBuffer = new DataBuffer( bufSize ); 
   dataBuffer->fillWithZeros();

   m_userRightsTable->save( *dataBuffer );
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   mc2dbg1 << "Saved " << "m_userRightsTable" << endl;
   dbgPrintFilePos("After user rights table", outfile);
   delete dataBuffer;
   dataBuffer = NULL;

   // Save m_adminAreaCentres table
   // size(m_nbrAdminAreaCentres) + 
   //       m_nbrAdminAreaCentres * ( itemId + lat + lon )
   bufSize = 4 + m_nbrAdminAreaCentres * ( 4 + 4 + 4 );
   dataBuffer = new DataBuffer( bufSize );
   DataBufferChecker dbc2( *dataBuffer,
                           "OldGenericMap::save admin area centres table." );
   dataBuffer->writeNextLong( m_nbrAdminAreaCentres );
   for ( uint32 i = 0; i < m_nbrAdminAreaCentres; i++ ) {
      adminAreaCentre_t elem = m_adminAreaCentres[i];
      dataBuffer->writeNextLong( elem.itemID );
      dataBuffer->writeNextLong( elem.centre.lat );
      dataBuffer->writeNextLong( elem.centre.lon );
   }
   dbc2.assertPosition( bufSize );
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   mc2dbg1 << "Saved " << m_nbrAdminAreaCentres << " admin area centres" 
           << endl; 
   dbgPrintFilePos("After admin area centre table", outfile);
   delete dataBuffer;
   dataBuffer = NULL;

   // Save official codes of items table:
   dataBuffer = new DataBuffer( m_itemOfficialCodes.getSizeInDataBuffer() );
   m_itemOfficialCodes.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After official codes", outfile);
   mc2dbg << "Saved " << m_itemOfficialCodes.size() 
          << " item official codes." 
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;


   // Save categories of items table:
   dataBuffer = new DataBuffer( m_itemCategories.getSizeInDataBuffer() );
   m_itemCategories.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After categories", outfile);
   mc2dbg << "Saved " << m_itemCategories.size() 
          << " item categories." 
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;


   // Save lane vector of nodes table.
   dataBuffer = new DataBuffer( m_nodeLaneVectors.getSizeInDataBuffer() );
   m_nodeLaneVectors.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After lane vectors", outfile);
   mc2dbg << "Saved " << m_nodeLaneVectors.size() 
          << " node lane vectors." 
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;


   // Save connecting lanes table.
   dataBuffer = new DataBuffer( m_connectingLanesTable.sizeInDataBuffer() );
   m_connectingLanesTable.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After connecting lanes", outfile);
   mc2dbg << "Saved " << m_connectingLanesTable.size() 
          << " connecting lanes posts."
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;

   // Save connection sign post table.
   dataBuffer = new DataBuffer( m_signPostTable.sizeInDataBuffer() );
   m_signPostTable.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After sign post table", outfile);
   mc2dbg << "Saved " << m_signPostTable.size() 
          << " sign posts."
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;

   // Save index areas order table.
   dataBuffer = new DataBuffer( m_indexAreasOrder.getSizeInDataBuffer() );
   m_indexAreasOrder.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After index areas order table", outfile);
   mc2dbg << "Saved " << m_indexAreasOrder.size() 
          << " index areas order posts."
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;

   // Save non searchable items table.
   dataBuffer = new DataBuffer( m_nonSearchableItems.getSizeInDataBuffer() );
   m_nonSearchableItems.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After non searchable items table", outfile);
   mc2dbg << "Saved " << m_nonSearchableItems.size() 
          << " non searchable items posts."
          << endl; 
   delete dataBuffer;
   dataBuffer = NULL;

   // Save road display class table.
   dataBuffer = new DataBuffer( m_roadDisplayClass.getSizeInDataBuffer() );
   m_roadDisplayClass.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After road display class table", outfile);
   mc2dbg << "Saved " << m_roadDisplayClass.size() 
          << " road display class posts." << endl; 
   delete dataBuffer;
   dataBuffer = NULL;

   // Save area feature draw display class table.
   dataBuffer = 
      new DataBuffer( m_areaFeatureDrawDisplayClass.getSizeInDataBuffer() );
   m_areaFeatureDrawDisplayClass.save(*dataBuffer);
   dataBuffer->alignToLongAndClear();
   write(outfile,
         dataBuffer->getBufferAddress(),
         dataBuffer->getCurrentOffset());
   dbgPrintFilePos("After area feature draw display class table", outfile);
   mc2dbg << "Saved " << m_areaFeatureDrawDisplayClass.size() 
          << " area feature draw display class posts." << endl; 
   delete dataBuffer;
   dataBuffer = NULL;



   // Add new stuff above this.
   // *********************************************************************

   // Write the size written
   off_t curFilePos = lseek(outfile, 0, SEEK_CUR);
   if ( curFilePos == (off_t)-1 ){
      mc2log << error << "OldGenericMap::internalSave falied to get file pos" 
             << endl;
      exit(1);
   }
   off_t result = lseek(outfile, origFilePos, SEEK_SET);
   if ( result == (off_t)-1 ){
      mc2log << error << "OldGenericMap::internalSave falied to set file pos" 
             << " 1" << endl;
      exit(1);
   }
   off_t sizeWritten = curFilePos - origFilePos;
   DataBuffer dbuf(4);                      // Reading with data buffer, 
   dbuf.writeNextLong(sizeWritten);         // therefore writing with it.
   write(outfile, dbuf.getBufferAddress(), 4);
   result = lseek(outfile, curFilePos, SEEK_SET);
   if ( result == (off_t)-1 ){
      mc2log << error << "OldGenericMap::internalSave falied to set file pos" 
             << " 2" << endl;
      exit(1);
   }
   mc2dbg << "OldGenericMap body size: " << sizeWritten << " written." 
          << endl;



   uint32 endTime = TimeUtility::getCurrentMicroTime();
   mc2dbg << "OldGenericMap::save(), OldMap: 0x" << hex 
          << this->getMapID() << dec << "(" << this->getMapID()
          << ") saved in "
          << (endTime - startTime) / 1000 << " ms." << endl;
   return (true);
}

bool
OldGenericMap::getItemCoordinates(uint32 itemID, uint16 offset,
                               int32 &lat, int32 &lon) const
{
   const OldItem* i = itemLookup(itemID);
   if (i == NULL) {
      mc2dbg2 << "getItemCoordinates returns false 1" << endl;
      return (false);
   }

   const GfxData* gfx = i->getGfxData();
   // Incase of a point of interest item,
   // get the streetsegmentitem gfxData and offset.
   if ( ( i->getItemType() == ItemTypes::pointOfInterestItem ) &&
        ( gfx == NULL ) ) {
      const OldPointOfInterestItem* poi =
         static_cast<const OldPointOfInterestItem*> (i);
      gfx = (itemLookup(poi->getStreetSegmentItemID()))->getGfxData();
      offset = poi->getOffsetOnStreet();
   } else if (i->getItemType() == ItemTypes::streetItem) {
       
      // Special treatment for street items since they don't have any
      // gfxdata, but consists of a number of street segments with gfxdata.
      // Note that offset is not defined for a street so therefore the 
      // middle of the street is returned.
      const OldStreetItem* streetItem = static_cast<const OldStreetItem*> ( i );
     
      // The highest road class of the street.
      byte roadClass = MAX_BYTE;
      
      // Get the boundingbox of all constituent street segments.
      MC2BoundingBox bbox;
      for ( uint32 j = 0; j < streetItem->getNbrItemsInGroup(); ++j ) {
         OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
            ( itemLookup( streetItem->getItemNumber( j ) ) );
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
         OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
            ( itemLookup( streetItem->getItemNumber( j ) ) );
         if ( ssi->getRoadClass() == roadClass ) { 
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
getCentreFromAdminTable( const OldGenericMap::adminAreaCentre_t* table,
                        int size,
                        uint32 itemID )
{
   if ( table == NULL ) {
      return MC2Coordinate();
   }
   const OldGenericMap::adminAreaCentre_t* end = table + size;
   const OldGenericMap::adminAreaCentre_t* findit =
      lower_bound( table, end, itemID, AdminAreaCentreCmp() );
   if ( findit != end && findit->itemID == itemID ) {
      return findit->centre;
   } else {
      return MC2Coordinate();
   }
}

MC2Coordinate
OldGenericMap::getCentreFromAdminTable(uint32 itemID) const
{
   return ::getCentreFromAdminTable( m_adminAreaCentres,
                                     m_nbrAdminAreaCentres,
                                     itemID );
}

void
OldGenericMap::getOneGoodCoordinate(MC2Coordinate& coord, 
                                 const OldItem* item) const
{
   const GfxData* gfxData = item->getGfxData();

   const bool closed = ( (gfxData != NULL) && (gfxData->closed()) );
   const uint32 itemID = item->getID();

   MC2Coordinate tableCoord = 
      ::getCentreFromAdminTable( m_adminAreaCentres,
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
      // will handle it. (OldStreetItem).
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
         case ItemTypes::individualBuildingItem: {
            // Check if the item is inside the map (convex hull)
            float64 tmpDist = getGfxData()->signedSquareDistTo(coord.lat,
                                                               coord.lon);
            // This distance happens to be the same as in OldMapReader.
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
               ((OldGenericMap*)(this))->getClosestItemID(coord,
                                                       dist,
                                                       allowedTypes);
            // Here we should use the closest coordinate.
            getOneGoodCoordinate(coord, itemLookup(closestSSI) );
            return; // RETURNS HERE
         }
         break;
         default:
            /* OldItem is always ok */
            break;
      }
   }
   
  
}

MC2Coordinate
OldGenericMap::getOneGoodCoordinate(const OldItem* item) const
{
   MC2Coordinate retVal;
   getOneGoodCoordinate(retVal, item);
   return retVal;
}

void
OldGenericMap::getItemBoundingBox(MC2BoundingBox& bbox,
                               const OldItem* item) const
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
OldGenericMap::getItemBoundingBox(const OldItem* item) const
{
   MC2BoundingBox retVal;
   getItemBoundingBox(retVal, item);
   return retVal;
}

int32
OldGenericMap::getItemOffset(uint32 itemID, int32 lat, int32 lon) const
{
   // If the item with itemID not is found, this is handled in the
   // other version of the getItemOffset-mathod...
   return (getItemOffset(itemLookup(itemID), lat, lon));
}
   
int32
OldGenericMap::getItemOffset(OldItem* item, int32 lat, int32 lon) const
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
   mc2dbg4 << "OldGenericMap:: offset=" << offset 
           << ", (latOnPoly, lonOnPoly)=" << latOnPoly << "," 
           << lonOnPoly << ")" << endl;

   return (offset);
                              
}

bool 
OldGenericMap::bindItemToGroup(uint32 itemID, uint32 groupID)
{
   // Find the items
   OldItem *thisItem = itemLookup(itemID);
   OldGroupItem* thisGroup = dynamic_cast<OldGroupItem*>(itemLookup(groupID));

   // Check that both items are correct
   if ( (thisItem == NULL) || (thisGroup == NULL))  {
      mc2dbg2 << "OldGenericMap::bindItemToGroup failed "
              << "groupID is not a group" << endl;
      return false;
   }

   return (bindItemToGroup(thisItem, thisGroup));
}


bool 
OldGenericMap::bindItemToGroup(OldItem* item, OldGroupItem* group)
{
   if (!addRegionToItem(item, group->getID()))
      return false;
   if (!group->addItem(item->getID()))
      return false;
   return true;
}

bool 
OldGenericMap::removeItemFromGroup(OldItem* item, OldGroupItem* group)
{
   if (!item->removeGroupWithID(group->getID()))
      return false;
   if (!group->removeItemWithID(item->getID()))
      return false;
   return true;
}

bool 
OldGenericMap::removeItemFromGroup(uint32 itemID, uint32 groupID)
{
   // Find the items
   OldItem* item = itemLookup(itemID);
   OldGroupItem* group = dynamic_cast<OldGroupItem*>(itemLookup(groupID));

   // Check that both items are correct
   if ( (item == NULL) || (group == NULL))  {
      mc2log << error << "OldGenericMap::rendItemToGroup failed "
             << "groupID is not a group" << endl;
      return false;
   }

   return (removeItemFromGroup(item, group));
}


uint32
OldGenericMap::addNameToItem(uint32 itemID, const char* name, 
                   LangTypes::language_t lang, ItemTypes::name_t type)
{
   if( name == NULL )
      return MAX_UINT32;

   OldItem* tmpItem = itemLookup(itemID);
   if (tmpItem == NULL)
      return (MAX_UINT32);
   return (addNameToItem(tmpItem, name, lang, type));
}

uint32
OldGenericMap::addNameToItem(OldItem* item, const char* name,
                   LangTypes::language_t lang, ItemTypes::name_t type)
{
   if( name == NULL )
      return MAX_UINT32;
   
   // Find out if the case of the item name should be changed
   bool changeCase = false;
   ItemTypes::itemType curType = ItemTypes::itemType(item->getItemType());
   if ( (curType != ItemTypes::pointOfInterestItem) &&
        (curType != ItemTypes::categoryItem) ) {
      changeCase = true;
   }

   if(type == ItemTypes::roadNumber){
      lang = NameUtility::getNumberNameValue(name);
   }
   
   uint32 nameIndex = m_itemNames->addString(name, changeCase);
   item->addName(lang, type, nameIndex);
   return (nameIndex);
}

bool
OldGenericMap::modifyItemNumberName(OldItem* item){
   if(item->getItemType() != ItemTypes::streetSegmentItem)
      return false;
   int nbrOfNames = item->getNbrNames();
   for(int i = 0; i < nbrOfNames ; i++){
      if(item->getNameType(i) == ItemTypes::roadNumber){
         if(GET_STRING_LANGUAGE(item->getRawStringIndex(i)) !=
            LangTypes::invalidLanguage){
            // allready set, abort.
            return false;
         }
         char* name = m_itemNames->getString(item->getStringIndex(i));
         if(name != NULL){
            LangTypes::language_t
               newLang = NameUtility::getNumberNameValue(name);
            item->setNameLanguage(i, newLang);
         }
      }
   }
   return true;
}


uint32 
OldGenericMap::addName(const char* name,
                       LangTypes::language_t lang, ItemTypes::name_t type)
{
   if ( name == NULL) {
      return (MAX_UINT32);
   }
   
   // Add name with changeCase == true.
   uint32 nameIndex = m_itemNames->addString(name, false);
   return (nameIndex);
}


void 
OldGenericMap::getMapBoundingBox(MC2BoundingBox& bbox) const
{
   MC2_ASSERT(m_gfxData != NULL);
   
   // Initialize the bbox to the map-boundry
   m_gfxData->getMC2BoundingBox(bbox);

   // Loop over all the water-items and update bbox
   for ( int z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      uint32 nbrItems = getNbrItemsWithZoom(z);
      for (uint32 i=0; i<nbrItems; i++) {
         // Check water items
         OldItem* item = getItem(z, i);
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
OldGenericMap::
getItemsWithinRadiusMeter(set<OldItem*>& resultItems,
                          const MC2Coordinate& center,
                          int radiusMeters,
                          const set<ItemTypes::itemType>& allowedTypes,
                          const UserRightsMapInfo* rights)
{
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

void
OldGenericMap::getItemsWithinBBox(set<OldItem*>& resultItems,
                               const MC2BoundingBox& bbox,
                               const set<ItemTypes::itemType>& allowedTypes,
                               const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes( allowedTypes, rights );

   set<uint32> idsInside;
   uint32 startTime = TimeUtility::getCurrentMicroTime();
   m_hashTable->getAllWithinMC2BoundingBox(idsInside,
                                           bbox);
   mc2dbg2 << "TIME: getAllWithinMC2BoundingBox  "
                << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0 
                << " ms" <<  endl;  
   // We need to lookup the id:s
   for( set<uint32>::const_iterator it = idsInside.begin();
        it != idsInside.end();
        ++it ) {
      resultItems.insert( itemLookup(*it) );
   }
}

void
OldGenericMap::getIDsWithinRadiusMeter(set<uint32>& resultIDs,
                                    const MC2Coordinate& center,
                                    int radiusMeters,
                                    const set<ItemTypes::itemType>&
                                    allowedTypes,
                                    const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes( allowedTypes, rights );
   
   int32 radiusMC2 = int32(radiusMeters*GfxConstants::METER_TO_MC2SCALE);
   m_hashTable->getAllWithinRadius(resultIDs, center.lon, center.lat,
                                   radiusMC2);
}

void
OldGenericMap::getIDsWithinBBox(set<uint32>& resultIDs,
                             const MC2BoundingBox& bbox,
                             const set<ItemTypes::itemType>& allowedTypes,
                             const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes( allowedTypes, rights );

   set<uint32> tempRes;
   m_hashTable->getAllWithinMC2BoundingBox(tempRes,
                                           bbox);

   resultIDs.insert( tempRes.begin(), tempRes.end() );
}

void
OldGenericMap::getItemsWithinGfxData(set<OldItem*>& resultItems,
                                  const GfxData* gfxData,
                                  const set<ItemTypes::itemType>& allowedTypes,
                                  const UserRightsMapInfo* rights)
{
   if ( gfxData == NULL ) {
      return;
   }
   MC2BoundingBox bbox;
   set<OldItem*> tempRes;

   // Get all the items within all the bounding boxes.
   int nbrPolygons = gfxData->getNbrPolygons();
   for( int polygon = 0; polygon < nbrPolygons; ++polygon ) {
      gfxData->getMC2BoundingBox(bbox, polygon);
      getItemsWithinBBox(tempRes, bbox, allowedTypes, rights);
   }

   // Remove the items that were outside.
   for( set<OldItem*>::iterator it = tempRes.begin();
        it != tempRes.end() ; ) {
      OldItem* item = *it;
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
OldGenericMap::getIDsWithinGfxData(set<uint32>& resultIDs,
                                const GfxData* gfxData,
                                const set<ItemTypes::itemType>& allowedTypes,
                                const UserRightsMapInfo* rights)
{
   set<OldItem*> resultItems;
   getItemsWithinGfxData(resultItems, gfxData, allowedTypes, rights);
   // Blargh - copy the ids into set
   for(set<OldItem*>::const_iterator it = resultItems.begin();
       it != resultItems.end();
       ++it ) {
      resultIDs.insert((*it)->getID());
   }
}

uint32
OldGenericMap::getClosestItemID( const MC2Coordinate& coord,
                              uint64& dist,
                              const set<ItemTypes::itemType>& allowedTypes,
                              const UserRightsMapInfo* rights)
{
   m_hashTable->setAllowedItemTypes(allowedTypes, rights);
   return m_hashTable->getClosest(coord.lon, coord.lat, dist);   
}


void 
OldGenericMap::printAllNames()
{
   m_itemNames->printAllStrings();
}


bool
OldGenericMap::getAllItemsWithStringIndex(uint32 stringIndex, Vector* result)
{
   // Check the inparameters
   if (result == NULL) {
      mc2log << error
             << "OldGenericMap::getAllItemsWithStringIndex, result == NULL"
             << endl;
      return (false);
   }     
   // Get_STRING_INDEX is defined in OldItem.h
   stringIndex = GET_STRING_INDEX(stringIndex);
   result->reset();

   // Loop over all the items in the map and check the items that
   // have the name stringIndex
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z) {
      uint32 nbrIWZ = getNbrItemsWithZoom(z);
      for(uint32 i = 0; i < nbrIWZ; ++i) {
         OldItem* item = getItem(z, i);
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


void
OldGenericMap::dumpConnections()
{
   OldItem *item;
   OldNode *curNode;

   mc2log << info << "OldGenericMap::dumpConnections()" << endl;

   for ( int currentZoom=0; 
         currentZoom<NUMBER_GFX_ZOOMLEVELS; 
         currentZoom++) {


      for (uint32 i=0; i<m_itemsZoomSize[currentZoom]; i++) {
         if ( (item = m_itemsZoom[currentZoom][i]) != NULL &&
            ( item->getItemType() == ItemTypes::streetSegmentItem) ) {

            for (int k=0; k<2; k++) {
               curNode = ((OldStreetSegmentItem*) item)->getNode(k);
               for (uint16 j=0; j<curNode->getNbrConnections(); j++) {
                  printItemNames(curNode->getEntryConnection(j)
                                        ->getConnectFromNode());
                  mc2log << "[0x" << hex 
                       << curNode->getEntryConnection(j) ->getConnectFromNode()
                       << "]" << dec;
                  if (MapBits::isNode0(curNode->getEntryConnection(j)
                                          ->getConnectFromNode() )  )
                     mc2log << "(0) --> ";
                  else
                     mc2log << "(1) --> ";
                  printItemNames(item->getID());
                  mc2log << "[0x" << hex << item->getID() << dec << "]";
                  mc2log << "(" << k << "): ";
                  curNode->getEntryConnection(j)->printTurnDirection();
                  
                  mc2log << ", vehicleParam = 0x" << hex
                       << curNode->getEntryConnection(j)
                           ->getVehicleRestrictions() << dec;
                   
                  /* 
                  mc2log << ", conlength = " 
                       << curNode->getEntryConnection(j)->getLength();
                  mc2log << ", time = " 
                       << curNode->getEntryConnection(j)->getTime();
                  mc2log << ", stand still time = " 
                       << curNode->getEntryConnection(j)->getStandStillTime();
                  mc2log << ", vehicleParam = 0x" << hex
                       << curNode->getEntryConnection(j)
                           ->getVehicleRestrictions() << dec;
                  mc2log << ", costA = "
                       << curNode->getEntryConnection(j)->getCostA();
                  mc2log << ", costB = "
                       << curNode->getEntryConnection(j)->getCostB();
                  mc2log << ", avg speed = " 
                       << float64(curNode->getEntryConnection(j)->getLength())/
                          float64(curNode->getEntryConnection(j)->getTime());
                  */
                  mc2log << endl;
               }
            }
         }
      }
   }
}

void
OldGenericMap::dumpLandmarks()
{
   uint32 nbrLandmarks = m_landmarkTable.size();
   mc2log << info << "OldGenericMap::dumpLandmarks()" << endl
          << "This map has " << nbrLandmarks << " landmark descriptions."
          << endl;

   if (nbrLandmarks > 0) {
      typedef landmarkTable_t::iterator LI;
      Vector* IDs = new Vector;
      for (LI it = m_landmarkTable.begin();
           it != m_landmarkTable.end(); it++) {
         uint32 fromNodeID = uint32 (((*it).first >> 32) & 0x00000000ffffffff);
         uint32 toNodeID = uint32 ((*it).first & 0x00000000ffffffff);

         uint32 itemID = (*it).second.itemID;
         uint32 importance = (*it).second.importance;
         SearchTypes::side_t side = (*it).second.side;
         ItemTypes::landmarklocation_t location = (*it).second.location;
         ItemTypes::landmark_t type = (*it).second.type;
                  
         cout << " [" << fromNodeID << "->" << toNodeID << "] "
              << "(" << getFirstItemName(fromNodeID) << "->" 
              << getFirstItemName(toNodeID) << ": "
              << getFirstItemName(itemID) << ")" << endl
              << "   landmarktype:" << int(type)
              << ", importance:" << importance
              << ", location:" << StringTable::getString(
                     ItemTypes::getLandmarkLocationSC(location),
                     StringTable::ENGLISH)
              << ", side:(" << int(side) << ")" << StringTable::getString(
                    ItemTypes::getLandmarkSideSC(side), StringTable::ENGLISH)
              << ", itemID:" << itemID << endl;
         if (IDs->binarySearch(itemID) == MAX_UINT32)
            IDs->addLast(itemID);
         IDs->sort();
      }
      mc2log << info << "Number \"LM items\": " << IDs->getSize() << endl;
      delete IDs;
   }

}

void
OldGenericMap::buildHashTable()
{
   if (m_gfxData != NULL) {
      DEBUG4(uint32 startTime = TimeUtility::getCurrentMicroTime());
      MC2BoundingBox bb;
      m_gfxData->getMC2BoundingBox(bb);
      
      mc2dbg2 << "bbox minLon = " << bb.getMinLon() << endl;
      mc2dbg2 << "     maxLon = " << bb.getMaxLon() << endl;
      mc2dbg2 << "     minLat = " << bb.getMinLat() << endl;
      mc2dbg2 << "     maxLat = " << bb.getMaxLat() << endl;

      delete m_hashTable;
      uint32 nbrCells = 100;
      
      if ( MapBits::getMapLevel( m_mapID ) >= 2 ) {
         // Super overview map. We need more cells for fast searches.
         nbrCells = 300;  // We try a factor 9. 
      }


      m_hashTable = new OldMapHashTable(bb.getMinLon(),
                                        bb.getMaxLon(), bb.getMinLat(), bb.getMaxLat(),
                                        nbrCells, nbrCells, this);

      for ( int z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
            // The hashtable decides what do do with POI:s.
            m_hashTable->addItem( getItem( z, i ) );
         }
      }
      DEBUG4(uint32 stopTime = TimeUtility::getCurrentMicroTime());
      DEBUG4(mc2dbg << "Time to generate hashtable: " << stopTime-startTime 
                  << " us. ("  << nbrGfxData << " elm)" << endl);
   } else {
      mc2dbg2 << "GfxData for entire map == NULL!" << endl;
   }
}

uint32
OldGenericMap::addItemToDatabase( DataBuffer* dataBuffer,
                               byte zoomlevel,
                               bool asignNewItemID)
{
   // Create the right kind of OldItem.
   OldItem* i = createNewItem(dataBuffer);

   if (i != NULL) {
      switch (i->getItemType()) {
         case ItemTypes::streetSegmentItem :{
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 5;
         }

         break;
         case ItemTypes::streetItem: {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 4;
         } 

         break;
         case ItemTypes::municipalItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }

         break;
         case ItemTypes::waterItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::parkItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::forestItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::buildingItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::railwayItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 1;
         }
         break;
         case ItemTypes::islandItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::nullItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::zipAreaItem :
         case ItemTypes::zipCodeItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 10;
         }
         break;
         case ItemTypes::pointOfInterestItem : {
            zoomlevel = 14;   // Companies are _always_ in level 14
         }
         break;
         case ItemTypes::categoryItem : {
            zoomlevel = 15;   // Categories are _always_ in level 15
         }
         break;
         case ItemTypes::builtUpAreaItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::cityPartItem : {
            zoomlevel = ItemTypes::zoomConstCPI; //zoomConstCPI states
                                                 //the CP zoomlvl
         }
         break;
         
         case ItemTypes::busRouteItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;

         case ItemTypes::airportItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;

         case ItemTypes::aircraftRoadItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;

         case ItemTypes::pedestrianAreaItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;

         case ItemTypes::militaryBaseItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;

         case ItemTypes::individualBuildingItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;

         case ItemTypes::subwayLineItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 0;
         }
         break;
         case ItemTypes::ferryItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 1;
         }
         break;
         case ItemTypes::borderItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 4;
         }
         case ItemTypes::cartographicItem : {
            if (zoomlevel >= NUMBER_GFX_ZOOMLEVELS)
               zoomlevel = 4;
         }
         break;
         default :{
            mc2log << error << "OldGenericMap::addItemToDatabase Unknown item, "
                   << "id=" << i->getID() << ", type=" 
                   << (uint32) i->getItemType() << endl;
         }
         break;
      } // switch
   }

   if (i != NULL) {
      // Set the ID of the item. If we should asign a new ID, the
      // old one is not interesting
      uint32 currentItemID = MAX_UINT32;
      if (!asignNewItemID) {
         currentItemID = i->getID();
      }
      uint32 currentZoom = (currentItemID & 0x78000000) >> 27;
      // Check if it is a new item that should be added
      if ((currentItemID & 0x07ffffff) < m_itemsZoomSize[currentZoom]) {
         // The supplied item ID is already present.
         delete m_itemsZoom[currentZoom][currentItemID & 0x07FFFFFF];
         m_itemsZoom[currentZoom][currentItemID & 0x07FFFFFF] = i;
         return (currentItemID);
      } else {
         // The supplied item ID is not present or invalid
         if (currentItemID == MAX_UINT32)
            // Use the default zoomlevel for this item
            currentZoom = zoomlevel;
         return (addItem(i, currentZoom));
      }
   } else {
      return (MAX_UINT32);
   }
}


GMSGfxData*
OldGenericMap::createNewGfxData(DataBuffer* dataBuffer) 
{
   mc2dbg8 << here << " createNewGfxData in OldGenericMap!" << endl;
   return GMSGfxData::createNewGfxData(dataBuffer, this);
}

OldConnection* 
OldGenericMap::createNewConnection(DataBuffer* dataBuffer)
{
   return (OldConnection::createNewConnection(dataBuffer, this));
}

OldItem*
OldGenericMap::createNewItem(DataBuffer* dataBuffer)
{
   return (OldItem::createNewItem(dataBuffer, this));
}

bool
OldGenericMap::saveOneItem(DataBuffer* dataBuffer, OldItem* item)
{
   if (item == NULL) {
      // Error!
      mc2log << fatal << "NULL in the m_itemsZoom-vector" << endl;
      return (false);
   }

   // Make sure the buffer is aligned, pad with zero if not!
   dataBuffer->alignToLongAndClear();

   // Store the offset of the start to be able to fill in the 
   // length of the item
   uint32 startIndex = dataBuffer->getCurrentOffset() ;

   DEBUG_DB(mc2dbg << "Saving item of type "
            << (int) item->getItemType() << endl);
   item->save(dataBuffer);
   // Make sure the buffer is aligned at the end
   dataBuffer->alignToLongAndClear();

   // Fill in the length of this item at the correct index
   uint32 endIndex = dataBuffer->getCurrentOffset();
   if ( ! ((endIndex-startIndex) < 0xffffff) ) {
      mc2log << error << "Item " << item->getID() 
             << " startIndex=" << startIndex
             << " endIndex=" << endIndex 
             << " - diff" << (endIndex-startIndex)
             << " allowed=" << 0xffffff << endl;
   }
   MC2_ASSERT( (endIndex-startIndex) < 0xffffff);
   uint32 lengthAndType =  
      ( (uint32(item->getItemType()) << 24) & 0xff000000 ) |
      ( (endIndex-startIndex) & 0xffffff);
   DEBUG_DB(mc2dbg << "lengthAndType=0x" << hex << lengthAndType
            << dec << endl);
   dataBuffer->writeLong(lengthAndType, startIndex);

   // Everything OK!
   return (true);
}


const OldItem* 
OldGenericMap::getItemFromItemIdentifier( const ItemIdentifier& ident ) const
{
   set<const OldItem*> itemCandidates;

   // Get all items of the correct type and name.
   getItemsWithName( ident.getName(), 
                     itemCandidates, 
                     ident.getItemType() );

   const OldItem* result = NULL;
   
   // Find out if the item is identified by insideName or coordinates.
   if ( ident.getInsideName() != NULL ) {
      // Identified by being inside an item named ident.getInsideName().
   
      set<const OldItem*> areaItems;
      getItemsWithName( ident.getInsideName(), areaItems );

      set<const OldItem*>::const_iterator it = itemCandidates.begin();
      bool multipleHits = false;
      
      while ( ( it != itemCandidates.end() ) && ( ! multipleHits ) ) {
         
         set<const OldItem*>::const_iterator jt = areaItems.begin();
         while ( ( jt != areaItems.end() ) && ( ! multipleHits ) ) {
            
            // Check if *it logically is inside *jt.
            const OldGroupItem* groupItem = 
               dynamic_cast<const OldGroupItem*> (*jt);
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
  
      const OldItem* closestLinearItem = NULL;
      uint32 nbrCloseLinearItems = 0;
      uint64 closestSqDist = MAX_UINT64;

      int32 lat = ident.getLat();
      int32 lon = ident.getLon();
     
      bool multipleHits = false;
      set<const OldItem*>::const_iterator it = itemCandidates.begin();
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
OldGenericMap::getItemIdentifierForItem( const OldItem* item, 
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

      OldItem* areaItem = getRegion(item, ItemTypes::builtUpAreaItem);
      if ( areaItem == NULL ) {
         areaItem = getRegion(item, ItemTypes::municipalItem);
      }

      if ( ( areaItem != NULL ) && ( areaItem->getNbrNames() > 0 ) ) {
         areaName = getName( areaItem->getStringIndex( 0 ) );
         retVal = true;
      }

      if ( retVal ) {
         ident.setParameters( itemName, item->getItemType(), areaName );
      }
   }

   if ( retVal ) {
      // Make sure the item identifier is ok.
      const OldItem* result = getItemFromItemIdentifier( ident );
      if ( ( result == NULL ) || ( result->getID() != item->getID() ) ) {
         retVal = false;
      }
   }

   return retVal;
}

uint32
OldGenericMap::updateOfficialNames()
{
   uint32 nbrChanged = 0;
   set<LangTypes::language_t> langs;
   for ( uint32 i = 0; i < getNbrNativeLanguages(); ++i ) {
      langs.insert(getNativeLanguage(i));
   }
   
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++ ) {
         OldItem* item = getItem( z, i );
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
                     mc2dbg4 << "[GMap]: Official itemname \""
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
OldGenericMap::updateNativeLanguages()
{
   // Copied to GenericMap
   // Keep the rules for deciding what is a native language in sync with
   // the rules in GenericMap.
   
   // Count nbr official names per languagae in this map
   // Key is native langauge, value is nbr occasions of the langauge
   typedef map<LangTypes::language_t, uint32> langMap_t;
   typedef langMap_t::iterator langMapIt_t;
   langMap_t nativeLanguages;

   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++ ) {
         OldItem* item = getItem( z, i );
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
   mc2dbg4 << "updateNativeLanguages: found "
           << nativeLanguages.size() << " langs with on" << endl;
   
   // Sort the native languages by order of occasions.
   multimap<uint32, LangTypes::language_t> langByNbrOccasions;
   uint32 totalNbrNames = 0;
   for ( langMapIt_t langMapIt = nativeLanguages.begin(); 
         langMapIt != nativeLanguages.end(); ++langMapIt ) {
      mc2dbg4 << "updateNativeLanguages: " << int(langMapIt->first)
              << ":" << LangTypes::getLanguageAsString(langMapIt->first)
              << " has " << langMapIt->second << " names" << endl;
      langByNbrOccasions.insert(
         make_pair(langMapIt->second, langMapIt->first));
      totalNbrNames += langMapIt->second;
   }
   mc2dbg4 << "updateNativeLanguages: totalNbrNames=" << totalNbrNames
           << endl;

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



uint32
OldGenericMap::expandNodeIDs( list<uint32>& nodeIDs ) const
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
OldGenericMap::getItemsWithName( const char* name, 
                              set<const OldItem*>& items,
                              ItemTypes::itemType itemType ) const
                              
{
   set<OldItem*> nonConstItems;
   getItemsWithName( name, 
                     nonConstItems,
                     itemType );
   set<OldItem*>::const_iterator it = nonConstItems.begin();
   while ( it != nonConstItems.end() ){
      items.insert(*it);
      ++it;
   }
}


void
OldGenericMap::getItemsWithName( const char* name, 
                              set<OldItem*>& items,
                              ItemTypes::itemType itemType ) const
                              
{
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++ ) {
         
         OldItem* item = getItem( z, i );
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
OldGenericMap::getDistanceToNode(byte nodeNbr, GfxData* gfx, int32 lat, int32 lon)
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


uint32 OldGenericMap::getMemoryUsage(void) const {
   // Calculate sizes of items
   uint32 sum = sizeof(OldGenericMap);
   for(uint i = 0; i < NUMBER_GFX_ZOOMLEVELS; i++ ) {
      uint32 index = 0;
      while ( index < this->getNbrItemsWithZoom(i) ) {              
         OldItem* curItem = this->getItem(i, index);
         if (curItem != NULL)
            sum += curItem->getMemoryUsage();
         index++;
      }
   }
   if ( m_gfxData != NULL )
      sum += m_gfxData->getMemoryUsage();
   if ( m_hashTable != NULL )
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
   for(uint32 i=0; i < NUMBER_GFX_ZOOMLEVELS; i++ ) {
      sum += m_itemsZoomAllocated[i] * sizeof(uint32);
   }

   if ( m_nativeLanguages != NULL )
      sum += m_nativeLanguages->getMemoryUsage();
   if ( m_currency != NULL )
      sum += m_currency->getMemoryUsage();

   return sum;
}


void 
OldGenericMap::dumpExternalConnections(uint32 toMapID) 
{
   if (m_segmentsOnTheBoundry != NULL) {
      m_segmentsOnTheBoundry->dump(toMapID);
   }
}

void 
OldGenericMap::printItemNames(uint32 itemID) 
{
   OldItem* item = itemLookup(itemID);
   mc2log << info;
   if (  (item != NULL) && 
         (item->getItemType() != ItemTypes::nullItem) ) {
      
      for(byte i=0; i<item->getNbrNames() - 1; i++) {
         mc2log << info << m_itemNames->getString(item->getStringIndex(i)) 
                << ", ";
      }
      mc2log << info << m_itemNames->getString(
                  item->getStringIndex(item->getNbrNames()-1));
   } else {
      mc2log << info << "(no item with id = " << itemID << ")";
   }
   mc2log << info << endl;
}

bool 
OldGenericMap::findEmptySlot(uint32 zoomlevel, uint32 &cnt) 
{
   // Increase cnt until an empty slot is found
   OldItem* curItem = NULL;
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
OldGenericMap::swapItems(uint32 aID, uint32 bID) 
{
   // NB! READ THE NOTE IN THE DOCUMENTAION (OldGenericMap.h)!!!

   // Find the items
   OldItem* aItem = itemLookup(aID);
   OldItem* bItem = itemLookup(bID);

   bool swapOK = true;
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
      swapOK = true;
   } else {
      // One of both of the items where NULL
      swapOK = false;
      return (false);
   }

   // Handle all the misc containers with items ids.
   // Keep in sync with removeItem
   if ( swapOK ) {
      // adminAreaCentres table
      MC2Coordinate aCoord = getCentreFromAdminTable(aID);
      MC2Coordinate bCoord = getCentreFromAdminTable(bID);
      if ( aCoord.isValid() || bCoord.isValid() ) {
         // remove the centre coord, and insert it for the other item instead
         set<uint32> rmCentres;
         adminAreaCentreTable_t newCentres;
         if ( aCoord.isValid() ) {
            mc2dbg8 << "swap centre a " << endl;
            rmCentres.insert(aID);
            newCentres.insert(make_pair(bID,aCoord));
         }
         if ( bCoord.isValid() ) {
            mc2dbg8 << "swap centre b " << endl;
            rmCentres.insert(bID);
            newCentres.insert(make_pair(aID,bCoord));
         }
         mc2dbg8 << "swap centres rmCentres=" << rmCentres.size()
                 << " newCentres=" << newCentres.size() << endl;

         removeAdminCenters( rmCentres );
         addAdminCenters( newCentres );
      }

      // item category table
      ItemMap< set<uint16> >::iterator catIt = m_itemCategories.find(aID);
      bool aPresent = false; bool bPresent = false;
      set<uint16> aCategories;
      set<uint16> bCategories;
      if ( catIt !=  m_itemCategories.end() ){
         aPresent = true;
         aCategories = catIt->second;
         m_itemCategories.erase(catIt);
      }
      catIt = m_itemCategories.find(bID);
      if ( catIt !=  m_itemCategories.end() ){
         bPresent = true;
         bCategories = catIt->second;
         m_itemCategories.erase(catIt);
      }
      mc2dbg8 << "swap categories a=" << aCategories.size()
              << " b=" << bCategories.size() << endl;
      if ( aPresent ) {
         bItem->addCategories( *this, aCategories );
      }
      if ( bPresent ) {
         aItem->addCategories( *this, bCategories );
      }

      // index area order table
      ItemMap< uint32 >::iterator indexAreaIt = m_indexAreasOrder.find(aID);
      uint32 aOrder = MAX_UINT32; uint32 bOrder = MAX_UINT32;
      if ( indexAreaIt != m_indexAreasOrder.end() ){
         aOrder = indexAreaIt->second;
         mc2dbg8 << " swap idex area order a " << aOrder << endl;
         m_indexAreasOrder.erase(indexAreaIt);
      }
      indexAreaIt = m_indexAreasOrder.find(bID);
      if ( indexAreaIt != m_indexAreasOrder.end() ){
         bOrder = indexAreaIt->second;
         mc2dbg8 << " swap idex area order b " << bOrder << endl;
         m_indexAreasOrder.erase(indexAreaIt);
      }
      if ( aOrder != MAX_UINT32 ) {
         setIndexAreaOrder(bID, aOrder );
      }
      if ( bOrder != MAX_UINT32 ) {
         setIndexAreaOrder(aID, bOrder );
      }

      // non searchable items table
      bool aNoSearch = false; bool bNoSearch = false;
      ItemMap< bool >::iterator nonSearchIt = 
         m_nonSearchableItems.find(aID);
      if ( nonSearchIt != m_nonSearchableItems.end() ){
         cout << " swap non search a " << endl;
         aNoSearch = true;
         m_nonSearchableItems.erase(nonSearchIt);
      }
      nonSearchIt = m_nonSearchableItems.find(bID);
      if ( nonSearchIt != m_nonSearchableItems.end() ){
         cout << " swap non search b " << endl;
         bNoSearch = true;
         m_nonSearchableItems.erase(nonSearchIt);
      }
      if ( aNoSearch ) {
         setItemNotInSearchIndex(bID);
      }
      if ( bNoSearch ) {
         setItemNotInSearchIndex(aID);
      }
      
      // road display class table
      ItemTypes::roadDisplayClass_t aRDClass = 
            ItemTypes::nbrRoadDisplayClasses;
      ItemTypes::roadDisplayClass_t bRDClass = 
            ItemTypes::nbrRoadDisplayClasses;
      ItemMap< uint32 >::iterator dispIt = m_roadDisplayClass.find(aID);
      if ( dispIt != m_roadDisplayClass.end() ){
         aRDClass = ItemTypes::roadDisplayClass_t(dispIt->second);
         mc2dbg8 << " swap road disp class a " << int(aRDClass) << endl;
         m_roadDisplayClass.erase(dispIt);
      }
      dispIt = m_roadDisplayClass.find(bID);
      if ( dispIt != m_roadDisplayClass.end() ){
         bRDClass = ItemTypes::roadDisplayClass_t(dispIt->second);
         mc2dbg8 << " swap road disp class b " << int(bRDClass) << endl;
         m_roadDisplayClass.erase(dispIt);
      }
      if ( aRDClass != ItemTypes::nbrRoadDisplayClasses ) {
         setRoadDisplayClass(bID, aRDClass );
      }
      if ( bRDClass != ItemTypes::nbrRoadDisplayClasses ) {
         setRoadDisplayClass(aID, bRDClass );
      }

      // area feature draw display class table
      ItemTypes::areaFeatureDrawDisplayClass_t aAFDDClass = 
            ItemTypes::nbrAreaFeatureDrawDisplayClasses;
      ItemTypes::areaFeatureDrawDisplayClass_t bAFDDClass =
            ItemTypes::nbrAreaFeatureDrawDisplayClasses;
      dispIt = m_areaFeatureDrawDisplayClass.find(aID);
      if ( dispIt != m_areaFeatureDrawDisplayClass.end() ){
         aAFDDClass = ItemTypes::areaFeatureDrawDisplayClass_t(dispIt->second);
         mc2dbg8 << " swap area draw disp class a " << int(aAFDDClass) << endl;
         m_areaFeatureDrawDisplayClass.erase(dispIt);
      }
      dispIt = m_areaFeatureDrawDisplayClass.find(bID);
      if ( dispIt != m_areaFeatureDrawDisplayClass.end() ){
         bAFDDClass = ItemTypes::areaFeatureDrawDisplayClass_t(dispIt->second);
         mc2dbg8 << " swap area draw disp class b " << int(bAFDDClass) << endl;
         m_areaFeatureDrawDisplayClass.erase(dispIt);
      }
      if ( aAFDDClass != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
         setAreaFeatureDrawDisplayClass(bID, aAFDDClass, false );
      }
      if ( bAFDDClass != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
         setAreaFeatureDrawDisplayClass(aID, bAFDDClass, false );
      }

   }

   return (swapOK);
}

uint32
OldGenericMap::getConnection( uint32 fromItemID, uint32 toItemID,
                           OldConnection*& conn ) const
{
   OldRouteableItem* toRI =  dynamic_cast<OldRouteableItem*>
                                 (itemLookup(toItemID));
   if (toRI == NULL) {
      // Wrong ssi id specified
      conn = NULL;
      return (MAX_UINT32);
   }
   
   // Check connection from both nodes
   for (uint16 i = 0; i < 2; i++) {
      OldNode* toNode = toRI->getNode(i);
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
OldGenericMap::getTotalNbrItemNames() const
{
   return (m_itemNames->getNbrStrings());
}


bool
OldGenericMap::getConnectionData(OldConnection* conn, OldNode* toNode,
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
            OldNode* curNode = nodeLookup( *nextIt );
            OldConnection* curConn = curNode->getEntryConnectionFrom( *it );

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
OldGenericMap::getSingleConnectionLength(OldConnection* conn, OldNode* toNode,
                                      float64& length, 
                                      bool externalConnection) const
{
   // Get to and from segments.
   
   OldRouteableItem* toSegment = static_cast<OldRouteableItem*> 
      (itemLookup(toNode->getNodeID() & 0x7FFFFFFF));
   
    OldRouteableItem* fromSegment;
   if (!externalConnection) { 
      fromSegment = static_cast<OldRouteableItem*> 
         (itemLookup(conn->getConnectFromNode() & 0x7FFFFFFF));
   } else {
      // External connection. 
      // The length of the external connection is 0 since
      // there is a 0-length segment on each side of the map boundary.
      length = 0;
      return true;
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
      OldBusRouteItem* bri = static_cast<OldBusRouteItem*> (toSegment);
      length = calcBusSSILength(bri, 
            fromSegment->getGfxData()->getLength(0),
            ( conn->getConnectFromNode() & 0x80000000 ) == 0);
      
   }
   else if ((fromSegment->getItemType() == ItemTypes::busRouteItem) &&
            (toSegment->getItemType() == ItemTypes::streetSegmentItem)) {
         
         // bus -> ssi
      OldBusRouteItem* bri = static_cast<OldBusRouteItem*> (fromSegment);
      length = calcBusSSILength(bri, toSegment->getGfxData()->getLength(0), 
                                toNode->isNode0());
   
   }   
   else if ((fromSegment->getItemType() == ItemTypes::busRouteItem) &&
            (toSegment->getItemType() == ItemTypes::busRouteItem)) {
      
      // bus -> bus
      OldBusRouteItem* fromBRI = static_cast<OldBusRouteItem*> (fromSegment);
      OldBusRouteItem* toBRI = static_cast<OldBusRouteItem*> (toSegment);
      
      // Check that it is an actual bus change.
      if (fromBRI->getBusRouteID() != toBRI->getBusRouteID()) {
         
         // Find the connecting streetsegment.
         uint16 i = 0;
         OldStreetSegmentItem* connectingSSI = NULL;

         while (  (i < fromSegment->getNode(0)->getNbrConnections()) &&
                  (connectingSSI == NULL) ) {
            uint32 itemID = 
               fromSegment->getNode(0)->getEntryConnection(i)
                  ->getConnectFromNode() & 0x7FFFFFFF;
            connectingSSI = dynamic_cast<OldStreetSegmentItem*> 
               (itemLookup(itemID));
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
OldGenericMap::getSingleConnectionTime(OldConnection* conn, OldNode* toNode,
                                    float64& time, float64& standStillTime,
                                    float64 length,
                                    bool externalConnection ) const
{
   OldRouteableItem* toRI = dynamic_cast<OldRouteableItem*> 
      (itemLookup(toNode->getNodeID() & 0x7FFFFFFF));
   
   OldRouteableItem* fromRI;
   OldNode* fromNode;
   if (!externalConnection) { 
      fromRI = dynamic_cast<OldRouteableItem*> 
         (itemLookup(conn->getConnectFromNode() & 0x7FFFFFFF));
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
                  OldConnection::STANDSTILLTIME_LEFTTURN_MAJOR_ROAD;
               mc2dbg4 << "StandStillTime = LEFT (MAJOR)" << endl;
            } else {
               standStillTime = 
                  OldConnection::STANDSTILLTIME_LEFTTURN_MINOR_ROAD;
               mc2dbg4 << "StandStillTime = LEFT (MINOR)" << endl;
            }
            break;
          case (ItemTypes::UTURN) :
            // If the u-turn is made on large roads, 
            // the standstill time will be higher.
            if ((fromRI->getRoadClass() <= 2) || 
                (toRI->getRoadClass() <= 2)) {
               standStillTime = 
                  OldConnection::STANDSTILLTIME_U_TURN_MAJOR_ROAD;
               mc2dbg4 << "StandStillTime = UTURN (MAJOR)" << endl;
            } else {
               standStillTime = 
                  OldConnection::STANDSTILLTIME_U_TURN_MINOR_ROAD;
               mc2dbg4 << "StandStillTime = UTURN (MINOR)" << endl;
            }
            break;
        case (ItemTypes::RIGHT) :
	    // If the left turn is made on large roads,
	    // the standstill time will be higher.
	    if ((fromRI->getRoadClass() <= 2) &&
	        (toRI->getRoadClass() <= 2)) {
               standStillTime = 
                  OldConnection::STANDSTILLTIME_RIGHTTURN_MAJOR_ROAD;;
               mc2dbg4 << "StandStillTime = RIGHT (MAJOR)" << endl;
	    } else {
	       standStillTime =
	          OldConnection::STANDSTILLTIME_RIGHTTURN_MINOR_ROAD;
	       mc2dbg4 << "StandStillTime = RIGHT (MINOR)" << endl;
	    }
            break;
         case (ItemTypes::ENTER_ROUNDABOUT) :
            standStillTime = 
               OldConnection::STANDSTILLTIME_ENTER_ROUNDABOUT;
            mc2dbg4 << "StandStillTime = ENTER ROUNDABOUT" << endl;
            break;
         case (ItemTypes::ENTER_FERRY) :
            standStillTime = 
               OldConnection::STANDSTILLTIME_ENTER_FERRY;
            mc2dbg4 << "StandStillTime = ENTER FERRY" << endl;
            break;
         case (ItemTypes::EXIT_FERRY) :
            standStillTime = 
               OldConnection::STANDSTILLTIME_EXIT_FERRY;
            mc2dbg4 << "StandStillTime = EXIT FERRY" << endl;
            break;
         case (ItemTypes::CHANGE_FERRY) :
            standStillTime = 
               OldConnection::STANDSTILLTIME_CHANGE_FERRY;
            mc2dbg4 << "StandStillTime = CHANGE FERRY" << endl;
            break;
         default:
            standStillTime = 0;
            break;
      }
   }

   time = length / float64(Math::KMH_TO_MS * speedLimit);
   
   OldStreetSegmentItem* fromSSI = dynamic_cast<OldStreetSegmentItem*> (fromRI);
   if (fromSSI != NULL) {
      time = time * conn->getPenaltyFactor(fromSSI->getRoadClass(),
                                           fromSSI->isRamp(),
                                           speedLimit);
   }
   
   return (true);

}
   

bool
OldGenericMap::getConnectionCost(OldConnection* conn, OldNode* toNode,
                              uint32& costA, uint32& costB,
                              uint32& costC, uint32& costD,
                              bool externalConnection) const
{
   float64 t, sst, length;
   if (! getConnectionData( conn, toNode, t, sst, length, 
                            externalConnection ) ) {
      return false;
   }

   costB = costC = uint32( rint( OldConnection::secToTimeCost( t + sst ) ) );
   
   costA = uint32( rint( OldConnection::metersToDistCost( length ) ) );
   costD = 0;
   
   return (true);
}

bool
OldGenericMap::writeItemMifHeader( ofstream& mifFile,
                               ItemTypes::itemType headerTypeToWrite )
{

   switch ( headerTypeToWrite ) {
      case ItemTypes::streetSegmentItem : {
         OldStreetSegmentItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::municipalItem : {
         OldMunicipalItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::waterItem : {
         OldWaterItem::writeMifHeader(mifFile);   
      } break;
      case ItemTypes::parkItem : {
         OldParkItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::forestItem : {
         OldForestItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::buildingItem : {
         OldBuildingItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::railwayItem : {
         OldRailwayItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::islandItem : {
         OldIslandItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::streetItem : {
         OldStreetItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::builtUpAreaItem : {
         OldBuiltUpAreaItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::cityPartItem : {
         OldCityPartItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::airportItem : {
         OldAirportItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::aircraftRoadItem : {
         OldAircraftRoadItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::militaryBaseItem : {
         OldMilitaryBaseItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::individualBuildingItem : {
         OldIndividualBuildingItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::subwayLineItem : {
         OldSubwayLineItem::writeMifHeader(mifFile);
      } break;
      case ItemTypes::cartographicItem : {
         OldCartographicItem::writeMifHeader(mifFile);
      } break;
      default : {
         return false;
      }
   }
   
   return true;
}

void
OldGenericMap::printMidMif(const char* outName, ItemTypes::itemType type,
                        bool writeMifHeader)
{
   uint32 sentItems = 0;
   char* midFileName = new char[strlen(outName)+5];
   char* mifFileName = new char[strlen(outName)+5];

   //Give the files names and create files to write to.

   strcpy(midFileName, outName);
   strcpy(mifFileName, outName);
   strcat(midFileName, ".mid");
   strcat(mifFileName, ".mif");
   ofstream midFile(midFileName, ios::app);
   ofstream mifFile(mifFileName, ios::app);
   delete midFileName;
   delete mifFileName;
   
   //Write header for mif-file
   if (writeMifHeader) {
      if ( ! writeItemMifHeader(mifFile, type) ) {
         mc2log << warn <<"OldGenericMap::printMidMif writeItemMifHeader "
                << "- Type Not Handled" << endl;
         return;
      }
   }
                    
   // Loop through all zoom-levels and items and write asked for items to 
   // mid- and mif-file. There must be a gfxData to write the mif file!
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++){
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++){
         OldItem* curItem = getItem(z, i);

         bool wantedItem = true;
         
         if ( (curItem != NULL) && (curItem->getItemType() == type) &&
              (curItem->getGfxData() != NULL) &&
               wantedItem ) {
            curItem->printMidMif(midFile, mifFile, m_itemNames);
            
            if (type == ItemTypes::waterItem) {
               //mapID useful when printing water
               midFile << "," << getMapID() << endl;
            }
            
            sentItems++;
         }
      }
   }
   mc2log << info << "Items printed to mid/mif-file: " << sentItems << endl;
}

void
OldGenericMap::printMidMifMapGfxData(
                     char* fileName, bool municipals, bool writeMifHeader)
{
   // Open files for writing.
   char* midFileName = new char[strlen(fileName)+5];
   char* mifFileName = new char[strlen(fileName)+5];

   strcpy(midFileName, fileName);
   strcpy(mifFileName, fileName);
   strcat(midFileName, ".mid");
   strcat(mifFileName, ".mif");
   ofstream midfile(midFileName, ios::app);
   ofstream miffile(mifFileName, ios::app);
   delete midFileName;
   delete mifFileName;

   if (writeMifHeader) {
      miffile << "VERSION 300" << endl
              << "Charset \"WindowsLatin1\"" << endl
              << "DELIMITER \",\"" << endl
              << "COORDSYS mc2" << endl;
   }

   if (!municipals) {
      
      // print the map gfxData
      mc2dbg << "Printing map gfxdata for map " << getMapID() << endl;
      if (writeMifHeader) {
         miffile << "COLUMNS " << 2 << endl
                 << "  MAP_ID integer(16,0)" << endl
                 << "  MAP_NAME char(70)" << endl
                 << "DATA" << endl;
      }
      
      dynamic_cast<GMSGfxData*>(m_gfxData)->printMif(miffile);
      midfile << getMapID() << "," << getMapName() << endl;

      
   } else {

      // print the gfxdata of municipals, build convex hull if not present.
      mc2dbg << "Printing municipal gfxdatas for map " << getMapID() << endl;
      if (writeMifHeader) {
         miffile << "COLUMNS " << 4 << endl
                 << "  MAP_ID integer(16,0)" << endl
                 << "  MAP_NAME char(70)" << endl
                 << "  MUN_ID integer(16,0)" << endl
                 << "  NAME char(70)" << endl
                 << "DATA" << endl;
      }

      //count nbr municipals
      uint32 nbrMun = 0;
      for (uint32 i = 0; i < getNbrItemsWithZoom(0); i++) {
         OldItem* item = getItem(0,i);
         if ((item != NULL) &&
             (item->getItemType() == ItemTypes::municipalItem)) {
            nbrMun++;
         }
      }
      mc2dbg << "The map " << getMapID() << " has " << nbrMun 
             << " municipals" << endl;
      uint32 munID[nbrMun];
      uint32 tmp = 0;
      for (uint32 i = 0; i < getNbrItemsWithZoom(0); i++) {
         OldItem* item = getItem(0,i);
         if ((item != NULL) &&
             (item->getItemType() == ItemTypes::municipalItem)) {
            munID[tmp] = item->getID();
            tmp++;
         }
      }

      for (uint32 mun = 0; mun < nbrMun; mun++) {
         
         OldItem* munItem = itemLookup(munID[mun]);
         if (munItem->getGfxData() != NULL) {
            munItem->getGfxData()->printMif(miffile);
            midfile << getMapID() << ","
                    << getMapName() << ","
                    << munID[mun] << ","
                    << getFirstItemName(itemLookup(munID[mun])) << endl;
            
         } else {

            mc2dbg << "Build convex hull gfxdata for municipal id="
                   << munID[mun] << " name=" << getFirstItemName(munID[mun])
                   << endl;
            GfxDataFull* allSSI = GMSGfxData::createNewGfxData( static_cast<OldGenericMap* >( NULL ), true );

            uint32 nbrAddedCoords = 0;
            for (uint32 z=0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
               for ( uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
                  OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
                     (getItem(z, i));
                  if ( (ssi != NULL) && 
                       (getRegionID(
                           ssi, ItemTypes::municipalItem) == munID[mun]) && 
                       (ssi->getGfxData() != NULL) ) {
                     GMSGfxData* tmpGfx = ssi->getGfxData();
                     allSSI->addCoordinate(tmpGfx->getLat(0,0), 
                                           tmpGfx->getLon(0,0));
                     allSSI->addCoordinate(tmpGfx->getLastLat(0), 
                                           tmpGfx->getLastLon(0));
                     nbrAddedCoords +=2;
                  }
               }
            }
            mc2dbg4 << " Added " << nbrAddedCoords << " coords" << endl;
            
            // Create convex hull out of allSSI
            GMSGfxData* convHullGfx = 
               GMSGfxData::createNewGfxData( static_cast<OldGenericMap* >( NULL ), true );

            Stack stack;
            if (allSSI->getConvexHull(&stack, 0)) {
               uint32 size = stack.getStackSize();
               for (uint32 i = 0; i < size; i++) {
                  uint32 idx = stack.pop();
                  convHullGfx->addCoordinate( allSSI->getLat(0,idx),
                                              allSSI->getLon(0,idx) );
               }
               convHullGfx->setClosed(0, true);
            } else {
               // Could not create convex hull...
               delete convHullGfx;
               convHullGfx = NULL;
            }
            delete allSSI;

            if (convHullGfx != NULL) {
               convHullGfx->printMif(miffile);
               midfile << getMapID() << ","
                       << getMapName() << ","
                       << munID[mun] << ","
                       << getFirstItemName(itemLookup(munID[mun])) << endl;
            }
         }
      }
   }
}

float64
OldGenericMap::calcBusSSILength(OldBusRouteItem* bri, float64 dist,
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


OldNode*
OldGenericMap::getNodeFromCoordinates(int32 lat1, int32 lon1,
                                   int32 lat2, int32 lon2)
{
   const uint32 FIND_NODE_MAX_DIST_METER = 5;

   // Initiate the hash-table
   m_hashTable->clearAllowedItemTypes();
   m_hashTable->addAllowedItemType(ItemTypes::ferryItem);
   m_hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);

   // Find item candidates for the first coordinate
   bool killNodeVector = false;
   Vector* nodeCandidates = m_hashTable->getAllWithinRadius_meter(
                                                   lon1,
                                                   lat1,
                                                   FIND_NODE_MAX_DIST_METER,
                                                   killNodeVector);
   
   // Make sure that we have some candidates
   if ( (nodeCandidates == NULL) || (nodeCandidates->getSize() == 0) ) {
      mc2dbg2 << here << " no node coord1 candidates... " << endl;
      if (killNodeVector)
         delete nodeCandidates;
      return NULL;
   }


   // Find the item among the nodeCandidates
   // that is closest to the second coordinate
   uint64 sqMinDist = MAX_UINT64;
   uint32 nodeCandidate = MAX_UINT32;
   for (uint32 i = 0; i < nodeCandidates->getSize(); i++) {
      uint32 id = nodeCandidates->getElementAt(i);
      OldItem* item = itemLookup(id);
      uint64 sqDist = item->getGfxData()->squareDistToLine_mc2(lat2, lon2);
      mc2dbg8 << "cand[" << i << "] " << id << " sqdist=" << sqDist << endl;
      if (sqDist < sqMinDist) {
         sqMinDist = sqDist;
         nodeCandidate = id;
      }
   }
   mc2dbg4 << " cand=" << nodeCandidate << " sqMinDist=" << sqMinDist << endl;
   DEBUG4(
      nodeCandidates->dump();
      mc2dbg4 << "nodeCandidate=" << nodeCandidate << endl;
   );
   if (killNodeVector)
      delete nodeCandidates;

   // Make sure that distance to coord2 is less than FIND_NODE_MAX_DIST_METER
   float64 sqMinDistMeters =
         int(::sqrt(sqMinDist)) * GfxConstants::MC2SCALE_TO_METER;
   
   if ( (nodeCandidate != MAX_UINT32) && 
        ( sqMinDistMeters < FIND_NODE_MAX_DIST_METER) ) {
      // Found routeable item, now find node (dist is meters squared)
      OldRouteableItem* rItem = static_cast<OldRouteableItem*>
                                          (itemLookup(nodeCandidate));
      float64 dist0 = getDistanceToNode(0, rItem->getGfxData(), lat1, lon1);
      float64 dist1 = getDistanceToNode(1, rItem->getGfxData(), lat1, lon1);
      mc2dbg8 << "   dist0=" << dist0 << ", dist1=" << dist1 << endl;

      const float64 SAFE_SQ_DISTANCE = 9;
      if ( (dist0 < dist1) && (dist1 > SAFE_SQ_DISTANCE)) {
         // OldNode 0 is closer than node 1
         mc2dbg8 << "      case 1" << endl;
         return rItem->getNode(0);
      } else if ( (dist1 < dist0) && (dist0 > SAFE_SQ_DISTANCE)) {
         // OldNode 1 is closer than node 0
         mc2dbg8 << "      case 2" << endl;
         return rItem->getNode(1);
      } else if ((dist0 <= SAFE_SQ_DISTANCE) && (dist1 <= SAFE_SQ_DISTANCE)) {
         // Both nodes close, use offset
         int32 offset = getItemOffset(rItem, lat2, lon2);
         if (offset < MAX_UINT16/2) {
            // OldNode 0 closest
            mc2dbg8 << "      case 3, offset=" << offset << endl;
            return rItem->getNode(0);
         } else {
            // OldNode 1 closest
            mc2dbg8 << "      case 4, offset=" << offset << endl;
            return rItem->getNode(1);
         }
      } else  if  (dist0 < dist1) {
         // OldNode 0 is closer than node 1
         mc2dbg8 << "      case 5" << endl;
         return rItem->getNode(0);
      } else {
         // OldNode 1 is closer than node 0
         mc2dbg8 << "      case 6" << endl;
         return rItem->getNode(1);
      }
   }

   return NULL;
}


bool
OldGenericMap::getCoordinatesFromNode(const OldNode* node,
                                   int32 &lat1, int32 &lon1,
                                   int32 &lat2, int32 &lon2)
{
   // Get the coordinates of the node.
   uint32 nodeID = node->getNodeID();
   uint16 offset = 0;
   bool retVal = getNodeCoordinates(nodeID, lat1, lon1);
   
   if (retVal) {
      // Get the coordinates on the routeable item
      const uint32 DIST_ON_SSI_METER = 10;
      OldRouteableItem* rItem = static_cast<OldRouteableItem*>
                               (itemLookup(nodeID & 0x7fffffff));
      MC2_ASSERT(rItem->getGfxData()->getNbrPolygons() == 1);
      float64 totLength = rItem->getGfxData()->getLength(0);
      float64 offsetFloat = DIST_ON_SSI_METER / totLength;
      if (offsetFloat > 0.45){
         offsetFloat = 0.45;
         mc2dbg4 << "More than 45% in on the item, using 45%" << endl;
      }
      if ((nodeID & 0x80000000) == 0x80000000) {
         // OldNode 1 closest, reverse offsetFloat
         offsetFloat = 1 - offsetFloat;
      }

      offset = uint16(offsetFloat * MAX_UINT16);
      retVal = getItemCoordinates(rItem->getID(), offset, lat2, lon2);
   }

   // Check that a reverse-lookup gives the correct node
   if (retVal) {
      OldNode* revNode = getNodeFromCoordinates(lat1, lon1, lat2, lon2);
      if ((revNode == NULL) || (revNode->getNodeID() != node->getNodeID())) {
         mc2log << error << "OldGenericMap::getCoordinatesFromNode Failed to "
                << "reverse-lookup node. Algorithm must be modified!"
                << "   (lat1,lon1)=(" << lat1 << "," << lon1 
                << "), (lat2,lon2)=(" << lat2 << "," << lon2 << ")" << endl
                << "   offset=" << offset << endl
                << "   nodeID=" << nodeID << "=0x" << hex << nodeID << dec;
         if (revNode != NULL) {
            mc2log << ", revNodeID=" << revNode->getNodeID() << "=0x" << hex
                   << revNode->getNodeID() << dec;
         }
         mc2log << endl;

         retVal = false;
      }
   }

   return (retVal);
}

bool
OldGenericMap::itemHasNameAndType(OldItem* item,
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

OldItem* 
OldGenericMap::getItemFromCoordinate(ItemTypes::itemType type,
                                  int32 lat, int32 lon,
                                  vector<OldItem*>& closeItems,
                                  const char* itemName,
                                  const int r,
                                  ItemTypes::pointOfInterest_t poiType)
{
   mc2dbg8 << "Testing coord: " << lat << "," << lon << endl;
   // Get an item from the hash table
   m_hashTable->clearAllowedItemTypes();
   if (type == ItemTypes::pointOfInterestItem) {
      // If the searched for poi has gfxData it can be found this way.
      // If no poi fond use street segments below.
      m_hashTable->addAllowedItemType(ItemTypes::pointOfInterestItem);
   } else if (type == ItemTypes::streetItem) {
      // because streets have no gfxData
      m_hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);
      mc2dbg8 << "Added SSI as allowed type for street" << endl;
   } else {
      m_hashTable->addAllowedItemType(type);
      mc2dbg8 << "Added " << uint32(type) << " as allowed type" << endl;
   }
   bool shouldKill = false;
   int radius = r;
   if (radius < 1) {
      radius = abs(r);
   }
   Vector* closeIDs = m_hashTable->getAllWithinRadius_meter(lon, 
                                                            lat, 
                                                            radius, 
                                                            shouldKill);
   // If looking for pois and found none, try to use ssi to find them
   bool findPoiBySsi = false;
   if ((type == ItemTypes::pointOfInterestItem) && 
       ((closeIDs == NULL) || (closeIDs->getSize() == 0))) {
      m_hashTable->clearAllowedItemTypes();
      m_hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);
      mc2dbg8 << "Added SSI as allowed type for poi" << endl;
      findPoiBySsi = true;
      closeIDs = m_hashTable->getAllWithinRadius_meter(
                        lon, lat, radius, shouldKill);
   }

   if (closeIDs == NULL) {
      shouldKill = true;
      closeIDs = new Vector();
   }
   mc2dbg4 << "Found " << closeIDs->getSize() << " close item(s)" << endl;
   

   // No close items 
   if ( (closeIDs == NULL) || (closeIDs->getSize() == 0))
      return NULL;

   // Dump closeIDs
   DEBUG8(
   for (uint32 i=0; i<closeIDs->getSize(); i++) {
      mc2dbg << here << " closeIDs[" << i << "]=" 
             << closeIDs->getElementAt(i) << endl;
   });

   // Check the distances from the coord to the found item(s)
   // If the inparam radius was negative and the dist is larger than 0,
   // it is not precise enough to find the correct item
   for (int32 i=closeIDs->getSize()-1; i >= 0; i--) {
      OldItem* item = itemLookup(closeIDs->getElementAt(i));
      GfxData* testGfx = item->getGfxData();
      if ( testGfx != NULL ) {
         float64 meters = 1000;
         if ( testGfx->getClosed(0) ) {
            // polygon
            int64 sqDistMeters = testGfx->signedSquareDistTo(lat, lon);
            if ( sqDistMeters >= 0 ) {
               // coord is on the border or outside the gfx
               meters = ::sqrt(sqDistMeters);
               if ( meters > 0 ) {
                  // outside, not good
                  meters = 10;
               }
            } else {
               // coord is inside the gfx
               meters = 0;
            }
            
         } else {
            // line item, check if close enough
            uint64 sqDist = testGfx->squareDistToLine_mc2(lat, lon);
            meters = ::sqrt(sqDist)*GfxConstants::MC2SCALE_TO_METER;
         }
         mc2dbg4 << "sqDist for " << item->getID() << " is "
                 << meters << " meters" << endl;
         if ( (r < 1) && (meters > 0.5) ) {
            // this item is not good, the distance from the coord to 
            // the item is too large to be the correct one.
            mc2dbg << "getItemFromCoordinate candidate " << item->getID()
                   << " is not close enough " << meters << endl;
            closeIDs->removeElementAt(i);
         }
      }
   }
   mc2dbg8 << "Found " << closeIDs->getSize() << " close item(s)" << endl;
   
   // Lookup the close items
   closeItems.clear();
   closeItems.reserve(closeIDs->getSize());
   if ((type == ItemTypes::pointOfInterestItem) && findPoiBySsi) {
      // Find the POIs that are located on the ssi's in closeIDs
      //      This code is more or less copied from MapProcessor 
      //      (when handling PACKETTYPE_ITEMINFO_REQUEST's)
      const uint32 z = uint32(ItemTypes::pointOfInterestItem);
      for (uint32 i=0; i<closeIDs->getSize(); i++) {
         OldItem* item = itemLookup(closeIDs->getElementAt(i));
         if ( (item != NULL) && 
              (item->getItemType() == ItemTypes::streetSegmentItem)) {
            // Get the id of the current ssi
            const uint32 ssiID = item->getID();
            const int deltaOffset = 100;
            // Calculate offset
            uint16 maxOffset = MAX_UINT16;
            uint16 minOffset = 0;
            // Caclulate allowed offset-interval
            int tmp = getItemOffset(ssiID, lat ,lon);
            if (tmp >=0 ) {
               maxOffset = uint16(MIN(MAX_UINT16, tmp+deltaOffset));
               minOffset = uint16(MAX(0, tmp-deltaOffset));
            } else {
               mc2log << warn << here << " Failed to get offset for ssi="
                      << ssiID << " (" << lat << "," << lon << ")" << endl;
            }
            
            // Find the POI's the naive-way :(
            for (uint32 j=0; j<getNbrItemsWithZoom(z); ++j) {
               OldPointOfInterestItem* poi = 
                  dynamic_cast<OldPointOfInterestItem*>(getItem(z, j));
               if ( (poi != NULL) && 
                    (poi->getStreetSegmentItemID() == ssiID) && 
                    ( (poiType == ItemTypes::nbr_pointOfInterest) ||
                      (poiType == poi->getPointOfInterestType()) ) ) {
                  if ( (poi->getOffsetOnStreet() >= minOffset) &&
                       (poi->getOffsetOnStreet() <= maxOffset) ) {
                     // Found poi, add to closeItems
                     mc2dbg4 << "Adding POI-candidate, ID="
                             << poi->getID() << ", name=" 
                             << getFirstItemName(poi) << ", offset=" 
                             << poi->getOffsetOnStreet() << ", SSI=" 
                             << getFirstItemName(poi->getStreetSegmentItemID()) 
                             << "=" << poi->getStreetSegmentItemID() << endl;
                     closeItems.push_back(poi);
                  } else {
                     mc2dbg4 << "Wrong offset, ID=" << poi->getID()
                             << ", POI-offset=" 
                             << poi->getOffsetOnStreet() << "min=" 
                             << minOffset << ", max=" << maxOffset 
                             << ", name=" << getFirstItemName(poi) 
                             << ", SSI=" 
                             << getFirstItemName(poi->getStreetSegmentItemID()) 
                             << "=" << poi->getStreetSegmentItemID() << endl;
                  }
               }
            }
         }
      }
   } else if ((type == ItemTypes::pointOfInterestItem) && !findPoiBySsi) {
      for (uint32 i=0; i<closeIDs->getSize(); i++) {
         OldPointOfInterestItem* poi = dynamic_cast<OldPointOfInterestItem*>
                  (itemLookup(closeIDs->getElementAt(i)));
         if ( (poi != NULL) && 
              ( (poiType == ItemTypes::nbr_pointOfInterest) ||
                (poiType == poi->getPointOfInterestType()) ) ) {
            mc2dbg4 << " Adding poi to closeItems: " << poi->getID() << endl;
            closeItems.push_back(poi);
         }
      }
      
   } else if (type == ItemTypes::streetItem) {
      // Find the street for any found ssi
      for (uint32 i=0; i<closeIDs->getSize(); i++) {
         OldItem* ssi = itemLookup(closeIDs->getElementAt(i));
         if ((ssi != NULL) && isPartOfStreet(ssi)) {
            mc2dbg4 << "Close ssi " << ssi->getID() << ", part of " 
                    << int(ssi->getNbrGroups()) << " groups." << endl;

            for (uint32 g=0; g<ssi->getNbrGroups(); g++) {
               uint32 group = ssi->getGroup(g);
               OldItem* street = itemLookup(group);
               if ( (street != NULL) && 
                    (street->getItemType() == ItemTypes::streetItem)) {
                  // we have the street for the ssi found
                  mc2dbg4 << here << " Adding item to closeItems: " 
                          << street->getID() << endl;
                  closeItems.push_back(street);
               }
            }

         } else if (ssi != NULL) {
            mc2log << warn << "Found close ssi for street " << ssi->getID() 
                   << ", but the ssi is not part of any group." << endl;
         }
      }
   } else {
      // Find the items with the itemID's in closeIDs
      for (uint32 i=0; i<closeIDs->getSize(); i++) {
         OldItem* item = itemLookup(closeIDs->getElementAt(i));
         if (item != NULL) {
            mc2dbg4 << here << " Adding item to closeItems: "
                    << item->getID() << endl;
            closeItems.push_back(item);
         }
      }
   }
   
   OldItem* returnItem = NULL;
   if (closeItems.size() == 1) {
      // Only one item close to the given coordinate
      // If a special name is requested check if the item has this name.
      if (itemName != NULL) {
         for (uint32 j=0; j<closeItems[0]->getNbrNames(); j++) {
            if (StringUtility::strcmp(
                            itemName, 
                            getName(closeItems[0]->getStringIndex(j))) == 0) {

               returnItem = closeItems[0];
               mc2dbg4 << here << " Found one item: " 
                       << getFirstItemName(returnItem) << endl;
            }
         }
      // If no name given, this is the correct item!
      } else {
         returnItem = closeItems[0];
         mc2dbg4 << here << " Found one item: " 
                 << getFirstItemName(returnItem) << endl;
      }
            
   } else if (closeItems.size() > 1) {
      if (itemName != NULL) {
         // More than one close item, check all the names to find one unique!
         // Make this a while loop?
         mc2dbg4 << here << " Found " << closeItems.size() << " items" << endl;
         bool found = false;
         uint32 firstReturnItemId = MAX_UINT32;
         for (uint32 i=0; i<closeItems.size(); i++) {
            OldItem* closeItem = closeItems[i];
            for (uint32 j = 0; j < closeItem->getNbrNames(); j++) {
               if (StringUtility::strcmp(
                     itemName,
                     getName(closeItem->getStringIndex(j))) == 0) {
                  // Got matching name, good if not already found
                  if (found) {
                     // One of the items found in closeItems may have 
                     // several names with the same namestring, which is ok.
                     if ( closeItem->getID() != firstReturnItemId ) {
                        mc2log << warn << here << " Ambigous items found"
                               << " before=" << firstReturnItemId << " now="
                               << closeItem->getID() << " (name " << itemName
                               << "), totally " << closeItems.size()
                               << " close items." << endl;
                        for (uint32 k=0; k<closeItems.size(); k++) {
                           mc2dbg << " Item " << k << ": " 
                                  << closeItems[k]->getID() << endl;
                        }
                        returnItem = NULL;
                     }
                  } else {
                     found = true;
                     returnItem = closeItems[i];
                     firstReturnItemId = returnItem->getID();
                     mc2dbg4 << "Found item" << endl;
                  }
               }
            }
         }
      }

      // Ok, did not find a single item using name either. Try which item
      // is really closest to the requested coordinate.
      if ( returnItem == NULL ) {
         
         int64 sqMinDist = MAX_UINT64;
         uint32 candidate = MAX_UINT32;
         for (uint32 i = 0; i < closeItems.size(); i++) {
            OldItem* testItem = closeItems[i];
            GfxData* testGfx = testItem->getGfxData();
            if ( testGfx != NULL ) {
               int64 sqDist = testGfx->signedSquareDistTo_mc2(lat, lon);
               /*
               // another method directly separating the closed from non-closed
               // polygons, not necessary?
               uint64 sqDist = 1000000;
               if ( testGfx->getClosed(0) ) {
                  // polygon
                  uint64 sqDistMeters = testGfx->squareDistTo(lat, lon);
                  sqDist = uint64 (sqDistMeters * 
                              GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
               } else {
                  // line
                  sqDist = testGfx->squareDistToLine_mc2(lat, lon);
               }*/
               mc2dbg8 << "itemcand[" << i << "] " << testItem->getID()
                       << " sqDist=" << sqDist << endl;
               if (sqDist < sqMinDist) {
                  sqMinDist = sqDist;
                  candidate = i;
               }
            }
         }

         if ( candidate != MAX_UINT32 ) {
            // Check if the test item has the provided itemName
            returnItem = closeItems[candidate];
            mc2dbg1 << "Candidate " << candidate << " itemId="
                    << returnItem->getID() << " closest to coord" << endl;
            if ( itemName != NULL ) {
               uint32 strIdx;
               if (!itemHasNameAndType(returnItem, LangTypes::invalidLanguage,
                        ItemTypes::invalidName, itemName, strIdx) ) {
                  mc2log << warn << "The closest candidate item did not have "
                         << "the correct itemName " << itemName << endl;
                  returnItem = NULL;
               }
            }
         }
      }
      
      if ( returnItem == NULL ) {
         mc2log << error << here << " Found more than one close item, and "
                << "can not separate them on name or geographically." << endl;
      }
   }

   // Delete vector?
   if (shouldKill)
      delete closeIDs;

   DEBUG4(
      mc2dbg << "#closeItems=" << closeItems.size() 
             << ", #closeIDs=" << closeIDs->getSize() << " ";
      if (returnItem == NULL) {
         mc2dbg << "Failed to find item with type=" << uint32(type) 
                << ", close to (" << lat << "," << lon << "), name="
                << itemName << endl;
      } else {
         mc2dbg << "Found item with type=" << uint32(type) 
                << ", close to (" << lat << "," << lon << "), name="
                << itemName << endl;
      }
   );
   
   return returnItem;
}

// Called from MapEditor
bool 
OldGenericMap::getCoordinateFromItem(const OldItem* item, 
                                  int32 &lat, int32 &lon,
                                  byte &nameOffset,
                                  bool printLog)
{
   MC2_ASSERT(item != NULL);
   MC2_ASSERT( (item->getGfxData() != NULL) ||
               (item->getItemType() == ItemTypes::streetItem) );
   // streetItems have no gfxData, but are handles in the
   // getItemCoordinates and getItemFromCoordinate methods
   // so it is possible to get a coordinate.

   bool returnValue = false;
   vector<OldItem*> closeItems;
   const uint32 n = 1; //5;
   nameOffset = MAX_BYTE;
   uint32 i = 0;
   while ( (i < n) && (!returnValue) ) {
      //uint16 offset = 4096+(int) (57344.0*rand()/(RAND_MAX+1.0));
      uint16 offset = MAX_UINT16/2;
      if (getItemCoordinates(item->getID(), offset, lat, lon)) {

         // If this item have polygon gfx data, try to get a coordinate
         // inside it instead of on the border.
         if (item->getGfxData() && item->getGfxData()->getClosed(0)){
            ItemTypes::itemType type = item->getItemType();
            uint32 mc2CoordOffset = 50;
            
            if ( getItemFromCoordinate(type, lat+mc2CoordOffset, lon, 
                                       closeItems, NULL, 1) == item ){
               mc2dbg8 << "Lat +" <<endl;
               lat = lat+mc2CoordOffset;
            }
            else if (getItemFromCoordinate(type, lat-mc2CoordOffset, lon, 
                                           closeItems, NULL, 1) == item ){
               mc2dbg8 << "Lat -" <<endl;
               lat = lat-mc2CoordOffset;
            }
            else if (getItemFromCoordinate(type, lat, lon+mc2CoordOffset, 
                                           closeItems, NULL, 1) == item ){
             
               mc2dbg8 << "Lon +" <<endl;
               lon = lon+mc2CoordOffset;
            }
            else if (getItemFromCoordinate(type, lat, lon-mc2CoordOffset, 
                                           closeItems, NULL, 1) == item ){
               mc2dbg8 << "Lon -" <<endl;
               lon = lon-mc2CoordOffset;
            }
            else {
               mc2dbg8 << "No coord offset." <<endl;
            }
         }


         // Try with different names
         uint32 j=0;
         while ( (j<item->getNbrNames()) && (!returnValue)) {
            nameOffset = j;
            OldItem* testItem = getItemFromCoordinate(
                                 item->getItemType(), 
                                 lat, lon,
                                 closeItems,
                                 getName(item->getStringIndex(j)),
                                 1);
            returnValue = (testItem == item);
            j++;
         }
         // If no name, check without!
         if (item->getNbrNames() == 0) {
            OldItem* testItem = getItemFromCoordinate(item->getItemType(), 
                             lat, lon, closeItems, NULL, 1);
            returnValue = (testItem == item);
         }
      }
      i++;
   }

   if ( printLog ) {
      mc2dbg1 << here << " Coordinate set to " << lat << "," << lon 
              << ", nameOffset = " << uint32(nameOffset) << " (" 
              << getName(item->getStringIndex(nameOffset)) << ")" << endl;
   }
   return returnValue;
}


bool
OldGenericMap::coordExistInMap( MC2Coordinate coord )
{
   uint32 radius = 1000; //mc2 units, about 10 meters.
   MC2BoundingBox bBox(coord, radius);
   
   set<OldItem*> closeItems;
   // Look among all item types.
   set<ItemTypes::itemType>allowedTypes;
   for (uint32 i=0; i<ItemTypes::numberOfItemTypes; i++){
      allowedTypes.insert(static_cast<ItemTypes::itemType>(i));
   }
   getItemsWithinBBox(closeItems,
                      bBox,
                      allowedTypes );
   //mc2dbg << "Nbr items close: " << closeItems.size() << endl;
   for (set<OldItem*>::const_iterator itemIt = closeItems.begin(); 
        itemIt!= closeItems.end(); ++itemIt){
      if ((*itemIt) == NULL){
         continue;
      }
      GfxData* gfx = (*itemIt)->getGfxData();
      if ( gfx == NULL ){
         continue;
      }
      //uint32 totalNbrCoords = gfx->getTotalNbrCoordinates();
      //mc2dbg << "Nbr coords: " <<totalNbrCoords<< endl;
      uint32 nbrPol = gfx->getNbrPolygons();
      for (uint32 p=0; p<nbrPol; p++){
         uint32 nbrCoords = gfx->getNbrCoordinates(p);
         for (uint32 c=0; c<nbrCoords; c++){
            MC2Coordinate testCoord = gfx->getCoordinate(p, c);
            if ( testCoord == coord ){
               return true;
            }
         }
      }
      // mc2dbg << "Checked coords" << endl;  
   }
   return false;
   

} // coordExistInMap


vector<OldItem*>
OldGenericMap::getRegions(const OldItem* item, ItemTypes::itemType type,
                          bool recursive /* = false */ ) const
{
   vector<OldItem*>items;
   for (uint32 n=0; n<item->getNbrGroups(); ++n) {
      OldItem* group = itemLookup(item->getGroup(n));
      MC2_ASSERT(group != NULL);
      if (group != NULL){
         if (group->getItemType() == type) {
            items.push_back(group);
            if ( recursive ) {
               vector<OldItem*> nextItems = 
                  getRegions(group, type, recursive);
               for (uint32 g=0; g<nextItems.size(); g++) {
                  bool add = true;
                  for ( vector<OldItem*>::const_iterator it = items.begin();
                        it != items.end(); it++ ) {
                     if ( nextItems[g]->getID() == (*it)->getID() ) {
                        add = false;
                     }
                  }
                  if ( add ) {
                     items.push_back( nextItems[g] );
                  }
               }
            }
         }
      }
   }
   return items;
}

uint32
OldGenericMap:: polygonHasItemsInside(OldItem* polyItem,
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
         OldItem* item = m_itemsZoom[currentZoom][i];

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
OldGenericMap::isUniqueCityCentre( OldItem* item )
{
   // Respons don't like city centers, always return false.
   // return false;

   OldPointOfInterestItem* cc = dynamic_cast<OldPointOfInterestItem*> ( item );
   if ( ( cc != NULL ) && 
        ( cc->isPointOfInterestType( ItemTypes::cityCentre ) ) &&
        ( cc->getNbrNamesWithType( ItemTypes::officialName ) > 0 ) ) {
      uint32 buaID = getRegionID( cc, ItemTypes::builtUpAreaItem ); 
      OldItem* bua = itemLookup( buaID );

      if ( bua != NULL ) {
         return !oneNameSimilarCase( bua, cc );
      } 
      else {

         // Use the bua of the street.
         uint32 ssiID = cc->getStreetSegmentItemID();
         //      OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*> 
         //                         ( itemLookup(ssiID) );
         OldItem* ssi = itemLookup(ssiID);
         if ( ssi != NULL ){
            vector<OldItem*>buaRegions = 
               getRegions(ssi, ItemTypes::builtUpAreaItem);
            for(uint32 i=0; i<buaRegions.size(); i++){
               if ( oneNameSimilarCase( buaRegions[i], cc ) ){
                  return false;
               }
            }
         }
            


         // No bua when using location. Try geometry.
         uint32 i = 0;
         while ( i < getNbrItemsWithZoom( 0 ) ) {
            OldBuiltUpAreaItem* tmpBua = 
               dynamic_cast<OldBuiltUpAreaItem*> ( getItem( 0, i ) );
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

const MapRights&
OldGenericMap::getRights( uint32 itemID ) const
{
   return m_userRightsTable->getRights( itemID );
}

bool 
OldGenericMap::getNodeCoordinates(uint32 nodeID, int32 &lat, int32 &lon) const
{
   OldItem* i = itemLookup(nodeID);
   MC2_ASSERT(i != NULL);
   if (i == NULL)
      return (false);

   GfxData* gfx = i->getGfxData();
   MC2_ASSERT(gfx->getNbrPolygons() == 1);
   if ((gfx == NULL) && gfx->getNbrCoordinates(0) == 0)
      return (false);

   if ((nodeID & 0x80000000) == 0x80000000) {
      // OldNode 1
      lat = gfx->getLastLat(0);
      lon = gfx->getLastLon(0);
      return (true);
   } else {
      // OldNode 0
      lat = gfx->getLat(0,0);
      lon = gfx->getLon(0,0);
      return (true);
   }

   return (false);
}


bool
OldGenericMap::oneNameSimilarCase( const OldItem* item, 
                                const OldItem* otherItem ) const
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
OldGenericMap::oneNameSameCase( const OldItem* item, 
                             const OldItem* otherItem ) const
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
OldGenericMap::getItemGfx( const OldItem* item, GfxDataFull& gfx ) const
{
   {
      const GfxData* itemgfx = item->getGfxData();
      if ( itemgfx ) {
         return itemgfx;
      }
   }
   if ( item->getItemType() == ItemTypes::pointOfInterestItem ) {
      
      const OldPointOfInterestItem* poi =
         static_cast<const OldPointOfInterestItem*>(item);
      
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
OldGenericMap::itemAllowedByUserRights( uint32 id,
                                     const UserRightsMapInfo& rights ) const
{
   return rights.itemAllowed( m_userRightsTable->getRights( id ),
                              IDPair_t( getMapID(), id ),
                              m_idTranslationTable );
}

bool
OldGenericMap::itemAllowedByUserRights( const OldItem& item,
                                     const UserRightsMapInfo& rights ) const
{
   return itemAllowedByUserRights( item.getID(), rights );
}

MC2String
OldGenericMap::getNameOfNeededRight( uint32 id,
                                  LangTypes::language_t lang ) const
{   
   return m_userRightsTable->getRights( id ).getName( lang );
}

MC2String
OldGenericMap::getNameOfNeededRight( const OldItem& item,
                                  LangTypes::language_t lang ) const
{
   return getNameOfNeededRight( item.getID(), lang );
}

bool
OldGenericMap::itemUsedAsGroup(OldItem* group) const
{
   bool found = false;
   uint32 z = 0;
   while ( !found && ( z <  NUMBER_GFX_ZOOMLEVELS ) ){

      uint32 i=0;
      while ( !found && ( i < getNbrItemsWithZoom(z) ) ) {

         OldItem* item = getItem(z, i);
         if ( item != NULL ){
            if ( item->memberOfGroup(group->getID()) ){
               found = true;
               mc2dbg8 << "   OldGenericMap::itemUsedAsGroup: used for:" 
                       << item->getID() << endl;
            }
         }

         i++;
      }
      z++;
   }
   return found;
}

bool
OldGenericMap::createGfxDataFromSSIs( ItemTypes::itemType itemType )
{
   bool result = true;
   set<uint32> itemsWithUpdatedGfx;
   set<uint32> itemsWithNewGfx;
   
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( ( item != NULL ) && 
              ( item->getItemType() == ItemTypes::streetSegmentItem ) ){
            for ( uint32 g=0; g < item->getNbrGroups(); g++ ){
               uint32 groupID = item->getGroup(g);
   
               OldItem* group = itemLookup( groupID );
               if ( group->getItemType() == itemType ){

                  // Update gfx data for group item

                  MC2BoundingBox bBox;
                  if ( group->getGfxData() != NULL ){
                     
                     // Get existing gfx data bounding box.
                     MC2BoundingBox gfxBBox;
                     group->getGfxData()->getMC2BoundingBox(gfxBBox);
                     bBox.update(gfxBBox);
                     itemsWithUpdatedGfx.insert(groupID);
                     
                  }
                  if ( item->getGfxData() != NULL ){
                     
                     // Update with item gfx data bounding box.
                     MC2BoundingBox gfxBBox;
                     item->getGfxData()->getMC2BoundingBox(gfxBBox);
                     bBox.update(gfxBBox);
                     itemsWithNewGfx.insert(groupID);
                     
                  }
                  // Create gfx data from the new bounding box.
                  GfxDataFull* gfx = new GMSGfxData();
                  gfx->addCoordinate(bBox.getMinLat(), bBox.getMinLon(), 
                                     true);
                  gfx->addCoordinate(bBox.getMaxLat(), bBox.getMinLon());
                  gfx->addCoordinate(bBox.getMaxLat(), bBox.getMaxLon());
                  gfx->addCoordinate(bBox.getMinLat(), bBox.getMaxLon());
                  gfx->setClosed(0, true);
                  gfx->updateBBox();
                  gfx->updateLength();

                  group->setGfxData(gfx);

               }
            } // for each group item
         }
      } // for each item
   } // for each zoom
   mc2dbg1 << "OldGenericMap::createGfxDataFromSSIs "
           << "updated gfx for " << itemsWithUpdatedGfx.size() << " items, "
           << "new gfx for " << itemsWithNewGfx.size() << " items"
           << endl;

   return result;
}

bool
OldGenericMap::getConnectToNodeIDs(uint32 fromNodeID, Vector* toNodeIDVec)
{
   // Opposing node means node 0 -> node 1 and vice versa.
   
   // The opposite node of fromNode
   OldNode* oppositeNode = nodeLookup(fromNodeID ^ 0x80000000);
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

OldConnection*
OldGenericMap::getOpposingConnection(OldConnection* conn, uint32 toNodeID)
{
   if ( conn->isMultiConnection() ) {
      return NULL;
   }

   OldNode* fromNode = nodeLookup(conn->getConnectFromNode() ^ 0x80000000);
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

OldNode* 
OldGenericMap::nodeLookup(uint32 nodeID) const
{
   OldItem* i = itemLookup(nodeID); 
   OldRouteableItem* ri = dynamic_cast<OldRouteableItem*>(i);
   if (ri != NULL)
      return ri->getNodeFromID(nodeID);
   else
      return (NULL);
}


bool
OldGenericMap::getOtherConnections( OldConnection* conn, 
                           uint32 toNodeID,
                           vector<pair<OldConnection*, OldNode*> >& otherConn )
{
   OldNode* toNode = nodeLookup( toNodeID );
   if ( ( toNode == NULL ) || ( conn == NULL ) || 
        ( conn->isMultiConnection() ) ) {
      return false;
   }
   
   OldConnection* oppConn = getOpposingConnection(conn, toNodeID);
   uint32 oppToNodeID = conn->getConnectFromNode() ^ 0x80000000;
   //cout << "OppToNode: " << oppToNodeID << " OppFromNode: "
   //     << oppConn->getConnectFromNode() << endl;
   OldNode* oppToNode = nodeLookup( oppToNodeID );
   //cout << "Nbr conn: " << oppToNode->getNbrConnections() << endl;
   for ( uint32 i = 0; i < oppToNode->getNbrConnections(); ++i ) {
      OldConnection* oppToNodeConn = oppToNode->getEntryConnection(i);
      //cout << "i:" << i << " FromNode:"
      //     << oppToNodeConn->getConnectFromNode() << endl;
      if ( ( ! oppToNodeConn->isMultiConnection() ) && 
           ( oppToNodeConn->getConnectFromNode() !=
             oppConn->getConnectFromNode() ) ) {
         OldConnection* returnConn = getOpposingConnection
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
OldGenericMap::isPartOfStreet(const OldItem* item) const
{  
   uint32 nbrGroups = item->getNbrGroups();
   uint32 i = 0;
   bool partOfStreet = false;
   
   while ((i < nbrGroups) && (!partOfStreet)) {
      // Check if the group item is a street.
      OldStreetItem* si = dynamic_cast<OldStreetItem*> 
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
OldGenericMap::isPartOfGfxStreet(const OldItem* item) const
{  
   uint32 nbrGroups = item->getNbrGroups();
   uint32 i = 0;
   bool partOfStreet = false;
   
   while ((i < nbrGroups) && (!partOfStreet)) {
      // Check if the group item is a street.
      OldStreetItem* si = dynamic_cast<OldStreetItem*> 
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
OldGenericMap::partOfNbrStreets(const OldItem* item) const
{
   uint32 count = 0;

   for (uint32 i = 0; i < item->getNbrGroups(); i++) {
      if ( dynamic_cast<OldStreetItem*> 
            (itemLookup(item->getGroup(i))) != NULL ) {
         count++;
      }
   }
  
   return (count);
}

uint32
OldGenericMap::getCityPartID(const OldItem* item) const 
{
   OldItem* cp = getRegion(item, ItemTypes::cityPartItem);
   if (cp != NULL)
      return cp->getID();
   return MAX_UINT32;
}

uint32
OldGenericMap::getRegionID(const OldItem* item, ItemTypes::itemType type) const
{
   OldItem* groupItem = getRegion(item, type);
   if (groupItem != NULL)
      return groupItem->getID();
   return MAX_UINT32;
}

OldItem* 
OldGenericMap::getRegion(const OldItem* item, ItemTypes::itemType type) const
{
   uint32 n=0;
   while (n<item->getNbrGroups()) {
      //      mc2log << info << "GroupID: " << item->getGroup(n) << endl;

      //      if ( ! MapBits::isCountryMap(getMapID())){
         // We can't expect to find groups with the right ID in 
         // country overview maps.

         OldItem* group = itemLookup(item->getGroup(n));
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
OldGenericMap::getNbrRegions(const OldItem* item, ItemTypes::itemType type) const
{
   uint32 nbrFound=0;
   for (uint32 n=0; n<item->getNbrGroups(); ++n) {
      OldItem* group = itemLookup(item->getGroup(n));
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
OldGenericMap::getRegionIDIndex(const OldItem* item, uint32 i,
                             ItemTypes::itemType type) const
{
   OldItem* loc = getRegionIndex(item, i, type);
   if (loc != NULL)
      return loc->getID();
   return MAX_UINT32;
}

OldItem*
OldGenericMap::getRegionIndex(const OldItem* item, uint32 i,
                             ItemTypes::itemType type) const
{
   uint32 n=0, nbrFound=0;
   while (n<item->getNbrGroups()) {
      OldItem* group = itemLookup(item->getGroup(n));
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
OldGenericMap::hasRegion( const OldItem* item, 
                          const OldItem* region ) const
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

bool
OldGenericMap::addRegionToItem( OldItem* item, const OldItem* region,
                             bool noLocationRegion )
{
   MC2_ASSERT( item != region );
   if ( ( item != NULL ) && ( region != NULL ) ) {
      return addRegionToItem( item, region->getID(), noLocationRegion );
   } else {
      return false;
   }
}

bool 
OldGenericMap::addRegionToItem( OldItem* item, uint32 regionID,
                             bool noLocationRegion )
{
   if (itemNotInSearchIndex(item->getID())){
      mc2dbg << "Not setting group to non-search-index item." << endl;
      return false;
   }
   if ( itemNotInSearchIndex(regionID) ){
      mc2dbg << "Not adding non search index group: " << regionID << " to "
             << item->getID() << endl;
      return false;
   }
   regionID &= 0x7fffffff;
   MC2_ASSERT( item->getID() != regionID );
   if ( ( item != NULL ) && ( itemLookup( regionID ) != NULL ) ) {
      if (noLocationRegion) {
         regionID |= 0x80000000;
      }
      return item->addGroup( regionID );
   } else {
      return false;
   }
}

bool
OldGenericMap::removeRegionFromItem( OldItem* item, OldItem* region )
{
   if ( ( item != NULL ) && ( region != NULL ) ) {
      OldGroupItem* group = dynamic_cast<OldGroupItem*> ( region );
      if ( group != NULL ) {
         group->removeItemWithID( item->getID() );
      }
      return item->removeGroupWithID( region->getID() );
   } 
   return false;
}

bool 
OldGenericMap::removeRegionFromItem( OldItem* item, uint32 regionID )
{
   regionID &= 0x7fffffff;
   OldItem* region = itemLookup( regionID );
   if ( item != NULL ) {
      if ( region != NULL ) {
         return removeRegionFromItem( item, region );
      } else {
         return item->removeGroupWithID( regionID );
      }
   }
   return false;
}

void
OldGenericMap::clearRegionsForItem( OldItem* item, 
                                 ItemTypes::itemType itemType 
                                    /* = ItemTypes::numberOfItemTypes */ ) 
{
   int32 lastGroup = item->getNbrGroups() - 1;
   for ( int32 i = lastGroup; i >= 0; --i ) {
      OldItem* region = itemLookup( item->getGroup( i ) );
      if ( ( region->getItemType() == itemType ) ||
           ( itemType == ItemTypes::numberOfItemTypes ) ) {
         OldGroupItem* group = dynamic_cast<OldGroupItem*> ( region );
         if ( group != NULL ) {
            group->removeItemWithID( item->getID() );
         }
         item->removeGroup( i );
      } 
   }      
}

bool
OldGenericMap::replaceRegions( OldItem* item, OldItem* modelItem,
                     ItemTypes::itemType itemType
                     /*= ItemTypes::numberOfItemTypes */)
{
   bool retVal = true;
   // first remove all regions of item, considering itemType
   clearRegionsForItem(item, itemType);

   retVal = copyRegions(item, modelItem, itemType);
   
   return retVal;
}

bool
OldGenericMap::copyRegions( OldItem* item, OldItem* modelItem,
                     ItemTypes::itemType itemType
                     /*= ItemTypes::numberOfItemTypes */)
{
   bool retVal = true;
   // add the regions from modelItem
   for (uint32 g=0; g<modelItem->getNbrGroups(); g++) {
      OldItem* group = itemLookup(modelItem->getGroup(g));
      MC2_ASSERT(group != NULL);
      if ( (itemType == ItemTypes::numberOfItemTypes) ||
           (itemType == group->getItemType()) ) {
         if ( ! addRegionToItem(item, group->getID()) ) {
            retVal = false;
         }
      }
   }
   return retVal;
}

void
OldGenericMap::setUserRights(UserRightsItemTable::itemMap_t& itemMap)
{
   //UserRightsItemTable itemMap_t oldURItems = 
   // m_userRightsTable->getItems();
   //
   // Add the old to the new ones and set them to the new URIT.
   
   UserRightsItemTable newURIT( ~MapRights(), itemMap );
   m_userRightsTable->swap(newURIT);

} // OldGenericMap::setUserRight

int
OldGenericMap::getUserRights(UserRightsItemTable::itemMap_t& itemMap)
{
   return m_userRightsTable->getItems(itemMap);

} // OldGenericMap::getUserRight

void
OldGenericMap::dbgPrintFilePos(MC2String message, 
                            int outfile, 
                            DataBuffer* dbuf)
{
   off_t filePos = lseek(outfile, 0, SEEK_CUR);
   if ( filePos == (off_t)-1 ){
      mc2dbg8 << "FilePos: Could not get position in file: "
              << outfile << endl;
   }
   else {
      uint32 pos = filePos;
      if ( dbuf!=NULL ){
         pos+=dbuf->getCurrentOffset();
      }
      mc2dbg8 << "FilePos: 0x" << hex << pos << dec << ": " << message 
              << endl;
   }
} // dbgPrintFilePos

OldGenericMap::adminAreaCentreTable_t
OldGenericMap::createCentreCoordinatesForAdminAreas(
      ItemTypes::itemType adminType )
{

   uint32 mapId = getMapID();
   mc2dbg1 << "createCentreCoordinatesForAdminAreas for adminType="
           << ItemTypes::getItemTypeAsString(adminType)
           << ", creating from map " << mapId << endl;

   // Count how many admin area items there are to create centre coord for,
   // so we can compare at the end.
   set<uint32> candidateAdminItems;
   // Re-use centre coords for index areas
   adminAreaCentreTable_t indexAreaCentres;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( item == NULL ) {
            continue;
         }
         if (item->getItemType() != adminType) {
            continue;
         }
         uint32 itemID = item->getID();

         // Skip index areas that already has a centre, because
         // it is not certain that they can be created correctly
         // from the map data.
         if ( isIndexArea(itemID) &&
              hasAdminAreaCentre(itemID) ) {
            indexAreaCentres.insert( make_pair(
               itemID, getCentreFromAdminTable(itemID)) );
         }
         // Skip items not in search index
         else if ( ! itemNotInSearchIndex(itemID) ) {
            candidateAdminItems.insert(itemID);
         }
      }
   }
   mc2dbg1 << "cCCFAA: candidateAdminItems=" << candidateAdminItems.size()
           << " indexAreaCentres=" << indexAreaCentres.size()
           << endl;

   // Find all POI city centres in this map. For each city centre, check
   // if it located in an admin area item (adminType) with the same name
   // as the city centre. If so, store info about this in a map.

   // A map with admin area id connected to a city centre mc2 coordinate
   // Do we need a multimap??
   adminAreaCentreTable_t adminCentres;
   adminAreaCentreTable_t::const_iterator it;
   set<uint32>::const_iterator setIt;

   for ( uint32 i = 0; i < getNbrItemsWithZoom( ItemTypes::poiiZoomLevel );
         i++ ) {
      OldPointOfInterestItem* poi = static_cast<OldPointOfInterestItem*> 
         ( getItem( ItemTypes::poiiZoomLevel, i ) );
      if ( poi == NULL ) {
         continue;
      }
      if ( ! poi->isPointOfInterestType(ItemTypes::cityCentre) ) {
         continue;
      }

      // we have a city centre poi, get admin area items for this cc
      uint32 ssiId = poi->getStreetSegmentItemID();
      OldItem* ssi = itemLookup( ssiId );
      if ( ssi == NULL ) {
         continue;
      }
      bool recursive = 
         ( NationalProperties::useIndexAreas(
               getCountryCode(), getMapOrigin()) &&
            (adminType == ItemTypes::builtUpAreaItem) );
      vector<OldItem*> adminItems = getRegions(ssi, adminType, recursive);
      if ( adminItems.size() > 0 ) {
      mc2dbg8 << "for poi " << getBestItemName(poi->getID()) << " got "
              << adminItems.size() << " regions"
              << " (no recursive " << getRegions(ssi, adminType).size()
              << " regions)" << endl;
      }

      for ( vector<OldItem*>::iterator a = adminItems.begin();
            a != adminItems.end(); a++ ) {
         
         uint32 adminId = (*a)->getID();
         mc2dbg8 << " admin " << adminId 
                 << " " << getBestItemName( adminId ) << endl;
         
         // Check if the admin area is among the candidates
         setIt = candidateAdminItems.find(adminId);
         if ( setIt == candidateAdminItems.end() ) {
            continue;
         }
         
         // Check if the city centre share a name with this admin area
         // This oneNameSameCase method does not compare synonymNames
         if ( ! oneNameSameCase((*a), poi) ) {
            continue;
         }

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
   mc2dbg1 << "cCCFAA: created " << adminCentres.size()
           << " centres from city centres for "
           << candidateAdminItems.size() << " candidates" << endl;
   if ( indexAreaCentres.size() > 0 ) {
      for ( it = indexAreaCentres.begin(); 
            it != indexAreaCentres.end(); it++ ) {
         adminCentres.insert( make_pair(it->first, it->second) );
      }
   }
   mc2dbg1 << "cCCFAA: filled with " << indexAreaCentres.size()
           << " index area centres -> " << adminCentres.size() << endl;

   // Check if any of the candidate admin areas did not get 
   // a centre coordinate
   uint32 nbrBoxCentres = 0;
   for ( set<uint32>::const_iterator setIt = candidateAdminItems.begin();
         setIt != candidateAdminItems.end(); setIt++ ) {
      OldItem* item = itemLookup( *setIt );
      if ( item == NULL ) {
         MC2_ASSERT(false);
      }
      if ( item->getItemType() != adminType ) {
         MC2_ASSERT(false);
      }
      uint32 adminId = item->getID();
      if ( adminCentres.find(adminId) == adminCentres.end() ) {
         mc2dbg1 << "cCCFAA: no normal centre coord for admin area "
              << mapId << ":" << adminId << " " 
              << getFirstItemName( item ) << " ("
              << ItemTypes::getItemTypeAsString(adminType) << ")"
              << endl;
      
         // Get centre coordinates for all admin areas that have
         // no gfxData, since this cannot be handled by the server
         // Build bbox of all items in this area and get centre coord
         GfxData* gfx = item->getGfxData();
         MC2BoundingBox bbox;
         bool createdBox = false;
         if ( gfx != NULL ) {
            // No need to do anything, the searching will zoom
            // to the centre of the item's gfxData
            //if (gfx->getMC2BoundingBox(bbox)) {
            //   createdBox = true;
            //}
            // Index area buas will have their gfx data removed, so 
            // we must create admin centre for them
            if ( NationalProperties::useIndexAreas(
                     getCountryCode(), getMapOrigin()) &&
               (adminType == ItemTypes::builtUpAreaItem) &&
               (isIndexArea(item->getID())) ) {
               if (gfx->getMC2BoundingBox(bbox)) {
                  createdBox = true;
               }
            }
         } else {
            
            vector<uint32> myItems = getItemsInGroup(adminId);
            for (uint32 i=0; i<myItems.size(); i++){
               OldItem* myItem = itemLookup( myItems[i] );
               if (myItem == NULL) {
                  continue;
               }
               GfxData* myGfx = myItem->getGfxData();
               if (myGfx != NULL) {
                  MC2BoundingBox bb;
                  if (myGfx->getMC2BoundingBox(bb)) {
                     bbox.update( bb );
                     createdBox = true;
                  }
               }
            }
         }
         if ( createdBox ) {
            int32 clat, clon;
            bbox.getCenter( clat, clon );
            adminCentres.insert( 
               make_pair(adminId, MC2Coordinate(clat, clon)) );
            nbrBoxCentres++;
            mc2dbg1
               << "cCCFAA: created bbox centre coord for admin area "
               << mapId << ":" << adminId << " "
               << getFirstItemName( item ) << " ("
               << ItemTypes::getItemTypeAsString(adminType) << ")"
               << endl;
         } else {
            mc2dbg1 
               << "cCCFAA: REALLY NO centre coord for admin area "
               << mapId << ":" << adminId << " " 
               << getFirstItemName( item ) << " ("
               << ItemTypes::getItemTypeAsString(adminType) << ")"
               << endl;
         }
      }
   }
   mc2dbg1 << "cCCFAA: created " << nbrBoxCentres 
           << " centres from bbox for " << candidateAdminItems.size() 
           << " candidates" << endl;
   mc2log << info << "cCCFAA: created " << adminCentres.size()
          << " admin area centre coords for " 
          << ItemTypes::getItemTypeAsString(adminType)
          << "s in map " << mapId << endl;

   return adminCentres;
} // createCentreCoordinatesForAdminAreas

void
OldGenericMap::dumpAdminAreaCentres() 
{
   mc2log << info << "OldGenericMap::dumpAdminAreaCentres()" << endl
          << "This map " << getMapID() << " has " << m_nbrAdminAreaCentres 
          << " admin area centre coordinates." << endl;
   
   for ( uint32 i = 0; i < m_nbrAdminAreaCentres; i++ ) {
      adminAreaCentre_t elem = m_adminAreaCentres[i];
      cout << "  " << elem.itemID << ": " << elem.centre << endl;
   }
}

void
OldGenericMap::setAminAreaCentres ( const adminAreaCentreTable_t& adminCentres )
{
   // loop the map with centres and put in m_adminAreaCentres

   // delete old 
   delete [] m_adminAreaCentres;
   m_adminAreaCentres = NULL;
   m_nbrAdminAreaCentres = 0;

   // then make new, and fill
   m_nbrAdminAreaCentres = adminCentres.size();
   m_adminAreaCentres = new adminAreaCentre_t[m_nbrAdminAreaCentres];
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

OldGenericMap::adminAreaCentreTable_t
OldGenericMap::getAdminAreaCentres() const
{
   adminAreaCentreTable_t adminCentres;
   for ( uint32 i = 0; i < m_nbrAdminAreaCentres; i++ ) {
      adminAreaCentre_t elem = m_adminAreaCentres[i];

      adminCentres.insert( make_pair(elem.itemID, elem.centre) );
   }
   return adminCentres;
}

bool
OldGenericMap::hasAdminAreaCentre( uint32 itemID ) const
{
   MC2Coordinate tableCoord = 
      ::getCentreFromAdminTable( m_adminAreaCentres,
                                 m_nbrAdminAreaCentres,
                                 itemID );
   if ( tableCoord.isValid() ) {
      return true;
   }

   return false;
   
}

vector<ItemTypes::itemType>
OldGenericMap::getItemTypesToCreateAdminCentres()
{
   vector<ItemTypes::itemType> adminTypes;
   adminTypes.push_back(ItemTypes::municipalItem);
   adminTypes.push_back(ItemTypes::builtUpAreaItem);

   // If adding item types here, also update
   // updateAdminAreaCentres
   // createAndSetCentreCoordinatesForAdminAreas

   return adminTypes;
}

uint32
OldGenericMap::createAndSetCentreCoordinatesForAdminAreas()
{
   if (! MapBits::isUnderviewMap( getMapID()) ) {
      mc2log << error 
             << "OldGenericMap::createAndSetCentreCoordinatesForAdminAreas: "
             << " only use this method for underviews" << endl;
      return 0;
   }

   // Collect any zip centre coordinates already stored in this map
   // since they cannot be created here (from information in this map).
   // Collect centre coordinates for index areas, which can/may not
   // either be created correctly here
   adminAreaCentreTable_t myAdminAreaCentres = getAdminAreaCentres();
   adminAreaCentreTable_t zipCentres;
   adminAreaCentreTable_t indexAreaCentres;
   adminAreaCentreTable_t::const_iterator it;
   for ( it = myAdminAreaCentres.begin();
         it != myAdminAreaCentres.end(); it++ ) {
      uint32 id = it->first;
      OldZipCodeItem* zip = dynamic_cast<OldZipCodeItem*>(itemLookup(id));
      if ( zip != NULL ) {
         zipCentres.insert( make_pair(it->first, it->second) );
      }
      else {
         if ( isIndexArea(id) ) {
            indexAreaCentres.insert( make_pair(it->first, it->second) );
         }
      }
   }
   mc2dbg << "OldGenericMap::createAndSetCentreCoordinatesForAdminAreas: "
          << "saved centres coordinates for " << zipCentres.size()
          << " zip codes" << endl;
   mc2dbg << "OldGenericMap::createAndSetCentreCoordinatesForAdminAreas: "
          << "saved centres coordinates for " << indexAreaCentres.size()
          << " index areas" << endl;
   
   
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
   mc2dbg8 << "adminCentres normal   " << adminCentres.size() << endl;
   // Re-add the zip centres
   if ( zipCentres.size() > 0 ) {
      for ( it = zipCentres.begin(); it != zipCentres.end(); it++ ) {
         adminCentres.insert( make_pair(it->first, it->second) );
      }
   }
   mc2dbg8 << "adminCentres after zip " << adminCentres.size() << endl;
   // Include the index area centres that was not already in adminCentres
   if ( indexAreaCentres.size() > 0 ) {
      adminAreaCentreTable_t::const_iterator newIt;
      for ( it = indexAreaCentres.begin(); 
            it != indexAreaCentres.end(); it++ ) {
         newIt = adminCentres.find(it->first);
         if ( newIt != adminCentres.end() ) {
            if ( newIt->second != it->second ) {
               mc2log << warn << "Index area " << it->first
                      << " has another centre coord now "
                      << it->second << " -> " << newIt->second
                      << endl;
               MC2_ASSERT(false);
            }
         } else {
            adminCentres.insert( make_pair(it->first, it->second) );
         }
      }
   }
   mc2dbg8 << "adminCentres after ia " << adminCentres.size() << endl;

   // Replace the admin centres in this map
   setAminAreaCentres( adminCentres );

   mc2log << info << "Filled admin area centres table with totally "
          << adminCentres.size() << " centre coordinates" << endl;
   return ( adminCentres.size() );
}

bool
OldGenericMap::updateAdminAreaCentres()
{
   bool updated = false;
   if (! MapBits::isUnderviewMap( getMapID()) ) {
      mc2log << error << "OldGenericMap::updateAdminAreaCentres: "
             << " only use this method for underviews" << endl;
      return false;
   }
   mc2dbg << "GMap::updateAdminAreaCentres: in map " << getMapID() << endl;
   
   // Check if the admin area centres need to be updated
   // Please save the zipCode centre coordinates, because they cannot
   // be created from the map content.
   // Please save the index area centre coordinates, because they cannot
   // be created correctly from the map content.
   // 1. Get the admin area centres from the table of this map
   // 2. Create/calculate admin area centres in this map
   // 3. Compare the content, re-set the table if they differ

   // 1
   adminAreaCentreTable_t myAdminAreaCentres = getAdminAreaCentres();
   adminAreaCentreTable_t zipCentres;
   adminAreaCentreTable_t indexAreaCentres;
   adminAreaCentreTable_t::const_iterator it;
   for ( it = myAdminAreaCentres.begin();
         it != myAdminAreaCentres.end(); it++ ) {
      uint32 id = it->first;
      OldZipCodeItem* zip = dynamic_cast<OldZipCodeItem*>(itemLookup(id));
      if ( zip != NULL ) {
         zipCentres.insert( make_pair(it->first, it->second) );
      }
      else {
         if ( isIndexArea(id) ) {
            indexAreaCentres.insert( make_pair(it->first, it->second) );
         }
      }
   }
   mc2dbg << "OldGenericMap::updateAdminAreaCentres: "
          << "saved centres coordinates for " << zipCentres.size()
          << " zip codes" << endl;
   mc2dbg << "OldGenericMap::updateAdminAreaCentres: "
          << "saved centres coordinates for " << indexAreaCentres.size()
          << " index areas" << endl;
   

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
   // also include the zip centres (that was saved)
   if ( zipCentres.size() > 0 ) {
      for ( it = zipCentres.begin(); it != zipCentres.end(); it++ ) {
         newAdminAreaCentres.insert( make_pair(it->first, it->second) );
      }
   }
   // Include the index area centres that was not already 
   // in newAdminAreaCentres
   if ( indexAreaCentres.size() > 0 ) {
      adminAreaCentreTable_t::const_iterator newIt;
      for ( it = indexAreaCentres.begin(); 
            it != indexAreaCentres.end(); it++ ) {
         newIt = newAdminAreaCentres.find(it->first);
         if ( newIt != newAdminAreaCentres.end() ) {
            if ( newIt->second != it->second ) {
               mc2log << warn << "Index area " << it->first
                      << " has another centre coord now "
                      << it->second << " -> " << newIt->second
                      << endl;
               MC2_ASSERT(false);
            }
         } else {
            newAdminAreaCentres.insert( make_pair(it->first, it->second) );
         }
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
               float64 sqDist = 
                  GfxUtility::squareP2Pdistance_linear(newCoord, jt->second);
               mc2dbg1 << " GMap::uAAC: coord changed for admin id "
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
                   << "use new coord for id " << id 
                   << " " << newCoord << endl;
         } else {
            updatedAdminAreaCentres.insert( make_pair(id, jt->second) );
         }
      }
      mc2dbg << "GMap::updateAdminAreaCentres: "
             << "using " << nbrNew << " admin coords from update ("
             << nbrDiffers << " coords changed in the update)" << endl;
      // size of newAAC and updatedAAC must be the same
      if ( updatedAdminAreaCentres.size() != newAdminAreaCentres.size() ) {
         mc2dbg << "OldGenericMap::updateAdminAreaCentres: "
                << "ERROR size of newAAC (" << newAdminAreaCentres.size()
                << ") and updatedAAC (" << updatedAdminAreaCentres.size()
                << ") not the same" << endl;
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
          // newAdminAreaCentres has same size as updatedAdminAreaCentres
          
   return updated;
   
}

vector<uint32>
OldGenericMap::getItemsInGroup(uint32 groupID, 
                               ItemTypes::itemType typeToLookFor) const{
   vector<uint32>result;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( item == NULL) {
            continue;
         }
         if ( (typeToLookFor != ItemTypes::numberOfItemTypes) &&
              (item->getItemType() != typeToLookFor)) {
            continue;
         }
         if (item->memberOfGroup(groupID)){
            result.push_back(item->getID());
         }
      }
   }
   return result;
}

set<uint32>
OldGenericMap::getItemsInGroupRecursive(uint32 groupID) const{
   set<uint32>result;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( item == NULL) {
            continue;
         }
         if (item->memberOfGroup(groupID)){
            result.insert(item->getID());
            // in reality, only index areas that are in a hierachical
            // region structure
            if ( isIndexArea(item->getID()) ) {
               set<uint32> tmpSet = 
                  getItemsInGroupRecursive(item->getID());
               for ( set<uint32>::const_iterator tmpIT = tmpSet.begin();
                     tmpIT != tmpSet.end(); tmpIT++ ) {
                  result.insert(*tmpIT);
               }
            }
         }
      }
   }
   return result;
}


OldItem* 
OldGenericMap::getTopGroup( set<OldItem*> items ){
   set<uint32> visitedItemIDs;
   if (items.size() > 0){
      set<OldItem*> topNodes;
      // Includes recursive calls.
      getTopGroupProcessItem(topNodes, // items with no group.
                             visitedItemIDs, // all visited item IDs
                             (*items.begin()) // the item to process.
                             );
      if ( topNodes.size() == 1 ){
         set<OldItem*>::const_iterator itemIt = items.begin();
         while ( itemIt != items.end() ){
            if ( visitedItemIDs.find((*itemIt)->getID()) == 
                 visitedItemIDs.end() ){
               // Some item was not visited, return NULL
               return NULL;
            }
            ++itemIt;
         }
         // Everything OK, return the top node.
         return *(topNodes.begin());
      }
   }
   // Something is not as it should, return NULL.
   return NULL;
}

void
OldGenericMap::getTopGroupProcessItem( set<OldItem*>& topNodes,
                                       set<uint32>& visitedItemIDs,
                                       OldItem* item )
{
   visitedItemIDs.insert(item->getID());

   // Check higher nodes.
   if (item->getNbrGroups() == 0){
      topNodes.insert(item);
   }
   else {
      for (uint32 g=0; g<item->getNbrGroups(); g++){
         uint32 groupID = item->getGroup(g);
         if ( visitedItemIDs.find(groupID) == visitedItemIDs.end() ){
            // Not visited before, make the recursive call.
            OldItem* groupItem = itemLookup(groupID);
            getTopGroupProcessItem(topNodes, visitedItemIDs, groupItem);
         }
      }
   }

   // Check lower nodes.
   vector<uint32> lowerItemIDs = getItemsInGroup(item->getID());
   for (uint32 i=0; i<lowerItemIDs.size(); i++){
      uint32 lowerID = lowerItemIDs[i];
      if ( visitedItemIDs.find(lowerID) == visitedItemIDs.end() ){
         // Not visited before, make the recursive call.
         OldItem* lowerItem = itemLookup(lowerID);
         getTopGroupProcessItem(topNodes, visitedItemIDs, lowerItem);
      }
   }
}


vector<OldItem*>
OldGenericMap::searchItemsWithName(MC2String searchStr){
   vector<OldItem*> matchedItems;

   uint32 strIdx = m_itemNames->stringExist(searchStr.c_str());
   if (strIdx != MAX_UINT32){
      for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
            OldItem* item = getItem(z, i);
            if ( item == NULL) {
               continue;
            }
            for (uint32 n=0; n<item->getNbrNames(); n++) {
               if (strIdx == item->getStringIndex(n)){
                  matchedItems.push_back(item);
               }
            }
         }
      }
   }
   return matchedItems;
}

uint32
OldGenericMap::rmUnusedWater()
{
   uint32 nbrRemoved=0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item == NULL){
            continue;
         }
         OldWaterItem* waterItem = dynamic_cast<OldWaterItem*>(item);
         if (waterItem == NULL){
            continue;
         }
         if (! ( (waterItem->getWaterType() == ItemTypes::otherWaterElement) ||
                 (waterItem->getWaterType() == ItemTypes::ocean) ) ){
            continue;
         }
         // Here we have water items of type otherWaterElement or ocean.
         removeItem(waterItem->getID(), 
                    false,  // update hash table
                    false   // update admin centers.
                    );
         nbrRemoved++;
      }
   }
   buildHashTable();
   return nbrRemoved;
}


void 
OldGenericMap::reorderConnectingLanes(const set<uint32>& itemIDs)
{
   m_connectingLanesTable.reorderConnectingLanes(*this, itemIDs);

} // reorderConnectingLanes

void
OldGenericMap::signPostShapeUp()
{
   m_signPostTable.signPostShapeUp();

} // signPostShapeUp



const SignPostTable& 
OldGenericMap::getSignPostTable() const
{
   return m_signPostTable;

} // getSignPostTable


SignPostTable& 
OldGenericMap::getNonConstSignPostTable()
{
   return m_signPostTable;

} // getNonConstSignPostTable

const ItemMap< vector<GMSLane> >& 
OldGenericMap::getNodeLaneVectors(){

   return m_nodeLaneVectors;
}

const ConnectingLanesTable& 
OldGenericMap::getConnectingLanes(){
   return m_connectingLanesTable;
}

bool 
OldGenericMap::isIndexArea(uint32 itemID) const
{
   bool result = false;
   uint32 indexAreaOrder = getIndexAreaOrder(itemID);
   if ( indexAreaOrder != MAX_UINT32 ) {
      result = true;
   }
   return result;
}

uint32 
OldGenericMap::getIndexAreaOrder(uint32 itemID) const{
   uint32 result = MAX_UINT32;
   ItemMap<uint32>::const_iterator it = m_indexAreasOrder.find(itemID);
   if (it != m_indexAreasOrder.end()){
      result = it->second;
   }
   return result;
}


void 
OldGenericMap::setIndexAreaOrder(uint32 itemID, uint32 indexAreaOrder){

   ItemMap<uint32>::iterator it = m_indexAreasOrder.find(itemID);
   if (it != m_indexAreasOrder.end()){
      if ((indexAreaOrder != it->second) && indexAreaOrder >= MAX_UINT32-1){
         // We don't want to overwrite a valid value with an invalid one.
         mc2log << error << "Changing index area order from " << hex << "0x"
                << it->second << " to 0x" << indexAreaOrder << dec << endl;
         MC2_ASSERT(false);
      }
      it->second = indexAreaOrder;
   }
   else {
      m_indexAreasOrder.insert(make_pair(itemID, indexAreaOrder));
   }
}


bool 
OldGenericMap::itemNotInSearchIndex(uint32 itemID) const
{
   ItemMap<bool>::const_iterator it = m_nonSearchableItems.find(itemID);
   if (it != m_nonSearchableItems.end()){
      if ( it->second == false ){
         mc2dbg << error << "Found false post in itemNotInSearchIndex" << endl;
         // Right now 20080630, nothing should set this to false. This might
         // change.
      }
      return it->second;
   }
   return false;
}

void
OldGenericMap::setItemNotInSearchIndex(uint32 itemID)
{
   mc2dbg8 << "Tries to set " << itemID << " not in search index" << endl;
   ItemMap<bool>::iterator it = m_nonSearchableItems.find(itemID);
   if (it != m_nonSearchableItems.end()){
      it->second = true;
      mc2dbg8 << "Found post." << endl;
   }
   else {
      m_nonSearchableItems.insert(make_pair(itemID, true));
      mc2dbg8 << "Created new post." << endl;
   }
   
}

bool
OldGenericMap::indexAreaGroupsSameNameAndOrder(
                        const OldItem* indexAreaItem ) const
{
   if ( ! isIndexArea(indexAreaItem->getID()) ) {
      return false;
   }
   
   bool sameNames = true;
   bool sameIAOrder = true;
   if ( indexAreaItem->getNbrGroups() == 0 ) {
      return true;
   }
   OldItem* firstGroup = itemLookup(indexAreaItem->getGroup(0));
   MC2_ASSERT(firstGroup != NULL);
   uint32 firstIAorder = 
      getIndexAreaOrder(indexAreaItem->getGroup(0));
   for (uint32 gr = 0; gr < indexAreaItem->getNbrGroups(); gr++ ) {
      OldItem* tmpItem = itemLookup(indexAreaItem->getGroup(gr));
      mc2dbg8 << " item " << indexAreaItem->getID() 
              << " group " << gr << " " << tmpItem->getID()
              << " iaOrder=" << getIndexAreaOrder(tmpItem->getID())
              << " " << getFirstItemName(tmpItem->getID())
              << endl;
      if ( !firstGroup->hasSameNames(tmpItem) ) {
         sameNames = false;
      }
      if ( firstIAorder != getIndexAreaOrder(tmpItem->getID()) ) {
         sameIAOrder = false;
      }
   }
   if ( ! sameNames ) {
      mc2log << warn << "OldGenericMap::indexAreaGroupsSameNameAndOrder "
             << "The groups of index area item "
             << indexAreaItem->getID()
             << " have different names" << endl;
      //MC2_ASSERT(false);
   }
   else if (! sameIAOrder ) {
      mc2log << warn << "OldGenericMap::indexAreaGroupsSameNameAndOrder "
             << "The groups of index area item "
             << indexAreaItem->getID()
             << " have different ia orders" << endl;
      //MC2_ASSERT(false);
   }

   return ( sameNames && sameIAOrder );
}
bool
OldGenericMap::mergeIndexAreas(
         const OldItem* firstIA, const OldItem* secondIA) const
{
   bool result = true;
   mc2dbg8 << "OldGenericMap::mergeIndexAreas for items: " 
           << firstIA->getID() << " " << secondIA->getID() << endl;

   if ( ! isIndexArea(firstIA->getID()) ||
        ! isIndexArea(secondIA->getID()) ) {
      mc2log << error << "OldGenericMap::mergeIndexAreas "
             << "the items are not index areas "
             << firstIA->getID() << " " << secondIA->getID()
             << endl;
      return false;
   }

   // 1. Same item
   if ( firstIA->getID() == secondIA->getID() ) {
      return true;
   }

   // 2. Same index area order
   if ( getIndexAreaOrder(firstIA->getID()) !=
        getIndexAreaOrder(secondIA->getID()) ) {
      return false;
   }

   // 3. Same name
   if ( ! firstIA->hasCommonLangName(secondIA) ) {
      return false;
   }

   // 4. Check if same groups
   uint32 firstNbrGroups = firstIA->getNbrGroups();
   uint32 secondNbrGroups = secondIA->getNbrGroups();
   // no groups, it was enough with same index area order and same name
   if ( (firstNbrGroups == 0) && (secondNbrGroups == 0) ) {
      return true;
   }
   // same group ids
   set<uint32> firstGroupIDs;
   set<uint32> secondGroupIDs;
   for ( uint32 g = 0; g < firstNbrGroups; g++ ) {
      if ( isIndexArea(firstIA->getGroup(g)) ) {
         firstGroupIDs.insert(firstIA->getGroup(g));
      }
   }
   for ( uint32 g = 0; g < secondNbrGroups; g++ ) {
      if ( isIndexArea(secondIA->getGroup(g)) ) {
         secondGroupIDs.insert(secondIA->getGroup(g));
      }
   }
   if ( firstGroupIDs == secondGroupIDs ) {
      return true;
   }

   // 5. groups same name and other index area properties
   OldItem* firstGroup = NULL;
   OldItem* secondGroup = NULL;
   if ( firstNbrGroups > 0 ) {
      if ( ! indexAreaGroupsSameNameAndOrder(firstIA) ) {
         mc2log << warn << "The " << firstNbrGroups << " groups of item " 
                << firstIA->getID() << " not same ia order and name" << endl;
         return false;
      } else {
         firstGroup = itemLookup(firstIA->getGroup(0));
      }
   }
   if ( secondNbrGroups > 0 ) {
      if ( ! indexAreaGroupsSameNameAndOrder(secondIA) ) {
         mc2log << warn << "The " << secondNbrGroups << " groups of item " 
                << secondIA->getID() << " not same ia order and name" << endl;
         return false;
      } else {
         secondGroup = itemLookup(secondIA->getGroup(0));
      }
   }
   if ( (firstGroup == NULL) && (secondGroup != NULL) ) {
      return false;
   }
   if ( (firstGroup != NULL) && (secondGroup == NULL) ) {
      return false;
   }
   
   if ( (firstGroup != NULL) && (secondGroup != NULL) ) {
      result = mergeIndexAreas(firstGroup,secondGroup);
   }


   return result;
} // mergeIndexAreas

uint32
OldGenericMap::itemNameMaxLength( ItemTypes::itemType type ) {
   uint32 maxlen = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         OldItem* curItem = getItem(z, i);
         if ( (curItem != NULL) && (curItem->getItemType() == type ) ){
            for ( uint32 j=0; j<curItem->getNbrNames(); j++) {
               MC2String name = getName( curItem->getStringIndex ( j ) );
               //mc2log << info << "Item name: " << name << endl;
               if ( name.length() > maxlen)
                  maxlen = name.length();
            }   
         }
      }
   }

   return maxlen;
}

ItemTypes::roadDisplayClass_t
OldGenericMap::getRoadDisplayClass(uint32 itemID) const
{
   ItemTypes::roadDisplayClass_t result = 
         ItemTypes::nbrRoadDisplayClasses;
   ItemMap<uint32>::const_iterator it = m_roadDisplayClass.find(itemID);
   if (it != m_roadDisplayClass.end()){
      result = ItemTypes::roadDisplayClass_t(it->second);
   }
   return result;
}


void 
OldGenericMap::setRoadDisplayClass(
   uint32 itemID, ItemTypes::roadDisplayClass_t dispClass)
{
   if ( dispClass >= ItemTypes::nbrRoadDisplayClasses ) {
      mc2log << error << "Invalid road display class " << int(dispClass)
             << " for item " << itemID << endl;
      MC2_ASSERT(false);
   }
   ItemMap<uint32>::iterator it = m_roadDisplayClass.find(itemID);
   if (it != m_roadDisplayClass.end()){
      it->second = uint32(dispClass);
   }
   else {
      m_roadDisplayClass.insert(
            make_pair(itemID, uint32(dispClass)));
   }
}

void
OldGenericMap::printRoadDisplayClassInfo()
{
   // Collect nbr item of each display class
   map<ItemTypes::roadDisplayClass_t,uint32> roadDispClasses;
   map<ItemTypes::roadDisplayClass_t,uint32>::iterator rdcIt;
   ItemMap< uint32 >::iterator dispIt;
   for ( dispIt = m_roadDisplayClass.begin();
         dispIt != m_roadDisplayClass.end(); dispIt++ ) {
      ItemTypes::roadDisplayClass_t rdClass =
         ItemTypes::roadDisplayClass_t(dispIt->second);
      rdcIt = roadDispClasses.find(rdClass);
      if ( rdcIt == roadDispClasses.end() ) {
         roadDispClasses.insert(make_pair(rdClass, 1));
      } else {
         rdcIt->second++;
      }
   }

   uint32 nbrStreetSegmentItems = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(getItem(z, i));
         if ( ssi == NULL ) {
            continue;
         }
         nbrStreetSegmentItems++;
      }
   }
   
   for ( rdcIt = roadDispClasses.begin();
         rdcIt != roadDispClasses.end(); rdcIt++ ) {
      mc2dbg1 << "Map " << this->getMapID()
              << " road display class=" << int(rdcIt->first)
              << " " 
              << ItemTypes::getStringForRoadDisplayClass(rdcIt->first)
              << " nbrItems=" << rdcIt->second 
              << "  (" 
              << (float64(rdcIt->second)/nbrStreetSegmentItems) * 100
              << " %)" << endl;
   }
}

ItemTypes::areaFeatureDrawDisplayClass_t
OldGenericMap::getAreaFeatureDrawDisplayClass(uint32 itemID) const
{
   ItemTypes::areaFeatureDrawDisplayClass_t result = 
         ItemTypes::nbrAreaFeatureDrawDisplayClasses;
   ItemMap<uint32>::const_iterator it = 
         m_areaFeatureDrawDisplayClass.find(itemID);
   if (it != m_areaFeatureDrawDisplayClass.end()){
      result = ItemTypes::areaFeatureDrawDisplayClass_t(it->second);
   }
   return result;
}


void 
OldGenericMap::setAreaFeatureDrawDisplayClass(
   uint32 itemID, 
   ItemTypes::areaFeatureDrawDisplayClass_t dispClass,
   bool usePriority)
{
   if ( dispClass >= ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
      mc2log << error << "Invalid area feature draw display class " 
             << int(dispClass) << " for item " << itemID << endl;
      MC2_ASSERT(false);
   }
   ItemMap<uint32>::iterator it = m_areaFeatureDrawDisplayClass.find(itemID);
   if (it != m_areaFeatureDrawDisplayClass.end()){
      if( !usePriority ) {
         it->second = uint32(dispClass);
      } else {
         if( NationalProperties::AFDDCPriority ( dispClass ) > 
                NationalProperties::AFDDCPriority ( 
                   ( ItemTypes::areaFeatureDrawDisplayClass_t )it->second ) ) {
            it->second = uint32(dispClass);
         } 
      }  
   }
   else {
      m_areaFeatureDrawDisplayClass.insert(
            make_pair(itemID, uint32(dispClass)));
   }
}

bool
OldGenericMap::removeAreaFeatureDrawDisplayClass( uint32 itemID )
{
   bool result = false;
   ItemMap< uint32 >::iterator dispIt =
      m_areaFeatureDrawDisplayClass.find(itemID);
   if ( dispIt != m_areaFeatureDrawDisplayClass.end() ){
      m_areaFeatureDrawDisplayClass.erase(dispIt);
      result = true;
   }
   return result;
}

void
OldGenericMap::printAreaFeatureDrawDisplayClassInfo( bool detailedPrint )
{
   // Collect nbr item of each display class
   map<ItemTypes::areaFeatureDrawDisplayClass_t,uint32> aFDDispClasses;
   map<ItemTypes::areaFeatureDrawDisplayClass_t,uint32>::iterator adcIt;
   
   ItemMap< uint32 >::iterator dispIt;
   for ( dispIt = m_areaFeatureDrawDisplayClass.begin();
         dispIt != m_areaFeatureDrawDisplayClass.end(); dispIt++ ) {
      ItemTypes::areaFeatureDrawDisplayClass_t afddClass =
         ItemTypes::areaFeatureDrawDisplayClass_t(dispIt->second);
      adcIt = aFDDispClasses.find(afddClass);
      if ( adcIt == aFDDispClasses.end() ) {
         aFDDispClasses.insert(make_pair(afddClass, 1));
      } else {
         adcIt->second++;
      }

      if ( detailedPrint ) {
         mc2dbg1 << "AFDDC Item " << this->getMapID() << ";" << dispIt->first
                 << " " 
                 << ItemTypes::getStringForAreaFeatureDrawDisplayClass(afddClass)
                 << " " << getFirstItemName(dispIt->first)
                 << endl;
      }
   }

   for ( adcIt = aFDDispClasses.begin();
         adcIt != aFDDispClasses.end(); adcIt++ ) {
      mc2dbg1 << "Map " << this->getMapID()
              << " area feature draw display class=" << int(adcIt->first)
              << " "
              << ItemTypes::getStringForAreaFeatureDrawDisplayClass(
                     adcIt->first)
              << " nbrItems=" << adcIt->second << endl;
   }
}


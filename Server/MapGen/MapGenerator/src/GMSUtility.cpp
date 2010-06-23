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
#include "GMSUtility.h"
#include "OldExternalConnections.h"
#include "Utility.h"
#include "GfxUtility.h"
#include "GMSGfxData.h"
#include "GfxFilterUtil.h"
#include "Stack.h"
#include "OldStreetSegmentItem.h"
#include "OldMapHashTable.h"
#include "OldMapFilter.h"
#include "FilteredCoord.h"
#include "OldNode.h"
#include "NationalProperties.h"
#include "MapBits.h"

int
GMSUtility::addExternalConnection( OldGenericMap* theMap,
                                   GMSMap::extdata_t data)
{
   int retVal = 0;
   
   uint32 connectFromNodeIndex;
   OldConnection* tmpCon;
   uint32 curItemID;
   OldRouteableItem* curItem;
   GfxData* gfx;
   theMap->m_segmentsOnTheBoundry->sort();

   OldBoundrySegment::closeNode_t closeNodeVal;
   for (uint32 i=0; i < theMap->m_segmentsOnTheBoundry->getSize(); i++) {
      OldBoundrySegment* bs = (OldBoundrySegment*)
            theMap->m_segmentsOnTheBoundry->getElementAt(i);
      curItemID = bs->getConnectRouteableItemID();
      closeNodeVal = bs->getCloseNodeValue();
      
      curItem = (OldRouteableItem*) theMap->itemLookup(curItemID);
      gfx = curItem->getGfxData();
      const float64 maxSquareDist = 25; //meters squared

      float64 node0dist = maxSquareDist + 1;  // +1 to make sure not to round
      float64 node1dist = maxSquareDist + 1;
      // The z-level of the curItem:s closeNode. 
      int8 curLevel = 0;

      // Calculate the distance to the node
      switch (closeNodeVal) {
         case OldBoundrySegment::node0close :
            node0dist = 
               GMSUtility::getDistanceToNode(0, gfx, data.lat, data.lon);
            curLevel = theMap->nodeLookup( curItemID )->getLevel();
            break;
         case OldBoundrySegment::node1close :
            node1dist = 
               GMSUtility::getDistanceToNode(1, gfx, data.lat, data.lon);
            curLevel = 
               theMap->nodeLookup( curItemID | 0x80000000 )->getLevel();
            break;
         default :
            mc2log << fatal << here << "Unknown closeNodeVal "
                   << int(closeNodeVal) << endl;
            exit (1);
      }

      // Test START=======
      DEBUG4(
         if ( (GMSUtility::getDistanceToNode(0, gfx, data.lat, 
                                         data.lon) < 100000) ||
              (GMSUtility::getDistanceToNode(1, gfx, data.lat, 
                                         data.lon) < 100000)) {
            mc2dbg4 << "curItemID=" << curItemID << ", dist0="
              << GMSUtility::getDistanceToNode(0, gfx, data.lat, data.lon)
                    << ", dist1="
              << GMSUtility::getDistanceToNode(1, gfx, data.lat, data.lon)
                    << endl;
         }
      );
      /// TEST END

      // Assure that the nodes are on the same z-level if creating 
      // an external connection.
      // We have found two nodes that are close to each other,
      // node "curItem, closeNode" and "data.mapID, data.nodeID"
      if ( ( curLevel == data.level ) &&
           ( ( node0dist < maxSquareDist ) || 
             ( node1dist < maxSquareDist ) ) ) {

         // Get the node id of the remote node closest to the boundry
         connectFromNodeIndex = data.nodeID;

         DEBUG1(
            {
               uint32 coordIndex = 0;
               float64 coordDist = node0dist;
               if (node1dist < node0dist) {
                  coordIndex = gfx->getNbrCoordinates(0)-1;
                  coordDist = node1dist;
               }
               mc2dbg1 << "To add connection: (" << data.lat << "," 
                       << data.lon << ") <--> (" << gfx->getLat(0,coordIndex)
                       << "," << gfx->getLon(0,coordIndex) << "), dist=" 
                       << coordDist << endl;
            }
         );


         // Add the connection to the local node _on_ the boundry
         connectFromNodeIndex ^= 0x80000000;    // Flip bit 0 of remote id
         mc2dbg1 << "  Adding connection " << data.mapID << "."
                 << connectFromNodeIndex << " --> " << theMap->getMapID() 
                 << "." << curItemID << endl;
         tmpCon = new OldConnection(connectFromNodeIndex);
         bs->addConnection(tmpCon, data.mapID, false);
         
         retVal++;
         
      } else {
         mc2dbg8 << "No connections to " << curItemID << endl;
         // If the distance is "close enough" and the level differ
         // write a warning
         if ( ( node0dist < maxSquareDist ) || 
              ( node1dist < maxSquareDist ) ) {
            mc2log << warn << "Virtual nodes are close enough, but"
                   << " the level differs, trying to add ext conn in map "
                   << theMap->getMapID() << " from map "
                   << data.mapID << " nodeID " << data.nodeID
                   << " (" << data.lat << "," << data.lon << ")" << endl;
         }
      }
   }

   return retVal;

}


float64
GMSUtility::getDistanceToNode( byte nodeNbr, GfxData* gfx, 
                               int32 lat, int32 lon )
{
   uint32 n = 0;
   if (nodeNbr > 0) {
      n = gfx->getNbrCoordinates(0) - 1;
   }
   int32 otherLat = gfx->getLat(0,n);
   int32 otherLon = gfx->getLon(0,n);

   return (  GfxUtility::squareP2Pdistance_linear( otherLat, otherLon,
                                                   lat, lon));
}


void 
GMSUtility::filterCoordinates(OldGenericMap* theMap)
{
   uint32 unfilteredCoords = 0;
   uint32 filteredCoords = 0;
   uint32 totalCoords = 0;
   uint32 totalCoordsToFilter = 0;
   double e1 = 0;
   double e2 = 0;

   uint32 processedItems = 0;
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      mc2dbg1 << "=== Filtering on zoom " << i << "===" << endl;
      mc2dbg << "items on " << i << ":" << theMap->getNbrItemsWithZoom(i)
             << endl;
      for (uint32 j=0; j < theMap->getNbrItemsWithZoom(i); j++) {
         mc2dbg8 << "i=" << i << " j=" << j << endl;
         OldItem* item = theMap->getItem(i, j);
         if (item != NULL ) {
            if ( filterItem(*item, 
                            *theMap,
                            e1, 
                            e2,
                            totalCoords,
                            totalCoordsToFilter,
                            unfilteredCoords,
                            filteredCoords) ){
               processedItems++;
            }
            if ( (processedItems % 1000) == 0 ){
               mc2dbg << "Processed: " << processedItems << " items." << endl;
            }
         }
      }
   }
   mc2log << info << "Total error before filter: " << e1 << endl;
   mc2log << info << "Total error after filter: " << e2 << endl;

   mc2log << info << "unfilteredCoords = " << unfilteredCoords
          << ", filteredCoords = " << filteredCoords
          << ", totalCoords = " << totalCoords 
          << " ( coordsToFilter = " << totalCoordsToFilter << ")" << endl;

} // filterCoordinates



bool 
GMSUtility::filterItem( OldItem& item, 
                        OldGenericMap& theMap,
                        double& e1, double& e2, 
                        uint32& totalCoords, uint32& totalCoordsToFilter,
                        uint32& unfilteredCoords, uint32& filteredCoords 
                 ){
   bool result = false;
   GfxData* gfx = item.getGfxData();
   mc2dbg8 << "Item:" << item.getID() << " type:" << item.getItemType() 
          << endl;
   
   if ( gfx != NULL ) {
      mc2dbg8 << "Nbr coords of item " << item.getID() << ": " 
              << gfx->getTotalNbrCoordinates() << endl; 
      
      totalCoords += gfx->getTotalNbrCoordinates();
      
      // Max dist used when filtrating streets in the country overview map is
      // larger.
      uint32 streetMaxDist = 1;
      uint32 streetRdbtMaxDist = 1;
      if ( MapBits::isCountryMap(theMap.getMapID()) ){
         streetMaxDist = 175;
         streetRdbtMaxDist = 175;
      }
         
      // Check if to filter this item
      bool filter = false;
      bool openFiltering = false;
      uint32 maxDist = 0;
      bool useKolensikov = true;
      switch ( item.getItemType() ) {
      case ( ItemTypes::streetSegmentItem ): {
         openFiltering = true;
         useKolensikov = false;
         filter = true;
         const OldStreetSegmentItem* ssi = 
            static_cast<const OldStreetSegmentItem*> ( &item );
         if ( ssi->isRoundabout() ) {
            maxDist = streetMaxDist;
         } else {
            maxDist = streetRdbtMaxDist;
         }
         break;
      }
      case ( ItemTypes::ferryItem ):
         openFiltering = true;
         filter = true;
         maxDist = 1;
         break;
      case ( ItemTypes::builtUpAreaItem ):
         openFiltering = false;
         filter = true;
         maxDist = 5;
         break;
      case ( ItemTypes::waterItem ):
         openFiltering = false;
         filter = true;
         maxDist = 1;
         break;
      case ( ItemTypes::forestItem ):
      case ( ItemTypes::parkItem ):
      case ( ItemTypes::buildingItem ):
      case ( ItemTypes::islandItem ):
         openFiltering = false;
         filter = true;
         maxDist = 5;
         break;
      default:
         break;
      }
      if ( filter ) {
         mc2dbg8 << "Filtering" << endl;
         GfxDataFull* newGfx = 
            GMSGfxData::createNewGfxData( &theMap );
         totalCoordsToFilter += gfx->getTotalNbrCoordinates();

         for ( uint16 p = 0; p < gfx->getNbrPolygons(); ++p ) {
            mc2dbg8 << "Open polygon filter of p: " << p << endl;
            Stack coordIdx;
            bool filterOK = false;
            if ( openFiltering ) {
               filterOK = gfx->openPolygonFilter( &coordIdx, p,
                                                  maxDist, 
                                                  MAX_UINT32, 
                                                  true // minimize error
                                                  );
            } else {
               filterOK = gfx->openPolygonFilter( &coordIdx, p, 
                                                  maxDist, 
                                                  MAX_UINT32, 
                                                  true // minimize error
                                                  );
               // The closedPolygon filtering does not work very good,
               // so use the openPolyFilter instead
               //      filterOK = gfx->closedPolygonFilter( &coordIdx, 0,
               //                                100,
               //                                MAX_UINT32, true );
            }
            mc2dbg8 << "Open polygon filter done" << endl;
            
            // Use the new filtering method.
            vector<int> refFilterIndecesVec;
            for ( uint32 i = 0; i < coordIdx.getStackSize(); ++i ) {
               refFilterIndecesVec.push_back(
                                             coordIdx.getElementAt( i ) );
            }
            

            GfxData::const_iterator beginIt = gfx->polyBegin( p );
            GfxData::const_iterator endIt = gfx->polyEnd( p );
            
            vector<int> indeces;

            mc2dbg8 << "Get total error" << endl;
            // Get the error before the Kolesnikov filter is applied.
            e1 += GfxFilterUtil::getTotalError( refFilterIndecesVec,
                                                MC2CoordXYHelper(),
                                                beginIt, endIt);
            

            if (useKolensikov){
               
               mc2dbg8 << "Kolensikov filtering" << endl;
               // Get the error after the filter is applied to 
               // compare the result with the original.
               e2 += GfxFilterUtil::filter( indeces,   
                                            refFilterIndecesVec, 
                                            MC2CoordXYHelper(), 
                                            beginIt, endIt );
               mc2dbg8 << "Kolensikov filtering done" << endl;            
            }
            else {
               //swap(indeces,refFilterIndecesVec);
               indeces = refFilterIndecesVec;
            }
            mc2dbg8 << refFilterIndecesVec.size() << "->" 
                    << indeces.size() << endl; 


            for ( uint32 k = 0; k < indeces.size(); ++k ) {
               newGfx->addCoordinate( gfx->getLat( p, indeces[k] ),
                                      gfx->getLon( p, indeces[k] ),
                                      k == 0 ? true : false );
            }
            
            unfilteredCoords += gfx->getNbrCoordinates( p );
            filteredCoords += coordIdx.getStackSize();
            newGfx->setClosed( p, gfx->getClosed( p ) );
         }
         newGfx->updateLength();
         item.setGfxData( newGfx );
         result = true;
      }
   }
   return result;
}


void
GMSUtility::filterCoordinateLevels( OldGenericMap* theMap )
{
   mc2log << info << "Filter coordinates to 16 filter levels in map 0x" 
          << hex << theMap->getMapID() << dec << endl;
   OldMapFilter mapFilter;
   set<OldGenericMap*> mapsToConsider;
   mapsToConsider.insert( theMap );

   int minAreaFilterDistance = 3;
   int minLineFilterDistance = 1;

   if ( MapBits::isCountryMap( theMap->getMapID() ) ) {
      minLineFilterDistance = 50;
   }

   // Decide which items that should be filtered together, and the
   // filter distances for each group of items
   typedef set<ItemTypes::itemType> itemGroup_t;
   map<itemGroup_t, vector<int> > itemFilterGroups;
   map<itemGroup_t, vector<int> >::iterator filterGroupIt;

   // Define area item types and filter distances
   itemGroup_t areaItemTypes;
   areaItemTypes.insert( ItemTypes::waterItem );
   areaItemTypes.insert( ItemTypes::parkItem );
   areaItemTypes.insert( ItemTypes::builtUpAreaItem );
   areaItemTypes.insert( ItemTypes::individualBuildingItem );
   areaItemTypes.insert( ItemTypes::forestItem ); //?
   areaItemTypes.insert( ItemTypes::buildingItem );
   areaItemTypes.insert( ItemTypes::islandItem );
   
   vector<int> areaFilterMaxDists;
   int filterDist = 1;
   for( uint32 level = 1; level < 16; level++ ) {
      filterDist = mapFilter.getFiltDistanceForAreaItem(
                        level, minAreaFilterDistance );
      areaFilterMaxDists.push_back( filterDist );
   }
   itemFilterGroups.insert( make_pair(areaItemTypes, areaFilterMaxDists) );
   
   // Define line item types and filter distances
   itemGroup_t lineItemTypes;
   lineItemTypes.insert( ItemTypes::streetSegmentItem );
   lineItemTypes.insert( ItemTypes::ferryItem );
   lineItemTypes.insert( ItemTypes::railwayItem );
   
   vector<int> lineFilterMaxDists;
   filterDist = 1;
   for( uint32 level = 1; level < 16; level++ ) {
      filterDist = mapFilter.getFiltDistanceForLineItem(
                         level, minLineFilterDistance );
      lineFilterMaxDists.push_back( filterDist );
   }
   itemFilterGroups.insert( make_pair(lineItemTypes, lineFilterMaxDists) );
   

   // First filter all items to remove coordinates "on a string"
   mc2dbg << "Filtering to remove coordinates \"on a string\"" << endl;
   vector<int> firstFilterMaxDists;
   firstFilterMaxDists.push_back( minAreaFilterDistance );
   mapFilter.filter( theMap, mapsToConsider, areaItemTypes,
                     false, firstFilterMaxDists );
   firstFilterMaxDists.clear();
   firstFilterMaxDists.push_back( minLineFilterDistance );
   mapFilter.filter( theMap, mapsToConsider, lineItemTypes,
                     false, firstFilterMaxDists );
   
   // Remove coordinates on filter level 0, keep the ones on filter level 1
   // Remember to only affect those item types that have been filtered,
   // for all other items filter level stored in lat+lon is undefined
   itemGroup_t allItemTypesToFilter;
   for ( itemGroup_t::const_iterator it = areaItemTypes.begin();
         it != areaItemTypes.end(); it++ ) {
      allItemTypesToFilter.insert( *it );
   }
   for ( itemGroup_t::const_iterator it = lineItemTypes.begin();
         it != lineItemTypes.end(); it++ ) {
      allItemTypesToFilter.insert( *it );
   }
   uint32 nbrItemsWithCoordsFiltered = 0;
   uint32 nbrCoordsBeforeFilter = 0;
   uint32 nbrCoordsAfterFilter = 0;
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < theMap->getNbrItemsWithZoom(z); i++ ) {
         OldItem* item = theMap->getItem(z, i);
         
         if ( item == NULL ) {
            continue;
         }

         GfxData* gfx = item->getGfxData();
         if ( gfx == NULL ) {
            continue;
         }

         if ( allItemTypesToFilter.find( item->getItemType() ) == 
              allItemTypesToFilter.end() ) {
            continue;
         }

         nbrCoordsBeforeFilter += gfx->getTotalNbrCoordinates();
         nbrItemsWithCoordsFiltered++;

         GfxDataFull* newGfx = GMSGfxData::createNewGfxData( theMap );
         for ( uint32 p = 0; p < gfx->getNbrPolygons(); p++ ) {
            newGfx->addPolygon();
            GfxDataFilterIterator end = gfx->endFilteredPoly( p, 1 );
            for ( GfxDataFilterIterator it = gfx->beginFilteredPoly( p, 1 );
                  it != end; ++it ) {
               newGfx->addCoordinate( (*it).lat, (*it).lon );
            }
            newGfx->setClosed( p, gfx->getClosed( p ) );
            newGfx->updateLength();
         }
         item->setGfxData( newGfx );
         nbrCoordsAfterFilter += newGfx->getTotalNbrCoordinates();
      }
   }
   mc2dbg << "Removed coordinates from " << nbrItemsWithCoordsFiltered
          << " filtered items, nbr coords " << nbrCoordsBeforeFilter
          << " -> " << nbrCoordsAfterFilter << endl;

   // Filter the map again to get the 16 filtering levels
   mc2dbg << "Filtering levels" << endl;
   for ( filterGroupIt = itemFilterGroups.begin();
         filterGroupIt != itemFilterGroups.end(); filterGroupIt++ ) {
      mapFilter.filter( theMap, mapsToConsider, filterGroupIt->first,
                        false, filterGroupIt->second );
   }

   // All items of other item types than the ones filtered need to have
   // filter level default set to 15. Otherwise displaying these items
   // based on filterIterators will be completely at random.
   uint32 nbrItemsWithCoordsUnfiltered = 0;
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0; i < theMap->getNbrItemsWithZoom(z); i++ ) {
         OldItem* item = theMap->getItem(z, i);
         
         if ( item == NULL ) {
            continue;
         }

         GfxData* gfx = item->getGfxData();
         if ( gfx == NULL ) {
            continue;
         }

         if ( allItemTypesToFilter.find( item->getItemType() ) != 
              allItemTypesToFilter.end() ) {
            continue;
         }

         // Border items were created with co pol filtering so they are ok.
         if ( item->getItemType() == ItemTypes::borderItem ) {
            continue;
         }

         nbrItemsWithCoordsUnfiltered++;

         GfxDataFull* newGfx = GMSGfxData::createNewGfxData( theMap );
         for ( uint32 p = 0; p < gfx->getNbrPolygons(); p++ ) {
            newGfx->addPolygon();
            GfxData::iterator end = gfx->polyEnd( p );
            for ( GfxData::iterator it = gfx->polyBegin( p );
                  it != end; it++ ) {
               MC2Coordinate coord( it->lat, it->lon );
               static_cast<FilteredCoord&>(coord).setFilterLevel(15);
               newGfx->addCoordinate( coord.lat, coord.lon );
            }
            newGfx->setClosed( p, gfx->getClosed( p ) );
            newGfx->updateLength();
         }
         item->setGfxData( newGfx );
      }
   }
   mc2dbg << "Filter level set to 15 in " << nbrItemsWithCoordsUnfiltered
          << " unfiltered items." << endl;
   
   theMap->setMapFiltered( true );

   mc2log << info << "Filtered coordinates to 16 filter levels in map 0x" 
          << hex << theMap->getMapID() << dec << endl;
}

bool
GMSUtility::filterCountryPolygonLevels(
              OldGenericMap* theMap, set<MC2Coordinate> breakPoints )

{
   if ( ! MapBits::isCountryMap( theMap->getMapID() ) ) {
      mc2log << error << "filterCountryPolygonLevels map "
             << hex << theMap->getMapID() << dec
             << " is not co map" << endl;
      return false;
   }

   if ( theMap->mapGfxDataIsFiltered() ) {
      mc2log << info << "filterCountryPolygonLevels map 0x"
             << hex << theMap->getMapID() << dec << " (" 
             << theMap->getMapName()
             << ") already has filtered map gfx." << endl;
      return false;
   }

   mc2log << info << "Filter country polygon to 16 filter levels in map 0x" 
          << hex << theMap->getMapID() << dec << " ("
          << theMap->getMapName() << ")" << endl;
   OldMapFilter mapFilter;

   // -------- FILTER AND REMOVE COORDS ON A STRING ?? BEGIN -----
   /*
   // First filter all items to remove coordinates "on a string"
   mc2dbg << "Filtering to remove coordinates \"on a string\"" << endl;
   vector<int> firstFilterMaxDists;
   firstFilterMaxDists.push_back( 1 );
   mapFilter.filterCountryPolygon( theMap, firstFilterMaxDists );

   const GfxData* mapGfx = theMap->getGfxData();
   GfxData* newGfx = GMSGfxData::createNewGfxData( theMap );
   uint32 nbrCoordsBefore = mapGfx->getTotalNbrCoordinates();
   for ( uint32 p = 0; p < mapGfx->getNbrPolygons(); p++ ) {
      newGfx->addPolygon();
      GfxDataFilterIterator end = mapGfx->endFilteredPoly( p, 1 );
      for ( GfxDataFilterIterator it = mapGfx->beginFilteredPoly( p, 1 );
            it != end; it++ ) {
         newGfx->addCoordinate( (*it).lat, (*it).lon );
      }
      newGfx->setClosed( p, mapGfx->getClosed( p ) );
      newGfx->updateLength();
   }
   theMap->setGfxData( newGfx );
   mc2dbg << "OldMap country polygon before " << nbrCoordsBefore
          << " coords, now " << newGfx->getTotalNbrCoordinates() << endl;
   */
   // -------- FILTER AND REMOVE COORDS ON A STRING ?? END -------
   
   // Define the filter level distances
   vector<int> coPolFilterMaxDists;
   int coFilterDist;
   for( uint32 level = 1; level < 16; level++ ) {
      coFilterDist = mapFilter.getFiltDistanceForMapGfx( level );
      coPolFilterMaxDists.push_back( coFilterDist );
   }
   
   // Filter
   mapFilter.filterCountryPolygon( theMap, coPolFilterMaxDists, breakPoints );
   
   // Mark the map gfx data filtered
   theMap->setMapGfxDataFiltered( true );
   
   mc2log << info << "Filtered country polygon to 16 filter levels in map 0x" 
          << hex << theMap->getMapID() << dec << endl;
   return true;
}


bool
GMSUtility::extractCoPolBreakPoints(
      const char* breakPointFile, set<MC2Coordinate>& breakPoints )
{
   // Country polygon break points file, fields delimited with spaces
   //
   //  lat lon description
   //
   // e.g.
   //  823915938 245139346 swe-fin-no
   //  785357201 288128788 swe-fin-ocean
   //
   
   bool retVal = true;
   
   ifstream file(breakPointFile);
   
   if ( file ) {
      uint32 maxLength = 500;
      char buf[maxLength];
      buf[0] = '\0';
      char* dest;
      int32 lat, lon;
      while ( ! file.eof() ) {
         file.getline( buf, maxLength, ' ');
         if ( Utility::getNumber(buf, dest, lat) ) {
            file.getline( buf, maxLength, ' ');
            if ( Utility::getNumber(buf, dest, lon) ) {
               MC2Coordinate coord( lat, lon );
               breakPoints.insert( coord );
               file.getline( buf, maxLength);
            }
         }
      }
   
   } else {
      retVal = false;
   
   }

   return retVal;
}

void
GMSUtility::createBorderBoundrySegments(const char* mapPath, uint32 startAtMap)
{
   // Loop all maps in this directory and identify candidates for
   // creating boundry segments.
   // All routeable item nodes that are outside the map gfx data or inside and
   // close (5 meters) to the map border should be marked. Also include 
   // coordinates for nodes that are marked as borderCrossing (junction_t).
   // (these might have been added before).
   // Collect all coordinates.
   // Use a set<> in order for one coordinate not to be stored several times.

   // The definition of close = 5 meters is used from addExternalConnection

   set<MC2Coordinate> borderCoordsByMap;
   set<MC2Coordinate>::const_iterator bCIt;

   uint32 curMapID = startAtMap;
   bool cont = true;

   uint32 nbrCoordCandidates = 0;
   while ( cont ) {
      OldGenericMap* curMap = OldGenericMap::createMap(curMapID, mapPath);
      if ( curMap == NULL ) {
         cont = false;
      } else {

         // Possible improvement (speed-up)
         // 1. Loop the map and collect all node coordinates in a set
         // 2. Check inside/close for all coords in the set
         //    (not checking inside while looping the map)

         
         // Make sure the boundry segments vector is not NULL
         if ( curMap->getBoundrySegments() == NULL ) {
            curMap->createOldBoundrySegmentsVector();
         }
         
         set<MC2Coordinate> noBorderCoords;
         uint32 nbrCandidatesFromMap = 0;
         nbrCoordCandidates = borderCoordsByMap.size();
         mc2dbg1 << "Finding boundry segment candidates in map "
                 << curMap->getMapID() << endl;
         
         const GfxData* mapGfx = curMap->getGfxData();
         MC2BoundingBox mapBBox;
         mapGfx->getMC2BoundingBox(mapBBox);
         
         for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
            for ( uint32 i = 0; i < curMap->getNbrItemsWithZoom(z); i++ ) {
               OldRouteableItem* rItem =
                  dynamic_cast<OldRouteableItem*>(curMap->getItem(z, i));
               if ( rItem == NULL ) {
                  continue;
               }
               
               GfxData* riGfx = rItem->getGfxData();
               if ( riGfx == NULL ) {
                  continue;
               }
               if ( curMap->getBoundrySegments()
                     ->getBoundrySegment(rItem->getID()) != NULL ) {
                  continue;
               }

               // If this routeable item is outside or inside and close
               // to the map gfx data border.
               bool candidate[2];
               candidate[0] = false;
               candidate[1] = false;
               int32 lat[2];
               int32 lon[2];
               MC2Coordinate mc2coords[2];
               // get the coordinates for first and last coord of the gfx
               lat[0] = riGfx->getLat(0,0);
               lon[0] = riGfx->getLon(0,0);
               lat[1] = riGfx->getLastLat(0);
               lon[1] = riGfx->getLastLon(0);
               mc2coords[0] = MC2Coordinate( lat[0], lon[0] );
               mc2coords[1] = MC2Coordinate( lat[1], lon[1] );
               
               // 1. Check if the nodes are marked as borderCrossing
               // (these might have been added before, but that is ok).
               for ( uint32 n = 0; n < 2; n++ ) {
                  if ( rItem->getNode(n)->getJunctionType() == 
                           ItemTypes::borderCrossing ) {
                     candidate[n] = true;
                     mc2dbg2 << " node " << rItem->getNode(n)->getNodeID()
                             << " borderCrossing " << endl;
                  }
               }

               // 2. Check if the nodes are outside border, or inside and close
               // map bbox
               MC2BoundingBox riBBox;
               riGfx->getMC2BoundingBox(riBBox);
               if ( ! riBBox.inside(mapBBox) ) {
                  // The ri is outside the map bbox, both nodes candidates
                  candidate[0] = true;
                  candidate[1] = true;

               } else {
                  // Check if the node coordinates are already present in
                  // the noBorderCoords coordinate sets
                  for ( uint32 c = 0; c < 2; c++ ) {
                     if ( noBorderCoords.find(mc2coords[c]) !=
                              noBorderCoords.end() ) {
                        // already found inside map = no borderCoord
                     } else {
                        // Check if the ri is outside or less than 5 meters
                        // inside the map gfx
                        int64 dist =
                           mapGfx->signedSquareDistTo(lat[c], lon[c]);
                        if ( dist > -25 ) {
                           candidate[c] = true;
                        }
                     }
                  }
                  
               }

               for ( uint32 c = 0; c < 2; c++ ) {
                  if ( candidate[c] ) {
                     mc2dbg1 << "Candidate item " << rItem->getID()
                             << " node " << c << " coord " << mc2coords[c]
                             << endl;
                     borderCoordsByMap.insert( mc2coords[c] );
                     nbrCandidatesFromMap++;
                  } else {
                     noBorderCoords.insert( mc2coords[c] );
                  }
               }
               
               
            }
         }

         mc2log << info << "Found " << nbrCandidatesFromMap
                << " candidates resulting in "
                << borderCoordsByMap.size() - nbrCoordCandidates
                << " border coordinates from map " << curMap->getMapID()
                << endl;
         
         curMapID = MapBits::nextMapID(curMapID);
      }

      delete curMap;
   }

   DEBUG8(
   for ( bCIt = borderCoordsByMap.begin();
         bCIt != borderCoordsByMap.end(); bCIt++ ) {
      mc2dbg << "Coordinate candidates " << bCIt->lat << " " << bCIt->lon
             << endl;
   });
   
   // Loop all maps again and find all nodes that are close to the candidate 
   // coordinates. For all such border nodes, create a virtual item and 
   // add it to the boundry segments vector of the map.
   
   curMapID = startAtMap;
   cont = true;
   if ( borderCoordsByMap.size() == 0 ) {
      mc2dbg1 << "No border coordinates found! Can't create virtual items"
              << endl;
      cont = false;
   }
   
   while ( cont ) {
      GMSMap* curMap = static_cast<GMSMap*>
                       (GMSMap::createMap(curMapID, mapPath));
      if ( curMap == NULL ) {
         cont = false;
      } else {
         
         OldMapHashTable* mht = curMap->getHashTable();
         mht->clearAllowedItemTypes();
         mht->addAllowedItemType(ItemTypes::streetSegmentItem);
         mht->addAllowedItemType(ItemTypes::ferryItem);
         
         // Loop the border coordinates and check if there are any
         // close routeable items in this map. // Collect the node ids.

         set<uint32> borderNodes;

         for ( bCIt = borderCoordsByMap.begin();
               bCIt != borderCoordsByMap.end(); bCIt++ ) {
            
            int32 candidateLat = bCIt->lat;
            int32 candidateLon = bCIt->lon;
            bool killCloseIDs = false;
            Vector* closeIDs = mht->getAllWithinRadius_meter(
                  candidateLon, candidateLat, 0, killCloseIDs);

            if ( (closeIDs == NULL) || (closeIDs->getSize() == 0) ) {
               if ( killCloseIDs )
                  delete closeIDs;
               continue;
            }
            
            mc2dbg4 << "Coord " << candidateLat << " " << candidateLon
                    << ": " << closeIDs->getSize() << " close items" << endl;
            for ( uint32 i = 0; i < closeIDs->getSize(); i++ ) {
               
               OldRouteableItem* rItem = dynamic_cast<OldRouteableItem*>
                     (curMap->itemLookup( closeIDs->getElementAt(i) ));
               // Make sure we have no 0-length item
               if ( (rItem == NULL) || (rItem->getGfxData() == NULL) ||
                    (rItem->getGfxData()->getLength(0) <= 0) ) {
                  continue;
               }
               // Decide which node is the one close to the coordinate
               int32 nodeLat = MAX_INT32, nodeLon = MAX_INT32;
               float64 minSqDist = MAX_FLOAT64;
               uint32 closeNodeId = MAX_UINT32;
               for ( uint32 n = 0; n < 2; n++ ) {
                  int c = 0;
                  if ( n == 1 ) {
                     c = rItem->getGfxData()->getNbrCoordinates(0) - 1;
                  }
                  nodeLat = rItem->getGfxData()->getLat(0, c);
                  nodeLon = rItem->getGfxData()->getLon(0, c);
                  
                  float64 cosLat = GfxUtility::getCoslat(
                     MIN(nodeLat, candidateLat), MAX(nodeLat, candidateLat));
                  float64 nodeSqDist = GfxUtility::squareP2Pdistance_linear(
                        candidateLat, candidateLon, nodeLat, nodeLon, cosLat);
                  if ( nodeSqDist < minSqDist ) {
                     closeNodeId = rItem->getNode(n)->getNodeID();
                     minSqDist = nodeSqDist;
                  }
               }

               // Ok, node closeNodeId was closest to the candidate coord
               mc2dbg8 << "Border item " << rItem->getID() << " close node "
                       << closeNodeId << " dist " << minSqDist << endl;
               borderNodes.insert( closeNodeId );
            }
            
            
            if ( killCloseIDs ) {
               delete closeIDs;
            }
         }

         mc2dbg1 << "Found " << borderNodes.size() << " border nodes" << endl;

         // Make sure the boundry segments vector is not NULL
         if ( curMap->getBoundrySegments() == NULL ) {
            curMap->createOldBoundrySegmentsVector();
         }
         
         // For every collected node, make a virtual item
         // and add to boundry segments vector.
         // If a virtual item already exists (e.g. because of borderNodes in
         // GMSMidMifHandler) nothing will happen
         uint32 nbrVirtuals = 0;
         for ( set<uint32>::const_iterator nit = borderNodes.begin();
               nit != borderNodes.end(); nit++ ) {
            if ( curMap->addToBoundary( *nit ) != MAX_UINT32 ) {
               nbrVirtuals++;
            }
         }

         // Sort the boundry segments
         curMap->getBoundrySegments()->sort();
         
         mc2log << info  << "Created " << nbrVirtuals 
                << " virtual boundry segments "
                << "from " << borderNodes.size() << " border nodes" << endl;
         
         // Save the map if any boundry segments were created.
         if ( nbrVirtuals > 0 ) {
            curMap->save();
         }
         
         curMapID = MapBits::nextMapID(curMapID);
      }
      
      delete curMap;
   }

}


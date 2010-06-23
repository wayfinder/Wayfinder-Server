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

#include "OldMapFilter.h"
#include "MapFilterUtil.h"

#include "OldGenericMap.h"

#include "StringUtility.h"
#include "GMSGfxData.h"
#include "MapBits.h"

// Using two last bits in each of lat and lon for storing coordinate level
const uint32 OldMapFilter::nrOflsbsToBeUsed = 2;
const int OldMapFilter::coordBlankMask = 
         ( 0xffffffff >> nrOflsbsToBeUsed ) << nrOflsbsToBeUsed; //0xfffffffc

OldMapFilter::OldMapFilter()
{
   m_mapGfxDistDefined = false;
   m_mapGfxHighLevelDistDefined = false;
   m_areaItemDistDefined = false;
   m_lineItemDistDefined = false;
}

OldMapFilter::~OldMapFilter()
{
}

void
OldMapFilter::filter( OldGenericMap* mapToFilter,
                      const set<OldGenericMap*>& mapsToConsider,
                      const set<ItemTypes::itemType>& itemTypes,
                      bool  useGfxData,
                      vector<int> filterMaxDists )
{
   mc2log << info << "OldMapFilter::filter map 0x"
          << hex << mapToFilter->getMapID() << dec << endl;
   
   // Check item types
   if ( itemTypes.size() == 0 ) {
      mc2log << info << "OldMapFilter::filter map 0x" << hex 
             << mapToFilter->getMapID() << dec << " no item types to filter"
             << endl;
      return;
   }
   mc2log << info << "OldMapFilter::filter map 0x"
          << hex << mapToFilter->getMapID() << dec << " item types:";
   for ( set<ItemTypes::itemType>::iterator it = itemTypes.begin();
         it != itemTypes.end(); it++ ) {
      if ( it != itemTypes.begin() )
         mc2log << ",";
      mc2log << " " << ItemTypes::getItemTypeAsString( *it );
   }
   mc2log << endl;

   // Check filter level distances
   if ( ! validFilterLevelDistances( filterMaxDists ) ) {
      return;
   }
   

   coordMap_t coordIntersectMap;

   // Send all maps in to be added to the coordIntersectMap.
   mc2dbg2 << "Begin update of cim." << endl;
   for ( set<OldGenericMap*>::const_iterator 
         it = mapsToConsider.begin();
         it != mapsToConsider.end(); ++it ) { 
      updateCoordIntersectionMap(
            *it,
            itemTypes,
            useGfxData,
            coordIntersectMap);
   }
   mc2dbg2 << "End update of cim, contains " << coordIntersectMap.size()
           << " coordinates" << endl;

   filterUsingInfoInCoordsIntersectionMap(
         mapToFilter,
         itemTypes,
         useGfxData,
         coordIntersectMap,
         filterMaxDists);
   
   mc2log << info << "OldMapFilter::filter map 0x" << hex
          << mapToFilter->getMapID() << dec << " - done" << endl;
}

void
OldMapFilter::filterCountryPolygon( OldGenericMap* mapToFilter,
            vector<int> filterMaxDists,
            set<MC2Coordinate> breakPoints )
{
   mc2log << info << "OldMapFilter::filter country polygon map 0x"
          << hex << mapToFilter->getMapID() << dec << endl;
   
   if ( ! MapBits::isCountryMap( mapToFilter->getMapID() ) ) {
      mc2dbg << "Map is not a cuntry overview map!" << endl;
      return;
   }
   
   if ( mapToFilter->getGfxData() == NULL ) {
      mc2log << error << "Country map 0x" << hex << mapToFilter->getMapID()
             << dec << " has no map gfx data. Exit!" << endl;
      return;
   }
   
   // Check filter level distances
   if ( ! validFilterLevelDistances( filterMaxDists ) ) {
      return;
   }
   
   // Collect coordinates to the coordIntersectMap, the id pairs
   // will be (mapID, countryCode)
   coordMap_t coordIntersectMap;
   mc2dbg2 << "Begin update of cim." << endl;
   updateCoordIntersectionMapForMapGfx( mapToFilter, coordIntersectMap);
   mc2log << info << "End update of cim, contains "
          << coordIntersectMap.size() << " co pol coordinates" << endl;

   // Also add flag in coords that are break points = defining common lines
   // between neighbouring country polygons, id pair (mapID, countryCode)
   // Use a dummy countryCode for the break points, starting at MAX_UINT32
   // Each break point should have a uniq "countryCode" to handle if there
   // are several breakpoints in a row
   uint32 nbrBreakPoints = 0;
   uint32 bpId = MAX_UINT32;
   for ( set<MC2Coordinate>::const_iterator it = breakPoints.begin();
         it != breakPoints.end(); it++ )  {
      MC2Coordinate coord( it->lat & coordBlankMask, it->lon & coordBlankMask );
      coordMap_t::iterator cit = coordIntersectMap.find( coord );
      if ( cit != coordIntersectMap.end() ) {
         IDPair_t idPair( mapToFilter->getMapID(), bpId );
         bpId--;
         set<IDPair_t>& idSet = (*cit).second;
         idSet.insert( idPair );
         nbrBreakPoints++;
         mc2dbg1 << "Break point " << *it << " added to cim" << endl;
      }
   }
   mc2log << info << "Added " << nbrBreakPoints << " break point flags for "
          << mapToFilter->getMapName() << endl;


   // Filter the country polygon
   const GfxData* countryGfx = mapToFilter->getGfxData();
   GfxDataFull* newGfx = GMSGfxData::createNewGfxData( mapToFilter );
   filterOneGfxUsingInfoInCoordsIntersectionMap( 
      mapToFilter, countryGfx, newGfx, false,
      coordIntersectMap, filterMaxDists, true );
   mapToFilter->setGfxData( newGfx );
   
   mc2log << info << "OldMapFilter::filter country polygon map 0x" << hex
          << mapToFilter->getMapID() << dec << " - done" << endl;
}

void 
OldMapFilter::filterUsingInfoInCoordsIntersectionMap( 
            OldGenericMap* theMap,
      const set<ItemTypes::itemType>& itemTypes,
      bool  useGfxData,
            coordMap_t& coordIntersectMap,
            vector<int> filterMaxDists)
{
   mc2dbg << "OldMapFilter::filterUsingInfoInCoordsIntersectionMap map 0x"
          << hex << theMap->getMapID() << dec << endl;
   bool dumpFilterInfo = false;
   
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      mc2dbg4 << "NGZ loop." << endl;
      for (uint32 j=0; j<theMap->getNbrItemsWithZoom(i); j++) {
         OldItem* item = theMap->getItem(i, j);
         if (item != NULL ) {
            
            // Check if the item is of the specified types.
            if ( itemTypes.find( item->getItemType() ) !=
                 itemTypes.end() ) {
               // Is of specified type.
               
               GfxData* gfx = item->getGfxData();
               if ( gfx != NULL) {
                  GfxDataFull* newGfx = GMSGfxData::createNewGfxData( theMap );

                  filterOneGfxUsingInfoInCoordsIntersectionMap( 
                     theMap, gfx, newGfx, useGfxData, coordIntersectMap,
                     filterMaxDists, false, dumpFilterInfo );

                  if ( dumpFilterInfo ) {
                     cout << " nbr coords=" << newGfx->getTotalNbrCoordinates()
                          << endl;
                  }
                  
                  item->setGfxData( newGfx );
               }
               else {
                  mc2dbg2
                     << "OldMapFilter::filterUsingInfoInCoordsIntersectionMap "
                     << "item " << item->getID() << " has no gfx data" << endl;
               }
            }
         }
      }
   }
   mc2dbg << "OldMapFilter::filterUsingInfoInCoordsIntersectionMap map 0x"
          << hex << theMap->getMapID() << dec << " done" << endl;
}

void 
OldMapFilter::filterOneGfxUsingInfoInCoordsIntersectionMap( 
            OldGenericMap* theMap,
            const GfxData* gfx,
            GfxDataFull* newGfx,
            bool  useGfxData,
            coordMap_t& coordIntersectMap,
            vector<int> filterMaxDists,
            bool countryBorder,
            bool dumpFilterInfo )
{
   if ( gfx == NULL ) {
      return;
   }

   for ( uint32 p = 0; 
         (p < gfx->getNbrPolygons());
         ++p ) {
      mc2dbg8 << " Filtering polygon p=" << p << " nbr coords=" 
           << gfx->getNbrCoordinates(p) << " length="
           << gfx->getLength(p) << endl;
      newGfx->addPolygon();
      
      MC2Coordinate firstCoord(
            gfx->getLat( p, 0 ) & coordBlankMask,
            gfx->getLon( p, 0 ) & coordBlankMask);
      coordMap_t::iterator cimBeginIt =
         coordIntersectMap.find( firstCoord );

      // Find coord to start from (a break point if such exists), compare
      // with cimBeginIt until a difference is found
      bool startFromBreakPoint = false;
      bool weHaveBreakPoints = false;
      int32 k_start = -1;
      coordMap_t::iterator startFromIt = cimBeginIt;
      GfxData::const_iterator end = gfx->polyEnd( p );
      for ( GfxData::const_iterator iter = gfx->polyBegin( p );
            iter != end; iter++ ) {
         k_start++;
         MC2Coordinate coord( iter->lat & coordBlankMask,
                              iter->lon & coordBlankMask );
         coordMap_t::iterator cand = coordIntersectMap.find(coord);
         if ( cand->second != cimBeginIt->second ) {
            // found break point
            startFromBreakPoint = true;
            weHaveBreakPoints = true;
            if ( cand->second.size() < cimBeginIt->second.size() ) {
               // iter-- is the break point
               k_start--;
            } else {
               // iter is the break point
               startFromIt = cand;
            }
            break;
         }
         startFromIt = cand;
      }
      if ( startFromBreakPoint && (k_start == 0) ) {
         // we will actually go from coord 0, so use this marker to detect
         // that we must filter from 0 -> nbrCoordsInPoly-1+1
         startFromBreakPoint = false;
      }
      if ( !startFromBreakPoint ) {
         // start from the beginning...
         k_start = 0;
         startFromIt = cimBeginIt;
      }
      DEBUG1(
      if ( countryBorder && ((p == 0) || (p == 1)) ) {
         mc2dbg << "poly " << p << " start filt from coord " << k_start << " " 
            << gfx->getLat( p, k_start) << "," << gfx->getLon( p, k_start) 
            << " = masked " << (gfx->getLat( p, k_start) & coordBlankMask)
            << "," << (gfx->getLon( p, k_start) & coordBlankMask)
            << " " << startFromIt->first 
            << " cimBegin.size = " << startFromIt->second.size()
            << " breakpoints=" << int(weHaveBreakPoints)
            << " startFromBreakPoint=" << int(startFromBreakPoint)
            << endl;
      });
      
      const uint32 nbrCoordsInPoly = gfx->getNbrCoordinates( p );
      uint32 nbrCoordsInNewPoly = newGfx->getNbrCoordinates( p );
      uint32 k_prev = k_start;
      cimBeginIt = startFromIt;
      bool moreToFilter = true;
      bool filterBeyond = false;
      
      uint32 k = k_start;
      while ( moreToFilter ) {
        // Don't forget to blank out the LSB:s.
         MC2Coordinate coord(
               gfx->getLat( p, k ) & coordBlankMask, 
               gfx->getLon( p, k ) & coordBlankMask);
         coordMap_t::iterator cimEndIt = 
            coordIntersectMap.find( coord );
         if(  ( !( (*cimBeginIt).second == (*cimEndIt).second) ) ||
              ( !startFromBreakPoint && (k == nbrCoordsInPoly - 1) )  ) {
            
            if ( dumpFilterInfo ) {
               cout << "k_prev=" << k_prev << " k=" << k 
                    << " (" << nbrCoordsInPoly << ") begin.size="
                    << (*cimBeginIt).second.size() << " end.size="
                    << (*cimEndIt).second.size() << endl;
            }
            
            int sizeDiff = (*cimEndIt).second.size()
                                 - (*cimBeginIt).second.size();
            if ( ( sizeDiff > 0 ) || 
                 ( !startFromBreakPoint && (k == nbrCoordsInPoly - 1) ) ) {
               filterBeyond = true;
               if ( startFromBreakPoint &&
                    (nbrCoordsInNewPoly == 0) && (k_prev == k) ) {
                  // filter from break point, and poly has no other but this
                  // start break point. We end up here when having looped
                  // around poly one time = back at start break point
                  mc2dbg8 << " filter beyond is not to be!" << endl;
                  filterBeyond = false;
               }
            } else {
               filterBeyond = false;
            }

            if ( filterBeyond ) {
               k++;
            }
            mc2dbg8 << " startFromBreakPoint=" << int(startFromBreakPoint)
                    << " filterBeyond=" << int(filterBeyond) << endl;

            // Adjust opfEnd if we are filtering to the end of the polygon
            // to reach all the way (israel - palestine) ***
            // watch out if we are dealing with break points or not (oman)
            if ( (k == nbrCoordsInPoly) && (k_prev != 0) ) {
               if ( !startFromBreakPoint ) {
                  mc2dbg8 << "  adjusting k from " << k << " -> "
                          << k+1 << endl;
                  k++;
               }
            }
            
            DEBUG8(
            if ( countryBorder && ((p == 0) || (p == 1)) ) {
               mc2dbg << " co pol filter1 " << k_prev << "->" << k 
                      << " (" << nbrCoordsInPoly << ")" << endl;
            });
            filterSegment(
                  theMap,
                  MC2CoordXYHelper(),
                  k_prev, k,
                  filterMaxDists, 
                  p, 
                  gfx,
                  newGfx,
                  countryBorder,weHaveBreakPoints,
                  dumpFilterInfo );
          
            k_prev = k;
            if ( k_prev > 0 ) {
               k_prev--;
            } else {
               mc2dbg8 << "tried to make k_prev negative" << endl;
               k_prev = nbrCoordsInPoly-1;
            }
            
            if ( filterBeyond ) {
               k--;
            }
            else if ( sizeDiff == 0 ) {
               // filter the last segment (k-- -> k), the above
               // filterSegment did not include k
               DEBUG8(
               if ( countryBorder && ((p == 0) || (p == 1)) ) {
                  mc2dbg << " co pol filter2 " << k-1 << "->" << k+1 << endl;
               });
               filterSegment(
                     theMap,
                     MC2CoordXYHelper(),
                     (k-1), (k+1),
                     filterMaxDists, 
                     p, 
                     gfx,
                     newGfx,
                     countryBorder, weHaveBreakPoints,
                     dumpFilterInfo );
               // do not decrease k_prev, since k was filtered
               k_prev++;
            }
            nbrCoordsInNewPoly = newGfx->getNbrCoordinates( p );
         }
         cimBeginIt = cimEndIt;

         // prepare for check of next coordinate
         k++;
         // if filtered to the end of this poly, continue at coord 0
         if ( (k == nbrCoordsInPoly) || ( k > nbrCoordsInPoly) /* ** */ ) {
            k = 0;
         }
         // Check if all parts of this polygon have been filtered to newGfx
         // go to next poly:
         if ( countryBorder && weHaveBreakPoints ) {
            // if we have coords in newGfx and we are back at start point
            if ( (nbrCoordsInNewPoly > 0) && (cimBeginIt == startFromIt) ) {
               moreToFilter = false;
            }
         } else {
            // if the newGfx have all coords in this poly
            if ( nbrCoordsInNewPoly >= nbrCoordsInPoly ) {
               moreToFilter = false;
            }
         }
      } // end while (moreToFilter)
      newGfx->setClosed( p, gfx->getClosed( p ) );
      newGfx->updateLength();
      
      if ( startFromBreakPoint && !countryBorder ) {
         // Now we have actually moved the coordinates around, so the poly
         // of newGfx does not start with the same coordinate as the poly
         // of gfx does.
         // Also: the break point where filtering started from were added 
         // twice to newGfx (both as coord 0 and coord n). (only when a
         // poly has more than one break point...)
         //
         // When re-uing pre-filtered country polygons from neighbouring
         // countries, the number of ccords in newGfx is not the same as
         // number of coords in original map gfx - > we can't do this.
         // Duplicated break point can be removed with removeIdenticalCoords
         //
         if ( dumpFilterInfo ) {
            cout << " re-build coord order of poly filtered from break point"
                 << endl;
         }

         GfxDataFull* tmpGfx = GMSGfxData::createNewGfxData( theMap, true );
         const uint32 nbrCoordsInNew = newGfx->getNbrCoordinates(p);
         bool duplicatedCoordInNew = ( nbrCoordsInPoly != nbrCoordsInNew );
         const uint32 startIdx = ( newGfx->getNbrCoordinates(p) - k_start -1 );
         bool cont = true;
         uint32 c = startIdx;
         while ( cont ) {
            tmpGfx->addCoordinate( newGfx->getLat(p, c),
                                   newGfx->getLon(p, c) );
            c = ( (c+1) % nbrCoordsInNew );
            if ( duplicatedCoordInNew && (c == 0) ) {
               c = 1;
            }
            if ( c == startIdx ) {
               cont = false;
            }
         }
         tmpGfx->setClosed( 0, gfx->getClosed( p ) );
         tmpGfx->updateLength();
         c = 0;
         GfxData::const_iterator end = tmpGfx->polyEnd( 0 );
         for ( GfxData::const_iterator it = tmpGfx->polyBegin( 0 );
               it != end; it++ ) {
            newGfx->setCoordinate( p, c, it->lat, it->lon);
            c++;
         }
         if ( duplicatedCoordInNew ) {
            newGfx->deleteCoordinate(p, c);
         } else if ( dumpFilterInfo ) {
            mc2dbg8 << " not deleting last (no duplicated) in new gfx" << endl;
         }
      }
      else if ( weHaveBreakPoints && countryBorder) {
         newGfx->removeIdenticalCoordinates();
      }
      
      mc2dbg8 << " Filtered polygon p=" << p << " nbr coords=" 
              << newGfx->getNbrCoordinates(p) << endl;
   }

}

void
OldMapFilter::updateCoordIntersectionMapForMapGfx(
            OldGenericMap* theMap,
            coordMap_t& coordIntersectMap )
{
   const GfxData* mapGfx = theMap->getGfxData();

   // Fill the coordIntersectMap with coordinates from this country polygon
   // Let idPair be mapId+"countryId" (really we probably want top region id)
   
   for ( uint32 p = 0; p < mapGfx->getNbrPolygons(); ++p ) {
      GfxData::const_iterator end = mapGfx->polyEnd( p );
      for ( GfxData::const_iterator git = mapGfx->polyBegin( p );
            git != end; git++ ) {
         MC2Coordinate coord(  git->lat & coordBlankMask,
                               git->lon & coordBlankMask );
         // Build id pair from mapID, countryCode
         IDPair_t idPair( theMap->getMapID(), int( theMap->getCountryCode()) );
         coordMap_t::iterator it = coordIntersectMap.find( coord );

         // Fixme:
         // Fix this so updateCoordIntersectionMapForMapGfx
         // and updateCoordIntersectionMap uses same methods...
         // ...               
         if ( it == coordIntersectMap.end() ) {
            set<IDPair_t> idSet;
            idSet.insert( idPair );
            coordIntersectMap.insert( make_pair( coord, idSet ) );
         }
         else {
            set<IDPair_t>& idSet = (*it).second;
            idSet.insert( idPair );
         }
         // ...
         
      }
   }
}

void 
OldMapFilter::updateCoordIntersectionMap( OldGenericMap* theMap,
      const set<ItemTypes::itemType>& itemTypes,
      bool useGfxData,
      coordMap_t& coordIntersectMap )
{
   mc2dbg << "OldMapFilter::updateCoordIntersectionMap" << endl;
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      mc2dbg4 << "NGZ loop." << endl;
      for (uint32 j=0; j<theMap->getNbrItemsWithZoom(i); j++) {
         OldItem* item = theMap->getItem(i, j);
         if (item != NULL ) {
            // Check if the item is of the specified types.
            if ( itemTypes.find( item->getItemType() ) !=
                  itemTypes.end() ) {
               // Is of specified type.
               GfxData* gfx = item->getGfxData();

               if ( gfx != NULL) {

                  // ------ break from here ----------
                  // send itemID/countryID as inparam so we can create idPairs
                  // regardless of item gfxs or map gfx (co polygon)
                  for ( uint32 p = 0; 
                        p < gfx->getNbrPolygons(); 
                        ++p ) {
                     GfxData::const_iterator end = gfx->polyEnd( p );
                     for ( GfxData::const_iterator git = gfx->polyBegin( p );
                           git != end; git++ ) {
                        // Don't forget to blank out the LSB:s.
                        mc2dbg4 << "Blank out lsb's.." << endl;
                        MC2Coordinate coord( git->lat & coordBlankMask, 
                                             git->lon & coordBlankMask );
                        mc2dbg4 << "Lat = " << coord.lat
                           << ", Lon = " << coord.lon << endl;
                        IDPair_t idPair( theMap->getMapID(), item->getID() );
                        coordMap_t::iterator it = 
                           coordIntersectMap.find( coord );
                        if ( it == coordIntersectMap.end() ) {
                           // The coord didn't exist in the map.
                           // Add it.
                           set<IDPair_t> idSet;
                           idSet.insert( idPair );
                           coordIntersectMap.insert( 
                                 make_pair( coord, idSet ) );
                           mc2dbg4 << "This was a new one." << endl;
                        } else {
                           // The coordinate already exists.
                           // Append the id to the existing set.
                           //(*it).second.insert( idPair );
                           set<IDPair_t>& idSet = (*it).second;
                           idSet.insert( idPair );
                           mc2dbg4 << "This was a old one." << endl;
                           for ( set<IDPair_t>::iterator jt = idSet.begin();
                                 jt != idSet.end(); ++jt ) {
                              mc2dbg8 << *jt << endl;
                           }
                        }
                     }
                  }
                  // ------- to here -----------
               } else {
                  mc2dbg2 << "OldMapFilter::updateCoordIntersectionMap item "
                          << item->getID() << " has no gfx data" << endl;
               }
               
            }
         }
      }
   }
   mc2dbg << "Done updating coordIntersectMap, contains " 
          << coordIntersectMap.size() << " coordinates" << endl;
}


bool
OldMapFilter::validFilterLevelDistances( vector<int> filterMaxDists )
{
   if ( filterMaxDists.size() > 15 ) {
      mc2log << error << "Can not process " << filterMaxDists.size()
             << " filter levels, max is 15 (level 0 = unfiltered)" << endl;
      exit(1);
   }
   
   mc2log << info << "OldMapFilter::filter distances:";
   for ( vector<int>::iterator it = filterMaxDists.begin();
         it != filterMaxDists.end(); it++ ) {
      mc2log << " " << *it;
   }
   mc2log << endl;

   return true;
}

void
OldMapFilter::printCountryBorderToFile(
      MC2Coordinate firstBreakPoint,
      MC2Coordinate lastBreakPoint,
      MC2Coordinate onTheWayPoint,
      const char* countryMapName, GfxData* printGfx,
      const uint32 poly,
      int startIdx, int nbrCoordsInBorderPart)
{
   ofstream outfile("countryBorders.txt", ios::app);
   
   // First the key, identification coords are blanked
   //    BORDERPART co map name
   //    startBreakPoint (mc2lat,mc2lon)
   //    endBreakPoint (mc2lat,mc2lon)
   //    pointOnTheWay (mc2lat,mc2lon) (50% offset from start->end)
   outfile << "BORDERPART " << countryMapName << endl
           << firstBreakPoint << endl
           << lastBreakPoint << endl
           << onTheWayPoint << endl;
   
   // Then number of coordinates in this part
   //    nbrCoords
   outfile << nbrCoordsInBorderPart << endl;
   
   // Then all the coordinates of this part (including both break points)
   // True filtered coordinates (not blanked).
   //    mc2lat mc2lon
   int endBefore = startIdx + nbrCoordsInBorderPart;
   if ( endBefore > int(printGfx->getNbrCoordinates(poly)) ) {
      mc2log << error << "printCountryBorderToFile Printing invalid "
             << "coord idxs" << endl;
   }
   for( int j = startIdx; j < endBefore; j++ ) {
      outfile << " " << printGfx->getLat( poly, j )
              << " " << printGfx->getLon( poly, j ) << endl;
   }
   
}

bool
OldMapFilter::oldBorderFiltering(
      GfxData* gfxPartToFilter,
      const char* countryMapName,
      MC2Coordinate firstBreakPoint, MC2Coordinate lastBreakPoint,
      MC2Coordinate onTheWayPoint,
      bool gfxPartForward,
      GfxDataFull* outGfx, uint32 polyNbr )
{
   ifstream borderFile("countryBorders.txt");
   const int maxLineLength = 200;
   char lineBuffer[maxLineLength];
   lineBuffer[0] = '\0';
   
   // Border file structure
   //
   //    BORDERPART germany_1
   //    (startBreakPointLat,startBreakPointLon)
   //    (endBreakPointLat,endBreakPointLon)
   //    (pointOnTheWayLat,pointOnTheWayLon) // 50% offset from start->end
   //    nbrCoords
   //    mc2lat mc2lon
   //    mc2lat mc2lon
   //    ...
   
   // Loop the file with "border parts" until we have read the whole file,
   // or until we have found our border
   bool foundBorder = false;

   // read the first line from the file with borders
   borderFile.getline( lineBuffer, maxLineLength );
   
   uint32 nbrBordersInFile = 0;
   while ( !foundBorder && !borderFile.eof() && (strlen(lineBuffer) > 0) ) {

      if ( strstr(lineBuffer, "BORDERPART") != NULL ) {
         nbrBordersInFile++;
         
         // Read co map name for which this borderpart was written
         const char* borderCountryMapName = 
               StringUtility::newStrDup(lineBuffer + strlen( "BORDERPART "));
         
         // The three coordinates
         MC2Coordinate firstCoord, lastCoord, middleCoord;
         borderFile.getline( lineBuffer, maxLineLength );
         MapFilterUtil::coordFromString( lineBuffer, firstCoord );
         borderFile.getline( lineBuffer, maxLineLength );
         MapFilterUtil::coordFromString( lineBuffer, lastCoord );
         borderFile.getline( lineBuffer, maxLineLength );
         MapFilterUtil::coordFromString( lineBuffer, middleCoord );

         // Check if this is the border we want to filter
         // Identifier coords in file and inparam identifier coords
         // have been blanked with coordBlankMask
         mc2dbg8 << " to check if we match border " << nbrBordersInFile << endl;
         bool add = false;
         if ( ((firstBreakPoint == firstCoord) &&
               (lastBreakPoint == lastCoord)) ||
              ((firstBreakPoint == lastCoord) &&
               (lastBreakPoint == firstCoord)) ) {
            add = true;
            mc2dbg << "oldBorderFiltering breakpoints matches border " 
                   << nbrBordersInFile << endl;
         }
         if ( add ) {
            // Check if the distance from the middle point of the pre-stored
            // filtered border to this partToFilter is small enough
            // -> the same border (with gap)
            float64 sqDist;
            bool ok = coordCloseToBorderPart( gfxPartToFilter, 
                                              middleCoord, sqDist );
            if ( ok ) {
               mc2dbg << "oldBorderFiltering breakpoints matches, dist " 
                      << "to middleCoord = " << sqDist << endl;
            } else {
               // Too far from the stored border to our check point
               add = false;
               mc2dbg << "oldBorderFiltering breakpoints matches, but " 
                      << "checkpoint not ok (" << sqDist << " meters"
                      << ", border length=" << gfxPartToFilter->getLength(0) 
                      << "meters)" << endl;
            }
         }

         if ( add ) {
            foundBorder = true;
            // If the countryMapName.mif (this map) is not equal to the 
            // borderCountryMapName.mif (read from country borders file):
            // - this is a true country border (on land) and interesting
            //   for creating "border items"
            MC2String thisName = StringUtility::copyLower(countryMapName);
            MC2String thisMifName = thisName + ".mif";
            DataBuffer thisBuf;
            thisBuf.memMapFile( thisMifName.c_str() );
            
            MC2String borderName = 
               StringUtility::copyLower(borderCountryMapName);
            MC2String borderMifName = borderName + ".mif";
            DataBuffer borderBuf;
            borderBuf.memMapFile( borderMifName.c_str() );

            mc2dbg8 << " bufSize "
                    << thisMifName << "=" << thisBuf.getBufferSize() << " "
                    << borderMifName << "=" << borderBuf.getBufferSize()
                    << endl;
            
            if ( ! thisBuf.equals( borderBuf ) ) {
               // Print this border part to border item file if
               // it was not there before.
               printBorderItemToFile( 
                  nbrBordersInFile, firstCoord, lastCoord, middleCoord );
            }
         }
         
         // When just checking if this border part already was written in 
         // countryBorders.txt break loop to return..
         if ( add && ( outGfx == NULL ) ) {
            break;
         }

         if ( add ) {
            // read nbr coordinates and add to a filteredPart
            borderFile.getline( lineBuffer, maxLineLength );
            uint32 nbrCoordsInPart = strtoul(lineBuffer, NULL, 10);
            GfxDataFull* filteredPart = GMSGfxData::createNewGfxData( static_cast<OldGenericMap *>( NULL ), true );
            for ( uint32 c = 0; c < nbrCoordsInPart; c++ ) {
               int32 lat, lon;
               borderFile >> lineBuffer;
               lat = strtol(lineBuffer, NULL, 10);
               borderFile >> lineBuffer;
               lon = strtol(lineBuffer, NULL, 10);
               filteredPart->addCoordinate( lat, lon );
            }
            mc2dbg << "Add the border part (" << nbrCoordsInPart
                   << " coords) " << "from file to outGfx" << endl;
            
            // check if to add filteredPart backwards or forwards to outGfx
            // don't add doubles
            if ( outGfx->getNbrCoordinates(polyNbr) > 0 ) {
               // some coord(s) alredy added to outGfx, filteredPart has one 
               // break point shared with outGfx
               // (both outGfx and filteredPart have filtered coords)
               int32 lastLat = outGfx->getLastLat( polyNbr );
               int32 lastLon = outGfx->getLastLon( polyNbr );
               mc2dbg << " outgfx with content ("
                    << outGfx->getNbrCoordinates(polyNbr) << " coords)"
                    << ", connecting at " << lastLat << "," << lastLon << endl;
               if ( (lastLat == filteredPart->getLat(0,0)) &&
                    (lastLon == filteredPart->getLon(0,0)) ) {
                  mc2dbg << " adding forwards" << endl;
                  GfxData::const_iterator end = filteredPart->polyEnd(0);
                  GfxData::const_iterator it = filteredPart->polyBegin(0);
                  it++;
                  for ( ; it != end; it++ ) {
                     outGfx->addCoordinate( it->lat, it->lon );
                  }
               } else {
                  // add backwards
                  mc2dbg << " adding backwards" << endl;
                  uint32 startAt = nbrCoordsInPart - 1 -1;
                  for ( int c = startAt; c >= 0; c-- ) {
                     outGfx->addCoordinate(
                        filteredPart->getLat(0,c), filteredPart->getLon(0,c));
                  }
               }
               mc2dbg << "outgfx now " << outGfx->getNbrCoordinates(polyNbr)
                       << " coords" << endl;
            } else {
               // Add the whole filteredPart to outGfx, use gfxPartToFilter to
               // find out if to add forwards or backwards
               // (filteredPart has filtered coords, gfxPartToFilter has not)
               mc2dbg << " outgfx without content" << endl;
               bool matchesForward = true;
               if ( ( (gfxPartToFilter->getLat(0,0) & coordBlankMask) == 
                           (filteredPart->getLat(0,0) & coordBlankMask) ) &&
                    ( (gfxPartToFilter->getLon(0,0) & coordBlankMask) ==
                           (filteredPart->getLon(0,0) & coordBlankMask) ) ) {
                  matchesForward = true;
               } else {
                  matchesForward = false;
               }

               if ( ( matchesForward && gfxPartForward ) ||
                    ( !matchesForward && !gfxPartForward ) ) {
                  mc2dbg << " adding forwards, 0->" << nbrCoordsInPart-1 << endl;
                  GfxData::const_iterator end = filteredPart->polyEnd(0);
                  GfxData::const_iterator it = filteredPart->polyBegin(0);
                  for ( ; it != end; it++ ) {
                     outGfx->addCoordinate( it->lat, it->lon );
                  }
               } else {
                  uint32 startAt = nbrCoordsInPart - 1;
                  mc2dbg << " adding backwards: " << startAt << "-> 0" << endl;
                  for ( int c = startAt; c >= 0; c-- ) {
                     outGfx->addCoordinate(
                        filteredPart->getLat(0,c), filteredPart->getLon(0,c));
                  }
               }
               mc2dbg << "outgfx now " << outGfx->getNbrCoordinates(polyNbr)
                       << " coords" << endl;
            }

         }
      }
      
      // read next line from the file with borders
      borderFile.getline( lineBuffer, maxLineLength );
   }

   if ( foundBorder ) {
      mc2log << info << "Found border part among pre-filtered, part " 
             << nbrBordersInFile << endl;
   }
   return foundBorder;
}

void
OldMapFilter::printBorderItemToFile( uint32 borderId,
         MC2Coordinate& firstCoord, MC2Coordinate& lastCoord,
         MC2Coordinate& middleCoord )
{
   // First check if this border part is in the border item file or not
   bool write = true;
   ifstream is("borderItems.txt");
   if ( is ) {
      const int maxLineLength = 200;
      char buf[maxLineLength];
      buf[0] = '\0';

      is >> buf;

      while ( write && !is.eof() && (strlen(buf) > 0) ) {
         if ( strstr(buf, "BORDERITEM") != NULL ) {
            // read the border part id
            is >> buf;
            uint32 id = strtoul(buf, NULL, 10);
            if ( id != MAX_UINT32 ) {
               if ( id == borderId ) {
                  write = false;
               }
            }
         }
         
         is >> buf;
      }
      
   }

   if ( write ) {
      mc2dbg << "Writing border part " << borderId
             << " to border item file" << endl;
      ofstream borderItemFile("borderItems.txt", ios::app);
      borderItemFile << "BORDERITEM " << borderId << endl
                     << " from " << firstCoord << endl
                     << " to   " << lastCoord << endl
                     << " via  " << middleCoord << endl;
   }
}

bool
OldMapFilter::coordCloseToBorderPart( const GfxData* borderPart,
                                   const MC2Coordinate coord, float64& sqDist )
{
   sqDist = 
      sqrt( double( borderPart->squareDistToLine( coord.lat, coord.lon) ) );
   float64 borderLength = borderPart->getLength(0);
   uint32 maxDist = 50;
   if ( borderLength > 150000 ) { // 150 km
      maxDist = 700;
   } else if ( borderLength > 100000 ) { // 100 km
      maxDist = 350;
   } else if ( borderLength > 50000 ) { // 50 km
      maxDist = 100;
   }
   
   if ( sqDist < maxDist ) {
      return true;
   }
   return false;
}


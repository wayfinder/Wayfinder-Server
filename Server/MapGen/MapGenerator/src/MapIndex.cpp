/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapIndex.h"

#include "MC2String.h"

#include "OldGenericMap.h"
#include "OldCountryOverviewMap.h"
#include "GMSGfxData.h"
#include "MapModuleNotice.h"
#include "MapGenUtil.h"
#include "ItemIdentifier.h"

#include "XMLIndata.h"
#include "XMLParserHelper.h"

#include "MapBits.h"

#include <map>
#include <sstream>

MapIndex::MapIndex(const char* mapPath)
{
   m_mapNotices = new WriteableMapModuleNoticeContainer();
   
   m_mapPath = mapPath;
   if ( m_mapPath[m_mapPath.size()-1] != '/' ){
      // Add slash if not there
      m_mapPath+="/";
   }

} // MapIndex::MapIndex

MapIndex::~MapIndex(){
   delete m_mapNotices;

} // ~MapIndex

void
MapIndex::load(){
   MC2String indexFileName = INDEX_NAME; // From config.h
   MC2String indexFileAndPath = m_mapPath+indexFileName;
   mc2log << info << "MapIndex::loading index: " << indexFileAndPath 
          << endl;

   m_mapNotices->load( indexFileAndPath.c_str() );

   mc2log << info << "MapIndex::index loaded." << endl;
} // MapIndex::load

void
MapIndex::save(){
   MC2String indexFileName = INDEX_NAME; // From config.h
   MC2String indexFileAndPath = m_mapPath+indexFileName;

   mc2log << info << "MapIndex::saving index: " << indexFileAndPath 
          << endl;

   m_mapNotices->save( indexFileAndPath.c_str() );

   mc2log << info << "MapIndex::index saved" << endl;
} // MapIndex::save

void
MapIndex::createFromMaps( const map<MC2String, MC2String>& 
                          overviewByUnderview,
                          const map<MC2String, MC2String>& 
                          superByOverview ) 
{
   mc2log << info << "MapIndex::createNewIndex called" << endl;


   // Map map name to map ID.
   mc2log << info << "MapIndex::createFromMaps "
          << "Reads names of maps in dir" << endl;
   map<MC2String, uint32> mapIDByName; // OldMaps map name to map ID.
   set<uint32> mapIDs;
   MapGenUtil::getMapIDsInDir( mapIDs, m_mapPath );
   set<uint32>::iterator mapID_it = mapIDs.begin();
   while (mapID_it != mapIDs.end()){

      uint32 curMapID = *mapID_it;
      OldGenericMapHeader* curMap = 
         new OldGenericMapHeader( curMapID, m_mapPath.c_str() );
      if ( ! curMap->load() ){
         mc2log << error << "MapIndex::createFromMaps "
                << "Could not load map with ID: 0x" << hex << curMapID
                << dec << endl;
         exit(1);
      }

      MC2String mapName = curMap->getMapName();
      mapIDByName.insert(pair<MC2String, uint32> (mapName, curMapID ) );

      delete curMap;
      curMap=NULL;
      ++mapID_it;
   }
   
   // Make sure that all map names are present in either in one of the XML
   // input files.
   map<MC2String, uint32>::const_iterator mapNameIt= 
      mapIDByName.begin();
   while ( mapNameIt != mapIDByName.end() ){
      bool ok = false;
      MC2String mapName = mapNameIt->first;
      // Check the underview maps of level 1 XML input.
      map<MC2String, MC2String>::const_iterator xmlIt = 
         overviewByUnderview.find(mapName);
      if ( xmlIt != overviewByUnderview.end() ){
         ok = true;
      }
      if ( ! ok ){
         // Check the overview maps of level 1 of XML input.
         xmlIt = overviewByUnderview.begin();
         while ( xmlIt != overviewByUnderview.end() ){
            MC2String ovrName = xmlIt->second;
            if ( ovrName == mapName ){
               ok = true;
            }
            ++xmlIt;
         }
      }
      if ( ! ok ){
         // Check the overview maps of XML input.
         xmlIt = superByOverview.find(mapName);  
         if ( xmlIt != superByOverview.end() ){
            ok = true;
         }
         if ( ! ok ){
            // Check the super overview maps of XML input.
            xmlIt = superByOverview.begin();
            while ( xmlIt != superByOverview.end() ){
               MC2String superName = xmlIt->second;
               if ( superName == mapName ){
                  ok = true;
               }
               ++xmlIt;
            }
         }
      }
      if (!ok){
         mc2log << error << "Map on disc missing in the supplied XML files."
                << endl;
         mc2dbg << "Missing map name: " << mapName << endl;
         MC2_ASSERT(false);
      }
      ++mapNameIt;
   }


   // Add map notices to index.db.
   mc2log << info << "MapIndex::createFromMaps "
          << "Adds overview by underview map notices" << endl;
   this->addMapNotices( mapIDByName, overviewByUnderview );

   mc2log << info << "MapIndex::createFromMaps "
          << "Adds super by overview map notices" << endl;
   this->addMapNotices( mapIDByName, superByOverview );

   mc2log << info << "MapIndex::createFromMaps "   
          << "Adds country map notices" << endl;
   this->addCountryMapNotices();

   // To use the maps, add regions and update creation time has to be run.
   mc2log << info << "MapIndex::createFromMaps Done!" << endl;   
      
} // createFromMaps


void
MapIndex::addMapNotices( map<MC2String, uint32> mapIDByName,
                          const map<MC2String, MC2String>&
                          higherMapByLowerMap )
{
   mc2dbg << "MapIndex::addMapNotices higherMapByLowerMap.size() = " 
          << higherMapByLowerMap.size() << endl;

   // Create a map with IDs instead of names.
   //
   // This is done so the maps are sorted in ID order when added.
   map<uint32, uint32> highIDByLowID;
   map<MC2String, MC2String>::const_iterator lowNameIt =
      higherMapByLowerMap.begin();
   while ( lowNameIt != higherMapByLowerMap.end() ){

      // The low map ID
      const MC2String lowerName = lowNameIt->first;
      map<MC2String, uint32>::const_iterator IDIt = 
         mapIDByName.find(lowerName);
      if ( IDIt == mapIDByName.end() ){
         mc2log << warn << "MapIndex::addMapNotices "
                << "Could not find lower map name:" << lowerName
                << " among maps in current directory" <<  endl;
         ++lowNameIt;
         continue;
      }
      uint32 lowID = IDIt->second;

      // The high map ID
      const MC2String higherName = lowNameIt->second;
      IDIt = mapIDByName.find(higherName);
      if ( IDIt == mapIDByName.end() ){
         mc2log << warn << "MapIndex::addMapNotices "
                << "Could not find higher map name:" << higherName
                << " among maps in current directory" <<  endl;
         if ( MapBits::isUnderviewMap(lowID) ){
            exit(1); // If this is underview to overview, we demand that
                     // the overview is in place when the underview is.
         }
         else{
            ++lowNameIt;
            continue; // Lower ID is an overview. It is OK that the
                      // super overview is missing.
         }
      }
      uint32 highID = IDIt->second;

      // Add them to the map.
      highIDByLowID.insert( make_pair(lowID, highID) );

      ++lowNameIt;
   }
   
   // The map tree to fill in below.
   ItemIDTree& mapTree = m_mapNotices->getMapTree();   





 
   // Loop lower IDs and add higher and lower map notices.
   map<uint32, uint32>::const_iterator lowerIt =
      highIDByLowID.begin();
   while ( lowerIt != highIDByLowID.end() ){
      // The IDs
      uint32 lowerID = lowerIt->first;
      uint32 higherID = lowerIt->second;

      // Add the maps to the map hierarchy.
      mapTree.addMap( higherID, lowerID );            




      // Add map module notice for higher maps and update map tree.
      // If this higher map is not already added.
      //
      //uint32 higherIDFromMapNotice = 
      //   m_mapNotices->getMapID( higherName.c_str() );
      //mc2log << info << "higherIDFromMapNotice = "
      //       << "0x" << hex << higherIDFromMapNotice << dec << endl;
      //if ( higherIDFromMapNotice == MAX_UINT32 ){
      MapModuleNotice* highNotice = 
         m_mapNotices->getMap( higherID );
      if ( highNotice == NULL ){
         OldGenericMap* higherMap = 
            OldGenericMap::createMap( higherID, m_mapPath.c_str() );
         
         MC2BoundingBox bbox;
         const GfxData* gfx = higherMap->getGfxData();
         for ( uint32 i = 0; i < gfx->getNbrCoordinates(0); i++ ) {
            bbox.update( gfx->getLat( 0, i ), gfx->getLon( 0, i ) );
         }
         GfxData* noticeGfx = GMSGfxData::createNewGfxData( static_cast<OldGenericMap* >( NULL ), &bbox );
         

         highNotice = new MapModuleNotice(higherMap->getMapID(),
                                          noticeGfx,
                                          higherMap->getCreationTime());
         highNotice->setMapName( higherMap->getMapName() );         
         // Set the country of the map notice
         highNotice->setCountryCode(higherMap->getCountryCode());
         // Print the name of the map together with the map ID
         mc2log << info << "Add notice ID: " 
                << higherMap->getMapID() << endl;

         m_mapNotices->addLast( highNotice );

         // Update map tree.
         mapTree.addMap( MAX_UINT32, higherID );     
         delete higherMap;
         higherMap = NULL;
      }
      

      // Add lower map notice.
      
      // Load lower map and create a map module notice for it.
      OldGenericMap* lowerMap = OldGenericMap::createMap(lowerID,
                                                   m_mapPath.c_str());
      // Error checking.
      if ( lowerMap == NULL ) {
         mc2log << error << "Could not load map ID:0x" << hex << lowerID
                << dec << endl;
         exit(1);
      }
      if ( m_mapNotices->getMapID( lowerMap->getMapName() ) != 
           MAX_UINT32 ) {
         mc2dbg << "Already added map module notice for map "
                << lowerMap->getMapName() << endl;
         ++lowerIt;
         delete lowerMap;
         lowerMap=NULL;
         continue;
      }
      // Add map module notice for the lower map.
      GfxData* noticeGfxData = 
         GMSGfxData::createNewGfxData( static_cast<OldGenericMap *>( NULL ), lowerMap->getGfxData() );
      MapModuleNotice *notice = 
         new MapModuleNotice( lowerID,
                              noticeGfxData,
                              lowerMap->getCreationTime());
      notice->setMapName( lowerMap->getMapName() );
      // Set the country of the map notice
      notice->setCountryCode(lowerMap->getCountryCode());
      // Print the name of the map together with the map ID
      mc2log << info << "Add notice ID: " << lowerID << endl;
      m_mapNotices->addLast(notice);


      delete lowerMap;
      lowerMap=NULL;


      ++lowerIt;
   } // Loop lower maps.
   

   m_mapNotices->setMapTree( mapTree );
} // addMapNotices


void
MapIndex::addCountryMapNotices()
{

   // Add the country-maps to the index.db-file.


   bool cont = true;
   uint32 countryMapID = FIRST_COUNTRYMAP_ID;
   while (cont) {
      OldCountryOverviewMap* countryMap = dynamic_cast<OldCountryOverviewMap*>
         (OldGenericMap::createMap(countryMapID, m_mapPath.c_str()));
      if (countryMap != NULL) {

            
         //const GfxData* coGfx = countryMap->getGfxData();
         //GfxData* noticeGfxData = coGfx->createNewConvexHull();

         //GfxData* noticeGfxData = 
         //   GMSGfxData::createNewGfxData( static_cast<OldGenericMap* >( NULL ), countryMap->getGfxData() );
         
         // Create gfx data for index.db (use bbox of the countryMap gxfData)
         MC2BoundingBox bbox;
         const GfxData* gfx = countryMap->getGfxData();
         for ( uint32 p = 0; p < gfx->getNbrPolygons(); p++ ) {
            for ( uint32 i = 0; i < gfx->getNbrCoordinates(p); i++ ) {
               bbox.update( gfx->getLat( p, i ), gfx->getLon( p, i ) );
            }
         }
         GfxData* noticeGfxData = GMSGfxData::createNewGfxData(
                     static_cast<OldGenericMap* >( NULL ), &bbox );
         
         MapModuleNotice *notice = new MapModuleNotice(  
                                            countryMapID,
                                            noticeGfxData,
                                            countryMap->getCreationTime());
         notice->setCountryMapName( countryMap->getMapName() );
         // Set the country of the map notice
         notice->setCountryCode(countryMap->getCountryCode());

         // Print the name of the map together with the map ID
         mc2log << info << "Add notice(name, ID): " 
                << countryMap->getMapName() << ", " 
                << countryMapID << endl;
         m_mapNotices->addLast(notice);
         countryMapID = MapBits::nextMapID(countryMapID);

          
      } else {
         cont = false;
      }

      // Delete the map to avoid memory leaks
      delete countryMap;
   }
} // addCountryMapNotices


void 
MapIndex::addRegions( XMLParserHelper::xmlByTag_t::const_iterator& begin,
                      XMLParserHelper::xmlByTag_t::const_iterator& end,
                      const map<MC2String, uint32>& regionIDsByIdent )
{
   // Remove existing regions (and the region tree).
   m_mapNotices->clearRegions();
   
   RegionIDTree regionTree;
   
   // Go through all the regions
   for ( XMLParserHelper::xmlByTag_t::const_iterator it = begin;
         it != end; ++it ) {

      if ( XMLString::equals( it->second->getNodeName(), "add_region" ) ) {
         DOMNamedNodeMap* attributes = it->second->getAttributes();
         DOMNode* attribute = 
            attributes->getNamedItem( X( "region_ident" ) );
         char* tmpStr = 
            XMLUtility::transcodefromucs( attribute->getNodeValue() );
         mc2dbg << "Add region from XML: " << tmpStr << endl;
      }

      
      NameCollection regionNames;
      TopRegionMatch::topRegion_t regionType;
      MC2String regionIdent;
      typedef multimap<MC2String, ItemIdentifier*> contents_t;
      contents_t contents;
      set<MC2String> insideRegions;
      
      // Parse xml data
      if (! XMLIndata::parseAddRegion( it->second,
                                       regionNames,
                                       regionType,
                                       regionIdent,
                                       contents,
                                       insideRegions ) ) {
         mc2log << error << "Could not parse regions. Exiting." << endl;
         exit(1);
      }
      
      // Find region id
      map<MC2String, uint32>::const_iterator regIDIt = 
         regionIDsByIdent.find( regionIdent );
      if ( regIDIt != regionIDsByIdent.end() ) {
         // Region id was found.
         if ( !insideRegions.empty() ) {
            for ( set<MC2String>::const_iterator it = insideRegions.begin();
                  it != insideRegions.end(); ++it ) {
               // Find parent region id
               map<MC2String, uint32>::const_iterator parentIDIt = 
                  regionIDsByIdent.find( *it );
                
               if ( parentIDIt != regionIDsByIdent.end() ) {
                  // Add parent region
                  mc2dbg << "OldMapReader::addRegions Adding region "
                         << regIDIt->first << " as child to "
                         << parentIDIt->first << endl;
                  regionTree.addRegion( parentIDIt->second, 
                                        regIDIt->second );
               } else {
                  mc2log << warn << here << "addRegions Invalid parent "
                         << "region " << *it << " supplied for region " 
                         << regionIdent << endl;
               }
            }
         } else {
            // This region is not inside another region.
            mc2dbg << "OldMapReader::addRegions Adding top region " 
                   << regIDIt->first << endl;
            regionTree.addRegion( MAX_UINT32, regIDIt->second );
         }
      } else {
         mc2log << error << here << " addRegions region " << regionIdent
                << " not found." << endl;
         exit(1);
      }

      
      ItemIDTree idTree;
      
      // Add content.
     
      // Go through all maps.
      MC2String key = contents.begin()->first;
      contents_t::const_iterator contIt = 
         contents.lower_bound( key );

      MC2BoundingBox regionBBox;
      
      while ( contIt != contents.end() ) {
         key = contIt->first;
          
         pair<contents_t::const_iterator, contents_t::const_iterator>
            range =  contents.equal_range( key );
         
         // Get map id from index.db using the map ident
         uint32 mapID = m_mapNotices->getMapID( key.c_str() );
         if ( mapID != MAX_UINT32 ) {
               
            // Add map for region.
            uint32 parentMapID = MAX_UINT32;
            vector<uint32> parents;
            m_mapNotices->getMapsAbove( mapID, parents );
            // If parent map exists and current map is underview map, 
            // add it to the idTree as well.
            // This means we won't have overview maps being overviews to
            // other overview maps in the topregion map tree. The reason
            // for this is that we should only search in the first level
            // of overview maps.
            if ( MapBits::isUnderviewMap( mapID ) && (! parents.empty()) ) 
               parentMapID = parents.back();
            
            idTree.addMap( parentMapID, mapID );

            if ( parentMapID != MAX_UINT32 ) {
               idTree.addMap( MAX_UINT32, parentMapID );  
            }
            
            if ( range.first != range.second ) {
               // There may be items for this map.
               
               // Load map.
               OldGenericMap* theMap = 
                  OldGenericMap::createMap( mapID, m_mapPath.c_str() );
               
               for ( contents_t::const_iterator itemIt = range.first;
                     itemIt != range.second; ++itemIt ) {
                  if ( itemIt->second != NULL ) { 
                     const OldItem* item = 
                        theMap->getItemFromItemIdentifier( 
                                                *(itemIt->second) );
                     if ( item != NULL ) {
                        // Add to id tree
                        idTree.addItem( mapID, item->getID() );
                        // Update region bbox
                        if ( item->getGfxData() != NULL ) {
                           // Quick fix...
                           MC2BoundingBox bb;
                           item->getGfxData()->getMC2BoundingBox(bb);
                           regionBBox.update( bb );
                        }
                     } else {
                        mc2log << warn << "addRegions: "
                               << "Could not find item " 
                               << itemIt->second->getName() 
                               << " from the map." 
                               << endl;
                     }
                  } else {
                     // The region consists of all of the map.
                     // Add all submaps to the item id tree.
                     m_mapNotices->getMapTree().getContents( mapID, idTree ); 
                     
                     // Update region bbox.
                     if ( theMap->getGfxData() != NULL ) {
                        MC2BoundingBox bb;
                        theMap->getGfxData()->getMC2BoundingBox(bb);
                        regionBBox.update( bb );
                     }
                  }
               }
               delete theMap;
            }

         } else { 
            // else map couldn't be found. Don't add to region.
            mc2log << warn << "addRegions: Could not find the map " 
                   << key << endl;
            
         }
         
         // Move on to next map.
         contIt = contents.upper_bound( key );
      }

      // Add the region.
      if ( idTree.empty() ){
         // This is OK if only generating a sub set of the maps
         // in the ar.xml file.

         mc2log << warn << "addRegions: OldMap: " << regionIdent 
                << " in ar.xml file does not exist in index.db"
                << endl;
      }


      // Adding the region even if the idTree is empty because 
      // otherwise we must also make sure not to call 
      // regionTree.addRegion above.
      MapTopRegion* region = new MapTopRegion( regIDIt->second,
                                               regionType,
                                               idTree,
                                               regionBBox,
                                               regionNames );
      mc2dbg << "OldMapReader::addRegions Adding region " 
             << regionIdent << " to the map notices." << endl;
      m_mapNotices->addRegion( region );
      

      // Delete all the ItemIdentifiers in contents.
      for ( contents_t::iterator contIt = contents.begin(); 
            contIt != contents.end(); ++contIt ) {
         delete contIt->second;
      }
 
   }
   
   m_mapNotices->setRegionTree( regionTree );
 
   ConstTopRegionMatchesVector regions;
   m_mapNotices->getTopRegions( regions );
  
   mc2dbg << "OldMapReader::addRegions Nbr top regions = " 
          <<  regions.size() << endl;
} // addRegions


bool
MapIndex::updateCreationTimes()
{
   bool retVal = false;
   
   set<uint32> ids;   
   MapGenUtil::getMapIDsInDir( ids, m_mapPath );

   // Creation times per map
   map<uint32, uint32> creationTimeByMap;

   // Country maps
   set<uint32> countryMaps;
   
   // Store the creation times for all maps found.
   for ( set<uint32>::const_iterator it = ids.begin(); it != ids.end();
         ++it ) {
      OldGenericMapHeader* theMap = 
         new OldGenericMapHeader( *it, m_mapPath.c_str() );
      if ( ! theMap->load() ){
         mc2log << error << "OldMapReader::updateCreationTimes "
                << "Could not load map with ID: 0x" << hex << *it << dec
                << endl;
         exit(1);
      }
      creationTimeByMap[ *it ] = theMap->getCreationTime();
         
      // Handle country maps below.
      if ( MapBits::isCountryMap( *it ) ) {
         countryMaps.insert( *it );
      }

      delete theMap;
   }
   
   mc2log << info << "OldMapReader::updateCreationTimes " 
          << "Updates list of underview maps creation time in coutry maps"
          << endl;
   // Update the country maps list of underview maps creation time.
   for ( set<uint32>::const_iterator it = countryMaps.begin();
         it != countryMaps.end(); ++it ) {
      
      OldCountryOverviewMap* country = static_cast<OldCountryOverviewMap*> 
         (OldGenericMap::createMap( *it, m_mapPath.c_str() ));
      if ( country->updateCreationTimes( creationTimeByMap ) ) {
         retVal = true;
         // Save -> the country gets a new creation time.
         country->save();

         // Now update the creation time of the country map.
         creationTimeByMap[ *it ] = country->getCreationTime();
      }
      
      delete country;                
   }
   

   mc2log << info << "OldMapReader::updateCreationTimes " 
          << "Sets creation time in index.db." 
          << endl;
   // Update creation times in the map notices.
   for ( uint32 i = 0; i < m_mapNotices->getSize(); ++i ) {
      MapModuleNotice* notice = (*m_mapNotices)[ i ];

      map<uint32, uint32>::const_iterator it = 
         creationTimeByMap.find( notice->getMapID() );
      if ( it != creationTimeByMap.end() ) {
         if ( notice->getCreationTime() != it->second ) {
            mc2dbg1 << "[OldMapReader] Creation time for map " 
                    << notice->getMapID() << " stored as " 
                    << notice->getCreationTime() << " but it really is "
                    << it->second << endl;
            retVal = true;
    
            // Update the creation time.
            notice->setCreationTime( it->second );
         }
      }
   }
   
   return retVal;
} // END: updateCreationTimes

void 
MapIndex::setMapSupCoverage( MapModuleNoticeContainer::mapSupCoverage_t& 
                             mapSupCov,
                             MapModuleNoticeContainer::coverageTree_t&
                             coverageTree)

{
   m_mapNotices->setMapSupCoverage(mapSupCov, coverageTree);

} // setMapSupCoverage


void 
MapIndex::setMapSupNames( MapModuleNoticeContainer::mapSupNamesByMapSup_t 
                          mapSupNamesByMapSup ){
   m_mapNotices->setMapSupNames(mapSupNamesByMapSup);

   // Check that all map suppliers stored have a corresponding name now.
   const MapModuleNoticeContainer::mapSupCoverage_t& mapSupCov =
      m_mapNotices->getMapSupCoverageAreas();
   for (uint32 i = 0; i<mapSupCov.size(); i++){
      MapGenEnums::mapSupplier mapSup = mapSupCov[i].second;
      const NameCollection& names = m_mapNotices->getMapSupNames(mapSup);
      if (names.getSize() == 0){
         mc2log << error << "Missing name for map supplier ID: " << mapSup
                << endl;
         MC2_ASSERT(false);
      }
   }
   mc2dbg << "Checked that all map suppliers used for copyright boxes have "
          << "names." << endl;

} // setMapSupNames


void
MapIndex::list()
{
   for (uint32 i=0; i < m_mapNotices->getSize(); i++) {
      const MapModuleNotice* mn = (*m_mapNotices)[i];
      MC2_ASSERT(mn != NULL);
      
      cout << "creationTime  [0x" << hex << setw(8) << setfill('0')
           << mn->getMapID() << dec << " " 
           << mn->getCreationTime() << "] - \""
           << mn->getMapName() << "\" " << endl << "              "
           << mn->getBBox() << endl;
   }
   cout << endl;
   
   ConstTopRegionMatchesVector regions;
   m_mapNotices->getTopRegions(regions);
   cout << "Printing top region names" << endl;
   for (uint32 i=0; i<regions.size(); i++){
      cout << "   TopRegionID: " << regions[i]->getID() << " names:" 
           << endl;
      const NameCollection* names = regions[i]->getNames();
      for (uint32 j=0; j<names->getSize(); j++){
         cout << "      Name[" << j << "]: " << *(names->getName(j))
              << endl;
         
      }
   }
   cout << endl;
   
   
   cout << "Printing map supplier coverage hierarchy." << endl;
   // Put coverage hierarchy in a multimap.
   const MapModuleNoticeContainer::mapSupCoverage_t& mapSupCov = 
      m_mapNotices->getMapSupCoverageAreas();
   cout << "mapSupCovAreas.size():" 
        << mapSupCov.size()
        << endl;
   const MapModuleNoticeContainer::coverageTree_t& mapSupCovTree = 
      m_mapNotices->getMapSupCoverageTree();
   cout << "mapSupCovTree.size(): " << mapSupCovTree.size() << endl;
   MapModuleNoticeContainer::mapSupNamesByMapSup_t allMapSupNames = 
      m_mapNotices->getAllMapSupNames();
   cout << "allMapSupNames.size(): " << allMapSupNames.size() << endl;
   multimap<uint32, pair<uint32, uint32> > sortedTree;
   uint32 lastParent = MAX_UINT32;
   uint32 childIdx = 0;
   for ( uint32 i=0; i<mapSupCovTree.size(); i++){
      uint32 parent = mapSupCovTree[i].first;
      uint32 child = mapSupCovTree[i].second;
      if ( lastParent != parent ){
         childIdx = 0;
      }
      else {
         childIdx++;
      }
      sortedTree.insert(make_pair(parent, make_pair(childIdx, child)));
      lastParent = parent;
   }
   // Recursive call.
   printMapSupNode(mapSupCov,
                   MAX_UINT32, // Top index.
                   sortedTree,
                   0); // First level
   cout << endl;

      
} // list


void
MapIndex::printMapSupNode(
                      const MapModuleNoticeContainer::mapSupCoverage_t& 
                      mapSupCov,
                      uint32 idx,
                      const multimap<uint32, pair<uint32, uint32> >&
                      sortedCovTree, 
                      uint32 level)
{
   MC2String indent;
   for (uint32 i=0; i<level; i++){
      indent += "   ";
   }

   if ( idx != MAX_UINT32 ){
      // Not printing the dummy top node.
      MapGenEnums::mapSupplier mapSup = mapSupCov[idx].second;
      MC2String mapSupIdName = MapGenUtil::getIdStringFromMapSupplier(mapSup);
      const NameCollection& names = m_mapNotices->getMapSupNames(mapSup);

      // Remove last null character.
            stringstream strstr;
      strstr << names;
      MC2String namesStr = strstr.str();
      
      cout << "MapSupCov:" << indent 
           << mapSupIdName << "(" << mapSup << "): "
           << mapSupCov[idx].first << endl;
      cout << "MapSupCov:" << indent << "   " << "Names: " << namesStr << endl;

   }

   // Sort the childs
   map<uint32, uint32> childs;
   multimap<uint32, pair<uint32, uint32> >::const_iterator 
      childIt = sortedCovTree.lower_bound(idx);
   while (childIt != sortedCovTree.upper_bound(idx) ){
      uint32 childOrder = childIt->second.first;
      uint32 child = childIt->second.second;
      childs.insert(make_pair(childOrder, child));
      ++childIt;
   }

   // Print the childs.
   uint32 childLevel = level+1;
   map<uint32, uint32>::const_iterator childIt2 = childs.begin();
   while ( childIt2 != childs.end() ){
      uint32 childIdx = childIt2->second;
      
      printMapSupNode(mapSupCov, childIdx, sortedCovTree, childLevel); 
      ++childIt2;
   }
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapModuleNoticeContainer.h"
#include "MapModuleNotice.h"
#include "Name.h"
#include "DataBuffer.h"
#include "Properties.h"
#include "STLStringUtility.h"
#include "DataBufferCreator.h"
#include "MapGenEnums.h"
#include "MapBits.h"
#include "Utility.h"

#include <memory>

// For file reading. Taken from the MapModuleObjVector.
#ifdef __linux
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
#endif

MapModuleNoticeContainer::~MapModuleNoticeContainer()
{
   deleteAllObjs();
}

// Definition of static variable
const NameCollection
MapModuleNoticeContainer::m_emptyNameCollection; 

void
MapModuleNoticeContainer::addNotice( MapModuleNotice* notice,
                                     bool fakeRegions )
{
   
   // Add the notice to our own vector
   m_allNotices.push_back(notice);
   
   // Add the map to the map of maps
   m_noticeMap.insert(make_pair(notice->getMapID(), notice));

   // Add notice by name
   m_noticesByName.insert(make_pair(notice->getMapName(), notice));

   if ( fakeRegions ) {
      // This is the old method which should only be used when loading
      // an old index.db file.
      
      // Do some tricks with the country maps. XXX
      if ( MapBits::isCountryMap( notice->getMapID() ) ) {
         MapTopRegion* region = 
            new MapTopRegion( notice->getCountryCode(),
                              TopRegionMatch::country );
         MC2BoundingBox bb;
         notice->getGfxData()->getMC2BoundingBox(bb);
         region->setBoundingBox( bb );
         
         // FIXME: This is not all the languages, loop all langs
         const char* name_eng = StringTable::getString( 
            StringTable::getCountryStringCode(
               notice->getCountryCode()),
            StringTable::ENGLISH );
         
         const char* name_ger = StringTable::getString( 
            StringTable::getCountryStringCode(
               notice->getCountryCode()),
            StringTable::GERMAN );
         
         const char* name_swe = StringTable::getString( 
            StringTable::getCountryStringCode(
               notice->getCountryCode()),
            StringTable::SWEDISH );
         
         region->addName( name_eng,
                          LangTypes::english,
                          ItemTypes::officialName );
         
         region->addName( name_ger,
                          LangTypes::german,
                          ItemTypes::officialName );
         
         region->addName( name_swe,
                          LangTypes::swedish,
                          ItemTypes::officialName );
         
         // Add to the map.
         m_regions[ region->getID() ] = region;

         // Print the names
         const NameCollection* nc = region->getNames();
         for( uint32 i = 0; i < nc->getSize(); ++i ) {
            mc2dbg4 << "Name " << i << " = " << *nc->getName(i)
                   << endl;
         }
         // Add region to top of region tree.
         m_regionTree.addRegion( MAX_UINT32, region->getID() );
      }
   }
}


bool
MapModuleNoticeContainer::load( const char* inFilename, uint32 mapSet )
{
   mc2dbg << "[MMNC]: Will try to load \"" << inFilename
          << "\"" << endl;

   if ( inFilename == NULL )
      return false;

   MC2String filename = inFilename;

   MC2String urlPath;

   // determine if we should use fallback or not
   bool useFallback = false;
   if ( STLStringUtility::dirname( filename ).size() < 2 ) {

      // no path specified, construct from MAP_PATH_<num>
      // and use fallback
      filename = STLStringUtility::basename( inFilename );
      useFallback = true;      
   } 

   //
   // Load data from two sources:
   // 1) Load from URL if filename contains http://
   // 2) Load from file
   //

   std::auto_ptr<DataBuffer> db;
   DataBufferCreator dbcreator; // for less typing...

   if ( strstr( filename.c_str(), "http://" ) == filename.c_str() ) {
      // save old map set
      uint32 oldMapSet = Properties::getMapSet();
      Properties::setMapSet( mapSet );      

      db.reset( dbcreator.loadFromURL( filename.c_str() ) );

      // restore map set
      Properties::setMapSet( oldMapSet );

   } else {
      if ( useFallback ) {
         db.reset( DataBufferCreator::loadMapOrIndexDB( filename,
                                                        mapSet ) );
      } else {
         // Use the path
         db.reset( DataBufferCreator::loadFromFile( filename.c_str() ) );
      }
   }

   return db.get() ? load( *db.get(), mapSet ) : false;
}


const MapModuleNotice*
MapModuleNoticeContainer::getMap( uint32 mapID ) const
{
   noticeMap_t::const_iterator it = m_noticeMap.find( mapID );
   if ( it != m_noticeMap.end() ) {
      return it->second;
   } else {
      return NULL;
   }
}

void 
MapModuleNoticeContainer::getTopRegions( 
                              ConstTopRegionMatchesVector& regions,
                              TopRegionMatch::topRegion_t type ) const 
{
   for ( topRegionMap_t::const_iterator it = m_regions.begin();
         it != m_regions.end();
         ++it ) {
      if ( ( type == TopRegionMatch::nbr_topregion_t ) ||
           ( it->second->getType() == type ) ) {
         regions.push_back( it->second );
      }
   }
}

bool
MapModuleNoticeContainer::
getMapsAbove( uint32 mapID, vector<uint32>& res ) const
{   
   return m_mapTree.getOverviewMapsFor(mapID, res);
}

void
MapModuleNoticeContainer::
initUsingCC( const vector<MapModuleNotice*>& oldVector )
{
   // This method should only be used for the old index.db.
   // Go through all the notices
   for ( uint32 i = 0; i < oldVector.size(); i++ ) {      
      MapModuleNotice* notice = oldVector[i];
      addNotice( notice, true ); // True means that regions should be faked.
   }

   // Build the ItemIDTrees
   // We know that the top-regions only contain countries.
   // Save all the overview maps in a map sorted by cc.
   // WARNING: This will not work when there is more than
   //          one overview map per region,.
   map<StringTable::countryCode, uint32> overviewMaps;
   typedef  multimap<StringTable::countryCode, uint32> underviewMapMap_t;
   underviewMapMap_t underviewMaps;
   for( uint32 i = 0; i < oldVector.size(); ++i ) {
      MapModuleNotice* notice = m_allNotices[i];
      if ( MapBits::isCountryMap(notice->getMapID()) ) {
         overviewMaps[m_allNotices[i]->getCountryCode()] =
            MapBits::countryToOverview(notice->getMapID());
      } else if ( MapBits::isUnderviewMap( notice->getMapID() ) ) {
         underviewMaps.insert(make_pair(m_allNotices[i]->getCountryCode(),
                                        notice->getMapID()));
      }
   }

   // Loop over the found overview maps.
   for( map<StringTable::countryCode, uint32>::iterator
           it(overviewMaps.begin());
        it != overviewMaps.end();
        ++it ) {
      // Add the top level.
      ItemIDTree* idTree = findRegion( it->first )->getItemIDTreeForWriting();
      idTree->addMap( MAX_UINT32, it->second );
      // Add to or internal tree of all maps too.
      m_mapTree.addMap( MAX_UINT32, it->second );
      // Get the range of underview maps with the same county code
      pair<underviewMapMap_t::const_iterator,
           underviewMapMap_t::const_iterator> range =
         underviewMaps.equal_range( it->first );
      for ( underviewMapMap_t::const_iterator ut(range.first);
            ut != range.second;
            ++ut ) {
         // Add it to the tree of the region
         idTree->addMap( it->second, ut->second);
         // Also add it to our tree of maps.
         m_mapTree.addMap( it->second, ut->second );
      }
   }

#ifdef DEBUG_LEVEL_4
   for(uint32 i = 0; i < m_allNotices.size(); ++i ) {
      mc2dbg << "Overview maps for " << m_allNotices[i]->getMapID()
             << endl;
      vector<uint32> overviewIDs;
      m_mapTree.getOverviewMapsFor(m_allNotices[i]->getMapID(),
                                   overviewIDs);
      for(uint32 j = 0; j < overviewIDs.size(); ++j ) {
         mc2dbg << "    " << j << "=" << overviewIDs[j] << endl;
      }
   }
#endif
#ifdef DEBUG_LEVEL_4
   // Print stuff
   mc2dbg << "MAP_HIERARCHY" << endl;
   set<uint32> mapIDs;
   m_mapTree.getTopLevelMapIDs(mapIDs);
   for(set<uint32>::const_iterator it = mapIDs.begin();
       it != mapIDs.end();
       ++it) {
      mc2dbg << "TopLevelMap :" << hex << *it << dec << " contains " 
             << endl;
      set<IDPair_t> items;
      m_mapTree.getContents(*it, items);
      for( set<IDPair_t>::iterator it = items.begin();
           it != items.end();
           ++it ) {
         mc2dbg << "      " << it->getMapID() << ":" << hex
                << it->getItemID() << dec << endl;
      }
   }
#endif
}

void
MapModuleNoticeContainer::deleteAllObjs()
{
   // Delete the mapNotices
   for( noticeVect_t::iterator it( m_allNotices.begin()) ;
        it != m_allNotices.end();
        ++it ) {
      delete *it;
   }
   // Empty the vector and map
   m_allNotices.clear();
   m_noticeMap.clear();
   m_noticesByName.clear();
   
   // Delete the contents of the TopRegionVector
   for ( topRegionMap_t::iterator it ( m_regions.begin() );
         it != m_regions.end();
         ++it ) {
      delete it->second;
   }
   // Clear the map
   m_regions.clear();

}

uint32
MapModuleNoticeContainer::getMapID(const char* mapName) const
{
   noticesByName_t::const_iterator it = m_noticesByName.find(mapName);
   if ( it != m_noticesByName.end() ) {
      return it->second->getMapID();     
   } else {
      return MAX_UINT32;
   }
}


// ------ WriteableMapModuleNoticeContainer +
// ------ MapModuleNoticeContainer::newLoad

void
WriteableMapModuleNoticeContainer::addLast(MapModuleNotice* notice)
{
   addNotice( notice );
}
   

void 
WriteableMapModuleNoticeContainer::addRegion( MapTopRegion* region )
{
   m_regions[ region->getID() ] = region;
}


void 
WriteableMapModuleNoticeContainer::setMapTree( const ItemIDTree& tree )
{
   m_mapTree = tree; // assignment operator works for tree.
}


void 
WriteableMapModuleNoticeContainer::setRegionTree( const RegionIDTree& tree )
{
   m_regionTree = tree; // assignment operator works for tree.
}


void 
WriteableMapModuleNoticeContainer::clearRegions()
{
   // Delete the contents of the TopRegionVector
   for ( topRegionMap_t::iterator it ( m_regions.begin() );
         it != m_regions.end();
         ++it ) {
      delete it->second;
   }
   // Clear the map
   m_regions.clear();
   m_regionTree = RegionIDTree();
}

bool
MapModuleNoticeContainer::oldLoad( int fd,
                                   uint32 totalLength,
                                   uint32 nbrMaps)
{
   mc2dbg << "[MMNC]: Using old type of index.db!" << endl;
   
   

   vector<MapModuleNotice*> notices;

   {
      errno = 0;
      DataBuffer dataBuffer(totalLength);
      if (! Utility::read( fd,
                           dataBuffer.getBufferAddress(), 
                           totalLength) ) {     
         mc2log << error << "[MMNC]::oldLoad - error reading index : "
                << strerror(errno) << ", totalLength = " << totalLength
                << endl;
         return false;
      } else {
         for (uint32 i=0; i<nbrMaps; i++) {
            MapModuleNotice *tmpMN = new MapModuleNotice(&dataBuffer);
            mc2dbg4 << "MapNotice for map with ID=" 
                    << tmpMN->getMapID() << ", country=" 
                    << StringTable::getString(
                       StringTable::getCountryStringCode(
                          tmpMN->getCountryCode()),
                       StringTable::SWEDISH) << endl;
            mc2dbg4 << "MapModuleNotice with mapID = "  
                    << tmpMN->getMapID() << " loaded." << endl;
            notices.push_back(tmpMN);
#if 0
            if ( MapBits::isCountryMap(tmpMN->getMapID()) ) {
               notices.push_back(new MapModuleNotice(
                  MapBits::countryToOverview(tmpMN->getMapID())));
            }
#endif
         }
      }
   }

   // Remove contents of container.
   deleteAllObjs();
   // Fake regions.
   initUsingCC( notices );
   return true;
   
}

bool
MapModuleNoticeContainer::load( DataBuffer& dataBuf, uint32 mapSet )
{
   dataBuf.readNextLong(); // old totalLength
   dataBuf.readNextLong(); // nbrMaps

   mc2dbg << "[MMNC]: Using new kind of index.db" << endl;
   // Format on disk begins with two 32-bit words with zeroes.
   // They should already have been read by load.
   // We should really implement version 0 here as well so that
   // we can remove the MapModuleObjVector.
   // Then comes length and version

   uint32 totalLength = dataBuf.readNextLong(); // length
   uint32 version  = dataBuf.readNextLong();
   uint32 nbrItems = dataBuf.readNextLong();

   if (dataBuf.getNbrBytesLeft() == 0) {
      // we must have some bytes left
      // TODO: check if nbr bytes left is at least 
      // = length * something...
      return false;
   }

   // Reset the internal structs.
   deleteAllObjs();
   
   // Version should only be 1 now.
   for ( uint32 i = 0; i < nbrItems; ++i ) {
      MapModuleNotice* notice = 
         new MapModuleNotice( &dataBuf, version, mapSet );
      // Add the notice to the appropriate vectors and maps.
      addNotice( notice );
      mc2dbg << "[MMNC]: Added MapModuleNotice for "
             << notice->getMapName() << endl;
   }

   // Read the map hierarchy
   m_mapTree.load( &dataBuf, mapSet );

   // Read the regions
   int nbrRegions = dataBuf.readNextLong();
   m_regions.clear();

   mc2dbg << "[MMNC]: Number of top regions:" << nbrRegions << endl;

   for ( int i = 0; i < nbrRegions; ++i ) {

      MapTopRegion* region = new MapTopRegion();
      region->load( &dataBuf, mapSet );

      m_regions[ region->getID() ] = region;
      const char* name = region->getName( LangTypes::english );

      if ( name == NULL ) {
         name = "NULL";
      }

      mc2dbg << "[MMNC]: Added Top Region ID:" << region->getID()
             << " name: " << name << endl;
   }
   
   // Read the region hierarchy
   m_regionTree.load( &dataBuf );

   // Read map supplier coverage data.
   mc2dbg8 << "Curr offset: " << dataBuf.getCurrentOffset()
           << " total length " << totalLength << endl;
   if (dataBuf.getCurrentOffset() < totalLength+12 ){
      mc2dbg << "[MMNC]: Loading map supplier coverage." << endl;
      loadMapSupCoverage( &dataBuf );
      mc2dbg << "[MMNC]: Done loading map supplier coverage." << endl;
   }
   else {
      mc2dbg << "[MMNC]: Not loading map supplier coverage." << endl;
   }

#ifdef DEBUG_LEVEL_1
   //
   // debug
   //
   for(uint32 i = 0; i < m_allNotices.size(); ++i ) {
      mc2dbg << "Overview maps for " << m_allNotices[i]->getMapID()
             << endl;
      vector<uint32> overviewIDs;
      m_mapTree.getOverviewMapsFor(m_allNotices[i]->getMapID(),
                                   overviewIDs);
      for(uint32 j = 0; j < overviewIDs.size(); ++j ) {
         mc2dbg << "    " << j << "=" << overviewIDs[j] << endl;
      }
   }

   {
      // Print stuff
      mc2dbg << "MAP_HIERARCHY" << endl;
      set<uint32> mapIDs;
      m_mapTree.getTopLevelMapIDs(mapIDs);
      for(set<uint32>::const_iterator it = mapIDs.begin();
          it != mapIDs.end();
          ++it) {
         mc2dbg << "TopLevelMap :" << hex << *it << dec << " contains " 
                << endl;
         set<IDPair_t> items;
         m_mapTree.getContents(*it, items);
         for( set<IDPair_t>::iterator it = items.begin();
              it != items.end();
              ++it ) {
            mc2dbg << "      " << it->getMapID() << ":" << hex
                   << it->getItemID() << dec << endl;
         }
      }
   }

   // Print stuff
   mc2dbg << "REGION_HIERARCHY" << endl;
   set<uint32> mapIDs;
   m_regionTree.getTopLevelRegionIDs(mapIDs);
   for(set<uint32>::const_iterator it = mapIDs.begin();
       it != mapIDs.end();
       ++it) {
      mc2dbg << "TopLevelRegion : " 
             << getRegion( *it )->getNames()
                  ->getBestName( LangTypes::swedish )->getName()
             << " " << *it << endl;
      
      const ItemIDTree& idTree = getRegion( *it )->getItemIDTree();
      set<uint32> mapIDs;
      mc2dbg << " consists of the following items:" << endl;
      idTree.getTopLevelMapIDs( mapIDs );
      for ( set<uint32>::const_iterator jt = mapIDs.begin();
            jt != mapIDs.end(); ++jt ) {
         mc2dbg << " map id = " << *jt << endl;
         set<IDPair_t> items;
         idTree.getContents( *jt, items );
         for ( set<IDPair_t>::const_iterator kt = items.begin();
               kt != items.end(); ++kt ) {
            mc2dbg << "   [" << kt->first << ", " << kt->second << "]"
                   << endl;
         }
      }
      
      
      mc2dbg << " contains the following other regions: " << endl;
      set<uint32> items;
      m_regionTree.getContents(*it, items);
      for( set<uint32>::iterator jt = items.begin();
           jt != items.end();
           ++jt ) {
         mc2dbg << "      " << *jt << endl;
      }
   }

   //
   // end debug
   //   
#endif // DEBUG_LEVEL_1

   return true;
}


bool
WriteableMapModuleNoticeContainer::save( const char* filename )
{
   mc2dbg << "[WMMNC]: Saving..." << endl;

   int version = 1;
   // Assumes that the average MapModuleNotice is less than
   // 256 kB
   // TODO: Calculate the size
   uint32 noticesSize=0;
   for (uint32 i = 0; i < getSize(); i++){
      noticesSize+=getElementAt( i )->getSizeInDataBuffer(version);  
   }
   
   DataBuffer dataBuffer(noticesSize +
                         m_mapTree.getSizeInDataBuffer()+2000000 +
                         mapSupCovSizeInDataBuffer() );
   dataBuffer.fillWithZeros();
   //   DataBuffer dataBuffer((getSize() + 1) * 262144*8 +
   //                         m_mapTree.getSizeInDataBuffer()+2000000 );
   
   dataBuffer.writeNextLong(0);         // Old size
   dataBuffer.writeNextLong(0);         // Old nbritems
   dataBuffer.writeNextLong(0);         // New length
   dataBuffer.writeNextLong(version);   // Version
   dataBuffer.writeNextLong(getSize()); // Nbr mapModulenotices
   for( uint32 i = 0; i < getSize(); ++i ) {
      mc2dbg << "[WMMNC]: Saving notice"
             << m_allNotices[i]->getMapID() << endl;
      m_allNotices[i]->save(&dataBuffer, version);
   }

   // Save the ItemIDTree
   mc2dbg << "[WMMNC]: Saving mapTree" << endl;
   m_mapTree.save(&dataBuffer);

   // Save the regions
   mc2dbg << "[WMMNC]: Saving regions" << endl;
   int nbrRegions = m_regions.size();
   dataBuffer.writeNextLong(nbrRegions);
   for( topRegionMap_t::const_iterator it = m_regions.begin();
        it != m_regions.end();
        ++it ) {
      it->second->save(&dataBuffer);
   }
   
   // Save the RegionIDTree
   mc2dbg << "[WMMNC]: Saving regionTree" << endl;
   m_regionTree.save(&dataBuffer);
   
   // Save map supplier coverage data
   mc2dbg << "[WMMNC]: Saving map supplier coverage data" << endl;
   saveMapSupCoverage(&dataBuffer);
   mc2dbg << "[WMMNC]: Done saving map supplier coverage data" << endl;

   // TotalSize, but not including old size,nbr items and new length
   dataBuffer.writeLong(dataBuffer.getCurrentOffset()-12, 8);
   int fd;
   errno = 0;
#  ifdef __linux
   fd = creat(filename, S_IRUSR | S_IWUSR | // owner rw
                        S_IRGRP | S_IWGRP | // group rw
                        S_IROTH );          // other r
#  else   
   fd = creat(filename, 664);
#  endif
   
   if (fd <= 0) { 
      mc2log << error << "MapModuleNoticeContainer::Save(), File error"
             << strerror(errno) << endl;
      return false;
   } else {
      // Fill in TotalSize
      write(fd, 
            dataBuffer.getBufferAddress(),
            dataBuffer.getCurrentOffset() );
   }
   ::close(fd);
   mc2dbg << "[WMMNC]: Everything saved" << endl;
   return true;
}


uint32
WriteableMapModuleNoticeContainer::mapSupCovSizeInDataBuffer(){
   uint32 sizeInDataBuffer = 0;
   // Number of elements
   sizeInDataBuffer += 4;

   mapSupCoverage_t::const_iterator supBoxIt = m_mapSupCoverage.begin();
   while (supBoxIt != m_mapSupCoverage.end()){
      // Bounding box
      sizeInDataBuffer += 4 * 4;

      // Supplier name.
      sizeInDataBuffer += 4;
      
      ++supBoxIt;
   }


   // Number of elements
   sizeInDataBuffer += 4;
   // Parent and child index.
   sizeInDataBuffer += (4 + 4) * m_mapSupCoverageTree.size();

   return sizeInDataBuffer;
} // WriteableMapModuleNoticeContainer::mapSupCovSizeInDataBuffer

void 
WriteableMapModuleNoticeContainer::saveMapSupCoverage( DataBuffer* db ){
   MC2_ASSERT(db != NULL);


   // Save coverage areas.

   // Write number of elements
   db->writeNextLong(m_mapSupCoverage.size() );

   mapSupCoverage_t::const_iterator supBoxIt = m_mapSupCoverage.begin();
   while (supBoxIt != m_mapSupCoverage.end()){
      // Write bounding box
      const MC2BoundingBox& bBox = supBoxIt->first;
      db->writeNextLong(bBox.getMaxLat());
      db->writeNextLong(bBox.getMinLat());
      db->writeNextLong(bBox.getMaxLon());
      db->writeNextLong(bBox.getMinLon());

      // Write map supplier.
      db->writeNextLong(supBoxIt->second);
      
      ++supBoxIt;
   }
   mc2dbg << "[WMMNC]: Saved " << m_mapSupCoverage.size() 
          << " map supplier coverage areas." << endl;


   // Save coverage areas tree.

   // Number of elements
   db->writeNextLong(m_mapSupCoverageTree.size());
   
   coverageTree_t::const_iterator covIt = m_mapSupCoverageTree.begin();
   while (covIt != m_mapSupCoverageTree.end()){
      // Write parent and child.
      db->writeNextLong(covIt->first);
      db->writeNextLong(covIt->second);
      
      ++covIt;
   }
   mc2dbg << "[WMMNC]: Saved " << m_mapSupCoverageTree.size() 
          << " notices of covearage tree." 
          << endl;

   // Map supplier names.
   db->writeNextLong(m_mapSupNamesByMapSup.size() );
   mapSupNamesByMapSup_t::const_iterator supIt = 
      m_mapSupNamesByMapSup.begin();
   while ( supIt != m_mapSupNamesByMapSup.end() ){
      db->writeNextLong(supIt->first);
      supIt->second.save(db);
      ++supIt;
   }
   mc2dbg << "[WMMNC]: Saved " << m_mapSupNamesByMapSup.size() 
          << " map supplier name collections." 
          << endl;


} // WriteableMapModuleNoticeContainer::saveMapSupCoverage

void 
MapModuleNoticeContainer::loadMapSupCoverage( DataBuffer* db ){
   MC2_ASSERT(db != NULL);

   // Read number of elements
   uint32 nbrElements = db->readNextLong();
   
   for (uint32 i=0; i<nbrElements; i++){

      // Read bounding box
      uint32 maxLat = db->readNextLong();
      uint32 minLat = db->readNextLong();
      uint32 maxLon = db->readNextLong();
      uint32 minLon = db->readNextLong();
      MC2BoundingBox bBox;
      bBox.setMaxLat(maxLat);
      bBox.setMinLat(minLat);
      bBox.setMaxLon(maxLon);
      bBox.setMinLon(minLon);

      // Read supplier name.
      MapGenEnums::mapSupplier mapSupplier = 
         static_cast<MapGenEnums::mapSupplier>(db->readNextLong());
      m_mapSupCoverage.push_back(make_pair(bBox, mapSupplier));
   }
   mc2dbg << "[MMNC]: Loaded " << m_mapSupCoverage.size() 
          << " map supplier coverage bounding boxes." << endl;


   // Read coverage tree.
   nbrElements = db->readNextLong();

   m_mapSupCoverageTree.reserve(nbrElements);   
   for (uint32 i=0; i<nbrElements; i++){   
      uint32 parent =  db->readNextLong();
      uint32 child =  db->readNextLong();

      m_mapSupCoverageTree.push_back(make_pair(parent, child));
   }
   MC2_ASSERT(m_mapSupCoverageTree.size() == nbrElements);

   // Load map supplier names.
   uint32 nbrToRead = db->readNextLong();
   for (uint32 i=0; i<nbrToRead; i++){
      NameCollection tmpNames;
      MapGenEnums::mapSupplier mapSup = 
         static_cast<MapGenEnums::mapSupplier>(db->readNextLong());
      tmpNames.load(db);
      m_mapSupNamesByMapSup.insert(make_pair(mapSup, tmpNames));
   }
   mc2dbg << "[MMNC]: Loaded " << m_mapSupNamesByMapSup.size() 
          << " map supplier names." << endl;

} // MapModuleNoticeContainer::loadMapSupCoverage



void 
WriteableMapModuleNoticeContainer::setMapSupCoverage( mapSupCoverage_t& 
                                                      mapSupCov,
                                                      coverageTree_t& 
                                                      coverageTree )
{
   m_mapSupCoverage = mapSupCov;
   m_mapSupCoverageTree = coverageTree;
   
   // Check sanity.
   if ( m_mapSupCoverageTree.size() != m_mapSupCoverage.size() ){
      mc2log << error 
             << "m_mapSupCoverageTree.size() != m_mapSupCoverage.size()" 
             << endl;
      MC2_ASSERT(false);
   }
   // Make sure all areas are present in the hierarchy.
   for (uint32 i=0; i<m_mapSupCoverage.size(); i++){
      bool found = false;
      uint32 j = 0;
      while (!found && (j<m_mapSupCoverageTree.size()) ){
         found = (m_mapSupCoverageTree[j].second == i);
         j++;
      }
      if (!found){
         mc2log << error
                << "Coverage area " << m_mapSupCoverage[i].first << " : " 
                << m_mapSupCoverage[i].second << " missing in hierarchy." 
                << endl;
         MC2_ASSERT(false);
      }
   }

} // WriteableMapModuleNoticeContainer::setMapSupCoverage


const MapModuleNoticeContainer::mapSupCoverage_t& 
MapModuleNoticeContainer::getMapSupCoverageAreas() const {

   return m_mapSupCoverage;
} // MapModuleNoticeContainer::getAllMapSupCoverage

const MapModuleNoticeContainer::coverageTree_t&
MapModuleNoticeContainer::getMapSupCoverageTree() const {

   return m_mapSupCoverageTree;
} // MapModuleNoticeContainer::getMapSupCoverageTree


const MapModuleNoticeContainer::copyrightByMapSup_t& 
MapModuleNoticeContainer::getCopyrightByMapSup(
     LangTypes::language_t lang /*= LangTypes::invalidLanguage*/ ) const
{
   if ( m_copyrightByMapSup.size() == 0 ){
      // First call, init m_copyrightByMapSup.
      for ( uint32 i=0; i<m_mapSupCoverage.size(); i++){
         MapGenEnums::mapSupplier mapSup = m_mapSupCoverage[i].second;
         const NameCollection& names = getMapSupNames(mapSup);
         const MC2String mapSupStr = names.getBestName(lang)->getName();
         if ( mapSupStr != "" ){
            m_copyrightByMapSup.insert( make_pair( mapSup, mapSupStr ) );
         }
         else {
            mc2log << error << "Missing copyright name of map supplier "
                   << "code: " << mapSup << endl;
            // Shoud I MC2_ASSERT(false)?
         }
      }
   }
   return m_copyrightByMapSup;

} // MapModuleNoticeContainer::getCopyrightByMapSup


const NameCollection& 
MapModuleNoticeContainer::getMapSupNames(MapGenEnums::mapSupplier 
                                         mapSupplier) const
{
   mapSupNamesByMapSup_t::const_iterator it = 
      m_mapSupNamesByMapSup.find(mapSupplier);
   if ( it != m_mapSupNamesByMapSup.end() ){
      return it->second;
   }
   else {
      return m_emptyNameCollection;
   }
}


MapModuleNoticeContainer::mapSupNamesByMapSup_t
MapModuleNoticeContainer::getAllMapSupNames(){
   return m_mapSupNamesByMapSup;

} // getAllMapSupNames

void 
WriteableMapModuleNoticeContainer::setMapSupNames(
       mapSupNamesByMapSup_t& 
       mapSupNamesByMapSup)
{
   m_mapSupNamesByMapSup = mapSupNamesByMapSup;
}

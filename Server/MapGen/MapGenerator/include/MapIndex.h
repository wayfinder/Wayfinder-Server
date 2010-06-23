/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPINDEX_H
#define MAPINDEX_H

#include "config.h"
#include "MapModuleNoticeContainer.h"
#include "XMLParserHelper.h"
#include "MapGenEnums.h"

class StringTable;


/**
 * Handles creation and modification of the index.db file.
 *
 */
class MapIndex {
 public:

   /**
    * Constructor. Existing index will be overwritten if not loaded before 
    * saving.
    *
    * @param mapPath Path to where to find index file and map files to use
    */ 
   MapIndex(const char* mapPath);

   /**
    * Destructor
    */
   ~MapIndex();

   /**
    * Load index.
    */
   void load();

   /**
    * Save index.
    */
   void save();
   
   /**
    * Print index content to standard out
    */
   void list();

   /**
    * Creates a new index.db from existing maps.
    */
   void createFromMaps( const map<MC2String, MC2String>& 
                        overviewByUnderview,
                        const map<MC2String, MC2String>& 
                        superByOverview );

   /**
    * Adds regions from XML data.
    */
   void addRegions( XMLParserHelper::xmlByTag_t::const_iterator& begin,
                    XMLParserHelper::xmlByTag_t::const_iterator& end,
                    const map<MC2String, uint32>& regionIDsByIdent );

   /**
    *    Updates the creation times stored index.db and the
    *    country maps.
    *    @param   onlyPerformCheck  Optional parameter that if 
    *                               specified indicates that no 
    *                               actual update will be performed.
    *
    *    @return  True if any creation times were updated.
    */
   bool updateCreationTimes();

   /**
    *    Set the map supplier coverage data.
    *
    *    @mapSupCov Complete map supplier coverage data. Any present data
    *               is replaced.
    *    @coverageTree Tells what coverage areas include other coverage
    *                  areas. Parent mapped to child, a child is smaller
    *                  than a parent.
    */
   void setMapSupCoverage( MapModuleNoticeContainer::mapSupCoverage_t& 
                           mapSupCov,
                           MapModuleNoticeContainer::coverageTree_t&
                           coverageTree);

   /**
    *    Set the map supplier name info.
    *
    *    @param mapSupNamesByMapSup An STL map with map supplier as key
    *           and its names in different languages as value.
    */
   void setMapSupNames( MapModuleNoticeContainer::mapSupNamesByMapSup_t 
                        mapSupNamesByMapSup );

 private:

   /**
    * Adds map notices for underview and overview maps, or overview
    * and super overview maps.
    *
    * @param higherMapByLowerMap
    *        A STL::map with map IDs as strings. In case of underview
    *        and overview maps, the overview maps are considered 
    *        higher, in case of super overview and overview maps,
    *        super overview maps are considered higher.
    */
   void addMapNotices( map<MC2String, uint32> mapIDByName,
                       const map<MC2String, MC2String>&
                       higherMapByLowerMap );
   /**
    * Adds map notices for country overview masps.
    *
    * @param mapNotices
    *        The map notices of the index.db file to add notices to.
    */
   void addCountryMapNotices();


   /** 
    * Print the content of a map supplier node to standard out. Intended for 
    * recursive calls.
    *
    * @param mapSupCov      All map suppler boxes.
    * @param idx            Index in mapSupCov of the box to print.
    * @param mapSupCovTree  The tree of supplier boxes.
    * @param level          The current level in the tree.
    */
   void printMapSupNode(const MapModuleNoticeContainer::mapSupCoverage_t& 
                        mapSupCov,
                        uint32 idx,
                        const multimap<uint32, pair<uint32, uint32> > &
                        mapSupCovTree, 
                        uint32 level);



   /////////////////////////////////////////////////////////////////////
   // Member variables

   /// The map notice container corresponding to the index.db edited.
   WriteableMapModuleNoticeContainer* m_mapNotices;

   /// The path   to where to find index file and map files to use.
   MC2String m_mapPath;

   typedef multimap<StringTable::countryCode, MapGenEnums::mapSupplier> 
      mapSupsByCountry_t;

   /**
    * Includes map supplier for all countries included in the index. 
    * Only filled in when adding regions to the index.
    * 
    * Used by map supplier coverage bounding box check.
    */
   mapSupsByCountry_t m_mapSupsByCountry;

}; // class MapIndex

#endif // MAPINDEX_H

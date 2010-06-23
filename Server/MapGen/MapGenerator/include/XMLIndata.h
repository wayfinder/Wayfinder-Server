/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLINDATA_H
#define XMLINDATA_H

#include "MC2String.h"
#include <map>
#include <set>
#include "NameCollection.h"
#include "TopRegionMatch.h"

#include "MapIndex.h"

#include "XMLUtility.h"
#include "XMLParserHelper.h"

class ItemIdentifier;

/**
 * Handles processing of XML data controlling generation process in 
 * GenerateMapServer.
 *
 */
class XMLIndata {
 public:
   /**
    * Make sure to initiate the XML system (XMLPlatformUtils::Initialize)
    * before calling this method.
    *
    * @return Map with higher level map names by lower level map names. 
    *
    *         When parsing *_co.xml files, lower level are underview maps
    *         and higher are overview maps. When parsing *_coo.xml files,
    *         lower level are overview maps and higher level are super
    *         overview maps.
    */
   static map<MC2String, MC2String> 
      parseCreateOverviewXMLData ( const set<const char*>& xmlFiles );

   /**
    * Make sure to initiate the XML system (XMLPlatformUtils::Initialize)
    * before calling this method.
    *
    * Parse regions from XML files and adds them to mapIndex.
    *
    * @param xmlFiles The XML files where the regions to add can be found,
    *                 and the XML file containing the region IDs.
    */
   static void
      parseAndAddRegionsToIndex( MapIndex& mapIndex, 
                                 const set<const char*>& xmlFiles );

   static void
      parseAndAddMapSupCoverageToIndex( MapIndex& mapIndex, 
                                        const char* xmlFileName );
   static void
      parseAndAddMapSupNamesToIndex( MapIndex& mapIndex, 
                                     const char* mapSupNameXmlFile );

      /**
       *    Parse create_overview xml node. Used by overview map creation and
       *    XMLIndata::parseCreateOverviewXMLData
       *    
       *    @param   createOverview       The create_overview node.
       *    @param   overviewByUnderview  Out parameter.
       *                                  Map containing overview map 
       *                                  idents as value and
       *                                  underview map idents as key.
       *    @return  True if the parsing went well, otherwise false.
       */
      static bool parseCreateOverview( 
                              const DOMNode* createOverview, 
                              map<MC2String, MC2String>& overviewByUnderview );

      /**
       * 
       *    Parse add_region xml node.
       *    Note that the ItemIdentifiers in contents must be deleted
       *    by the caller of this method.
       *
       *    @param   addRegion   The add_region node.
       *    @param   regionNames Out parameter. The names of the region.
       *    @param   regionType  Out parameter. The region type.
       *    @param   regionIdent Out parameter. The region ident.
       *    @param   contents    Out parameter. Multimap containing
       *                         the contents of the region. The map id
       *                         is key and an item identifier is value.
       *                         If the item identifier is NULL, then that
       *                         means that all of the map is inside the
       *                         region. If not NULL, only the 
       *                         corresponding item is inside the region.
       *                         Note that the ItemIdentifiers are 
       *                         allocated here and must be
       *                         deleted by the caller of this method.
       *    @param   insideRegions  Out parameter. Which regions that this
       *                            region is part of.
       *    @return  True if the parsing went well, otherwise false.
       */
      static bool parseAddRegion( 
                              const DOMNode* addRegion,
                              NameCollection& regionNames,
                              TopRegionMatch::topRegion_t& regionType,
                              MC2String& regionIdent,
                              multimap<MC2String, ItemIdentifier*>& contents,
                              set<MC2String>& insideRegions );

       

private:
      
      /**
       *    Parse region_ids xml node.
       *
       *    @param   regionIDs         The region_ids node.
       *    @param   regionIDsByIdent  Out parameter.
       *                               Map containing regionIDs as value
       *                               and  region idents as key.
       *    @return  True if the parsing went well, otherwise false.
       */
      static bool parseRegionIDs( const DOMNode* regionIDs, 
                                  map<MC2String, uint32>& regionIDsByIdent );


      /**
       *    Parse content xml node.
       *
       *    @param   content     The content node.
       *    @param   contents    Out parameter. Multimap containing
       *                         the contents of the region. The map id
       *                         is key and an item identifier is value.
       *                         If the item identifier is NULL, then that
       *                         means that all of the map is inside the
       *                         region. If not NULL, only the 
       *                         corresponding item is inside the region.
       *                         Note that the ItemIdentifiers are 
       *                         allocated here and must be
       *                         deleted by the caller of this method.
       *    @return  True if the parsing went well, otherwise false.
       */
      static bool parseContent( const DOMNode* content,
                           multimap<MC2String, ItemIdentifier*>& contents );
     
      
      /**
       *    Parse name xml node.
       *    @param   name  The name node.
       *    @param   names Out parameter. The names.
       *    @return  True if the parsing went well, otherwise false.
       */
      static bool parseName( const DOMNode* name, NameCollection& names );
      
      
      /**
       *    Parse item_ident xml node.
       *    
       *    @param   node        The item_ident node.
       *    @param   itemIdent   Out parameter. The item ident (describes
       *                         an item without specifying item id).
       *    @return  True if the parsing went well, otherwise false.
       */
      static bool parseItemIdent( const DOMNode* node, 
                                  ItemIdentifier& itemIdent );



   /**
    * Loads map generation XML files, allocates an XMLParserHelper
    * with new and inits the XMLParserHelper with the XML files.
    * Use deleteXMLParserHelper to delete a parser created from this
    * one.
    *
    * @param xmlFiles The xml files to parse. NB not all kinds of
    *                 XML files are supported.
    *
    * @return A XMLParserHelper that must be deleted after usage.
    */
   static XMLParserHelper* 
      createXMLParserHelper(const set<const char*>& xmlFiles);


   /**
    * @name For parsing copyright coverage.
    */
   //@{

   /**
    * Each element contain the map supplier and the countires one of its
    * coverage bounding boxes covers.
    */
   typedef 
      vector <pair<MapGenEnums::mapSupplier,
      set<StringTable::countryCode> > > mapSupAndCountries_t;


   /**
    * Parse a map supplier coverage as defined in map_generation-mc2.dtd
    * from XML.
    *
    * @param mapSupCovNode Top node of the map_suppler_coverage node to 
    *                      parse.
    * @param parentSupIdx  Give the index of the parent map supplier 
    *                      element in the mapSupCov vector.
    * @param parentHierarchyLevel The level of nested map supplier 
    *                      coverage XML element of the parent. Start with
    *                      -1 to make the first parsed end up on 0.
    * @param mapSupCov     Bounding boxes mapped to map supplier.
    * @param supAndCntrListVec Countries covered by each map supplier 
    *                          area.
    * @param covTreeByLevel Vector of vectors with hierarchy of supplier 
    *                      boundig boxes. Index in outer vector determines
    *                      level in the xml hierarchy. Second vector is
    *                       ordered by the order to test the bounding 
    *                      boxes. The pair keeps parent idx in first and
    *                      child idx in second. The idx:es correspond to
    *                      the ones in mapSupCov.
    */
   static void parseMapSupCoverage( const DOMNode* mapSupCovNode,
                             uint32 parentSupIdx,
                             int32 parentHierarchyLevel,
                             MapModuleNoticeContainer::mapSupCoverage_t&
                             mapSupCov,
                             mapSupAndCountries_t& supAndCntrListVec,
                             vector< vector<pair<uint32, uint32> > >&
                             coverageTree );

   /**
    * Parse map suppler name XML element of one map supplier
    * @param mapSupNameElm The DOMNode to parse
    * @param mapSupNames   The names are stored here.
    */
   static void parseMapSupName(DOMNode* mapSupNameNode,
                               MapModuleNoticeContainer::mapSupNamesByMapSup_t&
                               mapSupNames );

   
   /**
    * Parse a bounding box as defined in map_generation-mc2.dtd from XML.
    *
    * @param bBoxTopNode The top element of a bounding box.
    *
    * @return The bounding box defined by the bBoxTopNode element.
    */ 
   static MC2BoundingBox parseBoundingBox(const DOMNode* bBoxTopNode);

   /**
    * Parse a country list as defined in map_generation-mc2.dtd from XML.
    *
    * @param cntrListTopNode The top element of a country list.
    *
    * @return The country list cntrListTopNode element.
    */ 
   static set<StringTable::countryCode>
      parseCountryList(const DOMNode* cntrListTopNode);

   //@}

   
   /// Used together with XML parser helper.
   static const XStr* m_createOverviewXStr;
   /// Used together with XML parser helper.
   static const XStr* m_addRegionXStr;
   /// Used together with XML parser helper.
   static const XStr* m_regionIDsXStr;
   /// Used together with XML parser helper.
   static const XStr* m_mapSupCovXStr;
   /// Used together with XML parser helper.
   static const XStr* m_mapSupNameXStr;

}; // class XMLIndata

#endif // XMLINDATA_H

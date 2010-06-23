/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLIndata.h"

#include "XMLCommonEntities.h"
#include "XMLCommonElements.h"

#include "MapGenUtil.h"
#include "Name.h"
#include "ItemIdentifier.h"




const XStr* 
XMLIndata::m_createOverviewXStr = new XStr( "create_overviewmap" );
const XStr* 
XMLIndata::m_addRegionXStr =  new XStr( "add_region" );
const XStr* 
XMLIndata::m_regionIDsXStr = new XStr( "region_ids" );
const XStr* 
XMLIndata::m_mapSupCovXStr = new XStr( "map_supplier_coverage" );
const XStr* 
XMLIndata::m_mapSupNameXStr = new XStr( "map_supplier_name" );

map<MC2String, MC2String>
XMLIndata::parseCreateOverviewXMLData( const set<const char*>& xmlFiles)
{
   // Map names. Higher are overviews or super overivews and lower are 
   // underviews or overvies
   map<MC2String, MC2String> higherByLower;

   if (  m_createOverviewXStr == NULL ){
      mc2log << error << "XMLIndata::processCreateOverveiwXMLData"
             << "XMLParserHelper incorrectly initiated." << endl;
      exit(1);
   }

   XMLParserHelper* helpParser = createXMLParserHelper(xmlFiles);
   if ( helpParser == NULL ){
      mc2log << error << "XMLIndata::processCreateOverviewXMLData"
             <<" Failed to create XML parser helper." << endl;
      exit(1);
      return higherByLower;
   }

   XMLParserHelper::xmlByTag_t xmlByTag = helpParser->getXMLNodesByTag();

   pair<XMLParserHelper::xmlByTag_t::iterator, 
      XMLParserHelper::xmlByTag_t::iterator> createHigherRange =
      xmlByTag.equal_range( m_createOverviewXStr->XMLStr() );
   
   if ( createHigherRange.first != createHigherRange.second ) {
      // Create overview maps.
      
      // Parse xml
      for ( XMLParserHelper::xmlByTag_t::iterator it = 
               createHigherRange.first;
            it != createHigherRange.second; ++it ) {
         XMLIndata::parseCreateOverview( 
               it->second, higherByLower );
      }
   }
   else {
      mc2log << warn << "MapReader::processCreateOverviewXMLData "
             << "Could not parse XML file as create overview XML."
             << endl;
      if ( xmlFiles.size() > 0 ){
         mc2dbg << "Name of first XML file: \"" << *xmlFiles.begin() 
                << "\"" << endl;
      }
      else {
         mc2dbg << "Size of xmlFiles == 0" << endl;
      }
   }
   delete helpParser;
   
   return higherByLower;
} // XMLIndata::processCreateOverviewXMLData

XMLParserHelper*
XMLIndata::createXMLParserHelper(const set<const char*>& xmlFiles)               
{
   XMLParserHelper::domStringSet_t toSortByOrder;
   XMLParserHelper::domStringSet_t toSortByTag;
   toSortByTag.insert( m_createOverviewXStr->XMLStr() );
   toSortByTag.insert( m_addRegionXStr->XMLStr() );
   toSortByTag.insert( m_regionIDsXStr->XMLStr() );
   toSortByTag.insert( m_mapSupCovXStr->XMLStr() );
   toSortByTag.insert( m_mapSupNameXStr->XMLStr() );
   
   XStr topTag( "map_generation-mc2" );
   XMLParserHelper* helpParser =
      new XMLParserHelper( topTag.XMLStr(), toSortByOrder, toSortByTag );

   if ( ! helpParser->load( xmlFiles ) ) {
      mc2log << warn 
             << "XMLIndata::processXMLData Failed loading xmlFiles. "
             << "xmlFiles.size() == " << xmlFiles.size()
             << endl;
      // Print the file names
      set<const char*>::const_iterator it = xmlFiles.begin();
      for (uint32 i=0; it != xmlFiles.end(); ++it, ++i ){
         mc2dbg << "File: " << i << " " << *it << endl;
      }
      mc2log << error << "XMLIndata::processXMLData Failed loading xmlFiles"
             << endl;
      mc2dbg << "See details from the XML parser above the file list above"
             << endl;
         
      delete helpParser;
      return NULL;
   }
   
   return helpParser;
} // createXMLParserHelper

void
XMLIndata::parseAndAddRegionsToIndex( MapIndex& mapIndex, 
                                      const set<const char*>& xmlFiles )
{
   XMLParserHelper* helpParser = 
      createXMLParserHelper( xmlFiles );
   XMLParserHelper::xmlByTag_t xmlByTag = helpParser->getXMLNodesByTag();

   if ( ( m_regionIDsXStr == NULL ) ||
        ( m_addRegionXStr == NULL ) )
   {
      mc2log << error << "XMLIndata Problem with XML strings" << endl;
      exit(1);
   }

   map<MC2String, uint32> regionIDsByIdent;
   pair<XMLParserHelper::xmlByTag_t::iterator, 
        XMLParserHelper::xmlByTag_t::iterator> regionIDsRange =
           xmlByTag.equal_range( m_regionIDsXStr->XMLStr() );
   
   for ( XMLParserHelper::xmlByTag_t::iterator it = regionIDsRange.first;
         it != regionIDsRange.second; ++it ) {      
      XMLIndata::parseRegionIDs( it->second, regionIDsByIdent );
   }
   
   
   pair<XMLParserHelper::xmlByTag_t::const_iterator, 
        XMLParserHelper::xmlByTag_t::const_iterator> addRegionRange =
           xmlByTag.equal_range( m_addRegionXStr->XMLStr() );
               
   if ( addRegionRange.first != addRegionRange.second ) {
      // Add regions.
      mc2dbg << "To add regions" << endl;
      // XXX Most of the content in this method should probably be in 
      //     XMLIndata instead.
      mapIndex.addRegions( addRegionRange.first, addRegionRange.second,
                           regionIDsByIdent );
   }

   delete helpParser;
}


MC2BoundingBox
XMLIndata::parseBoundingBox(const DOMNode* bBoxTopNode){
   const XMLCh* name = bBoxTopNode->getNodeName();
   if ( ! WFXMLStr::equals(name, "bounding_box") ){
      mc2log << error << "Strange bounding box element name: " << name 
             << endl;
      MC2_ASSERT(false);
   }

   // upper and lower latitude and longitude.
   int32 ulat = MAX_INT32;
   int32 ulon = MAX_INT32;
   int32 llat = MAX_INT32;
   int32 llon = MAX_INT32;

   XMLCommonEntities::coordinateType coordType = XMLCommonEntities::MC2;

   // Get attributes
   DOMNamedNodeMap* attributes = bBoxTopNode->getAttributes();
   DOMNode* attribute = NULL;
   char* tmpStr = NULL;

   attribute = attributes->getNamedItem( X( "north_lat" ) );
   MC2_ASSERT( attribute != NULL );
   tmpStr = XMLString::transcode( attribute->getNodeValue() );
   if (! XMLCommonEntities::coordinateFromString(tmpStr, ulat, coordType)){
      mc2log << error << "Strange ulat value: "  << tmpStr  << endl;
      MC2_ASSERT(false);  
   }
   delete[] tmpStr;

   attribute = attributes->getNamedItem( X( "south_lat" ) );
   MC2_ASSERT( attribute != NULL );
   tmpStr = XMLString::transcode( attribute->getNodeValue() );
   if (! XMLCommonEntities::coordinateFromString(tmpStr, llat, coordType)){
      mc2log << error << "Strange llat value: "  << tmpStr  << endl;
      MC2_ASSERT(false);  
   }
   delete[] tmpStr;

   attribute = attributes->getNamedItem( X( "west_lon" ) );
   MC2_ASSERT( attribute != NULL );
   tmpStr = XMLString::transcode( attribute->getNodeValue() );
   if (! XMLCommonEntities::coordinateFromString(tmpStr, llon, coordType)){
      mc2log << error << "Strange llon value: "  << tmpStr  << endl;
      MC2_ASSERT(false);  
   }
   delete[] tmpStr;

   attribute = attributes->getNamedItem( X( "east_lon" ) );
   MC2_ASSERT( attribute != NULL );
   tmpStr = XMLString::transcode( attribute->getNodeValue() );
   if (! XMLCommonEntities::coordinateFromString(tmpStr, ulon, coordType)){
      mc2log << error << "Strange ulon value: "  << tmpStr  << endl;
      MC2_ASSERT(false);  
   }
   delete[] tmpStr;



   // Check validity.
   if (  (ulat == MAX_INT32) || (ulon == MAX_INT32) || 
         (llat == MAX_INT32) || (llon == MAX_INT32) ){
      mc2log << error << "Incomplete bounding box: " 
             << ulat << "," << ulon << "," << llat << "," << llon 
             << endl;
      MC2_ASSERT(false);  
   }
   return MC2BoundingBox(ulat, llon, llat, ulon); 
   
} // parseBoundingBox


set<StringTable::countryCode>
XMLIndata::parseCountryList(const DOMNode* cntrListTopNode){
   set<StringTable::countryCode> countryList;
   mc2dbg << "Parsing country list" << endl;


   const XMLCh* name = cntrListTopNode->getNodeName();
   if ( ! WFXMLStr::equals(name, "country_list") ){
      mc2log << error << "Strange country list element name: " << name 
             << endl;
      MC2_ASSERT(false);
   }

   const DOMNodeList* countryNodes = cntrListTopNode->getChildNodes();
   for (XMLSize_t i=0; i<countryNodes->getLength(); i++){
      DOMNode* countryNode = countryNodes->item(i);
      DOMNode::NodeType nodeType = 
         static_cast<DOMNode::NodeType>(countryNode->getNodeType());
      const XMLCh* name = countryNode->getNodeName();
      if ( nodeType == DOMNode::ELEMENT_NODE ){
         if ( WFXMLStr::equals(name,"country") ){
            DOMNamedNodeMap* attributes = countryNode->getAttributes();
            DOMNode* attribute = 
               attributes->getNamedItem( X( "id_string" ) );
            MC2_ASSERT( attribute != NULL );
            MC2String gmsName = 
               XMLString::transcode( attribute->getNodeValue() );
            StringTable::countryCode countryCode = 
               MapGenUtil::getCountryCodeFromGmsName( gmsName );
            mc2dbg << "   Country: " << gmsName << " " 
                   << countryCode << endl;
            if ( countryCode == StringTable::NBR_COUNTRY_CODES ){
               mc2log << error << "Strange GMS name: " 
                      << gmsName << endl;
               MC2_ASSERT(false);
            }
            countryList.insert(countryCode);
         }
         else {
            mc2log << error << "Strange element name: " << name << endl;
            MC2_ASSERT(false);
         }
      }
      else if ( nodeType == DOMNode::COMMENT_NODE ){
         // Don't bother with comments.
      }
      else {
         mc2log << error << "Strange node type: " << nodeType << endl;
         MC2_ASSERT(false);
      }
   }

   return countryList;
} // parseCountryList

void
XMLIndata::parseMapSupCoverage( const DOMNode* mapSupCovNode,
                                uint32 parentSupIdx,
                                int32 parentHierarchyLevel,
                                MapModuleNoticeContainer::mapSupCoverage_t&
                                mapSupCov,
                                mapSupAndCountries_t& supAndCntrListVec,
                                vector< vector<pair<uint32, uint32> > >&
                                covTreeByLevel )
{
   uint32 hierarchyLevel = parentHierarchyLevel+1; // Set to this level.
   mc2dbg << "Parsing map sup coverage of parent: " << parentSupIdx << " on level " << hierarchyLevel << endl;

   // If first time on this level, add it to hierarchy.
   if ( hierarchyLevel >= covTreeByLevel.size() ){
      vector<pair<uint32, uint32> > tmpVector;
      covTreeByLevel.push_back(tmpVector);
      mc2dbg8 << "covTreeByLevel levels size: " << covTreeByLevel.size() 
              << endl;
      MC2_ASSERT(hierarchyLevel < covTreeByLevel.size());
   }
   

   pair<MC2BoundingBox, MapGenEnums::mapSupplier> boxAndSup;
   pair<MapGenEnums::mapSupplier, 
      set<StringTable::countryCode> > supAndCntrList;
   set<DOMNode*> mapSupCovChildNodes;


   // Get attributes
   DOMNamedNodeMap* attributes = mapSupCovNode->getAttributes();
   
   DOMNode* attribute = attributes->getNamedItem( X( "map_supplier" ) );
   MC2_ASSERT( attribute != NULL );
   char* tmpStr = XMLString::transcode( attribute->getNodeValue() );
   MapGenEnums::mapSupplier mapSup = 
      MapGenUtil::getMapSupFromString(tmpStr);
   mc2dbg8 << "Map sup str:" << tmpStr << " map sup code:" << mapSup 
          << endl;
   boxAndSup.second = mapSup;
   supAndCntrList.first = mapSup;
   delete[] tmpStr;
   
   // Get parent bounding box
   MC2BoundingBox* parentBox = NULL;
   if ( parentSupIdx != MAX_UINT32 ){
      // We end up here if this is not the top most box.
      parentBox = &mapSupCov[parentSupIdx].first;
   }
   
   // Get child nodes.
   const DOMNodeList* childNodes = mapSupCovNode->getChildNodes();
   //(*mapSupConvIt)->getChildNodes();
   MC2BoundingBox bbox;
   for (XMLSize_t i=0; i<childNodes->getLength(); i++){
      DOMNode* node = childNodes->item(i);
      DOMNode::NodeType nodeType = 
         static_cast<DOMNode::NodeType>(node->getNodeType());
      const XMLCh* name = node->getNodeName();
      if ( nodeType == DOMNode::ELEMENT_NODE ){
         if ( WFXMLStr::equals(name,"bounding_box") ){
            bbox = parseBoundingBox(node);
            // Check that this box is actually inside parent box.
            if ((parentBox != NULL) && (!bbox.inside(*parentBox)) ){
               mc2log << error << "Problem with geometry of nested boxes. "
                      << "A child box should never cover an area outside "
                      << "the parent box." << endl; 
               mc2dbg << "Parent box: " << *parentBox << endl;
               mc2dbg << "Child box: " << bbox << endl;
               exit(1);
            }
            // Make sure this box does not overlap any other boxes of this
            // parent
            for (uint32 i=0; i<covTreeByLevel[hierarchyLevel].size(); i++){
               if (covTreeByLevel[hierarchyLevel][i].first == parentSupIdx){
                  // This is a sibling
                  uint32 siblingIdx = covTreeByLevel[hierarchyLevel][i].second;
                  MC2BoundingBox& siblingBox = mapSupCov[siblingIdx].first;
                  bool overlaps = true;
                  // Change this test if adjecent boxes are not allowed.
                  if ( ( bbox.getMinLat() >= siblingBox.getMaxLat() ||
                         bbox.getMinLon() >= siblingBox.getMaxLon() ||
                         bbox.getMaxLat() <= siblingBox.getMinLat() ||
                         bbox.getMaxLon() <= siblingBox.getMinLon() ) ){
                     overlaps = false;
                  }
                  if ( overlaps ) {
                     mc2log << error << "Problem with geometry of sibling "
                            << "boxes. A sibling box should never overlap one "
                            << "of its siblings." << endl; 
                     mc2dbg << "New sibling box: " << bbox << endl;
                     mc2dbg << "Added sibling box: " << siblingBox << endl;
                     exit(1);
                  }
               }
            }

            boxAndSup.first = bbox;
            mc2dbg8 << "Parsed bbox: " << bbox << endl;
            mapSupCov.push_back(boxAndSup);
   
         }
         else if ( WFXMLStr::equals(name,"country_list") ){
            const set<StringTable::countryCode>& cntrList = 
               parseCountryList(node);
            supAndCntrList.second = cntrList;
            supAndCntrListVec.push_back(supAndCntrList);
         }
         else if ( WFXMLStr::equals(name,"map_supplier_coverage") ){
            mapSupCovChildNodes.insert(node); // Handled recursively when 
            // all child elements have been read.
         }
         else {
            mc2log << error << "Strange element name: " << name << endl;
            MC2_ASSERT(false);
         }
         
      }
      else if ( nodeType == DOMNode::COMMENT_NODE ){
         // Don't bother with comments.
         }
      else {
         mc2log << error << "Strange node type: " << nodeType << endl;
         MC2_ASSERT(false);
      }
   }

   // Update coverage tree.
   MC2_ASSERT(mapSupCov.size() > 0);
   uint32 supplierIdx = mapSupCov.size() - 1; // This index.
   covTreeByLevel[hierarchyLevel].push_back(make_pair(parentSupIdx, 
                                                      supplierIdx));
   mc2dbg << "   Parsed " << parentSupIdx << " -> " << supplierIdx << endl;
   mc2dbg8 << "Level: " << hierarchyLevel << " size: " 
           << covTreeByLevel[hierarchyLevel].size() << endl;

   // Handle map supplier coverage child nodes.
   set<DOMNode*>::const_iterator childIt = mapSupCovChildNodes.begin();
   while ( childIt != mapSupCovChildNodes.end() ){
      // Recursive call.
      parseMapSupCoverage( *childIt,
                           supplierIdx,
                           hierarchyLevel,
                           mapSupCov,
                           supAndCntrListVec,
                           covTreeByLevel );
      ++childIt;
   }


} // parseMapSupCoverage


void
XMLIndata::parseMapSupName( DOMNode* mapSupNameNode,
                            MapModuleNoticeContainer::mapSupNamesByMapSup_t& 
                            mapSupNames )
{
   // Get attributes
   DOMNamedNodeMap* attributes = mapSupNameNode->getAttributes();
   
   // Get map supplier.
   DOMNode* attribute = attributes->getNamedItem( X( "map_supplier" ) );
   MC2_ASSERT( attribute != NULL );
   char* tmpStr = XMLString::transcode( attribute->getNodeValue() );
   MapGenEnums::mapSupplier mapSup = 
      MapGenUtil::getMapSupFromString(tmpStr);
   mc2dbg << "Parsing names of map sup str:\"" << tmpStr 
          << "\" map sup code:\"" << mapSup 
          << "\"" << endl;
   if ( mapSup == MapGenEnums::unknownSupplier ){
      mc2log << error << "Unknown map supplier identified by string \""
             << tmpStr << "\", exits!" << endl;
      exit(1);
   }

   // Get or create names collection of this map supplier
   MapModuleNoticeContainer::mapSupNamesByMapSup_t::iterator mapSupIt = 
      mapSupNames.find(mapSup);
   if ( mapSupIt == mapSupNames.end() ){
      NameCollection tmpCollection;
      mapSupIt = mapSupNames.insert(make_pair(mapSup, tmpCollection)).first;
      mc2dbg8 << "Added names for map supplier ID: " << mapSup << endl;
   }
   NameCollection& names = mapSupIt->second;

   // Get child nodes.
   const DOMNodeList* childNodes = mapSupNameNode->getChildNodes();
   for (XMLSize_t i=0; i<childNodes->getLength(); i++){
      DOMNode* node = childNodes->item(i);
      DOMNode::NodeType nodeType = 
         static_cast<DOMNode::NodeType>(node->getNodeType());
      const XMLCh* name = node->getNodeName();
      if ( nodeType == DOMNode::ELEMENT_NODE ){
         if ( WFXMLStr::equals(name,"name") ){
            bool result = parseName( node, names );
            if ( !result ){
               mc2log << error << "Strange name node:" << node << endl;
               MC2_ASSERT(false);
            }
         }
         else {
            mc2log << error << "Strange element name: " << name << endl;
            MC2_ASSERT(false);
         }
         
      }
      else if ( nodeType == DOMNode::COMMENT_NODE ){
         // Don't bother with comments.
         }
      else {
         mc2log << error << "Strange node type: " << nodeType << endl;
         MC2_ASSERT(false);
      }
   }

} // parseMapSupName


void
XMLIndata::parseAndAddMapSupCoverageToIndex( MapIndex& mapIndex, 
                                        const char* xmlFileName )
{
mc2dbg << "Called: parseAndAddMapSupCoverageToIndex" << endl;

   // Stores the bounding box and the corresponding map supplier
   MapModuleNoticeContainer::mapSupCoverage_t mapSupCov;

   // Stores the countries covered by a bounding box. Used for checking
   // the bounding box coverage validity.
   vector <pair<MapGenEnums::mapSupplier,
      set<StringTable::countryCode> > > supAndCntrListVec;

   // Stored the hierarchy of bounding boxes as the indexes in the 
   // mapSupCov vector.
   // First:  Index of the larger box.
   // Second: Index of a box covered by the larger box.

   // Vector of vectors with hierarchy of supplier boundig boxes. 
   // Oter vector: Index determines level in the xml hierarchy.
   // Inner vector: Ordered by the order to test the bounding boxes when 
   //               finding copyright of an area.
   // Element pair
   //    First:  Parent idx in mapSupCov.
   //    Second: Child idx in mapSupCov.
   vector< vector<pair<uint32, uint32> > > covTreeByLevel;


   // Create the xml parser helper.
   set<const char*> fileNameSet;
   fileNameSet.insert(xmlFileName);
   XMLParserHelper* helpParser = 
      createXMLParserHelper( fileNameSet );

   //XMLParserHelper::xmlByOrder_t& mapSupCovElmts = 
   //   helpParser->getXMLNodesByOrder();
   //   XMLParserHelper::xmlByOrder_t::const_iterator mapSupConvIt = 
   //   mapSupCovElmts.begin();

   XMLParserHelper::xmlByTag_t mapSupCovElmts = 
      helpParser->getXMLNodesByTag();
   XMLParserHelper::xmlByTag_t::const_iterator mapSupConvIt = 
      mapSupCovElmts.begin();
   while ( mapSupConvIt != mapSupCovElmts.end() ){
      DOMNode* mapSupCovElm = (*mapSupConvIt).second;

      parseMapSupCoverage( mapSupCovElm,
                           MAX_UINT32, // mapSupplieIdx,
                           -1,          // parent hierarchy level.
                           mapSupCov,
                           supAndCntrListVec,
                           covTreeByLevel );

      ++mapSupConvIt;
   }
   
   mc2log << info << "Added " << mapSupCov.size() << " map supplier " 
          << "coverage bounding boxes." << endl;
   
   mc2dbg8 << "covTreeByLevel.size(): " << covTreeByLevel.size() << endl;
   
   // Fill in coverage tree.
   MapModuleNoticeContainer::coverageTree_t coverageTree;
   for (uint32 level=0; level<covTreeByLevel.size(); level++){
      vector<pair<uint32, uint32> >& hrchyPairs = covTreeByLevel[level];
      for (uint32 i=0; i<hrchyPairs.size(); i++){
         coverageTree.push_back(hrchyPairs[i]);
      }
   }

   mapIndex.setMapSupCoverage( mapSupCov, coverageTree );
   delete helpParser;
} // parseAndAddMapSupCoverageToIndex

void
XMLIndata::parseAndAddMapSupNamesToIndex( MapIndex& mapIndex, 
                                          const char* mapSupNameXmlFile ){


   // One NameCollection per map supplier
   MapModuleNoticeContainer::mapSupNamesByMapSup_t mapSupNamesByMapSup;


   // Create the xml parser helper.
   set<const char*> fileNameSet;
   fileNameSet.insert(mapSupNameXmlFile);
   XMLParserHelper* helpParser = 
      createXMLParserHelper( fileNameSet );

   XMLParserHelper::xmlByTag_t mapSupNameElmts = 
      helpParser->getXMLNodesByTag();
   XMLParserHelper::xmlByTag_t::const_iterator mapSupNameIt = 
      mapSupNameElmts.begin();
   while ( mapSupNameIt != mapSupNameElmts.end() ){
      DOMNode* mapSupNameElm = (*mapSupNameIt).second;
      mc2dbg8 << "DOMNode name: " << mapSupNameElm->getNodeName() << endl;
      parseMapSupName( mapSupNameElm,
                       mapSupNamesByMapSup );

      ++mapSupNameIt;
   }

   
   mc2log << info << "Added " << mapSupNamesByMapSup.size() 
          << " map suppliers names." << endl;
   
   mapIndex.setMapSupNames( mapSupNamesByMapSup );
   delete helpParser;


} // parseAndAddMapSupNamesToIndex



bool 
XMLIndata::parseCreateOverview( 
                              const DOMNode* createOverview, 
                              map<MC2String, MC2String>& overviewByUnderview )
{
   if ( XMLString::equals( createOverview->getNodeName(),
                           "create_overviewmap" ) )
   {
      bool ok = true;
      
      // Get attributes
      DOMNamedNodeMap* attributes = createOverview->getAttributes();

      
      // Note that overview_ident attribute is #REQUIRED.
      DOMNode* attribute = 
         attributes->getNamedItem( X( "overview_ident" ) );
      MC2_ASSERT( attribute != NULL );
      char* tmpStr = XMLUtility::transcodefromucs(
         attribute->getNodeValue() );
      MC2String overviewIdent( tmpStr );
      delete [] tmpStr;

      // Get children
      DOMNode* child = createOverview->getFirstChild();
   
      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), "map" ) ) {
                  attributes = child->getAttributes();
                  // map_ident is #REQUIRED. 
                  attribute = attributes->getNamedItem( X( "map_ident" ) );
                  MC2_ASSERT( attribute != NULL );
                  tmpStr = XMLUtility::transcodefromucs(
                        attribute->getNodeValue() );
                  MC2String mapIdent( tmpStr );

                  // Add to data to map.
                  overviewByUnderview[ mapIdent ] = overviewIdent;

                  delete [] tmpStr;
               } else {
                  mc2log << warn 
                         << "XMLIndata::parseCreateOverview:"
                         << " odd Element in create_overviewmap element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn 
                      << "XMLIndata::parseCreateOverview odd "
                      << "node type in create_overviewmap element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
      
      return ok;      
      
   } else {
      // Not a create overview node.
      mc2log << warn 
             << "XMLIndata::parseCreateOverview:"
             << " not a create_overview node."
             << createOverview->getNodeName() << endl;
      return false;
   }
}

bool 
XMLIndata::parseRegionIDs( 
                              const DOMNode* regionIDs, 
                              map<MC2String, uint32>& regionIDsByIdent )
{
   if ( XMLString::equals( regionIDs->getNodeName(), "region_ids" ) ) {
      
      // Get children
      DOMNode* child = regionIDs->getFirstChild();
   
      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), "region" ) ) {
                  DOMNamedNodeMap* attributes = child->getAttributes();
                  
                  // ident is #REQUIRED. 
                  DOMNode* attribute = attributes->getNamedItem( 
                     X( "ident" ) );
                  MC2_ASSERT( attribute != NULL );
                  char* tmpStr = XMLUtility::transcodefromucs(
                     attribute->getNodeValue() );
                  MC2String ident( tmpStr );
                  delete [] tmpStr;
                  
                  // id is #REQUIRED. 
                  attribute = attributes->getNamedItem( X( "id" ) );
                  MC2_ASSERT( attribute != NULL );
                  tmpStr = XMLUtility::transcodefromucs(
                     attribute->getNodeValue() );
                  uint32 id = strtol( tmpStr, (char**) NULL, 10 );
                  delete [] tmpStr;

                  // Add to data to map.
                  regionIDsByIdent[ ident ] = id;

               } else {
                  mc2log << warn 
                         << "XMLIndata::parseRegionIDs:"
                         << " odd Element in region_ids element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn 
                      << "XMLIndata::parseCreateOverview odd "
                      << "node type in region_ids element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
     
      return true;      
      
   } else {
      // Not a parse region ids node.
      mc2log << warn 
             << "XMLIndata::parseRegionIDs:"
             << " not a region_ids node."
             << regionIDs->getNodeName() << endl;
      return false;
   }

}

bool 
XMLIndata::parseAddRegion( 
                              const DOMNode* addRegion,
                              NameCollection& regionNames,
                              TopRegionMatch::topRegion_t& regionType,
                              MC2String& regionIdent,
                              multimap<MC2String, ItemIdentifier*>& contents,
                              set<MC2String>& insideRegions )
{

   if ( XMLString::equals( addRegion->getNodeName(), "add_region" ) ) {
      
      // Get attributes
      DOMNamedNodeMap* attributes = addRegion->getAttributes();

      
      // Note that region_ident attribute is #REQUIRED.
      DOMNode* attribute = 
         attributes->getNamedItem( X( "region_ident" ) );
      MC2_ASSERT( attribute != NULL );
      char* tmpStr = XMLUtility::transcodefromucs(
            attribute->getNodeValue() );
      regionIdent = tmpStr;
      delete [] tmpStr;

      // Note that type attribute is always present.
      attribute = 
         attributes->getNamedItem( X( "type" ) );
      MC2_ASSERT( attribute != NULL );
      tmpStr = XMLUtility::transcodefromucs(
            attribute->getNodeValue() );
      regionType = XMLCommonEntities::stringToTopRegionType( tmpStr );
      delete [] tmpStr;

      
      // Get children
      DOMNode* child = addRegion->getFirstChild();

      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), "name" ) ) {
                  
                  parseName( child, regionNames );
                  
               } else if ( XMLString::equals( child->getNodeName(),
                                              "content" ) ) 
               {
                  
                  parseContent( child, contents );
               
               } else if ( XMLString::equals( child->getNodeName(),
                                              "inside_region" ) )
               {
                  
                  attributes = child->getAttributes();
                  // ident is #REQUIRED. 
                  attribute = attributes->getNamedItem( X( "ident" ) );
                  MC2_ASSERT( attribute != NULL );
                  tmpStr = XMLUtility::transcodefromucs(
                        attribute->getNodeValue() );
                  insideRegions.insert( tmpStr );
                  delete [] tmpStr;
                  
               } else {
                  mc2log << warn 
                         << "XMLIndata::parseAddRegion:"
                         << " odd Element in add_region element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn 
                      << "XMLIndata::parseAddRegion odd "
                      << "node type in add_region element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
      
      return true; 
      
   } else {
      // Not an add region node.
      mc2log << warn 
             << "XMLIndata::parseAddRegion:"
             << " not an add_region node."
             << addRegion->getNodeName() << endl;
      return false;
   }

}

bool
XMLIndata::parseName( const DOMNode* name,
                      NameCollection& names ) 
{
   if ( XMLString::equals( name->getNodeName(), "name" ) ) {
      
      // Get attributes
      DOMNamedNodeMap* attributes = name->getAttributes();

      LangTypes::language_t lang;
      ItemTypes::name_t nameType;
      
      // Note that language attribute is #REQUIRED.
      DOMNode* attribute = 
         attributes->getNamedItem( X( "language" ) );
      MC2_ASSERT( attribute != NULL );
      MC2String langStr = 
         XMLUtility::transcodefromucs(attribute->getNodeValue() );
      // Replace any occurance of '_' with space.
      for (MC2String::iterator strIt = langStr.begin();
           strIt != langStr.end(); ++strIt){
         if ( *strIt == '_' ){
            *strIt = ' ';
         }
      }

      lang = LangTypes::getStringAsLanguage( langStr.c_str(), true );
      if ( lang == LangTypes::invalidLanguage ){
         mc2log << error << "Could not interpret language code of string"
                << endl;
         MC2_ASSERT(false);
      }

      // Note that type attribute is always present.
      attribute = 
         attributes->getNamedItem( X( "type" ) );
      MC2_ASSERT( attribute != NULL );
      const char* tmpStr = XMLUtility::transcodefromucs(
            attribute->getNodeValue() );
      nameType = ItemTypes::getStringAsNameType( tmpStr );
      delete [] tmpStr;

      // Get the name.
      tmpStr = XMLUtility::getChildTextValue( name );
      
      // Add name
      Name* tmpName = new Name( tmpStr, lang, nameType );
      names.addName( tmpName );
      mc2dbg << "Added name: " << (*tmpName) << endl;
      mc2dbg8 << "All names: " << names << endl;
      delete tmpStr;

      return true;
   } else {
      // Not a name node.
      mc2log << warn 
             << "XMLIndata::parseName:"
             << " not a name node."
             << name->getNodeName() << endl;
      return false;
   }
}

bool
XMLIndata::parseContent(const DOMNode* content,
                        multimap<MC2String, ItemIdentifier*>& contents )
{
   if ( XMLString::equals( content->getNodeName(), "content" ) ) {
      
      // Get attributes
      DOMNamedNodeMap* attributes = content->getAttributes();

      
      // Note that map_ident attribute is #REQUIRED.
      DOMNode* attribute = 
         attributes->getNamedItem( X( "map_ident" ) );
      MC2_ASSERT( attribute != NULL );
      char* tmpStr = XMLUtility::transcodefromucs(
            attribute->getNodeValue() );
      MC2String mapIdent( tmpStr );
      delete [] tmpStr;

      // Get children
      DOMNode* child = content->getFirstChild();

      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "item_ident" ) )
               {
                  
                  // Parse item ident
                  ItemIdentifier* itemIdent = new ItemIdentifier();
                  parseItemIdent( child, *itemIdent );

                  contents.insert( make_pair( mapIdent, itemIdent ) );
                  
               } else if ( XMLString::equals( child->getNodeName(),
                                              "whole_map" ) )
               {
                  
                  // Content is all of the specified map.
                  contents.insert( make_pair( mapIdent, 
                                              (ItemIdentifier*) NULL ) );
               
               } else {
                  mc2log << warn 
                         << "XMLIndata::parseContent:"
                         << " odd Element in content element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn 
                      << "XMLIndata::parseContent odd "
                      << "node type in content element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
      
      return true; 
      
   } else {
      // Not a content node.
      mc2log << warn 
             << "XMLIndata::parseContent:"
             << " not a content node."
             << content->getNodeName() << endl;
      return false;
   }
}

bool
XMLIndata::parseItemIdent( const DOMNode* node, 
                                     ItemIdentifier& itemIdent )
{
   if ( XMLString::equals( node->getNodeName(), "item_ident" ) ) {
      
      // Get attributes
      DOMNamedNodeMap* attributes = node->getAttributes();

      
      // Note that name attribute is #REQUIRED.
      DOMNode* attribute = 
         attributes->getNamedItem( X( "name" ) );
      MC2_ASSERT( attribute != NULL );
      char* name = XMLUtility::transcodefromucs(
            attribute->getNodeValue() );

      // Note that item_type attribute is #REQUIRED.
      attribute = attributes->getNamedItem( X( "type" ) );
      MC2_ASSERT( attribute != NULL );
      char* tmpStr = XMLUtility::transcodefromucs(
         attribute->getNodeValue() );
      ItemTypes::itemType itemType = 
         ItemTypes::getItemTypeFromString( tmpStr );
      delete [] tmpStr;
      
      // Get children
      DOMNode* child = node->getFirstChild();

      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "position_item" ) ) 
               {
                  
                  // Parse position item.
                  MC2String errorCode, errorMessage;
                  int32 lat;
                  int32 lon;
                  uint16 angle;
                  XMLCommonElements::getPositionItemData( child, 
                                                          lat, lon,
                                                          angle,
                                                          errorCode,
                                                          errorMessage );
                  
                  itemIdent.setParameters( name, itemType, lat, lon );
                  
               } else if ( XMLString::equals( child->getNodeName(),
                                              "inside_name" ) ) 
               {
                  
                  char* insideName = 
                     XMLUtility::getChildTextValue( child );
                  itemIdent.setParameters( name, itemType, insideName );

                  delete insideName;
               
               } else {
                  mc2log << warn 
                         << "XMLIndata::parseItemIdent:"
                         << " odd Element in item_ident element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn 
                      << "XMLIndata::parseItemIdent odd "
                      << "node type in item_ident element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
      
      delete [] name;
      
      return true; 
      
   } else {
      // Not a item ident node.
      mc2log << warn 
             << "XMLIndata::parseItemIdent:"
             << " not a item_ident node."
             << node->getNodeName() << endl;
      return false;
   }
}

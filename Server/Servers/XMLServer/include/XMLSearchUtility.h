/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLSEARCHUTILITY_H
#define XMLSEARCHUTILITY_H

#include "config.h"
#include "XMLCommonEntities.h"
#include "XMLUtility.h"
#include "SearchRequestParameters.h"

class SearchResultRequest;

class VanillaMatch;
class OverviewMatch;
class VanillaRegionMatch;
class SearchMatch;
class XMLParserThread;

namespace XMLSearchUtility {

/**
 * Prints a search_item node as child to list.
 *
 * @param list Where to put the search_item node as last child.
 * @param reply The document to make the nodes in.
 * @param match The match to print.
 * @param currentLocation The name of the current location.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param latlonForSearchHits If to add lat and lon from matches as
 *                            explicit_itemid. OBSOLETE!
 * @param lat The latitude for the item.
 * @param lon The longitude for the item.
 * @param positionSearchMatch If to add lat and lon as lat and lon 
 *                            nodes to the search_item.
 * @param bbox Added as a boundingbbox to the item if not NULL and is valid.
 * @param positionSystem The coordinate system to use for 
 *                       coordinates.
 * @param params The search request parameters used to select what
 *               search areas to print for the seacrch area.
 * @param usePersistentIds If to not use normal itemid but "persistent"
 *                         identifiers, not supported by almost all requests.
 * @param thread Thread to get Handler to get image names from.
 * @param addImageName If to add image attribute using searchHandler.
 * @param includeInfoItem If to add an info_item in the node.
 * @return The created search item element.
 */
DOMElement* appendSearchItem( DOMNode* list, 
                              DOMDocument* reply,
                              const VanillaMatch* match,
                              const char* currentLocation,
                              int indentLevel,
                              bool indent,
                              bool latlonForSearchHits = false,
                              int32 lat = 0,
                              int32 lon = 0,
                              bool positionSearchMatch = false,
                              MC2BoundingBox* bbox = NULL,
                              XMLCommonEntities::coordinateType 
                              positionSystem = XMLCommonEntities::MC2,
                              const SearchRequestParameters& params = 
                              SearchRequestParameters(),
                              bool usePersistentIds = false,
                              XMLParserThread* thread = NULL,
                              bool addImageName = false,
                              bool includeInfoItem = false );


/**
 * Appends a search_item_list to curr.
 *
 * @param curr Where to put the search_item_list as last child.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param searchItemStartingIndex The index in matches to start list 
 *                                at.
 * @param searchItemEndingIndex The index in matches to stop list at.
 * @param latlonForSearchHits If to add lat and lon from matches as
 *                            explicit_itemid. OBSOLETE!
 * @param positionSearchItems If to add lat and lon as lat and lon 
 *                            nodes to search_items.
 * @param positionSystem The coordinate system to use for 
 *                       coordinates.
 * @param params The search request parameters used to select what
 *               search areas to print for the seacrch area.
 * @param optionalSearchList If not NULL this list is used to add
 *                           search_item into.
 * @param thread Thread to get Handler to get image names from.
 * @param headingID The heading Id for the list.
 * @param usePersistentIds If to not use normal itemid but "persistent"
 *                         identifiers, not supported by almost all requests.
 * @param includeInfoItem If to add an info_item in the nodes.
 * @return The created search item list, can be optionalSearchList.
 */
DOMElement*
appendSearchItemList( DOMNode* curr, DOMDocument* reply,
                      int indentLevel, bool indent,
                      const SearchResultRequest& req,
                      const char* currentLocation,
                      int32 searchItemStartingIndex,
                      int32 searchItemEndingIndex,
                      bool latlonForSearchHits,
                      bool positionSearchItems,
                      XMLCommonEntities::coordinateType 
                      positionSystem,
                      const SearchRequestParameters& 
                      params,
                      DOMElement* optionalSearchList = NULL,
                      XMLParserThread* thread = NULL,
                      uint32 headingID = MAX_UINT32,
                      bool usePersistentIds = false,
                      bool includeInfoItem = false );

/**
 * Appends search_hit_list to curr with search_area nodes.
 *
 * @param curr Where to put the search_item_list as last child.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param searchItemStartingIndex The index in matches to start list 
 *                                at.
 * @param searchItemEndingIndex The index in matches to stop list at.
 * @param latlonForSearchHits If to add lat and lon from matches as
 *                            explicit_itemid. OBSOLETE!
 * @param positionSearchItems If to add lat and lon as lat and lon 
 *                            nodes to search_items.
 * @param positionSystem The coordinate system to use for 
 *                       coordinates.
 * @return DOMElement to created search item list
 */
DOMElement* 
appendSearchAreaList( DOMNode* curr, DOMDocument* reply,
                      int indentLevel, bool indent,
                      const SearchResultRequest& req,
                      const char* currentLocation,
                      int32 searchItemStartingIndex,
                      int32 searchItemEndingIndex,
                      bool latlonForSearchHits,
                      bool positionSearchItems,
                      XMLCommonEntities::coordinateType 
                      positionSystem,
                      const SearchRequestParameters& params,
                      DOMElement* inlistNode = NULL );



/**
 * Prints n search_area nodes as children to list where n
 * is the size of the OverviewMatch.
 *
 * @param list Where to put the search_area node as last child.
 * @param reply The document to make the nodes in.
 * @param match The match to print.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param showLatLon If to print lat and lon nodes.
 * @param lat The latitude of the area center, if any.
 * @param lon The longitude of the area center, if any.
 * @param bbox The boundingbox for the search_areas. Default NULL.
 * @param positionSystem The coordinate format to use. Default MC2.
 * @param params The search request parameters used to select what
 *               search areas to print for the seacrch area.
 */
void appendSearchArea( DOMNode* list, 
                       DOMDocument* reply,
                       OverviewMatch* match,
                       int indentLevel,
                       bool indent,
                       bool showLatLon = false,
                       int32 lat = MAX_INT32,
                       int32 lon = MAX_INT32,
                       MC2BoundingBox* bbox = NULL,
                       XMLCommonEntities::coordinateType positionSystem = 
                       XMLCommonEntities::MC2,
                       const SearchRequestParameters& params = 
                       SearchRequestParameters() );


/**
 * Prints a search_area node as children to list, using a 
 * VanillaRegionMatch.
 *
 * @param list Where to put the search_area node as last child.
 * @param reply The document to make the nodes in.
 * @param match The match to print.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param showLatLon If to print lat and lon nodes.
 * @param lat The latitude of the area center, if any.
 * @param lon The longitude of the area center, if any.
 * @param bbox The boundingbox for the search_area. Default NULL.
 * @param positionSystem The coordinate format to use. Default MC2.
 * @param params The search request parameters used to select what
 *               search areas to print for the seacrch area.
 */
void appendSearchArea( DOMNode* list, 
                       DOMDocument* reply,
                       VanillaRegionMatch* match,
                       int indentLevel,
                       bool indent,
                       bool showLatLon = false,
                       int32 lat = MAX_INT32,
                       int32 lon = MAX_INT32,
                       MC2BoundingBox* bbox = NULL,
                       XMLCommonEntities::coordinateType positionSystem = 
                       XMLCommonEntities::MC2,
                       const SearchRequestParameters& params = 
                       SearchRequestParameters() );

/**
 * Prints a search_area node as child to list, uses explicit 
 * parametrs.
 *
 * @param list Where to put the search_area node as last child.
 * @param reply The document to make the node in.
 * @param type The type of area match.
 * @param name The name of the area.
 * @param mapID The mapID.
 * @param itemID The itemID.
 * @param mask The mask.
 * @param regions The regions of this area.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param locationName The location name of the area.
 * @param showLatLon If to print lat and lon nodes.
 * @param lat The latitude of the area center, if any.
 * @param lon The longitude of the area center, if any.
 * @param bbox The boundingbox for the search_area. Default NULL.
 * @param positionSystem The coordinate format to use. Default MC2.
 * @param params The search request parameters used to select what
 *               search areas to print for the seacrch area.
 */
void appendSearchArea( DOMNode* list,
                       const SearchMatch* match,
                       DOMDocument* reply,
                       uint32 type,
                       const char* name,
                       uint32 mapID,
                       uint32 itemID,
                       uint32 mask,
                       vector<VanillaRegionMatch*>& regions,
                       int indentLevel,
                       bool indent,
                       const char* locationName,
                       bool showLatLon = false,
                       int32 lat = MAX_INT32,
                       int32 lon = MAX_INT32,
                       MC2BoundingBox* bbox = NULL,
                       XMLCommonEntities::coordinateType positionSystem = 
                       XMLCommonEntities::MC2,
                       const SearchRequestParameters& params = 
                       SearchRequestParameters() ) ;
 
/**
 * Prints a boundingbox node as child to curr.
 * @param curr Where to put the boundingbox node as last child.
 * @param reply The document to make the nodes in.
 * @param format The coordinate format to use.
 * @param northLat The northmost latitude.
 * @param westLon The westmost longitude.
 * @param southLat The southmost latitude.
 * @param eastLon The eastmost longitude.
 * @param indentLevel The indent to use.
 * @param indent If indent.
 */
void appendBoundingbox( DOMNode* curr, 
                        DOMNode* reply,
                        XMLCommonEntities::coordinateType 
                        format,
                        int32 northLat,
                        int32 westLon,
                        int32 southLat,
                        int32 eastLon,
                        int indentLevel,
                        bool indent );

/**
 * Appends a position_item as child to cur.
 * @param lat The latitude.
 * @param lon The longitude.
 * @param angle The angle of the position. If > 360 then not valid
 *              and is not added.
 * @param positionSystem The positionSystem to use.
 * @param cur Where to put the position_item node as last child.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If indent.
 */
void appendPositionItem( int32 lat, int32 lon, uint16 angle,
                         XMLCommonEntities::coordinateType positionSystem,
                         DOMNode* cur, DOMDocument* reply,
                         int indentLevel, bool indent );


/**
 * Makes a VanillaMatch from data.
 *
 * @param mapID The mapID for the VanillaMatch.
 * @param itemID The itemID for the VanillaMatch.
 * @param offset The offset for the VanillaMatch.
 * @param name The name for the VanillaMatch.
 * @param locationName The name of the location.
 * @param streetNbr The streetNbr for the VanillaMatch.
 * @param itemType The itemType for the VanillaMatch.
 * @return A VanillaMatch or NULL if not suppored itemType.
 */
VanillaMatch* makeVanillaMatchFromData( uint32 mapID, uint32 itemID,
                                        uint16 offset,
                                        const char* name,
                                        const char* locationName,
                                        uint32 streetNbr, 
                                        uint32 itemType );

      
      
/**
 * Gets the data from an search_item node.
 * @param searchItem The search_item node to get data from.
 * @param mapID The mapID of the item.
 * @param itemID The itemID of the item.
 * @param offset The offset of the item.
 * @param name Name of the item, user must delete this string!
 * @param itemType SEARCH_STREETS, SEARCH_COMPANIES or
 *                 SEARCH_CATEGORIES.
 * @param errorCode If problem then this is set to error code.
 * @param errorMessage If problem then this is set to error message.
 * @return True if searchItem really is a search_item node.
 */
bool getSearchItemData( const DOMNode* searchItem,
                        uint32& mapID, uint32& itemID,
                        uint16& offset, char*& name,
                        uint32& streetNbr,
                        uint32& itemType,
                        MC2String& errorCode,
                        MC2String& errorMessage );


/**
 * Gets the data from an search_area node.
 * @param searchArea The search_area node to get data from.
 * @param mapID The mapID of the area.
 * @param itemID The itemID of the area.
 * @param maskID The maskID of the area.
 * @param name Name of the area, user must delete this string!
 * @param areaType SEARCH_MUNICIPALS, SEARCH_BUILT_UP_AREAS or
 *                 SEARCH_CITY_PARTS.
 * @param errorCode If problem then this is set to error code.
 * @param errorMessage If problem then this is set to error message.
 * @return True if searchArea really is a search_area node.
 */
bool getSearchAreaData( DOMNode* searchArea,
                        uint32& mapID, uint32& itemID,
                        uint32& maskID, char*& name,
                        uint32& areaType,
                        MC2String& errorCode,
                        MC2String& errorMessage );
/**
 * Extracts data from a search_item and puts it into a VanillaMatch.
 *
 * @param searchItem The search_item node to get the data from.
 * @param errorCode If problem then this is set to error code.
 * @param errorMessage If problem then this is set to error message.
 * @return A VanillaMatch or NULL if something was really wrong.
 */
VanillaMatch* getVanillaMatchFromSearchItem( const DOMNode* searchItem,
                                             MC2String& errorCode,
                                             MC2String& errorMessage );

/**
 * Creates a SearchMatch from a itemid element.
 *
 * @param itemId ItemID element.
 */
SearchMatch* 
getVanillaMatchFromItemId( const DOMNode* itemId );


/**
 * Converts a ItemTypes::itemType to a Search match type.
 * @param type The ItemTypes::itemType to convert.
 * @return The converted ItemTypes::itemType value.
 */
uint32 itemTypeToSearchItemType( ItemTypes::itemType type );

/**
 * appends search_list element including the list of matches to parentNode.
 *
 * @param parentNode The parent node.
 * @param replyDoc The owner document
 * @param req The search result request with the matches
 * @param maxHits Maximum number of hits to print
 * @param positionSystem Coordinate system
 * @param thread The parser thread.
 */
void appendSearchListElement( DOMNode* parentNode, 
                              DOMDocument* replyDoc,
                              const SearchResultRequest* req,
                              uint32 maxHits,
                              XMLCommonEntities::coordinateType positionSystem,
                              LangTypes::language_t language,
                              XMLParserThread* thread);
} // End namespace XMLSearchUtility

#endif // XMLSEARCHUTILITY_H


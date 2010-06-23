/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLSearchUtility.h"

#include "XMLTool.h"
#include "SearchResultRequest.h"
#include "SearchRequestParameters.h"
#include "SearchMatch.h"
#include "SearchParserHandler.h"
#include "StringConversion.h"
#include "STLStringUtility.h"
#include "XMLServerElements.h"
#include "ExternalSearchConsts.h"
#include "XMLCategoryListNode.h"
#include "PersistentSearchIDs.h"
#include "XMLParserThread.h"
#include "XMLItemInfoUtility.h"
#include "XMLItemDetailUtility.h"
#include <memory>

namespace XMLSearchUtility {

void
appendSearchArea( DOMNode* list, 
                  DOMDocument* reply,
                  OverviewMatch* match,
                  int indentLevel,
                  bool indent,
                  bool showLatLon,
                  int32 lat,
                  int32 lon,
                  MC2BoundingBox* bbox,
                  XMLCommonEntities::coordinateType
                  positionSystem,
                  const SearchRequestParameters& params ) {
   for ( uint32 i = 0 ; i < 1 ; i++ ) {
      vector<VanillaRegionMatch*> regions;
      for ( uint32 j = 0 ; j < match->getNbrRegions() ; ++j ) {
         regions.push_back( match->getRegion( j ) );
      }
      appendSearchArea( list, match, reply, match->getType( ), 
                        match->getName0(), match->getMapID(),
                        match->getItemID(), MAX_UINT32 /* Mask */,
                        regions, indentLevel, indent, 
                        match->getLocationName(),
                        showLatLon, lat, lon,
                        bbox, positionSystem, params );
   }
}


void 
appendSearchArea( DOMNode* list, 
                  DOMDocument* reply,
                  VanillaRegionMatch* match,
                  int indentLevel,
                  bool indent,
                  bool showLatLon,
                  int32 lat,
                  int32 lon,
                  MC2BoundingBox* bbox,
                  XMLCommonEntities::coordinateType
                  positionSystem,
                  const SearchRequestParameters& params ) {

   vector<VanillaRegionMatch*> regions;
   for ( uint32 i = 0 ; i < match->getNbrRegions() ; ++i ) {
      regions.push_back( match->getRegion( i ) );
   }
   appendSearchArea( list, match, reply, match->getType(), match->getName(),
                     match->getMapID(), match->getMainItemID(), 
                     MAX_UINT32, regions, indentLevel, indent, 
                     match->getLocationName(),
                     showLatLon, lat, lon,
                     bbox, positionSystem, params );
}

DOMElement* 
appendSearchAreaList( DOMNode* curr, DOMDocument* reply,
                      int indentLevel, bool indent,
                      const SearchResultRequest& req,
                      const char* currentLocation,
                      int32 searchItemStartingIndex,
                      int32 searchItemEndingIndex,
                      bool latlonForSearchHits,
                      bool positionSearchItems,
                      XMLCommonEntities::coordinateType positionSystem,
                      const SearchRequestParameters& params,
                      DOMElement* inlistNode ) {
   using namespace XMLTool;
   DOMElement* listNode;
   if ( inlistNode ) {
      listNode = inlistNode;
   } else {
      listNode = reply->createElement( X( "search_hit_list" ) );
      curr->appendChild( listNode );
   }

   // Number of wanted matches
   int nbrWanted = searchItemEndingIndex - searchItemStartingIndex + 1;
   
   int searchItemStartingIndexToUse =
      req.translateMatchIdx( searchItemStartingIndex );
   int searchItemEndingIndexToUse =
      req.translateMatchIdx( searchItemStartingIndex + nbrWanted );
   
   int nbrItemMatches = searchItemEndingIndexToUse -
      searchItemStartingIndexToUse;

   // And update the ending index
   searchItemEndingIndex = searchItemStartingIndex + nbrItemMatches - 1;
         
   // numberitems
   addAttrib( listNode, "numberitems", nbrItemMatches );
   addAttrib( listNode, "ending_index", searchItemEndingIndex );
   addAttrib( listNode, "starting_index", searchItemStartingIndex );
   addAttrib( listNode, "total_numberitems", req.getTotalNbrMatches() );

   for ( uint32 i = 0; i < req.getNbrOverviewMatches(); ++i ) {
      OverviewMatch* match = req.getOverviewMatch( i );
      appendSearchArea( listNode, reply,
                        match,
                        indentLevel, // indent level
                        indent, // indent
                        latlonForSearchHits, // show lat/lon
                        0, 0, // lat, lon
                        NULL, // bounding box
                        positionSystem,
                        params );
      
   }
   return listNode;
}

void 
appendSearchArea( DOMNode* list,
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
                  bool showLatLon,
                  int32 lat, int32 lon,
                  MC2BoundingBox* bbox,
                  XMLCommonEntities::coordinateType
                  positionSystem,
                  const SearchRequestParameters& params ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   DOMElement* matchNode = reply->createElement( X( "search_area" ) );
   matchNode->setAttribute( 
                           X( "search_area_type" ), 
                           X( StringConversion::
                              searchLocationTypeToString( type ) ) );

   // Name
   DOMElement* nameNode = reply->createElement( X( "name" ) );
   XMLCh* nameToAppend =
      XMLUtility::transcodetoucs(name);
   nameNode->appendChild( reply->createTextNode( nameToAppend ) );
   delete [] nameToAppend;
   
   // areaid
   char ctmp[65];

   MC2String a_str_id ( match->matchToString() );

   DOMElement* areaidNode = reply->createElement( X( "areaid" ) );
   areaidNode->appendChild( reply->createTextNode( X( a_str_id.c_str() ) ) );

   // location_name
   DOMElement* locationNameNode = NULL;
   if ( locationName != NULL && locationName[ 0 ] != '\0' &&
        strcmp( locationName, "MISSING" ) != 0 )
      {
         locationNameNode = reply->createElement( X( "location_name" ) );
         XMLCh* locNameToAppend =
            XMLUtility::transcodetoucs(locationName);
         locationNameNode->appendChild( reply->
                                        createTextNode( locNameToAppend ) );
         delete [] locNameToAppend;
      }

   // boundingbox handled below
      
   // Append to matchNode
   int childIndentLevel = indentLevel + 1;
   MC2String childIndent = indentStr;
   childIndent.append( 3, ' ' );
   XStr XchildIndent( childIndent.c_str() );
      
   if ( indent ) {
      matchNode->appendChild( reply->
                              createTextNode( XchildIndent.XMLStr() ) );
   }
   matchNode->appendChild( nameNode );
   if ( indent ) {
      matchNode->appendChild( reply->
                              createTextNode( XchildIndent.XMLStr() ) );
   }
   matchNode->appendChild( areaidNode );
   if ( locationNameNode != NULL ) {
      if ( indent ) {
         matchNode->appendChild( reply->
                                 createTextNode( XchildIndent.XMLStr() ) );
      }
      matchNode->appendChild( locationNameNode );
   }

   using namespace XMLServerUtility;

   if ( showLatLon ) {
      // Lat
      appendElementWithText( 
                            matchNode, reply, "lat", 
                            XMLCommonEntities::
                            coordinateToString( ctmp, lat,  
                                                positionSystem, true ),
                            childIndentLevel, indent );
      // Lon
      appendElementWithText( matchNode, reply, "lon", 
                             XMLCommonEntities::
                             coordinateToString( ctmp, lon,  
                                                 positionSystem, false ), 
                            childIndentLevel, indent );
   }
   if ( bbox != NULL && bbox->isValid() ) {
      appendBoundingbox( matchNode, reply, positionSystem,
                         bbox->getMaxLat(), bbox->getMinLon(),
                         bbox->getMinLat(), bbox->getMaxLon(),
                         childIndentLevel, indent );
      
   }

   // Add top region id of the search match is a vanilla country match.
   if ( params.shouldIncludeTopRegionInArea() &&
        match->getType() & SEARCH_COUNTRIES ) {

      const VanillaCountryMatch* country =
         dynamic_cast< const VanillaCountryMatch* >( match );

      if ( country &&
           country->getTopRegionID() != MAX_UINT32 ) {
         if ( indent ) {
            matchNode->appendChild( reply->
                                    createTextNode( XchildIndent.XMLStr() ) );
         }
         XMLTool::addNode( matchNode, "top_region_id",
                           country->getTopRegionID() );
      }
   }

   // In SearchArea
   for ( uint32 j = 0 ; j < regions.size() ; ++j ) {
      if ( regions[ j ]->getType() & params.getRegionsInOverviewMatches() )
         {
            appendSearchArea( matchNode, reply, regions[ j ], 
                              indentLevel + 1, indent, false, MAX_INT32, 
                              MAX_INT32, NULL, XMLCommonEntities::MC2, 
                              params );
         }
   }

   // Add matchNode to list
   if ( indent ) {
      list->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   list->appendChild( matchNode );
   if ( indent ) {
      matchNode->appendChild( reply->createTextNode( 
                                                    XindentStr.XMLStr() ) );
   }
}
      

DOMElement*
appendSearchItem( DOMNode* list, 
                  DOMDocument* reply,
                  const VanillaMatch* match,
                  const char* currentLocation,
                  int indentLevel,
                  bool indent,
                  bool latlonForSearchHits,
                  int32 lat,
                  int32 lon,
                  bool positionSearchMatch,
                  MC2BoundingBox* bbox,
                  XMLCommonEntities::coordinateType positionSystem,
                  const SearchRequestParameters& params,
                  bool usePersistentIds,
                  XMLParserThread* thread,
                  bool addImageName,
                  bool includeInfoItem ) {

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   
   DOMElement* matchNode = reply->createElement( X( "search_item" ) );
   matchNode->setAttribute( 
                           X( "search_item_type" ),
                           X( StringConversion::
                              searchCategoryTypeToString( match->getType() ) ) );

   // image
   if ( addImageName && thread != NULL ) {
      uint32 currentHeading = thread->getSearchHandler().getHeadingForMatch( 
         match );
      MC2String imageName = thread->getSearchHandler().
         getImageName( *match, currentHeading );
      if ( ! imageName.empty() ) {
         // add image name
         XMLTool::addAttrib( matchNode, "image", imageName );
      }
   }

   // Name
   DOMElement* nameNode = reply->createElement( X( "name" ) );
   MC2String name( match->getName() );
   XMLTool::setNodeValue( nameNode, name );
      
   // itemid
   MC2String str_id = match->matchToString();
   if ( usePersistentIds && thread != NULL ) {
      str_id = PersistentSearchIDs::makeID( *match, 
                                            thread->getSearchHandler() );
   }
   DOMElement* itemidNode = reply->createElement( X( "itemid" ) );
   itemidNode->appendChild( reply->createTextNode( X( str_id.c_str() ) ) );

   // explicit_itemid
   DOMElement* explicitItemIDNode = NULL;
   if ( latlonForSearchHits ) {
      explicitItemIDNode = reply->createElement( X( "explicit_itemid" ) );
      char latstr[30];
      char lonstr[30];
      if ( lat > 0 ) {
         sprintf( latstr, "%.8X", lat );
      } else {
         latstr[ 0 ] = '-';
         sprintf( latstr + 1, "%.8X", -lat );
      }
      if ( lon > 0 ) {
         sprintf( lonstr, "%.8X", lon );
      } else {
         lonstr[ 0 ] = '-';
         sprintf( lonstr + 1, "%.8X", -lon );
      }
      char ctmp[1024];
      sprintf( ctmp, "%s%s", latstr, lonstr );
      explicitItemIDNode->appendChild( reply->createTextNode( X( ctmp ) ) );
   }

   // location_name
   DOMElement* locationNameNode = NULL;
   const char* location = match->getLocationName();
   if ( location != NULL && location[ 0 ] != '\0' &&
        strcmp( location, "MISSING" ) != 0 )
      {
         locationNameNode = reply->createElement( X( "location_name" ) );
         XMLCh* locNameToAppend =
            XMLUtility::transcodetoucs(location);
         locationNameNode->appendChild( reply->
                                        createTextNode( locNameToAppend ) );
         delete [] locNameToAppend;      
      }
   
   // lat
   DOMElement* latNode = NULL;
   char ctmp[1024];
   if ( positionSearchMatch ) {
      latNode = reply->createElement( X( "lat" ) );
      latNode->
         appendChild( reply->
                      createTextNode( X( XMLCommonEntities::
                                         coordinateToString( ctmp, lat, 
                                                             positionSystem, 
                                                             true ) ) ) );
   }

   // lon
   DOMElement* lonNode = NULL;
   if ( positionSearchMatch ) {
      lonNode = reply->createElement( X( "lon" ) );
      lonNode->
         appendChild( reply->
                      createTextNode( X( XMLCommonEntities::
                                         coordinateToString( ctmp, lon, 
                                                             positionSystem, 
                                                             false ) ) ) );
   }

   // Boundingbox handled below

      
   // Append to matchNode
   int childIndentLevel = indentLevel + 1;
   MC2String childIndent = indentStr;
   childIndent.append( 3, ' ' );
   XStr XchildIndent( childIndent.c_str() );

   if ( indent ) {
      matchNode->appendChild( reply->createTextNode( XchildIndent.XMLStr() ) );
   }
   matchNode->appendChild( nameNode );
   if ( indent ) {
      matchNode->appendChild( reply->createTextNode( XchildIndent.XMLStr() ) );
   }
   matchNode->appendChild( itemidNode );

   if ( explicitItemIDNode != NULL ) {
      if ( indent ) {
         matchNode->appendChild( reply->
                                 createTextNode( XchildIndent.XMLStr() ) );
      }
      matchNode->appendChild( explicitItemIDNode );
   }
   if ( locationNameNode != NULL ) {
      if ( indent ) {
         matchNode->appendChild( reply->
                                 createTextNode( XchildIndent.XMLStr() ) );
      }
      matchNode->appendChild( locationNameNode );
   }
   if ( latNode != NULL ) {
      if ( indent ) {
         matchNode->appendChild( reply->createTextNode( 
                                                       XchildIndent.XMLStr() ) );
      }
      matchNode->appendChild( latNode ); 
   }
   if ( lonNode != NULL ) {
      if ( indent ) {
         matchNode->appendChild( reply->
                                 createTextNode( XchildIndent.XMLStr() ) );
      }
      matchNode->appendChild( lonNode ); 
   }

   if ( bbox != NULL && bbox->isValid() ) {
      appendBoundingbox( matchNode, reply, positionSystem,
                         bbox->getMaxLat(), bbox->getMinLon(),
                         bbox->getMinLat(), bbox->getMaxLon(),
                         childIndentLevel, indent );
   }

   if ( params.shouldIncludeCategoriesInResult() &&
        match->getType() == SEARCH_COMPANIES ) {
      const VanillaCompanyMatch& poi =
         static_cast< const VanillaCompanyMatch& >( *match );
      if ( ! poi.getCategories().empty() ) {
         // Search match has uint32 as category id...
         // TODO: change search match category ids.
         CompactSearch::CategoryIDs ids( poi.getCategories().begin(),
                                         poi.getCategories().end() );
         CategoryListNode::addCategoryList( matchNode, ids );
      }
   }

   // In SearchArea
   for ( uint32 j = 0 ; j < match->getNbrRegions() ; ++j ) {
      if ( match->getRegion( j )->getType() & 
           params.getRegionsInMatches() )
         {
            appendSearchArea( matchNode, reply, match->getRegion( j ), 
                              indentLevel + 1, indent, false, MAX_INT32, 
                              MAX_INT32, NULL, XMLCommonEntities::MC2, 
                              params );
         }
   }

   // Include info_item
   if ( includeInfoItem ) {
      XMLItemInfoUtility::appendInfoItem( 
         matchNode, reply, match, 
         positionSystem,
         params.shouldIncludeCategoriesInResult(),
         indentLevel + 1, indent, thread );
   }

   // Add matchNode to list
   if ( indent ) {
      list->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   list->appendChild( matchNode );
   if ( indent ) {
      matchNode->appendChild( reply->
                              createTextNode( XindentStr.XMLStr() ) );
   }

   return matchNode;
}


DOMElement*
appendSearchItemList( DOMNode* curr, DOMDocument* reply,
                      int indentLevel, bool indent,
                      const SearchResultRequest& req,
                      const char* currentLocation,
                      int32 searchItemStartingIndex,
                      int32 searchItemEndingIndex,
                      bool latlonForSearchHits,
                      bool positionSearchItems,
                      XMLCommonEntities::coordinateType positionSystem,
                      const SearchRequestParameters& params,
                      DOMElement* optionalSearchItem,
                      XMLParserThread* thread,
                      uint32 currentHeading,
                      bool usePersistentIds,
                      bool includeInfoItem ) {

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   DOMElement* search_item_list;
   if ( optionalSearchItem ) {
      search_item_list = optionalSearchItem;
   } else {
      search_item_list = reply->createElement( X( "search_item_list" ) );
      curr->appendChild( search_item_list );
   }

   // Update the indeces

   // Number of wanted matches
   int nbrWanted = searchItemEndingIndex - searchItemStartingIndex + 1;
   
   int searchItemStartingIndexToUse =
      req.translateMatchIdx( searchItemStartingIndex );
   int searchItemEndingIndexToUse =
      req.translateMatchIdx( searchItemStartingIndex + nbrWanted );
   
   int nbrItemMatches = searchItemEndingIndexToUse -
      searchItemStartingIndexToUse;

   // And update the ending index
   searchItemEndingIndex = searchItemStartingIndex + nbrItemMatches - 1;
   uint32 totalNbrMatches = req.getTotalNbrMatches();
   if ( req.getMatches().empty() ) {
      nbrItemMatches = searchItemEndingIndex = 0;
      searchItemStartingIndex = -1;
      totalNbrMatches = 0;
   }
   // numberitems
   search_item_list->setAttribute( X( "numberitems" ), 
                                   XInt32( nbrItemMatches ) );
   // ending_index
   search_item_list->setAttribute( X( "ending_index" ), 
                                   XInt32( searchItemEndingIndex ) );
   // starting_index
   search_item_list->setAttribute( X( "starting_index" ), 
                                   XInt32( searchItemStartingIndex ) );
   // total_numberitems
   search_item_list->setAttribute( X( "total_numberitems" ), 
                                   XInt32( totalNbrMatches ) );
   if ( indent ) {
      curr->appendChild( 
                        reply->createTextNode( XindentStr.XMLStr() ) );
   }

   if ( req.getMatches().empty() ) {
      return search_item_list;
   }

   bool showLatLon = (latlonForSearchHits || positionSearchItems);

   const vector<VanillaMatch*>& matches = req.getMatches();
   uint32 nbrAdverts = 0;
   for ( int32 i = searchItemStartingIndexToUse ; 
         i < searchItemEndingIndexToUse ; i++ ) {

      const VanillaMatch* match = matches[ i ];
      MC2BoundingBox bbox( match->getBBox() );
      // Print search_item
      DOMElement* element = 
         appendSearchItem( search_item_list, reply, match,
                           currentLocation,
                           indentLevel + 1, indent,
                           latlonForSearchHits ? 
                           match->getCoords().isValid() : false,
                           showLatLon ? match->getCoords().lat : 0,
                           showLatLon ? match->getCoords().lon : 0,
                           positionSearchItems ? 
                           match->getCoords().isValid() : false,
                           &bbox,
                           positionSystem, params, usePersistentIds, 
                           thread, true/*addImageName*/,
                           includeInfoItem );

      // If we do not have search handler, then do not
      // add any more attributes.
      if ( thread == NULL ) {
         continue;
      }

      // if source is advertisement server, then mark it
      if ( match->getExtSource() == ExternalSearchConsts::adserver &&
           nbrAdverts < 25 ) {
         XMLTool::addAttrib( element, "advert", true );
         ++nbrAdverts;
      }
   }
            
   if ( nbrItemMatches > 0 ) {
      if ( indent ) {
         // Add newline and indent before search_item_list
         // endtag
         search_item_list->
            appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
      }
   }
   return search_item_list;
}

void appendBoundingbox( DOMNode* curr, 
                        DOMNode* reply,
                        XMLCommonEntities::coordinateType format,
                        int32 northLat,
                        int32 westLon,
                        int32 southLat,
                        int32 eastLon,
                        int indentLevel,
                        bool indent ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   char target[128];

   DOMDocument* doc = XMLTool::getOwner( reply );

   DOMElement* boundingboxNode = doc->createElement( X( "boundingbox" ) );
   boundingboxNode->setAttribute( X( "north_lat" ),
                                  X( XMLCommonEntities::
                                     coordinateToString( target, northLat, 
                                                         format, true ) ) );
   boundingboxNode->setAttribute( X( "west_lon" ),
                                  X( XMLCommonEntities::
                                     coordinateToString( target, westLon, 
                                                         format, false ) ) );
   boundingboxNode->setAttribute( X( "south_lat" ),
                                  X( XMLCommonEntities::
                                     coordinateToString( target, southLat, 
                                                         format, true ) ) );
   boundingboxNode->setAttribute( X( "east_lon" ),
                                  X( XMLCommonEntities::
                                     coordinateToString( target, eastLon, 
                                                         format, false ) ) );
   boundingboxNode->setAttribute( X( "position_sytem" ),
                                  X( XMLCommonEntities::
                                     coordinateFormatToString( format ) ) );

   // Add boundingboxNode to curr
   if ( indent ) {
      curr->appendChild( doc->createTextNode( X( indentStr.c_str() ) ) );
   }
   curr->appendChild( boundingboxNode );
}


void 
appendPositionItem( int32 lat, int32 lon, uint16 angle,
                    XMLCommonEntities::coordinateType positionSystem,
                    DOMNode* cur, DOMDocument* reply,
                    int indentLevel, bool indent ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   char target[512];

   DOMElement* positionItem = reply->createElement( X( "position_item" ) );
   positionItem->setAttribute( 
      X( "position_system" ),
      X( XMLCommonEntities::coordinateFormatToString( positionSystem ) ) );

   using XMLServerUtility::appendElementWithText;

   // Lat
   appendElementWithText( positionItem, reply, "lat", 
                          XMLCommonEntities::coordinateToString( 
                             target, lat, positionSystem, true ),
                          indentLevel+1, indent );
   // Lon
   appendElementWithText( positionItem, reply, "lon", 
                          XMLCommonEntities::coordinateToString( 
                             target, lon, positionSystem, false ),
                          indentLevel+1, indent );
   // Angle?
   if ( angle <= 360 ) {
      MC2String angleStr;
      STLStringUtility::uint2str( angle, angleStr );
      appendElementWithText( positionItem, reply, "angle", 
                             angleStr.c_str(),
                             indentLevel+1, indent );
   }

   // Add positionItem to cur
   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   cur->appendChild( positionItem );
   if ( indent ) {
      positionItem->appendChild( reply->createTextNode( 
                                    XindentStr.XMLStr() ) );
   }
}




VanillaMatch* 
makeVanillaMatchFromData( uint32 mapID, uint32 itemID,
                          uint16 offset,
                          const char* name,
                          const char* locationName,
                          uint32 streetNbr, 
                          uint32 itemType ) {
   VanillaMatch* match = NULL;
   VanillaMatch* standaloneMatch = NULL;

   // I don't think the standalone matches will be needed anymore.
   
   if ( itemType == SEARCH_STREETS ) {
      match = new VanillaStreetMatch( IDPair_t(mapID, itemID),
                                      name, locationName,
                                      offset, streetNbr);
      standaloneMatch = new VanillaStreetMatch( 
         *static_cast<VanillaStreetMatch*>( match ) );
   } else if ( itemType == SEARCH_COMPANIES ) {
      match = new VanillaCompanyMatch( IDPair_t(mapID, itemID),
                                       name, locationName,
                                       offset, streetNbr);
      standaloneMatch = new VanillaCompanyMatch( 
         *static_cast<VanillaCompanyMatch*>( match ) );
   } else if ( itemType == SEARCH_CATEGORIES ) {
      match = new VanillaCategoryMatch( IDPair_t(mapID, itemID),
                                        name, locationName);
      standaloneMatch = new VanillaCategoryMatch( 
         *static_cast<VanillaCategoryMatch*>( match ) );
   } else if ( itemType == SEARCH_MISC ) {
      match = new VanillaMiscMatch( IDPair_t(mapID, itemID),
                                    name, locationName);
      standaloneMatch = new VanillaMiscMatch( 
         *static_cast<VanillaMiscMatch*>( match ) );
   } else {
      mc2log << warn << "XMLParserThread::makeVanillaMatchFromData "
                "unknown itemType " << itemType << endl
             << "   name " << name << endl;
   }
   delete match;   

   return standaloneMatch;
}


bool 
getSearchItemData( const DOMNode * searchItem,
                   uint32& mapID, uint32& itemID,
                   uint16& offset, char*& name,
                   uint32& streetNbr,
                   uint32& itemType,
                   MC2String& errorCode,
                   MC2String& errorMessage ) {
   if ( XMLString::equals( searchItem->getNodeName(), "search_item" ) ) {
      bool ok = true;
      // Get attributes
      const DOMNamedNodeMap* attributes = searchItem->getAttributes();
      const DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
         
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(),
                                 "search_item_type" ) ) 
         {
            itemType = StringConversion::searchCategoryTypeFromString( 
               tmpStr );
         } else {
            mc2log << warn << "XMLParserThread::getsearchItemData "
                   << "unknown attribute " << "Name " 
                   << attribute->getNodeName() << "Type " 
                   << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Get children
      const DOMNode* child = searchItem->getFirstChild();
   
      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), "name" ) ) {
                  name = XMLUtility::getChildTextValue( child );
               } else if ( XMLString::equals( child->getNodeName(),
                                              "itemid" ) ) 
               {
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  if ( sscanf( tmpStr, "%X;%X;%hX", 
                               &mapID, &itemID, &offset ) != 3 )
                  {
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Bad itemid for search_item.";
                     mc2log << warn 
                            << "XMLParserThread::getsearchItemData "
                            << "Bad search_item itemid \""
                            << tmpStr << "\"" << endl;
                  }
                  delete [] tmpStr;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "streetnbr" ) ) 
               {
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  streetNbr = strtol( tmpStr, NULL, 10 );
                  delete [] tmpStr;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "explicit_itemid" ) )
               {
                  // Not used right now (Depricated use lat, lon)
               } else if ( XMLString::equals( child->getNodeName(),
                                              "location_name" ) )
               {
                  // Not used right now but could be usefull
               } else if ( XMLString::equals( child->getNodeName(),
                                              "lat" ) ) 
               {
                  // Add coordinate to searchmatches first
               } else if ( XMLString::equals( child->getNodeName(),
                                              "lon" ) ) 
               {
                  // Add coordinate to searchmatches first
               } else if ( XMLString::equals( child->getNodeName(),
                                              "boundingbox" ) ) 
               {
                  // Add boundingbox to searchmatches first
               } else {
                  mc2log << warn << "XMLParserThread::getsearchItemData "
                         << "odd Element in search_item element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::getsearchItemData odd "
                      << "node type in search_item element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType()<< endl;
               break;
         }
         child = child->getNextSibling();
      }
      return ok;
   } else {
      mc2log << warn << "XMLParserThread::getSearchItemData not "
             << "search_item" << endl;
      errorCode = "-1";
      errorMessage = "search_item is not a search_item.";
      return false;
   }  
}

bool 
getSearchAreaData( DOMNode* searchArea,
                   uint32& mapID, uint32& itemID,
                   uint32& maskID, char*& name,
                   uint32& areaType,
                   MC2String& errorCode,
                   MC2String& errorMessage ) {
   bool ok = true;

   if ( XMLString::equals( searchArea->getNodeName(), "search_area" ) ) {
      // Get attributes
      DOMNamedNodeMap* attributes = searchArea->getAttributes();
      DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
         
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(),
                                 "search_area_type" ) ) 
         {
            areaType = StringConversion::searchLocationTypeFromString( 
               tmpStr );
         } else {
            mc2log << warn << "XMLParserThread::getSearchAreaData "
                   << "unknown attribute "
                   << "Name " << attribute->getNodeName()
                   << "Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Get children
      DOMNode* child = searchArea->getFirstChild();
   
      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), "name" ) ) {
                  name = XMLUtility::getChildTextValue( child );
               } else if ( XMLString::equals( child->getNodeName(),
                                              "areaid" ) ) 
               {                  
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  auto_ptr<SearchMatch> match (
                     SearchMatch::createMatch( tmpStr ) );
                  if ( match.get() != NULL ) {
                     ok = true;
                     mapID = match->getMapID();
                     itemID = match->getItemID();
                     maskID = 0xffffffff;
                  } else if ( sscanf( tmpStr, "%X;%X;%X", 
                               &mapID, &itemID, &maskID ) != 3 ) {
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "Bad areaid in search_area.";
                     mc2log << warn 
                            << "XMLParserThread::getSearchAreaData "
                               "Bad search_area areaid " << tmpStr << endl;
                  }
                  delete [] tmpStr;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "location_name" ) )
               {
                  // Add location name to match
               } else if ( XMLString::equals( child->getNodeName(),
                                              "lat" ) ) 
               {
                  // Add lat to SearchMatch
               } else if ( XMLString::equals( child->getNodeName(),
                                              "lon" ) ) 
               {
                  // Add lat to SearchMatch
               } else if ( XMLString::equals( child->getNodeName(),
                                              "boundingbox" ) ) 
               {
                  // Add boundingbox to SearchMatch first
               } else if ( XMLString::equals( child->getNodeName(),
                                              "search_area" ) ) 
               {
                  // Add SearchRegion to SearchMatch
               } else {
                  mc2log << warn << "XMLParserThread::getSearchAreaData "
                         << "odd Element in search_area element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            case DOMNode::TEXT_NODE :
               // Ignore stray texts
               break;
            default:
               mc2log << warn << "XMLParserThread::getSearchAreaData odd "
                      << "node type in search_area element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType()<< endl;
               break;
         }
         child = child->getNextSibling();
      }

   } else {
      ok = false;
      errorCode = "-1";
      errorMessage = "search_area not a search_area.";
   }

   return ok;
}

VanillaMatch* 
getVanillaMatchFromSearchItem( const DOMNode* searchItem,
                               MC2String& errorCode, MC2String& errorMessage ) {
   SearchMatch* match = NULL;
   const DOMNodeList* ids = static_cast<const DOMElement*>(
      searchItem )->getElementsByTagName( X( "itemid" ) );

   if ( ids->getLength() == 1 ) {
      match = getVanillaMatchFromItemId( ids->item( 0 ) );
   }
   
   if ( match != NULL ) {
      return static_cast< VanillaMatch* > ( match );
   } else {
      uint32 mapID = 0;
      uint32 itemID = 0;
      uint16 offset = 0;
      char* name = NULL;
      uint32 streetNbr = 0;
      uint32 itemType = 0;

      if ( getSearchItemData( searchItem, mapID, itemID, 
                              offset, name, streetNbr, itemType,
                              errorCode, errorMessage ) ) {
         VanillaMatch* match = 
            makeVanillaMatchFromData( mapID, itemID, 
                                      offset, name, "", 
                                      streetNbr, itemType );
         delete [] name;

         mc2dbg8 << " XMLParserThread::getVanillaMatchFromSearchItem "
                 << "return " << match << endl;
         return match;
      } else {
         mc2dbg8 << " XMLParserThread::getVanillaMatchFromSearchItem "
                 << "getSearchItemData returned false" << endl;
         delete [] name;
         return NULL;
      }
   }
}

SearchMatch* 
getVanillaMatchFromItemId( const DOMNode* itemId ) {
   SearchMatch* match = NULL;
   if ( XMLString::equals( itemId->getNodeName(), "itemid" ) ) {
      MC2String itemIDStr = XMLUtility::getChildTextStr( *itemId );
      if ( PersistentSearchIDs::isPersistentID( itemIDStr ) ) {
         return NULL;
      }
      match = SearchMatch::createMatch( itemIDStr );
   } else {
      mc2log << "[XMLSearchUtility]::getVanillaMatchFromItemId No "
         "Itemid element." << endl;
   }
   
   return static_cast< VanillaMatch* > ( match );
}

uint32 itemTypeToSearchItemType( ItemTypes::itemType type ) {
   switch( type ) {
      case ItemTypes::streetSegmentItem :
      case ItemTypes::streetItem :
         return SEARCH_STREETS;
      break;

      case ItemTypes::pointOfInterestItem :
         return SEARCH_COMPANIES;
         break;

      case ItemTypes::categoryItem :
         return SEARCH_CATEGORIES;
         break;

      default:
         return SEARCH_MISC;
         break;
   }

   // Unreachable code, just to make compiler happy
   return SEARCH_STREETS;
}


void appendSearchMatchElement( DOMNode* parentNode, 
                               DOMDocument* replyDoc,
                               VanillaMatch* match,
                               XMLCommonEntities::coordinateType positionSystem,
                               LangTypes::language_t language,
                               XMLParserThread* thread) {
   
   // Create the search_match element
   DOMElement* matchElement = replyDoc->createElement( X( "search_match" ) );

   // Add the attributes to the search_match element
   XMLTool::addAttrib( matchElement, "search_match_type",
                       MC2String( StringConversion::
                       searchCategoryTypeToString( match->getType() ) ) ) ;
   if ( thread != NULL ) {
      XMLTool::addAttrib( matchElement, "category_image",
                          (thread->getSearchHandler()).getCategoryImageName( *match ) );
      XMLTool::addAttrib( matchElement, "provider_image",
       thread->getSearchHandler().getProviderImageName( *match ) );
      XMLTool::addAttrib( matchElement, "brand_image",
       thread->getSearchHandler().getBrandImageName( *match ) );
   }
   XMLTool::addAttrib(matchElement, "additional_info_exists", 
                      match->getAdditionalInfoExists() );
   

   // Create the name element and set value
   DOMElement* nameElement = replyDoc->createElement( X( "name" ) );
   MC2String name( match->getName() );
   if ( match->getType() == SEARCH_COMPANIES ) {
      const VanillaCompanyMatch& poi =
         static_cast< const VanillaCompanyMatch& >( *match );
      if ( ! poi.getCleanCompanyName().empty() ) {
         name = poi.getCleanCompanyName();
      }
   }
   XMLTool::setNodeValue( nameElement, name );
   matchElement->appendChild( nameElement );

   // Create the itemid element
   DOMElement* itemIdElement = replyDoc->createElement( X( "itemid" ) );
   XMLTool::setNodeValue( itemIdElement, match->matchToString() );
   matchElement->appendChild( itemIdElement );

   // Create the location_name element
   const char* location = match->getLocationName();
   if ( location == NULL || strcmp( location, "MISSING" ) == 0 ) {
      location = "";
   }
   using XMLServerUtility::appendElementWithText;
   appendElementWithText( 
      matchElement, replyDoc, "location_name", location, 0, false/*indent*/ );

   // Create the lat element
   char ctmp[1024];
   DOMElement* latElement = replyDoc->createElement( X( "lat" ) );
   XMLTool::setNodeValue( latElement, MC2String( XMLCommonEntities::
                          coordinateToString( ctmp, 
                                              match->getCoords().lat,
                                              positionSystem, true ) ) ) ;
  matchElement->appendChild( latElement );
   
   // Create the lon element
   DOMElement* lonElement = replyDoc->createElement( X( "lon" ) );
   XMLTool::setNodeValue( lonElement, MC2String( XMLCommonEntities::
                          coordinateToString( ctmp, 
                                              match->getCoords().lon,
                                              positionSystem, false ) ) ) ;
   
   matchElement->appendChild( lonElement );

   // Create the category_list element
   if ( match->getType() == SEARCH_COMPANIES ) {
      const VanillaCompanyMatch& poi =
         static_cast< const VanillaCompanyMatch& >( *match );
      if ( ! poi.getCategories().empty() ) {
         CompactSearch::CategoryIDs ids( poi.getCategories().begin(),
                                         poi.getCategories().end() );
         CategoryListNode::addCategoryList( matchElement, ids );
      }
   }
   
   // Create the search_area element
   SearchRequestParameters params;
   for ( uint32 j = 0 ; j < match->getNbrRegions() ; ++j ) {
      if ( match->getRegion( j )->getType() ) {
         XMLSearchUtility::appendSearchArea( matchElement,
                                             replyDoc, 
                                             match->getRegion( j ),
                                             0, 
                                             false );
      }
   }

   // Create the info_field nodes
   XMLItemDetailUtility::appendDetailItem( matchElement, match );

   parentNode->appendChild( matchElement );
}



void appendSearchListElement( DOMNode* parentNode, 
                              DOMDocument* replyDoc,
                              const SearchResultRequest* req,
                              uint32 maxHits,
                              XMLCommonEntities::coordinateType positionSystem,
                              LangTypes::language_t language,
                              XMLParserThread* thread) {
   DOMElement *searchListElement = replyDoc->createElement( X( "search_list" ) );
   parentNode->appendChild( searchListElement );

   const vector<VanillaMatch*>& matches = req->getMatches();

   uint32 numberMatches = MIN( maxHits, matches.size() );

   XMLTool::addAttrib( searchListElement, "number_matches", 
                       numberMatches );
   XMLTool::addAttrib( searchListElement, "total_number_matches", 
                       req->getTotalNbrMatches());
   
    for (uint32 i = 0 ; i < numberMatches ; ++i ) {
      appendSearchMatchElement( searchListElement, 
                                replyDoc, 
                                matches[i],
                                positionSystem, 
                                language,
                                thread);
   }
}

}

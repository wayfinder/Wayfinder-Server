/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLItemInfoUtility.h"
#include "XMLUtility.h"
#include "XMLTool.h"
#include "ItemInfoPacket.h"
#include "SearchMatch.h"
#include "XMLCategoryListNode.h"
#include "XMLParserThread.h"
#include "InfoTypeConverter.h"

namespace XMLItemInfoUtility {

using namespace XMLTool;

void extractItemInfoData( const SearchMatch* match, 
                          ItemInfoData& infoData) {
   // Get the information data from the match
   infoData = ItemInfoData( match->getSynonymName(),
                            match->getName(),  
                            match->getItemType(),
                            0/*itemSubType*/,
                            match->getCoords() );
   if ( match->getType() == SEARCH_COMPANIES ) {
      const VanillaCompanyMatch& poi =
         static_cast< const VanillaCompanyMatch& >( *match );
      infoData.setSubType( poi.getItemSubtype() );
      infoData.setCategories( poi.getCategories() );
      if ( ! poi.getCleanCompanyName().empty() ) {
         infoData.setItemName( poi.getCleanCompanyName() );
      }
   }
   infoData.setFields( match->getItemInfos() );
}


void addInfoField( 
   ItemInfoEnums::InfoType type,
   const MC2String key,
   const MC2String val,
   DOMElement* infoItemNode,
   XMLParserThread* thread ) {
   // info_field
   DOMElement* infoFieldNode = addNode( infoItemNode, "info_field" );
   // info_type
   addAttrib( infoFieldNode, "info_type", 
              MC2String( thread->getInfoTypeConverter().
                         infoTypeToStr( type ) ) );
   // fieldName
   addNode( infoFieldNode, "fieldName", key );
   // fieldValue
   addNode( infoFieldNode, "fieldValue", val );
}

void addInfoField( ItemInfoData::ItemInfoEntryCont::const_iterator it,
                   DOMElement* infoItemNode,
                   XMLParserThread* thread ) {
   addInfoField( it->getInfoType(), it->getKey(), it->getVal(), 
                 infoItemNode, thread);
}

void addInfoFields( DOMElement* currentElement, 
                    const VanillaMatch* match,
                    XMLParserThread* thread ) {
   ItemInfoData data;

   extractItemInfoData( match, data );
   
   ItemInfoData::ItemInfoEntryCont::const_iterator it;
   ItemInfoData::ItemInfoEntryCont::const_iterator endIt = 
      data.getFields().end();
   for ( it = data.getFields().begin() ; it != endIt ; ++it ) {
      addInfoField( it, currentElement, thread );
   }
}

DOMElement* appendInfoItem( DOMNode* cur, DOMDocument* reply,
                            const ItemInfoData& data,
                            XMLCommonEntities::coordinateType format,
                            bool includeCategoryID,
                            int indentLevel, bool indent,
                            XMLParserThread* thread ) {
   DOMElement* infoItemNode = addNode( cur, "info_item" );

   // numberfields
   addAttrib( infoItemNode, "numberfields", data.getFields().size() );
   // typeName
   MC2String type( data.getType() );
   if ( data.getType().empty() ) {
      // Fake it
      type= StringTable::getString(ItemTypes::getPOIStringCode( 
                                      ItemTypes::invalidPOIType ),
                                   StringTable::ENGLISH );
   }
   addNode( infoItemNode, "typeName", type );
   // itemName
   addNode( infoItemNode, "itemName", data.getItemName() );
   if ( data.getCoord().isValid() ) {
      char tmpStr[ 128 ];
      // lat
      XMLCommonEntities::coordinateToString( 
         tmpStr, data.getCoord().lat, format, true );
      addNode( infoItemNode, "lat", MC2String( tmpStr ) );
      // lon
      XMLCommonEntities::coordinateToString( 
         tmpStr, data.getCoord().lon, format, true );
      addNode( infoItemNode, "lon", MC2String( tmpStr ) );
   }
   // Append category list of requested.
   if ( includeCategoryID ) {
      if ( ! data.getCategories().empty() ) {
         CategoryListNode::addCategoryList( infoItemNode,
                                            data.getCategories() );
      }
   }
   // info_field*
   ItemInfoData::ItemInfoEntryCont::const_iterator longDesc = 
      data.getFields().end();
   bool haveShortDesc = false;
   ItemInfoData::ItemInfoEntryCont::const_iterator fullAddress = 
      data.getFields().end();
   bool haveVisAddress = false;
   for ( ItemInfoData::ItemInfoEntryCont::const_iterator it = 
            data.getFields().begin(), endIt = data.getFields().end() ; 
         it != endIt ; ++it ) {
      addInfoField( it, infoItemNode, thread );
      if ( it->getInfoType() == ItemInfoEnums::long_description ) {
         longDesc = it;
      } else if ( it->getInfoType() == ItemInfoEnums::short_description ) {
         haveShortDesc = true;
      } else if ( it->getInfoType() == ItemInfoEnums::vis_full_address ) {
         fullAddress = it;
      } else if ( it->getInfoType() == ItemInfoEnums::vis_address ) {
         haveVisAddress = true;
      }
   }

   if ( indent ) {
      XMLUtility::indentPiece( *infoItemNode, indentLevel );
   }

   return infoItemNode;
}


DOMElement* appendInfoItem( DOMNode* cur, DOMDocument* reply,
                            const VanillaMatch* match,
                            XMLCommonEntities::coordinateType format,
                            bool includeCategoryID,
                            int indentLevel, bool indent,
                            XMLParserThread* thread ) {
   ItemInfoData data;
   extractItemInfoData( match, data );
   
   // Make the node
   return appendInfoItem( cur, reply, data, format, includeCategoryID, 
                          indentLevel, indent, thread );
}

} // End namespace XMLItemInfoUtility


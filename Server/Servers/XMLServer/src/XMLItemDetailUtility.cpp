/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLItemDetailUtility.h"

#include "XMLItemInfoUtility.h"
#include "XMLUtility.h"
#include "XMLTool.h"
#include "ItemInfoPacket.h"
#include "SearchMatch.h"
#include "ItemDetailEnums.h"
#include "ItemInfoEntry.h"
#include "XMLItemInfoUtility.h"

namespace XMLItemDetailUtility {

using namespace XMLTool;

uint32 addDetailFields( DOMElement* detailItemNode, const SearchMatch* match );
void addDetailField( const ItemInfoEntry& data, DOMElement* detailItemNode );

DOMElement* 
appendDetailItem( DOMNode* rootNode, 
                  const SearchMatch* match ) {
   
   DOMElement* detailItemNode = addNode( rootNode, "detail_item" );
   
   uint32 nbrOfFields = addDetailFields( detailItemNode, match );

   // numberfields on detail_item
   addAttrib( detailItemNode, "numberfields", nbrOfFields );
   

   return detailItemNode;
}

uint32 
addDetailFields( DOMElement* detailItemNode, 
                 const SearchMatch* match ) {
   ItemInfoData data;
   
   XMLItemInfoUtility::extractItemInfoData( match, data );

   for ( ItemInfoData::ItemInfoEntryCont::const_iterator it = 
            data.getFields().begin(), endIt = data.getFields().end(); 
         it != endIt ; ++it ) {
      addDetailField( (*it), detailItemNode );
   }

   return data.getFields().size();
}


void 
addDetailField( const ItemInfoEntry& data,
                DOMElement* detailItemNode ) {
   // Element detail_field
   DOMElement* detailFieldNode = addNode( detailItemNode, 
                                          "detail_field" );
   // Attribute detail_type
   addAttrib( detailFieldNode, "detail_type", 
              ItemDetailEnums::poiDetailAsString(
                 ItemDetailEnums::poiInfoToPoiDetail( 
                    data.getInfoType() ) ) );

   // Attribute detail_content_type
   addAttrib( detailFieldNode, "detail_content_type", 
              ItemDetailEnums::poiContentTypeAsString(
                 ItemDetailEnums::getContentTypeForPoiInfo( 
                    data.getInfoType() ) ) );
   // Element fieldName
   addNode( detailFieldNode, "fieldName", MC2String( data.getKey() ) );
   // Element fieldValue
   addNode( detailFieldNode, "fieldValue", MC2String( data.getVal() ) );
   
}

} // End namespace XMLItemDetailUtility

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLTopRegionElement.h"

#include "XMLSearchUtility.h"
#include "XMLServerElements.h"
#include "XMLTool.h"

#include "TopRegionMatch.h"
#include "DataBuffer.h"
#include "MC2BoundingBoxBuffer.h"

namespace XMLTopRegionElement {

DOMElement* 
makeTopRegionElement( const TopRegionMatch* topRegion,
                      XMLCommonEntities::coordinateType format,
                      LangTypes::language_t language,
                      DOMNode* reply,
                      int indentLevel, bool indent,
                      DataBuffer* buf ) {
   char tmpStr[ 128 ];
   DOMDocument* doc = XMLTool::getOwner( reply );
   DOMElement* topRegionNode = doc->createElement( X( "top_region" ) );
   topRegionNode->setAttribute( 
      X( "top_region_type" ), X( XMLCommonEntities::topRegionTypeToString(
                                    topRegion->getType() ) ) );

   // top_region_id
   sprintf( tmpStr, "%u", topRegion->getID() );

   using namespace XMLServerUtility;

   appendElementWithText( topRegionNode, reply, "top_region_id", tmpStr,
                          indentLevel + 1, indent );
   
   // boundingbox
   const MC2BoundingBox& bbox = topRegion->getBoundingBox();
   XMLSearchUtility::
      appendBoundingbox( topRegionNode, reply, format, 
                         bbox.getMaxLat(), bbox.getMinLon(),
                         bbox.getMinLat(), bbox.getMaxLon(),
                         indentLevel + 1, indent );

   // name_node
   const Name* name = topRegion->getNames()->getBestName( language );
   char* languageStr = StringUtility::newStrDup(
      StringUtility::copyLower( MC2String(
      LangTypes::getLanguageAsString( name->getLanguage(), true))).c_str());
   
   attrVect attr; 
   attr.push_back ( stringStringStruct( "language", languageStr ) );
   appendElementWithText( topRegionNode, reply, "name_node", 
                          name->getName(),
                          indentLevel + 1, indent,
                          &attr );

   if ( buf ) {
      buf->writeNextLong( topRegion->getType() );
      buf->writeNextLong( topRegion->getID() );
      MC2BoundingBoxBuffer::writeNextBBox( *buf, bbox );
      buf->writeNextString( name->getName() );
      buf->alignToLongAndClear();
   }

   delete [] languageStr;

   return topRegionNode;
}

}

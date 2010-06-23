/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"

#ifdef USE_XML
#include "XMLTool.h"
#include "XMLUtility.h"
#include "STLStringUtility.h"
#include "MC2CRC32.h"
#include "XMLServerElements.h"
#include "LocaleUtility.h"
#include "XMLCategoriesData.h"
#include "XMLAuthData.h"
#include "ClientSettings.h"
#include "categoryRegionIDFromCoord.h"

namespace {
/**
 * Figures out which category region id to use for this request.
 */
CategoryRegionID getCategoryRegionID( DOMNode* cur, MapRequester* requester ) {

   DOMNode* positionNode = XMLTool::findNode( cur, "position_item" );

   if ( positionNode == NULL ) {
      return CategoryRegionID::NO_REGION;
   }

   MC2Coordinate coord;
   uint16 angle;
   MC2String errorCode, errorMessage;

   if ( !XMLCommonElements::getPositionItemData( positionNode,
                                                 coord,
                                                 angle,
                                                 errorCode,
                                                 errorMessage ) ) {
      mc2log << warn << "Failed to parse position_item, errorCode: "
             << errorCode << " errorMessage: " << errorMessage << endl;
      return CategoryRegionID::NO_REGION;
   }
   else {
      return categoryRegionIDFromCoord( coord,
                                        requester );
   }
}

}

bool
XMLParserThread::xmlParseCategoryListRequest(  DOMNode* cur,
                                               DOMNode* out,
                                               DOMDocument* reply,
                                               bool indent )
try { 
   // create root node category_list_reply
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "category_list_reply" );

   using namespace XMLTool;

   // get request attributes
   LangTypes::language_t language;
   MC2String crcIn;

   getAttrib( crcIn, "crc", cur );
   getAttrib( language, "language", cur );

   // Get the region for which to get the category list
   CategoryRegionID regionID = ::getCategoryRegionID( cur, this );

   // Get the CategoriesData for this client's type and language.
   auto_ptr<CategoriesData> autoCat(
      m_authData->clientSetting->getCategories( ItemTypes::getLanguageTypeAsLanguageCode( language ), 
                                                false, // do not use latin1
                                                regionID
                                                ) );
   XMLCategoriesData* catData = 
      static_cast<XMLCategoriesData*>( autoCat.get() );

   if ( ! catData ) {
      return false;
   }

   MC2String crcOut;
   STLStringUtility::uint2strAsHex( catData->getCRC(), crcOut );

   if ( crcOut == crcIn ) {
      addNode( root, "crc_ok" );
   } else {
      // append category nodes to category_list_reply
      for ( XMLCategoriesData::const_iterator it = catData->begin() ;
            it != catData->end() ; ++it ) {
         DOMElement* categoryNode = addNode( root, "cat" );
         addNode( categoryNode, "name", *(*it).getName() );
         addAttrib( categoryNode, "cat_id", 
                    static_cast<uint32>( (*it).getID() ) );
         addNode( categoryNode, "image_name", *it->getImage() );
      }
   } // End else not same crc

   out->appendChild( root );

   // add attributes
   addAttrib( root, "count", catData->getNbrCategories() );
   addAttrib( root, "crc",  crcOut );

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[CompactSarchRequest]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 

#endif // USE_XML

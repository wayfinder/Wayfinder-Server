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
#include "XMLServerElements.h"
#include "categoryRegionIDFromCoord.h"
#include "LocalCategoryTrees.h"
#include "BinaryCategoryTreeFormat.h"

class CategoryTree;

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
      return categoryRegionIDFromCoord( coord, requester );
   }
}

/**
 * Adds a Base64 encoded data buffer to the XML document.
 * @param root Parent to the new node
 * @param buffer DataBuffer to Base64Encode and add
 * @param name Name of the element containing the data
 */
void 
encodeAndAdd( DOMElement* root, 
              DataBuffer* buffer, 
              const char* name ) {
   using namespace XMLTool;
   
   if( buffer == NULL ) {
      DOMElement* categoryTableNode = addNode( root, name );
      addAttrib( categoryTableNode, "length", 0 );
      
      return;
   }

   // allocate enough space for base64
   uint32 base64Size = buffer->getBufferSize() * 4 / 3 + 4;
   ScopedArray<char> base64Out( new char[ base64Size ] );
   memset( base64Out.get(), 0, base64Size );
   if ( ! StringUtility::base64Encode( buffer->getBufferAddress(),
                                       buffer->getBufferSize(),
                                       base64Out.get() ) ) {
      MC2String msg = "Failed to base64 encode ";
      throw MC2Exception( msg + name );
   }
   
   StringUtility::base64Encode( buffer->getBufferAddress(), 
                                buffer->getBufferSize(), base64Out.get() );
   DOMElement* categoryTableNode = addNode( root, name, 
                                            MC2String( base64Out.get() ) );
   
   addAttrib( categoryTableNode, "length", buffer->getBufferSize() );
}

}

bool
XMLParserThread::xmlParseLocalCategoryTreeRequest(  DOMNode* cur,
                                                    DOMNode* out,
                                                    DOMDocument* reply,
                                                    bool indent )
try { 
   // create root node local_category_tree_reply
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "local_category_tree_reply" );

   using namespace XMLTool;
   using namespace CategoryTreeUtils;

   // get request attributes
   LangTypes::language_t language;
   MC2String crcIn;

   getAttrib( crcIn, "crc", cur );
   getAttrib( language, "language", cur );

   // Get the region for which to get the category tree
   CategoryRegionID regionID = ::getCategoryRegionID( cur, this );

   // Get the Category tree for this client's language.
   const LocalCategoryTreesPtr localCatTrees =  m_group->getLocalCategoryTrees();

   CategoryTreePtr categoryTree = localCatTrees->getTree( regionID );

   // Serialize the tree to binary format
   BinaryCategoryTreeFormat serializedTree;
   serializeTree( categoryTree.get(), language, &serializedTree );
 
   //Get crc for the tree
   MC2String crcOut = getCrcForTree( serializedTree );

   if ( crcOut == crcIn ) {
      addNode( root, "crc_ok" );
   } else {
      // Add category table
      encodeAndAdd( root, serializedTree.m_categoryTable.get(), "category_table" );

      // Add lookup table
      encodeAndAdd( root, serializedTree.m_lookupTable.get(), "lookup_table" );
      
      // Add string table
      encodeAndAdd( root, serializedTree.m_stringTable.get(), "string_table" );

      // add crc attribute
      addAttrib( root, "crc",  crcOut );
   } // End else not same crc

   out->appendChild( root );
   
   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }
   
   return true;
   
} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[XMLParserThread]  " << e.what() << endl;
   XMLServerUtility::appendStatusNodes( out, reply, 1, false, "-1", e.what() );
   return false;
} 
catch ( const MC2Exception& ex ) {
   mc2log << error << "[XMLParserThread] " << ex.what() << endl;
   XMLServerUtility::appendStatusNodes( out, reply, 1, false,"-1", ex.what() );
   return false;
}

#endif // USE_XML

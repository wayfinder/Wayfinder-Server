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

#include "SearchParserHandler.h"
#include "XMLTool.h"
#include "XMLUtility.h"
#include "STLStringUtility.h"
#include "MC2CRC32.h"
#include "XMLServerElements.h"
#include "LangTypes.h"
#include "CategoryTree.h"
#include "EventFinderCategories.h"
#include <vector>

using namespace CategoryTreeUtils;

namespace {
/**
 * Create one category node
 */
DOMElement* createCategoryNode( DOMElement* root, 
                                const CategoryTree& tree,
                                uint32 categoryID, LangTypes::language_t language,
                                MC2String& idAndName ) {
   DOMElement* cat = XMLTool::addNode( root, "cat" );
   XMLTool::addAttrib( cat, "cat_id", categoryID );

   idAndName += STLStringUtility::uint2str( categoryID );
   MC2String name = tree.getTranslation( categoryID, language );
   // try english translation if the first one failed
   if ( name.empty() ) {
      name = tree.getTranslation( categoryID, LangTypes::english );

   }

   if ( ! name.empty() ) {
      XMLTool::addNode( cat, "name", name );
      idAndName += name;
   } else {
      mc2dbg << warn << "[XMLCategoryTreeRequest] no name for category id: " 
             << categoryID << endl;
   }

   return cat;
}


/**
 * Create a xml reply tree from the category tree
 */
void createTree( DOMElement* root,
                 const CategoryTree& tree,
                 const CategoryTree::Node::NodeVector& children,
                 const LangTypes::language_t language,
                 MC2String& crcBuffer ) {
   for ( uint32 i = 0; i < children.size(); ++i ) {
      const CategoryTree::Node& child = *children[ i ];
      DOMElement* childNode = 
         createCategoryNode( root, tree, child.getID(), language,
                             crcBuffer );

      root->appendChild( childNode );
      // has any children to append?
      if ( ! child.getChildren().empty() ) {
         createTree( childNode, tree, child.getChildren(), 
                     language, crcBuffer );
      }
   }
   
}
void addCatNode( DOMElement* root, CategoryID id,
                 const MC2String& name ) {
   DOMElement* cat = XMLTool::addNode( root, "cat" );
   XMLTool::addAttrib( cat, "cat_id", id );
   XMLTool::addNode( cat, "name", name );
   root->appendChild( cat );
}

void createMusicTree( DOMElement* root,
                      MC2String& crcBuffer ) {
   using namespace EventFinder;
#define ADDCAT(x,y) addCatNode( root, x, y ); \
   STLStringUtility::uint2str( x, crcBuffer ); \
   crcBuffer += y;

   ADDCAT( MusicCategories::ROCK, "Rock" );
   ADDCAT( MusicCategories::POP, "Pop" );
   ADDCAT( MusicCategories::JAZZ, "Jazz" );
   ADDCAT( MusicCategories::BLUES, "Blues" );
   ADDCAT( MusicCategories::DANCE_DJ, "Dance & DJ" );
   ADDCAT( MusicCategories::HARD_ROCK_HEAVY_METAL, "Heavy Metal" );
   ADDCAT( MusicCategories::HIP_HOP_RAP, "Hip Hop & Rap" );
   ADDCAT( MusicCategories::FUNK_SOUL_RB, "Funk, Soul & R&B" );
   ADDCAT( MusicCategories::OTHER, "Other music" );
   ADDCAT( MusicCategories::EVENTS, "Events" );

#undef ADDCAT

}
                      
}

bool
XMLParserThread::xmlParseCategoryTreeRequest( DOMNode* cur,
                                              DOMNode* out,
                                              DOMDocument* reply,
                                              bool indent )
try { 
   // create root node category_list_reply
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "category_tree_reply" );
   out->appendChild( root );

   using namespace XMLTool;

   // get request attributes
   LangTypes::language_t language;
   MC2String treeCRC, namesCRC, type;

   getAttrib( treeCRC, "crc", cur );
   getAttrib( language, "language", cur );
   getAttrib( type, "type", cur );

   CategoryTreeUtils::CategoryTreePtr catTree = 
      getGroup()->getCategoryTree();
   if ( ! catTree ) {
      mc2dbg << "[CategoryTreeRequest] No tree to parse." << endl;
      XMLServerUtility::appendStatusNodes( out->getFirstChild(), reply, 1,
                                           false,
                                           "-1", "No tree to parse" );
      return false;
   }


   DOMElement* tree = addNode( root, "category_tree" );


   MC2String crcBuffer;

   if ( type == "eventfinder" ) {
      ::createMusicTree( tree, crcBuffer );
   } else if ( type == "vicinity" ) {
      ::createTree( tree, *catTree, catTree->getRootNodes(), 
                    language, crcBuffer );
   }

   // calculate crc and see if it matches
   uint32 crc = MC2CRC32::crc32( reinterpret_cast<const byte*>( crcBuffer.c_str() ), 
                                 crcBuffer.size() );
   MC2String crcOut;
   STLStringUtility::uint2strAsHex( crc, crcOut );
   addAttrib( root, "crc", crcOut );
   // if crc match, then remove category_tree node and add crc_ok
   if ( crcOut == treeCRC ) {
      root->replaceChild( reply->createElement( X( "crc_ok" ) ),
                          tree );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[CategoryTreeRequest]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 

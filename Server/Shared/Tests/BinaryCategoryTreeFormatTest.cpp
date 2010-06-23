/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BinaryCategoryTreeFormat.h"
#include "XMLInit.h"
#include "MC2UnitTestMain.h"
#include "STLUtility.h"
#include "File.h"
#include "Properties.h"

XMLTool::XMLInit xmlInit;

using namespace CategoryTreeUtils;

typedef vector<CategoryTree::StandaloneNode> Categories;

/**
 * Simply tries to load the full category tree and serialize it.
 */
MC2_UNIT_TEST_FUNCTION( testFullTree ) {
   Properties::setPropertyFileName( "/dev/null" );
   Properties::insertProperty( "IMAGES_PATH", "../../bin/Images" );
   CategoryTree tree;
   MC2_TEST_REQUIRED_EXT( tree.load( 
                             "data/poi_category_tree.xml",
                             "data/category_tree_default_configuration.xml" ),
                          "Failed to load category tree, not in path?" );

   BinaryCategoryTreeFormat binaryFormat;
   serializeTree( &tree, LangTypes::english, &binaryFormat );
}

/**
 * Tests the serializeTree function and does some checking on its output.
 */
MC2_UNIT_TEST_FUNCTION( testSerialize ) {
   // create and load the tree
   CategoryTree tree;
   MC2_TEST_REQUIRED_EXT( tree.load( 
                             "data/small_category_tree.xml",
                             "data/category_tree_default_configuration.xml" ), 
                          "Failed to load category tree, not in path?" );

   BinaryCategoryTreeFormat binaryFormat;
   serializeTree( &tree, LangTypes::english, &binaryFormat );

/*   File::writeFile( "strings.dat", 
                    binaryFormat.m_stringTable->getBufferAddress(),
                    binaryFormat.m_stringTable->getBufferSize() );
   
   File::writeFile( "lookup.dat", 
                    binaryFormat.m_lookupTable->getBufferAddress(),
                    binaryFormat.m_lookupTable->getBufferSize() );

   File::writeFile( "categories.dat", 
                    binaryFormat.m_categoryTable->getBufferAddress(),
                    binaryFormat.m_categoryTable->getBufferSize() );*/
                    

   MC2_TEST_CHECK( binaryFormat.m_lookupTable->getBufferSize() == 24 );
   MC2_TEST_CHECK( binaryFormat.m_stringTable->getBufferSize() == 78 );

   Categories categories;
   try {
      parseBinaryFormat( &binaryFormat, categories );
   } catch( const MC2Exception& e ) {
      MC2_TEST_CHECK_EXT( false, e.what() );
   }

   MC2_TEST_CHECK( categories.size() == 3 );

   MC2_TEST_CHECK( categories[0].getID() == 1 );
   MC2_TEST_CHECK( categories[1].getID() == 2 );
   MC2_TEST_CHECK( categories[2].getID() == 3 );

   MC2_TEST_CHECK( categories[0].getName() == "On the move" );
   MC2_TEST_CHECK( categories[1].getName() == "By car" );
   MC2_TEST_CHECK( categories[2].getName() == "Food and drink" );

   MC2_TEST_CHECK( categories[0].getIcon() == "tat_hospital" );
   MC2_TEST_CHECK( categories[1].getIcon() == "tat_hotel" );
   MC2_TEST_CHECK( categories[2].getIcon() == "tat_shop" );

   MC2_TEST_CHECK( categories[0].getChildren().size() == 1 );
   MC2_TEST_CHECK( categories[1].getChildren().size() == 0 );
   MC2_TEST_CHECK( categories[2].getChildren().size() == 0 );

   MC2_TEST_CHECK( categories[0].getChildren()[0] == 2 );
}

/**
 * Tests the copy constructor and makes sure the serialization doesn't
 * generate any random bytes.
 */
MC2_UNIT_TEST_FUNCTION( testCopyAndSerialize ) {
   // create and load the tree
   CategoryTree tree1;
   MC2_TEST_REQUIRED_EXT( tree1.load(
                             "data/small_category_tree.xml",
                             "data/category_tree_default_configuration.xml" ), 
                          "Failed to load category tree, not in path?" );

   // serialize the first tree
   BinaryCategoryTreeFormat binaryFormat1;
   serializeTree( &tree1, LangTypes::english, &binaryFormat1 );

   // copy
   CategoryTree tree2( tree1);

   // serialize the second tree
   BinaryCategoryTreeFormat binaryFormat2;
   serializeTree( &tree2, LangTypes::english, &binaryFormat2 );

   // compare the buffers
   MC2_TEST_CHECK( binaryFormat1.m_categoryTable->equals(
                      *binaryFormat2.m_categoryTable ) );
   MC2_TEST_CHECK( binaryFormat1.m_stringTable->equals(
                      *binaryFormat2.m_stringTable ) );
   MC2_TEST_CHECK( binaryFormat1.m_lookupTable->equals(
                      *binaryFormat2.m_lookupTable ) );
}

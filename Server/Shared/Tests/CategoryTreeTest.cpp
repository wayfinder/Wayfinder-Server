/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryTree.h"
#include "XMLInit.h"
#include "UTF8Util.h"
#include "MC2UnitTestMain.h"

XMLTool::XMLInit xmlInit;

using namespace CategoryTreeUtils;

/**
 * Verifies that the correct categories are returned for a given
 * search string.
 * @param tree             The category tree to search.
 * @param languageString   The language to use.
 * @param searchString     What to search for.
 * @param correct          What should be found.
 */
void verifySearch( const CategoryTree& tree,
                   const MC2String& languageString,
                   const MC2String& searchString,
                   CategorySet correct ) {

   CategorySet categories;
   LangTypes::language_t language;
   StringConvert::assign( language, languageString );
   tree.findCategories( searchString, language, categories );

   MC2_TEST_CHECK_EXT(  categories == correct,
                        languageString + " search for " + 
                        searchString + " failed");
}

/**
 * Tests CategoryTree::findCategories.
 */
MC2_UNIT_TEST_FUNCTION( findCategory ) {
   // create and load the tree

   CategoryTree originalTree;
   MC2_TEST_REQUIRED_EXT( originalTree.load( "data/poi_category_tree.xml", "" ),
                          "Failed to load category tree, not in path?" );
   
   // do the tests with a copy so we test the copy constructor as well
   CategoryTree tree( originalTree );

   const size_t cidsize = sizeof( CategoryID );

   // test various search strings in different languages
   CategoryID italianArr[] = { 190 };
   CategorySet 
      italian( italianArr, 
               italianArr+sizeof( italianArr )/cidsize );

   verifySearch( tree, "english", "italian", italian );
   verifySearch( tree, "english", "italian restaurant", italian );
   verifySearch( tree, "english", "italian restaurants", italian );
   verifySearch( tree, "english", "restaurants, italian", italian );
   verifySearch( tree, "english", ",,italian ,,,", italian );
   verifySearch( tree, "norwegian", "italiensk restaurant", italian );
   verifySearch( tree, "norwegian", "italiensk", italian );
   verifySearch( tree, "norwegian", "rest ital", italian );
   verifySearch( tree, "swedish",
                 UTF8Util::isoToUtf8(MC2String("italienskt kök")), italian );
   verifySearch( tree, "swedish", "italiensk restaurang", italian );
   verifySearch( tree, "swedish", "rest ital", italian );
   verifySearch( tree, "italian", "cuCIna itaLIANA", italian );

   verifySearch( tree, "italian", "ristoranti italiana", italian );
   verifySearch( tree, "italian", " italiana ristoranti", italian );

   CategoryID vegetarianArr[] = { 156 };
   CategorySet 
      vegetarian( vegetarianArr, 
                  vegetarianArr+sizeof( vegetarianArr )/cidsize );

   verifySearch( tree, "english", "veg", vegetarian );
   verifySearch( tree, "italian", "veg", vegetarian );

   CategoryID barsArr[] = { 218, 100, 229, 230, 
                                          231, 232, 233, 234, 235,
                                          167, 226, 66 };
   CategorySet bars( barsArr,
                                   barsArr+sizeof( barsArr )/cidsize );

   verifySearch( tree, "english", "bar", bars );
   // test separators
   verifySearch( tree, "english", ".", CategorySet() );
   verifySearch( tree, "english", ",", CategorySet() );
   verifySearch( tree, "english", " ", CategorySet() );
   verifySearch( tree, "english", " ,.", CategorySet() );
   verifySearch( tree, "english", " .,", CategorySet() );
   verifySearch( tree, "english", ",.", CategorySet() );
   verifySearch( tree, "english", " .", CategorySet() );
   verifySearch( tree, "english", ". ", CategorySet() );
   verifySearch( tree, "english", ",.,.,.,. ,. ,. ,., ., ",
                 CategorySet() );
}

/**
 * Tests CategoryTree::getAllCategories, especially its ability
 * to follow linked categories.
 */
MC2_UNIT_TEST_FUNCTION( getAllCategories ) {
   // create and load the tree
   CategoryTree originalTree;
   MC2_TEST_REQUIRED_EXT( originalTree.load( "data/poi_category_tree.xml", "" ),
                          "Failed to load category tree, not in path?" );

   // do the tests with a copy so we test the copy constructor as well
   CategoryTree tree( originalTree );

   CategoryTreeUtils::CategorySet cs;
   tree.getAllCategories( 274, cs ); // Food and drink

   // Check that Bars is included even though it's a link
   MC2_TEST_CHECK( cs.count( 100 ) > 0 );

   // Check that Wine bars, which is a subcategory to Bars, is included
   MC2_TEST_CHECK( cs.count( 229 ) > 0 );
}

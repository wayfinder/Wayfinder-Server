/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"

#include "CategoryTreeRegionConfiguration.h"
#include "BinaryCategoryTreeFormat.h"
#include "XMLTool.h"
#include "XMLInit.h"
#include "ArrayTools.h"
#include "Properties.h"

XMLTool::XMLInit xmlInit;

using namespace CategoryTreeUtils;

typedef vector<CategoryTree::StandaloneNode> Categories;

// Helper for finding StandaloneNode by CategoryID
struct CatEqualCmp {
   explicit CatEqualCmp( const CategoryID& id ):
      m_id( id ) { }

   bool operator () ( const CategoryTree::StandaloneNode& node ) const {
      return m_id == node.getID();
   }
   const CategoryID m_id;
};

// Compare two StandaloneNodes
bool equalNodes( CategoryTree::StandaloneNode n1, CategoryTree::StandaloneNode n2 ) {
   return n1.getID() == n2.getID() &&
      n1.getName() == n2.getName() &&
      n1.getIcon() == n2.getIcon();
}

MC2_UNIT_TEST_FUNCTION( CategoryTreeRegionConfigurationTest ) {
   RegionConfigurations* regionConfs = 
      CategoryTreeUtils::loadRegionConfigurations(
         MC2String("data/category_tree_region_configuration.xml"));
   MC2_TEST_CHECK( regionConfs != NULL );
   
   if ( regionConfs != NULL ) {
      RegionConfigurations::const_iterator regionConfsIt = regionConfs->begin();
      MC2_TEST_CHECK( regionConfsIt != regionConfs->end() );
      if ( regionConfsIt != regionConfs->end() ) {
         RegionConfiguration::IconInfo info;
         regionConfsIt->second->getCategoryIcons( info );
         MC2_TEST_CHECK( info.size() == 3 );
         MC2_TEST_CHECK( ( (info[0].first == static_cast< CategoryID >( 1 )) && 
                           (info[0].second == "new.ico") ) );
         MC2_TEST_CHECK( ( (info[1].first == static_cast< CategoryID >( 2 )) && 
                           (info[1].second == "new2.ico") ) );
         MC2_TEST_CHECK( ( (info[2].first == static_cast< CategoryID >( 3 )) && 
                           (info[2].second == "new3.ico") ) );
         
         MC2_TEST_CHECK(  regionConfsIt->second->isCategoryVisible( 
                             static_cast< CategoryID >( 1 ) ) );
         MC2_TEST_CHECK( !regionConfsIt->second->isCategoryVisible( 
                            static_cast< CategoryID >( 2 ) ) );
         MC2_TEST_CHECK( !regionConfsIt->second->isCategoryVisible( 
                            static_cast< CategoryID >( 3 ) ) );
         MC2_TEST_CHECK(  regionConfsIt->second->isCategoryVisible( 
                             static_cast< CategoryID >( 4 ) ) );
         MC2_TEST_CHECK( !regionConfsIt->second->isCategoryVisible( 
                            static_cast< CategoryID >( 5 ) ) );
         MC2_TEST_CHECK( !regionConfsIt->second->isCategoryVisible( 
                            static_cast< CategoryID >( 6 ) ) );
      }
      delete regionConfs;
   }
}

MC2_UNIT_TEST_FUNCTION( applyRegionConfigurationTest ) {
   // TEST 1 - Normal behavior
   {
      Properties::setPropertyFileName( "/dev/null" );
      Properties::insertProperty( "IMAGES_PATH", "../../bin/Images" );
      CategoryTree tree;
      MC2_TEST_REQUIRED_EXT( tree.load( "data/poi_category_tree.xml", 
                                        "data/category_tree_default_configuration.xml" ),
                             "Failed to load category tree, not in path?" );
      
      RegionConfiguration config;
      
      CategoryID categories[] = { 85, 
                                  156, 
                                  274, // food and drink
                                  225  // tea (cafe not included though)
      };
      
      MC2String iconExceptions[] = { "",
                                     "tat_hotel",
                                     "",
                                     "tat_atm"
      };
      
      for ( size_t i = 0; i < NBR_ITEMS( categories ); ++i ) {
         config.addCategory( categories[ i ], true, iconExceptions[i] );
      }
      
      CategoryTree* newTree = applyRegionConfiguration( &tree, config );
      
      // the only root is food and drink
      MC2_TEST_CHECK( newTree->getRootNodes().size() == 1 );
      
      // make sure the tree has the correct categories
      CategorySet cs;
      newTree->getAllCategories( newTree->getRootNodes()[ 0 ]->getID(), cs );
      MC2_TEST_CHECK( cs == CategorySet( BEGIN_ARRAY( categories ),
                                      END_ARRAY( categories ) ) );
      
      // the root should have two children, restaurants and tea
      CategoryTree::Node* root = newTree->getRootNodes()[ 0 ];
      
      MC2_TEST_CHECK( root->getChildren().size() == 2 );
      MC2_TEST_CHECK( root->getChildren()[ 0 ]->getID() == 85 );
      MC2_TEST_CHECK( root->getChildren()[ 1 ]->getID() == 225 );
      
      // check the translations
      MC2String translations[] = { "Restaurants",
                                   "Vegetarian",
                                   "Food & Drink",
                                   "Tea"
      };
      
      Categories regionNodes = newTree->getStandaloneNodes( LangTypes::english );
      
      MC2String iconResult[] = { "tat_bank",
                                 "tat_hotel",
                                 "tat_shop",
                                 "tat_atm" };
      
      for ( size_t i = 0; i < NBR_ITEMS( categories ); ++i ) {
         MC2String translation = 
            newTree->getTranslation( categories[ i ], LangTypes::english );
         MC2_TEST_CHECK( translation == translations[ i ] );
         
         // check that icon exceptions has been applied
         CatEqualCmp cmp( categories[ i ] );
         Categories::iterator findIt = 
            find_if( regionNodes.begin(), regionNodes.end(), cmp );
         
         MC2_TEST_CHECK( findIt != regionNodes.end() &&
                         findIt->getIcon() == iconResult[i] );
      }
      
      // Test the serialization of the regional tree
      BinaryCategoryTreeFormat binaryFormat;
      serializeTree( newTree, LangTypes::english, &binaryFormat );
      
      Categories serializedCategories;
      try {
         parseBinaryFormat( &binaryFormat, serializedCategories ); 
      } catch( const MC2Exception& e ) {
         MC2_TEST_REQUIRED_EXT( false, "Failed to parse the serialized tree!" );
      }
      
      // Verify that the serialized tree equals the origial tree
      MC2_TEST_CHECK_EXT( std::equal( regionNodes.begin(), regionNodes.end(), 
                                      serializedCategories.begin(), equalNodes),
                          "Serialized tree differs from original." );
   }
   // TEST 2 - Non existing icons
   {
      CategoryTree tree;
      MC2_TEST_REQUIRED_EXT( tree.load( "data/poi_category_tree.xml", "" ),
                             "Failed to load category tree, not in path?" );
      
      RegionConfiguration config;
      
      CategoryID categories[] = { 85, 
                                  156, 
                                  274, // food and drink
                                  225  // tea (cafe not included though)
      };
      
      MC2String iconExceptions[] = { "nullIcon",
                                     "tat_hotel",
                                     "",
                                     "tat_atm"
      };
      
      for ( size_t i = 0; i < NBR_ITEMS( categories ); ++i ) {
         config.addCategory( categories[ i ], true, iconExceptions[i] );
      }
      try {
         applyRegionConfiguration( &tree, config );
         MC2_TEST_CHECK( false );
      } catch (MC2Exception&) {
         MC2_TEST_CHECK( true );
      }
   }
}



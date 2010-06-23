/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryTreeDefaultConfiguration.h"
#include "XMLInit.h"
#include "MC2UnitTestMain.h"
#include "Properties.h"

XMLTool::XMLInit xmlInit;

using namespace CategoryTreeUtils;

MC2_UNIT_TEST_FUNCTION( CategoryTreeDefaultConfigurationTest ) {
   Properties::setPropertyFileName( "/dev/null" );
   Properties::insertProperty( "IMAGES_PATH", "../../bin/Images" );

   // First test. Valid file. Make sure the icons exist.
   DefaultConfiguration* defaultConfs = CategoryTreeUtils::loadDefaultConfiguration(
      MC2String("data/category_tree_default_configuration.xml"));
   MC2_TEST_CHECK( defaultConfs != NULL );
   
   if ( defaultConfs != NULL ) {
      MC2_TEST_CHECK( defaultConfs->getCategoryIcon( static_cast< CategoryID >( 1 ) ) == "tat_hospital");
      MC2_TEST_CHECK( defaultConfs->getCategoryIcon( static_cast< CategoryID >( 2 ) ) == "tat_hotel");
      MC2_TEST_CHECK( defaultConfs->getCategoryIcon( static_cast< CategoryID >( 3 ) ) == "");
      MC2_TEST_CHECK( defaultConfs->getCategoryIcon( static_cast< CategoryID >( 4 ) ) == "");

      delete defaultConfs;
   }

   // Second test. Empty path to file. Shall return NULL.
   defaultConfs = CategoryTreeUtils::loadDefaultConfiguration( "" );
   MC2_TEST_CHECK( defaultConfs == NULL );

   // Third test. Path to file.
   try {
      defaultConfs = CategoryTreeUtils::loadDefaultConfiguration( 
         "data/category_tree_unknown_file.xml" );
      MC2_TEST_CHECK( false ); // Exception shall be thrown
   } catch (MC2Exception& e) {
      MC2_TEST_CHECK( defaultConfs == NULL );
   }
   

   // Fourth test. Invalid icons.
   try {
      defaultConfs = CategoryTreeUtils::loadDefaultConfiguration( "data/category_tree_bad_icons.xml" );
      MC2_TEST_CHECK( false ); // Exception shall be thrown
   } catch (MC2Exception& e) {
      MC2_TEST_CHECK( defaultConfs == NULL );
   }

}


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

#include "CentroidCalculation.h"
#include "GfxFeature.h"
#include "MC2Coordinate.h"
#include "GfxPolygon.h"
#include "GfxDataFull.h"
#include "Item.h"

#include <vector>
#include <memory>

//
// Test centroid lookup table and large area centroid calculations.
//

MC2_UNIT_TEST_FUNCTION( centroidCalculationTest ) {
   using namespace CentroidCalculation;

   GfxFeature feature( GfxFeature::LAND, "" );
   MC2Coordinate swedenCoord( 748006449, 185322523 );
   //
   // test an existing land names
   //
   feature.setBasename( "sweden" );
   MC2_TEST_CHECK( getCentroidForLandFeature( feature ) == swedenCoord );

   feature.setBasename( "finland" );
   MC2_TEST_CHECK( getCentroidForLandFeature( feature ) ==
                   MC2Coordinate( 756900185, 311107365 ) );

   feature.setBasename( "norway" );
   MC2_TEST_CHECK( getCentroidForLandFeature( feature ) ==
                   MC2Coordinate( 734035313, 105903372 ) );

   // should be case insensitive
   feature.setBasename( "SwEdeN" );
   MC2_TEST_CHECK( getCentroidForLandFeature( feature ) == swedenCoord );

   // test a non existing land name
   feature.setBasename( "weden" );
   MC2_TEST_CHECK( getCentroidForLandFeature( feature ) ==
                   MC2Coordinate() );

   // it should not care about the normal name.
   feature.setName( "sweden" );
   MC2_TEST_CHECK( getCentroidForLandFeature( feature ) ==
                   MC2Coordinate() );

   // Create a simple polygon with four corners, so we
   // can verify that the centre calculation is correct
   auto_ptr< GfxPolygon > poly( new GfxPolygon( false ) );
   vector< MC2Coordinate > coords;
   coords.push_back( MC2Coordinate( 100, -100 ) );
   coords.push_back( MC2Coordinate( 100, 100 ) );
   coords.push_back( MC2Coordinate( -100, 100 ) );
   coords.push_back( MC2Coordinate( -100, -100 ) );
   poly->setCoords( coords );

   // Setup feature to return an invalid center.
   feature.addNewPolygon( poly.release() );
   feature.setName( "" );
   feature.setBasename( "" );
   MC2_TEST_CHECK_EXT( getCentroidForLandFeature( feature ) ==
                       MC2Coordinate(),
                       "Should not try to calculate coordinate from feature!" );

   // setup an item with a gfx data with polygon centre at 0, 0
   Item item;
   auto_ptr< GfxDataFull > gfx( new GfxDataFull() );
   gfx->addPolygon( (uint32)0 );
   // must have a closed polygon.
   gfx->setClosed( 0, true );
   gfx->addCoordinate( MC2Coordinate( 100, -100 ) );
   gfx->addCoordinate( MC2Coordinate( 100, 100 ) );
   gfx->addCoordinate( MC2Coordinate( -100, 100 ) );
   gfx->addCoordinate( MC2Coordinate( -100, -100 ) );
   item.setGfxData( gfx.get() );

   // pre-requirements
   MC2_TEST_REQUIRED( feature.getNbrPolygons() == 1 );

   addCentroidForAreas( feature, item );

   // post-requirements, should have a new polygon with one coordinate
   MC2_TEST_REQUIRED( feature.getNbrPolygons() == 2 );
   MC2_TEST_REQUIRED( feature.getPolygon( 1 )->getNbrCoordinates() == 1 );
   MC2Coordinate center = feature.getPolygon( 1 )->getCoord( 0 );
   MC2_TEST_CHECK_EXT( center == MC2Coordinate( 0, 0 ), center );

   // Check so it does not add any coordinates for features that are not
   // large areas.
   GfxFeature notLargeAreaFeature( GfxFeature::STREET_MAIN );
   // pre-requirements
   MC2_TEST_REQUIRED( notLargeAreaFeature.getNbrPolygons() == 0 );
   addCentroidForAreas( notLargeAreaFeature, item );
   // post-requirements, should not have a new polygon.
   MC2_TEST_CHECK( notLargeAreaFeature.getNbrPolygons() == 0 );
}

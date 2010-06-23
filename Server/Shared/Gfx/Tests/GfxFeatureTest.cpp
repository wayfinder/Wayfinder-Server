/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxFeature.h"
#include "DataBuffer.h"
#include "MC2UnitTestMain.h"

void comparePOI( const GfxPOIFeature& first,
                 const GfxPOIFeature& second ) {
   MC2_TEST_REQUIRED( first.getPOIType() == second.getPOIType() );
   MC2_TEST_REQUIRED( first.getExtraInfo() == second.getExtraInfo() );
   MC2_TEST_REQUIRED( first.getCustomImageName() ==
                      second.getCustomImageName() );
   MC2_TEST_REQUIRED( first.getCategories() == second.getCategories() );
}

void compareStreet( const GfxRoadFeature& first,
                    const GfxRoadFeature& second ) {
   // No special data for this one.
}

void compareEvent( const GfxEventFeature& first,
                   const GfxEventFeature& second ) {
   MC2_TEST_REQUIRED( first.getStrings() == second.getStrings() );
   MC2_TEST_REQUIRED( first.getDate() == second.getDate() );
   MC2_TEST_REQUIRED( first.getDuration() == second.getDuration() );
   MC2_TEST_REQUIRED( first.getID() == second.getID() );
}

void compareTraffic( const GfxTrafficInfoFeature& first,
                     const GfxTrafficInfoFeature& second ) {
   MC2_TEST_REQUIRED( first.getTrafficInfoType() == second.getTrafficInfoType() );
   MC2_TEST_REQUIRED( first.isValidInBothDirections() ==
                      second.isValidInBothDirections() );
   MC2_TEST_REQUIRED( first.getAngle() == second.getAngle() );
   MC2_TEST_REQUIRED( first.getStartTime() == second.getStartTime() );
   MC2_TEST_REQUIRED( first.getEndTime() == second.getEndTime() );
}

void compareSymbol( const GfxSymbolFeature& first,
                    const GfxSymbolFeature& second ) {
   MC2_TEST_REQUIRED( first.getSymbol() == second.getSymbol() );
   MC2_TEST_REQUIRED( strcmp( first.getSymbolImage(),
                              second.getSymbolImage() ) == 0);
}

/*
 * Compare base class and then compare type specific.
 */
void compareFeature( const GfxFeature& first,
                     const GfxFeature& second ) {
   MC2_TEST_REQUIRED( first.getType() == second.getType() );
   MC2_TEST_REQUIRED( first.getSize() == second.getSize() );
   MC2_TEST_REQUIRED( strcmp( first.getName(), second.getName() ) == 0 );
   MC2_TEST_REQUIRED( first.getNbrPolygons() == second.getNbrPolygons() );
   MC2_TEST_REQUIRED( first.getScaleLevel() == second.getScaleLevel() );
   MC2_TEST_REQUIRED( first.getTextLat() == second.getTextLat() );
   MC2_TEST_REQUIRED( first.getTextLon() == second.getTextLon() );
   MC2_TEST_REQUIRED( first.getDisplayText() == second.getDisplayText() );
   MC2_TEST_REQUIRED( first.getFontSize() == second.getFontSize() );
   MC2_TEST_REQUIRED( first.getDrawTextStart() == second.getDrawTextStart() );
   MC2_TEST_REQUIRED( first.getNameIsAbbreviated() ==
                      second.getNameIsAbbreviated() );
   MC2_TEST_REQUIRED( first.getLangType() == second.getLangType() );
   MC2_TEST_REQUIRED( first.getCountryCode() == second.getCountryCode() );
   MC2_TEST_REQUIRED( first.getBasename() == second.getBasename() );

   switch ( first.getType() ) {
   case GfxFeature::STREET_MAIN:
   case GfxFeature::STREET_FIRST:
   case GfxFeature::STREET_SECOND:
   case GfxFeature::STREET_THIRD:
   case GfxFeature::STREET_FOURTH:
      compareStreet( dynamic_cast< const GfxRoadFeature& >( first ),
                     dynamic_cast< const GfxRoadFeature& >( second ) );
      break;
   case GfxFeature::POI:
      comparePOI( dynamic_cast< const GfxPOIFeature& >( first ),
                  dynamic_cast< const GfxPOIFeature& >( second ) );
      break;
   case GfxFeature::SYMBOL:
      compareSymbol( dynamic_cast< const GfxSymbolFeature& >( first ),
                     dynamic_cast< const GfxSymbolFeature& >( second ) );
      break;
   case GfxFeature::TRAFFIC_INFO:
      compareTraffic( dynamic_cast< const GfxTrafficInfoFeature& >( first ),
                      dynamic_cast< const GfxTrafficInfoFeature& >( second ) );
      break;
   case GfxFeature::EVENT:
      compareEvent( dynamic_cast< const GfxEventFeature& >( first ),
                    dynamic_cast< const GfxEventFeature& >( second ) );
      break;
   default:
      break;
   }
}

/*
 * This test is to make sure we dont get the default values
 * when saving and loading.
 */
void testFeatureSpecific( GfxFeature& feature ) {
   switch ( feature.getType() ) {
   case GfxFeature::STREET_MAIN:
   case GfxFeature::STREET_FIRST:
   case GfxFeature::STREET_SECOND:
   case GfxFeature::STREET_THIRD:
   case GfxFeature::STREET_FOURTH:
      // nothing special here, yet.
      break;
   case GfxFeature::POI: {
      GfxPOIFeature& poi = dynamic_cast< GfxPOIFeature& >( feature );

      poi.setPOIType( ItemTypes::cinema );
      MC2_TEST_REQUIRED( poi.getPOIType() == ItemTypes::cinema );
      poi.setExtraInfo( 37 );
      MC2_TEST_REQUIRED( poi.getExtraInfo() == 37 );
      poi.setCustomImageName( "test" );
      MC2_TEST_REQUIRED( poi.getCustomImageName() == "test" );

      GfxPOIFeature::Categories cats, orgcats;
      cats.push_back( 37 );
      cats.push_back( 48 );
      orgcats = cats;
      poi.swapCategories( cats );
      MC2_TEST_REQUIRED( poi.getCategories() == orgcats );
   } break;

   case GfxFeature::SYMBOL: {
      GfxSymbolFeature& symbol = dynamic_cast< GfxSymbolFeature& >( feature );
      symbol.setSymbol( GfxSymbolFeature::USER_DEFINED );
      MC2_TEST_REQUIRED( symbol.getSymbol() ==
                         GfxSymbolFeature::USER_DEFINED );
      symbol.setSymbolImage( "test1" );
      MC2_TEST_REQUIRED( strcmp( symbol.getSymbolImage(), "test1" ) == 0 );
   } break;

   case GfxFeature::TRAFFIC_INFO: {
      GfxTrafficInfoFeature& traffic =
         dynamic_cast< GfxTrafficInfoFeature& >( feature );
      traffic.setTrafficInfoType( TrafficDataTypes::ActionPlan );
      MC2_TEST_REQUIRED( traffic.getTrafficInfoType() ==
                         TrafficDataTypes::ActionPlan );
      // so we set it to another value.
      MC2_TEST_REQUIRED( traffic.isValidInBothDirections() == false );
      traffic.setValidInBothDirections( true );
      MC2_TEST_REQUIRED( traffic.isValidInBothDirections() == true );
      traffic.setAngle( 180 );
      MC2_TEST_REQUIRED( traffic.getAngle() >= 179 &&
                         traffic.getAngle() <= 181 );

      traffic.setStartTime( 87 );
      MC2_TEST_REQUIRED( traffic.getStartTime() == 87 );
      traffic.setEndTime( 92 );
      MC2_TEST_REQUIRED( traffic.getEndTime() == 92 );
   } break;

   case GfxFeature::EVENT: {
      GfxEventFeature& event = dynamic_cast< GfxEventFeature& >( feature );
      GfxEventFeature::Strings strings;
      strings.push_back( GfxEventFeature::StringType( 27, "1test1" ) );
      event.addString( GfxEventFeature::StringType( 27, "1test1" ) );
      strings.push_back( GfxEventFeature::StringType( 65, "2test2" ) );
      event.addString( GfxEventFeature::StringType( 65, "2test2" ) );

      MC2_TEST_REQUIRED( event.getStrings() == strings );

      GfxEventFeature::Categories cats;
      cats.push_back( GfxEventFeature::Categories::value_type( 20 ) );
      event.addCategory( GfxEventFeature::Categories::value_type( 20 ) );
      MC2_TEST_REQUIRED( event.getCategories() == cats );

      event.setDate( 98 );
      MC2_TEST_REQUIRED( event.getDate() == 98 );
      event.setDuration( 100 );
      MC2_TEST_REQUIRED( event.getDuration() == 100 );
      event.setID( 57 );
      MC2_TEST_REQUIRED( event.getID() == 57 );
   } break;
   default:
      break;
   }

   DataBuffer buffer( feature.getSize() );
   feature.save( &buffer );
   buffer.reset(); // restore offset
   GfxFeature* clonedFeature =
      GfxFeature::createNewFeature( &buffer );
   compareFeature( feature, *clonedFeature );
}


void testFeatureSaveAndLoad( GfxFeature& feature ) {
   const char name[] = "abcdefgh";

   // store 16 bit or 32 bit coordinates
   for ( bool coord16Bit = false; ! coord16Bit; coord16Bit = true ) {
      // for different nbr of coordinates
      for ( uint32 nbrCoordinates = 0;
            nbrCoordinates < 3; ++nbrCoordinates ) {
         // for different nbr polys
         for ( uint32 nbrPolys = 0; nbrPolys < 3; ++nbrPolys ) {
            // for different name lengths
            for ( uint32 i = 0;
                  i < sizeof ( name ) / sizeof ( name[ 0 ] ); ++i ) {
               feature.setName( &name[ i ] );
               feature.setBasename( &name[ i ] );

               // add polys and coordinates
               for ( uint32 polyIndex = 0; polyIndex < nbrPolys; ++polyIndex ) {
                  feature.addNewPolygon( coord16Bit );
                  for ( uint32 coordIndex = 0; coordIndex < nbrCoordinates;
                        ++coordIndex ) {
                     feature.addCoordinateToLast( 123, 456,
                                                  0, 0 );
                  }
               }
               DataBuffer buffer( feature.getSize() );
               feature.save( &buffer );
               buffer.reset(); // restore offset
               GfxFeature* clonedFeature =
                  GfxFeature::createNewFeature( &buffer );
               // compare feature
               compareFeature( feature, *clonedFeature );
            }
         }
      }
   } // 16 bit coord

   // now set some feature specific stuff and test them
   testFeatureSpecific( feature );

}

MC2_UNIT_TEST_FUNCTION( gfxFeatureTest ) {

   GfxFeature::gfxFeatureType testTypes[] = {
      GfxFeature::STREET_MAIN, // one street covers all street types
      GfxFeature::POI, // point of interest
      GfxFeature::SYMBOL, // Images
      GfxFeature::TRAFFIC_INFO, // For traffic info
      GfxFeature::EVENT, // creates event type
      GfxFeature::EMPTY, // creates generic feature
   };

   for ( uint32 featureTypeIndex = 0;
         featureTypeIndex < sizeof ( testTypes ) / sizeof ( testTypes[ 0 ] );
         ++featureTypeIndex ) {
      auto_ptr<GfxFeature>
         testFeature( GfxFeature::
                      createNewFeature( testTypes[ featureTypeIndex ], "" ) );
      testFeatureSaveAndLoad( *testFeature );
   }

}

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

#include "TrafficElementDatabase.h"
#include "SQLTrafficElementStrings.h"
#include "DisturbanceElement.h"
#include "SQLTrafficElementDatabase.h"
#include "CommandlineOptionHandler.h"
#include "DisturbanceChangeset.h"

#include "DeleteHelpers.h"

class TestTED: public TrafficElementDatabase {
public:
   virtual ~TestTED() {
   }

   int updateChangeset( const DisturbanceChangeset& changes );
};

int TestTED::updateChangeset( const DisturbanceChangeset& changes ) {
   return TrafficElementDatabase::OK;
}

void createFakeElements( TrafficElementDatabase::TrafficElements& elements) {
   // create some dummy elements
   elements.
      push_back( new DisturbanceElement( 10, // DisturbanceID
                                         "abc", // Situation reference
                                         TrafficDataTypes::Accident, // type
                                         TrafficDataTypes::ABL, // phras
                                         1, // event code
                                         10, // start time
                                         20, // end time
                                         0, // creation time
                                         TrafficDataTypes::Blocked, // severity
                                         TrafficDataTypes::Positive, // direction
                                         "first location",
                                         "second location",
                                         30, // extent
                                         4000, // cost Factor
                                         "Description text", // text
                                         50 // queue length
                                         ) );
   // add some fake route indexes
   elements.back()->addCoordinate( 0, // node ID
                                   100, // latitude
                                   200, // longitude
                                   180, // angle
                                   3 ); // route index

   elements.back()->addCoordinate( 1, // node ID
                                   300, // latitude
                                   400, // longitude
                                   90, // angle
                                   4 ); // route index

   elements.back()->setMapID( 14 );
      
   elements.
      push_back( new DisturbanceElement( 40, // DisturbanceID
                                         "def", // Situation reference
                                         TrafficDataTypes::Activities, // type
                                         TrafficDataTypes::ABX, // phras
                                         6, // event code
                                         70, // start time
                                         80, // end time
                                         1, // creation time
                                         TrafficDataTypes::Closed, // severity
                                         TrafficDataTypes::Negative, // direction
                                         "second first location",
                                         "second second location",
                                         90, // extent
                                         9900, // cost Factor
                                         "second Description text", // text
                                         120 // queue length
                                         ) );
   // add some fake route indexes
   elements.back()->addCoordinate( 2, // node ID
                                   500, // latitude
                                   600, // longitude
                                   270, // angle
                                   5 ); // route index

   elements.back()->addCoordinate( 3, // node ID
                                   700, // latitude
                                   800, // longitude
                                   360, // angle
                                   6 ); // route index

   elements.back()->setMapID( 14 );
}

/* TODO: This requires a real connection to the database.
         It works though...(if you have mc2.prop in the same directory)

MC2_UNIT_TEST_FUNCTION( databaseTest ) {
   int argc = 1;
   char* argv[] = { "databaseTest" };
   CommandlineOptionHandler coh( argc, argv );
   coh.parse();

   SQLTrafficElementDatabase database;
   DisturbanceChangeset::Elements elements;
   DisturbanceChangeset::Elements emptySet;
   createFakeElements( elements );
   DisturbanceChangeset changeset( elements, emptySet );
   // add new elements
   database.updateChangeset( changeset );
   // swap update set with remove set
   changeset.swapRemoveSet( changeset.getUpdateSet() );
   // remove them again
   database.updateChangeset( changeset );
}
*/
/*
MC2_UNIT_TEST_FUNCTION( packetTest ) {

   // setup properties
   int argc = 1;
   char* argv[] = { "databaseTest" };
   CommandlineOptionHandler coh( argc, argv );
   coh.parse();


   SQLTrafficElementDatabase database;
   TrafficElementDatabase::TrafficElements disturbances;
   database.fetchAllDisturbances( "SRA", // provider ID
                                  disturbances );
   MC2_TEST_REQUIRED( ! disturbances.empty() );

   for ( TrafficElementDatabase::TrafficElements::const_iterator
            it = disturbances.begin();
         it != disturbances.end(); ++it ) {
      MC2_TEST_CHECK( strncmp( "SRA", (*it)->getSituationReference().c_str(), 3 ) == 0 );
   }
   STLUtility::deleteValues( disturbances );
}
*/

MC2_UNIT_TEST_FUNCTION( sqlStringTest ) {
   using namespace SQLTrafficElementStrings;

   TrafficElementDatabase::TrafficElements elements;
   createFakeElements( elements );

   MC2_TEST_CHECK( createDeleteQuery( "FakeTable", elements ) == "DELETE FROM FakeTable WHERE situationReference IN('abc', 'def')" );

   MC2String query;
   createAddQueryDisturbance( elements.begin(), elements.end(), query, 800000 );
   MC2_TEST_CHECK ( query == "REPLACE INTO ISABDisturbance(disturbanceID, situationReference, type, phrase, eventCode, startTime, endTime, creationTime, severity, direction, text, deleted, firstLocation, secondLocation, extent, costFactor, queueLength) VALUES(10, 'abc', 0, 0, 1, 10, 20, 0, 0, 0, 'Description text', 0, 'first location', 'second location', 30, 4000, 50),(40, 'def', 1, 1, 6, 70, 80, 1, 1, 1, 'second Description text', 0, 'second first location', 'second second location', 90, 9900, 120)");   

   query.clear();
   createAddQueryCoords( elements.begin(), elements.end(), query, 800000 );
   MC2_TEST_CHECK( query == "REPLACE INTO ISABDisturbanceCoords(disturbanceID, situationReference, latitude, longitude, angle, routeIndex) VALUES(10, 'abc', 100, 200, 180, 3),(10, 'abc', 300, 400, 90, 4),(40, 'def', 500, 600, 270, 5),(40, 'def', 700, 800, 360, 6)");
   
   STLUtility::deleteValues( elements );
}


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

#include "FetchAllDisturbancesPacket.h"
#include "DisturbanceElement.h"
#include "TrafficDataTypes.h"
#include "DeleteHelpers.h"
#include "DisturbanceCompareSet.h"

void createFakeElements( vector<DisturbanceElement*>& elements) {
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

typedef STLUtility::AutoContainer
< FetchAllDisturbancesReplyPacket::Disturbances > ElementContainer;

MC2_UNIT_TEST_FUNCTION( fetchAllDisturbancesTest ) {

    MC2String provider( "test" );
    // test request
    FetchAllDisturbancesRequestPacket request( Packet::PacketID( 0 ),
                                               Packet::RequestID( 0 ),
                                               provider );
    MC2_TEST_REQUIRED( request.getProviderID() == provider );

    // create a set of valid elements
    ElementContainer inputElements;
    createFakeElements( inputElements );

    // test reply packet
    FetchAllDisturbancesReplyPacket reply( &request, inputElements );

    // fetch elements from reply and compare #-elements in the containers.
    ElementContainer outputElements;
    reply.getDisturbances( outputElements );
    MC2_TEST_REQUIRED( outputElements.size() == inputElements.size() );

    // compare element data too.
    compareSet( inputElements, outputElements );
}

// Tests a very large set of disturbances to be put in the reply packet
MC2_UNIT_TEST_FUNCTION( largeFetchAllDisturbancesTest ) {

   // push down a large set of disturbances in the reply and
   // make sure it does not crash
   ElementContainer inputElements;
   inputElements.resize( MAX_PACKET_SIZE );

   for ( size_t i = 0, n = inputElements.size(); i < n; ++i ) {
      inputElements[ i ] = new DisturbanceElement();
   }
   FetchAllDisturbancesRequestPacket request( Packet::PacketID( 0 ),
                                              Packet::RequestID( 0 ),
                                              "testprovider" );
   FetchAllDisturbancesReplyPacket reply( &request, inputElements );
   ElementContainer outputElements;
   reply.getDisturbances( outputElements );
   MC2_TEST_REQUIRED( outputElements.size() == inputElements.size() );
}

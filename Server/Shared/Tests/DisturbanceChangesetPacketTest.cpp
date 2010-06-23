/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbanceChangesetPacket.h"
#include "FetchAllDisturbancesPacket.h"

#include "DisturbanceElement.h"
#include "DisturbanceChangeset.h"

#include "MC2UnitTestMain.h"
#include "DeleteHelpers.h"
#include "DisturbanceCompareSet.h"

#include <iostream>
using namespace std;

typedef DisturbanceChangeset::Elements Elements;

void createUpdateSet( Elements& updateSet ) {
   updateSet.
      push_back( new DisturbanceElement( 0, // disturbance id
                                         // situation reference
                                         "abc",
                                         // disturbance type
                                         TrafficDataTypes::Accident,
                                         // phrase
                                         TrafficDataTypes::ABL,
                                         1, // event code
                                         10, // start time
                                         30, // end time
                                         20, // creation time
                                         // severity
                                         TrafficDataTypes::Blocked,
                                         // direction
                                         TrafficDataTypes::Positive,
                                         "first location",
                                         "second location",
                                         4, // extent
                                         5, // cost factor
                                         "description",
                                         10 // queue length
                                         ) );
   updateSet.
      push_back( new DisturbanceElement( 60, // disturbance id
                                         // situation reference
                                         "efg",
                                         // disturbance type
                                         TrafficDataTypes::SnowIceEquipment,
                                         // phrase
                                         TrafficDataTypes::APT,
                                         70, // event code
                                         65, // start time
                                         90, // end time
                                         85, // creation time
                                         // severity
                                         TrafficDataTypes::LongQueues,
                                         // direction
                                         TrafficDataTypes::Negative,
                                         "dist 2 first - location",
                                         "dist 2 second - location",
                                         95, // extent
                                         97, // cost factor
                                         "description",
                                         24 // queue length
                                         ) );

}

void createRemoveSet( Elements& removeSet ) {
  removeSet.
      push_back( new DisturbanceElement( 98, // disturbance id
                                         // situation reference
                                         "hijk",
                                         // disturbance type
                                         TrafficDataTypes::Carparks,
                                         // phrase
                                         TrafficDataTypes::AOV,
                                         170, // event code
                                         265, // start time
                                         390, // end time
                                         685, // creation time
                                         // severity
                                         TrafficDataTypes::HeavyTraffic,
                                         // direction
                                         TrafficDataTypes::BothDirections,
                                         "remove first - location",
                                         "remove 2 second - location",
                                         123, // extent
                                         997, // cost factor
                                         "description",
                                         224 // queue length
                                         ) );

}

MC2_UNIT_TEST_FUNCTION( testData ) {
   Elements updateSet;
   Elements removeSet;
   createUpdateSet( updateSet );
   createRemoveSet( removeSet );

   DisturbanceChangeset changes( updateSet, removeSet );
   DisturbanceChangesetRequestPacket packet( Packet::PacketID( 0 ),
                                             Packet::RequestID( 0 ),
                                             changes );

   DisturbanceChangeset readChanges;
   packet.getChangeset( readChanges );

   compareSet( readChanges.getUpdateSet(), changes.getUpdateSet() );
   compareSet( readChanges.getRemoveSet(), changes.getRemoveSet() );

}

MC2_UNIT_TEST_FUNCTION( largeDisturbanceChangeSetTest ) {

   Elements updateSet;
   Elements removeSet;
   updateSet.resize( MAX_PACKET_SIZE );
   removeSet.resize( MAX_PACKET_SIZE );

   for ( size_t i = 0, n = updateSet.size(); i < n; ++i ) {
      updateSet[ i ] = new DisturbanceElement();
      removeSet[ i ] = new DisturbanceElement();
   }
   DisturbanceChangeset changes( updateSet, removeSet );
   DisturbanceChangesetRequestPacket packet( Packet::PacketID( 0 ),
                                             Packet::RequestID( 0 ),
                                             changes );
   DisturbanceChangeset readChanges;
   packet.getChangeset( readChanges );

   MC2_TEST_REQUIRED( readChanges.getUpdateSet().size() == 
                      changes.getUpdateSet().size() );

   MC2_TEST_REQUIRED( readChanges.getRemoveSet().size() == 
                      changes.getRemoveSet().size() );

   STLUtility::deleteValues( updateSet );
   STLUtility::deleteValues( removeSet );   
}

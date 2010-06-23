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
#include "MapStatistics.h"
#include "Packet.h"

/** 
 * Tests construction.
 */
MC2_UNIT_TEST_FUNCTION( instantiateMapElement ) {
   MapElement me( 1, 2 );
   MC2_TEST_CHECK( me.getMapID() == 1 );
   MC2_TEST_CHECK( me.getMapSize() == 2 );
   MC2_TEST_CHECK( me.getStatus() == MapElement::LOADED );
}

/**
 * Tests the load and save functions.
 */
MC2_UNIT_TEST_FUNCTION( serializeMapElement ) {
   uint32 mapID = 0x17;
   uint32 mapSize = 100;
   MapElement::status_t status = MapElement::TOLD_TO_DELETE;

   MapElement me( mapID, mapSize );
   me.setStatus( status );
   Packet pack( 10000 );
   int pos = 0;
   me.save( &pack, pos );
   
   MapElement me2( 0, 0 );
   pos = 0;
   me2.load( &pack, pos );

   MC2_TEST_CHECK( me2.getMapID() == mapID );
   MC2_TEST_CHECK( me2.getMapSize() == mapSize );
   MC2_TEST_CHECK( me2.getStatus() == status );
}

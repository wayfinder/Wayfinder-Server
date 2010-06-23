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

#include "ExternalSearchRequestData.h"
#include "SearchRequestParameters.h"
#include "Packet.h"

MC2_UNIT_TEST_FUNCTION( externalSearhRequestDataTest ) {

   MC2Coordinate coord( 12345678, 12345 );
   SearchRequestParameters reqParams;
   ExternalSearchRequestData::stringMap_t stringMap;
   stringMap[7] = MC2String( "Doh!" );

   ExternalSearchRequestData reqData( reqParams,
                                      44,
                                      stringMap,
                                      5,
                                      10,
                                      ItemInfoEnums::OneSearch_All,
                                      coord,
                                      752 );


    // Check that sizes are correct
   const uint32 calculatedSize = reqData.getSizeInPacket();
   Packet pack( calculatedSize + HEADER_SIZE );
   int pos = 0;
   uint32 acctualSize = reqData.save( &pack, pos );

   MC2_TEST_REQUIRED_EXT( calculatedSize >= acctualSize,
                          "Calculated size less than written size" );

   ExternalSearchRequestData loadedData;
   pos = 0;
   uint32 loadedSize = loadedData.load( &pack, pos );

   MC2_TEST_REQUIRED_EXT( loadedSize == acctualSize,
                          "Size of read containers does not match written size");

   // Tests on some data members
   MC2_TEST_CHECK( loadedData.getCoordinate() == reqData.getCoordinate() );
   MC2_TEST_CHECK( loadedData.getValues() == reqData.getValues() );
   MC2_TEST_CHECK( loadedData.getService() == reqData.getService() );
   MC2_TEST_CHECK( loadedData.getNbrWantedHits() == reqData.getNbrWantedHits() );
   MC2_TEST_CHECK( loadedData.getInfoFilterLevel() == reqData.getInfoFilterLevel() );
}

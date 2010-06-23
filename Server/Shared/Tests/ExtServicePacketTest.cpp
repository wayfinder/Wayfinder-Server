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

#include "ExtServicePacket.h"

//
// Tests ExtServiceRequestPacket
//
MC2_UNIT_TEST_FUNCTION( extServiceRequestPacket ) {
   uint32 crc = 0xBEEFCACE;
   ExtServiceRequestPacket request( 0, // packet id
                                    0, // request id
                                    crc );
   // written crc must match
   MC2_TEST_CHECK( request.getCRC() == crc );
}

//
// Tests ExtServiceReplyPacket
//
MC2_UNIT_TEST_FUNCTION( extServiceReplyPacket ) {
   CompactSearchHitType hitTypes[] = {
      {
         "name", // name
         StringTable::OK, // nameStringCode
         "type", // type
         StringTable::NOTOK, // typeStringCode
         "image", // image name
         0, // round
         1, // service id
         2, // heading
         3, // search type mask
         4, // top region
         LangTypes::swedish, // language
         5, // map rights
         false, // inverted rights
      },
      {
         "a", // name
         StringTable::NOSTRING, // nameStringCode
         "b", // type
         StringTable::FERRYITEM, // typeStringCode
         "c", // image name
         6, // round
         7, // service id
         8, // heading
         9, // search type mask
         10, // top region
         LangTypes::danish, // language
         11, // map rights
         true, // inverted rights
      }
   }; 
   CompactSearchHitTypeVector writeContainer( hitTypes,
                                              hitTypes + 
                                              sizeof ( hitTypes ) /
                                              sizeof ( hitTypes[ 0 ] ) );
   uint32 crc = 0xBEEFCACE;
   ExtServiceRequestPacket request( 0, // packet id
                                    0, // request id
                                    crc );
   ExtServiceReplyPacket reply( request,
                                writeContainer,
                                crc );
   // written crc must matchx
   MC2_TEST_CHECK( crc == reply.getCRC() );

   CompactSearchHitTypeVector readContainer;
   reply.getHitTypes( readContainer );

   //
   // compare with original container
   //
   MC2_TEST_REQUIRED_EXT( readContainer.size() == writeContainer.size(),
                          "Size of read containers does not match written size");
   for ( uint32 i = 0; i < readContainer.size(); ++i ) {
      MC2_TEST_CHECK_EXT( readContainer[ i ] == writeContainer[ i ], i );
   }

}


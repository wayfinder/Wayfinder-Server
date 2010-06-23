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

#include "CompactSearchPacketTools.h"
#include "Packet.h"

MC2_UNIT_TEST_FUNCTION( compactSearchPacketTools ) {
   using namespace CompactSearchPacketTools;

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

   CompactSearchHitTypeVector container( hitTypes,
                                         hitTypes + 
                                         sizeof ( hitTypes ) /
                                         sizeof ( hitTypes[ 0 ] ) );

   const uint32 size = calcPacketSize( container );
   Packet pack( size + REQUEST_HEADER_SIZE );
   int pos = REQUEST_HEADER_SIZE;
   writeToPacket( pack, pos, container );
   MC2_TEST_REQUIRED_EXT( size + REQUEST_HEADER_SIZE == (uint32)pos,
                          "Calculated size does not match written size" );

   CompactSearchHitTypeVector readContainer;
   pos = REQUEST_HEADER_SIZE;
   readFromPacket( pack, pos, readContainer );

   MC2_TEST_REQUIRED_EXT( readContainer.size() == container.size(),
                          "Size of read containers does not match written size");

   for ( uint32 i = 0; i < readContainer.size(); ++i ) {
      MC2_TEST_CHECK_EXT( readContainer[ i ] == container[ i ], i );
   }

}

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

#include "DisturbancePacketUtility.h"
#include "DisturbanceChangeset.h"
#include "StringTable.h"

int calcPacketSize( const DisturbanceChangeset& changes ) {
   return DisturbancePacketUtility::calcPacketSize( REQUEST_HEADER_SIZE, 
                                                    changes.getUpdateSet() ) +
      DisturbancePacketUtility::calcPacketSize( REQUEST_HEADER_SIZE,
                                                changes.getRemoveSet() );
}

DisturbanceChangesetRequestPacket::
DisturbanceChangesetRequestPacket( PacketID pid, RequestID rid,
                                   const DisturbanceChangeset& changes ):
   RequestPacket( calcPacketSize( changes ),
                  0, // prio, not important
                  PACKETTYPE_DISTURBANCE_CHANGESET_REQUEST,
                  pid, // packet id
                  rid, // request id
                  Packet::MAX_MAP_ID ) {

   using namespace DisturbancePacketUtility;

   int position = REQUEST_HEADER_SIZE;

   writeToPacket( changes.getUpdateSet(),
                  this, position, // packet and position
                  false, // don't remove disturbances ( not actually used )
                  false ); // remove all, ( not actually used )

   writeToPacket( changes.getRemoveSet(),
                  this, position, // packet and position
                  false, // don't remove disturbances ( not actually used )
                  false ); // remove all, ( not actually used )

   setLength( position );
}

void DisturbanceChangesetRequestPacket::
getChangeset( DisturbanceChangeset& changes ) const {
   using namespace DisturbancePacketUtility;
   bool notUsedRemoveDist; // dummy variable, not used
   bool notUsedRemoveAll; // dummy variable, not used.

   int position = REQUEST_HEADER_SIZE;

   // load update set
   DisturbanceChangeset::Elements updateSet;
   getDisturbances( updateSet,
                    this, position,
                    notUsedRemoveDist, notUsedRemoveAll );
   changes.swapUpdateSet( updateSet );

   // load remove set
   DisturbanceChangeset::Elements removeSet;
   getDisturbances( removeSet,
                    this, position,
                    notUsedRemoveDist, notUsedRemoveAll );
   changes.swapRemoveSet( removeSet );
}

DisturbanceChangesetReplyPacket::
DisturbanceChangesetReplyPacket( const RequestPacket* request,
                                 Status updateStatus,
                                 Status removeStatus ):
   ReplyPacket( REPLY_HEADER_SIZE + 2 * 4, // 2 extra status
                PACKETTYPE_DISTURBANCE_CHANGESET_REPLY,
                request,
                // The main status is only OK if both status are OK.
                ( updateStatus == StringTable::NOTOK ||
                  removeStatus == StringTable::NOTOK ) ?
                StringTable::NOTOK : StringTable::OK ) {

   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, updateStatus );
   incWriteLong( pos, removeStatus );

   setLength( REPLY_HEADER_SIZE + 2 * 4 ); // 2 extra status
}


DisturbanceChangesetReplyPacket::Status
DisturbanceChangesetReplyPacket::getUpdateStatus() const {
   return readLong( REPLY_HEADER_SIZE );
}

DisturbanceChangesetReplyPacket::Status
DisturbanceChangesetReplyPacket::getRemoveStatus() const {
   return readLong( REPLY_HEADER_SIZE + 4 );
}

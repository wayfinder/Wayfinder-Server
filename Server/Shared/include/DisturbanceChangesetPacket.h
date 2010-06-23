/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCE_CHANGESET_PACKET_HH
#define DISTURBANCE_CHANGESET_PACKET_HH

#include "Packet.h"

#include <vector>

class DisturbanceChangeset;

/**
 * Requests an update of the database for new,updated and/or removed set of
 * disturbances.
 */
class DisturbanceChangesetRequestPacket: public RequestPacket {
public:

   /**
    * @param pid Packet ID.
    * @param rid Request ID.
    * @param changes The set of elements to be changed.
    */
   explicit DisturbanceChangesetRequestPacket( PacketID pid,
                                               RequestID rid,
                                               const DisturbanceChangeset&
                                               changes );

   /**
    * Fetch update, add and remove set from packet.
    * @param updateSet The set of elements to be updated or added to the
    *                  database.
    * @param removeSet The set of elements to be removed from the database.
    */
   void getChangeset( DisturbanceChangeset& changes ) const;
};

/**
 * Returns status on update and remove set.
 * The status on this packet is \c OK if both update and remove status are \c
 * OK. If either of the two are \c NOTOK the status will be \c NOTOK.
 */
class DisturbanceChangesetReplyPacket: public ReplyPacket {
public:
   typedef uint32 Status;

   /** Setup status on reply. 
    * @param updateStatus Whether or not the update set was successfully
    *                     updated in the database.
    * @param removeStatus Whether or not the removeSet was successfully removed
    *                     from the database.
    */
   DisturbanceChangesetReplyPacket( const RequestPacket* request,
                                    Status updateStatus,
                                    Status removeStatus );

   /// @return Status of the update set changes on the database.
   Status getUpdateStatus() const;
   /// @return Status of the remove set changes on the database.
   Status getRemoveStatus() const;
};

#endif // DISTURBANCE_CHANGESET_PACKET_HH

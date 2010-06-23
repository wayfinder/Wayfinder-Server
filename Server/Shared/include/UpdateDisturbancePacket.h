/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UPDATEDISTURBANCEPACKET_H
#define UPDATEDISTURBANCEPACKET_H

#include "config.h"
#include "Packet.h"
#include "TrafficDataTypes.h"
#include "DisturbanceElement.h"
#include <vector>
#include "MC2String.h"

/**
 *   Request Packet used for updating Disturbances.
 */

class UpdateDisturbanceRequestPacket : public RequestPacket
{
  public:

   /** Constructor.
    *  @param packetID  The ID of the packet.
    *  @param requestID The ID of the request.
    *  @param type      The new disturbance type.
    *  @param severity The new severity.
    *  @param startTime The new disturbance start time.
    *  @param endTime   The new disturbance end time.
    *  @param creationTime The time of the update.
    *  @param situationID The situationID for the updated disturbance.
    *  @param elementID   The elementID for the updated disturbance.
    *  @param mapID     The mapID, where the disturbance is situated.
    *  @param comment   The new disturbance comment.
    *  @param lastEditUser The user name of the editor.
    */
   UpdateDisturbanceRequestPacket(uint16 packetID,
                                  uint32 requestID,
                                  uint32 disturbanceID,
                                  TrafficDataTypes::disturbanceType type,
                                  TrafficDataTypes::severity severity,
                                  uint32 startTime,
                                  uint32 endTime,
                                  uint32 costFactor,
                                  MC2String text);

   /**
    *  Destructor.
    */
   virtual ~UpdateDisturbanceRequestPacket() {};

   /**
    *  Returns all the new parameters.
    *  @param type       The new disturbance type.
    *  @param startTime  The new disturbance start time.
    *  @param endTime    The new disturbance end time.
    *  @param creationTime The time of the update.
    *  @param situationID The situationID for the updated disturbance.
    *  @param elementID   The elementID for the updated disturbance.
    *  @param mapID     The mapID, where the disturbance is situated.
    *  @param comment   The new disturbance comment.
    *  @param lastEditUser The user name of the editor.
    */
   void getUpdateData(uint32 &disturbanceID,
                      TrafficDataTypes::disturbanceType &type,
                      TrafficDataTypes::severity &severity,
                      uint32 &startTime,
                      uint32 &endTime,
                      uint32 &costFactor,
                      MC2String &text) const;
   
};

/**
 *  Reply packet used for updating disturbances.
 */
class UpdateDisturbanceReplyPacket : public ReplyPacket
{
  public:

   /**
    *  Constructor..
    *  @param p  The requestPacket.
    *  @param status The status.
    */
   UpdateDisturbanceReplyPacket(const RequestPacket* p,
                                uint32 status );

   /**
    *  Destructor.
    */
   virtual ~UpdateDisturbanceReplyPacket();   
};

#endif

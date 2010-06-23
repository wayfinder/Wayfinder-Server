/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALIVEPACKET_H
#define ALIVEPACKET_H

#include "config.h"
#include "Packet.h"
#include "ModuleTypes.h"

// The priorities will not really be used, since this packet
// should be handled by the Reader.
#define ALIVE_REQUEST_PRIO DEFAULT_PACKET_PRIO 
#define ALIVE_REPLY_PRIO   DEFAULT_PACKET_PRIO

//-----------------------------------------------------------------
// AliveRequestPacket
//-----------------------------------------------------------------

/**
 *   Packet to be sent to a module and check if it is there.
 *   To be used in requests that can work with or without a certain
 *   module. For this to work, there has to be a module that answers
 *   the packets, but of a different type.
 *   After the general RequestPacket-header the format is
 *   @packetdesc
 *   @row REQUEST_HEADER_SIZE @sep 4 bytes @sep Wanted module type @endrow
 *   @endpacketdesc
 *   <br>
 */
class AliveRequestPacket : public RequestPacket {
public:

   /**
    *   Creates a new AliveRequestPacket. Map ID will be set
    *   to MAX_UINT32. This packet should be handled by the Reader.
    *   @param moduleType The module type that is to be checked.
    *   @param packetID   The packet id of the packet. To be used by
    *                     the server.
    *   @param requestID  The request id of the packet. Used by server.
    */
   AliveRequestPacket(moduletype_t moduleType,
                      uint32 packetID  = MAX_UINT32,
                      uint32 requestID = MAX_UINT32);


   /**
    *   The wanted type of module for the packet.
    *   @return The wanted module type of the packet.
    */
   moduletype_t getWantedModuleType() const;
   
private:
   /**   The position of the requested module type  variable */
   static const int REQUESTED_MODULE_POS = REQUEST_HEADER_SIZE;
};

//-----------------------------------------------------------------
// AliveReplyPacket
//-----------------------------------------------------------------

/**
 *  The answer to an AliveRequestPacket.
 *  Contains the wanted module type and the type of module that answers.
 *  The status is set to OK if the wanted type and the found type are the
 *  same.
 *   <br>
 *   After the general ReplyPacket-header the format is
 *   @packetdesc
 *   @row REPLY_HEADER_SIZE @sep 4 bytes @sep Wanted module type @endrow
 *   @row REPLY_HEADER_SIZE + 4  @sep 4 bytes @sep Answering module type
 *                                                   @endrow
 *   @endpacketdesc
 *   <br>
 */
class AliveReplyPacket : public ReplyPacket {
public:

   /**
    *   Creates a new AliveReplyPacket from the requestpacket
    *   and the current module type.
    *   @param req The AliveRequestPacket that caused the reply.
    *   @param myModuleType The type of module that answered.
    */
   AliveReplyPacket(const AliveRequestPacket *req,
                    moduletype_t myModuleType);
                    

   /**
    *   The wanted type of module for the packet.
    *   @return The wanted module type of the packet.
    */
   moduletype_t getWantedModuleType() const;

   /**
    *   The type of module that answered.
    *   @return The type of module that answered.
    */
   moduletype_t getAnsweringModuleType() const;
   
private:
   /**   The position of the requested module type  variable */
   static const int REQUESTED_MODULE_POS = REQUEST_HEADER_SIZE;

   /**   The position of the answering module type  variable */
   static const int ANSWERING_MODULE_POS = REQUESTED_MODULE_POS + 4;
};

#endif

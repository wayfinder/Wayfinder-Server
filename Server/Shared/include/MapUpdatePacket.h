/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ADDMAPUPDATEPACKET_H
#define ADDMAPUPDATEPACKET_H

#include "config.h"
#include "Packet.h"

#define SQL_OPERATION_TYPE_POS  (REQUEST_HEADER_SIZE)
#define FIST_DBASEOBJECT_POS    (REQUEST_HEADER_SIZE + 4)

/**
 *    This packet is used to send information about a possible error in 
 *    the map to the info-module. This is a quite informal description that
 *    is inserted into the database. The information is not used until
 *    it is post-processed and inserted to the maps by a human.
 *
 *    The packet contans a MapUpdateDBaseObject after the standard
 *    RequestPacket-header.
 *   
 *    @see MapUpdateDBaseObject
 */
class MapUpdateRequestPacket : public RequestPacket {
   public:
      
      enum  sqloperation_t {ADD, UPDATE};
      
      /**
       *    Default constructor.
       */
      MapUpdateRequestPacket();

      /**
       *    Constuctor that sets the IDs in the packet.
       *    @param reqID      The ID of the request that this packet comes 
       *                      from.
       *    @param packetID   The ID of this packet.
       */
      MapUpdateRequestPacket(uint32 reqID, uint32 packetID);

      /**
       *    Set the operation to perform on the sql-database.
       */
      inline void setSQLOperation(sqloperation_t op);

      /**
       *    Get the operation to perform on the sql-database.
       */
      inline sqloperation_t getSQLOperation() const;

      /**
       *    Get the first position of the next DBase-object that should
       *    be stored in this packet.
       *    @return  The first position of the next DBase-object in this
       *             packet.
       */
      inline int getNextDBaseObjectPosition() const;

   private:
      /**
        *   Reset the packet.
        */
      void initEmptyPacket();

};

class StringCode;
/**
 *    The reply to a MapUpdateRequestPacket. This packet contains
 *    nothing but the status of the update-operation.
 *
 */
class MapUpdateReplyPacket : public ReplyPacket {
   public:
      /**
        *   Create an empty MapUpdateReplyPacket.
        */
      MapUpdateReplyPacket();

      /**
       *    Create an MapUpdateReplyPacket from the corresponding
       *    request packet.
       *    @param req     The MapUpdateRequestPacket that this is
       *                   a reply to.
       *    @param status  The status of the update-operation.
       */
      MapUpdateReplyPacket(const MapUpdateRequestPacket* req,
                           StringCode status);
};

// ========================================================================
//                                      Implementation of inlined methods =

inline void 
MapUpdateRequestPacket::setSQLOperation(sqloperation_t op)
{
   writeLong(SQL_OPERATION_TYPE_POS, op);
}

inline MapUpdateRequestPacket::sqloperation_t 
MapUpdateRequestPacket::getSQLOperation() const
{
   return sqloperation_t(readLong(SQL_OPERATION_TYPE_POS));

}

   
inline int 
MapUpdateRequestPacket::getNextDBaseObjectPosition() const
{
   return (FIST_DBASEOBJECT_POS);
}


#endif


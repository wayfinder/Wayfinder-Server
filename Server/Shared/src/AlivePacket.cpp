/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AlivePacket.h"
#include "StringTable.h"

// Packet max lengths
#define ALIVE_REQUEST_MAX_LENGTH 256
#define ALIVE_REPLY_MAX_LENGTH   256

//-----------------------------------------------------------------
// AliveRequestPacket
//-----------------------------------------------------------------

AliveRequestPacket::AliveRequestPacket(moduletype_t moduleType,
                                       uint32 packetID,
                                       uint32 requestID)
      : RequestPacket( ALIVE_REQUEST_MAX_LENGTH,
                       ALIVE_REQUEST_PRIO,
                       Packet::PACKETTYPE_ALIVEREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32 /* mapid */) 
{
   int pos = REQUESTED_MODULE_POS;
   incWriteLong( pos, moduleType );
   setLength(pos);   
}


moduletype_t 
AliveRequestPacket::getWantedModuleType() const
{
   return moduletype_t(readLong( REQUESTED_MODULE_POS ));
}


//-----------------------------------------------------------------
// AliveReplyPacket
//-----------------------------------------------------------------

AliveReplyPacket::AliveReplyPacket(const AliveRequestPacket *req,
                                   moduletype_t myModuleType)
      : ReplyPacket(ALIVE_REPLY_MAX_LENGTH,
                    Packet::PACKETTYPE_ALIVEREPLY,
                    req,
                    StringTable::OK)
{
   int pos = REQUESTED_MODULE_POS;
   incWriteLong(pos, req->getWantedModuleType());
   incWriteLong(pos, myModuleType);
   setLength(pos);
   
   if ( req->getWantedModuleType() == myModuleType ) {
      setStatus(StringTable::OK);
   } else {
      setStatus(StringTable::NOTOK);
   }
}
                    
moduletype_t 
AliveReplyPacket::getWantedModuleType() const
{
   return moduletype_t(readLong( REQUESTED_MODULE_POS ));
}

moduletype_t
AliveReplyPacket::getAnsweringModuleType() const
{
   return moduletype_t(readLong( ANSWERING_MODULE_POS ));
}


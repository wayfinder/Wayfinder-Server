/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GetUserTrackRequest.h"
#include "GetUserTrackPacket.h"

GetUserTrackRequest::GetUserTrackRequest(uint32 reqID, uint32 UIN) 
   : RequestWithStatus(reqID), m_state(INIT), 
     m_startTime(MAX_UINT32), m_endTime(MAX_UINT32),
     m_maxNbrHits(MAX_UINT32), m_UIN(UIN)
{ 

}

void 
GetUserTrackRequest::setTimeInterval(uint32 startTime, uint32 endTime) 
{
   m_startTime = startTime;
   m_endTime = endTime;
}

void 
GetUserTrackRequest::setMaxNbrHits(uint32 n)
{
   m_maxNbrHits = n;
}

const UserTrackElementsList&
GetUserTrackRequest::getResult() 
{ 
   return m_resultElements;
}

PacketContainer* 
GetUserTrackRequest::getNextPacket() 
{
   if (m_state == INIT) {
      GetUserTrackRequestPacket* p = 
            new GetUserTrackRequestPacket(getNextPacketID(), getID(), m_UIN);
      p->setInterval(m_startTime, m_endTime);
      p->setMaxNbrHits(m_maxNbrHits);
      PacketContainer* pc = new PacketContainer(p);
      pc->setModuleType(MODULE_TYPE_USER);
      m_packetsReadyToSend.add(pc);
      m_state = GET_USER_TRACK_PACKET;
   }

   return Request::getNextPacket();
}

void 
GetUserTrackRequest::processPacket(PacketContainer* pack) 
{
   switch (m_state) {
      case INIT:
         mc2log << error << "GetUserTrackRequest: Got packet in INIT" << endl;
         break;
      case GET_USER_TRACK_PACKET : {
         GetUserTrackReplyPacket* p = 
            static_cast<GetUserTrackReplyPacket*>(pack->getPacket());
         if (p->getStatus() == StringTable::OK) {
            p->getUserTrackElements(m_resultElements);
            m_state = DONE;
            setDone(true);
         } else {
            m_state = ERROR;
         }
         } break;
      case DONE:
         mc2log << error << "GetUserTrackRequest: Got packet in DONE" << endl;
         break;
      case ERROR:
         mc2log << error << "GetUserTrackRequest: Got packet in ERROR" << endl;
         break;
   }

   delete pack;
}

StringTable::stringCode 
GetUserTrackRequest::getStatus() const 
{
   switch (m_state) {
      case INIT:
         return StringTable::UNKNOWN;
      case GET_USER_TRACK_PACKET:
         return StringTable::TIMEOUT_ERROR;
      case DONE:
         return StringTable::OK;
      case ERROR:
         return StringTable::INTERNAL_SERVER_ERROR;
   }
   // Keep compiler happy ;-)
   return StringTable::INTERNAL_SERVER_ERROR;
}


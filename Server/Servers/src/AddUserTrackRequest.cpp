/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AddUserTrackRequest.h"
#include "AddUserTrackPacket.h"

AddUserTrackRequest::AddUserTrackRequest(uint32 reqID, uint32 UIN) 
   : RequestWithStatus(reqID), m_state(INIT), m_UIN(UIN) 
{ 

}

void 
AddUserTrackRequest::addUserTrackElement(UserTrackElement* elm) 
{
   m_sendElements.push_back(elm);
}

PacketContainer* 
AddUserTrackRequest::getNextPacket() 
{
   if (m_state == INIT) {
      if (m_sendElements.size() == 0) {
         m_state = ERROR;
      } else {
         AddUserTrackRequestPacket* p = 
               new AddUserTrackRequestPacket(getNextPacketID(), 
                                             getID(), m_UIN);
         p->setUserTrackElements(m_sendElements);
         PacketContainer* pc = new PacketContainer(p);
         pc->setModuleType(MODULE_TYPE_USER);
         m_packetsReadyToSend.add(pc);
         m_state = ADD_USER_TRACK_PACKET;
      }
   }

   return Request::getNextPacket();
}

void 
AddUserTrackRequest::processPacket(PacketContainer* pack) 
{
   switch (m_state) {
      case INIT:
         mc2log << error << "AddUserTrackRequest: Got packet in INIT" << endl;
         break;
      case ADD_USER_TRACK_PACKET : {
         AddUserTrackReplyPacket* p = 
            static_cast<AddUserTrackReplyPacket*>(pack->getPacket());
         if (p->getStatus() == StringTable::OK) {
            m_state = DONE;
            setDone(true);
         } else {
            m_state = ERROR;
         }
         } break;
      case DONE:
         mc2log << error << "AddUserTrackRequest: Got packet in DONE" << endl;
         break;
      case ERROR:
         mc2log << error << "AddUserTrackRequest: Got packet in ERROR" << endl;
         break;
   }

   delete pack;
}

StringTable::stringCode 
AddUserTrackRequest::getStatus() const 
{
   switch (m_state) {
      case INIT:
         return StringTable::UNKNOWN;
      case ADD_USER_TRACK_PACKET:
         return StringTable::TIMEOUT_ERROR;
      case DONE:
         return StringTable::OK;
      case ERROR:
         return StringTable::INTERNAL_SERVER_ERROR;
   }
   // Keep compiler happy ;-)
   return StringTable::INTERNAL_SERVER_ERROR;
}


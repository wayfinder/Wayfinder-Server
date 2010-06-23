/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SinglePacketRequest.h"
#include "Packet.h"
#include <typeinfo>


SinglePacketRequest::SinglePacketRequest( uint16 reqID, 
                                          PacketContainer* cont ) 
   : Request( reqID ),
     m_cont( cont )
{
   m_answer = NULL;
   if ( cont != NULL ) {
      m_name = MC2String("SPR: ") + cont->getPacket()->getSubTypeAsString();
      m_packetID = getNextPacketID();
      cont->getPacket()->setRequestID( getID() );
      cont->getPacket()->setPacketID( m_packetID );      
   } else {
      m_name = "SinglePacketReq(NULL)";
      m_done = true;
   }
}


SinglePacketRequest::~SinglePacketRequest() {
   delete m_answer;
}


PacketContainer*
SinglePacketRequest::getNextPacket() {
   PacketContainer* container = m_cont;
   m_cont = NULL;
   return container;
}


void
SinglePacketRequest::processPacket( PacketContainer* cont ) {
   if ( cont != NULL ) {
      if ( cont->getPacket()->getPacketID() == m_packetID ) {
         DEBUG4( cerr << "SinglePacketRequest::processPacket "
                          << "answer received" << endl; );
         m_answer = cont;
         m_done = true;
      } else {
         DEBUG4( cerr << "SinglePacketRequest::processPacket "
                          << "odd packet received" << endl; );
         cont->getPacket()->dump();
         delete cont;
         m_done = true;
      }
   }
}


PacketContainer*
SinglePacketRequest::getAnswer() {
   PacketContainer* cont = m_answer;
   m_answer = NULL;
   return cont;
}


const char*
SinglePacketRequest::getName() const
{
   return m_name.c_str();
}

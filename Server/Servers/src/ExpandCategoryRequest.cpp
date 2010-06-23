/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandCategoryRequest.h"

#if 0
ExpandCategoryRequest::ExpandCategoryRequest( uint16 reqID, 
                                              ExpandCategorySearchRequestPacket* p) 
      : Request(reqID)
{
   cout << "ExpandCategoryRequest created" << endl;
   p->setRequestID(reqID);
   m_packetID = getNextPacketID();
   p->setPacketID( m_packetID  );
   m_request = new PacketContainer( p,
                                    0,
                                    0,
                                    MODULE_TYPE_MAP );
   m_answer = NULL;
   m_done = false;
}



PacketContainer* ExpandCategoryRequest::getNextPacket()
{
   DEBUG8(cerr << "getNextPacket sending " << m_request << endl;);
   PacketContainer* container = m_request;
   m_request = NULL;
   return container;
}

void ExpandCategoryRequest::processPacket( PacketContainer* pack )
{
   DEBUG8(cerr << "processPacket " << endl;);
	m_answer = pack;
   if ( m_answer->getPacket()->getPacketID() == m_packetID ) {
      m_done = true;
   }
   else {
      delete m_answer;
      m_answer = NULL;
   }
}

PacketContainer* ExpandCategoryRequest::getAnswer() {
	return m_answer;
}
#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AllUserRequest.h"
#include "UserConstants.h"

AllUserRequest::AllUserRequest( uint16 reqID,
                                FindUserRequestPacket* p ) : Request(reqID){
   p->setRequestID(reqID);
   m_currentPacketID = getNextPacketID();
   p->setPacketID(m_currentPacketID);
   m_request = new  PacketContainer( p,
                                     0,
                                     0,
                                     MODULE_TYPE_USER );
   m_answer = NULL;
   m_done = false;

   m_userReply = NULL;
   
   m_userVect = NULL;
   m_userIndex = 0;

   m_state = FIND_USER_SENT;
}

PacketContainer* AllUserRequest::getNextPacket() 
{
   if(m_done) {
      return NULL;
   }
   else {
      PacketContainer* temp = m_request;
      m_request = NULL;
      return temp;
   }
}

void AllUserRequest::processPacket(PacketContainer *pack) 
{
   if(pack != NULL){
      Packet* reply = pack->getPacket();
      if( reply != NULL ){
         if(reply->getPacketID() == m_currentPacketID ){
            switch(m_state){
               case FIND_USER_SENT:{
                  cout << "Got a find user reply" << endl;
                  reply->dump();
                  m_userReply = static_cast<FindUserReplyPacket*>(reply);
                  if( m_userReply->getStatus() == StringTable::OK ){
                     m_currentPacketID = getNextPacketID();
                     m_request = getNextUserPacketContainer(NULL);
                     m_state = USER_REQUEST_SENT;
                  }
                  else{
                     cout << "Status not OK after find user reply " << endl; 
                     m_done = true;
                  }
                  break;
               }
               case USER_REQUEST_SENT:{
                  cout << "Got a user reply" << endl;                  
                  GetUserDataReplyPacket* replyPacket =
                     static_cast<GetUserDataReplyPacket*>(pack->getPacket());
                  m_request =  getNextUserPacketContainer( replyPacket );
                  if( m_request == NULL)
                     m_done = true;
                  break;
               }
               default:
                  MC2ERROR("ERROR: AllUserRequest::getNextPacket()"
                           "case statement that shouldn't be reached");
                  break;
            }
         }
      }
   }
}


PacketContainer* AllUserRequest::getAnswer() 
{
   return m_answer;
}

PacketContainer*
AllUserRequest::getNextUserPacketContainer( GetUserDataReplyPacket* p )
{
   //cout << "--------------- Getting a user packet --------------" << endl;
   if( p == NULL ){ // First time init variables
      cout << "First time initalizing the variables" << endl;
      m_userIndex = 0;
      cout << "Nbr users " << m_userReply->getNbrUsers() << endl;
      m_userVect = new GetUserDataReplyPacket*[m_userReply->getNbrUsers()];
      m_UI = m_userReply->getUINs();
   }
   else{
      //cout << "Adding a packet to the vector!" << endl;
      m_userVect[m_userIndex-1]=p;
   }

   if( m_userIndex < m_userReply->getNbrUsers() ){
      //cout << "Trying " << m_userIndex << " of " << m_userReply->getNbrUsers() << endl;
      m_currentPacketID = getNextPacketID();
      GetUserDataRequestPacket* request 
         = new GetUserDataRequestPacket( m_currentPacketID,
                                         getID(),
                                         m_UI[m_userIndex],
                                         UserConstants::TYPE_ALL );
      cout << "Sending " << m_UI[m_userIndex] << endl;
      m_userIndex++;
      return new PacketContainer( request,
                                  0,
                                  0,
                                  MODULE_TYPE_USER );
         
   }
   else{
      cout << "Done returning NULL" << endl;
      return NULL; 
   }
}




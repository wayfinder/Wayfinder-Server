/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "NavMessage.h"
#include "NameUtility.h"

uint8 NavMessage::MAX_PROTOVER = 0x09;


NavMessage::NavMessage(const NavAddress& address,
                       NavMessage::MessageType messType,
                       bool isRequest,
                       NavSession * session) : m_type(messType),
                                               m_address(address),
                                               m_isRequest(isRequest),
                                               m_session(session),
                                               m_validCoordinates(false)
{
   m_protoVer = 0;
   m_length = 0;
   m_requestID = 0;
   m_crc = 0;
   m_cookie = NULL;
   m_navID = 0;
   m_userID = 0;
   for ( uint32 i = 0 ; i < 20 ; ++i ) {
      m_userName[ i ] = 0;
   }
}



NavMessage::~NavMessage()
{
   /* Nothing is newed */
}

NavMessage::MessageType
NavMessage::getType() const
{
   return m_type;
}

void
NavMessage::setType(uint16 type)
{
   m_type = NavMessage::MessageType(type);
}

const NavAddress*
NavMessage::getAddress() const
{
   return &m_address;
}

bool
NavMessage::isRequest() const
{
   return m_isRequest;
}

uint8
NavMessage::getProtoVer() const
{
   return m_protoVer;
}

void
NavMessage::setProtoVer(uint8 protoVer)
{
   m_protoVer = protoVer;
}

uint32
NavMessage::getLength() const
{
   return m_length;
}

void
NavMessage::setLength(uint32 length)
{
   m_length = length;
}


uint8
NavMessage::getReqID() const
{
   return m_requestID;
}

void
NavMessage::setReqID(uint8 id)
{
   m_requestID = id;
}

uint32
NavMessage::getCRC() const
{
   return m_crc;
}

void
NavMessage::setCRC(uint32 crc)
{
   m_crc = crc;
}

void*
NavMessage::getCookie() const
{
   return m_cookie;
}

NavSession *
NavMessage::getSession() const
{
   return m_session;
}

void
NavMessage::setCookie(void* cookie)
{
   m_cookie = cookie;
}


void
NavMessage::setNavID(uint32 navID)
{
   m_navID = navID;
}

uint32
NavMessage::getNavID() const
{
   return m_navID;
}

void
NavMessage::setUserID(uint32 userID)
{
   m_userID = userID;
}
   
uint32
NavMessage::getUserID() const
{
   return m_userID;
}

void
NavMessage::setUserName(const byte* userName)
{ 
   memcpy(m_userName,userName,20);   
}

char*
NavMessage::getUserName()
{
   return m_userName;
}

NavMessage::MessageType 
NavMessage::replyType( MessageType request ) {
   return MessageType( request + 1 );
}

/*------------------------------------------------------------------*/

OutGoingNavMessage::OutGoingNavMessage(const NavAddress& recipientAddress,
                                       NavMessage::MessageType type,
                                       bool isRequest,
                                       NavSession * session)
      : NavMessage(recipientAddress, type, isRequest, session),
        m_status( NAV_STATUS_OK )
{
}

OutGoingNavMessage::~OutGoingNavMessage()
{
}


void 
OutGoingNavMessage::setStatus( uint8 status ) {
   m_status = status;
}


uint8 
OutGoingNavMessage::getStatus() const {
   return m_status;
}


uint32 
OutGoingNavMessage::getReplySize() const {
   return 0;
}


/*------------------------------------------------------------------*/

IncomingNavMessage::IncomingNavMessage(const NavAddress& senderAddress,
                                       NavMessage::MessageType type,
                                       bool isRequest,
                                       NavSession * session)
      : NavMessage(senderAddress, type, isRequest, session)
{
}

IncomingNavMessage::~IncomingNavMessage()
{
}

bool
IncomingNavMessage::convertFromBytes(const byte* buf,
                                     int bufSize)
{
   // Dummt implementation
   return true;
}

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
#include "isabBoxSession.h"
#include "UserData.h"


isabBoxSession::isabBoxSession()
{
   const char randChars[80] = 
             "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   m_inSession = false;
   m_dataSocketDone = false;
   m_user = NULL;
   m_peerIP = 0;
   strcpy(m_logPrefix, "[XXX][NA;NA] ");
   m_logPrefix[1] = randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))];
   m_logPrefix[2] = randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))];
   m_logPrefix[3] = randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))];
}


isabBoxSession::~isabBoxSession()
{
   delete m_user;
}


bool 
isabBoxSession::connected()
{
   return m_inSession;
}


void 
isabBoxSession::setSocketBusy()
{
   mc2dbg2 << " Session busy " << endl;
   m_dataSocketDone=false;
}


void 
isabBoxSession::beginSession()
{
   mc2dbg2 << " Session entered " << endl;

   /* This is done just in case the session is not already cleared */
   clearSessionState();

   m_inSession=true;
}

uint8 
isabBoxSession::getDownloadAdditionalRouteData()
{
   return 0x1;
}

void 
isabBoxSession::clearSessionState() {
   // Call super
   NavSession::clearSessionState();

   m_dataSocketDone = true;
   m_inSession = false;
   
   delete m_user;
   m_user = NULL;
   m_peerIP = 0;
}


UserItem* 
isabBoxSession::getUser() {
   return m_user;
}


void 
isabBoxSession::setUser( UserItem* user ) {
   uint32 lastUserNameLen = 5;
   if ( m_user != NULL ) {
      lastUserNameLen = strlen( m_user->getUser()->getLogonID() ) + 3; 
   }
   delete m_user;
   m_user = user;
   m_logPrefix[strlen(m_logPrefix) - lastUserNameLen] = '\0';
   strcat(m_logPrefix, ";");
   
   StringUtility::strlcat( m_logPrefix, 
           m_user != NULL ? m_user->getUser()->getLogonID() : "NA", 
           45);
   strcat(m_logPrefix, "] ");
}


uint32 
isabBoxSession::getPeerIP() const {
   return m_peerIP;
}

IPnPort
isabBoxSession::getPeerIPnPort() const {
   return IPnPort( m_peerIP, m_peerPort );
}


void 
isabBoxSession::setPeerIP( uint32 IP, uint16 port ) {
   if ( m_peerIP == 0 ) {
      snprintf(m_logPrefix + 6, 45, "%d.%d.%d.%d:%d;NA] ", 
               int((IP & 0xff000000) >> 24), int((IP & 0x00ff0000) >> 16), 
               int((IP & 0x0000ff00) >>  8), int (IP & 0x000000ff), port);
   }
   m_peerIP = IP;
   m_peerPort = port;
}

void 
isabBoxSession::dump( ostream& out ) {
   out << "isabBoxSession " << endl
       << " m_dataSocketDone " << m_dataSocketDone << endl
       << " m_inSession " << m_inSession << endl;
   out << " m_user " << m_user << endl
       << " m_peerIP " << m_peerIP << endl;
}


const MC2String&
isabBoxSession::getSessionKey() const {
   return m_sessionKey;
}


void
isabBoxSession::setSessionKey( const MC2String& key ) {
   m_sessionKey = key;
}

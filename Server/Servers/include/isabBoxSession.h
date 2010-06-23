/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABBOXSESSION_H
#define ISABBOXSESSION_H

#include "config.h"
#include "NavSession.h"
#include "ISABThread.h"
#include <map>
#include "ImageDrawConfig.h"
#include "MC2String.h"
#include "IPnPort.h"

class UserItem;


/**
 *   Class that handles session management when communicating with a 
 *   navigator.
 *
 *   This class provides a place to store data during a session.
 *
 */
class isabBoxSession : public NavSession {

public:

   /**
    *  Creates a new isabBoxNavComm
    */
   isabBoxSession();
   
   /** 
    *   Destructor. 
    */
   virtual ~isabBoxSession();

   /**
    *   Are we conected?
    *   @return bool connected.
    */
   virtual bool connected();
   
   /** 
    *   The data socket is busy. Implies we can not answer
    *   new sessions.
    */
   void setSocketBusy();

   /** 
    *   Begin a new session. Implies a call to endSession().
    */
   void beginSession();

   /**
    *   Set the download additional route data variable.
    */
   uint8 getDownloadAdditionalRouteData();
 
   /**
    * Get the user assosiated with this session, is initaly NULL.
    */
   UserItem* getUser();

   /**
    * Set the user assosiated with this session.
    */
   void setUser( UserItem* user );

   /**
    * Get the other ends IP.
    */
   uint32 getPeerIP() const;

   /**
    * Get the other ends ip and port.
    */
   IPnPort getPeerIPnPort() const;

   /**
    * Set the other ends IP.
    */
   void setPeerIP( uint32 IP, uint16 port );

   /**
    * Dumps the session data to the ostream.
    */
   virtual void dump( ostream& out );

   /**
    *   Logging prefix used to identify this session in the log.
    */
   char m_logPrefix[100];


   /**
    * Get the current sessionKey.
    */
   const MC2String& getSessionKey() const;


   /**
    * Set the current sessionKey.
    */
   void setSessionKey( const MC2String& key );


protected:
   /**
    *   Clears all stored state and cached data.
    *   Call supers clearSessionState() too.
    */
   virtual void clearSessionState();

   /**
    *   Set if the system is ready to recieve a new session
    */
   bool m_dataSocketDone;
   
   /**
    *   Set if this session is valid (connected to a navigator)
    */
   bool m_inSession;

   /**
    * The user assosiated with this session.
    */
   UserItem* m_user;

   /**
    * The other ends IP, if any.
    */
   uint32 m_peerIP;

   /**
    * The other ends port, if any.
    */
   uint16 m_peerPort;

   /**
    * The current session key.
    */
   MC2String m_sessionKey;
};


#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATASOCKET_H
#define DATASOCKET_H

#include "config.h"
#include "TCPSocket.h"
#include "isabBoxSession.h"

/**
 *   Class that inherits from TCPSocket to create different kind of sockets.
 *
 */
class DataSocket : public TCPSocket {

public:

   /**
    *  Creates a new DataSocket
    *  @param sess The session this socket is associsated with.
    */
   DataSocket(isabBoxSession *sess );
   

   /**
    * Constructor that takes an fd as parameter.
    *
    *    @param   sess     The session this socket is associsated with.
    *    @param   sock     The filedescriptor of the socket.
    *    @param   backlog  The value of the backlog variable.
    */
   DataSocket( isabBoxSession* sess, SOCKET sock, int backlog );


   /** 
    *   Destructor. 
    */
   virtual ~DataSocket();

   /**
    *   Get the session this socket is associated with
    *   @return isabBoxSession session
    */
   isabBoxSession * getSession();

   /**
    *   Sets the session for this socket, old session is just dropped
    *   not deleted.
    *
    *   @param session The new session.
    */
   void setSession( isabBoxSession* session );

   /**
    *  Virtual function that creates a DataSocket object instead of
    *  a general TCPSocket. The Session of the new object is the same as of
    *  the old DataSocket.
    *  @return A TCPSocket pointer to the {\em DataSocket} created. NULL
    *          if method failed.
    */
   virtual TCPSocket* accept();
   
protected:
   /** 
    *   The session this data socket is associated with
    */
   isabBoxSession * m_session;
};


#endif // DATASOCKET_H

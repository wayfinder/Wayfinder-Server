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
#include "DataSocket.h"

DataSocket::DataSocket(isabBoxSession* session)
{
      m_session = session;
}

DataSocket::DataSocket(isabBoxSession* sess, SOCKET sock, int backlog )
      : TCPSocket(sock, backlog)
{
      m_session = sess;
}

DataSocket::~DataSocket()
{

}

isabBoxSession *
DataSocket::getSession()
{
   return m_session;
}

void 
DataSocket::setSession( isabBoxSession* session ) {
   m_session = session;
}

TCPSocket*
DataSocket::accept()
{
   mc2dbg8 << "DataSocket::accept()" << endl;

   SOCKET tmpsock;
   
   if ((tmpsock = ::accept(m_sock, (struct sockaddr *)0, 0)) == 
       SOCKET_ERROR) 
   {
      if ( errno == EAGAIN ) {
         // Not any sockets in queue, try again after a select
         mc2dbg8 << "DataSocket::accept() EAGAIN: No socket right now."
                 << endl;
      } else {
         mc2log << error << "DataSocket::accept() failure: " 
                << strerror( errno ) << endl;
      }
      return NULL;
   }

   return new DataSocket(m_session, tmpsock, DEFAULT_BACKLOG);
}

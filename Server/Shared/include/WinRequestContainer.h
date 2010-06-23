/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC_REQUEST_CONTAINER_H
#define MC_REQUEST_CONTAINER_H

#include "TCPSocket.h"

/**
 *    A request that remembers the client's socket. This is nice when 
 *    communicating with clients running on windows.
 *
 */
class WinRequestContainer : public RequestContainer
{
public:
   /**
    * Creates a WinRequestContainer.
    *
    * @param replySocket the socket to use when communicating with the client.
    * @param theRequest an "oridinary" request.
    */
   WinRequestContainer( TCPSocket* replySocket,
                        Request* theRequest );

   /**
    * Destroys the socket.
    */
   virtual ~WinRequestContainer();

   /**
    * Returns the socket.
    *
    * @return the socket.
    */
   inline TCPSocket* getReplySocket();

private:

   ///Socket to use when communicating with the client.
   TCPSocket* m_replySocket;
   
};


/*
 *         \\ \\
 *       - // //
 *     -- //_//__     INLINES
 *   --- /____)__)          
 * ......00000000.............
 */


WinRequestContainer::WinRequestContainer( TCPSocket* replySocket,
                                          Request* theRequest )
      : RequestContainer( theRequest )
{
   m_replySocket = replySocket;
}

WinRequestContainer::~WinRequestContainer()
{
   delete m_replySocket;
}

inline TCPSocket* WinRequestContainer::getReplySocket()
{
   return m_replySocket;
}

#endif;

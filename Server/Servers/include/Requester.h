/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUESTER_H
#define REQUESTER_H

#include "config.h"
#include "ModuleTypes.h"

#include <vector>

class ThreadRequestContainer;
class Request;
class RequestWithStatus;
class PacketContainer;
class Packet;
class RequestData;

/**
 * Interface for sending \c Requests, \c Packets, and \c PacketContainers.
 */
class Requester {
public:

   virtual ~Requester() {
   }

   /**
    *    Sets everything that needs to be set in the reqCont and
    *    calls the groups putRequest(). {\tt Monitor method.}
    *    @param reqCont The Request to send.
    */
   virtual void putRequest( ThreadRequestContainer* reqCont ) = 0;

   /**
    *    Wrapper for putRequest( ThreadRequestContainer* ) this
    *    handles the ThreadRequestContainer.
    *    @param reqCont The Request to send.
    */
   virtual void putRequest( Request* req ) = 0;

   /**
    *    Puts multiple requests at the same time and returns when
    *    all are done.
    *    @param reqs The Requests to send.
    */
   virtual void putRequests( const vector<RequestWithStatus*>& reqs ) = 0;

   /**
    * Wrapper for putRequest( Request* ) this
    * handles the PacketContainer by using a SinglePacketRequest.
    *
    * @param cont The PacketContainer to send.
    * @return The answer PacketContainer, may be NULL.
    */
   virtual PacketContainer* putRequest( PacketContainer* cont ) = 0;

   /**
    * Wrapper for putRequest( PacketContainer* ) this
    * handles the Packet and module_t by using a PacketContainer.
    *
    * @param pack The Packet to send.
    * @param module Where to send packet.
    * @return The answer PacketContainer, may be NULL.
    */
   virtual PacketContainer* putRequest( Packet* pack,
                                        moduletype_t module ) = 0;
   /**
    * Sends a number of packets and returns the replies.
    *
    * @param reqs The packets to send, is cleared as packets are
    *             consumed.
    * @param reps The reply packets, is the same size as reqs was.
    */
   virtual void putRequest( vector< PacketContainer* >& reqs, 
                            vector< PacketContainer* >& reps ) = 0;

   /**
    * Send a number of packets and returns the replies.
    * 
    * @param packets When the function is called the vector should
    *                contain all the packets to send. When the
    *                function returns it will cotain all the
    *                returned packages.
    */
   virtual void putRequest( vector< PacketContainer* >& packets ) = 0;

   /**
    * The ID of the next request. Asks the ParserGroup.
    *
    * @return The next RequestID.
    */
   virtual RequestData getNextRequestID() = 0;

};

#endif // REQUESTER_H

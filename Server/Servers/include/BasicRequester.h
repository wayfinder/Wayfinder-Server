/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BASIC_REQUESTER_H
#define BASIC_REQUESTER_H

#include "Requester.h"
#include "ServerTypes.h"

/**
 * A basic implementation of a Requestor.
 * Sends requests to modules.
 */
class BasicRequester: public Requester {
public:
   explicit BasicRequester( ServerTypes::servertype_t type );
   ~BasicRequester();

   /// start sub threads
   void start();

   /// stop threads
   void stop();

   /// @see Requester
   virtual void putRequest( ThreadRequestContainer* reqCont );

   /// @see Requester
   virtual void putRequest( Request* req );

   /// @see Requester
   virtual void putRequests( const vector<RequestWithStatus*>& reqs );

   /// @see Requester
   virtual PacketContainer* putRequest( PacketContainer* cont );

   /// @see Requester
   virtual PacketContainer* putRequest( Packet* pack,
                                        moduletype_t module );

   /// @see Requester
   virtual void putRequest( vector< PacketContainer* >& reqs,
                            vector< PacketContainer* >& reps );

   /// @see Requester
   virtual void putRequest( vector< PacketContainer* >& packets );

   /// @see Requester
   virtual RequestData getNextRequestID();

private:
   struct Impl;
   Impl* m_impl;
};

#endif // BASIC_REQUESTER_H

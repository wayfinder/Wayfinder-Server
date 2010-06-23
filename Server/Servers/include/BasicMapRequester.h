/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BASIC_MAP_REQUESTER_H
#define BASIC_MAP_REQUESTER_H

#include "MapRequester.h"
#include "ServerTypes.h"
#include "NotCopyable.h"

#include <memory>

/**
 * A basic implementation of a MapRequestor. Works like a cache for
 * top region request.
 */
class BasicMapRequester: public MapRequester, private NotCopyable {
public:
   explicit BasicMapRequester( ServerTypes::servertype_t type );
   ~BasicMapRequester();

   /// Start sub threads
   void start();

   /// Stop sub threads.
   void stop();

   /// @see MapRequester
   const TopRegionRequest* getTopRegionRequest();

   /// @see Requester
   void putRequest( ThreadRequestContainer* reqCont );

   /// @see Requester
   void putRequest( Request* req );

   /// @see Requester
   void putRequests( const vector<RequestWithStatus*>& reqs );

   /// @see Requester
   PacketContainer* putRequest( PacketContainer* cont );

   /// @see Requester
   PacketContainer* putRequest( Packet* pack,
                                moduletype_t module );

   /// @see Requester
   void putRequest( vector< PacketContainer* >& reqs,
                    vector< PacketContainer* >& reps );

   /// @see Requester
   void putRequest( vector< PacketContainer* >& packets );

   /// @see Requester
   RequestData getNextRequestID();

private:
   /// Real implementation, avoids "dreaded diamon of death", and some strange
   /// "non-virtual thunk"-linkage problems with ParserThread and XMLServer.
   std::auto_ptr< Requester > m_requester;

   std::auto_ptr< TopRegionRequest > m_topRegionRequest; ///< Top region cache.
};

#endif // BASIC_MAP_REQUESTER_H

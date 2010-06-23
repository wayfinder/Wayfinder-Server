/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEND_REQUEST_HOLDER_H
#define SEND_REQUEST_HOLDER_H

#include "config.h"
#include "ModuleTypes.h"

// Forwards
class ThreadRequestContainer;
class Request;
class SinglePacketRequest;
class PacketContainer;
class Packet;
class ParserThread;

/**
 * Holds the created data for the request while it is being processed.
 * Used by ParserThread.
 */
class SendRequestHolder {
public:
   /**
    * Constructor.
    */
   SendRequestHolder( ThreadRequestContainer* reqCont, ParserThread* thread );

   /**
    * Constructor.
    */
   SendRequestHolder( Request* req, ParserThread* thread );

   /**
    * Constructor.
    */
   SendRequestHolder( PacketContainer* cont, ParserThread* thread );

   /**
    * Constructor.
    */
   SendRequestHolder( Packet* pack, moduletype_t module, 
                      ParserThread* thread );

   /**
    * Destructor.
    */
   ~SendRequestHolder();

   /**
    * Get the ThreadRequestContainer for the request.
    */
   ThreadRequestContainer* getReqCont();

   /**
    * Get the ThreadRequestContainer for the request.
    */
   const ThreadRequestContainer* getReqCont() const;

private:
   /// The resulting ThreadRequestContainer.
   ThreadRequestContainer* m_usedReqCont;

   /// The created ThreadRequestContainer.
   ThreadRequestContainer* m_reqCont;

   /// The created SinglePacketRequest.
   SinglePacketRequest* m_req;

   /// The created PacketContainer.
   PacketContainer* m_packCont;

};


// ========================================================================
//                                      Implementation of inlined methods =


inline ThreadRequestContainer*
SendRequestHolder::getReqCont() {
   return m_usedReqCont;
}

inline const ThreadRequestContainer*
SendRequestHolder::getReqCont() const {
   return m_usedReqCont;
}


#endif // SEND_REQUEST_HOLDER_H


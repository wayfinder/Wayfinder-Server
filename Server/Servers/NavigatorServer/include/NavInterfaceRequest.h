/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVINTERFACEREQUEST_H
#define NAVINTERFACEREQUEST_H

#include "config.h"
#include "SelectableInterfaceRequest.h"

// Forward
class NavMessage;
class OutGoingNavMessage;


/**
 * Receives and sends Nav packets.
 *
 */
class NavInterfaceRequest : public SelectableInterfaceRequest {
   public:
      /**
       * Creates a new NavInterfaceRequest.
       */
      NavInterfaceRequest();


      /**
       * Destructor.
       */
      virtual ~NavInterfaceRequest();


      /**
       * Get the request NavMessage. Might be NULL.
       *
       * @return The request message.
       */
      virtual NavMessage* getRequestMessage() = 0;


      /**
       * Set the reply NavMessage.
       */
      virtual void setReplyMessage( OutGoingNavMessage* reply );
      

      /**
       * The server is overloaded make mimimal reply with that status.
       * Default here is to set state to Error.
       */
      virtual void handleOverloaded( int overLoad );


      /**
       * Get the request data size.
       */
      virtual uint32 getRequestSize() const = 0;


      /**
       * Get the reply data size.
       */
      virtual uint32 getReplySize() const = 0;


      /**
       * Get the protoVer of the request.
       */
      byte getProtoVer() const;


   protected:
      /// The reply NavMessage.
      OutGoingNavMessage* m_reply;


      /// The ProtoVer of the request
      byte m_protoVer;


   private:
};


// =======================================================================
//                                     Implementation of inlined methods =


inline void 
NavInterfaceRequest::setReplyMessage( OutGoingNavMessage* reply ) {
   m_reply = reply;
}
      

inline byte
NavInterfaceRequest::getProtoVer() const {
   return m_protoVer;
}


#endif // NAVINTERFACEREQUEST_H



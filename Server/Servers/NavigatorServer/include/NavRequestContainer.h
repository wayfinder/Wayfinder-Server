/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVREQUESTCONTAINER_H
#define NAVREQUESTCONTAINER_H


#include "config.h"
#include "RequestContainer.h"

// Forward declaration
class NavMessage;
class OutGoingNavMessage;

/** 
 *   NavigatorServer-specific request container.
 *   The Request is only borrowed by this notice and is not changed 
 *   nor deleted.
 *
 *   The container is special in the sense that it is created without
 *   a request. The request is added later by the setRequest-method.
 *
 */
class NavRequestContainer : public RequestContainer {

public:
   /** 
    *   Create a new NavRequestContainer.
    *   @param threadID   The ID of the NavThread that created the 
    *                     request.
    *   @param request    The request.
    */
   NavRequestContainer( uint32 threadID, Request* request );


   /**
    *   Temporary constructor for testing that the idea works.
    *   @param mess The navmessage to use when creating the
    *               request.
    */
   NavRequestContainer( NavMessage* mess );
   
   /**
    *   Delete this NavRequestContainer. 
    *   NB! The request is not deleted.
    */
   virtual ~NavRequestContainer();
   
   
   /**
    *   Return the ID of the NavThread that generated this Request.
    */
   inline uint32 getThreadID();
   
   
   /**
    *   Sets the ID of the NavThread that generated this Request.
    */
   inline void setThreadID( uint32 id );

   /**
    *    Set the request of this container.
    */
   void setRequest(Request* request);

   
   /**
    *    @return The message of this container.
    */
   virtual NavMessage* getMessage() = 0;

   /**
    *    Send the answer to the sender of the request.
    *    @param answer The answer to send.
    */
   virtual bool setAndSendAnswer(OutGoingNavMessage* answer) = 0;
      
protected:
   
   /** 
    *   The ID of the NavThread that this Request belongs to.
    */
   uint32 m_threadID;

   /**
    *   The message of this container. Should maybe be moved later.
    */
   NavMessage* m_navMess;
};


// ========================================================================
//                                       Implementation of inline methods =


uint32
NavRequestContainer::getThreadID() {
   return m_threadID;
}


void 
NavRequestContainer::setThreadID( uint32 id ) {
   m_threadID = id;
}


#endif


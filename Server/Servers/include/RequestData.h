/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUEST_DATA_H
#define REQUEST_DATA_H

#include "config.h"

class Request;
class UserUser;

/**
 *   Class representing either a Request OR a Request ID.
 *   This can be used to keep down the number of parameters
 *   to a request that can be a sub-request or a request of
 *   its own by using a single parameter in the constructor.
 */
class RequestPtrOrRequestID {
public:
   /**
    *   Creates a RequestOrRequestID where the id should be used.
    */
   RequestPtrOrRequestID( uint16 id ) : m_request(NULL), m_id(id) {}
   
   /**
    *   Creates a RequestOrRequestID where the Request should be used.
    */
   RequestPtrOrRequestID( Request* req ) : m_request(req), m_id(MAX_UINT16){}

   /**
    *   Returns the request. If NULL, the request should be used.
    */
   Request* getRequest() const;

   /**
    *   Returns the request id of the parent if the parent is not
    *   NULL. If the parent is NULL, the id will be used.
    */
   uint16 getID() const;
   
protected:

private:
   /// The request or NULL.
   Request* m_request;

   /// The id to be used if the Request is NULL:
   uint16 m_id;   
};

/**
 *    Information that is good for the request to have that
 *    the ParserThread might know about.
 *    Should replace request id in all requests.
 *    Currently contains the user plus request id to use.
 *    Do not create RequestDatas unless you know what you are doing.
 */
class RequestData : public RequestPtrOrRequestID {
public:
   /**
    *   Takes the user from the request if there is one.
    *   Used for compatibility.
    */
   RequestData( const RequestPtrOrRequestID& reqid );
   
   /**
    *   Create a RequestData using the request id and a user.
    *   Should be used by parser thread.
    */
   RequestData( uint16 reqID,
                const UserUser* user );

   /**
    *   Create RequestData from another request.
    */
   RequestData( Request* other );
   
   /**
    *   Returns the user if any.
    */
   const UserUser* getUser() const;

   /// Returns the request id. Needed for old requests.
   operator uint16() const;
   
private:
   /// The user.
   const UserUser* m_user;
   
};


#endif

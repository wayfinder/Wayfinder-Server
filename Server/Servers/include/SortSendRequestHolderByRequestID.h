/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SORT_SEND_REQUEST_HOLDER_BY_REQUEST_ID_H
#define SORT_SEND_REQUEST_HOLDER_BY_REQUEST_ID_H

#include "config.h"
#include "SendRequestHolder.h"
#include "ThreadRequestContainer.h"
#include "Request.h"

/**
 * Sorts SendRequestHolder by the Requests ID.
 */
class SortSendRequestHolderByRequestID {
public:
   bool operator()( const SendRequestHolder* a,
                    const SendRequestHolder* b ) const {
      return a->getReqCont()->getRequest()->getID() < 
             b->getReqCont()->getRequest()->getID();
   }

   bool operator()( const SendRequestHolder* a,
                    uint16 requestID ) const {
      return a->getReqCont()->getRequest()->getID() < requestID;
   }

   bool operator()( uint16 requestID,
                    const SendRequestHolder* b ) const {
      return requestID < b->getReqCont()->getRequest()->getID();
   }
};


#endif // SORT_SEND_REQUEST_HOLDER_BY_REQUEST_ID_H


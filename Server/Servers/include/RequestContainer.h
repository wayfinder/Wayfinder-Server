/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUESTCONTAINER_H
#define REQUESTCONTAINER_H

#include "config.h"
#include "Types.h"
#include "Request.h"

/**
  *   A Request holder. The only purpose for objects of this class is to 
  *   have the request in other data-structures, e.g. a list. The request
  *   container does {\bf not} own the request, it only contains it. That
  *   means that the request will {\bf not} be deleted when the request
  *   container is deleted.
  *
  */
class RequestContainer : public Link {
   public:
      /**
       *    Creates a new container for theRequest.
       *    @param theRequest The Request to hold.
       */
      RequestContainer(Request* theRequest);

      /**
       *    Delete this request. Will not delete the request inside the
       *    container.
       */
      virtual ~RequestContainer();
      
      /**
       *    Returns the contained Request.
       *    @return The Request of this container.
       */
      inline Request* getRequest() const;

      /**
       *    When this method is called the request is deleted and the
       *    membervariable is set to NULL.
       */
      void deleteRequest();
      
   protected:
      /**
       *    The request in this container.
       */
      Request* m_request; 
};


// ========================================================================
//                                      Implementation of inlined methods =

inline Request* 
RequestContainer::getRequest() const {
   return m_request;
}


#endif // REQUESTCONTAINER_H


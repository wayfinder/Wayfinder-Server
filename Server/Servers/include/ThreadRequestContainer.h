/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef THREADREQUESTCONTAINER_H
#define THREADREQUESTCONTAINER_H


#include "config.h"
#include "RequestContainer.h"
#include "ISABThread.h"
#include "ParserThreadConfig.h"


/**
 *    RequestContainer for the servert that uses a threadgroup to
 *    administrate the threads.
 *
 */
class ThreadRequestContainer : public RequestContainer {
   public:
      /**
       *    Creates a new ThreadRequestContainer for req, use
       *    setThread to set return thread.
       * 
       *    @param req The Request that this container should have.
       */
      ThreadRequestContainer( Request* req );

      /**
       *    Destructor, does nothing.
       */
      virtual ~ThreadRequestContainer();
       
      /**
       *    Set the thread that this container should be returned to.
       *    @param thread The thread to return the Request to.
       */
      void setThread( ParserThreadHandle thread );
   
      /**
       *    Get the thread that this container should be returned to.
       *    @return The thread to return the Request to.
       */
      ParserThreadHandle getThread();


   private:
      /**
       *    The thread of this ThreadRequestContainer.
       */
      ParserThreadHandle m_thread;
};


#endif // THREADREQUESTCONTAINER_H


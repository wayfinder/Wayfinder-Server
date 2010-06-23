/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVTHREAD_H
#define NAVTHREAD_H

#include "config.h"
#include "InterfaceParserThread.h"
#include "NotCopyable.h"

class NavInterfaceRequest;

// Forward declaration
class isabBoxSession;
class NavSession;
class NavParserThreadGroup;
class IsabBoxInterfaceRequest;
class HttpHeader;
class HttpBody;
class HttpInterfaceRequest;
class NavRequestHandler;
class NavUserHelp;
class NavMapHelp;
class NavMessHelp;


/**
 *   Thread that handles incoming messages from NavComm.
 *
 *   When the thread is idle and waits for requests it waits
 *   for a requestmessage to be put into the reqList.
 *
 *   When a requestmessage is processed and it waits for a reply from
 *   a navigator putNavigatorReply is called instead.
 *
 *   putNavigatorReply is also called when a timeout occurs.
 *
 */
class NavThread : public InterfaceParserThread, private NotCopyable {
   public:
      /**
       *
       *  Creates a new NavThread.
       *
       *  @param id   ID of thread.
       *  @param group The NavParserThreadGroup that this is part of.
       */
      NavThread( uint32 id,
                 NavParserThreadGroup* group );

      /**
       *  Destructor. Don't call it. It will be called when the thread
       *  exits, I think. That is true.
       */
      virtual ~NavThread();


      /**
       *   Get the id number of the thread.
       *   @return The id number of the thread.
       */
      int getID() const;


   protected:
      /**
       * This function is called when a InterfaceRequest has been received.
       *
       * @param ireq The InterfaceRequest to process.
       */
      virtual void handleInterfaceRequest( InterfaceRequest* ireq );


      /**
       * Set the nav packet as http reply in Http request.
       *
       * @param ireq The request with Nav packet.
       * @param outHead Content-Length is set in this.
       * @param outBody Body is set to the Nva packet.
       * @param hreq outHead and outBody is set as reply, hreq
       *             now owns them.
       */
      void setHttpReply( IsabBoxInterfaceRequest* ireq,
                         HttpHeader* outHead, HttpBody* outBody,
                         HttpInterfaceRequest* hreq );


      /**
       * Get a string that describes the type of server, "NS".
       *
       * @return  A string that describes the server that does the 
       *          debiting.
       */
      virtual const char* getServerType();


      /**
       *   The ID of the thread.
       */
      uint32 m_id;


      /**
       * The NavParserThreadGroup.
       */
      NavParserThreadGroup* m_server;


      /**
       * The current irequest.
       */
      NavInterfaceRequest* m_ireq;


      /**
       * The session for the current irequest.
       */
      NavSession* m_session;


      /**
       * The handler of Nav request for protoVer >= 0xa.
       */
      NavRequestHandler* m_navHandler;
};


#endif


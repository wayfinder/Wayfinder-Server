/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPINTERFACEFACTORY_H
#define HTTPINTERFACEFACTORY_H

#include "config.h"
#include "InterfaceFactory.h"
#include "ISABThread.h"
#include "NotCopyable.h"

class InterfaceRequest;
class SelectableSelector;
class TCPSocket;


/**
 * Class for creating interface requests for mc2 using HTTP.
 *
 */
class HttpInterfaceFactory : public InterfaceFactory, private NotCopyable {
   public:
      /**
       * Creates a new HttpInterfaceFactory.
       *
       * @param group The InterfaceParserThreadGroup to send new
       *              InterfaceRequests to.
       * @param port The port to listen on.
       */
      HttpInterfaceFactory( InterfaceParserThreadGroup* group,
                            const char* port );


      /**
       * Creates a new HttpInterfaceFactory.
       *
       * @param group The InterfaceParserThreadGroup to send new
       *              InterfaceRequests to.
       * @param sock The socket to listen on.
       */
      HttpInterfaceFactory( InterfaceParserThreadGroup* group,
                            TCPSocket* sock );


      /**
       * Destructor.
       */
      virtual ~HttpInterfaceFactory();


      /**
       * Start making InterfaceRequests.
       */
      virtual void start();


      /**
       * Stop making InterfaceRequests and await destruction.
       */
      virtual void terminate();


      /**
       * Handle a done irequest.
       */
      virtual void handleDoneInterfaceRequest( InterfaceRequest* ireply );


   private:
      /**
       * New irequests.
       */
      void putNewRequest( InterfaceRequest* ireq );


      /**
       * Class for listening on the http port and accepting new 
       * InterfaceRequest from it.
       */
      class HttpInterfaceFactoryThread : public ISABThread {
         public:
            /**
             * Constructor.
             *
             * @param port The port to listen on.
             * @param factory The HttpInterfaceFactory to send new 
             *                InterfaceRequest to.
             */
            HttpInterfaceFactoryThread( const char* port,
                                        HttpInterfaceFactory* factory );


            /**
             * Constructor.
             *
             * @param sock The socket to listen on.
             * @param factory The HttpInterfaceFactory to send new 
             *                InterfaceRequest to.
             */
            HttpInterfaceFactoryThread( TCPSocket* sock,
                                        HttpInterfaceFactory* factory );


            /**
             * Destructor.
             */
            ~HttpInterfaceFactoryThread();


            /**
             * The thread method.
             */
            void run();


            /**
             * Notifies the selector.
             */
            void notify();


         private:
            /**
             * The listen socket.
             */
            TCPSocket* m_socket;


            /**
             * The selector.
             */
            SelectableSelector* m_selector;


            /**
             * The HttpInterfaceFactory that this is a part of.
             */
            HttpInterfaceFactory* m_factory;
      };


      /**
       * The thread.
       */
      HttpInterfaceFactoryThread* m_thread;
};


#endif // HTTPINTERFACEFACTORY_H


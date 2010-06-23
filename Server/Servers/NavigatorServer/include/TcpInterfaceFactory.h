/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPINTERFACEFACTORY_H
#define TCPINTERFACEFACTORY_H

#include "config.h"
#include "NavInterfaceFactory.h"
#include "ISABThread.h"


class SelectableSelector;
class DataSocket;


/**
 * Class for creating interface requests for Nav using TCP.
 *
 */
class TcpInterfaceFactory : public NavInterfaceFactory {
   public:
      /**
       * Creates a new TcpInterfaceFactory.
       *
       * @param group The InterfaceParserThreadGroup to send new
       *              InterfaceRequests to.
       * @param dataPort The port to listen on.
       * @param httpReq If to create Http interface requests.
       */
      TcpInterfaceFactory( InterfaceParserThreadGroup* group,
                           const char* dataPort, bool httpReq = false );


      /**
       * Creates a new TcpInterfaceFactory using a already open socket.
       *
       * @param group The InterfaceParserThreadGroup to send new
       *              InterfaceRequests to.
       * @param fd The already open socket.
       * @param httpReq If to create Http interface requests.
       */
      TcpInterfaceFactory( InterfaceParserThreadGroup* group,
                           int fd, bool httpReq = false );


      /**
       * Destructor.
       */
      virtual ~TcpInterfaceFactory();


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
       * Class for listening on the tcp port and accepting new 
       * InterfaceRequest from it.
       */
      class TcpInterfaceFactoryThread : public ISABThread {
         public:
            /**
             * Constructor.
             *
             * @param dataPort The port to listen on.
             * @param factory The TcpInterfaceFactory to send new 
             *                InterfaceRequest to.
             * @param httpReq If to create Http interface requests.
             */
            TcpInterfaceFactoryThread( const char* dataPort,
                                       TcpInterfaceFactory* factory,
                                       bool httpReq );

            /**
             * Constructor using an already open socket.
             *
             * @param fd The already open socket.
             * @param factory The TcpInterfaceFactory to send new 
             *                InterfaceRequest to.
             * @param httpReq If to create Http interface requests.
             */
            TcpInterfaceFactoryThread( int fd,
                                       TcpInterfaceFactory* factory,
                                       bool httpReq );


            /**
             * Destructor.
             */
            ~TcpInterfaceFactoryThread();


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
            DataSocket* m_socket;


            /**
             * The selector.
             */
            SelectableSelector* m_selector;


            /**
             * The TcpInterfaceFactory that this is a part of.
             */
            TcpInterfaceFactory* m_factory;


            /**
             * If to create Http interface request.
             */
            bool m_httpReq;
      };


      /**
       * The thread.
       */
      TcpInterfaceFactoryThread* m_thread;
};


#endif // TCPINTERFACEFACTORY_H


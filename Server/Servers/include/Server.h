/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVER_H
#define SERVER_H

#include "config.h"
#include "GenericServer.h"
#include "ISABThread.h"
#include "ServerTypes.h"

/** 
  *   Abstract simle server class. Used when a simple get- put-request
  *   functions in the subclass interface is sufficient to run the server.
  *
  */
class Server : public GenericServer {
public:
   /**
    * Create a server of a given type and name.
    * @param type  The kind of server beeing created.
    */
   Server( const MC2String& name,
           CommandlineOptionHandler* coh,
           ServerTypes::servertype_t type );


      /**
        *   Delete the server.
        *   Subclasses must: m_getThread->terminate()
        *                    and release m_getThread from getRequest().
        */
      virtual ~Server();


      void init();

      /**
       * Starts the GetThread and PutThread.
       */
      virtual void start();

      
   protected:    
      /** 
        *   Virtual method to receive new requests from Server
        *   subclasses.
        *   @return  A new request that should be processed by the 
        *            modules.
        */
      virtual RequestContainer *getRequest() = 0;


      /** 
        *   Virtual method to return processed requests to Server
        *   subclasses.
        *   @param   req   A request that is done.
        */
      virtual void putAnswer(RequestContainer *req) = 0;


   private:
      /** 
        *   Class to handle processing of requests from the network
        *   by calling Server::putAnswer.
        */
      class PutThread : public ISABThread {
         public:
            /**
              *   Create a new PutThread.
              *   @param   server   The server this is part of.
              */
            PutThread( Server *server );


            /**
              *   Delete the PutThread.
              */
            virtual ~PutThread();


            /**
              *   Start the putThread.
              */
            virtual void run();


         private:
            /**   
              *   The server this PutThread is part of.
              */
            Server* m_server;
      };
      

      /** 
        *   Class to get requests from Server::getRequest and handle them.
        */
      class GetThread : public ISABThread {
         public:
            /**
              *   Create a new GetThread.
              *   @param   server   The server this is part of.
              */
            GetThread( Server *server );


            /**
              *   Delete the GetThread.
              */
            virtual ~GetThread();


            /**
              *   Start the GetThread.
              */
            virtual void run();


         private:
            /**   
              *   The server this ReadThread is part of.
              */
            Server* m_server;
      };


      // ************************************************************
      // Member variables
      // ************************************************************
   protected:
      /**
       *   The thread used to process requests in Server::putAnswer.
       */
      PutThread* m_putThread;


      /**
       *   The thread used to get packets from the subclasses and 
       *   process in RequestHandler.
       */
      GetThread* m_getThread;


   private:      
      /**
       * True if this server is running, false otherwise.
       */
      bool m_running;


      /// PutThread is a friend
      friend class PutThread;


      /// GetThread is a fiend
      friend class GetThread;
};


#endif // SERVER_H

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef THREADREQUESTHANDLER_H
#define THREADREQUESTHANDLER_H


#include "config.h"
#include "GenericServer.h"
#include "ParserThreadGroup.h"
#include "ThreadRequestContainer.h"
#include "NotCopyable.h"

/**
 *    Class for retrieving processed Requests from the server and send 
 *    them to the ParserThreadGroup. Also used when sending Requests for
 *    logical reasons.
 *    It also optionally handles PushServices if the GenericServer 
 *    supports it.
 *
 */
class ThreadRequestHandler: private NotCopyable {
   public:
      /**
       *    Create a new request handler.
       *
       *    @param server      The GenericServer to interface with 
       *                       regarding Requests.
       *    @param threadGroup The ParserThreadGroup to send processed
       *                       Requests to.
       */
      ThreadRequestHandler(GenericServer* server, 
                           ParserThreadGroupHandle threadGroup );
      
      /**
       *    Destructor, terminates internal thread.
       */
      virtual ~ThreadRequestHandler();

      /**
       *    Starts the Receiving of Requests.
       */
      void start();

      /**
       *    Inserts a new Request to be processed.
       *
       *    @param rc RequestContainer to insert.
       */
      inline void insert(RequestContainer* rc);

      /**
       *    Get the next RequestID.
       *    @return The next valid request ID.
       */
      inline uint16 getNextRequestID();

      /**
       *    Get the cache of the server. This must not be deleted!
       *    @return  The cache in the server.
       */
      inline MonitorCache* getCache();


      /**
       * Add a PushService. Nothing will be done if not created with
       * pushService.
       * 
       * @param service The PushService to add. The object is no longer
       *                owned by caller after the call to this method.
       * @param lastUpdateTime For all resources the lastUpdateTime is 
       *                       added to this vector.
       */
      inline void addPushService( PushService* service,
                                  vector<uint32>& lastUpdateTime );


      /**
       * Removes a PushService entierly. Nothing will be done if not
       * created with pushService.
       *
       * @param serviceID The ID of the PushService to remove.
       * @return True if serviceID did exist, false if it didn't.
       */
      inline bool removePushService( uint32 serviceID );


      /**
       * Removes a certain resource from a PushService.
       * Mutex function.
       *
       * @param serviceID The ID of the PushService to remove.
       * @param resource The resource to stop having push for.
       * @return True if resource did exist, false if it didn't.
       */
      inline bool removePushServiceResource( 
         uint32 serviceID, SubscriptionResource& resource );



      /**
       * Call when system is going down to stop PushPackets to be sent
       * to ParserThreadGroup.
       */
      void terminate();

   private:
      /**
       *    Calls getAnswer in RequestHandler.
       *
       *    @return a answer, or NULL if notified.
       */
      inline RequestContainer* getAnswer();

      /**
       *    Sends a processed Request to the ParserThreadGroup.
       */
      void putAnswer(RequestContainer* rc);

      /** 
       *    Class to handle processing of processed Requests from the
       *    RequestsHandler.
       */
      class RequestThread : public ISABThread {
         public:
            /**
             *    Create a new RequestThread.
             *    @param handler The ThreadRequestHandler this is part of.
             */
            RequestThread( ISABThreadGroup* group,
                           ThreadRequestHandler* handler );

            /**
             *    Delete the RequestThread.
             */
            virtual ~RequestThread();

            /**
             *    Start the RequestThread.
             */
            virtual void run();


         private:
            /**   
             *    The ThreadRequestHandler this RequestThread is part of.
             */
            ThreadRequestHandler* m_handler;
      };


      /** 
       * Class to handle processing of received PushPackets.
       */
      class PushReceiveThread : public ISABThread {
         public:
            /**
             * Create a PushReceiveThread.
             * @param handler The ThreadRequestHandler this is part of.
             */
            PushReceiveThread( ISABThreadGroup* group,
                               ThreadRequestHandler* handler );

            /**
             * Delete the PushReceiveThread.
             */
            virtual ~PushReceiveThread();

            /**
             * Start the PushReceiveThread.
             */
            virtual void run();


         private:
            /**   
             * The ThreadRequestHandler this PushReceiveThread is part of.
             */
            ThreadRequestHandler* m_handler;
      };


      /**
       * Returns received a PushPacket. 
       *
       * @param serviceID Set to the serviceID of the PushPacket.
       * @param resource Set to the resource of the PushPacket or NULL
       *                 if notified. Deleted by caller.
       * @return a PushPacket or NULL if notified.
       */
      inline PushPacket* getPushPacket( uint32& serviceID, 
                                        SubscriptionResource*& resource );


      /**
       * Sends a pushed PushPacket to the ParserThreadGroup.
       *
       * @param packet The PushPacket.
       * @param serviceID The serviceID of the PushPacket.
       * @param resource The resource of the PushPacket.
       */
      void putPushPacket( PushPacket* packet, uint32 serviceID, 
                          SubscriptionResource* resource );
      
      
      /**
       *    The RequestThread.
       */
      RequestThread* m_requestThread;


      /**
       * The PushReceiveThread.
       */
      PushReceiveThread* m_pushReceiveThread;


      /**
       *    The Generic Server that should process the requests.
       */
      GenericServer* m_server;

      /**
       *    The ParserThreadGroup to send processed requests to.
       */
      ParserThreadGroupHandle m_group;

      /**
       * If system is up.
       */
      bool m_running;

      
      /**
       *    RequestThread is a friend.
       */
      friend class RequestThread;

      /**
       * ThreadRequestHandler is a friend.
       */
      friend class PushReceiveThread;
};


// =======================================================================
//                                 Implementation of the inlined methods =


inline void 
ThreadRequestHandler::insert(RequestContainer* rc) 
{
   m_server->insertRequest(rc); 
}


inline RequestContainer* 
ThreadRequestHandler::getAnswer() 
{
   return m_server->getAnswer();
}


inline uint16
ThreadRequestHandler::getNextRequestID() 
{
   return m_server->getNextRequestID();
}

MonitorCache* 
ThreadRequestHandler::getCache()
{
   return m_server->getCache();
}


inline PushPacket* 
ThreadRequestHandler::getPushPacket( uint32& serviceID, 
                                     SubscriptionResource*& resource )
{
   return m_server->getPushPacket( serviceID, resource );
}


inline void 
ThreadRequestHandler::addPushService( PushService* service,
                                      vector<uint32>& lastUpdateTime )
{
   m_server->addPushService( service, lastUpdateTime );
}


inline bool 
ThreadRequestHandler::removePushService( uint32 serviceID ) {
   return m_server->removePushService( serviceID );
}


inline bool 
ThreadRequestHandler::removePushServiceResource( 
   uint32 serviceID, SubscriptionResource& resource ) 
{
   return m_server->removePushServiceResource( serviceID, resource );
}


#endif // THREADREQUESTHANDLER_H


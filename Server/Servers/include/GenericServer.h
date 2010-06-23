/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GENERICSERVER_H
#define GENERICSERVER_H

#include "config.h"
#include "ISABThread.h"
#include "RequestHandler.h"
#include "PushServiceHandler.h"
#include "Component.h"
#include "ServerTypes.h"
#include "NotCopyable.h"

class MonitorCache;
class PacketResendHandler;
class TopRegionRequest;

/**
 * Generic Server interface with a insert- getAnswer- Request interface.
 * Used when the subclass wants to use its own threads to make Requests.
 *
 */
class GenericServer: public Component, private NotCopyable {
public:
      /**
       * Create a Server of a given type.
       * @param type The kind of Server beeing created.
       */
      GenericServer( const MC2String& name,
                     CommandlineOptionHandler* handler,
                     ServerTypes::servertype_t type, bool pushService = false );
      

      /**
       * Destructor, shuts down the internal threads and deletes 
       * internal data.
       */
      virtual ~GenericServer();


      /**
       * Method to get the next unique request ID.
       * @return The next unique requestID .
       */
      inline uint16 getNextRequestID();


      /**
       * Should be called when request is returned.
       * @param reqID The id to be reused.
       */
      inline void returnRequestID( uint16 reqID );


      /**
       * The cache of this Server.
       * @return The Cache.
       */
      inline MonitorCache* getCache();

      
      /** 
       * Start the Server.
       * If subclasses implements this method they MUST call 
       * GenericServer::start() first in their start() method.
       */
      virtual void start();

      
      /** 
       * Let the Component's gotoWork run. This method blocks until
       * application shutdown.
       */
      virtual void callGotoWork();


      /**
       * Inserts a new RequestContainer to be processed.
       * 
       * @param rc The RequestContainer of the Request.
       */
      inline void insertRequest( RequestContainer* rc );


      /**
       * Returns a processed RequestContainer.
       * This method hangs until a processed Request is available
       * or notifyAnswer() is called.
       * notifyAnswer is usually called when shutting down.
       *
       * @return A processed RequestContainer or NULL if notifyAnswer()
       *         was called.
       */
      inline RequestContainer* getAnswer();
      

      /**
       * Notifies all threads waiting in getAnswer(). If created with
       * pushService then all threads waiting in getPushPacket() is
       * notified too.
       */
      inline void notifyAnswer();


      /**
       * Returns received a PushPacket. If not created with
       * pushService then this method returns NULL immediately.
       * This method hangs until a PushPacket is available
       * or notifyAnswer() is called.
       * notifyAnswer is usually called when shutting down.
       *
       * @param serviceID Set to the serviceID of the PushPacket.
       * @param resource Set to the resource of the PushPacket or NULL
       *                 if notified. Deleted by caller.
       * @return a PushPacket or NULL if notified.
       */
      inline PushPacket* getPushPacket( uint32& serviceID, 
                                        SubscriptionResource*& resource );


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
       * Removes a certain resource from a PushService. Nothing will be
       * done if not created with pushService.
       *
       * @param serviceID The ID of the PushService to remove.
       * @param resource The resource to stop having push for.
       * @return True if resource did exist, false if it didn't.
       */
      inline bool removePushServiceResource( 
         uint32 serviceID, SubscriptionResource& resource );


      /**
       * Returns true if created with PushService.
       * @return True if created with PushService, false if not.
       */
      inline bool getPushService() const;

      /// Get TopRegioonRequest with data
      const TopRegionRequest* getTopRegionRequest();
   /// shutdown threads
   virtual void shutdown();

protected:

   private:
      /**
       * Get the IP of the server.
       * @return The IP of the server.
       */
      inline uint32 getIP();


      /**
       * Get the port of the server.
       * @return The port of the server.
       */
      inline uint16 getPort();

      // ************************************************************
      // Member variables
      // ************************************************************
      
      /** 
       * Server IP address.
       */
      uint32 s_ip;


      /**
       * Server Port.
       */
      uint16 s_port;


      /**
       * The type of server.
       */
      ServerTypes::servertype_t s_type;


      /**
       * The next request ID to use.
       */
      uint16 m_reqID;


      /**
       * The Cache used by this server.
       */
      MonitorCache* m_cache;

      
      /**
       * PacketHandler.
       */
      PacketResendHandler* m_handler;


      /**
       * RequestHandler.
       */
      RequestHandler* m_requestHandler;


      /**
       * PushServiceHandler, optional.
       */
      PushServiceHandler* m_pushServiceHandler;


      /**
       * If to have PushService.
       */
      bool m_pushService;

      /**
       * The TopRegionRequest.
       */
      TopRegionRequest* m_topRegionRequest;


      /**
       * The mutex used to lock getTopRegionRequest.
       */
      ISABMutex m_topRegionsMutex;
   ISABThreadInitialize m_threadInit;
};


// =======================================================================
//                                 Implementation of the inlined methods =


uint16 
GenericServer::getNextRequestID(){
   return m_reqID++;
}

void 
GenericServer::returnRequestID(uint16 reqID ) {
}


MonitorCache* 
GenericServer::getCache() { 
   return m_cache; 
}


RequestContainer* 
GenericServer::getAnswer() {
   return m_requestHandler->getAnswer();
}


void
GenericServer::insertRequest( RequestContainer* rc ) {
   rc->getRequest()->setOriginator(s_type);
   m_requestHandler->insert( rc );
}


void 
GenericServer::notifyAnswer() {
   m_requestHandler->notifyAnswer();
   if ( m_pushService ) {
      m_pushServiceHandler->notifyGetPushPacket();
   }
}


PushPacket* 
GenericServer::getPushPacket( uint32& serviceID, 
                              SubscriptionResource*& resource )
{
   if ( m_pushService ) {
      return m_pushServiceHandler->getPushPacket( serviceID, resource );
   } else {
      return NULL;
   }
}


void 
GenericServer::addPushService( PushService* service,
                               vector<uint32>& lastUpdateTime ) {
   if ( m_pushService ) {
      m_pushServiceHandler->addPushService( service, lastUpdateTime );
   }
}


bool 
GenericServer::removePushService( uint32 serviceID ) {
   if ( m_pushService ) {
      return m_pushServiceHandler->removePushService( serviceID );
   } else {
      return false;
   }
}


bool 
GenericServer::removePushServiceResource( 
   uint32 serviceID, SubscriptionResource& resource )
{
   if ( m_pushService ) {
      return m_pushServiceHandler->removePushServiceResource(
         serviceID, resource );
   } else {
      return false;
   }
}


inline bool 
GenericServer::getPushService() const {
   return m_pushService;
}


uint32 
GenericServer::getIP() { 
   return s_ip; 
}


uint16 
GenericServer::getPort() { 
   return s_port; 
}


#endif // GENERICSERVER_H

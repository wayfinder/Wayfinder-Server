/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_PROCESSOR_H
#define TILE_PROCESSOR_H

#include "Processor.h"
#include "InterfaceHandleIO.h"
#include "SelectableInterfaceIO.h"
#include "SSLSocket.h"
#include <memory>
#include <set>

// Foraward declarations
class SendDataRequestPacket;
class SendDataReplyPacket;
class SSL_CONTEXT;
class SendDataInterfaceRequest;


/**
 * Processes Communication RequestPackets. 
 *
 */
class CommunicationProcessor : public Processor,
                               public InterfaceHandleIO
{
   public:
      /**
       * Creates a new CommunicationProcessor with a vector of loaded maps
       *
       * @param loadedMaps The standard list of loaded maps for modules.
       */
      CommunicationProcessor( MapSafeVector* loadedMaps );

      /**
       * Destructor.
       */
      virtual ~CommunicationProcessor();
       
      /**
       * Returns the status of the module.
       *
       * @return 0.
       */
      int getCurrentStatus();
     
      /**
       * Put a InterfaceRequest to handle, put ready and new
       * InterfaceRequests here.
       * Monitor method.
       *
       * @param ireq The InterfaceRequest to handle, set to NULL if
       *             put into this else caller must handle it by
       *             making a minimal effort returning an 
       *             "Server overloaded" error.
       * @return Queue full number, the amount of irequests that are 
       *         over the full limit.
       */
      virtual int putInterfaceRequest( InterfaceRequest*& ireq );

      /**
       * Handle a done InterfaceRequest, default implementation here
       * simply deletes it.
       *
       * @param ireply The InterfaceRequest that is done.
       */
      virtual void handleDoneInterfaceRequest( InterfaceRequest* ireply );

   protected:
      /**
       * Handles a request and returns a reply, can be NULL.
       *
       * @param p The packet to handle.
       * @return A reply to p or NULL if p has unknown packet type.
       */
      Packet* handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo );

   private:
      /**
       * Handle a SendDataRequest.
       */
      SendDataReplyPacket* handleSendDataRequest( 
         const SendDataRequestPacket* req, char* packetInfo );

      /**
       * The SelectableInterfaceIO.
       */
      auto_ptr<SelectableInterfaceIO> m_io;

      /**
       * The SSL_CONTEXT.
       */
#ifdef USE_SSL
      auto_ptr<SSL_CONTEXT> m_ctx;
#endif

      /**
       * Class for holding a SendDataInterfaceRequest
       */
      class SDIRHolder {
         public:
            /**
             * Constructor.
             */
            SDIRHolder( SendDataInterfaceRequest* s );

            /**
             * Less than operator.
             */
            bool operator < ( const SDIRHolder& o ) const;

            /**
             * Get the time when timeout ends.
             */
            uint32 timeOutEnd() const;

            /**
             * Get the SendDataInterfaceRequest pointer.
             */
            SendDataInterfaceRequest* getS();

         private:
            /// The SendDataInterfaceRequest.
            SendDataInterfaceRequest* m_s;

            /// Time of start queueing.
            uint32 m_queueStartTime;
      };

      typedef multiset< SDIRHolder > SDIRQueue;
     
      /**
       * The queue of requests waiting for resending.
       */
      SDIRQueue m_queue;
};

#endif


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "config.h"
#include "Request.h"
#include "RequestContainer.h"
#include "RequestList.h"
#include "ISABThread.h"
#include "ServerTypes.h"

class PacketResendHandler;


/**
 * Handles Requests using a PacketResendHandler.
 *
 */
class RequestHandler {
   public:
      /**
       * Handles a set of outstanding requests.
       * Has one thread.
       *
       * @param handler The PacketResendHandler to send packets with.
       * @param serverType the server type
       */
      RequestHandler( PacketResendHandler* handler, ServerTypes::servertype_t serverType);


      /**
       * Destructor, notifies threads waiting in getAnswer().
       */
      ~RequestHandler();


      /**
       * Starts the thread.
       */
      void start();
      
      /**
       * Returns a answer, waits if no answer available. 
       * Use notifyAnswer to release threads waiting here.
       * AnswerMonitor function.
       *
       * @return a answer, or NULL if notified.
       */
      RequestContainer* getAnswer();


      /**
       * Awakens all threads waiting in getAnswer();
       * AnswerMonitor function.
       */
      void notifyAnswer();


      /**
       * Inserts a new Request to be processed.
       * Monitor function.
       *
       * @param rc RequestContainer to insert.
       */
      void insert( RequestContainer* rc );

   private:
      /**
       * Adds an answer
       * AnswerMonitor function.
       */
      void addAnswer( RequestContainer* rc );


      /**
       * Processes a answer packet, sends all new packets and returns
       * request if its done.
       * Monitor function.
       *
       * @param pc The PacketContainer with the answer.
       * @return The request if its done, NULL otherwise.
       */
      RequestContainer* processPacket( PacketContainer* cont );

   
      /**
       * Removes and reuturns the request with requestID reqID.
       * 
       * @param reqID The id of the request to remove.
       */
      RequestContainer* removeRequest( uint16 reqID );

      
      /** 
       * Returns a answer from the PacketResendHandler. Might be NULL.
       */
      inline PacketContainer* getPacketAnswer();


      /**
       * Checks for done requests and returns the first, might return NULL.
       * Monitor function.
       *
       * @return Done request, or NULL if not done requests.
       */
      RequestContainer* checkDoneRequest();


      /** 
       *   Class to handle processing of packets from the
       *   PacketResendHandler in the RequestHandler.
       */
      class RequestThread : public ISABThread {
         public:
            /**
             *   Create a new RequestThread.
             *   @param   handler   The RequestHandler this is part of.
             */
            RequestThread( RequestHandler* handler );


            /**
             *   Delete the RequestThread.
             */
            virtual ~RequestThread();


            /**
             *   Start the RequestThread.
             */
            virtual void run();


         private:
            /**   
             *   The RequestHandler this RequestThread is part of.
             */
            RequestHandler* m_handler;
      };
      
      
      /**
       * The RequestThread.
       */
      RequestThread* m_requestThread;

      
      /**
       * If the system is going down.
       */
      bool m_running;

      
      /**
       * The packet resend handler.
       */
      PacketResendHandler* m_handler;


      /**
       *   List of pending requests.
       */
      RequestList* m_requestList;


      /**
       *   List of processed requests.
       */
      RequestList* m_answerList;

      /// the server type
      ServerTypes::servertype_t m_serverType;

      /**
       * Monitor protecting handler and list.
       */
      ISABMonitor m_monitor;


      /**
       * Monitor protecting answerlist.
       */
      ISABMonitor m_answerMonitor;


      /// RequestThread is a friend
      friend class RequestThread;
};


#endif // REQUEST_HANDLER_H

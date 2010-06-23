/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTERFACEREQUEST_H
#define INTERFACEREQUEST_H

#include "config.h"

class InterfaceHandleIO;

/**
 * Abstract super class for requests comming from outside mc2.
 *
 */
class InterfaceRequest {
   public:
      /**
       * The states of an InterfaceRequest.
       */
      enum state_t {
         /// Ready to do IO for receiving request
         Ready_To_IO_Request = 1,
         /// Ready to do IO for sending reply
         Ready_To_IO_Reply = 2,
         /// Has a complete request that is ready to be processed
         Ready_To_Process = 3,
         /// Done, all done
         Done = 4, 
         /// Error, something went wrong
         Error = 5,
         /// Timeout on IO when receiving request
         Timeout_request = 6,
         /// Timeout on IO when sending reply
         Timeout_reply = 7,
         /// Not yet initialized
         Uninitalized = 8,
      };
   

      /**
       * Creates a new InterfaceRequest.
       */
      InterfaceRequest();


      /**
       * Destructor.
       */
      virtual ~InterfaceRequest();


      /**
       * The currect state of the InterfaceRequest.
       */
      inline state_t getState() const;


      /**
       * The state as string.
       *
       * @param state The state to return string for.
       */
      static const char* getStateAsString( state_t state );


      /**
       * The irequest should try to get done as soon as possible.
       * This means that reuse of IO for another read of user request
       * is not wanted.
       * Default is to do nothing.
       */
      virtual void terminate();


      /**
       * Get the start time of this request, in ms.
       */
      uint32 getStartTime() const;


      ///  Sets the queue start time to now.
      void startQueueTime();
      

      /**
       *   Get the time that this request has been in the queue
       *   provided that startQueueTime() was called when it was
       *   put into the queue.
       */
      uint32 getTimeFromPutInQueueMs() const;


      /**
       * Get the startQueueTime value.
       */
      uint32 getStartQueueTime() const;


      /**
       * Get request priority. Default is zero.
       */
      virtual int getPriority( const InterfaceHandleIO* g ) const;


   protected: 
      /**
       * Set the currect state of the InterfaceRequest.
       */
      inline void setState( state_t state );


      /// The time when the request was put into the queue
      uint32 m_startQueueTime;


   private:
      /// The state of the request
      state_t m_state;


      /// The start time of the request, in ms.
      uint32 m_startTime;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline InterfaceRequest::state_t 
InterfaceRequest::getState() const {
   return m_state;
}


inline void 
InterfaceRequest::setState( state_t state ) {
   m_state = state;
}


inline uint32 
InterfaceRequest::getStartTime() const {
   return m_startTime;
}


#endif // INTERFACEREQUEST_H



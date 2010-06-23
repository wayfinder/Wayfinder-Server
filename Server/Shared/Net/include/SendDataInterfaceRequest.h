/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SENDDATAINTERFACEREQUEST_H
#define SENDDATAINTERFACEREQUEST_H

#include "config.h"
#include "MC2String.h"
#include "SocketInterfaceRequest.h"
#include "URL.h"
#include <vector>

// Forward declatations
class SSL_CONTEXT;
class TCPSocket;

/**
 * Class for sending data to outside of mc2.
 *
 */
class SendDataInterfaceRequest : public SocketInterfaceRequest {
   public:
      /**
       * Creates a new InterfaceRequest.
       *
       * @param peer The peer to connect and send data too.
       * @param data The data to send, is copied.
       * @param dataLen The length of data.
       * @param expectedReplyData The regular expression required in the 
       *                          reply from the peer.
       */
      SendDataInterfaceRequest( const URL& peer,
                                const byte* data, uint32 dataLen,
                                const char* expectedReplyData,
#ifdef USE_SSL
                                SSL_CONTEXT* SSLContext,
#endif
                                uint32 queueTimeout );

      /**
       * Destructor.
       */
      virtual ~SendDataInterfaceRequest();

      /**
       * The irequest should try to get done as soon as possible.
       * This means that reuse of IO for another read of user request
       * is not wanted, Keep-Alive is set to close.
       */
      virtual void terminate();

      /**
       * Get request priority. May be set if special conditions.
       */
      virtual int getPriority( const InterfaceHandleIO* g ) const;

      /**
       * Read and or write is ready make the IO.
       * May only call read/write once and only if 
       * readyRead/readyWrite is true.
       */
      virtual void handleIO( bool readyRead, bool readyWrite );

      /**
       * There was a timeout, change state.
       */
      virtual void timedout();

      /**
       * The server is overloaded make mimimal reply with that status.
       */
      virtual void handleOverloaded( int overLoad );

      /**
       * If the request idle right now. Waiting for client to reuse
       * connection.
       */
      virtual bool isIdle() const;

      /**
       * Resets the state and data.
       */
      void reset();

      /**
       * Get the peer.
       */
      const URL& getPeer() const;

      /**
       * Get the size of the request.
       */
      uint32 getRequestSize() const;

      /**
       * Get the request buffer.
       */
      const byte* getRequestBuffer() const;

      /**
       * The reply data.
       */
      const vector<byte>& getReplyBytes() const;

      /**
       * Set the maximum socket wait time.
       *
       * @param val The new timeout in microseconds.
       */
      static void setMaxSocketWaitTime( uint32 val );

      /**
       * Prints the current information about the request.
       */
      virtual ostream& dumpState( ostream& out ) const;

      /**
       * Get the queue timeout.
       */
      uint32 getQueueTimeout() const;

      /**
       * Set the queue timeout.
       */
      void setQueueTimeout( uint32 t );

      /**
       * Get the expected regular expression.
       */
      const MC2String& getExpectedReplyData() const;

   protected: 
      /**
       * Max time to wait for data on socket in microseconds.
       */
      static uint32 MAX_SOCKET_WAITTIME;
   
   private:
      /**
       * The IO states.
       */
      enum io_state_t {
         /// Connecting
         connecting = 0,
         /// Initial handchake
         initial_hand_shake = 1,
         /// Writing request 
         writing_request = 2,
         /// Reading reply
         reading_reply = 3,
         /// Done
         io_done = 4,
         /// Error
         io_error = 5,
         /// Uninitalized
         io_uninitalized = 6
      } m_iostate;

      /**
       * The io state as string.
       *
       * @param state The io state to return string for.
       */
      static const char* getIOStateAsString( io_state_t state );

      /**
       * The request as bytes.
       */
      byte* m_requestBuffer;

      /**
       * The total size of the request buffer.
       */
      uint32 m_requestSize;

      /**
       * The position in the request buffer.
       */
      uint32 m_requestPos;

      /**
       * The reply data.
       */
      vector<byte> m_replyBytes;

      /**
       * The peer.
       */
      URL m_peer;

      /**
       * The queue timeout, used outside of request.
       */
      uint32 m_queueTimeout;

      /**
       * The expectedReplyData.
       */
      MC2String m_expectedReplyData;
};


// =======================================================================
//                                     Implementation of inlined methods =

inline const URL&
SendDataInterfaceRequest::getPeer() const {
   return m_peer;
}

inline uint32 
SendDataInterfaceRequest::getRequestSize() const {
   return m_requestSize;
}

inline const byte* 
SendDataInterfaceRequest::getRequestBuffer() const {
   return m_requestBuffer;
}

inline const vector<byte>&
SendDataInterfaceRequest::getReplyBytes() const {
   return m_replyBytes;
}

inline uint32
SendDataInterfaceRequest::getQueueTimeout() const {
   return m_queueTimeout;
}

inline void
SendDataInterfaceRequest::setQueueTimeout( uint32 t ) {
   m_queueTimeout = t; 
}

inline const MC2String&
SendDataInterfaceRequest::getExpectedReplyData() const {
   return m_expectedReplyData;
}

#endif // SENDDATAINTERFACEREQUEST_H


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABBOXINTERFACEREQUEST_H
#define ISABBOXINTERFACEREQUEST_H

#include "config.h"
#include "NavInterfaceRequest.h"


// Forward
class DataSocket;
class isabBoxSession;
class isabBoxNavMessage;
class NavRequestPacket;
class NavReplyPacket;
class HttpInterfaceRequest;
class HttpHeader;
class HttpBody;


/**
 * Receives and sends isabbox packets.
 *
 */
class IsabBoxInterfaceRequest : public NavInterfaceRequest {
   public:
      /**
       * Creates a new NavInterfaceRequest.
       */
      IsabBoxInterfaceRequest( DataSocket* dataSocket );


      /**
       * Creates a new NavInterfaceRequest.
       */
      IsabBoxInterfaceRequest( HttpInterfaceRequest* ireq,
                               HttpHeader*& outHead, 
                               HttpBody*& outBody );


      /**
       * Destructor.
       */
      virtual ~IsabBoxInterfaceRequest();


      /**
       * The irequest should try to get done as soon as possible.
       * This means that reuse of IO for another read of user request
       * is not wanted.
       */
      virtual void terminate();


      /**
       * The selectable
       */
      virtual Selectable* getSelectable();


      /**
       * If read is wanted.
       */
      virtual bool wantRead() const;


      /**
       * If write is wanted.
       */
      virtual bool wantWrite() const;


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
       * If the request idle right now. Waiting for client to reuse
       * connection.
       */
      virtual bool isIdle() const;


      /**
       * Get the request NavMessage. Might be NULL.
       *
       * @return The request message.
       */
      virtual NavMessage* getRequestMessage();


      /**
       * Set the reply NavMessage. Changes state.
       */
      virtual void setReplyMessage( OutGoingNavMessage* reply );


      /**
       * Reset the state and data.
       */
      void reset();


      /**
       * Get the request data size.
       */
      uint32 getRequestSize() const;

      /**
       * The uncompressed request size.
       */
      uint32 getUncompressedRequestSize() const;

      /**
       * Get the reply data size.
       */
      uint32 getReplySize() const;

      /**
       * The uncompressed reply size.
       */
      uint32 getUncompressedReplySize() const;

      /**
       * Get pointer to the reply data.
       */
      const byte* getReplyData() const;


      /**
       * Get the request NavPacket, might be NULL.
       * Version 10+.
       */
      NavRequestPacket* getRequestPacket();


      /**
       * Set the reply NavPacket. Changes state.
       * Version 10+.
       */
      void setReplyPacket( NavReplyPacket* reply );


      /**
       * Get the session.
       */
      isabBoxSession* getSession();


      /**
       * Get the HttpInterfaceRequest that this isabBox request 
       * came in, is NULL if this request didn't come via HTTP.
       *
       * @return The HttpInterfaceRequest, NULL if none.
       */
      HttpInterfaceRequest* getHttpRequest();


      /**
       * Decode a buffer.
       */
      static void decodeBuffer( byte* buff, uint32 len );


      /**
       * Encode a buffer.
       */
      static void encodeBuffer( byte* buff, uint32 len );


   private:
      /**
       * The IO states.
       */
      enum io_state_t {
         /// Reading STX
         reading_stx = 0,
         /// Reading header start
         reading_header = 1,
         /// Reading rest of request
         reading_request_data = 2,
         /// Writing reply
         writing_reply = 10,
         /// Done
         io_done = 19,
         /// Error
         io_error = 20,
         /// Uninitalized
         io_uninitalized = 21
      } m_iostate;


      /**
       * The io state as string.
       *
       * @param state The io state to return string for.
       */
      static const char* getIOStateAsString( io_state_t state );


      /// The datasocket with session
      DataSocket* m_dataSocket;


      /// The session.
      isabBoxSession* m_session;


      /// STX (1byte) + length (4bytes) + protover (1byte)
      static const uint32 startHeaderSize;


      /// Max size for reply message
      static const uint32 maxReplyMsgSize;


      /// The header of the request storage
      byte m_startHeader[ 6 ];


      /// The request buffer;
      byte* m_requestBuff;


      /// The number of read bytes for request
      uint32 m_requestPos;


      /// The total size of the request used in reading_request_data
      uint32 m_requestSize;


      /// The reply buffer;
      byte* m_replyBuff;


      /// The total size of the reply used in writing_reply
      uint32 m_replySize;


      /// The position n the reply buffer
      uint32 m_replyPos;


      /// The request message
      NavMessage* m_navMess;


      /// The NavRequestPacket, v10+
      NavRequestPacket* m_reqPack;


      /// The NavReplyPacket, v10+
      NavReplyPacket* m_replyPack;

      /// The HttpInterfaceRequest, if any.
      HttpInterfaceRequest* m_httpRequest;

      /// The uncompressed request size.
      uint32 m_uncompressedRequestSize;

      /// The uncompressed reply size.
      uint32 m_uncompressedReplySize;

      /// TcpInterfaceFactory is a friend
      friend class TcpInterfaceFactory;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline bool 
IsabBoxInterfaceRequest::wantRead() const {
   return getState() == Ready_To_IO_Request;
}


inline bool 
IsabBoxInterfaceRequest::wantWrite() const {
   return getState() == Ready_To_IO_Reply;
}


inline isabBoxSession* 
IsabBoxInterfaceRequest::getSession() {
   return m_session;
}


inline const byte*
IsabBoxInterfaceRequest::getReplyData() const {
   return m_replyBuff;
}


inline HttpInterfaceRequest* 
IsabBoxInterfaceRequest::getHttpRequest() {
   return m_httpRequest;
}


#endif // ISABBOXINTERFACEREQUEST_H



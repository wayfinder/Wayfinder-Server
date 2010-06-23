/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPINTERFACEREQUEST_H
#define HTTPINTERFACEREQUEST_H

#include "config.h"
#include "MC2String.h"
#include "SocketInterfaceRequest.h"
#include "IPnPort.h"

// Forward declarations
class HttpHeader;
class HttpBody;


/**
 * Class for requests comming from outside mc2 that uses HTTP.
 * Reads the header and body from a socket.
 *
 */
class HttpInterfaceRequest : public SocketInterfaceRequest {
   public:
      /**
       * Creates a new InterfaceRequest.
       *
       * @param reqSock The TCPSocket to communicate with.
       */
      HttpInterfaceRequest( TCPSocket* reqSock, const IPnPort& serverAddress );

      /**
       * Destructor.
       */
      virtual ~HttpInterfaceRequest();

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
       * Make a 503 reply.
       */
      virtual void handleOverloaded( int overLoad );

      /**
       * If the request idle right now. Waiting for client to reuse
       * connection.
       */
      virtual bool isIdle() const;

      /**
       * Return the header of the request.
       */
      HttpHeader* getRequestHeader() const;

      /**
       * Return the body of the request.
       */
      HttpBody* getRequestBody();

      /**
       * Sets the http reply, is now owned by this class.
       */
      void setReply( HttpHeader* outHead, HttpBody* outBody );

      /**
       * Sets a standard reply indicated by the statusNbr.
       * Returns false if the statusNbr is unknown and the standard
       * error "500 Internal Server Error" is sent.
       * 
       * @param statusNbr is the number of the standard reply.
       * @param retryTime Used if statusNbr == 503 to tell client how
       *                  long to wait before retrying.
       * @param extraHeaderFields A list of extra header fields to send
       *                          with the reply, default NULL.
       */
      bool setStatusReply( uint32 statusNbr, uint32 retryTime = 0,
                           vector< MC2String >* extraHeaderFields = NULL );

      /**
       *   Returns the HTTP status code of the reply.
       */
      uint32 getStatusReply() const {
         return m_replyCode;
      }

      /**
       * Resets the state and data.
       */
      void reset();

      /**
       * Returns true if usig SSL.
       */
      bool isHttps() const;

      /**
       * The peer of the connection.
       */
      IPnPort getPeer() const;

      const IPnPort& getServerAddress() const;

      /**
       * The number of requests served.
       */
      uint32 getNbrReuses() const;

      /**
       * Get the size of the current reply.
       */
      uint32 getReplySize() const;

      /**
       * Get the reply buffer.
       */
      const byte* getReplyBuffer() const;

      /**
       * Max number of reuses of a connection.
       */
      static uint32 MAX_NUMBER_SOCKET_REUSES;

      /**
       * If to keep connection open.
       */
      bool keepConnection( uint32 statusNbr ) const;

      /**
       * Set the username to use in logprefix, updates logprefix.
       *
       * @param userName The new username, use NULL to unset username.
       */
      void setLogUserName( const char* userName );

      /**
       * Set the maximum socket wait time.
       *
       * @param val The new timeout in microseconds.
       */
      static void setMaxSocketWaitTime( uint32 val );

      /**
       * Set the maximum nbr socket reuses.
       *
       * @param val The new maximum nbr socket reuses.
       */
      static void setMaxSocketReuses( uint32 val );

      /**
       * Prints the current information about the request.
       */
      virtual ostream& dumpState( ostream& out ) const;

   protected: 
      /**
       * How many times the connection has been reused, if any.
       */
      uint32 m_reusedConnection;

      /**
       * Max time to wait for data on socket in microseconds.
       */
      static uint32 MAX_SOCKET_WAITTIME;

      /**
       * Max time to wait for data on a reused connection in microseconds.
       */
      static uint32 MAX_REUSED_SOCKET_WAITTIME;

      /**
       * Maximum length of a line. Maximum size of incomming body is 
       * 2^8 times max line length.
       */
      static const uint32 MAX_LINE_LENGTH;
   
   private:
      /**
       * The IO states.
       */
      enum io_state_t {
         /// Initial handchake
         initial_hand_shake = 0,
         /// Reading start line
         reading_start_line = 1,
         /// Reading header
         reading_header = 2,
         /// Reading body
         reading_request_body = 3,
         /// Reading body chunked
         reading_request_body_chunked = 4,
         
         /// Writing reply header
         writing_reply_header = 10,
         /// Writing reply body
         writing_reply_body = 11,
         /// Done
         io_done = 19,
         /// Error
         io_error = 20,
         /// Uninitalized
         io_uninitalized = 21
      } m_iostate;

      /**
       * The chunked body states.
       */
      enum chunked_body_t {
         /// Reading chunk size
         reading_chunk_size = 0,
         /// Reading a chunk
         reading_chunk = 1,
         /// Reading CRLF after a chunk
         reading_chunk_crlf = 2,
         /// Reading trailer
         reading_chunk_trailer = 3,
      } m_chunkedstate;

      /**
       * The io state as string.
       *
       * @param state The io state to return string for.
       */
      static const char* getIOStateAsString( io_state_t state );

      /**
       * Makes the m_replyBuffer from m_replyHeader and m_replyBody.
       */
      void makeReplyBuffer();

      /**
       * Handles bytes in request state.
       */
      int handleRequestBytes( bool readyRead, bool readyWrite );

      /**
       * Get some bytes from socket.
       */
      int getBytes( byte* buff, size_t length );

      /**
       * Get the currect HTTP-version as string like "HTTP/1.0 ".
       */
      const char* getCurrHTTPVer() const;

      /**
       * The current line of the header.
       */
      MC2String m_currLine;

      /**
       * The last line of the header.
       */
      MC2String m_lastHeaderLine;

      /**
       * The request header.
       */
      HttpHeader* m_requestHeader;

      /**
       * The length of the request body, 0 means read until closed.
       */
      uint32 m_bodyRequestLength;

      /**
       * The request body.
       */
      HttpBody* m_requestBody;

      /**
       * The reply header.
       */
      HttpHeader* m_replyHeader;

      /**
       * The reply body.
       */
      HttpBody* m_replyBody;

      /**
       * The reply as bytes.
       */
      byte* m_replyBuffer;

      /**
       * The total size of the reply buffer.
       */
      uint32 m_replySize;

      /**
       * The position in the reply buffer.
       */
      uint32 m_replyPos;

      /**
       * The peer IPnPort.
       */
      IPnPort m_IPnPort;

      /**
       *
       */
      IPnPort m_serverAddress;

      /**
       * The reply status code.
       */
      uint32 m_replyCode;

      /**
       * HttpInterfaceFactory is a friend.
       */
      friend class HttpInterfaceFactory;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline HttpHeader*
HttpInterfaceRequest::getRequestHeader() const {
   return m_requestHeader;
}


inline HttpBody* 
HttpInterfaceRequest::getRequestBody() {
   return m_requestBody;
}


inline IPnPort 
HttpInterfaceRequest::getPeer() const {
   return m_IPnPort;
}

inline const IPnPort&
HttpInterfaceRequest::getServerAddress() const {
   return m_serverAddress;
}

inline uint32 
HttpInterfaceRequest::getNbrReuses() const {
   return m_reusedConnection;
}


inline uint32 
HttpInterfaceRequest::getReplySize() const {
   return m_replySize;
}


inline const byte* 
HttpInterfaceRequest::getReplyBuffer() const {
   return m_replyBuffer;
}


#endif // HTTPINTERFACEREQUEST_H


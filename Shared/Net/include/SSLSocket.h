/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SSLSOCKET_H
#define SSLSOCKET_H

#include "config.h"
#include "sockets.h"
#include "TCPSocket.h"
#include "NotCopyable.h"

#ifdef USE_SSL
// This should be enough for openssl
#include <openssl/ssl.h>
// some more functions, currently only used under solaris
#include <openssl/rand.h>
// Reading pem formated certificates and keys
#include <openssl/pem.h>
// Error printing
#include <openssl/err.h>
// Compression
#include <openssl/comp.h>
// CLient side session caching
#include "BinarySearchTree.h"


/**
 * System independant ssl-socket interface. 
 * SSL = Secure sockets layer.
 * The context holds information about chiphers and sessions.
 */
class SSL_CONTEXT: private NotCopyable {
   public:
      /**
       * Creates a new context.
       * @param SSL_CTX The SSL context to use.
       */
      SSL_CONTEXT( SSL_CTX* ctx );


      /**
       * Destructor calls freeCTX for the SSL_CTX.
       */
      ~SSL_CONTEXT();

      /**
       * Call this to cleanup at application exit.
       */
      void exitCleanup();

      /**
       * Sets the clientConnections.
       */
      inline void setClientConnections( 
         BinarySearchTree* clientConnections );
      
      
      /**
       * Returns the clientConnections.
       */
      inline BinarySearchTree* getClientConnections();


      /**
       * Reutrns the SSL_CTX.
       */
      inline SSL_CTX* getCTX();


   private:
      /**
       * The SSL_CTX.
       */ 
      SSL_CTX* m_ctx;


      /**
       * The clients old connection sessions to reuse when connecting.
       */
      BinarySearchTree* m_clientConnections;
};

/**
 * Helps to clean up ssl stuff on exit.
 */
class SSLCleaner {
public:
   SSLCleaner();
   ~SSLCleaner();
};

/**
 *    System independant ssl-socket interface. SSL = Secure sockets layer.
 *
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 *    @todo Check Server certificate against a set of authorized CA servers 
 *          in sslConnect.
 *
 */
class SSLSocket : public TCPSocket {
   public:
      /**
       * Type of SSL Sockets.
       * If you use SSL_SOCKET_CLIENT then you don't have to enter
       * certificates or keys.
       */
      enum ssl_socket_t {
         /// If the Sockets should connect.
         SSL_SOCKET_CLIENTS,
         /// If the sockets should connect using TLSv1
         SSL_SOCKET_CLIENTS_TLSv1,
         /// If the Sockets should accept.
         SSL_SOCKET_SERVERS,
         /// If the Sockets should accept using TLSv1.
         SSL_SOCKET_SERVERS_TLSv1,
         /// If the Sockets should connect and accept.
         SSL_SOCKET_CLIENTS_AND_SERVERS,
         /// If the Sockets should connect and accept using TLSv1.
         SSL_SOCKET_CLIENTS_AND_SERVERS_TLSv1
      };


      /**
       * Sets up a new SSL_CONTEXT, there should only be one per program.
       * 
       * Default X509 certificates directory usualy is placed with
       * the opensssl distributation.
       *
       * @param server The type of connections the socket can make
       *               connect, accept or both.
       * @param defaultCertDir The directory with the certificates and
       *                       keys, if NULL then the default X509 dir
       *                       is used.
       * @param ssl_public_cert_file_name Filename of the X509 public
       *        cert file in the default cert dir. May be NULL.
       * @param ssl_private_cert_file_name Filename of the X509 private
       *        cert file in the default cert dir. May be NULL.
       * @param ssl_private_key_name Filename of the private RSA key
       *        file in the default cert dir. May be NULL.
       * @param ssl_cipher_list List of cipher used by socket. May be NULL.
       * @param compress Enables compression for connections. Both ends
       *                 must support this or no compression is used.
       *                 Please note that compression is only available
       *                 in the newest SSL specs (SSL3 and TLSv1).
       *                 Default off.
       * @param bugs Enables support for various known bugs in other SSL
       *             implementations. Default on.
       * @return A new SSL_CONTEXT with the specified certificates and keys
       *         loaded.
       */
      static SSL_CONTEXT* makeNewCTX( 
         ssl_socket_t ssl_type,
         const char* defaultCertDir,
         const char* ssl_public_cert_file_name,
         const char* ssl_private_cert_file_name,
         const char* ssl_private_key_file_name,
         const char* ssl_cipher_list,
         bool compress = false,
         bool bugs = true );


      /**
       * Removes a SSL_CTX.
       * @param ctx The SSL_CTX to free.
       */
      static void freeCTX( SSL_CTX* ctx );
      

      /**
       * Create a new socket.
       * @param ctx The SSL Context 
       */
      SSLSocket( SSL_CONTEXT* ctx );


      /**
       * Adopts a already connected SSL socket or just a SOCKET to listen 
       * on.
       * Used when accepting a secure socket.
       *
       * @param sock The socket of the connection.
       * @param ssl The SSL of the connection.
       * @param ctx The context to use.
       */
      SSLSocket( SOCKET sock, SSL* ssl, SSL_CONTEXT* ctx );


      /**
       * Delete this socket. If the socket is open, it is also closed.
       */
      virtual ~SSLSocket();


      /**
       * Close the socket.
       */
      virtual void close();


      /**
       * Only close the TCPSocket don't send anything. Doesn't
       * close the SSLSession by sending closenotify.
       * Used when no write may be done on the socket.
       */
      void nonWriteClose();


      /**
       * Initiate a connection on a tcp socket then adds SSL handshake
       * (calls sslConnect). 
       * Since this method looks
       * up the ip-address from the hostname (gethostbyname(char*)), the
       * other connect-method should be used when possible.
       * 
       * @param hostname The name of the host to connect to.
       * @param portnumber The port number to connect to at hostname.
       * @return True is returned on success, false upon error.
       */
      virtual bool connect( const char *hostname, int portnumber );

      
      /**
       * Initiate a connection on a tcp socket and then makes a 
       * SSLHandshake (calls sslConnect).
       *
       * @param hostip The IP-address of the host to connect to.
       * @param portnumber The port number to connect to at 
       *                   hostname.
       * @return True is returned on success, false upon error.
       */
      virtual bool connect( uint32 hostip, uint16 hostport );


      /**
       *    Initiate a connection on a tcp socket with a timeout . 
       *    Since this method looks
       *    up the ip-address from the hostname (gethostbyname(char*)), the
       *    other connect-method should be used when possible.
       * 
       *    @param   hostname    The name of the host to connect to.
       *    @param   portnumber  The port number to connect to at 
       *                         hostname.
       *    @param   timeout_us  The timeout in micro seconds
       *
       *    @return  True is returned on success, false upon error.
       */
      virtual bool connect( const char *hostname, int portnumber, 
                            uint32 timeout_us );


      /**
       *    Initiate a connection on a tcp socket.
       *
       *    @param   addr      The ip-address and port of the host 
       *                       to connect to.
       *    @param   timeout_us  The timeout in micro seconds
       *
       *    @return  True is returned on success, false upon error.
       */   
      virtual bool connect( const IPnPort& addr, uint32 timeout_us );


      /**
       * Initiate a SSLconnection on a already connected tcp socket.
       *
       * @param timeout_us The timeout in micro seconds, MAX_UINT32 means
       *                   no timeout.
       * @return True if connected ok false if not.
       */
      bool sslConnect( uint32 timeout_us );
      
      
      /**
       * Accept a connection. 
       * @return  A SSLSocket that is connected to a socket at the 
       *          caller. This socket must be deleted by the caller
       *          to accept.
       */
      virtual TCPSocket* accept();

      
      /**
       * Initializes a socket using reads/writes after an accept.
       * This calls the ssl-handshake and returns true if handshake was
       * ok, false if not.
       * @param micros The timeout value of this function. If
       *               no data is received for micros microseconds
       *               the function will timeout.
       *               Default value is TCPSOCKET_INFINITE_TIMEOUT.
       * @return True if socket now is ready for reading and writing.
       */
      virtual bool initialHandshake( 
         long micros = TCPSOCKET_INFINITE_TIMEOUT );


      /**
       * Initializes a socket using reads/writes after an accept.
       * This calls the ssl-handshake and returns 1 if done and
       * -4 for want read -5 for want write, 0 for closed.
       *
       * @return The result of the accept -1 on error.
       */
      virtual int initialHandshaking();


      /**
       * Set the certificate and key. Mostly used to set client certificate.
       * Used in connect when setting up SSL connection.
       *
       * @param certificateFile Path to file with certificate in pem format.
       * @param keyFile Path to file with private key, for certificate, in 
       *                pem format.
       */
      void setCertificateAndKey( const char* certificateFile,
                                 const char* keyFile );

   protected:
      /**
       * Reads from the socket taking care of EINTR.
       * @param buffer The buffer to put the result into.
       * @param size   The maximum size to read.
       * @return The number of successfully read bytes or -1
       *         upon failure, 0 on closed socket and -3 on EAGAIN.
       *         EAGAIN can only happen on non blocking sockets.
       *         A special -4 is returned if SSL-socket wants to write
       *         to the socket, this is like EAGAIN but write is wanted.
       */
      virtual ssize_t protectedRead( byte* buffer,
                                     size_t size );
      
      
      /**
       * Writes to the socket taking care of EINTR.
       * @param buffer The buffer to write.
       * @param size   The maximum size to write.
       * @return The number of successfully writen bytes or -1
       *         upon failure, 0 on closed socket and -3 on EAGAIN.
       *         EAGAIN can only happen on non blocking sockets.
       *         A special -4 is returned if SSL-socket wants to read
       *         to the socket, this is like EAGAIN but read is wanted.
       */
      virtual ssize_t protectedWrite( const byte* buffer,
                                      size_t size );


      /**
       * Checks if there is enough cached data on the socket to
       * fill the buffer. The implementation here in SSLSocket 
       * checks for pending bytes from the last received SSL-packet.
       *
       * @param buffer The buffer to write cached data into.
       * @param size The amount of data to get.
       * @return True if size bytes was written into buffer from cache,
       *         false if not.
       */
      virtual bool checkForCachedData( byte* buffer, size_t size );

   protected:
      /**
       * The SSL context of this SSLSocket.
       */
      SSL_CONTEXT* m_ctx;


   private:
      /**
       * Sets up the socket.
       * @param ctx The SSL context to use.
       */
      void init( SSL_CONTEXT* ctx );

      
      /**
       * The SSL connection of this SSLSocket.
       */
      SSL* m_ssl;


      /**
       * Holds a SSL_SESSION id for a connection to a server, port.
       * 
       */
      class clientHostSession : public BinarySearchTreeNode {
         public:
            /**
             * A new connection.
             * @param IP The IP of the server.
             * @param port The port on the server.
             * @param sessionID The SSL_SESSION id negociated, is copied
             *                  into the clientHostSession.
             */
            clientHostSession( uint32 IP, uint16 port, 
                               const SSL_SESSION* const session );

            
            /**
             * Deletes the sessionID.
             */
            ~clientHostSession();


            /**
             *    @name Operators.
             *    Implementation of some operators.
             */
            //@{
               /// True if node's key is higher. 
               virtual bool
                  operator >  (const BinarySearchTreeNode &node) const;

               /// True if node's key is lower. 
               virtual bool
                  operator <  (const BinarySearchTreeNode &node) const;

               /// True if keys match.
               virtual bool
                  operator == (const BinarySearchTreeNode &node) const;
            //@}


            /**
             * The session of this clientHostSession.
             */
            inline const SSL_SESSION* const getSession();


            /**
             * Sets the session of this clientHostSession.
             */
            void setSession( const SSL_SESSION* const session );


         private:
            /**
             * The IP of the server.
             */
            uint32 m_IP;

            
            /**
             * The port of the server.
             */
            uint16 m_port;

            
            /**
             * The session.
             */
            const SSL_SESSION* m_session;
      };

      /**
       * The temporary CallBack RSA key used in getTmpRSACallback,
       * 512 bit version
       */
      static RSA* tmpRSA512;


      /**
       * The temporary CallBack RSA key used in getTmpRSACallback,
       * 1024 bit version
       */
      static RSA* tmpRSA1024;


      /**
       * Returnes the temporary RSACallback.
       * Used for SSL_CTX_set_tmp_rsa_callback function.
       */
      static RSA* getTmpRSACallback( SSL* ssl, 
                                     int nExport, 
                                     int nKeyLength );


      /**
       * The certificate to use. May be NULL.
       */
      MC2String m_certificateFile;


      /**
       * The private key of the certificate to use. May be NULL.
       */
      MC2String m_keyFile;
};


// ========================================================================
//                                  Implementation of the inlined methods =


const SSL_SESSION* const 
SSLSocket::clientHostSession::getSession() {
   return m_session;
}


void
SSL_CONTEXT::setClientConnections( BinarySearchTree* clientConnections ) {
   delete m_clientConnections;
   m_clientConnections = clientConnections;
}


BinarySearchTree* 
SSL_CONTEXT::getClientConnections() {
   return m_clientConnections;
}


SSL_CTX* 
SSL_CONTEXT::getCTX() {
   return m_ctx;
}


#endif // USE_SSL

#endif // SSLSOCKET_H

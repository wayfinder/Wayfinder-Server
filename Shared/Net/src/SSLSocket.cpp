/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SSLSocket.h"
#include "FilePtr.h"
#include "DebugClock.h"
#include "openssl/opensslv.h"

#ifdef USE_SSL

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define SESSION_LHASH LHASH_OF(SSL_SESSION)
#define SESSION_LHASH_RETRIEVE( lh, data ) LHM_lh_retrieve(SSL_SESSION, lh, data )
#else
#define SESSION_LHASH struct lhash_st
#define SESSION_LHASH_RETRIEVE lh_retrieve
#endif

SSLCleaner::SSLCleaner() {
}
SSLCleaner::~SSLCleaner() {

#if OPENSSL_VERSION_NUMBER >= 0x00907001
   CRYPTO_cleanup_all_ex_data();
#endif
   //CONF_modules_free();
   ERR_free_strings();
   EVP_cleanup();
}

RSA* SSLSocket::tmpRSA512 = NULL;
RSA* SSLSocket::tmpRSA1024 = NULL;


//----------------------------------------------------------------------
// Thread locking
//----------------------------------------------------------------------

// We have one extra mutex for client session caching in sslConnect

// Sets up the thread locking
void thread_setup(void);


// Cleans up the thread locking
void thread_cleanup(void);


static bool thread_initialized = false;


#if defined(_WIN32) || defined(__CYGWIN32__)
#include <windows.h>

static HANDLE* lock_cs;


void win32_locking_callback( int mode, int type, 
                             const char *file, int line ) 
{
   if ( (mode & CRYPTO_LOCK) ) {
      WaitForSingleObject( lock_cs[ type ], INFINITE );
   } else {
      ReleaseMutex(        lock_cs[ type ] );
   }
}


void thread_setup(void) {
   int i;
   if ( thread_initialized ) {
      return;
   }
   thread_initialized = true;

   lock_cs = (HANDLE*) OPENSSL_malloc( 
      ( CRYPTO_num_locks() + 1 ) * sizeof( HANDLE ) ) );
   for ( i = 0 ; i <= CRYPTO_num_locks() ; i++ ) {
      lock_cs[ i ] = CreateMutex( NULL, FALSE, NULL );
   }

   CRYPTO_set_locking_callback( (void (*)(int, int, const char *, int))
                                win32_locking_callback );
   // id callback is not needed on Windows nor on platforms where getpid()
   // returns a different ID for each thread

   mc2log << info
          << "[SSLSocket]  This product includes software developed by the "
          << endl << info 
          << "[SSLSocket] OpenSSL Project for use in the OpenSSL Toolkit "
          << endl << info
          << "[SSLSocket] (http://www.openssl.org/)" << endl;
   mc2log << info 
          << "[SSLSocket] OPENSSL_VERSION_TEXT " << OPENSSL_VERSION_TEXT 
          << endl;
}


void thread_cleanup(void) {
   int i;

   if ( !thread_initialized ) {
      return;
   }
   thread_initialized = false;

   CRYPTO_set_locking_callback( NULL );
   for ( i = 0 ; i <= CRYPTO_num_locks() ; i++ ) {
      CloseHandle( lock_cs[ i ] );
   }
   OPENSSL_free( lock_cs );
}


#elif defined(__unix__) // End WIN32
#include <pthread.h>

static pthread_mutex_t* lock_cs;


void pthreads_locking_callback( int mode, int type, 
                                const char *file, int line ) 
{
   if ( (mode & CRYPTO_LOCK) ) {
      pthread_mutex_lock(   &(lock_cs[ type ]) );
   } else {
      pthread_mutex_unlock( &(lock_cs[ type ]) );
   }
}


unsigned long pthreads_thread_id( void ) {
   unsigned long ret;
   
   ret = (unsigned long) pthread_self();
   return ret;
}


void thread_setup( void ) {
   int i;

   if ( thread_initialized ) {
      return;
   }
   thread_initialized = true;

   lock_cs = (pthread_mutex_t*) OPENSSL_malloc(
      ( CRYPTO_num_locks() + 1 ) * sizeof( pthread_mutex_t ) );
   for ( i = 0 ; i <= CRYPTO_num_locks() ; i++ ) {
      pthread_mutex_init( &(lock_cs[ i ]), NULL );
   }
   
   CRYPTO_set_id_callback( (unsigned long (*)())pthreads_thread_id );
   CRYPTO_set_locking_callback( (void (*)( int, int, const char *, int ))
                                pthreads_locking_callback );

   mc2log << info
          << "[SSLSocket]  This product includes software developed by the "
          << endl << info 
          << "[SSLSocket] OpenSSL Project for use in the OpenSSL Toolkit "
          << endl << info
          << "[SSLSocket] (http://www.openssl.org/)" << endl;
   mc2log << info 
          << "[SSLSocket] OPENSSL_VERSION_TEXT " << OPENSSL_VERSION_TEXT 
          << endl;
}


void thread_cleanup(void) {
   int i;

   if ( !thread_initialized ) {
      return;
   }
   thread_initialized = false;

   CRYPTO_set_locking_callback( NULL );
   for ( i = 0 ; i <= CRYPTO_num_locks() ; i++ ) {
      pthread_mutex_destroy( &(lock_cs[ i ]) );
   }
   OPENSSL_free( lock_cs );
}


#else // End PTHREADS


// If not known threads use no thread locking, it might crash randomly.
void thread_setup( void ) {
   thread_initialized = true;

   mc2log << info
          << "[SSLSocket]  This product includes software developed by the "
          << endl << info 
          << "[SSLSocket] OpenSSL Project for use in the OpenSSL Toolkit "
          << endl << info
          << "[SSLSocket] (http://www.openssl.org/)" << endl;
   mc2log << info 
          << "[SSLSocket] OPENSSL_VERSION_TEXT " << OPENSSL_VERSION_TEXT 
          << endl;
}


void thread_cleanup(void) {
   thread_initialized = false;
}


#endif


//----------------------------------------------------------------------
// SSL_CONTEXT
//----------------------------------------------------------------------

SSL_CONTEXT::SSL_CONTEXT( SSL_CTX* ctx ) {
   m_ctx = ctx;
   m_clientConnections = NULL;
   thread_setup();
}


SSL_CONTEXT::~SSL_CONTEXT() {
   delete m_clientConnections;
   SSLSocket::freeCTX( m_ctx );
   // May have temporary ctxes needs too keep thread locks here.
//   thread_cleanup();
}


void
SSL_CONTEXT::exitCleanup() {
   // As long as we don't call this in ~SSL_CONTEXT above do it here.
   thread_cleanup();
}


//----------------------------------------------------------------------
// SSLSocket
//----------------------------------------------------------------------

SSL_CONTEXT* 
SSLSocket::makeNewCTX( ssl_socket_t ssl_type,
      const char* defaultCertDir,
      const char* ssl_public_cert_file_name,
      const char* ssl_private_cert_file_name,
      const char* ssl_private_key_file_name,
      const char* ssl_cipher_list,
      bool compress,
      bool bugs )
{
   SSL_CTX* ctx = NULL;
   SSL_CONTEXT* context = NULL;

   char filePath[255];
   const char* defaultDir = defaultCertDir;
   char tmpStr[512];
   //Only to remove an anoying warning in Visual Studio.
   strcpy(tmpStr, "");

   // Must run inintialize before using SSL libarary
   SSL_library_init();

   // Make it possible to write helpfull errors.
   SSL_load_error_strings();

   if ( compress ) {
      // Add zlib compression
#ifdef ZLIB
      if ( COMP_zlib()->type != NID_undef ) {
         mc2log << info << "[SSLSocket] Adding compression, name: "
                << COMP_zlib()->name << ",type " << COMP_zlib()->type << endl;
         SSL_COMP_add_compression_method( COMP_zlib()->type, COMP_zlib() );
      } else {
         mc2log << info << "[SSLSocket] makeNewCTX(): zlib not available,"
                   " no compression used." << endl;
         compress = false;
      }
#else
      mc2log << warn << "[SSLSocket] makeNewCTX(): compress is set but "
                "no compression is available.";
      compress = false;
#endif
   }

   // If defaultDir isn't set set it to default.
   if ( defaultDir == NULL ) {
      defaultDir = X509_get_default_cert_dir();
   }

   bool isServerSocket = 
      ssl_type == SSLSocket::SSL_SOCKET_SERVERS ||
      ssl_type == SSLSocket::SSL_SOCKET_SERVERS_TLSv1;
   bool isServerClientSocket = 
      ssl_type == SSLSocket::SSL_SOCKET_CLIENTS_AND_SERVERS ||
      ssl_type == SSLSocket::SSL_SOCKET_CLIENTS_AND_SERVERS_TLSv1;
   bool isTLSv1 = ssl_type == SSLSocket::SSL_SOCKET_SERVERS_TLSv1 ||
      ssl_type == SSLSocket::SSL_SOCKET_CLIENTS_AND_SERVERS_TLSv1 ||
      ssl_type == SSLSocket::SSL_SOCKET_CLIENTS_TLSv1;
   if (isServerSocket) {
      if (isTLSv1) {
         ctx = SSL_CTX_new( TLSv1_server_method() );
      } else {
         ctx = SSL_CTX_new( SSLv23_server_method() );
      }
      context = new SSL_CONTEXT( ctx );
   } else if (isServerClientSocket) {
      if (isTLSv1) {
         ctx = SSL_CTX_new( TLSv1_method() );
      } else {
         ctx = SSL_CTX_new( SSLv23_method() );
      }
      SSL_CTX_set_session_cache_mode( ctx, SSL_SESS_CACHE_BOTH );
      context = new SSL_CONTEXT( ctx );
      context->setClientConnections( new BinarySearchTree() );
   } else { // Client socket
      if (isTLSv1) {
         ctx = SSL_CTX_new( TLSv1_client_method() );
      } else {
         ctx = SSL_CTX_new( SSLv23_client_method() );
      }
      SSL_CTX_set_session_cache_mode( ctx, SSL_SESS_CACHE_CLIENT );
      context = new SSL_CONTEXT( ctx );
      context->setClientConnections( new BinarySearchTree() );
   }

   if ( bugs ) {
      // Set some usefull options too be more compatible
      SSL_CTX_set_options( ctx , SSL_OP_MICROSOFT_SESS_ID_BUG );
      SSL_CTX_set_options( ctx , SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG );
      SSL_CTX_set_options( ctx , SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER );
      SSL_CTX_set_options( ctx , SSL_OP_MSIE_SSLV2_RSA_PADDING );
      SSL_CTX_set_options( ctx , SSL_OP_SSLEAY_080_CLIENT_DH_BUG );
      SSL_CTX_set_options( ctx , SSL_OP_TLS_D5_BUG );
      // Disable Netscape bugs, to be more compatible
      //SSL_CTX_set_options( ctx , SSL_OP_NETSCAPE_CHALLENGE_BUG );
      //SSL_CTX_set_options( ctx , SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG );
   }

   // Use default ciphers SSL_DEFAULT_CIPHER_LIST but not the patented ones
   // RC5 is patented by RSA (until 2023 i think) http://www.rsasecurity.com/.
   // The IDEA algorithm is patented by Ascom in Austria, France, Germany,
   // Italy, Japan, Netherlands, Spain, Sweden, Switzerland, UK and the USA.
   // They should be contacted if that algorithm is to be used, their web page 
   // is http://www.ascom.ch/.
   if (ssl_cipher_list == NULL) {
      SSL_CTX_set_cipher_list( 
         ctx, "ALL:!aNULL:!ADH:!eNULL:!LOW:!EXP:RC4+RSA:+HIGH:+MEDIUM" );
   } else {
      SSL_CTX_set_cipher_list( ctx, ssl_cipher_list );
      mc2log << info << "[SSLSocket] Sets cipher list: " 
             << ssl_cipher_list <<  endl;
   }

   // Make all SSL-renegotiations transparent.
   SSL_CTX_set_mode( ctx, SSL_MODE_AUTO_RETRY );
   // Modes for not needing to call ssl_write with same arguments after
   // a SSL_ERROR_WANT_WRITE or SSL_ERROR_WANT_READ.
   SSL_CTX_set_mode( ctx, SSL_MODE_ENABLE_PARTIAL_WRITE );
   // and this if application (MC2) wants to make a new buffer after 
   // SSL_ERROR_WANT_WRITE on ssl_write.
   // SSL_CTX_set_mode( ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER );

   // Check if ssl has good path and set them
   if ( !SSL_CTX_set_default_verify_paths( ctx ) )  {
      PANIC( "SSLSocket::makeNewCTX ", 
             "SSL_CTX_set_default_verify_paths falied." );
   }

   // Load Cert files.
   if ( ssl_public_cert_file_name != NULL ) {
      // Public cert
      sprintf( filePath, "%s/%s",
               defaultDir,
               ssl_public_cert_file_name );
      
      FileUtils::FilePtr fp( fopen( filePath, "r" ) );
      if ( fp.get() == NULL )  {
         PANIC( "SSLSocket::makeNewCTX cannot open public cert file: ",
                filePath );
      }

      X509* ssl_public_cert = X509_new();
      
      if ( PEM_read_X509( fp.get(), &ssl_public_cert, NULL, NULL ) == 0 )  {
         PANIC( "SSLSocket::makeNewCTX error reading public cert: ",
                ERR_error_string( ERR_get_error(), tmpStr ) );
      }
      if ( ! SSL_CTX_use_certificate( ctx, ssl_public_cert ) ) {
         mc2dbg << error 
                << "[SSLSocket] Can not use public cert." << endl;
      }

      X509_free( ssl_public_cert );
   }

   if ( ssl_private_cert_file_name != NULL ) {
      // Private cert
      sprintf( filePath, "%s/%s",
               defaultDir,
               ssl_private_cert_file_name );
      
      FileUtils::FilePtr fp( fopen( filePath, "r" ) );
      if ( fp.get() == NULL )  {
         PANIC( "SSLSocket::makeNewCTX cannot open private cert file: ",
                filePath );
      }
      
      X509* ssl_private_cert = X509_new();
      
      if ( PEM_read_X509( fp.get(), &ssl_private_cert, NULL, NULL ) == 0 )  {
         cerr << ERR_reason_error_string( ERR_get_error() ) << endl;
         cerr << ERR_func_error_string( ERR_get_error() ) << endl;
         PANIC( "SSLSocket::makeNewCTX error reading private cert: ",
                ERR_error_string( ERR_get_error(), tmpStr ) );
      }
      if ( ! SSL_CTX_use_certificate( ctx, ssl_private_cert ) ) {
         mc2dbg << error 
                << "[SSLSocket] Can not use private cert." << endl;
      }
      X509_free( ssl_private_cert );

   }

   if ( ssl_private_key_file_name != NULL ) {
      // Private RSA key
      sprintf( filePath, "%s/%s", defaultDir, ssl_private_key_file_name );
      
      FileUtils::FilePtr fp( fopen( filePath, "r" ) );

      RSA* ssl_private_key = RSA_new();
      if ( PEM_read_RSAPrivateKey( fp.get(), &ssl_private_key, NULL, NULL ) 
           == 0 )  
      {
         PANIC( "SSLSocket::makeNewCTX error reading private RSA key: ",
                ERR_error_string( ERR_get_error(), tmpStr ) );
      }
      if ( SSL_CTX_use_RSAPrivateKey( ctx, ssl_private_key ) == 0 ) {
         PANIC( "SSLSocket::makeNewCTX error using key ", 
                ERR_error_string(ERR_get_error(), tmpStr ) );
      }

      RSA_free( ssl_private_key );
   }
   
   // Set tmp RSA key for weak connections (40 bit).
   if ( isServerSocket || isServerClientSocket ) {
      SSL_CTX_set_tmp_rsa_callback( ctx, &getTmpRSACallback );
   }

   return context;
}


void 
SSLSocket::freeCTX( SSL_CTX* ctx ) {
   SSL_CTX_free( ctx );
}


SSLSocket::SSLSocket( SSL_CONTEXT* ctx ) 
      : TCPSocket()
{
   init( ctx );
}


SSLSocket::SSLSocket( SOCKET sock, SSL* ssl, SSL_CONTEXT* ctx ) 
      : TCPSocket( sock, DEFAULT_BACKLOG )
{
   init( ctx );
   m_ssl = ssl;
}


SSLSocket::~SSLSocket() {
   if ( m_sock != 0 ) {
      close();
   }
}


void 
SSLSocket::close() {
   // Close SSL connection
   if ( m_sock == 0 ) {
      MC2WARNING( "SSLSocket::close called on empty socket!" );
      return;
   }

   // If socket is listening it hasn't any SSLSession, check
   if ( m_ssl != NULL ) { 
      SysUtility::ignorePipe();
      /* send SSL/TLS close_notify */
      SSL_shutdown( m_ssl );
      SysUtility::setPipeDefault();
      // Returns SSL connection data
      SSL_free( m_ssl );
      m_ssl = NULL;
   }

   // Closes socket nicely and sets m_sock to 0
   TCPSocket::close();
}


void 
SSLSocket::nonWriteClose() {
   // Close SSL connection, but don't write on the socket
   if ( m_sock == 0 ) {
      MC2WARNING( "SSLSocket::nonWriteClose called on empty socket!" );
      return;
   }

   // If socket is listening it hasn't any SSLSession, check
   if ( m_ssl != NULL ) { 
      // Don't send SSL/TLS close_notify as write is not allowed in this
      // method
      // Returns SSL connection data
      SSL_free( m_ssl );
      m_ssl = NULL;
   }

   // Closes socket nicely and sets m_sock to 0
   TCPSocket::close();  
}


bool 
SSLSocket::connect( const char *hostname, int portnumber ) {
   bool ok = true;
   if ( TCPSocket::connect( hostname, portnumber ) ) {
      if ( sslConnect( MAX_UINT32 ) ) {
         DEBUG4(cerr << "SSLSocket::connect conected with encrypted socket"
                << endl;);
         DEBUG8(cerr << "SSL_get_cipher_list " 
                << SSL_get_cipher_list( m_ssl, 0 ) << endl;);
      } else {
         // Falied return
         ok = false;
      }
   } else {
      // Falied return
      ok = false;
   }

   return ok;
}


bool 
SSLSocket::connect( uint32 hostip, uint16 hostport ) {
   bool ok = true;
   if ( TCPSocket::connect( hostip, hostport ) ) {
      if ( sslConnect( MAX_UINT32 ) ) {
         DEBUG4(cerr << "SSLSocket::connect conected with encrypted socket"
                << endl;);
      } else {
         // Falied return
         ok = false;
      }
   } else {
      // Falied return
      ok = false;
   }

   return ok;
}


bool 
SSLSocket::connect( const char *hostname, int portnumber, 
                    uint32 timeout_us ) {
   return TCPSocket::connect( hostname, portnumber, timeout_us ) && 
      sslConnect( timeout_us );
} 


bool 
SSLSocket::connect( const IPnPort& addr, uint32 timeout_us ) {
   return TCPSocket::connect( addr, timeout_us ) && 
      sslConnect( timeout_us );
}


bool 
SSLSocket::sslConnect( uint32 timeout_us ) {
   bool ok = true;
   int err = 0;
   // Connected add SSL handshake
   DebugClock peerClock;
   uint32 IP = 0;
   uint16 port = 0;
   if ( getState() == CONNECTED && !getPeerName( IP, port ) ) {
      // Close TCPConnection, if it is connected
      TCPSocket::close();
      return false;
   }
#define ADD_STEPS
#ifdef ADD_STEPS
   addStepTime( "SSLCheckPeer", peerClock );
#endif

   m_ssl = SSL_new( m_ctx->getCTX() );
   err = SSL_set_fd( m_ssl, m_sock );
   DebugClock certClock;
   // Set certificate if set
   if ( ! m_certificateFile.empty() && ! m_keyFile.empty() ) {
      if ( ! ( SSL_use_certificate_file( m_ssl, m_certificateFile.c_str(),
                                         SSL_FILETYPE_PEM ) &&
               SSL_use_PrivateKey_file( m_ssl, m_keyFile.c_str(),
                                        SSL_FILETYPE_PEM ) ) ) {
         ERR_print_errors_fp( stderr );
         return false;
      }
   }
#ifdef ADD_STEPS
   addStepTime( "SSLSetCert", certClock );
#endif

   if ( err != -1 ) {
      // The actual SSL connect
      if ( m_ctx->getClientConnections() != NULL ) {
         CRYPTO_w_lock( CRYPTO_num_locks() );
         clientHostSession* keySession = new clientHostSession(
            IP, port, NULL );
         clientHostSession* reusedSession = 
            static_cast< clientHostSession* >( 
               m_ctx->getClientConnections()->equal( keySession ) );
         delete keySession;
         if ( reusedSession != NULL ) {
            m_ctx->getClientConnections()->remove( reusedSession );
         }

         if ( reusedSession != NULL ) {
            SESSION_LHASH* sessions = SSL_CTX_sessions( 
               m_ctx->getCTX() );
            SSL_SESSION* reSess = NULL;

            reSess = static_cast<SSL_SESSION*> ( 
               SESSION_LHASH_RETRIEVE( sessions, const_cast<SSL_SESSION*>( 
                  reusedSession->getSession() ) ) );

            if ( reSess != NULL ) {
               SSL_set_session( m_ssl, reSess );
               // Remove now used session
               delete reusedSession;
            }
         }
         CRYPTO_w_unlock( CRYPTO_num_locks() );
      }

      SysUtility::ignorePipe();
      if ( timeout_us != MAX_UINT32 ) {
         DebugClock connectClock;
         bool old_blocking = m_blocking;
         if ( m_blocking ) {
            if ( !setBlocking( false ) ) {
               mc2log << "SSLSocket::sslConnect(): Non blocking failed"
                      << endl;
            }
         }
         bool success = true;

         while ( success ) {
            DebugClock connectCallClock;
            err = SSL_connect ( m_ssl );
          
            bool wantRead = false;
            bool wantWrite = false;
            if ( err == 1 ) {
               // Ok done
               success = true;
               break;
            } else if ( err <= 0 ) {
               switch ( SSL_get_error( m_ssl, err ) ) {
                  case SSL_ERROR_WANT_READ :
                     // Want read
                     wantRead = true;
                     break;
                  case SSL_ERROR_WANT_WRITE :
                     wantWrite = true;
                     // Want write
                     break;
                  default:
                     // Error
                     success = false;
                     break;
               }
            }

            if ( success ) {
               // Now select if success
               struct timeval tv; 
               tv.tv_sec = timeout_us / 1000000;
               tv.tv_usec = timeout_us % 1000000;
               fd_set theSet;
               FD_ZERO(&theSet);
               FD_SET(m_sock, &theSet);
               fd_set* rset = NULL;
               fd_set* wset = NULL;
            
               if ( success ) {
                  if ( wantRead ) {
                     rset = &theSet;
                  } else { // wantWrite
                     wset = &theSet;
                  }
               }
            
               int result = select( m_sock + 1, rset, wset, NULL, &tv );
               if ( result < 0 && result != EINTR ) {
                  mc2dbg << "SSLSocket::sslConnect(): Error " <<
                     strerror(result) << endl;
                  success = false;
               } else if ( result == 0 ) {
                  // Timeout
                  mc2dbg << "SSLSocket::sslConnect(): Timeout." << endl;
                  success = false;
               }
            }
#ifdef ADD_STEPS
            addStepTime( "SSLConnectCallLoop", connectCallClock );
#endif

         }

         // restore blocking status
         if (old_blocking != m_blocking)
            setBlocking(old_blocking);

         if ( !success ) {
            err = -1;
         }
#ifdef ADD_STEPS
         addStepTime( "SSLConnect", connectClock );
#endif
      } else {
         // Do blocking connect
         err = SSL_connect ( m_ssl );
      }
      SysUtility::setPipeDefault();

      if ( err > 0 ) {
         // SSL connected
         mc2log << info << "[SSLSocket] Connected using cipher "
                << SSL_get_cipher_name( m_ssl ) << " with "
                << SSL_get_cipher_bits( m_ssl, &err ) << "bits, algobits: "
                << err << ", SSL_session_reused (bool); " 
                << SSL_session_reused( m_ssl ) << endl;

         // Check server certificate
         
         X509* server_cert = NULL;
         server_cert = SSL_get_peer_certificate ( m_ssl );
         if ( server_cert != NULL ) {
            char* name_str = X509_NAME_oneline( 
               X509_get_subject_name( server_cert ), 0, 0 );
            char* issue_str = X509_NAME_oneline( 
               X509_get_issuer_name( server_cert ), 0, 0 );
            if ( name_str != NULL && issue_str != NULL ) {
               // All ok I think
               DEBUG4(cerr << "Server certificate " << endl
                      << "subject: " << name_str << endl
                      << "issue: " << issue_str << endl;);
            } else {
               MC2WARNING( "SSLSocket::sslConnect X509_NAME_oneline "
                           "falied, server has no valid certificate!" );
            }
            // Must use OPENSSL_free as X509_NAME_oneline uses 
            // OPENSSL_malloc.
            OPENSSL_free( name_str );
            OPENSSL_free( issue_str );

            X509_free( server_cert );
         } else {
            MC2WARNING( "SSLSocket::sslConnect SSL_get_peer_certificate "
                        "falied, server has no certificate!" );
         }

         if ( m_ctx->getClientConnections() != NULL ) {
            SSL_SESSION* ssl_session = SSL_get1_session( m_ssl );

            // Save session for next connect
            // Insert new 
            DEBUG8(cerr << "Setting clientSession for IP " << IP 
                   << " port " << port 
                   << endl;);
            CRYPTO_w_lock( CRYPTO_num_locks() );
            m_ctx->getClientConnections()->add( new clientHostSession( 
               IP, port, ssl_session ) );
            CRYPTO_w_unlock( CRYPTO_num_locks() );
         }
      } else { 
         // Error, but may be just need for more data
         int error = SSL_get_error( m_ssl, err );
         if ( error != SSL_ERROR_WANT_CONNECT &&
              error != SSL_ERROR_WANT_READ &&
              error != SSL_ERROR_WANT_WRITE )
         {
            mc2log << warn << "SSLSocket::sslConnect failed error "
                   << err << endl;
            mc2log << "error " << SSL_get_error( m_ssl, err ) << endl;
            ERR_print_errors_fp( stderr );
            SSL_free( m_ssl );
            m_ssl = NULL;
            // Close TCPConnection
            TCPSocket::close();
            ok = false;
         }
      }
   } else {
      MC2WARNING( "SSLSocket::sslConnect socket set error " );
      ERR_print_errors_fp( stderr );
      SSL_free( m_ssl );
      m_ssl = NULL;
      // Close TCPConnection, if it is connected
      TCPSocket::close();
      ok = false;
   }

   return ok;
}


ssize_t 
SSLSocket::protectedRead( byte* buffer,
                          size_t size ) {
   ssize_t readLength;
#  ifdef EINTR
   do { // Read until the error isn't EINTR
      readLength = SSL_read(m_ssl, (char *)buffer, size);
      if ( readLength < 0 && errno == EINTR ) {
         MC2WARNING("SSLSocket::privateRead - EINTR - trying again");
      }
   } while ( readLength < 0 && errno == EINTR );
#  else
   readLength = SSL_read(m_ssl, (char *)buffer, size);
#  endif
   // if error check for WANT_WRITE, WANT_READ
   if ( readLength == -1 ) {
      int err = SSL_get_error( m_ssl, readLength );
      if ( err == SSL_ERROR_WANT_READ ) {
         // nothing right now, try later
         readLength = -3;
      } else if ( err == SSL_ERROR_WANT_WRITE ) {
         readLength = -4;
      }
   }

   return readLength;
}


ssize_t
SSLSocket::protectedWrite( const byte* buffer,
                           size_t size ) {
   ssize_t writtenLength;
#  ifdef EINTR
   do { // Write until the error isn't EINTR
      writtenLength = SSL_write(m_ssl, (const char *)buffer, size);
      if ( writtenLength < 0 && errno == EINTR ) {
         MC2WARNING("SSLSocket::privateWrite - EINTR - trying again");
      }
   } while ( writtenLength < 0 && errno == EINTR );
#  else
   writtenLength = SSL_write(m_ssl, (const char *)buffer, size);
#  endif
   // if error check for WANT_WRITE, WANT_READ
   if ( writtenLength == -1 ) {
      int err = SSL_get_error( m_ssl, writtenLength );
      if ( err == SSL_ERROR_WANT_WRITE ) {
         // nothing right now, try later
         writtenLength = -3;
      } else if ( err == SSL_ERROR_WANT_READ ) {
         writtenLength = -4;
      }
   }

   return writtenLength;
}


bool 
SSLSocket::checkForCachedData( byte* buffer, size_t size ) {
   size_t readLength = 0;
   if ( SSL_pending( m_ssl ) >= (int32)size ) {
      readLength = SSL_read(m_ssl, (char *)buffer, size);
   }

   return (readLength == size);
}


TCPSocket* 
SSLSocket::accept() {
   DEBUG4( cerr << "SSLSocket::accept()" << endl; );

   SOCKET tmpsock = internalAccept();
   if ( tmpsock != 0 ) {
      return new SSLSocket( tmpsock, NULL, m_ctx );
   } else {
      return NULL;
   }
}


bool
SSLSocket::initialHandshake( long micros ) {
   if ( m_ssl == NULL && m_sock != 0 ) {
      // SSL 
      m_ssl = SSL_new( m_ctx->getCTX() );

#ifndef _MSC_VER
      bool issync = fcntl( F_GETFL, O_NONBLOCK );
      if ( issync && micros != TCPSOCKET_INFINITE_TIMEOUT
           && fcntl( m_sock, F_SETFL, O_NONBLOCK ) < 0 ) 
#else
      u_long FAR * argp = (u_long*)1;
      if ( micros != TCPSOCKET_INFINITE_TIMEOUT
           && ioctlsocket( m_sock, FIONBIO, argp ) < 0 )
#endif
      {
         mc2log << warn << "SSLSocket::initialHandshake() O_NONBLOCK failed"
                << endl;
      }

      SSL_set_fd( m_ssl, m_sock );

      // Doesn't require cert for peer
      SSL_set_verify( m_ssl, SSL_VERIFY_NONE, NULL ); 

      int err = 0;
      int retVal = 0;
      do {
         err = 0;
         mc2dbg4 << "SSLSocket::initialHandshake "
                   "About to call SSL_accept" << endl;
         retVal = SSL_accept( m_ssl );
         if ( retVal < 0 && micros != TCPSOCKET_INFINITE_TIMEOUT ) {
            mc2dbg4 << "SSLSocket::initialHandshake "
                       "start check of eror" << endl;
            err = SSL_get_error( m_ssl, retVal );
            if ( err == SSL_ERROR_WANT_READ ||
                 err == SSL_ERROR_WANT_WRITE )
            {
               mc2dbg4 << "SSLSocket::initialHandshake "
                          "read/write wanted" << endl;
               fd_set fds;
               fd_set* rfds = NULL;
               fd_set* wfds = NULL;
               struct timeval tv;
               FD_ZERO(&fds);
               FD_SET(m_sock, &fds);
               tv.tv_sec = micros / 1000000;
               tv.tv_usec = micros % 1000000;

               if ( err == SSL_ERROR_WANT_READ ) {
                  rfds = &fds;
               } else {
                  wfds = &fds;
               }
#ifndef _MSC_VER
               select(getdtablesize(), rfds, wfds, NULL, &tv);
#else
               select(0, rfds, wfds, NULL, &tv);
#endif
               
               if(!FD_ISSET(m_sock, &fds)) {
                  retVal = -1;
                  break;
               }
            }
         }
      } while ( err == SSL_ERROR_WANT_READ ||
                err == SSL_ERROR_WANT_WRITE );

#ifndef _MSC_VER
      if ( issync && micros != TCPSOCKET_INFINITE_TIMEOUT && 
           fcntl( m_sock, F_SETFL, O_SYNC ) < 0 ) 
#else
      argp = 0;
      if ( micros != TCPSOCKET_INFINITE_TIMEOUT &&
           ioctlsocket( m_sock, FIONBIO, argp ) < 0 )
#endif
      {
         mc2log << warn << "SSLSocket::initialHandshake() O_SYNC failed" << endl;
      }

      if ( retVal <= 0 )  {
         // Error
         mc2log << warn << "SSLSocket::initialHandshake "
                   "SSL_accept failed error " << endl;
         ERR_print_errors_fp( stderr );
         SSL_free( m_ssl );
         m_ssl = NULL;

#ifndef _MSC_VER
         ::close( m_sock ); 
#else
         ::closesocket( m_sock );
#endif
         m_sock = 0;
         m_currentState = CLOSED;

         return false;
      }

      // All ok I think
      DEBUG2(int fooErr = 0;
             mc2dbg2 << "SSLSocket::accept(): using cipher " 
             << SSL_get_cipher_name(m_ssl)
             << ", with " << SSL_get_cipher_bits(m_ssl, &fooErr) 
             << " bits, algobits " << fooErr 
             << ". SSL_session_reused (bool): " 
             << SSL_session_reused(m_ssl) << endl);
   
//      SSL_SESSION_print_fp( stdout, SSL_get_session( m_ssl ) );
      
      return true;
   } else {
      if ( m_ssl == NULL || m_sock == 0 ) {
         return false;
      } else {
         // Already shaked
         return true;
      }
   }
}


int 
SSLSocket::initialHandshaking() {
   if ( m_ssl == NULL ) {
      // Create SSL 
      m_ssl = SSL_new( m_ctx->getCTX() );
      SSL_set_fd( m_ssl, m_sock );
      // Doesn't require cert for peer
      SSL_set_verify( m_ssl, SSL_VERIFY_NONE, NULL ); 
   }

   int res = 0;
   if ( /*SSL_in_accept_init( m_ssl ) Is allways in_connect*/ 
      getState() == CONNECTED ) {
      res = SSL_accept( m_ssl );
   } else { // SSL_in_connect_init
      res = SSL_connect( m_ssl );
   }
   if ( res < 0 ) {
      int err = SSL_get_error( m_ssl, res );
      if ( err == SSL_ERROR_WANT_READ ) {
         res = -4;
      } else if ( err == SSL_ERROR_WANT_WRITE ) {
         res = -5;
      }
   }

   return res;
}


void
SSLSocket::setCertificateAndKey( const char* certificateFile,
                                 const char* keyFile ) 
{
   m_certificateFile = certificateFile ? certificateFile : "";
   m_keyFile = keyFile ? keyFile : "";
}


void
SSLSocket::init( SSL_CONTEXT* ctx ) {
   m_ctx = ctx;
   m_ssl = NULL;
   m_certificateFile.clear();
   m_keyFile.clear();
}


RSA* 
SSLSocket::getTmpRSACallback(SSL* ssl, 
                             int nExport, 
                             int nKeyLength ) {
   DEBUG8(cerr << "SSLSocket::getTmpRSACallback( " << nKeyLength << " )" 
          << endl;);
   if ( nKeyLength == 512 ) {
      if ( tmpRSA512 == NULL ) { 
         tmpRSA512 = RSA_generate_key( 512, RSA_F4, NULL, NULL );
      }
      return tmpRSA512;
   } else if ( nKeyLength == 1024 ) {
      if ( tmpRSA1024 == NULL ) { 
         tmpRSA1024 = RSA_generate_key( 1024, RSA_F4, NULL, NULL );
      }
      return tmpRSA1024;
   } else {
      PANIC( "SSLSocket::getTmpRSACallback ", 
             "nKeyLength invalid value" );
   }

   // Should never happen though.
   return NULL;
}


//----------------------------------------------------------------------
// clientHostSession
//----------------------------------------------------------------------


SSLSocket::clientHostSession::clientHostSession( 
   uint32 IP, uint16 port, 
   const SSL_SESSION* const session )
      : m_IP( IP ),
        m_port( port )
{
   m_session = session;
}


SSLSocket::clientHostSession::~clientHostSession() {
   // m_session is reference counted
   SSL_SESSION_free( const_cast<SSL_SESSION*>( m_session ) );
}


bool
SSLSocket::clientHostSession::operator > (
   const BinarySearchTreeNode &node) const 
{
   const clientHostSession& other = static_cast< 
      const clientHostSession& >( node );

   if ( m_IP > other.m_IP ) {
      return true;
   } else if ( m_IP < other.m_IP ) {
      return false;
   } else if ( m_port > other.m_port ) {
      return true;
   } else {
      return false;
   }
}


bool
SSLSocket::clientHostSession::operator <  (
   const BinarySearchTreeNode &node) const
{
   const clientHostSession& other = static_cast< 
      const clientHostSession& >( node );

   if ( m_IP < other.m_IP ) {
      return true;
   } else if ( m_IP > other.m_IP ) {
      return false;
   } else if ( m_port < other.m_port ) {
      return true;
   } else {
      return false;
   }  
}


bool
SSLSocket::clientHostSession::operator == (
   const BinarySearchTreeNode &node) const
{
   const clientHostSession& other = static_cast< 
      const clientHostSession& >( node );  

   return ( m_IP == other.m_IP && m_port == other.m_port);
}


void 
SSLSocket::clientHostSession::setSession( 
   const SSL_SESSION* const session ) 
{
   m_session = session;   
}


#endif // USE_SSL

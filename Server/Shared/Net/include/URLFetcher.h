/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef URL_FETCHE_H
#define URL_FETCHE_H

#include "config.h"

#include "URLFetcherNoSSL.h"

class SSL_CONTEXT;


/**
 *   URLFetcherNoSSL with SSL
 */
class URLFetcher : public URLFetcherNoSSL {
public:
   
   /**
    *   Inits the SSL context.
    *
    * @param certificateFile Path to client cerificate to use. In pem format.
    * @param keyFile Path to private key for client cerificate. In pem format.
    */
   URLFetcher( SSL_CONTEXT* ctx = NULL,
               const char* certificateFile = NULL,
               const char* keyFile = NULL );
   
   /**
    *   Deletes the SSL context.
    */
   ~URLFetcher();

protected:
   /**
    *   Really creates an SSL socket.
    */
   TCPSocket* createSSLSocket();

private:
   /**
    *   The SSL context, caches SSL sessions.
    */
   SSL_CONTEXT* m_ctx;
   
   /**
    *   SSL context to delete, if local.
    */
   SSL_CONTEXT* m_deleteCtx;

   /**
    * The certificate to use.
    */
   const char* m_certificateFile;

   /**
    * The key of the certificate to use.
    */
   const char* m_keyFile;
};

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "URLFetcher.h"

#include "SSLSocket.h"

URLFetcher::URLFetcher( SSL_CONTEXT* ctx,
                        const char* certificateFile,
                        const char* keyFile )
      : m_certificateFile( certificateFile ),
        m_keyFile( keyFile )
{
   if ( ctx != NULL ) {
      m_ctx = ctx;
      m_deleteCtx = NULL;
   } else {
      m_deleteCtx = m_ctx = SSLSocket::makeNewCTX( 
         SSLSocket::SSL_SOCKET_CLIENTS,
         NULL, NULL, NULL, NULL, NULL);
   }
   m_userAgent = "MC2-URLFetcher/1.1";
#ifdef __linux__
   m_userAgent += " (linux)";
#endif
}

URLFetcher::~URLFetcher()
{
   delete m_deleteCtx;
}

TCPSocket*
URLFetcher::createSSLSocket() {
   SSLSocket* sock = new SSLSocket( m_ctx );
   sock->setCertificateAndKey( m_certificateFile, m_keyFile );
   return sock;
}

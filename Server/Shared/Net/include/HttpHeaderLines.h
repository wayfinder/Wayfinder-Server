/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPHEADERLINES_H
#define HTTPHEADERLINES_H

// Includes
#include "config.h"
#include "MC2String.h"

/**
 * Http headers as strings.
 */
namespace HttpHeaderLines {
   /// Content-Type
   extern const MC2String CONTENT_TYPE;

   /// X-Forwarded-For
   extern const MC2String X_FORWARDED_FOR;

   /// X-WF-ID
   extern const MC2String X_WF_ID;

   /// Proxy-Connection
   extern const MC2String PROXY_CONNECTION;

   extern const MC2String KEEP_ALIVE;
   extern const MC2String RETRY_AFTER;
   extern const MC2String CONTENT_LENGTH;
   extern const MC2String CONNECTION;
   extern const MC2String CLOSE;
   extern const MC2String EXPECT;
   extern const MC2String TRANSFER_ENCODING;
   extern const MC2String CHUNKED;
   extern const MC2String IDENTITY;
   extern const MC2String X_CACHE;
   extern const MC2String HOST;
   extern const MC2String ALLOW;
   extern const MC2String DATE;
   extern const MC2String AGE;
   extern const MC2String CONTENT_ENCODING;
   extern const MC2String ACCEPT_ENCODING;
   extern const MC2String ACCEPT_CHARSET;
   extern const MC2String USER_AGENT;
   extern const MC2String TE;
   extern const MC2String TRAILER;
   extern const MC2String PROXY_AUTHENTICATE;
   extern const MC2String PROXY_AUTHORIZATION;
   extern const MC2String UPGRADE;
}


#endif // HTTPHEADERLINES_H


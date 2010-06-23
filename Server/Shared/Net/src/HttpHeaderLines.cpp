/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpHeaderLines.h"


namespace HttpHeaderLines {
   extern const MC2String CONTENT_TYPE = "Content-Type";
   extern const MC2String X_FORWARDED_FOR = "X-Forwarded-For";
   extern const MC2String X_WF_ID = "X-WF-ID";
   extern const MC2String PROXY_CONNECTION = "Proxy-Connection";
   extern const MC2String KEEP_ALIVE = "Keep-Alive";
   extern const MC2String RETRY_AFTER = "Retry-After";
   extern const MC2String CONTENT_LENGTH = "Content-Length";
   extern const MC2String CONNECTION = "Connection";
   extern const MC2String CLOSE = "close";
   extern const MC2String EXPECT = "Expect";
   extern const MC2String TRANSFER_ENCODING = "Transfer-Encoding";
   extern const MC2String CHUNKED = "chunked";
   extern const MC2String IDENTITY = "identity";
   extern const MC2String X_CACHE = "X-Cache";
   extern const MC2String HOST = "Host";
   extern const MC2String ALLOW = "Allow";
   extern const MC2String DATE = "Date";
   extern const MC2String AGE = "Age";
   extern const MC2String CONTENT_ENCODING = "Content-Encoding";
   extern const MC2String ACCEPT_ENCODING = "Accept-Encoding";
   extern const MC2String ACCEPT_CHARSET = "Accept-Charset";
   extern const MC2String USER_AGENT = "User-Agent";
   extern const MC2String TE = "TE";
   extern const MC2String TRAILER = "Trailer";
   extern const MC2String PROXY_AUTHENTICATE = "Proxy-Authenticate";
   extern const MC2String PROXY_AUTHORIZATION = "Proxy-Authorization";
   extern const MC2String UPGRADE = "Upgrade";
}

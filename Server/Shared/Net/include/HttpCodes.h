/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPCODES_H
#define HTTPCODES_H

namespace HttpCode {

/**
 * Http response codes.
 */
enum Codes {
   // Success 2xx
   // These codes indicate success.

   /**
    * The request was fulfilled.
    */
   OK = 200,


   // Redirect 3xx
   // The codes in this section indicate action to be taken (normally
   //  automatically) by the client in order to fulfill the request


   /**
    * The data resides under a different URL. The redirection may be altered on occasion.
    */
   FOUND = 302,

   /**
    * The response to the request can be found under another URI.
    */
   SEE_OTHER = 303,

   /**
    * If the client has performed a conditional GET request and access is
    * allowed, but the document has not been modified, the server SHOULD
    * respond with this status code.
    */
   NOT_MODIFIED = 304,

   // Error 4xx, 5xx
   // The 5xx codes are intended for cases in which the client seems to have
   // erred, and the 5xx codes for the cases in which the server is aware that
   // the server has erred. It is impossible to distinguish these cases in
   // general, so the difference is only informational.

   /**
    * The request had bad syntax or was inherently impossible to be satisfied.
    */
   BAD_REQUEST = 400,

   /**
    * The request is for something forbidden.
    */
   FORBIDDEN = 403,

   /**
    * The server encountered an unexpected condition which prevented it from
    * fulfilling the request
    */
   NOT_FOUND = 404,

   /**
    * The method specified in the Request-Line is not allowed for the resource
    * identified by the Request-URI.
    */
   METHOD_NOT_ALLOWED = 405,

   /**
    * The requested resource is no longer available at the server and no
    * forwarding address is known.
    */
   GONE = 410,

   /**
    * The request is larger than the server is willing or able to process.
    */
   REQUEST_ENTITY_TOO_LARGE = 413,

   /**
    * The URI provied was too long for the server to process.
    */
   REQUEST_URI_TOO_LONG = 414,

   /**
    * The server cannot meet the requirements of the Expected request-header
    * field.
    */
   EXPECTATION_FAILED = 417,

   /**
    * The HTCPCP server is a teapot.
    */
   IM_A_TEAPOT = 418,

   /**
    * A generica error message, give when no more specific message is suitable.
    */
   INTERNAL_ERROR = 500,

   /**
    * The server either does not recognise the request method, or it lacks the
    * ability to fulfil the request.
    */
   NOT_IMPLEMENTED = 501,

   /**
    * The server is currently unavailable, this is a temporary state.
    */
   SERVICE_UNAVAILABLE = 503,

   /**
    * The server does not support the HTTP protocol version used in the
    * request.
    */
   HTTP_VERSION_NOT_SUPPORTED = 505
};

} // namespace HttpCode

#endif // HTTPCODES_H

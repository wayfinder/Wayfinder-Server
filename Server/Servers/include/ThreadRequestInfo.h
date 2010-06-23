/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef THREAD_REQUEST_INFO_H
#define THREAD_REQUEST_INFO_H

#include "Types.h"
#include "ServerTypes.h"

#include <iosfwd>

class ThreadRequestContainer;

namespace ThreadRequestInfo {

/**
 * Print begin information for a request before sending it.
 * @param ostr Will output ot this stream.
 * @param server The server type from which the request will be sent.
 * @param cont The request to print info about.
 */
void printBeginInfo( std::ostream& ostr,
                     ServerTypes::servertype_t server,
                     const ThreadRequestContainer& cont );

/**
 * Print request end information, use when a request has finished processing.
 * @param ostr Output to this stream.
 * @param server The server type from which the request was sent.
 * @param cont The request to print info about
 * @param reqTime The time it took for requesting.
 */
void printEndInfo( std::ostream& ostr,
                   ServerTypes::servertype_t server,
                   const ThreadRequestContainer& cont,
                   uint32 reqTime );

} // ThreadRequestInfo

#endif // THREAD_REQUEST_INFO_H

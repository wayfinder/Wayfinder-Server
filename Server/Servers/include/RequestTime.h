/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REQUESTTIME_H
#define REQUESTTIME_H


#include "config.h"

#include "MC2String.h"
#include <iosfwd>
#include <vector>

class Request;

/**
 * Contains function to print timed types in Requests
 */
namespace RequestTime {

/// timed types
enum TimeType {
   putRequest,
   getNextPacket,
   processPacket,
   totalCPU
};

/// @return string representation of a time type
const char* toString( TimeType type );

/// prints time information to stream 
void printRequestTime( ostream& out, 
                       const Request& request );

/// prints time information about a request
void printRequestTime( const MC2String& filename,
                       const Request& request );
/// Prints values of time types from a vector
void printVector( std::ostream& str, 
                  const std::vector< uint32 >& times );

/// helper to dump Request time to stream
struct Dump {
   Dump( const Request& req):request( req ) { }
   const Request& request;
};


}

std::ostream& operator << ( std::ostream& ostr, 
                            const RequestTime::Dump& dump );

#endif // REQUESTTIME_H

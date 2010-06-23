/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IPNPORT_H
#define IPNPORT_H

#include "config.h"

#include <utility>
#include "MC2String.h"

/**
 *   After a long discussion with several more or less
 *   appealing suggestions. IPAndPort was suggested.
 *   Some then though that name was too long so
 *   we came up with IPEndPoint which was voted down.
 *   <br />
 *   So we came to the conclusion that IPnPort would do.
 */
class IPnPort : public pair<uint32,uint16> {
public:
   /**
    *   Constructor.
    *   @param ipaddr The IP of the pair.
    *   @param port   The port of the pair.
    */
   IPnPort(uint32 ipaddr, uint16 port) : pair<uint32,uint16>(ipaddr,port) {}

   /**
    *   Constructor, initializes to 0:0.
    */
   IPnPort() : pair< uint32,uint16 > ( 0, 0 ) {}

   /**
    *   Returns the ip adddress.
    */
   uint32 getIP() const { return first; }

   /**
    *   Returns the port.
    */
   uint16 getPort() const { return second; }
   /// @return ip+port represented as a string
   MC2String toString() const;
   /**
    *   Prints the pair on the supplied stream.
    *   @param stream The stream
    *   @param addr   The address.
    */  
   inline friend ostream& operator<<( ostream& stream,
                                      const IPnPort& addr );
};

inline
ostream& operator<<( ostream& stream,
                     const IPnPort& addr )
{
   return stream << prettyPrintIP(addr.first) << ":" << addr.second;
}

#endif

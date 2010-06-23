/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IP_H
#define IP_H

#include "config.h"
#include "MC2String.h"
#include "MC2Exception.h"

/**
 * Class representing an Internet Protocol address.
 * Currently only IPv4 addresses are supported. 
 */
class IP {
public:
   typedef uint32 ipType;

   /**
    * Construct an IP from a number.
    */
   IP( ipType ip );

   /**
    * Construct an IP from a string.
    * Converts a string in the format "a.b.c.d" where a, b, c and d is
    * numbers from 0 too 255.
    *
    * @param ipAsString The dotted decimal IP string.
    * @return The IP for the input string or throws an exception if
    *         malformed IP.
    */
   static IP fromString( const MC2String& ipAsString ) throw ( MC2Exception );

   /**
    * Get the IP address.
    */
   const ipType getIP() const; 

   /**
    * Turn an IP to a number.
    */
   operator ipType() const;

private:
   /// The IP.
   ipType m_ip;
};

// =======================================================================
//                                     Implementation of inlined methods =

inline IP::IP( ipType ip )
      : m_ip( ip )
{
}

inline const IP::ipType
IP::getIP() const {
   return m_ip;
}

inline IP::operator ipType() const {
   return m_ip;
}

/**
 * Print the IP on a stream.
 */
std::ostream& operator<<( std::ostream& stream, const IP& ip );


#endif // IP_H

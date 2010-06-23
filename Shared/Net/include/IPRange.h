/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IP_RANGE_H
#define IP_RANGE_H

#include "config.h"
#include "IP.h"
#include "MC2String.h"


/**
 * Class representing a range of IP addresses.
 */
class IPRange {
public:
   /**
    * Construct an IP range from numbers for lowest and highest
    * IP address in the range.
    *
    * @param lowestIP The lowest IP address number in the range.
    * @param highestIP The highest IP address number in the range.
    */
   IPRange( const IP& lowestIP, const IP& highestIP );

   /**
    * Construct an IP range from string, like a.b.c.d, for lowest and highest
    * IP address in the range.
    * Throws an exception if the IP strings are not valid.
    *
    * @param lowestIP The lowest IP address as string in the range.
    * @param highestIP The highest IP address as string in the range.
    */
   IPRange( const MC2String& lowestIP, const MC2String& highestIP )
      throw ( MC2Exception );

   /**
    * Construct an IP range from a IP-range string like
    * IP-IP, e.g. 192.168.0.10-192.168.0.12, or like
    * 192.168.0.10/26, where the the IP address before the slash
    * specifies the IP in the range and the number after the slash is
    * the number of same bits. In the example the IP is 192.168.10 and 
    * the number of same bits is 26 leaving the lowest 6 bits that 
    * should then be from 0 to 63 making the range 192.168.0.0 to
    * 192.168.0.63.
    * Note that in this example the ending .10 in the IP is inside the 
    * mask bits and one might consider it an invalid range, looks like 
    * someone didn't get it right when making the IP-range.
    * Another example is 192.168.0.163/26 where the 163(10100011) has bits
    * (10000000) above the lowest 6 bits and thus adds to the result giving
    * 192.168.0.128 to 192.168.0.191 and is equal to 192.168.0.128/26.
    *
    * Throws an exception if the string is not valid.

    * @param ipRange The range of IP addresses as string.
    */
   IPRange( const MC2String& ipRange ) throw ( MC2Exception );

   /**
    * Checks if this range is lower than another range, note that
    * overlapping ranges are not supported.
    *
    * @return True if this range is lower than the other range.
    */
   bool operator < ( const IPRange& other ) const;

   /**
    * Get the lower IP in the range.
    *
    * @return The lower IP in the range.
    */
   const IP& getLowerIP() const;

   /**
    * Get the higher IP in the range.
    *
    * @return The higher IP in the range.
    */
   const IP& getHigherIP() const;

   /**
    * Checks if an IP is in this range.
    *
    * @param ip The IP to check.
    * @return True if the ip is in range.
    */
   bool isInRange( const IP& ip ) const;

private:
   /// The lower IP.
   IP m_lowerIP;

   /// The higher IP.
   IP m_higherIP;
};

/**
 * Print the IPRange on a stream.
 */
std::ostream& operator<<( std::ostream& stream, const IPRange& ipRange );

// =======================================================================
//                                     Implementation of inlined methods =

inline bool 
IPRange::operator < ( const IPRange& other ) const {
   return getLowerIP() < other.getLowerIP() && 
      getHigherIP() < other.getHigherIP();
}

inline const IP& 
IPRange::getLowerIP() const {
   return m_lowerIP;
}

inline const IP&
IPRange::getHigherIP() const {
   return m_higherIP;
}

inline bool
IPRange::isInRange( const IP& ip ) const {
   return ip >= m_lowerIP && ip <= m_higherIP;
}

#endif // IP_RANGE_H


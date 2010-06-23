/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "IPRange.h"
#include "StringConvert.h"
#include "BitUtility.h"
#include <math.h>

IPRange::IPRange( const IP& lowestIP, const IP& highestIP )
      : m_lowerIP( lowestIP ), m_higherIP( highestIP )
{
}

IPRange::IPRange( const MC2String& lowestIP, const MC2String& highestIP ) 
   throw ( MC2Exception )
      : m_lowerIP( IP::fromString( lowestIP ) ), 
        m_higherIP( IP::fromString( highestIP ) )
{
}

IPRange::IPRange( const MC2String& ipRange ) throw ( MC2Exception )
      : m_lowerIP( 0 ), m_higherIP( 0 )
{
   size_t findPos = ipRange.find( "/" );
   if ( findPos != MC2String::npos ) {
      // Get the IP
      IP ip = IP::fromString( ipRange.substr( 0, findPos ) );

      // Get the Bits
      uint32 bits = StringConvert::convert< uint32 >( 
         ipRange.substr( findPos + 1 ) );
      if ( bits > 32 ) {
         throw MC2Exception( "IPRange", "Bad bit mask in: " + ipRange );
      }

      // Make the range
      IP::ipType ipNbr = ip.getIP();
      uint32 nbrBits = 32 - bits;
      uint32 bitMask = uint32( rint( pow( 2, nbrBits ) ) ) - 1;

      // Zero the lowest bits in the input IP
      for ( uint32 i = 0; i < nbrBits ; ++i ) {
         BitUtility::setBit( ipNbr, i, 0 );
      }
   
      // Make the IPs
      m_lowerIP = ipNbr;
      m_higherIP = (ipNbr | bitMask);
   } else {
      // Try IP-IP
      findPos = ipRange.find( "-" );
      if ( findPos == MC2String::npos ) {
         throw MC2Exception( "IPRange", "Bad IPRange string: " + ipRange );
      } // Else continue on next line
      m_lowerIP  = IP::fromString( ipRange.substr( 0, findPos ) );
      m_higherIP = IP::fromString( ipRange.substr( findPos + 1 ) );
      if ( m_lowerIP > m_higherIP ) {
         throw MC2Exception( "IPRange", "Lower IP higher than high IP! In "
                             + ipRange );
      }
   }
}

std::ostream& operator<<( std::ostream& stream, const IPRange& ipRange ) {
   stream << ipRange.getLowerIP() << " - " << ipRange.getHigherIP();
   return stream;
}

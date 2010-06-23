/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STREETNBR_H
#define STREETNBR_H

#include "config.h"
#include "DataBufferRW.h"

class DataBuffer;

/**
 * Holds street number start and end for left and right side
 */
class StreetNbr : public DataBufferRW {
public:
   StreetNbr() : m_rightSideNbrStart(0),
                 m_rightSideNbrEnd(0),
                 m_leftSideNbrStart(0),
                 m_leftSideNbrEnd(0) { }
   /// load from data buffer
   void load( DataBuffer& buff );
   /// save to data buffer
   void save( DataBuffer& buff ) const;

   /// @return size in data buffer
   uint32 getSizeInDataBuffer() const {
      return 8;
   }

   uint16 getRightSideNbrStart() const {
      return m_rightSideNbrStart;
   }

   uint16 getRightSideNbrEnd() const {
      return m_rightSideNbrEnd;
   }

   uint16 getLeftSideNbrStart() const { 
      return m_leftSideNbrStart;
   }

   uint16 getLeftSideNbrEnd() const {
      return m_leftSideNbrEnd;
   }

   bool operator != (const StreetNbr& rhs) const {
      return ! ( *this == rhs );
   }

   bool operator == (const StreetNbr& rhs) const {
      return 
         rhs.m_rightSideNbrStart == m_rightSideNbrStart &&
         rhs.m_rightSideNbrEnd == m_rightSideNbrEnd &&
         rhs.m_leftSideNbrStart == m_leftSideNbrStart &&
         rhs.m_leftSideNbrEnd == m_leftSideNbrEnd;
   }

   bool operator < (const StreetNbr& rhs) const {
     #define _LESS(x) x != rhs.x ? x < rhs.x :
     return 
        _LESS( m_leftSideNbrEnd )
        _LESS( m_leftSideNbrStart )
        _LESS( m_rightSideNbrEnd )
        _LESS( m_rightSideNbrStart ) false;
      #undef _LESS
   }

private:
   friend class M3Creator;

   uint16 m_rightSideNbrStart;
   uint16 m_rightSideNbrEnd;
   uint16 m_leftSideNbrStart;
   uint16 m_leftSideNbrEnd;
};
#endif // STREETNBR_H

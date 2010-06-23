/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "DisturbanceDescription.h"
#include "Packet.h"

//
// ====================== DisturbanceDescription =====================
// 

DisturbanceDescription::DisturbanceDescription()
      : m_splitPoint(), m_distPoint(), m_joinPoint()
{
   m_type   = TrafficDataTypes::NoType;
   m_distID = MAX_UINT32;
}

DisturbanceDescription::DisturbanceDescription(IDPair_t distPoint)
      : m_splitPoint(), m_joinPoint()
{
   m_distPoint  = distPoint;
   m_type       = TrafficDataTypes::NoType;
   m_distID     = MAX_UINT32;
}

DisturbanceDescription::DisturbanceDescription(IDPair_t splitPoint,
                                               IDPair_t distPoint,
                                               IDPair_t joinPoint)
{
   m_splitPoint = splitPoint;
   m_distPoint  = distPoint;
   m_joinPoint  = joinPoint;
   m_type       = TrafficDataTypes::NoType;
   m_distID     = MAX_UINT32;
}

DisturbanceDescription::DisturbanceDescription(IDPair_t splitPoint,
                                               IDPair_t distPoint,
                                               IDPair_t joinPoint,
                                               TrafficDataTypes::
                                               disturbanceType type,
                                               uint32 distID,
                                               const char* comment)
{
   m_splitPoint = splitPoint;
   m_distPoint  = distPoint;
   m_joinPoint  = joinPoint;
   m_type       = type;
   m_distID     = distID;
   m_comment    = MC2String(comment);
}

bool
DisturbanceDescription::isDetour() const {

   return (m_splitPoint.isValid() || m_joinPoint.isValid());
}

void
DisturbanceDescription::setDescription(TrafficDataTypes::disturbanceType type,
                                       uint32 distID,
                                       const char* comment){
   m_type       = type;
   m_distID     = distID;
   m_comment    = MC2String(comment);
}

bool
DisturbanceDescription::isDescriptionSet() const {
   return  (m_distID != MAX_UINT32);
}

uint32
DisturbanceDescription::save( Packet* p, int& pos ) const {
   uint32 startOffset = p->getLength();

   m_splitPoint.save( p, pos );
   m_distPoint.save( p, pos );
   m_joinPoint.save( p, pos );
   p->incWriteLong( pos, m_type );
   p->incWriteLong( pos, m_distID );
   p->incWriteString( pos, m_comment );

   p->setLength( pos ); 

   return p->getLength() - startOffset;
}

uint32
DisturbanceDescription::getSizeAsBytes() const {
   uint32 size = 0;

   // 3 IDPairs
   size += 3*4*2;
   // 2 uint32s
   size += 2*4;

   // Comment
   size += m_comment.size() +1;

   // Possible padding
   size += 3;
   
   return size;
}


void
DisturbanceDescription::load( const Packet* p, int& pos ) {
   m_splitPoint.load( p, pos );
   m_distPoint.load( p, pos );
   m_joinPoint.load( p, pos );
   m_type = TrafficDataTypes::disturbanceType( p->incReadLong( pos ) );
   m_distID = p->incReadLong( pos );
   p->incReadString( pos, m_comment );
}

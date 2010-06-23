/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RoutingInfoPacket.h"
#include "StringTable.h"

// ---------------- RoutingInfo -------------

RoutingInfo::RoutingInfo(const CriteriumMap& upCriteria,
                         const CriteriumMap& downCriteria)
      : m_upCriteria(upCriteria), m_downCriteria(downCriteria)
{
}

const CriteriumMap&
RoutingInfo::getUpCriteria() const
{
   return m_upCriteria;
}

const CriteriumMap&
RoutingInfo::getDownCriteria() const
{
   return m_downCriteria;
}

int
RoutingInfo::getSizeInPacket() const
{
   return m_downCriteria.size() * 8 + m_upCriteria.size() * 8 + 4 + 4;
}

bool
RoutingInfo::saveMap(Packet* p,
                     int& pos,
                     const CriteriumMap& theMap)
{
   p->incWriteLong(pos, theMap.size());
   for( CriteriumMap::const_iterator it = theMap.begin();
        it != theMap.end();
        ++it ) {
      p->incWriteLong(pos, it->first);
      p->incWriteLong(pos, it->second);
   }
   return true;
}

bool
RoutingInfo::loadMap(const Packet* p,
                     int& pos,
                     CriteriumMap& theMap)
{
   theMap.clear();
   int nbrItems = p->incReadLong(pos);
   for( int i = 0; i < nbrItems; ++i ) {
      int level = p->incReadLong(pos);
      uint32 distance = p->incReadLong(pos);
      theMap.insert(make_pair(level, distance));
   }
   return true;
}

bool
RoutingInfo::save(Packet* p, int& pos) const
{
   saveMap(p, pos, m_upCriteria);
   saveMap(p, pos, m_downCriteria);
   return true;
}

bool
RoutingInfo::load(const Packet* p, int& pos)
{
   loadMap(p, pos, m_upCriteria);
   loadMap(p, pos, m_downCriteria);
   return true;
}

// ---------------- RoutingInfoRequestPacket -------------

RoutingInfoRequestPacket::RoutingInfoRequestPacket()
      : RequestPacket( ROUTINGINFO_REQUEST_MAX_LENGTH,
                       ROUTINGINFO_REQUEST_PRIO,
                       Packet::PACKETTYPE_ROUTINGINFOREQUEST,
                       0,
                       0,
                       MAX_UINT32)
{
   setLength(REQUEST_HEADER_SIZE);
}

// ---------------- RoutingInfoReplyPacket -------------

RoutingInfoReplyPacket::
RoutingInfoReplyPacket(const RoutingInfoRequestPacket* req,
                       const RoutingInfo& info)
      : ReplyPacket( ROUTINGINFO_REQUEST_MAX_LENGTH,
                     Packet::PACKETTYPE_ROUTINGINFOREPLY,
                     req,
                     StringTable::OK )
{ 
   int pos = REPLY_HEADER_SIZE;
   info.save(this, pos);
   setLength(pos);
}

RoutingInfo
RoutingInfoReplyPacket::getRoutingInfo() const
{
   int pos = REPLY_HEADER_SIZE;
   RoutingInfo info;
   info.load(this, pos);
   return info;
}


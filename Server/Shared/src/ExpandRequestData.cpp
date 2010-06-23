/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandRequestData.h"
#include "Packet.h"
#include "PacketHelpers.h"

ExpandRequestData::ExpandRequestData() {
   m_id.first = MAX_UINT32;
   m_id.second = MAX_UINT32;
   m_reqLang = LangTypes::nbrLanguages;
}

ExpandRequestData::ExpandRequestData( const IDPair_t& id,
                                      LangTypes::language_t reqLang,
                                      set<ItemTypes::itemType> itemTypes) 
      : m_id( id ),
        m_reqLang( reqLang ),
        m_itemTypes( itemTypes )
{
}

ExpandRequestData::ExpandRequestData( const ExpandRequestData& data )
{
   m_id.first = data.getIDPair().first;
   m_id.second = data.getIDPair().second;
   m_reqLang = data.getLanguage();
   m_itemTypes = data.getItemTypes();
}

ExpandRequestData::~ExpandRequestData()  
{
}

void
ExpandRequestData::save( Packet* p, int& pos ) const
{
   SaveLengthHelper slh ( p, pos );
   slh.writeDummyLength( pos );
   p->incWriteLong( pos, m_id.first );
   p->incWriteLong( pos, m_id.second );
   p->incWriteLong( pos, m_reqLang );
   p->incWriteLong( pos, m_itemTypes.size() );
   set<ItemTypes::itemType>::iterator it;
   for(it = m_itemTypes.begin(); it != m_itemTypes.end(); ++it) {
      p->incWriteLong( pos, *it );
   }
   slh.updateLengthUsingEndPos( pos );
}

void
ExpandRequestData::load( const Packet* p, int& pos ) 
{
   LoadLengthHelper llh( p, pos );
   llh.loadLength( pos );
   p->incReadLong( pos, m_id.first );
   p->incReadLong( pos, m_id.second );
   p->incReadLong( pos, m_reqLang );
   uint32 n = p->incReadLong( pos );
   for(uint32 i = 0; i < n; i++) {
      ItemTypes::itemType type = (ItemTypes::itemType) p->incReadLong(pos);
      m_itemTypes.insert( type );
   }
   llh.skipUnknown( pos );
}

int
ExpandRequestData::getSizeInPacket() const
{
   return 20 + 4 * m_itemTypes.size();
}

IDPair_t
ExpandRequestData::getIDPair() const
{
   return m_id;
}

LangTypes::language_t
ExpandRequestData::getLanguage() const
{
   return m_reqLang;
}

set<ItemTypes::itemType>
ExpandRequestData::getItemTypes() const
{
   return m_itemTypes;
}

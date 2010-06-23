/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandRequest.h"
#include "ExpandRequestData.h"
#include "ExpandPacket.h"
#include "SearchReplyData.h"
#include "SearchRequestParameters.h"
#include "MC2CRC32.h"
#include "DataBuffer.h"
#include "SearchMatch.h"

ExpandRequest::ExpandRequest( const RequestData& parentOrID,
                              const ExpandRequestData& data )
   
      : SearchResultRequest( parentOrID )
{
   enqueuePacket( updateIDs( new ExpandRequestPacket( data,
                                                      getUser() ) ),
                  MODULE_TYPE_MAP );
   
   m_status = StringTable::TIMEOUT_ERROR;
   m_replyData = new SearchReplyData();
   m_params = new SearchRequestParameters();
}

ExpandRequest::~ExpandRequest()
{
   delete m_replyData;
   delete m_params;
}

StringTable::stringCode
ExpandRequest::getStatus() const
{
   return StringTable::OK;
}

void
ExpandRequest::processPacket( PacketContainer* pack )
{
   const ExpandReplyPacket* reply =
      static_cast<ExpandReplyPacket*>( pack->getPacket() );
   MC2_ASSERT( reply->getSubType() == Packet::PACKETTYPE_EXPAND_REPLY );

   reply->get( *m_replyData );
   m_status = reply->getStatus();
   m_done = true;
   calcCRC();
}

const vector<VanillaMatch*>&
ExpandRequest::getMatches() const
{
   return m_replyData->getMatchVector();
}

const SearchRequestParameters&
ExpandRequest::getSearchParameters() const
{
   return *m_params;
}

uint32
ExpandRequest::getTotalNbrMatches() const
{
   return m_replyData->getTotalNbrMatches();
}

int
ExpandRequest::translateMatchIdx( int wantedIdx ) const
{
   return m_replyData->translateMatchIdx( wantedIdx );
}


void
ExpandRequest::calcCRC()
{
   vector<VanillaMatch*>& matches = m_replyData->getMatchVector();
   vector<VanillaMatch*>::iterator it;

   uint32 n = matches.size() * 4;
   for(it = matches.begin(); it != matches.end(); ++it) {
      VanillaMatch* currMatch = *it;
      n = n + strlen( currMatch->getName() ) + 1;
   }

   DataBuffer buf( n );

   for(it = matches.begin(); it != matches.end(); ++it) {
      VanillaMatch* currMatch = *it;
      buf.writeNextLong( currMatch->getItemID() );
   }
   for(it = matches.begin(); it != matches.end(); ++it) {
      VanillaMatch* currMatch = *it;
      buf.writeNextString( currMatch->getName() );
   }

   m_crc = MC2CRC32::crc32( buf.getBufferAddress(),
                            buf.getCurrentOffset() );
}

uint32
ExpandRequest::getCRC() const
{
   return m_crc;
}

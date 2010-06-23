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

#include "ExternalSearchRequest.h"

#include "SearchRequestParameters.h"
#include "SearchReplyData.h"
#include "ExternalSearchPacket.h"
#include "ExternalSearchRequestData.h"


ExternalSearchRequest::
ExternalSearchRequest( const RequestData& data,
                       const ExternalSearchRequestData& searchData )
   : SearchResultRequest( data ),
     m_topHits( 0 )
{
   // Copy params
   m_params = new SearchRequestParameters( searchData.getSearchParams() );

   enqueuePacket( updateIDs( new ExternalSearchRequestPacket( searchData, getUser() ) ),
                  MODULE_TYPE_EXTSERVICE );

   m_status = StringTable::TIMEOUT_ERROR;
   m_replyData = new SearchReplyData();
}

ExternalSearchRequest::~ExternalSearchRequest()
{
   delete m_replyData;
   delete m_params;
}


StringTable::stringCode
ExternalSearchRequest::getStatus() const
{
   return StringTable::stringCode(m_status);
}

void
ExternalSearchRequest::processPacket( PacketContainer* pack )
{
   mc2dbg2 << "[ExternalSearchRequest::processPacket]" << endl;
   const ExternalSearchReplyPacket* reply =
      static_cast<const ExternalSearchReplyPacket*>(pack->getPacket());
   MC2_ASSERT( reply->getSubType() ==
               Packet::PACKETTYPE_EXTERNALSEARCH_REPLY );

   // Decode data
   reply->get( *m_replyData, m_topHits );
   // Save packet since it may contain data from matches
   m_replyData->addPacket( pack );
   
   
   m_status = reply->getStatus();
   m_done = true;
}

const vector<VanillaMatch*>&
ExternalSearchRequest::getMatches() const
{
   return m_replyData->getMatchVector();
}

const SearchRequestParameters&
ExternalSearchRequest::getSearchParameters() const
{
   return *m_params;
}

uint32
ExternalSearchRequest::getTotalNbrMatches() const
{
   return m_replyData->getTotalNbrMatches();
}

int
ExternalSearchRequest::translateMatchIdx( int wantedIdx ) const
{
   return m_replyData->translateMatchIdx( wantedIdx );
}

uint32 ExternalSearchRequest::getNbrOverviewMatches() const {
   return m_replyData->getOverviewMatches().size();
}

OverviewMatch* ExternalSearchRequest::getOverviewMatch( uint32 index ) const {
   return m_replyData->getOverviewMatches()[ index ];
}


uint32 ExternalSearchRequest::getNbrTopHits() const {
   return m_topHits;
}

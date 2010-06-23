/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNAL_SEARCH_REQUEST_H
#define EXTERNAL_SEARCH_REQUEST_H

#include "config.h"

#include "SearchResultRequest.h"

class SearchReplyData;
class SearchRequestParams;
class ExtService;
class ExternalSearchRequestData;

class ExternalSearchRequest : public SearchResultRequest {
public:

   ExternalSearchRequest( const RequestData& data,
                          const ExternalSearchRequestData& searchData );

   /// Well.
   ~ExternalSearchRequest();

   /// Returns StringTable::OK if OK.
   StringTable::stringCode getStatus() const;

   /// Processes one packet
   void processPacket( PacketContainer* pack );

   /// Implements SearchResultRequest
   const vector<VanillaMatch*>& getMatches() const;
   /// Implements SearchResultRequest
   const SearchRequestParameters& getSearchParameters() const;

   /// Overrides the one in SearchResultRequest
   uint32 getTotalNbrMatches() const;
   /// Overrides the one in SearchResultRequest
   int translateMatchIdx( int wantedIdx ) const;
   /// @return number of overview matches found in external search
   uint32 getNbrOverviewMatches() const;
   /// @return a specific overview match 
   OverviewMatch* getOverviewMatch( uint32 i ) const;
   /// @return number of top hits to be displayed above headings.
   uint32 getNbrTopHits() const;
private:

   /// The parameters of the request. Not all are used.
   SearchRequestParameters* m_params;   
   /// The reply data contains matches.
   SearchReplyData* m_replyData;  
   /// The status
   uint32 m_status;
   /// number of top hits
   uint32 m_topHits;
};

#endif // EXTERNAL_SEARCH_REQUEST_H

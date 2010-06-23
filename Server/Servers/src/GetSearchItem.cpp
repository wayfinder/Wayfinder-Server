/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GetSearchItem.h"
#include "SearchMatch.h"
#include "DeleteHelpers.h"
#include "SearchRequestParameters.h"
#include "PacketContainer.h"
#include "SearchRequestPacket.h"
#include "SearchReplyPacket.h"
#include "UserRightsMapInfo.h"
#include "ModuleTypes.h"
#include "Requester.h"
#include "SearchParserHandler.h"
#include "ExternalSearchConsts.h"

namespace GetSearchItem {

bool request( Requester& requester,
              const SearchMatch* inputMatch,
              const LangType& language,
              auto_ptr<SearchMatch>& result ) {
   bool res = true;

   if ( inputMatch->getExtSource() != ExternalSearchConsts::not_external ) {
      // If external sources gets more searchmatch data add way to get it here
      result.reset( static_cast<const VanillaMatch*>( inputMatch )->clone() );
      return res;
   }

   if ( ! inputMatch->getID().isValid() ) {
      mc2log << warn << "[GetSearchItem]:request Bad id in match " 
             << MC2CITE( *inputMatch ) << endl;
      // Return something?
      result.reset( static_cast<const VanillaMatch*>( inputMatch )->clone() );
      return false;
   }

   // Setup the request data
   IDPairVector_t itemIDs;
   itemIDs.push_back( inputMatch->getID() );
   SearchRequestParameters params;
   params.setRequestedLang( language );

   auto_ptr<PacketContainer> reply ( 
      requester.putRequest( 
         new SearchRequestPacket( inputMatch->getMapID(),
                                  0/*packetID*/,
                                  0/*requestID*/,
                                  SearchRequestPacket::PROXIMITY_SEARCH,
                                  "", // Search string
                                  params,
                                  itemIDs,
                                  vector<MC2BoundingBox>(),
                                  MC2Coordinate()/*Used to sort, if >1 match*/,
                                  UserRightsMapInfo() ),
         MODULE_TYPE_SEARCH ) ); 

   if ( reply.get() != NULL && 
        static_cast<ReplyPacket*>( reply->getPacket() )->getStatus() 
        == StringTable::OK ) {
      VanillaSearchReplyPacket* r = static_cast<VanillaSearchReplyPacket*> (
         reply->getPacket() );
      vector<VanillaMatch*> matches;
      r->getAllMatches( matches );
      if ( ! matches.empty() ) {
         result.reset( matches.front()->clone() );
         // Delete matches
         for_each( matches.begin(), matches.end(),
                   STLUtility::DeleteObject() );
      } else {
         mc2log << warn << "[GetSearchItem]:request No match for item: " 
                << MC2CITE( *inputMatch ) << endl;
         // Return something?
         result.reset( static_cast<const VanillaMatch*>( 
                          inputMatch )->clone() );
      }
   } else {
      res = false;
      mc2log << warn << "[GetSearchItem]:request Lookup failed ";
      if ( reply.get() != NULL ) {
         mc2log << "status: "
                << MC2CITE( StringTable::getString( StringTable::stringCode( 
                      static_cast<ReplyPacket*>( reply->getPacket() )->
                      getStatus() ), StringTable::ENGLISH ) );
      } else {
         mc2log << "No reply";
      }
      mc2log << endl;
   }

   return res;
}

bool
request( Requester& requester,
         const MC2String& itemIDStr,
         const LangType& language,
         auto_ptr<SearchMatch>& result ) {

   auto_ptr<SearchMatch> inputMatch( SearchMatch::createMatch( itemIDStr ) );
   if ( inputMatch.get() == NULL ) {
      mc2log << warn << "[GetSearchItem]:request Bad itemIDStr " 
             << MC2CITE( itemIDStr ) << endl;
      return false;
   }
   return request( requester, inputMatch.get(), language, 
                   result );
}

} // End namespace GetSearchItem



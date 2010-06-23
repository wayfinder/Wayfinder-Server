/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLOSEST_SEARCHREQUEST_H
#define CLOSEST_SEARCHREQUEST_H

#include "config.h"

#include "SearchResultRequest.h"

#include "SearchRequestParameters.h"
#include "MC2Coordinate.h"
#include "MC2String.h"

class CoordinateRequest;
class TopRegionRequest;
class VanillaMatch;
class VanillaSearchReplyPacket;
class MatchInfoReplyPacket;
class TopRegionRequest;

/**
 *   Request to be used when searching using a coordinate and 
 *   distance sorting. The request will search the whole countr.
 *   <br />
 *   <b>FIXME</b>: Expand id:s of regions <br />
 *   <b>FIXME</b>: Currently the whole country is always searched. <br />
 */
class ClosestSearchRequest : public SearchResultRequest {
public:

   /**
    *   @param parentOrID  The parent request or the id of the request.
    *   @param params      The parameters.
    *   @param sortOrigin  The origin coordinate.
    *   @param itemQuery   The string to look for.
    *   @param topReq      Pointer to valid TopRegionRequest with data
    *   @param allowedMaps Set of allowed map ids. If empy all is allowed.
    */
   ClosestSearchRequest( const RequestData& parentOrID,
                         const SearchRequestParameters& params,
                         const MC2Coordinate& sortOrigin,
                         const MC2String& itemQuery,
                         const TopRegionRequest* topReq,
                         const set<uint32>& allowedMaps = set<uint32>());

   /**
    *   Search in one top region.
    *   @param parentOrID  The parent request or the id of the request.
    *   @param params      The parameters.
    *   @param itemQuery   The string to look for.
    *   @param topReq      Pointer to valid TopRegionRequest with data
    *   @param allowedMaps Set of allowed map ids. If empy all is allowed.
    */
   ClosestSearchRequest( const RequestData& parentOrID,
                         const SearchRequestParameters& params,
                         uint32 topRegion,
                         const MC2String& itemQuery,
                         const TopRegionRequest* topReq,
                         const set<uint32>& allowedMaps = set<uint32>());

   /**
    *   Returns the SearchParameters used by the request.
    */
   const SearchRequestParameters& getSearchParameters() const {
      return m_params;
   }

   /**
    *   Deletes data used by the request. Deletes all the matches.
    */
   virtual ~ClosestSearchRequest();

   /**
    *   If the request is done.
    */
   bool requestDone();
   
   /**
    *   Returns the status of the request.
    */
   StringTable::stringCode getStatus() const;

   /**
    *   Handles that a packet has been returned
    *   by the modules.
    */
   void processPacket( PacketContainer *pack );

   /**
    *   Returns a reference to the matches.
    *   All matches still in the vector when
    *   the request is deleted will be deleted.
    */
   const vector<VanillaMatch*>& getMatches() const;
   
private:

   /**
    *   Describes what to do.
    */
   enum state_t {
      /// Uses CoordinateRequest to find the country of the coordinate
      FINDING_COUNTRY         = 100,
      /// Uses TopRegionRequest to find the maps of the current country.
      TOP_REGION              = 200,
      /// Waits for the searchPackets to arrive.
      SEARCHING               = 300,
      /// The request is done, but with an error.
      ERROR                   = 404,
      /// The request is done and ok.
      DONE_OK                 = 1000
   } m_state;
   
   /**
    *   Initializes some variables. (Used by the constructors).
    */
   void init( const SearchRequestParameters& params,
              const MC2Coordinate& sortOrigin,
              const MC2String& itemQuery);

   /**
    *   Called when the country is known.
    *   Enqueues packets from TopRegionRequest.
    */
   inline void handleCountryFound();

   /**
    *   Called when the top regions are done.
    *   Enqueues the SearchRequestPackets.
    */
   inline void handleTopRegionReceived();

   /**
    *   Enqueues packets for the supplied maps.
    */
   inline void createSearchPackets(set<uint32>& maps);

   /**
    *   Handles the reception of a VanillaSearchReplyPacket in the
    *   <code>SEARCHING</code> state.
    *   @param vrp The VanillaSearchReplyPacket
    */
   inline void handleSearchingVanillaReply( VanillaSearchReplyPacket* vrp);
                                            
   /**
    *   Handles the reception of a MatchInfoReplyPacket in the state
    *   <code>SEARCHING</code>.
    *   @param mirp The received packet.
    */
   inline void handleSearchingMatchInfoReply( MatchInfoReplyPacket* mirp );

   /**
    *   Puts MatchInfoRequestPackets in the outgoing queue if the
    *   matches in <code>matches</code> contain housenumbers that
    *   need lookup in the MapModule.
    *   @param matches The matches from the SearchModule.
    *   @return Nbr of packets sent.
    */
   inline int sendMatchInfoPacket( const vector<VanillaMatch*>& matches);

   /**
    *   Should be called after setting state to DONE.
    */
   inline void handleDoneOK();

   /**
    *   Returns true if the supplied map is allowed.
    */
   inline bool mapAllowed(uint32 mapID) const;

   /**
    *   
    *   @param inState The state.
    *   @return A string describing the state.
    */
   inline static const char *stateAsString(state_t inState);

   /**
    *   Sets new state.
    *   @param newState The new state.
    *   @param line     Line number.
    */
   inline void setState(state_t newState, int line);
   
   /// The search parameters
   SearchRequestParameters m_params;
   /// The origin coordinate
   MC2Coordinate m_sortOrigin;
   /// The search string
   MC2String m_searchString;

   /// The coordinate object is used to be able to find country
   CoordinateRequest* m_coordReq;

   /// The top region request.
   TopRegionRequest* m_topRegionReq;

   /// Status
   StringTable::stringCode m_status;

   /// Number of packets to wait for in SEARCHING.
   int m_nbrSearchPacketsLeft;

   /// True if matchinfos were sent
   bool m_matchInfoPacketsWereSent;

   /// The matches
   vector<VanillaMatch*> m_matches;

   /// Vector of received packets to be deleted
   vector< PacketContainer* > m_recvPackets;

   /// Set of allowed map ids
   set<uint32> m_allowedMaps;

   /// pointer to valid topregionrequest *with* data
   const TopRegionRequest* m_topReq;
};

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHREQUEST_H
#define SEARCHREQUEST_H

#include "config.h"

#include <vector>
#include <set>
#include "MC2String.h"
#include "Request.h"

#include "SearchResultRequest.h"
#include "MC2Coordinate.h"

/* Haha! The include-chain stops here */
class SearchHandler;
class OldSearchRequestPacket;
class OverviewMatch;
class PacketContainer;
class VanillaMatch;
class SearchRequestParameters;
class CoveredIDsRequest;
class SearchReqIntData;
class IDPair_t;
class MC2BoundingBox;
class TopRegionRequest;

/**
 *
 *    Do not use getAnswer. Use getMatches and getOverviewMatch.
 *    If getMatches.size() returns > 0 - dont bother with the
 *    overview matches.
 */
class SearchRequest : public SearchResultRequest {
public:
   
   /**
    *   Creates a new SearchRequest with the coordinate 
    *   used instead of OverviewMatches.
    *   All municipals inside the radius will be searched. If distance
    *   sort or confidence sort is requested, the position will be used
    *   for that too.
    *   @param reqID        The request id.
    *   @param params       The search parameters.
    *   @param position     The center of the circle.
    *   @param itemQuery    String to search for.
    *   @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                       data
    *   @param radiusMeters Municipals inside this radius will
    *                       be considered.
    *   @param overParams   The overview search parameters, default params.
    */
   SearchRequest( const RequestData& parentOrID,
                  const SearchRequestParameters& params,
                  const MC2Coordinate& position,
                  const MC2String& itemQuery,
                  const TopRegionRequest* topReq,
                  int radiusMeters = 1000,
                  const SearchRequestParameters* overParams = NULL );

   
   /**
    *   Creates a new SearchRequest with a boundingbox
    *   used instead of OverviewMatches.
    *   All municipals inside the boundingbox will be searched. If distance
    *   sort or confidence sort is requested, the center will be used
    *   for that.
    *
    *   @param reqID        The request id.
    *   @param params       The search parameters.
    *   @param itemQuery    String to search for.
    *   @param bbox         Municipals inside this will be considered.
    *   @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                       data
    *   @param overParams   The overview search parameters, default params.
    */
   SearchRequest( const RequestData& parentOrID,
                  const SearchRequestParameters& params,
                  const MC2String& itemQuery,
                  const MC2BoundingBox& bbox,
                  const TopRegionRequest* topReq,
                  const SearchRequestParameters* overParams = NULL );


   /**
    *   Will search inside the municipals around the supplied itemID.
    *   Right now it will be implemented the same way as searching
    *   using a coordinate and a radius.
    *   All municipals inside the radius will be searched. If distance
    *   sort or confidence sort is requested, the position will be used
    *   for that too.
    *   @param reqID        The request id.
    *   @param params       The search parameters.
    *   @param origin       The center of the circle.
    *   @param itemQuery    String to search for.
    *   @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                       data
    *   @param radiusMeters Municipals inside this radius will
    *                       be considered. -1 will mean that 
    *                       MapModule will decide. (When implemented).
    *   @param overParams   The overview search parameters, default params.
    */
   SearchRequest( const RequestData& parentOrID,
                  const SearchRequestParameters& params,
                  const IDPair_t& origin,
                  const MC2String& itemQuery,
                  const TopRegionRequest* topReq,
                  int radiusMeters = -1,
                  const SearchRequestParameters* overParams = NULL );

   /**
    *   Will search inside the supplied areas.
    *   @param reqID         Request ID.
    *   @param params        The search parameters.
    *   @param selectedAreas Vector of itemids of the overview matches.
    *                        Should not be empty.
    *   @param itemQuery     String to search for.
    *   @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                       data
    *   @param sortOrigin    Origin to sort from.
    *   @param overParams   The overview search parameters, default params.
    *   @param radiusMeters The cutoff point for matches in meters.
    */
   SearchRequest( const RequestData& parentOrID,
                  const SearchRequestParameters& params,
                  const vector<IDPair_t>& selectedAreas,
                  const char* itemQuery,
                  const TopRegionRequest* topReq,
                  const MC2Coordinate& sortOrigin =
                  MC2Coordinate::invalidCoordinate,
                  const SearchRequestParameters* overParams = NULL,
                  int32 radiusMeters = -1 );


   /**
    *   Will search to overview search and possibly underview search.
    *   @param reqID         Request ID.
    *   @param params        The search parameters.
    *   @param topRegionID   ID of top region to search in.
    *   @param areaQuery     String to search for on overview.
    *   @param itemQuery     String to search for on underview.
    *   @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                       data
    *   @param sortOrigin    Origin to sort from.
    *   @param overParams   The overview search parameters, default params.
    */
   SearchRequest( const RequestData& parentOrID,
                  const SearchRequestParameters& params,
                  uint32 topRegionID,
                  const char* areaQuery,
                  const char* itemQuery,
                  const TopRegionRequest* topReq,
                  const MC2Coordinate& sortOrigin =
                  MC2Coordinate::invalidCoordinate,
                  const SearchRequestParameters* overParams = NULL );

   /**
    *   Search the supplied map. Should not be used very often.
    *   Could maybe be used as a subrequest later.
    *   @param reqID         Request ID.
    *   @param params        The search parameters.
    *   @param mapIDs        The map ids.
    *   @param itemQuery     String to search for on underview.
    *   @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                       data
    *   @param overParams   The overview search parameters, default params.
    */
   SearchRequest( const RequestData& parentOrID,
                  const SearchRequestParameters& params,
                  const vector<uint32>& mapIDs,
                  const char* itemQuery,
                  const TopRegionRequest* topReq,
                  const MC2Coordinate& sortOrigin =
                  MC2Coordinate::invalidCoordinate,
                  const SearchRequestParameters* overParams = NULL );

   /**
    *  Create SearchRequestPackets containing one mapID each
    *  from a SearchRequestPacket containing 0 or more mapIDs.
    *
    *  This constructor is deprecated. 
    *
    *  @param reqID is the unique request ID for this request
    *  @param p The packet to be expanded, the regionsInMatches in p is 
    *           ignored use regionsInMatches and 
    *           overviewRegionsInMatches.
    *  @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                      data
    *  @param uniqueOrFull if locations must be unique of full match 
    *                      to match.
    *  @param searchOnlyIfUniqueOrFull send search only if location is
    *         uniqueOrFull.
    *  @param regionsInMatches The type of regions to return in 
    *                          searchmatches.
    *  @param overviewRegionsInMatches The type of regions to return in 
    *                                  overviewmatches.
    *  @param lookupCoordinates True if coordinates should be included
    *                           in the matches. Coordinates can only be
    *                           returned if getMatches is used, not in
    *                           getAnswer.
    *  @param lookupBBoxes      True if MC2BoundingBoxes should be looked up.
    *                           Sets lookupCoordinates to true if true.
    *  @param sortOrigin        Must be set if distance sort should be used.
    */
   SearchRequest( const RequestData& parentOrID,                  
                  OldSearchRequestPacket *p,
                  const TopRegionRequest* topReq,
                  bool uniqueOrFull               = true,
                  bool searchOnlyIfUniqueOrFull   = false,
                  uint32 regionsInMatches         = 0,
                  uint32 overviewRegionsInMatches = 0,
                  bool   lookupCoordinates        = false,
                  bool   lookupBBoxes             = false,
                  const MC2Coordinate& sortOrigin =
                          MC2Coordinate::invalidCoordinate);
   
   /** 
    *   Destructor
    */
	virtual ~SearchRequest();

   /**
    *   Deletes the requestpacket (if called)
    */
   void deleteRequestPacket();

   /**
    *   Returns the radius actually used (when using radius).
    *   Can be smaller than the one sent in.
    */
   uint32 getRadiusMeters() const;
   
   /** Indicate request done when processed */
	void processPacket( PacketContainer* pack );

	/**
    *   Return a PacketContainer with a VanillaSomethingPacket.
    *   Does not return matches with coordinates or bboxes or
    *   matches sorted by distance, so do not use it.
    */
	PacketContainer* getAnswer();

   
   /**
    *   If the request is done.
    */
   bool virtual requestDone();

   /**
    *  The number of overviewmatches
    */
   uint32 getNbrOverviewMatches() const;

   
   /**
    *  The index'th overviewmatch.
    */
   OverviewMatch* getOverviewMatch( uint32 index ) const;


   /**
    *  Returns a vector with new matches.
    */
   OverviewMatch** getOverviewMatches();


   /**
    * Returns unique or full match in locations.
    */
   uint32 getUniqueOrFull();

   /**
    *   Returns the status of the request.
    */
   StringTable::stringCode getStatus() const;

   /**
    *   Returns a vector containing the vanilla matches.
    *   Will be unusable when the request is deleted.
    */ 
   const vector<VanillaMatch*>& getMatches() const;

   /**
    * Set the allowed maps.
    *
    * @param maps The allowed maps. NULL means all maps are allowed.
    */
   inline void setAllowedMaps( set< uint32 >* maps );

   /**
    * The used search request parameters.
    */
   const SearchRequestParameters& getSearchParameters() const;

   /**
    * Get the position if any.
    */
   MC2Coordinate getPosition() const;

  private:
   
   /**
    * Initializes the SearchHandler using a packet.  *
    * @param reqID        The request id.
    * @param params       The search parameters.
    * @param nbrSelectedAreas The number of selected areas, may be 0.
    * @param selectedAreaMapIDs The mapIDs for the selected areas.
    * @param selectedAreaItemIDs The itemIDs for the selected areas.
    * @param areaQuery String to use when searching for areas, not
    *                  used when nbr selected areas is greater than 0.
    * @param searchRegion The region(country) to search in when using
    *                     areaQuery.
    * @param itemQuery String to use when searching for items.
    * @param sortOrigin Must be set if distance sort should be used.
    * @param overParams   The overview search parameters, default params.
    */
   void initPacket( const SearchRequestParameters& params,
                    const vector<IDPair_t>& selectedAreas,
                    const char* areaQuery,
                    uint32 searchRegion,
                    const char* itemQuery,
                    const MC2Coordinate& sortOrigin,
                    const SearchRequestParameters* overParams );
   
   /**
    *   Checks the distances of the matches and removes the
    *   ones with too large distance (in the proximity case).
    *   @param matches 
    */
   inline void checkDistances( vector<VanillaMatch*>& matches );
    
                                
   
   /** 
    *   Sets up some variables.
    */
   void init();

   /**
    *   Creates the CoveredIDs-request and sets correct state.
    */
   void initCoveredIDs(const MC2Coordinate& origin,
                       int radiusMeters = 10000);
   
   /**
    *   Calls the getNextPacket funtion in the SearchHandler
    *   and enqueues the packets from it into the packetqueue.
    *   @return The number of packets enqueued.
    */ 
   int enqueuePacketsFromSearchHandler();

   /**
    *   Creates searchHandler when coveredids is done.
    */
   void handleCoveredIDsDone();

   /**
    *   Handles the case when the request has no more packets to send.
    *   In the future it will update the location names and sort the
    *   matches. No packets will be sent here.
    */
   void handleDoneOK();

   /**
    *
    */
   void handleCoordForOrigReceived(PacketContainer* pack);

   /**
    *   Prints the matches on mc2dbg.
    */
   void printMatches();

   /// SearchRequestData
   SearchReqIntData* m_data;

   /**
    * The allowed maps, may be NULL.
    */ 
   set< uint32 >* m_allowedMaps;

   /// Pointer to valid m_topRegionRequest or NULL if not available
   const TopRegionRequest* m_topRegionRequest;

};


// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -


void 
SearchRequest::setAllowedMaps( set< uint32 >* maps ) {
   m_allowedMaps = maps; 
}


/// Empty search request
class EmptySearchRequest: public SearchResultRequest {
public:
   EmptySearchRequest();
   void processPacket( PacketContainer* pack );
   StringTable::stringCode getStatus() const;
   const vector<VanillaMatch*>& getMatches() const;
   const SearchRequestParameters& getSearchParameters() const;
};

#endif


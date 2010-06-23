/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHRHANDLER_H
#define SEARCHRHANDLER_H

#include "config.h"

#include<set>
#include<vector>
#include<map>

#include "IDPairVector.h"
#include "SearchTypes.h"
#include "StringTable.h"
#include "MC2Coordinate.h"
#include "MC2BoundingBox.h"
#include "SearchRequestParameters.h"
#include "TopRegionRequest.h"
#include "NotCopyable.h"

class Request;
class OldSearchRequestPacket;
class VanillaSearchReplyPacket;
class MatchLink;
class OverviewMatch;
class PacketContainer;
class PacketContainerTree;
class TopRegionMatch;
class CoordinateOnItemObject;
class SearchMatch;
class VanillaMatch;
class ReplyPacket;
class MatchInfoReplyPacket;


/**
 *   Class that handles a search.
 *   Do not use getAnswer if you want coordinates in the reply,
 *   because they cannot be written into the packets.
 *   <br />
 */
class SearchHandler: private NotCopyable {
public:
   
   /**
    * Handles the SearchRequest in p.
    * @param p The search request packet to be handled.
    *          The regionsInMatches in p is ignored use regionsInMatches
    *          and overviewRegionsInMatches.
    * @param request is the request using this SearchHandler. Used for 
    *        getting requestIDs.
    * @param params  The params just to make sure we don't miss anything.
    * @param topReq       Pointer to TopRegionRequest, witch current/valid
    *                     data
    * @param uniqueOrFull if locations must be unique of full match 
    *        to match.
    * @param searchOnlyIfUniqueOrFull send search only if location is
    *        uniqueOrFull.
    * @param regionsInMatches The type of regions to return in 
    *                         searchmatches.
    * @param overviewRegionsInMatches The type of regions to return in 
    *                                 overviewmatches.
    * @param lookupCoordinates        True if coordinates should be
    *                                 looked up for the hits. Use
    *                                 getMatches if you want coordinates,
    *                                 not getAnswer.
    * @param lookupBBoxes             True if bboxes should be included
    *                                 in the matches. 
    * @param overParams   The overview search parameters, default params.
    */
   SearchHandler( OldSearchRequestPacket *p,
                  Request* request,
                  const SearchRequestParameters& params,
                  const TopRegionRequest* topReq,
                  bool uniqueOrFull               = true,
                  bool searchOnlyIfUniqueOrFull   = false,
                  uint32 regionsInMatches         = 0,
                  uint32 overviewRegionsInMatches = 0,
                  bool lookupCoordinates = false,
                  bool lookupBBoxes = false,
                  const MC2Coordinate& sortOrigin =
                             MC2Coordinate::invalidCoordinate,
                  const SearchRequestParameters* overParams = NULL);
void sortOverviews() {
      calcDistsAndSort( m_unexpandedOverviewMatches,
                        m_sortOrigin,
                        m_params.getSortingType() );
   }

   
   /**
    * Deletes all currently allocated varibles.
    */
   virtual ~SearchHandler();

   /**
    * Prints some data.
    */
   void printData();
   
   /**
    * Handles a received packetcontainer.
    * @return true if the packet was handled.
    */
   bool processPacket( PacketContainer* cont );

   /**
    *   Returns the status of the request.
    */
   uint32 getStatus() const;
   
   /**
    *   Returns the next packet to be sent or NULL if there currently
    *   isn't any to send.
    */
   PacketContainer* getNextPacket();


   /**
    *   Returns the answer, might be NULL.
    */
   PacketContainer* getAnswer();


   /**
    *   Returns true if there is nothing more to be done.
    */
   bool requestDone();


   /**
    *   The total number of locations found
    */
   uint32 getNbrLocationsFound();


   /**
    * The number of overviewmatches.
    */
   uint32 getNbrOverviewMatches();

   /**
    *   Returns the vanilla matches.
    *   These can contain coordinates which the ones read from
    *   the answer packet cannot.
    *   Invalid after the SearchHandler is deleted.
    *   @return A reference to the internal vector with matches.
    */
   const vector<VanillaMatch*>& getMatches() const;

   /**
    *   Returns the matches for writing and changing.
    *   Used by SearchRequest.
    */
   inline vector<VanillaMatch*>& getMatchesForWriting();
   
   /**
    *   Returns a vector of expanded overview matches.
    *   To be used by SearchRequest only.
    */
   inline const vector<OverviewMatch*> getExpandedOverviewMatches() const;
   
   /**
    * The index'th overviewmatch.
    */
   OverviewMatch* getOverviewMatch( uint32 index ) const;


   /**
    *   Returns a vector with new matches.
    */
   OverviewMatch** getOverviewMatches();


   /**
    * Returns unique or full match in locations. If locations otherwise 0.
    */
   uint32 getUniqueOrFull();


   /**
    * Merges the array of MatchLinks.
    * The result will be one list of links at position 0 in the array.
    * @param loNdx the lowest index of the array to merge
    * @param hiNdx the highest index of the array to merge
    * @param resNdx the index where the result will be put
    * @param matches the array of lists
    * @param nbrLists the size of the array
    * @param sorting the type of comparison to perform for mergesort
    */
   static void merge( uint32 loNdx,  
                      uint32 hiNdx,
                      uint32 resNdx, 
                      MatchLink **matches,
                      uint32 nbrLists,
                      SearchTypes::SearchSorting sorting );

   
   /**
    * Packs a linked list of matches into a packet
    */
   static void convMatchLinkToPacket(   MatchLink *matchLink,
                                        VanillaSearchReplyPacket *p );


   /**
    * The used search request parameters.
    */
   inline const SearchRequestParameters& getSearchParameters() const;

   /**
    *   Sorts the supplied matches in distance order.
    *   @param matches The matches to sort.
    *   @param origin  The point to take the distance from.
    *   @param sorting  The type of sorting to use.
    */
   template<class SEARCHMATCH>
      static void calcDistsAndSort(vector<SEARCHMATCH*>& matches,
                                   const MC2Coordinate& origin,
                                   SearchTypes::SearchSorting sorting);


  private:

   /**
    *   Dequeues one packet container from the packetcontainertree.
    *   @return Packetcontainer or NULL.
    */
   PacketContainer* dequeFromTree();

   /**
    *   Returns true if the supplied match is allowed 
    *   to be returned by checking the wholeMaps and items.
    *   @param match     Match to check.
    *   @param mapIDs    Map containing map ids mapped by OverviewMatches.
    *   @param wholeMaps Set of totally covered maps.
    *   @param items     Pairs of mapID and itemIDs for municipals.
    */
   static inline bool
                  idAllowed(const OverviewMatch* match,
                            const map<const OverviewMatch*, uint32>& mapIDs,
                            const set<uint32>& wholeMaps,
                            const set<IDPair_t>& allowedItems);
   
   /**
    *   Check the overview matches and remove the ones with
    *   restrictions if there are matches without restrictions.
    *   @param matches A vector of matches which can be changed.
    *   @param nbrOverviewLocations Will be recalculated if the number changes.
    *   @param topRegion The topregion to search in.
    *   @param mapIDs    The overviewmatches in the vector together with 
    *                    their overview map ids. For removal when complete
    *                    maps are not covered. Will be updated to match the
    *                    vector.
    */
   static void reduceOverviewMatches(vector<OverviewMatch*>& matches,
                                     uint32& nbrOverviewLocations,
                                     const SearchRequestParameters& params,
                                     const TopRegionMatch& topRegion,
                                     map<const OverviewMatch*, uint32>&mapIDs);
      
   /// Find out what to do when all overview packets are received.
   void handleAllOverviewsReceived();

   /// Makes unexpanded overview matches from old version mapids and stuff
   bool makeOverviewMatchesFromMasks( uint32 nbrMapIDs,
                                      const uint32* mapIDs,
                                      uint32 nbrMasks,
                                      const uint32* maskItemIDs,
                                      const const_char_p* maskNames);
   
   /// Create search request packets from overview matches
   bool makeSearchMaskPackets( const vector<OverviewMatch*>& overviewMatches);
   
   /// Create search request packets from mapIDs and Masks
   bool makeSearchMaskPackets( const uint32* mapIDs, uint32 nbrMapIDs, 
                               const uint32* masks, 
                               const uint32* maskItemIDs,
                               const char** maskNames, uint32 nbrMasks );

   /**
    *   Converts the matchlinks to a vector of matches to be
    *   returned to the user.
    */
   void makeMatchVector();
  
   /// State of the handler
   enum state { START = 0,
                SEND_TOP_REGION = 10,
                AWAIT_TOP_REGION = 11 ,
                SEND_OVERVIEW = 20,
                AWAIT_OVERVIEW = 21 ,
                SEND_SEARCH = 30,
                AWAIT_SEARCH = 31,
                SEND_EXPAND_ITEM = 40,
                AWAIT_EXPAND_ITEM = 41,
                OVERVIEWEXPAND = 60,
                OVERVIEWEXPAND_THEN_OVERVIEW_CHECK = 61,
                UNEXPAND_OVERVIEW = 70,
                AWAIT_UNEXPAND_OVERVIEW = 71,
                COORDINATES = 73,
                /// Means that the MatchInfoPackets have not been received yet
                ALMOST_DONE_2 = 80,
                DONE = 1000 } m_state;

   /**
    *   Returns a pointer to a string describing current state.
    */
   inline static const char* stateAsString(state inState);
   
   /**
    *   Sets new state with optional debug output.
    *   @param newState New state to set.
    *   @param line The line number if you want it to be printed.
    */
   inline void setState(state newState, int line = -1);
   
   /**
    *   Creates the necessary packets for OVERVIEWEXPAND.
    *   @return The next state to use.
    */
   inline state initOverviewExpand();

   /**
    *   Check if all match infos are receieved. If they
    *   are, then update the matches with the new info.
    *   @return DONE if everything is done. ALMOST_DONE_2 if not.
    */
   state checkMatchInfosReceived();
   
   /**
    *    Handles a received overview match expansion packet.
    *    @param pack The packetcontainer.
    *    @return The next state to use.
    */
   inline state processOverviewExpand(PacketContainer* pack);

   /**
    *    Puts MatchInfoRequestPackets into the outgoing packet
    *    quueue and increases the variable that keeps track of
    *    how meny replies to expect.
    *    @param packet The Packet to get the matches from.
    */
   inline void sendMatchInfoPacket(VanillaSearchReplyPacket* packet);

   /**
    *   Handles the TopRegion data in the state AWAIT_UNEXPAND_OVERVIEW.
    *   If everything is ok it will enqueue new packets in
    *   the outgoing queue and keep the state. If error
    *   state = DONE.
    */
   inline void handleTopRegionReceivedInUnexpand();

   /**
    *   Handles that an unexpand packet is received.
    *   Can change state if needed.
    */
   inline void handleUnexpandItemPackets(const ReplyPacket* rep);
   
   /**
    *   Prepare the sending of coordinate on item.
    */
   inline SearchHandler::state initCoordinates();

   /**
    *
    */
   inline state handleAllCoordinatesReceived();
   
   /// The request of this handler
   Request* m_request;

   
   /// The SearchRequestPacket to use
   OldSearchRequestPacket* m_searchPacket;


   /// The answer
   PacketContainer* m_answer;

   /// The overview replies, sorted by PacketID.
   map<uint32, PacketContainer*> m_overviewReplies;

   /**
    *   The overview mapIDs of the overview search requests
    *   keyed on packetID
    */
   map<uint32, uint32> m_overviewRequestMaps;
   
   /// The number of for overview searched (expected)
   int m_nbrOverviewSearchesLeft;
   
   /**
    *  The overviewmatches. We have to use a vector since
    *  the api contains getOverviewMatch(i)
    */
   vector<OverviewMatch*> m_unexpandedOverviewMatches;

   /**
    *  Vector of expanded overview matches.
    *  (With lower level map and item ids).
    */
   vector<OverviewMatch*> m_expandedOverviewMatches;

   /// The number of OverviewMatches to expand. -1 means not initialized.
   int m_nbrOverviewMatchesToExpand;
   
   /// The overview matches with their overview map ids
   map<const OverviewMatch*, uint32> m_overviewMapIDsByOverviewMatch;


   /// The number of locations in the overviewmatches
   uint32 m_nbrOverviewLocations;
   

   /// The next search request packet to send
   uint32 m_currentRequestIndex;


   /// The number of search request packets to send
   uint32 m_nbrRequestPackets;

   
   /// The number of received packets
   uint32 m_nbrReceivedPackets;


   /// The packets to send
   PacketContainer **m_requestPackets; // array of pointers

   
   /// The packetIDs of the serach request packets
   uint32* m_requestPacketIDs;


   /// The search request packet answers
   PacketContainer **m_replyPackets; // array of pointers

   /// array of pointers to MatchLink
   MatchLink **m_matches; 

   /// The expanditem packet
   PacketContainer *m_expandItemCont;
   

   /// The packetID of the ExpandItem packet
   uint32 m_expandItemPacketID;


   /// The overview replies
   PacketContainer *m_expandItemReply;

   /// True if first packet is not sent yet. For debug
   bool m_firstPacket;
   
   // Search stuff
   char* m_zipCode;
   uint32 m_nbrLocations;
   char** m_locations;
   uint32 m_nbrCategories;
   uint32* m_categories;
   
   char* m_searchString;

   /// Sort origin
   MC2Coordinate m_sortOrigin;
   
   /// Search parameters
   SearchRequestParameters m_params;
   
   /// Overview Search parameters
   SearchRequestParameters m_overParams;

   /// Top region.
   StringTable::countryCode m_topRegionID;

   /**
    *   Container to put outgoing packet containers into.
    *   Used by getNextPacket in some states.
    */
   PacketContainerTree* m_packetsReadyToSend;

   /**
    *   The current top region.
    */
   const TopRegionMatch* m_topRegion;

   /// Coordinate object for looking up coords
   CoordinateOnItemObject* m_coordinateHandler;
 
   
   /**
    *  Map to find coordinate-lookuped matches in.
    *  Index is the same as the index used when adding to the
    *  coordinate object.
    */
   map<uint32, SearchMatch*> m_lookupCoordsMap;
   
   /// Vector containing the VanillaMatches. Could replace m_matches?
   vector<VanillaMatch*> m_matchVector;

   /**
    *   Vector containing pointers to the matches in m_matchVector and
    *   the regions waiting for unexpansion (back to overviewmapid)
    */
   vector<VanillaMatch*> m_matchesForUnexpansion;

   /**
    *   Vector containing replies to MatchInfoRequests.
    */
   vector<MatchInfoReplyPacket*> m_matchInfoReplies;

   /**
    *   The number of expected matchinforeplypackets.
    */
   int m_nbrExpectedMatchInfoPackets;

   /**
    *   Bounding boxes in which to search (besides the items) and maps.
    */
   vector<MC2BoundingBox> m_bboxesToSearchIn;

   /// Pointer to valid m_topRegionRequest or NULL if not available
   const TopRegionRequest* m_topRegionRequest;

   /**
    * The connection from m_expandedOverviewMatches index to 
    * m_unexpandedOverviewMatches index.
    */
   vector< uint32 > m_expandedToUnexpandedOverviewIndex;
};

// - Implementation of inlined functions
const vector<OverviewMatch*>
SearchHandler::getExpandedOverviewMatches() const
{
   return m_expandedOverviewMatches;
}

inline vector<VanillaMatch*>&
SearchHandler::getMatchesForWriting()
{
   return m_matchVector;
}


inline const SearchRequestParameters& 
SearchHandler::getSearchParameters() const {
   return m_params;
}

#endif


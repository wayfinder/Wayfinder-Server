/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROXIMITYREQUEST_H
#define PROXIMITYREQUEST_H

#include "config.h"
#include "SearchResultRequest.h"
#include "SearchTypes.h"
#include "SharedProximityPackets.h"
#include "StringTable.h"
#include "MC2Coordinate.h"

class ProximityItemSetRequestPacket;
class ProximityPositionRequestPacket;
class PositionRequestPacket;

class UserUser;
class UserCellular;

class PacketContainer;
class MatchLink;
class VanillaSearchReplyPacket;
class SearchRequestParameters;

/**
 *    Request for proximity Search. 
 *
 *    This request is used to get all the companies that are located 
 *    close to a given location. The given area might be given by 
 *    specifying the circle (coordinates and radius) or by given a set 
 *    of street segment items. The states for this request are:
 *    <dl>
 *       <dt>NOTHING_SENT</dt>
 *       <dd>The initial state, nothing has been done.</dd>
 *       <dt>GET_ALL_IDS</dt>
 *       <dd>To send packet to map module to get the IDs that are 
 *           coverd with the area specified by the area described by
 *           a position (coordinate or mapID.itemID.offset) and a 
 *           distance.</dd>
 *       <dt>AWAITING_ALL_IDS</dt>
 *       <dd>Wait for @c CoverdIDsReplyPacket.</dd>
 *       <dt>GET_ALL_ITEMS_COVERED</dt>
 *       <dd>The specified area coverd several maps, we have to send 
 *           one packet to the map module for each map.</dd>
 *       <dt>AWAITING_ALL_ITEMS_COVERED</dt>
 *       <dd>Wait for the @c CoverdIDsReplyPackets, when more than one
 *           @c CoverdIDsRequestPacket was send.</dd>
 *       <dt>GET_ALL_FITTING_ITEMS</dt>
 *       <dd>To get all the item that are located in the spceified area
 *           and matches the given search parameters.</dd>
 *       <dt>GET_ALL_FITTING_FROM_SET</dt>
 *       <dd>This state is entered if the indata conststed of a list of 
 *             item IDs. Here they are take care of.</dd>
 *       <dt>AWAITING_ALL_FITTING_ITEMS</dt>
 *       <dd>Wait for @cSearchReplyPacket to get the item to return.</dd>
 *       <dt>DONE</dt>
 *       <dd>This request is finished and will be returned to the caller.</dd>
 *    </dl>
 *
 *    \IMG{requestProximity.gif}
 *
 */
class ProximityRequest : public SearchResultRequest {
   public:
      /**
       *    Makes a proximity search from a position.
       * 
       *    @param requestID The unique id of the requets.
       *    @param packet    The ProximityPositionRequestPacket with the 
       *                     position.
       */
      ProximityRequest( const RequestData& requestID,
                        ProximityPositionRequestPacket* packet );
      
      /**
       *    Makes a proximity search from a set of items. Used if the 
       *    items in the proximity is allready known.
       *   
       *    @param requestID  The unique id of the requets.
       *    @param packet     The ProximityItemSetRequestPacket with 
       *                      the items.
       */
      ProximityRequest( const RequestData& requestID, 
                        ProximityItemSetRequestPacket* packet );

      /**
       * Makes a proximity search using a boundingbox.
       *
       * @param requestID The unique id of the requets.
       * @param params    The search parameters.
       * @param bbox      The boundingbox to get item inside.
       * @param itemTypes The itemtypes to look for.
       * @param searchString The string search that items must match.
       */
      ProximityRequest( const RequestData& requestID, 
                        const SearchRequestParameters& params,
                        const MC2BoundingBox& bbox,
                        const set<ItemTypes::itemType>& itemTypes,
                        const char* searchString = "" );

      
      /**
       *    Destructs the request.
       */
      virtual ~ProximityRequest();

      /**
       *    Inits all variables deleted in the destructor to NULL.
       */
      void init();

      /**
       *    Get the next packet from this request.
       *    @return the next packet to send or NULL if nothing to send.
       */
      PacketContainer* getNextPacket();
      
      /**
       *    Handle an answer, containing one ReplyPacket.
       *    @param ans  The answer, containing one ReplyPacket.
       */
      void processPacket(PacketContainer *ans);   
      
      /**
       *    Get the answer, in terms of one ProximityReplyPacket or
       *    VanillaSearchReplyPacket depending on proximitySearchItemsType.
       *    @return  A ProximityReplyPacket/VanillaSearchReplyPacket
       *             if all went ok or NULL if something went very wrong.
       */
      PacketContainer* getAnswer();

      /**
       *    Returns the status of the request.
       */
      StringTable::stringCode getStatus() const;      

      /**
       *    Implements part of SearchResultRequest.
       *    Returns a vector containing the vanilla matches.
       *    Will be unusable when the request is deleted.
       */
      const vector<VanillaMatch*>& getMatches() const;

      /**
       *    Returns the SearchParameters used by this request.
       */
      const SearchRequestParameters& getSearchParameters() const {
         return *m_params;
      }
      
      /**
       *    Adds a list of matches to a VanillaSearchReplyPacket.
       *    @param matchLink  The first MatchLink of the list.
       *    @param p          The VanillaSearchReplyPacket to add the 
       *                      matches to.
       *    @param maxNumberMatches Ts the maximun number of matches 
       *                      added to the packet.
       */
      static void convMatchLinkToPacket(MatchLink *matchLink,
                                        VanillaSearchReplyPacket *p,
                                        uint32 maxNumberMatches = MAX_UINT32);
      
      
   private:
      /**
       * Makes a CoveredIDsRequestPacket from a (Proximity)PositionPacket.
       * 
       * @param posPack A ProximityPositionPacket or PositionPacket.
       * @return A new PacketContainer with a CoveredIDsRequestPacket
       *         or NULL if posPack wasn't ok.
       */
      PacketContainer* makeCoveredIDsRequestPacket( Packet* posPack );


      /// The answer with items, perhaps null.
      PacketContainer* m_answer;
      
      /// The questioning (proximity) position packet
      Packet* m_positionPacket;

      /// The packet which to send asking for covered IDs
      PacketContainer* m_coveredIDs;

      /// The packetID of the coveredIDs packet.
      uint16 m_coveredIDsID;

      /// CoveredIDs reply
      PacketContainer* m_coveredIDsReply;
      
      /// CoveredIDs requests if many maps
      PacketContainer** m_coveredIDsRequests;

      /// CoveredIDS reply if many maps
      PacketContainer** m_coveredIDsReplys;

      /// The mapcount, the number of maps sent of m_nbrMaps
      uint16 m_currentCoveredRequest;

      /// The mapcount, the number of maps sent of m_nbrMaps
      uint16 m_currentCoveredReply;
   
      /**
       *    @name Position parameters
       *    @memo Variables needed to handle the position of the user.
       *    @doc  Variables needed to handle the position of the user.
       */
      //@{
         /**
          *    Position of the user.
          */
         MC2Coordinate m_position;

         /**
          *    The maximum distance from (m_lat, m_lon) for the items to 
          *    include. This is the radius of the circle.
          */
         uint32 m_dist;

         /**
          * The inner radius of the circle.
          */
         uint32 m_innerRadius;

         /**
          * The startAngle.
          */
         uint16 m_startAngle;

         /**
          * The stopAngle.
          */
         uint16 m_stopAngle;

         /// The type of content.
         PositionRequestPacketContent m_packetContent;
 
         ///   ID of the item.
         uint32 m_itemID;

         /**
          *    Offset from node 0 at item with m_itemID, if specified 
          *    for the user.
          */
         uint16 m_offset;

         /**
          *    Map ID for the given item (m_itemID), if specified.
          */
         uint32 m_mapID;
      //@}

      /// The questioning item set packet
      ProximityItemSetRequestPacket* m_itemSetPacket;

      /// The number of maps this request covers
      uint16 m_nbrMaps;

      /// The request to searchmodule
      PacketContainer** m_searchRequests;
      
      /// The answers from searchmodule
      PacketContainer** m_searchAnswers;

      /// The mapcount, the number of maps sent of m_nbrMaps
      uint16 m_currentSearchReply;

      /// The mapcount, the number of maps sent of m_nbrMaps
      uint16 m_currentSearchRequest;
      
      /// The mapcount, the number of maps sent of m_nbrMaps
      uint16 m_currentMap;

      /**
       *    @name Search parameters
       *    @memo Variables needed when searching for the matching items.
       *    @doc  Variables needed when searching for the matching items.
       */
      //@{
         /// The mask of databases available to the user.
         uint8 m_dbMask;
      
         /// The type of the items to return
         proximitySearchItemsType m_itemType;

         /**
          *    Should we use any string when searching or returning all 
          *    the items in the given area?
          */
         bool m_useString;

         ///   String used when (if) searching among the results
         char* m_searchString;
         
      //@}

      /** 
       *    The state of the request. An explanation of the states are
       *    given in the class documentation.
       */
      enum states { NOTHING_SENT, 
                    GET_ALL_IDS,
                    AWAITING_ALL_IDS,
                    GET_ALL_ITEMS_COVERED,
                    AWAITING_ALL_ITEMS_COVERED,
                    GET_ALL_FITTING_ITEMS,
                    GET_ALL_FITTING_FROM_SET,
                    AWAITING_ALL_FITTING_ITEMS,
                    DONE} m_state;


   /**
    *   The matches so far.
    */
   vector<VanillaMatch*> m_matches;

   /// PacketContainers to delete upon destruction
   vector<PacketContainer*> m_deleteLater;

   /// Saved answer. Will be put in m_deleteLater for deletion upon destr.
   PacketContainer* m_savedAnswer;
   
   /**
    *   Pointer to the SearchRequestParameters. May not be NULL.
    */
   SearchRequestParameters* m_params;
   
};

#endif // PROXIMITYREQUEST_H 


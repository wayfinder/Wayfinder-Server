/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEEXPANDER_H
#define ROUTEEXPANDER_H

#include "config.h"
#include "Request.h"
#include "PacketContainer.h"
#include "StringTable.h"
#include "TopRegionMatch.h"

class ExpandRouteConcatenator; // forward decl
class ExpandRouteRequestPacket; // forward decl
class ExpandRouteReplyPacket; // forward decl
class RouteReplyPacket; // forward decl
class SubRouteVector;   // forward decl
class IDTranslationReplyPacket;
class TopRegionReplyPacket;
class LandmarkHead;
class DisturbanceDescription;

/**
 *    Objects of this class will expand routes from lists of IDs to 
 *    strings and/or coordinates. This is done by sending packets to 
 *    the map module. 
 *
 */
class RouteExpander {
   public:  
      /**
       *    Create a new route expander.
       *
       *    @param   request           The request that this object "work 
       *                               for".
       *    @param   routeReply        The route that should be expanded.
       *                               Must be valid for the whole lifetime
       *                               of this.
       *    @param   expandRouteType   The type of expansion.
       *    @param   uTurn             Do we have an U-turn?
       *    @param   language          The prefered language.
       *    @param   landmarks         Should landmarks be added to the
       *                               result.
       */
      RouteExpander(Request* request,
                    RouteReplyPacket* routeReply,
                    uint32 expandRouteType,
                    bool uTurn,
                    bool noConcatinate,
                    uint32 passedRoads,
                    StringTable::languageCode language = StringTable::ENGLISH,
                    bool abbreviate = true,
                    bool landmarks = false,
                    bool removeAhead = false,
                    bool nameChangeWP = false);
      
      /**
       *    Create a new route expander. Newer version that does not use
       *    the RouteReplyPacket.
       *
       *    @param   request           The request that this object "work 
       *                               for".
       *    @param   srVect            The route that should be expanded.
       *                               Must be valid for the whole lifetime
       *                               of this. NB! Is deleted by this
       *                               class because of RouteObject.
       *    @param   expandRouteType   The type of expansion.
       *    @param   uTurn             Do we have an U-turn?
       *    @param   language          The prefered language.
       *    @param   landmarks         Should landmarks be added to the
       *                               result.
       */
      RouteExpander(Request* request,
                    const SubRouteVector* srVect,
                    uint32 expandRouteType,
                    bool uTurn,
                    bool noConcatinate,
                    uint32 passedRoads,
                    StringTable::languageCode language = StringTable::ENGLISH,
                    bool abbreviate = true,
                    bool landmarks = false,
                    bool removeAhead = false,
                    bool nameChangeWP = false);
      
      /**
       *    Delete this route expander.
       */
      ~RouteExpander();

      /**
       *    Handle one packet that have come from the modules. 
       *
       *    @param cont  A packet container that contains a reply
       *                 packet.
       *    @return  An Expand Route Reply packet containing the complete,
       *             expanded route or NULL if no finnished route yet.
       */
      void packetReceived( const PacketContainer* cont );

      /**
       *    Get the next request packet to send. The request packet is 
       *    returned in a packet container.
       *
       *    @return  A packet container containg the next request packet to
       *             send to the modules.
       */
      PacketContainer* getNextRequest();

      /**
       *    Find out if the processing is completed or not.
       *    @return  True if done, false otherwise.
       */
      bool getDone();

      /**
       *    Create the answer that is calculated in this object. The answer
       *    consists of one ExpandRouteReplyPacket, containing the complete
       *    route.
       *
       *    @return  A packet containing the expanded route. Do not delete.
       */
      PacketContainer* createAnswer();

      /**
       *    @name Get start direction of route.
       *    Get the start-direction of the route in terms of house numbers
       *    on the street where the route start.
       */
      //@{
         /**
          *    Get the start direction in terms of driving towards higher
          *    or lower street numbers.
          *    @return  The start direction of the route in terms of    
          *             increasing or decreasing house numbers.
          */
         inline ItemTypes::routedir_nbr_t getStartDirectionHousenumber();

         /**
          *    Get the start direction in terms of driving with the odd/even
          *    numbers to the left or right.
          *    @return  The start direction of the route in terms of    
          *             odd/even street numbers to the left/right.
          */
         inline ItemTypes::routedir_oddeven_t getStartDirectionOddEven();
      //@}

         void addDisturbanceInfo(const vector<DisturbanceDescription>& dists);

         void setUseAddCost(bool useAddCosts);
         
         
         
   private:
      /**
       * This function is called if all expand route packets are received. 
       * It concatenates the received expand route packets into one reply.
       */
      ExpandRouteReplyPacket* concatenateExpandRoute();

      /**
       *    Loop the landmarks in one route-part and select which 
       *    ones to keep in the route. The selection is performed
       *    based on the importance of the landmarks and the length
       *    of the route-part.
       *
       *    @param landmarks     The landmarks to check.
       *    @param routeLinkDist The length of the route-part for
       *                         which to select landmarks.
       */
      void selectLandmarks(LandmarkHead* landmarks, uint32 routeLinkDist);
      
      /**
       * The states of this class.
       */
      enum state {
         /// Sending and receiving IDTranslation.
         IDTRANS,

         /// Sending and receiving top region request.
         TOP_REGION,

         /// Sending and receiving expand route request.
         EXPAND,

         /// Expand concatenated ok.
         DONE,

         /**   
          * Something went wrong when processing any part of the 
          * task for this class.
          */
         ERROR
      } m_state;


      /**
       * Creates the IDTranslation packets needed, perhaps none then
       * state TOP_REGION is returned.
       * If to send IDTranslation packets m_idtransReplyPackets is 
       * allocated here.
       *
       * @param routeReply The RouteReplyPacket with the route.
       * @param packetsReadyToSend The PacketContainerTree to add 
       *                           packets to send to.
       * @return The next state. IDTRANS if to send IDTranslation packets,
       *         TOP_REGION if createTopRegionPackets should be 
       *         called and ERROR if something is really wrong.
       */
      state createIDTransPackets( 
         RouteReplyPacket* routeReply,
         PacketContainerTree& packetsReadyToSend );

      /**
       * Creates the IDTranslation packets needed, perhaps none then
       * state TOP_REGION is returned.
       * If to send IDTranslation packets m_idtransReplyPackets is 
       * allocated here.
       *
       * @param srVect The SubRouteVector with the route.
       * @param packetsReadyToSend The PacketContainerTree to add 
       *                           packets to send to.
       * @return The next state. IDTRANS if to send IDTranslation packets,
       *         TOP_REGION if createTopRegionPackets should be 
       *         called and ERROR if something is really wrong.
       */
      state createIDTransPackets( 
         const SubRouteVector* srVect,
         PacketContainerTree& packetsReadyToSend );

      /**
       * Creates the Top region packets needed.
       *
       * @param packetsReadyToSend The PacketContainerTree to add 
       *                           packets to send to.
       * @return The next state.
       *         ERROR if something is really wrong.
       */
      state createTopRegionPackets( 
         PacketContainerTree& packetsReadyToSend ) const;

      /**
       * Creates the expand route packets from a RouteReplyPacket.
       * Allocates m_expandRouteReplyPackets and sets 
       * m_nbrExpandRoutePackets.
       *
       * @param routeReply The RouteReplyPacket with the route.
       * @param idtransReplyPackets The IDTrans packets.
       * @param nbrIDTransPackets The number of IDTrans packets.
       * @param topregionPacket The top region packet.
       * @param startOffset Set to the start offset of the route.
       * @param endOffset Set to the end offset of the route.
       * @param startDir Set to the start dir of the route (toward 0).
       * @param endDir Set to the end dir of the route (toward 0).
       * @param addTree The PacketContainerTree to add expand route packets
       *                to.
       * @return The next state.
       *         ERROR if something is really wrong.
       */
      state createExpandRoutePackets( 
         RouteReplyPacket* routeReply, 
         IDTranslationReplyPacket** idtransReplyPackets, 
         uint32 nbrIDTransPackets, TopRegionReplyPacket* topregionPacket,
         byte expandRouteType, uint16& startOffset, 
         uint16& endOffset, bool& startDir, bool& endDir, 
         PacketContainerTree& addTree );

      /**
       * Creates the expand route packets from a SubRouteVector.
       * Allocates m_expandRouteReplyPackets and sets 
       * m_nbrExpandRoutePackets.
       *
       * @param srVect The SubRouteVector with the route.
       * @param idtransReplyPackets The IDTrans packets.
       * @param nbrIDTransPackets The number of IDTrans packets.
       * @param topregionPacket The top region packet.
       * @param expandRouteType The type of route expansion wanted.
       * @param startOffset Set to the start offset of the route.
       * @param endOffset Set to the end offset of the route.
       * @param startDir Set to the start dir of the route (toward 0).
       * @param endDir Set to the end dir of the route (toward 0).
       * @param addTree The PacketContainerTree to add expand route packets
       *                to.
       * @return The next state.
       *         ERROR if something is really wrong.
       */
      state createExpandRoutePackets( 
         const SubRouteVector* srVect, 
         IDTranslationReplyPacket** idtransReplyPackets, 
         uint32 nbrIDTransPackets, TopRegionReplyPacket* topregionPacket,
         byte expandRouteType, uint16& startOffset, 
         uint16& endOffset, bool& startDir, bool& endDir, 
         PacketContainerTree& addTree );

      /**
       * Handle a ID trans reply. 
       *
       * @param cont The packet to handle.
       * @return The next state possibly ERROR if problem.
       */
      state handleIDTransPack( const PacketContainer* cont );

      /**
       * Handle a Top region reply. 
       *
       * @param cont The packet to handle.
       * @return The next state possibly ERROR if problem.
       */
      state handleTopRegionPack( const PacketContainer* cont );

      /**
       * Transforms a undermapID to a countrycode.
       *
       * @param countryMapID The undermapID to find country for.
       * @param topRegions The top regions, with countries, to find
       *                   countryMapID in.
       * @return The country code for countryMapID, NBR_COUNTRY_CODES if
       *         error.
       */
      uint32 getCountryID( uint32 countryMapID, 
                           TopRegionMatchesVector& topRegions ) const;

      /**
       * The Request that this belongs to.
       */
      Request* m_request;

      /**
       * True if nothing more to do.
       */
      bool m_done;

      /**
       * The finished route or NULL if a problem.
       */
      PacketContainer* m_answer;
      
      /**
       * The packets that are ready to send.
       */
      PacketContainerTree m_packetsReadyToSend;

      /**
       * The number of expand route request packets sent and how many
       * we expect back before starting to concatenate them.
       */
      uint32 m_nbrExpandRoutePackets;

      /**
       * The number of expand route request packets received so far.
       */
      uint32 m_nbrReceivedExpandPackets;

      /**
       * The received expand route packets.
       */
      ExpandRouteReplyPacket** m_expandRouteReplyPackets;

      /**
       * The route reply packet.
       */
      RouteReplyPacket* m_routeReply;

      /**
       * The route.
       */
      const SubRouteVector* m_srVect;

      /**
       *    The start direction of the route in terms of increasing or
       *    decreasing housenumbers.
       */
      ItemTypes::routedir_nbr_t m_startDirectionHousenumber;

      /**
       *    The start direction of the route in terms of having odd/even
       *    housenumbers to the left or right.
       */
      ItemTypes::routedir_oddeven_t m_startDirectionOddEven;

      /**
       *   The type of route expansion.
       */
      byte m_expandRouteType;

      /**
       * If uTurn.
       */
      bool m_uTurn;

      /**
       * If no concatinate.
       */
      bool m_noConcatinate;

      /**
       * The number of passed roads to include as landmarks.
       */
      uint32 m_passedRoads;

      /**
       * If true AHEAD & FOLLOW_ROAD are removed even if road name changes.
       */
      bool m_removeNameChangeAhead;

      /// If name changes should be included as waypoints. Used by Wayfinder.
      bool m_nameChangeAsWP;

      /**
       * The prefered language of names.
       */
      StringTable::languageCode m_language;

      /**
       * If abbreviate names.
       */
      bool m_abbreviate;

      /**
       * If add landmarks.
       */
      bool m_landmarks;

      /**
       *   The start offset of the routerequest.
       */
      uint16 m_startOffset;

      /**
       *   The end offset of the route request.
       */
      uint16 m_endOffset;

      /**
       *   The start direction of the route.
       */
      bool m_startDir;

      /**
       *   The end direction of the route. (Towards 1 or not ).
       */
      bool m_endDir;

      /**
       * The number of IDTransPackets.
       */
      uint32 m_nbrIDTransPackets;

      /**
       * The IDTrans reply packets.
       */
      IDTranslationReplyPacket** m_idtransReplyPackets;

      /**
       * The number of received IDTrans packets.
       */
      uint32 m_nbrReceivedIDTransPackets;

      /**
       * The top region packet.
       */
      TopRegionReplyPacket* m_topregionPacket;

      /// Vector with disturbance info to use in the expansion.
      vector<DisturbanceDescription> m_disturbanceInfo;

      /// If additional costs should be presented in the route.
      bool m_useAddCost;
};


// ========================================================================
//                                  Implementation of the inlined methods =

inline ItemTypes::routedir_nbr_t 
RouteExpander::getStartDirectionHousenumber()
{
   return m_startDirectionHousenumber;
}

inline ItemTypes::routedir_oddeven_t 
RouteExpander::getStartDirectionOddEven()
{
   return m_startDirectionOddEven;
}

#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDEDROUTEITEM_H
#define EXPANDEDROUTEITEM_H

#include "config.h"
#include "ItemTypes.h"
#include "NameCollection.h"
#include "MC2BoundingBox.h"
#include "ExpandedRouteRoadItem.h"
#include "ExpandedRouteLandmarkItem.h"
#include "ExpandStringLane.h"
#include "ExpandStringSignPost.h"
#include "StringTable.h"

#include <list>


class Packet;


/**
 * Describes a part of a route.
 *
 */
class ExpandedRouteItem {
   public:
      /**
       * The possible turns in a route.
       */
      enum routeturn_t {
         /**
          * Undefined turndescription. Only for error-handling.
          */
         UNDEFINED = 0,


         /**
          * Not really a turn.
          */
         NO_TURN,


         /**
          * Start, beginning of route.
          */
         START,


         /**
          * Start by turning the car around.
          */
         START_WITH_U_TURN,


         /**
          * Finally, end of route.
          */
         FINALLY,


         /**
          * Turn to the left.
          */
         LEFT,


         /**
          * Drive straight ahead.
          */
         AHEAD,


         /**
          * Turn to the right.
          */
         RIGHT,


         /**
          * U-turn.
          */
         U_TURN,


         /**
          * Follow the same road.
          */
         FOLLOW_ROAD,


         /**
          * Drive into a roundabout.
          */
         ENTER_ROUNDABOUT,


         /**
          * Drive out from a roundabout.
          */
         EXIT_ROUNDABOUT,


         /**
          * Drive ahead at the roundabout.
          */
         AHEAD_ROUNDABOUT,


         /**
          * Drive right at the roundabout.
          */
         RIGHT_ROUNDABOUT,


         /**
          * Drive left at the roundabout.
          */
         LEFT_ROUNDABOUT,


         /**
          * Make a u-turn at the roundabout, ei. exit in the same direction
          * as you came.
          */
         U_TURN_ROUNDABOUT,


         /**
          * Enter the highway by ramp.
          */
         ON_RAMP,


         /**
          * Exit the highway by ramp.
          */
         OFF_RAMP_LEFT,


         /**
          * Exit the highway by ramp.
          */
         OFF_RAMP_RIGHT,


         /**
          * Exit the highway by ramp.
          */
         OFF_RAMP,


         /**
          * Enter bus.
          */
         ENTER_BUS,


         /**
          * Exit bus.
          */
         EXIT_BUS,


         /**
          * Change to another bus.
          */
         CHANGE_BUS,


         /**
          * Hold to the right. Choose the right road.
          */
         KEEP_RIGHT,


         /**
          * Hold to the left. Choose the left road.
          */
         KEEP_LEFT,


         /**
          * Drive onto a ferry.
          */ 
         ENTER_FERRY,


         /**
          * Leave a ferry.
          */
         EXIT_FERRY,


         /**
          * Leave one ferry and enter an other one.
          */
         CHANGE_FERRY,


         /**
          * Park vehicle here and walk on.
          */
         PARK_AND_WALK,

         /**
          * No true turndescription. This is a waypoint indicating a
          * state change
          */
         STATE_CHANGE,

         
         /**
          * Exit non highway road by ramp.
          */ 
         OFF_MAIN, 
 
         /**
          * Enter non highway road by ramp.
          */ 
         ON_MAIN,
         
      };


      /**
       * Converts a stringCode into a routeturn_t.
       *
       * @param turn The stringCode to convert into a routeturn_t.
       * @return The routeturn_t matching the stringCode, UNDEFINED if
       *         stringCode doesn't match.
       */
      static routeturn_t routeTurnFromStringCode( StringTable::stringCode
                                                  turn );


      /**
       * Converts a routeturn_t into a stringCode, that can be used for
       * printing.
       *
       * @param turn The routeturn_t to convert into a stringCode.
       * @param transport The type of transportation. Is used for some
       *                  turns.
       * @return The stringCode matching the routeturn_t.
       */
      static StringTable::stringCode routeTurnToStringCode( 
         routeturn_t turn, 
         ItemTypes::transportation_t transport );


      /**
       * Creates a new ExpandedRouteItem.
       *
       * @param turn The type of turn.
       * @param dist The distance this ExpandedRouteItem covers.
       * @param time The time it takes to drive this ExpandedRouteItem.
       * @param intoRoadName The name(s) of the road that the turn leads
       *                     to, the ownership is transfered to
       *                     this class.
       * @param transport The method of transportation for this route
       *                  part.
       * @param lat The latitude of the turn.
       * @param lon The longitude of the turn.
       * @param turnNumber The number of turns not to take before this 
       *                   turn.
       * @param lanes The lanes of the turn.
       * @param signPosts The signs of the turn.
       * @param crossingKind The type of crossing at the turn.
       * @param turnBoundingbox The area that covers the turn.
       */
      ExpandedRouteItem( routeturn_t turn,
                         uint32 dist,
                         uint32 time,
                         NameCollection* intoRoadName,
                         ItemTypes::transportation_t transport,
                         int32 lat, int32 lon,
                         uint32 turnNumber,
                         const ExpandStringLanesCont& lanes,
                         const ExpandStringSignPosts& signPosts,
                         ItemTypes::crossingkind_t crossingKind,
                         const MC2BoundingBox& turnBoundingbox );

      /**
       * Create from packet.
       */
      ExpandedRouteItem( const Packet* p, int& pos );
      
      /**
       * Destructor.
       */
      virtual ~ExpandedRouteItem();


      /**
       * Prints content to out stream.
       *
       * @param out the ostream to print to.
       */
      void dump( ostream& out ) const;


      /**
       * Get the type of turn.
       *
       * @return The type of turn.
       */
      routeturn_t getTurnType() const;


      /**
       * Get the distance.
       *
       * @return The distance.
       */
      uint32 getDist() const;


      /**
       * Get the time.
       *
       * @return The time.
       */
      uint32 getTime() const;


      /**
       * Get the NameCollection.
       *
       * @return The NameCollection.
       */
      const NameCollection* getIntoRoadName() const;


      /**
       * Get the type of transport.
       *
       * @return The type of transport.
       */
      ItemTypes::transportation_t getTransportation() const;


      /**
       * Get the latitude.
       *
       * @return The latitude.
       */
      int32 getLat() const;


      /**
       * Get the longitude.
       *
       * @return The longitude.
       */
      int32 getLon() const;


      /**
       * Get the number of turns not to take before this turn.
       *
       * @return The number of turns before this turn.
       */
      uint32 getTurnNumber() const;


      /**
       * Get the type of crossing.
       *
       * @return The type of crossing.
       */
      ItemTypes::crossingkind_t getCrossingType() const;


      /**
       * Get the boundingbox covering the turn area.
       *
       * @return The boundingbox covering the turn area.
       */
      const MC2BoundingBox& getTurnBoundingbox() const;


      /**
       * Set the type of turn.
       *
       * @param turn The new type of turn.
       */
      void setTurnType( routeturn_t turn );


      /**
       * Set the distance.
       *
       * @param dist The new distance.
       */
      void setDist( uint32 dist );


      /**
       * Set the time.
       *
       * @param time The new time.
       */
      void setTime( uint32 time );


      /**
       * Set the NameCollection.
       *
       * @param name The new NameCollection.
       */
      void setIntoRoadName( NameCollection* name );


      /**
       * Set the type of transport.
       *
       * @param transport The new type of transport.
       */
      void setTransportation( ItemTypes::transportation_t transport );


      /**
       * Set the latitude.
       *
       * @param lat The new latitude.
       */
      void setLat( int32 lat );


      /**
       * Set the longitude.
       *
       * @param lon The new longitude.
       */
      void setLon( int32 lon );


      /**
       * Set the number of turns not to take before this turn.
       *
       * @param turnNumber The new number of turns not to take before 
       *                   this turn.
       */
      void setTurnNumber( uint32 turnNumber );


      /**
       * Set the type of crossing.
       *
       * @param crossingKind The new type of crossing.
       */
      void setCrossingType( ItemTypes::crossingkind_t crossingKind );


      /**
       * Set the boundingbox covering the turn area.
       *
       * @param turnBoundingbox The new boundingbox covering the turn area.
       */
      void setTurnBoundingbox( const MC2BoundingBox& turnBoundingbox );


      /**
       * @name ExpandedRouteRoadItems.
       * Methods to get and set the ExpandedRouteRoadItems of the item.
       */
      //@{
         /**
          * The number of ExpandedRouteRoadItems.
          *
          * @return The number of ExpandedRouteRoadItems.
          */
         uint32 getNbrExpandedRouteRoadItems() const;


         /**
          * Get the index'th ExpandedRouteRoadItem.
          *
          * @param index The index of the ExpandedRouteRoadItem to get, 
          *              0 <= index < getNbrExpandedRouteRoadItems() must
          *              be true or undefined behavoiur is the result.
          * @return The index'th ExpandedRouteRoadItem.
          */
         const ExpandedRouteRoadItem* getExpandedRouteRoadItem( 
            uint32 index ) const;


         /**
          * Add a ExpandedRouteRoadItem.
          *
          * @param item The ExpandedRouteRoadItem to add, the 
          *             ownership is transfered to this class.
          */
         void addExpandedRouteRoadItem( ExpandedRouteRoadItem* item );
      //@}


      /**
       * @name ExpandedRouteLandmarkItems.
       * Methods to get and set the ExpandedRouteLandmarkItems of the item.
       */
      //@{
         /**
          * The number of ExpandedRouteLandmarkItems.
          *
          * @return The number of ExpandedRouteLandmarkItems.
          */
         uint32 getNbrExpandedRouteLandmarkItems() const;


         /**
          * Get the index'th ExpandedRouteLandmarkItem.
          *
          * @param index The index of the ExpandedRouteLandmarkItem to get,
          *              0 <= index < getNbrExpandedRouteLandmarkItems() 
          *              must be true or undefined behavoiur is the result.
          * @return The index'th ExpandedRouteLandmarkItem.
          */
         const ExpandedRouteLandmarkItem* getExpandedRouteLandmarkItem( 
            uint32 index ) const;


         /**
          * Add a ExpandedRouteLandmarkItem.
          *
          * @param item The ExpandedRouteLandmarkItem to add, the 
          *             ownership is transfered to this class.
          */
         void addExpandedRouteLandmarkItem( 
            ExpandedRouteLandmarkItem* item );
      //@}

      /**
       * Get the lanes.
       */
      const ExpandStringLanesCont& getLanes() const;
         
      /**
       * Get the signposts.
       */
      const ExpandStringSignPosts& getSignPosts() const;
         
      /**
       *  Add a possible turn at this location.
       *  @return True if it was possible to add more turns.
       */
      bool addPossibleTurn(routeturn_t possTurn);
      
      /**
       * The number of turns that was possible BUT NOT made at this location.
       */      
      uint32 getNbrPossibleTurns() const;

      /**
       *  Get one possible turn at this location with the given index.
       *  @return NO_TURN if the index was wrong ( eg. 2nd possible turn in
       *          a 3-way crossing), UNDEFINED if no possible turn was set at
       *          this index.
       */
      routeturn_t getPossibleTurn(uint32 index) const;

      void setEndOfRoad(bool isEndOfRoad)
         { m_isEndOfRoad = isEndOfRoad;}

      bool isEndOfRoad() const
         { return m_isEndOfRoad;}
         
      /**
       *    Write this ExpandedRouteItem  as a byte buffer.
       *    @param p    The packet where the ExpandedRouteItem will be saved.
       *    @param pos  Position in p, will be updated when calling this 
       *                method.
       */
      uint32 save( Packet* p, int& pos ) const;

      /**
       *    Get the size of this item when stored in a byte buffer.
       *    @return  The number of bytes this item will use when stored 
       *             in a byte buffer.
       */
      uint32 getSizeAsBytes() const;

      /**
       *    Set all members with data from the buffer.
       *    @param p    The packet where the ExpandedRouteItem will be loaded 
       *                from.
       *    @param pos  Position in p, will be updated when calling this 
       *                method.
       */
      void load( const Packet* p, int& pos );


   private:
      /**
       * The type of turn.
       */
      routeturn_t m_turn;

      /**
       * The other turns that was possible BUT NOT made at this location.
       */
      routeturn_t m_possTurns[7];

      /**
       * The number of turns that was possible BUT NOT made at this location.
       */
      uint32 m_nbrPossTurns;
      
      /**
       * The distance.
       */
      uint32 m_dist;


      /**
       * The time.
       */
      uint32 m_time;


      /**
       * The name(s) of the road that this turn leads to.
       */
      NameCollection* m_intoRoadName;


      /**
       * The method of transportation.
       */
      ItemTypes::transportation_t m_transport;


      /**
       * The latitude of the turn.
       */
      int32 m_lat;


      /**
       * The longitude of the turn.
       */
      int32 m_lon;


      /**
       * The number of turns not to take before this turn.
       */
      uint32 m_turnNumber;

      
      /**
       * If the turn (right or left) is at the end of the road.
       */
      bool m_isEndOfRoad;


      /**
       * The type of crossing at the turn.
       */
      ItemTypes::crossingkind_t m_crossingKind;


      /**
       * The boundingbox that covers the turn.
       */
      MC2BoundingBox m_turnBoundingbox;


      /**
       * The ExpandedRouteRoadItems.
       */
      ExpandedRouteRoadVector m_roadItems;


      /**
       * The ExpandedRouteLandmarkItems.
       */
      ExpandedRouteLandmarkVector m_landmarkItems;

      /**
       * The ExpandStringLanesCont.
       */
      ExpandStringLanesCont m_lanes;

      /**
       * The ExpandStringSignPosts.
       */
      ExpandStringSignPosts m_signPosts;
};



/**
 *    
 */
class ExpandedRouteItemsList : public list<ExpandedRouteItem*> {
   public:
      /**
       *    Delete the list and all elements in it (no matter where 
       *    they are allocated).
       */
      ~ExpandedRouteItemsList();

      /**
       *   Store the list in a packet.
       *   @param packet The packet to store this list in
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last ExpandedRouteItem when 
       *                 returning.
       */
      void store( Packet* packet, int& pos ) const;

      /**
       *   Restore a list from a packet.
       *   @param packet The packet to restore this list from.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last ExpandedRouteItem when 
       *                 returning.
       */
      void restore( const Packet* packet, int& pos );

      /**
       *    Add an already created ExpandedRouteItem to this list. The 
       *    added element will be deleted when the list is deleted!
       */
      void addExpandedRouteItem( ExpandedRouteItem* elm );

   private:
      /**
       *    Get the size of the elements in this list when stored in 
       *    a byte-buffer.
       */
      uint32 getSizeAsBytes() const;

};


// =======================================================================
//                                     Implementation of inlined methods =


inline const ExpandStringLanesCont&
ExpandedRouteItem::getLanes() const {
   return m_lanes;
}
 
inline const ExpandStringSignPosts&
ExpandedRouteItem::getSignPosts() const {
   return m_signPosts;
}
 

#endif // ExpandedRouteItem_H

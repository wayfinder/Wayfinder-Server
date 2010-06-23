/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDEDROUTEROADITEM_H
#define EXPANDEDROUTEROADITEM_H

#include "config.h"
#include "ItemTypes.h"
#include "MC2Coordinate.h"

#include<vector>

class Packet;
class NameCollection;

/**
 *    Describes a road part of a route.
 *
 */
class ExpandedRouteRoadItem {
   public:
      /**
       * Creates a new ExpandedRouteRoadItem.
       *
       * @param mapID The map id of the road.
       * @param nodeID The node id of the road.
       * @param roadClass The class of the road, from major road to
       *                  fourth class road.
       * @param roadName   The name(s) of the road, the ownership is 
       *                   transfered to this class.
       * @param posSpeedLimit The speed limit in positive direction
       *                      (first -> last coordinate). 
       *                      MAX_UINT32 is unknown speedlimit.
       * @param negSpeedLimit The speed limit in negative direction
       *                      (last -> first coordinate).
       *                      MAX_UINT32 is unknown speedlimit.
       * @param multidigitialized True if this road is multidigitalized,
       *        false otherwise.
       * @param ramp True if this road is a ramp, false otherwise.
       * @param roundabout True if this road is part of a roundabout, 
       *        false otherwise.
       * @param controlledAccess True if road access ways are controlled.
       * @param turn True if this road is the turn.
       * @param driveOnRightSide If to drive on the right side of the road.
       * @param startLevel The level, relative ground, of the first 
       *                   coordinate.
       * @param endLevel The level, relative ground, of the last 
       *                 coordinate.
       * @param posEntryrestr The entry restrictions when entering the road
       *                      at first ccordinate.
       * @param negEntryrestr The entry restrictions when entering the road
       *                      at last coordinate.
       */
      ExpandedRouteRoadItem( uint32 mapID,
                             uint32 nodeID,
                             ItemTypes::roadClass roadClass,
                             NameCollection* roadName,
                             uint32 posSpeedLimit = 0, 
                             uint32 negSpeedLimit = 0, 
                             bool multidigitialized = false,
                             bool ramp = false, bool roundabout = false,
                             bool controlledAccess = false,
                             bool turn = false,
                             bool driveOnRightSide = true,
                             int8 startLevel = 0, int8 endLevel = 0,
                             ItemTypes::entryrestriction_t posEntryrestr
                             = ItemTypes::noRestrictions,
                             ItemTypes::entryrestriction_t negEntryrestr
                             = ItemTypes::noRestrictions );


      /**
       *  Create a new Item from a packet.
       */
      ExpandedRouteRoadItem( const Packet* p, int &pos ); 
      
      /**
       * Destructor.
       */
      virtual ~ExpandedRouteRoadItem();


      /**
       * Prints content to out stream.
       *
       * @param out the ostream to print to.
       */
      void dump( ostream& out ) const;


      /**
       * Get the mapID.
       *
       * @return The mapID.
       */
      uint32 getMapID() const;


      /**
       * Set the mapID.
       *
       * @param speed The new mapID.
       */
      void setMapID( uint32 mapID );


      /**
       * Get the nodeID.
       *
       * @return The nodeID.
       */
      uint32 getNodeID() const;


      /**
       * Set the nodeID.
       *
       * @param speed The new nodeID.
       */
      void setNodeID( uint32 nodeID );


      /**
       * Get the road class.
       *
       * @return The road class.
       */
      ItemTypes::roadClass getRoadClass() const;


      /**
       * Set the road class.
       *
       * @param roadClass The new road class.
       */
      void setRoadClass( ItemTypes::roadClass roadClass );


      /**
       * Get the name(s) of the road.
       *
       * @return The name(s) of the road.
       */
      const NameCollection* getRoadName() const;


      /**
       * Set the name(s) of the road.
       *
       * @param roadName The new name(s) of the road, the ownership is 
       *                 transfered to this class.
       */
      void setRoadName( NameCollection* roadName );


      /**
       * Get the positive speed limit.
       *
       * @return The positive speed limit.
       */
      uint32 getPosSpeedLimit() const;


      /**
       * Set the positive speed limit.
       *
       * @param speed The new positive speed limit.
       */
      void setPosSpeedLimit( uint32 speed );


      /**
       * Get the negative speed limit.
       *
       * @return The negative speed limit.
       */
      uint32 getNegSpeedLimit() const;


      /**
       * Set the negative speed limit.
       *
       * @param speed The new negative speed limit.
       */
      void setNegSpeedLimit( uint32 speed );


      /**
       * Get if road is multi digitialized.
       *
       * @return If road is multi digitialized.
       */
      bool getMultidigitialized() const;


      /**
       * Set if road is multi digitialized.
       *
       * @param multidigitialized New value of multidigitialized.
       */
      void setMultidigitialized( bool multidigitialized );


      /**
       * Get if road is a ramp.
       *
       * @return If road is a ramp.
       */
      bool getRamp() const;


      /**
       * Set if road is a ramp.
       *
       * @param ramp New value of ramp.
       */
      void setRamp( bool ramp );


      /**
       * Get if road is a roundabout.
       *
       * @return If road is a roundabout.
       */
      bool getRoundabout() const;


      /**
       * Set if road is a roundabout.
       *
       * @param roundabout New value of roundabout.
       */
      void setRoundabout( bool roundabout );


      /**
       * Get if road is a controlledAccess.
       *
       * @return If road is a controlledAccess.
       */
      bool getControlledAccess() const;


      /**
       * Set if road is a controlledAccess.
       *
       * @param controlledAccess New value of controlledAccess.
       */
      void setControlledAccess( bool controlledAccess );


      /**
       * Get if road is a the turn.
       *
       * @return If road is the turn.
       */
      bool getTurn() const;


      /**
       * Set if road is the turn.
       *
       * @param turn New value of turn.
       */
      void setTurn( bool turn );


      /**
       * Get if to drive on the right side of the road.
       *
       * @return If to drive on the right side of the road.
       */
      bool getDriveOnRightSide() const;


      /**
       * Set if to drive on the right side of the road.
       *
       * @param turn New value of driveOnRightSide.
       */
      void setDriveOnRightSide( bool driveOnRightSide );


      /**
       * Get road start level.
       *
       * @return Road start level.
       */
      int8 getStartLevel() const;


      /**
       * Set road start level
       *
       * @param level The new start level.
       */
      void setStartLevel( int8 level );


      /**
       * Get road end level.
       *
       * @return Road end level.
       */
      int8 getEndLevel() const;


      /**
       * Set road end level
       *
       * @param level The new end level.
       */
      void setEndLevel( int8 level );


      /**
       * Get the positive entry restrictions.
       *
       * @return The positive entry restrictions.
       */
      ItemTypes::entryrestriction_t getPosEntryrestrictions() const;


      /**
       * Set the positive entry restrictions.
       *
       * @param entryrestr The new positive entry restrictions.
       */
      void setPosEntryrestrictions( 
         ItemTypes::entryrestriction_t entryrestr );


      /**
       * Get the negative entry restrictions.
       *
       * @return The negative entry restrictions.
       */
      ItemTypes::entryrestriction_t getNegEntryrestrictions() const;


      /**
       * Set the negative entry restrictions.
       *
       * @param entryrestr The new negative entry restrictions.
       */
      void setNegEntryrestrictions( 
         ItemTypes::entryrestriction_t entryrestr );


      /**
       * @name Coordinates.
       * Methods to get and set the coordinates of the road.
       */
      //@{
         /**
          * The number of coordinates.
          *
          * @return The number of coordinates.
          */
         uint32 getNbrCoordinates() const;


         /**
          * Get the index'th coordinate.
          *
          * @param index The index of the coordinate to get, 
          *              0 <= index < getNbrCoordinates() must be true or
          *              undefined behavoiur is the result.
          * @return The index'th coordinate.
          */
         const MC2Coordinate getCoordinate( uint32 index ) const;

         /// Get the start of the coordinate iterator.
         vector<MC2Coordinate>::const_iterator coordsBegin() const;
         
         /// Get the end of the coordinate iterator.
         vector<MC2Coordinate>::const_iterator coordsEnd() const;

         /**
          * Add a coordinate.
          *
          * @param coord The coordinate to add.
          */
         void addCoordinate( const MC2Coordinate coord );
      //@}

         /**
          * @return Size in packet. 
          */
         uint32 getSizeAsBytes();

         /**
          * Load from packet.
          */
         void load( const Packet* p, int& pos );

         /**
          * Save to packet.
          */
         uint32 save( Packet* p, int& pos ) const;

   private:
      /**
       * The map id.
       */
      uint32 m_mapID;


      /**
       * The node id.
       */
      uint32 m_nodeID;


      /**
       * The class of the road.
       */
      ItemTypes::roadClass m_roadClass;


      /**
       * The name(s) of the road.
       */
      NameCollection* m_roadName;


      /**
       * The speedlimit in the positive direction, first to last.
       */
      uint32 m_posSpeedLimit;


      /**
       * The speedlimit in the negative direction, last to first.
       */
      uint32 m_negSpeedLimit;


      /**
       * If multidigitialized.
       */
      bool m_multidigitialized;


      /**
       * If ramp.
       */
      bool m_ramp;


      /**
       * If roundabout.
       */
      bool m_roundabout;


      /**
       * If controlledAccess.
       */
      bool m_controlledAccess;


      /**
       * If turn.
       */
      bool m_turn;


      /**
       * If to drive on the right side of the road.
       */
      bool m_driveOnRightSide;


      /**
       * The level relative ground at first coordinate.
       */
      int8 m_startLevel;


      /**
       * The level relative ground at last coordinate.
       */
      int8 m_endLevel;


      /**
       * The entryrestr at first coordinate.
       */
      ItemTypes::entryrestriction_t m_posEntryrestr;


      /**
       * The entryrestr at last coordinate.
       */
      ItemTypes::entryrestriction_t m_negEntryrestr;


      /**
       * The coordinates.
       */
      vector<MC2Coordinate> m_coords;
};

/**
 *    
 */
class ExpandedRouteRoadVector : public vector<ExpandedRouteRoadItem*>
{
   public:
      /**
       *    Delete the vector and all elements in it (no matter where 
       *    they are allocated).
       */
      ~ExpandedRouteRoadVector();

      /**
       *   Store the list in a packet.
       *   @param packet The packet to store this vector in.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last ExpandedRouteRoadItem 
       *                 when returning.
       */
      void store( Packet* packet, int& pos ) const;

      /**
       *   Restore a vector from a packet.
       *   @param packet The packet to restore this lvector from.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last ExpandedRouteRoadItem 
       *                 when returning.
       */
      void restore( const Packet* packet, int& pos );

      /**
       *    Add an already created ExpandedRouteRoadItem to this list. 
       *    The added element will be deleted when the vector is deleted!
       */
      void addExpandedRouteRoad( ExpandedRouteRoadItem* item );

      /**
       *    Get the size of the elements in this vector when stored in 
       *    a byte-buffer.
       */
      uint32 getSizeAsBytes() const;

};


#endif // EXPANDEDROUTEROADITEM_H


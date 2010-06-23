/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDSTRINGLANE_H
#define EXPANDSTRINGLANE_H

#include "config.h"
#include <vector>

class Packet;


/**
 * Class representing a lane.
 *
 */
class ExpandStringLane {
   public:
      /**
       * The directions of a lane, a lane can have more than one direction.
       */
      enum laneDirection {
         /// no direction indicated
         no_dir = 0,
         /// sharp left (135 degrees)
         sharp_left = (1<<1),
         /// left (90 degrees)
         left = (1<<2),
         /// half left (45 degrees)
         half_left = (1<<3),
         /// ahead  
         ahead = (1<<4),
         /// half right (45 degrees)
         half_right = (1<<5),
         /// right (90 degrees)
         right = (1<<6),
         /// sharp right (135 degrees)
         sharp_right = (1<<7),
         /// U-Turn Left (180 degrees)
         uturn_left = (1<<8),
         /// U-Turn Right (180 degrees)
         uturn_right = (1<<9),

         /**
          * If lane is in the opposite direction.
          */
         opposite_direction = (1<<13),

         /// preferred lane, not a directon just stored internally.
         preferred_lane = (1<<14),
         /**
          * not allowed to drive in with car, not a directon just stored
          * internally.
          */
         not_car_lane = (1<<15),
      };

      /**
       * Create a new ExpandStringLane from values.
       *
       * @param directions Bits from laneDirection enum set.
       */
      ExpandStringLane( uint16 directions, bool preferredLane,
                        bool notCarLane );

      /**
       * Create a new ExpandStringLane to be called load on later.
       */
      ExpandStringLane();

      /**
       * Get the directions of this lane.
       */
      uint16 getDirections() const;


      /**
       * Get the "best" direction of this lane.
       */
      laneDirection getSingleDirection() const;

      /**
       * Get the "best" direction of this lane.
       */
      static laneDirection getSingleDirection( uint16 dir );

      /**
       * Get the "best" direction of this lane.
       *
       * @param turn The stringCode of the turn, like LEFT_TURN.
       */
      laneDirection getSingleDirectionUsingTurn( uint32 turn ) const;

      /**
       * Get if is preferred lane.
       */
      bool getPreferred() const;

      /**
       * Get if is not car lane.
       */
      bool getNotCar() const;

      /**
       * Load from packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Save to packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * @return Size in packet. 
       */
      static uint32 getSizeInPacket();

      /**
       * Prints lane to out.
       */
      void dump( ostream& out ) const;

      /**
       * laneDirection to string.
       */
      static const char* laneDirectionToString( laneDirection d );

      /**
       * Stream output.
       */
      inline friend ostream& operator << ( 
         ostream& os, const ExpandStringLane& s )
      {
         s.dump( os );
         return os;
      }

   private:
      /**
       * The directions and bools.
       */
      uint16 m_dir;
};


/**
 * Class holding lanes.
 *
 */
class ExpandStringLanes : public vector< ExpandStringLane > {
   public:
      /**
       * New vector of lanes.
       */
      ExpandStringLanes( bool stopOfLanes = false );

      /**
       * Load from packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Save to packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * @return Size in packet. 
       */
      uint32 getSizeInPacket() const;

      /**
       * The distance for the lanes. Distance into this segment.
       */
      uint32 getDist() const;

      /**
       * Set the distance for the lanes.
       */
      void setDist( uint32 d );

      /**
       * Add a lane to this.
       */
      void addLane( const ExpandStringLane& l );

      /**
       * Get if lanes stops here.
       */
      bool getStopOfLanes() const;

      /**
       * Prints lanes to out.
       */
      void dump( ostream& out ) const;

      /**
       * Stream output.
       */
      inline friend ostream& operator << ( 
         ostream& os, const ExpandStringLanes& s )
      {
         s.dump( os );
         return os;
      }

   private:
      /**
       * The distance. Distance from last turn.
       */
      uint32 m_dist;

      /**
       * Stop of lanes.
       */
      bool m_stopOfLanes;
};


/**
 * Holds a number of lane groups.
 *
 */
class ExpandStringLanesCont : public vector<ExpandStringLanes> {
   public:
      /**
       * Load from packet.
       */
      void load( const Packet* p, int& pos );

      /**
       * Save to packet.
       */
      void save( Packet* p, int& pos ) const;

      /**
       * @return Size in packet. 
       */
      uint32 getSizeInPacket() const;

      /**
       * Add a lane to this.
       */
      void addLaneGroup( const ExpandStringLanes& l );

      /**
       * Add a lane first into this.
       */
      void addLaneGroupFirst( const ExpandStringLanes& l );
};


// =======================================================================
//                                     Implementation of inlined methods =


inline 
ExpandStringLane::ExpandStringLane( uint16 directions, bool preferredLane,
                                    bool notCarLane )
{
   m_dir = directions;
   if ( preferredLane ) {
      m_dir |= preferred_lane;
   }
   if ( notCarLane ) {
      m_dir |= not_car_lane;
   }
}

inline 
ExpandStringLane::ExpandStringLane() {
   m_dir = 0;
}

inline uint16
ExpandStringLane::getDirections() const {
   return m_dir; // Remove preferred and no car bits? Not absolutely needed.
}

inline bool
ExpandStringLane::getPreferred() const {
   return (m_dir & preferred_lane );
}

inline bool
ExpandStringLane::getNotCar() const {
   return (m_dir & not_car_lane );
}

inline uint32
ExpandStringLane::getSizeInPacket() {
   return 2/*Each lane is an uint16*/;
}


inline
ExpandStringLanes::ExpandStringLanes( bool stopOfLanes ) 
      : m_dist( 0 ), m_stopOfLanes( stopOfLanes )
{
}

inline uint32
ExpandStringLanes::getSizeInPacket() const {
   return 2*4/*dist+nbrlanes*/ + 
      size() * ExpandStringLane::getSizeInPacket() + 3/*Possible padding*/;
}

inline uint32
ExpandStringLanes::getDist() const {
   return m_dist;
}

inline void
ExpandStringLanes::setDist( uint32 d ) {
   m_dist = d;
}

inline void
ExpandStringLanes::addLane( const ExpandStringLane& l ) {
   push_back( l );
}

inline bool
ExpandStringLanes::getStopOfLanes() const {
   return m_stopOfLanes;
}

inline void
ExpandStringLanesCont::addLaneGroup( const ExpandStringLanes& l ) {
   push_back( l );
}

inline void
ExpandStringLanesCont::addLaneGroupFirst( const ExpandStringLanes& l ) {
   insert( begin(), l );
}

#endif // EXPANDSTRINGLANE_H


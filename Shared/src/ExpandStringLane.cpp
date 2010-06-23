/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandStringLane.h"
#include "Packet.h"

#include "StringTable.h"

#include <map>

void
ExpandStringLane::load( const Packet* p, int& pos ) {
   m_dir = p->incReadShort( pos );
}

void
ExpandStringLane::save( Packet* p, int& pos ) const {
   p->incWriteShort( pos, m_dir );
}


ExpandStringLane::laneDirection
ExpandStringLane::getSingleDirection( uint16 dir ) {
   // Get one of five directions (or no_dir of no directions on lane)
   laneDirection sdir = no_dir;

   if ( (dir & sharp_left) || (dir & left) || 
        (dir & uturn_left) ) {
      sdir = left;
   } else if ( (dir & half_left) ) {
      sdir = half_left;
   } else if ( (dir & sharp_right) || (dir & right) || 
               (dir & uturn_right) ) {
      dir = right;
   } else if ( (dir & half_right) ) {
      sdir = half_right;
   }

   // Last check for ahead
   if ( sdir == no_dir ) {
      if ( (dir & ahead ) ) {
         sdir = ahead;
      }
   }

   return sdir;
}


ExpandStringLane::laneDirection
ExpandStringLane::getSingleDirection() const {
   return getSingleDirection( m_dir );
}


void
ExpandStringLane::dump( ostream& out ) const {
   bool first = true;
   out << "(";
   for ( int i = 0 ; i < 16 ; ++i ) {
      if ( BitUtility::getBit( m_dir, i ) ) {
         if ( !first ) {
            out << "|";
         } else {
            first = false;
         }
         out << laneDirectionToString( laneDirection( 1<<i ) );
      }
   }
   out << ")";
}

const char*
ExpandStringLane::laneDirectionToString( laneDirection d ) {
   switch ( d ) {
      case no_dir :
         return "no_dir";
      case sharp_left :
         return "sharp_left";
      case left :
         return "left";
      case half_left :
         return "half_left";
      case ahead :
         return "ahead";
      case half_right :
         return "half_right";
      case right :
         return "right";
      case sharp_right :
         return "sharp_right";
      case uturn_left :
         return "uturn_left";
      case uturn_right :
         return "uturn_right";
      case opposite_direction :
         return "opposite_direction";
      case preferred_lane :
         return "preferred_lane";
      case not_car_lane :
         return "not_car_lane";
   }
   return "unknown";
}


typedef std::map< StringTable::stringCode, ExpandStringLane::laneDirection > 
   LaneDirectionFromTurnMap;

LaneDirectionFromTurnMap initLaneDirectionFromTurnMap() {
   LaneDirectionFromTurnMap m;

   m.insert( make_pair( StringTable::LEFT_TURN, 
                        ExpandStringLane::left ) );
   m.insert( make_pair( StringTable::KEEP_LEFT, 
                        ExpandStringLane::half_left ) );
   m.insert( make_pair( StringTable::RIGHT_TURN, 
                        ExpandStringLane::right ) );
   m.insert( make_pair( StringTable::KEEP_RIGHT, 
                        ExpandStringLane::half_right ) );

   return m;
}

static LaneDirectionFromTurnMap laneDirectionFromTurnMap = 
   initLaneDirectionFromTurnMap();

ExpandStringLane::laneDirection
ExpandStringLane::getSingleDirectionUsingTurn( uint32 turn ) const {
   typedef ExpandStringLane EL;
   uint16 dirMask = MAX_UINT16;
   
   // Allow only left direction for left turn, dito for right
   if ( turn == StringTable::LEFT_TURN || turn == StringTable::KEEP_LEFT ) {
      dirMask = EL::sharp_left | EL::left | EL::half_left | EL::uturn_left;
   } else if ( turn == StringTable::RIGHT_TURN || 
               turn == StringTable::KEEP_RIGHT ) {
      dirMask = EL::sharp_right | EL::right | EL::half_right | EL::uturn_right;
   }

   laneDirection sdir = getSingleDirection( m_dir & dirMask );

   // If no dir try to set from turn
   if ( sdir == no_dir ) {
      LaneDirectionFromTurnMap::const_iterator findIt = 
         laneDirectionFromTurnMap.find( StringTable::stringCode( turn ) );
      if ( findIt != laneDirectionFromTurnMap.end() ) {
         sdir = findIt->second;
      }
   }

   return sdir;
}

void
ExpandStringLanes::load( const Packet* p, int& pos ) {
   m_dist = p->incReadLong( pos );
   uint32 nbr = p->incReadLong( pos );
   m_stopOfLanes = p->incReadLong( pos ) != 0;
   resize( nbr );
   for ( uint32 i = 0 ; i < nbr ; ++i ) {
      (*this)[ i ].load( p, pos );
   }
}

void
ExpandStringLanes::save( Packet* p, int& pos ) const {
   p->incWriteLong( pos, m_dist );
   p->incWriteLong( pos, size() );
   p->incWriteLong( pos, m_stopOfLanes );
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      (*this)[ i ].save( p, pos );
   }
}

void
ExpandStringLanes::dump( ostream& out ) const {
   bool first = true;
   out << "Lanes: ";
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      if ( !first ) {
         out << ", ";
      } else {
         first = false;
      }
      (*this)[ i ].dump( out );
   }
}

void
ExpandStringLanesCont::load( const Packet* p, int& pos ) {
   uint32 nbr = p->incReadLong( pos );
   resize( nbr );
   for ( uint32 i = 0 ; i < nbr ; ++i ) {
      (*this)[ i ].load( p, pos );
   }
}

void
ExpandStringLanesCont::save( Packet* p, int& pos ) const {
   p->incWriteLong( pos, size() );
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      (*this)[ i ].save( p, pos );
   }
}

uint32
ExpandStringLanesCont::getSizeInPacket() const {
   uint32 bytes = 0;
   for ( uint32 i = 0 ; i < size() ; ++i ) {
      bytes += (*this)[ i ].getSizeInPacket();
   }
   return bytes;
}

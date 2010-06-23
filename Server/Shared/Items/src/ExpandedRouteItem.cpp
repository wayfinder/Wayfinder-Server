/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandedRouteItem.h"
#include "StringUtility.h"
#include "Packet.h"


ExpandedRouteItem::routeturn_t 
ExpandedRouteItem::routeTurnFromStringCode( StringTable::stringCode
                                            turn )
{
   switch ( turn ) {
      case StringTable::UNDEFINED_TURN :
         return UNDEFINED;
      case StringTable::DRIVE_START :
         return START;
      case StringTable:: DRIVE_START_WITH_UTURN:
         return START_WITH_U_TURN;
      case StringTable::DRIVE_FINALLY :
         return FINALLY;
      case StringTable::LEFT_TURN :
         return LEFT;
      case StringTable::AHEAD_TURN :
         return AHEAD;
      case StringTable::RIGHT_TURN :
         return RIGHT;
      case StringTable::U_TURN :
         return U_TURN;
      case StringTable::FOLLOWROAD_TURN :
         return FOLLOW_ROAD;
      case StringTable::ENTER_ROUNDABOUT_TURN :
         return ENTER_ROUNDABOUT;
      case StringTable:: EXIT_ROUNDABOUT_TURN_WALK:
         return EXIT_ROUNDABOUT;
      case StringTable::EXIT_ROUNDABOUT_TURN :
         return EXIT_ROUNDABOUT;
      case StringTable::AHEAD_ROUNDABOUT_TURN :
         return AHEAD_ROUNDABOUT;
      case StringTable::RIGHT_ROUNDABOUT_TURN :
         return RIGHT_ROUNDABOUT;
      case StringTable::LEFT_ROUNDABOUT_TURN :
         return LEFT_ROUNDABOUT;
      case StringTable:: U_TURN_ROUNDABOUT_TURN:
         return U_TURN_ROUNDABOUT;
      case StringTable::ON_RAMP_TURN :
         return ON_RAMP;
      case StringTable::OFF_RAMP_TURN :
         return OFF_RAMP;
      case StringTable::LEFT_OFF_RAMP_TURN :
         return OFF_RAMP_LEFT;
      case StringTable::RIGHT_OFF_RAMP_TURN :
         return OFF_RAMP_RIGHT;
      case StringTable::ENTER_BUS_TURN :
         return ENTER_BUS;
      case StringTable::EXIT_BUS_TURN :
         return EXIT_BUS;
      case StringTable::CHANGE_BUS_TURN :
         return CHANGE_BUS;
      case StringTable::KEEP_RIGHT :
         return KEEP_RIGHT;
      case StringTable::KEEP_LEFT :
         return KEEP_LEFT;
      case StringTable::ENTER_FERRY_TURN :
         return ENTER_FERRY;
      case StringTable::EXIT_FERRY_TURN :
         return EXIT_FERRY;
      case StringTable::CHANGE_FERRY_TURN :
         return CHANGE_FERRY;
      case StringTable::PARK_CAR :
         return PARK_AND_WALK;
      case StringTable::PARK_BIKE :
         return PARK_AND_WALK;
      case StringTable::LMLOCATION_ROADNAMECHANGE :
         return STATE_CHANGE;
      case StringTable::OFF_MAIN_TURN :
         return OFF_MAIN;
      case StringTable::ON_MAIN_TURN :
         return ON_MAIN;
      default:
         mc2log << error << "ExpandedRouteItem::routeTurnFromStringCode "
                << "unknown stringCode " << int(turn) << "\""
                << StringTable::getString( turn, StringTable::ENGLISH )
                << "\"" << endl;
         return UNDEFINED;
   }
}


StringTable::stringCode 
ExpandedRouteItem::routeTurnToStringCode( 
   routeturn_t turn, 
   ItemTypes::transportation_t transport )
{
   switch ( turn ) {
      case UNDEFINED :
         return StringTable::UNDEFINED_TURN;
      case NO_TURN :
         return StringTable::UNDEFINED_TURN;
      case START :
         return StringTable::DRIVE_START;
      case START_WITH_U_TURN :
         return StringTable::DRIVE_START_WITH_UTURN;
      case FINALLY :
         return StringTable::DRIVE_FINALLY;
      case LEFT :
         return StringTable::LEFT_TURN;
      case AHEAD :
         return StringTable::AHEAD_TURN;
      case RIGHT :
         return StringTable::RIGHT_TURN;
      case U_TURN :
         return StringTable::U_TURN;
      case FOLLOW_ROAD :
         return StringTable::FOLLOWROAD_TURN;
      case ENTER_ROUNDABOUT :
         return StringTable::ENTER_ROUNDABOUT_TURN;
      case EXIT_ROUNDABOUT :
         if ( transport == ItemTypes::walk ) {
            return StringTable::EXIT_ROUNDABOUT_TURN_WALK;
         } else { // Default
            return StringTable::EXIT_ROUNDABOUT_TURN;
         }
      case AHEAD_ROUNDABOUT :
         return StringTable::AHEAD_ROUNDABOUT_TURN;
      case RIGHT_ROUNDABOUT :
         return StringTable::RIGHT_ROUNDABOUT_TURN;
      case LEFT_ROUNDABOUT :
         return StringTable::LEFT_ROUNDABOUT_TURN;
      case U_TURN_ROUNDABOUT :
         return StringTable::U_TURN_ROUNDABOUT_TURN;
      case ON_RAMP :
         return StringTable::ON_RAMP_TURN;
      case OFF_RAMP :
         return StringTable::OFF_RAMP_TURN;
      case OFF_RAMP_LEFT :
         return StringTable::LEFT_OFF_RAMP_TURN;
      case OFF_RAMP_RIGHT :
         return StringTable::RIGHT_OFF_RAMP_TURN;
      case ENTER_BUS :
         return StringTable::ENTER_BUS_TURN;
      case EXIT_BUS :
         return StringTable::EXIT_BUS_TURN;
      case CHANGE_BUS :
         return StringTable::CHANGE_BUS_TURN;
      case KEEP_RIGHT :
         return StringTable::KEEP_RIGHT;
      case KEEP_LEFT :
         return StringTable::KEEP_LEFT;
      case ENTER_FERRY :
         return StringTable::ENTER_FERRY_TURN;
      case EXIT_FERRY :
         return StringTable::EXIT_FERRY_TURN;
      case CHANGE_FERRY :
         return StringTable::CHANGE_FERRY_TURN;
      case PARK_AND_WALK :
         if ( transport == ItemTypes::drive ) {
            return StringTable::PARK_CAR;
         } else if ( transport == ItemTypes::bike ) {
            return StringTable::PARK_BIKE;
         } else { // Default
            return StringTable::PARK_CAR;
         }
       case STATE_CHANGE :
         return StringTable::LMLOCATION_ROADNAMECHANGE;
       case OFF_MAIN :
         return StringTable::OFF_MAIN_TURN; 
       case ON_MAIN : 
         return StringTable::ON_MAIN_TURN;  
   }

   // Unreachabe code
   return StringTable::UNDEFINED_TURN;
}



ExpandedRouteItem::ExpandedRouteItem( 
   routeturn_t turn,
   uint32 dist,
   uint32 time,
   NameCollection* intoRoadName,
   ItemTypes::transportation_t transport,
   int32 lat, int32 lon,
   uint32 turnNumber,
   const ExpandStringLanesCont& lanes,
   const ExpandStringSignPosts& signPosts,
   ItemTypes::crossingkind_t crossingKind,
   const MC2BoundingBox& turnBoundingbox )
      : m_turn( turn ),
        m_dist( dist ),
        m_time( time ),
        m_intoRoadName( intoRoadName ),
        m_transport( transport ),
        m_lat( lat ),
        m_lon( lon ),
        m_turnNumber( turnNumber ),
        m_crossingKind( crossingKind ),
        m_turnBoundingbox( turnBoundingbox ),
        m_lanes( lanes ),
        m_signPosts( signPosts )
{
   MC2_ASSERT( intoRoadName != NULL );
   m_nbrPossTurns = 0;
   m_isEndOfRoad  = false;
}

ExpandedRouteItem::ExpandedRouteItem( const Packet* p, int& pos ) {
   load( p, pos );
}

ExpandedRouteItem::~ExpandedRouteItem() {
   delete m_intoRoadName;
   for ( uint32 i = 0 ; i < m_roadItems.size() ; ++i ) {
      delete m_roadItems[ i ];
   }
   // You know that the destructor for the vector also deletes the
   // contents?
   m_roadItems.clear();
   for ( uint32 i = 0 ; i < m_landmarkItems.size() ; ++i ) {
      delete m_landmarkItems[ i ];
   }
   // You know that the destructor for the vector also deletes the
   // contents?
   m_landmarkItems.clear(); 
}


void 
ExpandedRouteItem::dump( ostream& out ) const {
   out << "## turn " << StringTable::getString( 
      routeTurnToStringCode( m_turn, m_transport ), 
      StringTable::ENGLISH ) << endl
       << "## dist " << m_dist << endl
       << "## time " << m_time << endl
       << "## intoRoadName " << *m_intoRoadName << endl
       << "## transport " << StringTable::getString( 
          ItemTypes::getVehicleSC( ItemTypes::transportation2Vehicle( 
             m_transport ) ), StringTable::ENGLISH ) << endl
       << "## lat " << m_lat << endl
       << "## lon " << m_lon << endl
       << "## turnNumber " << m_turnNumber << endl
       << "## crossingKind " << StringTable::getString( 
          ItemTypes::getCrossingKindSC( m_crossingKind ), 
          StringTable::ENGLISH ) << endl
       << "## turnBoundingbox " << "MaxLat=" 
       << m_turnBoundingbox.getMaxLat() << ", MinLon="
       << m_turnBoundingbox.getMinLon()
       << ", MinLat=" << m_turnBoundingbox.getMinLat() << ", MaxLon=" 
       << m_turnBoundingbox.getMaxLon() << endl
       << "## nbr road items " << m_roadItems.size() << endl
       << "## nbr landmark items " << m_landmarkItems.size() 
       << "## nbr lane groups " << m_lanes.size() << endl
       << "## nbr signposts " << m_signPosts.size() << endl;
   for ( uint32 i = 0 ; i < m_roadItems.size() ; ++i ) {
      out << "## road item " << i << endl;
      m_roadItems[ i ]->dump( out );
   }
   for ( uint32 i = 0 ; i < m_landmarkItems.size() ; ++i ) {
      out << "## road landmark " << i << endl;
      m_landmarkItems[ i ]->dump( out );
   }
   for ( uint32 i = 0 ; i < m_lanes.size() ; ++i ) {
      out << "## lane group " << i << endl;
      m_lanes[ i ].dump( out );
   }
   for ( uint32 i = 0 ; i < m_signPosts.size() ; ++i ) {
      out << "## Sign posts " << i << endl;
      m_signPosts[ i ].dump( out );
   }
}


ExpandedRouteItem::routeturn_t
ExpandedRouteItem::getTurnType() const {
   return m_turn;
}


uint32
ExpandedRouteItem::getDist() const {
   return m_dist;
}


uint32
ExpandedRouteItem::getTime() const {
   return m_time;
}


const NameCollection*
ExpandedRouteItem::getIntoRoadName() const {
   return m_intoRoadName;
}


ItemTypes::transportation_t
ExpandedRouteItem::getTransportation() const {
   return m_transport;
}


int32
ExpandedRouteItem::getLat() const {
   return m_lat;
}


int32
ExpandedRouteItem::getLon() const {
   return m_lon;
}


uint32
ExpandedRouteItem::getTurnNumber() const {
   return m_turnNumber;
}


ItemTypes::crossingkind_t
ExpandedRouteItem::getCrossingType() const {
   return m_crossingKind;
}


const MC2BoundingBox& 
ExpandedRouteItem::getTurnBoundingbox() const {
   return m_turnBoundingbox;
}


void
ExpandedRouteItem::setTurnType( routeturn_t turn ) {
   m_turn = turn;
}


void
ExpandedRouteItem::setDist( uint32 dist ) {
   m_dist = dist;
}


void
ExpandedRouteItem::setTime( uint32 time ) {
   m_time = time;
}


void
ExpandedRouteItem::setIntoRoadName( NameCollection* name ) {
   MC2_ASSERT( name != NULL );
   delete m_intoRoadName;
   m_intoRoadName = name;
}


void
ExpandedRouteItem::setTransportation( 
   ItemTypes::transportation_t transport )
{
   m_transport = transport;
}


void
ExpandedRouteItem::setLat( int32 lat ) {
   m_lat = lat;
}


void
ExpandedRouteItem::setLon( int32 lon ) {
   m_lon = lon;
}


void
ExpandedRouteItem::setTurnNumber( uint32 turnNumber ) {
   m_turnNumber = turnNumber;
}


void
ExpandedRouteItem::setCrossingType( 
   ItemTypes::crossingkind_t crossingKind )
{
   m_crossingKind = crossingKind;
}


void
ExpandedRouteItem::setTurnBoundingbox( 
   const MC2BoundingBox& turnBoundingbox )
{
   m_turnBoundingbox = turnBoundingbox;
}


uint32 
ExpandedRouteItem::getNbrExpandedRouteRoadItems() const {
   return m_roadItems.size();
}


const ExpandedRouteRoadItem* 
ExpandedRouteItem::getExpandedRouteRoadItem( uint32 index ) const {
   MC2_ASSERT( index < getNbrExpandedRouteRoadItems() );
   return m_roadItems[ index ];
}


void 
ExpandedRouteItem::addExpandedRouteRoadItem( ExpandedRouteRoadItem* item )
{
   m_roadItems.push_back( item );
}


uint32 
ExpandedRouteItem::getNbrExpandedRouteLandmarkItems() const {
   return m_landmarkItems.size();
}


const ExpandedRouteLandmarkItem* 
ExpandedRouteItem::getExpandedRouteLandmarkItem( uint32 index ) const {
   MC2_ASSERT( index < getNbrExpandedRouteLandmarkItems() );
   return m_landmarkItems[ index ];
}


void 
ExpandedRouteItem::addExpandedRouteLandmarkItem( 
   ExpandedRouteLandmarkItem* item )
{
   m_landmarkItems.push_back( item );
}

bool
ExpandedRouteItem::addPossibleTurn(routeturn_t possTurn)
{
   // Check if possible to add
   uint32 nbrPossExits;
   if(m_crossingKind <= ItemTypes::NO_CROSSING)
      nbrPossExits = 0;
   else if(m_crossingKind == ItemTypes::CROSSING_3WAYS_T)
      nbrPossExits = 1;
   else if(m_crossingKind < ItemTypes::CROSSING_2ROUNDABOUT)
      nbrPossExits = int(m_crossingKind)-2;
   else  // Roundabout xrossing difficult to know the number of exits.
      nbrPossExits = 2;
   
   if(m_nbrPossTurns < nbrPossExits){
      m_possTurns[m_nbrPossTurns++] = possTurn;
      return true;
   }
   return false;
}

uint32
ExpandedRouteItem::getNbrPossibleTurns() const
{
   return m_nbrPossTurns;
}

ExpandedRouteItem::routeturn_t
ExpandedRouteItem::getPossibleTurn(uint32 index) const
{
   uint32 nbrPossExits;
   if(m_crossingKind <= ItemTypes::NO_CROSSING)
      nbrPossExits = 0;
   else if(m_crossingKind == ItemTypes::CROSSING_3WAYS_T)
      nbrPossExits = 1;
   else if(m_crossingKind < ItemTypes::CROSSING_2ROUNDABOUT)
      nbrPossExits = int(m_crossingKind)-2;
   else  // Roundabout xrossing difficult to know the number of exits.
      nbrPossExits = 2;
   
   // Check if index ok
   if(index >= nbrPossExits){
      return NO_TURN;
   }

   // Check if poss turm exist
   if(index >= m_nbrPossTurns){
      return UNDEFINED;
   }
   
   // Return pos turn at index
   return m_possTurns[index];
}


uint32 
ExpandedRouteItem::save( Packet* p, int& pos ) const {
   uint32 startOffset = p->getLength();

   p->incWriteLong(pos, m_dist);
   p->incWriteLong(pos, m_time);
   p->incWriteLong(pos, m_lat);
   p->incWriteLong(pos, m_lon);
   p->incWriteLong(pos, m_turnNumber);
   p->incWriteLong(pos, m_turnBoundingbox.getMaxLat());
   p->incWriteLong(pos, m_turnBoundingbox.getMinLat());
   p->incWriteLong(pos, m_turnBoundingbox.getMaxLon());
   p->incWriteLong(pos, m_turnBoundingbox.getMinLon());
   
   p->incWriteByte(pos, (byte)m_turn);
   p->incWriteByte(pos, (byte)m_transport);
   p->incWriteByte(pos, (byte)m_crossingKind);
   p->incWriteByte(pos, (byte)m_nbrPossTurns);
   for(uint32 i = 0 ; i < m_nbrPossTurns ; i++){
      p->incWriteByte(pos, (byte)m_possTurns[i]);
   }

   if (m_isEndOfRoad)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);

   m_roadItems.store(p, pos);
   m_landmarkItems.store(p, pos);

   m_lanes.save( p, pos );
   m_signPosts.save( p, pos );

   p->setLength(pos); 
   return (p->getLength() - startOffset); 
}

uint32
ExpandedRouteItem::getSizeAsBytes() const
{
   // The (u)int32's 
   uint32 size = 5*4;

   // Adding bool and enums(routet + transp + cross)
   size += 1 + 1 +1 + 1;

   // Adding size of name(s)
   size += m_intoRoadName->getSizeInDataBuffer();

   // Adding vectors
   size += m_roadItems.getSizeAsBytes(); 
   size += m_landmarkItems.getSizeAsBytes();

   // Adding boundingBox
   size += 4*4;

   // Adding nbr possible turns + the turns.
   size += 1 + m_nbrPossTurns;

   // Adding end of road flag.
   size += 1;

   // Lanes
   size += m_lanes.getSizeInPacket();

   // Signpostsosts
   size += m_signPosts.getSizeInPacket();

   return size;
}

void
ExpandedRouteItem::load( const Packet* p, int& pos ) {
   m_dist         = p->incReadLong(pos);
   m_time         = p->incReadLong(pos);
   m_lat          = p->incReadLong(pos);
   m_lon          = p->incReadLong(pos);
   m_turnNumber   = p->incReadLong(pos);
   m_turnBoundingbox.setMaxLat(p->incReadLong(pos));
   m_turnBoundingbox.setMinLat(p->incReadLong(pos));
   m_turnBoundingbox.setMaxLon(p->incReadLong(pos));
   m_turnBoundingbox.setMinLon(p->incReadLong(pos));
   m_turn         = (routeturn_t)p->incReadByte(pos);
   m_transport    = (ItemTypes::transportation_t)p->incReadByte(pos);
   m_crossingKind = (ItemTypes::crossingkind_t)p->incReadByte(pos);
   m_nbrPossTurns = uint32(p->incReadByte(pos));
   for(uint32 i = 0 ; i < m_nbrPossTurns ; i++){
      m_possTurns[i] = (routeturn_t)p->incReadByte(pos);
   }
   
   m_isEndOfRoad  = (p->incReadByte(pos) != 0);

   m_roadItems.restore( p, pos );
   m_landmarkItems.restore( p, pos );

   m_lanes.load( p, pos );
   m_signPosts.load( p, pos );
} 


// =======================================================================
//                                                  ExpandedRouteItemsList =
// =======================================================================

ExpandedRouteItemsList::~ExpandedRouteItemsList()
{
   mc2dbg8 << "ExpandedRouteItemsList::~ExpandedRouteItemsList size " 
           << size() << endl;  
   iterator i = begin();  
   while (i != end()) {  
      delete (*i);  
      erase(i++);  
   } 
}

void
ExpandedRouteItemsList::store( Packet* packet, int& pos ) const {
   if (packet->getBufSize()-pos < getSizeAsBytes()) { 
      mc2log << warn << "ExpandedRouteItemsList (" << getSizeAsBytes()  
             << " bytes) to large for packet (" << packet->getBufSize() 
             << " - " << pos << " bytes). Resizing!" << endl; 
      packet->resize(pos + getSizeAsBytes() + 8); 
   } 

   const_iterator i; 

   packet->incWriteLong(pos, size()); 
   for (i = begin(); i != end(); i++) { 
      (*i)->save(packet, pos); 
   } 
}

void
ExpandedRouteItemsList::restore( const Packet* packet, int& pos ) {
   uint32 nbrElm = packet->incReadLong(pos); 
    
   for (uint32 i = 0; i < nbrElm; i++) {   
      push_back(new ExpandedRouteItem(packet, pos));  
   } 
}

void
ExpandedRouteItemsList::addExpandedRouteItem(ExpandedRouteItem* elm)
{
   push_back(elm);
}


uint32
ExpandedRouteItemsList::getSizeAsBytes() const
{
   
   uint32 size = 0;
   for (const_iterator i = begin(); i != end(); i++) { 
      size += (*i)->getSizeAsBytes();  
   } 
   return size;
}


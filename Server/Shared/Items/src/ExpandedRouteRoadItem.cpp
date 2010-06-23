/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandedRouteRoadItem.h"
#include "StringUtility.h"
#include "Packet.h"
#include "NameCollection.h"
#include "AlignUtility.h"
#include "StringTable.h"

ExpandedRouteRoadItem::ExpandedRouteRoadItem( 
   uint32 mapID,
   uint32 nodeID,
   ItemTypes::roadClass roadClass,
   NameCollection* roadName,
   uint32 posSpeedLimit, 
   uint32 negSpeedLimit, 
   bool multidigitialized,
   bool ramp, bool roundabout,
   bool controlledAccess,
   bool turn,
   bool driveOnRightSide,
   int8 startLevel, int8 endLevel,
   ItemTypes::entryrestriction_t posEntryrestr,
   ItemTypes::entryrestriction_t negEntryrestr )
      : m_mapID( mapID ),
        m_nodeID( nodeID ),
        m_roadClass( roadClass ),
        m_roadName( roadName ),
        m_posSpeedLimit( posSpeedLimit ),
        m_negSpeedLimit( negSpeedLimit ),
        m_multidigitialized( multidigitialized ),
        m_ramp( ramp ),
        m_roundabout( roundabout ),
        m_controlledAccess( controlledAccess ),
        m_turn( turn ),
        m_driveOnRightSide( driveOnRightSide ),
        m_startLevel( startLevel ),
        m_endLevel( endLevel ),
        m_posEntryrestr( posEntryrestr ),
        m_negEntryrestr( negEntryrestr )
{
   MC2_ASSERT( roadName != NULL );
}

ExpandedRouteRoadItem::ExpandedRouteRoadItem( const Packet* p, int &pos ) {
   load( p, pos );
}


ExpandedRouteRoadItem::~ExpandedRouteRoadItem() {
   delete m_roadName;
}


void 
ExpandedRouteRoadItem::dump( ostream& out ) const {
   out << "### mapID " << m_mapID << endl
       << "### nodeID " << m_nodeID << endl
       << "### roadClass " << StringTable::getString( 
      ItemTypes::getRoadClassSC( m_roadClass ),
      StringTable::ENGLISH ) << endl
       << "### roadName " << *m_roadName << endl
       << "### posSpeedLimit " << m_posSpeedLimit << endl
       << "### negSpeedLimit " << m_negSpeedLimit << endl
       << "### multidigitialized " << BP(m_multidigitialized) << endl
       << "### ramp " << BP(m_ramp) << endl
       << "### roundabout " <<BP(m_roundabout) << endl
       << "### controlledAccess " << BP(m_controlledAccess) << endl
       << "### turn " << BP(m_turn) << endl
       << "### driveOnRightSide " << BP(m_driveOnRightSide) << endl
       << "### startLevel " << int(m_startLevel) << endl
       << "### endLevel " << int(m_endLevel) << endl
       << "### posEntryrestr " << StringTable::getString( 
          ItemTypes::getEntryRestrictionSC( m_posEntryrestr ),
          StringTable::ENGLISH ) << endl
       << "### negEntryrestr " << StringTable::getString( 
          ItemTypes::getEntryRestrictionSC( m_negEntryrestr ),
          StringTable::ENGLISH ) << endl
       << "### nbr coordinates " << m_coords.size() << endl;
   for ( uint32 i = 0 ; i < m_coords.size() ; ++i ) {
      out << "### coord " << i << endl
          << "#### Lat " << m_coords[ i ].lat << endl
          << "#### Lon " << m_coords[ i ].lon << endl;
   }
}


uint32
ExpandedRouteRoadItem::getNodeID() const {
   return m_nodeID;
}


void
ExpandedRouteRoadItem::setNodeID( uint32 nodeID ) {
   m_nodeID = nodeID;
}


uint32
ExpandedRouteRoadItem::getMapID() const {
   return m_mapID;
}


void
ExpandedRouteRoadItem::setMapID( uint32 mapID ) {
   m_mapID = mapID;
}


ItemTypes::roadClass 
ExpandedRouteRoadItem::getRoadClass() const {
   return m_roadClass;
}


void 
ExpandedRouteRoadItem::setRoadClass( ItemTypes::roadClass roadClass ) {
   m_roadClass = roadClass;
}


const NameCollection* 
ExpandedRouteRoadItem::getRoadName() const {
   return m_roadName;
}


void 
ExpandedRouteRoadItem::setRoadName( NameCollection* roadName ) {
   m_roadName = roadName;
}


uint32 
ExpandedRouteRoadItem::getPosSpeedLimit() const {
   return m_posSpeedLimit;
}


void 
ExpandedRouteRoadItem::setPosSpeedLimit( uint32 speed ) {
   m_posSpeedLimit = speed;
}


uint32 
ExpandedRouteRoadItem::getNegSpeedLimit() const {
   return m_negSpeedLimit;
}


void 
ExpandedRouteRoadItem::setNegSpeedLimit( uint32 speed ) {
   m_negSpeedLimit = speed;
}


bool 
ExpandedRouteRoadItem::getMultidigitialized() const {
   return m_multidigitialized;
}


void 
ExpandedRouteRoadItem::setMultidigitialized( bool multidigitialized ) {
   m_multidigitialized = multidigitialized;
}


bool 
ExpandedRouteRoadItem::getRamp() const {
   return m_ramp;
}


void 
ExpandedRouteRoadItem::setRamp( bool ramp ) {
   m_ramp = ramp;
}


bool 
ExpandedRouteRoadItem::getRoundabout() const {
   return m_roundabout;
}


void 
ExpandedRouteRoadItem::setRoundabout( bool roundabout ) {
   m_roundabout = roundabout;
}


bool 
ExpandedRouteRoadItem::getControlledAccess() const {
   return m_controlledAccess;
}


void 
ExpandedRouteRoadItem::setControlledAccess( bool controlledAccess ) {
   m_controlledAccess = controlledAccess;
}


bool 
ExpandedRouteRoadItem::getTurn() const {
   return m_turn;
}


void 
ExpandedRouteRoadItem::setTurn( bool turn ) {
   m_turn = turn;
}

bool 
ExpandedRouteRoadItem::getDriveOnRightSide() const {
   return m_driveOnRightSide;
}


void 
ExpandedRouteRoadItem::setDriveOnRightSide( bool driveOnRightSide ) {
   m_driveOnRightSide = driveOnRightSide;
}


int8 
ExpandedRouteRoadItem::getStartLevel() const {
   return m_startLevel;
}


void 
ExpandedRouteRoadItem::setStartLevel( int8 level ) {
   m_startLevel = level;
}


int8 
ExpandedRouteRoadItem::getEndLevel() const {
   return m_endLevel;
}


void 
ExpandedRouteRoadItem::setEndLevel( int8 level ) {
   m_endLevel = level;
}


ItemTypes::entryrestriction_t 
ExpandedRouteRoadItem::getPosEntryrestrictions() const {
   return m_posEntryrestr;
}


void 
ExpandedRouteRoadItem::setPosEntryrestrictions(
   ItemTypes::entryrestriction_t entryrestr )
{
   m_posEntryrestr = entryrestr;
}


ItemTypes::entryrestriction_t 
ExpandedRouteRoadItem::getNegEntryrestrictions() const {
   return m_negEntryrestr;
}


void 
ExpandedRouteRoadItem::setNegEntryrestrictions( 
   ItemTypes::entryrestriction_t entryrestr )
{
   m_negEntryrestr = entryrestr;
}


uint32 
ExpandedRouteRoadItem::getNbrCoordinates() const {
   return m_coords.size();
}


const MC2Coordinate 
ExpandedRouteRoadItem::getCoordinate( uint32 index ) const {
   MC2_ASSERT( index < getNbrCoordinates() );
   return m_coords[ index ];
}


vector<MC2Coordinate>::const_iterator 
ExpandedRouteRoadItem::coordsBegin() const
{
   return m_coords.begin();
}


vector<MC2Coordinate>::const_iterator 
ExpandedRouteRoadItem::coordsEnd() const
{
   return m_coords.end();
}


void 
ExpandedRouteRoadItem::addCoordinate( const MC2Coordinate coord ) {
   m_coords.push_back( coord );
}

uint32
ExpandedRouteRoadItem::getSizeAsBytes()
{
   // uint32's : speed + negSpeed
   uint32 size = 2*4;

   // relative levels
   size += 1 +1;
   
   // enums : roadClass+ posEntry + negEntry
   size += 3;

   // bools : multiDig + ramp + rAbout+ cAccess + turn
   size += 5;

   // Names :
   size += m_roadName->getSizeInPacket();

   // Coords: +numberOfCoords 
   size += 2*4*m_coords.size()  +4;
   
   return size;
}

void
ExpandedRouteRoadItem::load( const Packet* p, int& pos ) {
   // Longs
   m_posSpeedLimit      = p->incReadLong(pos);
   m_negSpeedLimit      = p->incReadLong(pos);
   m_mapID              = p->incReadLong( pos );
   m_nodeID             = p->incReadLong( pos );

   // Coord vector
   uint32 nbrCoords     = p->incReadLong(pos);
   for(uint32 i = 1 ; i< nbrCoords ; i++){
      m_coords.push_back(MC2Coordinate(p->incReadLong(pos),
                                       p->incReadLong(pos)));
   }

   // Bytes, enums and bools
   m_startLevel         = p->incReadByte(pos);
   m_endLevel           = p->incReadByte(pos);
   m_roadClass          = (ItemTypes::roadClass)p->incReadByte(pos);
   m_posEntryrestr      = (ItemTypes::entryrestriction_t)p->incReadByte(pos);
   m_negEntryrestr      = (ItemTypes::entryrestriction_t)p->incReadByte(pos);
   m_multidigitialized  =(p->incReadByte(pos) !=0 );
   m_ramp               =(p->incReadByte(pos) !=0 );
   m_roundabout         =(p->incReadByte(pos) !=0 );
   m_controlledAccess   =(p->incReadByte(pos) !=0 );
   m_turn               =(p->incReadByte(pos) !=0 );

   // Make sure NameCollection gets aligned or NameCollection will abort.
   AlignUtility::alignLong( pos );
   // NameCollection
   m_roadName->load(p, pos);
}


uint32
ExpandedRouteRoadItem::save( Packet* p, int& pos ) const {
   uint32 startOffset = p->getLength();

   // Longs
   p->incWriteLong(pos, m_posSpeedLimit);
   p->incWriteLong(pos, m_negSpeedLimit);
   p->incWriteLong( pos, m_mapID );
   p->incWriteLong( pos, m_nodeID );

   // Coords vector
   p->incWriteLong(pos, m_coords.size());
   vector<MC2Coordinate>::const_iterator i = m_coords.begin();
   for(i = m_coords.begin(); i != m_coords.end(); i++) {
         p->incWriteLong(pos,(*i).lat);
         p->incWriteLong(pos,(*i).lon);
   }
                   
   // Bytes                
   p->incWriteByte(pos, m_startLevel);
   p->incWriteByte(pos, m_endLevel);
   p->incWriteByte(pos, (byte)m_roadClass);
   p->incWriteByte(pos, (byte)m_posEntryrestr);
   p->incWriteByte(pos, (byte)m_negEntryrestr);

   // Bools
   if (m_multidigitialized)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);
   if (m_ramp)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);
   if (m_roundabout)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);
   if (m_controlledAccess)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);
   if (m_turn)
      p->incWriteByte(pos, MAX_BYTE);
   else 
      p->incWriteByte(pos, 0);

   // Make sure NameCollection gets aligned or NameCollection will abort.
   AlignUtility::alignLong( pos );
   m_roadName->save(p, pos);
   p->setLength(pos);
   return (p->getLength() - startOffset); 
}

// =======================================================================
//                                           ExpandedRouteRoadVector =
// =======================================================================


ExpandedRouteRoadVector::~ExpandedRouteRoadVector()
{
   mc2dbg8<<"ExpandedRouteRoadVector::~ExpandedRouteRoadVector size "
          << size() << endl;
   for( iterator it(begin());
        it != end();
        ++it ) {
      delete *it;
   }
}

void
ExpandedRouteRoadVector::store( Packet* packet, int& pos ) const {
   if (packet->getBufSize()-pos < getSizeAsBytes()) { 
      mc2log << warn << "ExpandedRouteRoadVector (" << getSizeAsBytes()  
             << " bytes) too large for packet (" << packet->getBufSize() 
             << " - " << pos << " bytes). Resizing!" << endl; 
      packet->resize( packet->getBufSize() * 2 + getSizeAsBytes() );
   } 

   const_iterator i; 

   packet->incWriteLong(pos, size()); 
   for (i = begin(); i != end(); i++) { 
      (*i)->save(packet, pos); 
   } 

}

void
ExpandedRouteRoadVector::restore( const Packet* packet, int& pos ) {
   uint32 nbrElm = packet->incReadLong(pos); 
    
   for (uint32 i = 0; i < nbrElm; i++) {   
      push_back(new ExpandedRouteRoadItem(packet, pos));  
   } 

}

void
ExpandedRouteRoadVector::addExpandedRouteRoad(
   ExpandedRouteRoadItem* item)
{
   push_back( item );
}

uint32
ExpandedRouteRoadVector::getSizeAsBytes() const
{
   uint32 size = 0;
   for (const_iterator i = begin(); i != end(); i++) {
      size += (*i)->getSizeAsBytes();
   }
   return size;
}


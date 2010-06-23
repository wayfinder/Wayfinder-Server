/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandedRouteMatch.h"
#include "Packet.h"
#include "NameCollection.h"
#include "StringTable.h"

ExpandedRouteMatch::ExpandedRouteMatch( 
   const Name* name, 
   uint32 mapID, uint32 itemID,
   ItemTypes::itemType type,
   int32 lat, int32 lon,
   ItemTypes::routedir_nbr_t houseNbrDir,
   ItemTypes::routedir_oddeven_t houseNbrOdd,
   uint32 houseNbr, uint16 angle )
      : m_name( *name ),
        m_mapID( mapID ),
        m_itemID( itemID ),
        m_type( type ),
        m_lat( lat ),
        m_lon( lon ),
        m_houseNbrDir( houseNbrDir ),
        m_houseNbrOdd( houseNbrOdd ),
        m_houseNbr( houseNbr),
        m_angle( angle )
{
}


ExpandedRouteMatch::~ExpandedRouteMatch() {
   // m_name deletes its string self
}


void 
ExpandedRouteMatch::dump( ostream& out ) const {
   out << "## name " << m_name << endl
       << "## mapID " << m_mapID << endl
       << "## itemID " << m_itemID << endl
       << "## type " << StringTable::getString( 
          ItemTypes::getItemTypeSC( m_type ), StringTable::ENGLISH ) 
       << endl
       << "## houseNbrDir " << StringTable::getString( 
          ItemTypes::getStartDirectionHousenumberSC( m_houseNbrDir ), 
          StringTable::ENGLISH ) << endl
       << "## houseNbrOdd " << StringTable::getString( 
          ItemTypes::getStartDirectionOddEvenSC( m_houseNbrOdd ), 
          StringTable::ENGLISH ) << endl
       << "## houseNbr " << m_houseNbr << endl
       << "## angle " << m_angle << endl;
}


const Name* 
ExpandedRouteMatch::getName() const {
   return &m_name;
}


uint32
ExpandedRouteMatch::getMapID() const {
   return m_mapID;
}


uint32
ExpandedRouteMatch::getItemID() const {
   return m_itemID;
}


ItemTypes::itemType 
ExpandedRouteMatch::getType() const {
   return m_type;
}


void 
ExpandedRouteMatch::setType( ItemTypes::itemType type ) {
   m_type = type;
}


int32 
ExpandedRouteMatch::getLat() const {
   return m_lat;
}


void 
ExpandedRouteMatch::setLat( int32 lat ) {
   m_lat = lat;
}


int32 
ExpandedRouteMatch::getLon() const {
   return m_lon;
}


void 
ExpandedRouteMatch::setLon( int32 lon ) {
   m_lon = lon;
}


ItemTypes::routedir_nbr_t
ExpandedRouteMatch::getDirectionHousenumber() const {
   return m_houseNbrDir;
}


ItemTypes::routedir_oddeven_t
ExpandedRouteMatch::getDirectionOddEven() const {
   return m_houseNbrOdd;
}


uint32
ExpandedRouteMatch::getHousenumber() const {
   return m_houseNbr;
}


uint16
ExpandedRouteMatch::getAngle() const {
   return m_angle;
}


void
ExpandedRouteMatch::setName( const Name* name ) {
   m_name = *name;
}


void
ExpandedRouteMatch::setMapID( uint32 mapID ) {
   m_mapID = mapID;
}


void
ExpandedRouteMatch::setItemID( uint32 itemID ) {
   m_itemID = itemID;
}


void
ExpandedRouteMatch::setDirectionHousenumber( 
   ItemTypes::routedir_nbr_t houseNbrDir )
{
   m_houseNbrDir = houseNbrDir;  
}


void
ExpandedRouteMatch::setDirectionOddEven( 
   ItemTypes::routedir_oddeven_t houseNbrOdd )
{
   m_houseNbrOdd = houseNbrOdd;
}


void
ExpandedRouteMatch::setHousenumber( uint32 houseNbr ) {
   m_houseNbr = houseNbr;
}


void
ExpandedRouteMatch::setAngle( uint16 angle ) {
   m_angle = angle;
}

uint32
ExpandedRouteMatch::getSizeAsBytes() const {
    NameCollection nc;
    nc.addName( new Name( m_name ) );
    
    uint32 size = 0;
    // name
    size += nc.getSizeInPacket();
    // m_mapID, m_itemID, m_type, m_lat, m_lon, m_houseNbrDir, m_houseNbrOdd, m_houseNbr
    size += 8*4;
    // m_angle
    size += 1*2;

    return size;
}

void
ExpandedRouteMatch::load( Packet* p, int& pos ) {

}

uint32
ExpandedRouteMatch::save( Packet* p, int& pos ) const {
   uint32 startOffset = p->getLength();

   // Make sure NameCollection gets aligned or NameCollection will abort.
   AlignUtility::alignLong( pos );
   NameCollection nc;
   nc.addName( new Name( m_name ) );
   nc.save( p, pos );

   p->incWriteLong( pos, m_mapID  );
   p->incWriteLong( pos, m_itemID  );
   p->incWriteLong( pos, m_type  );
   p->incWriteLong( pos, m_lat  );
   p->incWriteLong( pos, m_lon  );
   p->incWriteLong( pos, m_houseNbrDir  );
   p->incWriteLong( pos, m_houseNbrOdd  );
   p->incWriteLong( pos, m_houseNbr  );
   p->incWriteShort( pos, m_angle  );

   p->setLength( pos ); 
   return p->getLength() - startOffset;
}

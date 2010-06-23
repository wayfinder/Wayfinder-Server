/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandedRouteLandmarkItem.h"
#include "StringUtility.h"
#include "Packet.h"
#include "DataBuffer.h"
#include "NameCollection.h"
#include "StringTable.h"

ExpandedRouteLandmarkItem::ExpandedRouteLandmarkItem( 
   ItemTypes::landmark_t type,
   ItemTypes::landmarklocation_t location,
   SearchTypes::side_t side,
   bool atTurn,
   uint32 importance,
   uint32 dist,
   NameCollection* name,
   uint32 landmarkID,
   bool isTraffic,
   bool isDetour,
   bool isStart,
   bool isStop,
   const char* streetName)
      : m_type( type ),
        m_location( location ),
        m_side( side ),
        m_atTurn( atTurn ),
        m_importance( importance ),
        m_dist( dist ),
        m_name( name ),
        m_landmarkID( landmarkID ),
        m_isTraffic(isTraffic),
        m_isDetour(isDetour),
        m_isStart(isStart),
        m_isStop(isStop)
{
   MC2_ASSERT( name != NULL );
   if(streetName == NULL)
      m_streetName = NULL;
   else
      addStreetName(streetName);
}

ExpandedRouteLandmarkItem::ExpandedRouteLandmarkItem( 
   const Packet* packet, int& pos ) {
   m_isTraffic  = false;
   m_isStart    = true;
   m_isStop     = true;
   m_isDetour   = false;
   m_streetName = NULL;
   load( packet, pos );
}


ExpandedRouteLandmarkItem::~ExpandedRouteLandmarkItem() {
   delete m_name;
   delete [] m_streetName;
}


void 
ExpandedRouteLandmarkItem::dump( ostream& out ) const {
   out << "### type " << ( m_type == ItemTypes::builtUpAreaLM ? "BUA" :
                           m_type == ItemTypes::railwayLM ? "Railway" :
                           m_type == ItemTypes::areaLM ? "Area" :
                           m_type == ItemTypes::poiLM ? "POI" :
                           m_type == ItemTypes::signPostLM ? "Signpost" :
                           m_type == ItemTypes::passedStreetLM ? "passStreet" :
                           m_type == ItemTypes::accidentLM ? "accident" :
                           m_type == ItemTypes::roadWorkLM ? "roadWork" :
                           m_type == ItemTypes::cameraLM ? "camera" :
                           m_type == ItemTypes::speedTrapLM ? "speedTrap" :
                           m_type == ItemTypes::weatherLM ? "weather" :
                           m_type == ItemTypes::policeLM ? "police" :
                           m_type == ItemTypes::trafficOtherLM ? "traffic" :
                           "unknown" ) << endl
       << "### location " << StringTable::getString( 
          ItemTypes::getLandmarkLocationSC( m_location ), 
          StringTable::ENGLISH ) << endl
       << "### side " << (m_side == SearchTypes::left_side ? "left_side" :
                          m_side == SearchTypes::right_side ? "right_side":
                          m_side == SearchTypes::unknown_side ? 
                          "unknown_side" :
                          m_side == SearchTypes::undefined_side ? 
                          "undefined_side" :
                          m_side == SearchTypes::side_does_not_matter ? 
                          "side_does_not_matter" :
                          m_side == SearchTypes::left_side_exit ? 
                          "left_side_exit" :
                          m_side == SearchTypes::right_side_exit ? 
                          "right_side_exit" :
                          m_side == SearchTypes:: max_defined_side? 
                          "max_defined_side" : "unknown value") << endl
       << "### atTurn " << BP(m_atTurn) << endl
       << "### importance " << m_importance << endl
       << "### dist " << m_dist << endl
       << "### name " << *m_name << endl;
   if(m_isTraffic){
      out << "#### traffic" <<endl;
      if(m_isDetour)
         out << "#### Detour" <<endl;
      if(m_isStart)
         out << "#### isStart" <<endl;
      if(m_isStop)
         out << "#### isStop" <<endl;
   } else {
      out << "#### not traffic" <<endl;
   }
   
   
}


ItemTypes::landmark_t 
ExpandedRouteLandmarkItem::getType() const {
   return m_type;
}


void 
ExpandedRouteLandmarkItem::setType( ItemTypes::landmark_t type ) {
   m_type = type;
}


ItemTypes::landmarklocation_t 
ExpandedRouteLandmarkItem::getLocation() const {
   return m_location;
}


void 
ExpandedRouteLandmarkItem::setLocation( 
   ItemTypes::landmarklocation_t location )
{
   m_location = location;
}


SearchTypes::side_t 
ExpandedRouteLandmarkItem::getRoadSide() const {
   return m_side;
}


void 
ExpandedRouteLandmarkItem::setRoadSide( SearchTypes::side_t side ) {
   m_side = side;
}


bool 
ExpandedRouteLandmarkItem::getAtTurn() const {
   return m_atTurn;
}


void 
ExpandedRouteLandmarkItem::setAtTurn( bool atTurn ) {
   m_atTurn = atTurn;  
}


uint32 
ExpandedRouteLandmarkItem::getImportance() const {
   return m_importance;
}


void 
ExpandedRouteLandmarkItem::setImportance( uint32 importance ) {
   m_importance = importance;
}


uint32 
ExpandedRouteLandmarkItem::getDist() const {
   return m_dist;
}


void 
ExpandedRouteLandmarkItem::setDist( uint32 dist ) {
   m_dist = dist;
}


const NameCollection* 
ExpandedRouteLandmarkItem::getName() const {
   return m_name;
}


void 
ExpandedRouteLandmarkItem::setName( NameCollection* name ) {
   m_name = name;
}

uint32
ExpandedRouteLandmarkItem::getSizeAsBytes() const
{
   // type+loc+side+atTurn+import+dist
   uint32 size = 1+1+1+1+4+4+4;

   
   size += m_name->getSizeInDataBuffer();
   
   return size;
}

void 
ExpandedRouteLandmarkItem::load( const Packet* p, int& pos ) {
   // Longs
   m_dist         = p->incReadLong(pos);
   m_importance   = p->incReadLong(pos);
   m_landmarkID   = p->incReadLong(pos);
   
   // Enums and bool
   m_type         = (ItemTypes::landmark_t)p->incReadByte(pos);
   m_location     = (ItemTypes::landmarklocation_t)p->incReadByte(pos);
   m_side         = (SearchTypes::side_t)p->incReadByte(pos);
   byte boolByte  = p->incReadByte(pos);
   m_atTurn       =((boolByte & 0x01) !=0 );
   m_isTraffic    =((boolByte & 0x02) !=0 );
   m_isDetour     =((boolByte & 0x04) !=0 );
   m_isStart      =((boolByte & 0x08) !=0 );
   m_isStop       =((boolByte & 0x10) !=0 );
   char* tmpStr = NULL;
   p->incReadString(pos, tmpStr);
   if ( tmpStr == NULL || tmpStr[0] == 0 ){
      m_streetName = NULL;
   } else {
      m_streetName = StringUtility::newStrDup(tmpStr);
   }
      
   // NameCollection
   uint32 bufSize = p->incReadLong(pos);
   byte* why_this = NULL; // Read the documentation, maybe?
   DataBuffer* tempBuf = new DataBuffer(p->incReadByteArray(pos, why_this,
                                                            bufSize), bufSize);
   m_name->load(tempBuf);   
   delete tempBuf;
                            
}

uint32
ExpandedRouteLandmarkItem::save( Packet* p, int& pos ) const
{
   uint32 startOffset = p->getLength();
   
   p->incWriteLong(pos, m_importance);
   p->incWriteLong(pos, m_dist);
   p->incWriteLong(pos, m_landmarkID);
   p->incWriteByte(pos, (byte)m_type);
   p->incWriteByte(pos, (byte)m_location);
   p->incWriteByte(pos, (byte)m_side);
   
   byte boolByte = 0;
   if (m_atTurn)
      boolByte |= 0x01;
   if(m_isTraffic)
      boolByte |= 0x02;
   if(m_isDetour)
      boolByte |= 0x04;
   if(m_isStart)
      boolByte |= 0x08;
   if(m_isStop)
      boolByte |= 0x10;
   p->incWriteByte(pos, boolByte );

   if(m_streetName != NULL)
      p->incWriteString(pos, m_streetName);
   else
      p->incWriteString(pos, "");
   
   DataBuffer* tempBuf = new DataBuffer(m_name->getSizeInDataBuffer());
   m_name->save(tempBuf);
   p->incWriteLong(pos, tempBuf->getBufferSize());
   p->incWriteByteArray(pos, tempBuf->getBufferAddress(),
                        tempBuf->getBufferSize());
   delete tempBuf;

   return (p->getLength() - startOffset); 
}


// =======================================================================
//                                           ExpandedRouteLandmarkVector =
// =======================================================================


ExpandedRouteLandmarkVector::~ExpandedRouteLandmarkVector()
{
   mc2dbg8<<"ExpandedRouteLandmarkVector::~ExpandedRouteLandmarkVector size "
          << size() << endl;
   for ( iterator it(begin());
         it != end();
         ++it ) {
      delete *it;
   }
}

void
ExpandedRouteLandmarkVector::store( Packet* packet, int& pos ) const {
   if (packet->getBufSize()-pos < getSizeAsBytes()) { 
      mc2log << warn << "ExpandedRouteLandmarkVector (" << getSizeAsBytes()  
             << " bytes) to large for packet (" << packet->getBufSize() 
             << " - " << pos << " bytes). Resizing!" << endl; 
      packet->resize( packet->getBufSize()*2 + 8 ); 
   } 

   const_iterator i; 

   packet->incWriteLong(pos, size()); 
   for (i = begin(); i != end(); i++) { 
      (*i)->save(packet, pos); 
   } 

}

void
ExpandedRouteLandmarkVector::restore( const Packet* packet, int& pos ) {
   uint32 nbrElm = packet->incReadLong(pos); 
    
   for (uint32 i = 0; i < nbrElm; i++) {   
      push_back(new ExpandedRouteLandmarkItem(packet, pos));  
   } 

}

void
ExpandedRouteLandmarkVector::addExpandedRouteLandmark(
   ExpandedRouteLandmarkItem* item)
{
   push_back( item );
}

uint32
ExpandedRouteLandmarkVector::getSizeAsBytes() const
{
   uint32 size = 0;
   for (const_iterator i = begin(); i != end(); i++) {
      size += (*i)->getSizeAsBytes();
   }
   return size;
}

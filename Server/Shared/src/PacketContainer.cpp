/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// ************************************************************
#include "PacketContainer.h"
#include "multicast.h"
#include "Packet.h"

#include "MapBits.h"

// ************************************************************

PacketContainer::PacketContainer(Packet* thePacket, 
                                 uint32 serverTimestamp,
                                 byte serverResend,
                                 moduletype_t type,
                                 TimeoutSequence resendTimeoutSequence,
                                 uint32 nbrResends,
                                 uint32 mapSet)
      : m_resendTimeoutSequence(resendTimeoutSequence)
{
   m_timeoutPacket    = NULL;
   m_packet           = thePacket;
   m_server_timestamp = serverTimestamp;
   m_moduleType       = type;
   m_answer           = NULL;
   m_answers          = NULL;
   m_nbrPackets       = 0;
   m_packetNbr        = 0;
   m_sendIP           = 0;
   m_sendPort         = 0;
   m_mapSet           = mapSet;
   
   mc2dbg2 << "[PackCont] Constructor with many params, mapSet: " << mapSet
          << endl;
   if (m_mapSet == MAX_UINT32) {
      checkMapSet();
      m_mapSet = m_packet->getMapSet(); // might be set in a reply packet
   } else
      m_packet->setMapSet(m_mapSet);
   mc2dbg2 << "[PackCont] Constructor with many params, final mapSet: " 
          << m_mapSet << endl;

   setNbrResend( nbrResends );
   // Sets resendNbr in packet
   setServerResend( serverResend );
}


PacketContainer::PacketContainer( Packet* thePacket ) 
: m_resendTimeoutSequence( defaultResendTimeoutTime ) {
   m_timeoutPacket    = NULL;
   m_packet           = thePacket;
   m_server_timestamp = 0;
   m_moduleType       = MODULE_TYPE_INVALID;
   m_answer           = NULL;
   m_answers          = NULL;
   m_nbrPackets       = 0;
   m_packetNbr        = 0;
   m_sendIP           = 0;
   m_sendPort         = 0;
   m_mapSet           = MAX_UINT32;
   checkMapSet();
   setNbrResend( PacketContainer::defaultResends );
}


PacketContainer::~PacketContainer() {
   delete m_packet;
   delete m_answer;
   for ( uint32 i = 0 ; i < m_nbrPackets ; i++ ) {
      if ( m_received[ i ] ) {
         delete m_answers[ i ];
      }
   }
   delete [] m_answers;
   delete m_timeoutPacket;
}

void
PacketContainer::putTimeoutPacket( Packet* toPack )
{
   MC2_ASSERT( m_packet != NULL );
   delete m_timeoutPacket;
   m_timeoutPacket = toPack;
}

PacketContainer*
PacketContainer::newTimeoutContainer() const
{
   if ( m_timeoutPacket ) {
      mc2dbg << "[PC]:ntc m_timeoutPacket != NULL" << endl;
      MC2_ASSERT( m_packet != NULL );
      // Clone the packet and transfer the request and packet IDs from
      // the sent packet to ensure correct id:s.
      Packet* cloned = m_timeoutPacket->getClone(false);
      cloned->setRequestID( m_packet->getRequestID() );
      cloned->setPacketID( m_packet->getPacketID() );
      PacketContainer* cont = new PacketContainer( cloned );
      return cont;
   } else {
      return NULL;
   }
}

void
PacketContainer::checkMapSet()
{
   mc2dbg8 << "[PackCont] checkMapSet top [" << this << "]" << endl;
   if (Properties::getProperty("MAP_SET_COUNT") == NULL) {
      mc2dbg8 << "[PackCont] checkMapSet returning, MAP_SET_COUNT is NULL"
             <<  endl;
      return;
   }
   mc2dbg8 << "[PackCont] checkMapSet m_packet type: " <<
   m_packet->getSubTypeAsString() <<  endl;

   RequestPacket* p = dynamic_cast<RequestPacket*>(m_packet);
   if (p != NULL) {
      uint32 mapID = p->getMapID();
      if (mapID != MAX_UINT32) {
         mc2dbg8 << "[PackCont] checkMapSet, checking mapID: " 
                << prettyMapIDFill(mapID) << endl;
         m_mapSet = MapBits::getMapSetFromMapID( mapID );
         m_packet->setMapSet(m_mapSet);
         mc2dbg8 << "[PackCont] checkMapSet, mapSet: " << m_mapSet << endl;
      } else {
         // either request for a module not using maps or coordinate based!
         mc2dbg8 << "[PackCont] checkMapSet: mapID is MAX_UINT32" << endl;
      }
   } else {
      mc2dbg8 << "[PackCont] checkMapSet packet not a RequestPacket" <<  endl;
   }
}


void 
PacketContainer::addAnswer( Packet* pack ) {
   if ( pack->getNbrPackets() == 1 ) {
      // pack is the answer 
      m_answer = new PacketContainer( pack );
      m_answer->setServerResend( getServerResend() );
   } else {
      if ( m_answers == NULL ) {
         initializeAnswerData( pack );
      }
      if ( addPartialAnswer( pack ) ) {
         makeAnswer();
      }
   }
}


bool 
PacketContainer::answerComplete() const {
   return m_answer != NULL;
}


PacketContainer* 
PacketContainer::getAnswer() {
   PacketContainer* tmp = m_answer;
   m_answer = NULL;
   return tmp;
}


void 
PacketContainer::resetAnswerData() {
   mc2dbg4 << "PacketContainer::resetAnswerData()" << endl;
   delete m_answer;
   m_answer = NULL;
   for ( uint32 i = 0 ; i < m_nbrPackets ; i++ ) {
      if ( m_received[ i ] ) {
         delete m_answers[ i ];
      }
   }
   for ( uint32 i = 0 ; i < m_nbrPackets ; i++ ) {
      mc2dbg8 << "Received[ " << i << " ] = " 
              << m_received[ i ] << endl;
   }
   delete [] m_answers;
   m_answers = NULL;
   m_received.reset( 0 );
   m_nbrPackets = 0;
   m_packetNbr = 0;
}


void
PacketContainer::setServerResend( uint16 count ) {
   getPacket()->setResendNbr( count );
}


byte
PacketContainer::getServerResend() const {
   return getPacket()->getResendNbr();
}


void 
PacketContainer::getIPAndPort( uint32& IP, uint16& port ) const {
   if ( getModuleType() != MODULE_TYPE_INVALID ) {
      // Get leader ip etc.
      IP = MultiCastProperties::getNumericIP( getModuleType(), true );
      port = MultiCastProperties::getPort( getModuleType(), true );
      // mapSet used ?
      if (m_mapSet != MAX_UINT32) {
         mc2dbg8 << "[PackCont] going to change IP and port due to mapSet being set. "
                   "mapSet: " << m_mapSet << ", IP before: " << prettyPrintIP(IP) 
                << ", port before: " << port << endl;
         IP = IP + (m_mapSet << 8);
         port = port | (m_mapSet << 13);
         mc2dbg8 << "[PackCont] changed IP and port. IP now: " << prettyPrintIP(IP) 
                << ", port now: " << port << endl;
      }
   } else {
      IP = getSendIP();
      port = getSendPort();
   }
}


void 
PacketContainer::initializeAnswerData( Packet* pack ) {
   m_nbrPackets = pack->getNbrPackets();   
   delete [] m_answers;
   m_answers = new  Packet*[ m_nbrPackets ];
   m_received.reset( new bool[ m_nbrPackets ] );
   for ( uint32 i = 0 ; i < m_nbrPackets ; i++ ) {
      m_received[ i ] = false;
   }
   m_packetNbr = 0;
}


bool 
PacketContainer::addPartialAnswer( Packet* pack ) {
   uint32 packetNbr = pack->getPacketNbr();
   if ( packetNbr < m_nbrPackets && 
        pack->getResendNbr() == getServerResend() && 
        !m_received[ packetNbr ] ) 
   {
      m_answers[ packetNbr ] = pack;
      m_received[ packetNbr ] = true;
      m_packetNbr++;
      mc2dbg4 << "PacketContainer::addPartialAnswer( " << packetNbr << " / "
              << m_nbrPackets << " ) received on " << (int)getServerResend() 
              << " resend" << endl;
   } else {  
      mc2log << warn << "PacketContainer::addAnswer bogus packet received"
             << endl;
      pack->dump( true ); 
      mc2log << warn << "m_nbrPackets: " << m_nbrPackets << endl
             << "m_packetNbr: " << m_packetNbr << endl << "packetNbr: "
             << packetNbr << endl << "getServerResend(): "
             << (int)getServerResend() << endl << "m_received[ packetNbr ] " 
             << m_received[ packetNbr ] << endl; 
      delete pack;
   }

   return (m_packetNbr == m_nbrPackets);
}


void 
PacketContainer::makeAnswer() {
   uint32 size = 0;
   size += m_answers[ 0 ]->getLength();
   for ( uint32 i = 1 ; i < m_nbrPackets ; i ++ ) {
      size += m_answers[ i ]->getLength() - HEADER_SIZE;
   }
   byte* buffer = reinterpret_cast< byte* > ( 
      new uint32[ (size + 3) >> 2 ] );
   
   // Copy first packet with header
   uint32 length = m_answers[ 0 ]->getLength();
   uint32 pos = 0;
   memcpy( buffer + pos, m_answers[ 0 ]->getBuf(), length );
   pos += length;
   // Copy rest of packets without header
   for ( uint32 i = 1 ; i < m_nbrPackets ; i ++ ) {
      length = m_answers[ i ]->getLength() - HEADER_SIZE;
      memcpy( buffer + pos, 
              m_answers[ i ]->getDataAddress(), 
              length );
      pos += length;
   }

   m_answer = new PacketContainer( Packet::makePacket( buffer, size ),
                                   getServerTimestamp(),
                                   getServerResend(),
                                   getModuleType(),
                                   defaultResendTimeoutTime,
                                   getNbrResend() );
}

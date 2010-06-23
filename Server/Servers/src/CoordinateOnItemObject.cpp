/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CoordinateOnItemObject.h"


CoordinateOnItemObject::CoordinateOnItemObject(  Request* request, 
                                                 bool bboxWanted,
                                                 uint32 startSize ) 
      : m_state( ADDING_ITEMS ),
        m_request( request ),
        m_packetsReadyToSend(),
        m_inputs(),
        m_packetIDs( 8, 8 ),
        m_mapIDs( 8, 8 ),
        m_nbrCoordinatePackets( 0 ),
        m_nbrReceivedCoordinatePackets( 0 ),
        m_bboxWanted( bboxWanted )
{
   m_inputs.reserve( startSize );
}


CoordinateOnItemObject::~CoordinateOnItemObject() {
   // delete all remaining packets in m_packetsReadyToSend
}


uint32
CoordinateOnItemObject::add( uint32 mapID, uint32 itemID, uint16 offset ) {
   inputOutputItem item;
   item.m_mapID = mapID;
   item.m_itemID = itemID;
   item.m_offset = offset;
   item.m_index = 0;
   item.m_lat = MAX_INT32;
   item.m_lon = MAX_INT32;
   item.m_hasLat = false;
   m_inputs.push_back( item );
   return m_inputs.size() - 1;
}


PacketContainer* 
CoordinateOnItemObject::getNextPacket() {
   if ( requestDone() ) {
      return NULL;
   }

   PacketContainer* retPacketCont = m_packetsReadyToSend.getMin();
   if (retPacketCont != NULL) {
      m_packetsReadyToSend.remove( retPacketCont );
   }

   switch( m_state ) {
      case ADDING_ITEMS:
         // Create request packets
         createCoordinatePackets();
         // Return the first packet
         retPacketCont = m_packetsReadyToSend.getMin();
         if (retPacketCont != NULL) {
            m_state = SENDING_COORDINATE_REQUEST;
            m_packetsReadyToSend.remove( retPacketCont );
         } else {
            mc2log << error << "CoordinateOnItemObject::getNextPacket "
               "createCoordinatePackets created no packets, going to "
               "ERROR state." << endl;
            m_state = ERROR;
         }
         break;
      case SENDING_COORDINATE_REQUEST:
         if ( retPacketCont == NULL ) {
            m_state = AWAITING_COORDINATE_REPLY;
         }
         break;

      case AWAITING_COORDINATE_REPLY:
         break;

      case DONE:
         break;

      case ERROR:
         mc2log << error << "CoordinateOnItemObject::getNextPacket "
            "called in ERROR state." << endl;
         break;
   }
   return retPacketCont;
}


void
CoordinateOnItemObject::processPacket( PacketContainer* cont ) {
   if ( cont == NULL ) {
      mc2dbg2 << "CoordinateOnItemObject::processPacket cont is NULL.";
      return;
   }

   switch ( m_state ) {
      case ADDING_ITEMS:
         mc2log << error << "CoordinateOnItemObject::processPacket "
            "called in ADDING_ITEMS state." << endl;
         break;
      case SENDING_COORDINATE_REQUEST:
      case AWAITING_COORDINATE_REPLY: {
         CoordinateOnItemReplyPacket* r = 
            static_cast< CoordinateOnItemReplyPacket* > ( 
               cont->getPacket() );
         if ( r->getStatus() == StringTable::OK ) {
            uint32 mapIndex = m_packetIDs.linearSearch( r->getPacketID() );
            MC2_ASSERT( mapIndex != MAX_UINT32 );
            uint32 mapID = m_mapIDs[ mapIndex ];

            // Add result to m_input
            uint32 currIndex = 0;

            for ( uint32 i = 0 ; i < r->getNbrItems() ; i++ ) {
               while ( m_inputs[currIndex].m_mapID != mapID ) {
                  currIndex++;
               }
               MC2_ASSERT( currIndex < m_inputs.size() );

               // Set the data in the m_inputs[currIndex]
               int32 lat = 0;
               int32 lon = 0;
               uint32 itemID = 0;
               r->getLatLongAndBB( i, itemID, lat, lon, 
                                   m_inputs[currIndex].m_bbox );
               m_inputs[currIndex].m_lat = lat;
               m_inputs[currIndex].m_lon = lon;
               m_inputs[currIndex].m_hasLat = true;
               
               currIndex++;
            }
         } else {
            mc2log << warn << "CoordinateOnItemObject::processPacket "
                   << "received a not ok CoordinateOnItemReplyPacket"
                   << endl;
         }

         m_nbrReceivedCoordinatePackets++;
         if ( m_nbrReceivedCoordinatePackets >= m_nbrCoordinatePackets ) {
            m_state = DONE;
         }
      } break;

      case DONE:
         mc2log << error << "CoordinateOnItemObject::processPacket "
            "called in DONE state." << endl;
         break;

      case ERROR:
         mc2log << error << "CoordinateOnItemObject::processPacket "
            "called in ERROR state." << endl;
         break; 
   }
   
   delete cont;
}


void 
CoordinateOnItemObject::createCoordinatePackets() {
   // The mapIDs to process
   for ( uint32 i = 0; i < m_inputs.size() ; i++) {
      if ( m_mapIDs.linearSearch( m_inputs[i].m_mapID ) == MAX_UINT32 ) {
         // Add unique mapID
         m_mapIDs.addLast( m_inputs[i].m_mapID );
         m_packetIDs.addLast( getNextPacketID() );
      }
   }
   
   for ( uint32 i = 0 ; i < m_mapIDs.getSize() ; i++ ) {
      uint32 mapID = m_mapIDs[ i ];
      CoordinateOnItemRequestPacket* p = new CoordinateOnItemRequestPacket(
         m_packetIDs[ i ], getRequestID(), m_bboxWanted );
      p->setMapID( mapID );

      uint32 nbrAdded = 0;
      for ( uint32 i = 0; i < m_inputs.size() ; i++) {
         if ( m_inputs[i].m_mapID == mapID ) {
            p->add( m_inputs[i].m_itemID, m_inputs[i].m_offset );
            m_inputs[i].m_index = nbrAdded;
            nbrAdded++;
         }
      }
      m_packetsReadyToSend.add( new PacketContainer( p, 0, 0, 
                                                     MODULE_TYPE_MAP ) ); 
   }

   m_nbrCoordinatePackets = m_mapIDs.getSize();
   mc2dbg2 << "[COIO]: m_nbrCoordinatePackets = " << m_nbrCoordinatePackets
          << endl;
   m_nbrReceivedCoordinatePackets = 0;
}

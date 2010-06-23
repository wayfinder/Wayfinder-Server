/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapModuleCom.h"

#include "DatagramSocket.h"
#include "multicast.h"
#include "AllMapPacket.h"
#include "MapPacket.h"
#include "DisturbanceElement.h"
#include "IPnPort.h"
#include "NetUtility.h"
#include "Properties.h"
#include "DataBuffer.h"
#include "TCPSocket.h"
#include "ModuleMap.h"

namespace MapModuleCom {

MC2BoundingBox getBBoxForMap( uint32 mapID, uint32 listenPort ) {
   DatagramReceiver 
      receiver( MultiCastProperties::changeMapSetPort( listenPort ),
                DatagramReceiver::FINDFREEPORT );
   uint32 mapip = MultiCastProperties::getNumericIP( MODULE_TYPE_MAP, true );
   uint16 mapport = MultiCastProperties::getPort( MODULE_TYPE_MAP, true );
   uint32 mapSet = Properties::getMapSet();
   
   if ( mapSet != MAX_UINT32 ) {
      // code also exists in PacketContainer.cpp and
      // ModuleMap.cpp, move to utility function?
      IPnPort before( mapip, mapport );
      IPnPort newaddr = MultiCastProperties::
         changeMapSetAddr( IPnPort( mapip, mapport ) );

      mapip = newaddr.getIP();
      mapport = newaddr.getPort();
      mc2dbg << "[getBBoxForMap] Changed map module addr from "
             << before << " -> " << newaddr
             << " because mapSet = " << mapSet;
   }

   Packet _pack( MAX_PACKET_SIZE ); // For receiving the mapreply
   DatagramSender sock;


   const int waittime = 1000000;
   uint32 status = StringTable::NOT;
   const int maxRetries = 5;
   int nbrRetries = 0;
   while ( status != StringTable::OK && nbrRetries++ <= maxRetries ) {      
      AllMapRequestPacket reqpack( Packet::RequestID( 1 ),
                                   Packet::PacketID( 1 ),
                                   AllMapRequestPacket::BOUNDINGBOX );
      
      reqpack.setOriginIP( NetUtility::getLocalIP() );
      reqpack.setOriginPort( receiver.getPort() );
      reqpack.setResendNbr((byte) nbrRetries-1);
      
      // Send request to open TCP connection between local and mapmodule
      
      if ( ! sock.send( &reqpack, mapip, mapport ) ) {
         mc2log << error << "[getBBoxForMap] could not send "
                << " AllMapRequestPacket - retrying." << endl;
         continue; // Go another round in the loop.
      }
      
      // Receive packet with ip and port to a mapModule
      if ( ! receiver.receive( &_pack, waittime ) ) {
         mc2log << error << "[getBBoxForMap] error receiving ack - retrying."
                << endl;
         continue; // Go another round in the loop.
      }
      
      if ( _pack.getSubType() != Packet::PACKETTYPE_ALLMAPREPLY ) {
         mc2log << error << "[getBBoxForMap] Got packet with subtype "
                << _pack.getSubTypeAsString()
                << " when expecting allmapreply." << endl;
         continue; // Please try again.
      }
      
      AllMapReplyPacket* pack = static_cast<AllMapReplyPacket *>( &_pack );
      
      status = pack->getStatusCode();
      

      uint32 myMap = 0;
      while ( myMap < pack->getNbrMaps() &&
              mapID != pack->getMapID( myMap ) ){
         ++myMap;
      }

      if ( myMap != pack->getNbrMaps() ) {

         mc2dbg << "[getBBoxForMap] map "
                << prettyMapID(mapID) << " index " << myMap
                << " found " << prettyMapID( pack->getMapID( myMap ) ) << endl;
         MC2BoundingBox bbox;
         pack->setMC2BoundingBox( myMap, &bbox );

         return bbox;

      } else {
         mc2log << warn << "[getBBoxForMap] Map " << prettyMapID( mapID ) 
                << " not found in AllMapReplyPacket." << endl;
      }
   }

   return MC2BoundingBox();
}


bool getDisturbanceNodes( uint32 mapID,
                          vector<DisturbanceElement*>& disturbances ) {
   // nothing to read
   if ( disturbances.empty() ) {
      return true;
   }

   char handshake[1024];
   uint32 IP = NetUtility::getLocalIP();
   sprintf(handshake, "InfoModule ready to roll out! (%u.%u.%u.%u)", 
           uint32((IP & 0xff000000) >> 24),
           uint32((IP & 0x00ff0000) >> 16),
           uint32((IP & 0x0000ff00) >>  8),
           uint32(IP & 0x000000ff));
   
   //set up a socket to the MapModule so that we can send a MAPREQUEST_INFO
   auto_ptr<TCPSocket>
      mapSocket( ModuleMap::
                 getMapLoadingSocket( mapID,
                                      MapRequestPacket::MAPREQUEST_INFO,
                                      handshake,
                                      0,
                                      NULL ) );

   if ( mapSocket.get() == NULL ) {
      mc2log << error
             << "[getDisturbanceNodes] ModuleMap::getMapLoadingSocket( "
             << MC2HEX( mapID )
             << ") failed. Aborting loadmap " << endl;
      return false;
   }

   uint32 nbrCoordinates = 0;
   // TODO: use std::accumulate
   for ( vector<DisturbanceElement*>::const_iterator 
            it = disturbances.begin(), itEnd = disturbances.end(); 
          it != itEnd; it++) {
      nbrCoordinates += (*it)->getNbrCoordinates();
   }

   // New scope
   // Send the lat/lon/angles to the mapmodule and receive the
   // itemID:s
   // Question contains 4 bytes lat
   //                   4 bytes lon
   //                   4 bytes angle 0-359
   
   const int qSize = nbrCoordinates * 12;
   DataBuffer qBuf( qSize );
   
   const int ansSize = nbrCoordinates * 20;
   DataBuffer ansBuf( ansSize );
   
   DataBuffer sizeBuf( 4 );
   sizeBuf.writeNextLong( nbrCoordinates );
   int keept = 0;
   int removed = 0;

   if ( mapSocket->writeAll( sizeBuf.getBufferAddress(), 4 ) != 4 ) {
      // Error
      mc2log << warn << "[getDisturbanceNodes] Error writing on mapsocket."
             << endl;
      return false;
   }

   sizeBuf.reset();

   typedef DisturbanceElement::RouteIndex2CoordMap RouteIndex2CoordMap;
   typedef DisturbanceElement::RouteIndex2AngleMap RouteIndex2AngleMap;
   typedef DisturbanceElement::RouteIndex RouteIndex;

   for ( vector<DisturbanceElement*>::iterator 
            it = disturbances.begin(), itEnd = disturbances.end();
         it != itEnd; ++it ) {
      DisturbanceElement* dist = *it;
      const RouteIndex& indexVector = dist->getRouteIndex();
      const RouteIndex2CoordMap& latVector = dist->getLatMap();
      const RouteIndex2CoordMap& lonVector = dist->getLonMap();
      const RouteIndex2AngleMap& angleVector = dist->getAngle();
      
      RouteIndex::const_iterator indexIt;
      RouteIndex2CoordMap::const_iterator latlonIt;
      RouteIndex2AngleMap::const_iterator angleIt;
      
      for ( indexIt = indexVector.begin(); indexIt != indexVector.end();
            ++indexIt ) {
         uint32 index = *indexIt;
         latlonIt = latVector.find( index );
         int32 lat = latlonIt->second;
         latlonIt = lonVector.find( index );
         int32 lon = latlonIt->second;
         angleIt = angleVector.find( index );
         uint32 angle = angleIt->second;
         qBuf.writeNextLong( lat );
         qBuf.writeNextLong( lon );
         qBuf.writeNextLong( angle );
      }
   }

   if ( mapSocket->writeAll( qBuf.getBufferAddress(),
                             qSize) != qSize ) {
      // Error
      mc2log << warn
             << "[getDisturbanceNodes] Error writing on mapsocket" << endl;
      return false;
   }

   qBuf.reset();
   ansBuf.reset();

   if ( mapSocket->readExactly( ansBuf.getBufferAddress(),
                                ansSize ) != ansSize ) {
      // Error
      mc2log << warn << "[getDisturbanceNodes] Error reading on mapsocket"
             << endl;
      return false;
   }

   // FIXME: Set the values and remove the ones with too large
   //        distances.
   vector<DisturbanceElement*>::iterator it = disturbances.begin();
   while ( it != disturbances.end() ) {
      DisturbanceElement* dist = *it;
      const RouteIndex& indexVector = dist->getRouteIndex();
      const RouteIndex2CoordMap& latVector = dist->getLatMap();
      const RouteIndex2CoordMap& lonVector = dist->getLonMap();
      const RouteIndex2AngleMap& angleVector = dist->getAngle();

      RouteIndex2CoordMap::const_iterator latlonIt;
      RouteIndex2AngleMap::const_iterator angleIt;
      const uint32 nbrCoordinates = dist->getNbrCoordinates();
      for ( uint32 i = 0; i < nbrCoordinates; ++i ) {
         const uint32 index    = indexVector[ i ];
         const uint32 mapID    = ansBuf.readNextLong();
         const uint32 nodeID_0 = ansBuf.readNextLong();
         const uint32 nodeID_1 = ansBuf.readNextLong();
         const uint32 distance = ansBuf.readNextLong();
         ansBuf.readNextLong(); // offset

         latlonIt = latVector.find( index );
         const int32 lat = latlonIt->second;
         latlonIt = lonVector.find( index );
         const int32 lon = latlonIt->second;
         angleIt = angleVector.find( index );
         const uint32 angle = angleIt->second;

         // less than 20 meters?, for some unknown reason.
         if ( distance < 20 ) {
            dist->addNodeID( nodeID_0, index );
            dist->setMapID( mapID );
            if( angle == 32767 ) { // 7FFF
               dist->addCoordinate( nodeID_1,
                                    lat, lon,
                                    MAX_UINT32,
                                    dist->getNbrCoordinates() );
            }
            keept++;
         } else {
            dist->removeIndex( index );
            removed++;
         }
      }

      if ( dist->getNbrCoordinates() > 0 ) {
         ++it;
      } else {
         // no coordinates, we dont need an empty disturbance
         delete *it;
         it = disturbances.erase( it );
      }
   }

   return true;
}

} // MapModuleCom

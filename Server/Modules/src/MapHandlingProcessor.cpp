/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapHandlingProcessor.h"

#include "MapSafeVector.h"
#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "SystemPackets.h" 

MapHandlingProcessor::MapHandlingProcessor( MapSafeVector* loadedMaps ):
   Processor( loadedMaps ),
   m_loadedMaps( loadedMaps ) {
   // initial state is "job ended"
   m_loadedMaps->setJobThreadEnd();
}

MapHandlingProcessor::~MapHandlingProcessor() {

}

Packet* MapHandlingProcessor::
handleSpecialPacket( const RequestPacket& p, char* packetInfo ) {

   Packet* pack = Processor::handleSpecialPacket( p, packetInfo );
   if ( pack != NULL ) {
      return pack;
   }

   ReplyPacket* reply = NULL;

   switch ( p.getSubType() ) {
      case Packet::PACKETTYPE_LOADMAPREQUEST: {
         uint32 mapSize = 0; // Will change or not
         mc2log << info
                << "Processor got LoadMapRequestPacket for map 0x"
                << hex << p.getMapID() << dec
                << endl;
         StringTable::stringCode status = StringTable::OK;
         uint32 mapID = p.getMapID();
         if( !startLoadingMap(mapID) ) {
            status = StringTable::ERROR_MAP_LOADED;
         } else {
            // Load the map using the subclass.
            status = loadMap( mapID, mapSize );
            if ( status != StringTable::OK &&
                 status != StringTable::ERROR_MAP_LOADED ) {
               // Failed - remove the map.
               cancelLoadingMap(mapID);
            } else {
               // Made it!
               // Avoid updating the size if the map wasn't loaded
               // for the first time               
               mc2dbg4 << "[Processor]: MapSize is = "
                       << mapSize << endl;
               // Update size.
               finishLoadingMap( mapID, mapSize);
            }
         }
         reply = new LoadMapReplyPacket(
            static_cast<const LoadMapRequestPacket&>(p),
            status,
            mapSize );
                                       
      }
      break;
      case Packet::PACKETTYPE_DELETEMAPREQUEST :
      {
         mc2log << info << "Processor got DeleteMapRequestPacket for map 0x"
                << hex << p.getMapID() << dec << endl;
         StringTable::stringCode status = StringTable::OK;
         uint32 mapID = p.getMapID();
         // Update status for Supervisor and friends.
         m_loadedMaps->setStatus(mapID,
                                 MapElement::DELETING );
         status = deleteMap(mapID);
         if( status == StringTable::OK ) {
            MapHandlingProcessor::removeMap( mapID );
         }
         reply = new DeleteMapReplyPacket(
            static_cast<const DeleteMapRequestPacket&>( p ),
            status);
      }
      break;

      case Packet::PACKETTYPE_LOADMAPREPLY:
      case Packet::PACKETTYPE_DELETEMAPREPLY:
      case Packet::PACKETTYPE_ACKNOWLEDGE:
      {
         mc2dbg << "[Proc]: Received ack/load/deletemapreply. Strange"
                << endl;
         reply = new AcknowledgeRequestReplyPacket( &p,
                                                    StringTable::NOTFOUND,
                                                    ACKTIME_NOMAP );
         // Make sure no-one gets the packet, but that the processor
         // won't do anything
         reply->setOriginIP( 0 );
         reply->setOriginPort( 0 );         
      }
      break;
      default: {
         if ( m_loadedMaps == NULL ) {
            // Generate Map Server?            
            return NULL;
         }
         // Check if map is loaded first.
         // Will this work for pushpackets?
         uint32 mapID = p.getMapID();
         // Update last use.
         m_loadedMaps->updateLastUse( mapID );
         // Origin IP should be zero for pushpackets.
         // MapID should be MAX_UINT32 if map id is not important.
         if ( ( mapID != MAX_UINT32 ) && ( p.getOriginIP() != 0 ) ) {
            // Check if loaded
            if ( ! m_loadedMaps->isMapLoaded( mapID ) ) {
               mc2dbg << "[Proc]: Map "
                      << p.getMapID()
                      << " is not loaded - sending ack"
                      << endl;
               reply = new AcknowledgeRequestReplyPacket( &p,
                                                          StringTable::OK,
                                                          ACKTIME_NOMAP );

               // This debug printout is here to confirm that there was
               // an error which occured when the Reader had marked a
               // map as TOLD_TO_LOAD after receiving a load map request
               // and the Processor already had a packet for that map in
               // the queue. (I.e. the order was reversed, loading failed
               // and packets resent etc.)
               if ( m_loadedMaps->isMapLoadedOrLoading( mapID ) ) {
                  mc2dbg << "[Proc]: Reader has received loadmap packet for "
                         << " map 0x" << hex << mapID << dec 
                         << " but not Processor ZX81" << endl;
               }
            }
         } else {
            // Packet sent to subclass Processor,
            // as its not a LoadMapRequest.
            mc2dbg2 << "Processor::handleReq, sending type "
                    << p.getSubType()
                    << " to subclass" <<endl;            
         }
      }
   }

   return reply;
}

void
MapHandlingProcessor::removeMap(uint32 mapID)
{
   mc2dbg2 << "Processor::removeMap(" << mapID << ")" << endl;
   m_loadedMaps->removeMap(mapID);
}

bool
MapHandlingProcessor::startLoadingMap(uint32 mapID)
{
   return m_loadedMaps->setStatus(mapID, MapElement::LOADING);
}


bool MapHandlingProcessor::cancelLoadingMap( uint32 mapID ) {
   return m_loadedMaps->setStatus(mapID, MapElement::DELETED);
}

bool MapHandlingProcessor::finishLoadingMap( uint32 mapID, uint32 size ) {
   return m_loadedMaps->finishLoadingMap(mapID, size);
}


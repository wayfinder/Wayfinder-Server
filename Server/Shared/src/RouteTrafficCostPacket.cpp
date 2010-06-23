/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "RouteTrafficCostPacket.h"
#include "DisturbanceList.h"
#include "UserRightsMapInfo.h"

#define ROUTETRAFFICCOSTREQUESTPACKET_MAX_LENGTH 65536
#define ROUTETRAFFICCOSTREQUESTPACKET_PRIO       DEFAULT_PACKET_PRIO

int
RouteTrafficCostRequestPacket::calcPacketSize( const UserRightsMapInfo& rights)
{
   // REQUEST_HEADER_SIZE + userDef + rights
   return REQUEST_HEADER_SIZE + 4 + rights.getSizeInPacket();
}

RouteTrafficCostRequestPacket::
RouteTrafficCostRequestPacket( const UserUser* user,
                               uint32 mapID,
                               uint32 userDefined )
      : RequestPacket( ROUTETRAFFICCOSTREQUESTPACKET_MAX_LENGTH,
                       ROUTETRAFFICCOSTREQUESTPACKET_PRIO,
                       Packet::PACKETTYPE_ROUTETRAFFICCOSTREQUEST,
                       Packet::NO_PACKET_ID,
                       0, // RequestID
                       mapID )
{
   // Rights should maybe be sent in from the beginning.
   MapRights mask( MapRights::TRAFFIC_AND_SPEEDCAM );
   
   UserRightsMapInfo rights( mapID, user, mask );
   uint32 minSize = calcPacketSize( rights );
   if ( minSize > getBufSize() ) {
      Packet::resize( minSize );
   }
   int pos = USER_DEF_POS;
   incWriteLong( pos, userDefined );
   MC2_ASSERT( pos == RIGHTS_POS );
   rights.save( this, pos );
   // FIXME: Write enough data for UserUser
   setLength( pos );
}

uint32
RouteTrafficCostRequestPacket::getUserDefData() const
{
   return readLong( USER_DEF_POS );
}

int
RouteTrafficCostRequestPacket::getRights( UserRightsMapInfo& rights ) const
{
   int pos = RIGHTS_POS;
   return rights.load( this, pos );
}


int
RouteTrafficCostReplyPacket::
calcPacketSize(uint32 nbrDists)
{
   return REQUEST_HEADER_SIZE + 8 + 4 + nbrDists * 8;
}

RouteTrafficCostReplyPacket::
RouteTrafficCostReplyPacket( const RouteTrafficCostRequestPacket* req,
                             uint32 status,
                             const vector<pair<uint32, uint32> >& dists )
      : ReplyPacket( calcPacketSize(dists.size()),
                     Packet::PACKETTYPE_ROUTETRAFFICCOSTREPLY,
                     req,
                     status )
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, req->getMapID() );
   incWriteLong( pos, req->getUserDefData() );

   MC2_ASSERT( NBR_TRAFFIC_DATA_POS == pos );

   // Write length and then vector
   incWriteLong( pos, dists.size() );   
   for ( vector<pair<uint32, uint32> >::const_iterator it = dists.begin();
         it != dists.end();
         ++it ) {
      incWriteLong( pos, it->first );
      incWriteLong( pos, it->second );
   }
   setLength( pos );
}

uint32
RouteTrafficCostReplyPacket::getUserDefData() const
{
   return readLong( USER_DEF_POS );
}


uint32
RouteTrafficCostReplyPacket::getMapID() const
{
   return readLong( MAP_ID_POS );
}

uint32
RouteTrafficCostReplyPacket::
getTraffic( DisturbanceVector& resultVect, bool costC ) const
{
   uint32 mapID = getMapID();
   int pos = NBR_TRAFFIC_DATA_POS;
   int size = incReadLong( pos );
   resultVect.reserve( resultVect.size() + size );
   for ( int i = 0; i < size; ++i ) {
      uint32 nodeID = incReadLong( pos );
      uint32 rawFactor = incReadLong( pos );
      // Do not block nodes when using costB. RM will block for all.
      if ( rawFactor != MAX_UINT32 || costC ) {
         resultVect.push_back(
            new DisturbanceListElement( mapID,
                                        nodeID,
                                        rawFactor,
                                        true,
                                        0 ) );
      }
                                     
   }
   return size;
}

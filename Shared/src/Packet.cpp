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

#include "Packet.h"
#include <iostream>
#include <stdio.h>
#include "StringTable.h"
#include "DataBuffer.h"
#include "Properties.h"
#include "IPnPort.h"
#include "PacketDump.h"

#ifdef _WIN32
   #define TRACE printf
#endif


const uint32 
Packet::m_nbrPriorities = 16;

const Packet::RequestID Packet::MAX_REQUEST_ID = MAX_UINT16;
const Packet::MapID Packet::MAX_MAP_ID = MAX_UINT32;

Packet::Packet(uint32 bufLength):
   m_arrivalTime( 0 )
{
   m_deleteBuffer = true;
   // Cannot allocate Packet smaller than the header, please
   MC2_ASSERT( bufLength >= HEADER_SIZE );
  
   this->bufSize = bufLength;
   this->length = HEADER_SIZE;
   // Make an aligned buffer
   buffer = MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufLength );
   // Zero the header (valgrind will be happier)
   memset(buffer, 0, bufLength/*HEADER_SIZE*/);
   
   setPacketTag(DEFAULT_PACKET_TAG);
   setPacketNbr( 0 );
   setResendNbr( 0 );
   setNbrPackets( 1 );
   setMapSet( MAX_UINT32 );
   setRequestTag(0);
   setProtocolVersion( 1, 0 );
   //setPacketID( NO_PACKET_ID );
}

Packet::Packet(byte* buf, uint32 bufLength, bool nodelete ):
   m_arrivalTime( 0 )
{
   this->bufSize = bufLength;
   this->length = bufLength;
   m_deleteBuffer = !nodelete;
   buffer = buf;
   if ( m_deleteBuffer ) {
      setPacketTag(DEFAULT_PACKET_TAG);
      setProtocolVersion( 1, 0 );
      //setPacketID( NO_PACKET_ID );
   }   
}

Packet::Packet( uint32 bufLength,
                byte prio,
                uint16 subType,
                PacketID packetId,
                RequestID requestID ):
   m_arrivalTime( 0 )
{
   // Cannot allocate Packet smaller than the header, please
   MC2_ASSERT( bufLength >= HEADER_SIZE );
   m_deleteBuffer = true;
   buffer = MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufLength );
   
   // Zero the header (valgrind will be happier)
   memset(buffer, 0, bufLength/*HEADER_SIZE*/);
   
   this->bufSize = bufLength;
   this->length = HEADER_SIZE;
   setPacketTag(DEFAULT_PACKET_TAG);
   setSubType(subType);
   setPriority(prio);
   setPacketID(packetId);
   setRequestID(requestID );
   setPacketNbr( 0 );
   setResendNbr( 0 );
   setNbrPackets( 1 );
   setProtocolVersion( 1, 0 );
   setMapSet( MAX_UINT32 );
   setRequestTag(0);
}
      
Packet::Packet(   uint32 bufLength,
                  byte prio,
                  uint16 subType,
                  uint32 ip,
                  uint16 port,
                  uint16 packetId,
                  uint32 deb        ):
   m_arrivalTime( 0 )
{
   // Cannot allocate Packet smaller than the header, please
   MC2_ASSERT( bufLength >= HEADER_SIZE );
   m_deleteBuffer = true;
   // Make a longword-aligned buffer
   buffer = MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufLength );
   // Zero the header (valgrind will be happier)
   memset(buffer, 0, HEADER_SIZE);
   this->bufSize = bufLength;
   this->length = HEADER_SIZE;
   setPacketTag(DEFAULT_PACKET_TAG);
   setSubType(subType);
   setPriority(prio);
   setOriginIP(ip);
   setOriginPort(port);
   setPacketID(packetId);
   setDebInfo(deb);
   setPacketNbr( 0 );
   setResendNbr( 0 );
   setNbrPackets( 1 );
   setProtocolVersion( 1, 0 );
   setMapSet( MAX_UINT32 );
   setRequestTag(0);
} 

Packet::Packet(   uint32 bufLength,
                  byte prio,
                  uint16 subType,
                  uint32 ip,
                  uint16 port,
                  uint16 packetId,
                  uint16 requestId,
                  uint32 deb        ):
   m_arrivalTime( 0 )
{
   // Cannot allocate Packet smaller than the header, please
   MC2_ASSERT( bufLength >= HEADER_SIZE );
   m_deleteBuffer = true;
   // Make a longword-aligned buffer
   buffer = MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufLength );
   // Zero the header (valgrind will be happier)
   memset(buffer, 0, HEADER_SIZE);
   this->bufSize = bufLength;
   this->length = HEADER_SIZE;
   setPacketTag(DEFAULT_PACKET_TAG);
   setSubType(subType);
   setPriority(prio);
   setOriginIP(ip);
   setOriginPort(port);
   setRequestID(requestId);
   setPacketID(packetId);
   setDebInfo(deb);
   setPacketNbr( 0 );
   setResendNbr( 0 );
   setNbrPackets( 1 );
   setProtocolVersion( 1, 0 );
   setMapSet( MAX_UINT32 );
   setRequestTag(0);
} 


Packet*
Packet::makePacket( byte* buff, uint32 buffLength ) {
   Packet* p = new Packet( buff, buffLength );
   p->setArrivalTime();
   return p;
}

void
Packet::copyRequestInfoFrom( const Packet* other )
{
   if ( other != NULL ) {
      setRequestID( other->getRequestID() );
      setRequestOriginator( other->getRequestOriginator() );
      setRequestTimestamp( other->getRequestTimestamp() );
   }
}

const uint32 
Packet::MAX_UDP_PACKET_SIZE = 65000;

uint32
Packet::getTCPLimitSize() 
{
   return Properties::getUint32Property( "TCP_LIMIT_SIZE",
                                         30000 );
}

Packet::~Packet() 
{
   if ( m_deleteBuffer ) {
      delete [] (uint32 *)buffer;
   }
}

const char* 
Packet::getSubTypeAsString() const
{
   packetType typ = getSubType();
   
   switch( typ ) {

      // CTRL-packets
      case PACKETTYPE_HEARTBEAT:
         return "HeartBeat";
      case PACKETTYPE_VOTE:
         return "Vote";
      case PACKETTYPE_IGNORE:
         return "Ignore";
      case PACKETTYPE_STATISTICS:
         return "Statistics";
      case PACKETTYPE_ALLMAPREQUEST:
         return "AllMapReq";
      case PACKETTYPE_ALLMAPREPLY:
         return "AllMapReply";
      case PACKETTYPE_LOADMAPREPLY:
         return "LoadMap reply";
      case PACKETTYPE_SUBROUTEREPLY:
         return "SR reply";
      case PACKETTYPE_USERADDREQUEST:
         return "Add user req";
      case PACKETTYPE_USERADDREPLY:
         return "Add user reply";
      case PACKETTYPE_USERDELETEREQUEST:
         return "Delete user req";
      case PACKETTYPE_USERDELETEREPLY:
         return "Delete user reply";
      case PACKETTYPE_USERCHANGEREQUEST:
         return "Change user req";
      case PACKETTYPE_USERCHANGEREPLY:
         return "Change user reply";
      case PACKETTYPE_ADDCELLULARPHONEMODELREQUESTPACKET:
         return "Add cellular model req";
      case PACKETTYPE_ADDCELLULARPHONEMODELREPLYPACKET:
         return "Add cellular model reply";
      case PACKETTYPE_CHANGEUSERPASSWORDREQUESTPACKET:
         return "Change user password req";
      case PACKETTYPE_CHANGEUSERPASSWORDREPLYPACKET:
         return "Change user password reply";
      case PACKETTYPE_CHANGECELLULARPHONEMODELREQUESTPACKET:
         return "Change cellular model req";   
      case PACKETTYPE_CHANGECELLULARPHONEMODELREPLYPACKET:
         return "Change cellular model reply";
      case PACKETTYPE_LISTDEBITREQUESTPACKET:
         return "List debit requestpacket";   
      case PACKETTYPE_LISTDEBITREPLYPACKET:
         return "List debit replypacket";
      case PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REQUEST:
         return "RouteStorage add route req";
      case PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REPLY:
         return "RouteStorage add route reply";
      case PACKETTYPE_USER_CREATE_SESSION_REQUEST:
         return "User create session req";
      case PACKETTYPE_USER_CREATE_SESSION_REPLY:
         return "User create session reply";
         
      case PACKETTYPE_TRAFFICPOINTREQUESTPACKET:
         return "Traffic point request";
         
      // "Ordinary" packets
      case PACKETTYPE_ACKNOWLEDGE:
         return "Acknowledge";
      case PACKETTYPE_SUBROUTE:
         return "SubRoute";
      case PACKETTYPE_TESTREQUEST:
         return "TestReq";
      case PACKETTYPE_TESTREPLY:
         return "TestReply";
      case PACKETTYPE_GETCELLULARPHONEMODELDATAREQUEST:
         return "GETCELLULARPHONEMODELDATAREQ";
      case PACKETTYPE_GETCELLULARPHONEMODELDATAREPLY:
         return "GETCELLULARPHONEMODELDATAREPLY";
      case PACKETTYPE_DELETEMAPREQUEST:
         return "DELETEMAPREQ";
      case PACKETTYPE_DELETEMAPREPLY:
         return "DELETEMAPREPLY";
      case PACKETTYPE_PROXIMITYPOSITIONREQUESTPACKET:
         return "PROXIMITYPOSITIONREQ";
      case PACKETTYPE_PROXIMITYITEMSETREQUESTPACKET:
         return "PROXIMITYITEMSETREQ";
      case PACKETTYPE_PROXIMITYREPLYPACKET:
         return "PROXIMITYREPLY";
      case PACKETTYPE_COVEREDIDSREPLYPACKET:
         return "COVEREDIDSREPLY";
      case PACKETTYPE_PROXIMITYSEARCHREPLY:
         return "PROXIMITYSEARCHREPLY";
      case PACKETTYPE_SMSLISTENREPLY:
         return "SMSLISTENREPLY";
      case PACKETTYPE_SMSLISTENREQUEST:
         return "SMSLISTENREQ";
      case PACKETTYPE_COORDINATEONITEMREQUEST:
         return "COORDINATEONITEMREQ";
      case PACKETTYPE_COORDINATEONITEMREPLY:
         return "COORDINATEONITEMREPLY";
      case PACKETTYPE_EXPANDITEMREQUEST:
         return "EXPANDITEMREQ";
      case PACKETTYPE_EXPANDITEMREPLY:
         return "EXPANDITEMREPLY";
      case PACKETTYPE_SEARCHSELECTIONREPORT:
         return "SEARCHSELECTIONREPORT";
      case PACKETTYPE_SENDEMAILREQUEST:
         return "SENDEMAILREQ";
      case PACKETTYPE_SENDEMAILREPLY:
         return "SENDEMAILREPLY";
      case PACKETTYPE_UPDATETRAFFICCOSTREPLY:
         return "UPDATETRAFFICCOSTREPLY";
      case PACKETTYPE_ADDDISTURBANCEREQUEST:
         return "ADDDISTURBANCEREQ";
      case PACKETTYPE_ADDDISTURBANCEREPLY:
         return "ADDDISTURBANCEREPLY";
      case PACKETTYPE_REMOVEDISTURBANCEREQUEST:
         return "REMOVEDISTURBANCEREQ";
      case PACKETTYPE_REMOVEDISTURBANCEREPLY:
         return "REMOVEDISTURBANCEREPLY";
      case PACKETTYPE_UPDATETRAFFICCOSTREQUEST:
         return "UPDATETRAFFICCOSTREQ";
      case PACKETTYPE_DISTURBANCESUBSCRIPTIONREQUEST:
         return "DISTURBANCESUBSCRIPTIONREQ";
      case PACKETTYPE_DISTURBANCESUBSCRIPTIONREPLY:
         return "DISTURBANCESUBSCRIPTIONREPLY";
      case PACKETTYPE_GETUSERNAVDESTINATIONREQUESTPACKET:
         return "GETUSERNAVDESTINATIONREQ";
      case PACKETTYPE_GETUSERNAVDESTINATIONREPLYPACKET:
         return "GETUSERNAVDESTINATIONREPLY";
      case PACKETTYPE_SORTDISTREQUEST:
         return "SORTDISTREQ";
      case PACKETTYPE_SORTDISTREPLY:
         return "SORTDISTREPLY";
      case PACKETTYPE_GETTMCCOORREQUEST:
         return "GETTMCCOORREQ";
      case PACKETTYPE_GETTMCCOORREPLY:
         return "GETTMCCOORREPLY";
      case PACKETTYPE_SMSSTATISTICS:
         return "SMSSTATISTICS";
      case PACKETTYPE_ADDUSERNAVDESTINATIONREQUESTPACKET:
         return "ADDUSERNAVDESTINATIONREQ";
      case PACKETTYPE_ADDUSERNAVDESTINATIONREPLYPACKET:
         return "ADDUSERNAVDESTINATIONREPLY";
      case PACKETTYPE_DELETEUSERNAVDESTINATIONREQUESTPACKET:
         return "DELETEUSERNAVDESTINATIONREQ";
      case PACKETTYPE_DELETEUSERNAVDESTINATIONREPLYPACKET:
         return "DELETEUSERNAVDESTINATIONREPLY";
      case PACKETTYPE_CHANGEUSERNAVDESTINATIONREQUESTPACKET:
         return "CHANGEUSERNAVDESTINATIONREQ";
      case PACKETTYPE_CHANGEUSERNAVDESTINATIONREPLYPACKET:
         return "CHANGEUSERNAVDESTINATIONREPLY";
      case PACKETTYPE_ALLCOUNTRYREQUEST:
         return "ALLCOUNTRYREQ";
      case PACKETTYPE_ALLCOUNTRYREPLY:
         return "ALLCOUNTRYREPLY";
      case PACKETTYPE_COVEREDIDSREQUESTPACKET:
         return "COVEREDIDSREQ";
      case PACKETTYPE_MAPREQUEST:
         return "Map req";
      case PACKETTYPE_MAPREPLY:
         return "Map reply";
      case PACKETTYPE_ROUTEREPLY:
         return "Route reply";
      case PACKETTYPE_VANILLASEARCHREQUEST:
         return "Vanilla search req";
      case PACKETTYPE_VANILLASEARCHREPLY:
         return "Vanilla search reply ";
      case PACKETTYPE_USERSEARCHREQUEST:
         return "User search req";
      case PACKETTYPE_USERSEARCHREPLY:
         return "User search reply";
      case PACKETTYPE_EXPANDCATEGORY2SEARCHREQUEST:
         return "Expand category 2 search req";
      case PACKETTYPE_USER_AUTH_REQUEST:
         return "USER_AUTH_REQ";
      case PACKETTYPE_USER_AUTH_REPLY:
         return "USER_AUTH_REPLY";
      case PACKETTYPE_ROUTEEXPANDITEMREQUEST:
         return "ROUTEEXPANDITEMREQ";
      case PACKETTYPE_ROUTEEXPANDITEMREPLY:
         return "ROUTEEXPANDITEMREPLY";
      case PACKETTYPE_EXPANDROUTEREQUEST:
         return "Expand route req";
      case PACKETTYPE_EXPANDROUTEREPLY:
         return "Expand route reply";
      case PACKETTYPE_LOADMAPREQUEST:
         return "LoadMap req";
      case PACKETTYPE_USERGETDATAREQUEST:
         return "User get data req";
      case PACKETTYPE_USERGETDATAREPLY:
         return "User get data reply";
      case PACKETTYPE_USERVERIFYREQUEST:
         return "User verify req";
      case PACKETTYPE_USERVERIFYREPLY:
         return "User verify reply";
      case PACKETTYPE_USERLOGOUTREQUEST:
         return "User logout req";
      case PACKETTYPE_USERLOGOUTREPLY:
         return "User logout reply";
      case PACKETTYPE_USERSESSIONCLEANUPREQUEST:
         return "User session cleanup req";
      case PACKETTYPE_USERSESSIONCLEANUPREPLY:
         return "User session cleanup reply";
      case PACKETTYPE_USERCHECKPASSWORDREQUEST:
         return "User check password req";
      case PACKETTYPE_USERCHECKPASSWORDREPLY:
         return "User check password reply";
      case PACKETTYPE_USERFINDREQUEST:
         return "User find req";
      case PACKETTYPE_USERFINDREPLY:
         return "User find reply";
      case PACKETTYPE_COORDINATEREPLY:
         return "Coordinate reply";
      case PACKETTYPE_COORDINATEREQUEST:
         return "Coordinate req";
      case PACKETTYPE_SMSREQUEST:
         return "SMS req";
      case PACKETTYPE_SMSREPLY:
         return "SMS reply";
      case PACKETTYPE_SUBROUTEREQUEST:
         return "SubR req";
      case PACKETTYPE_DEBITREQUEST:
         return "Debit req";
      case PACKETTYPE_DEBITREPLY:
         return "Debit reply";
      case PACKETTYPE_OVERVIEWSEARCHREQUESTPACKET:
         return "Overview search req";
      case PACKETTYPE_OVERVIEWSEARCHREPLYPACKET:
         return "Overview search reply";
      case PACKETTYPE_PROXIMITYSEARCHREQUEST:
         return "Proximity search req";
      case PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REQUEST:
         return "RouteStorage get req";
      case PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REPLY:
         return "RouteStorage get reply";
      case PACKETTYPE_ITEM_NAMES_REQUEST:
         return "ITEM_NAMES_REQ";
      case PACKETTYPE_ITEM_NAMES_REPLY:
         return "ITEM_NAMES_REPLY";
      case PACKETTYPE_ADDMAPUPDATE_REQUEST:
         return "ADDMAPUPDATE_REQ";
      case PACKETTYPE_ADDMAPUPDATE_REPLY:
         return "ADDMAPUPDATE_REPLY";
      case PACKETTYPE_ITEMINFO_REQUEST:
         return "ITEMINFO_REQ";
      case PACKETTYPE_ITEMINFO_REPLY:
         return "ITEMINFO_REPLY";
      case PACKETTYPE_GFXFEATUREMAPREQUEST:
         return "GFXFEATUREMAPREQ";
      case PACKETTYPE_GFXFEATUREMAPREPLY:
         return "GFXFEATUREMAPREPLY";
      case PACKETTYPE_GFXFEATUREMAP_IMAGE_REQUEST:
         return "GFXFEATUREMAP_IMAGE_REQ";
      case PACKETTYPE_GFXFEATUREMAP_IMAGE_REPLY:
         return "GFXFEATUREMAP_IMAGE_REPLY";
      case PACKETTYPE_LEADERIP_REQUEST:
         return "LEADERIP_REQ";
      case PACKETTYPE_LEADERIP_REPLY:
         return "LEADERIP_REPLY";
      case PACKETTYPE_EDGENODESREQUEST:
         return "ENodReq";
      case PACKETTYPE_EDGENODESREPLY:
         return "ENodReply";
      case PACKETTYPE_IDTRANSLATIONREQUEST:
         return "IDTrReq";
      case PACKETTYPE_IDTRANSLATIONREPLY:
         return "IDTrReply";
      case PACKETTYPE_STREET_SEGMENT_ITEMREQUEST:
         return "STREET_SEGMENT_ITEMREQ";
      case PACKETTYPE_STREET_SEGMENT_ITEMREPLY:
         return "STREET_SEGMENT_ITEMREPLY";
      case PACKETTYPE_MAPSUBSCRIPTIONREQUEST:
         return "MapSubscReq";
      case PACKETTYPE_MAPSUBSCRIPTIONHEARTBEATREQUEST:
         return "MapSubscHeartReq";
      case PACKETTYPE_USERFAVORITES_REQUEST:
         return "UFavReq";
      case PACKETTYPE_USERFAVORITES_REPLY:
         return "UFavReply";
      case PACKETTYPE_DISTURBANCEREQUEST:
         return "DisturbanceRequestPacket";
      case PACKETTYPE_DISTURBANCEREPLY:
         return "DisturbanceReplyPacket";
      case PACKETTYPE_DISTURBANCEPUSH:
         return "DisturbancePushPacket";
      case PACKETTYPE_TEST_PUSH:
         return "TestPush";
      case PACKETTYPE_ALIVEREQUEST:
         return "AliveReq";
      case PACKETTYPE_ALIVEREPLY:
         return "AliveReply";
      case PACKETTYPE_TOPREGIONREQUEST:
         return "TopRegionReq";
      case PACKETTYPE_TOPREGIONREPLY:
         return "TopRegionReply";
      case PACKETTYPE_UPDATEDISTURBANCEREQUEST:
         return "UpdateDisturbanceRequest";
      case PACKETTYPE_UPDATEDISTURBANCEREPLY:
         return "UpdateDisturbanceReply";
      case PACKETTYPE_SHUTDOWN:
         return "ShutdownPacket";
      case PACKETTYPE_ADDUSERTRACK_REQUEST:
         return "AddUserTrackRequestPacket";
      case PACKETTYPE_ADDUSERTRACK_REPLY:
         return "AddUserTrackReplyPacket";
      case PACKETTYPE_GETUSERTRACK_REQUEST:
         return "GetUserTrackRequestPacket";
      case PACKETTYPE_GETUSERTRACK_REPLY:
         return "GetUserTrackReplyPacket";
      case PACKETTYPE_SMALLESTROUTINGCOSTREQUEST:
         return "SmallestRoutingCostRequestPacket";
      case PACKETTYPE_SMALLESTROUTINGCOSTREPLY:
         return "SmallestRoutingCostReplyPacket";
      case PACKETTYPE_ROUTINGINFOREQUEST:
         return "RIReq";
      case PACKETTYPE_ROUTINGINFOREPLY:
         return "RIReply";
      case PACKETTYPE_SEARCHEXPANDITEMREQUEST:
         return "SEIReq";
      case PACKETTYPE_SEARCHEXPANDITEMREPLY:
         return "SEIReply";
      case PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REQUEST:
         return "RouteStorage change route req";
      case PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REPLY:
         return "RouteStorage change route reply";
      case PACKETTYPE_MATCHINFOREQUEST:
         return "MatchInfoReq";
      case PACKETTYPE_MATCHINFOREPLY:
         return "MatchInfoReply";
      case PACKETTYPE_SEARCHREQUEST:
         return "SearchReq";
      case PACKETTYPE_TRANSACTION_REQUEST:
         return "Transaction request";
      case PACKETTYPE_TRANSACTION_REPLY:
         return "Transaction reply";
      case PACKETTYPE_TRANSACTION_DAYS_REQUEST:
         return "Transaction days request";
      case PACKETTYPE_TRANSACTION_DAYS_REPLY:
         return "Transaction days reply";
      case PACKETTYPE_WFACTIVATIONREQUEST:
         return "WFActiReq";
      case PACKETTYPE_WFACTIVATIONREPLY:
         return "WFActiReply";
      case PACKETTYPE_QUEUESTARTREQUEST:
         return "QStartReq";
      case PACKETTYPE_QUEUESTARTREPLY:
         return "QStartReply";
      case PACKETTYPE_DISTURBANCEINFOREQUEST:
         return "DistInfoReq";
      case PACKETTYPE_DISTURBANCEINFOREPLY:
         return "DistInfoReply";
      case PACKETTYPE_ROUTETRAFFICCOSTREQUEST:
         return "RouteTrafficCostReq";
      case PACKETTYPE_ROUTETRAFFICCOSTREPLY:
         return "RouteTrafficCostReply";
      case PACKETTYPE_EXTERNALSEARCH_REQUEST:
         return "ExtSearchReq";
      case PACKETTYPE_EXTERNALSEARCH_REPLY:
         return "ExtSearchReply";
      case PACKETTYPE_EXTERNALITEMINFO_REQUEST:
         return "ExtItemInfoReq";
      case PACKETTYPE_TILEMAP_REQUEST:
         return "TileMapReq";
      case PACKETTYPE_TILEMAP_REPLY:
         return "TileMapReply";
      case PACKETTYPE_EXPAND_REQUEST:
          return "ExpandRequest";
      case PACKETTYPE_EXPAND_REPLY:
         return "ExpandReply";
      case PACKETTYPE_BBOXREQUEST:
         return "BBoxReq";
      case PACKETTYPE_BBOXREPLY:
         return "BBoxReply";
      case PACKETTYPE_TRAFFICPOINTREPLYPACKET:
         return "Traffic point Reply";
      case PACKETTYPE_INFOCOORDINATEREQUEST:
         return "Info Coordinate Request";
      case PACKETTYPE_INFOCOORDINATEREPLY:
         return "Infor Coordinate Reply";
      case PACKETTYPE_POIREVIEW_ADD_REQUEST:
         return "Add POI Review Request";
      case PACKETTYPE_POIREVIEW_ADD_REPLY:
         return "Add POI Review Reply";
      case PACKETTYPE_POIREVIEW_DELETE_REQUEST:
         return "Delete POI Review Request";
      case PACKETTYPE_POIREVIEW_DELETE_REPLY:
         return "Delete POI Review Reply";
      case PACKETTYPE_POIREVIEW_LIST_REQUEST:
         return "List POI Reviews Request";
      case PACKETTYPE_POIREVIEW_LIST_REPLY:
         return "List POI Reviews Reply";
      case PACKETTYPE_PERIODIC_REQUEST:
         return "Periodic request";
      case PACKETTYPE_PERIODIC_REPLY:
         return "Periodic reply";
      case PACKETTYPE_SEND_DATA_REQUEST:
         return "Send data request";
      case PACKETTYPE_SEND_DATA_REPLY:
         return "Send data reply";
      case PACKETTYPE_COPYRIGHTBOX_REQUEST:
         return "Copyright box request";
      case PACKETTYPE_COPYRIGHTBOX_REPLY:
         return "Copyright box reply";
      case PACKETTYPE_TILEMAPPOI_REQUEST :
         return "Tilemappoi request";
      case PACKETTYPE_TILEMAPPOI_REPLY :
         return "Tilemappoi reply";
      case PACKETTYPE_GET_STORED_USER_DATA_REQUEST:
         return "Get stored user data request";
      case PACKETTYPE_GET_STORED_USER_DATA_REPLY:
         return "Get stored user data reply";
      case PACKETTYPE_SET_STORED_USER_DATA_REQUEST:
         return "Set stored user data request";
      case PACKETTYPE_SET_STORED_USER_DATA_REPLY:
         return "Set stored user data reply";
      case PACKETTYPE_LICENCE_TO_REQUEST :
         return "Licence to request";
      case PACKETTYPE_LICENCE_TO_REPLY :
         return "Licence to reply";
      case PACKETTYPE_DISTURBANCE_CHANGESET_REQUEST:
         return "Disturbance changeset request";
      case PACKETTYPE_DISTURBANCE_CHANGESET_REPLY:
         return "Disturbance changeset reply";
      case PACKETTYPE_FETCH_ALL_DISTURBANCES_REQUEST:
         return "Fetch all disturbances request";
      case PACKETTYPE_FETCH_ALL_DISTURBANCES_REPLY:
         return "Fetch all disturbances reply";
      case PACKETTYPE_EXTSERVICE_LIST_REQUEST:
         return "ExtService list request";
      case PACKETTYPE_EXTSERVICE_LIST_REPLY:
         return "ExtService list reply";
      case PACKETTYPE_GET_ITEM_INFO_REQUEST :
         return "Get item info request";
      case PACKETTYPE_GET_ITEM_INFO_REPLY :
         return "Get item info reply";
     case PACKETTYPE_ID_KEY_TO_REQUEST :
         return "IDKey to request";
      case PACKETTYPE_ID_KEY_TO_REPLY :
         return "IDKey to reply";
      // no default.
   }

   // Cannot happen. Yes with gcc 2.96, it can.
   static char tempString[32];
   sprintf( tempString, "unknown typenumber %d", 
            getSubType() );
   return tempString;
}

void 
Packet::resize( uint32 newSize ) {
   if ( newSize < getLength() ) {
      newSize = getLength();
   }
   bufSize = newSize;
   byte* tmpBuff = MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufSize );
   // Copy old data
   memcpy( tmpBuff, buffer, getLength() );
   if ( m_deleteBuffer ) {      
      delete[] buffer;
   } else {
      mc2log << warn << "[Packet]: Resizing packet that doesn't own buffer"
             << endl;
      MC2_ASSERT( false );
   }
   buffer = tmpBuff;
}

IPnPort
Packet::getOriginAddr() const
{
   return IPnPort( getOriginIP(), getOriginPort() );
}

void
Packet::setOriginAddr( const IPnPort& addr )
{
   setOriginIP( addr.getIP() );
   setOriginPort( addr.getPort() );
}

void Packet::dump( bool headerOnly, bool lookupIP ) const {
   mc2dbg << warn << "[Packet::dump]: Obsolete function."
          << " Use PacketUtils::dumpToFile instead!" << endl;
   PacketUtils::dumpHeaderToFile( stdout, *this, lookupIP );
   if ( ! headerOnly ) {
      PacketUtils::dumpDataToFile( stdout, *this );
   }
}

void Packet::dumpData( uint32 startPos ) const {
   mc2dbg << warn << "[Packet::dumpData]: obsolete function."
          << " Use PacketUtils::dumpToFile instead!" << endl;
   PacketUtils::dumpDataToFile( stdout, *this, startPos );
}

bool Packet::timedOut() const
{
   //
   // If has timeout and packet timeout(in sec) is
   // less, or equal to the time since arrival(in millisec)
   //
   return hasTimeout() && 
      1000 * static_cast<uint32>( getTimeout() ) <= 
      getTimeSinceArrival();
}

const uint32 RequestPacket::REQUEST_PACKET_MAPID_POS = HEADER_SIZE;


RequestPacket::RequestPacket() : Packet(MAX_PACKET_SIZE)
{
   setMapID( MAX_UINT32 );
   setLength( REQUEST_HEADER_SIZE );
   setResourceID( 0 );
}

RequestPacket::RequestPacket(byte* buf, uint32 bufLength)
   : Packet(buf, bufLength)
{
   setResourceID( 0 );
}

RequestPacket::RequestPacket(uint32 length) : Packet(length)
{
   setResourceID( 0 );
   setMapID( MAX_UINT32 );
   setLength( REQUEST_HEADER_SIZE );
}

RequestPacket::RequestPacket( uint32 bufLength, uint16 subType )
      : Packet( bufLength )
{
   setResourceID( 0 );
   setSubType( subType );
   setMapID( MAX_UINT32 );
   setLength( REQUEST_HEADER_SIZE );
}

RequestPacket::RequestPacket( uint32 bufLength,
                              byte prio,
                              uint16 subType,
                              uint16 packetId,
                              uint16 requestID,
                              uint32 mapID)
   : Packet(bufLength,
            prio,
            subType,
            packetId,
            requestID)
{
   setResourceID( 0 );
   setMapID( mapID);
   setLength( REQUEST_HEADER_SIZE );
}


void
RequestPacket::init( byte prio,
                     uint16 subType,
                     PacketID packetID,
                     RequestID requestID,
                     MapID mapID)
{
   setResourceID( 0 );   
   setPriority(prio);
   setSubType(subType);
   setPacketID(packetID);
   setRequestID(requestID);
   setMapID( mapID );
   setLength( REQUEST_HEADER_SIZE );
}



const uint32 ReplyPacket::REPLY_PACKET_STATUS_POS = HEADER_SIZE;


ReplyPacket::ReplyPacket() : Packet(MAX_PACKET_SIZE)
{
   setStatus( StringTable::NOSTRING );
   setLength( REPLY_HEADER_SIZE );
}

ReplyPacket::ReplyPacket(byte* buf, uint32 bufLength)
   : Packet(buf, bufLength)
{

}

ReplyPacket::ReplyPacket(uint32 bufLength, uint16 subType )
   : Packet(bufLength,
            0,
            subType,
            0,
            0)
{
   setStatus( StringTable::NOSTRING );
   setLength( REPLY_HEADER_SIZE );
}

ReplyPacket::ReplyPacket(uint32 bufLength) : Packet(bufLength)
{
   setStatus( StringTable::NOSTRING );
   setLength( REPLY_HEADER_SIZE );
}

ReplyPacket::ReplyPacket(  uint32 bufLength,
                           uint16 subType,
                           const RequestPacket* req,
                           uint32 status)
   : Packet(bufLength,
            req->getPriority(),
            subType,
            req->getOriginIP(),
            req->getOriginPort(),
            req->getPacketID(),
            req->getRequestID(),
            req->getDebInfo())
{
   setResendNbr( req->getResendNbr() );
   setStatus( status );
   setMapSet( req->getMapSet() );
   setRequestTag( req->getRequestTag());
   setLength(REPLY_HEADER_SIZE);
}

void
ReplyPacket::init(uint16 subType,
                  const RequestPacket* req,
                  uint32 status)
{
   setPriority(req->getPriority());
   setSubType(subType);
   setOriginIP(req->getOriginIP());
   setOriginPort(req->getOriginPort());
   setPacketID(req->getPacketID());
   setRequestID(req->getRequestID());
   setDebInfo(req->getDebInfo());
   setStatus( status );
   setMapSet( req->getMapSet() );
   setRequestTag( req->getRequestTag() );
   setLength(REPLY_HEADER_SIZE);
}

bool
ReplyPacket::checkStatus(const char* sender)
{
   bool result = false;
   StringTable::stringCode statusCode =
      static_cast<StringTable::stringCode>(getStatus());
   if (statusCode < StringTable::NBR_STRINGS) {
      if (statusCode == StringTable::OK) {
         result = true;
      } else {
         DEBUG1(cerr << "ReplyPacket::checkStatus warning from "
                << sender << ": "
                << StringTable::getString(statusCode,
                                          StringTable::ENGLISH)
                << "." << endl; );
      }
   } else {
      DEBUG1(cerr << "Unknown error (not in stringtable): "
             << (uint32) statusCode << "." << endl; );
   }
   return result;
}

const char*
ReplyPacket::getStatusAsString() const
{
   StringTable::stringCode statusCode =
      static_cast<StringTable::stringCode>(getStatus());
   return StringTable::getString(statusCode,
                                 StringTable::ENGLISH);
}

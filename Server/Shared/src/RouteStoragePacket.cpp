/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RouteStoragePacket.h"
#include "DriverPref.h"
#include "StringTable.h"

//**********************************************************************
// RouteStorageAddRouteRequestPacket
//**********************************************************************

RouteStorageAddRouteRequestPacket::RouteStorageAddRouteRequestPacket( 
   const RouteReplyPacket* routePack,
   uint32 routeID,
   uint32 UIN,
   const char* const extraUserinfo,
   uint32 validUntil,
   uint32 createTime,
   int32 originLat,
   int32 originLon,
   uint16 originAngle,
   uint32 originMapID,
   uint32 originItemID,
   uint16 originOffset,
   int32 destinationLat,
   int32 destinationLon,
   uint32 destinationMapID,
   uint32 destinationItemID,
   uint16 destinationOffset,
   const DriverPref& driverPref,
   UserEnums::URType urmask,
   bool isReRoute )
      : RequestPacket( endStatic_POS + 3 +
                       routePack->getLength() + 
                       strlen( extraUserinfo ) + 1 +
                       driverPref.getSize() + urmask.getSizeInPacket(),
                       ROUTE_STORAGE_REQUEST_PRIO,
                       Packet::PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REQUEST,
                       0,             // packetId
                       0,            //  requestID
                       MAX_UINT32 ) //   mapID
{
   int pos = REQUEST_HEADER_SIZE;

   // routeID
   incWriteLong( pos, routeID );
   // UIN
   incWriteLong( pos, UIN );
   // validUntil
   incWriteLong( pos, validUntil );
   // createTime
   incWriteLong( pos, createTime );

   // originLat
   incWriteLong( pos, originLat );
   // destinationLat
   incWriteLong( pos, destinationLat );
   // originLon
   incWriteLong( pos, originLon );
   // destinationLon
   incWriteLong( pos, destinationLon );   
   // originMapID
   incWriteLong( pos, originMapID );
   // destinationMapID
   incWriteLong( pos, destinationMapID );
   // originItemID
   incWriteLong( pos, originItemID );
   // destinationItemID
   incWriteLong( pos, destinationItemID );
   // originOffset
   incWriteShort( pos, originOffset );
   // destinationOffset
   incWriteShort( pos, destinationOffset );
   // isReRoute
   incWriteLong( pos, isReRoute );

   // routePack->getLength 
   incWriteLong( pos, routePack->getLength() );
   // strlen extraUserinfo 
   incWriteShort( pos, strlen( extraUserinfo ) );
   // originAngle
   incWriteShort( pos, originAngle );
   // driverPref size
   incWriteLong( pos, driverPref.getSize() );
   // urmask size
   incWriteLong( pos, urmask.getSizeInPacket() );
   // extraUserinfo
   incWriteString( pos, extraUserinfo );
   // routePack
   for ( uint32 i = 0 ; i < routePack->getLength() ; i++ ) {
      incWriteByte( pos, routePack->getBuf()[ i ] );
   }
   driverPref.save( this, pos );
   urmask.save( this, pos );

   setLength( pos );
}


RouteReplyPacket*
RouteStorageAddRouteRequestPacket::getRoutePacket() const {
   int pos = endStatic_POS;
   // strlen extraUserinfo 
   pos += readShort( strlenExtraUserinfo_POS ) + 1;
   uint32 length = readLong( routePackLength_POS );
   
   byte* buf = new byte[ length ];
   for ( uint32 i = 0 ; i < length ; i++ ) {
      buf[ i ] = incReadByte( pos );
   }
   RouteReplyPacket* ans = static_cast< RouteReplyPacket* > (
      Packet::makePacket( buf, length ) );

   return ans;
}


uint32
RouteStorageAddRouteRequestPacket::getRouteID() const {
   return readLong( routeID_POS );
}


uint32
RouteStorageAddRouteRequestPacket::getUIN() const {
   return readLong( UIN_POS );
}


const char* 
RouteStorageAddRouteRequestPacket::getExtraUserinfo() const {
   char* str = NULL;
   int pos = endStatic_POS;
   incReadString( pos, str );

   return str;
}


uint32
RouteStorageAddRouteRequestPacket::getValidUntil() const {
   return readLong( validUntil_POS );
}


uint32
RouteStorageAddRouteRequestPacket::getCreateTime() const {
   return readLong( createTime_POS );
}


uint32 
RouteStorageAddRouteRequestPacket::getOriginLat() const {
   return readLong( originLat_POS );
}


uint32 
RouteStorageAddRouteRequestPacket::getDestinationLat() const {
   return readLong( destinationLat_POS );
}


uint32 
RouteStorageAddRouteRequestPacket::getOriginLon() const {
   return readLong( originLon_POS ); 
}

uint16
RouteStorageAddRouteRequestPacket::getOriginAngle() const {
   return readShort( originAngle_POS );
}

uint32 
RouteStorageAddRouteRequestPacket::getDestinationLon() const {
   return readLong( destinationLon_POS ); 
}


uint32 
RouteStorageAddRouteRequestPacket::getOriginMapID() const {
   return readLong( originMapID_POS );   
}


uint32 
RouteStorageAddRouteRequestPacket::getDestinationMapID() const {
   return readLong( destinationMapID_POS );   
}


uint32 
RouteStorageAddRouteRequestPacket::getOriginItemID() const {
   return readLong( originItemID_POS );   
}


uint32 
RouteStorageAddRouteRequestPacket::getDestinationItemID() const {
   return readLong( destinationItemID_POS );   
}


uint16 
RouteStorageAddRouteRequestPacket::getOriginOffset() const {
   return readShort( originOffset_POS );   
}


uint16 
RouteStorageAddRouteRequestPacket::getDestinationOffset() const {
   return readShort( destinationOffset_POS );   
}

void
RouteStorageAddRouteRequestPacket::getDriverPrefs( 
   DriverPref& driverPref ) const
{
   int pos = endStatic_POS;
   pos += readShort( strlenExtraUserinfo_POS ) + 1;
   pos += readLong( routePackLength_POS );
   driverPref.load( this, pos );
}

UserEnums::URType
RouteStorageAddRouteRequestPacket::getUrmask() const {
   int pos = endStatic_POS;
   pos += readShort( strlenExtraUserinfo_POS ) + 1;
   pos += readLong( routePackLength_POS );
   pos += readLong( driverPrefSize_POS );
   UserEnums::URType urmask;
   urmask.load( this, pos );

   return urmask;
}

bool
RouteStorageAddRouteRequestPacket::getIsReRoute() const {
   return readLong( isReRoute_POS );
}

//**********************************************************************
// RouteStorageAddRouteReplyPacket
//**********************************************************************


RouteStorageAddRouteReplyPacket::RouteStorageAddRouteReplyPacket(
   const RouteStorageAddRouteRequestPacket* inPacket,
   StringCode status )
      : ReplyPacket( REPLY_HEADER_SIZE, 
                     Packet::PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REPLY,
                     inPacket, 
                     status )
{
   setLength( REPLY_HEADER_SIZE );
}


//**********************************************************************
// RouteStorageGetRouteRequestPacket
//**********************************************************************


RouteStorageGetRouteRequestPacket::RouteStorageGetRouteRequestPacket(
   uint32 routeID,
   uint32 createTime )
      : RequestPacket( REQUEST_HEADER_SIZE + endStatic_POS,
                       ROUTE_STORAGE_REQUEST_PRIO,
                       Packet::PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REQUEST,
                       0,             // packetId
                       0,            //  requestID
                       MAX_UINT32 ) //   mapID
{
   writeLong( routeID_POS, routeID );
   writeLong( createTime_POS, createTime );
   setLength( endStatic_POS );
}


uint32
RouteStorageGetRouteRequestPacket::getRouteID() const {
   return readLong( routeID_POS );
}


uint32
RouteStorageGetRouteRequestPacket::getCreateTime() const {
   return readLong( createTime_POS );
}


//**********************************************************************
// RouteStorageGetRouteRequestPacket
//**********************************************************************


RouteStorageGetRouteReplyPacket::RouteStorageGetRouteReplyPacket( 
   const RouteStorageGetRouteRequestPacket* inPacket,
   StringCode status,
   const RouteReplyPacket* routePack,
   uint32 routeID,
   uint32 UIN,
   const char* const extraUserinfo,
   uint32 validUntil,
   uint32 createTime,
   int32 originLat,
   int32 originLon,
   uint16 originAngle,
   uint32 originMapID,
   uint32 originItemID,
   uint16 originOffset,
   int32 destinationLat,
   int32 destinationLon,
   uint32 destinationMapID,
   uint32 destinationItemID,
   uint16 destinationOffset,
   const DriverPref& driverPref,
   UserEnums::URType urmask )
      : ReplyPacket( endStatic_POS + 3 +
                     (routePack ? routePack->getLength() : 0 ) + 
                     (extraUserinfo ? strlen( extraUserinfo ) : 0 ) + 1 +
                     driverPref.getSize() + urmask.getSizeInPacket(),
                     Packet::PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REPLY,
                     inPacket, 
                     status )
{
   int pos = REPLY_HEADER_SIZE;

   // routeID
   incWriteLong( pos, routeID );
   // UIN
   incWriteLong( pos, UIN );
   // validUntil
   incWriteLong( pos, validUntil );
   // createTime
   incWriteLong( pos, createTime );

   // originLat
   incWriteLong( pos, originLat );
   // destinationLat
   incWriteLong( pos, destinationLat );
   // originLon
   incWriteLong( pos, originLon );
   // destinationLon
   incWriteLong( pos, destinationLon );   
   // originMapID
   incWriteLong( pos, originMapID );
   // destinationMapID
   incWriteLong( pos, destinationMapID );
   // originItemID
   incWriteLong( pos, originItemID );
   // destinationItemID
   incWriteLong( pos, destinationItemID );
   // originOffset
   incWriteShort( pos, originOffset );
   // destinationOffset
   incWriteShort( pos, destinationOffset );

   // routePack->getLength 
   incWriteLong( pos, routePack ? routePack->getLength() : 0 );
   // strlen extraUserinfo 
   incWriteShort( pos, extraUserinfo ? strlen( extraUserinfo ) : 0 );
   // originAngle
   incWriteShort( pos, originAngle );
   // driverPref size
   incWriteLong( pos, driverPref.getSize() );
   // urmask size
   incWriteLong( pos, urmask.getSizeInPacket() );
   // extraUserinfo
   incWriteString( pos, extraUserinfo ? extraUserinfo : "" );
   // routePack
   if ( routePack != NULL ) {
      for ( uint32 i = 0 ; i < routePack->getLength() ; i++ ) {
         incWriteByte( pos, routePack->getBuf()[ i ] );
      }
   }
   driverPref.save( this, pos );
   urmask.save( this, pos );

   setLength( pos );   
}


RouteReplyPacket*
RouteStorageGetRouteReplyPacket::getRoutePacket() const {
   int pos = endStatic_POS;
   // strlen extraUserinfo 
   pos += readShort( strlenExtraUserinfo_POS ) + 1;
   uint32 length = readLong( routePackLength_POS );
   RouteReplyPacket* ans = NULL;

   if ( length != 0 ) {
      byte* buf = new byte[ length ];
      for ( uint32 i = 0 ; i < length ; i++ ) {
         buf[ i ] = incReadByte( pos );
      }
      ans = static_cast< RouteReplyPacket* > ( 
         Packet::makePacket( buf, length ) );
   }

   return ans;
}

uint32
RouteStorageGetRouteReplyPacket::getRoutePacketLength() const {
   return readLong( routePackLength_POS );
}

uint32
RouteStorageGetRouteReplyPacket::getRouteID() const {
   return readLong( routeID_POS );
}


uint32
RouteStorageGetRouteReplyPacket::getUIN() const {
   return readLong( UIN_POS );
}


const char* 
RouteStorageGetRouteReplyPacket::getExtraUserinfo() const {
   char* str = NULL;
   int pos = endStatic_POS;
   incReadString( pos, str );
   
   return str;
}


uint32
RouteStorageGetRouteReplyPacket::getValidUntil() const {
   return readLong( validUntil_POS );
}


uint32
RouteStorageGetRouteReplyPacket::getCreateTime() const {
   return readLong( createTime_POS );
}


uint32 
RouteStorageGetRouteReplyPacket::getOriginLat() const {
   return readLong( originLat_POS );
}


uint32 
RouteStorageGetRouteReplyPacket::getDestinationLat() const {
   return readLong( destinationLat_POS );
}


uint32 
RouteStorageGetRouteReplyPacket::getOriginLon() const {
   return readLong( originLon_POS ); 
}

uint16
RouteStorageGetRouteReplyPacket::getOriginAngle() const {
   return readShort( originAngle_POS );
}

uint32 
RouteStorageGetRouteReplyPacket::getDestinationLon() const {
   return readLong( destinationLon_POS ); 
}


uint32 
RouteStorageGetRouteReplyPacket::getOriginMapID() const {
   return readLong( originMapID_POS );   
}


uint32 
RouteStorageGetRouteReplyPacket::getDestinationMapID() const {
   return readLong( destinationMapID_POS );   
}


uint32 
RouteStorageGetRouteReplyPacket::getOriginItemID() const {
   return readLong( originItemID_POS );   
}


uint32 
RouteStorageGetRouteReplyPacket::getDestinationItemID() const {
   return readLong( destinationItemID_POS );   
}


uint16 
RouteStorageGetRouteReplyPacket::getOriginOffset() const {
   return readShort( originOffset_POS );   
}


uint16 
RouteStorageGetRouteReplyPacket::getDestinationOffset() const {
   return readShort( destinationOffset_POS );   
}

void
RouteStorageGetRouteReplyPacket::getDriverPrefs( 
   DriverPref& driverPref ) const 
{
   int pos = endStatic_POS;
   pos += readShort( strlenExtraUserinfo_POS ) + 1;
   pos += readLong( routePackLength_POS );
   driverPref.load( this, pos );
}
 
UserEnums::URType
RouteStorageGetRouteReplyPacket::getUrmask() const {
   int pos = endStatic_POS;
   pos += readShort( strlenExtraUserinfo_POS ) + 1;
   pos += readLong( routePackLength_POS );
   pos += readLong( driverPrefSize_POS );
   UserEnums::URType urmask;
   urmask.load( this, pos );

   return urmask;
}

//**********************************************************************
// RouteStorageChangeRouteRequestPacket
//**********************************************************************


RouteStorageChangeRouteRequestPacket::RouteStorageChangeRouteRequestPacket(
   uint32 routeID,
   uint32 createTime,
   uint32 validUntil )
      : RequestPacket( 
         REQUEST_HEADER_SIZE + endStatic_POS,
         ROUTE_STORAGE_REQUEST_PRIO,
         Packet::PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REQUEST,
         0,             // packetId
         0,            //  requestID
         MAX_UINT32 ) //   mapID
{
   writeLong( routeID_POS, routeID );
   writeLong( createTime_POS, createTime );
   writeLong( validUntil_POS, validUntil );
   setLength( endStatic_POS );
}


uint32
RouteStorageChangeRouteRequestPacket::getRouteID() const {
   return readLong( routeID_POS );
}


uint32
RouteStorageChangeRouteRequestPacket::getCreateTime() const {
   return readLong( createTime_POS );
}


uint32
RouteStorageChangeRouteRequestPacket::getValidUntil() const {
   return readLong( validUntil_POS );
}


//**********************************************************************
// RouteStorageChangeRouteRequestPacket
//**********************************************************************


RouteStorageChangeRouteReplyPacket::RouteStorageChangeRouteReplyPacket( 
   const RouteStorageChangeRouteRequestPacket* inPacket,
   StringCode status )
      : ReplyPacket( REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REPLY,
                     inPacket, 
                     status )
{
}

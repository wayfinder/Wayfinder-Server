/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavPacket.h"
#include "MC2CRC32.h"


byte NavPacket::MAX_PROTOVER = 0x0c;

uint8 NavPacket::magicBytes[256] = {
   0x37, 0x77, 0x89, 0x05, 0x28, 0x72, 0x5b, 0x2a, 0xce, 0xe4, 0x44, 0x1a,
   0x28, 0x72, 0x5b, 0x2a, 0xce, 0xe4, 0x44, 0x1a, 0x79, 0x8a, 0xdb, 0x90,
   0xce, 0xe4, 0x44, 0x1a, 0x79, 0x8a, 0xdb, 0x90, 0x2f, 0x5b, 0xcc, 0xd1,
   0x79, 0x8a, 0xdb, 0x90, 0x2f, 0x5b, 0xcc, 0xd1, 0x6e, 0x72, 0xb4, 0x9a,
   0x2f, 0x5b, 0xcc, 0xd1, 0x6e, 0x72, 0xb4, 0x9a, 0x79, 0x9b, 0xa5, 0x2b,
   0x6e, 0x72, 0xb4, 0x9a, 0x79, 0x9b, 0xa5, 0x2b, 0x45, 0x6a, 0xc1, 0x7c,
   0x79, 0x9b, 0xa5, 0x2b, 0x45, 0x6a, 0xc1, 0x7c, 0xe0, 0x4a, 0x81, 0x08,
   0x45, 0x6a, 0xc1, 0x7c, 0xe0, 0x4a, 0x81, 0x08, 0xbc, 0xdb, 0x31, 0x8a,
   0xe0, 0x4a, 0x81, 0x08, 0xbc, 0xdb, 0x31, 0x8a, 0xbe, 0x75, 0xa3, 0x36,
   0xbc, 0xdb, 0x31, 0x8a, 0xbe, 0x75, 0xa3, 0x36, 0xfe, 0x7d, 0xc6, 0x2c,
   0xbe, 0x75, 0xa3, 0x36, 0xfe, 0x7d, 0xc6, 0x2c, 0xd8, 0x91, 0xfc, 0x45,
   0xfe, 0x7d, 0xc6, 0x2c, 0xd8, 0x91, 0xfc, 0x45, 0x03, 0xb0, 0xde, 0x7b,
   0xd8, 0x91, 0xfc, 0x45, 0x03, 0xb0, 0xde, 0x7b, 0x4b, 0x82, 0xa6, 0x90,
   0x03, 0xb0, 0xde, 0x7b, 0x4b, 0x82, 0xa6, 0x90, 0xeb, 0x67, 0x0c, 0xcb,
   0x4b, 0x82, 0xa6, 0x90, 0xeb, 0x67, 0x0c, 0xcb, 0xb0, 0x8c, 0xd2, 0x6c,
   0xeb, 0x67, 0x0c, 0xcb, 0xb0, 0x8c, 0xd2, 0x6c, 0x67, 0x03, 0xf5, 0x24,
   0xb0, 0x8c, 0xd2, 0x6c, 0x67, 0x03, 0xf5, 0x24, 0x77, 0x98, 0x5a, 0x74,
   0x67, 0x03, 0xf5, 0x24, 0x77, 0x98, 0x5a, 0x74, 0x14, 0x1f, 0x9f, 0xec,
   0x77, 0x98, 0x5a, 0x74, 0x14, 0x1f, 0x9f, 0xec, 0xb0, 0x9b, 0x30, 0xb2,
   0x14, 0x1f, 0x9f, 0xec, 0xb0, 0x9b, 0x30, 0xb2, 0x4b, 0x0e, 0x2d, 0x95,
   0xb0, 0x9b, 0x30, 0xb2, 0x4b, 0x0e, 0x2d, 0x95, 0x90, 0xd3, 0x25, 0x7a,
   0x4b, 0x0e, 0x2d, 0x95, 
};


NavPacket::NavPacket( byte protoVer, uint16 type, byte reqID, byte reqVer )
      : m_protoVer( protoVer ), m_type( type ), m_reqID( reqID ), 
        m_reqVer( reqVer )
{
}


NavPacket::NavPacket( byte protoVer, uint16 type, byte reqID, byte reqVer,
                      const byte* buff, uint32 buffLen,
                      uint32* uncompressedSize )
      : m_protoVer( protoVer ), m_type( type ), m_reqID( reqID ), 
        m_reqVer( reqVer ), m_params( buff, buffLen, protoVer, 
                                      uncompressedSize )
{
}


const char*
NavPacket::requestTypeAsString( uint16 type ) {
   switch( type ) {
      case NAV_INVALID :
         return "INVALID";
      case NAV_ROUTE_REQ :
         return "Route";
      case NAV_ROUTE_REPLY :
         return "Route Reply";
      case NAV_SEARCH_REQ :
         return "Search";
      case NAV_SEARCH_REPLY :
         return "Search Reply";
      case NAV_REV_GEOCODING_REQ :
         return "ReverseGeocoding";
      case NAV_REV_GEOCODING_REPLY :
         return "ReverseGeocoding Reply";
      case NAV_MAP_REQ :
         return "Map";
      case NAV_MAP_REPLY :
         return "Map Reply";
      case NAV_FAV_REQ :
         return "Fav";
      case NAV_FAV_REPLY :
         return "Fav Reply";
      case NAV_INFO_REQ :
         return "Info";
      case NAV_INFO_REPLY :
         return "Info Reply";
      case NAV_DETAIL_REQ :
         return "Detail";
      case NAV_DETAIL_REPLY :
         return "Detail Reply";
      case NAV_MESSAGE_REQ :
         return "Message";
      case NAV_MESSAGE_REPLY :
         return "Message Reply";
      case NAV_UPGRADE_REQ :
         return "Upgrade";
      case NAV_UPGRADE_REPLY :
         return "Upgrade Reply";
      case NAV_VECTOR_MAP_REQ :
         return "VectorMap";
      case NAV_VECTOR_MAP_REPLY :
         return "VectorMap Reply";
      case NAV_MULTI_VECTOR_MAP_REQ :
         return "MultiVectorMap";
      case NAV_MULTI_VECTOR_MAP_REPLY :
         return "MultiVectorMap Reply";
      case NAV_CELL_REPORT :
         return "CellReport";
      case NAV_CELL_CONFIRM :
         return "CellConfirm";
      case NAV_TOP_REGION_REQ :
         return "TopRegion";
      case NAV_TOP_REGION_REPLY :
         return "TopRegion Reply";
      case NAV_LATEST_NEWS_REQ :
         return "LatestNews";
      case NAV_LATEST_NEWS_REPLY :
         return "LatestNews Reply";
      case NAV_CATEGORIES_REQ :
         return "Categories";
      case NAV_CATEGORIES_REPLY :
         return "Categories Reply";
      case NAV_CALLCENTER_LIST_REQ :
         return "CallcenterList";
      case NAV_CALLCENTER_LIST_REPLY :
         return "CallcenterList Reply";
      case NAV_SERVER_LIST_REQ :
         return "ServerList";
      case NAV_SERVER_LIST_REPLY :
         return "ServerList Reply";
      case NAV_NEW_PASSWORD_REQ :
         return "NewPassword";
      case NAV_NEW_PASSWORD_REPLY :
         return "NewPassword Reply";
      case NAV_SERVER_INFO_REQ :
         return "ServerInfo";
      case NAV_SERVER_INFO_REPLY :
         return "ServerInfo Reply";
      case NAV_WHOAMI_REQ :
         return "WhoAmI";
      case NAV_WHOAMI_REPLY :
         return "WhoAmI Reply";
      case NAV_BINARY_TRANSFER_REQ :
         return "BinaryTransfer";
      case NAV_BINARY_TRANSFER_REPLY :
         return "BinaryTransfer Reply";
      case NAV_NOP_REQ :
         return "NOP";
      case NAV_NOP_REPLY :
         return "NOP Reply";
      case NAV_CHANGED_LICENCE_REQ :
         return "ChangedLicence";
      case NAV_CHANGED_LICENCE_REPLY :
         return "ChangedLicence Reply";
      case NAV_SERVER_AUTH_BOB_REQ :
         return "ServerAuthBob";
      case NAV_SERVER_AUTH_BOB_REPLY :
         return "ServerAuthBob Reply";
      case NAV_TRACK_REQ :
         return "Tracking";
      case NAV_TUNNEL_DATA_REQ :
         return "Tunnel";
      case NAV_COMBINED_SEARCH_REQ :
         return "CombinedSearch";
      case NAV_COMBINED_SEARCH_REPLY :
         return "CombinedSearch Reply";
      case NAV_SEARCH_DESC_REQ :
         return "SearchDesc";
      case NAV_SEARCH_DESC_REPLY :
         return "SearchDesc Reply";
      case NAV_CELLID_LOOKUP_REQ:
         return "CellIDLookup";
      case NAV_CELLID_LOOKUP_REPLY:
         return "CellIDLookup Reply";
      case NAV_GET_KEYED_DATA_REQ:
         return "GetKeyedData";
      case NAV_GET_KEYED_DATA_REPLY:
         return "GetKeyedData Reply";
      case NAV_ECHO_REQ :
         return "Echo";
      case NAV_ECHO_REPLY :
         return "Echo Reply";
      case NAV_VERIFY_THIRD_PARTY_TRANSACTION_REQ :
         return "VerifyThirdPartyTransaction";
      case NAV_VERIFY_THIRD_PARTY_TRANSACTION_REPLY :
         return "VerifyThirdPartyTransaction Reply";
      case NAV_LOCAL_CATEGORY_TREE_REQ :
         return "LocalCategoryTree";
      case NAV_LOCAL_CATEGORY_TREE_REPLY :
         return "LocalCategoryTree Reply";
      case NAV_ONE_SEARCH_REQ :
         return "OneSearch";
      case NAV_ONE_SEARCH_REPLY :
         return "OneSearch Reply";
    };

   return "UNKNOWN";
}


void
NavPacket::writeHeader( vector< byte >& buff ) const {
   // STX 1
   buff.push_back( 0x02 );
   // Length 4 (Real value written last)
   buff.push_back( 0 );
   buff.push_back( 0 );
   buff.push_back( 0 );
   buff.push_back( 0 );
   // protover 1 
   buff.push_back( m_protoVer );
   // type 2
   buff.push_back( ( m_type ) >> 8 );
   buff.push_back( ( m_type ) & 0xff );
   // req_id 1
   buff.push_back( m_reqID );
   // req_ver 1
   buff.push_back( m_reqVer );
}


void
NavPacket::writeLengthAndParams( vector< byte >& buff,
                                 bool mayUseGzip,
                                 uint32* uncompressedSize ) const
{
   // ParamBlock
   m_params.writeParams( buff, m_protoVer, mayUseGzip, uncompressedSize );
   if ( uncompressedSize ) {
      *uncompressedSize += 4; // crc
   }

   // Write length
   // Length 1-4
   buff[ 1 ] = (( buff.size() + 4 ) >> 24 ) & 0xff;
   buff[ 2 ] = (( buff.size() + 4 ) >> 16 ) & 0xff;
   buff[ 3 ] = (( buff.size() + 4 ) >> 8  ) & 0xff;
   buff[ 4 ] = (( buff.size() + 4 ) )       & 0xff;

   // CRC 4
   uint32 crc = MC2CRC32::crc32( &buff.front(), buff.size() );
   buff.push_back( (( crc ) >> 24 ) & 0xff );
   buff.push_back( (( crc ) >> 16 ) & 0xff );
   buff.push_back( (( crc ) >> 8  ) & 0xff );
   buff.push_back( (( crc ) )       & 0xff );
}


void
NavPacket::dump( ostream& out, const char* prefix, bool dumpValues, 
                 bool singleLine, uint32 maxLen,
                 uint32 maxSame ) const
{
   out << prefix << " protoVer " << MC2HEX( int(m_protoVer) );
   if ( !singleLine ) out << endl;
   out << " type " << MC2HEX( int(m_type) );
   if ( !singleLine ) out << endl;
   out << " reqID " << MC2HEX( int(m_reqID) );
   if ( !singleLine ) out << endl;
   out << " reqVer " << MC2HEX( int(m_reqVer) );
   if ( !singleLine ) out << endl;
   if ( singleLine )  out << " ";
   m_params.dump( out, dumpValues, singleLine, maxLen, maxSame );
}


NavRequestPacket::NavRequestPacket( byte protoVer, uint16 type, byte reqID,
                                    byte reqVer, const byte* buff,
                                    uint32 buffLen,
                                    uint32* uncompressedSize )
      : NavPacket( protoVer, type, reqID, reqVer, buff, buffLen, 
                   uncompressedSize )
{
}


void 
NavRequestPacket::writeTo( vector< byte >& buff,
                           bool mayUseGzip,
                           uint32* uncompressedSize ) const 
{
   writeHeader( buff );
   writeLengthAndParams( buff, mayUseGzip, uncompressedSize );
}


NavReplyPacket::NavReplyPacket( byte protoVer, uint16 type, byte reqID, 
                                byte reqVer, byte statusCode, 
                                const char* statusMessage )
      : NavPacket( protoVer, type, reqID, reqVer ), 
        m_statusCode( statusCode ), m_statusMessage( statusMessage )
{
}


NavReplyPacket::NavReplyPacket( 
   byte protoVer, uint16 type, byte reqID, byte reqVer,
   byte statusCode, const char* statusMessage,
   const byte* buff, uint32 buffLen, uint32* uncompressedSize )
      : NavPacket( protoVer, type, reqID, reqVer, buff, buffLen, 
                   uncompressedSize ),
        m_statusCode( statusCode ), m_statusMessage( statusMessage )
{
}


void 
NavReplyPacket::writeTo( vector< byte >& buff,
                         bool mayUseGzip, uint32* uncompressedSize ) const 
{
   // Header

   writeHeader( buff );
   // status_code 1
   buff.push_back( m_statusCode );
   // status_message string
   for ( uint32 i = 0 ; i < m_statusMessage.size() ; ++i ) {
      buff.push_back( m_statusMessage[ i ] );
   }
   buff.push_back( '\0' ); // Null byte

   writeLengthAndParams( buff, mayUseGzip, uncompressedSize );
}

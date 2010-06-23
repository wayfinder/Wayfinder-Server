/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "isabBoxNavMessage.h"
#include "StringUtility.h"
#include "GfxConstants.h"
#include "ExpandRoutePacket.h"
#include "SearchRequest.h"
#include "SearchPacket.h"
#include "SearchReplyPacket.h"
#include "RouteList.h"
#include "RouteElement.h"
#include "NavStringTable.h"
#include <time.h>
#include "NavMessage.h"
#include "UTF8Util.h"


const
struct isabBoxNavMessageUtil::turnCodeTable_t
isabBoxNavMessageUtil::turnCodeTable[] = {
   // don't use nav_route_point_start or nav_route_point_end !!
   { StringTable::AHEAD_TURN,      nav_route_point_ahead },
   { StringTable::LEFT_TURN,       nav_route_point_left },
   { StringTable::RIGHT_TURN,      nav_route_point_right },
   { StringTable::U_TURN,          nav_route_point_uturn },
   { StringTable::DRIVE_START,     nav_route_point_startat },
   { StringTable::DRIVE_FINALLY,   nav_route_point_finally },
   { StringTable::ENTER_ROUNDABOUT_TURN, nav_route_point_enter_rdbt },
   { StringTable::EXIT_ROUNDABOUT_TURN , nav_route_point_exit_rdbt },
   { StringTable::AHEAD_ROUNDABOUT_TURN, nav_route_point_ahead_rdbt },
   { StringTable::LEFT_ROUNDABOUT_TURN , nav_route_point_left_rdbt},
   { StringTable::RIGHT_ROUNDABOUT_TURN, nav_route_point_right_rdbt },
   { StringTable::OFF_RAMP_TURN, nav_route_point_exitat },
   { StringTable::ON_RAMP_TURN, nav_route_point_on },
   { StringTable::PARK_CAR    , nav_route_point_park_car },
   { StringTable::KEEP_LEFT   , nav_route_point_keep_left },
   { StringTable::KEEP_RIGHT  , nav_route_point_keep_right },
   { StringTable::DRIVE_START_WITH_UTURN,nav_route_point_start_with_uturn},
   { StringTable::U_TURN_ROUNDABOUT_TURN , nav_rotue_point_uturn_rdbt},
   { StringTable::FOLLOWROAD_TURN  , nav_route_point_follow_road},
   { StringTable::ENTER_FERRY_TURN  , nav_route_point_enter_ferry },
   { StringTable::EXIT_FERRY_TURN  , nav_route_point_exit_ferry },
   { StringTable::CHANGE_FERRY_TURN  , nav_route_point_change_ferry },
   { StringTable::ENDOFROAD_LEFT_TURN, nav_route_point_endofroad_left },
   { StringTable::ENDOFROAD_RIGHT_TURN, nav_route_point_endofroad_right },
   { StringTable::LEFT_OFF_RAMP_TURN, nav_route_point_exitat },
   { StringTable::RIGHT_OFF_RAMP_TURN, nav_route_point_exitat },
   { StringTable::OFF_MAIN_TURN, nav_route_point_exitat },
   { StringTable::ON_MAIN_TURN, nav_route_point_on },
   { StringTable::NOTOK ,  nav_route_point_not_ok }
   // Bus not supported here
// ENTER_BUS
// EXIT_BUS
// CHANGE_BUS
};

const
struct isabBoxNavMessageUtil::turnCodeTable_t
isabBoxNavMessageUtil::turnCodeTable7p[] = {
   // don't use nav_route_point_start or nav_route_point_end !!
   { StringTable::AHEAD_TURN,      nav_route_point_ahead },
   { StringTable::LEFT_TURN,       nav_route_point_left },
   { StringTable::RIGHT_TURN,      nav_route_point_right },
   { StringTable::U_TURN,          nav_route_point_uturn },
   { StringTable::DRIVE_START,     nav_route_point_startat },
   { StringTable::DRIVE_FINALLY,   nav_route_point_finally },
   { StringTable::ENTER_ROUNDABOUT_TURN, nav_route_point_enter_rdbt },
   { StringTable::EXIT_ROUNDABOUT_TURN , nav_route_point_exit_rdbt },
   { StringTable::EXIT_ROUNDABOUT_TURN , nav_route_point_exit_rdbt_8 },
   { StringTable::EXIT_ROUNDABOUT_TURN , nav_route_point_exit_rdbt_16 },
   { StringTable::EXIT_ROUNDABOUT_TURN_WALK, nav_route_point_exit_rdbt },
   { StringTable::EXIT_ROUNDABOUT_TURN_WALK, nav_route_point_exit_rdbt_8 },
   { StringTable::EXIT_ROUNDABOUT_TURN_WALK, nav_route_point_exit_rdbt_16},
   { StringTable::AHEAD_ROUNDABOUT_TURN, nav_route_point_ahead_rdbt },
   { StringTable::LEFT_ROUNDABOUT_TURN , nav_route_point_left_rdbt},
   { StringTable::RIGHT_ROUNDABOUT_TURN, nav_route_point_right_rdbt },
   { StringTable::OFF_RAMP_TURN, nav_route_point_exitat },
   { StringTable::ON_RAMP_TURN, nav_route_point_on },
   { StringTable::PARK_CAR    , nav_route_point_park_car },
   { StringTable::KEEP_LEFT   , nav_route_point_keep_left },
   { StringTable::KEEP_RIGHT  , nav_route_point_keep_right },
   { StringTable::DRIVE_START_WITH_UTURN,nav_route_point_start_with_uturn},
   { StringTable::U_TURN_ROUNDABOUT_TURN , nav_rotue_point_uturn_rdbt},
   { StringTable::FOLLOWROAD_TURN  , nav_route_point_follow_road},
   { StringTable::ENTER_FERRY_TURN  , nav_route_point_enter_ferry },
   { StringTable::EXIT_FERRY_TURN  , nav_route_point_exit_ferry },
   { StringTable::CHANGE_FERRY_TURN  , nav_route_point_change_ferry },
   { StringTable::ENDOFROAD_LEFT_TURN, nav_route_point_endofroad_left },
   { StringTable::ENDOFROAD_RIGHT_TURN, nav_route_point_endofroad_right },
   { StringTable::LEFT_OFF_RAMP_TURN, nav_route_point_off_ramp_left },
   { StringTable::RIGHT_OFF_RAMP_TURN, nav_route_point_off_ramp_right },
   { StringTable::LMLOCATION_ROADNAMECHANGE, nav_route_point_ahead },
   { StringTable::OFF_MAIN_TURN, nav_route_point_exitat },
   { StringTable::ON_MAIN_TURN, nav_route_point_on },
   { StringTable::NOTOK ,  nav_route_point_not_ok }
   // Bus not supported here
// ENTER_BUS
// EXIT_BUS
// CHANGE_BUS
};


int
isabBoxNavMessageUtil::convertHeaderFromBytes(NavMessage* mess,
                                              const byte* buf,
                                              int bufSize)
{
   if ( buf == NULL )
      return 0;
   if ( bufSize < ISABBOX_MESSAGE_HEADER_SIZE_VER_0 ) {
      mc2log << error << "isabBoxNavMessageUtil::convertHeaderFromBytes "
                "Buffer too short for header: " << bufSize << endl;
      return 0;
   } else {
      mc2dbg2 << "isabBoxNavMessageUtil::convertHeaderFromBytes" << endl;
      int pos = 0;
      /* Read the STX */
      incReadByte(buf, pos);
      mess->setLength(incReadLong(buf, pos));
      mc2dbg2 << "   Length: " << mess->getLength() << endl;
      mess->setProtoVer(incReadByte(buf, pos));
      mc2dbg2 << "   ProtoVer: " << (int)mess->getProtoVer() << endl;
      mess->setType(incReadShort(buf, pos));
      mc2dbg2 << "   Type: " << (uint16)mess->getType() << endl;
      mess->setReqID(incReadByte(buf, pos));
      mc2dbg2 << "   ReqId: " << (int)mess->getReqID() << endl;

      if ( mess->getProtoVer() >= 0x05 ) { // Version 5
         // CRC is 4 bytes and navID and userName removed
         mess->setCRC( incReadLong( buf, pos ) );
         mc2dbg2 << "   CRC: " << mess->getCRC() << endl;
         // Initialize navID
         mess->setNavID( 0 );
         // Initialize userName
         byte userName[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
         mess->setUserName( userName );
         // Initialize UserID
         mess->setUserID( 0 );
      } else { // Version 0-4
         mess->setCRC(incReadShort(buf, pos));
         mc2dbg2 << "   CRC: " << mess->getCRC() << endl;
         if (mess->getProtoVer() > 0) {
            mess->setNavID(incReadLong(buf, pos));
            mc2log << info << "   NavID: " << (int)mess->getNavID()
                   << endl;
         }
         if (mess->getProtoVer() == 1) {
            // replaced in ver 2 by a 20 byte string
            mess->setUserID(incReadLong(buf, pos));
            mc2log << info << "   UserID: " << (int)mess->getUserID()
                   << endl;
         } else {
            // Initialize UserID
            mess->setUserID( 0 );
         }
         if (mess->getProtoVer() > 1) {
            mess->setUserName(&buf[pos]);
            pos += 20;
         }
         mc2log << info << "   userName: " << mess->getUserName() << endl;
      }
      mc2dbg2 << "pos after header: " << pos << endl;

      return pos;
   }
}

int
isabBoxNavMessageUtil::convertHeaderToBytes(const NavMessage* mess,
                                            byte* buf,
                                            int bufSize)
{
   if (mess->getProtoVer() == 0) {
      if ( bufSize < ISABBOX_MESSAGE_HEADER_SIZE_VER_0 ) {
         mc2log << error << "isabBoxNavMessageUtil::convertHeaderToBytes "
                   " V0 Buffer too short for header" << endl;
         return 0;
      }
   } else if (mess->getProtoVer() == 1)  {
      if ( bufSize < ISABBOX_MESSAGE_HEADER_SIZE_VER_1 ) {
         mc2log << error << "isabBoxNavMessageUtil::convertHeaderToBytes "
                   " V1 Buffer too short for header" << endl;
         return 0;
      }
   } else if ( mess->getProtoVer() >= 5 ) {
      if ( bufSize < ISABBOX_MESSAGE_HEADER_SIZE_VER_5 ) {
         mc2log << error << "isabBoxNavMessageUtil::convertHeaderToBytes "
            " V5 Buffer too short for header" << endl;
         return 0;
      }
   } else {
      if ( bufSize < ISABBOX_MESSAGE_HEADER_SIZE_VER_2 ) {
         mc2log << error << "isabBoxNavMessageUtil::convertHeaderToBytes "
                   " V2 Buffer too short for header" << endl;
         return 0;
      }
   }
   int pos = 0;

   mc2dbg2 << "Length: " << mess->getLength() << endl;
   mc2dbg2 << "Proto : " << (long unsigned int)mess->getProtoVer() << endl;
   mc2dbg2 << "Type  : " << (long unsigned int)mess->getType() << endl;
   mc2dbg2 << "ReqID : " << (long unsigned int)mess->getReqID() << endl;
   mc2dbg2 << "CRC   : " << mess->getCRC() << endl;
   mc2dbg2 << "NavID : " << mess->getNavID() << endl;
   mc2dbg2 << "UserID: " << mess->getUserID() << endl;

   /* STX */
   incWriteByte(buf, pos, 0x02);
   /* Length is good for you */
   incWriteLong(buf, pos, mess->getLength());
   incWriteByte(buf, pos, mess->getProtoVer());
   incWriteShort(buf, pos, uint16(mess->getType()));
   incWriteByte(buf, pos, mess->getReqID());
   if ( mess->getProtoVer() >= 5 ) { // Version 5
      incWriteLong( buf, pos, mess->getCRC() );
      incWriteByte( buf, pos, static_cast<const OutGoingNavMessage*>( 
                       mess )->getStatus() );
      mc2dbg2 << "Status: 0x" << hex 
              << uint32( static_cast<const OutGoingNavMessage*>( mess )
                         ->getStatus() ) << dec << endl;
   } else { // Version 0-4
      incWriteShort(buf, pos, mess->getCRC());
      if (mess->getProtoVer() > 0) {
         incWriteLong(buf, pos, mess->getNavID());
      }
      if (mess->getProtoVer() == 1) {
         //removed for userName
         incWriteLong(buf, pos, mess->getUserID());
      }
   }
   mc2dbg2 << "Outgoing header size " << pos << endl;
   return pos;   
}


void 
isabBoxNavMessageUtil::writeCRC( byte* buf, uint8 protoVer, uint32 crc ) {
   int pos = 9;
   if ( protoVer >= 0x05 ) {
      incWriteLong( buf, pos, crc );
   } else { // Version 0-4
      incWriteShort( buf, pos, crc );
   }
}


uint32 
isabBoxNavMessageUtil::readCRC( byte* buf, uint8 protoVer ) {
   int pos = 9;
   if ( protoVer >= 0x05 ) {
      return incReadLong( buf, pos );
   } else { // Version 0-4
      return incReadShort( buf, pos );
   } 
}


uint8
isabBoxNavMessageUtil::incReadByte(const byte* buf,
                                   int& pos)
{
   return buf[pos++];
}

uint16
isabBoxNavMessageUtil::incReadShort(const byte* buf,
                                    int& pos)
{
   uint16 result = (uint16(buf[pos]) << 8) | buf[pos+1];
   pos += 2;
   return result;
}

uint32
isabBoxNavMessageUtil::incReadLong(const byte* buf,
                                   int& pos)
{
   uint32 result = 0;
   for(int i=0; i < 4; i++ ) {
      result <<= 8;
      result |= buf[pos+i];
   }
   pos += 4;
   return result;
}

char*
isabBoxNavMessageUtil::incReadString(const byte* buf,
                                     int& pos)
{
   char* s = (char*)(&buf[pos]);
   pos += strlen( (char*)(&buf[pos]) ) + 1;
   return StringUtility::newStrDup( UTF8Util::isoToMc2( s ).c_str() );
}

int
isabBoxNavMessageUtil::incWriteByte(byte* buf, int& pos, byte val)
{
   buf[pos++] = val;
   return pos;
}

int
isabBoxNavMessageUtil::incWriteShort(byte* buf, int& pos, uint16 val)
{
   buf[pos++] = val >> 8;
   buf[pos++] = val & 0xff;
   return pos;
}

int
isabBoxNavMessageUtil::incWriteLong(byte* buf, int& pos, uint32 val)
{
   buf[pos++] = (val >> 24) & 0xff;
   buf[pos++] = (val >> 16) & 0xff;
   buf[pos++] = (val >>  8) & 0xff;
   buf[pos++] = (val >>  0) & 0xff;
   return pos;
}

int
isabBoxNavMessageUtil::incWriteNavRad(byte* buf, int& pos, int32 mc2)
{
   return isabBoxNavMessageUtil::incWriteLong(buf,
                                              pos,
                                              isabBoxNavMessageUtil::MC2ToNavRad(mc2));
}

int
isabBoxNavMessageUtil::incWriteTurn( byte* buf,
                                     int& pos,
                                     const RouteElement* re,
                                     int protoVer )
{
   return isabBoxNavMessageUtil::incWriteByte(
      buf, pos, isabBoxNavMessageUtil::turnCodeToIsabBox( 
         re, protoVer ) );
}

int
isabBoxNavMessageUtil::incWriteString(byte* buf,
                                      int& pos,
                                      const char* str)
{
   MC2String isoStr = UTF8Util::mc2ToIso(MC2String(str));
   strcpy((char*)(buf + pos), isoStr.c_str());
   int len = isoStr.length() + 1;
   pos += len;
   return len;
}

int
isabBoxNavMessageUtil::incWriteStringUTF8(byte* buf,
                                          int& pos,
                                          const char* str)
{
   // Non-optimized
   MC2String utf8Str = UTF8Util::mc2ToUtf8( str );
   strcpy((char*)(buf + pos), utf8Str.c_str());
   int len = utf8Str.length() + 1;
   pos += len;
   return len;
}
                                   

uint16
isabBoxNavMessageUtil::turnCodeToIsabBox( const RouteElement* re, 
                                          int protoVer )
{
   int i = 0;
   StringTable::stringCode code = re->getTurnCode();

   if ( protoVer >= 0x07 ) {
      while ( turnCodeTable7p[ i ].isabboxcode != -1 ) {
         if ( turnCodeTable7p[ i ].mc2code == code ) {
            // Roundabout hack 20030912
            if ( (turnCodeTable7p[ i ].mc2code == 
                  StringTable::EXIT_ROUNDABOUT_TURN ||
                  turnCodeTable7p[ i ].mc2code == 
                  StringTable::EXIT_ROUNDABOUT_TURN_WALK) &&
                 re->getExitCount() >= 8 )
            {
               // Change
               RouteElement* ere = const_cast< RouteElement* > ( re );
               if ( re->getExitCount() < 16 ) {
                  i++; // rdt 8-15
                  ere->setExitCount( re->getExitCount() - 8 );
               } else {
                  i = i + 2; // rdt 16-23
                  ere->setExitCount( re->getExitCount() - 16 );
               }
            }
            return turnCodeTable7p[ i ].isabboxcode;
         }
         i++;
      }
   } else { // Version 6 and down
      while ( turnCodeTable[i].isabboxcode != -1 ) {
         if ( turnCodeTable[i].mc2code == code ) {
            return turnCodeTable[i].isabboxcode;
         }
         i++;
      }
   }
   mc2log << error << "isabBoxNavMessageUtil::turnCodeToIsabBox turn \""
          << StringTable::getString( code, StringTable::ENGLISH ) << "\" ("
          << int(code) << ") not supported for protoVer " << protoVer
          << " Sending 0x3ff." << endl;
   return 0x3ff; /* Maximum allowed turncode, unknown. */
}


StringTable::stringCode
isabBoxNavMessageUtil::isabBoxToTurnCode( uint16 turnCode, byte protoVer ) {
   StringTable::stringCode res = StringTable::NOTOK;
   const turnCodeTable_t* table = 
      protoVer >= 0x07 ? turnCodeTable7p : turnCodeTable;

   int i = 0;
   while ( table[ i ].isabboxcode != -1 ) {
      if ( table[ i ].isabboxcode == turnCode ) {
         res = table[ i ].mc2code;
         break;
      }
      ++i;
   }

   return res;
}

byte
isabBoxNavMessageUtil::attributeToFlags(byte attributes)
{
   bool autobahn, ramp;
   bool driveOnRightSide = true;
   byte flags;

   /* Ignore the ramp for now */
   flags=0;
   ExpandRouteReplyPacket::extractAttributes( attributes, autobahn, ramp,
                                              driveOnRightSide );
   if (autobahn) {
      flags |= 0x01;
   }
   if ( !driveOnRightSide ) { // isLeftTraffic
      flags |= 0x02;
   }
   return flags;
}

bool
isabBoxNavMessageUtil::validNavCoordinates(uint32 lat, uint32 lon)
{
   if (lat == 0 && lon == 0) {
      /* Actually, this is not an invalid coordinate, it */
      /* just happens to lie in the Atlantic Ocean next to Africa... */
      return false;
   }
   if (lat == MAX_INT32) {
      /* We should probably compare against latitudes north */
      /* of the north pole, and latitudes south of the south pole. */
      return false;
   }
   return true;
}


int32
isabBoxNavMessageUtil::navRadToMC2
(int32 navrad)
{
   return int32( navrad * GfxConstants::radianFactor / 100000000);
}

int32
isabBoxNavMessageUtil::MC2ToNavRad(int32 mc2)
{
   return int32( mc2 * GfxConstants::invRadianFactor * 100000000);
}


isabBoxTypeMessage::isabBoxTypeMessage(const NavAddress& senderAddress,
                                       const byte* buf,
                                       int bufLen,
                                       NavSession * session)
      : IncomingNavMessage(senderAddress, NavMessage::INVALID, false, session)
{
   if ( isabBoxNavMessageUtil::convertHeaderFromBytes(this, buf, bufLen) )
   {
      mc2dbg2 << "isabBoxTypeMessage::isabBoxTypeMessage "
                 "convertHeaderFromBytes returned true." << endl;
   } else {
      m_type = INVALID;
   }
   
}

bool
isabBoxTypeMessage::convertFromBytes(const byte* buf,
                                  int bufSize)
{
   return isabBoxNavMessageUtil::convertHeaderFromBytes(this, buf, bufSize);
}

int
isabBoxTypeMessage::getHeaderSize( int protoVer, bool reply )
{
   if (protoVer == 0) {
      return ISABBOX_MESSAGE_HEADER_SIZE_VER_0;      
   } else if (protoVer == 1) {
      return ISABBOX_MESSAGE_HEADER_SIZE_VER_1;
   } else if (protoVer >= 5 ) {
      if ( reply ) {
         return ISABBOX_MESSAGE_HEADER_SIZE_VER_5_REPLY;
      } else {
         return ISABBOX_MESSAGE_HEADER_SIZE_VER_5;
      }
   } else {
      if ( reply ) {
         // YES testing
         return ISABBOX_MESSAGE_HEADER_SIZE_VER_2_REPLY;
      } else {
         return ISABBOX_MESSAGE_HEADER_SIZE_VER_2;
      }
   }
}


/* isabBoxReplyMessage */

isabBoxReplyMessage::isabBoxReplyMessage( 
   const NavAddress& recipientAddress,
   MessageType type,
   uint8 status,
   NavSession* session, 
   uint8 protoVer )
      : OutGoingNavMessage( recipientAddress, type, false, session )
{
   setStatus( status );
   setProtoVer( protoVer );
}


bool 
isabBoxReplyMessage::convertToBytes( byte* buf,
                                     int bufSize )
{
   int pos = isabBoxTypeMessage::getHeaderSize( getProtoVer() );  
   if ( pos + 1 < bufSize ) { // Enough space
      if ( getProtoVer() >= 0x05 ) {
         setLength( pos );
      } else {
         setLength( pos + 1 ); // + 1 status byte
      }
      // Header
      isabBoxNavMessageUtil::convertHeaderToBytes( this, buf, bufSize );
      if ( getProtoVer() < 0x05 ) { // Add status
         // Write status
         isabBoxNavMessageUtil::incWriteByte( buf, pos, getStatus() );
      }

      return true;
   } else {
      return false;
   }
}

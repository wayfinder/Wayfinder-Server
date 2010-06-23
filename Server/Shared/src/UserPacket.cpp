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

#include "UserData.h"
#include "UserLicenceKey.h"
#include "UserRight.h"
#include "UserIDKey.h"
#include "UserPacket.h"


// === UserRequestPacket ===============================
UserRequestPacket::UserRequestPacket()
{
   mc2log << error << "This constructor shall not be used!" << endl;
}


UserRequestPacket::UserRequestPacket( uint32 bufLength,
                                      uint16 subType,
                                      uint16 packetID,
                                      uint32 requestID,
                                      uint32 UIN )
      : RequestPacket( bufLength,
                       USER_REQUEST_PRIO,
                       subType,
                       packetID,
                       requestID,
                       MAX_UINT32 ) // MapID not availiable
{
   writeLong( REQUEST_HEADER_SIZE, UIN );
   setLength( REQUEST_HEADER_SIZE + 4 );
}


UserRequestPacket::UserRequestPacket( uint32 bufLength,
                                      uint16 subType,
                                      uint32 UIN )
      : RequestPacket( bufLength,
                       USER_REQUEST_PRIO,
                       subType,
                       0,
                       0,
                       MAX_UINT32 ) // MapID not availiable
{
   writeLong( REQUEST_HEADER_SIZE, UIN );
   setLength( REQUEST_HEADER_SIZE + 4 );
}
                       

uint32 UserRequestPacket::getUIN() const
{
   return readLong(REQUEST_HEADER_SIZE);
}


// === UserReplyPacket ===============================
UserReplyPacket::UserReplyPacket()
{
   mc2log << error << "This constructor shall not be used!" << endl;
}


UserReplyPacket::UserReplyPacket( uint32      bufLength,
                                  uint16      subType,
                                  const UserRequestPacket* inpacket)
      : ReplyPacket( bufLength,
                     subType,
                     inpacket,
                     StringTable::OK ) // Status
{
   writeLong( REPLY_HEADER_SIZE, inpacket->getUIN() );
   setLength( REPLY_HEADER_SIZE + 4 );
}


UserReplyPacket::UserReplyPacket( uint32 bufLength,
                                  uint16 subType )
   : ReplyPacket( bufLength,
                  subType ) 
{
   setStatus( StringTable::OK );
}


uint32 UserReplyPacket::getUIN() const
{
   return readLong( REPLY_HEADER_SIZE );
}




// === GetUserDataRequestPacket ==============================
GetUserDataRequestPacket::GetUserDataRequestPacket(
   uint16 packetID,
   uint16 requestID,
   uint32 UIN,
   uint32 elementTypes,
   bool notFromCache )
      : UserRequestPacket( USER_REQUEST_HEADER_SIZE + 5,
                           PACKETTYPE_USERGETDATAREQUEST,
                           packetID, 
                           requestID,
                           UIN ) 
{
   int pos = USER_REQUEST_HEADER_SIZE;
   mc2dbg8 << "GetUserDataRequestPacket::GetUserDataRequestPacket "
           << "elementTypes: " << elementTypes  << endl;
   incWriteLong( pos, elementTypes );
   incWriteByte( pos, notFromCache );
   setLength( pos );
}


uint32
GetUserDataRequestPacket::getElementType() const {
   return readLong( USER_REQUEST_HEADER_SIZE );
}


bool
GetUserDataRequestPacket::getNotFromCache() const {
   return readByte( USER_REQUEST_HEADER_SIZE + 4 );
}


// === GetUserDataReplyPacket ==============================
GetUserDataReplyPacket::GetUserDataReplyPacket( 
   const GetUserDataRequestPacket* inPacket,
   uint16 bufLength )
      : UserReplyPacket( bufLength, 
                         PACKETTYPE_USERGETDATAREPLY, 
                         inPacket )
{
   // Write header
   int pos = USER_REQUEST_HEADER_SIZE;
   
   incWriteLong( pos, inPacket->getElementType() );
   // No fields
   incWriteShort( pos, 0 );
   // End of packet
   incWriteLong( pos, inPacket->getElementType() );
   setLength( pos );
}


GetUserDataReplyPacket::GetUserDataReplyPacket( uint16 bufLength )
      : UserReplyPacket( bufLength, 
                         PACKETTYPE_USERGETDATAREPLY )  
{
   // Write header
   int pos = USER_REQUEST_HEADER_SIZE;
   
   incWriteLong( pos, UserConstants::TYPE_ALL );
   // No fields
   incWriteShort( pos, 0 );
   // End of packet
   incWriteLong( pos, UserConstants::TYPE_ALL );
   setLength( pos );   
}


bool
GetUserDataReplyPacket::addUserElement( UserElement* elem ) {
   int pos = getLength() - 4; // Last ElementType overwritten and rewritten

   mc2dbg4 << "GetUserDataReplyPacket::addUserElement pos = "
           << pos << endl;

   if ( ( getBufSize() - pos ) > elem->getSize() ) {
      elem->packInto( this, pos );
      // Add to counter
      setNbrFields( getNbrFields() + 1 );
      mc2dbg8 << "Element end @ " << pos << endl;
      // End of packet
      incWriteLong( pos, readLong( USER_REQUEST_HEADER_SIZE ) );
      mc2dbg8 << "AddElement end @ " << pos << endl;
      setLength( pos );
      return true;
   } else {
      return false;
   }
}


uint16
GetUserDataReplyPacket::getNbrFields() const {
   return readShort( USER_REQUEST_HEADER_SIZE + 4);
}


void
GetUserDataReplyPacket::setNbrFields( uint16 nbrFields ) {
   writeShort( USER_REQUEST_HEADER_SIZE + 4, nbrFields );
}


int
GetUserDataReplyPacket::getFirstElementPosition() const {
   int pos = USER_REQUEST_HEADER_SIZE + 6;
   return pos;
}


UserElement*
GetUserDataReplyPacket::getNextElement( int& pos ) const {
   // Align if nessesary
   AlignUtility::alignLong( pos );
   mc2dbg4 << "GetUserDataReplyPacket::getNextElement " 
          << "pos " << pos << endl;

   if ( (uint32)(pos + 6) >= getLength() ) {
      mc2dbg4 << "GetUserDataReplyPacket::getNextElement " 
              << "Not more data in packet NULL returned" << endl;
      return NULL;
   } 

   uint32 type = incReadLong( pos );
   uint16 size = incReadShort( pos );

   if ( (uint32)( pos - 12 + size ) >= getLength() ) {
      mc2dbg4 << "GetUserDataReplyPacket::getNextElement " 
              << "Size of Element to large " << size
              << " bytes for packet. NULL returned" << endl;
      return NULL;
   }

   UserElement* elem = NULL;
   switch ( type ) {
      case UserConstants::TYPE_USER :
         elem = new UserUser( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || (type != UserConstants::TYPE_USER) ) {
            mc2dbg4 << "GetUserDataReplyPacket::getNextElement "
                    << "UserUser not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserUser OK" << endl;
         }
         break;
      case UserConstants::TYPE_CELLULAR :
         elem = new UserCellular( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_CELLULAR) ) {
            mc2dbg4 << "GetUserDataReplyPacket::getNextElement "
                    << "UserCellular not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserCellular OK" << endl;
         }
         break;
      case UserConstants::TYPE_BUDDY :
         elem = new UserBuddyList( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_BUDDY) ) {
            mc2dbg4 << "GetUserDataReplyPacket::getNextElement "
                    << "UserBuddyList not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserBUddyList OK" << endl;
         }
         break;
      case UserConstants::TYPE_NAVIGATOR :
         elem = new UserNavigator( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_NAVIGATOR) ) {
            mc2dbg4 << "GetUserDataReplyPacket::getNextElement "
                    << "UserNavigator not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserNavigator OK" << endl;
         }
         break;
      case UserConstants::TYPE_LICENCE_KEY :
         elem = new UserLicenceKey( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_LICENCE_KEY) )
         {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserLicenceKey not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserLicenceKey OK" << endl;
         }
         break;
      case UserConstants::TYPE_REGION_ACCESS :
         elem = new UserRegionAccess( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || 
              ( type != UserConstants::TYPE_REGION_ACCESS) )
         {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserRegionAccess not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserRegionAccess OK" << endl;
         }
         break;
      case UserConstants::TYPE_RIGHT :
         elem = new UserRight( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || 
              ( type != UserConstants::TYPE_RIGHT) )
         {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserRight not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserRight OK" << endl;
         }
         break;
      case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION :
         elem = new UserWayfinderSubscription( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || 
              ( type != UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) )
         {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserWayfinderSubscription not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserWayfinderSubscription OK" << endl;
         }
         break;
      case UserConstants::TYPE_TOKEN :
         elem = new UserToken( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_TOKEN ) ) {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserToken not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserToken OK" << endl;
         }
         break;

      case UserConstants::TYPE_PIN :
         elem = new UserPIN( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_PIN ) ) {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserPIN not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserPIN OK" << endl;
         }
         break;

      case UserConstants::TYPE_ID_KEY :
         elem = new UserIDKey( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_ID_KEY ) ) {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserIDKey not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserIDKey OK" << endl;
         }
         break;

      case UserConstants::TYPE_LAST_CLIENT :
         elem = new UserLastClient( this, pos );
         type = incReadLong( pos );
         if ( !elem->isOk() || ( type != UserConstants::TYPE_LAST_CLIENT ))
         {
            mc2log << warn  << "GetUserDataReplyPacket::getNextElement "
                   << "UserLastClient not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "GetUserDataReplyPacket::getNextElement "
                    << "UserLastClient OK" << endl;
         }
         break;

      default:
         mc2log << warn << "GetUserDataReplyPacket::getNextElement " 
                << " unsupported TYPE: " << (int)type 
                << "=" << hex << (int) type << dec << endl;
         break;
   }

   return elem;
}


// ===== AddUserRequestPacket ==============================
AddUserRequestPacket::AddUserRequestPacket( uint16 packetID, 
                                            uint16 requestID,
                                            UserUser* user,
                                            const char* passwd,
                                            uint32 changerUIN ) 
      : UserRequestPacket( MAX_PACKET_SIZE + strlen( passwd ) + 1,
                           PACKETTYPE_USERADDREQUEST,
                           packetID,
                           requestID,
                           0 )
{
   int pos  = USER_REQUEST_HEADER_SIZE + 2;

   uint16 length = strlen( passwd );
   incWriteShort( pos, length );
   incWriteLong( pos, changerUIN );
   incWriteString( pos, passwd );
   
   setNbrElements( 1 );
   user->addChanges( this, pos );
   setLength( pos );
}


uint16 
AddUserRequestPacket::getNbrElements() const {
   return readShort( USER_REQUEST_HEADER_SIZE );
}


bool
AddUserRequestPacket::addElement( UserElement* elem ) {
   int pos = getLength();

   mc2dbg4 << "AddUserRequestPacket::addElement pos = " << pos 
           << endl;

   if ( ( getBufSize() - pos ) > elem->getSize() ) {
      elem->addChanges( this, pos );
      setNbrElements( getNbrElements() + 1 );
      setLength( pos );
      return true;
   } else {
      mc2dbg1 << "AddUserRequestPacket::addElement no room for element"
             << endl;
      return false; 
   }
}


int
AddUserRequestPacket::getFirstElementPosition() const {
   int pos = USER_REQUEST_HEADER_SIZE + 4 + 4 + getPasswordLength() + 1;
   return pos;
}


UserElement* 
AddUserRequestPacket::getNextElement( int& pos ) const {
   mc2dbg4 << "AddUserRequestPacket::getNextElement " 
           << "pos " << pos << endl;

   if ( (uint32)(pos + 5) >= getLength() ) {
      mc2dbg << "AddUserRequestPacket::getNextElement " 
              << "Not more data in packet NULL returned" << endl;
      return NULL;
   } 

   uint32 type = incReadLong( pos );
   uint32 ID = incReadLong( pos );

   if ( ID != 0 ) {
      mc2dbg << "AddUserRequestPacket::getNextElement " 
              << " ID is not 0, not new element! returning NULL" << endl;
      return NULL;
   }

   UserElement* elem = NULL;
   switch ( type ) {
      case UserConstants::TYPE_USER : {
         elem = new UserUser( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || (type != UserConstants::TYPE_USER) || 
              action != UserConstants::ACTION_NEW ) 
         {
            mc2dbg << "AddUserRequestPacket::getNextElement "
                    << "UserUser not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserUser OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_CELLULAR : {
         elem = new UserCellular( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_CELLULAR) ||
              action != UserConstants::ACTION_NEW ) {
            mc2dbg2 << "AddUserRequestPacket::getNextElement "
                    << "UserCellular not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserCellular OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_BUDDY : {
         elem = new UserBuddyList( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_BUDDY) ||
              action != UserConstants::ACTION_NEW ) {
            mc2dbg2 << "AddUserRequestPacket::getNextElement "
                    << "UserBuddyList not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserBuddyList OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_NAVIGATOR : {
         elem = new UserNavigator( (uint32) 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_NAVIGATOR) ||
              action != UserConstants::ACTION_NEW ) {
            mc2dbg2 << "AddUserRequestPacket::getNextElement "
                   << "UserNavigator not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserNavigator OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_LICENCE_KEY : {
         elem = new UserLicenceKey( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_LICENCE_KEY) ||
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserLicenceKey not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserLicenceKey OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_REGION_ACCESS : {
         elem = new UserRegionAccess( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_REGION_ACCESS) ||
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserRegionAccess not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserRegionAccess OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_RIGHT : {
         elem = new UserRight( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_RIGHT) ||
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserRight not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserRight OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION : {
         elem = new UserWayfinderSubscription( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_WAYFINDER_SUBSCRIPTION)
              || action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserWayfinderSubscription not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserWayfinderSubscription OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_TOKEN : {
         elem = new UserToken( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_TOKEN) || 
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserToken not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserToken OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_PIN : {
         elem = new UserPIN( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || ( type != UserConstants::TYPE_PIN) || 
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserPIN not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserPIN OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_ID_KEY : {
         elem = new UserIDKey( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || (type != UserConstants::TYPE_ID_KEY) || 
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserIDKey not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserIDKey OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_LAST_CLIENT : {
         elem = new UserLastClient( 0 );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || (type != UserConstants::TYPE_LAST_CLIENT) || 
              action != UserConstants::ACTION_NEW ) 
         {
            mc2log << warn << "AddUserRequestPacket::getNextElement "
                   << "UserLastClient not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "AddUserRequestPacket::getNextElement "
                    << "UserLastClient OK" << endl;
         }
      }
      break;
      default:
         mc2log << error << "AddUserRequestPacket::getNextElement " 
                << " unsupported TYPE: " << (int)type << endl;
         break;
   }

   return elem;
}


const char* 
AddUserRequestPacket::getPassword() const {
   char* str;
   int pos = USER_REQUEST_HEADER_SIZE + 4 + 4;
   incReadString( pos, str);
   return str;
}


void 
AddUserRequestPacket::setNbrElements( uint16 nbrElements ) {
   writeShort( USER_REQUEST_HEADER_SIZE, nbrElements );
}


uint16 
AddUserRequestPacket::getPasswordLength() const {
   return readShort( USER_REQUEST_HEADER_SIZE + 2 );
}


uint32
AddUserRequestPacket::getChangerUIN() const {
   return readLong( USER_REQUEST_HEADER_SIZE + 4 );
}


// ===== AddUserReplyPacket ================================
AddUserReplyPacket::AddUserReplyPacket( const AddUserRequestPacket* packet, 
                                        uint32 UIN)
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 4, 
                         PACKETTYPE_USERADDREPLY,
                         packet )
{
   writeLong( USER_REPLY_HEADER_SIZE, UIN );
   setLength( USER_REPLY_HEADER_SIZE + 4 );
}


uint32
AddUserReplyPacket::getUIN() const {
   return readLong( USER_REPLY_HEADER_SIZE );
}


// ===== DeleteUserRequestPacket ===========================
DeleteUserRequestPacket::DeleteUserRequestPacket( uint16 packetID, 
                                                  uint16 requestID,
                                                  uint32 UIN )
      : UserRequestPacket( USER_REQUEST_HEADER_SIZE,
                           PACKETTYPE_USERDELETEREQUEST,
                           packetID,
                           requestID,
                           UIN )
{
}


// ===== DeleteUserReplyPacket =============================
DeleteUserReplyPacket::DeleteUserReplyPacket( 
   const DeleteUserRequestPacket* packet,
   bool ok )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 1, 
                         PACKETTYPE_USERDELETEREPLY,
                         packet )
{
   int pos = USER_REPLY_HEADER_SIZE;
   
   incWriteByte( pos, ok );
   setLength( pos );
}


bool
DeleteUserReplyPacket::operationOK() const {
   return readByte( USER_REPLY_HEADER_SIZE ) != 0 ;
}


// ===== ChangeUserDataRequestPacket =======================
ChangeUserDataRequestPacket::ChangeUserDataRequestPacket(
   uint16 packetID, 
   uint16 requestID,
   uint32 UIN,
   uint32 changerUIN )
      : UserRequestPacket( MAX_PACKET_SIZE,
                           PACKETTYPE_USERCHANGEREQUEST,
                           packetID,
                           requestID,
                           UIN )   
{
   setNbrElements( 0 );
   int pos = USER_REQUEST_HEADER_SIZE + 2;
   incWriteLong( pos, changerUIN );
   setLength( pos );
}


bool 
ChangeUserDataRequestPacket::addChangedElement( UserElement* elem ) {
   int pos = getLength();
   
   mc2dbg4 << "ChangeUserRequestPacket::addChangedElement pos = " 
           << pos << endl;

   if ( ( getBufSize() - pos ) > elem->getSize() ) {
      elem->addChanges( this, pos );
      setNbrElements( getNbrElements() + 1 );
      setLength( pos );
      return true;
   } else {
      mc2log << warn << "ChangeUserRequestPacket::addElement no room for"
             << " element" << endl;
      return false; 
   }
}


uint16 
ChangeUserDataRequestPacket::getNbrElements() const {
   return readShort( USER_REQUEST_HEADER_SIZE );
}


uint32
ChangeUserDataRequestPacket::getChangerUIN() const {
   return readLong( USER_REQUEST_HEADER_SIZE + 2 );
}


int
ChangeUserDataRequestPacket::getFirstElementPosition() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2 + 4;
   return pos;
}


UserElement* 
ChangeUserDataRequestPacket::getNextElement( int& pos ) const {
   mc2dbg4 << "ChangeUserDataRequestPacket::getNextElement " 
          << "pos " << pos << endl;

   if ( (uint32)(pos + 8) >= getLength() ) {
      mc2dbg1 << "ChangeUserDataRequestPacket::getNextElement " 
             << "Not more data in packet NULL returned" << endl;
      return NULL;
   } 

   uint32 type = incReadLong( pos );
   uint32 ID = incReadLong( pos );

  //   if ( ID == 0 ) {
//       mc2dbg1 << "ChangeUserDataRequestPacket::getNextElement " 
//               " ID is 0, not an element! returning NULL" << endl;
//        return NULL;
//     }

   UserElement* elem = NULL;
   switch ( type ) {
      case UserConstants::TYPE_USER : {
         elem = new UserUser( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         if ( !ok || (type != UserConstants::TYPE_USER) || 
              action != UserConstants::ACTION_CHANGE ) 
         {
            mc2dbg1 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserUser not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserUser OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_CELLULAR : {
         elem = new UserCellular( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_CELLULAR) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2dbg2 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserCellular not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserCellular OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_BUDDY : {
         elem = new UserBuddyList( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_BUDDY) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2dbg2 << "ChangeUserDataRequestPacket::getNextElement "
                   << "UserBuddyList not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserBuddyList OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_NAVIGATOR : {
         elem = new UserNavigator( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_NAVIGATOR) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2dbg1 << "ChangeUserDataRequestPacket::getNextElement "
                   << "UserNavigator not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserNavigator OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_LICENCE_KEY : {
         elem = new UserLicenceKey( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_LICENCE_KEY) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  " UserLicenceKey not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserLicenceKey OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_REGION_ACCESS : {
         elem = new UserRegionAccess( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_REGION_ACCESS) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  " UserRegionAccess not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserRegionAccess OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_RIGHT : {
         elem = new UserRight( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_RIGHT) ){ // ||
            //  action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  " UserRight not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserRight OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION : {
         elem = new UserWayfinderSubscription( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != 
                       UserConstants::TYPE_WAYFINDER_SUBSCRIPTION) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  " UserWayfinderSubscription not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserWayfinderSubscription OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_TOKEN : {
         elem = new UserToken( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_TOKEN) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  " UserToken not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserToken OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_PIN : {
         elem = new UserPIN( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_PIN) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  " UserPIN not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserPIN OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_ID_KEY : {
         elem = new UserIDKey( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_ID_KEY) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  "UserIDKey  not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserIDKey OK" << endl;
         }
      }
      break;
      case UserConstants::TYPE_LAST_CLIENT : {
         elem = new UserLastClient( ID );
         UserConstants::UserAction action;
         bool ok = elem->readChanges( this, pos, action );
         type = incReadLong( pos );
         mc2dbg4 << "ok : " << ok << " type : " << type << endl;
         if ( !ok || ( type != UserConstants::TYPE_LAST_CLIENT) ){ // ||
            //   action != UserConstants::ACTION_CHANGE ) {
            mc2log << warn << "ChangeUserDataRequestPacket::getNextElement"
                   <<  "UserLastClient  not ok" << endl;
            delete elem;
            elem = NULL;
         } else {
            mc2dbg8 << "ChangeUserDataRequestPacket::getNextElement "
                    << "UserLastClient OK" << endl;
         }
      }
      break;

      default:
         mc2log << warn << "ChangeUserDataRequestPacket::getNextElement " 
                << " unsupported TYPE: " << (int)type << endl;
         break;
   }
   return elem;
}


void
ChangeUserDataRequestPacket::setNbrElements( uint16 nbrElements ) {
   writeShort( USER_REQUEST_HEADER_SIZE, nbrElements );
}


// ===== ChangeUserDataReplyPacket =========================
ChangeUserDataReplyPacket::ChangeUserDataReplyPacket(  
   const ChangeUserDataRequestPacket* packet,
   bool ok )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 1, 
                         PACKETTYPE_USERCHANGEREPLY,
                         packet )

{
   int pos = USER_REPLY_HEADER_SIZE;
   
   incWriteByte( pos, ok );
   setLength( pos );
}


bool
ChangeUserDataReplyPacket::operationOK() const {
   return readByte( USER_REPLY_HEADER_SIZE ) != 0;
}


// ===== CheckUserPasswordRequestPacket ====================
CheckUserPasswordRequestPacket::CheckUserPasswordRequestPacket(
   uint16 packetID,
   uint16 requestID,
   const char* logonID,
   const char* password,
   bool checkExpired )
      : UserRequestPacket( ( USER_REQUEST_HEADER_SIZE + 2 + 2 + 1 +
                             strlen( logonID ) + 1 +
                             strlen( password ) + 1 ),
                           PACKETTYPE_USERCHECKPASSWORDREQUEST,
                           packetID,
                           requestID,
                           0 )
{
   int pos = USER_REQUEST_HEADER_SIZE;

   incWriteShort( pos, strlen(logonID) );
   incWriteShort( pos, strlen(password) );
   incWriteByte( pos, checkExpired );
   incWriteString( pos, logonID );
   incWriteString( pos, password );
   setLength( pos );
}


const char*
CheckUserPasswordRequestPacket::getLogonID() const {
   int pos = USER_REQUEST_HEADER_SIZE;

   incReadShort( pos ); // LogonID Size

   incReadShort( pos ); // Skipp password size

   incReadByte( pos ); // Skipp checkExpired

   char* logonID;
   incReadString( pos, logonID );

   return logonID;
}


const char*
CheckUserPasswordRequestPacket::getPassword() const {
   int pos = USER_REQUEST_HEADER_SIZE;
   
   uint16 logonIDSize = incReadShort( pos ); // LogonID Size

   incReadShort( pos ); // Skipp password size

   incReadByte( pos ); // Skipp checkExpired

   pos += logonIDSize + 1;

   char* passwd;
   incReadString( pos, passwd );

   return passwd;
}


bool 
CheckUserPasswordRequestPacket::getExpired() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2 + 2;

   return readByte( pos ) != 0;
}


// ===== CheckUserPasswordReplyPacket ========================
CheckUserPasswordReplyPacket::CheckUserPasswordReplyPacket(
   const CheckUserPasswordRequestPacket* packet,
   uint32 UIN,
   const char* sessionID,
   const char* sessionKey )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 4 + 
                         strlen( sessionID ) + 1 + 2 +
                         strlen( sessionKey ) + 1 + 2, 
                         PACKETTYPE_USERCHECKPASSWORDREPLY,
                         packet )
{
   int pos = USER_REPLY_HEADER_SIZE;
   
   incWriteLong( pos,  UIN );
   incWriteShort( pos,  strlen( sessionID ) );
   incWriteShort( pos,  strlen( sessionKey ) );
   incWriteString( pos, sessionID );
   incWriteString( pos, sessionKey );
   setLength( pos );
}


uint32
CheckUserPasswordReplyPacket::getUIN() const {
   return readLong( USER_REPLY_HEADER_SIZE );
}


const char* 
CheckUserPasswordReplyPacket::getSessionID() const {
   int pos = USER_REPLY_HEADER_SIZE + 4 + 2 + 2;
   char* str = NULL;
   incReadString( pos, str );

   return str;
}


const char* 
CheckUserPasswordReplyPacket::getSessionKey() const {
   int pos = USER_REPLY_HEADER_SIZE + 4 + 2;
   char* str = NULL;
   
   pos += readShort( USER_REPLY_HEADER_SIZE + 4 );
   pos += 3; // Skipp length int and terminating byte
   incReadString( pos, str );

   return str;
}


// ===== VerifyUserRequestPacket =============================
VerifyUserRequestPacket::VerifyUserRequestPacket( uint16 packetID,
                                                  uint16 requestID,
                                                  const char* sessionID,
                                                  const char* sessionKey,
                                                  bool checkExpired )
      : UserRequestPacket( ( USER_REQUEST_HEADER_SIZE + 1
                             + strlen( sessionID ) + 1 + 2
                             + strlen( sessionKey ) + 1 + 2),
                           PACKETTYPE_USERVERIFYREQUEST,
                           packetID,
                           requestID,
                           MAX_UINT32 )
{
   int pos = USER_REQUEST_HEADER_SIZE;
   
   incWriteShort( pos,  strlen( sessionID ) );
   incWriteShort( pos,  strlen( sessionKey ) );
   incWriteByte( pos, checkExpired );
   incWriteString( pos, sessionID );
   incWriteString( pos, sessionKey );
   setLength( pos );   
}


const char*
VerifyUserRequestPacket::getSessionID() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2 + 2 + 1;
   char* str = NULL;
   incReadString( pos, str );

   return str;  
}


const char*
VerifyUserRequestPacket::getSessionKey() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2;
   char* str = NULL;
   
   pos += readShort( USER_REQUEST_HEADER_SIZE );
   pos += 3 + 1; // Skipp length int and terminating byte + checkExpired 
   incReadString( pos, str );

   return str;
}


bool 
VerifyUserRequestPacket::getExpired() const {
   return readByte( USER_REQUEST_HEADER_SIZE + 2 + 2 ) != 0;
}


// ===== VerifyUserReplyPacket ===============================
VerifyUserReplyPacket::VerifyUserReplyPacket( 
   const VerifyUserRequestPacket* packet, 
   uint32 UIN  )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 4, 
                         PACKETTYPE_USERVERIFYREPLY,
                         packet )   
{
   int pos = USER_REPLY_HEADER_SIZE;
   incWriteLong( pos, UIN );
   setLength( pos );
}


uint32
VerifyUserReplyPacket::getUIN() const {
   return readLong( USER_REPLY_HEADER_SIZE );
}


// ===== LogoutUserRequestPacket =============================
LogoutUserRequestPacket::LogoutUserRequestPacket( uint16 packetID,
                                                  uint16 requestID,
                                                  const char* sessionID,
                                                  const char* sessionKey )
      : UserRequestPacket( ( USER_REQUEST_HEADER_SIZE
                             + strlen( sessionID ) + 1 + 2
                             + strlen( sessionKey ) + 1 + 2),
                           PACKETTYPE_USERLOGOUTREQUEST,
                           packetID,
                           requestID,
                           MAX_UINT32 )
{
   int pos = USER_REQUEST_HEADER_SIZE;
   
   incWriteShort( pos,  strlen( sessionID ) );
   incWriteShort( pos,  strlen( sessionKey ) );
   incWriteString( pos, sessionID );
   incWriteString( pos, sessionKey );
   setLength( pos );   
}


const char*
LogoutUserRequestPacket::getSessionID() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2 + 2;
   char* str = NULL;
   incReadString( pos, str );

   return str;  
}


const char*
LogoutUserRequestPacket::getSessionKey() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2;
   char* str = NULL;
   
   pos += readShort( USER_REQUEST_HEADER_SIZE );
   pos += 3; // Skipp length int and terminating byte
   incReadString( pos, str );

   return str;
}  


// ===== LogoutUserReplyPacket ===============================
LogoutUserReplyPacket::LogoutUserReplyPacket( 
   const LogoutUserRequestPacket* packet, uint32 UIN  )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 4, 
                         PACKETTYPE_USERLOGOUTREPLY,
                         packet )    
{
   int pos = USER_REPLY_HEADER_SIZE;

   incWriteLong( pos, UIN );
   setLength( pos );
}


uint32
LogoutUserReplyPacket::getUIN() const {
   return readLong( USER_REPLY_HEADER_SIZE );
}


// ===== SessionCleanUpRequestPacket ========================
SessionCleanUpRequestPacket::SessionCleanUpRequestPacket( 
   uint16 packetID,
   uint16 requestID,
   const char* sessionID,
   const char* sessionKey )
      : UserRequestPacket( ( USER_REQUEST_HEADER_SIZE
                             + strlen( sessionID ) + 1 + 2
                             + strlen( sessionKey ) + 1 + 2 ),
                           PACKETTYPE_USERSESSIONCLEANUPREQUEST,
                           packetID,
                           requestID,
                           MAX_UINT32 )
{
   int pos = USER_REQUEST_HEADER_SIZE;
   
   incWriteShort( pos,  strlen( sessionID ) );
   incWriteShort( pos,  strlen( sessionKey ) );
   incWriteString( pos, sessionID );
   incWriteString( pos, sessionKey );
   setLength( pos );   
}


const char*
SessionCleanUpRequestPacket::getSessionID() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2 + 2;
   char* str = NULL;
   incReadString( pos, str );

   return str;  
}


const char*
SessionCleanUpRequestPacket::getSessionKey() const {
   int pos = USER_REQUEST_HEADER_SIZE + 2;
   char* str = NULL;
   
   pos += readShort( USER_REQUEST_HEADER_SIZE );
   pos += 3; // Skipp length int and terminating byte
   incReadString( pos, str );

   return str;
}  


// ===== SessionCleanUpReplyPacket ===========================
SessionCleanUpReplyPacket::SessionCleanUpReplyPacket( 
   const SessionCleanUpRequestPacket* packet, 
   uint32 status  )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE, 
                         PACKETTYPE_USERSESSIONCLEANUPREPLY,
                         packet )
{
}


// ===== AuthUserRequestPacket ====================
AuthUserRequestPacket::AuthUserRequestPacket(
   uint16 packetID,
   uint16 requestID,
   const char* logonID,
   const char* password,
   bool checkExpired )
      : UserRequestPacket( ( USER_REQUEST_HEADER_SIZE + 2 + 2 + 1 +
                             strlen( logonID ) + 1 +
                             strlen( password ) + 1 ),
                           PACKETTYPE_USER_AUTH_REQUEST,
                           packetID,
                           requestID,
                           0 )
{
   int pos = USER_REQUEST_HEADER_SIZE;

   incWriteShort( pos, strlen(logonID) );
   incWriteShort( pos, strlen(password) );
   incWriteByte( pos, checkExpired );
   incWriteString( pos, logonID );
   incWriteString( pos, password );
   setLength( pos );
}


const char*
AuthUserRequestPacket::getLogonID() const {
   int pos = USER_REQUEST_HEADER_SIZE;

   incReadShort( pos ); // LogonID Size

   incReadShort( pos ); // Skipp password size

   incReadByte( pos ); // Skipp checkExpired

   char* logonID;
   incReadString( pos, logonID );

   return logonID;
}


const char*
AuthUserRequestPacket::getPassword() const {
   int pos = USER_REQUEST_HEADER_SIZE;
   
   uint16 logonIDSize = incReadShort( pos ); // LogonID Size

   incReadShort( pos ); // Skipp password size

   incReadByte( pos ); // Skipp checkExpired

   pos += logonIDSize + 1;

   char* passwd;
   incReadString( pos, passwd );

   return passwd;
}


bool 
AuthUserRequestPacket::getExpired() const {
   return readByte( USER_REQUEST_HEADER_SIZE + 2 + 2 );
}


// ===== AuthUserReplyPacket ========================
AuthUserReplyPacket::AuthUserReplyPacket(
   const AuthUserRequestPacket* packet,
   uint32 UIN )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 4,
                         PACKETTYPE_USER_AUTH_REPLY,
                         packet )
{
   int pos = USER_REPLY_HEADER_SIZE;
   
   incWriteLong( pos,  UIN );
   setLength( pos );
}


uint32
AuthUserReplyPacket::getUIN() const {
   return readLong( USER_REPLY_HEADER_SIZE );
}


// ===== CreateSessionRequestPacket ==========================
CreateSessionRequestPacket::CreateSessionRequestPacket( uint16 packetID,
                                                        uint16 requestID,
                                                        uint32 UIN )
      : UserRequestPacket( USER_REQUEST_HEADER_SIZE,
                           PACKETTYPE_USER_CREATE_SESSION_REQUEST,
                           packetID,
                           requestID,
                           UIN )
{
}


// ===== CreateSessionReplyPacket ===========================
CreateSessionReplyPacket::CreateSessionReplyPacket( 
   const CreateSessionRequestPacket* packet, 
   StringTable::stringCode status,
   const char* sessionID,
   const char* sessionKey )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 
                         strlen( sessionID ) + 1 + 2 +
                         strlen( sessionKey ) + 1 + 2, 
                         PACKETTYPE_USER_CREATE_SESSION_REPLY,
                         packet )
{
   int pos = USER_REPLY_HEADER_SIZE;
   
   incWriteShort( pos,  strlen( sessionID ) );
   incWriteShort( pos,  strlen( sessionKey ) );
   incWriteString( pos, sessionID );
   incWriteString( pos, sessionKey );
   setLength( pos );
}


const char* 
CreateSessionReplyPacket::getSessionID() const {
   int pos = USER_REPLY_HEADER_SIZE + 2 + 2;
   char* str = NULL;
   incReadString( pos, str );

   return str;
}


const char*
CreateSessionReplyPacket::getSessionKey() const {
   int pos = USER_REPLY_HEADER_SIZE + 2 + 2;
   char* str = NULL;
   
   pos += readShort( USER_REPLY_HEADER_SIZE );
   pos += 1; // Skipp length int and terminating byte
   incReadString( pos, str );

   return str;
}


// ===== FindUserRequestPacket ===============================
FindUserRequestPacket::FindUserRequestPacket( 
   uint16 packetID,
   uint16 requestID,
   UserElement* elem ) 
      : UserRequestPacket( ( USER_REQUEST_HEADER_SIZE + elem->getSize()*2 
                             + 8 + 2 + 16 ),
                           PACKETTYPE_USERFINDREQUEST,
                           packetID,
                           requestID,
                           MAX_UINT32 )
{
   int pos = USER_REQUEST_HEADER_SIZE;

   elem->addChanges( this, pos );
   setLength( pos );
}


UserElement* 
FindUserRequestPacket::getElement() const {
   int pos = USER_REQUEST_HEADER_SIZE;
   UserElement* reply = NULL;

   uint32 type = incReadLong( pos );
   incReadLong( pos ); // ID not used

   switch ( type ) {
      case UserConstants::TYPE_USER : {
         UserUser* user = new UserUser( this->getUIN() );
         UserConstants::UserAction action;
         if ( !user->readChanges( this, pos, action ) ) {
            delete user;
            user = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_USER ) {
            delete user;
            user = NULL;  
         }
         reply = user;
         break;
      }
      case UserConstants::TYPE_CELLULAR : {         
         UserCellular* cellular = new UserCellular( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !cellular->readChanges( this, pos, action ) ) {
            delete cellular;
            cellular = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_CELLULAR ) {
            delete cellular;
            cellular = NULL;  
         }
         reply = cellular;
         break;
      }
      case UserConstants::TYPE_BUDDY : {
         UserBuddyList* buddy = new UserBuddyList( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !buddy->readChanges( this, pos, action ) ) {
            delete buddy;
            buddy = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_BUDDY ) {
            delete buddy;
            buddy = NULL;  
         }
         reply = buddy;
         break;
      }
      case UserConstants::TYPE_NAVIGATOR : {
         UserNavigator* navi = new UserNavigator( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !navi->readChanges( this, pos, action ) ) {
            delete navi;
            navi = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_NAVIGATOR ) {
            delete navi;
            navi = NULL;  
         }
         reply = navi;
         break;
      }
      case UserConstants::TYPE_LICENCE_KEY : {         
         UserLicenceKey* licence = new UserLicenceKey( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !licence->readChanges( this, pos, action ) ) {
            delete licence;
            licence = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_LICENCE_KEY) {
            delete licence;
            licence = NULL;  
         }
         reply = licence;
         break;
      }
      case UserConstants::TYPE_REGION_ACCESS : {         
         UserRegionAccess* access = new UserRegionAccess( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !access->readChanges( this, pos, action ) ) {
            delete access;
            access = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_REGION_ACCESS ) {
            delete access;
            access = NULL;  
         }
         reply = access;
         break;
      }
      case UserConstants::TYPE_RIGHT : {         
         UserRight* right = new UserRight( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !right->readChanges( this, pos, action ) ) {
            delete right;
            right = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_RIGHT ) {
            delete right;
            right = NULL;  
         }
         reply = right;
         break;
      }
      case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION : {         
         UserWayfinderSubscription* subscr = 
            new UserWayfinderSubscription( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !subscr->readChanges( this, pos, action ) ) {
            delete subscr;
            subscr = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) {
            delete subscr;
            subscr = NULL;  
         }
         reply = subscr;
         break;
      }
      case UserConstants::TYPE_TOKEN : {         
         UserToken* t = 
            new UserToken( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !t->readChanges( this, pos, action ) ) {
            delete t;
            t = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_TOKEN ) {
            delete t;
            t = NULL;  
         }
         reply = t;
         break;
      }
      case UserConstants::TYPE_PIN : {         
         UserPIN* t = 
            new UserPIN( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !t->readChanges( this, pos, action ) ) {
            delete t;
            t = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_PIN ) {
            delete t;
            t = NULL;  
         }
         reply = t;
         break;
      }
      case UserConstants::TYPE_ID_KEY : {         
         UserIDKey* t = new UserIDKey( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !t->readChanges( this, pos, action ) ) {
            delete t;
            t = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_ID_KEY ) {
            delete t;
            t = NULL;  
         }
         reply = t;
         break;
      }
      case UserConstants::TYPE_LAST_CLIENT : {         
         UserLastClient* t = new UserLastClient( MAX_UINT32 );
         UserConstants::UserAction action;
         if ( !t->readChanges( this, pos, action ) ) {
            delete t;
            t = NULL;
         }
         type = incReadLong( pos );
         if ( type != UserConstants::TYPE_LAST_CLIENT ) {
            delete t;
            t = NULL;  
         }
         reply = t;
         break;
      }

      default:
         mc2log << warn  << "FindUserRequestPacket::getElement() unknown "
                << " TYPE " << (int)type << endl;
   };

   return reply;
}


// ===== FindUserReplyPacket ===============================
FindUserReplyPacket::FindUserReplyPacket( const FindUserRequestPacket* p,
                                          uint32 nbrUsers,
                                          uint32 UINs[],
                                          const vector<MC2String>* logonIDs )
      : UserReplyPacket( (USER_REPLY_HEADER_SIZE + 4 + 4*nbrUsers), 
                         PACKETTYPE_USERFINDREPLY,
                         p )
{
   int pos = USER_REPLY_HEADER_SIZE;

   // Resize to fit in all the strings
   uint32 stringSize = 3; // Possible padding
   if ( logonIDs != NULL ) {
      for ( uint32 i = 0 ; i < logonIDs->size() ; i++ ) {
         stringSize += (*logonIDs)[ i ].size() + 1;
      }
   }
   resize( getBufSize() + stringSize );

   incWriteLong( pos, nbrUsers );
   for ( uint32 i = 0 ; i < nbrUsers ; i++ ) {
      incWriteLong( pos, UINs[ i ] );
   }

   // Write all strings
   if ( logonIDs != NULL ) {
      for ( uint32 i = 0 ; i < nbrUsers ; i++ ) {
         incWriteString( pos, (*logonIDs)[ i ] );
      }
   }
   setLength( pos );
}


uint32
FindUserReplyPacket::getNbrUsers() const {
   return readLong(  USER_REPLY_HEADER_SIZE );
}


uint32*
FindUserReplyPacket::getUINs() const {
   uint32 nbrUsers = getNbrUsers();

   uint32* UINs = new uint32[ nbrUsers ];
   int pos = USER_REPLY_HEADER_SIZE + 4;
   for ( uint32 i = 0 ; i < nbrUsers ; i++ ) {
      UINs[ i ] = incReadLong( pos );
   }
   return UINs;
}


const_char_p* 
FindUserReplyPacket::getLogonIDs() const {
   char** logonIDS = new char*[ getNbrUsers() ];

   int pos = USER_REPLY_HEADER_SIZE + 4 + 4*getNbrUsers();
   for ( uint32 i = 0 ; i < getNbrUsers() ; i++ ) {
      incReadString( pos, logonIDS[ i ] );
   }

   return const_cast<const_char_p*> ( logonIDS );
}


// ===== GetCellularPhoneModelDataRequestPacket ================
GetCellularPhoneModelDataRequestPacket::
GetCellularPhoneModelDataRequestPacket( uint16 packetID,
                                        uint16 requestID )
      : RequestPacket( REQUEST_HEADER_SIZE + 4,
                       USER_REQUEST_PRIO,
                       PACKETTYPE_GETCELLULARPHONEMODELDATAREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   setNbrModels( 0 );
   setLength( REQUEST_HEADER_SIZE + 4 );
}

GetCellularPhoneModelDataRequestPacket::
GetCellularPhoneModelDataRequestPacket( uint16 packetID,
                                        uint16 requestID,
                                        CellularPhoneModel* cellular)
      : RequestPacket( MAX_PACKET_SIZE,
                       USER_REQUEST_PRIO,
                       PACKETTYPE_GETCELLULARPHONEMODELDATAREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   int pos = REQUEST_HEADER_SIZE + 4;
   setNbrModels( 1 );
   cellular->addChanges( this, pos );
   setLength( pos );
}

void
GetCellularPhoneModelDataRequestPacket::setNbrModels(uint32 nbrModels) {
   writeLong( REQUEST_HEADER_SIZE, nbrModels );
}


uint32
GetCellularPhoneModelDataRequestPacket::getNbrModels() const {
   return readLong( REQUEST_HEADER_SIZE );
}

CellularPhoneModel*
GetCellularPhoneModelDataRequestPacket::getModel() const {
   int pos = REQUEST_HEADER_SIZE + 4;
   CellularPhoneModel* cellular = NULL;

   if ( (uint32)(pos + 3) >= getLength() ) {
      mc2dbg1 <<
             "Change GetCellularPhoneModelDataRequestPacket::getNextElement " 
             << "Not more data in packet NULL returned" << endl;
      return NULL;
   } 

   uint32 type = incReadLong( pos );
   incReadShort( pos );

   
   if ( type == ( MAX_UINT8 / 2 ) ) {
      cellular = new CellularPhoneModel();;
      cellular->readChanges( this, pos );
      type = incReadLong( pos );
      if ( !cellular->getValid() || (type != ( MAX_UINT8 / 2 ) ) )
      {
         mc2dbg1 << "GetCellularPhoneModelDataRequestPacket::getNextElement "
                << "UserUser not ok" << endl;
         delete cellular;
         cellular = NULL;
      } else {
         mc2dbg8
            << "ChangeCellularPhoneModelDataRequestPacket::getNextElement "
            << "UserUser OK" << endl;
      }
   }
   return cellular;
}
      
// ===== GetCellularPhoneModelDataReplyPacket ==================
GetCellularPhoneModelDataReplyPacket::GetCellularPhoneModelDataReplyPacket(
   const GetCellularPhoneModelDataRequestPacket* p, 
   CellularPhoneModels* list ) 
      : ReplyPacket( MAX_PACKET_SIZE, 
                     PACKETTYPE_GETCELLULARPHONEMODELDATAREPLY,
                     p,
                     StringTable::OK )
{
   int pos = REPLY_HEADER_SIZE + 4;
   if ( list != NULL ) {
      uint32 size = list->size();
      bool ok = true;
      uint32 i;
      for ( i = 0 ; (i < size) && ok ; i++ ) {
         ok = static_cast<CellularPhoneModel*> ( list->getElementAt( i ) )
            ->packInto( this, pos );
      }
      setNbrCellularPhoneModels( i );
   } else {
      setNbrCellularPhoneModels( 0 );
      setStatus(  StringTable::NOTOK );
   }
   setLength( pos );
}


CellularPhoneModels*
GetCellularPhoneModelDataReplyPacket::getAllPhoneModels() const {
   CellularPhoneModels* list;

   int pos = REPLY_HEADER_SIZE + 4;
   
   list = new CellularPhoneModels( this, pos, 
                                   getNbrCellularPhoneModels() );

   if ( !list->getValid() ) {
      delete list;
      list = NULL;
   }

   return list;
}


uint32
GetCellularPhoneModelDataReplyPacket::getNbrCellularPhoneModels() const {
   return readLong( REPLY_HEADER_SIZE );
}


void
GetCellularPhoneModelDataReplyPacket::setNbrCellularPhoneModels( 
   uint32 nbrModels ) 
{
   writeLong( REPLY_HEADER_SIZE, nbrModels );
}

// ====================== AddCellularPhoneModelRequestPacket
AddCellularPhoneModelRequestPacket::AddCellularPhoneModelRequestPacket(
   uint16 packetID,
   uint16 requestID,
   CellularPhoneModel* model)
      : RequestPacket( MAX_PACKET_SIZE,
                       USER_REQUEST_PRIO,
                       PACKETTYPE_ADDCELLULARPHONEMODELREQUESTPACKET,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   int pos = REQUEST_HEADER_SIZE;
   model->packInto( this, pos );
   setLength( pos );
}

CellularPhoneModel*
AddCellularPhoneModelRequestPacket::getCellularPhoneModel() const {
   int pos = REQUEST_HEADER_SIZE;
   uint32 type;
   uint16 length;
         
   type = incReadLong( pos );
   length = incReadShort( pos );
   if ( type == (MAX_UINT8 / 2) && 
        pos - 6 + length < (int32)getLength() )
      return new CellularPhoneModel( this, pos );
   else
      return NULL;
}


// =======================AddCellularPhoneModelReplyPacket
AddCellularPhoneModelReplyPacket::AddCellularPhoneModelReplyPacket(
   const AddCellularPhoneModelRequestPacket* packet,
   uint32 status)
      : ReplyPacket( MAX_PACKET_SIZE, 
                     PACKETTYPE_ADDCELLULARPHONEMODELREPLYPACKET,
                     packet,
                     status)

{
 
}

//=======================ChangeUserPasswordRequestPacket
ChangeUserPasswordRequestPacket::ChangeUserPasswordRequestPacket(
   uint16 packetID,
   uint16 requestID,
   const char* password,
   uint32 UIN,
   uint32 changerUIN )
      : UserRequestPacket ( USER_REQUEST_HEADER_SIZE + 4 + 
                            strlen( password ) + 1,
                            PACKETTYPE_CHANGEUSERPASSWORDREQUESTPACKET,
                            packetID,
                            requestID,
                            UIN )
{
   
   int pos = USER_REQUEST_HEADER_SIZE;
   incWriteLong(   pos, changerUIN );
   incWriteString( pos, password );
   setLength( pos );
}

   
const char* 
ChangeUserPasswordRequestPacket::getUserPassword() const {

   int pos = USER_REQUEST_HEADER_SIZE + 4;
   char* password;
   incReadString( pos, password );
   return password;
  
}


uint32
ChangeUserPasswordRequestPacket::getChangerUIN() const {
   return readLong( USER_REQUEST_HEADER_SIZE );
}


//==============================ChangeUserPasswordReplyPacket
ChangeUserPasswordReplyPacket::ChangeUserPasswordReplyPacket(
   const ChangeUserPasswordRequestPacket* packet,
   uint32 status)
      : UserReplyPacket( USER_REPLY_HEADER_SIZE, 
                         PACKETTYPE_CHANGEUSERPASSWORDREPLYPACKET,
                         packet)
{

}

// ===== ChangeCellularPhoneModelRequestPacket =======================
ChangeCellularPhoneModelRequestPacket::ChangeCellularPhoneModelRequestPacket(
   uint16 packetID, 
   uint16 requestID,
   CellularPhoneModel* cellular,
   const char* name)
      : RequestPacket( MAX_PACKET_SIZE,
                       USER_REQUEST_PRIO,
                       PACKETTYPE_CHANGECELLULARPHONEMODELREQUESTPACKET,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   int pos= REQUEST_HEADER_SIZE;
   incWriteString( pos, name );
   cellular->addChanges( this,pos );
   setLength( pos );
}


CellularPhoneModel*
ChangeCellularPhoneModelRequestPacket::getCellularPhoneModel() const {
   int pos = REQUEST_HEADER_SIZE;
   uint32 type;
   uint16 length;
   char* name;
   incReadString( pos, name );
   type = incReadLong( pos );
   length = incReadShort( pos );
   mc2dbg8 << "Entered getCellularPhoneModel" << endl;
   if ( type == (MAX_UINT8 / 2) ) {
      CellularPhoneModel* cellular = new CellularPhoneModel();
      cellular->readChanges( this, pos );
      return cellular;
   } else {
      mc2dbg8 << "No cellular returned in getCellularPhonemodel" << endl;
      return NULL;
   }
}
const char*
ChangeCellularPhoneModelRequestPacket::getName() const {
   int pos = REQUEST_HEADER_SIZE;
   char* name;
   incReadString( pos, name );
   return name;
}

// ===== ChangeCellularPhoneModelReplyPacket =========================
ChangeCellularPhoneModelReplyPacket::ChangeCellularPhoneModelReplyPacket(  
   const ChangeCellularPhoneModelRequestPacket* packet,
   uint32 status )
      : ReplyPacket( MAX_PACKET_SIZE, 
                     PACKETTYPE_CHANGECELLULARPHONEMODELREPLYPACKET,
                     packet,
                     status )

{
   
}


//==============ListDebitRequestPacket==================================

   
ListDebitRequestPacket::ListDebitRequestPacket(
   uint32 UIN,
   uint32 startTime,
   uint32 endTime,
   uint32 startIndex,
   uint32 endIndex )
      : UserRequestPacket( MAX_PACKET_SIZE,
                           PACKETTYPE_LISTDEBITREQUESTPACKET,
                           0, 0, UIN )
{
   int pos = startTime_POS;
   incWriteLong( pos, startTime );
   incWriteLong( pos, endTime );
   incWriteLong( pos, startIndex );
   incWriteLong( pos, endIndex );
   setLength( pos );  
}


uint32
ListDebitRequestPacket::getStartTime() const {
   return readLong( startTime_POS );
}


uint32
ListDebitRequestPacket::getEndTime() const {
   return readLong( endTime_POS );
}


uint32
ListDebitRequestPacket::getStartIndex() const {
   return readLong( startIndex_POS );
}


uint32
ListDebitRequestPacket::getEndIndex() const {
   return readLong( endIndex_POS );
}



//========ListDebitReplyPacket====================================



ListDebitReplyPacket::ListDebitReplyPacket(
   const ListDebitRequestPacket* p,
   StringTable::stringCode status )
      : UserReplyPacket( MAX_PACKET_SIZE,
                         PACKETTYPE_LISTDEBITREPLYPACKET,
                         p )
{
   setStatus( status );
   writeLong( nbrElements_POS, 0 );
   writeLong( startIndex_POS, 0 );
   writeLong( endIndex_POS, 0 );
   writeLong( totalNbr_POS, 0 );
   setLength( endStatic_POS );
}


void
ListDebitReplyPacket::addDebitElement( const DebitElement* el ) {
   int pos = getLength();
   el->packInto( this, pos );
   incNbrElements();
}


uint32
ListDebitReplyPacket::getNbrDebitElements() const {
   return readLong( nbrElements_POS );
}


DebitElement*
ListDebitReplyPacket::getFirstElement( int& pos ) const {
   pos = endStatic_POS;
   return getNextElement( pos );
}


DebitElement*
ListDebitReplyPacket::getNextElement( int& pos ) const {
   DebitElement* el = NULL;
   if ( pos < int32(getLength()) ) {
      el = new DebitElement();
      el->readFromPacket( this, pos );
   }
   return el;
}


void 
ListDebitReplyPacket::incNbrElements() {
   uint32 nbr = readLong( nbrElements_POS );
   writeLong( nbrElements_POS, nbr + 1 );
}


uint32
ListDebitReplyPacket::getStartIndex() const {
   return readLong( startIndex_POS );
}


uint32
ListDebitReplyPacket::getEndIndex() const {
   return readLong( endIndex_POS );
}


uint32 
ListDebitReplyPacket::getTotalNbrDebits() const {
   return readLong( totalNbr_POS );
}


void 
ListDebitReplyPacket::setStartIndex( uint32 startIndex ) {
   writeLong( startIndex_POS, startIndex );
}


void 
ListDebitReplyPacket::setEndIndex( uint32 endIndex ) {
   writeLong( endIndex_POS, endIndex );
}


void 
ListDebitReplyPacket::setTotalNbrDebits( int32 nbr ) {
   writeLong( totalNbr_POS, nbr );
}


//**********************************************************************
// UserNavDestination Packets
//**********************************************************************


AddUserNavDestinationRequestPacket::AddUserNavDestinationRequestPacket( 
   DBUserNavDestination* nav )
      : RequestPacket( 
         MAX_PACKET_SIZE,
         USER_REQUEST_PRIO,
         Packet::PACKETTYPE_ADDUSERNAVDESTINATIONREQUESTPACKET,
         0, // packetId
         0, // requestID
         MAX_UINT32 ) // mapID
{
   int pos = REQUEST_HEADER_SIZE;
   nav->packInto( this, pos );
   setLength( pos );
}


DBUserNavDestination*
AddUserNavDestinationRequestPacket::getUserNavDestination() const {
   int pos = REQUEST_HEADER_SIZE;
   DBUserNavDestination* nav = new DBUserNavDestination( this, pos );
   return nav;
}


AddUserNavDestinationReplyPacket::AddUserNavDestinationReplyPacket( 
   const RequestPacket* reqPack,
   StringTable::stringCode status )
      : ReplyPacket( REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_ADDUSERNAVDESTINATIONREPLYPACKET,
                     reqPack,
                     status )
{
}


DeleteUserNavDestinationRequestPacket::
DeleteUserNavDestinationRequestPacket( uint32 ID ) 
      : RequestPacket( 
         REQUEST_HEADER_SIZE + 4,
         USER_REQUEST_PRIO,
         Packet::PACKETTYPE_DELETEUSERNAVDESTINATIONREQUESTPACKET,
         0, // packetId
         0, // requestID
         MAX_UINT32 ) // mapID
{
  int pos = REQUEST_HEADER_SIZE;
  incWriteLong( pos, ID );
  setLength( pos );
}


uint32
DeleteUserNavDestinationRequestPacket::getDeleteID() const {
   int pos = REQUEST_HEADER_SIZE;
   return incReadLong( pos);
}


DeleteUserNavDestinationReplyPacket::DeleteUserNavDestinationReplyPacket( 
   const RequestPacket* reqPack,
   StringTable::stringCode status )
      : ReplyPacket( REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_ADDUSERNAVDESTINATIONREPLYPACKET,
                     reqPack,
                     status )
{
}


GetUserNavDestinationRequestPacket::GetUserNavDestinationRequestPacket( 
   bool onlyUnSentNavDestinations,
   bool onlyNavigatorLastContact,
   uint32 navID,
   const char* navAddress )
      : RequestPacket( 
         // The 2 is padding
         REQUEST_HEADER_SIZE + 1 + 1 + 4 + 2 + strlen( navAddress ) + 1, 
         USER_REQUEST_PRIO,
         Packet::PACKETTYPE_GETUSERNAVDESTINATIONREQUESTPACKET,
         0, // packetId
         0, // requestID
         MAX_UINT32 ) // mapID
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteByte( pos, static_cast<byte>( onlyUnSentNavDestinations ) );
   incWriteByte( pos, static_cast<byte>( onlyNavigatorLastContact ) );
   incWriteLong( pos, navID );
   mc2dbg4 << "Start strign at " << pos << endl;
   incWriteString( pos, navAddress );
   setLength( pos );
}


bool
GetUserNavDestinationRequestPacket::getOnlyUnSentNavDestinations() const {
   int pos = REQUEST_HEADER_SIZE;

   return (incReadByte( pos ) != 0 );
}


bool
GetUserNavDestinationRequestPacket:: getOnlyNavigatorLastContact() const {
   int pos = REQUEST_HEADER_SIZE + 1;
   
   return (incReadByte( pos ) != 0 );  
}


uint32
GetUserNavDestinationRequestPacket::getNavID() const {
   int pos = REQUEST_HEADER_SIZE + 2;

   return incReadLong( pos );
}


const char* 
GetUserNavDestinationRequestPacket::getNavAddress() const{
   int pos = REQUEST_HEADER_SIZE + 6 + 2; // 2 is padding
   char* str = NULL;
   incReadString( pos, str );  
   return str;
}


GetUserNavDestinationReplyPacket::GetUserNavDestinationReplyPacket( 
   const RequestPacket* reqPack,
   StringTable::stringCode status )
      : ReplyPacket( MAX_PACKET_SIZE,
                     Packet::PACKETTYPE_GETUSERNAVDESTINATIONREPLYPACKET,
                     reqPack,
                     status )
{
   int pos = REPLY_HEADER_SIZE + 4;

   setNbrUserNavDestination( 0 );
   setLength( pos );
}


bool
GetUserNavDestinationReplyPacket::addUserNavDestination( 
   DBUserNavDestination* nav )
{
   if ( (getLength() + MAX_DBUSERNAVIGATION_SIZE) > getBufSize() ) {
      return false;
   }
   
   int pos = getLength();
   nav->packInto( this, pos );
   setLength( pos );
   setNbrUserNavDestination( getNbrUserNavDestination() + 1 );
   
   return true;
}


uint32
GetUserNavDestinationReplyPacket::getNbrUserNavDestination() const {
   int pos = REPLY_HEADER_SIZE;

   return incReadLong( pos );
}


int
GetUserNavDestinationReplyPacket::getFirstUserNavDestinationPos() const {
   return (REPLY_HEADER_SIZE + 4);
}


DBUserNavDestination* 
GetUserNavDestinationReplyPacket::getUserNavDestination( int& pos ) const {
   if ( (uint32)pos >= getLength() ) {
      return NULL;
   }

   DBUserNavDestination* nav = new DBUserNavDestination( this, pos );
   return nav;
}


void
GetUserNavDestinationReplyPacket::setNbrUserNavDestination( uint32 nbr ) {
   writeLong( REPLY_HEADER_SIZE, nbr );
}


const uint32 GetUserNavDestinationReplyPacket::MAX_DBUSERNAVIGATION_SIZE 
= 1024;


ChangeUserNavDestinationRequestPacket::
ChangeUserNavDestinationRequestPacket( DBUserNavDestination* nav,
                                       uint32 navID )
      : RequestPacket( 
         MAX_PACKET_SIZE,
         USER_REQUEST_PRIO,
         Packet::PACKETTYPE_CHANGEUSERNAVDESTINATIONREQUESTPACKET,
         0, // packetId
         0, // requestID
         MAX_UINT32 ) // mapID   
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong( pos, navID );
   nav->addChanges( this, pos );
   setLength( pos );
}


DBUserNavDestination* 
ChangeUserNavDestinationRequestPacket::getUserNavDestination() const {
   int pos = REQUEST_HEADER_SIZE + 4;
   DBUserNavDestination* nav = new DBUserNavDestination();
   
   nav->readChanges( this, pos );

   return nav;
}


uint32
ChangeUserNavDestinationRequestPacket::getNavID() const {
   return readLong( REQUEST_HEADER_SIZE );
}


ChangeUserNavDestinationReplyPacket::ChangeUserNavDestinationReplyPacket( 
   const RequestPacket* reqPack,
   StringTable::stringCode status )
      : ReplyPacket( 
         REPLY_HEADER_SIZE,
         Packet::PACKETTYPE_CHANGEUSERNAVDESTINATIONREPLYPACKET,
         reqPack,
         status )
{
}

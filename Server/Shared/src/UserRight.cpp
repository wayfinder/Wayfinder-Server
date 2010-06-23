/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserRight.h"
#include "Packet.h"


// **********************************************************************
// UserRight
// **********************************************************************

UserRight::UserRight( uint32 id, uint32 addTime, UserEnums::URType type,
                      uint32 regionID, uint32 startTime, uint32 endTime,
                      bool deleted, const char* origin )
      : UserElement( UserConstants::TYPE_RIGHT ),
        m_addTime( addTime ), m_userRightType( type ), 
        m_regionID( regionID ), m_startTime( startTime ),
        m_endTime( endTime ), m_deleted( deleted ), m_origin( origin )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
      m_changed[ i ] = false;
   }
}


UserRight::UserRight( uint32 id ) 
      : UserElement( UserConstants::TYPE_RIGHT ),
        m_addTime( 0 ), m_userRightType(), 
        m_regionID( MAX_INT32 ), m_startTime( 0 ), 
        m_endTime( MAX_INT32 ), m_deleted( false ), m_origin( "" )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
      m_changed[ i ] = false;
   }
}


UserRight::UserRight( const Packet* p, int& pos ) 
      : UserElement( UserConstants::TYPE_RIGHT )
{
   readFromPacket( p, pos );
}


UserRight::UserRight( const UserRight& user ) 
      : UserElement( UserConstants::TYPE_RIGHT )
{
   m_id = user.m_id;
   m_addTime = user.m_addTime;
   m_userRightType = user.m_userRightType;
   m_regionID = user.m_regionID;
   m_startTime = user.m_startTime;
   m_endTime = user.m_endTime;
   m_deleted = user.m_deleted;
   m_origin = user.m_origin;
   for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserRight::~UserRight() {
   // Nothing newed in constructor (yet)
}


uint32
UserRight::getSize() const {
   return 16 + 7*4 + 1 + m_origin.size()+1 + 2*4 + 3;
}


void
UserRight::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_RIGHT );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_RIGHT );
   p->incWriteShort( pos, size );

   p->incWriteLong(   pos, m_id );
   p->incWriteLong(   pos, m_addTime );
   p->incWriteLong(   pos, m_userRightType.level() );
   p->incWriteLong(   pos, m_userRightType.service() );
   p->incWriteLong(   pos, m_regionID );
   p->incWriteLong(   pos, m_startTime );
   p->incWriteLong(   pos, m_endTime );
   p->incWriteByte(   pos, m_deleted );
   p->incWriteString( pos, m_origin.c_str() );

   p->incWriteLong( pos, UserConstants::TYPE_RIGHT );
   p->incWriteLong( pos, UserConstants::TYPE_RIGHT );

   // Set length 
   p->setLength( pos );
}


void
UserRight::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ )
      if ( m_changed[ i ] )
         nbrChanges++;
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, UserConstants::TYPE_RIGHT );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, UserConstants::TYPE_RIGHT );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_NBRFIELDS -2 );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_ADD_TIME );
         p->incWriteLong( position, m_addTime );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_TYPE );
         p->incWriteLong( position, m_userRightType.level() );
         p->incWriteLong( position, m_userRightType.service() );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_REGION_ID );
         p->incWriteLong( position, m_regionID );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_START_TIME );
         p->incWriteLong( position, m_startTime );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_END_TIME );
         p->incWriteLong( position, m_endTime );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_DELETED );
         p->incWriteByte( position, m_deleted );
         p->incWriteByte( position, 
                          UserConstants::USER_RIGHT_ORIGIN );
         p->incWriteString( position, m_origin.c_str() );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ UserConstants::USER_RIGHT_ADD_TIME ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_ADD_TIME );
            p->incWriteLong( position, m_addTime );
         }
         if ( m_changed[ UserConstants::USER_RIGHT_TYPE ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_TYPE );
            p->incWriteLong( position, m_userRightType.level() );
            p->incWriteLong( position, m_userRightType.service() );
         }
         if ( m_changed[ UserConstants::USER_RIGHT_REGION_ID ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_REGION_ID );
            p->incWriteLong( position, m_regionID );
         }
         if ( m_changed[ UserConstants::USER_RIGHT_START_TIME ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_START_TIME);
            p->incWriteLong( position, m_startTime );
         }
         if ( m_changed[ UserConstants::USER_RIGHT_END_TIME ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_END_TIME );
            p->incWriteLong( position, m_endTime );
         }
         if ( m_changed[ UserConstants::USER_RIGHT_DELETED ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_DELETED );
            p->incWriteByte( position, m_deleted );
         }
         if ( m_changed[ UserConstants::USER_RIGHT_ORIGIN ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_RIGHT_ORIGIN );
            p->incWriteString( position, m_origin.c_str() );
         }
      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_RIGHT );
      p->incWriteLong( position, UserConstants::TYPE_RIGHT );
      for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserRight::addChanges but there is no "
             << "changes to save?" << endl;
   }
}



bool
UserRight::readChanges( const Packet* p, int& pos, 
                               UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_RIGHT && id == m_id ) {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_RIGHT ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserRight::readChanges not Type "
                         << " Right after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_RIGHT ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserRight::readChanges not Type "
                         << " Right after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserRight::readChanges unknown action: "
                      << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserRight::readChanges not correct type or "
                << " not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserRight::readChanges no space for changes "
             << "giving up." << endl;
      return false;
   }
}


bool
UserRight::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      uint32 addTime = m_addTime;
      UserEnums::URType userRightType = m_userRightType;
      uint32 regionID = m_regionID;
      uint32 startTime = m_startTime;
      uint32 endTime = m_endTime;
      bool deleted = m_deleted;
      MC2String origin = m_origin;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_RIGHT_ADD_TIME :
               m_changed[ UserConstants::USER_RIGHT_ADD_TIME ] = true;
               addTime = p->incReadLong( pos );
               break;
            case UserConstants::USER_RIGHT_TYPE :
               m_changed[ UserConstants::USER_RIGHT_TYPE ] = true;
               userRightType.setLevel( UserEnums::userRightLevel( 
                                          p->incReadLong( pos ) ) );
               userRightType.setService( UserEnums::userRightService(
                                            p->incReadLong( pos ) ) );
               break;
            case UserConstants::USER_RIGHT_REGION_ID :
               m_changed[ UserConstants::USER_RIGHT_REGION_ID ] = true;
               regionID = p->incReadLong( pos );
               break;
            case UserConstants::USER_RIGHT_START_TIME :
               m_changed[ UserConstants::USER_RIGHT_START_TIME ] = true;
               startTime = p->incReadLong( pos );
               break;
            case UserConstants::USER_RIGHT_END_TIME :
               m_changed[ UserConstants::USER_RIGHT_END_TIME ] = true;
               endTime = p->incReadLong( pos );
               break;
            case UserConstants::USER_RIGHT_DELETED :
               m_changed[ UserConstants::USER_RIGHT_DELETED ] = true;
               deleted = p->incReadByte( pos );
               break;
            case UserConstants::USER_RIGHT_ORIGIN :
               m_changed[ UserConstants::USER_RIGHT_ORIGIN ] = true;
               origin = p->incReadString( pos );
               break;

            default:
               mc2log << error
                      << "UserRight::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 type = p->incReadLong( pos );
      if ( type != UserConstants::TYPE_RIGHT ) {
         mc2log << error
                << "UserRight::readDataChanges Not RIGHT"
                << " type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_RIGHT_ADD_TIME ] ) {
            m_addTime = addTime;
         }
         if ( m_changed[ UserConstants::USER_RIGHT_TYPE ] ) {
            m_userRightType.setLevel( userRightType.level() ) ;
            m_userRightType.setService( userRightType.service() ) ;
         }
         if ( m_changed[ UserConstants::USER_RIGHT_REGION_ID ] ) {
           m_regionID = regionID;
         }
         if ( m_changed[ UserConstants::USER_RIGHT_START_TIME ] ) {
           m_startTime = startTime;
         }
         if ( m_changed[ UserConstants::USER_RIGHT_END_TIME ] ) {
           m_endTime = endTime;
         }
         if ( m_changed[ UserConstants::USER_RIGHT_DELETED ] ) {
            m_deleted = deleted;
         }
         if ( m_changed[ UserConstants::USER_RIGHT_ORIGIN ] ) {
            m_origin = origin;
         }
         return ok;
      } else {
         mc2log << error
                << "UserRight::readDataChanges failed" << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserRight::readDataChanges no space for changes" 
             << endl;
      return false;
   }
}


void
UserRight::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_RIGHT ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id = p->incReadLong( pos );
         m_addTime = p->incReadLong( pos );
         m_userRightType.setLevel( UserEnums::userRightLevel( 
                                      p->incReadLong( pos ) ) );
         m_userRightType.setService( UserEnums::userRightService( 
                                        p->incReadLong( pos ) ) );
         m_regionID = p->incReadLong( pos );
         m_startTime = p->incReadLong( pos );
         m_endTime = p->incReadLong( pos );
         m_deleted = p->incReadByte( pos );
         m_origin = p->incReadString( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_RIGHT ) {
            mc2log << warn
                   << "UserRight::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_RIGHT!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserRight::readFromPacket(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserRight::readFromPacket( Packet* p, int& pos )"
             << "Not TYPE_RIGHT_KEY type!!!!!" << endl;
      return;
   }

   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
      m_changed[ i ] = false;
   }
}


bool
UserRight::checkAccessAt( uint32 t ) const {
   if ( getStartTime() <= t && (getEndTime() >= t || getEndTime() == 0) ) {
      return true;
   } else {
      return false;
   }
}

uint32
UserRight::getRightTimeLength() const {
   if ( getEndTime() == 0 ) {
      return MAX_UINT32;
   } else {
      if ( getEndTime() >= getStartTime() ) {
         return getEndTime() - getStartTime();
      } else {
         return 0;
      }
   }
}

uint32
UserRight::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserRight::printValue( char* target, 
                       UserConstants::UserRightField field ) const
{
   switch ( field ) {
      case UserConstants::USER_RIGHT_ID :
      case UserConstants::USER_RIGHT_USERUIN :
         mc2log << error << "UserRight::printValue asked to print "
                << "ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_RIGHT_ADD_TIME :
         sprintf( target, "%u", getAddTime() );
         break;
      case UserConstants::USER_RIGHT_TYPE :
         // Needs to be signed for MySQL to keep all bits.
         sprintf( target, "%lld",
                  static_cast<int64> ( getUserRightType().getAsInt() ) );
         break;
      case UserConstants::USER_RIGHT_REGION_ID :
         sprintf( target, "%u", getRegionID() );
         break;
      case UserConstants::USER_RIGHT_START_TIME :
         sprintf( target, "%u", getStartTime() );
         break;
      case UserConstants::USER_RIGHT_END_TIME :
         sprintf( target, "%u", getEndTime() );
         break;
      case UserConstants::USER_RIGHT_DELETED :
         sprintf( target, "%hu", isDeleted() );
         break;
      case UserConstants::USER_RIGHT_ORIGIN :
         strcat( target, "'" );
         sqlString( target + 1, getOrigin() );
         strcat( target, "'" );
         break;

      default:
         mc2log << error << "UserRight::printValue unknown " 
                << "fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserRight::changed( UserConstants::UserRightField field ) const {
   return m_changed[ field ];
}


bool
UserRight::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}

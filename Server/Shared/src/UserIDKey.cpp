/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserIDKey.h"
#include "UserPacket.h"

// **********************************************************************
// UserIDKey
// **********************************************************************


UserIDKey::UserIDKey( uint32 id, const MC2String& idKey, idKey_t type )
      : UserElement( UserConstants::TYPE_ID_KEY ),
        m_idType( type ), m_idKey( idKey )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


UserIDKey::UserIDKey( uint32 id ) 
      : UserElement( UserConstants::TYPE_ID_KEY ),
        m_idType( account_id_key ), m_idKey( "" )
{
   m_id = id;
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


UserIDKey::UserIDKey( const Packet* p, int& pos ) 
      : UserElement( UserConstants::TYPE_ID_KEY ) 
{
   readFromPacket( p, pos );
}


UserIDKey::UserIDKey( const UserIDKey& user ) 
      : UserElement( UserConstants::TYPE_ID_KEY )
{
   m_id = user.m_id;
   m_idKey = user.m_idKey;
   m_idType = user.m_idType;
   for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i ) {
      m_changed[ i ] = user.m_changed[ i ];
   }
}


UserIDKey::~UserIDKey() {
   // Nothing newed in constructor (yet)
}


uint32
UserIDKey::getSize() const {
   return 16 + 1*4 + m_idKey.size() + 1 + 2*4 + 3;
}


void
UserIDKey::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_ID_KEY );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_ID_KEY );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteLong( pos, m_idType );
   p->incWriteString( pos, m_idKey.c_str() );

   p->incWriteLong( pos, UserConstants::TYPE_ID_KEY );
   p->incWriteLong( pos, UserConstants::TYPE_ID_KEY );

   // Set length 
   p->setLength( pos );
}


void
UserIDKey::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i ) {
      if ( m_changed[ i ] ) {
         nbrChanges++;
      }
   }
   
   if ( nbrChanges > 0 || removed() || m_id == 0 ) {
      p->incWriteLong( position, UserConstants::TYPE_ID_KEY );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, UserConstants::TYPE_ID_KEY );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and not userUIN
         // Nbr changed
         p->incWriteByte( 
            position, UserConstants::USER_ID_KEY_NBRFIELDS -2 );
         p->incWriteByte( position, UserConstants::USER_ID_KEY_TYPE );
         p->incWriteLong( position, m_idType );
         p->incWriteByte( position, UserConstants::USER_ID_KEY_KEY );
         p->incWriteString( position, m_idKey.c_str() );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         
         if ( m_changed[ UserConstants::USER_ID_KEY_TYPE ] ) {
            p->incWriteByte( position, UserConstants::USER_ID_KEY_TYPE );
            p->incWriteLong( position, m_idType );
         }
         if ( m_changed[ UserConstants::USER_ID_KEY_KEY ] ) {
            p->incWriteByte( position, UserConstants::USER_ID_KEY_KEY );
            p->incWriteString( position, m_idKey.c_str() );
         }
         
      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_ID_KEY );
      p->incWriteLong( position, UserConstants::TYPE_ID_KEY );
      for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i )
      {
         m_changed[ i ] = false;
      }
      p->setLength( position );
   } else {
      mc2log << warn << "UserIDKey::addChanges but there "
             << "is no changes to save?" << endl;
   }
}


bool
UserIDKey::readChanges( const Packet* p, int& pos, 
                        UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_ID_KEY && id == m_id ) {
         action = UserConstants::UserAction( p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_ID_KEY ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserIDKey::readChanges not "
                         << "Type ID_KEY after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_ID_KEY ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserIDKey::readChanges not "
                         << "Type ID_KEY after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserIDKey::readChanges unknown "
                      << "action: " << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserIDKey::readChanges not correct "
                << "type or not correct id, giving up." << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserIDKey::readChanges no space for "
             << "changes giving up." << endl;
      return false;
   }
}


bool
UserIDKey::readDataChanges( const Packet* p, int& pos ) {
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      idKey_t idType = m_idType;
      MC2String idKey = m_idKey;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ; ++i ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_ID_KEY_TYPE :
               m_changed[ UserConstants::USER_ID_KEY_TYPE ] = true;
               idType = idKey_t( p->incReadLong( pos ) );
               break;
            case UserConstants::USER_ID_KEY_KEY :
               m_changed[ UserConstants::USER_ID_KEY_KEY ] = true;
               idKey = p->incReadString( pos );
               break;

            default:
               mc2log << error
                      << "UserIDKey::readDataChanges "
                      << "unknown fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 packetType = p->incReadLong( pos );
      if ( packetType != UserConstants::TYPE_ID_KEY ) {
         mc2log << error
                << "UserIDKey::readDataChanges Not "
                << "ID_KEY type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_ID_KEY_TYPE ] ) {
            m_idType = idType;
         }
         if ( m_changed[ UserConstants::USER_ID_KEY_KEY ] ) {
            m_idKey = idKey;
         }

         return ok;
      } else {
         mc2log << error << "UserIDKey::readDataChanges failed" << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserIDKey::readDataChanges no space for changes" << endl;
      return false;
   }
}


void
UserIDKey::readFromPacket( const Packet* p, int& pos ) {
   uint32 type = p->incReadLong( pos );

   if ( type == UserConstants::TYPE_ID_KEY ) {
      uint32 size = p->incReadShort( pos );
      
      if ( (getSize() - 6) <= size ) {
         m_id     = p->incReadLong( pos );
         m_idType = idKey_t( p->incReadLong( pos ) );
         m_idKey  = p->incReadString( pos );

         type = p->incReadLong( pos );
         if ( type != UserConstants::TYPE_ID_KEY ) {
            mc2log << warn
                   << "UserIDKey::readFromPacket( "
                   << "Packet* p, int& pos )"
                   << "Data not ended by TYPE_ID_KEY!!!"
                   << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserIDKey::readFromPacket"
                << "(Packet* p, int& pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      }
   } else {
      mc2log << warn 
             << "UserIDKey::readFromPacket"
             << "( Packet* p, int& pos )"
             << "Not TYPE_ID_KEY type!!!!!" << endl;
      return;
   }
   
   setOk( true );

   for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i ) {
      m_changed[ i ] = false;
   }
}


void
UserIDKey::setIDKey( const MC2String& idKey ) {
   m_idKey = idKey;
   m_changed[ UserConstants::USER_ID_KEY_KEY ] = true;
}


void
UserIDKey::setIDType( idKey_t type ) {
   m_idType = type;
   m_changed[ UserConstants::USER_ID_KEY_TYPE ] = true;
}


uint32
UserIDKey::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; ++i ) {
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


uint32
UserIDKey::printValue( char* target, UserConstants::UserIDKeyField field )
   const
{
   switch ( field ) {
      case UserConstants::USER_ID_KEY_ID :
      case UserConstants::USER_ID_KEY_USERUIN :
         mc2log << error << "UserIDKey::printValue asked "
                << "to print ID or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_ID_KEY_TYPE :
         sprintf( target, "%d", getIDType() );
         break;
      case UserConstants::USER_ID_KEY_KEY :
         strcat( target, "'" );
         sqlString( target + 1, getIDKey().c_str() );
         strcat( target, "'" );
         break;

      default:
         mc2log << error << "UserIDKey::printValue unknown"
                << " fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}


bool
UserIDKey::changed( UserConstants::UserIDKeyField field ) const {
   return m_changed[ field ];
}


bool
UserIDKey::isChanged() const {
   return getNbrChanged() > 0 || m_id == 0 || removed();
}

ostream& operator<<( ostream& stream, const UserIDKey& idkey ) {
   stream << int(idkey.m_idType) << ":" << idkey.m_idKey;
   return stream;
}

bool
UserIDKey::compare( const UserIDKey& other ) const {
   return m_idType == other.m_idType && m_idKey == other.m_idKey;
}

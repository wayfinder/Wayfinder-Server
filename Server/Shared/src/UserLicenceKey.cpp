/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserLicenceKey.h"
#include "UserPacket.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include "LuhnCheckDigit.h"


// ---- UserLicenceKey ---------------------------------------------------


UserLicenceKey::UserLicenceKey( uint32 id ) 
      : UserElement( UserConstants::TYPE_LICENCE_KEY ),
        m_licenceKey( NULL ), m_licenceLength( 0 )
{
   setOk( true );
   m_id = id;

   for ( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; i++ )
   {
      m_changed[ i ] = false;
   }
}


UserLicenceKey::UserLicenceKey( uint32 id, const byte* licenceKey, 
                                uint32 licenceLength, 
                                const MC2String& product,
                                const MC2String& keyType )
      : UserElement( UserConstants::TYPE_LICENCE_KEY ),
        m_licenceKey( NULL ), 
        m_licenceLength( licenceLength ),
        m_product( product ),
        m_keyType( keyType )
{
   m_licenceKey = new byte[ m_licenceLength ];
   memcpy( m_licenceKey, licenceKey, m_licenceLength );
   m_id = id;

   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; i++ )
   {
      m_changed[ i ] = false;
   }
}

UserLicenceKey::UserLicenceKey( uint32 id, const MC2String& licenceKey,
                                const MC2String& product, 
                                const MC2String& keyType ) :
   UserElement( UserConstants::TYPE_LICENCE_KEY ),
   m_licenceKey( NULL ),
   m_licenceLength( 0 ),
   m_product( product ),
   m_keyType( keyType )
{
   m_id = id;
   setLicence( licenceKey ); //will set some indexes of m_changed to true
   setOk( true );
   for( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; ++i){
      m_changed[ i ] = false;
   }
}


UserLicenceKey::UserLicenceKey( const Packet* p, int& pos )
      : UserElement( UserConstants::TYPE_LICENCE_KEY )
{
   uint32 type = p->incReadLong( pos );
   m_licenceKey = NULL;
   m_licenceLength = 0;

   if ( type == UserConstants::TYPE_LICENCE_KEY ) {
      uint32 size = p->incReadShort( pos );
      
      if ( 6 + 8 < size ) {
         m_id = p->incReadLong( pos );
         m_licenceLength = p->incReadShort( pos );
         uint32 productLength = p->incReadShort( pos );
         uint32 keyTypeLength = p->incReadShort( pos );

         if ( 8 + m_licenceLength < size &&
              pos + m_licenceLength + productLength + keyTypeLength <= 
              p->getLength() ) 
         {
            m_licenceKey = new byte[ m_licenceLength ];
            memcpy( m_licenceKey, p->getBuf() + pos, m_licenceLength );
            // Update pos
            pos += m_licenceLength;

            m_product = p->incReadString( pos );
            m_keyType = p->incReadString( pos );

            type = p->incReadLong( pos );
            if ( type != UserConstants::TYPE_LICENCE_KEY ) {
               mc2log << warn
                      << "UserLicenceKey::UserLicenceKey( p, pos ) "
                      << "Data not ended by TYPE_LICENCE_KEY!!!" << endl;
               return;
            }
            // DONE
         } else {
            m_licenceLength = 0;
            mc2log << warn << 
               "UserLicenceKey::UserLicenceKey(UserReplyPacket* p, pos)"
                   << " Not space for licenceKey!!" << endl;
            return;
         }
         // DONE
      } else {
         mc2log << warn  
                << "UserLicenceKey::UserLicenceKey(UserReplyPacket* p,pos)"
                << " Not size for data!! Nothing read" << endl;
         return;
      
      }
   } else {
      mc2log << warn 
             << "UserLicenceKey::UserLicenceKey( UserReplyPacket* p, pos )"
             << "Not TYPE_LICENCE_KEY type!!!!!" << endl;
      return;
   }

   setOk( true );

   for ( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; i++ )
   {
      m_changed[ i ] = false;
   }
}


UserLicenceKey::UserLicenceKey( const UserLicenceKey& user )
      : UserElement( UserConstants::TYPE_LICENCE_KEY )
{
   m_licenceKey = NULL;
   *this = user;
}


UserLicenceKey&
UserLicenceKey::operator = ( const UserLicenceKey& o ) {
   // this = this != good!
   if ( this != &o ) {
      delete [] m_licenceKey;
      m_id = o.m_id;
      m_licenceLength = o.m_licenceLength;
      m_licenceKey = new byte[ m_licenceLength ];
      memcpy( m_licenceKey, o.m_licenceKey, m_licenceLength );
      m_product = o.m_product;
      m_keyType = o.m_keyType;

      for ( uint8 i = 0 ; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; i++ )
      {
         m_changed[ i ] = o.m_changed[ i ];
      }
   }

   return *this;
}


UserLicenceKey::~UserLicenceKey() {
   delete [] m_licenceKey;
}


uint32 
UserLicenceKey::getSize() const {
   return m_licenceLength + m_product.size() + 1 + m_keyType.size() + 1
      + 16 + 4 + 3*2 + 4*2 + 3;
}


void 
UserLicenceKey::packInto( Packet* p, int& pos ) {
   uint32 size = getSize();

   p->incWriteLong( pos, UserConstants::TYPE_LICENCE_KEY );
   p->incWriteShort( pos, size );
   p->incWriteLong( pos, UserConstants::TYPE_LICENCE_KEY );
   p->incWriteShort( pos, size );

   p->incWriteLong( pos, m_id );
   p->incWriteShort( pos, m_licenceLength );
   p->incWriteShort( pos, m_product.size() + 1 );
   p->incWriteShort( pos, m_keyType.size() + 1 );
   p->incWriteByteArray( pos, m_licenceKey, m_licenceLength );
   p->incWriteString( pos, m_product );
   p->incWriteString( pos, m_keyType );

   p->incWriteLong( pos, UserConstants::TYPE_LICENCE_KEY );
   p->incWriteLong( pos, UserConstants::TYPE_LICENCE_KEY );

   // Set length 
   p->setLength( pos );
}


void 
UserLicenceKey::addChanges( Packet* p, int& position ) {
   uint8 nbrChanges = 0;
   for ( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; i++ )
      if ( m_changed[ i ] )
         nbrChanges++;

   if ( nbrChanges > 0 || removed() ) {
      p->incWriteLong( position, UserConstants::TYPE_LICENCE_KEY );
      p->incWriteLong( position, m_id );
      p->incWriteLong( position, UserConstants::TYPE_LICENCE_KEY );
      p->incWriteLong( position, m_id );
      if ( removed() ) {
         p->incWriteByte( position, UserConstants::ACTION_DELETE );
      } else if ( m_id == 0 ) {
         p->incWriteByte(position, UserConstants::ACTION_NEW);
         // Not id and userUIN
         // Nbr changed
         p->incWriteByte( position, 
                          UserConstants::USER_LICENCE_KEY_NBRFIELDS - 2 );
         p->incWriteByte( position, UserConstants::USER_LICENCE_KEY );
         p->incWriteShort( position, m_licenceLength );
         p->incWriteByteArray( position, m_licenceKey, m_licenceLength );
         p->incWriteByte( position, UserConstants::USER_LICENCE_PRODUCT );
         p->incWriteString( position, m_product );
         p->incWriteByte( position, UserConstants::USER_LICENCE_KEY_TYPE );
         p->incWriteString( position, m_keyType );
      } else {
         p->incWriteByte( position, UserConstants::ACTION_CHANGE );
         p->incWriteByte( position, nbrChanges );
         if ( m_changed[ UserConstants::USER_LICENCE_KEY ] ) {
            p->incWriteByte( position, 
                             UserConstants::USER_LICENCE_KEY );
            p->incWriteShort( position, m_licenceLength );
            p->incWriteByteArray( position, 
                                  m_licenceKey, m_licenceLength );
         }
         if ( m_changed[ UserConstants::USER_LICENCE_PRODUCT ] ) {
             p->incWriteByte( position, UserConstants::USER_LICENCE_PRODUCT );
             p->incWriteString( position, m_product );
         }
         if ( m_changed[ UserConstants::USER_LICENCE_KEY_TYPE ] ) {
             p->incWriteByte( position, UserConstants::USER_LICENCE_KEY_TYPE );
             p->incWriteString( position, m_keyType );
         }

      } // End else changed

      p->incWriteLong( position, UserConstants::TYPE_LICENCE_KEY );
      p->incWriteLong( position, UserConstants::TYPE_LICENCE_KEY );
      for ( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS;i++)
         m_changed[i] = false;
      p->setLength( position );
   }
}


bool 
UserLicenceKey::readChanges( const Packet* p, int& pos, 
                             UserConstants::UserAction& action )
{
   if ( p->getLength() - pos - 5 > 0 ) {
      uint32 type;
      uint32 id;
      
      type = p->incReadLong( pos );
      id = p->incReadLong( pos );
      if ( type == UserConstants::TYPE_LICENCE_KEY && id == m_id ) {
         action = UserConstants::UserAction(p->incReadByte( pos ) );
         switch ( action ) {
            case UserConstants::ACTION_NEW:
               return readDataChanges( p, pos );
               break;
            case UserConstants::ACTION_DELETE:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_LICENCE_KEY ) {
                  remove();
                  return true;
               } else {
                  mc2log << warn
                         << "UserLicenceKey::readChanges not Type "
                         << " LicenceKey after action delete" << endl;
                  return false;
               }
               break;
            case UserConstants::ACTION_CHANGE:
               return ( readDataChanges( p, pos ) );
               break;
            case UserConstants::ACTION_NOP:
               // Check Type
               type = p->incReadLong( pos );
               if ( type == UserConstants::TYPE_LICENCE_KEY ) {
                  return true;
               } else {
                  mc2log << warn
                         << "UserLicenceKey::readChanges not Type "
                         << " LicenceKey after action NOP" << endl;
                  return false;
               }
               break;
            default:
               mc2log << warn
                      << "UserLicenceKey::readChanges unknown action: "
                      << (int)action << endl;
         }
         return false;
      } else {
         mc2log << error
                << "UserLicenceKey::readChanges not correct type or "
                << " not correct id, giving up. Type " << type
                << " expected " << int(UserConstants::TYPE_LICENCE_KEY)
                << ", id " << id << " expected " << m_id << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserLicenceKey::readChanges no space for changes "
             << "giving up." << endl;
      return false;
   }
}


bool
UserLicenceKey::readDataChanges( const Packet* p, int& pos ) {
   
   if ( p->getLength() - pos - 1 > 0 ) {
      byte nbrFields;

      nbrFields = p->incReadByte( pos );

      // tmpdata
      uint32 licenceLength = m_licenceLength;
      const byte* licenceKey = m_licenceKey;
      MC2String product = m_product;
      MC2String keyType = m_keyType;

      byte field;
      bool ok = true;
      for ( uint8 i = 0 ; 
            (i < nbrFields) && ok && (pos < (int32)p->getLength()) ;
            i ++ ) 
      {
         field = p->incReadByte( pos );
         switch ( field ) {
            case UserConstants::USER_LICENCE_KEY :
               m_changed[ UserConstants::USER_LICENCE_KEY ] = true;
               licenceLength = p->incReadShort( pos );
               licenceKey = p->incReadByteArray( pos, licenceLength );
               break;
            case UserConstants::USER_LICENCE_PRODUCT :
               m_changed[ UserConstants::USER_LICENCE_PRODUCT ] = true;
               product = p->incReadString( pos );
               break;
            case UserConstants::USER_LICENCE_KEY_TYPE :
               m_changed[ UserConstants::USER_LICENCE_KEY_TYPE ] = true;
               keyType = p->incReadString( pos );
               break;

            default:
               mc2log << error
                      << "UserLicenceKey::readDataChanges unknown " 
                      << "fieldType: " << (int)field << endl;
               ok = false;
               break;
         }
      }

      uint32 type = p->incReadLong( pos );
      if ( type != UserConstants::TYPE_LICENCE_KEY ) {
         mc2log << error
                << "UserLicenceKey::readDataChanges Not LicenceKey "
                << " type after data" << endl;
         ok = false;
      }
      if ( ok ) {
         // Transfer changes to membervariables
         if ( m_changed[ UserConstants::USER_LICENCE_KEY] ) {
            setLicence( licenceKey, licenceLength );
         }
         m_product = product;
         m_keyType = keyType;
         return ok;
      } else {
         mc2log << error
                << "UserLicenceKey::readDataChanges failed" << endl;
         return false;
      }
   } else {
      mc2log << error
             << "UserLicenceKey::readDataChanges no space for changes" 
             << endl;
      return false;
   }
}


const byte* 
UserLicenceKey::getLicenceKey() const {
   return m_licenceKey;
}


uint32
UserLicenceKey::getLicenceLength() const {
   return m_licenceLength;
}


void 
UserLicenceKey::setLicence( const byte* licenceKey, uint32 licenceLength ){
   delete [] m_licenceKey;
   m_licenceKey = new byte[ licenceLength ];
   memcpy( m_licenceKey, licenceKey, licenceLength );
   m_licenceLength = licenceLength;

   m_changed[ UserConstants::USER_LICENCE_KEY ] = true;
}

void 
UserLicenceKey::setLicence( const MC2String& licenceKey )
{
   setLicence( reinterpret_cast<const byte*>( licenceKey.c_str() ), 
               licenceKey.length() );
}


const MC2String&
UserLicenceKey::getProduct() const {
   return m_product;
}

void
UserLicenceKey::setProduct( const MC2String& name ) {
   m_product = name;
   m_changed[ UserConstants::USER_LICENCE_PRODUCT ] = true;
}

const MC2String&
UserLicenceKey::getKeyType() const {
   return m_keyType;
}

void
UserLicenceKey::setKeyType( const MC2String& type ) {
   m_keyType = type;
   m_changed[ UserConstants::USER_LICENCE_KEY_TYPE ] = true;
}

MC2String
UserLicenceKey::getLicenceKeyStr() const {
   return MC2String( reinterpret_cast< const char* > ( m_licenceKey ), 
                     reinterpret_cast< const char* > ( 
                        m_licenceKey + m_licenceLength ) );
}

uint32 
UserLicenceKey::getNbrChanged() const {
   uint32 nbrChanged = 0;
   for ( uint8 i = 0; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS; i++ ){
      if ( m_changed[ i ] ) {
         nbrChanged++;
      }
   }
   return nbrChanged;
}


bool 
UserLicenceKey::changed( UserConstants::UserLicenceKeyField field ) const {
   return m_changed[ field ];
}


bool 
UserLicenceKey::isChanged() const {
   return getNbrChanged() > 0 || removed();
}

bool
UserLicenceKey::compare( const UserLicenceKey& o ) const {
   return (getLicenceLength() == o.getLicenceLength() &&
           memcmp( getLicenceKey(), o.getLicenceKey(), 
                   getLicenceLength() ) == 0) &&
      getProduct() == o.getProduct() &&
      getKeyType() == o.getKeyType();
}

uint32
UserLicenceKey::printOldKeyValue( char* target ) const {
   strcat( target, "'" );
   MC2String licenceKey;
   if ( getKeyType() != "imei" ) {
      licenceKey.append( getKeyType() );
      licenceKey.append( ":" );
   }
   licenceKey.append( getLicenceKeyStr() );
   if ( !StringUtility::base64Encode( licenceKey.c_str(), 
                                      target + 1 ) ) 
   {
      mc2log << error << "UserLicenceKey::printValue(): "
             << " Base64 encoding of licence key failed!" << endl; 
      // Discard bad key
      target[ 1 ] = '\0';
   }
   strcat( target, "'" );

   return strlen( target );
}

uint32 
UserLicenceKey::printValue( char* target, 
                            UserConstants::UserLicenceKeyField field ) const
{
   switch ( field ) {
      case UserConstants::USER_LICENCE_KEY_ID :
      case UserConstants::USER_LICENCE_KEY_USERUIN :
         mc2log << error << "UserLicenceKey::printValue asked to print ID "
                << "or userUIN, not supported." << endl;
         MC2_ASSERT( false );
         break;
      case UserConstants::USER_LICENCE_KEY :
         strcat( target, "'" );
         sqlString( target + 1, getLicenceKeyStr().c_str() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_LICENCE_PRODUCT :
         strcat( target, "'" );
         sqlString( target + 1, getProduct() );
         strcat( target, "'" );
         break;
      case UserConstants::USER_LICENCE_KEY_TYPE :
         strcat( target, "'" );
         sqlString( target + 1, getKeyType() );
         strcat( target, "'" );
         break;
      default:
         mc2log << error << "UserLicenceKey::printValue unknown " 
                << "fieldType: " << (int)field << endl;
         break;
   }

   return strlen( target );
}

uint32
UserLicenceKey::printValueIMEIFix( 
   char* target, UserConstants::UserLicenceKeyField field ) const 
{
   if ( field == UserConstants::USER_LICENCE_KEY && isIMEIKey() 
        && isIMEISV() ) {
      MC2String imei15digts;
      if ( extract15Digits( imei15digts ) ) {
         strcat( target, "'" );
         sqlString( target + 1, imei15digts.c_str() );
         strcat( target, "'" );
         return strlen( target );
      } else {
         return printValue( target, field );
      }
   } else {
      return printValue( target, field );
   }
}

bool
UserLicenceKey::extract15Digits( MC2String& target ) const {
   // Imei formats known
   //      AABBBBBBCCCCCCD        Simple
   //      AA-BBBBBB-CCCCCC-D     Standard
   //      AA-BBBBBB-CCCCCC-EE    Standard
   //      AA-BBBBBB-CCCCCC-D-EE  Standard
   //      AABBBBBB-CCCCCC-D      Nokia
   //      AABBBBBB-CCCCCC-D-EE   SonyEricsson
   //      AABBBBBBCCCCCCDEE      ppc5
   //      AABBBBBBCCCCCC/D       sp-5
   //      AABBBB-BB-CCCCCC-D     uiq
   //      300300300300300 later 30030030030030000 from a 
   //                      wf7-s-60-v3-mapmyindia samsung sends this imei?
   //      35676100073688-1 SV:01 ppc3
   //      0x36B9CFA4             Old ppc/sp sends imsi in binary key
   //      ESC-TRAVELTAINER:AAAAAABBBBBBCC:   Esc navigation solution
   uint32 imei14digtsPos = 0;
   const uint32 startSize = target.size();
   while ( imei14digtsPos < getLicenceLength() &&
           target.size() < startSize + 14 ) {
      byte c = getLicenceKey()[ imei14digtsPos ];
      if ( isdigit( c ) ) {
         target.append( 1, char( c ) );
      } else if ( c != '-' ) {
         mc2dbg << "UserLicenceKey::extract15Digits Key is not just [0-9-]: "
                << getLicenceKeyStr() << endl;
         target.clear();
         break;
      }
      ++imei14digtsPos;
   }

   if ( target.size() == 14 ) {
      // And the luhn check
      STLStringUtility::int2str( LuhnCheckDigit::luhnDigit( target ),
                                 target );
      return true;
   } else {
      return false;
   }
}

bool
UserLicenceKey::isIMEIKey() const {
   return getKeyType() == "imei";
}

bool
UserLicenceKey::isIMEISV() const {
   return 
      StringUtility::regexp( "^[0-9]{8}-[0-9]{6}-[0-9]-[0-9]{2}$", 
                             getLicenceKeyStr() ) ||
      StringUtility::regexp( "^[0-9]{17}$",
                             getLicenceKeyStr() );
}

///////////////////////////////////////////////////////////////////////
// UserLicenceKeyProductCounter
///////////////////////////////////////////////////////////////////////
UserLicenceKeyProductCounter::UserLicenceKeyProductCounter( 
   const MC2String& prod )
      : m_product( prod )
{}

int
UserLicenceKeyProductCounter::getNbrKeys( 
   const UserLicenceKeyPVect& keys ) const {
   int nbr = 0;
   for ( UserLicenceKeyPVect::const_iterator it = keys.begin() ;
         it != keys.end() ; ++it ) {
      if ( (*it)->getProduct() == m_product ) {
         ++nbr;
      }
   }
   return nbr;
}

int
UserLicenceKeyProductCounter::getNbrKeys( 
   const UserLicenceKeyVect& keys ) const {
   int nbr = 0;
   for ( UserLicenceKeyVect::const_iterator it = keys.begin() ;
         it != keys.end() ; ++it ) {
      if ( (*it).getProduct() == m_product ) {
         ++nbr;
      }
   }
   return nbr;
}

int
UserLicenceKeyProductCounter::getNbrKeys( ConstUserElementRange keys ) const {
   int nbr = 0;
   for ( UserElementVect::const_iterator it = keys.first ;
         it != keys.second ; ++it ) {
      if ( static_cast< const UserLicenceKey* > ( *it )->getProduct() == 
           m_product ) {
         ++nbr;
      }
   }
   return nbr;
}

void
UserLicenceKeyProductCounter::getProductKeys( 
   const UserLicenceKeyPVect& keys, 
   UserLicenceKeyPVect& prodKeys ) const {
   for ( UserLicenceKeyPVect::const_iterator it = keys.begin() ;
         it != keys.end() ; ++it ) {
      if ( (*it)->getProduct() == m_product ) {
         prodKeys.push_back( *it );
      }
   }
}

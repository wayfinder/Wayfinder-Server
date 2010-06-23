/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DBaseObject.h"
#include "Packet.h"
#include "SQLQuery.h"
#include "STLUtility.h"

// ========================================================================
//                                              DBaseObject construction //

DBaseObject::DBaseObject( int nbrFields )
{
   m_dBaseElm.reserve( nbrFields );
   m_pszQuery = NULL;
   m_lastBoolOp = -1;
}

DBaseObject::DBaseObject( const DBaseObject& copy )
{
   m_dBaseElm.reserve( copy.m_dBaseElm.size() );
   copyMembers( copy );
}
   
DBaseObject::~DBaseObject()
{
   STLUtility::deleteValues( m_dBaseElm );

   delete [] m_pszQuery;
}

void 
DBaseObject::init( int nbrFields )
{
   // Get the "database table template" by using virtual methods.
   const DBaseElement::element_t* dBaseTypes = getFieldTypes();
   const char** pszDBaseNames = getFieldNames();

   // Create the fields by using the "template".
   DBaseElement* pDBElement;
   for( int i = 0; i < nbrFields; i++ ) {
      switch( dBaseTypes[i] ) {
         case DBaseElement::DB_INT : {
            pDBElement = new DBaseInt( i, pszDBaseNames[i] );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_INT_NOTNULL : {
            pDBElement = new DBaseInt( i, pszDBaseNames[i], true );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_SMALLINT : {
            pDBElement = new DBaseSmallInt( i, pszDBaseNames[i] );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_SMALLINT_NOTNULL : {
            pDBElement = new DBaseSmallInt( i, pszDBaseNames[i], true );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_LARGESTRING : {
            pDBElement = new DBaseString( i, pszDBaseNames[i], 255 );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_LARGESTRING_NOTNULL : {
            pDBElement = new DBaseString( i, pszDBaseNames[i], 255, true );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_MEDIUMSTRING : {
            pDBElement = new DBaseString( i, pszDBaseNames[i], 64 );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_MEDIUMSTRING_NOTNULL : {
            pDBElement = new DBaseString( i, pszDBaseNames[i], 64, true );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_SMALLSTRING : {
            pDBElement = new DBaseString( i, pszDBaseNames[i], 32 );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_SMALLSTRING_NOTNULL : {
            pDBElement = new DBaseString( i, pszDBaseNames[i], 32, true );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_BOOL : {
            pDBElement = new DBaseBool( i, pszDBaseNames[i] );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         case DBaseElement::DB_BOOL_NOTNULL : {
            pDBElement = new DBaseBool( i, pszDBaseNames[i], true );
            m_dBaseElm.push_back( pDBElement );
            break;
         }
         default : {
            mc2dbg << "DBaseObject::init() : Using an unknown "
                      "DBaseElement::element_t will cause unpredicted "
                      "behavior." << endl;
            break;
         }
      }
   }

   DEBUG4( if( (uint32) nbrFields != m_dBaseElm.size() )
              mc2dbg4 << "   Internal error in DBaseObject" << endl; );

}

void 
DBaseObject::init( int nbrFields, SQLQuery* pQuery)
{
   // Always call standard init method
   init( nbrFields );

   // Store the result in this object's fields
   DBaseElement* pDBElement;
   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      pDBElement->setValue( pQuery );
   }
}

void 
DBaseObject::init( int nbrFields, const Packet* pPacket, int& pos )
{
   // Always call standard init method
   init( nbrFields );

   // Store the packet data in this object's fields
   DBaseElement* pDBElement;
   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      pDBElement->readValue( pPacket, pos );
   }
}

const char* 
DBaseObject::m_pszCompOp[DBaseObject::COMPOP_NBROPS] = {
   ">",
   "<",
   "like",
   "=",
};

const char* DBaseObject::m_pszBoolOp[DBaseObject::BOOLOP_NBROPS] = {
   "AND",
   "OR",
};

///////////////////////////////////////////////////////////////////////////
//                                            DBaseObject public methods //
///////////////////////////////////////////////////////////////////////////

const 
DBaseObject& DBaseObject::operator=( const DBaseObject& copy )
{
   if( this != &copy ) {
      // First deallocate
      STLUtility::deleteValues( m_dBaseElm );
      delete [] m_pszQuery;

      // then copy
      copyMembers( copy );
   }
   return *this;
}

const char** 
DBaseObject::getFieldDescriptions() const
{
   return NULL;
}

const StringTable::stringCode* 
DBaseObject::getFieldStringCode() const
{
   return NULL;
}

void 
DBaseObject::packInto( Packet* pPacket, int& pos ) const
{
   int nbrFields = m_dBaseElm.size();

   // Store this object's fields into the packet
   DBaseElement* pDBElement;
   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      pDBElement->writeValue( pPacket, pos );
   }
   pPacket->setLength( pos );
}

void 
DBaseObject::readChanges( const Packet* pPacket, int& pos )
{
   int nIndex;
   DBaseElement* pDBElement;

   // Read the changes from the packet.
   int nbrChanges = pPacket->incReadLong( pos );
   for( int i = 0; i < nbrChanges; i++ ) {
      nIndex = pPacket->incReadByte( pos );
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[nIndex]);
      pDBElement->readValue( pPacket, pos );
      pDBElement->setChanged( true );
   }
}

void DBaseObject::addChanges( Packet* pPacket, int& pos ) const
{
   int nbrChanges = 0;
   int nbrFields = m_dBaseElm.size();

   // Make room for the nbrChanges value in the packet.
   int nbrChangesPos = pos;
   pPacket->incWriteLong( pos, nbrChanges );

   // Store the changes into the packet
   DBaseElement* pDBElement;
   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      if( pDBElement->changed() ) {
         pPacket->incWriteByte( pos, static_cast<int>(i) );
         pDBElement->writeValue( pPacket, pos );
         nbrChanges++;
      }
   }
   pPacket->setLength( pos );

   // Store the real nbrChanges value.
   pPacket->incWriteLong( nbrChangesPos, nbrChanges );
}

void DBaseObject::reset()
{
   int nbrFields = m_dBaseElm.size();

   // Set all fields to "not changed"
   DBaseElement* pDBElement;
   for( int i = 0; i < nbrFields; i++ ) {
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
      pDBElement->setChanged( false );
   }
}

const char*
DBaseObject::createTableQuery() const
{
   // Assuming key field stored at position 0.
   DBaseInt* pKey = dynamic_cast<DBaseInt*>(m_dBaseElm[0]);
   if( pKey != NULL ) {
      // Initialize command string
      int pos = 0;
      char* szCommand = new char[1024];
      pos += sprintf( szCommand, "CREATE TABLE %s ( ", getTableStr() );

      int nbrFields = m_dBaseElm.size();

      // Let all fields store info about themself.
      DBaseElement* pDBElement;
      for( int i = 0; i < nbrFields; i++ ) {
         pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
         pos += pDBElement->toDBType( szCommand + pos );
      }

      // Creating string for primary key
      char szPrimKey[100];
      strcpy( szPrimKey, getTableStr() );
      strcat(szPrimKey, "Key");

      // Finalize command string
      pos += sprintf( szCommand + pos, 
                      "CONSTRAINT %s PRIMARY KEY (%s) )",
                      szPrimKey,
                      pKey->getFieldName() );

      mc2dbg4 << "   DBaseObject::createTable() : command string = " 
              << szCommand << endl;

      // prepare the query
      return szCommand;
   }
   else {
      mc2log << warn << "DBaseObject::createTable() : Wrong or missing KEY "
             << "element. Place a DBaseInt at position 0!" << endl;
      return NULL;
   }
}

bool DBaseObject::insert( SQLQuery* pQuery, uint32 id )
{
   MC2_ASSERT(pQuery != NULL);

   if( id == MAX_UINT32 )
      id = getNewId( pQuery );

   // Assuming key field stored at position 0.
   DBaseInt* pKey = dynamic_cast<DBaseInt*>(m_dBaseElm[0]);
   if( pKey != NULL ) {
      pKey->setValue( id );
   }
   else {
      mc2log << warn << "DBaseObject::insert() : Wrong or missing KEY "
             << "element. Place a DBaseInt at position 0!" << endl;
      return false;
   }

   char szCommand[1024];

   // Init command string by calling virtual method.
   int pos = sprintf( szCommand, 
                      "INSERT INTO %s VALUES ( ",
                      getTableStr() );

   int nbrFields = m_dBaseElm.size();
   if( nbrFields > 0 ) {
      // Let all fields store info in command string
      DBaseElement* pDBElement;
      for( int i = 0; i < nbrFields-1; i++ ) {
         pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
         pos += pDBElement->toStr( szCommand + pos, true );
      }
      pDBElement = static_cast<DBaseElement*>(m_dBaseElm[nbrFields-1]);
      pos += pDBElement->toStr( szCommand + pos, false );

      // Add the final touch to the command string
      pos += sprintf( szCommand + pos, " );");

      mc2dbg4 << "   DBaseObject::insert() : command string = " 
             << szCommand << endl;

      // prepare the query
      return pQuery->prepare( szCommand );
   }
   else {
      mc2dbg4 << "DBaseObject::insert() : No fields declared for "
              << "this database class" << endl;
      return false;
   }
}

bool DBaseObject::update( SQLQuery* pQuery )
{
   MC2_ASSERT(pQuery != NULL);

   // Assuming key field stored at position 0.
   DBaseInt* pKey = dynamic_cast<DBaseInt*>(m_dBaseElm[0]);
   if( pKey != NULL ) {
      char szUpdate[1024];
      char szCommand[1024];
      int nbrFields = m_dBaseElm.size();
      int pos = 0;
      int foundChanges = 0;

      // Let all changed fields store info in command string
      DBaseElement* pDBElement;
      for( int i = 1; i < nbrFields; i++ ) {
         pDBElement = static_cast<DBaseElement*>(m_dBaseElm[i]);
         if( pDBElement->changed() ) {
            //Add comma to all but the first change.
            if( foundChanges > 0 )
               pos += sprintf( szUpdate + pos, ", " );

            foundChanges++;
            pos += pDBElement->toQuery( szUpdate + pos,
                                        m_pszCompOp[COMPOP_EXACT] );
         }
      }

      // Create update command
      pos += sprintf( szCommand,
                      "UPDATE %s SET %s WHERE %s = %d ;",
                      getTableStr(),
                      szUpdate, 
                      pKey->getFieldName(),
                      pKey->getValue() );

      mc2dbg4 << "   DBaseObject::update() : command string = " 
              << szCommand << endl;

      // prepare the query
      return pQuery->prepare( szCommand );
   }
   else {
      mc2log << warn << "DBaseObject::update() : Wrong or missing KEY "
             << "element. Place a DBaseInt at position 0!" << endl;
      return false;
   }
}

bool DBaseObject::remove( SQLQuery* pQuery )
{
   MC2_ASSERT(pQuery != NULL);

   // Assuming key field stored at position 0.
   DBaseInt* pKey = dynamic_cast<DBaseInt*>(m_dBaseElm[0]);
   if( pKey != NULL ) {
      char szCommand[1024];

      sprintf( szCommand,
               "DELETE FROM %s WHERE %s = %d;",
               getTableStr(),
               pKey->getFieldName(),
               pKey->getValue() );

      mc2dbg4 << "   DBaseObject::remove() : command string = " 
              << szCommand << endl;

      // prepare the query
      return pQuery->prepare( szCommand );
   }
   else {
      mc2log << warn << "DBaseObject::remove() : Wrong or missing KEY "
             << "element. Place a DBaseInt at position 0!" << endl;
      return false;
   }
}

void DBaseObject::getAllItems()
{
   if( m_pszQuery == NULL )
      m_pszQuery = new char[1024];

   sprintf( m_pszQuery,
            "SELECT * FROM %s",
            getTableStr() );              
}

void DBaseObject::addQuery( void* value,
                            int field,
                            DBaseObject::comp_t compOp,
                            DBaseObject::bool_t boolOp )
{
   // Initialize searchstring if needed
   if( m_pszQuery == NULL ) {
      m_pszQuery = new char[1024];

      sprintf( m_pszQuery,
               "SELECT * FROM %s WHERE ",
               getTableStr() );
   }

   // Make a search substring
   char szStr[200];

   DBaseElement* pDBElement = static_cast<DBaseElement*>(m_dBaseElm[field]);
   pDBElement->toQuery( szStr, m_pszCompOp[ compOp ], value );

   // Add substring to searchstring
   int len = strlen( m_pszQuery );
   if( m_lastBoolOp >= 0 ) {
      sprintf( m_pszQuery + len,
               "%s %s ",
               m_pszBoolOp[ m_lastBoolOp ],
               szStr );
   }
   else {
      strcat( m_pszQuery + len, szStr );
   }

   m_lastBoolOp = boolOp;
}

bool DBaseObject::makeQuery( Packet* pPacket, int& pos ) const
{
   if( m_pszQuery == NULL ) {
      DEBUG2( cerr << "DBaseObject::makeQuery : Query is NULL." << endl );
      return false;
   }

   strcat( m_pszQuery, ";" );
   int size = strlen( m_pszQuery ) + 1;

   if ( pos + size < static_cast<int>(pPacket->getBufSize()) ) {
      pPacket->incWriteString(pos, m_pszQuery);
      pPacket->setLength(pos);
      return true;
   }
   else {
      DEBUG2( cerr << "DBaseObject::makeQuery : String too large for packet"
                   << endl );
      return false;
   }   
}

bool DBaseObject::readQuery( Packet* pPacket, int& pos )
{
   char* str;
   pPacket->incReadString( pos, str);

   delete [] m_pszQuery;
   m_pszQuery = StringUtility::newStrDup( str );

   return true;
}

bool DBaseObject::compareTo( const DBaseObject& dbObj ) const
{
   int nbrFields  = m_dBaseElm.size();
   int nbrFields2 = dbObj.m_dBaseElm.size();

   // Compare the objects' fields
   if( nbrFields == nbrFields2 ) {
      bool isEqual = true;
      for( int i = 0; i < nbrFields; i++ ) {
         if( *(m_dBaseElm[i]) != *(dbObj.m_dBaseElm[i]) ) {
            isEqual = false;
            break;
         }
      }

      return isEqual;
   }
   else {
      // Different number of fields. The objects are not equal.
      return false;
   }
}

void
DBaseObject::dump() const
{
   char str[1024];
   for (uint32 i=0; i<m_dBaseElm.size(); i++){
      DBaseElement* elem = static_cast<DBaseElement*>(m_dBaseElm[i]);
      elem->toStr(str, i != m_dBaseElm.size()-1);
      cout << str;
   }
   cout << endl;
}

///////////////////////////////////////////////////////////////////////////
//                                           DBaseObject private methods //
///////////////////////////////////////////////////////////////////////////

uint32 DBaseObject::getNewId( SQLQuery* pQuery )
{
   MC2_ASSERT(pQuery != NULL);

   // Assuming key field stored at position 0.
   DBaseInt* pKey = dynamic_cast<DBaseInt*>(m_dBaseElm[0]);
   if( pKey != NULL ) {
      uint32 newID;
      char szQuery[500];
      bool exists = false;

      do {
         newID = 1 + (uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));

         sprintf( szQuery,
                  "SELECT %s FROM %s WHERE %s = %d;",
                  pKey->getFieldName(),
                  getTableStr(),
                  pKey->getFieldName(), // yes, twice
                  newID );

         if ( !pQuery->prepAndExec(szQuery) ) {
            mc2log << warn << "DBaseObject::getNewId : query " 
                   << szQuery << " failed" << endl;
            return 0;
         } 
         else {
            exists = pQuery->nextRow();
         }
      } while( exists );

      return newID;
   }
   else {
      mc2dbg1 << "DBaseObject::getNewId() : Wrong or missing KEY "
                 "element. Place a DBaseInt at position 0!" << endl;
      return 0;
   }
}

void DBaseObject::copyMembers( const DBaseObject& copy )
{
   if( copy.m_pszQuery != NULL ) {
      m_pszQuery = new char[1024];
      strcpy( m_pszQuery, copy.m_pszQuery );
   }
   else 
      m_pszQuery = NULL;

   m_lastBoolOp = copy.m_lastBoolOp;

   int nbrFields = copy.m_dBaseElm.size();

   DBaseInt* pInt;
   DBaseSmallInt* pSmInt;
   DBaseString* pStr;
   DBaseBool* pBool;
   for( int i = 0; i < nbrFields; i++ ) {
      if( (pInt = dynamic_cast<DBaseInt*>(copy.m_dBaseElm[i])) != NULL ) {
         m_dBaseElm.push_back( new DBaseInt( *pInt ) );
      }
      else if( (pSmInt = dynamic_cast<DBaseSmallInt*>(copy.m_dBaseElm[i])) != NULL ) {
         m_dBaseElm.push_back( new DBaseSmallInt( *pSmInt ) );
      }
      else if( (pStr = dynamic_cast<DBaseString*>(copy.m_dBaseElm[i])) != NULL ) {
         m_dBaseElm.push_back( new DBaseString( *pStr ) );
      }
      else if( (pBool = dynamic_cast<DBaseBool*>(copy.m_dBaseElm[i])) != NULL ) {
         m_dBaseElm.push_back( new DBaseBool( *pBool ) );
      }
   }
}

///////////////////////////////////////////////////////////////////////////
//                                 DBaseElement construction/destruction //
///////////////////////////////////////////////////////////////////////////

DBaseElement::DBaseElement()
{
   m_pszFieldName = NULL;
   m_changed = false;
}

DBaseElement::DBaseElement( int fieldNum, const char* pszFieldName,
                            bool bNotNull )
{
   // Just copy the pointer
   m_pszFieldName = pszFieldName;
   m_fieldNum = fieldNum;
   m_changed = false;
   m_notNull = bNotNull;
}

DBaseElement::~DBaseElement()
{
}

///////////////////////////////////////////////////////////////////////////
//                                     DBaseInt construction/destruction //
///////////////////////////////////////////////////////////////////////////

DBaseInt::DBaseInt( int fieldNum, const char* pszFieldName, bool bNotNull )
   : DBaseElement( fieldNum, pszFieldName, bNotNull )
{
   m_iValue = 0;
}

DBaseInt::~DBaseInt()
{
}

///////////////////////////////////////////////////////////////////////////
//                                               DBaseInt public methods //
///////////////////////////////////////////////////////////////////////////
   
bool DBaseInt::operator == (const DBaseElement& el) const 
{
   const DBaseInt* pInt = dynamic_cast<const DBaseInt*>(&el);
   return ((pInt != NULL) && (m_iValue == pInt->m_iValue));
}

bool DBaseInt::operator != (const DBaseElement& el) const
{
   const DBaseInt* pInt = dynamic_cast<const DBaseInt*>(&el);
   return ((pInt == NULL) || (m_iValue != pInt->m_iValue));
}

void DBaseInt::readValue( const Packet* pPacket, int& pos )
{
   m_iValue = pPacket->incReadLong( pos );
}

void DBaseInt::writeValue( Packet* pPacket, int& pos ) const
{
   pPacket->incWriteLong( pos, m_iValue );
}

int DBaseInt::toStr( char* pszStr, bool addComma ) const
{
   if( addComma )
      return sprintf( pszStr, "%d, ", m_iValue );
   else
      return sprintf( pszStr, "%d", m_iValue );
}

int DBaseInt::toQuery( char* pszStr, 
                       const char* pszCompOp, 
                       void* pValue ) const
{
   const int* pIntValue;
   if( pValue != NULL )
      pIntValue = static_cast<int*>(pValue);
   else
      pIntValue = &m_iValue;

   return sprintf( pszStr, 
                   "%s %s %d ",
                   m_pszFieldName,
                   pszCompOp,
                   *pIntValue );
}

int DBaseInt::toDBType( char* pszStr ) const
{
   if( m_notNull )
      return sprintf( pszStr, "%s INT NOT NULL, ", m_pszFieldName );
   else
      return sprintf( pszStr, "%s INT, ", m_pszFieldName );
}

void DBaseInt::setValue( SQLQuery* pQuery )
{
   m_iValue = strtol( pQuery->getColumn(m_fieldNum), NULL, 10 );
}

///////////////////////////////////////////////////////////////////////////
//                                DBaseSmallInt construction/destruction //
///////////////////////////////////////////////////////////////////////////

DBaseSmallInt::DBaseSmallInt( int fieldNum, const char* pszFieldName,
                              bool bNotNull )
   : DBaseElement( fieldNum, pszFieldName, bNotNull )
{
   m_nValue = 0;
}

DBaseSmallInt::~DBaseSmallInt()
{
}

///////////////////////////////////////////////////////////////////////////
//                                          DBaseSmallInt public methods //
///////////////////////////////////////////////////////////////////////////

bool DBaseSmallInt::operator == (const DBaseElement& el) const 
{
   const DBaseSmallInt* pInt = dynamic_cast<const DBaseSmallInt*>(&el);
   return ((pInt != NULL) && (m_nValue == pInt->m_nValue));
}

bool DBaseSmallInt::operator != (const DBaseElement& el) const
{
   const DBaseSmallInt* pInt = dynamic_cast<const DBaseSmallInt*>(&el);
   return ((pInt == NULL) || (m_nValue != pInt->m_nValue));
}

void DBaseSmallInt::readValue( const Packet* pPacket, int& pos )
{
   m_nValue = pPacket->incReadShort( pos );
}

void DBaseSmallInt::writeValue( Packet* pPacket, int& pos ) const
{
   pPacket->incWriteShort( pos, m_nValue );
}

int DBaseSmallInt::toStr( char* pszStr, bool addComma ) const
{
   if( addComma )
      return sprintf( pszStr, "%d, ", m_nValue );
   else
      return sprintf( pszStr, "%d", m_nValue );
}

int DBaseSmallInt::toQuery( char* pszStr,
                            const char* pszCompOp,
                            void* pValue ) const
{
   const int16* pShortValue;
   if( pValue != NULL )
      pShortValue = static_cast<int16*>(pValue);
   else
      pShortValue = &m_nValue;

   return sprintf( pszStr, 
                   "%s %s %d ",
                   m_pszFieldName,
                   pszCompOp,
                   *pShortValue );
}

int DBaseSmallInt::toDBType( char* pszStr ) const
{
   if( m_notNull )
      return sprintf( pszStr, "%s SMALLINT NOT NULL, ", m_pszFieldName );
   else
      return sprintf( pszStr, "%s SMALLINT, ", m_pszFieldName );
}

void DBaseSmallInt::setValue( SQLQuery* pQuery )
{
   m_nValue = static_cast<int16>( 
      strtol( pQuery->getColumn(m_fieldNum), NULL, 10 ) );
}

///////////////////////////////////////////////////////////////////////////
//                                  DBaseString construction/destruction //
///////////////////////////////////////////////////////////////////////////

DBaseString::DBaseString( const DBaseString& copy )
   : DBaseElement( copy )
{
   m_strSize = copy.m_strSize;
   if( copy.m_pszValue != NULL )
      m_pszValue = StringUtility::newStrDup( copy.m_pszValue );
   else
      m_pszValue = NULL;
}

DBaseString::DBaseString( int fieldNum, const char* pszFieldName, int size,
                          bool bNotNull )
   : DBaseElement( fieldNum, pszFieldName, bNotNull )
{
   m_strSize = size;
   m_pszValue = NULL;
}

DBaseString::~DBaseString()
{
   delete [] m_pszValue;
}

///////////////////////////////////////////////////////////////////////////
//                                            DBaseString public methods //
///////////////////////////////////////////////////////////////////////////

bool DBaseString::operator == (const DBaseElement& el) const 
{
   const DBaseString* pStr = dynamic_cast<const DBaseString*>(&el);
   if( pStr == NULL )
      return false;

   if( (m_pszValue == NULL) || (pStr->m_pszValue == NULL ) )
      return m_pszValue == pStr->m_pszValue;
   else
      return strcmp(m_pszValue, pStr->m_pszValue) == 0;
}

bool DBaseString::operator != (const DBaseElement& el) const
{
   const DBaseString* pStr = dynamic_cast<const DBaseString*>(&el);
   if( pStr == NULL )
      return true;

   if( (m_pszValue == NULL) || (pStr->m_pszValue == NULL ) )
      return m_pszValue != pStr->m_pszValue;
   else
      return strcmp(m_pszValue, pStr->m_pszValue) != 0;
}

void DBaseString::readValue( const Packet* pPacket, int& pos )
{
   char* pszTemp;
   pPacket->incReadString( pos, pszTemp );

   setValue( pszTemp );
}

void DBaseString::writeValue( Packet* pPacket, int& pos ) const
{
   pPacket->incWriteString( pos, m_pszValue );
}

int DBaseString::toStr( char* pszStr, bool addComma ) const
{
   if( addComma )
      return sprintf( pszStr, "'%s', ", m_pszValue );
   else
      return sprintf( pszStr, "'%s'", m_pszValue );
}

int DBaseString::toQuery( char* pszStr,
                          const char* pszCompOp,
                          void* pValue ) const
{
   const char* pszStrVal;
   if( pValue != NULL )
      pszStrVal = static_cast<char*>(pValue);
   else
      pszStrVal = m_pszValue;

   return sprintf( pszStr, 
                   "%s %s '%s' ",
                   m_pszFieldName,
                   pszCompOp,
                   pszStrVal );
}

int DBaseString::toDBType( char* pszStr ) const
{
   if( m_notNull )
      return sprintf( pszStr, 
                      "%s VARCHAR(%d) NOT NULL, ",
                      m_pszFieldName,
                      m_strSize );
   else
      return sprintf( pszStr, 
                      "%s VARCHAR(%d), ",
                      m_pszFieldName,
                      m_strSize );
}

void DBaseString::setValue( SQLQuery* pQuery )
{
   setValue( pQuery->getColumn(m_fieldNum) );
}

///////////////////////////////////////////////////////////////////////////
//                                     DBaseBool construction/destruction //
///////////////////////////////////////////////////////////////////////////

DBaseBool::DBaseBool( int fieldNum, const char* pszFieldName, bool bNotNull )
   : DBaseElement( fieldNum, pszFieldName,bNotNull  )
{
   m_bValue = false;
}

DBaseBool::~DBaseBool()
{
}

///////////////////////////////////////////////////////////////////////////
//                                               DBaseBool public methods //
///////////////////////////////////////////////////////////////////////////

bool DBaseBool::operator == (const DBaseElement& el) const 
{
   const DBaseBool* pBool = dynamic_cast<const DBaseBool*>(&el);
   return ((pBool != NULL) && (m_bValue == pBool->m_bValue));
}

bool DBaseBool::operator != (const DBaseElement& el) const
{
   const DBaseBool* pBool = dynamic_cast<const DBaseBool*>(&el);
   return ((pBool == NULL) || (m_bValue != pBool->m_bValue));
}

void DBaseBool::readValue( const Packet* pPacket, int& pos )
{
   m_bValue = pPacket->incReadByte( pos ) != 0;
}

void DBaseBool::writeValue( Packet* pPacket, int& pos ) const
{
   pPacket->incWriteByte( pos, m_bValue ? 1 : 0 );
}

int DBaseBool::toStr( char* pszStr, bool addComma ) const
{
   if( addComma )
      return sprintf( pszStr, "%d, ", m_bValue ? 1 : 0 );
   else
      return sprintf( pszStr, "%d", m_bValue ? 1 : 0 );
}

int DBaseBool::toQuery( char* pszStr, 
                        const char* pszCompOp,
                        void* pValue ) const
{
   const bool* pBoolValue;
   if( pValue != NULL )
      pBoolValue = static_cast<bool*>(pValue);
   else
      pBoolValue = &m_bValue;

   return sprintf( pszStr, 
                   "%s %s %d ",
                   m_pszFieldName,
                   pszCompOp,
                   *pBoolValue ? 1 : 0 );
}

int DBaseBool::toDBType( char* pszStr ) const
{
   if( m_notNull )
      return sprintf( pszStr, "%s SMALLINT NOT NULL, ", m_pszFieldName );
   else
      return sprintf( pszStr, "%s SMALLINT, ", m_pszFieldName );
}

void DBaseBool::setValue( SQLQuery* pQuery)
{
   uint32 val = strtol(pQuery->getColumn(m_fieldNum), NULL, 10);
   if( val == 1 )
      m_bValue = true;
   else
      m_bValue = false;
}

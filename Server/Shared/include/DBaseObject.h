/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DBASEOBJECT_H
#define DBASEOBJECT_H

#include "config.h"
#include "StringUtility.h"
#include "StringTable.h"
#include "vector"


class Packet;
class SQLQuery;


/**
 *    Abstract class for encapsulating databasetypes.
 *
 */
class DBaseElement {
   public:
      enum element_t {
         DB_INT = 0,
         DB_INT_NOTNULL,
         DB_SMALLINT,
         DB_SMALLINT_NOTNULL,
         DB_LARGESTRING,
         DB_LARGESTRING_NOTNULL,
         DB_MEDIUMSTRING,
         DB_MEDIUMSTRING_NOTNULL,
         DB_SMALLSTRING,
         DB_SMALLSTRING_NOTNULL,
         DB_BOOL,
         DB_BOOL_NOTNULL,

         DB_NBR_ITEMS
      };

      DBaseElement();

      DBaseElement( int fieldNum, const char* pszFieldName,
                    bool bNotNull = false );

      virtual ~DBaseElement();

      virtual bool operator == (const DBaseElement& elm) const = 0;
   
      virtual bool operator != (const DBaseElement& elm) const = 0;

      virtual void readValue( const Packet* pPacket, int& pos ) = 0;

      virtual void writeValue( Packet* pPacket, int& pos ) const = 0;

      virtual int toStr( char* pszStr, bool addComma ) const = 0;

      virtual int toQuery( char* pszStr, 
                           const char* pszCompOp,
                           void* pValue = NULL ) const = 0;

      virtual int toDBType( char* pszStr ) const = 0;

      virtual void setValue( SQLQuery* pQuery ) = 0;

      inline bool changed() const;

      inline void setChanged( bool changed );

      inline const char* getFieldName() const;

      inline bool notNull() const;

      inline void setNotNull( bool bNotNull );

   protected:
      bool m_changed;

      const char* m_pszFieldName;
      int m_fieldNum;

      bool m_notNull;
};

typedef std::vector< DBaseElement* > DBaseElementVector;

/**
 *    One query or similar that should be used with a SQL database.
 *    Could easily be inserted and extracted from a Packet.
 *
 */
class DBaseObject {
   public:

      /**
       *
       */
      enum comp_t {
         COMPOP_GREATER = 0,
         COMPOP_LESS,
         COMPOP_EQUAL,
         COMPOP_EXACT,

         COMPOP_NBROPS
      };

      enum bool_t {
         BOOLOP_AND = 0,
         BOOLOP_OR,

         BOOLOP_NBROPS
      };

      /**
       *    Create a new DBaseObject with a specified number of fields.
       *    Will create the fields according to the specifications in
       *    the virtual methods @p getFieldTypes() and @p getFieldNames().
       *    @param nbrFields  The number of fields in the database.
       */
      DBaseObject( int nbrFields );

      /**
       *    Copy-constructor.
       *    @param copy The source-object of this one.
       */
      DBaseObject( const DBaseObject& copy );
      
      /**   
       *    Delete all the objects in the database.
       */
      virtual ~DBaseObject();

      /**
       *    Get the name of the table.
       *    @return  A pointer to a string that contains the name of
       *             the table. This must not be deleted.
       */
      virtual const char* getTableStr() const = 0;

      /**
       *    Get the description-strings of all the columns.
       *    @return  An array of pointers to a strings with the names 
       *             of the columns in the table. This must not be deleted.
       */
      virtual const char** getFieldDescriptions() const;

      /**
       *    Get the description-strings of all the columns in terms of
       *    string codes.
       *    @return  An array of pointers to a string codes with the 
       *             names of the columns in the table. This must not 
       *             be deleted.
       */
      virtual const StringTable::stringCode* getFieldStringCode() const;

      /**
       *    Asign-operator.
       *    @param copy The object with the values to asign to this one.
       *    @return A reference to this object.
       */
      const DBaseObject& operator=( const DBaseObject& copy );

      /**
       *    Insert this object into a preallocated packet at a given
       *    position.
       *    @param pPacket The packet where to insert this object.
       *    @param pos     In/out-parameter with the position in the
       *                   packet where this object should be inserted.
       *                   Will be updated to the position after this
       *                   object in the packet.
       */
      void packInto( Packet* pPacket, int& pos ) const;

      void readChanges( const Packet* pPacket, int& pos );

      void addChanges( Packet* pPacket, int& pos ) const;

      void reset();

      /**
       *    Create a query to create the table described in this object.
       *    Should be delete[]d by the caller.
       *    @return  Pointer to the query, NULL if there was a problem.
       */
      const char* createTableQuery() const;

      /**
       *    Create a query to insert the data in this object.
       *    @param   pQuery   A pointer to the SQLDatabase where to
       *                      insert the data.
       *    @param   id       The id to use, if MAX_UINT32 a new unique
       *                      id will be automatically generated
       *    @return  True if the query was successfully created,
       *             false otherwise.
       */
      bool insert( SQLQuery* pQuery, uint32 id = MAX_UINT32 );

      /**
       *    Create a query to update the database using the data in this
       *    object.
       *    Insert the data in this object into the SQL-database.
       *    @param   pQuery   A pointer to the SQLDatabase where to
       *                      insert the data.
       *    @return  True if the query was successfully created,
       *             false otherwise.
       */
      bool update( SQLQuery* pQuery );

      /**
       *    Create a query to remove the corresponding data in the database.
       *    @param   pQuery   A pointer to the SQLDatabase where to
       *                      insert the data.
       *    @return  True if the query was successfully created,
       *             false otherwise.
       */
      bool remove( SQLQuery* pQuery );

      void getAllItems();

      void addQuery( void* value,
                     int field,
                     DBaseObject::comp_t compOp,
                     DBaseObject::bool_t boolOp );

      bool makeQuery( Packet* pPacket, int& pos ) const;

      bool readQuery( Packet* pPacket, int& pos );

      bool compareTo( const DBaseObject& dbObj ) const;

      inline const char* getQuery() const;

      /**
       *    Print the value of all the elements to standard out.
       */
      void dump() const;

   protected:
      virtual const char** getFieldNames() const = 0;

      virtual const DBaseElement::element_t* getFieldTypes() const = 0;

      void init( int nbrFields );

      void init( int nbrFields, SQLQuery* pQuery );

      void init( int nbrFields, const Packet* pPacket, int& pos );

      inline int getInt( unsigned int field ) const;

      inline void setInt( unsigned int field, int iValue );

      inline int16 getSmallInt( unsigned int field ) const;

      inline void setSmallInt( unsigned int field, int16 nValue );

      inline bool getBool( unsigned int field ) const;

      inline void setBool( unsigned int field, bool bValue );

      inline const char* getString( unsigned int field ) const;

      inline void setString( unsigned int field, const char* pszValue );

      DBaseElementVector m_dBaseElm;

      static const char* m_pszCompOp[];

      static const char* m_pszBoolOp[];

      /// Array to remember the query.
      char* m_pszQuery;

      /// The booloperation from the last addQuery().
      int m_lastBoolOp;

   private:
      uint32 getNewId( SQLQuery* pQuery );

      void copyMembers( const DBaseObject& copy );
};



/**
 * Class encapsulating an integer.
 *
 */
class DBaseInt : public DBaseElement
{
public:
   DBaseInt( int fieldNum, const char* pszFieldName, bool bNotNull = false );

   virtual ~DBaseInt();

   virtual bool operator == (const DBaseElement& elm) const;
   
   virtual bool operator != (const DBaseElement& elm) const;

   void readValue( const Packet* pPacket, int& pos );

   void writeValue( Packet* pPacket, int& pos ) const;

   int toStr( char* pszStr, bool addComma ) const;

   int toQuery( char* pszStr, 
                const char* pszCompOp,
                void* pValue = NULL ) const;

   int toDBType( char* pszStr ) const;

   void setValue( SQLQuery* pQuery );

   inline void setValue( int iValue );

   inline int getValue() const;

protected:
   int32 m_iValue;
};


/**
 * Class encapsulating a smallint.
 *
 */
class DBaseSmallInt : public DBaseElement
{
public:
   DBaseSmallInt( int fieldNum, const char* pszFieldName, 
                  bool bNotNull = false );

   virtual ~DBaseSmallInt();

   virtual bool operator == (const DBaseElement& elm) const;
   
   virtual bool operator != (const DBaseElement& elm) const;

   void readValue( const Packet* pPacket, int& pos );

   void writeValue( Packet* pPacket, int& pos ) const;

   int toStr( char* pszStr, bool addComma ) const;

   int toQuery( char* pszStr,
                const char* pszCompOp,
                void* pValue = NULL ) const;

   int toDBType( char* pszStr ) const;

   void setValue( SQLQuery* pQuery );

   inline void setValue( int16 nValue );

   inline int16 getValue() const;

protected:
   int16 m_nValue;
};


/**
 * Class encapsulating a string.
 *
 */
class DBaseString : public DBaseElement
{
public:
   DBaseString( int fieldNum, const char* pszFieldName, int size,
                bool bNotNull = false );

   DBaseString( const DBaseString& copy );

   virtual ~DBaseString();

   virtual bool operator == (const DBaseElement& elm) const;
   
   virtual bool operator != (const DBaseElement& elm) const;

   void readValue( const Packet* pPacket, int& pos );

   void writeValue( Packet* pPacket, int& pos ) const;

   int toStr( char* pszStr, bool addComma ) const;

   int toQuery( char* pszStr,
                const char* pszCompOp,
                void* pValue = NULL ) const;

   int toDBType( char* pszStr ) const;

   void setValue( SQLQuery* pQuery );

   inline void setValue( const char* pszStr );

   inline const char* getValue() const;

protected:
   char* m_pszValue;

   int m_strSize;
};


/**
 * Class encapsulating a boolean.
 *
 */
class DBaseBool : public DBaseElement
{
public:
   DBaseBool( int fieldNum, const char* pszFieldName, bool bNotNull = false );

   virtual ~DBaseBool();

   virtual bool operator == (const DBaseElement& elm) const;
   
   virtual bool operator != (const DBaseElement& elm) const;

   void readValue( const Packet* pPacket, int& pos );

   void writeValue( Packet* pPacket, int& pos ) const;

   int toStr( char* pszStr, bool addComma ) const;

   int toQuery( char* pszStr,
                const char* pszCompOp,
                void* pValue = NULL ) const;

   int toDBType( char* pszStr ) const;

   void setValue( SQLQuery* pQuery );

   inline void setValue( bool bValue );

   inline bool getValue() const;

protected:
   bool m_bValue;
};

/*
 * Inlined methods
 */

inline bool DBaseElement::changed() const
{
   return m_changed;
}

inline void DBaseElement::setChanged( bool changed )
{
   m_changed = changed;
}

inline const char* DBaseElement::getFieldName() const
{
   return m_pszFieldName;
}

inline void DBaseElement::setNotNull( bool bNotNull )
{
   m_notNull = bNotNull;
}

inline bool DBaseElement::notNull() const
{
   return m_notNull;
}

inline const char* DBaseObject::getQuery() const
{
   return m_pszQuery;
}

inline int DBaseObject::getInt( unsigned int field ) const
{
   DBaseInt* pInt = NULL;
   
   if ( field < m_dBaseElm.size() ) {
      pInt = dynamic_cast<DBaseInt*>( m_dBaseElm[field] );
   }

   if( pInt != NULL )
      return pInt->getValue();
   else
      return MAX_INT32;
}

inline void DBaseObject::setInt( unsigned int field, int iValue )
{
   DBaseInt* pInt = NULL; 
 
   if ( field < m_dBaseElm.size() ) {
      pInt = dynamic_cast<DBaseInt*>( m_dBaseElm[field] );
   }

   if( pInt != NULL )
      pInt->setValue( iValue );
}

inline int16 DBaseObject::getSmallInt( unsigned int field ) const
{
   DBaseSmallInt* pSmallInt = NULL;

   if ( field < m_dBaseElm.size() ) {
      pSmallInt = dynamic_cast<DBaseSmallInt*>( m_dBaseElm[field] );
   }

   if( pSmallInt != NULL )
      return pSmallInt->getValue();
   else
      return MAX_INT16;
}

inline void DBaseObject::setSmallInt( unsigned int field, int16 nValue )
{
   DBaseSmallInt* pSmallInt = NULL;

   if ( field < m_dBaseElm.size() ) {
      pSmallInt = dynamic_cast<DBaseSmallInt*>( m_dBaseElm[field] );
   }

   if( pSmallInt != NULL )
      pSmallInt->setValue( nValue );
}

inline bool DBaseObject::getBool( unsigned int field ) const
{
   DBaseBool* pBool = NULL;
   
   if ( field < m_dBaseElm.size() ) {
      pBool = dynamic_cast<DBaseBool*>( m_dBaseElm[field] );
   }

   if( pBool != NULL )
      return pBool->getValue();
   else
      return false;
}

inline void DBaseObject::setBool( unsigned int field, bool bValue )
{
   DBaseBool* pBool = NULL;

   if ( field < m_dBaseElm.size() ) {
      pBool = dynamic_cast<DBaseBool*>( m_dBaseElm[field] );
   }

   if( pBool != NULL )
      pBool->setValue( bValue );
}

inline const char* DBaseObject::getString( unsigned int field ) const
{
   DBaseString* pStr = NULL;

   if ( field < m_dBaseElm.size() ) {
      pStr = dynamic_cast<DBaseString*>( m_dBaseElm[field] );
   }

   if( pStr != NULL )
      return pStr->getValue();
   else
      return NULL;
}

inline void DBaseObject::setString( unsigned int field, const char* pszValue )
{
   DBaseString* pStr = NULL;

   if ( field < m_dBaseElm.size() ) {
      pStr = dynamic_cast<DBaseString*>( m_dBaseElm[field] );
   }

   if( pStr != NULL )
      pStr->setValue( pszValue );
}

inline void DBaseInt::setValue( int iValue )
{
   m_iValue = iValue;
   m_changed = true;
}

inline int DBaseInt::getValue() const
{
   return m_iValue;
}

inline void DBaseSmallInt::setValue( int16 nValue )
{
   m_nValue = nValue;
   m_changed = true;
}

inline int16 DBaseSmallInt::getValue() const
{
   return m_nValue;
}

inline void DBaseBool::setValue( bool bValue )
{
   m_bValue = bValue;
   m_changed = true;
}

inline bool DBaseBool::getValue() const
{
   return m_bValue;
}

inline void DBaseString::setValue( const char* pszStr )
{
   delete m_pszValue;
   m_pszValue = StringUtility::newStrDup( pszStr );
   m_changed = true;
}

inline const char* DBaseString::getValue() const
{
   return m_pszValue;
}

#endif //DBASEOBJECT_H

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLDataContainer.h"
#include "StringUtility.h"
#include "Packet.h"
#include <algorithm>


SQLDataContainer::SQLDataContainer()
{
}

SQLDataContainer::SQLDataContainer( const rowName_t& rowNames, 
                                    const rowValues_t::value_type& rowValue )
   : m_rowNames( rowNames ) 
{
   m_rowValues.push_back( rowValue );
}

SQLDataContainer::~SQLDataContainer()
{
}

void
SQLDataContainer::appendRowCol( MC2String& s ) const
{
   MC2_ASSERT( !m_rowValues.empty() );

   rowName_t::const_iterator it = m_rowNames.begin();
   rowValues_t::value_type::const_iterator it2 = (*m_rowValues.begin()).begin();
   for ( ; it != m_rowNames.end(); ++it, ++it2 ) {
      if ( it != m_rowNames.begin() ) {
         s.append( " AND " );
      }
      // Might be changed to SQLEscape if needed to use wildcards
      s.append( StringUtility::SQLEscapeSecure( it->first ) );
      // Check if its a numeric type and ommit ' ' around the value if it is
      if ( !it->second ) {
         s.append( "='" );
         s.append( StringUtility::SQLEscapeSecure( *it2  ) );
         s.append( "'" );
      } else {
         s.append( "=" );
         s.append( StringUtility::SQLEscapeSecure( *it2  ) );
      }
   }
}

void
SQLDataContainer::appendValues( MC2String& s ) const
{
   MC2_ASSERT( !m_rowValues.empty() );
   
   rowName_t::const_iterator it = m_rowNames.begin();
   s.append( "( " );
   // Add the keys
   for ( ; it != m_rowNames.end(); ++it ) {
      if ( it != m_rowNames.begin() ) {
         s.append( ", " );
      }
      s.append( StringUtility::SQLEscapeSecure( it->first ) );
   }
   s.append( " ) VALUES ( " );
   // Add the values
   it = m_rowNames.begin();
   rowValues_t::value_type::const_iterator it2 = (*m_rowValues.begin()).begin();
   for ( ; it2 != (*m_rowValues.begin()).end(); ++it, ++it2 ) {
      if ( it2 != (*m_rowValues.begin()).begin() ) {
         s.append( ", " );
      }
      // Check if its a numeric type and ommit ' ' around the value if it is
      if ( !it->second ) {
         s.append( "'" );
         s.append( StringUtility::SQLEscapeSecure( *it2  ) );
         s.append( "'" );
      } else {
         s.append( StringUtility::SQLEscapeSecure( *it2  ) );
      }
   }
   s.append( " )" );
}

bool 
SQLDataContainer::getSQLData( SQLQuery* p )
{
   // Clear data to make sure we start with empty vectors
   m_rowNames.clear();
   m_rowValues.clear();
   uint32 nbrCol = p->getNumColumns();
   vector< MC2String > rowNames;
   p->getColumnNames( rowNames );
   for ( uint32 i = 0; i != nbrCol; ++i ) {
      m_rowNames.push_back( NameAndIsNum( rowNames[ i ], false ) );
   }
   while ( p->nextRow() ) {
      m_rowValues.push_back( rowValues_t::value_type() );
      for ( uint32 i = 0; i != nbrCol; ++i ) {
         m_rowValues.back().push_back( p->getColumn(i) );
      }
   }
   return m_rowValues.empty();
}

// Used to find the column name position by name
struct FindName : public binary_function< SQLDataContainer::NameAndIsNum, 
                                          MC2String, bool >
{
   bool operator()( const SQLDataContainer::NameAndIsNum& nameAndType, 
                    const MC2String& colName ) const {
      return nameAndType.first == colName;
   }
};

bool
SQLDataContainer::getCol( uint32 row, const MC2String& col, 
                          MC2String& value ) const
{
   rowName_t::const_iterator result;
   result = find_if( m_rowNames.begin(), m_rowNames.end(), 
                     bind2nd( FindName(), col ) );
   // Check if the column and row exists
   if ( result == m_rowNames.end() || row >= m_rowValues.size() ) {
      return false;
   } else {
      rowName_t::size_type colNbr = result - m_rowNames.begin();
      value =  m_rowValues[ row ][ colNbr ];
   }
   return true;
}

void
SQLDataContainer::load( const Packet* p, int& pos )
{
   // Clear data to make sure we start with empty vectors
   m_rowNames.clear();
   m_rowValues.clear();
   uint32 nbrCol = p->incReadLong( pos );
   for (uint32 i = 0; i < nbrCol; ++i ) {
      m_rowNames.push_back( NameAndIsNum( p->incReadString( pos ), false ) );
      m_rowNames.back().second = p->incReadByte( pos ) != 0;
   }
   uint32 nbrRows = p->incReadLong( pos );
   for (uint32 j = 0; j < nbrRows; ++j) {
      m_rowValues.push_back( rowValues_t::value_type() );
      for ( uint32 k = 0; k < nbrCol; ++k ) {
         m_rowValues.back().push_back( p->incReadString( pos ) );
      }
   }
}

void
SQLDataContainer::save( Packet* p, int& pos ) const
{
   p->updateSize( getSizeInPacket(), p->getBufSize() * 2 );
   p->incWriteLong( pos, m_rowNames.size() );
   rowName_t::const_iterator name = m_rowNames.begin();
   for ( ; name != m_rowNames.end(); ++name ) {
      p->incWriteString( pos, name->first );
      p->incWriteByte( pos, name->second );
   }
   p->incWriteLong( pos, m_rowValues.size() );
   rowValues_t::const_iterator valueRow = m_rowValues.begin();
   for ( ; valueRow != m_rowValues.end(); ++valueRow ) {
      rowValues_t::value_type::const_iterator valueCol = ( *valueRow ).begin();
      for ( ; valueCol != ( *valueRow ).end(); ++valueCol ) {
         p->incWriteString( pos, *valueCol );
      }
   }
   p->setLength( pos );
}

uint32
SQLDataContainer::getSizeInPacket() const
{
   // This is for the 2 longs we store in the packet
   uint32 bytes = 2 * 4;
   rowName_t::const_iterator name = m_rowNames.begin();
   for ( ; name != m_rowNames.end(); ++name ) {
       bytes += name->first.size() + 1 + 1;
   }
   rowValues_t::const_iterator valueRow = m_rowValues.begin();
   for ( ; valueRow !=  m_rowValues.end(); ++valueRow ) {
      rowValues_t::value_type::const_iterator valueCol = ( *valueRow ).begin();
      for ( ; valueCol != ( *valueRow ).end(); ++valueCol ) {
         bytes += (*valueCol).size() + 1;
      }
   }
   return bytes;
}

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WriteBlockableSQLDriver.h"

namespace {
const char* IllegalWriteMessage = 
   "Tried to do something other than select while"\
   " in write block mode!";
}

WriteBlockableSQLDriver
::WriteBlockableSQLDriver( SQLDriver* decorated, 
                           BlockPredicate* blockPredicate ) :
                           m_decorated( decorated ),
                           m_blockPredicate( blockPredicate ),
                           m_writeFailure( false ) {
}

WriteBlockableSQLDriver
::~WriteBlockableSQLDriver() {
   delete m_decorated;
   delete m_blockPredicate;
}
                            
bool WriteBlockableSQLDriver::connect() { 
   m_writeFailure = false;
   return m_decorated->connect(); 
}

bool WriteBlockableSQLDriver::ping() { 
   m_writeFailure = false;
   return m_decorated->ping(); 
}

bool WriteBlockableSQLDriver::setMaster(bool master) { 
   return m_decorated->setMaster( master ); 
}

const char* WriteBlockableSQLDriver::prepare(const char* query) {
   if ( m_writeFailure ) {
      return NULL;
   }

   if ( !m_blockPredicate->shouldBlock() || strncasecmp( query, "select", 6 ) == 0) {
      return m_decorated->prepare( query );
   } else {
      mc2log << info 
             << MC2String( "WriteBlockableSQLDriver::prepare " ) 
                + IllegalWriteMessage 
             << endl;
      m_writeFailure = true;
      return NULL;
   }
}

void WriteBlockableSQLDriver::freePrepared(const char* prepQuery) 
{ m_decorated->freePrepared( prepQuery ); }

const void* WriteBlockableSQLDriver::execute(const char* query) { 
   if ( m_writeFailure ) {
      return NULL;
   } else {
      return m_decorated->execute( query );
   }
}

void WriteBlockableSQLDriver::freeResult(const void* result) 
{ m_decorated->freeResult( result ); }

uint32 WriteBlockableSQLDriver::getNumColumns(const void* result) 
{ return m_decorated->getNumColumns( result ); }

const void* WriteBlockableSQLDriver::nextRow(const void* result) 
{ return m_decorated->nextRow( result ); }

void WriteBlockableSQLDriver::freeRow(const void* row) 
{ m_decorated->freeRow( row ); }

void WriteBlockableSQLDriver::getColumnNames( const void* result, 
                                              vector< MC2String >& colNames ) 
{ m_decorated->getColumnNames( result, colNames ); }

const char* WriteBlockableSQLDriver::getColumn(const void* result,
                              const void* row,
                              int colIndex) 
{ return m_decorated->getColumn( result, row, colIndex ); }

int WriteBlockableSQLDriver::getError(const void* result) { 
   if ( m_writeFailure ) {
      return 1;
   } else {
      return m_decorated->getError( result ); 
   }
}

const char* WriteBlockableSQLDriver::getErrorString(const void* result) { 
   if ( m_writeFailure ) {
      return IllegalWriteMessage;
   } else {
      return m_decorated->getErrorString( result ); 
   }
}

bool WriteBlockableSQLDriver::tableExists(const char* tableName) { 
   return m_decorated->tableExists( tableName ); 
}

bool WriteBlockableSQLDriver::beginTransaction() { 
   if ( m_writeFailure ) {
      return false;
   }
   return m_decorated->beginTransaction(); 
}

bool WriteBlockableSQLDriver::commitTransaction() { 
   if ( m_writeFailure ) {
      return false;
   }
   return m_decorated->commitTransaction(); 
}

bool WriteBlockableSQLDriver::rollbackTransaction() { 
   if ( m_writeFailure ) {
      return false;
   }
   return m_decorated->rollbackTransaction(); 
}

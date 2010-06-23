/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __WRITEBLOCKABLESQLDRIVER_H__
#define __WRITEBLOCKABLESQLDRIVER_H__

#include "SQLDriver.h"

/** A class which can be used to block SQL statements that modify the
 * database. Currently blocks everything except SELECT and causes an
 * error if something else is attempted.
 * 
 * This class follows the decorator pattern, create a SQLDriver as usual
 * and give it to this one, then use this one instead. The decorated
 * SQLDriver will be deleted by this class.
 */
class WriteBlockableSQLDriver : public SQLDriver {
public:
   /// Sub-class this class to determine when to block statements.
   class BlockPredicate {
   public:
      /// @return Wether the driver should block write statements.
      virtual bool shouldBlock() const = 0;
   };   
   
   /**
    * @param decorated        The driver to decorate, 
    *                         will be deleted in destructor.
    * @param blockPredicate   The predicate which decides when to block
    *                         statements, will be deleted in destructor.
    */
   WriteBlockableSQLDriver( SQLDriver* decorated, 
                            BlockPredicate* blockPredicate );
   
   virtual ~WriteBlockableSQLDriver();
      
   /* Overloads, see SQLDriver */
   
   virtual bool connect();
   virtual bool ping();
   virtual bool setMaster(bool master);
   virtual const char* prepare(const char* query);
   virtual void freePrepared(const char* prepQuery);
   virtual const void* execute(const char* query);
   virtual void freeResult(const void* result);
   virtual uint32 getNumColumns(const void* result);
   virtual const void* nextRow(const void* result);
   virtual void freeRow(const void* row);
   virtual void getColumnNames( const void* result, 
                                vector< MC2String >& colNames );
   virtual const char* getColumn(const void* result,
                                 const void* row,
                                 int colIndex);
   virtual int getError(const void* result);
   virtual const char* getErrorString(const void* result);
   virtual bool tableExists(const char* tableName);
   virtual bool beginTransaction();
   virtual bool commitTransaction();
   virtual bool rollbackTransaction();   
   
private:
   SQLDriver* m_decorated;
   BlockPredicate* m_blockPredicate;

   /** 
    *  This variable is set to true when an illegal write is attempted,
    *  after it has been set the driver will not work until a ping or
    *  connect has been done again. Hence doing an illegal write works
    *  as if the database closes the connection.
    */
   bool m_writeFailure;
};

#endif // __WRITEBLOCKABLESQLDRIVER_H__

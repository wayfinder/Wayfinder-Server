/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQL_TRANSACTION_H
#define SQL_TRANSACTION_H

#include "SQLConnection.h"
#include "SQLException.h"

namespace SQL {

/** Defines a transaction scope for SQL.
 * Works like an auto_ptr but for SQL transactions, or in other words: makes
 * SQL queries exception safe.
 * Usage:
 * \code
 * 
 * SQL::Transaction transaction( connection );
 * // .. do some queries ...
 * transaction.commit(); // transaction was successful.
 *
 * \endcode
 *
 */
class Transaction {
public:
   /// Starts a SQL transaction.
   explicit Transaction( SQLConnection& connection ) throw (SQL::Exception):
      m_connection( connection ),
      m_commited( false ) {
      if ( ! m_connection.beginTransaction() ) {
         throw SQL::Exception( "Transaction: Begin transaction failed!" );
      }
   }

   /// If the transaction was not commited during the lifetime of this
   /// instance, there will be a rollback here.
   ~Transaction() throw() {
      if ( ! m_commited ) {
         if ( ! m_connection.rollbackTransaction() ) {
            // So, what can we actually do about this?
            // I figure we can not do anything about it.
            mc2log << error << "SQL::Transaction: rollback failed!" << endl;
         }
      }
   }

   /// Commits the transaction and marks this instance as 
   /// a successful transaction. Will throw exception if commit failed.
   void commit() throw (SQL::Exception) {
      if ( ! m_commited ) {
         if ( ! m_connection.commitTransaction() ) {
            throw SQL::Exception( "Transaction:: Commit transaction failed!" );
         }
         m_commited = true;
      }
   }

private:
   SQLConnection& m_connection; ///< Connection to database.
   bool m_commited; ///< whether or not the transaction was commited.
};

} // SQL

#endif // SQL_TRANSACTION_H

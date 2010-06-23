/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OracleSQLDriver.h"
#include "config.h"
#include "StringUtility.h"

#ifdef USE_ORACLE
OracleSQLDriver::OracleSQLDriver(const char* host,
                         const char* database,
                         const char* user,
                         const char* password)
{
   m_host = StringUtility::newStrDup(host);
   m_database = StringUtility::newStrDup(database);
   m_user = StringUtility::newStrDup(user);
   m_password = StringUtility::newStrDup(password);

   // Initialize Oracle Call Interface (OCI)
   OCIInitialize(OCI_THREADED,
                 NULL, NULL, NULL, NULL);

   m_dbState = ORACLE_NOT_CONNECTED;
   m_transState = ORACLE_TRANSACTION_DONE;
}

bool
OracleSQLDriver::connect()
{
   mc2dbg8 << "OracleSQLDriver::connect(): setting up the environment handle" 
           << endl;
   // set up the environment handle
   if (OCIEnvInit(&m_env, OCI_DEFAULT, 0, NULL)) {
      mc2log << warn << "[OracleSQLDriver] Internal error, OCIEnvInit failed";
      return false;
   }

   mc2dbg8 << "OracleSQLDriver::connect(): setting up the error handle" << endl;
   // set up the error handle
   if (OCIHandleAlloc((dvoid*) m_env, (dvoid**) &m_err,
                      OCI_HTYPE_ERROR, 0, NULL)) {
      mc2log << warn << "[OracleSQLDriver] Internal error, OCIEnvInit failed";
      return false;
   }

   // logon to the database
   if (int status = OCILogon(m_env, m_err, &m_svc,
                             (OraText*)m_user, strlen(m_user),
                             (OraText*)m_password, strlen(m_password),
                             (OraText*)m_database, strlen(m_database))) {

      mc2log << warn << "[OracleSQLDriver] OCILogon failed - ";
      if (OCI_ERROR == status)
         mc2log << getErrorString(NULL);
      mc2log << endl;
      return false;
   } else 
      m_dbState = ORACLE_CONNECTED;

   if (ORACLE_CONNECTED == m_dbState) {
      m_transState = ORACLE_TRANSACTION_DONE;
      // time for some scary stuff needed for compatibility reasons...
      mc2dbg2 << "[OracleSQLDriver] Setting session parameters." << endl;
      char query[1024];
      const char* pquery;
      const void* result;
      // fix the date format
      strcpy(query, "ALTER SESSION SET NLS_DATE_FORMAT = \"YYYY-MM-DD\"");
      pquery = prepare(query);
      if (pquery != NULL) {

         result = execute(pquery);
         
         if (result == NULL)
            mc2log << warn << "[OracleSQLDriver] Internal error when executing"
                   << " SET NLS_DATE_FORMAT in connect()" << endl;
         else
            freeResult(result);

         freePrepared(pquery);
      } else
         mc2log << warn << "[OracleSQLDriver] Internal error when preparing"
                << " SET NLS_DATE_FORMAT in connect()" << endl;
   }

   return (ORACLE_CONNECTED == m_dbState);
}   

bool
OracleSQLDriver::ping()
{
   char query[1024];
   const char* pquery;
   const void* result;
   const void* row;
   bool connection = false;

   // There's no easy way to check the connection status, so we
   // perform a simple query, "SELECT sysdate FROM dual", which
   // supposedly is cheap.
   if (ORACLE_CONNECTED == m_dbState) {
      strcpy(query, "SELECT sysdate FROM dual");

      pquery = prepare(query);
      if (pquery != NULL) {
         result = execute(pquery);
         if (result != NULL) {
            row = nextRow(result);
            if (row != NULL) {
               freeRow((void*)row);
               connection = true;
            }
            freeResult(result);
         }
         freePrepared(pquery);
      }
      if (!connection) {
         mc2log << warn << "[OracleSQLDriver] ping(): there seems to be a "
                << "problem, will attempt to reconnect, the error from the "
                << "test query: " << getErrorString(NULL) << endl;
      }
   }

   if (!connection) {
      return connect();
   } else
      return true;
}

OracleSQLDriver::~OracleSQLDriver()
{
   if (m_dbState == ORACLE_CONNECTED) {
      OCILogoff(m_svc, m_err);
      OCIHandleFree((dvoid*) m_env, OCI_HTYPE_ENV);
   }
}

bool
OracleSQLDriver::setMaster(bool master)
{
   // not supported
   return false;
}

const char*
OracleSQLDriver::prepare(const char* query)
{
   OCIStmt* stmth = NULL;
   char* replaced;
   const char* theQuery;

   mc2dbg8 << "OracleSQLDriver::prepare(): top" << endl;
   if ( strncasecmp( "create table", query, 12 ) == 0 ||
        strncasecmp( "alter table", query, 11 ) == 0 )
   {
      mc2dbg8 << "OracleSQLDriver::prepare(): this is a create/alter "
              << "table stmt!" << endl;
      char* replaced = StringUtility::replaceString(query, "MC2BLOB", "LONG RAW");
      if (replaced != NULL) {
         theQuery = replaced;
      } else {
         theQuery = query;
      }
   } else {
      theQuery = query;
   }
   if (OCIHandleAlloc((dvoid*)m_env, (dvoid**) &stmth,
                      OCI_HTYPE_STMT, 0, NULL)) {
      return NULL;
   }
   mc2dbg8 << "OracleSQLDriver::prepare(): after OCIHandleAlloc()" << endl;

   if (OCIStmtPrepare(stmth, m_err, (OraText*)theQuery, strlen(theQuery),
                      OCI_NTV_SYNTAX,
                      OCI_DEFAULT)) {
      freePrepared((const char*)stmth);
      delete replaced;
      return NULL;
   }
   mc2dbg8 << "OracleSQLDriver::prepare(): after OCIStmtPrepare()" << endl;

   mc2dbg8 << "OracleSQLDriver::prepare(): returning pointer: " << stmth
           << endl;
   return (const char*)stmth;
}

void
OracleSQLDriver::freePrepared(const char* prepQuery)
{
   mc2dbg8 << "OracleSQLDriver::freePrepared(): top, pointer: "
           << (void*) prepQuery << endl;
   if (int status = OCIHandleFree((dvoid*) prepQuery, OCI_HTYPE_STMT)) {

      mc2log << warn << "[OracleSQLDriver] Internal error, "
                        "OCIHandleFree failed: ";
      if (OCI_ERROR == status)
         mc2log << getErrorString(NULL);
      mc2log << endl;
      return;
   }
}

const void*
OracleSQLDriver::execute(const char* query)
{
   OCIStmt* stmth = (OCIStmt*)query;
   OracleSQLResult* res;
   OCIParam*        param;
   ub4 colCount;
   ub2 size;
   ub4 iters;
   ub2 stmtType;

   res = new OracleSQLResult;

   res->stmth = stmth;

   mc2dbg8 << "OracleSQLDriver::execute(): top()" << endl;
   // this could be moved to prepare() instead for a small performance
   // increase when executing the same query multiple times
   if ((res->lastError = OCIAttrGet((dvoid*)stmth,
                                   OCI_HTYPE_STMT,
                                   &stmtType,
                                   NULL,
                                   OCI_ATTR_STMT_TYPE,
                                   m_err))) {
      mc2log << warn << "[OracleSQLDriver] Internal error, "
                        "OCIAttrGet failed: \"";
      if (OCI_ERROR == res->lastError)
         mc2log << getErrorString(res);
      mc2log << "\" in execute()" << endl;
      delete res;
      return NULL;
   }
   
   if (OCI_STMT_SELECT == stmtType) {
      iters = 0;
   } else {
      if (0 == stmtType) {
         delete res;
         return NULL;
      } else {
         iters = 1;
      }
   }

   mc2dbg8 << "OracleSQLDriver::execute(): after OCIAttrGet(), stmtType: "
           << stmtType << "iters: " << iters << endl;
   
   res->lastError = OCIStmtExecute(m_svc, stmth, m_err, iters,
                                   0, NULL, NULL, OCI_DEFAULT);

   if ((res->lastError != OCI_SUCCESS) &&
       (res->lastError != OCI_SUCCESS_WITH_INFO)) {
      delete res;
      return NULL;
   }

   mc2dbg8 << "OracleSQLDriver::execute(): getting colCount" << endl;

   if ((res->lastError = OCIAttrGet((dvoid*)stmth, OCI_HTYPE_STMT,
                                    &colCount, NULL,
                                    OCI_ATTR_PARAM_COUNT, m_err))) {
      mc2log << warn << "[OracleSQLDriver] Internal error in execute(), "
             << "OCIAttrGet (colCount) failed - ";
      if (OCI_ERROR == res->lastError)
         mc2log << getErrorString(res);
      mc2log << endl;
      delete res;
      return NULL;
   }

   mc2dbg8 << "OracleSQLDriver::execute(): after OCIAttrGet, colCount: "
           << colCount << endl;

   res->numColumns = colCount;

   res->sizes = new int[res->numColumns];

   /* reset */
   for (int i = 0; i < res->numColumns; i++) {
      res->sizes[i] = 0;
   }

   /* get sizes */
   for (int i = 0; i < res->numColumns; i++) {
      res->lastError = OCIParamGet(stmth, OCI_HTYPE_STMT, m_err,
                                   (dvoid**)&param, i+1);
      if (OCI_SUCCESS == res->lastError) {
         res->lastError = OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM,
                                     (dvoid*)&size, NULL,
                                     OCI_ATTR_DATA_SIZE, m_err);
      }

      if (OCI_SUCCESS == res->lastError) {
         mc2dbg8 << "OracleSQLDriver::execute(): i: " << i << " size: "
                 << size << endl;
         res->sizes[i] = size + 4; // +1 isn't enough, neither is 2/3
      } else {
         mc2log << warn << "[OracleSQLDriver] Internal error in execute(), "
                << "OCIParamGet/OCIAttrGet failed - ";
         if (OCI_ERROR == res->lastError)
            mc2log << getErrorString(res);
         mc2log << endl;
         delete[] res->sizes;
         delete res;
         return NULL;
      }
   }

   // check whether to autocommit
   if ((OCI_STMT_SELECT != stmtType ) && 
       (ORACLE_TRANSACTION_DONE == m_transState)) {
      mc2dbg8 << "OracleSQLDriver::execute(): doing implicit "
              << "commitTransaction()" << endl;
      if (! commitTransaction())
         mc2log << warn << "OracleSQLDriver::execute(): implicit "
                << "commitTransaction() failed!" << endl;
   }

   mc2dbg8 << "OracleSQLDriver::execute(): after OCIStmtExecute(), returning"
              " pointer: " << res << endl;

   return res;
}

void
OracleSQLDriver::freeResult(const void* result)
{
   OracleSQLResult* res = (OracleSQLResult*)result;

   mc2dbg8 << "OracleSQLDriver::freeResult(): top, pointer: " << result << endl;
   delete[] res->sizes;
   delete res;

   // the stmt handle is freed by freePrepared()
}

uint32
OracleSQLDriver::getNumColumns(const void* result)
{
   OracleSQLResult* res = (OracleSQLResult*)result;

   return res->numColumns;
}

const void*
OracleSQLDriver::nextRow(const void* result)
{
   OracleSQLRow*    row;
   OracleSQLResult* res = (OracleSQLResult*)result;
   OCIDefine*       def = NULL;

   mc2dbg8 << "OracleSQLDriver::nextRow(): top, numCols: " << res->numColumns
           << endl;

   row = new OracleSQLRow;

   row->res = res;

   row->columns = new char*[res->numColumns];
   row->ind = new sb2[res->numColumns];
   mc2dbg8 << "OracleSQLDriver::nextRow(): after row->columns[] and ind[] alloc"
           << endl;

   /* allocate columns[] and ind[] */
   for (int i = 0; i < res->numColumns; i++) {
      row->columns[i] = new char[res->sizes[i]];
      row->columns[i][0] = '\0';
      row->ind[i] = 0;
   }

   mc2dbg8 << "OracleSQLDriver::nextRow(): after alloc" << endl;

   /* define ("bind") to the columns */
   for (int i = 0; i < res->numColumns; i++) {
      // do we need an array of define handles instead of just "def" ?!
      // the API docs are somewhat unclear, and how about freeing them??
      // check examples...
      res->lastError = OCIDefineByPos(res->stmth, &def, m_err,
                                      i+1, row->columns[i], res->sizes[i],
                                      SQLT_STR,
                                      (dvoid*)&(row->ind[i]),
                                      NULL, NULL, OCI_DEFAULT);
      if ((OCI_SUCCESS != res->lastError) &&
          (OCI_SUCCESS_WITH_INFO != res->lastError)) {
         mc2log << warn << "[OracleSQLDriver] Internal error in nextRow(), "
                << "OCIDefineByPos failed (status: " << res->lastError
                << ", error: ";
         if (OCI_ERROR == res->lastError)
            mc2log << getErrorString(res);
         mc2log << endl;
         freeRow((void*)row);
         return NULL;
      }
   }
   mc2dbg8 << "OracleSQLDriver::nextRow(): after bind" << endl;

   /* get the next row */
   res->lastError = OCIStmtFetch(res->stmth, m_err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
   if ((OCI_SUCCESS_WITH_INFO == res->lastError) ||
       (OCI_SUCCESS == res->lastError)) {
      mc2dbg8 << "OracleSQLDriver::nextRow(): fetched OK" << endl;
      return row;
   } else {
      mc2dbg8 << "OracleSQLDriver::nextRow(): fetch error, lastError: "
              << res->lastError << endl;
      freeRow((void*)row);
      return NULL;
   }
}

void
OracleSQLDriver::freeRow(const void* row)
{
   OracleSQLRow* rowp = (OracleSQLRow*)row;

   mc2dbg8 << "OracleSQLDriver::freeRow(): top" << endl;
   rowp = (OracleSQLRow*)row;
   if (rowp->res->numColumns != 0) {
      for (int i = 0; i < rowp->res->numColumns; i++) {
         if (rowp->res->sizes[i] > 0)
            delete[] rowp->columns[i];
      }
   }
   mc2dbg8 << "OracleSQLDriver::freeRow(): columns[] deleted" << endl;

   delete[] rowp->columns;
   delete[] rowp->ind;
   delete rowp; 
   mc2dbg8 << "OracleSQLDriver::freeRow(): all gone!" << endl;
}

const char*
OracleSQLDriver::getColumn(const void* result, const void* row, int colIndex)
{
   OracleSQLRow*    rowp = (OracleSQLRow*)row;
   OracleSQLResult* res  = (OracleSQLResult*)result;

   if (colIndex >= res->numColumns)
      return NULL;

   return(rowp->columns[colIndex]);
}
      
int
OracleSQLDriver::getError(const void* result)
{
   OracleSQLResult* res = (OracleSQLResult*)result;

   if (NULL == res)
      return 0;

   mc2dbg8 << "OracleSQLDriver::getError(): res->lastError is: "
           << res->lastError << endl;

   if (OCI_ERROR == res->lastError)
      return 1;
   else
      return 0;
}

const char*
OracleSQLDriver::getErrorString(const void* result)
{
   sb4 errcode;
   OracleSQLResult* res = (OracleSQLResult*)result;
   
   if ((res == NULL) || (OCI_ERROR == res->lastError)) {
      OCIErrorGet((dvoid*) m_err, 1, NULL,
                  &errcode,
                  (OraText*)m_errbuf, sizeof(m_errbuf),
                  OCI_HTYPE_ERROR);
      return m_errbuf;
   } else
      return NULL;
}


bool
OracleSQLDriver::tableExists(const char* tableName)
{
   char query[1024];
   const char* pquery;
   const void* result;
   const void* row;
   bool exists = false;

   mc2dbg8 << "OracleSQLDriver::tableExists(): checking for: " << tableName
           << endl;
   // \todo
   // to gain speed; read table list when you connect and put
   // them in a hash_map (problems when creating new ones though)
   sprintf(query,
      "SELECT table_name FROM user_all_tables WHERE table_name = UPPER('%s')",
      tableName);

   pquery = prepare(query);
   if (pquery != NULL) {
      result = execute(pquery);
      if (result != NULL) {
         row = nextRow(result);
         if (row != NULL) {
            freeRow((void*)row);
            exists = true;
         }
         freeResult(result);
      } else {
         // error 
         mc2log << warn << "[OracleSQLDriver] Internal error in "
                "tableExists(), execute: ";
         mc2log << getErrorString(NULL) << endl;
      }
      freePrepared(pquery);
   } else {
      // error 
      mc2log << warn << "[OracleSQLDriver] Internal error in "
             "tableExists(), prepare: ";
      mc2log << getErrorString(NULL) << endl;
   }


   return exists;
}

bool
OracleSQLDriver::beginTransaction()
{
   // implicit begin with oracle
   m_transState = ORACLE_TRANSACTION_IN_PROGRESS;
   return true;
}

bool
OracleSQLDriver::commitTransaction()
{
   m_transState = ORACLE_TRANSACTION_DONE;
   if (OCITransCommit(m_svc, m_err, OCI_DEFAULT))
      return false;
   else
      return true;
}

bool
OracleSQLDriver::rollbackTransaction()
{
   m_transState = ORACLE_TRANSACTION_DONE;
   if (OCITransRollback(m_svc, m_err, OCI_DEFAULT))
      return false;
   else
      return true;
}
#endif // USE_ORACLE

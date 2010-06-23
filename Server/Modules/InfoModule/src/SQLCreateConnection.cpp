/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLConnection.h"

#include "MySQLDriver.h"
#include "MySQLReplDriver.h"
#include "CharEncSQLConn.h"
#include "PropertyHelper.h"

namespace SQL {

CharEncSQLConn* createInfoConnectionFromProperties() try {
   using PropertyHelper::get;
   MC2String driverName = get<MC2String>( "INFO_SQL_DRIVER" );
   MC2String sqlHost = get<MC2String>( "INFO_SQL_HOST" );
   MC2String sqlDB = get<MC2String>( "INFO_SQL_DATABASE" );
   MC2String sqlUser = get<MC2String>( "INFO_SQL_USER" );
   MC2String sqlPassword = get<MC2String>( "INFO_SQL_PASSWORD" );
   const char* sqlChEnc = Properties::getProperty( "INFO_SQL_CHARENCODING",
                                                   "ISO-8859-1" );
   
   auto_ptr<SQLDriver> driver;
   if ( driverName == "mysql" ) {
      driver.reset( new MySQLDriver( sqlHost.c_str(), sqlDB.c_str(),
                                     sqlUser.c_str(), sqlPassword.c_str() ) );
   } else if ( driverName == "mysqlrepl" ) {
      driver.
         reset( new MySQLReplDriver( sqlHost.c_str(), sqlDB.c_str(),
                                     sqlUser.c_str(), sqlPassword.c_str() ) );
   } else {
      return NULL;
   }

   const CharEncodingType::charEncodingType infoDBChEnc =
      CharEncoding::encStringToEncType( sqlChEnc );
   const CharEncodingType::charEncodingType mc2ChEnc = 
      CharEncoding::getMC2CharEncoding();

   auto_ptr<CharEncSQLConn>
      connection( new CharEncSQLConn( driver.release(),
                                      infoDBChEnc, mc2ChEnc ) );

   if ( ! connection->connect() ) {
      return NULL;
   }

   return connection.release();

} catch ( const PropertyException& e ) {
   mc2log << error << "Failed to create database connection. " 
          << e.what() << endl;
   return NULL;
}

}

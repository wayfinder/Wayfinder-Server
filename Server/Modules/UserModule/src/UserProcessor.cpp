/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "UserProcessor.h"

#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "CharEncSQLConn.h"
#include "Properties.h"
#include "MySQLDriver.h"
#include "MySQLReplDriver.h"
#include "OracleSQLDriver.h"
#include "PostgreSQLDriver.h"
#include "WFActivationPacket.h"
#include "CharEncoding.h"
#include "STLStringUtility.h"
#include "ISABThread.h"
#include "UserFavorites.h"
#include "MapUpdateDBaseObject.h"
#include "stat_inc.h"
#include "zlib.h"
#include "TransactionPacket.h"
#include "Cache.h"
#include "UserSessionCache.h"
#include "PoiReviewItem.h"
#include "PoiReviewEnums.h"
#include "DriverPref.h"
#include "BitUtility.h"
#include "File.h"
#include "SQLQueryHandler.h"
#include "LicenceToPacket.h"
#include "DeleteHelpers.h"
#include "WriteBlockableSQLDriver.h"
#include "LeaderStatus.h"
#include "NetUtility.h"
#include "ScopedArray.h"
#include "IDKeyToPacket.h"
#include "UserIDKey.h"

#include "UserPassword.h"

#ifdef PARALLEL_USERMODULE
#include "Memcached.h"
#endif

#include "SystemPackets.h"

#include <memory>
#include <boost/lexical_cast.hpp>

using namespace DoQuerySpace;

#ifndef RELEASE_VERSION
//const int32 UserProcessor::sessionValidTime = 31536000; // 365 days
const int32 UserProcessor::sessionValidTime = 604800; // 7 days
//const int32 UserProcessor::sessionValidTime = 2100; // 35 mins
#else
//const int32 UserProcessor::sessionValidTime = 31536000; // 365 days
const int32 UserProcessor::sessionValidTime = 604800; // 7 days
//const int32 UserProcessor::sessionValidTime = 2100; // 35 mins
#endif

const int32 UserProcessor::sessionUpdateTime = 300;  // 5 min

const int32 UserProcessor::sessionOldTime = 2592000; // 30 days

static const uint32 MAX_USER_AGE = 1800; // 30 MIN
namespace {

/**
 * @see UserPassword::compare
 */
bool checkPassword( const char* passwordFromDb,
                    const char* userPassword,
                    const char* uin ) {
   return 
      UserPassword::compare( passwordFromDb,
                             userPassword, uin ) ||
      // this is here for a short period, during conversion
      // of old passwords.
      StringUtility::strcasecmp( passwordFromDb, userPassword ) == 0;      
}
}
/**
 * A predicate to use with WriteBlockableSQLDriver to block writes when
 * this module isn't leader.
 */
class BlockWritesWhenNotLeader : public WriteBlockableSQLDriver::BlockPredicate {
public:
   BlockWritesWhenNotLeader( const LeaderStatus* leaderStatus ) 
   : m_leaderStatus( leaderStatus ) {
   }
   
   virtual bool shouldBlock() const {
      return !m_leaderStatus->isLeader();
   }
   
private:
   const LeaderStatus* m_leaderStatus;
};

#ifdef PARALLEL_USERMODULE
/**
 * The session cache consists of these items.
 */
struct SessionCacheItem {
   SessionCacheItem() 
   : UIN(0), lastAccessTime(0) {
   }
   
   SessionCacheItem( const MC2String& _key, 
                     uint32 _UIN, 
                     uint32 _lastAccessTime ) 
   : key( _key ), UIN( _UIN ), lastAccessTime( _lastAccessTime ) {
   }
   
   DataBuffer getDataBuffer() const {
      DataBuffer result( sizeof( UIN ) + 
                         sizeof( lastAccessTime ) + 
                         key.size()+1 );
      result.writeNextLong( UIN );
      result.writeNextLong( lastAccessTime );
      result.writeNextString( key.c_str() );
      return result;
   }
   
   void load(DataBuffer& buffer ) {
      UIN = buffer.readNextLong();
      lastAccessTime = buffer.readNextLong();
      key = buffer.readNextString();
   }
   
   MC2String key;
   uint32 UIN;
   uint32 lastAccessTime;
};

class SessionCache : public Memcached::SimpleNoThrowCache<SessionCacheItem> {
public:
   SessionCache( const char* hosts,
                 const char* prefix ) 
   : Memcached::SimpleNoThrowCache<SessionCacheItem>( hosts, prefix ) {
   }
};
#endif

UserProcessor::UserProcessor( MapSafeVector* loadedMaps, 
                              bool noSqlUpdate, 
                              const LeaderStatus* leaderStatus )
      : Processor(loadedMaps),
      	m_doneInitialDatabaseCheck( false ),
      	m_leaderStatus( leaderStatus ),
      	m_noSqlUpdate( noSqlUpdate )
{

   // get parameters using mc2.prop
   const char* driverName     = Properties::getProperty("USER_SQL_DRIVER");
   const char* sqlHost        = Properties::getProperty("USER_SQL_HOST");
   const char* sqlDB          = Properties::getProperty("USER_SQL_DATABASE");
   const char* sqlUser        = Properties::getProperty("USER_SQL_USER");
   const char* sqlPasswd      = Properties::getProperty("USER_SQL_PASSWORD");
   const char* tmpSqlChEnc    = Properties::getProperty("USER_SQL_CHARENCODING");
#ifdef PARALLEL_USERMODULE
   const char* memcachedHosts = Properties::getProperty("DEFAULT_MEMCACHED_SERVERS");
#endif
  
   MC2String sqlChEnc = "ISO-8859-1";
   if ( tmpSqlChEnc != NULL ){
      sqlChEnc = tmpSqlChEnc;
   }

   SQLDriver* driver;
   if (strcmp(driverName, "mysql") == 0) {
      driver = new MySQLDriver(sqlHost, sqlDB, sqlUser, sqlPasswd);
   } else if (strcmp(driverName, "mysqlrepl") == 0) {
      driver = new MySQLReplDriver(sqlHost, sqlDB, sqlUser, sqlPasswd);
   } else if(strcmp(driverName, "postgresql") == 0) {
      driver = new PostgreSQLDriver(sqlHost, sqlDB, sqlUser, sqlPasswd);
#ifdef USE_ORACLE
   } else if(strcmp(driverName, "oracle") == 0) {
      driver = new OracleSQLDriver(sqlHost, sqlDB, sqlUser, sqlPasswd);
#endif
   } else {
      mc2log << fatal << "[UP] Unknown database driver: \"" << driverName
             << "\" specified in mc2.prop!" << endl;
      exit(1);
   }
   
   // block all writes from this module as long as we're not leader
   BlockWritesWhenNotLeader* blockPredicate = 
      new BlockWritesWhenNotLeader( leaderStatus );
   driver = 
      new WriteBlockableSQLDriver( driver, blockPredicate ); 
   
   mc2log << info << "[UP] Connecting as " << sqlUser << " to " << sqlDB << "@"
          << sqlHost << " using " << "the " << driverName << " driver."
          << endl;

   CharEncodingType::charEncodingType userDBChEnc = 
      CharEncoding::encStringToEncType( sqlChEnc );
   CharEncodingType::charEncodingType mc2ChEnc = 
      CharEncoding::getMC2CharEncoding();
   mc2log << info << "[UP] DB char encoding:" 
          << CharEncoding::encTypeToEncString(userDBChEnc) << endl;
   mc2log << info << "[UP] MC2 char encoding:" 
          << CharEncoding::encTypeToEncString(mc2ChEnc) << endl;


   m_sqlConnection = 
      new CharEncSQLConn( driver, userDBChEnc, mc2ChEnc );
   if ( !m_sqlConnection->connect() ) {
      mc2log << fatal << "[UP] Couldn't connect to SQL database! "
             << "Aborting!" << endl;
      exit(1);
   }

   // Initialize to unique value to avoid using same random numbers again
   srand( TimeUtility::getRealTime() );

   // populate the table of tables
   m_numTables = initTables();

#ifdef PARALLEL_USERMODULE
   m_sessionCache = new SessionCache( memcachedHosts, "UM-S" );
   m_loginCache = new SessionCache( memcachedHosts, "UM-L" );
#else
   m_userSessionCache = new UserSessionCache();
   m_userLoginCache = new UserSessionCache();
#endif
   m_cache = new Cache( CacheElement::SERVER_TYPE );
   m_cache->setMaxSize( 10*1024*1024 ); // 10 MB
   m_cache->setMaxElements( 10000 );
   m_useUserCache = true;

   // Create the table data for ISABStoredUserData
   m_storedUserTableData.reset( new SQLTableData("ISABStoredUserData") );
   m_storedUserTableData->setColumnName( 
         SQLTableData::TableDataColumn( "userUIN", true, true ) );
   m_storedUserTableData->setColumnName( 
         SQLTableData::TableDataColumn( "dataKey", false, true ) );
   m_storedUserTableData->setColumnName( 
         SQLTableData::TableDataColumn( "dataValue", false, false ) );

   // Create the table data for ISABUserUser
   m_isabuseruser.reset( new SQLTableData("ISABUserUser") );
   setISABUserUserColumnName( "UIN", true, true );
   setISABUserUserColumnName( "logonID", false, false );
   setISABUserUserColumnName( "logonPasswd", false, false );
   setISABUserUserColumnName( "firstname", false, false );
   setISABUserUserColumnName( "initials", false, false );
   setISABUserUserColumnName( "lastname", false, false );
   setISABUserUserColumnName( "sessionID", false, false );
   setISABUserUserColumnName( "measurementSystem", true, false );
   setISABUserUserColumnName( "language", false, false );
   setISABUserUserColumnName( "lastdestmapID", true, false );
   setISABUserUserColumnName( "lastdestitemID", true, false );
   setISABUserUserColumnName( "lastdestOffset", true, false );
   setISABUserUserColumnName( "lastdestTime", true, false );
   setISABUserUserColumnName( "lastdestString", false, false );
   setISABUserUserColumnName( "lastorigmapID", true, false );
   setISABUserUserColumnName( "lastorigitemID", true, false );
   setISABUserUserColumnName( "lastorigOffset", true, false );
   setISABUserUserColumnName( "lastorigTime", true, false );
   setISABUserUserColumnName( "lastorigString", false, false );
   setISABUserUserColumnName( "searchType", true, false );
   setISABUserUserColumnName( "searchSubstring", true, false );
   setISABUserUserColumnName( "searchSorting", true, false );
   setISABUserUserColumnName( "searchObject", true, false );
   setISABUserUserColumnName( "routeCostA", true, false );
   setISABUserUserColumnName( "routeCostB", true, false );
   setISABUserUserColumnName( "routeCostC", true, false );
   setISABUserUserColumnName( "routeCostD", true, false );
   setISABUserUserColumnName( "routeType", true, false );
   setISABUserUserColumnName( "editMapRights", true, false );
   setISABUserUserColumnName( "editDelayRights", true, false );
   setISABUserUserColumnName( "editUserRights", true, false );
   setISABUserUserColumnName( "wapService", false, false );
   setISABUserUserColumnName( "htmlService", false, false );
   setISABUserUserColumnName( "operatorService", false, false );
   setISABUserUserColumnName( "nbrMunicipals", true, false );
   setISABUserUserColumnName( "municipals", false, false );
   setISABUserUserColumnName( "vehicleType", true, false );
   setISABUserUserColumnName( "birthDate", false, false );
   setISABUserUserColumnName( "routeImageType", true, false );
   setISABUserUserColumnName( "validDate", false, false );
   setISABUserUserColumnName( "gender", true, false );
   setISABUserUserColumnName( "smsService", true, false );
   setISABUserUserColumnName( "defaultCountry", false, false );
   setISABUserUserColumnName( "defaultMunicipal", false, false );
   setISABUserUserColumnName( "defaultCity", false, false );
   setISABUserUserColumnName( "searchDbMask", true, false );
   setISABUserUserColumnName( "navService", false, false );
   setISABUserUserColumnName( "operatorComment", false, false );
   setISABUserUserColumnName( "emailAddress", false, false );
   setISABUserUserColumnName( "address1", false, false );
   setISABUserUserColumnName( "address2", false, false );
   setISABUserUserColumnName( "address3", false, false );
   setISABUserUserColumnName( "address4", false, false );
   setISABUserUserColumnName( "address5", false, false );
   setISABUserUserColumnName( "routeTurnImageType", true, false );
   setISABUserUserColumnName( "externalXMLService", false, false );
   setISABUserUserColumnName( "transactionBased", true, false );
   setISABUserUserColumnName( "deviceChanges", true, false );
   setISABUserUserColumnName( "supportComment", false, false );
   setISABUserUserColumnName( "postalCity", false, false );
   setISABUserUserColumnName( "zipCode", false, false );
   setISABUserUserColumnName( "companyName", false, false );
   setISABUserUserColumnName( "companyReference", false, false );
   setISABUserUserColumnName( "companyVATNbr", false, false );
   setISABUserUserColumnName( "emailBounces", true, false );
   setISABUserUserColumnName( "addressBounces", true, false );
   setISABUserUserColumnName( "customerContactInfo", false, false );
   setISABUserUserColumnName( "creationTime", false, false );
}

UserProcessor::~UserProcessor()
{
   delete m_sqlConnection;
#ifdef PARALLEL_USERMODULE
   delete m_sessionCache;
   delete m_loginCache;
#else
   delete m_userSessionCache;
   delete m_userLoginCache;
#endif
   delete m_cache;
   for ( int32 i = 0 ; i < m_numTables ; ++i ) {
      delete [] m_tableExtraQueries[ i ];
      delete [] m_tableCreateQueries[ i ];
      delete [] m_tableNames[ i ];
   }
   delete [] m_tableExtraQueries;
   delete [] m_tableCreateQueries;
   delete [] m_tableNames;
}

Packet* UserProcessor::handleRequestPacket( const RequestPacket& p,
                                            char* packetInfo )
{
   ReplyPacket* reply = NULL;

   mc2dbg4 << "UP::handleRequest() going to handle packet" << endl;
   
   // Have we incorrectly received a request which will cause a db write?
   if ( !readOnly( &p ) && !m_leaderStatus->isLeader() ) {
      
      // This should mean we were leader when the UserReader enqueued
      // the request but have since been demoted to available,
      // send an acknowledge so the request will be re-sent.
      return new AcknowledgeRequestReplyPacket( &p, StringTable::OK, 0);
   }

   // this core dumps sometimes
   // DEBUG8(p->dump( true ));

   // this is probably a bit too expensive to do every time
   // should probably be moved to doQuery and only performed
   // if the query fails
   uint32 nbrReconnects = 0;
   while ( ! m_sqlConnection->ping() && nbrReconnects < 10 ) {
      mc2log << warn << "UP::handleRequest(): "
             << " connection to database lost, will try again.";
      try{
         JTCThread::sleep(2000); // ms
      } catch (const JTCInterruptedException &) {}
      nbrReconnects++;
   }
   if ( nbrReconnects >= 10) {
      mc2log << fatal << "UP::handleRequest(): no connection to database, "
             << "emergency stop!" << endl;
      exit(1);
   }

   uint32 uin = MAX_UINT32;
   if ( p.getLength() >= USER_REQUEST_HEADER_SIZE ) {
      uin = static_cast<const UserRequestPacket&> ( p ).getUIN();
   }
   if ( packetInfo != NULL && uin != MAX_UINT32 && uin != 0 ) {
      // Add a printout so that we know what user it is
      int res = sprintf( packetInfo, "[uin=%u]", uin );
      packetInfo += res;
      
   }

   DEBUG1( uint32 startTime = TimeUtility::getCurrentTime(); );

   switch(p.getSubType())
   {
      case Packet::PACKETTYPE_USERGETDATAREQUEST: {
         mc2dbg << "UP::handleRequest(): GetUserDataRequest" << endl;
         const GetUserDataRequestPacket* inpacket = 
            static_cast<const GetUserDataRequestPacket*>( &p );
         reply = handleGetUserDataRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERADDREQUEST: {
         mc2dbg << "UP::handleRequest(): AddUserRequest" << endl;
         const AddUserRequestPacket* inpacket =
            static_cast<const AddUserRequestPacket*>( &p );
         reply = handleAddUserRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERDELETEREQUEST: {
         mc2dbg << "UP::handleRequest(): DeleteUserRequest" << endl;
         const DeleteUserRequestPacket* inpacket =
            static_cast<const DeleteUserRequestPacket*>( &p );
         reply = handleDeleteUserRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERCHANGEREQUEST: {
         mc2dbg << "UP::handleRequest(): ChangeUserDataRequest" << endl;
         const ChangeUserDataRequestPacket* inpacket =
            static_cast<const ChangeUserDataRequestPacket*>( &p );
         reply = handleChangeUserDataRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERCHECKPASSWORDREQUEST: {
         mc2dbg << "UP::handleRequest(): CheckUserPasswordRequest" << endl;
         const CheckUserPasswordRequestPacket* inpacket =
            static_cast<const CheckUserPasswordRequestPacket*>( &p );
         reply = handleCheckUserPasswordRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERFINDREQUEST: {
         mc2dbg << "UP::handleRequest(): FindUserRequest" << endl;
         const FindUserRequestPacket* inpacket =
            static_cast<const FindUserRequestPacket*>( &p );
         reply = handleFindUserRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERFAVORITES_REQUEST: {
         mc2dbg << "UP::handleRequest(): UserFavoritesRequest" << endl;
         const UserFavoritesRequestPacket* inpacket =
            static_cast<const UserFavoritesRequestPacket*>( &p );
         reply = handleUserFavoritesRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_DEBITREQUEST : {
         mc2dbg << "UP::handleRequest(): DebitRequest" << endl;
         const DebitRequestPacket* inpacket =
            static_cast<const DebitRequestPacket*>( &p );
         reply = handleDebitRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_GETCELLULARPHONEMODELDATAREQUEST : {
         mc2dbg << "UP::handleRequest(): GetCellularPhoneModelDataRequest"
                << endl;
         const GetCellularPhoneModelDataRequestPacket* inpacket =
            static_cast<const GetCellularPhoneModelDataRequestPacket*>( &p );
         reply = handleGetCellularPhoneModelsRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_ADDCELLULARPHONEMODELREQUESTPACKET : {
         mc2dbg << "UP::handleRequest(): AddCellularPhoneModelRequest" << endl;
         const AddCellularPhoneModelRequestPacket* inpacket =
            static_cast<const AddCellularPhoneModelRequestPacket*>( &p );
         reply = handleAddCellularPhoneModelRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_CHANGEUSERPASSWORDREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): ChangeUserPasswordRequest" << endl;
         const ChangeUserPasswordRequestPacket* inpacket =
            static_cast<const ChangeUserPasswordRequestPacket*>( &p );
         reply = handleChangeUserPasswordRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_CHANGECELLULARPHONEMODELREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): ChangeCellularPhoneModelRequest"
                << endl;
         const ChangeCellularPhoneModelRequestPacket* inpacket =
            static_cast<const ChangeCellularPhoneModelRequestPacket*>( &p );
         reply = handleChangeCellularPhoneModelRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERVERIFYREQUEST: {
         mc2dbg << "UP::handleRequest(): VerifyUserRequest" << endl;
         const VerifyUserRequestPacket* inpacket =
            static_cast<const VerifyUserRequestPacket*>( &p );
         reply = handleVerifyUserRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERLOGOUTREQUEST: {
         mc2dbg << "UP::handleRequest(): LogoutUserRequest" << endl;
         const LogoutUserRequestPacket* inpacket =
            static_cast<const LogoutUserRequestPacket*>( &p );
         reply = handleLogoutUserRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USERSESSIONCLEANUPREQUEST: {
         mc2dbg << "UP::handleRequest(): SessionCleanUpRequest" << endl;
         const SessionCleanUpRequestPacket* inpacket =
            static_cast<const SessionCleanUpRequestPacket*>( &p );
         reply = handleSessionCleanUpRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_LISTDEBITREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): ListDebitRequest" << endl;
         const ListDebitRequestPacket* inpacket =
            static_cast<const ListDebitRequestPacket*>( &p );
         reply = handleListDebitRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_ADDUSERNAVDESTINATIONREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): AddUserNavDestinationRequest" << endl;
         const AddUserNavDestinationRequestPacket* inpacket =
            static_cast<const AddUserNavDestinationRequestPacket*>( &p );
         reply = handleAddUserNavDestinationRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_DELETEUSERNAVDESTINATIONREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): DeleteUserNavDestinationRequest"
                << endl;
         const DeleteUserNavDestinationRequestPacket* inpacket =
            static_cast<const DeleteUserNavDestinationRequestPacket*>( &p );
         reply = handleDeleteUserNavDestinationRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_CHANGEUSERNAVDESTINATIONREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): ChangeUserNavDestinationRequest"
                << endl;
         const ChangeUserNavDestinationRequestPacket* inpacket =
            static_cast<const ChangeUserNavDestinationRequestPacket*>( &p );
         reply = handleChangeUserNavDestinationRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_GETUSERNAVDESTINATIONREQUESTPACKET: {
         mc2dbg << "UP::handleRequest(): GetUserNavDestinationRequest" << endl;
         const GetUserNavDestinationRequestPacket* inpacket =
            static_cast<const GetUserNavDestinationRequestPacket*>( &p );
         reply = handleGetUserNavDestinationRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USER_AUTH_REQUEST: {
         mc2dbg << "UP::handleRequest(): AuthUserRequest" << endl;
         const AuthUserRequestPacket* inpacket =
            static_cast<const AuthUserRequestPacket*>( &p );
         reply = handleAuthUserRequestPacket( inpacket );
      }
      break;      
      case Packet::PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REQUEST: {
         mc2dbg << "UP::handleRequest(): RouteStorageAddRouteRequest" << endl;
         const RouteStorageAddRouteRequestPacket* inpacket =
            static_cast<const RouteStorageAddRouteRequestPacket*>( &p );
         reply = handleRouteStorageAddRouteRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REQUEST: {
         mc2dbg << "UP::handleRequest(): RouteStorageGetRouteRequest" << endl;
         const RouteStorageGetRouteRequestPacket* inpacket =
            static_cast<const RouteStorageGetRouteRequestPacket*>( &p );
         reply = handleRouteStorageGetRouteRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_USER_CREATE_SESSION_REQUEST: {
         mc2dbg << "UP::handleRequest(): CreateSessionRequest" << endl;
         const CreateSessionRequestPacket* inpacket = 
            static_cast<const  CreateSessionRequestPacket*> ( &p );
         reply = handleCreateSessionRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_ADDMAPUPDATE_REQUEST: {
         mc2dbg << "UP::handleRequest(): AddMapUpdateRequest" << endl;
         const MapUpdateRequestPacket* inpacket = 
            static_cast<const  MapUpdateRequestPacket*> ( &p );
         reply = handleMapUpdateRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REQUEST: {
         mc2dbg << "UP::handleRequest(): RouteStorageGetRouteRequest" 
                << endl;
         const RouteStorageChangeRouteRequestPacket* inpacket =
            static_cast<const RouteStorageChangeRouteRequestPacket*>( &p );
         reply = handleRouteStorageChangeRouteRequestPacket( inpacket );
      }
      break;
      case Packet::PACKETTYPE_LOADMAPREQUEST : {
         // No use of maps so we simply drop it
         mc2dbg << "UP::handleRequest(): LoadMapRequest, ignored " 
                << "(Not interested in any maps)!" << endl;
      }
      break;

      case Packet::PACKETTYPE_ADDUSERTRACK_REQUEST: {
         mc2dbg << "UP::handleRequest(): AddUserTrackRequest" << endl;
         reply = handleAddUserTrackRequestPacket( 
                        static_cast<const AddUserTrackRequestPacket*>( &p ));
      }
      break;

      case Packet::PACKETTYPE_GETUSERTRACK_REQUEST: {
         mc2dbg << "UP::handleRequest(): GetUserTrackRequest" << endl;
         reply = handleGetUserTrackRequestPacket( 
                        static_cast<const GetUserTrackRequestPacket*>( &p ));
      }
      break;

      case Packet::PACKETTYPE_TRANSACTION_REQUEST : {
         mc2dbg << "UP::handleRequest(): TransactionPacket" << endl;
         reply = handleTransactionRequestPacket( 
            static_cast<const TransactionRequestPacket*>( &p ) );
      }
      break;

      case Packet::PACKETTYPE_TRANSACTION_DAYS_REQUEST : {
         mc2dbg << "UP::handleRequest(): TransactionDaysPacket" << endl;
         reply = handleTransactionDaysRequestPacket( 
            static_cast<const TransactionDaysRequestPacket*>( &p ) );
      }
      break;

      case Packet::PACKETTYPE_WFACTIVATIONREQUEST : {
         // A clue
         mc2dbg << "UP::handleRequest(): " << p.getSubTypeAsString()
                << endl;         
         reply = handleWFActivationRequestPacket( 
            static_cast<const WFActivationRequestPacket*>( &p ) );
      }
      break;
      
      // Poi Reviews
      case Packet::PACKETTYPE_POIREVIEW_ADD_REQUEST :
         reply = handlePoiReviewAdd( p );
         break;
      case Packet::PACKETTYPE_POIREVIEW_DELETE_REQUEST :
         reply = handlePoiReviewDelete( p );
         break;
      case Packet::PACKETTYPE_POIREVIEW_LIST_REQUEST :
         reply = handlePoiReviewList( p );
         break;

      case Packet::PACKETTYPE_PERIODIC_REQUEST : 
         reply = handlePeriodic( p );
         return reply; // This to avoid too much printing about periodic
         break;

      // Stored user data
      case Packet::PACKETTYPE_GET_STORED_USER_DATA_REQUEST :
         reply = handleGetStoredUserDataRequestPacket( 
               static_cast< const GetStoredUserDataRequestPacket& >( p ) );
         break;
      case Packet::PACKETTYPE_SET_STORED_USER_DATA_REQUEST :
         reply = handleSetStoredUserDataRequestPacket( 
               static_cast< const SetStoredUserDataRequestPacket& >( p ) );
         break;

      // Licence to
      case Packet::PACKETTYPE_LICENCE_TO_REQUEST :
         reply = handleLicenceToRequestPacket( 
            static_cast< const LicenceToRequestPacket& >( p ) );
         break;

      // IDKey to
      case Packet::PACKETTYPE_ID_KEY_TO_REQUEST :
         reply = handleIDKeyToRequestPacket( 
            static_cast< const IDKeyToRequestPacket& >( p ) );
         break;

      default: {
         mc2log << error << "UP::handleRequest(): Don't know what to do with "
                << "Packet with unhandled subType " << (int)p.getSubType()
                << endl;
      }
      break;
   }
//   DEBUG8(if ( reply != NULL )  reply->dump( true));

   DEBUG1( uint32 endTime = TimeUtility::getCurrentTime();
                 mc2dbg << "UP::handleRequest(): Process time: " << 
                 (endTime - startTime) << "ms" << endl;);
   return reply;
}


int UserProcessor::getCurrentStatus()
{
   /// TODO: Check contact with SQL
   return 0;
}


#define addToRights( theRight ) /* theRight */ \
               rights.append( "," ); \
               rights.append( theRight ); \
               /* Time */ \
               rights.append( "(" ); \
               STLStringUtility::uint2str( prePaidMonths, rights ); \
               rights.append( "m," ); \
               /* Region */ \
               STLStringUtility::uint2str( \
                  atoi( sqlQuery->getColumn( 5 ) ), rights ); \
               rights.append( ")" );

ReplyPacket*
UserProcessor::handleWFActivationRequestPacket(
   const WFActivationRequestPacket* req)
{
   char query[ 4096 ];
   char activationCode[ 256 ];
   char tmpBuff[ 256 ];
   uint32 ip = req->getIP();
   const char* userAgent = req->getUserAgent();
   const char* userInput = req->getUserInput();
   uint32 UIN = req->getUIN();
   uint32 now = TimeUtility::getRealTime();
   StringUtility::SQLEscapeSecure( req->getActivationCode(), 
                                   activationCode, 255 );
   
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   WFResCodes::activation_res wfRes = WFResCodes::OK;
   MC2String rights;
   uint32 ownerUIN = 0;
   MC2String server;

   sprintf( query, "SELECT ActivationCode, "
            "MC2UIN, Rights, Server "
            "FROM WFActivationCodes "
            "WHERE ActivationCode = '%s'", activationCode );
   if ( doQuery( sqlQuery, query,
                 "UP::handleWFActivationRequestPacket(): "
                 "get activation" ) )
   {
      if ( sqlQuery->nextRow()) {
         // Get data

         // Owner UIN
         if ( sqlQuery->getColumn( 1 )[ 0 ] != '\0' ) {
            ownerUIN = strtoul( sqlQuery->getColumn( 1 ), NULL, 10 );
         }

         // Rights
         rights = sqlQuery->getColumn( 2 );

         // Server
         server = sqlQuery->getColumn( 3 );
         // Add empty first if this instance and then the other tokens
         // in server string.
         vector<MC2String> splitStr;
         StringUtility::tokenListToVector( splitStr, server, ' ',
                                           false, false );
         server = "";
         for ( uint32 i = 0 ; i < splitStr.size() ; ++i ) {
            if ( i == 0 ) {
               if ( splitStr[ 0 ] == Properties::getProperty( 
                       "WF_INSTANCE", "" ) )
               {
                  server = ""; 
               } else {
                  server = splitStr[ 0 ];
               }
            } else {
               server += " "; 
               server += splitStr[ i ];
            }
         }

         // Get or use
         if ( req->getWhatToDo() == WFActivationRequestPacket::GET_TIME ) {
            // DONE!
         } else if ( req->getWhatToDo() == WFActivationRequestPacket::USE )
         { // USE (time)
            const uint32 step = 5;
            MC2String intStr;
            sprintf( query, 
                     "UPDATE WFActivationCodes SET " );
            // MC2UIN
            intStr.clear();STLStringUtility::uint2str( UIN, intStr );
            strcat( query, "MC2UIN = " );
            StringUtility::SQLEscapeSecure( intStr.c_str(), tmpBuff, 255 );
            strcat( query, tmpBuff );
            // IP
            MC2String IPStr = NetUtility::ip2str( ip );
            strcat( query, ", IP = '" );
            strcat( query, IPStr.c_str() );
            strcat( query, "'" );
            // UseTime
            intStr.clear();STLStringUtility::int2str( int32(now), intStr );
            strcat( query, ", UseTime = " );
            strcat( query, intStr.c_str() );
            // UserAgent
            StringUtility::SQLEscapeSecure( userAgent, tmpBuff, 255 );
            strcat( query, ", UserAgent = '" );
            strcat( query, tmpBuff );
            strcat( query, "'" );
            // UserInput
            StringUtility::SQLEscapeSecure( userInput, tmpBuff, 255 );
            strcat( query, ", UserInput = '" );
            strcat( query, tmpBuff );
            strcat( query, "'" );
            
            // Where
            strcat( query, " WHERE ActivationCode = '" );
            StringUtility::SQLEscapeSecure( activationCode, tmpBuff, 255 );
            strcat( query, tmpBuff );
            strcat( query, "'" );
            mc2dbg8 << "UP::handleWFActivationRequestPacket query: " 
                    << query << endl;

            if ( !doQuery( sqlQuery, query,
                     "UP::handleWFActivationRequestPacket(): "
                     "use code" ) )
            {
               mc2log << warn << "UP::handleWFActivationRequestPacket "
                      << "use code failed, step: "<< step << ", account: " 
                      << req->getUserName() << " ip " 
                      << prettyPrintIP( ip ) << " userAgent " << userAgent
                      << " userInput " << userInput << endl;
               wfRes = WFResCodes::UNKNOWN_ERROR;
            }

         } else { // UPDATE
            // Set server
            StringUtility::SQLEscapeSecure( 
               req->getServer(), tmpBuff, 255 );
            sprintf( query, 
                     "UPDATE WFActivationCodes SET Server = '%s'"
                     ", UpdateTime = %d"
                     " WHERE ActivationCode = '%s'", 
                     tmpBuff, now, activationCode );
            if ( !doQuery( sqlQuery, query,
                     "UP::handleWFActivationRequestPacket(): "
                     "update code" ) )
            {
               mc2log << warn << "UP::handleWFActivationRequestPacket "
                      << "update code failed, server: "<< server << endl;
               wfRes = WFResCodes::UNKNOWN_ERROR;
            }
         } // End else UPDATE
      } else { // No such ActivationCode
         mc2log << warn << "UP::handleWFActivationRequestPacket() "
                << "Unknown activation code \"" 
                << req->getActivationCode() << "\"" << endl;
         wfRes = WFResCodes::WRONG_ACTIVATION_CODE;
      }
   } else {
      mc2log << warn << "UP::handleWFActivationRequestPacket() "
             << " select from WFActivationCodes failed!" << endl;
      wfRes = WFResCodes::UNKNOWN_ERROR;
   }

   delete sqlQuery;

   mc2dbg2 << "res " << int(wfRes) << " rights " << rights << " ownerUIN "
           << ownerUIN << " server " << server << endl;
   return new WFActivationReplyPacket( req, wfRes, rights.c_str(), 
                                       ownerUIN, server.c_str() );
}


#define checkurpsize( user ) \
            if ( (reply->getLength() + (user)->getSize()) >= \
                 reply->getBufSize() ) \
            { \
               mc2log << warn << "resize GetUserData Reply size " \
                      << reply->getBufSize() << " extra needed " \
                      << (user)->getSize() << endl; \
               reply->resize( reply->getBufSize() * 2 ); \
            }

UserReplyPacket*
UserProcessor::handleGetUserDataRequestPacket( 
  const GetUserDataRequestPacket* p ) 
{
   GetUserDataReplyPacket* reply = 
      new GetUserDataReplyPacket( p, MAX_PACKET_SIZE );

   mc2dbg4 << "UP::handleGetUserDataRequestPacket()" << endl;
   uint32 rstartTime = TimeUtility::getCurrentMicroTime();
   uint32 startTime = rstartTime;

   // Check cache
   bool ok = false;
   uint32 now = TimeUtility::getRealTime();
   if ( m_useUserCache && 
        (p->getElementType() & UserConstants::TYPE_ALL) ) 
   {
      // Only if TYPE_ALL
      startTime = TimeUtility::getCurrentMicroTime();
      UserItem* userEl = static_cast<UserItem*>(
         m_cache->find( p->getUIN(), CacheElement::USER_ELEMENT_TYPE ) );
      uint32 endTime = TimeUtility::getCurrentMicroTime();
      mc2dbg8 << "Time to find is " \
              << (endTime-startTime) << " us"  << endl;
      
      if ( userEl != NULL ) {
         if ( now - userEl->getTimeStamp() < MAX_USER_AGE && 
              !p->getNotFromCache() ) 
         {
            ok = true;
            mc2log << "UP::GetUserData Got " 
                   << userEl->getUser()->getLogonID() << "(" 
                   << userEl->getUIN() << ")"<< " from cache"
                   << endl;
            // Add all to reply
            startTime = TimeUtility::getCurrentMicroTime();
            UserUser* user = userEl->getUser();
            checkurpsize( user );
            reply->addUserElement( user );
            vector<UserElement*> elements;
            user->getAllElements( elements );
            for ( vector<UserElement*>::iterator it = elements.begin();
                  it != elements.end();
                  ++it ) {
               checkurpsize( *it );
               reply->addUserElement( *it );
            }
            
            endTime = TimeUtility::getCurrentMicroTime();
            mc2dbg8 << "Time to add data to reply is " \
                    << (endTime-startTime) << " us"  << endl;
            startTime = TimeUtility::getCurrentMicroTime();
            m_cache->releaseCacheElement( 
               userEl, false/*checkCapacity not needed*/ );
            endTime = TimeUtility::getCurrentMicroTime();
            mc2dbg8 << "Time for releaseCacheElement is " \
                    << (endTime-startTime) << " us"  << endl;
         } else { // Too stale, get new
            m_cache->remove( userEl ); // Remove stale from cache
            m_cache->releaseCacheElement( 
               userEl, false/*checkCapacity not needed*/ );// Find locked it
         }
      } // End if userEl != NULL
   } // End if TYPE_ALL

   if ( !ok ) {   
      // Collect all data in userItem
      UserItem* userItem = NULL;

      if ( p->getElementType() & UserConstants::TYPE_USER ) {
         UserUser* user = getUserUser( p->getUIN() );
         if ( user != NULL ) {
            checkurpsize( user );
            reply->addUserElement( user );
            if ( p->getElementType() & UserConstants::TYPE_ALL ) {
               // Only if TYPE_ALL
               userItem = new UserItem( user );
            } else {
               delete user;
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket "
                   << "getUserUser returned NULL!" << endl;
            reply->setStatus( StringTable::NOTOK );
         }
      }
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_CELLULAR) ) 
      {
         // Add Users Cellulars
         UserCellular** cellulars;
         int32 nbrCellulars;
         nbrCellulars = getUserCellular( p->getUIN(), cellulars );
         if ( nbrCellulars != -1 ) {
            for ( int32 i = 0 ; i < nbrCellulars ; i++ ) {
               checkurpsize( cellulars[ i ] );
               reply->addUserElement( cellulars[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( cellulars[ i ] );
               } else {
                  delete cellulars[ i ];
               }
            }
            delete [] cellulars;
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserCellular returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_BUDDY) ) 
      {
         // Add Users BuddyLists
         UserBuddyList** buddies;
         int32 nbrBuddyLists;
         nbrBuddyLists = getUserBuddyLists( p->getUIN(), buddies );
         if ( nbrBuddyLists != -1 ) {
            for ( int32 i = 0 ; i < nbrBuddyLists ; i++ ) {
               mc2dbg8 << "UP::handleGetUserDataRequestPacket(): buddies["
                       << i << "]" << endl;
               checkurpsize( buddies[ i ] );
               reply->addUserElement( buddies[ i ] ); 
               mc2dbg8 << "UP::handleGetUserDataRequestPacket(): buddies["
                       << i << "] before delete" << endl;
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( buddies[ i ] );
               } else {
                  delete buddies[ i ];
               }
               mc2dbg8 << "UP::handleGetUserDataRequestPacket(): buddies["
                       << i << "] after delete" << endl;
            }
            delete[] buddies;
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket(): "
                   << " getUserBuddyLists returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }
      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserNavigators..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_NAVIGATOR) ) 
      {
         // Add Users Navigators
         UserNavigator** navis;
         int32 nbrNavis;
         nbrNavis = getUserNavigators( p->getUIN(), navis );
         if ( nbrNavis != -1 ) {
            for ( int32 i = 0 ; i < nbrNavis ; i++ ) {
               checkurpsize( navis[ i ] );
               reply->addUserElement( navis[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( navis[ i ] );
               } else {
                  delete navis[ i ];
               }
            }
            delete[] navis;
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket "
                      "getUserNavigators returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }

      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserLicenceKeys..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_LICENCE_KEY) ) 
      {
         // Add Users Licence
         UserLicenceKeyPVect licencies;
         int32 nbrLicencies;
         nbrLicencies = getUserLicence( p->getUIN(), licencies );
         if ( nbrLicencies != -1 ) {
            for ( int32 i = 0 ; i < nbrLicencies  ; i++ ) {
               checkurpsize( licencies[ i ] );
               reply->addUserElement( licencies[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( licencies[ i ] );
               } else {
                  delete licencies[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
                      "getUserLicenceKey returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }

      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserRegionAccess..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_REGION_ACCESS) ) 
      {
         // Add Users Region access
         UserRegionAccess** accesses;
         int32 nbrAccesses;
         nbrAccesses = getUserRegionAccess( p->getUIN(), accesses );
         if ( nbrAccesses != -1 ) {
            for ( int32 i = 0 ; i < nbrAccesses  ; i++ ) {
               checkurpsize( accesses[ i ] );
               reply->addUserElement( accesses[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( accesses[ i ] );
               } else {
                  delete accesses[ i ];
               }
            }
            delete [] accesses;
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserRegionAccess returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserRights..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_RIGHT) ) 
      {
         // Add Users Rights
         vector<UserRight*> rights;
         int32 nbrRights = getUserRights( p->getUIN(), rights );
         if ( nbrRights != -1 ) {
            for ( int32 i = 0 ; i < nbrRights  ; i++ ) {
               checkurpsize( rights[ i ] );
               reply->addUserElement( rights[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( rights[ i ] );
               } else {
                  delete rights[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserRights returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserWayfinderSubscription..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & 
            UserConstants::TYPE_WAYFINDER_SUBSCRIPTION) ) 
      {
         // Add User Wayfinder Subscription
         vector<UserWayfinderSubscription*> subcr;
         int32 nbrSubsr;
         nbrSubsr = getUserWayfinderSubscription( p->getUIN(), subcr );
         if ( nbrSubsr != -1 ) {
            for ( int32 i = 0 ; i < nbrSubsr  ; i++ ) {
               checkurpsize( subcr[ i ] );
               reply->addUserElement( subcr[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( subcr[ i ] );
               } else {
                  delete subcr[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserWayfinderSubscription returned an error!"
                   << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserTokens..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_TOKEN) ) 
      {
         // Add Users Tokens
         vector<UserToken*> tokens;
         int32 nbr = getUserTokens( p->getUIN(), tokens );
         if ( nbr != -1 ) {
            for ( int32 i = 0 ; i < nbr  ; i++ ) {
               checkurpsize( tokens[ i ] );
               reply->addUserElement( tokens[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( tokens[ i ] );
               } else {
                  delete tokens[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserTokens returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserPINs..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_PIN) ) 
      {
         // Add Users PINs
         vector<UserPIN*> PINs;
         int32 nbr = getUserPINs( p->getUIN(), PINs );
         if ( nbr != -1 ) {
            for ( int32 i = 0 ; i < nbr  ; i++ ) {
               checkurpsize( PINs[ i ] );
               reply->addUserElement( PINs[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( PINs[ i ] );
               } else {
                  delete PINs[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserPINs returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserIDKeys..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_ID_KEY) ) 
      {
         // Add User's UserIDKeys
         vector<UserIDKey*> els;
         int32 nbr = getUserIDKeys( p->getUIN(), els );
         if ( nbr != -1 ) {
            for ( int32 i = 0 ; i < nbr  ; i++ ) {
               checkurpsize( els[ i ] );
               reply->addUserElement( els[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( els[ i ] );
               } else {
                  delete els[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserIDKeys returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      mc2dbg8 << "UP::handleGetUserDataRequestPacket(): going to get "
              << "UserLastClients..." << endl;
      if ( reply->getStatus() == StringTable::OK &&
           (p->getElementType() & UserConstants::TYPE_LAST_CLIENT) ) 
      {
         // Add User's UserLastClients
         vector<UserLastClient*> els;
         int32 nbr = getUserLastClients( p->getUIN(), els );
         if ( nbr != -1 ) {
            for ( int32 i = 0 ; i < nbr  ; i++ ) {
               checkurpsize( els[ i ] );
               reply->addUserElement( els[ i ] ); 
               if ( userItem != NULL ) {
                  userItem->getUser()->addElement( els[ i ] );
               } else {
                  delete els[ i ];
               }
            }
            reply->setStatus( StringTable::OK );
         } else {
            mc2log << error << "UP::handleGetUserDataRequestPacket() "
               "getUserLastClients returned an error!" << endl;
            reply->setStatus( StringTable::NOTOK );  
         }
      }


      if ( m_useUserCache && userItem != NULL ) {
         // Add to cache
         m_cache->add( userItem );
      } else {
         delete userItem;
      }
   } // End if !ok

   return reply;
}

#define HANDLEADDUSERREQUESTPACKET_ABORT_STATUS( a ) \
      if ( !m_sqlConnection->rollbackTransaction()) { \
         mc2log << error << "UP::handleAddUserRequestPacket " \
                " rollbackTransaction() failed!" << endl; \
      } \
      delete user; \
      delete sqlQuery; \
      reply = new AddUserReplyPacket( p, 0 ); \
      reply->setStatus( a ); \
      return reply;   

#define HANDLEADDUSERREQUESTPACKET_ABORT \
   HANDLEADDUSERREQUESTPACKET_ABORT_STATUS( StringTable::NOTOK )

UserReplyPacket*
UserProcessor::handleAddUserRequestPacket( const
   AddUserRequestPacket* p ) 
{
   AddUserReplyPacket* reply = NULL;
   char query[4096];
   int pos;
   CharEncSQLQuery* sqlQuery = NULL;
   UserUser* user = NULL;

   // Extract user
   if ( p->getNbrElements() < 1 ) {
      mc2log << warn << "UP::handleAddUserRequestPacket(): "
             << " No elements to make new user of!" << endl;
      HANDLEADDUSERREQUESTPACKET_ABORT;
   }

   UserElement* elem = NULL;

   pos = p->getFirstElementPosition();
   // Get UserUser first as it is first
   elem = p->getNextElement( pos );
   user = dynamic_cast<UserUser*> ( elem );
   if ( user == NULL ) {
      mc2log << error << "UP::handleAddUserRequestPacket(): "
             << " Not UserUser element first!" << endl;
      HANDLEADDUSERREQUESTPACKET_ABORT;
   }
   
   for ( uint32 i = 1 ; i < p->getNbrElements() ; i++ ) {
      elem = p->getNextElement( pos );
      if ( (elem == NULL) || (!elem->isOk()) ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Element extraction failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
      user->addElement( elem );
   }

   sqlQuery = m_sqlConnection->newQuery();
   // Add user to database
   if ( !m_sqlConnection->beginTransaction()) {
      mc2log << error << "UP::handleAddUserRequestPacket "
             << " beginTransaction() failed!" << endl;
      HANDLEADDUSERREQUESTPACKET_ABORT;
   } 

   // Add user if unique logonID
   sprintf(query, "SELECT logonID FROM ISABUserUser WHERE logonID = '%s'",
           user->getLogonID() );
   if ( ! doQuery(sqlQuery, query, "UP::handleAddUserRequestPacket() find") ) {
      HANDLEADDUSERREQUESTPACKET_ABORT;
   }
   
   if ( sqlQuery->nextRow() && sqlQuery->nextRow()) {
      mc2log << error << "UP::handleAddUserRequestPacket "
             << " Check of logonID not unique! " << query << endl;
      if ( !m_sqlConnection->rollbackTransaction()) { 
         mc2log << error << "UP::handleAddUserRequestPacket " 
                << " rollbackTransaction() failed!" << endl; 
      } 
      HANDLEADDUSERREQUESTPACKET_ABORT_STATUS( StringTable::NOT_UNIQUE ); 
   }
 
   uint32 UINInstance = Properties::getUint32Property("UIN_INSTANCE");
   uint32 UIN = 0;
   if (UINInstance == MAX_UINT32)
      UIN = getNewUniqueID("ISABUserUser", "UIN");
   else 
      UIN = getNewUniqueID("ISABUserUser", "UIN", UINInstance);
   
   if ( UIN == 0 ) {
      mc2log << error << "UP::handleAddUserRequestPacket(): "
             << " getNewUniqueID() returned 0, aborting!";
      HANDLEADDUSERREQUESTPACKET_ABORT;
   }

   user->setUIN( UIN );
   // Actual add!
   // Add UserUser first
   strcpy( query, "INSERT INTO ISABUserUser VALUES ( ");
   addUserUserData( query + strlen(query), user, UIN, p->getPassword() );
   strcat( query, ", NOW())" );  // NOW() works in at least MySQL and 
                                 // PostgreSQL, should check Oracle!

   if ( ! doQuery(sqlQuery, query, "UP::handleAddUserRequestPacket() add") ) {
      mc2log << error << "UP::handleAddUserRequestPacket(): "
             << " Add of UserUser failed, aborting!" << endl;
      HANDLEADDUSERREQUESTPACKET_ABORT;
   }

   // Then its elements
   // All types of elements

   // Add cellulars
   UserCellular* cellular = NULL;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_CELLULAR ); 
         i++ ) 
   {
      cellular = static_cast<UserCellular*> ( 
         user->getElementOfType( i, UserConstants::TYPE_CELLULAR ) );
      uint32 ID = getNewCellularID();
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " getNewCellularID() returned 0, aborting!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserCellular VALUES ( ");
      addUserCellularData( query + strlen(query), ID, UIN, cellular );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add cell") ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserCellular failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }

   // Add buddylists
   UserBuddyList* buddy = NULL;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_BUDDY ); 
         i++ ) 
   {
      buddy = static_cast<UserBuddyList*> ( 
         user->getElementOfType( i, UserConstants::TYPE_BUDDY ) );
      uint32 ID = getNewBuddyListID();
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " getNewBuddyListID() returned 0, aborting!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserBuddyList VALUES ( " );
      addUserBuddyListData( query + strlen(query), buddy );
      strcat( query, " ) ;" );
      
      if ( ! doQuery(sqlQuery, query, 
                 "UP::handleAddUserRequestPacket() add buddy") ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserBuddyList failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
      
   }

   // Add Navigators
   UserNavigator* nav = NULL;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_NAVIGATOR ); 
         i++ ) 
   {
      nav = static_cast<UserNavigator*> ( 
         user->getElementOfType( i, UserConstants::TYPE_NAVIGATOR ) );

      if ( nav->getDB()->insert( sqlQuery ) && doQuery(sqlQuery, NULL,
                                        "UP::handleAddUserRequestPacket()" )) {
         mc2log << error << "UP::handleAddUserRequestPacket(): Add of "
                << "UserBuddyList failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }

   UserLicenceKey* licence = NULL;
   bool changedLicence = false;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ); 
         i++ ) 
   {
      licence = static_cast<UserLicenceKey*> ( 
         user->getElementOfType( i, UserConstants::TYPE_LICENCE_KEY ) );
      // Check if licence already exists 
      vector<uint32> users;
      vector<MC2String> logonIDs;
      uint32 nbrExistingUINs = getUINsFromUserLicenceKey( 
         licence, users, logonIDs, sqlQuery );
      if ( nbrExistingUINs != 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Existing UserLicenceKey exists! For " 
                << licence->getKeyType() << ":" << licence->getLicenceKeyStr()
                << " product " << licence->getProduct() << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
      // Add it if not aborted above
      if ( !addUserLicence( UIN, p->getChangerUIN(), licence, sqlQuery ) ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserLicenceKey failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
      changedLicence = true;
   }

   UserRegionAccess* access = NULL;
   bool changedRegion = false;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_REGION_ACCESS ); 
         i++ ) 
   {
      access = static_cast<UserRegionAccess*> ( 
         user->getElementOfType( i, UserConstants::TYPE_REGION_ACCESS ) );
      uint32 ID = getNewRegionAccessID();
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewRegionAccessID() returned 0, aborting!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserRegionAccess VALUES ( ");
      addUserRegionAccessData( query + strlen(query), ID, UIN, access );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add region access") )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserRegionAccess failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
      changedRegion = true;
   }
   if ( changedRegion ) {
      // Add change into changelogtable
      // Add what there is now
      addUserRegionTolog( user, sqlQuery );      
   }

   bool changedRight = false;
   UserRight* right = NULL;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
   {
      right = static_cast<UserRight*> ( 
         user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
      uint32 ID = getNewUniqueID( "ISABUserRight", "id" );
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewUniqueID(ISABUserRight) returned 0, aborting!" 
                << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserRight VALUES ( ");
      addUserRightData( query + strlen(query), ID, UIN, right );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add right") )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserRight failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }
   if ( changedRight ) {
      // Add change into changelogtable
      // Add what there is now
      addUserRightTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   UserWayfinderSubscription* wfsubscr = NULL;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( 
            UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) ; 
         i++ ) 
   {
      wfsubscr = static_cast<UserWayfinderSubscription*> ( 
         user->getElementOfType( 
            i, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) );
      uint32 ID = getNewUniqueID( "ISABUserWayfinderSubscription", "id" );
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewUniqueID(WayfinderSubscription) returned 0, "
                << "aborting!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      
      strcpy( query, 
              "INSERT INTO ISABUserWayfinderSubscription VALUES ( " );
      addUserWayfinderSubscriptionData( query + strlen(query), ID, UIN, 
                                        wfsubscr );
      strcat( query, " )" );
   
      if ( ! doQuery( sqlQuery, query, 
                      "UP::handleAddUserRequestPacket() add user "
                      "wayfinder subscription" ) )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserWayfinderSubscription failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }

   bool changedToken = false;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ ) 
   {
      UserToken* token = static_cast<UserToken*> ( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      uint32 ID = getNewUniqueID( "ISABUserToken", "id" );
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewUniqueID(ISABUserToken) returned 0, aborting!" 
                << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserToken VALUES ( ");
      addUserTokenData( query + strlen(query), ID, UIN, token );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add token") )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserToken failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }
   if ( changedToken ) {
      // Add change into changelogtable
      // Add what there is now
      addUserTokenTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   bool changedPIN = false;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_PIN ) ; i++ ) 
   {
      UserPIN* PIN = static_cast<UserPIN*> ( 
         user->getElementOfType( i, UserConstants::TYPE_PIN ) );
      uint32 ID = getNewUniqueID( "ISABUserPIN", "id" );
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewUniqueID(ISABUserPIN) returned 0, aborting!" 
                << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserPIN VALUES ( ");
      addUserPINData( query + strlen(query), ID, UIN, PIN );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add PIN") )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserPIN failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }
   if ( changedPIN ) {
      // Add change into changelogtable
      // Add what there is now
      addUserPINTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   // Check if unique UserIDKeys of type account
   UserUser::constUserElRange_t els = user->getElementRange( 
      UserConstants::TYPE_ID_KEY );
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it )
   {
      const UserIDKey* el = static_cast< const UserIDKey* > ( *it );
      if ( el->getIDType() == UserIDKey::account_id_key ) {
         sprintf( query, "SELECT id FROM ISABUserIDKey WHERE %s = '%s'"
                  " AND %s = %d",
                  UserConstants::UserIDKeyFieldName[ 
                     UserConstants::USER_ID_KEY_KEY ], 
                  StringUtility::SQLEscapeSecure( el->getIDKey() ).c_str(),
                  UserConstants::UserIDKeyFieldName[ 
                     UserConstants::USER_ID_KEY_TYPE ],
                  UserIDKey::account_id_key );
         if ( ! doQuery( sqlQuery, query, "UP::handleAddUserRequestPacket() "
                         "find UserIDKey" ) ) {
            HANDLEADDUSERREQUESTPACKET_ABORT;
         }

         if ( sqlQuery->nextRow() ) {
            mc2log << error << "UP::handleAddUserRequestPacket "
                   << " Check of UserIDKeys, type account, not unique! " 
                   << query << endl;
            if ( !m_sqlConnection->rollbackTransaction()) { 
               mc2log << error << "UP::handleAddUserRequestPacket " 
                      << " rollbackTransaction() failed! For UserIDKey." 
                      << endl; 
            } 
            HANDLEADDUSERREQUESTPACKET_ABORT_STATUS( 
               StringTable::NOT_ALLOWED ); 
         } // End if have a IDKey with same value already in db
      } // End if the IDKey is of type account
   } // End for all UserIDKeys
   bool changedidKey = false;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_ID_KEY ) ; i++ ) 
   {
      UserIDKey* idKey = static_cast<UserIDKey*> ( 
         user->getElementOfType( i, UserConstants::TYPE_ID_KEY ) );
      uint32 ID = getNewUniqueID( "ISABUserIDKey", "id" );
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewUniqueID(ISABUserIDKey) returned 0, aborting!" 
                << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserIDKey VALUES ( ");
      addUserIDKeyData( query + strlen(query), ID, UIN, idKey );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add idKey") )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserIDKey failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }
   // Add initial UserIDKeys to changelog
   if ( changedidKey ) {
      // Add change into changelogtable
      // Add what there is now
      addUserIDKeyTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   bool changedlastClient = false;
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_LAST_CLIENT ) ; i++ ) 
   {
      UserLastClient* lastClient = 
         static_cast<UserLastClient*> ( 
            user->getElementOfType( i, UserConstants::TYPE_LAST_CLIENT ) );
      uint32 ID = getNewUniqueID( "ISABUserLastClient", "id" );
      
      if ( ID == 0 ) {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << "getNewUniqueID(ISABUserLastClient) returned 0, "
                << "aborting!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }

      strcpy( query, "INSERT INTO ISABUserLastClient VALUES ( ");
      addUserLastClientData( query + strlen(query), ID, UIN, lastClient );
      strcat( query, " )" );
   
      if ( ! doQuery(sqlQuery, query, 
                    "UP::handleAddUserRequestPacket() add lastClient") )
      {
         mc2log << error << "UP::handleAddUserRequestPacket(): "
                << " Add of UserLastClient failed!" << endl;
         HANDLEADDUSERREQUESTPACKET_ABORT;
      }
   }
   if ( changedlastClient ) {
      // Add change into changelogtable
      // Add what there is now
      addUserLastClientTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   
   if ( !m_sqlConnection->commitTransaction()) {
      mc2log << error << "UP::handleAddUserRequestPacket(): "
             << " commitTransaction() failed!" << endl;
      HANDLEADDUSERREQUESTPACKET_ABORT;
   }
      
   reply = new AddUserReplyPacket( p, UIN );
   reply->setStatus( StringTable::OK );

   delete sqlQuery;
   delete user;
   return reply;
}


#define HANDLEDELETEUSERREQUESTPACKET_ABORT \
      if ( !m_sqlConnection->rollbackTransaction()) { \
         mc2log << error << "UP::handleDeleteUserRequestPacket " \
                << " rollbackTransaction() failed!" << endl; \
      } \
      delete sqlQuery; \
      reply = new DeleteUserReplyPacket( p, false ); \
      reply->setStatus( StringTable::NOTOK ); \
      return reply;   
  
UserReplyPacket*
UserProcessor::handleDeleteUserRequestPacket( const
   DeleteUserRequestPacket* p ) 
{
   DeleteUserReplyPacket* reply = NULL;
   uint32 UIN = p->getUIN();
   char query[1024];
   
   if ( UIN == 0 ) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << "UIN was 0, aborting!" << endl;
      reply = new DeleteUserReplyPacket( p, false );
      reply->setStatus( StringTable::NOTFOUND );
      return reply; 
   }

   // Find user and delete
   if ( !m_sqlConnection->beginTransaction() ) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << "beginTransaction() failed!" << endl;
      reply = new DeleteUserReplyPacket( p, false );
      reply->setStatus( StringTable::NOTOK );
      return reply;
   }

   CharEncSQLQuery *sqlQuery = m_sqlConnection->newQuery();
   
   // Remove the user from the login cache
   MC2String logonID;
   if ( getLogonIDForUIN( sqlQuery, UIN, logonID ) ) {
      MC2String logonIDLower = StringUtility::copyLower( logonID ); 
#ifdef PARALLEL_USERMODULE
      m_loginCache->remove( logonIDLower.c_str() );
#else
      m_userLoginCache->removeSession( logonIDLower.c_str() );
#endif
   }
   else {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << "failed to get logon ID for UIN = " << UIN
             << " dropping login cache!" << endl;
#ifdef PARALLEL_USERMODULE
      m_loginCache->flush();
#else
      delete m_userLoginCache;
      m_userLoginCache = new UserSessionCache();
#endif
   }
   
   sprintf( query, "DELETE FROM ISABUserUser WHERE UIN = %u", UIN);

   if ( !doQuery(sqlQuery, query,
              "UP::handleDeleteUserRequestPacket(): UserUser")) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserUser failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }   

   // Delete UserCellulars too
   sprintf( query, "DELETE FROM ISABUserCellular WHERE userUIN = %u", UIN);
   if ( !doQuery(sqlQuery, query,
             "UP::handleDeleteUserRequestPacket(): UserCellular")) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserCellular failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserBuddyLists too
   sprintf( query, "DELETE FROM ISABUserBuddyList WHERE userUIN = '%u'", UIN);
   if ( !doQuery(sqlQuery, query,
             "UP::handleDeleteUserRequestPacket(): UserBuddy")) {
      mc2log << error << "UP::handleDeleteUserRequestPacket() "
             << " Delete UserBuddyList failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserLicenceKeys too
   sprintf( query, "DELETE FROM ISABUserLicence WHERE userUIN = %u", UIN);
   if ( !doQuery(sqlQuery, query,
                 "UP::handleDeleteUserRequestPacket(): UserLicenceKey")) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserLicenceKey failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }
   // Delete ISABUserLicenceKey too
   sprintf( query, "DELETE FROM ISABUserLicenceKey WHERE userUIN = %u", UIN);
   if ( !doQuery(sqlQuery, query,
                 "UP::handleDeleteUserRequestPacket(): ISABLicenceKeys")) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete ISABUserLicenceKey failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete all UserRegionAccess
   sprintf( query, "DELETE FROM ISABUserRegionAccess WHERE userUIN = %u",
            UIN);
   if ( !doQuery(sqlQuery, query,
                 "UP::handleDeleteUserRequestPacket(): UserRegionAccess"))
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserRegionAccess failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete all UserRights
   sprintf( query, "DELETE FROM ISABUserRight WHERE userUIN = %u",UIN);
   if ( !doQuery(sqlQuery, query,
                 "UP::handleDeleteUserRequestPacket(): UserRight"))
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserRight failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserWayfinderSubscription
   sprintf( query, "DELETE FROM ISABUserWayfinderSubscription "
            "WHERE userUIN = %u",
            UIN );
   if ( !doQuery(sqlQuery, query,
                 "UP::handleDeleteUserRequestPacket(): "
                 "UserWayfinderSubscription" ) )
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserWayfinderSubscription failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserToken
   sprintf( query, "DELETE FROM ISABUserToken WHERE userUIN = %u", UIN );
   if ( !doQuery( sqlQuery, query,
                  "UP::handleDeleteUserRequestPacket(): UserToken" ) )
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserToken failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserPIN
   sprintf( query, "DELETE FROM ISABUserPIN WHERE userUIN = %u", UIN );
   if ( !doQuery( sqlQuery, query,
                  "UP::handleDeleteUserRequestPacket(): UserPIN" ) )
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserPIN failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserIDKey
   sprintf( query, "DELETE FROM ISABUserIDKey WHERE userUIN = %u", UIN );
   if ( !doQuery( sqlQuery, query,
                  "UP::handleDeleteUserRequestPacket(): UserIDKey" ) )
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserIDKey failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   // Delete UserLastClient
   sprintf( query, "DELETE FROM ISABUserLastClient WHERE userUIN = %u", 
            UIN );
   if ( !doQuery( sqlQuery, query,
                  "UP::handleDeleteUserRequestPacket(): UserLastClient" ) )
   {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " Delete UserLastClient failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   if ( !m_sqlConnection->commitTransaction() ) {
      mc2log << error << "UP::handleDeleteUserRequestPacket(): "
             << " commitTransaction() failed!" << endl;
      HANDLEDELETEUSERREQUESTPACKET_ABORT;
   }

   reply = new DeleteUserReplyPacket( p, true );
   reply->setStatus( StringTable::OK );

   return reply;
}



void 
UserProcessor::addUserChangeToChangelog( UserUser* user, 
                                         uint32 changerUIN,
                                         CharEncSQLQuery* sqlQuery ) 
{
   char query[ 32768 ];
   char userData[ 8193 ];
   char wftStr[100];
   uint32 now = TimeUtility::getRealTime();

   // Get wft
   sprintf( query, "SELECT type FROM ISABUserWayfinderSubscription "
            "where userUIN = %u", user->getUIN() );
   if ( !doQuery( sqlQuery, query,
                  "UP::addUserChangeToChangelog() get cur wft" ) ) 
   {
      mc2log << error << "UP::addUserChangeToChangelog(): "
             << "get cur wft failed!" << endl;
      strcpy( wftStr, "NULL" ); // Something
   } else {
      if ( sqlQuery->nextRow()) {
         strcpy( wftStr, sqlQuery->getColumn( 0 ) );
      } else { // Has none
         strcpy( wftStr, "NULL" );
      }
   }

   sprintf( query, "SELECT * from ISABUserUser WHERE UIN = %u", 
            user->getUIN() );

   if ( !doQuery( sqlQuery, query,
                  "UP::addUserChangeToChangelog() get cur user" ) ) 
   {
      mc2log << error << "UP::addUserChangeToChangelog(): "
             << "get cur user failed!" << endl;      
   } else {
      if ( sqlQuery->nextRow()) { 
         // Remove old login from cache if changed
         if ( user->changed( UserConstants::USER_LOGONID ) ) {
            char* lowerLogonID = StringUtility::newStrDup(
               MC2String(StringUtility::copyLower(
               sqlQuery->getColumn( UserConstants::USER_LOGONID ) )).c_str());
#ifdef PARALLEL_USERMODULE
            m_loginCache->remove( lowerLogonID );
#else
            m_userLoginCache->removeSession( lowerLogonID );
#endif
            delete [] lowerLogonID;
         }
         strcpy( query, "INSERT INTO ISABUserchangelog VALUES ( " );
         uint32 numcols = sqlQuery->getNumColumns();
         for ( uint32 i = 0 ; i < numcols ; ++i ) {
            StringUtility::SQLEscapeSecure( sqlQuery->getColumn( i ), 
                                            userData, 4095 );
            strcat( query, "'" );
            strcat( query, userData );
            strcat( query, "', " );
         }
         // Add type 
         strcat( query, wftStr );
         strcat( query, ", " );
         // Add changeTime
         sprintf( userData, "%d", now );
         strcat( query, userData );
         strcat( query, " )" );
         // XXX: add changerUIN!!!!!!!!! To db and code here.

         // Query it
         if ( !doQuery( sqlQuery, query,
                        "UP::addUserChangeToChangelog() "
                        "add user change" ) )
         {
            mc2log << error << "UP::addUserChangeToChangelog(): "
                   << "add user change failed!" << endl;
         }
      } else {
         mc2log << error << "UP::addUserChangeToChangelog(): "
                << "no cur user!" << endl;
      }
   }
}


void 
UserProcessor::addUserRegionTolog( UserUser* user, CharEncSQLQuery* sqlQuery ) {
   char query[4096];
   uint32 now = TimeUtility::getRealTime();

   // Add all current regions with same changeTime
   uint32 nbrAccesses = user->getNbrOfType( 
      UserConstants::TYPE_REGION_ACCESS );
   for ( uint32 i = 0 ; i < nbrAccesses ; i++ ) {
      UserRegionAccess* access = static_cast<UserRegionAccess*> ( 
         user->getElementOfType( i,
                                 UserConstants::TYPE_REGION_ACCESS ) );
      if ( ! access->removed() ) {
         // Not removed
         strcpy( query, "INSERT INTO ISABUserRegionchangelog VALUES ( ");
         addUserRegionAccessData( query + strlen( query ), access->getID(),
                                  user->getUIN(), access );
         // Add changeTime
         sprintf( query + strlen( query ), ", %d )", now );
      } else {
         // Removed
         strcpy( query, "INSERT INTO ISABUserRegionchangelog VALUES ( " );
         // Add removed data
         sprintf( query + strlen( query ), "%d, %u, -1, 0, 0, %d )", 
                  access->getID(), user->getUIN(), now );
      }
      if ( !doQuery( sqlQuery, query,
                     "UP::addUserRegionTolog() add user region "
                     "change" ) ) 
      {
         mc2log << error << "UP::addUserRegionTolog(): "
                << "add user region change failed! " << query << endl;
      }
   }
}


void 
UserProcessor::addUserRightTolog( UserUser* user, uint32 changerUIN,
                                  CharEncSQLQuery* sqlQuery ) 
{
   char query[4096];
   // Add all changed rights with same changeTime
   uint32 now = TimeUtility::getRealTime();

   uint32 nbrRights = user->getNbrOfType( 
      UserConstants::TYPE_RIGHT );
   for ( uint32 i = 0 ; i < nbrRights ; i++ ) {
      UserRight* right = static_cast<UserRight*> ( 
         user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
      if ( ! right->removed() ) {
         // Not removed
         strcpy( query, "INSERT INTO ISABUserRightchangelog SET ");
         // ID and UIN
         sprintf( query + strlen( query ), " %s=%d, %s=%u, ", 
                  UserConstants::UserRightFieldName[ 
                     UserConstants::USER_RIGHT_ID ],
                  right->getID(), UserConstants::UserRightFieldName[ 
                     UserConstants::USER_RIGHT_USERUIN ],
                  user->getUIN() );
         if ( addChangedUserRightData( query + strlen( query ),  right ) 
              > 0 )
         {
            // Added something, add ','
            strcat( query, ", " );
         }
         // Add changeTime, changerUIN
         sprintf( query + strlen( query ), " %s=%d, %s=%u ",
                  "changeTime", now, "changerUIN", changerUIN );
      } else {
         // Removed
         strcpy( query, "INSERT INTO ISABUserRightchangelog VALUES ( " );
         // Add removed data
         sprintf( query + strlen( query ), "%d, %u, -1, -1, -1, -1, -1, "
                  "1, 'REMOVED', %d, %u )", 
                  right->getID(), user->getUIN(), now, changerUIN );
      }
      if ( !doQuery( sqlQuery, query,
                     "UP::addUserRightTolog() add user right change" ) ) 
      {
         mc2log << error << "UP::addUserRightTolog(): "
                << "add user right change failed! " << query << endl;
      }
   }
}


void 
UserProcessor::addUserTokenTolog( UserUser* user, uint32 changerUIN,
                                  CharEncSQLQuery* sqlQuery ) 
{
   char query[4096];
   // Add all changed tokens with same changeTime
   uint32 now = TimeUtility::getRealTime();

   uint32 nbr = user->getNbrOfType( UserConstants::TYPE_TOKEN );
   for ( uint32 i = 0 ; i < nbr ; i++ ) {
      UserToken* token = static_cast<UserToken*> ( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( ! token->removed() ) {
         // Not removed
         strcpy( query, "INSERT INTO ISABUserTokenchangelog SET ");
         // ID and UIN
         sprintf( query + strlen( query ), " %s=%d, %s=%u, ", 
                  UserConstants::UserTokenFieldName[ 
                     UserConstants::USER_TOKEN_ID ],
                  token->getID(), UserConstants::UserTokenFieldName[ 
                     UserConstants::USER_TOKEN_USERUIN ],
                  user->getUIN() );
         if ( addChangedUserTokenData( query + strlen( query ), token ) 
              > 0 )
         {
            // Added something, add ','
            strcat( query, ", " );
         }
         // Add changeTime, changerUIN
         sprintf( query + strlen( query ), " %s=%d, %s=%u ",
                  "changeTime", now, "changerUIN", changerUIN );
      } else {
         // Removed
         strcpy( query, "INSERT INTO ISABUserTokenchangelog VALUES ( " );
         // Add removed data
         sprintf( query + strlen( query ), "%d, %u, -1, -1, 'REMOVED', '',"
                  "%d, %u )", 
                  token->getID(), user->getUIN(), now, changerUIN );
      }
      if ( !doQuery( sqlQuery, query,
                     "UP::addUserTokenTolog() add user token change" ) ) 
      {
         mc2log << error << "UP::addUserTokenTolog(): "
                << "add user token change failed! " << query << endl;
      }
   }
}


void 
UserProcessor::addUserPINTolog( UserUser* user, uint32 changerUIN,
                                CharEncSQLQuery* sqlQuery ) 
{
   char query[4096];
   // Add all changed PINs with same changeTime
   uint32 now = TimeUtility::getRealTime();

   uint32 nbr = user->getNbrOfType( UserConstants::TYPE_PIN );
   for ( uint32 i = 0 ; i < nbr ; i++ ) {
      UserPIN* PIN = static_cast<UserPIN*> ( 
         user->getElementOfType( i, UserConstants::TYPE_PIN ) );
      if ( ! PIN->removed() ) {
         // Not removed
         strcpy( query, "INSERT INTO ISABUserPINchangelog SET ");
         // ID and UIN
         sprintf( query + strlen( query ), " %s=%d, %s=%u, ", 
                  UserConstants::UserPINFieldName[ 
                     UserConstants::USER_PIN_ID ],
                  PIN->getID(), UserConstants::UserPINFieldName[ 
                     UserConstants::USER_PIN_USERUIN ],
                  user->getUIN() );
         if ( addChangedUserPINData( query + strlen( query ),  PIN ) 
              > 0 )
         {
            // Added something, add ','
            strcat( query, ", " );
         }
         // Add changeTime, changerUIN
         sprintf( query + strlen( query ), " %s=%d, %s=%u ",
                  "changeTime", now, "changerUIN", changerUIN );
      } else {
         // Removed
         strcpy( query, "INSERT INTO ISABUserPINchangelog VALUES ( " );
         // Add removed data
         sprintf( query + strlen( query ), "%d, %u, 'REMOVED', '', "
                  "%d, %u )", 
                  PIN->getID(), user->getUIN(), now, changerUIN );
      }
      if ( !doQuery( sqlQuery, query,
                     "UP::addUserPINTolog() add user PIN change" ) ) 
      {
         mc2log << error << "UP::addUserPINTolog(): "
                << "add user PIN change failed! " << query << endl;
      }
   }
}


void 
UserProcessor::addUserIDKeyTolog( UserUser* user, uint32 changerUIN,
                                  CharEncSQLQuery* sqlQuery ) 
{
   uint32 nbr = user->getNbrOfType( UserConstants::TYPE_ID_KEY );
   for ( uint32 i = 0 ; i < nbr ; i++ ) {
      UserIDKey* idKey = static_cast<UserIDKey*> ( 
         user->getElementOfType( i, UserConstants::TYPE_ID_KEY ) );
      addUserIDKeyTolog( user->getUIN(), idKey, changerUIN, sqlQuery );
   }
}


void 
UserProcessor::addUserIDKeyTolog( uint32 uin, 
                                  const UserIDKey* idKey,
                                  uint32 changerUIN,
                                  CharEncSQLQuery* sqlQuery )
{
   char query[4096];
   uint32 now = TimeUtility::getRealTime();

   if ( ! idKey->removed() ) {
      // Not removed
      strcpy( query, "INSERT INTO ISABUserIDKeychangelog SET ");
      // ID and UIN
      sprintf( query + strlen( query ), " %s=%d, %s=%u, ", 
               UserConstants::UserIDKeyFieldName[ 
                  UserConstants::USER_ID_KEY_ID ],
               idKey->getID(), UserConstants::UserIDKeyFieldName[ 
                  UserConstants::USER_ID_KEY_USERUIN ],
               uin );
      if ( addChangedUserIDKeyData( query + strlen( query ),  idKey ) > 0 ) {
         // Added something, add ','
         strcat( query, ", " );
      }
      // Add changeTime, changerUIN
      sprintf( query + strlen( query ), " %s=%d, %s=%u ",
               "changeTime", now, "changerUIN", changerUIN );
   } else {
      // Removed
      strcpy( query, "INSERT INTO ISABUserIDKeychangelog VALUES ( " );
      // Add removed data
      sprintf( query + strlen( query ), "%d, %u, -1, 'REMOVED', "
               "%d, %u )", 
               idKey->getID(), uin, now, changerUIN );
   }
   if ( !doQuery( sqlQuery, query,
                  "UP::addUserIDKeyTolog() add user idKey change" ) ) 
   {
      mc2log << error << "UP::addUserIDKeyTolog(): "
             << "add user idKey change failed! " << query << endl;
   }
}

void 
UserProcessor::addUserLastClientTolog( UserUser* user, uint32 changerUIN,
                                       CharEncSQLQuery* sqlQuery ) 
{
   char query[4096];
   // Add all changed lastClients with same changeTime
   uint32 now = TimeUtility::getRealTime();

   uint32 nbr = user->getNbrOfType( UserConstants::TYPE_LAST_CLIENT );
   for ( uint32 i = 0 ; i < nbr ; i++ ) {
      UserLastClient* lastClient = static_cast<UserLastClient*> ( 
         user->getElementOfType( i, UserConstants::TYPE_LAST_CLIENT ) );
      strcpy( query, "INSERT INTO ISABUserLastClientchangelog SET ");
      // ID and UIN
      sprintf( query + strlen( query ), " %s=%d, %s=%u, ", 
               UserConstants::UserLastClientFieldName[ 
                  UserConstants::USER_LAST_CLIENT_ID ],
               lastClient->getID(), UserConstants::
               UserLastClientFieldName[ 
                  UserConstants::USER_LAST_CLIENT_USERUIN ],
               user->getUIN() );
      if ( lastClient->removed() ) {
         strcat( query, UserConstants::UserLastClientFieldName[ 
                    UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ] );
         strcat( query, " = 'REMOVED' , " );
      } else if ( addChangedUserLastClientData( query + strlen( query ),  
                                         lastClient ) > 0 )
      {
         // Added something, add ','
         strcat( query, ", " );
      }
      // Add changeTime, changerUIN
      sprintf( query + strlen( query ), " %s=%d, %s=%u ",
               "changeTime", now, "changerUIN", changerUIN );
      if ( !doQuery( sqlQuery, query,
                     "UP::addUserLastClientTolog() add user lastClient "
                     "change" ) ) 
      {
         mc2log << error << "UP::addUserLastClientTolog(): "
                << "add user lastClient change failed! " << query << endl;
      }
   }
}


void 
UserProcessor::addOldUserLicenceTolog( uint32 uin, 
                                       const UserLicenceKey* licence,
                                       uint32 changerUIN,
                                       CharEncSQLQuery* sqlQuery )
{
   char query[ 4096 ];
   uint32 now = TimeUtility::getRealTime();

   if ( ! licence->removed() ) {
      // Not removed
      strcpy( query, "INSERT INTO ISABUserLicencechangelog SET ");
      // ID and UIN
      sprintf( query + strlen( query ), " %s=%d, %s=%u, ", 
               UserConstants::UserLicenceKeyFieldName[ 
                  UserConstants::USER_LICENCE_KEY_ID ],
               licence->getID(), UserConstants::UserLicenceKeyFieldName[
                  UserConstants::USER_LICENCE_KEY_USERUIN ],
               uin );
      if ( addChangedUserLicenceData( query + strlen( query ), 
                                      licence ) > 0 )
      {
         // Added something, add ','
         strcat( query, ", " );
      }
      // Add changeTime, changerUIN
      sprintf( query + strlen( query ), " changeTime=%d, changerUIN=%u ",
               now, changerUIN );
   } else {
      // Removed
      strcpy( query, "INSERT INTO ISABUserLicencechangelog VALUES ( " );
      // Add removed data
      sprintf( query + strlen( query ), "%d, %u, -1, 'REMOVED', "
               "%d, %u )", 
               licence->getID(), uin, now, changerUIN );
   }
   if ( !doQuery( sqlQuery, query,
                  "UP::addOldUserLicenceTolog() add user licence change" ))
   {
      mc2log << error << "UP::addOldUserLicenceTolog(): "
             << "add user licence change failed! " << query << endl;
   }
}

void 
UserProcessor::addUserLicenceTolog( uint32 uin, 
                                    const UserLicenceKey* licence,
                                    uint32 changerUIN,
                                    CharEncSQLQuery* sqlQuery )
{
   char query[ 4096 ];
   uint32 now = TimeUtility::getRealTime();

   MC2String keyStr( licence->getLicenceKeyStr() );
   if ( licence->removed() ) {
      keyStr.append( " REMOVED" );
   }

   int pos = sprintf( 
      query, "INSERT INTO ISABUserLicenceKeychangelog VALUES ( %d, %u, ",
      licence->getID(), uin );
   strcpy( query + pos, "'" ); pos += 1;
   pos += UserElement::sqlString( query + pos, keyStr );
   strcpy( query + pos, "', '" ); pos += 4;
   pos += UserElement::sqlString( query + pos, licence->getProduct() );
   strcpy( query + pos, "', '" ); pos += 4;
   pos += UserElement::sqlString( query + pos, licence->getKeyType() );
   strcpy( query + pos, "'" ); pos += 1;
   
   // Add changeTime, changerUIN
   pos += sprintf( query + pos, ", %d, %u ) ", now, changerUIN );

   if ( !doQuery( sqlQuery, query,
                  "UP::addUserLicenceTolog() add user licence change" ))
   {
      mc2log << error << "UP::addUserLicenceTolog(): "
             << "add user licence change failed! " << query << endl;
   }
}


#define HANDLECHANGEUSERDATAREQUESTPACKET_ABORT \
      if ( !m_sqlConnection->rollbackTransaction()) { \
         mc2log << error << "UP::handleChangeUserRequestPacket " \
                << " rollbackTransaction() failed!" << endl; \
      } \
      if (NULL == user) \
         delete user; \
      delete sqlQuery; \
      reply = new ChangeUserDataReplyPacket( p, 0 ); \
      reply->setStatus( StringTable::NOTOK ); \
      return reply;   
  

UserReplyPacket*
UserProcessor::handleChangeUserDataRequestPacket(
   const ChangeUserDataRequestPacket* p ) 
{
   char query[ 65537 ];
   uint32 UIN = p->getUIN();
   ChangeUserDataReplyPacket* reply = NULL;
   // Extract user
   UserUser* user = NULL;
   uint16 nbrElements = p->getNbrElements();
   int pos = p->getFirstElementPosition();
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   bool loggedUserChange = false; // Changelog save WFT only once

   if ( nbrElements < 1 ) {
      mc2log << warn << "UP::handleChangeUserDataRequestPacket(): "
             << " No elements to change, for UIN: " << UIN << "returning OK."
             << endl;
      delete sqlQuery;
      reply = new ChangeUserDataReplyPacket( p, true );
      reply->setStatus( StringTable::OK );
      return reply; 
   }

   // Remove from cache
   removeUserFromCache( p->getUIN() );

   UserElement* elem;
   for ( uint16 i = 0 ; i < nbrElements ; i++ ) {
      elem = p->getNextElement( pos );
      if ( elem == NULL ) {
         mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                << " UserElementExtraction failed for UIN: " << UIN << endl;
         reply = new ChangeUserDataReplyPacket( p, false );
         reply->setStatus( StringTable::NOTOK );
         return reply;
      }
      switch( elem->getType() ) {
         case UserConstants::TYPE_USER :
            if ( user != NULL ) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                      << " Duplicate User or not UserUser first, UIN: "
                      << UIN << endl;
               delete elem;
               delete user;
               reply = new ChangeUserDataReplyPacket( p, false );
               reply->setStatus( StringTable::NOTOK );
               return reply;
            }
            user = static_cast<UserUser*> ( elem );
            break;
         case UserConstants::TYPE_CELLULAR :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_CELLULAR" << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_BUDDY :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 << "Adding userelement TYPE_BUDDY" << endl;
            user->addElement( elem );
            break;
         case UserConstants::TYPE_NAVIGATOR :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 << "Adding userelement TYPE_NAVIGATOR" << endl;
            user->addElement( elem );
            break;
         case UserConstants::TYPE_LICENCE_KEY :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_LICENCE_KEY" << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_REGION_ACCESS :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_REGION_ACCESS" << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_RIGHT :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_RIGHT" << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_WAYFINDER_SUBSCRIPTION" 
                    << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_TOKEN :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_TOKEN" 
                    << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_PIN :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_PIN" 
                    << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_ID_KEY :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_ID_KEY" 
                    << endl; 
            user->addElement( elem );
            break;
         case UserConstants::TYPE_LAST_CLIENT :
            if ( user == NULL ) {
               user = new UserUser( UIN );
            }
            mc2dbg8 <<  "Adding userelement TYPE_LAST_CLIENT" 
                    << endl; 
            user->addElement( elem );
            break;

         default:
            mc2log << warn << "UP::handleChangeUserDataRequestPacket(): "
                   << " ) Unsupported element type: " 
                   << (int)elem->getType() << ", for UIN: " << UIN << endl;
            delete elem;
            delete user;
            reply = new ChangeUserDataReplyPacket( p, false );
            reply->setStatus( StringTable::NOTOK );
            return reply;
      }
   }
   
   if ( !m_sqlConnection->beginTransaction()) {
      mc2log << error << "UP::handleChangeUserRequestPacket(): "
             << " beginTransaction() failed!" << endl;
      delete user;
      reply = new ChangeUserDataReplyPacket( p, false );
      reply->setStatus( StringTable::NOTOK );
      return reply;
   } 

   if ( user->getNbrChanged() > 0 ) {
      // Add change into changelogtable, Must be before changing UserUser
      // table
      addUserChangeToChangelog( user, p->getChangerUIN(), sqlQuery );
      loggedUserChange = true;
      // Add UserUsers changes
      char changed[ 32768 ];
      addChangedUserUserData( changed, user );
      sprintf(query, "UPDATE ISABUserUser SET %s WHERE UIN = %u",
              changed, UIN ); 
      if ( ! doQuery(sqlQuery, query, 
                 "UP::handleChangeUserRequestPacket() User") ) {
         mc2log << error << "UP::handleChangeUserRequestPacket(): "
                << " change UserUser failed!" << endl;
         HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
      }
   }
   
   /// Add changes for all element types

   // Add All UserCellular changes
   uint32 nbrCellular = user->getNbrOfType( UserConstants::TYPE_CELLULAR );
   for ( uint32 i = 0 ; i < nbrCellular ; i++ ) {
      UserCellular* cellular = static_cast<UserCellular*>( 
         user->getElementOfType( i,
                                 UserConstants::TYPE_CELLULAR ) );
      if ( (cellular->getNbrChanged() > 0) || (cellular->removed()) ) {
         if ( cellular->removed() == 0 && cellular->getID() !=0 ) {
            // Add UserCellular changes
            char changed[4096];
            addChangedUserCellularData( changed, cellular );
            sprintf(query, "UPDATE ISABUserCellular SET %s WHERE id = %u",
                    changed, cellular->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() Cellular") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserCellular failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( cellular->removed() == 1 && cellular->getID() != 0 ) {
            //Remove cellularphone
            sprintf( query, "DELETE FROM ISABUserCellular WHERE id = '%d'",
                     cellular->getID() );
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() remove cellular")) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserCellular failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
             }
         } else {
            // Add new UserCellular
            uint32 ID = getNewCellularID();
      
            if ( ID == 0 ) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding cellularID" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserCellular VALUES ( ");
            addUserCellularData( query + strlen(query), ID, UIN, cellular );
            strcat( query, " )");
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add cellular")) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserCellular failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
      }
   }

   // Add All UserBuddyList changes
   uint32 nbrBuddies = user->getNbrOfType( UserConstants::TYPE_BUDDY );
   for ( uint32 i = 0; i < nbrBuddies ; i++ ) {
      UserBuddyList* buddy = static_cast<UserBuddyList*>( 
         user->getElementOfType( i, UserConstants::TYPE_BUDDY ) );
      if ( (buddy->getNbrChanged() > 0) || (buddy->removed()) ) {  
         if ( !buddy->removed() && buddy->getID() != 0 ) {
            // Add UserBuddyList changes
            char changed[4096];
            addChangedUserBuddyListData( changed, buddy );
            sprintf(query, "UPDATE ISABUserBuddyList SET %s WHERE id = %d",
                     changed, buddy->getID() );  
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() update buddy")) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserBuddyList failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( buddy->removed() == 1 && buddy->getID() != 0 ) {
            // Remove BuddyList
            sprintf( query, "DELETE FROM ISABUserBuddyList WHERE id = %d ",
                     buddy->getID() );
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() remove buddy")) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " remove UserBuddyList failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
             }
         } else {
            // Add new UserBuddyList
            uint32 ID = getNewBuddyListID();
      
            if ( ID == 0 ) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket "
                      << " failed finding BuddyListID!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserBuddyList VALUES ( ");
            addUserBuddyListData( query + strlen(query),  buddy );
            strcat( query, " )");
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add buddy")) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserBuddyList failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
      }
   }


   // Add All UserNavigator changes
   uint32 nbrNavigators = 
      user->getNbrOfType( UserConstants::TYPE_NAVIGATOR );
   for ( uint32 i = 0; i < nbrNavigators ; i++ ) {
      UserNavigator* nav = static_cast< UserNavigator* >( 
         user->getElementOfType( i, UserConstants::TYPE_NAVIGATOR ) );
      if ( ( nav->getNbrChanged() > 0) || ( nav->removed() ) ) {  
         if ( !nav->removed() && nav->getID() != 0 ) {
            // Add UserNavigator changes

            if ( !nav->getDB()->update( sqlQuery ) && doQuery(sqlQuery, NULL,
                                     "UP::handleChangeUserRequestPacket()") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket "
                      << " change UserNavigator failed! "  << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( nav->removed() && nav->getID() != 0 ) {
            // Remove Navigator
            if ( !nav->getDB()->remove( sqlQuery ) && doQuery(sqlQuery, NULL, 
                                     "UP::handleChangeUserRequestPacket()") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " remove UserNavigator failed! " << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
             }
         } else {
            // Add new UserNavigator
            if ( !nav->getDB()->insert( sqlQuery ) && doQuery(sqlQuery, NULL, 
                                     "UP::handleChangeUserRequestPacket()") ) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserNavigator failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
      }
   }


   // Add All UserLicenceKey changes
   uint32 nbrLicencies = user->getNbrOfType( 
      UserConstants::TYPE_LICENCE_KEY );
   bool changedLicence = false;
   for ( uint32 i = 0 ; i < nbrLicencies ; i++ ) {
      UserLicenceKey* licence = static_cast<UserLicenceKey*>( 
         user->getElementOfType( i,
                                 UserConstants::TYPE_LICENCE_KEY ) );
      if ( (licence->getNbrChanged() > 0) || (licence->removed()) ) {
         if ( licence->removed() == 0 && licence->getID() !=0 ) {
            // Add UserLicenceKey changes
            if ( !changeUserLicence( user->getUIN(), p->getChangerUIN(), 
                                     licence, sqlQuery ) ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserLicenceKey failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( licence->removed() == 1 && licence->getID() != 0 ) {
            // Remove UserLicenceKey
            if ( !deleteUserLicence( user->getUIN(), p->getChangerUIN(), 
                                     licence, sqlQuery ) ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserLicenceKey failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
             }
         } else {
            // Add new UserLicenceKey
            if ( !addUserLicence( user->getUIN(), p->getChangerUIN(), 
                                  licence, sqlQuery ) ) {
               mc2log << error << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserLicenceKey failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedLicence = true;
      }
   }


   // Add All UserRegionAccess changes
   bool changedRegion = false;
   uint32 nbrAccesses = user->getNbrOfType( 
      UserConstants::TYPE_REGION_ACCESS );
   for ( uint32 i = 0 ; i < nbrAccesses ; i++ ) {
      UserRegionAccess* access = static_cast<UserRegionAccess*> ( 
         user->getElementOfType( i,
                                 UserConstants::TYPE_REGION_ACCESS ) );
      if ( (access->getNbrChanged() > 0) || (access->removed()) ) {
         if ( access->removed() == 0 && access->getID() !=0 ) {
            // Add UserRegionAccess changes
            char changed[4096];
            addChangedUserRegionAccessData( changed, access );
            sprintf( query, "UPDATE ISABUserRegionAccess SET %s "
                     "WHERE id = %u",
                     changed, access->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() region access") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserRegionAccess failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( access->removed() == 1 && access->getID() != 0 ) {
            // Remove UserRegionAccess
            sprintf( query, "DELETE FROM ISABUserRegionAccess "
                     "WHERE id = '%d'",
                     access->getID() );
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() remove region "
                          "access") ) 
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserRegionAccess failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserRegionAccess
            uint32 ID = getNewRegionAccessID();
      
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding region access id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserRegionAccess VALUES ( ");
            addUserRegionAccessData( query + strlen(query), ID, UIN, 
                                     access );
            strcat( query, " )");
            access->setID( ID );
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add region access") )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserRegionAccess failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedRegion = true;
      }
   }
   if ( changedRegion ) {
      // Add change into changelogtable
      // Add what there is now
      addUserRegionTolog( user, sqlQuery );      
   }


   // Add All UserRight changes
   bool changedRight = false;
   uint32 nbrRights = user->getNbrOfType( UserConstants::TYPE_RIGHT );
   mc2dbg4 << "The number of rights " << nbrRights << endl;
   for ( uint32 i = 0 ; i < nbrRights ; i++ ) {
      UserRight* right = static_cast<UserRight*> ( 
         user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
      if ( (right->getNbrChanged() > 0) || (right->removed()) ) {
         if ( right->removed() == 0 && right->getID() != 0 ) {
            // Add UserRight changes
            char changed[4096];
            addChangedUserRightData( changed, right );
            sprintf( query, "UPDATE ISABUserRight SET %s "
                     "WHERE id = %u",
                     changed, right->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() right") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserRight failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( right->removed() == 1 && right->getID() != 0 ) {
            // Remove UserRight
            sprintf( query, "DELETE FROM ISABUserRight "
                     "WHERE id = '%d'",
                     right->getID() );
            if ( !doQuery( sqlQuery, query,
                           "UP::handleChangeUserRequestPacket() "
                           "remove right " ) )
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserRight failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserRight
            uint32 ID = getNewUniqueID( "ISABUserRight", "id" );
      
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding right id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserRight VALUES ( ");
            addUserRightData( query + strlen(query), ID, UIN, right );
            strcat( query, " )" );
            right->setID( ID );
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add right") )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserRight failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedRight = true;
      } // End if something changed or removed
   } // End for all rights
   if ( changedRight ) {
      // Add change into changelogtable
      // Add what there is now
      addUserRightTolog( user, p->getChangerUIN(), sqlQuery );      
   }


   // Add All UserWayfinderSubscription changes
   uint32 nbrWFSubscr = user->getNbrOfType( 
      UserConstants::TYPE_WAYFINDER_SUBSCRIPTION );
   for ( uint32 i = 0 ; i < nbrWFSubscr ; i++ ) {
      UserWayfinderSubscription* subscr = 
         static_cast<UserWayfinderSubscription*> ( 
            user->getElementOfType( 
               i,
               UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) );
      if ( (subscr->getNbrChanged() > 0) || (subscr->removed()) ) {
         // Add change into changelogtable 
         // Add what there is now, before change
         if ( !loggedUserChange ) {
            addUserChangeToChangelog( user, p->getChangerUIN(), sqlQuery );
         }      

         if ( subscr->removed() == 0 && subscr->getID() != 0 ) {
            // Add UserWayfinderSubscription changes
            char changed[ 4096 ];
            addChangedUserWayfinderSubscriptionData( changed, subscr );
            sprintf( query, "UPDATE ISABUserWayfinderSubscription SET %s "
                     "WHERE id = %u",
                     changed, subscr->getID() );
            
            if ( ! doQuery( sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() WF subscription") )
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserWayfinderSubscription failed!" 
                      << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( subscr->removed() == 1 && subscr->getID() != 0 ) {
            // Remove UserWayfinderSubscription
            sprintf( query, "DELETE FROM ISABUserWayfinderSubscription "
                     "WHERE id = '%d'",
                     subscr->getID() );
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() remove WF "
                          "Subsctiption") ) 
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " remove UserWayfinderSubscription failed!" 
                      << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserWayfinderSubscription
            uint32 ID = getNewUniqueID( "ISABUserWayfinderSubscription", 
                                        "id" );
            
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding wf subscr id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, 
                    "INSERT INTO ISABUserWayfinderSubscription VALUES ( ");
            addUserWayfinderSubscriptionData( query + strlen( query ), 
                                              ID, UIN, subscr );
            strcat( query, " )");
      
            if ( !doQuery( sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add wfsubscr" ) )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserWayfinderSubscription failed!" 
                      << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
      }
   }


   // Add All UserToken changes
   bool changedToken = false;
   uint32 nbrTokens = user->getNbrOfType( UserConstants::TYPE_TOKEN );
   mc2dbg4 << "The number of tokens " << nbrTokens << endl;
   for ( uint32 i = 0 ; i < nbrTokens ; i++ ) {
      UserToken* t = static_cast<UserToken*> ( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( (t->getNbrChanged() > 0) || (t->removed()) ) {
         if ( t->removed() == 0 && t->getID() != 0 ) {
            // Add UserToken changes
            char changed[4096];
            addChangedUserTokenData( changed, t );
            sprintf( query, "UPDATE ISABUserToken SET %s WHERE id = %u",
                     changed, t->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() token") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserToken failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( t->removed() == 1 && t->getID() != 0 ) {
            // Remove UserToken
            sprintf( query, "DELETE FROM ISABUserToken WHERE id = '%d'",
                     t->getID() );
            if ( !doQuery( sqlQuery, query,
                           "UP::handleChangeUserRequestPacket() "
                           "remove token " ) )
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserToken failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserToken
            uint32 ID = getNewUniqueID( "ISABUserToken", "id" );
      
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding token id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserToken VALUES ( ");
            addUserTokenData( query + strlen(query), ID, UIN, t );
            strcat( query, " )" );
            t->setID( ID );
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add token") )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserToken failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedToken = true;
      } // End if something changed or removed
   } // End for all rights
   if ( changedToken ) {
      // Add change into changelogtable
      // Add what there is now
      addUserTokenTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   // Add All UserPIN changes
   bool changedPIN = false;
   uint32 nbrPINs = user->getNbrOfType( UserConstants::TYPE_PIN );
   mc2dbg4 << "The number of PINs " << nbrPINs << endl;
   for ( uint32 i = 0 ; i < nbrPINs ; i++ ) {
      UserPIN* t = static_cast<UserPIN*> ( 
         user->getElementOfType( i, UserConstants::TYPE_PIN ) );
      if ( (t->getNbrChanged() > 0) || (t->removed()) ) {
         if ( t->removed() == 0 && t->getID() != 0 ) {
            // Add UserPIN changes
            char changed[4096];
            addChangedUserPINData( changed, t );
            sprintf( query, "UPDATE ISABUserPIN SET %s WHERE id = %u",
                     changed, t->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() PIN") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserPIN failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( t->removed() == 1 && t->getID() != 0 ) {
            // Remove UserPIN
            sprintf( query, "DELETE FROM ISABUserPIN WHERE id = '%d'",
                     t->getID() );
            if ( !doQuery( sqlQuery, query,
                           "UP::handleChangeUserRequestPacket() "
                           "remove PIN " ) )
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserPIN failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserPIN
            uint32 ID = getNewUniqueID( "ISABUserPIN", "id" );
      
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding PIN id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserPIN VALUES ( ");
            addUserPINData( query + strlen(query), ID, UIN, t );
            strcat( query, " )" );
            t->setID( ID );
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add PIN") )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserPIN failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedPIN = true;
      } // End if something changed or removed
   } // End for all rights
   if ( changedPIN ) {
      // Add change into changelogtable
      // Add what there is now
      addUserPINTolog( user, p->getChangerUIN(), sqlQuery );      
   }


   // Add All UserIDKey changes
   bool changedidKey = false;
   uint32 nbridKeys = user->getNbrOfType( UserConstants::TYPE_ID_KEY );
   mc2dbg4 << "The number of idKeys " << nbridKeys << endl;
   for ( uint32 i = 0 ; i < nbridKeys ; i++ ) {
      UserIDKey* t = static_cast<UserIDKey*> ( 
         user->getElementOfType( i, UserConstants::TYPE_ID_KEY ) );
      if ( (t->getNbrChanged() > 0) || (t->removed()) ) {
         if ( t->removed() == 0 && t->getID() != 0 ) {
            // Add UserIDKey changes
            char changed[4096];
            addChangedUserIDKeyData( changed, t );
            sprintf( query, "UPDATE ISABUserIDKey SET %s WHERE id = %u",
                     changed, t->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() idKey") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserIDKey failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( t->removed() == 1 && t->getID() != 0 ) {
            // Remove UserIDKey
            sprintf( query, "DELETE FROM ISABUserIDKey WHERE id = '%d'",
                     t->getID() );
            if ( !doQuery( sqlQuery, query,
                           "UP::handleChangeUserRequestPacket() "
                           "remove idKey " ) )
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserIDKey failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserIDKey
            uint32 ID = getNewUniqueID( "ISABUserIDKey", "id" );
      
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding idKey id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserIDKey VALUES ( ");
            addUserIDKeyData( query + strlen(query), ID, UIN, t );
            strcat( query, " )" );
            t->setID( ID );
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add idKey") )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserIDKey failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedidKey = true;
      } // End if something changed or removed
   } // End for all rights
   if ( changedidKey ) {
      // Add change into changelogtable
      // Add what there is now
      addUserIDKeyTolog( user, p->getChangerUIN(), sqlQuery );      
   }


   // Add All UserLastClient changes
   bool changedlastClient = false;
   uint32 nbrlastClients = user->getNbrOfType( 
      UserConstants::TYPE_LAST_CLIENT );
   mc2dbg4 << "The number of lastClients " << nbrlastClients << endl;
   for ( uint32 i = 0 ; i < nbrlastClients ; i++ ) {
      UserLastClient* t = static_cast<UserLastClient*> ( 
         user->getElementOfType( i, UserConstants::TYPE_LAST_CLIENT ) );
      if ( (t->getNbrChanged() > 0) || (t->removed()) ) {
         if ( t->removed() == 0 && t->getID() != 0 ) {
            // Add UserLastClient changes
            char changed[4096];
            addChangedUserLastClientData( changed, t );
            sprintf( query, "UPDATE ISABUserLastClient SET %s "
                     "WHERE id = %u", changed, t->getID() );
            
            if ( ! doQuery(sqlQuery, query, 
                "UP::handleChangeUserRequestPacket() lastClient") ) {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserLastClient failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else if ( t->removed() == 1 && t->getID() != 0 ) {
            // Remove UserLastClient
            sprintf( query, "DELETE FROM ISABUserLastClient "
                     "WHERE id = '%d'", t->getID() );
            if ( !doQuery( sqlQuery, query,
                           "UP::handleChangeUserRequestPacket() "
                           "remove lastClient " ) )
            {
               mc2log << error << "UP::handleChangeUserRequestPacket(): "
                      << " change UserLastClient failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         } else {
            // Add new UserLastClient
            uint32 ID = getNewUniqueID( "ISABUserLastClient", "id" );
      
            if ( ID == 0 ) {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " failed finding lastClient id" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }

            strcpy( query, "INSERT INTO ISABUserLastClient VALUES ( ");
            addUserLastClientData( query + strlen(query), ID, UIN, t );
            strcat( query, " )" );
            t->setID( ID );
      
            if ( !doQuery(sqlQuery, query,
                 "UP::handleChangeUserRequestPacket() add lastClient") )
            {
               mc2log << error 
                      << "UP::handleChangeUserDataRequestPacket(): "
                      << " Add of UserLastClient failed!" << endl;
               HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
            }
         }
         changedlastClient = true;
      } // End if something changed or removed
   } // End for all rights
   if ( changedlastClient ) {
      // Add change into changelogtable
      // Add what there is now
      addUserLastClientTolog( user, p->getChangerUIN(), sqlQuery );      
   }

   
   if ( !m_sqlConnection->commitTransaction()) {
      mc2log << error << "UP::handleChangeUserRequestPacket(): "
             << " commitTransaction() failed!" << endl;
      HANDLECHANGEUSERDATAREQUESTPACKET_ABORT;
   }

   mc2log << info << "UP::handleChangeUserRequestPacket(): Changed "
          << UIN << " by " << p->getChangerUIN() << endl;

   reply = new ChangeUserDataReplyPacket( p, true );
   reply->setStatus( StringTable::OK );

   delete sqlQuery;
   delete user;
   return reply;
}

UserReplyPacket*
UserProcessor::handleCheckUserPasswordRequestPacket(
   const CheckUserPasswordRequestPacket* p ) 
{
   CheckUserPasswordReplyPacket* reply = NULL;
   char logonID[256];
   char passwd[256];
   uint32 UIN = 0;
   uint32 now = TimeUtility::getRealTime();
   MC2String logon( p->getLogonID() );
   StringUtility::SQLEscapeSecure( p->getPassword(), passwd, 255 );

   if ( logon.find( '_' ) != MC2String::npos ) {
      // Have special '_' login like admin_wlfresa means login 
      // admin with password and check if edituser then make sesion for
      // wlfresa and return wlfresa UIN.
      char query[ 1024 ];
      CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
      bool ok = true;

      uint32 sepPos = logon.find( '_' );
      // Terminate at '_'
      logon[ sepPos ] = '\0';
#undef THIS_IS_REL_WITHOUT_UTF8
#ifdef THIS_IS_REL_WITHOUT_UTF8
      char* lowerLogonID = StringUtility::copyLower( logon.c_str() );
      StringUtility::SQLEscapeSecure( lowerLogonID, logonID, 255 );
      delete [] lowerLogonID;
      lowerLogonID = NULL;
#else 
      MC2String lowerLogonID( StringUtility::copyLower( logon.c_str() ) );
      StringUtility::SQLEscapeSecure( lowerLogonID.c_str(), logonID, 255 );
#endif

      // Check first user
      sprintf( query,
               "SELECT UIN, logonPasswd, validDate FROM ISABUserUser "
               "WHERE logonID = '%s' AND externalXMLService != 'f'", 
               logonID );
      
      if ( !doQuery( sqlQuery, query, "UP::CheckUserPassword other" ) ) {
         ok = false;
      }
  
      if ( ok && sqlQuery->nextRow() ) {
         //  Found one user, check password!
         const char* pwd = sqlQuery->getColumn( 1 );
         if ( ::checkPassword( pwd, passwd, sqlQuery->getColumn(0) ) ) {
            // OK, correct password
            UIN = strtoul( sqlQuery->getColumn(0), NULL, 10);
            if ( p->getExpired() ) {
               // Check valid until
               uint32 now = TimeUtility::getRealTime();
               uint32 validDate = StringUtility::makeDate( 
                  sqlQuery->getColumn( 2 ) );
               if ( validDate < now && validDate != 0 ) {
                  mc2log << info << "UP::CheckUserPassword other for "
                         << "logonId [" 
                         << logonID << "] User is expired, validDate " 
                         << sqlQuery->getColumn( 2 ) << endl;
                  UIN = MAX_UINT32 - 1;
               }
            }
         } else {
            mc2log << info << "UP::CheckUserPassword other for logonID ["
                   << logonID
                   << "] wrong password, logonPasswd: [" << passwd
                   << "], pwd: [" << pwd << "]" << endl;
         }
      } else {
         mc2log << info << "UP::CheckUserPassword other for logonID ["
                << logonID << "] logon failed: unknown/unauthorized user "
                << endl;
      }
      delete sqlQuery;

      // If UIN then get second user
      if ( ok && UIN != 0 && UIN != MAX_UINT32 - 1 ) {
         UserUser user( MAX_UINT32 );
         user.setLogonID( logon.c_str() + sepPos + 1 );
         vector<uint32> users;
         vector<MC2String> logonIDs;
         uint32 nbrUsers = getUINsFromUserUser( &user, users, logonIDs );
         if ( nbrUsers == 1 ) {
            UIN = users[ 0 ];
         } else {
            mc2log << info << "UP::CheckUserPassword other for logonID ["
                   << user.getLogonID() << "] unknown user " << endl; 
            UIN = 0;
         }
      }

   } else {
      StringUtility::SQLEscapeSecure( p->getLogonID(), logonID, 255 );

      mc2dbg4 << "UP::handleCheckUserPasswordRequestPacket "
              << "LogonID: #" << logonID << "#" << endl;

// #define USER_MODULE_SHUT_DOWN
#ifdef USER_MODULE_SHUT_DOWN
      if ( strcmp( logonID, "Shutdown" ) == 0 && strcmp( passwd, "now" ) 
           == 0 )
      {
         mc2log << info << "UserProcesser::"
                << "handleCheckUserPasswordRequestPacket "
                << "USER_MODULE_SHUT_DOWN" << endl;
         exit( 0 );
      }
#endif

      UIN = getUser( logonID, passwd, p->getExpired() );
   }

   if ( UIN != 0 && UIN != MAX_UINT32 - 1 ) {
      char id[31];
      char key[31];
      if ( makeSession( UIN, now, id, key ) ) {
         reply = new CheckUserPasswordReplyPacket( p, UIN, id,  key);
         reply->setStatus( StringTable::OK );
      } else {
         reply = new CheckUserPasswordReplyPacket( p, 0, "", "" );
         reply->setStatus( StringTable::NOTOK );
      }
   } else {
      reply = new CheckUserPasswordReplyPacket( p, UIN, "", "" );
      reply->setStatus( StringTable::OK );
   }

   return reply;
}


UserReplyPacket*
UserProcessor::handleFindUserRequestPacket(
   const FindUserRequestPacket* p ) 
{
   FindUserReplyPacket* reply = NULL;
   
   auto_ptr<UserElement> elem( p->getElement() );
   if ( elem.get() != NULL ) {
      uint32 nbrUsers = 0;
      vector<uint32> users;
      vector<MC2String> logonIDs;
      switch ( elem->getType() ) {
         case UserConstants::TYPE_USER :
            nbrUsers = getUINsFromUserUser( 
               static_cast<UserUser*>( elem.get() ), users, logonIDs );
            break;
         case UserConstants::TYPE_CELLULAR :
            nbrUsers = getUINsFromUserCellular( 
               static_cast<UserCellular*>( elem.get() ), users, logonIDs );
            break;
         case UserConstants::TYPE_LICENCE_KEY :
            nbrUsers = getUINsFromUserLicenceKey( 
               static_cast< UserLicenceKey* >( elem.get() ), users, logonIDs );
            break;
         case UserConstants::TYPE_WAYFINDER_SUBSCRIPTION :
            nbrUsers = getUINsFromUserWayfinderSubscription( 
               static_cast< UserWayfinderSubscription* >( elem.get() ), 
               users, logonIDs );
            break;
         case UserConstants::TYPE_ID_KEY :
            nbrUsers = getUINsFromUserIDKey( 
               static_cast< UserIDKey* >( elem.get() ), users, logonIDs );
            break; 
         default:
            mc2log << warn << "UP::handleFindUserRequestPacket "
                   << " unsupported type: " << (int)elem->getType() 
                   << endl;    
      };
      reply = new FindUserReplyPacket( 
         p, nbrUsers, &users.front(), &logonIDs );
      reply->setStatus( StringTable::OK );
   } else {
      reply = new FindUserReplyPacket( p, 0, NULL, NULL );
      reply->setStatus( StringTable::OK );
   }
   
   return reply;
}

#define HANDLEUSERFAVORITESREQUESTPACKET_ABORT \
      if ( !m_sqlConnection->rollbackTransaction()) { \
         mc2log << error << "UP::handleUserFavoritesRequestPacket " \
                " rollbackTransaction() failed!" << endl; \
      } \
      delete sqlQuery; \
      reply->setStatus( StringTable::NOTOK ); \
      return reply;   



      /**
       *   Add zero or more favorites from a SQLQuery result
       *   @param sqlQuery  Pointer to a sqlQuery that has been executed
       *                    and returned with no errors
       *   @param max       Maximum favorites to add
       *   @param onlyID    Only restore the ID from the query
       *   @return Number of favorites added or -1 if error
       */
int addFavoriteFromSQLQuery( SQLQuery* sqlQuery, UserFavoritesList& l,
                             bool onlyID = false, uint32 max = MAX_UINT32 )
{
   //      MOVE THIS SQL TO UserProcessor where all SQL belongs!!!
   //      This method puts requiremets on the SELECT statement so putting
   //      this near the SELECT is a good idea!
   int nbrAdded = 0;

   if ( onlyID ) {
      if ( sqlQuery->getNumColumns() == 9 || 
           sqlQuery->getNumColumns() == 1 ) 
      {
         while ( sqlQuery->nextRow() && uint32(nbrAdded) < max )  {
            l.addFavorite( atoi( sqlQuery->getColumn( 0 ) ) );
         }
      } else {
         mc2log << warn << "UserFavoritesList::addFromSQLQuery(): "
                << "number of columns in sqlQuery != 9 and != 1 (onlyID)" 
                << endl;
         nbrAdded = -1;
      }
   } else {
      if ( sqlQuery->getNumColumns() == 9 ) {
         while ( sqlQuery->nextRow() && uint32(nbrAdded) < max )  {
            l.addFavorite( atoi(sqlQuery->getColumn( 0 ) ),
                           atoi(sqlQuery->getColumn( 1 ) ),
                           atoi(sqlQuery->getColumn( 2 ) ),
                           sqlQuery->getColumn( 3 ),
                           sqlQuery->getColumn( 4 ),
                           sqlQuery->getColumn( 5 ),
                           sqlQuery->getColumn( 6 ),
                           sqlQuery->getColumn( 7 ),
                           sqlQuery->getColumn( 8 ) );
         }
      } else {
         mc2log << warn << "UserFavoritesList::addFromSQLQuery(): "
                << "number of columns in sqlQuery != 9 (is: "
                << sqlQuery->getNumColumns() << ")" << endl;
         nbrAdded = -1;
      }
   }

   return nbrAdded;
}


UserReplyPacket*
UserProcessor::handleUserFavoritesRequestPacket( const UserFavoritesRequestPacket* p )
{
   UserFavoritesReplyPacket* reply = new UserFavoritesReplyPacket(p);
   char query[65535];
   char* addQuery;
   char buf[2048];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   UserFavoritesList syncList;
   UserFavoritesList addList;
   UserFavoritesList deleteList;
   UserFavoritesList addReplyList;
   UserFavoritesList deleteReplyList;

   reply->setStatus( StringTable::NOTOK );
   int pos = USER_REQUEST_HEADER_SIZE;
   syncList.restore(p, pos, true);
   addList.restore(p, pos);
   deleteList.restore(p, pos, true);

   if ( !m_sqlConnection->beginTransaction()) {
      mc2log << error << "UP::handleChangeUserRequestPacket(): "
             << " beginTransaction() failed!" << endl;
      HANDLEUSERFAVORITESREQUESTPACKET_ABORT;
   }

   bool dontSync = ((syncList.size() == 1) && 
                   (syncList.front()->getID() == MAX_UINT32));
   
   UserFavoritesList::iterator iter;

   mc2dbg4 << "UP::handleUserFavoritesRequestPacket(): adding..." << endl;
   // add new favorites
   for (iter = addList.begin(); iter != addList.end(); iter++) {
      UserFavorite* fav = *iter;
      fav->setID( getNewUniqueID("ISABUserFavorites", "favID") );
      // Make infoStr and sqlesc it
      MC2String infoStr;
      UserFavorite::makeInfoStr( fav->getInfos(), infoStr );
      char infoBuff[ infoStr.size()*2 ];
      StringUtility::SQLEscape( infoStr.c_str(), infoBuff, 
                                infoStr.size()*2 );
      sprintf( query, "INSERT INTO ISABUserFavorites (userUIN, favID, "
               "lat, lon, name, shortName, category, description, "
               "mapIconName, infos ) "
               "VALUES (%u, %d, %d, %d, '%s', '%s', '%s', '%s', "
               "'%s', '%s' )",
               p->getUIN(), fav->getID(), fav->getLat(), fav->getLon(),
               StringUtility::SQLEscape( fav->getName(), buf, 255 ), 
               StringUtility::SQLEscape( fav->getShortName(), 
                                         buf + 260, 30 ),
               StringUtility::SQLEscape( fav->getCategory(), 
                                         buf + 300, 255 ),
               StringUtility::SQLEscape( fav->getDescription(), 
                                         buf + 600, 255 ),
               StringUtility::SQLEscape( fav->getMapIconName(), 
                                         buf + 900, 255 ), infoBuff );
      mc2dbg << "Query " << query << endl;
      if ( !doQuery(sqlQuery, query,
                "UP::handleUserFavoritesRequestPacket(): add fav")) {
         mc2log << error << "UP::handleUserFavoritesRequestPacket() "
                << " Couldn't add favorite!" << endl;
         HANDLEUSERFAVORITESREQUESTPACKET_ABORT;
      } else if (dontSync)
         addReplyList.addFavorite(new UserFavorite(fav));
   }

   mc2dbg4 << "UP::handleUserFavoritesRequestPacket(): deleting..." << endl;
   // delete favorites
   if ( !deleteList.empty() ) {
      const char* delstart = 
         "DELETE FROM ISABUserFavorites WHERE favID IN ( ";
      int queryPos = 0;
      strcpy( query, delstart );
      queryPos += strlen( delstart );
      int startPos = queryPos;
      for ( iter = deleteList.begin() ; iter != deleteList.end() ; iter++ )
      {
         queryPos += sprintf( query + queryPos, "%s%d", 
                              queryPos != startPos ? ", " : "",
                              (*iter)->getID() );
      }

      strcpy( query + queryPos, " )" );
  
      if ( !doQuery( sqlQuery, query,
                     "UP::handleUserFavoritesRequestPacket(): del fav" ) )
      {
         mc2log << error << "UP::handleUserFavoritesRequestPacket() "
                << "Couldn't delete favorites." << endl;
         HANDLEUSERFAVORITESREQUESTPACKET_ABORT;
      }
   
      if ( dontSync ) {
         // Add to delete list now as not syncing below
         for ( iter = deleteList.begin() ; iter != deleteList.end() ; 
               iter++ )
         {
            deleteReplyList.addFavorite( new UserFavorite( *iter, true ) );
         }
      }
   } // End if not empty deleteList

   mc2dbg4 << "UP::handleUserFavoritesRequestPacket(): syncing..." << endl;
   // sync favorites
   if (!dontSync) {
      sprintf(query, "SELECT favID FROM ISABUserFavorites "
                     "WHERE userUIN = %u", p->getUIN());
      if ( doQuery(sqlQuery, query, "UP::handleUserFavoritesRequest"
                                    "Packet(): get IDs (sync)") ) {
         UserFavoritesList serverList;
         if ( addFavoriteFromSQLQuery( sqlQuery, serverList, true ) == -1 )
         {
            mc2log << error << "UP::handleUserFavoritesRequestPacket() "
                   << " Problem when adding server IDs during sync" 
                   << endl;
            HANDLEUSERFAVORITESREQUESTPACKET_ABORT;
         }
         // sort the lists
         serverList.sortByID();
         syncList.sortByID();
         // syncing works like this:
         // go through them and remove every identical IDs from both lists
         // add any remaining in the client list to the deleteList
         // add any remaining in the server list to the addList
         // we do this in the same loop though...
         // instead of adding the IDs remaining in the server list we could
         // get the ones to add directly from the database (we have to run
         // that query for all the IDs to add anyway), something like:
         // "select * from ISABUserFavorites WHERE favID not IN
         // (518797341,5656666,the client ids...)"
         // But it only works for adding and we'd have to change the logic
         // here, probably wouldn't be faster.
         UserFavoritesList::iterator s = serverList.begin();
         UserFavoritesList::iterator c = syncList.begin();
         // the query we will use if there are any favorites to add
         // allocate the worst case size (ie everything in the server list
         // should be added, all IDs are 10 digits)
         addQuery = new char[256 + serverList.size() * 11];
         strcpy(addQuery, "SELECT favID, lat, lon, name, shortName, "
                       "description, category, mapIconName, infos "
                       "FROM ISABUserFavorites WHERE favID in (");
         int queryPos = strlen(addQuery);
         while (!(s == serverList.end() || c == syncList.end())) {
            uint32 sID = (*s)->getID();
            uint32 cID = (*c)->getID();
            if (sID == cID) { // identical, delete
               delete (*s);
               serverList.erase(s++);
               delete (*c);
               syncList.erase(c++);
            } else if (sID < cID) {  // s not in syncList (client list), add
               //addReplyList.addFavorite(*s);
               queryPos += sprintf(addQuery + queryPos, "%d,", sID);
               delete (*s);
               serverList.erase(s++);
            } else if (cID < sID) {  // c not in serverList, delete
               deleteReplyList.addFavorite(*c);
               syncList.erase(c++);
            }
         }
         // add any tails left in the lists
         while (s != serverList.end()) {
            //addReplyList.addFavorite(*s);
            queryPos += sprintf(addQuery + queryPos, "%d,", (*s)->getID());
            delete (*s);
            serverList.erase(s++);
         }
         while (c != syncList.end()) {
            deleteReplyList.addFavorite(*c);
            syncList.erase(c++);
         }
         // any favorites to add?
         if (addQuery[queryPos-1] != '(') {
            addQuery[queryPos-1] = ')';
            strcpy( addQuery + queryPos, " ORDER BY name" );
            if ( doQuery(sqlQuery, addQuery, "UP::handleUserFavoritesRequest"
                                          "Packet(): get adds (sync)") ) {
               addFavoriteFromSQLQuery( sqlQuery, addReplyList );
            } else {
               mc2log << error << "UP::handleUserFavoritesRequestPacket(): "
                         "Couldn't get favorite details for the additions."
                      << endl;
            }
         }
         delete[] addQuery;
      } else {
         mc2log << error << "UP::handleUserFavoritesRequestPacket(): couldn't"
                   " get IDs for syncing!" << endl;
      }
   }
   
   if ( !m_sqlConnection->commitTransaction()) {
      mc2log << error << "UP::handleUserFavoritesRequestPacket(): "
             << " commitTransaction() failed!" << endl;
      HANDLEUSERFAVORITESREQUESTPACKET_ABORT;
   }

   // if we get here everything is fine, insert contents into
   // reply packet and return
   pos = USER_REPLY_HEADER_SIZE;
   addReplyList.store(reply, pos);
   deleteReplyList.store(reply, pos, true);
   reply->setLength(pos);
   reply->setStatus( StringTable::OK );
   mc2dbg4 << "UP::handleUserFavoritesRequestPacket(): all done!" << endl;
   
   delete sqlQuery;
   return reply;
}

DebitReplyPacket* 
UserProcessor::handleDebitRequestPacket( const DebitRequestPacket* p ) {
   DebitReplyPacket* reply = NULL;

   m_debitSet.insert( 
      DebitData( p->getDate(), p->getUIN(), 
                 p->getServerID(), p->getUserOrigin(),
                 p->getMessageID(), p->getDebitInfo(),
                 p->getOperationType(), p->getSentSize(), 
                 p->getDescription() ) );

   if ( m_debitSet.size() >= 
        MIN( Properties::getUint32Property( "DEBIT_STORE_SIZE", 10 ), 75 )
        /*Or not add here at all and let handlePeriodic insert it, move 
          size check there then*/
#ifdef NO_MULTIPLE_INSERTS
        || true /*Only one record inserted at a time*/
#endif
        )
   {
      addStoredDebits();
   }
   reply = new DebitReplyPacket( p, StringTable::OK );

   if ( p->getNbrTransactions() != 0 ) {
      int32 changeNbrTransactions = -int32(p->getNbrTransactions());
      int32 nbrTransactions = 0;
      getAndChangeTransactions( p->getUIN(), changeNbrTransactions,
                                nbrTransactions );
   }
   return reply;
}
void
UserProcessor::addStoredDebits() {
   mc2dbg2 << "UP::addStoredDebits " << m_debitSet.size() << " debits" << endl;

   if ( !m_debitSet.empty() ) {
      char query[1024 + m_debitSet.size() * 1024];
      int pos = 0;
      CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

      strcpy( query, "INSERT INTO ISABDebit VALUES " );
      pos = strlen( query );

      for ( debitSet::const_iterator it = m_debitSet.begin() ;
            it != m_debitSet.end() ; ++it ) {
         if ( it != m_debitSet.begin() ) {
            strcpy( query + pos, ", " ); pos += 2;
         }
         strcpy( query + pos, "( " ); pos += 2;
         pos += addDebitData( query + pos, it->date, (*it).UIN, 
                              it->serverID.c_str(), it->userOrigin.c_str(),
                              it->messageID, it->debInfo, it->operationType,
                              it->sentSize, it->operationDescription.c_str() );
         strcpy( query + pos, " )" ); pos += 2;
      }
      
      if ( !doQuery(sqlQuery, query, "UP::addStoredDebits()")) {
         mc2log << error << "UP::addStoredDebits(): "
                << " insertion of new debit entries failed! Query " 
                << query << endl;
      }
      // Clearing here always even if error. If not it may lead
      // to doing it over and over again!
      m_debitSet.clear();

      delete sqlQuery;
   } // End if have any debits
}


GetCellularPhoneModelDataReplyPacket* 
UserProcessor::handleGetCellularPhoneModelsRequestPacket( 
   const GetCellularPhoneModelDataRequestPacket* p )
{
   GetCellularPhoneModelDataReplyPacket* reply = NULL; 
   CellularPhoneModel* cellular = NULL;
   uint32 nbrCellularModels = p->getNbrModels();
   if ( nbrCellularModels > 0 ) {
      cellular = p->getModel();
   }
   CellularPhoneModels* models =
      getCellularPhoneModels( nbrCellularModels, cellular );

   if ( models != NULL ) {
      reply = new GetCellularPhoneModelDataReplyPacket( p, models );
      delete models;
   } else {
      reply = new GetCellularPhoneModelDataReplyPacket( p, NULL );
      reply->setStatus( StringTable::NOTOK );
   }
   
   return reply;
}


ReplyPacket*
UserProcessor::handleAddCellularPhoneModelRequestPacket(
   const AddCellularPhoneModelRequestPacket* p )
{
   AddCellularPhoneModelReplyPacket* reply = NULL;
   char query[4096];
   
   CellularPhoneModel* model = p->getCellularPhoneModel();
   //Extract Cellularphonemodel
   if ( model == NULL || !model->getValid() ) {
      mc2log << error << "UP::handleAddCellularPhoneModelRequestPacket(): "
             << " No model to extract!" << endl;
      reply = new AddCellularPhoneModelReplyPacket( p, StringTable::NOTOK );
      delete model;
      return reply;
   }
   
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   strcpy ( query, "INSERT INTO ISABCellularModels VALUES ( ");
   uint32 pos = strlen( query );
   for ( uint32 i = 0; i < UserConstants::CELLULAR_MODEL_NBRFIELDS; i++ ) {
      pos += model->printValue( query + pos,
                                UserConstants::CellularModelField( i ) );
      strcpy( query + pos, ", " );
      pos += 2;
   }
   pos -= 2; //erases last ','
   strcpy( query + pos, " )" );
   
   if ( !doQuery(sqlQuery, query, "UP::handleAddPhoneModelRequestPacket()") ){
      mc2log << error << "UP::handleAddPhoneModelRequestPacket(): "
             << " Add of CellularPhoneModel failed!" << endl;
      reply = new AddCellularPhoneModelReplyPacket( p, 
                                                    StringTable::NOTOK );
      delete model;
      delete sqlQuery;
      return reply;
   }

   reply = new AddCellularPhoneModelReplyPacket( p, StringTable::OK );
   delete model;
   delete sqlQuery;
   return reply;
}  

bool UserProcessor::getLogonIDForUIN( CharEncSQLQuery* sqlQuery,
                                      uint32 UIN,
                                      MC2String& logonID ) {
   MC2String query( "SELECT logonID FROM ISABUserUser WHERE UIN = " + 
                    boost::lexical_cast<MC2String>( UIN ) );
   
   if (doQuery(sqlQuery, query.c_str(), "UP::getLogonIDForUIN get logon id") &&
         sqlQuery->nextRow() ) {
      logonID = sqlQuery->getColumn( 0 );
      return true;
   } else {
      logonID = "";
      return false;
   }
}
    
UserReplyPacket*
UserProcessor::handleChangeUserPasswordRequestPacket(
   const ChangeUserPasswordRequestPacket* p)
{
   char query[4096];
   uint32 UIN = p->getUIN();
   ChangeUserPasswordReplyPacket* reply = NULL;

   if ( strcmp(p->getUserPassword(), "") == 0 ) {
      mc2log << error << "UP::handleChangeUserPasswordRequestPacket(): "
             << " No password change! (empty password not allowed)" << endl;
      reply = new ChangeUserPasswordReplyPacket( p, false );
      reply->setStatus( StringTable::NOTOK );
      return reply;
   }
   
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   MC2String passwd =
      UserPassword::create( p->getUserPassword(),
                            boost::lexical_cast< MC2String >( UIN ).c_str() );

   sprintf(query, "UPDATE ISABUserUser SET logonPasswd = '%s'  WHERE UIN = %u",
           StringUtility::SQLEscape( passwd ).c_str(), UIN);
   
   if(!doQuery(sqlQuery, query, "UP::handleChangeUserPasswordRequestPacket()")){
      mc2log << error << "UP::handleChangeUserPasswordRequestPacket(): "
             << " password change failed!" << endl;
   
      reply = new ChangeUserPasswordReplyPacket( p, false );
      reply->setStatus( StringTable::NOTOK );
      delete sqlQuery;
      return reply;  
   }
   
   // remove from user login cache
   MC2String logonID;
   if ( getLogonIDForUIN( sqlQuery, UIN, logonID ) ) {
      MC2String logonIDLower = StringUtility::copyLower( logonID );
#ifdef PARALLEL_USERMODULE
      m_loginCache->remove( logonIDLower.c_str() );
#else
      m_userLoginCache->removeSession( logonIDLower.c_str() );
#endif
   }
   else {
      mc2log << error << "UP::handleChangeUserPasswordRequestPacket(): "
             << " get logonID failed, droping all cached logins" << endl;
#ifdef PARALLEL_USERMODULE
      m_loginCache->flush();
#else
      delete m_userLoginCache;
      m_userLoginCache = new UserSessionCache();      
#endif
   }

   reply = new ChangeUserPasswordReplyPacket( p, true );
   reply->setStatus( StringTable::OK );
   delete sqlQuery;
   return reply;
}


ReplyPacket*
UserProcessor::handleChangeCellularPhoneModelRequestPacket(
   const ChangeCellularPhoneModelRequestPacket* p )
{
   ChangeCellularPhoneModelReplyPacket* reply = NULL;
   char query[4096];
   uint32 nbrFields = 0;
   CellularPhoneModel* cellular = p->getCellularPhoneModel();
   
   //Extract Cellularphonemodel
   if ( cellular == NULL || !cellular->getValid() ) {
      mc2log << error << "UP::handleChangeCellularPhoneModelRequestPacket: "
             << " No model to extract!" << endl;
      reply = new ChangeCellularPhoneModelReplyPacket( p, StringTable::NOTOK );
      delete cellular;
      return reply;
   }
   
   uint32 pos = 0;
   const char* name = p->getName();
   char values[4096];
   for ( uint32 i = 0 ; i < UserConstants::CELLULAR_MODEL_NBRFIELDS ;
         i++ ) {
      if ( cellular->changed( UserConstants::CellularModelField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( values + pos, ", ");
            pos += 2;   
         }
         nbrFields++;
         strcpy( values + pos, 
                 UserConstants::CellularModelFieldName[ i ] );
         pos += strlen( UserConstants::CellularModelFieldName[ i ] );
         strcpy( values + pos, " = " );
         pos += 3;
         pos += cellular->printValue( values + pos, 
                                      UserConstants::CellularModelField( i ) );
      }
   }

   CharEncSQLQuery *sqlQuery = m_sqlConnection->newQuery();
   
   sprintf(query, 
           "UPDATE ISABCellularModels SET %s WHERE Name = '%s'",
           values, name);

   if(!doQuery(sqlQuery, query, "UP::handleChangePhoneModelRequestPacket()")) {
      mc2log << error << "UP::handleChangePhoneModelRequestPacket: "
             << " Change of CellularPhoneModel failed!" << endl;
      reply = new ChangeCellularPhoneModelReplyPacket( p, StringTable::NOTOK );
      delete cellular;
      delete sqlQuery;
      return reply;
   }
   
   reply = new ChangeCellularPhoneModelReplyPacket( p, StringTable::OK );
   delete cellular;
   delete sqlQuery;
   return reply;
}    


ReplyPacket* 
UserProcessor::handleVerifyUserRequestPacket( const VerifyUserRequestPacket* p) {
   VerifyUserReplyPacket* reply = NULL;
   uint32 UIN = 0;

   if ( !verifySession( p->getSessionID(), p->getSessionKey(), UIN,
                        true, p->getExpired() ) ) 
   {
      reply = new VerifyUserReplyPacket( p, 0 );
      reply->setStatus( StringTable::NOTOK );
   } else {
      reply = new VerifyUserReplyPacket( p, UIN );
   }

   return reply;
}


ReplyPacket* 
UserProcessor::handleLogoutUserRequestPacket( const LogoutUserRequestPacket* p) {
   LogoutUserReplyPacket* reply = NULL;

   char query[4096];
   const char* sessionID = NULL;
   const char* sessionKey = NULL;
   uint32 now = 0;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   sessionID = p->getSessionID();
   sessionKey = p->getSessionKey();

   sprintf(query, "SELECT * FROM ISABSession WHERE sessionID = '%s'", 
           sessionID);

   if ( ! doQuery(sqlQuery, query, "UP::handleLogoutUserRequestPacket()") ) {
      delete sqlQuery;
      mc2log << error <<  "UP::handleLogoutUserRequestPacket()"
             << " Selection of session failed." << endl;
      reply = new LogoutUserReplyPacket( p, 0 );
      reply->setStatus( StringTable::NOTOK );
      return reply;
   }

   if ( sqlQuery->nextRow()) {
      if ( strcmp( sqlQuery->getColumn(1), sessionKey ) == 0 )
      {
         uint32 UIN = strtoul( sqlQuery->getColumn(2), NULL, 10);

         now = TimeUtility::getRealTime();
         char tmp[20];
         sprintf( tmp, "%d", now );

         sprintf( query, "UPDATE ISABSession SET lastAccessTime = 0, "
                         "logoutTime = %s WHERE sessionID = '%s'",
                  tmp,
                  sessionID );
   
         if ( ! doQuery(sqlQuery, query,
                  "UP::handleLogoutUserRequestPacket update()") ) {
            mc2log << error << "UP::handleLogoutUserRequestPacket()"
                   << " Logout of session failed" << endl;
            reply = new LogoutUserReplyPacket( p, 0 );
            reply->setStatus( StringTable::NOTOK );
            delete sqlQuery;
            return reply;
         }
         reply = new LogoutUserReplyPacket( p, UIN );
         // Remove from session cache
#ifdef PARALLEL_USERMODULE
         m_sessionCache->remove( sessionID );
#else
         m_userSessionCache->removeSession( sessionID );
#endif
      } else {
         mc2dbg4 << "UP::handleLogoutUserRequestPacket(): " 
                << " SessionKey doesn't match for sessionID: "
                << sessionID << ", Key " << sessionKey << " != " <<
                sqlQuery->getColumn(1) << endl;
         reply = new LogoutUserReplyPacket( p, 0 );
      }
   } else {
      mc2dbg8 << "UP::handleLogoutUserRequestPacket(): " 
             << "No matches for sessionID " << sessionID << endl;
      reply = new LogoutUserReplyPacket( p, 0 );
   }

   delete sqlQuery;
   return reply;
}

#define HANDLESESSIONCLEANUPREQUESTPACKET_ABORT \
      if ( !m_sqlConnection->rollbackTransaction()) { \
         mc2log << error << "UP::handleChangeUserRequestPacket(): " \
                << " rollbackTransaction() failed!" << endl; \
      } \
      delete sqlQuery; \
      reply = new SessionCleanUpReplyPacket( p, StringTable::NOTOK ); \
      return reply;   

ReplyPacket* 
UserProcessor::handleSessionCleanUpRequestPacket(
   const SessionCleanUpRequestPacket* p)
{
   SessionCleanUpReplyPacket* reply = NULL;
   char query[4096];
   uint32 oldTime = 0;

   // Verify user
   uint32 UIN = 0;
   if ( !verifySession( p->getSessionID(), p->getSessionKey(), UIN, false ) )
   {
      mc2log << error << "UP::handleSessionCleanUpRequestPacket(): "
             << " verify session failed!" << endl;
      reply = new SessionCleanUpReplyPacket( p, StringTable::NOTOK );
      return reply;   
   }
   if ( UIN == 0 || UIN == MAX_UINT32 ) {
      mc2log << error << "UP::handleSessionCleanUpRequestPacket(): "
             << " invalid UIN!" << endl;
      reply = new SessionCleanUpReplyPacket( p, 
                                             StringTable::ACCESS_DENIED );
      return reply;
   }

   if ( !m_sqlConnection->beginTransaction() ) {
      mc2log << error << "UP::handleSessionCleanUpRequestPacket(): "
             << " beginTransaction failed!" << endl;   
      reply = new SessionCleanUpReplyPacket( p, StringTable::NOTOK );
      return reply;
   }

   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   // Copy all old sessions to history
   oldTime = sessionOldTime;
   sprintf( query, "INSERT INTO ISABSessionHistory SELECT * FROM ISABSession "
                   "WHERE lastAccessTime < %d", oldTime);
   if ( !doQuery(sqlQuery, query,
             "UP::handleSessionCleanUpRequestPacket(): insert") ) {
      mc2log << error << "UP::handleSessionCleanUpRequestPacket(): "
             << "Insert old sessions failed!" << endl;
      HANDLESESSIONCLEANUPREQUESTPACKET_ABORT;
   }

   // Delete old sessions
   sprintf(query, "DELETE FROM ISABSession WHERE lastAccessTime < %d",
           oldTime );
   if ( !doQuery(sqlQuery, query,
             "UP::handleSessionCleanUpRequestPacket(): delete") ) {
      mc2log << error << "UP::handleSessionCleanUpRequestPacket(): "
             << "Deletion of old sessions failed!" << endl;
      HANDLESESSIONCLEANUPREQUESTPACKET_ABORT;
   }
   
   // Done! Now commit...
   if ( !m_sqlConnection->commitTransaction() ) {
      mc2log << error << "UP::handleSessionCleanUpRequestPacket(): "
             << " commitTransaction() failed!" << endl;
      HANDLESESSIONCLEANUPREQUESTPACKET_ABORT;
   }
   
   reply =  new SessionCleanUpReplyPacket( p, StringTable::OK );

   delete sqlQuery;
   return reply;
}


ReplyPacket*
UserProcessor::handleListDebitRequestPacket( const ListDebitRequestPacket* p )
{

   mc2dbg8 << "handleListDebitRequestPacket p = " << p << endl;
   ListDebitReplyPacket* reply = NULL;
   char query[4096];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   uint32 endTime = 0;
   if ( p->getEndTime() == MAX_UINT32 || p->getEndTime() > MAX_INT32 ) {
      endTime = 2082675600; // 2035-12-31
   } else {
      endTime = p->getEndTime();
   }

   char fromDate[2000];
   char toDate[2000];
   UserUser::makeSQLOnlyDate( p->getStartTime(), fromDate );
   UserUser::makeSQLOnlyDate( endTime, toDate );
   mc2dbg2 << "UP::handleListDebitRequestPacket(): p->getStartTime(): "
           << p->getStartTime() << " = " << fromDate 
           << " endTime: " << endTime << " = " << toDate << endl;
   mc2dbg4 << "UP::handleListDebitRequestPacket(): p->getStartTime(): "
          << p->getStartTime() << " endTime: " << endTime << endl;
   mc2dbg4 << "UP::handleListDebitRequestPacket(): from/toDate : "
           << fromDate << " / " << toDate << endl;

   sprintf( query, "SELECT debitDate, debitTime, messageID, debitInfo, "
            "operationType, sentSize, "
            "userOrigin, serverID, operationDescription "
            "FROM ISABDebit "
            "WHERE debitUIN = '%u' AND debitDate BETWEEN %s AND %s "
            /* "ORDER BY debitDate, debitTime" */,
            p->getUIN(), fromDate, toDate );

   if ( !doQuery(sqlQuery, query, "UP::handleListDebitRequestPacket()") ) {
      mc2log << error <<"UP::handleListDebitRequestPacket(): aborting" 
             << endl;
      reply = new ListDebitReplyPacket( p, StringTable::NOTOK );
      delete sqlQuery;
      return reply;  
   } else {
      reply = new ListDebitReplyPacket( p, StringTable::OK );
      uint32 startIndex = p->getStartIndex();
      uint32 endIndex = p->getEndIndex();
      uint32 index = 0;

      // Make time of day
      char startDate[256];
      char endDate[256];
      char tmpTime[256];
      StringUtility::makeDateStr( p->getStartTime(), startDate, tmpTime );
      int32 startTimeOfDay = p->getStartTime() - StringUtility::makeDate( 
         startDate );
      StringUtility::makeDateStr( p->getEndTime(), endDate, tmpTime );
      int32 endTimeOfDay = p->getEndTime() - StringUtility::makeDate( 
         endDate );

      // Set first row
      bool hasRow = sqlQuery->nextRow();

      // Skipp until time of day is reached. BUT MUST BE START DAY!
      while ( hasRow && 
              (strcmp( sqlQuery->getColumn( 0 ), startDate ) < 0 ||
               (strcmp( sqlQuery->getColumn( 0 ), startDate ) == 0 &&
                atoi( sqlQuery->getColumn( 1 ) ) < startTimeOfDay ) ) )
      {
         hasRow = sqlQuery->nextRow();
      }

      // Skip until startIndex
      while ( hasRow && index < startIndex ) {
         // Skipp row
         hasRow = sqlQuery->nextRow();
         index++;
      }

      startIndex = index;

      while ( hasRow && index <= endIndex && 
              (strcmp( sqlQuery->getColumn( 0 ), endDate ) < 0 ||
               (strcmp( sqlQuery->getColumn( 0 ), endDate ) == 0 &&
                atoi( sqlQuery->getColumn( 1 ) ) <= endTimeOfDay ) ) )
      {
         uint32 time = StringUtility::makeDate( sqlQuery->getColumn( 0 ) );
         time += atoi( sqlQuery->getColumn( 1 ) );

         DebitElement el( atoi( sqlQuery->getColumn( 2 ) ),
                          atoi( sqlQuery->getColumn( 3 ) ),
                          time,
                          atoi( sqlQuery->getColumn( 4 ) ),
                          atoi( sqlQuery->getColumn( 5 ) ),
                          sqlQuery->getColumn( 6 ),
                          sqlQuery->getColumn( 7 ),
                          sqlQuery->getColumn( 8 ) );
         reply->addDebitElement( &el );
         index++;
         hasRow = sqlQuery->nextRow();
      }

      if ( !hasRow ) {
         // Add the not yet added ones in m_debitSet, 
         // sorted from oldest to newest
         debitSet::const_iterator debIt = m_debitSet.begin();
         while ( debIt != m_debitSet.end() &&
                 index <= endIndex && debIt->date <= p->getEndTime() &&
                 debIt->date >= p->getStartTime() ) {
            if ( debIt->UIN == p->getUIN() ) {
               DebitElement el( debIt->messageID,
                                debIt->debInfo,
                                debIt->date,
                                debIt->operationType,
                                debIt->sentSize,
                                debIt->userOrigin.c_str(),
                                debIt->serverID.c_str(),
                                debIt->operationDescription.c_str() );
               reply->addDebitElement( &el );
               ++index;
            }
            ++debIt;
         }
      }

      endIndex = MAX( 0, int32(index) - 1 );

      while ( hasRow && 
              (strcmp( sqlQuery->getColumn( 0 ), endDate ) < 0 ||
               (strcmp( sqlQuery->getColumn( 0 ), endDate ) == 0 &&
                atoi( sqlQuery->getColumn( 1 ) ) <= endTimeOfDay ) ) )
      {
         index++; 
         hasRow = sqlQuery->nextRow();
      }

      reply->setStartIndex( startIndex );
      reply->setEndIndex( endIndex );
      reply->setTotalNbrDebits( index );

      delete sqlQuery;
      return reply;
   }     
}


ReplyPacket* 
UserProcessor::handleAddUserNavDestinationRequestPacket( 
   const AddUserNavDestinationRequestPacket* p )
{
   AddUserNavDestinationReplyPacket* reply = NULL;
   CharEncSQLQuery *sqlQuery = m_sqlConnection->newQuery();
   DBUserNavDestination* nav = p->getUserNavDestination();

   // Make sure it is treated as a new
   nav->setID( MAX_UINT32 ); 

   if ( nav->insert( sqlQuery ) ) {
      reply = new AddUserNavDestinationReplyPacket( p, 
                                                    StringTable::OK );
   } else {
      reply = new AddUserNavDestinationReplyPacket( p, 
                                                    StringTable::NOTOK );
   }

   delete nav; 
   delete sqlQuery;
   return reply;
}


ReplyPacket* 
UserProcessor::handleDeleteUserNavDestinationRequestPacket( 
   const DeleteUserNavDestinationRequestPacket* p )
{
   DeleteUserNavDestinationReplyPacket* reply = NULL;
   CharEncSQLQuery *sqlQuery = m_sqlConnection->newQuery();
   DBUserNavDestination* nav = new DBUserNavDestination();
   
   nav->setID( p->getDeleteID() );
   if ( nav->remove( sqlQuery ) ) {
      reply = new DeleteUserNavDestinationReplyPacket( p,
                                                       StringTable::OK );
   } else {
      reply = new DeleteUserNavDestinationReplyPacket( 
         p, StringTable::NOTOK );
   }
   
   delete nav; 
   delete sqlQuery;
   return reply;
}


ReplyPacket* 
UserProcessor::handleChangeUserNavDestinationRequestPacket( 
   const ChangeUserNavDestinationRequestPacket* p )
{
   ChangeUserNavDestinationReplyPacket* reply = NULL;
   CharEncSQLQuery *sqlQuery = m_sqlConnection->newQuery();
   DBUserNavDestination* nav = p->getUserNavDestination();

   nav->setID( p->getNavID() );

   if ( nav->update( sqlQuery ) ) {
      reply = new ChangeUserNavDestinationReplyPacket( p,
                                                       StringTable::OK );
   } else {
      reply = new ChangeUserNavDestinationReplyPacket( 
         p, StringTable::NOTOK );
   }

   delete nav; 
   delete sqlQuery;
   return reply;
}


ReplyPacket* 
UserProcessor::handleGetUserNavDestinationRequestPacket( 
   const GetUserNavDestinationRequestPacket* p )
{
   GetUserNavDestinationReplyPacket* reply = NULL;
   bool onlyUnSentNavDestinations = p->getOnlyUnSentNavDestinations();
   bool onlyNavigatorLastContact = p->getOnlyNavigatorLastContact();
   uint32 navID = p->getNavID();
   const char* navAddress = p->getNavAddress();
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   char query[ 4096 ];
   char* pos = query;
   const char* str = NULL;

   pos += sprintf( pos, "SELECT %s.* FROM %s ",
                   DBUserNavDestination::getTableName(),
                   DBUserNavDestination::getTableName() );
   if ( onlyNavigatorLastContact ) 
   {
      // Use UserNavigator too
      strcpy( pos, ", " );
      pos += strlen( ", " );
      strcpy( pos, DBUserNavigator::getTableName() );
      pos += strlen( DBUserNavigator::getTableName() );
   }
   if ( onlyUnSentNavDestinations || 
        onlyNavigatorLastContact ||
        navID != 0 ||
        *navAddress != '\0' )
   {
      bool firstParam = true;
      if ( onlyUnSentNavDestinations ) {
         if ( firstParam ) {
            str = " WHERE ";
            strcpy( pos, str );
            pos += strlen( str );
         }
         str = DBUserNavDestination::getTableName();
         strcpy( pos, str );
         pos += strlen( str );
         str = ".";
         strcpy( pos, str );
         pos += strlen( str );
         str = DBUserNavDestination::m_fieldNames[ 
            DBUserNavDestination::field_sent ];
         strcpy( pos, str );
         pos += strlen( str );
         str = " = 'f' ";
         strcpy( pos, str );
         pos += strlen( str );
         firstParam = false;
      }
      if ( onlyNavigatorLastContact ) {
         if ( firstParam ) {
            str = " WHERE ";
            strcpy( pos, str );
            pos += strlen( str );
         } else {
            str = " AND ";
            strcpy( pos, str );
            pos += strlen( str ); 
         }
         str = DBUserNavigator::getTableName();
         strcpy( pos, str );
         pos += strlen( str );
         str = ".";
         strcpy( pos, str );
         pos += strlen( str );
         str = DBUserNavigator::m_fieldNames[ 
            DBUserNavigator::field_lastContactSuccess ];
         strcpy( pos, str );
         pos += strlen( str );
         str = " = 1 ";
         strcpy( pos, str );
         pos += strlen( str );
         firstParam = false;
      }
      if ( navID != 0 ) {
         if ( firstParam ) {
            str = " WHERE ";
            strcpy( pos, str );
            pos += strlen( str );
         } else {
            str = " AND ";
            strcpy( pos, str );
            pos += strlen( str ); 
         }
         str = DBUserNavDestination::getTableName();
         strcpy( pos, str );
         pos += strlen( str );
         str = ".";
         strcpy( pos, str );
         pos += strlen( str );
         str = DBUserNavDestination::m_fieldNames[ 
            DBUserNavDestination::field_navID ];
         strcpy( pos, str );
         pos += strlen( str );
         str = " = %d ";
         pos += sprintf( pos, str, navID );
         firstParam = false;
      }
      if ( *navAddress != '\0' ) {
         if ( firstParam ) {
            str = " WHERE ";
            strcpy( pos, str );
            pos += strlen( str );
         } else {
            str = " AND ";
            strcpy( pos, str );
            pos += strlen( str ); 
         }
         str = DBUserNavDestination::getTableName();
         strcpy( pos, str );
         pos += strlen( str );
         str = ".";
         strcpy( pos, str );
         pos += strlen( str );
         str = DBUserNavDestination::m_fieldNames[ 
            DBUserNavDestination::field_receiverAddress ];
         strcpy( pos, str );
         pos += strlen( str );
         str = " LIKE '%s' ";
         pos += sprintf( pos, str, navAddress );
         firstParam = false;
      }     
   }

   if ( onlyNavigatorLastContact ) 
   {
      // Add navID = navID
      str = " AND ";
      strcpy( pos, str );
      pos += strlen( str );
      str = DBUserNavigator::getTableName();
      strcpy( pos, str );
      pos += strlen( str );
      str = ".";
      strcpy( pos, str );
      pos += strlen( str );
      str = DBUserNavigator::m_fieldNames[ 
         DBUserNavigator::field_ID ];
      strcpy( pos, str );
      pos += strlen( str );
      str = " = ";
      strcpy( pos, str );
      pos += strlen( str );
      str = DBUserNavDestination::getTableName();
      strcpy( pos, str );
      pos += strlen( str );
      str = ".";
      strcpy( pos, str );
      pos += strlen( str );
      str = DBUserNavDestination::m_fieldNames[ 
         DBUserNavDestination::field_navID ];
      strcpy( pos, str );
      pos += strlen( str );
   }

   if ( !doQuery(sqlQuery, query,
              "UP::handleGetUserNavDestinationRequestPacket()")) {
      mc2log << error << "UP::handleGetUserNavDestinationRequestPacket(): "
                 "Failed, returning NOTOK" << endl;
      reply = new GetUserNavDestinationReplyPacket( p, StringTable::NOTOK );
      delete sqlQuery;
      return reply;
   }

   while (sqlQuery->nextRow() ) {
      if (NULL == reply)
         reply = new GetUserNavDestinationReplyPacket( p, StringTable::OK );

      // Extract DBUserNavDestinations and add to packet.
      DBUserNavDestination* nav = new DBUserNavDestination( sqlQuery );
      while ( !reply->addUserNavDestination( nav ) ) {
         mc2dbg8 << "Resizing GetUserNavDestinationReplyPacket from: "
                 << p->getBufSize() << " to " << p->getBufSize()*2 << endl;
         reply->resize( p->getBufSize()*2 );
      }
      delete nav;
      mc2dbg8 << "UP::handleGetUserNavDestinationRequestPacket(): "
              << " Resulting packet length: " << reply->getLength()
              << ", bufSize: " << reply->getBufSize() << endl;
   }

   delete sqlQuery;
   return reply;
}


UserReplyPacket* 
UserProcessor::handleAuthUserRequestPacket( const AuthUserRequestPacket* p ) {
   AuthUserReplyPacket* reply = NULL;
   char logonID[256];

   StringUtility::SQLEscapeSecure(p->getLogonID(), logonID, 255);

   mc2dbg4 << "UP::handleAuthUserRequestPacket "
           << "LogonID: #" << logonID << "#" << endl;

   mc2dbg4 << "UP::handleAuthUserRequestPacket "
           << "Password: #" << "*********" << "#" << endl;
   
   uint32 UIN = getUser( logonID, p->getPassword(), p->getExpired() );

   reply = new AuthUserReplyPacket( p, UIN );
   reply->setStatus( StringTable::OK );
   return reply;
}   



ReplyPacket* 
UserProcessor::handleRouteStorageAddRouteRequestPacket( 
   const RouteStorageAddRouteRequestPacket* p )
{
   RouteStorageAddRouteReplyPacket* reply = NULL;

   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   auto_ptr<RouteReplyPacket> routePack(p->getRoutePacket());
   uint32 routePackLength = routePack->getLength();
   char query[4096 + routePackLength * 4/3];
   bool ok = true;
   uint32 pos = 0;

   if ( !m_sqlConnection->beginTransaction() ) {
      mc2log << error << "UP::handleRouteStorageAddRouteRequestPacket(): "
             << "beginTransaction() failed" << endl;
      reply = new RouteStorageAddRouteReplyPacket( p, StringTable::NOTOK );
      delete sqlQuery;
      return reply;
   }

   if ( !p->getIsReRoute() ) {
      // ISABRouteStorageCoords
      DriverPref driverPref;
      p->getDriverPrefs( driverPref );
      uint32 driverFlags = 0;
      // useUturn
      BitUtility::setBit( driverFlags, 0, driverPref.useUturn() );
      // avoidTollRoads
      BitUtility::setBit( driverFlags, 1, driverPref.avoidTollRoads() );
      // avoidHighways
      BitUtility::setBit( driverFlags, 2, driverPref.avoidHighways() );
      // IsStartTime
      BitUtility::setBit( driverFlags, 3, driverPref.getIsStartTime() );
   
   
      pos = sprintf( query, "INSERT INTO ISABRouteStorageCoords VALUES ( "
                     "%d, %d, %u, " // routeID, createTime, userUIN
                     "%d, "    // validUntil
                     "%d, %d, %hd, " // originCoords + angle
                     "%d, %d, " // destinationCoords
                     "%d, " // routeCosts
                     "%d, " // vehicle
                     "%d, " // time
                     "%d, " // minWaitTime
                     "%d, " // driverFlags
                     "%lld ) ", // urmask
                     p->getRouteID(), p->getCreateTime(), p->getUIN(),
                     int32(p->getValidUntil()),
                     p->getOriginLat(), p->getOriginLon(), p->getOriginAngle(),
                     p->getDestinationLat(), p->getDestinationLon(),
                     driverPref.getRoutingCosts(), 
                     driverPref.getVehicleRestriction(),
                     driverPref.getTime(),
                     driverPref.getMinWaitTime(),
                     driverFlags, 
                     static_cast<int64> ( p->getUrmask().getAsInt() ) );

      if ( ok && !doQuery( sqlQuery, query,
                           "UP::handleRouteStorageAddRouteRequestPacket()" ) )
      {
         mc2log << error << "UP::handleRouteStorageAddRouteRequestPacket(): "
                << " Add of route coords failed!" << endl; 
      }
   } // End if not reroute of old stored route

   const char* preFix = "";
   if ( Properties::getProperty( "ROUTE_STORAGE_PATH" ) == NULL ) {
      preFix = "INSERT INTO ISABRouteStorage VALUES ";
   }
   ok = makeRouteStorageString( 
      query, preFix, routePack.get(),
      p->getRouteID(),
      p->getCreateTime(),
      p->getUIN(),
      p->getExtraUserinfo(),
      int32(p->getValidUntil()),
      p->getOriginLat(), p->getOriginLon(), 
      p->getOriginMapID(), p->getOriginItemID(), 
      p->getOriginOffset(), 
      p->getDestinationLat(), p->getDestinationLon(), 
      p->getDestinationMapID(), p->getDestinationItemID(), 
      p->getDestinationOffset() );

   if ( Properties::getProperty( "ROUTE_STORAGE_PATH" ) != NULL ) {
      // Store on disc
      if ( ok ) {
         MC2String fileName = makeRouteStoragePath( p->getRouteID(),
                                                    p->getCreateTime() );
         if ( !fileName.empty() ) {
            int res = File::writeFile( fileName.c_str(), 
                                       reinterpret_cast<byte*>( query ), 
                                       strlen( query ) );
            if ( res <= 0 ) {
               ok = false;
            }
         } else {
            ok = false;
         }
      }
   } else { // Use Sql storage
      // ISABRouteStorage
      if ( ok && !doQuery(sqlQuery, query,
                          "UP::handleRouteStorageAddRouteRequestPacket()") ) {
         mc2log << error << "UP::handleRouteStorageAddRouteRequestPacket(): "
                << " Add of route failed!" << endl; 
         ok = false;
      }
   } // End else use sql storage

   if ( ok && !m_sqlConnection->commitTransaction()) {
      mc2log << error << "UP::handleRouteStorageAddRouteRequestPacket(): "
             << " commitTransaction() failed!" << endl;
      ok = false;
   }

   if ( !ok ) {
      if ( !m_sqlConnection->rollbackTransaction()) {
         mc2log << error << "UP::handleRouteStorageAddRouteRequestPacket "
                << " rollbackTransaction failed!" << endl;
      }
      reply = new RouteStorageAddRouteReplyPacket( p, StringTable::NOTOK );
   } else {
      reply = new RouteStorageAddRouteReplyPacket( p, StringTable::OK );
   }
  
   delete sqlQuery;
   return reply;
}

ReplyPacket* 
UserProcessor::handleRouteStorageGetRouteRequestPacket( 
   const RouteStorageGetRouteRequestPacket* p )
{
   RouteStorageGetRouteReplyPacket* reply = NULL;
   DriverPref driverPref;
   UserEnums::URType urmask;
   bool ok = true;
   byte* routeBuf = NULL;
   char query[ 4096 ];
   bool fatalError = false;
   uint32 routeID;
   uint32 createTime;
   uint32 UIN;
   char extraUserinfo[ 256 ];
   uint32 validUntil;
   int32 originLat;
   int32 originLon;
   uint32 originMapID;
   uint32 originItemID;
   uint16 originOffset;
   int32 destinationLat;
   int32 destinationLon;
   uint32 destinationMapID;
   uint32 destinationItemID;
   uint16 destinationOffset;
   uint32 routePacketLength;
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );


   if ( Properties::getProperty( "ROUTE_STORAGE_PATH" ) != NULL ) {
      // Get from disc
      MC2String fileName = makeRouteStoragePath( p->getRouteID(),
                                                 p->getCreateTime() );
      
      if ( !fileName.empty() ) {
         vector<byte> buff;
         int res = File::readFile( fileName.c_str(), buff );

         if ( res > 0 ) {
            buff.push_back( 0 );
            char routeBuff[ buff.size() ];
            const char* buffStr = reinterpret_cast< char* > ( &buff.front() );

            int sres = sscanf( buffStr, "( %d, %d, "
                               "%u, %d, '%[^']', %d, "
                               "%d, %d, %d, %d, %hd, "
                               "%d, %d, %d, %d, %hd, '%[^']')",
                               &routeID, &createTime, &UIN, &validUntil,
                               extraUserinfo, &routePacketLength,
                               &originLat, &originLon, 
                               &originMapID, &originItemID, &originOffset,
                               &destinationLat, &destinationLon, 
                               &destinationMapID, &destinationItemID, 
                               &destinationOffset, routeBuff );
            if ( sres == 4 ) {
               // Empty extraUserinfo '' ?
               sres = sscanf( buffStr, "( %d, %d, "
                              "%u, %d, '', %d, "
                              "%d, %d, %d, %d, %hd, "
                              "%d, %d, %d, %d, %hd, '%[^']')",
                              &routeID, &createTime, &UIN, &validUntil,
                              &routePacketLength,
                              &originLat, &originLon, 
                              &originMapID, &originItemID, &originOffset,
                              &destinationLat, &destinationLon, 
                              &destinationMapID, &destinationItemID, 
                              &destinationOffset, routeBuff );
               if ( sres == 16 ) {
                  extraUserinfo[ 0 ] = '\0';
                  sres = 17;
               }
            }

            if ( sres == 17 ) {
               routeBuf = storedRouteToBuffer( 
                  routeBuff, routePacketLength,
                  routeID, createTime );
               ok = routeBuf != NULL;
            } else {
               mc2log << warn 
                      << "UP::handleRouteStorageGetRouteRequestPacket() "
                      << "Bad route data in stored file. sres " << sres
                      << " Data: " << MC2CITE( buffStr ) << endl;
               ok = false;
            }
         } else { // Else read file failed
            ok = false;
         }
      } else { // No fileName bad ROUTE_STORAGE_PATH
         ok = false;
         fatalError = true;
      }
   } else {
      // SQL storage
      sprintf( query, "SELECT * FROM ISABRouteStorage "
               "WHERE routeID = %d AND createTime = %d",
               p->getRouteID(),
               p->getCreateTime() );

      if ( ! doQuery( sqlQuery.get(), query, 
                      "UP::handleRouteStorageGetRouteRequestPacket()" ) ) {
         mc2log << error << "UP::handleRouteStorageGetRouteRequestPacket(): "
                << " attempt to get route from db failed!" << endl;
         ok = false;
         fatalError = true;
      }

      if ( ok && sqlQuery->nextRow() ) {
         routeID             = atoi( sqlQuery->getColumn(  0 ) );
         createTime          = atoi( sqlQuery->getColumn(  1 ) );
         UIN                 = strtoul(sqlQuery->getColumn(2 ), NULL, 10 );
         validUntil          = atoi( sqlQuery->getColumn(  3 ) );
         strcpy( extraUserinfo,      sqlQuery->getColumn(  4 ) );
         routePacketLength   = atoi( sqlQuery->getColumn(  5 ) );
         originLat           = atoi( sqlQuery->getColumn(  6 ) );
         originLon           = atoi( sqlQuery->getColumn(  7 ) );
         originMapID         = atoi( sqlQuery->getColumn(  8 ) );
         originItemID        = atoi( sqlQuery->getColumn(  9 ) );
         originOffset        = atoi( sqlQuery->getColumn( 10) );
         destinationLat      = atoi( sqlQuery->getColumn( 11) );
         destinationLon      = atoi( sqlQuery->getColumn( 12) );
         destinationMapID    = atoi( sqlQuery->getColumn( 13) );
         destinationItemID   = atoi( sqlQuery->getColumn( 14) );
         destinationOffset   = atoi( sqlQuery->getColumn( 15) );
         //      Fill in driverPref, urmask and originAngle, but this whould
         //      require either change of ISABRouteStorage or selecting from
         //      ISABRouteStorageCoords.
         //      They should not be used if there already is a route so
         //      let then be empty.
         routeBuf = storedRouteToBuffer( sqlQuery->getRawColumn( 16 ),
                                         routePacketLength,
                                         routeID, createTime );
         ok = routeBuf != NULL;
      } else {
         ok = false;
      }
   } // End else sql storage

   if ( ok ) {
      RouteReplyPacket* routePack = static_cast< RouteReplyPacket* >( 
         new Packet( routeBuf, routePacketLength ) );
      
      reply = new RouteStorageGetRouteReplyPacket( 
         p, 
         StringTable::OK,
         routePack,
         routeID,
         UIN,
         extraUserinfo,
         validUntil,
         createTime,
         originLat,
         originLon,
         MAX_UINT16/*originAngle*/,
         originMapID,
         originItemID,
         originOffset,
         destinationLat,
         destinationLon,
         destinationMapID,
         destinationItemID,
         destinationOffset,
         driverPref,
         urmask );
      delete routePack;
   } else if ( !fatalError ) {
      ok = true;
      delete [] routeBuf;
      mc2log << warn << "UP::handleRouteStorageGetRouteRequestPacket(): "
             << " No stored route with routeID: " << p->getRouteID()
             << ", createTime: " << p->getCreateTime() << endl;
      // Try ISABRouteStorageCoords
      sprintf( query, "SELECT routeID, createTime, userUIN, validUntil, "
               "originLat, originLon, originAngle, "
               "destinationLat, destinationLon, "
               "routeCosts, vehicle, time, minWaitTime, "
               "driverFlags, urmask "
               " FROM ISABRouteStorageCoords "
               "WHERE routeID = %d AND createTime = %d",
               p->getRouteID(),
               p->getCreateTime() );

      if ( ! doQuery( sqlQuery.get(), query, 
                      "UP::handleRouteStorageGetRouteRequestPacket()" ) ) {
         mc2log << error << "UP::handleRouteStorageGetRouteRequestPacket(): "
                << " attempt to get route coords from db failed!" << endl;
         ok = false;
      }

      if ( ok && sqlQuery->nextRow() ) {
         uint32 driverFlags     = atoi( sqlQuery->getColumn( 13 ) );
         driverPref.setUturn( BitUtility::getBit( driverFlags, 0 ) );
         driverPref.setAvoidTollRoads( BitUtility::getBit( driverFlags, 1 ) );
         driverPref.setAvoidHighways( BitUtility::getBit( driverFlags, 2 ) );
         driverPref.setIsStartTime( BitUtility::getBit( driverFlags, 3 ) );
         driverPref.setRoutingCosts( atoi( sqlQuery->getColumn( 9 ) ) );
         driverPref.setVehicleRestriction( atoi( sqlQuery->getColumn( 10 ) ) );
         driverPref.setTime( atoi( sqlQuery->getColumn( 11 ) ) );
         driverPref.setMinWaitTime( atoi( sqlQuery->getColumn( 12 ) ) );

         reply = new RouteStorageGetRouteReplyPacket( 
            p, 
            StringTable::OK,
            NULL/*routePack*/,
            atoi( sqlQuery->getColumn( 0 ) )/*routeID*/,
            strtoul(sqlQuery->getColumn(2), NULL, 10)/*UIN*/,
            ""/*extraUserinfo*/,
            atoi( sqlQuery->getColumn( 3 ) )/*validUntil*/,
            atoi( sqlQuery->getColumn( 1 ) )/*createTime*/,
            atoi( sqlQuery->getColumn( 4 ) )/*originLat*/,
            atoi( sqlQuery->getColumn( 5 ) )/*originLon*/,
            atoi( sqlQuery->getColumn( 6 ) )/*originAngle*/,
            MAX_UINT32/*originMapID*/,
            MAX_UINT32/*originItemID*/,
            MAX_UINT16/*originOffset*/,
            atoi( sqlQuery->getColumn( 7 ) )/*destinationLat*/,
            atoi( sqlQuery->getColumn( 8 ) )/*destinationLon*/,
            MAX_UINT32/*destinationMapID*/,
            MAX_UINT32/*destinationItemID*/,
            MAX_UINT16/*destinationOffset*/,
            driverPref,
            UserEnums::URType( strtoll( sqlQuery->getColumn( 14 ), 
                                        NULL, 10 ) )/*urmask*/ );
      } else {
         mc2log << warn << "UP::handleRouteStorageGetRouteRequestPacket(): "
                << " No stored route coords with routeID: " 
                << p->getRouteID() << ", createTime: " << p->getCreateTime()
                << endl;
         ok = false;
      }
   
   } else {
      mc2log << warn << "UP::handleRouteStorageGetRouteRequestPacket(): "
             << " No stored route with routeID: " 
             << p->getRouteID() << ", createTime: " << p->getCreateTime()
             << endl;
      ok = false;
   }

   if ( !ok ) {
      reply = new RouteStorageGetRouteReplyPacket( p, 
                                                   StringTable::NOTOK,
                                                   NULL,
                                                   0,
                                                   0,
                                                   NULL,
                                                   0,
                                                   0,
                                                   0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 
                                                   driverPref, urmask );
   }
   
   return reply;
}


ReplyPacket*
UserProcessor::handleCreateSessionRequestPacket(
   const CreateSessionRequestPacket* p )
{
   CreateSessionReplyPacket* reply = NULL;

   uint32 UIN = p->getUIN();

   if ( UIN != 0 ) {
      char id[31];
      char key[31];
      uint32 now = TimeUtility::getRealTime();
      if ( makeSession( UIN, now, id, key ) ) {
         reply = new CreateSessionReplyPacket( p, StringTable::OK,
                                               id, key );
      } else {
         reply = new CreateSessionReplyPacket( p,
                                               StringTable::NOTOK,
                                               "", "" ); 
      }
   } else {
      reply = new CreateSessionReplyPacket( p,
                                            StringTable::NOTFOUND,
                                            "", "" );
   }

   return reply;
}
ReplyPacket*
UserProcessor::handleMapUpdateRequestPacket(
   const MapUpdateRequestPacket* p )
{
   CharEncSQLQuery *sqlQuery = m_sqlConnection->newQuery();

   // Get the DBase object 
   int pos = p->getNextDBaseObjectPosition();

   MapUpdateDBaseObject* mapUpdateDBaseObject = 
      new MapUpdateDBaseObject( p, pos );

   // Add to the database
   StringTable::stringCode status = StringTable::OK;
   if (!mapUpdateDBaseObject->insert(sqlQuery) || 
       !doQuery(sqlQuery, NULL, "UP::handleMapUpdateRequestPacket()") ) {
      mc2log << error << "UP::handleMapUpdateRequestPacket(): failed!" << endl;
      status = StringTable::NOTOK;
   }

   delete sqlQuery;

   // Create the reply-packet
   return new MapUpdateReplyPacket( p, status );
}


ReplyPacket*
UserProcessor::handleAddUserTrackRequestPacket( const AddUserTrackRequestPacket* p)
{
   // Insert into database
   UserTrackElementsList trackElements;
   StringTable::stringCode status = StringTable::OK;
   p->getUserTrackElements(trackElements);
   uint32 UIN = p->getUIN();
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   const uint32 multiInsertSize = 100;
   uint32 nbrInserts = 0;
   UserTrackElementsList::iterator prev = trackElements.begin();
   char query[ 1024 + multiInsertSize * 1024 ];
   int pos = 0;
   strcpy( query, "INSERT INTO ISABUserTrackPoint "
           "(userUIN, lat, lon, dist, speed, heading, trackTime, source)"
           " VALUES " );
   pos = strlen( query );

   m_sqlConnection->beginTransaction();

   for ( UserTrackElementsList::iterator i = trackElements.begin() ; 
         i != trackElements.end() ; ++i ) {

      if ( i != prev && (*prev)->getTime() == (*i)->getTime() ) {
         // Same time skipp this
         mc2log << warn << "UP:AddUserTP Same time for two trackpoints "
                << "skipping this!" << endl;
         continue;
      }

      if ( nbrInserts != 0 ) {
         strcpy( query + pos, ", " ); pos += 2;
      }
      // Insert into relation in sql-database
      pos += sprintf( query + pos ,
                      "(%u,%d,%d,%d,%hd,%hd,%u,'%s')",
                      UIN,
                      (*i)->getLat(), 
                      (*i)->getLon(), 
                      (*i)->getDist(),
                      (*i)->getSpeed(),
                      (*i)->getHeading(),
                      (*i)->getTime(), 
                      (*i)->getSource() );
      
      nbrInserts++;
      if ( nbrInserts >= multiInsertSize ) {
         if ( ! doQuery( sqlQuery, query, "UP::handleAddUserTrack()" ) ) {
            mc2log << error << "UP:AddUserTP Failed to add track points "
                   << "to DB, " << query << endl;
            // Skipping error for now, user has to suffer...
            //status = StringTable::INTERNAL_SERVER_ERROR;
         }

         strcpy( query, "INSERT INTO ISABUserTrackPoint "
                 "(userUIN, lat, lon, dist, speed, heading, trackTime, source)"
                 " VALUES " );
         pos = strlen( query );
         nbrInserts = 0;
      }

      prev = i;
   }

   if ( nbrInserts > 0 ) {
      if ( ! doQuery( sqlQuery, query, "UP::handleAddUserTrack()" ) ) {
         mc2log << error << "UP:AddUserTP Failed to add last track points "
                << "to DB, " << query << endl;
         // Skipping error for now, user has to suffer...
         //status = StringTable::INTERNAL_SERVER_ERROR;
      }
   }

   if ( status == StringTable::OK ) {
      m_sqlConnection->commitTransaction();
   } else {
      m_sqlConnection->rollbackTransaction();
   }

   delete sqlQuery;

   // Create reply and set status
   AddUserTrackReplyPacket* reply = new AddUserTrackReplyPacket(p);
   reply->setStatus(StringTable::OK);
   return reply;
}

ReplyPacket*
UserProcessor::handleGetUserTrackRequestPacket( const GetUserTrackRequestPacket* p)
{
   // Get interval and/or max nbr hits
   uint32 startTime = p->getLowerInterval();
   uint32 endTime = p->getHigherInterval();
   uint32 maxNbrHits = p->getMaxNbrHits();
   mc2dbg2 << info << "Extracting track: " << startTime << "-" << endTime
           << ", maxNbrHits=" << maxNbrHits << endl;
   
   // Get from database into trackElements
   UserTrackElementsList trackElements;
   StringTable::stringCode status = StringTable::OK;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   char query[1024];
   sprintf(query,
           "SELECT lat, lon, dist, speed, heading, trackTime, source "
           "FROM ISABUserTrackPoint "
           "WHERE userUIN=%u AND source='Track' AND " // Only Track!
           "trackTime >= %u AND trackTime <= %u ORDER BY trackTime DESC;",
           p->getUIN(), startTime, endTime );
   mc2dbg4 << "UP:getUsertrack: " << query << endl;

   uint32 n=0;
   if ( doQuery(sqlQuery, query, "UP::handleGetUserTrack()") ) {
      while ( (sqlQuery->nextRow()) && (n++ < maxNbrHits)){
         //  Got one track point, data:
         //  userUIN, lat, lon, trackTime, source
         int32 lat = atoi( sqlQuery->getColumn(0) );
         int32 lon = atoi( sqlQuery->getColumn(1) );
         uint32 dist = atoi( sqlQuery->getColumn( 2 ) );
         uint16 speed = atoi( sqlQuery->getColumn( 3 ) );
         uint16 heading = atoi( sqlQuery->getColumn( 4 ) );
         uint32 trackTime = atoi( sqlQuery->getColumn(5) );
         const char* source = sqlQuery->getColumn(6);
         trackElements.push_back( 
            new UserTrackElement(
               lat, lon, dist, speed, heading, trackTime, source ) );
#ifdef DEBUG_LEVEL_2
         mc2dbg2 << "UP:getUserTP "; trackElements.back()->dump( mc2log );
         mc2dbg2 << endl;
#endif
      }
   } else {
      status = StringTable::DATA_BASE_ERROR_PLEASE_TRY_AGAIN_LATER;
   }

   if (n > maxNbrHits) {
      mc2dbg2 << "UP:getUserTrackPoint " << (n-maxNbrHits) 
              << " more hits in db." << endl;
   }

   delete sqlQuery;

   // Create reply and store list
   GetUserTrackReplyPacket* reply = new GetUserTrackReplyPacket(p);
   reply->setUserTrackElements(trackElements);
   reply->setStatus(status);
   return reply;
}


ReplyPacket* 
UserProcessor::handleTransactionRequestPacket( 
   const TransactionRequestPacket* p )
{
   TransactionReplyPacket* reply = NULL;  

   int32 nbrTransactions = 0;
   int32 transactionChange = 0;
   if ( p->getAction() == TransactionRequestPacket::increase ) {
      transactionChange = p->getNbrTransactions(); 
   } else if ( p->getAction() == TransactionRequestPacket::decrease ) {
      transactionChange = -int32(p->getNbrTransactions());
   }

   if ( !getAndChangeTransactions( p->getUIN(), transactionChange, 
                                   nbrTransactions ) )
   {
      reply = new TransactionReplyPacket( p, StringTable::NOTOK, 0 );
   } else {
      reply = new TransactionReplyPacket( p, StringTable::OK, 
                                          nbrTransactions );
   }

   return reply;
}


ReplyPacket* 
UserProcessor::handleTransactionDaysRequestPacket( 
   const TransactionDaysRequestPacket* p )
{
   TransactionDaysReplyPacket* reply = NULL;  

   int32 nbrTransactionDays = p->getNbrTransactionDays();
   uint32 curTime = 0;

   StringTable::stringCode status = getAndCheckTransactionDays(
      p->getUIN(), p->getCheck(), nbrTransactionDays, curTime );
   
   reply = new TransactionDaysReplyPacket( 
      p, status, nbrTransactionDays, curTime );

   return reply;   
}

uint32
UserProcessor::gradePoi( const PoiReviewItem& poi, const PoiReviewDetail& d,
                         CharEncSQLQuery* sqlQuery ) {
   uint32 status = StringTableUTF8::OK;
   char query[ 4096 ];

   // Check if user already voted on poi
   sprintf( query, "SELECT userUIN FROM ISABPoiGradeUser "
            "WHERE id = %s AND userUIN = %u", poi.getPoiID().c_str(), 
            d.getOwnerUIN() );
   if ( ! doQuery( sqlQuery, query, 
                   "UP::handlePoiReviewAdd check vote" ) ) 
   {
      mc2log << error 
             << "UP::handlePoiReviewAdd(): "
             << " check vote failed! Query \"" 
             << query << "\"" << endl;
      status = StringTableUTF8::NOTOK;
   } else {
      if ( sqlQuery->nextRow() ) {
         // Bad, user! You may not vote multiple times!
         status = StringTableUTF8::NOT_ALLOWED;
      } else {
         // Add user to voted
         sprintf( query, "INSERT INTO ISABPoiGradeUser "
                  "( id, userUIN ) VALUES "
                  "( %s, %u )",
                  poi.getPoiID().c_str(), d.getOwnerUIN() );
         if ( ! doQuery( sqlQuery, query, 
                         "UP::handlePoiReviewAdd add voted" ) ) 
         {
            mc2log << error 
                   << "UP::handlePoiReviewAdd(): "
                   << " add voted failed! Query \"" 
                   << query << "\"" << endl;
            status = StringTableUTF8::NOTOK;
         }
      } // End if not already voted
   }
   
   if ( status == StringTableUTF8::OK ) {
      // get present values for poi ISABPoiGrade
      sprintf( query, "SELECT totalGrade, totalVote FROM ISABPoiGrade "
               "WHERE id = %s", poi.getPoiID().c_str() );
      if ( ! doQuery( sqlQuery, query, 
                      "UP::handlePoiReviewAdd get grade" ) ) 
      {
         mc2log << error 
                << "UP::handlePoiReviewAdd(): "
                   << " get grade failed! Query \"" 
                << query << "\"" << endl;
         status = StringTableUTF8::NOTOK;
      } else {
         // Get entry
         // If none then inset else modify (+= this) query
         if ( sqlQuery->nextRow() ) {
            uint32 totalGrade = atoi( sqlQuery->getColumn( 0 ) );
            uint32 totalVote  = atoi( sqlQuery->getColumn( 1 ) );
            totalGrade += d.getGrade();
            ++totalVote;
            sprintf( query, "UPDATE ISABPoiGrade SET "
                     "totalGrade = %d, "
                     "totalVote = %d WHERE id = %s", 
                     totalGrade, totalVote, poi.getPoiID().c_str() );
         } else {
            // Add new
            uint32 ID = getNewUniqueID( "ISABPoiGrade", "id" );
            if ( ID == 0 ) {
               mc2log << error << "UP::handlePoiReviewAdd(): "
                      << "getNewUniqueID(ISABPoiGrade) returned 0, "
                      << "aborting!"  << endl;
               status = StringTableUTF8::NOTOK;
            } else {
               sprintf( query, "INSERT INTO ISABPoiGrade "
                        "(id, totalGrade, totalVote ) VALUES "
                        "( %s, %d, %d )",
                        poi.getPoiID().c_str(), d.getGrade(), 
                        1/*This vote*/ );
            }
         } // End else has entry for poi

         if ( status == StringTableUTF8::OK ) {
            if ( ! doQuery( sqlQuery, query, 
                            "UP::handlePoiReviewAdd add grade" ) ) 
            {
               mc2log << error 
                      << "UP::handlePoiReviewAdd(): "
                      << " add grade failed! Query \"" 
                      << query << "\"" << endl;
               status = StringTableUTF8::NOTOK;
            }
         }
      } // End else query succeeded
   } // End if ok

   return status;
}


ReplyPacket*
UserProcessor::handlePoiReviewAdd( const RequestPacket& p ) {

   PoiReviewItem poi;
   int packetPos = REQUEST_HEADER_SIZE;
   poi.load( &p, packetPos );
   ReplyPacket* r = new ReplyPacket( REPLY_HEADER_SIZE + 40/*outreviewID*/, 
                                     Packet::PACKETTYPE_POIREVIEW_ADD_REPLY,
                                     &p, StringTableUTF8::OK );
   const PoiReviewDetail& d = poi.getReviews()[ 0 ];
   char query[ 4096 + d.getTitle().size() + d.getText().size() ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   MC2String outreviewID;
   int pos = 0;
   bool runQuery = true;

   if ( !d.getReviewID().empty() ) {
      // Modifying
      // Check ownerUIN
      sprintf( query, "SELECT userUIN, lang FROM ISABPoiReview WHERE id = %s",
               d.getReviewID().c_str() );
      if ( ! doQuery( sqlQuery, query, 
                      "UP::handlePoiReviewAdd check ownerUIN" ) ) 
      {
         mc2log << error 
                << "UP::handlePoiReviewAdd(): "
                << " check owner failed! Query \"" 
                << query << "\"" << endl;
         r->setStatus( StringTableUTF8::NOTOK );
      } else {
         // Get entry
         if ( sqlQuery->nextRow() && 
              strtoul( sqlQuery->getColumn( 0 ), NULL, 0 ) == d.getOwnerUIN() )
         {
            // Update, grade, title and text
            pos = sprintf( query, "UPDATE ISABPoiReview SET " );
            bool comma = false;
            // Can't modify grade as it is in ISABPoiGrade too...
#if 0
            if ( d.getGrade() != MAX_UINT32 ) {
                pos += sprintf( query + pos, " grade = %d ", d.getGrade() );
                comma = true;
            }
#endif
            if ( !d.getTitle().empty() ) {
               if ( comma ) { pos += sprintf( query + pos, ", " );
               } else { comma = true; }
               pos += sprintf( query + pos, " title = '%s' ", 
                               StringUtility::SQLEscapeSecure( 
                                  d.getTitle() ).c_str() );
            }
            if ( !d.getText().empty() ) {
               if ( comma ) { pos += sprintf( query + pos, ", " );
               } else { comma = true; }
               pos += sprintf( query + pos, " text = '%s' ", 
                               StringUtility::SQLEscapeSecure( 
                                  d.getText() ).c_str() );
            }
            if ( d.getLang() != LangTypes::invalidLanguage &&
                 atoi( sqlQuery->getColumn( 1 ) ) != d.getLang() ) {
               if ( comma ) { pos += sprintf( query + pos, ", " );
               } else { comma = true; }
               pos += sprintf( query + pos, " lang = %d ", 
                               uint32(d.getLang()) );
            }
            pos += sprintf( query + pos, "WHERE id = %s",
                            d.getReviewID().c_str() );
         } else {
            if ( sqlQuery->getNumColumns() == 0 ) {
               // Not found
               r->setStatus( StringTableUTF8::NOTFOUND );
            } else {
               // Not same uin
               r->setStatus( StringTableUTF8::NOT_ALLOWED );
            }
         }
      } // End else query succedded!
   } else if ( !poi.getPoiID().empty() &&  
               !d.getTitle().empty() && !d.getText().empty() ) {

      sprintf( query, "SELECT userUIN FROM ISABPoiReview "
               "WHERE poiID = %s AND userUIN = %u", poi.getPoiID().c_str(), 
               d.getOwnerUIN() );
      if ( ! doQuery( sqlQuery, query, 
                      "UP::handlePoiReviewAdd check review" ) ) 
      {
         mc2log << error 
                << "UP::handlePoiReviewAdd(): "
                << " check review failed! Query \"" 
                << query << "\"" << endl;
         r->setStatus( StringTableUTF8::NOTOK );
      } else {
         if ( sqlQuery->nextRow() ) {
            // Bad, user! You may not review multiple times!
            r->setStatus( StringTableUTF8::NOT_ALLOWED );
         } else {
            // Grade if possible
            if ( gradePoi( poi, d, sqlQuery ) != StringTableUTF8::OK ) {
               delete sqlQuery;
               sqlQuery = m_sqlConnection->newQuery();
            }
      
            // Add new review
            uint32 ID = getNewUniqueID( "ISABPoiReview", "id" );
            if ( ID == 0 ) {
               mc2log << error << "UP::handlePoiReviewAdd(): "
                      << "getNewUniqueID(ISABPoiReview) returned 0, aborting!" 
                      << endl;
               r->setStatus( StringTableUTF8::NOTOK );
            } else {
               STLStringUtility::uint2str( ID, outreviewID );
               pos = sprintf( query, "INSERT INTO ISABPoiReview VALUES "
                              "( %d , %s, ",
                              ID, StringUtility::SQLEscapeSecure( 
                                 poi.getPoiID() ).c_str() );
               pos += d.addSQLData( query + pos );
               pos += sprintf( query + pos, ", %d ", 
                               TimeUtility::getRealTime() );
               strcat( query, " )" );
            }
         } // End else not reviewed poi yet
      } // End else request succeeded
   } else if ( !poi.getPoiID().empty() && d.getGrade() != MAX_UINT32 ) {
      // Grade poi
      r->setStatus( gradePoi( poi, d, sqlQuery ) );
      runQuery = false; // Done in gradePoi
   } else {
      mc2log << warn << "UP::handlePoiReviewAdd no data to work with!" << endl;
      r->setStatus( StringTableUTF8::NOTOK );
   }

   packetPos = REPLY_HEADER_SIZE;
   r->incWriteString( packetPos, outreviewID );

   if ( runQuery && r->getStatus() == StringTableUTF8::OK ) {
      if ( ! doQuery( sqlQuery, query, 
                      "UP::handlePoiReviewAdd()" ) ) 
      {
         mc2log << error 
                << "UP::handlePoiReviewAdd(): "
                << " db failed! Query \"" 
                << query << "\"" << endl;
         r->setStatus( StringTableUTF8::NOTOK );
      }
   }

   r->setLength( packetPos );
   delete sqlQuery;

   return r;
}

ReplyPacket*
UserProcessor::handlePoiReviewDelete( const RequestPacket& p ) {
   ReplyPacket* r = new ReplyPacket( 
      REPLY_HEADER_SIZE, Packet::PACKETTYPE_POIREVIEW_DELETE_REPLY,
      &p, StringTableUTF8::OK  );
   char query[ 4096 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   int pos = REQUEST_HEADER_SIZE;

   uint32 changerUIN = p.incReadLong( pos );
   MC2String reviewID = p.incReadString( pos );

   // Check changerUIN
   sprintf( query, "SELECT userUIN FROM ISABPoiReview where id = %s",
            reviewID.c_str() );
   if ( ! doQuery( sqlQuery, query, 
                   "UP::handlePoiReviewDelete check changerUIN" ) ) 
   {
      mc2log << error 
             << "UP::handlePoiReviewDelete(): "
             << " check uin failed! Query \"" 
             << query << "\"" << endl;
      r->setStatus( StringTableUTF8::NOTOK );
   } else {
      // Get entry
      bool hasEntry = sqlQuery->nextRow();
      if ( hasEntry && 
           strtoul( sqlQuery->getColumn( 0 ), NULL, 0 ) == changerUIN )
      {
         // Ok time to remove it!
         sprintf( query, "DELETE FROM ISABPoiReview where id = %s",
                  reviewID.c_str() );
         if ( ! doQuery( sqlQuery, query, 
                         "UP::handlePoiReviewDelete delete review" ) ) 
         {
            mc2log << error 
                   << "UP::handlePoiReviewDelete(): "
                   << " delete review failed! Query \"" 
                   << query << "\"" << endl;
            r->setStatus( StringTableUTF8::NOTOK );
         } else {
            // Ok!
         }
      } else {
         if ( !hasEntry ) {
            // Not found
            r->setStatus( StringTableUTF8::NOTFOUND );
         } else {
            // Not same uin
            r->setStatus( StringTableUTF8::NOT_ALLOWED );
         }
      }
   } // End else query succeeded 

   delete sqlQuery;
   return r;
}

ReplyPacket*
UserProcessor::handlePoiReviewList( const RequestPacket& p ) {
   int packetPos = REQUEST_HEADER_SIZE;
   ReplyPacket* r = new ReplyPacket( REPLY_HEADER_SIZE, 
                                     Packet::PACKETTYPE_POIREVIEW_LIST_REPLY,
                                     &p, StringTableUTF8::OK );
   char query[ 4096 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   // Read data
   uint32 uin = p.incReadLong( packetPos );
   MC2String reviewID = p.incReadString( packetPos );
   MC2String poiID = p.incReadString( packetPos );
   PoiReviewEnums::reviewDetails_t detail = PoiReviewEnums::reviewDetails_t(
       p.incReadLong( packetPos ) );
   LangTypes::language_t lang = LangTypes::language_t( 
      p.incReadLong( packetPos ) );

   typedef set<PoiReviewItem> reviewsC;
   reviewsC reviews;

   int pos = sprintf( query, "SELECT ISABPoiReview.id, poiID, userUIN, grade, "
                      "lang, title, text, totalGrade, totalVote, time, "
                      "logonID, firstname, lastname "
                      "FROM ISABPoiReview "
                      "LEFT JOIN ISABPoiGrade ON poiID = ISABPoiGrade.id "
                      " LEFT JOIN ISABUserUser ON userUIN = UIN "
                      " WHERE " );

   if ( uin != 0 ) {
      pos += sprintf( query + pos, "userUIN = %u", uin );
   } else if ( !reviewID.empty() ) {
      pos += sprintf( query + pos, "id = %s", StringUtility::SQLEscapeSecure( 
                         reviewID ).c_str() );
   } else if ( !poiID.empty() ) {
      pos += sprintf( query + pos, "poiID = %s",
                      StringUtility::SQLEscapeSecure( poiID ).c_str() );
   } else {
      mc2log << warn << "UP::handlePoiReviewList no data to work with!" 
             << endl;
      r->setStatus( StringTableUTF8::NOTOK );
   }

   if ( r->getStatus() == StringTableUTF8::OK && 
        lang != LangTypes::invalidLanguage) {
      // If not LangTypes::invalidLanguage then limit to lang
      pos += sprintf( query + pos, " AND lang = %d", uint32( lang ) );
   }

   if ( r->getStatus() != StringTableUTF8::OK ) {
      // Do nothing
   } else if ( ! doQuery( sqlQuery, query, 
                   "UP::handlePoiReviewList get list" ) ) 
   {
      mc2log << error 
             << "UP::handlePoiReviewList(): "
             << " list failed! Query \"" 
             << query << "\"" << endl;
      r->setStatus( StringTableUTF8::NOTOK );
   } else {
      while ( sqlQuery->nextRow() ) {
         // If some detail level then add only some
         set<PoiReviewItem>::iterator findIt = reviews.find( 
            PoiReviewItem( sqlQuery->getColumn( 1 ) ) );
         if ( findIt == reviews.end() ) {
            findIt = reviews.insert( 
               PoiReviewItem( 
                  sqlQuery->getColumn( 1 ),
                  detail >= PoiReviewEnums::some ? 
                  atoi( sqlQuery->getColumn( 7 ) ) : 0,
                  detail >= PoiReviewEnums::some ? 
                  atoi( sqlQuery->getColumn( 8 ) ) : 0 ) ).first;
         }

         PoiReviewItem* poi = const_cast<PoiReviewItem*>( &*findIt );
         poi->addReview( 
            PoiReviewDetail( 
               detail >= PoiReviewEnums::some ? strtoul( sqlQuery->getColumn( 
                                                            2 ), NULL, 0 ) : 0,
               
               detail >= PoiReviewEnums::some ? atoi( 
                  sqlQuery->getColumn( 3 ) ) : MAX_UINT32,
               detail >= PoiReviewEnums::none ? sqlQuery->getColumn( 0 ) : "",
               detail >= PoiReviewEnums::some ? sqlQuery->getColumn( 5 ) : "",
               detail >= PoiReviewEnums::all ? sqlQuery->getColumn( 6 )  : "",
               detail >= PoiReviewEnums::some ? LangTypes::language_t( 
                  atoi( sqlQuery->getColumn( 4 ) ) ) : 
               LangTypes::invalidLanguage,
               detail >= PoiReviewEnums::some ? atoi( 
                  sqlQuery->getColumn( 9 ) ) : 0,
               detail >= PoiReviewEnums::some ? sqlQuery->getColumn( 10)  : "",
               detail >= PoiReviewEnums::some ? sqlQuery->getColumn( 11)  : "",
               detail >= PoiReviewEnums::some ? sqlQuery->getColumn( 12)  : ""
               )
            );
      }
   }

   uint32 size = 4; // nbr
   for ( reviewsC::const_iterator it = reviews.begin() ; it != reviews.end() ;
         ++it ) {
      size += (*it).getSizeAsBytes();
   }

   // Size of reply data
   r->resize( r->getLength() + size );
   // Add data
   packetPos = REPLY_HEADER_SIZE;
   r->incWriteLong( packetPos, reviews.size() );
   for ( reviewsC::const_iterator it = reviews.begin() ; it != reviews.end() ;
         ++it ) {
      (*it).save( r, packetPos );
   }
   r->setLength( packetPos );

   delete sqlQuery;
   return r;
}


ReplyPacket* 
UserProcessor::handleRouteStorageChangeRouteRequestPacket( 
   const RouteStorageChangeRouteRequestPacket* p )
{
   RouteStorageChangeRouteReplyPacket* reply = NULL;

   if ( Properties::getProperty( "ROUTE_STORAGE_PATH" ) != NULL ) {
      // No way I loads the file changes validUntil and saves it again
      return new RouteStorageChangeRouteReplyPacket( p, 
                                                     StringTable::OK );
   }
   
   char query[ 4096 ];

   sprintf( query, "UPDATE ISABRouteStorage SET validUntil = %d "
                   "WHERE routeID = %d AND createTime = %d",
            p->getValidUntil(),
            p->getRouteID(),
            p->getCreateTime() );

   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   if ( ! doQuery( sqlQuery, query, 
                   "UP::handleRouteStorageChangeRouteRequestPacket()" ) ) 
   {
      mc2log << error 
             << "UP::handleRouteStorageChangeRouteRequestPacket(): "
             << " attempt to change route in db failed! Query \"" 
             << query << "\"" << endl;
   }

   sprintf( query, "UPDATE ISABRouteStorageCoords SET validUntil = %d "
                   "WHERE routeID = %d AND createTime = %d",
            p->getValidUntil(),
            p->getRouteID(),
            p->getCreateTime() );

   if( ! doQuery( sqlQuery, query, 
                  "UP::handleRouteStorageChangeRouteRequestPacket()" ) ) 
   {
      mc2log << error 
             << "UP::handleRouteStorageChangeRouteRequestPacket(): "
             << " attempt to change route coords in db failed! Query \"" 
             << query << "\"" << endl;
      reply = new RouteStorageChangeRouteReplyPacket( p, 
                                                      StringTable::NOTOK );
   } else {
      reply = new RouteStorageChangeRouteReplyPacket( p, 
                                                      StringTable::OK );
   }

   delete sqlQuery;
   return reply;
}

ReplyPacket*
UserProcessor::handlePeriodic( const RequestPacket& p ) {
   mc2dbg4 << "UP::handlePeriodic" << endl;

   if ( !m_doneInitialDatabaseCheck && m_leaderStatus->isLeader() ) {
      // Check database
      if ( ! initialCheckDatabase( m_noSqlUpdate ) ) {
         if ( !m_leaderStatus->isLeader() ) {
            // we lost leader status during the check, so the failure
            // is probably due to that (the sql-driver blocks writes 
            // when we're not leader), just ignore this periodic request
            return NULL;
         }
         mc2log << fatal << "[UP] Initial database test: [FAILED]" << endl;
         exit(1);
      } else {
         mc2log << info << "[UP] Initial database test: [SUCCESS]" << endl;
      }

      m_doneInitialDatabaseCheck = true;
   }
   
   // if oldest stored debitdata too old then make debit
   if ( !m_debitSet.empty() && m_debitSet.begin()->date + 
        Properties::getUint32Property( "DEBIT_STORE_TIMEOUT", 60 ) < 
        TimeUtility::getRealTime() )
   {
      addStoredDebits();
   }

   return NULL;
}

UserReplyPacket*
UserProcessor::handleGetStoredUserDataRequestPacket( const
      GetStoredUserDataRequestPacket& p )
{
   int reqPos = USER_REQUEST_HEADER_SIZE;
   int replyPos = USER_REPLY_HEADER_SIZE;
   auto_ptr< CharEncSQLQuery >  sqlQuery( m_sqlConnection->newQuery() );
   auto_ptr< GetStoredUserDataReplyPacket > 
      reply( new GetStoredUserDataReplyPacket( &p ) );
   const MC2String whereTag( "UP::handleGetStoredUserDataRequestPacket" );
   SQLDataContainer cont;
   SQLQueryHandler queryHandler;
   queryHandler.fetchData( sqlQuery.get(), whereTag, p, *reply, reqPos,
                           replyPos, *m_storedUserTableData, cont );
   return reply.release();
}

UserReplyPacket*
UserProcessor::handleSetStoredUserDataRequestPacket( const
      SetStoredUserDataRequestPacket& p )
{
   int reqPos = USER_REQUEST_HEADER_SIZE;
   auto_ptr< CharEncSQLQuery >  sqlQuery( m_sqlConnection->newQuery() );
   auto_ptr< SetStoredUserDataReplyPacket > 
      reply( new SetStoredUserDataReplyPacket( &p ) );
   const MC2String whereTag( "UP::handleSetStoredUserDataRequestPacket" );
   SQLDataContainer cont;
   SQLQueryHandler queryHandler;
   queryHandler.insertData( sqlQuery.get(), whereTag, p, *reply, reqPos,
                            *m_storedUserTableData, cont );
   return reply.release();
}


class UserLicenceKeyFinder {
public:
   UserLicenceKeyFinder( const UserLicenceKey* a ) : m_a( a ) {}

   bool operator()( const UserLicenceKey* b ) const {
      return m_a->compare( *b );
   }

private:
   const UserLicenceKey* m_a;
};

class UserIDKeyFinder {
public:
   UserIDKeyFinder( const UserIDKey* a ) : m_a( a ) {}

   bool operator()( const UserIDKey* b ) const {
      return m_a->compare( *b );
   }

private:
   const UserIDKey* m_a;
};

class UserIDKeyIDFinder {
public:
   UserIDKeyIDFinder( uint32 id ) : m_id( id ) {}

   bool operator()( const UserIDKey* b ) const {
      return m_id == b->getID();
   }

private:
   uint32 m_id;
};


UserReplyPacket*
UserProcessor::handleLicenceToRequestPacket( 
   const LicenceToRequestPacket& p ) {
   uint32 status = StringTable::OK;
   bool ok = true;

   uint32 uin = p.getUIN();
   auto_ptr<UserLicenceKey> key( p.getLicenceKey() );
   // Here we want only licences with right product
   key->setProduct( key->getProduct() );

   // begin transaction
   ok = m_sqlConnection->beginTransaction();

   // The query
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );

   // Get DEVICECHANGES for user and check if may change device
   int32 userDeviceChanges = 0;
   if ( ok ) {
      if ( getISABUserUserColumnValue( 
              sqlQuery.get(), "LicenceTo get user devicechanges", uin,
              "deviceChanges", userDeviceChanges ) ) {
         if ( userDeviceChanges <= 0 && userDeviceChanges != -1 ) {
            ok = false;
            status = StringTable::NOT_ALLOWED;
            mc2log << info << "UP:handleLicenceTo user has no devicechanges "
                   << userDeviceChanges << endl;
         }
      } else {
         mc2log << warn << "UP:handleLicenceTo get user devicechanges failed."
                << endl;
         ok = false;
      }
   }

   // Get nbr WF TRIAL rights for user, used later if user has none
   UserEnums::URType urwftrial( UserEnums::UR_TRIAL, UserEnums::UR_WF );
   int32 nbrWFTrialsForUser = -1;
   if ( ok ) {
      vector<UserRight*> rights;

      nbrWFTrialsForUser = getUserRights( uin, rights );
      ok = nbrWFTrialsForUser != -1;
      STLUtility::deleteValues( rights );
   }

   // Find key(s) for user. If one then set it as user's key
   auto_ptr<UserLicenceKey> userLicenceKey;
   if ( ok ) {
      UserLicenceKeyPVect licencies;
      ok = getUserLicence( uin, licencies, sqlQuery.get() ) >= 0;
      // Limt to product
      UserLicenceKeyPVect prodKeys;
      UserLicenceKeyProductCounter( key->getProduct() ).getProductKeys(
         licencies, prodKeys );
      if ( prodKeys.size() == 1 ) {
         userLicenceKey.reset( new UserLicenceKey( *prodKeys[ 0 ] ) );
      }
      STLUtility::deleteValues( licencies );
   }

   // Find owner of key
   uint32 ownerUIN = 0;
   uint32 ownerKeyID = 0;
   auto_ptr<UserLicenceKey> ownerLicenceKey;
   if ( ok ) {
      vector<uint32> users;
      vector<MC2String> logonIDs;
      uint32 nbrUsers = getUINsFromUserLicenceKey( 
         key.get(), users, logonIDs, sqlQuery.get() );
      if ( nbrUsers == 1 ) {
         ownerUIN = users[ 0 ];
         // Get the owner licence key id
         UserLicenceKeyPVect ownerLicencies;
         ok = getUserLicence( ownerUIN, ownerLicencies, sqlQuery.get() ) >= 0;
         // Find the key
         UserLicenceKeyPVect::const_iterator findKey = find_if( 
            ownerLicencies.begin(), ownerLicencies.end(),
            UserLicenceKeyFinder( key.get() ) );
         if ( findKey == ownerLicencies.end() ) {
            // Try with trimmed key!
            MC2String imei15digts;
            if ( key->extract15Digits( imei15digts ) && 
                 imei15digts != key->getLicenceKeyStr() ) {
               UserLicenceKey userKeyTrimmed( *key );
               userKeyTrimmed.setLicence( imei15digts );
               findKey = find_if( 
                  ownerLicencies.begin(), ownerLicencies.end(),
                  UserLicenceKeyFinder( &userKeyTrimmed ) );
            }
         }
         if ( findKey != ownerLicencies.end() ) {
            ownerKeyID = (*findKey)->getID();
            ownerLicenceKey.reset( new UserLicenceKey( *(*findKey) ) );
         }
         STLUtility::deleteValues( ownerLicencies );
      }
   }

   //  If owner then check for mayChangeDevice in owner
   int32 ownerDeviceChanges = 0;
   if ( ok && ownerUIN != 0 ) {
      if ( getISABUserUserColumnValue( 
              sqlQuery.get(), "LicenceTo get user devicechanges", ownerUIN,
              "deviceChanges", ownerDeviceChanges ) ) {
         if ( ownerDeviceChanges <= 0 && ownerDeviceChanges != -1 ) {
            //   If may not then error NOT_ALLOWED
            ok = false;
            status = StringTable::NOT_ALLOWED;
            mc2log << info << "UP:handleLicenceTo owner has no devicechanges "
                   << ownerDeviceChanges << endl;
         }
      } else {
         mc2log << warn << "UP:handleLicenceTo get owner devicechanges failed."
                << endl;
         ok = false;
      }

      // If user has no trial right copy from owner (if owner has > 0)
      // copy all (normally one but...)
      if ( ok && nbrWFTrialsForUser == 0 ) {
         vector<UserRight*> rights;

         int32 res = getUserRights( ownerUIN, rights );
         ok = res != -1;
         if ( ok ) {
            for ( uint32 i = 0 ; i < rights.size() && ok ; ++i ) {
               if ( rights[ i ]->getUserRightType() == urwftrial ) {
                  // Copy to user
                  char query[4096];
                  uint32 ID = getNewUniqueID( "ISABUserRight", "id" );
                  
                  if ( ID == 0 ) {
                     mc2log << error << "UP:handleLicenceTo failed finding "
                            << "right id" << endl;
                     ok = false;
                  }

                  strcpy( query, "INSERT INTO ISABUserRight VALUES ( ");
                  addUserRightData( query + strlen( query ), 
                                    ID, uin, rights[ i ] );
                  strcat( query, " )" );
                  if ( ok && !doQuery( sqlQuery.get(), query,
                                       "UP::handleLicenceTo add right" ) ) {
                     mc2log << error << "UP::handleLicenceTo "
                            << " Add of WF Trial UserRight failed!" << endl;
                     ok = false;
                  }
               }
            }
         }
         STLUtility::deleteValues( rights );
      }
   }

   // Fixup key, two choices
   if ( ok ) {
      if ( ownerUIN != 0 ) {
         // Change UIN on owner key from ownerUIN to uin
         ok = setUserForIDinUserLicence( uin, ownerUIN, 0/*changerUIN*/,
                                         ownerLicenceKey.get(), 
                                         sqlQuery.get() );
      } else {
         // Add new key to uin
         ok = addUserLicence( uin, 0/*changerUIN*/, key.get(), 
                              sqlQuery.get() );
      }
   }

   // Fixup userLicenceKey, again two choices
   if ( ok && userLicenceKey.get() != NULL ) {
      if ( ownerUIN != 0 ) {
         // Change UIN on userLicenceKey key from uin to ownerUIN
         ok = setUserForIDinUserLicence( ownerUIN, uin, 0/*changerUIN*/,
                                         userLicenceKey.get(), 
                                         sqlQuery.get() );
      } else {
         // Delete userLicenceKey
         ok = deleteUserLicence( uin, 0/*changerUIN*/, userLicenceKey.get(), 
                                 sqlQuery.get() );
      }
   }

   // Also update DEVICECHANGES in user if not -1
   if ( ok && userDeviceChanges > 0 ) {
      // Decrease by 1
      ok = setISABUserUserColumnValue( 
         sqlQuery.get(), "LicenceTo set user devicechanges", uin,
         0/*changerUIN*/, "deviceChanges", userDeviceChanges -1 );
   }

   if ( ok ) {
      // Remove uin and ownerUIN from cache
      removeUserFromCache( uin );
      removeUserFromCache( ownerUIN );
   }


   // All ok? then commit else rollback
   if ( ok ) {
      ok = m_sqlConnection->commitTransaction();
   } else {
      m_sqlConnection->rollbackTransaction();
   }

   // If error and status is not set then set default error status
   if ( !ok && status == StringTable::OK ) {
      status = StringTable::NOTOK;
   }

   // Reply
   auto_ptr<LicenceToReplyPacket> reply( new LicenceToReplyPacket( 
                                            &p, ownerUIN ) );
   reply->setStatus( status );

   return reply.release();
}


UserReplyPacket*
UserProcessor::handleIDKeyToRequestPacket( 
   const IDKeyToRequestPacket& p ) {
   uint32 status = StringTable::OK;
   bool ok = true;

   uint32 uin = p.getUIN();
   auto_ptr<UserIDKey> key( p.getIDKey() );
   typedef vector<UserIDKey*> UserIDKeyPVect;

   // begin transaction
   ok = m_sqlConnection->beginTransaction();

   // The query
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );

   // The remove id key
   if ( ok ) {
      if ( p.getRemoveIDkeyID() != 0 ) {
         UserIDKeyPVect userIDKeys;
         ok = getUserIDKeys( uin, userIDKeys, sqlQuery.get() ) >= 0;
         // Find the key
         UserIDKeyPVect::const_iterator findKey = find_if( 
            userIDKeys.begin(), userIDKeys.end(),
            UserIDKeyIDFinder( p.getRemoveIDkeyID() ) );
         if ( findKey != userIDKeys.end() ) {
            ok = deleteUserIDKey( uin, 0/*changerUIN*/, (*findKey),
                                  sqlQuery.get() );
         }
         STLUtility::deleteValues( userIDKeys );
      }
   }

   // Find owner of key
   uint32 ownerUIN = 0;
   uint32 ownerKeyID = 0;
   auto_ptr<UserIDKey> ownerIDKey;
   if ( ok ) {
      vector<uint32> users;
      vector<MC2String> logonIDs;
      uint32 nbrUsers = getUINsFromUserIDKey( 
         key.get(), users, logonIDs, sqlQuery.get() );
      if ( nbrUsers == 1 ) {
         ownerUIN = users[ 0 ];
         // Get the owner id key id
         UserIDKeyPVect ownerIDKeys;
         ok = getUserIDKeys( ownerUIN, ownerIDKeys, sqlQuery.get() ) >= 0;
         // Find the key
         UserIDKeyPVect::const_iterator findKey = find_if( 
            ownerIDKeys.begin(), ownerIDKeys.end(),
            UserIDKeyFinder( key.get() ) );
         if ( findKey != ownerIDKeys.end() ) {
            ownerKeyID = (*findKey)->getID();
            ownerIDKey.reset( new UserIDKey( *(*findKey) ) );
         } else {
            // This can't happen, there is an iDKey but can't find it
            ok = false;
         }
         STLUtility::deleteValues( ownerIDKeys );
      }
   }

   // Fixup key, two choices
   if ( ok ) {
      if ( ownerIDKey.get() != NULL ) {
         // Change UIN on owner key from ownerUIN to uin
         ok = setUserForIDinUserIDKey( uin, ownerUIN, 0/*changerUIN*/,
                                       ownerIDKey.get(), 
                                       sqlQuery.get() );
      } else {
         // Add new key to uin
         ok = addUserIDKey( uin, 0/*changerUIN*/, key.get(), 
                            sqlQuery.get() );
      }
   }

   if ( ok ) {
      // Remove uin and ownerUIN from cache
      removeUserFromCache( uin );
      removeUserFromCache( ownerUIN );
   }


   // All ok? then commit else rollback
   if ( ok ) {
      ok = m_sqlConnection->commitTransaction();
   } else {
      m_sqlConnection->rollbackTransaction();
   }

   // If error and status is not set then set default error status
   if ( !ok && status == StringTable::OK ) {
      status = StringTable::NOTOK;
   }

   // Reply
   auto_ptr<IDKeyToReplyPacket> reply( new IDKeyToReplyPacket( 
                                          &p, ownerUIN ) );
   reply->setStatus( status );

   return reply.release();
}


UserUser*
UserProcessor::getUserUser( uint32 UIN ) {
   mc2dbg4 << "UP::getUserUser( " << UIN << " )" << endl;
   UserUser* user = NULL;
   char query[1024];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   sprintf( query, 
      "SELECT UIN, logonID, firstname, initials, lastname, sessionID, "
      "measurementSystem, language, lastdestmapID, lastdestitemID, "
      "lastdestOffset, lastdestTime, lastdestString,  lastorigmapID, "
      "lastorigitemID, lastorigOffset, lastorigTime, lastorigString, "
      "searchType, searchSubstring, searchSorting, searchObject, searchDbMask,"
      "routeCostA, routeCostB, routeCostC, routeCostD, vehicleType, routeType,"
      "editMapRights, editDelayRights, editUserRights, wapService, "
      "htmlService, operatorService, smsService, nbrMunicipals, municipals, "
      "birthDate, routeImageType, validDate, gender, "
      "defaultCountry, defaultMunicipal, defaultCity, "
      "navService, operatorComment, emailAddress, "
      "address1, address2, address3, address4, address5, "
      "routeTurnImageType, externalXMLService, transactionBased, "
      "deviceChanges, supportComment, postalCity, zipCode, "
      "companyName, companyReference, companyVATNbr, emailBounces, "
      "addressBounces, customerContactInfo "
      "FROM ISABUserUser "
      "WHERE UIN = %u", UIN);
   if ( ! doQuery(sqlQuery, query, "UP::getUserUser()") ) {
      delete sqlQuery;
      return NULL;
   }

   if ( ! sqlQuery->nextRow() ) {
      mc2log << warn << "UP::getUserUser(): User with UIN "
             << UIN << " not found!" << endl; 
      delete sqlQuery;
      return NULL;
   }

   user = new UserUser( UIN );

   user->setLogonID             ( sqlQuery->getColumn(1)         );
   user->setFirstname           ( sqlQuery->getColumn(2)         );
   user->setInitials            ( sqlQuery->getColumn(3)         );
   user->setLastname            ( sqlQuery->getColumn(4)         );
   user->setSession             ( sqlQuery->getColumn(5)         );
   user->setMeasurementSystem   ( (UserConstants::MeasurementType)
                                  atoi(sqlQuery->getColumn(6))   );
   user->setLanguage            ( StringTable::getLanguageCode(
                                  sqlQuery->getColumn(7))        );
   // Dest
   user->setLastdest_mapID      ( atoi(sqlQuery->getColumn(8))   );
   user->setLastdest_itemID     ( atoi(sqlQuery->getColumn(9))   );
   user->setLastdest_offset     ( atoi(sqlQuery->getColumn(10))  );
   user->setLastdest_time       ( atoi(sqlQuery->getColumn(11))  );
   user->setBrand               ( sqlQuery->getColumn(12)        );
   // Orig
   user->setLastorig_mapID      ( atoi(sqlQuery->getColumn(13))  );
   user->setLastorig_itemID     ( atoi(sqlQuery->getColumn(14))  );
   user->setLastorig_offset     ( atoi(sqlQuery->getColumn(15))  );
   user->setLastorig_time       ( atoi(sqlQuery->getColumn(16))  );
   user->setBrandOrigin         ( sqlQuery->getColumn(17)        );
   // SEARCH
   user->setSearch_type         ( atoi(sqlQuery->getColumn(18))  );
   user->setSearch_substring    ( atoi(sqlQuery->getColumn(19))  );
   user->setSearch_sorting      ( atoi(sqlQuery->getColumn(20))  );
   user->setSearchForTypes      ( atoi(sqlQuery->getColumn(21))  );
   user->setSearchForLocationTypes( atoi(sqlQuery->getColumn(21)) );
   user->setSearch_DbMask       ( atoi(sqlQuery->getColumn(22))  );
   // Route
   user->setRouting_costA       ( atoi(sqlQuery->getColumn(23))  );
   user->setRouting_costB       ( atoi(sqlQuery->getColumn(24))  );
   user->setRouting_costC       ( atoi(sqlQuery->getColumn(25))  );
   user->setRouting_costD       ( atoi(sqlQuery->getColumn(26))  );
   user->setRouting_vehicle     ( (ItemTypes::vehicle_t) 
                                  atoi(sqlQuery->getColumn(27))  );
   user->setRouting_type        ( atoi(sqlQuery->getColumn(28))  );
   // Rights
   user->setEditMapRights       ( atoi(sqlQuery->getColumn(29))  );
   user->setEditDelayRights     ( atoi(sqlQuery->getColumn(30))  );
   user->setEditUserRights      ( atoi(sqlQuery->getColumn(31))  );
   // Services
   user->setWAPService          ( 't'==sqlQuery->getColumn(32)[0]);
   user->setHTMLService         ( 't'==sqlQuery->getColumn(33)[0]);
   user->setOperatorService     ( 't'==sqlQuery->getColumn(34)[0]);
   user->setSMSService          ( 't'==sqlQuery->getColumn(35)[0]);
    // Municipal
   uint32 nbrMunicipal          = atoi(sqlQuery->getColumn(36));
   const char* municipals       = sqlQuery->getColumn(37);

   const char* pos = municipals;

   uint32* municipal = new uint32[nbrMunicipal];
   uint32 i;
   for ( i = 0 ; (i < nbrMunicipal) && (pos != NULL) && (*pos != '\0');
         i++ ) 
   {
      pos = strchr( pos, ':' );
      municipal[ i ] = atoi(pos);
   }
   if ( i != nbrMunicipal ) {
      mc2log << error << "UP::getUserUser(): Municipal format "
             << "error, UIN: " << UIN << endl;
      delete user;
      delete [] municipal;
      delete sqlQuery;
      return NULL;
   }
   user->setMunicipal           ( municipal, nbrMunicipal        );
   // Birthdate
   user->setBirthDate           ( sqlQuery->getColumn(38)        );
   // RouteImageType
   user->setRouteImageType      ( (UserConstants::RouteImageType)
                                  atoi(sqlQuery->getColumn(39))  );
   // ValidDate
   mc2dbg8 << "UP::getUserUser(): before setting validdate"
           << endl;
   user->setValidDate           ( StringUtility::makeDate( 
                                  sqlQuery->getColumn(40))       );
   // Gender
   user->setGender              ( (UserConstants::GenderType)
                                  atoi(sqlQuery->getColumn(41))  );
    //Default search settings
   mc2dbg8 << "UP::getUserUser(): defaultCountry is: " << sqlQuery->getColumn(42) << endl;
   user->setDefaultCountry      ( sqlQuery->getColumn(42)        );
   user->setDefaultMunicipal    ( sqlQuery->getColumn(43)        );
   user->setDefaultCity         ( sqlQuery->getColumn(44)        );

   user->setNavService          ( sqlQuery->getColumn(45)[0] == 't' );
   user->setOperatorComment     ( sqlQuery->getColumn(46)        );
   user->setEmailAddress        ( sqlQuery->getColumn(47)        );
   user->setAddress1            ( sqlQuery->getColumn(48)        );
   user->setAddress2            ( sqlQuery->getColumn(49)        );
   user->setAddress3            ( sqlQuery->getColumn(50)        );
   user->setAddress4            ( sqlQuery->getColumn(51)        );
   user->setAddress5            ( sqlQuery->getColumn(52)        );
   user->setRouteTurnImageType  ( UserConstants::RouteTurnImageType( 
                                  atoi(sqlQuery->getColumn(53))  ));
   user->setExternalXmlService  ( sqlQuery->getColumn(54)[0] == 't' );
   user->setTransactionBased    ( UserConstants::transactionBased_t(
                                     atoi(sqlQuery->getColumn(55))) );
   user->setDeviceChanges       ( atoi(sqlQuery->getColumn( 56 ) ) );
   user->setSupportComment      ( sqlQuery->getColumn( 57 ) );
   user->setPostalCity          ( sqlQuery->getColumn( 58 ) );
   user->setZipCode             ( sqlQuery->getColumn( 59 ) );
   user->setCompanyName         ( sqlQuery->getColumn( 60 ) );
   user->setCompanyReference    ( sqlQuery->getColumn( 61 ) );
   user->setCompanyVATNbr       ( sqlQuery->getColumn( 62 ) );
   user->setEmailBounces        ( atoi(sqlQuery->getColumn( 63 ) ) );
   user->setAddressBounces       ( atoi(sqlQuery->getColumn( 64 ) ) );
   user->setCustomerContactInfo ( sqlQuery->getColumn( 65 ) );

   // all done
   mc2dbg4 << "UP::getUserUser(): Retrieved user with UIN: " 
           << UIN << " [OK]" << endl;

   if ( sqlQuery->nextRow() ) {
      mc2log << warn << "UP::getUserUser(): There is more than "
             << "one user with UIN: " << UIN << " in the database! "
             << "I returned the first one." << endl;
   }

   delete sqlQuery;
   return user;
}


int32 
UserProcessor::getUserCellular( uint32 UIN, UserCellular**& cellulars )
{
   int32 nbrCellular = 0;
   cellulars = NULL;
   char query[1024];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserCellular() for UIN: " << UIN << endl;

   sprintf( query, 
            "SELECT COUNT(*) FROM ISABUserCellular WHERE userUIN = %u", UIN);

   if ( ! doQuery(sqlQuery, query, "UP::getUserCellular() get count") ) {
      delete sqlQuery;
      return -1;
   }

   if (sqlQuery->nextRow())
      nbrCellular = atoi(sqlQuery->getColumn(0));

   sprintf(query, "SELECT id, phoneNumber, model, SMSParams, maxSearchHitsWap,"
           "maxRouteLinesWap, EOLType, CharsPerLine, posActive, typeOfPos, "
           "posUserName, posPassword, lastposLat, lastposLong, "
           "lastposInnerRadius, lastposOuterRadius, lastposStartAngle, "
           "lastposStopAngle, lastposTime "
           "FROM ISABUserCellular "
           "WHERE userUIN = %u", UIN);

   if ( ! doQuery(sqlQuery, query, "UP::getUserCellular() get cellulars") ) {
      delete sqlQuery;
      return -1;
   }

   cellulars = new UserCellular*[ nbrCellular ];
   const char* cellularNameVector[nbrCellular];
   int i = -1;
   while (sqlQuery->nextRow()) {
      i++;
      UserCellular* cell;
      mc2dbg8 << "UP::getUserCellular getting Cellular " 
              << i << endl;; 
      cell = new UserCellular( atoi( sqlQuery->getColumn(0)) );
      cellulars[i] = cell;
      cell->setPhoneNumber      ( sqlQuery->getColumn(1)         );
      // Model is handled specially:
      const char* modelName = sqlQuery->getColumn(2);
      cellularNameVector[i] = StringUtility::newStrDup(modelName);
      // Cellular Phone Model is gotten later after this query is done
      cell->setSMSParams         ( atoi(sqlQuery->getColumn(3))   );
      cell->setMaxSearchHitsWap  ( atoi(sqlQuery->getColumn(4))   );
      cell->setMaxRouteLinesWap  ( atoi(sqlQuery->getColumn(5))   );
      cell->setEOLType           ( (UserConstants::EOLType)
                                   atoi(sqlQuery->getColumn(6))   );
      cell->setSMSLineLength     ( atoi(sqlQuery->getColumn(7))   );
      cell->setPosActive         ( 't'==sqlQuery->getColumn(8)[0] );
      cell->setTypeOfPos         ( (UserConstants::posType)
                                   atoi(sqlQuery->getColumn(9))   );
      cell->setPosUserName       ( sqlQuery->getColumn(10)        );
      cell->setPosPassword       ( sqlQuery->getColumn(11)        );
      cell->setLastPosLat        ( atoi(sqlQuery->getColumn(12))  );
      cell->setLastPosLong       ( atoi(sqlQuery->getColumn(13))  );
      cell->setLastPosInnerRadius( atoi(sqlQuery->getColumn(14))  );
      cell->setLastPosOuterRadius( atoi(sqlQuery->getColumn(15))  );
      cell->setLastPosStartAngle ( atoi(sqlQuery->getColumn(16))  );
      cell->setLastPosStopAngle  ( atoi(sqlQuery->getColumn(17))  );
      cell->setLastPosLong       ( atoi(sqlQuery->getColumn(13))  );
      cell->setLastPosInnerRadius( atoi(sqlQuery->getColumn(14))  );
      cell->setLastPosOuterRadius( atoi(sqlQuery->getColumn(15))  );
      cell->setLastPosTime       ( atoi(sqlQuery->getColumn(18))  );

      // Done with this cellular
      mc2dbg8 << "UP::getUserCellular() " << i << " done" << endl;
   }

   // we're done with the query
   delete sqlQuery;

   for ( int32 i = 0 ; i < nbrCellular ; i++ ) {
      UserCellular* cell;
      cell = cellulars[i];
      CellularPhoneModel* model =
         getCellularPhoneModel( cellularNameVector[i] );
      if ( model == NULL ) {
         mc2log << error << "UP::getUserCellular(), error getting "
                << "phoneModel (" << cellularNameVector[i] << " for UIN: "
                << UIN << endl;
         delete cell;
         return -1;
      }
      cell->setModel( model );
      }
      for ( int32 i = 0; i < nbrCellular; i++ ) {
         delete [] cellularNameVector[i];
      }

   if (0 == nbrCellular) {
      nbrCellular = 0;
      mc2dbg4 << "UP::getUserCellular() UIN: " 
              << UIN << " has no Cellulars." << endl;
   }
  
   return nbrCellular;
}


int32 
UserProcessor::getOldUserLicence( 
   uint32 UIN, vector<UserLicenceKey*>& licencies,
   CharEncSQLQuery* sqlQuery )
{
   int32 nbrLicencies = 0;
   char query[ 1024 ];
   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }

   mc2dbg4 << "UP::getOldUserLicence() for UIN: " << UIN << endl;

   sprintf( query, "SELECT * FROM ISABUserLicence WHERE userUIN = %u"
            , UIN );


   if ( ! doQuery(sqlQuery, query, "UP::getOldUserLicence() get licencies") ) {
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      mc2dbg8 << "UP::getOldUserLicence getting licence " 
              << nbrLicencies << endl;
      UserLicenceKey* licence = 
         new UserLicenceKey( atoi( sqlQuery->getColumn(0)) );
      licencies.push_back( licence );
      uint32 licenceLength = atoi( sqlQuery->getColumn( 2 ) );
      ScopedArray<byte> licenceKey( new byte[ licenceLength + 1 ] );

      int licLen = 
         StringUtility::base64Decode( sqlQuery->getRawColumn(3), 
                                      licenceKey.get() );
      if ( licLen == -1 )
      {
         mc2log << error << "UP::():getOldUserLicence "
                   "Failed to do base64 decode of licence key, UIN: "
                << UIN << " id " << licence->getID() << endl;
         nbrLicencies = 0;
         break; // Leave the while
      }
      licence->setLicence( licenceKey.get(), licLen );

      // Done with this licence
      nbrLicencies++;
      mc2dbg8 << "UP::getOldUserLicence() " << nbrLicencies << " done" << endl;
   }

   mc2dbg4 << "UP::getOldUserLicence() UIN: " 
           << UIN << " has " << nbrLicencies << " licencies." << endl;
  
   return nbrLicencies;
}

int32 
UserProcessor::getUserLicence( 
   uint32 UIN, vector<UserLicenceKey*>& licencies,
   CharEncSQLQuery* sqlQuery )
{
   int32 nbrLicencies = 0;
   bool ok = true;
   char query[ 1024 ];
   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }

   mc2dbg4 << "UP::getUserLicence() for UIN: " << UIN << endl;

   // Get from old! Move to new
   vector<UserLicenceKey*> oldLicencies;
   int32 nbrOldLicencies = getOldUserLicence( 
      UIN, oldLicencies, sqlQuery );
      
   if ( nbrOldLicencies > 0 ) {
      // begin transaction
      ok = m_sqlConnection->beginTransaction();
      for ( uint32 i = 0 ; i < oldLicencies.size() && ok ; ++i ) {
         // Move key type from licence to keyType
         const char* oldKey = reinterpret_cast< const char* > ( 
            oldLicencies[ i ]->getLicenceKey() );
         const char* foundColon = static_cast<const char*>( 
            memchr( oldKey, ':', oldLicencies[ i ]->getLicenceLength() ) );
         MC2String keyType;
         static const char* validTypesArray[] = { 
            "imei", "imsi", "btmac", "bbpin", "esn" };
         static const set< MC2String > validHardwareKeyTypes(
            validTypesArray, validTypesArray + NBR_ITEMS( validTypesArray ) );
         if ( foundColon != NULL && 
              validHardwareKeyTypes.find( MC2String( oldKey, foundColon ) ) !=
              validHardwareKeyTypes.end() ) {
            // Check if type is really a type or it is a Windows Mobile
            // phone that sends firmware data in imei.
            keyType = MC2String( oldKey, foundColon );
            MC2String key( foundColon + 1, // Skipp colon
                           oldKey + oldLicencies[ i ]->getLicenceLength() );
            oldLicencies[ i ]->setLicence( key );
         } else {
            keyType = "imei";
         }
         oldLicencies[ i ]->setKeyType( keyType );
         
         // Remove from old
         if ( ok ) {
            ok = deleteOldUserLicence( UIN, 0/*changerUIN*/, 
                                       oldLicencies[ i ], sqlQuery );
         }

         // Old key is now good enough to be inserted into new table
         if ( ok ) {
            // This method sets the id to the new one.
            ok = addUserLicence( UIN, 0/*changerUIN*/, 
                                 oldLicencies[ i ], sqlQuery );
         }
      } // End for all old licence keys
      
      if ( ok ) {
         ok = m_sqlConnection->commitTransaction();
      } else {
         m_sqlConnection->rollbackTransaction();
         nbrLicencies = -1;
      }
   } else if ( nbrOldLicencies == -1 ) {
      nbrLicencies = -1;
   }
   STLUtility::deleteValues( oldLicencies );

   // Get the licences from new table that now has all licencekeys
   if ( ok ) {
      sprintf( query, "SELECT id, licenceKey, product, keyType "
               "FROM ISABUserLicenceKey WHERE userUIN = %u", UIN );
      
      ok = doQuery( sqlQuery, query, "UP::getUserLicence() get licencies" );

      while ( ok && sqlQuery->nextRow() ) {
         mc2dbg8 << "UP::getUserLicence getting licence " 
                 << nbrLicencies << endl;
         licencies.push_back( new UserLicenceKey(
                                 atoi( sqlQuery->getColumn( 0 ) ) ) );
         licencies.back()->setLicence( sqlQuery->getColumn( 1 ) );
         licencies.back()->setProduct( sqlQuery->getColumn( 2 ) );
         licencies.back()->setKeyType( sqlQuery->getColumn( 3 ) );

         // Done with this licence
         nbrLicencies++;
         mc2dbg8 << "UP::getUserLicence() " << nbrLicencies << " done" << endl;
      }

      if ( ok ) {
         // Check imei licence keys
         if ( !m_sqlConnection->beginTransaction() ) {
            mc2log << error << "UP::getUserLicence(): update IMEI "
                   << "licencies begin transaction failed." << endl;  
             ok = false;
         }
         for ( size_t i = 0 ; i < licencies.size() && ok ; ++i ) {
            if ( licencies[ i ]->isIMEIKey() ) {
               // Check key
               MC2String imei15digts;
               if ( licencies[ i ]->extract15Digits( imei15digts ) && 
                    imei15digts != licencies[ i ]->getLicenceKeyStr() &&
                    licencies[ i ]->isIMEISV() ) {
                  // Check if key already exists before adding another!
                  vector<uint32> presentUsers;
                  vector<MC2String> presentLogonIDs;
                  MC2String oldLicenceKeyStr( 
                     licencies[ i ]->getLicenceKeyStr() );
                  // Update key
                  licencies[ i ]->setLicence( imei15digts );
                  uint32 nbrPresent = getUINsFromUserLicenceKey( 
                     licencies[ i ], presentUsers, presentLogonIDs, sqlQuery );
                  if ( nbrPresent == 0 ) {
                     // Set in db 
                     if ( !changeUserLicence( UIN, 0/*changerUIN*/, 
                                              licencies[ i ], sqlQuery ) ) {
                        mc2log << error << "UP::getUserLicence(): update IMEI "
                               << "licence failed, id " 
                               << licencies[ i ]->getID() << " key " 
                               << licencies[ i ]->getLicenceKeyStr() << endl;  
                        ok = false;
                     }
                  } else { // End if key not already present
                     // Make it reflect what is in the db
                     licencies[ i ]->setLicence( oldLicenceKeyStr );
                  }
               } // End if needs to trim IMEI
            } // End if is IMEI key
         } // for all licence keys
         if ( ok ) {
            if ( !m_sqlConnection->commitTransaction() ) {
               mc2log << error << "UP::getUserLicence(): update IMEI "
                      << "licences commit failed." << endl;  
               ok = false;
            }
         } else {
            if ( !m_sqlConnection->rollbackTransaction() ) {
               mc2log << error << "UP::getUserLicence(): update IMEI "
                      << "licences rollbackTransaction() failed!" << endl;
               ok = false;
            }
         }
      }
   }

   if ( !ok ) {
      nbrLicencies = -1;
   }

   mc2dbg4 << "UP::getUserLicence() UIN: " 
           << UIN << " has " << nbrLicencies << " licencies." << endl;
  
   return nbrLicencies;
}


int32 
UserProcessor::getUserRegionAccess( uint32 UIN, 
                                    UserRegionAccess**& accesses )
{
   int32 nbrAccesses = 0;
   accesses = NULL;
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserRegionAccess() for UIN: " << UIN << endl;

   sprintf( query, 
            "SELECT COUNT(*) FROM ISABUserRegionAccess WHERE userUIN = %u",
            UIN );

   if ( ! doQuery(sqlQuery, query, "UP::getUserRegionAccess() get count") )
   {
      delete sqlQuery;
      return -1;
   }

   if ( sqlQuery->nextRow())
      nbrAccesses = atoi(sqlQuery->getColumn(0) );

   sprintf( query, "SELECT * FROM ISABUserRegionAccess WHERE userUIN = %u",
            UIN );

   if ( ! doQuery(sqlQuery, query, 
                  "UP::getUserRegionAccess() get regions") )
   {
      delete sqlQuery;
      return -1;
   }

   accesses = new UserRegionAccess*[ nbrAccesses ];
   int i = -1;
   while ( sqlQuery->nextRow() ) {
      i++;
      UserRegionAccess* access;
      mc2dbg8 << "UP::getUserRegionAccess() getting region access " 
              << i << endl;
      access = new UserRegionAccess( atoi( sqlQuery->getColumn( 0 ) ),
                                     atoi( sqlQuery->getColumn( 2 ) ),
                                     atoi( sqlQuery->getColumn( 3 ) ),
                                     atoi( sqlQuery->getColumn( 4 ) ) );
      accesses[ i ] = access;
      // Done with this region access
      mc2dbg8 << "UP::getUserRegionAccess() " << i << " done" << endl;
   }

   // we're done with the query
   delete sqlQuery;

   if ( nbrAccesses == 0 ) {
      mc2dbg4 << "UP::getUserRegionAccess() UIN: " 
              << UIN << " has no region accesses." << endl;
   }
  
   return nbrAccesses;
}


int32 
UserProcessor::getUserRights( uint32 UIN, vector<UserRight*>& rights ) {
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserRight() for UIN: " << UIN << endl;

   sprintf( query, "SELECT * FROM ISABUserRight WHERE userUIN = %u", UIN );

   if ( ! doQuery(sqlQuery, query, 
                  "UP::getUserRights() get rights") )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      UserRight* right = 
         new UserRight( atoi( sqlQuery->getColumn( 0 ) /*id*/ ),
                                                       /*userUIN*/
                        atoi( sqlQuery->getColumn( 2 ) /*addTime*/ ),
                        // Should come signed from DB.
                        UserEnums::URType( 
                           strtoll( 
                              sqlQuery->getColumn( 3 ), NULL, 10)/*type*/),
                        atoi( sqlQuery->getColumn( 4 ) /*regionID*/ ),
                        atoi( sqlQuery->getColumn( 5 ) /*startTime*/ ),
                        atoi( sqlQuery->getColumn( 6 ) /*endTime*/ ),
                        atoi( sqlQuery->getColumn( 7 ) /*deleted*/ ),
                        sqlQuery->getColumn( 8 )       /*origin*/ );
      rights.push_back( right );
   }

   // we're done with the query
   delete sqlQuery;

   if ( rights.empty() ) {
      mc2dbg4 << "UP::getUserRight() UIN: " 
              << UIN << " has no rights." << endl;
   }
  
   return rights.size();
}


int32 
UserProcessor::getUserWayfinderSubscription( 
   uint32 UIN, vector<UserWayfinderSubscription*>& subcr )
{
   int32 nbrSubsr = 0;
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserWayfinderSubscription() for UIN: " << UIN 
           << endl;

   sprintf( query, "SELECT * FROM ISABUserWayfinderSubscription WHERE "
            "userUIN = %u", UIN );

   if ( ! doQuery( sqlQuery, query, 
                   "UP::()getUserWayfinderSubscription get subscr" ) )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      nbrSubsr++;
      UserWayfinderSubscription* subscr = 
         new UserWayfinderSubscription( atoi( sqlQuery->getColumn( 0 ) ),
                                        atoi( sqlQuery->getColumn( 2 ) ) );
      subcr.push_back( subscr );
   }

   // we're done with the query
   delete sqlQuery;

   if ( nbrSubsr == 0 ) {
      mc2dbg4 << "UP::getUserWayfinderSubscription() UIN: " 
              << UIN << " has no Wayfinder subscription." << endl;
   }
  
   return nbrSubsr;
}


int32 
UserProcessor::getUserTokens( uint32 UIN, vector<UserToken*>& tokens ) {
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserTokens() for UIN: " << UIN << endl;

   sprintf( query, "SELECT id, createTime, age, token, group_name "
            "FROM ISABUserToken WHERE userUIN = %u", UIN );

   if ( ! doQuery(sqlQuery, query, 
                  "UP::getUserTokens() get tokens") )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      UserToken* token = 
         new UserToken( atoi( sqlQuery->getColumn( 0 ) /*id*/ ),
                        atoi( sqlQuery->getColumn( 1 ) /*createTime*/ ),
                        atoi( sqlQuery->getColumn( 2 ) /*age*/ ),
                        sqlQuery->getColumn( 3 )       /*token*/,
                        sqlQuery->getColumn( 4 )       /*group_name*/ );
      tokens.push_back( token );
   }

   // we're done with the query
   delete sqlQuery;

   if ( tokens.empty() ) {
      mc2dbg4 << "UP::getUserTokens() UIN: " 
              << UIN << " has no tokens." << endl;
   }
  
   return tokens.size();
}


int32 
UserProcessor::getUserPINs( uint32 UIN, vector<UserPIN*>& PINs ) {
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserPINs() for UIN: " << UIN << endl;

   sprintf( query, "SELECT id, PIN, comment "
            "FROM ISABUserPIN WHERE userUIN = %u", UIN );

   if ( ! doQuery(sqlQuery, query, 
                  "UP::getUserPINs() get PINs") )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      UserPIN* PIN = 
         new UserPIN( atoi( sqlQuery->getColumn( 0 ) /*id*/ ),
                      sqlQuery->getColumn( 1 )       /*PIN*/,
                      sqlQuery->getColumn( 2 )       /*comment*/ );
      PINs.push_back( PIN );
   }

   // we're done with the query
   delete sqlQuery;

   if ( PINs.empty() ) {
      mc2dbg4 << "UP::getUserPINs() UIN: " 
              << UIN << " has no PINs." << endl;
   }
  
   return PINs.size();
}


int32 
UserProcessor::getUserIDKeys( uint32 UIN, vector<UserIDKey*>& els,
                              CharEncSQLQuery* sqlQuery ) {
   char query[ 1024 ];
   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }

   mc2dbg4 << "UP::getUserIDKeys() for UIN: " << UIN << endl;

   sprintf( query, "SELECT id, type, idkey "
            "FROM ISABUserIDKey WHERE userUIN = %u", UIN );

   if ( ! doQuery(sqlQuery, query, 
                  "UP::getUserIDKeys() get Keys") )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      UserIDKey* el = 
         new UserIDKey( atoi( sqlQuery->getColumn( 0 ) /*id*/ ),
                        sqlQuery->getColumn( 2 ),      /*idkey*/
                        UserIDKey::idKey_t( atoi( sqlQuery->getColumn( 1 )
                                                       /*type*/ ) ) );
      els.push_back( el );
   }

   if ( els.empty() ) {
      mc2dbg4 << "UP::getUserIDKeys() UIN: " 
              << UIN << " has no UserIDKeys." << endl;
   }
  
   return els.size();
}


int32 
UserProcessor::getUserLastClients( uint32 UIN, 
                                   vector<UserLastClient*>& els ) 
{
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserLastClients() for UIN: " << UIN << endl;

   sprintf( query, "SELECT id, client_type, client_type_options, "
            "version, extra, origin "
            "FROM ISABUserLastClient WHERE userUIN = %u", UIN );

   if ( ! doQuery( sqlQuery, query, 
                   "UP::getUserLastClients() get Keys" ) )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      UserLastClient* el = 
         new UserLastClient( atoi( sqlQuery->getColumn( 0 ) /*id*/ ),
                             sqlQuery->getColumn( 1 ),      /*client_type*/
                             sqlQuery->getColumn( 2 ),      /*clt_tp_opt*/
                             sqlQuery->getColumn( 3 ),      /*version*/
                             sqlQuery->getColumn( 4 ),      /*extra*/
                             sqlQuery->getColumn( 5 )      /*origin*/
                             );
      els.push_back( el );
   }

   // Get history 
   sprintf( query, "SELECT id, client_type, client_type_options, "
            "version, extra, origin, changeTime, changerUIN "
            "FROM ISABUserLastClientchangelog WHERE userUIN = %u"
            " ORDER BY changeTime DESC", UIN );

   if ( ! doQuery( sqlQuery, query, 
                   "UP::getUserLastClients() get Keys" ) )
   {
      delete sqlQuery;
      return -1;
   }

   while ( sqlQuery->nextRow() ) {
      if ( strcmp( sqlQuery->getColumn( 1 ), "REMOVED" ) != 0 ) {
         UserLastClient* el = 
            new UserLastClient( 
               atoi( sqlQuery->getColumn( 0 ) /*id*/ ),
               sqlQuery->getColumn( 1 ),      /*client_type*/
               sqlQuery->getColumn( 2 ),      /*clt_tp_opt*/
               sqlQuery->getColumn( 3 ),      /*version*/
               sqlQuery->getColumn( 4 ),      /*extra*/
               sqlQuery->getColumn( 5 ),      /*origin*/
               true,
               strtoul( sqlQuery->getColumn( 7 ), NULL, 10 ),
               atoi( sqlQuery->getColumn( 6 ) )
               );
         els.push_back( el );
      }
   }

   // we're done with the query
   delete sqlQuery;

   if ( els.empty() ) {
      mc2dbg4 << "UP::getUserLastClients() UIN: " 
              << UIN << " has no UserLastClients." << endl;
   }
  
   return els.size();
}

uint32 
UserProcessor::getUser( const char* logonID, const char* logonPasswd,
                        bool checkExpired ) 
{
   // Check cache
   bool ok = false;
   uint32 UIN = 0;
#ifdef PARALLEL_USERMODULE
   SessionCacheItem cached;
   char* lowerLogonID = StringUtility::newStrDup(
      MC2String(StringUtility::copyLower( logonID )).c_str());
   if ( m_loginCache->get( lowerLogonID, cached ) ) {
      // Check cachedKey 
      if ( ::checkPassword( cached.key.c_str(), logonPasswd,
                            boost::lexical_cast< MC2String >
                            ( cached.UIN ).c_str() ) ) {
         // Ok!
         ok = true;
         UIN = cached.UIN;
         mc2log << "UP::getUser Got " << lowerLogonID << "(" << UIN 
                << ")"<< " from cache" << endl;
      } else {
         m_loginCache->remove( lowerLogonID );
      }
   }
#else
   int32 now = TimeUtility::getRealTime();
   const char* cachedKey = NULL;
   uint32 cachedUIN = 0;
   uint32 cachedTime = 0;
   char* lowerLogonID = StringUtility::newStrDup(
      MC2String(StringUtility::copyLower( logonID )).c_str());
   if ( m_userLoginCache->getSession( lowerLogonID, cachedKey, cachedUIN,
                                      cachedTime ) )
   {
      // Check cachedKey 
      if ( ::checkPassword( cachedKey, logonPasswd,
                            boost::lexical_cast< MC2String >
                            ( cachedUIN ).c_str() ) ) {
         // Check cachedTime
         if ( now - int32(cachedTime) < 
              sessionValidTime/*Check sometimes*/ ) 
         {
            // Ok!
            ok = true;
            UIN = cachedUIN;
            mc2log << "UP::getUser Got " << lowerLogonID << "(" << UIN 
                   << ")"<< " from cache" << endl;
         } else {
            m_userLoginCache->removeSession( lowerLogonID );
         }
      } else {
         m_userLoginCache->removeSession( lowerLogonID );
      }
   }
#endif

   if ( !ok ) {   
      // Check database
      char query[1024];
      CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

      sprintf( query,
               "SELECT UIN, logonPasswd, validDate FROM ISABUserUser "
               "WHERE logonID = '%s'", lowerLogonID );

      if ( ! doQuery(sqlQuery, query, "UP::getUser()") ) {
         delete sqlQuery;
         delete [] lowerLogonID;
         return 0;
      }
  
      if ( sqlQuery->nextRow() ) {
         //  Found one user, check password!
         const char* pwd = sqlQuery->getColumn( 1 );
         const char* uinStr = sqlQuery->getColumn( 0 );
         // If password matches the hashed value in database
         // or if it matches the possible unhashed value in database
         // then login was successful ( unless user expired )
         if ( ::checkPassword( pwd, logonPasswd,
                               // use UIN as seed
                               uinStr ) ){

            // OK, correct password
            UIN = strtoul( uinStr, NULL, 10);
            if ( checkExpired ) {
               // Check valid until
               uint32 now = TimeUtility::getRealTime();
               uint32 validDate = StringUtility::makeDate( 
                  sqlQuery->getColumn( 2 ) );
               if ( validDate < now && validDate != 0 ) {
                  mc2log << info << "UP::getUser() for logonId [" 
                         << logonID << "] User is expired, validDate " 
                         << sqlQuery->getColumn( 2 ) << endl;
                  UIN = MAX_UINT32 - 1;
               }
            }
            if ( UIN != 0 && UIN != MAX_UINT32 - 1 ) {
               // set the hashed password to cache
               MC2String hashedLogonPasswd = 
                  UserPassword::create( logonPasswd, uinStr );

               // Add to cache
#ifdef PARALLEL_USERMODULE
               m_loginCache->add( lowerLogonID, 
                                  SessionCacheItem( hashedLogonPasswd, UIN, 0 ),
                                  sessionValidTime );
#else
               m_userLoginCache->addSession( lowerLogonID, hashedLogonPasswd, 
                                             UIN, now );
#endif
            }
            if ( sqlQuery->nextRow() > 1 ) {
               mc2log << warn << "UP::getUser() for logonId [" 
                      << logonID << "] DATABASE ERRROR: More than one "
                      << "user with identical logonId!" << endl;
            }
         } else {
            mc2log << info << "UP::getUser() for logonID [" << logonID
                   << "] wrong password, logonPasswd: [" << logonPasswd
                   << "], pwd: [" << pwd << "]" << endl;
         }
      } else {
         mc2log << info << "UP::getUser() for logonID [" << logonID
                << "] logon failed: unknown user " << endl;
      }
      delete sqlQuery;
   } // End if !ok

   delete [] lowerLogonID;
   return UIN;
}


int32 
UserProcessor::getUserBuddyLists( uint32 UIN, UserBuddyList**& buddies ) {
   int32 nbrBuddyLists = 0;
   buddies = NULL;
   char query[1024];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   UserBuddyList* buddy;

   mc2dbg4 << "UP::getUserBuddyLists() for UIN: " << UIN << endl;

   sprintf( query, 
            "SELECT COUNT(*) FROM ISABUserBuddyList WHERE userUIN = %u", UIN);

   if ( ! doQuery(sqlQuery, query, "UP::getUserBuddyLists() get count") ) {
      delete sqlQuery;
      return -1;
   }

   if (sqlQuery->nextRow())
      nbrBuddyLists = atoi(sqlQuery->getColumn(0));

   mc2dbg4 << "UP::getUserBuddyLists getting " 
             << nbrBuddyLists << " BuddyLists" << endl;

   buddies = new UserBuddyList*[ nbrBuddyLists ];

   if (nbrBuddyLists > 0) {
      sprintf(query, "SELECT id, name, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10 "
              "FROM ISABUserBuddyList "
              "WHERE userUIN = %u",
              UIN);

      if ( ! doQuery(sqlQuery, query, "UP::getCellularPhoneModel()") ) {
         delete sqlQuery;
         return -1;
      }

      int i = -1;
      while (sqlQuery->nextRow()) {
         i++;
         buddy = new UserBuddyList( atoi( sqlQuery->getColumn(0)));
         buddies[i] = buddy;
         buddy->setName               ( sqlQuery->getColumn(1)      );

         // And the buddies
         for ( uint32 j = 1 ; j <= UserBuddyList::MAX_NBR_BUDDIES ; j++ )
            buddy->setBuddyAt( j - 1, atoi(sqlQuery->getColumn(1+j))); 

         // Done with this buddyList
         mc2dbg8 << "UP::getUserBuddyList " << i << " done"<< endl;
      }
   }

   mc2dbg8 << "UP::getUserBuddyList done, returning now."<< endl;
  
   delete sqlQuery;
   return nbrBuddyLists;
}


int32 
UserProcessor::getUserNavigators( uint32 UIN, UserNavigator**& navis ) {
   uint32 nbrNavis = 0;
   char query[1024];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   mc2dbg4 << "UP::getUserNavigators() for UIN: " << UIN << endl;

   DBUserNavigator* tmpNav = new DBUserNavigator();
   sprintf( query, "SELECT COUNT(*) FROM %s where %s = %d",
            DBUserNavigator::getTableName(),
            tmpNav->getFieldNames()[ DBUserNavigator::field_userUIN ],
            UIN );

   if ( ! doQuery(sqlQuery, query, "UP::getUserNavigators() get count") ) {
      delete sqlQuery;
      delete tmpNav;
      return -1;
   }

   if (sqlQuery->nextRow())
      nbrNavis = atoi(sqlQuery->getColumn(0));

   int i = 0;
   if (nbrNavis > 0) {
      sprintf( query, "SELECT * FROM %s where %s = %d",
               DBUserNavigator::getTableName(),
               tmpNav->getFieldNames()[ DBUserNavigator::field_userUIN ],
               UIN );

      if ( ! doQuery(sqlQuery, query, "UP::getUserNavigators() get navigators")) {
         delete sqlQuery;
         return -1;
      }
      navis = new UserNavigator*[ nbrNavis ];
      while (sqlQuery->nextRow()) {
         navis[ i++ ] = new UserNavigator( sqlQuery );
      }
   } else {
      mc2dbg4 << "UP::getUserNavigators() no Navigators for user."
              << endl; 
      navis = NULL;
   }

   delete tmpNav;
   delete sqlQuery;
   return nbrNavis;
}


uint32 
UserProcessor::getUINsFromUserUser( const UserUser* user, 
                                    vector<uint32>& users, 
                                    vector<MC2String>& logonIDs,
                                    CharEncSQLQuery* sqlQuery ) 
{
   char query[4096];
   char queryWhere[4096];
   uint32 pos = 0;
   uint32 nbrFields = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_NBRFIELDS ; i++ ) {
      if ( user->changed( UserConstants::UserDataField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( queryWhere + pos, " AND " );
            pos += 5;   
         }
         nbrFields++;
         strcpy( queryWhere + pos, UserConstants::UserUserFieldName[ i ] );
         pos += strlen( UserConstants::UserUserFieldName[ i ] );
         mc2dbg4 << "UP::getUINsFromUserUser(): useruser adding: " 
                 << UserConstants::UserUserFieldName[ i ] << endl;
         if ( i == UserConstants::USER_LOGONID || true ) {
            strcpy( queryWhere + pos, " LIKE " );
            pos += 6;
         } else {
            strcpy( queryWhere + pos, " = " );
            pos += 3;
         }
         pos += user->printValue( queryWhere + pos, 
                                  UserConstants::UserDataField( i ) ); 
      }
   }
   if ( nbrFields == 0 ) {
      mc2log << error << "UP::getUINsFromUserUser() "
             << "No data to find user with!" << endl;
      return 0;
   }

   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }
   strcpy( query, "SELECT UIN, logonID FROM ISABUserUser WHERE ");
   strcat( query, queryWhere );

   if ( !doQuery(sqlQuery, query, "UP::getUINsFromUserUser() normal" ) ) {
      mc2log << error << "UP::getUINsFromUserUser(): failed!"
             << endl;
      return 0;
   }
  
   while ( sqlQuery->nextRow() ) {
      mc2dbg8 << "UP::getUINsFromUserUser(): UIN[" << users.size() 
              << "] = " << sqlQuery->getColumn( 0 ) << endl;
      logonIDs.push_back( sqlQuery->getColumn( 1 ) );
      users.push_back( strtoul( sqlQuery->getColumn( 0 ), NULL, 10 ) );
      mc2dbg8 << "UP::getUINsFromUserUser(): UIN[" << (users.size() - 1) 
              << "]: " << users.back() << endl;
   }

   return users.size();
}

uint32 
UserProcessor::getUINsFromUserCellular( const UserCellular* cellular, 
                                        vector<uint32>& users, 
                                        vector<MC2String>& logonIDs,
                                        CharEncSQLQuery* sqlQuery ) 
{
   char query[4096];
   char queryWhere[4096];
   uint32 nbrFields = 0;
   uint32 pos = 0; 
   
   for ( uint32 i = 0 ; i < UserConstants::CELLULAR_NBRFIELDS ; i++ ) {
      if ( cellular->changed( UserConstants::UserCellularField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( queryWhere + pos, " AND " );
            pos += 5;   
         }
         nbrFields++;
         strcpy( queryWhere + pos, UserConstants::UserCellularFieldName[ i ] );
         pos += strlen( UserConstants::UserCellularFieldName[ i ] );
         if ( i == UserConstants::CELLULAR_PHONENUMBER || true ) {
            strcpy( queryWhere + pos, " LIKE " );
            pos += 6;
         } else {
            strcpy( queryWhere + pos, " = " );
            pos += 3;
         }
         pos += cellular->printValue( queryWhere + pos, 
                                      UserConstants::UserCellularField( i ) );
      }
   }
   if ( nbrFields == 0 ) {
      mc2log << error << "UP::getUINsFromUserCellular(): " 
             << "No data to find user with!" << endl;
      return 0;
   }

   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }
   strcpy(query,"SELECT DISTINCT userUIN, logonID FROM ISABUserCellular, "
          "ISABUserUser WHERE UIN = userUIN AND ");
   strcat(query, queryWhere);

   if ( !doQuery(sqlQuery, query, "UP::getUINsFromUserCellular() select") )
   {
      mc2log << error << "UP::getUINsFromUserCellular() "
             << "Failed to get users, aborting." << endl;
      return 0;
   }

   while ( sqlQuery->nextRow() ) {
      logonIDs.push_back( sqlQuery->getColumn( 1 ) );
      users.push_back( strtoul( sqlQuery->getColumn( 0 ), NULL, 10 ) );
   }

   return users.size();
}


uint32 
UserProcessor::getUINsFromUserLicenceKey( 
   const UserLicenceKey* licence, 
   vector<uint32>& users, 
   vector<MC2String>& logonIDs,
   CharEncSQLQuery* sqlQuery )
{
   char query[4096];
   char queryWhere[4096];
   int pos = 0;
      
   // Select using licence
   pos += sprintf( queryWhere, " %s LIKE ",
                   UserConstants::UserLicenceKeyFieldName[ 
                      UserConstants::USER_LICENCE_KEY ] );
   licence->printOldKeyValue( queryWhere + pos );

   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }

   strcpy( query, "SELECT DISTINCT userUIN, logonID FROM ISABUserLicence, "
           "ISABUserUser WHERE UIN = userUIN AND "
           );
   strcat( query, queryWhere );

   if ( !doQuery( sqlQuery, query, "UP::getUINsFromUserLicence() select" ))
   {
      mc2log << error << "UP::getUINsFromUserLicence() "
             << "Failed to get old users, aborting." << endl;
      return 0;
   }

   while ( sqlQuery->nextRow() ) {
      logonIDs.push_back( sqlQuery->getColumn( 1 ) );
      users.push_back( strtoul( sqlQuery->getColumn( 0 ), NULL, 10 ) );
   }

   // Make abbreviated imei
   bool testImei = licence->getKeyType().empty() || 
      licence->isIMEIKey();
   MC2String imei15digts;
   testImei = testImei && licence->extract15Digits( imei15digts ) &&
      (imei15digts != licence->getLicenceKeyStr()) &&
      licence->isIMEISV();

   // Both Tables
   strcpy( query, "SELECT DISTINCT userUIN, logonID, licenceKey FROM "
           "ISABUserLicenceKey, ISABUserUser WHERE UIN = userUIN AND " );
   pos = 0;
   if ( testImei ) {
      pos += sprintf( queryWhere + pos, "( " );
   }
   pos += sprintf( queryWhere + pos, " %s LIKE ",
                   UserConstants::UserLicenceKeyFieldName[ 
                      UserConstants::USER_LICENCE_KEY ] );
   pos += licence->printValue( queryWhere + pos,
                               UserConstants::USER_LICENCE_KEY );
   if ( testImei ) {
      pos += sprintf( queryWhere + pos, " OR %s LIKE '%s'",
                      UserConstants::UserLicenceKeyFieldName[ 
                         UserConstants::USER_LICENCE_KEY ],
                      imei15digts.c_str() );
      pos += sprintf( queryWhere + pos, " OR %s LIKE '%s__'",
                      UserConstants::UserLicenceKeyFieldName[ 
                         UserConstants::USER_LICENCE_KEY ],
                      imei15digts.c_str() );                      
      pos += sprintf( queryWhere + pos, " )" );
   }
   if ( !licence->getKeyType().empty() ) {
      pos += sprintf( queryWhere + pos, " AND %s LIKE ", 
                      UserConstants::UserLicenceKeyFieldName[ 
                         UserConstants::USER_LICENCE_KEY_TYPE ] );
      pos += licence->printValue( queryWhere + pos, 
                                  UserConstants::USER_LICENCE_KEY_TYPE );
   }
   if ( licence->changed( UserConstants::USER_LICENCE_PRODUCT ) ) {
      pos += sprintf( queryWhere + pos, " AND %s LIKE ", 
                      UserConstants::UserLicenceKeyFieldName[ 
                         UserConstants::USER_LICENCE_PRODUCT ] );
      pos += licence->printValue( queryWhere + pos, 
                                  UserConstants::USER_LICENCE_PRODUCT );
   }
   strcat( query, queryWhere );

   if ( !doQuery( sqlQuery, query, "UP::getUINsFromUserLicence() select2" ) )
   {
      mc2log << error << "UP::getUINsFromUserLicence() "
             << "Failed to get new users, aborting." << endl;
      return 0;
   }

   uint32 fullIMEIUIN = 0 ;
   MC2String fullIMEIlogonID;
   uint32 trimmedIMEIUIN = 0 ;
   MC2String trimmedIMEIlogonID;
   uint32 fullIMEISVUIN = 0;
   MC2String fullIMEISVlogonID;
   while ( sqlQuery->nextRow() ) {
      MC2String userUIN = sqlQuery->getColumn( 0 );
      MC2String logonID = sqlQuery->getColumn( 1 );
      MC2String licenceKey = sqlQuery->getColumn( 2 );

      if ( testImei && imei15digts == licenceKey ) {
         trimmedIMEIUIN = strtoul( userUIN.c_str(), NULL, 10 );
         trimmedIMEIlogonID = logonID;
      } else if ( testImei && 
                  licence->getLicenceKeyStr() == licenceKey ) {
         fullIMEIUIN = strtoul( userUIN.c_str(), NULL, 10 );
         fullIMEIlogonID = logonID;
      } else if ( testImei &&
                  StringUtility::regexp( "^"+imei15digts+"[0-9]{2}$",
                                         licenceKey ) ) {
         fullIMEISVUIN = strtoul( userUIN.c_str(), NULL, 10 );
         fullIMEISVlogonID = logonID;
      } else {
         logonIDs.push_back( logonID );
         users.push_back( strtoul( userUIN.c_str(), NULL, 10 ) );
      }
   }
   if ( trimmedIMEIUIN != 0 ) {
      logonIDs.push_back( trimmedIMEIlogonID );
      users.push_back( trimmedIMEIUIN );
   } else if ( fullIMEIUIN != 0 ) {
      logonIDs.push_back( fullIMEIlogonID );
      users.push_back( fullIMEIUIN );
   } else if ( fullIMEISVUIN != 0 ) {
      logonIDs.push_back( fullIMEISVlogonID );
      users.push_back( fullIMEISVUIN );
   }

   return users.size();
}


uint32 
UserProcessor::getUINsFromUserWayfinderSubscription( 
   const UserWayfinderSubscription* subscr, 
   vector<uint32>& users, 
   vector<MC2String>& logonIDs,
   CharEncSQLQuery* sqlQuery )
{
   char query[4096];
   char queryWhere[4096];
   int pos = 0;
      
   // Select using type
   pos += sprintf( 
      queryWhere, " %s like ",
      UserConstants::UserWayfinderSubscriptionFieldName[ 
         UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE ] );
   subscr->printValue( 
      queryWhere + pos, UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE );

   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }

   strcpy( query,"SELECT DISTINCT userUIN, logonID FROM "
           "ISABUserWayfinderSubscription, "
           "ISABUserUser WHERE UIN = userUIN AND " );
   strcat( query, queryWhere );

   if ( !doQuery( sqlQuery, query, 
                  "UP::getUINsFromUserWayfinderSubscription() select" ) )
   {
      mc2log << error << "UP::getUINsFromUserWayfinderSubscription() "
             << "Failed to get users, aborting." << endl;
      return 0;
   }

   while ( sqlQuery->nextRow() ) {
      logonIDs.push_back( sqlQuery->getColumn( 1 ) );
      users.push_back( strtoul( sqlQuery->getColumn( 0 ), NULL, 10 ) );
   }

   return users.size();
}


uint32 
UserProcessor::getUINsFromUserIDKey( const UserIDKey* key, 
                                     vector<uint32>& users, 
                                     vector<MC2String>& logonIDs,
                                     CharEncSQLQuery* sqlQuery )
{
   char query[4096];
   char queryWhere[4096];
   int pos = 0;
      
   // Select using idkey
   pos += sprintf( queryWhere, " %s like ",
                   UserConstants::UserIDKeyFieldName[ 
                      UserConstants::USER_ID_KEY_KEY ] );
   pos += key->printValue( 
      queryWhere + pos, UserConstants::USER_ID_KEY_KEY );

   if ( key->changed( UserConstants::USER_ID_KEY_TYPE ) ) {
      pos += sprintf( queryWhere + pos, " AND %s = %d",
                   UserConstants::UserIDKeyFieldName[ 
                      UserConstants::USER_ID_KEY_TYPE ],
                      key->getIDType() );
   }

   auto_ptr<CharEncSQLQuery> deleteSqlQuery;
   if ( sqlQuery == NULL ) {
      deleteSqlQuery.reset( m_sqlConnection->newQuery() );
      sqlQuery = deleteSqlQuery.get();
   }

   strcpy( query,"SELECT DISTINCT userUIN, logonID FROM ISABUserIDKey, "
           "ISABUserUser WHERE UIN = userUIN AND " );
   strcat( query, queryWhere );

   if ( !doQuery( sqlQuery, query, "UP::getUINsFromUserIDKey() select" ))
   {
      mc2log << error << "UP::getUINsFromUserIDKey() "
             << "Failed to get users, aborting." << endl;
      return 0;
   }

   while ( sqlQuery->nextRow() ) {
      logonIDs.push_back( sqlQuery->getColumn( 1 ) );
      users.push_back( strtoul( sqlQuery->getColumn( 0 ), NULL, 10 ) );
   }

   return users.size();
}


uint32 
UserProcessor::getNewUniqueID(const char* tableName, const char* colName,
                              int instance)
{
   char query[1024];
   bool ok = false;
   uint32 id = 0;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
  
   while (!ok) {
      if (instance > -1) {
         if (instance > 7) // too big, only 3 bits! (see below)
            return 0;
         // max 0xffffff0, ie reserve the upper 4 bits in the uint32, the
         // highest bit set indicates that the following 3 bits is the
         // "UIN instance" to use. The lowest tuple is reserved for error
         // states due to historical reasons...
         id = 1+(uint32)((float64)0xffffff0*rand()/(RAND_MAX + 1.0));
         id = id | 0x80000000 | (instance << 28);
      } else {
         id = 1+(uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));
      }
      sprintf( query, "SELECT %s FROM %s WHERE %s = %u", colName, tableName,
               colName, id);
      if ( ! doQuery(sqlQuery, query, "UP::getNewUniqueID()") ) {
         mc2log << error << "UP::getNewUniqueID() giving up!" << endl;
         delete sqlQuery;
         return 0;
      } else {
         ok = ( ! sqlQuery->nextRow());
      }
   }

   delete sqlQuery;
   return id;
}

uint32 
UserProcessor::addUserUserData( char* target, UserUser* user,
                                uint32 UIN, const char* passwd ) {
   int pos = 0;
   MC2String uinStr = boost::lexical_cast< MC2String >( UIN );
   // UIN
   pos += sprintf( target, "%s, ", uinStr.c_str() );
   // logonID
   strcpy( target + pos, "'" );
   pos++;
   char* lowerLogonID = StringUtility::newStrDup(
      MC2String(StringUtility::copyLower(user->getLogonID())).c_str());
   pos += UserElement::sqlString( target + pos, lowerLogonID );
   delete[] lowerLogonID;
   lowerLogonID = NULL;

   strcpy( target + pos, "' " );
   pos += 2;
   // logonPasswd
   strcpy( target + pos, ", '");
   pos += 3;
   MC2String userPw = UserPassword::create( passwd, uinStr.c_str() );
   pos += UserElement::sqlString( target + pos, 
                                  userPw.c_str() );

   strcpy( target + pos, "' " );
   pos += 2;
   
   for ( uint32 i =  UserConstants::USER_FIRSTNAME ; 
         i < UserConstants::USER_NBRFIELDS 
            ; i++ ) 
   {
      if ( i != 0 ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += user->printValue( target + pos, 
                               UserConstants::UserDataField( i ) );
   }
   return pos;
}


uint32 
UserProcessor::getNewCellularID() {
   char query[1024];
   bool ok = false;
   uint32 ID = 0;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   while (!ok) {
      ID = 1 + (uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));
      sprintf(query,
              "SELECT userUIN FROM ISABUserCellular WHERE id = %u",
              ID);
      if ( ! doQuery(sqlQuery, query, "UserProcess::getNewCellularID()") ) {
         mc2log << error << "UP::getNewCellularID(): giving up!"
                << endl;
         delete sqlQuery;
         return 0;
      } else {
         ok = ( !sqlQuery->nextRow());
      }
   }

   delete sqlQuery;
   return ID;
}


uint32 
UserProcessor::getNewLicenceID() {
   char query[ 1024 ];
   bool ok = false;
   uint32 ID = 0;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   while (!ok) {
      ID = 1 + (uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));
      sprintf(query,
              "SELECT DISTINCT userUIN FROM ISABUserLicenceKey WHERE id = %u",
              ID);
      if ( ! doQuery(sqlQuery, query, "UserProcess::getNewLicenceID()") ) {
         mc2log << error << "UP::getNewLicenceID(): giving up!" << endl;
         delete sqlQuery;
         return 0;
      } else {
         ok = ( !sqlQuery->nextRow());
      }
   }

   delete sqlQuery;
   return ID;
}


uint32 
UserProcessor::getNewRegionAccessID() {
   char query[ 1024 ];
   bool ok = false;
   uint32 ID = 0;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   while (!ok) {
      ID = 1 + (uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));
      sprintf(query,
              "SELECT DISTINCT userUIN FROM ISABUserRegionAccess "
              "WHERE id = %u",
              ID);
      if ( ! doQuery(sqlQuery, query, 
                     "UserProcess::getNewRegionAccessID()") ) 
      {
         mc2log << error << "UP::getNewRegionAccessID(): giving up!" 
                << endl;
         delete sqlQuery;
         return 0;
      } else {
         ok = ( !sqlQuery->nextRow());
      }
   }

   delete sqlQuery;
   return ID;
}



uint32
UserProcessor::addUserCellularData( char* target, uint32 ID, uint32 UIN,
                                    UserCellular* cellular )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::CELLULAR_PHONENUMBER ; 
         i < UserConstants::CELLULAR_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::CELLULAR_PHONENUMBER ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += cellular->printValue( target + pos, 
                                   UserConstants::UserCellularField( i ) );
   }
   return pos;
}


uint32
UserProcessor::addUserLicenceData( char* target, uint32 ID, uint32 UIN, 
                                   UserLicenceKey* licence )
{
   int pos = 0;

   pos = sprintf( target, "INSERT INTO ISABUserLicenceKey ( id, userUIN " );
              

   for ( uint32 i = UserConstants::USER_LICENCE_KEY ; 
         i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; 
         i++ ) 
   {
      strcpy( target + pos, ", " );
      pos += 2;
      pos += sprintf( target + pos, "%s", 
                      UserConstants::UserLicenceKeyFieldName[ i ] );
   }

   pos += sprintf( target + pos, " ) VALUES ( %d, %u ", ID, UIN );
   
   for ( uint32 i = UserConstants::USER_LICENCE_KEY ; 
         i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ; 
         i++ ) 
   {
      strcpy( target + pos, ", " );
      pos += 2;
      pos += licence->printValueIMEIFix( 
         target + pos, UserConstants::UserLicenceKeyField( i ) );
   }

   strcpy( target + pos, " )" ); 
   pos += 2;

   return pos;
}


uint32 
UserProcessor::addUserRegionAccessData( char* target, uint32 ID, 
                                        uint32 UIN, 
                                        UserRegionAccess* access )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::USER_REGION_ACCESS_REGION_ID ; 
         i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::USER_REGION_ACCESS_REGION_ID ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += access->printValue( 
         target + pos, UserConstants::UserRegionAccessField( i ) );
   }
   return pos;  
}


uint32 
UserProcessor::addUserRightData( char* target, uint32 ID, 
                                 uint32 UIN, UserRight* right )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::USER_RIGHT_ADD_TIME ; 
         i < UserConstants::USER_RIGHT_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::USER_RIGHT_ADD_TIME ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += right->printValue( 
         target + pos, UserConstants::UserRightField( i ) );
   }
   return pos;  
}


uint32 
UserProcessor::addUserWayfinderSubscriptionData( 
   char* target, uint32 ID, uint32 UIN, 
   UserWayfinderSubscription* wfsubscr )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   // Type
   pos += wfsubscr->printValue( 
      target + pos, UserConstants::USER_WAYFINDER_SUBSCRIPTION_TYPE );
   return pos;
}


uint32 
UserProcessor::addUserTokenData( char* target, uint32 ID, 
                                 uint32 UIN, UserToken* token )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::USER_TOKEN_CREATE_TIME ; 
         i < UserConstants::USER_TOKEN_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::USER_TOKEN_CREATE_TIME ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += token->printValue( 
         target + pos, UserConstants::UserTokenField( i ) );
   }
   return pos;  
}


uint32 
UserProcessor::addUserPINData( char* target, uint32 ID, 
                                 uint32 UIN, UserPIN* PIN )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::USER_PIN_PIN ; 
         i < UserConstants::USER_PIN_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::USER_PIN_PIN ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += PIN->printValue( 
         target + pos, UserConstants::UserPINField( i ) );
   }
   return pos;  
}


uint32 
UserProcessor::addUserIDKeyData( char* target, uint32 ID, 
                                 uint32 UIN, UserIDKey* idKey )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::USER_ID_KEY_TYPE ; 
         i < UserConstants::USER_ID_KEY_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::USER_ID_KEY_TYPE ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += idKey->printValue( 
         target + pos, UserConstants::UserIDKeyField( i ) );
   }
   return pos;  
}


uint32 
UserProcessor::addUserLastClientData( 
   char* target, uint32 ID, 
   uint32 UIN, UserLastClient* lastClient )
{
   int pos = 0;
   // ID
   pos += sprintf( target + pos, "%d, ", ID );
   // UIN
   pos += sprintf( target + pos, "%u, ", UIN );
   
   for ( uint32 i = UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ; 
         i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != UserConstants::USER_LAST_CLIENT_CLIENT_TYPE ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += lastClient->printValue( 
         target + pos, UserConstants::UserLastClientField( i ) );
   }
   return pos;  
}


uint32 
UserProcessor::addChangedUserUserData( char* target, UserUser* user ) {
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_NBRFIELDS ; i++ ) {
      if ( user->changed( UserConstants::UserDataField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, UserConstants::UserUserFieldName[ i ] );
         pos += strlen( UserConstants::UserUserFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += user->printValue( target + pos, 
                                  UserConstants::UserDataField( i ) );
      }
   }
   return strlen( target );
}


uint32
UserProcessor::addChangedUserCellularData( char* target, 
                                           UserCellular* cellular )
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::CELLULAR_NBRFIELDS ; i++ ) {
      if ( cellular->changed( UserConstants::UserCellularField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, UserConstants::UserCellularFieldName[ i ] );
         pos += strlen( UserConstants::UserCellularFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += cellular->printValue( 
            target + pos, 
            UserConstants::UserCellularField( i ) );
      }
   }
   return strlen( target );
}


uint32
UserProcessor::addChangedUserLicenceData( char* target, 
                                          const UserLicenceKey* licence )
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_LICENCE_KEY_NBRFIELDS ;
         i++ ) 
   {
      if ( licence->changed( UserConstants::UserLicenceKeyField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, 
                 UserConstants::UserLicenceKeyFieldName[ i ] );
         pos += strlen( UserConstants::UserLicenceKeyFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += licence->printValueIMEIFix( 
            target + pos, 
            UserConstants::UserLicenceKeyField( i ) );
      }
   }
   return strlen( target );
}


uint32 
UserProcessor::addChangedUserRegionAccessData( char* target, 
                                               UserRegionAccess* access )
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_REGION_ACCESS_NBRFIELDS ;
         i++ ) 
   {
      if ( access->changed( UserConstants::UserRegionAccessField( i ) ) ){
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, 
                 UserConstants::UserRegionAccessFieldName[ i ] );
         pos += strlen( UserConstants::UserRegionAccessFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += access->printValue( 
            target + pos, 
            UserConstants::UserRegionAccessField( i ) );
      }
   }
   return strlen( target );   
}


uint32 
UserProcessor::addChangedUserRightData( char* target, UserRight* right ) {
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_RIGHT_NBRFIELDS ; i++ ) {
      if ( right->changed( UserConstants::UserRightField( i ) ) ){
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, 
                 UserConstants::UserRightFieldName[ i ] );
         pos += strlen( UserConstants::UserRightFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += right->printValue( 
            target + pos, UserConstants::UserRightField( i ) );
      }
   }
   return strlen( target );   
}


uint32 
UserProcessor::addChangedUserWayfinderSubscriptionData( 
   char* target, UserWayfinderSubscription* subscr )
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; 
         i < UserConstants::USER_WAYFINDER_SUBSCRIPTION_NBRFIELDS ;
         i++ ) 
   {
      if ( subscr->changed( 
              UserConstants::UserWayfinderSubscriptionField( i ) ) )
      {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, 
                 UserConstants::UserWayfinderSubscriptionFieldName[ i ] );
         pos += strlen( 
            UserConstants::UserWayfinderSubscriptionFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += subscr->printValue( 
            target + pos, 
            UserConstants::UserWayfinderSubscriptionField( i ) );
      }
   }

   return strlen( target );   
}


uint32 
UserProcessor::addChangedUserTokenData( char* target, UserToken* token ) {
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_TOKEN_NBRFIELDS ; i++ ) {
      if ( token->changed( UserConstants::UserTokenField( i ) ) ){
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, UserConstants::UserTokenFieldName[ i ] );
         pos += strlen( UserConstants::UserTokenFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += token->printValue( 
            target + pos, UserConstants::UserTokenField( i ) );
      }
   }
   return strlen( target );   
}


uint32 
UserProcessor::addChangedUserPINData( char* target, UserPIN* PIN ) {
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_PIN_NBRFIELDS ; i++ ) {
      if ( PIN->changed( UserConstants::UserPINField( i ) ) ){
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, UserConstants::UserPINFieldName[ i ] );
         pos += strlen( UserConstants::UserPINFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += PIN->printValue( 
            target + pos, UserConstants::UserPINField( i ) );
      }
   }
   return strlen( target );   
}


uint32 
UserProcessor::addChangedUserIDKeyData( 
   char* target, const UserIDKey* idKey ) const 
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_ID_KEY_NBRFIELDS ; i++ ) {
      if ( idKey->changed( UserConstants::UserIDKeyField( i ) ) ){
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, UserConstants::UserIDKeyFieldName[ i ] );
         pos += strlen( UserConstants::UserIDKeyFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += idKey->printValue( 
            target + pos, UserConstants::UserIDKeyField( i ) );
      }
   }
   return strlen( target );   
}


uint32 
UserProcessor::addChangedUserLastClientData( char* target, 
                                             UserLastClient* lastClient ) 
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::USER_LAST_CLIENT_NBRFIELDS ; 
         i++ ) 
   {
      if ( lastClient->changed( 
              UserConstants::UserLastClientField( i ) ) )
      {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, 
                 UserConstants::UserLastClientFieldName[ i ] );
         pos += strlen( UserConstants::UserLastClientFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += lastClient->printValue( 
            target + pos, UserConstants::UserLastClientField( i ) );
      }
   }
   return strlen( target );   
}


uint32 
UserProcessor::getNewBuddyListID() {
   char query[1024];
   bool ok = false;
   uint32 ID = 0;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   while ( !ok ) {
      ID = 1 + (uint32)((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));
      sprintf(query,
              "SELECT DISTINCT userUIN FROM ISABUserBuddyList WHERE id = %u",
              ID);
      if ( ! doQuery(sqlQuery, query, "UP::getNewBuddyListID()") ) {
         mc2log << error << "UP::getNewBuddyListID(): giving up!"
                << endl;
         delete sqlQuery;
         return 0;
      } else {
         ok = ( !sqlQuery->nextRow());
      } 
   }
   delete sqlQuery;
   return ID;
}


uint32 
UserProcessor::addUserBuddyListData( char* target, UserBuddyList* buddy ) {
   int pos = 0;
   
   for ( uint32 i = 0 ; 
         i < UserConstants::BUDDY_BUDDIES ; 
         i++ ) 
   {
      if ( i != 0 ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += buddy->printValue( target + pos, 
                                UserConstants::UserBuddyListField( i ) );
   }
   // Add buddies
   char buddyID[40];
   buddy->printValue( 
      buddyID, 
      UserConstants::UserBuddyListField( UserConstants::BUDDY_BUDDIES ) );
   for ( uint32 j = 0 ; j < UserBuddyList::MAX_NBR_BUDDIES ; j++ ) {
      strcpy( target + pos, ", " );
      pos += 2;
      pos += sprintf( target + pos, buddyID, buddy->getBuddy( j ) );
   }

   return pos;
}


uint32 
UserProcessor::addChangedUserBuddyListData( char* target, 
                                            UserBuddyList* buddy )
{
   uint32 nbrFields = 0;
   int pos = 0;

   for ( uint32 i = 0 ; i < UserConstants::BUDDY_BUDDIES ; i++ ) {
      if ( buddy->changed( UserConstants::UserBuddyListField( i ) ) ) {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         nbrFields++;
         strcpy( target + pos, 
                 UserConstants::UserBuddyListFieldName[ i ] );
         pos += strlen( UserConstants::UserBuddyListFieldName[ i ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += buddy->printValue( 
            target + pos, UserConstants::UserBuddyListField( i ) );
      }
   }
   
   if ( buddy->changed( UserConstants::UserBuddyListField( 
      UserConstants::BUDDY_BUDDIES ) ) ) 
   {
      char buddyID[ 40 ];
      buddy->printValue( 
         buddyID, 
         UserConstants::UserBuddyListField( UserConstants::BUDDY_BUDDIES ) 
         );
      for ( uint32 j = 0; j < UserBuddyList::MAX_NBR_BUDDIES ; j++ ) 
      {
         if ( nbrFields != 0 ) {
            strcpy( target + pos, ", " );
            pos += 2;
         }
         strcpy( target + pos, 
                 UserConstants::UserBuddyListFieldName[ 
                    UserConstants::BUDDY_BUDDIES  ] );
         pos += strlen( 
            UserConstants::UserBuddyListFieldName[ 
               UserConstants::BUDDY_BUDDIES ] );
         strcpy( target + pos, " = " );
         pos += 3;
         pos += sprintf( target + pos, buddyID, buddy->getBuddy( j ) );
      }
   }
   
   return strlen( target );
}


uint32 
UserProcessor::addDebitData( char* target, 
                             uint32 debitTime, 
                             uint32 UIN,
                             const char* serverID, 
                             const char* userOrigin, 
                             uint32 messageID,
                             uint32 debInfo,
                             uint32 operationType, 
                             uint32 sentSize,
                             const char* operationDescription )
{
   int pos = 0;

   // Date and time
   pos = 14 + makeSQLDate( debitTime, target, target + 14 );
   target[12] = ',';
   target[13] = ' ';
   // UIN
   pos += sprintf( target + pos, ", %u", UIN );

   // ServerID
   pos += sprintf( target + pos, ", '%s'", serverID );

   // userOrigin
   pos += sprintf( target + pos, ", '%s'", userOrigin );

   // messageID, debInfo, operationType and sentSize
   pos += sprintf( target + pos, ", %d, %d, %d, %d, ", 
                   messageID, debInfo, operationType, sentSize );
   // OperationDescription
   target[ pos ] = '\'';
   pos++;   
   pos += UserElement::sqlString( target + pos, operationDescription );
   target[ pos ] = '\'';
   pos++;
   
   target[ pos ] = '\0';
   return pos;
}

CellularPhoneModel*
UserProcessor::fillCellularPhoneModel(CharEncSQLQuery* sqlQuery)
{
   CellularPhoneModel* model = new CellularPhoneModel();
   model->setName             ( sqlQuery->getColumn(0)        );
   model->setManufacturer     ( sqlQuery->getColumn(1)        );
   model->setChars            ( atoi(sqlQuery->getColumn(2))  );
   model->setEOL              ( (UserConstants::EOLType)
                                atoi(sqlQuery->getColumn(3))  );
   model->setLines            ( atoi(sqlQuery->getColumn(4))  );
   model->setDynamicWidth     ( 't'==sqlQuery->getColumn(5)[0]);
   model->setGraphicsWidth    ( atoi(sqlQuery->getColumn(6))  );
   model->setGraphicsHeight   ( atoi(sqlQuery->getColumn(7))  );
   model->setSMSCapable       ( (CellularPhoneModel::SMSCapableType)
                                atoi(sqlQuery->getColumn(8))  );
   model->setSMSConcatenation ( (CellularPhoneModel::SMSConcatenationType)
                                atoi(sqlQuery->getColumn(9))  );
   model->setSMSGraphic       ( (CellularPhoneModel::SMSGraphicsType)
                                atoi(sqlQuery->getColumn(10)) );
   model->setWAPCapable       ( (CellularPhoneModel::WAPCapableType)
                                atoi(sqlQuery->getColumn(11)) );
   model->setWAPVersion       ( sqlQuery->getColumn(12)       );
   model->setModelYear        ( atoi(sqlQuery->getColumn(13)) );
   model->setComment          ( sqlQuery->getColumn(14)       );
   return model;
}

CellularPhoneModels* 
UserProcessor::getCellularPhoneModels( uint32 nbrCellularModels,
                                       CellularPhoneModel* cellular)
{
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   if( (nbrCellularModels > 0) && (cellular != NULL) ) {
      char query[4096];
      
      uint32 nbrFields = 0;
      strcpy( query, "SELECT * FROM ISABCellularModels WHERE ");
      uint32 pos = strlen( query );
      
      for ( uint32 i = 0 ; i < UserConstants::CELLULAR_MODEL_NBRFIELDS ;
            i++ ) {
         if ( cellular->changed( UserConstants::CellularModelField( i ) ) ) {
            if ( nbrFields != 0 ) {
               strcpy( query + pos, " AND " );
               pos += 5;   
            }
            nbrFields++;
            strcpy( query + pos, UserConstants::CellularModelFieldName[ i ] );
            pos += strlen( UserConstants::CellularModelFieldName[ i ] );
            strcpy( query + pos, " = " );
            pos += 3;
            pos += cellular->printValue( query + pos, 
                                         UserConstants::CellularModelField(
                                            i ) );
         }
      }
      if ( nbrFields == 0 ) {
         mc2log << error << "UP::getCellularPhoneModels(): "
                << "No data to find cellular with!" << endl;
         delete sqlQuery;
         return NULL;
      }
      
      
      if ( ! doQuery(sqlQuery, query, "UP::getCellularPhoneModels() 1") ) {
         delete sqlQuery;
         return NULL;
      }

   } else {
      if ( ! doQuery(sqlQuery, "SELECT * FROM ISABCellularModels", 
                     "UP::getCellularPhoneModels() 2")) {
         delete sqlQuery;
         return NULL;
      }
   }
      
   CellularPhoneModels* models = new CellularPhoneModels();
   CellularPhoneModel* model = NULL;
   while (sqlQuery->nextRow()) {
     // use help function to get the model
     model = fillCellularPhoneModel(sqlQuery);
     // and add to the list
     models->addPhoneModel( model );
   }

   delete sqlQuery;

   if (NULL == model)  { // no models found
      delete models;
      return NULL;
   }
   else
      return models;  
}

CellularPhoneModel* 
UserProcessor::getCellularPhoneModel( const char* modelName )
{
   char query[4096];
   CellularPhoneModel* model = NULL;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
  
   sprintf(query, "SELECT Name, Manufacturer, CharsPerLine, EOLType, "
           "DisplayableLines, DynamicWidth, GraphicDisplayWidth, "
           "GraphicDisplayHeight, SMSCapable, SMSContatenated, SMSGraphic, "
           "WAPCapable, WAPVersion, ModelYear, CommentString "
           "FROM ISABCellularModels "
           "WHERE Name = '%s'",
            modelName);

   if ( ! doQuery(sqlQuery, query, "UP::getCellularPhoneModel()") ) {
      delete sqlQuery;
      return NULL;
   }

   if ( sqlQuery->nextRow() ) {
      model = new CellularPhoneModel();
      model->setName            ( sqlQuery->getColumn(0)         );
      model->setManufacturer    ( sqlQuery->getColumn(1)         );
      model->setChars           ( atoi(sqlQuery->getColumn(2))   );
      model->setEOL             ( (UserConstants::EOLType)
                                  atoi(sqlQuery->getColumn(3))   );
      model->setLines           ( atoi(sqlQuery->getColumn(4))   );
      model->setDynamicWidth    ( 't'==sqlQuery->getColumn(5)[0] );
      model->setGraphicsWidth   ( atoi(sqlQuery->getColumn(6))   );
      model->setGraphicsHeight  ( atoi(sqlQuery->getColumn(7))   );
      model->setSMSCapable      ( (CellularPhoneModel::SMSCapableType)
                                  atoi(sqlQuery->getColumn(8))   );
      model->setSMSConcatenation( (CellularPhoneModel::SMSConcatenationType)
                                  atoi(sqlQuery->getColumn(9))   );
      model->setSMSGraphic      ( (CellularPhoneModel::SMSGraphicsType)
                                  atoi(sqlQuery->getColumn(10))  );
      model->setWAPCapable      ( (CellularPhoneModel::WAPCapableType)
                                  atoi(sqlQuery->getColumn(11))  );
      model->setWAPVersion      ( sqlQuery->getColumn(12)        );
      model->setModelYear       ( atoi(sqlQuery->getColumn(13))  );
      model->setComment         ( sqlQuery->getColumn(14)        );
   } else {
      // unknown phone model
      model = new CellularPhoneModel( 
         "UNKNOWN", "", 10, UserConstants::EOLTYPE_AlwaysCRLF, 2, true,
         0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
         CellularPhoneModel::SMSCONCATENATION_NO, 
         CellularPhoneModel::SMSGRAPHICS_NO, 
         CellularPhoneModel::WAPCAPABLE_NO,
         "", 00, "Default unknown phone" );
   }

   delete sqlQuery;
   return model;
}


uint32 
UserProcessor::addCellularModelData( char* target, 
                                     CellularPhoneModel* model )
{
   int pos = 0;

   for ( uint32  i = 0 ; 
         i < UserConstants::CELLULAR_MODEL_NBRFIELDS ; 
         i++ ) 
   {
      if ( i != 0 ) {
         strcpy( target + pos, ", " );
         pos += 2;
      }
      pos += model->printValue( target + pos , 
                                UserConstants::CellularModelField( i ) );
   }
   
   return pos;
}


int
UserProcessor::makeSQLDate( uint32 time, char* dateStr, char* timeStr ) {
   struct tm *tm_struct;
   int size = 0;
   
   time_t rtime = time;
   struct tm result;
   tm_struct = gmtime_r( &rtime, &result );
   int timeSec = tm_struct->tm_hour * 3600 + tm_struct->tm_min * 60 
                 + tm_struct->tm_sec;

   sprintf( dateStr, "'%.4d-%.2d-%.2d'" /*"%Y-%m-%d"*/, 
            tm_struct->tm_year + 1900, tm_struct->tm_mon + 1,
            tm_struct->tm_mday );
   size += sprintf( timeStr, "%u", timeSec );
   return size;
}


bool 
UserProcessor::setSessionValue( const char* session,
                                const char* name, 
                                const char* value ) 
{
   char query[4096];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   bool status = true;

   sprintf(query, "UPDATE ISABSession SET %s = %s WHERE sessionID = '%s'",
           name, value, session );

   if ( ! doQuery(sqlQuery, query, "UP::setSessionValue()") ) {
      status = false;
   }

   delete sqlQuery;
   return status;
}


bool 
UserProcessor::makeSession( uint32 UIN, uint32 now, char* id, char* key )
{
   char query[1024];
   bool ok = false;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   
   while ( !ok ) {
      StringUtility::randStr( id, 30 );
      sprintf(query, "SELECT * FROM ISABSession WHERE sessionID = '%s'", id );   
      if ( ! doQuery(sqlQuery, query, "UP::makeSession() check 2") ) {
         delete sqlQuery;
         return false;
      }
      ok = ( ! sqlQuery->nextRow());
   }

   StringUtility::randStr( key, 30 );
   
   sprintf(query, "INSERT INTO ISABSession ( sessionID, sessionKey, "
           "sessionUIN, lastAccessTime, loginTime, logoutTime) "
           "VALUES ( '%s', '%s', %d, %d, %d, %d )",
           id, key, UIN, now, now, 0 );

   if ( ! doQuery(sqlQuery, query, "UP::makeSession() insert") ) {
      delete sqlQuery;
      return false;
   }
   
   mc2dbg8 << "UP::makeSession id: #" << id << "# " << endl 
           << "key #" << key << "#" << endl;

   // Add to cache
#ifdef PARALLEL_USERMODULE
   m_sessionCache->add( id, SessionCacheItem( key, UIN, now ) );
#else
   m_userSessionCache->addSession( id, key, UIN, now );
#endif
          
   delete sqlQuery;
   return true;
}


bool
UserProcessor::verifySession( const char* sessionID, 
                              const char* sessionKey,
                              uint32& UIN,
                              bool updateAccess,
                              bool checkExpired )
{
   bool updateLastAccess = false;
   int32 now = TimeUtility::getRealTime();
   // First try cache
#ifdef PARALLEL_USERMODULE
   bool ok = false;
   SessionCacheItem cached;
   if ( m_sessionCache->get( sessionID, cached ) )
   {
      // Check cachedKey 
      if ( StringUtility::strcmp( cached.key.c_str(), sessionKey ) == 0 ) {
         // Check cachedTime
         if ( now - int32(cached.lastAccessTime) < sessionValidTime ) {
            // Ok!
            ok = true;
            UIN = cached.UIN;
            if ( now - int32(cached.lastAccessTime) < sessionUpdateTime ) {
               // Update last access time in SQL
               updateLastAccess = true;
            }
            mc2log << "UP::verifySession Got " << sessionID 
                   << " = " << UIN << " from cache" << endl;
         } else {
            m_sessionCache->remove( sessionID );
         }
      } else {
         m_sessionCache->remove( sessionID );
      }
   }
#else
   const char* cachedKey = NULL;
   uint32 cachedUIN = 0;
   uint32 cachedTime = 0;
   bool ok = false;
   if ( m_userSessionCache->getSession( sessionID, cachedKey, cachedUIN,
                                        cachedTime ) )
   {
      // Check cachedKey 
      if ( StringUtility::strcmp( cachedKey, sessionKey ) == 0 ) {
         // Check cachedTime
         if ( now - int32(cachedTime) < sessionValidTime ) {
            // Ok!
            ok = true;
            UIN = cachedUIN;
            if ( now - int32(cachedTime) < sessionUpdateTime ) {
               // Update last access time in SQL
               updateLastAccess = true;
            }
            mc2log << "UP::verifySession Got " << sessionID 
                   << " = " << UIN << " from cache" << endl;
         } else {
            m_userSessionCache->removeSession( sessionID );
         }
      } else {
         m_userSessionCache->removeSession( sessionID );
      }
   }
#endif

   if ( !ok ) { 
      // Check database
      char query[4096];
      CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

      mc2dbg4 << "UP::verifySession(): id: " << sessionID
              << ", key: " << sessionKey << endl;
   
      if ( checkExpired ) {
         sprintf( query,
                  "SELECT ISABSession.*,ISABUserUser.validDate FROM "
                  "ISABSession, ISABUserUser WHERE "
                  "ISABSession.sessionID = '%s' AND UIN = sessionUIN", 
                  sessionID );
      } else {
         sprintf( query,
                  "SELECT * FROM ISABSession WHERE sessionID = '%s'", 
                  sessionID );
      }

      if ( ! doQuery(sqlQuery, query, "UP::verifySession()") ) {
         mc2log << warn << "UP::verifySession() query failed " << query
                << endl;
         delete sqlQuery;
         return false;
      }

//sessionID | sessionKey | sessionUIN | lastAccessTime | loginTime  | logoutTime

      if (sqlQuery->nextRow()) {
         if ( strcmp( sqlQuery->getColumn(1), sessionKey) == 0) {
            int32 lastAccessTime = atoi( sqlQuery->getColumn(3));
            if ( now - lastAccessTime < sessionValidTime ) {
               if ( now - lastAccessTime > sessionUpdateTime ) {
                  // Update last access time in SQL
                  updateLastAccess = true; 
               }
               UIN = strtoul( sqlQuery->getColumn(2), NULL, 10);
               if ( checkExpired ) {
                  // validDate in last column
                  int32 validDate = StringUtility::makeDate( 
                     sqlQuery->getColumn( sqlQuery->getNumColumns() -1 ) );
                  if ( validDate < now && validDate != 0 ) {
                     mc2log << info << "UP::verifySession() for UIN [" 
                            << UIN << "] User is expired, validDate " 
                            << sqlQuery->getColumn( 
                               sqlQuery->getNumColumns() -1 ) << endl;
                     UIN = MAX_UINT32 - 1;
                  }
               }
               if ( UIN != 0 && UIN != MAX_UINT32 - 1 ) {
                  // Add to cache
#ifdef PARALLEL_USERMODULE
                  m_sessionCache->add( sessionID, 
                                       SessionCacheItem( sessionKey, 
                                                         UIN, 
                                                         lastAccessTime ) );
#else
                  m_userSessionCache->addSession( sessionID, sessionKey, 
                                                  UIN, lastAccessTime );
#endif
               }
            } else {
               mc2log << info << "UP::verifySession(): "
                      << " Session too old sessionID: " << sessionID 
                      << " Tdiff: " << ((int32)now - lastAccessTime ) 
                      << " ( " << now << " - " << lastAccessTime <<" ) "
                      << endl;
               UIN = MAX_UINT32;
            }
         } else {
            mc2dbg4 << "UP::verifySession(): SessionKey doesn't match "
                    << " sessionID: " << sessionID << ", Key " 
                    << sessionKey
                    << " != " << sqlQuery->getColumn(1) << endl;
            UIN = 0;
         }
      } else {
         mc2dbg8 << "UP::verifySession() No matches for sessionID: " 
                 << sessionID << endl;
         UIN = 0;
      }
      delete sqlQuery;
   } // End if !ok
   
   if ( updateAccess && updateLastAccess ) {
      mc2dbg4 << "UP::verifySession(): Updating last access time "
              << "in SQL-database" << endl;
      char tmp[20];
      sprintf( tmp, "%d", now );
      setSessionValue( sessionID, "lastAccessTime", tmp);
      return true;
   }

   return true;
}


bool 
UserProcessor::getAndChangeTransactions( 
   uint32 UIN, int32 transactionChange, int32& nbrTransactions )
{
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   bool ok = true;

   sprintf( query, "SELECT nbrTransactions FROM ISABUserTransactions "
            "WHERE userUIN = %u", UIN );
   
   if ( !doQuery( sqlQuery, query, 
                  "UP::getAndChangeTransactions select nbr "
                  "transactions" ) )
   {
      mc2log << error << "UP::getAndChangeTransactions "
             << "select nbr transactions failed." << endl;
      ok = false;
   } else {
      if ( sqlQuery->nextRow() ) {
         // Ok get current nbr transactions and set new value
         int32 nbrTrans = atoi( sqlQuery->getColumn( 0 ) );
         nbrTransactions = nbrTrans + transactionChange;
         if ( transactionChange != 0 ) {
            sprintf( query, 
                     "UPDATE ISABUserTransactions SET nbrTransactions = %d "
                     "WHERE userUIN = %u", 
                     nbrTransactions, UIN );
         } // else no transactions to change
      } else {
         // No earlier entry create new
         if ( transactionChange != 0 ) {
            nbrTransactions = transactionChange;
            sprintf( query, 
                     "INSERT INTO ISABUserTransactions VALUES ( %d, %d )",
                     UIN, nbrTransactions );
         }
      }
      if ( transactionChange != 0 ) {
         if ( !doQuery( sqlQuery, query, 
                        "UP::getAndChangeTransactions set transactions" ) )
         {
            mc2log << error << "UP::handleTransactionRequestPacket(): "
                   << "update of transactions failed!" << endl;
            ok = false;
         } else {
            ok = true;
         }
      } else {
         ok = true;
      }
   }
   
   delete sqlQuery;
   return ok;
}


StringTable::stringCode
UserProcessor::getAndCheckTransactionDays( 
   uint32 UIN, bool check, int32& nbrTransactionDays, uint32& curTime )
{
   mc2dbg << "getAndCheckTransactionDays( " << UIN << ", " << check
          << ", " << nbrTransactionDays << ", " << curTime << " )" << endl;
   uint32 now = TimeUtility::getRealTime();
   char query[ 1024 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   StringTable::stringCode status = StringTable::OK;
   curTime = 0;
   int32 nbrDaysnow = 0;
   bool hasEntry = false;

   sprintf( query, "SELECT nbrTransactionDays, curTime "
            "FROM ISABUserTransactionDays "
            "WHERE userUIN = %u", UIN );
   
   if ( !doQuery( sqlQuery, query, 
                  "UP::handleTransactionDaysRequestPacket select nbr "
                  "days, curTime" ) )
   {
      mc2log << error << "UP::getAndCheckTransactionDays "
             << "select nbr transaction days failed." << endl;
      status = StringTable::NOTOK;
   } else {
      if ( sqlQuery->nextRow() ) {
         // Ok get current nbr transaction days and curTime
         nbrDaysnow = atoi( sqlQuery->getColumn( 0 ) );
         curTime = atoi( sqlQuery->getColumn( 1 ) );
         hasEntry = true;
      } else {
         // No entry
      }
   } // End else sqlQuery ok


   if ( status == StringTable::OK  ) {
      // Select ok above
      if ( hasEntry ) {
         if ( curTime + 24*60*60 < now || nbrTransactionDays != 0 ) {
            nbrTransactionDays += nbrDaysnow;
            bool newDay = check && curTime + 24*60*60 < now;
            if ( newDay ) {
               if ( nbrTransactionDays > 0 ) {
                  // Ok days left
               } else {
                  // No more days
                  status = StringTable::NOT_ALLOWED;
                  newDay = false;
               }
            }

            // Update
            sprintf( query, 
                     "UPDATE ISABUserTransactionDays SET "
                     "nbrTransactionDays = %d, curTime = %d "
                     "WHERE userUIN = %u", 
                     nbrTransactionDays - (newDay?1:0), 
                     newDay ? now : curTime, 
                     UIN );
            if ( !doQuery( sqlQuery, query, 
                           "UP::getAndCheckTransactionDays set "
                           "new nbrTransactionDays and curTime" ) )
            {
               // Error
               mc2log << error << "UP::getAndCheckTransactionDays "
                      << "set new nbrTransactionDays and curTime "
                      << "failed." << endl;
               status = StringTable::NOTOK;
               nbrTransactionDays = nbrDaysnow;
            } else {
               // Changed ok
               if ( newDay ) {
                  curTime = now;
                  nbrTransactionDays = nbrTransactionDays - 1;
               }
            }
         } else { // else still on last day and no change of days
            nbrTransactionDays = nbrDaysnow;
         }
      } else {
         // No entry at all
         if ( check || nbrTransactionDays != 0 ) {
            // Set days
            bool newDay = check;
            if ( newDay ) {
               if ( nbrTransactionDays > 0 ) {
                  // Ok days left
               } else {
                  // No more days
                  status = StringTable::NOT_ALLOWED;
                  newDay = false;
               }
            }
            // Insert
            sprintf( query, 
                     "INSERT INTO ISABUserTransactionDays ( "
                     "userUIN, nbrTransactionDays, curTime ) VALUES ( "
                     "%u, %d, %d ) ", 
                     UIN,
                     nbrTransactionDays - (newDay?1:0), 
                     newDay ? now : 0 );
            if ( !doQuery( sqlQuery, query, 
                           "UP::getAndCheckTransactionDays insert "
                           "new nbrTransactionDays and curTime" ) )
            {
               // Error
               mc2log << error << "UP::getAndCheckTransactionDays "
                      << "insert new nbrTransactionDays and curTime "
                      << "failed." << endl;
               status = StringTable::NOTOK;
               nbrTransactionDays = 0;
            } else {
               // Changed ok
               if ( newDay ) {
                  curTime = now;
                  nbrTransactionDays = nbrTransactionDays - 1;
               }
            }
         } // End if nbrTransactionDays != 0
      } // End else (no entry in db)

   } // End if status == StringTable::OK

   delete sqlQuery;

   return status;
}

bool
UserProcessor::setUserForIDinUserLicence( uint32 uin, uint32 ownerUIN,
                                          uint32 changerUIN,
                                          const UserLicenceKey* licence,
                                          CharEncSQLQuery* sqlQuery ) {
   bool ok = true;
   char query[ 4096 ];
   sprintf( query, "UPDATE ISABUserLicenceKey SET userUIN = %u WHERE id = %u",
            uin, licence->getID() );
   
   if ( ! doQuery( sqlQuery, query, "UP::setUserForIDinUserLicence()" ) ) {
      mc2log << error << "UP::setUserForIDinUserLicence(): "
             << "Update UIN failed! UIN " << uin << " id " << licence->getID()
             << endl;
      ok = false;
   } else {
      // Add to change log (uin to reflect new owner)
      addUserLicenceTolog( uin, licence, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::deleteUserLicence( uint32 uin, uint32 changerUIN,
                                  const UserLicenceKey* licence,
                                  CharEncSQLQuery* sqlQuery ) {
   bool ok = true;
   char query[ 4096 ];
   sprintf( query, "DELETE FROM ISABUserLicenceKey WHERE id = %u", 
            licence->getID() );
   
   if ( ! doQuery( sqlQuery, query, "UP::deleteUserLicence()" ) ) {
      mc2log << error << "UP::deleteUserLicence(): "
             << "Delete licence failed! id " << licence->getID() << endl;
      ok = false;
   } else {
      // Add to change log
      UserLicenceKey lic( *licence );
      lic.remove();
      addUserLicenceTolog( uin, &lic, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::deleteOldUserLicence( uint32 uin, uint32 changerUIN,
                                     const UserLicenceKey* licence,
                                     CharEncSQLQuery* sqlQuery ) {
   bool ok = true;
   char query[ 4096 ];
   sprintf( query, "DELETE FROM ISABUserLicence WHERE id = %u", 
            licence->getID() );
   
   if ( ! doQuery( sqlQuery, query, "UP::deleteOldUserLicence()" ) ) {
      mc2log << error << "UP::deleteOldUserLicence(): "
             << "Delete licence failed! id " << licence->getID() << endl;
      ok = false;
   } else {
      // Add to change log
      UserLicenceKey lic( *licence );
      lic.remove();
      addOldUserLicenceTolog( uin, &lic, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::addUserLicence( uint32 uin, uint32 changerUIN,
                               UserLicenceKey* licence, 
                               CharEncSQLQuery* sqlQuery ) {
   bool ok = true;
   char query[ 4096 ];

   uint32 ID = getNewLicenceID();
   licence->setID( ID );
      
   if ( ID == 0 ) {
      mc2log << error << "UP::addUserLicence(): "
             << " failed finding new licenceID" << endl;
      ok = false;
   }

   if ( ok ) {
      addUserLicenceData( query, ID, uin, licence );
      
      if ( !doQuery( sqlQuery, query, "UP::addUserLicence() add licence" ) ) {
         mc2log << error << "UP::addUserLicence(): "
                << "Add licence failed! UIN " << uin << " id " << ID 
                << " licenceKey " << licence->getLicenceKeyStr() << endl;
         ok = false;
      } else {
         // Add to change log
         addUserLicenceTolog( uin, licence, changerUIN, sqlQuery );
      }
   }
   
   return ok;
}

bool
UserProcessor::changeUserLicence( uint32 uin, uint32 changerUIN,
                                  const UserLicenceKey* licence, 
                                  CharEncSQLQuery* sqlQuery ) {
   bool ok = true;

   char query[ 4096 ];
   char changed[ 4096 ];
   addChangedUserLicenceData( changed, licence );
   sprintf( query, "UPDATE ISABUserLicenceKey SET %s WHERE id = %u",
            changed, licence->getID() );
   if ( !doQuery( sqlQuery, query, 
                  "UP::changeUserLicence() licence" ) ) {
      mc2log << error << "UP::changeUserLicence(): "
             << "Change licence failed! UIN " << uin 
             << " licenceKey " << licence->getLicenceKeyStr() << endl;
   } else {
      // Add to change log
      addUserLicenceTolog( uin, licence, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::deleteUserIDKey( uint32 uin, uint32 changerUIN,
                                const UserIDKey* key,
                                CharEncSQLQuery* sqlQuery ) {
   bool ok = true;
   char query[ 4096 ];
   sprintf( query, "DELETE FROM ISABUserIDKey WHERE id = %u", 
            key->getID() );
   
   if ( ! doQuery( sqlQuery, query, "UP::deleteUserIDKey()" ) ) {
      mc2log << error << "UP::deleteUserIDKey(): "
             << "Delete idKey failed! id " << key->getID() << endl;
      ok = false;
   } else {
      // Add to change log
      UserIDKey rmKey( *key );
      rmKey.remove();
      addUserIDKeyTolog( uin, &rmKey, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::setUserForIDinUserIDKey( uint32 uin, uint32 ownerUIN,
                                        uint32 changerUIN,
                                        const UserIDKey* key,
                                        CharEncSQLQuery* sqlQuery )
{
   bool ok = true;
   char query[ 4096 ];
   sprintf( query, "UPDATE ISABUserIDKey SET userUIN = %u WHERE id = %u",
            uin, key->getID() );
   
   if ( ! doQuery( sqlQuery, query, "UP::setUserForIDinUserIDKey()" ) ) {
      mc2log << error << "UP::setUserForIDinUserIDKey(): "
             << "Update UIN failed! UIN " << uin << " id " << key->getID()
             << endl;
      ok = false;
   } else {
      // Add to change log (uin to reflect new owner)
      addUserIDKeyTolog( uin, key, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::addUserIDKey( uint32 uin, uint32 changerUIN,
                             UserIDKey* idKey, 
                             CharEncSQLQuery* sqlQuery )
{
   bool ok = true;
   char query[ 4096 ];

   uint32 ID = getNewUniqueID( "ISABUserIDKey", "id" );
   idKey->setID( ID );
      
   if ( ID == 0 ) {
      mc2log << error << "UP::addUserLicence(): "
             << " failed finding new idKey ID" << endl;
      ok = false;
   }

   if ( ok ) {
      int pos = sprintf( query, "INSERT INTO ISABUserIDKey ( "
                     "id, userUIN, type, idkey ) VALUES ( " );

      pos += addUserIDKeyData( query + pos, ID, uin, idKey );
      pos = sprintf( query + pos, " )" );
      
      if ( !doQuery( sqlQuery, query, "UP::addUserLicence() add idKey" ) ) {
         mc2log << error << "UP::addUserIDKey(): "
                << "Add idKey failed! UIN " << uin << " id " << ID 
                << " idKey " << idKey->getIDKey() << " type " 
                << int(idKey->getIDType()) << endl;
         ok = false;
      } else {
         // Add to change log
         addUserIDKeyTolog( uin, idKey, changerUIN, sqlQuery );
      }
   }
   
   return ok;
}

void
UserProcessor::removeUserFromCache( uint32 uin ) {
   if ( m_useUserCache ) {
      UserItem* userEl = static_cast<UserItem*>(
         m_cache->find( uin, CacheElement::USER_ELEMENT_TYPE ) );
      if ( userEl != NULL ) {
         m_cache->remove( userEl );
         m_cache->releaseCacheElement( userEl, 
                                       false/*checkCapacity not needed*/ );
      }
   }
}

void
UserProcessor::addTable(int index, const char* name, const char* createQuery,
                        const char* extraQuery)
{
   m_tableNames[index] = StringUtility::newStrDup(name);
   m_tableCreateQueries[index] = StringUtility::newStrDup(createQuery);
   if (extraQuery != NULL)
      m_tableExtraQueries[index] = StringUtility::newStrDup(extraQuery);
   else
      m_tableExtraQueries[index] = NULL;
}

int 
UserProcessor::initTables()
{
   // Dont forget to imcrement this if you add a table
   int numTables   = 38;
   uint32 tableNbr = 0;

   m_tableNames = new char*[numTables];
   m_tableCreateQueries = new char*[numTables];
   m_tableExtraQueries = new char*[numTables];

   const char* ISABUserUserCreateTable = 
      "CREATE TABLE ISABUserUser ( UIN BIGINT NOT NULL, "
      "logonID VARCHAR(255) NOT NULL, logonPasswd VARCHAR(255) NOT NULL,"
      "firstname VARCHAR(255) NOT NULL, initials VARCHAR(255) NOT NULL,"
      "lastname VARCHAR(255) NOT NULL, sessionID VARCHAR(255),"
      "measurementSystem SMALLINT NOT NULL,"
      "language VARCHAR(255) NOT NULL, lastdestmapID INT NOT NULL,"
      "lastdestitemID INT NOT NULL, lastdestOffset SMALLINT NOT NULL,"
      "lastdestTime INT NOT NULL, lastdestString VARCHAR(255),"
      "lastorigmapID INT NOT NULL, lastorigitemID INT NOT NULL,"
      "lastorigOffset SMALLINT NOT NULL, lastorigTime INT NOT NULL,"
      "lastorigString VARCHAR(255),"
      "searchType SMALLINT NOT NULL, searchSubstring SMALLINT NOT NULL,"
      "searchSorting SMALLINT NOT NULL, searchObject SMALLINT NOT NULL,"
      "routeCostA SMALLINT NOT NULL, routeCostB SMALLINT NOT NULL,"
      "routeCostC SMALLINT NOT NULL, routeCostD SMALLINT NOT NULL,"
      "routeType SMALLINT NOT NULL, editMapRights INT NOT NULL,"
      "editDelayRights INT NOT NULL, editUserRights INT NOT NULL,"
      "wapService CHAR(1) NOT NULL, htmlService CHAR(1) NOT NULL,"
      "operatorService CHAR(1) NOT NULL,"
      "nbrMunicipals INT NOT NULL, municipals VARCHAR(255),"
      "vehicleType INT NOT NULL,"
      "birthDate VARCHAR(255),"
      "routeImageType SMALLINT NOT NULL,"
      "validDate DATE NOT NULL,"
      "gender SMALLINT NOT NULL,"
      "smsService CHAR(1) NOT NULL, "
      "defaultCountry VARCHAR(255), "
      "defaultMunicipal VARCHAR(255), "
      "defaultCity VARCHAR(255), "
      "searchDbMask SMALLINT NOT NULL, "
      "navService CHAR(1) NOT NULL,"
      "operatorComment VARCHAR(255), "
      "emailAddress VARCHAR(255), "
      "address1 VARCHAR(255), "
      "address2 VARCHAR(255), "
      "address3 VARCHAR(255), "
      "address4 VARCHAR(255), "
      "address5 VARCHAR(255), "
      "routeTurnImageType SMALLINT NOT NULL, "
      "externalXMLService CHAR(1) NOT NULL, "
      "transactionBased SMALLINT DEFAULT 0,"
      "deviceChanges INT DEFAULT -1, "
      "supportComment MC2BLOB NOT NULL,"
      "postalCity VARCHAR(100) NOT NULL,"
      "zipCode VARCHAR(40) NOT NULL,"
      "companyName VARCHAR(255) NOT NULL,"
      "companyReference VARCHAR(255) NOT NULL,"
      "companyVATNbr VARCHAR(50) NOT NULL,"
      "emailBounces INT DEFAULT 0,"
      "addressBounces INT DEFAULT 0,"
      "customerContactInfo VARCHAR(255) NOT NULL,"
      "creationTime DATE,"
      "PRIMARY KEY (UIN),"
      "INDEX UserUserLogonIDKey ( logonID ), "
      "INDEX UserUserfirstname ( firstName ), "
      "INDEX UserUserlastname ( lastname ), "
      "INDEX UserUseremailAddress ( emailAddress ), "
      "CONSTRAINT checkPassword CHECK (logonPasswd NOT LIKE ''),"
      "CONSTRAINT BoolWAPService CHECK (wapService IN ( 't', 'f') ),"
      "CONSTRAINT BoolHTMLService CHECK (htmlService IN ( 't', 'f') ),"
      "CONSTRAINT BoolOperatorService CHECK (operatorService "
      " IN ( 't', 'f') ),"
      "CONSTRAINT BoolNavService CHECK (navService IN ( 't', 'f') ),"
      "CONSTRAINT BoolexternalXMLService CHECK (externalXMLService"
      " IN ( 't', 'f') ),"
      "CONSTRAINT UserUserLogonIDConstr UNIQUE (logonID) )";
   addTable(tableNbr++, "ISABUserUser", ISABUserUserCreateTable );
  
   addTable(tableNbr++, "ISABUserCellular",
      "CREATE TABLE ISABUserCellular ( id INT NOT NULL,"
      "userUIN BIGINT NOT NULL, phoneNumber VARCHAR(255) NOT NULL,"
      "model VARCHAR(30) NOT NULL, SMSParams SMALLINT NOT NULL,"
      "maxSearchHitsWap SMALLINT NOT NULL,"
      "maxRouteLinesWap SMALLINT NOT NULL,"
      "smsService CHAR(1) NOT NULL,"
      "EOLType INT NOT NULL,"
      "CharsPerLine SMALLINT NOT NULL,"
      "posActive CHAR(1) NOT NULL,"
      "typeOfPos INT NOT NULL,"
      "posUserName VARCHAR(255),"
      "posPassword VARCHAR(255),"
      "lastposLat INT NOT NULL,"
      "lastposLong INT NOT NULL,"
      "lastposInnerRadius INT NOT NULL,"
      "lastposOuterRadius INT NOT NULL,"
      "lastposStartAngle INT NOT NULL,"
      "lastposStopAngle INT NOT NULL,"
      "lastposTime INT NOT NULL,"
      "PRIMARY KEY (phoneNumber),"
      "INDEX UserCellularUserKey (userUIN), "
      "INDEX UserCellularID (id), "
      "CONSTRAINT BoolSMS CHECK (smsService IN ('t', 'f')),"
      "CONSTRAINT refUserCell FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )");
      
   addTable(tableNbr++, "ISABDebit", 
      "CREATE TABLE ISABDebit ( debitDate DATE NOT NULL,"
      "debitTime INT NOT NULL, debitUIN BIGINT NOT NULL,"
      "serverID VARCHAR(30) NOT NULL,"
      "userOrigin VARCHAR(30) NOT NULL, messageID INT NOT NULL,"
      "debitInfo INT NOT NULL,"
      "operationType INT NOT NULL, sentSize INT NOT NULL,"
      "operationDescription VARCHAR(255) NOT NULL,"
      "INDEX DebitUserKey (debitUIN) )");
         
   addTable(tableNbr++, "ISABCellularModels",
      "CREATE TABLE ISABCellularModels ( Name VARCHAR(30) NOT NULL,"
      "Manufacturer VARCHAR(30) NOT NULL,"
      "CharsPerLine SMALLINT, EOLType INT, "
      "DisplayableLines SMALLINT, DynamicWidth CHAR(1), "
      "GraphicDisplayWidth SMALLINT, GraphicDisplayHeight SMALLINT,"
      "SMSCapable INT, SMSContatenated INT, SMSGraphic INT,"
      "WAPCapable INT, WAPVersion CHAR(5), ModelYear SMALLINT,"
      "CommentString VARCHAR(255), "
      "PRIMARY KEY (Name), "
      "CONSTRAINT BoolDynamicWidth CHECK (DynamicWidth IN ( 't', 'f') ) )");

   addTable(tableNbr++, "ISABSession",
      "CREATE TABLE ISABSession ( sessionID VARCHAR(30) NOT NULL,"
      "sessionKey VARCHAR(30) NOT NULL, sessionUIN BIGINT NOT NULL,"
      "lastAccessTime INT NOT NULL, loginTime INT NOT NULL,"
      "logoutTime INT NOT NULL,"
      "PRIMARY KEY (sessionID) )");

   addTable(tableNbr++, "ISABSessionHistory",
      "CREATE TABLE ISABSessionHistory ( sessionID VARCHAR(30) NOT NULL,"
      "sessionKey VARCHAR(30) NOT NULL, sessionUIN BIGINT NOT NULL,"
      "lastAccessTime INT NOT NULL, loginTime INT NOT NULL,"
      "logoutTime INT NOT NULL )");

   addTable(tableNbr++, "ISABUserBuddyList", 
      "CREATE TABLE ISABUserBuddyList ( "
      "id INT NOT NULL, "
      "userUIN BIGINT NOT NULL, "
      "name VARCHAR(255) NOT NULL,"
      "b1 INT NOT NULL, "
      "b2 INT NOT NULL, "
      "b3 INT NOT NULL, "
      "b4 INT NOT NULL, "
      "b5 INT NOT NULL, "
      "b6 INT NOT NULL, "
      "b7 INT NOT NULL, "
      "b8 INT NOT NULL, "
      "b9 INT NOT NULL, "
      "b10 INT NOT NULL, "
      "PRIMARY KEY (id, userUIN),"
      "INDEX UserBuddyUserKey (userUIN), "
      "CONSTRAINT buddyrefUser FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )");

   addTable(tableNbr++, "ISABRouteStorage",
      "CREATE TABLE ISABRouteStorage ( "
      "routeID INT NOT NULL, "
      "createTime INT NOT NULL, "
      "userUIN BIGINT NOT NULL, "
      "validUntil INT NOT NULL, "
      "extraUserinfo VARCHAR(255),"
      "routePacketLength INT NOT NULL, "
      "originLat INT NOT NULL, "
      "originLon INT NOT NULL, "
      "originMapID INT NOT NULL, "
      "originItemID INT NOT NULL, "
      "originOffset INT NOT NULL, "
      "destinationLat INT NOT NULL, "
      "destinationLon INT NOT NULL, "
      "destinationMapID INT NOT NULL, "
      "destinationItemID INT NOT NULL, "
      "destinationOffset INT NOT NULL, "
      "routeContents MC2BLOB, "
      "PRIMARY KEY (routeID, createTime ),"
      "INDEX RouteStorageValidUntil (validUntil),"
      "INDEX RouteStorageUserKey (userUIN), "
      "CONSTRAINT routestoragerefUser FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )");

   addTable(tableNbr++, "ISABUserFavorites",
      "CREATE TABLE ISABUserFavorites ( "
      "userUIN BIGINT NOT NULL, "
      "favID INT NOT NULL, "
      "lat INT NOT NULL, "
      "lon INT NOT NULL, "
      "name VARCHAR(255),"
      "shortName VARCHAR(30),"
      "description VARCHAR(255),"
      "category VARCHAR(255),"
      "mapIconName VARCHAR(255),"
      "infos MEDIUMBLOB,"
      "PRIMARY KEY (userUIN, favID ) )");

   addTable(tableNbr++, "ISABUserTrackPoint",
      "CREATE TABLE ISABUserTrackPoint ( "
      "userUIN BIGINT NOT NULL, "
      "lat INT NOT NULL, "
      "lon INT NOT NULL, "
      "dist INT NOT NULL, "
      "speed SMALLINT NOT NULL, "
      "heading SMALLINT NOT NULL, "
      "trackTime INT NOT NULL, "
      "source VARCHAR(16) NOT NULL,"
      "PRIMARY KEY (userUIN, trackTime), "
      "INDEX UserTrackPointSource( source ) "
      ")");

   const char* ISABUserLicenceCreateTable = 
      "CREATE TABLE ISABUserLicence ( id INT NOT NULL, "
      "userUIN BIGINT NOT NULL, "
      "length INT NOT NULL, "
      "licenceKey VARCHAR(255) NOT NULL, "
      "PRIMARY KEY (id), "
      "INDEX UserLicenceUserKey ( userUIN ), "
      "INDEX UserLicenceKey ( licenceKey ), "
      "UNIQUE INDEX UserLicenceKeyAndUIN ( userUIN, licenceKey ), "
      "CONSTRAINT refUserLicence FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserLicence", ISABUserLicenceCreateTable );

   // The changelogtable for ISABUserLicence
   const char* licencechangelogtablename = "ISABUserLicencechangelog";
   MC2String licencelogtable = ISABUserLicenceCreateTable;
   licencelogtable.erase( 0, licencelogtable.find( '(' ) + 1 );
   licencelogtable.insert( 0, " (" );
   licencelogtable.insert( 0, licencechangelogtablename );
   licencelogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   uint32 cPos = licencelogtable.find( "PRIMARY KEY" );
   cPos = licencelogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   licencelogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   licencelogtable.append( ", changeTime INT NOT NULL" );
   // Add changerUIN 
   licencelogtable.append( ", changerUIN BIGINT NOT NULL" );
   // Key
   licencelogtable.append( ", PRIMARY KEY (userUIN, changeTime, id)" );
   licencelogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, licencechangelogtablename, 
             licencelogtable.c_str() );

   const char* ISABUserLicenceKeyCreateTable = 
      "CREATE TABLE ISABUserLicenceKey ( id INT NOT NULL, "
      "userUIN BIGINT NOT NULL, "
      "licenceKey VARCHAR(255) NOT NULL, "
      "product VARCHAR(255) NOT NULL, "
      "keyType VARCHAR(255) NOT NULL, "
      "PRIMARY KEY (id), "
      "INDEX UserLicenceKeyUserKey ( userUIN ), "
      "INDEX UserLicenceKeyKey ( licenceKey ), "
      "INDEX UserLicenceKeyKeyTypeAndProduct ( licenceKey, keyType, product ),"
      "UNIQUE INDEX UserLicenceKeyAndUINKey"
      " ( userUIN, licenceKey, keyType, product ), "
      "CONSTRAINT refUserLicenceKey FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserLicenceKey", ISABUserLicenceKeyCreateTable );

   // The changelogtable for ISABUserLicenceKey
   const char* licencekeychangelogtablename = "ISABUserLicenceKeychangelog";
   MC2String licencekeylogtable = ISABUserLicenceKeyCreateTable;
   licencekeylogtable.erase( 0, licencekeylogtable.find( '(' ) + 1 );
   licencekeylogtable.insert( 0, " (" );
   licencekeylogtable.insert( 0, licencekeychangelogtablename );
   licencekeylogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = licencekeylogtable.find( "PRIMARY KEY" );
   cPos = licencekeylogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   licencekeylogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   licencekeylogtable.append( ", changeTime INT NOT NULL" );
   // Add changerUIN 
   licencekeylogtable.append( ", changerUIN BIGINT NOT NULL" );
   // Key
   licencekeylogtable.append( ", PRIMARY KEY (userUIN, changeTime, id)" );
   licencekeylogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, licencekeychangelogtablename, 
             licencekeylogtable.c_str() );


   const char* ISABUserRegionAccessCreateTable = 
             "CREATE TABLE ISABUserRegionAccess ( id INT NOT NULL,"
             "userUIN BIGINT NOT NULL, "
             "regionID INT NOT NULL, "
             "startTime INT NOT NULL, "
             "endTime INT NOT NULL, "
             "PRIMARY KEY (id),"
             "INDEX UserRegionAccessKey ( userUIN ), "
             "CONSTRAINT refUserRegionA FOREIGN KEY (userUIN) REFERENCES "
             "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserRegionAccess", 
             ISABUserRegionAccessCreateTable );

   const char* ISABUserRightCreateTable = 
      "CREATE TABLE ISABUserRight ( id INT NOT NULL,"
      "userUIN BIGINT NOT NULL, "
      "addTime INT NOT NULL, "
      "type BIGINT NOT NULL, "
      "regionID INT NOT NULL, "
      "startTime INT NOT NULL, "
      "endTime INT NOT NULL, "
      "deleted INT NOT NULL, "
      "origin MC2BLOB NOT NULL, "
      "PRIMARY KEY (id),"
      "INDEX UserRightKey ( userUIN ), "
      "CONSTRAINT refUserRight FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserRight", ISABUserRightCreateTable );

   // The changelogtable for ISABUserRight
   const char* rightchangelogtablename = "ISABUserRightchangelog";
   MC2String rightlogtable = ISABUserRightCreateTable;
   rightlogtable.erase( 0, rightlogtable.find( '(' ) + 1 );
   rightlogtable.insert( 0, " (" );
   rightlogtable.insert( 0, rightchangelogtablename );
   rightlogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = rightlogtable.find( "PRIMARY KEY" );
   cPos = rightlogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   rightlogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   rightlogtable.append( ", changeTime INT NOT NULL " );
   // Add changerUIN 
   rightlogtable.append( ", changerUIN BIGINT NOT NULL " );
   // Key
   rightlogtable.append( ", PRIMARY KEY (userUIN, changeTime, id) " );
   rightlogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, rightchangelogtablename, rightlogtable.c_str());


   addTable( tableNbr++, "ISABUserTransactions",
             "CREATE TABLE ISABUserTransactions ( userUIN BIGINT NOT NULL, "
             "nbrTransactions INT NOT NULL, "
             "PRIMARY KEY (userUIN),"
             "CONSTRAINT refUserTrans FOREIGN KEY (userUIN) REFERENCES "
             "ISABUserUser(UIN) ON DELETE CASCADE )" );

   addTable( tableNbr++, "ISABUserWayfinderSubscription",
             "CREATE TABLE ISABUserWayfinderSubscription ( "
             "id INT NOT NULL,"
             "userUIN BIGINT NOT NULL, "
             "type SMALLINT NOT NULL, "
             "PRIMARY KEY (id),"
             "INDEX UserWayfinderSubscriptionKey ( userUIN ), "
             "CONSTRAINT refUserSub FOREIGN KEY (userUIN) REFERENCES "
             "ISABUserUser(UIN) ON DELETE CASCADE )" );

   addTable( tableNbr++, "ISABUserTransactionDays",
             "CREATE TABLE ISABUserTransactionDays ( "
             "userUIN BIGINT NOT NULL, "
             "nbrTransactionDays INT NOT NULL, "
             "curTime INT NOT NULL, "
             "PRIMARY KEY (userUIN),"
             "CONSTRAINT refUserTransDays FOREIGN KEY (userUIN) REFERENCES "
             "ISABUserUser(UIN) ON DELETE CASCADE )" );


   // The changelogtable for user ISABUserUser and ISABUserWayfinderSubscr.
   const char* userchangelogtablename = "ISABUserchangelog";
   MC2String userlogtable = ISABUserUserCreateTable;
   userlogtable.erase( 0, userlogtable.find( '(' ) + 1 );
   userlogtable.insert( 0, " (" );
   userlogtable.insert( 0, userchangelogtablename );
   userlogtable.insert( 0, "CREATE TABLE " );
   // creationTime is the last column. And should always be last.
   cPos = userlogtable.find( "creationTime" );
   cPos = userlogtable.find( ',', cPos ); // The ',' after creationTime
   userlogtable.erase( cPos ); // All After and including ','
   // Add WFSType 
   userlogtable.append( ", type SMALLINT " );
   // Add changeTime 
   userlogtable.append( ", changeTime INT NOT NULL " );
   // Key
   userlogtable.append( ", PRIMARY KEY (UIN, changeTime) " );
   userlogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, userchangelogtablename, userlogtable.c_str() );

   // The changelogtable for ISABUserRegionAccess
   const char* regionchangelogtablename = "ISABUserRegionchangelog";
   MC2String regionlogtable = ISABUserRegionAccessCreateTable;
   regionlogtable.erase( 0, regionlogtable.find( '(' ) + 1 );
   regionlogtable.insert( 0, " (" );
   regionlogtable.insert( 0, regionchangelogtablename );
   regionlogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = regionlogtable.find( "PRIMARY KEY" );
   cPos = regionlogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   regionlogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   regionlogtable.append( ", changeTime INT NOT NULL " );
   // Key
   regionlogtable.append( ", PRIMARY KEY (userUIN, changeTime, id) " );
   regionlogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, regionchangelogtablename, regionlogtable.c_str());

   const char* ISABUserTokenCreateTable = 
      "CREATE TABLE ISABUserToken ( id INT NOT NULL,"
      "userUIN BIGINT NOT NULL, "
      "createTime INT NOT NULL, "
      "age SMALLINT NOT NULL, "
      "token MC2BLOB NOT NULL, "
      "group_name VARCHAR(255) NOT NULL, "
      "PRIMARY KEY (id),"
      "INDEX UserTokenKey ( userUIN ), "
      "CONSTRAINT refUserToken FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserToken", ISABUserTokenCreateTable );

   // The changelogtable for ISABUserToken
   const char* tokenchangelogtablename = "ISABUserTokenchangelog";
   MC2String tokenlogtable = ISABUserTokenCreateTable;
   tokenlogtable.erase( 0, tokenlogtable.find( '(' ) + 1 );
   tokenlogtable.insert( 0, " (" );
   tokenlogtable.insert( 0, tokenchangelogtablename );
   tokenlogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = tokenlogtable.find( "PRIMARY KEY" );
   cPos = tokenlogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   tokenlogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   tokenlogtable.append( ", changeTime INT NOT NULL " );
   // Add changerUIN 
   tokenlogtable.append( ", changerUIN BIGINT NOT NULL " );
   // Key
   tokenlogtable.append( ", PRIMARY KEY (userUIN, changeTime, id) " );
   tokenlogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, tokenchangelogtablename, tokenlogtable.c_str());
   
   const char* ISABUserPINCreateTable = 
      "CREATE TABLE ISABUserPIN ( id INT NOT NULL,"
      "userUIN BIGINT NOT NULL, "
      "PIN MC2BLOB NOT NULL, "
      "comment MC2BLOB NOT NULL, "
      "PRIMARY KEY (id),"
      "INDEX UserPINKey ( userUIN ), "
      "CONSTRAINT refUserPIN FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserPIN", ISABUserPINCreateTable );

   // The changelogtable for ISABUserPIN
   const char* PINchangelogtablename = "ISABUserPINchangelog";
   MC2String PINlogtable = ISABUserPINCreateTable;
   PINlogtable.erase( 0, PINlogtable.find( '(' ) + 1 );
   PINlogtable.insert( 0, " (" );
   PINlogtable.insert( 0, PINchangelogtablename );
   PINlogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = PINlogtable.find( "PRIMARY KEY" );
   cPos = PINlogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   PINlogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   PINlogtable.append( ", changeTime INT NOT NULL " );
   // Add changerUIN 
   PINlogtable.append( ", changerUIN BIGINT NOT NULL " );
   // Key
   PINlogtable.append( ", PRIMARY KEY (userUIN, changeTime, id) " );
   PINlogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, PINchangelogtablename, PINlogtable.c_str());
   

   const char* ISABUserIDKeyCreateTable = 
      "CREATE TABLE ISABUserIDKey ( id INT NOT NULL,"
      "userUIN BIGINT NOT NULL, "
      "type INT NOT NULL, "
      "idkey VARCHAR(255) NOT NULL, "
      "PRIMARY KEY (id),"
      "INDEX UserIDKeyUINKey ( userUIN ), "
      "INDEX UserIDKeyIDKeyKey ( idkey ), "
      "CONSTRAINT refUserIDKey FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserIDKey", ISABUserIDKeyCreateTable );

   // The changelogtable for ISABUserIDKey
   const char* idKeychangelogtablename = "ISABUserIDKeychangelog";
   MC2String idKeylogtable = ISABUserIDKeyCreateTable;
   idKeylogtable.erase( 0, idKeylogtable.find( '(' ) + 1 );
   idKeylogtable.insert( 0, " (" );
   idKeylogtable.insert( 0, idKeychangelogtablename );
   idKeylogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = idKeylogtable.find( "PRIMARY KEY" );
   cPos = idKeylogtable.rfind( ',', cPos ); // The ',' before PRIMARY KEY
   idKeylogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   idKeylogtable.append( ", changeTime INT NOT NULL " );
   // Add changerUIN 
   idKeylogtable.append( ", changerUIN BIGINT NOT NULL " );
   // Key
   idKeylogtable.append( ", PRIMARY KEY (userUIN, changeTime, id) " );
   idKeylogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, idKeychangelogtablename, idKeylogtable.c_str());

   const char* ISABUserLastClientCreateTable = 
      "CREATE TABLE ISABUserLastClient ( id INT NOT NULL,"
      "userUIN BIGINT NOT NULL, "
      "client_type VARCHAR(255) NOT NULL, "
      "client_type_options VARCHAR(255) NOT NULL, "
      "version VARCHAR(255) NOT NULL, "
      "extra MC2BLOB NOT NULL, "
      "origin MC2BLOB NOT NULL, "
      "PRIMARY KEY (id),"
      "INDEX UserLastClientUINKey ( userUIN ), "
      "INDEX UserLastClientClientTypeKey ( client_type ), "
      "CONSTRAINT refUserLastClient FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABUserLastClient", 
             ISABUserLastClientCreateTable );

   // The changelogtable for ISABUserLastClient
   const char* lastClientchangelogtablename = 
      "ISABUserLastClientchangelog";
   MC2String lastClientlogtable = ISABUserLastClientCreateTable;
   lastClientlogtable.erase( 0, lastClientlogtable.find( '(' ) + 1 );
   lastClientlogtable.insert( 0, " (" );
   lastClientlogtable.insert( 0, lastClientchangelogtablename );
   lastClientlogtable.insert( 0, "CREATE TABLE " );
   // PRIMARY KEY is after the last column
   cPos = lastClientlogtable.find( "PRIMARY KEY" );
   cPos = lastClientlogtable.rfind( ',', cPos );//The ',' before PRIMARYKEY
   lastClientlogtable.erase( cPos ); // All After and including ','
   // Add changeTime 
   lastClientlogtable.append( ", changeTime INT NOT NULL " );
   // Add changerUIN 
   lastClientlogtable.append( ", changerUIN BIGINT NOT NULL " );
   // Key
   lastClientlogtable.append( ", PRIMARY KEY (userUIN, changeTime, id) " );
   lastClientlogtable.append( " )" );
   // Add the table
   addTable( tableNbr++, lastClientchangelogtablename, 
             lastClientlogtable.c_str() );

   const char* ISABPoiReviewCreateTable = 
      "CREATE TABLE ISABPoiReview ( id INT NOT NULL, "
      "poiID INT NOT NULL, "
      "userUIN BIGINT NOT NULL, "
      "grade INT NOT NULL, "
      "lang INT NOT NULL, "
      "title MC2BLOB NOT NULL, "
      "text MC2BLOB NOT NULL, "
      "time INT NOT NULL, "
      "PRIMARY KEY (id),"
      "INDEX PoiReviewUINKey ( userUIN ), "
      "INDEX PoiReviewPoiKey ( poiID ), "
      "CONSTRAINT refPoiReview FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE )";
   addTable( tableNbr++, "ISABPoiReview", 
             ISABPoiReviewCreateTable );

   const char* ISABPoiGradeCreateTable = 
      "CREATE TABLE ISABPoiGrade ( id INT NOT NULL,"
      "totalGrade INT NOT NULL, "
      "totalVote BIGINT NOT NULL, "
      "PRIMARY KEY (id) )";
   addTable( tableNbr++, "ISABPoiGrade", 
             ISABPoiGradeCreateTable );

   const char* ISABPoiGradeUserCreateTable = 
      "CREATE TABLE ISABPoiGradeUser ( id INT NOT NULL, "
      "userUIN BIGINT NOT NULL, "
      "PRIMARY KEY (id, userUIN),"
      "INDEX PoiGradeUserIDKey ( id ), "
      "INDEX PoiGradeUserUINKey ( userUIN ), "
      "CONSTRAINT refPoiGradeUser FOREIGN KEY (userUIN) REFERENCES "
      "ISABUserUser(UIN) ON DELETE CASCADE "
      ")";
   addTable( tableNbr++, "ISABPoiGradeUser", 
             ISABPoiGradeUserCreateTable );

   addTable( tableNbr++, "ISABRouteStorageCoords",
             "CREATE TABLE ISABRouteStorageCoords ( "
             "routeID INT NOT NULL, "
             "createTime INT NOT NULL, "
             "userUIN BIGINT NOT NULL, "
             "validUntil INT NOT NULL, "
             "originLat INT NOT NULL, "
             "originLon INT NOT NULL, "
             "originAngle SMALLINT NOT NULL, "
             "destinationLat INT NOT NULL, "
             "destinationLon INT NOT NULL, "
             "routeCosts INT NOT NULL, "
             "vehicle INT NOT NULL, "
             "time INT NOT NULL, "
             "minWaitTime INT NOT NULL, "
             "driverFlags INT NOT NULL, "
             "urmask BIGINT NOT NULL, "
             "PRIMARY KEY (routeID, createTime ),"
             "INDEX RouteStorageCoordsValidUntil (validUntil),"
             "INDEX RouteStorageCoordsUserKey (userUIN), "
             "CONSTRAINT routestoragecoordsrefUser "
             "FOREIGN KEY (userUIN) REFERENCES "
             "ISABUserUser(UIN) ON DELETE CASCADE )" );
   
   addTable( tableNbr++, "ISABStoredUserData",
             "CREATE TABLE ISABStoredUserData ( "
             "userUIN BIGINT NOT NULL, "
             "dataKey VARCHAR(255) NOT NULL, "
             "dataValue MC2BLOB NOT NULL, "
             "PRIMARY KEY ( userUIN, dataKey ), "
             "CONSTRAINT storedUserDataRefUser "
             "FOREIGN KEY ( userUIN ) REFERENCES "
             "ISABUserUser( UIN ) ON DELETE CASCADE )" );

   addTable( tableNbr++, "WFActivationCodes",
             "CREATE TABLE WFActivationCodes ( "
             "ActivationCode VARCHAR(20) NOT NULL, "
             "MC2UIN BIGINT DEFAULT NULL, "
             "Info VARCHAR(255) NOT NULL, "
             "Rights MC2BLOB NOT NULL, "
             "Server VARCHAR(255) DEFAULT NULL, "
             "IP VARCHAR(15) DEFAULT NULL, "
             "UseTime INT DEFAULT NULL, "
             "UserAgent VARCHAR(200) DEFAULT NULL, "
             "UserInput MC2BLOB DEFAULT NULL, "
             "UpdateTime INT DEFAULT NULL, "
             "PRIMARY KEY ( ActivationCode ), "
             "INDEX WFActivationCodesMC2UIN (MC2UIN) )" );   

   UserNavigator* nav = new UserNavigator( (uint32) 0 );
   const char* tmpStr = nav->getDB()->createTableQuery();
   addTable( tableNbr++, DBUserNavigator::getTableName(), tmpStr );
   delete [] tmpStr;
   delete nav;

   DBUserNavDestination* navDest = new DBUserNavDestination();
   tmpStr = navDest->createTableQuery();
   addTable( tableNbr++, DBUserNavDestination::getTableName(), tmpStr );
   delete [] tmpStr;
   delete navDest;

   MapUpdateDBaseObject mapUpdateDBaseObj;
   tmpStr = mapUpdateDBaseObj.createTableQuery();
   addTable( tableNbr++, mapUpdateDBaseObj.getTableStr(), tmpStr );
   delete [] tmpStr;

   return numTables;
}


bool
UserProcessor::checkForColumnInISABUserUser( 
   CharEncSQLQuery* sqlQuery, const char* columnName, 
   const char* columnDefinition, const char* afterColumnName )
{
   char query[ 4096 ];
   bool ok = true;
   mc2dbg4 << "  ISABUserUser: " << columnName << endl;
   sprintf( query, (MC2String( "SELECT UIN, " ) + columnName + 
                    " FROM ISABUserUser WHERE UIN = 1" ).c_str() );
   if ( !doQuery( sqlQuery, query, 
                  (MC2String("UP::checkForColumnInISABUserUser() "
                             "DB Upgrade " ) + columnName ).c_str() ) )
   {
      mc2dbg4 << "Missing in ISABUserUser: " << columnName << ", adding it"
              << endl;
      sprintf( query, (MC2String("ALTER TABLE ISABUserUser ADD COLUMN ") +
                       columnName + " " + columnDefinition + " AFTER " + 
                       afterColumnName ).c_str() );
      if ( !doQuery( sqlQuery, query, 
                     (MC2String("UP::checkForColumnInISABUserUser() "
                                "DB Upgrade " ) + columnName ).c_str() ) )
      {
         ok = false;
         mc2log << error << "Error adding " << columnName << " to "
                << "ISABUserUser! Please check and correct." << endl;
      } else {
         mc2log << "Added " << columnName << " in table ISABUserUser." 
                << endl;
      }
   } // Else found and all is ok

   return ok;
}

bool
UserProcessor::checkForColumnInISABUserUINTable( 
   CharEncSQLQuery* sqlQuery, const MC2String& tableName,
   const char* columnName, 
   const char* columnDefinition, const char* afterColumnName,
   bool& added )
{
   char query[ 4096 ];
   bool ok = true;
   added = false;
   mc2dbg4 << "  " << tableName << ": " << columnName << endl;
   sprintf( query, (MC2String( "SELECT userUIN, " ) + columnName + 
                    " FROM " + tableName + " WHERE userUIN = 1" ).c_str());
   if ( !doQuery( sqlQuery, query, 
                  (MC2String("UP::checkForColumnInISABUserUINTable() "
                             "DB Upgrade " ) + tableName + " Column " 
                   + columnName ).c_str() ) )
   {
      mc2dbg4 << "Missing in " << tableName << ": " << columnName 
              << ", adding it" << endl;
      sprintf( query, (MC2String("ALTER TABLE ") + tableName + 
                       " ADD COLUMN " + columnName + " " + 
               columnDefinition + " AFTER " + 
               afterColumnName ).c_str() );
      if ( !doQuery( sqlQuery, query, 
                     (MC2String("UP::checkForColumnInISABUserUINTable() "
                                "DB Upgrade " ) + tableName + " Column " 
                      + columnName ).c_str() ) )
      {
         ok = false;
         mc2log << error << "Error adding " << columnName << " to "
                << tableName << "! Please check and correct." << endl;
      } else {
         mc2log << "Added " << columnName << " in table " << tableName 
                << "." << endl;
         added = true;
      }
   } // Else found and all is ok

   return ok;
}

bool
UserProcessor::makeRouteStorageString( char* s, const char* preFix,
                                       const RouteReplyPacket* routePack,
                                       uint32 routeID,
                                       uint32 createTime,
                                       uint32 UIN,
                                       const char* const extraUserinfo,
                                       uint32 validUntil,
                                       int32 originLat,
                                       int32 originLon,
                                       uint32 originMapID,
                                       uint32 originItemID,
                                       uint16 originOffset,
                                       int32 destinationLat,
                                       int32 destinationLon,
                                       uint32 destinationMapID,
                                       uint32 destinationItemID,
                                       uint16 destinationOffset ) const
{
   bool ok = true;
   
   uint32 pos = sprintf( s, "%s( %d, %d, "
                         "%u, %d, '%s', %d, "
                         "%d, %d, %d, %d, %d, "
                         "%d, %d, %d, %d, %d, '",
                         preFix,
                         routeID, createTime, UIN, int32(validUntil),
                         extraUserinfo, routePack->getLength(),
                         originLat, originLon, 
                         originMapID, originItemID, originOffset,
                         destinationLat, destinationLon, 
                         destinationMapID, destinationItemID, 
                         destinationOffset );
   ulong compLen = 12 + uint32( ceil( routePack->getLength() * 1.01 ) );
   byte compData[ compLen ];
   if ( compress2( compData, &compLen, routePack->getBuf(), 
                   routePack->getLength(), Z_DEFAULT_COMPRESSION ) != Z_OK ) 
   {
      mc2log << "UP::makeRouteStorageString(): "
             << " zlib compression of route contents failed!" << endl;
      ok = false;
   }
   
   if ( !StringUtility::base64Encode( compData, compLen,
                                      &s[ pos ] ) ) {
      mc2log << error << "UP::makeRouteStorageString(): "
             << " Base64 encoding of route contents failed!" << endl; 
      ok = false;
   } else {
      strcat( s, "')" );
   }

   return ok;
}

MC2String
UserProcessor::makeRouteStoragePath( uint32 routeID,
                                     uint32 createTime ) const {
   MC2String str;

   const char* pathStr = Properties::getProperty( "ROUTE_STORAGE_PATH" );
   uint32 fileNameLen = 512 + strlen( pathStr );
   char fileName[ fileNameLen ];
   struct tm tm;
   time_t rtime = createTime;
   gmtime_r( &rtime, &tm );
   size_t tres = strftime( fileName, fileNameLen, pathStr, &tm );
   if ( tres > 0 ) {
      str = fileName;
      File::mkdir_p( str );
      if ( !str.empty() && str[ str.size() -1 ] != '/' ) {
         str.append( "/" );
      }
      STLStringUtility::uint2str( routeID, str );
      str.append( "_" );
      STLStringUtility::uint2str( createTime, str );
   } else {
      mc2log << warn << "UP::makeRouteStoragePath "
             << "couldn't strftime " << MC2CITE( pathStr ) << endl;
   }
   return str;
}

byte*
UserProcessor::storedRouteToBuffer( const char* buff, 
                                    uint32 routePacketLength,
                                    uint32 routeID,
                                    uint32 createTime ) const
{
   byte* routeBuf = new byte[ routePacketLength ];
   ulong compLen = 12 + uint32( ceil( routePacketLength * 1.01 ) );
   byte* compBuf = new byte[ compLen ];
   int compBufLength = StringUtility::base64Decode( buff, compBuf );
   if ( compBufLength == -1 ) {
      mc2log << error << "UP::storedRouteToBuffer "
             << "Failed to do base64 decode of route content, routeID: "
             << routeID << ", createTime: " << createTime << endl;
      delete [] routeBuf;
      routeBuf = NULL;
   }
   if ( routeBuf != NULL ) {
      ulong resSize = routePacketLength;
      if ( uncompress( routeBuf, &resSize, 
                       compBuf, compBufLength ) != Z_OK )
      {
         mc2log << error << "UP::handleRouteStorageGetRouteRequestPacket(): "
            "Failed to do zlib decompress of route content, routeID: "
                << routeID << ", createTime: " << createTime << endl;
         delete [] routeBuf;
         routeBuf = NULL;
      }
      routePacketLength = resSize;
   }
   delete [] compBuf;

   return routeBuf;
}

void
UserProcessor::setISABUserUserColumnName( 
   const MC2String& name, bool isNumeric, bool isKey ) {
   m_isabuseruser->setColumnName( SQLTableData::TableDataColumn( 
                                     name, isNumeric, isKey ) );
}

bool
UserProcessor::getISABUserUserColumnValue( 
   CharEncSQLQuery* sqlQuery, const MC2String& whereTag, uint32 uin,
   const MC2String& column, int32& number ) const {
   bool ok = true;
   MC2String uinStr( STLStringUtility::uint2str( uin ) );
   SQLDataContainer::rowName_t names( 1, SQLDataContainer::NameAndIsNum(
                                         "UIN", true ) );
   SQLDataContainer::rowValues_t::value_type values( 1, uinStr );
   SQLDataContainer cont( names, values );
   MC2String value;
   // TODO: Only get column, not all columns
   if ( SQLQueryHandler::fetchData( 
           sqlQuery, whereTag, cont, *m_isabuseruser, cont ) && 
        cont.getCol( 0, column, value ) ) {
      number = STLStringUtility::strtol( value );
   } else {
      ok = false;
   }
   return ok;
}

bool
UserProcessor::setISABUserUserColumnValue( 
   CharEncSQLQuery* sqlQuery, const MC2String& whereTag, uint32 uin,
   uint32 changerUIN, const MC2String& column, int32 number )
{
   bool ok = true;
   char query[ 4096 ];
   sprintf( query, "UPDATE ISABUserUser SET %s = %d WHERE UIN = %u",
            column.c_str(), number, uin );
   
   if ( ! doQuery( sqlQuery, query, "UP::setISABUserUserColumnValue()" ) ) {
      mc2log << warn << "UP::setISABUserUserColumnValue(): "
             << "Set " << column <<  " column to " << number << " failed! UIN "
             << uin << endl;
      ok = false;
   } else {
      // Change log
      UserUser user( uin );
      addUserChangeToChangelog( &user, changerUIN, sqlQuery );
   }
   
   return ok;
}

bool
UserProcessor::initialCheckDatabase( bool noSqlUpdate ) {
   mc2dbg2 << "Doing initial database check" << endl;
   
   if ( !createNonExistentTables() ) {
      return false;
   }
   
   if ( !noSqlUpdate ) {
      mc2dbg2 << "Checking for DB updates" << endl;
      sqlUpdate();
   }
   else {
      mc2dbg2 << "Not checking for DB updates, because noSqlUpdate "
              << "is set." << endl;
   }
   
   return testDatabase();
}

bool
UserProcessor::createNonExistentTables() {
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );
   
   for (int i = 0; i < m_numTables; i++) {
      mc2dbg4 << "Checking table (" << i << ") \"" << m_tableNames[ i ] 
              << "\"" << endl;
      if (! m_sqlConnection->tableExists(m_tableNames[i])) {
         // create the table
         if (! sqlQuery->prepAndExec(m_tableCreateQueries[i]) && 
               sqlQuery->getError() > 0) {
            mc2log << error << "Table (" << i << ") name " 
                   << m_tableNames[i] << " not found in the database and "
                   << "table creation has failed with the error: \"" 
                   << sqlQuery->getErrorString() 
                   << "\", the query was: \"" 
                   << m_tableCreateQueries[i] << "\"" << endl;
            return false;
         }
         if ((m_tableExtraQueries[i] != NULL) &&
             (! sqlQuery->prepAndExec(m_tableExtraQueries[i])) &&
                sqlQuery->getError() > 0) {
            mc2log << error << " Extra SQL initialization for table"
                   << m_tableNames[i]
                   << " has failed with the error: \""
                   << sqlQuery->getErrorString() 
                   << "\", the query was: \"" 
                   << m_tableExtraQueries[i] << "\"" << endl;
            return false;
         }
         mc2dbg4 << "  <<>> created" << endl;
         if (strcmp(m_tableNames[i], "ISABUserUser") == 0)
            if(!addDefaultUser())
               return false;
         if (strcmp(m_tableNames[i], "ISABCellularModels") == 0)
            if(!addDefaultPhoneModels())
               return false;
         if ( strcmp( m_tableNames[ i ], "ISABUserRight" ) == 0 ) {
            if ( !addDefaultRight() ) {
               return false;
            }
         }
      } else {
         mc2dbg4 << "  <<>> OK" << endl;
      }
   }
   return true;
}

void 
UserProcessor::sqlUpdate() {
   char query[4096];
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );
   // creationTime, added in ISABUserUser 2003-11-10
   // NOTE! HAS to be the LAST column in the table for now, make
   // sure that it is added last if you do any other changes or
   // larger changes to the code than the current handling in addUser
   // will be needed!
   mc2dbg4 << "  ISABUserUser: creationTime" << endl;
   sprintf(query, "SELECT UIN, creationTime FROM ISABUserUser WHERE UIN = 1");
   if (!doQuery(sqlQuery.get(), query, "UP::initialCheckDatabase() DB Upgrade 1")) {
      mc2dbg4 << "Missing in ISABUserUser: creationTime, adding it" << endl;
      sprintf(query, "ALTER TABLE ISABUserUser ADD COLUMN creationTime DATE AFTER transactionBased");
      if (!doQuery(sqlQuery.get(), query, 
                   "UP::initialCheckDatabase() ISABUserUser creationTime")) {
         mc2log << error << "Error updating ISABUserUser! Please check and correct.";
      } else {
         mc2log << "Updated table ISABUserUser" << endl;
      }
   }

   // Transactionbased, modified in ISABUserUser 2003-11-2X
   sprintf( query, "SELECT UIN, transactionBased FROM ISABUserUser "
            "where UIN = 1" );
   if ( ! doQuery( sqlQuery.get(), query, "UP::initialCheckDatabase() "
                   "DB Upgrade 2" ) )
   {
      mc2log << error << "Missing in ISABUserUser: transactionBased"
             << endl;
   } else {
      if ( sqlQuery->nextRow() ) {
         if ( sqlQuery->getColumn( 1 )[ 0 ] == 't' ||
              sqlQuery->getColumn( 1 )[ 0 ] == 'f' )
         {
            // Modify
            m_sqlConnection->beginTransaction();
            // Drop old
            if ( doQuery( sqlQuery.get(), "ALTER TABLE ISABUserUser DROP "
                          "COLUMN transactionBased", 
                          "UP::initialCheckDatabase() DB Upgrade 2 DROP "
                          "transactionBased" ) )
            {
               // Add new 
               if ( doQuery( sqlQuery.get(), "ALTER TABLE ISABUserUser ADD "
                             "COLUMN transactionBased SMALLINT DEFAULT 0 "
                             "AFTER externalXMLService", 
                             "UP::initialCheckDatabase() DB Upgrade 2 ADD "
                             "transactionBased" ) )
               {
                  // Ok!
                  m_sqlConnection->commitTransaction();
               } else {
                  mc2log << error << "UP::initialCheckDatabase() "
                         << "DB Upgrade 2 ADD "
                         << "transactionBased failed. "
                         << "Restart to try again." << endl;
                  m_sqlConnection->rollbackTransaction();
               }
            } else {
               mc2log << error << "UP::initialCheckDatabase() "
                      << "DB Upgrade 2 DROP "
                      << "transactionBased failed. Restart to try again."
                      << endl;
               m_sqlConnection->rollbackTransaction();
            }
         } // Else ok
      } else {
         mc2log << error << "No isabuser in db, cannot check "
                << "transactionBased" << endl;
      }
   }

   // deviceChanges       INT           (default -1 = unlimited)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "deviceChanges", "INT DEFAULT -1", "transactionBased" );
   // supportComment      MC2BLOB       (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "supportComment", "MC2BLOB NOT NULL", 
      "deviceChanges" );
   // postalCity          VARCHAR(100)  (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "postalCity", "VARCHAR(100) NOT NULL", 
      "supportComment" );
   // zipCode             VARCHAR(40)   (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "zipCode", "VARCHAR(40) NOT NULL", 
      "postalCity" );
   // companyName         VARCHAR(255)  (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "companyName", "VARCHAR(255) NOT NULL", 
      "zipCode" );
   // companyReference    VARCHAR(255)  (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "companyReference", "VARCHAR(255) NOT NULL", 
      "companyName" );
   // companyVATNbr       VARCHAR(50)   (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "companyVATNbr", "VARCHAR(50) NOT NULL", 
      "companyReference" );
   // emailBounces  bool  INT           (default 0 = false)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "emailBounces", "INT DEFAULT 0", 
      "companyVATNbr" );
   // addressBounces  bool INT           (default 0 = false)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "addressBounces", "INT DEFAULT 0", 
      "emailBounces" );
   // customerContactInfo VARCHAR(255)  (default not null)
   checkForColumnInISABUserUser( 
      sqlQuery.get(), "customerContactInfo", "VARCHAR(255) NOT NULL", 
      "addressBounces" );

   // group in ISABUserToken 20060608
   bool added = false;
   checkForColumnInISABUserUINTable( 
      sqlQuery.get(), "ISABUserToken", "group_name", "VARCHAR(255) DEFAULT NULL",
      "token", added );
   if ( added ) {
      // Update Earth java tokens by setting IRON group. (Last last client
      // is java earth)
      if (!doQuery(sqlQuery.get(), "update ISABUserToken left join "
                   "ISABUserLastClient on ISABUserToken.userUIN = "
                   "ISABUserLastClient.userUIN set group_name = \"IRON\" "
                   "where client_type = 'je-1-earth'", 
                   "UP::initialCheckDatabase() ISABUserToken update "
                   "Earth java tokens" ) ) 
      {
         mc2log << error << "Error updating Earth java tokens in "
                << "ISABUserToken ! Please check and correct.";
      } else {
         mc2log << "Updated java earth tokens ok." << endl;
      }
   }
   checkForColumnInISABUserUINTable( 
      sqlQuery.get(), "ISABUserTokenchangelog", "group_name", 
      "VARCHAR(255) DEFAULT NULL", "token", added );

   // origin in ISABUserLastClient 20060713
   checkForColumnInISABUserUINTable( 
      sqlQuery.get(), "ISABUserLastClient", "origin", 
      "MC2BLOB DEFAULT '' NOT NULL", "extra", added );
   checkForColumnInISABUserUINTable( 
      sqlQuery.get(), "ISABUserLastClientchangelog", "origin", 
      "MC2BLOB DEFAULT '' NOT NULL", "extra", added );
}

bool
UserProcessor::testDatabase() {
   char query[4096];
   auto_ptr<CharEncSQLQuery> sqlQuery( m_sqlConnection->newQuery() );
   
   // begin with starting a new transaction
  if ( !m_sqlConnection->beginTransaction()) {
      mc2log << error << "UP::initialCheckDatabase(): "
             << " beginTransaction() failed! Error: " 
             << m_sqlConnection->getErrorString() << endl;
      return false;
  }

   // Find User with UIN -1
  sprintf(query,
          "SELECT UIN, logonID, firstname, initials, lastname, sessionID,"
          "measurementSystem, language, lastdestmapID, lastdestitemID,"
          "lastdestOffset, lastdestTime, lastdestString,  lastorigmapID,"
          "lastorigitemID, lastorigOffset, lastorigTime, lastorigString,"
          "searchType, searchSubstring, searchSorting, searchObject,"
          "routeCostA, routeCostB, routeCostC, routeCostD, routeType,"
          "editMapRights, editDelayRights, editUserRights, wapService,"
          "htmlService, operatorService, nbrMunicipals, municipals,"
          "vehicleType, birthDate, routeImageType, validDate, gender,"
          "smsService, defaultCountry, defaultMunicipal, defaultCity,"
          "searchDbMask "
          "navService, operatorComment, emailAddress, "
          "address1, address2, address3, address4, address5, "
          "routeTurnImageType, externalXMLService, transactionBased, "
          "deviceChanges, supportComment, postalCity, zipCode, "
          "companyName, companyReference, companyVATNbr, emailBounces, "
          "addressBounces, customerContactInfo "
          "FROM ISABUserUser WHERE UIN = %u", 0);

   if ( !doQuery(sqlQuery.get(), query, "UP::initialcheckDatabase() User check"))
      return false;

   if ( sqlQuery->nextRow()) {
      mc2log << error << "UP::initialCheckDatabase(): "
             << "Database ERROR it contains a user with UIN = -1 !" << endl;
      return false;
   } else {
      uint32* mun = new uint32[1];
      mun[0] = 0;
      // Try to add User with UIN 0
      UserUser* user = new UserUser( 0, "ISABtestUser", "First", 
                                     "FL", "Last", "", 
                                     UserConstants::MEASUREMENTTYPE_METRIC,
                                     StringTable::ENGLISH, MAX_UINT32, 
                                     MAX_UINT32, MAX_UINT16, 0, "",
                                     MAX_UINT32, MAX_UINT32, MAX_UINT16, 
                                     0, "","000000", 
                                     0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 
                                     ItemTypes::passengerCar, 0, 0, 0, 
                                     false, false, false,
                                     1, mun,
                                     UserConstants::ROUTEIMAGETYPE_NONE,
                                     25000, UserConstants::GENDERTYPE_MALE,
                                     false,"Sverige","Lund","Lund",
                                     false, "Auto test", "a@b", "AStreet",
                                     "A-Zip A-ZipArea", "A-Country", "", 
                                     "", UserConstants::
                                     ROUTE_TURN_IMAGE_TYPE_PICTOGRAM, 
                                     false, 
                                     UserConstants::NO_TRANSACTIONS, -1,
                                     "No support", "PTown", "12345", 
                                     "Företag AB", "", "OrgNr.",
                                     0, 0, "No contact" );

      strcpy( query, "INSERT INTO ISABUserUser VALUES ( ");
      addUserUserData( query + strlen( query ), user, (uint32)-1, "assword" );
      strcat( query, ", NOW())");
      if ( !doQuery(sqlQuery.get(), query, "UP::initialcheckDatabase() User add"))
         return false;

      delete user;
   }
   
   // Try to find UserCellular with userUIN -1
   strcpy( query, "SELECT * FROM ISABUserCellular WHERE userUIN = 0");
   if ( ! doQuery(sqlQuery.get(), query,
               "UP::initialcheckDatabase() Cellular check query"))
      return false;

   if ( sqlQuery->nextRow()) {
      mc2log << error << "UP::initialCheckDatabase(): "
             << "Database ERROR it contains a cellular referring to a user "
             << "with UIN = -1 !" << endl;
      return false;
   } else {
      // Add a Cellular with ID -1 and userUIN -1
      CellularPhoneModel* model = new CellularPhoneModel( 
         "UNKNOWN", "", 10, UserConstants::EOLTYPE_AlwaysCRLF, 2, true,
         0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
         CellularPhoneModel::SMSCONCATENATION_NO, 
         CellularPhoneModel::SMSGRAPHICS_NO, 
         CellularPhoneModel::WAPCAPABLE_NO,
         "", 00, "Default unknown phone" );
      UserCellular* cellular = new UserCellular( 
         0, "46XXXXXXXXX", 0, 6, 5,true, model, false,
         UserConstants::POSTYPE_NO_POSITIONING, "AB", "ab", 0, 0, 0,
         0, 0, 0, 0);

      strcpy(query, "INSERT INTO ISABUserCellular VALUES ( ");
      addUserCellularData( query + strlen( query ),
                           (uint32)-1, (uint32)-1, cellular );
      strcat( query, " )");

      if ( ! doQuery(sqlQuery.get(), query,
                 "UP::initialcheckDatabase() Cellular check add"))
         return false;

      delete cellular;
   }

   
   // Add debit
   strcpy( query, "INSERT INTO ISABDebit VALUES ( ");
   addDebitData( query + strlen( query ), TimeUtility::getRealTime(),
                 0, "9999", "46XXXXXXXXX", 0, 0, 1, MAX_UINT16, 
                    "___Test of Debit___" );
   strcat( query, " )" );

   if ( ! doQuery(sqlQuery.get(), query, "UP::initialcheckDatabase() Debit add"))
      return false;

   // Remove debit
   // This takes too long since there's no index, removed for now, 
   // the rollback below removes it anyway
//   sprintf( query, "DELETE FROM ISABDebit WHERE %s",
//            "operationDescription = '___Test of Debit___'" );
//   if ( ! doQuery(sqlQuery, query, "UP::initialcheckDatabase() Debit delete"))
//      return false;

   // Remove cellular
   strcpy( query, "DELETE FROM ISABUserCellular WHERE userUIN = 0");
   if ( ! doQuery(sqlQuery.get(), query, "UP::initialcheckDatabase() Cell delete"))
      return false;

   // Remove useruser
   strcpy( query, "DELETE FROM ISABUserUser WHERE UIN = 0");
   if ( ! doQuery(sqlQuery.get(), query, "UP::initialcheckDatabase() User delete"))
      return false;

   // End testing and scrap all changes (Not that there are any but anyway)
   if ( !m_sqlConnection->rollbackTransaction()) {
      mc2log << fatal << "UP::initialCheckDatabase():  " 
             << "Couldn't rollback transaction! Error: "
             << m_sqlConnection->getErrorString() << endl;
      return false;
   }

   // Everything went fine
   return true;
}

bool
UserProcessor::addPhoneModel(CellularPhoneModel* model) 
{
   char query[4096];
   bool ret = true;
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

   strcpy( query, "INSERT INTO ISABCellularModels VALUES ( ");
   addCellularModelData( query + strlen( query ), model );
   strcat( query, " )" );
   if ( ! doQuery(sqlQuery, query, "UP::addPhoneModel()") ) {
      mc2log << error << "UP::addPhoneModel(): Failed adding phone model:!"
             << endl;
      ret = false;
   }

   delete sqlQuery;
   return ret;
}

bool 
UserProcessor::addDefaultPhoneModels()
{
   bool ok = true;
   CellularPhoneModel* model;

   mc2dbg8 << "UP::addDefaultPhoneModels()" << endl;
   // Unknown
   model = new CellularPhoneModel( 
      "UNKNOWN", "Default", 10, UserConstants::EOLTYPE_AlwaysCRLF, 1, 
      false,
      0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_NO, 
      CellularPhoneModel::SMSGRAPHICS_NO, 
      CellularPhoneModel::WAPCAPABLE_NO,
      "", 00, "Standard unknown phone type" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;

   // Debug Xterm
   model = new CellularPhoneModel( 
      "Debug Xterm", "Default", 12, UserConstants::EOLTYPE_AlwaysLF, 1, 
      false,
      0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_NO, 
      CellularPhoneModel::SMSGRAPHICS_NO, 
      CellularPhoneModel::WAPCAPABLE_NO,
      "", 00, "Debug phone type for Xterm display" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;


   // Old Nokia
   model = new CellularPhoneModel( 
      "Old Nokia", "Default", 12, UserConstants::EOLTYPE_CRLF, 1, false,
      0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_NO, 
      CellularPhoneModel::SMSGRAPHICS_NO, 
      CellularPhoneModel::WAPCAPABLE_NO,
      "", 00, "Default type for old Nokia cellulars" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;

   // New Nokia
   model = new CellularPhoneModel( 
      "New Nokia", "Default", 12, UserConstants::EOLTYPE_AlwaysCRLF, 2, 
      false,
      0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_NO, 
      CellularPhoneModel::SMSGRAPHICS_NO, 
      CellularPhoneModel::WAPCAPABLE_NO,
      "", 00, "Default type for new Nokia cellulars" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;
   
   // Old Ericsson
   model = new CellularPhoneModel( 
      "Old Ericsson", "Default", 10, UserConstants::EOLTYPE_CR, 1, false,
      0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_NO, 
      CellularPhoneModel::SMSGRAPHICS_NO, 
      CellularPhoneModel::WAPCAPABLE_NO,
      "", 00, "Default type for old Ericsson cellulars" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;
   
   // New Ericsson
   model = new CellularPhoneModel( 
      "New Ericsson", "Default", 12, UserConstants::EOLTYPE_AlwaysCR, 2, 
      false,
      0, 0, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_NO, 
      CellularPhoneModel::SMSGRAPHICS_NO, 
      CellularPhoneModel::WAPCAPABLE_NO,
      "", 00, "Default type for new Ericsson cellulars" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;

   // 6600
   model = new CellularPhoneModel( 
      "6600", "Nokia", 20, UserConstants::EOLTYPE_AlwaysCRLF, 7, 
      true,
      176, 208, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_YES, 
      CellularPhoneModel::SMSGRAPHICS_YES, 
      CellularPhoneModel::WAPCAPABLE_YES_GPRS,
      "", 2003, "" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;

   // k750i
   model = new CellularPhoneModel( 
      "k750i", "Sony Ericsson", 20, UserConstants::EOLTYPE_AlwaysCRLF, 19, 
      true,
      176, 220, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_YES, 
      CellularPhoneModel::SMSGRAPHICS_YES, 
      CellularPhoneModel::WAPCAPABLE_YES_GPRS,
      "", 2005, "" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;

   // N71
   model = new CellularPhoneModel( 
      "N71", "Nokia", 20, UserConstants::EOLTYPE_AlwaysCRLF, 19, 
      true,
      320, 240, CellularPhoneModel::SMSCAPABLE_YES, 
      CellularPhoneModel::SMSCONCATENATION_YES, 
      CellularPhoneModel::SMSGRAPHICS_YES, 
      CellularPhoneModel::WAPCAPABLE_YES_GPRS,
      "", 2005, "" );
   if (! addPhoneModel(model))
      ok = false;
   delete model;

   return ok;
}



bool 
UserProcessor::addDefaultUser() {
   bool ok = true;
   char query[4096];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
   uint32* mun = new uint32[1];
   mun[0] = 0;

   // Add default User with UIN 1
   UserUser* user = new UserUser( 1, "ISABUser", "Itinerary", 
                                  "ISAB", "Systems", "", 
                                  UserConstants::MEASUREMENTTYPE_METRIC,
                                  StringTable::ENGLISH, MAX_UINT32, 
                                  MAX_UINT32, MAX_UINT16, 0, "",
                                  MAX_UINT32, MAX_UINT32, MAX_UINT16, 
                                  0, "","000000", 
                                  SearchTypes::CloseMatch,
                                  SearchTypes::Beginning,
                                  SearchTypes::ConfidenceSort,
                                  (SEARCH_STREETS | 
                                   SEARCH_COMPANIES | 
                                   SEARCH_CATEGORIES),
                                  SearchTypes::AllDB,
                                  0, 1, 0, 0, 0, 
                                  ItemTypes::passengerCar, 1, 1, 1, 
                                  true, true, true,
                                  1, mun,
                                  UserConstants::ROUTEIMAGETYPE_NONE,
                                  0, UserConstants::GENDERTYPE_MALE,
                                  true, "Sweden", "Lund", "Lund",
                                  true, "Op. comment", 
                                  "mc2@localhost.localdomain", 
                                  "Wayfinder Systems AB",
                                  "RegionHuset", "Baravägen 1", 
                                  "22122 LUND", "Sweden", UserConstants::
                                  ROUTE_TURN_IMAGE_TYPE_PICTOGRAM, true,
                                  UserConstants::NO_TRANSACTIONS, -1,
                                  "No support", "Lund", "221 05", 
                                  "Wayfinder Systems AB", "", "VAT",
                                  0, 0, "No contact" );
   strcpy( query, "INSERT INTO ISABUserUser VALUES ( ");
   addUserUserData( query + strlen( query ), user, 1, "password" );
   strcat( query, ", 0 )");

   if ( ! doQuery(sqlQuery, query, "UP::addDefaultUser()") ) {
      ok = false;
   }

   delete user;
   delete sqlQuery;

   if ( ok ) {
      // Also add iphpuser
      CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();
      mun = new uint32[1];
      mun[0] = 0;

      // Add default User with UIN 2
      user = new UserUser( 2, "iphpuser", "Php", 
                           "User", "", "", 
                           UserConstants::MEASUREMENTTYPE_METRIC,
                           StringTable::ENGLISH, MAX_UINT32, 
                           MAX_UINT32, MAX_UINT16, 0, "",
                           MAX_UINT32, MAX_UINT32, MAX_UINT16, 
                           0, "","", 
                           SearchTypes::CloseMatch,
                           SearchTypes::Beginning,
                           SearchTypes::ConfidenceSort,
                           (SEARCH_STREETS | 
                            SEARCH_COMPANIES | 
                            SEARCH_CATEGORIES),
                           SearchTypes::AllDB,
                           0, 1, 0, 0, 0, 
                           ItemTypes::passengerCar, 1, 1, 1, 
                           true, true, true,
                           1, mun,
                           UserConstants::ROUTEIMAGETYPE_NONE,
                           0, UserConstants::GENDERTYPE_MALE,
                           true, "Sweden", "Lund", "Lund",
                           true, "Added when creating db", 
                           "please_dont_reply@localhost.localdomain", 
                           "Wayfinder Systems AB",
                           "RegionHuset", "Baravägen 1", 
                           "22122 LUND", "Sweden", UserConstants::
                           ROUTE_TURN_IMAGE_TYPE_PICTOGRAM, true,
                           UserConstants::NO_TRANSACTIONS, -1,
                           "No support", "Lund", "221 05", 
                           "Wayfinder Systems AB", "", "VAT",
                           0, 0, "No contact" );
      strcpy( query, "INSERT INTO ISABUserUser VALUES ( ");
      addUserUserData( query + strlen( query ), user, 2, "password" );
      strcat( query, ", 0 )");

      if ( ! doQuery(sqlQuery, query, "UP::addDefaultUser() iphpuser") ) {
         ok = false;
      }

      delete user;
      delete sqlQuery;
   }
   
   return ok;
}


bool
UserProcessor::addDefaultRight() {
   bool ok = true;
   char query[ 4096 ];
   CharEncSQLQuery* sqlQuery = m_sqlConnection->newQuery();

      
   // Add the right to user 2 (iphpuser)
   UserRight* right = new UserRight( 
      2, 0, UserEnums::URType( UserEnums::UR_GOLD, UserEnums::UR_XML ),
      2147483647, 0, 2147483647, false, "SUP: Needs webpages" );
   strcpy( query, "INSERT INTO ISABUserRight VALUES ( ");
   addUserRightData( query + strlen(query), 2/*ID*/, 2/*UIN*/, right );
   strcat( query, " )" );
   
   if ( ! doQuery( sqlQuery, query, 
                   "UP::addDefaultRight() add iphpuser right" ) )
   {
      ok = false;
   }
   
   delete right;
   
   return ok;
}

bool 
UserProcessor::readOnly( const RequestPacket* request ) {
#ifdef PARALLEL_USERMODULE
   switch ( request->getSubType() ) {
   case Packet::PACKETTYPE_USERFINDREQUEST:
   case Packet::PACKETTYPE_GETCELLULARPHONEMODELDATAREQUEST:
   case Packet::PACKETTYPE_LISTDEBITREQUESTPACKET:
   case Packet::PACKETTYPE_GETUSERNAVDESTINATIONREQUESTPACKET:
   case Packet::PACKETTYPE_USER_AUTH_REQUEST:
   case Packet::PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REQUEST:
   case Packet::PACKETTYPE_GETUSERTRACK_REQUEST:
   case Packet::PACKETTYPE_POIREVIEW_LIST_REQUEST:
   case Packet::PACKETTYPE_GET_STORED_USER_DATA_REQUEST:
      return true;
   default:
      return false;
   }
#else
   return false;
#endif
}

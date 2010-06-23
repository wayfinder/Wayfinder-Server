/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserExternalAuth.h"
#include "ParserThreadGroup.h"
#include "STLStringUtility.h"
#include "SSLSocket.h"
#include "HttpHeader.h"
#include "HttpHeaderLines.h"
#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "Properties.h"
#include "HttpInterfaceRequest.h"
#include "ParserExternalAuthHttpSettings.h"
#include "ParserTokenHandler.h"
#include "URLFetcher.h"
#include "URL.h"
#include "LocaleUtility.h"
#include "ClientSettings.h"
#include "TimeUtility.h"
#include "NetUtility.h"
#include "IP.h"
#include "OperationTypes.h"
#include "MC2Digest.h"
#include "TimedOutSocketLogger.h"
#include "TimedOutSocketInfo.h"

#include <regex.h>

/*
#   ifndef DEBUG_LEVEL_4
#      define DEBUG_LEVEL_4
#      define undef_level_4
#   endif
*/

/// \namespace Parser External Auth (PEA) Result Codes
namespace PEAResultCode {
enum ResultCode {
   OK = 0,
   ERROR = -1,
   TIMEOUT = -2,
   EXPIRED = -3,
   UNAUTHORIZED = -4,
};

} // PEResultCode

bool ParserExternalAuth::HttpSettingsCmp::operator() ( 
   const ParserExternalAuthHttpSettings* a,
   const ParserExternalAuthHttpSettings* b ) const
{
   return *a < *b;
}


ParserExternalAuth::ParserExternalAuth( 
   ParserThread* thread,
   ParserThreadGroup* group )
      : ParserHandler( thread, group ),
        m_mc2Digest( new MC2Digest( "DSA-SHA1" ) )
{
#ifdef USE_SSL
   m_ctx = m_group->getSSLContext();
#endif

   // Add this property to 
   // ParserExternalAuthHttpSettings if more is ever needed.
   // Or even ClientSettings.
   // See getPopup for how this is used.
#if 0
   // Example popup setting
   m_popupDatas.insert( 
      make_pair(
         MC2String( "-unique-part-of-some-client-type" ), 
         popupData( "", 
                    StringTable::NOSTRING, // EXPIRED
                    StringTable::NOSTRING, // WELCOME
                    StringTable::WAYFINDER_JAVA_POPUP_TRIAL_MODE, // First part of trial starts message before the trial end date
                    StringTable::NOSTRING,// Second part of trial starts message after the trial end date
                    Properties::getProperty( "SOME_CLIENT_TYPE_START_URL", 
                                             "" ) ) ) );
#endif


   // Http External Auth settings

   ParserExternalAuthHttpSettings s;
   m_findSettings = new ParserExternalAuthHttpSettings( s );
   typedef UserEnums UE;
   typedef ParserExternalAuthHttpSettings PEH;

   // App Store (iPhone) clients, used by all App Store customers
   s.reqName = "AppStore";
   s.settingId = "AppStore";
   s.userHeader.clear();
   s.userHeaderListPartStart.clear();
   s.userHeaderListPartEnd.clear();
   s.userHeaderListPrefix.clear();
   s.logonIDHeader.clear();
   s.logonIDHeaderListPartStart.clear();
   s.logonIDHeaderListPartEnd.clear();
   s.statusHeader = "";
   s.okStatus = "";
   s.noStatusHeaderOK = false;
   s.ips.clear();
   s.userloginprefix = "ap";
   s.extraTypes.clear();
   // EXTRA, add the initial time for the purchase of the Application
   s.extraTypes.push_back( 
      make_pair( UE::URType( UE::UR_NO_LEVEL, UE::UR_APP_STORE_NAVIGATION ), 
                 ExtraAddInfo() ) );
   s.addLicence = false;
   s.addLicenceAsID = false;
   s.idAsKey = true;
   s.addTime = false;
   s.addOrigin = "";
   s.addRegionID = 2097173/*World*/;
   s.addTypes.clear();
   s.addYears = 0;
   s.addMonths = 0;
   s.addDays = 5;
   s.blockedDate = MAX_UINT32;
   s.lang = LangTypes::english;
   s.createLevel = WFSubscriptionConstants::GOLD;
   s.createYears = 0;
   s.createMonths = 0;
   s.createDays = 5;
   s.createExplicitTime = MAX_UINT32;
   s.createRegionID = 2097173/*World*/;
   s.createTransactionDays = 0;
   s.brand = "AppStore";
   s.clientType = "-apps";
   s.clientTypeEndsStr = true;
   s.userNamePrefix = "ap";
   s.extAuthFunc = &ParserExternalAuth::externalDevIDAuth;
   s.extAuthFuncCheckTime = 0; // The device id should be sent in all requests
   s.preAuthFunc = NULL;
   s.verifyIDFunction = NULL;
   s.createWFUserFunction = NULL;
   s.encryptIDFunction = NULL;
   s.usesHwKeys = true;
   s.userHeaderForServices = "";
   s.keepUpdatedHWKey = false;
   s.httpHeaderFunction = NULL;
   s.postAuthFunction = NULL;
   if ( ! m_httpSettings.
        insert( new ParserExternalAuthHttpSettings( s ) ).second ) {
      mc2log << fatal << "[PEA] Could not insert setting: " << s.reqName 
             << endl;
      MC2_ASSERT( false );
   }
}


ParserExternalAuth::~ParserExternalAuth() {
   delete m_findSettings;
   for ( HttpSettings::iterator it = m_httpSettings.begin() ; 
         it != m_httpSettings.end() ; ++it )
   {
      delete *it;
   }
}


MC2String::size_type
ParserExternalAuth::getNodeValue( const MC2String& name,
                                  const MC2String& data,
                                  MC2String& value,
                                  uint32 startPos ) const
{
   MC2String::size_type findPos = data.find( "<" + name + " ", startPos );
   if ( findPos == MC2String::npos ) {
      findPos = data.find( "<" + name + ">", startPos );
   }

   if ( findPos != MC2String::npos ) {
      // Find ">" end of start of node
      if ( data[ findPos + name.size() + 1 ] == ' ' ) {
         findPos = data.find( ">", findPos + name.size() + 1 );
      } else {
         findPos = findPos + name.size() + 1;
      }
      if ( findPos != MC2String::npos ) {
         // Find "<"
         MC2String::size_type endPos = data.find( "<", findPos + 1 );
         if ( endPos != MC2String::npos ) {
            value = data.substr( findPos + 1,
                                 endPos - findPos - 1 );  
         } else {
            findPos = endPos;
         }
      }
   }

   return findPos;
}


MC2String::size_type
ParserExternalAuth::getAllInNode( const MC2String& name,
                                  const MC2String& data,
                                  MC2String& value,
                                  uint32 startPos ) const
{
   MC2String::size_type findPos = data.find( "<" + name + " ", startPos );
   if ( findPos == MC2String::npos ) {
      findPos = data.find( "<" + name + ">", startPos );
   }

   if ( findPos != MC2String::npos ) {
      mc2dbg8 << "Found start of node." << endl;
      // Find end of start of node
      if ( data[ findPos + name.size() + 1 ] == ' ' ) {
         mc2dbg8 << "Found space after name " << endl;
         findPos = data.find( ">", findPos + name.size() + 1 );
      } else {
         mc2dbg8 << "Found > after name" << endl;
         findPos = findPos + name.size() + 1;
      }
      if ( findPos != MC2String::npos ) {
         mc2dbg8 << "Found end of node start." << endl;
         // Find "</name"  "/"  ">"
         MC2String endName( name.substr( 0, name.find( " " ) ) );
         mc2dbg8 << "endName " << endName << endl;
         MC2String::size_type endPos = data.find( "</" + endName + ">", 
                                                  findPos );
         if ( endPos != MC2String::npos ) {
            mc2dbg8 << "endPos " << endPos << " findPos " << findPos << endl;
            value = data.substr( findPos + 1,
                                 endPos - findPos - 1 );
            mc2dbg8 << "Found end node " << findPos << " "  << endPos
                   << " val " << value << endl;
         } else {
            findPos = endPos;
         }
      }
   }

   return findPos;
}


int
ParserExternalAuth::getAttribute( const MC2String& name,
                                  const MC2String& data,
                                  MC2String& value,
                                  uint32 startPos ) const
{
   regex_t p;
   int c = (REG_EXTENDED);
   MC2String format( name );
   format.append( " *= *\"([^\"]*)\"" );
   int res = regcomp( &p, format.c_str(), c );
   regmatch_t matches[ 2 ];
   if ( res == 0 ) {
      res = regexec( &p, data.c_str(), 2, matches, 0 );
      if ( res == 0 && matches[ 1 ].rm_so != -1 ) {
         mc2dbg8 << "matches[ 1 ].rm_so " << matches[ 1 ].rm_so 
                 << "matches[ 1 ].rm_eo " << matches[ 1 ].rm_eo << endl;
         value = data.substr( matches[ 1 ].rm_so, 
                              matches[ 1 ].rm_eo - matches[ 1 ].rm_so );
      }
      regfree( &p );
   } else {
      const uint32 errorBuffSize = 256;
      char errorBuff[ errorBuffSize ];
      regerror( res, &p, errorBuff, errorBuffSize );
      mc2log << warn << "getAttribute regcomp failed error: " << res 
             << " " << MC2CITE( errorBuff ) << endl;
   }
   mc2dbg8 << "format " << format << " data " << data 
           << " res " << res << " value " << value << endl;

   return res;
}


int
ParserExternalAuth::externalHWKeyAuth( uint32 IP,
                                       const handleHttpHeaderRequestParam& p,
                                       MC2String& userID, 
                                       const MC2String& keyType )
{
   int res = -1;

   // HWID
   const UserLicenceKey* hwKey = p.rd.getLicenceKeyType( keyType );
   if ( hwKey != NULL ) {
      MC2String licenceKey;
      m_thread->getUserHandler()->makeLicenceKeyStr( licenceKey, *hwKey );
      userID = licenceKey;
      res = 0;
   } else {
      mc2log << warn << "handle" << p.peh->reqName << "Request "
             << "externalHWIDAuth No hw id from client." << endl;
      res = -3;
   }

   return res;
}

int
ParserExternalAuth::externalDevIDAuth( uint32 IP,
                                       const handleHttpHeaderRequestParam& p,
                                       MC2String& userID ) {
   return externalHWKeyAuth( IP, p, userID, 
                             ParserUserHandler::iPhoneDevIDType );
}


bool
ParserExternalAuth::ipmatches( uint32 IP, MC2String& ipStr, 
                               const handleHttpHeaderRequestParam& p )
{
   bool ipmatch = false;
   ipStr = NetUtility::ip2str( IP );
   if ( p.peh->ips.size() == 0 ) {
      // No check needed 
      ipmatch = true;
   } else {
      mc2dbg2 << "handle" << p.peh->reqName << "Request peer IP " 
              << ipStr << endl;
   }
   
   for ( uint32 i = 0 ; i < p.peh->ips.size() ; ++i ) {
      MC2String ipStrb( ipStr );
      uint32 removes = 4 - MIN( p.peh->ips[ i ].second, 4 );
      for ( uint32 r = 0 ; r < removes ; ++r ) {
         MC2String::size_type findPos = ipStrb.rfind( '.' );
         if ( findPos != MC2String::npos ) {
            ipStrb.erase( findPos );
         }
      }
      mc2dbg4 << "Testing ip " << ipStrb << " with " 
              << p.peh->ips[ i ].first
              << " at " << p.peh->ips[ i ].second << " bytes " << endl;
      if ( ipStrb == p.peh->ips[ i ].first ) {
         // Match
         ipmatch = true;
         break;
      }
   }

   return ipmatch;
}


const MC2String*
ParserExternalAuth::getUserHeader( const HttpInterfaceRequest* hreq,
                                   const ParserExternalAuthHttpSettings* peh,
                                   MC2String& userHeaderStr ) const
{
   const MC2String* userHeader = NULL;

   if ( hreq != NULL ) {
      uint32 userHeaderIndex = 0;
      const MC2String* userHeaderRes = NULL;
      MC2String userHeaderStrRes;
      for ( uint32 i = 0 ; i < peh->userHeader.size() ; ++i ) {
         userHeaderRes = hreq->getRequestHeader()->getHeaderValue( 
            peh->userHeader[ i ] );
         if ( userHeaderRes != NULL && 
              !peh->userHeaderListPartStart.empty() &&
              !peh->userHeaderListPartStart[ i ].empty() &&
              !peh->userHeaderListPartEnd.empty() && 
              !peh->userHeaderListPartEnd[ i ].empty() )
         {
            // Get the part
            if ( stringPart( *userHeaderRes, 
                             peh->userHeaderListPartStart[ i ],
                             peh->userHeaderListPartEnd[ i ],
                             userHeaderStrRes ) ) {
               userHeaderRes = &userHeaderStrRes;
            } else {
               mc2log << warn << "handle" << peh->reqName 
                      << "Request not ok string part in header "
                      << *userHeaderRes << endl;
               userHeaderRes = NULL;
            }
         } // End if ListPart to extract from userHeaderRes

         // If to add a prefix
         if ( userHeaderRes != NULL && !userHeaderRes->empty() &&
              !peh->userHeaderListPrefix.empty() &&
              !peh->userHeaderListPrefix[ i ].empty() ) {
            // Add the prefix
            userHeaderStrRes = peh->userHeaderListPrefix[ i ] + *userHeaderRes;
            userHeaderRes = &userHeaderStrRes;
         }

         // If empty then don't use this header it is not usefull
         if ( userHeaderRes != NULL && userHeaderRes->empty() ) {
            userHeaderRes = NULL;
            mc2log << warn << "handle" << peh->reqName 
                   << "Request header " << peh->userHeader[ i ] 
                   << " is empty, skipping it." << endl;
         }

         if ( userHeaderRes != NULL ) {
            if ( userHeader == NULL ) {
               if ( userHeaderStrRes.empty() ) {
                  userHeader = userHeaderRes;
               } else {
                  userHeaderStr = userHeaderStrRes;
                  userHeader = &userHeaderStr;
               }
               userHeaderIndex = i;
            } else {
               // Check if this and the other differs
               if ( *userHeaderRes != *userHeader ) {
                  mc2dbg2 << warn << "handle" << peh->reqName << "Request "
                         << "userHeaders differ " << peh->userHeader[ 
                            userHeaderIndex ] << ": " << *userHeader 
                         << " != " << peh->userHeader[ i ] << ": " 
                         << *userHeaderRes << endl;
               }
            }
         }
      } // End for all peh->userHeader(s)

   } // End if hreg is not null

   return userHeader;
}


bool 
ParserExternalAuth::externalUserName( const MC2String& str ) const {
   return ( isHttpUserName( str ) );
}


bool
ParserExternalAuth::isExternalUser( const UserItem* userItem ) const {
   return isHttpHeaderRequestUser( userItem );
}


bool
ParserExternalAuth::isHttpUserName( const MC2String& str ) const {
   // Identified by userloginprefix + "-"
   for ( HttpSettings::const_iterator it = m_httpSettings.begin() ; 
         it != m_httpSettings.end() ; ++it )
   {
      MC2String idStr( (*it)->userloginprefix + '-' );
      if ( /*StringUtility::*/strncmp( str.c_str(), idStr.c_str(),
                                       idStr.size() ) == 0 ) 
      { 
         return true;
      }
   } // End for all httpSettings

   return false;
}


const ParserExternalAuthHttpSettings* 
ParserExternalAuth::getHttpSetting( const MC2String& type ) const {
   m_findSettings->settingId = type;
   HttpSettings::const_iterator findIt = m_httpSettings.find( 
      m_findSettings );
   if ( findIt != m_httpSettings.end() ) {
      return *findIt;
   } else {
      return NULL;
   }
}


const ParserExternalAuthHttpSettings* 
ParserExternalAuth::getNavHttpSetting( const MC2String& type ) const {
   for ( HttpSettings::const_iterator it = m_httpSettings.begin() ; 
         it != m_httpSettings.end() ; ++it )
   {
      char* s = StringUtility::strstr( type.c_str(),
                                       (*it)->clientType.c_str() );
      if ( s != NULL && (!(*it)->clientTypeEndsStr || 
                         strcmp( s, (*it)->clientType.c_str() ) == 0 ) ) {
         return *it;
      }
   }
   return NULL;
}


bool
ParserExternalAuth::checkUserNamePrefix( const MC2String& prefix, 
                                         const MC2String& logonID )
{
   bool ok = false;

   const uint32 prefSize = prefix.size() + 1;
   if ( strncmp( logonID.c_str(), (prefix + "-").c_str(), 
                 prefSize ) == 0 &&
        logonID.size() > prefSize &&
        StringUtility::onlyDigitsInString( logonID.c_str() + prefSize ) )
   {
      ok = true;
   }

   return ok;
}


bool
ParserExternalAuth::checkUserNamePrefix( const MC2String& prefix, 
                                         const UserItem* userItem )
{
   bool ok = false;
   if ( userItem != NULL ) {
      ok = checkUserNamePrefix( prefix, 
                                userItem->getUser()->getLogonID() );
   }
   return ok;
}

bool
ParserExternalAuth::getPopup( 
   const UserUser* user, const InterfaceRequestData& rd,
   MC2String& popOnce, MC2String& popMessage, MC2String& popURL,
   PopupEnums::popup_url_t& popURLType ) const
{
   bool res = false;

   for ( popupDataCont::const_iterator it = m_popupDatas.begin() ;
         it != m_popupDatas.end() ; ++it ) {
      if ( strstr( rd.clientSetting->getClientType(), 
                   it->second.clientTypeEnd.c_str() ) != NULL ) {
         byte userWFST = m_thread->getSubscriptionTypeForUser( 
            user, UserEnums::UR_WF, true, rd.urmask.level() );
         byte highestEver = m_thread->getSubscriptionTypeForUser( 
            user, UserEnums::UR_WF, false, rd.urmask.level() );
         mc2dbg << "userWFST " << int(userWFST) << " highestEver "
                << int(highestEver) << endl;
         if ( userWFST == MAX_BYTE ) { 
            // Expired
            res = true;
            popMessage += StringTable::getString( 
               it->second.expired, rd.clientLang );
            popURLType = PopupEnums::popup_yes_no;
            popURL = it->second.popURL;
         } else if ( highestEver == WFSubscriptionConstants::TRIAL ) {
            // Still in trial
            res = true;
            uint32 now = TimeUtility::getRealTime();
            if ( m_thread->getUserHandler()->getOldestRightTime( user ) +
                 5*60/*5m*/ > now ) 
            {
               // First time
               popOnce = StringTable::getString( 
                  it->second.welcome, rd.clientLang );
            }
         
            popMessage = StringTable::getString( 
               it->second.trialMode, rd.clientLang );
            popMessage.append( " " );
            // The end date
            uint32 endTime = m_thread->getUserHandler()->getEndTimeForUser(
               user, rd.clientSetting, rd.urmask.level() );
            MC2String dateStr = LocaleUtility::makeDate( 
               endTime, rd.clientLang );
            popMessage.append( dateStr );
            popMessage.append( ". " );
            popMessage += StringTable::getString( 
               it->second.trialMode2, rd.clientLang );

            popURL = it->second.popURL;
            if ( popURL.empty() ) {
               popURLType = PopupEnums::popup_no_type;
            } else {
               popURLType = PopupEnums::popup_yes_no;
            }
         }
         break; // Found a match
      } // If clientTypeEnd matches popupData
   } // For all popupDatas

   return res;
}


bool
ParserExternalAuth::isHttpHeaderRequestUser( 
   const UserItem* userItem,
   const ParserExternalAuthHttpSettings** peh ) const
{
   const char* str = userItem->getUser()->getLogonID();
   // Identified by user-name
   // name '-' foreign-id
   char* findChar = StringUtility::strchr( str, '-' );
   if ( findChar != NULL ) {
      const ParserExternalAuthHttpSettings* p = getHttpSetting( 
         MC2String( str, findChar - str ) );
      if ( p != NULL ) {
         if ( peh != NULL ) {
            *peh = p;
         }
         return true;
      }
   }

   // Identified by UserIDKey
   const UserUser* user = userItem->getUser();
   for ( HttpSettings::const_iterator it = m_httpSettings.begin() ; 
         it != m_httpSettings.end() ; ++it )
   {
      if ( (*it)->idAsKey ) {
         MC2String idStr( "-" + (*it)->userloginprefix );
         for ( uint32 i = 0 ; i < user->getNbrOfType( 
                  UserConstants::TYPE_ID_KEY ) ; i++ )
         {
            UserIDKey* id = static_cast< UserIDKey* > (
               user->getElementOfType( i, 
                                       UserConstants::TYPE_ID_KEY ) );
            if ( id->getIDKey().size() >= idStr.size() && 
                 id->getIDKey().find( idStr, id->getIDKey().size() - 
                                      idStr.size() ) != MC2String::npos ) 
            {
               if ( peh != NULL ) {
                  *peh = *it;
               }
               return true;
            }
         } // End for all idkeys
      } // End if idAsKey
   } // End for all httpSettings

   return false;
}


int
ParserExternalAuth::handleHttpHeaderRequest( 
   InterfaceRequest* ireq, UserItem* user, const MC2String& extType,
   InterfaceRequestData& rd,
   const MC2String& activationCode,
   int& status, UserItem*& setUserItem, 
   const MC2String& clientUserID, const MC2String& currentID,
   const MC2String& newID )
{
   const ParserExternalAuthHttpSettings* peh = NULL;
   int res = 0;

   // Get the external auth if defined.
   const ParserExternalAuthHttpSettings* p = getHttpSetting( extType );
   if ( p != NULL ) {
      peh = p;
   } else {
      mc2log << error << "handleHttpHeaderRequest userItem Unknown "
             << "extType " << extType << endl;
      res = -1;
   }

   // If external auth is not found and we got a user, try by using that.
   if ( peh == NULL && user != NULL ) {
      if ( isHttpHeaderRequestUser( user, &peh ) ) {
         res = 0;
      } else {
         mc2log << error << "handleHttpHeaderRequest userItem NOT "
                << "HttpHeaderRequestUser. User: " 
                << user->getUser()->getLogonID() << "(" << user->getUIN()
                << ")" << endl;
         res = -1;
      }
   }

   if ( res == 0 ) {
      ParserExternalAuthHttpSettings s( *peh );
      s.lang = rd.clientLang;
      res = handleHttpHeaderRequest( 
         handleHttpHeaderRequestParam( 
            ireq, &s, rd,
            activationCode, clientUserID, currentID, newID, user, 
            false/*No forced recheck external auth function here yet*/ ),
         status, setUserItem );
   }

   return res;
}



#define recentIDKeyAndTime( caseA, caseB ) \
         for ( UserUser::userElVect_t::iterator it = els.first ; \
               it != els.second ; ++it )\
         {\
            UserIDKey* el = static_cast< UserIDKey* > ( *it );\
            if ( el->getIDType() == UserIDKey::hardware_id_and_time ) {\
               uint32 time = 0;\
               MC2String key;\
               if ( !m_thread->getUserHandler()->getHardwareIdAndTime(\
                       el, time, key ) )\
               {\
                  mc2log << error << "handle" << p.peh->reqName << "Request"\
                         << " Bad IDKey, id: " << el->getID() << " key "\
                         << el->getIDKey() << endl;\
               }\
               if ( /*StringUtility::*/strncmp( \
                       el->getIDKey().c_str(), p.currentID.c_str(), \
                       p.currentID.size() ) == 0 )\
               {\
                  /* Same Hw.id.*/\
                  caseA;\
               } else {\
                  /* Other Hw.id.*/\
                  caseB;\
               }\
            }\
         }


int
ParserExternalAuth::handleHttpHeaderRequest( 
   const handleHttpHeaderRequestParam& pin,
   int& status, UserItem*& setUserItem )
{
   handleHttpHeaderRequestParam p( pin );
   int res = 0;

   setUserItem = NULL;
   uint32 now = TimeUtility::getRealTime();
   bool mayAddTime = true; // This and p.peh->addTime must be true
   bool accessNow = false;
   bool hasCheckedRecently = false;
   bool taintedUser = false;
   bool setCheckTime = false;
   UserItem* userItem = NULL;
   uint32 IP = m_thread->getPeerIP();
   const MC2String* userHeader = NULL;
   MC2String userHeaderStr;
   bool ipmatch = false;
   MC2String ipStr;


   //      Method to do pre auth stuff. Like merging three integrations
   //      into one.
   //      modify peh/p (set to different?) 
   //      modify old users? 
   if ( p.peh->preAuthFunc ) {
      int eres = (this->*(p.peh->preAuthFunc))( IP, p, userHeader, 
                                                userHeaderStr, userItem, 
                                                setUserItem, setCheckTime );
      if ( eres == 0 ) {
         // Ok
      } else if ( eres == -1 || eres == -2 ) {
         res = -1;
         status = eres;
      } else if ( eres == -3 ) {
         res = -1;
         status = -4;
      } else if ( eres == -4 ) {
         // Ok, but may not add time
         mayAddTime = false;
      }

      mc2dbg2 << "preAuthFunc eres " << eres << " userHeader " 
             << (userHeader != NULL ? *userHeader : "(null)") << endl;

      if ( userHeader != NULL && (eres == 0 || eres == -4) ) {
         ipmatch = true; // Have checked user is ok, right IP not needed
      }
   }


   // Http request with user id header
   const UserItem* clientUser = p.clientUser;

   mc2dbg4 << "userHeader is " << userHeader << " clientUser is " << clientUser
           << endl;

   // Check for peer in ips
   if ( res == 0 && userHeader == NULL ) {
      ipmatch = ipmatches( IP, ipStr, p );
   }
  
   HttpInterfaceRequest* hreq = dynamic_cast< HttpInterfaceRequest* > (
      p.ireq );

   if ( res == 0 && hreq != NULL && userHeader == NULL ) { 
      // If it is a http request and ok
      if ( p.peh->httpHeaderFunction != NULL ) {
         // Call the httpHeaderFunction
         (this->*p.peh->httpHeaderFunction)( p, IP, hreq );
      }
      // Get the user header
      userHeader = getUserHeader( hreq, p.peh, userHeaderStr );
      // Verify the user header
      if ( userHeader != NULL && p.peh->verifyIDFunction != NULL ) {
         int ret = (this->*p.peh->verifyIDFunction)( p,
                                                     IP, 
                                                     userHeader, 
                                                     userHeaderStr );
         if ( ret != 0 ) {
            res = -1;
            status = ret;
         }
      }
      // Add it to headers so it can be sent to services later
      if ( userHeader != NULL && !p.peh->userHeaderForServices.empty() &&
           hreq->getRequestHeader()->getHeaderValue( 
              p.peh->userHeaderForServices ) == NULL ) {
         hreq->getRequestHeader()->addHeaderLine( 
            p.peh->userHeaderForServices, *userHeader );
      }
      // Encrypt it if so configured
      addEncryptionToUserHeader( userHeader, userHeaderStr, p.peh );
   } // End if is http request


   // Check client supplied user
   // clientUser
   if ( res == 0 && clientUser != NULL ) {
      if ( p.peh->idAsKey ) {
         // Find idkey of type account in user.
         const UserIDKey* idKey = NULL;
         UserUser::constUserElRange_t els = clientUser->getUser()
            ->getElementRange( UserConstants::TYPE_ID_KEY );
         for ( UserUser::userElVect_t::const_iterator it = els.first ; 
               it != els.second ; ++it )
         {
            const UserIDKey* el = static_cast< const UserIDKey* > ( *it );
            if ( el->getIDType() == UserIDKey::account_id_key &&
                 /*StringUtility::*/strncmp( 
                    el->getIDKey().c_str(), 
                    (p.peh->userloginprefix + "-").c_str(),
                    p.peh->userloginprefix.size() + 1 ) == 0 )
            {
               idKey = el;
               break;
            }
         }

         if ( idKey ) {
            // Has id key of right type (but perhaps not right id)
            
         } else {
            // We don't use this
            clientUser = NULL;
         }
      } else {
         // Check logonID
         if ( /*StringUtility::*/strncmp( 
                 clientUser->getUser()->getLogonID(), 
                 (p.peh->userloginprefix + "-").c_str(),
                 p.peh->userloginprefix.size() + 1 ) == 0 )
         {
            // Has right start of logonID
            
         } else {
            // We don't use this
            clientUser = NULL;
         }
      }

      if ( clientUser != NULL ) {
         checkedRecently( clientUser, p, now, accessNow, hasCheckedRecently );
      }
   } // End if res == 0 and clientUser != NULL


   // Other way to get userHeader (user id) here
   if ( res == 0 && userHeader == NULL && p.peh->extAuthFunc ) {
      // Use p.currentID to check only every (setting) time.
      if ( p.clientUserID.empty() || !hasCheckedRecently || 
           (clientUser && !accessNow && p.peh->addTime) ) 
      {
         int eres = (this->*(p.peh->extAuthFunc))( IP, p, userHeaderStr );
         if ( eres == 0 ) {
            // Ok
         } else if ( eres == -1 || eres == -2 ) {
            res = -1;
            status = eres;
         } else if ( eres == -3 ) {
            res = -1;
            status = -4;
         } else if ( eres == -4 ) {
            // Ok, but may not add time
            mayAddTime = false;
         }

         if ( eres == 0 || eres == -4 ) {
            ipmatch = true; // Have checked user is ok, right IP not needed
            userHeader = &userHeaderStr;
            setCheckTime = true;
         }
      } else {
         // Hmm, use p.clientUserID (without prefix and dash)
         if ( p.peh->idAsKey ) {
            // In reality clientUser can't be NULL here (then clientUserID
            // is empty() too) but...
            if ( clientUser != NULL ) {
               const UserIDKey* idKey = NULL;
               UserUser::constUserElRange_t els = clientUser->getUser()
                  ->getElementRange( UserConstants::TYPE_ID_KEY );
               for ( UserUser::userElVect_t::const_iterator it = els.first ; 
                     it != els.second ; ++it )
               {
                  const UserIDKey* el = static_cast<const UserIDKey*> ( *it );
                  if ( el->getIDType() == UserIDKey::account_id_key ) {
                     idKey = el;
                     break;
                  }
               }

               if ( idKey ) {
                  // use this
                  userHeaderStr = idKey->getIDKey().substr( 
                     p.peh->userloginprefix.size() + 1 );
                  userHeader = &userHeaderStr;
               } else {
                  // We don't use this
                  clientUser = NULL;
               }
            }
         } else {
            userHeaderStr = p.clientUserID.substr( 
               p.peh->userloginprefix.size() + 1 );
            userHeader = &userHeaderStr;
         }
      }
   } 


   // Check if clientUser can be used without a userHeader
   if ( res == 0 && userHeader == NULL && clientUser != NULL ) {
      // We don't use this, needs to have userHeader to check against
      clientUser = NULL;
   }

   // Check clientUser's id
   if ( res == 0 && userHeader != NULL && clientUser != NULL ) {
      // Check id against clientUser
      if ( p.peh->idAsKey ) {
         // Check idkey
         if ( !m_thread->getUserHandler()->getIDKey( 
                 clientUser->getUser(), UserIDKey::account_id_key, 
                 p.peh->userloginprefix + "-" + *userHeader ) )
         {
            // We don't use this
            clientUser = NULL;
         } else {
            ipmatch = true; // Have checked user is ok, right IP not needed
         }
      } else {
         // Check logonID
         if ( /*StringUtility::*/strcmp( 
                 clientUser->getUser()->getLogonID(), 
                 (p.peh->userloginprefix + "-" + *userHeader).c_str() ) != 0)
         {
            // We don't use this
            clientUser = NULL;
         } else {
            ipmatch = true; // Have checked user is ok, right IP not needed
         }
      }
   }


   // Check some reasons for not accepted
   if ( res == 0 &&
        ( ! ipmatch ||
          ( userHeader == NULL && clientUser == NULL && userItem == NULL ) ||
          ( ! p.peh->statusHeader.empty() &&
            ! p.peh->noStatusHeaderOK &&
            hreq->getRequestHeader()->getHeaderValue( p.peh->statusHeader )
            == NULL ) ||
          p.peh->blockedDate < now ) )
   {
      // if not then not accepted and we should send Unauthorized
      
      // Not ok request
      res = -1;
      status = -4; // UNAUTHORIZED
      mc2log << warn << "handle" << p.peh->reqName << "Request UNAUTHORIZED!"
             << " IP " << ipStr << (ipmatch ? " matches" : "NOT MATCHING");
      if ( hreq != NULL ) {
         mc2log << " Http req";
         for ( uint32 i = 0 ; i < p.peh->userHeader.size() ; ++i ) {
            if ( hreq->getRequestHeader()->getHeaderValue( 
                    p.peh->userHeader[ i ] ) != NULL )
            {
               mc2log << " Has " << p.peh->userHeader[ i ] << " " 
                      << *hreq->getRequestHeader()->
                  getHeaderValue( p.peh->userHeader[ i ] );
            } else {
               mc2log << " Hasn't " << p.peh->userHeader[ i ];
            }
         }
         if ( p.peh->statusHeader.empty() ) {
            // Not care about the statusHeader
         } else if ( hreq->getRequestHeader()
                     ->getHeaderValue( p.peh->statusHeader ) != NULL )
         {
            mc2log << " Has " << p.peh->statusHeader << " " 
                   << *hreq->getRequestHeader()->getHeaderValue( 
                      p.peh->statusHeader );
         } else {
            mc2log << " Hasn't " << p.peh->statusHeader;
         }
      } else {
         mc2log << " Not Http req";
      }
      if ( p.peh->blockedDate < now ) {
         mc2log << " Blocked client type";
      }
      if ( clientUser != NULL ) {
         mc2log << " Has clientUser " 
                << clientUser->getUser()->getLogonID()
                << "(" << clientUser->getUIN() << ")";
      }
      mc2log << endl;
   }


   // Check status (if needed)
   const MC2String* statusHeaderValue = hreq ? hreq->getRequestHeader()
      ->getHeaderValue( p.peh->statusHeader ) : NULL;
   if ( res == 0 && !p.peh->statusHeader.empty() ) {
      if ( p.peh->statusHeader.empty() || 
           ((statusHeaderValue != NULL && 
             *statusHeaderValue == p.peh->okStatus) ||
            (statusHeaderValue == NULL && p.peh->noStatusHeaderOK)) ) 
      {
         // Ok!
      } else {
         mc2log << warn << "handle" << p.peh->reqName << "Request status "
                << "not ok \"" 
                << (statusHeaderValue ? statusHeaderValue->c_str() : NULL )
                << "\" sending expired. ";
         for ( uint32 i = 0 ; i < p.peh->userHeader.size() ; ++i ) {
            if ( hreq->getRequestHeader()->getHeaderValue( 
                    p.peh->userHeader[ i ] ) != NULL )
            {
               mc2log << " Has " << p.peh->userHeader[ i ] << " " 
                      << *hreq->getRequestHeader()->
                  getHeaderValue( p.peh->userHeader[ i ] );
            } else {
               mc2log << " Hasn't " << p.peh->userHeader[ i ];
            }
         }
         if ( clientUser != NULL ) {
            mc2log << " Has clientUser " 
                   << clientUser->getUser()->getLogonID()
                   << "(" << clientUser->getUIN() << ")";
         }
         mc2log << endl;
         status = -3; // EXPIRED
         res = -1;
      }      
   }


   // If userHeader and no clientUser then get user here
   if ( res == 0 && userItem == NULL &&
        (userHeader != NULL && clientUser == NULL) )
   {
      // UserID
      MC2String userID( p.peh->userloginprefix );
      userID.append( "-" );
      userID.append( *userHeader );

      // Find and create if no such user
      // Try to get user
      uint32 startTime = TimeUtility::getCurrentTime();
      bool commOk = false;
      if ( p.peh->idAsKey ) {
         // ID key
         uint32 nbrUsers = 0;
         commOk = m_thread->getUserHandler()->getUserFromIDKeyValue(
            userID, userItem, nbrUsers, true );
         if ( nbrUsers > 1 ) {
            mc2log << warn << "handle" << p.peh->reqName << " More "
                   << "than one user with id-key \"" << userID
                   << "\" " << "Sending error" << endl;
            res = -1;
            status = -1; // NOT_OK
         }
      } else { 
         // UserID
         commOk = m_thread->getUser( 
            userID.c_str(), userItem, true );
      }
      uint32 endTime = TimeUtility::getCurrentTime();

      if ( res != 0 ) {
         // Error set 
      } else if ( !commOk ) {
         mc2log << warn << "handleExternalAuth: error getting "
                << p.peh->reqName << " user "
                << "from " << MC2CITE( userID )
                << " error: ";
         if ( endTime - startTime > 3000 ) {
            status = -2; // TIMEOUT
            mc2log << "Timeout";
         } else {
            status = -1; // NOT_OK
            mc2log << "Error";
         }
         mc2log << endl;
         res = -1;
      } else if ( userItem == NULL ) {
         // Create user with gold access
         MC2String passwd;
         // Extra comment with licence
         MC2String extraComment;
         // Licence to add to user
         UserLicenceKey* userKey = NULL;
         // Extra UserElements to add to user
         vector< UserElement* > extraElements;
         if ( !p.rd.hwKeys.empty() ) {
            bool addKeyComment = false;
            if ( p.peh->addLicence ) {
               MC2String key( p.rd.hwKey.getKeyType() + ":" +
                              p.rd.hwKey.getLicenceKeyStr() );
               key.append( "-" );
               key.append( p.peh->userloginprefix );
               if ( p.peh->addLicenceAsID ) {
                  UserIDKey* userIDKey = new UserIDKey( 
                     0, key, UserIDKey::hardware_id_key );
                  extraElements.push_back( userIDKey );
                  addKeyComment = true;
               } else {
                  userKey = new UserLicenceKey( 0 );
                  userKey->setLicence( p.rd.hwKey.getLicenceKeyStr() + 
                                       "-" + p.peh->userloginprefix );
                  userKey->setKeyType( p.rd.hwKey.getKeyType() );
               }
            } else {
               addKeyComment = true;
            }
            if ( addKeyComment ) {
               extraComment.append( "BK: " );
               extraComment.append( p.rd.hwKey.getKeyType() + ":" +
                                    p.rd.hwKey.getLicenceKeyStr() );
            }
         }

         if ( p.peh->idAsKey ) {
            UserIDKey* userIDKey = new UserIDKey( 
               0, userID, UserIDKey::account_id_key );
            extraElements.push_back( userIDKey );
            if ( !extraComment.empty() ) {
               extraComment.append( " " );
            }
            extraComment.append( p.peh->reqName );
            extraComment.append( "-ID: " );
            extraComment.append( userID );
         }

         uint32 now = TimeUtility::getRealTime();
         uint32 createStartTime = now - 24*60*60;
         uint32 createEndTime = ParserUserHandler::addTime( 
            p.peh->createYears, p.peh->createMonths, p.peh->createDays,
            p.peh->createExplicitTime, now );

         MC2String logonID( userID );
         for ( uint32 i = 0 ; i < p.peh->logonIDHeader.size() ; ++i ) {
            const MC2String* tuserHeader = hreq->getRequestHeader()->
               getHeaderValue( p.peh->logonIDHeader[ i ] );
            if ( tuserHeader != NULL ) {
               bool found = true;
               if ( !p.peh->logonIDHeaderListPartStart.empty() &&
                    !p.peh->logonIDHeaderListPartStart[ i ].empty() &&
                    !p.peh->logonIDHeaderListPartEnd.empty() && 
                    !p.peh->logonIDHeaderListPartEnd[ i ].empty() )
               {
                  if ( !stringPart( *tuserHeader, 
                                    p.peh->logonIDHeaderListPartStart[ i ],
                                    p.peh->logonIDHeaderListPartEnd[ i ],
                                    logonID ) ) {
                     mc2log << warn << "handle" << p.peh->reqName 
                            << "Request not ok string part in logonID "
                            << "header " << *tuserHeader << endl;
                     found = false;
                  }
               } else {
                  logonID = *tuserHeader;
               }
               if ( found ) {
                  // Add prefix
                  logonID.insert( 0, "-" );
                  logonID.insert( 0, p.peh->userloginprefix );
                  // Only use the first header
                  break;
               }
            }
         }
         int cret = m_thread->getUserHandler()->createWayfinderUser( 
            passwd, userKey, userItem, createStartTime, 
            createEndTime, p.peh->createRegionID, p.peh->lang, 
            p.rd.clientType.c_str(), p.rd.clientTypeOptions.c_str(), 
            p.rd.clientV, p.activationCode, p.peh->extraTypes,
            p.peh->createLevel, p.peh->createTransactionDays, p.peh->brand,
            logonID.c_str(), 
            extraComment.empty() ? NULL : extraComment.c_str(), 
            extraElements,
            p.peh->userNamePrefix.empty() ? NULL : 
            p.peh->userNamePrefix.c_str() );

         if ( cret == 4 ) {
            // Not unique idkey, created user in a parallel request?
            // Try to find user again
            mc2log << warn << "handleHttpHeaderRequest: Not unique IDKey "
                   << "when creating " << p.peh->reqName 
                   << "-Wayfinder user " << userID << " tries to find again"
                   << endl;
            status = -1; // NOT_OK
            res = -1;
            if ( p.peh->idAsKey ) {
               // ID key
               uint32 nbrUsers = 0;
               commOk = m_thread->getUserHandler()->getUserFromIDKeyValue(
                  userID, userItem, nbrUsers, true );
               if ( nbrUsers > 1 ) {
                  mc2log << warn << "handle" << p.peh->reqName << " More "
                         << "than one user with id-key \"" << userID
                         << "\" " << " when tried to find again, "
                         << "sending error" << endl;
               } else if ( nbrUsers == 1 && userItem != NULL ) {
                  mc2log << warn << "handleHttpHeaderRequest: Found user "
                         << "created in parallel, using it." << endl;
                  // Continue with this user
                  res = 0;
                  status = 0;
               }
            }
         } else if ( cret != 0 ) {
            mc2log << warn << "handleHttpHeaderRequest: failed to "
                   << "create " << p.peh->reqName 
                   << "-Wayfinder user " << userID << endl;
            status = -1; // NOT_OK
            res = -1;
         } else {
            // userItem is set
            mc2log << info << "handleHttpHeaderRequest: created "
                   << p.peh->reqName << "-Wayfinder user with "
                   << "logonID \""
                   << userItem->getUser()->getLogonID() << "\""
                   << "(" << userItem->getUIN() << ")" << endl;
         }
         taintedUser = true;
      
      } // End if no user for the userID
      else if ( userItem != NULL ) {
         // User exists
         setUserItem = userItem;
         setUserItem->addLock(); // userItem released below
      }

   } // End if ok and has userHeader but no clientUser


   // Set userItem if has clientUser (userItem not set yet)
   if ( res == 0 && clientUser != NULL ) {
      userItem = const_cast< UserItem* > ( clientUser );
      userItem->addLock(); // clientUser released outside this method

      // User exists
      setUserItem = userItem;
      setUserItem->addLock(); // userItem released below
   }


   // setCheckTime
   if ( res == 0 && userItem != NULL && setCheckTime && 
        p.peh->extAuthFuncCheckTime > 0/*always check if 0*/ ) {
      // Update recent check time
      UserUser* user = new UserUser( *userItem->getUser() );
      UserUser::userElRange_t els = user
         ->getElementRange( UserConstants::TYPE_ID_KEY );
      recentIDKeyAndTime( 
         el->remove();,
         if ( int32(now) - int32(time) > p.peh->extAuthFuncCheckTime ) {
            // Too old to be usefull -> remove
            el->remove();
         } );

      MC2String key( p.newID );
      key.append( ":" );
      STLStringUtility::int2str( 
         int32( TimeUtility::getRealTime() ), key );
      user->addElement( 
         new UserIDKey( 0, key, 
                        UserIDKey::hardware_id_and_time ) );

      //      p.currentID token...?! Set createTime = 0 to get new token
      //      later in XMLParserThread. 
      //      This isn't really the place but it makes one less change
      //      user request and get uptodate user...
      // Set time in token
      UserToken* tok = m_thread->getTokenHandler()->getToken( 
         user, p.currentID, NULL/*No client type*/ );
      if ( tok != NULL ) {
         if ( tok->getCreateTime() < now ) {
            //      make new token
            //      Slightly bad as client has to 
            //      resend request with new token
            //      New token if this is expired set
            //      createtime to 0.
            tok->setCreateTime( 0 );
         } // Else in future and no need to change
         //   Feature used by testing houses wihtout correct operator
         //   SIM card.
      }      

      // Change user
      if ( !m_thread->changeUser( user, NULL/*changer*/ ) ) {
         mc2log << warn << "handle" << p.peh->reqName << "Request "
                << " user  " << user->getLogonID() << "(" 
                << user->getUIN() << ") Failed to update last "
                << "check time, will try again next session."
                << endl;
         // Let it pass. Will check again soon and try update
         // then.
      }

      delete user;
      taintedUser = true;
   }

   
   if ( res == 0 && userItem != NULL ) {
      // add access if needed
      if ( res == 0 && p.peh->addTime && mayAddTime ) {
         if ( !m_thread->checkAccessToService( 
                 userItem->getUser(), UserEnums::UR_WF, 
                 UserEnums::UR_GOLD, p.peh->addRegionID ) )
         {
            typedef UserEnums UE;
            UserUser* user = new UserUser( *userItem->getUser() );
            uint32 startTime = now;
            uint32 endTime = startTime;

            // If calender time then startTime back to 23:00

            endTime = ParserUserHandler::addTime( 
               p.peh->addYears, p.peh->addMonths, p.peh->addDays,
               MAX_UINT32/*explicitDate*/, startTime );

            // If calender time then add 2h (to 01:00)

            for ( uint32 i = 0 ; i < p.peh->addTypes.size() ; ++i ) {
               uint32 setEndTime = endTime;
               if ( p.peh->addTypes[ i ].second.isSet ) {
                  setEndTime = ParserUserHandler::addTime( 
                     p.peh->addTypes[ i ].second.year,
                     p.peh->addTypes[ i ].second.month,
                     p.peh->addTypes[ i ].second.day, 
                     p.peh->addTypes[ i ].second.explic,
                     endTime );
               }
               user->addElement( 
                  new UserRight( 0, now, p.peh->addTypes[ i ].first, 
                                 p.peh->addRegionID, 
                                 startTime, setEndTime,
                                 false, p.peh->addOrigin.c_str() ) );
            }

            // Change user
            uint32 cstartTime = TimeUtility::getCurrentTime();
            if ( !m_thread->changeUser( user, NULL/*changer*/ ) ) {
               res = -1;
               uint32 cendTime = TimeUtility::getCurrentTime();
               mc2log << warn << "handleHttpHeaderRequest: failed to "
                      << "add more access time to " << p.peh->reqName 
                      << " user " << user->getLogonID() << "(" 
                      << user->getUIN() << ")" << " error: ";
               if ( cendTime - cstartTime > 3000 ) {
                  status = -2; // TIMEOUT
                  mc2log << "Timeout";
               } else {
                  status = -1; // NOT_OK
                  mc2log << "Error";
               }
               mc2log << endl;
                  
            } else { // End if changeUser fails
               mc2log << info << "handleHttpHeaderRequest: added more "
                      << "access to " << p.peh->reqName << " user " 
                      << user->getLogonID() 
                      << "(" << user->getUIN() << ") " << endl;
            } // End else add more access ok

            delete user;
            taintedUser = true;
         } // End if no access now
            
      } // End if res == 0 and may add time

      // Do stuff to the authenticated user, like update information or
      // unauthenticate
      if ( res == 0 && p.peh->postAuthFunction != NULL ) {
         res = (this->*p.peh->postAuthFunction)( p, userItem, taintedUser,
                                                 status );
      }

      if ( res == 0 && taintedUser ) {
         // Get user with uptodate attributes and set in
         // session
         UserItem* rUserItem = NULL;
         if ( m_thread->getUser( userItem->getUIN(), 
                                 rUserItem, true ) ) 
         {
            if ( rUserItem != NULL ) {
               // DONE!
               if ( setUserItem != NULL ) {
                  m_thread->releaseUserItem( setUserItem );
               }
               setUserItem = rUserItem;
            } else {
               // We should never get here, we have valid UIN, 
               // but jic.
               status = -1; // NOT_OK
               res = -1;
               mc2log << warn << "handleHttpHeaderRequest: not able to "
                      << "get " << p.peh->reqName << " updated user " 
                      << userItem->getUser()->getLogonID() 
                      << "(" << userItem->getUIN() << ") in "
                      << "db but I just had it." << endl;
            }
         } else {
            status = -2; // TIMEOUT
            res = -1;
            mc2log << warn << "handleHttpHeaderRequest: failed to get "
                   << p.peh->reqName << " updated user " 
                   << userItem->getUser()->getLogonID() 
                   << "(" << userItem->getUIN() << ")" << endl;
         }
      } // End if res == 0 and taintedUser


      // Added lock above and is not released if error
      if ( res != 0 && setUserItem != NULL ) {
         m_thread->releaseUserItem( setUserItem );
         setUserItem = NULL;
      }


      if ( res == 0 && !p.peh->statusHeader.empty() &&
           statusHeaderValue == NULL && p.peh->noStatusHeaderOK )
      {
         mc2log << warn << "handle" << p.peh->reqName << "Request status "
                << "not present but that is ok.";
         for ( uint32 i = 0 ; i < p.peh->userHeader.size() ; ++i ) {
            if ( hreq->getRequestHeader()->getHeaderValue( 
                    p.peh->userHeader[ i ] ) != NULL )
            {
               mc2log << " Has " << p.peh->userHeader[ i ] << " " 
                      << *hreq->getRequestHeader()->
                  getHeaderValue( p.peh->userHeader[ i ] );
            } else {
               mc2log << " Hasn't " << p.peh->userHeader[ i ];
            }
         }
         mc2log << endl;
      }

   } else {
      // Perhaps special "test" user in p.clientUserID? Five zeros needed!
      if ( p.clientUserID.find( "00000" ) != MC2String::npos ) {
         // Get user
         uint32 startTime = TimeUtility::getCurrentTime();
         UserItem* tuserItem = NULL;
         bool commOk = m_thread->getUser( 
            p.clientUserID.c_str(), tuserItem, true );
         uint32 endTime = TimeUtility::getCurrentTime();
         if ( commOk && tuserItem != NULL ) {
            // Check if UserIDKey::hardware_id_and_time in future
            bool hasIdKeyFuture = false;
            UserUser::constUserElRange_t els = tuserItem->getUser()
               ->getElementRange( UserConstants::TYPE_ID_KEY );
            
            for ( UserUser::userElVect_t::const_iterator it = els.first ; 
                  it != els.second ; ++it )
            {
               uint32 time = 0;
               MC2String id;
               if ( ParserUserHandler::getHardwareIdAndTime( 
                       static_cast< UserIDKey* >( *it ), time, id ) &&
                    time > now )
               {
                  hasIdKeyFuture = true;
                  break;
               }
            }
            
            // Then use it
            if ( hasIdKeyFuture ) {
               mc2log << info << "handle" << p.peh->reqName << "Request "
                      << "Using test user " << p.clientUserID << "(" 
                      << tuserItem->getUIN() << ")" << endl;
               setUserItem = tuserItem;
               setUserItem->addLock(); // tuserItem released below
               // Unset prevoius error, we have a user now
               res = 0;
               status = 0;
            }
            
         } else {
            mc2log << warn << "handle" << p.peh->reqName << " Failed to get "
                   << "or no such test user \"" << p.clientUserID << "\""
                   << " Error: ";
            if ( endTime - startTime > 3000 ) {
               status = -2; // TIMEOUT
               mc2log << "Timeout";
            } else {
               status = -1; // NOT_OK
               mc2log << "Error";
            }
            mc2log << endl;
            res = -1;
         }

         m_thread->releaseUserItem( tuserItem );
      }

   } // End else if not res == 0


   // Check if needs to update hw idkey in user
   if ( res == 0 && p.peh->keepUpdatedHWKey && userItem != NULL &&
        !p.rd.hwKey.getLicenceKeyStr().empty() ) {
      // Compare the one in user with the received one
      MC2String hwKeyStr( p.peh->userloginprefix + "-" + 
                          p.rd.hwKey.getKeyType() + ":" );
      MC2String imeiStr;
      if ( p.rd.hwKey.isIMEIKey() && p.rd.hwKey.extract15Digits( imeiStr ) ) {
         hwKeyStr += imeiStr;
      } else {
         hwKeyStr += p.rd.hwKey.getLicenceKeyStr();
      }
      if ( m_thread->getUserHandler()->getIDKey( userItem->getUser(),
                                                 UserIDKey::hardware_id_key,
                                                 hwKeyStr ) == NULL ) {
         // Not found -> add it and remove any old one
         UserUser cuser( *userItem->getUser() );
         m_thread->getUserHandler()->removeAllUseIDKeyOfType( 
            &cuser, UserIDKey::hardware_id_key );
         cuser.addElement( new UserIDKey( 0, hwKeyStr, 
                                          UserIDKey::hardware_id_key ) );
         // Save changes
         m_thread->changeUser( &cuser, NULL/*changer*/ );
      }
      
   }

   // Gotten above
   m_thread->releaseUserItem( userItem );

   return res;
}

bool
ParserExternalAuth::stringPart( 
   const MC2String& str, const MC2String& partStart,
   const MC2String& partEnd, MC2String& val ) const
{
   bool found = false;

   MC2String::size_type findPos = str.find( partStart );
   if ( findPos != MC2String::npos ) {
      MC2String::size_type lsize = partStart.size();
      MC2String::size_type endPos = str.find( partEnd, findPos + lsize );
      if ( endPos != MC2String::npos || partEnd[ 0 ] == '\n' ) {
         val = str.substr( findPos + lsize, endPos - findPos - lsize );
         found = true;
      } else {
         mc2log << warn << "stringPart no end "
                << partEnd << " for " 
                << partStart << " in string "
                << str << endl;
      }
   } else {
      mc2log << warn << "stringPart no " << partStart << " in string "
             << str << endl;
   }

   return found;
}

const UserIDKey*
ParserExternalAuth::getUserIDKey( 
   const UserUser* user, const char* str, UserIDKey::idKey_t type ) const 
{
   return getUserIDKey( const_cast<UserUser*> ( user ), str, type );
}

UserIDKey*
ParserExternalAuth::getUserIDKey( 
   UserUser* user, const char* str, UserIDKey::idKey_t type ) const 
{
   UserIDKey* idKey = NULL;
   UserUser::constUserElRange_t els = user->getElementRange( 
      UserConstants::TYPE_ID_KEY );
   const uint32 strLen = strlen( str );
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it )
   {
      UserIDKey* el = static_cast<UserIDKey*> ( *it );
      if ( el->getIDType() == type &&
           /*StringUtility::*/strncmp( el->getIDKey().c_str(), 
                                       str, strLen ) == 0 ) {
         idKey = el;
         break;
      }
   }

   return idKey;
}

void
ParserExternalAuth::checkedRecently( const UserItem* clientUser, 
                                     const handleHttpHeaderRequestParam& p,
                                     uint32 now,
                                     bool& accessNow,
                                     bool& hasCheckedRecently )
{
   accessNow = m_thread->checkAccessToService( 
      clientUser->getUser(), UserEnums::UR_WF, 
      UserEnums::UR_GOLD, p.peh->addRegionID );
   int32 highestTime = 0;
   int32 highestOtherTime = 0;
   UserUser::userElRange_t els = const_cast< UserUser* > ( 
      clientUser->getUser() )->getElementRange( UserConstants::TYPE_ID_KEY );

   // hasCheckedRecently
   recentIDKeyAndTime(
      if ( int32(time) > highestTime ) {
         highestTime = time;
      },
      if ( int32(time) > highestOtherTime ) {
         highestOtherTime = time; 
      } );
   if ( int32(now) - highestTime < p.peh->extAuthFuncCheckTime ||
        int32(now) - highestOtherTime < 30 )
   {
      hasCheckedRecently = true;
   }
}

void
ParserExternalAuth::addEncryptionToUserHeader( 
   const MC2String*& userHeader,
   MC2String& userHeaderStr,
   const ParserExternalAuthHttpSettings* peh ) const {
   if ( userHeader != NULL && peh->encryptIDFunction != NULL ) {
      userHeaderStr = (this->*peh->encryptIDFunction)( *userHeader );
      userHeader = &userHeaderStr;
   }
}

#ifdef undef_level_4
#   undef DEBUG_LEVEL_4
#endif

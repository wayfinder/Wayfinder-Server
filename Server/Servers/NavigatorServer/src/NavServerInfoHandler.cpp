/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavServerInfoHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserRight.h"
#include "UserData.h"
#include "NavUtil.h"
#include "NavParserThreadGroup.h"
#include "NavUserHelp.h"
#include "ClientSettings.h"
#include "isabBoxInterfaceRequest.h"
#include "NavClientVersionStorage.h"
#include "NavCategoriesData.h"
#include "NavRequestData.h"
#include "isabBoxSession.h"
#include "ClientVersion.h"
#include "Properties.h"
#include "UserFavorites.h"
#include "UserFavoritesRequest.h"
#include "STLStringUtility.h"
#include "ParserUserHandler.h"
#include "SearchParserHandler.h"
#include "DataBuffer.h"
#include "categoryRegionIDFromCoord.h"

NavServerInfoHandler::NavServerInfoHandler( InterfaceParserThread* thread,
                                            NavParserThreadGroup* group,
                                            NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_userHelp( userHelp )
{
   m_expectations.push_back( ParamExpectation( 4300, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4302, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4303, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4305, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4307, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4000, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 27, NParam::String ) );
}


NavServerInfoHandler::~NavServerInfoHandler() {
}

namespace {

/// Extracts coordinates from param 4308
MC2Coordinate getCoordFromParam( const NParam* param ) {
   if ( param->getLength() < 8 ) { // invalid param
      return MC2Coordinate();
   }
   else {
      return MC2Coordinate( Nav2Coordinate( param->getInt32Array( 0 ),
                                            param->getInt32Array( 1 ) ) );
   }
}

/** 
 * Extracts coordinates from a param block that contains param 4308
 * and converts to a category region id. If 4308 isn't included in
 * the block NO_REGION is returned.
 */
CategoryRegionID getCategoryRegionIDFromParams( const NParamBlock& params,
                                                MapRequester* requester ) {
   const NParam* coordParam = params.getParam( 4308 );
   if ( coordParam != NULL ) {
      MC2Coordinate coord = getCoordFromParam( coordParam );
      return categoryRegionIDFromCoord( coord, requester );
   }
   return CategoryRegionID::NO_REGION;
}

} // namespace

bool
NavServerInfoHandler::handleServerInfo( NavRequestData& rd ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // The user
   UserUser* user = rd.session->getUser()->getUser();
   // Time now 
   uint32 now = TimeUtility::getRealTime();


   // Start parameter printing
   mc2log << info << "handleServerInfo:";

   mc2log << " Lang " << StringTable::getString( 
      StringTable::getLanguageAsStringCode( rd.stringtClientLang ), 
                                            StringTable::ENGLISH );
   WFSubscriptionConstants::subscriptionsTypes userWFST = 
      m_thread->getSubscriptionTypeForUser( user, UserEnums::UR_WF );

   mc2log << endl;

   mc2log << info << "handleServerInfo:";

   // Add all crcs
   uint32 crc = 0;
   
   // TopRegionCRC
   if ( ok ) {
      int res = m_group->getTopRegionCRC( 
         m_thread, user, rd.stringtClientLang, rd.urmask, crc );
      if ( res == 0 ) {
         mc2log << " TopRegionCRC " << MC2HEX( crc );
         rd.rparams.addParam( NParam( 4300, crc ) );
      } else {
         ok = false;
         mc2log << warn << "handleServerInfo: Failed to get TopRegionCRC ";
         if ( res == 2 ) {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            mc2log << "Timeout";
         } else {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
            mc2log << "Error";
         }
         mc2log << endl;
      }
   }

   // Latest news crc
   if ( ok ) {
      uint16 daysLeft = m_thread->getUserHandler()->getNbrDaysLeftForUser( 
         user, rd.clientSetting );

      if ( m_group->getLatestNewsCRC( 
              rd.clientType.c_str(), rd.clientLang, userWFST, daysLeft, 
              rd.clientSetting->getImageExtension(), crc ) )
      {
         mc2log << " LatestNewsCRC " << MC2HEX( crc );
         rd.rparams.addParam( NParam( 4302, crc ) );
      } else {
         // No latest news
      }
   }

   // Categories crc
   if ( ok ) {
      CategoryRegionID regionID = CategoryRegionID::NO_REGION;
      if ( rd.reqVer >= 2 ) {
         regionID = getCategoryRegionIDFromParams( rd.params, m_thread );
      }

      if ( rd.clientSetting->getCategoriesCRC( 
              rd.stringtClientLang, 
              m_thread->clientUsesLatin1(), 
              regionID,
              crc ) ) 
      {
         mc2log << " CategoriesCRC " << MC2HEX( crc );
         rd.rparams.addParam( NParam( 4303, crc ) );
      }  else {
         // No Categories
      }
   }

   // Call Center crc
   if ( ok && (userWFST != WFSubscriptionConstants::TRIAL &&
               userWFST != WFSubscriptionConstants::SILVER && 
               userWFST != MAX_BYTE ) )
   {
      crc = rd.clientSetting->getCallCenterListCRC();
      mc2log << " CallCenterListCRC " << MC2HEX( crc );
      rd.rparams.addParam( NParam( 4305, crc ) );
   }

   // Days left
   if ( ok && rd.clientSetting->usesRights() ) {
      uint16 daysLeft = m_thread->getUserHandler()->getNbrDaysLeftForUser( 
         user, rd.clientSetting );
      if ( daysLeft != MAX_UINT16 ) {
         mc2log << " only " << daysLeft << " days left for account.";
         rd.rparams.addParam( NParam( 18, daysLeft ) );
      }
   }

   // Public user name
   if ( ok ) {
      mc2log << " UserName " << user->getLogonID();
      rd.rparams.addParam( NParam( 4306, user->getLogonID(), 
                                   m_thread->clientUsesLatin1() ) );
   }

   // Alt. server list.
   if ( ok ) {
      m_userHelp->addServerListParams( 
         rd.rparams, *rd.clientSetting, rd.ireq->getHttpRequest() != NULL, 
         false/*addServerm*/, true/*printCRC*/ );
   }

   // PINs CRC
   if ( ok ) {
      // Check if POSITIONING right
      if ( m_thread->checkAccessToService( user, 
                                           UserEnums::UR_POSITIONING,
                                           UserEnums::UR_NO_LEVEL ) ) 
      {
         crc = m_userHelp->getUsersPINsCRC( rd.session->getUser() );
         mc2log << " UsersPINsCRC " << MC2HEX( crc );
         rd.reply->getParamBlock().addParam( 
            NParam( 5204, crc ) );
      }
   }

   // Rights bitfield
   // Add convertion if rights reorganized in mc2.
   if ( ok ) {
      UserEnums::userRightService rightServices = 
         UserEnums::userRightService( 0 );
      typedef set<UserEnums::userRightService> ExtraServices;
      ExtraServices extraServices;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
      {
         UserRight* r = static_cast<UserRight*> ( 
            user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
         if ( !r->isDeleted() && r->checkAccessAt( now ) ) {
            if ( UserEnums::isBitRight( r->getUserRightType().service() ) )
            {
               rightServices = UserEnums::userRightService(
                  rightServices | r->getUserRightType().service() );
               if ( r->getUserRightType().service() & 
                    UserEnums::UR_ALL_TRAFFIC_MASK ) 
               {
                  // All traffic sets UR_TRAFFIC
                  rightServices = UserEnums::userRightService(
                     rightServices | UserEnums::UR_TRAFFIC );
               }
            } else {
               extraServices.insert( r->getUserRightType().service() );
            }
         }
      } // End for all rights

      // 20080228 All Earth clients should have USE_GPS
      if ( m_thread->checkIfIronClient( rd.clientSetting ) ) {
         extraServices.insert( UserEnums::UR_USE_GPS );
      }

      // 20080421 All, precisely all, should have locator.
      // 20081007 But not for access clients. Request by PM.
      if ( ! m_thread->checkIfLithiumClient( rd.clientSetting ) ) {
         rightServices = UserEnums::userRightService(
            rightServices | UserEnums::UR_POSITIONING );
      }

      NParam rightServicesParam( 21 );
      rightServicesParam.addUint32( rightServices );
      mc2log << " rightServices " << MC2HEX( rightServices );
      for ( ExtraServices::const_iterator it = extraServices.begin() ;
            it != extraServices.end() ; ++it ) {
         mc2log << ", " << MC2HEX( *it );
         rightServicesParam.addUint32( *it );
      }
      rd.rparams.addParam( rightServicesParam );
   }

   // New version
   const char* vers = rd.clientSetting->getVersion();
   
   // TODO: Locked client to version check goes here
   //       use getLockedVersion( lockedVersion (4.50.0))
   
   if ( rd.clientVSet && vers != NULL && vers[ 0 ] != '\0' ) {
      ClientVersion cv( rd.clientV[ 0 ], rd.clientV[ 1 ], 
                        rd.clientV[ 2 ] );
      
      
      if ( cv < *rd.clientSetting->getVersionObj() ) {
         // Newer version exists. Inform the client
         rd.rparams.addParam( NParam( 23, vers, 
                                      m_thread->clientUsesLatin1() ) );
         if( rd.reqVer > 3 ) {
            // Add platform upgrade id if available
            if( !rd.clientSetting->getUpgradeId().empty() ) {
               rd.rparams.addParam( NParam( 4309, rd.clientSetting->getUpgradeId(),
                                            m_thread->clientUsesLatin1() ) );
            }
            
            // Set if we should force upgrade
            rd.rparams.addParam( NParam( 4308, rd.clientSetting->getForceUpgrade() ) );
         }
         
         // Also add a good url to download from
         MC2String url( Properties::getProperty( 
                           "DOWNLOAD_NEW_CLIENT_URL",
                           "" ) );
         rd.rparams.addParam( NParam( 34, url, 
                                      m_thread->clientUsesLatin1() ) );
         
      }
   } // End if client sent version and 
   //         vers != NULL and not empty string (has clientversion)
   

   // Favorites CRC
   if ( ok && rd.reqVer > 1 ) {
      MC2String crcStr;
      STLStringUtility::uint2strAsHex( m_thread->getUserHandler()->
         getUserFavCRC( user->getUIN() ), crcStr );
      mc2log << " UserFavCRC " << crcStr;
      rd.rparams.addParam( NParam( 4903, crcStr, 
                                   m_thread->clientUsesLatin1() ) );
   }

   // UTC time
   if ( ok ) {
      mc2log << " UTC " << MC2HEX( TimeUtility::getRealTime() );
      rd.rparams.addParam( NParam( 25, TimeUtility::getRealTime() ) );
   }

   // News string
   if ( ok && rd.reqVer > 1 ) {
      ParserThreadGroup::NewsStatus news = 
         m_group->getNewsString( *m_thread, 
                                 ParserThreadGroup::
                                 NewsKey( rd.clientType,
                                          rd.clientLang ) );
      if ( news.second < 0 || news.second == 404 ) {
         // on error, ignore news string
         if ( news.second == -2 ) {
            // timeout
            mc2log << warn << "[NavServerInfoHandler] news fetch timed out." 
                   << endl;
         } else {
            mc2log << warn 
                   << "[NavServerInfoHandler] Could not fetch news string." 
                   << " Status code: " << news.second << endl;

         }
         mc2log << warn << "[NavServerInfoHandler] client key is "
                << rd.clientType << ":" << rd.clientLang << endl;
      } else { 
         // ah, got news, nice. lets add param to reply.
         mc2dbg2 << "[NavServerInfoHandler] got news: " << news.first << endl;
         mc2log << " NewsString " << MC2CITE( news.first );
         rd.rparams.addParam( NParam( 27, news.first, 
                                      m_thread->clientUsesLatin1() ) );
      }

   }

   // Search desc CRC
   if ( ok && rd.reqVer > 2 ) {
      mc2log << " SearchHitTypesCRC " 
             << MC2HEX( m_thread->getSearchHandler().
                        getSearchHitTypesCRC( rd.clientLang ) );
      rd.rparams.addParam( NParam( 28, m_thread->getSearchHandler().
                                   getSearchHitTypesCRC( rd.clientLang ),
                                   m_thread->clientUsesLatin1() ) );
   }

   // PTUI
   if ( ok ) {
      mc2log << " PTUI " << MC2HEX( 
         m_group->getPeriodicTrafficUpdateInterval() );
      rd.rparams.addParam(
         NParam( 1107, m_group->getPeriodicTrafficUpdateInterval() ) );
   }

   mc2log << endl;

   return ok;
}


bool
NavServerInfoHandler::handleTopRegion( NavRequestData& rd ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // The user
   UserUser* user = rd.session->getUser()->getUser();

   // Start parameter printing
   mc2log << info << "handleTopRegion:";

   mc2log << " Lang " << StringTable::getString( 
      StringTable::getLanguageAsStringCode( rd.stringtClientLang ), 
                                            StringTable::ENGLISH );
   mc2log << endl;

   if ( ok ) {
      NParam data( 3100 );
      int res = m_group->getTopRegionData( 
         m_thread, user, rd.stringtClientLang, rd.urmask, data );
      uint32 crc = 0;
      if ( res == 0 ) {
         res = m_group->getTopRegionCRC( 
            m_thread, user, rd.stringtClientLang, rd.urmask, crc );
      }
      if ( res == 0 ) {
         rd.rparams.addParam( data );
         rd.rparams.addParam( NParam( 4300, crc ) );
      } else {
         ok = false;
         mc2log << warn << "handleTopRegion: Failed to get "
                << "TopRegionData ";
         if ( res == 2 ) {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            mc2log << "Timeout";
         } else {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
            mc2log << "Error";
         }
         mc2log << endl;
      }
   }

   return ok;
}


bool
NavServerInfoHandler::handleLatestNews( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();


   // Start parameter printing
   mc2log << info << "handleLatestNews:";

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
   
   MC2String clientType = "unknown";
   if ( params.getParam( 4 ) != NULL ) {
      clientType = params.getParam( 4 )->getString(
         m_thread->clientUsesLatin1());
   }
   const ClientSetting* clientSetting = m_group->getSetting( 
      params, m_thread );
   uint16 daysLeft = m_thread->getUserHandler()->getNbrDaysLeftForUser( 
      user, clientSetting );
   LangTypes::language_t lang = 
      ItemTypes::getLanguageCodeAsLanguageType( language );
   WFSubscriptionConstants::subscriptionsTypes userWFST = 
      m_thread->getSubscriptionTypeForUser( user, UserEnums::UR_WF );

   mc2log << endl;

   uint32 crc = 0;
   byte* latestNews = NULL;
   uint32 latestNewsLength = 0;
   if ( !m_group->getLatestNewsCRC( 
           clientType.c_str(), lang, userWFST, daysLeft, 
           clientSetting->getImageExtension(), crc ) )
   {
      ok = false;
   }

   if ( ok ) {
      ok = m_group->getLatestNews( 
         clientType.c_str(), lang, userWFST, daysLeft, clientSetting->
         getImageExtension(), latestNews, latestNewsLength );
   }

   if ( ok ) {
      rparams.addParam( NParam( 3300, latestNews, latestNewsLength ) );
      rparams.addParam( NParam( 4302, crc ) );
   } else {
      // No latest news
   }

   delete [] latestNews;

   return ok;
}


bool
NavServerInfoHandler::handleCategories( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();


   // Start parameter printing
   mc2log << info << "handleCategories:";

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
   mc2log << " Ver " << int(req->getReqVer());
   mc2log << endl;

   const ClientSetting* clientSetting = m_group->getSetting( 
      params, m_thread );

   mc2log << info << "handleCategories:";

   CategoryRegionID regionID = CategoryRegionID::NO_REGION;
   if ( req->getReqVer() >= 2 ) {
      regionID = getCategoryRegionIDFromParams( req->getParamBlock(), 
                                                m_thread );
   }

   uint32 crc = 0;
   if ( !clientSetting->getCategoriesCRC( 
           language, m_thread->clientUsesLatin1(), 
           regionID, crc ) ) {
      mc2log << " No categories for client type " 
             << clientSetting->getClientType();
      ok = false;
   }

   if ( ok ) {
      auto_ptr<CategoriesData> autoCat(
         clientSetting->getCategories( language, 
                                       m_thread->clientUsesLatin1(),
                                       regionID ) );
         
      NavCategoriesData* catData = 
         static_cast<NavCategoriesData*>( autoCat.get() );

      if ( catData != NULL ) {
         uint32 nbrCategories = 0;
         const byte* categories = NULL;
         uint32 categoriesLength = 0;

         catData->getCategories( categories, categoriesLength, nbrCategories );

         rparams.addParam( NParam( 4303, crc ) );
         rparams.addParam( NParam( 3500, categories, categoriesLength ) );

         if ( req->getReqVer() > 1 ) { // version 2

            // add filename for categories 
            const byte* filenamesData = NULL;
            uint32 filenamesDataLength = 0;
            catData->
               getCategoriesFilenames( filenamesData, filenamesDataLength);

            rparams.
               addParam( NParam( 3501, filenamesData, filenamesDataLength ) );

            // add ids for categories
            const byte* categoriesData;
            uint32 categoriesDataLength;
            catData->getCategoryIDs( categoriesData, categoriesDataLength );
            rparams.
               addParam( NParam( 3502, categoriesData, categoriesDataLength ));
         }
         mc2log << " CRC " << MC2HEX( crc ) << " nbrCategories " 
                << nbrCategories;
      }  else {
         // No Categories
         mc2log << " No categories data for client type " 
                << clientSetting->getClientType();
      }
   }

   mc2log << endl;

   return ok;
}


bool
NavServerInfoHandler::handleCallcenterList( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();
   
   const ClientSetting* clientSetting = m_group->getSetting( 
      req->getParamBlock(), m_thread );

   // Start parameter printing
   mc2log << info << "handleCallcenterList:";

   WFSubscriptionConstants::subscriptionsTypes userWFST = 
      m_thread->getSubscriptionTypeForUser( user, UserEnums::UR_WF );


   if ( userWFST != WFSubscriptionConstants::TRIAL &&
        userWFST != WFSubscriptionConstants::SILVER &&
        userWFST != MAX_BYTE )
   {
      NParam& pcallList = rparams.addParam( NParam( 3700 ) );
      pcallList.addString( clientSetting->getCallCenterList(),
                           m_thread->clientUsesLatin1() );
      rparams.addParam( NParam( 4305, 
                                clientSetting->getCallCenterListCRC() ) );
      mc2log << "\"" << clientSetting->getCallCenterList() << "\""
             << " CRC 0x" << hex << clientSetting->getCallCenterListCRC()
             << dec;
   }

   mc2log << endl;

   return ok;
}


bool
NavServerInfoHandler::handleServerList( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply, 
   IsabBoxInterfaceRequest* ireq )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   NParamBlock& rparams = reply->getParamBlock();

   // Start parameter printing
   mc2log << info << "handleServerList:";

   mc2log << endl;

   const ClientSetting& clientSetting = 
      *m_group->getSetting( req->getParamBlock(), m_thread );
   m_userHelp->addServerListParams( 
      rparams, clientSetting, ireq->getHttpRequest() != NULL, true );

   return ok;
}


bool
NavServerInfoHandler::handleNewPassword( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();


   // Start parameter printing
   mc2log << info << "handleNewPassword:";
   if ( params.getParam( 4000 ) ) {
      mc2log << " NewPassword: " << params.getParam( 4000 )->getString(
         m_thread->clientUsesLatin1());
   }

   mc2log << endl;

   if ( params.getParam( 4000 ) != NULL ) {
      int32 status = m_thread->changeUserPassword( 
         user, params.getParam( 4000 )->getString(
            m_thread->clientUsesLatin1()).c_str(), "", false, user );
      if ( status != 0 ) {
         mc2log << warn << "handleNewPassword: New password " 
                << "change failed " << int(status) << " (";
         uint8 navStatus = NavReplyPacket::NAV_STATUS_NOT_OK;
         if ( status == -2 ) { // timeout
            navStatus = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
            mc2log << "Timeout";
         } else {
            /*if ( if ( status == -3 ) {*/
            // bad old password
            mc2log << "Failed";
         }
         mc2log << ")" << endl;
         reply->setStatusCode( status );
      } else { // else ok
         mc2log << info << "handleNewPassword: changed "
                << "password." << endl;
         // Set param passwd
         rparams.addParam( NParam( 
                              2, params.getParam( 4000 )->getString(
                                 m_thread->clientUsesLatin1() ),
                              m_thread->clientUsesLatin1() ) );
      }

   }

   return ok;
}


bool
NavServerInfoHandler::handleServerAuthBob( UserItem* userItem, 
                                           NavRequestPacket* req,
                                           NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
//   NParamBlock& rparams = reply->getParamBlock();

   // Start parameter printing
   mc2log << info << "handleServerAuthBob:";

   mc2log << endl;

   return ok;
}

bool
NavServerInfoHandler::handleSearchDesc( NavRequestData& rd, 
                                        byte requestVersion ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // Start parameter printing
   mc2log << info << "handleSearchDesc:";

   mc2log << " Lang " << StringTable::getString( 
      StringTable::getLanguageAsStringCode( rd.stringtClientLang ), 
                                            StringTable::ENGLISH );
   mc2log << endl;

   if ( ok ) {
      mc2log << "handleSearchDesc: Reply:";
      MC2String crc = m_thread->getSearchHandler().getSearchHitTypesCRC( 
         rd.clientLang );
      rd.rparams.addParam( NParam( 28, crc,
                                   m_thread->clientUsesLatin1() ) );
      mc2log << " CRC " << crc;
      // get desc version 1, since nav clients implemented it in the correct way
      CompactSearchHitTypeVector types( 
         m_thread->getSearchHandler().getSearchHitTypes( rd.clientLang, 1 ) );

      NParam& data = rd.rparams.addParam( NParam ( 5900 ) );
      for ( CompactSearchHitTypeVector::const_iterator it = types.begin();
            it != types.end() ; ++it ) {
         mc2log << " {" << (*it).m_round << ", " 
                << (*it).m_heading << ", " 
                << MC2CITE( (*it).m_name ) << ", " 
                << MC2HEX( (*it).m_topRegionID ) << ", " 
                << MC2CITE( (*it).m_imageName ) << "}";
         data.addUint32( (*it).m_round );
         data.addUint32( (*it).m_heading );
         data.addString( (*it).m_name, m_thread->clientUsesLatin1() );
         if ( requestVersion > 1 ) {
            data.addString( (*it).m_type, 
                            m_thread->clientUsesLatin1() );
         }
         data.addUint32( (*it).m_topRegionID );
         data.addString( (*it).m_imageName, m_thread->clientUsesLatin1() );
      }
      mc2log << endl;
   }

   return ok;
}


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavRequestHandler.h"
#include "NavRequestData.h"
#include "InterfaceParserThread.h"
#include "isabBoxInterfaceRequest.h"
#include "NavPacket.h"
#include "isabBoxSession.h"
#include "NavUserHelp.h"
#include "STLStringUtility.h"
#include "NavUtil.h"
#include "NavParserThreadGroup.h"

// Handlers
#include "NavAuthHandler.h"
#include "NavSearchHandler.h"
#include "NavInfoHandler.h"
#include "NavRouteHandler.h"
#include "NavMapHandler.h"
#include "NavFavHandler.h"
#include "NavServerInfoHandler.h"
#include "NavMessHandler.h"
#include "NavCellHandler.h"
#include "NavTrackHandler.h"
#include "NavTunnelHandler.h"
#include "NavCombinedSearchHandler.h"
#include "NavCellIDHandler.h"
#include "NavKeyedDataHandler.h"
#include "NavRevGeocodingHandler.h"
#include "NavVerifyThirdPartyTransaction.h"
#include "NavLocalCategoryTreeHandler.h"
#include "NavOneSearchHandler.h"


NavRequestHandler::NavRequestHandler( InterfaceParserThread* thread,
                                      NavParserThreadGroup* group )
      : m_thread( thread ), m_group( group ),
        m_userHelp( new NavUserHelp( thread, group ) ),
        m_auth( new NavAuthHandler( thread, group, 
                                    m_userHelp ) ),
        m_search( new NavSearchHandler( thread, group, 
                                        m_userHelp ) ),
        m_info( new NavInfoHandler( thread, group ) ),
        m_route( new NavRouteHandler( thread, group, 
                                      m_userHelp ) ),
        m_map( new NavMapHandler( thread, group, m_userHelp ) ),
        m_fav( new NavFavHandler( thread, group, m_userHelp ) ),
        m_serverInfo( new NavServerInfoHandler( thread,
                                                group, m_userHelp ) ),
        m_mess( new NavMessHandler( thread, group, 
                                    m_userHelp ) ),
        m_cell( new NavCellHandler( thread, group, 
                                    m_userHelp ) ),
        m_track( new NavTrackHandler( thread, group, 
                                      m_userHelp ) ),
        m_tunnel( new NavTunnelHandler( thread, group, 
                                        m_userHelp ) ),
        m_csearch( new NavCombinedSearchHandler( thread, group, 
                                                 m_userHelp, m_search ) ),
        m_cellID( new NavCellIDHandler( thread, group ) ),
        m_keyedData( new NavKeyedDataHandler( thread, group ) ),
        m_revGeocoding( new NavRevGeocodingHandler( thread, group ) ),
        m_verifyThirdParty( new NavVerifyThirdPartyTransactionHandler(
                               thread, group ) ),
        m_localCategoryTree( new NavLocalCategoryTreeHandler( thread, group ) ),
        m_oneSearch( new NavOneSearchHandler( thread, group ) )
{
}


NavRequestHandler::~NavRequestHandler() {
   delete m_auth;
   delete m_userHelp;
   delete m_search;
   delete m_info;
   delete m_route;
   delete m_map;
   delete m_fav;
   delete m_serverInfo;
   delete m_mess;
   delete m_cell;
   delete m_track;
   delete m_tunnel;
   delete m_csearch;
   delete m_cellID;
   delete m_keyedData;
   delete m_revGeocoding;
}


bool copyRequestToReply( NavRequestData& rd ) {
   rd.rparams = rd.params;

   return true;
}


void
NavRequestHandler::handleRequest( NavInterfaceRequest* req ) {
   MC2_ASSERT( dynamic_cast< IsabBoxInterfaceRequest* > ( req ) );
   IsabBoxInterfaceRequest* ireq = 
      static_cast< IsabBoxInterfaceRequest* > ( req );

   uint32 startTime = TimeUtility::getCurrentTime();
   MC2String reqName;
   bool ok = true;

   isabBoxSession* isession = ireq->getSession();
   m_thread->setLogPrefix( isession->m_logPrefix );
   m_thread->setPeerIP( isession->getPeerIP() );

   // The request packet
   NavRequestPacket* pack = ireq->getRequestPacket();

   if ( pack == NULL ) {
      mc2log << "NavRequestHandler::handleRequest pack was NULL" 
             << endl;
      return;
   }

   // Use utf-8?
   if ( pack->getProtoVer() > 0xa ) { 
      m_thread->setClientUsesLatin1( false );
   }

   bool printedInParams = false;
   // Debug print all params
//#ifdef DEBUG_LEVEL_2
   pack->dump( mc2log, "NavRequest", true, true, MAX_UINT32, MAX_UINT32 );
   printedInParams = true;
//#endif

   // The reply
   NavReplyPacket* reply = new NavReplyPacket( 
      pack->getProtoVer(),  pack->getType() + 1, pack->getReqID(),
      pack->getReqVer(), 0, "" );

   // Holder of input
   NavRequestData rd( pack->getParamBlock(), reply->getParamBlock(),
                      pack, reply, ireq );
   rd.ipnport = isession->getPeerIPnPort();

   // Set some common parameters
   rd.clientType = "unknown";
   rd.clientTypeOptions = "";
   if ( rd.params.getParam( 4 ) != NULL ) { //client type
      rd.clientType = rd.params.getParam( 4 )->getString(
         m_thread->clientUsesLatin1() );
      rd.clientTypeSet = true;
   }
   if ( rd.params.getParam( 5 ) != NULL ) { //client type option
      rd.clientTypeOptions = rd.params.getParam( 5 )->getString(
         m_thread->clientUsesLatin1() );
      rd.clientTypeOptionsSet = true;
   }
   if ( rd.params.getParam( 6 ) != NULL ) { //language of client
      rd.stringtClientLang = NavUtil::mc2LanguageCode(
         rd.params.getParam( 6 )->getUint16() );
      rd.clientLang = ItemTypes::getLanguageCodeAsLanguageType(
         rd.stringtClientLang );
   }
   // Program version
   if ( rd.params.getParam( 11 ) != NULL ) { //program version
      rd.setClientVersion( 
         rd.params.getParam( 11 )->getUint32Array( 0 ),
         rd.params.getParam( 11 )->getUint32Array( 1 ),
         rd.params.getParam( 11 )->getUint32Array( 2 ) );
   }

   // Set the client setting from clientType, clientTypeOptions
   rd.clientSetting = m_group->getSetting( rd.clientType.c_str(),
                                           rd.clientTypeOptions.c_str() );
   m_thread->setClientSetting( rd.clientSetting );


   // Handle auth here 
   if ( ok ) {
      if ( rd.req->getType() != NavPacket::NAV_NOP_REQ &&
           rd.req->getType() != NavPacket::NAV_ECHO_REQ ) {
         ok = m_auth->handleAuth( rd );
      } else {
         // NOP request do not try to create user etc. in auth just reply
         mc2log << info << "NOP request "
                << "V" << rd.getClientVersionAsString() << " PV " 
                << int(rd.req->getProtoVer()) << endl;
      }
   }

   reqName = NavPacket::requestTypeAsString( pack->getType() );  

   if ( ok ) {
      // Set user in thread.
      if ( isession->getUser() != NULL ) {
         m_thread->setUser( isession->getUser() );
         m_thread->setClientSetting( rd.clientSetting );
         m_thread->setRequestData( &rd );
      }
      
      // Have route/search/map/... object to handle request types
      // These objects should have a "checkvalidparams" that has list of
      // params and types, max-, min-length, and checks them. called first
      // in handle"reqtype".


      switch( pack->getType() ) {

         case NavPacket::NAV_SEARCH_REQ :
            ok = m_search->handleSearch( 
               isession->getUser(), pack, reply );
            break;

         case NavPacket::NAV_INFO_REQ :
            ok = m_info->handleInfo( isession->getUser(), pack, reply );
            break;

         case NavPacket::NAV_DETAIL_REQ :
            ok = m_info->handleDetail( isession->getUser(), pack, reply );
            break;   

         case NavPacket::NAV_ROUTE_REQ :
            ok = m_route->handleRoute( rd );
            break;

         case NavPacket::NAV_MAP_REQ :
            ok = m_map->handleMap( isession->getUser(), pack, reply );
            break;

         case NavPacket::NAV_VECTOR_MAP_REQ :
            ok = m_map->handleVectorMap( isession->getUser(), 
                                         pack, reply );
            break;

         case NavPacket::NAV_MULTI_VECTOR_MAP_REQ :
            ok = m_map->handleMultiVectorMap( isession->getUser(), 
                                              pack, reply );
            break;

         case NavPacket::NAV_FAV_REQ :
            ok = m_fav->handleFav( isession->getUser(), pack, reply );
            break;

         case NavPacket::NAV_WHOAMI_REQ :
            // Is handled in auth
            if ( !printedInParams ) {
               pack->getParamBlock().dump( mc2log, true, true, 
                                           MAX_UINT32, MAX_UINT32 );
               printedInParams = true;
            }
            break;

         case NavPacket::NAV_NOP_REQ :
            // Easy No OPeration request
            break;

         case NavPacket::NAV_TOP_REGION_REQ :
            ok = m_serverInfo->handleTopRegion( rd );
            break;

         case NavPacket::NAV_LATEST_NEWS_REQ :
            ok = m_serverInfo->handleLatestNews( isession->getUser(), 
                                                 pack, reply );
            break;

         case NavPacket::NAV_CATEGORIES_REQ :
            ok = m_serverInfo->handleCategories( isession->getUser(), 
                                                 pack, reply );
            break;

         case NavPacket::NAV_CALLCENTER_LIST_REQ :
            ok = m_serverInfo->handleCallcenterList( isession->getUser(), 
                                                     pack, reply );
            break;

         case NavPacket::NAV_SERVER_LIST_REQ :
            ok = m_serverInfo->handleServerList( isession->getUser(), 
                                                 pack, reply, ireq );
            break;

         case NavPacket::NAV_NEW_PASSWORD_REQ :
            ok = m_serverInfo->handleNewPassword( isession->getUser(), 
                                                  pack, reply );
            break;

         case NavPacket::NAV_SERVER_INFO_REQ :
            if ( !printedInParams ) {
               pack->getParamBlock().dump( mc2log, true, true,
                                           MAX_UINT32, MAX_UINT32 );
            }
            ok = m_serverInfo->handleServerInfo( rd );
            break;

         case NavPacket::NAV_MESSAGE_REQ :
            ok = m_mess->handleMess( isession->getUser(), pack, reply );
            break;

         case NavPacket::NAV_UPGRADE_REQ :
            ok = m_auth->handleUpgrade( rd );
            break;

         case NavPacket::NAV_CELL_REPORT :
            ok = m_cell->handleCell( isession->getUser(), pack, reply );
            break;

         case NavPacket::NAV_CHANGED_LICENCE_REQ :
            // Is handled in auth, but external users uses extauth so it is 
            // needed here to.
            ok = m_auth->handleChangedLicence( rd );
            break;

         case NavPacket::NAV_SERVER_AUTH_BOB_REQ :
            ok = m_serverInfo->handleServerAuthBob( isession->getUser(), 
                                                    pack, reply );
            break;

         case NavPacket::NAV_TUNNEL_DATA_REQ :
            ok = m_tunnel->handleTunnel( rd );
            break;
         case NavPacket::NAV_TRACK_REQ :
            ok = m_track->handleTracking( rd );
            break;
         case NavPacket::NAV_COMBINED_SEARCH_REQ :
            ok = m_csearch->handleCombinedSearch( rd );
            break;

         case NavPacket::NAV_SEARCH_DESC_REQ :
            ok = m_serverInfo->handleSearchDesc( rd, pack->getReqVer() );
            break;

         case NavPacket::NAV_CELLID_LOOKUP_REQ :
            ok = m_cellID->handleCellIDLookup( rd );
            break;

         case NavPacket::NAV_GET_KEYED_DATA_REQ:
            ok = m_keyedData->handleGetKeyedData( rd.clientLang,
                                                  pack, reply );
            break;

         case NavPacket::NAV_REV_GEOCODING_REQ:
            ok = m_revGeocoding->handleRevGeocoding( isession->getUser(),
                                                     rd.urmask,
                                                     rd.stringtClientLang,
                                                     pack, reply );
            break;
         case NavPacket::NAV_ECHO_REQ :
            // Send back the request data
            ok = copyRequestToReply( rd );
            break;

         case NavPacket::NAV_VERIFY_THIRD_PARTY_TRANSACTION_REQ :
            ok = m_verifyThirdParty->handleVerifyThirdPartyTransaction( rd );
            break;

         case NavPacket::NAV_LOCAL_CATEGORY_TREE_REQ :
            ok = m_localCategoryTree->handleLocalCategoryTree( rd );
            break;

         case NavPacket::NAV_ONE_SEARCH_REQ :
            ok = m_oneSearch->handleOneSearch( rd );
            break;

         default:
            mc2log << "handleRequest: Unknown request type: 0x" << hex
                   << pack->getType() << dec << " params: ";
            if ( !printedInParams ) {
               pack->getParamBlock().dump( mc2log, true, true,
                                           MAX_UINT32, MAX_UINT32 );
            }
            reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_UNKNOWN_REQUEST );
            break;
      } // End switch request type

      if ( !ok ) {
         pack->getParamBlock().dump( mc2log, true, true, 
                                     MAX_UINT32, MAX_UINT32 );
      }
      
      m_thread->setUser( NULL );
      m_thread->setClientSetting( NULL );
      m_thread->setRequestData( NULL );
      
   } // End if ok


   ireq->setReplyPacket( reply );

   uint32 stopTime = TimeUtility::getCurrentTime();
   uint32 requestTime = stopTime - startTime;
   mc2log << info << "handleRequest " << reqName << " got " 
          << ireq->getRequestSize() << " (" 
          << ireq->getUncompressedRequestSize() << ")"
          << " bytes reply " 
          << ireq->getReplySize() << " (" 
          << ireq->getUncompressedReplySize() << ")"
          << " bytes time in sys "
          << ireq->getTimeFromPutInQueueMs() << " ms time "
          << requestTime << " ms IO time " 
          << ireq->getTotalUsedIOTime() << " ms status "
          << hex << "0x" << int(reply->getStatusCode()) << dec;
   if ( reply->getStatusMessage()[ 0 ] != '\0' ) {
      mc2log << " " << reply->getStatusMessage();
   }
   if ( reply->getParamBlock().getParam( 26 ) != NULL ) {
      const NParam* urlParam = reply->getParamBlock().getParam( 26 );
      mc2log << " url " << urlParam->getString( m_thread->clientUsesLatin1() );
   }
   mc2log << endl;
   // Debug print all params
#ifdef DEBUG_LEVEL_2
   reply->getParamBlock().dump( mc2log, true, true );
#endif

   // Reset thread setting
   m_thread->setClientUsesLatin1( true );
}


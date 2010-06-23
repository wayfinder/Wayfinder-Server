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

#include "ParserThread.h"
#include "ParserThreadGroup.h"
#include "SinglePacketRequest.h"
#include "DebitPacket.h"
#include "STLUtility.h"
#include "SendRequestHolder.h"
#include "SearchRequest.h"
#include "RouteRequest.h"
#include "SMSSendRequest.h"
#include "RouteStoragePacket.h"
#include "RouteMessageRequest.h"
#include "LocalMapMessageRequest.h"
#include "ExpandRouteRequest.h"
#include "UserRight.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "ProximityRequest.h"
#include "TopRegionRequest.h"
#include "TransactionPacket.h"
#include "ServerRegionIDs.h"
#include "UserPacket.h"
#include "DataBuffer.h"
#include "BitBuffer.h"
#include "ExpandRoutePacket.h"
#include "WFActivationPacket.h"
#include "MultiplePacketsRequest.h"
#include "ClientSettings.h"
#include "MC2Logging.h"
#include "URLFetcher.h"
#include "MC2SimpleString.h"
#include "RequestTime.h"
#include "NetUtility.h"
#include "BBoxPacket.h"
#include "DisturbanceDescription.h"
#include "Properties.h"
#include "RouteID.h"
#include "LogBuffer.h"
#include "OperationTypes.h"

#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "PurchaseOptions.h"
#include "IP.h"
#include "PositionInfo.h"

#include "ExpandedRoute.h"
#include "RouteRequestParams.h"
#include "DriverPref.h"

#include "MultiRequest.h"
#include "DebugClock.h"
#include "DriverPref.h"
#include "ServerSubRouteVector.h"
#include "UnexpandedRoute.h"
#include "GzipUtil.h"
#include "UserRightHelper.h"
#include "ParserExternalAuthHttpSettings.h"
#include "TimedOutSocketLogger.h"

// For fstat and friends
#include "stat_inc.h"

// Handlers
#include "ParserTokenHandler.h"
#include "ParserExternalAuth.h"
#include "ParserUserHandler.h"
#include "ParserDebitHandler.h"
#include "ParserTileHandler.h"
#include "ParserMapHandler.h"
#include "ParserActivationHandler.h"
#include "ParserCWHandler.h"
#include "ParserRouteHandler.h"
#include "SearchParserHandler.h"
#include "ParserPoiReviewHandler.h"
#include "ParserPosPush.h"
#include "ParserAppStoreHandler.h"

#include "TileMapBufferQuery.h"
#include "ThreadRequestInfo.h"
#include "SearchHeadingManager.h"

#include "MapBits.h"

#ifdef USE_SSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

#include <fstream>
#include <iomanip>
#include <memory>
#include <set>
#include <boost/lexical_cast.hpp>


using namespace UserRightHelper;

ParserThread::ParserThread( ParserThreadGroupHandle group,
                            const char* name ) 
      : ISABThread( group.get(), name ),
        m_clientSetting( NULL ),
        m_tokenHandler( new ParserTokenHandler( this,
                                                group.get() ) ),
        m_externalAuth( new ParserExternalAuth( this,
                                                group.get() ) ),
        m_userHandler( new ParserUserHandler( this,
                                              group.get() ) ),
        m_debitHandler( new ParserDebitHandler( this,
                                                group.get() ) ),
        m_activationHandler( new ParserActivationHandler( this,
                                                          group.get() ) ),
        m_tileHandler( new ParserTileHandler( this,
                                              group.get() ) ),
        m_parserMapHandler( new ParserMapHandler( this,
                                                  group.get() ) ),
        m_routeHandler( new ParserRouteHandler( this, group.get() ) ),
        m_cwHandler( new ParserCWHandler( this, group.get() ) ),
        /*
        m_searchHandler( new SearchParserHandler( this,
                                                  group.get() ) ),
        */
        m_poiReviewHandler( new ParserPoiReviewHandler( this,
                                                        group.get() ) ),
        m_posPush( new ParserPosPush( this, group.get() ) ),
        m_appStoreHandler( new ParserAppStoreHandler( this, group.get() ) ),
        m_clientUsesLatin1( true ),
        m_urlFetcher( new URLFetcher( group->getSSLContext() ) ),
        m_serverAddress(0, 0),
        m_searchHeadingCRC( 0 )
{
   m_user = NULL;
   m_group = group;
   m_isCheckUser = false;


   m_serverName = NetUtility::getLocalHostName();

   // end hostname at first dot (.)
   MC2String::size_type pos = m_serverName.find_first_of(".");
   if ( pos != MC2String::npos ) {
      m_serverName = m_serverName.substr( 0, pos );
   }

}


ParserThread::~ParserThread() 
{
#ifdef USE_SSL
    ERR_remove_state( 0 );
#endif
}


UserItem*
ParserThread::getCurrentUser()
{
   return m_user;
}

void
ParserThread::setUser( UserItem* user )
{
   MC2_ASSERT( user == NULL || m_user == NULL );
   // clear user stuff
   if ( user == NULL || m_user != user ) {
      clearUserItem();
   }
   m_user = user;
}

void
ParserThread::setClientSetting( const ClientSetting* settings ) {
   m_clientSetting = settings;
}

void
ParserThread::setRequestData( const InterfaceRequestData* rd ) {
   m_requestData = rd;
}

const InterfaceRequestData*
ParserThread::getRequestData() const {
   return m_requestData;
}

void
ParserThread::putRequest(ThreadRequestContainer* reqCont) 
{
   DebugClock clock;
   
   mc2dbg4 << "[PT::putRequest]: User is " 
           << MC2HEX(getCurrentUser()) << endl;
   ISABSync sync( m_monitor );

   mc2dbg4 << "ParserThread::putRequest()" << endl;

   mc2log << info << "[PT] Processing request ";
   ThreadRequestInfo::printBeginInfo( mc2log, m_group->m_serverType, *reqCont );
   mc2log << endl;

   reqCont->setThread( this );

   m_group->putRequest( reqCont );
   internalWaitForRequest( reqCont->getRequest()->getID() );

   // Done.
   mc2log << info << "[PT] Processed  request ";
   ThreadRequestInfo::printEndInfo( mc2log, m_group->m_serverType, *reqCont,
                                    clock.getTime() );
   mc2log << endl;

   reqCont->getRequest()->setTime( RequestTime::putRequest, 
                                   clock.getTime() );

   // enable this to dump time info about functions
   mc2dbg8 << RequestTime::Dump( *reqCont->getRequest() ) << endl;
}

void
ParserThread::sendRequest( ThreadRequestContainer* reqCont ) {
   ISABSync sync( m_monitor );
   SendRequestHolder* holder = new SendRequestHolder( reqCont, this );
   m_requests.insert( holder );
   m_group->putRequest( holder->getReqCont() );
}

void
ParserThread::sendRequest( Request* req ) {
   ISABSync sync( m_monitor );
   SendRequestHolder* holder = new SendRequestHolder( req, this );
   m_requests.insert( holder );
   m_group->putRequest( holder->getReqCont() );
}

void
ParserThread::sendRequest( PacketContainer* cont ) {
   ISABSync sync( m_monitor );
   SendRequestHolder* holder = new SendRequestHolder( cont, this );
   m_requests.insert( holder );
   m_group->putRequest( holder->getReqCont() );
}

void
ParserThread::sendRequest( Packet* pack, moduletype_t module ) {
   ISABSync sync( m_monitor );
   SendRequestHolder* holder = new SendRequestHolder( pack, module, this );
   m_requests.insert( holder );
   m_group->putRequest( holder->getReqCont() );
}

void
ParserThread::waitForRequest( uint16 requestID ) {
   ISABSync sync( m_monitor );
   internalWaitForRequest( requestID );
}

void
ParserThread::waitForRequest( const ThreadRequestContainer* reqCont ) {
   waitForRequest( reqCont->getRequest()->getID() );
}

void 
ParserThread::putRequest( Request* req ) {
   ThreadRequestContainer reqCont( req );
   putRequest( &reqCont );
}

void
ParserThread::putRequests( const vector<RequestWithStatus*>& reqs )
{
   mc2dbg4 << "[PT]: putRequests " << reqs.size() << " requests" << endl;
   if ( ! reqs.empty() ) {
      if ( true ) {
         MultiRequest req( getNextRequestID(), reqs );      
         putRequest ( & req );
         mc2dbg4 << "[PT]: Multi done. Status = "
                 << MC2CITE( StringTable::getString(req.getStatus(),
                                                    StringTable::ENGLISH ) )
                 << endl;
      } else {
         for ( vector<RequestWithStatus*>::const_iterator rt = reqs.begin();
               rt != reqs.end();
               ++rt ) {
            putRequest( *rt );
         }
      }
   }
}

PacketContainer* 
ParserThread::putRequest( PacketContainer* cont ) {
   SinglePacketRequest request ( getNextRequestID(), cont );
   putRequest( &request );
   PacketContainer* ans = request.getAnswer();
   return ans;
}

PacketContainer* 
ParserThread::putRequest( Packet* pack, moduletype_t module ) {
   return putRequest( new PacketContainer( pack, 0, 0, module ) );
}


void
ParserThread::putRequest( vector< PacketContainer* >& reqs, 
                          vector< PacketContainer* >& reps )
{
   //NOTE! void ParserThread::putRequest( vector< PacketContainer* >&
   //packets ) calls this function with reps and reqs referring to the
   //same vector. If this function is changed in a way that makes this
   //unsafe, you must also change that function.
   if ( ! reqs.empty() ) {
      uint32 nbrP = reqs.size();
      MultiplePacketsRequest* m = new MultiplePacketsRequest( 
         getNextRequestID(), nbrP );
      for ( uint32 i = 0 ; i < nbrP ; ++i ) {
         m->add( reqs[ i ] );
      }
      reqs.clear(); // The packets are now in m
      putRequest( m );
      for ( uint32 i = 0 ; i < nbrP ; ++i ) {
         reps.push_back( m->getAnswer( i ) );
      }
      delete m;
   }
}

void
ParserThread::putRequest( vector< PacketContainer* >& packets )
{
   putRequest( packets, packets );
}

void 
ParserThread::putAnswer( ThreadRequestContainer* reqCont ) 
{
   ISABSync sync( m_monitor );
   mc2dbg4 << "ParserThread::putAnswer()" << endl;
   m_replies.insert( reqCont );

   m_monitor.notify();
}

void
ParserThread::internalWaitForRequest( uint16 requestID ) {
   // m_monitor is locked
   MultiplePacketsRequest req( requestID, 0 );
   ThreadRequestContainer reqCont( &req );
   while ( ! STLUtility::has( m_replies, &reqCont ) ) {
      m_monitor.wait();
   }
   // Remove it from waiting replies
   m_replies.erase( m_replies.find( &reqCont ) );
   // Find and delete it from the m_requests
   SendRequestHolder holder( &reqCont, this );
   ReqHolderSet::iterator it = m_requests.find( &holder );
   if ( it != m_requests.end() ) {
      delete *it;
      m_requests.erase( it );
   }
}

void 
ParserThread::putPushPacket( PushPacket* packet, uint32 serviceID, 
                             SubscriptionResource& resource )
{
   mc2log << warn << "ParserThread::putPushPacket got PushPacket "
          << "implement putPushPacket in subclass." << endl;
   // Default dummy implementation does nothing
}

PacketContainer* 
ParserThread::getStoredRoute( uint32 routeID,
                              uint32 createTime,
                              RouteReplyPacket*& routePack,
                              uint32& UIN,
                              const char*& extraUserinfo,
                              uint32& validUntil,
                              int32& originLat,
                              int32& originLon,
                              uint32& originMapID,
                              uint32& originItemID,
                              uint16& originOffset,
                              int32& destinationLat,
                              int32& destinationLon,
                              uint32& destinationMapID,
                              uint32& destinationItemID,
                              uint16& destinationOffset )
{
   return m_group->getStoredRoute( 
      routeID, createTime, routePack, UIN, extraUserinfo, validUntil,
      originLat, originLon, originMapID, originItemID, originOffset,
      destinationLat, destinationLon, destinationMapID, destinationItemID,
      destinationOffset, this );
                                   
}


RouteReplyPacket*
ParserThread::getStoredRoute( const RouteID& routeID ) {
   return m_group->getStoredRoute( routeID, this );
}                           

bool
ParserThread::storeRoute( RouteRequest* req,
                          uint32 UIN,
                          const char* const extraUserinfo,
                          UserEnums::URType urmask,
                          const RouteID& forceRouteID,
                          RouteStorageGetRouteReplyPacket** p )
{
   if ( m_isCheckUser ) {
      return false;
   }
   bool ok = true;

   if ( req->getAnswer() != NULL && req->getRouteReplyPacket() != NULL ) {
      typedef const vector<DisturbanceDescription> cddv;
      cddv& dis = req->getDisturbanceInfo();
      uint32 now = TimeUtility::getRealTime();
      uint32 validUntil = now + static_cast< ExpandRouteReplyPacket* > (
         req->getAnswer()->getPacket() )->getTotalTime()*10 + 3600;
      uint32 createTime = req->getRouteCreateTime();
      uint32 routeID = req->getRouteID();

      if ( forceRouteID.isValid() ) {
         routeID = forceRouteID.getRouteIDNbr();
         createTime = forceRouteID.getCreationTime();
      }

      uint32 oMapID = 0;
      uint32 oItemID = 0;
      const char* oName = NULL;
      int32 oLat = 0;
      int32 oLon = 0;
      uint16 oOffset = 0;
      uint32 dMapID = 0;
      uint32 dItemID = 0;
      const char* dName = NULL;
      int32 dLat = 0;
      int32 dLon = 0;            
      uint16 dOffset = 0;

      req->getRouteOrigin( oMapID, oItemID, oOffset, oName );
      req->getRouteOriginCoord( oLat, oLon );
      req->getRouteDestination( dMapID, dItemID, dOffset, dName );
      req->getRouteDestinationCoord( dLat, dLon );


      // Start angle
      ExpandedRoute* er = req->getExpandedRoute();
      uint16 startAngle = 0;
      if ( er->getOrigin( 0 ) != NULL ) {
         startAngle = er->getOrigin( 0 )->getAngle();
      }

      if ( req->getOriginAngle() != MAX_UINT16 ) {
         startAngle = req->getOriginAngle();
      }

      // DriverPref to the rescue!
      RouteRequestParams params;
      req->extractParams( params );

      auto_ptr<RouteReplyPacket> routeReplyPack(
         static_cast<RouteReplyPacket*>( 
            req->getRouteReplyPacket()->getClone() ) );
      routeReplyPack->setDisturbanceDescriptions( dis );
      routeReplyPack->setNbrRouteObjectsUsed( 
         req->getNbrRouteObjectsUsed() );

      auto_ptr<PacketContainer> cont ( putRequest( 
         new RouteStorageAddRouteRequestPacket( 
            routeReplyPack.get(),
            routeID,
            UIN,
            extraUserinfo,
            validUntil,
            createTime,
            oLat, oLon, startAngle, oMapID, oItemID, oOffset,
            dLat, dLon, dMapID, dItemID, dOffset, params.getDriverPrefs(),
            urmask, forceRouteID.isValid()/*isReRoute*/ ),
         MODULE_TYPE_USER ) );

      if ( p != NULL ) {
         RouteStorageGetRouteRequestPacket in( routeID, createTime );
         *p = new RouteStorageGetRouteReplyPacket( 
            &in, StringTable::OK, routeReplyPack.get(), routeID, UIN,
            extraUserinfo,
            validUntil,
            createTime,
            oLat, oLon, startAngle, oMapID, oItemID, oOffset,
            dLat, dLon, dMapID, dItemID, dOffset, params.getDriverPrefs(),
            urmask );
      }
      
      if ( cont.get() == NULL ||
           static_cast< ReplyPacket* >( cont->getPacket() )->getStatus() 
           != StringTable::OK )
      {
         mc2log << warn <<  "ParserThread::storeRoute failed to store route."
                << "routeID " << routeID 
                << " createTime " << createTime 
                << " UIN " << UIN 
                << " extraUserinfo " << extraUserinfo
                << " validUntil " << validUntil << endl;
         ok = false;
      } else {
         mc2dbg2 << info << "ParserThread::storeRoute route stored "
                 << "routeID: " << routeID << " createTime " 
                 << createTime 
                 << " for UIN " << UIN 
                 << " extraUserinfo " << extraUserinfo << endl;
      }
   } else {
      mc2log << warn << "ParserThread::storeRoute no routedata to store"
             << endl;
      ok = false;
   }

   return ok;
}


StringTable::stringCode
ParserThread::updateStoredRoute( uint32 routeID, uint32 createTime, 
                                 uint32 validUntil )
{
   if ( m_isCheckUser ) {
      return StringTable::OK;
   }

   StringTable::stringCode res = StringTable::OK;

   RouteStorageChangeRouteRequestPacket* p = 
      new RouteStorageChangeRouteRequestPacket( routeID, createTime,
                                                validUntil );
   SinglePacketRequest* req = new SinglePacketRequest( 
      getNextRequestID(), 
      new PacketContainer( p,
                           0,
                           0,
                           MODULE_TYPE_USER ) );
   mc2dbg8 << "About to send RouteStorageChangeRouteRequest" << endl;
   putRequest( req );
   mc2dbg8 << "RouteStorageChangeRouteRequest returned" << endl;
   PacketContainer* cont = req->getAnswer();
   if ( cont != NULL &&
        static_cast< ReplyPacket* >( cont->getPacket() )->getStatus() 
        == StringTable::OK )
   {
      // Ok
      res = StringTable::OK;
   } else {
      mc2log << warn << "ParserThread::updateStoredRoute failed to update "
                "stored route routeID " << routeID 
             << " createTime " << createTime << endl;
      if ( cont != NULL ) {
         res = StringTable::stringCode( 
            static_cast< ReplyPacket* >( 
               cont->getPacket() )->getStatus() );
      } else {
         res = StringTable::TIMEOUT_ERROR;
      }
   }

   delete req; 
   delete cont;

   return res;
}



PacketContainer* 
ParserThread::getStoredRouteAndExpand( 
   uint32 routeID, uint32 createTime,
   uint32 expandType, StringTable::languageCode lang,
   bool abbreviate, bool landmarks, bool removeAheadIfDiff,
   bool nameChangeAsWP,
   RouteReplyPacket*& routePack,
   uint32& UIN,
   const char*& extraUserinfo,
   uint32& validUntil,
   int32& originLat, int32& originLon,
   uint32& originMapID, uint32& originItemID, 
   uint16& originOffset,
   int32& destinationLat, int32& destinationLon,
   uint32& destinationMapID, uint32& destinationItemID,
   uint16& destinationOffset,
   ExpandRouteRequest*& expReq,
   PacketContainer*& expandRouteCont )
{
   PacketContainer* cont = getStoredRoute( routeID, createTime,
                                           routePack, UIN,
                                           extraUserinfo, validUntil,
                                           originLat, originLon,
                                           originMapID, originItemID,
                                           originOffset,
                                           destinationLat, destinationLon,
                                           destinationMapID, 
                                           destinationItemID, 
                                           destinationOffset );
   if ( cont != NULL && routePack != NULL ) {
      routePack->setStartOffset( originOffset );
      routePack->setEndOffset( destinationOffset );
      RequestData reqID = getNextRequestID();
      expReq = 
         new ExpandRouteRequest( reqID,
                                 routePack, expandType, lang,
                                 abbreviate, landmarks, removeAheadIfDiff,
                                 nameChangeAsWP );
      ThreadRequestContainer* reqCont = new ThreadRequestContainer( 
         expReq );
      DEBUG8(cerr << "About to send ExpandRouteRequest" << endl;);
      putRequest( reqCont );
      DEBUG8(cerr << "ExpandRouteRequest returned" << endl;);
      
      expandRouteCont = expReq->getAnswer();
      if ( expandRouteCont != NULL ) {
         // Set requestID in expand so we can use it 
         // when debiting
         expandRouteCont->getPacket()->setRequestID( reqID );
      }
      
      delete reqCont;
   } else {
      expReq = NULL;
      expandRouteCont = NULL;
      delete cont;
      cont = NULL;
      delete routePack;
      routePack = NULL;
   }

   return cont;
}


RouteReplyPacket* 
ParserThread::getStoredRouteAndExpand( 
   RouteID routeID, uint32 expandType, 
   StringTable::languageCode lang, bool abbreviate, bool landmarks,
   bool removeAheadIfDiff, bool nameChangeAsWP,
   ExpandRouteRequest*& expReq, PacketContainer*& expandRouteCont )
{
   RouteReplyPacket* routePack = getStoredRoute( routeID );
   if ( routePack != NULL ) {
      RequestData reqID = getNextRequestID();
      expReq = new ExpandRouteRequest( reqID, routePack, expandType, lang,
                                       abbreviate, landmarks, 
                                       removeAheadIfDiff, nameChangeAsWP );
      mc2dbg8 << "About to send ExpandRouteRequest" << endl;
      putRequest( expReq );
      mc2dbg8 << "ExpandRouteRequest returned" << endl;
      
      expandRouteCont = expReq->getAnswer();
      if ( expandRouteCont != NULL ) {
         // Set requestID in expand so we can use it 
         // when debiting
         expandRouteCont->getPacket()->setRequestID( reqID );
      }
   } else {
      expReq = NULL;
      expandRouteCont = NULL;
   }

   return routePack;
}


bool 
ParserThread::getUser( const char* const logonID, UserItem*& userItem,
                       bool useCache, bool wipeFromCache )
{
   UserUser findUser( MAX_UINT32 );
   findUser.setLogonID( logonID );
   uint32 nbrUsers = 0;

   return getUserFromUserElement( &findUser, userItem, 
                                  nbrUsers, useCache, wipeFromCache );
}


bool 
ParserThread::getUser( uint32 UIN, UserItem*& userItem, 
                       bool useCache, bool wipeFromCache )
{
   return m_group->getUser( UIN, userItem, this, useCache, wipeFromCache );
}


bool
ParserThread::getUserBySession( 
   const char* sessionID, const char* sessionKey,
   UserItem*& userItem, bool useCache, bool wipeFromCache )
{
   return m_group->getUserBySession( sessionID, sessionKey, userItem,
                                     useCache, wipeFromCache, this );
}


uint32
ParserThread::createUser( UserUser* user, const char* const passwd,
                          const UserUser* changerUser )
{
   uint32 uin = MAX_UINT32;

   // Pack data into packet
   AddUserRequestPacket* p = new AddUserRequestPacket(
      0, 0, user, passwd, changerUser ? changerUser->getUIN(): 0 );

//    for ( uint32 i = 0 ; i < user->getNbrElements() ; i++ ) {
//       p->addElement( user->getElement( i ) );
//    }

   vector<UserElement*> elements;
   user->getAllElements( elements );
   for ( vector<UserElement*>::iterator it = elements.begin();
         it != elements.end();
         ++it ) {
      p->addElement( *it );
   }

   // Send packet
   SinglePacketRequest* req = new SinglePacketRequest(
      getNextRequestID(),
      new PacketContainer(
         p, 0, 0, MODULE_TYPE_USER ) );
   mc2dbg8 << "About to send AddUserRequest" << endl;
   putRequest( req );
   PacketContainer* ansCont = req->getAnswer();
   mc2dbg8 << "AddUserRequest returned" << endl;
   // Check answer
   if ( ansCont != NULL ) {
      uint32 status = static_cast< ReplyPacket* > ( ansCont->getPacket() )->
         getStatus();
      if ( status == StringTable::OK ) {
         uin = static_cast< AddUserReplyPacket* > (
            ansCont->getPacket() )->getUIN();
      } else if ( status == StringTable::NOT_UNIQUE ) {
         uin = 0;
      } else if ( status == StringTable::NOT_ALLOWED ) {
         // NOT_ALLOWED
         uin = MAX_UINT32 - 2;
      } else {
         // Error
         uin = MAX_UINT32-1;
      }
   }

   delete ansCont;
   delete req;

   return uin;
}


bool 
ParserThread::changeUser( UserUser* user, const UserUser* changerUser ) {
   return m_group->changeUser( user, changerUser, this );
}



int32 
ParserThread::changeUserPassword( const UserUser* user, 
                                  const char* newPassword, 
                                  const char* oldPassword, 
                                  bool checkPassword,
                                  const UserUser* changerUser )
{
   int32 status = 0;

   // Check old password first
   if ( checkPassword ) {
      uint32 UIN = authenticateUser( user->getLogonID(), oldPassword );
      if ( UIN == 0 || UIN == MAX_UINT32 ) {
         // Error, problem
         if ( UIN == 0 ) {
            // Old password not valid
            status = -3;
         } else {
            status = -2;
         }
         
         return status;
      }
   }

   // Change password
   SinglePacketRequest* req = new SinglePacketRequest( 
      getNextRequestID(), new PacketContainer(
         new ChangeUserPasswordRequestPacket(
            0, // PacketID
            0, // RequestID
            newPassword,
            user->getUIN(), changerUser ? changerUser->getUIN(): 0 ),
         0, 0, MODULE_TYPE_USER ) );

   mc2dbg8 << "changeUserPassword about to send "
           << "ChangeUserPasswordRequest" << endl;
   putRequest( req );
   PacketContainer* ansCont = req->getAnswer();
   mc2dbg8 << "changeUserPassword  "
           << "ChangeUserPasswordRequest returned." << endl;

   if ( ansCont != NULL ) {
      ChangeUserPasswordReplyPacket* ans = 
         static_cast< ChangeUserPasswordReplyPacket* > ( 
            ansCont->getPacket() );
      if ( ans->getStatus() != StringTable::OK ) {
         status = -1;
         mc2log << warn << "changeUserPassword Failed to "
                << "change user's password: "
                << StringTable::getString( 
                   StringTable::stringCode( ans->getStatus() ),
                   StringTable::ENGLISH ) << endl;
      } // else ok
   } else {
      status = -2;
      mc2log << warn << "changeUserPassword Timeout while "
             << "changing user's password." << endl;
   }

   delete req;
   delete ansCont;

   return status;
}


UserCellular* 
ParserThread::getUsersCellular( const UserUser* const user,
                                const char* const phonenumber ) const
{
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_CELLULAR ) ; 
         i++ )
   {
      UserCellular* cellular = static_cast< UserCellular* > (
         user->getElementOfType( 
            i, UserConstants::TYPE_CELLULAR ) );
      if ( strcmp( cellular->getPhoneNumber(), phonenumber ) == 0 ) {
         return cellular;
      }
   }

   return NULL;
}


uint32
ParserThread::removeAllUserLicenceKey( UserUser* user, 
                                       UserLicenceKey* userKey )
{
   uint32 nbrRemoved = 0;
   const byte* key = userKey->getLicenceKey();
   uint32 keyLength = userKey->getLicenceLength();
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_LICENCE_KEY ) ; ++i )
   {
      UserLicenceKey* licence = static_cast< UserLicenceKey* > ( 
         user->getElementOfType( i, UserConstants::TYPE_LICENCE_KEY ) );
      if ( (keyLength == licence->getLicenceLength() &&
            memcmp( key, licence->getLicenceKey(), keyLength ) == 0) &&
           userKey->getProduct() == licence->getProduct() &&
           userKey->getKeyType() == licence->getKeyType() )
      {
         licence->remove();
         nbrRemoved++;
      }
   } // End for all user's licences

   return nbrRemoved;
}


bool
ParserThread::getUserFromCellularNumber( const char* const number, 
                                         UserItem*& userItem,
                                         bool useCache, 
                                         bool wipeFromCache )
{
   UserCellular findCellular( MAX_UINT32 );
   findCellular.setPhoneNumber( number );
   uint32 nbrUsers = 0;

   return getUserFromUserElement( &findCellular, userItem, 
                                  nbrUsers, useCache, wipeFromCache );
}


bool 
ParserThread::getUserFromUserElement( const UserElement* elem,
                                      UserItem*& userItem, 
                                      uint32& nbrUsers, bool useCache, 
                                      bool wipeFromCache )
{
   bool ok = false;
   nbrUsers = 0;
   PacketContainer* cont = new PacketContainer( new FindUserRequestPacket(
      0, 0, const_cast< UserElement* >( elem ) ), 0, 0, MODULE_TYPE_USER );
   SinglePacketRequest* req = new SinglePacketRequest( 
      getNextRequestID(), cont );
   ThreadRequestContainer* reqCont = new ThreadRequestContainer( req );
   DEBUG8(cerr << "About to send FindUserRequest" << endl;);
   putRequest( reqCont );
   DEBUG8(cerr << "FindUserRequest returned" << endl;);
   
   PacketContainer* ansCont = req->getAnswer();
   
   delete req;
   delete reqCont;
   
   if ( ansCont != NULL ) {
      FindUserReplyPacket* findAns = static_cast< FindUserReplyPacket* > (
         ansCont->getPacket() );
      uint32* users = NULL;
      
      if ( findAns->getStatus() == StringTable::OK ) {
         nbrUsers = findAns->getNbrUsers();
         users = findAns->getUINs();

         if ( nbrUsers == 1 ) {
            ok = getUser( users[ 0 ], userItem, useCache, wipeFromCache );
         } else {
            DEBUG8(cerr << "ParserThread::getUser not one user " 
                   << nbrUsers << endl;);
            // User not found, but that is ok
            ok = true;
            userItem = NULL;
         }
         delete [] users;
      } else {
         DEBUG8(cerr << "ParserThread::getUser GetUser " 
                "GetUserPack not ok" << endl;);
      }
   } else {
      DEBUG8(cerr << "ParserThread::getUser FindUser no answer" 
             << endl;);
   }

   delete ansCont;
   
   return ok;   
}


uint32
ParserThread::authenticateUser( 
   const char* userName, 
   const char* userPasswd,
   bool checkExpired )
{
   return m_group->authenticateUser( userName, userPasswd, 
                                     checkExpired, this );
}



uint32
ParserThread::authenticateUserSession( 
   const char* sessionID, 
   const char* sessionKey,
   bool checkExpired )
{
   return m_group->authenticateUserSession( sessionID, sessionKey, 
                                            checkExpired, this );
}


bool 
ParserThread::checkUserRegionAccess( uint32 regionID, 
                                     const UserUser* const user, 
                                     UserEnums::URType urmask,
                                     bool checkEvenIfNotUsingRights )
{
   bool access = false;

   mc2dbg2 << "checkUserRegionAccess for " << user->getLogonID() 
          << " region " << regionID << endl;

   uint32 now = TimeUtility::getRealTime();
   if ( user->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 ) {
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
      {
         UserRight* r = static_cast<UserRight*> ( 
            user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
         if ( !r->isDeleted() &&
              (r->getUserRightType().levelAndServiceMatchMask( urmask ) ) ) 
         {
            if ( r->getRegionID() == regionID ||
                 r->getRegionID() == MAX_INT32 )
            {
               // Check date
               if ( r->checkAccessAt( now ) && validFor( r, getClientSetting() ) )
               {
                  access = true;
                  break;
               }
            } else if ( m_group->getRegionIDs()->isRegionGroupID( 
                           r->getRegionID() ) && 
                        !m_group->getRegionIDs()->isRegionGroupID( 
                           regionID ) ) 
            {
               vector<uint32> regionIDs;
               m_group->getRegionIDs()->addRegionIDsFor( 
                  regionIDs, r->getRegionID() );
               for ( uint32 i = 0 ; i < regionIDs.size() ; ++i ) {
                  if ( regionIDs[ i ] == regionID ) {
                     // Check date
                     if ( r->checkAccessAt( now ) &&
                          validFor( r, getClientSetting() ) ) {
                        access = true;
                        break;
                     }
                  }
               }
               if ( access ) {
                  break;
               }
            } // End else if regiongroup
         } // End if right URtype and not deleted
      } // End for all rights

   } else {
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_REGION_ACCESS ) ; 
         i++ )
   {
      const UserRegionAccess* region = static_cast< UserRegionAccess* > ( 
         user->getElementOfType( i, UserConstants::TYPE_REGION_ACCESS ) );
      mc2dbg4 << "checkUserRegionAccess user's region "
              << region->getRegionID() << endl;
      if ( region->getRegionID() == regionID ||
           region->getRegionID() == MAX_INT32 )
      {
         // Check date
         if ( region->getStartTime() <= now && 
              region->getEndTime() >= now )
         {
            access = true;
            break;
         }
      } else if ( m_group->getRegionIDs()->isRegionGroupID( 
                     region->getRegionID() ) && 
                  !m_group->getRegionIDs()->isRegionGroupID( regionID ) ) 
      {
         vector<uint32> regionIDs;
         m_group->getRegionIDs()->addRegionIDsFor( regionIDs, 
                                                   region->getRegionID() );
         for ( uint32 i = 0 ; i < regionIDs.size() ; ++i ) {
            if ( regionIDs[ i ] == regionID ) {
               // Check date
               if ( region->getStartTime() <= now && 
                    region->getEndTime() >= now )
               {
                  access = true;
                  break;
               }
            }
         }
         if ( access ) {
            break;
         }
      }
   }
   } // End else old region way

   if ( urmask.service() == UserEnums::UR_MYWAYFINDER ) {
      // As of MyWf 2.0 no MYWAYFINDER right is needed
      access = true;
   }

   return access;
}


bool 
ParserThread::getMapIdsForUserRegionAccess( 
   const UserUser* const user,
   RouteAllowedMap*& maps, 
   UserEnums::URType urmask )
{

   maps = new RouteAllowedMap;
   set<uint32>* mapIDs = NULL;
   uint32 now = TimeUtility::getRealTime();

   if ( !getMapIdsForUserRegionAccess( user, mapIDs, now, urmask ) ) {
      delete maps;
      maps = NULL;
      delete mapIDs;
      return false;
   }

   if ( mapIDs == NULL ) {
      // All maps
      delete maps;
      maps = NULL;
      return true;
   }
   const TopRegionRequest* topReg = m_group->getTopRegionRequest( this );

   if ( topReg == NULL ) {
      delete mapIDs;
      delete maps;
      return false;
   }
   const ItemIDTree& wholeTree = topReg->getWholeItemIDTree();

   // Fill the map.
   {
      vector<uint32> blockedItems;
      for ( set<uint32>::const_iterator it = mapIDs->begin() ;
            it != mapIDs->end() ; ++it ) {
         maps->insert( make_pair((*it), blockedItems ) );
      }
   }
   delete mapIDs;
   mapIDs = NULL;

   // All the valid region IDs
   vector< uint32 > regionIDs;
   bool allMaps = false;
   if ( !getAllValidRegionIDs( user, allMaps, regionIDs, now, urmask ) ) {
      return false;
   }

   if ( allMaps ) {
      return true;
   }

   // Blocking of parts of maps that isn't part of allowed regions
   for ( vector<uint32>::const_iterator sit = regionIDs.begin();
         sit != regionIDs.end();
         ++sit ) {
      const TopRegionMatch* topMatch = topReg->getTopRegionWithID( *sit );

      if ( topMatch == NULL ) {
         continue;
      }
      const ItemIDTree& tree = topMatch->getItemIDTree();
      set<uint32> mapIDs;
      tree.getTopLevelMapIDs( mapIDs );
      tree.getLowestLevelMapIDs( mapIDs );
         
      set<uint32>::iterator tit = mapIDs.begin();
      set<uint32>::iterator tItEnd = mapIDs.end();
      for ( ; tit != tItEnd; ++tit ) {
         // Higher maps for this map
         vector<uint32> higher;
         wholeTree.getOverviewMapsFor( (*tit), higher );
         for ( vector<uint32>::iterator hit = higher.begin(); 
               hit != higher.end() ; ++hit ) {
            mapIDs.insert( (*hit) );
         }
      }
         
      vector<uint32> randomMapIDs( mapIDs.begin(), mapIDs.end() );
      random_shuffle( randomMapIDs.begin(), randomMapIDs.end() );

      for ( vector<uint32>::const_iterator it = randomMapIDs.begin();
            it != randomMapIDs.end() ; ++it ) {
         if ( tree.wholeMap( (*it) ) ) {
            continue;
         }
         vector<uint32>& blockedItems = maps->find( (*it) )->second;
         // Add blocking items into blockedItems
         wholeTree.wholeMap( (*it) );
         vector<uint32> lower;
         wholeTree.getLowerMapsFor( (*it), lower );
         // shuffle so the ids can be sent by other threads while
         // this thread sends one.
         random_shuffle( lower.begin(), lower.end() );


         // holds all valid lower map ids which we 
         // should get edge nodes for.
         vector<uint32> lowerMapIDs;

         // for all lower check if in this region or any other 
         // region user have
         // if not then block all edgenodes of map
         for ( vector<uint32>::iterator lit = lower.begin(); 
               lit != lower.end() ; ++lit ) {
            if ( maps->find( (*lit) ) != maps->end() ) {
               continue;
            }
            lowerMapIDs.push_back( (*lit) );
         }
         // need some map ids to work with
         if ( lowerMapIDs.empty() ) {
            continue;
         }

         // get edge nodes for all map ids
         vector< uint32 > edgeNodes;
         StringTable::stringCode res = m_group->getEdgeNodesFor( lowerMapIDs, (*it), 
                                                                 edgeNodes, 
                                                                 this );
         // add edge nodes to blocked items 
         if ( res == StringTable::OK ) {
            for ( vector< uint32 >::const_iterator eit = 
                        edgeNodes.begin() ; 
                  eit != edgeNodes.end() ; ++eit ) {
               blockedItems.push_back( (*eit) ); 
            }
         } else {
            mc2log << warn << "ParserThread::"
                   << "getMapIdsForUserRegionAccess "
                   << " failed with status " 
                   << StringTable::getString( res, StringTable::ENGLISH ) 
                   << endl;
            delete maps;
            maps = NULL;
            return false;
         }

      } // End for all mapIDs for region

   } // End for all regionIDs

   // Print maps
#if 0
   mc2dbg4 << info << "Allowed maps: " << endl;
   if ( maps != NULL ) {
      for ( RouteAllowedMap::const_iterator it = maps->begin() ;
            it != maps->end() ; ++it )
      {
         mc2dbg4 << info << "   mapid 0x" << hex << it->first << dec;
         if ( it->second.size() > 0 ) {
            mc2dbg4 << " Blocked nodes: " << it->second.size() << endl;
            uint32 i = 0;
            for ( vector< uint32 >::const_iterator bit = 
                     it->second.begin() ;
                  bit != it->second.end() ; ++bit )
            {
               if ( i == 0 ) {
                  mc2dbg8 << info << "      ";
               }
               mc2dbg8 << "0x" << hex << (*bit) << dec;
               ++i;
               if ( i >= 4 ) {
                  i = 0;
                  mc2dbg8 << endl;
               } else {
                  mc2dbg8 << ", ";
               }
            }
            mc2dbg8 << endl;
         } else {
            mc2dbg4 << " No blocked nodes" << endl;
         }
      }
   } else {
      mc2dbg4 << info << "   maps is NULL all maps are allowed." << endl;
   }
#endif 

   return maps;
}


bool 
ParserThread::getMapIdsForUserRegionAccess( 
   const UserUser* const user, set<uint32>*& maps,
   uint32 now, UserEnums::URType urmask )
{
   const TopRegionRequest* topReg = m_group->getTopRegionRequest( this );

   if ( topReg == NULL ) {
      return false;
   }

   // All the valid region IDs
   vector< uint32 > regionIDs;
   bool allMaps = false;
   if ( !getAllValidRegionIDs( user, allMaps, regionIDs, now, urmask ) ) {
      return false;
   }

   if ( allMaps ) {
      return true;
   }

   maps = new set<uint32>;

   const ItemIDTree& wholeTree = topReg->getWholeItemIDTree();

   for ( vector<uint32>::const_iterator sit = regionIDs.begin();
         sit != regionIDs.end();
         ++sit ) {
      const TopRegionMatch* topMatch = topReg->getTopRegionWithID( *sit );

      if ( topMatch != NULL ) {
         const ItemIDTree& tree = topMatch->getItemIDTree();
         set<uint32> mapIDs;
         tree.getTopLevelMapIDs( mapIDs );
         tree.getLowestLevelMapIDs( mapIDs );

         for ( set<uint32>::const_iterator it = mapIDs.begin() ;
               it != mapIDs.end() ; ++it )
         {
            mc2dbg4 << "getMapIdsForUserRegionAccess mapID 0x" << hex 
                    << (*it) << dec
                    << " is ok using region " << topMatch->getName(
                       LangTypes::english ) 
                    << endl;
            maps->insert( (*it) );

            // Higher maps for this map
            vector<uint32> higher;
            wholeTree.getOverviewMapsFor( (*it), higher );
            for ( vector<uint32>::iterator hit = higher.begin() ; 
                  hit != higher.end() ; ++hit )
            {
               maps->insert( (*hit) );
               mc2dbg4 << "getMapIdsForUserRegionAccess higher mapID 0x"
                       << hex << (*hit) << dec
                       << " is ok using region " << topMatch->getName(
                          LangTypes::english ) 
                       << endl;
            }
         }
            
      } else {
         mc2log << info << "ParserThread::getMapIdsForUserRegionAccess "
                << "user has unknown region with id: " 
                << *sit << endl;
      }
   }

   return true;
}


bool
ParserThread::getAllValidRegionIDs( 
   const UserUser* const user, bool& allMaps,
   vector< uint32 >& regionIDs,
   uint32 now, UserEnums::URType urmask )
{
   // Check if there is a need to check for region access.
   if ( getClientSetting() != NULL ) {
      if ( ! getClientSetting()->usesRights() ) {
         // no rigths needed
         allMaps = true;
         return true;
      }
   }

   if ( urmask.service() == UserEnums::UR_MYWAYFINDER ) {
      // As of MyWf 2.0 no MYWAYFINDER right is needed
      allMaps = true;
      return true;
   }

   const TopRegionRequest* topReg = m_group->getTopRegionRequest( this );

   if ( topReg == NULL ) {
      return false;
   }

   regionIDs.reserve( 1024 );
  
   if ( user->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 ) {
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
      {
         UserRight* r = static_cast<UserRight*> ( 
            user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
         if ( !r->isDeleted() &&
              (r->getUserRightType().levelAndServiceMatchMask( urmask ) ) )
         {
            // Check date
            if ( r->checkAccessAt( now ) &&
                 validFor( r, getClientSetting() ) ) {
               if ( r->getRegionID() == MAX_INT32 ) {
                  // All maps!
                  allMaps = true;
                  return true;
               } else if ( m_group->getRegionIDs()->isRegionGroupID( 
                              r->getRegionID() ) ) 
               {
                  m_group->getRegionIDs()->addRegionIDsForReq( 
                     regionIDs, r->getRegionID(), topReg );
               } else {
                  regionIDs.push_back( r->getRegionID() );
               }
            } // End if vald now
         } // End if right URtype
      } // End for all rights

   } else {
      // OOOLD. Only on whopper and maybe some other places.
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_REGION_ACCESS ) ; 
            i++ )
      {
         UserRegionAccess* access = static_cast< UserRegionAccess* > ( 
            user->getElementOfType( i, 
                                    UserConstants::TYPE_REGION_ACCESS ) );
         
         // Check date
         if ( access->getStartTime() <= now && 
              access->getEndTime() >= now )
         {
            if ( access->getRegionID() == MAX_INT32 ) {
               // All maps!
               allMaps = true;
               return true;
            } else if ( m_group->getRegionIDs()->isRegionGroupID( 
                           access->getRegionID() ) ) 
            {
               m_group->getRegionIDs()->addRegionIDsForReq( 
                  regionIDs, access->getRegionID(), topReg );
            } else {
               regionIDs.push_back( access->getRegionID() );
            }
         } // End region access is valid now      
      } // For all UserRegionAccess
   } // End else use regions

   // Remove redundant id:s for less work afterwards.
   std::sort( regionIDs.begin(), regionIDs.end() );
   vector<uint32>::iterator last =
      std::unique( regionIDs.begin(), regionIDs.end() );
   // And remove.
   regionIDs.resize( distance ( regionIDs.begin(), last ) );
      
   return true;
}


bool
ParserThread::checkAccessToService( 
   const UserUser* const user,
   UserEnums::userRightService service,
   UserEnums::userRightLevel levelmask,
   uint32 regionID,
   bool checkTime,
   bool isSingleMultiBitService ) const
{
   if ( user->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 ) {
      UserEnums::URType urmask( levelmask, service );
      uint32 now = TimeUtility::getRealTime();
   
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
      {
         UserRight* r = static_cast<UserRight*> ( 
            user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
         if ( !r->isDeleted() && 
              (isSingleMultiBitService ? 
               r->getUserRightType().levelAndServiceMatch( urmask )  : 
               r->getUserRightType().levelAndServiceMatchMask( urmask ) ) )
         {
            // Check region, if is contained etc.
            bool regionMatches = false;
            if ( (r->getRegionID() == regionID || regionID == MAX_UINT32) ) {
               regionMatches = true;
            } else if ( m_group->getRegionIDs()->isRegionGroupID( 
                           r->getRegionID() ) && 
                        !m_group->getRegionIDs()->isRegionGroupID( 
                           regionID ) ) {
               regionMatches = m_group->getRegionIDs()->isTheRegionInGroup(
                  r->getRegionID(), regionID );
            }
            if ( regionMatches ) {
               // Check date
               if ( validFor( r, getClientSetting() ) && 
                    (!checkTime || r->checkAccessAt( now )) ) {
                  // Access!
                  return true;
               } // End if vald now
            }
         } // End if right URtype
      } // End for all rights
   } else {
      // No rights check old XXX: Remove when users have been transformed
      switch(service){
         case UserEnums::UR_WF :
            return user->getNavService();
         case UserEnums::UR_MYWAYFINDER :
            return true; // As of MyWf 2.0 no MYWAYFINDER right is needed
         case UserEnums::UR_MAPDL_PREGEN :
            return user->getHTMLService() || user->getNavService();
         case UserEnums::UR_MAPDL_CUSTOM :
            return user->getHTMLService() || user->getNavService();
         case UserEnums::UR_XML :
            return user->getExternalXmlService();
         case UserEnums::UR_TRAFFIC :
            return user->getHTMLService() || user->getNavService() ||
               user->getExternalXmlService();
         case UserEnums::UR_SPEEDCAM :
            return user->getHTMLService() || user->getNavService() ||
               user->getExternalXmlService();
         default:
            return false;
      };
   }

   if ( service == UserEnums::UR_MYWAYFINDER ) {
      // As of MyWf 2.0 no MYWAYFINDER right is needed
      return true;
   }
      
   // None found
   return false;
}


void
ParserThread::getMatchingRights( vector<UserRight*>& rights,
                                 const UserUser* user,
                                 UserEnums::URType type ) const
{
   for ( uint32 i = 0 ; 
         i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
   {
      UserRight* r = static_cast<UserRight*> ( 
         user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
      if ( !r->isDeleted() && r->getUserRightType() == type &&
           validFor( r, getClientSetting() ) ) {
         rights.push_back( r );
      }
   }
}


bool
ParserThread::checkIfIronClient( const ClientSetting* clientSetting ) const
{
   return clientSetting->getCreateLevel() == WFSubscriptionConstants::IRON;
}


bool
ParserThread::checkIfIronUserMay( 
   const UserUser* user, 
   const ClientSetting* clientSetting,
   UserEnums::userRightService may ) const
{
   typedef UserEnums UE;
   bool res = false;
   if ( checkIfIronClient( clientSetting ) ) {
      res = checkAccessToService( 
         user, may, UE::UR_NO_LEVEL, MAX_UINT32, true, true );
   } else { // All other levels
      res = true;
   }

   return res;
}


bool
ParserThread::checkIfLithiumClient( const ClientSetting* clientSetting ) const
{
   return ( strstr( clientSetting->getClientType(), "-lithium" ) != NULL );
}


/// The time a cached service check is valid, in seconds
const uint32 cachedServiceValidTime = 30;

bool 
ParserThread::hasCachedServiceCheck( 
   OperationType::operationType operationType,
   uint32 topRegionID,
   int& checkRes ) {
   bool foundCached = false;
   uint32 now = TimeUtility::getRealTime();
   MC2String id( boost::lexical_cast< MC2String > ( topRegionID ) + "," +
                 boost::lexical_cast< MC2String > ( operationType ) );
   uint32 foundTime = 0;
   // Check the UserIDKey with type service_id_and_time
   UserUser::constUserElRange_t els = getCurrentUser()->getUser()->
      getElementRange( UserConstants::TYPE_ID_KEY );
   for ( UserUser::userElVect_t::const_iterator it = els.first ; 
         it != els.second ; ++it )
   {
      UserIDKey* el = static_cast< UserIDKey* > ( *it );
      if ( el->getIDType() == UserIDKey::service_id_and_time ) {
         uint32 time = 0;
         MC2String key;
         uint32 res = 0;
         if ( ParserUserHandler::getServiceIdResAndTime( 
                 el, time, key, res ) ) {
            // Check for match and time
            if ( key == id && time + cachedServiceValidTime > now &&
                 time > foundTime ) {
               foundCached = true;
               checkRes = res;
               foundTime = time;
               // No break to get latest
            }
         }
      }
   } // End for all UserIDKeys in current user

   return foundCached;
}

void 
updateUserWithNewServiceCheck( UserUser& cuser, uint32 topRegionID,
                               OperationType::operationType operationType,
                               int res ) {
   // Remove any expired cached service checks
   uint32 now = TimeUtility::getRealTime();
   UserUser::userElRange_t els = cuser.getElementRange( 
      UserConstants::TYPE_ID_KEY );
   for ( UserUser::userElVect_t::iterator it = els.first ; 
         it != els.second ; ++it )
   {
      UserIDKey* el = static_cast< UserIDKey* > ( *it );
      if ( el->getIDType() == UserIDKey::service_id_and_time ) {
         uint32 time = 0;
         MC2String key;
         uint32 res = 0;
         if ( ParserUserHandler::getServiceIdResAndTime(
                 el, time, key, res ) ) {
            // Check for old service Ids
            if ( time + cachedServiceValidTime < now ) {
               // Too old
               el->remove();
            }
         }
      }
   } // End for all UserIDKeys in current user
   // Also add an IdKey of type service_id_and_time
   MC2String key( boost::lexical_cast< MC2String > ( topRegionID ) + ","+
                  boost::lexical_cast< MC2String > ( operationType ) + 
                  ":" +
                  boost::lexical_cast< MC2String > ( now ) + ":" +
                  boost::lexical_cast< MC2String > ( res ) );
   cuser.addElement( 
      new UserIDKey( 0, key, 
                     UserIDKey::service_id_and_time ) );
}

bool
ParserThread::checkService( const ClientSetting* clientSetting,
                            const HttpInterfaceRequest* httpRequest,
                            OperationType::operationType operationType,
                            PurchaseOptions& purchaseOptions,
                            set< MC2String >& checkedServiceIDs,
                            const MC2Coordinate& coord,
                            uint32 topRegionID,
                            LangTypes::language_t clientLang,
                            bool mayUseCachedCheckResult ) 
{
   if ( clientSetting == NULL  ) {
      mc2log << warn << "[PT]:checkService clientSetting is NULL." << endl;
      return true;
   }

   if ( ! clientSetting->isAppStoreClient() ) {
      // No service to check for this client type, so let the client go on as
      // usual
      mc2log << info << "[PT]:checkService Not external access client, "
             << "go on as usual." << endl;
      return true;
   }

   if ( httpRequest == NULL ) {
      mc2log << error << "[PT]:checkService httpRequest is NULL." << endl;
      return false;
   }

   MC2String errorReason;
   int res = internalCheckService( 
      clientSetting, httpRequest, operationType, purchaseOptions, 
      checkedServiceIDs, coord, topRegionID, clientLang,
      mayUseCachedCheckResult, errorReason );

   // Check if it is worth printing status
   if ( res != 2 && res != 1 && 
        !(mayUseCachedCheckResult && 
          purchaseOptions.getErrorDescription().empty()) ) {
      mc2log << "[PT]:checkService Status ";
      if ( ! purchaseOptions.getErrorDescription().empty() ) {
         mc2log << purchaseOptions.getErrorDescription();
      } else {
         mc2log << errorReason;
      }
      mc2log << endl;
   }

   bool access = (res == 0 || res == 1 || res == 2);

   return access;
}


namespace {

/**
 * Create an error message and code for bad top region.
 * @param coord The coordinate for top region.
 * @return pair ( error string, error code )
 */
pair< MC2String, int > createBadTopRegionError( const MC2Coordinate& coord ) {
   mc2log << error << "[PT]:checkService Can not find top region location"
          << " for the service requested" << endl;
   stringstream err;
   err << "No top region for coordinate: " << coord;
   return make_pair( MC2String( err.str() ), -2 );
}

} // anonymous

int
ParserThread::internalCheckService( 
   const ClientSetting* clientSetting,
   const HttpInterfaceRequest* httpRequest,
   OperationType::operationType operationType,
   PurchaseOptions& purchaseOptions,
   set< MC2String >& checkedServiceIDs,
   const MC2Coordinate& coord,
   uint32& topRegionID,
   LangType::language_t clientLang,
   bool mayUseCachedCheckResult,
   MC2String& errorReason ) 
{
   MC2_ASSERT( clientSetting != NULL );
   MC2_ASSERT( httpRequest != NULL );

   // Get the "user location" as a top region id
   if ( topRegionID == MAX_UINT32 ) {
      if ( coord.isValid() ) {
         topRegionID = PositionInfo::
            getTopRegionFromPosition( *this,
                                      getTopRegionRequest(),
                                      coord );
      }

      if ( topRegionID == MAX_UINT32 ) {
         pair< MC2String, int > retErr = ::createBadTopRegionError( coord );
         errorReason = retErr.first;
         return retErr.second;
      }
   }

   // Case for iPhone billing
   if ( clientSetting->isAppStoreClient() ) {
      bool appStore = getAppStore().checkService( 
         clientSetting, httpRequest, operationType, purchaseOptions,
         checkedServiceIDs, topRegionID, clientLang, 
         mayUseCachedCheckResult );
      if ( appStore ) {
         // the user has the sufficent rights
         return 0;
      }
      // the user has no access and need to purchase in App Store
      return 3;
   }

   // Other external access checks can be added here

   return 0;
}

bool
ParserThread::
checkIfWolframClient( const ClientSetting* clientSetting ) const {
   return clientSetting->getCreateLevel() == WFSubscriptionConstants::WOLFRAM;
}

void 
ParserThread::releaseUserItem( UserItem* userItem ) {
   return m_group->releaseUserItem( userItem );
}


RequestData
ParserThread::getNextRequestID() {
   const UserUser* user =
      getCurrentUser() ? getCurrentUser()->getUser() : NULL;
   return RequestData(m_group->getNextRequestID(), user );
}

UserEnums::userRightLevel
ParserThread::getUrLevel( const ClientSetting* clientSetting ) const {
   if ( checkIfIronClient( clientSetting ) ) {
      return UserEnums::UR_IRON;
   } else if ( checkIfLithiumClient( clientSetting ) ) {
      return UserEnums::TL_MASK; 
   } else if ( checkIfWolframClient( clientSetting ) ) {
      return UserEnums::TW_MASK;
   } else {
      return UserEnums::TSG_MASK;
   }
}

bool 
ParserThread::getAndChangeTransactions( uint32 UIN, 
                                        int32 transactionChange,
                                        int32& nbrTransactions )
{
   TransactionRequestPacket* p = new TransactionRequestPacket( 
      UIN, transactionChange > 0 ? TransactionRequestPacket::increase : 
      TransactionRequestPacket::decrease, 
      transactionChange > 0 ? transactionChange : -transactionChange );
   SinglePacketRequest* req = new SinglePacketRequest( 
      getNextRequestID(), 
      new PacketContainer( p,
                           0,
                           0,
                           MODULE_TYPE_USER ) );
   putRequest( req );
   PacketContainer* answerCont = req->getAnswer();

   bool ok = true;
   if ( answerCont != NULL &&
        static_cast< ReplyPacket* >( answerCont->getPacket() )->getStatus()
        == StringTable::OK )
   {
      nbrTransactions = static_cast< TransactionReplyPacket* > ( 
         answerCont->getPacket() )->getNbrTransactions();
      ok = true;
   } else {
      ok = false;
   }

   delete answerCont;
   delete req;

   return ok;
}


StringTable::stringCode 
ParserThread::getAndChangeTransactionDays( 
   uint32 UIN, bool check, int32& nbrTransactionDays, 
   uint32& curTime )
{
   StringTable::stringCode status = StringTable::OK;
   TransactionDaysRequestPacket* p = new TransactionDaysRequestPacket(
      UIN, check, nbrTransactionDays );
   SinglePacketRequest* req = new SinglePacketRequest( 
      getNextRequestID(), 
      new PacketContainer( p, 0, 0, MODULE_TYPE_USER ) );
   putRequest( req );
   PacketContainer* answerCont = req->getAnswer();

   if ( answerCont != NULL ) {
      TransactionDaysReplyPacket* r = static_cast< 
         TransactionDaysReplyPacket* > ( answerCont->getPacket() );
      nbrTransactionDays = r->getNbrTransactionDays();
      curTime = r->getCurrStartTime();
      status = StringTable::stringCode( r->getStatus() );
   } else {
      status = StringTable::TIMEOUT_ERROR;
   }

   delete answerCont;
   delete req;

   return status;
}


WFSubscriptionConstants::subscriptionsTypes 
ParserThread::getSubscriptionTypeForUser( 
   const UserUser* user, UserEnums::userRightService urservice,
   bool onlyActive, UserEnums::userRightLevel urlevel ) const
{
   byte userWFType = MAX_BYTE;
   uint32 now = TimeUtility::getRealTime();

   if ( user->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 ) {
      // Use rights
      int32 type = -1;
      bool hasIron = false;
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; i++ ) 
      {
         UserRight* r = static_cast<UserRight*> ( 
            user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
         if ( (r->getUserRightType().service() & urservice) && 
              (r->getUserRightType().level() & urlevel) ) {
            byte level = r->getUserRightType().getLevelAsWFST();
            // Check date and not deleted
            if ( (!onlyActive || r->checkAccessAt( now )) && 
                 !r->isDeleted() && validFor( r, getClientSetting() ) )
            {
               if ( level != MAX_BYTE && int32(level) > type &&
                    (level != WFSubscriptionConstants::IRON) )
               {
                  type = level;
                  userWFType = level;
               }
               if ( level == WFSubscriptionConstants::IRON ) {
                  hasIron = true;
               }
            }
         } // End if WF right
      } // End for all rights
      if ( type == -1 && hasIron ) {
         if ( StringUtility::validEmailAddress( user->getEmailAddress() ) )
         {
            type = WFSubscriptionConstants::IRON;
         } else {
            type = WFSubscriptionConstants::TRIAL;
         }
         userWFType = type;
      }
   } else { 
      // Use old way
      if ( user->getNbrOfType( UserConstants::TYPE_WAYFINDER_SUBSCRIPTION )
           > 0  )
      {
         userWFType = static_cast< UserWayfinderSubscription*> ( 
            user->getElementOfType( 
               0, UserConstants::TYPE_WAYFINDER_SUBSCRIPTION ) )
            ->getWayfinderType();
      }
   }
   return WFSubscriptionConstants::subscriptionsTypes( userWFType );
}

uint32 
ParserThread::getPeerIP()
{
   ISABSync sync(m_monitor);

   return (m_peerIP);
}


StringTable::stringCode 
ParserThread::getEdgeNodesFor( 
   uint32 mapID, uint32 overMapID, vector<uint32>& externalNodes )
{
   return m_group->getEdgeNodesFor( mapID, overMapID, externalNodes, 
                                    this );
}

void
ParserThread::setLogPrefix( const char* prefix ) {
   ScopedArray<char> oldPrefix( m_logPrefix.release() ); //delay delete of this array
   m_logPrefix.reset( StringUtility::newStrDup( prefix ) );
   static_cast<LogBuffer*>(mc2log.rdbuf())->setPrefix( m_logPrefix.get() );
}

const char*
ParserThread::getLogPrefix() const {
   return m_logPrefix.get();
}


const TopRegionRequest*
ParserThread::getTopRegionRequest() 
{
   return m_group->getTopRegionRequest(this);
}

ParserThreadGroupHandle
ParserThread::getGroup() const {
   return m_group;
}

DataBuffer*
ParserThread::getTileMaps( const vector<MC2SimpleString>& params,
                           uint32 startOffset,
                           uint32 maxBytes )
{
   return m_tileHandler->getTileMaps( params,
                                      startOffset,
                                      maxBytes );
}

DataBuffer*
ParserThread::getTileMap( const char* paramStr )
{
   vector<MC2SimpleString> params;
   params.push_back( paramStr );
   TileMapBufferQuery query( params, MAX_UINT32 );
   
   getTileMaps( query );
   
   const SharedBuffer* buf = query.getSingleBuffer( paramStr );
   if ( buf == NULL || buf->getBufferSize() == 0 ) {
      return NULL;
   } else {
      DataBuffer* tmpBuf = new DataBuffer( *buf );
      tmpBuf->reset();
      tmpBuf->readPastBytes( buf->getBufferSize() );
      return tmpBuf;
   }
}

void
ParserThread::getTileMaps( TileMapQuery& query )
{
   return m_tileHandler->getTileMaps( query );
}


std::set<uint32>::size_type
ParserThread::getMapsFromCoordinate(std::set<uint32>& maps, 
                                    bool underview, bool country, 
                                    const MC2Coordinate coord, int radius)
{
   return getMapsFromBBox( maps, underview, country, 
                           MC2BoundingBox(coord, radius) );
}

void
ParserThread::pushBBoxRequestPacketContainer(std::vector<PacketContainer*>& cont,
                                             const MC2BoundingBox& bbox, 
                                             bool underview, bool country,
                                             uint32 mapset)
{
   BBoxReqPacketData reqData( bbox, underview, country);
   std::auto_ptr<BBoxRequestPacket> req( new BBoxRequestPacket( reqData ) );
   std::auto_ptr<PacketContainer> packet(
         new PacketContainer( req.get(), 0, 0, MODULE_TYPE_MAP,
                              PacketContainer::defaultResendTimeoutTime,
                              PacketContainer::defaultResends, 
                              mapset ) );
   req.release(); //I think this is the first safe place to release the ptr
   cont.push_back(packet.get());
   packet.release(); //I think this is the first safe place to release the ptr
}

std::set<uint32>::size_type
ParserThread::getMapsFromBBox(std::set<uint32>& maps, 
                              bool underview, bool country, 
                              const MC2BoundingBox& bbox)
{
   //Find the number of map sets. We need to send the packet once for
   //each mapset.  If the property is not set, we will get MAX_UINT32.
   const uint32 mapSetCount = 
      Properties::getUint32Property("MAP_SET_COUNT", MAX_UINT32);

   vector< PacketContainer* > reqs;
   reqs.reserve( (mapSetCount == MAX_UINT32) ? 1 : mapSetCount );

   if(mapSetCount == MAX_UINT32){
      //Create the request packet
      pushBBoxRequestPacketContainer(reqs, bbox, 
                                     underview, country, mapSetCount);
   } else {
      for(uint32 mapSet = 0; mapSet < mapSetCount; ++mapSet){
         //Create the request packet
         pushBBoxRequestPacketContainer(reqs, bbox, 
                                        underview, country, mapSet);
      }
   }
   //multipacketrequest.
   //this function replaces the requests with the replies
   putRequest( reqs );

   const std::set<uint32>::size_type in_size = maps.size();

   //extract the map ids
   if(! reqs.empty() ){
      for( vector<PacketContainer*>::iterator it = reqs.begin(); 
           it != reqs.end(); ++it){
         if ( *it != NULL ) {
            BBoxReplyPacket* reply = 
               static_cast< BBoxReplyPacket* >( (*it)->getPacket() );
            //Extract the map ids (of the maps that overlap the sent
            //boundingbox) into the mapIDs vector.
            MC2BoundingBox bbox;
            vector<uint32> tmp;
            reply->get( bbox, tmp ); //all previous vector contents are lost
            maps.insert( tmp.begin(), tmp.end() );
            // Add the overview maps if country is true
            if( country ){
               for( vector<uint32>::const_iterator tmpIt = tmp.begin(); 
                     tmpIt != tmp.end(); ++tmpIt ) {
                  if( MapBits::isCountryMap( *tmpIt ) ){
                     maps.insert( MapBits::countryToOverview( *tmpIt ) );
                  }
               }
            }
            delete *it;
            *it = NULL;
         } else {
            mc2log << warn << "[PT] getMapsFromBBox bbox reply packet empty" 
                   << endl;
         }
      }
   }
   return maps.size() - in_size;
}


SearchParserHandler& ParserThread::getSearchHandler() {
   SearchHeadingManager& smanager = m_group->getSearchHeadingManager();

   // If we do not yet have a search handler or if the crc does not match
   // then we should request new headings from the heading manager and
   // recreate the search parser handler.
   if ( m_searchHandler.get() == NULL ||
        smanager.shouldUpdateSearchHeadings( m_searchHeadingCRC ) ) {
      CompactSearchHitTypeVector headings;
      uint32 oldCRC =  m_searchHeadingCRC;
      smanager.getSearchHeadings( *this, headings, m_searchHeadingCRC );
      
      if( m_searchHandler.get() == NULL || m_searchHeadingCRC != oldCRC ) {
         // We need to recreate the SearchParserHandler
         m_searchHandler.
            reset( new SearchParserHandler( const_cast< ParserThread* >( this ),
                                            m_group.get(),
                                            headings ) );
      }
   }

   return *m_searchHandler;
}

const SearchParserHandler& ParserThread::getSearchHandler() const {
   // call the other getSearchHandler
   return const_cast<SearchParserHandler&>
      ( const_cast<ParserThread&>( *this ).getSearchHandler() );
}


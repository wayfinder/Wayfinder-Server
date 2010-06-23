/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavRouteHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUtil.h"
#include "SearchMatch.h"
#include "RouteRequest.h"
#include "NavUserHelp.h"
#include "RouteID.h"
#include "NavParserThreadGroup.h"
#include "RouteList.h"
#include "isabRouteList.h"
#include "isabRouteElement.h"
#include "isabBoxSession.h"
#include "ExpandedRoute.h"
#include "MC2Coordinate.h"
#include "isabBoxRouteMessage.h"
#include "isabBoxSession.h"
#include "NavAddress.h"
#include "ExpandRoutePacket.h"
#include "RoutePacket.h"
#include "RequestUserData.h"
#include "ParserRouteHandler.h"
#include "RouteRequestParams.h"
#include "UnexpandedRoute.h"
#include "SubRouteVector.h"
#include "BitUtility.h"
#include "TimeUtility.h"
#include "NavRequestData.h"
#include "ParserDebitHandler.h"
#include "TopRegionRequest.h"
#include "ClientSettings.h"
#include "PurchaseOptions.h"
#include "NavParamHelper.h"

#include "HttpInterfaceRequest.h"
#include "IP.h"

#include <set>

NavRouteHandler::NavRouteHandler( InterfaceParserThread* thread,
                                  NavParserThreadGroup* group,
                                  NavUserHelp* userHelp )
      : NavHandler( thread, group ), m_userHelp( userHelp )
{
   m_expectations.push_back( 
      ParamExpectation( 1000, NParam::Int32_array, 8, 10 ) );
   m_expectations.push_back( 
      ParamExpectation( 1001, NParam::Int32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 1002, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 1004, NParam::Uint16 ) );
   m_expectations.push_back( ParamExpectation( 1005, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 1006, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1007, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1008, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1009, NParam::Bool ) );
   m_expectations.push_back( ParamExpectation( 1010, NParam::Uint32 ) );
   m_expectations.push_back( 
      ParamExpectation( 1011, NParam::Int32_array, 8, 8 ) );
   m_expectations.push_back( 
      ParamExpectation( 1012, NParam::Int32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 1013, NParam::Byte ) );
}


bool
NavRouteHandler::handleRoute( NavRequestData& rd ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }
   bool ok = true;

   // The user
   UserUser* user = rd.session->getUser()->getUser();
   uint32 startTime = TimeUtility::getCurrentTime();
   MC2Coordinate pos;

   std::set< MC2Coordinate > allRoutePoints;
   std::set< uint32 > allMapIDs;

   // Start parameter printing
   mc2log << info << "handleRoute:";

   StringTable::languageCode language = user->getLanguage();
   if ( rd.params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         rd.params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
//   LangTypes::language_t lang = ItemTypes::getLanguageCodeAsLanguageType( 
//      language );
   bool anyOrig = false;
   bool anyDest = false;
   RouteRequest* rr = new RouteRequest( rd.session->getUser()->getUser(),
                                        m_thread->getNextRequestID(),
                                        (ROUTE_TYPE_STRING |
                                         ROUTE_TYPE_NAVIGATOR |
                                         ROUTE_TYPE_ITEM_STRING |
                                         ROUTE_TYPE_GFX),
                                        language, false, 0,
                                        m_thread->getTopRegionRequest());
   
   bool useGPS = true;

   //origin coord and angle
   if ( rd.params.getParam( 1000 ) ) {
      vector< const NParam* > origs;
      rd.params.getAllParams( 1000, origs );
      for ( uint32 i = 0 ; i < origs.size() ; ++i ) {
         MC2Coordinate orig;
         uint16 angle = MAX_UINT16;
         NavParamHelper::getCoordAndAngle( origs[ i ], orig, angle );
         mc2log << " Orig " << orig;
         if ( angle <= 360 ) {
            mc2log << "," << angle << "°";
         }
         
         if ( orig.isValid() ) {
            anyOrig = true;
            rr->addOriginCoord( orig.lat, orig.lon, angle ); //<=
            pos = orig;
            //allRoutePoints.insert( orig );
            if(angle == MAX_UINT16){
               useGPS = false;
            }
         } else {
            mc2log << " INVALID! ";
         }
      }
   }
   
   //destination coord
   if ( rd.params.getParam( 1001 ) ) {
      vector< const NParam* > dests;
      rd.params.getAllParams( 1001, dests );
      for ( uint32 i = 0 ; i < dests.size() ; ++i ) {
         int32 lat = dests[ i ]->getInt32Array( 0 );
         int32 lon = dests[ i ]->getInt32Array( 1 );
         MC2Coordinate dest( Nav2Coordinate( 
                                lat, lon ) );
         mc2log << " Dest " << dest;
         if ( dest.isValid() ) {
            anyDest = true;
            rr->addDestinationCoord( dest.lat, dest.lon ); //<=
            allRoutePoints.insert( dest );
         } else {
            mc2log << " INVALID! ";
         }
      }
   }
   
   //time to trunc
   uint32 timeToTrunc = MAX_UINT32;
   if ( rd.params.getParam( 1002 ) ) {
      timeToTrunc = rd.params.getParam( 1002 )->getUint32();
      mc2log << " trunkTime " << timeToTrunc;
   }

   //Current velocity
   float32 currVel = 666.999;
   if ( rd.params.getParam( 1004 ) ) {
      if ( rd.params.getParam( 1004 )->getUint16() < MAX_INT16 ) {
         currVel = float32(rd.params.getParam( 1004 )->getUint16())/32;
         mc2log << " speed " << currVel;
      } else {
         mc2log << " speed N/A";
      }
   }

   //Route flags
   RouteTypes::routeCostType routeCost = RouteTypes::TIME;
   ItemTypes::vehicle_t routeVehicle = ItemTypes::passengerCar;
   bool addLandmarks = false;
   bool avoidTollRoads = false;
   bool avoidHighway = false;
   bool abbreviate = true;
   byte routeContent = 2; // Full, 1 is slim
   if ( rd.params.getParam( 1005 ) ) {
      const NParam* p = rd.params.getParam( 1005 );
      uint32 flags = p->getUint32();
      routeCost = RouteTypes::costToRouteCostType( 
         BitUtility::getBit( flags, 0 ), BitUtility::getBit( flags, 1 ),
         BitUtility::getBit( flags, 2 ), BitUtility::getBit( flags, 3 ) );
      avoidHighway   = BitUtility::getBit( flags, 4 ) != 0;
      avoidTollRoads = BitUtility::getBit( flags, 5 ) != 0;
      abbreviate     = BitUtility::getBit( flags, 6 ) != 0;
      addLandmarks   = BitUtility::getBit( flags, 7 ) != 0;
      routeContent = (flags >> 8) & 0xf;
      routeVehicle = mc2Vehicle( (flags >> 16) & 0xf );
      mc2log << " RC " << RouteTypes::routeCostTypeToString( routeCost )
             << " AH " << avoidHighway << " AT " << avoidTollRoads
             << " AL " << addLandmarks << " AB " << abbreviate 
             << " CO " << int(routeContent) << " RV " 
             << StringTable::getString( 
                ItemTypes::getVehicleSC( routeVehicle ),
                StringTable::ENGLISH );
   }

   //old route id
   MC2String oldRouteIDStr;
   RouteID oldRouteID( 0, 0 );
   if ( rd.params.getParam( 1006 ) ) {
      oldRouteIDStr = rd.params.getParam( 1006 )->getString(
         m_thread->clientUsesLatin1());
      oldRouteID = RouteID( oldRouteIDStr.c_str() );
      mc2log << " oldRouteID " << oldRouteIDStr;
      if ( oldRouteID.isValid() == 0 ) {
         mc2log << " INVALID";
      }
   }
   
   //search id route origin
   if ( rd.params.getParam( 1007 ) ) {
      vector< const NParam* > origs;
      rd.params.getAllParams( 1007, origs );
      for ( uint32 i = 0 ; i < origs.size() ; ++i ) {
         MC2String id = origs[ i ]->getString(m_thread->clientUsesLatin1());
         SearchMatch* match = SearchMatch::createMatch( id.c_str() );
         if ( match == NULL ) {
            mc2log << " Failed to make out origin id " << MC2CITE( id );
         } else if ( match->getID().isValid() ) {
            mc2log << " originID " << id;
            anyOrig = true;
            uint16 offset = 0;
            if ( dynamic_cast<VanillaStreetMatch*>( match ) ) {
               offset = static_cast<VanillaStreetMatch*>( match )
                  ->getOffset();
            }
            rr->addOriginID( match->getMapID(), match->getItemID(), //<=
                             offset );
            //allMapIDs.insert( match->getMapID() );
         } else if ( match->getCoords().isValid() ) {
            mc2log << " originCoord "  << id;
            anyOrig = true;
            rr->addOriginCoord( match->getCoords().lat,            //<=
                                match->getCoords().lon );
            //allRoutePoints.insert( match->getCoords() );
         } else {
            mc2log << " Not id nor coords valid for orig id " 
                   << MC2CITE( origs[ i ]->getString(
                                  m_thread->clientUsesLatin1()) );
         }
         delete match;
      } // End for all 1007s
   }
   
   //search id route destination
   if ( rd.params.getParam( 1008 ) ) {
      vector< const NParam* > dests;
      rd.params.getAllParams( 1008, dests );
      for ( uint32 i = 0 ; i < dests.size() ; ++i ) {
         MC2String id = dests[ i ]->getString(m_thread->clientUsesLatin1());
         SearchMatch* match = SearchMatch::createMatch( id.c_str() );
         if ( match == NULL ) {
            mc2log << " Failed to make out destination id " 
                   << MC2CITE( id );
         } else if ( match->getID().isValid() ) {
            mc2log << " destinationID " << id;
            anyDest = true;
            uint16 offset = 0;
            if ( dynamic_cast<VanillaStreetMatch*>( match ) ) {
               offset = static_cast<VanillaStreetMatch*>( match )
                  ->getOffset();
            }
            rr->addDestinationID( match->getMapID(), match->getItemID(), //<=
                                  offset );
            allMapIDs.insert( match->getMapID() );
         } else if ( match->getCoords().isValid() ) {
            mc2log << " destinationCoord "  << id;
            anyDest = true;
            rr->addDestinationCoord( match->getCoords().lat,  //<=
                                     match->getCoords().lon );
            allRoutePoints.insert( match->getCoords() );
         } else {
            mc2log << " Not id nor coords valid for dest id " 
                   << MC2CITE( dests[ i ]->getString(
                                  m_thread->clientUsesLatin1()) );
         }
         delete match;
      } // End for all 1008s
   }

   //Max route size
   uint32 maxRouteReplySize = 14000;
   if ( rd.params.getParam( 1010 ) ) {
      maxRouteReplySize = rd.params.getParam( 1010 )->getUint32();
      rd.session->setMaxBufferLength( maxRouteReplySize );
      mc2log << " max " << maxRouteReplySize;
   }

   //Origin favorite coordinate
   if ( rd.params.getParam( 1011 ) ) {
      vector< const NParam* > origs;
      rd.params.getAllParams( 1011, origs );
      for ( uint32 i = 0 ; i < origs.size() ; ++i ) {
         int32 lat = origs[ i ]->getInt32Array( 0 );
         int32 lon = origs[ i ]->getInt32Array( 1 );
         MC2Coordinate orig( Nav2Coordinate( 
//                                origs[ i ]->getInt32Array( 0 ),
//                                origs[ i ]->getInt32Array( 1 ) ) );
                                lat, lon ) );
         mc2log << " OrigF " << orig;
         if ( orig.isValid() ) {
            anyOrig = true;
            rr->addOriginCoord( orig.lat, orig.lon ); //<=
            //allRoutePoints.insert( orig );
         } else {
            mc2log << " INVALID! ";
         }
      }
   }

   //destination favorite coordinate
   if ( rd.params.getParam( 1012 ) ) {
      vector< const NParam* > dests;
      rd.params.getAllParams( 1012, dests );
      for ( uint32 i = 0 ; i < dests.size() ; ++i ) {
         int32 lat = dests[ i ]->getInt32Array( 0 );
         int32 lon = dests[ i ]->getInt32Array( 1 );
         MC2Coordinate dest( Nav2Coordinate( 
//                                dests[ i ]->getInt32Array( 0 ),
//                                dests[ i ]->getInt32Array( 1 ) ) );
                                lat, lon ) );
         mc2log << " DestF " << dest;
         if ( dest.isValid() ) {
            anyDest = true;
            rr->addDestinationCoord( dest.lat, dest.lon ); //<=
            allRoutePoints.insert( dest );
         } else {
            mc2log << " INVALID! ";
         }
      }
   }

   //reroute reason
   uint8 reason = 4; //user_request
   if ( rd.params.getParam( 1013 ) ) {
      const class NParam* p = rd.params.getParam( 1013 );
      reason = p->getByte();
      mc2log << " reroute Reason " << unsigned(reason);
   }

   // ReqVer
   mc2log << " Ver " << int(rd.req->getReqVer());

   // End parameter printing
   mc2log << endl;

   // ======== client route request decoded ================

   if ( m_thread->checkIfIronClient( rd.clientSetting ) ) {
      // Check if may route (Has access to service WF but maybe not route)
      if ( !m_thread->checkIfIronUserMay( user, rd.clientSetting, 
                                          UserEnums::UR_ROUTE ) )
      {
         mc2log << info << "handleRoute: Iron User has no route access." 
                << endl;
         ok = false;
         // Send error
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_EXTENDED_ERROR );
         rd.rparams.addParam( NParam( 24, uint32( 0x17001 ) ) );
      }
   }   

   // Get AURA
   RouteAllowedMap* maps = NULL;
   uint32 auraStart = TimeUtility::getCurrentTime();
   if ( ok ) {
      if ( !m_thread->getMapIdsForUserRegionAccess( user, maps, rd.urmask ) )
      {
         ok = false;
         mc2log << warn << "handleRoute: getMapIdsForUserRegionAccess"
                << " failed. Error: ";
         if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
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
   mc2dbg << "[NRH]: Aura calc took "
          << (TimeUtility::getCurrentTime() - auraStart) << endl;

   // Store user position
   if ( ok && pos.isValid() ) {
      m_userHelp->storeUserPosition( user->getUIN(), pos, "Route" );
   }


   if ( ok && (!anyOrig || !anyDest) ) {
      if ( !anyOrig ) {
         mc2log << "handleRoute: No origin!" << endl;
         ok = false;
         rd.reply->setStatusCode( ROUTE_REPLY_NO_ORIGIN );
      } else {
         mc2log << "handleRoute: No destination!" << endl;
         ok = false;
         rd.reply->setStatusCode( ROUTE_REPLY_NO_DESTINATION );
         
      }
   }

   //========== admin done ===========================
   if ( ok ) {
      rr->setRouteParameters( false,                    // isStartTime
                              routeCost,
                              routeVehicle,
                              0,                        // time
                              1,                        // U-turn cost
                              abbreviate,
                              true, //addLandmarks || routeCost == 
                              //RouteTypes::TIME_WITH_DISTURBANCES,
                              // Always true for now.
                              // Landmarks never set by client
                              avoidTollRoads,
                              avoidHighway );
      rr->setAllowedMaps( maps );

      
      if(useGPS){
         mc2dbg2 << "User using GPS. ";
         if(rd.req->getProtoVer() < 0x07 ){
            mc2dbg2 << "Removing Ahead." << endl;
            rr->setRemoveAheadIfDiff(true);
            rr->setNameChangeAsWP(false);
         } else {
            mc2dbg2 << "Name changes as waypoints" << endl;
            rr->setRemoveAheadIfDiff(false);
            // In ERP removeAheadIfNameDiffer = true if NameChangeAsWP
            rr->setNameChangeAsWP(true);
         }
      } else {
         mc2dbg2 << "User NOT using GPS! " << endl;
         if ( rd.req->getProtoVer() < 0x07 ) {
            mc2dbg2 << "Old leaving Ahead." << endl;
            rr->setRemoveAheadIfDiff( false );
            rr->setNameChangeAsWP( false );
         } else {
            mc2dbg2 << "Name changes as waypoints" << endl;
            rr->setRemoveAheadIfDiff( false );
            // In ERP removeAheadIfNameDiffer = true if NameChangeAsWP
            rr->setNameChangeAsWP( true );
         }
      }
      
      // If to check route w. disturbances
      if ( routeCost == RouteTypes::TIME_WITH_DISTURBANCES ) {
         rr->setCompareDisturbanceRoute( true );
      }

      RouteRequestParams rrp;
      rr->extractParams(rrp);
      if ( reason == 3/*traffic_info_update*/ &&
           ! m_thread->getRouteHandler().routeChanged( oldRouteID, 
                                                       rrp,
                                                       rd.session->getUser()->getUser(),
                                                       m_thread->getTopRegionRequest() ) ) {
         //keep old route. 
         mc2log << info << "handleRoute: Keep old route: "
                << oldRouteID << endl;

         rd.rparams.addParam( NParam( 1100, oldRouteID.toString().c_str(), 
                                   m_thread->clientUsesLatin1() ) );
         rd.reply->setStatusCode( ROUTE_REPLY_NO_ROUTE_CHANGE );
         delete rr->getAnswer();
         delete rr;
         return ok;
      }

      m_thread->sendRequest( rr );

      // While RouteRequest is processing do the checkService
      // Only check when it is a user request.
      bool statusCodeSet = false;
      if ( reason == 4 ) { //user_request

         // Check with external access authority  if user is allowed to route
         PurchaseOptions purchaseOptions( rd.clientSetting->getClientType() );
         set< MC2String > checkedServiceIDs;
         for ( set< MC2Coordinate >::const_iterator it = allRoutePoints.begin();
               it != allRoutePoints.end(); ++it ) {
            if ( ! m_thread->checkService( rd.clientSetting,
                                           rd.ireq->getHttpRequest(),
                                           OperationType::ROUTE_RIGHT,
                                           purchaseOptions,
                                           checkedServiceIDs,
                                           *it,
                                           MAX_UINT32,
                                           rd.clientLang ) ) {
               mc2log << warn << "handleRoute: checkService failed." << endl;
               ok = false;
            }
         }
         for ( set< uint32 >::const_iterator it = allMapIDs.begin();
               it != allMapIDs.end(); ++it ) {
            uint32 topregionID = m_thread->
               getTopRegionRequest()->getCountryForMapID( *it )->getID();
            if ( ! m_thread->checkService( rd.clientSetting,
                                           rd.ireq->getHttpRequest(),
                                           OperationType::ROUTE_RIGHT,
                                           purchaseOptions,
                                           checkedServiceIDs,
                                           MC2Coordinate::invalidCoordinate,
                                           topregionID,
                                           rd.clientLang ) ) {
               mc2log << warn << "handleRoute: checkService failed." << endl;;
               ok = false;
            }
         }
         if ( ! ok ) {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
            using namespace PurchaseOption;
            if ( purchaseOptions.getReasonCode() == 
                 PurchaseOption::NEEDS_TO_BUY_APP_STORE_ADDON ) {
               // Add the packages to reply
               MC2String packageStr( "iPhoneAppStore;" );
               const PurchaseOptions::packageIDs& packages =
                  purchaseOptions.getAppPackages();
               for ( PurchaseOptions::packageIDs::const_iterator it = 
                        packages.begin(), end = packages.end() ; it != end ; 
                     ++it ) {
                  if ( it != packages.begin() ) {
                     packageStr.append( "," );
                  }
                  packageStr.append( *it );
               }
               rd.rparams.addParam( NParam( 33, packageStr ) );
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_EXPIRED_USER );
               statusCodeSet = true;
            } else {
               mc2log << "handleRoute:checkService General error.";
               if ( purchaseOptions.getReasonCode() == SERVICE_NOT_FOUND ) {
                  mc2log << " Service id not possible to purchase.";
                  MC2String noServiceIdURL( "http://show_msg/?txt=" );
                  MC2String errorMessage( StringTable::
                                          getString( StringTable::WF_NO_BILL_AREA,
                                                     rd.clientLang ) );
                  noServiceIdURL += StringUtility::URLEncode( errorMessage );

                  rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_EXPIRED_USER );
                  statusCodeSet = true;
                  rd.rparams.addParam( NParam( 26,
                                               noServiceIdURL,
                                               m_thread->clientUsesLatin1() ) );
               }
            }

            mc2log << endl;
            //return ok; handled in the end of the function.
         }
      }

      // Wait for the RouteRequest to finish
      m_thread->waitForRequest( rr->getID() );

      if ( rr->getStatus() == StringTable::OK && ok ) {

         PacketContainer* pc = rr->getAnswer();

         // Make reply
         NavAddress address;
         isabBoxRouteReq rmess( address, rd.session );
         rmess.setRouteReqHeader( rd.req->getProtoVer(), 0, 0, "" );
         rmess.setRouteParams( language, routeVehicle, routeCost,
                               timeToTrunc, 
                               RouteReq::routeType( routeContent ) );
         RouteList routeList( &rmess, pc, rr );
         IsabRouteList::colorTable_t colorTable;
         IsabRouteList irl( rd.req->getProtoVer(), &routeList, 
                            &routeList.getStringTable(), rd.session,
                            rd.req->getReqVer(), &colorTable );

         if ( irl.isValid() ) {
            // Route id
            RouteID routeID( rr->getRouteID(), rr->getRouteCreateTime() );
            rd.rparams.addParam( NParam( 1100, routeID.toString().c_str(),
                                      m_thread->clientUsesLatin1() ) );
            if ( irl.isTruncated() ) {
               // Truncated dist
               rd.rparams.addParam( 
                  NParam( 1101, 
                          routeList.getTotalDist() - 
                          irl.getTruncatedDist() ) );
               // Dist to WP
               rd.rparams.addParam( 
                  NParam( 1102, irl.getDist2nextWPTFromTrunk() ) );
               // Phone dist
               uint32 phoneHomeDist = irl.getTruncatedDist() / 2;
               if ( phoneHomeDist > 2000 ) {
                  phoneHomeDist = 2000;
               }
               rd.rparams.addParam( NParam( 1103, phoneHomeDist ) );
            } // End if truncated
            
            // Route string table
            NavStringTable& navStringTable = routeList.getStringTable();
            NParam pstrings( 1104 );
            for ( int32 i = 0 ; i <= irl.getNbrStringsUsed() ; ++i ) {
               if ( pstrings.getLength() + strlen( navStringTable[ i ] ) +1
                    > MAX_UINT16 ) 
               {
                  rd.rparams.addParam( pstrings );
                  pstrings.clear();
               }
               pstrings.addString( navStringTable[ i ],
                                   m_thread->clientUsesLatin1() );
            }
            rd.rparams.addParam( pstrings );
            
            // Route data
            NParam proute( 1105 );
            uint32 pos = 0;
            byte buff[ 200 ]; // Should be enough with 12 I think
            for ( int32 i = 0 ; i < irl.getSize() ; i++ ) {
               pos = 0;
               const IsabRouteElement* ire = irl.isabRouteElementAt( i );
               pos = ire->write( buff, pos );
               if ( proute.getLength() + pos > MAX_UINT16 ) {
                  rd.rparams.addParam( proute );
                  proute.clear();
               }
               proute.addByteArray( buff, pos );
            }
            rd.rparams.addParam( proute );
            
            // Route bbox
            NParam& pbbox = rd.rparams.addParam( NParam( 1106 ) );
            MC2BoundingBox bbox = 
               rr->getExpandedRoute()->getRouteBoundingBox();
            Nav2Coordinate topLeft( 
               MC2Coordinate( bbox.getMaxLat(), bbox.getMinLon() ) );
            Nav2Coordinate bottomRight( 
               MC2Coordinate( bbox.getMinLat(), bbox.getMaxLon() ) );
            pbbox.addInt32( topLeft.nav2lat );
            pbbox.addInt32( topLeft.nav2lon );
            pbbox.addInt32( bottomRight.nav2lat );
            pbbox.addInt32( bottomRight.nav2lon );

            // Periodic traffic info interval
            rd.rparams.addParam( NParam( 1107, m_group->getPeriodicTrafficUpdateInterval() ) );

            // ColorTable
            NParam& pcolorTable = rd.rparams.addParam( NParam( 1108 ) );
            for ( IsabRouteList::colorTable_t::const_iterator it = 
                     colorTable.begin(); it != colorTable.end() ; ++it ) {
               pcolorTable.addByte( it->red );
               pcolorTable.addByte( it->green );
               pcolorTable.addByte( it->blue );
            }
            
            
            mc2log << info << "handleRoute: OK";
            mc2log << " routeID " << rr->getRouteID() << "," 
                   << rr->getRouteCreateTime() 
                   << " TotalDist " << routeList.getTotalDist()
                   << " TotalTime " << routeList.getTotalTime( false )
                   << " nbrWPTs " << routeList.getSize();
            if ( routeList.isTruncated() ) {
               mc2log << " Truncated at WPT " 
                      << routeList.getTruncatedWPTNbr()
                      << " after " << routeList.getTruncatedDist() << "m";
            }
            mc2log << endl;
            
            // Store route, for future requests (maps, email, reroute, etc)
            m_thread->storeRoute( rr, user->getUIN(), "NS", rd.urmask );
            
            if ( m_group->simulatedRouteFileName() ) {
               mc2dbg2 << "Saving simulator route." << endl;
               m_group->saveRouteForSimulator( rd.reply, rr );
            }
            
            // Debit route
            // Set debitamount here until module sets it ok
            rr->setProcessingTime( ( TimeUtility::getCurrentTime() - 
                                     startTime ) );
            MC2String extraInfo = m_userHelp->makeExtraUserInfoStr( 
               rd.params );
            if ( !m_thread->getDebitHandler()->makeRouteDebit( 
                    rd.session->getUser(), rr, oldRouteID, reason,
                    extraInfo.c_str() ) )
            {
               mc2log << warn << "handleRoute: failed to debit route."
                      << endl;
            }
            
         } else {
            mc2log << info << "handleRoute: Making irl failed: Setting ";
            uint8 errorStatus = NavReplyPacket::NAV_STATUS_NOT_OK;
            if ( routeList.getSize() < 2 ) {
               mc2log << "ERROR_NO_ROUTE";
               errorStatus = ROUTE_REPLY_NO_ROUTE_FOUND;
            } else {
               mc2log << "TOO_FAR_FOR_VEHICLE";
               errorStatus = ROUTE_REPLY_TOO_FAR_FOR_VEHICLE;
            }
            mc2log << endl;
            rd.reply->setStatusCode( errorStatus );
         }
      } else {
         // Send error
         mc2log << info << "handleRoute: Routing failed: ";
         uint8 errorStatus = NavReplyPacket::NAV_STATUS_NOT_OK;
         if ( rr->getStatus() == StringTable::TIMEOUT_ERROR ) {
            mc2log << "TIMEOUT_ERROR";
            errorStatus = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
         } else if ( rr->getStatus() == StringTable::MAPNOTFOUND ) {
            mc2log << "MAPNOTFOUND";
            errorStatus = NavReplyPacket::NAV_STATUS_OUTSIDE_MAP;
         } else if ( rr->getStatus() == StringTable::ERROR_NO_ROUTE ) {
            mc2log << "ERROR_NO_ROUTE";
            errorStatus = ROUTE_REPLY_NO_ROUTE_FOUND;
         } else if ( rr->getStatus() == StringTable::TOO_FAR_TO_WALK ) {
            mc2log << "TOO_FAR_TO_WALK";
            errorStatus = ROUTE_REPLY_TOO_FAR_FOR_VEHICLE;
         } else if ( rr->getStatus() == 
                     StringTable::ERROR_NO_VALID_START_ROUTING_NODE ||
                     rr->getStatus() == 
                     StringTable::ONE_OR_MORE_INVALID_ORIGS ) 
         {
            mc2log << "NO_VALID_START_ROUTING_NODE";
            errorStatus = ROUTE_REPLY_PROBLEM_WITH_ORIGIN;
         } else if ( rr->getStatus() == 
                     StringTable::ERROR_NO_VALID_END_ROUTING_NODE ||
                     rr->getStatus() == 
                     StringTable::ONE_OR_MORE_INVALID_DESTS ) 
         {
            mc2log << "NO_VALID_END_ROUTING_NODE";
            errorStatus = ROUTE_REPLY_PROBLEM_WITH_DEST;
         } else if ( rr->getStatus() == 
                     StringTable::DESTINATION_OUTSIDE_ALLOWED_AREA ||
                     rr->getStatus() == 
                     StringTable::ORIGIN_OUTSIDE_ALLOWED_AREA ||
                     rr->getStatus() == 
                     StringTable::OUTSIDE_ALLOWED_AREA )
         {
            mc2log << "OUTSIDE_ALLOWED_AREA";
            errorStatus = NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA; 
         } else {
            // Else not ok
            mc2log << "NOT_OK";
            errorStatus = NavReplyPacket::NAV_STATUS_NOT_OK;
         }
         mc2log << endl;
         // Check if the status code is set in the checkService section
         if ( ! statusCodeSet ) {
            // status code not set, do it!
            rd.reply->setStatusCode( errorStatus );
            rr->dumpState();
         }
      }
      
   }

   delete rr->getAnswer();
   delete rr;
   delete maps;

   return ok;
}


ItemTypes::vehicle_t
NavRouteHandler::mc2Vehicle( byte vehicle ) const {
   switch ( vehicle ) {
      case 0x01 :
         return ItemTypes::passengerCar;
      case 0x02 :
         return ItemTypes::pedestrian;
      case 0x03 :
         return ItemTypes::emergencyVehicle;
      case 0x04 :
         return ItemTypes::taxi;
      case 0x05 : 
         return ItemTypes::publicBus;
      case 0x06 :
         return ItemTypes::deliveryTruck;
      case 0x07 : 
         return ItemTypes::transportTruck;
      case 0x08 :
         return ItemTypes::highOccupancyVehicle;
      case 0x09 : 
         return ItemTypes::bicycle;
      case 0x0a :
         return ItemTypes::publicTransportation;
      case 0xff :
         return ItemTypes::passengerCar;
      default:
         mc2log << warn << "mc2Vehicle unknown vehicle " 
                << int(vehicle) << " using passengerCar. " << endl;
         return ItemTypes::passengerCar;
   }
}

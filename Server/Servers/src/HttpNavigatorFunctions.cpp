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

#include "HttpNavigatorFunctions.h"
#include "HttpParserThread.h"
#include "isabBoxRouteMessage.h"
#include "ExpandRouteRequest.h"
#include "RoutePacket.h"
#include "HttpHeader.h"
#include "HttpBody.h"
#include "RouteRequest.h"
#include "RequestUserData.h"
#include "UserData.h"
#include "LangUtility.h"


bool 
HttpNavigatorFunctions::htmlNavigatorRoute( stringVector* params,
                                            int paramc, 
                                            stringMap* paramsMap,
                                            HttpHeader* inHead, 
                                            HttpHeader* outHead,
                                            HttpBody* inBody,
                                            HttpBody* outBody,
                                            HttpParserThread* myThread,
                                            HttpVariableContainer* myVar )
{
   uint8 protoVer = MAX_UINT8;
   bool hasProtoVer = false;
   const char* routeIDStr = NULL;
   uint32 routeID = 0;
   uint32 routeCreateTime = 0;
   bool hasRouteID = false;
   char* tmp = NULL;
   StringTable::languageCode language = StringTable::ENGLISH;
   isabBoxSession* session = new isabBoxSession();
   session->beginSession();
   uint32 maxBufferLength = session->getMaxBufferLength();
   bool paramsOk = true;
   
   // protoVer (protoVer)
   if ( paramsMap->find( "protoVer" ) != paramsMap->end() ) {
      const MC2String* str = paramsMap->find( "protoVer" )->second;
      
      protoVer = strtol( str->c_str(), &tmp, 10 );
      if ( tmp != str->c_str() ) {
         hasProtoVer = true; 
      }      
   }

   // routeID (r)
   if  ( paramsMap->find( "r" ) != paramsMap->end() ) {
      routeIDStr = (*paramsMap )[ "r" ] ->c_str();
      if ( sscanf( routeIDStr, "%X_%X", &routeID, &routeCreateTime ) == 2 )
      {
         hasRouteID = true;
      }
   }

   // Max size (s)
   if ( paramsMap->find( "s" ) != paramsMap->end() ) {
      const MC2String* str = paramsMap->find( "s" )->second;
      
      maxBufferLength = strtol( str->c_str(), &tmp, 10 );
      if ( tmp != (str->c_str() + str->size()) ) {
         mc2log << warn << "HttpNavigatorFunctions::htmlNavigatorRoute "
                << "s-param not a number." << endl;
      } else {
         mc2dbg8 << "[HNF]: s param = " << maxBufferLength << endl;
         session->setMaxBufferLength( maxBufferLength );
      }
   }

   // Landmarks. 0 is off and default.
   long int landmarks = paramsMap->getIntParam( "lm", 0 );

   // Lang
   if ( paramsMap->find( "l" ) != paramsMap->end() ) {
      language = LangUtility::getLanguageCode( (*paramsMap )[ "l" ]->c_str() );
   }

   // reqVer
   uint16 reqVer = 1;
   if ( paramsMap->find( "rv" ) != paramsMap->end() ) {
      reqVer = paramsMap->getIntParam( "rv", 1 );
   }

   if ( hasProtoVer && hasRouteID && paramsOk ) {
      // Get stored route
      RouteReplyPacket* routePack = NULL;
      uint32 UIN = 0;
      const char* extraUserinfo = NULL;
      uint32 validUntil = 0;
      int32 originLat = 0;
      int32 originLon = 0;
      uint32 originMapID = 0;
      uint32 originItemID = 0;
      uint16 originOffset = 0;
      int32 destinationLat = 0;
      int32 destinationLon = 0;
      uint32 destinationMapID = 0;
      uint32 destinationItemID = 0;
      uint16 destinationOffset = 0;
      byte expandType = ( ROUTE_TYPE_STRING | 
                          ROUTE_TYPE_ITEM_STRING |
                          ROUTE_TYPE_GFX | 
                          ROUTE_TYPE_NAVIGATOR );
      PacketContainer* expandRouteCont = NULL;
      ExpandRouteRequest* expandReq = NULL;
      PacketContainer* cont = NULL;
      const bool abbrev = true;
      const bool removeAheadIfDiff = true;
      // In ERP removeAheadIfNameDiffer = true if NameChangeAsWP;
      const bool nameChangeAsWP = true;

      cont = myThread->getStoredRouteAndExpand(
         routeID, routeCreateTime,
         expandType, language, abbrev, landmarks, removeAheadIfDiff,
         nameChangeAsWP, routePack, UIN, extraUserinfo, validUntil,
         originLat, originLon, originMapID, originItemID, originOffset,
         destinationLat, destinationLon, destinationMapID, 
         destinationItemID, destinationOffset,
         expandReq, expandRouteCont );

      if ( cont != NULL ) {
         // Check Expand route
         if ( expandRouteCont != NULL &&
              static_cast< ReplyPacket* > ( expandRouteCont->getPacket() )
              ->getStatus() == StringTable::OK )
         {
            NavAddress* navAddress = new NavAddress( 
               NavAddress::ADDRESS_TYPE_ISABBOX, "htmlNavigatorRoute" );
            // Fake session support. Use an empty session. The route is 
            // already generated without regard to the session data.
            // Except for max size 
            isabBoxRouteReq* request = new isabBoxRouteReq( 
               *navAddress, session );
            request->setRouteParams( language, ItemTypes::passengerCar, 
                                     RouteTypes::TIME, MAX_UINT32, 
                                     RouteReq::full );
            RouteRequest* rr = new RouteRequest( NULL, /* UserUser */
               0, expandType, language, myThread->getTopRegionRequest(), 
               true, 0 );
            delete rr->getNextPacket(); // Inits RouteObject
            request->setRouteData( 0, 0, 0, 0, 0 );
            char userName[20] = "htmlNavigatorRoute ";
            request->setRouteReqHeader( protoVer, 0, 0, userName );

            isabBoxRouteReply* reply = new isabBoxRouteReply(
               *navAddress, expandRouteCont, routeID, routeCreateTime,
               request, session, rr, reqVer );

            const uint32 buffLength = 1000000;
            byte* buff = new byte[ buffLength ];
            
            if ( reply->convertToBytes( buff, buffLength ) ) {
               mc2log << info << "htmlNavigatorRoute navroutelength " 
                      << reply->getLength() << endl;
               outBody->setBody( buff, reply->getLength() );
            } else {
               mc2log << warn << "HttpNavigatorFunctions::"
                         "htmlNavigatorRoute failed to convert route "
                         "to bytes: "
                      << routeID << "_" << routeCreateTime << endl; 
            }

            delete rr;
            delete [] buff;
            delete reply;
            delete request;
            delete navAddress;
         } else {
            mc2log << warn << "HttpNavigatorFunctions::htmlNavigatorRoute "
                      " failed to expand stored route: " << routeID << "_" 
                << routeCreateTime << endl;
         }
      } else {
         mc2log << warn << "HttpNavigatorFunctions::htmlNavigatorRoute "
                   " failed to get stored route: " << routeID << "_" 
                << routeCreateTime << endl;
      }

      delete cont;
      delete routePack;
      delete expandReq;
      // expandRouteCont ownded by (isabBox)RouteReply
   } else {
      mc2log << warn << "HttpNavigatorFunctions::htmlNavigatorRoute "
                " not enough parameters. hasProtoVer " << hasProtoVer 
             << " hasRouteID " << hasRouteID << " paramsOk " << paramsOk
             << endl; 
   }

   delete session;
   return true;
}

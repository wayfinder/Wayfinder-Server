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
#include "HttpRouteFunctions.h"
#include "HttpParserThread.h"
#include "HttpFunctionHandler.h"
#include "HttpBody.h"
#include "HttpUtility.h"
// htmlShowStoredRoute uses
#include "RouteMessageRequest.h"
#include "MimeMessage.h"
#include "RoutePacket.h"
#include "ExpandRoutePacket.h"
#include "ExpandRouteRequest.h"
#include "ExpandItemID.h"
#include "HttpHeader.h"


bool 
HttpRouteFunctions::htmlShowStoredRoute( stringVector* params,
                                         int paramc, 
                                         stringMap* paramsMap,
                                         HttpHeader* inHead, 
                                         HttpHeader* outHead,
                                         HttpBody* inBody,
                                         HttpBody* outBody,
                                         HttpParserThread* myThread,
                                         HttpVariableContainer* myVar )
{
   myThread->addRequestName( "SHOW_ROUTE" );
   bool hasUserSettings = false;
   bool hasRouteID = false;
   uint32 routeID = 0;
   uint32 routeCreateTime = 0;
   const char* originString = "";
   const char* originLocationString = "";
   const char* destinationString = "";
   const char* destinationLocationString = "";
   const char* signature = "";


   // User settings (us)
   if ( paramsMap->find( "us" ) != paramsMap->end() ) {
      // Get user settings
      if ( HttpUtility::parseUserSettings( (*paramsMap)["us"]->c_str(),
                                           myVar->currentLanguage ) ) 
      {
         hasUserSettings = true;
      } else {
         mc2log << warn << "HttpRouteFunctions::htmlShowStoredRoute "
                << "invalid user settings! " << (*paramsMap)["us"] << endl;
      }
   }

   // RouteID (r)
   if ( paramsMap->find( "r" ) != paramsMap->end() ) {
      const char* routeIDStr = (*paramsMap)[ "r" ]->c_str();

      if ( sscanf( routeIDStr, "%X_%X", 
                   &routeID, &routeCreateTime ) == 2 ) 
      {
         hasRouteID = true;
      } else {
         mc2log << warn << "HttpRouteFunctions::htmlShowStoredRoute "
                << "invalid routeID syntax! " << routeIDStr << endl;
         outBody->addString( "Invalid routeID." );
      }
   }

   // originString (o_s)
   if ( paramsMap->find( "o_s" ) != paramsMap->end() ) {
      originString = (*paramsMap)[ "o_s" ]->c_str();
   }

   // originLocationString (ol_s)
   if ( paramsMap->find( "ol_s" ) != paramsMap->end() ) {
      originLocationString = (*paramsMap)[ "ol_s" ]->c_str();
   }
   
   // destinationString (d_s)
   if ( paramsMap->find( "d_s" ) != paramsMap->end() ) {
      destinationString = (*paramsMap)[ "d_s" ]->c_str();
   }

   // destinationLocationString (dl_s)
   if ( paramsMap->find( "dl_s" ) != paramsMap->end() ) {
      destinationLocationString = (*paramsMap)[ "dl_s" ]->c_str();
   }

   // Signature (sig)
   if ( paramsMap->find( "sig" ) != paramsMap->end() ) {
      signature = (*paramsMap)[ "sig" ]->c_str();
   }

   

   if ( hasRouteID && hasUserSettings ) {
      mc2log << info << myThread->getLogPrefix() << "htmlShowStoredRoute " 
             << *inHead->getStartLine() << endl;
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
                          ROUTE_TYPE_GFX_COORD );
      PacketContainer* expandRouteCont = NULL;
      ExpandRouteRequest* expandReq = NULL;
      PacketContainer* cont = NULL;

      cont = myThread->getStoredRouteAndExpand(
         routeID, routeCreateTime,
         expandType, myVar->currentLanguage, false, true, true, false,
         routePack, UIN, extraUserinfo, validUntil,
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
            ExpandRouteReplyPacket* expandPack = 
                  static_cast<ExpandRouteReplyPacket*> (  
                     expandRouteCont->getPacket() );
            // Get data
            ExpandItemID* exp = expandPack->getItemID();
            // The route vehicle
            uint32 mapID = 0;
            uint32 itemID = 0;
            ItemTypes::vehicle_t routeVehicle = ItemTypes::passengerCar;
            if ( routePack->getNbrItems() > 0 &&
                 GET_TRANSPORTATION_STATE( 
                    (routePack->getRouteItem(0, mapID, itemID),
                     itemID) ) != 0 )
            {
               routeVehicle = 
                  ItemTypes::transportation2Vehicle(
                     GET_TRANSPORTATION_STATE( itemID ) );
            }
            ImageDrawConfig::imageFormat imageFormat = 
               HttpUtility::getImageFormatForRequest( 
                  inHead, ImageDrawConfig::PNG );

            RouteMessageRequest* routeHtmlReq = new RouteMessageRequest( 
               myThread->getNextRequestID(), routeID, routeCreateTime, 
               myVar->currentLanguage, routePack, expandPack, exp, 
               500, 500, 200, 200,
               originString, originLocationString,
               destinationString, destinationLocationString,
               routeVehicle, myThread->getTopRegionRequest(),
               false, false, true, true, false, false, 
               false /* onlyOverview */,
               RouteMessageRequestType::HTML_CONTENT, 
               UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE, 
               RouteArrowType::TURN_ARROW,
               MAX_UINT32, false,
               "", "", "", signature, imageFormat );
            
            const MimeMessage* message = routeHtmlReq->getMimeMessage();
            if ( message->getNbrMimeParts() >= 1 ) {
               const MimePart* page = message->getMimePart( 0 );
               uint32 size = 0;
               byte* buff = page->getUnencodedContent( size );
               if ( buff != NULL ) {
                  buff[ size ] = '\0';
                  MC2String data( (char*)buff ); 
                  outBody->setBody( &data );
               }

               delete [] buff;
            }

            delete exp;
            delete routeHtmlReq;
         } else {
            mc2log << warn << "HttpRouteFunctions::htmlShowStoredRoute "
                   << " failed to expand stored route: " << routeID << "_" 
                   << routeCreateTime << endl;
            outBody->addString( "Expanded route not ok." );
         }
      } else {
         mc2log << warn << "HttpRouteFunctions::htmlShowStoredRoute  "
                << " failed to get stored route: " << routeID << "_" 
                << routeCreateTime << endl;
         outBody->addString( "Failed to expand route." );
      }

      delete cont;
      delete routePack;
      delete expandReq;
      delete expandRouteCont;
   } else {
      outBody->addString( "Not enough data." );
   }

   return true;
}

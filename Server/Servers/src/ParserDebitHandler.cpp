/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserDebitHandler.h"
#include "DebitMaking.h"

// Utility
#include "TimeUtility.h"
#include "NetUtility.h"

// Parser
#include "ParserThread.h"

// Requests
#include "ModuleTypes.h"
#include "SearchResultRequest.h"
#include "ProximityRequest.h"
#include "RouteRequest.h"
#include "SMSSendRequest.h"
#include "SinglePacketRequest.h"
#include "RouteMessageRequest.h"
#include "LocalMapMessageRequest.h"

// Search
#include "SearchMatch.h"

// Packets
#include "PacketContainer.h"
#include "DebitPacket.h"

// User
#include "UserData.h"


ParserDebitHandler::ParserDebitHandler( ParserThread* thread,
                                        ParserThreadGroup* group )
      : ParserHandler( thread, group )
{
}

bool
ParserDebitHandler::sendDebitInformation(
   const UserItem* user,
   uint32 messageID,
   uint32 debitAmount,
   uint32 operationType,
   char* debitText,
   uint32 size,
   const char* extraUserOrigin )
{
   if ( !m_thread->isCheckUser() ) {
      uint32 startTime = TimeUtility::getCurrentTime();
   
      debitText[255] = '\0'; // max size in database
      // note: seems a bit dangerous to have only 256 bytes
      char* userID = new char[ 256 ];
      mc2dbg4  << "ParserDebitHandler::sendDebitInfo(): peerIP = " 
               << m_thread->getPeerIP() << endl;

      strcpy( userID, NetUtility::getHostName( m_thread->getPeerIP(), 
                                               false ).c_str() );

      if ( (extraUserOrigin != NULL) && 
           ( strcmp( extraUserOrigin, "" ) != 0 )) {
         // Add extra user origin info
         uint32 length = strlen( userID );
         userID[ length ] = ',';
         length++;
         strcpy( userID + length, extraUserOrigin );
      }
      // note: seems a bit dangerous to have only 256 bytes
      //       better with strncat on the possibly 255 chars logonID.
      char* serverID = new char[ 256 ];
      strcpy( serverID, m_thread->getServerType() );
      strcat( serverID, ":" );
      if (user != NULL) {
         strncat( serverID, user->getUser()->getLogonID(), 128 );
         strcat( serverID, ":" );
      } else {
         mc2log << error << "ParserDebitHandler::sendDebitInformation NO user "
                   "to debit!" << endl;
         strcat( serverID, "UNKNOWN:" );
      }
      strcat( serverID, m_thread->getServerName().c_str() );
      
      uint32 userIN = 0;
      if (user != NULL) {
         userIN = user->getUIN();
      }
      DebitRequestPacket* debitPacket = new DebitRequestPacket( 
         0, 
         0, 
         userIN, 
         messageID,
         debitAmount,
         TimeUtility::getRealTime(),
         operationType,
         size,
         userID,
         serverID,
         debitText );
      delete [] userID;
      delete [] serverID;
      // Send
      mc2dbg4 << "ParserDebitHandler::sendDebitInfo(): "
              << "About to send DebitRequest" << endl;
      PacketContainer* cont = m_thread->putRequest( 
         new PacketContainer( debitPacket,
                              0, 0, MODULE_TYPE_USER ) );
      mc2dbg4 << "ParserDebitHandler::sendDebitInfo(): DebitRequest returned"
              << endl;
      bool ok = false;
      if ( cont != NULL &&
           static_cast<ReplyPacket*>(cont->getPacket())->getStatus() 
           == StringTable::OK ) 
      {
         ok = true;
      }
      delete cont;

      // Print statistsics and return
      mc2dbg2 << "ParserDebitHandler::sendDebitInfo(): processTime = "
              << TimeUtility::getCurrentTime() - startTime << "ms" << endl;
      return ok;
   } else {
      // Shouldn't debit, so done
      return true;
   }
}


bool 
ParserDebitHandler::makeSearchDebit(
   UserItem* user,
   Request* inReq, 
   PacketContainer* ansCont,
   const char* const locationString, 
   const char* const searchString,
   const char* extraUserOrigin,
   const char* extraSearchString )
{
   char operation[1024];   
   
   if ( dynamic_cast< SearchResultRequest* > ( inReq ) != NULL ) {
      SearchResultRequest* req = static_cast< SearchResultRequest* > ( 
         inReq );
      if ( DebitMaking::makeSearchDebit( operation, req, 
                                         locationString, searchString, 
                                         extraSearchString ) )
      {
         // Send the debit-information to the module
         uint32 debitAmount = inReq->getProcessingTime();
         uint32 size = inReq->getNbrReceivedBytes();
         return (sendDebitInformation( user, 
                                       inReq->getID(), 
                                       debitAmount, 
                                       OperationType::SEARCH,
                                       operation,
                                       size, extraUserOrigin ) );
      } else {
         mc2log << warn << "ParserDebitHandler::makeSearchDebit "
                << "DebitMaking::makeSearchDebit failed" << endl;
         return false;
      }
   } else {
      mc2log << error << "PT::makeSearchDebit, not SearchResultRequest!"
             << endl;
      return false;
   }
}


bool 
ParserDebitHandler::makeProximityDebit( UserItem* user, 
                                        ProximityRequest* inReq, 
                                        PacketContainer* ansCont, 
                                        const char* const searchString,
                                        int32 lat, int32 lon, 
                                        uint32 distance )
{
   char operation[256];
   
   if ( DebitMaking::makeProximityDebit( operation, inReq, ansCont,
                                         searchString, lat, lon, 
                                         distance ) )
   {
      // Send the debit-information to the module
      uint32 debitAmount = inReq->getProcessingTime();
      return sendDebitInformation( user, 
                                   inReq->getID(), 
                                   debitAmount, 
                                   OperationType::PROXIMITY,
                                   operation );
   } else {
      mc2log << warn << "ParserDebitHandler::makeProximityDebit "
             << "DebitMaking::makeProximityDebit failed" << endl;
      return false;
   }
}

bool 
ParserDebitHandler::makeProximityDebit( UserItem* user,
                                        const SearchResultRequest* inReq, 
                                        const SearchRequestParameters& params,
                                        const char* searchString,
                                        const MC2Coordinate& center,
                                        uint32 distance )
{
   char operation[256];
   
   if ( DebitMaking::makeProximityDebit( operation, inReq, params,
                                         searchString, center,
                                         distance ) )
   {
      // Send the debit-information to the module
      uint32 debitAmount = inReq->getProcessingTime();
      return sendDebitInformation( user, 
                                   inReq->getID(), 
                                   debitAmount, 
                                   OperationType::PROXIMITY,
                                   operation );
   } else {
      mc2log << warn << "ParserDebitHandler::makeProximityDebit "
             << "DebitMaking::makeProximityDebit failed" << endl;
      return false;
   }
}


bool 
ParserDebitHandler::makeRouteDebit( UserItem* user,
                                    Request* inReq, 
                                    PacketContainer* ansCont,
                                    const VanillaMatch* const originVM,
                                    const VanillaMatch* const destinationVM )
{
   
   if ( dynamic_cast< RouteRequest* > ( inReq ) != NULL ) {
      char* operation = new char[1024];   
      DebitMaking::makeRouteDebit( operation, 
                                   static_cast< RouteRequest* > ( inReq ),
                                   originVM, destinationVM );

      // Send the debit-information to the module
      uint32 debitAmount = inReq->getProcessingTime();
      uint32 size = inReq->getNbrReceivedBytes();
      bool res = sendDebitInformation( user, 
                                       inReq->getID(), 
                                       debitAmount, 
                                       OperationType::ROUTE,
                                      operation, size );
      delete [] operation;
      return res;
   } else {
      mc2log << error << "ParserDebitHandler::makeRouteDebit VM, "
             << "not RouteRequest!" << endl;
      return false;
   }
}

bool 
ParserDebitHandler::makeRouteDebit( UserItem* user,
                                    Request* inReq, 
                                    PacketContainer* ansCont,
                                    uint32 nbrRouteItems,
                                    RouteItem** theRouteItems )
{
   if ( dynamic_cast< RouteRequest* > ( inReq ) != NULL ) {
      char operation[1024];   
      DebitMaking::makeRouteDebit( operation, 
                                   static_cast< RouteRequest* > ( inReq ),
                                   nbrRouteItems, theRouteItems );

      // Send the debit-information to the module
      uint32 debitAmount = inReq->getProcessingTime();
      uint32 size = inReq->getNbrReceivedBytes();
      return (sendDebitInformation(user, 
                                   inReq->getID(), 
                                   debitAmount, 
                                   OperationType::ROUTE,
                                   operation, 
                                   size ) );
   } else {
      mc2log << error << "ParserDebitHandler::makeRouteDebit ri, "
             << "not RouteRequest!" << endl;
      return false;
   }
}


bool 
ParserDebitHandler::makeRouteDebit( UserItem* user, RouteRequest* rr, 
                              RouteID oldRouteID, uint32 rerouteReason,
                              const char* extraUserOrigin )
{
   // Check the inparameter
   if ( (rr == NULL) || 
        (rr->getAnswer() == NULL) || 
        (rr->getAnswer()->getPacket() == NULL) ||
        (static_cast<ReplyPacket*>(rr->getAnswer()->getPacket())
            ->getStatus() != StringTable::OK ))  {
      return (false);
   }
      
   char operation[1024];   
   DebitMaking::makeRouteDebit( operation, rr, oldRouteID.getRouteIDNbr(),
                                oldRouteID.getCreationTime(), rerouteReason );

   // Send the debit-information to the module
   uint32 debitAmount = rr->getProcessingTime();
   uint32 size = rr->getNbrReceivedBytes();
   return sendDebitInformation( user, 
                                rr->getID(), 
                                debitAmount, 
                                OperationType::ROUTE,
                                operation,
                                size, extraUserOrigin );
}


bool 
ParserDebitHandler::makeMessageDebit( UserItem* user,
                                      Request* inReq, 
                                      PacketContainer* ansCont,
                                      const char* recipient, 
                                      const char* message,
                                      uint32 size )
{
   if ( dynamic_cast< SMSSendRequest* > ( inReq ) != NULL ) {
      char operation[1024];   
      sprintf(
         operation, "SMS:\"%s\"", message );

      if ( ansCont != NULL &&
           static_cast<ReplyPacket*>( ansCont->getPacket() )->getStatus()
           == StringTable::OK ) 
      {
         if ( size == 0 ) {
            size = static_cast< SMSSendRequest* > ( inReq )->getNbrSMSs();
         }
      } else {
         size = 0;
      }

      // Send the debit-information to the module
      uint32 debitAmount = 0;
      if (ansCont != NULL) {
         debitAmount = ansCont->getPacket()->getDebInfo();
      }
      return (sendDebitInformation(user, 
                                   inReq->getID(), 
                                   debitAmount, 
                                   OperationType::MESSAGE,
                                   operation,
                                   size, 
                                   recipient));
   } else if ( ansCont != NULL &&
               ansCont->getPacket()->getSubType() == 
               Packet::PACKETTYPE_SMSREPLY &&
               (dynamic_cast< SinglePacketRequest* > ( inReq ) != NULL ) )
   {
      // Single SMS
      char operation[1024];   
      sprintf(
         operation, "SMS:\"%s\"", message );

      if ( ansCont != NULL &&
           static_cast<ReplyPacket*>( ansCont->getPacket() )->getStatus()
           == StringTable::OK ) 
      {
         if ( size == 0 ) {
            size = 1;
         }
      } else {
         size = 0;
      }

      // Send the debit-information to the module
      uint32 debitAmount = 0;
      if (ansCont != NULL) {
         debitAmount = ansCont->getPacket()->getDebInfo();
      }
      return sendDebitInformation( user, 
                                   inReq->getID(), 
                                   debitAmount, 
                                   OperationType::MESSAGE,
                                   operation,
                                   size, 
                                   recipient );
   } else if ( ansCont != NULL &&
               ansCont->getPacket()->getSubType() == 
               Packet::PACKETTYPE_SENDEMAILREPLY &&
               ( dynamic_cast< SinglePacketRequest* > ( inReq ) != NULL ||
                 dynamic_cast< RouteMessageRequest* > ( inReq ) != NULL ||
                 dynamic_cast< LocalMapMessageRequest* > ( 
                    inReq ) != NULL ) )
   { // Email
      // Uses phoneNumber as to-address in email and subject as message
      char operation[1024];   
      sprintf( operation, "Email:\"%s\"", message );

      // Send the debit-information to the module
      uint32 debitAmount = 0;
      if (ansCont != NULL) {
         debitAmount = ansCont->getPacket()->getDebInfo();
      }
      return sendDebitInformation( user, 
                                   inReq->getID(), 
                                   debitAmount, 
                                   OperationType::MESSAGE,
                                   operation,
                                   size, 
                                   recipient );
   } else {
      mc2log << error << "ParserDebitHandler::makeMessageDebit, "
             << "not recognized Request type!" << endl;
      return false;
   }
}


bool 
ParserDebitHandler::makeLocalizationDebit( UserItem* user,
                                           const char* itemName,
                                           int32 itemLat, int32 itemLon )
{
   char operation[1024];   
   sprintf( operation, "Localization:\"%s\"(%d,%d)", 
            itemName, itemLat, itemLon );

   uint32 size = 0;
   uint32 debitAmount = 0;
   uint32 messageID = 0;

   // Send the debit-information to the module
   return sendDebitInformation( 
      user, 
      messageID,
      debitAmount, 
      OperationType::LOCALIZATION,
      operation,
      size, 
      "");   
}


bool 
ParserDebitHandler::makeSortDistDebit( UserItem* user,
                                       const char* originName,
                                       int32 originLat, int32 originLon,
                                       uint32 sortDistType,
                                       RouteTypes::routeCostType routeCostType,
                                       ItemTypes::vehicle_t routeVehicle,
                                       uint32 numberReplyItems,
                                       uint32 bestSortDistance,
                                       uint32 bestSortEstimatedTime,
                                       const char* bestItemName,
                                       int32 bestItemLat, int32 bestItemLon )
{
   char operation[1024];   
   sprintf( operation, "SortDist:O\"%s\"(%d,%d):D\"%s\"(%d,%d):"
            ":%d,%d,%d:%u,%u,%u", 
            originName, originLat, originLon,
            bestItemName, bestItemLat, bestItemLon,
            sortDistType, routeCostType, routeVehicle,
            numberReplyItems, bestSortDistance, bestSortEstimatedTime );

   uint32 size = 0;
   uint32 debitAmount = 0;
   uint32 messageID = 0;

   // Send the debit-information to the module
   return sendDebitInformation( 
      user, messageID, debitAmount, 
      OperationType::SORT_DIST,
      operation, size, "");   
}


StringTable::stringCode 
ParserDebitHandler::makeMapDebit( 
   const UserItem* user, const char* extraUserInfo,
   uint32 size, uint32 debitAmount,
   const MC2BoundingBox& bbox,
   const char* inputData,
   uint16 screenX, uint16 screenY,
   bool image, ImageDrawConfig::imageFormat format,
   uint32 routeID, uint32 routeCreationTime,
   bool showMap, bool showPOI, bool showTopographMap,
   bool showRoute, bool showTrafficInfo,
   MC2Coordinate posCoord,
   uint8 dbMask )
{
   StringTable::stringCode res = StringTable::OK;

   char operation[ 256 ];
   // Max Size 5 + 12*4 + 1 + 82 + 1 + 3*6 + 3*12 + 6*6 + 2*12 = 251
   sprintf( operation, "Map:(%d,%d,%d,%d):\"%.80s\":%hu,%hu:%hu:%u:%u,%u:"
            "%hu,%hu,%hu,%hu,%hu:%hu:%d,%d",
            bbox.getMaxLat(), bbox.getMinLon(), 
            bbox.getMinLat(), bbox.getMaxLon(), inputData, 
            screenX, screenY, image, format, routeID, routeCreationTime,
            showMap, showPOI, showTopographMap, showRoute, showTrafficInfo,
            dbMask, posCoord.lat, posCoord.lon );

   uint32 messageID = 0;

   // Send the debit-information to the module
   if ( ! sendDebitInformation( 
           user, messageID, debitAmount, 
           OperationType::MAP,
           operation, size, extraUserInfo ) )
   {
      // TODO: Make sendDebitInformation return why (like timeout)
      res = StringTable::NOTOK;
   }

   return res;
}

bool
ParserDebitHandler::makeAuthorityCheckDebit( 
   UserItem* user,
   const MC2String& userID,
   OperationType::operationType operationType,
   int res,
   const MC2String& info,
   const MC2String& clientType,
   const IPnPort& userIP,
   uint32 serviceTopRegionID )
{
   char operation[ 256 ];
   
   sprintf( operation, "Auth:(%d,%u):\"%.30s\":\"%.60s\":\"%.40s\":\"%.80s\"",
            res, serviceTopRegionID, userIP.toString().c_str(), 
            clientType.c_str(), userID.c_str(), info.c_str() );

   // Not used values
   uint32 messageID = 0;
   uint32 debitAmount = 0;
   uint32 size = 0;

   // Send the debit-information to the module
   return sendDebitInformation( 
      user, messageID, debitAmount, 
      operationType,
      operation, size, NULL/*extraUserInfo*/ );
}

bool
ParserDebitHandler::makePurchaseDebit( 
   UserItem* user,
   const MC2String& userID,
   OperationType::operationType operationType,
   bool success,
   const MC2String& info,
   const IPnPort& userIP,
   const MC2String& packageID )
{
   char operation[ 256 ];
   
   sprintf( operation, "Purchase:(%d):\"%.30s\":\"%.60s\":\"%.80s\"",
            success, userIP.toString().c_str(), 
            packageID.c_str(), info.c_str() );

   // Not used values
   uint32 messageID = 0;
   uint32 debitAmount = 0;
   uint32 size = 0;

   // Send the debit-information to the module
   return sendDebitInformation( 
      user, messageID, debitAmount, 
      operationType,
      operation, size, userID.c_str()/*extraUserInfo*/ );
}

bool
ParserDebitHandler::makeAppStoreVerifyDebit( 
   const UserItem* user,
   const MC2String& userID,
   OperationType::operationType operationType,
   bool success,
   const MC2String& info,
   const MC2String& productID,
   const MC2String& transactionID )
{
   char operation[ 256 ];
   
   sprintf( operation, "AppStore:(%d):\"%.30s\":\"%.30s\":\"%.110s\"",
            success, productID.c_str(),
            transactionID.c_str(), info.c_str() );

   // Not used values
   uint32 messageID = 0;
   uint32 debitAmount = 0;
   uint32 size = 0;

   // Send the debit-information to the module
   return sendDebitInformation( 
      user, messageID, debitAmount, 
      operationType,
      operation, size, userID.c_str()/*extraUserInfo*/ );

}

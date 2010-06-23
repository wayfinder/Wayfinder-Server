/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSER_DEBIT_HANDLER_H
#define PARSER_DEBIT_HANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include "RouteID.h"
#include "RouteTypes.h"
#include "ItemTypes.h"
#include "StringTable.h"
#include "MC2BoundingBox.h"
#include "MC2Coordinate.h"
#include "OperationTypes.h"
#include "ImageDrawConfig.h"
#include "IPnPort.h"

// Forward declarations
class UserItem;
class Request;
class PacketContainer;
class ProximityRequest;
class RouteItem;
class RouteRequest;
class SearchResultRequest;
class VanillaMatch;
class SearchRequestParameters;

/**
 * Class handling debit, store information in database, operations.
 */
class ParserDebitHandler : public ParserHandler {
public:
   /**
    * Constructor.
    */
   ParserDebitHandler( ParserThread* thread,
                       ParserThreadGroup* group );

   /**
    * Send debiting information for one search for the given user
    * to the UserModule.
    *
    * @param user             The user to debit.
    * @param inReq            The Request to debit for, 
    *                         SearchRequest, RouteRequest and 
    *                         SMSSendRequest are supported.
    * @param ansCont          The answer of the Request, may be NULL.
    * @param locationString   The location searched for in 
    *                         SearchRequest.
    * @param searchString     The searchString in the SearchRequest.
    * @param extraUserOrigin Extra information about user, default "".
    * @param extraSearchString an extra string to be attached to search 
    *        debit
    * @return True if debited ok, false if not.
    */
   bool makeSearchDebit( UserItem* user, Request* inReq, 
                         PacketContainer* ansCont, 
                         const char* const locationString, 
                         const char* const searchString,
                         const char* extraUserOrigin = NULL,
                         const char* extraSearchString = NULL );

   /**
    * Send debiting information for one search for the given user
    * to the UserModule.
    *
    * @param user The user to debit.
    * @param inReq The ProximityRequest to debit for.
    * @param ansCont The answer of the Request
    * @param searchString The searchString in the ProximityRequest. NULL
    *                     if searchString was not used in search.
    * @param lat The latitude of the centerpoint in the circle 
    *            proximity area.
    * @param lon The longitude of the centerpoint in the circle 
    *            proximity area.
    * @param distance The distance from the centerpoint searched.
    * @return True if debited ok, false if not.
    */
   bool makeProximityDebit( UserItem* user, ProximityRequest* inReq, 
                            PacketContainer* ansCont, 
                            const char* const searchString,
                            int32 lat, int32 lon, 
                            uint32 distance );

   /**
    * Send debiting information for one proximity search
    * for the given user to the UserModule.
    *
    * @param user         The user to debit.
    * @param inReq        The SearchRequest to debit for.
    * @param params       The parameters used when searching.
    * @param searchString The searchString in the ProximityRequest. NULL
    *                     if searchString was not used in search.
    * @param center       The center.
    * @param distance     The distance from the centerpoint searched.
    * @return True if debited ok, false if not.
    */
   bool makeProximityDebit( UserItem* user,
                            const SearchResultRequest* inReq, 
                            const SearchRequestParameters& params,
                            const char* searchString,
                            const MC2Coordinate& center,
                            uint32 distance );

   /**
    * Send debiting information for one Route for the given user
    * to the UserModule.
    *
    * @param user             The user to debit.
    * @param inReq            The Request to debit for, 
    *                         RouteRequest is supported.
    * @param ansCont          The answer of the Request, may be NULL.
    * @param originVM         The origin of the route.
    * @param destinationVM    The destination of the route.
    * @return True if debited ok, false if not.
    */
   bool makeRouteDebit( UserItem* user, Request* inReq, 
                        PacketContainer* ansCont,
                        const VanillaMatch* const originVM,
                        const VanillaMatch* const destinationVM );

   /**
    * Send debit infromation for one route for the given user
    * to the UserModule.
    *
    * @param user             The user to debit.
    * @param inReq            The Request to debit for, RouteRequest is 
    *                         supported.
    * @param ansCont          The answer of the Request, may be NULL.
    * @param nbrRouteItems    The number of items in theRouteItems.
    * @param theRouteItems    The RouteItem array.
    * @return True if debited ok, false if not.
    */
   bool makeRouteDebit( UserItem* user, Request* inReq, 
                        PacketContainer* ansCont,
                        uint32 nbrRouteItems,
                        RouteItem** theRouteItems );


   /**
    * Debits a route.
    *
    * @param user The user to debit.
    * @param rr The route to debit.
    * @param oldRouteID The route thet this is a continuation of.
    * @param reason The reason for this reroute, if it is a reroute.
    * @param extraUserOrigin Extra information about user, default "".
    */
   bool makeRouteDebit( UserItem* user, RouteRequest* rr, 
                        RouteID oldRouteID = RouteID( 0, 0 ),
                        uint32 reason = 0,
                        const char* extraUserOrigin = NULL );

   /**
    * Send debiting information for one (free text) SMS for the 
    * given user to the UserModule or debiting information for
    * one email for the given user to the UserModule. 
    *
    * @param user             The user to debit.
    * @param inReq            The Request to debit for, 
    *                         SearchRequest, RouteRequest and 
    *                         SMSSendRequest are supported.
    * @param ansCont          The answer of the Request, may be NULL.
    * @param recipient        The cellular number that the SMS was 
    *                         sent to or the email-address of the
    *                         receiver.
    * @param message          The message sent in the SMS or a short
    *                         text describing the email, subject.
    * @param   size   The size of the message, number of SMS or size
    *                 in bytes. If 0 then try to get it from other 
    *                 inparamters. Default 0.
    * @return True if debited ok, false if not.
    */
   bool makeMessageDebit( UserItem* user, Request* inReq, 
                          PacketContainer* ansCont,
                          const char* recipient, 
                          const char* message,
                          uint32 size = 0 );

   /**
    * Send debit information for one localization of an item.
    *
    * @param user     The user to debit.
    * @param itemName The name of the item.
    * @param itemLat  The latitude of the item.
    * @param itemLon  The longitude of the item.
    * @param True if debited ok, false if not.
    */
   bool makeLocalizationDebit( UserItem* user,
                               const char* itemName,
                               int32 itemLat, int32 itemLon );

   /**
    * Send debit information for one sort dist for the giver user.
    *
    * @param user       The user to debit.
    * @param originName The name of the originItem.
    * @param originLat  The latitude of the originItem.
    * @param originLon  The longitude of the originItem.
    * @param sortDistType  The distance sorting type, radius or route.
    * @param routeCostType The route cost type.
    * @param routeVehicle  The route vehicle.
    * @param numberReplyItems The number of items in the reply.
    * @param bestSortDistance The distance of the first item in reply.
    * @param bestSortEstimatedTime The estimated time of the first item
    *                              in reply.
    * @param bestItemName The name of the first item in reply.
    * @param bestItemLat The latitude of first item in reply.
    * @param bestItemLon The longitude of first item in reply.
    * @param True if debited ok, false if not.
    */
   bool makeSortDistDebit( UserItem* user,
                           const char* originName,
                           int32 originLat, int32 originLon,
                           uint32 sortDistType,
                           RouteTypes::routeCostType routeCostType, 
                           ItemTypes::vehicle_t routeVehicle,
                           uint32 numberReplyItems,
                           uint32 bestSortDistance,
                           uint32 bestSortEstimatedTime,
                           const char* bestItemName,
                           int32 bestItemLat, int32 bestItemLon );

   /**
    * Send debit for a map.
    *
    * @param user       The user to debit.
    * @param extraUserInfo Extra info about the user.
    * @param size       The number of bytes sent in reply.
    * @param debitAmount The amount of debit.
    * @param bbox       The boundingbox of the map.
    * @param inputData  The user data for the map, max 80 chars.
    * @param screenX    The requested screen size.
    * @param screenY    The requested screen size.
    * @param image      If an image was returned if not GfxFeatMap.
    * @param format     The Image format.
    * @param routeID    The ID of the route, 0 if no route.
    * @param routeCreationTime The creation time of the route, 0 if no 
    *                          route.
    * @param showMap    If map shown in reply.
    * @param showPOI    If POI shown in reply.
    * @param showTopographMap If TopographMap shown in reply.
    * @param showRoute  If Route shown in reply.
    * @param showTrafficInfo if TrafficInfo shown in reply.
    * @param posCoord   The position shown in map, if any, default
    *                   invalidCoordiante.
    * @param dbMask     The POI-dbs used in the map, default UNKNOWN.
    * @return The status code, OK if debited ok.
    */
   StringTable::stringCode makeMapDebit( 
      const UserItem* user, const char* extraUserInfo,
      uint32 size, uint32 debitAmount,
      const MC2BoundingBox& bbox,
      const char* inputData,
      uint16 screenX, uint16 screenY,
      bool image, ImageDrawConfig::imageFormat format,
      uint32 routeID, uint32 routeCreationTime,
      bool showMap, bool showPOI, bool showTopographMap,
      bool showRoute, bool showTrafficInfo,
      MC2Coordinate posCoord = MC2Coordinate::invalidCoordinate,
      uint8 dbMask = SearchTypes::UNKNOWN );


   /**
    * Send debit information for one external authority check.
    *
    * @param user The user to debit.
    * @param userID The identfication for the user.
    * @param operationType The type of operation done.
    * @param res The return from ParserThread::internalCheckService.
    * @param info The information of the operation, especially why user
    *             wasn't allowed.
    * @param clientType The client type.
    * @param userIP The client IP.
    * @param serviceTopRegionID The top region requested by user, if any.
    * @param True if debited ok, false if not.
    */
   bool makeAuthorityCheckDebit( 
      UserItem* user,
      const MC2String& userID,
      OperationType::operationType operationType,
      int res,
      const MC2String& info,
      const MC2String& clientType,
      const IPnPort& userIP,
      uint32 serviceTopRegionID = MAX_UINT32 );

   /**
    * Send debit information for one external purchase.
    *
    * @param user The user to debit.
    * @param userID The identfication for the user.
    * @param operationType The type of operation done.
    * @param success If the purchase succeeded.
    * @param info The information of the operation, especially why purchase
    *             failed.
    * @param userIP The client IP.
    * @param packageID The thing purchased by user, or tried to.
    * @param True if debited ok, false if not.
    */
   bool makePurchaseDebit( 
      UserItem* user,
      const MC2String& userID,
      OperationType::operationType operationType,
      bool success,
      const MC2String& info,
      const IPnPort& userIP,
      const MC2String& packageID );

   /**
    * Send debit information for a App Store verification, ei. purchase.
    *
    * @param user The user to debit.
    * @param operationType The type of operation done.
    * @param res The return from ParserAppStoreHandler::internalVerifyReceipt.
    * @param info The information of the operation, especially why user
    *             wasn't allowed.
    * @param userIP The client IP.
    * @param productID The id of the product verified.
    * @param transactionID The transaction id of the verification.
    * @param True if debited ok, false if not.
    */
   bool makeAppStoreVerifyDebit( 
      const UserItem* user,
      const MC2String& userID,
      OperationType::operationType operationType,
      bool success,
      const MC2String& info,
      const MC2String& productID,
      const MC2String& transactionID );

private:

   /**
    * Send the debiting information to the UserModule. This method
    * is private, since it is only called by the "wrapper-methods"
    * {\tt make*Debit(..)}.
    * If isCheckUser is set in the ParserThread then this method return
    * true without doing anything.
    *
    * @param user            The user that have inserted the request
    *                        into the server.
    * @param messageID       A ID of this request that will be sent to
    *                        the UserModule. This might be RequestID 
    *                        or RouteID.
    * @param operationType   The type of operation that is performed,
    *                        the types in DebitRequestPacket are 
    *                        supported.
    * @param debitText       The text that will be inserted into the
    *                        UserModule.
    * @param size            The sise of the request that is sent to
    *                        user (e.g. the number of SMS:es). Optional
    *                        parameter with defaultvalue = 0.
    * @param extraUserOrigin If to use a extra string as origin text.
    *                        E.g. phonenumber or IP-address.
    * @return True if the debit information is registered in the 
    *         module, false otherwise.
    */
   bool sendDebitInformation( const UserItem* user, 
                              uint32 messageID,
                              uint32 debitAmount, 
                              uint32 operationType, 
                              char* debitText,
                              uint32 size = 0, 
                              const char* extraUserOrigin = NULL );
};

#endif // PARSER_DEBIT_HANDLER_H


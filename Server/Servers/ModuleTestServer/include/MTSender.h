/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MTSENDER_H
#define MTSENDER_H

#include "DatagramSocket.h"
#include "TCPSocket.h"
#include "MapPacket.h"
#include "RoutePacket.h"
#include "SearchPacket.h"
#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "TestPacket.h"
#include "UserPacket.h"
#include "UserData.h"
#include "AllMapPacket.h"
#include "CoordinatePacket.h"
#include "SendEmailPacket.h"
#include "AddDisturbancePacket.h"
#include "RemoveDisturbancePacket.h"
#include "DisturbanceSubscriptionPacket.h"
#include "PacketContainer.h"
#include "ServerProximityPackets.h"
#include "CoordinateOnItemPacket.h"

#include "Item.h"
#include "multicast.h"
#include "MTLog.h"

#define DEFAULT_MT_SERVER_PORT   8180

/**
  *  This class describes a server used for testing the modules.
  *
  */
class MTSender
{
   public:
      
   /**
    *  Default constructor
    */
   MTSender(uint16 port, MTLog* log);
   
   
   /**
     *  Destrucor.
     */
   virtual ~MTSender();
   
   
   /**
     *  Sends a packet with random parameters.
     *  @param   type  The type of packet to send.
     *  @return  True if successfull, false otherwise.
     */
   void sendRandomPacket(byte type);
   
   
   /**
     * @param mapID is the id of the map to load.
     * @param moduleType is the module to send the loadmaprequest to.
     */
   void sendLoadMapRequest(uint32 mapID, moduletype_t moduleType);

   /**
     * @param mapID is the id of the map to delete.
     * @param moduleType is the module to send the deletemaprequest to.
     */
   void sendDeleteMapRequest(uint32 mapID, moduletype_t moduleType);
   
   
   void sendCoveredIDsRequest(int32 lat, int32 lon,
                              uint32 outerR, uint32 innerR,
                              uint16 startA, uint16 stopA);
   
   /**
    */
   void sendCoordinateRequest( int32 lat, int32 lon, uint16 angle);

   
   /**
    */
   void sendAllMapRequest();

   
   /**
    */
   void sendSMSRequest(char* senderPhone, char* recipientPhone,
                       char* data);
   
   /**
    */
   void sendEmailRequest(char* adress, char* fromAdress, 
                         char* subject, char* data);
   
   /**
    */
   void sendTestRequest(uint32 mapID);
   
   
   /**
    *  Sends a request for a map to the mapmodule.
    *  @param   mapType  The type (according to ...) of the map
    *                    to request.
    *  @param   mapID    The ID of the map
    *  @return  True if successfull, false otherwise.
    */
   void sendMapRequest(byte mapType, uint32 mapID);
   
  /**
    *  Requests coordinates on an item from the mapmodule.
    * 
    *  @param   mapID    The ID of the map which contains the
    *                    item.
    *  @param   itemID   The ID of the item.
    *  @return  True if successfull, false otherwise.
    */
   void sendCoordinateOnItemRequest(uint32 mapID, uint32 itemID);
   
   /**
    *  Requests an expansion of one item from the mapmodule.
    *  @param   mapID    The ID of the map which contains the
    *                    item.
    *  @param   itemID   The ID of the item.
    *  @return  True if successfull, false otherwise.
    */
   void sendExpandItemRequest(uint32 mapID, uint32 itemID);
   
   
   /**
    *  Requests a route from the routemodule.
    *  @param   customerParam  Customer parameters.
    *  @param   vehicleParam   Vehicle parameters.
    *  @param   time           Start or stop time.
    *  @param   number         Number of origins and destinations.
    *  @param   vector         Vector containing origin and
    *                          destinations (12 bytes each).
    *  @return  True if successfull, false otherwise.
    */
   void sendRouteRequest(  byte     routeType,
                           uint32   customerParam,
                           uint32   vehicleParam,
                           bool     isStartTime,
                           uint32   time,
                           uint32   number,
                           uint32*  vector);
   
   
   /**
    *  Sends a VanillaSearchRequestPacket to the searchmodule.
    *  @param   numMapID       The number of Map IDs.
    *  @param   mapID          The ID of the map to search in.
    *  @param   zipCode        Zip code of the area to search.
    *                          May be empty.
    *  @param   city           City to search. May be empty.
    *  @param   searchString   String to search for.
    *  @param   nbrHits        The number of hits to request.
    *  @param   stringPart     ???
    *  @param   sortingType    ???
    *  @param   categoryType   ???
    *  @param   language       Preferred language.
    *  @param   dbMask         The databases to use.
    */
   void sendVanillaSearchRequest(uint32   numMapID,
                                 uint32*  mapID,
                                 char*    zipCode,
                                 char*    city,
                                 char*    searchString,
                                 uint8    nbrHits,
                                 SearchTypes::StringMatching matchType,
                                 SearchTypes::StringPart     stringPart,
                                 SearchTypes::SearchSorting  sortingType,
                                 uint16   categoryType,
                                 uint16   language,
                                 uint8    dbMask );
   
   /**
    *  Sends a UserSearchRequestPacket the searchmodule.
    *  @param   numMapID       The number of Map IDs.
    *  @param   mapID          The ID of the map to search in.
    *  @param   zipCode        Zip code of the area to search.
    *                          May be empty.
    *  @param   city           City to search. May be empty.
    *  @param   searchString   String to search for.
    *  @param   nbrHits        The number of hits to request.
    *  @param   stringPart     ???
    *  @param   sortingType    ???
    *  @param   categoryType   ???
    *  @param   language       Preferred language.
    *  @param   nbrSortedHits  The number of hits to be sorted.
    *  @param   dbMask         The databases to use.
    */
   void sendUserSearchRequest(uint32   numMapID,
                              uint32*  mapID,
                              char*    zipCode,
                              char*    city,
                              char*    searchString,
                              uint8    nbrHits,
                              SearchTypes::StringMatching matchType,
                              SearchTypes::StringPart stringPart,
                              SearchTypes::SearchSorting sortingType,
                              uint16   categoryType,
                              uint16   language,
                              uint8    nbrSortedHits,
                              uint16   editDistanceCutoff,
                              uint8    dbMask,
                              uint8    nbrMasks,
                              uint32*  maskItemIDs);


   /**
    *  Sends an OverviewSearchRequestPacket the searchmodule.
    *  @param   packetID is the id of the packet in the request
    *  @param   requestID is the id of the request sending the packet
    *  @param   nbrLocations
    *  @param   location is the string to match for locations
    *  @param   locationType is locationType mask.
    *  @param   requestedLanguage
    *  @param   mapID
    *  @param   matching
    *  @param   stringPart
    *  @param   uniqueOrFull if true, only unique locations or those matching
    *                        the string fully are returned.
    */


   void sendOverviewSearchRequest(uint32           nbrLocations, 
                                  const char**     locations, 
                                  uint32           locationType, 
                                  uint32           requestedLanguage, 
                                  uint32           mapID, 
                                  SearchTypes::StringMatching matching, 
                                  SearchTypes::StringPart     stringPart,
                                  SearchTypes::SearchSorting sortingType,
                                  bool             uniqueOrFull,
                                  uint8            dbMask,
                                  uint8            minNbrHits );


   /**
    * Sends a UserLoginPacket.
    * @param login The loginName.
    * @param passwd The password.
    */
   void sendUserLoginRequest( const char* login, const char* passwd );

   
   /**
    * Sends a sessioncleanuppacket.
    * @param sessionID ID.
    * @param sessionKey Key.
    */
   void sendSessionCleanUpRequest( const char* sessionID, 
                                   const char* sessionKey );

   /**
    * Sends a GetUserNavDestinationRequest.
    * @param onlyUnSentNavDestinations Get only unsent 
    *                                  UserNavDestinations.
    * @param onlyNavigatorLastContact Get only UserNavDestinations for
    *                                 UserNavigators where
    *                                 last contact was successfull.
    * @param navID Get only UserNavDestinations for a specific 
    *              UserNavigator, 0 means any UserNavigator.
    * @param navAddress Get only UserNavDestinations with a specific address.
    */
   void sendGetUserNavDestinationRequest( const char* onlyUnsent, 
                                          const char* onlyContact,
                                          const char* navID,
                                          const char* navAddress );


   /**
    * Sends a AddUserNavDestinationRequest. With the paramets.
    */
   void sendAddUserNavDestinationRequest( const char* navID, 
                                          const char* sent,
                                          const char* created,
                                          const char* type,
                                          const char* name,
                                          const char* senderID,
                                          const char* lat,
                                          const char* lon );

      
   /**
    * Sends a AddDisturbanceRequest.
    * @param mapID The map to add/edit a disturbance on.
    * @param nodeID The street segment node to add/edit a disturbance on.
    * @param distID The id of the disturbance to edit.
    */
   void sendAddDisturbanceRequest(uint32 mapID, uint32 nodeID, uint32 distID);

   /**
    * Sends a RemoveDisturbanceRequest.
    * @param mapID The map to remove a disturbance from.
    * @param nodeID The street segment node to remove a disturbance from.
    * @param distID The id of the disturbance to remove.
    */
   void sendRemoveDisturbanceRequest(uint32 datexID);

   /**
    * Sends a GetDisturbanceRequest.
    * @param mapID The map to get disturbances from.
    * @param nodeID The street segment node to get disturbances from.
    * @param distID The id of a disturbance to get info from.
    */
   void sendGetDisturbanceRequest(uint32 mapID, uint32 nodeID, uint32 distID);
   
   /**
    * Sends a DisturbanceSubscriptionRequest.
    * @param mapID The map to get disturbances from.
    * @param subscribe True to start subscription, false to unsubscribe.
    * @param minRoadSize Not used yet. Set it to MAX_BYTE. 
    */
   void sendDisturbanceSubscriptionRequest(uint32 mapID,
                                           bool subscribe,
                                           byte minRoadSize);

   /**
    *   Sends an EdgeNodesRequestPacket.
    *   @param mapID The map to get the edgenodes from.
    *   @param level The level where the nodes should be.
    */
   void sendEdgeNodesRequest(uint32 mapID,
                             int level);

   /**
    *   Sends a RouteExpandItemRequestPacket.
    *   @param mapID The map where the item is.
    *   @param itemID The item id to expand.
    */
   void sendRouteExpandItemRequest(uint32 mapID,
                                   uint32 itemID);

   /**
    *   Sends a MapSubscriptionRequestPacket to the TestModule.
    *   @param mapID Mapid to send.
    *   @param ip    IP to contact.
    *   @param port  Port to contact on above ip.
    */
   void sendMapSubscriptionRequestPacket(uint32 mapID,
                                         uint32 ip,
                                         uint32 port,
                                         uint32 serviceID);
   
   protected:
   
   /**
    *  This server's IP address
    */
   uint32 myIP;
   
   
   /**
    *  This server's ports
    */
   uint16 myPort;
   
   
   /**
    *  The multicast addresses
    */
   static uint32 mapModuleIP;
   static uint32 routeModuleIP;
   static uint32 searchModuleIP;
   static uint32 testModuleIP;
   static uint32 userModuleIP;
   static uint32 SMSModuleIP;
   static uint32 emailModuleIP;
   static uint32 trafficCostModuleIP;
   static uint32 gfxModuleIP;
   

   /**
    *  The multicast ports
    */
   static uint16 mapModulePort;
   static uint16 routeModulePort;
   static uint16 searchModulePort;
   static uint16 testModulePort;
   static uint16 userModulePort;
   static uint16 SMSModulePort;
   static uint16 emailModulePort;
   static uint16 trafficCostModulePort;
   static uint16 gfxModulePort;
   
   /**
    *  The log object
    */
   MTLog* log;
   
   
   /**
    *  The ID of the last send packet
    */
   uint16 packID;
   
   
   /**
    *  The socket used to send UDP packets
    */
   DatagramSender* UDPSender;
   
   
}; // MTSender

#endif // MTSENDER_H



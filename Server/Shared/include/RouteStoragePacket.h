/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTESTORAGEPACKET_H
#define ROUTESTORAGEPACKET_H

#include "config.h"
#include "Packet.h"
#include "RoutePacket.h"
#include "UserEnums.h"

class StringCode;
class DriverPref;

// Priorities of RouteStoragePackets
#define ROUTE_STORAGE_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define ROUTE_STORAGE_REPLY_PRIO   DEFAULT_PACKET_PRIO

// Paket in Base64 encoding (4/3 * MAX_PACKETSIZE + 1024)

/**
 * Request to add a route to the route storage.
 *
 * After the normal RequestPacket header the request packet contains:
 * \begin{tabular}{lll}
 *    Pos                         & Size     & Destription \\ \hline
 *    REQUEST_HEADER_SIZE         & 4 bytes  & RouteID \\
 *    +4                          & 4 bytes  & UIN \\
 *    +8                          & 4 bytes  & validUntil \\
 *    +12                         & 4 bytes  & createTime \\
 *    +16                         & 4 bytes  & originLat \\
 *    +20                         & 4 bytes  & destinationLat \\
 *    +20                         & 4 bytes  & originLon \\
 *    +24                         & 4 bytes  & destinationLon \\
 *    +28                         & 4 bytes  & originMapID \\
 *    +32                         & 4 bytes  & destinationMapID \\
 *    +36                         & 4 bytes  & originItemID \\
 *    +40                         & 4 bytes  & destinationItemID \\
 *    +44                         & 2 bytes  & originOffset \\
 *    +46                         & 2 bytes  & destinationOffset \\
 *    +48                         & 4 bytes  & flags, isReRoute \\
 *    +52                         & 4 bytes  & routePack->getLength \\
 *    +56                         & 2 bytes  & strlen extraUserinfo \\
 *    +58                         & 2 bytes  & originAngle \\
 *    +60                         & 4 bytes  & driverPref size \\
 *    +64                         & 4 bytes  & urmask size \\
 *    +68                         & extraUserinfo bytes & extraUserinfo \\
 *                                & routePack bytes & routePack \\
 *                                & driverPref bytes & driverPref \\
 *                                & urmask size & urmask \\
 *    \end{tabular}
 *
 */
class RouteStorageAddRouteRequestPacket : public RequestPacket {
   public:
      /**
       * Constructor.
       *
       * @param routePack The route to store.
       * @param UIN The user of the route.
       * @param extraUserinfo Extra information about the user.
       *        Things like "Navigatoraddress=46708111111&Navigatorid=6"
       *        may be usefull.
       * @param validUntil Time when route can be considered "old" in UTC.
       *        Usually (currentTime + Total time in route)*N where N is
       *        safetyfactor.
       * @param createTime The time, in UTC, when the route was created.
       *        Used to determine if the route contains current mc2-maps.
       * @param originLat The latitude of the route's origin.
       * @param originLon The longitude of the route's origin.
       * @param originAngle The angle of the route's origin.
       * @param originMapID The mapID of the route's origin.
       * @param originItemID The itemID of the route's origin.
       * @param originOffset The offset of the route's origin.
       * @param destinationLat The latitude of the route's destination.
       * @param destinationLon The longitude of the route's destination.
       * @param destinationMapID The mapID of the route's destination.
       * @param destinationItemID The itemID of the route's destination.
       * @param destinationOffset The offset of the route's destination.
       * @param driverPref The settings for the route.
       * @param urmask The urmask used for the route.
       * @param isReRoute If this is a reroute using route storage coords.
       */
      RouteStorageAddRouteRequestPacket( const RouteReplyPacket* routePack,
                                         uint32 routeID,
                                         uint32 UIN,
                                         const char* const extraUserinfo,
                                         uint32 validUntil,
                                         uint32 createTime,
                                         int32 originLat,
                                         int32 originLon,
                                         uint16 originAngle,
                                         uint32 originMapID,
                                         uint32 originItemID,
                                         uint16 originOffset,
                                         int32 destinationLat,
                                         int32 destinationLon,
                                         uint32 destinationMapID,
                                         uint32 destinationItemID,
                                         uint16 destinationOffset,
                                         const DriverPref& driverPref,
                                         UserEnums::URType urmask,
                                         bool isReRoute );
      
      
      /**
       * The RouteReplyPacket, allocates an new packet and returns it.
       */
      RouteReplyPacket* getRoutePacket() const;


      /**
       * The routeID.
       */
      uint32 getRouteID() const;

      
      /**
       * The UIN.
       */
      uint32 getUIN() const;

      
      /**
       * The extraUserinfo, returns a pointer directly into the packet
       * so don't delete it.
       */
      const char* getExtraUserinfo() const;

      
      /**
       * The validUntil time.
       */
      uint32 getValidUntil() const;
      
      
      /**
       * The createTime.
       */
      uint32 getCreateTime() const;


      /**
       * The originLat.
       */
      uint32 getOriginLat() const;


      /**
       * The destinationLat.
       */
      uint32 getDestinationLat() const;


      /**
       * The originLon.
       */
      uint32 getOriginLon() const;

      /**
       * The originAngle
       */
      uint16 getOriginAngle() const;
      
      /**
       * The destinationLon.
       */
      uint32 getDestinationLon() const;


      /**
       * The originMapID.
       */
      uint32 getOriginMapID() const;


      /**
       * The destinationMapID.
       */
      uint32 getDestinationMapID() const;
      
      
      /**
       * The originItemID.
       */
      uint32 getOriginItemID() const;


      /**
       * The destinationItemID.
       */
      uint32 getDestinationItemID() const;


      /**
       * The originOffset.
       */
      uint16 getOriginOffset() const;
      
      
      /**
       * The destinationOffset.
       */
      uint16 getDestinationOffset() const;

      /**
       * Fills in the driverPref.
       */
      void getDriverPrefs( DriverPref& driverPref ) const;

      /**
       * Get the urmask used for the route.
       */
      UserEnums::URType getUrmask() const;

      /**
       * Get the isReRoute.
       */
      bool getIsReRoute() const;

   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         routeID_POS             = REQUEST_HEADER_SIZE,
         UIN_POS                 = routeID_POS + 4,
         validUntil_POS          = UIN_POS + 4,
         createTime_POS          = validUntil_POS + 4,
         
         originLat_POS           = createTime_POS + 4,
         destinationLat_POS      = originLat_POS + 4,
         originLon_POS           = destinationLat_POS + 4,
         destinationLon_POS      = originLon_POS + 4,
         originMapID_POS         = destinationLon_POS + 4,
         destinationMapID_POS    = originMapID_POS + 4,
         originItemID_POS        = destinationMapID_POS + 4,
         destinationItemID_POS   = originItemID_POS + 4,
         originOffset_POS        = destinationItemID_POS + 4,
         destinationOffset_POS   = originOffset_POS + 2,
         isReRoute_POS           = destinationOffset_POS + 2,

         routePackLength_POS     = isReRoute_POS + 4,
         strlenExtraUserinfo_POS = routePackLength_POS + 4,

         originAngle_POS         = strlenExtraUserinfo_POS + 2,
         driverPrefSize_POS      = originAngle_POS + 2,
         urmaskSize_POS          = driverPrefSize_POS + 4,
         
         endStatic_POS           = urmaskSize_POS + 4
      };
};


/**
 * The reply to a RouteStorageAddRouteRequestPacket.
 */
class RouteStorageAddRouteReplyPacket : public ReplyPacket {
   public:
      /**
       * Constructor, sets the status.
       * @param inPacket The RouteStorageAddRouteRequestPacket that this
       *        is a reply to.
       * @param status The stringCode of the reply status.
       */
      RouteStorageAddRouteReplyPacket( 
         const RouteStorageAddRouteRequestPacket* inPacket,
         StringCode status );
};


/**
 * Request for a specific stored route.
 *
 * After the normal RequestPacket header the request packet contains:
 * \begin{tabular}{lll}
 *    Pos                         & Size     & Destription \\ \hline
 *    REQUEST_HEADER_SIZE         & 4 bytes  & routeID \\
 *    +4                          & 4 bytes  & createTime \\
 *    \end{tabular}
 *
 */
class RouteStorageGetRouteRequestPacket :  public RequestPacket {
   public:
      /**
       * Constructor.
       *
       * @param routeID The ID of the route to get.
       * @param createTime The creation time of the route.
       */
      RouteStorageGetRouteRequestPacket( uint32 routeID,
                                         uint32 createTime );
      
      
      /**
       * The routeID.
       */
      uint32 getRouteID() const;

      
      /**
       * The createTime.
       */
      uint32 getCreateTime() const;


   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         routeID_POS             = REQUEST_HEADER_SIZE,
         createTime_POS          = REQUEST_HEADER_SIZE + 4,
         
         endStatic_POS           = REQUEST_HEADER_SIZE + 4 + 4
      };
};


/**
 * The reply to a RouteStorageGetRouteRequestPacket.
 * After the normal RequestPacket header the request packet contains:
 * \begin{tabular}{lll}
 *    Pos                         & Size     & Destription \\ \hline
 *    REPLY_HEADER_SIZE           & 4 bytes  & RouteID \\
 *    +4                          & 4 bytes  & UIN \\
 *    +8                          & 4 bytes  & validUntil \\
 *    +12                         & 4 bytes  & createTime \\
 *    +18                         & 4 bytes  & routePack->getLength \\
 *    +16                         & 2 bytes  & strlen extraUserinfo \\
 *    +20                         & extraUserinfo bytes & extraUserinfo \\
 *                                & routePack bytes & routePack \\
 *    \end{tabular}
 *
 */
class RouteStorageGetRouteReplyPacket : public ReplyPacket {
   public:
      /**
       * @param inPacket The RouteStorageGetRouteRequestPacket that this
       *        is a reply to.
       * @param status The stringCode of the reply status.
       * @param routePack The route stored, or NULL if none found.
       * @param UIN The user of the route, or 0 if none found.
       * @param extraUserinfo Extra information about the user, or empty
       *        string if none found.
       * @param validUntil Time when route can be considered "old" in UTC.
       * @param createTime The time, in UTC, when the route was created.
       *        Used to determine if the route contains current mc2-maps.
       * @param originLat The latitude of the route's origin.
       * @param originLon The longitude of the route's origin.
       * @param originAngle The angle of the route's origin.
       * @param originMapID The mapID of the route's origin.
       * @param originItemID The itemID of the route's origin.
       * @param originOffset The offset of the route's origin.
       * @param destinationLat The latitude of the route's destination.
       * @param destinationLon The longitude of the route's destination.
       * @param destinationMapID The mapID of the route's destination.
       * @param destinationItemID The itemID of the route's destination.
       * @param destinationOffset The offset of the route's destination.
       * @param driverPref The settings for the route.
       * @param urmask The urmask used for the route.
       */
      RouteStorageGetRouteReplyPacket( 
         const RouteStorageGetRouteRequestPacket* inPacket,
         StringCode status,
         const RouteReplyPacket* routePack,
         uint32 routeID,
         uint32 UIN,
         const char* const extraUserinfo,
         uint32 validUntil,
         uint32 createTime,
         int32 originLat,
         int32 originLon,
         uint16 originAngle,
         uint32 originMapID,
         uint32 originItemID,
         uint16 originOffset,
         int32 destinationLat,
         int32 destinationLon,
         uint32 destinationMapID,
         uint32 destinationItemID,
         uint16 destinationOffset,
         const DriverPref& driverPref,
         UserEnums::URType urmask );
      
      
      /**
       * The RouteReplyPacket, allocates an new packet and returns it.
       */
      RouteReplyPacket* getRoutePacket() const;

      /**
       * Get the length of the RouteReplyPacket. Mostly to check if 
       * it is zero.
       */
      uint32 getRoutePacketLength() const;

      /**
       * The routeID.
       */
      uint32 getRouteID() const;


      /**
       * The UIN.
       */
      uint32 getUIN() const;
      
      
      /**
       * The extraUserinfo, returns a pointer directly into the packet
       * so don't delete it.
       */
      const char* getExtraUserinfo() const;
      
      
      /**
       * The validUntil time.
       */
      uint32 getValidUntil() const;
      
      
      /**
       * The createTime.
       */
      uint32 getCreateTime() const;


      /**
       * The originLat.
       */
      uint32 getOriginLat() const;


      /**
       * The destinationLat.
       */
      uint32 getDestinationLat() const;


      /**
       * The originLon.
       */
      uint32 getOriginLon() const;

      /**
       * The originAngle
       */
      uint16 getOriginAngle() const;      
      
      /**
       * The destinationLon.
       */
      uint32 getDestinationLon() const;


      /**
       * The originMapID.
       */
      uint32 getOriginMapID() const;


      /**
       * The destinationMapID.
       */
      uint32 getDestinationMapID() const;
      
      
      /**
       * The originItemID.
       */
      uint32 getOriginItemID() const;


      /**
       * The destinationItemID.
       */
      uint32 getDestinationItemID() const;


      /**
       * The originOffset.
       */
      uint16 getOriginOffset() const;
      
      
      /**
       * The destinationOffset.
       */
      uint16 getDestinationOffset() const;

      /**
       * Fills in the driverPref.
       */
      void getDriverPrefs( DriverPref& driverPref ) const;

      /**
       * Get the urmask used for the route.
       */
      UserEnums::URType getUrmask() const;

   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         routeID_POS             = REPLY_HEADER_SIZE,
         UIN_POS                 = routeID_POS + 4,
         validUntil_POS          = UIN_POS + 4,
         createTime_POS          = validUntil_POS + 4,
         
         originLat_POS           = createTime_POS + 4,
         destinationLat_POS      = originLat_POS + 4,
         originLon_POS           = destinationLat_POS + 4,
         destinationLon_POS      = originLon_POS + 4,
         originMapID_POS         = destinationLon_POS + 4,
         destinationMapID_POS    = originMapID_POS + 4,
         originItemID_POS        = destinationMapID_POS + 4,
         destinationItemID_POS   = originItemID_POS + 4,
         originOffset_POS        = destinationItemID_POS + 4,
         destinationOffset_POS   = originOffset_POS + 2,

         routePackLength_POS     = destinationOffset_POS + 2,
         strlenExtraUserinfo_POS = routePackLength_POS + 4,

         originAngle_POS         = strlenExtraUserinfo_POS + 2,
         driverPrefSize_POS      = originAngle_POS + 2,
         urmaskSize_POS          = driverPrefSize_POS + 4,
         
         endStatic_POS           = urmaskSize_POS + 4
      };
};



/**
 * Request to change a route in the route storage.
 *
 * After the normal RequestPacket header the request packet contains:
 * \begin{tabular}{lll}
 *    Pos                         & Size     & Destription \\ \hline
 *    REQUEST_HEADER_SIZE         & 4 bytes  & routeID \\
 *    +4                          & 4 bytes  & createTime \\
 *    +8                          & 4 bytes  & validUntil\\
 *    \end{tabular}
 *
 */
class RouteStorageChangeRouteRequestPacket :  public RequestPacket {
   public:
      /**
       * Constructor.
       *
       * @param routeID The ID of the route to change.
       * @param createTime The creation time of the route to change.
       * @param validUntil The new validUntil value to set for route.
       */
      RouteStorageChangeRouteRequestPacket( uint32 routeID,
                                            uint32 createTime,
                                            uint32 validUntil );
      
      
      /**
       * The routeID.
       */
      uint32 getRouteID() const;

      
      /**
       * The createTime.
       */
      uint32 getCreateTime() const;


      /**
       * The validUntil.
       */
      uint32 getValidUntil() const;


   private:
      /**
       * The positions of the things with static locations in the packet.
       */
      enum positions {
         routeID_POS             = REQUEST_HEADER_SIZE,
         createTime_POS          = routeID_POS + 4,
         validUntil_POS          = createTime_POS + 4,
         
         endStatic_POS           = validUntil_POS + 4
      };
};


/**
 * The reply to a RouteStorageChangeRouteRequestPacket.
 * The reply only contains the status of the operation.
 *
 */
class RouteStorageChangeRouteReplyPacket : public ReplyPacket {
   public:
      /**
       * Constructor.
       *
       * @param inPacket The RouteStorageChangeRouteRequestPacket that this
       *        is a reply to.
       * @param status The stringCode of the reply status.
       */
      RouteStorageChangeRouteReplyPacket( 
         const RouteStorageChangeRouteRequestPacket* inPacket,
         StringCode status );
      

   private:
};


#endif // ROUTESTORAGEPACKET_H

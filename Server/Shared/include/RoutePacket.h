/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEPACKET_H
#define ROUTEPACKET_H

#include "Packet.h"
#include "Types.h"
#include "config.h"
#include <vector>
#include "DisturbanceDescription.h"

#define ROUTE_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define ROUTE_REPLY_PRIO   DEFAULT_PACKET_PRIO

#define ROUTE_REPLY_SUB_HEADER_SIZE 16

#define ROUTE_TYPE_STRING            1
#define ROUTE_TYPE_GFX               2
// The route should be used in a navigator/eBox
#define ROUTE_TYPE_NAVIGATOR         4
#define ROUTE_TYPE_GFX_COORD         8
// if included tells how many items is corresponding to each string in reply
#define ROUTE_TYPE_ITEM_STRING      16
// All coordinates for the route
#define ROUTE_TYPE_ALL_COORDINATES  32
// Used by the TRISS server, for calculation the route between two
// TMC-points
#define ROUTE_TYPE_GFX_TRISS        64

class SubRouteVector;   // forward declaration
class DriverPref;       // forward declaration
class Vehicle;          // forward declaration
/**
  *   Contains information from decoded route packet.
  *
  */
class RouteItem 
{
   public:
      /**
        *   Constructs a new RouteItem.
        */
      RouteItem( bool isOrigin,
                 bool dirFromZero,
                 uint16 offset,
                 uint32 mapID,
                 uint32 itemID,
                 int32 lat,
                 int32 lon );

      /**
       *    Find out if this is RouteItem is an origin or destination.
       *    @return  True if item is origin false otherwise.
       */
      inline bool getIsOrigin();

      /**
       *    Find out the direction of the car.
       *    @return  True if car is driving towards node zero, false 
       *             otherwise.
       */
      inline bool getFromZero();

      /**
       *    Get the offset for this origin or destination.
       *    @return  The dist from node zero to X. Dist is in percent 
       *             normalized to 2^16-1 
       */
      inline uint16 getOffset();

      /**
       *    Get the ID of the map where this origin or destination is 
       *    located.
       *    @return  The upper part of the item ID = (mapID).
       */
      inline uint32 getMapID();

      /**
       *    Get the ID of the Street Segment Item where this origin or
       *    destination is located.
       *    @return  The lower part of the item ID.
       */
      inline uint32 getItemID();

      /**
       *    Get the latitude for this origin or destination.
       *    @return The latitude for this item.
       */
      inline int32 getLat();

      /**
       *    Get the longitude for this origin or destination.
       *    @return The longitude for this item.
       */
      inline int32 getLong();

      /**
       *    Dump the contents of the RouteItem to cout.
       */
      void dump();

  
  private:
      /**
        *   Is this an origin (true) or destination (false)?
        */
      bool m_isOrigin;

      /**
        *   True if driving towards node 1, false if driving towads node 0.
        */
      bool m_dirFromZero;

      /**
        *   The relative distance to node 0 (in 2^16-1 parts).
        */
      uint16 m_percentToZero;

      /**
        *   Map ID of this origin/destination.
        */
      uint32 m_mapID;

      /**
        *   Item ID of this origin/destination.
        */
      uint32 m_itemID;

      /** 
        *   The latitude for this item.
        */
      int32 m_latitude;

      /** 
        *   The longitude for this item.
        */
      int32 m_longitude;
};

// ======================================================================
//                                                     RouteReplyPacket =

/**
  *   The route reply. After the normal header this packet 
  *   contains (subType = ROUTE_REPLY):
  *   \begin{tabular}{lll}
  *      Pos      & Size    & \\ \hline
  *      24       & 2 bytes  & Number of elements in result \\
  *      26       & 1 bit   & Is the origin heading towards node 
  *                           0 or 1 ( 1 == node 0 ) \\
  *      26.125   & 1 bit   & Is the destination heading towards 
  *                           node 0 or 1 ( 1 == node 0 ) \\  
  *      27       & 1 byte  & The expandRoutetype used
  *                           ExpandRouteRequestPacket.\\
  *      28       & 4 bytes & Start and end offset \\
  *      32       & 4 bytes & Uturn flag \\
  *      for each & 8 bytes & Element index (mapID + elementID)\\
  *               & 4 bytes & Dist \\
  *               & 4 bytes & Time \\
  *      end each & 4 bytes & Standstilltime \\
  *      X        & 4 bytes & nbrRouteObjectsUsed \\
  *      X        & 4 bytes & Nbr DisturbanceDescriptions.\\
  *      for each & X bytes & DisturbanceDescription.\\
  *   \end{tabular}
  *   
  *   And finally for routing error reply. After the normal 
  *   reply header this packet contains (subType = ROUTE_REPLY):
  *   \begin{tabular}{lll}
  *      Pos      & Size    & \\ \hline
  *      24       & 2 bytes & Number of elements in result = 0 \\
  *      26       & 2 bytes & Parameters = 0 \\
  *   \end{tabular}
  *      
  */
class RouteReplyPacket : public ReplyPacket
{
   public:
      /**
        *   Creates an empty RouteReplyPacket.
        */
      RouteReplyPacket();

      /**
       *    Creates a RouteReplyPacket from a subRouteList.
       *    Temporarily here.
       *    @param pref The DriverPref of the customer.
       *    @param vect The SubRouteVector to read from.
       */
      RouteReplyPacket(const DriverPref* pref,
                       const SubRouteVector& vect);

      /**
        *   Adds one RouteItem to this packet.
        *
        *   @param   mapID          ID of the map where itemID is located.
        *   @param   itemID         ID of one item on the route.
        *   @param   offset         is the offset from node 0
        */
      void addRouteItem( uint32 mapID, uint32 itemID );

      /**
        *   Get the number of segments in this route.
        *   @return  The number of (streetSegment-)items in this route.
        */
      inline uint32 getNbrItems() const;

      /**
        *   Set the number of items in the route to zero.
        */
      inline void resetNbrItems();

      /**
        *   Get one of the route items.
        *
        *   @param i       The number of the item (valid values are 
        *                  {\tt 0 <= i < getNbrItems()}.
        *   @param mapID   Outparameter that is set to mapID of item 
        *                  number i.
        *   @param itemID  Outparameter that is set to the itemID of 
        *                  item number i.
        */
      void getRouteItem(uint32 i, uint32 &mapID, uint32 &itemID) const;

      /**
        *   @name Get/set offsets.
        *   Methods to get/set the offsets in the route.
        */
      //@{
         /** 
           *   Get the start offset.
           *   @return  Start offset of the route.
           */
         uint16 getStartOffset() const;

         /** 
           *   Set start offset of the route.
           *   @param   sOffset  The new value of start offset.
           */
         void setStartOffset(uint16 sOffset);

         /** 
           *   Get the end offset.
           *   @return  End offset of the route.
           */
         uint16 getEndOffset() const;

         /** 
           *   Set end offset of the route.
           *   @param   eOffset  The new value of end offset.
           */
         void setEndOffset(uint16 eOffset);
      //@}

      /**
        *   @name Get/set directions.
        *   Methods to get/set the start and end directions.
        */
      //@{
         /** 
           *   Find out the start direction of the route. 
           *   @return True if start towards node 0 .
           */
         inline bool getStartDir();

         /** 
           *   Set the start direction to a new value.
           *   @param   towards0 True if starting towards node 0, false 
           *                     otherwise.
           */
         inline void setStartDir(bool towards0);

         /** 
           *   Find out the end direction of the route. 
           *   @return True if end towards node 0.
           */
         inline bool getEndDir();

         /** 
           *   Set the end direction to a new value.
           *   @param   towards0 True if ending towards node 0, false 
           *                     otherwise.
           */
         inline void setEndDir(bool towards0);
      //@}

      /**
        *   Set the type of route that should be used when expanding.
        *   NB: Probably not used, but it should be.
        *   @param   routType    The type of the route.
        */
      inline void setRouteType(byte routeType);

      /**
        *   Get the route type. 
        *   @return The type of route that should be created in 
        *           expandroute.
        */
      inline byte getRouteType(void) const;

      /** 
       *    Searches the driverpref to find the Vehicle that
       *    corresponds to the <code>stateElement</code>.
       *    Used by createSubRouteVector.
       *    @param driverPref The current driver preferences.
       *    @param stateElement The state element to convert.
       */
      static const Vehicle* stateElementToVehicle(const DriverPref* driverPref,
                                                  uint32 stateElement);
      
      /**
       *    Returns the route as a SubRouteVector.
       *    @param driverPref The driver settings to use for vehicles.
       */
      SubRouteVector* createSubRouteVector(const DriverPref* driverPref) const;


      static uint32 getMaxSimDataLength(/*const*/ RouteReplyPacket* rr);

      static void printSimulateRouteData(/*const*/ RouteReplyPacket* rr,
                                         char* data, uint32 maxNbrBytes);

      /**
       * Set nbrRouteObjectsUsed.
       */
      void setNbrRouteObjectsUsed( uint32 n );

      /**
       * Get nbrRouteObjectsUsed.
       */
      uint32 getNbrRouteObjectsUsed() const;

      /**
       * Set the disturbances.
       */
      void setDisturbanceDescriptions( 
         const vector<DisturbanceDescription>& v );

      /**
       * Get the disturbances.
       */
      vector<DisturbanceDescription> getDisturbanceDescription() const; 

      /**
       * Set the U-turn flag.
       */
      void setUTurn( bool uturn );

      /**
       * Get the U-turn flag.
       */
      bool getUTurn() const;
      
   private:
      /**
        *   Inrease the number of items in this packet with one.
        */
      inline void incNbrItems();
};  


// ------------------------------------------------------------------------
//                                     Implementaion of the inlined methods


inline bool 
RouteItem::getIsOrigin()
{
   return m_isOrigin;
}

inline bool 
RouteItem::getFromZero()
{
   return m_dirFromZero;
}

inline uint16 
RouteItem::getOffset()
{
   return m_percentToZero;
}

inline uint32 
RouteItem::getMapID()
{
   return m_mapID; 
}

inline uint32 
RouteItem::getItemID()
{
   return m_itemID;
}

inline int32 
RouteItem::getLat()
{
   return m_latitude;
}

inline int32 
RouteItem::getLong()
{
   return m_longitude;
}

// ========================================================================
//                                                       RouteReplyPacket =

inline uint32 
RouteReplyPacket::getNbrItems() const
{
   return readShort(REPLY_HEADER_SIZE);
}

inline void 
RouteReplyPacket::resetNbrItems() 
{
   writeShort(REPLY_HEADER_SIZE, 0);
}

inline bool 
RouteReplyPacket::getStartDir() 
{
   byte b = readByte(REPLY_HEADER_SIZE + 2);
   if ( (b & 0x80) == 0x80) {
      // The first bit is set
      return (true);
   } else {
      return (false);
   }
}

inline void 
RouteReplyPacket::setStartDir(bool towards0) 
{
   byte b = readByte(REPLY_HEADER_SIZE+2);
   if (towards0) {
      b |= 0x80;
   } else {
      b &= 0x7f;
   }
   writeByte(REPLY_HEADER_SIZE+2, b);
}

inline bool 
RouteReplyPacket::getEndDir() 
{
   byte b = readByte(REPLY_HEADER_SIZE + 2);
   if ( (b & 0x40) == 0x40) {
      // The first bit is set
      return (true);
   } else {
      return (false);
   }
}

inline void 
RouteReplyPacket::setEndDir(bool towards0) 
{
   byte b = readByte(REPLY_HEADER_SIZE+2);
   if (towards0) {
      b |= 0x40;
   } else {
      b &= 0xbf;
   }
   writeByte(REPLY_HEADER_SIZE+2, b);
}

inline void 
RouteReplyPacket::setRouteType(byte routeType) 
{
   writeByte(REPLY_HEADER_SIZE + 3, routeType);
}

inline byte 
RouteReplyPacket::getRouteType(void)  const
{
   return readByte(REPLY_HEADER_SIZE + 3);
}

inline void 
RouteReplyPacket::incNbrItems()
{
   writeShort(REPLY_HEADER_SIZE, getNbrItems()+1);
}

#endif



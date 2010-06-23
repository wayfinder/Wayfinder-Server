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

#include "GfxUtility.h"
#include "RouteObject.h"
#include "ServerSubRouteVector.h"
#include "RouteSender.h"
#include "OrigDestInfo.h"
#include "IDPairVector.h"
#include "RoutingInfoPacket.h"
#include "UnexpandedRoute.h"
#include "Connection.h"
#include "Properties.h"
#include "RequestUserData.h"
#include "UserData.h"
#include "TimeUtility.h"
#include "Math.h"
#include "STLUtility.h"

#include <algorithm>

/*
 *    The maximum number of orig- and destDescriptions. To make sure that
 *    we do not overfill the RouteRequestPacket.
 */
#define MAX_NBR_DESCRIPTIONS 16000

#define ROU "[" << (m_user.getUser() ? m_user.getUser()->getLogonID() : "NA") << "]"


RouteObject::RouteObject( const RequestUserData& user,
                          Request* req,
                          uint32 expandType,
                          StringTable::languageCode language,
                          bool noConcatenate,
                          uint32 passedRoads,
                          const TopRegionRequest* topReq,
                          bool abbreviate,
                          const DisturbanceList* disturbances,
                          uint32 nbrWantedRoutes,
                          bool removeAheadIfDiff,
                          bool nameChangeAsWP)
      : m_routeReplyPacket( NULL ),
        m_request( req ), 
        m_coordinateObject(req, topReq),
        m_language( language ),
        m_nbrWantedRoutes( nbrWantedRoutes ),
        m_user( user )
{
   mc2dbg8 << ROU << "----------------------" << endl;
   mc2dbg8 << ROU << (uint32)language << endl;

   m_driverPref         = NULL;
   m_driverPrefToDelete = NULL;
   // Reset some members.
   m_answer = NULL;
   m_routeSender = NULL;   
   m_routeExpander = NULL;
   m_expandType = expandType;
   m_origPoint.itemName = NULL;
   m_destPoint.itemName = NULL;
   m_createTime = TimeUtility::getRealTime();
   m_abbreviate     = abbreviate;
   m_disturbances   = disturbances;
   m_noConcatenate  = noConcatenate;
   m_passedRoads    = passedRoads;
   m_removeAheadIfDiff = removeAheadIfDiff;
   m_nameChangeAsWP = nameChangeAsWP;

   m_status = StringTable::TIMEOUT_ERROR; // Assume timeout for now.
   memset( &m_origPoint, 0, sizeof( origDestFinalPoint_t ) );
   memset( &m_destPoint, 0, sizeof( origDestFinalPoint_t ) );
   m_allowedMaps = NULL;
   mc2dbg2 << ROU << "dist = " << disturbances << endl;

   if (m_expandType == 0) {
      mc2dbg4 << ROU << "ExpandType == 0 no expansion will be done (yet)" << endl;
      // No expansion should be done if m_expandType == 0 from now on.
   }

   m_state = AWAITING_ORIGDEST;

   if (m_abbreviate) {
      mc2dbg2 << ROU << here << " m_abbreviate set to true (ctor)" << endl;
   } else {
      mc2dbg2 << ROU << here << " m_abbreviate set to false (ctor)" << endl;
   }

   m_origList = new OrigDestInfoList;
   m_destList = new OrigDestInfoList;
   m_routeResultVector = NULL;
   m_unexpandedRoute = NULL;
   m_sendItemNames = true;
   m_nbrItemNamesToReceive = 0;
   m_routingInfo = NULL;
}

RouteObject::state_t
RouteObject::createAndPrepareExpandItemPackets()
{
   DEBUG8(
      mc2dbg << ROU << "RouteObject::createAndPrepareExpandItemPackets()" 
             << endl;
      dumpState();
   );

   // Make sure that we have both origins and destinations
   uint32 nbrOrigs = m_allOrigs.size();
   uint32 nbrDests = m_allDests.size();
   if (nbrOrigs < 1) {
      // No expanding if we have no origins or no destinations
      mc2log << error << ROU 
             << "Leaving createAndPrepareExpandItemPacket, no orig"
             << endl;
      return setErrorState( StringTable::ERROR_NO_VALID_START_ROUTING_NODE );
   } else if ( nbrDests < 1) {
      mc2log << error << ROU 
             << "Leaving createAndPrepareExpandItemPacket, no dest"
             << endl;
      return setErrorState( StringTable::ERROR_NO_VALID_END_ROUTING_NODE );
   }

   // First sort the m_allOrigs and the m_allDests by mapID
   std::sort( m_allOrigs.begin(), m_allOrigs.end(), STLUtility::RefLess() );
   std::sort( m_allDests.begin(), m_allDests.end(), STLUtility::RefLess() );
   
   // The mapID to process for ExpandID
   Vector mapIDsToSendExpandItem(8,8);
   for (uint32 i=0; i<m_allOrigs.size(); i++) {
      if (m_allOrigs[i]->expandID()) {
         mc2dbg8 << ROU << "Adding "  
                 << m_allOrigs[i]->mapID 
                 << " to mapIDsToSendExpandItem, itemID=" 
                 << m_allOrigs[i]->itemID 
                 << endl;
         mapIDsToSendExpandItem.addLast(
            m_allOrigs[i]->mapID);
      }
   }
   for (uint32 i=0; i<m_allDests.size(); i++) {
      if (m_allDests[i]->expandID()) {
         mc2dbg8 << ROU << "Adding "  
                 << m_allDests[i]->mapID 
                 << " to mapIDsToSendExpandItem itemID=" 
                 << m_allDests[i]->itemID 
                 << endl;
         mapIDsToSendExpandItem.addLast(
            m_allDests[i]->mapID);
      }
   }
   mapIDsToSendExpandItem.removeDoubles();

   bool toSendCoordinateExpand = false;
   const byte itemTypes = byte(ItemTypes::streetSegmentItem);
   // Add the origins where to expand coordinate to a packet
   for (uint32 i=0; i<m_allOrigs.size(); i++) {
      if (m_allOrigs[i]->
          expandCoordinate()) {
         mc2dbg8 << ROU << "RouteObject::createAndPrepareExpandItemPackets()"
                 << " adding orig (" 
                 << m_allOrigs[i]->lat << ","
                 << m_allOrigs[i]->lon << ")"
                 << m_allOrigs[i]->startAngle
                 << endl;
         int pos = m_coordinateObject.addCoordinate(
                     m_allOrigs[i]->lat,
                     m_allOrigs[i]->lon,
                     0,    // All outdata types
                     1, 
                     &itemTypes,
                     m_allOrigs[i]->startAngle);

         m_allOrigs[i]->coordinatePos = pos;
         toSendCoordinateExpand = true;
      }
   }
   // Add the destination where to expand coordinate to a packet
   for (uint32 i=0; i<m_allDests.size(); i++) {
      if ( m_allDests[i]->expandCoordinate() ) {
         mc2dbg8 << ROU << "RouteObject::createAndPrepareExpandItemPackets()"
                 << " adding dest (" 
                 << m_allDests[i]->lat << ","
                 << m_allDests[i]->lon << ")" 
                 << endl;
         int pos = m_coordinateObject.addCoordinate(
                     m_allDests[i]->lat,
                     m_allDests[i]->lon,
                     0,    // All outdata types
                     1, 
                     &itemTypes,
                     m_allDests[i]->startAngle);
         m_allDests[i]->coordinatePos = pos;
         toSendCoordinateExpand = true;
      }
   }

   // Insert all the coordinate expand packets into the tree
   PacketContainer* pc = m_coordinateObject.getNextPacket();
   while (pc != NULL) {
      m_packetsReadyToSend.add(pc);
      pc = m_coordinateObject.getNextPacket();
   }

   // Insert all completed OrigDestPoints into m_origList and m_destList
   for (uint32 i=0; i<m_allOrigs.size(); i++) {
      if ( ! m_allOrigs[i]->expandID() &&
           ! m_allOrigs[i]->expandCoordinate( ) ) {
         OrigDestInfo o(m_driverPref,
                        m_allOrigs[i]->mapID,
                        m_allOrigs[i]->itemID,
                        m_allOrigs[i]->lat,
                        m_allOrigs[i]->lon,
                        m_allOrigs[i]->offset);

         mc2dbg4 << ROU << "Adding OrigDestInfo to m_origList {"
                 << o.getMapID() << "." << o.getNodeID() << " "
                 << o.getLat() << "." << o.getLon() << "}" << endl;
         m_origList->addOrigDestInfo(o);
      }
   }
   for (uint32 i=0; i<m_allDests.size(); i++) {
      if ( (! m_allDests[i]->expandID()) &&
           (! m_allDests[i]->expandCoordinate())) {
         OrigDestInfo o(m_driverPref,
                        m_allDests[i]->mapID,
                        m_allDests[i]->itemID,
                        m_allDests[i]->lat,
                        m_allDests[i]->lon,
                        m_allDests[i]->offset);

         mc2dbg4 << ROU << "Adding OrigDestInfo to m_origList {"
                 << o.getMapID() << "." << o.getNodeID() << " "
                 << o.getLat() << "." << o.getLon() << "}" << endl;
         m_destList->addOrigDestInfo(o);
      }
   }

   state_t returnValue;
   mc2dbg4 << ROU << "toSendCoordinateExpand=" << toSendCoordinateExpand
           << ", mapIDsToSendExpandItem.getSize()=" 
           << mapIDsToSendExpandItem.getSize() << endl;
   if ( (toSendCoordinateExpand ) || 
        (mapIDsToSendExpandItem.getSize() > 0)) {

      // Create the packets and insert them into the tree
      for (uint32 i=0; i<mapIDsToSendExpandItem.getSize(); i++) {
         uint32 curMapID = mapIDsToSendExpandItem[i];
         uint16 curPacketID = m_request->getNextPacketID();
         RouteExpandItemRequestPacket* pack = 
            new RouteExpandItemRequestPacket(curPacketID,
                                             m_request->getID() );
         pack->setMapID(curMapID);

         // Get the position of the first origin with curMapID
         OrigDestPoint odp(curMapID, 0, 0);
         OrigDestPointVector::iterator iter = find_if( m_allOrigs.begin(), m_allOrigs.end(),
                                                       STLUtility::RefEqualCmp<OrigDestPoint>(odp) );

         // Add all origins to this packet
         while ((iter != m_allOrigs.end() ) && 
                 ( (*iter)->mapID == 
                  curMapID)) {
            mc2dbg8 << ROU << "   Adding " << curMapID << "."
                    << (*iter)->itemID
                    << " to RouteExpandItemRequestPacket" << endl;
            pack->add(
                 (*iter)->itemID,
                 (*iter)->offset);
            (*iter)->packetID = 
               curPacketID;
            //static_cast<OrigDestPoint*>(m_allOrigs[pos])->nbrPacketsOut++;
            ++iter;
         }

         // Get the position of the first destination with curMapID
         iter = find_if ( m_allDests.begin(), m_allDests.end(), 
                          STLUtility::RefEqualCmp<OrigDestPoint>(odp) );
         // Add all origins to this packet
         while ((iter != m_allDests.end() ) && 
                 ( (*iter)->mapID == 
                  curMapID)) {
            mc2dbg8 << ROU << "   Adding " << curMapID << "."
                    << (*iter)->itemID
                    << " to RouteExpandItemRequestPacket" << endl;
            pack->add(
                 (*iter)->itemID,
                 (*iter)->offset);
            (*iter)->packetID = 
               curPacketID;
            //static_cast<OrigDestPoint*>(m_allDests[pos])->nbrPacketsOut++;
            ++iter;
         }

         // Add to the tree of packets to sedn
         m_packetsReadyToSend.add(
            new PacketContainer(pack, 0, 0, MODULE_TYPE_MAP));
      }

      returnValue = SENDING_ITEM_EXPAND;
   } else {
      // No need to expand the items
      returnValue = createAndPrepareRoutePacket();
   }

   DEBUG8(
      mc2dbg << ROU << "RouteObject::createAndPrepareExpandItemPackets() "
             << "Leaving" << endl;
      dumpState();
   );

   return (returnValue);
}

RouteObject::~RouteObject() 
{
   uint32 m, i;
   const char* n;
   ItemTypes::itemType type;
   mc2dbg8 << ROU << "getNbrValidOrigins() = " << getNbrValidOrigins() << endl;
   for (uint32 j=0; j<getNbrValidOrigins(); j++) {
      int32 lat, lon;
      if ( getValidOrigin(0, m, i, lat, lon, n, type)) {
         mc2dbg8 << ROU << "   valid origin number " << j <<" : " 
                 << m << "." << i << ", " << n << ", " << (int)type << endl;
      } else {
         mc2dbg8 << ROU << "   Failed to get origin number " << j << endl;
      }
   }
   mc2dbg8 << ROU << "getNbrValidDests() = " << getNbrValidDestinations() 
           << endl;
   for (uint32 j=0; j<getNbrValidDestinations(); j++) {
      int32 lat, lon;
      if ( getValidDestination(0, m, i, lat, lon, n, type)) {
         mc2dbg8 << ROU << "   valid destination number " << j <<" : " 
                 << m << "." << i << ", " << n << ", " << (int)type << endl;
      } else {
         mc2dbg8 << ROU << "   Failed to get destination number " << j << endl;
      }
   }

   
   delete m_routeReplyPacket;
   delete m_routeExpander;

   STLUtility::deleteValues( m_allOrigs );

   STLUtility::deleteValues( m_allDests );

   STLUtility::deleteValues( m_origDescriptions );

   STLUtility::deleteValues( m_destDescriptions );

   delete [] m_origPoint.itemName;
   delete [] m_destPoint.itemName;

   delete m_driverPrefToDelete;
   delete m_routeSender;

   delete m_origList;
   delete m_destList;

   delete m_unexpandedRoute;
   delete m_routeResultVector;
   delete m_routingInfo;
}


bool 
RouteObject::operator == ( const RouteObject &other ) const {
   // Check routeparameters
   bool pIsStartTime;
   byte pRouteType;
   byte pCostA;
   byte pCostB;
   byte pCostC;
   byte pCostD;
   uint32 pVehicleParam;
   uint32 pTime;
   uint32 pTurnCost;
   bool oIsStartTime;
   byte oRouteType;
   byte oCostA;
   byte oCostB;
   byte oCostC;
   byte oCostD;
   uint32 oVehicleParam;
   uint32 oTime;
   uint32 oTurnCost;
   
   getRouteParameters( pIsStartTime,
                       pRouteType,
                       pCostA,
                       pCostB,
                       pCostC,
                       pCostD,
                       pVehicleParam,
                       pTime,
                       pTurnCost );
   other.getRouteParameters( oIsStartTime,
                             oRouteType,
                             oCostA,
                             oCostB,
                             oCostC,
                             oCostD,
                             oVehicleParam,
                             oTime,
                             oTurnCost );
   if ( pIsStartTime  != oIsStartTime ||
        pRouteType    != oRouteType ||
        pCostA        != oCostA ||
        pCostB        != oCostB ||
        pCostC        != oCostC ||
        pCostD        != oCostD ||
        pVehicleParam != oVehicleParam ||
        pTime         != oTime ||
        pTurnCost     != oTurnCost ||
        m_language    != other.m_language )
   {
      return false;
   }


   // Check origins
   if ( getNbrOrigin() != other.getNbrOrigin() ) {
      return false;
   }

   for ( uint32 i = 0 ; i < getNbrOrigin() ; i++ ) {
      uint32 pMapID;
      uint32 pItemID;
      uint16 pOffset;
      int32 pLat;
      int32 pLon;
      uint32 oMapID;
      uint32 oItemID;
      uint16 oOffset;
      int32 oLat;
      int32 oLon;
      
      getOrigin( i, pMapID, pItemID, pOffset, pLat, pLon );
      other.getOrigin( i, oMapID, oItemID, oOffset, oLat, oLon );
      
      if ( pMapID  != oMapID ||
           pItemID != oItemID ||
           pOffset != oOffset ||
           pLat    != oLat ||
           pLon    != oLon )
      {
         return false;
      }
   }

   // Check destinations
   if ( getNbrDestination() != other.getNbrDestination() )
      return false;
   
   if ( m_nbrWantedRoutes != other.m_nbrWantedRoutes ) 
      return false;

   for ( uint32 i = 0 ; i < getNbrDestination() ; i++ ) {
      uint32 pMapID;
      uint32 pItemID;
      uint16 pOffset;
      int32 pLat;
      int32 pLon;
      uint32 oMapID;
      uint32 oItemID;
      uint16 oOffset;
      int32 oLat;
      int32 oLon;
      
      getDestination( i, pMapID, pItemID, pOffset, pLat, pLon );
      other.getDestination( i, oMapID, oItemID, oOffset, oLat, oLon );
      
      if ( pMapID  != oMapID ||
           pItemID != oItemID ||
           pOffset != oOffset ||
           pLat    != oLat ||
           pLon    != oLon )
      {
         return false;
      }
   }

   
   // Passed all tests
   return true;
}


void
RouteObject::dumpState()
{
   cout << "- - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
   cout << "currentState:";
   switch (m_state) {
      case AWAITING_ORIGDEST:
         cout << "AWAITING_ORIGDEST" << endl;
         break;
      case (SENDING_ITEM_EXPAND):
         cout << "SENDING_ITEM_EXPAND" << endl;
         break;
      case (RECEIVING_ITEM_EXPAND):
         cout << "RECEIVING_ITEM_EXPAND" << endl;
         break;
      case (SENDING_ROUTE):
         cout << "SENDING_ROUTE" << endl;
         break;
      case (RECEIVING_ROUTE):
         cout << "RECEIVING_ROUTE" << endl;
         break;
      case (SENDING_EXPAND_ROUTE):
         cout << "SENDING_EXPAND_ROUTE" << endl;
         break;
      case (RECEIVING_EXPAND_ROUTE):
         cout << "RECEIVING_EXPAND_ROUTE" << endl;
         break;
      case (SENDING_NAME_REQUEST):
         cout << "SENDING_NAME_REQUEST" << endl;
         break;
      case (RECEIVING_NAME_REQUEST):
         cout << "RECEIVING_NAME_REQUEST" << endl;
         break;
      case (DONE):
         cout << "DONE" << endl;
         break;
      case (ERROR):
         cout << "ERROR" << endl;
         break;
      case ROUTINGINFO:
         cout << "ROUTINGINFO" << endl;
         break;
   }
   cout << "   m_allOrigs:" << endl;
   for (uint32 i=0; i<m_allOrigs.size(); i++)
      m_allOrigs[i]->dump();
   cout << "   m_allDests:" << endl;
   for (uint32 i=0; i<m_allDests.size(); i++)
      m_allDests[i]->dump();
   cout << "   m_origDescriptions (" << m_origDescriptions.size() 
        << "):" << endl;
   for (uint32 i=0; i<m_origDescriptions.size(); i++)
      static_cast<OrigDestDescription*>(m_origDescriptions[i])->dump();
   cout << "   m_destDescriptions (" << m_destDescriptions.size() 
        << "):" << endl;
   for (uint32 i=0; i<m_destDescriptions.size(); i++)
      static_cast<OrigDestDescription*>(m_destDescriptions[i])->dump();
   cout << "   m_origPoint: " << m_origPoint.mapID << "." 
        <<  m_origPoint.itemID << "." <<  m_origPoint.offset << endl;
   cout << "   m_destPoint: " << m_destPoint.mapID << "." 
        <<  m_destPoint.itemID << "." <<  m_destPoint.offset << endl;
   cout << "   m_packetsReadyToSend.getCardinal() " 
        << m_packetsReadyToSend.getCardinal() << endl;
   cout << "   m_status " << StringTable::getString( 
      m_status, StringTable::ENGLISH ) << " (" << int(m_status) << ")"
        << endl;
   cout << "- - - - - - - - - - - - - - - - - - - - - - - - " << endl;
}

void RouteObject::dumpData()
{
   cout << "isStartTime : " << (int)m_routeData.isStartTime << endl;
   cout << "routeType : " << (int)m_routeData.routeType << endl;
   cout << "costA : " << (int)m_routeData.costA << endl;
   cout << "costB : " << (int)m_routeData.costB << endl;
   cout << "costC : " << (int)m_routeData.costC << endl;
   cout << "costD : " << (int)m_routeData.costD << endl;
   cout << "vehicleParam : " << m_routeData.vehicleParam << endl;
   cout << "time : " << m_routeData.time << endl;
   cout << "turnCost : " << m_routeData.turnCost << endl;
   cout << "expandType : " << m_expandType << endl;
   cout << "landmarks : " << (int)m_landmarks << endl;

}

PacketContainer* 
RouteObject::getNextPacket() 
{   

   // The packet to return
   PacketContainer* retPacketCont = m_packetsReadyToSend.getMin();
   if (retPacketCont != NULL) {
      m_packetsReadyToSend.remove(retPacketCont);
      
   }
 
   switch (m_state) {
      case AWAITING_ORIGDEST:
         if (retPacketCont == NULL) {
            // Sneak in a RoutingInfoRequest first.
            RoutingInfoRequestPacket* ripack = 
               new RoutingInfoRequestPacket();
            ripack->setRequestID( m_request->getID() );
            ripack->setPacketID( m_request->getNextPacketID() );
            m_packetsReadyToSend.add( 
               new PacketContainer( ripack, 0, 0, MODULE_TYPE_MAP ) );
            m_state = ROUTINGINFO;
            // And return the first packet
            retPacketCont = m_packetsReadyToSend.getMin();
            if (retPacketCont != NULL) {
               m_packetsReadyToSend.remove(retPacketCont);
            }
         }
         break;
      case ROUTINGINFO :
         // Nothing to do, see processpacket
         break;

      case (SENDING_ITEM_EXPAND):
      case (RECEIVING_ITEM_EXPAND):
         if (retPacketCont == NULL) {
            // All ItemExpandPackets sent
            m_state = RECEIVING_ITEM_EXPAND;
         }
      break;

      case (RECEIVING_ROUTE):
      case (SENDING_ROUTE):
         if (retPacketCont == NULL) {
            // The Route Packet is sent
            m_state = RECEIVING_ROUTE;
         }
      break;

      case (SENDING_EXPAND_ROUTE):
      case RECEIVING_EXPAND_ROUTE : 
         if (retPacketCont == NULL) {
            // Check if we have any new packet to send.
            retPacketCont = m_routeExpander->getNextRequest();
            //m_state = RECEIVING_EXPAND_ROUTE; // Maybe, maybe not
         } 
         
      break;

      case (SENDING_NAME_REQUEST):
         if (retPacketCont == NULL) {
            // The Route Packet is sent
            m_state = RECEIVING_NAME_REQUEST;
         }
      break;

      default:
         mc2dbg8 << ROU << "RouteObject::getNextPacket default state "
                     << uint32(m_state) << ", retPacketCont == " 
                     << retPacketCont  << endl;
         delete retPacketCont;
         retPacketCont = NULL;
      break;

   }

   // Return
   return (retPacketCont);
  
}

void 
RouteObject::processPacket( const PacketContainer* packetContainer ) 
{
   if ( packetContainer == NULL ) {
      // PacketContainer == NULL indicates a timeout.
      m_status = StringTable::TIMEOUT_ERROR;
      return;
   }
   Packet* packet = packetContainer->getPacket();

   // Handle the incomming packet depending on current state.
   switch (m_state) {
      case AWAITING_ORIGDEST:         
         mc2log << error << ROU 
                << "In state 'AWAITING_ORIGDEST' RouteObject got packet of type "
                << packet->getSubTypeAsString()
                << endl;
         m_state = setErrorState( StringTable::INTERNAL_SERVER_ERROR );
         break;
      case ROUTINGINFO:
         if ( packet->getSubType() == Packet::PACKETTYPE_ROUTINGINFOREPLY )
         {
            mc2dbg2 << ROU << "[RO]: Got routing info" << endl;
            // Create routing info.
            RoutingInfoReplyPacket* rinfo =
               static_cast<RoutingInfoReplyPacket*>(packet);
            m_routingInfo = new RoutingInfo(rinfo->getRoutingInfo());
            // Done! Move on, we should initialize the itemexpand
            m_state = createAndPrepareExpandItemPackets();
         } else {
            mc2dbg << ROU << "[RO]: Not routing info packet: "
                   << packet->getSubTypeAsString() << endl;
            m_state = setErrorState( StringTable::INTERNAL_SERVER_ERROR);
         }
         break;
      case SENDING_ITEM_EXPAND:
      case RECEIVING_ITEM_EXPAND:
         if ( packet->getSubType() == Packet::PACKETTYPE_COORDINATEREPLY ){
            m_state = handleCoordinateReplyPacket(packetContainer);
         } else if ( packet->getSubType() ==
                   Packet::PACKETTYPE_ROUTEEXPANDITEMREPLY) {
            m_state = handleRouteExpandItemReplyPacket(
                        static_cast<RouteExpandItemReplyPacket*>(packet));
         } else {
            mc2dbg << ROU << "[RO]: Wrong type of packet : "
                   << packet->getSubTypeAsString() << endl;
            m_state = setErrorState( StringTable::INTERNAL_SERVER_ERROR);
         }
          
         break;
      case (SENDING_ROUTE):
      case (RECEIVING_ROUTE): {
         // First send the packet on to the RouteSender
         m_routeSender->processPacket( packetContainer );

         // Then get new request packets from the RouteSender         
         PacketContainer* pc = m_routeSender->getNextPacket();
         while (pc != NULL) {
            m_packetsReadyToSend.add( pc );
            pc = m_routeSender->getNextPacket( );
         }
         
         // If RouteSender is finished routing and take care of the answer.
         if ( m_routeSender->requestDone() == true ) {
            if ( m_routeSender->getStatus() == StringTable::OK ) {
               // In the future, it will be possible to get more
               // than one route here.
               ServerSubRouteVectorVector* SRVV =
                  m_routeSender->getSortedRoute();

               // Fix the offsets.
               for(uint32 i = 0; i < SRVV->size(); ++i ) {
                  ServerSubRouteVector* srVect = (*SRVV)[i];
                  if ( srVect->getSize() > 0 ) {
                     SubRoute* firstSR = srVect->getSubRouteAt( 0 );
                     // Fixing the offsets, which were manipulated to be on the
                     // form that the RouteModule requires
                     if ( firstSR->getOrigNodeID() & 0x80000000 ){
                        firstSR->setOrigOffset(1.0-firstSR->getOrigOffset() );
                     }
                     SubRoute* lastSR = srVect->getSubRouteAt(
                        srVect->getSize() - 1 );
                     if ( lastSR->getDestNodeID() & 0x80000000 ){
                        lastSR->setDestOffset( 1.0 - lastSR->getDestOffset() );
                     }
                  }
               }
               ServerSubRouteVector* srVect =
                  SRVV->empty() ? NULL : SRVV->front();
               // Only handle RouteReply if there is something in
               // the vector and we should expand the route.
               if ( srVect != NULL &&
                    srVect->getSize() > 0 ) {
                  mc2dbg1 << ROU << "[RO]: Size of srVect = "
                         << srVect->getSize();
                  mc2dbg4 << " Cost of first "
                          << srVect->front()->getCost();
                  mc2dbg2 << " Cost of last " << srVect->back()->getCost();
                  uint32 secs =
                     Connection::timeCostToSec(srVect->back()->getCost());
                  mc2dbg2 << " Number of seconds "
                          << secs;
                  mc2dbg2 << " That is " << (secs / 60)
                          << " minutes and " <<  (secs % 60) << " sec";
                  mc2dbg4 << " Time : " << srVect->getTotalTimeSec(false);
                  mc2dbg1 << " Routing time: "
                          << (TimeUtility::getCurrentTime()-m_routingStartTime)
                          << endl;
                  if ( m_expandType != 0 ) {
                     RouteReplyPacket* p = 
                        new RouteReplyPacket(m_driverPref, *srVect);
                     m_state = handleRouteReplyPacket( p );
                     delete p;
                  } else {

                     // To store the packet.
                     m_routeReplyPacket = new RouteReplyPacket(m_driverPref,
                                                               *srVect);
                  }
                  
               } else {
                  mc2log << info << ROU 
                         << "No route was found to the destination"
                         << endl;
                  m_state = setErrorState( StringTable::ERROR_NO_ROUTE );
               }
               // Remove unwanted routes.
               if ( m_nbrWantedRoutes != MAX_UINT32 )
                  fixupRouteResult(SRVV, m_nbrWantedRoutes);
               
               m_routeResultVector = SRVV;

               if ( srVect ) {
                  m_unexpandedRoute = new UnexpandedRoute(*srVect,
                                                          *m_driverPref);
               } else {
                  m_unexpandedRoute = NULL;
               }
               if ( m_expandType == 0 ) {
                  // That concludes it - we're done.
                  if ( m_status == StringTable::TIMEOUT_ERROR)
                     m_status = StringTable::OK;
                  m_state = DONE;
               }
            } else {
               m_state = setErrorState( m_routeSender->getStatus() );
            }
         }
      }
      break;
      
      case (SENDING_EXPAND_ROUTE):
      case (RECEIVING_EXPAND_ROUTE):
         // Get the next packet from concatenator
         m_routeExpander->packetReceived(packetContainer);
         if (m_routeExpander->getDone()) {
            // We're done, create answer. Answer can be null,
            // then something went wrong.
            delete m_answer;
            m_answer = m_routeExpander->createAnswer();
            if ( m_answer == NULL || static_cast<ReplyPacket*>( 
                    m_answer->getPacket() )->getStatus() != 
                 StringTable::OK )
            {
               // Error we have
               StringTable::stringCode errCode = 
                  StringTable::TIMEOUT_ERROR;
               if ( m_answer != NULL ) {
                  errCode = StringTable::stringCode( 
                     static_cast<ReplyPacket*>( 
                        m_answer->getPacket() )->getStatus() );
               }
               m_state = setErrorState( errCode );
            } else if ( m_sendItemNames ) {
               m_state = createAndPrepareNamePackets();
            } else {
               // That concludes it - we're done.
               if ( m_status == StringTable::TIMEOUT_ERROR)
                  m_status = StringTable::OK;
               m_state = DONE;
            }
         }
         break;

      case (SENDING_NAME_REQUEST):
      case (RECEIVING_NAME_REQUEST):
         mc2dbg8 << ROU << "GOT NAME REPLY!!!"
                 << endl;
         m_state = handleNameReplyPacket(static_cast<ItemNamesReplyPacket*>
                                                    (packet));
         
      break;

      case (DONE):
         mc2log << error << ROU 
                <<"RouteObject::processPacket strange state: DONE"
                << ", pack->packetID = " << packet->getPacketID() 
                << endl;
      break;

      case (ERROR):
         mc2log << error << ROU 
                << "RouteObject::processPacket strange state: ERROR"
                << ", pack->packetID = " << packet->getPacketID() 
                << endl;
      break;
   }

   DEBUG8(
      mc2dbg << ROU << "RouteObject::processPacket. Leaving" << endl;
      dumpState();
   );

   // The packet p is deleted in the Request, i.e. RouteRequest in
   // this case.
}

                  
PacketContainer* 
RouteObject::getAnswer() 
{
   DEBUG8(
      if (m_answer != NULL){
         mc2dbg << ROU << "last coordinates = " 
                << static_cast<ExpandRouteReplyPacket*>
            (m_answer->getPacket())->getLastItemLat()
                << ", " << static_cast<ExpandRouteReplyPacket*>
            (m_answer->getPacket())->getLastItemLon()
                << endl;
      } else { 
        mc2dbg << ROU << "last coordinates:: m_answer == NULL" << endl;
      }
   );

   return m_answer;
}

ServerSubRouteVectorVector*
RouteObject::getRoute(bool steal)
{
   ServerSubRouteVectorVector* retVal = m_routeResultVector;
   if ( steal )
      m_routeResultVector = NULL;
   return retVal;
}

UnexpandedRoute*
RouteObject::getUnexpandedRoute(bool steal)
{
   UnexpandedRoute* retVal = m_unexpandedRoute;
   if ( steal ) {
      m_unexpandedRoute = NULL;
   }
   return retVal;
}

void 
RouteObject::setAbbreviate(bool abbreviate)
{
   m_abbreviate = abbreviate;

   if (m_abbreviate) {
      mc2dbg8 << ROU << here << " m_abbreviate set to true" << endl;
   } else {
      mc2dbg8 << ROU << here << " m_abbreviate set to false" << endl;
   }

}

void 
RouteObject::setLandmarks( bool landmarks ) {
   m_landmarks = landmarks;
}

void 
RouteObject::setRemoveAheadIfDiff( bool removeAheadIfDiff){
   m_removeAheadIfDiff = removeAheadIfDiff;
}

void 
RouteObject::setNameChangeAsWP( bool nameChangeAsWP){
   m_nameChangeAsWP = nameChangeAsWP;
}
   
RouteObject::state_t
RouteObject::handleCoordinateReplyPacket(const PacketContainer* pc)
{
   // coordinateObject deletes pc so copy it
   PacketContainer* coordPC = new PacketContainer( 
      pc->getPacket()->getClone(), 0, 0, MODULE_TYPE_INVALID );
   m_coordinateObject.processPacket(coordPC);
   PacketContainer* packCont = m_coordinateObject.getNextPacket();
   while (packCont != NULL) {
      m_packetsReadyToSend.add(packCont);
      packCont = m_coordinateObject.getNextPacket();
   }
   
   state_t retVal = RECEIVING_ITEM_EXPAND;
   if (m_coordinateObject.getDone()) {
      // Got reply for all coordinate-requests
      for (uint32 i=0; i<m_allOrigsSafe.size(); i++) {
         OrigDestPoint* odp = static_cast<OrigDestPoint*>(
            m_allOrigsSafe[i]);
         CoordinateReplyPacket* p = 
            m_coordinateObject.getCoordinateReply(odp->coordinatePos);
         mc2dbg8 << ROU << "Getting packet at position " 
                 << odp->coordinatePos << endl;
         if ( (odp->coordinatePos < MAX_UINT16) && ( p != NULL) &&
              p->getStatus() == StringTable::OK )
         {
            odp->mapID = p->getMapID();
            odp->itemID = p->getItemID();
            odp->offset = p->getOffset();
            
            OrigDestDescription* desc = new OrigDestDescription(
               odp->lat,
               odp->lon,
               p->getMapID(),
               p->getItemID(),
               p->getOffset(),
               ItemTypes::streetSegmentItem);
            // Add to description-array
            m_origDescriptions.push_back(desc);
            
            // start direction
            if (odp->startAngle < 360) {
               // Caluculate the positive angle-differances
               int32 posAngle = p->getPositiveAngle();
               int32 negAngle = p->getNegativeAngle();
               // Values greater than 360 indicates errors
               if ( posAngle < 361 && negAngle < 361 ) {
                  negAngle = negAngle % 360;
                  posAngle = posAngle % 360;
                  // Find the smallest difference in cw and ccw.
                  const int nbrDiff = 4;
                  int32 diff[nbrDiff];
                  diff[0] = posAngle - odp->startAngle;
                  diff[1] = negAngle - odp->startAngle;
                  diff[2] = odp->startAngle - posAngle;
                  diff[3] = odp->startAngle - negAngle;
                  // Adjust for 360 and find the smallest
                  int32 smallest = 30000;
                  int smallestPos = 5;
                  mc2dbg8 << ROU << "Startangle= " << odp->startAngle << endl
                          << "Posangle = " << posAngle << endl
                          << "Negangle = " << negAngle << endl;
                  for(int i=0; i < nbrDiff; ++i ) {
                     diff[i] = diff[i] < 0 ? diff[i] + 360 : diff[i];
                     mc2dbg8 << ROU << "diff[" << i << "] = " << diff[i] 
                             << endl;
                     if ( diff[i] < smallest ) {
                        smallestPos = i;
                        smallest = diff[i];
                     }                  
                  }
                  // If posangle is the nearest dirFromZero is true.
                  desc->bDirectionFromZero = 
                     ((smallestPos == 0) || (smallestPos == 2));
                  desc->bDirectionFromZeroSet = true;
                  
               } else {
                  mc2log << warn << ROU 
                         << "RouteObject: posAngle or negAngle more than "
                         << "360 deg"
                         << endl;
                  desc->bDirectionFromZero = false;
               }
               
               mc2dbg8 << ROU << "   bDirectionFromZero set to " 
                       << desc->bDirectionFromZero << endl;
            }
            desc->origDestIndex = i; //-1;
            odp->processed = true;
         } else if ( odp->coordinatePos == MAX_UINT16 ) {
            // Not coordinate!!!
         } else {
            // Error, stop end set error
            retVal = ERROR;
            if ( p == NULL ) {
               m_status = StringTable::TIMEOUT_ERROR;
            } else {
               if ( p->getStatus() == StringTable::MAPNOTFOUND ) {
                  m_status = StringTable::ONE_OR_MORE_INVALID_ORIGS;
               } else {
                  m_status = StringTable::INTERNAL_SERVER_ERROR;
               }
            }
            DEBUG8(
               if (p == NULL){
                  cerr << " p == NULL" << endl;
                  cerr << "odp->coordinatePos = "
                       << odp->coordinatePos  
                       << endl;
               }
               );
            mc2dbg << ROU << "[RO]: Coordinate error 1 - "
                   << StringTable::getString( m_status,
                                              StringTable::ENGLISH ) << endl;
         }
      }
      
      // Got reply for all coordinate-requests
      for (uint32 i=0; i<m_allDestsSafe.size(); i++) {
         OrigDestPoint* odp = static_cast<OrigDestPoint*>(
            m_allDestsSafe[i]);
         CoordinateReplyPacket* p = 
            m_coordinateObject.getCoordinateReply(odp->coordinatePos);
         mc2dbg8 << ROU << "Getting packet at position " 
                 << odp->coordinatePos
                 << endl;
         if ( (odp->coordinatePos < MAX_UINT16) && ( p!= NULL) &&
              p->getStatus() == StringTable::OK )
         {
            odp->mapID = p->getMapID();
            odp->itemID = p->getItemID();
            odp->offset = p->getOffset();
            OrigDestDescription* desc = new OrigDestDescription(
               odp->lat,
               odp->lon,
               p->getMapID(),
               p->getItemID(),
               p->getOffset(),
               ItemTypes::streetSegmentItem);
            // Add to description-array
            m_destDescriptions.push_back(desc);
            desc->origDestIndex = i; // -1;
            odp->processed = true;
         } else if ( odp->coordinatePos == MAX_UINT16 ) {
            // Not coordinate!!!
         } else {
            // Error, stop end set error
            retVal = ERROR;
            if ( p == NULL ) {
               m_status = StringTable::TIMEOUT_ERROR;
            } else {
               if ( p->getStatus() == StringTable::MAPNOTFOUND ) {
                  m_status = StringTable::ONE_OR_MORE_INVALID_DESTS;
               } else {
                  m_status = StringTable::INTERNAL_SERVER_ERROR;
               }
            }
            DEBUG8(
               if (p == NULL) {
                  cerr << " p == NULL" << endl;
                  cerr << "odp->coordinatePos = " << odp->coordinatePos << endl;
               }
               );
            mc2dbg << ROU << "[RO]: Coordinate error 2 - "
                   << StringTable::getString( m_status,
                                              StringTable::ENGLISH ) << endl;
         }
      }
      
      if ( retVal != ERROR ) { // No error above
         retVal = createAndPrepareRoutePacket( true );
      }
   }
   
   return (retVal);
}


RouteObject::state_t
RouteObject::handleRouteExpandItemReplyPacket(
                  RouteExpandItemReplyPacket* expandReplyPacket )
{
   if ( expandReplyPacket->getStatus() != StringTable::OK ) {
      mc2log << warn << ROU << "RouteObject::handleRouteExpandItemReplyPacket "
             << "reply not ok: " << StringTable::getString(
                StringTable::stringCode( expandReplyPacket->getStatus() ),
                StringTable::ENGLISH ) << endl;
      return ( setErrorState( StringTable::stringCode( expandReplyPacket->getStatus() ) ) );
   }
   
   // Initiate the position in the reply packet
   int position = expandReplyPacket->positionInit();

   uint16 expandePacketID = expandReplyPacket->getPacketID();

   for (uint32 i=0; i<expandReplyPacket->getNbrItems(); i++) {
      // Get data from the expandReplyPacket
      uint16 nbrPositions = 0;
      uint32 expandedItemID = MAX_UINT32;
      ItemTypes::itemType type = ItemTypes::nullItem;
      uint32* ssItemIDs = NULL;
      int32* lats = NULL;
      int32* lons = NULL;
      uint16* offsets = NULL;
      uint32* parentItemIDs = NULL;
      expandReplyPacket->getItem(position, nbrPositions, 
                                 expandedItemID, type,
                                 ssItemIDs, offsets, 
                                 lats, lons, parentItemIDs);

      mc2dbg8 << ROU << "      expandedItemID = "
              << expandedItemID
              << endl;
      // Find the expanded item in the m_allOrigins or m_allDests
      uint32 allPos = 0;
      bool origin = true;
      uint32 mapID = expandReplyPacket->getMapID();
      while ( (allPos < m_allOrigs.size()) && 
              (expandedItemID != MapBits::nodeItemID(
                 m_allOrigs[allPos]->itemID) ||
               mapID != m_allOrigs[allPos]->mapID ||
               m_allOrigs[ allPos ]->packetID == MAX_UINT16 ) ) 
      {
         allPos++;
      }
      mc2dbg8 << ROU << "         allPos = " << allPos 
              << ", m_allOrigs.size()=" << m_allOrigs.size() 
              << endl;
      if (allPos >= m_allOrigs.size()) {
         // Not an origin
         origin = false;
         allPos = 0;
         while ( (allPos < m_allDests.size()) && 
                 (expandedItemID != MapBits::nodeItemID(
                    m_allDests[allPos]->itemID) ||
                  mapID != m_allDests[allPos]->mapID ||
                  m_allDests[ allPos ]->packetID == MAX_UINT16 ) ) 
         {
            allPos++;
         }
         m_allDests[allPos]->packetID = MAX_UINT16;
         m_allDests[allPos]->processed = true;
      } else {
         m_allOrigs[allPos]->packetID = MAX_UINT16;
         m_allOrigs[allPos]->processed = true;
      }

      // Add expanded data to 
      mc2dbg8 << ROU << "      nbrPositions=" << nbrPositions << endl;
      for (uint32 j=0; j<nbrPositions; j++) {
         // Create OrigDestDescription
         OrigDestDescription* desc = new OrigDestDescription(
            lats[j],
            lons[j],
            mapID,
            ssItemIDs[j],
            offsets[j],
            type);
         desc->parentItemID = parentItemIDs[j];
         desc->origDestIndex = allPos;
         if (origin) {
            if (m_origDescriptions.size() < MAX_NBR_DESCRIPTIONS) {
               m_origDescriptions.push_back(desc);
            } else {
               mc2log << error << ROU << "Too many origs" << endl;
               delete desc;
            }
         } else {
            if (m_destDescriptions.size() < MAX_NBR_DESCRIPTIONS) {
               m_destDescriptions.push_back(desc);
            } else {
               mc2log << error << ROU << "Too many dests" << endl;
               delete desc;
            }
         }
      }
      
      // Clean up
      delete [] ssItemIDs;
      delete [] lats;
      delete [] lons;
      delete [] offsets;
      delete [] parentItemIDs;
   }

   // Check if we got answers for all the items with this packetID
   for (uint32 i=0; i<m_allOrigs.size(); i++) {
      if ( m_allOrigs[i]->packetID == expandePacketID  &&
            m_allOrigs[i]->packetID != MAX_UINT16)  {

         // Found unexpanded item, indicate with MAX_UINT16-1
         m_allOrigs[i]->packetID = MAX_UINT16-1;
         mc2log << warn << ROU << here << " FOUND invalid expanded item (orig)"
                << endl;
      }
   }
   for (uint32 i=0; i<m_allDests.size(); i++) {
      if ( m_allDests[i]->packetID == expandePacketID &&
           m_allDests[i]->packetID != MAX_UINT16 ) {
         
         // Found unexpanded item, indicate with MAX_UINT16-1
         m_allDests[i]->packetID = MAX_UINT16-1;
         mc2log << warn << ROU << here << "FOUND invalid expanded item (dest)" 
                << endl;
      }
   }
   
   // By sending true as parameter to createAndPrepareRoutePacket it will
   // check if all OrigDestPoints are processed
   //      BUT IF MapModule REPLIES NOTFOUND THE REQUEST SHOULD STOP
   //      BUT IT DOESN'T DO THAT IN createAndPrepareRoutePacket SO
   //      I'VE ADDED A CHECK FIRST IN THIS FUNCTION!
   return (createAndPrepareRoutePacket(true));
}

bool
RouteObject::checkSame(OrigDestDescription* origDest,
                       origDestFinalPoint_t& finalOrigDest)
{
   // ItemID - not node ID
   uint32 origDestItemID = origDest->itemID & 0x7fffffff;
   // Not node ID
   uint32 finalItemID    = finalOrigDest.itemID & 0x7fffffff;

   // We can check the item and MapID now.
   if ( (finalOrigDest.mapID != origDest->mapID) ||
        (finalItemID != origDestItemID) ) {
      return false;
   }
   
   // Cannot check offset for this one. Can this happen?
   if ( origDest->itemType != ItemTypes::streetSegmentItem )
      return true;

   // Ehm. The offset is transformed into floats and changes sometimes.
   if ( ::abs((int)finalOrigDest.offset - (int)origDest->offset) > 3 ) {
      mc2dbg2 << "[RO]: Item id is same but offset differs "
             << "finalOffset = " << finalOrigDest.offset << " origdestoffset = "
             << origDest->offset << ", MAX_UINT16-origdestoffset = "
             << (MAX_UINT16-origDest->offset) << endl;
      return false;
   } 
   // Everything is the same now.
   return true;
}

RouteObject::state_t
RouteObject::handleRouteReplyPacket(RouteReplyPacket* routeReply)
{
   // Set the m_routeReplyPacket-member to be a copy of the 
   // incomming packet
   delete m_routeReplyPacket;
   m_routeReplyPacket = static_cast<RouteReplyPacket*>
      (routeReply->getClone());
   
   // Checking start and dest 
   uint32 nbrRouteItems = routeReply->getNbrItems();
   if ( nbrRouteItems > 1 ) {
      // Add data for origin to m_originPoint. Also find the descriptions
      // that matches the origin in the route         
      uint32 routeItemNbr = 0;
      do {
         routeReply->getRouteItem( routeItemNbr,
                                   m_origPoint.mapID,
                                   m_origPoint.itemID);
         mc2dbg2 << ROU << "handleRouteReplyPacket routeItemNbr " 
                 << routeItemNbr << " of " << nbrRouteItems << " mapID " 
                << m_origPoint.mapID << " itemID " << m_origPoint.itemID
                << endl;
         routeItemNbr++;
      } while ( GET_TRANSPORTATION_STATE( m_origPoint.itemID ) != 0 &&
                routeItemNbr < nbrRouteItems );
      m_origPoint.offset = routeReply->getStartOffset();
      
      for (uint32 j=0; j< m_origDescriptions.size(); j++) {
         OrigDestDescription* curOrig =
            static_cast<OrigDestDescription*>(m_origDescriptions[j]);         
         curOrig->validOrigDest = checkSame(curOrig, m_origPoint);
      }
      m_origPoint.mapID = REMOVE_UINT32_MSB(m_origPoint.mapID);
      m_origPoint.itemID = REMOVE_UINT32_MSB(m_origPoint.itemID);
      
      // Add data for destination to m_destPoint. Also find the descriptions
      // that matches the origin in the route
      routeReply->getRouteItem( nbrRouteItems - 1, 
                                m_destPoint.mapID,
                                m_destPoint.itemID);
      m_destPoint.offset = routeReply->getEndOffset();
      for (uint32 j=0; j< m_destDescriptions.size(); j++) {
         OrigDestDescription* curDest =
            static_cast<OrigDestDescription*>(m_destDescriptions[j]);
         curDest->validOrigDest = checkSame(curDest, m_destPoint);            
      }
      m_destPoint.mapID = REMOVE_UINT32_MSB(m_destPoint.mapID);
      m_destPoint.itemID = REMOVE_UINT32_MSB(m_destPoint.itemID);
   } // nbrRouteItems > 1
   
   // Create the route expander
   mc2dbg8 << ROU << "Creating expander with type 0x" << hex 
           << m_expandType << dec << endl;
   uint32 mapID, itemID, index = 0;
   routeReply->getRouteItem(0, mapID, itemID);
   itemID &= 0x7fffffff;
   
   for(uint32 i = 0; i < m_origDescriptions.size(); i++) {
      if(static_cast<OrigDestDescription*>
         (m_origDescriptions[i])->itemID == itemID ) 
         index = i;
   }
   bool uTurn;
   if ( m_driverPref->useUturn() && static_cast<OrigDestDescription*>
        (m_origDescriptions[index])->bDirectionFromZeroSet ) {
      bool startDir = static_cast<OrigDestDescription*>
         (m_origDescriptions[index])->bDirectionFromZero; 
      uTurn = ( startDir == routeReply->getStartDir() );
   } else
      uTurn = false;
   m_routeReplyPacket->setUTurn( uTurn );

   if (m_abbreviate) {
      mc2dbg8 << ROU << here << " m_abbreviate = true" << endl;
   } else {
      mc2dbg8 << ROU << here << " m_abbreviate = false" << endl;
   }
#define TEST_SUBROUTEVECTOR      
#ifndef TEST_SUBROUTEVECTOR
   // Remove this one later, when we know that the SubRouteVector
   // has enough information
   m_routeExpander = new RouteExpander( 
      m_request,
      m_routeReplyPacket,
      m_expandType,
      uTurn,
      m_noConcatenate,
      m_passedRoads,
      m_language,
      m_abbreviate,
      m_landmarks,
      m_removeAheadIfDiff,
      m_nameChangeAsWP
      );
#else
   SubRouteVector* srVect =
      routeReply->createSubRouteVector(m_driverPref);
   // New version. Uses the SubRouteVector instead of just the
   // route reply.
   m_routeExpander = new RouteExpander( 
      m_request,
      srVect,
      m_expandType,
      uTurn,
      m_noConcatenate,
      m_passedRoads,
      m_language,
      m_abbreviate,
      m_landmarks,
      m_removeAheadIfDiff,
      m_nameChangeAsWP);
#endif
   // Add all packets from the expander to m_packetsReadyToSend
   PacketContainer* pc = m_routeExpander->getNextRequest();
   while (pc != NULL) {
      mc2dbg8 << ROU << "   ADDING requestPacket from the route expander" 
              << endl;
      m_packetsReadyToSend.add(pc);
      pc = m_routeExpander->getNextRequest();
   }
   if ( m_packetsReadyToSend.getCardinal() > 0 ) {
      return (SENDING_EXPAND_ROUTE);
   } else {
      // Here we should return error message from RouteExpander.
      // That still does not exist however.
      return setErrorState( StringTable::NOTOK ); 
   }
}

RouteExpander*
RouteObject::getRouteExpander(uint32 expandType)
{
   
   // Checking start and dest 
   uint32 nbrRouteItems = m_routeReplyPacket->getNbrItems();
   if ( nbrRouteItems > 1 ) {
      // Add data for origin to m_originPoint. Also find the descriptions
      // that matches the origin in the route         
      uint32 routeItemNbr = 0;
      do {
         m_routeReplyPacket->getRouteItem( routeItemNbr,
                                   m_origPoint.mapID,
                                   m_origPoint.itemID);
         mc2dbg2 << ROU << "handleRouteReplyPacket routeItemNbr " 
                 << routeItemNbr
                << " of " << nbrRouteItems << " mapID " 
                << m_origPoint.mapID << " itemID " << m_origPoint.itemID
                << endl;
         routeItemNbr++;
      } while ( GET_TRANSPORTATION_STATE( m_origPoint.itemID ) != 0 &&
                routeItemNbr < nbrRouteItems );
      m_origPoint.offset = m_routeReplyPacket->getStartOffset();
      
      for (uint32 j=0; j< m_origDescriptions.size(); j++) {
         OrigDestDescription* curOrig =
            static_cast<OrigDestDescription*>(m_origDescriptions[j]);         
         curOrig->validOrigDest = checkSame(curOrig, m_origPoint);
      }
      m_origPoint.mapID = REMOVE_UINT32_MSB(m_origPoint.mapID);
      m_origPoint.itemID = REMOVE_UINT32_MSB(m_origPoint.itemID);
      
      // Add data for destination to m_destPoint. Also find the descriptions
      // that matches the origin in the route
      m_routeReplyPacket->getRouteItem( nbrRouteItems - 1, 
                                m_destPoint.mapID,
                                m_destPoint.itemID);
      m_destPoint.offset = m_routeReplyPacket->getEndOffset();
      for (uint32 j=0; j< m_destDescriptions.size(); j++) {
         OrigDestDescription* curDest =
            static_cast<OrigDestDescription*>(m_destDescriptions[j]);
         curDest->validOrigDest = checkSame(curDest, m_destPoint);            
      }
      m_destPoint.mapID = REMOVE_UINT32_MSB(m_destPoint.mapID);
      m_destPoint.itemID = REMOVE_UINT32_MSB(m_destPoint.itemID);
   } // nbrRouteItems > 1
   
   // Create the route expander
   mc2dbg8 << ROU << "Creating expander with type 0x" << hex 
           << m_expandType << dec << endl;
   uint32 mapID, itemID, index = 0;
   m_routeReplyPacket->getRouteItem(0, mapID, itemID);
   itemID &= 0x7fffffff;
   
   for(uint32 i = 0; i < m_origDescriptions.size(); i++) {
      if(static_cast<OrigDestDescription*>
         (m_origDescriptions[i])->itemID == itemID ) 
         index = i;
   }
   bool uTurn;
   if ( m_driverPref->useUturn() && static_cast<OrigDestDescription*>
        (m_origDescriptions[index])->bDirectionFromZeroSet ) {
      bool startDir = static_cast<OrigDestDescription*>
         (m_origDescriptions[index])->bDirectionFromZero; 
      uTurn = ( startDir == m_routeReplyPacket->getStartDir() );
   } else
      uTurn = false;
   m_routeReplyPacket->setUTurn( uTurn );

   if (m_abbreviate) {
      mc2dbg8 << ROU << here << " m_abbreviate = true" << endl;
   } else {
      mc2dbg8 << ROU << here << " m_abbreviate = false" << endl;
   }
#define TEST_SUBROUTEVECTOR      
#ifndef TEST_SUBROUTEVECTOR
   // Remove this one later, when we know that the SubRouteVector
   // has enough information
   RouteExpander* routeExpander = new RouteExpander( 
      m_request,
      m_routeReplyPacket,
      expandType,
      uTurn,
      m_noConcatenate,
      m_passedRoads,
      m_language,
      m_abbreviate,
      m_landmarks,
      m_removeAheadIfDiff,
      m_nameChangeAsWP
      );
#else
   SubRouteVector* srVect =
      m_routeReplyPacket->createSubRouteVector(m_driverPref);
   // New version. Uses the SubRouteVector instead of just the
   // route reply.
   RouteExpander* routeExpander = new RouteExpander( 
      m_request,
      srVect,
      expandType,
      uTurn,
      m_noConcatenate,
      m_passedRoads,
      m_language,
      m_abbreviate,
      m_landmarks,
      m_removeAheadIfDiff,
      m_nameChangeAsWP);
#endif

   return routeExpander;
}

bool
RouteObject::readyToExpand(RouteExpander* expander){
   
   // Add all packets from the expander to m_packetsReadyToSend
   m_routeExpander = expander;
   m_state = SENDING_EXPAND_ROUTE;
   return true;
}


RouteObject::state_t
RouteObject::handleNameReplyPacket(ItemNamesReplyPacket* p)
{
   --m_nbrItemNamesToReceive;
   mc2dbg2 << ROU << "RouteObject::handleNameReplyPacket" << endl;
   // Fill the description-arrays with the names 
   uint32 mapID = p->getMapID();
   int pos = p->getFirstNamePosition();
   mc2dbg8 << ROU << "   mapID=" << mapID << " pos = " << pos << " nbrNames=" 
           << p->getNbrNames() << endl;
   for (uint32 j=0; j<p->getNbrNames(); j++) {
      uint32 itemID;
      const char* name = p->getNextName(pos, itemID);
      mc2dbg8 << ROU << "   name=" << name << endl;
      if (name != NULL) {
         for (uint32 i=0; i<m_origDescriptions.size(); i++) {
            OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                                  (m_origDescriptions[i]);
            mc2dbg4 << ROU << "odd->origDestIndex = " << odd->origDestIndex
                    << endl;
            
            if ( (odd->parentItemID == itemID) &&
                 (odd->mapID == mapID)) {
               delete [] odd->itemName;
               odd->itemName = StringUtility::newStrDup(name);
               mc2dbg8 << ROU << "      inserted name " << name 
                       << " into origDescriptions ID=" << itemID << endl;
            }
         }
         for (uint32 i=0; i<m_destDescriptions.size(); i++) {
            OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                                  (m_destDescriptions[i]);
            mc2dbg8 << ROU << "odd->origDestIndex = " << odd->origDestIndex
                    << endl;
            if ( (odd->parentItemID == itemID) &&
                 (odd->mapID == mapID)) {
               delete [] odd->itemName;
               odd->itemName = StringUtility::newStrDup(name);
               mc2dbg8 << ROU << "inserted name " << name 
                       << " into destDescriptions ID=" << itemID << endl;
            }
         }
         if ( m_origPoint.mapID == mapID &&
              m_origPoint.itemID == itemID )
         {
            delete [] m_origPoint.itemName;
            m_origPoint.itemName = StringUtility::newStrDup( name );
         }
         if ( m_destPoint.mapID == mapID &&
              m_destPoint.itemID == itemID )
         {
            delete [] m_destPoint.itemName;
            m_destPoint.itemName = StringUtility::newStrDup( name );
         }
      }
   }

   // Check if done
   bool ready = true;
   pos = 0;
   while ((ready) && (pos < int(m_origDescriptions.size()))) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                            (m_origDescriptions[pos]);
      ready = !((odd->validOrigDest) && (odd->itemName == NULL));
      pos++;
   }
   pos = 0;
   while ((ready) && (pos < int(m_destDescriptions.size()))) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                            (m_destDescriptions[pos]);
      ready = !((odd->validOrigDest) && (odd->itemName == NULL));
      pos++;
   }

   ready = ! ( m_origPoint.itemName == NULL );
   ready = ! ( m_destPoint.itemName == NULL );
   
   if (m_nbrItemNamesToReceive == 0) {
      if ( m_status == StringTable::TIMEOUT_ERROR) {
         m_status = StringTable::OK;
      }
      mc2dbg2 << ROU << "[RO]: Ready == true NbrItemsToReceive = "
             << m_nbrItemNamesToReceive << endl;
      return (DONE);
   } else {
      return (RECEIVING_NAME_REQUEST);
   }
}

RouteObject::state_t
RouteObject::setErrorState(StringTable::stringCode stringCode)
{
   // Check inparameter
   if (stringCode == StringTable::OK) {
      mc2log << error << ROU 
             << "RouteObject::setErrorState(OK) Could not set "
             << "error-state to OK" << endl;
      stringCode = StringTable::NOTOK;
   }
   
   m_state = ERROR;

   // Set m_answer to a dummy-value
   delete m_answer;
   ExpandRouteReplyPacket* errp = new ExpandRouteReplyPacket();
   errp->setStatus(stringCode);
   errp->setNumStringData(0);
   errp->setSizeStringData(0);
   errp->setNumItemData(0);
   errp->setSizeItemsData(0);
   m_answer = new PacketContainer(errp);
   m_status = stringCode;
   
   return (ERROR);
}


RouteObject::state_t
RouteObject::createAndPrepareNamePackets()
{
   // Check that the packet-tree is empty
   if (m_packetsReadyToSend.getCardinal() > 0) {
      return (setErrorState( StringTable::NOTOK ));
   }

   // Find the map IDs where to send name-requests
   Vector mapIDs(8,8);
   for (uint32 i=0; i<m_origDescriptions.size(); i++) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                            (m_origDescriptions[i]);
      if ( (odd->validOrigDest) &&
           (odd->itemName == NULL)){
         mapIDs.addLastIfUnique(odd->mapID);
      }
   }
   for (uint32 i=0; i<m_destDescriptions.size(); i++) {
      OrigDestDescription* odd = m_destDescriptions[i];
      if ( (odd->validOrigDest) &&
           (odd->itemName == NULL)){
         mapIDs.addLastIfUnique(odd->mapID);
      }
   }
   mapIDs.addLastIfUnique( m_origPoint.mapID );
   mapIDs.addLastIfUnique( m_destPoint.mapID );

   for (uint32 i=0; i < mapIDs.getSize(); i++) {
      // Add all items with same mapID into a ItemNamesRequestPacket
      uint32 curMapID = mapIDs[i];
      ++m_nbrItemNamesToReceive;
      ItemNamesRequestPacket* curPacket = 
         new ItemNamesRequestPacket(curMapID, 
                                    m_request->getID(), 
                                    m_request->getNextPacketID());
      
      curPacket->setPreferedLanguage(LangTypes::language_t(
         StringTable::getNormalLanguageCode( m_language ) ) );
      
      for (uint32 j=0; j<m_origDescriptions.size(); j++) {
         OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                               (m_origDescriptions[j]);
         if ( odd->mapID == curMapID &&
              (odd->validOrigDest) && (odd->itemName == NULL)) {
            curPacket->addItem(odd->parentItemID);
         }
      }
      for (uint32 j=0; j<m_destDescriptions.size(); j++) {
         OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                               (m_destDescriptions[j]);
         if (odd->mapID == curMapID &&
             (odd->validOrigDest) && (odd->itemName == NULL)) {
            curPacket->addItem(odd->parentItemID);
         }
      }
      if ( m_origPoint.mapID == curMapID ) {
         curPacket->addItem( m_origPoint.itemID ); 
      }
      if ( m_destPoint.mapID == curMapID ) {
         curPacket->addItem( m_destPoint.itemID ); 
      }

      m_packetsReadyToSend.add( new PacketContainer(curPacket, 
                                                    0, 
                                                    0, 
                                                    MODULE_TYPE_MAP));
   }

   return (SENDING_NAME_REQUEST);
}

uint32
RouteObject::getTurnCost(uint32 nodeID) 
{
   return Connection::secToTimeCost(120U*20U);
}

uint32
RouteObject::calcNbrWantedRoutes(uint32 nbrWantedResults)
{
   if ( nbrWantedResults == MAX_UINT32 ) {
      return nbrWantedResults;
   }
   
   // Vector to count destinations in
   vector<uint32> countVect;
   const int nbrAllDests = m_allDests.size();
   countVect.reserve(nbrAllDests);
   for(int i=0; i  < nbrAllDests; ++i ) {
      countVect.push_back(0);
   }
   
   // Calc the number of needed routes
   int nbrDestDesc = m_destDescriptions.size();
   for( int i=0; i < nbrDestDesc ; ++i ) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
         (m_destDescriptions[i]);
      
      MC2_ASSERT( int(odd->origDestIndex) < nbrAllDests );
      // Increase the number of odd:s pointing to the original dest.
      ++(countVect[odd->origDestIndex]);
   }
   
   // Print the count for each original destination
   for ( int i = 0; i < nbrAllDests; ++i ) {
      mc2dbg2 << ROU << "Number of things for dest " << i << " = "
             << countVect[i] << endl;
   }
   
   sort(countVect.begin(), countVect.end(), greater<uint32>());

   // Sum up the worst ones.
   uint32 retVal = 0;

   for(int i=0; i < (int)nbrWantedResults; ++i ) {
      retVal += countVect[i];
      mc2dbg4 << ROU << "CountVect[" << i << "] = " << countVect[i] << endl;
   }
   
   return retVal*2; // Node 0 and 1 are needed.
}


void
RouteObject::fixupRouteResult(ServerSubRouteVectorVector* srvv,
                              uint32 nbrWanted)
{
   if ( nbrWanted == MAX_UINT32 )
      return;

   // Make a map of the m_destDescriptions
   typedef map<IDPair_t, OrigDestDescription*> destMap_t;
   destMap_t destMap;
   int nbrDestDesc = m_destDescriptions.size();
   
   for( int i = 0; i < nbrDestDesc; ++i ) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
         (m_destDescriptions[i]);
      destMap.insert(pair<IDPair_t, OrigDestDescription*>
                     (IDPair_t(odd->mapID, odd->itemID & 0x7fffffff), odd));
   }

   // Array to mark used original destinations in.
   const int nbrAllDests = m_allDestsSafe.size();
   bool used[nbrAllDests];
   
   for( int i = 0; i < nbrAllDests; ++i ) {
      used[i] = false;
   }

   // Finally - go through all SubRouteVectors and remove the ones
   // that belong to the same destination in m_allDestsSafe.
   // FIXME: Only go through the vector until we have the right number
   //        of routes left. After that we will remove the unneeded
   //        routes from the back instead.
   ServerSubRouteVectorVector::iterator it = srvv->begin();
   while ( it != srvv->end() ) {
      SubRouteVector* srv = *it;
      // Compare itemID - not nodeID
      IDPair_t idpair(srv->back()->getNextMapID(),
                      srv->back()->getDestNodeID() & 0x7fffffff);
      destMap_t::iterator found = destMap.find(idpair);
      // We really should find it in the map!
      MC2_ASSERT( found != destMap.end() );
      // Get the index of the parent
      int index = found->second->origDestIndex;
      MC2_ASSERT( index < nbrAllDests );
      if ( used[index] == false ) {
         // Not used - keep in vector
         ++it;
         used[index] = true;
         // Save which destination this SubRouteVector
         // belongs to.
         srv->setOriginalDestIndex(index);
      } else {
         // Already used - remove from vector
         // Hope that there aren't too many
         delete srv;
         it = srvv->erase(it);
      }
   }
   
   mc2dbg2 << ROU << "Size of route vector vector is now " << srvv->size()
           << endl;

   // Remove the ones that aren't needed.
   while ( srvv->size() > nbrWanted ) {
      SubRouteVector* srv = srvv->back();
      delete srv;
      srvv->pop_back();
   }
   
   mc2dbg2 << ROU << "Size of route vector vector is now " << srvv->size() 
           << endl;
}

bool
RouteObject::checkDistOKForVehicle(const DriverPref& dp,
                                   const OrigDestInfoList& origList,
                                   const OrigDestInfoList& destList)
{
   ItemTypes::vehicle_t vehicle =
      ItemTypes::vehicle_t( dp.getVehicleRestriction() );
   if ( vehicle != ItemTypes::pedestrian ) {
      return true;
   }

   const uint32 maxWalkingDistance = Properties::getUint32Property(
     "ROUTE_MAX_WALKING_DISTANCE", 10000 );
   const float64 maxDistWalking = SQUARE( float64( maxWalkingDistance ) );

   // It is a pedestrian
   for( OrigDestInfoList::const_iterator oit = origList.begin();
        oit != origList.end();
        ++oit) {
      for( OrigDestInfoList::const_iterator dit = destList.begin();
           dit != destList.end();
           ++dit) {
         if ( GfxUtility::squareP2Pdistance_linear(oit->getLat(),
                                                   oit->getLon(),
                                                   dit->getLat(),
                                                   dit->getLon())
              < maxDistWalking ) {
            // The distance is less that the maxdistance for one
            // pair of origdest. That is ok.
            return true;
         }
      }      
   }
   // Too far
   return false;
}

int
RouteObject::calcMinDistFromOrigsToDest(const OrigDestInfoList& origs,
                                        const OrigDestInfoList& dests)
{
   float64 minDist = MAX_INT64;
   // It is a pedestrian
   for( OrigDestInfoList::const_iterator oit = origs.begin();
        oit != origs.end();
        ++oit) {
      for( OrigDestInfoList::const_iterator dit = dests.begin();
           dit != dests.end();
           ++dit) {
         float64 curDist = GfxUtility::squareP2Pdistance_linear(oit->getLat(),
                                                                oit->getLon(),
                                                                dit->getLat(),
                                                                dit->getLon());
         if ( curDist < minDist ) {
            minDist = curDist;
         }
      }
   }
   return int(::sqrt(minDist));
}

RouteObject::state_t
RouteObject::createAndPrepareRoutePacket(bool checkProcessed) 
{
   
   if (checkProcessed) {
      // Check if all OrigDestPoint's are processed. If not, return 
      // RECEIVING_ITEM_EXPAND

      // Ever heard of a for-loop?
      uint32 j=0;
      while ( (j<m_allOrigs.size()) && 
              (m_allOrigs[j]->processed)) {
         j++;
      }
      if (j<m_allOrigs.size()) {
         return (RECEIVING_ITEM_EXPAND);
      } else {
         // Check destinations
         j=0;
         while ( (j<m_allDests.size()) && 
                 (m_allDests[j]->processed)) {
            j++;
         }
      }
      if (j<m_allDests.size()) {
         return (RECEIVING_ITEM_EXPAND);
      }
      // OK, all OrigDestPoints processed.
   }

   // Check that the packet-tree is empty and that there is
   // no previous RouteSender
   if (m_packetsReadyToSend.getCardinal() > 0 || m_routeSender != NULL ) {
      return (setErrorState( StringTable::NOTOK ));
   }

   // Add origins and destinations
   mc2dbg8 << ROU << "m_origDescriptions.size() = " 
           << m_origDescriptions.size() << endl;
   for (uint32 j=0; j<m_origDescriptions.size(); j++) {

      
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                             (m_origDescriptions[j]);

      // Fix the turncost. Turncost isn't really used. It's
      // recalculated by the RM, but it should be calculated here.
      uint32 turnCost = 0;
      if ( m_driverPref->useUturn() ) {
         // NB. This isn't very scientific.
         turnCost = getTurnCost(odd->itemID);
         turnCost = turnCost * m_driverPref->getCostA() +
            turnCost * m_driverPref->getCostB() +
            turnCost * m_driverPref->getCostC() +
            turnCost * m_driverPref->getCostD();
      }


      if ((odd->mapID != MAX_UINT32 ) && (odd->itemID != MAX_UINT32)) {

         mc2dbg8 << ROU << "   Adding origin to SubRouteList: " 
                 << odd->mapID << "." << odd->itemID << "." 
                 << odd->offset << endl;

         // Adding origin node 0 to the outgoing list
         m_origList->addOrigDestInfo(
            OrigDestInfo( m_driverPref,
                          odd->mapID,
                          odd->itemID & 0x7fffffff,
                          odd->lat,
                          odd->lon,
                          float(odd->offset)/MAX_UINT16 ) );

         // Driving towards node 0, penalize if node 0
         if ( odd->bDirectionFromZeroSet && !odd->bDirectionFromZero ) {
            mc2dbg4 << ROU << "Setting turncost on node 0 " << hex
                   << m_origList->back().getNodeID()
                   << dec << " to " << turnCost << endl;
            (&(m_origList->back()))->setTurnCost(turnCost);
         }

         // Adding origin node 1 to the outgoing list
         // Offset here is only for old Route Module.
         // Everywhere else the offset in node 0 is used.
         m_origList->addOrigDestInfo(
            OrigDestInfo( m_driverPref,
                          odd->mapID,
                          odd->itemID | 0x80000000,
                          odd->lat,
                          odd->lon,                          
                          1.0 - float(odd->offset)/MAX_UINT16 ) );

         // Driving towards node 1, penalize if node 1
         if ( odd->bDirectionFromZeroSet && odd->bDirectionFromZero ) {
            mc2dbg4 << ROU << "Setting turncost on node 1 " << hex
                   << m_origList->back().getNodeID()
                   << dec << " to " << turnCost << endl;
            (&(m_origList->back()))->setTurnCost(turnCost);
            mc2dbg4 << ROU << "Turncost after == " 
                    << m_origList->back().getTurnCost() << endl;
         }
      }
   }

   for (uint32 j=0; j<m_destDescriptions.size(); j++) {

      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                             (m_destDescriptions[j]);

      if ((odd->mapID != MAX_UINT32 ) && (odd->itemID != MAX_UINT32)) {

         mc2dbg8 << ROU << "   Adding dest to SubRouteList: " 
                 << odd->mapID << "." << odd->itemID << "." 
                 << odd->offset << endl;

         // Adding destination node 0 to the outgoing list
         m_destList->addOrigDestInfo(
            OrigDestInfo( m_driverPref,
                          odd->mapID,
                          odd->itemID & 0x7fffffff,
                          odd->lat,
                          odd->lon,
                          float(odd->offset)/MAX_UINT16 ) );
 
         // Adding destination node 1 to the outgoing list        
         // Offset here is only for old Route Module.
         // Everywhere else the offset in node 0 is used.
         m_destList->addOrigDestInfo(
            OrigDestInfo( m_driverPref,
                          odd->mapID,
                          odd->itemID | 0x80000000,
                          odd->lat,
                          odd->lon,
                          1.0 - float(odd->offset)/MAX_UINT16 ) );
      }
   }

   // Check if it is too far to walk
   bool allowed = checkDistOKForVehicle(*m_driverPref,
                                        *m_origList,
                                        *m_destList);
   
   if ( (m_origList->size() > 0) && (m_destList->size() > 0) && allowed ) {

      // Calc the nbr of routes needed to satisfy the user.
      uint32 nbrRoutesToCalc = calcNbrWantedRoutes(m_nbrWantedRoutes);
      // If we have both origins and destinations, create a RouteSender
      // and start getting packets from it.     
      m_routingStartTime = TimeUtility::getCurrentTime();
      m_routeSender = new RouteSender( m_user,
                                       m_request,
                                       m_origList,
                                       m_destList,
                                       NULL,
                                       m_driverPref,
                                       m_routingInfo,
                                       calcMinDistFromOrigsToDest(*m_origList,
                                                                  *m_destList),
                                       m_disturbances,
                                       nbrRoutesToCalc,
                                       0,MAX_UINT32,false,false,NULL,
                                       m_allowedMaps
                                       );
      PacketContainer* pc = m_routeSender->getNextPacket();
      while (pc != NULL) {
         m_packetsReadyToSend.add( pc );
         pc = m_routeSender->getNextPacket( );
      }
      
      if ( m_routeSender->getStatus() == StringTable::OK ) {
         return (SENDING_ROUTE);
      } else {
         return setErrorState( m_routeSender->getStatus() );
      }
   } else if ( m_origList->size() == 0 ){
      // No routing if we have no origins or no destinations
      mc2log << error << ROU 
             << "Leaving createAndPrepareRoutePacket, no orig"
             << endl;
      return setErrorState( StringTable::ERROR_NO_VALID_START_ROUTING_NODE );
   } else if ( m_destList->size() == 0 ) {
      mc2log << error << ROU 
             << "Leaving createAndPrepareRoutePacket, no dest"
             << endl;
      return setErrorState( StringTable::ERROR_NO_VALID_END_ROUTING_NODE );
   } else {
      mc2log << warn << ROU 
             << "RO: Distance too far for vehicle " << hex
             << m_driverPref->getVehicleRestriction() << dec << endl;
      return setErrorState( StringTable::TOO_FAR_TO_WALK);
   }
}


void 
RouteObject::clearTemporaryRouteData() {
   // Clear origDescriptions from all non validOrigDest
  OrigDestDescVector::iterator iter = m_origDescriptions.begin();
  while ( iter != m_origDescriptions.end() ) {
      if ( !(*iter)->validOrigDest ) {
         // Remove from vector
         delete *iter;
         iter = m_origDescriptions.erase(iter);
      } else {
         //Next
         ++iter;
      }
   }
   
  //m_origDescriptions.trimToSize();

   // Clear destDescriptions from all non validOrigDest
   iter = m_destDescriptions.begin();
   while ( iter != m_destDescriptions.end() ) {
      if ( ! (*iter)->validOrigDest )
      {
         // Remove this from vector
         delete *iter;
         iter = m_destDescriptions.erase( iter );
      } else {
         // Next 
         ++iter;
      }
   }
   //m_destDescriptions.trimToSize();

   // Delete m_routeExpander as it isn't needed any more
   delete m_routeExpander;
   m_routeExpander = NULL;
}


uint32 
RouteObject::getSize() {
   uint32 size = 0;

   // Two origDestFinalPoint_t
   size += 2*( 4 + 4 + 2 );

   // Origin + Destination OrigDestPoint static sizes
   size += (m_allOrigs.size() + m_allDests.size()) *
      ( 4 + 4 + 4 + 4 + 2 + 2 + 1 + 1 );
   // OrigDestPoint names
   for ( uint32 i = 0 ; i < m_allOrigs.size() ; i++ ) {
      const char* name = m_allOrigs[ i ]->itemName;
      if ( name != NULL ) {
         size += strlen( name );
      }
      // The 1 terminating byte is in the static sizes.
   }
   for ( uint32 i = 0 ; i < m_allDests.size() ; i++ ) {
      const char* name = m_allDests[ i ]->itemName;
      if ( name != NULL ) {
         size += strlen( name );
      }
      // The 1 terminating byte is in the static sizes.
   }

   // Origin + Destination OrigDestDescription static sizes
   size += (m_origDescriptions.size() + m_destDescriptions.size()) *
      ( 4 + 4 + 4 + 4 + 4 + 2 + 2 + 1 + 1 + 1 + 1 + 1 );
   // OrigDestPoint names
   for ( uint32 i = 0 ; i < m_origDescriptions.size() ; i++ ) {
      const char* name = m_origDescriptions[ i ]->itemName;
      if ( name != NULL ) {
         size += strlen( name );
      }
      // The 1 terminating byte is in the static sizes.
   }
   for ( uint32 i = 0 ; i < m_destDescriptions.size() ; i++ ) {
      const char* name = m_destDescriptions[ i ]->itemName;
      if ( name != NULL ) {
         size += strlen( name );
      }
      // The 1 terminating byte is in the static sizes.
   }

   // m_packetsReadyToSend the empty send tree, about 4 + 1 pointers
   size += 5 * 4;
   
   // m_state
   size += 4;

   // m_expandType
   size += 4;

   // m_answer
   size += 4;
   if ( m_answer != NULL ) {
      size += m_answer->getPacket()->getLength();
   }

   // m_routeReplyPacket
   size += 4;
   if ( m_routeReplyPacket != NULL ) {
      size += m_routeReplyPacket->getLength();
   }

   // m_request
   size += 4;
   
   // m_routeExpander
   size += 4;

   return size;
}

int
RouteObject::getOriginalIndex(const OrigDestDescVector& origDestDescriptions,
                              const OrigDestPointVector& origDestPoints,
                              const origDestFinalPoint_t&
                              finalOrigOrDest )
{
   // Look for the origdestDescription with the dest as id
   for( uint32 pos = 0; pos < origDestDescriptions.size(); ++pos ) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
         (origDestDescriptions[pos]);
      
      if (odd->validOrigDest && odd->mapID == finalOrigOrDest.mapID ) {
         if ( ( odd->itemID ==
                (finalOrigOrDest.itemID & 0x7fffffff ) ) ||
              ( odd->parentItemID ==
                ( finalOrigOrDest.itemID & 0x7fffffff) ) ) {
            // Look trough the originally added items.
            for ( uint32 i = 0; i < origDestPoints.size(); ++i ) {
               OrigDestPoint* dest =
                  static_cast<OrigDestPoint*>(origDestPoints[i]);
               if ( dest->mapID == odd->mapID ) {
                  if ( dest->itemID == odd->itemID ) {
                     return i;
                  } else if ( dest->itemID == odd->parentItemID ) {
                     return i;
                  }
               }
            }               
         }
      }
   }
   return -1;
}

int
RouteObject::getOriginalDestIndex() const
{
   return getOriginalIndex(m_destDescriptions, m_allDests,
                           m_destPoint);      
}

int
RouteObject::getOriginalOrigIndex() const
{
   return getOriginalIndex(m_origDescriptions, m_allOrigs,
                           m_origPoint);  
}

int
RouteObject::getOriginalOrigOrDestIndex(bool dest) const
{
   if ( dest ) {
      return getOriginalDestIndex();
   } else {
      return getOriginalOrigIndex();
   }
}

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

#include "RMSubRoutePacket.h"
#include "RMSubRoute.h"
#include "SubRouteList.h"
#include "multicast.h"
#include "DisturbanceList.h"
#include "GfxConstants.h"
#include "OrigDestNodes.h"
#include "RoutingMap.h"

#define ORIG_DEST_SIZE 20
#define SUB_ROUTE_SIZE 12
#define SUB_ROUTE_CONNECTION_SIZE 16

RMSubRouteRequestPacket::RMSubRouteRequestPacket(uint32 leaderIP,
                                                 uint16 leaderPort,
                                                 SubRouteList* subRouteList,
                                                 RMDriverPref* driverPref,
                                                 Head* origin, 
                                                 Head* dest,
                                                 bool original,
                                                 uint32 mapID)
{
   DEBUG4(mc2dbg4
          << "RMSubRouteRequestPacket::RMSubRouteRequestPacket" << endl);
   // Calculate size of request
   // Origins and destinations
   uint32 reqSize = SUBROUTE_REQUEST_HEADER_SIZE;

   // Origin. Will probably alloc some more than necessary.
   if ( origin != NULL && original )
      reqSize += origin->cardinal() * ORIG_DEST_SIZE;
   // Destinations 
   if ( dest != NULL )
      reqSize += dest->cardinal() * ORIG_DEST_SIZE;
   // Subroutes.
   for(uint32 i=0; i < subRouteList->getNbrSubRoutes() ; i++ ) {
      RMSubRoute* subRoute = subRouteList->getSubRoute(i);
      reqSize += SUB_ROUTE_SIZE +
         SUB_ROUTE_CONNECTION_SIZE * subRoute->getNbrConnections();
   }

   if ( reqSize > getBufSize() ) {
      mc2dbg << "RMSubRouteRequestPacket - buffer too small reallocing"
             << endl;
      
      byte* temp = MAKE_UINT32_ALIGNED_BYTE_BUFFER( reqSize );
      memcpy(temp, this->buffer, SUBROUTE_REQUEST_HEADER_SIZE);      
      delete this->buffer;
      this->buffer = temp;
      this->bufSize = reqSize;
   }
   
   setOriginIP(leaderIP);
   setOriginPort(leaderPort);
   setSubType(Packet::PACKETTYPE_SUBROUTEREQUEST);
   setMapID(mapID);
   setVehicleRestrictions(driverPref->getVehicleRestriction());
   setRoutingCosts(driverPref->getRoutingCosts());
   setUseTurnCost(driverPref->useUturn());
   setTime(driverPref->getTime());
   setMinWaitTime(driverPref->getMinWaitTime());
   
   setListType(subRouteList->getListType());
   setRouteToAll(false);
   setRouteID(subRouteList->getRouteID());
   setCutOff(subRouteList->getCutOff());
   
   setIsOriginalRequest(original);
   
   // Add the origins that belongs to this mapID and only the first time.
   setNbrOrigins(0);
   int pos = SUBROUTE_REQUEST_HEADER_SIZE;
   if ((origin != NULL) && original) {
      OrigDestNode* tempNode = static_cast<OrigDestNode*>(origin->first());
      while (tempNode != NULL) {
         if (mapID == tempNode->getMapID())
            addOrigin(tempNode, pos);
         tempNode = static_cast<OrigDestNode*>(tempNode->suc()); 
      }
   }
   
   // Add the destinations (always add all for estimation).
   setNbrDestinations(0);
   if (dest != NULL) {
      OrigDestNode* tempNode = static_cast<OrigDestNode*>(dest->first());
      while (tempNode != NULL) {
         addDestination(tempNode, pos);
         tempNode = static_cast<OrigDestNode*>(tempNode->suc());
      }
   }
   
   // Add the subroutes..
   setNbrSubRoutes(subRouteList->getNbrSubRoutes());
   for (uint32 i = 0; i < subRouteList->getNbrSubRoutes(); i++) {
      RMSubRoute* subRoute = subRouteList->getSubRoute(i);
      if (subRoute != NULL) {
         addSubRoute(subRoute, pos);
      }
   }   
   setLength(pos);
   if ( true || pos > (int)reqSize || (reqSize - pos) > 65536 ) {
      mc2dbg << "RMSubRouteRequestPacket - calcSize = " << reqSize 
             << "real size = " << pos << endl;
   }
} // Constructor


void
RMSubRouteRequestPacket::getSubRouteList( SubRouteList* subRouteList,
                                          DisturbanceVector* disturbances) const
{
   DEBUG4( mc2dbg4 << "RMSubRouteRequestPacket::getSubRouteList" << endl );
   int pos = SUBROUTE_REQUEST_HEADER_SIZE + 
      ( getNbrOrigins() + getNbrDestinations() ) * ORIG_DEST_SIZE;
   
   if( subRouteList != NULL ){

      // Add header parameters
      subRouteList->setListType( getListType() );
      subRouteList->setRouteID( getRouteID() );
      subRouteList->setCutOff( getCutOff() );

      // Add the subroutes
      uint32 nbrSubRoutes = getNbrSubRoutes();   
   
      for( uint32 i=0 ; i<nbrSubRoutes; i++ ) {
         RMSubRoute* subRoute = new CompleteRMSubRoute;
         subRoute->setSubRouteID( incReadLong(pos) );
         subRoute->setPrevSubRouteID( incReadLong(pos) );
         subRoute->setRouteComplete( bool( incReadByte(pos) ) );
         subRoute->setForward( bool( incReadByte(pos) ) );      

         uint16 nbrConnections = incReadShort(pos);
         for (uint32 j = 0; j < nbrConnections; j++) {
            uint32 mapID         = incReadLong(pos);
            uint32 nodeID        = incReadLong(pos);
            uint32 cost          = incReadLong(pos);
            uint32 estimatedCost = incReadLong(pos);
            uint32 costASum      = incReadLong(pos);
            uint32 costBSum      = incReadLong(pos);
            uint32 costCSum      = incReadLong(pos);
            subRoute->addExternal(mapID, nodeID, cost, estimatedCost,
                                  GfxConstants::IMPOSSIBLE,
                                  GfxConstants::IMPOSSIBLE,
                                  costASum,
                                  costBSum,
                                  costCSum);
         }      

         uint32 index = subRouteList->addSubRoute(subRoute);
         if( index == MAX_UINT32 )
            delete subRoute;
         else
            subRoute->setVisited(subRoute->getRouteComplete());
      }
   }
   
   if ( disturbances != NULL ) {
      int nbrDist = incReadLong(pos);
      disturbances->reserve(nbrDist);
      for(int i=0; i < nbrDist; ++i ) {
         uint32 mapID     = incReadLong(pos);
         uint32 nodeID    = incReadLong(pos);
         // FIXME: It shouldn't be necessary to send this much data
         //        since we do not use both time and factor.
         uint32 rawFactor = incReadLong(pos);
         uint32 useFactor = incReadLong(pos);
         uint32 time      = incReadLong(pos);
         DisturbanceListElement* el = 
            new DisturbanceListElement(mapID, nodeID, rawFactor, useFactor,
                                       time);
         disturbances->push_back(el);
      }
   }
   if ( pos >= (int)getLength() ) {
      mc2dbg << "[RMSRP]: No more data in packet. No infomoduledists. "
             << "Old server?" << endl;
      return;
   }
   if ( disturbances != NULL ) {
      // Add the dists from the InfoModule
      int nbrDist = incReadLong(pos);
      IDPair_t id( getMapID(), MAX_UINT32 );
      for( int i = 0; i < nbrDist; ++i ) {
         incReadLong( pos, id.second );
         uint32 rawFactor = incReadLong( pos );
         disturbances->push_back( new DisturbanceListElement( id.first,
                                                              id.second,
                                                              rawFactor,
                                                              true,
                                                              0 ) );
      }
   }
} 


void
RMSubRouteRequestPacket::getOrigins( RoutingMap* theMap,
                                     Head* origins ) const
{
   int pos = SUBROUTE_REQUEST_HEADER_SIZE;
   int nbrOrigins = getNbrOrigins();
   for( int i=0; i<nbrOrigins; i++ ){

      uint32 mapID  = incReadLong( pos );
      uint32 itemID = incReadLong( pos );
      uint16 offset = incReadShort( pos );

      mc2dbg8 << "New origin " << mapID << ':'
              << hex << itemID << dec << endl;
      
      OrigDestNode* temp = theMap->newOrigDestNode( IDPair_t(mapID, itemID),
                                                    offset );
      temp->setStartDirection( bool( incReadByte( pos ) ) );
      incReadByte( pos ); // Pad byte

      temp->setLat( incReadLong( pos ) );
      temp->setLong( incReadLong( pos ) );
      temp->into( origins );
   }
}


void
RMSubRouteRequestPacket::getDestinations( RoutingMap* theMap,
                                          Head* destinations ) const
{
   int pos = SUBROUTE_REQUEST_HEADER_SIZE + getNbrOrigins()*ORIG_DEST_SIZE;
   int nbrDest = getNbrDestinations();
   for( int i=0; i<nbrDest; i++ ){

      uint32 mapID  = incReadLong( pos );
      uint32 itemID = incReadLong( pos );
      uint16 offset = incReadShort( pos );
      incReadShort( pos ); // Pad byte

      int32 lat = incReadLong( pos );
      int32 lon = incReadLong( pos );
      if( mapID == getMapID() ) {
         mc2dbg8 << "New destination " << mapID << ':'
                << hex << itemID << dec << endl;
         OrigDestNode* temp = theMap->newOrigDestNode( IDPair_t(mapID, itemID),
                                                       offset );
         temp->setLat( lat );
         temp->setLong( lon );
         temp->into( destinations );
      }
   }
}


void
RMSubRouteRequestPacket::getAllDestinations( RoutingMap* theMap,
                                             Head* destinations ) const
{
   int pos = SUBROUTE_REQUEST_HEADER_SIZE + getNbrOrigins()*ORIG_DEST_SIZE;
   int nbrDest = getNbrDestinations();
   for( int i=0; i<nbrDest; i++ ){

      uint32 mapID  = incReadLong( pos );
      uint32 itemID = incReadLong( pos );
      uint16 offset = incReadShort( pos );

      OrigDestNode* temp = theMap->newOrigDestNode( IDPair_t(mapID, itemID),
                                                    offset );

      incReadShort( pos ); // Pad byte

      temp->setLat( incReadLong( pos ) );
      temp->setLong( incReadLong( pos ) );
      temp->into( destinations );
   }
}

void RMSubRouteRequestPacket::getDriverPreferences( RMDriverPref* driverPref ) const
{   
   driverPref->setVehicleRestriction( getVehicleRestrictions() );
   driverPref->setRoutingCosts( getRoutingCosts() );
   driverPref->setUturn( getUseTurnCost() );
   driverPref->setTime( getTime() );
   driverPref->setMinWaitTime( getMinWaitTime() );
}


void RMSubRouteRequestPacket::addOrigin( OrigDestNode* origin,
                                         int32& pos )
{
   incWriteLong( pos, origin->getMapID() );
   incWriteLong( pos, origin->getItemID() );

   incWriteShort( pos, origin->getOffset() );
   incWriteByte( pos, origin->getStartDirection() );
   incWriteByte( pos, 0 );  // pad byte

   incWriteLong( pos, origin->getLat() );
   incWriteLong( pos, origin->getLong() );
   setNbrOrigins( getNbrOrigins()+1 );
}

void RMSubRouteRequestPacket::addDestination( OrigDestNode* dest,
                                              int32& pos )
{
   incWriteLong( pos, dest->getMapID() );
   incWriteLong( pos, dest->getItemID() );

   incWriteShort( pos, dest->getOffset() );
   incWriteShort( pos, 0 );                  // pad bytes

   incWriteLong( pos, dest->getLat() );
   incWriteLong( pos, dest->getLong() );
   setNbrDestinations( getNbrDestinations()+1 );
}

void RMSubRouteRequestPacket::addSubRoute( RMSubRoute* subRoute,
                                           int32& pos )
{
   incWriteLong( pos, subRoute->getSubRouteID() );
   incWriteLong( pos, subRoute->getPrevSubRouteID() );
   incWriteByte( pos, uint8(subRoute->getRouteComplete()) );
   incWriteByte( pos, uint8(subRoute->isForward()) );

   uint32 nbrConnections = subRoute->getNbrConnections();
   incWriteShort( pos, nbrConnections );
   for (uint32 j = 0; j < nbrConnections; j++) {
      uint32 mapID, nodeID, cost, estimatedCost;
      int32 lat, lon;
      uint32 dummy; // Sum for costs. Not used here, only in SubRoutePacket
      subRoute->getExternal( j, mapID, nodeID, cost, estimatedCost, lat, lon,
                             dummy, dummy, dummy);
      incWriteLong(pos, mapID);
      incWriteLong(pos, nodeID);
      incWriteLong(pos, cost);
      incWriteLong(pos, estimatedCost);
      // Don't send lat/lon. The modules knows them already
   }
}

void RMSubRouteRequestPacket::dumpRoutePacket()
{
   cout << "RMSubRouteRequestPacket:" << endl;
   cout << "......................" << endl;
   cout << "routeID             " << getRouteID() << endl;
   cout << "cutOff              " << getCutOff() << endl;
   cout << "routingCosts        " << hex << getRoutingCosts() << dec
        << endl;
   cout << "vehicleRestrictions " << hex << getVehicleRestrictions() << dec
        << endl;
   cout << "listType            " << getListType() << endl;
   cout << "firsttime request   " << BP(isOriginalRequest()) << endl;
   cout << "uses turnCost       " << BP(getUseTurnCost()) << endl;
   cout << "UMT time            " << getTime() << endl;
   cout << "minWaitTime         " << getMinWaitTime() << endl;
   uint16 nbrSubRoutes            = getNbrSubRoutes();
   cout << "nbrSubRoutes        " << nbrSubRoutes << endl;
   int nbrOrigins      = getNbrOrigins();
   int nbrDestinations = getNbrDestinations();
   cout << "nbrOrigins          " << nbrOrigins << endl;
   cout << "nbrDestinations     " << nbrDestinations << endl;
   cout << "...................." << endl;

   int pos = SUBROUTE_REQUEST_HEADER_SIZE;

   for (int i = 0; i < nbrOrigins; i++) {
      cout << "   origin        " << i << ":" << endl;
      cout << "   mapID         " << incReadLong(pos) << endl;
      uint32 itemID = incReadLong(pos);
      cout << "   itemID        " << hex << itemID << dec << " (" <<
         itemID << ")" << endl;  
      cout << "   offset        " << incReadShort(pos) << endl;
      cout << "   isDirTowards0 " << BP(incReadByte(pos)) << endl;
      incReadByte(pos); // padbyte
      cout << "   latitude      " << incReadLong(pos) << endl;
      cout << "   longitude     " << incReadLong(pos) << endl;
      cout << "................." << endl;
   }
   
   for (int32 j = 0; j < nbrDestinations; j++) {
      cout << "   destination " << j << ":" << endl;
      cout << "   mapID         " << incReadLong(pos) << endl;
      uint32 itemID = incReadLong(pos);
      cout << "   itemID        " << hex << itemID << " (" << dec
           << itemID << ")"  << endl;
      cout << "   offset        " << incReadShort(pos) << endl;
      incReadShort(pos); // 2 padbytes
      cout << "   latitude      " << incReadLong(pos) << endl;
      cout << "   longitude     " << incReadLong(pos) << endl;
      cout << "................." << endl;
   }

   cout << "................." << endl;
   cout << "................." << endl;
   cout << "................." << endl;
   cout << "................." << endl;
   
   for (uint32 k = 0; k < nbrSubRoutes; k++) {
      cout << "subRoute " << k << ":" << endl;
      cout << "subRouteID     " << incReadLong(pos) << endl;
      cout << "prevSubRouteID " << incReadLong(pos) << endl;
      cout << "routeComplete  " << BP(incReadByte(pos)) << endl;
      cout << "forward        " << BP(incReadByte(pos)) << endl;
      uint32 nbrConnections = incReadShort(pos);
      cout << "nbrConnections " << nbrConnections << endl;
      cout << "...................." << endl;
      for (uint32 l = 0; l < nbrConnections; l++) {
         cout << "connection " << l << " from subroute " << k << ":"
              << endl;
         cout << "leadsToMapID    " << incReadLong(pos) << endl;
         cout << "leadsToNodeID   " << incReadLong(pos) << endl;
         cout << "costToReachNode " << incReadLong(pos) << endl;
         cout << "estimatedCost   " << incReadLong(pos) << endl;
         cout << "................" << endl;
      }
      cout << ".........................................." << endl;
   }
      
} // dumpRoutePacket


// ========================================================================
//                                                    RMSubRouteReplyPacket =

RMSubRouteReplyPacket::RMSubRouteReplyPacket(const RMSubRouteRequestPacket*
                                             requestPacket,
                                             uint32 listSize,
                                             uint32 status) 
      : ReplyPacket(listSize + SUBROUTE_REPLY_PACKET_SIZE,
                    Packet::PACKETTYPE_SUBROUTEREPLY,
                    requestPacket,
                    status)
{
   DEBUG4( cerr << "RMSubRouteReplyPacket::RMSubRouteReplyPacket" << endl );
   DEBUG4( cerr << "size " << listSize << endl );

   setListType(0); // Perhaps something less hardcoded??
   setRouteID(requestPacket->getRouteID());
   setCutOff( MAX_UINT32 );
   setNbrSubRoutes( 0 );
   setLength(SUBROUTE_REPLY_PACKET_SIZE);
}


void
RMSubRouteReplyPacket::addSubRouteList(SubRouteList* subRouteList)
{
   DEBUG4( cerr << "RMSubRouteReplyPacket::addSubRouteList" << endl );

   uint16 listType = subRouteList->getListType();
   setListType(listType);
   setCutOff(subRouteList->getCutOff());
   // This is redundant and could probably be removed.
   // routeID must be set in constructor
   setRouteID(subRouteList->getRouteID());
   uint32 nbrSubRoutes = subRouteList->getNbrSubRoutes();
   setNbrSubRoutes(nbrSubRoutes);

   int pos = SUBROUTE_REPLY_PACKET_SIZE;
   // Add all SubRoutes to the packet
   for (uint32 i = 0; i < nbrSubRoutes; i++) {
      CompleteRMSubRoute* subRoute =
         static_cast<CompleteRMSubRoute*>(subRouteList->getSubRoute(i));
      
         
      incWriteLong(pos, subRoute->getSubRouteID());
      incWriteLong(pos, subRoute->getPrevSubRouteID());
      incWriteByte(pos, uint8(subRoute->getRouteComplete()));
      incWriteByte(pos, uint8(subRoute->isForward()));
      incWriteShort(pos , 0);
      incWriteLong(pos, subRoute->getMapID());
         
      const uint32 nbrConnections = subRoute->getNbrConnections();
      incWriteLong(pos, nbrConnections);

      Vector* nodeIDs = subRoute->getNodeIDs();
      incWriteLong(pos, nodeIDs->getSize());
         
      uint32 mapID, nodeID, cost, estimatedCost;
      int32 lat, lon;
      uint32 costAsum;
      uint32 costBsum;
      uint32 costCsum;
      
      for (uint32 j = 0; j < nbrConnections; j++) {
         subRoute->getExternal(j, mapID, nodeID, cost, estimatedCost,
                               lat, lon, costAsum, costBsum, costCsum);
         mc2dbg8 << "RMSRP: Costs are "
                 << costAsum << ':'
                 << costBsum << ':'
                 << costCsum << endl;
            
         incWriteLong(pos, mapID);
         incWriteLong(pos, nodeID);
         incWriteLong(pos, cost);
         incWriteLong(pos, estimatedCost);
         incWriteLong(pos, lat);
         incWriteLong(pos, lon);
         mc2dbg4 << "Writing offset = "
                 << (float(subRoute->getEndOffset())/float(MAX_UINT16))
                 << endl;
         incWriteShort(pos, subRoute->getEndOffset());

         // Write the costs that could be used for the sort-dist
         // requests.
         if ( (costAsum == 0) && (costBsum == 0) && (costCsum == 0) ) {
            // All costs are zero. The other end should know what
            // to do.
            incWriteShort(pos, 0);
         } else {
            incWriteShort(pos, 3); // Nbr of costs (Cost A-C)
            incWriteLong(pos, costAsum); // Not calculated yet A
            incWriteLong(pos, costBsum); // Not calculated yet B
            incWriteLong(pos, costCsum); // Not calculated yet.C
         }
      }
         
      if (listType == SubRouteListTypes::PROXIMITY_REQUEST) {
         CompleteProximityRMSubRoute* proximitySubRoute =
            static_cast<CompleteProximityRMSubRoute*>(subRoute);
         Vector offsets   = proximitySubRoute->getOffsets();
         Vector nodeCosts = proximitySubRoute->getNodeCosts();
         for (uint32 k = 0; k < nodeIDs->getSize(); k++) {
            incWriteLong(pos, nodeIDs->getElementAt(k));
            incWriteLong(pos, offsets.getElementAt(k));
            incWriteLong(pos, nodeCosts.getElementAt(k));
         }
      } else {
         for (uint32 k = 0; k < nodeIDs->getSize(); k++)
            incWriteLong(pos, nodeIDs->getElementAt(k));
      }
   }
   setLength(pos);
} // addSubRouteList

void RMSubRouteReplyPacket::getSubRouteList( SubRouteList* subRouteList )
{
   int pos = SUBROUTE_REPLY_PACKET_SIZE;
   
   const uint16 listType = getListType();
   subRouteList->setListType(listType);
   subRouteList->setRouteID(getRouteID());
   subRouteList->setCutOff(getCutOff());
   const uint32 nbrSubRoutes = getNbrSubRoutes();
   
   for (uint32 i = 0; i < nbrSubRoutes; i++) {
      CompleteRMSubRoute* subRoute;
      if (listType == SubRouteListTypes::PROXIMITY_REQUEST) {
         subRoute = new CompleteProximityRMSubRoute;
      } else {
         subRoute = new CompleteRMSubRoute;
      }

      subRoute->setSubRouteID(incReadLong(pos));
      subRoute->setPrevSubRouteID(incReadLong(pos));
      subRoute->setRouteComplete(bool(incReadByte(pos)));
      subRoute->setForward(bool(incReadByte(pos)));
      incReadShort( pos );
      subRoute->setMapID( incReadLong(pos) );
      
      uint32 nbrConnections = incReadLong(pos);
      uint32 nbrNodes = incReadLong(pos);      
      
      for (uint32 j = 0; j < nbrConnections; j++) {
         uint32 mapID         = incReadLong(pos);
         uint32 nodeID        = incReadLong(pos);
         uint32 cost          = incReadLong(pos);
         uint32 estimatedCost = incReadLong(pos);
         int32 lat            = incReadLong(pos);
         int32 lon            = incReadLong(pos);
         incReadShort(pos);  // Offset - only for RouteSender
         // Number of costs a-c
         int nbrCosts = incReadShort(pos);
         for ( int x = 0; x < nbrCosts; ++x )
            incReadLong(pos);
         subRoute->addExternal(mapID, nodeID, cost, estimatedCost, lat, lon,
                               0,0,0);
      }
      
      if (listType == SubRouteListTypes::PROXIMITY_REQUEST) {
         CompleteProximityRMSubRoute* proximitySubRoute =
            static_cast<CompleteProximityRMSubRoute*>(subRoute);
         // Kopiering, kasst
         Vector offsets = proximitySubRoute->getOffsets();
         Vector nodeCosts = proximitySubRoute->getNodeCosts();
         for (uint32 j = 0; j < nbrNodes; j++) {
            subRoute->addEndNode(incReadLong(pos));
            offsets.addLast(incReadLong(pos));
            nodeCosts.addLast(incReadLong(pos));
         }
      }
      else {
         for (uint32 j = 0; j < nbrNodes; j++)
            subRoute->addEndNode(incReadLong(pos));
      }
      
      uint32 index = subRouteList->addSubRoute(subRoute);
      if (index == MAX_UINT32) {
         delete subRoute;
      }
      else {
         subRoute->setVisited(subRoute->getRouteComplete());
         // Has to be done, because addSubRoute resets m_visited
      }
   }
} // getSubRouteList

void
RMSubRouteReplyPacket::dumpRoutePacket()
{
   cout << "RMSubRouteReplyPacket: " << endl;
   cout << "....................." << endl;
   uint16 listType = getListType();
   cout << "listType             " << listType << endl;
   cout << "routeID              " << getRouteID() << endl;
   cout << "cutOff               " << getCutOff() << endl;
   uint32 nbrSubRoutes = getNbrSubRoutes();
   cout << "nbrSubRoutes         " << nbrSubRoutes << endl;
   cout << "....................." << endl;

   int pos = SUBROUTE_REPLY_PACKET_SIZE;
   for (uint32 j = 0; j < nbrSubRoutes; j++) {
      cout << "subRoute " << j << ":" << endl; 
      cout << "subRouteID     " << incReadLong(pos) << endl;
      cout << "prevSubRouteID " << incReadLong(pos) << endl;
      cout << "routeComplete  " << BP(incReadByte(pos)) << endl;
      cout << "forward        " << BP(incReadByte(pos)) << endl;
      pos +=2;    // padbytes
      cout << "comesFromMapID " << incReadLong(pos) << endl;
      uint32 nbrConnections = incReadLong(pos);
      cout << "nbrConnections " << nbrConnections << endl;
      uint32 nbrNodes = incReadLong(pos);
      cout << "nbrNodes       " << nbrNodes << endl;
      cout << "..............." << endl;
      
      for (uint32 k = 0; k < nbrConnections; k++) {
         cout << "connection     " << k << ":" << endl;
         cout << "mapID          " << incReadLong(pos) << endl;
         cout << "nodeID         " << hex << "0x" << incReadLong(pos)
              << dec << endl;
         cout << "cost           " << incReadLong(pos) << endl;
         cout << "estimated cost " << incReadLong(pos) << endl;
      }
      cout << "..............." << endl;
      
      for (uint32 l = 0; l < nbrNodes; l++) {
         uint32 nodeID = incReadLong(pos);
         cout << "0x" << hex << nodeID << dec; 
         if (listType == SubRouteListTypes::PROXIMITY_REQUEST) {
            cout << ", offset " << incReadShort(pos) << " and cost "
                 << incReadLong(pos) << endl;
         }
         else {
            cout << endl;
         }
      }
   }      
} // dumpRoutePacket

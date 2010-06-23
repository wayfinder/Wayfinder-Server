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
#include "RoutePacket.h"
#include <vector>
#include "StringTable.h"
#include "DataBuffer.h"
#include "OrigDestInfo.h"
#include "SubRoute.h"
#include "SubRouteVector.h"
#include "Vehicle.h"
#include "DriverPref.h"


#define IS_STATE_ELEMENT(a) ( ( (a) & 0xff000000 ) == 0xf0000000 )

RouteItem::RouteItem( bool   isOrigin,
                      bool   dirFromZero,
                      uint16 offset,
                      uint32 mapID,
                      uint32 itemID,
                      int32 lat,
                      int32 lon )
{
   m_isOrigin = isOrigin;
   m_dirFromZero = dirFromZero;
   m_percentToZero = offset;
   m_mapID = mapID;
   m_itemID = itemID;
   m_latitude = lat;
   m_longitude = lon;
}

void RouteItem::dump()
{  
   cerr << "m_isOrigin    " << (m_isOrigin ? "true" : "false" ) << endl
        << "m_dirFromZero   " << (m_dirFromZero ? "true" : "false" ) << endl
        << "percentToZero " << m_percentToZero << endl
        << "m_mapID       " << m_mapID << endl
        << "m_itemID      " << m_itemID << endl
        << "m_latitude    " << m_latitude << endl
        << "m_longitude   " << m_longitude << endl;
  
} 

// =======================================================================
//                                                      RouteReplyPacket =
// =======================================================================

RouteReplyPacket::RouteReplyPacket()
      :  ReplyPacket( MAX_PACKET_SIZE,
                      Packet::PACKETTYPE_ROUTEREPLY )
{
   mc2dbg8 << "RouteReplyPacket::RouteReplyPacket()" << endl;
   setLength(REPLY_HEADER_SIZE+ROUTE_REPLY_SUB_HEADER_SIZE);
   resetNbrItems();
   setStatus(StringTable::OK);
}


RouteReplyPacket::RouteReplyPacket(const DriverPref* pref,
                                   const SubRouteVector& vect)
      : ReplyPacket( MAX_PACKET_SIZE,
                     Packet::PACKETTYPE_ROUTEREPLY)
{
   if ( vect.getSize() == 0 ){
      setStatus(StringTable::ERROR_NO_ROUTE);
   } else {
      setLength(REPLY_HEADER_SIZE+ROUTE_REPLY_SUB_HEADER_SIZE);
      setStatus(StringTable::OK);
      setStartOffset( vect.getStartOffsetUint16() );
      setEndOffset( vect.getEndOffsetUint16() );
      setStartDir( ! vect.getStartDirFromZero() );
      setEndDir ( vect.getEndDirFromZero() );
      resetNbrItems();
      // Add all the node and mapID:s
      for(uint32 i = 0; i < vect.getSize(); ++i ) {
         const SubRoute* subRoute = vect.getSubRouteAt(i);
         if ( subRoute->getOrigVehicle()->getVehicleMask()
              != ItemTypes::passengerCar ) {
            // Insert state element if there should be one first in
            // the packet.
            ItemTypes::transportation_t transport =
               ItemTypes::vehicle2Transportation(
                  subRoute->getOrigVehicle()->getVehicleMask());            
            addRouteItem(subRoute->getThisMapID(),
                         transport |
                         0xf0000000 );
         }
         // Add the origin
         addRouteItem( subRoute->getThisMapID(), subRoute->getOrigNodeID());
         // Add all the other nodes.
         for(uint32 j=0; j < subRoute->size(); j++) {
            addRouteItem( subRoute->getThisMapID(),
                          subRoute->getNodeID(j) );
         }
      }
      // Add last destination too. It should be on the same map as
      // the last added nodes.
      SubRoute* lastSubRoute = vect.getSubRouteAt( vect.getSize() - 1 );
      addRouteItem( lastSubRoute->getNextMapID(),
                    lastSubRoute->getDestNodeID());
   }
}

void
RouteReplyPacket::addRouteItem( uint32 mapID, 
                                uint32 itemID ){
   // Double the size if there isn't room for 50 more bytes.
   if ( updateSize( 50, getBufSize()) ) {
      mc2dbg << "[RRP]: Resized packet to " << getBufSize()
             << " bytes" << endl;
   }
      
   int position = getLength();
   incWriteLong(position,mapID);
   incWriteLong(position,itemID);
   incNbrItems();
   setLength( position );
}

void
RouteReplyPacket::getRouteItem(  uint32 i, 
                                 uint32 &mapID, 
                                 uint32 &itemID) const
{
   int position = REPLY_HEADER_SIZE + ROUTE_REPLY_SUB_HEADER_SIZE + i*8;
   mapID          = incReadLong(position);
   itemID         = incReadLong(position);
}
 
uint16
RouteReplyPacket::getStartOffset() const
{
   return (readShort(REPLY_HEADER_SIZE+4));
}

void
RouteReplyPacket::setStartOffset(uint16 sOffset)
{
   writeShort(REPLY_HEADER_SIZE+4, sOffset);
}

uint16
RouteReplyPacket::getEndOffset() const
{
   return (readShort(REPLY_HEADER_SIZE+6));
}

void
RouteReplyPacket::setEndOffset(uint16 eOffset)
{
   writeShort(REPLY_HEADER_SIZE+6, eOffset);
}

const Vehicle*
RouteReplyPacket::stateElementToVehicle(const DriverPref* driverPref,
                                        uint32 stateElement)
{
   uint32 vehicleToFind = ItemTypes::passengerCar;
   switch ( stateElement ) {
      case 0xf0000002:
         vehicleToFind = ItemTypes::pedestrian;
         break;
      case 0xf0000003:
         vehicleToFind = ItemTypes::bicycle;
         break;
   }
   
   for(int i=0; i < driverPref->getNbrVehicles(); ++i ) {
      const Vehicle* curVeh = driverPref->getVehicle(i);
      if ( curVeh->getVehicleMask() == vehicleToFind )
         return curVeh;
   }
   mc2log << warn << here << " Couldn't find suitable vehicle for "
          << "state element " << hex << stateElement << dec << endl;
   return driverPref->getBestVehicle();
}

SubRouteVector*
RouteReplyPacket::createSubRouteVector(const DriverPref* driverPref) const
{
   // I won't do this function especially fast, since
   // I hope that this packet will deprecated by the
   // new RouteModule.
   // This function is very much inspired by ExpandRouteConcatenator.

   // Now we have vehicles.
   const Vehicle* vehicle = driverPref->getBestVehicle();
   float startOffset = float(getStartOffset()) / MAX_UINT16;
   float endOffset   = float(getEndOffset()) / MAX_UINT16;
   //bool startDir     = getStartDir();
   //bool endDir       = getEndDir();

   uint32 currMapID  = MAX_UINT32;
   uint32 lastNodeID = MAX_UINT32;
   uint32 lastMapID  = MAX_UINT32;
   uint32 nodeID     = MAX_UINT32;
   
   typedef vector<uint32> nodeVect_t;
   typedef pair<uint32, nodeVect_t*> mapPair_t;
   typedef vector<mapPair_t> mapNodeVect_t;

   mapNodeVect_t nodesPerMap;
   nodeVect_t* nodesForThisMap = NULL;
   
   for(uint32 i=0; i < getNbrItems(); ++i) {
      lastNodeID = nodeID;
      uint32 mapID;
      getRouteItem(i, mapID, nodeID);
      // Create SubRoute if we are on a new map
      if ( currMapID != mapID ) {
         nodesForThisMap = new nodeVect_t;
         // MapID - insert the pair into the vector of maps
         nodesPerMap.push_back(mapPair_t(mapID, nodesForThisMap));
         lastMapID = currMapID;
         currMapID = mapID;
      }
      // Add node to current vector of nodes
      nodesForThisMap->push_back(nodeID);
   }
   mc2dbg4 << "Found " << nodesPerMap.size() << " maps in RouteReply" << endl;

   // Now we must create the vector
   SubRouteVector* resVect = new SubRouteVector();

   // Only add the orig and destination once.
   // No vehicle today.
   uint32 i = 0;
   mc2dbg << "[RRP]:";
   for(mapNodeVect_t::iterator it = nodesPerMap.begin();
       it != nodesPerMap.end();
       ++it) {
      // Get the nodes in a handy format.
      uint32 mapID = (*it).first;
      nodeVect_t* nodes = (*it).second;

      OrigDestInfo* orig = NULL;
      uint32 startNode = 1; // The first node to add among the normal nodes.
      
      // Orig is on this map. Should be the same as last dest if any.
      // Where can i find a macro for this?
      if ( IS_STATE_ELEMENT( nodes->front() ) ) {
         // Special case - the first element is a state element.
         // We will change the Vehicle instead.
         vehicle = stateElementToVehicle( driverPref, nodes->front() );
         orig = new OrigDestInfo(vehicle,
                                 mapID, (*nodes)[1], 0, 0, 0);
         // Now we can't add node 1 again
         startNode = 2;
      } else {
         orig = new OrigDestInfo(vehicle, mapID, nodes->front(), 0, 0, 0);
      }
      
      if ( it == nodesPerMap.begin() ) {
         // First SubRoute
         orig->setOffset(startOffset);
      }
      
      OrigDestInfo* dest = NULL;
      uint32 nbrNodesToAdd;
      if ( i == nodesPerMap.size() - 1 ) {
         // Last SubRoute - dest should be on this map.     
         dest = new OrigDestInfo(vehicle, mapID, nodes->back(), 0, 0, 0);
         dest->setOffset(endOffset);
         // Do not add destination twice.
         nbrNodesToAdd = nodes->size() - 1;
      } else {
         // Not last SubRoute - map id is on next map and
         // we should add all the nodes on this map.
         uint32 nextMapID = nodesPerMap[i+1].first;
         uint32 nextNodeID = nodesPerMap[i+1].second->front();
         dest = new OrigDestInfo(vehicle, nextMapID, nextNodeID, 0, 0, 0);
         nbrNodesToAdd = nodes->size();
      }
      mc2dbg << " " << MC2HEX(orig->getMapID()) << ":"
             << MC2HEX(orig->getNodeID()) << ":"
             << orig->getOffset() << " ("
             << vehicle->getName() << ")";
 
      SubRoute* curSubRoute = new SubRoute(*orig, *dest);
      // Add all the other nodes, but not the first it is only in
      // the origin. If we are on the last SubRoute we don't add
      // the destination either
      for(uint32 j=startNode; j < nbrNodesToAdd; ++j) {
         mc2dbg4 << "Adding node " << MC2HEX((*nodes)[j])
                 << " to subRoute " << i << endl;
         uint32 nodeToAdd = (*nodes)[j];
         if ( IS_STATE_ELEMENT(nodeToAdd) ) {
            // Change the vehicle, but add the node anyway.
            vehicle = stateElementToVehicle(driverPref, nodeToAdd);
         }
         curSubRoute->addNodeID(nodeToAdd);
      }

      dest->setVehicle(vehicle);
      
      mc2dbg << " -> " << MC2HEX(dest->getMapID()) << ":"
             << MC2HEX(dest->getNodeID()) << ":" 
             << dest->getOffset() << " ("
             << vehicle->getName() << ")";
      
      resVect->insertSubRoute(curSubRoute);
      ++i;
      delete orig;
      delete dest;
   }
   mc2dbg << endl;
   
   // Delete the temporary vectors
   mc2dbg << "[RRP]: Nbr of nodes in map";
   for(mapNodeVect_t::iterator it=nodesPerMap.begin();
       it != nodesPerMap.end();) {
      mc2dbg << " " << MC2HEX((*it).first) << "=" << (*it).second->size();
      delete (*it++).second;
   }
   mc2dbg << endl;
   return resVect;
}

uint32 
RouteReplyPacket::getMaxSimDataLength(/*const*/ RouteReplyPacket* rr)
{
   MC2_ASSERT(rr != NULL);
   // max header (offset, dir, etc.) + nbrItems * maxItemSize
   return 100 + rr->getNbrItems() * 24;
}

void 
RouteReplyPacket::printSimulateRouteData(/*const*/ RouteReplyPacket* rr,
                                         char* data, uint32 maxNbrBytes)
{
   MC2_ASSERT(rr != NULL);
   uint32 pos = 0;
   // Header
   pos += sprintf( data+pos, "StartOffset %u\n", rr->getStartOffset() );
   MC2_ASSERT(pos < maxNbrBytes);
   pos += sprintf( data+pos, "EndOffset %u\n", rr->getEndOffset() );
   MC2_ASSERT(pos < maxNbrBytes);
   pos += sprintf( data+pos, "StartDir %d\n", rr->getStartDir() );
   MC2_ASSERT(pos < maxNbrBytes);
   pos += sprintf( data+pos, "EndDir %d\n", rr->getEndDir() );
   MC2_ASSERT(pos < maxNbrBytes);

   // Body
   pos += sprintf( data+pos, "DATA\n" );
   MC2_ASSERT(pos < maxNbrBytes);
   uint32 mapID = 0;
   uint32 itemID = 0;
   for ( uint32 i = 0 ; i < rr->getNbrItems() ; i++ ) {
      rr->getRouteItem( i, mapID, itemID );
      pos += sprintf( data+pos, "%u %u\n", mapID, itemID );
      MC2_ASSERT(pos < maxNbrBytes);
   }

}


void
RouteReplyPacket::setNbrRouteObjectsUsed( uint32 n ) {
   int pos = REPLY_HEADER_SIZE + ROUTE_REPLY_SUB_HEADER_SIZE + 
      getNbrItems()*8;

   if ( uint32(pos) + 4 >= getBufSize() ) {
      resize( getBufSize()*2 );
   }
   writeLong( pos, n );
}

uint32
RouteReplyPacket::getNbrRouteObjectsUsed() const {
   int pos = REPLY_HEADER_SIZE + ROUTE_REPLY_SUB_HEADER_SIZE + 
      getNbrItems()*8;
   if ( uint32(pos) < getLength() ) {
      return 0;
   } else {
      return readLong( pos );
   }
}


typedef const vector<DisturbanceDescription> cddv;

void
RouteReplyPacket::setDisturbanceDescriptions( 
   const vector<DisturbanceDescription>& v )
{
   int pos = REPLY_HEADER_SIZE + ROUTE_REPLY_SUB_HEADER_SIZE + 
      getNbrItems()*8 + 4/*nbrRouteObjectsUsed*/;

   uint32 ddSize = 0;
   for ( cddv::const_iterator it = v.begin() ; it != v.end() ; ++it ) {
      ddSize += it->getSizeAsBytes();
   }

   if ( pos + ddSize + 8 >= getBufSize() ) {
      resize( getBufSize() + ddSize + 8 );
   }

   incWriteLong( pos, v.size() );
   for ( cddv::const_iterator it = v.begin() ; it != v.end() ; ++it ) {
      it->save( this, pos );
   }
}

vector<DisturbanceDescription>
RouteReplyPacket::getDisturbanceDescription() const {
   int pos = REPLY_HEADER_SIZE + ROUTE_REPLY_SUB_HEADER_SIZE + 
      getNbrItems()*8 + 4/*nbrRouteObjectsUsed*/;

   vector<DisturbanceDescription> v;

   if ( uint32(pos) + 4 < getLength() ) {
      uint32 nbr = incReadLong( pos );
      v.resize( nbr );
      for ( uint32 i = 0 ; i < nbr ; ++i ) {
         v[ i ].load( this, pos );
      }
   }

   return v;
}

      
void
RouteReplyPacket::setUTurn( bool uturn ) {
   writeLong( REPLY_HEADER_SIZE+8, uint32(uturn) );
}

bool
RouteReplyPacket::getUTurn() const {
   return readLong( REPLY_HEADER_SIZE+8 ) != 0;
}
 

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

#include <algorithm>
#include <set>

#include "LevelTransitObject.h"

#include "Request.h"
#include "PacketContainer.h"
#include "EdgeNodesPacket.h"
#include "IDTranslationPacket.h"
#include "DriverPref.h"
#include "SubRoute.h"
#include "Properties.h"

#include <sstream>
#include <iterator>

LevelTransitObject::LevelTransitObject( Request* request,
                                        OrigDestInfoList* origList,
                                        const OrigDestInfoList* destList,
                                        uint32 level,
                                        const DriverPref* driverPref) :
      m_origMapSet(), // Why?
      m_destMapSet(),
      m_lowerOrigMap(),
      m_lowerDestMap(),
      m_matchHighLowIDMap(),
      m_outgoingQueue(),
      m_tempQueue(),
      m_request(request),
      m_higherLevelOrigList(),
      m_lowerLevelDestList(),
      m_higherLevelDestList(),
      m_driverPrefs(driverPref),
      m_originalDests(*destList) // FIXME: Ok with pointers instead?
{
   // Get property for the maximum allowed distance from a destination
   // to the map edge
   if ( level == 1 ) {
      m_minDistFromDestToEdge =
         Properties::getUint32Property("ROUTE_MIN_DIST_TO_EDGE",
                                       1000);
   } else {
      // Don't route on neighbour maps..
      m_minDistFromDestToEdge = -1;
   }
         
   m_destPacketCounter = 0;
   m_level = level;
   m_nbrOutstanding = 0;
   m_status = StringTable::OK;

   // In this first version we take the maps on which the destinations are.
   // An improvement would be to use the criterion to find maps
   // within a certain radius from the dest.
   OrigDestInfoList::const_iterator it = destList->begin();
   while ( it != destList->end() ) {
      m_destMapSet.insert( it->getMapID() );
      it++;
   }
   
   it = origList->begin();
   while ( it != origList->end() ) {
      m_origMapSet.insert( it->getMapID() );
      it++;
   }

   m_lowerLevelOrigList = origList;
   // origList is deleted in RouteSender (owner of this LevelTransitObject)
   
   gotoFindingNodes();
}

//void
//LevelTransitObject::processFindingMaps( MapFindReplyPacket* packet  )
//{
   // Should not be implemented yet.
//}

LevelTransitObject::~LevelTransitObject()
{
   map<uint32, OrigDestInfoList*>::iterator it_map = m_lowerOrigMap.begin();
   while ( it_map != m_lowerOrigMap.end() ){
      delete it_map->second;
      it_map++;
   }
   it_map = m_lowerDestMap.begin();
      while ( it_map != m_lowerDestMap.end() ){
      delete it_map->second;
      it_map++;
   }
}

void
LevelTransitObject::gotoFindingNodes( )
{
   set<uint32>::iterator set_it;

   for ( set_it = m_destMapSet.begin();
         set_it != m_destMapSet.end();
         set_it++ ) {
      // Create packets for each of the maps to route to on higher level.
      EdgeNodesRequestPacket* pack =
         new EdgeNodesRequestPacket( m_request->getNextPacketID(),
                                     m_request->getID(),
                                     (*set_it),
                                     m_level,
                                     m_originalDests,
                                     (m_destPacketCounter | 1<<31),
                                     m_driverPrefs->getVehicleRestriction());
      
      PacketContainer* packContainer =
         new PacketContainer( pack, 0, 0, MODULE_TYPE_ROUTE);
      
      mc2dbg2 << " Created Dest ENReqP for map: "
             << *set_it
             << endl;

      
      m_outgoingQueue.add( packContainer );
      m_nbrOutstanding++;
      m_destPacketCounter++;
   }

   // Empty list for destinations - these are origins.
   OrigDestInfoList emptyDestList;
   
   uint32 packetCounter = 0;
   for ( set_it = m_origMapSet.begin();
         set_it != m_origMapSet.end();
         set_it++ ) {
      // Create packets for each of the maps to route from on higher level.
      EdgeNodesRequestPacket* pack =
         new EdgeNodesRequestPacket( m_request->getNextPacketID(),
                                     m_request->getID(),
                                     (*set_it),
                                     m_level,
                                     emptyDestList,
                                     packetCounter,
                                     MAX_UINT32 // All vehicles
                                     );
      
      PacketContainer* packContainer =
         new PacketContainer( pack, 0, 0, MODULE_TYPE_ROUTE);

      mc2dbg2 << " Created Orig ENReqP for map: "
             << *set_it
             << endl;
      
      m_outgoingQueue.add( packContainer );
      m_nbrOutstanding++;
      packetCounter++;
   }
   m_state = FINDING_NODES;
}

void
LevelTransitObject::processFindingOrigs( EdgeNodesReplyPacket* packet )
{
   uint32 lowerMapID = packet->getMapID();

   OrigDestInfoList* requestList = new OrigDestInfoList;
   OrigDestInfoList::iterator it = m_lowerLevelOrigList->begin();
   while ( it != m_lowerLevelOrigList->end() ){
      if ( it->getMapID() == lowerMapID ){
         requestList->addOrigDestInfo( *it );
         m_lowerLevelOrigList->erase( it++ );
      } else {
         it++;
      }
   }
   if ( requestList->size() == 0 ){
      mc2dbg << here
             << "requestList contained no elements..."
             << endl;
   } else {
      IDTranslationRequestPacket* pack =
         new IDTranslationRequestPacket( m_request->getNextPacketID(),
                                         m_request->getID(),
                                         packet->getOverviewMapID(),
                                         false, // Translate to higher
                                         *requestList,
                                         packet->getUserDefinedData() );
   
      PacketContainer* packContainer =
         new PacketContainer( pack, 0, 0, MODULE_TYPE_ROUTE);

      mc2dbg2 << " Created Orig IDTReqP for map: "
             << lowerMapID
             << endl;
      
      m_tempQueue.add( packContainer );

   }
   
   // Put OrigDestInfoList in a map with its serial number.
   m_lowerOrigMap.insert( pair<uint32, OrigDestInfoList*>(
      packet->getUserDefinedData(),
      requestList ) );
   requestList = NULL;
      
   // If number of outstanding packets is zero, move on to next state.
   if ( m_nbrOutstanding == 0 ) {
      // Add all the IDTranslationRequestPackets to m_outgoingQueue.
      while ( m_tempQueue.getMin() != NULL ){
         m_outgoingQueue.add( m_tempQueue.extractMin() );
         m_nbrOutstanding++;
      }
      m_state = TRANSLATING_NODES;
   }         
}

void
LevelTransitObject::processFindingDests( EdgeNodesReplyPacket* packet  )
{
   OrigDestInfoList* higherList = new OrigDestInfoList;
   map<uint32, uint32> distances;

   // Get the edge nodes. Note that the node number is
   // flipped, i.e. the packet contains the nodes closest
   // to the map edge, but since these are destinations
   // from the outside, we want the other node too.
   packet->getEdgeNodes( *higherList, &distances, true );
   packet->getEdgeNodes( *higherList, &distances, false );

   // Testing
   packet->getBorderMaps( m_destBorderMaps);
   // Testing even more
   m_destBorderMaps.clear();
   
   // Print the destination maps and level at once.
   stringstream strstr;
   strstr << "[LTO]: Allowed destination maps on level " << m_level << ":";
   copy(m_destMapSet.begin(),
        m_destMapSet.end(),
        ostream_iterator<uint32>(strstr, " "));
   copy(m_destBorderMaps.begin(),
        m_destBorderMaps.end(),
        ostream_iterator<uint32>(strstr, " "));
   strstr << endl;
   strstr << "[LTO]: Distances: ";
   for( map<uint32,uint32>::const_iterator it = distances.begin();
        it != distances.end();
        ++it ) {
      strstr << " Map = " << it->first << ", dist = " << it->second; 
   }
   strstr << endl << ends;
   mc2dbg2 << strstr.str();
   
   for( map<uint32,uint32>::const_iterator it = distances.begin();
        it != distances.end();
        ++it ) {
      if ( int(it->second) < m_minDistFromDestToEdge &&
           (m_destMapSet.find(it->first) == m_destMapSet.end()) ) {
         // There is a map border close to the destinations and the
         // map has not been used before.
         // Make a new EdgeNodesRequestPacket and add the map to the
         // destination maps.
         EdgeNodesRequestPacket* pack =
         new EdgeNodesRequestPacket( m_request->getNextPacketID(),
                                     m_request->getID(),
                                     it->first, // Map ID
                                     m_level,
                                     m_originalDests,
                                     (m_destPacketCounter | 1<<31),
                                     m_driverPrefs->getVehicleRestriction());
         mc2dbg2 << " Created Dest ENReqP for map: "
                << it->first
                << endl;
         PacketContainer* packContainer =
         new PacketContainer( pack, 0, 0, MODULE_TYPE_ROUTE);
         m_outgoingQueue.add( packContainer );
         m_nbrOutstanding++;     // Stay in this state.
         m_destPacketCounter++;  // Increase ID.
          // We want to route and not send EdgeNodesRequest again
         m_destMapSet.insert(it->first);
      }
   }
   
   
   // Create an IDTranslationRequestPacket for this edgePacket.
   // Put it in a container that will be moved over to m_outgoingQueue
   // as soon as all the replies from EdgeNodesPackets have arrived.
   IDTranslationRequestPacket* pack =
      new IDTranslationRequestPacket( m_request->getNextPacketID(),
                                      m_request->getID(),
                                      packet->getOverviewMapID(),
                                      false, // Translate to higher
                                      *higherList,
                                      packet->getUserDefinedData() );
   
   PacketContainer* packContainer =
      new PacketContainer( pack, 0, 0, MODULE_TYPE_ROUTE);

   mc2dbg2 << " Created Dest IDTReqP for map: "
          << packet->getMapID()
          << endl;
   
   m_tempQueue.add( packContainer );

   // Put OrigDestInfoList in a map with its serial number.
   m_lowerDestMap.insert( pair<uint32, OrigDestInfoList*>(
      packet->getUserDefinedData(),
      higherList ) );
   higherList = NULL;
   
   // If number of outstanding packets is zero, move on to next state.
   if ( m_nbrOutstanding == 0 ) {
      // Add all the IDTranslationRequestPackets to m_outgoingQueue.
      while ( m_tempQueue.getMin() != NULL ){
         m_outgoingQueue.add( m_tempQueue.extractMin() );
         m_nbrOutstanding++;
      }
      m_state = TRANSLATING_NODES;
   }
}

void
LevelTransitObject::processOrigTranslation( IDTranslationReplyPacket* packet )
{
   // Get the nodes on higher level from the packet. These will automatically
   // have the correct data, i.e. cost, prevSubRoute etc.
   OrigDestInfoList resultList;
   OrigDestInfoList oldList;
   if ( m_lowerOrigMap.find( packet->getUserDefinedData() ) !=
        m_lowerOrigMap.end() ){
      oldList = *( m_lowerOrigMap.find(
         packet->getUserDefinedData() )->second );
      packet->getTranslatedNodes(0,  // This index is always zero
                                 resultList,
                                 *( m_lowerOrigMap.find(
                                    packet->getUserDefinedData() )->second ) );
   } else {
      mc2log << error
             << "Orig: Got packet with strange UserDefinedData"
             << endl;
      return;
   }

   int nbrVal = 0;
   int nbrInval = 0;
   // Add these OrigDestInfo to the outgoing OrigList
   OrigDestInfoList::iterator it = resultList.begin();
   while ( it != resultList.end() ) {
      if ( it->getNodeID() != MAX_UINT32 ){
         m_higherLevelOrigList.addOrigDestInfo( *it );
         ++nbrVal;
      } else {
         ++nbrInval;
      }
      it++;
   }

   mc2dbg2 << "[LTO]: Number of translatable orignodes = " << nbrVal
          << " of " << int(nbrVal + nbrInval) << endl;
   
   if ( m_nbrOutstanding == 0 ){
      m_state = DONE;
   }
}

void
LevelTransitObject::processDestTranslation( IDTranslationReplyPacket* packet )
{
   // Read the nodes from the packet.
   // Match with serial number in an edgePacket.
   OrigDestInfoList resultList;
   OrigDestInfoList* lowLevelDestList;
   if ( m_lowerDestMap.find( packet->getUserDefinedData() ) !=
        m_lowerDestMap.end() ){
      lowLevelDestList = m_lowerDestMap.find(
         packet->getUserDefinedData() )->second;
      packet->getTranslatedNodes(0,  // This index is always zero
                                 resultList,
                                 *lowLevelDestList ); 
   } else {
      mc2log << error
             << "Dest: Got packet with strange UserDefinedData"
             << endl;
      return;
   }

   // For each node in packet, get the correct node in the saved list,
   // and put this in m_lowerLevelDestList, and at the same time
   // create entry in multimap< NodeID, pair<MapID, OrigDestInfo*> >
   // m_matchHighLowIDMap. Also put the node in packet into
   // m_higherLevelDestList.
   int nbrInval = 0; // Nbr nodes not translatable to higher
   int nbrVal   = 0;
   OrigDestInfoList::iterator it_HL = resultList.begin();
   OrigDestInfoList::iterator it_LL = lowLevelDestList->begin();
   while ( it_HL != resultList.end() && it_LL != lowLevelDestList->end() ) {
      if ( it_HL->getNodeID() != MAX_UINT32 ){
         m_higherLevelDestList.addOrigDestInfo( *it_HL );
         m_lowerLevelDestList.addOrigDestInfo( *it_LL );
         m_lowLevelDestNodes.insert(
            IDPair_t(it_LL->getMapID(), it_LL->getNodeID()) );
         m_matchHighLowIDMap.insert(
            pair< uint32, pair<uint32, OrigDestInfo*> >(
               it_HL->getNodeID(), pair<uint32, OrigDestInfo*>(
                  it_HL->getMapID(), &m_lowerLevelDestList.back() ) ) );
         mc2dbg8 << "[LTO]: Added " << it_HL->getMapID() << ":"
                 << hex << it_HL->getNodeID() << dec << " = "
                 << it_LL->getMapID() << ":"
                 << hex << it_LL->getNodeID() << dec
                 << " to matchlist"
                 << endl;
         ++nbrVal;
      } else {
         ++nbrInval;
      }
      it_HL++;
      it_LL++;
   }
   mc2dbg2 << "[LTO]: Number of translatable destnodes = " << nbrVal
          << " of " << int(nbrVal + nbrInval) << endl;
   if ( m_nbrOutstanding == 0 ){
      m_state = DONE;
   }
}

void
LevelTransitObject::processPacket( const PacketContainer* p )
{
   if ( p == NULL ) {
      // PacketContainer == NULL indicates a timeout.
      m_status = StringTable::TIMEOUT_ERROR;
      return;
   }

   m_nbrOutstanding--;

   // Check the state, and take action accordingly
   switch ( m_state ) {

      case FINDING_MAPS:
         // Packet has to be MAPFINDREPLY
         // This case will be used when dest map is not enough.
         m_state = FINDING_NODES;
         break;
         
      case FINDING_NODES:
      {
         mc2dbg2 << "In FINDING_NODES"
                << endl;
         // Packet has to be EDGENODESREPLY
         Packet* packet = p->getPacket();
         if ( packet->getSubType() == Packet::PACKETTYPE_EDGENODESREPLY ) {
            EdgeNodesReplyPacket* edgePacket =
               static_cast<EdgeNodesReplyPacket*>(packet);
            // Set the status of the RouteSender if the packet we got is not OK.
            if ( edgePacket->getStatus() != StringTable::OK ) {
               m_status = StringTable::stringCode( edgePacket->getStatus() );
            } else {
               if ( ( edgePacket->getUserDefinedData()>>31 ) == 1 ){
                  processFindingDests( edgePacket );
               } else {
                  processFindingOrigs( edgePacket );
               }
            }
         } else {
            mc2log << error
                   << "[LevelTransitObject] FINDING_NODES got packet of type "
                   << packet->getSubTypeAsString()
                   << endl;
            // We cannot recover from this.
            m_status = StringTable::INTERNAL_SERVER_ERROR;
         }
         break;
      }
      
      case TRANSLATING_NODES:
      {
         mc2dbg2 << " In TRANSLATING_NODES"
                << endl;
         // Packet has to be IDTRANSLATIONREPLY
         Packet* packet = p->getPacket();
         if ( packet->getSubType() == Packet::PACKETTYPE_IDTRANSLATIONREPLY ) {
            IDTranslationReplyPacket* transPacket =
               static_cast<IDTranslationReplyPacket*>(packet);
            // Set the status of the RouteSender if the packet we got is not OK.
            if ( transPacket->getStatus() != StringTable::OK ) {
               m_status = StringTable::stringCode( transPacket->getStatus() );
            } else {
               if ( ( transPacket->getUserDefinedData()>>31 ) == 1 ){
                  processDestTranslation( transPacket );
               } else {
                  processOrigTranslation( transPacket );
               }
            }
         } else {
            mc2log << error
                   << "[LevelTransitObject] TRANSLATING_NODES got packet of type "
                   << packet->getSubTypeAsString()
                   << endl;
             // We cannot recover from this.
             m_status = StringTable::INTERNAL_SERVER_ERROR;
         }
         break;
      }
      
      default:
         // I.e we are in DONE state.
         // No packets should arrive here
         mc2log << error
                << "[LevelTransitObject] DONE got packet of type "
                << p->getPacket()->getSubTypeAsString()
                << endl;
         // We cannot recover from this.
         m_status = StringTable::INTERNAL_SERVER_ERROR;
         
         break;
   }
   // The packet p is deleted in the Request, i.e. RouteRequest in
   // this case.
}

PacketContainer*
LevelTransitObject::getNextPacket()
{
   if (m_outgoingQueue.getMin() == NULL) {
      return NULL;
   } else {
      return m_outgoingQueue.extractMin();
   }
}

OrigDestInfo*
LevelTransitObject::getLowerLevelDest( uint32 nodeID,
                                       uint32 mapID )
{
  multimap< uint32, pair<uint32, OrigDestInfo*> >::iterator it;
  it = m_matchHighLowIDMap.lower_bound( nodeID );
  while ( it->second.first != mapID && it->first == nodeID ){
     it++;
  }
  if (it->first == nodeID){
     return it->second.second;
  } else {
     mc2log << warn
            << "Returning NULL. This is not good"
            << endl;
     return NULL;
  }
}
  

bool
LevelTransitObject::allowedMapID( uint32 mapID )
{
   // Going through the list looking for occurance  
   if ( (m_destMapSet.find( mapID ) != m_destMapSet.end()) ||
        (m_destBorderMaps.find(mapID ) != m_destBorderMaps.end()) ) {
      return true;
   } else {
      mc2dbg8 << " [LTO]: Map 0x" << hex << mapID << dec << " is not allowed"
              << endl;
      return false;
   }
}


bool
LevelTransitObject::isDest(const SubRoute* subRoute) const
{
   return (m_lowLevelDestNodes.find(IDPair_t(subRoute->getNextMapID(),
                                             subRoute->getDestNodeID() ) ) )
      != m_lowLevelDestNodes.end();
}
























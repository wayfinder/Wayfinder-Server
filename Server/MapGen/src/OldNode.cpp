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

#include "OldConnection.h"
#include "DataBuffer.h"

#include "OldNode.h"
#include "OldGenericMap.h"
#include "AllocatorTemplate.h"
#include "BitUtility.h"
#include "Utility.h"

#define MAJOR_ROAD_BIT_NBR    7
#define ROAD_TOLL_BIT_NBR     6


OldNode::OldNode(uint32 nodeID) : m_entryConnections(NULL)
{
   mc2dbg8 << "OldNode created, nodeID=" << nodeID << endl;

   m_nodeID = nodeID;

   m_majorRoad = false;
   m_entryRestrictions = ItemTypes::noRestrictions;
   m_level = 0;

   m_roadToll = false;
   m_maximumWeight = 0x7;
   m_maximumHeight = 0x3;

   m_speedLimit = MAX_BYTE;
   
   m_junctionType = ItemTypes::normalCrossing;
}

OldNode::OldNode(DataBuffer* dataBuffer, uint32 nodeID, bool fullCreate) 
   : m_entryConnections(NULL)
{
   DEBUG_DB(mc2dbg << "OldNode created, nodeID=" << nodeID << endl);

   m_nodeID = nodeID;

   if (fullCreate)
      createFromDataBuffer(dataBuffer, NULL);
}

void
OldNode::createFromDataBuffer(DataBuffer* dataBuffer, OldGenericMap* theMap) 
{
   // The first byte...
   uint8 bitField = dataBuffer->readNextByte();
   m_majorRoad = BitUtility::getBit(bitField, MAJOR_ROAD_BIT_NBR);
   m_roadToll = BitUtility::getBit(bitField, ROAD_TOLL_BIT_NBR);
   m_entryRestrictions = ItemTypes::entryrestriction_t((bitField & 0x0C) >> 2);
   // space previously used for number of lanes:
   // m_nbrLanes = ItemTypes::entryrestriction_t((bitField & 0x03));
   DEBUG_DB(mc2dbg << "OldNode::createFromDataBuffer bitfield: 0x" 
                 << hex << uint32(bitField) << dec << endl);
   DEBUG_DB(mc2dbg << "      majorRoad="
                 << Utility::convertBoolToString(m_majorRoad) << endl);
   DEBUG_DB(mc2dbg << "      roadToll=" 
                 << Utility::convertBoolToString(m_roadToll) << endl);
   DEBUG_DB(mc2dbg << "      entryRest=" << (uint32) m_entryRestrictions
            << endl);
   // The next three bytes
   m_level = (int8) dataBuffer->readNextByte();
   m_maximumWeight = dataBuffer->readNextByte();
   m_maximumHeight = dataBuffer->readNextByte();
   DEBUG_DB(mc2dbg << "      level=" << (uint32) m_level << endl);
   DEBUG_DB(mc2dbg << "      maxWidth=" << (uint32) m_maximumWeight << endl);
   DEBUG_DB(mc2dbg << "      maxHidth=" << (uint32) m_maximumHeight << endl);

   m_speedLimit = dataBuffer->readNextByte();
   m_junctionType = ItemTypes::junction_t(dataBuffer->readNextByte());
   uint32 nbrEntryConnections = dataBuffer->readNextShort();   
   DEBUG_DB(mc2dbg << "      speedLimit=" << (uint32) m_speedLimit << endl);
   DEBUG_DB(mc2dbg << "      junctionType=" << (uint32) m_junctionType <<endl);
   DEBUG_DB(mc2dbg << "      nbrConnections=" << (uint32)nbrEntryConnections
                 << endl);

   m_entryConnections = NULL;
   if ( nbrEntryConnections > 0 ) {
      m_entryConnections =
         theMap->getConnectionAllocator().getNewArray( nbrEntryConnections );
      for (uint32 i=0; i < nbrEntryConnections; ++i) {
         m_entryConnections[i] = theMap->createNewConnection(dataBuffer);
      }
   }
   
   m_nbrEntryConnections = nbrEntryConnections;
}

bool
OldNode::save(DataBuffer *dataBuffer) const
{
   uint8 bitField = 0;
   bitField = BitUtility::setBit(bitField, MAJOR_ROAD_BIT_NBR, m_majorRoad);
   bitField = BitUtility::setBit(bitField, ROAD_TOLL_BIT_NBR, m_roadToll);
   bitField |= ( uint32(m_entryRestrictions) << 2);
   // Not using number of lanes anymore. bitField |= (m_nbrLanes);
   DEBUG_DB(mc2dbg << "OldNode::save bitfield: 0x" << hex << uint32(bitField) 
                 << dec << endl);
   DEBUG_DB(mc2dbg << "   majorRoad="
                 << Utility::convertBoolToString(m_majorRoad) << endl);
   DEBUG_DB(mc2dbg << "   roadToll=" 
                 << Utility::convertBoolToString(m_roadToll) << endl);
   DEBUG_DB(mc2dbg << "   entryRest=" << (uint32) m_entryRestrictions << endl);
   // The next three bytes
   dataBuffer->writeNextByte(bitField);
   dataBuffer->writeNextByte(m_level);
   dataBuffer->writeNextByte(m_maximumWeight);
   dataBuffer->writeNextByte(m_maximumHeight);
   DEBUG_DB(mc2dbg << "   level=" << (uint32) m_level << endl);
   DEBUG_DB(mc2dbg << "   maxWidth=" << (uint32) m_maximumWeight << endl);
   DEBUG_DB(mc2dbg << "   maxHidth=" << (uint32) m_maximumHeight << endl);

   dataBuffer->writeNextByte(m_speedLimit);
   dataBuffer->writeNextByte(m_junctionType);
   dataBuffer->writeNextShort(m_nbrEntryConnections);
   DEBUG_DB(mc2dbg << "   speedLimit=" << (uint32) m_speedLimit << endl);
   DEBUG_DB(mc2dbg << "   junctionType=" << (uint32) m_junctionType << endl);
   DEBUG_DB(mc2dbg << "   nbrConnections=" << nbrConnections << endl);
   
   for (uint32 i=0; i<m_nbrEntryConnections; i++) {
      m_entryConnections[i]->save(dataBuffer);
   }
   
   return (true);
}


char*
OldNode::toString(char* buf, size_t maxLen)
{
   // Check the parameters
   if ((buf == NULL) || (maxLen < 2))
      return (NULL);

   sprintf(buf, "      nodeID=%d\n"
                "      entryRestrictions=%x\n"
                "      speedlimit=%d\n"
                "      max height=%d\n"
                "      max weight=%d\n",
                m_nodeID,
                m_entryRestrictions,
                m_speedLimit,
                m_maximumHeight,
                m_maximumWeight);
   
   return (buf);
}

uint32 
OldNode::getMemoryUsage(void) const 
{
   uint32 conSize = 0;
   for (uint32 i=0; i<getNbrConnections(); ++i) {
      conSize += getEntryConnection(i)->getMemoryUsage();
   }
   return sizeof(*this) + conSize;
}

void
OldNode::printNodeMifHeader(ofstream& mifFile, uint32 n)
{
   mifFile << "  " << n << "_NODE_ID integer(12,0)\r\n"
           << "  " << n << "_NODESPD integer(5,0)\r\n"
           << "  " << n << "_NBRLANES integer(3,0)\r\n"
           << "  " << n << "_LEVELS integer(3,0)\r\n"
           << "  " << n << "_JUNCTYPE char(25)\r\n"
           << "  " << n << "_ROADTOLL char(2)\r\n"
           << "  " << n << "_MAJORROAD char(2)\r\n"
           << "  " << n << "_MAXHEIGHT integer(5,0)\r\n"
           << "  " << n << "_MAXWEIGHT integer(5,0)\r\n"
           << "  " << n << "_NODEREST char(40)\r\n"
           << "  " << n << "_RULES char(100)\r\n";   
}

void
OldNode::printNodeMidData(ofstream& midFile) const
{
   midFile << m_nodeID << ",";

   midFile << int32(m_speedLimit);
   
   midFile << ","
           << "0" <<  "," // not using number of lanes.
           << int32(m_level) << ",";

   //Check junction type
   switch(m_junctionType){
      case ItemTypes::normalCrossing : {
         midFile << "\"normalCrossing\",";
      } break;
      case ItemTypes::bifurcation : {
         midFile << "\"bifurcation\",";
      } break;
      case ItemTypes::railwayCrossing  : {
         midFile << "\"railwayCrossing\",";
      } break;
      case ItemTypes::borderCrossing  : {
         midFile << "\"borderCrossing\",";
      } break;                                         
   }
   
   midFile << "\"" << Utility::convertBoolToString(m_roadToll) << "\","
           << "\"" << Utility::convertBoolToString(m_majorRoad) << "\","
           << int32(m_maximumHeight) << ","
           << int32(m_maximumWeight) << ",";
   
   //Check entry restrictions
   switch(m_entryRestrictions){
      case ItemTypes::noRestrictions : {
         midFile << "\"noRestrictions\"";
      } break;
      case ItemTypes::noThroughfare : {
         midFile << "\"noThroughfare\"";
      } break;
      case ItemTypes::noEntry : {
         midFile << "\"noEntry\"";
      } break;
      case ItemTypes::noWay : {
         midFile << "\"noWay\"";
      } break;
   }

   midFile << ",\"";

   //Print connections for current node.
   for (uint32 i=0; i<getNbrConnections(); i++) {
      if(i > 0)
         midFile << " ";
      getEntryConnection(i)->printMidFile(midFile);
       
   }
   
   midFile << "\"";
}

bool
OldNode::updateNodeAttributesFromNode(OldNode* otherOldNode, uint32 nodeNbr, 
                                      bool sameMap)
{
   bool retVal = false;

   if (otherOldNode == NULL)
      return retVal;
   
   // speed limit
   // level
   // height
   // weight
   // entryrestr
   // lanes
   // junction type
   // majorroad
   // tollroad
   // connections if items(nodes) originates from the same map
   
   if (m_speedLimit != otherOldNode->getSpeedLimit()) {
      m_speedLimit = otherOldNode->getSpeedLimit();
      mc2dbg4 << "    changing speed limit for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_level != otherOldNode->getLevel()) {
      m_level = otherOldNode->getLevel();
      mc2dbg4 << "    changing level for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_maximumHeight != otherOldNode->getMaximumHeight()) {
      m_maximumHeight = otherOldNode->getMaximumHeight();
      mc2dbg4 << "    changing height for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_maximumWeight != otherOldNode->getMaximumWeight()) {
      m_maximumWeight = otherOldNode->getMaximumWeight();
      mc2dbg4 << "    changing weight for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_entryRestrictions != otherOldNode->getEntryRestrictions()) {
      m_entryRestrictions = otherOldNode->getEntryRestrictions();
      mc2dbg4 << "    changing entry restrictions for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_junctionType != otherOldNode->getJunctionType()) {
      m_junctionType = otherOldNode->getJunctionType();
      mc2dbg4 << "    changing junction type for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_majorRoad != otherOldNode->isMajorRoad()) {
      m_majorRoad = otherOldNode->isMajorRoad();
      mc2dbg4 << "    changing majorRoad for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }
   
   if (m_roadToll != otherOldNode->hasRoadToll()) {
      m_roadToll = otherOldNode->hasRoadToll();
      mc2dbg4 << "    changing roadToll for routeable item " 
              << (m_nodeID &ITEMID_MASK) << " node " << nodeNbr << endl;
      retVal = true;
   }

   // OldConnections
   if ( sameMap ) {
      // The nodes (items) originates from the same map, which means 
      // that the fromOldNodeIds of the connections will match - 
      // check connection attributes...
      for (uint32 myc = 0; myc < getNbrConnections(); myc++) {
         OldConnection* myConn = getEntryConnection(myc);
         if (myConn != NULL) {
            // Find this conn in otherOldNode
            uint32 myFromOldNodeID = myConn->getConnectFromNode();
            OldConnection* otherConn = NULL;
            uint32 otherc = 0;
            while ((otherc < otherOldNode->getNbrConnections()) &&
                   (otherConn == NULL)) {
               uint32 otherFromOldNodeID = otherOldNode->
                  getEntryConnection(otherc)->getConnectFromNode();
               if (myFromOldNodeID == otherFromOldNodeID) {
                  otherConn = otherOldNode->getEntryConnection(otherc);
               } else {
                  otherc++;
               }
            }

            if (otherConn != NULL) {
               // We have the connections, update attributes!
               if (myConn->updateConnAttributesFromConn(otherConn))
                  retVal = true;
            }
            // else otherConn removed ?!?
         }
      }
   }

   return retVal;
}

bool  
OldNode::addConnection(OldConnection* newCon,
                    OldGenericMap& theMap ) 
{
   if (getNbrConnections() >= MAX_NBR_ENTRYCONNECTIONS) {
      return false;
   }
   
   // Make sure that we don't already have added a connection from
   // newCon->getFromNode()
   for (uint32 i=0; i<getNbrConnections(); ++i) {
      if (m_entryConnections[i]->getFromNode() == newCon->getFromNode()) {
         mc2dbg4 << "OldNode::addConnection Already added con from"
                 << newCon->getFromNode();
         return false;
      }
   }
   
   // "Add" one element by creating a new array
   uint32 tmpSize = m_nbrEntryConnections;
   m_entryConnections =
      theMap.getConnectionAllocator().addElement(m_entryConnections, 
                                                 newCon, 
                                                 tmpSize);
   m_nbrEntryConnections = tmpSize;
   return true;
}

bool  
OldNode::deleteConnectionsFrom(uint32 nodeID, OldGenericMap& theMap) 
{
   // Get index of connection from nodeID
   uint32 i = 0;
   while ( (i < getNbrConnections()) && 
           (getEntryConnection(i)->getFromNode() != nodeID) ) {
      ++i;
   }

   // Remove if found
   if (i < getNbrConnections()) {
      deleteConnection(i, theMap);
      return true;
   }

   return false;
}

void  
OldNode::deleteAllConnections(OldGenericMap& theMap)
{
   // Renove data from the map involving the connections of this node.
   uint32 toNodeID = m_nodeID;
   for ( int i = 0; i < m_nbrEntryConnections; i++ ) {
      uint32 fromNodeID = m_entryConnections[i]->getFromNode();
      theMap.m_connectingLanesTable.removeConnectingLanes(fromNodeID,
                                                          toNodeID);
   }

   // Not allowed to delete the actual connections since they
   // may be present in an allocator.
   m_nbrEntryConnections = 0;
}

void  
OldNode::deleteConnection( uint32 index, OldGenericMap& theMap )
{
   if ( index < m_nbrEntryConnections ) {
      // Remove data from the map
      uint32 toNodeID = m_nodeID;
      uint32 fromNodeID = m_entryConnections[index]->getFromNode();
      theMap.m_connectingLanesTable.removeConnectingLanes(fromNodeID,
                                                          toNodeID);

      // Remove the reference to this connection (not deleteing)
      for ( int i = index; i < m_nbrEntryConnections - 1; ++i ) {
         m_entryConnections[i] = m_entryConnections[i+1];
      }
      --m_nbrEntryConnections;
   }
}

OldConnection*  
OldNode::getEntryConnectionFrom( uint32 fromOldNode ) const
{
   uint32 i = 0; 
   OldConnection* conn = NULL;
   while ( ( i < getNbrConnections() ) && ( conn == NULL ) ) { 
      if ( m_entryConnections[ i ]->getConnectFromNode() == 
           fromOldNode ) {
         conn = m_entryConnections[ i ];
      } else {
         ++i;
      }
   }
   return conn;
}

void 
OldNode::setLane(const GMSLane& lane, uint32 laneIndex, OldGenericMap& theMap){

   mc2dbg8 << "OldNode::setLane called" << endl;
   
   vector<GMSLane>& lanes = theMap.m_nodeLaneVectors[m_nodeID];
   if (laneIndex+1 > lanes.size() ){
      lanes.resize(laneIndex+1);
   }
   lanes[laneIndex] = lane;

} // setLane


vector<GMSLane>
OldNode::getLanes(const OldGenericMap& theMap) const
{
   vector<GMSLane> result;
   ItemMap< vector<GMSLane> >::const_iterator laneVecIt = 
      theMap.m_nodeLaneVectors.find(this->getNodeID());
   if (laneVecIt != theMap.m_nodeLaneVectors.end() ){
      result = laneVecIt->second;
   }
   return result;
}

uint32
OldNode::getNbrLanes(const OldGenericMap& theMap) const
{
   return getLanes(theMap).size();
}

void 
OldNode::completeLaneInfoFromGdf( OldGenericMap& theMap)
{
   if (this->isNode0()){

      // Reorder the lanes vector backwards, to make sure that the first lane
      // in the vector is the lane mostly to the left.
      ItemMap< vector<GMSLane> >::iterator laneVecIt = 
         theMap.m_nodeLaneVectors.find(this->getNodeID());
      if ( laneVecIt != theMap.m_nodeLaneVectors.end() ){
         vector<GMSLane> tmpLanes = laneVecIt->second;
         vector<GMSLane>& lanes = laneVecIt->second;
         lanes.clear();
         for (int32 i=tmpLanes.size()-1; i>=0; i--){
            lanes.push_back(tmpLanes[i]);
         }
      }
   }
   else {

      // Move lane divider type one lane up to make sure that the divider value
      // of each lane represents the divider to the left of the lane.
      ItemMap< vector<GMSLane> >::iterator laneVecIt = 
         theMap.m_nodeLaneVectors.find(this->getNodeID());
      if ( laneVecIt != theMap.m_nodeLaneVectors.end() ){
         vector<GMSLane>& lanes = laneVecIt->second;
         for (int32 laneIdx=lanes.size()-1; laneIdx>=0; laneIdx--){
            if ( laneIdx-1 >= 0 ){
               lanes[laneIdx].setDividerType(lanes[laneIdx-
                                                   1].getDividerType());
            }
         }
         // invalid value
         lanes[0].setDividerType(GMSLane::nbrLaneDividerTypes);
      }
   }
   
  
   ItemMap< vector<GMSLane> >::iterator laneVecIt = 
         theMap.m_nodeLaneVectors.find(this->getNodeID());
   if ( laneVecIt != theMap.m_nodeLaneVectors.end() ){
      vector<GMSLane>& lanes = laneVecIt->second;
      for (uint32 laneIdx=0; laneIdx<lanes.size(); laneIdx++){
         lanes[laneIdx].handleVehicleRestr();
      }
   }



} // correcetLaneInfoFromGdf


void 
OldNode::addLaneConnectivity( OldGenericMap& theMap, uint32 fromNodeID, 
                              uint32 fromLaneIndex )
{
   theMap.m_connectingLanesTable.addConnectingLane(fromNodeID, 
                                                   this->getNodeID(), 
                                                   fromLaneIndex);
} // addLaneConnectivity

uint32
OldNode::getConnectedLanes( const OldGenericMap& theMap, uint32 fromNodeID ) const{
   
   return theMap.m_connectingLanesTable.getConnectingLanes(fromNodeID,
                                                           this->getNodeID());
}

bool
OldNode::addSignPostPart( OldGenericMap& theMap, uint32 fromNodeID,
                          uint32 signPostIndex,
                          uint32 signPostSetIdx, uint32 signPostPartIdx,
                          GMSSignPostElm& signPostElm )
{
   // Make sure a connection exists between from node and to node.
   if (this->getEntryConnectionFrom( fromNodeID ) != NULL){
      
      // Add the sign post part.
      bool result = 
         theMap.m_signPostTable.addSignPostElm( fromNodeID, 
                                                this->getNodeID(), // To ID
                                                signPostIndex,
                                                signPostSetIdx,
                                                signPostPartIdx, 
                                                signPostElm);
      return result;
   }
   else {
      mc2log << error  << "No connection between: " << fromNodeID << " and " 
             << this->getNodeID() << endl;
      return false;
   }
} // addSignPostPart

uint32 
OldNode::getNbrSignPosts(const OldGenericMap& theMap, 
                         uint32 fromNodeID) const {
   
   return theMap.m_signPostTable.getNbrSignPosts(fromNodeID, 
                                                 this->getNodeID());
} // getNbrSignPosts

bool
OldNode::signPostExists( const OldGenericMap& theMap, uint32 fromNodeID, 
                         const GMSSignPostElm& signPostElm ) const
{
   return theMap.m_signPostTable.signPostExists(fromNodeID, this->getNodeID(),
                                                signPostElm);

} // signPostExists


bool
OldNode::removeSignPost( OldGenericMap& theMap, uint32 fromNodeID,
                         MC2String signPostText )
{
   return theMap.m_signPostTable.removeSignPostElm(theMap,
                                                   fromNodeID, 
                                                   this->getNodeID(),
                                                   signPostText );
} // removeSignPost

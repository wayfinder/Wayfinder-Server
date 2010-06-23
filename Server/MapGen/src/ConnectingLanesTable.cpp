/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ConnectingLanesTable.h"

#include "DataBuffer.h"
#include "OldGenericMap.h"
#include "OldNode.h"

#include "NodeBits.h"

ConnectingLanesTable::ConnectingLanesTable(){

   m_connIt=m_connLanesTable.begin();
}

ConnectingLanesTable::connLanesByFromAndToNode_t
ConnectingLanesTable::getFirstConnectingLanes() const{
   m_connIt=m_connLanesTable.begin();
   if ( m_connIt != m_connLanesTable.end() ){
      connLanesTable_t::const_iterator resultIt = m_connIt;
      ++m_connIt;
      return *resultIt;
   }
   else {
      mc2dbg8 << "Breaking" << endl;
      return make_pair(make_pair(MAX_UINT32, MAX_UINT32), MAX_UINT32);

   }
}


ConnectingLanesTable::connLanesByFromAndToNode_t
ConnectingLanesTable::getNextConnectingLanes() const {
   if ( m_connIt != m_connLanesTable.end() ){
      connLanesTable_t::const_iterator resultIt = m_connIt;
      ++m_connIt;
      return *resultIt;
   }
   else {
      mc2dbg8 << "Breaking" << endl;
      return make_pair(make_pair(MAX_UINT32, MAX_UINT32), MAX_UINT32);

   }
}

void 
ConnectingLanesTable::save(DataBuffer& dataBuffer) const
{
   dataBuffer.alignToLong();
   
   // Size
   dataBuffer.writeNextLong(m_connLanesTable.size());

   // Table content
   for ( connLanesTable_t::const_iterator it = m_connLanesTable.begin();
         it != m_connLanesTable.end(); ++it) {
      dataBuffer.writeNextLong(it->first.first); // from node
      dataBuffer.writeNextLong(it->first.second); // to node
      dataBuffer.writeNextLong(it->second); // connecting lanes.
   }

} // save


void 
ConnectingLanesTable::load(DataBuffer& dataBuffer)
{
   dataBuffer.alignToLong();
   
   // Size
   uint32 tableSize = dataBuffer.readNextLong();

   // Table content
   uint32 fromNodeID;
   uint32 toNodeID;
   uint32 connLanes;
   for ( uint32 i=0; i<tableSize; i++){
      fromNodeID = dataBuffer.readNextLong();
      toNodeID = dataBuffer.readNextLong();
      connLanes = dataBuffer.readNextLong();
      m_connLanesTable.insert(make_pair( make_pair(fromNodeID, toNodeID),
                                         connLanes ));
      mc2dbg8 << "m_connLanesTable: " << fromNodeID << ":" << toNodeID <<endl;
   }
   m_connIt=m_connLanesTable.begin();
} // load

uint32 
ConnectingLanesTable::sizeInDataBuffer() const
{
   return
      4 + // size
      m_connLanesTable.size() * ( 4 +   // from node ID
                                  4 +   // to node ID
                                  4 );  // connecting lanes
} // sizeInDataBuffer

uint32 
ConnectingLanesTable::size() const
{
   return m_connLanesTable.size();

} // sizeInDataBuffer

bool
ConnectingLanesTable::removeConnectingLanes(uint32 fromNodeID, uint32 toNodeID)
{
   uint32 nbrRemoved = m_connLanesTable.erase(make_pair(fromNodeID,
                                                        toNodeID));
   return (nbrRemoved > 0);

} // removeConnectingLanes


bool
ConnectingLanesTable::laneConnects(uint32 index, uint32 laneBitField){
   if ( laneBitField == MAX_UINT32 ){
      return false;
   }
   return ( ( laneBitField & (1 << index) ) != 0 );
}

void
ConnectingLanesTable::addConnectingLane(uint32 fromNodeID, 
                                        uint32 toNodeID, 
                                        uint32 fromLaneIndex)
{
   connLanesTable_t::iterator insertIt
      = m_connLanesTable.find(make_pair(fromNodeID, toNodeID));
   if ( insertIt == m_connLanesTable.end() ){
      insertIt = 
         m_connLanesTable.insert(make_pair(make_pair(fromNodeID, toNodeID),
                                           0)).first;
   }
   uint32 allLaneIndexes = insertIt->second;
   
   mc2dbg8 << "addConnectingLane: " << allLaneIndexes;
   allLaneIndexes = allLaneIndexes | (1 << fromLaneIndex);
   mc2dbg8 << "->" << allLaneIndexes << endl;
   mc2dbg8 << fromNodeID << ":" << toNodeID << ":" 
           << allLaneIndexes << endl;

   insertIt->second = allLaneIndexes;

} // addConnectingLanes


void
ConnectingLanesTable::insertConnectingLanes(uint32 fromNodeID, 
                                            uint32 toNodeID, 
                                            uint32 fromLaneBitField)
{
   if ( m_connLanesTable.find(make_pair(fromNodeID, toNodeID)) != 
        m_connLanesTable.end() ){
      mc2log << error << "Inserting connecting lane data overwrite existing"
             << " data.";
      mc2dbg << fromNodeID << ":" << toNodeID << ":" 
             << fromLaneBitField << endl;
      MC2_ASSERT(false);
   }
   mc2dbg8 << "insert " << fromNodeID << ":" << toNodeID << ":" << fromLaneBitField << endl;
   m_connLanesTable[make_pair(fromNodeID, toNodeID)]=fromLaneBitField;

} // insertConnectingLanes


uint32
ConnectingLanesTable::getConnectingLanes(uint32 fromNodeID, 
                                         uint32 toNodeID) const{
   connLanesTable_t::const_iterator connIt = 
      m_connLanesTable.find(make_pair(fromNodeID, toNodeID));
   if ( connIt != m_connLanesTable.end() ){
      return connIt->second;
   }
   else {
      return MAX_UINT32;
   }
   
} // getConnectingLanes

void
ConnectingLanesTable::reorderConnectingLanes(const OldGenericMap& theMap,
                                             const set<uint32>& itemIDs){
   
   for ( connLanesTable_t::iterator connIt = m_connLanesTable.begin();
         connIt != m_connLanesTable.end(); ++connIt ){
      uint32 fromNodeID = connIt->first.first;
      uint32 toNodeID = connIt->first.second;
      if ( MapBits::isNode0(fromNodeID) &&
           itemIDs.find(fromNodeID & 0x7FFFFFFF) != itemIDs.end() ){
         // This one should be reordered because in GDF, lanes are ordered from
         // right to left and we want it from left to right.
         OldNode* fromNode = theMap.nodeLookup(fromNodeID);
         const vector<GMSLane>& lanes = fromNode->getLanes(theMap);
         uint32 nbrLanes = lanes.size();
         mc2dbg8 << "nbrLanes:" << nbrLanes << endl;

         // Reorder the bits.
         uint32 connectingLanesBits = connIt->second;
         uint32 tmpConnectingLanesBits = 0;
         for (int32 i=nbrLanes-1; i>=0; i--){
            if ( connectingLanesBits & (1 << i) ){
               mc2dbg8 << i << "->" << ((nbrLanes-1) - i) << endl;
               tmpConnectingLanesBits = 
                  tmpConnectingLanesBits | (1 << ( (nbrLanes-1)-i ) );
            }
         }
         mc2dbg << "reorderConnectingLanes " << fromNodeID << "->" 
                << toNodeID << ":" 
                << MapGenUtil::intToBitFieldStr(connectingLanesBits, 32) 
                << "->" 
                << MapGenUtil::intToBitFieldStr(tmpConnectingLanesBits, 32)
                << endl;
         connIt->second = tmpConnectingLanesBits;
      }
      else {
         // One reason for getting here: only ids from one GDF section is 
         // present in the itemIDs set, as while this connLaneTable has all
         // lanes in the complete mcm map
         mc2dbg << "NOT reorderConnectingLanes " << fromNodeID << "->" 
                << toNodeID << endl;
      } 

   }
} // reorderConnectingLanes
 
const ConnectingLanesTable&
ConnectingLanesTable::operator=(const ConnectingLanesTable& other)
{
   m_connLanesTable = other.m_connLanesTable;
   m_connIt = m_connLanesTable.begin();
   return *this;
} // operator=

const set<uint32> 
ConnectingLanesTable::getAddedItemIDs(const ConnectingLanesTable& 
                                      connLanesTable){
   set<uint32> itemIDs;
   
   for ( connLanesTable_t::const_iterator it = 
            connLanesTable.m_connLanesTable.begin();
         it != connLanesTable.m_connLanesTable.end(); ++it ){
      if ( m_connLanesTable.find(it->first) != m_connLanesTable.end() ){
         // This is not a new connecting lanes post.
      }
      else {
         uint32 fromNodeID = it->first.first;
         itemIDs.insert(fromNodeID & 0x7FFFFFFF); // mask to get item ID from 
         // node ID.
      }
   }
   return itemIDs;

} // getAddedItemIDs

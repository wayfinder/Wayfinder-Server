/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONNECTINGLANESTABLE_H
#define CONNECTINGLANESTABLE_H

#include "config.h"
#include <map>
#include <vector>
#include <set>

// Forward declarations.
class DataBuffer;
class OldGenericMap;

/**
 *   Used for storing the indexes of lanes connecting two nodes.
 *
 */
class ConnectingLanesTable {
 public:

   /// Default constructor.
   ConnectingLanesTable();

   /**
    * @return Returns the number of connecting lanes elements stored in this
    *         table.
    */
   uint32 size() const;


   /**
    * Key: From and to node ID
    * Value: Connecting lanes bitfield.
    */
   typedef pair< pair<uint32, uint32>, uint32>  connLanesByFromAndToNode_t;
   
   /*
    * @return Returns the next connecting lanes post from the table, or
    *         make_pair( make_pair(MAX_UINT32, MAX_UINT32), MAX_UINT32)
    *         if no next post exists.
    */
   connLanesByFromAndToNode_t getFirstConnectingLanes() const;
   connLanesByFromAndToNode_t getNextConnectingLanes() const;


   /**
    * Methods for saving and loading from and to DataBuffer.
    */
   //@{
   void save(DataBuffer& dataBuffer) const;
   void load(DataBuffer& dataBuffer);

   /**
    * Returns the maximum number of bytes this object will occupy when saved 
    * in  a data buffer.
    */
   uint32 sizeInDataBuffer() const;

   /**
    * Remove the connecting lane info for between fromNodeID och toNodeID
    *
    * @return True if anything was remvoed, false otherwise.
    */
   bool removeConnectingLanes(uint32 fromNodeID, uint32 toNodeID);
   
   /**
    * Add another connecting lane to the ones stored for the connection from
    * fromNode to toNode. This does not remove any existing connecting lanes,
    * and if the fromLaneIndex is already set as a connecting lane, the data
    * is not changed.
    */
   void addConnectingLane(uint32 fromNodeID, uint32 toNodeID, 
                          uint32 fromLaneIndex);

   /**
    * Sets the whole bitfield for the connection between toNodeID and 
    * fromNodeID. If this data already exists, it craches the application.
    *
    * @param fromLaneBitField The bitfield with the lanes of the from node that
    *          connects to the to node. Lowest bit is representing the left
    *          most lane. 
    */ 
   void insertConnectingLanes(uint32 fromNodeID, 
                              uint32 toNodeID, 
                              uint32 fromLaneBitField);

   /**
    *  @return Returns a bit field with the lanes of the from node that
    *          connects to the to node. Lowest bit is representing the left
    *          most lane. If no data about this connection
    *          exists, MAX_UINT32 is returned.
    */ 
   uint32 getConnectingLanes(uint32 fromNodeID, uint32 toNodeID) const;

   /**
    * Reorders connecting lanes and switches order of the lane data
    * of nodes indicating that the direction of the connection is in 
    * negative direction compared to the direction of the SSI.
    */
   void reorderConnectingLanes(const OldGenericMap& theMap,
                               const set<uint32>& itemIDs );


   /**
    * Tells if a specific lane is set as connecting in a lane connection 
    * bit field.
    *
    * @param index The index of the lane to check.
    * @param laneBitField The bit field to check in.
    *
    * @return Returns true if the bit corresponding to index is set to 1, 
    *         otherwise false. Always retruns false if the laneBitField is 
    *         MAX_UINT32.
    */
   static bool laneConnects(uint32 index, uint32 laneBitField);
   
   /**
    * Assignment operator
    */
   const ConnectingLanesTable& operator=( const ConnectingLanesTable& other );

   /**
    * @returns The item IDs of connLanesTalble that are not present in this
    *          connecting lanes table. 
    */
   const set<uint32> 
      getAddedItemIDs(const ConnectingLanesTable& connLanesTable);


 private:
 // Member types
   /**
    * Key:   from node ID, to node ID
    * Value: From node lane indexes of lanes connecting to the to node.
    */
   typedef map< pair<uint32, uint32>, uint32 > connLanesTable_t;

 // Member methods
   
      

 // Member variables
   
   /**
    * The connecting lanes of connections, defned by from and to node ID.
    */
   connLanesTable_t m_connLanesTable;
   
   mutable connLanesTable_t::const_iterator m_connIt;

}; // ConnectingLanesTable

#endif // CONNECTINGLANESTABLE_H


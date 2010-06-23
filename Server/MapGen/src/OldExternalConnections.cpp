/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OldExternalConnections.h"
#include "OldGenericMap.h"
#include "MapBits.h"

// ==================================================================
//                                                 ExternalConnection
// ==================================================================
OldBoundrySegment::OldBoundrySegment() 
   : VectorElement()
{
   m_routeableItemID = INVALID_ITEMID;
   m_closeNode = noNodeClose;
}

OldBoundrySegment::OldBoundrySegment(uint32 routeableItemID,
                               closeNode_t closeNodeValue)
   : VectorElement()
{
   if ((routeableItemID & 0x80000000) != 0) {
      mc2log << error
             << "ERROR in OldBoundrySegment() routeableItemID is nodeID" 
             << endl;
   }
   m_routeableItemID = routeableItemID & ITEMID_MASK;
   m_closeNode = closeNodeValue;
}

OldBoundrySegment::~OldBoundrySegment()
{
   // All connections are allocated in the allocator in the map
}

OldBoundrySegment::OldBoundrySegment(DataBuffer* dataBuffer, OldGenericMap* theMap)
   : VectorElement()
{
   m_routeableItemID = dataBuffer->readNextLong();

   uint32 nbrConnectionsToNode0 = dataBuffer->readNextByte();
   uint32 nbrConnectionsToNode1 = dataBuffer->readNextByte();
   m_closeNode = closeNode_t (dataBuffer->readNextByte());
   dataBuffer->readNextByte();   // PAD

   uint32 mapSet = MapBits::getMapSetFromMapID(theMap->getMapID());
   // Read connections to node0
   m_connectionsToNode0.reserve(nbrConnectionsToNode0);
   for (uint32 i=0; i<nbrConnectionsToNode0; ++i) {
      uint32 fromMapID = dataBuffer->readNextLong();
      fromMapID = MapBits::getMapIDWithMapSet(fromMapID, mapSet);
      fromMapID = MapBits::getMapIDWithMapSet(fromMapID, mapSet);
      OldConnection* con = theMap->createNewConnection(dataBuffer);
      m_connectionsToNode0.push_back(make_pair(fromMapID, con));
   }

   // Read connections to node1
   m_connectionsToNode1.reserve(nbrConnectionsToNode1);
   for (uint32 i=0; i<nbrConnectionsToNode1; ++i) {
      uint32 fromMapID = dataBuffer->readNextLong();
      fromMapID = MapBits::getMapIDWithMapSet(fromMapID, mapSet);
      OldConnection* con = theMap->createNewConnection(dataBuffer);
      m_connectionsToNode1.push_back(make_pair(fromMapID, con));
   }
}

void
OldBoundrySegment::save(DataBuffer* dataBuffer)
{
   dataBuffer->writeNextLong(m_routeableItemID);

   dataBuffer->writeNextByte((byte) m_connectionsToNode0.size());
   dataBuffer->writeNextByte((byte) m_connectionsToNode1.size());
   dataBuffer->writeNextByte((byte) m_closeNode);
   dataBuffer->writeNextByte(0); // PAD

   // Save connections to node0
   for (externalConnectionIt it=m_connectionsToNode0.begin(); 
        it != m_connectionsToNode0.end(); ++it) {
      // Save fromMapID and the connection
      dataBuffer->writeNextLong(it->first);
      it->second->save(dataBuffer);
   }

   // Save connections to node1
   for (externalConnectionIt it=m_connectionsToNode1.begin(); 
        it != m_connectionsToNode1.end(); ++it) {
      // Save fromMapID and the connection
      dataBuffer->writeNextLong(it->first);
      it->second->save(dataBuffer);
   }
   mc2dbg4 << "Saved boundry segment with " 
           << m_connectionsToNode0.size() + m_connectionsToNode1.size()
           << " external connections" << endl;
}

bool
OldBoundrySegment::addConnection(OldConnection* con, uint32 fromMapID,
                                 bool closestToBoundry)
{
   externalConnection_t extCon = make_pair(fromMapID, con);
   bool added = false;
   
   if (!closestToBoundry) {
      if (m_closeNode == node0close) {
         if (getExternalConnectionIdx(0, fromMapID, 
                             con->getConnectFromNode()) == MAX_UINT32) {
            m_connectionsToNode0.push_back(extCon);
            added = true;
         }
      } else if (m_closeNode == node1close) {
         if (getExternalConnectionIdx(1, fromMapID, 
                             con->getConnectFromNode()) == MAX_UINT32) {
            m_connectionsToNode1.push_back(extCon);
            added = true;
         }
      } 
   } else {
      if (m_closeNode == node0close) {
         if (getExternalConnectionIdx(1, fromMapID, 
                             con->getConnectFromNode()) == MAX_UINT32) {
            m_connectionsToNode1.push_back(extCon);
            added = true;
         }
      } else if (m_closeNode == node1close) {
         if (getExternalConnectionIdx(0, fromMapID, 
                             con->getConnectFromNode()) == MAX_UINT32) {
            m_connectionsToNode0.push_back(extCon);
            added = true;
         }
      } 
   }

   if (!added)
      mc2dbg << "Failed to add external connection" << endl;
   
   // Return true if the connection is added (newIndex < MAX_UINT32)
   return added;
}

bool 
OldBoundrySegment::addConnection(OldConnection* con, 
                              uint32 fromMapID,
                              uint32 toItemID) 
{
   if ( (toItemID & ITEMID_MASK) != m_routeableItemID) {
      mc2log << error
             << "OldBoundrySegment::addCOnnection returning false!" << endl;
      return (false);
   } else {
      uint32 added = false;
      if ( (toItemID & 0x80000000) == 0x80000000) {
         // Node 1
         if (getExternalConnectionIdx(1, fromMapID, 
                             con->getConnectFromNode()) < MAX_UINT32) {
            externalConnection_t extCon = make_pair(fromMapID, con);
            m_connectionsToNode1.push_back(extCon);
            added = true;
         }
      } else {
         // Node 0
         if (getExternalConnectionIdx(0, fromMapID, 
                             con->getConnectFromNode()) < MAX_UINT32) {
            externalConnection_t extCon = make_pair(fromMapID, con);
            m_connectionsToNode0.push_back(extCon);
            added = true;
         }
      }
      // Return true if the connection is added (newIndex < MAX_UINT32)
      return added;
   }
}


void
OldBoundrySegment::deleteAllConnections()
{
   // All connections are allocated in allocator in the map
   m_connectionsToNode0.clear();
   m_connectionsToNode1.clear();
}

uint32 
OldBoundrySegment::getMemoryUsage() const {
   uint32 size = sizeof(OldBoundrySegment);
   for( uint32 i=0; i < m_connectionsToNode0.size(); i++ )
      size += m_connectionsToNode0[i].second->getMemoryUsage();

   for( uint32 i=0; i < m_connectionsToNode1.size(); i++ )
      size += m_connectionsToNode1[i].second->getMemoryUsage();
   
   return size;
}

uint32 
OldBoundrySegment::getExternalConnectionIdx( byte node, 
                                          uint32 fromMapID, 
                                          uint32 fromItemID) const
{
   MC2_ASSERT( (node == 0) || (node == 1));

   bool found = false;
   uint32 nbrCon = getNbrConnectionsToNode(node);
   uint32 foundIdx = 0;
   while ( (foundIdx < nbrCon) && (!found) ) {
      if ( (getFromMapIDToNode(node, foundIdx) == fromMapID) &&
           (getConnectToNode(node, foundIdx)->getConnectFromNode() 
               == fromItemID)) {
         found = true;
      } else {
         ++foundIdx;
      }
   }

   // Return
   if (foundIdx < nbrCon)
      return foundIdx;
   return MAX_UINT32;
}



// =======================================================================
//                                                 OldBoundrySegmentsVector =

OldBoundrySegmentsVector::OldBoundrySegmentsVector() 
   : ObjVector(0, 8)
{

}

OldBoundrySegmentsVector::OldBoundrySegmentsVector(DataBuffer* dataBuffer,
                                             OldGenericMap* theMap) 
   : ObjVector()
{
   uint32 nbrItems = dataBuffer->readNextLong();
   setAllocSize(nbrItems);

   for (uint32 i=0; i<nbrItems; i++) {
      addLast(new OldBoundrySegment(dataBuffer, theMap));
   }
   
}

OldBoundrySegmentsVector::~OldBoundrySegmentsVector()
{
   deleteAllObjs();
}

void
OldBoundrySegmentsVector::save(DataBuffer* dataBuffer)
{
   // Make sure this vector is always sorted when saved.
   sort();

   dataBuffer->writeNextLong(getSize());
   for (uint32 i=0; i<getSize(); i++) {
      ((OldBoundrySegment*) getElementAt(i) )->save(dataBuffer);
   }
}

bool
OldBoundrySegmentsVector::addBoundrySegment(
                     uint32 riID, 
                     OldBoundrySegment::closeNode_t closeNodeValue)
{
   bool retVal = true;
   OldBoundrySegment* theNewOldBoundrySegment = 
         new OldBoundrySegment(riID, closeNodeValue);
   if (addLastIfUnique(theNewOldBoundrySegment) == MAX_UINT32) {
      // this ssi already present in the vector,
      retVal = false;
      
      // delete the new boundrySegment
      delete theNewOldBoundrySegment;
   }

   return (retVal);
}

void 
OldBoundrySegmentsVector::addConnection(uint32 fromMapID,
                                     uint32 fromNodeID,
                                     uint32 toNodeID)
{
   OldBoundrySegment* b = getBoundrySegment(toNodeID & 0x7fffffff); 
   if (b != NULL) {
      OldConnection* c = new OldConnection( fromNodeID );
      b->addConnection(c, fromMapID, toNodeID);
   } else {
      DEBUG1(
         mc2dbg << "addConnection: TO " << toNodeID << " b == NULL" << endl;
         if (getBoundrySegment(fromNodeID & 0x7fffffff) == NULL)
            mc2dbg << "   FROM " << fromNodeID << " b == NULL" << endl;
         else
            mc2dbg << "   but FROM " << fromNodeID << " b != NULL" << endl;
      );
   }
}

void
OldBoundrySegmentsVector::dump(uint32 toMapID)
{
   cout << "OldBoundrySegmentsVector::dump(" << toMapID << ")" << endl;
   cout << "   Nbr boundry segments = " << getSize() << endl;
   cout << "   Total nbr connections = " << getTotNbrConnections() << endl;

   // Count the number of segments with external connecitons
   uint32 nbrSegWithConnection = 0;
   for (uint32 i=0; i<getSize(); i++) {
      OldBoundrySegment* seg = (OldBoundrySegment*) m_buf[i];
      if ( (seg->getNbrConnectionsToNode(0) > 0) ||
           (seg->getNbrConnectionsToNode(1) > 0)) {
         nbrSegWithConnection++;
      }
   }
   cout << "   Nbr boundry segments with connections = " 
        << nbrSegWithConnection << endl;

   char s[16];
   if (toMapID < MAX_UINT32) {
      sprintf(s, "%d.%c", toMapID, '\0');
   } else {
      sprintf(s, "%c", '\0');
   }

   // "parameter" that tells if segments without connections should be 
   // printed or not.
   bool printEmptySegments = true;
   
   OldBoundrySegment* seg;
   int nodeNumber;
   for (uint32 i=0; i<getSize(); i++) {
      seg = (OldBoundrySegment*) m_buf[i];

      const char* closestNode = NULL;
      OldBoundrySegment::closeNode_t closeNodeValue = seg->getCloseNodeValue();
      if (closeNodeValue == OldBoundrySegment::node0close)
         closestNode = "(0 closest)";
      else if (closeNodeValue == OldBoundrySegment::node1close)
         closestNode = "(1 closest)";

      // To node 0
      uint32 j;
      for ( j=0; j<seg->getNbrConnectionsToNode(0); j++) {
         if ( (seg->getConnectToNode0(j)->getConnectFromNode() & 0x80000000) 
              == 0x80000000) {
            nodeNumber = 1;
         } else {
            nodeNumber = 0;
         }
         cout << "      " << dec << i << ". From: " << hex
              << seg->getFromMapIDToNode0(j) << "."
              << seg->getConnectToNode0(j)->getConnectFromNode() 
              << "[" << nodeNumber << "] = (" << dec
              << seg->getConnectToNode0(j)->getConnectFromNode()
              << "d) to " << hex << s // including the period
              << seg->getConnectRouteableItemID() << "[0] = (" 
              << dec << seg->getConnectRouteableItemID() << "d)" 
              << closestNode << endl;
      }

      // To node 1
      for (j=0; j<seg->getNbrConnectionsToNode(1); j++) {
         if ( (seg->getConnectToNode1(j)->getConnectFromNode() & 0x80000000) 
              == 0x80000000) {
            nodeNumber = 1;
         } else {
            nodeNumber = 0;
         }
         cout << "      " << dec << i << ". From: " << hex
              << seg->getFromMapIDToNode1(j) << "."
              << seg->getConnectToNode1(j)->getConnectFromNode() 
              << "[" << nodeNumber << "] = (" << dec
              << seg->getConnectToNode1(j)->getConnectFromNode()
              << "d) to " << hex << s // including the period
              << seg->getConnectRouteableItemID() << "[1] = (" 
              << dec << seg->getConnectRouteableItemID() << "d)" 
              << closestNode << endl;
      }
      if ( (printEmptySegments) &&
           (seg->getNbrConnectionsToNode(0) == 0) &&
           (seg->getNbrConnectionsToNode(1) == 0) ) {
         cout << "      " << i << ". No connections to " << s
              << seg->getConnectRouteableItemID() << " = 0x" << hex
              << seg->getConnectRouteableItemID() << dec 
              << closestNode << endl;
      }

   }
}

void
OldBoundrySegmentsVector::getBoundrySegments(uint32 mapID,
                                          uint32 nodeID, 
                                          ObjVector* result) const
{
   MC2_ASSERT (result != NULL);
   
   for(uint32 i = 0; i < getSize(); i++ ) {
      OldBoundrySegment* seg = (OldBoundrySegment*)getElementAt(i);
      for (int n = 0; n < 2; n++) {
         for (uint j=0; j < seg->getNbrConnectionsToNode(n); j++ ) {
            uint32 fromMapID = seg->getFromMapIDToNode(n, j);
            OldConnection* connection = seg->getConnectToNode(n, j);
            if ( (fromMapID == mapID) &&
                 (connection->getConnectFromNode() == nodeID))
               result->addLastIfUnique(seg);
         }
      }
   }
   mc2dbg2 << "getBoundrySegments, result->getSize=" 
           << result->getSize() << endl;
}


OldConnection*
OldBoundrySegmentsVector::getBoundryConnection(uint32 itemID,
                                            uint32 mapID) const
{
   // Mask of the high bit.
   uint32 nodeID = itemID & 0x7fffffff;
   for(uint32 i = 0; i < getSize(); i++ ) {
      OldBoundrySegment* seg = (OldBoundrySegment*)getElementAt(i);
      for (byte n=0; n<2; ++n) {
         for (uint j=0; j < seg->getNbrConnectionsToNode(0); j++ ) {
            uint32 fromMapID = seg->getFromMapIDToNode(n, j);
            OldConnection* connection = seg->getConnectToNode(n, j);
            if ( (fromMapID == mapID) &&
                 (connection->getConnectFromNode() == nodeID)) {
               return connection;
            }
         }
      }
   }
   return NULL;
}

uint32
OldBoundrySegmentsVector::getTotNbrConnections()
{
   uint32 totNbrCon = 0;
   for(uint32 i = 0; i < getSize(); i++ ) {
      OldBoundrySegment* seg = (OldBoundrySegment*)getElementAt(i);
      totNbrCon += seg->getNbrConnectionsToNode(0);
      totNbrCon += seg->getNbrConnectionsToNode(1);
   }
   return (totNbrCon);
}
 
void 
OldBoundrySegmentsVector::deleteAllConnections()
{
   for(uint32 i = 0; i < getSize(); i++ ) {
      static_cast<OldBoundrySegment*>(getElementAt(i))->deleteAllConnections();
   }
}
  

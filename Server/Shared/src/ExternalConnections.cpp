/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExternalConnections.h"
#include "GenericMap.h"
#include "MapBits.h"
#include "STLUtility.h"

// ==================================================================
//                                                 ExternalConnection
// ==================================================================
BoundrySegment::BoundrySegment() 
{
   m_routeableItemID = INVALID_ITEMID;
   m_closeNode = noNodeClose;
}

BoundrySegment::BoundrySegment(uint32 routeableItemID,
                               closeNode_t closeNodeValue)
{
   if ((routeableItemID & 0x80000000) != 0) {
      mc2log << error
             << "ERROR in BoundrySegment() routeableItemID is nodeID" 
             << endl;
   }
   m_routeableItemID = routeableItemID & ITEMID_MASK;
   m_closeNode = closeNodeValue;
}

BoundrySegment::~BoundrySegment()
{
   // All connections are allocated in the allocator in the map
}


BoundrySegment::BoundrySegment(DataBuffer* dataBuffer, GenericMap* theMap)
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
      Connection* con = theMap->createNewConnection(dataBuffer);
      m_connectionsToNode0.push_back(make_pair(fromMapID, con));
   }

   // Read connections to node1
   m_connectionsToNode1.reserve(nbrConnectionsToNode1);
   for (uint32 i=0; i<nbrConnectionsToNode1; ++i) {
      uint32 fromMapID = dataBuffer->readNextLong();
      fromMapID = MapBits::getMapIDWithMapSet(fromMapID, mapSet);
      Connection* con = theMap->createNewConnection(dataBuffer);
      m_connectionsToNode1.push_back(make_pair(fromMapID, con));
   }
}

void
BoundrySegment::save( DataBuffer& dataBuffer, const GenericMap& map )
{
   dataBuffer.writeNextLong(m_routeableItemID);

   dataBuffer.writeNextByte((byte) m_connectionsToNode0.size());
   dataBuffer.writeNextByte((byte) m_connectionsToNode1.size());
   dataBuffer.writeNextByte((byte) m_closeNode);
   dataBuffer.writeNextByte(0); // PAD

   // Save connections to node0
   for (externalConnectionIt it=m_connectionsToNode0.begin(); 
        it != m_connectionsToNode0.end(); ++it) {
      // Save fromMapID and the connection
      dataBuffer.writeNextLong(it->first);
      it->second->save( dataBuffer );
   }

   // Save connections to node1
   for (externalConnectionIt it=m_connectionsToNode1.begin(); 
        it != m_connectionsToNode1.end(); ++it) {
      // Save fromMapID and the connection
      dataBuffer.writeNextLong(it->first);
      it->second->save( dataBuffer );
   }
   mc2dbg4 << "Saved boundry segment with " 
           << m_connectionsToNode0.size() + m_connectionsToNode1.size()
           << " external connections" << endl;
}

void
BoundrySegment::deleteAllConnections()
{
   // All connections are allocated in allocator in the map
   m_connectionsToNode0.clear();
   m_connectionsToNode1.clear();
}

uint32 
BoundrySegment::getMemoryUsage() const {
   uint32 size = sizeof(BoundrySegment);
   for( uint32 i=0; i < m_connectionsToNode0.size(); i++ )
      size += m_connectionsToNode0[i].second->getMemoryUsage();

   for( uint32 i=0; i < m_connectionsToNode1.size(); i++ )
      size += m_connectionsToNode1[i].second->getMemoryUsage();
   
   return size;
}

uint32 
BoundrySegment::getExternalConnectionIdx( byte node, 
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
//                                                 BoundrySegmentsVector =

BoundrySegmentsVector::BoundrySegmentsVector() 
{

}

BoundrySegmentsVector::BoundrySegmentsVector(DataBuffer* dataBuffer,
                                             GenericMap* theMap) 
{
   uint32 nbrItems = dataBuffer->readNextLong();
   m_boundrySegments.reserve(nbrItems);

   for (uint32 i=0; i<nbrItems; i++) {
      m_boundrySegments.push_back(new BoundrySegment(dataBuffer, theMap));
   }
   
}

BoundrySegmentsVector::~BoundrySegmentsVector()
{
   STLUtility::deleteValues( m_boundrySegments );
}

void
BoundrySegmentsVector::save( DataBuffer& dataBuffer, const GenericMap& map )
{
   dataBuffer.writeNextLong(m_boundrySegments.size());
   for(BoundrySegmentVector::iterator it = m_boundrySegments.begin();
       it != m_boundrySegments.end();
       ++it) {
      (*it)->save ( dataBuffer, map );
   }
}

bool
BoundrySegmentsVector::addBoundrySegment(
                     uint32 riID, 
                     BoundrySegment::closeNode_t closeNodeValue)
{
   bool retVal = true;
   BoundrySegment* theNewBoundrySegment = 
         new BoundrySegment(riID, closeNodeValue);

    if( !addLastIfUnique(&m_boundrySegments, theNewBoundrySegment ) ) {
      // this ssi already present in the vector,
      retVal = false;
      // delete the new boundrySegment
      delete theNewBoundrySegment;
   }

   return (retVal);
}
/*
void 
BoundrySegmentsVector::addConnection(uint32 fromMapID,
                                     uint32 fromNodeID,
                                     uint32 toNodeID)
{
   BoundrySegment* b = getBoundrySegment(toNodeID & 0x7fffffff); 
   if (b != NULL) {
      Connection* c = new Connection( fromNodeID );
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
*/

void
BoundrySegmentsVector::getBoundrySegments(uint32 mapID,
                                          uint32 nodeID, 
                                          BoundrySegmentVector* result) const
{
   MC2_ASSERT (result != NULL);
   
   for(uint32 i = 0; i < m_boundrySegments.size(); i++ ) {
      BoundrySegment* seg = m_boundrySegments[ i ];
      for (int n = 0; n < 2; n++) {
         for (uint j=0; j < seg->getNbrConnectionsToNode(n); j++ ) {
            uint32 fromMapID = seg->getFromMapIDToNode(n, j);
            Connection* connection = seg->getConnectToNode(n, j);
            if ( (fromMapID == mapID) &&
                 (connection->getConnectFromNode() == nodeID)) {
               addLastIfUnique(result, seg);
            }
         }
      }
   }
   mc2dbg2 << "getBoundrySegments, result->getSize=" 
           << result->size() << endl;
}


Connection*
BoundrySegmentsVector::getBoundryConnection(uint32 itemID,
                                            uint32 mapID) const
{
   // Mask of the high bit.
   uint32 nodeID = itemID & 0x7fffffff;
   for(uint32 i = 0; i < m_boundrySegments.size(); i++ ) {
      BoundrySegment* seg = m_boundrySegments[ i ];
      for (byte n=0; n<2; ++n) {
         for (uint j=0; j < seg->getNbrConnectionsToNode(0); j++ ) {
            uint32 fromMapID = seg->getFromMapIDToNode(n, j);
            Connection* connection = seg->getConnectToNode(n, j);
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
BoundrySegmentsVector::getTotNbrConnections()
{
   uint32 totNbrCon = 0;
   for(uint32 i = 0; i < getSize(); i++ ) {
      BoundrySegment* seg = (BoundrySegment*)getElementAt(i);
      totNbrCon += seg->getNbrConnectionsToNode(0);
      totNbrCon += seg->getNbrConnectionsToNode(1);
   }
   return (totNbrCon);
}
 
void 
BoundrySegmentsVector::deleteAllConnections()
{
   for(uint32 i = 0; i < getSize(); i++ ) {
      static_cast<BoundrySegment*>(getElementAt(i))->deleteAllConnections();
   }
}
  

bool 
BoundrySegmentsVector::addLastIfUnique( BoundrySegmentVector* vec, BoundrySegment* seg ) const{

   BoundrySegmentVector::iterator it = find_if(vec->begin(), 
                                               vec->end(), 
           STLUtility::RefEqualCmp< BoundrySegment >( *seg ) );

   if( it !=  vec->end() ) {
      // this ssi already present in the vector,
      return false;
   } else {
      vec->push_back( seg );
      return true;
   }
}

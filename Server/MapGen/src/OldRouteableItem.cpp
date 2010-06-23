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
#include "OldRouteableItem.h"

#include "MC2String.h"
#include "StringUtility.h"
#include "OldGenericMap.h"
#include "AllocatorTemplate.h"
#include "GfxUtility.h"
#include "GfxData.h"
#include "OldNode.h"

OldRouteableItem::OldRouteableItem(ItemTypes::itemType type,
                             uint32 id)
   : OldItem(type, id) 
{
   DEBUG_DB(mc2dbg << "OldRouteableItem created" << endl);
   m_node0 = NULL;
   m_node1 = NULL;
}

OldRouteableItem::~OldRouteableItem()
{
   MINFO("~OldRouteableItem");
   DEBUG_DB(mc2dbg << "OldRouteableItem destructed" << endl);
   // Do _not_ delete nodes since they are allocated in an array in 
   // the map.
   // XXX: But you do not allways have a map so it is WRONG to assume you
   // have one!!!! Call deleteMapData, and delete gfx, before destructor.
}

bool
OldRouteableItem::save(DataBuffer *dataBuffer) const
{
   DEBUG_DB(mc2dbg << "      OldRouteableItem::save()" << endl;)

   OldItem::save(dataBuffer);

   m_node0->save(dataBuffer);
   m_node1->save(dataBuffer);

   return (true);
}

OldNode* 
OldRouteableItem::getNewNode(OldGenericMap* theMap) 
{
   return (static_cast<MC2Allocator<OldNode>*>
                      (theMap->m_nodeAllocator)->getNextObject());
}

OldNode*
OldRouteableItem::createNewNode(DataBuffer* dataBuffer, uint32 nodeID)
{
   return (new OldNode(dataBuffer, nodeID));
}

double
OldRouteableItem::sqrDistanceFromLast(OldRouteableItem* routeableItem)
{
    GfxData *g1 = getGfxData();
    GfxData *g2 = routeableItem->getGfxData();
    if ( g1 == NULL || g2 == NULL )
        return -1.0;

    uint32 lastIdxG1;
    MC2_ASSERT(g1->getNbrPolygons() == 1);
    MC2_ASSERT(g2->getNbrPolygons() == 1);
    // XXX Why not if 
    // (g1->getNbrCoordinates(0) > 1) lastIdxG1=g1->getLastLat(0) 
    // else return -1
    if (g1->getNbrCoordinates(0) > 2) 
      lastIdxG1 = g1->getNbrCoordinates(0) - 1;
    else if (g1->getNbrCoordinates(0) > 0)
      lastIdxG1 = 0;
    else
      return (-1.0);
    
    uint32 lastIdxG2;
    if (g2->getNbrCoordinates(0) > 2) 
      lastIdxG2 = g2->getNbrCoordinates(0) - 1;
    else if (g2->getNbrCoordinates(0) > 0)
      lastIdxG2 = 0;
    else
      return (-1.0);
    
    return MIN(
         GfxUtility::squareP2Pdistance_linear(
                           g1->getLat(0,lastIdxG1),
                           g1->getLon(0,lastIdxG1),
                           g2->getLat(0,0),
                           g2->getLon(0,0)),
         GfxUtility::squareP2Pdistance_linear(
                           g1->getLat(0,lastIdxG1),
                           g1->getLon(0,lastIdxG1),
                           g2->getLat(0,lastIdxG2),
                           g2->getLon(0,lastIdxG2)));
}


double
OldRouteableItem::sqrDistanceFromFirst(OldRouteableItem* routeableItem)
{
   GfxData *g1 = getGfxData();
   GfxData *g2 = routeableItem->getGfxData();
   if ( g1 == NULL || g2 == NULL )
      return -1.0;

   MC2_ASSERT(g1->getNbrPolygons() == 1);
   MC2_ASSERT(g2->getNbrPolygons() == 1);
   uint32 lastIdx = g2->getNbrCoordinates(0) - 1;
   if (g2->getNbrCoordinates(0) > 2) 
      lastIdx = g2->getNbrCoordinates(0) - 1;
   else if (g2->getNbrCoordinates(0) > 0)
      lastIdx = 0;
   else
      return (-1.0);
    
   return MIN(GfxUtility::squareP2Pdistance_linear(g1->getLat(0,0),
                                                 g1->getLon(0,0),
                                                 g2->getLat(0,0),
                                                 g2->getLon(0,0)),
              GfxUtility::squareP2Pdistance_linear(g1->getLat(0,0),
                                                 g1->getLon(0,0),
                                                 g2->getLat(0,lastIdx),
                                                 g2->getLon(0,lastIdx)));
}


double
OldRouteableItem::sqrFlightDistance()
{
   GfxData *g = getGfxData();
   MC2_ASSERT(g->getNbrPolygons() == 1);
   if ((g != NULL) && (g->getNbrCoordinates(0) > 3)) {
      uint32 lastIdx = g->getNbrCoordinates(0) - 1;
      return GfxUtility::squareP2Pdistance_linear(
                           g->getLat(0,0), g->getLon(0,0),
                           g->getLat(0,lastIdx), g->getLon(0,lastIdx));
   } else {
      return (0);
   }
}

char* OldRouteableItem::toString()
{
   char tmpStr[ITEM_AS_STRING_LENGTH];

   const uint32 MAX_NODE_SIZE = 1024;
   char m_node0Str[MAX_NODE_SIZE];
   if (m_node0 != NULL)
      m_node0->toString(m_node0Str, MAX_NODE_SIZE);
   else 
      m_node0Str[0] = '\0';
   char m_node1Str[MAX_NODE_SIZE];
   if (m_node1 != NULL)
      m_node1->toString(m_node1Str, MAX_NODE_SIZE);
   else 
      m_node1Str[0] = '\0';
    
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString,   "%s"
                           "   node 0 :\n%s"
                           "   node 1 :\n%s",
                           tmpStr,
                           m_node0Str,
                           m_node1Str);
   return itemAsString;
}


bool
OldRouteableItem::isConnectedTo(OldRouteableItem* other, int inNodeNbr)
{
   int start;
   int stop;
   
   if (inNodeNbr == 0) 
      start = stop = 0;
   else if (inNodeNbr == 1)
      start = stop = 1;
   else {
      start = 0;
      stop  = 1;
   }
   stop++;
      
   for ( int nodeNbr=start; nodeNbr < stop; nodeNbr++) {
      OldNode* node = this->getNode(nodeNbr);
      for( uint16 i=0; i < node->getNbrConnections(); i++) {
         OldConnection* connection = node->getEntryConnection(i);
         uint32 nodeIdx = connection->getConnectFromNode();
         for ( int j=0; j < 2; j++) {
            if ( nodeIdx == (other->getNode(j)->getNodeID()) )
               return true;
         } // next j
      } // next i
   } // next nodeNbr

   // No connection found!
   return false;
}

bool 
OldRouteableItem::deleteConnectionsFrom(uint32 nodeID, 
                                        OldGenericMap& theMap) 
{
   bool b1 = m_node0->deleteConnectionsFrom(nodeID, theMap);
   bool b2 = m_node1->deleteConnectionsFrom(nodeID, theMap);
   return (b1 || b2);
}

uint32 
OldRouteableItem::getMemoryUsage() const 
{
   uint32 totalSize = OldItem::getMemoryUsage()
      - sizeof(OldItem) + sizeof(OldRouteableItem);
   
   if ( m_node0 != NULL ) {
      totalSize += m_node0->getMemoryUsage();
   }
   if ( m_node1 != NULL ) {
      totalSize += m_node1->getMemoryUsage();
   }
   return totalSize;
}

/*
void 
OldRouteableItem::deleteMapData(OldGenericMap& theMap) {
   m_node0->deleteAllConnections(theMap);
   delete m_node0;
   m_node0 = NULL;
   m_node1->deleteAllConnections(theMap);
   delete m_node1;
   m_node1 = NULL;
}
*/

bool
OldRouteableItem::createFromDataBuffer(DataBuffer* dataBuffer,
                                    OldGenericMap* theMap)
{
   if (OldItem::createFromDataBuffer(dataBuffer, theMap)) {
      if (theMap == NULL) {
         m_node0 = createNewNode(dataBuffer, (m_localID & 0x7FFFFFFF));
         m_node1 = createNewNode(dataBuffer, (m_localID | 0x80000000));
      } else {
         m_node0 = getNewNode(theMap);
         m_node0->setNodeID(m_localID & 0x7FFFFFFF);
         m_node0->createFromDataBuffer(dataBuffer, theMap);
         m_node1 = getNewNode(theMap);
         m_node1->setNodeID(m_localID | 0x80000000);
         m_node1->createFromDataBuffer(dataBuffer, theMap);
 
      }
      
      return (true);
   } else {
      m_node0 = NULL;
      m_node1 = NULL;
      return (false);
   }
}


bool
OldRouteableItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   mc2dbg4 << "OldRouteableItem::updateAttributesFromItem" << endl;
   if (otherItem == NULL) {
      return retVal;
   }

   // Update routeable item specific attributes
   OldRouteableItem* otherRi = (OldRouteableItem*) otherItem;
   if (otherRi == NULL)
      return retVal;

   // Loop nodes
   for (uint32 n = 0; n < 2; n++) {
      OldNode* myNode = getNode(n);
      OldNode* otherNode = otherRi->getNode(n);
      if ((myNode == NULL) || (otherNode == NULL))
         return retVal;

      if (myNode->updateNodeAttributesFromNode(otherNode, n, sameMap))
         retVal = true;
   }
   
   return retVal;
}


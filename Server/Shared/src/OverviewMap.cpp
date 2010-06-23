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

#include "OverviewMap.h"

#include "StreetSegmentItem.h"

#include "DataBuffer.h"
#include "Node.h"

#include "StringUtility.h"

#include "DataBufferUtil.h"
#include "MapBits.h"

#include "DebugClock.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Note that index 0 of this array is invalid, since an overview map
// must be of maplevel 1 or higher (maplevel 0 means underview map).
const byte
OverviewMap::maxZoomLevelForMapLevel[] = { 6, 4, 3, 2 };


OverviewMap::OverviewMap()
      : GenericMap(),         
        m_containingMaps(0, 2)
{
   m_itemNames = new ItemNames("MISSING");
}

OverviewMap::OverviewMap(uint32 mapID)
      : GenericMap(mapID), 
        m_containingMaps(0, 2)
{
   m_itemNames = new ItemNames("MISSING");
}

OverviewMap::OverviewMap(uint32 mapID, const char* path)
   : GenericMap(mapID, path),
     m_containingMaps(0, 2)
{
   mc2dbg1 << "OverviewMap::OverviewMap(" << mapID << ", "
           << path << ")" << endl;
}

bool
OverviewMap::addToLookupTable(uint32 origMapID, uint32 origItemID,
                              uint32 newItemID)
{
   return m_idTranslationTable.addElement(newItemID, origMapID, origItemID);
}

bool
OverviewMap::removeFromLookupTable(uint32 newItemID)
{
   return m_idTranslationTable.removeElement(newItemID);
}

bool
OverviewMap::internalLoad(DataBuffer& dataBuffer)
{
   bool retVal = GenericMap::internalLoad(dataBuffer);
   mc2dbg8 << here << " GenericMap::internalLoad(dataBuffer) returned" 
           << endl;
   
   if (!retVal) {
      mc2log << fatal << "OverviewMap::internalLoad() Error calling "
             << "internalLoad() in superclass" << endl;
      return false;
   }
   uint32 mapSet = MapBits::getMapSetFromMapID(m_mapID);
   // Read the size of the m_containingMaps

   CLOCK_MAPLOAD(DebugClock clock);

   uint32 size = dataBuffer.readNextLong();
   m_containingMaps.setAllocSize(size);
   // Read and insert the elements into the m_containingMaps
   for (uint32 i=0; i<size; i++) {
      uint32 tmpItemID = dataBuffer.readNextLong();
      // these seem to be map IDs! fix mapSet info!
      tmpItemID = MapBits::getMapIDWithMapSet(tmpItemID, mapSet);
      m_containingMaps.addLast(tmpItemID);
   }
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Containing maps loaded in "
                        << clock << endl;
                 clock = DebugClock() );

   m_idTranslationTable.load(dataBuffer, mapSet);

   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                 << "] Lookup table loaded in "
                 << clock << endl;
                 clock = DebugClock());

   
   // Print all connections of this overview map
   DEBUG4(
      mc2dbg << "Connections in the OverviewMap" << endl;
      dumpConnections();
   );

   // Print all items in this overview map
   DEBUG4(
      mc2dbg << "All items in the overview map!" << endl;
      printAllItems(0);
   );

   // copy poi info data from databuffer
   copyPOIInfoData( dataBuffer );

   // If the native language index was not generated properly
   // at creation, we need to create it here.
   if ( getNbrNativeLanguages() == 0 ) {
      updateNativeLanguages();
   }

   return true;
}


OverviewMap::~OverviewMap()
{
}

bool
OverviewMap::internalSave(int outfile)
{
   // Save the general map data
   bool retVal = GenericMap::internalSave(outfile);

   if ( retVal == false ) {
      return retVal;
   }
   
   // Save the m_containingMaps
   DataBuffer* dataBuffer = 
      new DataBuffer(4*(m_containingMaps.getSize()+1));
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(m_containingMaps.getSize());
   for (uint32 i=0; i<m_containingMaps.getSize(); i++) {
      dataBuffer->writeNextLong(m_containingMaps.getElementAt(i));
   }

   DataBufferUtil::saveBuffer( *dataBuffer, outfile );

   delete dataBuffer;


   // Make sure they are sorted..
   m_idTranslationTable.sortElements();
   DataBufferUtil::saveObject( m_idTranslationTable, outfile );

   return true;
}



bool
OverviewMap::ssiConnectedToOverviewRoundabout(GenericMap* theMap, 
                                const int maxZoomLevelStreetSegmentItems,
                                StreetSegmentItem* ssi)
{
   bool retVal = false;
  
   // Check both nodes.
   for (byte i = 0; i < 2; i++) {
      Node* node = ssi->getNode(i);
      // Check all connections to the node.
      for (uint32 j = 0; j < node->getNbrConnections(); j++) {
         Connection* conn = node->getEntryConnection(j);
         StreetSegmentItem* connectedSSI = item_cast<StreetSegmentItem*> 
            (theMap->itemLookup(conn->getConnectFromNode()));
         if ( (connectedSSI != NULL) && 
              (connectedSSI->isRoundabout() ||
               connectedSSI->isRoundaboutish()) &&
              ( int(connectedSSI->getID() >> 27) 
                 <= maxZoomLevelStreetSegmentItems) ) {
            // The ssi is connected to a roundabout or roundaboutish
            // that is added to the overview map. 
            // Add this ssi also then!
            mc2dbg4 << "   Added roundabout(ish) exit segment." << endl;
            retVal = true;
         }
      }
   }

   return (retVal);
}

uint32
OverviewMap::reverseLookupNodeID(uint32 mapID, uint32 nodeID) const
{
   uint32 itemID =
      m_idTranslationTable.translateToHigher(mapID, nodeID & 0x7fffffff);
   uint32 nodePattern = nodeID & 0x80000000;
   return (itemID | nodePattern);
}

uint32
OverviewMap::reverseLookup(uint32 mapID, uint32 itemID) const
{
   return m_idTranslationTable.translateToHigher(mapID, itemID);
}

bool
OverviewMap::lookupNodeID(uint32 overviewNodeID,
                          uint32& mapID,
                          uint32& nodeID) const
{

   IDPair_t fullID =
      m_idTranslationTable.translateToLower(overviewNodeID&0x7fffffff);
   
   if (fullID.getMapID() != MAX_UINT32) {
      mapID = fullID.getMapID();
      uint32 nodePattern = overviewNodeID & 0x80000000;
      nodeID = fullID.getItemID() | nodePattern;
      return true;
   } else {
      mapID = MAX_UINT32;
      nodeID = MAX_UINT32;
      return false;
   }
}

IDPair_t
OverviewMap::lookup( uint32 overviewID ) const
{
   return m_idTranslationTable.translateToLower(overviewID);
}

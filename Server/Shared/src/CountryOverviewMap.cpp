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

#include "Stack.h"
#include "GfxData.h"
#include "CountryOverviewMap.h"
#include "PointOfInterestItem.h"
#include "DataBufferUtil.h"
#include "DebugClock.h"

// specialization for originalIDs_t
template <>
void SimpleArrayObject<CountryOverviewMap::originalIDs_t>::
save( DataBuffer& buff ) const {

   buff.writeNextLong( size() );

   for (uint32 i = 0; i < size(); ++i ) {
      buff.writeNextLong( data()[ i ].itemID );
      buff.writeNextLong( data()[ i ].origMapID );
      buff.writeNextLong( data()[ i ].origItemID );
   }
}

// specialization for originalIDs_t
template <>
void SimpleArrayObject<CountryOverviewMap::originalIDs_t>::
load( DataBuffer& buff ) {

   allocate( buff.readNextLong() );

   for ( uint32 i = 0; i < size(); ++i ) {
      data()[ i ].itemID = buff.readNextLong();
      data()[ i ].origMapID = buff.readNextLong();
      data()[ i ].origItemID = buff.readNextLong();
   }
}



const byte CountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX = 7;

const uint32 CountryOverviewMap::filtMaxDistance[] = 
   // 10000 is a reasonable alternative. We can see skälderviken, and
   //       bjärehalvön pretty good.
   // 5000  gives us höllviken/skanör pretty good. Skälderviken, bjärehalvön
   //       very good.
   { 25000, 15000, 10000, 5000, 2500, 1000, 500 };
const uint32 CountryOverviewMap::filtMinDistance[] =
   { 0, 0, 0, 0, 0, 0, 0 };

const uint32 CountryOverviewMap::mapGfxFilterLevels[] =
   { 12, 11, 10, 9, 8, 6, 5 };

CountryOverviewMap::CountryOverviewMap(uint32 id)
   :  GenericMap(id),
      m_mapsInCountry()
{
   // Create an emty ItemNames
   m_itemNames = new ItemNames();

   m_nbrGfxPolygons = 0;
   m_simplifiedGfxStack = NULL;
   
}

CountryOverviewMap::CountryOverviewMap(uint32 id,
                                       const char* path)
   :  GenericMap(id, path),
      m_mapsInCountry()
{
   // Create an emty ItemNames
   m_itemNames = new ItemNames();
   
   m_nbrGfxPolygons = 0;
   m_simplifiedGfxStack = NULL;
}

CountryOverviewMap::~CountryOverviewMap()
{
   deleteSimplifiedGfxStack();
}

bool 
CountryOverviewMap::internalSave(int outfile) try
{
   // Save the general map data
   bool retVal = GenericMap::internalSave(outfile);
   if ( ! retVal ) {
      return retVal;
   }
   
   mc2dbg1 << "SAVING COUNTRYMAP" << endl;

   // Scope to delete databuffer.
   {
      // Save the ID of the maps in this country
      DataBuffer dataBuffer((m_mapsInCountry.size()+1)*24);
      dataBuffer.writeNextLong(m_mapsInCountry.size());
      mc2dbg1 << "Nbr maps = " << m_mapsInCountry.size() << endl;
      for (uint32 i=0; i<m_mapsInCountry.size(); i++) {
         mc2dbg4 << "Saving map with ID=" 
                 << m_mapsInCountry[i].mapID << endl;
         dataBuffer.writeNextLong(m_mapsInCountry[i].mapID);
         dataBuffer.writeNextLong(m_mapsInCountry[i].creationTime);
         mc2dbg4 << "creationTime = " 
                 << m_mapsInCountry[i].creationTime << endl;
         dataBuffer.writeNextLong(m_mapsInCountry[i].maxLat);
         dataBuffer.writeNextLong(m_mapsInCountry[i].minLon);
         dataBuffer.writeNextLong(m_mapsInCountry[i].minLat);
         dataBuffer.writeNextLong(m_mapsInCountry[i].maxLon);
      }

      DataBufferUtil::saveBuffer( dataBuffer, outfile );
   }

   // Save the original ID of the items in this map
   DataBufferUtil::saveObject( m_originalIDs, outfile );
   
   // Scope to delete databuffer.
   {
      // Write the array of filter stacks.
      if (m_simplifiedGfxStack != NULL) {
         
         // 10MB should be enough.
         DataBuffer stackBuf(10000000); 

         // Write the number of bytes to follow (filled in later)
         stackBuf.writeNextLong(0);
         
         // Nbr levels of stack.
         stackBuf.writeNextLong(NBR_SIMPLIFIED_COUNTRY_GFX);

         // Nbr of polygons per level.
         stackBuf.writeNextLong(m_nbrGfxPolygons);
         // For each level.
         for (byte i = 0; i < NBR_SIMPLIFIED_COUNTRY_GFX; i++) {
            // And each stack for that level
            for (uint32 j = 0; j < m_nbrGfxPolygons; j++) {
               // The number of elements of this stack.
               Stack* curStack = m_simplifiedGfxStack[i][j];
               stackBuf.writeNextLong(curStack->getStackSize());
               // And write all elements
               for (uint32 k = 0; k < curStack->getStackSize(); k++) {
                  stackBuf.writeNextLong(curStack->getElementAt(k));
               }
            }
         }
         // Fill in size
         stackBuf.writeLong( stackBuf.getCurrentOffset() - 4, 0);         
         DataBufferUtil::saveBuffer( stackBuf, outfile );
      }
   }  

   return true;

} catch (MC2String err) {
   mc2dbg << error << "Exception in CountryOverviewMap::internalSave: " << err << endl;
   mc2dbg << error << strerror(errno) << endl;
   return false;
} catch (...) {
   mc2dbg << error << "Exception in CountryOverviewMap::internalSave!" << endl;
   mc2dbg << error << strerror(errno) << endl;
   return false;
}

bool 
CountryOverviewMap::internalLoad(DataBuffer& dataBuffer)
{
   // Load the general map data
   bool retVal = GenericMap::internalLoad(dataBuffer);

   if (retVal) {
      // Read the number of countries
      uint32 nbrCountries = dataBuffer.readNextLong();
      if (nbrCountries > 0) {
         CLOCK_MAPLOAD(DebugClock loadClock);
         // Insert the mapID's into the m_mapsInCountry-array
         for (uint32 i=0; i<nbrCountries; i++) {
            struct mapnotice_t notice;
            notice.mapID = dataBuffer.readNextLong();
            notice.creationTime = dataBuffer.readNextLong();
            notice.maxLat = dataBuffer.readNextLong();
            notice.minLon = dataBuffer.readNextLong();
            notice.minLat = dataBuffer.readNextLong();
            notice.maxLon = dataBuffer.readNextLong();
            mc2dbg2 << "[COMap] Creation time(0x" << hex << notice.mapID 
                    << dec  << "):" << notice.creationTime << endl;
            m_mapsInCountry.push_back(notice);
            mc2dbg2 << "   Added map with ID=" << notice.mapID << endl;
         }
         CLOCK_MAPLOAD(mc2log << "[" << m_mapID << "] Countries read in "
                        << loadClock
                        << "." << endl;
                 loadClock = DebugClock());


         // Read the number of original IDs
         m_originalIDs.load( dataBuffer );

         CLOCK_MAPLOAD(mc2log << "[" << m_mapID << "] Orig IDs read in "
                        << loadClock
                        << "." << endl;
                 loadClock = DebugClock());

         // Create the simplified representation of the country gfxdata.
         // If it exists on disk, then read from there, otherwise filter.
         if ( (dataBuffer.getBufferSize() - dataBuffer.getCurrentOffset()) 
                  > 0) {
            // Read the size.(not used now)
             dataBuffer.readNextLong();
            // Ok, read from disk.
               
            // Nbr levels of stack.
            uint32 levelsOfStack = dataBuffer.readNextLong();

            // Nbr polygons per level.
            m_nbrGfxPolygons = dataBuffer.readNextLong();

            // Allocate stack matrix.
            m_simplifiedGfxStack = new Stack**[levelsOfStack];

            // For each level.
            for (byte i = 0; i < levelsOfStack; i++) {
               // And each stack for that level
               m_simplifiedGfxStack[i] = new Stack*[m_nbrGfxPolygons];
               for (uint32 j = 0; j < m_nbrGfxPolygons; j++) {
                  Stack* curStack = new Stack;
                  // The number of elements of this stack.
                  uint32 nbrElems = dataBuffer.readNextLong();
                  curStack->reserve( nbrElems );
                  // And read all elements
                  for (uint32 k = 0; k < nbrElems; k++) {
                     curStack->push(dataBuffer.readNextLong());
                  }
                  m_simplifiedGfxStack[i][j] = curStack;
               }
            }
         } else {
            // Could not read from disk, filter now instead.
            mc2log << fatal << "CountryOverviewMap::internalLoad(), "
                   << "Error reading simplified representation of "
                   << "country gfxdata. Crash." << endl;
            MC2_ASSERT( false );
         }
         CLOCK_MAPLOAD(mc2log << "[" << m_mapID << "] Country GfxData "
                        << "created/read in "
                        << loadClock
                        << "." << endl);
      } else {
         mc2log << warn << "CountryOverviewMap::internalLoad Country "
                << "without any maps!" << endl;
      }
   }


   // copy poi info data from databuffer
   copyPOIInfoData( dataBuffer );

   mc2dbg << "[CountryOverviewMap] poi info offset updated: " 
          << m_poiInfoStartOffset << endl;

   // If the native language index was not generated properly
   // at creation, we need to create it here.
   if ( getNbrNativeLanguages() == 0 ) {
      updateNativeLanguages();
   }

   return retVal;
}


bool 
CountryOverviewMap::saveStringItems(DataBuffer* dataBuffer)
{
   // Nbr items, filled in later
   uint32 nbrStringItemsOffset = dataBuffer->getCurrentOffset();
   dataBuffer->writeNextLong(0);

   uint32 nbrStringItems = 0;
   mc2dbg << "CountryOverviewMap::saveStringItems" << endl;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      mc2dbg << here << " z = " << z << ", nbrItems=" 
             << getNbrItemsWithZoom(z) << endl;
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         Item* item = getItem(z,i);
         if ( (item != NULL) &&
              //(item->getGfxData() != NULL) &&
              ( (item->getItemType() == ItemTypes::municipalItem) ||
                (item->getItemType() == ItemTypes::builtUpAreaItem) ||
                (item->getItemType() == ItemTypes::zipCodeItem) ||
                ( ( item->getItemType() == 
                    ItemTypes::pointOfInterestItem ) &&
                  ( z == 1 ) && // GfxClient POI:s are on zoomlevel 1.
                  ( ( static_cast<PointOfInterestItem*> ( item )
                      ->isPointOfInterestType( ItemTypes::cityCentre ) ) ||
                    ( static_cast<PointOfInterestItem*> ( item )
                      ->isPointOfInterestType( ItemTypes::mountainPass ) )
                  ) 
                )    
              )
            ) {
            // Item type
            if (item->getItemType() == ItemTypes::pointOfInterestItem) { 
               // We already checked that it is a citycentre or a mountain
               // pass. Add it as a BUA.
               dataBuffer->writeNextShort(ItemTypes::builtUpAreaItem);
            } else {
               dataBuffer->writeNextShort(item->getItemType());
            }
            mc2dbg4 << here << " Including item with type= " 
                   << uint32(item->getItemType()) << endl;

            // Names
            uint16 nbrNames = item->getNbrNames();
            Vector nameIdx(nbrNames);
            Vector uniqueNameIdx(nbrNames);
            for (byte j=0; j<nbrNames; j++) { 
               if (item->getNameType(j) == ItemTypes::uniqueName) {
                  uniqueNameIdx.addLast(j);
                  mc2dbg8 << "Found unique name, " 
                          << getName(item->getStringIndex(j)) << "("
                          << item->getStringIndex(j) << ")" << endl;
               } else if( item->getNameType(j) != ItemTypes::synonymName) {
                  nameIdx.addLast(j);
                  mc2dbg8 << "Added name, " 
                          << getName(item->getStringIndex(j)) << endl;
               } else if ( item->getItemType() == 
                           ItemTypes::municipalItem )
               {
                  // Synonym name and municipal.
                  nameIdx.addLast(j);
                  mc2dbg8 << "Added name, " 
                          << getName(item->getStringIndex(j)) << endl;
               }
            }  
            // Replace all official names with the unique name in the same 
            // language
            for (byte j=0; j<nameIdx.getSize(); j++) {
               byte idx = nameIdx.getElementAt(j);
               if (item->getNameType(idx) == ItemTypes::officialName) {
                  LangTypes::language_t lang = item->getNameLanguage(idx);
                  byte uo = 0;
                  while ( (uo < uniqueNameIdx.getSize()) &&
                          (item->getNameLanguage(
                            uniqueNameIdx.getElementAt(uo)) != lang)) {
                     ++uo;
                  }
                  if (uo < uniqueNameIdx.getSize()) {
                     // name with index uo and idx has the same language.
                     nameIdx.setElementAt(j, uniqueNameIdx.getElementAt(uo));
                     uniqueNameIdx.removeElementAt(uo);
                     mc2dbg8 << "Exchanging strings" << endl;
                  }
               }
            }
            // If there are any names left in uniqueNameIdx -- add to nameIdx
            for (byte uo=0; uo<uniqueNameIdx.getSize(); ++uo) {
               nameIdx.addLast(uniqueNameIdx.getElementAt(uo));
               mc2log << warn << here << " Added unique name ("
                      << getName(item->getStringIndex(
                                   uniqueNameIdx.getElementAt(uo)))
                      << ") without corresponding official" << endl;
            }
            // Save to databuffer
            dataBuffer->writeNextShort(nameIdx.getSize());
            mc2dbg8 << "   NbrNames = " << nameIdx.getSize() << endl;
            for (uint32 j=0; j<nameIdx.getSize(); j++) {
               dataBuffer->writeNextLong(
                     item->getRawStringIndex(nameIdx.getElementAt(j)));
               mc2dbg8 << "      name = " 
                       << item->getRawStringIndex(nameIdx.getElementAt(j))
                       << getName(item->getStringIndex(
                          nameIdx.getElementAt(j)))
                       << endl;
            }

            // Original IDs
            uint32 origMapID;
            uint32 origItemID;
            if (!getOriginalIDs(item->getID(), origMapID, origItemID)) {
               mc2log << error << here << "CountryOverviewMap, "
                      << "Error getting original IDs" << endl;
            }
            dataBuffer->writeNextLong(origMapID);
            mc2dbg8 << "origMapID = " << origMapID << endl;
            dataBuffer->writeNextLong(origItemID);
            mc2dbg8 << "origItemID = " << origItemID << endl;

            // SearchMask
            //uint32 searchMask = Utility::MM2SMLocation(item->getRawLocation());
            // True item type.
            dataBuffer->writeNextLong( item->getItemType() );
            //dataBuffer->writeNextLong(searchMask);
            //mc2dbg8 << "searchMask = " << searchMask << endl;

            // Bounding box
            const GfxData* gfx = NULL;
            if (item->getGfxData() != NULL) {
               gfx = item->getGfxData();
            } else {
               // Approximate with GfxData for the map...
               gfx = getGfxData();
            }

            dataBuffer->writeNextLong(gfx->getMaxLat());
            dataBuffer->writeNextLong(gfx->getMinLon());
            dataBuffer->writeNextLong(gfx->getMinLat());
            dataBuffer->writeNextLong(gfx->getMaxLon());
            mc2dbg8 << "bbox = " << gfx->getMaxLat() << ", " 
                    << gfx->getMinLon() << ", " << gfx->getMinLat() << ", "
                    << gfx->getMaxLon() << endl;

            // Center-point. 
            // FIXME: Currently uses the middle of bbox
            dataBuffer->writeNextLong((gfx->getMaxLat() + gfx->getMinLat())/2);
            dataBuffer->writeNextLong((gfx->getMaxLon() + gfx->getMinLon())/2);
            mc2dbg8 << "centerPoint = " 
                    << (gfx->getMaxLat() + gfx->getMinLat())/2 << ", "
                    << (gfx->getMaxLon() + gfx->getMinLon())/2 << endl;

            // Update the number of written string items
            nbrStringItems++;
         }
      }
   }
   
   // Write the number of string items
   dataBuffer->writeLong(nbrStringItems, nbrStringItemsOffset);

   return (true);
}

void
CountryOverviewMap::deleteSimplifiedGfxStack()
{
   if (m_simplifiedGfxStack != NULL) {
      for (uint32 i = 0; 
           i < CountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX; 
           i++) {
         if (m_simplifiedGfxStack[i] != NULL) {
            for (uint32 j = 0; j < m_nbrGfxPolygons; j++) {
               delete m_simplifiedGfxStack[i][j];
            }
         }
         delete[] m_simplifiedGfxStack[i];
      }
   }
   delete[] m_simplifiedGfxStack;
}


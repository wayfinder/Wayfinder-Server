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
#include "OldCountryOverviewMap.h"
#include "OldPointOfInterestItem.h"
#include "TimeUtility.h"
      
const byte OldCountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX = 7;

const uint32 OldCountryOverviewMap::filtMaxDistance[] = 
   { 25000, 15000, 10000, 5000, 2500, 1000, 500 };
const uint32 OldCountryOverviewMap::filtMinDistance[] =
   { 0, 0, 0, 0, 0, 0, 0 };

const uint32 OldCountryOverviewMap::mapGfxFilterLevels[] =
   { 12, 11, 10, 9, 8, 6, 5 };

OldCountryOverviewMap::OldCountryOverviewMap(uint32 id)
   :  OldGenericMap(id),
      m_mapsInCountry()
{
   // Create an emty OldItemNames
   m_itemNames = new OldItemNames();

   m_nbrGfxPolygons = 0;
   m_simplifiedGfxStack = NULL;
   
}

OldCountryOverviewMap::OldCountryOverviewMap(uint32 id,
                                       const char* path)
   :  OldGenericMap(id, path),
      m_mapsInCountry()
{
   // Create an emty OldItemNames
   m_itemNames = new OldItemNames();
   
   m_nbrGfxPolygons = 0;
   m_simplifiedGfxStack = NULL;
}

OldCountryOverviewMap::~OldCountryOverviewMap()
{
   deleteSimplifiedGfxStack();
}

bool 
OldCountryOverviewMap::internalSave(int outfile)
{
   // Save the general map data
   bool retVal = OldGenericMap::internalSave(outfile);
   
   mc2dbg1 << "SAVING COUNTRYMAP" << endl;
   if (retVal) {
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

      // Save to file
      write(outfile, 
            dataBuffer.getBufferAddress(), 
            dataBuffer.getCurrentOffset());

      // Save the original ID of the items in this map
      uint32 nbrOriginalIDs = m_originalIDs.size();
      mc2dbg1 << "To save " << nbrOriginalIDs << " original ID:s" << endl;
      DataBuffer origIDs(4 + nbrOriginalIDs*12);
      origIDs.writeNextLong(nbrOriginalIDs);
      map<uint32, struct originalIDs_t>::iterator p = m_originalIDs.begin();
      while (p != m_originalIDs.end()) {
         origIDs.writeNextLong(p->first);
         origIDs.writeNextLong(p->second.origMapID);
         origIDs.writeNextLong(p->second.origItemID);
         mc2dbg4 << "Saving itemID " << p->first << " orig("
                 << p->second.origMapID << "." << p->second.origItemID 
                 << ")" << endl;
         p++;
      }

      // Write the original IDs to file
      write(outfile, 
            origIDs.getBufferAddress(), 
            origIDs.getCurrentOffset());
      
      // Write the array of filter stacks.
      if (m_simplifiedGfxStack != NULL) {

         int numElements = m_nbrGfxPolygons * NBR_SIMPLIFIED_COUNTRY_GFX;
         // 10MB should be enough.
         for (byte i = 0; i < NBR_SIMPLIFIED_COUNTRY_GFX; i++) {
            // And each stack for that level
            for (uint32 j = 0; j < m_nbrGfxPolygons; j++) {
               // The number of elements of this stack.
               numElements += m_simplifiedGfxStack[i][j]->getStackSize();
            }
         }

         mc2dbg4 << "required stack size="<<4*numElements+12<<endl;

         DataBuffer* stackBuf = new DataBuffer(4*numElements + 12);

         // Write the number of bytes to follow (filled in later)
         stackBuf->writeNextLong(0);
         
         // Nbr levels of stack.
         stackBuf->writeNextLong(NBR_SIMPLIFIED_COUNTRY_GFX);

         // Nbr of polygons per level.
         stackBuf->writeNextLong(m_nbrGfxPolygons);
         // For each level.
         for (byte i = 0; i < NBR_SIMPLIFIED_COUNTRY_GFX; i++) {
            // And each stack for that level
            for (uint32 j = 0; j < m_nbrGfxPolygons; j++) {
               // The number of elements of this stack.
               Stack* curStack = m_simplifiedGfxStack[i][j];
               stackBuf->writeNextLong(curStack->getStackSize());
               // And write all elements
               for (uint32 k = 0; k < curStack->getStackSize(); k++) {
                  stackBuf->writeNextLong(curStack->getElementAt(k));
               }
            }
         }
         // Fill in size
         stackBuf->writeLong( stackBuf->getCurrentOffset() - 4, 0);
         write(outfile, 
               stackBuf->getBufferAddress(), 
               stackBuf->getCurrentOffset());
         delete stackBuf;
      }
      

   } else {
      mc2log << error << here << " Failed to save country overview map" 
             << endl;
   }

   return (retVal);
}

bool 
OldCountryOverviewMap::internalLoad(DataBuffer& dataBuffer)
{
   // Load the general map data
   bool retVal = OldGenericMap::internalLoad(dataBuffer);

   if (retVal) {
      // Read the number of countries
      uint32 nbrCountries = dataBuffer.readNextLong();
      if (nbrCountries > 0) {
         CLOCK_MAPLOAD(uint32 startTime = TimeUtility::getCurrentMicroTime());
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
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());


         // Read the number of original IDs
         uint32 nbrOrigIDs = dataBuffer.readNextLong();

         // Read the original IDs of the items in this map
         mc2dbg2 << "To insert " << nbrOrigIDs << " original IDs" << endl;
         for (uint32 i=0; i<nbrOrigIDs; i++) {
            uint32 newID = dataBuffer.readNextLong();
            struct originalIDs_t origIDs;
            origIDs.origMapID = dataBuffer.readNextLong();
            origIDs.origItemID = dataBuffer.readNextLong();
            mc2dbg4 << "   ID: " << newID << " <==> " << origIDs.origMapID
                    << "." << origIDs.origItemID << endl;
            m_originalIDs.insert(make_pair(newID, origIDs));
         }
         CLOCK_MAPLOAD(mc2log << "[" << m_mapID << "] Orig IDs read in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());

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
                  // And read all elements
                  for (uint32 k = 0; k < nbrElems; k++) {
                     curStack->push(dataBuffer.readNextLong());
                  }
                  m_simplifiedGfxStack[i][j] = curStack;
               }
            }
         } else {
            // Could not read from disk, filter now instead.
            mc2log << info << "OldCountryOverviewMap::internalLoad(), "
                   << "Error reading simplified representation of "
                   << "country gfxdata. Exits." << endl;
            //makeSimplifiedCountryGfx();
            exit(1);
         }
         CLOCK_MAPLOAD(mc2log << "[" << m_mapID << "] Country GfxData "
                        << "created/read in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());
      } else {
         mc2log << warn << "OldCountryOverviewMap::internalLoad Country "
                << "without any maps!" << endl;
      }
   }

   return retVal;
}


bool 
OldCountryOverviewMap::saveStringItems(DataBuffer* dataBuffer)
{
   // Nbr items, filled in later
   uint32 nbrStringItemsOffset = dataBuffer->getCurrentOffset();
   dataBuffer->writeNextLong(0);

   uint32 nbrStringItems = 0;
   mc2dbg << "OldCountryOverviewMap::saveStringItems" << endl;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      mc2dbg << here << " z = " << z << ", nbrItems=" 
             << getNbrItemsWithZoom(z) << endl;
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z,i);
         if ( (item != NULL) &&
              //(item->getGfxData() != NULL) &&
              ( (item->getItemType() == ItemTypes::municipalItem) ||
                (item->getItemType() == ItemTypes::builtUpAreaItem) ||
                (item->getItemType() == ItemTypes::zipCodeItem) ||
                ( ( item->getItemType() == 
                    ItemTypes::pointOfInterestItem ) &&
                  ( z == 1 ) && // GfxClient POI:s are on zoomlevel 1.
                  ( ( static_cast<OldPointOfInterestItem*> ( item )
                      ->isPointOfInterestType( ItemTypes::cityCentre ) ) ||
                    ( static_cast<OldPointOfInterestItem*> ( item )
                      ->isPointOfInterestType( ItemTypes::mountainPass ) )
                  ) 
                )    
              )
            ) {
            // OldItem type
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
               mc2log << error << here << "OldCountryOverviewMap, "
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
            // Currently uses the middle of bbox
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

bool
OldCountryOverviewMap::makeSimplifiedCountryGfx() 
{
   bool retVal = true;

   const GfxData* mapGfx = getGfxData();
   m_nbrGfxPolygons = mapGfx->getNbrPolygons();
   
   // Avoid memory leaks
   delete[] m_simplifiedGfxStack;

   // Allocate matrix.
   m_simplifiedGfxStack = new Stack**[NBR_SIMPLIFIED_COUNTRY_GFX];
   
   
   if ( mapGfxDataIsFiltered() ) {
      // create the stacks using the filter levels stored in map gfx data
      // coordinates
      const char* mapName = getMapName();
      mc2dbg1 << "Simplify " << mapName << " using coordinate filter levels"
              << endl;
      
      for (uint32 level = 0; level < NBR_SIMPLIFIED_COUNTRY_GFX; level++) {
         
         // Allocate array
         m_simplifiedGfxStack[level] = new Stack*[m_nbrGfxPolygons];
         
         // Go through all polygons of the country.
         for (uint32 poly = 0; poly < m_nbrGfxPolygons; poly++) {
            Stack* polyStack = new Stack;

            uint32 coordIdx = 0;
            GfxData::const_iterator end = mapGfx->polyEnd( poly );
            for ( GfxData::const_iterator it = mapGfx->polyBegin( poly );
                  it != end; it++ ) {
               uint32 filtLevel = 
                  static_cast<const FilteredCoord&>(*it).getFilterLevel();
               if ( filtLevel >= mapGfxFilterLevels[level] ) {
                  polyStack->push( coordIdx );
               }
               coordIdx++;
            }
            mc2dbg4 << " Filtering " << mapName 
                    << " [" << level << "][" << poly << "]" 
                    << " resulted in stack size " << polyStack->getStackSize()
                    << endl;
            // Add the stack to the array.
            m_simplifiedGfxStack[level][poly] = polyStack;
         }
      }
   
      return (retVal);
   }
   
   // else the original method below
   for (uint32 level = 0; level < NBR_SIMPLIFIED_COUNTRY_GFX; level++) {
      
      // Allocate array
      m_simplifiedGfxStack[level] = new Stack*[m_nbrGfxPolygons];
      
      // Go through all polygons of the country.
      for (uint32 poly = 0; poly < m_nbrGfxPolygons; poly++) {
         Stack* polyStack = new Stack;

         if (! mapGfx->getSimplifiedPolygon( polyStack,
                                             poly,
                                             filtMaxDistance[level],
                                             filtMinDistance[level] ) ) {
            // The stack did not get filled.
            // Lets make it simple for us, simply add all indices to the
            // stack!
            mc2dbg1 << "  Did not fill stack. Add all indices instead!" 
                    << endl;
            for (uint32 i = 0; i < mapGfx->getNbrCoordinates(poly); i++)
               polyStack->push(i);
         }
         
         mc2dbg1 << " Filtering [" << level << "][" << poly << "]" 
                 << " resulted in stack size " << polyStack->getStackSize()
                 << endl;
         // Add the stack to the array.
         m_simplifiedGfxStack[level][poly] = polyStack;

      }

   }
   
   return (retVal);
}

void
OldCountryOverviewMap::deleteSimplifiedGfxStack()
{
   if (m_simplifiedGfxStack != NULL) {
      for (uint32 i = 0; 
           i < OldCountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX; 
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
      
bool 
OldCountryOverviewMap::updateCreationTimes( 
                              map<uint32, uint32> creationTimeByMap,
                              bool onlyPerformCheck )
{
   bool retVal = false;
   for ( vector<mapnotice_t>::iterator it = m_mapsInCountry.begin(); 
         it != m_mapsInCountry.end(); ++it ) {
      // Try to find the creation time in creationTimeByMap
      map<uint32, uint32>::const_iterator jt = 
         creationTimeByMap.find( it->mapID );
      if ( jt != creationTimeByMap.end() ) {
         if ( it->creationTime != jt->second ) {
            mc2dbg1 << "[COMap] Creation time for map " << it->mapID
                    << " stored as " << it->creationTime
                    << " but it really is " << jt->second << endl;
            retVal = true;
            if ( ! onlyPerformCheck ) {
               // Found. Update creation time.
               it->creationTime = jt->second;
            }
         }
      }
      else {
         mc2log << error << "OldCountryOverviewMap::updateCreationTimes. "
                << " OldMap: 0x" << hex << it->mapID 
                << " referenced by country overview map: 0x" 
                << this->getMapID() << " is not present in current"
                << " generation directory." << dec << endl;
         exit(1);
      }

   }
   return retVal;
}


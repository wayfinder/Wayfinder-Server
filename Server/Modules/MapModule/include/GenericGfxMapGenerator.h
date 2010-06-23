/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GENERICGFXMAPGENERATOR_H
#define GENERICGFXMAPGENERATOR_H

#include "MapGenerator.h"
#include "ItemTypes.h"

#define PRINT_GFX_STATISTICS

class MapHandler;
class GenericMap;
class DataBuffer;

class Item;

/**
  *   Abstract class which is superclass to all GfxMapGenerators.
  *   Has methods for selecting which items to be included and
  *   how to store the items into a databuffer.
  *   
  *   Objects of this class extracts an geographic "map" and sends it 
  *   via socket to the module/server.
  *
  */
class GenericGfxMapGenerator : public MapGenerator {
   public:
      /**
        *   Creates one GenericGfxMapGenerator.
        *
        *   @param   mh The MapHandler to use to retreiving the Map:s
        *   @param   mapID Pointer to the mapID that the searchmap 
        *            should be generated for.
        *   @param   port The port to listen to connection on.
        */
      GenericGfxMapGenerator( MapHandler *mh,
                              uint32 *mapIDs,
                              uint32 port );
      
      /**
       *    Delete this map generator.
       */
      virtual ~GenericGfxMapGenerator();


   protected:
      
      /**
       *    Abstract method.
       *    Determine if to include a item on given zoomlevel.
       *
       *    The caller make sure that in case a StreetItem is 
       *    sent to this method, all polygons of that StreetItem must be
       *    checked seperately (with the polygon parameter set).
       *    This is the case since different polygons of a StreetItem
       *    can have different roadclasses.
       *    
       *    @param item    The item to check.
       *    @param theMap  The map containing the item.
       *    @param reduceGfxToPoint Outparameter, set to true if the 
       *                            graphics should be reduced to a
       *                            single point.
       *    @param polygon Only need to be supplied when passing a 
       *                   StreetItem as item. Which polygon of the
       *                   StreetItem to check if it is to be included.
       *    @return True if item should be included, false otherwise.
       */
      virtual bool includeItem(const Item* item, GenericMap* theMap,
                               bool& reduceGfxToPoint, 
                               uint16 polygon = MAX_UINT16) = 0;
      
      /**
        *   Saves the graphical information about one item into
        *   a DataBuffer.
        *
        *    The caller make sure that in case a StreetItem is 
        *    sent to this method, all polygons of that StreetItem must be
        *    checked seperately (with the polygon parameter set).
        *    This is the case since different polygons of a StreetItem
        *    can have different roadclasses.
        *    
        *   @param   item        The item to save.
        *   @param   itemBuffer  Pointer to where the item is saved.
        *   @param   theMap      The map containing the item.
        *   @param   polygon     Only need to be supplied when passing a 
        *                        StreetItem as item. Which polygon of the
        *                        StreetItem to check if it is to be 
        *                        included.
        *   @return  True if the item saved alright, false otherwise.
        */
      bool writeItem(Item* item, DataBuffer* itemBuffer, 
                     GenericMap* theMap, uint16 polygon = MAX_UINT16);

      /**
        *   Write's all items in the specified zoomlevel to the databuffer.
        *   @param   theMap      The map holding the items.
        *   @param   itemBuffer  Pointer to where the items is saved.
        *   @param   zoomLevel   The zoomlevel in the Map.
        *   @param   nbrItems    In/Outparameter, which is increased with 
        *                        the number of items written.
        */
      void writeItemsInZoomLevel(GenericMap* theMap,
                                 DataBuffer* itemBuffer,
                                 uint32 zoomLevel, 
                                 uint32& nbrItems);
      
      /**
       *    Array containing number items per itemtype.
       */
      uint32 m_nbrItemsPerItemType[ItemTypes::numberOfItemTypes];

      #ifdef PRINT_GFX_STATISTICS
         void resetStatistics();
         void calculateStatistics(uint32 itemLength, Item* item);
         void printStatistics(uint32 zoomLevel, GenericMap* theMap);
      
         uint32 m_nbrMunicipals;
         uint32 m_munizipalSize;
         uint32 m_nbrStreetSegments;
         uint32 m_streetSegmentSize;
         uint32 m_nbrStreets;
         uint32 m_streetSize;
         uint32 m_nbrCityParts;
         uint32 m_cityPartSize;
         uint32 m_nbrWaters;
         uint32 m_waterSize;
         uint32 m_nbrPark;
         uint32 m_parkSize;
         uint32 m_nbrBuildings;
         uint32 m_buildingSize;
         uint32 m_nbrIndividualBuildings;
         uint32 m_individualBuildingSize;
         uint32 m_nbrBuiltUpAreas;
         uint32 m_builtUpAreaSize;
         uint32 m_nbrIslands;
         uint32 m_islandSize;
         uint32 m_nbrRailways;
         uint32 m_railwaySize;
         uint32 m_nbrFerries;
         uint32 m_ferrySize;
         uint32 m_nbrPointOfInterest;
         uint32 m_pointOfInterestSize;
         uint32 m_nbrDefault;
         uint32 m_defaultSize;
      #endif
};

#endif

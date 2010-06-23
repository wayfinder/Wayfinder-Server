/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OVERVIEWMAP_H
#define OVERVIEWMAP_H

#include "config.h"
#include "GenericMap.h"
#include "Map.h"
#include "Vector.h"
#include "ItemTypes.h"
#include <deque>
#include <list>


class DataBuffer;

/**
  *   Describes one overview map in the system. A overview looks 
  *   almost the same as an ordinary map, but with with following
  *   exceptions:
  *   \begin{description}
  *      \item[Included items] Only municipals and built up areas are
  *         included. In the future also bigger lakes, forests etc.
  *         will be included.
  *      \item[Large roads] In the overview map only the 
  *         StreetSegmentItems at the highest three levels (according
  *         to the GDF-file) are included. There are no small roads, 
  *         no areas, no companies, no categories etc.
  *      \item[Translation table] To make it possible to translate
  *         the IDs in this overview map, to the true ID in the
  *         base map, there is one translation table in this overview 
  *         map.
  *   \end{description}
  *
  */
class OverviewMap : public GenericMap {

public:

   /**
    *    The maximum zoomlevel for streetsegments to be added
    *    to the overviewmap. Used by this class to when adding
    *    and the RouteModule for knowing what low level is.
    */
   static const int maxZoomLevelStreetSegmentItems = 4;

   /**
    *    Vector specifying the maximum zoomlevel that should be included
    *    in the overview map given the maplevel of the overview map.
    *    Note that index 0 of this array is invalid, since an overview 
    *    map must be of maplevel 1 or higher 
    *    (maplevel 0 means underview map).
    *    The overview map lowest in hierarchy has maplevel 1, the
    *    second lowest has maplevel 2 etc.
    */
   static const byte maxZoomLevelForMapLevel[];
      
   /**
    *    Returns true if the item is on low level.
    */
   static inline bool isLowerLevel(uint32 itemID);

   /**
    *    Returns true if the node should exist on level
    *    <code>level</code>.
    *    @param level The level to check.
    *    @param nodeID The node to check.
    *    @return true if the node should exist on the supplied level.
    */
   static inline bool existsOnLevel(int level, uint32 nodeID);
      
   /**
    *   Create one empty overview map.
    */
   OverviewMap();

   /**
    *   Create one overview map with given ID.
    */
   explicit OverviewMap(uint32 mapID);

   /**
    *   Create a new OverviewMap. The map is {\bf not} filled with
    *   data from the file. This must be done by calling the 
    *   load()-method after creation.
    *
    *   @param   mapID The ID (and also the name) of the map.
    *   @param   path  Path to map to load.
    */
   OverviewMap(uint32 mapID, const char* path);

   /**
    *   Delete this map.
    */
   virtual ~OverviewMap();

   /**
    *   Check if a specified map is contained in this overview.
    *
    *   @return  True if the map with the given ID is already in this
    *            overview map, false if it is not.
    */
   inline bool containsMap(uint32 mapID);

   /**
    *   Find the node ID in this overview map when the original
    *   map and item IDs are known. NB! Before calling this method
    *   the m_reverseLookupTable {\it must} be sorted!
    *
    */
   uint32 reverseLookupNodeID(uint32 mapID, uint32 nodeID) const;

   /**
    *   Find the original map ID and item ID when the ID in this
    *   overview map is known.
    *
    *   @param   overviewNodeID
    *                     ID in this overview map.
    *   @param   mapID    Outparameter that is set to the true
    *                     map ID of this node.
    *   @param   nodeID   Outparameter that is set to the true
    *                     node ID of this node.
    *   @return  True if the outparameters are set correctly, false
    *            otherwise.
    */
   bool lookupNodeID(uint32 overviewNodeID, 
                     uint32& mapID, uint32& nodeID) const;

   /**
    *   Get a lookup element that contains the true IDs as well as
    *   the ID in the overviwe map from true mapID and true item ID.
    *   NB! Before calling this method the m_reverseLookupTable 
    *   {\it must} be sorted!
    *   
    *   @param   mapID    ID of the true map where this item is 
    *                     located.
    *   @param   itemID   ID of the item on the true map.
    *   @return  One element containing data about this item, NULL is
    *            return if no element is found.
    */
   uint32 reverseLookup(uint32 mapID, uint32 itemID) const;

   /**
    *   Get a lookup element that contains the true IDs as well as
    *   the ID in the overviwe map from IDon the overview map.
    *   NB! Before calling this method the m_reverseLookupTable 
    *   {\it must} be sorted!
    *   
    *   @param   overviewID  ID of the item on the overview map.
    *   @return  One element containing data about this item, NULL is
    *            return if no element is found.
    */
   IDPair_t lookup( uint32 overviewID ) const;

   /**
    *   Get the number of items on this overview map.
    *   @return  The number of Items in this map.
    */
   inline uint32 getNbrOverviewItems() const;

   /**
    *   Get an item on the overviewmap. This method is to be used
    *   to retrieve item number i.
    *
    *   @param   i           The cardinal number of the item.
    *   @param   overviewID  Outparameter that is set to the overview
    *                        ID of item number i.
    *   @param   origMapID   Outparameter that is set to the ID of the
    *                        map where item number i is located.
    *   @param   origItemID  Outparameter that is set to the ID of 
    *                        item number i on the true map.
    */
   inline Item* getOverviewItem(uint32 i,
                                uint32 &overviewID,
                                uint32 &origMapID,
                                uint32 &origItemID) const;


   Vector &getContainingMaps() { return m_containingMaps; }


protected:

   /**
    *   Load this overview map from file.
    */
   virtual bool internalLoad(DataBuffer& dataBuffer);

   /**
    *   Save this overviewmap to disc.
    */
   virtual bool internalSave(int outfile);

private:

   /**
    *   Methos that adds one item to the reverse lookup table.
    *
    *   @param   origMapID   ID of this map where this item is
    *                        located.
    *   @param   origItemID  ID of this item on the map with trueMapID.
    *   @param   newItemID   ID of this item in this overview map.
    *   @return  True if the item is added correctly, false
    *            otherwise.
    */
   bool addToLookupTable(uint32 origMapID, 
                         uint32 origItemID, 
                         uint32 newItemID);

   /**
    *    Method that removes one item from the lookup table.
    *    @param   newItemID   ID of this item in this overview map.
    *    @return  True if the item is removed, false otherwise.
    */
   bool removeFromLookupTable(uint32 newItemID);

   /**
    *    Help method to addMap() used to check if a certain 
    *    streetsegmentitem is connected to a roundabout that is
    *    added to the overviewmap. This check is performed in order
    *    to make sure that all exits of a roundabout are added, since
    *    the turndescriptions will be incorrect otherwise.
    *    
    *    @param   theMap   The map containing the items.
    *    @param   maxZoomLevelStreetSegmentItems
    *                      The maximum zoomlevel of the streetsegmentitems
    *                      that are added to the overviewmap.
    *    @param   ssi      The ssi that should be checked if it has any
    *                      connections to a overviewmap roundabout.
    *    @return  True if the ssi was connected to a overviewmap roundabout,
    *             false otherwise.
    */
   bool ssiConnectedToOverviewRoundabout(GenericMap* theMap, 
                                 const int maxZoomLevelStreetSegmentItems,
                                         StreetSegmentItem* ssi);




   /** Creates and adds a group item for grouping admin areas in 
    *  overview map. Sets the new group item as group to the items in
    *  the parameter items.
    *
    *  @parma nameItem      The item to pick the names. from. Typically
    *                       one of the items in the items parameter.
    *  @parma groupItemType The type of the group item to create.
    *  @parma items         The items to group with the new group item.
    *
    *  @return Returns the new group item if successful, otherwise NULL.
    */
   Item* addGroupItem( Item* nameItem, 
                       ItemTypes::itemType groupItemType,
                       set<Item*>& items);


   /**
    *   Vector with ID of the maps that is located on this overview.
    */
   Vector m_containingMaps;


};


// ==================================================================
//                                Implementation of inlined methods =

inline bool
OverviewMap::containsMap(uint32 mapID) 
{
   uint32 tmpVal = m_containingMaps.binarySearch(mapID);
   return tmpVal < MAX_UINT32;
}

inline uint32 
OverviewMap::getNbrOverviewItems() const
{
   return m_idTranslationTable.getNbrElements();
}

inline Item*
OverviewMap::getOverviewItem(uint32 i, uint32 &overviewID,
                             uint32 &origMapID, uint32 &origItemID) const
{

   if (m_idTranslationTable.getElement(i, overviewID, origMapID, origItemID)) {
      return itemLookup(overviewID);
   }
   return NULL;
}

inline bool
OverviewMap::isLowerLevel(uint32 itemID)
{
   // From RouteConstants.
#define GET_ZOOM_LEVEL(a) (((a) & 0x7fffffff) >> 27)
#define IS_LOWER_LEVEL(a) ( GET_ZOOM_LEVEL(a) > \
                         (uint32)OverviewMap::maxZoomLevelStreetSegmentItems )
   return IS_LOWER_LEVEL(itemID);
#undef IS_LOWER_LEVEL
#undef GET_ZOOM_LEVEL
}

inline bool
OverviewMap::existsOnLevel(int level, uint32 nodeID)
{
   // From RouteConstants.
#define GET_ZOOM_LEVEL(a) (((a) & 0x7fffffff) >> 27)
   const uint32 zoom = maxZoomLevelForMapLevel[level];
   return GET_ZOOM_LEVEL(nodeID) <= zoom;
#undef IS_LOWER_LEVEL
#undef GET_ZOOM_LEVEL  
}

#endif


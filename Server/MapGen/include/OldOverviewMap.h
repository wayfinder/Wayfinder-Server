/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDOVERVIEWMAP_H
#define OLDOVERVIEWMAP_H

#include "config.h"
#include "OldGenericMap.h"
#include "OldMap.h"
#include "Vector.h"
#include "ItemTypes.h"
#include <deque>
#include <list>

class DataBuffer;
#include "NationalProperties.h"
#include "OldStreetSegmentItem.h"

class OldOverviewMap : public OldGenericMap {

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
        *   Create one empty overview map.
        */
      OldOverviewMap();

      /**
        *   Create one overview map with given ID.
        */
      OldOverviewMap(uint32 mapID);

      /**
        *   Create a new OldOverviewMap. The map is {\bf not} filled with
        *   data from the file. This must be done by calling the 
        *   load()-method after creation.
        *
        *   @param   mapID The ID (and also the name) of the map.
        *   @param   path  Path to map to load.
        */
      OldOverviewMap(uint32 mapID, const char* path);

      /**
        *   Delete this map.
        */
      virtual ~OldOverviewMap();

      /**
        *   Add one map to this overview map.
        *   @param   theMap   The map that should be added.
        *   @param   mapLevel The level of overviewmap that should be 
        *                     created. 0 means underview map 
        *                     (ie. an invalid value), 1 first
        *                     overview map etc.
        *   @return  True if the map is added alright, false
        *            otherwise.
        */
      bool addMap(OldGenericMap* theMap, bool addExternalAndSave = true,
                  byte mapLevel = 1 );

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
        *   @return  The number of OldItems in this map.
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
      inline OldItem* getOverviewItem(uint32 i,
                                   uint32 &overviewID,
                                   uint32 &origMapID,
                                   uint32 &origItemID) const;

      /**
        *   Update external connections from an other map. Does not save
        *   the overview tp disc.
        *
        *   @param   lowerMapNameByID All IDs of maps of which items have
        *                             been added to this overview map.
        *   @param   otherMap The map where to find the data to add to
        *                     this overview map.
        *   @param   mapLevel The level of overviewmap that should be 
        *                     created. 0 means underview map 
        *                     (ie. an invalid value), 1 first
        *                     overview map etc.
        */
      void updateExternalConnectionsFromMap(map<uint32, MC2String>&
                                            lowerMapNameByID,
                                            OldGenericMap* otherMap, 
                                            byte mapLevel);


      /**
       *    Create zip code agglomerations, 2 steps:
       *
       *    Merge zip codes that were split between underviews by adding
       *    same-name zip codes grouping the split zips.
       *    
       *    Then create zip code overviews - create groups to the zip
       *    codes. The group/overview zip name is defined by removeing one 
       *    digit/character from the end of the name of the full-detailed
       *    zip code name, thus creating the hierarchy.
       *    Example in Sweden of the zip code hierarchy:
       *    Zip code 224 is overview to 2240, 2241, 2242, etc
       *    and 2242 is overview of 22420, 22421, 22422, etc.
       */
      void  createZipCodeAgglomerations();

     
      /**
        *   Removes item with localID. 64b not necessary when the
        *   correct map is located.
        *   @param localID          The id of the item to remove.
        *   @param updateHashTable  Default set to true. 
        *                           If the map hashtable should be 
        *                           updated.
        *   @param unusedUkZips Set to tru for not checking groups of all
        *                       items of the map, since the item we remove
        *                       is removed because it is unused as group.
        *                       Don't use this bool for anything else, 
        *                       dangerous.
        */
      virtual bool removeItem(uint32 localID, 
                              bool updateHashTable = true,
                              bool updateAdminCenters = true,
                              bool unusedUkZips = false );

      /**
       *    Method for applying changes to production overview maps all
       *    levels, typically when adding some extra data or changes 
       *    from WASP to the maps.
       *    The affected items could have been removed from or added to 
       *    the otherMap, or simply had some attribute updated. These 
       *    changes are here transferred to the overview map.
       *    @param otherMap            The map from which to apply changes.
       *    @param itemsAffectedInOtherMap   OldItem ids of items that have
       *                                     changed in otherMap.
       *    @param mapLevel            The level of the overview map.
       *    @param overviewMapChanged  Outparam that tells if this overview 
       *                               map was affected by applying changes.
       *    @param changedOverviewIds  Outparam with the overview map ids +
       *                               item ids of overview items that were 
       *                               affected by applying the changes.
       *    @returns 
       */
      bool applyChangesFromOtherMap(OldGenericMap* otherMap, 
            vector<uint32> itemsAffectedInOtherMap, byte mapLevel,
            bool& overviewMapChanged,
            multimap<uint32,uint32>& changedOverviewIds);

      /**
       *    Update the admin area centre table of this overview map with
       *    info from the table in a lower level map. Used in dynamic map
       *    generation.
       *    @param   otherMap    The lower level map.
       *    @param   mapLevel    The level of the overview map to update.
       *    @return  True if the table was updated, false if not.
       */
      bool updateAdminAreaCentres( OldGenericMap* otherMap, byte mapLevel );


      /**
       * Methods handling admin area centre coordinates
       */
      //@{
      /**
       *    Fill the admin area centre table of this overview map with
       *    centre coordinates for built-up areas and municipals 
       *    from the table of a lower level map.
       *    @param   otherMap    The lower level map.
       *    @param   mapLevel    The level of the overview map beeing created.
       *    @return  Number of centre coordinates added to the table.
       */
      uint32 fillAdminAreaCentres( OldGenericMap* otherMap, byte mapLevel );
      //@}


      /**
       *    Adds lane node vectors from otherMap.
       *
       *    @param   otherMap    The lower level map.
       *    @return  The number of lanes added to this map.
       */
      uint32 fillLanes(OldGenericMap* otherMap);

      /**
       *    Adds connecting lanes from otherMap.
       *
       *    @param   otherMap    The lower level map.
       *    @return  The number of connecting lanes added to this map.
       */
      uint32 fillConnectingLanes(OldGenericMap* otherMap);

      
      /** Creates group items in overiview map for items with same type,
       *  same name and close enough to each other. Grouping here means 
       *  creating a new group item with the same name as all items to 
       *  group and setting it as group for all items to group.
       *  
       *  @param itemType     Groups items if this item type.
       *  @param maxMergeDist The distance within itmes need to be located
       *                      in order to get grouped.
       *
       *  @return Returns true if successful, otherwise false. 
       *
       */
      bool groupAdminAreas(ItemTypes::itemType itemType,
                           uint32 maxMergeDist );


      /** Creates group items in overview map for items only having the 
       *  same name and located in a same-name group, 
       *  disregarding type and distance. Grouping here means 
       *  creating a new group item with the same name as all items to 
       *  group and setting it as group for all items to group.
       *
       *  @param itemTypes Only items of these item types are grouped.
       *                   Choose from municipals and BUAs.
       * @return Returns true if successful, otherwise false. 
       */
      bool groupSameNameAdminIndexAreas(set<ItemTypes::itemType> itemTypes);

      /**
       * Removes gfx data from items.
       * @param itemType The gfx data from all items of this type.
       *
       * @return Returns true if successful, otherwise false. 
       */
      bool removeAllGfxData( ItemTypes::itemType itemType );

      const Vector& getContainingMaps() const { return m_containingMaps; }

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
        *   Change the ID of a node from the original OldMapID.nodeID
        *   to the ID in this overview map. The ID of connections 
        *   in the node is also updated.
        *   
        *   @param   node        The node where the ID and the 
        *                        connection IDs should be changed.
        *   @param   trueMapID   The ID of the map where the node
        *                        originally came from.
        *   @param   overviewItemID 
        *                        ID of the node in this overview map.
        *   @return  True if everything is changed properly, false
        *            otherwise.
        */
      bool changeNodeID(OldNode* node, uint32 trueMapID, 
                        uint32 overviewItemID);

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
      bool ssiConnectedToOverviewRoundabout(OldGenericMap* theMap, 
                                const int maxZoomLevelStreetSegmentItems,
                                OldStreetSegmentItem* ssi);

      /**
       *    Help method to addMap for deciding if an item from another map
       *    should be included in this overview map or not.
       *    @param   origItem    The item from the other map.
       *    @param   origMap     The map where otherItem comes from.
       *    @param   byte mapLevel  The level of this overview map.
       *    @return  True if the item should be included in this overview
       *             map, false if not.
       */
      bool toIncludeItem(OldItem* origItem, OldGenericMap* origMap, byte mapLevel);
      
      /**
       *    Help method to addMap.
       *    Adds the specified item from another map to this map.
       *
       *    @param   dataBuffer     Preallocated databuffer that will be
       *                            used temporarily during the adding 
       *                            of the item. The buffer is a parameter
       *                            so a new buffer does not have to be
       *                            allocated for each call.
       *    @param   curItem        The item to add.
       *    @param   currentZoom    The zoomlevel of curItem.
       *    @param   changedItems   Vector containing the id:s of the 
       *                            items added to this map. Will be
       *                            updated. The new item ID will be put 
       *                            last in this vector.
       *    @param   changedItemsOriginalID  Vector containing the original
       *                                     item id:s of the items added 
       *                                     to this map. Will be updated.
       *                                     The original id of the added 
       *                                     item will be put last in this 
       *                                     vector.
       *    @param   theMap         The map that curItem comes from.
       *    @return  The overview item id of the new item if it was 
       *             added succesfully, MAX_UINT32 if it was not added.
       */
      uint32 addOneItem( DataBuffer* dataBuffer,
                         OldItem* curItem,
                         uint32 currentZoom,
                         Vector& changedItems,
                         Vector& changedItemsOriginalID,
                         OldGenericMap* theMap );



      /**
       * Transfers data from the node expansion table in the underview
       * map to the overview map. Only transfers data concerning multi
       * connections, of which both the to node and the from node already
       * have been added to the overview map.
       *
       * If items reffered by nodes in the expand nodes list, i.e. nodes
       * between the from and to node of a multi connection, does not
       * exist in the overview map already, the items are added from the
       * underview map to the overview map.
       *
       * The node IDs in the expand nodes list are updated to overview
       * IDs.
       *
       * @param theMap The underview map, of which to transfer data to 
       *               this overview map.
       * @param dataBuffer 
       */
      void transferNodeExpansionTableToOverviewMap(
             OldGenericMap* theMap, 
             DataBuffer* dataBuffer,
             Vector& changedItems,
             Vector& changedItemsOriginalID );


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
      OldItem* addGroupItem( OldItem* nameItem, 
                          ItemTypes::itemType groupItemType,
                          set<OldItem*>& items);


      /**
       *   Groups items in adminAreas by name.
       *
       *   @param items The items to group. 
       *   @param resultItemsByItemID The items grouped.
       */
      bool mapCommonNames(vector<OldItem*>& items,
                          map< uint32, set<OldItem*> >& resultItemsByItemID);

      
      /**
       * Help methods to createZipCodeAgglomerations
       */
      //@{
      /**
       *    Creates groups for merging zipcodes over undeview map borders.
       *    @return Number of groups created.
       */
      uint32 createZipMergeGroups();

      /**
       *    @return Number of zip overview groups created.
       */
      uint32 createZipOverviews(uint32 nbrCharacters,
                                NationalProperties::zipCodeType_t 
                                zipCodeType );
      //@}
      

      /**
        *   Vector with ID of the maps that is located on this overview.
        */
      Vector m_containingMaps;

      // Only present for location backwards compability.
      // From here -->
      friend class OldItem;
      /**
       *    Temporary table needed 
       *    for location backwardscompability.
       */
      vector<pair<uint32, uint32> > m_tempLocationTable;
      // <-- To here

      /**
       *    Help method when updating names (string indexes) for
       *    a new overview item. All names are removed from the 
       *    newItem, and then added from otherItem to update the
       *    string table in this overview map.
       *    @param   newItem   The overview item to update names for.
       *    @param   otherItem The item from which to copy names.
       *    @param   otherMap  The map where otherItem is located.
       */
      void updateNames(OldItem* newItem, OldItem* otherItem, OldGenericMap* otherMap);
      
      /**
       *    Help method when adding items to this map. Removes all
       *    groups from the new item, except bua and mun groups
       *    belonging to bua and municipal items.
       *    @param   newItem  The new overview item from which unwanted 
       *                      groups should be removed.
       *    @param   otherMap The map from which the new item originates.
       */
      void removeUnwantedGroups(OldItem* newItem, OldGenericMap* otherMap);

      /**
       *    Help method when adding items to this map. Adds bua and
       *    municipal groups to newItem if it is a point of interest item.
       *    The groups are copied from the ssi this poi (other item) 
       *    belongs to in the other map. Note that the group ids will
       *    be ids in the otherMap.
       *    @param   newItem   The overview item (poi) to which bua and 
       *                       municipal groups should be added.
       *    @param   otherItem The item corresponding to the new item.
       *    @param   otherMap  The map where otherItem is located and
       *                       from which newItem originates.
       */
      void addGroupsToPoi(OldItem* newItem,
                          OldItem* otherItem, OldGenericMap* otherMap);

      /**
       *    Help method when adding items to this map. All new items in
       *    a vector are processed and: The nodeids of routeable items
       *    are updated to overview nodeids, as well as the fromNodes
       *    in the connections. Sign post names are updated to have
       *    the correct string index in this map. Finally also
       *    group ids for all new items are updated to overview ids.
       *
       *    @param changeItems   A vector with ids of overview items
       *                         that should have their nodes, conenctions
       *                         and groups updated.
       *    @param otherMap      The map from which the new items originates.
       */
      void updateConnectionsAndGroups(Vector changeItems,
                                      OldGenericMap* otherMap);
     
      /**
       *    Update the sign post names of one connection in this map. 
       *    @param thisFromNodeID  The from node ID of this map.
       *    @param thisToNodeID    The to node ID of this map.
       *    @param otherFromNodeID The from node ID of the other map to copy 
       *                           from.
       *    @param otherToNodeID   The to node ID of the other map to copy 
       *                           from.
       *    @param otherMap        The map to copy the sign post from.
       */
      void updateSignposts(uint32 thisFromNodeID, 
                           uint32 thisToNodeID,
                           uint32 otherFromNodeID, 
                           uint32 otherToNodeID,
                           const OldGenericMap* otherMap);

      /**
       *    Update attributes of an overviewItem from another otherItem.
       *    Typically used when applying changes to overview production 
       *    maps from underview maps or overview maps of a lower level.
       *    This method updates attributes that requires lookup of
       *    true ids in overview/underview, e.g. connections (requiring
       *    fromNodeIds) and groups (from underview to overview group id).
       *    It also updates the names of the overview item and
       *    its gfxData (gfxdata currently only implemented if it is a poi).
       *    
       *    @param overviewItem The overviewItem to update attributes for.
       *    @param otherItem    The item from which to get attribute values.
       *    @param otherMap     The map where otherItem is located.
       */
      bool updateAttributes(OldItem* overviewItem, 
                            OldItem* otherItem, OldGenericMap* otherMap);


      /**
       * Help methods used by groupAdminAreas and
       * groupSameNameAdminIndexAreas
       */
      //@{

      void printItemsToGroup( 
               map< OldItem*, list<OldItem*> > mergeLists ) const;
      
      /**
       * Get next items that, compared with compareItem, have the same 
       * index area order and a group with the exact same name as the
       * group of the compareItem. Special case is 
       * 1) items have the same group as the compareItem
       * 2) compareItem has no group, selects items having no group.
       */
      vector <OldItem*> nextItemsSameIAOrderInSameNameGroup(
               const set<OldItem*>& items, 
               set<OldItem*>::const_iterator& startAtIt,
               const OldItem* compareItem ) const;
      /**
       * Get next items that are close to the compareItem
       * based on the given max distance.
       */
      vector <OldItem*> nextItemsClose( const set<OldItem*>& items, 
                                     set<OldItem*>::const_iterator& startAtIt,
                                     const OldItem* compareItem,
                                     uint32 maxDist  ) const;
      
      void moveToMergeList( const vector<OldItem*>& itemsToAdd, 
                             list<OldItem*>* mergeVector,
                             set<OldItem*>* processedItems) const;

      
      /**
       * Returns the first group of item type itemType of the item, which
       * has a common name with the item. Pays respect to language when 
       * comparing names.
       *
       * @param item     The item to compare names with its groups names.
       * @param itemType The item type of the groups compared with.
       * @returns The group of type itemType, which have a common name with
       *          item.
       */
      OldItem* getGroupWithCommonLangName( OldItem* item, 
                                        ItemTypes::itemType 
                                        itemType ) const;
      /**
       * Find an item to use the name of for the group item to be created.
       * Try to locate a BUA with a municipal with the same name as group 
       * or a municipal with same name with this BUA as group.
       */
      OldItem* findNameItem( OldItem* firstItem,
                             list<OldItem*> itemList ) const;

      //@}

};


// ==================================================================
//                                Implementation of inlined methods =

inline bool
OldOverviewMap::containsMap(uint32 mapID) 
{
   uint32 tmpVal = m_containingMaps.binarySearch(mapID);
   return tmpVal < MAX_UINT32;
}

inline uint32 
OldOverviewMap::getNbrOverviewItems() const
{
   return m_idTranslationTable.getNbrElements();
}

inline OldItem*
OldOverviewMap::getOverviewItem(uint32 i, uint32 &overviewID,
                             uint32 &origMapID, uint32 &origItemID) const
{

   if (m_idTranslationTable.getElement(i, overviewID, origMapID, origItemID)) {
      return itemLookup(overviewID);
   }
   return NULL;
}


#endif


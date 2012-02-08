/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDGENERICMAP_H
#define OLDGENERICMAP_H

#include "config.h"
#include <set>

#include <list>
#include <map>

#include "Vector.h"

#include "OldItemNames.h"

#include <fstream>

#include "OldItem.h"

#include "OldGenericMapHeader.h"
#include "IDTranslationTable.h"

#include "UserRightsItemTable.h"

#include "OldConnection.h"
#include "MC2Coordinate.h"
#include "OldMapHashTable.h"

#include "GMSGfxData.h"

#include "ItemMap.h"
#include "ConnectingLanesTable.h"
#include "SignPostTable.h"
#include "MapBits.h"

// This is the maximum size of a zoomlevel of a map, 
#define MAX_BUFFER_SIZE 250000000
// Max size of a zoomlevel of a super overview map (level 2 overview map)
// (0x90000000 or greater)
#define MAX_BUFFER_SIZE_SUPER_OVERVIEW 450000000
// NB! Keep defines in sync with defines in GenericMap

// Forward declarations.
class DataBuffer;
template <class T> class MC2Allocator;
class OldBusRouteItem;
class GfxData;
class GfxDataFull;
class OldGroupItem;
class OldNode;
class OldNullItem;
class UserRightsMapInfo;
class OldBoundrySegmentsVector;



class AbstractMC2Allocator;
template<class T> class MC2ArrayAllocator;
class ItemIdentifier;

/**
  *   Description of one map in the OldMapCentral-system.
  *   Uses VectorElement as superclass to make it possible to
  *   store the maps in a Vector.
  *
  */
class OldGenericMap : public OldGenericMapHeader {
   /**
    *    @name Friend classes
    *    The classes that might access the private fields.
    */
   //@{
   friend class WriteableRouteMultiConnTable;
   friend class OldExtraDataUtility;
   friend class OldItem;
   friend class OldNode;
   friend class OldRouteableItem;
   friend class OldBoundrySegment;
   friend class OldBoundrySegmentsVector;
   friend class GMSStreetSegmentItem;
   friend class GMSItem;
   friend class GMSNode;
   friend class GMSUtility;
   friend class GfxData;
   friend class GMSGfxData;
   friend class MEItemInfoDialog;
   friend class MapGenUtil;
   friend class M3Creator;
   friend class GMSConnection;
   //@}



public:

   /**
    *    Create a map from file with the correct dynamic qualification.
    *    @param   id    The ID of the map to create from file.
    *    @param   path  The path on the filesystem to the .mcm-file
    *                   that contains the data for the new map.
    *    @return  A new map, created with data from the file at the
    *             given path. NULL is returned upon error.
    */
   static OldGenericMap* createMap(uint32 id, const char* path);

   /**
    *    Create a map from file with the correct dynamic qualification.
    *    @param   mcmName  The name of the map on the filesystem that 
    *                      contains the map.
    *    @return  A new map, created with data from the file at the
    *             given path. NULL is returned upon error.
    */
   static OldGenericMap* createMap(const char* mcmName);


   /**
    *    Returns an approximate map size.
    *    @return An approximate map size in bytes.
    */
   uint32 getApproxMapSize() const;

   /**
    *   @name Modify the region/location of items
    */
   //@{
   /**
    *    Add region to item.
    *    @param   item           The item to add a region to.
    *    @param   region         The region to add.
    *    @param   noLocationRegion [Optional] If the region is
    *                            noLocationRegion = true, 
    *                            the region will be used as group
    *                            but not as location for item.
    *    @return  True if success, false otherwise.
    */
   bool addRegionToItem( OldItem* item, const OldItem* region, 
                         bool noLocationRegion = false);

   /**
    *    Add region to item.
    *    @param   item           The item to add a region to.
    *    @param   regionID       The region id to add.
    *    @param   noLocationRegion [Optional] If the region is
    *                            noLocationRegion = true, 
    *                            the region will be used as group
    *                            but not as location for item.
    *    @return  True if success, false otherwise.
    */
   bool addRegionToItem( OldItem* item, uint32 regionID,
                         bool noLocationRegion = false);

   /**
    *    Replace the regions of one item with the regions used for
    *    another item. First removes all regions for the item, then adds
    *    the ones used for the other item. Possible to only affect regions 
    *    with a certain item type.
    *    @param   item        The item which will have it's regions updated.
    *    @param   modelItem   The item to get the regions from.
    *    @param   regionType  If given, update only the regions with
    *                         this item type. Default is to update all 
    *                         regions.
    *    @return  True if success, false otherwise.
    */
   bool replaceRegions( OldItem* item, OldItem* modelItem,
                     ItemTypes::itemType itemType = 
                           ItemTypes::numberOfItemTypes );
                     
   /**
    *    Copy the regions from one item to another item.
    *    @param   item        The item which will have regions added.
    *    @param   modelItem   The item to get the regions from.
    *    @param   regionType  If given, copy only the regions with
    *                         this item type. Default is to copy all 
    *                         regions.
    *    @return  True if success, false otherwise.
    */
   bool copyRegions( OldItem* item, OldItem* modelItem,
                     ItemTypes::itemType itemType = 
                           ItemTypes::numberOfItemTypes );

   /**
    *    Remove region from item.
    *    @param   item           The item to remove a region from.
    *    @param   regionID       The region to remove.
    *    @return  True if success, false otherwise.
    */
   bool removeRegionFromItem( OldItem* item, OldItem* region );
      
   /**
    *    Remove region from item.
    *    @param   item           The item to remove a region from.
    *    @param   regionID       The region id to remove.
    *    @return  True if success, false otherwise.
    */
   bool removeRegionFromItem( OldItem* item, uint32 regionID );
      
      
   /**
    *    Remove all regions from item. Either all regions may be
    *    removed or all regions of a specific type.
    *    @param   item           The item to remove regions from.
    *    @param   itemType       [Optional] The type of regions to
    *                            remove from the item. If not 
    *                            specified, all regions will be 
    *                            removed.
    *    @return  True if success, false otherwise.
    */
   void clearRegionsForItem( OldItem* item, 
                             ItemTypes::itemType itemType 
                             = ItemTypes::numberOfItemTypes );
   //@}


   /**
    *    @param  group The item to look for in the groups of the other
    *                  items of the map.
    *
    *    @return Returns true if there are at least one item in the map
    *            which has this item as a group.
    *
    */
   bool itemUsedAsGroup( OldItem* group ) const;

      
   /**
    * @param groupID The ID of an item to look for other items being 
    *                a member of the item with ID groupID.
    * @param typeToLookFor If you want only some specific item type, 
    *                else items of all item types are counted.
    * @return A vector with item IDs of items being members of the item
    *         with ID groupID.
    */
   vector<uint32> getItemsInGroup(uint32 groupID, 
      ItemTypes::itemType typeToLookFor = ItemTypes::numberOfItemTypes) const;

   /**
    * Recursive get items in group method. Will collect all items that are
    * members of members of the item with ID groupID.
    * @param groupID The ID of an item to look for other items being 
    *                a member of the item with ID groupID.
    * @return A set with item IDs of items being members of the item
    *         with ID groupID.
    */
   set<uint32> getItemsInGroupRecursive( uint32 groupID ) const;



   /**
    *    Returns true if there exists items of type typeOfItemsToLookFor
    *    in the polyItem. It is not enough to find coordinates on the 
    *    edge of the polygon to return true, there must be at least one
    *    coordinate inside the polygon.
    *
    *    @param polyItem A polygon item to look for items in side in.
    *    @param typeOfItemsToLookFor Only looks for items of this type
    *                                inside the polyItem. Default 
    *                                ItemTypes::numberOfItemTypes means
    *                                items of all item types are looked
    *                                for.
    *    $return True if at least one coordinate of an item of type
    *            typeOfItemsToLookFor are found in the interior of
    *            polyItem.
    */
   uint32 polygonHasItemsInside(OldItem* polyItem, 
                                ItemTypes::itemType 
                                typeOfItemsToLookFor = 
                                ItemTypes::numberOfItemTypes);
 
   /**
    *   @name Get the region/location for one item
    */
   //@{
   /**
    *   Get the region of an item in the map. Will loop over all 
    *   the items the item is member of and return the first one 
    *   of the given type.
    *
    *   @param item The item to get region for.
    *   @param type What kind of region, e.g. bua, mun, city part
    *               or zip.
    *
    *   @return The ID of the first group/item that item is member 
    *           of.
    */
   uint32 getRegionID(const OldItem* item, 
                      ItemTypes::itemType type) const;

   /**
    *   Get the region of an item in the map. Will loop over all 
    *   the items the item is member of and return the first one 
    *   of the given type.
    *
    *   @param item The item to get region for.
    *   @param type What kind of region, e.g. bua, mun, city part
    *               or zip.
    *
    *   @return The first group/item that item is member 
    *           of.
    */
   OldItem* getRegion(const OldItem* item, 
                      ItemTypes::itemType type) const;
   
   /**
    *    Get the citypart id that this item is part of.
    *    @param   item  The item to check.
    *    @return  The citypart id that the item is part of, or
    *             MAX_UINT32 if it wasn't part of any citypart.
    */
   uint32 getCityPartID(const OldItem* item) const;

   /**
    *    Get the number of regions of a specific type.
    *    @param   item  The item.
    *    @param   type  The type.
    *    @return  The number of regions of that itemtype found.
    */
   byte getNbrRegions(const OldItem* item, 
                      ItemTypes::itemType type) const;

   /**
    *    Get regions of a specific type
    *    @param   item  The item.
    *    @param   type  The type.
    *    @param   recursive Set to true if you want to have also 
    *                   the regions of the regions.
    */
   vector<OldItem*> getRegions(const OldItem* item,
                               ItemTypes::itemType type,
                               bool recursive = false) const;
      
   /**
    *    Get a specific region id.
    *    @param   item  The item.
    *    @param   i     The region index.
    *    @param   type  The item type of the region.
    *    @return  The region id.
    */
   uint32 getRegionIDIndex(const OldItem* item, uint32 i,
                           ItemTypes::itemType type) const;
      
   /**
    *    Get a specific region.
    *    @param   item  The item.
    *    @param   i     The region index.
    *    @param   type  The item type of the region.
    *    @return  The region.
    */
   OldItem* getRegionIndex(const OldItem* item, uint32 i,
                           ItemTypes::itemType type) const;

   /**
    *    Check if an item has a specified region.
    *    @param   item     The item.
    *    @param   region   The region to check for.
    *    @return  True if the item has the specified region.
    */
   bool hasRegion( const OldItem* item, 
                   const OldItem* region ) const; 
   //@}

   /**
    *    Type describing a pair of multi nodes.
    *    First uint32 represents the first node id and
    *    second uint32 represents second node id.
    */
   typedef pair<uint32, uint32> multiNodes_t;

   /**
    *    Type describing a list of expanded node ids. 
    */
   typedef list<uint32> expandedNodes_t;
      


   /**
    * Returns pointer to the m_nodeExpansionTable
    */
   inline const map< multiNodes_t, expandedNodes_t>* 
                        getNodeExpansionTable() const;

   /**
    *    Updates the native languages field in the map so that it 
    *    will contain all the languages that are present in the map
    *    as officialName:s.
    *    @return  The number of native languages found.
    */
   uint32 updateNativeLanguages();

   /**  
    *    Changes official names to alternative if not found among
    *    the native languages.
    *    @return The number of itemnames changed.
    */ 
   uint32 updateOfficialNames();


   /**
    * Creates gfx data for items of certain item type from street 
    * segment geometry. Will create bbox
    * from all bbox of all street segment that has the item as group.
    * If the item has a true (exact) gfxdata when entring this method,
    * also that true gfxdata will be boxified.
    * @param itemType Gfx data is created for all items of this type.
    *
    * @return Returns true if successful, otherwise false. 
    */
   bool createGfxDataFromSSIs( ItemTypes::itemType itemType );

   /**
    *   Returns pointer to the next array of connections for
    *   use in the nodes.
    */
   MC2ArrayAllocator<OldConnection*>& getConnectionAllocator();

   /**
    *   Prints the mid- and mif-file version of all items of a
    *   specific item type in this map.
    *   @param  outName   The name of the mid/mif files to be written
    *   @param  type      The OldItemType to write.
    *   @param  writeMifHeader  States whether mif-header should
    *                           be written to the mif file or not.
    */
   void printMidMif( const char* outName, ItemTypes::itemType type,
                     bool writeMifHeader );

   /**
    *    Print the map gfxdata to a mid/mif file. It is useful when 
    *    creating country polygons for new countrys etc. It is possible 
    *    to either print the gfxdata of this map, or print the gfxdatas 
    *    of the municipal items in this map. If a municipal item does 
    *    not have any gfxdata, it is replaced by the convex hull of 
    *    all street segment items in that municipal.
    *    A mif-header is printed to the mif file if requested.
    *
    *    @param fileName   The name of the mid/mif file to be written.
    *                      The mid and mif files are opened for writing
    *                      with append.
    *    @param municipals If set to true the gfxdata of the municipals in
    *                      this map will be printed, if false the map 
    *                      gfxdata will be printed.
    *    @param writeMifHeader If set to true a mif header is
    *                      written to the mif file before printing
    *                      any gfxdata.
    */
   void printMidMifMapGfxData(char* fileName,
                              bool municipals, bool writeMifHeader);

   static bool writeItemMifHeader( ofstream& mifFile,
                                   ItemTypes::itemType headerTypeToWrite );

   /**
    *   Deletes the map.
    */
   virtual ~OldGenericMap();
     
   /**   
    *   Adds one item to this map.
    *
    *   @param   item        The item to be added.
    *   @param   zoomLevel   What zoomlevel the item should have.
    *   @return  ID of the new item, MAX_UINT32 if not added.
    */
   uint32 addItem(OldItem* item, uint32 zoomLevel);
       
   /**
    *   Removes item with localID. 64b not necessary when the
    *   correct map is located.
    *   @param localID          The id of the item to remove.
    *   @param updateHashTable  Default set to true. 
    *                           If the map hashtable should be 
    *                           updated.
    *   @param updateAdminCenters If false, not removing from 
    *                             admin area centers table.
    *                             Collect the item IDs and use
    *                             removeAdminCenters after removal.
    *   @param unusedUkZips Set to true for not checking groups of all
    *                       items of the map, since the item we remove
    *                       is removed because it is unused as group.
    *                       Don't use this bool for anything else, dangerous.
    *   @return Right now, always returns true.
    */
   virtual bool removeItem(uint32 localID,
                           bool updateHashTable = true,
                           bool updateAdminCenters = true,
                           bool unusedUkZips = false);

   /**
    *   A more convenient way to use the quicker removal of many items,
    *   compared to calling removeItem with false and make the extra needed
    *   updates.
    *
    *   @param itemIDs The IDs of the items to remove.
    *   @return Returns true if all items were removed.
    */
   bool removeItems(const set<uint32>& itemIDs);

   /**
    *   Add all items to the hashTable.
    */
   void buildHashTable();


   /**
    *   @name Debugmethods.
    *   These methods are for debugging purposes.
    */
   //@{

   /** 
    *   Prints data about the external connections.
    */
   void dumpExternalConnections(uint32 toMapID = MAX_UINT32);

   /** 
    *   Prints data about all connections (turndescriptions etc.).
    */
   void dumpConnections();

   /**
    *   Prints the landmarks in this map.
    */
   void dumpLandmarks();

   /** 
    *   Prints all names of the specified item to stdout. No endl 
    *   is written.
    */
   void printItemNames(uint32 itemID);

   /**
    *    Print all the names in m_itemNames to stadard out.
    */
   void printAllNames();

   /**
    *    Print all admin area centre coordinates in this map.
    */
   void dumpAdminAreaCentres();
         
   //@}

   /**
    *    @return Returns a vector with all items matching searchStr.
    *            Returns an empty vector if non matched. Only exact matches
    *            are returned.
    */
   vector<OldItem*> searchItemsWithName(MC2String searchStr);


   /**
    *   @name Get item information
    *   Get one specific item or the number of items.
    */
   //@{
   /**
    *   Get the number of items with a given zoomlevel.
    *   @param   z Zoomlevel.
    *   @return  The number of items with zoomlevel z.
    */
   inline uint32 getNbrItemsWithZoom(uint32 z) const;
         
   /**
    *   Get the number of items in this map.
    *   @return The number of items.
    */
   inline uint32 getNbrItems() const; 

   /**
    *   Get one item with a given zoomlevel and given index.
    *   @param   zoom  The zoomlevel of the item to return.
    *   @param   index The cardinal number of the item at 
    *                  zoomlevel zoom.
    *   @return  Pointer to the item that matches the given 
    *            parametervalues. NULL is returned if no such item.
    */
   inline OldItem* getItem(uint32 zoom, uint32 index) const;

   /**
    *  Get an item with the given ID.   
    *  @param   itemID of needed OldItem.
    *  @return  Pointer to item-object with given itemID.
    */
   inline OldItem* itemLookup(uint32 itemID) const;

   /**
    *  Get an node with the given ID.   
    *  @param   nodeID   The ID of the node, this is the same as
    *                    the ID .
    *  @return  Pointer to item-object with given itemID.
    */
   OldNode* nodeLookup(uint32 nodeID) const;

   /**
    *   Returns the user rights for the item with the
    *   supplied ID.
    */
   const MapRights& getRights( uint32 itemID ) const;
   //@}


   /**
    *   @name Get item coordinates
    *   Get coordinates of one item or node.
    */
   //@{
   /**
    *   Used to get the coorinates for a specified node.
    *   @param   nodeID   The ID of the node.
    *   @param   lat      Outparameter for the latitude part of
    *                     the position for node nodeID.
    *   @param   lon      Outparameter for the longitude part of
    *                     the position for node nodeID.
    *   @return  True if the outparamters are set properly, false
    *            otherwise.
    */
   bool getNodeCoordinates(uint32 nodeID, 
                           int32 &lat, int32 &lon) const;

   /**
    *   Used to get the coorinates for an item with a specified
    *   offset. If a pointOfInterestItem is specified, the
    *   coordinates of the corresponding ssi are extracted.
    *   If a street is specified the middle of the street 
    *   is returned since offset is not defined for a street.
    *
    *   @param   itemID   The ID of the item.
    *   @param   offset   The relative distance from node 0.
    *   @param   lat      Outparameter for the latitude part of
    *                     the position for node nodeID.
    *   @param   lon      Outparameter for the longitude part of
    *                     the position for node nodeID.
    *   @return  True if the outparamters are set properly, false
    *            otherwise.
    */
   bool getItemCoordinates(uint32 itemID, uint16 offset, 
                           int32 &lat, int32 & lon) const;

   /**
    *    Returns one coordinate suitable for locating the item
    *    in e.g. searches.
    *    @return One coordinate.
    */
   MC2Coordinate getOneGoodCoordinate(const OldItem* item) const;
         
   /**
    *    Sets the coordinate to one coordinate suitable for
    *    locating the item in e.g. searches.
    *    @param coord Coordinate to change into good one.
    */
   void getOneGoodCoordinate(MC2Coordinate& coord,
                             const OldItem* item) const;

   /**
    *    Returns a bounding box for the item.
    *    @param bbox The bounding box to fill in.
    *    @param item The item.
    */
   void getItemBoundingBox(MC2BoundingBox& bbox,
                           const OldItem* item) const;

   /**
    *    Returns a bounding box for the item.
    *    @param item The item to get the bbox for.
    *    @return The bounding box.
    */
   MC2BoundingBox getItemBoundingBox(const OldItem* item) const;

   /**
    *    Get the offset for the point on a the item, with given ID,
    *    that is closest to a specified coordinate.
    *
    *    @param itemID  The ID of the item that offset will be on.
    *    @param lat     The latitude part of the coordinate that
    *                   the offset should be close to.
    *    @param lon     The longitude part of the coordinate that
    *                   the offset should be close to.
    *    @return The offset at item with itemID that is closest to
    *            the given coordinate (lat, lon). NB! A negative value 
    *            is returned upon failure.
    */
   int32 getItemOffset(uint32 itemID, int32 lat, int32 lon) const;

   /**
    *    Get the offset for the point on a given item that is 
    *    closest to a specified coordinate.
    *    @param item    The item that offset will be on.
    *    @param lat     The latitude part of the coordinate that
    *                   the offset should be close to.
    *    @param lon     The longitude part of the coordinate that
    *                   the offset should be close to.
    *    @return The offset at item with itemID that is closest to
    *            the given coordinate (lat, lon). NB! A negative value 
    *            is returned upon failure.
    */
   int32 getItemOffset(OldItem* item, int32 lat, int32 lon) const;
   //@}
      
   /**
    *   @name Bind a group to an item.
    */
   //@{
   /**
    *   Bind a group to an item when the ID:s are known.
    *   @param   itemID   The identification number of the item
    *                     that is to be added to the group
    *   @param   groupID  The identification number of the group
    *                     to which the item is to be added
    *   @return  True if successful, false otherwise
    */
   bool bindItemToGroup(uint32 itemID, uint32 groupID);

   /**
    *   Bind a group to an item when the items are known.
    *   @param   item  The item that is to be added to the group.
    *   @param   group The group to which the item is to be added.
    *   @return  True if successful, false otherwise
    */      
   bool bindItemToGroup(OldItem* item, OldGroupItem* group);

   bool removeItemFromGroup(uint32 itemID, uint32 groupID);
   bool removeItemFromGroup(OldItem* item, OldGroupItem* group);
   //@}
   
   /**
    *   @name Add a name to an item.
    */
   //@{
   /**
    *   Add a name to an item when the ID is known.
    *   If the item is known, use the other method,
    *   it is more efficient.
    *
    *   @param   itemID   The identification number of the item
    *                     to which the name should be added.
    *   @param   newName  The name that is to be added to the
    *                     item.
    *   @param   lang     The language of the new item.
    *   @param   type     The type of the new item.
    *   @return  The offset in the itemNames-vector. MAX_UINT32 is
    *            returned upon error.
    */
   uint32 addNameToItem(uint32 itemID, const char* name,
                        LangTypes::language_t lang, 
                        ItemTypes::name_t type);

   /**
    *   Add a name to an item when the item is known.
    *
    *   @param   item     The item to which the name should be added.
    *   @param   newName  The name that is to be added to the
    *                     item.
    *   @param   lang     The language of the new item.
    *   @param   type     The type of the new item.
    *   @return  The offset in the itemNames-vector. MAX_UINT32 is
    *            returned upon error.
    */
   uint32 addNameToItem(OldItem* item, const char* name, 
                        LangTypes::language_t lang, 
                        ItemTypes::name_t type);
         
         
   /**
    *  If the item is a ssi any number name will have their
    *  language changes to represent name priority
    *  @param item The item to modify.
    *  @return True if ssi and numbernames had no priorities.
    */
   bool modifyItemNumberName(OldItem* item);
         
   //@}

   /**
    *   Add a name to the map.
    *   @param   name     The name that is to be added to the
    *                     map.
    *   @return  The offset in the itemNames-vector. MAX_UINT32 is
    *            return upon error.
    */
   uint32 addName(const char* name,
                      LangTypes::language_t lang, 
                      ItemTypes::name_t type);

   /**
    *   Get the graphical representation of the map.
    *   
    *   @return Pointer to the gfxData of this map.
    */
   inline const GMSGfxData* getGfxData() const;
   inline GMSGfxData* getGfxData();

   /**
    *   Set the graphical representation of the map.
    */
   inline void setGfxData( GfxDataFull* mapGfx );

   /**
    *    Returns a gfxData for an OldItem. If the OldItem is a poi
    *    then the supplied GfxDataFull will be filled in with
    *    the coordinate of the poi and a pointer to it will be
    *    returned.
    */
   const GfxData* getItemGfx( const OldItem* item, GfxDataFull& gfx ) const;
      
   /**
    *    Get the bounding box of this map. This is not the same thing 
    *    as getting the bounding box of the GfxData for the map, 
    *    since this method also will consider lakes etc. that reach 
    *    outside the real map boundry.
    *
    *    The implementation is linear in the number of lakes in the
    *    map!
    *    
    *    @param bbox Outparameter that will be set to the bounding box
    *                of this map.
    */
   void getMapBoundingBox(MC2BoundingBox& bbox) const;

   /**
    *   Get the hashtable for this map. The hashtable is hashed by
    *   the geograpical position of the items.<br />
    *   @return  Pointer to the hashTable of this map.
    */
   inline OldMapHashTable* getHashTable() const;

   /**
    *    Puts the items within the radius in the supplied set.
    *    @param resultItems  The result is put here.
    *    @param center       Center of the circle.
    *    @param radiusMeters Radius in meters of the circle.
    *    @param allowedTypes Set of allowed item types of the returned items.
    */
   void
   getItemsWithinRadiusMeter(set<OldItem*>& resultItems,
                             const MC2Coordinate& center,
                             int radiusMeters,
                             const set<ItemTypes::itemType>&
                             allowedTypes,
                             const UserRightsMapInfo* rights = NULL);

   /**
    *    Puts the items within the bounding box in the supplied set.
    *    @param resultItems  The result is put here.
    *    @param bbox         Bounding box to look inside.
    *    @param allowedTypes Set of allowed types of the returned items.
    */
   void getItemsWithinBBox(set<OldItem*>& resultItems,
                           const MC2BoundingBox& bbox,
                           const set<ItemTypes::itemType>& allowedTypes,
                           const UserRightsMapInfo* rights = NULL);
   
   /**
    *    Puts the items within the gfxData in the supplied set.
    *    @param resultItems  The result is put here.
    *    @param gfxData      GfxData to check inside.    
    *    @param allowedTypes Set of allowed types of the returned items.
    */
   void getItemsWithinGfxData(set<OldItem*>& resultItems,
                              const GfxData* gfxData,
                              const set<ItemTypes::itemType>& allowedTypes,
                              const UserRightsMapInfo* rights = NULL);

   /**
    *    Puts the items within the radius in the supplied set.
    *    @param resultIDs    The result is put here.
    *    @param center       Center of the circle.
    *    @param radiusMeters Radius in meters of the circle.
    *    @param allowedTypes Set of allowed item types of the returned items.
    */
   void getIDsWithinRadiusMeter(set<uint32>& resultIDs,
                                const MC2Coordinate& center,
                                int radiusMeters,
                                const set<ItemTypes::itemType>&
                                allowedTypes,
                                const UserRightsMapInfo* rights = NULL);

   /**
    *    Puts the items within the bounding box in the supplied set.
    *    @param resultIDs    The result is put here.
    *    @param bbox         Bounding box to look inside.
    *    @param allowedTypes Set of allowed types of the returned items.
    */
   void getIDsWithinBBox(set<uint32>& resultIDs,
                         const MC2BoundingBox& bbox,
                         const set<ItemTypes::itemType>& allowedTypes,
                         const UserRightsMapInfo* rights = NULL );

   /**
    *    Puts the ids within the gfxData in the supplied set.
    *    @param resultIDs    The result is put here.
    *    @param gfxData      GfxData to check inside.    
    *    @param allowedTypes Set of allowed types of the returned items.
    */
   void getIDsWithinGfxData(set<uint32>& resultIDs,
                            const GfxData* gfxData,
                            const set<ItemTypes::itemType>& allowedTypes,
                            const UserRightsMapInfo* rights = NULL );
   
   /**
    *    Returns the item closest to the coordinate <code>coord</code>.
    *    @param coord Coordinate to use.
    *    @param dist  The distance is put here. Squared mc2 units.
    *    @return The closest item id found.
    */
   uint32 getClosestItemID( const MC2Coordinate& coord,
                            uint64& dist,
                            const set<ItemTypes::itemType>& allowedTypes,
                            const UserRightsMapInfo* rights = NULL );

   /**
    *    Returns true if the item is allowed according to the
    *    rights.
    */
   bool itemAllowedByUserRights( uint32 id,
                                 const UserRightsMapInfo& rights ) const;
      
   /**
    *    Returns true if the item is allowed according to the
    *    rights.
    */
   bool itemAllowedByUserRights( const OldItem& item,
                                 const UserRightsMapInfo& rights ) const;

   /**
    *    Returns the copyright string for an item.
    *    Uses the UserRights for the item and if no good match is found
    *    the copyright is taken from the map.
    *    Not completely implemented yet.
    */
   MC2String getNameOfNeededRight( const OldItem& item,
                                   LangTypes::language_t lang ) const;
   
   /**
    *    Returns the copyright string for an item.
    *    Uses the UserRights for the item and if no good match is found
    *    the copyright is taken from the map.
    *    Not completely implemented yet.
    */
   MC2String getNameOfNeededRight( uint32 id,
                                   LangTypes::language_t lang ) const;
                                        
      
   /**
    *   @name Get the name of a given OldItem.
    */
   //@{
   /**
    *   Get the first name as string of a given OldItem.
    *   @param   item  The item whos name will be returned.
    *   @return  Pointer to the name of item. <i>@b NB! This name
    *            must not be deleted.</i>
    */
   inline const char* getFirstItemName(const OldItem* item) const;

   /**
    *   Get the first name as string of a given OldItem.
    *   @param   itemID   The ID of the item whos name will be 
    *                     returned.
    *   @return  Pointer to the name of item. <i> @b NB! This name
    *            must not be deleted.</i>
    */
   inline const char* getFirstItemName(uint32 itemID) const;

   /**
    *   Get a name of an item where the name is of a specific type 
    *   and language.
    *
    *   @param   item  The item whose name will be returned.
    *   @param   strLang  The language of the string. If
    *                     "invalidLanguage" is sent as parameter
    *                     the language of the returned string is 
    *                     undefined.
    *   @param   strType  The type of the string. If
    *                     "invalidName" is sent as parameter
    *                     the type of the returned string is 
    *                     undefined.
    *   @return  The FIRST name that fits the specified type and 
    *            language or NULL if no match.
    */
   inline const char* getItemName( OldItem* item,
                                   LangTypes::language_t strLang,
                                   ItemTypes::name_t strType ) const;

   /**
    *   Get a name of an item where the name is of a specific type 
    *   and language.
    *
    *   @param   itemID   The ID of the item whose name will be 
    *                     returned.
    *   @param   strLang  The language of the string. If
    *                     "invalidLanguage" is sent as parameter
    *                     the language of the returned string is 
    *                     undefined.
    *   @param   strType  The type of the string. If
    *                     "invalidName" is sent as parameter
    *                     the type of the returned string is 
    *                     undefined.
    *   @return  The FIRST name that fits the specified type and 
    *            language or NULL if no match.
    */
   inline const char* getItemName( uint32 itemID,
                                   LangTypes::language_t strLang,
                                   ItemTypes::name_t strType ) const;

   /**
    *    Check if an item has a name that matches a name string 
    *    of specific type and language. If it does, true is 
    *    returned and the string index of the name is given 
    *    via outparam.
    *    @param   item     The item for which to check for the name.
    *    @param   strLang  The language of the string. If
    *                      "invalidLanguage" is sent as parameter
    *                      the language of the returned string is 
    *                      undefined.
    *    @param   strType  The type of the string. If
    *                      "invalidName" is sent as parameter
    *                      the type of the returned string is 
    *                      undefined.
    *    @param   nameStr  The name string to match.
    *    @param   strIdx   Outparam, that is set to the string index
    *                      if the item has the searched for name.
    *    @return  True if the item has a name that matches the searched
    *             for name string, false if not.
    */
   bool itemHasNameAndType(OldItem* item,
                           LangTypes::language_t strLang,
                           ItemTypes::name_t strType,
                           const char* nameStr,
                           uint32& strIdx) const;
         
   /**
    *    Will look for the best name for the item.
    *    See NameUtility.
    *
    *    @param item    The item to get the best name for.
    *    @param strLang The language of the name to return. If not
    *                   found in this language, it is replaced in
    *                   the order in the list.
    *    @return A pointer to "the best" name of <tt>item>/tt> (must 
    *            not be deleted. NULL is returned if no such name.
    */
   inline const char* getBestItemName( uint32 itemID ) const;
   inline const char* getBestItemName(
                                      uint32 itemID, LangTypes::language_t strLang) const;

   inline const char* getBestItemName(uint32 itemID, 
                                      vector<LangTypes::language_t> &strLang) const;


   inline const char* getBestItemName(
                                      const OldItem* item, LangTypes::language_t strLang) const;

   inline const char* getBestItemName(
                                      const OldItem* item, 
                                      vector<LangTypes::language_t> &strLang) const;
         
   /**
    *    Will look for a name in <tt>item</tt> with <tt>strType</tt>
    *    in language <tt>strLang</tt>. The returned name will always 
    *    have <tt>strType</tt> as type, but the language are 
    *    selected in this order:
    *    <ol>
    *       <li><tt>strLang</tt></li>
    *       <li>English</li>
    *       <li>Any other language</li>
    *    </ol>
    *    @param item    The item to get the best name for.
    *    @param strType The type of the name to return.
    *    @param strLang The language of the name to return. If not
    *                   found in this language, it is replaced in
    *                   the order in the list.
    *    @return A pointer to a name with <tt>strType</tt> (must not
    *            be deleted. NULL is returned if no such name.
    */
   inline const char* getBestItemName(
                                      const OldItem* item, 
                                      ItemTypes::name_t strType,
                                      LangTypes::language_t strLang) const;

   inline const char* getBestItemName(
                                      const OldItem* item, 
                                      ItemTypes::name_t strType,
                                      vector<LangTypes::language_t> &strLang) const;


   /**
    *   Get a specified name from the stringtable.
    *   @param   stringIndex String index of the string that will
    *                        be returned
    *   @return  Pointer to string number stingIndex, NULL is 
    *            returned upon error. <i> @b NB! This name must not 
    *            be deleted.</i>
    */
   inline const char* getName(uint32 stringIndex) const;

   /**
    *    Returns the name specified by the raw index.
    *    Maybe this function could replace getName, since
    *    GET_STRING_INDEX(GET_STRING_INDEX(stringIndex)) ==
    *    GET_STRING_INDEX(stringIndex).
    *    @param  rawIndex The raw string index.
    *    @return Pointer to the name or NULL if the index
    *            was invalid.
    */
   inline const char* getRawName(uint32 rawIndex) const;
         
   /**
    *    Get the total number of names for the items in this map.
    *    @return  The number of names for the items.
    */
   uint32 getTotalNbrItemNames() const;
   //@}
      
   /**
    *   @name Name comparison between items.
    */
   //@{
   /**
    *    Checks if at least one name is same regardless of case for
    *    two items. Does not count synonym names as names.
    *    @param   item        One item.
    *    @param   otherItem   Another item.
    *    @return  True if at least one name was same, 
    *             regardless of case.
    */
   bool oneNameSameCase( const OldItem* item, 
                         const OldItem* otherItem ) const;
      
   /**
    *    Checks if at least one word of one name is same regardless 
    *    of case for two items. Does not count synonym names as names.
    *    @param   item        One item.
    *    @param   otherItem   Another item.
    *    @return  True if at least one word of one name was same, 
    *             regardless of case.
    */
   bool oneNameSimilarCase( const OldItem* item, 
                            const OldItem* otherItem ) const;
   //@}

   /**
    *   Get the boundry segments vector (holds external connections)
    *   of this map.
    *   @return  Pointer to the vector with the boundry segments.
    */
   inline OldBoundrySegmentsVector* getBoundrySegments() const;
      
   /**
    *    If the boundry segments vector is NULL, create a new one.
    */
   void createOldBoundrySegmentsVector();

   /**
    *   Checks if the supplied item is part of a street or not.
    *   @param   item  The item to check if it's part of a street.
    *   @return  True if the item was part of a street, 
    *            false otherwise.
    */
   bool isPartOfStreet(const OldItem* item) const;
      
   /**
    *   Checks if the supplied item is part of a street with gfxdata 
    *   or not.
    *   @param   item  The item to check if it's part of a street with
    *                  gfxdata..
    *   @return  True if the item was part of a street with gfxdata, 
    *            false otherwise.
    */
   bool isPartOfGfxStreet(const OldItem* item) const;

   /**
    *    Counts the number of streets the specified item is part of.
    *    @param   item  The item.
    *    @return  The number of streets the item is part of.
    */
   uint32 partOfNbrStreets(const OldItem* item) const;
      
   /**
    * Reorders connecting lanes and switches order of the lane data
    * of nodes indicating that the direction of the connection is in 
    * negative direction compared to the direction of the SSI.
    */
   void reorderConnectingLanes(const set<uint32>& itemIDs);
   
   /**
    * Removes duplicated sing posts, and checks that no holes exists in the 
    * sing post vectors.
    */
   void signPostShapeUp();

      
   /**
    *   Add one item to the database from a Databuffer*.
    *
    *   @param   dataBuffer        The databuffer that contains all 
    *                              information about the new item.
    *   @param   currentItemtype   The type of the item.
    *   @param   currentItemID     The ID of the new item. If this
    *                              is MAX_UINT32 then the item is
    *                              added last and the new ID is
    *                              returned.
    *   @return  ID of the added item. MAX_UINT32 is returned upon
    *            error.
    */
   uint32 addItemToDatabase(  DataBuffer* dataBuffer,
                              /*byte currentItemType,
                                uint32 currentItemID,*/
                              byte zoomlevel = MAX_BYTE,
                              bool asignNewItemID = false);
      
   /**
    *   Get the memory usage for this object (including all the 
    *   child-objects).
    *
    *   @return The total memory usage for the map.
    */
   virtual uint32 getMemoryUsage() const;

      
   /**
    *   Find the node ids that has connections coming FROM
    *   the node with id fromNodeID
    *   @param   fromNodeID  The node id that the connections
    *                        should start from.
    *   @param   toNodeIDVec Outparamater: vector of the node ids
    *                        that has connections coming from
    *                        the specified node. This vector should
    *                        be created by the caller of this method.
    *   @return  True if the fromNodeID was a real node id, false
    *            otherwise.
    */
   bool getConnectToNodeIDs(uint32 fromNodeID,
                            Vector* toNodeIDVec);
     
   /**
    *   Find the opposing (reversed) connection given
    *   a connection and the node where it is attached to.
    *   @param   conn     The connection in question.
    *   @param   toNodeID The node id where conn is connected
    *                     to.
    *   @return  The opposing (reversed) connection if found,
    *            otherwise NULL (can only occur if the wrong
    *            toNodeID is supplied).
    */
   OldConnection* getOpposingConnection(OldConnection* conn, 
                                        uint32 toNodeID);
      
   /**
    *   Find the other connections from the from_node of the
    *   specified connection.
    *   @param   conn     The connection in question.
    *   @param   toNodeID The node id where conn is connected
    *                     to.
    *   @param   otherConn Out parameter. Will be filled with the 
    *                      other connections and nodes where the
    *                      connections are connected to (to nodes).
    *   @return  false if toNodeID is not a valid node or conn is NULL.
    */
   bool getOtherConnections( OldConnection* conn, 
                             uint32 toNodeID,
                             vector<pair<OldConnection*,OldNode*> >& otherConn );
      
   /**
    *   Get the (first) connection between two routeable items.
    *   @param   fromItemID  Id of routeable item from where
    *                        the connection is coming FROM.
    *   @param   toItemID    Id of routeable item from where
    *                        the connection is leading TO.
    *   @param   conn        Out paramater. The connection pointer
    *                        will be set to point on the connection if
    *                        found, otherwise set to NULL.
    *   @return  The "to node" of the connection if found, 
    *            otherwise MAX_UINT32.
    */
   uint32 getConnection(uint32 fromItemID, uint32 toItemID, 
                        OldConnection*& conn) const;
      
   /**
    *    @name Methods for getting connection costs.
    */
   //@{
      
   /**
    *    Calculates the length and time of the connection.
    *
    *    @param   conn     The connection to be examined.
    *    @param   toNode   The node that the connection is attached
    *                      to. (Ie. the node that the connection is
    *                      leading to.)
    *    @param   time     Outparameter that is set to the time
    *                      to traverse the connection.
    *    @param   standStillTime Outparameter that is set to the 
    *                            standstill time to traverse the
    *                            connection.
    *    @param   length   Outparameter that is set to the 
    *                      length of the connection in meters.
    *    @param   externalConnection
    *                      If this is set to true, then it signals
    *                      that the supplied connection is an
    *                      external connection. Since it is not
    *                      possible to find the segment that it
    *                      is coming from (is in "another" map),
    *                      it is assumed that the properties of the
    *                      "fromSegment" can be approximated with the
    *                      properties of the "toSegment", (even if
    *                      we know that it is not correct). This
    *                      parameter defaults to false.
    *    @return  True if the outparameters are set correctly,
    *             false otherwise.
    */
   bool getConnectionData( OldConnection* conn, OldNode* toNode,
                           float64& time, 
                           float64& standStillTime,
                           float64& length,
                           bool externalConnection = false ) const;
         
   /**
    *    Calculates the costs "on the fly". This means that the costs
    *    etc don't have to be stored in the connectinos, and thus
    *    less memory usages!
    *    @param   conn     The connection to be examined.
    *    @param   toNode   The node that the connection is attached
    *                      to. (Ie. the node that the connection is
    *                      leading to.)
    *    @param   costA    Outparameter that is set to the 
    *                      "distanceCost".
    *    @param   costB    Outparameter that is set to the 
    *                      "timeCost".
    *    @param   costC    Outparameter that is set to the third cost
    *                      (currently not defined).
    *    @param   costD    Outparameter that is set to the fourth 
    *                      cost (currently not defined).
    *    @param   externalConnection
    *                      If this is set to true, then it signals
    *                      that the supplied connection is an
    *                      external connection. Since it is not
    *                      possible to find the segment that it
    *                      is coming from (is in "another" map),
    *                      it is assumed that the properties of the
    *                      "fromSegment" can be approximated with the
    *                      properties of the "toSegment", (even if
    *                      we know that it is not correct). This
    *                      parameter defaults to false.
    *    @return  True if the outparameters are set correctly,
    *             false otherwise.
    */
   bool getConnectionCost(OldConnection* conn, OldNode* toNode,
                          uint32& costA, uint32& costB,
                          uint32& costC, uint32& costD,
                          bool externalConnection = false) const;
       
   //@}
         
   /**
    *    @name Idenfify a node/item by coordinates.
    *    Some methods to get unique coordinates for an item or node,
    *    and to get the node/item from coordinates.
    */
   //@{
   /**
    *    Find a node from two coordinates. The first coordinate
    *    should be the of the node and the second on the street
    *    segment. The second coordinate should be located on the
    *    correct half of the segment (e.g. if the first coordinate
    *    represents node 0, the second coordinate should be on the
    *    first half of the segment).
    *    @param lat1 The latitude part of the first coordinate.
    *    @param lon1 The longitude part of the first coordinate.
    *    @param lat2 The latitude part of the second coordinate.
    *    @param lon2 The longitude part of the second coordinate.
    *    @return The node that was found, NULL will be returned if
    *            no node is found.
    */
   OldNode* getNodeFromCoordinates(int32 lat1, int32 lon1,
                                   int32 lat2, int32 lon2);

   /**
    *    Get two coordinates that represents one node. The coordinates
    *    that are returned (via outparameters) will match the criterias
    *    for the getNodeFromCoordinate-method. This method contains a
    *    self-check that means that it will only return true if the
    *    reverse-lookup (by using getNodeFromCoordinates) returns the
    *    correct node.
    *    @param node The node to get a coordinate representation for.
    *    @param lat1 Outparameter that is set to the latitude of the 
    *                node.
    *    @param lon1 Outparameter that is set to the longitude of the 
    *                node.
    *    @param lat2 Outparameter that is set to the latitude of the 
    *                ssi where the node is located.
    *    @param lon2 Outparameter that is set to the longitude of the 
    *                ssi where the node is located.
    *    @return True if the coordinates are set and if a reverse-lookup
    *            returnes the correct node.
    */
   bool getCoordinatesFromNode(const OldNode* node,
                               int32 &lat1, int32 &lon1,
                               int32 &lat2, int32 &lon2);

   /**
    *    @param type       The item type.
    *    @param lat        Latitude.
    *    @param lon        Longitude.
    *    @param closeItems Vector in which will be stored all items
    *                      that are found close to the coordinate given,
    *                      (e.g. more than one poi at one coordinate).
    *    @param itemName   The name of the item. If a name is specified
    *                      the item candidates will be checked for this
    *                      name. If no name is given the closest item
    *                      will be returned.
    *    @param r          The distance in meters from (lat,lon) for the 
    *                      candidates. If the radius is negative, the
    *                      closest item to (lat,lon) is used. Default
    *                      is 5 meters.
    *    @param poiType    The point of interest type (if the item is a
    *                      poi).             
    */
   OldItem* getItemFromCoordinate(ItemTypes::itemType, 
                                  int32 lat, int32 lon,
                                  vector<OldItem*>& closeItems,
                                  const char* itemName = NULL,
                                  const int r = 5,
                                  ItemTypes::pointOfInterest_t poiType = 
                                  ItemTypes::nbr_pointOfInterest);


   bool coordExistInMap( MC2Coordinate coord );

   bool getCoordinateFromItem(const OldItem* item, 
                              int32 &lat, int32 &lon,
                              byte &nameOffset,
                              bool printLog = true);
   //@}

   /**
    *    Create all allocators that are used to store the OldItems, 
    *    OldConnections, OldNodes and GfxDatas.
    */
   virtual void createAllocators();

   /**
    *    Initializes the non item allocators.
    *    @param   nbrGfxDatas                nbrGfxDatas.
    *    @param   nbrGfxDataSingleSmallPoly  nbrGfxDataSingleSmallPoly.
    *    @param   nbrGfxDataSingleLine       nbrGfxDataSingleLine.
    *    @param   nbrGfxDataSinglePoint      nbrGfxDataSinglePoint.
    *    @param   nbrGfxDataMultiplePoints   nbrGfxDataMultiplePoints.
    *    @param   nbrNodes                   nbrNodes.
    *    @param   nbrConnections             nbrConnections.
    */
   virtual void initNonItemAllocators( uint32 nbrGfxDatas, 
                                       uint32 nbrGfxDataSingleSmallPoly,
                                       uint32 nbrGfxDataSingleLine, 
                                       uint32 nbrGfxDataSinglePoint,
                                       uint32 nbrGfxDataMultiplePoints, 
                                       uint32 nbrNodes, 
                                       uint32 nbrConnections );

   /**
    *   Save one item to a databuffer.
    *   @param   dataBuffer  Where the item is saved.
    *   @param   item        The item to save into dataBuffer.
    *   @return  True if item is saved alright, false otherwise.
    */
   bool saveOneItem(DataBuffer* dataBuffer, OldItem* item);

   /**
    *   The type of the landmark table.
    */
   typedef multimap<uint64, ItemTypes::lmdescription_t> landmarkTable_t;
      
   /**
    *   Get the landmark table of this map.
    */
   inline landmarkTable_t& getLandmarkTable();
   inline const landmarkTable_t& getLandmarkTable() const;

   /**
    *    Find an unique item described by the specified ItemIdentifier.
    *    In case a unique item can't be found, NULL is returned.
    *    @param   ident The ItemIdentifier.
    *    @return  The item uniquely identified by the ItemIdentifier or 
    *             NULL.
    */
   const OldItem* getItemFromItemIdentifier( 
                                         const ItemIdentifier& ident ) const;
            
   /**
    *    Set the specified ItemIdentifier to uniquely identify
    *    the item. The method may fail doing this so make sure you
    *    check the return value.
    *    @param   item  The item to uniquely identify.
    *    @param   ident Output parameter. The ItemIdentifier that
    *                   will be set to identify the item.
    *    @return  If the ItemIdentifier was set to uniquely identify
    *             the item.
    */
   bool getItemIdentifierForItem( const OldItem* item, 
                                  ItemIdentifier& ident ) const;

      
   /**
    *    Expand node ids. Expand the list of node ids (route)
    *    so that any multi connections will be expanded.
    *    @param   nodeIDs  In/out parameter. List of node ids that
    *                      will also include the expanded node ids.
    *    @return  The size of the resulting nodeIDs list.
    */
   uint32 expandNodeIDs( list<uint32>& nodeIDs ) const;
      
   /**
    *    Check if the specified item is a city center poi that is unique
    *    in the sense that it doesn't belong to a builtup area with
    *    the same name as the city centre.
    *    @param   item  The item to check. May be NULL and may not be
    *                   a poi of type citycentre.
    *    @return  True if the poi is a "unique" citycentre, otherwise
    *             false.
    */
   bool isUniqueCityCentre( OldItem* item );

   /**
    *    Returns a reference to the rights table. Should not
    *    be used directly, only be SearchUnitBuilder etc.
    */
   const UserRightsItemTable& getRightsTable() const {
      return *m_userRightsTable;
   }


   /**
    *    Set all user right items in the map. I.e. replaces existing
    *    user rights with the ones in itemMap.
    *
    *    @param itemMap OldMaps item ID to user right of that item for all
    *                   items in the map with rights.
    */
   void setUserRights(UserRightsItemTable::itemMap_t& itemMap);

   /**
    *     Get the user rights of this map.
    *
    *     @param itemMap Outparameter Is set to all rights of this map.
    *     @return Nbr of items in itemMap.
    */
   int getUserRights(UserRightsItemTable::itemMap_t& itemMap);
      
   const uint32 *getItemsZoomAllocated() const { return m_itemsZoomAllocated; }
   const uint32 *getItemsZoomSize() const { return m_itemsZoomSize; }
   const OldBoundrySegmentsVector *getSegmentsOnTheBoundry() const { return m_segmentsOnTheBoundry; }


   /**
    * @name Admin area centres.
    */
   //@{
   /**
    *    The type of the admin area centre coordinates table.
    */
   struct adminAreaCentre_t {
      uint32 itemID;
      MC2Coordinate centre;
   };

   /**
    *    The type of the admin area centre coordinates table in map
    *    generation.
    *    Key:   item ID
    *    Value: The items center coordinage. (Logical center, and 
    *           possibly also gemetrical).
    */
   typedef map<uint32, MC2Coordinate> adminAreaCentreTable_t;

      
   /**
    *    In this method is defined which item type that should be
    *    handled when creating/updating admin area centres from 
    *    the content of this map.
    */
   vector<ItemTypes::itemType> getItemTypesToCreateAdminCentres();

   /**
    *    @param The ID of the item to try to find an entry in admin
    *           area center table for.
    *    @return The coordinate stored in the admin area center table,
    *            or an invalid coordinate if not found. 
    */
   MC2Coordinate getCentreFromAdminTable(uint32 itemID) const;

   /**
    *    @param The ID of the item to check if it has a centre coord 
    *           in the admin area center table.
    *    @return True if the item has a centre in the table, false if not.
    */
   bool hasAdminAreaCentre(uint32 itemID) const;


   /**
    *    Removes the items with IDs in itemIDs from the admin area
    *    center table. When removing many items it is faster to use
    *    this method for all items at the same time compared to letting
    *    remove item do the work.
    *
    *    @param itemIDs IDs of the items to remove from admin area 
    *           center table.
    */
   void removeAdminCenters(const set<uint32>& itemIDs);
   
   /**
    *    Adds item ID and center coordinage to the ones stored in the map.
    *    I.e. not replacing all centers like setAminAreaCentres does.
    *
    *    @param areaCenters The admin area centers to add to the map. If
    *                       it contains an item ID already included in
    *                       admin area centers of the map, the center of
    *                       the map is overwritten.
    */
   void addAdminCenters(adminAreaCentreTable_t& areaCenters);

   /**
    *    Set (replace) the admin area centre table in this mcm map.
    *    @param   adminCentres   A map with admin area centre coordinates.
    */
   void setAminAreaCentres( const adminAreaCentreTable_t& adminCentres );

   /**
    *    Get a map with the admin area centres in this mcm map.
    */
   adminAreaCentreTable_t getAdminAreaCentres() const;

   /**
    *    Create an adminAreaCentres table (map).
    *    The items with adminType will be given a centre coordinate
    *    using city centre coordinates, when the name of the admin area
    *    and the city centre is equal and the cc is located in the admin
    *    area. Must only be called for underview maps.
    *    Will re-use centre coords for admin areas that are index areas 
    *    that already has a centre coordinate.
    *    @param   adminType   Fill the adminAreaCentres table with centre
    *                         coordinates for admin area items of this
    *                         item type.
    *    @return  The table with admin area centre coordinates.
    */
   adminAreaCentreTable_t createCentreCoordinatesForAdminAreas(
                                    ItemTypes::itemType adminType );

   /**
    *    Create info for and set (fill) the admin area centres table.
    *    Admin area centres are created for built-up area and municipal
    *    items. Overwrites any old content of the table. Must only be
    *    called on underview maps.
    *    @return  The number of admin area centres in the table.
    */
   uint32 createAndSetCentreCoordinatesForAdminAreas();

   uint32 getNbrAdminAreaCentres(){
      return m_nbrAdminAreaCentres;
   }

   /**
    *    Update the admin area centre table of this map. Used in dynamic
    *    map generation, and from any method affecting base info for
    *    the admin area centres. Must only be called for underview maps.
    *    @return  True if the table was updated, false if not.
    */
   bool updateAdminAreaCentres();
   //@} // admin area centres


   /**   Returns the top group among a set of items. This group is found to 
    *    be the top group if it has no parent group, and all items are 
    *    included in its tree.
    */
   OldItem* getTopGroup( set<OldItem*> items );

   /**
    * Removes all water not used by the server.
    */
   uint32 rmUnusedWater();

   /** 
    *   The geographicla description af this map.
    */
   GfxDataFull* m_gfxData;

   /**
    * @return the sign post table of this map.
    */
   const SignPostTable& getSignPostTable() const;
   SignPostTable& getNonConstSignPostTable();
   
   /**
    * @return Returns a map container with all node lane vectors of this map
    *         sorted by node IDs. Index of a lane in each vector gives the
    *         position of the lane on the road with the left most as index 0.
    */
   const ItemMap< vector<GMSLane> >& getNodeLaneVectors();

   /**
    * @return The table with all connecting lanes information of this map.
    */
   const ConnectingLanesTable& getConnectingLanes();


   /**
    * @name Index area order.
    */
   //@{
   /**
    * @param itemID The ID of the item to get the index area order of.
    * @return Returns the index area order of the item ID if the item itemID
    *         is an index area. If it is, but no index area have been set, 
    *         (MAX_UINT32-1) is returned. If itemID is not an index area,
    *         MAX_UINT32 is returned.
    */
   uint32 getIndexAreaOrder(uint32 itemID) const;
   
   /**
    * @param itemID The item ID of the item to set index area order of.
    * @param indexAreaOrder The index area order of the item with ID itemID.
    */
   void setIndexAreaOrder(uint32 itemID, uint32 indexAreaOrder);
   
   /**
    * @param itemID The ID of the item to check if it is an index area.
    * @return Returns True if the item is an index area, i.e. the index area
    *         order of the item is not MAX_UINT32.
    */
   bool isIndexArea(uint32 itemID) const;
   //@}

   /**
    * @name Item in search index
    */
   //@{
   /**
    * @return Returns true if the item with itemID is to not be included in 
    *         the search index. This could for instance be an item marked as 
    *         replaced by an TA MultiNet index area. Which means e.g. the normal
    *         built-up area that is there only for display, while the index 
    *         area bua is there not for display but only for search index.
    */
   bool itemNotInSearchIndex(uint32 itemID) const;

   /**
    * @param itemID The ID of the item that shoud be marked to not be included 
    *               in the search index.
    */
   void setItemNotInSearchIndex(uint32 itemID);
   //@}

   /**
    * @name Index area comparisons.
    */
   //@{
      /**
       *    Check the groups of one index area.
       *    This function returns
       *    true if the groups (only one level up) of the index area
       *    have the same names and index area order.
       *    Automatically returns true if the index area has no groups.
       */
      bool indexAreaGroupsSameNameAndOrder( 
                  const OldItem* indexAreaItem ) const;

      /**
       *    Check if two index area items have the same properties.
       *    Same properties means: same name, same index area order,
       *    and groups with the same properties. Recursive call to
       *    check also the groups of the index areas.
       *    @returns True if all properties of the index areas
       *             are equal.
       */
      bool mergeIndexAreas( const OldItem* firstIA,
                            const OldItem* secondIA) const;
   //@}

       /**
       *    Return the maximum length of all item names names for the
       *    chosen item type.
       *    @param   itemType  Type of item 
       *    @returns int       Maxlength of all item names for the
       *                       chosen type. Returns -1 if no zip code
       *                       items were found.
       */
      uint32 itemNameMaxLength( ItemTypes::itemType itemType );

   /**
    * @name Road display class.
    */
   //@{
      /**
       * @param itemID The ID of the item to get the display class for.
       * @return Returns the road display class of the item ID 
       *         if the item itemID is a street segment item. If itemID is 
       *         not a street segment or has no displayClass, the invalid
       *         nbrRoadDisplayClasses is returned.
       */
      ItemTypes::roadDisplayClass_t getRoadDisplayClass(uint32 itemID) const;
      
      /**
       * @param itemID The item ID of the ssi to set display class of.
       * @param dispClass The display class of the ssi with ID itemID.
       */
      void setRoadDisplayClass( uint32 itemID, 
                                ItemTypes::roadDisplayClass_t dispClass);

      /**
       * Print how many items there are of each road display class
       * in this map. 
       */
      void printRoadDisplayClassInfo();
      
   //@}

   /**
    * @name Area feature draw display class.
    */
   //@{
      /**
       * @param itemID The ID of the item to get the display class for.
       * @return Returns the area feature draw display class of the itemID
       *         if the item has such display class set. If itemID has no
       *         displayClass, the invalid end marker 
       *         nbrAreaFeatureDrawDisplayClasses is returned.
       */
      ItemTypes::areaFeatureDrawDisplayClass_t 
            getAreaFeatureDrawDisplayClass(uint32 itemID) const;
      
      /**
       * @param itemID The item ID of the ssi to set display class of.
       * @param dispClass The display class of the area feature 
       *                  with ID itemID.
       * @param priority  Whether to use AFDDC priorites defined in 
       *                  NationalProperties::AFDDCPriority
       */
      void setAreaFeatureDrawDisplayClass( 
               uint32 itemID, 
               ItemTypes::areaFeatureDrawDisplayClass_t dispClass,
               bool usePriority);
      
      /**
       * @param itemID The item ID of the ssi to remove display class of.
       *          The entry for this item will be removed from 
       *          the m_areaFeatureDrawDisplayClass item map.
       * @return True if the item had a display class to remove, else false.
       */
      bool removeAreaFeatureDrawDisplayClass( uint32 itemID );
      
      /**
       * Print how many items there are of each area feature draw 
       * display class in this map.
       * @param detailedPrint If set to true, will dump the display class
       *                      for all items.
       */
      void printAreaFeatureDrawDisplayClassInfo( bool detailedPrint = false );
      
   //@}

protected:

   /**
    *   Creates an empty map.
    */  
   OldGenericMap();

   /**
    *   Create a map with given ID.
    *   @param   id    The ID of the new map.
    */  
   OldGenericMap(uint32 id);

   /**
    *   Create a map with mapID and path. The map is @b not filled
    *   with data from the file. The load-method must be called after
    *   creation!
    *
    *   @param   mapID Id of the map to be loaded.
    *   @param   path The file-path to the map.
    */
   OldGenericMap(uint32 mapID, const char *path);

       

   /**
    *   Save this map into outfile.
    *   @param   outfile  The filedescriptor of the file where the
    *                     map will be saved.
    *   @return  True if the map is saved correctly, false otherwise.
    */
   virtual bool internalSave(int outfile);

   /**
    *    Create the map from a databuffer (preferrably with a 
    *    memory-mapped file).
    *    @param   A data buffer with all raw data for this map.
    */
   virtual bool internalLoad(DataBuffer& dataBuffer);

   /**
    *    This is loading of the static part of the map loading,
    *    always loaded when there is no mapBodySize given, like in
    *    the map versions lower than 6.
    *
    *    @param   A data buffer with all raw data for this map.
    */
   bool loadFirstBodyBlock(DataBuffer& dataBuffer);


   /**
    *   Find empty slots in the itemsZoom-array.
    *   @param   zoomLevel   The zoomlevel where we will search for 
    *                        empty slots.
    *   @param   cnt         Outparameter that indicates the offset 
    *                        where we start search in zoomLevel. <i>
    *                        Thus should this variable be 0 at the 
    *                        first call!</i>
    *   @return  True if a empty slot is found, false otherwise.
    */
   bool findEmptySlot(uint32 zoomlevel, uint32 &cnt);

   /**
    *   Change places of the items with given IDs.
    *   NB! Currently only the itemID is changed, if they are
    *       members of any gruops this must also be changed,
    *       and if any street segment item is involved connections
    *       etc must be changed.
    */
   bool swapItems(uint32 aID, uint32 bID);

   /**
    *   Create a new item from the data buffer given as parameter and
    *   return that. This exisist mainly to be able to create a GMSMap
    *   (with GMS*OldItem's) in the subclass without have to rewrite all
    *   code that reads the map from disc.
    *
    *   @param   type        The type of the item that will be created.
    *   @param   dataBuffer  The buffer where the data for the item 
    *                        is loacted.
    *   @param   itemID      ID of the new item.
    *   @return  A new item, created from dataBuffer with type and ID
    *            itemID.
    */
   virtual OldItem* createNewItem(DataBuffer* dataBuffer);

   /**
    *   Create a new gfxData from the data buffer given as parameter and
    *   return that. This exisist mainly to be able to create a GMSMap
    *   (with GMSGfxData's) in the subclass without have to rewrite all
    *   code that reads the map from disc.
    *
    *   @param   dataBuffer  The buffer where the data for the GfxData
    *                        is loacted.
    *   @return  A new gfxData.
    */
   GMSGfxData* createNewGfxData(DataBuffer* dataBuffer);

   /**
    *   Virtual method that creates a new OldConnection out of a 
    *   databuffer. This method excists only to make it possible
    *   to create subclasses of this class (e.g. GMSConnection).
    *   @param   dataBuffer  The buffer where the data for the 
    *                        OldConnection is loacted.
    *   @return  A new OldConnection.
    */
   virtual OldConnection* createNewConnection(DataBuffer* dataBuffer);

   /**
    * Help method to getTopGroup. Used recursively.
    * @param topNodes Outparameter Fill this with all top nodes found so far.
    *                              Is filled in with all top nodes found.
    * @param vistitedItemIDs Outparameter Fill this one in with the visited 
    *                        nodes so far. Is filled in with all 
    *                        visitedItemIDs.
    * @param item The item to check for top nodes in the group hierarchy.
    */
   void getTopGroupProcessItem( set<OldItem*>& topNodes,
                                set<uint32>& visitedItemIDs,
                                OldItem* item );


      

   /**
    *   Get all items that have a given string index. 
    *   @note This is not very effecive, since all items on the map 
    *         are checked.
    *
    *   @param   strIndex The string index to look for. Observe that
    *                     only the string index (and not the language
    *                     or stringtype) is checked!
    *   @param   result   A Vector that is filled with the ID of the
    *                     items that have strIndex among their names.
    *   @return  True if the result-Vector is set, false otherwise.
    */
   bool getAllItemsWithStringIndex(uint32 strIndex, Vector* result);
      
      
   /**
    *    Get items with the specified name.
    *    @param   name     The name.   
    *    @param   items    Out parameter. Will be filled with items that
    *                      has the same name.
    *    @param   itemType Optional parameter. If specified the items
    *                      must be of this item type.
    */
   void getItemsWithName( 
                         const char* name, 
                         set<const OldItem*>& items,
                         ItemTypes::itemType itemType 
                         = ItemTypes::numberOfItemTypes ) const;


   void getItemsWithName( 
                         const char* name, 
                         set<OldItem*>& items,
                         ItemTypes::itemType itemType 
                         = ItemTypes::numberOfItemTypes ) const;


   /**
    *    Debug printing of file size + offset in databuffer.
    *
    *    @param message Some text telling where the file position is
    *                   printed.
    *    @param outfile The file to print size of.
    *    @param dbuf    The data buffer of which the offset is added. 
    *                   to null if not to use a databuffer.
    */
   void dbgPrintFilePos(MC2String message, 
                        int outfile, 
                        DataBuffer* dbuf=NULL);

      
   /**
    *    Approximate map size.
    */
   uint32 m_approxMapSize;
      
   /**
    *   Pointer to the hashtable uesed to search for items close
    *   to a location.
    */
   OldMapHashTable* m_hashTable;

   /**
    *   An array with pointers to the OldBoundrySegments-objects for
    *   this map. Every item in this ObjVector contains information 
    *   about one @c OldStreetSegmentItem, including possible 
    *   connections.
    */
   OldBoundrySegmentsVector* m_segmentsOnTheBoundry;

   /**
    *   The table holding landmark descriptions in this map. The 
    *   key (not necessarily unique) is fromNodeID.toNodeID, 
    *   representing one connection the landmark is connected to.
    */
   landmarkTable_t m_landmarkTable;

   /**
    *    Table mapping together multi nodes -> expanded nodes.
    *    I.e. a multi connection going between the two node id as
    *    specified in multiNodes_t will be expanded to the list of
    *    node ids expandedNodes_t.
    *
    *      ________________
    *     /   multi conn   \
    *    /                  V
    *    o--oo--oo--oo--oo--o
    *    ^   ^   ^   ^      ^
    *    |   \---|---|      |
    *    |         |        |
    *  first    expanded  second
    *  multi    nodes     multi
    *  node               node
    *  
    */
   map<multiNodes_t, expandedNodes_t> m_nodeExpansionTable;

   /**
    *   NUMBER_GFX_ZOOMLEVELS vectors used for fast retreiving
    *   of an item when the item-id is known. 
    *   @verbatim
    +-+----+-------------------------+
    |1|  4 |           27            |
    +-+----+-------------------------+
    1:  The first bit is used only to determine which node
    (when an item has nodes...)
    4:  4 bits that indicates the zoomlevel. These bits are used
    for selecting in witch of the vectors the item is located.
    27: The itemnumber within the zoomlevel.
    @endverbatim
   */
   OldItem **m_itemsZoom[NUMBER_GFX_ZOOMLEVELS];

   /**
    *   The size of the itemsZoom-vectors (in items). The sum of 
    *   this numbers gives the total numbers of items on this map.
    */
   uint32 m_itemsZoomSize[NUMBER_GFX_ZOOMLEVELS];

   /**
    *   The number of allocated bytes in the itemsZoom vectors.
    *   Please set this to the correct value when changing the
    *   size of the itemsZoom vector. 
    */
   uint32 m_itemsZoomAllocated[NUMBER_GFX_ZOOMLEVELS];
      
   /**
    *   The strings are now contained in this object.
    */
   OldItemNames *m_itemNames;

   /**
    *    @name Allocators
    *    Pointers to the allocators that are used to allocate
    *    memory for the items in the map. These must be initiated 
    *    before the items in the map are created.
    */
   //@{
   /// Allocator for the street segment items
   AbstractMC2Allocator* m_streetSegmentItemAllocator;

   /// Allocator for the street items
   AbstractMC2Allocator* m_streetItemAllocator;

   /// Allocator for the municipal items
   AbstractMC2Allocator* m_municipalItemAllocator;

   /// Allocator for the districts (cityPartItem)
   AbstractMC2Allocator* m_cityPartItemAllocator;

   /// Allocator for the water items
   AbstractMC2Allocator* m_waterItemAllocator;

   /// Allocator for the park items
   AbstractMC2Allocator* m_parkItemAllocator;

   /// Allocator for the forest items
   AbstractMC2Allocator* m_forestItemAllocator;

   /// Allocator for the building items
   AbstractMC2Allocator* m_buildingItemAllocator;

   /// Allocator for the railway items
   AbstractMC2Allocator* m_railwayItemAllocator;

   /// Allocator for the island items
   AbstractMC2Allocator* m_islandItemAllocator;

   /// Allocator for the zip code items
   AbstractMC2Allocator* m_zipCodeItemAllocator;

   /// Allocator for the zip code items
   AbstractMC2Allocator* m_zipAreaItemAllocator;

   /// Allocator for the point of interest items
   AbstractMC2Allocator* m_pointOfInterestItemAllocator;

   /// Allocator for the category items
   AbstractMC2Allocator* m_categoryItemAllocator;

   /// Allocator for the build up area items
   AbstractMC2Allocator* m_builtUpAreaItemAllocator;

   /// Allocator for the bus route items
   AbstractMC2Allocator* m_busRouteItemAllocator;

   /// Allocator for the airport items
   AbstractMC2Allocator* m_airportItemAllocator;

   /// Allocator for the aircraftRoadItem items
   AbstractMC2Allocator* m_aircraftRoadItemAllocator;

   /// Allocator for the pedestrianAreaItem items
   AbstractMC2Allocator* m_pedestrianAreaItemAllocator;

   /// Allocator for the militaryBaseItem items
   AbstractMC2Allocator* m_militaryBaseItemAllocator;

   /// Allocator for the individualBuildingItem items
   AbstractMC2Allocator* m_individualBuildingItemAllocator;

   /// Allocator for the subway line items
   AbstractMC2Allocator* m_subwayLineItemAllocator;

   /// Allocator for the NULL-items
   AbstractMC2Allocator* m_nullItemAllocator;

   /// Allocator for the ferry items
   AbstractMC2Allocator* m_ferryItemAllocator;

   /// Correct type for the GfxData allocator to avoid problems.
   MC2Allocator<class GMSGfxData>* m_gfxDataAllocator;
   MC2Allocator<class GMSGfxData>* m_gfxDataSingleSmallPolyAllocator;
   MC2Allocator<class GMSGfxData>* m_gfxDataSingleLineAllocator;
   MC2Allocator<class GMSGfxData>* m_gfxDataSinglePointAllocator;
   MC2Allocator<class GMSGfxData>* m_gfxDataMultiplePointsAllocator;

   /// Allocator for the nodes
   AbstractMC2Allocator* m_nodeAllocator;

   /// Allocator for the connections
   AbstractMC2Allocator* m_connectionAllocator;

   /// Allocator for the connection pointers in the OldNodes.
   MC2ArrayAllocator<OldConnection*>* m_connectionArrayAllocator;
   //AbstractMC2Allocator* m_connectionArrayAllocator;

   /// Allocator for item types that are part of class OldItem
   AbstractMC2Allocator* m_simpleItemAllocator;

   /// Allocator for cartographic item.
   AbstractMC2Allocator* m_cartographicItemAllocator;

   /// Contains the user rights for items
   UserRightsItemTable* m_userRightsTable;

   /// IDTranslation table. Empty in non-overview maps
   IDTranslationTable m_idTranslationTable;

   /// Default argument to functions taking UserRightsMapInfo
   UserRightsMapInfo* m_allRight;

   /// Let the create-method access these members
   friend OldItem* OldItem::createNewItem(DataBuffer*, OldGenericMap*);
   friend OldConnection* OldConnection::createNewConnection(DataBuffer*, 
                                                            OldGenericMap*);
   friend class GMSFerryItem;

   //@}

   /**
    *   Maps administrative area item ids to a centre coordinate,
    *   from the city centre that is located in the admin area and
    *   have the same name.
    */
   adminAreaCentre_t* m_adminAreaCentres;

   /**
    *    The number of elements in m_adminAreaCentres.
    */
   uint32 m_nbrAdminAreaCentres;

   /**
    *   Stored the NAVTEQ GDF attribute official code [OC]. 
    *   Accessed directly from class OldItem.
    */
   ItemMap<uint32> m_itemOfficialCodes;

   typedef ItemMap< set<uint16> > CategoryMap;
   /**
    *   Stores the category IDs of items. More than one category per item is 
    *   possilbe.
    */
   CategoryMap m_itemCategories;

   /**
    *   Stores the connecting lanes of connections identified with from node
    *   ID and to node ID.
    */
   ConnectingLanesTable m_connectingLanesTable;

   /**
    *   Stores the lane vectors of nodes. Index in the vector indicates
    *   the index of the lane, with 0 as the left most lane.
    */
   ItemMap< vector<GMSLane> > m_nodeLaneVectors;

   /**
    *   Stores the sign posts of connections. The connections are identified 
    *   with from and to node ID.
    */
   SignPostTable m_signPostTable;
   
   /**
    *   Stores the hierarchy order of index areas. For an index area with no
    *   order, a post with value (MAX_UINT32-1) is stored.
    */
   ItemMap<uint32> m_indexAreasOrder;

   /**
    *   Stores IDs of items that should not be part of the search index. The 
    *   value of an item ID being false is considered to be the same as a the
    *   item ID missing from this table.
    */
   ItemMap<bool> m_nonSearchableItems;

   /**
    *   Stores the displayClass for street segments.
    */
   ItemMap<uint32> m_roadDisplayClass;

   /**
    *   Stores the draw display class for area features.
    */
   ItemMap<uint32> m_areaFeatureDrawDisplayClass;


private:
   /**
    *    @name Help methods to getConnectionData. Works on single
    *          connections, i.e. non multi connections.
    */
   //@{
   /**
    *    Calculate the length of the specified connection.
    *    @param   conn     The connection to be examined.
    *    @param   toNode   The node that the connection is attached
    *                      to. (Ie. the node that the connection is
    *                      leading to.)
    *    @param   length   Outparameter that is set to the length
    *                      in meters of the connection.
    *    @param   externalConnection
    *                      If this is set to true, then it signals
    *                      that the supplied connection is an
    *                      external connection. Since it is not
    *                      possible to find the segment that it
    *                      is coming from (is in "another" map),
    *                      it is assumed that the properties of the
    *                      "fromSegment" can be approximated with the
    *                      properties of the "toSegment", (even if
    *                      we know that it is not correct). This
    *                      parameter defaults to false.
    *    @return  True if the length was successfully computed,
    *             false otherwise.
    */
   bool getSingleConnectionLength(OldConnection* conn, OldNode* toNode,
                                  float64& length,
                                  bool externalConnection 
                                  = false) const;
         
   /**
    *    Calculate the time to traverse the specified connection
    *    based on roadclasses, speedlimits, turndirections etc.
    *    Note that this method only handles streetsegment to 
    *    streetsegment connections (as opposed to 
    *    getSingleConnectionLength() for instance). 
    *    If a non ssi -> ssi connection is detected, the 
    *    method returns false. The time for those kinds of
    *    connections should come from the InfoModule instead!
    *
    *    Uses precalculated length of the connection.
    *    
    *    @param   conn     The connection to be examined.
    *    @param   toNode   The node that the connection is attached
    *                      to. (Ie. the node that the connection is
    *                      leading to.)
    *    @param   time     Outparameter that is set to the time
    *                      to traverse the connection.
    *    @param   standStillTime Outparameter that is set to the 
    *                            standstill time to traverse the
    *                            connection.
    *    @param   length   The length of the connection in meters. 
    *                      Should come from getSingleConnectionLength 
    *                      method.
    *    @param   externalConnection
    *                      If this is set to true, then it signals
    *                      that the supplied connection is an
    *                      external connection. Since it is not
    *                      possible to find the segment that it
    *                      is coming from (is in "another" map),
    *                      it is assumed that the properties of the
    *                      "fromSegment" can be approximated with the
    *                      properties of the "toSegment", (even if
    *                      we know that it is not correct). This
    *                      parameter defaults to false.
    *    @return  True if the time was successfully computed,
    *             false otherwise.
    */
   bool getSingleConnectionTime(OldConnection* conn, OldNode* toNode,
                                float64& time, 
                                float64& standStillTime,
                                float64 length,
                                bool externalConnection 
                                = false) const;
   //@}

    
   /**
    *   Initiates the map with empty values. This is used as a
    *   default constructor.
    *   @param   id    The ID of the "empty" map. This is to make
    *                  it easier to search for maps.
    */
   void initEmptyMap(uint32 id);

   /**
    *    Remove all identical coordinates in all GfxData's.
    */
   void removeAllIdenticalCoordinates();

   /**
    *    Get the distance from a given coordinate to a node. The value
    *    that will be returned is the distance in meters squared.
    *    
    *    @param   nodeNbr  0 for node zero, otherwise node 1.
    *    @param   gfx      Pointer to the GfxData where nodeNbr
    *                      is located.
    *    @param   lat      Latitude part of the coordinatde.
    *    @param   lon      Longitude part of the coordinatde.
    *    @return  The distance from the coordinates to
    *             the node in meters squared.
    */
   float64 getDistanceToNode( byte nodeNbr, GfxData* gfx, 
                              int32 lat, int32 lon);

   /**
    *   Get a pointer to the itemnames of this map.
    *   @return  Pointer to the names on this map.
    */
   inline OldItemNames* getItemNames() const;

   /**
    *    Help method for getSingleConnectionLength() that is 
    *    used to calculate
    *    the length between a OldBusRouteItem and a StreetsegmentItem
    *    based on the offset of the OldBusRouteItem and the distance of
    *    the ssi.
    *    @param   bri         The OldBusRouteItem,
    *    @param   dist        The length of the streetsegmentitem.
    *    @param   isSSINode0  True if the streetsegmentitem's node0
    *                         is closest to the OldBusRouteItem, false
    *                         otherwise.
    *    @return  The distance.
    */
   float64 calcBusSSILength(OldBusRouteItem* bri, float64 dist, 
                            bool isSSINode0) const;



  
};

// ========================================================================
//                                      Implementation of inlined methods =

inline const map<OldGenericMap::multiNodes_t, OldGenericMap::expandedNodes_t>*
OldGenericMap::getNodeExpansionTable() const
{
   return &m_nodeExpansionTable;
}

inline uint32 
OldGenericMap::getNbrItemsWithZoom(uint32 z) const 
{
   if (z<NUMBER_GFX_ZOOMLEVELS)
      return (m_itemsZoomSize[z]);
   else
      return (0);
}

inline uint32 
OldGenericMap::getNbrItems() const 
{
   uint32 retVal = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++)
      retVal += m_itemsZoomSize[z];
   return (retVal);
}

inline OldItem* 
OldGenericMap::getItem(uint32 zoom, uint32 index) const 
{
   if (index < getNbrItemsWithZoom(zoom))
      return (m_itemsZoom[zoom][index]);
   else
      return (NULL);
}

inline OldItem*
OldGenericMap::itemLookup(uint32 itemID) const 
{
   if ((itemID & 0x07ffffff) < 
      getNbrItemsWithZoom((itemID & 0x78000000) >> 27))
      return m_itemsZoom[(itemID & 0x78000000) >> 27]
                      [itemID & 0x07ffffff];
   else
      return NULL;
}
    
inline const GMSGfxData* 
OldGenericMap::getGfxData() const
{
   // we do know what this is
   return static_cast<GMSGfxData*>(m_gfxData);
}
inline GMSGfxData* 
OldGenericMap::getGfxData()
{
   // we do know what this is
   return static_cast<GMSGfxData*>(m_gfxData);
}

inline void 
OldGenericMap::setGfxData( GfxDataFull* mapGfx )
{
   m_gfxData = mapGfx;
}
      
inline OldGenericMap::landmarkTable_t&
OldGenericMap::getLandmarkTable()
{
   return (m_landmarkTable);
}

inline const OldGenericMap::landmarkTable_t&
OldGenericMap::getLandmarkTable() const
{
   return (m_landmarkTable);
}


inline OldMapHashTable* 
OldGenericMap::getHashTable() const
{
   return (m_hashTable);
}

inline OldItemNames* 
OldGenericMap::getItemNames() const 
{
   return (m_itemNames);
}

inline const char* 
OldGenericMap::getFirstItemName(const OldItem* item) const 
{
   // Make sure that nothing is null
   if ((item == NULL) || (m_itemNames == NULL)) {
      return (NULL);
   }

   // Return the name, if any -- otherwise "MISSING"
   if (item->getNbrNames() > 0)
      return (m_itemNames->getString(item->getStringIndex(0)));
   else  
      return (m_itemNames->getString(0));
      // Does not work for co map, where "MISSING" is not stored.
}

inline const char* 
OldGenericMap::getFirstItemName(uint32 itemID) const 
{
   return (getFirstItemName(itemLookup(itemID)));
}

inline const char* 
OldGenericMap::getName(uint32 stringIndex) const
{
   if (m_itemNames != NULL) {
      return (m_itemNames->getString(stringIndex));
   } else {
      return (NULL);
   }
}

inline const char*
OldGenericMap::getRawName(uint32 rawIndex) const
{
   if ( rawIndex == MAX_UINT32 ) {
      return NULL;
   } else {
      return getName(GET_STRING_INDEX(rawIndex));
   }
}

inline const char* 
OldGenericMap::getBestItemName(const OldItem* item, 
                            LangTypes::language_t strLang) const
{
#if 1
   if ((item == NULL) || (item->getNbrNames() == 0)) {
      return NULL;
   }
   return getRawName(item->getBestRawName(strLang));
#else
   vector<LangTypes::language_t> strLangs;
   strLangs.push_back(strLang);
   // Use English as default language
   if (strLang != LangTypes::english) {
      strLangs.push_back(LangTypes::english);
   }
   return getBestItemName(item, strLangs);
#endif
}

inline const char* 
OldGenericMap::getBestItemName(const OldItem* item, 
                            vector<LangTypes::language_t> &strLang) const
{
   // Make sure we have anything
   if ((item == NULL) || (item->getNbrNames() == 0)) {
      return NULL;
   }

#if 1
   // getRawName will return NULL if the index is MAX_UINT32
   return getRawName(item->getBestRawName(strLang));
#else
   
   // 1. Official name
   const char* retVal = getBestItemName(item, ItemTypes::officialName, 
                                        strLang);
   if (retVal != NULL) {
      return retVal;
   }

   // 2. Alternative name in requested language, return when/if found!
   retVal = getBestItemName(item, ItemTypes::alternativeName, strLang);
   if (retVal != NULL) {
      return retVal;
   }

   // 3. Any name in requested language, return when/if found!
   retVal = getBestItemName(item, ItemTypes::invalidName, strLang);
   if (retVal != NULL) {
      return retVal;
   }

   // 4. Any name (the first one)
   return getName(item->getRawStringIndex(0));
#endif
}

inline const char* 
OldGenericMap::getBestItemName(uint32 itemID) const
{
   return (getBestItemName(itemID, LangTypes::invalidLanguage));
}

inline const char* 
OldGenericMap::getBestItemName(uint32 itemID, LangTypes::language_t strLang) const
{
   return (getBestItemName(itemLookup(itemID), strLang));
}

inline const char* 
OldGenericMap::getBestItemName(uint32 itemID, 
                            vector<LangTypes::language_t> &strLang) const
{
   return (getBestItemName(itemLookup(itemID), strLang));
}


inline const char* 
OldGenericMap::getBestItemName(const OldItem* item, 
                            ItemTypes::name_t strType,
                            LangTypes::language_t strLang) const
{
#if 1
   if ((item == NULL) || (item->getNbrNames() == 0)) {
      return NULL;
   }
   return getRawName(item->getBestRawNameOfType(strType, strLang) );
#else
   vector<LangTypes::language_t> strLangs;
   strLangs.push_back(strLang);
   // Use English as default language
   if (strLang != LangTypes::english) {
      strLangs.push_back(LangTypes::english);
   }
   return getBestItemName(item, strType, strLangs);
#endif
}

inline const char* 
OldGenericMap::getBestItemName(const OldItem* item, 
                            ItemTypes::name_t strType,
                            vector<LangTypes::language_t> &strLang) const
{
#if 1
   if ((item == NULL) || (item->getNbrNames() == 0)) {
      return NULL;
   }
   return getRawName(item->getBestRawNameOfType(strType, strLang));
#else
   uint32 i = 0;
   // In requested language, return when/if found!
   while (i < strLang.size()) {
      uint32 strIndex = item->getNameWithType(strType, strLang[i]);
      if (strIndex != MAX_UINT32) {
         return getName(strIndex);
      }
      ++i;
   }

   // Try any language
   uint32 strIndex = item->getNameWithType(strType, 
                                           LangTypes::invalidLanguage);
   if (strIndex != MAX_UINT32) {
      return getName(strIndex);
   }
   
   return NULL;
#endif
}


inline const char* 
OldGenericMap::getItemName( OldItem* item,
                         LangTypes::language_t strLang,
                         ItemTypes::name_t strType ) const 
{
   if ( item == NULL || m_itemNames == NULL ) {
      return NULL;
   }
#if 1
   return getRawName(item->getBestRawName(strLang, strType));
#else
   // Get the name with correct type and language
   uint32 stringIndex = item->getNameWithType(strType, strLang);

   // Try the languages of the map
   if (m_nativeLanguages != NULL) {
      uint32 mapLangNbr = 0;
      while ( (stringIndex == MAX_UINT32) && 
              (mapLangNbr < m_nativeLanguages->getSize())) {
         stringIndex = item->getNameWithType(strType, 
               LangTypes::language_t(m_nativeLanguages->
                  getElementAt(mapLangNbr)));
         mapLangNbr++;
      }
   }

   // Try english
   if ((stringIndex == MAX_UINT32) && (strLang != LangTypes::english))
      stringIndex = item->getNameWithType(strType, LangTypes::english);

   // Take the first name with correct type
   if (stringIndex == MAX_UINT32) 
      stringIndex = item->getNameWithType(strType, LangTypes::invalidLanguage);

   // Take the first name with any type
   if (stringIndex == MAX_UINT32) 
      stringIndex = item->getNameWithType(ItemTypes::invalidName,
                                          LangTypes::invalidLanguage);

   // Return the found name (or NULL if stringIndex == MAX_UINT32)
   return m_itemNames->getString(stringIndex);
#endif
}

inline const char* 
OldGenericMap::getItemName( uint32 itemID,
                         LangTypes::language_t strLang,
                         ItemTypes::name_t strType ) const 
{
   return getItemName( itemLookup( itemID ), strLang, strType );
}

inline OldBoundrySegmentsVector* 
OldGenericMap::getBoundrySegments() const 
{
   return m_segmentsOnTheBoundry;
}



#endif

/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GENERICMAP_H
#define GENERICMAP_H

#include "config.h"

#include <set>
#include <list>
#include <map>
#include <fstream>

#include "ItemNames.h"
#include "Item.h"
#include "GenericMapHeader.h"
#include "IDTranslationTable.h"
#include "UserEnums.h"
#include "UserRightsItemTable.h"
#include "SelectableDataTable.h"
#include "SimpleArrayObject.h"
#include "ScopedArray.h"
#include "Connection.h"
#include "MC2Coordinate.h"
#include "ExpandStringLane.h"
#include "MC2MapAllocator.h"
#include "Array.h"
#include "ValuePair.h"
#include "SignPost.h"
#include "MapBits.h"

#include "MC2Exception.h"
#include "NotCopyable.h"

// XXX: This is the maximum size of a zoomlevel of a map, 
//      this is an ugly way
//      to handle this. Should/will be replaced with the 
//      totalMapSize-attribute when this is implemented
#define MAX_BUFFER_SIZE 250000000
// XXX: Max size of a zoomlevel of a super overview map 
// (0x90000000 or greater)
#define MAX_BUFFER_SIZE_SUPER_OVERVIEW 450000000
// Keep in sync with define in OldGenericMap

// Forward declarations.
class DataBuffer;
class MapHashTable;
class AirportItem;
class AircraftRoadItem;
class BuildingItem;
class BuiltUpAreaItem;
class BusRouteItem;
class CategoryItem;
class CityPartItem;
class Connection;
class FerryItem;
class ForestItem;
class GfxData;
class GfxDataFull;
class GroupItem;
class IndividualBuildingItem;
class IslandItem;
class MunicipalItem;
class Node;
class NullItem;
class ParkItem;
class PedestrianAreaItem;
class PointOfInterestItem;
class RailwayItem;
class StreetItem;
class StreetSegmentItem;
class SubwayLineItem;
class UserRightsMapInfo;
class WaterItem;
class ZipCodeItem;
class CartographicItem;
class ZipAreaItem;
class GfxDataFactory;
class TileMapCoord;
class StreetNbr;
class POIInfo;
class MC2BoundingBox;

class BoundrySegmentsVector;

class AbstractMC2Allocator;
template <class T> class MC2ArrayAllocator;
template <class T> class MC2Allocator;
template <typename T> class ItemAllocator;

struct DataBufferMapObject;

class ItemIdentifier;

/**
 * For general GenericMap errors.
 */
class GenericMapError: public MC2Exception {
public:
   /// @param msg Message.
   GenericMapError( const MC2String& msg ):
      MC2Exception( MC2String("[GenericMap]") + msg ) {
   }
};

/**
  *   Description of one map in the MapCentral-system.
  *   Uses VectorElement as superclass to make it possible to
  *   store the maps in a Vector.
  *
  */
class GenericMap : public GenericMapHeader, private NotCopyable {
   /**
    *    @name Friend classes
    *    The classes that might access the private fields.
    *    The MapHandler must be able to change the map ID when one 
    *    map is copied and the MapReader is calling some really 
    *    dangerous methods that recreats the maps...
    */
   //@{
   friend class MapHandler;
   friend class MapReader;
   friend class RouteMapGenerator;
   friend class WriteableRouteMultiConnTable;
   friend class StringTableMapGenerator;
   friend class ExtraDataUtility;
   friend class Item;
   friend class BoundrySegment;
   friend class BoundrySegmentsVector;
   friend class ItemInfoDialog;
   friend class MapGenUtil;
   friend class M3Creator;
   //@}

public:
   typedef SelectableDataTable<StreetNbr> streetSideTable_t;
   typedef vector<uint16> Categories;

   /**
    *   Create a map with given ID.
    *   @param   id    The ID of the new map.
    */  
   explicit GenericMap(uint32 id);

   /**
    *    Create a map from file with the correct dynamic qualification.
    *    @param   id    The ID of the map to create from file.
    *    @param   path  The path on the filesystem to the .mcm-file
    *                   that contains the data for the new map.
    *    @return  A new map, created with data from the file at the
    *             given path. NULL is returned upon error.
    */
   static GenericMap* createMap(uint32 id, const char* path = NULL);

   /**
    *    Create a map from file with the correct dynamic qualification.
    *    @param   mcmName  The name of the map on the filesystem that 
    *                      contains the map.
    *    @return  A new map, created with data from the file at the
    *             given path. NULL is returned upon error.
    */
   static GenericMap* createMap(const char* mcmName);


   /**
    *    Returns an approximate map size.
    *    @return An approximate map size in bytes.
    */
   uint32 getApproxMapSize() const;

   /**
    *    Get the citypart id that this item is part of.
    *    @param   item  The item to check.
    *    @return  The citypart id that the item is part of, or
    *             MAX_UINT32 if it wasn't part of any citypart.
    */
   uint32 getCityPartID(const Item* item) const;


   /**
    *    @param  group The item to look for in the groups of the other
    *                  items of the map.
    *
    *    @return Returns true if there are at least one item in the map
    *            which has this item as a group.
    *
    */
   bool itemUsedAsGroup( Item* group ) const;

      
   /**
    * @param groupID The ID of an item to look for other items being 
    *                a member of the item with ID groupID.
    * @return A vector with item IDs of items being members of the item
    *         with ID groupID.
    */
   vector<uint32> getItemsInGroup(uint32 groupID) const;

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
   uint32 polygonHasItemsInside(Item* polyItem, 
                                ItemTypes::itemType 
                                typeOfItemsToLookFor = 
                                ItemTypes::numberOfItemTypes);
 
   /**
    *
    */
   //@{
   /**
    *   Get the location of an item in the map. Will loop over all 
    *   the items the item is member of and return the first one 
    *   of the given type.
    *
    *   @param item The item to get location for.
    *   @param type What kind of location, e.g. bua, mun, city part
    *               or zip.
    *
    *   @return The ID of the first group/item that item is member 
    *           of.
    */
   uint32 getRegionID(const Item* item, 
                      ItemTypes::itemType type) const;

   /**
    *   Get the location of an item in the map. Will loop over all 
    *   the items the item is member of and return the first one 
    *   of the given type.
    *
    *   @param item The item to get location for.
    *   @param type What kind of location, e.g. bua, mun, city part
    *               or zip.
    *
    *   @return The first group/item that item is member 
    *           of.
    */
   Item* getRegion(const Item* item, 
                   ItemTypes::itemType type) const;

   /**
    *    Get the number of regions of a specific type.
    *    @param   item  The item.
    *    @param   type  The type.
    *    @return  The number of regions of that itemtype found.
    */
   byte getNbrRegions(const Item* item, 
                      ItemTypes::itemType type) const;

   /**
    *    Get regions of a specific type
    */
   vector<Item*> getRegions(const Item* item,
                            ItemTypes::itemType type) const;
      
   /**
    *    Get a specific region id.
    *    @param   item  The item.
    *    @param   i     The region index.
    *    @param   type  The item type of the region.
    *    @return  The region id.
    */
   uint32 getRegionIDIndex(const Item* item, uint32 i,
                           ItemTypes::itemType type) const;
      
   /**
    *    Get a specific region.
    *    @param   item  The item.
    *    @param   i     The region index.
    *    @param   type  The item type of the region.
    *    @return  The region.
    */
   Item* getRegionIndex(const Item* item, uint32 i,
                        ItemTypes::itemType type) const;

   /**
    *    Check if an item contains a specified region.
    *    @param   item     The item.
    *    @param   region   The region to check for.
    *    @return  True if the item contains the specified region.
    */
   bool containsRegion( const Item* item, 
                        const Item* region ) const; 

   /**
    * Get the index area for an item, NULL if none.
    *
    * @param item The Item to check for an Index Area group.
    * @return The Index Area that item belongs to or NULL if no such
    *         group in item.
    */
   Item* getIndexAreaFor( const Item* item ) const;
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
    *   Returns connection allocator.
    */
   MC2Allocator<Connection>& getConnectionAllocator();
   const MC2Allocator<Connection>& getConnectionAllocator() const;


   MC2Allocator<TileMapCoord>& getCoordinateAllocator();

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

   /**
    *    Update the admin area centre table of this map. Used in dynamic
    *    map generation, and from any method affecting base info for
    *    the admin area centres. Must only be called for underview maps.
    *    @return  True if the table was updated, false if not.
    */
   bool updateAdminAreaCentres();

  /**
    *   Deletes the map.
    */
   virtual ~GenericMap();

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
   inline Item* getItem(uint32 zoom, uint32 index) const;

   /**
    *  Get an item with the given ID.   
    *  @param   itemID of needed Item.
    *  @return  Pointer to item-object with given itemID.
    */
   inline Item* itemLookup(uint32 itemID) const;

   /**
    *  Get an node with the given ID.   
    *  @param   nodeID   The ID of the node, this is the same as
    *                    the ID .
    *  @return  Pointer to item-object with given itemID.
    */
   Node* nodeLookup(uint32 nodeID) const;

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
   MC2Coordinate getOneGoodCoordinate(const Item* item) const;
         
   /**
    *    Sets the coordinate to one coordinate suitable for
    *    locating the item in e.g. searches.
    *    @param coord Coordinate to change into good one.
    */
   void getOneGoodCoordinate(MC2Coordinate& coord,
                             const Item* item) const;

   /**
    *    Returns a bounding box for the item.
    *    @param bbox The bounding box to fill in.
    *    @param item The item.
    */
   void getItemBoundingBox(MC2BoundingBox& bbox,
                           const Item* item) const;

   /**
    *    Returns a bounding box for the item.
    *    @param item The item to get the bbox for.
    *    @return The bounding box.
    */
   MC2BoundingBox getItemBoundingBox(const Item* item) const;

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
   int32 getItemOffset(Item* item, int32 lat, int32 lon) const;
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
   uint32 addNameToItem(Item* item, const char* name, 
                        LangTypes::language_t lang, 
                        ItemTypes::name_t type);
         
         
         
   //@}

   /**
    *   Get the graphical representation of the map.
    *   
    *   @return Pointer to the gfxData of this map.
    */
   inline const GfxData* getGfxData() const;

   /**
    *   Set the graphical representation of the map.
    */
   inline void setGfxData( GfxData* mapGfx );


   inline GfxDataFactory& getGfxDataFactory();

   /**
    *    Returns a gfxData for an Item. If the Item is a poi
    *    then the supplied GfxDataFull will be filled in with
    *    the coordinate of the poi and a pointer to it will be
    *    returned.
    */
   const GfxData* getItemGfx( const Item* item, GfxDataFull& gfx ) const;
      
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
    *   <b>Deprecated</b>. Use the functions in GenericMap instead.
    *   (Or add a new one).
    *   @return  Pointer to the hashTable of this map.
    */
   inline MapHashTable* getHashTable() const;

   /**
    *    Puts the items within the radius in the supplied set.
    *    @param resultItems  The result is put here.
    *    @param center       Center of the circle.
    *    @param radiusMeters Radius in meters of the circle.
    *    @param allowedTypes Set of allowed item types of the returned items.
    */
   void
   getItemsWithinRadiusMeter(set<Item*>& resultItems,
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
   void getItemsWithinBBox(set<Item*>& resultItems,
                           const MC2BoundingBox& bbox,
                           const set<ItemTypes::itemType>& allowedTypes,
                           const UserRightsMapInfo* rights = NULL);
   
   /**
    *    Puts the items within the gfxData in the supplied set.
    *    @param resultItems  The result is put here.
    *    @param gfxData      GfxData to check inside.    
    *    @param allowedTypes Set of allowed types of the returned items.
    */
   void getItemsWithinGfxData(set<Item*>& resultItems,
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
                                const UserRightsMapInfo* rights = NULL) const;

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
   bool itemAllowedByUserRights( const Item& item,
                                 const UserRightsMapInfo& rights ) const;

   /**
    *    Returns the copyright string for an item.
    *    Uses the UserRights for the item and if no good match is found
    *    the copyright is taken from the map.
    *    Not completely implemented yet.
    */
   MC2String getNameOfNeededRight( const Item& item,
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
    *   @name Get the name of a given Item.
    */
   //@{
   /**
    *   Get the first name as string of a given Item.
    *   @param   item  The item whos name will be returned.
    *   @return  Pointer to the name of item. <i>@b NB! This name
    *            must not be deleted.</i>
    */
   inline const char* getFirstItemName(const Item* item) const;

   /**
    *   Get the first name as string of a given Item.
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
   inline const char* getItemName( const Item* item,
                                   LangTypes::language_t strLang,
                                   ItemTypes::name_t strType ) const;

   inline uint32 getNativeLanguageIndex( const Item& item,
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
    *             for anme string, false if not.
    */
   bool itemHasNameAndType(Item* item,
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
   inline const char* getBestItemName( uint32 itemID, 
                                       LangTypes::language_t strLang ) const;

   inline const char* getBestItemName( uint32 itemID, 
                                vector<LangTypes::language_t> &strLang ) const;


   inline const char* getBestItemName( const Item* item, 
                                       LangTypes::language_t strLang ) const;

   inline const char* getBestItemName( const Item* item, 
                                vector<LangTypes::language_t> &strLang ) const;
         
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
   inline const char* getBestItemName( const Item* item, 
                                       ItemTypes::name_t strType,
                                       LangTypes::language_t strLang) const;

   inline const char* getBestItemName( const Item* item, 
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
    *    Checks if at least one name is same regardless of case for
    *    two items. Does not count synonym names as names.
    *    @param   item        One item.
    *    @param   otherItem   Another item.
    *    @return  True if at least one name was same, 
    *             regardless of case.
    */
   bool oneNameSameCase( const Item* item, 
                         const Item* otherItem ) const;
      
   /**
    *    Checks if at least one word of one name is same regardless 
    *    of case for two items. Does not count synonym names as names.
    *    @param   item        One item.
    *    @param   otherItem   Another item.
    *    @return  True if at least one word of one name was same, 
    *             regardless of case.
    */
   bool oneNameSimilarCase( const Item* item, 
                            const Item* otherItem ) const;

   /**
    *   Get the external connections of this map.
    *   @return  Pointer to the vector with the external 
    *            connections.
    */
   inline BoundrySegmentsVector* getBoundrySegments() const;
      
   /**
    *    If the boundry segments vector is NULL, create a new one.
    */
   void createBoundrySegmentsVector();

   /**
    *   Checks if the supplied item is part of a street or not.
    *   @param   item  The item to check if it's part of a street.
    *   @return  True if the item was part of a street, 
    *            false otherwise.
    */
   bool isPartOfStreet(const Item* item) const;
      
   /**
    *   Checks if the supplied item is part of a street with gfxdata 
    *   or not.
    *   @param   item  The item to check if it's part of a street with
    *                  gfxdata..
    *   @return  True if the item was part of a street with gfxdata, 
    *            false otherwise.
    */
   bool isPartOfGfxStreet(const Item* item) const;

   /**
    *    Counts the number of streets the specified item is part of.
    *    @param   item  The item.
    *    @return  The number of streets the item is part of.
    */
   uint32 partOfNbrStreets(const Item* item) const;
      
   /**
    *   Get the memory usage for this object (including all the 
    *   child-objects).
    *
    *   @return The total memory usage for the map.
    */
   virtual uint32 getMemoryUsage() const;

   uint32 getLaneConnectionIdx( uint32 conId ) const;
   uint32 getLaneConnectionIdx( const Connection& con ) const;

   typedef STLUtility::Array< ExpandStringLane > NodeLaneMapArray;
   typedef STLUtility::ValuePair< uint32, NodeLaneMapArray > NodeLaneMapPair;
   typedef vector< NodeLaneMapPair, MC2Map::Allocator< NodeLaneMapPair > >
      NodeLaneMap;
   const NodeLaneMapArray* getNodeLanes( uint32 nodeId ) const;

   typedef STLUtility::Array< SignPost > ConnectionSignPostMapArray;
   typedef STLUtility::ValuePair< uint32, ConnectionSignPostMapArray >
      ConnectionSignPostMapPair;
   typedef vector< ConnectionSignPostMapPair,
                   MC2Map::Allocator< ConnectionSignPostMapPair > >
      ConnectionSignPostMap;
   const ConnectionSignPostMapArray* getConnectionSigns( uint32 conID ) const;
   

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
   Connection* getOpposingConnection(Connection* conn, 
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
   bool getOtherConnections( Connection* conn, 
                             uint32 toNodeID,
                             vector<pair<Connection*,Node*> >& otherConn );
      
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
                        Connection*& conn) const;
      
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
   bool getConnectionData( Connection* conn, Node* toNode,
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
   bool getConnectionCost(Connection* conn, Node* toNode,
                          uint32& costA, uint32& costB,
                          uint32& costC, uint32& costD,
                          bool externalConnection = false) const;
       
   //@}

   /// @return connection id
   uint32 getConnectionID( const Connection& con ) const;


   /**
    *    Create all allocators that are used to store the Items, 
    *    Connections, Nodes and GfxDatas.
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
    * @param nbrLanes nbrLanes
    * @param nbrSignPosts nbrSignPosts.
    */
   virtual void initNonItemAllocators( uint32 nbrGfxDatas, 
                                       uint32 nbrGfxDataSingleSmallPoly,
                                       uint32 nbrGfxDataSingleLine, 
                                       uint32 nbrGfxDataSinglePoint,
                                       uint32 nbrGfxDataMultiplePoints, 
                                       uint32 nbrNodes, 
                                       uint32 nbrConnections,
                                       uint32 nbrCoordinates,
                                       uint32 nbrLanes,
                                       uint32 nbrCategories,
                                       uint32 nbrSignPosts );

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
   const Item* getItemFromItemIdentifier( const ItemIdentifier& ident ) const;
            
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
   bool getItemIdentifierForItem( const Item* item, 
                                  ItemIdentifier& ident ) const;

   uint16 getStreetLeftSideNbrStart( const StreetSegmentItem& item ) const;
   uint16 getStreetLeftSideNbrEnd( const StreetSegmentItem& item ) const;
   uint16 getStreetRightSideNbrStart( const StreetSegmentItem& item ) const;
   uint16 getStreetRightSideNbrEnd( const StreetSegmentItem& item ) const;

   /**
    * Fetches categories for a specific item.
    * @param item the item to get categories for.
    * @param categories the returned categories.
    */
   void getCategories( const Item& item, Categories& categories ) const;
                       
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
   bool isUniqueCityCentre( Item* item );

   /// adds item to zoom vector
   inline void updateZoom( Item& item );

   /**
    *    Returns a reference to the rights table. Should not
    *    be used directly, only be SearchUnitBuilder etc.
    */
   const UserRightsItemTable& getRightsTable() const {
      return *m_userRightsTable;
   }

   /// @return pointer to the item group index array
   inline const uint32* getItemGroups() const;
   /// @return pointer to the item names index array
   inline const uint32* getItemNameIdx() const;
   /// @return pointer to the item in names group index array
   inline const uint32* getItemsInGroup() const;

   /**
    *   Find out if a vehicle is allowed to traverse this connection.
    *   @param   con the connection in question
    *   @param   vehicle  The vehicle to ask for.
    *   @return  True if the vehicle is allowed to traverse this 
    *            connection, false otherwise.
    */
   bool isVehicleAllowed(const Connection& con, ItemTypes::vehicle_t vehicle ) const;
   uint32 getVehicleRestrictions( const Connection& con ) const;

   /**
    * Get information from a specific poi
    * @param POI the POI object to retrieve information about
    * @return allocated poi information
    */
   auto_ptr<POIInfo> getPOIInfo( const PointOfInterestItem* poi ) const;

   /**
    * Gets poi information for a specific item range with filtered
    * key types. 
    * @param begin start of item range
    * @param end end of item range
    * @param keyTypes the types of information that are relevant to the client
    * @return mapping of PointOfInterested::getWASPID to POIInfo data
    */
   map<uint32, POIInfo*> getPOIInfos( vector<Item*>::const_iterator begin,
                                      vector<Item*>::const_iterator end,
                                      const set<uint32>& keyTypes ) const;
   /**
    *    Set all user right items in the map. I.e. replaces existing
    *    user rights with the ones in itemMap.
    *
    *    @param itemMap Maps item ID to user right of that item for all
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

   /** 
    *   The geographicla description af this map.
    */
   GfxData* m_gfxData;
   BoundrySegmentsVector *getSegmentsOnTheBoundry() { 
      return m_segmentsOnTheBoundry; 
   }

   /**
    * Check if an itemID is an index area.
    *
    * @param itemID The ID of the item to check if it is an index area.
    * @return Returns True id the item is an index area.
    */
   bool isIndexArea( uint32 itemID ) const;

   /**
    * Get the index area level for the itemID.
    *
    * @param itemID The ID of the item to get the index area order of.
    * @return Returns the index area order of the item ID if the item itemID
    *         is an index area. If it is, but no index area have been set, 
    *         (MAX_UINT32-1) is returned. If itemID is not an index area,
    *         MAX_UINT32 is returned.
    */
   uint32 getIndexAreaOrder( uint32 itemID ) const;

protected:
   /**
    *   Creates an empty map.
    */  
   GenericMap();



   /**
    *   Create a map with mapID and path. The map is @b not filled
    *   with data from the file. The load-method must be called after
    *   creation!
    *
    *   @param   mapID Id of the map to be loaded.
    *   @param   path The file-path to the map.
    */
   GenericMap(uint32 mapID, const char *path);


   /**
    *   Add all items to the hashTable.
    *   Rebuilding hash table
    */
   void buildHashTable();

   /**
    * Saves map hash table to a file.
    * @param outfile the file descriptor to which the map hash table should be written
    */
   void saveHashTable( int outfile );

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
    * Do fiddling of rights for tetsing purposes in this function that
    * is called at the end of internalLoad.
    */
   void doMapTricks();

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
   bool swapItems( uint32 aID, uint32 bID );

   /**
    *   Create a new item from the data buffer given as parameter and
    *   return that. This exisist mainly to be able to create a GMSMap
    *   (with GMS*Item's) in the subclass without have to rewrite all
    *   code that reads the map from disc.
    *
    *   @param   type        The type of the item that will be created.
    *   @param   dataBuffer  The buffer where the data for the item 
    *                        is loacted.
    *   @param   itemID      ID of the new item.
    *   @return  A new item, created from dataBuffer with type and ID
    *            itemID.
    */
   virtual Item* createNewItem( DataBuffer* dataBuffer );

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
   virtual GfxData* createNewGfxData( DataBuffer* dataBuffer );

   /**
    *   Virtual method that creates a new Connection out of a 
    *   databuffer. This method excists only to make it possible
    *   to create subclasses of this class (e.g. GMSConnection).
    *   @param   dataBuffer  The buffer where the data for the 
    *                        Connection is loacted.
    *   @return  A new Connection.
    */
   virtual Connection* createNewConnection( DataBuffer* dataBuffer );
      

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
   bool getAllItemsWithStringIndex( uint32 strIndex, Vector* result );
      
      
   /**
    *    Get items with the specified name.
    *    @param   name     The name.   
    *    @param   items    Out parameter. Will be filled with items that
    *                      has the same name.
    *    @param   itemType Optional parameter. If specified the items
    *                      must be of this item type.
    */
   void getItemsWithName( const char* name, 
                          set<const Item*>& items,
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
   Item **m_itemsZoom[NUMBER_GFX_ZOOMLEVELS];

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
   ItemNames *m_itemNames;

   /**
    *    Approximate map size.
    */
   uint32 m_approxMapSize;
      
   /**
    *   Pointer to the hashtable uesed to search for items close
    *   to a location.
    *   TODO: remove mutable
    */
   mutable auto_ptr<MapHashTable> m_hashTable;

   /**
    *   An array with pointers to the BoundrySegments-objects for
    *   this map. Every item in this ObjVector contains information 
    *   about one @c StreetSegmentItem, including possible 
    *   connections.
    */
   BoundrySegmentsVector* m_segmentsOnTheBoundry;

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
       

private:
   /**
    *  If the item is a ssi any number name will have their
    *  language changes to represent name priority
    *  @param item The item to modify.
    *  @return True if ssi and numbernames had no priorities.
    */
   bool modifyItemNumberName( Item& item );

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
   bool getSingleConnectionLength(Connection* conn, Node* toNode,
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
   bool getSingleConnectionTime(Connection* conn, Node* toNode,
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

public:
   /**
    *   Get a pointer to the itemnames of this map.
    *   @return  Pointer to the names on this map.
    */
   inline ItemNames* getItemNames() const;
private:

   /**
    *    Help method for getSingleConnectionLength() that is 
    *    used to calculate
    *    the length between a BusRouteItem and a StreetsegmentItem
    *    based on the offset of the BusRouteItem and the distance of
    *    the ssi.
    *    @param   bri         The BusRouteItem,
    *    @param   dist        The length of the streetsegmentitem.
    *    @param   isSSINode0  True if the streetsegmentitem's node0
    *                         is closest to the BusRouteItem, false
    *                         otherwise.
    *    @return  The distance.
    */
   float64 calcBusSSILength(BusRouteItem* bri, float64 dist, 
                            bool isSSINode0) const;

protected:
   /**
    *    @name Allocators
    *    Pointers to the allocators that are used to allocate
    *    memory for the items in the map. These must be initiated 
    *    before the items in the map are created.
    *    Should be typedeffed for easier changing in cpp file.
    */
   //@{
   /// Allocator for the street segment items
   ItemAllocator<StreetSegmentItem>* m_streetSegmentItemAllocator;

   /// Allocator for the street items
   ItemAllocator<StreetItem>* m_streetItemAllocator;

   /// Allocator for the municipal items
   ItemAllocator<MunicipalItem>* m_municipalItemAllocator;

   /// Allocator for the districts (cityPartItem)
   ItemAllocator<CityPartItem>* m_cityPartItemAllocator;

   /// Allocator for the water items
   ItemAllocator<WaterItem>* m_waterItemAllocator;

   /// Allocator for the park items
   ItemAllocator<ParkItem>* m_parkItemAllocator;

   /// Allocator for the forest items
   ItemAllocator<ForestItem>* m_forestItemAllocator;

   /// Allocator for the building items
   ItemAllocator<BuildingItem>* m_buildingItemAllocator;

   /// Allocator for the railway items
   ItemAllocator<RailwayItem>* m_railwayItemAllocator;

   /// Allocator for the island items
   ItemAllocator<IslandItem>* m_islandItemAllocator;

   /// Allocator for the zip code items
   ItemAllocator<ZipCodeItem>* m_zipCodeItemAllocator;

   /// Allocator for the zip area items
   ItemAllocator<ZipAreaItem>* m_zipAreaItemAllocator;

   /// Allocator for the point of interest items
   ItemAllocator<PointOfInterestItem>* m_pointOfInterestItemAllocator;

   /// Allocator for the category items
   ItemAllocator<CategoryItem>* m_categoryItemAllocator;

   /// Allocator for the build up area items
   ItemAllocator<BuiltUpAreaItem>* m_builtUpAreaItemAllocator;

   /// Allocator for the bus route items
   ItemAllocator<BusRouteItem>* m_busRouteItemAllocator;

   /// Allocator for the airport items
   ItemAllocator<AirportItem>* m_airportItemAllocator;

   /// Allocator for the aircraftRoadItem items
   ItemAllocator<AircraftRoadItem>* m_aircraftRoadItemAllocator;

   /// Allocator for the pedestrianAreaItem items
   ItemAllocator<PedestrianAreaItem>* m_pedestrianAreaItemAllocator;

   /// Allocator for the militaryBaseItem items
   ItemAllocator<Item>* m_militaryBaseItemAllocator;

   /// Allocator for the individualBuildingItem items
   ItemAllocator<IndividualBuildingItem>*
                              m_individualBuildingItemAllocator;

   /// Allocator for the subway line items
   ItemAllocator<SubwayLineItem>* m_subwayLineItemAllocator;

   /// Allocator for the NULL-items
   ItemAllocator<NullItem>* m_nullItemAllocator;

   /// Allocator for the ferry items
   ItemAllocator<FerryItem>* m_ferryItemAllocator;

   /// Allocator for item types that are part of class Item
   ItemAllocator<Item>* m_simpleItemAllocator;
   /// Allocator for item types that are Cartographic items
   ItemAllocator<CartographicItem>* m_cartographicItemAllocator;


   typedef std::vector<DataBufferMapObject* > ItemAllocatorContainer;
   ItemAllocatorContainer m_itemAllocators;

   /// Allocator for the GfxDatas
   std::auto_ptr<GfxDataFactory> m_gfxDataFactory;

   /// Allocator for the connections
   MC2Allocator<class Connection>* m_connectionAllocator;


   MC2Allocator<TileMapCoord>* m_coordinateAllocator;

   /// the category type
   typedef uint16 CategoryID;

   /// Allocator for category arrays
   typedef MC2ArrayAllocator<CategoryID> CategoryAllocator;
   CategoryAllocator* m_categoryAllocator;

   /// Allocator for lanes
   MC2ArrayAllocator< class ExpandStringLane >* m_laneAllocator;

   /// Allocator for SignPosts
   MC2ArrayAllocator< class SignPost >* m_signPostAllocator;

   /// Contains the user rights for items
   UserRightsItemTable* m_userRightsTable;

   /// IDTranslation table. Empty in non-overview maps
   IDTranslationTable m_idTranslationTable;

   /// Default argument to functions taking UserRightsMapInfo
   UserRightsMapInfo* m_allRight;

   /// Let the create-method access these members
   friend Item* Item::createNewItem( DataBuffer&, GenericMap& );
   friend Connection* Connection::createNewConnection( DataBuffer&, 
                                                       GenericMap& );

   //@}
   uint32 m_poiInfoStartOffset; //< start offset of POI Info data in file
   /**
    * @param buffer with poi info
    */
   void copyPOIInfoData( const DataBuffer& buffer );
private:
   void loadSignPosts( DataBuffer& buff );
   void saveSignPosts( int outfile );
   void loadLanes( DataBuffer& buff );
   void saveLanes( int outfile );
   void loadCategoryIds( DataBuffer& buff );
   void saveCategoryIds( int outfile ) const;
   void loadRoadDisplayClasses( DataBuffer& buff );
   void saveRoadDisplayClasses( int outfile ) const;
   void loadAreaDisplayClasses( DataBuffer& buff );
   void saveAreaDisplayClasses( int outfile ) const;

   /**
    * Load the index area order level map.
    */
   void loadIndexAreaOrders( DataBuffer& buff );

   /**
    * Save the index area order level map.
    */
   void saveIndexAreaOrders( int outfile );


   /**
    *   Maps administrative area item ids to a centre coordinate,
    *   from the city centre that is located in the admin area and
    *   have the same name.
    */
   ScopedArray<adminAreaCentre_t> m_adminAreaCentres;

   /**
    *    The number of elements in m_adminAreaCentres.
    */
   uint32 m_nbrAdminAreaCentres;


   streetSideTable_t m_streetSideTable;

   SimpleArrayObject<uint32> m_groups; ///< Item groups
   SimpleArrayObject<uint32> m_names; ///< Item names
   SimpleArrayObject<uint32> m_itemsInGroup; ///< Item in a group

   /// POI Offset Table, offsets into file
   SimpleArrayObject<uint32> m_poiOffsetTable;

   /// The stl Allocator for this map
   MC2Map::Allocator< char > m_stlAllocator;
 
//   typedef std::map<uint32, SignPostArray *> SignPostMap;
//   SignPostMap  m_signPostMap; ///< sign posts in Connections

   /// The lanes for a segment
   NodeLaneMap m_nodeLane;

   typedef vector< STLUtility::ValuePair< uint32, uint32 >, MC2Map::Allocator< 
      STLUtility::ValuePair< uint32, uint32 > > > ConnectionLaneMap;
   /// The lanes going thru a connection
   ConnectionLaneMap m_connectionLaneIdx;

   /// an array holder for categories
   typedef STLUtility::Array< CategoryID > CategoryArray;
   /// The value_type of categories
   typedef STLUtility::ValuePair< uint32, CategoryArray > CategoryMapPair;
   /// maps item id to category id
   typedef vector< CategoryMapPair, 
                   MC2Map::Allocator< CategoryMapPair > > CategoryMap;
   CategoryMap m_categoryIds;

   ConnectionSignPostMap m_connectionSignPost;

   /// vehicle restrictions on connections
   typedef SimpleArrayObject<uint32> VehicleRestrictionArray;
   VehicleRestrictionArray m_vehicleRestrictions;
   auto_ptr<DataBuffer> m_poiInfoData; ///< holds entire memory of poi info data

   /// The index area order table , itemID -> index area order.
   typedef vector< STLUtility::ValuePair< uint32, uint32 >, MC2Map::Allocator< 
      STLUtility::ValuePair< uint32, uint32 > > > IndexAreaOrderMap;

   /// Lookup for itemID to index area order.
   IndexAreaOrderMap m_indexAreaOrderMap;
};

// ========================================================================
//                                      Implementation of inlined methods =

inline const map<GenericMap::multiNodes_t, GenericMap::expandedNodes_t>*
GenericMap::getNodeExpansionTable() const
{
   return &m_nodeExpansionTable;
}

inline uint32 
GenericMap::getNbrItemsWithZoom(uint32 z) const 
{
   if (z<NUMBER_GFX_ZOOMLEVELS)
      return (m_itemsZoomSize[z]);
   else
      return (0);
}

inline uint32 
GenericMap::getNbrItems() const 
{
   uint32 retVal = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++)
      retVal += m_itemsZoomSize[z];
   return (retVal);
}

inline Item* 
GenericMap::getItem(uint32 zoom, uint32 index) const 
{
   if (index < getNbrItemsWithZoom(zoom))
      return (m_itemsZoom[zoom][index]);
   else
      return (NULL);
}

inline Item*
GenericMap::itemLookup(uint32 itemID) const 
{
   if ((itemID & 0x07ffffff) < 
      getNbrItemsWithZoom((itemID & 0x78000000) >> 27))
      return m_itemsZoom[(itemID & 0x78000000) >> 27]
                      [itemID & 0x07ffffff];
   else
      return NULL;
}
    
inline const GfxData* 
GenericMap::getGfxData() const
{
   return (m_gfxData);
}

inline void 
GenericMap::setGfxData( GfxData* mapGfx )
{
   m_gfxData = mapGfx;
}


inline GfxDataFactory& GenericMap::getGfxDataFactory() {
   return *m_gfxDataFactory.get();
}

inline GenericMap::landmarkTable_t&
GenericMap::getLandmarkTable()
{
   return (m_landmarkTable);
}
inline const GenericMap::landmarkTable_t&
GenericMap::getLandmarkTable() const
{
   return (m_landmarkTable);
}

inline const uint32* 
GenericMap::getItemGroups() const {
   return m_groups.data();
}

inline const uint32*
GenericMap::getItemsInGroup() const {
   return m_itemsInGroup.data();
}

inline const uint32*
GenericMap::getItemNameIdx() const {
   return m_names.data();
}


inline MapHashTable* 
GenericMap::getHashTable() const
{
   return m_hashTable.get();
}

inline ItemNames* 
GenericMap::getItemNames() const 
{
   return (m_itemNames);
}

inline const char* 
GenericMap::getFirstItemName(const Item* item) const 
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
GenericMap::getFirstItemName(uint32 itemID) const 
{
   return (getFirstItemName(itemLookup(itemID)));
}

inline const char* 
GenericMap::getName(uint32 stringIndex) const
{
   if (m_itemNames != NULL) {
      return (m_itemNames->getString(stringIndex));
   } else {
      throw GenericMapError("Map has not been loaded!");
   }
}

inline const char*
GenericMap::getRawName(uint32 rawIndex) const
{
   if ( rawIndex == MAX_UINT32 ) {
      return NULL;
   } else {
      return getName(GET_STRING_INDEX(rawIndex));
   }
}

inline const char* 
GenericMap::getBestItemName(const Item* item, 
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
GenericMap::getBestItemName(const Item* item, 
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
GenericMap::getBestItemName(uint32 itemID) const
{
   return (getBestItemName(itemID, LangTypes::invalidLanguage));
}

inline const char* 
GenericMap::getBestItemName(uint32 itemID, LangTypes::language_t strLang) const
{
   return (getBestItemName(itemLookup(itemID), strLang));
}

inline const char* 
GenericMap::getBestItemName(uint32 itemID, 
                            vector<LangTypes::language_t> &strLang) const
{
   return (getBestItemName(itemLookup(itemID), strLang));
}


inline const char* 
GenericMap::getBestItemName(const Item* item, 
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
GenericMap::getBestItemName(const Item* item, 
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

inline uint32 GenericMap::
getNativeLanguageIndex( const Item& item,
                        ItemTypes::name_t strType ) const {
   // Get the name with correct type and language
   uint32 stringIndex = MAX_UINT32;

   // Try the languages of the map
   if ( ! m_nativeLanguages.empty() ) {
      uint32 mapLangNbr = 0;
      while ( (stringIndex == MAX_UINT32) && 
              (mapLangNbr < m_nativeLanguages.size())) {
         stringIndex = item.getNameWithType(strType, m_nativeLanguages[ mapLangNbr ] );
         mapLangNbr++;
      }
   }

   return stringIndex;
}

inline const char* 
GenericMap::getItemName( const Item* item,
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

   if ( stringIndex == MAX_UINT32 ) {
      // Try the languages of the map
      stringIndex = getNativeLanguageIndex( *item, strType );
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
GenericMap::getItemName( uint32 itemID,
                         LangTypes::language_t strLang,
                         ItemTypes::name_t strType ) const 
{
   return getItemName( itemLookup( itemID ), strLang, strType );
}

inline BoundrySegmentsVector* 
GenericMap::getBoundrySegments() const 
{
   return m_segmentsOnTheBoundry;
}

inline void
GenericMap::updateZoom( Item& item ) {
   int zoom = (item.getID() & 0x78000000) >> 27;
   m_itemsZoom[ zoom ][ item.getID() & 0x07FFFFFF ] = &item;
}

#endif

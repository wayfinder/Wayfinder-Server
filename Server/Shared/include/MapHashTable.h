/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPHASHTABLE_H
#define MAPHASHTABLE_H

#include <set>

#include "ItemTypes.h"
#include "LocationHashTable.h"
#include "NotCopyable.h"
#include "SimpleArrayObject.h"

class GenericMap;
class GfxData;
class GfxDataFull;
class Item;
class MapHashTable;
class MC2BoundingBox;
class UserRightsMapInfo;
class DataBuffer;

class OldMapHashTable;

/**
  *   Class that represent a cell in the MapHashTable. The cell has
  *   functionality for finding an item at a specific location.
  *   Supports functions for selecting one, several or all types of
  *   items in the answer.
  *
  *   Do not use the MapHashTable directly, use the functions in GenericMap.
  *
  *   Is not completely thread safe yet. The allowed item
  *   types are stored in the HashTable. Also bboxes and GfxDataFull.
  *
  */
class MapHashCell : public LocationHashCell, private NotCopyable {
public:

   /**
    *   Constructor. Allocates memory for the elements in this cell
    *   @param   hashTable   Pointer to the hashtable where this cell
    *                        is  located.
    */
   MapHashCell(MapHashTable* hashTable);

   /**
    *   Destructor, deletes allocated memory.
    */
   virtual ~MapHashCell();

   /**
    *   Get an element at a specific index.
    *
    *   @param   index    The index in the element-array.
    *   @return  The Item id at position index.
    */
   inline uint32 getItemID( uint32 index ) const;

   /**
    *    Find out if this cell contains any element that is valid to
    *    return. E.g. any element of the given types.
    *    @return True if this cell contains any valid element, false
    *            otherwise.
    */
   bool containsValidElement() const;

   /**
    *   Checks if the specified itemID exists in this cell and
    *   that it is of an allowed type.
    *   @param   itemID   The itemid of the item.
    *   @return  True if the item was allowed, false otherwise.
    */
   inline bool isItemAllowed(uint32 itemID);

   /**
    *    Checks if the specified itemID exists in this cell and
    *    that it is of an allowed type.
    *    @param itemID The itemID of the item.
    *    @parma type   A set of allowed itemTypes.
    *    @return True if the item is allowed.
    */
   inline bool isItemAllowed(uint32 itemID,
                             const set<ItemTypes::itemType>& types);
      
   /**
    *    Get the memory usage of this object.
    *    @return The number of bytes used by this object.
    */ 
   virtual uint32 getMemoryUsage() const;
   /// create this from a data buffer
   void load( DataBuffer& buff );
   /// save internal data to buffer
   void save( DataBuffer& buff ) const;

   bool operator == (const MapHashCell& other ) const;
   bool operator != ( const MapHashCell& other ) const {
      return ! ( *this == other );
   }

private:
   /**
    *   Implementation of abstract method. Calculate the minimum 
    *   distance from the boundingbox to (hpos, vpos).
    *
    *   @param   index    The array-offset for a hashelement in this cell.
    *   @param   hpos     The horizontal coordinate.
    *   @param   vpos     The vertical coordinate.
    *   @param   maxdist  The max dist for which the distance
    *                     will be calculated.
    *   @return  The minimum distance to (hpos, vpos).
    *            Or MAX_UINT64 if not less than maxdist.
    */
   uint64 minSquaredistToBoundingbox(uint32 index, const int32 hpos,
                                     const int32 vpos,
                                     uint64 maxdist = MAX_UINT64 );

   /**
    *   Implementation of abstract method. Calculate the maximum 
    *   distance from the boundingbox to (hpos, vpos).
    *
    *   @param   index    The array-offset for a hashelement in this cell.
    *   @param   hpos     The horizontal coordinate.
    *   @param   vpos     The vertical coordinate.
    *   @return  The maximum distance to (hpos, vpos).
    */
   uint64 maxSquaredistToBoundingbox(uint32 index, const int32 hpos,
                                     const int32 vpos );

   /**
    *   Implementation of abstract method. Calculate the closest 
    *   distance from the element to (hpos, vpos).
    *
    *   @param   index    The array-offset for a hashelement in this cell.
    *   @param   hpos     The horizontal coordinate.
    *   @param   vpos     The vertical coordinate.
    *   @return  The closest distance to (hpos, vpos).
    */
   uint64 minSquaredist(uint32 index, const int32 hpos, 
                        const int32 vpos);

   /**
    *    Checks if an item is inside (part of or the whole item) 
    *    the specified boundingbox. Implementation of abstract method. 
    *    @param   index The index of the item to check.
    *    @param   bbox  The bounding box.
    *    @return  True if part or the whole item was inside the bounding
    *             box.
    */
   bool insideMC2BoundingBox(uint32 index, const MC2BoundingBox* bbox);

   /**
    *   Index in to array with item id's.
    */
   uint32 m_allItemIDIndex;
   uint32 m_allItemIDSize;


   /**
    *   Pointer to the map where the items are stored.
    */
   MapHashTable* m_hashTable;
   // so it can be verified with M3Creator.
   friend bool operator == ( const MapHashTable& first, 
                             const OldMapHashTable& old );
   // our builder
   friend class MapHashTableBuildable;
};


/**
  *   Class that represent a hashtable for Items.
  *
  */
class MapHashTable : public LocationHashTable, private NotCopyable {
public:
   typedef SimpleArray<uint32> ItemIDArray;

   explicit MapHashTable( GenericMap& map );

   /**
    *   Constructor. Calculates the parameters of the hashingfunction 
    *   by using the parameters.
    *
    *   @param   hmin  Minimum horizontal coordinate of the boundingbox
    *   @param   hmax  Maximum horizontal coordinate of the boundingbox
    *   @param   vmin  Minimum vertical coordinate of the boundingbox
    *   @param   vmax  Maximum vertical coordinate of the boundingbox
    *   @param   nbrVerticalCells     The preferred number of vertical 
    *                                 cells.
    *   @param   nbrHorizontalCells   The preferred number of horizontal
    *                                 cells.
    *   @param   theMap   Pointer to the map where the items are located.
    */
   MapHashTable( int32 hmin, int32 hmax, int32 vmin, int32 vmax,
                 uint32 nbrVerticalCells, uint32 nbrHorizontalCells, 
                 GenericMap* theMap );
      
   /**
    *   Destructor. Frees memory allocated by the cells.
    */
   virtual ~MapHashTable();

   /**
    *   Finds an element closest to (hpos, vpos).
    *
    *   @param   hpos        Horizontal position to match
    *   @param   vpos        Vertical position to match
    *   @param   closestDist Distance to the found gfxitem.
    *   @return  The closest element to (hpos, vpos). If no element 
    *            is found NULL is returned, this is however extremely 
    *            rare. 
    */
   uint32 getClosest( int32 hpos, int32 vpos, uint64 &closestDist );

   /**
    *   Clears all allowed itemTypes.\\
    *   {\bf NB!} After this call all item types are allowed!
    *
    */
   void clearAllowedItemTypes();

   /**
    *   Adds one itemType to the set of allowed types.
    */
   void addAllowedItemType(ItemTypes::itemType t);

   /**
    *    Clears allowed item types and adds the types in the set.
    *    @param types The allowed item types for this run.
    */
   void setAllowedItemTypes(const set<ItemTypes::itemType>& types,
                            const UserRightsMapInfo* rights = NULL );
      
   /**
    *   Checks if the itemtype is inlined.
    *   @param t The itemtype
    *   @return True if the itemtype was allowed, false otherwise.
    */
   bool isItemTypeAllowed(ItemTypes::itemType t);


   /// save to a databuffer
   void save( DataBuffer& buff ) const;
   /// load from a databuffer
   void load( DataBuffer& buff );
   /// @return size in databuffer
   uint32 getSizeInDataBuffer() const;

   /**
    *   Finds all elements inside the circle defined by (hpos, vpos) and 
    *   radius.
    *   @param   hpos     The horizontal part of the coordinate for the
    *                     center of the circle.
    *   @param   vpos     The vertical part of the coordinate for the
    *                     center of the circle.
    *   @param   radius   The maximum distance in meters from 
    *                     (hpos, vpos) to the items returned by this 
    *                     method.
    *   @param   shouldKill  Is set to true if the Vector* returned
    *                     by this methos should be deleted by the caller.
    *                     Set to false if the caller {\bf must not}
    *                     delete the Vector.
    *   @return  Pointer to a Vector containing IDs of the items within
    *            a distance < radius from (hpos, vpos).
    */  

   Vector* getAllWithinRadius_meter(int32 hpos, 
                                    int32 vpos, 
                                    int32 radius,
                                    bool &shouldKill);
      
   /**
    *    Get the memoryusage of this object.
    *   @return The number of bytes used by this object.
    */
   virtual uint32 getMemoryUsage() const;

      
   /**
    *   Finds all elements inside the circle defined by (hpos, vpos) and 
    *   radius.
    *   @param   hpos     The horizontal part of the coordinate for the
    *                     center of the circle.
    *   @param   vpos     The vertical part of the coordinate for the
    *                     center of the circle.
    *   @param   radius   The maximum distance in MC2 scale from 
    *                     (hpos, vpos) to the items returned by this 
    *                     method.
    *   @param   shouldKill  Is set to true if the Vector* returned
    *                     by this methos should be deleted by the caller.
    *                     Set to false if the caller {\bf must not}
    *                     delete the Vector.
    *   @return  Pointer to a Vector containing IDs of the items within
    *            a distance < radius from (hpos, vpos).
    */
   Vector* getAllWithinRadius_MC2Scale(int32 hpos, 
                                       int32 vpos, 
                                       int32 radius,
                                       bool &shouldKill);

   /**
    *    Returns the gfxData for the item or a pointer to a
    *    fake GfxData.
    */
   const GfxData* getGfxData(const Item* item);

   bool operator == ( const MapHashTable& other ) const;
   bool operator != ( const MapHashTable& other ) const {
      return ! ( *this == other );
   }

   const ItemIDArray& getItemIDs() const { return m_itemIDs; }

protected:
   // so buildable map hash table can build ...
   ItemIDArray& getItemIDs() { return m_itemIDs; }
private:
   /**
    *   Some coordinate-systems are scaled horizontally.
    *
    *   @return  The horizontalFactor cosine for the current
    *            latitude.
    */
   double getHorizontalFactor();


   /**
    *   Used to get a item with a specified id. The returned item has
    *   a valid type, otherwise NULL is returned. NB! If no item 
    *   types has been set allowed then all are supposed to be valid!
    *   This method should not be used, since it is not thread safe.
    *   This method is inlined.
    *
    *   @param   itemID   ID of the wanted item.
    *   @return  Pointer to the item with itemID {\bf if} it is a 
    *            valid id {\bf and} it has allowed type -- false 
    *            otherwise.
    */
   inline Item* itemLookup(uint32 itemID);

   /**
    *   Vector with the supported types.
    */
   Vector m_allowedItemTypes;

   /**
    *   Pointer to the map where the items are stored.
    */
   GenericMap* m_map;

   /**
    *    Current user rights.
    */
   UserRightsMapInfo* m_rights;

   /**
    *    Fake GfxData to be used for some POI:s that do not have
    *    a real GfxData. Contains one coordinate and is not closed.
    */
   GfxDataFull* m_fakeGfxData;

   friend bool operator == ( const MapHashTable& first, 
                             const OldMapHashTable& other );
   void deleteTable();
   void init();
   // array with all item id groups aligned
   SimpleArrayObject<uint32> m_itemIDs;

   friend class MapHashCell;

};


inline bool operator != ( const MapHashTable& first,
                          const OldMapHashTable& second ) {
   return ! ( first == second );
}

// =======================================================================
//                                     Implementation of inlined methods =

inline
uint32 MapHashCell::getItemID( uint32 index ) const {
   return m_hashTable->getItemIDs()[ m_allItemIDIndex + index ];
}

#endif

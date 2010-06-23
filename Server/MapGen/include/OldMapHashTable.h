/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDMAPHASHTABLE_H
#define OLDMAPHASHTABLE_H

#include <set>

#include "ItemTypes.h"
#include "LocationHashTable.h"

class OldGenericMap;
class GfxData;
class GfxDataFull;
class OldItem;
class OldMapHashTable;
class MC2BoundingBox;
class UserRightsMapInfo;

class MapHashTable;

/**
  *   Class that represent a cell in the OldMapHashTable. The cell has
  *   functionality for finding an item at a specific location.
  *   Supports functions for selecting one, several or all types of
  *   items in the answer.
  *
  *   Do not use the OldMapHashTable directly, use the functions in OldGenericMap.
  *
  *   Is not completely thread safe yet. The allowed item
  *        types are stored in the HashTable. Also bboxes and GfxDataFull.
  *
  */
class OldMapHashCell : public LocationHashCell {
   public:

      /**
        *   Constructor. Allocates memory for the elements in this cell
        *   @param   hashTable   Pointer to the hashtable where this cell
        *                        is  located.
        */
      OldMapHashCell(OldMapHashTable* hashTable);

      /**
        *   Destructor, deletes allocated memory.
        */
      virtual ~OldMapHashCell();

      /**
       *    Reallocates the storage vectors.
       *    @return The number of removed positions.
       */
      int trimToSize();
   
      /**
        *   Add an element to the hashcell.
        *
        *   @param   itemID   The element to be added
        */
      void addElement(uint32 itemID);

      /**
       *    Find out if this cell contains any element that is valid to
       *    return. E.g. any element of the given types.
       *    @return True if this cell contains any valid element, false
       *            otherwise.
       */
      virtual bool containsValidElement() const;

      /**
        *   Get an element at a specific index.
        *
        *   @param   index    The index in the element-array.
        *   @return  The OldItem id at position index.
        */
      inline uint32 getItemID(uint32 index) const;

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
        *   Debugmethod that prints the IDs of all items in this
        *   hash cell.
        */
      inline void dump();

      /**
       *    Get the memory usage of this object.
       *    @return The number of bytes used by this object.
       */ 
      virtual uint32 getMemoryUsage() const;
      

   protected:
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
        *   Array with item id's.
        */
      Vector* m_allItemID;

      /**
        *   Pointer to the map where the items are stored.
        */
      OldMapHashTable* m_hashTable;

   friend bool operator == ( const MapHashTable& first, 
                             const OldMapHashTable& old );

};


/**
  *   Class that represent a hashtable for OldItems.
  *
  */
class OldMapHashTable : public LocationHashTable {

   friend class OldMapHashCell;

   public:
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
      OldMapHashTable( int32 hmin, int32 hmax, int32 vmin, int32 vmax,
                    uint32 nbrVerticalCells, uint32 nbrHorizontalCells, 
                    OldGenericMap* theMap );
      
      /**
        *   Destructor. Frees memory allocated by the cells.
        */
      virtual ~OldMapHashTable();

      /**
       *    Trims the storage of the elements to the actual size.
       *    @return The number of elements removed.
       */
      int trimToSize();
        
      /**
        *   Adds one item to the hashtable. Handles poi:s internally.
        *
        *   @param   item     The item to be added.
        *   @param   itemID   ID of the item ot be added.
        */
      void addItem(const OldItem* item);

      /**
       *    Adds one item to the HashTable.
       *
       *    @param   bbox     The bbox to be added.
       *    @param   itemID   ID of the item ot be added.       
       */
      void addItem( const MC2BoundingBox* bbox, uint32 itemID );

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

      /**
        *   Debugmethod that prints the hashtable to standard out.
        */
      void dump();

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
      const GfxData* getGfxData(const OldItem* item);

   protected:
      /**
        *   Some coordinate-systems are scaled horizontally.
        *
        *   @return  The horizontalFactor cosine for the current
        *            latitude.
        */
      inline double getHorizontalFactor();


   private:
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
      inline OldItem* itemLookup(uint32 itemID);

      /**
        *   Vector with the supported types.
        */
      Vector m_allowedItemTypes;

      /**
        *   Pointer to the map where the items are stored.
        */
      OldGenericMap* m_map;

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
                             const OldMapHashTable& old );
};



// =======================================================================
//                                     Implementation of inlined methods =


#endif

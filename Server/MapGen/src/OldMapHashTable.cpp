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

#include "OldMapHashTable.h"
#include "OldGenericMap.h"
#include "GfxConstants.h"
#include "GfxData.h"
#include "GfxDataFull.h"
#include "UserRightsMapInfo.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - -OldMapHashTable -
// -- Inlined functions for OldMapHashTable
inline const GfxData*
OldMapHashTable::getGfxData( const OldItem* item )
{
   return m_map->getItemGfx( item, *m_fakeGfxData );
}

bool
OldMapHashTable::isItemTypeAllowed(ItemTypes::itemType t)
{
   return (m_allowedItemTypes.linearSearch(uint32(t)) != MAX_UINT32);
}

inline double 
OldMapHashTable::getHorizontalFactor() 
{
   MC2BoundingBox bb;
   m_map->getGfxData()->getMC2BoundingBox(bb);
   return bb.getCosLat();
}

inline OldItem* 
OldMapHashTable::itemLookup(uint32 itemID) 
{
   OldItem* item = m_map->itemLookup(itemID);   

   if ( m_rights ) {
      if ( ! m_map->itemAllowedByUserRights( itemID, *m_rights ) ) {
         mc2dbg8 << "[MHT]: OldItem " << MC2HEX( itemID )
                 << " forbidden by rights" << endl;
         return NULL;
      }
   }
   
   if ((m_allowedItemTypes.getSize() == 0) || (item == NULL)) {
      return (item);
   } else if (m_allowedItemTypes.linearSearch(
               uint32(item->getItemType())) < MAX_UINT32) {
      return (item);
   } else {
      return (NULL);
   }
}

/* ************************************************* */
/* ***************** OldMapHashCell ******************* */
/* ************************************************* */

inline uint32 
OldMapHashCell::getItemID(uint32 index) const 
{
   return (m_allItemID->getElementAt(index));
}

inline bool
OldMapHashCell::isItemAllowed(uint32 itemID)
{
   return (m_hashTable->itemLookup(itemID) != NULL);
}

inline void 
OldMapHashCell::dump() 
{
   m_allItemID->dump(false);
}

OldMapHashCell::OldMapHashCell(OldMapHashTable* hashTable) : LocationHashCell()
{
   this->m_hashTable = hashTable;
   m_nbrElements = 0;

   //the vector starts at 64, increase-step 64
   m_allItemID = new Vector(64, 64); 
}

OldMapHashCell::~OldMapHashCell()
{
   delete m_allItemID;
   DEBUG_DEL(mc2dbg << "~OldMapHashCell: m_allItemID destr." << endl);
}

void
OldMapHashCell::addElement(uint32 itemID)
{
   m_allItemID->addLast(itemID);
   m_nbrElements++;
}

int
OldMapHashCell::trimToSize()
{
   int before = m_allItemID->getAllocSize();
   m_allItemID->trimToSize();
   return before - m_allItemID->getAllocSize();
}

bool 
OldMapHashCell::containsValidElement() const
{
   uint32 i=0;
   bool found = false;
   while ((i<m_nbrElements) && (!found)) {
      found = (m_hashTable->itemLookup(m_allItemID->getElementAt(i++)) != NULL);
   }
   return (found);
}

uint32 
OldMapHashCell::getMemoryUsage() const 
{
   uint32 size = LocationHashCell::getMemoryUsage() -
      sizeof(LocationHashCell) + sizeof(OldMapHashCell);
   if ( m_allItemID != NULL )
      size += m_allItemID->getMemoryUsage();
   return size;
}


uint64 OldMapHashCell::minSquaredistToBoundingbox( uint32 index, const int32 hpos,
                                               const int32 vpos,
                                                uint64 maxdist)
{
   MC2_ASSERT(index < m_allItemID->getSize());

   // Will return if not valid type for item at position index.
   OldItem* item = m_hashTable->itemLookup(m_allItemID->getElementAt(index));

   if (item == NULL)
      return MAX_UINT64;

   // Optimized a bit so that no extra variables are used.
   MC2_ASSERT(item != NULL);
   MC2_ASSERT(m_hashTable->getGfxData(item) != NULL);

   //     This is just a quick fix. The methods should be implemented in
   //     the GfxData instead. Static variables are not thread safe...
   static MC2BoundingBox bb;
   m_hashTable->getGfxData(item)->getMC2BoundingBox(bb);
   return (bb.squareMC2ScaleDistTo( vpos, hpos ));
}


uint64 OldMapHashCell::maxSquaredistToBoundingbox( uint32 index, const int32 hpos,
                                                const int32 vpos )
{
   MC2_ASSERT(index < m_allItemID->getSize());

   // Will return if not valid type for item at position index.
   OldItem* item = m_hashTable->itemLookup(m_allItemID->getElementAt(index));
   if (item == NULL)
      return MAX_UINT64;

   // Optimized a bit so that no extra variables are used.
   MC2_ASSERT(item != NULL);
   MC2_ASSERT(m_hashTable->getGfxData(item) != NULL);
   //MC2_ASSERT(item->getGfxData()->getMC2BoundingBox() != NULL);
   //return (item->getGfxData()->getMC2BoundingBox()
   //            ->maxMC2ScaleSquareDistTo( vpos, hpos ));

   //     This is just a quick fix. The methods should be implemented in
   //     the GfxData instead. Static variables are not thread safe...
   static MC2BoundingBox bb;
   m_hashTable->getGfxData(item)->getMC2BoundingBox(bb);
   return (bb.maxMC2ScaleSquareDistTo( vpos, hpos ));
}


uint64 
OldMapHashCell::minSquaredist( uint32 index, const int32 hpos,
                            const int32 vpos )
{
   MC2_ASSERT(index < m_allItemID->getSize());

   // Will return if not valid type for item at position index.
   OldItem* item = m_hashTable->itemLookup(m_allItemID->getElementAt(index));
   if (item == NULL)
      return false;

   // Optimized a bit so that no extra variables are used.
   MC2_ASSERT(item != NULL);
   const GfxData* gfx = m_hashTable->getGfxData( item );
   MC2_ASSERT( gfx != NULL );

   // Return 0 if the item is inside.
   int64 res = gfx->signedSquareDistTo(vpos, hpos);  
   if ( res < 0 ) {
      return 0;
   } else {
      // Remember to use the correct unit...
      return uint64(res * GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
   }
}


bool
OldMapHashCell::insideMC2BoundingBox(uint32 index, const MC2BoundingBox* bbox) 
{
   MC2_ASSERT(index < m_allItemID->getSize());

   // Will return if not valid type for item at position index.
   OldItem* item = m_hashTable->itemLookup(m_allItemID->getElementAt(index));
   if (item == NULL)
      return false;

   const GfxData* gfx = m_hashTable->getGfxData( item );
   MC2_ASSERT(gfx != NULL);

   bool inside = false;
   
   MC2BoundingBox gfxBBox;
   gfx->getMC2BoundingBox(gfxBBox);
   // To speed up things:
   // First check if bbox overlaps the boundingbox of the GfxData
   // If they don't then item is not inside.
   // Otherwise we must check each coordinate.
   if (bbox->overlaps(gfxBBox)) {
      // Check every coordinate
   
      // It's NOT enough that just one coordinate is inside the boundingbox.
      // The distances must be checked to minimize the number of included 
      // items.
      /*uint32 i = 0; 
      while ((i < gfx->getNbrCoordinates()) && (!inside)) {
         if (bbox->contains(gfx->getLat(i), gfx->getLon(i))) {
            inside = true;
         } else {
            i++;
         }
      }*/
      inside = true;

   } else {
      // Boundingboxes does not even overlap.
      inside = false;
   }

   return (inside);
}


/***************************************************/
/****************** OldMapHashTable *******************/
/***************************************************/

OldMapHashTable::OldMapHashTable(int32 hmin, int32 hmax, int32 vmin, int32 vmax,
                           uint32 nbrVerticalCells, uint32 nbrHorizontalCells,
                           OldGenericMap* theMap)
                           : LocationHashTable(hmin, hmax, vmin, vmax,
                                               nbrVerticalCells,
                                               nbrHorizontalCells)
{
   //create the array (vertically)
   m_hashCell = (LocationHashCell***)(new OldMapHashCell**[m_nbrVerticalCells]);
   m_map = theMap;
   m_rights = NULL;

   for( uint32 v = 0; v < m_nbrVerticalCells; v++ ) {
      //create the array horizontally
      m_hashCell[v] = (LocationHashCell**) 
               new OldMapHashCell*[m_nbrHorizontalCells];

      //Store the boundingboxes
      for( uint32 h = 0; h < m_nbrHorizontalCells; h++ ) {
         //convert hashindex to  boundingbox
         int32 minHoriz, maxHoriz, minVert, maxVert;

         invHashIndex(minHoriz, minVert, h, v);
         invHashIndex(maxHoriz, maxVert, h+1, v+1);

         m_hashCell[v][h] = new OldMapHashCell(this);
         
         m_hashCell[v][h]->setMC2BoundingBox(minHoriz,
            maxHoriz, minVert, maxVert);
      }
   }
   // Set up the fake GfxData.
   m_fakeGfxData = new GfxDataFull();
   m_fakeGfxData->setClosed( 0, false );
   m_fakeGfxData->addPolygon();
   m_fakeGfxData->quickAddCoordinate( 0, 0, 1 );
}

OldMapHashTable::~OldMapHashTable()
{
   for( uint32 v = 0; v < m_nbrVerticalCells; v++ ) {
      for( uint32 h = 0; h < m_nbrHorizontalCells; h++ ) {
         delete m_hashCell[v][h];
      }
      delete [] m_hashCell[v];
      DEBUG_DEL(mc2dbg << "~OldMapHashTable: m_hashCell[" << v << "] destr." 
                     << endl);
   }
   delete [] m_hashCell;
   DEBUG_DEL(mc2dbg << "~OldMapHashTable: m_hashCell destr." << endl);
   delete m_fakeGfxData;
   clearAllowedItemTypes();
}

Vector* 
OldMapHashTable::getAllWithinRadius_MC2Scale(int32 hpos, 
                                          int32 vpos, 
                                          int32 radius,
                                          bool &shouldKill) 
{
   return (LocationHashTable::getAllWithinRadius(hpos, 
                                                 vpos, 
                                                 radius, 
                                                 shouldKill));
}

int
OldMapHashTable::trimToSize()
{
   int res = 0;
   for( uint32 v = 0; v < m_nbrVerticalCells; v++ ) {
      for( uint32 h = 0; h < m_nbrHorizontalCells; h++ ) {
         res += static_cast<OldMapHashCell*>(m_hashCell[v][h])->trimToSize();
      }
   }
   return res;
}

void
OldMapHashTable::addItem( const MC2BoundingBox* bbox,
                       uint32 itemID )
{
   uint32 hHashStart, vHashStart, hHashEnd, vHashEnd;
   
   //Get indexes for hashcells to put elements in
   getHashIndex(bbox->getMinLon(), bbox->getMinLat(), hHashStart, vHashStart);
   getHashIndex(bbox->getMaxLon(), bbox->getMaxLat(), hHashEnd, vHashEnd);

   for( uint32 v = vHashStart; v <= vHashEnd; v++ ) {
      for( uint32 h = hHashStart; h <= hHashEnd; h++ ) {
         //Add element to cell(s)
         ((OldMapHashCell*)m_hashCell[v][h])->addElement(itemID);
      }
   }

}

void
OldMapHashTable::addItem(const OldItem* item)
{
   if ( item == NULL ) {
      return;
   }
   // Create uninitialised bbox on stack.
   MC2BoundingBox bbox( false, false );
   const GfxData* gfx = getGfxData( item );
   if ( gfx == NULL ) {
      return;
   }
   
   gfx->getMC2BoundingBox( bbox );   
   uint32 itemID = item->getID();
   addItem( &bbox, itemID );
}


uint32
OldMapHashTable::getClosest(int32 hpos, int32 vpos, uint64 &closestDist)
{
   uint32 offset;

   LocationHashCell* cell = LocationHashTable::getClosest(hpos, 
                                                          vpos, 
                                                          offset,
                                                          closestDist);

   if ((cell != NULL) && (offset < cell->getNbrElements())) {
      mc2dbg4 << "OldMapHashTable::getClosest offset=" << offset 
                  << ", closestDist=" << closestDist << endl;
      return ((OldMapHashCell*)cell)->getItemID((uint32) offset);
   } else {
      return (MAX_UINT32);
   }
}

Vector* 
OldMapHashTable::getAllWithinRadius_meter(int32 hpos, 
                                      int32 vpos, 
                                      int32 radius,
                                      bool &shouldKill) 
{
   radius = (int32) (radius * GfxConstants::METER_TO_MC2SCALE);
   return (LocationHashTable::getAllWithinRadius(hpos, 
                                                 vpos, 
                                                 radius, 
                                                 shouldKill));
}


uint32 
OldMapHashTable::getMemoryUsage() const 
{
   uint32 sum = 0;
   sum += LocationHashTable::getMemoryUsage()-sizeof(LocationHashTable);
   sum += sizeof(OldMapHashTable) - sizeof(m_allowedItemTypes);
   sum += m_allowedItemTypes.getMemoryUsage();
   return sum;
}



void 
OldMapHashTable::dump() 
{
   cout << "MapHashtable" << endl;
   for( uint32 v = 0; v < m_nbrVerticalCells; v++ ) {
      for( uint32 h = 0; h < m_nbrHorizontalCells; h++ ) {
         cout << "Cell [" << v << ", " << h << "]" << endl;
         ((OldMapHashCell*)m_hashCell[v][h])->dump();
      }
   }

}

void 
OldMapHashTable::clearAllowedItemTypes() 
{
   m_allowedItemTypes.reset();
   delete m_rights;
   m_rights = NULL;
}

void
OldMapHashTable::setAllowedItemTypes(const set<ItemTypes::itemType>& types,
                                  const UserRightsMapInfo* rights )
{
   clearAllowedItemTypes();
   
   for(set<ItemTypes::itemType>::const_iterator it = types.begin();
       it != types.end();
       ++it ) {
      // Already unique.
      m_allowedItemTypes.push_back(*it);
   }
   
   if ( rights ) {
      m_rights = new UserRightsMapInfo( *rights );
   }
}

void 
OldMapHashTable::addAllowedItemType(ItemTypes::itemType t) 
{
   m_allowedItemTypes.addLastIfUnique( uint32(t));      
}


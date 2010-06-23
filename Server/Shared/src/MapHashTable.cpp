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

#include "MapHashTable.h"
#include "GenericMap.h"
#include "GfxConstants.h"
#include "GfxData.h"
#include "GfxDataFull.h"
#include "UserRightsMapInfo.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - -MapHashTable -
// -- Inlined functions for MapHashTable
const GfxData*
MapHashTable::getGfxData( const Item* item )
{
   return m_map->getItemGfx( item, *m_fakeGfxData );
}

inline bool
MapHashTable::isItemTypeAllowed(ItemTypes::itemType t)
{
   return (m_allowedItemTypes.linearSearch(uint32(t)) != MAX_UINT32);
}

inline double 
MapHashTable::getHorizontalFactor() 
{
   MC2BoundingBox bb;
   m_map->getGfxData()->getMC2BoundingBox(bb);
   return bb.getCosLat();
}

inline Item* 
MapHashTable::itemLookup(uint32 itemID) 
{
   Item* item = m_map->itemLookup(itemID);   

   if ( m_rights ) {
      if ( ! m_map->itemAllowedByUserRights( itemID, *m_rights ) ) {
         mc2dbg8 << "[MHT]: Item " << MC2HEX( itemID )
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
/* ***************** MapHashCell ******************* */
/* ************************************************* */


inline bool
MapHashCell::isItemAllowed(uint32 itemID)
{
   return (m_hashTable->itemLookup(itemID) != NULL);
}

MapHashCell::MapHashCell(MapHashTable* hashTable):
   m_hashTable( hashTable )
{
   m_nbrElements = 0;
}

MapHashCell::~MapHashCell()
{
}

bool 
MapHashCell::containsValidElement() const 
{
   for ( uint32 i = 0; i < m_nbrElements; ++i ) {
      if ( m_hashTable->itemLookup( getItemID( i ) ) != NULL ) {
         return true;
      }
   }
   return false;
}

uint32 
MapHashCell::getMemoryUsage() const 
{
   uint32 size = LocationHashCell::getMemoryUsage() -
      sizeof(LocationHashCell) + sizeof(MapHashCell);

   size += 2 * sizeof(uint32); // allItemIDSize + allItemIDIndex
   return size;
}


uint64 MapHashCell::minSquaredistToBoundingbox( uint32 index, const int32 hpos,
                                                const int32 vpos,
                                                uint64 maxdist)
{
   MC2_ASSERT( index < m_allItemIDSize );


   // Will return if not valid type for item at position index.
   Item* item = m_hashTable->itemLookup( getItemID( index ) );

   if (item == NULL)
      return MAX_UINT64;

   // Optimized a bit so that no extra variables are used.
   MC2_ASSERT( item != NULL );
   MC2_ASSERT( m_hashTable->getGfxData( item ) != NULL );
   //MC2_ASSERT(item->getGfxData()->getMC2BoundingBox() != NULL);
   //return (item->getGfxData()->getMC2BoundingBox()
   //            ->squareMC2ScaleDistTo( vpos, hpos ));

   // TODO: This is just a quick fix. The methods should be implemented in
   //       the GfxData instead. Static variables are not thread safe...
   static MC2BoundingBox bb;
   m_hashTable->getGfxData(item)->getMC2BoundingBox(bb);
   return (bb.squareMC2ScaleDistTo( vpos, hpos ));
}


uint64 MapHashCell::maxSquaredistToBoundingbox( uint32 index, const int32 hpos,
                                                const int32 vpos )
{
   MC2_ASSERT(index < m_allItemIDSize );
   
   // Will return if not valid type for item at position index.
   Item* item = m_hashTable->itemLookup( getItemID( index ) );
   if (item == NULL)
      return MAX_UINT64;

   // Optimized a bit so that no extra variables are used.
   MC2_ASSERT(item != NULL);
   MC2_ASSERT(m_hashTable->getGfxData(item) != NULL);
   //MC2_ASSERT(item->getGfxData()->getMC2BoundingBox() != NULL);
   //return (item->getGfxData()->getMC2BoundingBox()
   //            ->maxMC2ScaleSquareDistTo( vpos, hpos ));

   // TODO: This is just a quick fix. The methods should be implemented in
   //       the GfxData instead. Static variables are not thread safe...
   static MC2BoundingBox bb;
   m_hashTable->getGfxData(item)->getMC2BoundingBox(bb);
   return (bb.maxMC2ScaleSquareDistTo( vpos, hpos ));
}


uint64 
MapHashCell::minSquaredist( uint32 index, const int32 hpos,
                            const int32 vpos )
{
   MC2_ASSERT( index < m_allItemIDSize );

   // Will return if not valid type for item at position index.
   Item* item = m_hashTable->itemLookup( getItemID( index ) );
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
MapHashCell::insideMC2BoundingBox(uint32 index, const MC2BoundingBox* bbox) 
{
   MC2_ASSERT( index < m_allItemIDSize );

   // Will return if not valid type for item at position index.
   Item* item = m_hashTable->itemLookup( getItemID( index ) );
   if ( item == NULL ) {
      return false;
   }

   const GfxData* gfx = m_hashTable->getGfxData( item );
   MC2_ASSERT(gfx != NULL);

   bool inside = false;
   
   //const MC2BoundingBox* gfxBBox = gfx->getMC2BoundingBox();
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
/****************** MapHashTable *******************/
/***************************************************/
MapHashTable::MapHashTable( GenericMap& map ):
   m_map( &map ),
   m_rights( NULL ) {
}

MapHashTable::MapHashTable(int32 hmin, int32 hmax, int32 vmin, int32 vmax,
                           uint32 nbrVerticalCells, uint32 nbrHorizontalCells,
                           GenericMap* theMap):
   LocationHashTable( hmin, hmax, vmin, vmax,
                      nbrVerticalCells,
                      nbrHorizontalCells ),
   m_map( theMap ),
   m_rights( NULL )
{
   init();
}

MapHashTable::~MapHashTable()
{
   deleteTable();
   delete m_fakeGfxData;
   clearAllowedItemTypes();
}

Vector* 
MapHashTable::getAllWithinRadius_MC2Scale(int32 hpos, 
                                          int32 vpos, 
                                          int32 radius,
                                          bool &shouldKill) 
{
   return (LocationHashTable::getAllWithinRadius(hpos, 
                                                 vpos, 
                                                 radius, 
                                                 shouldKill));
}

uint32
MapHashTable::getClosest(int32 hpos, int32 vpos, uint64 &closestDist)
{
   uint32 offset;

   LocationHashCell* cell = LocationHashTable::getClosest(hpos, 
                                                          vpos, 
                                                          offset,
                                                          closestDist);

   if ((cell != NULL) && (offset < cell->getNbrElements())) {
      mc2dbg4 << "MapHashTable::getClosest offset=" << offset 
                  << ", closestDist=" << closestDist << endl;
      return ((MapHashCell*)cell)->getItemID((uint32) offset);
   } else {
      return (MAX_UINT32);
   }
}

Vector* 
MapHashTable::getAllWithinRadius_meter(int32 hpos, 
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
MapHashTable::getMemoryUsage() const 
{
   uint32 sum = 0;
   sum += LocationHashTable::getMemoryUsage()-sizeof(LocationHashTable);
   sum += sizeof(MapHashTable) - sizeof(m_allowedItemTypes);
   sum += m_allowedItemTypes.getMemoryUsage();
   return sum;
}


void 
MapHashTable::clearAllowedItemTypes() 
{
   m_allowedItemTypes.reset();
   delete m_rights;
   m_rights = NULL;
}

void
MapHashTable::setAllowedItemTypes(const set<ItemTypes::itemType>& types,
                                  const UserRightsMapInfo* rights )
{
   clearAllowedItemTypes();

   set<ItemTypes::itemType>::const_iterator it = types.begin();
   set<ItemTypes::itemType>::const_iterator end = types.end();
   for (; it != end; ++it ) {
      // Already unique.
      m_allowedItemTypes.push_back(*it);
   }
   
   if ( rights ) {
      m_rights = new UserRightsMapInfo( *rights );
   }
}

void 
MapHashTable::addAllowedItemType(ItemTypes::itemType t) 
{
   m_allowedItemTypes.addLastIfUnique( uint32(t));      
}

void MapHashTable::save( DataBuffer& buff ) const 
{
   mc2dbg << "MapHashTable: Saving" << endl;
   buff.writeNextLong( m_leftHorizontal );
   buff.writeNextLong( m_rightHorizontal );

   buff.writeNextLong( m_bottomVertical );
   buff.writeNextLong( m_topVertical );

   buff.writeNextLong( m_nbrVerticalCells );
   buff.writeNextLong( m_nbrHorizontalCells );

   buff.writeNextLong( m_horizontalShift );
   buff.writeNextLong( m_verticalShift );

   m_itemIDs.save( buff );

   for ( uint32 v = 0; v < m_nbrVerticalCells; ++v ) {
      for ( uint32 h = 0; h < m_nbrHorizontalCells; ++h ) {
         static_cast< MapHashCell* >( m_hashCell[ v ][ h ] )->save( buff );
      }
   }
   mc2dbg << "done" << endl;
}

void MapHashTable::load( DataBuffer& buff ) 
{
   deleteTable();

   m_leftHorizontal = buff.readNextLong();
   m_rightHorizontal = buff.readNextLong();

   m_bottomVertical = buff.readNextLong();
   m_topVertical = buff.readNextLong();

   m_nbrVerticalCells = buff.readNextLong();
   m_nbrHorizontalCells = buff.readNextLong();

   m_horizontalShift = buff.readNextLong();
   m_verticalShift = buff.readNextLong();

   m_itemIDs.load( buff );

   init();

   // format: 
   // v * h * mapHashCell format
   for ( uint32 v = 0; v < m_nbrVerticalCells; ++v ) {
      for ( uint32 h = 0; h < m_nbrHorizontalCells; ++h ) {
         static_cast< MapHashCell* >( m_hashCell[ v ][ h ] )->load( buff );
      }
   }
}

void MapHashCell::save( DataBuffer& buff ) const
{
   buff.writeNextLong( m_allItemIDIndex );
   buff.writeNextLong( m_allItemIDSize );
}

void MapHashCell::load( DataBuffer& buff ) 
{
   buff.readNextLong( m_allItemIDIndex );
   buff.readNextLong( m_allItemIDSize );
   m_nbrElements = m_allItemIDSize;
}

uint32 MapHashTable::getSizeInDataBuffer() const
{
   // hmin+hmax+vmin+vmax + vertCell + horizCell + 
   // hor/vert shift
   uint32 sum = 32; 

   sum += m_itemIDs.getSizeInDataBuffer();

   // each cell * 4 ( 4 = index + size )
   sum += m_nbrVerticalCells * m_nbrHorizontalCells * 2*sizeof(uint32);
   mc2dbg << "mapHashTable size: " << sum << endl;
   return sum;
}

bool MapHashTable::operator == ( const MapHashTable& other ) const {
   if ( &other == this ) {
      return true;
   }

#define _NEQ( x ) if ( x != other.x ) { \
cout << "MapHashTable item: " << #x << " differs. " \
 << " new/old: " << x << "/" << other.x << endl; \
 return false; \
}
   _NEQ( m_nbrHorizontalCells );
   _NEQ( m_nbrVerticalCells );
   _NEQ( m_map->getMapID() );
   // LocationHashTable members:
   _NEQ( m_verticalShift );
   _NEQ( m_horizontalShift );
   _NEQ( m_topVertical );
   _NEQ( m_bottomVertical );
   _NEQ( m_rightHorizontal );
   _NEQ( m_leftHorizontal );
   
   for ( uint32 v = 0; v < m_nbrVerticalCells; ++v ) {
      for ( uint32 h = 0; h < m_nbrHorizontalCells; ++h ) {
         MapHashCell& cell = 
            static_cast< MapHashCell& >( *m_hashCell[ v ][ h ] );
         MapHashCell& otherCell =
            static_cast< MapHashCell& >( *other.m_hashCell[ v ][ h ] );
         if ( cell != otherCell ) {
            return false;
         }
      }
   }


   return true;
}


bool MapHashCell::operator == ( const MapHashCell& other ) const {
   if ( &other == this ) {
      return true;
   }

#undef _NEQ
#define _NEQ( x ) if ( x != other.x ) { \
cout << "MapHashCell item: " << #x << " differs. " \
 << " new/old: " << x << "/" << other.x << endl; \
 return false; \
}
    // LocationHashCell members:
    _NEQ( m_nbrElements ); 
    _NEQ( m_topVertical );
    _NEQ( m_bottomVertical );
    _NEQ( m_rightHorizontal );
    _NEQ( m_leftHorizontal );

    /*   
   for ( uint32 i = 0; i < m_allItemID.size(); ++i ) {
      _NEQ( m_allItemID[ i ] );
   }
    */
   return true;

#undef _NEQ

}

void MapHashTable::deleteTable() {
   for( uint32 v = 0; v < m_nbrVerticalCells; v++ ) {
      for( uint32 h = 0; h < m_nbrHorizontalCells; h++ ) {
         delete m_hashCell[v][h];
      }
      delete [] m_hashCell[v];
   }

   delete [] m_hashCell;
   m_hashCell = NULL;
}


void MapHashTable::init() {
   // assumes m_hashCell is deleted before calling this.

   //create the array (vertically)
   m_hashCell = (LocationHashCell***)(new MapHashCell**[m_nbrVerticalCells]);
   for( uint32 v = 0; v < m_nbrVerticalCells; v++ ) {
      //create the array horizontally
      m_hashCell[v] = (LocationHashCell**) 
               new MapHashCell*[m_nbrHorizontalCells];

      //Store the boundingboxes
      for( uint32 h = 0; h < m_nbrHorizontalCells; h++ ) {
         //convert hashindex to  boundingbox
         int32 minHoriz, maxHoriz, minVert, maxVert;

         invHashIndex(minHoriz, minVert, h, v);
         invHashIndex(maxHoriz, maxVert, h+1, v+1);

         m_hashCell[v][h] = new MapHashCell(this);
         
         m_hashCell[v][h]->
            setMC2BoundingBox( minHoriz, maxHoriz, minVert, maxVert );
      }
   }
   // Set up the fake GfxData.
   m_fakeGfxData = new GfxDataFull();
   m_fakeGfxData->setClosed( 0, false );
   m_fakeGfxData->addPolygon();
   m_fakeGfxData->quickAddCoordinate( 0, 0, 1 );
}

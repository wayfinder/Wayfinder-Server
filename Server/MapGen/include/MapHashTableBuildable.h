/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPHASHTABLEBUILDABLE_H
#define MAPHASHTABLEBUILDABLE_H

#include "config.h"
#include "MapHashTable.h"
#include <vector>
#include <map>

/**
 * This class is used to build up the real MapHashTable
 * The MapHashTable is really slow to build up from scratch so
 * this class uses a STL vector matrix to build fast and then set 
 * the MapHashTable matrix.
 * It should only be used with M3Creator.
 * Call buildTable() once all items have been added.
 */
class MapHashTableBuildable: public MapHashTable {
public:
   typedef vector< uint32 > CellType;
   typedef vector< vector< CellType > > CellMatrix;
   typedef map< CellType, uint32 > CellOffsetMap;

   MapHashTableBuildable( uint32 minLon, uint32 maxLon,
                          uint32 minLat, uint32 maxLat,
                          uint32 nbrCellsX, uint32 nbrCellsY, 
                          GenericMap* map );
   void addItem( const Item* item );
   // add to a faster vector
   void addItem( const MC2BoundingBox* bbox, uint32 itemID );

   const CellMatrix& getCells() const { return m_cells; }
   /**
    * Sets new item ids and all MapHashCells index in to this array
    * using the offset map.
    * @param newIDs will be swapped with the old one
    * @param offsetMap maps array to offset
    */
   void setItemIDs( ItemIDArray& newIDs, const CellOffsetMap& offsetMap);
   uint32 getNbrHorizontalCells() const { return m_nbrHorizontalCells; }
   uint32 getNbrVerticalCells() const { return m_nbrVerticalCells; }
private:
   CellMatrix m_cells;
};

#endif 


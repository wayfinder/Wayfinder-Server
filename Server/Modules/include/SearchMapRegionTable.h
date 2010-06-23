/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHMAPREGIONTABLE_H
#define SEARCHMAPREGIONTABLE_H

#include "config.h"

#include <map>
#include <set>
#include "ValuePair.h"

class DataBuffer;

typedef uint32* SearchMapRegionIterator;

class SearchMapRegionTable {

public:

   /**
    *   Currently 24 bits are used for the index and
    *   8 bits for the number of regions, which makes
    *   it possible to keep approx 16 millions of combinations.
    */
   static const uint32 maxNbrRegionsInTable = 0x00ffffff;
   
   /**
    *   Creates a new, empty SearchMapRegionTable.
    *   The only way to add someting to it is to use
    *   a load-function.
    */
   SearchMapRegionTable();

   /**
    *   Destructor.
    */
   virtual ~SearchMapRegionTable();

   /**
    *   Saves the table in a DataBuffer.
    */
   int save(DataBuffer& buff) const;

   /**
    *   Loads the table from a DataBuffer.
    */
   int load(DataBuffer& buff);

   /**
    *   Returns the number of bytes needed to save
    *   this object into a DataBuffer.
    */
   int getSizeInDataBuffer() const;

   /**
    *   Returns an iterator to the start of 
    *   the region combo at index idx. Should not be called
    *   by any other than the SearchMapItems.
    */
   const SearchMapRegionIterator begin(uint32 idx) const;

   /**
    *   Returns an iterator past the end
    *   of the region combo at index idx. Should not be called
    *   by any other than the SearchMapItems.
    */
   const SearchMapRegionIterator end(uint32 idx) const;

   /**
    *   Returns region number <code>nameNbr</code> in
    *   combination number <code>idx</code>.
    */
   uint32 getRegion(int nameNbr, uint32 idx);
   
   /**
    *   Returns the number of regions for the
    *   requested index.
    */
   int getNbrRegions(uint32 idx) const;

   /**
    *   Returns the number of combinations in the table.
    */
   int getNbrCombos() const;

   /**
    * Makes the overflow lookup table for the region table.
    */
   void fixupRegionCombo();
   
protected:

   /**
    *   Creates the type index to return from the start position
    *   in the array and the number of regions.
    */
   static uint32 makeIndex(uint32 posInArray, uint32 nbrRegions);
     
   /**
    *   Returns the offset from the combined index.
    */
   static uint32 getOffset(uint32 index);
         
   /**
    *   All the regions in a row.
    */
   uint32* m_allRegions;

   /**
    *   The number of all regions.
    */
   uint32 m_nbrAllRegions;

   /**
    *   The number of combinations.
    */
   uint32 m_nbrCombos;
   
   /**
    * Type of container for overflow regionindeces.
    */
   typedef map< uint32, uint16 > overflowTable_t;

   /**
    * The region indeces that have more than, or equal to, 0xff regions.
    */
   overflowTable_t m_overflowTable;

   typedef STLUtility::ValuePair< uint32, uint16 > overflowVector_t;
   /**
    * The overflow table in read only version, from load.
    */
   overflowVector_t* m_overflowVector;

   size_t m_nbrOverflow;
};

/**
 *   Class to be used when creating the SearchMapRegionTable
 *   in the MapModule.
 */
class WriteableSearchMapRegionTable : public SearchMapRegionTable {
public:
   /**
    *   Constructor.
    */
   WriteableSearchMapRegionTable();

   /**
    *   Virtual destructor.
    */
   virtual ~WriteableSearchMapRegionTable();
   
   /**
    *   Adds the combination to the region table and
    *   returns and index into the table which can be
    *   used to retreive the information again.
    *   @param nbrRegions The number of regions to add.
    *   @param regions    Pointer to the first region to add.
    *   @return An index in the table.
    */
   uint32 addRegionCombo(const set<uint32>& combo);
   
private:

   /**
    *   Typedef for convenience.
    */
   typedef map<set<uint32>, uint32> lookupTable_t;
   
   /**
    *   Structure used to speed up the adding and avoiding
    *   linear searches. The region combination is in first and
    *   the index of it is in second. The structures in the ordinary
    *   SearchMapRegionTable should also always be updated.
    */
   lookupTable_t m_lookupTable;

   /**
    *   Number of regions that have been allocated so far.
    *   This variable does not exist in the non-writeable version,
    *   since it should not be expanded.
    */
   uint32 m_nbrAllocatedRegions;
};

// -- Inlined methods for SearchMapRegionTable


inline const SearchMapRegionIterator
SearchMapRegionTable::begin(uint32 idx) const
{
   return &m_allRegions[getOffset(idx)];
}

inline const SearchMapRegionIterator
SearchMapRegionTable::end(uint32 idx) const
{
   return &(begin(idx)[getNbrRegions(idx)]);
}

inline uint32
SearchMapRegionTable::getRegion(int nbr, uint32 idx)
{
   return begin(idx)[nbr];
}

#endif

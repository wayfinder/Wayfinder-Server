/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGSEARCHSTRUCT_H
#define STRINGSEARCHSTRUCT_H

#include "config.h"
#include "SearchMap2.h"

#include <map>

class SearchMap2;
class SearchMapItem;
class WriteableSearchMapStringTable;
class DataBuffer;
class MultiSearchNotice;

class MSSTempDataMap; // Is in cpp-file.
class MSSTempData;

#define BEGINNING_OF_WORD 0x01
#define BEGINNING_OF_STR  0x02
#define ANYWHERE_IN_STR   0x04

/**
 *   Searcher that can search using beginning of string and beginning
 *   of word.
 */
class MultiStringSearch {
public:
   /**
    *   Constructor. Use load after this.
    */
   MultiStringSearch(const SearchMap2* searchMap);
   
   /**
    *   Destructor.
    */
   virtual ~MultiStringSearch();
   
   /**
    *   Returns the size of the object in a DataBuffer.
    */   
   int getSizeInDataBuffer() const;

   /**
    *   Loads the object from a DataBuffer.
    */
   int load(DataBuffer& dataBuffer);

   /**
    *   Saves the object into a DataBuffer.
    */
   int save(DataBuffer& dataBuffer) const;

   /**
    *   Returns the index of the notice.
    *   @param notice The notice to get the index from.
    *   @return The index of the notice.
    */
   inline uint32 getIndex(const MultiSearchNotice* notice) const;
   
   /**
    *   Returns a pointer to the infovector table
    *   where the SearchNotice that has endOfItems
    *   begins.
    */
   inline uint32 getStartOffset(const MultiSearchNotice* notice) const;

   /**
    *   Returns a pointer to the infovector table
    *   where the SearchNotice that has endOfItems
    *   begins.
    */
   inline uint32 getEndOffset(const MultiSearchNotice* notice) const;

   /**
    *   Returns the SearchMapItem at index offset in this object.
    */
   inline const SearchMapItem* getSearchMapItem(uint32 offset) const;
   
   /**
    *   Returns the name-number at index idx in the struct.
    */
   inline uint8 getNameNbr(uint32 idx) const;

   /**
    *   Returns the word-number of the notice.
    *   Must be word-match to work, though.
    *   @param notice The notice to get it for.
    */
   inline uint8 getWordNbr(const MultiSearchNotice* notice,
                           int offset) const;

   /**
    *   Returns the searchmask of the item. Beginning of word etc.
    *   @param notice The notice to get it for.
    */
   inline uint8 getStringPartMask(const MultiSearchNotice* notice,
                                  int offset) const;

   
protected:

   /**
    *   Vector of pairs of string index (that can be looked up in
    *   the stringtable) in first and the end index in the
    *   info-vector in second. One position in this vector
    *   corresponds to one name.
    */
   MultiSearchNotice* m_stringIdx;

   /**
    *   The size of the m_stringIdx-array.
    */
   int m_nbrStrIdx;

   /**
    *   The m_stringIdx vector points into this array of item
    *   indeces. Each position in the m_stringIdx corresponds
    *   to one or more items in this vector.
    */
   uint32* m_infoVectorIdx;

   /**
    *   This array contains information about which name
    *   in the item in m_infoVectorIdx the string corresponds to.
    */
   uint8* m_infoVectorNameNbr;

   /**
    *   This array contains the stringpart mask of the items.
    */
   uint8* m_infoVectorStringPartMasks;

   /**
    *   The size of the m_infoVectorIdx, m_infoVectorNameNbr and
    *   m_infoVectorStringPartMasks arrays.
    */
   int m_infoArraySize;

   /**
    *   The searchmap. For string lookups etc.
    */
   const SearchMap2* m_map;
};

#include "SearchNotice.h"

inline uint32
MultiStringSearch::getIndex(const MultiSearchNotice* notice) const
{
   // Pointer arithmetics.
   return notice-m_stringIdx;
}

inline uint32
MultiStringSearch::getStartOffset(const MultiSearchNotice* notice) const
{
   if ( getIndex(notice) == 0 ) {
      return 0;
   } else {
      // Take the previous notice
      --notice;
      return notice->m_endOfItems;
   }
}

inline uint32
MultiStringSearch::getEndOffset(const MultiSearchNotice* notice) const
{
   return notice->m_endOfItems;
}

inline const SearchMapItem*
MultiStringSearch::getSearchMapItem(uint32 idx) const
{
   return m_map->getItemByIndex(m_infoVectorIdx[idx]);
}

inline uint8
MultiStringSearch::getNameNbr(uint32 idx) const
{
   return m_infoVectorNameNbr[idx];
}

inline uint8
MultiStringSearch::getWordNbr(const MultiSearchNotice* notice,
                              int offset) const
{
   const int index = getStartOffset(notice)+offset;
   return (m_infoVectorStringPartMasks[index] >> 3 ) & 0x07;
}

inline uint8
MultiStringSearch::getStringPartMask(const MultiSearchNotice* notice,
                                     int offset) const
{
   const int index = getStartOffset(notice)+offset;
   return m_infoVectorStringPartMasks[index] & 0x07;
}


#endif


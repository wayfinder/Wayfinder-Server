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

#include <map>
#include <set>

#include "WriteableStringSearch.h"
#include "StringSearchUtility.h"
#include "NewStrDup.h"
#include "SearchMap2.h"

#include "SearchNotice.h"

#define STRING_PART_NBR_BITS 3

// -- For creation from SearchMap :

class MSSTempData {
public:

   /**
    *   Creates a new MSSTempData.
    *   @param itemIdx Index in SearchMap for the item added.
    *   @param nameNbr The number of the name in the SearchItem.
    *   @param stringPartMask Bitmask describing which types of search that
    *                         should be able to find the string.
    *   @param wordNbr        Which word is it. Usable when doing allwords.
    */
   MSSTempData(uint32 itemIdx, int nameNbr, uint8 stringPartMask,
               uint8 wordNbr = MAX_UINT8)
         : m_itemIdx(itemIdx), m_nameNbr(nameNbr),
           m_stringPart(stringPartMask), m_wordNumber(MIN(wordNbr, 7))
    
      {
      }

   /**
    *   Sorting order. Is important when inserting into the set.
    *   Does not compare the stringpart since it will be or:ed
    *   together by addMSSTempData.
    */
   bool operator<(const MSSTempData& other) const {
      if ( m_itemIdx < other.m_itemIdx ) return true;
      if ( m_itemIdx > other.m_itemIdx ) return false;
      return m_nameNbr < other.m_nameNbr;

   }
   
   /// Index of the item where the data came from
   uint32 m_itemIdx;

   /// Name number in the item
   uint8 m_nameNbr;
      
   /// How the item should be found.
   uint8 m_stringPart : STRING_PART_NBR_BITS;

   /// Word number (if word) up to 7
   uint8 m_wordNumber : 3;
   
};

/**
 *   Change this comparator to get a different order
 *   of the strings. There may be some small gains to
 *   make by sorting the strings in length-order, since
 *   the end of the strings may be used again.
 *   In that case the indeces must be sorted again before
 *   they can be used for searching.
 *
 */
class MyStrComp {
public:
   bool operator()(const char* a, const char* b) {
      //if ( a.size() > b.size() ) return true;
      //if ( a.size() < b.size() ) return false;
      // Same as when searching later.
      return strcmp(a,b) < 0;
   }
};

class MSSTempDataMap : public map<const char*, set<MSSTempData>, MyStrComp > {

};

WriteableMultiStringSearch::~WriteableMultiStringSearch()
{
   mc2dbg << "[WMSS]: Deleting "
          << m_stringGarbage.size()
          << " string-garbage items" << endl;
   for( vector<char*>::iterator it = m_stringGarbage.begin();
        it != m_stringGarbage.end();
        ++it ) {
      delete [] *it;
   }
   m_stringGarbage.clear();
   mc2dbg << "[WMSS]: String garbage deleted" << endl;
}

inline void
WriteableMultiStringSearch::addMSSTempData(MSSTempDataMap& dataMap,
                                           char* str,
                                           const MSSTempData& data)
{
   int sizeBefore = dataMap.size();
   
   pair<set<MSSTempData>::iterator, bool> addRes =
      dataMap[str].insert(data);
   
   int sizeAfter = dataMap.size();

   
   if ( addRes.second == false ) {
      // The item was not inserted. Or the stringparts together and
      // add it.      
      mc2dbg8 << "[MSS]: Same name - different stringpart" << endl;
      uint8 newStringPart = addRes.first->m_stringPart |
                            data.m_stringPart;
      
      // Only wordmatches have wordnumbers and it should then
      // be less than the other unused number.
      uint8 newWordNbr = MIN(addRes.first->m_wordNumber,
                             data.m_wordNumber);
      dataMap[str].erase(addRes.first);
      addMSSTempData(dataMap, str, MSSTempData(data.m_itemIdx,
                                               data.m_nameNbr,
                                               newStringPart,
                                               newWordNbr));
                     
   } else {
      // The item was inserted - add the string to the garbage
      // to be deleted later
      if ( sizeBefore == sizeAfter ) {
         //delete [] str;
         // Maybe this will decrease the fragmentation?
         m_stringGarbage.push_back(str);
      } else {
         m_stringGarbage.push_back(str);
      }
   }
}

inline void
WriteableMultiStringSearch::addString(const SearchMapItem* item,
                                      const SearchMap2* theMap,
                                      int nameNbr,
                                      MSSTempDataMap& tempStrings)
{
   const char* name = item->getName(theMap, nameNbr);
   // Beginning of string
   {
      // Convert to close and remove the spaces.
      MC2String strippedCloseIdent =
         StringSearchUtility::convertIdentToClose(name,
                                                  false);
      
      addMSSTempData(tempStrings, NewStrDup::newStrDup(strippedCloseIdent),
                     MSSTempData(item->getIndex(theMap),
                                 nameNbr, BEGINNING_OF_STR));
      // The string is deleted in addMSSTempData if needed
   }
   
   // Beginning of word
   {
      if( item->getSearchType(theMap) != SEARCH_ZIP_CODES ) {         
         // Could be nice to have the wordNbr in the map
         // when doing allwords...
         int wordNbr = 0;
         
         vector<MC2String> words;
         StringSearchUtility::splitIntoWords(words, name);
         for ( vector<MC2String>::iterator it = words.begin();
               it != words.end();
               ++it ) {
            MC2String closeString =
               StringSearchUtility::convertIdentToClose(*it);
            
            if ( ! closeString.empty() ) {
               // FIXME: Dup the string inside instead. It knows
               //        there if it is needed.
               addMSSTempData(tempStrings,
                              NewStrDup::newStrDup(closeString),
                              MSSTempData(item->getIndex(theMap),
                                          nameNbr, BEGINNING_OF_WORD,
                                          wordNbr));
               ++wordNbr;
               // The string is deleted in addMSSTempData if needed
            } else {
               // Don't use the empty string
            }
         }
      }
   }
   
}

inline void
WriteableMultiStringSearch::addPermanent(uint32 strIdx,
                                         const MSSTempData& tmpData)
{
   // Add stuff to all vectors.
   
   int idxSize  = m_infoArraySize;
   int idxAlloc = m_infoArrayAllocSize;
   m_infoVectorIdx = ArrayTool::addElement(m_infoVectorIdx,
                                         tmpData.m_itemIdx,
                                         idxSize, idxAlloc);
   int infoSize = m_infoArraySize;
   int infoAlloc = m_infoArrayAllocSize;
   m_infoVectorNameNbr = ArrayTool::addElement(m_infoVectorNameNbr,
                                             uint8(tmpData.m_nameNbr),
                                             infoSize, infoAlloc);
   int masksSize = m_infoArraySize;
   int masksAlloc = m_infoArrayAllocSize;
   // Add stringpart and word number
   uint8 both = (tmpData.m_stringPart) |
                (tmpData.m_wordNumber << STRING_PART_NBR_BITS);
   m_infoVectorStringPartMasks =
      ArrayTool::addElement(m_infoVectorStringPartMasks,
                          both,
                          masksSize, masksAlloc);

   MC2_ASSERT( ( idxSize == infoSize ) && ( infoSize == masksSize ) );
   MC2_ASSERT( ( idxAlloc == infoAlloc ) && ( infoAlloc == masksAlloc ) );
   
   m_infoArraySize      = idxSize;
   m_infoArrayAllocSize = idxAlloc;
   
}

WriteableMultiStringSearch::
WriteableMultiStringSearch(const SearchMap2* searchMap,
                           WriteableSearchMapStringTable* stringTable)
      : MultiStringSearch(searchMap)
{
   m_map                = searchMap;
   m_nbrAllocatedStrIdx = 0;
   m_infoArrayAllocSize = 0;
   
   // Build the temporary structure.
   MSSTempDataMap tempStrings;

   mc2dbg << "[MSS]: Adding search strings " << endl;
   // Iterate over the items in the map.
   for( SearchMap2::const_iterator it = searchMap->begin();
        it != searchMap->end();
        ++it ) {
      const SearchMapItem* curItem = it;
      for( int i=0; i < curItem->getNbrNames(searchMap); ++i ) {
         addString(curItem, searchMap, i, tempStrings);
      }
   }
   
   mc2dbg << "[MSS]: StringTable size = " << stringTable->getTotalStringSize()
          << endl;
   mc2dbg << "[MSS]: Making string table" << endl;
   
   // Add all the strings to the stringtable
   for( MSSTempDataMap::const_iterator it = tempStrings.begin();
        it != tempStrings.end();
        ++it ) {
      stringTable->addItemName(it->first);
   }
   
   
   mc2dbg << "[MSS]: StringTable size = " << stringTable->getTotalStringSize()
          << endl;
   mc2dbg << "[MSS]: StringTable size in databuffer "
          << stringTable->getSizeInDataBuffer() << endl;
   mc2dbg << "[MSS]: " << tempStrings.size() << " string items" << endl;
   mc2dbg << "[MSS]: SearchMap is now " << searchMap->getSizeInDataBuffer()
          << " bytes" << endl;


   // Build the ordinary data structures.
   mc2dbg << "[MSS]: Building real search structures" << endl;

   // Pre-alloc some space so that we don't have to realloc so often.
   // Doesn't seem to help very much thouhg
   m_nbrStrIdx = 0;   
   m_nbrAllocatedStrIdx = tempStrings.size() * 4;  
   m_stringIdx = new MultiSearchNotice[m_nbrAllocatedStrIdx];
   
   for( MSSTempDataMap::const_iterator it = tempStrings.begin();
        it != tempStrings.end();
        ++it ) {
      uint32 strIdx = stringTable->addItemName(it->first);
      for(set<MSSTempData>::const_iterator jt = it->second.begin();
          jt != it->second.end();
          ++jt) {         
         addPermanent(strIdx,
                      *jt);
      }
      m_stringIdx = ArrayTool::addElement(m_stringIdx,
                                        MultiSearchNotice(strIdx,
                                                          m_infoArraySize),
                                        m_nbrStrIdx, m_nbrAllocatedStrIdx);
   }
   
   // Now we should be done, I think.
   mc2dbg << "[MSS]: Size of allIdx " << m_infoArraySize << endl;
}



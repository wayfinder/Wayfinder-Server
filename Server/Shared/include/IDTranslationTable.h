/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IDTRANSLATIONTABLE_H
#define IDTRANSLATIONTABLE_H

#include "config.h"
#include "IDPairVector.h"
#include "DataBufferObject.h"

#include <vector>
#include <cstdlib>
#include <functional>
#if defined (__GNUC__) && __GNUC__ > 2
#include <ext/algorithm>
using namespace __gnu_cxx;
#else
#include <algorithm>
#endif

#include "STLUtility.h"

class DataBuffer;

/**
 *    Handle translation tables and lookups to translate between 
 *    itemID <--> mapID.itemID, typically in the overview maps to
 *    go from higher to lower level and reverse. 
 *
 *    TODO: Clean up!
 */
class IDTranslationTable: public DataBufferObject {
public:
   /**
    *    Create an empty translation table.
    *   @param oldVersion whether to load/save the old version
    *          of the map or not
    */
   explicit IDTranslationTable(int version = 0):
      m_version( version ) { };
   
   /**
    *    Delete memory allocated here (actually nothing).
    */
   virtual ~IDTranslationTable() { };
   
   /**
    *   Returns the size of the table in a DataBuffer.
    *   Both tables are saved.
    */
   uint32 getSizeInDataBuffer() const;
   
   /**
    *   Saves the table into the databuffer. Both tables are saved.
    *   @param dbuf Buffer to save the table into.
    *   @return Number of bytes written.
    */
   void save(DataBuffer& dbuf) const;
   
   /**
    *   Loads the table from the databuffer.
    *   @param dbuf Buffer to load from.
    *   @return Number of bytes read.
    */
   void load(DataBuffer& dbuf);

   /**
    *    Saves the table in the DataBuffer.
    *    @param dataBuffer Buffer to save the table in.
    *    @param twoTables  True if both forward and backward table
    *                      should be saved.
    */
   bool save(DataBuffer* dataBuffer, bool twoTables = false) const;
   /**
    *    Build the translation tables from a DataBuffer. Will
    *    handle both versions of save, i.e. with both reverse and
    *    real table in databuffer or with just one.
    *    @param mapSet The map set ID to encode, pass 0 if not used
    */
   void load(DataBuffer& dataBuffer, uint32 mapSet);

   /**
    *    Translate the lowerID into higher ID. Node number is kept.
    *    @param lowerID Id on low level map.
    *    @return The id on higher level.
    */
   inline uint32 translateToHigher(const IDPair_t& lowerID) const;

   /**
    *    Translate the lowmapID and low item id into higher. Node number
    *    is kept.
    *    @param lowmapID  Lower map id.
    *    @param lowitemID Lower item id.
    *    @return The id on higher level.
    */
   inline uint32 translateToHigher(uint32 lowmapID, uint32 lowitemID) const;

   /**
    *    Translate the high level id <code>higherID</code> into low
    *    level id. Node number is kept.
    *    @param higherID The high level id.
    *    @return The low level id or MAX_UINT32,MAX_UINT32 if not found.
    */
   inline IDPair_t translateToLower(uint32 higherID) const;
   

   typedef pair<uint32, IDPair_t> lookupElement_t;
   typedef vector<lookupElement_t> lookupTable_t;
   typedef vector<uint32> reverseLookupTable_t;
   
      
   uint32 getNbrElements() const {
      return m_lookupTable.size();
   }

   bool getElement(uint32 i,
                   uint32& itemID,
                   uint32& fullMapID,
                   uint32& fullItemID) const {
      if (i < m_lookupTable.size()) {
         lookupElement_t elm = m_lookupTable[m_reverseLookupTable[i]];
         itemID = elm.first;
         fullMapID = elm.second.first;
         fullItemID = elm.second.second;
         return true;
      }
      return false;
   }

   bool addElement(uint32 itemID, uint32 fullMapID, uint32 fullItemID) {
      return addElement( IDPair_t(fullMapID, fullItemID), itemID);
   };

   bool addElement(const IDPair_t& fullID, uint32 itemID) {
      lookupElement_t elm(itemID, fullID);
      m_lookupTable.push_back(elm);
      return true;
   };

   /**
    * Very time consuming.
    */
   bool removeElement( uint32 itemID ) {
      reverseLookupTable_t::iterator search_it =
         lower_bound( m_reverseLookupTable.begin(),
                      m_reverseLookupTable.end(), itemID,
                      SearchLessThanIndex( m_lookupTable ) );

      if ( search_it != m_reverseLookupTable.end() &&
           m_lookupTable[*search_it].first == itemID) {
         lookupTable_t::iterator it = m_lookupTable.begin();
         advance(it,(*search_it));
         m_lookupTable.erase( it );
         m_reverseLookupTable.erase( search_it );
         sortElements();
         return true;
      }
      return false;

   }

   bool sortElements() {
      sort( m_lookupTable.begin(), m_lookupTable.end(), 
            SortLookup() );
      sortReverseTable();

      return true;
   }

private:
   void loadOld( DataBuffer& dbuf, uint32 mapSet );
   void saveOld( DataBuffer& dbuf ) const;

   void loadNew( DataBuffer& dbuf, uint32 mapSet );
   void saveNew( DataBuffer& dbuf ) const;

   void sortReverseTable() {
      // resize and fill array with numbers from 0 to n
      if ( m_reverseLookupTable.size() != m_lookupTable.size() )
         m_reverseLookupTable.resize( m_lookupTable.size() );

      generate_n( m_reverseLookupTable.begin(),
                  m_lookupTable.size(), STLUtility::Counter<uint32>( 0 ) );

      // sort numbers by using the lookupTable
      sort( m_reverseLookupTable.begin(), m_reverseLookupTable.end(),
            SortLessThanIndex( m_lookupTable ) );
   }

   struct SortLessThanIndex {
      SortLessThanIndex(const lookupTable_t& index):m_index(index) { }
      bool operator()( uint32 a, uint32 b ) const {
         return m_index[a].first < m_index[b].first;
      }

      const lookupTable_t& m_index;
   };

   struct SearchLessThanIndex {
      SearchLessThanIndex(const lookupTable_t& index):
         m_index(index) { }
      bool operator()( uint32 a, uint32 b ) const {
         return m_index[a].first < b;
      }
      const lookupTable_t& m_index;
   };


   struct SearchLookup:
      public binary_function<const lookupElement_t, 
      const IDPair_t, bool> {
      bool operator()( const lookupElement_t& x, 
                       const IDPair_t& y ) const {
         return 
            x.second.first < y.first ||
            ( x.second.first == y.first &&
              x.second.second < y.second );
      }
   };


   struct SortLookup:
      public binary_function<const lookupElement_t,
      const lookupElement_t, bool> {
      bool operator()(const lookupElement_t& x,
                      const lookupElement_t& y) const {
         return 
            x.second.first < y.second.first ||
            ( x.second.first == y.second.first &&
              x.second.second < y.second.second );
      }
   };

   /**
    *    Ordered by itemID;
    */
   lookupTable_t m_lookupTable;

   /**
    *    Ordered by IDPair_t;
    */
   reverseLookupTable_t m_reverseLookupTable;

   int m_version;
   friend class M3Creator; // must be able to modifiy old/new version
};

inline uint32
IDTranslationTable::translateToHigher(const IDPair_t& lowerID) const
{
   // Keep the node !
   const uint32 nodePattern = lowerID.second & 0x80000000;
   IDPair_t lookupNode(lowerID.first, lowerID.second & 0x7fffffff);
   lookupTable_t::const_iterator it = 
      lower_bound( m_lookupTable.begin(),
                   m_lookupTable.end(),
                   lookupNode, SearchLookup() );
   if ( it == m_lookupTable.end() ||
        (*it).second != lookupNode ) {
      return MAX_UINT32;
   }
   // add the node-information again  ...?
   return (*it).first | nodePattern;
}

inline uint32
IDTranslationTable::translateToHigher(uint32 lowmapID, 
                                      uint32 lowitemID) const
{   
   return translateToHigher( IDPair_t(lowmapID, lowitemID) );
}

inline IDPair_t
IDTranslationTable::translateToLower(uint32 higherID) const
{
   static const IDPair_t inval(MAX_UINT32, MAX_UINT32);
   const uint32 nodePattern = higherID & 0x80000000;
   const uint32 itemID =      higherID & 0x7fffffff;         
   reverseLookupTable_t::const_iterator it = 
      lower_bound( m_reverseLookupTable.begin(),
                   m_reverseLookupTable.end(),
                   itemID,
                   SearchLessThanIndex( m_lookupTable ) );
   if ( it == m_reverseLookupTable.end() ||
        m_lookupTable[*it].first != itemID ) {
      return inval;
   }

   // add node pattern and return search result
   return IDPair_t(m_lookupTable[*it].second.first,
                   m_lookupTable[*it].second.second | nodePattern);

}


#endif


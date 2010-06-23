/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHMAPSTRINGTABLE_H
#define SEARCHMAPSTRINGTABLE_H

#include "config.h"

#include "STLStringUtility.h"
#include "ItemTypes.h"
#include "NameUtility.h"
#include "ArrayTools.h"
#include "SearchMapRegionTable.h"

#include <set>
#include <map>


class DataBuffer;
class SearchMapRegionTable;
class WriteableSearchMapRegionTable;
class ItemNames;
class GenericMap;

/**
 *   Class keeping the strings of a SearchMap and the SearchMapStructs.
 *   Also keeps combinations of string indeces and nbr of names.
 */
class SearchMapStringTable {
public:
   /**
    *   Creates a new empty SearchMapStringTable.
    */
   SearchMapStringTable();

   /**
    *   Destructor..
    */
   virtual ~SearchMapStringTable();

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
    *   Should only be used by SearchMapItem
    *   Returns the number of names for the index in question.
    */   
   inline int getComboNbrNames(uint32 idx) const;

   /** 
    *   Should only be used by SearchMapItem.
    *   Returns the nameindex for name idx.
    */
   inline uint32 getComboNameIdx(int nameNbr, uint32 idx) const;

   /**
    *   Returns the best name in the combination.
    *   @param comboIdx The combination index.
    *   @param lang     The language requested.
    *   @return The index of the best name.
    */
   inline uint32 getBestIdxInCombo(LangTypes::language_t lang,
                                   uint32 comboIdx) const;

   /**
    *   Returns the best name of the requested type in
    *   the combination.
    *   If the right type of name is not found, MAX_UINT32 will be
    *   returned.
    *   @param lang The requested language.
    *   @param type The requested type.
    *   @param comboIdx The combination index.
    *   @return Best name or MAX_UINT32.
    */
   inline uint32 getBestIdxOfTypeInCombo( LangTypes::language_t lang,
                                          ItemTypes::name_t type,
                                          uint32 comboIdx) const ;

   /**
    *   Returns the best name in the combination.
    *   @param comboIdx The combination index.
    *   @param lang     The language requested.
    *   @return The index of the best name.
    */
   inline pair<const char*, LangTypes::language_t>
      getBestNameInCombo(LangTypes::language_t lang,
                         uint32 idx) const;

   /**
    *   Uses the ItemNames to get the string.
    */   
   inline const char* getName(uint32 stringIndex) const;
   
   
   /**
    *   Returns the number of characters in the table.
    */
   inline uint32 getTotalStringSize() const;   
   
protected:

   /**
    *   Index to offset factor. Increasing it means more strings
    *   but more padding and less re-use.
    *   If e.g. the factor is 2 the strings can only start on
    *   even adresses and must thus be padded.
    */
   enum {
      index_to_offset_factor = 4,
   };
   
   /**
    *   Converts index to offset
    */
   static inline uint32 indexToOffset( uint32 idx ) {
      return idx * index_to_offset_factor;
   }

   /**
    *   Converts offset to index.
    */
   static inline uint32 offsetToIndex( uint32 offset ) {
      return offset / index_to_offset_factor;
   }
   
   /**
    *   Returns true if a raw offset can be indexed.
    */
   static inline bool canBeIndexed( uint32 offset ) {
      return ( offset % index_to_offset_factor ) == 0;
   }

   /**
    *    Pointer to the first string.
    */
   char* m_strings;

   /**
    *    Size of all strings.
    */
   uint32 m_nbrCharsUsed;

   /**
    *   Collection of string combinations for the
    *   SearchMapItems. Rename that class?
    */
   SearchMapRegionTable* m_stringCombos;
   
};

// -- The writeable version

class WriteableSearchMapStringTable : public SearchMapStringTable {

public:
   /**
    *   Creates a new, empty WriteableSearchMapStringTable.
    */
   WriteableSearchMapStringTable();

   /**
    *   Prints the total pad
    */
   ~WriteableSearchMapStringTable() {
      mc2dbg << "[WSMST]: Total pad is " << m_totalPad << endl;
   }
  
   /**
    *   Adds the strings to the ItemNames, and the stringCombos.
    *   The strings are looked up in the map and then added
    *   to the internal strings if not already there. The index in the
    *   map is then converted into an index in the internal
    *   structure here. The nametype and language type is kept.
    *   Should only be used when assigning indeces to the SearchMapItems.
    *   @param rawStrings Raw string idx in first (for type and lang)
    *                     String index in second.
    *   @param theMap     Maps index from the index in rawStrings to our
    *                     index if not null.
    *   @return Index to the combination of strings.
    */
   template<class ITERATABLE>
   uint32 addStringCombo(const ITERATABLE& rawStrings,
                         const map<uint32, uint32>* theMap = NULL);


   /**
    *   Adds the name to the ItemNames.
    *   @param name String to add.
    *   @return Index to the string (not combo).
    */
   inline uint32 addItemName(const char* name);
   
protected:
      
   /**
    *   WriteableString table for the string combinations.
    */
   WriteableSearchMapRegionTable* m_writeableStringCombos;

   /**
    *   Size of all the allocated strings.
    */
   uint32 m_allocatedStringSize;

   /**
    *   Type of map to check for dups.
    */
   typedef map<const char*, uint32, STLStringUtility::ltstr> stringMap_t;
   
   /**
    *   Map used to keep track of the already added strings.
    *   String in first and offset in second.
    */
   stringMap_t m_stringMap;

   /**
    *   Total pad due to the offset to index etc.
    */
   uint32 m_totalPad;
   
};

// -- Inlined funcs

inline int
SearchMapStringTable::getComboNbrNames(uint32 idx) const
{
   return m_stringCombos->getNbrRegions(idx);
}

inline uint32
SearchMapStringTable::getComboNameIdx(int nameNbr, uint32 idx) const
{
   return m_stringCombos->getRegion(nameNbr, idx);
}

const char*
SearchMapStringTable::getName(uint32 idx) const
{
   uint32 offset = indexToOffset( idx );
   MC2_ASSERT( offset < m_nbrCharsUsed );
   return &m_strings[ offset ];
}
   
inline uint32
SearchMapStringTable::getBestIdxInCombo(LangTypes::language_t lang,
                                        uint32 idx) const
{
   const int nbrNames = getComboNbrNames(idx);
   if ( nbrNames == 0 ) {
      return MAX_UINT32;
   }
   SearchMapRegionIterator ptr = m_stringCombos->begin(idx);
   int chosenNbr = NameUtility::getBestName(nbrNames,
                                            ptr,
                                            lang);
   if ( chosenNbr < 0 ) {
      return MAX_UINT32;
   } else {
      return getComboNameIdx(chosenNbr, idx);
   }
}

inline uint32
SearchMapStringTable::getBestIdxOfTypeInCombo( LangTypes::language_t lang,
                                               ItemTypes::name_t type,
                                               uint32 comboIdx ) const
{
   const int nbrNames = getComboNbrNames(comboIdx);
   
   if ( nbrNames == 0 ) {
      return MAX_UINT32;
   }
   SearchMapRegionIterator ptr = m_stringCombos->begin(comboIdx);
   int chosenNbr = NameUtility::getBestNameOfType( nbrNames,
                                                   ptr,
                                                   lang,
                                                   type );
   if ( chosenNbr < 0 ) {
      return MAX_UINT32;
   } else {
      return getComboNameIdx(chosenNbr, comboIdx);
   }
}

inline pair<const char*, LangTypes::language_t>
SearchMapStringTable::getBestNameInCombo(LangTypes::language_t lang,
                                         uint32 idx) const
{
   const uint32 tmpNameOffset = getBestIdxInCombo(lang, idx);
   if ( tmpNameOffset != MAX_UINT32 ) {
      return pair<const char*, LangTypes::language_t>
         (getName(GET_STRING_INDEX(tmpNameOffset)),
          GET_STRING_LANGUAGE(tmpNameOffset));
   } else {
      return pair<const char*, LangTypes::language_t>
         (NULL, LangTypes::invalidLanguage);
   }
}

inline uint32
SearchMapStringTable::getTotalStringSize() const
{
   return m_nbrCharsUsed;
}

inline uint32
WriteableSearchMapStringTable::addItemName(const char* name)
{
   MC2_ASSERT( canBeIndexed( m_nbrCharsUsed ) );

   pair<stringMap_t::iterator, bool> res =
      m_stringMap.insert(make_pair(name, m_nbrCharsUsed) );

   if ( res.second == false ) {
      // Not inserted
   } else {
      // Inserted - now we must keep our promise and add it
      int len = strlen(name) + 1;
      
      // Insert all the substrings in the map too. There may
      // be some room that can be saved that way.
      // The first one is already added.

      //
      // Adds substrings as:
      // abcd
      // bcd
      // cd
      // d
      for( int i = 0; i < len; ++i ) {
         // Skip the ones that cannot be indexed anyway.
         if ( canBeIndexed( m_nbrCharsUsed ) ) {
            m_stringMap.insert(make_pair(&name[i], m_nbrCharsUsed));
         } else {
            mc2dbg8 << "[WSMST]: " << m_nbrCharsUsed << " cannot be indexed"
                    << endl;
         }

         // Add the character to the array of characters
         m_strings = ArrayTool::addElement(m_strings, name[i], m_nbrCharsUsed,
                                         m_allocatedStringSize);
      }
      // Pad if uneven number of characters.
      while ( ! canBeIndexed( m_nbrCharsUsed ) ) {
         ++m_totalPad;
         m_strings = ArrayTool::addElement(m_strings, '\0', m_nbrCharsUsed,
                                         m_allocatedStringSize);
      }
   }

   return offsetToIndex( res.first->second );
   
}

template <class ITERATABLE>
uint32
WriteableSearchMapStringTable::
addStringCombo(const ITERATABLE& rawStrings,
               const map<uint32,uint32>* theMap)
{
   // Start by adding and exchanging the indeces for our indeces.
   set<uint32> newCombo;
   for(typename ITERATABLE::const_iterator it = rawStrings.begin();
       it != rawStrings.end();
       ++it ) {
      LangTypes::language_t strLang = GET_STRING_LANGUAGE(it->first);
      ItemTypes::name_t strType     = GET_STRING_TYPE(it->first);
      uint32 strIdx                 = GET_STRING_INDEX(it->second);
      
      // Exchange the name for our name
      if ( theMap ) {
         strIdx = theMap->find(strIdx)->second;
      }

      // Check so that the strIdx isn't too large.
      if ( strIdx != GET_STRING_INDEX( strIdx ) ) {
         mc2dbg << "[SMST]: String index " << strIdx << " to high"
                << endl;
      }
      MC2_ASSERT( strIdx == GET_STRING_INDEX( strIdx ) );
     
      newCombo.insert( CREATE_NEW_NAME(strLang, strType, strIdx) );      
   }

   // Return the index of the combination.
   return m_writeableStringCombos->addRegionCombo(newCombo);
}



#endif

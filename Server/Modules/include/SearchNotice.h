/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGSEARCHSTRUCTNOTICE_H
#define STRINGSEARCHSTRUCTNOTICE_H

#include "config.h"

class MultiStringSearch;

/**
 *   Class containing information about the 
 *   contents of a MultiStringSearch.
 *   Uses the MultiStringSearch to get some of the values.
 *   @see SearchMapItem.
 */
class MultiSearchNotice {
public:   
   /**
    *   To be used before loading from disk and in 
    *   Utility when doing addElement.
    */
   inline MultiSearchNotice();
   
   /**
    *   Returns the index in the map for the string that
    *   is represented by the current SearchNotice.
    */
   inline uint32 getStringIndex() const;

   /**
    *   Returns the number of items for this notice.
    */
   inline uint32 getNbrItems(const MultiStringSearch* mss) const;

   /**
    *   Returns item number <code>offset</code> for the notice.
    */
   inline const SearchMapItem* getItem(const MultiStringSearch* mss,
                                       uint32 offset) const;

   /**
    *   Returns the namenumber used in the item at the same offset.
    */
   inline uint8 getNameNbr(const MultiStringSearch* mss,
                           uint32 offset) const;

   /**
    *   Returns the word number for the item.
    */
   inline uint8 getWordNbr(const MultiStringSearch* mss,
                           uint32 offset) const;

   /**
    *   Returns the search type mask for the notice at
    *   offset.
    */
   inline uint8 getStringPartMask(const MultiStringSearch* mss,
                                  uint32 offset) const;
   
   
private:
   friend class MultiStringSearch;
   friend class WriteableMultiStringSearch;

   /**
    *   To be used by WriteableMultiStringSearch when
    *   creating new notices from a map.
    */
   inline MultiSearchNotice(uint32 stringIndex, uint32 endOfItems);
   
   /**
    *   String index of the string that this notice represents.
    */
   uint32 m_strIdx;

   /**
    *   End index where the items that contain this string
    *   ends in the MultiSearchNotice.
    */
   uint32 m_endOfItems;
};

#include "SearchStruct.h"

inline MultiSearchNotice::MultiSearchNotice()
{
}

inline
MultiSearchNotice::MultiSearchNotice(uint32 stringIndex, uint32 endOfItems)
      : m_strIdx(stringIndex), m_endOfItems(endOfItems)
{
}

inline uint32
MultiSearchNotice::getStringIndex() const
{
   return m_strIdx;
}

inline uint32
MultiSearchNotice::getNbrItems(const MultiStringSearch* mss) const
{
   return  mss->getEndOffset(this) - mss->getStartOffset(this);
}


inline const SearchMapItem*
MultiSearchNotice::getItem(const MultiStringSearch* mss,
                           uint32 offset) const
{
   MC2_ASSERT( offset < getNbrItems(mss) );
   return mss->getSearchMapItem(mss->getStartOffset(this)+offset);
}

inline uint8
MultiSearchNotice::getNameNbr(const MultiStringSearch* mss,
                              uint32 offset) const
{
   MC2_ASSERT( offset < getNbrItems(mss) );
   return mss->getNameNbr(mss->getStartOffset(this)+offset);
}

inline uint8
MultiSearchNotice::getWordNbr(const MultiStringSearch* mss,
                              uint32 offset) const
{
   MC2_ASSERT( offset < getNbrItems(mss) );
   return mss->getWordNbr(this, offset);
}

inline uint8
MultiSearchNotice::getStringPartMask(const MultiStringSearch* mss,
                                     uint32 offset) const
{
   MC2_ASSERT( offset < getNbrItems(mss) );
   return mss->getStringPartMask(this, offset);
}

#endif

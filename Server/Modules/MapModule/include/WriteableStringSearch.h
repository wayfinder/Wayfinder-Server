/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WRITEABLE_STRINGSEARCHSTRUCT_H
#define WRITEABLE_STRINGSEARCHSTRUCT_H

#include "config.h"

#include <vector>

#include "SearchStruct.h"


class WriteableMultiStringSearch : public MultiStringSearch {
public:
   /**
    *   Build the internal structures from the searchMap and
    *   WriteableSearchMapStringTable. Should only be used
    *   by SearchUnit when building the struct.
    */
   WriteableMultiStringSearch(const SearchMap2* searchMap,
                              WriteableSearchMapStringTable* stringTable);

   /**
    *   Destructor
    */
   virtual ~WriteableMultiStringSearch();

private:
   /**
    *   Adds a string for use in BeginningOfWord and BeginningOfString-
    *   searching.
    */
   inline void addString(const SearchMapItem* item,
                         const SearchMap2* theMap,
                         int nameNbr,
                         MSSTempDataMap& tempStrings);

   /**
    *   Adds a temporary data structure to the internal
    *   storage. Checks if there already is one and or:s
    *   the stringparts together if there is.
    *   The string is owned by the WriteableMultiStringSearch
    *   after this so it must be a copy.
    */
   inline void addMSSTempData(MSSTempDataMap& dataMap,
                              char* str,
                              const MSSTempData& data);

   /**
    *   Adds the tmpData to the real storage.
    */
   inline void addPermanent(uint32 stringIdx,
                            const MSSTempData& tmpData);

   /**
    *   The number of allocated stringIdx,
    */
   int m_nbrAllocatedStrIdx;

   /**
    *   The number of allocated items in m_info*
    */
   int m_infoArrayAllocSize;

   /**
    *   Allocated strings to be deleted after everything
    *   is done.
    */
   vector<char*> m_stringGarbage;
   
   
};

#endif

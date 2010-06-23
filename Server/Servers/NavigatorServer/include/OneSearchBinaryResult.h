/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ONESEARCHBINARYRESULT_H
#define ONESEARCHBINARYRESULT_H

#include "config.h"
#include "DataBuffer.h"
#include "CategoryTree.h"
#include <memory>
#include "SearchParserHandler.h"

class VanillaMatch;

typedef vector<VanillaMatch*> ResultVector;

namespace OneSearchUtils {

/**
 * Represents a category tree in our unified binary format.
 */
struct BinarySearchResult {
   auto_ptr<DataBuffer> m_stringTable;
   auto_ptr<DataBuffer> m_areaTable;
   auto_ptr<DataBuffer> m_infoItemTable;
   auto_ptr<DataBuffer> m_matchesTable;
};

/**
 * Serializes the search results to the binary format.
 * 
 * @param matchesBegin  Iterator to first match to serialize
 * @param matchesEnd Iterator to position after last match to serialize
 * @param searchHandler SearchParserHandler, used to get icon names
 * @param format   The output data.
 */
void serializeResults( ResultVector::const_iterator matchesBegin,
                       ResultVector::const_iterator matchesEnd,
                       SearchParserHandler& searchHandler,
                       BinarySearchResult* format);

}

#endif

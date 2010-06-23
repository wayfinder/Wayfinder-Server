/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BINARYCATEGORYTREEFORMAT_H
#define BINARYCATEGORYTREEFORMAT_H

#include "config.h"
#include "DataBuffer.h"
#include "CategoryTree.h"
#include <memory>

namespace CategoryTreeUtils {

/**
 * Represents a category tree in our unified binary format.
 */
struct BinaryCategoryTreeFormat {
   auto_ptr<DataBuffer> m_categoryTable;
   auto_ptr<DataBuffer> m_lookupTable;
   auto_ptr<DataBuffer> m_stringTable;
};

/**
 * Serializes a category tree to our unified binary format.
 * 
 * @param tree     The category tree, must contain at least one category.
 *                 The binary format is unspecified for an empty tree, we
 *                 should consider that to be an error and send an error code 
 *                 instead.
 * @param language The language to serialize for.
 * @param format   The output data.
 */
void serializeTree( const CategoryTree* tree,
                    LangTypes::language_t language,
                    BinaryCategoryTreeFormat* format);

/**
 * Calculate CRC for the binary category tree.
 *
 * @param catTreeFormat  The catebory tree to calculate CRC for
 * @return               String containing the CRC.
 */
MC2String getCrcForTree( const BinaryCategoryTreeFormat& catTreeFormat );

/**
 * NOTE! This function is implemented for use in unit test and should be
 * reviewed before use from production code.
 *
 * Parses the binary format into a vector of categories.
 */
void parseBinaryFormat( BinaryCategoryTreeFormat* binaryFormat,
                        vector<CategoryTree::StandaloneNode>& categories );

}

#endif // BINARYCATEGORYTREEFORMAT_H

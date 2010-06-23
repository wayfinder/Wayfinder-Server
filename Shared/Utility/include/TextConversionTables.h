/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 *  NOTE! This file is machine generated - do not edit directly!!!
 *  cmdline: ./Shared/src/genTextConvTables.py
 *  http://www.unicode.org/Public/UNIDATA/UnicodeData.txt
 *  http://www.unicode.org/Public/UNIDATA/CaseFolding.txt
 *  Shared/include/TextConversionTables.h
 *  Shared/include/TextConversionTables.h Shared/src/TextConversionTables.cpp
 */

#ifndef TEXT_CONVERSUIONT_AATBLES_H
#define TEXT_CONVERSUIONT_AATBLES_H

#include "config.h"
#include <algorithm>

class TextConversionTables {

public:
   /**
    *   One entry in a conv table.
    */
   struct convTable_entry {
      /// The unicode character to be converted
      const uint32 unicodeChar;
      /// The utf-8 string of the converted character
      const char* const utf8String;
      /// Length of the utf-8 string.
      const int utf8Length;
   };

   /**
    *    A table of convTable_entry. Also contains the number
    *    of entries in the table.
    */
   struct convTable {
      uint32 nbrEntries;
      const convTable_entry* const entries;
   };

   class ConvEntryComp {
     public:
      bool operator()(const convTable_entry& a, const convTable_entry& b) {
         return a.unicodeChar < b.unicodeChar;
      }
      bool operator()(const convTable_entry& a, uint32 code ) {
         return a.unicodeChar < code;
      }
   };

   /// Result of a findInTable operation.
   struct convTableFindRes {
      convTableFindRes( const char* aStr, int aLen ) : resString(aStr),
                                                       resStringLength(aLen) {}
      const char* resString;
      int resStringLength;
   };

   /**
    *   Looks for the code <code>code</code> in the table <code>table</code>.
    *   @param table Table to search.
    *   @param code  Code to look for.
    *   @return Result with string set to NULL and length set to zero
    *           if not found.
    */
   static convTableFindRes findInTable( const convTable& table,
                                        uint32 code ) {
      const convTable_entry* begin = table.entries;
      const convTable_entry* end   = table.entries + table.nbrEntries;
      const convTable_entry* found = std::lower_bound( begin, end,
                                                       code,
                                                       ConvEntryComp() );
      if ( found->unicodeChar == code ) {
         return convTableFindRes(found->utf8String,
                                 found->utf8Length );
      } else {
         return convTableFindRes(NULL, 0);
      }
   }
   

   /*
    *  Table to convert from upper to lower case. If a character does not
    *  exist in the table, it should be kept as is
    */
   static convTable c_lowerTable;
   /*
    *  Table to convert from lower to uppercase. If a character does not
    *  exist in the table, it should be kept as is.
    */
   static convTable c_upperTable;
   /*
    *  Table remove strange characters and convert accented characters to
    *  non-accented ones. For use in the SM. If a character does not exist
    *  in the table, it should be removed. If a space is returned, the
    *  character corresponds to a space.
    */
   static convTable c_removeStrange;

private:

   static convTable_entry c_lowerTableEntries [825];

   static convTable_entry c_upperTableEntries [834];

   static convTable_entry c_removeStrangeEntries [30254];


};
#endif

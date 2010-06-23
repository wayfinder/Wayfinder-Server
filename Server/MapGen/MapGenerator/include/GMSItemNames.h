/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSITEMNAMES_H
#define GMSITEMNAMES_H

#include "OldItemNames.h"
#include "StringUtility.h"
#include "MC2String.h"


/**
  *   This class is used by the generate map server because it is not
  *   possible to search for strings in the standard itemNames class.
  *
  */
class GMSItemNames : public OldItemNames
{
   public:

      /**
        *   Default constructor.
        */
      GMSItemNames();

      /**
        *   Default destructor.
        */
      virtual ~GMSItemNames();

      /**
        *   Sorts the namevector. This function must be called prior to
        *   calling the findName function.
        */
      void sort();

      /**
        *   Find a name stored in the sortedStringTable.
        *   @param   name  The name to search for
        *   @return  The index of the name
        */
      uint32 findName(char* name);

      /**
        *   @return  The number of sorted strings.
        */
      uint32 getNbrSortedStrings() {
         return sortedStringTable.size();
      }

      /**
       *    Load the OldItemNames from file.
       *    @param   infile   The file to load from.
       *    @return  True upon success, false otherwise.
       */
      bool load(ifstream& infile);

      /**
       *    Exchange a string in the stringtable with a new one.
       *    NB! Make sure that you know what you are doing when you call
       *    this method!! Make sure that all items that has this name
       *    would like it to be exchanged with the new one.
       *    @param   strIdx   The stringindex to be exchanged with a new
       *                      string.
       *    @param   newName  The new name.
       *    @return  True if the exchanging was succesful, otherwise
       *             false (could happen if the name was in the blocked
       *             strings).
       */
      bool exchangeString(uint32 strIdx, const char* newName);
      
   protected:

      /**
       *    Internal load of the OldItemNames from a databuffer.
       *    Fragments the strings.
       *    @param   buffer   The databuffer to load from.
       *    @return  True upon success, false otherwise.
       */
      bool internalLoad(DataBuffer& buffer, bool utf8InMaps = false);

      /**
       *    Fragments the strings, ie. moves the strings from the blocked
       *    area to be seperate strings instead. Is called upon loading.
       *    @return  True upon success, false otherwise.
       */
      bool fragStrings();
      
      /**
        *   This table contains a sorted version of the strings.
        */
      vector< char* > sortedStringTable;

      /**
        *   Pivotelement used by quickSort
        */
      char* pivot;

      /**
        *   Sorts the sortedStringTable. Called by sort().
        */
      void quickSort(uint32 start, uint32 stop);
};

#endif

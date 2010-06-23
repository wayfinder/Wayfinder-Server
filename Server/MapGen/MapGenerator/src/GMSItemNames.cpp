/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSItemNames.h"

GMSItemNames::GMSItemNames() : OldItemNames()
{
   pivot = new char[512];
}

GMSItemNames::~GMSItemNames()
{
   delete [] pivot;
}

void GMSItemNames::sort()
{
   sortedStringTable.clear();
   sortedStringTable.reserve( getNbrStrings() );
   for (uint32 i=0; i<getNbrStrings(); i++) {
      sortedStringTable.push_back( stringTable.getElementAt(i) );
   }
   quickSort(0, getNbrStrings()-1);
}

uint32 GMSItemNames::findName(char* key)
{
   // XXX: The binary serach (binarysearch.scr) should be used.

   uint32 left = 0;
   uint32 right = getNbrSortedStrings()-1;
   uint32 mid = (left + right) / 2;

   while (left < right) {
      MC2String upper =
         StringUtility::copyUpper(MC2String( sortedStringTable[ mid ]));
      if (strcmp(upper.c_str(), key) > 0) {
         right = mid-1;
      } else if (strcmp(upper.c_str(), key) < 0) {
         left = mid+1;
      } else {  // key found
         left = right = mid;
      }
      mid = (left + right) / 2;
   }
   MC2String upper =
      StringUtility::copyUpper(MC2String(sortedStringTable[ left ]));
   if ( (left == right) && 
        (strcmp(upper.c_str(), key) == 0) ) {
      return (left);
   } else {
      return (MAX_UINT32);
   }
}


void GMSItemNames::quickSort(uint32 start, uint32 stop)
{
   if (start < stop) {
      char* pivot = sortedStringTable[ start ];
      uint32 i = start - 1;
      uint32 j = stop + 1;
      bool done = false;

      while (!done) {
         while (strcmp(sortedStringTable[ --j ], pivot) > 0) {
         }
         while (strcmp( sortedStringTable[ ++i ], pivot) < 0) {
         }
         if (i < j) {
            swap( sortedStringTable[ i ], sortedStringTable[ j ] );
         }
         else
            done = true;
      }
      quickSort(start, j);
      quickSort(j + 1, stop);
   }
}


bool 
GMSItemNames::load(ifstream& infile)
{
   bool retVal = OldItemNames::load(infile);
   if (retVal) {
      // Split the block with strings into seperate strings
      return (fragStrings());
   } else {
      return (false);
   }
}
      
bool 
GMSItemNames::internalLoad(DataBuffer& buffer, bool utf8InMaps)
{
   bool retVal = OldItemNames::internalLoad(buffer, utf8InMaps);
   if (retVal) {
      // Split the block with strings into seperate strings
      return (fragStrings());
   } else {
      return (false);
   }
}


bool
GMSItemNames::fragStrings()
{

   // Copy the strings from the blocked area to seperate strings
   for (uint32 i = 0; i < nbrBlockedStrings; i++) {
      char* str = (char*) stringTable.getElementAt(i);
      char* newstr = NULL; 
      if (str != NULL) {
         newstr = StringUtility::newStrDup(str);
      }
      stringTable.setElementAt(i, newstr);
   }
   
   // Delete the blocked area and update nbrBlockedStrings.
   delete[] theStrings;
   theStrings = NULL;
   
   nbrBlockedStrings = 0;
   sizeOfBlock = 0;

   return (true);   
   
}


bool 
GMSItemNames::exchangeString(uint32 strIdx, const char* newName)
{
   // Note, in order to do this exchange, the string is not allowed
   // to be in the blocked string area.
   if ( (strIdx > nbrBlockedStrings) && 
        (strIdx < getNbrStrings()) &&
        (newName != NULL) ) {
      delete stringTable.getElementAt(strIdx);
      stringTable.setElementAt(strIdx, StringUtility::newStrDup(newName));
      
      return (true);
   } 
   else if ( strIdx < getNbrStrings() ){
      // These strings are allocated in the string block. Don't delete.
      stringTable.setElementAt(strIdx, StringUtility::newStrDup(newName)); 
      return (true);
   }
   else {
      return (false);
   }
}














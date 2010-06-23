/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OldItemNames.h"
#include "StringUtility.h"
#include "NewStrDup.h"
#include "MC2SimpleString.h"
#include "UTF8Util.h"
#include "MapGenUtil.h"
#include "STLStringUtility.h"
#include "Utility.h"

OldItemNames::OldItemNames() : stringTable(0, 128)
{
   mc2dbg4 << "OldItemNames::OldItemNames()" << endl;
   init();
}

OldItemNames::OldItemNames(const char* firstName) : stringTable(0, 128)
{
   mc2dbg4 << "OldItemNames::OldItemNames()" << endl;
   init();
   addString(firstName);
}

OldItemNames::OldItemNames(ifstream& infile) : stringTable(0, 128)
{
   mc2dbg4 << "OldItemNames::OldItemNames(file)" << endl;
   init();
   load(infile);

   DEBUG8(printAllStrings());
}

OldItemNames::OldItemNames(DataBuffer& buffer) : stringTable(0, 128)
{
   mc2dbg4 << "OldItemNames::OldItemNames(file)" << endl;
   init();
   internalLoad(buffer);

   DEBUG8(printAllStrings());
}

void
OldItemNames::init() 
{
   theStrings = NULL;
   nbrBlockedStrings = 0;
   sizeOfBlock = 0;
   sizeOfStrings = 0;
   m_strIndex = NULL;
}

OldItemNames::~OldItemNames()
{
   // Delete the memoryarea that contains the first nbrBlockedStrings
   delete [] theStrings;
   
   // Delete all the strings that was allocated one by one
   for (uint32 i = nbrBlockedStrings; i<getNbrStrings(); i++) {
      delete [] stringTable.getElementAt(i);
   }
   delete m_strIndex;
}

bool OldItemNames::load(ifstream& infile)
{
   DataBuffer *dataBuffer = new DataBuffer(8);

   // Read the size of the stringtable
   mc2dbg8 << "   loadMap: read 8 bytes" << endl;
   if (! Utility::read(infile, dataBuffer->getBufferAddress(), 8) ) {
      DEBUG1(mc2log << error
             << "   OldItemNames::load() Error when reading sizeOfStrings "
             << strerror(errno) << endl;)
      delete dataBuffer;
      return (false);
   }
   sizeOfStrings = sizeOfBlock = dataBuffer->readNextLong();
   nbrBlockedStrings = dataBuffer->readNextLong();
   delete dataBuffer;

   mc2dbg4 << "   loadMap: sizeOfStrings = " << sizeOfStrings << endl;
   mc2dbg4 << "   loadMap: nbrStrings = " << nbrBlockedStrings << endl;

   if ((sizeOfBlock == 0) || (nbrBlockedStrings == 0)) {
      mc2dbg2 << "   No strings, returning!" << endl;
      theStrings = NULL;
      return (true);
   }

   mc2dbg4 << "   loadMap: reads theStrings" << endl;
   theStrings = new char[sizeOfStrings];
   if (!Utility::read(infile, theStrings, sizeOfBlock) ) {
      DEBUG1(mc2log << error
             << "   OldItemNames::load() Error when reading strings "
             << strerror(errno) << endl;)
      return (false);
   }
   // Read past the possible padds after the strings
   /*int padSize = (4 - (sizeOfBlock % 4)) % 4;
   if ( padSize  > 0) {
      mc2dbg4 << "PAD: " << 4 - (sizeOfBlock % 4) << endl;
      dataBuffer = new DataBuffer(4);
      if (!Utility::read(infile, dataBuffer->getBufferAddress(), padSize)) {
         DEBUG1(mc2log << error
         << "   OldItemNames::load() Error when reading padbytes "
         << strerror(errno) << endl;)
         delete dataBuffer;
         return (false);
      }
      delete dataBuffer;
      mc2dbg2 << here << padSize << " paddbytes read after stringtable" 
              << endl;
   }*/

   // Read past the possible padds after the strings
   if ( (sizeOfBlock % 4) > 0) {
      mc2dbg4 << "PAD: " << 4 - (sizeOfBlock % 4) << endl;
      dataBuffer = new DataBuffer(4);
      if (!Utility::read(  infile,
                           dataBuffer->getBufferAddress(),
                           4 - (sizeOfBlock % 4) ) ) {
         DEBUG1(mc2log << error
                << "   OldItemNames::load() Error when reading padbytes "
                << strerror(errno) << endl;)
         delete dataBuffer;
         return (false);
      }
      for(uint i = 0; i < 4 - (sizeOfBlock % 4); i++) {
         mc2dbg2 << (int)dataBuffer->getBufferAddress()[i] 
                 << " " << flush;
      }
      mc2dbg8 << "Aligned" << endl;
      delete dataBuffer;
      mc2dbg2 << "   OldMap::loadMap(): " << 4 - (sizeOfBlock % 4)
              << " Paddbytes read after stringtable" << endl;
   }
   

   if ((sizeOfBlock > 0) && (nbrBlockedStrings > 0)) {
      // Create the stringtable with from memoryarea theStrings
      stringTable.setAllocSize(nbrBlockedStrings);
      stringTable.addLast(&theStrings[0]);
      uint32 pos=1;
      uint32 limit = sizeOfBlock-1;
      while (pos < limit) {
         if (theStrings[pos] == '\0')
            stringTable.addLast(&theStrings[pos+1]);
         pos++;
      }
      DEBUG_DB(printAllStrings());
   } else {
      mc2dbg1 << "   loadMap(): stringLength <= 0" << endl;
   }

   return (true);
}

bool
OldItemNames::internalLoad(DataBuffer& buffer,
                        bool utf8InMaps)
{
   sizeOfStrings = sizeOfBlock = buffer.readNextLong();
   nbrBlockedStrings = buffer.readNextLong();

   DEBUG_DB(mc2dbg << "   loadMap: sizeOfStrings = " << sizeOfStrings << endl);
   DEBUG_DB(mc2dbg << "   loadMap: nbrStrings = " << nbrBlockedStrings
            << endl);
   DEBUG_DB(mc2dbg << "   loadMap: reads theStrings" << endl);
   theStrings = new char[sizeOfStrings];
   if (buffer.readNextByteArray((byte*)theStrings, 
                                sizeOfStrings) != int(sizeOfStrings)) {
      mc2log << fatal << "OldItemNames::internalLoad(DataBuffer&) "
             << "Failed to read all strings, sizeOfStrings=" 
             << sizeOfStrings << endl;
   }

   // Read past the possible padds after the strings
   int padSize = (4 - (sizeOfBlock % 4)) % 4;
   for(int i = 0; i < padSize; i++) {
      buffer.readNextByte();
      mc2dbg4 << here << "Reading pad-byte" << endl;
   }
   
   if ((sizeOfBlock > 0) && (nbrBlockedStrings > 0)) {
      // Create the stringtable with from memoryarea theStrings
      stringTable.setAllocSize(nbrBlockedStrings);
      stringTable.addLast(&theStrings[0]);
      uint32 pos=1;
      uint32 limit = sizeOfBlock-1;
      while (pos < limit) {
         if (theStrings[pos] == '\0')
            stringTable.addLast(&theStrings[pos+1]);
         pos++;
      }
      DEBUG_DB(printAllStrings());
   } else {
      DEBUG1(mc2dbg << "   loadMap(): stringLength <= 0" << endl);
   }

   // Code for changing a string into some greek characters
//          if ( strstr( stringTable.getElementAt(i), "poleg" ) ) {
//             currString = "";
//             currString += UTF8Util::ucsToUtf8(0x0391);
//             currString += UTF8Util::ucsToUtf8(0x0392);
//             currString += UTF8Util::ucsToUtf8(0x0393);
//             currString += UTF8Util::ucsToUtf8(0x0394);
//             currString += UTF8Util::ucsToUtf8(0x0395);
//             currString += UTF8Util::ucsToUtf8(0x0396);
//             currString += UTF8Util::ucsToUtf8(0x0397);
//             currString += UTF8Util::ucsToUtf8(0x0398);
//             currString += UTF8Util::ucsToUtf8(0x0399);
//             currString += UTF8Util::ucsToUtf8(0x039a);
//             testString = currString;
//             currString = "KAX";
//             testString = StringUtility::copyLower( testString );
//          }

   
   // Convert to UTF8 or ISO-8859-1 if needed
#ifdef MC2_UTF8
   // Check if the table needs conversion
   if ( ! utf8InMaps ) {
      mc2log << info
             << "[OldItemNames]: Converting strings from ISO-8859-1 to UTF-8"
             << endl;
      // Must frag strings to avoid memory leaks
      fragStrings();
      for(uint32 i = 0; i < stringTable.getSize(); i++) {
         const char* currString = stringTable.getElementAt(i);
         MC2String converted = UTF8Util::isoToMc2(currString);
         if ( converted != currString ) {
            delete [] stringTable[i];
            stringTable[i] = NewStrDup::newStrDup( converted.c_str() );
         }
      }
   } else {
      mc2log << info << "[OldItemNames]: Maps and mc2 use UTF-8 - no conversion"
             << endl;
   }
#else
   // Check if the table needs conversion.
   if ( utf8InMaps ) {
      mc2log << info 
             << "[OldItemNames]: Converting strings from UTF-8 to ISO-8859-1"
             << endl;
      // Must frag strings to avoid memory leaks
      fragStrings();
      for(uint32 i = 0; i < stringTable.getSize(); i++) {
         const char* currString = stringTable.getElementAt(i);
         MC2String converted = UTF8Util::utf8ToMc2(currString);
         if ( converted != currString ) {
            delete [] stringTable[i];
            stringTable[i] = NewStrDup::newStrDup( converted.c_str() );
         }
      }
   } else {
      mc2log << info 
             << "[OldItemNames]: Maps and mc2 use ISO-8859-1 - no conversion"
             << endl;
   }
#endif
   
   return true;
}

bool OldItemNames::save(int outfile)
{
   // The total number of string bytes written so far,
   // This is calculated in a naive way...
   uint32 totalBytesToWrite = sizeOfBlock;
   char* curStr;
   uint32 tmpLength;
   for (uint32 i = nbrBlockedStrings; i<getNbrStrings(); i++) {
      curStr = (char*) stringTable.getElementAt(i);
      tmpLength = strlen(curStr) + 1;
      totalBytesToWrite += tmpLength;
   }

   // Write totalSize and number of strings for the stringtable
   DataBuffer* dataBuffer = new DataBuffer(8);
   dataBuffer->writeNextLong(totalBytesToWrite);
   dataBuffer->writeNextLong(getNbrStrings());
   write(outfile, dataBuffer->getBufferAddress(), 8);
   delete dataBuffer;
   DEBUG_DB(mc2dbg << dec << "   sizeOfStrings: " << sizeOfStrings
                 << ", nbrStrings: " << getNbrStrings() << endl;)

   // Write all the strings that was loaded into the memoryblock
   write(outfile, theStrings, sizeOfBlock);

   // Write the string that have been added, one by one.
   for (uint32 i = nbrBlockedStrings; i<getNbrStrings(); i++) {
      curStr = (char*) stringTable.getElementAt(i);
      tmpLength = strlen(curStr) + 1;
      write(outfile, curStr, tmpLength);
   }

   // Write pads?
   int padSize = (4 - (totalBytesToWrite % 4)) % 4;
   if (padSize > 0) {
      dataBuffer = new DataBuffer(4);
      dataBuffer->writeNextLong(0);
      dataBuffer->reset();
      write(outfile, dataBuffer->getBufferAddress(), padSize);
      mc2dbg4 << here << " Writing " << padSize << " pad bytes" << endl;
      delete dataBuffer;
   }

   DEBUG_DB(mc2dbg << "   Strings written ok (?)" << endl;)
   return true;
}

uint32 OldItemNames::addString(const char* str, bool changeCase)
{
   if ( m_strIndex == NULL ){
      // This has the effect that quickAddString always will be used.
      createIndex();
   }
   if ( m_strIndex != NULL){

      // We have created an index. Faster to use quickAddString.
      return quickAddString( str, changeCase );
   }

   // If to change case, then do it first!
   char* newString = NULL;
   if (changeCase) {
      newString = 
         NewStrDup::newStrDup(MapGenUtil::handleItemNameCase(str));
   }

   // Check if the string already is present
   uint32 id;
   if (newString != NULL)
      id = stringExist(newString);
   else
      id = stringExist(str);

   if (id < MAX_UINT32) {
      // The string was already in the stringTable, return the ID of 
      // that string
      delete [] newString;
      return (id);
   } else {
      // The string did not excist, add it and return the new ID.
      if (newString == NULL) {
         uint32 length = strlen(str)+1;
         newString = new char[length];
         strcpy(newString, str);
      }
      if (stringTable.addLast(newString) != MAX_UINT32) {
         sizeOfStrings += strlen(newString) + 1;
         return (stringTable.getSize()-1);
      }
      else
         return (MAX_UINT32);
   }
}


uint32 OldItemNames::quickAddString(const char* str, bool changeCase)
{
   MC2_ASSERT( m_strIndex != NULL );
   MC2_ASSERT( m_strIndex->size() == stringTable.size() );
   // If to change case, then do it first!
   char* newString = NULL;
   const char* searchString = str;
   if (changeCase) {
      // "true" to make roman numerals stay in upper-case
      newString = 
         NewStrDup::newStrDup( MapGenUtil::handleItemNameCase(str) );
      searchString = newString;
   }


   
   // Check if the string already is present
   map<const char*,uint32,STLStringUtility::ltstr>::const_iterator it =
      m_strIndex->find( searchString );
   
   uint32 id = MAX_UINT32;
   if ( it != m_strIndex->end() ) {
      id = it->second;
   }

   if (id < MAX_UINT32) {
      // The string was already in the stringTable, return the ID of 
      // that string
      delete [] newString;
      return (id);
   } else {
      // The string did not excist, add it and return the new ID.
      if (newString == NULL) {
         uint32 length = strlen(str)+1;
         newString = new char[length];
         strcpy(newString, str);
      }
      if (stringTable.addLast(newString) != MAX_UINT32) {
         sizeOfStrings += strlen(newString) + 1;
         m_strIndex->insert(make_pair(newString, stringTable.getSize()-1));
         return (stringTable.getSize()-1);
      }
      else
         return (MAX_UINT32);
   }
}

void
OldItemNames::createIndex()
{
   mc2dbg << "OldItemNames::createIndex called!" << endl;

   delete [] m_strIndex;
   m_strIndex = new map<const char*,uint32,STLStringUtility::ltstr>;
   for( uint32 i = 0; i < stringTable.size(); ++i ) {
      m_strIndex->insert(make_pair(stringTable[i], i));
   }
}


uint32
OldItemNames::stringExist(const char* str)
{
   uint32 i = 0;
   uint32 nbrStrings = getNbrStrings();
   while (i < nbrStrings) {
      if (strcmp(str, (char*) stringTable.getElementAt(i)) == 0)
         return (i);
      i++;
   }
   return (MAX_UINT32);
}

void
OldItemNames::printAllStrings()
{
   DEBUG1(
   char tmpStr[16];
   mc2dbg << "OldItemNames::printAllStrings(), to print " << getNbrStrings()
        << " strings" << endl;
   for (uint32 i=0; i<getNbrStrings(); i++) {
      sprintf(tmpStr, "%06i%c", i, '\0');
      mc2dbg << tmpStr << " " << ((char*) stringTable.getElementAt(i)) 
           << endl;
   })
}


uint32 
OldItemNames::getMemoryUsage() const 
{
   return sizeof(OldItemNames) + sizeOfStrings - sizeof(StringVector) +
          stringTable.getMemoryUsage();
}


char**
OldItemNames::getCopyOfStringTable() const {
   // Create the array to retrun
   char** retArray = new char*[stringTable.getSize()];
   for (uint32 i=0; i<stringTable.getSize(); i++) {
      retArray[i] = NewStrDup::newStrDup(
                    (char*) stringTable.getElementAt(i) );
   }
   return (retArray);
}

bool
OldItemNames::isFragmented()
{
   return (nbrBlockedStrings == 0);
}

bool
OldItemNames::fragStrings()
{

   // Copy the strings from the blocked area to seperate strings
   for (uint32 i = 0; i < nbrBlockedStrings; i++) {
      char* str = (char*) stringTable.getElementAt(i);
      char* newstr = NULL; 
      if (str != NULL) {
         newstr = NewStrDup::newStrDup(str);
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
OldItemNames::exchangeString(uint32 strIdx, const char* newName)
{
   // Note, in order to do this exchange, the string is not allowed
   // to be in the blocked string area.
   if ( (strIdx > nbrBlockedStrings) && 
        (strIdx < getNbrStrings()) &&
        (newName != NULL) ) {
      delete [] stringTable.getElementAt(strIdx);
      stringTable.setElementAt(strIdx, NewStrDup::newStrDup(newName));
      
      return (true);
   } else {
      return (false);
   }
}


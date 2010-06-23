/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "SearchMapStringTable.h"

#include "SearchMapRegionTable.h"
#include "ItemNames.h"
#include "ItemTypes.h"
#include "DataBuffer.h"
#include "AlignUtility.h"

SearchMapStringTable::SearchMapStringTable()
{
   m_strings        = NULL;
   m_stringCombos   = NULL;
   m_nbrCharsUsed   = 0;
}

SearchMapStringTable::~SearchMapStringTable()
{
   delete [] m_strings;
   delete m_stringCombos;
}

int
SearchMapStringTable::getSizeInDataBuffer() const
{
   return 4 + 4 + AlignUtility::alignToLong(m_nbrCharsUsed) +
          m_stringCombos->getSizeInDataBuffer();
}

int
SearchMapStringTable::save(DataBuffer& dataBuffer) const
{
   // Save the offset of the dataBuffer.
   DataBufferChecker dbc(dataBuffer, "SearchMapStringTable::save");
   dbc.assertRoom(getSizeInDataBuffer());
   
   // Version - can be good to have
   dataBuffer.writeNextLong(0);
   
   // Size of char-block
   dataBuffer.writeNextLong(m_nbrCharsUsed);
   
   // All the characters
   dataBuffer.writeNextByteArray((const byte*)m_strings, m_nbrCharsUsed);

   dataBuffer.alignToLongAndClear();
   
   // The stringCombos can save themselves.
   m_stringCombos->save(dataBuffer);

   // Check that we have written the number of bytes promised.
   dbc.assertPosition(getSizeInDataBuffer());
   
   return getSizeInDataBuffer();
}

int
SearchMapStringTable::load(DataBuffer& dataBuffer)
{
   // Remove old stuff.
   delete [] m_strings;
   delete m_stringCombos;

   // Save the offset of the dataBuffer.
   DataBufferChecker dbc(dataBuffer, "SearchMapStringTable::load");
   
   // Load new stuff
   // Version
   dataBuffer.readNextLong();
   // Size of m_strings
   m_nbrCharsUsed = dataBuffer.readNextLong();
   // Alloc and read the strings
   m_strings = new char[m_nbrCharsUsed];
   dataBuffer.readNextByteArray((byte*)m_strings, m_nbrCharsUsed);
   dataBuffer.alignToLong();
   
   // Create the combination table
   m_stringCombos = new SearchMapRegionTable;
   // And load it.
   m_stringCombos->load(dataBuffer);

   // Check that we have written the number of bytes promised.
   dbc.assertPosition(getSizeInDataBuffer());

   return getSizeInDataBuffer();
}

// -- WriteableSearchMapStringTable

WriteableSearchMapStringTable::WriteableSearchMapStringTable()
      : SearchMapStringTable()
{
   m_totalPad = 0;
   m_writeableStringCombos = new WriteableSearchMapRegionTable();
   // We must be able to save it
   m_stringCombos = m_writeableStringCombos;

   // Setup the strings.
   m_allocatedStringSize = 900000;
   m_nbrCharsUsed        = 0;
   m_strings             = new char[m_allocatedStringSize];
}



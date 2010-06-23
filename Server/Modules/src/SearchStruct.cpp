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

#include "MC2String.h"
#include "SearchMap2.h"
#include "SearchMapItem.h"
#include "SearchStruct.h"
#include "SearchNotice.h"

#include "StringSearchUtility.h"

#include "DataBuffer.h"

#include "SearchComparators.h"
#include "AlignUtility.h"

#include <map>
#include <algorithm>


int
MultiStringSearch::getSizeInDataBuffer() const
{
   int strIdxSize = 4 + m_nbrStrIdx * 8;
   int infoSize   = 4 + m_infoArraySize * 4; // Size + m_infoVectorIdx
   infoSize += AlignUtility::alignToLong(m_infoArraySize*2); // masks+namenbrs.
   return 4 + strIdxSize + infoSize;
}

MultiStringSearch::MultiStringSearch(const SearchMap2* searchMap)
{ 
   m_stringIdx                 = NULL;
   m_nbrStrIdx                 = 0;
   m_infoVectorIdx             = NULL;
   m_infoVectorNameNbr         = NULL;
   m_infoVectorStringPartMasks = NULL;
   m_infoArraySize              = 0;
   m_map                        = searchMap;
}

MultiStringSearch::~MultiStringSearch()
{
   delete [] m_stringIdx;
   delete [] m_infoVectorIdx;
   delete [] m_infoVectorNameNbr;
   delete [] m_infoVectorStringPartMasks;
}


int
MultiStringSearch::save(DataBuffer& buf) const
{
   DataBufferChecker dbc(buf, "MultiStringSearch::save");

   dbc.assertRoom(getSizeInDataBuffer());

   // Version
   buf.writeNextLong(0); 
   
   // Number of stringIdx
   buf.writeNextLong(m_nbrStrIdx);
   // The idx
   for( int i = 0; i < m_nbrStrIdx; ++i ) {
      buf.writeNextLong(m_stringIdx[i].m_strIdx);
      buf.writeNextLong(m_stringIdx[i].m_endOfItems);
   }

   // Number of infos
   buf.writeNextLong(m_infoArraySize);
   for( int i = 0; i < m_infoArraySize; ++i ) {
      buf.writeNextLong(m_infoVectorIdx[i]);
   }

   // Write the namenbrs and masks
   buf.writeNextByteArray(m_infoVectorNameNbr, m_infoArraySize);
   buf.writeNextByteArray(m_infoVectorStringPartMasks, m_infoArraySize);

   buf.alignToLongAndClear();
   
   // Done   
   dbc.assertPosition(getSizeInDataBuffer());
   return getSizeInDataBuffer();
}

int
MultiStringSearch::load(DataBuffer& buf)
{
   DataBufferChecker dbc(buf, "MultiStringSearch::load");

   // Read the version
   int version = buf.readNextLong();
   MC2_ASSERT( version == 0 ); version |= 0;

   // Number of strIdx
   m_nbrStrIdx = buf.readNextLong();
   m_stringIdx = new MultiSearchNotice[m_nbrStrIdx];

   for( int i = 0; i < m_nbrStrIdx; ++i ) {
      m_stringIdx[i].m_strIdx     = buf.readNextLong();
      m_stringIdx[i].m_endOfItems = buf.readNextLong();
   }

   // Number of infos
   m_infoArraySize = buf.readNextLong();
   m_infoVectorIdx = new uint32[m_infoArraySize];
   for( int i = 0; i < m_infoArraySize; ++i ) {
      m_infoVectorIdx[i] = buf.readNextLong();
   }

   // Create the name numbers and masks
   m_infoVectorNameNbr = new uint8[m_infoArraySize];
   m_infoVectorStringPartMasks = new uint8[m_infoArraySize];
   
   buf.readNextByteArray(m_infoVectorNameNbr, m_infoArraySize);
   buf.readNextByteArray(m_infoVectorStringPartMasks, m_infoArraySize);

   buf.alignToLong();
   
   
   // Done   
   dbc.assertPosition(getSizeInDataBuffer());
   return getSizeInDataBuffer();
}


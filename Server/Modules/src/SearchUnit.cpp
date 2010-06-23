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

#include "SearchUnit.h"
#include "SearchStruct.h"
#include "SearchMap2.h"
#include "DataBuffer.h"
#include "MC2String.h"

SearchUnit::SearchUnit()
{
   m_searchMap         = NULL;
   m_ownedMultiStringSearch = NULL;
}

SearchUnit::~SearchUnit()
{
   delete m_searchMap;
   delete m_ownedMultiStringSearch;
}

void
SearchUnit::createMultiStringSearch()
{
   m_ownedMultiStringSearch = new MultiStringSearch(m_searchMap);
}

int
SearchUnit::getSizeInDataBuffer() const
{
   return 4 + m_searchMap->getSizeInDataBuffer() +
      m_ownedMultiStringSearch->getSizeInDataBuffer();
}

int
SearchUnit::save(DataBuffer& buf) const
{
   DataBufferChecker dbc(buf, "SearchUnit::save");

   dbc.assertRoom(getSizeInDataBuffer());
   
   // Write the version
   buf.writeNextLong(0);
   
   m_searchMap->save(buf);
   m_ownedMultiStringSearch->save(buf);
   
   // Done   
   dbc.assertPosition(getSizeInDataBuffer());
   return getSizeInDataBuffer();
}


int
SearchUnit::load(DataBuffer& buf)
{
   DataBufferChecker dbc(buf, "SearchUnit::load");
   
   // Read the version
   int version = buf.readNextLong();
   MC2_ASSERT( version == 0 ); version |= 0;

   delete m_searchMap;
   delete m_ownedMultiStringSearch;
   
   m_searchMap = new SearchMap2();
   
   m_searchMap->load(buf);

   createMultiStringSearch();
   m_ownedMultiStringSearch->load(buf);
   
   // Done   
   dbc.assertPosition(getSizeInDataBuffer());
   
   return getSizeInDataBuffer();
}


/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OldSubwayLineItem.h"

OldSubwayLineItem::OldSubwayLineItem(uint32 id) 
   : OldItem(ItemTypes::subwayLineItem, id)
{
}

OldSubwayLineItem::~OldSubwayLineItem()
{
}

bool
OldSubwayLineItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(cerr << "      OldSubwayLineItem::save()" << endl;)
   return (OldItem::save(dataBuffer));
}

char*
OldSubwayLineItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString, "***** SubwayLineItem\n%s", tmpStr);
   return itemAsString;
}

bool
OldSubwayLineItem::createFromDataBuffer(DataBuffer* dataBuffer,
                                     OldGenericMap* theMap)
{
   m_type = ItemTypes::subwayLineItem;
   return (OldItem::createFromDataBuffer(dataBuffer, theMap));
}

void
OldSubwayLineItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(0, mifFile);
   mifFile << "DATA" << endl;
}

void
OldSubwayLineItem::printMidMif(ofstream& midFile, ofstream& mifFile,
		                OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   midFile << endl;
}


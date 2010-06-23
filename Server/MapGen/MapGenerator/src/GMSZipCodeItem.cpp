/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSZipCodeItem.h"
#include "StringUtility.h"
#include "VectorIncl.h"

// Call the constructor with parameter to initalize!!
GMSZipCodeItem::GMSZipCodeItem()
   :  OldZipCodeItem(MAX_UINT32),
      GMSItem(GMSItem::GDF)
{
   initMembers();
}

GMSZipCodeItem::GMSZipCodeItem(uint32 id) 
   :  OldZipCodeItem(id),
      GMSItem(GMSItem::GDF)
{
   initMembers();
}

GMSZipCodeItem::GMSZipCodeItem(GMSItem::map_t mapType)
   :  OldZipCodeItem(),
      GMSItem(mapType)
{
   initMembers();
}

void
GMSZipCodeItem::initMembers()
{
   init(ItemTypes::zipCodeItem);
}

GMSZipCodeItem::~GMSZipCodeItem()
{
}




bool
GMSZipCodeItem::readFromFile(ifstream& infile, char* streetName, 
                              char* zipCode, char* zipArea)
{
	static const int LINESIZE = 128;
	static char fstreetname[LINESIZE];
	static char fzip[LINESIZE];
	static char fzipcodestr[LINESIZE];

	bool result = infile.getline(fstreetname, LINESIZE);
	if (result) {
		result = infile.getline(fzipcodestr, LINESIZE);
		if (result) {
			result = infile.getline(fzip, LINESIZE);
			if (result) {
            StringUtility::trimEnd(fstreetname);
				strcpy(streetName, fstreetname);

            StringUtility::trimEnd(fzip);
				strcpy(zipArea, fzip);

            StringUtility::trimEnd(fzipcodestr);
				strcpy(zipCode, fzipcodestr);
				result = true;
			}
		}
	}
	return result;
}


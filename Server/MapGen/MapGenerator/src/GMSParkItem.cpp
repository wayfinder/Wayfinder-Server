/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSParkItem.h"
#include "GMSMidMifHandler.h"


GMSParkItem::GMSParkItem()
   :  OldParkItem(),
      GMSItem(GMSItem::GDF)
{
   initMembers();
}

GMSParkItem::GMSParkItem(uint32 id) 
   :  OldParkItem(id),
      GMSItem(GMSItem::GDF)
{
   initMembers();
}

GMSParkItem::GMSParkItem(GMSItem::map_t mapType)
   :  OldParkItem(),
      GMSItem(mapType)
{
   initMembers();
}

void
GMSParkItem::initMembers()
{
   init(ItemTypes::parkItem);
   m_parkType = ItemTypes::regionOrNationalPark;
}

GMSParkItem::~GMSParkItem()
{
}

bool
GMSParkItem::createFromMidMif(ifstream& midFile, bool readRestOfLine)
{
   uint32 maxLineLength = GMSMidMifHandler::maxMidLineLength;
   char inbuffer[maxLineLength];
   
   // Get and set the parktype
   // read past the first '"'
   midFile.getline(inbuffer, maxLineLength, '"');
   // get the parktype
   midFile.getline(inbuffer, maxLineLength, '"');
   
   // extract the parktype from the string
   int type = ItemTypes::getParkTypeFromString(inbuffer);
   if ( type < 0 ) {
      mc2log << fatal << "Could not extract park type from the string '"
             << inbuffer << "'" << endl;
      MC2_ASSERT(false);
      // skip the rest of the mid line
      if (readRestOfLine)
         midFile.getline(inbuffer, maxLineLength);
      return false;
   }
   
   mc2dbg8 << "parktype \"" << type << "\"" << endl;
   setParkType(type);
   
   // Check if we have any extra attributes to read,
   // don't count any map ssi coordinates
   uint32 nbrExtraAttributes = 
         GMSMidMifHandler::getNumberExtraAttributes( midFile, !readRestOfLine);

   // Read any extra item attributes
   /*
   // Template for next item attribute
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      nbrExtraAttributes--;
   }*/

   if (nbrExtraAttributes != 0) {
      mc2log << error << here << "Unused extra attributes" << endl;
   }

   // Skip the rest of the mid line, nothing there (we need to read 
   // past eol for the while-loop in GMSMap to be able to check eof)
   if (readRestOfLine)
      midFile.getline(inbuffer, maxLineLength);
   // else
   // Don't read the rest of the mid line, we have a map ssi coordinate 
   // to use for checking which map is the correct one
   
   return true;
   
}


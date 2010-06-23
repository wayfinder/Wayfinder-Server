/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSCartographicItem.h"
#include "GMSMidMifHandler.h"

GMSCartographicItem::GMSCartographicItem(): 
   OldCartographicItem(),
   GMSItem(GMSItem::GDF)
{
   initMembers();
}


GMSCartographicItem::GMSCartographicItem(ItemSubTypes::cartographicType_t 
                                         cartographicType)
   :  OldCartographicItem(),
      GMSItem(GMSItem::GDF)
{
   initMembers();
   setCartographicType(cartographicType);
}

GMSCartographicItem::GMSCartographicItem(uint32 id) 
   :  OldCartographicItem(id),
      GMSItem(GMSItem::GDF)
{
   initMembers();
}

GMSCartographicItem::GMSCartographicItem(GMSItem::map_t mapType)
   :  OldCartographicItem(),
      GMSItem(mapType)
{
   initMembers();
}

void
GMSCartographicItem::initMembers()
{
   init(ItemTypes::cartographicItem);
}

GMSCartographicItem::~GMSCartographicItem()
{
}

bool
GMSCartographicItem::createFromMidMif(ifstream& midFile, bool readRestOfLine)
{

   // get and set the cartographic type
   uint32 maxLineLength = GMSMidMifHandler::maxMidLineLength;
   char inbuffer[maxLineLength];

   //read past the first '"'
   midFile.getline(inbuffer, maxLineLength, '"');
   //get the building type
   midFile.getline(inbuffer, maxLineLength, '"');

   ItemSubTypes::cartographicType_t type = stringToCartographicType(inbuffer);
   if (type == ItemSubTypes::noCartographicType ) {
      mc2log << fatal 
             << "Could not extract cartographic type from the string."
             << endl;
      // skip the rest of the mid line
      if (readRestOfLine)
         midFile.getline(inbuffer, maxLineLength);
      return false;
   }
   mc2dbg4 << "cartographic type \"" << inbuffer << "\" = type " 
           << int(type) << endl;
   setCartographicType( type );

   // Check if we have any extra attributes to read,
   // don't count any map ssi coordinates
   uint32 nbrExtraAttributes = 
         GMSMidMifHandler::getNumberExtraAttributes( midFile, !readRestOfLine);

   // Read any extra item attributes
   
   // Template for next item attribute
   //if ( nbrExtraAttributes > 0 ) {
   //   midFile.getline(inbuffer, maxLineLength, '"');
   //   midFile.getline(inbuffer, maxLineLength, '"');
   //   nbrExtraAttributes--;
   //}

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


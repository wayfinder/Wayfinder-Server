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
#include "OldIndividualBuildingItem.h"
#include "DataBuffer.h"

OldIndividualBuildingItem::OldIndividualBuildingItem(uint32 id) 
   : OldItem(ItemTypes::individualBuildingItem, id)
{
   m_buildingType = ItemSubTypes::noIndividualBuildingType;
}

OldIndividualBuildingItem::~OldIndividualBuildingItem()
{
}

bool
OldIndividualBuildingItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(cerr << "      OldIndividualBuildingItem::save()" << endl;)

   if (OldItem::save(dataBuffer)) {
      dataBuffer->writeNextLong(m_buildingType);
      return (true);
   } else {
      return (false);
   }
}

char*
OldIndividualBuildingItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString, "***** IndividualBuildingItem\n%s", tmpStr);
   return itemAsString;
}

bool
OldIndividualBuildingItem::createFromDataBuffer(DataBuffer* dataBuffer, 
                                             OldGenericMap* theMap)
{
   m_type = ItemTypes::individualBuildingItem;
   if (OldItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_buildingType =
         ItemSubTypes::individualBuildingType_t(dataBuffer->readNextLong());
      return (true);
   } else {
      return (false);
   }
}

void
OldIndividualBuildingItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(1, mifFile);
   mifFile << "IBI_TYPE char(40)\r\n" << "DATA" << endl;
}

void
OldIndividualBuildingItem::printMidMif(ofstream& midFile, ofstream& mifFile,
		                OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   //byte type = getBuildingType();
   midFile << ",\"" ;
   MC2String typeString = buildingTypeToString(m_buildingType);
   midFile << typeString;
   midFile << "\"" << endl;
}

bool
OldIndividualBuildingItem::updateAttributesFromItem(
      OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::individualBuildingItem)) {
      return retVal;
   }

   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then item specific attributes
   OldIndividualBuildingItem* otherIbi = (OldIndividualBuildingItem*) otherItem;
   if (otherIbi != NULL) {
      if (m_buildingType != otherIbi->getBuildingType()) {
         m_buildingType = otherIbi->getBuildingType();
         retVal = true;
      }
   }

   return retVal;
}

MC2String
OldIndividualBuildingItem::buildingTypeToString(
   ItemSubTypes::individualBuildingType_t buildingType)
{
   switch (buildingType) {
   case ItemSubTypes::noIndividualBuildingType : {
      return "noIndividualBuildingType";
   } break;
   case ItemSubTypes::publicIndividualBuilding : {
      return "publicIndividualBuilding";
   } break;
   case ItemSubTypes::otherIndividualBuilding : {
      return "otherIndividualBuilding";
   } break;
   case ItemSubTypes::airportTerminal : {
      return "airportTerminal";
   } break;
   case ItemSubTypes::parkingGarage : {
      return "parkingGarage";
   } break;


   default: {
      mc2log << error << "Individual building has not handled type" 
             << int(buildingType) << endl;
      return "XXX";
   }
   } // switch
}

ItemSubTypes::individualBuildingType_t
OldIndividualBuildingItem::stringToBuildingType(MC2String typeString)
{
   if ( typeString ==  "publicIndividualBuilding" ) {
      return ItemSubTypes::publicIndividualBuilding;
   }
   else if ( typeString ==  "otherIndividualBuilding" ) {
      return ItemSubTypes::otherIndividualBuilding;
   }
   else if ( typeString == "airportTerminal" ) {
      return ItemSubTypes::airportTerminal;
   }
   else if ( typeString == "parkingGarage" ) {
      return ItemSubTypes::parkingGarage;
   }

   return ItemSubTypes::noIndividualBuildingType;
   
} // stringToBuildingType


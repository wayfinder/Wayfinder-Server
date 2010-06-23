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
#include "OldCartographicItem.h"

#include "DataBuffer.h"

OldCartographicItem::OldCartographicItem(uint32 id) 
   : OldItem(ItemTypes::cartographicItem, id)
{
   m_cartographicType = ItemSubTypes::noCartographicType;
}

OldCartographicItem::~OldCartographicItem()
{

}

bool
OldCartographicItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(cerr << "      OldCartographicItem::save()" << endl;)
   
   if (OldItem::save(dataBuffer)) {     
      dataBuffer->writeNextLong(m_cartographicType);
      return (true);
   } else {
      return (false);
   }
}

char* OldCartographicItem::toString()
{
   char tmpStr[1024];
   strcpy(tmpStr, OldItem::toString());
   sprintf(itemAsString, 
           "***** CartographicItem\n   cartographic type=%u\n%s", 
           m_cartographicType, 
           tmpStr);
   return itemAsString;
}

bool
OldCartographicItem::createFromDataBuffer(DataBuffer *dataBuffer, 
                                   OldGenericMap* theMap)
{
   
   m_type = ItemTypes::cartographicItem;
   if (OldItem::createFromDataBuffer(dataBuffer, theMap)) {
      m_cartographicType = 
         ItemSubTypes::cartographicType_t(dataBuffer->readNextLong());
      return (true);
   } else {
      return (false);
   }
}

void
OldCartographicItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(1, mifFile);
   //writeSpecificHeader(mif)   
   mifFile << "  CARTOGRAPHIC_TYPE char(40)\r\n" << "DATA" << endl;
}
void
OldCartographicItem::printMidMif(ofstream& midFile, 
                          ofstream& mifFile, 
                          OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);
   
   midFile << ",\"" ;
   MC2String cartoTypeStr = cartographicTypeToString(m_cartographicType);
   if ( cartoTypeStr == "" ) {
      mc2log << error << here << "Cartographic has invalid poi type" 
             << int(m_cartographicType) << endl;
   }
   midFile << cartoTypeStr;
   midFile << "\"" << endl;
}

bool
OldCartographicItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   bool retVal = false;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::cartographicItem)) {
      return retVal;
   }

   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then item specific attributes
   OldCartographicItem* otherCartographic = (OldCartographicItem*) otherItem;
   if (otherCartographic != NULL) {
      if (m_cartographicType != otherCartographic->getCartographicType()) {
         m_cartographicType = otherCartographic->getCartographicType();
         retVal = true;
      }
   }

   return retVal;
}

MC2String
OldCartographicItem::cartographicTypeToString(ItemSubTypes::cartographicType_t
                                              cartographicType)
{
   switch (cartographicType) {
   case ItemSubTypes::campingGround: {
      return "campingGround";
   } break;
   case ItemSubTypes::toll: {
      return "toll";
   } break;
   case ItemSubTypes::freeport: {
      return "freeport";
   } break;
   case ItemSubTypes::abbeyGround: {
      return "abbeyGround";
   } break;
   case ItemSubTypes::amusementParkGround: {
      return "amusementParkGround";
   } break;
   case ItemSubTypes::artsCentreGround: {
      return "artsCentreGround";
   } break;
   case ItemSubTypes::castleNotToVisitGround: {
      return "castleNotToVisitGround";
   } break;
   case ItemSubTypes::castleToVisitGround: {
      return "castleToVisitGround";
   } break;
   case ItemSubTypes::churchGround: {
      return "churchGround";
   } break;
   case ItemSubTypes::cityHallGround: {
      return "cityHallGround";
   } break;
   case ItemSubTypes::courthouseGround: {
      return "courthouseGround";
   } break;
   case ItemSubTypes::fireStationGround: {
      return "fireStationGround";
   } break;
   case ItemSubTypes::fortressGround: {
      return "fortressGround";
   } break;
   case ItemSubTypes::golfGround: {
      return "golfGround";
   } break;
   case ItemSubTypes::governmentBuildingGround: {
      return "governmentBuildingGround";
   } break;
   case ItemSubTypes::hospitalGround: {
      return "hospitalGround";
   } break;
   case ItemSubTypes::libraryGround: {
      return "libraryGround";
   } break;
   case ItemSubTypes::lightHouseGround: {
      return "lightHouseGround";
   } break;
   case ItemSubTypes::monasteryGround: {
      return "monasteryGround";
   } break;
   case ItemSubTypes::museumGround: {
      return "museumGround";
   } break;
   case ItemSubTypes::parkingAreaGround: {
      return "parkingAreaGround";
   } break;
   case ItemSubTypes::placeOfInterestBuilding: {
      return "placeOfInterestBuilding";
   } break;
   case ItemSubTypes::policeOfficeGround: {
      return "policeOfficeGround";
   } break;
   case ItemSubTypes::prisonGround: {
      return "prisonGround";
   } break;
   case ItemSubTypes::railwayStationGround: {
      return "railwayStationGround";
   } break;
   case ItemSubTypes::recreationalAreaGround: {
      return "recreationalAreaGround";
   } break;
   case ItemSubTypes::restAreaGround: {
      return "restAreaGround";
   } break;
   case ItemSubTypes::sportsHallGround: {
      return "sportsHallGround";
   } break;
   case ItemSubTypes::stadiumGround: {
      return "stadiumGround";
   } break;
   case ItemSubTypes::statePoliceOffice: {
      return "statePoliceOffice";
   } break;
   case ItemSubTypes::theatreGround: {
      return "theatreGround";
   } break;
   case ItemSubTypes::universityOrCollegeGround: {
      return "universityOrCollegeGround";
   } break;
   case ItemSubTypes::waterMillGround: {
      return "waterMillGround";
   } break;
   case ItemSubTypes::zooGround: {
      return "zooGround";
   } break;
   case ItemSubTypes::postOfficeGround: {
      return "postOfficeGround";
   } break;
   case ItemSubTypes::windmillGround: {
      return "windmillGround";
   } break;
   case ItemSubTypes::institution: {
      return "institution";
   } break;
   case ItemSubTypes::otherLandUse: {
      return "otherLandUse";
   } break;
   case ItemSubTypes::cemetaryGround: {
      return "cemetaryGround";
   } break;
   case ItemSubTypes::militaryServiceBranch: {
      return "militaryServiceBranch";
   } break;
   case ItemSubTypes::shoppingCenterGround: {
      return "shoppingCenterGround";
   } break;



   default: {
      return "";
   }
   } // switch
}

ItemSubTypes::cartographicType_t
OldCartographicItem::stringToCartographicType(MC2String typeString)
{
   if ( typeString ==  "amusementParkGround" ) {
      return ItemSubTypes::amusementParkGround;
   }
   else if ( typeString ==  "campingGround" ) {
      return ItemSubTypes::campingGround;
   }
   else if ( typeString ==  "toll" ) {
      return ItemSubTypes::toll;
   }
   else if ( typeString ==  "freeport" ) {
      return ItemSubTypes::freeport;
   }
   else if ( typeString ==  "abbeyGround" ) {
      return ItemSubTypes::abbeyGround;
   }
   else if ( typeString ==  "artsCentreGround" ) {
      return ItemSubTypes::artsCentreGround;
   }
   else if ( typeString ==  "castleNotToVisitGround" ) {
      return ItemSubTypes::castleNotToVisitGround;
   }
   else if ( typeString ==  "castleToVisitGround" ) {
      return ItemSubTypes::castleToVisitGround;
   }
   else if ( typeString ==  "churchGround" ) {
      return ItemSubTypes::churchGround;
   }
   else if ( typeString ==  "cityHallGround" ) {
      return ItemSubTypes::cityHallGround;
   }
   else if ( typeString ==  "courthouseGround" ) {
      return ItemSubTypes::courthouseGround;
   }
   else if ( typeString ==  "fireStationGround" ) {
      return ItemSubTypes::fireStationGround;
   }
   else if ( typeString ==  "fortressGround" ) {
      return ItemSubTypes::fortressGround;
   }
   else if ( typeString ==  "golfGround" ) {
      return ItemSubTypes::golfGround;
   }
   else if ( typeString ==  "governmentBuildingGround" ) {
      return ItemSubTypes::governmentBuildingGround;
   }
   else if ( typeString ==  "hospitalGround" ) {
      return ItemSubTypes::hospitalGround;
   }
   else if ( typeString ==  "libraryGround" ) {
      return ItemSubTypes::libraryGround;
   }
   else if ( typeString ==  "lightHouseGround" ) {
      return ItemSubTypes::lightHouseGround;
   }
   else if ( typeString ==  "monasteryGround" ) {
      return ItemSubTypes::monasteryGround;
   }
   else if ( typeString ==  "museumGround" ) {
      return ItemSubTypes::museumGround;
   }
   else if ( typeString ==  "parkingAreaGround" ) {
      return ItemSubTypes::parkingAreaGround;
   }
   else if ( typeString ==  "placeOfInterestBuilding" ) {
      return ItemSubTypes::placeOfInterestBuilding;
   }
   else if ( typeString ==  "policeOfficeGround" ) {
      return ItemSubTypes::policeOfficeGround;
   }
   else if ( typeString ==  "prisonGround" ) {
      return ItemSubTypes::prisonGround;
   }
   else if ( typeString ==  "railwayStationGround" ) {
      return ItemSubTypes::railwayStationGround;
   }
   else if ( typeString ==  "recreationalAreaGround" ) {
      return ItemSubTypes::recreationalAreaGround;
   }
   else if ( typeString ==  "restAreaGround" ) {
      return ItemSubTypes::restAreaGround;
   }
   else if ( typeString ==  "sportsHallGround" ) {
      return ItemSubTypes::sportsHallGround;
   }
   else if ( typeString ==  "stadiumGround" ) {
      return ItemSubTypes::stadiumGround;
   }
   else if ( typeString ==  "statePoliceOffice" ) {
      return ItemSubTypes::statePoliceOffice;
   }
   else if ( typeString ==  "theatreGround" ) {
      return ItemSubTypes::theatreGround;
   }
   else if ( typeString ==  "universityOrCollegeGround" ) {
      return ItemSubTypes::universityOrCollegeGround;
   }
   else if ( typeString ==  "waterMillGround" ) {
      return ItemSubTypes::waterMillGround;
   }
   else if ( typeString ==  "zooGround" ) {
      return ItemSubTypes::zooGround;
   }
   else if ( typeString ==  "postOfficeGround" ) {
      return ItemSubTypes::postOfficeGround;
   }
   else if ( typeString ==  "windmillGround" ) {
      return ItemSubTypes::windmillGround;
   }
   else if ( typeString ==  "institution" ) {
      return ItemSubTypes::institution;
   }
   else if ( typeString ==  "otherLandUse" ) {
      return ItemSubTypes::otherLandUse;
   }
   else if ( typeString ==  "cemetaryGround" ) {
      return ItemSubTypes::cemetaryGround;
   }
   else if ( typeString ==  "militaryServiceBranch" ) {
      return ItemSubTypes::militaryServiceBranch;
   }
   else if ( typeString ==  "shoppingCenterGround" ) {
      return ItemSubTypes::shoppingCenterGround;
   }

   return ItemSubTypes::noCartographicType;
   
} //stringToCartographicType


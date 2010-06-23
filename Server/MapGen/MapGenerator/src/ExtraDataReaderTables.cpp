/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExtraDataReaderTables.h"
#include "StringTable.h"

void
ExtraDataReaderTables::initItemTypes(
   map<MC2String, ItemTypes::itemType, strNoCaseCompareLess>& itemTypes)
{
   itemTypes["buildingItem"]
      = itemTypes["bi"]        = ItemTypes::buildingItem;
   itemTypes["builtUpArea"]
      = itemTypes["builtUpAreaItem"]
      = itemTypes["bua"]       = ItemTypes::builtUpAreaItem;
   itemTypes["cityAreaItem"]
      = itemTypes["cai"]
      = itemTypes["mun"]
      = itemTypes["municipal"] = ItemTypes::municipalItem;
   itemTypes["forestItem"]
      = itemTypes["fi"]        = ItemTypes::forestItem;
   itemTypes["parkItem"]
      = itemTypes["pi"]        = ItemTypes::parkItem;
   itemTypes["pointOfInterestItem"]
      = itemTypes["poi"]       = ItemTypes::pointOfInterestItem;
   itemTypes["railwayItem"]
      = itemTypes["ri"]        = ItemTypes::railwayItem;
   itemTypes["streetItem"]
      = itemTypes["si"]        = ItemTypes::streetItem;
   itemTypes["streetSegmentItem"]
      = itemTypes["ssi"]       = ItemTypes::streetSegmentItem;
   itemTypes["waterItem"]
      = itemTypes["wi"]        = ItemTypes::waterItem;
   // Add also english names for all item types from stringtable
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if (i != 13) { // Skip unsupported itemtypes.
         const char* str = StringTable::getString( 
                              ItemTypes::getItemTypeSC(ItemTypes::itemType(i)), 
                              StringTable::ENGLISH);
         itemTypes[str] = ItemTypes::itemType(i);
      }
   }
}

void 
ExtraDataReaderTables::
initNameTypes(map<MC2String, ItemTypes::name_t, strNoCaseCompareLess>& nameTypes)
{
   nameTypes["abbreviationName"]
      = nameTypes["abb"]    
      = nameTypes["ab"]       = ItemTypes::abbreviationName;
   nameTypes["alternativeName"]
      = nameTypes["an"]       = ItemTypes::alternativeName;
   nameTypes["officialName"]
      = nameTypes["on"]       = ItemTypes::officialName;
   nameTypes["roadNumber"]
      = nameTypes["RN"]
      = nameTypes["nbr"]      = ItemTypes::roadNumber;
   nameTypes["exitNumber"]
      = nameTypes["enbr"]
      = nameTypes["exit"]     = ItemTypes::exitNumber;
   nameTypes["synonymName"]
      = nameTypes["syn"]      = ItemTypes::synonymName;
}

void
ExtraDataReaderTables::
initLanguages(map<MC2String, LangTypes::language_t, strNoCaseCompareLess>& languages)
{
   languages["swedish"]
      = languages["se"]
      = languages["swe"] = LangTypes::swedish;
   languages["english"]
      = languages["uk"]
      = languages["gb"]
      = languages["eng"]
      = languages["en"]  = LangTypes::english;
   languages["german"]
      = languages["de"]
      = languages["deu"]
      = languages["ger"] = LangTypes::german;
   languages["danish"]
      = languages["dk"]
      = languages["dan"] = LangTypes::danish;
   languages["italian"]
      = languages["it"]
      = languages["ita"] = LangTypes::italian;
   languages["dutch"]
      = languages["nl"]
      = languages["dut"] = LangTypes::dutch;
   languages["spanish"]
      = languages["es"]
      = languages["esp"] = LangTypes::spanish;
   languages["french"]
      = languages["fr"]
      = languages["fre"] = LangTypes::french;
   languages["finnish"]
      = languages["fi"]
      = languages["fin"] = LangTypes::finnish;
   languages["norwegian"]
      = languages["no"]
      = languages["nor"] = LangTypes::norwegian;
   languages["portuguese"]
      = languages["po"]
      = languages["por"] = LangTypes::portuguese;
   languages["czech"]
      = languages["cz"]
      = languages["cze"] = LangTypes::czech;
}

void
ExtraDataReaderTables::initPOITypes(
   map<MC2String, ItemTypes::pointOfInterest_t, strNoCaseCompareLess>& poiTypes)
{
   poiTypes["airlineAccess"]
      = poiTypes["aa"]      = ItemTypes::airlineAccess;
   poiTypes["airport"]
      = poiTypes["ap"]      = ItemTypes::airport;
   poiTypes["amusementPark"]
      = poiTypes["amp"]     = ItemTypes::amusementPark;
   poiTypes["atm"]          = ItemTypes::atm;
   poiTypes["automobileDealership"]
      = poiTypes["amd"]     = ItemTypes::automobileDealership;
   poiTypes["bank"]
      = poiTypes["b"]       = ItemTypes::bank;
   poiTypes["beach"]
      = poiTypes["bea"]      = ItemTypes::recreationFacility; //beach;
   poiTypes["bowlingCentre"]
      = poiTypes["bc"]      = ItemTypes::bowlingCentre;
   poiTypes["busStation"]
      = poiTypes["bs"]      = ItemTypes::busStation;
   poiTypes["businessFacility"]
      = poiTypes["bf"]      = ItemTypes::businessFacility;
   poiTypes["campingGround"]
      = poiTypes["cg"]      = ItemTypes::recreationFacility; //campingGround;
   poiTypes["carDealer"]
      = poiTypes["cd"]      = ItemTypes::carDealer;
   poiTypes["casino"]
      = poiTypes["cas"]     = ItemTypes::casino;
   poiTypes["cinema"]
      = poiTypes["cin"]     = ItemTypes::cinema;
   poiTypes["cityCentre"]
      = poiTypes["cc"]      = ItemTypes::cityCentre;
   poiTypes["cityHall"]
      = poiTypes["ch"]      = ItemTypes::cityHall;
   poiTypes["communityCentre"]
      = poiTypes["comc"]    = ItemTypes::communityCentre;
   poiTypes["commuterRailStation"]
      = poiTypes["crs"]     = ItemTypes::commuterRailStation;
   poiTypes["company"]
      = poiTypes["comp"]    = ItemTypes::company;
   poiTypes["concertHall"]
      = poiTypes["concert"]      = ItemTypes::touristAttraction; //concertHall;
   poiTypes["tollRoad"]
      = poiTypes["tr"]      = ItemTypes::tollRoad;
   poiTypes["courtHouse"]
      = poiTypes["court"]   = ItemTypes::courtHouse;
   poiTypes["culturalCentre"]
      = poiTypes["cultural"]      = ItemTypes::touristAttraction; //culturalCentre;
   poiTypes["dentist"]
      = poiTypes["dent"]      = ItemTypes::dentist;
   poiTypes["doctor"]
      = poiTypes["doc"]      = ItemTypes::doctor;
   poiTypes["driveThroughBottleShop"]
      = poiTypes["dtbs"]      = ItemTypes::driveThroughBottleShop;
   poiTypes["embassy"]
      = poiTypes["emb"]      = ItemTypes::embassy;
   poiTypes["entryPoint"]
      = poiTypes["ep"]      = ItemTypes::entryPoint;
   poiTypes["exhibitionOrConferenceCentre"]
      = poiTypes["eocc"] = ItemTypes::exhibitionOrConferenceCentre;
   poiTypes["ferryTerminal"]
      = poiTypes["ft"]      = ItemTypes::ferryTerminal;
   poiTypes["frontierCrossing"]
      = poiTypes["fc"]      = ItemTypes::frontierCrossing;
   poiTypes["golfCourse"]
      = poiTypes["gc"]      = ItemTypes::golfCourse;
   poiTypes["governmentOffice"]
      = poiTypes["go"]      = ItemTypes::governmentOffice;
   poiTypes["groceryStore"]
      = poiTypes["gs"]      = ItemTypes::groceryStore;
   poiTypes["historicalMonument"]
      = poiTypes["hm"]      = ItemTypes::historicalMonument;
   poiTypes["hospital"]
      = poiTypes["hos"]     = ItemTypes::hospital;
   poiTypes["hotel"]
      = poiTypes["hot"]     = ItemTypes::hotel;
   poiTypes["iceSkatingRink"]
      = poiTypes["isr"]     = ItemTypes::iceSkatingRink;
   poiTypes["library"]
      = poiTypes["lib"]     = ItemTypes::library;
   poiTypes["marina"]
      = poiTypes["mar"]     = ItemTypes::marina;
   poiTypes["motoringOrganisationOffice"]
      = poiTypes["moo"]     = ItemTypes::motoringOrganisationOffice;
   poiTypes["mountainPass"]
      = poiTypes["mpass"]      = ItemTypes::mountainPass;
   poiTypes["mountainPeak"]
      = poiTypes["mpeak"]      = ItemTypes::mountainPeak;
   poiTypes["museum"]
      = poiTypes["mus"]     = ItemTypes::museum;
   poiTypes["musicCentre"]
      = poiTypes["mc"]      = ItemTypes::touristAttraction; //musicCentre;
   poiTypes["nightlife"]
      = poiTypes["nl"]      = ItemTypes::nightlife;
   poiTypes["openParkingArea"]
      = poiTypes["opa"]     = ItemTypes::openParkingArea;
   poiTypes["opera"]
      = poiTypes["ope"]      = ItemTypes::touristAttraction; //opera;
   poiTypes["parkAndRecreationArea"]
      = poiTypes["para"]      = ItemTypes::recreationFacility; //parkAndRecreationArea;
   poiTypes["parkAndRide"]
      = poiTypes["par"]     = ItemTypes::parkAndRide;
   poiTypes["parkingGarage"]
      = poiTypes["pg"]      = ItemTypes::parkingGarage;
   poiTypes["petrolStation"]
      = poiTypes["ps"]      = ItemTypes::petrolStation;
   poiTypes["pharmacy"]
      = poiTypes["pharm"]      = ItemTypes::pharmacy;
   poiTypes["placeOfWorship"]
      = poiTypes["pow"]      = ItemTypes::placeOfWorship;
   poiTypes["policeStation"]
      = poiTypes["pol"]     = ItemTypes::policeStation;
   poiTypes["postOffice"]
      = poiTypes["po"]      = ItemTypes::postOffice;
   poiTypes["publicSportAirport"]
      = poiTypes["psa"]     = ItemTypes::publicSportAirport;
   poiTypes["railwayStation"]
      = poiTypes["rs"]      = ItemTypes::railwayStation;
   poiTypes["recreationFacility"]
      = poiTypes["rf"]      = ItemTypes::recreationFacility;
   poiTypes["rentACarFacility"]
      = poiTypes["racf"]    = ItemTypes::rentACarFacility;
   poiTypes["rentACarParking"]
      = poiTypes["racp"]    = ItemTypes::openParkingArea; //rentACarParking;
   poiTypes["restArea"]
      = poiTypes["ra"]      = ItemTypes::restArea;
   poiTypes["restaurant"]
      = poiTypes["rest"]    = ItemTypes::restaurant;
   poiTypes["restaurantArea"]
      = poiTypes["resta"]      = ItemTypes::restaurant; //restaurantArea;
   poiTypes["scenicView"]
      = poiTypes["sv"]      = ItemTypes::touristAttraction; //scenicView;
   poiTypes["school"]
      = poiTypes["scl"]     = ItemTypes::school;
   poiTypes["shop"]         = ItemTypes::shop;
   poiTypes["shoppingCentre"]
      = poiTypes["shc"]     = ItemTypes::shoppingCentre;
   poiTypes["skiResort"]
      = poiTypes["sr"]      = ItemTypes::skiResort;
   poiTypes["sportsActivity"]
      = poiTypes["sa"]      = ItemTypes::sportsActivity;
   poiTypes["sportsCentre"]
      = poiTypes["sc"]      = ItemTypes::sportsCentre;
   poiTypes["stadium"]
      = poiTypes["stad"]      = ItemTypes::sportsActivity; //stadium;
   poiTypes["swimmingPool"]
      = poiTypes["sp"]      = ItemTypes::sportsActivity; //swimmingPool;
   poiTypes["tennisCourt"]
      = poiTypes["tc"]      = ItemTypes::sportsActivity; //tennisCourt;
   poiTypes["theatre"]
      = poiTypes["tht"]     = ItemTypes::theatre;
   poiTypes["touristAttraction"]
      = poiTypes["ta"]      = ItemTypes::touristAttraction;
   poiTypes["touristOffice"]
      = poiTypes["to"]      = ItemTypes::touristOffice;
   poiTypes["university"]
      = poiTypes["univ"]    = ItemTypes::university;
   poiTypes["vehicleRepairFacility"]
      = poiTypes["vrf"]     = ItemTypes::vehicleRepairFacility;
   poiTypes["vetrinarian"]
      = poiTypes["vet"]      = ItemTypes::vetrinarian;
   poiTypes["waterSports"]
      = poiTypes["ws"]      = ItemTypes::sportsActivity; //waterSports;
   poiTypes["winery"]
      = poiTypes["win"]     = ItemTypes::winery;
   poiTypes["yachtBasin"]
      = poiTypes["yb"]      = ItemTypes::marina; //yachtBasin;
   poiTypes["zoo"]          = ItemTypes::touristAttraction; //zoo;
   poiTypes["tramStation"]
      = poiTypes["tram"]    = ItemTypes::tramStation;
}


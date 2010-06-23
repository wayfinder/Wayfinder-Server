#!/usr/bin/perl
#

use strict;

package TypeMappings;

my %types;

sub get_mapped_types {
	my ($in_type) = @_;

	return $types{$in_type};
	
}


$types{"0"} = "0"; # | company                      |
$types{"1"} = "1"; #airport                      |
$types{"2"} = "2"; #amusementPark                |
$types{"3"} = "3"; #atm                          |
$types{"4"} = "4,66"; #automobileDealership         |
$types{"5"} = "5"; #bank                         |
$types{"6"} = "6,44"; #bowlingCentre                |
$types{"7"} = "7"; #busStation                   |
$types{"8"} = "8"; #businessFacility             |
$types{"9"} = "9"; #casino                       |
$types{"10"} = "10"; #cinema                       |
$types{"11"} = "11"; #cityCentre                   |
$types{"12"} = "12"; #cityHall                     |
$types{"13"} = "13"; #communityCentre              |
$types{"14"} = "14,36"; #commuterRailStation          |
$types{"15"} = "15"; #courtHouse                   |
$types{"16"} = "16"; #exhibitionOrConferenceCentre |
$types{"17"} = "17"; #ferryTerminal                |
$types{"18"} = "18"; #frontierCrossing             |
$types{"19"} = "19,44"; #golfCourse                   |
$types{"20"} = "20,56"; #groceryStore                 |
$types{"21"} = "21,47"; #historicalMonument           |
$types{"22"} = "22"; #hospital                     |
$types{"23"} = "23"; #hotel                        |
$types{"24"} = "24,44"; #iceSkatingRink               |
$types{"25"} = "25"; #library                      |
$types{"26"} = "26"; #marina                       |
$types{"27"} = "27"; #motoringOrganisationOffice   |
$types{"28"} = "28,47"; #museum                       |
$types{"29"} = "29,40"; #nightlife                    |
$types{"30"} = "30"; #openParkingArea              |
$types{"31"} = "31"; #parkAndRide                  |
$types{"32"} = "32"; #parkingGarage                |
$types{"33"} = "33"; #petrolStation                |
$types{"34"} = "34"; #policeStation                |
$types{"35"} = "35,1"; #publicSportAirport           |
$types{"36"} = "36,14"; #railwayStation               |
$types{"37"} = "37"; #recreationFacility           |
$types{"38"} = "38"; #rentACarFacility             |
$types{"39"} = "39"; #restArea                     |
$types{"40"} = "40,29"; #restaurant                   |
$types{"41"} = "41"; #school                       |
$types{"42"} = "42"; #shoppingCentre               |
$types{"43"} = "43,44"; #skiResort                    |
$types{"44"} = "44"; #sportsActivity               |
$types{"45"} = "45,44"; #sportsCentre                 |
$types{"46"} = "46"; #theatre                      |
$types{"47"} = "47,21,28,85"; #touristAttraction            |
$types{"48"} = "48"; #touristOffice                |
$types{"49"} = "49"; #university                   |
$types{"50"} = "50"; #vehicleRepairFacility        |
$types{"51"} = "51"; #winery                       |
$types{"52"} = "52"; #postOffice                   |
$types{"53"} = "53"; #tramStation                  |
$types{"56"} = "56,20,72"; #shop                         |
$types{"57"} = "57"; #cemetery                     |
$types{"58"} = "58"; #industrialComplex            |
$types{"62"} = "62"; #unknownType                  |
$types{"63"} = "63"; #airlineAccess                |
$types{"64"} = "64"; #beach                        |
$types{"65"} = "65"; #campingGround                |
$types{"66"} = "66,4"; #carDealer                    |
$types{"67"} = "67"; #concertHall                  |
$types{"68"} = "68"; #tollRoad                     |
$types{"69"} = "69"; #culturalCentre               |
$types{"70"} = "70"; #dentist                      |
$types{"71"} = "71"; #doctor                       |
$types{"72"} = "72,56"; #driveThroughBottleShop       |
$types{"73"} = "73"; #embassy                      |
$types{"74"} = "74"; #entryPoint                   |
$types{"75"} = "75"; #governmentOffice             |
$types{"76"} = "76"; #mountainPass                 |
$types{"77"} = "77"; #mountainPeak                 |
$types{"78"} = "78"; #musicCentre                  |
$types{"79"} = "79"; #opera                        |
$types{"80"} = "80"; #parkAndRecreationArea        |
$types{"81"} = "81"; #pharmacy                     |
$types{"82"} = "82"; #placeOfWorship               |
$types{"83"} = "83"; #rentACarParking              |
$types{"84"} = "84"; #restaurantArea               |
$types{"85"} = "85,47"; #scenicView                   |
$types{"86"} = "86"; #stadium                      |
$types{"87"} = "87,44"; #swimmingPool                 |
$types{"88"} = "88,44"; #tennisCourt                  |
$types{"89"} = "89"; #vetrinarian                  |
$types{"90"} = "90,44"; #waterSports                  |
$types{"91"} = "91"; #yachtBasin                   |
$types{"92"} = "92"; #zoo                          |
$types{"93"} = "93"; #wlan                         |
$types{"94"} = "94"; #noType                       |
$types{"95"} = "95"; #invalidPOIType               |
$types{"96"} = "96"; # church
$types{"97"} = "97"; # mosque
$types{"98"} = "98"; # synagogue
$types{"99"} = "99"; # subway
$types{"100"} = "100"; # cafe
$types{"101"} = "101"; # hinduTemple
$types{"102"} = "102"; # buddhist site
$types{"103"} = "103"; # busstop
$types{"104"} = "104"; # taxi stop



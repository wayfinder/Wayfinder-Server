#!/usr/bin/perl
#

package GDFPoiFeatureCodes;

use strict;

use PerlWASPTools;

require Exporter;
our @ISA       = qw( Exporter );
our @EXPORT    = qw( &get_type_and_cat_from_gdf_feature_code );

my %poi_type_trans;

# Mapping ItemTypes::poiType to poiTypeID
#
# The unwantedPOIType returns -1, which in midmif-content means
# that the poi is not wanted = it is never written toCPI/added to WASP

my $type_select = "select typeName, ID from POITypeTypes";
my $dbh = db_connect();
my $sth = $dbh->prepare($type_select);
$sth->execute();
while ( my($typeName, $typeID) = $sth->fetchrow()) {
   $poi_type_trans{$typeName} = $typeID;
}
$poi_type_trans{'unwantedPOIType'} = -1;
db_disconnect($dbh);

# type and category from GDF feature code
my %gdfFeatureCodeMap = (

      # POI Feature codes
      # TeleAtlas POI specification 1.7
      '7356' => ['airlineAccess', 0], # Airline Access
      '7383' => ['airport', 0], # Airport
      '9902' => ['amusementPark', 0], # Amusement Park
      '9910' => ['carDealer', 0], # Automotive Dealer
      '7328' => ['bank', 0], # Bank
      '9357' => ['beach', 0], # Beach
      '7378' => ['businessFacility', 0], # Business Park
      '9376' => ['cafe', 0], # Café/Pub
      '7360' => ['campingGround', 0], # Camping Ground
      '9155' => ['noType', 0], # Car Wash # could use "POIInfoKey: Has carwash" ?
      '7397' => ['atm', 0], # Cash Dispenser
      '7341' => ['casino', 0], # Casino
      '7342' => ['cinema', 0], # Cinema
      '7379' => ['cityCentre', 0], # City Center
      '7377' => ['university', 0], # College/University
      '7389' => ['noType', 0], # access gateway / airport terminal
      '8099' => ['noType', 0], # Mountain peak
      '9382' => ['noType', 0], # Commercial Building
      '9352' => ['company', 0], # Company
      '9381' => ['noType', 0], # Condominium
      '9363' => ['courtHouse', 0], # Courthouse
      '7319' => ['touristAttraction', 0], # Cultural Center
      '9374' => ['dentist', 0], # Dentist
      '7327' => ['shop', 59], # Department Stores
      '9373' => ['doctor', 0], # Doctor
      '7365' => ['embassy', 0], # Embassy
      '9900' => ['noType', 271], # Entertainment
      '9920' => ['entryPoint', 0], # Entry Point
      '9160' => ['noType', 0], # Exchange # use subcat to set category currency exchange
      '9377' => ['exhibitionOrConferenceCentre', 0], # Exhibition & Convention Center
      '7352' => ['ferryTerminal', 0], # Ferry Terminal
      '7392' => ['noType', 0], # Fire Station/Brigade
      '7366' => ['frontierCrossing', 0], # Frontier Crossing
      '9911' => ['golfCourse', 0], # Golf Course
      '7367' => ['governmentOffice', 0], # Government Office
      '9663' => ['noType', 0], # Health Care Services
      '9356' => ['sportsActivity', 51], # Hippodrome
      '7321' => ['hospital', 0], # Hospital/Polyclinic
      '7314' => ['hotel', 0], # Hotel/Motel
      '9360' => ['iceSkatingRink', 0], # Ice Skating Rink
      '7376' => ['touristAttraction', 0], # Important Tourist Attraction
      '9383' => ['noType', 0], # Industrial Building
      '9378' => ['sportsActivity', 0], # Leisure Center
      '9913' => ['library', 0], # Library
      '9156' => ['noType', 0], # Manufacturing Facility
      '9158' => ['noType', 0], # Media Facility
      '9388' => ['noType', 81], # Military Installation
      '9935' => ['mountainPass', 0], # Mountain Pass
      '9364' => ['mountainPeak', 0], # Mountain Peak
      '7317' => ['museum', 0], # Museum
      '9213' => ['noType', 0], # Native Reservation
      '9379' => ['nightlife', 0], # Nightlife
      '9152' => ['noType', 109], # Non Governmental Organization # cat = organization
      '7369' => ['openParkingArea', 0], # Open Parking Area
      '9362' => ['parkAndRecreationArea', 0], # Park & Recreation Area
      '7313' => ['parkingGarage', 0], # Parking Garage
      '7311' => ['petrolStation', 0], # Petrol Station
      '7326' => ['pharmacy', 0], # Pharmacy
      '7339' => ['placeOfWorship', 0], # Place of Worship
      '7322' => ['policeStation', 0], # Police Station
      '9159' => ['noType', 0], # Port/Warehouse Facility
      '7324' => ['postOffice', 0], # Post Office
      '9150' => ['noType', 0], # Primary Resources/Utilities
      '9154' => ['noType', 0], # Prison/Correctional Facility
      '9932' => ['noType', 0], # Public Amenities
      '9942' => ['unknownType', 0], # Public Transport Stop # Set later by bus stop type and public transp type
      '7380' => ['railwayStation', 0], # Railway Station
      '7312' => ['rentACarFacility', 0], # Rent-a-Car Facility
      '9930' => ['openParkingArea', 0], # Rent-a-Car-Parking
      '7310' => ['vehicleRepairFacility', 0], # Repair Facility
      '9157' => ['noType', 0], # Research Facility
      '7395' => ['restArea', 0], # Rest Area
      '7315' => ['restaurant', 0], # Restaurant
      '9359' => ['restaurant', 0], # Restaurant Area
      '7337' => ['scenicView', 0], # Scenic/Panoramic View
      '7372' => ['school', 0], # School
      '9361' => ['shop', 0], # Shop
      '7373' => ['shoppingCentre', 0], # Shopping Center
      '7320' => ['sportsCentre', 0], # Sports Center
      '7374' => ['stadium', 0], # Stadium
      '7332' => ['shop', 59], # Supermarket & Hypermarket
      '7338' => ['swimmingPool', 0], # Swimming Pool # pre 201003 ("sportsActivity");
      '9369' => ['tennisCourt', 0], # Tennis Court
      '7318' => ['theatre', 0], # Theater
      '7375' => ['noType', 0], # Toll Gate
      '7316' => ['touristOffice', 0], # Tourist Information Office
      '9151' => ['noType', 0], # Transport Authority/Vehicle Registration
      '9375' => ['veterinarian', 0], # Veterinarian
      '9371' => ['waterSports', 0], # Water Sport
      '9153' => ['noType', 0], # Welfare Organization
      '7349' => ['winery', 0], # Winery
      '9380' => ['marina', 0], # Yacht Basin
      '9927' => ['zoo', 0], # Zoo, Arboreta & Botanical Garden
      ####
      # old POI Feature codes
      # from different sources
      # not in TA POI 1.7
      '9112' => 
         ["amusementPark", 0],
      '9102' => 
         ["atm", 0],
      '9126' => 
         ["carDealer", 0],
      '9118' => 
         ["bowlingCentre", 0],
      '7384' => 
         ["busStation", 0],
      '9119' => 
         ["casino", 0],
      '9120' => 
         ["cinema", 0],
      '7323' => 
         ["cityHall", 0],
      '7363' => 
         ["communityCentre", 0],
      '9110' => 
         ["commuterRailStation", 0],
      '9367' => #TA
         ["concertHall", 0],
         # pre 200704 ("touristAttraction");
      '9111' => 
         ["courtHouse", 0],
      '9358' => #TA
         ["driveThroughBottleShop", 0],
      '7385' => 
         ["exhibitionOrConferenceCentre", 0], 
      '9101' => 
         ["golfCourse", 0],
      '9105' => 
         ["groceryStore", 0],
      '9113' => 
         ["historicalMonument", 130],
      '9121' => 
         ["iceSkatingRink", 0],
      '9107' => 
         ["library", 0],
      '9108' => 
         ["marina", 0],
      '7368' => 
         ["motoringOrganisationOffice", 0],
      '9372' => #TA
         ["touristAttraction", 0],
      '9122' => 
         ["nightlife", 0],
      '9365' => #TA
         ["opera", 0],
      '7387' => 
         ["parkAndRide", 0],
      '9123' => 
         ["publicSportAirport", 0],
      '7370' => 
         ["recreationFacility", 0],
      '7334' => 
         ["skiResort", 0], # ski lift station
      '9124' => 
         ["sportsCentre", 0],
      '9125' => 
         ["winery", 0],
####
      '9977' =>
         ["exhibitionOrConferenceCentre", 0],
      '7331' => # Warehouse
         ["shop", 0],
      '9354' => # ATM from InfoTech
         ["atm", 0],
      '9370' => # Skating Rink
         ["iceSkatingRink", 0],

      '7329' => # TA NA Travel Agency
         ["noType", 55], #"cat travel agent"
      '7391' => # TA NA Emergency Medical Service
         ["noType", 0],
      '9600' => # TA NA General POI
         ["unknownType", 0],
      '9645' => # TA NA Breakdown Service
         ["noType", 0],
      '9646' => # TA NA Vehicle Equipment Provider
         ["noType", 0],
      '9903' => # TA NA Arts Centre
         ["noType", 79], # cat "art"
      '9906' => # TA NA church
         ["church", 0],
      '9390' => # TA NA Cemetery
         ["cemetery", 0],

      '7340' => # TA MN Shape: Transport Company
         ["company", 0],
      '7386' => # TA MN Shape: Kindergarten
         ["noType", 0],
      '7390' => # TA MN Shape: Emergency Call Station
         ["noType", 0],
      '7500' => # TA MN Shape: Brunnel
         ["unwantedPOIType", 0],
      '9904' => # TA MN Shape: Building Footprint
         ["noType", 0],
      '9909' => # TA MN Shape: Fortress
         ["noType", 0],
      '9914' => # TA MN Shape: Lighthouse
         ["noType", 0],
      '9918' => # TA MN Shape: Natural Reserve
         ["noType", 0],
      '9919' => # TA MN Shape: Prison
         ["noType", 0],
      '9921' => # TA MN Shape: Rocks
         ["noType", 0],
      '9922' => # TA MN Shape: Sports Hall
         ["sportsCentre", 0],
      '9924' => # TA MN Shape: Walking Area
         ["noType", 0],
      '9926' => # TA MN Shape: Windmill
         ["noType", 0],
      '9931' => # TA MN Shape: Car Racetrack
         ["sportsActivity", 0],
      '9964' => # TA MN Shape: ?????????
         ["unknownType", 0],
      '7336' => #TA MN Shape
         ["zoo", 0],
      '7330' => #TA MN (Public Phone)
         ["noType", 0],
      '7361' => #TA MN (Caravan Site)
         ["campingGround", 0],
      '9650' => #TA MN (MANUFACTURERS AND SUPPLIERS, Sensis)
         ["noType", 0],
      '9660' => #TA MN CONSTRUCTION AND HOME IMPROVEMENTS,Sensis
         ["noType", 0],
      '9661' => #TA MN (SERVICE PROVIDERS, Provided by Sensis)
         ["noType", 0],
      '9662' => #TA MN (PRIMARY PRODUCERS, Provided by Sensis)
         ["noType", 0], 
      '9664' => #TA MN (HORTICULTURE, Provided by Sensis)
         ["noType", 0],
      '9937' => #TA MN (CLUBS AND ASSOCIATIONS, by Sensis)
         ["noType", 0],
      '9992' => #TA MN (LAUNDRY / DRY CLEANER, by Sensis)
         ["noType", 73], # cat "drycleaners"
      '9387' => # TA forest area
         ["parkAndRecreationArea", 0],
      '9389' => # TA native reservation
         ["noType", 0],
      '7371' => # TA Road Side Diner
         ["noType", 184], # cat restaurants:local
      '9917' => # TA Monument
         # since monument is child category to tourist attraction
         ["touristAttraction", 130], # cat "monument"
);

sub get_type_and_cat_from_gdf_feature_code {

   my $gdf = $_[0];
   my $noDie = $_[1];

   my $type;
   my $cat;

   if ( ! ( $gdf =~ /^[0-9][0-9]*$/ ) ){
      print "Error: gdf is: $gdf\n";
   } 
   if (exists $gdfFeatureCodeMap{$gdf}) {
      if (defined $gdfFeatureCodeMap{$gdf}[0]) {
         $type = $poi_type_trans{$gdfFeatureCodeMap{$gdf}[0]};
         
         if ($gdfFeatureCodeMap{$gdf}[1] > 0) {
            $cat = $gdfFeatureCodeMap{$gdf}[1];
         } else {
            $cat = undef;
         }
      }

   } else {
      # don't know the poi type of that, edit!
      print "WARN GDFPoiFeatureCodes get_type_and_cat_from_gdf_feature_code: " .
            " GDF feature type not defined $gdf - edit script!!!\n";
      if ( !defined $noDie ) {
         die "exit";
      }
      return ("invalidPOIType", undef);
   }
   return ($type, $cat)
}

1;


=head1 NAME 

GDFPoiFeatureCodes

Package mapping GDF Feature codes to Wayfinder poi types.

=head1 USE

Include into your perl file with the combination of:
   use lib "${BASEGENFILESPATH}/script/perllib";
   use GDFPoiFeatureCodes;
pointing to the directory where the perl modules are stored

=head1 FUNCTIONS

get_type_and_cat_from_gdf_feature_code


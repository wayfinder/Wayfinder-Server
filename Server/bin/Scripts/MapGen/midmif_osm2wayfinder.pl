#!/usr/bin/perl -w
# Script to create Wayfinder midmif files from OSM midmif files.
#

# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;

# Print command line.
print "Command: $0";
foreach my $arg (@ARGV){
   print " $arg";
}
print "\n\n";




use strict;

use Getopt::Std;

use vars qw( $opt_h $opt_s $opt_r $opt_v $opt_t $opt_y $opt_l
             $opt_c );
use vars qw( $m_dbh 
             %m_poiTypes %m_ccDisplayClasses
             %m_streetTypes %m_roadDisplayClasses
             $m_langStr $m_dbLangId $m_countryID
             $m_coordOrder );

# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use POIObject;
use PerlWASPTools;
use PerlTools;

use DBI;


getopts('hs:r:v:t:l:c:y:');


if (defined $opt_h) {
   print <<EOM;
Script to create Wayfinder midmif files from some midmif files.

Usage: $0 
       options
       midmif-file(s)

Options:
-h          Display this help.

-y string   Coordinate system in mif files, complete with coord order,
            e.g. 'wgs84_latlon_deg'. See Wayfinder midmif spec for
            possible value.
-s string   The mid file separator char (default ",")
-r string   A temp char to use as separator when having mid file separators
            within strings (default "}")
-c string   The 2-letter ISO contry code of the country to process
-l string   The language string to use for official names. Check with
            Wayfinder midmif spec to get a valid language string.
            eng - english
-v string   The version of the data (the format specification may differ
            between the versions):
               OSM_201005            OpenStreetMap data from Geofabrik
-t string   The type of item that is in the mid/mif file to process:
               buildings         mid+mif
               naturals          mid+mif  (forest, water, park, riverbank)
               network           mid+mif  (Network (ssi+ferry))
                  Will print forbidden turns to wf turn table
               railway           mid+mif
               waterways         mid+mif

               poi               mid+mif  (POIs)
               cityCentre        mid+mif  (places file contains city centres
                                           and some other stuff)





Note:
   * Run dos2unix on all midmif files before running this script
     to avoid windows eol.
   * POI-files need to have UTF-8 char encoding (and mc2-coords) before
     added to WASP POI db with poi_autoImport script.
EOM
  exit 1;
}

# Hash mapping poi types to WF POI categories
# Only set the categories that are not given from WF POI type, see WASP 
# POICategoryTypes table, 
# e.g. special food type restaurant that are sub categories to restaurant
my %types2wfcat = ();

# template
# $types2wfcat{"OSM-POI-TYPE-UPPER-CASE"} = wfPOICatID;

$types2wfcat{"CHINA RESTAURANT"} = 208; # chinese kitchen (restaurant)
$types2wfcat{"PIZZERIA"} = 175;  # pizzeria (restaurant)
$types2wfcat{"FAST_FOOD"} = 202; # fast food (restaurant)





# Connect to database for working with POI objects..
sub connectToDBH {
   $m_dbh = db_connect();
}
sub disconnectFromDBH {
   $m_dbh->disconnect();
}


# Open the files in tail
my $MID_FILE_NAME;
if ( scalar @ARGV >= 1) {
   $MID_FILE_NAME = $ARGV[ 0 ];
   open MID_FILE, $MID_FILE_NAME
      or die "Can not open mid file $MID_FILE_NAME: $!\n";
}
my $SECOND_FILE_NAME;
if ( scalar @ARGV >= 2) {
   $SECOND_FILE_NAME = $ARGV[ 1 ];
   open SECOND_FILE, $SECOND_FILE_NAME
      or die "Can not open second file $SECOND_FILE_NAME: $!\n";
}

# Define global variables
my $midSepChar = ',';
my $midRplChar = '}';
my $MAX_UINT32 = 4294967295;
my %m_unknownWaterwayTypes = ();
my %m_unknownStreetTypes = ();
my %m_unknownPOIcategories = ();
my %m_unknownPlaceTypes = ();

# Check that map version is valid.
if ( !defined($opt_v) ) {
   die "Midmif map version not specified! - exit\n";
} else {
   if ( ($opt_v eq "OSM_201005")
        ) {
      dbgprint "Extract data, version $opt_v";
   } else {
      die "Invalid version specified ($opt_v) - exit!!";
   }
}

# Check that country id given for those map releases that requires it
if ( !defined($opt_c) ) {
   die "Country (2 letter ISO-code) not given - exit";
} else {
   connectToDBH();
   my $id = getCountryIDFromISO3166Code($m_dbh, $opt_c);
   disconnectFromDBH();
   if ( !defined $id ) {
      die "No country with iso code $opt_c - exit"
   } else {
      dbgprint "Parsing data in country $opt_c with country id $id";
      $m_countryID = $id;
   }
}

# get language to use for the country
$m_langStr = "";
if ( !defined($opt_l) ) {
   # define the language string, as it is given in Wayfinder midmif spec
   # The short name in column 0 of LangTypes::languageAsString
   if ( $opt_c eq "DK" ) {
      $m_langStr = "dan";
   }
   elsif ( $opt_c eq "SE" ) {
      $m_langStr = "swe";
   }
   else {
      die "Language for names in $opt_c not specified! - exit";
   }
} else {
   dbgprint "Will use nameStr '$opt_l' for names";
   $m_langStr = "$opt_l";

}
if ( length $m_langStr > 1 ) {
   # get also language id for POIs from the WASP POI database 
   # POINameLanguages table
   if ( $m_langStr eq "eng" ) {
      $m_dbLangId = 0;
   } elsif ( $m_langStr eq "swe" ) {
      $m_dbLangId = 1;
   } elsif ( $m_langStr eq "dan" ) {
      $m_dbLangId = 3;
   } else {
      die "Please define WASP POI language id for language $m_langStr";
   }
   dbgprint "Will use language id $m_dbLangId for poi names";
} else {
   die "The m_langStr not populated! - exit";
}


# Must give the coordinate system
# Extract coordinate order from the coord sys string
if ( ! defined $opt_y) {
   die "No coordinate system given";
} else {
   if ( index($opt_y, "latlon") > -1 ) {
      $m_coordOrder = "latlon";
   }
   elsif ( index($opt_y, "lonlat") > -1 ) {
      $m_coordOrder = "lonlat";
   }
   else {
      die "Cannot extract coord order from the coord system $opt_y";
   }
}
print "\n";

# Check what to do
if (defined($opt_t)) {
   if ($opt_t eq "railway") {
      handleRailwayFile();
   } elsif ($opt_t eq "buildings") {
      handleBuildingsFile();
   } elsif ($opt_t eq "waterways") {
      handleWaterwaysFile();
   } elsif ($opt_t eq "naturals") {
      handleNaturalsFile();
   } elsif ($opt_t eq "network") {
      handleNetworkFile();
   } elsif ($opt_t eq "poi") {
      handlePoiFile();
   } elsif ($opt_t eq "cityCentre") {
      handleCCFile();
   } else {
      die "Unknown item type: $opt_t\n";
   }
} else {
   die "No item type specified! Run $0 -h for help.\n";
}


exit;



# Function to check midId, that it is not large than MAX_UINT32
# Parseonly mode: the function returns 1 if midId is ok, it reutrns 0 
# if the midId is to large
# Running live-mode: the function dies if the midId is too large
sub checkMidId {
   my $midId = $_[0];
   my $parseOnly = $_[1];

   my $retVal = 1;
   if ( $midId > $MAX_UINT32 ) {
      $retVal = 0;
      if ( ! $parseOnly ) {
         errprint "Mid id is larger than MAX_UINT32 ($midId)";
         dbgprint "Use row number plus an offset " .
                  "- check in mid file to get a good offset!";
         die "exit";
      }
   }

   if ( $midId <= 0 ) {
      $retVal = 0;
      if ( ! $parseOnly ) {
         errprint "Mid id is too small ($midId)";
         die "exit";
      }
   }
   
   return $retVal;
}




# This action processes a railway file
sub handleRailwayFile {
   dbgprint "handleRailwayFile";
   
   # Define attribute columns, 0,1,2,... and whether to use name or not
   my $useName = 1;
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   if ( ($opt_v eq "OSM_201005") ) {
      $midIdCol = 0;
      $nameCol = 1;
      $typeCol = 2;
      $useName = 0;
   } else {
      die "Define attribute columns for $opt_v";
   }
   
   # Open Wayfinder mid files
   open(WF_FILE, ">WFrailwayItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFrailwayItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
   my %usedids = ();
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() >= 0) {
         $nbrItemsInFile += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         # id
         my $midId = $.;
         if ( (length($midIdCol) > 0) and
              (length($midRow[$midIdCol]) > 0) ) {
            $midId = $midRow[ $midIdCol ];
            $midId = int $midId;
         }
         if ( defined($usedids{"$midId"}) ) {
            errprint "Duplicated id: $midId"; 
            die "exit";
         } else {
            $usedids{"$midId"} = 1;
         }
         
         my $itemName = "";
         if ( $useName and length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1 ) {
            $nbrItemsWithName += 1;
            $allNames = "$itemName:officialName:$m_langStr";
         }
         
         
         # print WF mid file
         print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";

         # Read next feature from mif file and write to correct WF mif
         my $wanted = 1;
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Read totally $nbrItemsInFile items from file, " .
            "$nbrItemsWithName have name.";
} # handleRailwayFile

# This action processes a building file
sub handleBuildingsFile {
   dbgprint "handleBuildingsFile";
   
   # Define attribute columns, 0,1,2,... and whether to use name or not
   my $useName = 1;
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   if ( ($opt_v eq "OSM_201005") ) {
      $midIdCol = 0;
      $nameCol = 1;
      $typeCol = 2;
      $useName = 1;
   } else {
      die "Define attribute columns for $opt_v";
   }
   
   # Open Wayfinder mid files
   open(WF_FILE, ">WFindividualBuildingItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFindividualBuildingItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
   my %usedids = ();
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() >= 0) {
         $nbrItemsInFile += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         # id
         my $midId = $.;
         if ( (length($midIdCol) > 0) and
              (length($midRow[$midIdCol]) > 0) ) {
            $midId = $midRow[ $midIdCol ];
            $midId = int $midId;
         }
         if ( defined($usedids{"$midId"}) ) {
            errprint "Duplicated id: $midId"; 
            die "exit";
         } else {
            $usedids{"$midId"} = 1;
         }
         
         my $itemName = "";
         if ( $useName and length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1 ) {
            $nbrItemsWithName += 1;
            $allNames = "$itemName:officialName:$m_langStr";
         }
         
         
         # print WF mid file
         print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";

         # Read next feature from mif file and write to correct WF mif
         my $wanted = 1;
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Read totally $nbrItemsInFile items from file, " .
            "$nbrItemsWithName have name.";
} # handleBuildingsFile

sub handleWaterwaysFile {
   dbgprint "handleWaterwaysFile";
   
   # Define attribute columns, 0,1,2,... and whether to use name and id or not
   my $useName = 1;
   my $useMidId = 1;
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   my $widthCol = "";
   if ( ($opt_v eq "OSM_201005") ) {
      $useName = 1;
      $useMidId = 1;
      $midIdCol = 0;
      $nameCol = 1;
      $typeCol = 2;
      $widthCol = 3;
   } else {
      die "Define attribute columns for $opt_v";
   }
   
   # Open Wayfinder mid files
   open(WF_FILE, ">WFwaterways.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFwaterways.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrWantedItems = 0;
   
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
   my %usedids = ();
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() >= 0) {
         $nbrItemsInFile += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         # for id, use row number 
         my $midId = $.;
         if ( $useMidId and length $midIdCol > 0 ) {
            $midId = $midRow[ $midIdCol ];
            $midId = int $midId;
         }
         if ( defined($usedids{"$midId"}) ) {
            errprint "Duplicated id: $midId"; 
            die "exit";
         } else {
            $usedids{"$midId"} = 1;
         }
         
         my $itemName = "";
         if ( $useName and length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1 ) {
            $nbrItemsWithName += 1;
            $allNames = "$itemName:officialName:$m_langStr";
         }

         # set Wyafinder water type from the OSM type
         # Most water line features can be skipped as lines are not
         # drawn much in map images, espcially skip the types that do 
         # not have many features
         my $type = "";
         if ( length($typeCol) > 0 ) {
            $type = $midRow[ $typeCol ];
         }
         my $waterType = "river";
         my $wanted = 1;
         if ( $type eq "stream" ) { $waterType = "river"; }
         elsif ( $type eq "river" ) { $waterType = "river"; }
         elsif ( $type eq "ditch" ) { $waterType = "river"; }
         elsif ( $type eq "canal" ) { $waterType = "canal"; }
         elsif ( $type eq "derelict_canal" ) { $waterType = "canal"; }
         elsif ( $type eq "stream; drain" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "drain" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "dock" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "dam" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "boatyard" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "dry_dock" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "grund" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "lock_gate" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "mooring" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "quay" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "rapids" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "riverbank_to_be_" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "shallow" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "shipyard" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "waterfall" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "weir" ) { $waterType = ""; $wanted=0; }
         elsif ( $type eq "yes" ) { $waterType = ""; $wanted=0; }
         else {
            if ( defined $m_unknownWaterwayTypes{$type} ) {
               $m_unknownWaterwayTypes{$type} += 1;
            } else {
               $m_unknownWaterwayTypes{$type} = 1;
               print "unknown waterway type $type\n";
            }
         }
         
         # print WF mid file
         if ( $wanted ) {
            $nbrWantedItems += 1;
            print WF_FILE "$midId,\"$itemName\",\"$allNames\"" .
                          ",\"$waterType\"\n";
         }

         # Read next feature from mif file and write to correct WF mif
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Read totally $nbrItemsInFile items from file, " .
            "$nbrItemsWithName have name.";
   
   if ( scalar (keys %m_unknownWaterwayTypes) > 0 ) {
      errprint "We have unknown waterway types - edit parse script!";
      foreach my $r ( sort (keys (%m_unknownWaterwayTypes)) ) {
         print "  waterway type \"$r\" has "
               . $m_unknownWaterwayTypes{ $r } . " waters\n";
      }
      errprint "add these types in the handleWaterwaysFile function";
      die "exit";
   }
   
   dbgprint "Wrote $nbrWantedItems items to WF midmif file";
   
} # handleWaterwaysFile


# This action processes a naturals file (water, forest, park)
sub handleNaturalsFile {
   dbgprint "handleNaturalsFile";
   
   # Define attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   my $useName = 1;
   if ( ($opt_v eq "OSM_201005") ) {
      $midIdCol = 0;
      $nameCol = 1;
      $typeCol = 2;
   } else {
      die "Define attribute columns for $opt_v";
   }
   
   # Open Wayfinder waterpoly + forest + park mid files
   open(WF_WATER_FILE, ">WFwaterpoly.mid");
   open(WF_FOREST_FILE, ">WFforestItems.mid");
   open(WF_PARK_FILE, ">WFparkItems.mid");
   # Open Wayfinder water + forest + park mif files,
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outWaterFileName = "WFwaterpoly.mif";
   printOneWayfinderMifHeader( $outWaterFileName, "$opt_y" );
   my $outForestFileName = "WFforestItems.mif";
   printOneWayfinderMifHeader( $outForestFileName, "$opt_y" );
   my $outParkFileName = "WFparkItems.mif";
   printOneWayfinderMifHeader( $outParkFileName, "$opt_y" );
   
   my $nbrPark = 0;
   my $nbrRiverbankPark = 0;
   my $nbrWater = 0;
   my $nbrForest = 0;
   
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
   my %usedids = ();
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() >= 0) {
         $nbrItemsInFile += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         if ( (length($midIdCol) > 0) and
              (length($midRow[$midIdCol]) > 0) ) {
            $midId = $midRow[ $midIdCol ];
            $midId = int $midId;
         }
         if ( defined($usedids{"$midId"}) ) {
            errprint "Duplicated id: $midId"; 
            die "exit";
         } else {
            $usedids{"$midId"} = 1;
         }
         
         my $itemName = "";
         if ( $useName and length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1 ) {
            $nbrItemsWithName += 1;
            $allNames = "$itemName:officialName:$m_langStr";
         }
         
         my $type = "";
         if ( length($typeCol) > 0 ) {
            $type = $midRow[ $typeCol ];
         }

         # print WF mid file and define WF mif file name
         my $wanted = 1;
         my $outMifFileName = "_invalidMifFileName";
         if ( $type eq "park" ) {
            $nbrPark += 1;
            $outMifFileName = $outParkFileName;
            print WF_PARK_FILE "$midId,\"$itemName\",\"$allNames\"" .
                               ",\"cityPark\"\n";
         }
         elsif ( $type eq "riverbank" ) {
            $nbrRiverbankPark += 1;
            $outMifFileName = $outParkFileName;
            print WF_PARK_FILE "$midId,\"$itemName\",\"$allNames\"" .
                               ",\"cityPark\"\n";
         }
         elsif ( $type eq "water" ) {
            $nbrWater += 1;
            $outMifFileName = $outWaterFileName;
            print WF_WATER_FILE "$midId,\"$itemName\",\"$allNames\"" .
                                ",\"river\"\n";
         }
         elsif ( $type eq "forest" ) {
            $nbrForest += 1;
            $outMifFileName = $outForestFileName;
            print WF_FOREST_FILE "$midId,\"$itemName\",\"$allNames\"\n";
         }
         else {
            errprint "Not handled naturals type $type";
            die "exit";
         }

         # Read next feature from mif file and write to correct WF mif
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Read totally $nbrItemsInFile items from file, " .
            "$nbrItemsWithName have name.";
   dbgprint "Created nbrForest=$nbrForest nbrPark=$nbrPark " .
            "nbrRiverbankPark=$nbrRiverbankPark nbrWater=$nbrWater";
} # handleNaturalsFile




# This action processes a network mid mif file
sub handleNetworkFile {
   dbgprint "handleNetworkFile";

   my $defaultWFspeed = 50;
   dbgprint "handleNetworkFile $opt_v";
   dbgprint "Default speed limit $defaultWFspeed";
   print "\n";


   # We want the node file (junctions) to extract borderNode attributes
   my $weHaveBorderNodes = 0;
   
   # misc things to calculate
   my @midIdsWithInvalidSpeedVec;
   my %speedValueOfInvalidSpeed;
   my $nbrLeftSettlementDefined = 0;
   my %leftSettlements;
   my $nbrRightSettlementDefined = 0;
   my $nbrFerry = 0;

   # Open Wayfinder ssi + ferry mid files
   open(WF_SSI_FILE, ">WFstreetSegmentItems.mid");
   open(WF_FERRY_FILE, ">WFferryItems.mid");
   # Define Wayfinder ssi + ferry mif files, 
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outSSIFileName = "WFstreetSegmentItems.mif";
   printOneWayfinderMifHeader( $outSSIFileName, "$opt_y" );
   my $outFerryFileName = "WFferryItems.mif";
   printOneWayfinderMifHeader( $outFerryFileName, "$opt_y" );
   # Open Wayfinder restrictions file
   open(WF_RESTR_FILE, ">WFnetwork_restrictions.txt");
   my $nbrRestrictions = 0;
   # Open and write Wayfinder turntable header file
   open(WF_RESTRHEAD_FILE, ">WFturntableheader.txt");
   print WF_RESTRHEAD_FILE "KEY\tNODE_\tARC1_\tARC2_\tIMPEDANCE\tLAT\tLON\n";
   
   
   #statistics
   my %streetTypes = ();
   my %wfRoadClass = ();
   my %wfRoadDispClass = ();
   my $nbrRamps = 0;
   my $nbrRbt = 0;
   my $nbrCtrlAcc = 0;
   
   my $nbrMidItems = 0;
   my $nbrPrintedToWFFile = 0;
   my %usedids = ();
   my $mifFileReadPos = 0;
   my $maxMidID = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 2) {

         $nbrMidItems += 1;

         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

         my $midId;
         my $midStrName;
         my $midStrType;
         my $midOneWay;
         my $midBridge;
         my $midSpeed;

         # No values from supplier, but still used in parsing...
         my $midLeftPostal = ""; my $midRightPostal = "";
         
         # Parse attributes
         if ( ($opt_v eq "OSM_201005") ) {
            $midId = $midRow[0];
            $midStrName = $midRow[1];
            $midStrType = $midRow[3];
            $midOneWay = $midRow[4];
            $midBridge = $midRow[5];
            $midSpeed = $midRow[6];

         }
         else {
            errprint "Implement attribute parsing for version $opt_v";
            die "exit";
         }


         $midId = int $midId;
         if ( !checkMidId($midId, 1) ) {  # parseonly check
            die "What to do with bad midid $midId for $opt_v??";
         }
         checkMidId($midId, 0); # not parseonly, die if check fails
         if ( $midId > $maxMidID ) {
            $maxMidID = $midId;
         }
         if ( defined($usedids{"$midId"}) ) {
            errprint "Duplicated id: $midId"; 
            die "exit";
         } else {
            $usedids{"$midId"} = 1;
         }

         
         
         
         if ($nbrMidItems == 1) {
            print " midId = $midId - max 4294967295, length midId=" .
                  length($midId) . "\n";

            print " midId=$midId " .
                  " midOneWay=$midOneWay midSpeed=$midSpeed" .
                  " midStrName='$midStrName' \n";
         }
         if ( defined $streetTypes{$midStrType} ) {
            $streetTypes{$midStrType} += 1;
         } else {
            $streetTypes{$midStrType} = 1;
         }
         

         # Admin areas
         my $midLeftSettle = "";
         my $midRightSettle = "";
         
         # Build item name
         my $itemName = "";
         if (length($midStrName) > 0) {
            $itemName = "$midStrName";
         }

         # Build allNames from itemName and all road numbers and alt.names
         my $allNames = "";
         if (length($itemName) > 0) {
            $allNames = "$itemName:officialName:$m_langStr";
         }
         

         # Road class (default Wayfinder road class is 4)
         my $WFroadClass = 4;
         if ( length($midStrType) > 0 ) {
            $WFroadClass = getWFroadClassFromStreetType(
                  $midStrType, $midId, $itemName);
         } else {
            print "WARN item $midId with undefined midStrType\n";
            $WFroadClass = 4;
            die "exit";
         }
         if ( defined $wfRoadClass{$WFroadClass} ) {
            $wfRoadClass{$WFroadClass} += 1;
         } else {
            $wfRoadClass{$WFroadClass} = 1;
         }
         # Road display class
         my $WFroadDispClass = getWFroadDisplayClassFromStreetType(
                  $midStrType, $midId, $itemName);
         if ( defined $wfRoadDispClass{$WFroadDispClass} ) {
            $wfRoadDispClass{$WFroadDispClass} += 1;
         } else {
            $wfRoadDispClass{$WFroadDispClass} = 1;
         }

         # Entry restrictions (default is 0=noRestrictions)
         my $WFposEntryRestr = 0;
         my $WFnegEntryRestr = 0;
         if ( length($midOneWay) > 0 ) {
            $midOneWay = int $midOneWay;
            if ($midOneWay == 0) {
               # ok
            }
            elsif ($midOneWay == 1) { # Oneway in postive direction
               $WFnegEntryRestr = 3;
            }
            else {
               die "unknown oneway value $midOneWay for $midId";
            }
         }
         elsif ( streetTypeNoThroughfare( $midStrType) ) {
            $WFposEntryRestr = 1;
            $WFnegEntryRestr = 1;
         }

         # Number lanes
         my $WFnumberLanes = "";
         # Road width
         my $WFwidth = "";
         # Max height
         my $WFheight = "";
         # Max weight
         my $WFweight = "";
         
         # House numbers
         # Use even nbrs on the left side and odd on the right side
         my $WFleftStart = 0;
         my $WFleftEnd = 0;
         my $WFrightStart = 0;
         my $WFrightEnd = 0;
         
         # Level node0 and node1
         my $WFlevel0 = 0;
         my $WFlevel1 = 0;

         # Toll road
         my $WFtoll = "N";
         
         #
         my $isStreetSegment = "true";
         my $WFcontrolledAccess = "N";
         my $WFdivided = "N";
         my $WFmulti = "N";
         my $WFpaved = "Y";
         my $WFramp = "N";
         my $WFroundabout = "N";
         my $WFroundaboutish = "N";
         if ( streetTypeIsRamp( $midStrType) ) {
            $nbrRamps += 1;
            $WFramp = "Y";
         }
         if ( streetTypeIsRbt( $midStrType) ) {
            $nbrRbt += 1;
            $WFroundabout = "Y";
         }
         if ( streetTypeIsControlledAccess($midStrType) ) {
            $nbrCtrlAcc += 1;
            $WFcontrolledAccess = "Y";
         }
         
         # Speed limit (use default Wayfinder speed)
         my $WFposSpeed = $defaultWFspeed;
         my $WFnegSpeed = $defaultWFspeed;
         if ( (length($midSpeed) > 0) and ($midSpeed > 0) ) {
            $WFposSpeed = $midSpeed;
            $WFnegSpeed = $midSpeed;
         } else {
            $speedValueOfInvalidSpeed { $midId } = $midSpeed;
         }
         # Use roadClass to set speed instead
         if ( ($opt_v eq "OSM_201005") ) {
            # According to countrySpeedMatrix table in NationalProperties
            # {110,  90,  70, 50}, // most countries
            # Normal: rc 0=0, rc1=1, rc2=2, rc3=3, rc4=3
            my $tmpSpeed = $defaultWFspeed;
            if ( $WFroadClass == 0 ) {
               $tmpSpeed = 110;
            } elsif ( $WFroadClass == 1 ) {
               $tmpSpeed = 90;
            } elsif ( $WFroadClass == 2 ) {
               $tmpSpeed = 70;
            } elsif ( $WFroadClass == 3 ) {
               $tmpSpeed = 50;
            } elsif ( $WFroadClass == 4 ) {
               $tmpSpeed = 50;
            }
            # GDF: if roundabout max 50/30
            if ( $WFroundabout eq "Y" ) {
               if ( $tmpSpeed >= 50 ) {
                  $tmpSpeed = 50;
               } else {
                  $tmpSpeed = 30;
               }
            }
            # GDF: if ramp 76% of speed
            if ( $WFramp eq "Y" ) {
               $tmpSpeed = int( $tmpSpeed * 0.76 + 0.5 );
            }
            # MINE: if rc3or4 and midspeed is less than the tmpspeed, keep it
            if ( (($WFroadClass == 3) or ($WFroadClass == 4)) and
                 ($midSpeed != 0) and
                 ($midSpeed < $tmpSpeed) ) {
               $tmpSpeed = $midSpeed;
            }
            $WFposSpeed = $tmpSpeed;
            $WFnegSpeed = $tmpSpeed;
            #print "midId $midId rc=$WFroadClass ramp=$WFramp rbt=$WFroundabout midSpeed=$midSpeed WFspeed=$WFposSpeed\n";
         } else {
            die "Use default GDF speed for version $opt_v??";
         }

         # Zip codes
         # no zip codes in the data
   
         # Check left|right settlement.. from midLeftSettle/midRightSettle
         # If settlement is 0, use some kind of invalid mark. ?
         my $WFleftSettle = "";
         my $WFrightSettle = "";
         my $WFsettleOrder = "";      # Order 9 = city parts..

         # borderNodes...
         my $node0Border = "";
         my $node1Border = "";
         if ( $weHaveBorderNodes ) {
         }


         # Separate ssi from ferry, both mid and mif file
         my $wanted = 1;
         $wanted = streetTypeIsWanted( $midStrType, $midId);
         if ( $wanted ) {
            $nbrPrintedToWFFile += 1;
            if ( $isStreetSegment eq "true" ) {
               print WF_SSI_FILE "$midId,\"$itemName\",\"$allNames\"" .
                                 ",$WFroadClass" .
                                 ",$WFposSpeed,$WFnegSpeed" .
                                 ",$WFposEntryRestr,$WFnegEntryRestr" .
                                 ",$WFnumberLanes,$WFwidth,$WFheight,$WFweight" .
                                 ",$WFleftStart,$WFleftEnd" .
                                 ",$WFrightStart,$WFrightEnd" .
                                 ",\"$WFpaved\"" .
                                 ",$WFlevel0,$WFlevel1" .
                                 ",\"$WFroundabout\",\"$WFramp\"" .
                                 ",\"$WFdivided\",\"$WFmulti\"" .
                                 ",\"$WFtoll\",\"$WFcontrolledAccess\"" .
                                 ",\"$WFroundaboutish\"" .
                                 ",\"$midLeftPostal\",\"$midRightPostal\"" .
                                 ",$WFleftSettle,$WFrightSettle" .
                                 ",$WFsettleOrder" .
                                 ",\"$node0Border\",\"$node1Border\"" .
                                 ",$WFroadDispClass" .
                                 "\n";
            } else {
               print WF_FERRY_FILE "$midId,\"$itemName\",\"$allNames\"" .
                                 ",$WFroadClass" .
                                 ",$WFposSpeed,$WFnegSpeed" .
                                 ",$WFposEntryRestr,$WFnegEntryRestr" .
                                 ",$WFlevel0,$WFlevel1" .
                                 ",\"$WFtoll\"" .
                                 "\n";
               # I think I can assume that ferries are not part of the borderNodes
            }
         }

         # Read next Pline/Line from mif file and write to correct WF mif
         my $outFileName = "_invalidMifFileName";
         if ( $isStreetSegment eq "true" ) {
             $outFileName = $outSSIFileName;
         } else {
             $outFileName = $outFerryFileName;
         }
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outFileName, $wanted, $midId );

         
         # Set no access for vehicles
         # (ok for pedestrians (and bicyclists))
         if ( streetTypeWantedInTurnTable($midStrType) ) {
            print WF_RESTR_FILE "$.\t-1\t-1\t$midId\t-1\t0\t0\n";
            $nbrRestrictions += 1;
         }

      }
   }

   
   # Result
   print "\n=======================================================\n";
   dbgprint "Network file had $nbrMidItems mid items";
   if ( scalar (keys %m_unknownStreetTypes) > 0 ) {
      errprint "We have unknown street types - edit parse script!";
      foreach my $r ( sort (keys (%m_unknownStreetTypes)) ) {
         print "  street type \"$r\" has "
               . $m_unknownStreetTypes{ $r } . " segments\n";
      }
      errprint "add these street types in the initStreetTypesHash function";
      die "exit";
   }
   
   dbgprint " - wrote $nbrPrintedToWFFile items to WF file";
   if ( scalar keys (%speedValueOfInvalidSpeed) > 0 ) {
      print "There were " .  scalar( keys (%speedValueOfInvalidSpeed) ) . 
            " segments with invalid speed\n";
      #foreach my $r ( sort (keys (%speedValueOfInvalidSpeed)) ) {
      #   print " midId $r = \"" . $speedValueOfInvalidSpeed{ $r } . "\"\n";
      #}
   }

   print "There are $nbrLeftSettlementDefined roads in " .
         keys(%leftSettlements) . " different left settlements.\n";
   print "There are $nbrRightSettlementDefined roads with right settlement.\n";
   
   print "Make sure that there are no mid replace chars \"$midRplChar\" " .
         "in the resulting ssi mid file.\n";

   if ( $nbrRestrictions > 0 ) {
      print "Wrote $nbrRestrictions restrictions to turn table.\n";
   }

   if ( $nbrFerry > 0 ) {
      print "Number ferry=$nbrFerry\n";
   }
   

   print "\n=====================================================\n";
   print "Number segments with street type:\n";
   foreach my $r ( sort (keys (%streetTypes)) ) {
      print " strType $r = " . $streetTypes{ $r } . "\n";
   }
   print "Number segments with wf road class:\n";
   foreach my $r ( sort (keys (%wfRoadClass)) ) {
      print " rc $r = " . $wfRoadClass{ $r } . "\n";
   }
   print "Number segments with wf road display class:\n";
   foreach my $r ( sort (keys (%wfRoadDispClass)) ) {
      print " rc $r = " . $wfRoadDispClass{ $r } . "\n";
   }
   print "Number segments that are ramps = $nbrRamps\n";
   print "Number segments that are roundabouts = $nbrRbt\n";
   print "Number segments with ctrl acc = $nbrCtrlAcc\n";
   
} # handleNetworkFile





# Handle City centre POI file, write to CPIF
sub handleCCFile {
   dbgprint "handleCCFile";

   # Define attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   my $cityNameCol = "";
   my $strNameCol = "";
   my $ccTypeCol = "";
   my $ccPopulationCol = "";
   if ( ($opt_v eq "OSM_201005") ) {
      $midIdCol = 0; # cannot use midId because of duplicated ids
      $nameCol = 1;
      $ccTypeCol = 2;
      $ccPopulationCol = 3;
   } else {
      die "check attribute column order for $opt_v";
   }
   
   handleGenPoiFile( "cc",
                     11,            # given poi type = city centre
                     $midIdCol,
                     $nameCol,
                     $typeCol,
                     $strNameCol,
                     $cityNameCol,
                     $ccTypeCol, # place type
                     $ccPopulationCol
                     );
}

#
# Handle POI file, write to CPIF
sub handlePoiFile {
   dbgprint "handlePoiFile";

   # Define attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   my $strNameCol = "";
   my $cityNameCol = "";
   my $defaultPoiType = "";
   
   if ( ($opt_v eq "OSM_201005") ) {
      $midIdCol = 0; # Osm_id
      $nameCol = 2;  # Name
      $typeCol = 3;  # Type
   } else {
      die "Attribute column order for $opt_v?";
   }
  
   handleGenPoiFile( "poi",
                     $defaultPoiType,     # no given POI type
                     $midIdCol,
                     $nameCol,
                     $typeCol,
                     $strNameCol,
                     $cityNameCol,
                     "",
                     ""
                     );
}

#
# Genral function that handles misc POI files,
# write to CPIF (use POI object)
sub handleGenPoiFile {
   my $fileType = $_[0];
   my $givenWFpoiTypeID = $_[1];  # given POI type
   my $midIdCol = $_[2];
   my $nameCol = $_[3];
   my $typeCol = $_[4];
   my $strNameCol = $_[5];
   my $cityNameCol = $_[6];
   my $ccTypeCol = $_[7]; # place type
   my $ccPopCol = $_[8];
   dbgprint "handleGenPoiFile: $fileType";

   
   # Define out file name, open to empty any old file, then it is
   # written with append.
   my $wfFileName = "";
   if ( ($opt_v eq "OSM_201005") ) {
      $wfFileName = "WFcpif_${fileType}_${opt_c}.txt";
   } else {
      die "File name for poi file for $opt_v?";
   }
   open(WF_CPIF_FILE, ">$wfFileName");
   print WF_CPIF_FILE "";
   
   # We need a database connection to handle POIObject
   connectToDBH();

   
   # Read the mif file and store poi coordinates in coordinate hashes
   my ($latitudes, $longitudes) = 
      readMifPointFile( $SECOND_FILE_NAME, $m_coordOrder );
   
   my %midIds = ();

   # Read the mid file
   my $nbrPois = 0;
   my $nbrUnwantedPOIs = 0;
   my $nbrPOIsToCPIF = 0;
   my %poisWithWFType = ();
   my %poisWithWFCat = ();
   my %ccWithDispClass = ();
   my %poisWithSupplierType = ();
   my $nbrColumns = 0;
   my $nbrNoNamePOIsFromFile = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 2) {
      
         $nbrPois += 1;

         # Split the midRow, handling $midSepChar within strings
         # also removing "-chars from string values
         my @poiRecord = 
               splitOneMidRow( $midSepChar, $midRplChar, $_,
                               "true" ); # quotationMarkWithinStrings
         
         if ( $. == 1 ) {
            $nbrColumns = scalar(@poiRecord);
         } else {
            if ( scalar(@poiRecord) != $nbrColumns ) {
               errprint "Problems with number of columns for " .
                        "mid row $., prev=$nbrColumns this=" .
                        scalar(@poiRecord);
               die "exit";
            }
         }
         
         # Get midId, and remove decimals 80512.0000000000000 -> 80512
         my $midId = $.;
         if (defined $midIdCol and (length $midIdCol > 0) ) {
            $midId = $poiRecord[$midIdCol];
         }
         $midId = int $midId;
         $midId = "$fileType$midId";
         if ( defined $midIds{$midId} ) {
            die "more than one POI has midId $midId";
         } else {
            $midIds{$midId} = 1;
         }

         # POI official name
         my $poiName = $poiRecord[$nameCol];
         if (length $poiName == 0) {
            $nbrNoNamePOIsFromFile +=1;
         }
         
         # Get wfPoiTypeId for this poi. Either it is given with
         # inparam or needs to be translated from supplier category
         my $wfPoiTypeId = 95; # invalid
         if ( defined $givenWFpoiTypeID and
              (length($givenWFpoiTypeID) > 0) ) {
            $wfPoiTypeId = $givenWFpoiTypeID;
         } else {
            $wfPoiTypeId  = getWayfinderPoiTypeId(
                  $poiRecord[$typeCol], $midId, $poiName);
            if ( defined $poisWithSupplierType{$poiRecord[$typeCol]} ) {
               $poisWithSupplierType{$poiRecord[$typeCol]} += 1;
            } else {
               $poisWithSupplierType{$poiRecord[$typeCol]} = 1;
            }
         }
         # Get city centre display class for city centres
         my $ccDispClass = 12; # default
         if ( $wfPoiTypeId == 11 ) {
            my $pop = 0;
            my $ccType = ""; # placeType
            my $poiName = "";
            if ( defined $ccPopCol and (length $ccPopCol > 0) ) {
               $pop = $poiRecord[$ccPopCol];
            }
            if ( defined $ccTypeCol and (length $ccTypeCol > 0) ) {
               $ccType = $poiRecord[$ccTypeCol];
            }
            if ( defined $nameCol and (length $nameCol > 0) ) {
               $poiName = $poiRecord[$nameCol];
            }
            $ccDispClass = getCCdispClassFromPlaceTypeAndPopulation(
               $pop, $ccType, $midId, $poiName );

            if ( $ccDispClass < 0 ) {
               # not wanted, or special - change poi type or set to notWanted
               if ( $ccDispClass == -99 ) {
                  $wfPoiTypeId = 94; # noType
               } else {
                  $wfPoiTypeId = $ccDispClass;
               }
            }
            if ( defined $ccWithDispClass{$ccDispClass} ) {
               $ccWithDispClass{$ccDispClass} += 1;
            } else {
               $ccWithDispClass{$ccDispClass} = 1;
            }
         }
         # statistics for poi type, may have changed when calculating
         # city centre display class
         if ( defined $poisWithWFType{$wfPoiTypeId} ) {
            $poisWithWFType{$wfPoiTypeId} += 1;
         } else {
            $poisWithWFType{$wfPoiTypeId} = 1;
         }


         # Only work with POI if poi type is not -1 (unwanted pois..)
         if ( $wfPoiTypeId < 0 ) {
            $nbrUnwantedPOIs += 1;
         } 
         else {
            
            # Get poi coordinate from mif file hash, and convert to mc2
            my $lat; my $lon; 
            my $normalWGS84 = "";
            if ( index(uc $opt_y, "WGS84") > -1 ) {
               # normal wgs84
               $normalWGS84 = 1;
            } else {
               die "Coordsys for $opt_v???";
            }
            if ( $normalWGS84 ) {
               ($lat, $lon) = 
                  convertFromWgs84ToMc2(  $latitudes->{ $nbrPois },
                                          $longitudes->{ $nbrPois } );
            }
            if ( !defined($lat) or !defined($lon) ) {
               die "No coordinate defined for poi midId $midId - exit";
            }

            
            # Create POI object and fill with info
            my $poiObject = POIObject->newObject();
            $poiObject->setDBH($m_dbh);

            # POIMain
            $poiObject->setCountry( $m_countryID );
            $poiObject->setSourceReference($midId);
            $poiObject->setSymbolCoordinate($lat, $lon);

            # POITypes
            $poiObject->addPOIType( $wfPoiTypeId );

            # POICategories
            if ( defined $typeCol and length($typeCol) > 0 ) {
               # uppercase type
               my $ucType = uc $poiRecord[$typeCol];
               my $wfPoiCat = $types2wfcat{$ucType};
               if ( defined $wfPoiCat ) {
                  $poiObject->addCategory( $wfPoiCat );
                  if ( defined $poisWithWFCat{$wfPoiCat} ) {
                     $poisWithWFCat{$wfPoiCat} += 1;
                  } else {
                     $poisWithWFCat{$wfPoiCat} = 1;
                  }
               }
            }

            # POINames
            # official name
            $poiObject->addName( $poiName, 0, $m_dbLangId );
            
            # POIInfo
            #  0 | Vis. address
            if ( defined $strNameCol and (length($strNameCol) > 0) and
                 $poiRecord[$strNameCol] ) {
               $poiObject->addPOIInfo( 0, 31, $poiRecord[$strNameCol] ); }
            #  5 | Vis. zip area (city name)
            if ( defined $cityNameCol and (length($cityNameCol) > 0) and
                 $poiRecord[$cityNameCol] ) {
               $poiObject->addPOIInfo( 5, 31, $poiRecord[$cityNameCol] ); }

            # cc display class, set the one calculated above
            if ( $wfPoiTypeId == 11 ) {
               $poiObject->addPOIInfo( 47, 31, $ccDispClass );
            }


            # Write this POIObject to CPIF
            if ( $poiObject->writeCPIF($wfFileName) ) {
               $nbrPOIsToCPIF += 1;
            } else {
               print " POI object not written to CPIF midId $midId\n";
            }

         }
      }
   }


   print "\n=======================================================\n";
   dbgprint "Read $nbrPois pois from mid file";
   dbgprint " of which $nbrNoNamePOIsFromFile have no name";
   if ( scalar (keys %m_unknownPOIcategories) > 0 ) {
      errprint "We have unknown poi categories - edit parse script!";
      foreach my $r ( sort (keys (%m_unknownPOIcategories)) ) {
         print "  poi category \"$r\" has "
               . $m_unknownPOIcategories{ $r } . " pois\n";
      }
      errprint "add these poi categories in the initPOITypesHash function";
      die "exit";
   }
   if ( scalar (keys %m_unknownPlaceTypes) > 0 ) {
      my $anyHasMoreThan10 = 0;
      errprint "We have unknown place types " .
               "- edit parse script to include them";
      foreach my $r ( sort (keys (%m_unknownPlaceTypes)) ) {
         print "  place type \"$r\" has "
               . $m_unknownPlaceTypes{ $r } . " pois\n";
         if ( $m_unknownPlaceTypes{ $r } > 10 ) {
            $anyHasMoreThan10 = 1;
         }
      }
      errprint "add these place types in the " .
               "initPlaceCCdisplayClassHash function";
      if ( $anyHasMoreThan10 ) {
         die "exit";
      }
      print "\n";
   }
   
   print "Not using $nbrUnwantedPOIs unwanted pois\n" .
         "Wrote $nbrPOIsToCPIF pois to CPIF $wfFileName\n";

   print "\nNumber of POIs for WF poi type:\n";
   foreach my $t ( sort {$a <=> $b} (keys (%poisWithWFType)) ) {
      my $typeName="notWanted";
      if ( $t >= 0 ) {
         $typeName = getPOITypeNameFromTypeID($m_dbh, $t);
      }
      print " pt $t:  " . $poisWithWFType{$t} . " ($typeName)\n";
   }
   if ( scalar keys (%poisWithSupplierType) > 0 ) {
      print "\nNumber of POIs for supplier poi type:\n";
      foreach my $t ( sort (keys (%poisWithSupplierType)) ) {
         print " $t:  " . $poisWithSupplierType{$t} . "\n";
      }
   }
   if ( scalar keys (%ccWithDispClass) > 0 ) {
      print "\nNumber of CCs for WF display class:\n";
      foreach my $t ( sort {$a <=> $b} (keys (%ccWithDispClass)) ) {
         print " displclass $t:  " . $ccWithDispClass{$t} . "\n";
      }
   }
   if ( scalar keys (%poisWithWFCat) > 0 ) {
      print "\nNumber of POIs for WF category " .
            "(cats not set from poi type):\n";
      foreach my $t ( sort {$a <=> $b} (keys (%poisWithWFCat)) ) {
         print " cat $t:  " . $poisWithWFCat{$t} . " (" . 
               getPOICatDescriptFromPOICatID($m_dbh, $t) . ")\n";
      }
   }

   disconnectFromDBH();

} # handleGenPoiFile


# Get Wayfinder city cente display class from the population
sub getCCdispClassFromPlaceTypeAndPopulation {
   my $population = $_[0];
   my $placeType = $_[1];
   my $midId = $_[2];
   my $poiName = $_[3];

   my $displayClass = 12;

   # make type uppercase before comparing
   $placeType = uc $placeType;

   # get display class from type attribute in place file
   $displayClass = getCCdispClassFromPlaceType(
      $placeType, $midId, $poiName );


   # Adjust the display classes using the population attribute
   # for cities and towns
   if ( $population > 0 ) {
      if ( ($placeType eq "CITY") or
           ($placeType eq "TOWN") ) {
         my $tmpDC = getCCdispClassFromPopulation(
               $population, $midId, $poiName );
         if ( $tmpDC < $displayClass ) {
            #print " changing placeType cc dp $displayClass" .
            #      " to $tmpDC for $poiName\n";
            $displayClass = $tmpDC;
         }
      }
   }

   return $displayClass;

} # getCCdispClassFromPlaceTypeAndPopulation

sub getCCdispClassFromPopulation {
   my $population = $_[0];
   my $midId = $_[1];
   my $poiName = $_[2];

   my $displayClass = 12;
   if ( $population > 1000000 ) { # 1 milion
      $displayClass = 4;
   }
   elsif ( $population > 500000 ) {
      $displayClass = 5;
   }
   elsif ( $population > 100000 ) {
      $displayClass = 7;
   }
   elsif ( $population > 50000 ) {
      $displayClass = 8;
   }
   elsif ( $population > 10000 ) {
      $displayClass = 10;
   }

   return $displayClass;

#   # Display class according to Tele Atlas Europe rules
#   # for the main settlement in a order8
#   if ( $capital ) {
#      $displayClass = 2;
#   }
#   elsif ( $population > 1000000 ) { # 1 milion
#      $displayClass = 4;
#   }
#   elsif ( $population > 500000 ) {
#      $displayClass = 5;
#   }
#   elsif ( $population > 100000 ) {
#      $displayClass = 7;
#   }
#   elsif ( $population > 50000 ) {
#      $displayClass = 8;
#   }
#   elsif ( $population > 10000 ) {
#      $displayClass = 10;
#   }
#   elsif ( $population > 0 ) {
#      $displayClass = 12;
#   }

#   # Tele Atlas Canada/USA rules
#   # for the main settlement in a order8
#   if ( $population > 5000000 ) { # 5 milj
#      $displayClass = 2; }
#   elsif ( $population > 2000000 ) {
#      $displayClass = 3; }
#   elsif ( $population > 1000000 ) {
#      $displayClass = 4; }
#   elsif ( $population > 500000 ) { # 500 thousand
#      $displayClass = 5; }
#   elsif ( $population > 200000 ) {
#      $displayClass = 6; }
#   elsif ( $population > 100000 ) {
#      $displayClass = 7; }
#   elsif ( $population > 50000 ) {  # 50 thousand
#      $displayClass = 8; }
#   elsif ( $population > 20000 ) {
#      $displayClass = 9; }
#   elsif ( $population > 10000 ) {
#      $displayClass = 10; }
#   elsif ( $population > 5000 ) { # 5 thousand
#      $displayClass = 11; }
#   else {
#      $displayClass = 12;
#   }

} # getCCdispClassFromPopulation


# Get a rough city centre display class from the type attribute in place file
# Default return value is 12
sub getCCdispClassFromPlaceType {
   my $placeType = $_[0];
   my $midId = $_[1];
   my $poiName = $_[2];

   # make type uppercase before comparing
   $placeType = uc $placeType;

   my $displayClass = 12;

   if ( !scalar %m_ccDisplayClasses ) {
      %m_ccDisplayClasses = initPlaceCCdisplayClassHash();
   }

   if ( length $placeType == 0 ) {
      print "WARN no place type (0-length string) for poi $midId\n";
      $displayClass = 12;
   }
   elsif ( defined( $m_ccDisplayClasses{$placeType} ) ) {
      $displayClass = $m_ccDisplayClasses{$placeType};
   }
   else {
      # Don't die here,
      # Count the number of pois with each unknown cc type
      # and summarize when all pois in the file are looped
      
      if ( defined $m_unknownPlaceTypes{$placeType} ) {
         $m_unknownPlaceTypes{$placeType} += 1;
      } else {
         $m_unknownPlaceTypes{$placeType} = 1;
         print "unknown place type $placeType for poi $poiName\n";
      }

      # unwanted place
      $displayClass = -1;
   }

   # return
   return ($displayClass);
} # getCCdispClassFromPlaceType

sub initPlaceCCdisplayClassHash {
   my %types = ();

   # Note that the cc type is made upper-case before comparing
   # Set types not wanted to -1
   # Set types wanted as normal poi in the maps to -99
   # (there are lots of different kind of places in the OSM places file)

   $types{"CITY"} = 8;
   $types{"TOWN"} = 10;
   $types{"VILLAGE"} = 12;
   $types{"HAMLET"} = 12;
   $types{"SUBURB"} = 12;
   $types{"AIRPORT"} = 12;
   
   $types{"LOCALITY"} = -99;
   $types{"FARM"} = -99;
   
   $types{"COUNTRY"} = -1;
   $types{"COUNTY"} = -1;
   $types{"MUNICIPALITY"} = -1;
   $types{"REGION"} = -1;
   $types{"HAMLET_HISTORICA"} = -1;
   $types{"ISLAND"} = -1;
   $types{"ISLET"} = -1;
   $types{"FJORD"} = -1;
   $types{"CAPE"} = -1;
   $types{"SEA"} = -1;
   $types{"LAKE"} = -1;
   $types{"PORT"} = -1;
   $types{"HARBOUR"} = -1;
   
   return %types;
} # initPlaceCCdisplayClassHash


# Get Wayfinder poi type id from categories..
# Default return value is 95 = invalid POI type.
# If the returned value is -1 or less, the poi should not be included
# into Wayfinder maps.
sub getWayfinderPoiTypeId {
   # get Wayfinder WASP POI type ids from the supplier poi category
   my $poiCategory = $_[0];
   my $midId = $_[1];
   my $poiName = $_[2];

   # make poi category uppercase before comparing
   $poiCategory = uc $poiCategory;

   my $wfPoiTypeId = 95;

   # Get Wayfinder poi type from the poiTypes hash
   if ( !scalar %m_poiTypes ) {
      %m_poiTypes = initPOITypesHash();
   }

   if ( length $poiCategory == 0 ) {
      print "WARN no poi type (0-length string) for poi $midId\n";
      $wfPoiTypeId = 94;
   }
   elsif ( defined( $m_poiTypes{$poiCategory} ) ) {
      $wfPoiTypeId = $m_poiTypes{$poiCategory};

   }
   else {
      # Don't die here,
      # Count the number of pois with each unknown poi category
      # and summarize when all pois in the poi file are looped
      # Then die...
      #die "getWayfinderPoiTypeId: unknow poi type " .
      #    "'$poiCategory' for poi $midId";
      
      if ( defined $m_unknownPOIcategories{$poiCategory} ) {
         $m_unknownPOIcategories{$poiCategory} += 1;
      } else {
         $m_unknownPOIcategories{$poiCategory} = 1;
         print "unknown poi category $poiCategory\n";
      }
      $wfPoiTypeId = -1;
   }
   
   # Calculate displayClass for city centre POIs
   if ( $wfPoiTypeId == 11 ) {
      die "Decide rules for calculating displayclass for cc POI " .
          "with id $midId";
   }
   
   # return
   return ($wfPoiTypeId);
} # getWayfinderPoiTypeId


sub initPOITypesHash {
   my %types = ();

   # Translate to Wayfinder POI type ID, as defined in 
   # ItemTypes::pointOfInterest_t
   # and
   # POI database POITypeTypes table
   #
   # POI type 95 = invalidPOItype = marker for error type
   # POI type -1 = marker in this script for that we do not want 
   #               the actual POI to be included into the CPIF file
   # 

   # Note that the category is made upper-case before comparing

   $types{"AIRPORT"} = 1;
   $types{"ATM"} = 3;
   $types{"BANK"} = 5;
   $types{"BESöKSPARKERING"} = 30;  # openParkingArea
   $types{"BUS_STATION"} = 7;
   $types{"BUS_STOP"} = 103;
   $types{"CAFE"} = 100;
   $types{"CAMP_SITE"} = 65;        # campingGround
   $types{"CARAVAN_SITE"} = 65;     # campingGround
   $types{"CARAVAN_SITE;CAM"} = 65; # campingGround
   $types{"CAR_RENTAL"} = 38;
   $types{"CASINO"} = 9;
   $types{"CHINA RESTAURANT"} = 40;
   $types{"CHURCH"} = 96;
   $types{"CINEMA"} = 10;
   $types{"DOCTOR"} = 71;
   $types{"DOCTORS"} = 71;
   $types{"EMBASSY"} = 73;
   $types{"FAST_FOOD"} = 40;
   $types{"FERRY"} = 17;
   $types{"FERRY_TERMINAL"} = 17;
   $types{"FUEL"} = 33;    # petrolStation
   $types{"HOSPITAL"} = 22;
   $types{"HOTEL"} = 23;
   $types{"LIBRARY"} = 25;
   $types{"MUSEUM"} = 28;
   $types{"PARKING"} = 30;    # openParkingArea
   $types{"PHARMACY"} = 81;
   $types{"PIZZERIA"} = 40;
   $types{"PLACE_OF_WORSHIP"} = 82;
   $types{"POLICE"} = 34;
   $types{"POST_OFFICE"} = 52;
   $types{"RESTAURANT"} = 40;
   $types{"REST_AREA"} = 39;
   $types{"SCHOOL"} = 41;
   $types{"STATION"} = 36;    # railwayStation
   $types{"SUBWAY_ENTRANCE"} = 99;
   $types{"SWIMMING"} = 87;         # swimmingPool
   $types{"SWIMMING_POOL"} = 87;    # swimmingPool
   $types{"TAXI"} = 104;
   $types{"THEATRE"} = 46;
   $types{"VETERINARY"} = 89; # vetrinarian
   $types{"WIFI"} = 93; # wlan
   $types{"WI-FI HOT SPOT"} = 93;   # wlan
   $types{"ZOO"} = 92;

   # Not wanted in Wayfinder maps, at least it does not fit any POI type 
   # defined in Wayfinder maps
   $types{"CROSSING"} = -2;
   $types{"LEVEL_CROSSING"} = -2;
   $types{"OLD_LEVEL_CROSSI"} = -2;
   $types{"MINI_ROUNDABOUT"} = -2;
   $types{"MOTORWAY_JUNCTIO"} = -2;
   $types{"ROUNDABOUT"} = -2;
   $types{"TRAFFIC_SIGNALS"} = -2;
   $types{"TURNING_CIRCLE"} = -2;


   # TODO: 
   # Set valid POI type ID for the POIs you want to have in the CPIF
   # Also set the WF POI category in the types2wfcat hash if appropriate

   $types{"ABANDONED_CARGO_"} = -1;
   $types{"ABANDONED_FUEL"} = -1;
   $types{"ABANDONED_HALT"} = -1;
   $types{"ABANDONED_STATIO"} = -1;
   $types{"ALPINE_HUT"} = -1;
   $types{"ARCHAEOLOGICAL_S"} = -1;
   $types{"ARTS_CENTRE"} = -1;
   $types{"ARTWORK"} = -1;
   $types{"ATTRACTION"} = -1;
   $types{"B"} = -1;
   $types{"BADING_PLACE"} = -1;
   $types{"BAKERY"} = -1;
   $types{"BANVAKTSTUGA"} = -1;
   $types{"BAR"} = -1;
   $types{"BARBEQUE"} = -1;
   $types{"BARBERSHOP"} = -1;
   $types{"BARRIER"} = -1;
   $types{"BASKET"} = -1;
   $types{"BATH"} = -1;
   $types{"BATH_PLACE"} = -1;
   $types{"BATHROOM"} = -1;
   $types{"BATTLEFIELD"} = -1;
   $types{"BBQ"} = -1;
   $types{"BE"} = -1;
   $types{"BEACON"} = -1;
   $types{"BED_AND_BREAKFAS"} = -1;
   $types{"BENCH"} = -1;
   $types{"BICYCLE AIR"} = -1;
   $types{"BICYCLE_AIR"} = -1;
   $types{"BICYCLE LET"} = -1;
   $types{"BICYCLE_PARKING"} = -1;
   $types{"BICYCLE_PUMP"} = -1;
   $types{"BICYCLE_RENTAL"} = -1;
   $types{"BIERGARTEN"} = -1;
   $types{"BILLBOARD"} = -1;
   $types{"BILPROVNINGEN"} = -1;
   $types{"BIRD WATCH"} = -1;
   $types{"BIRD WATCH TOWER"} = -1;
   $types{"BOAT_HIRE"} = -1;
   $types{"BOLLARD"} = -1;
   $types{"BOOKS"} = -1;
   $types{"BøRNEHAVE"} = -1;
   $types{"BOUNDARY_STONE"} = -1;
   $types{"BRIDGE"} = -1;
   $types{"BTS"} = -1;
   $types{"BUFFER_STOP"} = -1;
   $types{"BUILDING"} = -1;
   $types{"BULLETIN_BOARD"} = -1;
   $types{"BUREAU_DE_CHANGE"} = -1;
   $types{"BUS_SLUICE"} = -1;
   $types{"BUSSPARKERING M."} = -1;
   $types{"CABIN"} = -1;
   $types{"CAMPANILE"} = -1;
   $types{"CAMPER_WASTEWATE"} = -1;
   $types{"CAR_SHARING"} = -1;
   $types{"CAR_WASH"} = -1;
   $types{"CASTLE"} = -1;
   $types{"CHALET"} = -1;
   $types{"CHILD_CARE"} = -1;
   $types{"CHIMNEY"} = -1;
   $types{"CLINIC"} = -1;
   $types{"CLOCK"} = -1;
   $types{"COAST_GUARD"} = -1;
   $types{"COLLEGE"} = -1;
   $types{"COMMON"} = -1;
   $types{"COMMUNICATIONS_T"} = -1;
   $types{"COMMUNITY_CENTRE"} = -1;
   $types{"CONCERT_HALL"} = -1;
   $types{"CONFERENCE_CENTR"} = -1;
   $types{"CONFERENCE HALL"} = -1;
   $types{"CONSTRUCTION"} = -1;
   $types{"CONSULATE"} = -1;
   $types{"COTTAGE"} = -1;
   $types{"COURTHOUSE"} = -1;
   $types{"CRANE"} = -1;
   $types{"CRAZY CHICKEN"} = -1;
   $types{"CREMATORIUM"} = -1;
   $types{"CUSTOMS"} = -1;
   $types{"CYCLEWAY"} = -1;
   $types{"DENTIST"} = -1;
   $types{"DISUSED_FUEL"} = -1;
   $types{"DISUSED_STATION"} = -1;
   $types{"DOJO"} = -1;
   $types{"DORMITORY"} = -1;
   $types{"DRINKING_WATER"} = -1;
   $types{"DRIVING_SCHOOL"} = -1;
   $types{"ECO FARM AND STO"} = -1;
   $types{"ELEVATOR"} = -1;
   $types{"EMERGENCY_ACCESS"} = -1;
   $types{"EMERGENCY_PHONE"} = -1;
   $types{"END"} = -1;
   $types{"FIRE_HYDRANT"} = -1;
   $types{"FIREPLACE"} = -1;
   $types{"FIRE_STATION"} = -1;
   $types{"FITNESS"} = -1;
   $types{"FITNESS_CENTRE"} = -1;
   $types{"FLAGPOLE"} = -1;
   $types{"FLOWERS"} = -1;
   $types{"FODBOLDT BANE"} = -1;
   $types{"FOLKETS HUS"} = -1;
   $types{"FOLKPARK"} = -1;
   $types{"FOOD_COURT"} = -1;
   $types{"FOOTBALL"} = -1;
   $types{"FOOTWAY"} = -1;
   $types{"FORD"} = -1;
   $types{"FöRENINGSTAVLA"} = -1;
   $types{"FORMER_RESTAURAN"} = -1;
   $types{"FORSAMLINGSHUS"} = -1;
   $types{"FORT"} = -1;
   $types{"FORTIFCATION"} = -1;
   $types{"FORTIFICATION"} = -1;
   $types{"FORTIFICATIONS"} = -1;
   $types{"FOUNTAIN"} = -1;
   $types{"FUEL;CAR_RENTAL;"} = -1;
   $types{"FUEL;CAR_WASH"} = -1;
   $types{"FUGLETåRN"} = -1;
   $types{"FYSIOTERAPI"} = -1;
   $types{"GADSTRUP MOSE"} = -1;
   $types{"GANTRY"} = -1;
   $types{"GATE"} = -1;
   $types{"GIVE_WAY"} = -1;
   $types{"GRAVE YARD"} = -1;
   $types{"GRAVE_YARD"} = -1;
   $types{"GRAVFäLTET"} = -1;
   $types{"GRIT_BIN"} = -1;
   $types{"GUEST_HOUSE"} = -1;
   $types{"GUN_EMPLACEMENT"} = -1;
   $types{"GYM"} = -1;
   $types{"HäLSOCENTRAL"} = -1;
   $types{"HALT"} = -1;
   $types{"HARBOUR"} = -1;
   $types{"HIDE"} = -1;
   $types{"HIGHLAND CATTLE"} = -1;
   $types{"HIGHSCHOOL"} = -1;
   $types{"HISTORIC"} = -1;
   $types{"HJäRTERö"} = -1;
   $types{"HOLIDAY_COTTAGES"} = -1;
   $types{"HOME"} = -1;
   $types{"HOMELESS_SHELTER"} = -1;
   $types{"HOSTEL"} = -1;
   $types{"HOUSE"} = -1;
   $types{"HUNTING_STAND"} = -1;
   $types{"ICON"} = -1;
   $types{"INCLINE"} = -1;
   $types{"INCLINE_STEEP"} = -1;
   $types{"INDUSTRIAL"} = -1;
   $types{"INFORMATION"} = -1;
   $types{"INFOTAFEL"} = -1;
   $types{"INTERNET_CAFE"} = -1;
   $types{"JAZZCLUB"} = -1;
   $types{"JUNCTION"} = -1;
   $types{"KAFFE"} = -1;
   $types{"KIDERGARDEN"} = -1;
   $types{"KINDERGARTEN"} = -1;
   $types{"KIOSK"} = -1;
   $types{"KIROPRAKTOR"} = -1;
   $types{"KLOAKPUMPESTATIO"} = -1;
   $types{"KORT"} = -1;
   $types{"LEAN_TO"} = -1;
   $types{"LIGHTHOUSE"} = -1;
   $types{"LOOKOUT"} = -1;
   $types{"MARKETPLACE"} = -1;
   $types{"MAYPOLE"} = -1;
   $types{"MEETING"} = -1;
   $types{"MEETING_PLACE"} = -1;
   $types{"MEETING_POINT"} = -1;
   $types{"MEMORIAL"} = -1;
   $types{"MINE"} = -1;
   $types{"MINI_GOLF"} = -1;
   $types{"MINI_RAMP"} = -1;
   $types{"MINNESPLATS"} = -1;
   $types{"MONUMENT"} = -1;
   $types{"MOTEL"} = -1;
   $types{"NARROW"} = -1;
   $types{"NIGHTCLUB"} = -1;
   $types{"NURSING_HOME"} = -1;
   $types{"ODDERBANEN"} = -1;
   $types{"OFFICE_HOTEL"} = -1;
   $types{"OK"} = -1;
   $types{"OLD_HALT"} = -1;
   $types{"OLD WATERMILL (Y"} = -1;
   $types{"ÖPPEN FöRSKOLA"} = -1;
   $types{"OTHER"} = -1;
   $types{"PALACE"} = -1;
   $types{"PARK"} = -1;
   $types{"PARKING,TIL FUGL"} = -1;
   $types{"PARKING TO BJöR"} = -1;
   $types{"PARTYVILLAN"} = -1;
   $types{"PASSING_PLACE"} = -1;
   $types{"PATH"} = -1;
   $types{"PAWNBROKER"} = -1;
   $types{"PEAT_BARN"} = -1;
   $types{"PERSONENFäHREN"} = -1;
   $types{"PHONEBOOTH"} = -1;
   $types{"PHYSIOTHERAPIST"} = -1;
   $types{"PICNIC_SITE"} = -1;
   $types{"PIER"} = -1;
   $types{"PLATFORM"} = -1;
   $types{"PLEJEHJEM"} = -1;
   $types{"POST_BOX"} = -1;
   $types{"POWER_BIO"} = -1;
   $types{"PRE"} = -1;
   $types{"PRESCHOOL"} = -1;
   $types{"PRISON"} = -1;
   $types{"PUB"} = -1;
   $types{"PUBLIC BATH"} = -1;
   $types{"PUBLIC_BUILDING"} = -1;
   $types{"PUMPING_STATION"} = -1;
   $types{"RADIO_TOWER"} = -1;
   $types{"RAIL"} = -1;
   $types{"RAILWAY_ENGINE"} = -1;
   $types{"RCN_REF"} = -1;
   $types{"RECREATION"} = -1;
   $types{"RECYCLE"} = -1;
   $types{"RECYCLING"} = -1;
   $types{"REMAINS"} = -1;
   $types{"RENTAL DIY"} = -1;
   $types{"RESERVOIR_COVERE"} = -1;
   $types{"RESIDENTIAL"} = -1;
   $types{"RETIREMENT_HOME"} = -1;
   $types{"RIDING_SCHOOL"} = -1;
   $types{"ROAD"} = -1;
   $types{"RUINS"} = -1;
   $types{"RUINS?"} = -1;
   $types{"RUNE_STONE"} = -1;
   $types{"RUNSTEN"} = -1;
   $types{"SAKHON THAI TAKE"} = -1;
   $types{"SAUNA"} = -1;
   $types{"SCHOOL, KINDERGA"} = -1;
   $types{"SCIENCE_PARK"} = -1;
   $types{"SEAT"} = -1;
   $types{"SERVICE HUS"} = -1;
   $types{"SERVICES"} = -1;
   $types{"SHELTER"} = -1;
   $types{"SHELTER AND TOIL"} = -1;
   $types{"SHIP"} = -1;
   $types{"SHOP"} = -1;
   $types{"SHOPPING"} = -1;
   $types{"SHOWER"} = -1;
   $types{"SICKLA UDDE"} = -1;
   $types{"SIGHTSEEING"} = -1;
   $types{"SIGNAL"} = -1;
   $types{"SIGNPOST"} = -1;
   $types{"SKI_RENTAL"} = -1;
   $types{"SNOWMOBILE_RENTA"} = -1;
   $types{"SöDERKISEN"} = -1;
   $types{"SOLARIUM"} = -1;
   $types{"SOPOR"} = -1;
   $types{"SPåRVAGNS HåLL"} = -1;
   $types{"SPEED_BUMP"} = -1;
   $types{"SPEED_CAMERA"} = -1;
   $types{"SPORT"} = -1;
   $types{"STATUE"} = -1;
   $types{"STEPPING_STONES"} = -1;
   $types{"STEPS"} = -1;
   $types{"STILE"} = -1;
   $types{"STOP"} = -1;
   $types{"STORE_HOUSE"} = -1;
   $types{"STRIPCLUB"} = -1;
   $types{"STUDIO"} = -1;
   $types{"SUNNE MEDICAL CE"} = -1;
   $types{"SUNNE POLICE STA"} = -1;
   $types{"SUPERMARKET"} = -1;
   $types{"SURVEILLANCE"} = -1;
   $types{"SURVEY_POINT"} = -1;
   $types{"SUSHI"} = -1;
   $types{"SVENKS BILPROVNI"} = -1;
   $types{"SVøMMEHAL"} = -1;
   $types{"SWEDBANK"} = -1;
   $types{"TABLE"} = -1;
   $types{"TANNING BED"} = -1;
   $types{"TECHNICAL_BUILDI"} = -1;
   $types{"TECHNICAL_STATIO"} = -1;
   $types{"TELEPHONE"} = -1;
   $types{"TELESCOPE"} = -1;
   $types{"TERTIARY"} = -1;
   $types{"THEATRE_OUTDOOR"} = -1;
   $types{"THEME_PARK"} = -1;
   $types{"TIRES"} = -1;
   $types{"TJäRDAL"} = -1;
   $types{"TOILETS"} = -1;
   $types{"TOLL_BOOTH"} = -1;
   $types{"TOMB"} = -1;
   $types{"TOURIST INFORMAT"} = -1;
   $types{"TOWER"} = -1;
   $types{"TOWNHALL"} = -1;
   $types{"TRACK"} = -1;
   $types{"TRAFFIC_SCHOOL"} = -1;
   $types{"TRAFFIC_SIGN"} = -1;
   $types{"TRAM_ENTRANCE"} = -1;
   $types{"TRAM_STOP"} = -1;
   $types{"TRAP PIT"} = -1;
   $types{"TUMULUS"} = -1;
   $types{"TURNTABLE"} = -1;
   $types{"UNCLASSIFIED"} = -1;
   $types{"UNDEFINED"} = -1;
   $types{"UNIVERSITY"} = -1;
   $types{"UTö BAGGERI"} = -1;
   $types{"VALö"} = -1;
   $types{"VåRDCENTRAL"} = -1;
   $types{"VENDING MACHINE"} = -1;
   $types{"VENDING_MACHINE"} = -1;
   $types{"VIADUCT"} = -1;
   $types{"VIDEO RENTAL STO"} = -1;
   $types{"VIEWPOING"} = -1;
   $types{"VIEWPOINT"} = -1;
   $types{"WASTE_BASKET"} = -1;
   $types{"WASTE_BIN"} = -1;
   $types{"WASTE_CONTAINER"} = -1;
   $types{"WASTE_DISPOSAL"} = -1;
   $types{"WASTE_TRANSFER_S"} = -1;
   $types{"WASTEWATER_PLANT"} = -1;
   $types{"WATER"} = -1;
   $types{"WATERMILL"} = -1;
   $types{"WATER_SUPPLY"} = -1;
   $types{"WATER_TOWER"} = -1;
   $types{"WATER_WELL"} = -1;
   $types{"WELL"} = -1;
   $types{"WILDERNESS_HUT"} = -1;
   $types{"WILDLIFE_HIDE"} = -1;
   $types{"WINDMILL"} = -1;
   $types{"WOLF_PIT"} = -1;
   $types{"WORKS"} = -1;
   $types{"WRECK"} = -1;
   $types{"YES"} = -1;
   $types{"YOUTH CLUB"} = -1;


   return %types;
} # initPOITypesHash



# Get Wayfinder road class from street type
sub getWFroadClassFromStreetType {
   my $streetType = $_[0];
   my $midId = $_[1];
   my $streetName = $_[2];

   # return value, sofar invalid
   my $wfRoadClass = -1;

   # Get Wayfinder road class from the streetTypes hash
   if ( !scalar %m_streetTypes ) {
      %m_streetTypes = initStreetTypesHash();
   }

   if ( length $streetType == 0 ) {
      print "WARN no street type (0-length string) for segment $midId\n";
      $wfRoadClass = 4;
   }
   elsif ( defined( $m_streetTypes{$streetType} ) ) {
      $wfRoadClass = $m_streetTypes{$streetType};

   }
   else {
      # Don't die here,
      # Count the number of segments with each unknown street type
      # and summarize when all streets in the file are looped
      # Then die...
      
      if ( defined $m_unknownStreetTypes{$streetType} ) {
         $m_unknownStreetTypes{$streetType} += 1;
      } else {
         $m_unknownStreetTypes{$streetType} = 1;
         print "unknown street type: $streetType for segment $midId\n";
      }
      $wfRoadClass = -1;
   }
   
   
   # return
   return ($wfRoadClass);
} # getWFroadClassFromStreetType

sub initStreetTypesHash {
   
   # the map features are defined on 
   # http://wiki.openstreetmap.org/wiki/Map_Features
   my %types = ();
   $types{"motorway"}  = 0;
   $types{"motorway_link"}  = 0;
   $types{"trunk"}  = 0;
   $types{"trunk_link"}  = 0;

   $types{"primary"}  = 1;
   $types{"primary_link"}  = 1;

   $types{"secondary"}  = 2;
   $types{"secondary_link"}  = 2;
   
   $types{"tertiary"}  = 3;
   $types{"tertiary_link"}  = 3;

   # Set to road class higher than 4, since these may be part
   # of larger roads, not to loose them on high-level routing
   $types{"mini_roundabout"}  = 3;
   
   # road under construction, could actually be any road class
   $types{"construction"}  = 4;
   $types{"proposed"}  = 4;
   
   $types{"abandoned"}  = 4;
   $types{"disused"}  = 4;
   $types{"historic"}  = 4;
   $types{"old"}  = 4;
   
   $types{"bus_guideway"}  = 4;
   $types{"byway"}  = 4;
   $types{"ford"}  = 4; # road crosses stream or river, vehicle must enter water
   $types{"living_street"}  = 4;
   $types{"minor"}  = 4;
   $types{"Private road"}  = 4;
   $types{"raceway"}  = 4; # track for racing
   $types{"residential"}   = 4;
   $types{"residential;uncl"}   = 4;
   $types{"ramp"}  = 4;    # looks like part of a walkway in the OSM sweden 
                           # map, so dont set the ramp attribute
   $types{"road"}  = 4;
   $types{"Road"}  = 4;
   $types{"service"}  = 4;
   $types{"turning_circle"}  = 4; # 
   $types{"unclassified"}  = 4;
   $types{"undefined"}  = 4;
   $types{"unsurfaced"}  = 4;

   # uncertain what these are.
   # TODO: investigate and possible set them to a better roadclass and 
   #       other apropriate attribute
   $types{"access"}  = 4;
   $types{"incline_steep"}  = 4;
   $types{"psv"}  = 4;
   $types{"survey"}  = 4;
   $types{"un"}  = 4;
   
   # walkways and such that are not for cars and normal vehicles
   $types{"bridleway"}  = 4;  # for horses
   $types{"crossing"}  = 4;   # walkway where pedestrians can cross the street
   $types{"cycleway"}  = 4;
   $types{"cycleway;footway"}  = 4;
   $types{"elevator"}  = 4;
   $types{"escalator"}  = 4;
   $types{"footpath"}  = 4;
   $types{"footway"}  = 4;
   $types{"footway; cyclewa"}  = 4;
   $types{"path"}  = 4;
   $types{"platform"} = 4;    # platform of a bus stop or station
   $types{"pedestrian"}  = 4;
   $types{"steps"}  = 4;
   $types{"track"}  = 4;
   $types{"trail"}  = 4;
   $types{"walkway"}  = 4;
   $types{"Walk/Cycle path"}  = 4;
   $types{"ski_jump"}  = 4;
   $types{"snowmobileway"}  = 4;
   
   $types{"traffic_signals"}  = 4;


   return %types;
} # initStreetTypesHash


# Get Wayfinder road display class from street type
sub getWFroadDisplayClassFromStreetType {
   my $streetType = $_[0];
   my $midId = $_[1];
   my $streetName = $_[2];

   # return value, sofar invalid
   my $wfRoadDispClass = -1;

   # Get Wayfinder road class from the streetTypes hash
   if ( !scalar %m_roadDisplayClasses ) {
      %m_roadDisplayClasses = initStreetTypesRoadDisplayClassHash();
   }

   if ( length $streetType == 0 ) {
      print "WARN no street type (0-length string) for segment $midId\n";
      $wfRoadDispClass = -1;
   }
   elsif ( defined( $m_roadDisplayClasses{$streetType} ) ) {
      $wfRoadDispClass = $m_roadDisplayClasses{$streetType};

   }
   # else,
   # ok to return -1 for streetTypes that are not connected to any
   # road display class
   
   # return
   return ($wfRoadDispClass);
} # getWFroadDisplayClassFromStreetType


sub initStreetTypesRoadDisplayClassHash {

   # From mc2/Shared ItemTypes::roadDisplayClass_t
   my %enum = ();
   $enum{"partOfWalkway"} = 0;
   $enum{"partOfPedestrianZone"} = 1;
   $enum{"roadForAuthorities"} = 2;
   $enum{"entranceExitCarPark"} = 3;
   $enum{"etaParkingGarage"} = 4;
   $enum{"etaParkingPlace"} = 5;
   $enum{"etaUnstructTrafficSquare"} = 6;
   $enum{"partOfServiceRoad"} = 7;
   $enum{"roadUnderContruction"} = 8;

   my %dispClass = ();
   $dispClass{"service"} = $enum{"partOfServiceRoad"};
   $dispClass{"services"} = $enum{"partOfServiceRoad"};
   
   $dispClass{"bridleway"} = $enum{"partOfWalkway"}; # for horses
   
   $dispClass{"crossing"} = $enum{"partOfWalkway"};
   $dispClass{"cycleway"} = $enum{"partOfWalkway"};
   $dispClass{"cycleway;footway"} = $enum{"partOfWalkway"};
   $dispClass{"elevator"} = $enum{"partOfWalkway"};
   $dispClass{"escalator"} = $enum{"partOfWalkway"};
   $dispClass{"footpath"} = $enum{"partOfWalkway"};
   $dispClass{"footway"} = $enum{"partOfWalkway"};
   $dispClass{"footway; cyclewa"} = $enum{"partOfWalkway"};
   $dispClass{"path"} = $enum{"partOfWalkway"};
   $dispClass{"platform"} = $enum{"partOfWalkway"};
   $dispClass{"steps"} = $enum{"partOfWalkway"};
   $dispClass{"walkway"} = $enum{"partOfWalkway"};
   $dispClass{"Walk/Cycle path"} = $enum{"partOfWalkway"};
   
   $dispClass{"ski_jump"} = $enum{"partOfWalkway"};
   $dispClass{"trail"} = $enum{"partOfWalkway"};
   
   # Dont't know which is best for pedestrian.
   # pedestrian zone currenlty has no special drawsetting, walkway has. 
   # So I go with walkway.
   $dispClass{"pedestrian"} = $enum{"partOfPedestrianZone"};
   $dispClass{"pedestrian"} = $enum{"partOfWalkway"};

   $dispClass{"construction"}  = $enum{"roadUnderContruction"};
   $dispClass{"proposed"}  = $enum{"roadUnderContruction"};

   return %dispClass;
}

sub streetTypeIsRamp {
   my $streetType = $_[0];

   if ( ($streetType eq "motorway_link") or
        ($streetType eq "primary_link") or
        ($streetType eq "secondary_link") or
        ($streetType eq "tertiary_link") or
        ($streetType eq "trunk_link") ) {
      return 1;
   }

   return 0;
}
sub streetTypeIsRbt {
   my $streetType = $_[0];

   if ( ($streetType eq "mini_roundabout") ) {
      return 1;
   }

   return 0;
}
sub streetTypeIsControlledAccess {
   my $streetType = $_[0];

   if ( ($streetType eq "motorway") ) {
      return 1;
   }

   return 0;
}

sub streetTypeNoThroughfare {
   my $streetType = $_[0];

   if ( ($streetType eq "bus_guideway") or
        ($streetType eq "track") or # Roads for agricultural use, 
                                    # gravel roads in the forest etc.
        ($streetType eq "raceway") or
        ($streetType eq "Private road") or
        ($streetType eq "ford")
       ) {
      return 1;
   }

   return 0;
}

sub streetTypeWantedInTurnTable {
   my $streetType = $_[0];

   if ( ($streetType eq "bridleway") or
        ($streetType eq "crossing") or
        ($streetType eq "cycleway") or
        ($streetType eq "cycleway;footway") or
        ($streetType eq "elevator") or
        ($streetType eq "escalator") or
        ($streetType eq "footpath") or
        ($streetType eq "footway") or
        ($streetType eq "footway; cyclewa") or
        ($streetType eq "path") or
        ($streetType eq "steps") or
        ($streetType eq "trail") or
        ($streetType eq "walkway") or
        ($streetType eq "Walk/Cycle path")
      ) {
      return 1;
   }

   return 0;
}

sub streetTypeIsWanted {
   my $streetType = $_[0];
   my $midId = $_[1];

   if ( ($streetType eq "traffic_signals") ) {
      print " skipping streetType $streetType midId $midId\n";
      return 0;
   }

   return 1;
}



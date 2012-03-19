#!/usr/bin/perl -w
# Script to create Wayfinder midmif files from TA MultiNet Shape midmif files.
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

use vars qw( $opt_h $opt_v $opt_t $opt_y $opt_c );
use vars qw( $m_dbh 
             $m_countryID $m_sourceID $m_validFromID $m_validToID
             %m_idSubtraction 
             %m_poiEA
             $m_coordOrder );

# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
# Set lib assuming the script is located in genfiles/script
use FindBin '$Bin';
use lib "$Bin/perllib";

use POIObject;
use PerlWASPTools;
use GDFPoiFeatureCodes;
use PerlTools;

use DBI;


getopts('hv:t:y:c:');


if (defined $opt_h) {
   print <<EOM;
Script to create Wayfinder midmif files from some midmif files.

Usage: $0 
       options
       midmif-file(s)

Options:
-h          Display this help.

-y string   Coordinate system in mif files, complete with coord order,
            e.g. 'wgs84_latlon_deg'
-c string   The 2-letter ISO contry code of the country to process
-v string   The version of the data (the format specification differs
            between the versions):
               TA_2006_10_sa     Southern Africa
               TA_2008_v1_31_sa  Southern Africa
-t string   The type of item that is in the mid/mif file to process:
               municipal         mid+mif+altNameFile  (order1 province)
               builtUpArea       mid+mif              (builtUpArea)
               cityPart          mid+mif+buaOrder8mid (order9)
               landcover         mid+mif              (Land cover)
                  Will produce forest
               landuse           mid+mif              (Land use)
                  Will produce park+airportitems+cartographic+...
               railway           mid+mif
               network           mid+mif+taFile+municipalmid (Network (ssi+ferry))
               restrictions      manouvers.mid + manouvers path index.txt + network.mid
                  Will print forbidden turns to wf turn table
                  FIXME: this method needs change to handle all entries from the manouvers file
               waterLine         mid+mif
               waterPoly         mid+mif

               poi               mid+mif  (POIs)
               cityCentre        mid+mif  (city centres)


Note:
   * Run dos2unix on all midmif files before running this script
     to avoid windows eol.
   * POI-files need to have UTF-8 char encoding (and mc2-coords) before
     added to WASP db with poiImport script.
EOM
  exit 1;
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
my $THIRD_FILE_NAME;
if ( scalar @ARGV >= 3) {
   $THIRD_FILE_NAME = $ARGV[ 2 ];
   open THIRD_FILE, $THIRD_FILE_NAME
      or die "Can not open third file $THIRD_FILE_NAME: $!\n";
}
my $FOURTH_FILE_NAME;
if ( scalar @ARGV >= 4) {
   $FOURTH_FILE_NAME = $ARGV[ 3 ];
   open FOURTH_FILE, $FOURTH_FILE_NAME
      or die "Can not open fourth file $FOURTH_FILE_NAME: $!\n";
}
my $FIFTH_FILE_NAME;
if ( scalar @ARGV >= 5) {
   $FIFTH_FILE_NAME = $ARGV[ 4 ];
   open FIFTH_FILE, $FIFTH_FILE_NAME
      or die "Can not open fifth file $FIFTH_FILE_NAME: $!\n";
}
my $SIXTH_FILE_NAME;
if ( scalar @ARGV >= 6) {
   $SIXTH_FILE_NAME = $ARGV[ 5 ];
   open SIXTH_FILE, $SIXTH_FILE_NAME
      or die "Can not open sixth file $SIXTH_FILE_NAME: $!\n";
}


# Connect to database for working with POI objects, etc
$m_dbh = db_connect();


# Define global variables
my $midSepChar = ',';
my $midRplChar = '}';
my $MAX_UINT32 = 4294967295;
my %m_unknownPOIcategories = ();

# Check that map version is valid.
if ( !defined($opt_v) ) {
   die "Midmif map version not specified! - exit\n";
} else {
   if ( ($opt_v eq "TA_2010_06") ) {
      dbgprint "Extract data, version $opt_v";
   } elsif ( ($opt_v =~ m/TA_2[0-9]{3}_[01][0-9]/) ) {
      dbgprint "Extract data in compatibility mode TA_2010_06 from successor version $opt_v";
      $opt_v = "TA_2010_06";
   } else {
      print "Invalid version specified ($opt_v)\n";
      die "Exit!!!";
   }
}

# Check that country id given for those map releases that requires it
if ( !defined($opt_c) ) {
   die "Country (2 letter ISO-code) not given - exit";
} else {
  # SB HACK XXX
  #my $id = 12;	# France
  # my $id = 2; # Germany


   # SB HACK XXX
   my $id = getCountryIDFromISO3166Code($m_dbh, $opt_c);
   if ( !defined $id ) {
      die "No country with iso code $opt_c - exit"
   } else {
      dbgprint "Parsing data in country $opt_c with id $id";
      $m_countryID = $id;
   }
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


# Check what to do
if (defined($opt_t)) {
   dbgprint "Run $opt_t\n";
   if ($opt_t eq "builtUpArea") {
      handleBuiltUpAreaFile();
   } elsif ($opt_t eq "cityPart") {
      handleCityPartFile();
   } elsif ($opt_t eq "landcover") {
      handleLandcoverFile();
   } elsif ($opt_t eq "landuse") {
      handleLanduseFile();
   } elsif ($opt_t eq "municipal") {
      handleMunicipalFile();
   } elsif ($opt_t eq "network") {
      handleNetworkFile();
   } elsif ($opt_t eq "restrictions") {
      handleRestrictions();
   } elsif ($opt_t eq "poi") {
      handlePoiFile();
   } elsif ($opt_t eq "cityCentre") {
      handleCCFile();
   } elsif ($opt_t eq "railway") {
      handleRailwayFile();
   } elsif ($opt_t eq "waterLine") {
      handleWaterLineFile();
   } elsif ($opt_t eq "waterPoly") {
      handleWaterPolyFile();
   } else {
      die "Unknown item type: $opt_t\n";
   }
} else {
   die "No item type specified! Run $0 -h for help.\n";
}


exit;



# Function to check midId, that it is not large than MAX_UITN32
# Parseonly mode: the function returns 1 if midId is ok, it returns 0 
# if the midId si to large
# Running live-mode: the function dies if the midId is too large
sub checkMidId {
   my $midId = $_[0];
   my $parseOnly = $_[1];

   my $retVal = 1;
   if ( $midId > $MAX_UINT32 ) {
      $retVal = 0;
      if ( ! $parseOnly ) {
         errprint "Mid id is larger than MAX_UINT32 ($midId)";
         dbgprint "Use row number plus an offset or do some cut-off";
         die "exit";
      }
   } elsif ( $midId < 1 ) {
      $retVal = 0;
      if ( ! $parseOnly ) {
         errprint "Mid id is less than 0 ($midId)";
         dbgprint "Fix it!";
         die "exit";
      }
   }
   return $retVal;
}



# Get wayfinder mid file name lang string from the Tele Atlas nameLanguage
sub getWFNameLanguage {
   my $lang = $_[0];
   my $id = $_[1];

   if ( $lang eq "UND" ) {
      return "invalidLanguage";
   }
   elsif (getWFNameLanguageID($lang) != 31) {
       return lc($lang);
   }
   elsif ( $lang eq "" ) {
      return "invalidLanguage"; #workaround for entry:
      # 582760049030874,1119,"$DE","","","","","","","","$$WATER$$06",3675865170,1657476,"","",6,0,-1
      # in /root/Wayfinder-Conversion-3/Wayfinder-Server/Server/MapConversion/SupplierMapData/TeleAtlas/2011_06_eu_shp/midmif/deud58_a8.mid
   }
}
# Get language id, see WASP POINameLanguages table and LangTypes::language_t
sub getWFNameLanguageID {
   my $lang = $_[0];
   my $id = $_[1];
   my $name = $_[2];

   if ( $lang eq "ENG" ) {
      return 0;
   }
   if ( $lang eq "SWE" ) {
      return 1;
   }
   if ( $lang eq "GER" ) {
      return 2;
   }
   if ( $lang eq "DAN" ) {
      return 3;
   }
   if ( $lang eq "ITA" ) {
      return 4;
   }
   if ( $lang eq "DUT" ) {
      return 5;
   }
   if ( $lang eq "SPA" ) {
      return 6;
   }
   if ( $lang eq "FRE" ) {
      return 7;
   }
   if ( $lang eq "FIN" ) {
      return 9;
   }
   if ( $lang eq "WEL" ) {
      return 9;
   }
   if ( $lang eq "NOR" ) {
      return 10;
   }
   if ( $lang eq "POR" ) {
      return 11;
   }
   if ( $lang eq "AME" ) {
      return 12;
   }
   if ( $lang eq "CZE" ) {
      return 13;
   }
   if ( $lang eq "ALB" ) {
      return 14;
   }
   if ( $lang eq "BAQ" ) {
      return 15;
   }
   if ( $lang eq "CAT" ) {
      return 16;
   }
   if ( $lang eq "FRY" ) {
      return 17;
   }
   if ( $lang eq "GLE" ) {
      return 18;
   }
   if ( $lang eq "GLG" ) {
      return 19;
   }
   if ( $lang eq "LTZ" ) {
      return 20;
   }
   if ( $lang eq "ROH" ) {
      return 21;
   }
   if ( $lang eq "SCR" ) { # maybe this should be serbian latin syntax?
      return 22;
   }
   if ( $lang eq "SLV" ) {
      return 23;
   }
   if ( $lang eq "VAL" ) { # valencian - off standard
      return 24;
   }
   if ( $lang eq "HUN" ) {
      return 25;
   }
   if ( $lang eq "GRE" ) {
      return 26;
   }
   if ( $lang eq "ELL" ) {
      return 26;
   }
   if ( $lang eq "POL" ) {
      return 27;
   }
   if ( $lang eq "SLK" ) {
      return 28;
   }
   if ( $lang eq "SLO" ) {
      return 28;
   }
   if ( $lang eq "RUS" ) {
      return 29;
   }
   if ( $lang eq "GRL" ) {
      return 30;
   }
   if ( $lang eq "RUL" ) {
      return 32;
   }
   if ( $lang eq "TUR" ) {
      return 33;
   }
   if ( $lang eq "ARA" ) {
      return 34;
   }

   # this is completely off specification:
   if ( $lang eq "CHI" ) {
      return 35;
   }
   if ( $lang eq "ZHO" ) {
      return 35;
   }
   if ( $lang eq "EST" ) {
      return 37;
   }
   if ( $lang eq "LAV" ) {
      return 38;
   }
   if ( $lang eq "LIT" ) {
      return 39;
   }
   if ( $lang eq "THA" ) {
      return 40;
   }
   if ( $lang eq "BUL" ) {
      return 41;
   }
   if ( $lang eq "IND" ) {
      return 43;
   }
   if ( $lang eq "MAY" ) {
      return 44;
   }
   if ( $lang eq "MSA" ) {
      return 44;
   }
   if ( $lang eq "TGL" ) {
      return 49;
   }
   if ( $lang eq "BEL" ) {
      return 50;
   }
   if ( $lang eq "CRO" ) { # croatian; off-standard
      return 53;
   }
   if ( $lang eq "FAR" ) { # farsi; off-standard
      return 54;
   }
   if ( $lang eq "HEB" ) {
      return 58;
   }
   if ( $lang eq "MAC" ) {
      return 65;
   }
   if ( $lang eq "MAK" ) {
      return 65;
   }
   if ( $lang eq "MOL" ) {
      return 68;
   }
   if ( $lang eq "RON" ) {
      return 71;
   }
   if ( $lang eq "RUM" ) {
      return 71;
   }
   if ( $lang eq "SER" ) { # serbian ; off-standard
      return 72;
   }
   if ( $lang eq "UKR" ) {
      return 81;
   }
   if ( $lang eq "BOS" ) { # bosnian; off-standard
      return 87;
   }
   if ( $lang eq "SLA" ) {
      return 88;
   }
   if ( $lang eq "MLT" ) {
      return 93;
   }
   if ( $lang eq "SRD" ) {
       return 97;
   }

   # these are new occurrences from eastern europe.
   if ( $lang eq "SCC" ) { # serbo croatian cyrillic ?
      return 22;
   }
   if ( $lang eq "UKL" ) { # Ukranian latin syntax?
      return 92;
   }
   if ( $lang eq "MAT" ) { # Macedonian latin syntax?
      return 90;
   }
   if ( $lang eq "BET" ) { # Belarusian latin syntax?
      return 89;
   }
   if ( $lang eq "BUN" ) { # Bulgarian latin syntax?
      return 86;
   }
   


   if ( $lang eq "UND" ) {
      return 31; # undefined = invalidLanguage
   }

   if ( $lang eq "" ) {
      return 31; # empty -> invalidLanguage. hack for now.
   }
   else {
      errprint "getWFNameLanguageID unknown lang $lang " .
               "for id $id name $name";
      die "exit";
   }
}

# Get wayfinder mid file name type string from the nameType
sub getWFNameType {
   my $type = $_[0];
   my $id = $_[1];

   if ( $type eq "ON" ) {
      return "officialName";
   }
   if ( $type eq "AN" ) {
      return "alternativeName";
   }
   else {
      errprint "unknown type $type for id $id";
      die "exit";
   }
}

sub getWFNameTypeFromBitMask {
   my $bitmask = $_[0];
   my $id = $_[1];

   # Street Name Type (Bit Mask)
   # 0 = No Name
   # 1 = Official Name
   # 2 = Alternate Name
   # 4 = Route Name
   # 8 = Brunnel Name
   # 16 = Street Name
   # 32 = Locality Name
   # 64 = Postal Street Name

   my $on=0;
   my $an=0;
   my $rn=0;
   my $bn=0;
   my $sn=0;
   my $ln=0;
   my $psn=0;
   
   my $tmpType = $bitmask;
   if ( ($tmpType - 64) >= 0 ) {
      $psn = 1;
      $tmpType -= 64;
   }
   if ( ($tmpType - 32) >= 0 ) {
      $ln = 1;
      $tmpType -= 32;
   }
   if ( ($tmpType - 16) >= 0 ) {
      $sn = 1;
      $tmpType -= 16;
   }
   if ( ($tmpType - 8) >= 0 ) {
      $bn = 1;
      $tmpType -= 8;
   }
   if ( ($tmpType - 4) >= 0 ) {
      $rn = 1;
      $tmpType -= 4;
   }
   if ( ($tmpType - 2) >= 0 ) {
      $an = 1;
      $tmpType -= 2;
   }
   if ( ($tmpType - 1) >= 0 ) {
      $on = 1;
      $tmpType -= 1;
   }
   if ( $tmpType != 0 ) {
      errprint "bitmask name type $bitmask -> $tmpType after masking up";
      die "exit";
   }

   #print "$bitmask for id $id" .
   #      " -> on=$on an=$an rn=$rn bn=$bn sn=$sn ln=$ln psn=$psn\n";

   if ( $on ) {
      return "officialName";
   } elsif ( $an ) {
      return "alternativeName";
   } elsif ( $bn ) {
      return "brunnelName";
   } elsif ( $rn ) {
      return "roadNumber";
   }
   else {
      errprint "unhandled name type from bitmask value $bitmask for id $id";
      print " on=$on an=$an rn=$rn bn=$bn sn=$sn ln=$ln psn=$psn\n";
      die "exit";
   }
}


# This action processes a built_up area mid file
sub handleBuiltUpAreaFile {
   dbgprint "handleBuiltUpAreaFile";

   my $midIdCol = "";
   my $featTypeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $featTypeCol = 1;
      $nameCol = 4;
      $nameLcCol = 5;
   } else {
      die "Specify attribute parsing for $opt_v";
   }

   
   # Open Wayfinder builtUpArea mid file
   open(WF_FILE, ">areaWFbuiltUpAreaItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "areaWFbuiltUpAreaItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItems = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 0) {
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         checkMidId($midId, 0); # not parseonly die if check fails
         
         # Names
         my $itemName = $midRow[ $nameCol ];
         my $nameLang =
            getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
         
         # If the miditem has no name at all set name to "$$$"
         if ( length($itemName) == 0 ) {
            print "ERROR Bua midId=$midId has no name \n";
            die "exit";
         }
         
         my $allNames = "";
         if ( length($itemName) > 0) {
            $allNames = "$itemName:officialName:$nameLang";
         }

         print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";
         $nbrItems += 1;
            
         # Read next feature from mif file and write to correct WF mif
         my $wanted = 1;
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Wrote $nbrItems bua items to mid";
   
} # handleBuiltUpAreaFile


# This action processes the order9 file as citypart
sub handleCityPartFile {
   dbgprint "handleCityPartFile";

   # define columns for citypart file
   my $midIdCol = "";
   my $featTypeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $featTypeCol = 1;
      $nameCol = 14;
      $nameLcCol = 15;
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   
   # Open Wayfinder item mid file
   open(WF_FILE, ">areaWFcityPartItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "areaWFcityPartItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItems = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 0) {
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         checkMidId($midId, 0); # not parseonly die if check fails
         
         # Names
         my $itemName = $midRow[ $nameCol ];
         my $nameLang =
            getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
         
         # If the miditem has no name at all set name to "$$$"
         if ( length($itemName) == 0 ) {
            print "WARNING Item midId=$midId has no name\n";
            $itemName='$$$';
            #die "exit";
         }
         
         my $allNames = "";
         if ( length($itemName) > 1) {
            $allNames = "$itemName:officialName:$nameLang";
         }

         print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";
         $nbrItems += 1;
            
         # Read next feature from mif file and write to correct WF mif
         my $wanted = 1;
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Wrote $nbrItems order9 citypart items to mid";
   
} # handleCityPartFile


# This action processes a railway mid file
sub handleRailwayFile {
   dbgprint "handleRailwayFile";

   my $midIdCol = "";
   my $typeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $typeCol = 1;
      $nameCol = 7;
      $nameLcCol = 8;
   } else {
      die "Specify attribute parsing for $opt_v";
   }

   
   # Open Wayfinder item mid file
   open(WF_FILE, ">WFrailwayItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFrailwayItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItems = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 0) {
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         #if ( length($midIdCol) > 0 ) {
         #   $midId = $midRow[ $midIdCol ];
         #}
         
         # Names
         # we want no names
         my $itemName = "";
         my $allNames = "";
         if ( length($itemName) > 1) {
            my $nameLang =
            getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
            $allNames = "$itemName:officialName:$nameLang";
         }
         
         print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";
         $nbrItems += 1;
            
         # Read next feature from mif file and write to correct WF mif
         my $wanted = 1;
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Wrote $nbrItems railway items to mid";
   
}



# This action processes a Landuse file
sub handleLanduseFile {
   dbgprint "handleLanduseFile - create misc WF items";
   
   # Define attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $typeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   my $dispTypeCol = "";
   my $classCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $typeCol = 1;
      $nameCol = 4;
      $nameLcCol = 5;
      $dispTypeCol = 7;
      $classCol = 8;
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   
   # Open Wayfinder mid files
   # Define WF mif files, open to clear for append writing
   open(WF_B_FILE, ">WFbuildingItems.mid");
   open(WF_P_FILE, ">WFparkItems.mid");
   open(WF_A_FILE, ">WFairportItems.mid");
   open(WF_ACRI_FILE, ">WFaircraftRoadItems.mid");
   open(WF_ISL_FILE, ">WFislandItems.mid");
   open(WF_CARTO_FILE, ">WFcartographicItems.mid");
   open(WF_IBI_FILE, ">WFindividualBuildingItems.mid");
   # Define Wayfinder mif files,
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outBFileName = "WFbuildingItems.mif";
   printOneWayfinderMifHeader( $outBFileName, "$opt_y" );
   my $outPFileName = "WFparkItems.mif";
   printOneWayfinderMifHeader( $outPFileName, "$opt_y" );
   my $outAFileName = "WFairportItems.mif";
   printOneWayfinderMifHeader( $outAFileName, "$opt_y" );
   my $outAcriFileName = "WFaircraftRoadItems.mif";
   printOneWayfinderMifHeader( $outAcriFileName, "$opt_y" );
   my $outIslFileName = "WFislandItems.mif";
   printOneWayfinderMifHeader( $outIslFileName, "$opt_y" );
   my $outCartoFileName = "WFcartographicItems.mif";
   printOneWayfinderMifHeader( $outCartoFileName, "$opt_y" );
   my $outIBIFileName = "WFindividualBuildingItems.mif";
   printOneWayfinderMifHeader( $outIBIFileName, "$opt_y" );
   
   my %nbrOrigFeatures = ();
   my %nbrItems = ();
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
   my $nbrItemsToWFfile = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() >= 0) {
         $nbrItemsInFile += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         #if ( length($midIdCol) > 0 ) {
         #   $midId = $midRow[ $midIdCol ];
         #}
         my $itemName = "";
         if ( length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1 ) {
            my $nameLang =
            getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
            $nbrItemsWithName += 1;
            $allNames = "$itemName:officialName:$nameLang";
         }
         
         # displayType, class
         my $dispType = $midRow[ $dispTypeCol ];
         my $class = $midRow[ $classCol ];
         
         # find out which type of item this is
         my $wfType = "none";
         my $wfBuildingType = "unknownType";
         my $wfCartoType = "noCartographicType";
         my $wfParkType = "";
         my $wfIBIType = "";
         if ( ($midRow[ $typeCol ] == 7110) ) {    # building
            $wfType = "ibi";
            if ( $class == 1 ) {
               $wfIBIType = "airportTerminal";
            } elsif ( $class == 2 ) {
               $wfIBIType = "otherIndividualBuilding";
            } elsif ( $class == 13 ) {
               $wfIBIType = "parkingGarage";
            } else {
               die "handle ibi class = $class";
            }
         }
         elsif ( ($midRow[ $typeCol ] == 7170) ) {    # park/garden
            $wfType = "park";
            if ( ($dispType == 5) or ($dispType == 2) ) {
               $wfParkType = "regionOrNationalPark";
            } elsif ( ( $dispType == 1 ) or ($dispType == 3) ) {
               $wfParkType = "cityPark";
            } elsif ( $dispType == 4 ) {
               $wfParkType = "stateOrProvincePark";
            } else {
               die "handle park dispType = $dispType";
            }
         }
         elsif ( ($midRow[ $typeCol ] == 7180) ) {    # island
            $wfType = "island";
         }
         elsif ( ($midRow[ $typeCol ] == 9353) ) {    # company ground
            $wfType = "cartographic";
            $wfCartoType = "institution";
         }
         elsif ( ($midRow[ $typeCol ] == 9710) ) {    # beach/dune/sand area
            $wfType = "park";
            $wfParkType = "cityPark";
         }
         elsif ( ($midRow[ $typeCol ] == 9715) ) {    # industry
            $wfType = "building";
            $wfBuildingType = "unknownType";
         }
         elsif ( ($midRow[ $typeCol ] == 9720) ) {    # gewerbegebiet?
            $wfType = "building";
            $wfBuildingType = "otherIndividualBuilding";
         }
         elsif ( $midRow[ $typeCol ] == 9730 ) {   # freeport
            $wfType = "cartographic";
            $wfCartoType = "freeport";
         }
         elsif ( $midRow[ $typeCol ] == 9731 ) {   # Abbey Ground
            $wfType = "cartographic";
            $wfCartoType = "abbeyGround";
         }
         elsif ( $midRow[ $typeCol ] == 9732 ) {   # airport ground
            $wfType = "airport";
         }
         elsif ( $midRow[ $typeCol ] == 9733 ) {   # amusement park ground
            $wfType = "cartographic";
            $wfCartoType = "amusementParkGround";
         }
         elsif ( $midRow[ $typeCol ] == 9734 ) {   # arts centre ground
            $wfType = "cartographic";
            $wfCartoType = "artsCentreGround";
         }
         elsif ( $midRow[ $typeCol ] == 9735 ) {   # camping site ground
            $wfType = "cartographic";
            $wfCartoType = "campingGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9737) ) {    # castleNotToVisitGround
            $wfType = "cartographic";
            $wfCartoType = "castleNotToVisitGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9738) ) {    # castleToVisitGround
            $wfType = "cartographic";
            $wfCartoType = "castleToVisitGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9739) ) {    # Church ground
            $wfType = "cartographic";
            $wfCartoType = "churchGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9740) ) {    # city hall ground
            $wfType = "cartographic";
            $wfCartoType = "cityHallGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9741) ) {    # Court House Ground
            $wfType = "cartographic";
            $wfCartoType = "courthouseGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9742) ) {    # fire station ground
            $wfType = "cartographic";
            $wfCartoType = "fireStationGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9743) ) {    # fortress ground
            $wfType = "cartographic";
            $wfCartoType = "fortressGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9744) ) {    # golf
            $wfType = "cartographic";
            $wfCartoType = "golfGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9745) ) {    # governmentBuildingGround
            $wfType = "cartographic";
            $wfCartoType = "governmentBuildingGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9748) ) {    # hospital ground
            $wfType = "cartographic";
            $wfCartoType = "hospitalGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9750) ) {    # Library ground
            $wfType = "cartographic";
            $wfCartoType = "libraryGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9751) ) {    # Lighthouse ground
            $wfType = "cartographic";
            $wfCartoType = "lightHouseGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9753) ) {    # monastery ground
            $wfType = "cartographic";
            $wfCartoType = "monasteryGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9754) ) {    # museum ground
            $wfType = "cartographic";
            $wfCartoType = "museumGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9756) ) {    # parking area ground
            $wfType = "cartographic";
            $wfCartoType = "parkingAreaGround";
         } # skip petrol station ground
         elsif ( ($midRow[ $typeCol ] == 9758) ) {    # place Of Interest Building
            $wfType = "cartographic";
            $wfCartoType = "placeOfInterestBuilding";
         } # skip monument ground
         elsif ( ($midRow[ $typeCol ] == 9760) ) {    # policeOfficeGround
            $wfType = "cartographic";
            $wfCartoType = "policeOfficeGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9761) ) {    # prisonGround
            $wfType = "cartographic";
            $wfCartoType = "prisonGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9762) ) {    # railway station ground
            $wfType = "cartographic";
            $wfCartoType = "railwayStationGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9763) ) {    # recreational area ground
            $wfType = "cartographic";
            $wfCartoType = "recreationalAreaGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9765) ) {    # rest area ground
            $wfType = "cartographic";
            $wfCartoType = "restAreaGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9767) ) {    # sports hall ground
            $wfType = "cartographic";
            $wfCartoType = "sportsHallGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9768) ) {    # stadium ground
            $wfType = "cartographic";
            $wfCartoType = "stadiumGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9769) ) {    # state police office
            $wfType = "cartographic";
            $wfCartoType = "statePoliceOffice";
         }
         elsif ( ($midRow[ $typeCol ] == 9770) ) {    # theatre
            $wfType = "cartographic";
            $wfCartoType = "theatreGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9771) ) {    # university/college
            $wfType = "cartographic";
            $wfCartoType = "universityOrCollegeGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9774) ) {    # waterMillGround
            $wfType = "cartographic";
            $wfCartoType = "waterMillGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9775) ) {    # zoo
            $wfType = "cartographic";
            $wfCartoType = "zooGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9776) ) {    # runway
            $wfType = "acri";
         }
         elsif ( ($midRow[ $typeCol ] == 9777) ) {    # post office ground
            $wfType = "cartographic";
            $wfCartoType = "postOfficeGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9778) ) {    # windmillGround
            $wfType = "cartographic";
            $wfCartoType = "windmillGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9780) ) {    # institution
            $wfType = "cartographic";
            $wfCartoType = "institution";
         }
         elsif ( ($midRow[ $typeCol ] == 9781) ) {    # other landuse
            $wfType = "cartographic";
            $wfCartoType = "otherLandUse";
         }
         elsif ( ($midRow[ $typeCol ] == 9788) ) {    # cemetery
            $wfType = "cartographic";
            $wfCartoType = "cemetaryGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9790) ) {    # shopping centre
            $wfType = "cartographic";
            $wfCartoType = "shoppingCenterGround";
         }
         elsif ( ($midRow[ $typeCol ] == 9791) ) {    # school ground -> treat as university or college
            $wfType = "cartographic";
            $wfCartoType = "universityOrCollegeGround";
         } else {
            errprint "Handle land use type " . $midRow[ $typeCol ];
            die "exit";
         }
         
         my $wanted = ( $wfType ne "none" );
         # count nbr items of each feature type
         if ( defined($nbrOrigFeatures{ $midRow[$typeCol] }) ) {
            $nbrOrigFeatures{ $midRow[$typeCol] } += 1;
         } else {
            $nbrOrigFeatures{ $midRow[$typeCol] } = 1;
         }
         
         # print to the correct WF mid file if wanted
         if ( $wanted ) {
            $nbrItemsToWFfile += 1;
            if ( $wfType eq "park" ) {
               print WF_P_FILE "$midId,\"$itemName\",\"$allNames\"" .
                               ",\"$wfParkType\"\n";
            } elsif ( $wfType eq "building" ) {
               print WF_B_FILE "$midId,\"$itemName\",\"$allNames\"" .
                               ",\"$wfBuildingType\"\n";
            } elsif ( $wfType eq "airport" ) {
               print WF_A_FILE "$midId,\"$itemName\",\"$allNames\"\n";
            } elsif ( $wfType eq "acri" ) {
               print WF_ACRI_FILE "$midId,\"$itemName\",\"$allNames\"\n";
            } elsif ( $wfType eq "island" ) {
               print WF_ISL_FILE "$midId,\"$itemName\",\"$allNames\"\n";
            } elsif ( $wfType eq "cartographic" ) {
               print WF_CARTO_FILE "$midId,\"$itemName\",\"$allNames\"" .
                               ",\"$wfCartoType\"\n";
            } elsif ( $wfType eq "ibi" ) {
               print WF_IBI_FILE "$midId,\"$itemName\",\"$allNames\"" .
                               ",\"$wfIBIType\"\n";
            } else {
               errprint "wfType '$wfType' - which mid file?";
               die "exit";
            }
            # count nbr items of each item type
            if ( defined($nbrItems{$wfType}) ) {
               $nbrItems{$wfType} += 1;
            } else {
               $nbrItems{$wfType} = 1;
            }
         }

         # read the feature from mif, print to WF mif if wanted
         my $outFileName = "_invalidMifFileName";
         if ( $wfType eq "park" ) {
             $outFileName = $outPFileName;
         } elsif ( $wfType eq "building") {
             $outFileName = $outBFileName;
         } elsif ( $wfType eq "airport") {
             $outFileName = $outAFileName;
         } elsif ( $wfType eq "acri") {
             $outFileName = $outAcriFileName;
         } elsif ( $wfType eq "island") {
             $outFileName = $outIslFileName;
         } elsif ( $wfType eq "cartographic") {
             $outFileName = $outCartoFileName;
         } elsif ( $wfType eq "ibi") {
             $outFileName = $outIBIFileName;
         } elsif ( $wfType eq "none") {
             # do nothing.
         } else {
            errprint "wfType '$wfType' - which mif file?";
            die "exit";
         }
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature( 
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outFileName, $wanted, $midId );
      }
   }

   print "\n=======================================================\n";
   dbgprint "Read totally $nbrItemsInFile items from file, " .
            "$nbrItemsWithName have name.";
   foreach my $t ( sort(keys(%nbrOrigFeatures)) ) {
      print "  $t: " . $nbrOrigFeatures{$t} . "\n";
   }
   dbgprint "Wrote these $nbrItemsToWFfile items to WF mid/mif files:";
   foreach my $t ( sort(keys(%nbrItems)) ) {
      print "  $t: " . $nbrItems{$t} . "\n";
   }
}

# This action processes a Landcover file
sub handleLandcoverFile {
   dbgprint "handleLandcoverFile - create misc WF items";
   
   # Define attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $typeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   my $dispTypeCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $typeCol = 1;
      $nameCol = 4;
      $nameLcCol = 5;
      $dispTypeCol = 7;
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   
   # Open Wayfinder mid files
   # Define WF mif files, open to clear for append writing
   open(WF_FOREST_FILE, ">WFforestItems.mid");
   open(WF_P_FILE, ">WFparkItems.mid");
   # Define Wayfinder mif files,
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outForestFileName = "WFforestItems.mif";
   printOneWayfinderMifHeader( $outForestFileName, "$opt_y" );

   my $outParkFileName = "WFparkItems.mif";
   printOneWayfinderMifHeader( $outParkFileName, "$opt_y" );
   
   my %nbrOrigFeatures = ();
   my %nbrItems = ();
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
   my $nbrItemsToWFfile = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() >= 0) {
         $nbrItemsInFile += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         #if ( length($midIdCol) > 0 ) {
         #   $midId = $midRow[ $midIdCol ];
         #}
         my $itemName = "";
         if ( length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1 ) {
            my $nameLang =
            getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
            $nbrItemsWithName += 1;
            $allNames = "$itemName:officialName:$nameLang";
         }
         
         # displayType, class
         my $dispType = $midRow[ $dispTypeCol ];
         
         # find out which type of item this is
         my $wfType = "none";
         if ( ($midRow[ $typeCol ] == 7120) ) {    # forest
             $wfType = "forest";
         } elsif ( ($midRow[ $typeCol ] == 9725) ) {    # moors and heathland - data type appearing in deu_d52.
	     $wfType = "park";
         } elsif ( ($midRow[ $typeCol ] == 9710) ) {    # beach/dune - appearing in ndlndl
             $wfType = "park";
         } else {
            errprint "Handle land cover type " . $midRow[ $typeCol ];
            die "exit";
         }
         
         my $wanted = ( $wfType ne "none" );
         # count nbr items of each feature type
         if ( defined($nbrOrigFeatures{ $midRow[$typeCol] }) ) {
            $nbrOrigFeatures{ $midRow[$typeCol] } += 1;
         } else {
            $nbrOrigFeatures{ $midRow[$typeCol] } = 1;
         }
         
         # print to the correct WF mid file if wanted
         if ( $wanted ) {
            $nbrItemsToWFfile += 1;
            if ( $wfType eq "forest" ) {
               print WF_FOREST_FILE "$midId,\"$itemName\",\"$allNames\"\n";
            } elsif ( $wfType eq "park" ) {
               print WF_P_FILE "$midId,\"$itemName\",\"$allNames\"\n";
            } else {
               errprint "wfType '$wfType' - which mid file?";
               die "exit";
            }
            # count nbr items of each item type
            if ( defined($nbrItems{$wfType}) ) {
               $nbrItems{$wfType} += 1;
            } else {
               $nbrItems{$wfType} = 1;
            }
         }

         # read the feature from mif, print to WF mif if wanted
         my $outFileName = "_invalidMifFileName";
         if ( $wfType eq "forest" ) {
             $outFileName = $outForestFileName;
         } elsif ($wfType eq "park") {
             $outFileName = $outParkFileName;
         } elsif ($wanted) {
            errprint "wfType '$wfType' - which mif file?";
            die "exit";
         }
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature( 
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outFileName, $wanted, $midId );
      }
   }

   print "\n=======================================================\n";
   dbgprint "Read totally $nbrItemsInFile items from file, " .
            "$nbrItemsWithName have name.";
   foreach my $t ( sort(keys(%nbrOrigFeatures)) ) {
      print "  $t: " . $nbrOrigFeatures{$t} . "\n";
   }
   dbgprint "Wrote these $nbrItemsToWFfile items to WF mid/mif files:";
   foreach my $t ( sort(keys(%nbrItems)) ) {
      print "  $t: " . $nbrItems{$t} . "\n";
   }
}

# This action processes a the mid file to be municipals
sub handleMunicipalFile {
   dbgprint "Handles municipals";
  
   # Define municipal file attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $featTypeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $featTypeCol = 1;
      $nameCol = 13;
      $nameLcCol = 14;
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   
   # get a good field sep for the admin area names
   my $poiObject = POIObject->newObject();
   my $niceSeparator = $poiObject->getFieldSeparator();

   # read file with admin area names
   my %extraNames = ();
   if ( scalar @ARGV == 3 ) {
      my $altIdCol = "";
      my $altNameCol = "";
      my $altNameTypeCol = "";
      my $altNameLangCol = "";
      if ( ($opt_v eq "TA_2010_06") ){
         $altIdCol = 0;
         $altNameCol = 3;
         $altNameTypeCol = 2;
         $altNameLangCol = 4;
      } else {
         die "Specify admin area names attribute parsing for $opt_v";
      }
      print "\n";
      print "Store admin area names\n";
      my $rows = 0;
      while (<THIRD_FILE>) {
         chomp;
         if (length() > 1) {
            $rows += 1;
            # Split the mid line, handling $midSepChar within strings
            # and removing "-chars from string values
            my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

            my $id = $midRow[ $altIdCol ];
            my $name = $midRow[ $altNameCol ];
            my $nameType = $midRow[ $altNameTypeCol ];
            my $nameLang = $midRow[ $altNameLangCol ];
            my $nameTypeLangString = "$nameType$niceSeparator$nameLang$niceSeparator$name";
            push @{$extraNames{$id}}, "$nameTypeLangString";
         }
      }
      print "Stored admin area names from $rows rows for " .
            scalar(keys(%extraNames)) . " areas\n";
   }
  
   # Open Wayfinder municipal file
   open(WF_FILE, ">WFmunicipalItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFmunicipalItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );

   my $nbrItems = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 0) {
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         my $trueMidId = $midRow[ $midIdCol ];
         checkMidId($midId, 0); # not parseonly die if check fails
         
         # Names
         my $nameLang =
            getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
         my $itemName = $midRow[ $nameCol ];
         
         # If the miditem has no name, exit
         if ( (length($itemName) == 0) ) {
            print "WARNING Municipal midId=$midId has no name!\n";
            $itemName='$$$';
            #die "exit";
         }
         
         my $allNames = "";
         if ( length($itemName) > 1) {
            $allNames = "$itemName:officialName:$nameLang";
            if ( defined $extraNames{$trueMidId} ) {
               foreach my $nameTypeLangString ( @{$extraNames{$trueMidId}} ) {
                  ( my $extraType, my $extraLang, my $extraName) =
                     split( "$niceSeparator", $nameTypeLangString);
                  
                  my $wfLang = getWFNameLanguage( $extraLang, $midId);
                  my $wfType = getWFNameType( $extraType, $midId);
                  $allNames .= " $extraName:$wfType:$wfLang";
               }
            }
         }
         
         print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";
         $nbrItems += 1;
            
         # Read next feature from mif file and write to correct WF mif
         my $wanted = 1;
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Wrote $nbrItems municipal items to mid";
   
}



# This action processes a network mid mif file
sub handleNetworkFile {
   dbgprint "handleNetworkFile";

   my $defaultWFspeed = 50;
   my $readNoThroughfareIds = 0;
   my $readHnbrs = 0;
   dbgprint "handleNetworkFile $opt_v";
   dbgprint "Default speed limit $defaultWFspeed";
   if ( scalar @ARGV != 6 ) {
      errprint "handleNetworkFile: needs\n" .
               " - network.mid\n" .
               " - network.mif\n" .
               " - ta.txt (transportation elem belonging to area)\n" .
               " - orderX.mid\n" .
               " - pc.txt (postal codes)\n" .
               " - lrs.txt (restrictions for logistics)";
      die "exit";
   }


   # FIXME: To get house numbers for the street segments, need to load
   # another file from Tele Atlas that holds that information
   # Fill this hash, so you can print to WF midmif for the street segments
   my %hnbrs = ();
   # $WFleftStart =  $hnbrs{$midId}->{m_leftStart};
   # $WFleftEnd =    $hnbrs{$midId}->{m_leftEnd};
   # $WFrightStart = $hnbrs{$midId}->{m_rightStart};
   # $WFrightEnd =   $hnbrs{$midId}->{m_rightEnd};



   # Read the logistics restrictions file.
   # example: "522760003006761","1","4110","!A","50","1","2.80","3","","1","0"
   # specification: FeatureID, SEQNR, FEATTYP, RESTRTYP, VEHTYP, RSTRVAL(Scope), LIMIT(value of restr.), UNIT_MEAS, LANE_VALID, VARDIR, VERIFIED
   my %restrictions = ();
   if ( scalar @ARGV >= 6 ) {
      
      my $idCol = "";
      my $seqCol = "";
      my $feattypeCol = "";
      my $restrtypeCol= "";
      my $scopeCol = "";
      my $valueCol = "";

      if ( ($opt_v eq "TA_2010_06") ) {
          $idCol = 0;
          $seqCol = 1;
          $feattypeCol = 2;
          $restrtypeCol= 3;
          $scopeCol = 5;
          $valueCol = 6;
      } else {
         die "Specify attribute parsing for $opt_v";
      }
      
      print "\n";
      print "Store restrictions ids-rownbr\n";
      my $rows = 0;
      my $lastID = "";
      while (<SIXTH_FILE>) {
         chomp;
         if (length() > 1) {
            $rows += 1;
            # Split the mid line, handling $midSepChar within strings
            # and removing "-chars from string values
            my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

            if (($midRow[ $feattypeCol ] != 4110) && ($midRow[ $feattypeCol ] != 4130)) {
                # only feature type 4110 and 4130 refer to street segments; others refer to ID of lmn table.
                #print "skipping midrow $rows because feature type is $midRow[$feattypeCol].\n";
		next
	    }

            my $midid = $midRow[ $idCol ];

            if ((($midRow[$restrtypeCol] eq "!*") || ($midRow[$restrtypeCol] eq "@*")) && ($midRow[ $scopeCol ] != 1)) {
                # not a hard restriction; may be ignored.
                die "$midid does not contain a hard restriction, but rather $midRow[$scopeCol]\n";
		next;
	    }

            $restrictions{$midid}{$midRow[$restrtypeCol]} = $midRow[$valueCol];
         }
      }
      print "Stored " . scalar(keys(%restrictions)) . 
            " restrictions from $rows rows\n";

#      foreach my $midid (keys(%{%restrictions})) {
#	  foreach my $a (keys(%{$restrictions{$midid}})) {
#	      print "$a:",${$restrictions{$midid}}{$a}, " ";
#	  }
#	  print "\n";
#      }
   }
   dbgprint "Stored " . scalar(keys(%restrictions)) . " restrictions by id";
   print "\n";



   # Read the postal codes file.
   # example: "522760004345792","1","0","1","63741","",""
   # specification: ID, POSTALTYP (1:Main, 2:Sub), Side of Line (0:Both sides, 1:left, 2:right), SeqNR, POSTCODE, NAME, NAMELC(languagecode)
   my %postalcodes = ();
   if ( scalar @ARGV >= 5 ) {
      
      my $idCol = "";
      my $typeCol = "";
      my $sideCol = "";
      my $codeCol = "";

      if ( ($opt_v eq "TA_2010_06") ) {
         $idCol = 0;
         $typeCol = 1;
         $sideCol = 2;
         $codeCol = 4;
      } else {
         die "Specify attribute parsing for $opt_v";
      }
      
      print "\n";
      print "Store postal code ids-rownbr\n";
      my $rows = 0;
      while (<FIFTH_FILE>) {
         chomp;
         if (length() > 1) {
            $rows += 1;
            # Split the mid line, handling $midSepChar within strings
            # and removing "-chars from string values
            my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

            my $rowId = $.;
            my $midid = $midRow[ $idCol ];
            #$postalcodes{$midid} = $rowId;
            $postalcodes{$midid} = $midRow[ $codeCol ];
         }
      }
      print "Stored " . scalar(keys(%postalcodes)) . 
            " postal codes from $rows rows\n";
   }
   dbgprint "Stored " . scalar(keys(%postalcodes)) . " postal codes by id";
   print "\n";

   # Read the orderX file, to store rownbr (municipal midId)
   # for the orderX ids
   my %orderXRowIds = ();
   if ( scalar @ARGV >= 4 ) {
      
      my $idCol = "";
      my $typeCol = "";
      my $order = "";
      if ( ($opt_v eq "TA_2010_06") ) {
         $idCol = 0;
         $typeCol = 1;
         $order = 8;
      } else {
         die "Specify attribute parsing for $opt_v";
      }
      
      print "\n";
      print "Store order$order ids-rownbr\n";
      my $rows = 0;
      while (<FOURTH_FILE>) {
         chomp;
         if (length() > 1) {
            $rows += 1;
            # Split the mid line, handling $midSepChar within strings
            # and removing "-chars from string values
            my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

            my $rowId = $.;
            my $orderXid = $midRow[ $idCol ];
            $orderXRowIds{$orderXid} = $rowId;
         }
      }
      print "Stored " . scalar(keys(%orderXRowIds)) . 
            " order$order ids-rownbr from $rows rows\n";
   }
   dbgprint "Stored " . scalar(keys(%orderXRowIds)) . " orderX ids-rownbr";
   print "\n";

   # Read the TA file, transport element belonging to area
   # and store the orderX = municipal for each item
   # (orderX is used as municipal in our mcm maps)
   my %location = ();
   if ( scalar @ARGV >= 3 ) {
      print "\n";
      print "Store location: transport element belonging to area\n";

      my $elemIdCol = "";
      my $elemTypeCol = "";
      my $areaIdCol = "";
      my $areaTypeCol = "";
      my $sideCol = "";
      my $theAreaTypeWeWant = "";
      # For checking if we have a header line in the manouvers path index file
      # to be able to skip it
      my $areaTypeString = "";
      my $sideOfLineString = "";
      if ( ($opt_v eq "TA_2010_06") ) {
         $elemIdCol = 0;
         $elemTypeCol = 1;
         $areaIdCol = 2;
         $areaTypeCol = 3;
         $sideCol = 4;
         $theAreaTypeWeWant = 1119; # area type, order8 = 1119
         $areaTypeString = "Aretyp";
         $sideOfLineString = "Sol";
      } else {
         die "Specify attribute parsing for $opt_v";
      }

      my $rows = 0;
      my $nbrWithLeftRightSide = 0;
      while (<THIRD_FILE>) {
         chomp;
         if (length() > 1) {
            $rows += 1;
            # Split the mid line, handling $midSepChar within strings
            # and removing "-chars from string values
            my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

            # side 0 = both sides = default
            if ( $midRow[ $sideCol ] eq $sideOfLineString ) {
               # header line, skip it
            } 
            elsif ( $midRow[ $sideCol ] != 0 ) {
               # The element has different admin areas on the right/left side
               # Let this method store only one area = the last one
               # in the $location hash for now.
               # FIXME: if this is a problem, fix me!
               #die "Need to handle element in area with sideOfLine=" .
               #    $midRow[ $sideCol ];
               $nbrWithLeftRightSide += 1;
            }

            # area type
            my $areaType = $midRow[ $areaTypeCol ];
            if ( $areaType eq $areaTypeString ) {
               # header line, skip it
            }
            elsif ( $areaType == $theAreaTypeWeWant ) {
               my $elemId = $midRow[ $elemIdCol ];
               my $areaId = $midRow[ $areaIdCol ];
               $location{$elemId} = $areaId;
            }
         }
         if ( ($rows % 100000) == 0 ) {
            dbgprint " read row $rows";
         }
      }
      dbgprint "Stored " . scalar(keys(%location)) . 
               " locations from $rows rows";
      dbgprint "There were $nbrWithLeftRightSide entries with sideOfLine " .
               "left or right";
   }
   dbgprint "Stored " . scalar(keys(%location)) .  " locations";
   print "\n";


   # misc things to calculate
   my @midIdsWithInvalidSpeedVec;
   my %speedValueOfInvalidSpeed;
   my $nbrLeftSettlementDefined = 0;
   my %leftSettlements;
   my $nbrRightSettlementDefined = 0;
   my $nbrLeftPostalZero = 0;
   my $nbrRightPostalZero = 0;
   my $nbrFerry = 0;
   my $nbrNoThroughfareFromPrivate = 0;
   my $nbrUnPaved = 0;
   my $nbrBorderNodes = 0;

   # Open Wayfinder ssi + ferry mid files
   open(WF_SSI_FILE, ">WFstreetSegmentItems.mid");
   open(WF_FERRY_FILE, ">WFferryItems.mid");
   # Define Wayfinder ssi + ferry mif files
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
   
   # hash with info for bifurcations
   # key = ssi mid id, value = vector with the mid ids of the nodes
   # of the ssi that has junctionType = bifurcation
   my %bifurcations = ();
   
   my $nbrMidItems = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 2) {

         $nbrMidItems += 1;

         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         
         my $midOrigId;
         my $midFeatType;        # MultiNet feature type
         my $midFerryType;
         my $midFromNodeID; my $midToNodeID;
         my $midFromNodeType; my $midToNodeType; # junction type
         my $midPJ;              # plural junction
         my $midLength;
         my $midFRC;
         my $midNetClass; my $midNetBClass; my $midNet2Class;
         my $midName;         # street name (complete)
         my $midNameLC;       # street name language code
         my $midSOL;          # side of line ???
         my $midNameType;     # street name type
         my $midCharge;
         my $midRouteNbr;     # route number on shield
         my $midRouteNbrType; # route number type
         my $midRouteDir;     # route directional
         my $midRouteDirValid; # route directional validity direction
         my $midProcStat;        # processing status
         my $midFOW;             # form of way
         my $midSlipRoad;
         my $midFreeway;
         my $midBackRoad;
         my $midToll;
         my $midRoadCond;
         my $midStubble;
         my $midPrivateRd;       # private road
         my $midConstrStatus;
         my $midOneWay;
         my $midFromBlocked;  # blocked passage
         my $midToBlocked;    # blocked passage
         my $midFromLevel; my $midToLevel;
         my $midKPH; my $midMinutes;
         my $midPosAcc;       # positional accuracy
         my $midCarriage; my $midNbrLanes;

         my $midRampType;
         my $midAdasFlag;
         my $midTransition;
         my $midDynamicSpeed;
         my $midSpeedCategory;
         my $midNoThroughTrafficAllowed;
         my $midRoughRoad;
         my $midPartOfStructure;
         
         
         # No values from supplier, but still used in parsing...
         # FIXME: if you want to have zip codes in the maps, you need to 
         # get zip code as attribute for the street segments from some table
         my $midLeftPostal = ""; my $midRightPostal = "";
         
         # Parse attributes
         if ( $opt_v eq "TA_2010_06" ) {
            $midOrigId = $midRow[0];
            $midFeatType = $midRow[1];
            $midFerryType = $midRow[2];
            $midFromNodeID = $midRow[3];
            $midFromNodeType = $midRow[4];
            $midToNodeID = $midRow[5];
            $midToNodeType = $midRow[6];
            $midPJ = $midRow[7];
            $midLength = $midRow[8];
            $midFRC = $midRow[9];
            $midNetClass = $midRow[10];
            $midNetBClass = $midRow[11];
            $midNet2Class = $midRow[12];
            $midName = $midRow[13];
            $midNameLC = $midRow[14];
            $midSOL = $midRow[15];
            $midNameType = $midRow[16]; # bitmask
            $midCharge = $midRow[17];
            $midRouteNbr = $midRow[18];
            $midRouteNbrType = $midRow[19];
            $midRouteDir = $midRow[20];
            $midRouteDirValid = $midRow[21];
            $midProcStat = $midRow[22];
            $midFOW = $midRow[23];
            $midSlipRoad = $midRow[24];
            $midFreeway = $midRow[25];
            $midBackRoad = $midRow[26];
            $midToll = $midRow[27];
            $midRoadCond = $midRow[28];
            $midStubble = $midRow[29];
            $midPrivateRd = $midRow[30];
            $midConstrStatus = $midRow[31];
            $midOneWay = $midRow[32];
            $midFromBlocked = $midRow[33];
            $midToBlocked = $midRow[34];
            $midFromLevel = $midRow[35];
            $midToLevel = $midRow[36];
            $midKPH = $midRow[37];
            $midMinutes = $midRow[38];
            $midPosAcc = $midRow[39];
            $midCarriage = $midRow[40];
            $midNbrLanes = $midRow[41];
   
            $midRampType = $midRow[42];
            $midAdasFlag = $midRow[43];
            $midTransition = $midRow[44];
            $midDynamicSpeed = $midRow[45];
            $midSpeedCategory = $midRow[46];
            $midNoThroughTrafficAllowed = $midRow[47];
            $midRoughRoad = $midRow[48];
            $midPartOfStructure = $midRow[49];
            
            
         } else {
            errprint "Implement attribute parsing for version $opt_v";
            die "exit";
         }

         my $midId = $.;
         checkMidId($midId, 0); # not parseonly die if check fails

	 if ($postalcodes{$midOrigId}) {
	     $midLeftPostal = $postalcodes{$midOrigId};
	     $midRightPostal = $postalcodes{$midOrigId};
	     #print "setting postal code for $midOrigId to $midLeftPostal.\n";
	 }


         if ($nbrMidItems == 1) {
            print " midId = $midId - max 4294967295, length midId=" .
                  length($midId) . "\n";
            #print " midId > max: " . ($midId > 4294967295) . "\n";
               
            print " midId=$midId roadFRC=$midFRC" .
                  " midOneWay=$midOneWay midKPH=$midKPH" .
                  " midName='$midName' \n" .
                  "\n";
         }
         

         # ssi or ferry or...
         my $isStreetSegment = "true";
         my $WFferryType = 0;
         if ( $midFeatType == 4110 ) {
            # ssi, ok
         } elsif ( $midFeatType == 4130 ) {
            # ferry
            $isStreetSegment = "false";
            if ( $midFerryType == 1 ) {   # by ship or hovercraft
               $WFferryType = 0;
            } elsif ( $midFerryType == 2 ) { # by train
               $WFferryType = 1;
            } else {
               die "Unhandled ferry type $midFerryType for $midId";
            }
         } elsif ( $midFeatType == 4165 ) {
            # dunno. tolerate "address area boundary element"
         } else {
            die "unhandled feature type $midFeatType for $midId";
         }

         
         # Build item name, $midName
         my $itemName = "";
         my $nameLang = "";
         my $nameType = "";
         if (length($midName) > 0) {
            $itemName = "$midName";
         }
         if (length($midNameLC) > 0) {
            $nameLang = getWFNameLanguage ( $midNameLC, $midId );
         }
         if ( (length($midNameType) > 0) and $midNameType ) {
            $nameType = getWFNameTypeFromBitMask ( $midNameType, $midId );
         }

         # Build allNames from itemName and route number
         my $allNames = "";
         if (length($itemName) > 0) {
            #print "names $midId itemName=$itemName nameLang=$nameLang " .
            #      "nameType=$nameType midRouteNbr=$midRouteNbr\n";
            if ( length $nameLang == 0 ) {
               die "no name lang for itemName for $midId";
            }
            if ( length $nameType == 0 ) {
               die "no name type for itemName for $midId";
            }
            $allNames = "$itemName:$nameType:$nameLang";
         }
         if ( length($midRouteNbr) > 0 ) {
            my $tmp = "";
            if (length($allNames) > 0) {
               $tmp = " ";
            }
            $allNames = "$allNames$tmp" .
                        "$midRouteNbr:roadNumber:invalidLanguage";
         }

         # Road class (default Wayfinder road class is 4)
         # using Net2Class combined with FRC
         my $WFroadClass = 4;
         if ( (length($midNet2Class) > 0) and (length($midFRC) > 0) ) {
            if ( $midNet2Class == 0 ) {
               $WFroadClass = 0;
            }
            elsif ( $midNet2Class == -1 ) {
		warn "found net2class = -1 in midid $midId.\n";
                $WFroadClass = 1;
            }
            elsif ( $midNet2Class == 1 ) {
               $WFroadClass = 1;
            }
            elsif ( $midNet2Class == 2 ) {
               if ( $midFRC <= 2 ) {
                  $WFroadClass = 1;
               } else {
                  $WFroadClass = 2;
               }
            }
            elsif ( $midNet2Class == 3 ) {
               if ( $midFRC <= 3 ) {
                  $WFroadClass = 2;
               } else {
                  $WFroadClass = 3;
               }
            }
            elsif ( $midNet2Class == 4 ) {
               if ( $midFRC <= 6 ) {
                  $WFroadClass = 3;
               } else {
                  $WFroadClass = 4;
               }
            }
            elsif ( ($midNet2Class == 5) or
                    ($midNet2Class == 6) or
                    ($midNet2Class == 7) ) {
               $WFroadClass = 4;
            }
            else {
               errprint "Unknown road net2class $midNet2Class " .
                        "(frc=$midFRC net=$midNetClass netB=$midNetBClass) " .
                        "for midId $midId";
               die "exit";
            }
         } else {
            print "WARN item $midId with undefined net2class and/or frc\n";
            die "exit";
         }

         # Speed limit (use default Wayfinder speed)
         my $WFposSpeed = $defaultWFspeed;
         my $WFnegSpeed = $defaultWFspeed;
         if ( (length($midKPH) > 0) and ($midKPH > 0) ) {
            $WFposSpeed = $midKPH;
            $WFnegSpeed = $midKPH;
         } else {
            $speedValueOfInvalidSpeed { $midId } = $midKPH;
         }

         # Entry restrictions (default is 0=noRestrictions)
         my $WFposEntryRestr = 0;
         my $WFnegEntryRestr = 0;
         if ( length($midOneWay) > 0 ) {
            if ( $midOneWay eq "FT" ) { # Open in postive direction
               $WFnegEntryRestr = 3;
            }
            elsif ( $midOneWay eq "TF" ) { # Open in negative direction
               $WFposEntryRestr = 3;
            }
            elsif ( $midOneWay eq "N" ) { # Closed both directions
               $WFposEntryRestr = 3;
               $WFnegEntryRestr = 3;
            }
            else {
               die "Unhandled midOneWay=$midOneWay for $midId";
            }
         }
         # Set noThroughfare from private/public attribute
         if ( $midPrivateRd == 2 ) {
            $WFnegEntryRestr = 1;
            $WFposEntryRestr = 1;
            $nbrNoThroughfareFromPrivate += 1;
         } elsif ($midPrivateRd == 0) {
            # ok
         } else {
            errprint "unhandled private value $midPrivateRd for $midId";
            die "exit";
         }

         # Number lanes
         my $WFnumberLanes = "";
         if ( length $midNbrLanes > 0 ) {
            $WFnumberLanes = $midNbrLanes;
         }

         # House numbers from house_numbers file
         my $WFleftStart = 0;
         my $WFleftEnd = 0;
         my $WFrightStart = 0;
         my $WFrightEnd = 0;
         if ( defined($hnbrs{$midId}) ) {
            $WFleftStart =  $hnbrs{$midId}->{m_leftStart};
            $WFleftEnd =    $hnbrs{$midId}->{m_leftEnd};
            $WFrightStart = $hnbrs{$midId}->{m_rightStart};
            $WFrightEnd =   $hnbrs{$midId}->{m_rightEnd};
         }
         
         # Level node0 and node1
         my $WFlevel0 = 0;
         my $WFlevel1 = 0;
         if ( $midFromLevel and (length($midFromLevel) > 0) ) {
            $WFlevel0 = $midFromLevel;
         }
         if ( $midToLevel and (length($midToLevel) > 0) ) {
            $WFlevel1 = $midToLevel;
         }

         # Toll road
         my $WFtoll = "N";

         if ( length $midToll > 0 ) {
            if ( ($midToll eq "11") ) {
                $WFtoll = "Y";
            } elsif ( ($midToll eq "12") ) {
            } elsif ( ($midToll eq "13") ) {
            } elsif ( ($midToll eq "21") ) {
                $WFtoll = "Y";
            } elsif ( ($midToll eq "22") ) {
            } elsif ( ($midToll eq "23") ) {
            } elsif ( ($midToll eq "B") or 
                 ($midToll eq "FT") or
                 ($midToll eq "TF") ) {
                $WFtoll = "Y"; # hack - apparently, midCharge and midToll are being confused, e.g. in itai16.
            } else {
               die "unhandled midToll $midToll for $midId";
	    }
         }
         if ( length $midCharge > 0 ) {
            if ( ($midCharge eq "B") or 
                 ($midCharge eq "FT") or
                 ($midCharge eq "TF") ) {
               $WFtoll = "Y";
            } else {
               die "unhandled midCharge $midCharge for $midId";
            }
         }



         # Road condition
         my $WFpaved = "Y";
         if ( ($midRoadCond == 2) or ($midRoadCond == 3) ) {
            $WFpaved = "N";
            $nbrUnPaved += 1;
         }
         
         # Misc Wayfinder attributes from the FOW attribute
         my $WFcontrolledAccess = "N";
         my $WFdivided = "N";
         my $WFmulti = "N";
         my $WFramp = "N";
         my $WFroundabout = "N";
         my $WFroundaboutish = "N";
         my $WFroadDisplayClass = -1;
         if ( length($midFOW) > 0 && $midFOW != -1 ) {
            if ($midFOW == 1 ) {
               $WFcontrolledAccess = "Y";
            } elsif ($midFOW == 2 ) {
               $WFmulti = "Y";
            } elsif ($midFOW == 4 ) {
               $WFroundabout = "Y";
            } elsif ($midFOW == 6 ) {
               $WFroadDisplayClass = 5; # etaParkingPlace
            } elsif ($midFOW == 7 ) {
               $WFroadDisplayClass = 4; # etaParkingGarage
            } elsif ($midFOW == 8 ) {
               $WFroadDisplayClass = 6; # etaUnstructuredTrafficSquare
            } elsif ($midFOW == 10 ) {
               $WFramp = "Y";
            } elsif ($midFOW == 11 ) {
               $WFroadDisplayClass = 7; # partOfServiceRoad
            } elsif ($midFOW == 12 ) {
               $WFroadDisplayClass = 3; # entranceExitCarPark
            } elsif ($midFOW == 14 ) {
               $WFroadDisplayClass = 1; # partOfPedestrianZone
            } elsif ($midFOW == 15 ) {
               $WFroadDisplayClass = 0; # partOfWalkway
            } elsif ($midFOW == 17 ) {
               $WFroundaboutish = "Y";
            } elsif ($midFOW == 20 ) {
               $WFroadDisplayClass = 2; # roadForAuthorities
            } elsif ( ($midFOW == 3)  # single carriageway
                  ) {
                  # ok
            } elsif ( ($midFOW == 18 || $midFOW == 19)  # had to be added for TA 2011-12. dunno what that is.
                  ) {
                  # ok
            } else {
               errprint "Unknown FOW $midFOW for midId $midId";
               die "exit";
            }
         }

         # Zip codes
         my $postalCodeOK = 1;
         if ( $midLeftPostal eq "0" ) {
            $nbrLeftPostalZero += 1;
            $midLeftPostal = "";
            $postalCodeOK = 0;
         }
         if ( $midRightPostal eq "0" ) {
            $nbrRightPostalZero += 1;
            $midRightPostal = "";
            $postalCodeOK = 0;
         }

         # Get settlement ids from the %location and %orderXRowIds hash
         my $midLeftSettle = "";
         my $midRightSettle = "";
         if ( defined $location{$midOrigId} ) {
            my $orderXid = $location{$midOrigId};
            if ( defined $orderXRowIds{$orderXid} ) {
               $midLeftSettle = $orderXRowIds{$orderXid};
               $midRightSettle = $orderXRowIds{$orderXid};
            }
         }
         
         # Check left|right settlement..
         # If settlement is 0, use some kind of invalid mark. ?
         my $WFleftSettle = "";
         my $WFrightSettle = "";
         my $WFsettleOrder = "";      # Order 9 = city parts..
         if ( $opt_v eq "TA_2010_06" ) {
            $WFsettleOrder = "8";   # 8 means municipal
         } else {
            die "Implement WFsettleOrder for version $opt_v\n";
         }
         if ( defined($midLeftSettle) and (length($midLeftSettle) > 0) ) {
            my $tmp = $midLeftSettle;
            $tmp =~ s/[0-9]//g;
            if ( length($tmp) > 0 ) {
               print " $midId midLeftSettle not digits '$midLeftSettle'\n";
            } elsif ($midLeftSettle > 0) {
               $WFleftSettle = $midLeftSettle;
               $nbrLeftSettlementDefined +=1;
               if ( defined ( $leftSettlements { $midLeftSettle } ) ) {
                  $leftSettlements { $midLeftSettle } += 1;
               } else {
                  $leftSettlements { $midLeftSettle } = 1;
               }
            }
         }
         if ( defined($midRightSettle) and (length($midRightSettle) > 0) ) {
            my $tmp = $midRightSettle;
            $tmp =~ s/[0-9]//g;
            if ( length($tmp) > 0 ) {
               print " $midId midRightSettle not digits '$midRightSettle'\n";
            } elsif ($midRightSettle > 0) {
               $WFrightSettle = $midRightSettle;
               $nbrRightSettlementDefined +=1;
            }
         }

         # Node type = junctiontype gives borderNodes and bifurcations
         my $node0Border = "";
         my $node1Border = "";
         if ( $midFromNodeType == 4 ) {
            # border node (country border crossing)
            $node0Border = "Y";
            $nbrBorderNodes += 1;
         } elsif ($midFromNodeType == 2) {
            # bifurcation
            push @{$bifurcations{$midId}}, $midFromNodeID;
            #print "bifurcation for midId $midId node0 $midFromNodeID\n";
         } elsif ($midFromNodeType != 0) {
            if ( $midFromNodeType == 3|| $midFromNodeType == 5 || $midFromNodeType == 6 ){
               # ok
            } else {
               die "ERROR: Handle node0 junction type $midFromNodeType for midId=$midId";
            }
         }
         if ( $midToNodeType == 4 ) {
            # border node (country border crossing)
            $node1Border = "Y";
            $nbrBorderNodes += 1;
         } elsif ($midToNodeType == 2) {
            # bifurcation
            push @{$bifurcations{$midId}}, $midToNodeID;
            #print "bifurcation for midId $midId node1 $midToNodeID\n";
         } elsif ($midToNodeType != 0) {
            if ( $midToNodeType == 3 || $midToNodeType == 5 || $midToNodeType == 6 ){
               # ok
            } else {
               die "ERROR: Handle node1 junction type $midToNodeType for midId=$midId";
            }
         }


         # FIXME handle blocked passage in nodes
         # (might be solved in handleRestrictions reading the maneuvers file
         #  but uncertain about that)
         if ( $midFromBlocked != 0 ) {
            #die "ERROR: Handle node0 blockedPassage $midFromBlocked for midId=$midId";
         }
         if ( $midToBlocked != 0 ) {
            #die "ERROR: Handle node1 blockedPassage $midToBlocked for midId=$midId";
         }



         # Separate ssi from ferry, both mid and mif file
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
                              ",$WFroadDisplayClass";
            print WF_SSI_FILE "\n";
         } else {
            $nbrFerry += 1;
            print WF_FERRY_FILE "$midId,\"$itemName\",\"$allNames\"" .
                              ",$WFroadClass" .
                              ",$WFposSpeed,$WFnegSpeed" .
                              ",$WFposEntryRestr,$WFnegEntryRestr" .
                              ",$WFlevel0,$WFlevel1" .
                              ",\"$WFtoll\",$WFferryType" .
                              "\n";
         }

         # Read next Pline/Line from mif file and write to correct WF mif
         my $wanted = 1;
         my $outFileName = "_invalidMifFileName";
         if ( $isStreetSegment eq "true" ) {
             $outFileName = $outSSIFileName;
         } else {
             $outFileName = $outFerryFileName;
         }
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outFileName, $wanted, $midId );
         
         
         # Set no access for vehicles ???
         # (ok for pedestrians (and bicyclists?))
         # XXX Want 0x88
#         if ( $midMotorable eq "N" ) {
#            print WF_RESTR_FILE "$.\t-1\t-1\t$midId\t-1\t0\t0\n";
#            $nbrRestrictions += 1;
#         }

      }
   
   }

   # FIXME: Sort out the bifurcations and write info to the turntable
   # If it is not solved by handleRestrictions, reading the maneuvers file
   if ( scalar (keys %bifurcations) > 0 ) {
      print "\nNeed to handle bifurcations for " . 
            scalar (keys %bifurcations) . " nodes!\n";
      if ( $opt_v eq "TA_2010_06" ) {
         # ok, skip for now, needs FIXME
      }
      else {
         die "exit";
      }
   }
   
   # Result
   print "\n=======================================================\n";
   dbgprint "Network has $nbrMidItems mid items.";
   print "There are " .  scalar( keys (%speedValueOfInvalidSpeed) ) . 
         " segments with invalid speed:\n";
   foreach my $r ( sort (keys (%speedValueOfInvalidSpeed)) ) {
      print " midId $r = \"" . $speedValueOfInvalidSpeed{ $r } . "\"\n";
   }

   print "There are $nbrLeftSettlementDefined roads in " .
         keys(%leftSettlements) . " different left settlements.\n";
   print "There are $nbrRightSettlementDefined roads with right settlement.\n";
   print "Postal code zero left=$nbrLeftPostalZero right=$nbrRightPostalZero\n";
   print "Nbr noThroughfare due to private road = " .
         "$nbrNoThroughfareFromPrivate\n";
   print "Nbr un-paved roads = $nbrUnPaved\n";
   print "Nbr borderNodes = $nbrBorderNodes\n";
   
   print "Make sure that there are no mid replace chars \"$midRplChar\" " .
         "in the resulting ssi mid file.\n";

   if ( $nbrRestrictions > 0 ) {
      print "Wrote $nbrRestrictions restrictions to turn table. " .
            "Merge with turn table created from restrictions file.\n";
   }

   if ( $nbrFerry > 0 ) {
      print "Number ferry=$nbrFerry\n";
   }
   
} # handleNetworkFile



# Translate POI extra attribtues attribute type to
# poi_wasp log strings, so we can use the functions
# in the POIObject myAddGDFPOIAttribute
sub getInfoStringForPIEA {
   my $attrType = $_[0];
   my $attrVal = $_[1];
   my $poiInfoFieldSeparator = $_[2];
   
   my $infoString = undef;
   my $useEA = 0;
   
   # 1J park classification
   # 5P Public Transport Stop Location Type
   # 6F facilities
   # 6H place of worship type
   # 6J Tourist Attraction Type
   # 8J Parking Garage Construction Type
   # 8Q Truck Stop
   # 9F food type
   # FT Ferry type
   # RY Railway Station Type
   
   # FIXME: go through these and set useEA=1 if they are useful
   # 6A Geocoding accuracy level
   # 7H 24 hour service
   # 7Q Quality mark
   # 8K parking size
   # 9G Government Type
   # 9M major road feature
   # 9P park and ride facility
   # BQ bus stop type (terminal or other, see POIObject BUS_STOP_TYPE)

   # Already included in the PI base file
   # TL telephone number
   # 8L http
   # 5M main postal code
   # 8M email
   # AP positional accuracy
   
   if ( ($attrType eq "Atttyp") and ($attrVal eq "Attvalue") ) {
      # header line of PIEA file, skip
      $useEA = 0;
   }
   # the ones to FIXME
   elsif ( ($attrType eq "6A") or ($attrType eq "7H") or
           ($attrType eq "7Q") or ($attrType eq "8K") or
           ($attrType eq "9G") or ($attrType eq "9M") or
           ($attrType eq "9P") or ($attrType eq "BQ") ) {
      $useEA = 0;
   }
   # the ones to skip beacuse already included as attributes in the PI file
   elsif ( ($attrType eq "TL") or ($attrType eq "8L") or
           ($attrType eq "5M") or ($attrType eq "8M") or
           ($attrType eq "AP") ) {
      $useEA = 0;
   }

   # 1J park classification
   elsif ( $attrType eq "1J" ) {
      $useEA = 0;
   }
   # 5P Public Transport Stop Location Type
   elsif ( $attrType eq "5P" ) {
      $infoString = "4: PUBLIC_TRANSPORT_STOP_TYPE" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # 6F facilities (LPG, petrol, diesel, CNG)
   elsif ( $attrType eq "6F" ) {
      $useEA = 0;
   }
   # 6H place of worship type
   elsif ( $attrType eq "6H" ) {
      $infoString = "4: PLACE_OF_WORSHIP_TYPE" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # 6J Tourist Attraction Type
   elsif ( $attrType eq "6J" ) {
      $infoString = "4: TOURIST_ATTRACTION_TYPE" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # height of mountain peak
   elsif ( $attrType eq "6P" ) {
      $useEA = 0;
   }
   # 8J Parking Garage Construction Type
   elsif ( $attrType eq "8J" ) {
      $useEA = 0;
   }
   # 8U Rest Area Facilities
   elsif ( $attrType eq "8U" ) {
      $infoString = "4: REST_AREA_FACILITIES" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # 8Q Truck Stop
   elsif ( $attrType eq "8Q" ) {
      $useEA = 0;
   }
   elsif ( $attrType eq "9F" ) {
      $infoString = "4: FOOD_TYPE" . $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # AD Departure / Arrival
   elsif ( $attrType eq "AD" ) {
      $infoString = "4: DEPARTURE_ARRIVAL" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # FT Ferry type
   elsif ( $attrType eq "FT" ) {
      $infoString = "4: FERRY_TYPE" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # Domestic / International
   elsif ( $attrType eq "NI" ) {
      $useEA = 0;
   }
   # Passport control
   elsif ( $attrType eq "PU" ) {
      $infoString = "4: POLICE_STATION_TYPE" . 
                     $poiInfoFieldSeparator . "Passport control";
      $useEA = 0;
   }
   # RY Railway Station Type
   elsif ( $attrType eq "RY" ) {
      $infoString = "4: RAILWAY_STATION_TYPE" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   elsif ( $attrType eq "NG" ) {
      $infoString = "4: RAILWAY_STATION_TYPE" . 
                     $poiInfoFieldSeparator . "$attrVal";
      $useEA = 1;
   }
   # Park Type - national park
   elsif ( $attrType eq "PT" ) {
      $useEA = 0;
   }
   # 7V - 2 Validity direction: Valid in Positive Line Direction. ignored.
   elsif ( $attrType eq "7V" ) {
      $useEA = 0;
   }
   # HP - height of peak. ignored for now.
   elsif ( $attrType eq "HP" ) {
      $useEA = 0;
   }
   # ER - emergency room
   elsif ( $attrType eq "ER" ) {
      $useEA = 0;
   }
   else {
      die "Handle getInfoStringForPIEA for " .
          "attrType = $attrType attrVal=$attrVal";
   }

   return ( $infoString, $useEA );
}

sub readPoiExtAttributes {
   dbgprint "readPoiExtAttributes";
   
   if ( scalar @ARGV < 3) {
      return;
   }
   
   my $idCol = undef;
   my $featTypeCol = undef;
   my $attrTypeCol = undef;
   my $attrValCol = undef;

   if ( $opt_v eq "TA_2010_06" ) {
      $idCol = 0;
      $featTypeCol = 1;
      $attrTypeCol = 2;
      $attrValCol = 3;
   } else {
      die "Specify attribute parsing for $opt_v";
   }

   %m_poiEA = ();
   my %attrTypes = ();

   my $poiObject = POIObject->newObject();
   my $poiInfoFieldSeparator = $poiObject->getFieldSeparator();
   my $rows = 0;
   while (<THIRD_FILE>) {
      chomp;
      if (length() > 1) {
         $rows += 1;
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

         my $id = $midRow[ $idCol ];
         my $poiId = "poi" . $id;

         my $featType =  $midRow[ $featTypeCol ];
         my $attrType =  $midRow[ $attrTypeCol ];
         my $attrVal =   $midRow[ $attrValCol ];

         (my $infoStr, my $useEA ) = 
                  getInfoStringForPIEA( $attrType, $attrVal, 
                                        $poiInfoFieldSeparator );
         if ( $useEA ) {
            if ( ! defined $infoStr ) {
               die "Handle getInfoStringForPIEA for attrType = $attrType";
            }
            push @{$m_poiEA{$poiId}}, "$infoStr";
         }
         $attrTypes{$attrType} = $useEA;
         

      }
   }
   dbgprint "Read $rows rows from PIEA file";

   # Loop and print all attribute types
   dbgprint "These attribute types:";
   foreach my $at (sort (keys (%attrTypes)) ) {
      print "attrType = $at\n";
   }
   print "\n";
   
} # readPoiExtAttributes

sub addExtAttributes {
   my $poi = $_[0];
   my $eaInfoString = $_[1];


   #print " poi " . $poi->getSourceReference() .
   #      " eaInfoString = $eaInfoString\n";

   my $retVal = $poi->myAddGDFPOIAttribute( 
                     $eaInfoString , $poi->getSourceReference() );
   if ( ! $retVal ) {
      die "addExtAttributes: myAddGDFPOIAttribute exit";
   }

   return $retVal;

}



#
# Handle POI file, write to CPIF
sub handlePoiFile {
   dbgprint "handlePoiFile";

   # Define attribute columns, 0,1,2,...
   my $midIdCol = undef;
   my $featureTypeCol = undef;
   my $impCol = undef;
   my $nameLcCol = undef;
   my $nameCol = undef;
   my $streetNameCol = undef;
   my $streetNameLcCol = undef;
   my $hnbrCol = undef;
   my $postCodeCol = undef;
   my $order8IdCol = undef;   # municipal identification
   my $order8CodeCol = undef;
   my $order8NameCol = undef;
   my $buaNameCol = undef;
   my $phoneCol = undef;
   my $faxCol = undef;
   my $emailCol = undef;
   my $urlCol = undef;
   my $compNameCol = undef;
   my $roadIdCol = undef;
   my $relPosCol = undef;
   my $extPoiIdCol = undef; # external poi identification
   my $addrPointCol = undef;

   if ( $opt_v eq "TA_2010_06" ) {
      $midIdCol = 0;
      $featureTypeCol = 1;
      $impCol = 2;
      $nameLcCol = 3;
      $nameCol = 4;
      $streetNameCol = 5;
      $streetNameLcCol = 6;
      $hnbrCol = 7;
      $postCodeCol = 8;
      $order8IdCol = 9;
      $order8CodeCol = 10;
      $order8NameCol = 11;
      $buaNameCol = 12;
      $phoneCol = 13;
      $faxCol = 14;
      $emailCol = 15;
      $urlCol = 16;
      $compNameCol = 17;
      $roadIdCol = 18;
      $relPosCol = 19;
      $extPoiIdCol = 20;
      $addrPointCol = 21; 
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   
   if ( scalar @ARGV >= 3) {
      readPoiExtAttributes();
   }
   
   my $adminClassCol = undef;
   my $displClassCol = undef;

   handleGenPoiFile( "poi",
                     "",         # no given POI type
                     $midIdCol,
                     $featureTypeCol,
                     $impCol,
                     $nameLcCol,
                     $nameCol,
                     $streetNameCol,
                     $streetNameLcCol,
                     $hnbrCol,
                     $postCodeCol,
                     $order8IdCol,
                     $order8CodeCol,
                     $order8NameCol,
                     $buaNameCol,
                     $phoneCol,
                     $faxCol,
                     $emailCol,
                     $urlCol,
                     $compNameCol,
                     $roadIdCol,
                     $relPosCol,
                     $extPoiIdCol,
                     $addrPointCol,
                     $adminClassCol,
                     $displClassCol
                     );
} # handlePoiFile

# Handle City centre POI file, write to CPIF
sub handleCCFile {
   dbgprint "handleCCFile";

   # Define attribute columns, 0,1,2,...
   my $midIdCol = undef;
   my $featureTypeCol = undef;
   my $impCol = undef;
   my $nameLcCol = undef;
   my $nameCol = undef;
   my $streetNameCol = undef;
   my $streetNameLcCol = undef;
   my $hnbrCol = undef;
   my $postCodeCol = undef;
   my $order8IdCol = undef;
   my $order8CodeCol = undef;
   my $order8NameCol = undef;
   my $buaNameCol = undef;
   my $phoneCol = undef;
   my $faxCol = undef;
   my $emailCol = undef;
   my $urlCol = undef;
   my $compNameCol = undef;
   my $roadIdCol = undef;
   my $relPosCol = undef;
   my $extPoiIdCol = undef;
   my $addrPointCol = undef;
   
   my $adminClassCol = undef;
   my $displClassCol = undef;
   
   if ( ($opt_v eq "TA_2010_06") ) {
      $midIdCol = 0;
      $featureTypeCol = 1;
      $nameLcCol = 2;
      $nameCol = 3;
      $postCodeCol = 15;
      $buaNameCol = 17;
      $roadIdCol = 18;
      $relPosCol = 19;
      $addrPointCol = 20;
      
      $adminClassCol = 4;
      $displClassCol = 5;
   }
   else {
      die "Specify attribute parsing for $opt_v";
   }
   
   
   handleGenPoiFile( "cc",
                     11,            # given poi type = city centre
                     $midIdCol,
                     $featureTypeCol,
                     $impCol,
                     $nameLcCol,
                     $nameCol,
                     $streetNameCol,
                     $streetNameLcCol,
                     $hnbrCol,
                     $postCodeCol,
                     $order8IdCol,
                     $order8CodeCol,
                     $order8NameCol,
                     $buaNameCol,
                     $phoneCol,
                     $faxCol,
                     $emailCol,
                     $urlCol,
                     $compNameCol,
                     $roadIdCol,
                     $relPosCol,
                     $extPoiIdCol,
                     $addrPointCol,
                     $adminClassCol,
                     $displClassCol
                     );
} # handleCCFile


# General function that handles misc POI files,
# write to CPIF (use POI object)
sub handleGenPoiFile {
   my $fileType = $_[0];
   my $givenWFpoiTypeID = $_[1];  # given POI type
   my $midIdCol = $_[2];
   my $featureTypeCol = $_[3];
   my $impCol = $_[4];
   my $nameLcCol = $_[5];
   my $nameCol = $_[6];
   my $streetNameCol = $_[7];
   my $streetNameLcCol = $_[8];
   my $hnbrCol = $_[9];
   my $postCodeCol = $_[10];
   my $order8IdCol = $_[11];
   my $order8CodeCol = $_[12];
   my $order8NameCol = $_[13];
   my $buaNameCol = $_[14];
   my $phoneCol = $_[15];
   my $faxCol = $_[16];
   my $emailCol = $_[17];
   my $urlCol = $_[18];
   my $compNameCol = $_[19];
   my $roadIdCol = $_[20];
   my $relPosCol = $_[21];
   my $extPoiIdCol = $_[22];
   my $addrPointCol = $_[23];
   
   my $adminClassCol = $_[24];
   my $displClassCol = $_[25];

   dbgprint "handleGenPoiFile: $fileType";

   #$m_dbh = db_connect();	# SB!!!
   
   # Define out file name, open to empty any old file, then it is
   # written with append.
   my $wfFileName = "WFcpif_${fileType}_${opt_c}.txt";
   open(WF_CPIF_FILE, ">$wfFileName");
   print WF_CPIF_FILE "";
   
   # Read the mif file and store poi coordinates in coordinate hashes
   my ($latitudes, $longitudes) = 
      readMifPointFile( $SECOND_FILE_NAME, $m_coordOrder );
   

   # Read the mid file
   my $nbrPois = 0;
   my $nbrUnwantedPOIs = 0;
   my $nbrPOIsToCPIF = 0;
   my $nbrPOIsNoName = 0;
   my %poiTypeStat = ();
   my %importanceStat = ();
   my %WFccDC = ();
   my $nbrPOIsWithEA = 0;
   my $nbrEAadded = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 2) {
      
         $nbrPois += 1;

         # Split the midRow, handling $midSepChar within strings
         # also removing "-chars from string values
         my @poiRecord =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         # Get midId, 
         # ok to keep the long id string for pois
         my $midId = $poiRecord[$midIdCol];
         $midId = "$fileType$midId";

         # POI official name
         my $poiName = $poiRecord[$nameCol];
         
         # POI importance
         my $poiImportance = "";
         if ( defined $impCol and (length $impCol > 0) and
              $poiRecord[$impCol] ) {
            $poiImportance = $poiRecord[$impCol];
         }

         # Get wfPoiTypeId for this poi. Either it is given with
         # inparam or needs to be translated from feature category
         my $wfPoiTypeId = 95; # invalid
         if ( defined $givenWFpoiTypeID and
              (length($givenWFpoiTypeID) > 0) ) {
            $wfPoiTypeId = $givenWFpoiTypeID;
         } else {
            $wfPoiTypeId  = getWayfinderPoiTypeId(
                  $poiRecord[$featureTypeCol], $midId, $poiName,
                  $poiImportance );
         }


         # Statistics
         if ( defined $impCol ) {
            if ( defined $importanceStat{$poiRecord[$impCol]} ) {
               $importanceStat{$poiRecord[$impCol]} += 1;
            } else {
               $importanceStat{$poiRecord[$impCol]} = 1;
            }
         }


         # Only work with POI if poi type is not -1 (unwanted pois..)
         if ( $wfPoiTypeId < 0 ) {
            $nbrUnwantedPOIs += 1;
         } 
         else {
            
            # Get poi coordinate from mif file hash, and convert to mc2
            (my $lat, my $lon) = 
               convertFromWgs84ToMc2(  $latitudes->{ $nbrPois },
                                       $longitudes->{ $nbrPois } );
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
            #$poiObject->setComment(
            #   "featureCode=" . $poiRecord[$featureTypeCol]);

            # POITypes
            $poiObject->addPOIType( $wfPoiTypeId );

            # POINames
            # official name
            if ( length $poiName > 0 ) {
               my $nameLangId =
                  getWFNameLanguageID ( $poiRecord[ $nameLcCol ], 
                                        $midId, $poiName );
               $poiObject->addName( $poiName, 0, $nameLangId );
            } else {
               #print "WARN: poi $midId with no name\n";
               $nbrPOIsNoName += 1;
            }
            
            # POIInfo
            # 0 | Vis. address
            if ( defined $streetNameCol and (length $streetNameCol > 0) and
                 $poiRecord[$streetNameCol] ) {
               my $nameLangId = getWFNameLanguageID (
                     $poiRecord[ $streetNameLcCol ], $midId );
               $poiObject->addPOIInfo(
                  0, $nameLangId, $poiRecord[$streetNameCol] ); }
            # 1 | Vis. house nbr
            if ( defined $hnbrCol and (length $hnbrCol > 0) and
                 $poiRecord[$hnbrCol] ) {
               $poiObject->addPOIInfo( 1, 31, $poiRecord[$hnbrCol] ); }
            # 2 | Vis. zip code
            if ( defined $postCodeCol and (length $postCodeCol > 0) and
                 $poiRecord[$postCodeCol] ) {
               $poiObject->addPOIInfo( 2, 31, $poiRecord[$postCodeCol] ); }
            # 4 | Phone
            if ( defined $phoneCol and (length $phoneCol > 0) and
                 $poiRecord[$phoneCol] ) {
               $poiObject->addPOIInfo( 4, 31, $poiRecord[$phoneCol] ); }
            #  5 | Vis. zip area (bua or order8 name)
            if ( defined $buaNameCol and (length($buaNameCol) > 0) and
                 $poiRecord[$buaNameCol] and
                 (length $poiRecord[$buaNameCol] > 0) ) {
               $poiObject->addPOIInfo( 5, 31, $poiRecord[$buaNameCol] );
            } elsif ( defined $order8NameCol and
                      (length($order8NameCol) > 0) and
                      $poiRecord[$order8NameCol] and
                      (length $poiRecord[$order8NameCol] > 0) ) {
               $poiObject->addPOIInfo( 5, 31, $poiRecord[$order8NameCol] ); }
            # 7 | Fax
            if ( defined $faxCol and (length $faxCol > 0) and
                 $poiRecord[$faxCol] ) {
               $poiObject->addPOIInfo( 7, 31, $poiRecord[$faxCol] ); }
            # 8 | Email
            if ( defined $emailCol and (length $emailCol > 0) and
                 $poiRecord[$emailCol] ) {
               $poiObject->addPOIInfo( 8, 31, $poiRecord[$emailCol] ); }
            # 9 | Url
            if ( defined $urlCol and (length $urlCol > 0) and
                 $poiRecord[$urlCol] ) {
               $poiObject->addPOIInfo( 9, 31, $poiRecord[$urlCol] ); }
            # 87 | Importance
            if ( defined $impCol and (length $impCol > 0) and
                 $poiRecord[$impCol] ) {
               $poiObject->addPOIInfo( 87, 31, $poiRecord[$impCol] ); }
            # cc display class
            if ( $wfPoiTypeId == 11 ) {
               my $ccDispClass = 12; # default
               if ( defined $displClassCol and defined $adminClassCol ) {
                  $ccDispClass = getCCdispClass(
                     $poiRecord[$displClassCol], $poiRecord[$adminClassCol],
                     $midId, $poiRecord[$nameCol] );
               } else {
                  errprint "displClassCol or adminClassCol " .
                           "not defined for $midId";
                  die "exit";
               }
               $poiObject->addPOIInfo( 47, 31, $ccDispClass );
               if ( defined $WFccDC{$ccDispClass} ) {
                  $WFccDC{$ccDispClass} += 1;
               } else {
                  $WFccDC{$ccDispClass} = 1;
               }
            }

            # POI Extra attributes
            if ( defined $m_poiEA{$midId} ) {
               $nbrPOIsWithEA +=1;
               foreach my $eaInfoString ( @{$m_poiEA{$midId}} ) {
                  
                  my $retVal = addExtAttributes( $poiObject, $eaInfoString );
                  if ( $retVal ) {
                     $nbrEAadded += 1;
                  }
               }
            }
            
            # Statistics abt WF POI type
            # The POI type may have changed in handling of the extra 
            # attributes, e.g. church type, public transport stop type
            # The POIObject really only has ONE type, so get the first one
            my $finalPT = undef;
            foreach my $poiType ($poiObject->getPOITypes()){
               if ( ! defined $finalPT ) {
                  $finalPT = $poiType;
               }
            }
            if ( defined $poiTypeStat{$finalPT} ) {
               $poiTypeStat{$finalPT} += 1;
            } else {
               $poiTypeStat{$finalPT} = 1;
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
   dbgprint "Read $nbrPois pois/cc from mid file";
   if ( scalar (keys %importanceStat) > 0 ) {
      dbgprint "Number pois with importance";
      foreach my $c (sort (keys (%importanceStat)) ) {
         print "  '$c' $importanceStat{$c}\n";
      }
   }
   if ( scalar (keys %m_unknownPOIcategories) > 0 ) {
      errprint "We have unknown poi categories - edit parse script!";
      foreach my $r ( sort (keys (%m_unknownPOIcategories)) ) {
         print "  poi category $r has "
               . $m_unknownPOIcategories{ $r } . " pois\n";
      }
      errprint "add these poi categories in GDFPoiFeatureCodes\n" .
      die "exit";
   }
   
   print "\n";
   print "Number POIs with info from extra attributes = $nbrPOIsWithEA\n";
   print "Totally handled $nbrEAadded infos from extra attributes\n";
   
   print "\nNot using $nbrUnwantedPOIs unwanted pois\n" .
         "Wrote $nbrPOIsToCPIF pois to CPIF $wfFileName" .
         " (of which $nbrPOIsNoName has no name)\n";
   
   if ( scalar (keys %poiTypeStat) > 0 ) {
      print "Number pois with WF poi type:\n";
      foreach my $c (sort {$a <=> $b} (keys (%poiTypeStat)) ) {
         my $ptString = getPOITypeNameFromTypeID( $m_dbh, $c);
         print "  $c = $poiTypeStat{$c}   $ptString\n";
      }
   }
   if ( scalar (keys %WFccDC) > 0 ) {
      print "City centre with WF display class:\n";
      foreach my $r ( sort {$a <=> $b} (keys (%WFccDC)) ) {
         print "  $r " . $WFccDC{ $r } . "\n";
      }
   }
   print "\n";
} # handleGenPoiFile


# Get Wayfinder city cente display class from displayClass/adminClass
sub getCCdispClass {
   my $dispClass = $_[0];
   my $adminClass = $_[1];
   my $midId = $_[2];
   my $poiName = $_[3];

   my $WFdisplayClass = 12;

   if ( !defined $dispClass ) {
      print "WARN: no dispClass for cc $midId $poiName " .
            "- will use display class $WFdisplayClass " .
            "- change it manually!?!?\n";
      return $WFdisplayClass;
   }

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

   if ( $opt_v eq "TA_2010_06" ) {
      # The data follows the specification
      $WFdisplayClass = $dispClass;
   }
   else {
      errprint "implement getCCdispClass for $opt_v";
      die "exit";
   }

   return $WFdisplayClass;

} # getCCdispClass



# Get Wayfinder poi type id from categories..
# Default return value is 95 = invalid POI type.
# If the returned value is -1, the poi should not be included into WF maps.
sub getWayfinderPoiTypeId {
   # get Wayfinder WASP poi type ids from the supplier poi category
   my $featureCode = $_[0]; # gdf feature code
   my $midId = $_[1];
   my $poiName = $_[2];
   my $poiImportance = $_[3];


   my $wfPoiTypeId = 95;

   my $tmpCat;
   # Get Wayfinder poi type id
   # Use the function in GDFPoiFeatureCodes
   # It returns 95 = invalidPOIType if the MultiNet POI feature code is unknown
   ($wfPoiTypeId, $tmpCat) = 
         get_type_and_cat_from_gdf_feature_code( $featureCode, 1 );
         # 1 = no die if not defined
   
   # Fix for airports, if there are MANY airports, store the ones with lower
   # importance as noType, not to clutter the map image with airport icons.
   # The not-so-important are local apts, likely not with passenger traffic
   if ( $wfPoiTypeId == 1 ) {
      if ( ($opt_v eq "TA_2006_10_sa") or
           ($opt_v eq "TA_2008_v1_31_sa") or
           ($opt_v eq "TA_2010_06") ) {
         if ( $poiImportance == 1 ) {
            # ok, keep as airport
         } elsif ( $poiImportance == 2 ) {
            $wfPoiTypeId = 94; # noType
         } else {
            # parse error. flatly accept; as an experiment.
	    # $wfPoiTypeId = -1; # don't include.
            # die "Special for airport with poiImportance=$poiImportance";
         }
      } else {
         errprint "Airport poi type fix for $opt_v ???";
         die "exit";
      }
   }
   
   if ( $wfPoiTypeId == 95 ) {
      # Don't die here,
      # Count the number of pois with each unknown poi category
      # and summarize when all pois in the poi file are looped
      # Then die...
      #die "getWayfinderPoiTypeId: unknow poi type " .
      #    "'$featureCode' for poi $midId";
      
      if ( defined $m_unknownPOIcategories{$featureCode} ) {
         $m_unknownPOIcategories{$featureCode} += 1;
      } else {
         $m_unknownPOIcategories{$featureCode} = 1;
         print "unknown poi feature code $featureCode\n";
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




# This action creates an ESRI turntable from the turn_table mid file.
sub handleRestrictions {
   dbgprint "handleRestrictions";

   if ( scalar @ARGV != 3 ) {
      errprint "Need 3 files\n" .
               " - manouvers mid\n" .
               " - manouvers path index txt\n" .
               " - network mid";
      die "exit";
   }


   # Files to process:
   # manouvers
   # manouvers path idx
   # network: 
   #  - needed to translate network id to rownbr
   

   # Read network file to translate network id to rownbr
   my %networkIDsToRowNbr = ();
   print "\n";
   print "Store network id-rownbr\n";
   my $idCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      # ok
      $idCol = 0;
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   while (<THIRD_FILE>) {
      chomp;
      if (length() > 1) {
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

         my $row = $.;
         my $id = $midRow[ $idCol ];
         $networkIDsToRowNbr{$id} = $row;
         
         if ( ($. % 100000) == 0 ) {
            print " read row $.\n";
         }
      }
   }
   print "Stored " . scalar(keys(%networkIDsToRowNbr)) . 
         " network id-rownbr\n";


   # Read manouvers path index to store start/end network segment for each
   # manouver (network row nbr)
   my %seq1NetworkId = ();
   my %seq2NetworkId = ();
   my %seq3NetworkId = ();
   my %seq4NetworkId = ();
   my %seq5NetworkId = ();
   my %seq6NetworkId = ();
   my %seq7NetworkId = ();
   my %seq8NetworkId = ();
   my %seq9NetworkId = ();
   print "\n";
   print "Store start/end network id\n";
   my $manIdCol = "";
   my $seqNbrCol = "";
   my $elemIdCol = "";
   # For checking if we have a header line in the manouvers path index file
   # to be able to skip it
   my $SeqnrString = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $manIdCol = 0;
      $seqNbrCol = 1;
      $elemIdCol = 2;
      my $SeqnrString = "Seqnr";
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   while (<SECOND_FILE>) {
      chomp;
      if (length() > 1) {
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );

         my $seq = $midRow[ $seqNbrCol ];
         my $elemId = $midRow[ $elemIdCol ];
         if ( length $elemId > 0 ) {
            # FIXME: want to test equal the $SeqnrString instead of
            # the hardcoded string "Seqnr", for the script to be more generic
            #if ( $seq eq "$SeqnrString" )
            if ( $seq eq "Seqnr" ) {
               # header line, skip it
            }
            elsif ( $seq == 1 ) {
               $seq1NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 2 ) {
               $seq2NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 3 ) {
               $seq3NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 4 ) {
               $seq4NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 5 ) {
               $seq5NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 6 ) {
               $seq6NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 7 ) {
               $seq7NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 8 ) {
               $seq8NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            elsif ( $seq == 9 ) {
               $seq9NetworkId{ $midRow[$manIdCol] } = $elemId;
            }
            else {
               errprint "unhandled sequential number = $seq for " .
                        "man $midRow[$manIdCol]";
               #die "exit";
            }
         } else {
            print "WARN: no element id for this manover path idx " .
                  $midRow[$manIdCol] . "\n";
         }
      }
   }
   print "Stored " . scalar(keys(%seq1NetworkId)) . " seq1 ids, " .
         scalar(keys(%seq2NetworkId)) . " seq2 ids, " .
         scalar(keys(%seq3NetworkId)) . " seq3 ids, " .
         scalar(keys(%seq4NetworkId)) . " seq4 ids, and " .
         scalar(keys(%seq5NetworkId)) . " seq5 ids, and " .
         scalar(keys(%seq6NetworkId)) . " seq6 ids, and " .
         scalar(keys(%seq7NetworkId)) . " seq7 ids, and " .
         scalar(keys(%seq8NetworkId)) . " seq8 ids, and " .
         scalar(keys(%seq9NetworkId)) . " seq9 ids for manouvers\n";
   print "\n";
   



   # Open and write Wayfinder turntable header file
   open(WF_RESTRHEAD_FILE, ">WFturntableheader.txt");
   print WF_RESTRHEAD_FILE "KEY\tNODE_\tARC1_\tARC2_\tIMPEDANCE\tLAT\tLON\n";
   
   # Open Wayfinder turntable file
   open(WF_RESTR_FILE, ">WFrestrictionsTurntable.txt");

   if ( (scalar(keys(%seq1NetworkId)) == 0) and
        (scalar(keys(%seq2NetworkId)) == 0) and
        (scalar(keys(%seq3NetworkId)) == 0) ) {
      print "No manouvers to write - exit \n";
      return 1;
   }

   
   # Read the restrictions file (manouvers file)
   # Define attribute columns, 0,1,2,...
   my $manCol = "";
   my $featTypeCol = "";
   my $bifurcTypeCol = "";
   my $proManTypeCol = "";
   my $juncCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      # ok
      $manCol = 0;
      $featTypeCol = 1;
      $bifurcTypeCol = 2;
      $proManTypeCol = 3;
      $juncCol = 4;
   } else {
      die "Specify attribute parsing for $opt_v";
   }
   
   my $nbrRestrRowsInFile = 0;
   my $nbrRestr = 0;
   my $nbrUnknown = 0;
   my $nbrNotHandledOK = 0;
   my %unhandledManouversByFeatType = ();
   while (<MID_FILE>) {
      chomp;

      # Split the mid line, handling $midSepChar within strings
      # and removing "-chars from string values
      my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
      
      my $manId = $midRow[$manCol];
      my $seq1Id = $seq1NetworkId{ $manId };
      my $seq2Id = $seq2NetworkId{ $manId };
      my $seq3Id = $seq3NetworkId{ $manId };
      my $seq4Id = $seq4NetworkId{ $manId };
      my $seq5Id = $seq5NetworkId{ $manId };
      my $seq6Id = $seq6NetworkId{ $manId };
      my $seq7Id = $seq7NetworkId{ $manId };
      my $seq8Id = $seq8NetworkId{ $manId };
      my $seq9Id = $seq9NetworkId{ $manId };
         
      # translate the road elem id to road row id
      my $seq1RowId = undef;
      if ( defined $seq1Id ) {
         $seq1RowId = $networkIDsToRowNbr{ $seq1Id };
      }
      my $seq2RowId = undef;
      if ( defined $seq2Id ) {
         $seq2RowId = $networkIDsToRowNbr{ $seq2Id };
      }
      my $seq3RowId = undef;
      if ( defined $seq3Id ) {
         $seq3RowId = $networkIDsToRowNbr{ $seq3Id };
      }
      my $seq4RowId = undef;
      if ( defined $seq4Id ) {
         $seq4RowId = $networkIDsToRowNbr{ $seq4Id };
      }
      my $seq5RowId = undef;
      if ( defined $seq5Id ) {
         $seq5RowId = $networkIDsToRowNbr{ $seq5Id };
      }
      my $seq6RowId = undef;
      if ( defined $seq6Id ) {
         $seq6RowId = $networkIDsToRowNbr{ $seq6Id };
      }
      my $seq7RowId = undef;
      if ( defined $seq7Id ) {
         $seq7RowId = $networkIDsToRowNbr{ $seq7Id };
      }
      my $seq8RowId = undef;
      if ( defined $seq8Id ) {
         $seq8RowId = $networkIDsToRowNbr{ $seq8Id };
      }
      my $seq9RowId = undef;
      if ( defined $seq9Id ) {
         $seq9RowId = $networkIDsToRowNbr{ $seq9Id };
      }

      # FIXME: the handledOK marks the manouvers that are handled properly 
      # by this method. If the handledOK=0, something needs change to 
      # handle all entries from the manouvers file properly
      my $handledOK = 1;
      if ( (defined $seq4Id) or
           (defined $seq5Id) ) {
         $handledOK = 0;
      }

      my $junctionId = $.; # not using $midRow[$juncCol];
      my $featType = $midRow[$featTypeCol];

      # Get restrictions type
      my $impedance = 0;
      my $printManeuverToSeq2 = 0;
      my $printManeuverToSeq3 = 0;
      if ( $featType == 2103 ) {
         # 2103 Prohibited Maneuver
         $impedance = -1;
         $printManeuverToSeq2 = 1;
         if ( $midRow[$proManTypeCol] == 0 ) {
            # ok, prohibited manouver
         }
         elsif ( $midRow[$proManTypeCol] == 1 ) {
            # implicit turn, action not decided for this type
            $handledOK = 0;
         }
         else {
            errprint "prohib man with prohib man type " .
                     $midRow[$proManTypeCol];
            die "exit";
         }
      } 
      elsif ( $featType == 9401 ) { 
         # Bifurcation
         $impedance = -2;
         my $bifurcType = $midRow[$bifurcTypeCol];
         if ( ($bifurcType == 1) or ($bifurcType == 2) ) {
            # multi-lane fork, simple fork
            $printManeuverToSeq2 = 1;
            $printManeuverToSeq3 = 1;
         } elsif ( $bifurcType == 9) {
            # exit bifurcation
            $handledOK = 0;
         } else {
            errprint "Bifurcation type $bifurcType";
            die "exit";
         }
      }
      elsif ( $featType == 2101) { 
         # 2101 Calculated/Derived Prohibited Maneuver
         # Derived from Blocked passeges (network file) + Restricted manouver
         $handledOK = 0;
      }
      elsif ( $featType == 2102) { 
         # 2102 Restricted Maneuver
         $handledOK = 0;
      }
      elsif ( $featType == 2104) { 
         # 2102 Priority Maneuver
         $handledOK = 0;
      } else {
         errprint "unknown man featureType $featType for junctionId=$junctionId";
         die "exit";
      }

      # Set coords to 0
      my $juncLat = 0;
      my $juncLon = 0;
      # Write restrictions to turn table, if we handled them OK
      if ( $handledOK ) {
         if ( $printManeuverToSeq2 ) {
            my $isOK = 1;
            if ( ! defined $seq1RowId ) {
               errprint "seq 1 row ids not defined manId $manId";
               if ( defined $seq1Id ) {
                  die "seq 1 row ids not defined manId $manId seqid $seq1Id";
               }
               $isOK = 0;
            }
            if ( ! defined $seq2RowId ) {
               errprint "seq 2 row ids not defined manId $manId";
               if ( defined $seq2Id ) {
                  die "seq 2 row ids not defined manId $manId seqid $seq2Id";
               }
               $isOK = 0;
            }
            if ( $isOK ) {
               print WF_RESTR_FILE 
                  "$nbrRestr\t$junctionId\t$seq1RowId\t$seq2RowId\t$impedance" .
                  "\t$juncLat\t$juncLon\n";
               $nbrRestr += 1;
            } else {
               $handledOK = 0;
            }
         }
         if ( $printManeuverToSeq3 ) {
            my $isOK = 1;
            if ( ! defined $seq1RowId ) {
               errprint "seq 1 row ids not defined manId $manId seqid $seq1Id";
               if ( defined $seq1Id ) {
                  die "seq 1 row ids not defined manId $manId seqid $seq1Id";
               }
               $isOK = 0;
            }
            if ( ! defined $seq3RowId ) {
               errprint "seq 3 row ids not defined manId $manId";
               if ( defined $seq3Id ) {
                  die "seq 3 row ids not defined manId $manId seqid $seq3Id";
               }
               $isOK = 0;
            }
            if ( $isOK ) {
               print WF_RESTR_FILE 
                  "$nbrRestr\t$junctionId\t$seq1RowId\t$seq3RowId\t$impedance" .
                  "\t$juncLat\t$juncLon\n";
               $nbrRestr += 1;
            } else {
               $handledOK = 0;
            }
         }
      }
      if ( ! $handledOK ) {
         #warnprint "manouver $manId not handled OK";
         $nbrNotHandledOK += 1;
         if ( defined $unhandledManouversByFeatType{"$featType"} ) {
            $unhandledManouversByFeatType{"$featType"} += 1;
         } else {
            $unhandledManouversByFeatType{"$featType"} = 1;
         }
      }
      $nbrRestrRowsInFile += 1;
   }
   if ( $nbrUnknown > 0 ) {
      die " unhandled featTypes, exit";
   }


   print "\n=======================================================\n";
   dbgprint "Read $nbrRestrRowsInFile rows from restrictions (maneuvers) file.";
   dbgprint "Wrote $nbrRestr restrictions/bifurcations.";
   dbgprint "There are $nbrNotHandledOK manouvers that are not handled, " .
            "needs to be fixed.";
   if ( scalar (keys (%unhandledManouversByFeatType)) ) {
      foreach my $featType (sort (keys (%unhandledManouversByFeatType)) ) {
         print " manouver $featType " . 
               $unhandledManouversByFeatType{"$featType"} . "\n";
      }
   }
} # handleRestrictions

# This action processes a water area/poly mid file
sub handleWaterPolyFile {
   dbgprint "handleWaterPolyFile";

   # columns
   my $idCol = "";
   my $featTypeCol = "";
   my $waterTypeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $idCol = 0;
      $featTypeCol = 1;
      $waterTypeCol = 2;
      $nameCol = 5;
      $nameLcCol = 6;
   } else {
      die "Specify attribute parsing for $opt_v";
   }

   handleWaterFile( "poly", $idCol, $featTypeCol, $waterTypeCol,
                    $nameCol, $nameLcCol );
}

# This action processes a water line mid file
sub handleWaterLineFile {
   dbgprint "handleWaterLineFile";

   # columns
   my $idCol = "";
   my $featTypeCol = "";
   my $waterTypeCol = "";
   my $nameCol = "";
   my $nameLcCol = "";
   if ( ($opt_v eq "TA_2010_06") ) {
      $idCol = 0;
      $featTypeCol = 1;
      $waterTypeCol = 2;
      $nameCol = 4;
      $nameLcCol = 5;
   } else {
      die "Specify attribute parsing for $opt_v";
   }

   handleWaterFile( "line", $idCol, $featTypeCol, $waterTypeCol,
                    $nameCol, $nameLcCol );
}

# This action processes a water mid file, line or poly
sub handleWaterFile {
   dbgprint "handleWaterFile";
   my $fileType = $_[0];
   dbgprint "handleWaterFile fileType = $fileType";
   if ( ! (($fileType eq "line") or ($fileType eq "poly")) ) {
      die "Incorrect fileType provided to handleWaterFile '$fileType'";
   }
   
   
   # Define attribute columns, 0,1,2,...
   my $midIdCol = $_[1];
   my $featTypeCol = $_[2];
   my $waterTypeCol = $_[3];
   my $nameCol = $_[4];
   my $nameLcCol = $_[5];
   
   
   # Open Wayfinder water file
   my $baseName = "WFwater_$fileType";
   open(WF_FILE, ">$baseName.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "$baseName.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItemsInFile = 0;
   my $nbrItemsToWFfile = 0;
   my $nbrWithNames = 0;
   my $mifFileReadPos = 0;
   my %wfWaterTypes = ();
   while (<MID_FILE>) {
      chomp;
      if (length() > 1) {
         $nbrItemsInFile += 1;
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $midId = $.;
         # Need to have unique ids for lines and polys
         if ( $fileType eq "line" ) {
            $midId += 1000000;
         }
         
         my $itemName = "";
         if ( length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         my $allNames = "";
         if ( length($itemName) > 1) {
            my $nameLang =
               getWFNameLanguage ( $midRow[ $nameLcCol ], $midId );
            $allNames = "$itemName:officialName:$nameLang";
            $nbrWithNames += 1;
         }

         # Check the feature type, don't want water centre line or
         # water shore line
         my $wanted = 1;
         my $featType = "";
         if (length($featTypeCol) > 0 ) { 
            $featType = $midRow[ $featTypeCol ];
            if ( $featType == 4310 ) {
               # ok, water element (area) or water line
            }
            elsif ( ($featType == 9315) or
                    ($featType == 9317) ) {
               # water center line, water shore line
               $wanted = 0;
            } else {
               errprint "Unknown feature type " . $featType ;
               die "exit";
            }
         }

         if ( $wanted ) {
            my $waterType = "river";
            if (length($waterTypeCol) > 0 ) {
               $waterType = getWFwaterType( $midRow[ $waterTypeCol ]);
            }
            if ( defined $wfWaterTypes{$waterType} ) {
               $wfWaterTypes{ $waterType } += 1;
            } else {
               $wfWaterTypes{ $waterType } = 1;
            }
        
            print WF_FILE "$midId,\"$itemName\",\"$allNames\"" .
                          ",\"$waterType\"" .
                          "\n";
            $nbrItemsToWFfile += 1;
         }
         
         # Read next feature from mif file and write to correct WF mif
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $SECOND_FILE_NAME, $mifFileReadPos,
            $outMifFileName, $wanted, $midId );
         
      }
   }

   print "\n=======================================================\n";
   dbgprint "Wrote $nbrItemsToWFfile of $nbrItemsInFile water items " .
            "to WF mid";
   dbgprint " - $nbrWithNames had names";
   dbgprint "Number features with wf water type";
   foreach my $c (sort (keys (%wfWaterTypes)) ) {
      print "  $c $wfWaterTypes{$c}\n";
   }
   print "Cat the 2 water files (line+poly) into one WF water file\n";
} # handleWaterFile

sub getWFwaterType {
   my $type = $_[0];

   # 0 no type
   # 1 oceans and seas
   # 2 lake
   # 7 others

   if ( ($type == 1) ) {
      return "ocean";
   }
   elsif ( $type == 2 ) {
      return "lake";
   }
   elsif ( $type == 7 ) {
      return "river";
   }
   else {
      die "getWFwaterType unknown type $type - exit";
   }
}



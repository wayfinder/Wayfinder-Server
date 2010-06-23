#!/usr/bin/perl -w
# Script to create Wayfinder midmif files from Carmenta Nice World Map
# midmif files.
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
use vars qw( $m_dbh %m_poiTypes
             $m_langStr $m_dbLangId 
             $m_coordOrder %m_cityTypes 
             %m_countryInfo 
             %m_tmpCountryIdFromIso
             %m_isoFromFips );

# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
use lib ".";
use POIObject;
use PerlWASPTools;
use GDFPoiFeatureCodes;
use PerlTools;

use DBI;


getopts('hs:r:v:t:l:c:d:u:y:');


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
-s string   The mid file separator char (default ",")
-r string   A temp char to use as separator when having mid file separators
            within strings (default "}")
-l string   The language string to use for official names. Check with
            Wayfinder midmif spec to get a valid language string.
            eng - english
-c string   The countryInfo.txt file to use. Mandatory for
            municipal and cityCentre parsing.
-v string   The version of the data (the format specification differs
            between the versions):
               Carmenta_200809   World
-t string   The type of item that is in the mid/mif file to process:
               municipal         mid+mif     (land)
               builtUpArea       mid+mif     (builtup)
               forest            mid+mif     (forest)
               waterLake         mid+mif     (lake)
               waterRiver        mid+mif     (river)

               cityCentre        mid+mif  (city centres)
               

Note:
   * Run dos2unix on all midmif files before running this script
     to avoid windows eol.
   * POI-files need to have UTF-8 char encoding (and mc2-coords) before
     added to WASP db with poiImport script.
EOM
  exit 1;
}


# Hash mapping poi types to WF POI categories
# Only set the categories that are not given from WF POI type
# (no need to duplicate that info..)
my %types2wfcat = ();





# Connect to database for working with POI objects..
sub connectToDBH {
   # 
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
my %m_unknownPOIcategories = ();
my $m_dollarName = "\$\$\$\$\$";

# Check that map version is valid.
if ( !defined($opt_v) ) {
   die "Midmif map version not specified! - exit\n";
} else {
   if ( $opt_v eq "Carmenta_200809" ) {
      dbgprint "Extract data, version $opt_v";
   } else {
      die "Invalid version specified ($opt_v) - exit!!";
   }
}

if ( !defined($opt_l) ) {
   die "Language for names not specified! - exit";
} else {
   dbgprint "Will use nameStr '$opt_l' for names";
   $m_langStr = "$opt_l";

   # get also language id for POIs
   if ( $opt_l eq "eng" ) {
      $m_dbLangId = 0;
   } else {
      die "Please define WASP language id for language $opt_l";
   }
   dbgprint "Will use language id $m_dbLangId for poi names";
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

# Check the country info file, and load
if ( defined $opt_c ) {
   if ( testFile($opt_c) ) {
      dbgprint "Country info file $opt_c";
      loadCountryInfo();
      dbgprint "We loaded info for " . scalar (keys %m_countryInfo) .
               " countries";
   } else {
      die "problem with country info file $opt_c";
   }
} else {
   die "no countryInfo file given"
}

# Check what to do
if (defined($opt_t)) {
   if ($opt_t eq "builtUpArea") {
      handleBuiltUpAreaFile();
   } elsif ($opt_t eq "forest") {
      handleForestFile();
   } elsif ($opt_t eq "river") {
      handleWaterFile($opt_t);
   } elsif ($opt_t eq "lake") {
      handleWaterFile($opt_t);
   } elsif ($opt_t eq "municipal") {
      handleMunicipalFile();
   } elsif ($opt_t eq "cityCentre") {
      handleCCFile();
   } else {
      die "Unknown item type: $opt_t\n";
   }
} else {
   die "No item type specified! Run $0 -h for help.\n";
}


exit;



# Function to check midId, that it is not large than MAX_UITN32
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
         dbgprint "Use e.g. row number plus an offset " .
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

# Load function copied from andWorld.pl script
# The %m_countryInfo is a hash of hashes
#  key = andID, value = 
#        hash ( key = [countryName, gmsName, alpha2, ..], value )
sub loadCountryInfo {
   if (open INFO_FILE, $opt_c) {
      %m_countryInfo = ();
      %m_tmpCountryIdFromIso = ();
      while( <INFO_FILE> ) {
         chomp;
         (my $id, 
          my $countryName, 
          my $gmsName, 
          my $alpha2, 
          my $continent) = split (";");
         if ( (! defined $id) or
              (! defined $countryName) or
              (! defined $gmsName) or
              (! defined $alpha2 ) or
              (! defined $continent ) ) {
            errprint "Something wrong with country info file $opt_c\n" .
                     "on row $.";
            die "exit";
         }
         $m_countryInfo{$id}{"countryName"} = $countryName;
         $m_countryInfo{$id}{"gmsName"} = $gmsName;
         $m_countryInfo{$id}{"alpha2"} = $alpha2;
         $m_countryInfo{$id}{"continent"} = $continent;
         #print "$gmsName\n"

         $m_tmpCountryIdFromIso{"$alpha2"} = $id;
      }
      dbgprint "loadCountryInfo: m_countryInfo=" .
               scalar(keys(%m_countryInfo)) . 
               " m_tmpCountryIdFromIso=" .
               scalar( keys %m_tmpCountryIdFromIso );
   } else {
      errprint "loadCountryInfo: no info file to read";
      die "exit";
   }
}

# This action processes a built_up area mid file
sub handleBuiltUpAreaFile {
   dbgprint "handleBuiltUpAreaFile";

   my $midIdCol = undef;
   my $nameCol = undef;
   my $useMidId = 0;
   my $useName = 0;
   if ( $opt_v eq "Carmenta_200809" ) {
      # no attributes to use
   } else {
      die "Define attribute columns for $opt_v";
   }
   
   # Open Wayfinder builtUpArea mid file
   open(WF_FILE, ">areaWFbuiltUpAreaItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "areaWFbuiltUpAreaItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItems = 0;
   my %usedids = ();
   my $mifFileReadPos = 0;
   my $nbrNoNames = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 0) {
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         # for id, use row number + offset
         my $midId = $.;
         if ( defined($usedids{"$midId"}) ) {
            errprint "Duplicated id: $midId"; 
            die "exit";
         } else {
            $usedids{"$midId"} = 1;
         }
         if ( !checkMidId($midId, 1) ) {  # parseonly check
            die "What to do with bad midid ($midRow[ $midIdCol ]) for $opt_v??";
         }
         checkMidId($midId, 0); # not parseonly die if check fails
         
         # Names
         my $itemName = "";
         if ( $useName and length($nameCol) > 0 ) {
            $itemName = $midRow[ $nameCol ];
         }
         
         # If the miditem has no name at all..
         if ( length($itemName) == 0 ) {
            $nbrNoNames += 1;
         }
         
         my $allNames = "";
         if ( length($itemName) > 1) {
            $allNames = "$itemName:officialName:$m_langStr";
         }

         # settlement ids
         my $settlId = "";
         my $settlOrder = 8;
         
         print WF_FILE "$midId,\"$itemName\",\"$allNames\"" .
                       ",$settlId,$settlOrder\n";
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
   dbgprint "Nbr buas no name = $nbrNoNames";
   print "\n";
}


sub handleWaterFile {
   my $sub = $_[0];
   if ( ! defined $sub ) {
      die "no sub defined for handleWaterFile";
   }
   dbgprint "handleWaterFile type=$sub";
   
   # Define attribute columns, 0,1,2,... and whether to use name and id or not
   my $useName = 0;
   my $useMidId = 0;
   my $midIdCol = undef;
   my $nameCol = undef;
   if ( $opt_v eq "Carmenta_200809" ) {
      # no attributes to use
   } else {
      die "Define attribute columns for $opt_v and possible adjust the idOffset after checking how many river/lake fetures there are";
   }
   
   my $idOffset = 0;
   my $waterType = "unknownWaterElement";
   if ( $sub eq "river" ) {
      $idOffset = 0;
      $waterType = "river";
   } elsif ( $sub eq "lake" ) {
      $idOffset = 10000000;
      $waterType = "lake";
   } else {
      die "add instructions for $sub";
   }

   
   # Open Wayfinder mid files
   open(WF_FILE, ">WFwaterItems_$sub.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFwaterItems_$sub.mif";
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
         
         # for id, use row number + offset
         my $midId = $. + $idOffset;
         checkMidId($midId, 0); # not parseonly die if check fails
         
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
         print WF_FILE "$midId,\"$itemName\",\"$allNames\"" .
                       ",\"$waterType\"\n";

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
}

# This action processes a forest file
sub handleForestFile {
   dbgprint "handleForestFile";
   
   # Define attribute columns, 0,1,2,... and whether to use name or not
   my $useName = 0;
   my $useId = 0;
   my $midIdCol = undef;
   my $nameCol = undef;
   if ( $opt_v eq "Carmenta_200809" ) {
      # no attributes to use
   } else {
      die "Define attribute columns for $opt_v";
   }
   
   # Open Wayfinder mid files
   open(WF_FILE, ">WFforestItems.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   my $outMifFileName = "WFforestItems.mif";
   printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
   
   my $nbrItemsInFile = 0;
   my $nbrItemsWithName = 0;
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
} # handleForestFile


# This action processes a (order8) mid file
sub handleMunicipalFile {
   dbgprint "Handles municipals";
   
   # Need countryInfo.txt to get a good mid id for the municipals
   if ( ! scalar %m_countryInfo ) {
      die "no countryInfo.txt loaded - exit";
   }
  
   # Define attribute columns, 0,1,2,...
   # if changing here, plase also change in handleNetwork
   my $midIdCol = undef;
   my $nameCol = undef;
   my $fipsCol = undef;
   if ( $opt_v eq "Carmenta_200809" ) {
      $fipsCol = 0;
   } else {
      die "Define attribute columns for $opt_v";
   }
  

   
   ## Open Wayfinder municipal file
   #open(WF_FILE, ">WFmunicipalItems.mid");
   ## Define Wayfinder mif file
   ## print the Wayfinder mif header with correct coordsys tag, this clears 
   ## the files for future append writing
   #my $outMifFileName = "WFmunicipalItems.mif";
   #printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );

   # Cannot decide midmif file name until we have read the row from the file
   # Use a hash with info about which files have been created - to only
   # open the mid file and print mif header into the files one time.
   my %processedFiles = ();

   my $nbrItems = 0;
   my $mifFileReadPos = 0;
   while (<MID_FILE>) {
      chomp;
      if (length() > 0) {
         
         # Split the mid line, handling $midSepChar within strings
         # and removing "-chars from string values
         my @midRow =  splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
         my $rowNbr= $.;
         my $midId = $rowNbr;
         # fips -> iso -> tmpCountryID -> countryName
         if ( ! defined $fipsCol ) {
            die "must have fips column - exit";
         }
         my $fipsCode = $midRow[ $fipsCol ];
         my $isoCode = getIsoFromFips($fipsCode, $rowNbr);
         my $tmpCountryID = $m_tmpCountryIdFromIso{"$isoCode"};
         if ( ! defined $tmpCountryID ) {
            print "No tmp country id defined for iso=$isoCode " .
                "rowNbr=$rowNbr (fips $fipsCode) - SKIP\n";
            # We do not want this mid row, but we nee to read the feature
            # from the mif file
            my $outMifFileName = "_crap.mif";
            my $wanted = 0;
            ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
               $SECOND_FILE_NAME, $mifFileReadPos,
               $outMifFileName, $wanted, $midId );
         } else {
            my $countryName = $m_countryInfo{$tmpCountryID}{"countryName"};
            if ( ! defined $countryName ) {
               die "countryName not there for iso=$isoCode " .
                   "rowNbr=$rowNbr - exit";
            }
            print "country $isoCode fipsCode=$fipsCode " .
                  "countryName=$countryName\n";
            
            # midId = tmpCountryID
            $midId = $tmpCountryID;
            if ( !checkMidId($midId, 1) ) {  # parseonly check
               die "What to do with bad midid $midId for $opt_v??";
            }
            checkMidId($midId, 0); # not parseonly die if check fails
            
            # Names
            my $itemName = $countryName;
            
            # If the miditem has no name at all set name to m_dollarName
            if ( length($itemName) == 0 ) {
               print "ERROR Municipal midId=$midId has no name " .
                     "-> '$m_dollarName'\n";
               $itemName = "$m_dollarName";
            }
            my $allNames = "";
            if ( length($itemName) > 1) {
               $allNames = "$itemName:officialName:$m_langStr";
            }

            my $isoLower = lc $isoCode;
            my $fileBaseName = "$isoLower" . "_whole_municipalItems";
            print " $fileBaseName\n";
            my $outMifFileName = "$fileBaseName.mif";
            if ( ! defined $processedFiles{$fileBaseName} ) {
               # open files
               $processedFiles{$fileBaseName} = 1;
               # Open Wayfinder municipal mid file
               open(WF_FILE, ">$fileBaseName.mid");
               # Define Wayfinder mif file
               # print the Wayfinder mif header with correct coordsys tag,
               # this clears the files for future append writing
               printOneWayfinderMifHeader( $outMifFileName, "$opt_y" );
            } else {
               # nothing
               $processedFiles{$fileBaseName} += 1;
               print "$isoCode " . $processedFiles{$fileBaseName} . 
                     " times\n";
            }
            # Open Wayfinder municipal mid file
            open(WF_FILE, ">>$fileBaseName.mid");
            

            print WF_FILE "$midId,\"$itemName\",\"$allNames\"\n";
            $nbrItems += 1;
               
            # Read next feature from mif file and write to correct WF mif
            my $wanted = 1;
            ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
               $SECOND_FILE_NAME, $mifFileReadPos,
               $outMifFileName, $wanted, $midId );
            
         }
      }
   }

   print "\n=======================================================\n";
   dbgprint "Wrote $nbrItems municipal items to mid";
   print "Fix mcm map division and copy mif files\n";
   print "\n";
   dbgprint "These municipals had info from many fips codes:\n";
   foreach my $m (sort keys %processedFiles) {
      if ( $processedFiles{$m} > 1 ) {
         print "  $m " . $processedFiles{$m} . "\n";
      }
   }


} # handleMunicipalFile




# Handle City centre POI file, write to CPIF
sub handleCCFile {
   dbgprint "handleCCFile";

   # Define attribute columns, 0,1,2,...
   my $midIdCol = "";
   my $nameCol = "";
   my $typeCol = "";
   my $cityNameCol = "";
   my $strNameCol = "";
   my $stateNameCol = "";
   my $ccPopulationCol = "";
   my $ccTypeCol = "";
   my $fipsCol = "";
   if ( $opt_v eq "Carmenta_200809" )  {
      $nameCol = 0;
      $ccPopulationCol = 1;
      $ccTypeCol = 2;
      $fipsCol = 3;
   } else {
      die "check attribute column order for $opt_v";
   }

   if ( (! defined $fipsCol) or (length $fipsCol < 1) ) {
      die "need fips code column";
   }
    
   handleGenPoiFile( "cc",
                     11,            # given poi type = city centre
                     $midIdCol,
                     $nameCol,
                     $typeCol,
                     $strNameCol,
                     $cityNameCol,
                     $stateNameCol,
                     $ccPopulationCol,
                     $ccTypeCol,
                     $fipsCol
                     );
}

#
# General function that handles misc POI files,
# write to CPIF (use POI object)
sub handleGenPoiFile {
   my $fileType = $_[0];
   my $givenWFpoiTypeID = $_[1];  # given POI type
   my $midIdCol = $_[2];
   my $nameCol = $_[3];
   my $typeCol = $_[4];
   my $strNameCol = $_[5];
   my $cityNameCol = $_[6];
   my $stateNameCol = $_[7];
   my $ccPopCol = $_[8];
   my $ccTypeCol = $_[9];
   my $fipsCol = $_[10];
   dbgprint "handleGenPoiFile: $fileType";
   
   # Need countryInfo.txt for mapping POI into correct country
   if ( ! scalar %m_countryInfo ) {
      die "no countryInfo.txt loaded - exit";
   }
   
   # Open Wayfinder mid files
   my $baseSsiFileName = "WFstreetSegmentItems_$fileType";
   open(WF_SSIMID_FILE, ">$baseSsiFileName.mid");
   # Define Wayfinder mif file
   # print the Wayfinder mif header with correct coordsys tag, this clears 
   # the files for future append writing
   printOneWayfinderMifHeader( "$baseSsiFileName.mif", "mc2" );

   
   # Define out file name, open to empty any old file, then it is
   # written with append.
   my $wfFileName = "WFcpif_${fileType}.txt";
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
   my %poisWithCCtype = ();
   my $nbrColumns = 0;
   my $nbrNoFips = 0;
   my %nbrDispClassForCountryID = ();
   while (<MID_FILE>) {
      chomp;
      if (length() > 2) {
      
         $nbrPois += 1;

         # Split the midRow, handling $midSepChar within strings
         # also removing "-chars from string values
         my @poiRecord = 
               splitOneMidRow( $midSepChar, $midRplChar, $_ );
         
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
         my $pureMidId = int $midId;
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
            die "Any alternative name to use for name-less " .
                "poi $midId for version $opt_v ?? - exit ";
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
         if ( defined $poisWithWFType{$wfPoiTypeId} ) {
            $poisWithWFType{$wfPoiTypeId} += 1;
         } else {
            $poisWithWFType{$wfPoiTypeId} = 1;
         }

         # Check FIPS
         if ( ( ! defined $poiRecord[$fipsCol] ) or 
              ( length $poiRecord[$fipsCol] < 1 ) ) {
            if ( length $poiRecord[$fipsCol] == 0 ) {
               # no fips code for this poi, skip it since we cannot calculate
               # which country it is located in.
               $wfPoiTypeId = -1;
               $nbrNoFips += 1;
            } else {
               die "no fips code for poi $midId";
            }
         }
         my $fipsCode = $poiRecord[$fipsCol];

         # Count nbr pois with certain city centre type
         if ( defined $ccTypeCol ) {
            if ( defined $poisWithCCtype{$poiRecord[$ccTypeCol]} ) {
               $poisWithCCtype{$poiRecord[$ccTypeCol]} += 1;
            } else {
               $poisWithCCtype{$poiRecord[$ccTypeCol]} = 1;
            }
         }
         
         # Only work with POI if poi type is not -1 (unwanted pois..)
         if ( $wfPoiTypeId < 0 ) {
            $nbrUnwantedPOIs += 1;
         } 
         else {
            
            # Get iso code from FIPS
            my $isoCode = getIsoFromFips($fipsCode, $midId);
            #print " poi $midId fips $fipsCode iso $isoCode\n";

            # Get poi coordinate from mif file hash, and convert to mc2
            my $lat; my $lon; 
            if ( index(uc $opt_y, "WGS84") > -1 ) {
               # normal wgs84
               # ok
            } else {
               die "Coordsys for $opt_v???";
            }
            ($lat, $lon) = 
               convertFromWgs84ToMc2(  $latitudes->{ $nbrPois },
                                       $longitudes->{ $nbrPois } );
            if ( !defined($lat) or !defined($lon) ) {
               die "No coordinate defined for poi midId $midId - exit";
            }

            
            # Create POI object and fill with info
            my $poiObject = POIObject->newObject();
            $poiObject->setDBH($m_dbh);

            # POIMain
            my $countryID = getCountryIDFromISO3166Code($m_dbh, $isoCode);
            if (! defined $countryID) {
               die "no country id for iso $isoCode (fips $fipsCode) " .
                   "for poi $midId - exit";
            }
            #print " country is $isoCode -> id $countryID\n";
            $poiObject->setCountry( $countryID );
            $poiObject->setSourceReference($midId);
            $poiObject->setSymbolCoordinate($lat, $lon);

            # POITypes
            $poiObject->addPOIType( $wfPoiTypeId );

            # POICategories
            if ( defined $typeCol and length($typeCol) > 0 ) {
               my $wfPoiCat = $types2wfcat{$poiRecord[$typeCol]};
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

            # POIEntryPoints
            # we can add the symbol coord as entry point, since
            # we will create street segment items from the symbol coord
            if ( $opt_v eq "Carmenta_200809" ) {
               $poiObject->addEntryPoint($lat, $lon);
            } else {
               die "use symbol coord as entry point for $opt_v ??? - exit";
            }
            
            # POIInfo
            #  0 | Vis. address
            if ( defined $strNameCol and (length($strNameCol) > 0) and
                 $poiRecord[$strNameCol] ) {
               $poiObject->addPOIInfo( 0, 31, $poiRecord[$strNameCol] ); }
            #  5 | Vis. zip area (city name)
            if ( defined $cityNameCol and (length($cityNameCol) > 0) and
                 $poiRecord[$cityNameCol] ) {
               $poiObject->addPOIInfo( 5, 31, $poiRecord[$cityNameCol] ); }
            #  14 | State
            if ( defined $stateNameCol and (length $stateNameCol > 0) and
                 $poiRecord[$stateNameCol] ) {
               $poiObject->addPOIInfo( 14, 31, $poiRecord[$stateNameCol] ); }
            # cc display class
            if ( $wfPoiTypeId == 11 ) {
               my $ccDispClass = 12; # default
               if ( defined $ccPopCol and (length $ccPopCol > 0) and
                    defined $ccTypeCol and (length $ccTypeCol > 0) ) {
                  $ccDispClass = getCCdispClassFromPopulation(
                     $poiRecord[$ccPopCol], $poiRecord[$ccTypeCol],
                     $midId, $poiRecord[$nameCol] );
               }
               $poiObject->addPOIInfo( 47, 31, $ccDispClass );
               if ( defined $ccWithDispClass{$ccDispClass} ) {
                  $ccWithDispClass{$ccDispClass} += 1;
               } else {
                  $ccWithDispClass{$ccDispClass} = 1;
               }
               if ( defined 
                     $nbrDispClassForCountryID{$countryID}{$ccDispClass} ) {
                  $nbrDispClassForCountryID{$countryID}{$ccDispClass} += 1;
               } else {
                  $nbrDispClassForCountryID{$countryID}{$ccDispClass} = 1;
               }
            }


            # Write this POIObject to CPIF
            if ( $poiObject->writeCPIF($wfFileName) ) {
               $nbrPOIsToCPIF += 1;
            } else {
               print " POI object not written to CPIF midId $midId\n";
            }

            # Write street segment for this poi
            if ( $opt_v eq "Carmenta_200809" ) {
               open (WF_SSIMIF_FILE, ">>$baseSsiFileName.mif");
               my $otherLon = $lon-1;
               my $otherLat = $lat-1;
               print WF_SSIMIF_FILE "Pline 2\n" .
                                    "$lat $lon\n" .
                                    "$otherLat $otherLon\n";

               my $settlOrder = 8;
               my $settleID = $m_tmpCountryIdFromIso{"$isoCode"};
               if ( ! defined $settleID ) {
                  die "no tmp country id defined for $isoCode - exit";
               }
               #print " iso $isoCode -> tmp country id = $settleID\n";
               my $tmpSsiName = getCountryNameFromID($m_dbh, $countryID);
               print WF_SSIMID_FILE 
                        "$pureMidId,\"$tmpSsiName\",\"\",4,50,50," .  
                        "0,0,,,,,0,0,0,0,\"Y\",0,0," .
                        "\"N\",\"N\",\"N\",\"N\",\"N\",\"N\",\"N\"," .
                        "\"\",\"\",$settleID,$settleID,$settlOrder," .
                        "\"\",\"\"\n";
            } else {
               die "print street segments from the poi " .
                   "symbol coord for $opt_v??? - exit";
            }
         }
      }
   }


   print "\n=======================================================\n";
   dbgprint "Read $nbrPois pois from mid file";
   if ( scalar (keys %m_unknownPOIcategories) > 0 ) {
      errprint "We have unknown poi categories - edit parse script!";
      foreach my $r ( sort (keys (%m_unknownPOIcategories)) ) {
         print "  poi category \"$r\" has "
               . $m_unknownPOIcategories{ $r } . " pois\n";
      }
      errprint "add these poi categories in the initPOITypesHash function";
      die "exit";
   }
   
   print "Not using $nbrUnwantedPOIs unwanted pois\n" .
         "(of which $nbrNoFips had no fips code defined)\n" .
         "Wrote $nbrPOIsToCPIF pois to CPIF $wfFileName\n";
   print "Convert char encoding to UTF-8 before adding to WASP db!!\n" .
         "Add to WASP db with poiImport.pl -f readCPIF\n";

   print "\nNumber of POIs for WF poi type:\n";
   foreach my $t ( sort {$a <=> $b} (keys (%poisWithWFType)) ) {
      print " pt $t:  " . $poisWithWFType{$t} . " (" .
            getPOITypeNameFromTypeID($m_dbh, $t) . ")\n";
   }
   if ( scalar keys (%poisWithSupplierType) > 0 ) {
      print "\nNumber of POIs for supplier poi type:\n";
      foreach my $t ( sort (keys (%poisWithSupplierType)) ) {
         print " $t:  " . $poisWithSupplierType{$t} . "\n";
      }
   }
   if ( scalar keys (%poisWithCCtype) > 0 ) {
      print "\nNumber of CCs with cc type:\n";
      foreach my $t ( sort (keys (%poisWithCCtype)) ) {
         print " $t:  " . $poisWithCCtype{$t} . "\n";
      }
   }
   if ( scalar keys (%ccWithDispClass) > 0 ) {
      print "\nNumber of CCs for WF display class:\n";
      foreach my $t ( sort {$a <=> $b} (keys (%ccWithDispClass)) ) {
         print " displclass $t:  " . $ccWithDispClass{$t} . "\n";
      }
   }
   if ( scalar keys (%nbrDispClassForCountryID) > 0 ) {
      print "\nCountries display class 2:\n";
      foreach my $t ( sort {$a <=> $b} (keys (%nbrDispClassForCountryID)) ) {
         if ( ! defined $nbrDispClassForCountryID{$t}{2} ) {
            print " country $t (" . getCountryNameFromID($m_dbh, $t) .
                  ") has none!\n";
         }
         elsif ( $nbrDispClassForCountryID{$t}{2} > 1 ) {
            print " country $t (" . getCountryNameFromID($m_dbh, $t) .
                  ") has " .
                  $nbrDispClassForCountryID{$t}{2} . "\n";
         }
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
   print "\n";

} # handleGenPoiFile


# Get Wayfinder city cente display class from the population
sub getCCdispClassFromPopulation {
   my $population = $_[0];
   my $cityType = $_[1];
   my $midId = $_[2];
   my $poiName = $_[3];

   my $displayClass = 12;

   if ( !defined $population ) {
      print "WARN: no population for cc $midId $poiName " .
            "- will use display class $displayClass " .
            "- change it manually!?!?\n";
      die "exit";
      return $displayClass;
   }
   if ( !defined $cityType ) {
      print "WARN: no city type for cc $midId $poiName " .
            "- will use display class $displayClass " .
            "- change it manually!?!?\n";
      die "exit";
      return $displayClass;
   }
   if ( ! cityTypeOK($cityType, $midId) ) {
      die "unhandled city type $cityType for cc $midId $poiName";
   }


   # Display class according to TA Europe rules
   # XXX It might not be correct for this supplier
   # Some adjustmens have been done
   if ( $cityType eq "PPLC" ) {
      $displayClass = 2;
   }
   elsif ( (length $cityType == 0) or
           ($cityType eq "PPLL") or
           ($cityType eq "PPLR") or
           ($cityType eq "PPLX") or
           ($cityType eq "PPLW") or
           ($cityType eq "PPLQ") ) {
      $displayClass = 12;
   } else {

      if ( $population > 1000000 ) { # 1 milion
         $displayClass = 4;
      }
      elsif ( $population > 500000 ) {
         $displayClass = 5;
      }
      #elsif ( $population > 100000 )
      elsif ( $population > 120000 ) {
         $displayClass = 7;
      }
      #elsif ( $population > 50000 )
      elsif ( $population > 60000 ) {
         $displayClass = 8;
      }
      elsif ( $population > 10000 ) {
         $displayClass = 10;
      }
      elsif ( $population > 0 ) {
         #$displayClass = 11;
         $displayClass = 12;
      }
   }

   return $displayClass;

#   # TA Canada/USA rules
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

sub cityTypeOK {
   my $cityType = $_[0];
   my $midId = $_[1];

   if ( length $cityType == 0 ) {
      warnprint "no city type for $midId";
      return 1;
   }

   if ( ! scalar %m_cityTypes ) {
      initCityTypes();
   }

   if ( defined $m_cityTypes{$cityType} ) {
      return 1;
   }

   return 0;
}

sub initCityTypes {
   %m_cityTypes = ();

   $m_cityTypes{"PPL"} = 1;   # any populated place
   $m_cityTypes{"PPLA"} = 1;  # seat of first-order admin division
   $m_cityTypes{"PPLC"} = 1;  # capital
   $m_cityTypes{"PPLG"} = 1;  # seat of government of political entity
   $m_cityTypes{"PPLL"} = 1;  # locality
   $m_cityTypes{"PPLQ"} = 1;  # abandoned
   $m_cityTypes{"PPLR"} = 1;  # religious
   $m_cityTypes{"PPLS"} = 1;  # populated places
   $m_cityTypes{"PPLW"} = 1;  # destroyed
   $m_cityTypes{"PPLX"} = 1;  # section of pop place
}

sub getIsoFromFips {
   my $fipsCode = $_[0];
   my $id = $_[1];
   
   if (!defined $fipsCode ) {
      die "no fips defined for $id";
   }

   if ( ! scalar %m_isoFromFips ) {
      initFIPScodes();
   }

   my $isoCode = $m_isoFromFips{"$fipsCode"};
   if ( ! defined $isoCode ) {
      die "no iso code to be found for fips $fipsCode for id=$id";
   }

   return $isoCode;
}

sub initFIPScodes {
   %m_isoFromFips = ();
   
   $m_isoFromFips{"AN"} = "AD"; #Andorra Andorra la Vella 
   $m_isoFromFips{"AE"} = "AE"; #United Arab Emirates Abu Dhabi 
   $m_isoFromFips{"AF"} = "AF"; #Afghanistan Kabul 
   $m_isoFromFips{"AC"} = "AG"; #Antigua and Barbuda Saint John's 
   $m_isoFromFips{"AV"} = "AI"; #Anguilla The Valley 
   $m_isoFromFips{"AL"} = "AL"; #Albania Tirane 
   $m_isoFromFips{"AM"} = "AM"; #Armenia Yerevan 
   $m_isoFromFips{"NT"} = "AN"; #Netherlands Antilles Willemstad 
   $m_isoFromFips{"AO"} = "AO"; #Angola Luanda 
   $m_isoFromFips{"AY"} = "AQ"; #Antarctica South Pole 
   $m_isoFromFips{"AR"} = "AR"; #Argentina Buenos Aires 
   $m_isoFromFips{"AQ"} = "AS"; #American Samoa Pago Pago 
   $m_isoFromFips{"AU"} = "AT"; #Austria Vienna 
   $m_isoFromFips{"AS"} = "AU"; #Australia Canberra 
   $m_isoFromFips{"AA"} = "AW"; #Aruba Oranjestad 
   $m_isoFromFips{"AJ"} = "AZ"; #Azerbaijan Baku 
   $m_isoFromFips{"AT"} = "xx"; #Ashmore and Cartier Islands
   $m_isoFromFips{"BK"} = "BA"; #Bosnia and Herzegovina Sarajevo 
   $m_isoFromFips{"BB"} = "BB"; #Barbados Bridgetown 
   $m_isoFromFips{"BG"} = "BD"; #Bangladesh Dhaka 
   $m_isoFromFips{"BE"} = "BE"; #Belgium Brussels 
   $m_isoFromFips{"UV"} = "BF"; #Burkina Faso Ouagadougou 
   $m_isoFromFips{"BU"} = "BG"; #Bulgaria Sofia 
   $m_isoFromFips{"BA"} = "BH"; #Bahrain Manama 
   $m_isoFromFips{"BY"} = "BI"; #Burundi Bujumbura 
   $m_isoFromFips{"BN"} = "BJ"; #Benin Porto-Novo 
   $m_isoFromFips{"BD"} = "BM"; #Bermuda Hamilton 
   $m_isoFromFips{"BX"} = "BN"; #Brunei Darussalam Bandar Seri Begawan 
   $m_isoFromFips{"BL"} = "BO"; #Bolivia La Paz 
   $m_isoFromFips{"BR"} = "BR"; #Brazil Brasilia 
   $m_isoFromFips{"BF"} = "BS"; #Bahamas Nassau 
   $m_isoFromFips{"BT"} = "BT"; #Bhutan Thimphu 
   $m_isoFromFips{"BV"} = "BV"; #Bouvet Island n/a 
   $m_isoFromFips{"BC"} = "BW"; #Botswana Gaborone 
   $m_isoFromFips{"BO"} = "BY"; #Belarus Minsk 
   $m_isoFromFips{"BH"} = "BZ"; #Belize Belmopan 
   $m_isoFromFips{"BQ"} = "xx"; #Navassa
   $m_isoFromFips{"BS"} = "xx"; #Bassas da India
   $m_isoFromFips{"CA"} = "CA"; #Canada Ottawa 
   $m_isoFromFips{"CK"} = "CC"; #Cocos Islands (Keeling Islands) West Island 
   $m_isoFromFips{"CG"} = "CD"; #Dem. Rep. of the Congo (Zaire) Kinshasa 
   $m_isoFromFips{"CT"} = "CF"; #Central African Republic Bangui 
   $m_isoFromFips{"CF"} = "CG"; #Congo (Rep. of the Congo) Brazzaville 
   $m_isoFromFips{"SZ"} = "CH"; #Switzerland Bern 
   $m_isoFromFips{"IV"} = "CI"; #Cote D'Ivoire (Ivory Coast) Yamoussoukro 
   $m_isoFromFips{"CW"} = "CK"; #Cook Islands Rarotonga 
   $m_isoFromFips{"CI"} = "CL"; #Chile Santiago 
   $m_isoFromFips{"CM"} = "CM"; #Cameroon Yaounde 
   $m_isoFromFips{"CH"} = "CN"; #China Beijing 
   $m_isoFromFips{"CO"} = "CO"; #Colombia Bogota 
   $m_isoFromFips{"CS"} = "CR"; #Costa Rica San Jose 
   $m_isoFromFips{"CU"} = "CU"; #Cuba Havana 
   $m_isoFromFips{"CV"} = "CV"; #Cape Verde Praia 
   $m_isoFromFips{"KT"} = "CX"; #Christmas Island The Settlement 
   $m_isoFromFips{"CY"} = "CY"; #Cyprus Nicosia 
   $m_isoFromFips{"EZ"} = "CZ"; #Czech Republic Prague 
   $m_isoFromFips{"CR"} = "xx"; #Coral Sea Islands
   $m_isoFromFips{"GM"} = "DE"; #Germany Berlin 
   $m_isoFromFips{"DJ"} = "DJ"; #Djibouti Djibouti 
   $m_isoFromFips{"DA"} = "DK"; #Denmark Copenhagen 
   $m_isoFromFips{"DO"} = "DM"; #Dominica Roseau 
   $m_isoFromFips{"DR"} = "DO"; #Dominican Republic Santo Domingo 
   $m_isoFromFips{"AG"} = "DZ"; #Algeria Algiers
   $m_isoFromFips{"DQ"} = "UM"; #United States minor outlying islands
   $m_isoFromFips{"EC"} = "EC"; #Ecuador Quito 
   $m_isoFromFips{"EN"} = "EE"; #Estonia Tallinn 
   $m_isoFromFips{"EG"} = "EG"; #Egypt Cairo 
   $m_isoFromFips{"WI"} = "EH"; #Western Sahara El Aioun 
   $m_isoFromFips{"ER"} = "ER"; #Eritrea Asmara 
   $m_isoFromFips{"SP"} = "ES"; #Spain Madrid 
   $m_isoFromFips{"ET"} = "ET"; #Ethiopia Addis Ababa
   $m_isoFromFips{"EU"} = "xx"; #?????
   $m_isoFromFips{"FI"} = "FI"; #Finland Helsinki 
   $m_isoFromFips{"FJ"} = "FJ"; #Fiji Suva 
   $m_isoFromFips{"FK"} = "FK"; #Falkland Islands Stanley 
   $m_isoFromFips{"FM"} = "FM"; #Federated States of Micronesia Palikir 
   $m_isoFromFips{"FO"} = "FO"; #Faroe Islands Torshavn
   $m_isoFromFips{"FQ"} = "xx"; #????
   $m_isoFromFips{"FR"} = "FR"; #France Paris 
   $m_isoFromFips{"GB"} = "GA"; #Gabon Libreville 
   $m_isoFromFips{"UK"} = "GB"; #United Kingdom London 
   $m_isoFromFips{"GJ"} = "GD"; #Grenada Saint George's 
   $m_isoFromFips{"GG"} = "GE"; #Georgia Tbilisi 
   $m_isoFromFips{"FG"} = "GF"; #French Guiana Cayenne 
   $m_isoFromFips{"GH"} = "GH"; #Ghana Accra 
   $m_isoFromFips{"GI"} = "ES"; #Gibraltar Gibraltar 
   $m_isoFromFips{"GL"} = "GL"; #Greenland Nuuk 
   $m_isoFromFips{"GA"} = "GM"; #Gambia Banjul 
   $m_isoFromFips{"GV"} = "GN"; #Guinea Conakry 
   $m_isoFromFips{"GP"} = "GP"; #Guadeloupe Basse-Terre 
   $m_isoFromFips{"EK"} = "GQ"; #Equatorial Guinea Malabo 
   $m_isoFromFips{"GR"} = "GR"; #Greece Athens 
   $m_isoFromFips{"SX"} = "GS"; #South Georgia and the South Sandwich Islands n/a 
   $m_isoFromFips{"GT"} = "GT"; #Guatemala Guatemala City 
   $m_isoFromFips{"GQ"} = "GU"; #Guam Agana 
   $m_isoFromFips{"PU"} = "GW"; #Guinea-Bissau Bissau 
   $m_isoFromFips{"GY"} = "GY"; #Guyana Georgetown
   $m_isoFromFips{"GK"} = "GB"; #Guernsey
   $m_isoFromFips{"GO"} = "xx"; #Glorioso Islands
   $m_isoFromFips{"HK"} = "HK"; #Hong Kong Beijing 
   $m_isoFromFips{"HM"} = "HM"; #Heard Island and McDonald Islands n/a 
   $m_isoFromFips{"HO"} = "HN"; #Honduras Tegucigalpa 
   $m_isoFromFips{"HR"} = "HR"; #Croatia Zagreb 
   $m_isoFromFips{"HA"} = "HT"; #Haiti Port-au-Prince 
   $m_isoFromFips{"HU"} = "HU"; #Hungary Budapest
   $m_isoFromFips{"HQ"} = "xx"; #???
   $m_isoFromFips{"ID"} = "ID"; #Indonesia Jakarta 
   $m_isoFromFips{"EI"} = "IE"; #Ireland Dublin 
   $m_isoFromFips{"IS"} = "IL"; #Israel Jerusalem 
   $m_isoFromFips{"IN"} = "IN"; #India New Delhi 
   $m_isoFromFips{"IO"} = "IO"; #British Indian Ocean Territory n/a 
   $m_isoFromFips{"IZ"} = "IQ"; #Iraq Baghdad 
   $m_isoFromFips{"IR"} = "IR"; #Iran Tehran 
   $m_isoFromFips{"IC"} = "IS"; #Iceland Reykjavik 
   $m_isoFromFips{"IT"} = "IT"; #Italy Rome 
   $m_isoFromFips{"IM"} = "GB"; #Isle of Man
   $m_isoFromFips{"IP"} = "xx"; #???
   $m_isoFromFips{"JM"} = "JM"; #Jamaica Kingston 
   $m_isoFromFips{"JO"} = "JO"; #Jordan Amman 
   $m_isoFromFips{"JA"} = "JP"; #Japan Tokyo 
   $m_isoFromFips{"JE"} = "GB"; #Jersey
   $m_isoFromFips{"JN"} = "xx"; #Jan Mayen
   $m_isoFromFips{"JQ"} = "xx"; #Johnston Atoll
   $m_isoFromFips{"JU"} = "xx"; #Juan de Nova Island
   $m_isoFromFips{"KE"} = "KE"; #Kenya Nairobi 
   $m_isoFromFips{"KG"} = "KG"; #Kyrgyzstan Bishkek 
   $m_isoFromFips{"CB"} = "KH"; #Cambodia Phnom Penh 
   $m_isoFromFips{"KR"} = "KI"; #Kiribati Tarawa 
   $m_isoFromFips{"CN"} = "KM"; #Comoros Moroni 
   $m_isoFromFips{"SC"} = "KN"; #Saint Kitts and Nevis Basseterre 
   $m_isoFromFips{"KN"} = "KP"; #North Korea Pyongyang 
   $m_isoFromFips{"KS"} = "KR"; #South Korea Seoul 
   $m_isoFromFips{"KU"} = "KW"; #Kuwait Kuwait City 
   $m_isoFromFips{"CJ"} = "KY"; #Cayman Islands George Town 
   $m_isoFromFips{"KZ"} = "KZ"; #Kazakhstan Astana
   $m_isoFromFips{"KQ"} = "xx"; #Kingman Reef
   $m_isoFromFips{"LA"} = "LA"; #Laos Vientiane 
   $m_isoFromFips{"LE"} = "LB"; #Lebanon Beirut 
   $m_isoFromFips{"ST"} = "LC"; #Saint Lucia Castries 
   $m_isoFromFips{"LS"} = "LI"; #Liechtenstein Vaduz 
   $m_isoFromFips{"CE"} = "LK"; #Sri Lanka Colombo 
   $m_isoFromFips{"LI"} = "LR"; #Liberia Monrovia 
   $m_isoFromFips{"LT"} = "LS"; #Lesotho Maseru 
   $m_isoFromFips{"LH"} = "LT"; #Lithuania Vilnius 
   $m_isoFromFips{"LU"} = "LU"; #Luxembourg Luxembourg 
   $m_isoFromFips{"LG"} = "LV"; #Latvia Riga 
   $m_isoFromFips{"LY"} = "LY"; #Libya Tripoli
   $m_isoFromFips{"LQ"} = "xx"; #???
   $m_isoFromFips{"MO"} = "MA"; #Morocco Rabat 
   $m_isoFromFips{"MN"} = "MC"; #Monaco Monaco 
   $m_isoFromFips{"MD"} = "MD"; #Moldova Chisinau 
   $m_isoFromFips{"MA"} = "MG"; #Madagascar Antananarivo 
   $m_isoFromFips{"RM"} = "MH"; #Marshall Islands Majuro 
   $m_isoFromFips{"MK"} = "MK"; #Macedonia Skopje 
   $m_isoFromFips{"ML"} = "ML"; #Mali Bamko 
   $m_isoFromFips{"BM"} = "MM"; #Myanmar (Burma) Rangoon 
   $m_isoFromFips{"MG"} = "MN"; #Mongolia Ulaanbaatar 
   $m_isoFromFips{"MC"} = "MO"; #Macao (Macau) Macao 
   $m_isoFromFips{"CQ"} = "MP"; #Northern Mariana Islands Saipan 
   $m_isoFromFips{"MB"} = "MQ"; #Martinique Fort-de-France 
   $m_isoFromFips{"MR"} = "MR"; #Mauritania Nouakchott 
   $m_isoFromFips{"MH"} = "MS"; #Montserrat Plymouth 
   $m_isoFromFips{"MT"} = "MT"; #Malta Valletta 
   $m_isoFromFips{"MP"} = "MU"; #Mauritius Port Louis 
   $m_isoFromFips{"MV"} = "MV"; #Maldives Male 
   $m_isoFromFips{"MI"} = "MW"; #Malawi Lilongwe 
   $m_isoFromFips{"MX"} = "MX"; #Mexico Mexico City 
   $m_isoFromFips{"MY"} = "MY"; #Malaysia Kuala Lumpur 
   $m_isoFromFips{"MZ"} = "MZ"; #Mozambique Maputo 
   $m_isoFromFips{"MQ"} = "xx"; #Midway Islands
#   $m_isoFromFips{"MW"} = "ME"; #Montenegro
   $m_isoFromFips{"MW"} = "CS"; #Montenegro -> Serbia and Montenegro
   $m_isoFromFips{"WA"} = "NA"; #Namibia Windhoek 
   $m_isoFromFips{"NC"} = "NC"; #New Caledonia Noumea 
   $m_isoFromFips{"NG"} = "NE"; #Niger Niamey 
   $m_isoFromFips{"NF"} = "NF"; #Norfolk Island Kingston (north of NZ)
   $m_isoFromFips{"NI"} = "NG"; #Nigeria Abuja 
   $m_isoFromFips{"NU"} = "NI"; #Nicaragua Managua 
   $m_isoFromFips{"NL"} = "NL"; #Netherlands Amsterdam 
   $m_isoFromFips{"NO"} = "NO"; #Norway Oslo 
   $m_isoFromFips{"NP"} = "NP"; #Nepal Kathmandu 
   $m_isoFromFips{"NR"} = "NR"; #Nauru Yaren District 
   $m_isoFromFips{"NE"} = "NU"; #Niue Alofi 
   $m_isoFromFips{"NZ"} = "NZ"; #New Zealand Wellington 
   $m_isoFromFips{"MU"} = "OM"; #Oman Muscat 
   $m_isoFromFips{"OS"} = "xx"; #??? (outside Saudi Arabia)
   $m_isoFromFips{"PM"} = "PA"; #Panama Panama City 
   $m_isoFromFips{"PE"} = "PE"; #Peru Lima 
   $m_isoFromFips{"FP"} = "PF"; #French Polynesia Papeete 
   $m_isoFromFips{"PP"} = "PG"; #Papua New Guinea Port Moresby 
   $m_isoFromFips{"RP"} = "PH"; #Philippines Manila 
   $m_isoFromFips{"PK"} = "PK"; #Pakistan Islamabad 
   $m_isoFromFips{"PL"} = "PL"; #Poland Warsaw 
   $m_isoFromFips{"SB"} = "PM"; #Saint-Pierre and Miquelon Saint-Pierre 
   $m_isoFromFips{"PC"} = "PN"; #Pitcairn Islands Adamstown 
   $m_isoFromFips{"RQ"} = "US"; #Puerto Rico San Juan 
   $m_isoFromFips{"GZ"} = "PS"; #Occupied Palestinian Territories n/a 
   $m_isoFromFips{"WE"} = "PS"; #Occupied Palestinian Territories n/a 
   $m_isoFromFips{"PO"} = "PT"; #Portugal Lisbon 
   $m_isoFromFips{"PS"} = "PW"; #Palau Koror 
   $m_isoFromFips{"PA"} = "PY"; #Paraguay Asuncion 
   $m_isoFromFips{"PF"} = "VN"; #???? outside vietnam
   $m_isoFromFips{"PG"} = "PH"; #??? outside philipines
   $m_isoFromFips{"QA"} = "QA"; #Qatar Doha 
   $m_isoFromFips{"RE"} = "RE"; #Rйunion Saint-Denis 
   $m_isoFromFips{"RO"} = "RO"; #Romania Bucharest 
   $m_isoFromFips{"RW"} = "RW"; #Rwanda Kigali 
   $m_isoFromFips{"SA"} = "SA"; #Saudi Arabia Riyadh 
   $m_isoFromFips{"BP"} = "SB"; #Solomon Islands Honiara 
   $m_isoFromFips{"SE"} = "SC"; #Seychelles Victoria 
   $m_isoFromFips{"SU"} = "SD"; #Sudan Khartoum 
   $m_isoFromFips{"SW"} = "SE"; #Sweden Stockholm 
   $m_isoFromFips{"SN"} = "SG"; #Singapore Singapore 
   $m_isoFromFips{"SH"} = "SH"; #Saint Helena Jamestown 
   $m_isoFromFips{"SI"} = "SI"; #Slovenia Ljubljana 
   $m_isoFromFips{"SV"} = "SJ"; #Svalbard and Jan Mayen Islands Longyearbyen 
   $m_isoFromFips{"LO"} = "SK"; #Slovakia Bratislava 
   $m_isoFromFips{"SL"} = "SL"; #Sierra Leone Freetown 
   $m_isoFromFips{"SM"} = "IT"; #San Marino San Marino 
   $m_isoFromFips{"SG"} = "SN"; #Senegal Dakar 
   $m_isoFromFips{"SO"} = "SO"; #Somalia Mogadishu 
   $m_isoFromFips{"NS"} = "SR"; #Suriname Paramaribo 
   $m_isoFromFips{"TP"} = "ST"; #Sao Tome and Principe n/a 
   $m_isoFromFips{"RS"} = "RU"; #USSR Moskva 
   $m_isoFromFips{"ES"} = "SV"; #El Salvador San Salvador 
   $m_isoFromFips{"SY"} = "SY"; #Syria Damascus 
   $m_isoFromFips{"WZ"} = "SZ"; #Swaziland Mbabane 
#   $m_isoFromFips{"SR"} = "RS"; #Serbia
   $m_isoFromFips{"SR"} = "CS"; #Serbia
   $m_isoFromFips{"TK"} = "TC"; #Turks and Caicos Islands n/a 
   $m_isoFromFips{"CD"} = "TD"; #Chad (Tchad) N'Djamena 
   $m_isoFromFips{"FS"} = "TF"; #French Southern Territories Paris 
   $m_isoFromFips{"TO"} = "TG"; #Togo Lome 
   $m_isoFromFips{"TH"} = "TH"; #Thailand Bangkok 
   $m_isoFromFips{"TI"} = "TJ"; #Tajikistan Dushanbe 
   $m_isoFromFips{"TL"} = "TK"; #Tokelau n/a 
   $m_isoFromFips{"TT"} = "TL"; #Timor-Leste (East Timor) Dili 
   $m_isoFromFips{"TX"} = "TM"; #Turkmenistan Ashgabat 
   $m_isoFromFips{"TS"} = "TN"; #Tunisia Tunis 
   $m_isoFromFips{"TN"} = "TO"; #Tonga Nuku'alofa 
   $m_isoFromFips{"TU"} = "TR"; #Turkey Ankara 
   $m_isoFromFips{"TD"} = "TT"; #Trinidad and Tobago Port-of-Spain 
   $m_isoFromFips{"TV"} = "TV"; #Tuvalu Funafuti 
   $m_isoFromFips{"TW"} = "TW"; #Taiwan Taipei 
   $m_isoFromFips{"TZ"} = "TZ"; #Tanzania Dar es Salaam 
   $m_isoFromFips{"TE"} = "xx"; #Tromelin Island
   $m_isoFromFips{"UP"} = "UA"; #Ukraine Kiev 
   $m_isoFromFips{"UG"} = "UG"; #Uganda Kampala 
   $m_isoFromFips{"-"} = "UM";  #United States Minor Outlying Islands Washington
   $m_isoFromFips{"US"} = "US"; #United States Washington 
   $m_isoFromFips{"UY"} = "UY"; #Uruguay Montevideo 
   $m_isoFromFips{"UZ"} = "UZ"; #Uzbekistan Tashkent 
   $m_isoFromFips{"UN"} = "xx"; #??? Outside north korea
   $m_isoFromFips{"VT"} = "IT"; #Vatican City State Vatican 
   $m_isoFromFips{"VC"} = "VC"; #Saint Vincent and the Grenadines Kingstown 
   $m_isoFromFips{"VE"} = "VE"; #Venezuela Caracas 
   $m_isoFromFips{"VI"} = "VG"; #British Virgin Islands Road Town 
   $m_isoFromFips{"VQ"} = "VI"; #U.S. Virgin Islands Saint Thomas 
   $m_isoFromFips{"VM"} = "VN"; #Viet Nam (Vietnam) Hanoi 
   $m_isoFromFips{"NH"} = "VU"; #Vanuatu Port-Vila 
   $m_isoFromFips{"WF"} = "WF"; #Wallis and Futuna Mata-Utu 
   $m_isoFromFips{"WS"} = "WS"; #Samoa (Western Samoa) Apia 
   $m_isoFromFips{"WQ"} = "xx"; #Wake Island
   $m_isoFromFips{"YM"} = "YE"; #Yemen Sanaa 
   $m_isoFromFips{"MF"} = "YT"; #Mayotte Mamoutzou 
   $m_isoFromFips{"YY"} = "IN"; #Kashmir??
   $m_isoFromFips{"SF"} = "ZA"; #South Africa Pretoria 
   $m_isoFromFips{"ZA"} = "ZM"; #Zambia Lusaka 
   $m_isoFromFips{"ZI"} = "ZW"; #Zimbabwe Harare 

   dbgprint "initFIPScodes: " . scalar (keys %m_isoFromFips) .
            " entries in m_isoFromFips";
}


#AD AN Andorra Andorra la Vella 
#AE AE United Arab Emirates Abu Dhabi 
#AF AF Afghanistan Kabul 
#AG AC Antigua and Barbuda Saint John's 
#AI AV Anguilla The Valley 
#AL AL Albania Tirane 
#AM AM Armenia Yerevan 
#AN NT Netherlands Antilles Willemstad 
#AO AO Angola Luanda 
#AQ AY Antarctica South Pole 
#AR AR Argentina Buenos Aires 
#AS AQ American Samoa Pago Pago 
#AT AU Austria Vienna 
#AU AS Australia Canberra 
#AW AA Aruba Oranjestad 
#AZ AJ Azerbaijan Baku 
#xx AT Ashmore and Cartier Islands
#BA BK Bosnia and Herzegovina Sarajevo 
#BB BB Barbados Bridgetown 
#BD BG Bangladesh Dhaka 
#BE BE Belgium Brussels 
#BF UV Burkina Faso Ouagadougou 
#BG BU Bulgaria Sofia 
#BH BA Bahrain Manama 
#BI BY Burundi Bujumbura 
#BJ BN Benin Porto-Novo 
#BM BD Bermuda Hamilton 
#BN BX Brunei Darussalam Bandar Seri Begawan 
#BO BL Bolivia La Paz 
#BR BR Brazil Brasilia 
#BS BF Bahamas Nassau 
#BT BT Bhutan Thimphu 
#BV BV Bouvet Island n/a 
#BW BC Botswana Gaborone 
#BY BO Belarus Minsk 
#BZ BH Belize Belmopan 
#xx BQ Navassa
#xx BS Bassas da India
#CA CA Canada Ottawa 
#CC CK Cocos Islands (Keeling Islands) West Island 
#CD CG Dem. Rep. of the Congo (Zaire) Kinshasa 
#CF CT Central African Republic Bangui 
#CG CF Congo (Rep. of the Congo) Brazzaville 
#CH SZ Switzerland Bern 
#CI IV Cote D'Ivoire (Ivory Coast) Yamoussoukro 
#CK CW Cook Islands Rarotonga 
#CL CI Chile Santiago 
#CM CM Cameroon Yaounde 
#CN CH China Beijing 
#CO CO Colombia Bogota 
#CR CS Costa Rica San Jose 
#CU CU Cuba Havana 
#CV CV Cape Verde Praia 
#CX KT Christmas Island The Settlement 
#CY CY Cyprus Nicosia 
#CZ EZ Czech Republic Prague 
#xx CR Coral Sea Islands
#DE GM Germany Berlin 
#DJ DJ Djibouti Djibouti 
#DK DA Denmark Copenhagen 
#DM DO Dominica Roseau 
#DO DR Dominican Republic Santo Domingo 
#DZ AG Algeria Algiers
#UM DQ United States minor outlying islands
#EC EC Ecuador Quito 
#EE EN Estonia Tallinn 
#EG EG Egypt Cairo 
#EH WI Western Sahara El Aioun 
#ER ER Eritrea Asmara 
#ES SP Spain Madrid 
#ET ET Ethiopia Addis Ababa
#xx EU ?????
#FI FI Finland Helsinki 
#FJ FJ Fiji Suva 
#FK FK Falkland Islands Stanley 
#FM FM Federated States of Micronesia Palikir 
#FO FO Faroe Islands Torshavn
#xx FQ ????
#FR FR France Paris 
#GA GB Gabon Libreville 
#GB UK United Kingdom London 
#GD GJ Grenada Saint George's 
#GE GG Georgia Tbilisi 
#GF FG French Guiana Cayenne 
#GH GH Ghana Accra 
#ES GI Gibraltar Gibraltar 
#GL GL Greenland Nuuk 
#GM GA Gambia Banjul 
#GN GV Guinea Conakry 
#GP GP Guadeloupe Basse-Terre 
#GQ EK Equatorial Guinea Malabo 
#GR GR Greece Athens 
#GS SX South Georgia and the South Sandwich Islands n/a 
#GT GT Guatemala Guatemala City 
#GU GQ Guam Agana 
#GW PU Guinea-Bissau Bissau 
#GY GY Guyana Georgetown
#GB GK Guernsey
#xx GO Glorioso Islands
#HK HK Hong Kong Beijing 
#HM HM Heard Island and McDonald Islands n/a 
#HN HO Honduras Tegucigalpa 
#HR HR Croatia Zagreb 
#HT HA Haiti Port-au-Prince 
#HU HU Hungary Budapest
#xx HQ ???
#ID ID Indonesia Jakarta 
#IE EI Ireland Dublin 
#IL IS Israel Jerusalem 
#IN IN India New Delhi 
#IO IO British Indian Ocean Territory n/a 
#IQ IZ Iraq Baghdad 
#IR IR Iran Tehran 
#IS IC Iceland Reykjavik 
#IT IT Italy Rome 
#GB IM Isle of Man
#xx IP ???
#JM JM Jamaica Kingston 
#JO JO Jordan Amman 
#JP JA Japan Tokyo 
#GB JE Jersey
#xx JN Jan Mayen
#xx JQ Johnston Atoll
#xx JU Juan de Nova Island
#KE KE Kenya Nairobi 
#KG KG Kyrgyzstan Bishkek 
#KH CB Cambodia Phnom Penh 
#KI KR Kiribati Tarawa 
#KM CN Comoros Moroni 
#KN SC Saint Kitts and Nevis Basseterre 
#KP KN North Korea Pyongyang 
#KR KS South Korea Seoul 
#KW KU Kuwait Kuwait City 
#KY CJ Cayman Islands George Town 
#KZ KZ Kazakhstan Astana
#xx KQ Kingman Reef
#LA LA Laos Vientiane 
#LB LE Lebanon Beirut 
#LC ST Saint Lucia Castries 
#LI LS Liechtenstein Vaduz 
#LK CE Sri Lanka Colombo 
#LR LI Liberia Monrovia 
#LS LT Lesotho Maseru 
#LT LH Lithuania Vilnius 
#LU LU Luxembourg Luxembourg 
#LV LG Latvia Riga 
#LY LY Libya Tripoli
#xx LQ ???
#MA MO Morocco Rabat 
#MC MN Monaco Monaco 
#MD MD Moldova Chisinau 
#MG MA Madagascar Antananarivo 
#MH RM Marshall Islands Majuro 
#MK MK Macedonia Skopje 
#ML ML Mali Bamko 
#MM BM Myanmar (Burma) Rangoon 
#MN MG Mongolia Ulaanbaatar 
#MO MC Macao (Macau) Macao 
#MP CQ Northern Mariana Islands Saipan 
#MQ MB Martinique Fort-de-France 
#MR MR Mauritania Nouakchott 
#MS MH Montserrat Plymouth 
#MT MT Malta Valletta 
#MU MP Mauritius Port Louis 
#MV MV Maldives Male 
#MW MI Malawi Lilongwe 
#MX MX Mexico Mexico City 
#MY MY Malaysia Kuala Lumpur 
#MZ MZ Mozambique Maputo 
#xx MQ Midway Islands
#ME MW Montenegro
#NA WA Namibia Windhoek 
#NC NC New Caledonia Noumea 
#NE NG Niger Niamey 
#NF NF Norfolk Island Kingston 
#NG NI Nigeria Abuja 
#NI NU Nicaragua Managua 
#NL NL Netherlands Amsterdam 
#NO NO Norway Oslo 
#NP NP Nepal Kathmandu 
#NR NR Nauru Yaren District 
#NU NE Niue Alofi 
#NZ NZ New Zealand Wellington 
#OM MU Oman Muscat 
#xx OS ??? (outside Saudi Arabia)
#PA PM Panama Panama City 
#PE PE Peru Lima 
#PF FP French Polynesia Papeete 
#PG PP Papua New Guinea Port Moresby 
#PH RP Philippines Manila 
#PK PK Pakistan Islamabad 
#PL PL Poland Warsaw 
#PM SB Saint-Pierre and Miquelon Saint-Pierre 
#PN PC Pitcairn Islands Adamstown 
#US RQ Puerto Rico San Juan 
#PS GZ Occupied Palestinian Territories n/a 
#PS WE Occupied Palestinian Territories n/a 
#PT PO Portugal Lisbon 
#PW PS Palau Koror 
#PY PA Paraguay Asuncion 
#VN PF ???? outside vietnam
#PH PG ??? outside philipines
#QA QA Qatar Doha 
#RE RE Rйunion Saint-Denis 
#RO RO Romania Bucharest 
#RW RW Rwanda Kigali 
#SA SA Saudi Arabia Riyadh 
#SB BP Solomon Islands Honiara 
#SC SE Seychelles Victoria 
#SD SU Sudan Khartoum 
#SE SW Sweden Stockholm 
#SG SN Singapore Singapore 
#SH SH Saint Helena Jamestown 
#SI SI Slovenia Ljubljana 
#SJ SV Svalbard and Jan Mayen Islands Longyearbyen 
#SK LO Slovakia Bratislava 
#SL SL Sierra Leone Freetown 
#IT SM San Marino San Marino 
#SN SG Senegal Dakar 
#SO SO Somalia Mogadishu 
#SR NS Suriname Paramaribo 
#ST TP Sao Tome and Principe n/a 
#RU RS USSR Moskva 
#SV ES El Salvador San Salvador 
#SY SY Syria Damascus 
#SZ WZ Swaziland Mbabane 
#RS SR Serbia
#TC TK Turks and Caicos Islands n/a 
#TD CD Chad (Tchad) N'Djamena 
#TF FS French Southern Territories Paris 
#TG TO Togo Lome 
#TH TH Thailand Bangkok 
#TJ TI Tajikistan Dushanbe 
#TK TL Tokelau n/a 
#TL TT Timor-Leste (East Timor) Dili 
#TM TX Turkmenistan Ashgabat 
#TN TS Tunisia Tunis 
#TO TN Tonga Nuku'alofa 
#TR TU Turkey Ankara 
#TT TD Trinidad and Tobago Port-of-Spain 
#TV TV Tuvalu Funafuti 
#TW TW Taiwan Taipei 
#TZ TZ Tanzania Dar es Salaam 
#xx TE Tromelin Island
#UA UP Ukraine Kiev 
#UG UG Uganda Kampala 
#UM - United States Minor Outlying Islands Washington 
#US US United States Washington 
#UY UY Uruguay Montevideo 
#UZ UZ Uzbekistan Tashkent 
#KP UN ??? Outside north korea
#IT VT Vatican City State Vatican 
#VC VC Saint Vincent and the Grenadines Kingstown 
#VE VE Venezuela Caracas 
#VG VI British Virgin Islands Road Town 
#VI VQ U.S. Virgin Islands Saint Thomas 
#VN VM Viet Nam (Vietnam) Hanoi 
#VU NH Vanuatu Port-Vila 
#WF WF Wallis and Futuna Mata-Utu 
#WS WS Samoa (Western Samoa) Apia 
#xx WQ Wake Island
#YE YM Yemen Sanaa 
#YT MF Mayotte Mamoutzou 
#IN YY Kashmir??
#ZA SF South Africa Pretoria 
#ZM ZA Zambia Lusaka 
#ZW ZI Zimbabwe Harare 


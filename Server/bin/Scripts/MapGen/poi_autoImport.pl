#!/usr/bin/perl -w
#
# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;
use strict;
use Getopt::Long ':config',
                  'no_auto_abbrev', # Impossible to use -s for --source.
                  'no_ignore_case'  # Case sensitive options.
;
use Cwd 'chdir',  # Keeps PWD up to date.
        'abs_path'
;

use File::Temp ':mktemp';


# Print command line.
print "Command: $0";
foreach my $arg (@ARGV){
   print " $arg";
}
print "\n\n";



# BASEGENFILESPATH 
# genfiles is the base directory where all setting files for map generation
# is stored
# Update it to point to the full path of where you create the BASEGENFILESPATH
#my $BASEGENFILESPATH="fullpath/genfiles";
use lib "/home/is/devel/Maps/genfilesPSTC/script/perllib"; # LLLPSTC
my $BASEGENFILESPATH=".";
#my $BASEGENFILESPATH="/home/is/devel/Maps/genfilesPSTC"; # LLLPSTC
my $genfilesScriptPath = "$BASEGENFILESPATH/script";
my $poiCategoryXMLFile = "$BASEGENFILESPATH/xml/poi_category_tree.xml";


# Use perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;
use PerlWASPTools;


use vars qw( $opt_help $opt_verbose $opt_quiet $opt_test
             $opt_poiSource $opt_poiData 
             $opt_mapCountriesDirs $opt_mapVerOverride
             $opt_okWithMissingCountries
             $opt_newProduct $opt_latitudeBand
             $opt_startAt $opt_stopAfter 
             $opt_noEntryPoints $opt_noInitCount $opt_onlyPrintSourceInfo
             $opt_createNewStaticIds
             $opt_geocodeIfEmptyPos );
GetOptions('help|h',
           'verbose',
           'quiet|q',
           'test',
           'newProduct',
           'latitudeBand=s',
           'noEntryPoints',
           'noInitCount',
           'onlyPrintSourceInfo',
           'poiSource=s',
           'poiData=s',
           'mapCountriesDirs=s',
           'okWithMissingCountries',
           'mapVerOverride=s',
           'startAt=s',
           'stopAfter=s',
           'createNewStaticIds',
           'geocodeIfEmptyPos=s'
           ) or die "Use -help for help.";

# Print help message.
if ( $opt_help ){
   print "USAGE:
poi_autoImport.pl OPTIONS

This script is used when importing POI data to WASP. The script calls POI source dependent scripts for the import step. Some of the other tasks are also POI source dependent, like geocoding.
All non script executable files should be located in the current directory (WASPExtractor and mc2.prop).

Make sure to run the script on a CentOS computer since it is needed to run some of the used modules.

OPTIONS:


GENERIC OPTIONS:

-help
Print this help.

-verbose
Make the script print more debug information.

-quiet
Turn off all debug printing.

-test
Run the script without calling any of the subscripts. When this option is 
given, no data is changed.



SCRIPT PARAMETERS OPTIONS:
Depending on the script control options, some of these are mandatory.

-newProduct
Give this option if this is the first POI source of this POI product. That way the script wont die when not finding the previous source of the product of the source. Static IDs will not be migrated, simply creating new ones.

-latitudeBand BAND_NUMBER
Determines which latitude band to start processing in static migration. Used when some latitude bands have already been processed. Number 1 is the first band.

-poiSource POI_SOURCE_STRING
The name of the POI source pointed at from the POIMain.source field. Set in the database  when importing the POIs to WASP, and after that step for identifying the POIs to work with.

-poiData POI_DATA_PATH
The disc location for original POI data. Can be a file or a directory, depending on the type of data. Value depending on POI product.
   TeleAtlas GDF           The file poi_wasp.log 
   Misc suppliers CPIF     The CPIF file (e.g. from shape/midmif)

-mapCountriesDirs COUNTRIES_DIR1,COUNTRIES_DIR2 ...
Countries directories of map generations in a comma spearated list with no space. Typically \"EW,WW\" pointing at a EW countries and a WW countries directory. These should have mapOrigin.txt files for each country. Default: the countries dirs must contain all countries that are defined in the WASP POICountries table. If not, combine with the -okWithMissingCountries option.
Special value for this option is NOT_USED. Use it with care. If this value is given, the map version wont be fetched from the country directories. Only values given in option -mapVerOverride will be used.

-okWithMissingCountries
Give this option if the -mapCountriesDirs only contain some countries.

-mapVerOverride IS03166:MAPVER,ISO3166:MAPVER ...
Overrides map version values found in directories pointed at by -mapCountiresDirs. Example value: sg:TA_2007_02,hk:TA_2006_11,gb:TA_2007_01 (alpha2 ISO country code:map release as given in the countries' mapOrigin.txt file)

-createNewStaticIds
If given, we do not try to migrate static IDs from a previous source, instead we simply set new static IDs.

-geocodeIfEmptyPos
Set to 0 or 1 if POIs with empty coordinates should be prepared for geocoding. The POIs will be written to a geocode-file. Geocode and parse back the geocodes to e.g. a CPIF or insert the coords into the POI database with poi_admin importPosFromGeocodeResult function.

SCRIPT CONTROL OPTIONS

-startAt START_STEP
Makes the script start at the given start step, instead of from the beginning. Valid values for start step are: import, staticID, setNullCoords, setProductRights, entryPts.

-stopAfter STOP_STEP
Makes the script stop after the given stop step, instead of running to the end. Use the same values to this option as you can use for option -startAt.

-noEntryPoints
Makes the script not run entry points.

-onlyPrintSourceInfo
Makes the script print info about the source and then return, disregarding other script control options such as -startAt etc. You cannot combile this option with -test. Only mandatory option is -poiSource.

";
   exit;
}

# Some "global" variables
##############################################################################
my $waspDbh = db_connect(); # The connection to the database.

my $sourceID;      # The WASP POI source ID.
my $prevSourceID;  # The WASP POI source ID of previous release of this data.
my $productID;     # The ID of the product in POIProducts.

my $sourceName;      # The WASP POI source name.
my $prevSourceName;  # The WASP POI source name of previous release.
my $productName;     # The name of the product in POIProducts.

my $productTrustSrcRef; # Tells if the ID in src ref can be used for finding
                        # the same POI in a previous source of this product.

my $productGeocodingCandidate; # Tells if product should do geocoding

# This one is true if the POIs are delivered with the map data, and that it
# therefore exists a chance that they have entry points  delivered with them.
my $mapDataPOIs; # The POIs are map data POIs

my @countriesDirs; # The countries directories to find EDVersions in.
my @epMapFiles;    # The map files to use when creating entry points.

my $latestPoiCountNbr; # Used for determining whether to use latitude bands
# in static migration.

my $query=""; # Used for SQL queries.
my $sth;      # Used for select query results;

my $nbrPOIsSaved = 0;
my $nbrEntryPointsSet = 0;


# Print methods
##############################################################################
if ( defined($opt_quiet) && defined($opt_verbose) ){
   errprint("Can't combine -verbose and -quiet options.");
   exit 1;
}
sub dprint1(@){
   if ( ! $opt_quiet ){
      dbgprint(@_);
   }
};
sub dprint2(@){
   if ( $opt_verbose ){
      dbgprint("[v]", @_);
   }
};

# Option to use when calling other script with support for quiet and verbose.
my $quietOrVerboseOpt="";
if ( $opt_verbose ){
   $quietOrVerboseOpt="-verbose";
}
elsif ( $opt_quiet ){
   $quietOrVerboseOpt="-quiet";
}


# Fetch values of some important globals needed early in the script
##############################################################################

# Make sure we have a source paremter.
if (!defined($opt_poiSource)){
    errprint("This command demands the option -poiSource to be set." .
             " Give -h for help.");
    exit 1;
}
# Fetch POI source ID and product.
$query =
    "SELECT POISources.ID, source, productID,".
    " productName, trustedSourceRef, geocodingCandidate, mapDataPOIs" .
    " FROM POISources, POIProducts WHERE source = '$opt_poiSource' AND" .
    " productID = POIProducts.ID";
dprint2 $query;
$sth = $waspDbh->prepare( $query );
$sth->execute();
($sourceID,
 $sourceName,
 $productID,
 $productName,
 $productTrustSrcRef,
 $productGeocodingCandidate,
 $mapDataPOIs) = $sth->fetchrow();

if ( $sth->rows == 0 ){
    dprint1("Query: $query");
    errprint("Invalid --poiSource: $opt_poiSource");
    exit 1;
}
elsif ( $sth->rows > 1 ){
    # This is very strange
    dprint1("Query: $query");
    errprint("Ambigous --poiSource: $opt_poiSource");        
    exit 1;
}
dprint1 "POI source:  $sourceName ($sourceID)"; 
dprint1 "POI product: $productName ($productID)";
$sth->finish();


# This one is true if the POIs are delivered with the map
# data, and that it therefore exists a chance that they have entry points 
# delivered with them.
if ($mapDataPOIs){
   timeprint "This POI set is a map POI set.\nvalidToVersion should be equal to validFromVersion.";
} else {
   timeprint "This POI set is NOT a map POI set.\nvalidToVersion should be set to NULL.";  
}



# Script control variables:
use vars qw( $runImport 
             $runGeocode 
             $runStaticID 
             $runInternalDblElm 
             $runFillNullCoords
             $runProductRights
             $runEntryPts
             $runInterSrcDblElm
             $runActivate );

# Scipt control
##############################################################################

# Script control default
$runImport=1;
$runStaticID=1;
$runFillNullCoords=1;
$runProductRights=1;
$runEntryPts=1;
# When adding to this list, make sure to carry out validity checks on the 
# options needed 



if ( defined $opt_startAt ){
   if ( $opt_startAt eq "import" ){
      # No change, this is the defalt start step.
   }
   elsif ( $opt_startAt eq "staticID" ){
      $runImport=0;
   }
   elsif ( $opt_startAt eq "setNullCoords" ){
      $runImport=0;
      $runStaticID=0;
   }
   elsif ( $opt_startAt eq "setProductRights" ) {
      $runImport=0;
      $runStaticID=0;
      $runFillNullCoords=0;
   }
   elsif ( $opt_startAt eq "entryPts" ){
      $runImport=0;
      $runStaticID=0;
      $runFillNullCoords=0;
      $runProductRights=0;
   }
   else {
      errprint("Invalid value: \"$opt_startAt\", given for -startAt option");
      exit 1;
   }
}

if ( defined $opt_stopAfter ){
   if ( $opt_stopAfter eq "import" ){
      $runStaticID=0;
      $runFillNullCoords=0;
      $runProductRights=0;
      $runEntryPts=0;
   }
   elsif ( $opt_stopAfter eq "staticID" ){
      $runFillNullCoords=0;
      $runProductRights=0;
      $runEntryPts=0;
   }
   elsif ( $opt_stopAfter eq "setNullCoords" ){
      $runProductRights=0;
      $runEntryPts=0;
   }
   elsif ( $opt_stopAfter eq "setProductRights" ){
      $runEntryPts=0;
   }
   elsif ( $opt_stopAfter eq "entryPts" ){
      # empty
   }
   else {
      errprint("Invalid value: \"$opt_stopAfter\", given for -stopAfter". 
               " option");
      exit 1;
   }
}

if ($mapDataPOIs){
   $runFillNullCoords=0;
}

# No entry points.
if ( defined $opt_noEntryPoints ){
   $runEntryPts=0;
}

if ( defined $opt_onlyPrintSourceInfo){
   # Keep this one last, so it can be combined with the other script control
   # options without them having any effect.
   $runImport=0;
   $runStaticID=0;
   $runFillNullCoords=0;
   $runProductRights=0;
   $runEntryPts=0;
}


# Check the computer
##############################################################################
print "\n";
dprint1 "Checking computer.";
open (VERSIONFILE, "< /etc/redhat-release")
    or die "Could not open version file";
my $versionString = <VERSIONFILE>;
chomp $versionString;
my $checkString = "CentOS release 4\.. (Final)";
if ( ! "$versionString" =~ /$checkString/ ){
   # Some modules used are only runnable on cent-OS computers.
   errprint("Check your computer");
   dprint1 ("   current version:     $versionString");
   dprint1 ("   should have matched: $checkString");
   exit 1;
}
close (VERSIONFILE);
dprint1 "Computer OK.";
dprint1 "";

##############################################################################

# Constants
##############################################################################

# Threshold value for using latitude bands in static migration.
my $noLatBandStaticMigMaxNbrPOIs = 2000000;


# Script directory
my $scriptDir="$genfilesScriptPath";
timeprint "Using script directory: $scriptDir";


# Logpath directory
my $logDirName="logpath";
if ( mkdir $logDirName ){
   # Create the logpath directory if it does not exist.
   dprint1 "Created directory $logDirName";
}
my $logPath=abs_path($logDirName);
timeprint "Logpath directory: $logPath";


# File path directory
my $fileDirName="filepath";
if ( mkdir $fileDirName ){
   # Create the filepath directory if it does not exist.
   dprint1 "Created directory $fileDirName";
}
my $filePath=abs_path($fileDirName);
timeprint "Filepath directory: $filePath";




# Execute command method
##############################################################################
# Fill this one with all sub scripts and programs to be used below, 
# so they can be tested.
my $poiImportExe = "$scriptDir/poiImport.pl";
my @scriptTestCommands=( "$scriptDir/poi_doublesCheck.pl -h",
                         "$scriptDir/poi_admin.pl -h",
                         "$poiImportExe -h");

if ( $runEntryPts ){
   push @scriptTestCommands, "./WASPExtractor -h";
}

sub executeCommand {
   my $command = $_[0];
   dprint2 "Executing: $command";

   # Check if this command is present among the ones tested.
   my $scriptFile = $command;
   $scriptFile =~ s/ .*$//;
   if ( ! grep /$scriptFile -h/, @scriptTestCommands ){
      if ( defined ($opt_test) ){
         errprint "Command: $scriptFile not present in \@scriptTestCommands";
         exit 1;
      }
      else {
         warnprint "Command: $scriptFile not present in \@scriptTestCommands";
      }
   }
   if ( ! defined($opt_test) ){
      `$command`;
      my $exitCode = $?;
      if ( $exitCode != 0 ){
         errprint "Command failed with exit code $exitCode";
         exit($exitCode);
      }
   }
   else {
      dprint2 "Not executing command in test mode.";
   }
}

sub bzipIfExists{
   if( defined($_[0])){
      my $fileName = $_[0];
      if ( -s $_[0]){
         # File exists and is non zero size.
         if ( ! -d $_[0] ){
            # File is not a directory.
            dprint1 "Zipping: $fileName";
            `bzip2 $fileName`;
         }
      }
   }   
}

# Use this function to get a unique logfile name, not to override
# any existing logfiles from previous calls
# Appends X until not-existing filename is found
sub getNotExistingLogFileName{
   my $logFileName = $_[0];
   
   my $logFileExist = 0;
   if ( -s $logFileName ) {
      $logFileExist = 1;
   }
   #print " logFileName $logFileName logFileExist=$logFileExist\n";
   while ( $logFileExist ) {
      $logFileName =~ s/.log$/X.log/;
      if ( -s $logFileName ) {
         $logFileExist = 1;
      } else {
         if ( -s "$logFileName.bz2" ) {
            $logFileExist = 1;
         } else {
            $logFileExist = 0;
         }
      }
      #print " logFileName $logFileName logFileExist=$logFileExist\n";
   }
   return "$logFileName";
}

sub printSourceInfo {
   my $localSourceID = $_[0];
   if (!$opt_test){
      db_disconnect($waspDbh);
      $waspDbh = db_connect(); # The connection to the database.
      timeprint "";
      timeprint "Counting POIs of this source...";
   
      ( my $nbrTotal, 
        my $nbrCountries, 
        my $nbrDeleted, 
        my $nbrInUse, 
        my $nbrStaticOK, 
        my $nbrPosOK,
        my $nbrEtrPtsOK ) = getSourceInfo($waspDbh, $localSourceID);
      $latestPoiCountNbr = $nbrTotal;
      my $localSourceName = getSourceNameFromID($waspDbh, $localSourceID);

      my $countryStr = "countries";
      if ( $nbrCountries == 1 ) {
         $countryStr = "country";
      }

      my $posNotOkStr = "";
      if ( $nbrPosOK != $nbrTotal ){
         $posNotOkStr = "    (not OK: ". ($nbrTotal - $nbrPosOK) . ")";
      }
      my $staticNotOkStr = "";
      if ( ($nbrStaticOK != $nbrTotal) && ($nbrStaticOK != 0) ){
         $staticNotOkStr = "    (not OK: ". ($nbrTotal - $nbrStaticOK) . ")";
      }

      my $etrPtsNotOkStr = "";
      if ( ($nbrEtrPtsOK != $nbrTotal) && ($nbrEtrPtsOK != 0) ){
         $etrPtsNotOkStr = "    (not OK: ". ($nbrTotal - $nbrEtrPtsOK) . ")";
      }



      timeprint "Found POIs of source $localSourceName in $nbrCountries".
          " $countryStr.";
      timeprint "COUNT DETAILS";
      timeprint "   Total nbr:  $nbrTotal";
      timeprint "   In use:     $nbrInUse";
      timeprint "   Pos OK:     $nbrPosOK" . "$posNotOkStr";
      timeprint "   Deleted:    $nbrDeleted";
      timeprint "   Static OK : $nbrStaticOK" . "$staticNotOkStr";
      timeprint "   Etr pts OK: $nbrEtrPtsOK" . "$etrPtsNotOkStr";
      timeprint "";

      if ( ! defined $nbrTotal ) { $nbrTotal = 0; }
      if ( ! defined $nbrEtrPtsOK ) { $nbrEtrPtsOK = 0; }
      return ($nbrTotal, $nbrEtrPtsOK);
   }
}


# Checking parameters
##############################################################################
timeprint "";
timeprint "Basic checking of in-parameters";


# Special for print POI info.
if ( $opt_onlyPrintSourceInfo && $opt_test ){
   errprint "You cannot combine options -test and -onlyPrintSourceInfo,".
       " Exits!";
   exit 1;
}


if ( $runImport ||
     $runStaticID || 
     $runFillNullCoords ||
     $runEntryPts )
{
   
   if ($runImport) {
        
      # If runImport, make sure we have a valid $opt_poiData.
      if (!defined ($opt_poiData) ){
         errprint("Required option -poiData not given. Use -h for help");
         exit 1;
      }
      # Further checking of data parameter validity should be done in each
      # import script. 



      # Geocoding
      if ( $productGeocodingCandidate ) {
         if (! defined $opt_geocodeIfEmptyPos ) {
            errprint("This product is a geocoding candidate. Need to specify -geocodeIfEmptyPos 1 or 0\n");
            exit 1;
         }
      }
   }

   if ($runImport || $runEntryPts ){
      # Validate countries directory
      if (!defined($opt_mapCountriesDirs)){
         errprint("This command reqires the option -mapCountriesDirs to".
                  " be set." .
                  " Give -h for help.");
         exit 1;
      }

      my %mapVerByCountryID=(); # Stores the map version of each country.
      if ( !($opt_mapCountriesDirs eq "NOT_USED") ){
         # It is possible to not use the countries directories. In this case the
         # map version of the POI data must be given by the -mapVerOverride option.

         @countriesDirs = split(",", $opt_mapCountriesDirs);
         my $i=0;
         for my $countryDir (@countriesDirs){
            $i++;
            dprint1("Countries dir $i: $countryDir");
            if ( ! testDir($countryDir ) ){
               errprint("Countries directory: $countryDir not OK");
               exit 1;
            }
         }

         # Get map version (map origin) for each country that is defined
         # in POICountries table. The map origin will be used as
         # validFromVersion for the imported POIs 
         # (if mapPOIs also for the validToVersion)
         getMapVersionByCountryID($waspDbh, $opt_mapCountriesDirs, 
                                  \%mapVerByCountryID,
                                  $opt_okWithMissingCountries);
         foreach my $key (keys(%mapVerByCountryID)){
            dprint2 "OK: $mapVerByCountryID{$key}";
         }
         dprint1 "Countries directories OK.";
      }
      else {
         dbgprint "NOT using map version country dirs.";
         if (! defined $opt_mapVerOverride ){
            errprint "You need to use option -mapVerOverride when not using ".
                "map version country dirs.";
            exit 1;
         }
      }
      
      # Validates that the ovverride parameters are OK.
      if ( defined $opt_mapVerOverride ){
         handleMapVerOverrides($waspDbh, 
                               $opt_mapVerOverride, 
                               \%mapVerByCountryID);
      }
      timeprint "";

   }
   
   if ( $runStaticID || $runFillNullCoords){
      
      # Validate previuos source of this product. If this is a first import of
      # a specific poi product, it is OK not to have a previous product.
      ($prevSourceID, $prevSourceName) = getPrevPoiSource($waspDbh,
                                                          $sourceID);
      
      if ( ! $prevSourceID ){
         if ( $opt_newProduct ){
            warnprint "No previous source of this product, which is OK with ".
                "option -newProduct.";
         }
         else {
            errprint "Could not find previous source for source ID: $sourceID".
                " of product ID: $productID.";
            errprint "If this is the first source of this".
                " product, override this check with option -newProduct.";
            exit 1;
         }
      }
      else {
         dprint1 "Previous POI source: $prevSourceName ($prevSourceID)";
      }
   }
   
   if ( $runStaticID ) {
      if ( $opt_newProduct ) {
         dprint1 "For static: only creating new static ids " .
                 "due to option -newProduct.";
      }
      if ( $opt_createNewStaticIds ) {
         dprint1 "For static: only creating new static ids " .
                 "due to option -createNewStaticIds.";
      }
   }

   # If run entry points, make sure the map data is OK
   if ($runEntryPts) {
      # Gather the map files to use when creating entry points. Exits on error.
      dprint1 "Checking map files from country dirs";
      my @allEpMapFiles = getMapsFromCountryDirs ($waspDbh, 
                                               $opt_mapCountriesDirs,
                                               undef, # sourceID
                                               undef, # mapDataPOIs
                                               $opt_okWithMissingCountries);
      # allEpMapFiles is usually not used. Refined below before creating 
      # entry points.

      # Check that we have an mc2.prop file in current directory.
      if ( ! testFile("mc2.prop") ){
         errprint "Needed mc2.prop file missing, exits.";
         exit 1;
      }
   }


   # Test run some scripts to see that all modules and include script can be
   # loaded.
   timeprint "";
   timeprint "Testing sub script and program execution.";
   my $tmpFileName = mktemp( "poi_autoImport_execTestXXXXXXXXX" );
   foreach my $command (@scriptTestCommands) {
      dprint1 "Executing: $command";
      `$command >& $tmpFileName`;
      my $exitCode=$?;
      dprint2 "Exit code: $exitCode";
      if ( $exitCode != 0 ){
         errprint "Failed script execution with exit code: $exitCode. Find".
             " script log in $tmpFileName";
         errprint "================ Dumping $tmpFileName";
         open (TMPFILE, "<", $tmpFileName);
         while( <TMPFILE> ) {
            print $_;
         }
         errprint "================";
         close (TMPFILE);
         unlink("$tmpFileName");
         exit 1;
      }
   }
   unlink("$tmpFileName");
}


# Common operations
##############################################################################


# Print how many POIs we have when starting the script.
if (!$opt_noInitCount){
   ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);
}

if ($opt_onlyPrintSourceInfo){
   # When this option is given, we always stop after the initial printing of
   # POI info.
   
   exit;
}



# Importing data
##############################################################################
#
#   Standard options for poi_add... scritps.
#
#   -verbose
#       Tells the script to use both dprint1 and dprint2 prints. Default is
#       only dprint1.
#
#   -quiet
#       Tells the script to use neighter dprint1 or dprint2 prints. Default is
#       dprint1.
#
#   -logDir DIRECTORY
#       The directory where all logs will be written.
#
#   -mapCountriesDirs COUNTRYDIR1,COUNTRYDIR2, ...
#       Commaseparated list without space with directories to use when 
#       resolving map version to use in each country. All countries in WASP
#       should be included in these directories and no duplicates should be
#       present.
#
#   -dataDir DIRECTORY | -dataFile FILE
#       Directory or file where the source data of the POIs is located.
#
#   -poiSourceID WASPSOURCEID
#       The value to use for POIMain.source in WASP.
#
if ( $runImport ){
   timeprint "";
   timeprint "Importing data to WASP.";
   dprint1 "Parameters used:";
   dprint1 "  Countries: ", $opt_mapCountriesDirs;
   if (defined $opt_mapVerOverride ){
      dprint1 "  Map version override: ", $opt_mapVerOverride;
   }
   dprint1 "  POISource:  $sourceName ($sourceID)";
   dprint1 "  POIProduct: $productName ($productID)";
   dprint1 "  POIData:    $opt_poiData";
   if (defined $opt_geocodeIfEmptyPos) {
      dprint1 "  GeocodeIfEmptyPos:    $opt_geocodeIfEmptyPos";
      # stop after import to create geocoding file
      if ($opt_geocodeIfEmptyPos) {
         $runGeocode=1;
         $runStaticID=0;
         $runFillNullCoords=0;
         $runProductRights=0;
         $runEntryPts=0;
      }
   }
   dprint1 "";


   # 
   #
   # GDF POI data format ======================================================
   if ( 0
           )
   {
      timeprint "Importing GDF POI data";
      my $supplierName = getProductSupplierName($waspDbh, $productID);
      dprint1 "  Supplier name: $supplierName";
      my $mapVerOverrideOpt = "";
      if ( defined($opt_mapVerOverride) ){
         $mapVerOverrideOpt = "-o $opt_mapVerOverride";
      }
      my $m_opt = "";
      if ($mapDataPOIs) {
         $m_opt = " -m";
      }

      my $logFileName = 
         getNotExistingLogFileName("$logPath/poi_readGDFPOIFormat.log");
      my $command="$poiImportExe".
#          " -p".
          " -f readGDFPOIFormat".
          $m_opt.
          " -d $opt_mapCountriesDirs".
          " $mapVerOverrideOpt".
          " -s $sourceName".
          " -u '$supplierName'".
          " -i 1".
          " -X $poiCategoryXMLFile".
          " $opt_poiData".
          " >& $logFileName";
      dprint1 "  Starting import, see logs in $logFileName.";
      executeCommand("$command");
      bzipIfExists("$logFileName");
      
   }
   #
   #
   # CPIF POIs from for instance mid/mif ======================================
   elsif ( 
           ($productID == 51 )   # OSM_eu
           or ($productID == 52) # TA_eu_oss
           ) 
   {
      timeprint "Importing CPIF POI data.";
      my $supplierName = getProductSupplierName($waspDbh, $productID);
      dprint1 "  Supplier name: $supplierName";
      my $mapVerOverrideOpt = "";
      if ( defined($opt_mapVerOverride) ){
         $mapVerOverrideOpt = "-o $opt_mapVerOverride";
      }
      
      my $logFileName = 
         getNotExistingLogFileName("$logPath/poi_readCPIF.log");
      my $m_opt = "";
      if ($mapDataPOIs) {
         $m_opt = " -m";
      }
      my $okWithMissingCountriesOpt = "";
      if ( defined $opt_okWithMissingCountries ) {
         $okWithMissingCountriesOpt = " -M";
      }
      my $command="$poiImportExe".
          #" -p".
          " -n" . # uncomment to allow empty coordinates
          " -f readCPIF".
          " -d $opt_mapCountriesDirs".
          " $okWithMissingCountriesOpt".
          " $mapVerOverrideOpt".
          " -u '$supplierName'".
          $m_opt.
          " -s $sourceName".
          " -i 1".
          " -X $poiCategoryXMLFile".
          " $opt_poiData".
          " >& $logFileName";
      dprint1 "  Starting import, see logs in $logFileName.";
      executeCommand("$command");
      bzipIfExists("$logFileName");
   }
   #
   #
   # Import routine missing ===================================================
   else {
      errprint "No import routine for source: $sourceName ($sourceID), ".
        "product: $productName ($productID), exits!";
      die;
   }
   ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);
} # runImport




##############################################################################
#
# XXX Create an option to control this stop.
#
# If we do not have trusted source IDs, stop the script here, geocode and 
# restart with runStatic as the first step.
#
##############################################################################

##############################################################################
#
# Prepare for geocoding
#
##############################################################################
if ( $runGeocode ) {
   dprint1 "Creating geocoding file.";
   timeprint "";
   timeprint "Print file for geocoding.";
   dprint1 "Parameters used:";
   dprint1 "  POISource:             $sourceName ($sourceID)";

   my $coordFile_start = "geocoding_";
   my $coordFile;
   my @countriesOfSource;
   # poi_admin.pl script wants sourceID and countryID
   # we want to do one file per country
   # and -geocFile filename of file to geocode
   if ( $productGeocodingCandidate ){
      # loop countries of source
      @countriesOfSource = getCountriesOfSource($waspDbh, $sourceID);
      foreach my $c (@countriesOfSource) {
         my $countryCodeIso = getISO3166CodeFromCountryID($waspDbh, $c);
         if (! defined $countryCodeIso ) {
            dprint1 "Did not get iso country code. Exiting\n";
            exit;
         }
         $coordFile = $coordFile_start . $countryCodeIso . ".txt";

         my $command = "$scriptDir/poi_admin.pl -exportPOIsWithNoPos".
             " -poiSource $sourceName -poiCountry $countryCodeIso -verbose".
             " -geocFile $coordFile" .
             " >& $logPath/poi_exportPOIsWithNoPos.log";
         executeCommand("$command");
         
         ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);

      }
   } else {
      timeprint "Not printing file for geocoding on non-geocodingCandidate source reference";
   }
}

# Creating/migrating static IDs
##############################################################################
if ( $runStaticID ){
   timeprint "";
   timeprint "Handling static IDs.";
   dprint1 "Parameters used:";
   dprint1 "  POISource:             $sourceName ($sourceID)";
   if ( defined($prevSourceName) ){
      dprint1 "  Previous POI source:   $prevSourceName ($prevSourceID)";
   }
   else {
      dprint1 "  No previous POI source.";
   }
   dprint1 "  POIProduct:            $productName ($productID)";
   dprint1 "  Trusted src ref (bool) $productTrustSrcRef";
   my $dateStr = dateAndTime();
   $dateStr =~ s/[ :]/_/g;
   my $manualCheckFile = $filePath . "/nonMatchStatic_" . $dateStr . ".txt";
   dprint1 "Manual check file: $manualCheckFile";
   my $command;
   if ( $opt_newProduct || $opt_createNewStaticIds ){
      # Creating new static IDs.
      timeprint "Creating new static IDs for all POIs";

      my $quietPrint = "";
      if ( $opt_createNewStaticIds ) {
         $quietPrint = "-q";
      }

      $command = "$scriptDir/poi_doublesCheck.pl -f newPOIsToPOIStatic".
          " -m $sourceName ".
          " $quietPrint ".
          " >& $logPath/poi_staticCreate.log";
      executeCommand("$command");
   }
   else {
      # Using static IDs from previous release of this product.
      timeprint "Migrating static POI IDs.";

      my $latBandOpt = "";
      if (defined($latestPoiCountNbr) && 
          ($latestPoiCountNbr > $noLatBandStaticMigMaxNbrPOIs)){
         dprint1 "Using latitude bands in static migration: " .
                 "latestPoiCountNbr=$latestPoiCountNbr";
         if ( !defined ($opt_latitudeBand) ){
            $opt_latitudeBand = 0;
         }
         dprint1 "  Starting static migration from latitude band".
             "  $opt_latitudeBand";
         $latBandOpt = "-b $opt_latitudeBand";
      }
      

      my @staticCountries = (); # The countries to migrate static of
      if ( ($latBandOpt eq "") &&
           (staticIDsPresentInSource($waspDbh, $sourceID) ) ){
         dprint1 "  Static IDs already present in this source, and not using,".
             " latitude bands. Checking which countries have no static.";
         @staticCountries = getNonStaticCountries($waspDbh, $sourceID);
      }
      elsif ($latBandOpt eq ""){
          dprint1 "  No static IDs already present in this source.";
      }
      if (scalar(@staticCountries) == 0){
         push @staticCountries, "xxxxx"; # A marker telling to migreate static
         # for all countries at once.
      }
      
      my $trustSrcRefFlag = ""; # Don't trust source reference ID
      if ( $productTrustSrcRef ){
         $trustSrcRefFlag = "-r"; # Trust src ref.
      }
      foreach my $countryName (@staticCountries){
          my $countryOpt = "";
          if ($countryName ne "xxxxx"){
              # If the country name is set to xxxxx it means that we should do
              # all countries at once.
              $countryOpt = "-c \"$countryName\"";
              dprint1 "  Migrating for $countryName";
          }
          
          $command = "$scriptDir/poi_doublesCheck.pl -f migratePOIStatic".
              " -m $sourceName -s $prevSourceName $trustSrcRefFlag ".
              " -n $manualCheckFile $latBandOpt $countryOpt".
              " >& $logPath/poi_staticMig.log";
          executeCommand("$command");
      }

      # This sets the static IDs of all POIs not found in previous 
      # POI set.
      dprint1 "  Setting static IDs for rest.";
      $command = "$scriptDir/poi_doublesCheck.pl -f newPOIsToPOIStatic".
          " -m $sourceName -d".
          " >& $logPath/poi_staticCreateForRest.log";
      executeCommand("$command");

      bzipIfExists("$logPath/poi_staticCreateForRest.log");
   }

   ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);
} # runStaticID


# Fill coordinate values from corresponding POI of previous source
##############################################################################
if ( $runFillNullCoords ) {
   if ($opt_newProduct ){
      timeprint "";
      timeprint "NOT filling blank coordinates for a new product.";
   }
   else {

      timeprint "";
      timeprint "Fill blank coordinates via static ID.";
      dprint1 "Parameters used:";
      dprint1 "  POISource:             $sourceName ($sourceID)";
      dprint1 "  Trusted src ref (bool) $productTrustSrcRef";
      if ( $productTrustSrcRef ){
         my $command = "$scriptDir/poi_admin.pl -setPosFromPrevSource".
             " -poiSource $sourceName -verbose".
             " >& $logPath/poi_adminSetPosFromPrevSrc.log";
         executeCommand("$command");
         
         ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);
      }
      else {
         timeprint "Not filling blank coordinates via static ID when we have".
             " no".
             " trusted source reference, continues.";
      }
   }
}   

# Set rights according to the product of this source
##############################################################################
if ( $runProductRights ) {
   timeprint "";
   timeprint "Set rights according to product of this source.";
   dprint1 "Parameters used:";
   dprint1 "  POISource:             $sourceName ($sourceID)";
   dprint1 "  POIProduct:            $productName ($productID)";

   my $command = "$scriptDir/poi_admin.pl -setSourceProductRights" .
      " -poiSource $sourceName -verbose" .
      " >& $logPath/poi_adminSetSourceProductRights.log";
   executeCommand("$command");

   ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);

}

# Create entry points
##############################################################################
if ($mapDataPOIs){
   createEntryPoints();
}
else {
   createEntryPoints();
}



#   timeprint "runGeocode";
### Perhaps use static ID to find position of previous POI location.
#



#if ( $runInternalDblElm ){
#   # internal duplicate elimination (within the imported source)
#   timeprint "runInternalDblElm";
#}





#if ( $runInterSrcDblElm ){
#   # inter duplicate elimination (between the imported source and other sources)
#   timeprint "runInterSrcDblElm";
#}
#if ( $runActivate ){
#   timeprint "runActivate";
#}


db_disconnect($waspDbh);

timeprint("");
timeprint("Done!");

######################################################
# Sub functions
######################################################

# Creating entry points
##############################################################################
sub createEntryPoints {
   # The startAt/stopAfter may give that we should not run entry points
   if ( $runEntryPts ){
      timeprint "";
      timeprint "Creating entry points.";
      if ($nbrPOIsSaved == $nbrEntryPointsSet) {
         dprint1 "All POIs already have entryPoints. Will return";
         return;
      }
      dprint1 "Parameters used:";
      dprint1 "  Countries: ", @countriesDirs;
      if (defined $opt_mapVerOverride ){
         dprint1 "  Map version override: ", $opt_mapVerOverride;
      }
      dprint1 "  POISource:  $sourceName ($sourceID)";

      # Fetch map files of the countries where POIs of this source exists.
      # Country names of these countries are also printed to the log here.
      dprint1 "";
      dprint1 "Fetching map files...";
      
      my @epMapFiles = ();
      if (!$opt_test){ 
         # Time consuming, don't run in test mode.

         @epMapFiles = getMapsFromCountryDirs($waspDbh, 
                                              $opt_mapCountriesDirs,
                                              $sourceID,
                                              $mapDataPOIs );
         dprint1 "Using " .scalar(@epMapFiles). " map files.";
         if ( scalar(@epMapFiles)== 0 ){
            errprint "No map files to use for creating entry points.";
            dbgprint "Want to use all countries if we have map data POIs, " .
                     "else only using the map files of countries considered ".
                     "detailed.";
            exit(1);
         }
      }


      # Normal case.
      my $mapFiles="";
      foreach my $file (@epMapFiles) {
         $mapFiles.="$file ";
      }


      my $command="./WASPExtractor -c --source=$sourceName ".
          "$mapFiles".
          " >& $logPath/we_createEP_$sourceName.log";
      #dbgprint ($command);
      executeCommand("$command"); 

      #$mapFiles =~ s/[ ]/\n/g;
      #dprint1 $mapFiles;

      ($nbrPOIsSaved, $nbrEntryPointsSet) = printSourceInfo($sourceID);
   }
} # createEntryPoints


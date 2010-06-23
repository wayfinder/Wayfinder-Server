#!/usr/bin/perl -w
#
#
# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;
use strict;

# Print command line.
print "Command: $0";
foreach my $arg (@ARGV){
   print " $arg";
}
print "\n\n";

use Getopt::Long ':config',
                  'no_auto_abbrev', # Impossible to use -s for --source.
                  'no_ignore_case'  # Case sensitive options.
;


# Use perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;
use PerlWASPTools;
use POIObject;



use vars qw( $opt_help $opt_verbose $opt_quiet 
             $opt_setPosFromPrevSource
             $opt_setSourceInUse
             $opt_setSourceNotInUse
             $opt_setPOICategoryFromPOIType
             $opt_rmPoisOfSource
             $opt_rmStaticOfSource
             $opt_rmCategoriesOfSource
             $opt_rmEntryPtsOfSource
             $opt_setSourceProductRights 
             $opt_poiSource 
             $opt_poiCountry 
             $opt_poiIdMin
             $opt_poiIdMax
             $opt_setNullValidTo
             $opt_noUpdateValidTo
             $opt_exportPOIsWithNoPos
             $opt_geocFile
             $opt_importPosFromGeocodeResult
);
GetOptions('help|h',
           'verbose|v',
           'quiet',
           # Function options
           'setPosFromPrevSource',
           'setSourceInUse',
           'setSourceNotInUse',
           'setPOICategoryFromPOIType',
           'rmPoisOfSource',
           'rmStaticOfSource',
           'rmCategoriesOfSource',
           'rmEntryPtsOfSource',
           'setSourceProductRights',
           # Indata options
           'poiSource=s',
           'poiCountry=s',
           'poiIdMin=s',
           'poiIdMax=s',
           'setNullValidTo',
           'noUpdateValidTo',
           'exportPOIsWithNoPos',
           'geocFile=s',
           'importPosFromGeocodeResult',
           ) or die "Use -help for help.";

# Print help message.
if ( $opt_help ){
   print "USAGE poi_admin COMMAND_OPTION [PARAMETER_OPTIONS]

COMMAND OPTIONS:

-setPosFromPrevSource
Valid parameters:  -poiSource
Takes all POIs of source -poiSource with no symbol coordinates and sets the 
symbol coordinate to the symbol coordinate from corresponding POI of prevoius
POI source. Symbol coordinate = POIMain.lat and POIMain.lon.

-setSourceInUse
Function -setSourceInUse sets inUse value for the source -poiSource to true. It has to have one of the following mandatory options -noUpdateValidTo or -setNullValidTo set.
If option -setNullValidTo is defined, it will set validToVersion to NULL for all pois of the source. 
If validToVersion should not be set (modified), give the option -noUpdateValidTo.

-setSourceNotInUse
Function -setSourceNotInUse compares current -poiSource with next source of product. The comparision is done per country of -poiSource. 
If validFromVersion is equal, it sets inUse to false.
If validFromVersion differs, validToVersion of -poiSource and current country will be set to validFromVersion of next source - 1 (minus one).
The option -setSourceNotInUse can have additional option -noUpdateValidTo which will only set inUse to false on all records of the source, and keep validToVersion unchanged.

-setPOICategoryFromPOIType
Function -setPOICategoryFromPOIType will check POIs for source -poiSource and if POI has no category set, will try to set a category from POIType.

-rmPoisOfSource
Removes all POI data from the database of POIs with source matching the POI source given by mandatory option -poiSource and country given by mandatory option -poiCountry. Use optional option -poiIdMin and/or -poiIdMax, if you want to remove only the latest added POIs of a source.

-rmStaticOfSource
Removes all static POI data from the POIStatic table  POIs with source matching the POI source given by mandatory option -poiSource and country given by mandatory option -poiCountry.

-rmCategoriesOfSource
Removes all Category POI data from the POICategories table  POIs with source matching the POI source given by mandatory option -poiSource and country given by mandatory option -poiCountry.

-rmEntryPtsOfSource
Removes all entry points from the POIEntryPoints table  POIs with source matching the POI source given by mandatory option -poiSource and country given by mandatory option -poiCountry.

-setSourceProductRights
Sets the defaultMapRight for all POIs in a source according to the product it belongs to.

-exportPOIsWithNoPos
Exports POIs in source -poiSource and country -poiCountry that don't have coordinates, to a geocoding file \"-geocFile\".

-importPosFromGeocodeResult
Reads geocoded file -geocFile and for POIs with coordinates, updates POI if it has no coordinates for source -poiSource and country -poiCountry.


PARAMETER OPTIONS:

-poiSource POI_SOURCE_WASP_NAME
Give the WASP name of the POI source.

-poiCountry POI_COUNTRY_ISO3166_1_ALPHA2
Give the ISo 3166-1 Alfa 2 code of the country. Give value \"xx\" for all countries.

-poiIdMin WASP_ID
The ID of POIMain.ID. The lowest ID of the WASP database that will be affected. Only a few command options take this parameter option.

-poiIdMax WASP_ID
The ID of POIMain.ID. The largest ID of the WASP database that will be affected. Only a few command options take this parameter option.

-geocFile filename
Filename to write file to geocode to / to read coordinates from.

";
   exit;
}

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
      my @printArray = ();
      foreach my $elm (@_) {
         push @printArray, split("\n", $elm);
      }
      foreach my $row (@printArray) {
         $row =~ s/^/[v] /;
         $row =~ s/$/\n/;
      }
      if (scalar(@printArray) > 0 ){
         # Remove last new line.
         chomp ($printArray[scalar(@printArray-1)]);
      }
      dbgprint(@printArray);
   }
};



# Some variables set from parameter options
my $sourceID;
my $sourceName;
my $sourceID2;
my $sourceName2;
my $productID;
my $productName;
my $productTrustSrcRef;
my $prevSourceID;
my $prevSourceName;
my $countryID;
my $countryName;
my $geocFile;
my $waspDbh = db_connect();


#Handle parameter options
##############################################################################
if ( $opt_setPosFromPrevSource || 
     $opt_rmPoisOfSource ||
     $opt_rmStaticOfSource ||
     $opt_rmCategoriesOfSource ||
     $opt_rmEntryPtsOfSource ||
     $opt_setSourceInUse ||
     $opt_setSourceNotInUse ||
     $opt_setPOICategoryFromPOIType ||
     $opt_setSourceProductRights ||
     $opt_exportPOIsWithNoPos ||
     $opt_importPosFromGeocodeResult ){
   if (!defined($opt_poiSource)){
      errprint "Missing value for -poiSource";
      die;
   }


   # Fetch POI source ID and product.
   my $query =
       "SELECT POISources.ID, source, productID,".
       " productName, trustedSourceRef" .
       " FROM POISources, POIProducts WHERE source = '$opt_poiSource' AND" .
       " productID = POIProducts.ID";
   dprint2 $query;
   my $sth = $waspDbh->prepare( $query );
   $sth->execute() or die "Could not execute query";
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
   ($sourceID,
    $sourceName,
    $productID,
    $productName,
    $productTrustSrcRef ) = $sth->fetchrow();
   dprint1 "POI source:  $sourceName ($sourceID)"; 
   dprint1 "POI product: $productName ($productID)";

   ($prevSourceID, $prevSourceName) = getPrevPoiSource($waspDbh,
                                                       $sourceID);
}
if ( $opt_rmPoisOfSource || $opt_rmStaticOfSource || 
      $opt_rmCategoriesOfSource || $opt_rmEntryPtsOfSource || 
      $opt_exportPOIsWithNoPos || $opt_importPosFromGeocodeResult ){
   # Handle country.
   if (!defined($opt_poiCountry)){
      errprint("Missing option -poiCountry");
      die;
   }
   if ( $opt_poiCountry eq "xx" ){
      # Country code xx means all countries.
      $countryID = 0xffffffff;
      $countryName = "All countries";
   }
   else {
      my $upperCaseCountryCode = uc($opt_poiCountry);
      my $query =
          "SELECT ID, country" .
          " FROM POICountries WHERE iso3166_1_alpha2 = '$upperCaseCountryCode';";
      dprint2 $query;
      my $sth = $waspDbh->prepare( $query );
      $sth->execute() or die "Could not execute query";
      if ( $sth->rows == 0 ){
         dprint1("Query: $query");
         errprint("Invalid -poiCountry: $opt_poiCountry");
         exit 1;
      }
      elsif ( $sth->rows > 1 ){
         # This is very strange
         dprint1("Query: $query");
         errprint("Ambigous -poiCountry: $opt_poiCountry");        
         exit 1;
      }
      ($countryID, $countryName) = $sth->fetchrow();
   }
   dprint1 "POI country: $countryName ($countryID)"; 
}

# check input geocFile
if ( $opt_exportPOIsWithNoPos || $opt_importPosFromGeocodeResult ) {
   if (defined $opt_geocFile and $opt_geocFile ne '') {
      if ($opt_exportPOIsWithNoPos) {
         if (!-e $opt_geocFile) {
            $geocFile = $opt_geocFile;
         } else {
            errprint("File $opt_geocFile exists, use new filename\n");
            exit 1;
         }
      } elsif ($opt_importPosFromGeocodeResult) {
         if (!-e $opt_geocFile) {
            errprint("Can not find file with coordinates: $opt_geocFile. Exiting.\n");
            exit 1;
         } else {
            $geocFile = $opt_geocFile;
         }
      }
   } else {
      errprint("Need input on filename for geocode file\n");
      exit 1;
    }
}

# Handle commands.
##############################################################################
#=============================================================================
if ( $opt_setPosFromPrevSource ){
   # Check sanity
   if ( ! defined $sourceID ){
      errprint "No source, exits";
      exit 1;
   }
   if ( ! defined $prevSourceID ){
      errprint "No previous source, exits";
      exit 1;
   }
   if ( ! defined $productTrustSrcRef ){
      errprint "This product has not a trusted source ref, it is vital for ".
          "this operation, exits";
      exit 1;
   }

   # Fetch the IDs of POIs with no symbol coordinate.
   my @poiIdAndStatics = ();
   my $query =
       "SELECT POIMain.ID, POIStatic.staticID".
       " FROM POIMain, POIStatic".
       " WHERE".
       " POIMain.source = $sourceID AND".
       " POIMain.lat IS NULL AND".
       " POIMain.lon IS NULL AND".
       " POIMain.ID = POIStatic.poiID;";
   dprint2 $query;
   my $sth = $waspDbh->prepare( $query );
   $sth->execute() or die "Failed: $query";;
   my $nbrNoPosPOIs = $sth->rows;
   dprint1 "Found ". $nbrNoPosPOIs . " POIs with no position.";
   while ( (my $poiID, my $staticID) = $sth->fetchrow()) {
      my %poiIdAndStatic = ('poiID' => $poiID,
                            'staticID' => $staticID);
      push(@poiIdAndStatics, \%poiIdAndStatic);
   }
   

   
   # Find the corresponding POIs from previous source and set the symbol
   # coordinate from it.
   my $nbrPositionsSet = 0;
   foreach my $poiIdAndStatic (sort(@poiIdAndStatics)){
      $query =
          "SELECT POIMain.ID, POIMain.lat, POIMain.lon".
          " FROM POIMain, POIStatic".
          " WHERE".
          " POIStatic.staticID = $poiIdAndStatic->{'staticID'} AND".
          " POIMain.source = $prevSourceID AND".
          " POIMain.lat IS NOT NULL AND".
          " POIMain.lon IS NOT NULL AND".
          " POIMain.deleted != 1 AND".
       " POIMain.ID = POIStatic.poiID;";
      
      dprint2 $query;
      $sth = $waspDbh->prepare( $query );
      $sth->execute() or die "Failed: $query";;
      if ( $sth->rows == 0 ){
         dprint1("No prev source POI for WASP ID: $poiIdAndStatic->{'poiID'}");
      }
      elsif ( $sth->rows > 1 ){
         # This is very strange
         dprint1("Query: $query");
         dprint1("Dumping ambigues hits");
         while ( (my $poiID, my $lat, my $lon) = $sth->fetchrow()) {      
            dprint1("  $poiID");
         }
         errprint("Ambigous prev source POI for WASP ID: ".
                  "$poiIdAndStatic->{'poiID'}");
         exit 1;
      }
      else {
         # All is fine
         (my $poiID,
          my $lat,
          my $lon ) = $sth->fetchrow();
         $nbrPositionsSet++;
         $query = "UPDATE POIMain".
             " SET lat = $lat, lon = $lon".
             " WHERE ID = $poiIdAndStatic->{'poiID'}";
         $sth = $waspDbh->prepare( $query );
         $sth->execute() or die "Failed: $query";;
         dprint1("Old ID: $poiID, new ID: $poiIdAndStatic->{'poiID'},".
                 " Updated pos:($lat,$lon)");
      }
   }
   dprint1 "Set position on " . $nbrPositionsSet . " of ". $nbrNoPosPOIs;
}
#
#
#
#=============================================================================
elsif ( $opt_setSourceProductRights ){
   timeprint "";
   timeprint "Updating rights in POIMain from defaultMapRight for their product.";
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   if (!defined($productID) || !defined($productName)){
      errprint "Something wrong with poiProduct";
      exit 1;
   }
   my $query = "SELECT defaultMapRight FROM POIProducts WHERE " .
         "ID = ?";
   dprint2 $query;
   my $sth = $waspDbh->prepare( $query );
   $sth->execute($productID) or die "Failed: $query";;
   if ( $sth->rows == 0 ){
      dprint1("Query: $query");
      errprint("Invalid poiProduct: $productName");
      exit 1;
   }
   elsif ( $sth->rows > 1 ){
      # This is very strange
      dprint1("Query: $query");
      errprint("Ambigous -poiProduct: $productName");        
      exit 1;
   }
   my ($defaultMapRight) = $sth->fetchrow();
            
   timeprint "Updates value of rights of the POIs.";
   $query =
       "UPDATE POIMain".
       " SET rights = '$defaultMapRight', lastModified = now()".
       " WHERE source = $sourceID and rights != '$defaultMapRight';";
   dprint2 $query;
   $sth = $waspDbh->prepare( $query );
   $sth->execute() or die "Failed: $query";;
   timeprint "Done!";

}
#
#
#
#=============================================================================
elsif ( $opt_setPOICategoryFromPOIType ) {
   timeprint "";
   timeprint "Adding categories to POICategories using values in POIType.";
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   my $query = "SELECT ID, typeID FROM POIMain left join POITypes " .
         " on POIMain.ID = POITypes.poiID WHERE " .
         "source = $sourceID";
   my $catQuery = "SELECT catID FROM POICategories WHERE poiID = ?";
   my $catSth = $waspDbh->prepare( $catQuery );
   my $sth = $waspDbh->prepare( $query );
   my $insertQuery = "INSERT INTO POICategories set poiID=?, catID=?";
   my $insertSth = $waspDbh->prepare( $insertQuery );
   $sth->execute() or die "Failed: $query";
   while (my ($poiID, $typeID) = $sth->fetchrow()) {
      $catSth->execute($poiID) or die "Failed $catQuery, $poiID";
      if ($catSth->rows == 0) {
         my $categoryID = getPOICatFromPOIType($waspDbh, $typeID);
         if (defined $categoryID) {
            $insertSth->execute($poiID, $categoryID) or die "Failed $insertQuery";
            dprint1("$insertQuery");
            dprint1("\tfound category, poiID=$poiID, typeID=$typeID, category=$categoryID");
         }
      } else {
         dprint1("POI with ID $poiID already has a category. Skipping.");
      }
   }
}
#
#
#
#=============================================================================
elsif ( $opt_rmPoisOfSource ){
   timeprint "";
   timeprint "Removing POIs from database.";
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   if (!defined($countryID) || !defined($countryName)){
      errprint "Something wrong with mandatory option -poiCountry";
      exit 1;
   }
   my $deleteMode = "allTables";
   handleDeletePOIsFromSource($sourceID, 
                              $sourceName, 
                              $countryID, 
                              $deleteMode,
                              $opt_poiIdMin,
                              $opt_poiIdMax);
}
#
#
#
#=============================================================================
elsif ( $opt_rmStaticOfSource ){
   timeprint "";
   timeprint "Removing static from POIStatic.";
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   if (!defined($countryID) || !defined($countryName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   my $deleteMode = "onlyStatic";
   handleDeletePOIsFromSource($sourceID, $sourceName, $countryID, $deleteMode);
}
#
#
#
#=============================================================================
elsif ( $opt_rmCategoriesOfSource ){
   timeprint "";
   timeprint "Removing categories from POICategories.";
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   if (!defined($countryID) || !defined($countryName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   my $deleteMode = "onlyCategories";
   handleDeletePOIsFromSource($sourceID, $sourceName, $countryID, $deleteMode);
}
#
#
#
#=============================================================================
elsif ( $opt_rmEntryPtsOfSource ){
   timeprint "";
   timeprint "Removing entry points from POIEntryPoints.";
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   if (!defined($countryID) || !defined($countryName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   my $deleteMode = "onlyEntryPts";
   handleDeletePOIsFromSource($sourceID, $sourceName, $countryID, $deleteMode);
}
#
#
#
#=============================================================================
elsif ( $opt_setSourceInUse || $opt_setSourceNotInUse ){
   # Handle in parameters.
   if (!defined($sourceID) || !defined($sourceName)){
      errprint "Something wrong with mandatory option -poiSource";
      exit 1;
   }
   my $inUseVal = 0;
   my $inUseStr = "false";
   my $update_valid_to = 0;
   my $valid_to_value;
   my $valid_to_statement = "";
   my $nextSourceID;
   my $nextSourceName;
   my @countriesOfSource;
   my $thisCountryValidFromVersion;
   my $nextCountryValidFromVersion;
   my $newValidToVersion;
   my $update_in_use_query;
   my $update_valid_to_query;
   my $update_in_use_sth;
   my $update_valid_to_sth;
   my $inUseProblem = 0;
   my %update_inuse_hash;
   my %update_validto_hash;
   my $something_to_change = 0;

   if ( $opt_setSourceInUse && $opt_setSourceNotInUse ){
      errprint "Cannot set both -setSourceInUse and -setSourceNotInUse at".
          " the same time";
      exit 1;
   }
   if ($opt_setSourceInUse) {
      if ($opt_setNullValidTo && $opt_noUpdateValidTo) {
         errprint "Cannot select both -setNullValidTo and -noUpdateValidTo " .
            "at the same time";
         exit 1;
      }
      if (!$opt_setNullValidTo && !$opt_noUpdateValidTo) {
         errprint "When doing -setSourceInUse, need to give option " .
            "-setNullValidTo or -noUpdateValidTo";
         exit 1;
      }
   }
   # print and get current values of inUse
   my ($currentInUse, $all_inuse_value) = getInUseInfoFromSource($waspDbh, $sourceID);

   if ( $opt_setSourceInUse ){
      if ($opt_setNullValidTo || $opt_noUpdateValidTo) {
         $inUseVal = 1;
         $inUseStr = "true";

         if ($all_inuse_value == 1) {
            errprint "You want to set inUse to $inUseStr but it is already $inUseVal. Exiting.";
            exit 1;
         }

         if ($opt_setNullValidTo) {
            $update_valid_to = 1;
            $valid_to_value = "NULL";
            $valid_to_statement = "validToVersion = $valid_to_value, ";
         }
         # selection is to set inUse to true
         dbgprint "You have selected to set inUse to $inUseStr ";
         if ($update_valid_to) {
            dbgprint "and $valid_to_statement for source $sourceID.";
         }
      }
      # This continues below, after the elsif
      
   } elsif ($opt_setSourceNotInUse) {

      if (! $opt_noUpdateValidTo) {
         @countriesOfSource = getCountriesOfSource($waspDbh, $sourceID);
         if (scalar(@countriesOfSource) > 0) {
            ($nextSourceID, $nextSourceName) = getNextPoiSource($waspDbh, $sourceID);
            if (defined $nextSourceID) {
               dbgprint "Will set validToVersion per country.";
               timeprint "";
               
               $update_in_use_query = "UPDATE POIMain SET inUse = $inUseVal, " .
                  "lastModified=now() WHERE " .
                  "source = $sourceID AND country = ?";
               $update_in_use_sth = $waspDbh->prepare($update_in_use_query) or die "Could not prepare query: $update_in_use_query";
            
               $update_valid_to_query = "UPDATE POIMain SET " .
                  "validToVersion = ?, lastModified=now() WHERE " .
                  "source = $sourceID AND country = ?";
               $update_valid_to_sth = $waspDbh->prepare($update_valid_to_query) or die "Could not prepare query: $update_valid_to_query";

               foreach my $countryID (@countriesOfSource) {
                  my (@currInUse) = getInUseFromSourceAndCountry($waspDbh, $sourceID, $countryID);
                  my $amount_of_elements = scalar(@currInUse);
                  if ($amount_of_elements > 1) {
                     $inUseProblem = 1;
                  }
                  $thisCountryValidFromVersion = 
                     getMostFrequentValidFromVersionOfSourceAndCountry($waspDbh, $sourceID, $countryID);
                  $nextCountryValidFromVersion = 
                     getMostFrequentValidFromVersionOfSourceAndCountry($waspDbh, $nextSourceID, $countryID);

                  if (defined $thisCountryValidFromVersion and defined $nextCountryValidFromVersion) {

                     if ($thisCountryValidFromVersion != $nextCountryValidFromVersion) {

                        $newValidToVersion = $nextCountryValidFromVersion - 1;
                        dbgprint "validFromVersion of source $sourceName ($thisCountryValidFromVersion) " .
                           "and $nextSourceName ($nextCountryValidFromVersion) differ. " .
                           "Will set validToVersion of source $sourceID in country $countryID to ".
                           "$newValidToVersion.";
                        if ($currentInUse->{$countryID}->{'validToVersion'} eq $newValidToVersion) {
                           dbgprint "   Wanting to set same validToVersion as it is now. Won't change.";
                        } else {
                           # store new value
                           $update_validto_hash{$countryID}{'validToVersion'} = $newValidToVersion;
                           $something_to_change = 1;
                        }

                     } else {
                        dbgprint "validFromVersion of source $sourceName ($thisCountryValidFromVersion) " .
                           "and $nextSourceName ($nextCountryValidFromVersion) for country $countryID don't differ. ";
                        if ($inUseVal == $currentInUse->{$countryID}->{'inUse'}) {
                              dbgprint "   Wanting to set same inUse as it is now ($inUseVal). Won't change.";

                        } else {
                           # store new value
                           $update_inuse_hash{$countryID}{'inUse'} = $inUseStr;
                              dbgprint "   Will set inUse to $inUseStr.";
                           $something_to_change = 1;
                        }
                     }
                  } else {
                     dbgprint "One validFromVersion is undefined for country $countryID. Will set inUse to $inUseStr.";

                     if ($inUseVal == $currentInUse->{$countryID}->{'inUse'}) {
                        dbgprint "   Wanting to set same inUse as it is now ($inUseVal). Won't change.";
                     } else {
                        # the pois of this country can have in use set to false
                        # as one of the validFromVersions is undefined
                        # store new value
                        $update_inuse_hash{$countryID}{'inUse'} = $inUseStr;
                        $something_to_change = 1;
                     }
                  }
               }
               if ($inUseProblem) {
                  errprint "One or more countries have more than one inUse value. This is incorrect. Exiting.";
                  exit 1;
               }
               
               if (!$something_to_change) {
                  errprint "No changes to make. Exiting.";
                  exit 1;
               } else {
                  dbgprint "";
                  dbgprint "Will do updates. Press ctrl-c to skip.";
                  dbgprint "";
                  # Give some time for breaking the script.
                  for (my $i=10; $i > -1; $i = $i-1){ 
                     sleep(1);   
                     print $i . " sec to continue. " . "\n\10\10\10\15";
                  }
                  print "\n";
                  timeprint "Continues";

                  foreach $countryID (keys %update_inuse_hash) {
                     dbgprint "here we would update inUse ($update_in_use_query) for country $countryID";
                     $update_in_use_sth->execute($countryID) or die "Could not execute query: $update_in_use_query";
                  }
                  foreach $countryID (keys %update_validto_hash) {
                     $newValidToVersion = $update_validto_hash{$countryID}{'validToVersion'};
                     dbgprint "here we would update validTo ($update_valid_to_query)";
                     dbgprint "validTo=$update_validto_hash{$countryID}{'validToVersion'} where country = $countryID";
                     $update_valid_to_sth->execute($newValidToVersion, $countryID) or die "Could not execute query: $update_valid_to_query";
                  }
               
                  $update_in_use_sth->finish();
                  $update_valid_to_sth->finish();
                  timeprint "Done!";
                  exit;
               }
            } else {
               errprint "Did not find nextSourceID for this source $sourceID. Exiting.";
               exit 1;
            }
         } else {
            errprint "Seems source has no POIs, as no countries were found. Exiting.";
            exit 1;
         }
      } else {
         if ($all_inuse_value == 0) {
            errprint "You want to set inUse to $inUseVal but it is already $inUseVal. Exiting.";
            exit 1;
         }
         # continues below
      }
   }
   
   timeprint "";
   timeprint "Setting in use to $inUseStr.";
   if ($update_valid_to) {
      timeprint "Setting $valid_to_statement";
   }
   timeprint "Start by counting POIs to update.";
   my $query =
       "SELECT count(*) FROM POIMain".
       " WHERE source = $sourceID;";
   dprint2 $query;
   my $sth = $waspDbh->prepare( $query );
   $sth->execute() or die "Failed: $query";
   (my $nbrPOIs) = $sth->fetchrow();
   timeprint "Will update inUse value of $nbrPOIs POIs.";

   $query =
       "UPDATE POIMain".
       " SET inUse = $inUseVal, $valid_to_statement lastModified = now()".
       " WHERE source = $sourceID;";
   dbgprint $query;

   # Give some time for breaking the script.
   for (my $i=10; $i > -1; $i = $i-1){ 
      sleep(1);   
      print $i . " sec to continue. " . "\n\10\10\10\15";
   }
   print "\n";
   timeprint "Continues";

   timeprint "Updates value of in use flag of the POIs.";
   $sth = $waspDbh->prepare( $query );
   $sth->execute() or die "Failed: $query";
   timeprint "Done!";
}
#
#
#
#=============================================================================
elsif ( $opt_exportPOIsWithNoPos ){
   my $nbrToGeoc = 0;
   my $tmpGeocFile = $geocFile . ".1";
   timeprint "";
   timeprint "Will export POIs with no coordinates\n";
   # Check sanity
   if ( ! defined $sourceID ){
      errprint "No source, exits";
      exit 1;
   }
   if ( ! defined $countryID ) {
      errprint "No country, exits";
      exit 1;
   }
   if ($countryID == 0xffffffff) {
      errprint "Please take one country at a time, geocoding will be easier\n";
      exit 1;
   }

   my $query = "SELECT ID FROM POIMain WHERE source = $sourceID AND " . 
               "country = $countryID AND (lat = '' or lat is NULL) ORDER BY ID";
   my $sth = $waspDbh->prepare($query) or die "Cannot prepare query \"$query\"\n";
   $sth->execute() or die "Cannot execute query \"$query\"\n";
   while ((my $poiid) = $sth->fetchrow()) {
      # Build a POI object
      my $poiObject = POIObject->newObject( );
      $poiObject->setDBH($waspDbh);

      $poiObject->{m_id} = $poiid;
      # read poi from WASP
      $poiObject->readFromWASP();

      my $prioStreetNumberCombination = 1;
      $poiObject->writeGeocodeFile( $geocFile, $prioStreetNumberCombination );
      $nbrToGeoc += 1;
   }

   if ($nbrToGeoc < 1) {
      dbgprint("No POIs without coordinates were found\n");
      dbgprint("Exiting\n");
      exit 1;
   }
   dbgprint "Wrote $nbrToGeoc pois to geocode file $geocFile";
   dbgprint "Now sorting on zip area...";
   `sort -t";" -k5 -o$tmpGeocFile $geocFile`;
   `mv $tmpGeocFile $geocFile`;
   dbgprint "Done!";
}
#
#
#
#=============================================================================
elsif ( $opt_importPosFromGeocodeResult ) {
   timeprint "";
   timeprint "Will import coordinates for POIs with no coordinates\n";
   my $nbrRows = 0;
   my $updatedCoords = 0;
   my $hasCoords = 0;
   my $alreadyHasCoords = 0;
   # Check sanity
   if ( ! defined $sourceID ){
      errprint "No source, exits";
      exit 1;
   }
   if ( ! defined $countryID ) {
      errprint "No country, exits";
      exit 1;
   }
   if ($countryID == 0xffffffff) {
      errprint "Please take one country at a time, geocoding will be easier\n";
      exit 1;
   }
   my $selectQuery = "SELECT ID, lat, lon FROM POIMain WHERE sourceReference = ? " .
               " AND source = $sourceID\n";
   my $updateQuery = "UPDATE POIMain set lastModified=now(), lat=?, lon=? where ID=?";
   my $sth = $waspDbh->prepare($selectQuery);
   my $sthUpdate = $waspDbh->prepare($updateQuery);
   # example geocoded row:
   # 10933297;united kingdom;Knutsford Road 73;WA4 1AB;Warrington;636817919;-30427866;
   open (COORDS, "< $geocFile")
          or die "Could not open file with coordinates";
   while (<COORDS>) {
      $nbrRows++;
      chomp;
      my $line = $_;
      my (@row) = split(/;/, $line);
      if ($row[1] eq 'united kingdom') {
         $row[1] = "england";
      }
      if (!defined getCountryIDFromName($waspDbh, $row[1])) {
         errprint "Check format of geocoded file. Country name not valid\n";
         exit 1;
      }
      if ( (!defined $row[0]) or ($row[0] eq '') ) {
         errprint "Line has no source reference. Exiting\n";
      }
      if (defined $row[5] and defined $row[6]) {
         if ( ($row[5] =~ m/^-?\d+$/) and ($row[6] =~ m/^-?\d+$/) ) {
            print "POI srcRef $row[0], in $row[1]\n";
            $hasCoords++;
            $sth->execute($row[0]);
            my ($poiID, $lat, $lon) = $sth->fetchrow();
            if (defined $poiID) {
               print "poi found in db with poiID $poiID\n";
               if (!defined $lat or $lat eq '') {
                  timeprint "Will update coord in WASP";
                  timeprint "$updateQuery, $row[5], $row[6], $poiID\n";
                  $sthUpdate->execute( $row[5], $row[6], $poiID);
                  $updatedCoords++;
               } else {
                  $alreadyHasCoords++;
               }
            }
         } else {
            print "Coordinates don't seem like correct mc2. Will not update.\n";
         }
      } else {
         print "Coordinates are not defined! Cannot update.\n";
      }
   }
   timeprint "Updated coordinates for $updatedCoords of $hasCoords POIs with coordinates in geocoded file.";
   timeprint "In WASP $alreadyHasCoords POIs already had coordinates.";
   timeprint "File had $nbrRows rows\n";
   timeprint "Done!";
}
#
#
#
#=============================================================================
else {
   print "\nNo command given, use -help to see your options.\n\n";
}


# Methods
##############################################################################

# This action deletes all POIs from a given source. Dangerous.
sub handleDeletePOIsFromSource {
   my $sourceID = $_[0];
   my $sourceName = $_[1];
   my $countryID = $_[2];
   my $deleteMode = $_[3];
   my $poiIdMin = $_[4];
   my $poiIdMax = $_[5];
   
   if ( !defined($sourceID) ) {
      die "handleDeletePOIsFromSource: No source given\n";
   }
   if ( !defined($countryID) ) {
      die "handleDeletePOIsFromSource: No country given\n";
   }
   if (!defined($sourceName)) {
      die "handleDeletePOIsFromSource: No valid source defined\n";
   }
   if ( !defined($deleteMode) ) {
      die "handleDeletePOIsFromSource: No delete mode given";
   }
   if ( defined($poiIdMin) ){
      dprint1 "Using min PIO ID: $poiIdMin";
   }
   if ( defined($poiIdMax) ){
      dprint1 "Using max PIO ID: $poiIdMax";
   }
   my $deleteEP = 0;
   my $deleteInfo = 0;
   my $deleteTypes = 0;
   my $deleteNames = 0;
   my $deleteStatic = 0;
   my $deleteCategories = 0;
   my $deleteMain = 0;
   if  ($deleteMode eq "onlyInfo") {
      $deleteInfo = 1;
   } elsif ($deleteMode eq "allTables") {
      $deleteEP = 1;
      $deleteInfo = 1;
      $deleteTypes = 1;
      $deleteNames = 1;
      $deleteStatic = 1;
      $deleteCategories = 1;
      $deleteMain = 1;
   } elsif ($deleteMode eq "onlyStatic") {
      $deleteStatic = 1;
   } elsif ($deleteMode eq "onlyCategories") {
      $deleteCategories = 1;
   } elsif ($deleteMode eq "onlyEntryPts") {
      $deleteEP = 1;
   } else {
      die "handleDeletePOIsFromSource: unknown deleteMode $deleteMode";
   }

   my $allCountries;
   if ($countryID == 0xffffffff) {
      $allCountries = 1;
   }

   dprint1 "Delete from POIs with source $sourceName ($sourceID) in country ".
       "$countryName ($countryID)";
   dprint1 "Delete from tables: ";
   if ($deleteEP) {     dprint1 "  POIEntryPoints";}
   if ($deleteInfo) {   dprint1 "  POIInfo";}
   if ($deleteTypes) {  dprint1 "  POITypes";}
   if ($deleteNames) {  dprint1 "  POINames";}
   if ($deleteStatic) { dprint1 "  POIStatic";}
   if ($deleteCategories) { dprint1 "  POICategories";}
   if ($deleteMain) {   dprint1 "  POIMain";}
   dprint1 "";
   dprint1 "Counting number pois - Press ctrl-c to interrupt";
   
   # Handle special options.
   my $countryQueryEnd;
   if (!$allCountries) {
      $countryQueryEnd = " AND country=$countryID";
   } else {
      $countryQueryEnd = "";
   }
   my $poiIdMinQuery = "";
   if ( defined($poiIdMin) ){
      $poiIdMinQuery = " AND POIMain.ID >= $poiIdMin ";
   }
   my $poiIdMaxQuery = "";
   if ( defined($poiIdMax) ){
      $poiIdMaxQuery = " AND POIMain.ID <= $poiIdMax ";
   }

   #Get number of POIs to be deleted
   my $countQuery = "select count(ID) from POIMain " .
                    "where source=$sourceID" . $poiIdMinQuery. $poiIdMaxQuery. $countryQueryEnd;
   dprint1 "$countQuery";
   my $sth2 = $waspDbh->prepare($countQuery);
   $sth2->execute;
   my $count = $sth2->fetchrow();
   if ( $count == 0 ) {
      dprint1 "Found $count POIs to delete";
      die "No pois to delete - exit\n";
   }
   dprint1 "Found $count POIs of which data will be deleted,".
       " press ctrl-c NOW to interrupt.";
   for (my $i=10; $i > -1; $i = $i-1){ 
      sleep(1);   
      print $i . " sec to continue. " . "\n\10\10\10\15";
   }
   print "\n";
   dprint1 "Continues";
   

   #Delete POIs
   dprint1 "Starts deleting data of POIs, too late to interrupt";
   if ( $deleteEP ) {
      dprint1 "Starts deleting with poi entry points";
      my $deleteEntryQuery = "DELETE POIEntryPoints FROM POIEntryPoints, " .
                             "POIMain WHERE POIMain.ID=POIEntryPoints.poiID " .
                             "AND source=$sourceID" . 
                             $poiIdMinQuery . 
                             $poiIdMaxQuery . 
                             $countryQueryEnd;
      dprint1 $deleteEntryQuery;
      my $sth3 = $waspDbh->prepare($deleteEntryQuery);
      $sth3->execute or die "Failed DELETE POIEntryPoints";
      dprint1 "Poi entry points deleted";
   }
   if ( $deleteInfo ) {
      dprint1 "Starts deleting poi info";
      my $deleteInfoQuery = "DELETE POIInfo FROM POIInfo, " . 
                            "POIMain WHERE POIMain.ID=POIInfo.poiID " .
                            "AND source=$sourceID" .
                            $poiIdMinQuery . 
                            $poiIdMaxQuery . 
                            $countryQueryEnd;
      dprint1 $deleteInfoQuery;
      my $sth4 = $waspDbh->prepare($deleteInfoQuery);
      $sth4->execute or die "Failed DELETE POIInfo";
      dprint1 "Poi info deleted";
   }
   if ( $deleteTypes ) {
      dprint1 "Starts deleting poi types";
      my $deleteTypesQuery = "DELETE POITypes FROM POITypes, " . 
                             "POIMain WHERE POIMain.ID=POITypes.poiID " .
                             "AND source=$sourceID" .
                             $poiIdMinQuery . 
                             $poiIdMaxQuery . 
                             $countryQueryEnd;
      dprint1 $deleteTypesQuery;
      my $sth5 = $waspDbh->prepare($deleteTypesQuery);
      $sth5->execute or die "Failed DELETE POITypes";
      dprint1 "Poi types deleted";
   }
   if ( $deleteNames ) {
      dprint1 "Starts deleting poi names";
      my $deleteNamesQuery = "DELETE POINames FROM POINames, " . 
                             "POIMain WHERE POIMain.ID=POINames.poiID " .
                             "AND source=$sourceID" .
                             $poiIdMinQuery .
                             $poiIdMaxQuery . 
                             $countryQueryEnd;
      dprint1 $deleteNamesQuery;
      my $sth6 = $waspDbh->prepare($deleteNamesQuery);
      $sth6->execute or die "Failed DELETE POINames";
      dprint1 "Poi names deleted";
   }
   if ( $deleteStatic ) {
      dprint1 "Starts deleting poi static";
      my $deleteStaticQuery = "DELETE POIStatic".
          " FROM POIStatic, POIMain". 
          " WHERE".
          " POIMain.ID=POIStatic.poiID AND".
          " source=$sourceID" .
          $poiIdMinQuery .
          $poiIdMaxQuery . 
          $countryQueryEnd;
      dprint1 $deleteStaticQuery;
      my $sth7 = $waspDbh->prepare($deleteStaticQuery);
      $sth7->execute or die "Failed DELETE POIStatic";
      dprint1 "Poi static deleted";
   }
   if ( $deleteCategories ) {
      dprint1 "Starts deleting poi categories";
      my $deleteCategoriesQuery = "DELETE POICategories".
          " FROM POICategories, POIMain". 
          " WHERE".
          " POIMain.ID=POICategories.poiID AND".
          " source=$sourceID" .
          $poiIdMinQuery .
          $poiIdMaxQuery . 
          $countryQueryEnd;
      dprint1 $deleteCategoriesQuery;
      my $sth8 = $waspDbh->prepare($deleteCategoriesQuery);
      $sth8->execute or die "Failed DELETE POICategories";
      dprint1 "Poi categories deleted";
   }
   if ( $deleteMain ) {
      dprint1 "Starts deleting poi main";
      my $deleteMainQuery = "DELETE FROM POIMain " . 
                            "WHERE source=$sourceID" .
                             $poiIdMinQuery .
                             $poiIdMaxQuery . 
                             $countryQueryEnd;
      dprint1 $deleteMainQuery;
      my $sth10 = $waspDbh->prepare($deleteMainQuery);
      $sth10->execute or die "Failed DELETE POIMain";
      dprint1 "Poi main deleted";
   }
   dprint1 "Data of all " . $count . 
       " POIs deleted from source=$sourceName countryID=$countryID";
} #handleDeletePOIsFromSource




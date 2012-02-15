#!/usr/bin/perl -w
#
#
# This perl package contains common functions used in communication 
# with the WASP (POI or ED) database.

package PerlWASPTools;


use lib ".";
use PerlTools; # for dbgprint
use PerlCategoriesObject;

use strict;
use DBI;
use File::Find;
use File::Basename;

require Exporter;
our @ISA       = qw( Exporter );
our @EXPORT    = qw( &getCountryIDFromName 
                     &getCountryNameFromID
                     &getCountryIDFromISO3166Code 
                     &getISO3166CodeFromCountryID
                     &getCountryDirNameFromIso3166Code
                     &getCountryNameFromISO3166Code
                     &getCountryDirNamesOfPoiSource
                     &getDetailLevelOfCountryID
                     &getAllCountryDirNames
                     &countryWaspNameToDirName

                     &getLanguageIDFromName
                     &getLanguageNameFromID
                     &getLangNameFromDodonaLangCode
                     &getDodonaLangCodeFromLangName

                     &getSourceIDFromName 
                     &getSourceNameFromID
                     &getVersionIDFromName 
                     &getVersionNameFromID
                     &getPOITypeIDFromTypeName
                     &getPOITypeNameFromTypeID
                     &getPOICatFromPOIType
                     &getPOICatDescriptFromPOICatID
                     
                     &getProductID
                     &getProductSupplierName
                     &getPrevPoiSource
                     &getNextPoiSource
                     &getSourcesOfProduct
                     &getMostFrequentValidFromVersionOfSourceAndCountry
                     &getInUseFromSourceAndCountry
                     &getSourceIsMapPoi

                     &getStaticIDForPOIID
                     &getCountriesOfSource
                     &staticIDsPresentInSource
                     &getNonStaticCountries

                     &getSourceInfo
                     &getInUseInfoFromSource
                     &getNbrPOIsOfSourceID
                     &getNbrStaticIDsOfSourceID

                     &getMaxAttributeTypeID 
                     &getMaxAttributeTypeIDLessThanInfo

                     &getMapVersionByCountryID
                     &handleMapVerOverrides
                     &getMapsFromCountryDirs
                     &mapVersionToEDVersionID
                     &getDodonaKeysByCategory

                     &storePOIs
                     &db_connect &db_disconnect );

#our $VERSION     = 1.00;


sub getDodonaKeysByCategory {
   my $dbh = $_[0];

   my %result = ();
   my $query=
       "select catID, dodonaStringKey from POICategoryTypes;";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   while ( (my $catID, my $dodonaKey) = $sth->fetchrow() ){
      $result{$catID} = $dodonaKey;
   }
   return %result;
}


sub getCountryIDFromName {
   my $dbh = $_[0];
   my $countryName = $_[1];
   my $countryQuery=
      "select ID from POICountries where country like '$countryName';";
   my $sth = $dbh->prepare($countryQuery);
   $sth->execute or die "Query execute failed: $countryQuery";
   my $countryID = $sth->fetchrow();
   return $countryID;
}

sub getCountryNameFromID {
   my $dbh = $_[0];
   my $countryID = $_[1];
   my $countryQuery=
      "select country from POICountries where ID = $countryID;";
   my $sth = $dbh->prepare($countryQuery);
   $sth->execute or die "Query execute failed: $countryQuery";
   my $countryName = $sth->fetchrow();
   return $countryName;
}

sub getCountryIDFromISO3166Code {
   my $dbh = $_[0];
   my $countryCode = $_[1];
   my $countryQuery =
      "SELECT ID FROM POICountries WHERE " .
      "iso3166_1_alpha2 = '$countryCode'";
   my $sth = $dbh->prepare($countryQuery);
   $sth->execute or die "Query execute failed: $countryQuery";
   my $countryID = $sth->fetchrow();
   return $countryID;
}

sub getISO3166CodeFromCountryID {
   my $dbh = $_[0];
   my $countryID = $_[1];
   my $countryQuery =
      "SELECT iso3166_1_alpha2 FROM POICountries WHERE ID = $countryID;";
   my $sth = $dbh->prepare($countryQuery);
   $sth->execute or die "Query execute failed: $countryQuery";
   my $isoCode = $sth->fetchrow();
   return $isoCode;
}

sub getCountryNameFromISO3166Code {
   my $dbh = $_[0];
   my $countryCode = $_[1];
   my $countryQuery =
      "SELECT country FROM POICountries WHERE " .
      "iso3166_1_alpha2 = '$countryCode'";
   my $sth = $dbh->prepare($countryQuery);
   $sth->execute or die "Query execute failed: $countryQuery";
   my $country = $sth->fetchrow();
   return $country;
}

sub getLanguageNameFromID {
   my $dbh = $_[0];
   my $langID = $_[1];
   my $langQuery =
      "SELECT langName FROM POINameLanguages WHERE ID = $langID;";
   my $sth = $dbh->prepare($langQuery);
   $sth->execute or die "Query execute failed: $langQuery";
   my $langName = $sth->fetchrow();
   return $langName;
}

sub getLanguageIDFromName {
   my $dbh = $_[0];
   my $langName = $_[1];
   my $langQuery =
      "SELECT ID FROM POINameLanguages WHERE langName LIKE '$langName';";
   my $sth = $dbh->prepare($langQuery);
   $sth->execute or die "Query execute failed: $langQuery";
   my $langID = $sth->fetchrow();
   return $langID;
}

sub getLangNameFromDodonaLangCode {
   my $dbh = $_[0];
   my $dodonaLangCode = $_[1];
   my $langQuery =
       "select langName " .
       "from POINameLanguages where ".
       "dodonaLangID = '" . $dodonaLangCode . "';";
   my $sth = $dbh->prepare($langQuery);
   $sth->execute or die "Query execute failed: $langQuery";
   my $langID = $sth->fetchrow();
   return $langID;
}

sub getDodonaLangCodeFromLangName {
   my $dbh = $_[0];
   my $langName = $_[1];
   my $langQuery =
       "select dodonaLangID " .
       "from POINameLanguages where ".
       "langName = '" . $langName . "';";
   my $sth = $dbh->prepare($langQuery);
   $sth->execute or die "Query execute failed: $langQuery";
   my $dodonaLang = $sth->fetchrow();
   return $dodonaLang;
}

sub getSourceIDFromName {
   my $dbh = $_[0];
   my $sourceName = $_[1];
   my $sourceQuery =
      "SELECT ID FROM POISources WHERE source LIKE '$sourceName';";
   my $sth = $dbh->prepare($sourceQuery);
   $sth->execute or die "Query execute failed: $sourceQuery";
   my $sourceID = $sth->fetchrow();
   return $sourceID;
}

sub getSourceNameFromID {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $sourceQuery = 
      "SELECT source FROM POISources WHERE ID=$sourceID;";
   my $sth = $dbh->prepare($sourceQuery);
   $sth->execute or die "Query execute failed: $sourceQuery";
   my $sourceName = $sth->fetchrow();
   return $sourceName;
}

sub getNbrPOIsOfSourceID {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $query =
      "SELECT count(*) FROM POIMain WHERE source = $sourceID";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $count = $sth->fetchrow();
   return $count;
}

sub getNbrStaticIDsOfSourceID {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $query =
      "SELECT count(*) FROM POIMain join POIStatic on POIMain.ID = POIStatic.poiID WHERE source = $sourceID";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $count = $sth->fetchrow();
   return $count;
}

sub staticIDsPresentInSource {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $query =
      "SELECT * FROM POIMain join POIStatic on POIMain.ID = POIStatic.poiID WHERE source = $sourceID LIMIT 1";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   return ( $sth->rows > 0 );
}


sub getNonStaticCountries {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $query =
       "SELECT DISTINCT POICountries.country".
       " FROM ( POIMain left join POIStatic on POIMain.ID = POIStatic.poiID )".
       " join POICountries on POIMain.country = POICountries.ID".
       " WHERE source = $sourceID and POIStatic.staticID is NULL";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my @resultArray = ();
   while (my $countryDbName = $sth->fetchrow()){
       push @resultArray, $countryDbName;
   }
   return @resultArray;
}


sub getVersionIDFromName {
   my $dbh = $_[0];
   my $versionName = $_[1];
   my $versionQuery =
      "SELECT ID FROM EDVersion WHERE version LIKE '$versionName';";
   my $sth = $dbh->prepare($versionQuery);
   $sth->execute or die "Query execute failed: $versionQuery";
   my $versionID = $sth->fetchrow();
   return $versionID;
}
sub getVersionNameFromID {
   my $dbh = $_[0];
   my $versionID = $_[1];
   my $versionQuery = 
      "SELECT version FROM EDVersion WHERE ID=$versionID;";
   my $sth = $dbh->prepare($versionQuery);
   $sth->execute or die "Query execute failed: $versionQuery";
   my $versionName = $sth->fetchrow();
   return $versionName;
}


# Retuns undef if it is not possible to lookup a category of a the type.
sub getPOICatFromPOIType {
   my $dbh = $_[0];
   my $poiType = $_[1];
   my $query = "SELECT catID FROM POICategoryTypes " .
      "WHERE poiTypeID = " . $poiType;
   my $sth = $dbh->prepare($query);
   $sth->execute() or die "Query execute failed: $query\n";
   my $poiCatID = $sth->fetchrow();
   return $poiCatID;
}

sub getPOICatDescriptFromPOICatID {
   my $dbh = $_[0];
   my $catID = $_[1];
   my $query = "SELECT description FROM POICategoryTypes " .
      "WHERE catID = " . $catID;
   my $sth = $dbh->prepare($query);
   $sth->execute() or die "Query execute failed: $query\n";
   my $descript = $sth->fetchrow();
   if ( ! defined $descript ) {
      $descript = "-no match-";
   }
   return $descript;
}


sub getPOITypeIDFromTypeName {
   my $dbh = $_[0];
   my $typeName = $_[1];
   my $query = 
      "SELECT ID FROM POITypeTypes where typeName like '$typeName';";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $poiTypeID = $sth->fetchrow();
   return $poiTypeID;
}

sub getPOITypeNameFromTypeID {
   my $dbh = $_[0];
   my $typeID = $_[1];
   my $query = 
      "SELECT typeName FROM POITypeTypes where ID = $typeID;";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $poiTypeName = $sth->fetchrow();
   return $poiTypeName;
}

sub getMaxAttributeTypeID {
   my $dbh = $_[0];
   my $query = 
      "SELECT max(id) FROM POIAttributeTypes;";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $maxID = $sth->fetchrow();
   return $maxID;
}
sub getMaxAttributeTypeIDLessThanInfo {
   my $dbh = $_[0];
   my $query = 
      "SELECT max(id) FROM POIAttributeTypes where id < 1000;";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $maxID = $sth->fetchrow();
   return $maxID;
}

sub getCountryDirNamesOfPoiSource {
   my $dbh = $_[0];
   my $poiSourceID = $_[1];
   my $query = 
      "SELECT DISTINCT POICountries.country, POICountries.detailLevel " .
      "FROM POICountries inner join POIMain " . 
      "     on POIMain.country = POICountries.ID " .
      "WHERE POIMain.source = $poiSourceID " .
      "order by POICountries.country;";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my @resultArray = ();
   dbgprint "Printing countries (only detailed are used) of source:" .
       getSourceNameFromID($dbh, $poiSourceID);
   while ( (my $countryDbName, my $detailed) = $sth->fetchrow() ){
      my $detailedStr = "[DETAILED]";
      if ( !$detailed ){
         $detailedStr = "[NOT DETAILED]";
      }
      dbgprint "   " . $countryDbName."\t".$detailedStr;
      push @resultArray, countryWaspNameToDirName($countryDbName);
   }
   return @resultArray;
} # getCountryDirNamesOfPoiSource


sub getCountryDirNameFromIso3166Code {
   my $dbh = $_[0];
   my $iso3166Code = $_[1];
   if ( !defined($iso3166Code) ){
      errprint "PerlWASPTools::iso3166CodeToCountryDirName iso3166Code not ".
          "defined";
   }
   my $countryName = getCountryNameFromISO3166Code($dbh, $iso3166Code);
   return countryWaspNameToDirName($countryName);
} # getCountryDirNameFromIso3166Code

sub countryWaspNameToDirName {
   my $waspName = $_[0];

# The names that aren't right only after usual lower case underscore 
# substitution.
   my %specDirNameByWaspName 
       = ( 'United Arab Emirates'     => 'uae',
           'Bosnia and Herzegovina'   => 'bosnia',
           'D.R. Congo'               => 'dr_congo',
           'Georgia'                  => 'georgia_country',
           'Guinea-Bissau'            => 'guinea_bissau',
           'Macedonia, TFYRO'         => 'macedonia',
           'Serbia and Montenegro'    => 'serbia_montenegro',
           'Taiwan Province of China' => 'taiwan',
           'Timor-Leste'              => 'timor_leste',
           'United Republic of Tanzania' => 'tanzania',
           'Viet Nam'                 => 'vietnam' );
   


   # Fix special names
   if (defined($specDirNameByWaspName{$waspName})){
      $waspName = $specDirNameByWaspName{$waspName};
   }

   # Do the usual lower case underscore substitution.
   my $dirCountry = lc($waspName);
   $dirCountry =~ s/[ ]/_/g;
   return $dirCountry;
}


# Map version is the name of the map release in map generation, 
# e.g. for Tele Atlas map releases TA_2009_06 (the map release string
# that is written to before_merge/mapOrigin.txt file in the individual
# country map generation).
# In the WASP database version table the release might have another name
# e.g. Tele Atlas map releases TeleAtlas_2009_06
# since it uses the *long* version of the supplier name, as specified in 
# variable file
# So you need to translate the short mapgen-mapver to the long WASP-mapver
# in order to find the WASP database version id.
#
# This function is used in poiImport
sub mapVersionToEDVersionID {
   my $dbh = $_[0];
   my $mapVersion = $_[1];
   
   # Modify map version to fit ED version.
   if ( $mapVersion =~ /^TA/ ){
       # Tele Atlas
       $mapVersion =~ s/^TA/TeleAtlas/;
   }
   elsif ( $mapVersion =~ /^AND/ ){
       # AND, no change needed.
   }
   elsif ( $mapVersion =~ /^TM/ ){
       # TopMap
       $mapVersion =~ s/^TM/TopMap/;
   }
   elsif ( $mapVersion =~ /^Monolit/ ){
       # Monolit, no changed needed.
   }
   elsif ( $mapVersion =~ /^GISrael/ ){
       # GISrael, no changed needed.
   }
   elsif ( $mapVersion =~ /^CEInfoSystems/ ){
       # CEInfoSystems, no change needed.
   }
   elsif ( $mapVersion =~ /^DMapas/ ){
       # DMapas, no change needed.
   }
   else {
      # no need to die with new map version name
      #die "Map supplier not handled for map version: $mapVersion";
   }
   
   my $query = 
       "SELECT ID FROM EDVersion WHERE version = '$mapVersion'";
   my $sth = $dbh->prepare($query) or die "Query prepare failed: $query";
   $sth->execute or die "Query execute failed: $query";
   if ( $sth->rows == 0 ){
      errprint "Map version not found in database EDVersion: $mapVersion";
      exit 1;
   }
   elsif ( $sth->rows > 1 ){
      errprint "Multiple matches for map version in database: $mapVersion";
      exit 1;
   }
   (my $edVersionID) = $sth->fetchrow();
   return $edVersionID;

} # mapVersionToEDVersionID



sub getMapsFromCountryDirs {
   my $dbh = $_[0];
   my $countriesDirs = $_[1];
   my $poiSourceID = $_[2];
   my $mapDataPOIs = $_[3];
   my $okWithMissingCountries = $_[4];

   my @result = (); # Filled in with full paths to all map files.

   # The countries where this poiSource have POIs.
   my @wantedCountries;
   if (defined $poiSourceID ){
      @wantedCountries = getCountryDirNamesOfPoiSource($dbh, $poiSourceID);
   }

   my %allCountryDirs = (); # Country dir path by country dir name.
   # Collect all the country directories (from EW,WW).
   foreach my $countriesDir (split ",", $countriesDirs){
      dbgprint "Checkning: $countriesDir";
      opendir (COUNTRIES, "$countriesDir") 
          or die "Could not open dir $countriesDir";
      foreach my $dir (readdir(COUNTRIES)){
         #dbgprint($dir);
         if ( ( $dir =~ /^\.$/ ) || ( $dir =~ /^\.\.$/ ) ){
            # Uninteresting, and always duplicates
         }
         else {
            if ( defined( $allCountryDirs{$dir} )){
               errprint "Dir $dir already present: $allCountryDirs{$dir}," .
                   " duplicated in $countriesDir/$dir";
               exit 1;
            }
            $allCountryDirs{$dir} = "$countriesDir/$dir";
         }
      }
      closedir COUNTRIES or die "Could not close dir";
   }



   # Find the map files of all "good quality" countries in WASP.
   # check that all of them are represented in the EW,WW dirs
   # If param okWithMissingCountries given, ok for some to be missing
   # If we have map data POIs we use map files of all countries, regardless
   # of quality
   my $nbrCountries = 0;
   my $nbrCountriesSub = 0;
   my $nbrCountriesMissing = 0;
   my $detailedSubQuery = " WHERE detailLevel > 0";
   if ( (defined $mapDataPOIs) and $mapDataPOIs ) {
      $detailedSubQuery = " WHERE detailLevel >= 0";
   }
   my $query = 
       "SELECT ID, country FROM POICountries $detailedSubQuery;"; 
   my $sth = $dbh->prepare($query);
   $sth->execute;
   while ((my $countryID, my $dbCountry) = $sth->fetchrow()) {
      my $dirCountry = countryWaspNameToDirName($dbCountry);
      #dbgprint $dirCountry;
      $nbrCountries += 1;

      if ( (defined($poiSourceID) && grep /$dirCountry/, @wantedCountries ) ||
           (!defined($poiSourceID) ) )
      {
         $nbrCountriesSub += 1;
         # When poiSource is defined, only use the wanted countries.
         if ( defined ($allCountryDirs{$dirCountry}) ){
            my $mapDir = "$allCountryDirs{$dirCountry}/before_merge";         
            # Read the map files of the country dir.
            opendir (MAPDIR, "$mapDir") 
                or die "Could not open dir $mapDir";
            foreach my $mapFile (readdir(MAPDIR)){
               if ( ( "$mapFile" =~ /\.mcm$/ ) || 
                    ( "$mapFile" =~ /\.mcm.bz2$/ ) ) {
                  # This is a valid map file, store it with full path.
                  push(@result, "$mapDir/$mapFile");
               }
            }
            closedir MAPDIR or die "Could not close dir";
         }
         else {
            $nbrCountriesMissing += 1;
            if ( defined $okWithMissingCountries ) {
               # ok
            } else {
               errprint "Missing country directory: $dirCountry\n" .
                        "Try okWithMissingCountries option.";
               die;
            }
         }
      }
      else {
         #warnprint "Not using not wanted country: $dirCountry";
      }
   }
   if ( defined $okWithMissingCountries ) {
      dbgprint "From $nbrCountries/$nbrCountriesSub available " .
               "POICountries countries, " .
               "$nbrCountriesMissing are missing in EW,WW countriesDirs " .
               "(running okWithMissingCountries)";
   }
   
   return sort(@result); # return an arrray with full paths to all map files.
} # getMapsFromCountryDirs


sub handleMapVerOverrides {
   my $dbh = $_[0];
   my $iso3166AndMapVerStr = $_[1];
   my $resultHash = $_[2];
   foreach my $iso3166AndMapVerPair ( split(",", $iso3166AndMapVerStr) ){
      dbgprint $iso3166AndMapVerPair;
      (my $iso3166, my $mapVer) = split(":", $iso3166AndMapVerPair);
      #dbgprint "($iso3166):($mapVer)";
      # Validate country code.
      my $countryID = getCountryIDFromISO3166Code($dbh, $iso3166);
      if ( ! defined $countryID ) {
         die "incorrect iso3166 \"$iso3166\"";
      }
      # Vailiate map version.
      my $edVerID = mapVersionToEDVersionID($dbh, $mapVer);
      if ( ! defined $edVerID ) {
         die "incorrect map version \"$mapVer\"";
      }
      dbgprint "$iso3166:$mapVer:$countryID";
      $resultHash->{$countryID}=$mapVer;
   }
   foreach my $key (keys(%{$resultHash})){
      dbgprint "handleMapVerOverrides $key:$resultHash->{$key}";
   }
}
# handleMapVerOverrides




sub getMapVersionByCountryID {
   my $dbh = $_[0];
   my $countriesDirs = $_[1];
   my $resultHash = $_[2];
   my $okWithMissingCountries = $_[3];

   my %mapVerByCountry = ();
   
   # Get mapOrigin from mapOrigin.txt in the EW,WW countries directories
   foreach my $countriesDir (split ",", $countriesDirs){
      dbgprint "Checking: $countriesDir";
      opendir (COUNTRIES, "$countriesDir") 
          or die "Could not open dir $countriesDir";
      foreach my $dir (readdir(COUNTRIES)){
         #dbgprint($dir);
         my $mapOrigFile = $countriesDir . "/" . $dir .
             "/before_merge/mapOrigin.txt";
         #dbgprint($mapOrigFile);

         if ( -e $mapOrigFile ){
            open MAPORIGFILE, "<$mapOrigFile" 
                or die "Could not open $mapOrigFile.";
            if ( defined ($mapVerByCountry{$dir}) ){
               errprint "This country already exists in previous country".
                   " directory, country: $dir";
               #print "rm $dir && ";
               exit 1;
            }
            ($mapVerByCountry{$dir}) =<MAPORIGFILE>;
            chomp $mapVerByCountry{$dir};
            #dbgprint "$dir: $mapVerByCountry{$dir}";
            close MAPORIGFILE or die "Could not close dir";
         }
         else {
            dbgprint "$dir have no mapOrigin.txt";
         }
      }
   }

   # Get all countries from POICountries table,
   # check that all of them were represented in the EW,WW dirs
   # If param okWithMissingCountries given, ok for some to be missing
   my $nbrCountries = 0;
   my $nbrCountriesMissing = 0;
   my $query = 
       "SELECT ID, country FROM POICountries";
   my $sth = $dbh->prepare($query);
   $sth->execute;
   my %checkHash = ();
   while ((my $countryID, my $dbCountry) = $sth->fetchrow()) {
      my $dirCountry = countryWaspNameToDirName($dbCountry);
      #dbgprint $dirCountry;
      $nbrCountries += 1;
      if ( defined ($mapVerByCountry{$dirCountry}) ){
         $resultHash->{$countryID}=$mapVerByCountry{$dirCountry};
         $checkHash{$dirCountry}=$mapVerByCountry{$dirCountry};
      }
      else {
         $nbrCountriesMissing += 1;
         if ( defined $okWithMissingCountries ) {
            # ok
         } else {
            errprint "Missing map version for country: $dbCountry($dirCountry)\n" .
                     "Try okWithMissingCountries option.";
            die;
         }
      }
   }
   if ( defined $okWithMissingCountries ) {
      dbgprint "From $nbrCountries available POICountries countries, " .
               "$nbrCountriesMissing are missing in EW,WW countriesDirs " .
               "(running okWithMissingCountries)";
   }
   
   # Printing what country directories that have not been used.
   # Might be that country dir name does not match the name extracted
   # from POICountries table, need to fix!?
   my $someNotUsed = 0;
   foreach my $countryDir (keys(%mapVerByCountry)){
      if ( ! defined $checkHash{$countryDir} ){
         warnprint "Not using country dir: $countryDir";
         $someNotUsed = 1;
      }
   }
   if ( $someNotUsed ) {
      die "Fix country dir names so they match " .
          "the name extracted from POICountries table";
   }

} # getMapVersionByCountryID


sub getDetailLevelOfCountryID {
   my $dbh = $_[0];
   my $countryID = $_[1];


    my $query = 
        "SELECT detailLevel FROM POICountries " .
        "WHERE ID = $countryID";
   my $sth = $dbh->prepare( $query ) or 
       die "Could not prepare query: $query\n";
   $sth->execute() or die "Could not execute query: $query\n";
    
    my ($detailLevel) = $sth->fetchrow();
   if (!defined $detailLevel or $detailLevel eq '') {
      dbgprint "No detail level found\n";
      return undef;
   }
   else {
      return $detailLevel;
   }

} # getCountryDetailLevel


sub getSourceIsMapPoi {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $productID = getProductID($dbh, $sourceID);
   if ( defined($productID) ){
      my $query = "SELECT mapDataPOIs from POIProducts WHERE ID = $productID";
      my $sth = $dbh->prepare($query) || 
          die "Prepare error in getSourceIsMapPoi".
          ", failed query: $query";
      $sth->execute()|| die "Execute error in getSourceIsMapPoi".
          ", failed query: $query";
      if ($sth->rows() == 1){
         (my $isMapPoiValue) = $sth->fetchrow();
         return $isMapPoiValue;
      }
      else {
         die "getSourceIsMapPoi unexpected query result from query: $query"; 
      }
   }
   else {
      die "getSourceIsMapPoi could not find product for source $sourceID";
   }
   
} # getSourceIsMapPoi

sub getProductSupplierName {
    my $dbh = $_[0];
    my $productID = $_[1];

    my $query = 
        "SELECT supplierName FROM POIProducts " .
        "WHERE ID = $productID";
    my $sth = $dbh->prepare( $query ) or 
        die "Could not prepare query: $query\n";
    $sth->execute() or die "Could not execute query: $query\n";
    
    my ($supplierName) = $sth->fetchrow();
    if (!defined $supplierName or $supplierName eq '') {
        dbgprint "No supplierName found\n";
        return undef;
    }
    else {
        return $supplierName;
    }
}

sub getProductID {
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $sourceQuery = 
      "SELECT productID FROM POISources " .
      "WHERE ID = $sourceID";
   my $sth = $dbh->prepare( $sourceQuery ) or die "Could not prepare query: $sourceQuery\n";
   $sth->execute() or die "Could not execute query: $sourceQuery\n";
   #dbgprint "source id = $sourceID\n";

   my ($productID) = $sth->fetchrow();
   if (!defined $productID or $productID eq '' or $productID == 0 ) {
      dbgprint "No productID found\n";
      return undef;
   }
   else {
      return $productID;
   }
}

sub getPrevPoiSource {
   my $dbh = $_[0];
   my $sourceID = $_[1];

   getPrevOrNextPoiSource($dbh, $sourceID, 'prev');

} # getPrevPoiSource

sub getNextPoiSource {
   my $dbh = $_[0];
   my $sourceID = $_[1];

   getPrevOrNextPoiSource($dbh, $sourceID, 'next');
} # getNextPoiSource

sub getPrevOrNextPoiSource {
   # In parameters
   my $dbh = $_[0];
   my $sourceID = $_[1];
   my $prevOrNextKeyword = $_[2];

   if ( ($prevOrNextKeyword ne 'prev') and ($prevOrNextKeyword ne 'next') ) {
      dbgprint "No \"prev\" or \"next\" keyword as input.\n";
      return undef;
   }

   # Result variables;
   my $prevSourceID;
   my $prevSourceName;
   my @sortedSourceIDs;

   dbgprint "source id = $sourceID";

   my $productID = getProductID ($dbh, $sourceID);

   if (!defined $productID or $productID eq '' or $productID == 0 ) {
      dbgprint "No productID found\n";
      return undef;
   }

   my @sourceIDs = getSourcesOfProduct($dbh, $productID);

   if ($prevOrNextKeyword eq 'prev') {
      @sortedSourceIDs = sort {$a <=> $b} @sourceIDs;
   } else {
      @sortedSourceIDs = sort {$b <=> $a} @sourceIDs;
   }
   
   my $tmpPrevSrcID="";
   foreach my $tmpSrcID (@sortedSourceIDs){
      #dbgprint "Candidate: ($tmpSrcID)";
      if ( $tmpSrcID eq $sourceID ){
         $prevSourceID=$tmpPrevSrcID;
      }
      $tmpPrevSrcID=$tmpSrcID;
   }
   if ($prevSourceID) {
      $prevSourceName = getSourceNameFromID( $dbh, $prevSourceID );
      return ($prevSourceID, $prevSourceName);
   }
   else {
      return undef;
   }
} # getPrevOrNextPoiSource

sub getInUseInfoFromSource {

   # In parameters
   my $dbh = $_[0];
   my $sourceID = $_[1];

   timeprint "";
   timeprint "Will print out info about inUse, validTo and validFrom for source";

   # result variables
   my %resultHash;

   my $query = "SELECT country, inUse, validFromVersion, validToVersion " .
      "FROM POIMain " .
      "WHERE source = $sourceID GROUP BY country";

   my $sth = $dbh->prepare($query) or die "Could not prepare query: $query";
   $sth->execute() or die "Could not execute query: $query";

   my %all_inuse;
   my ($inuse_false, $inuse_true, $num_rows) = 0;
   my $all_inuse_value;

   timeprint "";
   dbgprint "country\tinUse\tvalidFromVersion\tvalidToVersion";
   while (my ($c, $i, $vf, $vt) = $sth->fetchrow()) {
      if ($i == 0) {
         $inuse_false++;
         $all_inuse{$i} = $inuse_false;
      } else {
         $inuse_true++;
         $all_inuse{$i} = $inuse_true;
      }
      if (!defined $vf) {
         $vf = 'NULL';
      }
      if (!defined $vt) {
         $vt = 'NULL';
      }
      $resultHash{$c}{'inUse'} = $i;
      $resultHash{$c}{'validFromVersion'} = $vf;
      $resultHash{$c}{'validToVersion'} = $vt;
      dbgprint "$c       \t$i     \t$vf                \t$vt              ";
      $num_rows++;
   }
   timeprint "";
   if ((exists $all_inuse{0}) && ($all_inuse{0} == $num_rows)) {
      $all_inuse_value = 0;
      dbgprint "All inUse are set to false";
   } elsif ((exists $all_inuse{1}) && ($all_inuse{1} == $num_rows)) {
      dbgprint "All inUse are set to true";
      $all_inuse_value = 1;
   } else {
      dbgprint "Countries have different inUse settings";
   }
   return (\%resultHash, $all_inuse_value);

} # getInUseInfoFromSource

sub getSourcesOfProduct {
   # In parameters
   my $dbh = $_[0];
   my $productID = $_[1];

   # Result variables
   my @sourceIDs;

   #dbgprint "product id = $productID\n";

   my $query =
       "SELECT ID " .
       " FROM POISources WHERE productID = $productID" . 
       " ORDER BY ID";
   #dbgprint $query;
   my $sth = $dbh->prepare( $query ) or die "Could not prepare query: $query";
   $sth->execute() or die "Could not execute query: $query";

   while (my $sourceID = $sth->fetchrow()){
      push @sourceIDs, $sourceID;
   }
   return @sourceIDs;

} # getSourcesOfProduct

sub getMostFrequentValidFromVersionOfSourceAndCountry {
   # in parameters
   my $dbh = $_[0];
   my $sourceID  = $_[1];
   my $countryID  = $_[2];

   # result variables
   my $mostFrequentValidFromVersion;

   my $query = "SELECT validFromVersion, count(*) as number FROM POIMain WHERE " .
      "source = $sourceID and country = $countryID GROUP BY validFromVersion " .
      "ORDER BY number DESC LIMIT 1";
   #dbgprint $query;
   my $sth = $dbh->prepare($query) or die "Could not prepare query: $query";
   $sth->execute() or die "Could not execute query: $query";

   ($mostFrequentValidFromVersion) = $sth->fetchrow();
   return $mostFrequentValidFromVersion;

} # getMostFrequentValidFromVersionOfSourceAndCountry

sub getInUseFromSourceAndCountry {
   # in parameters
   my $dbh = $_[0];
   my $sourceID  = $_[1];
   my $countryID  = $_[2];

   # result variables
   my @resultValue;

   my $query = "SELECT DISTINCT inUse FROM POIMain WHERE " .
      "source = $sourceID and country = $countryID";

   my $sth = $dbh->prepare($query) or die "Could not prepare query: $query";
   $sth->execute() or die "Could not execute query: $query";

   while ((my $tmpInUseValue) = $sth->fetchrow()) {
      push @resultValue, $tmpInUseValue;
   }
   return @resultValue;
} # getInUseFromSourceAndCountry

sub getStaticIDForPOIID {
   my $dbh = $_[0];
   my $poiID = $_[1];
   my $query = "SELECT staticID FROM POIStatic WHERE poiID = $poiID;";
   my $sth = $dbh->prepare($query);
   $sth->execute or die "Query execute failed: $query";
   my $staticID = $sth->fetchrow();
   return $staticID;
   
}


sub storePOIs {
   my $dbh         = $_[0];
   my $allPOIs     = $_[1];
   my $addSynonyms = $_[2]; # obsolete, no action
   my $addToWasp   = $_[3];
   my $cpifFile    = $_[4];
   my $categoryObject = $_[5];
   
   dbgprint "PWT::storePOIs Called PerlWASPTools::storePOIs for " .
            scalar( keys( %{$allPOIs} ) ) . " POIs in allPOIs hash.";
   if ( defined($cpifFile) && ($cpifFile ne "") ) { 
      dbgprint("PWT::storePOIs Will write CPIF to: $cpifFile");
   }
   else {
      dbgprint("PWT::storePOIs Will NOT write CPIF.");
   }
   if ( defined($addToWasp) ) { 
      dbgprint("PWT::storePOIs Will add POIs to WASP.");
   }
   else {
      dbgprint("PWT::storePOIs Will NOT add POIs to WASP.");
   }
   
   # Check that all POIs have a name
   foreach my $poiKey ( sort keys( %{$allPOIs} ) ){
      if (!$allPOIs->{$poiKey}->getNbrNamesNonSynonyms()) {
         # poi has no name (except perhaps synonym), don't use
         dbgprint "POI " . $allPOIs->{$poiKey}->getSourceReference();
         dbgprint "has no name except synonym. Will not use this POI.\n";
         delete($allPOIs->{$poiKey});
      }
   }

   # Extract categories from POI types 
   dbgprint "Extract categories from POI types";
   my $tmpNbr = 0;
   foreach my $poiKey ( sort keys( %{$allPOIs} ) ){
      $tmpNbr += 1;
      if ( $tmpNbr % 50000 == 0 ){ # Prints progress
         dbgprint " processed $tmpNbr pois";
      }

      foreach my $poiType ($allPOIs->{$poiKey}->getPOITypes()){
         my $catID = getPOICatFromPOIType($dbh, $poiType);
         if ( defined $catID ) {
            $allPOIs->{$poiKey}->addCategory($catID);
            #dbgprint "Type: $poiType, cat:", $catID;
         }
      }
      #$allPOIs{$poiKey}->dumpInfo();
      # Remove all categories being parents of other categories of this POI.
      my @newCategories = 
          $categoryObject->
          eliminateHigherLevelDoubles($allPOIs->{$poiKey}->getCategoriesRef());
      #$allPOIs->{$poiKey}->dumpInfo();

      # If this POI has a category outside the tree, remove all other
      # categories.
      #
      # This is because in this case, the category outside the
      # tree is probably a category that should be below the POI type connected
      # category in the tree, but since it's not, the parent category could not
      # be removed by eliminateHigherLevelDoubles.
      my @catInTree;
      my $rmCatsOutsideTree = 0;
      my $myCats = $allPOIs->{$poiKey}->getCategoriesRef;
      if ( defined $myCats ) {
         foreach my $catID ( @{$myCats} ){
            if (! $categoryObject->catInTree($catID) ){
               $rmCatsOutsideTree = 1;
            }
            else {
               push @catInTree, $catID;
            }
         }
      }
      if ( $rmCatsOutsideTree ){
         # We found a category outside the tree, so we remove the others.
         foreach my $catID (@catInTree){
            $allPOIs->{$poiKey}->removeCategory($catID);
         }
      }
   }
   dbgprint "Extracted categories from POI types";


   # Writing CPIF
   my $nbrPois = 0;
   my $writtenPois = 0;
   if ( defined($cpifFile) && ($cpifFile ne "") ) { 
      dbgprint "PWT::storePOIs Writing POIs to CPIF";
      # Open outfile CPIF, and write it empty (used for backup)
      open(CPIF, ">$cpifFile");
      print CPIF "";
      foreach my $poiKey ( sort (keys( %{$allPOIs} ) ) ){
         $nbrPois++;
         if ( $allPOIs->{$poiKey}->writeCPIF($cpifFile) ){
            $writtenPois++;
         }
      }
      dbgprint "PWT::storePOIs Wrote $writtenPois POIs of $nbrPois to CPIF.";
   }

   # Add to WASP
   $nbrPois = 0;
   my $insertedPois = 0;
   if ( ! $addToWasp ) {
      dbgprint "PWT::storePOIs Return without adding to WASP.";
      return 0;
   }
   foreach my $poiKey ( sort keys( %{$allPOIs} ) ){
      $nbrPois++;
      my $newID = $allPOIs->{$poiKey}->addToWASP();
      if ( $newID > 0 ) {
         $insertedPois += 1;
         dbgprint "WASP_INSERT[$insertedPois]: srcRef=".
             $allPOIs->{$poiKey}->getSourceReference().
             " waspID=". $newID;
      }
      elsif( $newID == -2 ) {
         # Special return code for when the POI was not added because it
         # Already exists.
         dbgprint "WASP_PRESENT[$insertedPois]: srcRef=".
             $allPOIs->{$poiKey}->getSourceReference();
      }
      else {
         my $srcRef = $allPOIs->{$poiKey}->getSourceReference();
         if (!defined ($srcRef) ){
            $srcRef = "UNKNOWN SOURCE REF";
         }
         my $failedID = $insertedPois+1;
         die ("PWT::storePOIs Failed to add poi[". $failedID . "]".
            " srcRef: ". $srcRef. " Totally added $insertedPois");
      }

   }
   dbgprint "PWT::storePOIs Added $insertedPois POIs of $nbrPois to WASP.";
   dbgprint "PWT::storePOIs Wrote $writtenPois POIs of $nbrPois to CPIF.";
   return $insertedPois;
}

# Returns a list with all country names
sub getAllCountryDirNames{
   my $dbh = $_[0];

   # result array 
   my @countries = ();
   my $query = "SELECT country FROM POICountries";
   my $sth = $dbh->prepare($query) || die "Prepare failed, query: $query";
   $sth->execute() || die "Execute failed, query: $query";
   
   while ((my $waspCountry) = $sth->fetchrow()) {
      my $dirCountry = countryWaspNameToDirName($waspCountry);
      push @countries, $dirCountry;
   }
   return @countries;

} # getAllCountryDirNames


sub getCountriesOfSource {
   my $dbh = $_[0];
   my $sourceID = $_[1];

   # result array 
   my @countries = ();
   my $query = "SELECT distinct country FROM POIMain WHERE " .
      "source = $sourceID order by country";

   my $sth = $dbh->prepare($query) || die "Prepare failed, query: $query";
   $sth->execute() || die "Execute failed, query: $query";

   while ((my $countryID) = $sth->fetchrow()) {
      push @countries, $countryID;
   }
   return @countries;

} # getCountriesOfSource

sub getSourceInfo {
   my $dbh = $_[0];
   my $sourceID = $_[1];

   my $query =
       "SELECT".
       " POICountries.country, deleted, inUse, (POIMain.lat is NULL),".
       " (staticID is NULL), (POIEntryPoints.lat IS NULL), count(distinct POIMain.ID)".
       " FROM (POIMain left join POIStatic on POIMain.ID = POIStatic.poiID)".
       " join POICountries on POIMain.country = POICountries.ID".
       " left join POIEntryPoints on POIMain.ID = POIEntryPoints.poiID".
       " WHERE source = $sourceID".
       " GROUP BY POICountries.country, deleted, inUse,".
       " (POIMain.lat is NULL), (staticID is NULL),".
       " (POIEntryPoints.lat IS NULL);";
   my $sth = $dbh->prepare( $query ) || die "Prepare failed, query: $query";
   $sth->execute() ||die "Execute failed, query: $query";
   
   my $nbrTotal = 0;
   my $nbrDeleted = 0;
   my $nbrStaticOK = 0;
   my $nbrPosOK = 0;
   my $nbrInUse = 0;
   my $nbrEtrPtsOK = 0;

   my %uniqueCountryIDs = ();
   while ( (my $countryID, 
            my $deleted, 
            my $inUse, 
            my $latIsNull, 
            my $staticIsNull, 
            my $etrPtsIsNull,
            my $count ) = $sth->fetchrow() ){
      $nbrTotal+=$count;
      $uniqueCountryIDs{$countryID}=1;
      if ($deleted){
         $nbrDeleted += $count;
      }
      if ($inUse){
         $nbrInUse += $count;
      }
      if (!$latIsNull){
         $nbrPosOK += $count;
      }
      if (!$staticIsNull){
         $nbrStaticOK += $count;
      }
      if (!$etrPtsIsNull){
         $nbrEtrPtsOK += $count;
      }
   }
   my $nbrCountries = scalar(keys(%uniqueCountryIDs));
   
   return ( $nbrTotal, 
            $nbrCountries, 
            $nbrDeleted, 
            $nbrInUse, 
            $nbrStaticOK, 
            $nbrPosOK,
            $nbrEtrPtsOK );
} # getSourceInfo


sub db_connect {
   my $noPrint = 0;
   if ( $_[0] ) {
      $noPrint = 1;
   }

   # Define the settings for connecting to the WASP database here
   my $dbname = "poi";     # sql database name
   my $dbhost = "poihost"; # sql host
   my $dbuser = "poi";     # sql user
   my $dbpw = "UghTre6S";  # sql database password

   my $ldbh = DBI->connect("DBI:mysql:$dbname:$dbhost", $dbuser, $dbpw);
   if ( ! $noPrint ) {
      dbgprint "Connecting to db: $dbname on $dbhost";
   }
   return $ldbh;

}

sub db_disconnect {
   my ($ldbh) = @_;

   $ldbh->disconnect;

}


1;

=head1 NAME 

PerlWASPTools

Package with common functions used in communication with 
the WASP (POI or ED) database.

=head1 USE

Include into your perl file with the combination of:
   use lib "$BASEGENFILESPATH/script/perllib";
   use PerlWASPTools;
pointing to the directory where the perl modules are stored

=head1 FUNCTIONS

getCountryIDFromName( $dbh, $countryName )
   Asks the database connection $dbh to find countryID for
   the $countryName given. If the $countryName is unknown
   the countryID returned is undefined.

getCountryIDFromISO3166Code( $dbh, $countryCode )
   Asks the database connection $dbh to find countryID for
   the $countryCode given. If the $countryCode is unknown
   the countryID returned is undefined. $countryCode
   should be the two-letter code from ISO3166-1 alpha-2.

getCountryNameFromISO3166Code( $dbh, $countryCode )
   Asks the database connection $dbh to find country
   name for the $countryCode given. If the $countryCode
   is unknown the country returned is undefined.
   $countryCode should be the two-letter code from
   ISO3166-1 alpha-2.

getCountryDirNameFromIso3166Code
   Returns the country directory name, used when storing GDF and mcm maps in 
   one directory per country.

getLanguageIDFromName( $dbh, $langName )
   Asks the database connection $dbh to find langID for
   the $langName given. If the $langName is unknown the
   langID returned is undefined.

getSourceIDFromName( $dbh, $sourceName )
   Asks the database connection $dbh to find sourceID for
   the $sourceName given. If the $sourceName is unknown
   the sourceID returned is undefined.

getSourceNameFromID( $dbh, $sourceID )
   Asks the database connection $dbh to find sourceName
   for the $sourceID given. If the $sourceID is unknown
   in WASP, the sourceName returned is undefined.

getVersionIDFromName( $dbh, $versionName )
   Version ID is the one stored in EDVersion in the database.
   Asks the database connection $dbh to find versionID for
   the $versionName given. If the $versionName is unknown
   the $versionID returned is undefined.

getVersionNameFromID( $dbh, $versionID )
   Version ID is the one stored in EDVersion in the database.
   Asks the database connection $dbh to find versionName
   for the $versionID given. If the $versionID is unknown
   in WASP, the versionName returned is undefined.

getPOITypeIDFromTypeName( $dbh, $typeName )
   Asks the database connection $dbh to find poiTypeID for
   the $typeName given.

getPrevPoiSource ( $dbh, $sourceID )
   see getPrevOrNextPoiSource

getNextPoiSource ( $dbh, $sourceID )
   see getPrevOrNextPoiSource

getPrevOrNextPoiSource ( $dbh, $sourceID, $prevOrNext )
   Using database connection $dbh, in case $prevOrNext keyword 
   is "prev", returns the highest source ID lower than $sourceID, 
   and having the same product ID as $sourceID. In case $prevOrNext
   keyword is "next", returns the lowest source ID higher than
   $sourceID, and having the same product ID as $sourceID.
   Returns an array with product ID in element 0 and product 
   name in element 1, or undef on
   failure.

getMostFrequentValidFromVersionOfSourceAndCountry ( $dbh, $sourceID, $countryID )
   Will return the most frequent validFromVersion id for the 
   combination of source $sourceID and country $countryID.

getInUseFromSourceAndCountry ( $dbh, $sourceID, $countryID )
   Will return the value of inUse for the combination
   of source $sourceID and country $countryID. The result
   will come in an array, for the case if in use is both
   1 and 0 for a source and country (which it should not be)

getInUseInfoFromSource ( $dbh, $sourceID )
   Will print out info about inUse, validFromVersion and
   validToVersion for each country in source $sourceID. Returns hash
   with this info.

getSourcesOfProduct ( $dbh, $productID )
   Using database connection $dbh, returns the sourceIDs of a product
   with $productID. Returns an array with sourceIDs, or undef on failure.

getMaxAttributeTypeID( $dbh )
   Get the max ID from the POIAttributeTypes table.
   
getMaxAttributeTypeIDLessThanInfo( $dbh )
   Get the max ID from the POIAttributeTypes table where
   id is less than poi info.

db_connect ( $noPrint )
   Connects to poi db. Returns database handle.
   Give $noPrint = 01 if dbg print is no wanted.

db_disconnect ( $dbh )
   Disconnects from db.

getMapVersionByCountryID( $dbh, $countriesDirs, $resultHash )
   Fills in the hash reference in $resultHash with the map versions found
   in mapOrigin.txt of countries in $countriesDirs. Using $dbh for finding
   database IDs of countries. Use mapVersionToEDVersionID to get EDVersion
   from the map version in $resultHash.

handleMapVerOverrides ( $dbh, $iso3166AndMapVerStr, $resultHash )
   Changes map version values of the counties that match the countries
   given in $iso3166AndMapVerStr of the $resultHash. Give $iso3166AndMapVerStr
   on the form iso3166Code:mapVer,iso3166Code:mapVer, ... for example:
   sg:TA_2007_02,hk:TA_2006_10,se:TA_2006_10. $resultHash is the both in and out
   parameter on the same format as returned by getMapVersionByCountryID.

mapVersionToEDVersionID ($dbh, $mapVersion)
   Translate the map version string in $mapVersion to WASP EDVersion.ID using
   $dbh.

getStaticIDForPOIID ($dbh, $poiID)
   Get the static ID from POIStatic table for a POI with id $poiID

getNbrStaticIDsOfSourceID($dbh, $sourceID)
   Get number of static IDs of POIs of $sourceID. Counts multiple static IDs
   of the same POI many times.

getNbrPOIsOfSourceID($dbh, $sourceID)
   Get number of POIs of the given $sourceID

storePOIs($allPOIs, $addSynonyms, $addToWasp, $cpifFile)
   Stores all POIObjects given in the hash $allPOIs sorted on source reference.
   The $addSynonyms is obsolete - no action
   Storing options are $addToWasp, which makes the POIs get inserted to WASP
   and $cpifFile, which if specified writes the POIs to the CPIF file 
   given in the option.

getSourceIsMapPoi($dbh, $sourceID)
   Returns true if POI sets of $sourceID are considered map POIs, that is, POIs
   that may have entry points delivered from the map provider.

getSourceInfo($dbh, $sourceID)
   Returns an array with different statistical counts of the POI set from 
   source $sourceID. Returned values:
   ($nbrTotal, $nbrCountries, $nbrDeleted, $nbrInUse, $nbrStaticOK, $nbrPosOK)

getCountriesOfSource($dbh, $sourceID)
   Returns an array with countryIDs of the source $sourceID.

getAllCountryDirNames($dbh)
   Returns an array with the dir name of all countries.

getProductID($dbh, $sourceID)
   Returns product ID of $sourceID.

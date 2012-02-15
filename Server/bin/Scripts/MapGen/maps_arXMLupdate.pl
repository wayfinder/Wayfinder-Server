#!/usr/bin/perl -w
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

use Getopt::Long ':config',
   'no_auto_abbrev', # Impossible to use -s for --source.
   'no_ignore_case'  # Case sensitive options.
;
use vars qw( $opt_help $opt_m $opt_mapCountriesDirs 
             $opt_okWithMissingCountries
             %m_countryOvr %m_countryInfo $m_dbh @alreadyIncluded );
             
# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use POIObject;
use PerlWASPTools;
use PerlTools;
use DBI;
use Data::Dumper;
use POSIX qw(mktime);
use Time::Local 'timegm_nocheck';

GetOptions('help|h',
           'mapCountriesDirs=s',
           'okWithMissingCountries',
           'm=s'
           ) or die "Use -help for help.";

my $m_countryInfoFileName = "countryInfo.txt";
my $m_countryOvrFileName = "countryOverviews.txt";
my @countriesDirs;

if (defined $opt_help) {
   print <<EOM;
Script that creates ar-xml files for countries with translations of country names.

Fixme: The routine for getting all translations into a stringhash must
       implemented, right now the stringhash is empty, except a few
       hardcoded examples.
       (before the translations were fetched from a translation management 
        system called Dodona, hence the param names in this script)

Note: with the adminworld.pl script you can create the genfiles country 
structure, and setting files: co.xml and variable file for a certain map release

Usage: $0 
       options

Options:
-h          Display this help.

-mapCountriesDirs COUNTRIES_DIR1,COUNTRIES_DIR2
Countries directories of map generations in a comma spearated list with no space. Typically \"EW,WW\" pointing at a EW countries and a WW countries directory. These should have mapOrigin.txt files for each country. Default: the countries dirs must contain all countries that are defined in the WASP POICountries table. If not, combine with the -okWithMissingCountries option.
The countries in these directories can be links to individual country 
generation or just dummy dirs only containing the before_merge/mapOrigin.txt
which has the essential info. Example
_tmpEW/denmark/before_merge/mapOrigin.txt
_tmpWW/chile/before_merge/mapOrigin.txt
The mapOrigin.txt files contain one row with the map release string OSM_201005

-okWithMissingCountries
Give this option if the -mapCountriesDirs only contain some countries.

-m string   Functions working with the countries we use for production. 
            You need the country info file in the current directory. It
            should be called countryInfo.txt
               createArXML
                  Create ar.xml files using name translations from XXX.
                  You need to specify the map version of different 
                  countries by pointing at one/more directories 
                  (the -mapCountriesDirs option) containing 
                  all countries with a mapOrigin.txt file in the 
                  before_merge directory.
               createArXML_us
                  Like createArXML but state names are expected in the 
                  countryInfo.txt file.


EOM
  exit 1;
}

# Countries in countryInfo.txt not wanted
# (already included inside other countries)
push @alreadyIncluded, "GI";  # gibraltar in spain
push @alreadyIncluded, "VA";  # vatican in italy
push @alreadyIncluded, "AX";  # åland islands in finland
push @alreadyIncluded, "PR";  # puerto rico in USA
push @alreadyIncluded, "X1";  # Channel islands in uk
push @alreadyIncluded, "X2";  # Isle of Man in uk


if ( defined($opt_m) ) {

   if ( ($opt_m eq "createArXML") or
        ($opt_m eq "createArXML_us") ) {
      # ok
   } else {
      die "Uknown function $opt_m - exit!";
   }

    multiCountry($opt_m);
    exit;
}

sub multiCountry {
   my $function = $_[0]; 
   dbgprint "multiCountry:$function";

   my $poi_dbh;
   my $query;

   # Connect to POI database.
   $poi_dbh = db_connect();
   if (!defined $poi_dbh) {
      die "Cannot connect to the database ($DBI::errstr)\n";
   }

   # Local settings
   my $rootDir = "."; #No last slash
   
   # Options
   my $mapCountriesDirs = $opt_mapCountriesDirs;
      my %mapVerByCountryID=(); # Stores the map version of each country.
      if ( $opt_mapCountriesDirs ){

         @countriesDirs = split(",", $opt_mapCountriesDirs);
         my $i=0;
         for my $countryDir (@countriesDirs){
            $i++;
            print("Countries dir $i: $countryDir\n");
            if ( ! testDir($countryDir ) ){
               errprint("Countries directory: $countryDir not OK");
               exit 1;
            }
         }

         # Get map version for each country.
         getMapVersionByCountryID($poi_dbh, $opt_mapCountriesDirs, 
                                  \%mapVerByCountryID,
                                  $opt_okWithMissingCountries);
         foreach my $key (keys(%mapVerByCountryID)){
            print "OK: $mapVerByCountryID{$key} ";
         }
         print "\n" . "Countries directories OK.\n";
      }

   # Loads countryInfo.txt in current directory.
   loadCountryInfo();

   # Get the translations
   # Fixme: get a routine for getting all the translations!
   # The can be picked from MC2 StringTableUTF8::strings, using 
   # the StringTable::getCountryStringCode to get the stringCode for 
   # the specific country.
   #
   # Store all translations in $stringhash
   # $stringhash{$stringkey}{$lang} = $trstring;
   # stringkey= dodonaStringKey
   #            in most cases the upper-case version of gmsName 
   #            from m_countryInfo
   # lang= dodona language id as in POINameLanguages table
   # trstring= the translated string on language $lang

   # map languageName to language code, according to POINameLanguages.
   my %languageCodes =(); 

   $query = "select langName, dodonaLangID from POINameLanguages;";

   my $poi_sth = $poi_dbh->prepare($query);
   $poi_sth->execute();

   my $langName;
   my $langCode;

   while ( ($langName, $langCode) = $poi_sth->fetchrow() ) {
       $languageCodes{$langName} = $langCode;
   }

   # map country codes to cleartext, take from countryinfo.txt
   #

   my %countryNames = ();

   foreach my $id ( keys(%m_countryInfo) ) {
       $countryNames{$m_countryInfo{$id}{"alpha2"}} = $m_countryInfo{$id}{"gmsName"};
   }

   #

   my %stringhash = ();

   open (COUNTRYNAMES, "<", "countrynames.csv") || die "Cant open country names file!";

   while (<COUNTRYNAMES>) {
       chomp;

       (my $languageName, 
	my $countryCode, 
	my $caption) = split (":");
        
       $stringhash{uc($countryNames{$countryCode})}{$languageCodes{lc($languageName)}} = $caption;
   }

   # get all the translations here!
   # Example with hardcoded values
   print "size %stringhash = " . scalar keys(%stringhash) . "\n";

   my %newCountries = ();

   # Loop country info
   #print "Loop country info\n";
   foreach my $id ( keys(%m_countryInfo) ) {

      # Check if this country should be used

      my $alpha2 = $m_countryInfo{$id}{"alpha2"};
      my $wanted = 1;
      foreach my $a ( @alreadyIncluded ) {
         if ( $a eq $alpha2 ) {
            $wanted = 0;
         }
      }
      if ( $wanted ) {
         
          # store in hash on string key
          my $stringKey = ( $m_countryInfo{$id}{"gmsName"} );

          $newCountries{ "$stringKey" }{"countryName"} = 
              $m_countryInfo{$id}{"countryName"};
          
          $newCountries{ "$stringKey" }{"alpha2"} = lc($alpha2);
          $newCountries{ "$stringKey" }{"continent"} = 
              $m_countryInfo{$id}{"continent"};
          
      }
   }


   # Handle each country
   print "Handle each country\n";
   my $count = 0;
   foreach my $key ( sort(keys(%newCountries)) ) {
       my $gmsName = $key;

       my $mapVersion2;
       my $alpha2 = $newCountries{$key}{"alpha2"};
       my $continent = $newCountries{$key}{"continent"};

       # Handle the dirName
       my $dirName = $gmsName;
       if ($dirName eq "united_arab_emirates"){
           $dirName = "uae";
       }
 
       # Get the mapVersion from the mapOrigin.txt from 
       # mapCountriesDirs/country/before_merge/mapOrigin.txt
       dbgprint("$mapCountriesDirs");
       my $wdir = $countriesDirs[0];
       my $edir = $countriesDirs[1];
       #opendir WW, $wdir || die "Cant open ww dir\n";
       #opendir EW, $edir || die "Cant open ew dir\n";
       my $mapVersionFileName1=
                "$wdir/$dirName/before_merge/mapOrigin.txt";
       my $mapVersionFileName2=
                "$edir/$dirName/before_merge/mapOrigin.txt";
       if ( $function eq "createArXML_us" ){
           $mapVersionFileName1 = 
               "$wdir/usa/before_merge/mapOrigin.txt";
           $mapVersionFileName2 = 
               "$edir/usa/before_merge/mapOrigin.txt";
       }

       my $skipCountry = 0;
       my $mapVersionFileName = "";
       if ( -e $mapVersionFileName1 ) {
         $mapVersionFileName = $mapVersionFileName1;
       } elsif ( -e $mapVersionFileName2 ) {
         $mapVersionFileName = $mapVersionFileName2;
       } else {
         errprint "No mapOrigin.txt file for $dirName";
         if ( $opt_okWithMissingCountries ) {
            # ok, skip this country
            print " okWithMissingCountries\n";
            $skipCountry = 1;
         } else {
            die "Cant open map version file: $mapVersionFileName $!";
         }
       }
       if ( $skipCountry ) {
         next;
       }

         open (MAPVERSIONFILE, "<", "$mapVersionFileName") || die "Cant open map version file: $mapVersionFileName $!";
           my $mapVersion = readline(*MAPVERSIONFILE) || die "Failed to read $!";
           chomp($mapVersion);
       
       # Create ar-file for this country/state and write them
       if ( $function eq "createArXML" || 
            $function  eq "createArXML_us" ){

          my $filePath = "";
          my $dirPath = "";
          if ( $function eq "createArXML" ){
             # Make sure the directory is there.
             $dirPath = "${rootDir}/${dirName}";          
             if (mkdir($dirPath)){
                dbgprint("Created directory: $dirPath");
             }
             $dirPath = "${rootDir}/${dirName}/${mapVersion}";
             if (mkdir($dirPath)){
                dbgprint("Created directory: $dirPath");
             }
             $dirPath = "${rootDir}/${dirName}/${mapVersion}/xml";
             if (mkdir($dirPath)){
                dbgprint("Created directory: $dirPath");
             }
             $filePath = "${dirPath}" . "/" . "${dirName}" . "_ar.xml";
          }
          elsif ( $function  eq "createArXML_us" ){
             $filePath = "${rootDir}/usa_ar.xml";  
          }

          dbgprint("");
          dbgprint("File to print:     $filePath");
          dbgprint("Map version:       $mapVersion");
          
          if (-e $filePath and $count == 0) {
            die "$filePath already exists!\n";
          }
          open ARFILE, ">>$filePath" || die "Could not open $filePath. $!";
          
          # Get country names from Dodona. dodonaName = the dodonaStringKey
          my $dodonaName = uc($gmsName);
          
           # Handle special map names.
           my $regionIdent = $gmsName;
           if ($regionIdent eq "uk"){
               $regionIdent = "england";
           }
           if ($regionIdent eq "united_arab_emirates"){
               $regionIdent = "uae";
           }


           # Print ar.xml file content.
          if ( ($function eq "createArXML") or
               ($count == 0) ){
             print(ARFILE '<?xml version="1.0"  encoding="UTF-8"?>' .
                   "\n") || die "Could not print to ARFILE. $!";
             print(ARFILE '<!DOCTYPE map_generation-mc2 SYSTEM "map_generation-mc2.dtd">' ."\n");
             print(ARFILE '' ."\n");
             print(ARFILE '<map_generation-mc2>' ."\n");
          }

            print(ARFILE '   <add_region type="country" region_ident="' . 
                 $regionIdent . '">' . "\n");
            # loop over the languages for this stringKey.
            my $nbrLangs = 0;
            my $allLangsPrint = "Languages: ";
            my %stringKeysHash;
            foreach my $dodonaLangCode (sort (keys %{$stringhash{$dodonaName}})) {
              $allLangsPrint .= "$dodonaLangCode,";
              $stringKeysHash{"$dodonaName"}=1;
              
              # Get the language name.
              $query = "select langName, dodonaLangID " .
                  "from POINameLanguages where ".
                  "dodonaLangID = '" . $dodonaLangCode . "';";
              #print "query: $query\n";
              my $poi_sth = $poi_dbh->prepare( $query );
              $poi_sth->execute();
              my $langName;
              if ( ! ( ($langName) = $poi_sth->fetchrow() ) ){
                 die("Could not find lang for dodona code " .
                     $dodonaLangCode);
              } else {
                 $langName =~ s/[ ]/_/g;
                 $langName =~ s/\(//;
                 $langName =~ s/\)//;
                 
                 # don't write english_us because we can't use it
                 if ($langName eq "english_us") {
                     next;
                 }
                 print(ARFILE '      <name language="' . $langName . 
                       '" type="officialName">' . $stringhash{$dodonaName}{$dodonaLangCode} . 
                       '</name>' . "\n");
                 $nbrLangs++;
              }
           }
           # Print some info to the log, and check validity.
           dbgprint($allLangsPrint . " Nbr: $nbrLangs");
           if ( $nbrLangs == 0 ){
               die("No translated names for $dodonaName in the stringhash hash.");
           }
           my $nbrStringKeys = 0;
           my $allStringKeys = "";
           foreach my $usedStringKey (keys(%stringKeysHash)){
               $nbrStringKeys++;
               $allStringKeys .= "$usedStringKey, ";
           }
           dbgprint("String key: " . $allStringKeys);
           if ( $nbrStringKeys != 1 ){
              die("$nbrStringKeys string keys, should have been 1");
           }
           dbgprint("");
           # end of translation print for this country
           print(ARFILE '' ."\n");
           
           my @ovrNames = getOvrMapIdent($gmsName);
           foreach my $ovrName ( @ovrNames ) {
               print(ARFILE '      <content map_ident="' . $ovrName . '">'.
                     "\n");
               print(ARFILE '         <whole_map/>' ."\n");
               print(ARFILE '      </content>' ."\n");
           }
          
          print(ARFILE '   </add_region> ' ."\n");

          if ( ($function eq "createArXML") or
               ($count == keys(%newCountries)-1) ){
             print(ARFILE '</map_generation-mc2>' ."\n");
             print(ARFILE "\n");
             print(ARFILE "\n");
          }

       }
       $count++;
   } # foreach country code.
   

   dbgprint("");
   dbgprint("Handled $count countries");
   close ARFILE;
   
} # multiCountry

# The %m_countryInfo is a hash of hashes
#  key = andID, value = 
#        hash ( key = [countryName, gmsName, alpha2, ..], value )
sub loadCountryInfo {
   dbgprint "loadCountryInfo from $m_countryInfoFileName";
   if (open INFO_FILE, $m_countryInfoFileName) {
      %m_countryInfo = ();
      while( <INFO_FILE> ) {
         chomp;
         (my $id, 
          my $countryName, 
          my $gmsName, 
          my $alpha2, 
          my $continent) = split (";");
         $m_countryInfo{$id}{"countryName"} = $countryName;
         $m_countryInfo{$id}{"gmsName"} = $gmsName;
         $m_countryInfo{$id}{"alpha2"} = $alpha2;
         $m_countryInfo{$id}{"continent"} = $continent;
         #print "$gmsName\n"
      }
      print " loaded info for " . 
            scalar(keys(%m_countryInfo)) . " countries\n";
   } else {
      die "ERROR loadCountryInfo: no info file to read\n";
   }

   dbgprint "loadCountryOverviews from $m_countryOvrFileName";
   if (open INFO_FILE, $m_countryOvrFileName) {
      %m_countryOvr = ();
      while( <INFO_FILE> ) {
         chomp;
         (my $countryName, 
          my $ovrs) = split (":");
         $m_countryOvr{$countryName} = $ovrs;
         print $countryName, $ovrs;
      }
      print " loaded overview list for " . 
            scalar(keys(%m_countryOvr)) . " countries\n";
   } else {
      die "ERROR loadCountryInfo: no ovr file to read\n";
   }
}

# Returns the names of ovierview maps of a country.
# Standard name of overview map is the same as the name of the country
sub getOvrMapIdent {
   my $gmsName = $_[0];
   my @result;

   if ( $gmsName eq "united_arab_emirates" ){
       @result = ("uae");
   }

#   # Example if one country has more than one overview map
#   elsif ( $gmsName eq "germany" ){
#       @result = ("germany_1", "germany_2", "germany_3", "germany_4");
#   }

   elsif ( $m_countryOvr{$gmsName} ) {
       @result = split(" ", $m_countryOvr{$gmsName});
   }
   else {
       # standard
       @result = $gmsName;
   }
   return @result;
}


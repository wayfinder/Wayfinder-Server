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

use Getopt::Std;

use vars qw( $opt_h $opt_v 
             $opt_m $opt_a $opt_d
             %m_countryInfo @alreadyIncluded
             );




# Use perl modules from here or BASEGENFILESPATH/script/perllib
# Todo: get BASEGENFILESPATH into the use-lib-string possible?
# use lib "fullpath/genfiles/script/perllib";
#use lib ".";

# Set lib assuming the script is located in genfiles/script
use FindBin '$Bin';
use lib "$Bin/perllib";
use PerlTools;


getopts('hv:m:a:d:');

my $m_countryInfoFileName = "countryInfo.txt";

if (defined $opt_h) {
   print <<EOM;
Script to create genfiles country setting files for all countries in the world

Usage: $0 
       options
       mid-file [ mif-file other-file(s) ]

Options:
-h          Display this help.

-m string   Functions working with the countries we use for production. 
            You need the countryInfo.txt in the current directory.
            If you want to do stuff for USA, you must add USA to 
            the countryInfo.txt file as well. USA is per default not included
            in the countryInfo.txt, since USA is split in states in the
            map generation.

               createCountryDirs
                  Create the genfiles country directory structure.
                  It is created in the directory where this script runs, 
                  the current directory.
                  Example
                     denmark/countrypol
                     denmark/script
                     denmark/mapRelease
                     denmark/mapRelease/areafiles
                     denmark/mapRelease/xml
               createCoXML
                  Create co.xml files.
                  The file is created in the denmark/mapRelease/xml
                  sub dir of this current directory
                  The map_ident tag in the co.xml file (underview map name)
                  will be named "alpha2_whole" so if you have other mcm map 
                  names you need to fix this.
               createVarFile
                  Create variable files with standard params in it.
                  The file is created in the denmark/script
                  sub dir of this current directory.
                  The AREALIST will contain one areafile and that will
                  be named "alpha2_whole_municipalItems". If you have
                  other mcm map names you need to fix this.

-v string   The map release, example OSM_201005 or TA_2010_06

-d string   The (wf) MAPPATH when creating variable files
            All countries get the same MAPPATH, so this needs to be modified
            in order to get individual map paths, with one path per country,
            e.g. by appending the ISO alpha2 country code in some way...

-a string   The long version of the map supplier to be used in variable 
            files, e.g. "OpenStreetMap" or "TeleAtlas"
            Will be written to variable file MAP_SUPPLIER with 
            a '_* appended to it i.e.
            MAP_SUPPLIER="OpenStreetMap_"


Creation of the ar.xml file is done with the maps_arXMLupdate.pl script


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

    multiCountry($opt_m);
    exit;
}



exit;





# The %m_countryInfo is a hash of hashes
#  key = andID, value = 
#        hash ( key = [countryName, gmsName, alpha2, ..], value )
sub loadCountryInfo {
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
      print "WARN loadCountryInfo: no info file to read\n";
      die "exit";
   }
}






#####################################################################
# Functions for creating input for generation binaries/files



# Returns the names of ovierview maps of a country.
sub getOvrMapIdent {
   my $gmsName = $_[0];
   my @result;

   if ( $gmsName eq "england" ){
       @result = ("great_britain");
   }
   elsif ( $gmsName eq "united_arab_emirates" ){
       @result = ("uae");
   }
   
#   # Example if one country has more than one overview map
#   elsif ( $gmsName eq "germany" ){
#       @result = ("germany_1", "germany_2", "germany_3", "germany_4");
#   }
   
   else {
      # standard
      @result = $gmsName;
   }
   return @result;
}




sub multiCountry {
   my $function = $_[0]; 
   dbgprint "multiCountry:$function";


   # Local settings
   my $rootDir = "."; #No last slash
   
   # Options, map version 
   my $mapVersion = $opt_v;
   if ( !defined($mapVersion) ) {
       die "Map version not specified! - exit\n";
   }

   # Loads countriInfo.txt in current directory.
   loadCountryInfo();

   my %newCountries = ();
   
   # Loop country info
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
   my $count = 0;
   foreach my $key ( sort(keys(%newCountries)) ) {
       my $gmsName = $key;
#       print "processing gmsName $gmsName\n";

       my $mapVersion2;
       my $alpha2 = $newCountries{$key}{"alpha2"};
       my $continent = $newCountries{$key}{"continent"};


       # Handle the dirName
       my $dirName = $gmsName;
       if ($dirName eq "united_arab_emirates"){
           $dirName = "uae";
       }
       print "processing dirName $dirName\n";
 
       
       if ($function eq "createCoXML" ){
           my $filePath = "${rootDir}/${dirName}/${mapVersion}/xml/" . 
               ${dirName} . "_co.xml";
           dbgprint($filePath);
           open COFILE, ">$filePath";

           # Print co.xml file content.
           print(COFILE '<?xml version="1.0"?>' ."\n");
           print(COFILE '<!DOCTYPE map_generation-mc2 SYSTEM "map_generation-mc2.dtd">' ."\n");
           
           print(COFILE '' ."\n");
           print(COFILE '<map_generation-mc2>' ."\n");
           print(COFILE '   <create_overviewmap overview_ident = "' . 
                 $gmsName .'">' . "\n");
           print(COFILE '      <map map_ident = "' . $alpha2 . '_whole"/>' .
                 "\n");
           print(COFILE '   </create_overviewmap>' ."\n");
           print(COFILE '</map_generation-mc2>' ."\n");
           print(COFILE '' ."\n");
           print(COFILE "\n");
           print(COFILE "\n");

       }
       elsif ($function eq "createVarFile" ){
           my $mapSupplier = "";
           if ( defined $opt_a ) {
              $mapSupplier = "$opt_a";
              dbgprint "mapSupplier $mapSupplier";
           } else {
              die "no map supplier option given";
           }
           
           my $MAPPATH = "";
           if ( defined $opt_d ) {
              $MAPPATH = "$opt_d";
              dbgprint "MAPPATH $MAPPATH";
           } else {
              die "no wf mappath option given";
           }
           

           my $filePath = "${rootDir}/${dirName}/script/" . 
               "variables_" . ${dirName} . "." . $mapVersion . ".sh";
           dbgprint($filePath);
           open VARFILE, ">$filePath" or die "Could not open $filePath. $!";

           # Print variable file content.
           print(VARFILE "MAPRELEASE=\"${mapVersion}\";" . "\n");
           print(VARFILE "# Long version of the map supplier, " .
                         "needed in variable file" . "\n");
           print(VARFILE "# to override makemaps default value TeleAtlas_" . "\n");
           print(VARFILE 'MAP_SUPPLIER="' . $mapSupplier . '_"' . "\n");
           print(VARFILE "\n");
           print(VARFILE "AREAPATH=" . 
                 '"${GENFILESPATH}/${MAPRELEASE}/areafiles"' . "\n");
           print(VARFILE 'CO_CODE="' . $alpha2 . '_"' . "\n");
           print(VARFILE "\n");
           print(VARFILE 'MAPPATH="' . $MAPPATH . '"'. "\n");
           print(VARFILE "\n");
           print(VARFILE 'AREALIST="' . ${alpha2} . 
                 '_whole_municipalItems"' . "\n");
           
           print(VARFILE "\n");
           print(VARFILE "\n");

       }
       elsif ($function eq "createCountryDirs" ){
           my $dirPath = "${rootDir}/${dirName}";
           if (opendir(TSTDIR, $dirPath)){
               dbgprint("Directory $dirPath exists, exits");
               exit;
           }
           
           #Dirs to create
           my $coPolDir = $dirPath . "/" . "countrypol";
           my $scriptDir = $dirPath . "/" . "script";
           my $mapReleaseDir = $dirPath . "/" . $mapVersion;
           my $areaDir = $mapReleaseDir . "/" ."areafiles";
           my $xmlDir = $mapReleaseDir . "/" . "xml";
           
           # Create the directories.
           mkdir($dirPath);
           mkdir($coPolDir);
           mkdir($scriptDir);
           mkdir($mapReleaseDir);
           mkdir($areaDir);
           mkdir($xmlDir);
       }
       else {
           die("Undefined function $function");
       }
       $count++;
       
   } # foreach country code.
   

   dbgprint("Handled $count countries");
   
} # multiCountry











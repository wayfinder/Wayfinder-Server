#!/usr/bin/perl -w
#
#
# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;
use strict;
use File::Basename;
use File::Temp ':mktemp';
use Getopt::Long ':config',
                  'no_auto_abbrev', # Impossible to use -s for --source.
                  'no_ignore_case'  # Case sensitive options.
;

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
my $BASEGENFILESPATH="."
my $scriptPath = "$BASEGENFILESPATH/script";
my $genfilesCountries = "$BASEGENFILESPATH/countries";


# Use perl modules from here or BASEGENFILESPATH/script/perllib
# Todo: get BASEGENFILESPATH into the use-lib-string possible?
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;
use PerlCountryCodes;
use Getopt::Std;

use vars qw( $opt_help 
             $opt_edMigration $opt_m3andCache 
             $opt_diffSecondToFirst $opt_diffSecondToSecond $opt_diffFirstToFirst
             $opt_findSecondGenDir $opt_findFirstGenDir
             $opt_mapProvider $opt_mapRelease 
             $opt_mcmBaseDir $opt_prevBaseDir $opt_mapProduct 
             $opt_yesForSingleDirs
             $opt_prevMapRelease 
             $opt_genType $opt_mc2dir $opt_mapSet $opt_binDir 
             $opt_mapToolBinDir $opt_edCheckFileStoreBasePath );
GetOptions('help|h',
           'edMigration',
           'm3andCache',
           'diffSecondToFirst',
           'diffSecondToSecond',
           'diffFirstToFirst',
           'findSecondGenDir',
           'findFirstGenDir',
           'yesForSingleDirs|y',
           'mapProvider=s',
           'prevMapRelease=s',
           'mapRelease=s',
           'mapProduct=s',
           'mcmBaseDir=s',
           'prevBaseDir=s',
           'genType=s',
           'mc2dir=s',
           'mapSet=s',
           'binDir=s',
           'mapToolBinDir=s',
           'edCheckFileStoreBasePath=s',
           ) or die "Use -help for help.";

# Print help message.
if ( $opt_help ){
   print "
Script to run new map release tasks for multiple countries

USAGE:
   1) Log on to computer that can run the appropriate binaries of the task
   2) Give command: 
            $0 
            options 
            country1 country2 country3 ...


OPTIONS:

WHAT TODO
-edMigration
Run the ed_migration script on the first generation of the countries in tail,
to check validity of map correction records from WASP database (records that
were used for previous map release).
Mandatory options: -mapProvider -mapRelease -prevMapRelease -mapProduct -mcmBaseDir -mapToolBinDir -edCheckFileStoreBasePath
Optional option: -yesForSingleDirs -binDir (for GMS)
(if -binDir not given, will use the GMS in resp country first gen dir)

-m3andCache
Create m3 and search and route caches for the countries in tail.
Fetches the binaries from dir given with option -binDir where it expects
to find MapHttpServer, MapModule makemaps.mcgen and mc2.prop.end
Mandatory options: -mcmBaseDir -genType -mapSet -mc2dir -binDir

-diffSecondToFirst
Create qual diffs comparing a second gen with the first gen of the same map release with script maps_genQualityCheck
Mandatory options: -mcmBaseDir -binDir (for QualTool)

-diffSecondToSecond
Create qual diffs comparing two second generations with the maps_genQualityCheck
script. Prints differences from -prevBaseDir to -mcmBaseDir and prints 
the result in -mcmBaseDir subdirectory diff2otherSecond.
Mandatory options: -prevBaseDir -mcmBaseDir -binDir (for QualTool)

-diffFirstToFirst
Like -diffSecondToSecond, but comparing two first generations.

-findFirstGenDir
Lookup directories with first generation in.
If -mcmBaseDir is not set, the current directory is used.

-findSecondGenDir
Lookup directories with second generation in.
If -mcmBaseDir is not set, the current directory is used.



SCRIPT PARAMETERS OPTIONS:
Depending on what to do, some of these are mandatory

-mapProvider
The map release provider to check ED for, e.g. \"TeleAtlas\" or \"OpenStreetMap\".

-mapRelease
The map release to check ED for, e.g. \"2010_06\" or \"201005\".

-prevMapRelease
The previous map release for this product, e.g. \"2010_03\" or \"201002\".
Used for checking mapssicoord files validity 

-mapProduct
The map product, e.g. \"2010_06_eu\" 201005_world.

-mcmBaseDir
The root directory where the mcm maps are generated for this map release, e.g. \"mcm/TA_2010_06_eu or \"mcm/OSM_201005\".
The script will try to locate the wanted directory for each of the countries in tail, the user of the script will be promted to push yes/no if the directories are the correct ones.

-prevBaseDir
Like -mcmBaseDir. Used when the previous mcm dir is needed.

-yesForSingleDirs -y
When a directory to choose from is uniq, no confirmation is needed when this option is given.

-genType
You want to process mcm maps of \"first\" or \"second\" generations

-mc2dir
For -m3andCache option. Example \"mc2_m17_sr26\". Script will create mc2dir with this name for the M3 maps.

-mapSet
For -m3andCache option, the search and route caches will be created for this map set, 0 or 1.

-binDir
Where to find appropriate binaries for running the task, e.g. 
edmigration: GenerateMapServer 
m3andCache:  MapHttpServer MapModule makemaps.mcgen mc2.prop.end
qualDiffs:   QualTool

-mapToolBinDir
Where to find the MapTool binary to use for running the task

-edCheckFileStoreBasePath
The base dir where the log files etc in migrate extradata will be stored. Subdir will be created for this specific migration with sub dir name \"mapProvider_mapProduct\" which will contain country dirs with migration result for each country.
";
   exit;
}


##############################################################
# Fix relative paths

$opt_mcmBaseDir = relPathToAbsPath( $opt_mcmBaseDir );
$opt_prevBaseDir = relPathToAbsPath( $opt_prevBaseDir ); 
$opt_binDir = relPathToAbsPath( $opt_binDir ); 
$opt_mapToolBinDir = relPathToAbsPath( $opt_mapToolBinDir ); 
$opt_edCheckFileStoreBasePath = relPathToAbsPath( $opt_edCheckFileStoreBasePath );

##############################################################
# Check harcoded paths

if ( ! testDir("$BASEGENFILESPATH") ) {
   errprint "BASEGENFILESPATH does not exist $BASEGENFILESPATH";
   errprint "Edit this script!!";
   exit 1;
}
if ( ! testDir("$genfilesCountries") ) {
   errprint "genfilesCountries does not exist $genfilesCountries";
   errprint "Edit this script!!";
   exit 1;
}
if ( ! testDir("$scriptPath") ) {
   errprint "scriptPath does not exist $scriptPath";
   errprint "Edit this script!!";
   exit 1;
}

##############################################################
# Check what we are about to do 
# and that all mandatory options for the action are given
if ( $opt_edMigration ) {
   if ( !defined $opt_mapRelease ||
        !defined $opt_prevMapRelease ||
        !defined $opt_mapProduct ||
        !defined $opt_mapProvider ||
        !defined $opt_mcmBaseDir ||
        !defined $opt_mapToolBinDir ||
        !defined $opt_edCheckFileStoreBasePath ){
      errprint "Invalid input for edMigration, " .
               "missing option, use -h for help.";
      exit 1;
   }

   dbgprint "Check extradata for provider: $opt_mapProvider";
   dbgprint " release: $opt_mapRelease";
   dbgprint " product $opt_mapProduct";
   dbgprint " (prev release: $opt_prevMapRelease)";
   dbgprint "Base dir for mcm: $opt_mcmBaseDir";
   if (defined $opt_binDir ){
      dbgprint "Will use misc binaries from: $opt_binDir";
   }
   dbgprint "Will use MapTool from: $opt_mapToolBinDir";
   print "\n";

   # run the check on first gen maps
   $opt_genType = "first";

   # We need the MapTool in mapToolBinDir
   if ( ! testDir("$opt_mapToolBinDir") ) {
      errprint "Dir to pick MapTool from does not exist $opt_mapToolBinDir";
      exit 1;
   }
   if ( ! -e "$opt_mapToolBinDir/MapTool" ) {
      errprint "No MapTool in $opt_mapToolBinDir";
      exit 1;
   }

   if ( ! testDir("$opt_edCheckFileStoreBasePath") ) {
      errprint "Dir where to store log files in migrate extradata does not exist $opt_edCheckFileStoreBasePath";
      exit 1;
   }
   
   # We need the ed_migration script
   if ( ! -e "$scriptPath/ed_migration" ) {
      errprint "No ed_migration in $scriptPath";
      exit 1;
   }
}
elsif ( $opt_m3andCache ) {
   if ( !defined $opt_genType ||
        !defined $opt_mc2dir ||
        !defined $opt_mapSet ||
        !defined $opt_binDir ||
        !defined $opt_mcmBaseDir ){
      errprint "Invalid input for m3andCache, " .
               "missing option, use -h for help.";
      exit 1;
   }
   dbgprint "Will generate M3 maps for $opt_genType maps";
   dbgprint "Base dir for mcm: $opt_mcmBaseDir";
   dbgprint "Will generate search&route cache in $opt_mc2dir for map set: $opt_mapSet";
   dbgprint "Will use binaries from $opt_binDir";
   print "\n";
}
elsif ( $opt_diffSecondToFirst ||
        $opt_diffSecondToSecond ||
        $opt_diffFirstToFirst ) {
   my $diffType = "";
   if ( $opt_diffSecondToFirst ) { $diffType = "diffSecondToFirst"; }
   elsif ( $opt_diffSecondToSecond ) { $diffType = "diffSecondToSecond"; }
   elsif ( $opt_diffFirstToFirst ) { $diffType = "diffFirstToFirst"; }
   else { die "specify correct diffType"; }
   if ( !defined $opt_mcmBaseDir ||
        !defined $opt_binDir ){
      errprint "Invalid input for $diffType, " .
               "missing option, use -h for help.";
      exit 1;
   }
   dbgprint "Create qual diffs of type: $diffType";
   dbgprint "Base dir for mcm: $opt_mcmBaseDir";
   dbgprint "QualTool from: $opt_binDir";
   print "\n";
   
   # We need the QualTool in binDir
   if ( ! testDir("$opt_binDir") ) {
      errprint "Dir to pick binaries from does not exist $opt_binDir";
      exit 1;
   }
   if ( ! -e "$opt_binDir/QualTool" ) {
      errprint "No QualTool in $opt_binDir";
      exit 1;
   }

   # We need the maps_genQualityCheck script
   if ( ! -e "$scriptPath/maps_genQualityCheck" ) {
      errprint "No maps_genQualityCheck in $scriptPath";
      exit 1;
   }
   if ( ! -e "$scriptPath/maps_mcmAnalyseQual" ) {
      errprint "No maps_mcmAnalyseQual in $scriptPath";
      exit 1;
   }

   # Diff between same generation type.
   if ( $opt_diffSecondToSecond ||
        $opt_diffFirstToFirst ){
      if ( !defined $opt_prevBaseDir ){
         errprint "Invalid input, missing option -prevBaseDir, use -h for help.";
         exit 1;
      }
   }
   
   # start getting the second gen maps
   $opt_genType = "second"; # for opt_diffSecondToFirst
   if ( $opt_diffSecondToSecond ){
      $opt_genType = "second";
   }
   elsif ($opt_diffFirstToFirst ){
      $opt_genType = "first";
   }
   

}
elsif ( $opt_findSecondGenDir || $opt_findFirstGenDir ) {
    if ( ! defined($opt_mcmBaseDir) ){
        $opt_mcmBaseDir=".";
    }
    $opt_yesForSingleDirs=1;

    if ( $opt_findSecondGenDir ){
        $opt_genType="second";
    }
    else {
        $opt_genType="first";
    }
}
else {
   dbgprint "Nothing todo - exit";
   exit 0;
}



if ($opt_mapRelease and $opt_prevMapRelease) {
   if ($opt_mapRelease eq $opt_prevMapRelease) {
      errprint "The map release and previous map release can not be identical";
      exit 1;
   }
}


# Check that general inparams have valid values
if ( $opt_mcmBaseDir ) {
   if ( ! -d $opt_mcmBaseDir ) {
      errprint "The mcm base dir: $opt_mcmBaseDir does not exist - exit";
      exit 1;
   }
}

if ( $opt_prevBaseDir ) {
   if ( ! -d $opt_prevBaseDir ) {
      errprint "The previous mcm base: $opt_prevBaseDir does not exist - exit";
      exit 1;
   }
}


if ( $opt_binDir ) {
   if ( ! testDir("$opt_binDir") ) {
      errprint "Dir to pick binaries from does not exist $opt_binDir";
      exit 1;
   }
}

if ( $opt_m3andCache ) {
   # for create m3 and caches
   if ( ! testFile("$opt_binDir/MapHttpServer") ) {
      errprint "MapHttpServer missing in $opt_binDir";
      exit 1;
   }
   if ( ! testFile("$opt_binDir/MapModule") ) {
      errprint "MapModule missing in $opt_binDir";
      exit 1;
   }
   if ( ! testFile("$opt_binDir/makemaps.mcgen") ) {
      errprint "makemaps.mcgen missing in $opt_binDir";
      exit 1;
   }
   if ( ! testFile("$opt_binDir/mc2.prop.end") ) {
      errprint "mc2.prop.end missing in $opt_binDir";
      exit 1;
   }
}

if ( $opt_genType ) {
   if ( ($opt_genType eq "first") or
        ($opt_genType eq "second") ) {
      # ok
   } else {
      errprint "Incorrect genType $opt_genType";
      exit 1;
   }
}

if ( $opt_mapSet ) {
   if ( ($opt_mapSet == 0) or
        ($opt_mapSet == 1) ) {
      # ok
   } else {
      errprint "Incorrect mapSet $opt_mapSet";
      exit 1;
   }
}

if ( $opt_mapToolBinDir ) {
   if ( ! testDir("$opt_mapToolBinDir") ) {
      errprint "MapTool bin directory does not exist $opt_mapToolBinDir";
      exit 1;
   }
   if ( ! -e "$opt_mapToolBinDir/MapTool" ) {
      errprint "No MapTool in $opt_mapToolBinDir";
      exit 1;
   }
}

##############################################################
# List the countries to process
my $nbrCountries = scalar @ARGV;
dbgprint "I have $nbrCountries countries to process:";
my $arg=0;
while ( $arg < $nbrCountries ) {
   print "  " . $ARGV[ $arg ] . "\n";
   $arg += 1;
}
print "\n";
if ( $nbrCountries < 1 ) {
   errprint "No countries in tail! - exit";
   exit 0;
}


##############################################################
# Start processes
####################################################

my %varFiles = ();
my %mcmDirs;

# get mcm dirs if mcmBaseDir is defined
if ( $opt_mcmBaseDir) {
   # Get the country mcm directories
   %mcmDirs = getMapGenDirs( $opt_genType, $opt_mcmBaseDir, "mcmBaseDir" );
   if ( ! %mcmDirs ) {
      print "getMapGenDirs not ok - exit\n";
      exit 0;
   }
   print "\n";
}



# ed migration
if ( $opt_edMigration ) {
   my $ok = getVarFiles();
   if ( ! $ok ) {
      print "getVarFiles not ok - exit\n";
      exit 0;
   }
   print "\n";

   runMultiMigration();
}


# m3 and caches
if ( $opt_m3andCache ) {
   runMultiM3();
}

# diffSecondToFirst
if ( $opt_diffSecondToFirst ) {
   runDiffSecondToFirst();
}

# diffSecondToSecond diffFirstToFirst
if ( $opt_diffSecondToSecond ||
     $opt_diffFirstToFirst ){
   runDiffSameGenType();
}


# findSecondGenDir, findFirstGenDir
if ( $opt_findSecondGenDir || $opt_findFirstGenDir ){
    #Nothing more to do.
}

print "\n";
dbgprint "All done!";
exit 0;







####################################################
# Sub functions
####################################################
sub handleUserInput {
   # Handle user input.
   print "Is this OK? (y/n + [enter]): ";
   my $ok;
   my $handleDirStr = <STDIN> ;
   chomp $handleDirStr;
   while (!defined $ok){
      if ( $handleDirStr eq "y" || $handleDirStr eq "Y" ){
         $ok = 1;
      }
      elsif ( $handleDirStr eq "n" || $handleDirStr eq "N" ){
         $ok = 0;
      }
      else {
         print "\nIncorrect input, press \"y\" or \"n\" and enter key: ";
         $handleDirStr = <STDIN> ;
         chomp $handleDirStr;
      }
   }
   return $ok;
}

sub handleUserInputNumChoosing {
   my $result;
   my $handleDirStr = <STDIN> ;
   chomp $handleDirStr;
   while (!defined $result){
      if ( $handleDirStr =~ /^[0-9][0-9]*/ ){
         $result = $handleDirStr;
      }
      else {
         print "\nIncorrect input, only numbers are allowed: ";
         $handleDirStr = <STDIN>;
         chomp $handleDirStr;
      }
   }
   return $result;
}

sub get2letterCode {
   my $country = $_[0];
   my $iso2letter = getAlpha2FromCountry($country);
   return $iso2letter;
}

sub getMapGenDirs {
   my $genType = $_[0];
   my $mcmBaseDir = $_[1];
   my $optionDir = $_[2];
   dbgprint "Collects country _$genType\_ gen " .
            "directories in $optionDir $mcmBaseDir for all countries...";
   
   my %mcmDirs = ();
   
   # Loop all countries in tail
   my $arg = 0;
   while ( $arg < $nbrCountries ) {
      print "\n";
      my $country = $ARGV[ $arg ];
      print "country: $country\n";
      $arg += 1;
      

      my $iso2letter = get2letterCode($country); 
      if ( ! defined $iso2letter ) {
         errprint "Could not find a valid iso 2-letter " .
             "country code for $country";
         die "error";
      }
      
      # Check for not first/second tagged directories.
      my $checkDirs = `find $mcmBaseDir -type d -name $country -o -type d -name '$iso2letter\_*' |grep -iv first| grep -iv second`;
      if ( length $checkDirs > 0 ){
          errprint "Directories with neither first or second tag exists.".
              " Remove them, rename them (include first or second in their".
              " names or directory name), or move them to a better place.".
              " Files listed below:";
          print "$checkDirs\n";
          die;
      }

      my $findExpr = "find $mcmBaseDir -type d -name $country -o -type d -name '$iso2letter\_*' |grep -i $genType";
      #dbgprint $findExpr;
      `$findExpr > tmp_$country`;
      my $dir = `$findExpr`;
      chomp $dir;

      print "dir: $dir\n";
      
      if ( length $dir < 1 ) {
         errprint "cannot find dir for $country - you have to process this country manually";
         errprint "Tried to find dirs with exact name '$country' or iso2letter '$iso2letter\_*'";
         `rm tmp_$country >& /dev/null`;
         #$mcmDirs{ "$country" } = "$dir";
         die "exit";
         # another option is to go via the country.mif file...
         #my $mif = `find $opt_mcmBaseDir -type f -name "$country.mif"`;
         #chomp $mif;
         #print "mif: $mif\n";
      }

      # Make sure we found only one directory for the country, 
      # then ask the user if that dir was ok.
      my $nbrDirs = `grep -c "." tmp_$country`;
      if ( $nbrDirs != 1 ) {
         print "Choose which directory to use or abort if the right one is".
               " not included.\n";

         my $i=1;
         my @dirs;
         open (COUNTRYFILE, "tmp_$country");
         while (<COUNTRYFILE>) {
            chomp;
            print "\n$i: $_";
            push @dirs, $_;
            $i++;
         }
         print "\n";

         # user input
         print "(1,2... + [enter]): ";
         my $choosenDirIdx = 666;
         while ( ! defined $dirs[$choosenDirIdx-1] ){         
            $choosenDirIdx = handleUserInputNumChoosing();
            #print "$choosenDirIdx\n";
            if ( ! defined $dirs[$choosenDirIdx-1] ){
               print "You have not choosen a valid directory, please try again: ";
            }
            else {
               $mcmDirs{ "$country" } = $dirs[$choosenDirIdx-1];
               print "You have choosen: ".$mcmDirs{ "$country" }."\n";
            }
         }
      }
      else {
         if ( defined ($opt_yesForSingleDirs) && $opt_yesForSingleDirs ){
            # No confirmation needed.
            print "Auto confirmed\n";
            $mcmDirs{ "$country" } = "$dir";
         }
         else {
            # user input
            my $ok = handleUserInput();
            if ( $ok ) {
               $mcmDirs{ "$country" } = "$dir";
            }
         }
      }
      `rm tmp_$country >& /dev/null`;
   
   }
   print "\n";

   if ( scalar (keys %mcmDirs) < 1 ) {
      print "No map gen dirs collected - skip\n";
      return %mcmDirs;
   }

   dbgprint "Collected these " . scalar (keys %mcmDirs) . " map dirs in $optionDir:";
   foreach my $country ( sort keys %mcmDirs ) {
      print " $country " . $mcmDirs{ "$country" } . "\n";
   }
   # user input
   my $result = handleUserInput();
   if ( ! $result ) {
      # not ok, reset the hash
      %mcmDirs = ();
   }
   print "\n";

   return ( %mcmDirs );

} # getMapGenDirs

sub getVarFiles {
   my $usePrevRelease = shift;
   dbgprint "Collects variable files for all countries...";

   my $mapRelease = $opt_mapRelease;
   if (defined $usePrevRelease) {
      $mapRelease = $opt_prevMapRelease;
   }
   dbgprint "For map release $mapRelease";
   
   # Loop all countries in tail
   $arg = 0;
   while ( $arg < $nbrCountries ) {
      my $country = $ARGV[ $arg ];
      print "country: $country\n";
      $arg += 1;
      
      # Tele Atlas varfile names differ from other supplier's varfile names
      my $varFile = "";
      if ( $opt_mapProvider eq "TeleAtlas" ) {
         $varFile = `find $genfilesCountries/$country -name "variables_*\.$mapRelease\.sh"`;
      } else {
         $varFile = `find $genfilesCountries/$country -name "variables_*\.${opt_mapProvider}_$mapRelease\.sh"`;
      }   
      chomp $varFile;
      #print "varFile: $varFile\n";
      # if no varFile -> problems!
      if ( length $varFile < 1 ) {
         errprint "cannot find var file for $country - you have to process this country manually";
         die "exit";
         
      }

      # skip user input
      #my $ok = handleUserInput();
      #if ( $ok ) {
         $varFiles{ "$country" } = "$varFile";
      #}
   
   }
   print "\n";
   
   dbgprint "Collected these variable files:";
   foreach my $country ( sort keys %varFiles ) {
      print " $country " . $varFiles{ "$country" } . "\n";
   }
   # user input
   my $result = handleUserInput();
   if ( ! $result ) {
      return $result;
   }
   print "\n";

   return $result;

} # getVarFiles


sub runMultiMigration {
   print "\n";
   dbgprint "Start runMultiMigration";
   
   my $gmsBinOption = "";
   if (defined $opt_binDir ){
      if ( ! testFile("$opt_binDir/GenerateMapServer") ) {
         errprint "GenerateMapServer missing in opt_binDir $opt_binDir";
         exit 1;
      }
      $gmsBinOption = " -gmsBin $opt_binDir/GenerateMapServer ";
      # test the GMS
      my $logfile = "gms_h_" . $opt_mapProvider . "_" . $opt_mapProduct . ".log";
      my $command = "$opt_binDir/GenerateMapServer -h >& $logfile";
      `$command`;
      my $exitCode = $?;
      if ( $exitCode != 0 ){
         errprint "Problem with GenerateMapServer, exit code $exitCode";
         errprint "See log in $logfile";
         exit($exitCode);
      }
   }
         

   # Dir where the country check directories are created
   my $edCheckFileStorePath =
      "$opt_edCheckFileStoreBasePath/" . $opt_mapProvider . "_" . $opt_mapProduct . "/";
   
   
   foreach my $country ( sort keys %mcmDirs ) {
      print "\n";
      dbgprint "$country";

      # Check if this country was already processed
      my $runCommand = 1;
      # either logfile already exists
      my $logFileBaseName = "migrate_$country\_$opt_mapRelease";
      if ( -s "$logFileBaseName.log" ) {
         print "   logfile already exists - will skip country\n";
         print "   $logFileBaseName\n";
         $runCommand = 0;
      }
      else {
         # or if the country check directory already exists
         my $countryCheckFileDir = "$edCheckFileStorePath/$country";
         if ( -d $countryCheckFileDir ) {
            print "   check dir already exists - will skip country\n" .
                  "   ($countryCheckFileDir)\n";
            $runCommand = 0;
         }
      }

      if ( $runCommand ) {
         if ( ! defined $mcmDirs{ "$country" } ) {
            print "   no mcm dir defined - will skip country\n";
            $runCommand = 0;
         }
         if ( ! defined $varFiles{ "$country" } ) {
            print "   no variable file defined - will skip country\n";
            $runCommand = 0;
         }
      }
      
      if ( $runCommand ) {
         my $command =
            "$scriptPath/ed_migration " .
            " -country $country" .
            " -prevMapRelease $opt_mapProvider\_$opt_prevMapRelease" .
            " -mapRelease $opt_mapProvider\_$opt_mapRelease" .
            " -mapProduct $opt_mapProvider\_$opt_mapProduct" .
            " -firstGenDir " . $mcmDirs{ "$country" } .
            " -varFile " . $varFiles{ "$country" } . 
            " -mapToolBin $opt_mapToolBinDir/MapTool " .
            " $gmsBinOption " .
            " 2>&1 | $scriptPath/mlog -noStdOut $logFileBaseName";

         print "\n command:\n" .
               " $command\n\n";
      
      
         `$command`;
         my $exitCode = $?;
         if ( $exitCode != 0 ){
            `chgrp maps $edCheckFileStorePath -R >& /dev/null`;
            errprint "Command for $country failed with exit code $exitCode";
            exit($exitCode);
         }
         `chgrp maps $edCheckFileStorePath -R >& /dev/null`;
      }
   }
   `chgrp maps * >& /dev/null`;
   
   dbgprint "Done runMultiMigration!";
} # runMultiMigration


sub runMultiM3 {
   print "\n";
   dbgprint "Start runMultiM3";

   my %verifyCount = ();

   foreach my $country ( sort keys %mcmDirs ) {
      print "\n";
      dbgprint "$country";

      my $countryDir = $mcmDirs{ "$country" };
      print " goto country dir $countryDir\n";
      #`cd $countryDir`;
      chdir $countryDir or die "Cannot chdir to $countryDir";
      if ( -d $opt_mc2dir ) {
         print " mc2dir already exists\n";
      }

      # copy the makemaps.mcgen and mc2.prop.end
      # (remove any existsing first, incase we have links..)
      print " copy makemaps.mcgen\n";
      `rm makemaps.mcgen >& /dev/null`;
      `cp $opt_binDir/makemaps.mcgen .`;
      print " copy mc2.prop.end\n";
      `rm mc2.prop.end >& /dev/null`;
      `cp $opt_binDir/mc2.prop.end .`;

      # copy the MapHttpServer and MapModule
      # (remove any existsing first, incase we have links..)
      print " copy MapHttpServer and MapModule\n";
      `rm MapHttpServer >& /dev/null`;
      `rm MapModule >& /dev/null`;
      my $cpCmd = "cp $opt_binDir/Map* .";
      `$cpCmd`;

      # create the m3 and s&r caches
      my $mapSet = $opt_mapSet;
      my $command =
         "./makemaps.mcgen -fromM3 -mapSet $mapSet -mc2Dir $opt_mc2dir " .
         "2>&1 | $scriptPath/mlog -noStdOut cache_$mapSet";
      print "\n Command:\n" .
            " $command\n\n";
      `$command`;
      my $exitCode = $?;
      if ( $exitCode != 0 ){
         `chmod a+r $opt_mc2dir -R >& /dev/null`;
         `chmod g+w $opt_mc2dir -R >& /dev/null`;
         errprint "Command for $country failed with exit code $exitCode";
         exit($exitCode);
      }
      `chmod a+r $opt_mc2dir -R >& /dev/null`;
      `chmod g+w $opt_mc2dir -R >& /dev/null`;
   
      # count number of maps
      my @mcmList = glob "0*.mcm";
      my @m3List  = glob "$opt_mc2dir/0*.m3";
      my @sorList = glob "$opt_mc2dir/cache$mapSet/0*.cached";
      print "Verify count $country: mcm=" . scalar(@mcmList) .
            " m3=" . scalar(@m3List) .  " sor=" . scalar(@sorList) . "\n";
      $verifyCount{"$country"}{"mcm"} = scalar(@mcmList);
      $verifyCount{"$country"}{"m3"} = scalar(@m3List);
      $verifyCount{"$country"}{"sor"} = scalar(@sorList);
   }

   print "\n";
   dbgprint "Verification of runMultiM3 (mcm m3 s&r)";
   foreach my $country ( sort keys %verifyCount ) {
      my $nbrMcm = $verifyCount{"$country"}{"mcm"};
      my $nbrM3  = $verifyCount{"$country"}{"m3"};
      my $nbrSor = $verifyCount{"$country"}{"sor"};
      print "$country" .
            " $nbrMcm $nbrM3 $nbrSor(" . $nbrSor/2.0 . ")\n";
      if ( ($nbrMcm != $nbrM3) or
           ($nbrMcm != ($nbrSor/2.0) ) ) {
         print " runMultiM3 problem for country $country\n";
      }
   }
   print "\n";

   
   dbgprint "Done runMultiM3!";
} # runMultiM3


sub runDiffSecondToFirst {
   # We already asked for the second gens in %mcmDirs
   # now ask for the first gens
   my %firstDirs = getMapGenDirs( "first", $opt_mcmBaseDir, "mcmBaseDir" );
   if ( ! %firstDirs ) {
      print "getMapGenDirs not ok - exit\n";
      exit 0;
   }
   print "\n";

   
   if ( scalar (keys %mcmDirs) != scalar (keys %firstDirs) ) {
      errprint "Number of second gen dirs " .
               "does not match number of firstGen dirs";
      exit 1;
   }

   dbgprint "Do qual diffs, second to first";
   
   foreach my $country ( sort keys %mcmDirs ) {
      print "\n";
      dbgprint "$country";

      my $secondDir = $mcmDirs{ "$country" };
      my $firstDir = $firstDirs{ "$country" };
      if ( !defined $secondDir || !defined $firstDir ) {
         errprint "Either first or second gen dir not defined for country";
         exit 0;
      }

      my $ok = makeQualDiffForOneCountry(
            $country, $secondDir, $firstDir, "diff2first" );
   }
   dbgprint "Done runDiffSecondToFirst!";
} # runDiffSecondToFirst


sub runDiffSameGenType {
   # We already asked for the current gens in %mcmDirs
   # now ask for the previous gens
   my %prevMcmDirs = getMapGenDirs( $opt_genType, $opt_prevBaseDir, "prevBaseDir" );
   if ( ! %prevMcmDirs ) {
      print "getMapGenDirs not ok - exit\n";
      exit 0;
   }
   print "\n";

   
   if ( scalar (keys %mcmDirs) != scalar (keys %prevMcmDirs) ) {
      errprint "Number of current gen dirs " .
               "does not match number of previous gen dirs";
      exit 1;
   }

   dbgprint "Do qual diffs, $opt_genType to $opt_genType";
   
   foreach my $country ( sort keys %mcmDirs ) {
      print "\n";
      dbgprint "$country";

      my $dir1 = $prevMcmDirs{ "$country" };
      my $dir2 = $mcmDirs{ "$country" };
      if ( !defined $dir1 || !defined $dir2 ) {
         errprint "Missing directory for $country";
         exit 0;
      }

      dbgprint "Diffing:";
      dbgprint $dir1;
      dbgprint $dir2;
      my $ok = makeQualDiffForOneCountry(
            $country, $dir2, $dir1, "diff2other_$opt_genType" );
   }
   dbgprint "Done runDiffSameGenType!";
} # runDiffSameGenType


sub makeQualDiffForOneCountry {
   my $country = $_[0];
   my $countryDir = $_[1];
   my $compareDir = $_[2];
   my $diffDirName = $_[3];

   print "makeQualDiffForOneCountry $country: $diffDirName in $countryDir\n";
   
   print " goto country dir $countryDir\n";
   chdir $countryDir or die "Cannot chdir to countryDir";
   my $runCommand = 1;
   if ( -d $diffDirName ) {
      #print " diffdir $diffDirName already exists - skip\n";
      #$runCommand = 0;
   } else {
      `mkdir $diffDirName`;
      #print " created diffdir $diffDirName\n";
   }
   
   if ( $runCommand ) {
      chdir $diffDirName or die "Cannot chdir to diffDir";
      my $command =
         "$scriptPath/maps_genQualityCheck -bin $opt_binDir " .
         "$compareDir $countryDir >& genQualDiff.log";
      print "\n Command:\n" .
            " $command\n\n";
      `$command`;
      my $exitCode = $?;
      if ( $exitCode != 0 ){
         `chmod a+r . -R >& /dev/null`;
         `chmod g+w . -R >& /dev/null`;
         errprint "Command for $country failed with exit code $exitCode";
         exit($exitCode);
      }
      `chmod a+r . -R >& /dev/null`;
      `chmod g+w . -R >& /dev/null`;

      # also make the analyse of the long maps qual diff file
      analyseLongQualDiff( $country, $countryDir, $diffDirName );
   }
   
} # makeQualDiffForOneCountry

sub analyseLongQualDiff {
   my $country = $_[0];
   # the mcm dir of the country, for which qualdiff should be analysed
   my $countryDir = $_[1]; 
   # the name of the qualdiff dir in countryDir
   my $diffDirName = $_[2];
   
   my $analyzDir = "$opt_mcmBaseDir/analyzeQual_$diffDirName";

   `mkdir -p $analyzDir`;
   print "print $country analyze to $analyzDir\n";
   
   chdir "$countryDir/$diffDirName" or die "Cannot chdir to countryDir/diffDirName";

   `$scriptPath/maps_mcmAnalyseQual -p 4 *_MapsQualDiff.log >& $analyzDir/$country\_decrease4_1000.log`;
   `$scriptPath/maps_mcmAnalyseQual -p 4 -i *_MapsQualDiff.log >& $analyzDir/$country\_increase4_1000.log`;
   `$scriptPath/maps_mcmAnalyseQual -p 4 -n 500 *_MapsQualDiff.log >& $analyzDir/$country\_decrease4_500.log`;
   `$scriptPath/maps_mcmAnalyseQual -p 4 -n 500 -i *_MapsQualDiff.log >& $analyzDir/$country\_increase4_500.log`;
   `grep -i "Kilometer road" *_MapsQualDiff.log >& $analyzDir/$country\_misc.log`;
   `grep "    x" *_MapsQualDiff.log | grep -v "  nbr gfx data" | grep -v "  nbr items with location" >> $analyzDir/$country\_misc.log`;
   `grep -v "  (pt " $analyzDir/$country\_increase4_500.log >& $analyzDir/$country\_increase4_500_noPOIs.log`;

   `chmod a+r $analyzDir -R >& /dev/null`;
   print "\n";
} # analyseLongQualDiff


sub relPathToAbsPath {
   my $path = $_[0];
   if ( defined $path ){
      if ( $path =~ /^[^\/].*/ ){
         # This is a relative path.

         if ( ! testDir($path) ){
            errprint "Something wrong with relative path: $path";
            die;
         }

         my $origDir = `pwd`;
         chdir $path;
         $path = `pwd`;
         chomp $path;
         chdir $origDir;
      }
   }
   return $path;
}



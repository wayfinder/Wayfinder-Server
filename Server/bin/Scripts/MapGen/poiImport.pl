#!/usr/bin/perl -w
#
#
# Script for parsing text files with poi data into WASP.
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



# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use POIObject;
use PerlWASPTools;
use PerlTools;
use PerlCategoriesObject;

use strict;
use DBI;

use Getopt::Std;

use vars qw( $opt_c $opt_f $opt_h $opt_i $opt_l $opt_p $opt_r 
             $opt_s $opt_t $opt_C $opt_q
             $opt_u $opt_d $opt_M $opt_o $opt_n $opt_X $opt_m );
use vars qw( $m_parseOnly $m_quickPO $m_countryID $m_languageID $m_sourceID
             $m_validFromID $m_validToID $m_categoriesObj
             $m_setSupplier $m_writeCPIF );

# Holds the map versions to use as valid to and valid from version.
my %m_mapVerByCountryID=(); 

getopts('hpqnmd:Mc:i:l:r:s:t:f:C:u:o:X:') or die;

# Decompress .gz files to standard out for this script to read from.
@ARGV = map { /^.*\.gz$/ ? "gzip -dc $_ |" : $_  } @ARGV;

if (defined $opt_h) {
   print <<EOM;
Script for parsing text files with poi data into WASP. Needs Perl modules
POIObject.pm PerlWASPTools.pm etc to work. 
Usage: $0 [ options ] [ file(s) ]

Options:
-h          Display this help.
-p          Parse only.
-q          Quick. Adds POIs to WASP with quick method. No validity check of 
            parameters in the POIObject.
-m          POIs are map data POIs, i.e. comes with a map release
            and should thus have validToVersion=validFromVersion.
-n          Allow null coordinate. POIs with no symbol coordinate in e.g.
            CPIF file will still be added to WASP (default not to add them).
-c string   country
-i integer  inUse 0 or 1
-l string   default language for name and info val
-s string   source
-r string   validFromVersion
-t string   validToVersion
-u string   Supplier string (UTF8 char encoding) 
            (any supplier string in SPIF/CPIF etc will be replaced)
-C string   convert
            Convert coordinate value to mc2 coordinate.
            Possible values:
               wgs84    Convert from WGS84 on decimal form
               ta_wgs84 Convert from WGS84 on integer form with decimal
                        point shifted 7 positions to the right.
-d string   Directories where to find the map version of countries to set as
            valid to and valid from version, from the mapOrigin.txt file in
            countries/country/before_merge dir. Typically west and east world
            countries directories. Give on the format: 'dir1,dir2'. 
            Default is that all countries that are defined in the WASP 
            POICountries table are expected, else give option -M for 
            okWithMissingCountries.
            Special value is NOT_USED. Use it with care. If this value is 
            given, the map version wont be fetched from the country 
            directories. Only values given in option -o will be used.
-M          okWithMissingCountries. Give this option if the -d option
            mapCountriesDirs only contain some countries.
-o string   String on the format 'IS03166:MAPVER,ISO3166:MAPVER'. Overrides map
            version values found in directories pointed at by -d.
            Example value: sg:TA_2007_02,hk:TA_2006_11,uk:TA_2007_01
-X string   File name of the category XML file.

-f string   The function to run. Valid values are:
       readCPIF ( (validFromVersion), (source), (country), (validToVersion),
                  (convert) (quick) (supplierString) (inUse)
                  (allowNullCoord) (categoryXML) )
            Parse one file with POI data in CPIF - complex POI import format
            version 1.0 and add the POIs to WASP db.
            Give validFromVersion if not in CPIF file, source if not in CPIF,
            country if not in CPIF, inUse if not in CPIF, validToVersion 
            if that should be set.
            The validFromVersion and validToVersion are preferably set via 
            the -d option mapCountriesDirs.
            (Uses the info from CPIF file first, then info given with options)
            Combine with -p for parse only (not adding POIs to WASP db).
            To be able to handle categories, add -X with category xml file.
            NB! before import: need to convert to UTF-8 char encoding and 
                run dos2unix to avoid windows-newlines!
      readSPIF ( validFromVersion, source, country, inUse, language,
                 (validToVersion), (convert) (quick)
                 (supplierString))
            Parse one file with POI data in SPIF - simple POI import format
            version 2.2 ff and add the POIs to WASP db. SPIF country is used
            before -c country.
            Combine with -p for parse only (not adding POIs to WASP db).
            NB! before import: need to convert to UTF-8 char encoding and 
                run dos2unix to avoid windows-newlines!
      readGDFPOIFormat
            ( source validFromVersion validToVersion inUse supplierString )
            Parse a GDF POI file (produced by map gen), and add to WASP.
            Combine with -p for parse only (not adding POIs to WASP db).
      convertSPIFtoCPIF
           If you want to convert a SPIF file to CPIF.
      trimMidMifSPIF
           Minor modifications of midmifSPIFs to get a true SPIF
           - replace comma with semicolon as field separators
           - remove char-signs round text fields
      spifFromXlsCsv
            Create clean SPIF from a semi-colon separated utf8 csv file
            saved from xls. The csv has the SPIF attribtues in the correct
            column order, but might be in need of some clean-up:
            - removes \"-signs, remove leading and tracing space-chars
              in each attribute, adds srcRef if not present (it will
              be rowNbr + a constant harcoded in the function)

EOM
  exit 0;
}


# Connect to database
my $dbh = db_connect();
if (!defined $dbh) {
   die "Cannot connect to the database ($DBI::errstr)\n";
}


# field separator
my $m_fieldSep = "<¡>";
$m_writeCPIF = 0;

# Check that POIInfoKeys and POIAttributeTypes tables are in sync
# meaning max(keyID)+1000 = max(attrTypeID)
if (1) {
   my $maxATID = getMaxAttributeTypeID($dbh);
   my $sth = $dbh->prepare("SELECT max(id) FROM POIInfoKeys;");
   $sth->execute;
   my $maxKeyID = $sth->fetchrow();
   if ( defined($maxATID) and defined($maxKeyID) and
        ( ($maxKeyID+1000) == $maxATID) ) {
      print "POIInfoKeys and POIAttributeTypes in sync\n";
   } else {
      die "POIInfoKeys and POIAttributeTypes NOT in sync - check max..\n"
   }
}




# Set parse only flag
$m_parseOnly = 0;
if ( defined($opt_p) ) {
   $m_parseOnly = 1;
   print "Running parse-only\n";
}

# Set supplier flag
$m_setSupplier = 0;
if (defined $opt_u) {
   $m_setSupplier = 1;
   print "Will set supplier string to all PIF POIs '$opt_u'\n";
}

# Check that any given option is valid
if (defined $opt_c) {
   $m_countryID = getCountryIDFromName($dbh, $opt_c);
   if ( !defined($m_countryID) ) {
      die "Error: Incorrect country given ($opt_c)\n";
   }
}
if (defined $opt_l ) {
   $m_languageID = getLanguageIDFromName($dbh, $opt_l);
   if ( !defined($m_languageID) ) {
      die "Error: Incorrect language given ($opt_l)\n";
   }
}
if (defined $opt_r) {
   $m_validFromID = getVersionIDFromName($dbh, $opt_r);
   if ( !defined($m_validFromID) ) {
      die "Error: Incorrect validFromVersion given ($opt_r)\n";
   }
}
if (defined($opt_s)) {
   $m_sourceID = getSourceIDFromName($dbh, $opt_s);
   if ( !defined($m_sourceID) ) {
      die "Error: Incorrect source given ($opt_s)\n";
   }
}
if (defined $opt_t) {
   $m_validToID = getVersionIDFromName($dbh, $opt_t);
   if ( !defined($m_validToID) ) {
      die "Error: Incorrect validToVersion given ($opt_t)\n";
   }
}
if (defined $opt_d) {
   my $mapCountriesDirs = $opt_d;
   if ( ! ($mapCountriesDirs eq "NOT_USED") ){
      # It is possible to not use the countries directories. In this case the
      # map version of the POI data must be given by the -mapVerOverride option.

      # Get map version (map origin) for each country that is defined
      # in POICountries table. The map origin will be used as
      # validFromVersion for the imported POIs 
      # (if mapPOIs also for the validToVersion)
      getMapVersionByCountryID($dbh, # dies on error.
                               $mapCountriesDirs, 
                               \%m_mapVerByCountryID,
                               $opt_M );
   }
   else {
      if (!defined $opt_o){
         die "Error: -d is $mapCountriesDirs but no -o is given.\n";
      }
   }

}
if (defined $opt_o){
   handleMapVerOverrides($dbh, $opt_o, \%m_mapVerByCountryID);

   foreach my $key (keys(%m_mapVerByCountryID)){
      dbgprint "poiImport $key:$m_mapVerByCountryID{$key}";
   }

}
if (defined $opt_i) {
	if( ($opt_i == 0) or ($opt_i == 1) ) {
		# correct values
	} else {
		die "Error: Incorrect inUse value set ($opt_i)\n";
	}
}

if ( defined $opt_q ){
   $m_quickPO=1;
}

if ( defined $opt_X ){
   dbgprint "Loading categories from XML";
   $m_categoriesObj = PerlCategoriesObject->newObject( );
   $m_categoriesObj->loadFromXML($opt_X);
}


# Decide what function to run
if (defined($opt_f)){
   if ($opt_f eq "readCPIF") {
      readCPIF();
   } elsif ($opt_f eq "readSPIF") {
      readSPIF();
   } elsif ($opt_f eq "readGDFPOIFormat") {
      readGDFPOIFormat();
   } elsif ($opt_f eq "convertSPIFtoCPIF") {
      convertSPIFtoCPIF();
   } elsif ($opt_f eq "trimMidMifSPIF") {
      trimMidMifSPIF();
   } elsif ($opt_f eq "spifFromXlsCsv") {
      spifFromXlsCsv();
   } else {
       db_disconnect($dbh);
      die "Error Unknown function: $opt_f\n";
   }
} else {
    db_disconnect($dbh);
   die "Error: No function specified!\n";
}

db_disconnect($dbh);

exit;



# Create POIs from a CPIF file and add them to db
sub readCPIF {
   if ( scalar(@ARGV) < 1 ) {
      die "readCPIF: give CPIF file in tail\n";
   }
   dbgprint "readCPIF $ARGV[0]";
   
   # Do not require validFromVersion (it may be defined in CPIF and does not
   # have to be the same for all POIs in CPIF)
   #if ( !defined($m_validFromID) ) {
   #   die "readCPIF: Must specify validFromVersion - exit\n";
   #}

   (my $nbrPOIs, my $nbrPOISNotAdded, my $nbrPOIsNotAddedDueToEP) =
         generalReadPIF( $ARGV[0], "readCPIF" );
   
   dbgprint "readCPIF: $nbrPOIs POIs created and added to WASP";
   dbgprint "readCPIF: $nbrPOISNotAdded POIs NOT added " .
            "($nbrPOIsNotAddedDueToEP EPs)";
}

sub stripOuterQuotes {
   my $text = $_[0];
   $text =~ s/^\"//;
   $text =~ s/\"$//;
   return $text;
}



# Read a SPIF and write the POIs to CPIF
sub convertSPIFtoCPIF {
   dbgprint "convertSPIFtoCPIF $ARGV[0]";

   if ( !defined($m_validFromID) ) {
      die "convertSPIFtoCPIF: Must specify validFromVersion - exit\n";
   }
   if ( !defined($m_sourceID) ) {
      die "convertSPIFtoCPIF: Must specify source - exit\n";
   }
   if ( !defined($m_languageID) ) {
      die "convertSPIFtoCPIF: Must specify default language - exit\n";
   }
   
   # run parse only not to add anyting to WASP from the SPIF by accident
   $m_parseOnly = 1;

   # run writeCPIF
   $m_writeCPIF = 1;
   
   # open the cpif file and write it empty
   open CPIF, ">cpif.txt";
   print CPIF "";

   # read SPIF and write CPIF
   (my $nbrPOIs, my $nbrPOISNotAdded, my $nbrPOIsNotAddedDueToEP) =
      generalReadPIF( $ARGV[0], "readSPIF" );
   
   dbgprint "convertSPIFtoCPIF: wrote $nbrPOISNotAdded POIS to 'cpif.txt'";
}


# Create POIs from a SPIF file and add them to db
sub readSPIF {
   dbgprint "readSPIF $ARGV[0]";

   # Not required as direct imparams, can also be collected from 
   # the -d option for the m_mapVerByCountryID hash
   #if ( !defined($m_validFromID) ) {
   #   die "readSPIF: Must specify validFromVersion - exit\n";
   #}
   if ( !defined($m_sourceID) ) {
      die "readSPIF: Must specify source - exit\n";
   }
   if ( !defined($m_countryID) ) {
      die "readSPIF: Must specify country - exit\n";
   }
   if ( !defined($m_languageID) ) {
      die "readSPIF: Must specify default language - exit\n";
   }
   if ( !defined($ARGV[0]) ) {
      die "readSPIF: no poi spif file given in tail - exit\n";
   }
   if ( !defined($opt_i) ) {
      die "readSPIF: Must specify inUse - exit\n";
   }

   (my $nbrPOIs, my $nbrPOISNotAdded, my $nbrPOIsNotAddedDueToEP) =
      generalReadPIF( $ARGV[0], "readSPIF" );
   
   dbgprint "readSPIF: $nbrPOIs POIs created and added to WASP";
   dbgprint "readSPIF: $nbrPOISNotAdded POIs NOT added " .
            "($nbrPOIsNotAddedDueToEP EPs)";
}


# Create POIs from a GDF POI file and add them do db
sub readGDFPOIFormat {
   dbgprint "readGDFPOIFormat $ARGV[0]";
   if ( scalar @ARGV < 1 ) {
      die "No files in tail - exit";
   }
   if ( ! testFile("$ARGV[0]") ) {
      die "File in tail does not exist";
   }
   
   if ( !defined($m_sourceID) ) {
      die "readGDFPOIFormat: Must specify source - exit\n";
   }

   if ( scalar(keys(%m_mapVerByCountryID)) < 1 ){
      errprint "m_mapVerByCountryID empty, exits.";
      die;
   }

   if ( !defined($opt_i) ) {
      die "readGDFPOIFormat: Must specify inUse - exit\n";
   }
   if ( !defined($opt_u) ) {
      die "readGDFPOIFormat: Must specify supplier string - exit\n";
	}
   
   my $printValidFrom = "[BY_COUNTRY]";
   if ( defined $m_validFromID ){
      $printValidFrom = $m_validFromID;
   }
   my $printValidTo = "[BY_COUNTRY]";
   if ( defined $m_validToID ){
      $printValidTo = $m_validToID;
   }
   dbgprint "readGDFPOIFormat sourceID=$m_sourceID " .
            "validFromID=$printValidFrom validToID=$printValidTo " .
            "inUse=$opt_i supplier='$opt_u'";

   (my $nbrPOIs, my $nbrPOISNotAdded, my $nbrPOIsNotAddedDueToEP) =
      generalReadPIF( $ARGV[0], "readGDFPOIFormat" );
   
   dbgprint "readGDFPOIFormat: $nbrPOIs POIs created and added to WASP";
   dbgprint "readGDFPOIFormat: $nbrPOISNotAdded POIs NOT added " .
            "($nbrPOIsNotAddedDueToEP EPs)";
} # readGDFPOIFormat


# General method that reads any Wayfinder POI import format (PIF) files
# and creates POIs.
# 2 inparams. The first gives the file name, the second gives 
# the type of file format.
sub generalReadPIF {
   
   my $fileName = $_[0];
   my $typeOfPIF = $_[1];
   dbgprint "generalReadPIF: $typeOfPIF $fileName";
   if ( defined($m_quickPO) ){
      dbgprint "Using quick POIObject";
   }
   else {
      dbgprint "Not using quick POIObject";
   }
   if ( defined($opt_n) ){
      dbgprint "Adding POIs with null coordinates to WASP";
   }
   else {
      dbgprint "Not adding POIs with null coordinates to WASP";
   }


   # If convertSPIFtoCPIF
   my $cpifFile = "";
   if ( $m_writeCPIF ) {
      $cpifFile = "cpif.txt";
   }

   
   # Loop PIF infile "<>". Create a POIObject and let it read/create itself 
   # from file. The POIObject returns filePos where to start reading info
   # for next POI.
   # Store the POI objects in a hash, to parse all POI files before
   # adding to WASP
   my $filePos = 0;
   my %allPOIs = ();
   my $nbrPOIsInFile = 0;
   my $nbrPOIsNotAdded = 0;
   my $nbrPOIsNotAddedDueToEP = 0;

   my $nbrParsed = 0;
   my $nbrParsedInBatch = 0;
   my $nbrPOIsAddedToWASP = 0;
   
   my $cont = 1;
   while ( $cont ) {
      
      # Build a POI object
      my $poiObject = POIObject->newObject( );
      if ( defined($m_quickPO) ){
         $poiObject->noValidationByDb();
      }
      $poiObject->setDBH($dbh);
      if ( defined $opt_n ) {
         $poiObject->allowNoPosInDb();
      }
      
      # Read POI Object from file
      my $nextPos = $filePos;
      if ( $typeOfPIF eq "readCPIF" ) {
         $nextPos = $poiObject->readFromCPIF( $fileName, $filePos );
      } elsif ( $typeOfPIF eq "readSPIF" ) {
         $poiObject->setDefaultLang( $m_languageID );
         $nextPos = $poiObject->readFromSPIF( $fileName, $filePos );
      } elsif ( $typeOfPIF eq "readGDFPOIFormat" ) {
         # create POI
         $nextPos =
            $poiObject->readFromGDFPOIFormat( $fileName, $filePos );
         # POIs with type airline acces are wanted, but set deleted
         if ( $poiObject->hasPOIType(63) ) {
            $poiObject->setDeleted(1);
         }
      } else {
         errprint "generalReadPIF: Unknown POI file format '$typeOfPIF'";
         die "exit";
      }

      
      if ( $nextPos == $filePos ) {
         # We had reached eof, no POI was read from PIF
         $cont = 0;
      }
      else {
         # We read a POI from PIF
         $nbrPOIsInFile += 1;
         if ( ($nbrPOIsInFile%25000) == 0 ) {
            timeprint "generalReadPIF: read $nbrPOIsInFile pois (" .
                      scalar(keys(%allPOIs)) . " in hash)";
         }

         # Some POIs are not wanted in WASP
         # e.g. never add POIs with poiType entry point
         my $wantThisPOI = 1;
         if ( $poiObject->hasPOIType(74) ) {
            $wantThisPOI = 0;
            $nbrPOIsNotAddedDueToEP += 1;
            print " unwantedPOI entry point poi " .
                  $poiObject->getSourceReference() .
                  " is not added\n";
         }

         # Special things for gdf pois
         elsif ( $typeOfPIF eq "readGDFPOIFormat" ) {

            # noname poi = poi with no names that are not syonnyms
            if ($poiObject->getNbrNamesNonSynonyms() == 0 ) {
               # SOLUTION TO INCLUDE SOME NONAME GDF POIS
               # Add a default official name
               if ( $poiObject->hasPOIType(3) ) {
                  # ATM, ok
                  $poiObject->addName( "ATM", 0, 0);
               } else {
                  $wantThisPOI = 0;
                  print " unwantedPOI without names " .
                        $poiObject->getSourceReference() .
                        " is not added\n";
               }
            }
            # artificial city centres
            elsif ( $poiObject->hasPOIType(11) and
                    $poiObject->isArtificial() ) {
               $wantThisPOI = 0;
               print " unwantedPOI artificial cc " .
                     $poiObject->getSourceReference() .
                     " is not added\n";
            }

         }
         elsif ( $typeOfPIF eq "readCPIF" ) {
            # noname poi = poi with no names that are not syonnyms
            # should not be included unless we can add a default name to them
            if ($poiObject->getNbrNamesNonSynonyms() == 0 ) {
               $wantThisPOI = 0;
               print " unwantedPOI without names " .
                     $poiObject->getSourceReference() .
                     " is not added\n";
            }
         }
         elsif ($poiObject->getNbrNamesNonSynonyms() == 0 ) {
            errprint "POI without name - do we want it?\n";
            die "die";
         }
         
         # Convert the coordinate.
         if (defined $opt_C ){
             (my $lat, my $lon) = 
                 split(",", $poiObject->getSymbolCoordinate());
             if ( $opt_C eq "ta_wgs84" ){
                 # Fix the decimal point.
                 #$lat =~ s/^(.*)(.{7})$/$1\.$2/;
                 #   $lon =~ s/^(.*)(.{7})$/$1\.$2/;
                 $lat = $lat / 10000000;
                 $lon = $lon / 10000000;
                 #dbgprint "lat: $lat lon: $lon";
             }
             if ( ( $opt_C eq "ta_wgs84" ) || ( $opt_C eq "wgs84" ) ){
                 ( my $mc2Lat, 
                   my $mc2Lon ) = convertFromWgs84ToMc2( $lat,
                                                         $lon );
                 $poiObject->setSymbolCoordinate($mc2Lat, $mc2Lon);
                    #dbgprint $poiObject->getSymbolCoordinate();
             }
         }
      
         if ( ! $wantThisPOI ) {
            # don't add this POI
            $nbrPOIsNotAdded += 1;
         }
         else {
            # store the POI in hash, before add the POI to WASP db
            # - add admin info
            $poiObject = addAdminInfoFromInparams( $poiObject );
            # - set supplier?
            if ( $m_setSupplier ) {
              $poiObject->addPOIInfo( 49, 0, "$opt_u" );
            }
            # set validToVersion = validFromVersion for map data POIs
            if ( $opt_m ) { # poi is map data poi
               my $vf = $poiObject->getValidFromVersion();
               $poiObject->setValidToVersion($vf);
               #dbgprint "Is Map Data POI. Setting validToVersion to $vf";
            } else {
               # if not map data POI, should set validToVersion to NULL
               if ( defined( $poiObject->getValidToVersion() ) )  {
                  dbgprint "This is not a map data POI.\nvalidToVersion should not be set.\n";
                  dbgprint "It should be set to NULL\n";
               }
               dbgprint "Correct setting of validToVersion to NULL\n";
            }
            # - insert into hash
            my $srcRef = $poiObject->getSourceReference();
            $allPOIs{ $srcRef } = $poiObject;
            $nbrParsedInBatch++;
            
         }

         # update filePos to read next POI
         $filePos = $nextPos;
         
         
         
         
         if ( $nbrParsedInBatch == 1000000 ){
            # Sanity check
            dbgprint "generalReadPIF: nbrParsedInBatch:$nbrParsedInBatch, ".
               "scalar(keys(allPOIs)):". scalar(keys(%allPOIs));
            
            # Add the POIs of the hash to WASP, if not one of these
            #  - parseOnly
            #  - converting SPIF to CPIF
            if ( $m_parseOnly and ! $m_writeCPIF ) {
               $nbrPOIsNotAdded += scalar(keys(%allPOIs));
               $nbrParsed+=scalar(keys(%allPOIs));
            }
            else {
               # Add the POIs to WASP every 1 000 000th parsed POI.
               dbgprint "Will add " . scalar(keys(%allPOIs)) .
                        " POIs to WASP";
               my $addedFromBatch += storePOIs($dbh, \%allPOIs,
                     0,   # add synonyms (obsolete)
                     !$m_parseOnly, # add to WASP.
                     $cpifFile,     # cpif file
                     $m_categoriesObj, # category object
                     );
               $nbrPOIsAddedToWASP+=$addedFromBatch;
               $nbrParsed+=scalar(keys(%allPOIs));
               dbgprint "generalReadPIF: $addedFromBatch of ". 
                  scalar(keys(%allPOIs)) . 
                  " POIs added to WASP in this batch.";
            }
            # Reset batch variables.
            %allPOIs = ();
            $nbrParsedInBatch = 0;
         }
      }
   }


   # Add POIs of the last batch.
   dbgprint "generalReadPIF: Last batch";
   # Sanity check
   dbgprint "generalReadPIF: nbrParsedInBatch:$nbrParsedInBatch, ".
      "scalar(keys(allPOIs)):". scalar(keys(%allPOIs));
   # Add the POIs of the hash to WASP, if not one of these
   #  - parseOnly
   #  - converting SPIF to CPIF
   if ( $m_parseOnly and ! $m_writeCPIF ) {
      $nbrPOIsNotAdded += scalar(keys(%allPOIs));
      $nbrParsed+=scalar(keys(%allPOIs));
   }
   else {
      # Add the last POIs to WASP
      dbgprint "Will add " . scalar(keys(%allPOIs)) . " POIs to WASP";
      my $addedFromBatch += storePOIs($dbh, \%allPOIs,
            0,   # add synonyms (obsolete)
            !$m_parseOnly, # add to WASP.
            $cpifFile,     # cpif file
            $m_categoriesObj, # category object
            );
      $nbrPOIsAddedToWASP+=$addedFromBatch;
      $nbrParsed+=scalar(keys(%allPOIs));
      dbgprint "generalReadPIF: $addedFromBatch of ". 
         scalar(keys(%allPOIs)) . " POIs added to WASP in this batch.";
   } 


   #Print total status.
   dbgprint "generalReadPIF: Read $nbrPOIsInFile pois from POI file";
   dbgprint "generalReadPIF: $nbrPOIsAddedToWASP of ". 
      "$nbrParsed POIs added to WASP";
   
   if ( $m_parseOnly and ! $m_writeCPIF ) {
      dbgprint "generalReadPIF: parse-only return";
   }

   return $nbrPOIsAddedToWASP, $nbrPOIsNotAdded, $nbrPOIsNotAddedDueToEP;
} # generalReadPIF



# Add admin info etc to a POI object, using the given inparams
sub addAdminInfoFromInparams {
   my $poiObject = $_[0];

   if ( !defined( $poiObject->getSource() ) )  {
      if ( defined($m_sourceID) ) {
         $poiObject->setSource( $m_sourceID );
      } else {
         errprint "POI source not defined for poi src ref:", 
                  $poiObject->getSourceReference();
         die "exit";
      }
   }

   if ( !defined( $poiObject->getCountry() ) )  {
      if ( defined($m_countryID) ) {
         $poiObject->setCountry( $m_countryID );
      } else {
         errprint "POI country not defined";
         die "exit";
      }
   }
   my $countryID = $poiObject->getCountry();

   if ( !defined( $poiObject->getValidFromVersion() ) )  {
      if ( scalar(keys(%m_mapVerByCountryID)) > 0 ){
         my $edVerID = 
            mapVersionToEDVersionID( $dbh, 
                  $m_mapVerByCountryID{$countryID} );
         $poiObject->setValidFromVersion( $edVerID );
      }
      elsif ( defined($m_validFromID) ) {
         $poiObject->setValidFromVersion( $m_validFromID );
      } 
      else {
         errprint "POI fromVersion not defined for POI with srcRef:" . 
            $poiObject->getSourceReference();
         die "exit";
      }
   }

   if ( !defined( $poiObject->getValidToVersion() ) )  {
      if ( scalar(keys(%m_mapVerByCountryID)) > 0 ){
         my $edVerID = 
            mapVersionToEDVersionID( $dbh, 
                  $m_mapVerByCountryID{$countryID} );
         $poiObject->setValidToVersion( $edVerID );
      }
      elsif ( defined($m_validToID) ) {
         $poiObject->setValidToVersion( $m_validToID );
      }
   }

   if ( !defined( $poiObject->getInUse() ) ) {
      if ( defined($opt_i) ) {
         $poiObject->setInUse($opt_i);
      } else {
         errprint "POI inUse not defined for POI with srcRef:" .
            $poiObject->getSourceReference();
         die "exit";
      }
   }
   return $poiObject;
}





# When we get a SPIF in midmif some modifications must be done to
# get a text file according to Wayfinder SPIF specification.
sub trimMidMifSPIF {
   dbgprint "trimMidMifSPIF";

   my $midSepChar = ',';
   my $midRplChar = '}';

   # Open outfile
   my $fileName = "wfSPIF.txt";
   open(WF_FILE, ">$fileName");
   
   my $inFile = $ARGV[0];
   open FILE, $inFile or die "trimMidMifSPIF: cannot open file: $inFile\n";
   
   my $nbrPOIs = 0;
   while (<FILE>) {
      chomp;
      if (length() > 2) {

         # Split the midRow, handling $midSepChar within strings
         # also removing "-chars from string values
         my @poiRecord = splitOneMidRow( $midSepChar, $midRplChar, $_ );
         $nbrPOIs += 1;

         # Print the records to wayfinder SPIF semicolon delimited
         my $i = 0;
         foreach my $r (@poiRecord) {
            if ($i > 0) {
               print WF_FILE ";";
            }
            print WF_FILE "$r";
            $i += 1;
         }
         print WF_FILE "\n";

         print " row $nbrPOIs nbrFields $i\n";
      }
   }

   dbgprint "Read $nbrPOIs POIs from mid-spif and wrote to SPIF";
}


sub spifFromXlsCsv {
   dbgprint "Create SPIF file 'SPIF.txt' from xls-csv $ARGV[0]\n";

   # open outfile
   open (OUT, ">SPIF.txt") or die "Cannot open file SPIF.txt";

   # first record will have this HARDCODED srcRef+rowNbr
   my $startWithSrcRef = 100;
   
   # read infile
   my $nbrRows = 0;
   while (<>) {
      chomp;
      $nbrRows += 1;
      my @record = splitOneMidRow( ";", ";", $_ );
      
      # handle srcRef (add rowNbr if no srcRef)
      my $srcRef = $record[0];
      if ( length($record[0]) < 1 )  {
         $srcRef = $. + $startWithSrcRef;
      }
      print OUT $srcRef;

      # Handle the name
      # example of clean-up:
      # "Eurotel, Brno, Masarykova 12" -> "Eurotel Brno Masarykova 12"
      my $name = $record[1];
      $name =~ s/,//g;
      print OUT ";" . $name;

      
      # handle the other attributes
      my $nbr = scalar(@record);
      my $i = 2; # start at attribute 2
      while ( $i < ($nbr) ) {
         my $r = $record[$i];
         $r =~ s/^\s//g;
         $r =~ s/\s+$//g;
            
         print OUT ";" . $r;
         
         $i += 1;
      }
      print OUT "\n";
   }
   
   dbgprint "Read $nbrRows rows\n";
}



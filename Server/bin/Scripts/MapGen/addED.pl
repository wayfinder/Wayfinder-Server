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


# Use perl modules from here or BASEGENFILESPATH/script/perllib
use lib ".";
use PerlWASPTools;



use DBI;
use Getopt::Std;

use vars qw( $opt_c $opt_f $opt_g $opt_h $opt_i $opt_p $opt_r 
             $opt_t $opt_u $opt_v);
use vars qw( $m_edInsertTypeID $m_parseOnly $m_edFieldSep );

getopts('hpvc:r:t:f:g:i:u:');

if (defined $opt_h) {
   print <<EOM;
Operates on extradata in database, by e.g. adding or extracting ed.
The ed field separator are hardcoded in this script.
Connects to WASP database with db_connect from PerlWASPTools.
Usage: $0 [ options ] [ files ]

Options:
-h          Display this help.
-p          Parse only. Just check format of indata. Nothing is added to db.
-v          Print available EDVersions from db (for -r and -t options).

-c string   country
               For adding ED, it is only needed if no country is given in 
               the comment (ex sweden). Alsof or extracting ED.
-r string   fromVersion
               The map version extradata is valid from (ex TeleAtlas_2002_2)
               if not given in the extradata record comment.
-t string   toVersion
               The map version extradata is valid to (ex TeleAtlas_2003_1)
               if not given in the extradata record comment.
-g int      groupID
               A group ID for all the extradata in the file, only nessesary 
               if the extradata need to be found toghether.
-i string   insertType
               The extradata insert type, possible types:
                  beforeInternalConnections
                  beforeGenerateStreets
                  beforeGenerateTurndescriptions
                  afterGenerateTurndescriptions
                  dynamicExtradata
-u string   updateStatus 'use' 'nouse'
               Used when running updateValidToVersion function on result
               files from auto extradata validity checks. If the ed file 
               includes extradata records that are to be USED in
               the new map release give 'use' and toVersion=newVersion, 
               if NOTUSED give 'nouse' and toVersion=oldVersion.

-f string   The format of the data to process. Valid values are:
       addED (fromVersion, (toVersion), (groupID), (country), (insertType))
            Adds ed to db reading an extradata file created with MapEditor.
            Run dos2unix before adding.  Combine with -p for parse only.
       extractED (country, fromVersion, toVersion, (insertType))
            Extract ed for one country from db, used for ed validity check.
            If insertType is given: extracts ed with this type only, else
            extracts all ed with normal insertTypes (not dynamicExtradata).
       updateValidToVersion (toVersion, updateStatus)
            Update validToVersion for records after a validity check.
            Combine with -p for parse only.
       addNewGroup
            Create and add a new group to db. Give group name in utf-8 
            char encoding within quotes. Combine with -p for parse only.
EOM
  exit 1;
}

use Time::localtime;
sub dateAndTime (){
   my $sec = sprintf( "%02d", localtime->sec() );
   my $min = sprintf( "%02d", localtime->min() );
   my $hour = sprintf( "%02d", localtime->hour() );
   
   my $day = sprintf( "%02d", localtime->mday() );
   my $mon = sprintf( "%02d", localtime->mon()+1 );
   my $year = localtime->year() + 1900;
   
   $year . $mon . $day . " " . $hour . ":" . $min . ":" . $sec;
}
sub dbgprint($){ 
   print STDERR dateAndTime() . " DEBUG: ", $_[0], "\n";
}


# Connect to database
my $dbh = db_connect();

if (!defined $dbh) {
   die "Cannot connect to the database ($DBI::errstr)\n";
}

# ED field separator, 
# the inverted exclamation mark within less-than and greater-than signs
$m_edFieldSep = "<¡>";


# Print available ED versions and exit.
if ( defined($opt_v) ) {
   my $sth = $dbh->prepare("SELECT ID, version FROM EDVersion order by ID;");
   $sth->execute;
   print "ID\tversion\n-------------------------\n";
   while ( (my $ID, my $version) = $sth->fetchrow()) {
      print "$ID\t$version\n";
   }
   $sth->finish;
   db_disconnect($dbh);
   exit;
}


# Set parse only flag
$m_parseOnly = 0;
if ( defined($opt_p) ) {
   $m_parseOnly = 1;
   print "Parse only\n";
}

# Check that any given option is valid
if ( defined($opt_i) ) {
   $m_edInsertTypeID = getInsertTypeID($opt_i);
   if ( !defined($m_edInsertTypeID) ) {
      die "Error: Incorrect ed insert type given ($opt_i)\n";
   }
}
if (defined($opt_r)) {
   my $fromVersionID = getVersionID($opt_r);
   if ( !defined($fromVersionID) ) {
      die "Error: Incorrect valid from version given ($opt_r)\n";
   }
}
if (defined($opt_t)) {
   my $toVersionID = getVersionID($opt_t);
   if ( !defined($toVersionID) ) {
      die "Error: Incorrect valid to version given ($opt_t)\n";
   }
}
if (defined $opt_c) {
   my $countryID = getCountryID($opt_c);
   if ( !defined($countryID) ) {
      die "Error: Incorrect country given ($opt_c)\n";
   }
}

if (defined $opt_u) {
   if ( ! ( $opt_u eq "use" || $opt_u eq "nouse" ) ) {
      die "Error: Incorrect updateStatus given ($opt_u)\n";
   }
}

if (defined($opt_f)){
   if ($opt_f eq "addED") {
      handleAddED();
   } elsif ($opt_f eq "extractED") {
      handleExtractED();
   } elsif ($opt_f eq "updateValidToVersion") {
      handleUpdateValidToVersion();
   } elsif ($opt_f eq "addNewGroup") {
      handleAddNewGroup( $ARGV[0] );
   } else {
       db_disconnect($dbh);

       die "Error Unknown format: $opt_f\n";
   }
} else {
   db_disconnect($dbh);
   die "Error: No format specified!\n";
}

db_disconnect($dbh);

exit;

# This action adds extradata to WASP from files with extradata records
sub handleAddED {

   print "Handles add extradata\n";
   if ( !defined( $opt_r ) ) {
      die " From version not given - exits\n";
   }
   
   my $primGroup = "";
   my $nextGroupID = "";
   my $groupID = 0;
   my $insertTypeID;
   my $group = 0;
   my $validToVersionID;
   my $validFromVersionID;
   my $countryID;
   
   my $sourceQuery;
   my $writerQuery;
   my $latQuery;
   my $lonQuery;
   my $TARefQuery;
   my $commentQuery;
   my $validToVersionQuery;
   my $orgValueQuery;
   my $mapIDQuery;
   
   my $nbrRecordsInFile = 0;
   while (<>) {
      if (length()>2){
         #Find out if line is a comment or a extradatarecord
         (my $crap1, my @craprecord) = split ($m_edFieldSep);
         (my $crap3, my $crap4) = split(/ /, $crap1, 2);
         
         #Line is a comment, get values
         if ($crap3 eq "#"){
             use vars qw($crap1 $writer $source $comment $orgValue $TARef 
                         $crapID $insertTypeName $mapRelease $group 
                         $countryName $mapID $crap2);
            ($crap1, $writer, $source, $comment, $orgValue, $TARef,
             $crapID, $insertTypeName, $mapRelease, $group,
             $countryName, $mapID, $crap2) = split ($m_edFieldSep);
            if ( ! defined($mapID) ) {
               print "comment record is crap for ed file line $." .
                     " ed record " . $nbrRecordsInFile+1 . "\n";
               die "exit - check your ed file!!!!";
            }
         }
         
         #Line is an extradatarecord
         else {
            $nbrRecordsInFile += 1;
            
            chomp;
            my $edRecord = $_;

            #Get groupID
            if (defined $opt_g) {
               $groupID = $opt_g;
            } else {
               if ( $group eq "" ) {
                  # no group
                  $groupID = 0;
               } elsif (($group >= 1) && ($group <= 100)) {
                  # new group
                  if ($primGroup ne $group) {
                     $primGroup=$group;
                     $nextGroupID=getNextGroupID();
                  }
                  $groupID=$nextGroupID;
               } elsif ($group > 100) {
                  # existsing group
                  my $groupName= checkGroupID($group);
                  if (defined($groupName)) {
                     $groupID =$group;
                  } else {
                     print "ERROR:No valid group\n";
                  }
               } else {
                  # given group 0
                  $groupID =$group;
               }
            }
            
            #Get mapVersion
            if (defined($mapRelease)) {
               $validFromVersionID = getVersionID($mapRelease)
            }
            if (defined($opt_r)) {
               $validFromVersionID = getVersionID($opt_r);
            }
            if (defined($opt_t)) {
               $validToVersionID = getVersionID($opt_t);
            }
            
            # Split the extradata record to extract e.g. ed type id,
            # identification coordinate, insertType etc
            (my $edType, my $objectType, my @crap ) = 
                  split ($m_edFieldSep, $edRecord);
            
            #Get values for identifying lat, lon 
            (my $lat, my $lon) = 
                  split (",", getIdentyfyingCoord($edRecord, $edType));
            
            # Get value for ed insertType
            # 1: given with opt_i, 2: from record comment 3: default type
            if ( defined($m_edInsertTypeID) ) {
               $insertTypeID = $m_edInsertTypeID;
            }
            elsif ( defined($insertTypeName) && 
                    (length($insertTypeName) > 0) ) {
               $insertTypeID = getInsertTypeID($insertTypeName);
               # Make some kind of validity check of the insert type
               # Update if it was not correct in the ed records comment
               my $checkInsertTypeID = 
                  checkInsertTypeValidity($insertTypeID, $edType, $objectType);
               if ( ($checkInsertTypeID != -1) &&
                    ($checkInsertTypeID != $insertTypeID ) ) {
                  print "ERROR: insertType for extradata record " .
                        " $nbrRecordsInFile is not correct!" .
                        " - changing $insertTypeID to $checkInsertTypeID\n";
                  $insertTypeID = $checkInsertTypeID;
               }
            }
            else {
               $insertTypeID = getDefaultInsertType($edType, $objectType);
            }
            
            #Get value for EDType
            my $edTypeID = getEDTypeID($edType);
            
            #Get value for country
            if ( defined($countryName) && (length($countryName) > 0) ) {
               $countryID = getCountryID($countryName);
            } elsif (defined $opt_c) {
               $countryID = getCountryID($opt_c);
            }
            
            #Check that all nessesary data exist
            if ( (defined $edTypeID) && (defined $countryID) && 
                 (defined $edRecord) && (defined $insertTypeID) && 
                 (defined $validFromVersionID) ) {
               
               my $qedRecord = $dbh->quote($edRecord);
               print "edTypeID=$edTypeID, countryID=$countryID, " .
                     "insertTypeID=$insertTypeID, " .
                     "validFromVersionID=$validFromVersionID\n";
               print "edRecord=$qedRecord\n";
               
               #Create queries for values that might be inserted
               $sourceQuery = "";
               $writerQuery = "";
               $latQuery = "";
               $lonQuery = "";
               $TARefQuery = "";
               $commentQuery = "";
               $validToVersionQuery = "";
               $orgValueQuery = "";
               $mapIDQuery = "";
               if ( defined($source) && ($source ne "") ) {
                  my $qsource = $dbh->quote($source);
                  $sourceQuery = ", source=$qsource";
                  print "source=$qsource ";
               }
               if ( defined($writer) && ($writer ne "") ) {
                  my $qwriter = $dbh->quote($writer);
                  $writerQuery = ", writer=$qwriter";
                  print "writer=$qwriter ";
               }
               if ( defined($lat) ) {
                  $latQuery = ", lat=$lat";
               }
               if ( defined($lon) ) {
                  $lonQuery = ", lon=$lon";
               }
               if ( defined($TARef) && ($TARef ne "") ) {
                  my $qTARef = $dbh->quote($TARef);
                  $TARefQuery = ", TAreference=$qTARef";
                  print "TAreference=$qTARef\n";
               }
               if ( defined($comment) && ($comment ne "") ) {
                  my $qcomment = $dbh->quote($comment);
                  $commentQuery = ", comment=$qcomment";
                  print "comment=$qcomment ";
               }
               if (defined($validToVersionID)) {
                  $validToVersionQuery = ", validToVersion=$validToVersionID";
                  print "validToVersion=$validToVersionID ";
               }
               if ( defined($orgValue) && ($orgValue ne "") ) {
                  my $qorgValue = $dbh->quote($orgValue);
                  $orgValueQuery = ", orgValue=$qorgValue";
                  print "orgValue=$qorgValue ";
               }
               if ( defined($mapID) && ($mapID ne "") ) {
                  $mapIDQuery = ", mapID=$mapID";
                  print "mapID=$mapID ";
               }
               print "\n";
               
               #Insert values into database    
               my $query = "INSERT INTO EDMain SET " .
                           "edType=$edTypeID" .
                           "$sourceQuery" .
                           "$writerQuery" .
                           ", added=NOW()" .
                           ", lastModified=NOW()" .
                           "$latQuery" .
                           "$lonQuery".
                           ", active=1" .
                           "$TARefQuery" .
                           ", country=$countryID" .
                           ", edRecord=$qedRecord" .
                           ", insertType=$insertTypeID" .
                           "$commentQuery" .
                           ", validFromVersion=$validFromVersionID" .
                           "$validToVersionQuery" .
                           "$orgValueQuery" .
                           ", groupID=$groupID" .
                           "$mapIDQuery;";

               if ( $m_parseOnly ) {
                  print "query=\"$query\"\n";
                  print " parse only: extradata not added\n";
               } else {
                  $dbh->do($query);
                  my $ID = $dbh->{'mysql_insertid'};
                  print "ID=$ID\n";
               }
               print "\n";
            } else {
               print "ERROR Country, fromversion or other information is " .
                     "incorrect - extradata line $. not added to database.\n";
            }
   
            #Reset some values (needed if extradata with comment is 
            #mixed with extradata without comment)
            $writer="";
            $source="";
            $comment="";
            $orgValue="";
            undef $lat;
            undef $lon;
            $TARef="";
            undef $validFromVersionID;
            undef $validToVersionID;
            undef $groupID;
            $mapID="";
            undef $countryName;
            undef $countryID;

         }
      }
   }
   dbgprint "Added $nbrRecordsInFile records to WASP";
}

# This action extracts extradata from WASP as extradata records
# Used when new releases needs to be tested. Takes country, validFromVersion
# and validToVersion as inparam.
sub handleExtractED {
   print "Handling Extraction of extradata\n";
   
   # Check that options are set
   if ( !defined($opt_r) or !defined($opt_t) or !defined($opt_c) ) {
      die "Country, fromVersion or toVersion option not given!\n";
   }
   my $validFromVersionID;
   my $validToVersionID;
   my $countryID;
   if (defined $opt_r) {
      $validFromVersionID = getVersionID($opt_r);}
   if (defined $opt_t) {
      $validToVersionID = getVersionID($opt_t);}
   if (defined $opt_c) {
      $countryID = getCountryID($opt_c);}
   if ((!defined $validFromVersionID) || (!defined $validToVersionID)  
      || (!defined $countryID)){
      die "Country, fromVersion or toVersion incorrect\n";
   }

   # Check if any specific insert type was given
   my $insertTypeSubQuery = "";
   my $fileNameAppend = "";
   if ( defined($m_edInsertTypeID) ) {
      $insertTypeSubQuery = "(insertType = $m_edInsertTypeID) and ";
      $fileNameAppend = "_it$m_edInsertTypeID";
   }
   
   # Decide file name
   my $fileName = "EDList_$countryID${fileNameAppend}.txt";
   print "Extract ed to file $fileName\n";
   # We want generalExtractED to print the select query to stdout.
   my $printQuery = 1;
   # Open file here just to empty any old file, because it is opened 
   # with append in generalExtractED below.
   open(LIST, ">$fileName");
   
   my $nbrRecords = 0;
   
   # Create where query 
   
   # If no specfic insertType given, we don't want ed with dynamicExtradata
   my $dynamicSubQuery = "";
   if ( ! defined($m_edInsertTypeID) ) {
      $dynamicSubQuery = "(insertType != 5) and ";
   }



   # Don't include records from other suppliers than the ones impled 
   # by the validFrom and validTo. In this context, the supplier
   # is the string that is beore the first '_' in the version string
   # TeleAtlas_2006_07 -> TeleAtlas
   (my $fromSupplier, my @crap) = split ("_", $opt_r );
   (my $toSupplier, @crap) = split ("_", $opt_t );
   my $validFromVersionSubQuery = "validFromVersion<=$validToVersionID";
   if ( $fromSupplier and $toSupplier ) {
      if ( $fromSupplier eq $toSupplier ) {
         # Limit the extracted records to be validFrom NULL or
         # validFrom one of the releases of this supplier 
         print "Only select records validFrom $fromSupplier\_%\n";
         my $query = "select id from EDVersion " .
                     "where version like '$fromSupplier\_%' order by id;";
         
         my $sth = $dbh->prepare($query);
         $sth->execute;
         
         my $versions = "";
         my $nbrVersions = 0;
         while ( (my $versionID) = $sth->fetchrow() ) {
            if ( $versionID <= $validFromVersionID ) {
               if ($nbrVersions > 0) {
                  $versions .= ", ";
               }
               $versions .= "$versionID";
               $nbrVersions += 1;
            }
         }
         print " valid from versions NULL or in '$versions'\n";
         if ( $nbrVersions > 0 ) {
            $validFromVersionSubQuery =
               "validFromVersion in ($versions)";
         }
      }
   }
   #print " validFromVersionSubQuery '$validFromVersionSubQuery'\n";

   my $whereQuery = "where country=$countryID and active=1 and " .
                    "$insertTypeSubQuery" .
                    "$dynamicSubQuery" .
                    "($validFromVersionSubQuery or " .
                    "validFromVersion is NULL) and " .
                    "(validToVersion>=$validFromVersionID or " .
                    "validToVersion is NULL) order by edType";
   
   $nbrRecords = 
      generalExtractED( $fileName, $whereQuery, $nbrRecords, $printQuery);
   

   print "Extracted $nbrRecords records from db to '$fileName'\n";
}


# General extract ED method, called from handleExtractED.
# It takes 4 inparams.
#  1 The filename of the file where to write the extradata (opened with
#    append mode).
#  2 The where query for selecting extradata from db (no ending semicolon!)
#  3 The number of records extracted (so far), which will be increased 
#    for each record extracted in this method.
#  4 If the select query should be printed to stdout or not.
# Outparam is number of records extracted after running this method.
sub generalExtractED {
   
   my $fileName = $_[0];
   my $whereQuery = $_[1];
   my $nbrRecords = $_[2];
   my $printQuery = $_[3];
   
   #Open file
   open(LIST, ">>$fileName");
   
   #reset all values
   my $added = "";
   my $writer = "";
   my $source = "";
   my $comment = "";
   my $orgValue = "";
   my $TAreference = "";
   my $ID = "";
   my $insertType = "";
   my $validFromVersion = "";
   my $validToVersion = "";
   my $fromMapVersion = "";
   my $toMapVersion = "";
   my $groupID = "";
   my $country = "";
   my $mapID = "";
   my $edRecord = "";
   my $validFromVersionID;
   my $validToVersionID;
   my $countryID;
   
   
   # Create select query
   my $selectQuery = "select added, writer, source, comment, orgValue, " .
                     "TAreference, ID, insertType, validFromVersion, " .
                     "validToVersion, groupID, country, mapID, edRecord " .
                     "from EDMain " .
                     "$whereQuery;";

   if ( $printQuery ) {
      print " $selectQuery\n";
   }

   my $sth = $dbh->prepare($selectQuery);
   $sth->execute;
   
   while ( ( ($added, $writer, $source, $comment, $orgValue, $TAreference,
              $ID, $insertType, $validFromVersion, $validToVersion, $groupID,
              $country, $mapID, $edRecord) = $sth->fetchrow()) ) {
  
      # Get Version names (from and to)
      if ($validFromVersion) {
         my $fromVersionQuery = "select version from EDVersion " .
                                "where ID=$validFromVersion;";
         my $sth2 = $dbh->prepare($fromVersionQuery);
         $sth2->execute;
         $fromMapVersion = $sth2->fetchrow();
      } else {
         # there is no specified validFromVersion ERROR
         $fromMapVersion = "";
      }
      if ($validToVersion) {
         my $toVersionQuery = "select version from EDVersion " .
                              "where ID=$validToVersion;";
         my $sth2b = $dbh->prepare($toVersionQuery);
         $sth2b->execute;
         $toMapVersion = $sth2b->fetchrow();
      } else {
         # there is no specified validToVersion (no limit)
         $toMapVersion = "";
      }
      
      # Get insertType name
      my $insertTypeQuery= "select insertTypeName from EDInsertTypes " .
                           "where ID=$insertType;";
      my $sth3 = $dbh->prepare($insertTypeQuery);
      $sth3->execute;
      $insertType = $sth3->fetchrow();

      # Get country name
      my $countryQuery= "select country from POICountries where ID=$country;";
      my $sth4 = $dbh->prepare($countryQuery);
      $sth4->execute;
      $country = $sth4->fetchrow();

      # Print extradata comment
      print LIST "# ";
      if ($added) { 
         print LIST "$added";
      }
      print LIST "$m_edFieldSep";
      if ($writer) { 
         print LIST "$writer";
      }
      print LIST "$m_edFieldSep";
      if ($source) {
         print LIST "$source";
      }
      print LIST "$m_edFieldSep";
      if ($comment) {
         print LIST "$comment";
      }
      print LIST "$m_edFieldSep";
      if ($orgValue) {
         print LIST "$orgValue";
      }
      print LIST "$m_edFieldSep";
      if ($TAreference) {
         print LIST "$TAreference";
      }
      print LIST "$m_edFieldSep$ID$m_edFieldSep";
      if ($insertType) {
         print LIST "$insertType";
      }
      print LIST "$m_edFieldSep";
      
      if ($fromMapVersion) {
         print LIST "$fromMapVersion-";
      } else {
         print LIST "no limit-";
      }
      if ($toMapVersion) {
         print LIST "$toMapVersion";
      } else {
         print LIST "no limit";
      }
      print LIST "$m_edFieldSep";
      
      if ($groupID) {
         print LIST "$groupID";
      }
      print LIST "$m_edFieldSep";
      if ($country) {
         print LIST "$country";
      }
      print LIST "$m_edFieldSep";
      if ( defined($mapID) && ($mapID ne "")) {
         print LIST "$mapID"; 
      }
      print LIST "$m_edFieldSep" . "EndOfRecord" . "$m_edFieldSep\n";
      
      # Print extradata record
      print LIST "$edRecord\n";
      print LIST "\n";
      
      # reset all values 
      my $added = "";
      my $writer = "";
      my $source = "";
      my $comment = "";
      my $orgValue = "";
      my $TAreference = "";
      my $ID = "";
      my $insertType = "";
      my $validFromVersion = "";
      my $validToVersion = "";
      my $fromMapVersion = "";
      my $toMapVersion = "";
      my $groupID = "";
      my $country = "";
      my $mapID = "";
      my $edRecord = "";

      $nbrRecords += 1;
   }

   return $nbrRecords;
}

# This function reads an extradata file and check if db validToVersion
# for the records in the file needs to be updated. Used when updating
# validToVersions in db after extradata validity checks for new map
# release.
# The option -t gives a validToVersion that according to extradata validity
# checks should apply for the records (-t = the new map release).
# The option -u tells if the records are needed in the -t version, or 
# if they are no longer needed.
sub handleUpdateValidToVersion
{
   if ( ! defined($opt_t) ) {
      die "Error handleUpdateValidToVersion no validToVersion given";
   }
   if ( ! defined($opt_u) ) {
      die "Error handleUpdateValidToVersion no updateStatus given";
   }

   if ( $opt_u eq "use" ) {
      print "Will update validToVersion to be $opt_t or later\n" .
            "  -> all records in infile that have validToVersion earlier " .
            "than $opt_t will be changed.\n";
   }
   if ( $opt_u eq "nouse" ) {
      print "Will update validToVersion to be $opt_t or earlier\n" .
            "  -> all records in infile that have validToVersion no limit " .
            "or later than $opt_t will be changed.\n";
   }

   # Read the extradata file, extract ed record id
   # Check if to update the validToVersion for that ed record

   my $givenValidToVersionID = getVersionID($opt_t);
   if ( ! defined($givenValidToVersionID) ) {
      die "Could not extract version id from given '$opt_t'\n";
   }

   my $nbrRecords = 0;
   my $nbrUpdates = 0;
   while (<>) {
      if (length()>2){
         #Find out if line is a comment or a extradatarecord
         (my $crap1, my @craprecord) = split($m_edFieldSep);
         (my $crap3, my $crap4) = split(/ /, $crap1, 2);
         
         #Line is a comment, get values
         if ($crap3 eq "#"){
            use vars qw($crap1 $writer $source $comment $orgValue $TARef 
                        $edRecID $insertTypeName $mapRelease $group 
                        $countryName $mapID $crap2);
            ($crap1, $writer, $source, $comment, $orgValue, $TARef,
             $edRecID, $insertTypeName, $mapRelease, $group,
             $countryName, $mapID, $crap2) = split($m_edFieldSep);
            
            # Ask database for ValidToVersion for this ed record with
            # ed record id edRecID, to find out if an update is needed.
            if ( defined($edRecID) && (length($edRecID) > 0) ) {
               $nbrRecords += 1;

               # Get database validToVersion for this ed record
               my $selectQuery = "SELECT validToVersion from EDMain " .
                                 "where id=$edRecID;";
               my $sth = $dbh->prepare($selectQuery);
               $sth->execute or die "handleUpdateValidToVersion";
               my $dbValidToVersionID = $sth->fetchrow();

               # DEBUG
               print "edRecID=$edRecID dbVersion=";
               if ( !defined($dbValidToVersionID) ) {
                  print "no-limit";
               } else {
                  print "$dbValidToVersionID";
               }
               print " givenVersion=$givenValidToVersionID\n";

               my $updateVersion = 0;
               if ( $opt_u eq "use" ) {
                  # make sure that the validToVersion for this ed record id 
                  # is at least as great as $opt_t
                  if ( defined($dbValidToVersionID) &&
                       ($dbValidToVersionID < $givenValidToVersionID) ) {
                     $updateVersion = 1;
                  }
                  
               }
               if ( $opt_u eq "nouse" ) {
                  # make sure that the validToVersion for this ed record id
                  # is NOT greater than $opt_t
                  if ( !defined($dbValidToVersionID) ||
                       ( $dbValidToVersionID > $givenValidToVersionID ) ) {
                     $updateVersion = 1;
                  }
               }

               # update the validToVersion for this ed record!
               if ( $updateVersion == 1 ) {
                  $nbrUpdates += 1;
                  my $updateQuery =
                        "UPDATE EDMain " .
                        "set validToVersion=$givenValidToVersionID, " .
                        "lastModified=NOW() " .
                        "where id=$edRecID;";
                  print " updateQuery=$updateQuery\n";
                  if ( $m_parseOnly ) {
                     print " parse only: no update in db\n";
                  } else {
                     $dbh->do($updateQuery);
                  }
               }
            }
            
         }
         
         # else Line is the ed record.. nothing to do
         
      }
   }
   print "handleUpdateValidToVersion: updated validToVersion " .
         "for $nbrUpdates of $nbrRecords ed records\n";

} # handleUpdateValidToVersion


# Create and add new group to database
sub handleAddNewGroup
{
   # Get group name (inparam), and do db quote
   my $groupName = $_[0];
   my $qgroupName = $dbh->quote($groupName);
   # Get next available group id
   my $nextID = getNextGroupID();
   
   print " will add $qgroupName as new group, groupid $nextID\n";
   
   # Create insert query
   my $insertQuery = "INSERT INTO EDGroups " .
                     "values ($nextID, $qgroupName);";
   print " insertQuery=\"$insertQuery\"\n";
   
   if ( $m_parseOnly ) {
      print " parse only: group not added\n";
   } else {
      $dbh->do( $insertQuery );
      print " group was added to db\n";
   }
}



sub getEDTypeID {
   my $edType = $_[0];

   my $selectQuery = "select ID from EDTypes where typeName LIKE '$edType';";
   my $sth = $dbh->prepare($selectQuery);
   $sth->execute;
   my $edTypeID = $sth->fetchrow(); 
   return $edTypeID;
}

sub getDefaultInsertType
{
   my $edType = $_[0]; 
   my $objectType = $_[1];
   
   my $insertTypeName = "";
   
   # before internal connections
   # (= before mixed post proc which is what e really want to achieve)
   if ( ($edType eq "setLevel") ||
        ($edType eq "setWaterType") ||
        (($edType eq "removeItem") && ($objectType eq "island")) 
      ) {
      $insertTypeName = "beforeInternalConnections";
   }
   
   #before generate street segments
   elsif ( ($edType eq "addNameToItem" ||
            $edType eq "AddNameToItem" ||
            $edType eq "UpdateName" || $edType eq "removeItem" ||
            $edType eq "removeNameFromItem") && 
           ($objectType eq "SSI" || $objectType eq "ssi" || 
            $objectType eq "streetsegment") ) {
      $insertTypeName = "beforeGenerateStreets";
   }
   
   #before generate turndescriptions
   elsif ($edType eq "setRamp" || $edType eq "setRoundabout" ||
          $edType eq "setMultidigitised" || $edType eq "setMultiDigitised" || 
          $edType eq "setEntryRestrictions" || 
          $edType eq "setVehicleRestrictions" || 
          $edType eq "setSpeedLimit" || $edType eq "setHousenumber" ||
          $edType eq "addSignpost" || $edType eq "addsinglelandmark" || 
          $edType eq "addlandmark" || $edType eq "removeSignpost" || 
          $edType eq "setRoundaboutish" ||
          $edType eq "setTollRoad" ||
          $edType eq "setControlledAccess" ) {
      $insertTypeName = "beforeGenerateTurndescriptions";
   }
   elsif ( ($edType eq "AddNameToItem" ||
            $edType eq "addNameToItem" ||
            $edType eq "UpdateName" || $edType eq "removeItem" || 
            $edType eq "removeNameFromItem") &&
           ($objectType ne "SSI" && $objectType ne "ssi" &&
            $objectType ne "streetsegment") ) {
      $insertTypeName = "beforeGenerateTurndescriptions";
   }
   
   #after generate turndescriptions
   elsif ( $edType eq "setTurnDirection" || $edType eq "addCityPart" ) {
      $insertTypeName = "afterGenerateTurndescriptions";
   }
   
   else {
      die "Could not extract insertTypeName for this" .
          " edType '$edType' and objectType '$objectType'\n";
   }
   
   #before internal connections && dynamic ed
   # - set value via ed record comment or directly in database
     
   my $insertTypeID = getInsertTypeID($insertTypeName);
   return $insertTypeID;
}

sub  getInsertTypeID {
   my $insertTypeName = $_[0];
   my $selectQuery = "select ID from EDInsertTypes where " .
                     "insertTypeName LIKE \"$insertTypeName\";";
   my $sth = $dbh->prepare($selectQuery);
   $sth->execute;
   my $insertTypeID = $sth->fetchrow();
   return $insertTypeID; 
}

sub checkInsertTypeValidity
{
   my $insertTypeID = $_[0];
   my $checkEDtype = $_[1];
   my $checkObjectType = $_[2];

   my $checkReturnId = -1;

   if ( ($checkEDtype eq "setTurnDirection") &&
        (($insertTypeID == 1) ||
         ($insertTypeID == 2) ||
         ($insertTypeID == 3)) ) {
      $checkReturnId = getDefaultInsertType($checkEDtype, $checkObjectType);
   }
   
   return $checkReturnId;
}

sub getIdentyfyingCoord {

   my $edRecord = $_[0];
   my $edType = $_[1];

   my $coord = "";
   my $lat = "NULL";
   my $lon = "NULL";

   #split extradataRecord into 8 pieces
   (my $crap1, my $coord2, my $crap3, my $coord4, my $crap5, my $coord6, 
    my $coord7, my $crap8) = split($m_edFieldSep, $edRecord, 8);
   
   #select coordinates from 2:a place in record
   if ($edType eq "addCityPart" || $edType eq "setRamp" || 
       $edType eq "setRoundabout" || $edType eq "setMultidigitised" ||
       $edType eq "setMultiDigitised" ||
       $edType eq "setEntryRestrictions" || $edType eq "setSpeedLimit" || 
       $edType eq "setLevel" || $edType eq "setHousenumber" ||
       $edType eq "setTollRoad" ||
       $edType eq "setRoundaboutish" || $edType eq "setControlledAccess"){
       $coord = $coord2;
   }
   #select coordinates from 4:a place in record
   elsif ( $edType eq "setTurnDirection" ||
             $edType eq "setVehicleRestrictions" || 
             $edType eq "addSignpost" ||  $edType eq "removeSignpost" ||
             $edType eq "removeNameFromItem" ||  $edType eq "removeItem" ||
             $edType eq "UpdateName" ||  
             $edType eq "AddNameToItem" || $edType eq "addNameToItem" ||
             $edType eq "setWaterType" ) {
      $coord = $coord4;
   }
   #select coordinates from 6:a place in record
   elsif ($edType eq "addlandmark"){
      $coord = $coord6;
   }
   #select coordinates from 7:a place in record
   elsif ($edType eq "addsinglelandmark"){
      $coord = $coord7;
   }
   else {
      die "getIdentyfyingCoord: fails for edType $edType record " .
          "'$edRecord'\n";
   }

   if ($coord ne ""){
      $coord =~ s/\(//;
      $coord =~ s/\)//;
      ($lat, $lon) = split(",", $coord);
   }
   return $lat . "," . $lon;
}

sub getVersionID {
   my $version = $_[0];
   my $selectQuery = "select ID from EDVersion where version LIKE '$version';";
   my $sth = $dbh->prepare($selectQuery);
   $sth->execute;
   my $versionID = $sth->fetchrow();
   return $versionID;
}

sub checkGroupID {
   my $groupID = $_[0];
   my $groupQuery="select groupName from EDGroups where ID=$groupID;";
   my $sth = $dbh->prepare($groupQuery);
   $sth->execute;
   my $groupName = $sth->fetchrow();
   return $groupName;
} 
sub  getNextGroupID {
  my $nextGroupQuery="select max(ID) from EDGroups;"; 
  my $sth = $dbh->prepare($nextGroupQuery);
  $sth->execute;
  my $maxGroupID = $sth->fetchrow();
  my $nextGroupID = ($maxGroupID + 1);
  return $nextGroupID;
}

sub getCountryID {
   my $countryName = $_[0];
   my $countryQuery=
      "select ID from POICountries where country like '$countryName';";
   my $sth = $dbh->prepare($countryQuery);
   $sth->execute;
   my $country = $sth->fetchrow();
   return $country;
}

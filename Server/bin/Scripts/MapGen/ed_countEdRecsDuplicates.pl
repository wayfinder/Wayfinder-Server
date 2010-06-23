#!/usr/bin/perl -w
#
#
# Count and print info about duplicate extra data records found in 
# file produced by extra data validity check.
#

use strict;
use Getopt::Std;
use vars qw( $opt_h );

getopts('h');

if (defined $opt_h) {
   print <<EOM;

Count and print info about duplicate extra data records found in
file produced by extra data validity check.
 
Usage: $0 [ validity-check file ]

Options:
-h    Display this help.

EOM
  exit 1;
}


my $edRec = 0;
my %nbrEdRecsInMap;

my %correction = ();
my %itemType = ();
my %EDRecsMaps = ();
my $nbrOfMaps;
my $storeRecordInfo = 0;

my $EDRecID; # Global variabel, as we use it both when processing comment and
             # record.

print "Processing ed file...\n";

while (<>) {
   chomp;
   if ( length($_) > 2 ) {
      my @record = split("<¡>", $_, -1);  #split each record (row) at <¡> (edSep)
      foreach my $tmp (@record) {
         $tmp =~ s/\s+$//g;            #remove any space-char at the end of rec
      }

      # if comment, 
      # - increase edRec count
      # - extract map id and store in hash
      if (m/^\s*#/) {
         $edRec += 1;
         my $mapID = $record [ 11 ];
         $EDRecID = $record [ 6 ];
         if ( ! exists $EDRecsMaps { $EDRecID } ) { # First time EDrecId 
            $nbrOfMaps = 0;
         } else {
            $nbrOfMaps = @ { $EDRecsMaps { $EDRecID } };
         }
            
         if ( length $mapID < 1 ) { 
            print "WARNING: No map defined for edRec $edRec ", $record[6], "\n";            
            $EDRecsMaps { $EDRecID } [ $nbrOfMaps ] = "-";
         } else {
            $EDRecsMaps { $EDRecID } [ $nbrOfMaps ] = $mapID;
         }
         
         $storeRecordInfo = 1; # 1 = true        
      }
      else { # record
         if ( $storeRecordInfo == 1) {
            $itemType { $EDRecID } = $record[1];
            $correction { $EDRecID } = $record[0];
            $storeRecordInfo = 0; # 0 = false
         }
      }
   }
}

print "\nRead $edRec records from ed file\n";

my $duplicateFound = 0;
foreach my $edRecId ( sort ( keys ( %itemType ) ) ) {
   my @maps = @ { $EDRecsMaps { $edRecId } };
   if ( (scalar @maps ) > 1 ) {
      if ( ! $duplicateFound ) {
         print "\nThe following ed records existed in multiples (undefined map id's\ ";
         print "are written with a '-'):\n";
         print "<ED record id>, <operation>, <item info>, <map id 1>, <map id 2>, ...\n\n";
         $duplicateFound = 1;
      }
      print $edRecId."   ";
      print "   ";
      print $correction { $edRecId }."   ";
      print "   ";
      print $itemType { $edRecId }."    ";
      print "   ";

      foreach my $mapId ( sort ( @maps ) ) {
         print $mapId . "   ";
      } 
      print "\n";
   }
}

if ( ! $duplicateFound ) {
   print "No multiple ed records were found.\n";
}

print "\n";


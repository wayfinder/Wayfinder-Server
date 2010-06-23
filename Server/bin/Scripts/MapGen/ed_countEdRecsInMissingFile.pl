#!/usr/bin/perl -w
#
#
# Count how many extra data records there are in each mcm map in 
# the missing-file from the extra data validity check.
#

use strict;
use Getopt::Std;
use vars qw( $opt_h );

getopts('h');

if (defined $opt_h) {
   print <<EOM;

Count how many extra data records there are in each mcm map in 
the missing-file from the extra data validity check.
Usage: $0 [ missing-file ]

Options:
-h    Display this help.

EOM
  exit 1;
}


my $edRec = 0;
my %nbrEdRecsInMap;

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
         my $mapID = $record[11];
         #print "record $edRec mapId '$mapID'\n";
         if ( length $mapID < 1 ) {
            print "No map defined for edRec $edRec ", $record[6], "\n";
         }

         if (  defined( $nbrEdRecsInMap{$mapID} ) ) {
            $nbrEdRecsInMap{$mapID} += 1;
         } else {
            $nbrEdRecsInMap{$mapID} = 1;
         }
      }

      # if record, nothing to do
      else { # record
      }
   }


}

print "Read $edRec records from ed file\n";

foreach my $mapID ( sort(keys(%nbrEdRecsInMap)) ) {
   print "In map $mapID nbr edRecords ", $nbrEdRecsInMap{$mapID}, "\n";
}



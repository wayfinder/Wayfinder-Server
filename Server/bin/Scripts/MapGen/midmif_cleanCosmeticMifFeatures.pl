#!/usr/bin/perl -w
#
#  Clean out 'cosmetic' mif features such as PEN, BRUSH etc. 
#  To prepare for conversion using ESRI MIFSHAPE Utility.
#
#  Usage:  midmif_cleanCosmeticMifFeatures.pl miffile
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
use vars qw( $opt_h $opt_c $opt_f $opt_p );

getopts('hcfp');

if (defined $opt_h) {
   print <<EOM;
Usage: $0 [ options ] [ miffile ]

Options:
-h    Display this help.
-c    Clean the mif file from cosmetic mif fetures, such as PEN, BRUSH.
      See option -p which are handled. Prints result to cleanedMif.mif.
-f    Find cosmetic mif features in the mif file. Prints to stdout.
-p    Print the so far detected cosmetics.
EOM
  exit 1;
}


# Load file
#my @miffile = (<>);
#print "read " . scalar(@miffile) . " rows from file\n";

if ( defined($opt_f) ) {
   find();
} elsif ( defined($opt_c) ) {
   clean();
} elsif ( defined($opt_p) ) {
   printDetected();
}

exit;


# Clean out cosmetic features
sub clean {
   print "Clean miffile\n";

   open(OUT, ">cleanedMif.mif");

   my $nbrCleaned = 0;
   while (<>) {
      chomp;
      my $mifrow = $_;
      my $upperrow = uc $mifrow;

      if ( (index($upperrow, "PEN") < 0) and 
           (index($upperrow, "BRUSH") < 0) and
           (index($upperrow, "CENTER") < 0) ) {
         print OUT "$mifrow\n";
      }
      else{
         $nbrCleaned += 1;
      }
   }
   print "Cleaned $nbrCleaned features\n";
   
}

sub printDetected {
   print "So far handling:\n";
   print " BRUSH\n";
   print " PEN\n";
   print " CENTER\n";
}

# Find cosmetic features
sub find {
   print "Find cosmetic \n";

   # store cosmetics in a hash, key = cosmetic, value = nbr occurances;
   my %cosmetics = ();

   my $header = 0;
   while (<>) {
      chomp;

      if ( length > 2 ) {
         my $mifrow = uc $_;
         
         # after header: look for cosmetic features
         if ( $header ) {

            # get first character on the row
            my @row = split (" ", $mifrow);
            (my $first, my @crap) = split ( //, $row[0] );
            # if first is not a digit
            my $ordo = ord( $first );
            # ordo 0 = 48, ordo 9 = 57, ordo '-' = 45
            # no digit (typically a coord)
            if ( (($ordo < 48) or ($ordo > 57)) and ($ordo != 45) ) {
               if ( defined($cosmetics{$row[0]}) ) {
                  $cosmetics{$row[0]} += 1;
               } else {
                  $cosmetics{$row[0]} = 1;
               }
            }
         } else {
            # read header
            if ( index($mifrow, "DATA") != -1 ) {
               $header = 1;
            }
         }
      }
   }

   # print the cosmetics candidates
   print "Cosmetic candidates:\n";
   foreach my $key (keys(%cosmetics) ) {
      print " " . $cosmetics{$key} . "\t" . $key . "\n";
   }
}


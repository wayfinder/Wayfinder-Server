#!/usr/bin/perl -w
#

# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;


use strict;

use Getopt::Std;

use vars qw( $opt_h $opt_c $opt_d $opt_i $opt_g 
             $opt_e $opt_o $opt_I );

# Use perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
#use lib ".";
# Set lib assuming the script is located in genfiles/script
use FindBin '$Bin';
use lib "$Bin/perllib";

use PerlCountryCodes;


getopts('hcdgieoI');


if (defined $opt_h) {
   print <<EOM;
Script to get country strings and info for use in map generation binaries
and files. Uses PerlCountryCodes module.

Usage: $0 
       options

Options:
-h          Display this help.

-i  country
      Get iso alpha2-code from a single country name. Prints to std out.
-I  country
      Get database countryID for a single country name (as stored
      in POICountries table). Prints to std out.
      If the input country does not give a hit (e.g. it is the gms name)
      it tries to find the string table English name via the alpha-2 code,
      which should give the correct country name and thus find ID.
-c  alpha2-code 
      Get country directory name for a ISO 3166 alpha2 country code.
      The tail can be either a alpha-2 code or a file with alpha-2 codes.
      Prints to std out.
-g  alpha2-code
      Get gms name (in param to GenerateMapServer) for a ISO 3166 alpha2
      country code. Prints to std out.
-e  alpha2-code
      Get the database country name, the string table English name, which 
      is used as e.g. inparam to ExtradataExtractor,
      for a ISO 3166 alpha2 country code. Prints to std out.

-d  country / aplha2-code
      Get driving side for one country. Specify country with alpha2
      or with country name string

-o  country-list
      Get an ordered country list from an unordered list. Give the countries
      as a space separated list.

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


########################################################################
# Action

# Options that does not require a tail


# Options that requires somthing in tail
# get first in tail to check if it is a file or not
my $tail = $ARGV[0];
if ( ! $tail ) {
   die "nothing in tail - exit\n";
}

my $fileInTail = 0;
if ( open(FILE, "$tail") ) {
   $fileInTail = 1;
}

if ( defined( $opt_c ) ) {
   printCountry($tail);
}
if ( defined( $opt_g ) ) {
   printGmsName($tail);
}
if ( defined( $opt_i ) ) {
   printAlpha2Code($tail);
}
if ( defined( $opt_I ) ) {
   printCountryID($tail);
}
if ( defined( $opt_d ) ) {
   printDrivingSide($tail);
}
if ( defined( $opt_e ) ) {
   printStringTableEnglishNameName($tail);
}
if ( defined( $opt_o ) ) {
   printOrderedCountryList(@ARGV);
}

exit;


########################################################################
# Sub routines
sub printCountry {

   if ( $fileInTail ) {
      print "printCountry file in tail\n";
      while ( <FILE> ) {
         chomp;
         my $country = getCountryFromAlpha2($_);
         if ( $country ) {
            print "$country\n";
         }
      }
      return;
   }

   my $alpha2 = $_[0];
   my $retVal = getCountryFromAlpha2($alpha2);
   if ($retVal) {
      print "$retVal\n";
   }
}

sub printGmsName {
   my $alpha2 = $_[0];
   my $retVal = getGmsNameFromAlpha2($alpha2);
   if ($retVal) {
      print "$retVal\n";
   }
}

sub printAlpha2Code {
   my $country = $_[0];
   my $retVal = getAlpha2FromCountry($country);
   if ($retVal) {
      print "$retVal\n";
   }
}


sub printDrivingSide {
   my $side = getDrivingSide( $_[0] );
   if ( $side ) {
      print "$side\n";
   }
}

sub printStringTableEnglishNameName {
   my $alpha2 = $_[0];
   
   # get name from POICountries table
   use PerlWASPTools;
   use DBI;
   my $dbh = db_connect( "noPrint" );
   my $country = getCountryNameFromISO3166Code( $dbh, $alpha2 );
   if ( defined($country) ) {
      print "$country\n";
   }
   db_disconnect( $dbh );
}

sub printCountryID {
   my $country = $_[0];
   
   # get ID from POICountries table
   use PerlWASPTools;
   use DBI;
   my $dbh = db_connect( "noPrint" );
   my $retVal = getCountryIDFromName( $dbh, $country );
   if ( defined($retVal) ) {
      print "$retVal\n";
   } else {
      # the gms name and database name may differ
      # try this
      my $alpha2 = getAlpha2FromCountry($country);
      if (  $alpha2 ) {
         my $newCountryName = getCountryNameFromISO3166Code( $dbh, $alpha2 );
         if ( defined($newCountryName) ) {
            my $retVal = getCountryIDFromName( $dbh, $newCountryName );
            if ( defined($retVal) ) {
               print "$retVal\n";
            }
         }
      }
   }
   db_disconnect( $dbh );
}

sub printOrderedCountryList {
    print getCountryOrder(@_) . "\n";
    #($tail);

}

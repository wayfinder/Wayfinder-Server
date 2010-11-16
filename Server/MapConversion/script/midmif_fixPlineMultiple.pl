#!/usr/bin/perl -w
#
#  Fix midmif files that includes "Pline Multiple"
#
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



if ( scalar @ARGV != 2) {
   die(" Usage: $0 midfile miffile\n" .
       " Prints result in new midmif files\n" .
       " Handles only LINE PLINE POINT REGION\n");
}

if ( scalar @ARGV >= 1) {
   $MID_FILE_NAME = $ARGV[ 0 ];
   open MID_FILE, $MID_FILE_NAME
      or die "Can not open mid file $MID_FILE_NAME: $!\n";
}
if ( scalar @ARGV >= 2) {
   my $MIF_FILE_NAME = $ARGV[ 1 ];
   open MIF_FILE, $MIF_FILE_NAME
      or die "Can not open mif file $MIF_FILE_NAME: $!\n";
}


# Read mid, and store rows in a hash
my $feature = 0;
my %midRows = ();
while (<MID_FILE>) {
   chomp;
   $feature += 1;
   $midRows{ $feature } = $_;
}
print "Stored " . scalar(keys(%midRows)) . " mid rows\n";

# Open out files for printing the result
open(OUTMIF, ">newmif.mif");
open(OUTMID, ">newmid.mid");

$nbrMultiple = 0;
$nbrNewFeatures = 0;
$rows = 0;
$feature = 0;
while (<MIF_FILE>) {
   chomp;
   $rows += 1;
   my $mifRow = $_;
   
   # If we have a Pline Multiple, find out number of lines, write mid 
   # row that number of times and split mif feature into that many 
   # normal Plines
   if ((length() > 2) and index( uc($mifRow), "PLINE MULTIPLE") != -1) {
      $nbrMultiple += 1;
      $feature += 1;
      (my $crap1, my $crap2, $nbrLines) = split (" ");
      print " multiple $nbrMultiple $nbrLines\n";
      $nbrNewFeatures += ( $nbrLines - 1 );

      # Write mid row $nbrLines times
      $tmp = 0;
      while ( $tmp < $nbrLines ) {
         print OUTMID $midRows{ $feature } . "\n";
         $tmp += 1;
      }
      # Split mif feature into $nbrLines normal Plines
      $line = 0;
      while ( $line < $nbrLines ) {
         # Read next mif row which holds nbr of coords in each part of
         # the multiple line
         $mifRow = (<MIF_FILE>);
         @tmpSplit = split(" ", $mifRow, -1);
         $nbrCoords = 0;
         if ( defined($tmpSplit[0]) ) {
            $nbrCoords = $tmpSplit[0];
         } else {
            die("Couldn't get number coords in multiple, " .
                "feature = $feature\n");
         }
         # Print Pline marker to out file
         print OUTMIF "Pline  $nbrCoords\n";
         # Copy all coordinates in this part
         $coord = 0;
         while ( $coord < $nbrCoords ) {
            $mifRow = (<MIF_FILE>);
            print OUTMIF "$mifRow";
            $coord += 1;
         }
         # Goto next part of the multiple line
         $line += 1;
      }
   
   }
   else {
      
      # Mif row that does not contain "Pline Multiple"
      # Copy to out file
      print OUTMIF "$mifRow\n";

      # Decide if to write to mid file (= when we have a new mif feature)
      if ( (length() > 2) and
           (  (index( uc($mifRow), "LINE") != -1) or
              (index( uc($mifRow), "REGION") != -1) or
              (index( uc($mifRow), "POINT") != -1)  ) ) {
         $feature += 1;
         # print one mid row
         print OUTMID $midRows{ $feature } . "\n";
      }

      if ( (length() > 2) and
           (index( uc($mifRow), "COLLECTION") != -1) ) {
         print "ERROR: Mif feature 'Collection' in mif file - must fix that first!";
         die "exit";
      }
   
   }
}

print "\n=======================================================\n";
print "Read $feature features from mif\n" .
      "   fixed $nbrMultiple 'Pline Multiple\n" .
      "   created $nbrNewFeatures new features.\n";
print "Wrote result to files: newmid.mid and newmif.mif\n";
print "Make sure number of midRows matches the original file \n" .
      " - if not, perhaps we had unknown mif features..\n";

exit(0);


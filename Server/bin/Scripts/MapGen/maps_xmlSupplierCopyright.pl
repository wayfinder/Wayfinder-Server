#!/usr/bin/perl -w
#
# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;
use strict;
use Getopt::Long ':config',
    'no_auto_abbrev', # Impossible to use -s for --source.
    'no_ignore_case'  # Case sensitive options.
;    


# Print command line.
print STDERR "Command: $0";
foreach my $arg (@ARGV){
    print STDERR " $arg";
}
print STDERR "\n\n";



# Use perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;

use vars qw( $opt_help $opt_xml2box $opt_box2xml);
GetOptions('help|h',
           'xml2box',
           'box2xml');


sub extractXMLAttr {
    my $attributeName = shift;
    my $string = shift;
    chomp $string;
    
    if ( "$string" =~ /$attributeName/ ){
        $string =~ s/.*${attributeName}=["']//;
        $string =~ s/['"].*//;
        return $string;
    }
    else {
        return undef;
    }
}

##########################################################################
if (defined($opt_help)){
    print "Use this script to handle the xml files defining copyright \n";
    print "coverage of a map supplier.\n";
    print "\n";
    print "USAGE:\n";
    print "maps_xmlSupplierCopyright.pl OPTIONS INDATA\n";
    print "\n";
    print "OPTIONS:\n";
    print "-xml2box INDATA\n";
    print "Converts the xml file given as INDATA to the box format read by\n";
    print "the BTGPSSimulator java klient.\n";
    print "\n";
    print "-box2xml INDATA\n";
    print "Converts the box file given as INDATA to an xml file.\n";
    print "Comments last on each row, e.g.\n";
    print "[(725495615,46795596),(622659810,210394288)] main box supplier A\n";
    print "\n";
    print "\n";
    exit;
}
    
if (defined ($opt_box2xml)){
    
   dbgprint "Create supplier bbox xml indata\n";
   
   my $srcFile = $ARGV[0];
   open (SRCFILE, "<", $srcFile);
   
   # read infile
   my $nbrRows = 0;
   while (<SRCFILE>) {
      chomp;
      if (length() > 2) {
         $nbrRows += 1;
         
         (my $boxCoords, my $boxInfo) = split("]");
         if ( defined($boxCoords) and defined($boxInfo) ) {
            (my $nLat, my $wLon, my $sLat, my $eLon) = split(",", $boxCoords);
            my @boxInfos = split(" ", $boxInfo);
            $nLat =~ s/\[//g;
            $nLat =~ s/\(//g;
            $sLat =~ s/\(//g;
            $wLon =~ s/\)//g;
            $eLon =~ s/\)//g;

            print "   <!-- ";
            foreach my $i (@boxInfos) {
               print "$i ";
            }
            print " -->\n";

            print "   <map_supplier_coverage map_supplier=\"\">\n";
            print "      " .
               "<bounding_box north_lat=\"$nLat\" south_lat=\"$sLat\"\n" .
               "                    " .
               "west_lon=\"$wLon\" east_lon=\"$eLon\"/>\n";
            print "   </map_supplier_coverage>\n";
            print "\n";
         }
      }
   }



}
elsif  ( defined ($opt_xml2box)){
   # Parse XML to bounding box format.
   dbgprint $ARGV[0];
   

   # States
   my $beforeState=1;
   my $inCovState=2;
   my $inBoxState=3;
   my $state=$beforeState;
   
   # Box corners
   my $nlat = undef;
   my $wlon = undef;
   my $slat = undef;
   my $elon = undef;

   my $srcFile = $ARGV[0];
   open (SRCFILE, "<", $srcFile);
   while(<SRCFILE>){
       if ( $_ =~ /<map_supplier_coverage/ ){
           $state=$inCovState;
       }
       elsif ( $_ =~ /<bounding_box/ ){
           $state=$inBoxState;
       }
       elsif ( $_ =~ '/</map_supplier_coverage/' ){
           $state=$beforeState;
       }


       if ( $state == $inBoxState ){
           if (!defined($nlat) ){
               $nlat = extractXMLAttr("north_lat", $_);
           }
           if (!defined($wlon) ){
               $wlon = extractXMLAttr("west_lon", $_);
           }
           if (!defined($slat) ){
               $slat = extractXMLAttr("south_lat", $_);
           }
           if (!defined($elon) ){
               $elon = extractXMLAttr("east_lon", $_);
           }
       }
       if ( defined($nlat) &&
            defined($wlon) &&
            defined($slat) &&
            defined($elon) ){
           print "[($nlat,$wlon),($slat,$elon)]\n";
           $nlat = undef;
           $wlon = undef;
           $slat = undef;
           $elon = undef;
       }
   }



    timeprint "Parsed bounding box file.";
}
exit;



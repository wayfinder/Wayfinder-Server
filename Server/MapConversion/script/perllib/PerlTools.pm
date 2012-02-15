#!/usr/bin/perl -w
#
#
# This perl package contains common functions used in perl scripts.

package PerlTools;

use strict;

require Exporter;
our @ISA       = qw( Exporter );
our @EXPORT    = qw( &convertFromWgs84ToMc2 
                     &convertFromMc2ToWgs84
                     &splitOneMidRow
                     &readMifPointFile
                     &readAndPrintOneMifFeature
                     &printOneWayfinderMifHeader
                     &getP2PdistMeters
                     &dateAndTime
                     &dbgprint
                     &warnprint
                     &errprint
                     &timeprint
                     &redirectStdErr
                     &resetStdErr
                     &testFile
                     &testDir);

#our $VERSION     = 1.00;



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
sub stderrprintln(@) {
   my $printStr;
   my $i=0;
   for my $str (@_){
      if ($i != 0){
         $printStr.=" ";
      }
      if (!defined($str)){
         $printStr.="[UNDEFINED]";
      }
      else {
         $printStr.="$str";
      }
      $i++;
   }
   print STDERR $printStr, "\n";
}

sub dbgprint(@){ 
   stderrprintln dateAndTime(), "DEBUG:", @_;
}
sub errprint(@){ 
   stderrprintln dateAndTime(), "ERROR:", @_;
}
sub warnprint(@){ 
   stderrprintln dateAndTime(), "WARN:", @_;
}
sub timeprint(@) { 
   stderrprintln dateAndTime(), @_;
}


sub redirectStdErr {
   my $fileNameAndPath=$_[0];

   # Save STDERR and redirect it.
   open(SAVEERR, ">&STDERR"); 
   print SAVEERR ""; # Avoid warnings;
   open (STDERR, ">$fileNameAndPath")  
       or die "Can't redirect std err to $fileNameAndPath";
   select(STDERR); $| = 1; # Make STDERR unbuffered.
}

sub resetStdErr {
   # Close STDERR and reset it.
   close (STDERR) or die "Can't close STDERR";
   open(STDERR, ">&SAVEERR");
}



# Calculate distance (meters) between 2 mc2 coordinates
sub getP2PdistMeters {
   my $lat1 = $_[0];
   my $lon1 = $_[1];
   my $lat2 = $_[2];
   my $lon2 = $_[3];
   #print "Calculate distance between 2 points:($lat1,$lon1) ($lat2,$lon2)\n";
   if ( ! ( defined($lat1) and defined($lon1) and
            defined($lat2) and defined($lon2) )  ) {
      return undef;
   }

   my $pi = 3.141592653589793;
   my $degreeFactor = 11930464.7111;
   my $radianFactor = $degreeFactor * 180 / $pi;
   my $invRadianFactor = 1.0 / $radianFactor;
   
   my $coslat = cos($invRadianFactor * ( ($lat1+$lat2)/2.0 ) );
   my $latDist = abs($lat1 - $lat2);
   my $lonDist = abs($lon1 - $lon2) * $coslat;
   my $dist_mc2Sq = ($latDist*$latDist) + ($lonDist*$lonDist);
   my $dist_mc2 = sqrt($dist_mc2Sq);
   #print "dist mc2= $dist_mc2 \n";
   
   my $EARTH_RADIUS = 6378137;
   my $POW2_32 = 4294967296;
   my $MC2SCALE_TO_METER = ($EARTH_RADIUS * 2 * $pi) / $POW2_32;
   my $dist_meters = $dist_mc2 * $MC2SCALE_TO_METER;
   #print "dist meters = $dist_meters\n";

   return $dist_meters;
}


# Convert coordinates from wgs84 to mc2.
# Takes wgs84 lat+lon as inparams in that order, returns array with mc2 
# lat first followed by lon.
sub convertFromWgs84ToMc2 {
   my $lat = $_[0];
   my $lon = $_[1];
   if ( defined($lat) && defined($lon) ){
       
      # Slower more generic implementation
      #my $coordRes = `./CoordConvert --inFormat=wgs84_deg --convertCoord -- $lat $lon 2>&1 | grep mc2`;
      #$coordRes =~ s/^.*://;
      #($lat, $lon) = split (' ', $coordRes);

      my $degreeFactor = 11930464.7111;
      $lat = $lat * $degreeFactor;
      $lon = $lon * $degreeFactor;

      # make it integer, function int() truncates towards 0
      if ( $lat > 0 ) {
         $lat = int ( $lat + 0.5 );
      } else {
         $lat = int ( $lat - 0.5 );
      }
      if ( $lon > 0 ) {
         $lon = int ( $lon + 0.5 );
      } else {
         $lon = int ( $lon - 0.5 );
      }
      
      return ( $lat, $lon );      
  }
}

# Convert coordinates from wgs84 to mc2.
# Takes mc2 lat+lon as inparams in that order, returns array with WGS84
# lat first followed by lon.
sub convertFromMc2ToWgs84 {
   my $lat = $_[0];
   my $lon = $_[1];
   if ( defined($lat) && defined($lon) ){
       
      my $degreeFactor = 11930464.7111;
      $lat = $lat / $degreeFactor;
      $lon = $lon / $degreeFactor;

      return ( $lat, $lon );      
  }
}

# Split one midRow in attributes
# handling $midSepChar within strings, and removing "-chars from string values
sub splitOneMidRow {
   my $midSepChar = $_[0];
   my $midRplChar = $_[1];
   my $row = $_[2];
   my $quotationMarkWithinStrings = $_[3];

   my $midRow = "";
   if ( defined($quotationMarkWithinStrings) and
        ($quotationMarkWithinStrings eq "true") ) {
      # Check if there are any "-char(s) in the middle of strings,
      # i.e. that is not before or after a ,-char
      # 23,3,"POSL. 4. BAR 2BIG STAR PUB"","LJUBLJANSKA CESTA 69"
      (my @tmpVec) = split($midSepChar, $row, -1);
      $midRow = "";
      my $n = 0;
      foreach my $tmp (@tmpVec) {
         my $origStr = $tmp;

         # remove any "-chars that are first or last in the tmp string
         my $startFnutt = ""; my $endFnutt = "";
         my $len = length $tmp;
         $tmp =~ s/^\"//;
         if ( length $tmp < $len ) {
            $startFnutt = "\"";
            $len = length $tmp;
         }
         $tmp =~ s/\"$//;
         if ( length $tmp < $len ) {
            $endFnutt = "\"";
            $len = length $tmp;
         }
         # if still "-chars remaining, this is an error, remove it
         if ( index($tmp, "\"") > -1 ) {
            $tmp =~ s/\"//g;
         }
         $tmp = "$startFnutt$tmp$endFnutt";
         if ( $tmp ne $origStr ) {
            print "fixed \"-char(s) within string for row $. $origStr => $tmp\n";
         }
         $n += 1;
         
         my $sep = "";
         if ( length $midRow > 0 ) {
            $sep = $midSepChar;
         }
         $midRow .= "$sep$tmp";
      }
      $row = $midRow;
   }
   
   # Get the mid row and check for $midSepChar within strings
   # Replace any occurence with a $midRplChar
   (my @midRowVec) = split(//, $row);
   $midRow = "";
   my $withinString = 0;
   my $m = "";
   foreach $m (@midRowVec) {
      if ( index($m, "\"") == 0 ) {
         if ( $withinString == 0 ) {
            $withinString = 1;
         } else {
            $withinString = 0;
         }
      }
      
      if ( (index($m, "$midSepChar") == 0) and ($withinString == 1) ) {
         $midRow = "$midRow$midRplChar";
         #print "Here is a '$midSepChar' within strings for " .
         #      "midrow=$.  midRow='$midRow'\n";
      } else {
         $midRow = "$midRow$m";
      }
   }
   
   # Split the mid line
   my @midAttributes = split($midSepChar, $midRow, -1);
   # re-replace $midSepChar-$midRplChar, remove "-chars
   foreach my $tmp (@midAttributes) {
      $tmp =~ s/$midRplChar/$midSepChar/g;
      $tmp =~ s/\"//g;
   }

   return ( @midAttributes );
}


# Read a mif point file. Store point coordinates
# in coordinate hashes keyed on point number. There is no 
# coordinate conversion here!
sub readMifPointFile {
   my $mifFileName = $_[0];
   my $coordOrder = $_[1];
   print "readMifPointFile\n";

   if (! defined($coordOrder) ) {
      die "readMifPointFile: Must define coord order of mif file\n";
   }
   
   my $fileOK = testFile($mifFileName);
   if ( ! $fileOK ) {
      errprint "The mif file is not ok $mifFileName";
      die "exit";
   }
   
   open FILE, $mifFileName;
   
   my %latitudes = ();
   my %longitudes = ();
   my $nbrCoords = 0;
   my $nbrNoneCoords = 0;
   my $lat;
   my $lon;
   my $nbrProblems = 0;
   while (<FILE>) {
      chomp;
      my $mifRow = $_;
      
      # Read past mif header
      if ( index(uc($mifRow), "VERSION") == 0 ) {
         my $contHeader = 1;
         while ( $contHeader == 1 ) {
            $mifRow = (<FILE>);
            if ( index(uc($mifRow), "DATA") == 0 ) {
               $contHeader = 0;
            }
         }
      }

      # Then read the geomtery data
      if ((length() > 2) and index($_, "none") != -1) {
         $nbrCoords += 1;
         $nbrNoneCoords += 1;
         $latitudes{ $nbrCoords } = "";
         $longitudes{ $nbrCoords } = "";
      }
      elsif ((length() > 2) and index($_, "Line") != -1) {
         $nbrCoords += 1;
         dbgprint "WARN readMifPointFile " .
                  "Line feature for coord $nbrCoords " .
                  "- will store empty coord";
         $latitudes{ $nbrCoords } = "";
         $longitudes{ $nbrCoords } = "";
         $nbrProblems += 1;
      }
      elsif ((length() > 2) and index($_, "Region") != -1) {
         $nbrCoords += 1;
         dbgprint "WARN readMifPointFile " .
                  "Region feature for coord $nbrCoords " .
                  "- will store empty coord";
         $latitudes{ $nbrCoords } = "";
         $longitudes{ $nbrCoords } = "";
         $nbrProblems += 1;
      }
      elsif ((length() > 2) and index($_, "Point") != -1) {
         $nbrCoords += 1;
         if ( $coordOrder eq "lonlat" ) {
            (my $crap1, $lon, $lat) = split (" ");
         } elsif ( $coordOrder eq "latlon" ) {
            (my $crap1, $lat, $lon) = split (" ");
         }
         $latitudes{ $nbrCoords } = $lat;
         $longitudes{ $nbrCoords } = $lon;
      }
      else {
         #print "mif file error?: row $.\n";
      }
   }
   print "Stored coords for $nbrCoords points in coordinate hashes (" .
         "$nbrNoneCoords had 'none')\n\n";

   if ( $nbrProblems >  0 ) {
      errprint "File had $nbrProblems problems";
      die "exit";
   }

   # returning references of the coordinate hashes
   return (\%latitudes, \%longitudes );
   
} # readMifPointFile

# Read from a mif file in stream one mif feature of some type
# Then, if the printOption is true, write it to a file out stream
sub readAndPrintOneMifFeature {
   my $inFileName = $_[0];
   my $inFileReadFromPos = $_[1];
   my $outFileName = $_[2];
   my $print = $_[3];
   my $midId = $_[4];
   
   open INFILE, $inFileName;
   seek( INFILE, $inFileReadFromPos, 0);

   open (OUTFILE, ">>$outFileName");
   
   my $firstCoord = 0;
   
   # Read next feature from mif file and write to correct WF mif
   my $cont = 1;
   while ( $cont == 1 ) {
      my $mifRow = (<INFILE>);
      if ( $mifRow ) {
         chomp $mifRow;
      } else {
         errprint "End of mif file, when reading midId $midId\n" .
                  "- most likely we have not-handled mif features in the mif file";
         die "readAndPrintOneMifFeature: exit";
      }

      # First, check if we are reading some things that should not be
      # printed to the WF mif out file
      # Do not print the mif header to the out file
      if ( index(uc($mifRow), "VERSION") == 0 ) {
         my $contHeader = 1;
         while ( $contHeader == 1 ) {
            $mifRow = (<INFILE>);
            if ( index(uc($mifRow), "DATA") == 0 ) {
               $contHeader = 0;
            }
         }
      }
      elsif ( length($mifRow) < 1 ) {
         # skip
      }

      # We want to print the mifRow to the out file
      else {
      
         #Print things to mif file
         if ( $print ) {
            print OUTFILE "$mifRow\n";
         }
         if ( index(uc($mifRow), "REGION") == 0 ) {
            # get nbr polygons
            my @regionRow = split(" ", $mifRow, -1);
            my $nbrPolys = 0;
            if ( ! defined($regionRow[1]) ) {
               die "Nbr polygons for Region not defined, midId $midId";
            }
            $nbrPolys = $regionRow[1];
            #print "   midId $midId nbrPolys $nbrPolys\n";
            # loop all polys
            my $curPoly = 0;
            while ( $curPoly < $nbrPolys ) {
               # get nbr coords for this poly
               $mifRow = (<INFILE>);
               chomp $mifRow;
               if ( $print ) {
                  print OUTFILE "$mifRow\n";
               }
                
               my $nbrCoords = 0;
               if ( length($mifRow) < 1 ) {
                  die "Nbr coordinates for Region poly $curPoly not " .
                      "defined, midId $midId";
               }
               $nbrCoords = $mifRow;
               #print "     curPoly $curPoly nbrCoords $nbrCoords\n";
               my $curRow = 0;
               while ( $curRow < $nbrCoords ) {
                  $mifRow = (<INFILE>);
                  chomp $mifRow;
                  if ( ! $firstCoord ) {
                     $firstCoord = $mifRow;
                  }
                  if ( $print ) {
                     print OUTFILE "$mifRow\n";
                  }
                  $curRow += 1;
               }
               $curPoly += 1;
            }
            $cont = 0;
         }
         elsif ( index(uc($mifRow), "PLINE") == 0 ) {
            # get nbr coords for the Pline
            my @plineRow = split(" ", $mifRow, -1);
            my $nbrCoords = 0;
            if ( defined($plineRow[1]) and (length $plineRow[1] > 0)) {
               if ( uc($plineRow[1]) eq "MULTIPLE" ) {
                  die "Pline Multiple in network mif file, midId $midId";
               }
               $nbrCoords = $plineRow[1];
            } else {
               # Nbr coordinates in Pline not defined
               # can be located separately on the next row
               $mifRow = (<INFILE>);
               chomp $mifRow;
               @plineRow = split(" ", $mifRow, -1);
               if ( #(scalar(@plineRow) == 1) and
                     defined($plineRow[0]) and (length $plineRow[0] > 0) ) {
                  $nbrCoords = $plineRow[0];
               } else {
                  die "Problem finding nbrCoords for Pline, midId $midId";
               }
               if ( $print ) {
                  print OUTFILE "$mifRow\n";
               }
               
               if ( $nbrCoords == 0) {
                  die "Did not find nbr coords for Pline, midId $midId";
               }
            }
            my $curRow = 0;
            while ( $curRow < $nbrCoords ) {
               $mifRow = (<INFILE>);
               chomp $mifRow;
               if ( ! $firstCoord ) {
                  $firstCoord = $mifRow;
               }
               if ( $print ) {
                  print OUTFILE "$mifRow\n";
               }
               $curRow += 1;
            }
            $cont = 0;
         }
         elsif ( index(uc($mifRow), "LINE") == 0 ) {
            my @plineRow = split(" ", $mifRow, -1);
            my $nbrCoords = 2;
            if ( ! defined($plineRow[1]) ) {
               die "Line mif feature incorrect, midId $midId";
            }
            # already written to mif
            $cont = 0;
         }
         else {
            if ( (index(uc($mifRow), " PEN (") > -1) or
                 (index(uc($mifRow), " BRUSH (") > -1) or
                 (index(uc($mifRow), "SMOOTH") > -1) or
                 (index(uc($mifRow), " CENTER ") > -1) ) {
               # ok
               # already written to mif
            }
            else {
               die "readAndPrintOneMifFeature unknown mif row '$mifRow'";
            }
         }
      }
   }# while cont in mif

   # Return position where to continue reading mif file
   # also return firstCoord...
   return ( tell(INFILE), $firstCoord );
} # readAndPrintOneMifFeature

# Print a wayfinder mif header to a mif file
# Coordsys from inparam
sub printOneWayfinderMifHeader {
   my $outFileName = $_[0];
   my $coordsys = $_[1];
   
   # open to clear for append writing by the main script
   open (OUTFILE, ">$outFileName");

   print OUTFILE
         "Version 300\n" .
         "Charset \"WindowsLatin1\"\n" .
         "Delimiter \",\"\n" .
         "Coordsys $coordsys\n" .
         "Columns 3\n" .
         "  Id Decimal (16,0)\n" .
         "  Name Char (50)\n" .
         "  All_names Char (150)\n" .
         "Data\n";

}

# File options validity check methods
##############################################################################
sub testFile($){
   if ( ! defined($_[0] ) ){
      errprint("File not defined.");   
      return undef;
   }
   if ( -s $_[0]){
      if ( -d $_[0] ){
         errprint("File is a directory: $_[0]");
         return undef;
      }
      else {
         return 1;
      }
   }
   else {
      if ( -e $_[0] ){
         errprint("File: $_[0] has zero size.");
         return undef;
      }
      else {
         errprint("File: $_[0] does not exist.");
         return undef;
      }
   }
}

sub testDir($) {

   if ( ! defined($_[0] ) ){
      errprint("Directory not defined.");
      return undef;
   }

   if ( -d $_[0] ) {
       return 1;
   } elsif ( -s $_[0] ) {
       errprint("Directory $_[0] is a regular file.");
       return undef;
   } elsif ( -e $_[0] ) {
       errprint("Directory: $_[0] is a zero size file.");
       return undef;
   } else {
       errprint("Directory: $_[0] does not exist.");
       return undef;
   }
}


1;

=head1 NAME 

PerlTools

Package with common functions used in perl scripts.

=head1 USE

Include into your perl file with the combination of:
   use lib "${BASEGENFILESPATH}/script/perllib";
   use PerlTools;
pointing to the directory where the perl modules are stored

=head1 FUNCTIONS

getP2PdistMeters( $mc2lat1, $mc2lon1, $mc2lat2, $mc2lon2 )
   Get the distance (in meters) between two coords.

convertFromWgs84ToMc2( $lat, $lon )
   Convert coordinates from wgs84_deg to mc2. Takes wgs84
   lat+lon as inparams in that order, returns array with
   mc2 lat first followed by lon.
   (my $mc2Lat, my $mc2Lon ) = 
      convertFromWgs84ToMc2(  $wgs84Lat, $wgs84Lon );
   
splitOneMidRow( $midSepChar, $midRplChar, $row,
                $quotationMarkWithinStrings );
   Split one mid row in attributes. Provide mid separator
   (often ',') and a midReplaceChar (e.g. '}').
   Returns a vector with all mid attributes, keeps mid
   sep chars within strings, removes "-chars from string
   values.
   Give quotationMarkWithinStrings = true if you have "-chars
   within the strings of some mid attributes.
      my @midRow =  
         splitOneMidRow( $midSepChar, $midRplChar, $row
                         $quotationMarkWithinStrings );

readAndPrintOneMifFeature( .... )
   Read one Region/Pline mif feature from a mif file, start
   reading from a specific position in the file. If the
   wanted-param is set (1), the Region feature is
   printed to the given out mif file with append.
   Provide id for debug printing puposes. It returns
   the file position where to continue reading next Region
   mif feature. Also returns the first coord read from the
   mif file for the current feature.
      my $inFileReadPos = readAndPrintOneMifFeature(
            $inFileName, $inFileReadPos,
            $outFileName, $wanted, $id );

dbgprint( ... )
    Debug print with new line.

warnprint( ... )
    Warn print with new line.

errprint( ... )
    Error print with new line.

timeprint( ... )
    Prints the current time, the text and new line.
    
redirectStdErr( $fileNameAndPath )
    Redirects standard error to the file in fileNameAndPath. Reset with 
    resetStdErr.Calling this method twice without reset makes it impossible to
    reset to original standard error.
    

resetStdErr()
    Resets standard error to after a redirect with redirectStdErr. 

testFile($)
    Checks that a file parameter is set, exists and is non-empty.

testDir($)
    Checks that a directory parameter is set, exists and is non-empty.

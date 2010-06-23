#!/usr/bin/perl -w
#


# BASEGENFILESPATH 
# genfiles is the base directory where all setting files for map generation
# is stored
# Update it to point to the full path of where you create the BASEGENFILESPATH
#my $BASEGENFILESPATH="fullpath/genfiles";
my $BASEGENFILESPATH=".";

# Use POIObject and perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;
use PerlWASPTools;
use PerlCountryCodes;

use strict;
use Getopt::Std;
use vars qw( $opt_h $opt_l $opt_n $opt_t $opt_u $m_maxId %m_usedMidIds );
use vars qw( %m_detailedCountries $m_longSegment );

getopts('hl:ntu');

if (defined $opt_h) {
   print <<EOM;
Usage: $0 [ options ]

Options:
-h    Display this help.
-n    Get next mid id to use in stitch file in this directory.
      This option also runs all other checks.
-u    Check that mid ids are unique among the stitch files in this directory.
-l    Length to use for checking/detecting too-long stich segments between
      (detailed) neighbours. Default is 1500 meters.
-t    Instructions for testing the mif files
EOM
  exit 1;
}



$m_longSegment = 1500;
if ( defined $opt_l ) {
   $m_longSegment = $opt_l;
}

if ( defined($opt_n) ) {
   getNextMidId();
} elsif ( defined($opt_u) ) {
   checkMidIdUnique();
} elsif ( defined($opt_t) ) {
   testInstructions();
} else {
   print "nothing todo\n";
}

exit;


sub getNextMidId {
   print "Will get next mid id to use in stitching files\n";

   # Check unique first, then use the $m_maxId to get next mid id
   my $idsUnique = internalCheckMidIdUnique();
   if ( $idsUnique ) {
      my $nextId = $m_maxId + 1;
      print "Next mid id to use = $nextId\n";
   } else {
      print "You will not get a nextId before the midIds are unique\n";
      die "exit\n";
   }

   # Run all other checks here as well
   print "\n-------Running other checks as well-------\n";
   print "Check file names for obvious errors\n";
   my $fileNamesOK = internalCheckFileNames();
   if ( $fileNamesOK ) {
      print " ok\n";
   } else {
      print " not ok\n";
   }
   print "\n";

   print "Check that mid+mif files matches, e.g. nbr rows in mid " .
         "files vs. nbr Pline features in mif files\n";
   my $midRowsMifFeaturesOK = internalCmpMidRowsMifFeatures();
   if ( $midRowsMifFeaturesOK ) {
      print " ok\n";
   } else {
      print " not ok\n";
   }
   print "\n";

   print "Check mif files content\n";
   my $mifFilesOK = internalCheckMifFiles();
   if ( $mifFilesOK ) {
      print " ok\n";
   } else {
      print " not ok\n";
   }
   print "\n";
   
   print "Check mid files content\n";
   my $midFilesOK = internalCheckMidFiles();
   if ( $midFilesOK ) {
      print " ok\n";
   } else {
      print " not ok\n";
   }
   print "\n";

   # 1. number of columns on each midRow
   #    (merge 2 first stc_ssi file id 866 has 303 instead of 3,0)

}

sub checkMidIdUnique {
   print "Check that the mid ids are unique\n";
   my $idsUnique = internalCheckMidIdUnique();
   if ( $idsUnique ) {
      print "Ids are unique\n";
   } else {
      print "Ids are not unique - please fix this!\n";
   }
}
   
sub internalCheckMidIdUnique {
   
   $m_maxId = 0;
   %m_usedMidIds = ();
   # get all files in this directory, loop and read the files
   my @fileList = glob "*.mid";
   foreach my $file (@fileList) {
      #print " file: $file\n";
      open MIDFILE, $file or die "Cannot open mid file $file\n";

      while ( <MIDFILE> ) {
         chomp;
         if ( length() > 2 ) {
            (my $id, my @crap) = split (",");
            if ( $id and ($id > $m_maxId) ) {
               $m_maxId = $id;
            }
            if ( defined($m_usedMidIds{$id}) ) {
               print "ERROR: ids are not unique. I found midId $id in:\n" .
                     "  - $m_usedMidIds{$id}\n" .
                     "  - $file\n";
               return 0;
            }
            $m_usedMidIds{$id} = "$file";
         }
      }
   }
   return 1;
}


sub internalCheckMidFiles {
   my $OK = 1;

   # get all mid files in this directory, loop and read the files
   my @fileList = glob "*.mid";

   my $nbrFiles = 0;
   my $nbrColumns = 0;
   foreach my $file (@fileList) {
      $nbrFiles += 1;
      open FILE, $file or die "Cannot open mid file $file\n";

      while ( <FILE> ) {
         chomp;
         if ( length() > 1 ) {
            my @midRow =  splitOneMidRow( ",", "}", $_ );

            # nbr columns
            if ( ($nbrFiles == 1) and ($. == 1) ) {
               $nbrColumns = scalar( @midRow );
               print "Standard nbr cols = $nbrColumns\n";
            } else {
               if ( scalar( @midRow ) != $nbrColumns ) {
                  print "ERROR: incorrect nbr columns " .
                        scalar(@midRow) .  " in $file\n";
                  $OK = 0;
               }
            }
               
         }
      }
   }
   return $OK;
}

sub internalCheckMifFiles {
   my $OK = 1;

   # get all mif files in this directory, loop and read the files
   my @fileList = glob "*.mif";

   foreach my $file (@fileList) {
      #print " file: $file\n";
      open MIFFILE, $file or die "Cannot open mif file $file\n";

      my $headerDone = 0;
      my $coordSysTagExist = 0;
      my $plineNbr = 0;
      while ( <MIFFILE> ) {
         chomp;
         my $mifRow = $_;
         if ( length() > 2 ) {
            if ( ! $headerDone ) {
               # Do we read the Data-tag?
               if ( index(uc($mifRow), "DATA") > -1 ) {
                  $headerDone = 1;
                  #print " header done coordSysTagExist=$coordSysTagExist\n";
                  if ( ! $coordSysTagExist ) {
                     print "ERROR: no coord sys tag found in $file\n";
                     $OK = 0;
                  }
               }
               # Do we have a coord sys tag?
               elsif ( index(uc($mifRow), "COORDSYS") > -1 ) {
                  $coordSysTagExist = 1;
                  if ( index( uc($mifRow), "MC2") > -1 ) {
                     #print " coordsys mc2 in $file\n";
                     if ( index( uc($mifRow), "LONLAT") > -1 ) {
                        print "ERROR: incorrect coordsys tag " .
                              "LONLAT in $file\n";
                        $OK = 0;
                     }
                  } else {
                     print "ERROR: incorrect coordsys tag in $file\n";
                     $OK = 0;
                  }
               }
               
            } else {
               # Check that coord values are mc2 (abs(coord) > 180) XXX
               # Read the Plines and check the length of the segments
               if ( index(uc($mifRow), "PLINE") > -1 ) {
                  $plineNbr += 1;
                  # get nbr coords for the Pline
                  my @plineRow = split(" ", $mifRow, -1);
                  my $nbrCoords = 0;
                  if ( defined($plineRow[1]) and
                       (length $plineRow[1] > 0)) {
                     $nbrCoords = $plineRow[1];
                  }
                  if ( $nbrCoords == 0) {
                     print "ERROR: Problem finding nbrCoords for " .
                           "Pline $plineNbr in $file\n";
                     $OK = 0;
                  }
                  my $curRow = 0;
                  my $lat0 = 0; my $lon0 = 0; my $lat = 0; my $lon = 0;
                  while ( $curRow < $nbrCoords ) {
                     $mifRow = (<MIFFILE>);
                     if ( ! defined $mifRow ) {
                        print "ERROR: Problem with number coordinates for " .
                              "Pline $plineNbr coord $curRow in $file\n";
                        $OK = 0;
                     } else {
                        chomp $mifRow;
                        ($lat, $lon) = split(" ", $mifRow, -1);
                        if ( $curRow == 0 ) {
                           $lat0 = $lat;
                           $lon0 = $lon;
                        }
                     }
                     $curRow += 1;
                  }
                  # distance
                  my $dist_meters = 
                     getP2PdistMeters($lat0,$lon0,$lat,$lon);
                  if ( $dist_meters > $m_longSegment ) {
                     # 1.5 km
                     if ( detailedCountries($file) ) {
                        print "WARN: length = $dist_meters in $file" .
                              " $lat0 $lon0 -> $lat $lon\n";
                     } elsif ($dist_meters > 1000000 ) {
                        # 1000 km
                        print "WARN: length = $dist_meters in $file" .
                              " $lat0 $lon0 -> $lat $lon\n";
                     }
                  }
               }
               
            }
         }
      }
   }
   return $OK;
}

# Compare nbr lines in the mid file with nbr Pline features in mif file
sub internalCmpMidRowsMifFeatures {
   my $OK = 1;
   
   # Get all mif files in this directory
   my @miffileList = glob "*.mif";
   my @midfileList = glob "*.mid";

   print "Number mif files " . scalar(@miffileList) . "\n";
   print "Number mid files " . scalar(@midfileList) . "\n";
   if ( scalar(@miffileList) != scalar(@midfileList) ) {
      print " different - please fix!\n";
      return 0;
   }
   
   # Loop and read the mif files, count and store nbr Plines in each
   # The hash key is the base name of the midmif file (no file extension)
   my %nbrPlinesInMif = ();
   foreach my $file (@miffileList) {
      open MIFFILE, $file or die "Cannot open mif file $file\n";

      my $nbrPlines = 0;
      while ( <MIFFILE> ) {
         chomp;
         #print "$file: $_\n";
         if ( index(uc($_), "PLINE") > -1 ) {
            $nbrPlines += 1;
            #print "  pline\n";
         }
      }
      my $baseName = $file;
      $baseName =~ s/\.mif$//;
      $nbrPlinesInMif{$baseName} = $nbrPlines;

      my $midfile = $baseName . ".mid";
      if ( ! open MIDFILE, $midfile ) {
         print "ERROR: no mid file for $file\n";
         $OK = 0;
      }
   }
   
   # Loop and read the mid files, count nbr rows and compare with
   # nbr mif features
   foreach my $file (@midfileList) {
      open MIFFILE, $file or die "Cannot open mif file $file\n";
      my $wcResult = `wc -l $file`;
      (my $nbrRows, my @crap) = split (' ', $wcResult);
      
      my $baseName = $file;
      $baseName =~ s/\.mid$//;
      
      if ( !defined $nbrPlinesInMif{$baseName} ) {
         print "ERROR: no mif file for $file\n";
         $OK = 0;
      }
      elsif ( $nbrRows != $nbrPlinesInMif{$baseName} ) {
         print "ERROR: nbrMidRows=$nbrRows nbr Pline=" .
               $nbrPlinesInMif{$baseName} . " for file $file\n";
         $OK = 0;
      }
   }
   return $OK;
}

sub testInstructions {
   print "\nInstructions for testing that the stitching mif files are ok\n" .
         "Will give result if all the mif features are correct, e.g. that \n" .
         "each Pline N reallyhas N nbr of coords, else MifTool will fail\n" .
         "reading/creating gfxDatas from the mif files.\n";
   print "\n";
   print "1 Cat all mif files into one\n";
   print "2 Try to load the file with MifTool\n";
   print "    MifTool --load stitchFile.mif\n";
   print "3 Compare with number of Plines in stitch file\n";
   print "\n";
   
}

# Check that the file name of all the stitch files are "sane"
sub internalCheckFileNames {
   my $OK = 1;
   
   # Get all files in this directory
   my @midfileList = glob "*.mid";
   my @miffileList = glob "*.mif";
   my @extfileList = glob "*.ext";

   my @fileList;
   foreach my $file (@midfileList) {
      push @fileList, $file;
   }
   foreach my $file (@miffileList) {
      push @fileList, $file;
   }
   foreach my $file (@extfileList) {
      push @fileList, $file;
   }
   
   
   # report each neighbour pair problem only once
   my %unknownNeighbours = ();

   # Loop files
   foreach my $file (@fileList) {
      open MIFFILE, $file or die "Cannot open file $file\n";

      # there must never be double-underscores
      if ( index($file, "__") > -1 ) {
         print "ERROR: file name $file (double underscore)\n";
         $OK = 0;
      }
      
      # there must always be a - sign separating the 2 country_mapreleas
      if ( index($file, "-") == -1 ) {
         print "ERROR: file name $file (missing '-')\n";
         $OK = 0;
      }

      # get the 2 letter country codes
      my $ccOK = 1;
      (my $cc1, my $cc2 ) = getCountryCodesFromFileName($file);
      if ( (!defined $cc1) or (length($cc1) != 2) ) {
         print "ERROR: problem with 1st 2-letter country code in $file\n";
         $ccOK = 0;
      }
      if ( (!defined $cc2) or (length($cc2) != 2) ) {
         print "ERROR: problem with 2nd 2-letter country code in $file\n";
         $ccOK = 0;
      }
      # the 2 letter country codes must be in the alphabetical order
      if ( $ccOK ) {
         # -1 less than, 0 equal, 1 greater than
         my $compare = ($cc1 cmp $cc2);
         if ( $compare >= 0 ) {
            print "ERROR: wrong alphabetical order in file $file\n";
            $ccOK = 0;
         }
      }
      # translate to country name
      my $country1; my $country2;
      if ( $ccOK ) {
         $country1 = getCountryFromAlpha2($cc1);
         $country2 = getCountryFromAlpha2($cc2);
         if ( ! defined $country1 ) {
            print "ERROR: no valid 2-letter country code in $cc1\n";
            $ccOK = 0;
         }
         if ( ! defined $country2 ) {
            print "ERROR: no valid 2-letter country code in $cc2\n";
            $ccOK = 0;
         }
      }
      # check that the countries are defined as neighbours in mfunctions
      if ( $ccOK ) {
         #print "test $cc1 $country1 $cc2 $country2\n";
         my $grep = `grep $country1 ${BASEGENFILESPATH}/script/mfunctions.sh |grep $country2`;
         chomp $grep;
         #print " grep = '$grep'\n";
         if ( length $grep < 1 ) {
            $unknownNeighbours{"$country1 $country2"} = 1;
         }
      }
      if ( ! $ccOK ) {
         $OK = 0;
      }
      
   }

   foreach my $n (keys %unknownNeighbours) {
      print "ERROR: unknown neighbours in mfunctions.sh: $n - please add!\n";
      $OK = 0;
   }
   
   return $OK;
}

sub getCountryCodesFromFileName
{
   my $fileName = $_[0];
   
   (my $tmp1, my $tmp2) = split("-", $fileName, -1);
   (my $cc1, my @crap) = split("_", $tmp1, -1);
   (my $cc2, @crap) = split("_", $tmp2, -1);
   
   return ( $cc1, $cc2);
}


# return true if both countries are detailed
sub detailedCountries
{
   my $fileName = $_[0];
   (my $cc1, my $cc2 ) = getCountryCodesFromFileName($fileName);
   $cc1 = uc $cc1;
   $cc2 = uc $cc2;

   if ( ! scalar %m_detailedCountries ) {
      initDetailedCountries();
   }

   if ( $m_detailedCountries{$cc1} and 
        $m_detailedCountries{$cc2} ) {
      # both countries detailed
      
      # execption for bg
      if ( ($cc1 eq "BG") and ($cc2 eq "TR") ) {
         return 0;
      }
      
      return 1;
   }


   # only one, or none of the countries are detailed
   return 0;
}

sub initDetailedCountries
{
   %m_detailedCountries = ();
   
   my $dbh = db_connect("noprint");
   my $query = 
        "SELECT iso3166_1_alpha2, detailLevel FROM POICountries;";
   my $sth = $dbh->prepare( $query ) or 
       die "Could not prepare query: $query\n";
   $sth->execute() or die "Could not execute query: $query\n";
   
   while ( (my $cc, my $detailed) = $sth->fetchrow() ) {
      $m_detailedCountries{$cc} = $detailed;
   }
}



#!/usr/bin/perl -w
# Misc midmif tools
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



use strict;
use Getopt::Std;
use vars qw( $opt_h $opt_m $opt_w $opt_v);

# Use perl modules from here or BASEGENFILESPATH/script/perllib
# use lib "fullpath/genfiles/script/perllib";
use lib ".";
use PerlTools;

getopts('hmwv:');

if (defined $opt_h) {
   print <<EOM;
Other useful midmif tools can be found in:
* midmif_cleanCosmeticMifFeatures.pl
   Find and clean out cosmetic features (PEN, BRUSH) from a mif file.
   It is necessary if you want to run ESRI's MIFSHAPE Utility.
* midmif_fixPlineMultiple.pl
   Replace 'Pline Multiple' features with 'Pline' features, duplicating
   rows in the mid file. Necessary for Wayfinder midmif format.
Or use the MifTool from the mc2 tree


Usage: $0 [ options ] [ file(s) ]

Options:
-h    Display this help.
-m    Get min/max coord values in the mif file(s) in tail. Currently 
      it handles Pline+Region fetures (not Point).
-w    Find which midIds that are not added to a mcm map. Compares the
      WF midmif file with the log file from addition in map generation.
      Result presented is midId, midRow, name (if any) and coord.
      Inparams, either 2 or 3
         - origWayfinderMidFile, createWFitem log file
         - origWayfinderMidFile, origWayfinderMiFFile, createWFitem log file
      If providing the mif file, the method is slower, but you will be
      presented with the first coordinate of the not added item.
-v    Verify that mid and mif files from a supplier are correct, i.e.
      that the number of rows in the mid file is equalto the number of
      features in the mif file. Provide directory to check.
EOM
  exit 1;
}



if ( defined($opt_m) ) {
   getMinAndMax();
} elsif ( defined($opt_w) ) {
   findMissingMidIds();
} elsif ( defined($opt_v) ) {
   verifyMidMifFiles();
} else {
   print "nothing todo\n";
}

exit;





# Compare nbr lines in the mid file with nbr Pline features in mif file
# Copied from / inspired by checkStitchFiles.pl
sub internalCmpMidRowsMifFeatures {
   my $mifDir = $_[0];
   my $OK = 1;
   
   # Get all mif files in this directory, *mif *.mid *.MIF *.MID
   my @miffileList = glob "$mifDir/*.mif";
   my @bigmiffileList = glob "$mifDir/*.MIF";
   my @midfileList = glob "$mifDir/*.mid";
   my @bigmidfileList = glob "$mifDir/*.MID";

   my @allmiffileList;
   foreach my $file (@miffileList) {
      push @allmiffileList, $file;
   }
   foreach my $file (@bigmiffileList) {
      push @allmiffileList, $file;
   }
   my @allmidfileList;
   foreach my $file (@midfileList) {
      push @allmidfileList, $file;
   }
   foreach my $file (@bigmidfileList) {
      push @allmidfileList, $file;
   }

   print "Number mif files " . scalar(@allmiffileList) . "\n";
   print "Number mid files " . scalar(@allmidfileList) . "\n";
   if ( scalar(@allmiffileList) != scalar(@allmidfileList) ) {
      print " different - please fix!\n";
      return 0;
   }
   if ( scalar(@allmiffileList) == 0 ) {
      print " no mif files found in $mifDir\n";
      return 0;
   }
   
   # Loop and read the mif files, count and store nbr Plines in each
   # The hash key is the base name of the midmif file (no file extension)
   my %nbrFeaturesInMif = ();
   foreach my $file (@allmiffileList) {
      open MIFFILE, $file or die "Cannot open mif file $file\n";

      my $nbrPlines = 0;
      my $nbrLines = 0;
      my $nbrRegions = 0;
      my $nbrPoints = 0;
      my $nbrNone = 0;
      while ( <MIFFILE> ) {
         chomp;
         if ( index(uc($_), "PLINE") > -1 ) {
            $nbrPlines += 1;
         }
         elsif ( index(uc($_), "LINE") > -1 ) {
            $nbrLines += 1;
         }
         elsif ( index(uc($_), "REGION") > -1 ) {
            $nbrRegions += 1;
         }
         elsif ( index(uc($_), "POINT") > -1 ) {
            $nbrPoints += 1;
         }
         elsif ( index(uc($_), "NONE") > -1 ) {
            $nbrNone += 1;
         }
      }
      my $nbrMifFeatures = $nbrPlines + $nbrLines + 
                           $nbrRegions + $nbrPoints + $nbrNone;
      
      my $baseName = $file;
      $baseName =~ s/\.mif$//;
      $baseName =~ s/\.MIF$//;
      $nbrFeaturesInMif{$baseName} = $nbrMifFeatures;

      print " $baseName pline=$nbrPlines line=$nbrLines" .
            " region=$nbrRegions point=$nbrPoints none=$nbrNone\n";
      
      my $midfile = $baseName . ".mid";
      my $bigmidfile = $baseName . ".MID";
      if ( (! open MIDFILE, $midfile) and (! open MIDFILE, $bigmidfile) ) {
         print "ERROR: no mid file for $file\n";
         $OK = 0;
      }
   }
   
   # Loop and read the mid files, count nbr rows and compare with
   # nbr mif features
   foreach my $file (@allmidfileList) {
      open MIDFILE, $file or die "Cannot open mif file $file\n";
      my $wcResult = `wc -l $file`;
      (my $nbrRows, my @crap) = split (' ', $wcResult);
      
      my $baseName = $file;
      $baseName =~ s/\.mid$//;
      $baseName =~ s/\.MID$//;
      
      if ( !defined $nbrFeaturesInMif{$baseName} ) {
         print "ERROR: no mif file for $file\n";
         $OK = 0;
      }
      elsif ( $nbrRows != $nbrFeaturesInMif{$baseName} ) {
         print "ERROR: nbrMidRows=$nbrRows nbr mif features=" .
               $nbrFeaturesInMif{$baseName} . " for file $file\n";
         $OK = 0;
      }
   }
   return $OK;
} # internalCmpMidRowsMifFeatures

sub verifyMidMifFiles {

   my $mifDir = $opt_v;
   my $OK = internalCmpMidRowsMifFeatures( $mifDir );

   if ( $OK ) {
      dbgprint "Everything OK";
   } else {
      dbgprint "Problems with midmif files";
   }
}

sub findMissingMidIds {
   dbgprint "Find which midmif items from a Wayfinder mid file were " .
            "never added to mcm maps";

   my $MIDFILE_NAME = "";
   my $MIFFILE_NAME = "";
   my $LOGFILE_NAME = "";
   if ( scalar @ARGV == 2 ) {
      $MIDFILE_NAME = $ARGV[ 0 ];
      $LOGFILE_NAME = $ARGV[ 1 ];
   }
   elsif ( scalar @ARGV == 3 ) {
      $MIDFILE_NAME = $ARGV[ 0 ];
      $MIFFILE_NAME = $ARGV[ 1 ];
      $LOGFILE_NAME = $ARGV[ 2 ];
   }


   # Read original mid+mif file and store midIds
   print "Wayfinder item mid file: $MIDFILE_NAME\n";
   open MIDFILE, $MIDFILE_NAME
      or die "Can not open mid file $MIDFILE_NAME: $!";
   my $nbrMidItems = 0;
   my %midItems = ();
   my %midItemRows = ();
   my %midItemNames = ();
   my %mifCoord = ();
   my $mifFileReadPos = 0;
   while(<MIDFILE>) {
      chomp;
      $nbrMidItems += 1;
      (my $id, my $name, my @crap) = split( "," );
      if ( defined($id) ) {
         $id = int $id;
      }
      if ( defined($id) ) {
         $midItems{$id} = 0;
         $midItemRows{$id} = $nbrMidItems;
         $name =~ s/\"//g;
         if ( length $name > 1 ) {
            $midItemNames{$id} = $name
         }
      }
      
      if ( length $MIFFILE_NAME > 1 ) {
         open(TMP, ">blaj");
         print TMP "";
         ($mifFileReadPos, my $coord) = readAndPrintOneMifFeature(
            $MIFFILE_NAME, $mifFileReadPos,
            "blaj", 1, # 0 = don't print
            $id );
         $mifCoord{$id} = "$coord";
      }
      
   }
   print "Read " . scalar(keys(%midItems)) . " items from mid file\n";
   print "This nbr must be the same: $nbrMidItems\n";
   print "\n";

   
   # Read the map generating log files, looking for the IATM-tag
   # Mark the ids in %midItems
   print "Log file: $LOGFILE_NAME\n";
   open LOGFILE, $LOGFILE_NAME
      or die "Can not open log file $LOGFILE_NAME: $!";
   my $nbrAddedItems = 0;
   my %duplicatedMidItems = ();
   while (<LOGFILE>) {
      chomp;
      (my $crap, my $idInfoStr) = split("IATM: ");
      if ( defined($idInfoStr) ) {
         $nbrAddedItems += 1;
         (my $mapIdStr,
          my $midIdStr,
          my $mcmIdStr) = split( " ", $idInfoStr);
         my $mapId = $mapIdStr;
         $mapId =~ s/map=//;
         my $midId = $midIdStr;
         $midId =~ s/midid=//;
         my $mcmId = $mcmIdStr;
         $mcmId =~ s/newid=//;

         if ( $midItems{$midId} ) {
            $duplicatedMidItems{$midId} = 1;
         }
         
         $midItems{$midId} = "$mapId:$mcmId";
      }
   }
   print "read $nbrAddedItems IATM from log file\n";
   if ( scalar(keys(%duplicatedMidItems)) ) {
      print " of which " . scalar(keys(%duplicatedMidItems)) .
            " midIds are added to more than one map\n";
      foreach my $id ( sort {$a <=> $b} (keys %duplicatedMidItems) ) {
         print "  $id";
         if ( defined ($mifCoord{$id}) ) {
            print " coord: $mifCoord{$id}";
         }
         if ( defined ($midItemNames{$id}) ) {
            print " name: $midItemNames{$id}";
         }
         print "\n";

      }
   }
   print "\n";

   # Check if some ids in %midItems are not added to mcm maps
   dbgprint "Result:";
   my $nbrNotAdded = 0;
   foreach my $i ( sort {$a <=> $b} (keys(%midItems)) ) {
      #print " $i = $midItems{$i}\n";
      if ( ! $midItems{$i} ) {
         $nbrNotAdded += 1;
         print "Not added midId: " . $i .
               " midRow: " . $midItemRows{$i};
         if ( defined ($mifCoord{$i}) ) {
            print " coord: $mifCoord{$i}";
         }
         if ( defined ($midItemNames{$i}) ) {
            print " name: $midItemNames{$i}";
         }
         print "\n";
      }
   }
   if ( $nbrNotAdded ) {
      print "\n" .
            "Totally $nbrNotAdded not added\n";
   } else {
      print "All items were added to the map according to the log\n";;
   }
   
   print "\n";
   dbgprint "Done!";
}


sub getMinAndMax {

   my $nbrFiles = scalar @ARGV;
   print "Number files $nbrFiles\n";

   my $MAX_UINT32 = 4294967295;
   my $file = 0;
   my $totMinLat = 4294967295; my $totMaxLat = -4294967295;
   my $totMinLon = 4294967295; my $totMaxLon = -4294967295;
   while ( $file < $nbrFiles ) {

      my $FILE_NAME = $ARGV[ $file ];
      open FILE, $FILE_NAME;
      print "File $FILE_NAME\n";

      my $rows = 0;
      my $nbrFeatures = 0;
      my $tmp = 0;
      my $minLat = 4294967295; my $maxLat = -4294967295;
      my $minLon = 4294967295; my $maxLon = -4294967295;
      while (<FILE>) {
         chomp;
         $rows += 1;
         my $mifRow = $_;
         
         if ( length() > 2 ) {
            if ( index( uc($mifRow), "PLINE") != -1 ) {

               $nbrFeatures += 1;
               (my @rec) = split (" ");
               my $nbrCoords = $rec[( scalar(@rec) - 1)];
               
               $tmp = 0;
               while ( $tmp < $nbrCoords ) {
                  $mifRow = (<FILE>);
                  (my $lat, my $lon) = split (" ",  $mifRow, -1);
                  if ( ($nbrFeatures == 1) and ($tmp == 0) ) {
                     $minLat = $lat;
                     $maxLat = $lat;
                     $minLon = $lon;
                     $maxLon = $lon;
                  } else {
                     if ( $lat < $minLat ) {
                        $minLat = $lat;
                     }
                     if ( $lat > $maxLat ) {
                        $maxLat = $lat;
                     }
                     if ( $lon < $minLon ) {
                        $minLon = $lon;
                     }
                     if ( $lon > $maxLon ) {
                        $maxLon = $lon;
                     }
                  }
                  $tmp += 1;
               }


            } # PLINE
            elsif ( index( uc($mifRow), "REGION") != -1) {
               $nbrFeatures += 1;
               (my @rec) = split (" ");
               my $nbrPolys = $rec[( scalar(@rec) - 1)];
               # Read $nbrPolys
               $tmp = 0;
               while ( $tmp < $nbrPolys ) {
                  $mifRow = (<FILE>);
                  chomp $mifRow;
                  my @rec2 = split ( " ", $mifRow);
                  my $nbrCoords = $rec2[( scalar(@rec2) - 1)];
                  # Read $nbrCoords 
                  my $tmp2 = 0;
                  while ( $tmp2 < $nbrCoords ) {
                     $mifRow = (<FILE>);
                     chomp $mifRow;
                     (my $lat, my $lon) = split (" ",  $mifRow, -1);
                     if ( $lat < $minLat ) {
                        $minLat = $lat;
                     }
                     if ( $lat > $maxLat ) {
                        $maxLat = $lat;
                     }
                     if ( $lon < $minLon ) {
                        $minLon = $lon;
                     }
                     if ( $lon > $maxLon ) {
                        $maxLon = $lon;
                     }
                     $tmp2 += 1;
                  }
                  $tmp += 1;
               }
            } # REGION
            
         }
      }

      print " Read $nbrFeatures mif features\n";
      print "  minLat = $minLat\n";
      print "  maxLat = $maxLat\n";
      print "  minLon = $minLon\n";
      print "  maxLon = $maxLon\n";

      if ( $minLat < $totMinLat ) {
         $totMinLat = $minLat;
      }
      if ( $maxLat > $totMaxLat ) {
         $totMaxLat = $maxLat;
      }
      if ( $minLon < $totMinLon ) {
         $totMinLon = $minLon;
      }
      if ( $maxLon > $totMaxLon ) {
         $totMaxLon = $maxLon;
      }
 
      # Goto next file
      $file += 1;
   }
   print "Read $file mif files\n";
   print " tot minLat = $totMinLat\n";
   print " tot maxLat = $totMaxLat\n";
   print " tot minLon = $totMinLon\n";
   print " tot maxLon = $totMaxLon\n";
   
}



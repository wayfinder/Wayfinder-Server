#!/usr/bin/perl -w
#
#

# To make sure output is printed directly to stdout even if piping 
# with e.g. tee
$| = 1;

use strict;
#XXXuse Data::Dumper;

# Print command line.
print "Command: $0";
foreach my $arg (@ARGV){
   print " $arg";
}
print "\n\n";


use DBI;
use Getopt::Std;

# Use POIObject and perl modules and other scripts from 
# here or BASEGENFILESPATH/script/perllib or BASEGENFILESPATH/script
# use lib "fullpath/genfiles/script/perllib";
# use lib "fullpath/genfiles/script";
use lib ".";
use String::Similarity;
use Encode;
use PerlWASPTools;
use PerlTools;
use POIObject;



# use encoding will create perl segmentation fault
#use encoding 'utf8';

require "poi_doublesTypeMapping.pl";

use vars qw( $opt_a $opt_c $opt_d $opt_f $opt_h $opt_i $opt_l $opt_m $opt_p 
             $opt_r $opt_s $opt_t $opt_n $opt_b $opt_q );

my $debug = 0;
my ($dbh);

my ($source, $country, $nonMatchFile);

my $parse_only;
my $trust_sourceref;
my ($check_against_sql, $check_against_sth);
my ($check_names_sql, $check_names_sth);
my (@check_names_res);
my ($ot_sql, $ot_sth);
my (@check_against_res, @ot_res);
my ($ot_names_sth);
my (@ot_names_res);

my $default_similarity;
my ($dist, $coord_const, $mc2dist);
my ($maxlat, $minlat, $maxlon, $minlon);

my ($i, $k);


my $individual_poi_id;
my $type_mapping;
my $type_search;
my $invRadianFactor;
my $radianFactor;
my $degreeFactor;
my $pi;
my $coslat;
my $lat;
my $lon;
my $lon_mc2dist;
my $lat_mc2dist;
my $main_source;
my ($latest_poiid, $same_poiid);
my (%parent);
my $number_staticids_found = 0;
my (%poiid_to_staticid);
my (%duplicate_not_found);
my (%old_parent);
my (@children);
my $coord_source;
my $create_parents;
my $last_poiid;
my $current_poiid;
my $self_check_alt = "";
my ($prevSourceId, $prevSourceName);

my $externalCheck = 0; # Set to true if we compare two sources of POIs.

my @things_in_parantheses;
@things_in_parantheses = ('\(les\)', 
                           '\(le\)', 
                           '\(the\)', 
                           '\(de\)', 
                           '\(la\)', 
                           '\(l\'\)', 
                           '\(au\)', 
                           '\(d\'\)',
                           '\(des\)', 
                           '\(de\)', 
                           '\(die\)', 
                           '\(der\)',
                           '\(das\)');

#########################################################################
# setting variables
#########################################################################

# this is the amount of mc2-units in one meter at the Equator
$coord_const = 107.173187966397;

$pi = 3.141592653589793;
$degreeFactor = 11930464.7111;
$radianFactor = $degreeFactor * 180 / $pi;
$invRadianFactor = 1.0 / $radianFactor;

$i = 0;

########################################################################
# check options

getopts('hpqdrb:a:c:i:l:m:s:f:t:n:');

if (defined $opt_h) {
   print <<EOM;
POI doubles elimination script.

-m mainSourceName
-c countryName
-s testSourceName
-q quiet, not so much dbgprint. This is not implemented in all
   functions.
-p flag to create parents
-d flag to print debug during run
-i individualPoiID (check just this poiID)
-t typeMapping only supports "PAR" or nothing
   "PAR" will exclude typeMapping. Nothing will use standard typeMapping.
-r flag to say that we can trust sourceReference so that we only the 
   sourceReference to find the doubles from the same kind of source
   (in checkDoubles or migratePOIStatic)
-a distance in metres for bounding box (100 is default)
-l similarity of words in percent when comparing names (90(%)) is default
-n name and path of file where to print IDs of non-matching POIs. 
   Used together with migratePOIStatic.
-b Give this option with static check to make the script try only some of the
   POIs at the same time, avoiding out of memory problems. Give the
   latitude band to start at as value. 1 if to start from beginning.
Use with -f function to run -m mainSourceID -c countryName -s testSourceID

functions: checkDoubles
           selfCheck
           migratePOIStatic
           newPOIsToPOIStatic

checkDoubles checks for pois in main source that match pois in test source
on type, coordinates and name.

selfCheck does the same as checkDoubles but within one source only

migratePOIStatic will check for pois in test source that have an ID in
table POIStatic. For each of these POIs there will be a check to see
if the main source has a poi with the exact same sourceReference. Stores all
non-matching POI IDs in the file given by -n. Combine with -b for large POI sets.

newPOIsToPOIStatic will add new poiIDs from a new source to POIStatic

EOM

exit 0;
}

#######################################################################
# connect to database
$dbh = &db_connect;
dbgprint "Connected to db\n";

# check options
if (defined($opt_d)) {
   dbgprint "Will do debug printing\n";
	$debug = 1;
}
if (defined($opt_s)) {
   $source = getSourceIDFromName($dbh, $opt_s);
	if ( !defined($source) ) {
      die "Error: Incorrect testing source id given ($opt_s)\n";
   }
}
if (defined($opt_p)) {
	$create_parents = 1;
} else {
	$create_parents = 0;
}
if (defined($opt_r)) {
	$trust_sourceref = 1;
} else {
	$trust_sourceref = 0;
}
if (defined($opt_m)) {
   $main_source = getSourceIDFromName($dbh, $opt_m);

	# set from which source to get the coords
	$coord_source = $main_source;

	if ( !defined($main_source) ) {
      die "Error: Incorrect main source id given ($opt_m)\n";
   }
}
if (defined $opt_c) {
  $country = getCountryIDFromName($dbh, $opt_c);
  if ( !defined($country) ) {
     die "Error: Incorrect country given ($opt_c)\n";
  }
}
if (defined $opt_i && $opt_i ne "") {
   dbgprint ("Using individual POI ID: $opt_i");
	$individual_poi_id = "POIMain.ID IN ($opt_i) AND ";
} else {
	$individual_poi_id = "";
	$opt_i = "none";
}
# choose type mapping
if (defined($opt_t)) {
	if ($opt_t eq "par") {
		$type_mapping = "par";
	} else {
		$type_mapping = "standard";
	}
} else {
	$type_mapping = "standard";
}
# distance in meters 
if ((defined($opt_a)) && $opt_a ne '') {
   $dist = $opt_a;
} else {
	$dist = 100;
}
# similarity in percent 
if ((defined($opt_l)) && $opt_l ne '') {
   $default_similarity = ($opt_l / 100);
} else {
   $default_similarity = 0.92;
}
if ( defined $opt_n ){
   $nonMatchFile = $opt_n;
}
# Decide what function to run
if (defined($opt_f)){
   if ($opt_f eq "checkDoubles") {
      dbgprint "Will check doubles\n";
      $externalCheck=0;
      &check_mandatory_input();
      &print_start_info();
      &check_doubles();
      &print_summary();
      if ($create_parents) {
         &check_all_parents_common_children();
         #&create_parents_poimain();
      }
   } elsif ($opt_f eq "selfCheck") {
      # set source and main_source to be the same
      $source = $main_source;
      $opt_s = $opt_m;
      ($prevSourceId, $prevSourceName) = getPrevPoiSource($dbh, $source);
      timeprint "Do selfcheck on source $main_source\n";
      $self_check_alt = "AND POIMain.ID <> ? ";
      &check_mandatory_input();
      &print_start_info();
      &check_doubles();
      # here we want to 
      # check what duplicate to delete
      # and delete it
      #if (!defined $prevSourceId) {
      #   dbgprint "Did not find previous source, maybe there is no?\n";
      #   dbgprint "No need to check which doubles were deleted in previous source\n";
      #} else {
      #   &double_deleted_in_previous_source();
      #}
      &print_summary();
   } elsif ($opt_f eq "migratePOIStatic") {
      # set type mapping to par
      # to prevent more than one try per poi
      #$type_mapping = "par";
      dbgprint "Doing migratePOIStatic\n";
      &check_static_input();
      &print_start_info();
      &migrate_poi_static();
   } elsif ($opt_f eq "newPOIsToPOIStatic") {
      dbgprint "Doing newPOIsToPOIStatic\n";
      # use main source
      $source = $main_source;
      $opt_s = $opt_m;
      &insert_to_poistatic();
   
   } else {
      die "Error Unknown function: $opt_f\n";
   }
} else {
  die "Error: No function specified!\n";
}
########################################################################
# disconnect from database
&db_disconnect($dbh);



#########################################################################
# sub check_doubles
#########################################################################
sub check_doubles {
   my $deletedCond = " AND POIMain.deleted = 0";
   my $coordCond =   "POIMain.lat IS NOT NULL AND " .
                     "POIMain.lon IS NOT NULL";

   if ( defined $_[0] ){
      # This means that we should check against deleted POIs as well.
      # Used when migrating static IDs.
      $deletedCond = "";
   }

   my $countryCond=""; # Condition for country.
   if ( defined($country) && $country ne "" ){
      $countryCond = "POIMain.country = $country AND ";
   }
   
   $check_against_sql = "SELECT POIMain.ID, lat, lon, POITypes.typeID, " .
       "sourceReference " .
       "FROM POIMain left join POITypes ON " .
       "POIMain.ID = POITypes.poiID WHERE " .
       "POIMain.source IN ($source) AND " .
       $individual_poi_id .
       " " . $countryCond .
       " " . $coordCond . 
       " " . $deletedCond;
   
   $check_names_sql = "SELECT name FROM " .
       "POINames WHERE poiID = ? AND " .
       "(type >= 0 AND type <= 1)";
   
   $debug && timeprint "CHECK AGAINST QUERY: $check_against_sql\n";
   
   $check_against_sth = $dbh->prepare($check_against_sql);
   $check_names_sth = $dbh->prepare($check_names_sql);
   $ot_names_sth = $dbh->prepare($check_names_sql);
   
   $check_against_sth->execute() 
       or die ;
   
   if ($type_mapping eq "par") {
      $type_search = "";
   } else {
      $type_search = "POITypes.typeID IN (?) AND ";
   }
   
   $ot_sql = "SELECT POIMain.ID, lat, lon, source, POITypes.typeID FROM " .
       "POIMain LEFT JOIN POITypes ON " .
       "POIMain.ID = POITypes.poiID WHERE " .
       "POIMain.source IN ($main_source) AND " .
       "$countryCond ";
   
   my $rest_ot_sql = "";

   if ($trust_sourceref) {
      $rest_ot_sql = " sourceReference = ? " .
          $deletedCond;
   } else {
      $rest_ot_sql = 
          $type_search .
          "POIMain.lat <= ? AND " .
          "POIMain.lat >= ? AND " .
          "POIMain.lon <= ? AND " .
          "POIMain.lon >= ? " .
          $self_check_alt .
          $deletedCond;
   }
   
   $ot_sql = $ot_sql . $rest_ot_sql;
   
   $ot_sth = $dbh->prepare($ot_sql);
   
   $same_poiid = 0;
   
#########################################################################
# starting running through test pois

   $last_poiid = "";
   $current_poiid = "";
   
   while (@check_against_res = $check_against_sth->fetchrow_array()) {
      $current_poiid = $check_against_res[0];
      if ( $type_mapping eq "par" && ($last_poiid == $current_poiid) ) {
         $debug && dbgprint "INFO: Last poiid equal to current poiid. Not checking this!\n";
         $last_poiid = $current_poiid;
         next;
      }
      $last_poiid = $current_poiid;
      
      $check_names_sth->execute($check_against_res[0]) or die;
      my $old_parent_id;
      my @check_names;
      my $j = 0;
      while (@check_names_res = $check_names_sth->fetchrow_array()) {
         $check_names[$j] = $check_names_res[0];
         $j++;
      }
      
      if (!$trust_sourceref) {
         # creating bounding box
         $coslat = get_coslat($check_against_res[1]);
         $lon_mc2dist = ($dist * ($coord_const / $coslat));
         $lat_mc2dist = ($dist * $coord_const);
         
         
         $maxlat = ($check_against_res[1] + $lat_mc2dist);
         $minlat = ($check_against_res[1] - $lat_mc2dist);
         $maxlon = ($check_against_res[2] + $lon_mc2dist);
         $minlon = ($check_against_res[2] - $lon_mc2dist);
      }
      $debug && timeprint "\nChecking $i, poiid=$check_against_res[0], source=$source, type_id=$check_against_res[3]\n";
      
      if ( defined($latest_poiid) && 
           ($latest_poiid == $check_against_res[0]) ) {
         $same_poiid = 1;
      } else {
         $same_poiid = 0;
      }
      $latest_poiid = $check_against_res[0];
      
      # this is where we do something about the types
      #
      my $type_list;
      if ($trust_sourceref) {
         $debug && dbgprint "Checking of sourceReference: $check_against_res[4]\n";
         $ot_sth->execute($check_against_res[4]) or die ("Statement: $ot_sth->{Statement}\n" .
            "Parameter: $check_against_res[4]\n" .
            "Error string: $ot_sth->errstr");
      } else {
         if ($type_mapping eq 'par') {
            $ot_sth->execute($maxlat, $minlat, $maxlon, $minlon) or die;
         } else {
            $type_list = TypeMappings::get_mapped_types($check_against_res[3]);
            $debug && dbgprint "The types to search for: $type_list\n";
            $debug && dbgprint "OTHER SQL QUERY: $ot_sql\n";
            $debug && dbgprint "coords: $maxlat, $minlat, $maxlon, $minlon\n";
            if ($opt_f eq 'selfCheck') {
               $ot_sth->execute($type_list, $maxlat, $minlat, $maxlon, $minlon, $check_against_res[0]) or die;
            } else {
               $ot_sth->execute($type_list, $maxlat, $minlat, $maxlon, $minlon) or die;
            }
         }

      }
      
      $k = 0;
      $debug && dbgprint "Will check if $check_against_res[0] is a child\n";
      if ( $externalCheck ){
         # Only check child in external checks.
         $old_parent_id = &check_if_child($check_against_res[0]);
         if (defined ($old_parent_id)) {
            $old_parent{$i}[$k] = "$old_parent_id,$check_against_res[0]";
            $debug && dbgprint "  This is a child!\n";
         } 
      }
      if ( !$externalCheck || !defined($old_parent_id) ){
         $parent{$i}[$k] = $check_against_res[0];
         $debug && dbgprint "  This is not a child\n";
      }
      $k = 1;
      
      my $found_name_match = 0;
      
      # go through hits based on coords and type
      #
      while (@ot_res = $ot_sth->fetchrow_array()) {
         
         if ($found_name_match == 0) {
            
            if ($trust_sourceref) {
               $debug && dbgprint "- Match on sourceReference!\n";
               $debug && dbgprint "Will check if $ot_res[0] is a child\n";
               if ( $externalCheck ){
                  $debug && print "Will check if $ot_res[0] is a child\n";
                  $old_parent_id = &check_if_child($ot_res[0]);
                  if (defined $old_parent_id) {
                     $old_parent{$i}[$k] = "$old_parent_id,$ot_res[0]";
                     $debug && dbgprint "This is a child!\n";
                  }
               }
               if ( !$externalCheck || !defined($old_parent_id) ){
                  # e.g. migrate poi static,
                  # found a poi with the same source ref, need to check 
                  # if at least one name matches
                  my $oneNameMatches = 0;
                  $ot_names_sth->execute($ot_res[0]) || die "DB ERROR: $dbh->errstr";
                  while (@ot_names_res = $ot_names_sth->fetchrow_array()) {
                     my $main_string = 
                        decodeAndTrimNameString("utf8", $ot_names_res[0]);
                     foreach my $name (@check_names) {
                        my $test_string = 
                           decodeAndTrimNameString("utf8", $name);
                        if ($oneNameMatches == 0) {	
                           # if the strings match completely
                           if (lc $main_string eq lc $test_string) {
                              $oneNameMatches = 1;
                           }
                        }
                     }
                  }

                  if ( $oneNameMatches ) {
                     $parent{$i}[$k] = $ot_res[0];
                     $debug && dbgprint "This is not a child!\n";
                     $k++;
                  }
               }
               next;
            } else {
               $debug && dbgprint "- Match on coords and type: id=$ot_res[0], typeid=$ot_res[4], source=$ot_res[3]\n";
               $debug && dbgprint "-- Will check names\n";
            }
            
            $ot_names_sth->execute($ot_res[0]) || die "DB ERROR: $dbh->errstr";
            
            while (@ot_names_res = $ot_names_sth->fetchrow_array()) {
               
               foreach my $name (@check_names) {
                  
                  my $similarity = &name_similarity($ot_names_res[0], $name);
                  
                  my $main_string = 
                     decodeAndTrimNameString("utf8", $ot_names_res[0]);
                  my $test_string = 
                     decodeAndTrimNameString("utf8", $name);
                  
                  if ($found_name_match == 0) {	
                     # if the strings match completely
                     if ($main_string =~ /^$test_string$/i) {
                        $debug && dbgprint "We have a duplicate (type 1) (typeid=$ot_res[4]): $name = $ot_names_res[0]\n";
                        if ( $externalCheck ){
                           $debug && dbgprint "Will check if $ot_res[0] is a child\n";
                           $old_parent_id = &check_if_child($ot_res[0]);
                           if (defined $old_parent_id) {
                              $old_parent{$i}[$k]="$old_parent_id,$ot_res[0]";
                              $debug && dbgprint "This is a child!\n";
                           }
                        }
                        if ( !$externalCheck || !defined($old_parent_id) ){
                           $parent{$i}[$k] = $ot_res[0];
                           $debug && dbgprint "This is not a child!\n";
								}
                        $found_name_match = 1;
                        $k++;
                        # if the strings are similar
                     } elsif ($similarity > $default_similarity) {
                        $debug && dbgprint "We have a duplicate (type 2) (typeid=$ot_res[4]):".
                            " $name = $ot_names_res[0]".
                            " (similarity=$similarity)\n";
                        if ( $externalCheck ){
                           $old_parent_id = &check_if_child($ot_res[0]);
                           if (defined $old_parent_id) {
                              $old_parent{$i}[$k] = 
                                  "$old_parent_id,$ot_res[0]";
                              $debug && dbgprint "This is a child!\n";
                           }
                        }
                        if ( !$externalCheck || !defined($old_parent_id) ){
                           $parent{$i}[$k] = $ot_res[0];
                           $debug && dbgprint "This is not a child!\n";
                        }
                        $found_name_match = 1;
                        $k++;
                        # if one string contains all of the other string
                        # will not be used in combination with "par" typemapping
                     } elsif (($main_string =~ /$test_string/i ||
                               $test_string =~ /$main_string/i) && 
                              $type_mapping ne "par") {
                        if (length($main_string) <= 3 || length($test_string) <= 3) {
                           $debug && dbgprint "Warning: could be a faulty duplicate, short string\n";
								}
                        $debug && print "We have a duplicate (type 3) (typeid=$ot_res[4]): $name = $ot_names_res[0]\n";
                        if ( $externalCheck ){
                           $old_parent_id = &check_if_child($ot_res[0]);
                           if (defined $old_parent_id) {
                              $old_parent{$i}[$k] = 
                                  "$old_parent_id,$ot_res[0]";
                              $debug && dbgprint "This is a child!\n";
                           }
                        }
                        if ( !$externalCheck || !defined($old_parent_id) ){
                           $parent{$i}[$k] = $ot_res[0];
                           $debug && dbgprint "This is not a child!\n";
                        }
                        $found_name_match = 1;
                        $k++;
                     } else {
                        $debug && dbgprint "-- (Not a duplicate: $name != $ot_names_res[0])\n";
                     }
                  }
                  # else found_name_match already 1
               }
            }
         }
      }
      if (!defined $parent{$i}[1] || $parent{$i}[1] eq '') {
         $debug && dbgprint "Deleting parent with id $parent{$i}[0] (typeid=$check_against_res[3])\n";
         $duplicate_not_found{$parent{$i}[0]} = 1;
         delete $parent{$i};
      } else {
         # this deletes ids that were in duplicate_not_found-hash
         # but have now been found
         if ($duplicate_not_found{$parent{$i}[0]}) {
            delete $duplicate_not_found{$parent{$i}[0]};
         }
      }
      $i++;
   }
}

#####################################################################
# print info at start 
#####################################################################
sub print_start_info {

print <<XXX;

Main source = $opt_m
Test against source = $opt_s
Country = $opt_c
Type mapping = $type_mapping
Individual poi id = $opt_i
Coordinate radius = $dist
Create parents = $create_parents
Debug printing = $debug
Trusted sourceRef = $trust_sourceref
Similarity has to be over = $default_similarity

XXX

}

#####################################################################
# check script input 
#####################################################################
sub check_mandatory_input {
   if (!defined $source) {
      die "ERROR: test source (-s) has to be defined\n";
   } elsif (!defined $main_source) {
      die "ERROR: main source (-m) has to be defined!\n";
   } elsif (!defined $country) {
      die "ERROR: country (-c) has to be defined!\n";
   }
}

sub check_static_input {
   if (!defined( $country )){
      $country = "";
   }
   if (!defined( $nonMatchFile ) ){
      die "ERROR: migratePOIStatic: non-matching ID file (-n) has to be defined\n";      
   }
   check_mandatory_input();
}


#####################################################################
# migrate POI static IDs
#####################################################################
sub migrate_poi_static {
   if ( testFile( $nonMatchFile ) ){
       my $dateStr = dateAndTime();
       $dateStr =~ s/[ :]/_/g;
       dbgprint "Moving $nonMatchFile to $nonMatchFile.$dateStr";
       `mv $nonMatchFile $nonMatchFile.$dateStr`;
   }
   


   if ( defined ($opt_b) ){
      timeprint "Determine latitude bands to use.";
          # Calculate latitude bands to use for avoiding memory problems.
          my $bandHeight = 30000000;    # MC2 unit band width
      # The tolerance is used for handling POIs on latitude band borders.
      my $tolerance = 0; #int($dist * $coord_const) ;
      dbgprint "   Latitude band height: $bandHeight";
      dbgprint "   Tolerance: $tolerance";
      my $query = "SELECT min(lat), max(lat)".
          " FROM POIMain".
          " WHERE POIMain.source IN ($source)";
      my $sth = $dbh->prepare($query);
      $sth->execute() or die $dbh->errstr . "\n";   
      (my $minLat, my $maxLat) = $sth->fetchrow();
      my %latBands = ();
      my $lat = $minLat;
      while ($lat < $maxLat){
         my $upperLat = $lat + $bandHeight;
         $latBands{$lat} = $upperLat;
         $lat = $upperLat;
      }
      dbgprint "   Number of latitude bands: " . scalar(keys(%latBands));
      
      my $bandNbr = 1;
      foreach my $lowLat (sort(keys %latBands)){
         if ( $bandNbr < $opt_b ){
            dbgprint " Skipping band $bandNbr.";  
            $bandNbr++;
         }
         else {
            if ( -e 'stop' ){
               dbgprint " Exits because told to stop.";
                  exit 1;
            }
            else {
               timeprint "Handling latitude band $bandNbr of ". 
                   scalar(keys(%latBands)).", ".
                   $lowLat."-".$latBands{$lowLat};
               &prepare_static_check( $tolerance, 
                                      $lowLat, 
                                         $latBands{$lowLat} );
               &add_poiids_to_poistatic();      
               # Empty the use hashes to release memory.
               $individual_poi_id = "";
               $number_staticids_found = 0;
               %parent = ();
               %poiid_to_staticid = ();
               %duplicate_not_found = ();
               $bandNbr++;
            }
         }
      }
   }
   else {
      timeprint "Migrates static IDs without latitude bands.";
      &prepare_static_check();
      &add_poiids_to_poistatic();      
   }
}




#####################################################################
# prepare to do POIStatic
#
# Paramters:
# $tolerance The distance in mc2 units that any latitude band is expanded.
#            When this one is given, latitude bans will be used.
# $lowLat    Lower latitude of the latitude band.
# $highLat    Higher latitude of the latitude band.
#
#####################################################################
sub prepare_static_check {
   
   my $latBandCond = "";
   if ( defined $_[0] ){
      my $tolerance = $_[0];
      my $lowLat = $_[1];
      my $highLat = $_[2];
      
      my $lowLatToUse = $lowLat - $tolerance;
      my $highLatToUse = $highLat + $tolerance;
      $latBandCond = " POIMain.lat > $lowLatToUse AND".
          " POIMain.lat < $highLatToUse AND";
   }

   my $countryCond=""; # Condition for country.
   if ( defined($country) && $country ne "" ){
      $countryCond = "POIMain.country = $country AND ";
   }
   
   my $static_sql = "SELECT POIStatic.staticID, " .
       "POIMain.ID " .
       "FROM POIMain left join POIStatic ON " .
       "POIMain.ID = POIStatic.poiID WHERE " .
       $individual_poi_id .
       $latBandCond .
       " $countryCond ". 
       "POIMain.source IN ($source)";

   my $static_sth = $dbh->prepare($static_sql) or die "$dbh->errstr\n";
   timeprint "STATIC QUERY: $static_sql";   
   $static_sth->execute() or die $dbh->errstr . "\n";

   dbgprint "Will try to migrate static IDs for ".$static_sth->rows()," POIs";

   while ((my $static_id, my $tmp_id) = $static_sth->fetchrow()) {
      $debug && print "\nFound id in POIStatic: $tmp_id\n";
      if ( !defined($static_id) || $static_id eq "" ){
         errprint("Static ID missing for $tmp_id");
         #exit 1;
         next;
      }
      else {
         $number_staticids_found++;
         $individual_poi_id = "POIMain.ID IN ($tmp_id) AND ";
         $poiid_to_staticid{$tmp_id} = $static_id;
         # Check doubles for this POI.
         &check_doubles("Check deleted as well");
      }
   }
   # if there are pois with no duplicate, check them again
   # now with coords etc...
   if ($trust_sourceref) {
      dbgprint "Try to find static IDs with doubles check for ". 
          scalar(keys(%duplicate_not_found)) . " POIs.";

      if ((keys %duplicate_not_found) > 0) {
         dbgprint "";
         timeprint "Checking pois where duplicates were not found\n";
         $trust_sourceref = 0;
         my $nbrCountPrints = 100;
         my $div = int( scalar(keys(%duplicate_not_found)) / $nbrCountPrints );
         my $printProgress = 1;
         if ($div < 1 ) { $div = 1; $printProgress = 0; }
         my $poiNbr = 0; my $prints = 0;
         foreach my $not_found_id (keys %duplicate_not_found) {
            $poiNbr += 1;
            if ( $printProgress && (($poiNbr % $div) == 0) ) {
               $prints += 1;
               timeprint " poi nbr $poiNbr ($prints/$nbrCountPrints)";
            }
            $individual_poi_id = "POIMain.ID IN ($not_found_id) AND ";
            # Check doubles for this POI.
            &check_doubles("Check deleted as well");
         }
         $trust_sourceref = 1;
      }
   }
   timeprint "Number of static IDs: $number_staticids_found";
   dbgprint "Size of poiid_to_staticid: ".scalar(keys(%poiid_to_staticid));

} # prepare_static_check

#####################################################################
# insert new ids to poistatic 
#####################################################################
sub insert_to_poistatic {
   my @insert_ids;

   my $countryCond=""; # Condition for country.
   if ( defined($country) && $country ne "" ){
      $countryCond = "POIMain.country = $country AND ";
   }

   my $printAlot = 1;
   if ( defined $opt_q ) {
      $printAlot = 0;
   }
   

   # finds all ids in source that don't exist in POIStatic
   my $select_sql = "select POIMain.ID, POIMain.country ".
       " from POIMain left join POIStatic on POIMain.ID = POIStatic.poiID".
       " where". 
       " $countryCond ".
       " POIMain.source = $source and".
       " POIStatic.staticID is NULL;";

   my $select_sth = $dbh->prepare($select_sql) or die $dbh->errstr;
   my $found_id = 0;
   my %foundByCountry = ();
   $select_sth->execute() or die;
   while ((my $id, my $countryID) = $select_sth->fetchrow()) {
      $found_id++;
      if ( defined $foundByCountry{$countryID} ){
         $foundByCountry{$countryID}++; 
      }
      else {
         $foundByCountry{$countryID}=1;  
      }
      push @insert_ids, $id;
      if ( $printAlot ) {
         dbgprint "ID not in POIStatic: $id\n";
      } else {
         if ( $found_id % 10000 == 0 ){ # Progress print
            dbgprint "So far found $found_id IDs not in POIStatic";
         }
      }
   }
   if (!$found_id) {
      dbgprint "All poiID:s from source $opt_s already in POIStatic!\n";
      return;
   }
   
   foreach my $countryID (sort(keys(%foundByCountry))){
      my $countryName = getCountryNameFromID($dbh, $countryID);
      dbgprint "Nbr POIs of country $countryName($countryID):".
          " $foundByCountry{$countryID}\n";
   }
   dbgprint "";
   
   # This only works if static IDs have already been set, e.g. for all non
   # deleted POIs. Checks source ref of each POI to find any POI with a 
   # static ID and equal source ref, for reusing its static ID.
   # XXX Does not work if all POIs with a specific source ref lack static ID.
   #     In this case, the all get different static IDs.
   my %idsToReuse = (); # Not used anymore.
   #my %idsToReuse = get_static_ids_by_poi_id($source, @insert_ids);
   #foreach my $poiID ( keys(%idsToReuse) ){
   #   dbgprint ("Reusing static ID ".$idsToReuse{$poiID}." for POI ID $poiID");
   #}

   timeprint "In 15 secs script will insert the $found_id new ID:s from source $opt_s.\nPress ctrl-c to skip.\n\n";
   sleep(15);
   dbgprint "Start insert";
   &lock_poistatic();
   my $insert_sql = "INSERT into POIStatic set staticID = ?, poiID = ?";
   my $insert_sth = $dbh->prepare($insert_sql);
   my $staticid;
   my $nbr_inserted_static = 0;
   foreach my $id2 (@insert_ids) {
      
      if ( defined( $idsToReuse{$id2} ) ){
         $staticid = $idsToReuse{$id2}; 
      }
      else {
         $staticid = &get_max_id_from_poistatic();
      }
      $insert_sth->execute($staticid, $id2) or die "Failed query: $insert_sql";
      $debug && dbgprint "staticID $staticid, poiID $id2 inserted!\n";
      $nbr_inserted_static++;
      if ( ($nbr_inserted_static%500000) == 0 ) {
         timeprint "inserted $nbr_inserted_static ids";
      }
   }
   &unlock_poistatic();
   dbgprint "Inserted $nbr_inserted_static static IDs";
   timeprint "Done!\n";
   
}

#####################################################################
# lock POIStatic
#####################################################################
sub lock_poistatic {
   my $lock_query = "LOCK TABLES POIStatic WRITE";
   my $lock_sth = $dbh->prepare($lock_query) or die $dbh->errstr;
   $lock_sth->execute() or die;
}
 
#####################################################################
# unlock POIStatic
#####################################################################
sub unlock_poistatic {
   my $unlock_query = "UNLOCK TABLES";
   my $unlock_sth = $dbh->prepare($unlock_query) or die $dbh->errstr;
   $unlock_sth->execute() or die;
}

#####################################################################
# get max ID from POIStatic
#####################################################################
sub get_max_id_from_poistatic {

   my $new_id;
   my $select_max = "SELECT max(staticID) FROM POIStatic";
   my $select_sth = $dbh->prepare($select_max) or die $dbh->errstr;
   $select_sth->execute() or die;
   while ((my $id) = $select_sth->fetchrow()) {
      $new_id = $id;
      $debug && dbgprint "max id = $id\n";
   }
   $new_id++;
   return($new_id);
}

#####################################################################
# get the static IDs of POIs having the same src ref as the POIs 
# asked for.
#####################################################################

sub get_static_ids_by_poi_id {
   my $source=shift; # The POI source to search.
   my @poiIDs=@_;    # The POI IDs of the POIs to get the static ID of
   # other POIs with equal src ref of.
   
   my %staticIdByPoiId = (); # Result variable.
   
   # Collect the POI IDs to get static IDs for.
   my $idStr = "";
   foreach my $poiID (@poiIDs) {
      $idStr.="$poiID,";
   }
   if (length ($idStr) == 0){
      errprint("Invalid poiIDs prameter");
      die;
   }
   chop $idStr; # Remove last comma.
   
   # Collect all src refs, and sort POIs having a specific src ref.
   dbgprint "get_static_ids_by_poi_id: Collecting src refs";
   my $query = "SELECT ID, sourceReference FROM POIMain".
       " where source = $source and".
       " ID IN ($idStr)";
   my $sth = $dbh->prepare($query) or die $dbh->errstr;
   $sth->execute() or die "Failed: $query";
   # 
   my $srcRefStr = "";
   my %poiIdsBySrcRef = ();
   my $n = 0;
   my $nbrSrcRefs = $sth->rows();
   while ( (my $poiID, my $srcRef) = $sth->fetchrow() ){
      $srcRefStr.="(POIMain.sourceReference=\"$srcRef\")";
      if ( ($n+1) < $nbrSrcRefs ) {
         $srcRefStr.=" OR ";
      }
      push @{$poiIdsBySrcRef{$srcRef}}, $poiID;
      $n += 1;
   }
   if (length ($srcRefStr) == 0){
      # Nothing more to do.
      dbgprint "get_static_ids_by_poi_id: No srcRefs found";
      return %staticIdByPoiId;
   }
   #print "srcRefStr: $srcRefStr\n";

   # Hashes used for checking sanity.
   my %allStatIDs = (); # Key stat ID, value src ref
   my %allSrcRefs = (); # Key src ref, value stat ID.
   #
   # Get all static IDs of the src refs collected above.
   # This query takes about 5 minutes (2006-12-21).
   dbgprint "get_static_ids_by_poi_id: Collecting static IDs";
   $query = "SELECT DISTINCT POIMain.sourceReference, POIStatic.staticID".
       " FROM POIMain inner join POIStatic on POIMain.ID = POIStatic.poiID".
       " WHERE".
       " POIMain.source = $source and". 
       " ($srcRefStr);";
   $sth = $dbh->prepare($query) or die $dbh->errstr;
   $sth->execute() or die "Failed: $query";
   while ( (my $srcRef, my $staticID) = $sth->fetchrow() ){
      
      # Check sanity
      if ( defined ($allStatIDs{$staticID}) ) {
         errprint "Same stat ID: $staticID for multiple src refs: ".
             "$allStatIDs{$staticID}, $srcRef";
         exit 1;
      }
      else {
         $allStatIDs{$staticID}=$srcRef;
      }
      if ( defined ($allSrcRefs{$srcRef}) ) {
         errprint "Same src ref: $srcRef for multiple static IDs: ".
             "$allSrcRefs{$srcRef}, $staticID";
         exit 1;
      }
      else {
         $allSrcRefs{$srcRef}=$staticID;
      }
      
      # Collect static IDs.
      foreach my $poiID (@{$poiIdsBySrcRef{$srcRef}}){
         $staticIdByPoiId{ $poiID } = $staticID;
      }
   }
   dbgprint "staticIdByPoiId = " . scalar(keys %staticIdByPoiId) . "\n";
   return %staticIdByPoiId;
}

#####################################################################
# do similarity check
#####################################################################
sub name_similarity {
	my ($name1, $name2) = @_;
	my $similarity;
   # removing anything in parantheses
   # is not always a good thing...
   #
	# remove anything in parantheses
	#$name1 =~ s/\(.*\)//g;
	#$name2 =~ s/\(.*\)//g;
   my $paranthesematch = "";
   foreach $paranthesematch (@things_in_parantheses) {
      $name1 =~ s/$paranthesematch//gi;
      $name2 =~ s/$paranthesematch//gi;
   }
   # remove single paranthese character
   $name1 =~ s/\(//g;
   $name1 =~ s/\)//g;
   $name2 =~ s/\(//g;
   $name2 =~ s/\)//g;
	# remove hotel/hotell
	$name1 =~ s/hotel+\s*//i;
	$name2 =~ s/hotel+\s*//i;
	# remove space(s) at end of string
	$name1 =~ s/\s+$//;
	$name2 =~ s/\s+$//;
	# remove space(s) at start of string
	$name1 =~ s/^\s+//;
	$name2 =~ s/^\s+//;
	# remove dashes
	$name1 =~ s/-/ /g;
	$name2 =~ s/-/ /g;
	# make space out of slashes
	$name1 =~ s/\// /g;
	$name2 =~ s/\// /g;
	# remove more than one space
	$name1 =~ s/\s\s+/ /g;
	$name2 =~ s/\s\s+/ /g;
	# remove "st. " at start of string
	$name1 =~ s/^st\.\s*//i;
	$name2 =~ s/^st\.\s*//i;

   # remove dr. med. dent.
   # used when comparing schober. don't know if it affects other things in a bad way.
   #$name1 =~ s/dr\.\s*//i;
   #$name2 =~ s/dr\.\s*//i;
   #$name1 =~ s/med\.\s*//i;
   #$name2 =~ s/med\.\s*//i;
   #$name1 =~ s/dent\.\s*//i;
   #$name2 =~ s/dent\.\s*//i;

# debug print of what is tested for similarity
#   print "testing similarity between :$name1: and :$name2:\n";

   $similarity = similarity lc $name1, lc $name2;

	return $similarity;

}

#########################################################################
# sub check_all_parents_common_children
#########################################################################
sub check_all_parents_common_children {
	my %special = ();

	$debug && dbgprint "\nTop of check_all_parents_common_children\n"; 

	foreach my $id (keys %parent) {
		foreach my $id2 (keys %parent) {
			# debug print below is a bit too much :)
			#$debug && print "Check parent $id against parent $id2\n";
			if (($id ne $id2) && ($parent{$id}[1] eq $parent{$id2}[1]) && ($parent{$id}[1] ne '')) {
				if (!defined $special{$id2}) {
					$special{$id} = $id2;
					# when two parents have the same second child, this could mean that there
					# is a duplicate in the test source
					# ie the test source has two pois that are more or less
					# identical
					dbgprint "\nParent $id ($parent{$id}[0]:$parent{$id}[1])has the same " . 
						"second child as parent $id2 ($parent{$id2}[0]:$parent{$id2}[1])!\n";

					# here the least similar/close poi-couple is deleted
					&select_closest_children ($id, $id2);

#					dbgprint "Parent $id2 ($parent{$id2}[0]:$parent{$id2}[1]) " .
#						"will be deleted, and you need to MANUALLY check it later!\n";
#					delete $parent{$id2};
				}
			}
		}
	}
}

#########################################################################
# sub select_closest_children
#########################################################################
sub select_closest_children {

	my ($id1, $id2) = @_;
	my $name;
	my (@coords1, @coords2, @coords3);
	my (@names1, @names2);
	my ($similarity1, $similarity2);
	my ($last_similarity1, $last_similarity2);
	my ($max_similarity1, $max_similarity2);
	my ($diff1, $diff2);
	my ($square1, $square2, $square3);
	my $i = 0;

	my $name_query = "SELECT name FROM POINames WHERE poiID = ? AND " .
				"(type >= 0 AND type <= 1)";
	my $name_sth = $dbh->prepare($name_query);
	my $coord_query = "SELECT lat, lon FROM POIMain WHERE ID = ?";
	my $coord_sth = $dbh->prepare($coord_query);

	$name_sth->execute($parent{$id1}[0]) or die;
	while (($name) = $name_sth->fetchrow()) {
		$names1[$i] = $name;
		$i++;
	}
	$i = 0;
	$name_sth->execute($parent{$id2}[0]) or die;
	while (($name) = $name_sth->fetchrow()) {
		$names2[$i] = $name;
		$i++;
	}
	# checking similarity for first parent
	$last_similarity1 = 0;
	$name_sth->execute($parent{$id1}[1]) or die;
	while (($name) = $name_sth->fetchrow()) {
		foreach my $n1 (@names1) {
			$similarity1 = &name_similarity($n1, $name);
			$debug && dbgprint "  Testing similarity between $n1 and $name result: $similarity1\n";
			if ($similarity1 > $last_similarity1) {
				$max_similarity1 = $similarity1;
			}
			$last_similarity1 = $similarity1;
		}
	}

	# checking similarity for second parent
	$last_similarity2 = 0;
	$name_sth->execute($parent{$id2}[1]) or die;
	while (($name) = $name_sth->fetchrow()) {
		foreach my $n2 (@names2) {
			$similarity2 = &name_similarity($n2, $name);
			$debug && dbgprint "  Testing similarity between $n2 and $name result: $similarity2\n";
			if ($similarity2 > $last_similarity2) {
				$max_similarity2 = $similarity2;
			}
			$last_similarity2 = $similarity2;
		}
	}
	# comparing similarity for the parents
	$debug && dbgprint "  Max similarity for parent $id1 was $max_similarity1\n";
	$debug && dbgprint "  Max similarity for parent $id2 was $max_similarity2\n";
	if ($similarity1 >= $default_similarity || $similarity2 >= $default_similarity) {
		if ($similarity1 >= $default_similarity) {
			$debug && dbgprint "Deleting parent $id2 because of better name similarity of other parent\n";
			delete $parent{$id2};
		} else {
			$debug && dbgprint "Deleting parent $id1 because of better name similarity of other parent\n";
			delete $parent{$id1};
		}
		return 1;
	} else {
		$debug && dbgprint "Need to check geographic similarity\n";
	}
	# start checking geographic distance
	$coord_sth->execute($parent{$id1}[0]) or die;
	(@coords1) = $coord_sth->fetchrow_array();

	$coord_sth->execute($parent{$id2}[0]) or die;
	(@coords2) = $coord_sth->fetchrow_array();

	$coord_sth->execute($parent{$id1}[1]) or die;
	(@coords3) = $coord_sth->fetchrow_array();

	dbgprint "printing coords for btgpssimulator-test ($id1, $id2, main source poi)\n===========================\n";
	print "$coords1[0] $coords1[1]\n";
	print "$coords2[0] $coords2[1]\n";
	print "$coords3[0] $coords3[1]\n";

	# deltalat and deltalon for the distances
	my $deltalat3_1 = ($coords3[0] - $coords1[0]);
	my $deltalon3_1 = ($coords3[1] - $coords1[1]);
	my $deltalat3_2 = ($coords3[0] - $coords2[0]);
	my $deltalon3_2 = ($coords3[1] - $coords2[1]);

	my $square3_1 = (($deltalat3_1 * $deltalat3_1) + ($deltalon3_1 * $deltalon3_1));
	my $square3_2 = (($deltalat3_2 * $deltalat3_2) + ($deltalon3_2 * $deltalon3_2));

	print "square parent $id1: $square3_1 parent $id2: $square3_2\n";
	if ($square3_1 < $square3_2) {
		$debug && dbgprint "Deleting parent $id2 because other parent was closer geographically\n";
		delete $parent{$id2};
	} else {
		$debug && dbgprint "Deleting parent $id1 because other parent was closer geographically\n";
		delete $parent{$id1};
	}
	return 1;
}

#########################################################################
# sub create_parents_poimain
#########################################################################
sub create_parents_poimain {
	my $m = 0;
	my $child_ids;
	my $insert_sth;
	my @coords;
	my $main_lat;
	my $main_lon;
	my $rights;
	my $validfrom;
	my $validto;
	my $insert_query = "INSERT INTO POIMain SET " .
			"source = 38, " . ## 38 is the POI_Parent_Source
			"added = now(), lastModified = now(), " .
			"lat = ?, lon = ?, " .
			"inUse = 0, " .
			"comment = 'This is a parent poi', " .
			"country = $country, " .
			"validFromVersion = ?, " .
			"validToVersion = ?";
	my $coord_sth;
	my $parent_poiid;

# dummy values
	$validfrom = 0;
	$validto = 0;

	
	$debug && dbgprint "\nTop of create_parents_poimain function\n";

	# prepare insert for POIMain
	$insert_sth = $dbh->prepare($insert_query) || die "ERROR: $dbh->errstr\n";

	foreach my $id (keys %parent) {
		$child_ids = "";
		if ($parent{$id}[0] ne '' && $parent{$id}[1] ne '') {

			foreach $m (0 .. $#{$parent{$id} } ) {

				$child_ids .= $parent{$id}[$m] . ",";

			}
		} else {
			$debug && dbgprint "ERROR: one or more children of parent $id did not have id:s ($parent{$id}[0]:$parent{$id}[1]), will not add parent\n";
			next;
		}

		$child_ids =~ s/,$//;

		# check coordinates of main source child
		my $coord_query = "SELECT lat, lon FROM POIMain WHERE " .
			"source = " . $coord_source . " AND ID IN ";
		$coord_query = $coord_query . "(" . $child_ids . ")";
		$debug && dbgprint "Coord query: $coord_query\n";

dbgprint "Will stop here until I get control of children!\n";
next;
		
		$coord_sth = $dbh->prepare($coord_query) || die "ERROR: $dbh->errstr\n";
		$coord_sth->execute() or die;
		($main_lat, $main_lon) = $coord_sth->fetchrow();
		$debug && dbgprint "Main child coords: $main_lat, $main_lon\n";	

		$debug && dbgprint "Insert query: $insert_query\n";
		# insert parent into poi main
		$insert_sth->execute($main_lat, $main_lon, $validfrom, $validto) or die;
		
		$parent_poiid = $insert_sth->{'mysql_insertid'};


		if (defined $parent_poiid && $parent_poiid ne "" && $parent_poiid != 0) {
			$debug && dbgprint "Parent got id: $parent_poiid\n";	

			&create_mychild_info($parent_poiid, $child_ids);

			&create_myparent_info($parent_poiid, $child_ids);

			&add_parent_names ($parent_poiid, $child_ids);
		
			&add_parent_types ($parent_poiid, $child_ids);

			&copy_child_rights($child_ids);

			&create_parent_rights($parent_poiid, $child_ids);

		} else {
			errprint "ERROR: Parent did not get an id!\n";
			errprint "Script exits!\n";
			exit;
		}
	}
}

#########################################################################
# sub check_if_child
#########################################################################
my $child_check_query = "SELECT val FROM POIInfo WHERE poiID = ? " .
    "AND keyID = 59";
my $child_check_sth = $dbh->prepare($child_check_query);
sub check_if_child {
   my ($poiid) = @_;
   my $is_child = 0;
   my $id;
   my $sth;
   # we want to get the poiID of the parent POI if there is one
   #$debug && print "  $select_query\n";
   #$sth = $dbh->prepare($select_query);
   $sth->execute($poiid) or die "Child check error: $dbh->errstr";
   $id = $sth->fetchrow();
   if (defined $id && $id >= 1) {
      return $id;
   } else {
      return undef;
   }
}

#########################################################################
# sub create_mychild_info
#########################################################################
sub create_mychild_info {
	my ($parent_poiid, $child_ids) = @_;

	my $child_sth;
	my $insert_query = "INSERT INTO POIInfo SET " .
		"lang = 31, " .
		"poiID = $parent_poiid, " .
		"keyID = 58, " .
		"val = ?";
	my @children = split(/,/, $child_ids);
	$debug && dbgprint "DEBUG: create_mychild_info\n";
	$child_sth = $dbh->prepare($insert_query);

	foreach my $child (@children) {
		$debug && dbgprint "  INSERT query: \n  $insert_query\n  Child_id: $child\n";
		$child_sth->execute($child) or die;
	}

}

#########################################################################
# sub create_myparent_info
#########################################################################
sub create_myparent_info {
	my ($parent_poiid, $child_ids) = @_;

	my $parent_sth;
	my $insert_query = "INSERT INTO POIInfo SET " .
		"lang = 31, " .
		"poiID = ?, " .
		"keyID = 59, " .
		"val = $parent_poiid";
	my @children = split(/,/, $child_ids);
	$debug && dbgprint "DEBUG: create_myparent_info\n";
	$parent_sth = $dbh->prepare($insert_query);

	foreach my $child (@children) {
		$debug && dbgprint "  INSERT query: \n  $insert_query\n  Child_id: $child\n";
		$parent_sth->execute($child) or die;
	}
}

#########################################################################
# sub create_parent_rights
#########################################################################
sub create_parent_rights {
	my ($parent_poiid, $child_ids) = @_;

	my ($update_sth, $select_sth);
	my $right;
	my @rights;
	my $combined_right;
	my $i = 0;
	my $select_query = "SELECT rights FROM POIMain WHERE " .
		"ID = ?";
	my $update_query = "UPDATE POIMain SET " .
		"rights = ? " .
		"WHERE ID = $parent_poiid";
	my @children = split(/,/, $child_ids);
	$debug && dbgprint "DEBUG: create_parent_rights\n";
	$update_sth = $dbh->prepare($update_query);
	$select_sth = $dbh->prepare($select_query);

	foreach my $child (@children) {
		$debug && dbgprint "  SELECT query: \n  $select_query\n  Child_id: $child\n";
		$select_sth->execute($child) or die;
		$right = $select_sth->fetchrow();
		$debug && dbgprint "  GOT right $i: $right for child $child\n";
		$rights[$i] = $right;
		$i++;
	}
	$debug && dbgprint "  Combining child rights\n";
	# code to combine rights goes here :-)

	# dummy combined right
	$combined_right = "FFFFFFFFFFFFFFFF";

	$debug && dbgprint "  UPDATE query: \n  $update_query\nright: $combined_right\n";
	$update_sth->execute($combined_right) or die;
}

#########################################################################
# sub copy_child_rights
#########################################################################
sub copy_child_rights {
	my ($child_ids) = @_;
	my ($insert_sth, $select_sth);
	my $select_query = "SELECT rights FROM POIMain " .
		"WHERE ID = ?";
	my $insert_query = "INSERT INTO POIInfo SET " .
		"lang = 31, " .
		"poiID = ?, " .
		"keyID = 57, " .
		"val = ?";
	my @children = split(/,/, $child_ids);
	$debug && dbgprint "DEBUG: copy_child_rights\n";
	$insert_sth = $dbh->prepare($insert_query);
	$select_sth = $dbh->prepare($select_query);

	foreach my $child (@children) {
		$debug && dbgprint "  SELECT query: \n  $select_query\n  Child_id: $child\n";
		$select_sth->execute($child) or die;
		my $right = $select_sth->fetchrow();
		$debug && dbgprint "  INSERT query: \n  $insert_query\n  Child_id: $child, value=$right\n";
		$insert_sth->execute($child, $right) or die;
	}
}

#########################################################################
# sub add_parent_names
#########################################################################
sub add_parent_names {
	my ($parent_poiid, $child_ids) = @_;

	my (@res, $type, $lang, $name);
	my ($select_sth, $insert_sth);
	my (%lang_flag);
	my ($source_order);

	# this depends on if we want to use the names from the
	# test source or the main source

	# using main source names
	if ($source >= $main_source) {
	# using test source names
	#if ($source <= $main_source) {
		$source_order = "ASC";
	} else {
		$source_order = "DESC";
	}
	
	my $select_query = "SELECT DISTINCT type, lang, name FROM " .
		"POINames, POIMain WHERE (POIMain.ID = POINames.poiID) AND " .
		"poiID IN ($child_ids) AND " .
		"(type >= 0 AND type <= 1) ORDER BY source $source_order";
	my $insert_query = "INSERT INTO POINames SET " .
		"poiID = $parent_poiid, type = ?, lang = ?, name = ?"; 
	$debug && dbgprint "DEBUG: add_parent_names\n";
	$debug && dbgprint "  SELECT query: \n  $select_query\n";

	$select_sth = $dbh->prepare($select_query);
	$insert_sth = $dbh->prepare($insert_query);
	
	$select_sth->execute() or die;

	while (@res = $select_sth->fetchrow_array()) {
		$type = $res[0];
		$lang = $res[1];
		$name = $res[2];
		# if a name was added as type 0 (officialName) in lang x
		# there should not be any more names of type 0 in lang x
		# but they need to be saved as type 1 (alternativeName)
		if ($type == 0 && $lang_flag{$lang} == 1) {
			$debug && dbgprint "  INSERT query: \n  $insert_query\nName: $name Lang: $lang Type: 1\n";
			$insert_sth->execute(1, $lang, $name) or die;
		} else {
			$lang_flag{$lang} = 1;
			$debug && dbgprint "  INSERT query: \n  $insert_query\nName: $name Lang: $lang Type: $type\n";
			$insert_sth->execute($type, $lang, $name) or die;
		}
	}
}

#########################################################################
# sub add_parent_types
#########################################################################
sub add_parent_types {
	my ($parent_poiid, $child_ids) = @_;

	my ($type);
	my ($select_sth, $insert_sth);
	my $select_query = "SELECT DISTINCT typeID FROM " .
		"POITypes WHERE poiID IN ($child_ids)";
	my $insert_query = "INSERT INTO POITypes SET " .
		"poiID = $parent_poiid, typeID = ?";
	$debug && dbgprint "DEBUG: add_parent_types\n";
	$debug && dbgprint "  SELECT query: \n  $select_query\n";

	$select_sth = $dbh->prepare($select_query);
	$insert_sth = $dbh->prepare($insert_query);

	$select_sth->execute() or die;

	while (($type) = $select_sth->fetchrow()) {
		$debug && dbgprint "  INSERT query: $insert_query\n  Type: $type\n";
		$insert_sth->execute($type) or die;
	}

}


#########################################################################
#########################################################################
sub print_summary {
   timeprint "\n\nRESULTS\n==========================\n\n";
   my $m = 0;
   my $check_source_query = "select source, comment from POIMain where ID = ?";
   # print out new parents!
   foreach my $id (keys %parent) {
      if ($parent{$id}[1] ne '') {
         dbgprint "PARENT $id has children:\n";
         
         foreach $m (0 .. $#{ $parent{$id} } ) {
            dbgprint " $m = $parent{$id}[$m]\n";
         }
      }
   }
   $m = 0;
   # print out "old" parents
   foreach my $id (keys %old_parent) {
      if ($old_parent{$id}[1] ne '') {
         dbgprint "old PARENT $id has children:\n";
         
         foreach $m (0 .. $#{ $old_parent{$id} } ) {
            dbgprint " $m = $old_parent{$id}[$m]\n";
         }
      }
   }
}

#########################################################################
#########################################################################
sub select_internal_double_to_delete {

}

#########################################################################
#########################################################################
sub double_deleted_in_previous_source {
   my $m = 0;
   my $check_source_query = "select source, comment from POIMain where ID = ?";
   my $static_query = "select staticID from POIStatic where poiID = ?";
   my $staticid;
   my $prev_poi_query = "select POIMain.ID, POIMain.deleted from POIMain left join POIStatic on POIMain.ID = POIStatic.poiID where staticID = ? and POIMain.source = $prevSourceId";
   my ($prev_poiid, $prev_poi_deleted, $any_of_prev_was_deleted);

   my $static_sth = $dbh->prepare($static_query);
   my $poi_sth = $dbh->prepare($prev_poi_query);

   foreach my $id (keys %parent) {
      if ($parent{$id}[1] ne '') {
         # reset flag
         $any_of_prev_was_deleted = 0;

         dbgprint "PARENT $id has children:\n";
         
         foreach $m (0 .. $#{ $parent{$id} } ) {
            $static_sth->execute($parent{$id}[$m]);
            $staticid = $static_sth->fetchrow();
            if (defined $staticid) {
               $poi_sth->execute($staticid);
               ($prev_poiid, $prev_poi_deleted) = $poi_sth->fetchrow();
               if ($prev_poi_deleted) {
                  $any_of_prev_was_deleted = 1;
               }
            }
            dbgprint " $m = $parent{$id}[$m] prev id: $prev_poiid deleted: $prev_poi_deleted\n";
         }
         if ($any_of_prev_was_deleted) {

         } else {
            &select_internal_double_to_delete();
         }
      }
   }
   $m = 0;
   # print out "old" parents
   foreach my $id (keys %old_parent) {
      if ($old_parent{$id}[1] ne '') {
         dbgprint "old PARENT $id has children:\n";
         
         foreach $m (0 .. $#{ $old_parent{$id} } ) {
            dbgprint " $m = $old_parent{$id}[$m]\n";
         }
      }
   }
}

#########################################################################
#########################################################################
sub add_poiids_to_poistatic {
   dbgprint "Add poiIDs to POIStatic";

   # When we come here:
   #
   # parent{poiIdx}[0]: old source, POI ID
   # parent{poiIdx}[1]: new source, POI ID
   #
   # parent{poiIdx}[2...]: new source ID doubles found 

   # Handle double new POI IDs.
   # 1) Find all parent{poiIdx}[2...] new POI IDs and put them in a hash
   # 2) Check if these existi in any of the parent{poiIdx}[1]
   # 3) Order all old IDs with the same new ID with the ones matching fewest
   #    IDs first.
   # 4) Give the ones with few new IDs a new ID first, until new IDs run out.
   #    make sure not to give the same new ID to more than one old ID POI.
   #my %staticid_to_new_poiid;
   my %notAddedStaticMatches = ();
   my %new_poidid_to_static = ();
   my $nbrStaticsAdded = 0;

   foreach my $id (keys %parent) {
      if (exists $poiid_to_staticid{$parent{$id}[0]}) {
         #print "Found static id for poi id $parent{$id}[0], which is $poiid_to_staticid{$parent{$id}[0]}\n";
         #print "then the new poi id should be inserted: $parent{$id}[1]\n";
         #$staticid_to_new_poiid{$poiid_to_staticid{$parent{$id}[0]}} = $parent{$id}[1];

         # Find a not used new sorce POI ID to set this static to.
         my $matchIdx = 1;
         while ( $matchIdx < scalar( @{$parent{$id}} )  &&
                 defined($new_poidid_to_static{$parent{$id}[$matchIdx]}) ){
            $matchIdx++;
         }
         # Set this static ID to the not used new POI ID.
         if ( $matchIdx < scalar( @{$parent{$id}} ) ){
            $new_poidid_to_static{$parent{$id}[$matchIdx]} = 
                $poiid_to_staticid{$parent{$id}[0]};
            $nbrStaticsAdded++;
         }
         else {
            # This static ID has not been added to a POI. It's OK if we had 
            # more than one old POI with this static. In that case it has been
            # added to a new POI already, but we don't know that here.
            $notAddedStaticMatches{$parent{$id}[0]} =
                $poiid_to_staticid{$parent{$id}[0]};
         }
      }
   }
   #if ($number_staticids_found == keys %staticid_to_new_poiid) {
   #   print "\nHave found new ids for all static ids\n";
   #} else {
   #   print "\nSome ids missing\n";
   #   my $num_found = keys %staticid_to_new_poiid;
   #   print "Had $number_staticids_found static ids, but only $num_found to update\n\n";
   #}

   dbgprint "Checking which not added static IDs was added to another new".
       " POI.";
   my $query = "SELECT * ".
       " FROM POIMain JOIN POIStatic ON POIMain.ID=POIStatic.poiID".
       " WHERE POIStatic.staticID = ? AND POIMain.source = $main_source".
       " LIMIT 1";
   my $statSth = $dbh->prepare($query) || die "Prepare of $query failed";
   my $nbrStaticAddedAnyway = 0;
   foreach my $notAddedID (keys %notAddedStaticMatches) {
      $statSth->execute($notAddedStaticMatches{$notAddedID});
      if ( $statSth->rows() > 0 ){
         # This static ID has been added to another POI, so it is OK.
         
         delete($notAddedStaticMatches{$notAddedID});
         $nbrStaticAddedAnyway++;
      }
   }
   dbgprint "Number of not added static IDs added to another new POI: ".
       $nbrStaticAddedAnyway;
   


   # Print some statistics.
   my $nbrStaticInOldSource = scalar(keys(%poiid_to_staticid));
   dbgprint "";
   dbgprint "Statistics:";
   dbgprint "Matched $nbrStaticsAdded of $nbrStaticInOldSource static IDs in".
       " old source to POIs of new source.";
   dbgprint "Did not match ". ($nbrStaticInOldSource-$nbrStaticsAdded) .
       " static IDs (some of these may be doubles)";
   dbgprint "";
   dbgprint "Sanity checks:";
   dbgprint "new_poidid_to_static: ".scalar(keys(%new_poidid_to_static));
   dbgprint "Number of unique old source static IDs not added because new".
       " POI to add to".
       " already had a static ID: " . scalar(keys(%notAddedStaticMatches));
   dbgprint "Number of old source POIs with no duplicate in new POIs source".
       " found: ". scalar(keys(%duplicate_not_found));
   dbgprint "";
   dbgprint "Counting all static IDs in old source...";
   dbgprint "Total number of static IDs in old source: ".    
       getNbrStaticIDsOfSourceID($dbh, $source);
   dbgprint "";

   
   if ( $opt_f eq "migratePOIStatic"){
      open (NON_MATCH_FILE, ">> $nonMatchFile");
   }
   foreach my $notfound_id (keys %duplicate_not_found) {
      dbgprint "Duplicate not found for id: $notfound_id";
      if ( $opt_f eq "migratePOIStatic"){
         print NON_MATCH_FILE "$notfound_id\n";
      }
   }
   foreach my $notfound_id (keys %notAddedStaticMatches) {
      dbgprint "Could not add static ID of: $notfound_id";
      if ( $opt_f eq "migratePOIStatic"){
         print NON_MATCH_FILE "$notfound_id\n";
      }
   }
   if ( $opt_f eq "migratePOIStatic"){
      close (NON_MATCH_FILE);
   }

   dbgprint "\nIn 15 secs script will insert the $number_staticids_found " .
            "(= " . scalar( keys(%new_poidid_to_static) ) . ") " .
            "ID:s from source $opt_s.\nPress ctrl-c to skip.\n\n";
   sleep(15);
   dbgprint "Start insert";



   #my $nbr_inserted_static = 0;
   #foreach my $update_id (keys %staticid_to_new_poiid) {
   #   $update_sth->execute($staticid_to_new_poiid{$update_id}, $update_id) or die $dbh->errstr . "\n";
   #   #print "INSERT INTO POIStatic set poiID = $staticid_to_new_poiid{$update_id}, staticID = $update_id;\n";
   #   $nbr_inserted_static++;
   #}

   my $update_sql = "INSERT INTO POIStatic SET poiID = ?, staticID = ?";
   my $update_sth = $dbh->prepare($update_sql) or die ;


   my $nbr_inserted_static = 0;
   foreach my $newPoiID (keys %new_poidid_to_static) {
      $update_sth->execute($newPoiID, $new_poidid_to_static{$newPoiID}) or
          dbgprint "Failed INSERT, for ".$newPoiID." - ".
          $new_poidid_to_static{$newPoiID}, 
          $nbr_inserted_static--;
      $nbr_inserted_static++;
      if ( ($nbr_inserted_static%500000) == 0 ) {
         timeprint "inserted $nbr_inserted_static ids";
      }
   }
   dbgprint "Inserted $nbr_inserted_static static IDs";
}


#########################################################################
#########################################################################
sub get_coslat {
        ($lat) = @_;
        return cos($invRadianFactor * ( $lat ) );
}

#########################################################################
#########################################################################
sub decodeAndTrimNameString {
   my $decodeString = $_[0]; # typically "utf8"
   my $name = $_[1];
   
   $name =~ s/\*/\\*/g;
   my $resultName = decode("$decodeString", $name);
   
   # the changes below were added
   # when checking doubles for
   # teleatlas canada new against old
   #$resultName =~ s/\(.*\)//g;
   $resultName =~ s/\(//g;
   $resultName =~ s/\)//g;
   $resultName =~ s/\?//g;
   # removing all metacharacters
   # to avoid crashes
   $resultName =~ s/\[//g;
   $resultName =~ s/\]//g;
   $resultName =~ s/\{//g;
   $resultName =~ s/\}//g;
   $resultName =~ s/\$//g;
   $resultName =~ s/\^//g;
   $resultName =~ s/\.//g;
   $resultName =~ s/\|//g;
   $resultName =~ s/\+//g;

   return $resultName;

}


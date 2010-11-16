#!/usr/bin/perl
#
#
# Class that represents POIs in perl.
# A POI object can hold all attributes that are present for POIs in WASP db.
# Contains methods for creating POI objects from WASP db, adding POI objects
# to WASP db, and misc methods that updates POI object attributes.

package POIObject;

# Todo: get BASEGENFILESPATH into the use-lib- string possible?
use lib ".";
use PerlWASPTools;
use PerlTools;
use GDFPoiFeatureCodes;

use strict;

#########################################################
# Private attributes
# POIMain
#my $self->{m_id};
#my $self->{m_source};
#my $self->{m_added};
#my $self->{m_lastModified};
#my $self->{m_lat};
#my $self->{m_lon};
#my $self->{m_inUse};
#my $self->{m_deleted};
#my $self->{m_sourceReference};
#my $self->{m_comment};
#my $self->{m_country};
#my $self->{m_validFromVersion};
#my $self->{m_validToVersion};
#my $self->{m_gdfFeatureCode};

# POIEntryPoints, POIInfo, POINames, POITypes
#my @{$self->{m_entryPoints}};   # array with ep strings "lat,lon"
#my @{$self->{m_poiInfo}};       # array with info strings "keyID<¡>lang<¡>val"
#my @{$self->{m_poiNames}};      # array with name strings "name<¡>type<¡>lang"
#my @{$self->{m_poiTypes}};      # array with type IDs

#my $dbh;                # Connection to db

# Other variabels needed
# The inverted exclamation mark within less-than and greater-than signs
my $fieldSep = "<¡>";   # Separator in array strings, 
# The inverted question mark within less-than and greater-than signs
my $gdfNameSep = "<¿>";    # Sep in GDF POI file
# The plus/minus sign within less-than and greater-than signs
my $gdfNameSubSep = "<±>"; # Sep in GDF POI file
my $coordSep = ",";     # Separator in latlon strings
my $m_language;         # Language for name strings and poi info values
my $geocFieldSep = ";"; # Separator in file for geocoding
# Hash wiht IDs of languages
#   key: Language name or abbreviation
#   value: Language database ID.
my %m_nameLang = ();
# Hash with IDs of nameTypes
#   key: nameType name or abbreviation
#   value: NameType database ID.
my %m_nameTypes = ();
# Hash with keyIDs of poi info type strings from GDF POI file
#   key: poi info type string
#   value: kayID database ID
my %m_gdfPOIInfoTypes = ();


# Insert statement handles
my $sthInsertMain;
my $sthInsertMainWRights;
my $sthInsertExtID;
my $sthInsertEtrPts;
my $sthInsertInfo;
my $sthInsertInfoDynamic;
my $sthInsertName;
my $sthInsertType;
my $sthInsertCategory;


# get topRegionName from countryID (english name in country_ar.xml file)
my %g_topRegionsFromCountryID = ();
$g_topRegionsFromCountryID{  0 } = "united kingdom"; # England
$g_topRegionsFromCountryID{  1 } = "sweden"; # Sweden
$g_topRegionsFromCountryID{  2 } = "germany"; # Germany
$g_topRegionsFromCountryID{  3 } = "denmark"; # Denmark
$g_topRegionsFromCountryID{  4 } = "finland"; # Finland
$g_topRegionsFromCountryID{  5 } = "norway"; # Norway
$g_topRegionsFromCountryID{  6 } = "belgium"; # Belgium
$g_topRegionsFromCountryID{  7 } = "netherlands"; # Netherlands
$g_topRegionsFromCountryID{  8 } = "luxembourg"; # Luxembourg
$g_topRegionsFromCountryID{  9 } = "xxx"; # USA  - need state name
$g_topRegionsFromCountryID{ 10 } = "switzerland"; # Switzerland
$g_topRegionsFromCountryID{ 11 } = "austria"; # Austria
$g_topRegionsFromCountryID{ 12 } = "france"; # France
$g_topRegionsFromCountryID{ 13 } = "spain"; # Spain
$g_topRegionsFromCountryID{ 14 } = "andorra"; # Andorra
$g_topRegionsFromCountryID{ 15 } = "liechtenstein"; # Liechtenstein
$g_topRegionsFromCountryID{ 16 } = "italy"; # Italy
$g_topRegionsFromCountryID{ 17 } = "monaco"; # Monaco
$g_topRegionsFromCountryID{ 18 } = "ireland"; # Ireland
$g_topRegionsFromCountryID{ 19 } = "portugal"; # Portugal
$g_topRegionsFromCountryID{ 20 } = "canada"; # Canada
$g_topRegionsFromCountryID{ 21 } = "hungary"; # Hungary
$g_topRegionsFromCountryID{ 22 } = "czech republic"; # Czech Republic
$g_topRegionsFromCountryID{ 23 } = "poland"; # Poland
$g_topRegionsFromCountryID{ 24 } = "greece"; # Greece
$g_topRegionsFromCountryID{ 25 } = "israel"; # Israel
$g_topRegionsFromCountryID{ 26 } = "brazil"; # Brazil
$g_topRegionsFromCountryID{ 27 } = "slovakia"; # Slovakia
$g_topRegionsFromCountryID{ 28 } = "russia"; # Russia
$g_topRegionsFromCountryID{ 29 } = "turkey"; # Turkey
$g_topRegionsFromCountryID{ 30 } = "slovenia"; # Slovenia
$g_topRegionsFromCountryID{ 31 } = "xxx"; # Bulgaria
$g_topRegionsFromCountryID{ 32 } = "xxx"; # Romania
$g_topRegionsFromCountryID{ 33 } = "xxx"; # Ukraine
$g_topRegionsFromCountryID{ 34 } = "xxx"; # Serbia and Montenegro
$g_topRegionsFromCountryID{ 35 } = "croatia"; # Croatia
$g_topRegionsFromCountryID{ 36 } = "xxx"; # Bosnia Herzegovina
$g_topRegionsFromCountryID{ 37 } = "xxx"; # Moldova
$g_topRegionsFromCountryID{ 38 } = "xxx"; # FYR Macedonia
$g_topRegionsFromCountryID{ 39 } = "xxx"; # Estonia
$g_topRegionsFromCountryID{ 40 } = "xxx"; # Latvia
$g_topRegionsFromCountryID{ 41 } = "xxx"; # Lithuania
$g_topRegionsFromCountryID{ 42 } = "xxx"; # Belarus
$g_topRegionsFromCountryID{ 43 } = "xxx"; # Malta
$g_topRegionsFromCountryID{ 44 } = "xxx"; # Cyprus
$g_topRegionsFromCountryID{ 45 } = "xxx"; # Iceland
$g_topRegionsFromCountryID{ 46 } = "hong kong"; # Hong Kong
$g_topRegionsFromCountryID{ 47 } = "singapore"; # Singapore
$g_topRegionsFromCountryID{ 48 } = "australia"; # Australia 
$g_topRegionsFromCountryID{ 49 } = "united arab emirates"; # UAE 
$g_topRegionsFromCountryID{ 95 } = "egypt"; # Egypt
$g_topRegionsFromCountryID{ 120 } = "indonesia"; # Indonesia 
$g_topRegionsFromCountryID{ 130 } = "kuwait"; # Kuwait 
$g_topRegionsFromCountryID{ 137 } = "macao"; # Macao 
$g_topRegionsFromCountryID{ 140 } = "malaysia"; # Malaysia 
$g_topRegionsFromCountryID{ 187 } = "saudi arabia"; # Saudi Arabia
$g_topRegionsFromCountryID{ 193 } = "south africa"; # South Africa
$g_topRegionsFromCountryID{ 201 } = "taiwan"; # Taiwan
$g_topRegionsFromCountryID{ 204 } = "thailand"; # Thailand 

# get topRegionName for USA states (POI info)
my %g_topRegionsFromStateCode = ();
$g_topRegionsFromStateCode{ "AK" } = "alaska";
$g_topRegionsFromStateCode{ "AL" } = "alabama";
$g_topRegionsFromStateCode{ "AR" } = "arkansas";
$g_topRegionsFromStateCode{ "AZ" } = "arizona";
$g_topRegionsFromStateCode{ "CA" } = "california";
$g_topRegionsFromStateCode{ "CO" } = "colorado";
$g_topRegionsFromStateCode{ "CT" } = "connecticut";
$g_topRegionsFromStateCode{ "DC" } = "district of columbia";
$g_topRegionsFromStateCode{ "DE" } = "delaware";
$g_topRegionsFromStateCode{ "FL" } = "florida";
$g_topRegionsFromStateCode{ "GA" } = "georgia";
$g_topRegionsFromStateCode{ "GU" } = "guam";
$g_topRegionsFromStateCode{ "HI" } = "hawaii";
$g_topRegionsFromStateCode{ "IA" } = "iowa";
$g_topRegionsFromStateCode{ "ID" } = "idaho";
$g_topRegionsFromStateCode{ "IL" } = "illinois";
$g_topRegionsFromStateCode{ "IN" } = "indiana";
$g_topRegionsFromStateCode{ "KS" } = "kansas";
$g_topRegionsFromStateCode{ "KY" } = "kentucky";
$g_topRegionsFromStateCode{ "LA" } = "louisiana";
$g_topRegionsFromStateCode{ "MA" } = "massachusetts";
$g_topRegionsFromStateCode{ "MD" } = "maryland";
$g_topRegionsFromStateCode{ "ME" } = "maine";
$g_topRegionsFromStateCode{ "MI" } = "michigan";
$g_topRegionsFromStateCode{ "MN" } = "minnesota";
$g_topRegionsFromStateCode{ "MO" } = "missouri";
$g_topRegionsFromStateCode{ "MS" } = "mississippi";
$g_topRegionsFromStateCode{ "MT" } = "montana";
$g_topRegionsFromStateCode{ "NC" } = "north carolina";
$g_topRegionsFromStateCode{ "ND" } = "north dakota";
$g_topRegionsFromStateCode{ "NE" } = "nebraska";
$g_topRegionsFromStateCode{ "NH" } = "new hampshire";
$g_topRegionsFromStateCode{ "NJ" } = "new jersey";
$g_topRegionsFromStateCode{ "NM" } = "new mexico";
$g_topRegionsFromStateCode{ "NV" } = "nevada";
$g_topRegionsFromStateCode{ "NY" } = "new york";
$g_topRegionsFromStateCode{ "OH" } = "ohio";
$g_topRegionsFromStateCode{ "OK" } = "oklahoma";
$g_topRegionsFromStateCode{ "OR" } = "oregon";
$g_topRegionsFromStateCode{ "PA" } = "pennsylvania";
$g_topRegionsFromStateCode{ "PR" } = "puerto rico";
$g_topRegionsFromStateCode{ "RI" } = "rhode island";
$g_topRegionsFromStateCode{ "SC" } = "south carolina";
$g_topRegionsFromStateCode{ "SD" } = "south dakota";
$g_topRegionsFromStateCode{ "TN" } = "tennessee";
$g_topRegionsFromStateCode{ "TX" } = "texas";
$g_topRegionsFromStateCode{ "UT" } = "utah";
$g_topRegionsFromStateCode{ "VA" } = "virginia";
$g_topRegionsFromStateCode{ "VT" } = "vermont";
$g_topRegionsFromStateCode{ "WA" } = "washington";
$g_topRegionsFromStateCode{ "WI" } = "wisconsin";
$g_topRegionsFromStateCode{ "WV" } = "west virginia";
$g_topRegionsFromStateCode{ "WY" } = "wyoming";

#########################################################
# Public methods
# Create empty POI object, possible to set id
sub newObject {
   my $poiID = 2147483647;
   if ( defined( $_[1]) ) {
      $poiID = $_[1];
   }
   my $self = {};
   #$self = \$self;

   # Scalar values
   $self->{m_id} = $poiID;
   undef $self->{m_source};
   undef $self->{m_added};
   undef $self->{m_lastModified};
   undef $self->{m_lat};
   undef $self->{m_lon};
   undef $self->{m_inUse};
   undef $self->{m_deleted};
   undef $self->{m_sourceReference};
   undef $self->{m_comment};
   undef $self->{m_country};
   undef $self->{m_validFromVersion};
   undef $self->{m_validToVersion};
   undef $self->{m_rights};
   undef $self->{m_gdfFeatureCode};
   undef $self->{m_noValidationByDb};
   undef $self->{m_allowNoPosInDb}; # If this one is true, it is OK to add a 
                                    # POI witout symbol coordinate.
   $self->{bestTypeFromRailWayStationType} = 0;

   # Array references
   undef $self->{m_entryPoints};
   undef $self->{m_poiInfo};
   undef $self->{m_poiNames};
   undef $self->{m_poiTypes};
   undef $self->{m_poiCategories};

   # Object references
   undef $self->{m_dbh};
   undef $self->{m_artificial};
   undef $self->{m_externalIdentifier};
   
   #splice @m_entryPoints;
   #splice @m_poiInfo;
   #splice @m_poiNames;
   #splice @m_poiTypes;
   
   return bless $self;
}


# Set dbh
sub setDBH {
   my $self = $_[0];
   if ( defined($_[1]) ) {
       $self->{m_dbh} = $_[1];
       return 1;
   }
   else {
       return 0;
   }
}




# The field separators
sub getFieldSeparator {
   return $fieldSep;
}
sub getCoordinateSeparator {
   return $coordSep;
}

# Set the default language used for reading SPIF
sub setDefaultLang {
   my $self = $_[0];
   if ( defined($_[1]) ) {
       $self->{m_language}=$_[1];
       return 1;
   }
   else {
       return 0;
   }
}

# Create this POI from WASP db
sub readFromWASP {
   my $self = $_[0];

   # Can not create POI without a valid id
   if ( ! defined($self->{m_id}) ) {
      return;
   }

   if ( ! defined($self->{m_dbh}) ) {
      print " no connection to database! - need to set DBH\n";
      return;
   }

   # POIMain
   my $mainQuery = "SELECT ID, source, added, lastModified, " .
         "lat, lon, inUse, deleted, sourceReference, comment, " .
         "country, validFromVersion, validToVersion, rights " .
         "FROM POIMain WHERE id = $self->{m_id};";
   my $sth = $self->{m_dbh}->prepare( $mainQuery );
   $sth->execute;
   ( $self->{m_id}, $self->{m_source}, $self->{m_added}, 
     $self->{m_lastModified}, $self->{m_lat}, $self->{m_lon},
     $self->{m_inUse}, $self->{m_deleted}, $self->{m_sourceReference}, $self->{m_comment}, 
     $self->{m_country}, $self->{m_validFromVersion}, 
     $self->{m_validToVersion}, $self->{m_rights} ) = $sth->fetchrow();

   # POIEntryPoints
   my $epQuery = "SELECT * FROM POIEntryPoints WHERE poiID = $self->{m_id};";
   $sth = $self->{m_dbh}->prepare( $epQuery );
   $sth->execute;
   my $epID = ""; my $epPoiID = ""; my $epLat = ""; my $epLon = "";
   my $count = 0;
   while (  ( $epID, $epPoiID, $epLat, $epLon ) = $sth->fetchrow() ) {
      # array with ep strings "lat,lon"
      my $epString = "$epLat$coordSep$epLon";
      push @{$self->{m_entryPoints}}, $epString;
      $count += 1;
   }
   
   # POIInfo
   my $infoQuery = "SELECT keyID, val, lang FROM POIInfo " .
       "WHERE poiID = $self->{m_id};";
   my $dynInfoQuery = "SELECT keyID, val, lang FROM POIInfoDynamic " .
       "WHERE poiID = $self->{m_id};";
   $sth = $self->{m_dbh}->prepare( $infoQuery );
   $sth->execute;
   my $keyID = ""; my $val = ""; my $infoLang = "";
   my $infoString = "";
   $count = 0;
   while ( ( $keyID, $val, $infoLang ) = $sth->fetchrow() ) {
      # array with info strings "keyID<¡>lang<¡>val"
      $infoString = "$keyID$fieldSep$infoLang$fieldSep$val";
      push @{$self->{m_poiInfo}}, $infoString;
      $count += 1;
   }
   $sth = $self->{m_dbh}->prepare( $dynInfoQuery );
   $sth->execute;
   $keyID = ""; $val = ""; $infoLang = "";
   while ( ( $keyID, $val, $infoLang ) = $sth->fetchrow() ) {
      # array with info strings "keyID<¡>lang<¡>val"
      $infoString = "$keyID$fieldSep$infoLang$fieldSep$val";
      push @{$self->{m_poiInfo}}, $infoString;
      $count += 1;
   }
   
   # POINames
   my $nameQuery = "SELECT type, lang, name FROM POINames WHERE poiID = $self->{m_id};";
   $sth = $self->{m_dbh}->prepare( $nameQuery );
   $sth->execute;
   my $type = ""; my $nameLang = ""; my $name = "";
   $count = 0;
   while ( ( $type, $nameLang, $name ) = $sth->fetchrow() ) {
      # array with name strings "name<¡>type<¡>lang"
      my $nameString = "$name$fieldSep$type$fieldSep$nameLang";
      push @{$self->{m_poiNames}}, $nameString;
      $count += 1;
   }
   
   # POITypes
   my $typeQuery = "SELECT typeID from POITypes WHERE poiID = $self->{m_id};";
   $sth = $self->{m_dbh}->prepare( $typeQuery );
   $sth->execute;
   my $poiTypeID = "";
   $count = 0;
   while (  ( $poiTypeID ) = $sth->fetchrow() ) {
      push @{$self->{m_poiTypes}}, $poiTypeID;
      $count += 1;
   }
   
   # POICategories
   my $categoryQuery = "SELECT catID from POICategories WHERE poiID = $self->{m_id};";
   $sth = $self->{m_dbh}->prepare( $categoryQuery );
   $sth->execute;
   my $poiCategoryID = "";
   $count = 0;
   while (  ( $poiCategoryID ) = $sth->fetchrow() ) {
      push @{$self->{m_poiCategories}}, $poiCategoryID;
      $count += 1;
   }
   
}

# Add one attribute to myself from the GDF POI file
# Example of GDF POI file line
#  YYY-1879048197-1: 372<¡>5<¡>300000002<¡>7379<¡>city centre<¡>11<¡>
# Inparam to this function is not the complete line,
# the poi mcm item id has been removed
#  1: 372<¡>5<¡>300000002<¡>7379<¡>city centre<¡>11<¡>
sub myAddGDFPOIAttribute {
   my $self = $_[0];
   my $attrLine = $_[1];
   my $poiItemID = $_[2];
   #print " attr: '$attrLine'\n";
   
   my $retVal = 1;
   ( my $recordType, my $attributes ) = split( ": ", $attrLine, 2 );

   if ( $recordType == 1 ) {
      # Dataset;section;featureID;featureCode;poi-type;poi-type-id;
      (my $dataset, my $section, my $featureID, 
        my $gdfFeatureCode, my $poiTypeStr, my $poiTypeID)
         = split ("$fieldSep", $attributes);
      my $srcRef = "$dataset:$section:$featureID";
      if ( ! $self->setSourceReference($srcRef) ) {
         $retVal = 0; errprint "Problem adding srcRef attribute.";
      }
      my $comment = "GDF featureCode=$gdfFeatureCode";
      if ( ! $self->setComment($comment)  ) {
         $retVal = 0; errprint "Problem adding comment attribute.";
      }
      if ( ! $self->setGdfFeatureCode($gdfFeatureCode) ) {
         $retVal = 0; errprint "Problem adding gdf feature code.";
      }
      my ($tmpTypeID, $tmpCatID) = get_type_and_cat_from_gdf_feature_code($gdfFeatureCode);
      if ( ! $self->addPOIType($poiTypeID) ) {
         $retVal = 0; errprint "Problem adding poi type attribute.";
      }
      if (defined $tmpCatID) {
         if ( ! $self->addCategory($tmpCatID) ) {
            $retVal = 0; errprint "Problem adding poi category id.";
         }
      }
   }
   
   elsif ( $recordType == 2 ) {
      # country;allnames;symbolLat;symbolLon;
      (my $countryStr, my $allNames, my $symbolLat, my $symbolLon)
         = split ("$fieldSep", $attributes);
      
      # country = english string from StringTable
      my $countryID = getCountryIDFromName($self->{m_dbh}, $countryStr);
      if ( ! defined $countryID ) {
         $retVal = 0; errprint "No countryID for countryStr $countryStr.";
      } else {
         if ( ! $self->setCountry($countryID) ) {
            $retVal = 0; errprint "Problem adding countryID attribute.";
         }
      }
      if ( ! $self->setSymbolCoordinate($symbolLat, $symbolLon) ) {
         $retVal = 0; errprint "Problem adding symbol coord attribute.";
      }
      if ( ! $self->myAddGDFPOINames($allNames, $poiItemID) ) {
         $retVal = 0; errprint "Problem adding name attribute.";
      }
      
   }
   
   elsif ( $recordType == 3 ) {
      # entryLat;entryLon
      (my $entryLat, my $entryLon) = split ("$fieldSep", $attributes);
      if ( ! $self->addEntryPoint($entryLat, $entryLon) ) {
         $retVal = 0; errprint "Problem adding entry point attribute.";
      }
   }
   
   elsif ( $recordType == 4 ) {
      # Additional information, e.g. display class for city centers or
      # type of placeOfWorship
      # infoType;infoValue
      (my $infoType, my $infoValue) = split ("$fieldSep", $attributes);

      if ( $infoType eq "CITY_CENTRE_DISPLAY_CLASS" ) {
         if ( ! $self->myAddGDFPOIInfo(
                      $infoType, $infoValue, $poiItemID) ) {
            $retVal = 0; errprint "Problem adding poi info attribute.";
         }
      }
      elsif ( $infoType eq "PLACE_OF_WORSHIP_TYPE" ) {
         # The poiType 82=placeOfWorship was set reading recordType 1
         # it must be replaced
         if ( $self->hasPOIType(82) and $self->removePOIType(82) ) {
            my $poiTypeID = 82;
            if ( $infoValue == 0 ){ # no type
               $poiTypeID = 82;
            } elsif ( $infoValue == 1 ){ # church
               $poiTypeID = 96;
            } elsif ( $infoValue == 2 ){ # mosque
               $poiTypeID = 97;
            } elsif ( $infoValue == 3 ){ # synagogue
               $poiTypeID = 98;
            } elsif ( $infoValue ==  4){ # others
               $poiTypeID = 82;
            }
            if ( ! $self->addPOIType($poiTypeID) ) {
               $retVal = 0; errprint "Problem adding pow poi type.";
            }
         }
      }
      elsif ( $infoType eq "PUBLIC_TRANSPORT_STOP_TYPE" ) {
         if ( $self->hasPOIType(62) ){
            # Noting set yet, set poi type from this attribute.
            $self->removePOIType(62); # remove unknown type

            my $poiTypeID = 95; # invalid poi type
            if ( $infoValue == 1 ){
               # Bus stop
               $poiTypeID = 103;
            }
            elsif ( $infoValue == 2 ){
               # Tram stop
               $poiTypeID = 53;
            }
            elsif ( $infoValue == 3 ){
               # Taxi stop
               $poiTypeID = 104;  
            }
            if ( $poiTypeID != 95 ){
               $self->addPOIType($poiTypeID);
            }
            else {
               $self->dumpInfo();
               die "Strange value on public transportation type: $infoValue";
            }
         }
      }
      elsif ( $infoType eq "BUS_STOP_TYPE" ) {
         if ( $self->hasPOIType(62) || $self->hasPOIType(103) ){
            $self->removePOIType(62); # remove unknown type
            $self->removePOIType(103); # remove bus stop type

            my $poiTypeID = 95; # invalid poi type
            if ( $infoValue == 1 ){
               $poiTypeID = 7; # Bus station
            }
            elsif ( $infoValue == 2 ){
               $poiTypeID = 103; # Bus stop
            }
            if ( $poiTypeID != 95 ){
               $self->addPOIType($poiTypeID);
            }
            else {
               $self->dumpInfo();
               die "Strange value on bus stop type: $infoValue";
            }
         }
         else {
            $self->dumpInfo();
            die "Bus stop type on strange poi type.";
         }
      }
      elsif ( $infoType eq "RAILWAY_STATION_TYPE" ) {
         # The poiType 36=railwayStation should be
         # more detailed, using the infoValue

         # Clear all raylway station types.
         $self->removePOIType(36); # railway station
         $self->removePOIType(14); # commuter rail
         $self->removePOIType(99); # subway

         # Calculate the POI type.
         my $poiTypeID = 36;
         if ( ($infoValue == 11) or ($infoValue == 12) ) { 
            # 11=international railway station 
            # 12=national railway station
            $poiTypeID = 36;
         } elsif ( $infoValue == 13) { 
            # 13=(sub) urban train station 
            $poiTypeID = 14; # commuterRail
         } elsif ( $infoValue == 3) { 
            # 3=underground/metro station
            $poiTypeID = 99;
         }
         
         # Make sure not to overwrite a railway station type indicating
         # railway station.
         if ( $self->{bestTypeFromRailWayStationType} != 36){
            $self->{bestTypeFromRailWayStationType} = $poiTypeID;
         }

         if ( ! $self->addPOIType($self->{bestTypeFromRailWayStationType}) ) {
            $retVal = 0; 
            errprint "Problem adding railway station poi type.";
         }
      }
      elsif ( $infoType eq "IMPORTANCE" ) {
         if ( ! $self->myAddGDFPOIInfo(
                      $infoType, $infoValue, $poiItemID) ) {
            $retVal = 0; 
            errprint "Problem adding poi info attribute: IMPORTANCE.";
         }
      }
      elsif ( $infoType eq "MAJOR_ROAD_FEATURE" ) {
         if ( ! $self->myAddGDFPOIInfo(
                      $infoType, $infoValue, $poiItemID) ) {
            $retVal = 0; 
            errprint "Problem adding poi info attribute: MAJOR_ROAD_FEATURE.";
         }
      }
      elsif ( $infoType eq "FOOD_TYPE") {
         #dbgprint "Handling food type: $infoValue";
         my $catID = PerlCategoriesObject::taFoodTypeToCategory($infoValue);
         if ( defined $catID ){
            #dbgprint "Setting food type";
            if (!$self->addCategory($catID)){
               $retVal = 0; 
               errprint "Failed to set category $catID from foodType $infoValue";
            }
            #$self->dumpInfo();
         }
      }
      elsif ( $infoType eq "MILITARY_SERVICE_BRANCH") {
         my $catID = 
             PerlCategoriesObject::taMilitaryServiceBranchToCategory($infoValue);
         if ( defined $catID ){
            $self->addCategory($catID);
         }
      }   
      elsif ( $infoType eq "FERRY_TYPE") {
         my $catIDs = PerlCategoriesObject::taFerryTypeToCategory($infoValue);
         if ( defined $catIDs ){
            foreach my $catID (@{$catIDs}){
               $self->addCategory($catID);
            }
         }
      }   
      elsif ( $infoType eq "TOURIST_ATTRACTION_TYPE") {
         my $catID = PerlCategoriesObject::taTouristAttrTypeToCategory($infoValue);
         if ( defined $catID ){
            $self->addCategory($catID);
         }
      }
      elsif ( $infoType eq "ARTIFICIAL") {
         #print " poi " . " artificial infoValue = $infoValue\n";
         if ( $infoValue == 1 ) {
            $self->{m_artificial} = 1;
         }
      }
      elsif ( $infoType eq "EXTERNALIDENTIFIER" ) {
         #print " poi externalIdentifier = $infoValue\n";
         $self->{m_externalIdentifier} = $infoValue;
      }
      elsif ( $infoType eq "STAR_RATE" ) {
         my $add_star_rate = 0;
         if ($infoValue =~ m/\*+/) {
            #print "star rate value $infoValue\n";
            $add_star_rate = 1;
         } elsif ($infoValue =~ m/^\d$/) {
            my $counter = 0;
            my $newInfoValue = "";
            #print "star rate has number $infoValue\n";
            while ($counter < $infoValue) {
               $newInfoValue .= "*";
               $counter++;
            }
            $infoValue = $newInfoValue;
            $add_star_rate = 1;
         } else {
            $retVal = 0; 
            errprint "STAR_RATE info format not correct ($infoValue)." .
                     " Will not add.";
         }
         #print " poi star rate = $infoValue\n";
         if ($add_star_rate) {
            if ( ! $self->myAddGDFPOIInfo(
                         $infoType, $infoValue, $poiItemID) ) {
               $retVal = 0; 
               errprint "Problem adding poi info attribute: STAR_RATE.";
            }
         }

      # CODE_TYPE is a flag telling what info comes in CODE_VALUE
      # these are combined to the infoTypes
      # XTRA_FEATURE_CODE and XTRA_SERVICE_SUB_CATEGORY_
      } elsif ( $infoType eq 'XTRA_FEATURE_CODE' ) {
         my ($typeID, $catID) = get_type_and_cat_from_gdf_feature_code($infoValue);
         #dbgprint "XTRA_FEATURE_CODE gives additional cat=$catID type=$typeID\n";
         if (!$self->addCategory($catID)){
            errprint "Failed to set category", $catID;
         }
         if (!$self->addPOIType($typeID)) {
            errprint "Failed to set type", $typeID;
         }

      } elsif ( $infoType eq 'XTRA_SERVICE_SUB_CATEGORY' ) {
         my ($catID) = PerlCategoriesObject::taServiceSubCategoryToCategory($infoValue);
         my ($typeID) = PerlCategoriesObject::taServiceSubCategoryToType($infoValue);
         #dbgprint "XTRA_SERVICE_SUB_CATEGORY gives additional cat=$catID type=$typeID\n";
         # above: getting two things in an array
         if ( defined $catID ){
            #dbgprint "Setting category $catID";
            # are we setting any types according to categories?
            if (!$self->addCategory($catID)){
               errprint "Failed to set category", $catID;
            }
         }
         if ( defined $typeID ) {
            if ( ($typeID == 96) or ($typeID == 97) or ($typeID == 98) ) {
               $self->removePOIType(82); # place of worship
            } elsif ( ($typeID == 78) or ($typeID == 79) ) {
               $self->removePOIType(46); # theatre
            } elsif ( $typeID == 20) { 
               $self->removePOIType(56); # shop
            } elsif ( $typeID == 57) {
               $self->removePOIType(80); # park and recreation area
            } elsif ( $typeID == 6) {
               $self->removePOIType(94); # notype
            }
            #dbgprint "Setting type $typeID\n";
            if (!$self->addPOIType($typeID)) {
               errprint "Failed to set type", $typeID;
            }
         }
      } elsif ( $infoType eq "SERVICE_SUB_CATEGORY" ) {
         #print " poi serviceSubCategory = $infoValue\n";
         my $catID = PerlCategoriesObject::taServiceSubCategoryToCategory($infoValue);
         my $typeID = PerlCategoriesObject::taServiceSubCategoryToType($infoValue);
         # above: getting two things in an array
         if ( defined $catID ){
            #dbgprint "Setting category $catID";
            # are we setting any types according to categories?
            if (!$self->addCategory($catID)){
               errprint "Failed to set category", $catID;
            }
         }
         if ( defined $typeID ) {
            if ( ($typeID == 96) or ($typeID == 97) or ($typeID == 98) ) {
               $self->removePOIType(82); # place of worship
            } elsif ( ($typeID == 78) or ($typeID == 79) ) {
               $self->removePOIType(46); # theatre
            } elsif ( $typeID == 20) { 
               $self->removePOIType(56); # shop
            } elsif ( $typeID == 57) {
               $self->removePOIType(80); # park and recreation area
            } elsif ( $typeID == 6) {
               $self->removePOIType(94); # notype
            }
            #dbgprint "Setting type $typeID\n";
            if (!$self->addPOIType($typeID)) {
               errprint "Failed to set type", $typeID;
            }
         }
      }
      elsif ( $infoType eq "DEPARTURE_ARRIVAL" ) {
         # if it's a terminal of type arrivals
         # we set it deleted, to avoid getting too many
         # terminals
         # if there is an arrivals terminal
         # there is also at least one departures terminal
         if ($infoValue == 2) {
            $self->setDeleted(1);
         }
      }
      elsif ( $infoType eq "GEOCODING_ACCURACY_LEVEL" ) {
         # if GAL == 16 we might not want the POI
         # if the street it is on, is a long one
         if ( ! $self->myAddGDFPOIInfo(
                      $infoType, $infoValue, $poiItemID) ) {
            $retVal = 0; 
            errprint "Problem adding poi info attribute: $infoType.";
         }
     }
     elsif ( $infoType eq "RATING_SOURCE" ) {
         # this is the source that is connected to the star rate
         if ( ! $self->myAddGDFPOIInfo(
                      $infoType, $infoValue, $poiItemID) ) {
            $retVal = 0; 
            errprint "Problem adding poi info attribute: $infoType.";
         }
     }
      else {
         # Not handling these types
         if ( ( $infoType eq "REST_AREA_FACILITIES") or
              ( $infoType eq "GOVERNMENT_TYPE") or
              ( $infoType eq "SERVICE_GROUP") 
              ){
            # not handling these types
         } else {
            # unknown info type, something to handle?
            $retVal = 0; errprint "Unknown info type $infoType";
         }
      }
   }
   
   elsif ( $recordType == 5 ) {
      # Additional information (the normal POI info)
      # infoType;infoValue
      # where infoValue = infoLang;infoText or only infoText
      
      (my $infoType, my $infoValue) = split ("$fieldSep", $attributes);

      if ( ! $self->myAddGDFPOIInfo(
                   $infoType, $infoValue, $poiItemID) ) {
         $retVal = 0; errprint "Problem adding poi info attribute.";
      }
   }
   
   else {
      errprint "record type $recordType not handled";
      $retVal = 0;
   }

   if ( ! $retVal ) {
      errprint "Problem adding POI attribute '$attrLine' " .
               "to POI poiItemID=$poiItemID";
   }

   return $retVal;
} # myAddGDFPOIAttribute

# Add names from GDF POI file
sub myAddGDFPOINames {
   my $self = $_[0];
   my $allNames = $_[1];
   my $poiItemID = $_[2];
   
   my $retVal = 1;
   my @names = split(/$gdfNameSep/, $allNames);

   foreach my $nameRec (@names) {
      #print " poi $poiItemID name: $nameRec\n";
      (my $typeStr, my $langStr, my $name) = 
          split (/$gdfNameSubSep/, $nameRec);
      my $typeID = getNameType($self->{m_dbh}, $typeStr);
      my $langID = getNameLang($self->{m_dbh}, $langStr);
      
      if (!defined($name) || !defined($typeID) || !defined($langID)) {
         errprint "Missing name, type and/or lang for POI $poiItemID\n";
         $retVal = 0;
      }
      
      # we don't want empty names
      if ( $name eq "" || $name eq " " ){
         print "  not adding empty name\n";
      }
      else {
         if ( ! $self->addName( $name, $typeID, $langID ) ) {
            $retVal = 0;
         }
      }
   }

   return $retVal;
} # myAddGDFPOINames

# Add one poi info from GDF POI file
# infoValue = infoLang;infoText or only infoText
# POSTALCODE
# PHONENUMBER
# URL
# EMAIL
# HOUSENUMBER
# PLACENAME
# STREETNAME
# BRANDNAME
# CITY_CENTRE_DISPLAY_CLASS
sub myAddGDFPOIInfo {
   my $self = $_[0];
   my $infoType = $_[1];
   my $infoValue = $_[2];
   my $poiItemID = $_[3];
   #print " poi $poiItemID info: $infoType $infoValue\n";

   my $retVal = 1;

   (my $infoLang, my $infoText) = split (/$gdfNameSubSep/, $infoValue);
   if ( !defined($infoText) ){
      # No language found, set invalid language.
      $infoText = $infoLang;
      $infoLang = "invalidLanguage"; # invalid language
   }

   # get info keyID, langID
   my $keyID = getInfoKeyIDforGDFPOI($infoType);
   my $langID = getNameLang($self->{m_dbh}, $infoLang);
   # format the info text
   $infoText = formatGDFPOIInfoValue( $infoText, $keyID );
   
   if (!defined($infoText) || !defined($keyID) || !defined($langID)) {
      errprint "No infoText, keyID and/or infoLang for POI $poiItemID\n";
      $retVal = 0;
   }
   if ( ! $self->addPOIInfo( $keyID, $langID, $infoText ) ) {
      $retVal = 0;
   }
      
   return $retVal;
} # myAddGDFPOIInfo

# Create this POI from Wayfinder GDF POI import format
sub readFromGDFPOIFormat {
   my $self = $_[0];
   my $FILENAME = $_[1];
   my $startPos = $_[2];
   #print "readFromGDFPOIFormat file '$FILENAME', start at pos $startPos\n";
   open FILE, $FILENAME or die "POIObject::readFromGDFPOIFormat: ".
       "cannot open file:" . $FILENAME . "\n";
   
   undef $self->{m_id};

   # return the position in the file where to start reading NEXT POI
   my $returnPos = $startPos;

   # start reading from startPos
   seek( FILE, $startPos, 0 );
   
   # Keep on reading, as long as it is the same poi marker add
   # the attributes to myself
   my $nbrRows = 0;
   my $startPoiItemID = "-1";
   my $cont = 1;
   
   while ( $cont ) {
      # read one row from FILE
      $nbrRows += 1;
      my $line= (<FILE>);
      if ( $line ) {
         chomp $line;
         #print "line $nbrRows: '$line'\n";
         ( my $crap, my $poiItemID, my $attrLine ) = split( "-", $line, 3 );
         #print "poiItemID $poiItemID\n";

         if ( $nbrRows == 1) {
            # first attribute for this POI, save the poi marker
            $startPoiItemID = $poiItemID;
            print "readFromGDFPOIFormat poi $poiItemID\n";
         } elsif ( $startPoiItemID ne $poiItemID ) {
            # peeked into a row with info for next POI, break while loop
            # and return
            $cont = 0;
         }

         if ( $cont ) {
            # add this attribute to myself!
            my $retVal =
               $self->myAddGDFPOIAttribute($attrLine, $poiItemID);
            if ( ! $retVal ) {
               die "readFromGDFPOIFormat: myAddGDFPOIAttribute exit";
            }
            
            # update the returnPos, where to start reading next time
            $returnPos = tell( FILE );
         }
      } else {
         $cont = 0;
      }
   }

   # Set some misc attributes, if they were not read from GDF POI file
   if ( $startPos != $returnPos ) {
      # nothing as far as I know...

   }

   return $returnPos;
} # readFromGDFPOIFormat

# Add one attribute to myself from the CPIF file
sub myAddCPIFAttribute {
   my $self = $_[0];
   my $poiAttr = $_[1];
   my $attrVal = $_[2];

   #print "    adding cpif attr ", $poiAttr, ": '", $attrVal, "'\n";

   my $retVal = 0;

   # 1 active obsolete - REPLACED by inUse and deleted
#   if ( $poiAttr == 1 ) {
#      $retVal = $self->setActive( $attrVal );
#   }
   
   # 2 comment
   if ( $poiAttr == 2 ) {
      $retVal = $self->setComment( $attrVal );
   }
   
   # 3 country
   elsif ( $poiAttr == 3 ) {
      $retVal = $self->setCountry( $attrVal );
   }
   
   # 4 symbolCoordinate
   elsif ( $poiAttr == 4 ) {
      ( my $lat, my $lon ) = split( ";", $attrVal);
      $retVal = $self->setSymbolCoordinate( $lat, $lon );
   }
   
   # 5 validFromVersion
   elsif ( $poiAttr == 5 ) {
      $retVal = $self->setValidFromVersion( $attrVal );
   }
   
   # 6 validToVersion
   elsif ( $poiAttr == 6 ) {
      $retVal = $self->setValidToVersion( $attrVal );
   }
   
   # 7 poiType
   elsif ( $poiAttr == 7 ) {
      $retVal = $self->addPOIType( $attrVal );
   }
   
   # 8 entryPointCoordinate
   elsif ( $poiAttr == 8 ) {
      ( my $epLat, my $epLon ) = split( ";", $attrVal);
      $retVal = $self->addEntryPoint( $epLat, $epLon );
   }
   
   # 9 name
   elsif ( $poiAttr == 9 ) {
      ( my $type, my $lang, my $name ) = split( ";", $attrVal);
      $retVal = $self->addName( $name, $type, $lang );
   }
   
   # 10 source
   elsif ( $poiAttr == 10 ) {
      $retVal = $self->setSource( $attrVal );
   }
   
   # 11 sourceRef
   elsif ( $poiAttr == 11 ) {
      $retVal = $self->setSourceReference( $attrVal );
   }
   
   # 12 poiID
   elsif ( $poiAttr == 12 ) {
      # nothing, should not set $self->{m_id} from CPIF file
   }

   # 13 deleted
   elsif ( $poiAttr == 13 ) {
      $retVal = $self->setDeleted( $attrVal );
   }

   # 14 inUse
   elsif ( $poiAttr == 14 ) {
      $retVal = $self->setInUse( $attrVal );
   }
   
   # 15 poiCategory
   elsif ( $poiAttr == 15 ) {
      $retVal = $self->addCategory( $attrVal );
   }
   
   # 16 poiExternalIdentifier
   # will perhaps not get from cpif
   #elsif ( $poiAttr == 16 ) {
   #}
   
   # 1000++ misc poi info
   elsif ( $poiAttr >= 1000 ) {
      my $infoKey = $poiAttr - 1000;
      ( my $lang, my $val ) = split( ";", $attrVal);
      $retVal = $self->addPOIInfo( $infoKey, $lang, $val );
   }
   
   # Else unknown attribute type for correction
   else {
      warn "Undefined poi attribute type $poiAttr\n";
   }

   return $retVal;
}

# Create this POI from Wayfinder complex POI import format
sub readFromCPIF {
   my $self = $_[0];
   my $FILENAME = $_[1];
   my $startPos = $_[2];
   #print "readFromCPIF file '$FILENAME', start at pos $startPos\n";
   open FILE, $FILENAME or die "POIObject::readFromGDFPOIFormat: ".
       "cannot open file:" . $FILENAME . "\n";
  
   undef $self->{m_id};

   # return the position in the file where to start reading NEXT POI
   my $returnPos = $startPos;

   # start reading from startPos
   seek( FILE, $startPos, 0 );
   
   # Keep on reading, as long as it is the same poi marker id add
   # the attributes to myself
   my $nbr = 0;
   my $startPoiMarker = "-1";
   my $cont = 1;
   
   while ( $cont ) {
      # read one row from FILE
      $nbr += 1;
      my $line= (<FILE>);
      if ( $line ) {
         chomp $line;
         #print "  line $nbr: '$line'\n";
         ( my $poiMarker, my $attrType, my $attrVal ) = split( ";", $line, 3 );

         # Some attributes might cover several lines in the CPIF file
         # (typically WCities long_description).
         # Peek forward to see if this is the case, to build the complete
         # attribute val before adding to self
         #print " nbr $nbr start peeking: at " . tell( FILE ) . "\n";
         my $tmpPos = tell( FILE );
         my $tmpCont = 1;
         while ( $tmpCont ) {
            # peek into next row to see if the info continues there
            my $peekLine = (<FILE>);
            if ( $peekLine ) {
               chomp $peekLine;

               if ( length($peekLine) < 1 ) {
                  # we have an empty line
                  $attrVal = "$attrVal\n";
                  #print "  empty line: $attrVal\n";
                  
                  # update the returnPos, where to start reading next time
                  $tmpPos = tell( FILE );
                  
               } else {
                  # info continues, or we have a new attribute.
                  ( my $poiMarker2, my $attrType2,
                    my $attrVal2 ) = split( ";", $peekLine, 3 );
                  
                  my $maxATID = getMaxAttributeTypeID( $self->{m_dbh} );
                  my $minorMaxATID = 
                     getMaxAttributeTypeIDLessThanInfo( $self->{m_dbh} );
                  
                  if ( defined($poiMarker2) and defined($attrType2) and
                       defined($attrVal2) and ($attrType2 =~ /^[0-9]+$/) and
                       (($attrType2 >= 1 and $attrType2 <= $minorMaxATID) or
                        ($attrType2 >= 1000 and $attrType2 <= $maxATID)) ) {
                     # New attribute
                     # Can be 99,999% sure about this: there is at least
                     # 2 semi colons, and the $attrType2 is a valid type.
                     $tmpCont = 0;
                  } else {
                     # same attribute, fill...
                     $attrVal = "$attrVal\n$peekLine";
                     #print "  same attri: $attrVal\n";
                     $tmpPos = tell( FILE );
                  }
               }
            } else {
               $tmpCont = 0;
               #print "   eof\n";
            }
         }
         seek(FILE, $tmpPos, 0);
         #print " nbr $nbr end peeking: at " . tell( FILE ) . "\n";
         # end of peeking

         if ( $nbr == 1) {
            # first attribute for this POI, save the poi marker
            $startPoiMarker = $poiMarker;
         } elsif ( $startPoiMarker ne $poiMarker ) {
            # peeked into a row with info for next POI, break while loop
            # and return
            $cont = 0;
         }

         if ( $cont ) {
            # add this attribute to myself!
            $self->myAddCPIFAttribute($attrType, $attrVal);
            
            # update the returnPos, where to start reading next time
            $returnPos = tell( FILE );
         }
      } else {
         $cont = 0;
      }
   }

   # Set some misc attributes, if they were not read from CPIF
   if ( $startPos != $returnPos ) {
      
      # add startPoiMarker as sourceRef if not set
      if ( !defined($self->{m_sourceReference}) ) {
         $self->setSourceReference( $startPoiMarker );
      }
      # set active true if not defined
		# replaced by inUse and deleted
		# inUse defaults to "1" and deleted to "0" if not specified
#      if ( !defined($self->{m_active}) ) {
#         $self->setActive( 1 );
#      }
   }

   return $returnPos;
}

sub myAddSPIFAttributes {
   #print " myAddSPIFAttributes: adding attr for POI $_[0]\n";
   my $self = shift;
   my $defaultLang = 31; # invalidLang
   if ( defined($self->{m_language}) ) {
      $defaultLang = $self->{m_language};
   }
   
   my $retVal = 1;

   # src ref
   if ( defined($_[0]) and length($_[0]) and
        !$self->setSourceReference($_[0]) ) {
      $retVal = 0;
      print "ERROR Problem with source reference attribute.\n";
   }
   # official name
   if ( $retVal and defined($_[1]) and length($_[1]) 
         and !$self->addName($_[1], 0, $defaultLang) ) {
      $retVal = 0;
      print "ERROR Problem with official name attribute.\n";
   }
   # poi type
   if ( $retVal and defined($_[2]) and length($_[2]) 
         and !$self->addPOIType($_[2]) ) {
      $retVal = 0;
      print "ERROR Problem with poi type attribute.\n";
   }
   # symbol coord
   if ( $retVal and defined($_[3]) and length($_[3]) and defined($_[4]) 
         and length($_[4]) and !$self->setSymbolCoordinate($_[3], $_[4]) ) {
      $retVal = 0;
      print "ERROR Problem with symbol coord attribute.\n";
   }
   # country
   if ( $retVal and defined($_[5]) and length($_[5]) ) {
      my $countryID = getCountryIDFromName($self->{m_dbh}, $_[5]);
      if ( !$self->setCountry($countryID) ) {
         $retVal = 0;
         print "ERROR Problem with country attribute.\n";
      }
   }
   # zip code
   if ( $retVal and defined($_[6]) and length($_[6])
         and !$self->addPOIInfo(2, 31, $_[6]) ) {
      $retVal = 0;
      print "ERROR Problem with zip code attribute.\n";
   }
   # zip area
   if ( $retVal and defined($_[7]) and length($_[7])
         and !$self->addPOIInfo(5, 31, $_[7]) ) {
      $retVal = 0;
      print "ERROR Problem with zip area attribute.\n";
   }
   # streetName 
   if ( $retVal and defined($_[8]) and length($_[8])
         and !$self->addPOIInfo(0, 31, $_[8]) ) {
      $retVal = 0;
      print "ERROR Problem with streetName attribute.\n";
   }
   # houseNumber
   if ( $retVal and defined($_[9]) and length($_[9])
         and !$self->addPOIInfo(1, 31, $_[9]) ) {
      $retVal = 0;
      print "ERROR Problem with houseNumber attribute.\n";
   }
   # full address
   if ( $retVal and defined($_[10]) and length($_[10])
         and !$self->addPOIInfo(6, 31, $_[10]) ) {
      $retVal = 0;
      print "ERROR Problem with full address attribute.\n";
   }
   # phone number
   if ( $retVal and defined($_[11]) and length($_[11])
         and !$self->addPOIInfo(4, 31, $_[11]) ) {
      $retVal = 0;
      print "ERROR Problem with phone number attribute.\n";
   }
   # fax number
   if ( $retVal and defined($_[12]) and length($_[12])
         and !$self->addPOIInfo(7, 31, $_[12]) ) {
      $retVal = 0;
      print "ERROR Problem with fax number attribute.\n";
   }
   # email
   if ( $retVal and defined($_[13]) and length($_[13])
         and !$self->addPOIInfo(8, 31, $_[13]) ) {
      $retVal = 0;
      print "ERROR Problem with email attribute.\n";
   }
   # url
   if ( $retVal and defined($_[14]) and length($_[14])
         and !$self->addPOIInfo(9, 31, $_[14]) ) {
      $retVal = 0;
      print "ERROR Problem with url attribute.\n";
   }
   # openHours
   if ( $retVal and defined($_[15]) and length($_[15])
         and !$self->addPOIInfo(16, 31, $_[15]) ) {
      $retVal = 0;
      print "ERROR Problem with openHours attribute.\n";
   }
   # short description
   if ( $retVal and defined($_[16]) and length($_[16])
         and !$self->addPOIInfo(11, $defaultLang, $_[16]) ) {
      $retVal = 0;
      print "ERROR Problem with short description attribute.\n";
   }
   # long description
   if ( $retVal and defined($_[17]) and length($_[17])
         and !$self->addPOIInfo(12, $defaultLang, $_[17]) ) {
      $retVal = 0;
      print "ERROR Problem with long description attribute.\n";
   }
   # image
   if ( $retVal and defined($_[18]) and (length($_[18]) > 1) # windows newline
         and !$self->addPOIInfo(48, 31, $_[18]) ) {
      $retVal = 0;
      print "ERROR Problem with image attribute.\n";
   }
   # cc display class
   if ( $retVal and defined($_[19]) and length($_[19])
         and !$self->addPOIInfo(47, 31, $_[19]) ) {
      $retVal = 0;
      print "ERROR Problem with cc display class attribute.\n";
   }

   if ( ! $retVal ) {
      print "ERROR failed to add SPIF attribute for poi ", $_[0], "\n";
   }
   return $retVal;
}

# Create this POI from Wayfinder simple POI import format
sub readFromSPIF {
   my $self = $_[0];
   my $FILENAME = $_[1];
   my $startPos = $_[2];
	my $inUse    = $_[3];
   #print "readSPIF file '$FILENAME', start at pos $startPos\n";
   open FILE, $FILENAME or die "POIObject::readFromGDFPOIFormat: ".
       "cannot open file:" . $FILENAME . "\n";
   
   undef $self->{m_id};

   # return the position in the file where to start reading NEXT POI
   my $returnPos = $startPos;

   # start reading from startPos
   seek( FILE, $startPos, 0 );
   
   my $line = (<FILE>);
   my $filePos = tell( FILE );
   if ( $line and ( $filePos > $startPos) ) {
      $returnPos = $filePos;
      chomp $line;

      # split the line at semicolons and add all attributes to myself!
      my @record = split( ";", $line );
      $self->myAddSPIFAttributes( @record );
      
      # set active
		# replaced by inUse and deleted
		# inUse defaults to 1 and deleted to 0
      $self->setInUse( $inUse );
   }

   return $returnPos;
}

# Dump some info..
sub dumpInfo {
   my $self = $_[0];

   my $dumpStr = "************ POI info: **********\n";
   if ( defined($self->{m_id} ) ) {
      $dumpStr .= " id           = $self->{m_id}\n";
   }
   if ( defined($self->{m_inUse}) ) {
      $dumpStr .= " inUse       = $self->{m_inUse}\n";
   }
   if ( defined($self->{m_deleted}) ) {
      $dumpStr .= " deleted       = $self->{m_deleted}\n";
   }
   if ( defined($self->{m_source}) ) {
      $dumpStr .= " source       = $self->{m_source}\n";
   }
   if ( defined($self->{m_sourceReference}) ) {
      $dumpStr .= " sourceRef    = $self->{m_sourceReference}\n";
   }
   if ( defined($self->{m_country}) ) {
      $dumpStr .= " country      = $self->{m_country}\n";
   }
   if ( defined($self->{m_rights}) ) {
      $dumpStr .= " right      = $self->{m_rights}\n";
   }
   if ( defined($self->{m_validFromVersion}) ) {
      $dumpStr .= " validFrom    = $self->{m_validFromVersion}\n";
   }
   if ( defined($self->{m_validToVersion}) ) {
      $dumpStr .= " validTo      = $self->{m_validToVersion}\n";
   }
   if ( defined( $self->getSymbolCoordinate() ) ) {
      $dumpStr .= " coord        = " . $self->getSymbolCoordinate() . "\n";
   }
   if ( ( defined($self->{m_entryPoints} ) ) and
        ( scalar( @{$self->{m_entryPoints}} ) > 0 ) ) {
      $dumpStr .= " ep           = @{$self->{m_entryPoints}}\n";
   }
   if ( defined($self->{m_poiInfo}) and 
        ( scalar( @{$self->{m_poiInfo}}) > 0 ) ) {
      my @infoStrs = @{$self->{m_poiInfo}};
      #foreach my $infoStr ( @infoStrs ){
      #   $infoStr =~ s/$/\n               /;
      #}
      #$dumpStr .= " info         = @{$self->{m_poiInfo}}\n";
      $dumpStr .= " info         = @infoStrs\n";
   }
   if ( defined($self->{m_poiNames}) and
        ( scalar( @{$self->{m_poiNames}}) > 0 ) ) {
      $dumpStr .= " names        = @{$self->{m_poiNames}}\n";
   }
   if ( defined($self->{m_poiTypes}) and
        ( scalar( @{$self->{m_poiTypes}}) > 0 ) ) {
      $dumpStr .= " types        = @{$self->{m_poiTypes}}\n";
   }
   if ( defined($self->{m_poiCategories}) and
        ( scalar( @{$self->{m_poiCategories}}) > 0 ) ) {
      $dumpStr .= " categories   = @{$self->{m_poiCategories}}\n";
   }
   if ( defined($self->{m_comment}) ) {
      $dumpStr .= " comment      = $self->{m_comment}\n";
   }
   print $dumpStr;
}

# add this POIObject as a new POI to WASP db, and return the new poiID
sub addToWASP {

   my $self = $_[0];

   if ( !defined($self->{m_dbh}) ) {
      print "POIObject addToWASP - no connection to database\n";
      return 0;
   }

   # XXX If attributes are not definded, don't add them in the INSERT
   # query, but let the attribute fields have default values.

   # Used for setting the the symbol coordinate in the database.
   my $lat; 
   my $lon;

   # The following attributes must exist:
   # - source
   # - sourceReference
   # - country
   # - symbol coordinate
   # - one official name
   # - one poi type
   if ( !defined($self->{m_source}) or 
        !defined($self->{m_sourceReference}) or 
        !defined($self->{m_lat}) or 
        !defined($self->{m_lon}) or
        !defined($self->{m_country}) or 
        !defined($self->{m_poiNames}) or 
        (scalar(@{$self->{m_poiNames}}) == 0) or
        !defined($self->{m_poiTypes}) or
        (scalar(@{$self->{m_poiTypes}}) == 0) ) {
      #print "POIObject addToWASP - some mandatory attribute(s) missing\n";
      #$self->dumpInfo();

      if ( defined($self->{m_source}) and
           defined($self->{m_sourceReference}) and
           defined($self->{m_country}) and
           defined($self->{m_poiNames}) and 
           (scalar(@{$self->{m_poiNames}}) != 0) and
           defined($self->{m_poiTypes}) and
           (scalar(@{$self->{m_poiTypes}}) != 0) ) {
         # This POI only needs geocoding.

         if (! $self->{m_allowNoPosInDb} ){
            print "GGG:" . $self->{m_sourceReference} . "\n";
            return 0;
         }
         else {
            #continue without coordinate.
            $lat = undef; # this indicates NULL
            $lon = undef; # this indicates NULL
         }
      }
      else {
         # This POI have some serious problem.
         print "EEE:" . $self->{m_sourceReference} . "\n";        
         return 0;
      }
   }
   else {
      $lat = $self->{m_lat};
      $lon = $self->{m_lon};
   }


   # POIMain
   my $useRights = 0;
   my $query = "INSERT INTO POIMain " .
       " SET".
       " source=?,".
       " added=NOW(),".
       " lastModified=NOW(),".
       " lat=?,".
       " lon=?,".
       " inUse=?,".
       " deleted=?,".
       " sourceReference=?,".
       " comment=?,".
       " country=?,".
       " validFromVersion=?,".
       " validToVersion=?";

   my $queryWRights = $query . ", rights=?";

   my $rightsValue = undef;
   if (defined ($self->{m_rights}) and ($self->{m_rights} ne "") ) {
      $rightsValue = $self->{m_rights};
      $useRights = 1;
   }
   if ($useRights) {
      if (!defined $sthInsertMainWRights){

         $sthInsertMainWRights = $self->{m_dbh}->prepare($queryWRights) || 
             die "Prepare failed, query: $queryWRights";
      }
   } else {
      if (!defined $sthInsertMain){

         $sthInsertMain = $self->{m_dbh}->prepare($query) || 
             die "Prepare failed, query: $query";
      }
   }
   my $validFromValue = undef; # this is NULL
   if ( defined($self->{m_validFromVersion}) and 
        ($self->{m_validFromVersion} ne "") ) {
      $validFromValue = $self->{m_validFromVersion};
   }
   my $validToValue = undef; # this is NULL
   if ( defined($self->{m_validToVersion}) and 
        ($self->{m_validToVersion} ne "") ) {
      $validToValue = $self->{m_validToVersion};
   }
   my $deletedValue = 0;
   if ( defined($self->{m_deleted}) and ($self->{m_deleted} ne "")) {
     $deletedValue = $self->{m_deleted};
   }

   if ($useRights) {
      $sthInsertMainWRights->execute($self->{m_source},
                           $lat,
                           $lon,
                           $self->{m_inUse},
                           $deletedValue,
                           $self->{m_sourceReference},
                           $self->{m_comment},
                           $self->{m_country},
                           $self->{m_validFromVersion},
                           $self->{m_validToVersion},
                           $rightsValue)
         || die "Failed adding to POIMain for srcRef $self->{m_sourceReference}".
         " $self->{m_dbh}->errstr";
   } else {
      $sthInsertMain->execute($self->{m_source},
                           $lat,
                           $lon,
                           $self->{m_inUse},
                           $deletedValue,
                           $self->{m_sourceReference},
                           $self->{m_comment},
                           $self->{m_country},
                           $self->{m_validFromVersion},
                           $self->{m_validToVersion})   
         || die "Failed adding to POIMain for srcRef $self->{m_sourceReference}".
         " $self->{m_dbh}->errstr";
   }
   $self->{m_id} = $self->{m_dbh}->{'mysql_insertid'};   # get the new poiID
   if ($self->{m_id} > 4000000000){
      die "POI ID larger than 4000000000, it will soon not fit into a uint32";
   }
   #print " added to POIMain, id=$self->{m_id}\n";

   # If the insertion into POIMain failed, don't insert into other tables!
   

   
   # Fixme: add the external identifier to some table in WASP database
   #        if you want to use it

   # POIEntryPoints "lat,lon"
   if (!defined $sthInsertEtrPts){
      my $query = "INSERT  INTO POIEntryPoints".
          " (poiID, lat, lon)".
          " VALUES (?, ?, ?);";
      $sthInsertEtrPts = $self->{m_dbh}->prepare($query) || 
          die "Prepare failed, query: $query";
   }
   foreach my $epStr ( @{$self->{m_entryPoints}} ) {
      (my $epLat, my $epLon) = split( $coordSep, $epStr );
      if ( defined($epLat) and defined($epLon) ) {
         $sthInsertEtrPts->execute($self->{m_id}, 
                                   $epLat, 
                                   $epLon)
             || die "Failed adding to POIEP waspID $self->{m_id}".
             " $self->{m_dbh}->errstr";
      }
   }


   # POIInfo, make sure city centres have a city centre display class
   if ( $self->hasPOIType(11) ) {
      my $myDisplClassVal = $self->getPOIInfoWithKey(47);
      if ( ! defined $myDisplClassVal ) {
         # no display class, please add default display class 12
         $self->addPOIInfo(47, 31, 12);
         print "Added default cc display class to POI " .
               $self->{m_sourceReference} . "\n";
      }
   }

   # POIInfo "keyID<¡>lang<¡>val"
   if (!defined $sthInsertInfo){
      my $dynamic_query = "INSERT  INTO " .
         "POIInfoDynamic (poiID, " .
         "keyID, val, lang) " .
         "VALUES (?, ?, ?, ?)";
      my $query = "INSERT  INTO POIInfo".
          " (poiID, keyID, val, lang)".
          " VALUES (?, ?, ?, ?);";
      $sthInsertInfo = $self->{m_dbh}->prepare($query) || 
          die "Prepare failed, query: $query";
      $sthInsertInfoDynamic = $self->{m_dbh}->prepare($dynamic_query) || 
          die "Prepare failed, dynamic query: $dynamic_query";
   }
   foreach my $infoStr ( @{$self->{m_poiInfo}} ) {
      (my $keyID, my $infoLang, my $infoVal) = split( $fieldSep, $infoStr );
      if ( defined($keyID) and defined($infoLang) and defined($infoVal) ) {
         # check if keyID is for dynamic updates
         if ($self->infoKeyIsDynamic($keyID)) {
            $sthInsertInfoDynamic->execute($self->{m_id},
                                 $keyID,
                                 $infoVal,
                                 $infoLang)
             || die "Failed adding to POIInfoDynamic waspID $self->{m_id}".
             " $self->{m_dbh}->errstr";

         } else {
            $sthInsertInfo->execute($self->{m_id},
                                 $keyID,
                                 $infoVal,
                                 $infoLang)
             || die "Failed adding to POIInfo waspID $self->{m_id}".
             " $self->{m_dbh}->errstr";
         }
      }
   }
   
   # POINames "name<¡>type<¡>lang"
   if (!defined $sthInsertName){
      my $query = "INSERT  INTO POINames".
          " (name, type, lang, poiID)" .
          " VALUES(?, ?, ?, ?);";
      $sthInsertName = $self->{m_dbh}->prepare($query) || 
          die "Prepare failed, query: $query";
   }
   foreach my $nameStr ( @{$self->{m_poiNames}} ) {
      (my $name, my $type, my $lang) = split( $fieldSep, $nameStr );
      if ( defined($name) and defined($type) and defined($lang) ) {
         $sthInsertName->execute($name,
                                 $type,
                                 $lang,
                                 $self->{m_id})
             || die "Failed adding to POINames waspID $self->{m_id}".
             " $self->{m_dbh}->errstr";
      }
   }
   # POITypes
   if (!defined $sthInsertType){
      my $query = "INSERT  INTO POITypes (poiID, typeID) ".
          " VALUES (?, ?);";
      $sthInsertType = $self->{m_dbh}->prepare($query) || 
          die "Prepare failed, query: $query";
   }
   foreach my $typeID ( @{$self->{m_poiTypes}} ) {
      if ( defined($typeID) ) {
         $sthInsertType->execute( $self->{m_id},
                                  $typeID )
             || die "Failed adding to POITypes waspID $self->{m_id}".
             " $self->{m_dbh}->errstr";
      }
   }

   # POICategories
   if (!defined $sthInsertCategory){
      my $query = "INSERT INTO POICategories (poiID, catID) ".
          " VALUES (?, ?);";
      $sthInsertCategory = $self->{m_dbh}->prepare($query) || 
          die "Prepare failed, query: $query";
   }
   if (defined $self->{m_poiCategories} ){
      foreach my $catID ( @{$self->{m_poiCategories}} ) {
         if ( defined($catID) ) {
            $sthInsertCategory->execute( $self->{m_id},
                                         $catID )
                || die "Failed adding to POICategories waspID $self->{m_id}".
                " $self->{m_dbh}->errstr";
         }
      }
   }
   
   return $self->{m_id};
}


# Write this POI object to a SPIF file
sub writeSPIF {
   my $self = $_[0];
   my $FILENAME = $_[1];
   
   # print info to SPIF file, requires sourceReference
   if ( defined($self->{m_sourceReference}) ) {

      # Names
      my $poiName = "";
      if ( defined($self->{m_poiNames}) and 
           (scalar( @{$self->{m_poiNames}}) > 0) ) {
         my %nameHash = (); # Store all official names.
         foreach my $nameStr ( @{$self->{m_poiNames}} ) {
            (my $name, my $type, my $lang) = split( $fieldSep, $nameStr );
            if ( defined($name) and defined($type) and defined($lang) ) {
               if ( $type == 0 ){
                  # Official name.
                  $nameHash{$lang}=$name;
               }
            }
            else {
               errprint "Broken name: $nameStr";
            }
         }
         if (scalar(keys(%nameHash)) > 0 ){
            my @langs = sort(keys(%nameHash));
            $poiName = $nameHash{$langs[0]}; # Pick name with lowest lang ID.
         }
      }

      # Types
      my $poiType = "";
      if ( defined($self->{m_poiTypes}) &&
           ( scalar(@{$self->{m_poiTypes}}) > 0 ) ) {
         $poiType = @{$self->{m_poiTypes}}[0]; # Pick first POI type.
      }

      # Position
      my $lat = "";
      my $lon = "";
      if ( defined($self->{m_lat}) && defined($self->{m_lon}) ) {
         $lat = $self->{m_lat};
         $lon = $self->{m_lon};
         ($lat, $lon) = convertFromMc2ToWgs84($lat, $lon);
      }

      # Country
      my $countryName = "";
      if ( defined($self->{m_country}) ){
         $countryName = getCountryNameFromID($self->{m_dbh}, 
                                             $self->{m_country});
      }
      
      # Info
      my $zipCode = "";
      my $zipArea = "";
      my $addressStreetName = "";
      my $addressHouseNumber ="";
      my $fullAddress = "";
      my $phoneNumber = "";
      my $faxNumber = "";
      my $email = "";
      my $url = "";
      my $openHours = "";
      my $shortDescription = "";
      my $longDescription = "";
      my $image = "";
      my $ccDisplayClass = "";
      if ( defined $self->{m_poiInfo} ){
         my @infoStrs = @{$self->{m_poiInfo}};
         foreach my $infoStr ( @infoStrs ){
            (my $keyID, my $infoLang, my $infoVal) = 
                split( $fieldSep, $infoStr );
            if ( defined($keyID) &&
                 defined($infoLang) &&
                 defined($infoVal) ){
               $infoVal =~ s/\n/ /g; # Remove new lines.
               $infoVal =~ s/\r//g;  # Remove new lines.
               $infoVal =~ s/;/,/g;  # Remove SPIF field separator.
               if ( $keyID == 2){
                  $zipCode = $infoVal;
               }
               elsif ( $keyID == 5){
                  $zipArea = $infoVal;
               }
               elsif ( $keyID == 0){
                  $addressStreetName = $infoVal;
               }
               elsif ( $keyID == 1){
                  $addressHouseNumber = $infoVal;
               }
               elsif ( $keyID == 6){
                  $fullAddress = $infoVal;
               }
               elsif ( $keyID == 4){
                  $phoneNumber = $infoVal;
               }
               elsif ( $keyID == 7){
                  $faxNumber = $infoVal;
               }
               elsif ( $keyID == 8){
                  $email = $infoVal;
               }
               elsif ( $keyID == 9){
                  $url = $infoVal;
               }
               elsif ( $keyID == 16){
                  $openHours = $infoVal;
               }
               elsif ( $keyID == 11){
                  $shortDescription = $infoVal;
               }
               elsif ( $keyID == 12){
                  $longDescription = $infoVal;
               }
               elsif ( $keyID == 48){
                  $image = $infoVal;
               }
               elsif ( $keyID == 47){
                  $ccDisplayClass = $infoVal;
               }
            }
         }
      }


      my $spifRow =
          #"$self->{m_sourceReference};".
          "$self->{m_id};".
          "$poiName;".
          "$poiType;".
          "$lat;".
          "$lon;".
          "$countryName;".
          "$zipCode;".
          "$zipArea;".
          "$addressStreetName;".
          "$addressHouseNumber".
          "$fullAddress;".
          "$phoneNumber;".
          "$faxNumber;".
          "$email;".
          "$url;".
          "$openHours;".
          "$shortDescription;".
          "$longDescription;".
          "$image;".
          "$ccDisplayClass;\n";

      # Validate the SPIF row.
      my @fields = split ";", $spifRow;
      my $nbrFields = scalar(@fields);
      if ( $nbrFields != 20 ){
         errprint "Invalid SPIF, fields=$nbrFields: $spifRow";
      }
      else {
         # Print SPIF row.
         open (FILE, ">>$FILENAME") or 
             die "cannot open file $FILENAME to write\n";
         print FILE $spifRow;
         close (FILE);
         return 1;
      }
   }
   return 0;
}



# Write this POI object to a CPIF file
sub writeCPIF {
   my $self = $_[0];
   my $FILENAME = $_[1];
   
   # Open file for writing with append
   open (FILE, ">>$FILENAME") or die "cannot open file $FILENAME to write\n";
   
   # print info to CPIF file, requires sourceReference
   if ( defined($self->{m_sourceReference}) ) {
      my $srcRef = $self->{m_sourceReference};

      # 1 active
		# replaced by inUse and deleted
#      if ( defined($self->{m_active}) ) {
#         print FILE "$srcRef;1;$self->{m_active}\n";
#      }
      # 2 comment
      if ( defined($self->{m_comment}) ) {
         print FILE "$srcRef;2;$self->{m_comment}\n";
      }
      # 3 country
      if ( defined($self->{m_country}) ) {
         print FILE "$srcRef;3;$self->{m_country}\n";
      }
      # 4 symbol coordinate
      if ( defined($self->{m_lat}) and defined($self->{m_lon}) ) {
         print FILE "$srcRef;4;$self->{m_lat};$self->{m_lon}\n";
      }
      # 5 valid from version
      if ( defined($self->{m_validFromVersion}) ) {
         print FILE "$srcRef;5;$self->{m_validFromVersion}\n";
      }
      # 6 valid to version
      if ( defined($self->{m_validToVersion}) ) {
         print FILE "$srcRef;6;$self->{m_validToVersion}\n";
      }
      # 7 POI type
      if ( defined($self->{m_poiTypes}) and
           ( scalar(@{$self->{m_poiTypes}}) > 0 ) ) {
         foreach my $typeID ( @{$self->{m_poiTypes}} ) {
            if ( defined($typeID) ) {
               print FILE "$srcRef;7;$typeID\n";
            }
         }
      }
      # 8 entry point coordinates
      if ( defined($self->{m_entryPoints}) and
           ( scalar( @{$self->{m_entryPoints}} ) > 0 ) ) {
         my @epStrs = @{$self->{m_entryPoints}};
         foreach my $epStr ( @epStrs ){
            (my $epLat, my $epLon) = split( $coordSep, $epStr );
            if ( defined($epLat) and defined($epLon) ) {
               print FILE "$srcRef;8;$epLat;$epLon\n";
            }
         }
      }
      # 9 names
      if ( defined($self->{m_poiNames}) and 
           (scalar( @{$self->{m_poiNames}}) > 0) ) {
         foreach my $nameStr ( @{$self->{m_poiNames}} ) {
            (my $name, my $type, my $lang) = split( $fieldSep, $nameStr );
            if ( defined($name) and defined($type) and defined($lang) ) {
               print FILE "$srcRef;9;$type;$lang;$name\n";
            }
            else {
               errprint "Broken name: $nameStr";
            }
         }
      }
      # 10 source
      if ( defined($self->{m_source}) ) {
         print FILE "$srcRef;10;$self->{m_source}\n";
      }
      # 11 source reference id
      if ( defined($self->{m_sourceReference}) ) {
         print FILE "$srcRef;11;$self->{m_sourceReference}\n";
      }
      # 12 POI wasp id (not max_uint32)
      if ( defined($self->{m_id}) and ($self->{m_id} != 2147483647) ) {
         print FILE "$srcRef;12;$self->{m_id}\n";
      }
      if ( defined($self->{m_deleted}) ) {
         print FILE "$srcRef;13;$self->{m_deleted}\n";
      }
      if ( defined($self->{m_inUse}) ) {
         print FILE "$srcRef;14;$self->{m_inUse}\n";
      }
      # 15 POI category
      if ( defined($self->{m_poiCategories}) and
           ( scalar(@{$self->{m_poiCategories}}) > 0 ) ) {
         foreach my $categoryID ( @{$self->{m_poiCategories}} ) {
            if ( defined($categoryID) ) {
               print FILE "$srcRef;15;$categoryID\n";
            }
         }
      }
      # 16 POI External Identifier
      if ( defined ($self->{m_externalIdentifier})) {
         print FILE "$srcRef;16;$self->{m_externalIdentifier}\n";
      }

      # 1000 ++ misc poi info
      if ( defined($self->{m_poiInfo}) and 
           (scalar( @{$self->{m_poiInfo}}) > 0) ) {
         my @infoStrs = @{$self->{m_poiInfo}};
         foreach my $infoStr ( @infoStrs ){
            (my $keyID, my $infoLang, my $infoVal) = 
               split( $fieldSep, $infoStr );
            if ( defined($keyID) and 
                 defined($infoLang) and defined($infoVal) ) {
               print FILE "$srcRef;", 1000+$keyID, ";$infoLang;$infoVal\n";
            }
         }
      }

      return 1;
   }
   return 0;
}

# Write file for geocoding, use ed separator <¡>
# id<¡>topRegionName<¡>fullAddress<¡>zipCode<¡>city(zipArea)
sub writeGeocodeFile {
   my $self = $_[0];
   my $FILENAME = $_[1];
   my $streetPrio = $_[2];
   
   # Open file for writing with append
   open (FILE, ">>$FILENAME") or die "cannot open file $FILENAME to write\n";
   
   # print info to GEOC file, requires sourceReference for back-linking..
   if ( defined($self->{m_sourceReference}) ) {

      # geoc id
      my $srcRef = $self->{m_sourceReference};

      # top region
      my $topRegion = getTopRegionName( $self );

      # geoc address = full address
      my $visAddr = getPOIInfoWithKey($self, 0);
      my $visHnbr = getPOIInfoWithKey($self, 1);
      my $visFullAddr = getPOIInfoWithKey($self, 6);
      my $geocAddr = "";
      if ( (defined ($streetPrio)) and ($streetPrio == 1)) {
         if (defined($visAddr)) {
            $geocAddr = $visAddr;
            if (defined($visHnbr)) {
               $geocAddr .= " $visHnbr";
            }
         } elsif (defined($visFullAddr)) {
            $geocAddr = $visFullAddr;
         }

      } else {
         if ( defined($visFullAddr) ) {
            $geocAddr = $visFullAddr;
         } elsif ( defined($visAddr) ) {
            # TODO: addr before streetName, or vice versa?
            $geocAddr = $visAddr;
            if ( defined($visHnbr) ) {
               $geocAddr .= " $visHnbr";
            }
         }
      }

      #print "geocAddr $geocAddr\n";
      # XXX Formatting of address: must only include true plain housenumber..

      # zipCode
      my $zipCode = "";
      if ( defined( getPOIInfoWithKey($self, 2) ) ) {
         $zipCode = getPOIInfoWithKey($self, 2);
      }

      # zipArea = city
      my $zipArea = "";
      if ( defined( getPOIInfoWithKey($self, 5) ) ) {
         $zipArea = getPOIInfoWithKey($self, 5);
      }

      # Make sure there are no $geocFieldSep within any of the
      # fields
      $srcRef =~ s/$geocFieldSep/","/g;
      $topRegion =~ s/$geocFieldSep/","/g;
      $geocAddr =~ s/$geocFieldSep/","/g;
      $zipCode =~ s/$geocFieldSep/","/g;
      $zipArea =~ s/$geocFieldSep/","/g;

      print FILE "$srcRef$geocFieldSep$topRegion$geocFieldSep$geocAddr" .
                 "$geocFieldSep$zipCode$geocFieldSep$zipArea$geocFieldSep\n";
      
      return 1;
   }
   return 0;
}

sub getTopRegionName {
   my $self = $_[0];
   
   if ( defined($self->{m_country}) ) {

      # if country is USA, states are top region. Use state from POIInfo
      if ( $self->{m_country} == 9 ) {
         # find state in poi info, make uppercase and look in 
         # the %g_topRegionsFromStateCode hash
         my $stateCode = getPOIInfoWithKey( $self, 14 );
         if ( defined($stateCode) and
              defined($g_topRegionsFromStateCode{ uc($stateCode) }) ) {
            return $g_topRegionsFromStateCode{ uc($stateCode) };
         }
         # if we end up here, something is wrong
         #  - state code is missing or not correct
         # Return the state code string - it might be the full name..
         print "getTopRegionName: problem with state code";
         if ( defined($stateCode) ) { 
            print " $stateCode\n";
            return $stateCode;
         } else { 
            print " (undef)\n";
         }
      } else {
         if ( defined( $g_topRegionsFromCountryID{$self->{m_country}} ) ) {
            return $g_topRegionsFromCountryID{ $self->{m_country} };
         }
         # if we end up here, something is wrong
         #  - countryID not in %g_topRegionsFromCountryID.size hash
         print "getTopRegionName: problem with countryID ($self->{m_country})\n";
      }
   }

   print "Can not get top region for " . $self->{m_sourceReference} .  "\n";

   # return empty string (can't write undef to geocoding file)
   return "";
}




sub my_db_connect {
	my ($dbname, $dbhost, $dbuser, $dbpw) = @_;

	my $ldbh = DBI->connect("DBI:mysql:$dbname:$dbhost", $dbuser, $dbpw);
	return $ldbh;

}

sub my_db_disconnect {
	my ($ldbh) = @_;

	$ldbh->disconnect;

}


#
#  For each setMethod: the public method passes the request to the private one
#  Public value is $_[1], private value is $_[0]
#

# Get id
sub getId {
   my $self = $_[0];
   return $self->{m_id};
}

# Get/set source
sub getSource {
   my $self = $_[0];
   return $self->{m_source};
}

sub setSource {
   my $self = $_[0];
   if ( defined($_[1]) and $self->sourceIdValid($_[1]) ) {
      $self->{m_source} = $_[1];
      return 1;
   }
   return 0;
}

# Set no validation by data base.
sub noValidationByDb {
   my $self = $_[0];
   $self->{m_noValidationByDb}=1;
   return 1;
}

# Allow this POI to be added to the database without a position set, i.e. 
# no symbol coordinate.
sub allowNoPosInDb {
   my $self = $_[0];
   $self->{m_allowNoPosInDb}=1;
   return 1;
}

# Get/set added
sub getAdded {
   my $self = $_[0];
   return $self->{m_added};
}

sub setAdded {
   my $self = $_[0];
   my $newAdded = $_[1];
   if ( ! defined( $newAdded) ) {
      $self->{m_added} = $self->getCurrentTime();
      return 1;
   }
   elsif ( $self->dateStringValid($newAdded) ) {
      $self->{m_added} = $newAdded;
      return 1;
   }
   return 0;
}

# Get/set lastModified
sub getLastModified {
   my $self = $_[0];
   return $self->{m_lastModified};
}
sub setLastModified {
   my $self = $_[0];
   my $newLastModified = $_[1];

   if ( ! defined( $newLastModified) ) {
      $self->{m_lastModified} = $self->getCurrentTime();
      return 1;
   }
   elsif ( $self->dateStringValid($newLastModified) ) {
      $self->{m_lastModified} = $newLastModified;
      return 1;
   }
   return 0;
}

# Get/set symbol coordinate
sub getSymbolCoordinate {
   my $self =  $_[0];
   my $coordString = undef;
   if ( defined($self->{m_lat}) and defined($self->{m_lon}) ) {
      $coordString = "$self->{m_lat}$coordSep$self->{m_lon}";
   }
   return $coordString;
}
sub setSymbolCoordinate {
   my $self =  $_[0];
   my $lat =  $_[1];   
   my $lon =  $_[2];

   if ( defined($lat) and (length($lat) > 0) and
        defined($lon) and (length($lon) > 0) ) {
      $self->{m_lat} = $lat;
      $self->{m_lon} = $lon;
      return 1;
   }
   return 0;
}

# Get/set active
# REPLACED by inUse and deleted
#sub getActive {
#   my $self = $_[0];
#   return $self->{m_active};
#}
#sub setActive {
#   my $self = $_[0];
#   my $active = $_[1];
#   if ( defined($active) and $self->activeValueValid($active) ) {
#      $self->{m_active} = $active;
#      return 1;
#   }
#   return 0;
#}

# Get/set deleted 
sub getDeleted {
   my $self = $_[0];
   return $self->{m_deleted};
}
sub setDeleted {
   my $self = $_[0];
   my $deleted = $_[1];
   if ( defined($deleted) and $self->binaryValueValid($deleted) ) {
      $self->{m_deleted} = $deleted;
      return 1;
   }
   return 0;
}

# Get/set inUse 
sub getInUse {
   my $self = $_[0];
   return $self->{m_inUse};
}
sub setInUse {
   my $self = $_[0];
   my $inUse = $_[1];
   if ( defined($inUse) and $self->binaryValueValid($inUse) ) {
      $self->{m_inUse} = $inUse;
      return 1;
   }
   return 0;
}

# Get/set sourceReference
sub getSourceReference {
   my $self = $_[0];
   return $self->{m_sourceReference};
}

sub setSourceReference {
   my $self = $_[0];
   my $sourceRef = pack ("a*", $_[1]);
   if ( defined($_[1]) ) {
      
      $self->{m_sourceReference}=$sourceRef;
      return 1;
   }
   else {
      return 0;
   }
}

# Get/set comment
sub getComment {
   my $self = $_[0];
   return $self->{m_comment};
}
sub setComment {
   my $self = $_[0];
   my $comment = $_[1];

   if ( defined($comment) ) {
      $self->{m_comment} = $comment;
      return 1;
   }
   return 0;
}

# set GdfFeatureCode
sub setGdfFeatureCode {
   my $self = $_[0];
   my $featureCode = $_[1];

   if ( defined($featureCode) ) {
      $self->{m_gdfFeatureCode} = $featureCode;
      return 1;
   }
   return 0;
}

# get GdfFeatureCode
sub getGdfFeatureCode {
   my $self = $_[0];
   return $self->{m_gdfFeatureCode};
}

# Get/set country
sub getCountry {
   my $self = $_[0];
   return $self->{m_country};
}
sub setCountry {
   my $self = $_[0];
   my $country = $_[1];

   if ( defined($country) and $self->countryIdValid($country) ) {
      $self->{m_country} = $country;
      return 1;
   }
   return 0;
}

# Get/set validFromVersion
sub getValidFromVersion {
   my $self = $_[0];
   return $self->{m_validFromVersion};
}
sub setValidFromVersion {
   my $self = $_[0];
   my $verID = $_[1];
   if ( defined($verID) and $self->versionIdValid($verID) ) {
      $self->{m_validFromVersion} = $verID;
      return 1;
   }
   return 0;
}

# Get/set validToVersion
sub getValidToVersion {
   my $self = $_[0];
   return $self->{m_validToVersion};
}
sub setValidToVersion {
   my $self = $_[0];
   my $verID = $_[1];

   if ( defined($verID) and $self->versionIdValid($verID) ) {
      $self->{m_validToVersion} = $verID;
      return 1;
   }
   return 0;
}

# Edit entry points
sub addEntryPoint {
   my $self = $_[0];
   my $lat = $_[1];
   my $lon = $_[2];

   if ( defined($lat) and defined($lon) ) {
      # make sure we have both lat and lon, then push to m_entryPoints array
      if ( defined($lat) and (length($lat) > 0) and
           defined($lon) and (length($lon) > 0) ) {
         my $epString = "$lat$coordSep$lon";
         push @{$self->{m_entryPoints}}, $epString;
         return 1;
      }
   }
   return 0;
}
sub removeEntryPoint {
   my $self = $_[0];
   my $lat = $_[1];
   my $lon = $_[2];

   if ( defined($lat) and defined($lon) ) {
      my $epToRemove = $lat . "$coordSep" . $lon;
      #print "EP: POI ID: ".$self->getId().
      #" rm coord: $lat ". $coordSep ." $lon\n";
      
      my $foundEp = findClosestPoint($epToRemove, @{$self->{m_entryPoints}});
      if ( $foundEp ){
         (my $closeLat, my $closeLon) = split $coordSep, $foundEp;
         my $epDistMeters = getP2PdistMeters($lat, $lon, $closeLat, $closeLon);
         #print "EP: distance: $epDistMeters meters\n";
         if ($epDistMeters < 3.5){
            # This entry point is close enough.

            # Loop the m_entryPoints array and see which index it has.
            my $offset = -1;
            my $i = -1;
            foreach my $ep ( @{$self->{m_entryPoints}} ) {
               $i += 1;
               if ( $ep eq $foundEp ) {
                  $offset = $i;
               }
            }
            if ( $offset > -1 ) {
               my $removed = splice( @{$self->{m_entryPoints}}, $offset, 1);
               return 1;
            }
            else {
               die "Could not find entry point that was there just now!";
            }
         }
      }
   }
   return 0;
}
sub findClosestPoint {
   #print "=== EP:findClosestPoint ========================================\n";
   my $origPoint =  shift @_;
   my @pointsToCompare =  @_;

   my $closestPoint = "";
   my $closestMc2DistPow2 = 0xffffffff**2; 

   (my $origLat, my $origLon) = split $coordSep, $origPoint;
   
   foreach my $cmpPoint (@pointsToCompare){
      # Debug print
      if ( $origPoint eq $cmpPoint ){
         #print "EP: exact EP point match found: $cmpPoint\n";
      }
      

      (my $cmpLat, my $cmpLon) = split $coordSep, $cmpPoint;

      my $latDiff = abs($origLat-$cmpLat);
      my $lonDiff = abs($origLon-$cmpLon);
      
      my $distPow2 = ($latDiff**2+$lonDiff**2);
      #print "EP: closest dist^2 $closestMc2DistPow2, cmp dist^2 $distPow2\n";
      if ( $distPow2 < $closestMc2DistPow2 ){
         $closestMc2DistPow2 = $distPow2;
         $closestPoint = $cmpPoint;
      }
   }
   if ( $origPoint eq $closestPoint ){
      #print "EP: exact EP point match returned: $closestPoint\n";
   }
   elsif ( ! $closestPoint ){
      #print "EP: no close EP point match returned: $closestPoint\n";
   }
   else {
      #print "EP: closest EP point match returned: $closestPoint\n";  
   }
   #print "=== EP:findClosestPoint ========================================\n";
   return ($closestPoint);
}
sub removeAllEntryPoints {
   my $self = $_[0];
   # remove everything in entryPoints array
   splice( @{$self->{m_entryPoints}} );
}
sub getNbrEntryPoints {
   my $self = $_[0];
   if ( defined ($self->{m_entryPoints}) ){
      return ( scalar @{$self->{m_entryPoints}} );
   }
   return 0;
}

#use Unicode::String qw(latin1 utf8);
# sub pushIfUniq {
#    my @array;
#    undef @array;
#    if ( defined $_[0] ) {
#       @array = @$_[0];
#       print "A: defined\n"; 
#    }

#    my $value = $_[1];
   
#    print "A:", @array, "\n";
#    print "V:", $value, "\n";

#    my $exists = 0;

#    if ( ( defined (@array) ) && (scalar(@array) > 0)  ){
#       my $i=0;
#       print "A:" . scalar @array . "\n";
      
#       while ( ($i < scalar(@array) ) && ( ! $exists ) ){
#          if ( $array[$i] eq $value ){
#             $exists = 1;
#          }
#          $i = $i+1;
#       print $i . "\n";
#       }
#    }

#    if ( ! $exists ){
#       push ( @$_[0], $value);
#    }
# }

# Get/Edit poi info
sub getPOIInfoWithKey {
   my $self = $_[0];
   my $wantedKeyID = $_[1];
   
   if ( defined($wantedKeyID) ) {
      foreach my $info ( @{$self->{m_poiInfo}} ) {
         (my $keyID, my $lang, my $val) = split( $fieldSep, $info );
         if ( $wantedKeyID == $keyID ) {
            return $val;
         }
      }
   }
   return undef;
}

sub addPOIInfo {
   my $self = $_[0];
   my $keyID = $_[1];
   my $lang = $_[2];
   my $val = $_[3];
   # make sure we have keyID, lang, val (and that they are valid),
   # then push to m_poiInfo array
   if ( defined($keyID) and $self->poiInfoKeyIdValid($keyID) and
        defined($lang) and $self->languageValid($lang) and
        defined($val) and (length($val) > 0) ) {

      $val = pack("a*", $val);
      my $infoString = "$keyID" . "$fieldSep" . "$lang" . "$fieldSep" . "$val";
      #print "  B1:" . $infoString . "\n";

      # phone and fax should contain numbers
      if (($keyID == 4) or ($keyID == 7)) {
         if ($val !~ m/\d/) {
            # don't add
            # but don't fail
            return 1;
         }
      }

      #$infoString = "$keyID" . "$fieldSep" . "$lang" . "$fieldSep" . "$val";
#      print "  B1:" . latin1($infoString) . "\n";

      #print "  B2:" . $infoString . "\n";
#      print "  B1:" . pack("a*", $infoString) . "\n";
#      print "  B2:" . $infoString . "\n";
      
      # If this info is not already present, add it.
      my $exists = 0;
      if ( ( defined (@{$self->{m_poiInfo}}) ) && 
           ( scalar( @{$self->{m_poiInfo}}) > 0 )  ){
         my $i=0;
         #print "A:" . scalar @array . "\n";
         
         while ( ($i < scalar( @{$self->{m_poiInfo}}) ) && ( ! $exists ) ){
            if ( ${$self->{m_poiInfo}}[$i] eq $infoString ){
               $exists = 1;
            }
            $i = $i+1;
         }
      }
      
      if ( ! $exists ){
         push ( @{$self->{m_poiInfo}}, $infoString);
      }

      #pushIfUniq( $self->{m_poiInfo}, $infoString);
      #push @{$self->{m_poiInfo}}, $infoString;
      #print ( "  A:", @{$self->{m_poiInfo}},  "\n");
      #print ( "FILE SEP: " . "$fieldSep" . "\n");
      #my $aVar = "FILE SEP: " . "$fieldSep" . "\n";
      #print ( $aVar );
      #print ( "FILE SEP: " . getFieldSeparator() . "\n");
      #my $infoString2 = "$fieldSep" .  "$fieldSep";
      #print $infoString2 . "\n";
      return 1;
   }
   return 0;
}
sub removePOIInfo {
   my $self = $_[0];
   my $keyID = $_[1];
   my $lang = $_[2];
   my $val = $_[3];
   if ( defined($keyID) and defined($lang) and
        defined($val) and (length($val) > 0) ) {
      # Build info string, then loop the m_poiInfo array to see which
      # info to remove
      my $infoString = "$keyID$fieldSep$lang$fieldSep$val";
      my $offset = -1;
      my $i = -1;
      foreach my $info ( @{$self->{m_poiInfo}} ) {
         $i += 1;
         if ( $info eq $infoString ) {
            $offset = $i;
         }
      }
      if ( $offset > -1 ) {
         my $removed = splice( @{$self->{m_poiInfo}}, $offset, 1);
         return 1;
      }
   }
   return 0;
}

# Get/Edit names
sub getNameOfType {
   my $self = $_[0];
   my $givenType = $_[1];

   if ( defined($givenType) and $self->nameTypeValid($givenType) ) {
      foreach my $nameStr ( @{$self->{m_poiNames}} ) {
         (my $name, my $type, my $lang) = split( $fieldSep, $nameStr );
         if ( defined($name) and defined($type) and $type == $givenType ) {
            return $name;
         }
      }
   }
   return undef;
}
sub getNbrNames {
   my $self = $_[0];
   if ( defined (@{$self->{m_poiNames}}) ) {
      return ( scalar( @{$self->{m_poiNames}}) );
   }
   return 0;
}

sub getNbrNamesNonSynonyms {
   my $self = $_[0];
   my $countNames = 0;
   if ( defined (@{$self->{m_poiNames}}) ) {
      foreach my $nameStr ( @{$self->{m_poiNames}} ) {
         (my $name, my $type, my $lang) = split( $fieldSep, $nameStr );
         if ( $type != 7) {
            $countNames++;
         }
      }
      return ($countNames);
   }
   return 0;
}

sub getAllNames {
   my $self = $_[0];
   my @names;
   foreach my $nameStr ( @{$self->{m_poiNames}} ) {
      push @names, $nameStr;
   }
   return @names;
}
sub hasNameWithTypeAndLang {
   my $self = $_[0];
   my $givenName = $_[1];
   my $givenType = $_[2];
   my $givenLang = $_[3];
   
   if ( defined($givenName) and 
        defined($givenType) and defined($givenLang) ) {
      foreach my $nameStr ( @{$self->{m_poiNames}} ) {
         (my $name, my $type, my $lang) = split( $fieldSep, $nameStr );
         if ( ($name eq $givenName) and ($type == $givenType) and
              ($lang == $givenLang) ) {
            return 1;
         }
      }
   }
   return 0;
}

sub addName {
   my $self = $_[0];
   my $name = $_[1];
   my $type = $_[2];
   my $lang = $_[3];
   my $parseOnly = 0;
   if ( defined($_[4]) ) {
      $parseOnly = $_[4];
   }

   # make sure we have name, type, lang (and that they are valid)
   # push to m_poiNames if not parseOnly
   if ( defined($name) and (length($name) > 0) and
        defined($type) and $self->nameTypeValid($type) and
        defined($lang) and $self->languageValid($lang) ) {
      if ( ! $parseOnly ) {
         $name = pack("a*", $name);
         my $nameString = "$name$fieldSep$type$fieldSep$lang";
         #print "C:" . $nameString . "\n";

         # If this name is not already present, add it.
         my $exists = 0;
         if ( ( defined (@{$self->{m_poiNames}}) ) && 
              ( scalar( @{$self->{m_poiNames}}) > 0 )  ){
            my $i=0;
            while ( ($i < scalar( @{$self->{m_poiNames}}) ) && 
                    ( ! $exists ) ){
               if ( ${$self->{m_poiNames}}[$i] eq $nameString ){
                  $exists = 1;
               }
               $i = $i+1;
            }
         }
         if ( ! $exists ){
            push ( @{$self->{m_poiNames}}, $nameString);
         }
      }
      return 1;
   }
   return 0;
}

sub removeName {
   my $self = $_[0];
   my $name = $_[1];
   my $type = $_[2];
   my $lang = $_[3];
   my $parseOnly = 0;
   if ( defined($_[4]) ) {
      $parseOnly = $_[4];
   }

   if ( defined($name) and (length($name) > 0) and
        defined($type) and defined($lang) ) {
      # Build name string, then loop the m_poiNames array to see which
      # name to remove
      my $removeNameString = "$name$fieldSep$type$fieldSep$lang";
      my $offset = -1;
      my $i = -1;
      foreach my $nameStr ( @{$self->{m_poiNames}} ) {
         $i += 1;
         if ( $nameStr eq $removeNameString ) {
            $offset = $i;
         }
      }
      if ( $offset > -1 ) {
         if ( ! $parseOnly ) {
            my $removed = splice( @{$self->{m_poiNames}}, $offset, 1);
         }
         return 1;
      }
   }
   return 0;
}

sub updateName {
   my $self = $_[0];
   
   # first check parse-only (need both remove + add for update to succeed)
   if ( $self->removeName($_[1], $_[2], $_[3], 1) and 
        $self->addName($_[4], $_[5], $_[6], 1) ) {
      
      $self->removeName($_[1], $_[2], $_[3]);
      $self->addName($_[4], $_[5], $_[6]);
      return 1;
   }
   return 0;
}

# Edit poi types
sub getOnePOIType {
   my $self = $_[0];

   foreach my $type ( @{$self->{m_poiTypes}} ) {
      if ( defined($type) and $self->poiTypeValid($type) ) {
         return $type;
      }
   }
   return undef;
}
sub getPOITypes {
   my $self = $_[0];
   if ( defined ($self->{m_poiTypes}) ){
      return @{$self->{m_poiTypes}};
   }
   else {
      return undef;
   }
}
sub hasPOIType {
   my $self = $_[0];
   foreach my $type ( @{$self->{m_poiTypes}} ) {
      if ( defined($type) and ($type == $_[1]) ) {
         return 1;
      }
   }
   return 0;
}


sub getCategoriesRef {
   my $self = $_[0];
   if ( defined @{$self->{m_poiCategories}} ){
      return $self->{m_poiCategories};
   }
   else {
      return undef;
   }
}



sub addCategory {
   my $self = $_[0];
   my $poiCat = $_[1];

   # make sure the poi category is valid
   if ( defined($poiCat) and $self->poiCatValid($poiCat) ) {

      # Add this POI category if not already present.
      my $exists = 0;
      if ( ( defined (@{$self->{m_poiCategories}}) ) && 
           ( scalar( @{$self->{m_poiCategories}}) > 0 )  ){

         my $i=0;
         while ( ($i < scalar( @{$self->{m_poiCategories}}) ) && 
                 ( ! $exists ) ){
            if ( ${$self->{m_poiCategories}}[$i] == $poiCat ){
               $exists = 1;
            }
            $i = $i+1;
            #print $i . "\n";
         }
      }
      
      if ( ! $exists ){
         push ( @{$self->{m_poiCategories}}, $poiCat);
      }
      return 1;
   }
   return 0;
}


sub removeCategory {
   my $self = $_[0];
   my $poiCat = $_[1];

   if ( defined($poiCat) ) {
      # Loop the m_poiCategories array and see which category to remove
      my $offset = -1;
      my $i = -1;
      foreach my $cat ( @{$self->{m_poiCategories}} ) {
         $i += 1;
         if ( $cat eq $poiCat ) {
            $offset = $i;
         }
      }
      if ( $offset > -1 ) {
         my $removed = splice( @{$self->{m_poiCategories}}, $offset, 1);
         return 1;
      }
   }
   return 0;
}


sub addPOIType {
   my $self = $_[0];
   my $poiType = $_[1];

   # make sure the poi type is valid
   if ( defined($poiType) and $self->poiTypeValid($poiType) ) {

      # Add this POI type if not already present.
      my $exists = 0;
      if ( ( defined (@{$self->{m_poiTypes}}) ) && 
           ( scalar( @{$self->{m_poiTypes}}) > 0 )  ){

         my $i=0;
         while ( ($i < scalar( @{$self->{m_poiTypes}}) ) && ( ! $exists ) ){
            if ( ${$self->{m_poiTypes}}[$i] == $poiType ){
               $exists = 1;
            }
            $i = $i+1;
            #print $i . "\n";
         }
      }
      
      if ( ! $exists ){
         push ( @{$self->{m_poiTypes}}, $poiType);
      }
      return 1;
   }
   return 0;
}
sub removePOIType {
   my $self = $_[0];
   my $poiType = $_[1];

   if ( defined($poiType) ) {
      # Loop the m_poiTypes array and see which type to remove
      my $offset = -1;
      my $i = -1;
      foreach my $type ( @{$self->{m_poiTypes}} ) {
         $i += 1;
         if ( $type eq $poiType ) {
            $offset = $i;
         }
      }
      if ( $offset > -1 ) {
         my $removed = splice( @{$self->{m_poiTypes}}, $offset, 1);
         return 1;
      }
   }
   return 0;
}

# Check/set artificial
sub isArtificial {
   my $self = $_[0];
   if ( (defined $self->{m_artificial}) and
         $self->{m_artificial} ) {
      return 1;
   }
   return 0;
}

# Check externalIdentifier
sub getExternalIdentifier {
   my $self = $_[0];
   if ( (defined $self->{m_externalIdentifier}) and
         $self->{m_externalIdentifier} ) {
      return $self->{m_externalIdentifier};
   }
   return undef;
}

# Time
sub getCurrentTime {
   my $self = $_[0];

   if ( defined($self->{m_dbh}) ) {
      my $timeQuery = "SELECT NOW();";
      my $sth = $self->{m_dbh}->prepare( $timeQuery );
      $sth->execute;
      return ( $sth->fetchrow() );
   }
   return undef;
}



#
# Check some attributes' validity
#
sub binaryValueValid {
   my $self = $_[0];
   my $binaryVal = $_[1];

   if ( ($binaryVal == 0) or ($binaryVal == 1) ) {
      return 1;
   }
   return 0;
}

sub countryIdValid {
   my $self = $_[0];
   my $countryID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) FROM POICountries;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($countryID >= $min) and ($countryID <= $max) ) {
            return 1;
         }
      }
      else {
         #print " No validation\n";
         return 1;
      }
   }
   return 0;
}

sub dateStringValid {
   my $self = $_[0];

   my $strLen = length( $_[1] );
   if ( $strLen == 19 ) {
      return 1;
   }
   return 0;
}

sub poiInfoKeyIdValid {
   my $self = $_[0];
   my $keyID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) FROM POIInfoKeys;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($keyID >= $min) and ($keyID <= $max) ) {
            return 1;
         }
      }
      else {
         return 1;
      }
   }
   return 0;
}

sub languageValid {
   my $self = $_[0];
   my $langID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) " .
             "FROM POINameLanguages;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($langID >= $min) and ($langID <= $max) ) {
            return 1;
         }
      }
      else {
         return 1;
      }
   }
   return 0;
}

sub nameTypeValid {
   my $self = $_[0];
   my $typeID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) FROM POINameTypes;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($typeID >= $min) and ($typeID <= $max) ) {
            return 1;
         }
      }
      else {
         return 1;
      }
   }
   return 0;
}

sub poiTypeValid {
   my $self = $_[0];
   my $typeID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) FROM POITypeTypes;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($typeID >= $min) and ($typeID <= $max) ) {
            return 1;
         }
      }
      else {
         return 1;
      }
   }
   return 0;
}


sub poiCatValid {
   my $self = $_[0];
   my $catID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $catQuery = "SELECT * FROM POICategoryTypes WHERE catID = $catID;";
         my $sth = $self->{m_dbh}->prepare( $catQuery );
         $sth->execute || die "Query execute failed: $catQuery";
         if ( $sth->rows == 0 ){
            # No such category exists.
            return 0;
         }
         elsif ( $sth->rows > 1 ){
            # Very strange result, exit.
            errprint "Strange categories in database";
            exit 1;
         }
         else {
            # All fine.
            return 1;
         }
      }
      else {
         # Not using data base for validation.
         return 1;
      }
   }
   return 0;
}


sub sourceIdValid {
   my $self = $_[0];
   my $sourceID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) FROM POISources;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($sourceID >= $min) and ($sourceID <= $max) ) {
            return 1;
         }
      }
      else {
         return 1;
      }
   }
   return 0;
}

sub versionIdValid {
   my $self = $_[0];
   my $versionID = $_[1];

   if ( defined($self->{m_dbh}) ) {
      if ( ! defined ($self->{m_noValidationByDb}) ){
         my $minmaxQuery = "SELECT min(ID), max(ID) FROM EDVersion;";
         my $sth = $self->{m_dbh}->prepare( $minmaxQuery );
         $sth->execute;
         (my $min, my $max) = $sth->fetchrow();
         if ( ($versionID >= $min) and ($versionID <= $max) ) {
            return 1;
         }
      }
      else {
         return 1;
      }
   }
   return 0;
}

sub infoKeyIsDynamic {
   my $self = $_[0];
   my $keyID = $_[1];

   if ( defined ($self->{m_dbh}) ) {
      my $keyIdQuery = "SELECT isDynamic FROM POIInfoKeys WHERE " .
         "ID = $keyID";
      my $sth = $self->{m_dbh}->prepare( $keyIdQuery );
      $sth->execute();
      (my $isDynamic) = $sth->fetchrow();
      if (defined $isDynamic) {
         if (defined $isDynamic and $isDynamic == 1) {
            return 1;
         } else {
            return 0;
         }
      }
   }
   return undef;
}

sub formatGDFPOIInfoValue {
   my $infoValue = $_[0];
   my $infoKey = $_[1];

   $infoValue=~s/\r//g;   # Remove all ASCII:13 signs.
   $infoValue=~s/\n//g;   # Remove all ASCII:10 signs.

   if ( $infoKey == 0 ){
      $infoValue=~s/\/\///g; # Remove all "//" from visiting address.
   }
   if ( $infoKey == 4 ){
      $infoValue=~s/\(0\)//g;# Remove all "(0)" from phone numbers
      $infoValue=~s/\(//g;   # Remove all "(" from phone numbers
      $infoValue=~s/\)//g;   # Remove all ")" from phone numbers
      $infoValue=~s/,/ /g;   # Replace all "," with " " in phone numbers.
   }
   return $infoValue;
}

sub getInfoKeyIDforGDFPOI {
   my $infoTypeStr = $_[0];
   if ( ! scalar(%m_gdfPOIInfoTypes) ){
      #Initiate info type id variable, since it is empty
      $m_gdfPOIInfoTypes{"STREETNAME"}  = 0; # Vis. address
      $m_gdfPOIInfoTypes{"HOUSENUMBER"} = 1; # Vis. house nbr
      $m_gdfPOIInfoTypes{"POSTALCODE"}  = 2; # Vis. zip code
      $m_gdfPOIInfoTypes{"PHONENUMBER"} = 4; # Phone
      $m_gdfPOIInfoTypes{"PLACENAME"}   = 5; # Vis. zip area
      $m_gdfPOIInfoTypes{"EMAIL"}       = 8; # Email
      $m_gdfPOIInfoTypes{"URL"}         = 9; # URL
      $m_gdfPOIInfoTypes{"BRANDNAME"}   = 10; # Brandname
      $m_gdfPOIInfoTypes{"CITY_CENTRE_DISPLAY_CLASS"} = 47; # Display class
      $m_gdfPOIInfoTypes{"IMPORTANCE"} = 87; # Importance attribute
      $m_gdfPOIInfoTypes{"MAJOR_ROAD_FEATURE"} = 88; # Major road feature
      $m_gdfPOIInfoTypes{"STAR_RATE"} = 105; # Star rate
      $m_gdfPOIInfoTypes{"RATING_SOURCE"} = 112; # Rating source for star rate
      $m_gdfPOIInfoTypes{"GEOCODING_ACCURACY_LEVEL"} = 113; # Geocoding accuracy level
   }
   if ( defined($m_gdfPOIInfoTypes{$infoTypeStr}) ) {
      return $m_gdfPOIInfoTypes{$infoTypeStr};
   } else {
      errprint "Unknown info type '$infoTypeStr' " .
          "- add it in the m_gdfPOIInfoTypes hash";
      die("exit");
   }
}

sub getNameType {
   my $dbh =  $_[0];
   my $nameTypeStr = lc $_[1];
   if ( ! scalar(%m_nameTypes) ){
      #Initiate name type id variable, since it is empty
      my $typeSth = $dbh->prepare("SELECT ID, typeName FROM POINameTypes;");
      $typeSth->execute;
      while ( (my $ID, my $typeName) = $typeSth->fetchrow()) {
         $m_nameTypes{lc $typeName} = $ID;
      }
      # Copied from addwasp.pl to handle name type parsing
      # GDF POI file (ItemTypes::getNameTypeAsString)
      $m_nameTypes{"abb"} = getNameType($dbh, "abbreviationName");
      $m_nameTypes{"an"} = getNameType($dbh, "alternativeName");
      $m_nameTypes{"on"} = getNameType($dbh, "officialName");
      $m_nameTypes{"rn"} = getNameType($dbh, "roadNumber");
      $m_nameTypes{"nbr"} = getNameType($dbh, "roadNumber");
      $m_nameTypes{"enbr"} = getNameType($dbh, "exitNumber");
      $m_nameTypes{"syn"} = getNameType($dbh, "synonymName");
      $typeSth->finish;
   }
   if ( defined($m_nameTypes{$nameTypeStr}) ) {
      return $m_nameTypes{$nameTypeStr};
   } else {
      errprint "Unknown name type '$nameTypeStr' " .
          "- add it in the m_nameTypes hash";
      die ("exit");
   }
}

sub getNameLang {
   my $dbh =  $_[0];
   my $langStr = lc $_[1];
   if ( ! scalar(%m_nameLang) ){
      # Initiate the language ID variable, since its empty.

      my $langSth = 
          $dbh->prepare("SELECT ID, langName FROM POINameLanguages;");
      $langSth->execute;
      while ( (my $ID, my $nameLang) = $langSth->fetchrow()) {
         $m_nameLang{lc $nameLang} = $ID;
      }
      # Copied from addwasp.pl to handle special abbreviations of lang
      # - parsing GDF POI file (it uses the 4th column of 
      #   LangTypes::languageAsString table)
      $m_nameLang{"se"} = getNameLang($dbh, "swedish");
      $m_nameLang{"swe"} = getNameLang($dbh, "swedish");
      $m_nameLang{"uk"} = getNameLang($dbh, "english");
      $m_nameLang{"gb"} = getNameLang($dbh, "english");
      $m_nameLang{"eng"} = getNameLang($dbh, "english");
      $m_nameLang{"en"} = getNameLang($dbh, "english");
      $m_nameLang{"de"} = getNameLang($dbh, "german");
      $m_nameLang{"deu"} = getNameLang($dbh, "german");
      $m_nameLang{"ger"} = getNameLang($dbh, "german");
      $m_nameLang{"no"} = getNameLang($dbh, "norwegian");
      $m_nameLang{"nor"} = getNameLang($dbh, "norwegian");
      $m_nameLang{"spa"} = getNameLang($dbh, "spanish");
      $m_nameLang{"fre"} = getNameLang($dbh, "french");
      $m_nameLang{"dut"} = getNameLang($dbh, "dutch");
      $m_nameLang{"dan"} = getNameLang($dbh, "danish");
      $m_nameLang{"fin"} = getNameLang($dbh, "finnish");
      $m_nameLang{"ita"} = getNameLang($dbh, "italian");
      $m_nameLang{"wel"} = getNameLang($dbh, "welch");
      $m_nameLang{"por"} = getNameLang($dbh, "portuguese");
      $m_nameLang{"alb"} = getNameLang($dbh, "albanian");
      $m_nameLang{"baq"} = getNameLang($dbh, "basque");
      $m_nameLang{"cat"} = getNameLang($dbh, "catalan");
      $m_nameLang{"fry"} = getNameLang($dbh, "frisian");
      $m_nameLang{"gle"} = getNameLang($dbh, "irish");
      $m_nameLang{"gai"} = getNameLang($dbh, "irish");
      $m_nameLang{"glg"} = getNameLang($dbh, "galician");
      $m_nameLang{"ltz"} = getNameLang($dbh, "letzeburgesch");
      $m_nameLang{"roh"} = getNameLang($dbh, "raeto romance");
      $m_nameLang{"scr"} = getNameLang($dbh, "serbo croatian");
      $m_nameLang{"slv"} = getNameLang($dbh, "slovenian");
      $m_nameLang{"val"} = getNameLang($dbh, "valencian");
      $m_nameLang{"ame"} = getNameLang($dbh, "english (us)");
      $m_nameLang{"cze"} = getNameLang($dbh, "czech");
      $m_nameLang{"hun"} = getNameLang($dbh, "hungarian");
      $m_nameLang{"gre"} = getNameLang($dbh, "greek");
      $m_nameLang{"pol"} = getNameLang($dbh, "polish");
      $m_nameLang{"slo"} = getNameLang($dbh, "slovak");
      $m_nameLang{"rus"} = getNameLang($dbh, "russian");
      $m_nameLang{"grl"} = getNameLang($dbh, "greek latin syntax");
      $m_nameLang{"inv"} = getNameLang($dbh, "invalidLanguage");
      $m_nameLang{"rul"} = getNameLang($dbh, "russian latin syntax");
      $m_nameLang{"tur"} = getNameLang($dbh, "turkish");
      $m_nameLang{"ara"} = getNameLang($dbh, "arabic");
      $m_nameLang{"xxx"} = getNameLang($dbh, "invalidLanguage");
      $m_nameLang{"chi"} = getNameLang($dbh, "chinese");
      $m_nameLang{"chl"} = getNameLang($dbh, "chinese latin syntax");
      $m_nameLang{"est"} = getNameLang($dbh, "estonian");
      $m_nameLang{"lav"} = getNameLang($dbh, "latvian");
      $m_nameLang{"lit"} = getNameLang($dbh, "lithuanian");
      $m_nameLang{"tha"} = getNameLang($dbh, "thai");
      $m_nameLang{"bul"} = getNameLang($dbh, "bulgarian");
      $m_nameLang{"cyt"} = getNameLang($dbh,"cyrillic transcript");
      $m_nameLang{"ind"} = getNameLang($dbh,"indonesian");
      $m_nameLang{"may"} = getNameLang($dbh,"malay");
      $m_nameLang{"bun"} = getNameLang($dbh, "bulgarian latin syntax");
      $m_nameLang{"hrv"} = getNameLang($dbh,"croatian");
      $m_nameLang{"bel"} = getNameLang($dbh, "belarusian");
      $m_nameLang{"bos"} = getNameLang($dbh, "bosnian");
      $m_nameLang{"mkd"} = getNameLang($dbh, "macedonian");
      $m_nameLang{"mol"} = getNameLang($dbh, "moldavian");
      $m_nameLang{"ron"} = getNameLang($dbh, "romanian");
      $m_nameLang{"srp"} = getNameLang($dbh, "serbian");
      $m_nameLang{"sla"} = getNameLang($dbh, "slavic");
      $m_nameLang{"ukr"} = getNameLang($dbh, "ukrainian");
      $m_nameLang{"bet"} = getNameLang($dbh, "belarusian latin syntax");
      $m_nameLang{"mat"} = getNameLang($dbh, "macedonian latin syntax");
      $m_nameLang{"scc"} = getNameLang($dbh, "serbian latin syntax");
      $m_nameLang{"ukl"} = getNameLang($dbh, "ukrainian latin syntax");
      $m_nameLang{"mlt"} = getNameLang($dbh, "maltese");
      $m_nameLang{"thl"} = getNameLang($dbh, "thai latin syntax");

      $langSth->finish;
   }
   if ( defined($m_nameLang{$langStr}) ) {
      return $m_nameLang{$langStr};
   } else {
      errprint "Unknown language '$langStr' " .
          "- add it in the m_nameLang hash";
      die ("exit");
   }
}




1;

=head1 NAME

POIObject - class representing Wayfinder POI

=head1 USE

Include into your perl file using the combination of:
   use lib "${BASEGENFILESPATH}/script/perllib";
   use POIObject;
pointing to the directory where the perl modules are stored

=head1 DESCRIPTION

The POI Object is a class for representing POIs in perl.
A POI object can hold all attributes that are present for
POIs in the WASP database. It also contains methods for
e.g. creating POI objects from WASP database or any of 
Wayfinder's POI import formats. 

=head1 ATTRIBUTES

The POI Object class has the following private attributes
for holding all attributes of a POI. For more description
of possible values of the attributes, please see 
documentation of the WASP SQL database for POIs.

The attributes are private and can therefore only be
accessed and edited via the provided get-, set-, add- and
remove functions.

$m_id
   The WASP POI ID. An unsigned integer value as defined
   in the POIMain table. Can not be altered manually, but
   only when reading from or writing to WASP.

$m_source
   The source of the POI. An unsigned integer value, as
   defined in the POISources table.

$m_sourceReference
   Reference id of the POI from the source.

$m_deleted
   Whether the POI is deleted or not. Boolean value 0 or 1.

$m_inUse
   Whether the POI is in use or not. Boolean value 0 or 1.

$m_added
   The time the POI was added to WASP. String with pattern
   "YYYY-MM-DD HH:MM:SS".

$m_lastModified
   The time the POI was last modified in WASP. String with
   pattern "YYYY-MM-DD HH:MM:SS".

$m_lat
   The POI symbol coordinate latitude.

$m_lon
   The POI symbol coordinate longitude.

$m_comment
   Any comment.

$m_country
   The country of the POI. An unsigned interger value, as
   defined in the POICountries table.

$m_validFromVersion
   The map version the POI is valid from. An unsigned
   integer value as defined in the EDVersion table.

$m_validToVersion
   The map version the POI is valid to. An unsigned
   integer value as defined in the EDVersion table.

@m_entryPoints
   Array with all entry points of the POI. Each entry 
   point is a string "epLat,epLon". Corresponds to the 
   POIEntryPoints table.

@m_poiInfo
   Array with all POI info for the POI. Each info is a 
   string "keyId<¡>lang<¡>val". Corresponds to the POIInfo
   table, with keyId unsigned integer value as defined in 
   the POIInfoKeys table and lang unsigned integer value
   as defined in the POINameLanguages table.

@m_poiNames
   Array with all names of the POI. Each name is a string
   "name<¡>type<¡>lang". Corresponds to the POINames 
   table, with type unsigned integer value as defined in
   the POINameTypes table and lang unsigned integer as
   defined in the POINameLanguages table.

@m_poiTypes
   Array with all POI types of the POI. Each type is a 
   simple integer value as defined in the POITypes table.


=head3 String separators

In combined attribute strings different separators used as
seen above. They are defined in the POI Object class:
 $coordSep , 
   The comma character is used in coordinate strings.
 $fieldSep <¡> 
   The inverted exclamation mark within less-than and 
   greater-than signs is used in other array strings.

=head3 Database connection

The POI Object class has one attribute for storing 
the SQL database connection to use when communicating with
WASP.  This attribute must be set before calling functions
that talk to the database (most do in some way).

=head3 Other help attributes

$m_language
   Default language used for name strings and info values
   (where language matters) when reading and adding 
   attributes from SPIF.

%g_topRegionsFromCountryID
   Hash that translates countryID to topRegionName.
   
%g_topRegionsFromStateCode
   Hash that translates uppercase 2-letter USA state codes
   to topRegionName.

=head1 FUNCTIONS

=head2 Constructor

newObject( $poiID )
   Creates an empty POI object. By giving an id as inpara-
   meter, the $m_id attribute is set, else the $m_id will
   temporarily be set to MAX_UINT32.
   
   my $poiID = 10149444;
   ...
   my $poiObject = POIObject->newObject( $poiID );

   If the wish is to create the POI object from WASP, the
   id inparam must be set, else the object does not know
   what info to read from WASP.

=head2 Set database connection

Build the database connection $dbh in your perl file and 
parse that on to the new (empty) POI object with:
   $poiObject->setDBH($dbh);

=head2 Set help attributes

setDefaultLang( $languageID )
   Set the default language.

=head2 Read functions

Functions that create a POI object either from WASP or
from any of Wayfinder's POI import formats. Requires that
the database connection is set.

readFromWASP()
   Fill a POI object with data from WASP. The $m_id
   attribute must be set, else the object does not know 
   what info to read from WASP.

   my $poiObject = POIObject->newObject( $poiID );
   $poiObject->setDBH($dbh);
   $poiObject->readFromWASP();

readFromCPIF( $fileName, $readFromPos )
   Create myself from Wayfinder complex POI import format
   (CPIF), start reading the $fileName at $readFromPos.
   If deleted is not read from the CPIF file, it is set to
   default 0. If inUse is not read, it is set to 1. 
	Likewise sourceReference is by default
   set to the poiMarkerID on the CPIF rows.
   Returns the position in the file where to start reading
   next POI.
   Handles attributes covering > 1 line.
   
   my $poiObject = POIObject->newObject( );
   $poiObject->setDBH($dbh);
   # Read first POI Object from file
   my $filePos = 0;
   my $nextPos = $poiObject->readFromCPIF( $CPIF_FILENAME, $filePos );

readFromSPIF( $fileName, $readFromPos )
   Create myself from Wayfinder simple POI import format
   (SPIF), start reading the $fileName at $readFromPos.
   Possible to set default language to be used for name
   string and info values.
   Returns the position in the file where to start reading
   next POI.
   
   my $poiObject = POIObject->newObject( );
   $poiObject->setDBH($dbh);
   $poiObject->setDefaultLang(10);  # norwegian
   # Read first POI Object from file
   my $filePos = 0;
   my $nextPos = $poiObject->readFromSPIF( $SPIF_FILENAME, $filePos );

readFromGDFPOIFormat( $fileName, $readFromPos )
   Create myself from GDF POI file format (the one created in
   map generation), start reading the $fileName at $readFromPos.
   Returns the position in the file where to start reading
   next POI.

   my $poiObject = POIObject->newObject( );
   $poiObject->setDBH($dbh);
   # Read first POI Object from file
   my $filePos = 0;
   my $nextPos = $poiObject->readFromGDFPOIFormat( $FILENAME, $filePos );


=head2 Write functions

Functions that write a POI object either to WASP or to any
of Wayfinder's POI import formats. Requires that the 
database connection is set.

addToWASP()
   The POI object adds itself to WASP. The $m_id variable
   is updated to be the (new) database poiID. If addition
   to the POIMain table fails, no info is added to any of
   the other tables.
   Returns:
      The new database poiID, or
      -2 if checking if the source reference is already present and 
         not adding the POI because of that, or
      0 if unsuccessful.

   $newPOIID = $poiObject->addToWASP();

   The following attributes MUST exist, else the POI is
   not added to WASP:
   - source
   - sourceReference
   - country
   - symbol coordinate
   - one official name
   - one poi type

writeCPIF($CPIF_FILENAME)
   The POI object is written to file in Wayfinder complex
   POI import format (CPIF). The file $CPIF_FILENAME is 
   written with append. The sourceReference is used as
   poiMarkerID on the CPIF rows, the POI is not written
   if sourceReference is undefined.

writeSPIF($SPIF_FILENAME)
   The POI object is written to file in Wayfinder simple
   POI import format (SPIF). The file $SPIF_FILENAME is 
   written with append. 

writeGeocodeFile($GEOC_FILENAME)
   Info needed for geocoding is written to file. The file
   $GEOC_FILENAME is written with append.
   srcRef;topRegionName;fullAddress;zipCode;city

=head2 Functions to get/set attributes

Methods for getting and setting the scalar attributes of
the POI object. Note that the set-methods overwrite any 
old attribute value of the POI object and return true (1)
if the attribute was set successfully, false (0) if not.
Most set-methods require the database connection, to check
that setValues are valid before editing the POI object.

=head3 getDeleted()

   Returns the value of $m_deleted as boolean integer with
   values 0 or 1.

=head3 getInUse()

   Returns the value of $m_inUse as boolean integer with
   values 0 or 1.

=head3 getAdded()
   
   Returns the value of $m_added as datetime string.

=head3 noValidationByDb()

   Makes this object not contact the database for validating its
   attributes.

= head3 allowNoPosInDb()

   Allow this POI to be added to the database without a position set, i.e. 
   no symbol coordinate.


=head3 getComment()

   Returns the value of m_comment.

=head3 getCountry()
   
   Returns the value of $m_country as unsigned integer.

=head3 getId()
   
   Returns the value of $m_id as unsigned integer.

=head3 getLastModified()

   Returns the value of $m_lastModified as datetime string.

=head3 getNameOfType( $nameType )

   Returns the first name in the @m_poiNames array that
   matches the given nameType (returns just the name!).

=head3 getNbrNames()

   Returns the number of names in the vector with all names.

=head3 getNbrNamesNonSynonyms()

   Returns the number of names in the vector with all names, not
   counting synonym names.

=head3 getAllNames()

   Returns a vector with all names, the name strings are given
   as they are stored in the @m_poiNames array.

=head3 hasNameWithTypeAndLang( $name, $nameType, $nameLang)

   Checks if the POI has the given name with given name
   type id and name language id. Returns 1 if it does, 
   0 if not.

=head3 getPOIInfoWithKey( $keyID )

   Returns the first info value in the @m_poiInfo array that
   matches the given keyID (returns just the info value!).

=head3 getOnePOIType()

   Returns the first type in the @m_poiTypes array.

=head3 getPOITypes()

   Returns the array with types (the @m_poiTypes array).

=head3 hasPOIType( $type )

   Checks if the POI has the given POI type id. Returns 1
   if it does, 0 if not.

=head3 getSource()
   
   Returns the value of $m_source as unsigned integer.

=head3 getSourceReference()
   
   Returns the value of $m_sourceReference.

=head3 getSymbolCoordinate()

   Returns a coordinate string with "$m_lat,$m_lon".

=head3 getValidFromVersion()
   
   Returns the value of $m_validFromVersion as unsigned
   integer.

=head3 getValidToVersion()
   
   Returns the value of $m_validToVersion as unsigned
   integer.

=head3 setDeleted($deletedVal)
   
   Sets the $m_deleted attribute. First, makes sure that 
   $deletedVal is a valid value.

=head3 setInUse($inUseVal)
   
   Sets the $m_inUse attribute. First, makes sure that 
   $inUseVal is a valid value.

=head3 setAdded($addedDateTime)
   
   Sets the $m_added attribute. If $addedDateTime is left
   out, the database calculates NOW().

=head3 setComment($commentVal)
   
   Sets the $m_comment attribute.

=head3 setCountry($countryVal)
   
   Sets the $m_country attribute, First, makes sure that 
   $countryVal is a valid value.

=head3 setLastModified($modifiedDateTime)
   
   Sets the $m_lastModified attribute. If $modifiedDateTime
   is left out, the database calculates NOW().

=head3 setSource($sourceVal)
   
   Sets the $m_source attribute. First, makes sure that
   $sourceVal is a valid value.

=head3 setSourceReference($srcRefVal)
   
   Sets the $m_soruceReference attribute.

=head3 setSymbolCoordinate($lat, $lon)
   
   Sets the $m_lat and $m_lon attributes.

=head3 setValidFromVersion($versionVal)
   
   Sets the $m_validFromVersion attribute. First, makes 
   sure that $versionVal is a valid value.

=head3 setValidToVersion($versionVal)
   
   Sets the $m_validToVersion attribute. First, makes sure
   that $versionVal is a valid value.


=head2 Functions to add/remove/update attributes

Edit the different array attributes of the POI object.
Requires the database connection, to check that addValues
are valid before editing the POI object. All methods
return true (1) if they are successful and false (0) if
not.

addEntryPoint($epLat, $epLon)
   Adds one entry point coordinate to the @m_entryPoints
   array.

removeEntryPoint($epLat, $epLon)
   Removes one specified entry point coordinate from the
   @m_entryPoints array.

removeAllEntryPoints()
   Removes all entry point coordinates from the
   @m_entryPoints array.

getNbrEntryPoints
   Return the number of EPs in the @m_entryPoints array.

addPOIInfo($infoKeyID, $infoLang, infoVal)
   Adds one POI info to the @m_poiInfo array.

removePOIInfo($infoKeyID, $infoLang, infoVal)
   Removes one specific POI info from @m_poiInfo array.

addName($name, $nameType, $nameLang, ($parseOnly))
   Adds one name to the @m_poiNames array. Returns 1 if
   successful.
   If the parseOnly-inparam is set, the name is not added,
   the method returns 1 if the given name can be added and
   0 if not.

removeName($name, $nameType, $nameLang, ($parseOnly))
   Removes one specific anme from the @m_poiNames array.
   Returns 1 if successful.
   If the parseOnly-inparam is set, no name is removed,
   the method returns 1 if the given name exists and can
   be removed and 0 if not.

updateName($oldName, $oldNameType, $oldNameLang,
           $newName, $newNameType, $newNameLang)
   Updates one name in the @m_poiNames array. Really, the
   old one is removed and the new one added.

addPOIType($type)
   Adds one POI type to the @m_poiTypes array.

removePOIType($type)
   Removes a specific POI type from the @m_poiTypes array.


=head2 Static metods
getNameLang($dbh, $langStr)
   Returns the database ID of the language identified by $langStr.

getNameType($dbh, $nameTypeStr)
   Returns the database ID of the name type identified by $nameTypeStr.

getInfoKeyIDforGDFPOI( $infoType )
   Returns the info key ID of the poi info type
   identified by $infoType. Used in GDF POI file.

formatGDFPOIInfoValue( $infoValue, $keyID )
   Removes dangerous signs from GDF POI info values.

=head2 Misc. private functions

myAddGDFPOIAttribute( $attrLine, $poiItemID )
   Help method to readFromGDFPOIFormat. Add one attribute 
   read from GDF POI file to myself. The $attrLine is the
   line read from POI file, with the poiWaspPrefix and 
   poi mcm item id removed.

myAddGDFPOINames( $allNames, $poiItemID )
   Help method to readFromGDFPOIFormat. Add all names to myself.

myAddGDFPOIInfo( $infoType, $infoValue, $poiItemID )
   Help method to readFromGDFPOIFormat. Add a poi info attribute
   to myself.

myAddCPIFAttribute( $attrType, $attrVal )
   Help method to readFromCPIF. Add one attribute read
   from CPIF to myself.

myAddSPIFAttributes( @attributes )
   Help method to readFromSPIF. Add attributes that was
   read from SPIF file to myself.

my_db_connect 
	connects to db

my_db_disconnect
	disconnects from db


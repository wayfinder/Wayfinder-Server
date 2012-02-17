#!/usr/bin/perl -w
#
#

package PerlCountryCodes;

use strict;

require Exporter;
our @ISA       = qw( Exporter );
our @EXPORT    = qw( &getCountryFromAlpha2 &getGmsNameFromAlpha2 
                     &getAlpha2FromCountry
                     &getDrivingSide
                     &getCountryOrder );





# Hash keyed on ISO 3166 alpha2 country code. Translates to:
# country=  The name of the country directory on BASEGENFILESPATH/countries,
#           re-used for basename of country ar+co xml files, 
#           map_ident of co- and o-maps, country polygon mif file..
# gmsName=  The name of the language when calling GMS (if other than country)
my %countryCode = ();

# TODO perhaps get this info from WASP POICountries instead!
# else we will have problems if we make changes/additions 
# in POICountries

$countryCode{"ae"}{"gmsName"} = "united_arab_emirates";
$countryCode{"gb"}{"gmsName"} = "england";

$countryCode{"ad"}{"country"} 	= "andorra";
$countryCode{"ae"}{"country"} 	= "uae";
$countryCode{"af"}{"country"} 	= "afghanistan";
$countryCode{"ag"}{"country"} 	= "antigua_and_barbuda";
$countryCode{"ai"}{"country"} 	= "anguilla";
$countryCode{"al"}{"country"} 	= "albania";
$countryCode{"am"}{"country"} 	= "armenia";
$countryCode{"an"}{"country"} 	= "netherlands_antilles";
$countryCode{"ao"}{"country"} 	= "angola";
$countryCode{"aq"}{"country"} 	= "antarctica";
$countryCode{"ar"}{"country"} 	= "argentina";
$countryCode{"as"}{"country"} 	= "american_samoa";
$countryCode{"at"}{"country"} 	= "austria";
$countryCode{"au"}{"country"} 	= "australia";
$countryCode{"aw"}{"country"} 	= "aruba";
$countryCode{"az"}{"country"} 	= "azerbaijan";
$countryCode{"ba"}{"country"} 	= "bosnia";
$countryCode{"bb"}{"country"} 	= "barbados";
$countryCode{"bd"}{"country"} 	= "bangladesh";
$countryCode{"be"}{"country"} 	= "belgium";
$countryCode{"bf"}{"country"} 	= "burkina_faso";
$countryCode{"bg"}{"country"} 	= "bulgaria";
$countryCode{"bh"}{"country"} 	= "bahrain";
$countryCode{"bi"}{"country"} 	= "burundi";
$countryCode{"bj"}{"country"} 	= "benin";
$countryCode{"bm"}{"country"} 	= "bermuda";
$countryCode{"bn"}{"country"} 	= "brunei_darussalam";
$countryCode{"bo"}{"country"} 	= "bolivia";
$countryCode{"br"}{"country"} 	= "brazil";
$countryCode{"bs"}{"country"} 	= "bahamas";
$countryCode{"bt"}{"country"} 	= "bhutan";
$countryCode{"bw"}{"country"} 	= "botswana";
$countryCode{"by"}{"country"} 	= "belarus";
$countryCode{"bz"}{"country"} 	= "belize";
$countryCode{"cd"}{"country"} 	= "dr_congo";
$countryCode{"cf"}{"country"} 	= "central_african_republic";
$countryCode{"cg"}{"country"} 	= "congo";
$countryCode{"ch"}{"country"} 	= "switzerland";
$countryCode{"ci"}{"country"} 	= "ivory_coast";
$countryCode{"ck"}{"country"} 	= "cook_islands";
$countryCode{"cl"}{"country"} 	= "chile";
$countryCode{"cm"}{"country"} 	= "cameroon";
$countryCode{"cn"}{"country"} 	= "china";
$countryCode{"co"}{"country"} 	= "colombia";
$countryCode{"cr"}{"country"} 	= "costa_rica";
$countryCode{"cs"}{"country"} 	= "serbia_montenegro";
$countryCode{"cu"}{"country"} 	= "cuba";
$countryCode{"cv"}{"country"} 	= "cape_verde";
$countryCode{"cy"}{"country"} 	= "cyprus";
$countryCode{"cz"}{"country"} 	= "czech_republic";
$countryCode{"de"}{"country"} 	= "germany";
$countryCode{"dj"}{"country"} 	= "djibouti";
$countryCode{"dk"}{"country"} 	= "denmark";
$countryCode{"dm"}{"country"} 	= "dominica";
$countryCode{"do"}{"country"} 	= "dominican_republic";
$countryCode{"dz"}{"country"} 	= "algeria";
$countryCode{"ec"}{"country"} 	= "ecuador";
$countryCode{"ee"}{"country"} 	= "estonia";
$countryCode{"eg"}{"country"} 	= "egypt";
$countryCode{"eh"}{"country"} 	= "western_sahara";
$countryCode{"er"}{"country"} 	= "eritrea";
$countryCode{"es"}{"country"} 	= "spain";
$countryCode{"et"}{"country"} 	= "ethiopia";
$countryCode{"fi"}{"country"} 	= "finland";
$countryCode{"fj"}{"country"} 	= "fiji";
$countryCode{"fk"}{"country"} 	= "falkland_islands";
$countryCode{"fm"}{"country"} 	= "micronesia";
$countryCode{"fo"}{"country"} 	= "faeroe_islands";
$countryCode{"fr"}{"country"} 	= "france";
$countryCode{"ga"}{"country"} 	= "gabon";
$countryCode{"gb"}{"country"} 	= "england";
$countryCode{"gd"}{"country"} 	= "grenada";
$countryCode{"ge"}{"country"} 	= "georgia_country";
$countryCode{"gf"}{"country"} 	= "french_guiana";
$countryCode{"gh"}{"country"} 	= "ghana";
$countryCode{"gl"}{"country"} 	= "greenland";
$countryCode{"gm"}{"country"} 	= "gambia";
$countryCode{"gn"}{"country"} 	= "guinea";
$countryCode{"gp"}{"country"} 	= "guadeloupe";
$countryCode{"gq"}{"country"} 	= "equatorial_guinea";
$countryCode{"gr"}{"country"} 	= "greece";
$countryCode{"gt"}{"country"} 	= "guatemala";
$countryCode{"gu"}{"country"} 	= "guam";
$countryCode{"gw"}{"country"} 	= "guinea_bissau";
$countryCode{"gy"}{"country"} 	= "guyana";
$countryCode{"hk"}{"country"} 	= "hong_kong";
$countryCode{"hn"}{"country"} 	= "honduras";
$countryCode{"hr"}{"country"} 	= "croatia";
$countryCode{"ht"}{"country"} 	= "haiti";
$countryCode{"hu"}{"country"} 	= "hungary";
$countryCode{"id"}{"country"} 	= "indonesia";
$countryCode{"ie"}{"country"} 	= "ireland";
$countryCode{"il"}{"country"} 	= "israel";
$countryCode{"in"}{"country"} 	= "india";
$countryCode{"iq"}{"country"} 	= "iraq";
$countryCode{"ir"}{"country"} 	= "iran";
$countryCode{"is"}{"country"} 	= "iceland";
$countryCode{"it"}{"country"} 	= "italy";
$countryCode{"jm"}{"country"} 	= "jamaica";
$countryCode{"jo"}{"country"} 	= "jordan";
$countryCode{"jp"}{"country"} 	= "japan";
$countryCode{"ke"}{"country"} 	= "kenya";
$countryCode{"kg"}{"country"} 	= "kyrgyzstan";
$countryCode{"kh"}{"country"} 	= "cambodia";
$countryCode{"ki"}{"country"} 	= "kiribati";
$countryCode{"km"}{"country"} 	= "comoros";
$countryCode{"kn"}{"country"} 	= "saint_kitts_and_nevis";
$countryCode{"kp"}{"country"} 	= "north_korea";
$countryCode{"kr"}{"country"} 	= "south_korea";
$countryCode{"kw"}{"country"} 	= "kuwait";
$countryCode{"ky"}{"country"} 	= "cayman_islands";
$countryCode{"kz"}{"country"} 	= "kazakhstan";
$countryCode{"la"}{"country"} 	= "laos";
$countryCode{"lb"}{"country"} 	= "lebanon";
$countryCode{"lc"}{"country"} 	= "saint_lucia";
$countryCode{"li"}{"country"} 	= "liechtenstein";
$countryCode{"lk"}{"country"} 	= "sri_lanka";
$countryCode{"lr"}{"country"} 	= "liberia";
$countryCode{"ls"}{"country"} 	= "lesotho";
$countryCode{"lt"}{"country"} 	= "lithuania";
$countryCode{"lu"}{"country"} 	= "luxembourg";
$countryCode{"lv"}{"country"} 	= "latvia";
$countryCode{"ly"}{"country"} 	= "libya";
$countryCode{"ma"}{"country"} 	= "morocco";
$countryCode{"mc"}{"country"} 	= "monaco";
$countryCode{"md"}{"country"} 	= "moldova";
$countryCode{"mg"}{"country"} 	= "madagascar";
$countryCode{"mh"}{"country"} 	= "marshall_islands";
$countryCode{"mk"}{"country"} 	= "macedonia";
$countryCode{"ml"}{"country"} 	= "mali";
$countryCode{"mm"}{"country"} 	= "myanmar";
$countryCode{"mn"}{"country"} 	= "mongolia";
$countryCode{"mo"}{"country"} 	= "macao";
$countryCode{"mp"}{"country"} 	= "northern_mariana_islands";
$countryCode{"mq"}{"country"} 	= "martinique";
$countryCode{"mr"}{"country"} 	= "mauritania";
$countryCode{"ms"}{"country"} 	= "montserrat";
$countryCode{"mt"}{"country"} 	= "malta";
$countryCode{"mu"}{"country"} 	= "mauritius";
$countryCode{"mv"}{"country"} 	= "maldives";
$countryCode{"mw"}{"country"} 	= "malawi";
$countryCode{"mx"}{"country"} 	= "mexico";
$countryCode{"my"}{"country"} 	= "malaysia";
$countryCode{"mz"}{"country"} 	= "mozambique";
$countryCode{"na"}{"country"} 	= "namibia";
$countryCode{"nc"}{"country"} 	= "new_caledonia";
$countryCode{"ne"}{"country"} 	= "niger";
$countryCode{"ng"}{"country"} 	= "nigeria";
$countryCode{"ni"}{"country"} 	= "nicaragua";
$countryCode{"nl"}{"country"} 	= "netherlands";
$countryCode{"no"}{"country"} 	= "norway";
$countryCode{"np"}{"country"} 	= "nepal";
$countryCode{"nr"}{"country"} 	= "nauru";
$countryCode{"nu"}{"country"} 	= "niue";
$countryCode{"nz"}{"country"} 	= "new_zealand";
$countryCode{"om"}{"country"} 	= "oman";
$countryCode{"pa"}{"country"} 	= "panama";
$countryCode{"pe"}{"country"} 	= "peru";
$countryCode{"pf"}{"country"} 	= "french_polynesia";
$countryCode{"pg"}{"country"} 	= "papua_new_guinea";
$countryCode{"ph"}{"country"} 	= "philippines";
$countryCode{"pk"}{"country"} 	= "pakistan";
$countryCode{"pl"}{"country"} 	= "poland";
$countryCode{"pm"}{"country"} 	= "saint_pierre_and_miquelon";
$countryCode{"pn"}{"country"} 	= "pitcairn";
$countryCode{"ps"}{"country"} 	= "occupied_palestinian_territory";
$countryCode{"pt"}{"country"} 	= "portugal";
$countryCode{"pw"}{"country"} 	= "palau";
$countryCode{"py"}{"country"} 	= "paraguay";
$countryCode{"qa"}{"country"} 	= "qatar";
$countryCode{"re"}{"country"} 	= "reunion";
$countryCode{"ro"}{"country"} 	= "romania";
$countryCode{"ru"}{"country"} 	= "russia";
$countryCode{"rw"}{"country"} 	= "rwanda";
$countryCode{"sa"}{"country"} 	= "saudi_arabia";
$countryCode{"sb"}{"country"} 	= "solomon_islands";
$countryCode{"sc"}{"country"} 	= "seychelles";
$countryCode{"sd"}{"country"} 	= "sudan";
$countryCode{"se"}{"country"} 	= "sweden";
$countryCode{"sg"}{"country"} 	= "singapore";
$countryCode{"sh"}{"country"} 	= "saint_helena";
$countryCode{"si"}{"country"} 	= "slovenia";
$countryCode{"sj"}{"country"} 	= "svalbard_and_jan_mayen";
$countryCode{"sk"}{"country"} 	= "slovakia";
$countryCode{"sl"}{"country"} 	= "sierra_leone";
$countryCode{"sm"}{"country"} 	= "san_marino";
$countryCode{"sn"}{"country"} 	= "senegal";
$countryCode{"so"}{"country"} 	= "somalia";
$countryCode{"sr"}{"country"} 	= "suriname";
$countryCode{"st"}{"country"} 	= "sao_tome_and_principe";
$countryCode{"sv"}{"country"} 	= "el_salvador";
$countryCode{"sy"}{"country"} 	= "syria";
$countryCode{"sz"}{"country"} 	= "swaziland";
$countryCode{"tc"}{"country"} 	= "turks_and_caicos_islands";
$countryCode{"td"}{"country"} 	= "chad";
$countryCode{"tg"}{"country"} 	= "togo";
$countryCode{"th"}{"country"} 	= "thailand";
$countryCode{"tj"}{"country"} 	= "tajikistan";
$countryCode{"tk"}{"country"} 	= "tokelau";
$countryCode{"tl"}{"country"} 	= "timor_leste";
$countryCode{"tm"}{"country"} 	= "turkmenistan";
$countryCode{"tn"}{"country"} 	= "tunisia";
$countryCode{"to"}{"country"} 	= "tonga";
$countryCode{"tr"}{"country"} 	= "turkey";
$countryCode{"tt"}{"country"} 	= "trinidad_and_tobago";
$countryCode{"tv"}{"country"} 	= "tuvalu";
$countryCode{"tw"}{"country"} 	= "taiwan";
$countryCode{"tz"}{"country"} 	= "tanzania";
$countryCode{"ua"}{"country"} 	= "ukraine";
$countryCode{"ug"}{"country"} 	= "uganda";
$countryCode{"um"}{"country"} 	= "united_states_minor_outlying_islands";
$countryCode{"uy"}{"country"} 	= "uruguay";
$countryCode{"uz"}{"country"} 	= "uzbekistan";
$countryCode{"vc"}{"country"} 	= "saint_vincent_and_the_grenadines";
$countryCode{"ve"}{"country"} 	= "venezuela";
$countryCode{"vg"}{"country"} 	= "british_virgin_islands";
$countryCode{"vi"}{"country"} 	= "united_states_virgin_islands";
$countryCode{"vn"}{"country"} 	= "vietnam";
$countryCode{"vu"}{"country"} 	= "vanuatu";
$countryCode{"wf"}{"country"} 	= "wallis_and_futuna_islands";
$countryCode{"ws"}{"country"} 	= "samoa";
$countryCode{"ye"}{"country"} 	= "yemen";
$countryCode{"yt"}{"country"} 	= "mayotte";
$countryCode{"za"}{"country"} 	= "south_africa";
$countryCode{"zm"}{"country"} 	= "zambia";
$countryCode{"zw"}{"country"} 	= "zimbabwe";
$countryCode{"us"}{"country"} 	= "usa";
$countryCode{"ca"}{"country"} 	= "canada";


# Hash that defines the iso alpha-2 codes that are not used in
# mc2 or map generation binaries.
my %notHandled = ();
$notHandled{"gi"} = 1; # gibraltar  (in spain)
$notHandled{"va"} = 1; # vatican    (in italy)
#$notHandled{"sm"} = 1; # san_marino (in italy)
$notHandled{"ax"} = 1; # aland_islands (in finland)
$notHandled{"pr"} = 1; # puerto_rico (in usa)
$notHandled{"nf"} = 1; # norfolk islands (in antarctica)


# Countries where you drive on the left side of the road
my @leftSideCountries;
push @leftSideCountries, "anguilla";
push @leftSideCountries, "antigua_and_barbuda";
push @leftSideCountries, "australia";
push @leftSideCountries, "bahamas";
push @leftSideCountries, "bangladesh";
push @leftSideCountries, "barbados";
push @leftSideCountries, "bermuda";
push @leftSideCountries, "bhutan";
push @leftSideCountries, "botswana";
push @leftSideCountries, "brunei_darussalam";
push @leftSideCountries, "cayman_islands";
push @leftSideCountries, "cook_islands";
push @leftSideCountries, "cyprus";
push @leftSideCountries, "dominica";
push @leftSideCountries, "timor_leste";
push @leftSideCountries, "falkland_islands";
push @leftSideCountries, "fiji";
push @leftSideCountries, "grenada";
push @leftSideCountries, "guyana";
push @leftSideCountries, "hong_kong";
push @leftSideCountries, "india";
push @leftSideCountries, "indonesia";
push @leftSideCountries, "ireland";
push @leftSideCountries, "jamaica";
push @leftSideCountries, "japan";
push @leftSideCountries, "kenya";
push @leftSideCountries, "kiribati";
push @leftSideCountries, "lesotho";
push @leftSideCountries, "macao";
push @leftSideCountries, "malawi";
push @leftSideCountries, "malaysia";
push @leftSideCountries, "maldives";
push @leftSideCountries, "malta";
push @leftSideCountries, "mauritius";
push @leftSideCountries, "montserrat ";
push @leftSideCountries, "mozambique";
push @leftSideCountries, "namibia";
push @leftSideCountries, "nauru";
push @leftSideCountries, "nepal";
push @leftSideCountries, "new_zealand";
push @leftSideCountries, "niue ";
push @leftSideCountries, "antarctica #(norfolk Island (Australia) )";
push @leftSideCountries, "pakistan";
push @leftSideCountries, "papua_new_guinea";
push @leftSideCountries, "pitcairn";
push @leftSideCountries, "saint_helena ";
push @leftSideCountries, "saint_kitts_and_nevis";
push @leftSideCountries, "saint_lucia";
push @leftSideCountries, "saint_vincent_and_the_grenadines";
push @leftSideCountries, "seychelles";
push @leftSideCountries, "singapore";
push @leftSideCountries, "solomon_islands";
push @leftSideCountries, "south_africa";
push @leftSideCountries, "sri_lanka";
push @leftSideCountries, "suriname";
push @leftSideCountries, "swaziland";
push @leftSideCountries, "tanzania";
push @leftSideCountries, "thailand";
push @leftSideCountries, "tokelau";
push @leftSideCountries, "tonga";
push @leftSideCountries, "trinidad_and_tobago";
push @leftSideCountries, "turks_and_caicos_islands";
push @leftSideCountries, "tuvalu";
push @leftSideCountries, "uganda";
push @leftSideCountries, "england";
push @leftSideCountries, "british_virgin_islands";
push @leftSideCountries, "united_states_virgin_islands";
push @leftSideCountries, "zambia";
push @leftSideCountries, "zimbabwe";
# Countries not defined
# Christmas Island (Australia) 
# Guernsey (Channel Islands)
# Isle of Man
# Jersey (Channel Islands)
# Cocos (Keeling) Islands (Australia) 


######### Info from alpha 2 codes ###################
sub getCountryFromAlpha2 {
   my $alpha2 = $_[0];
   
   if ( length($alpha2) != 2 ) {
      return undef;
   }
   $alpha2 = lc($alpha2);

   if ( defined( $countryCode{"$alpha2"}{"country"}) ) {
      return $countryCode{"$alpha2"}{"country"};
   }

   return undef;
}

sub getGmsNameFromAlpha2 {
   my $alpha2 = $_[0];
   
   if ( length($alpha2) != 2 ) {
      return undef;
   }
   $alpha2 = lc($alpha2);

   my $retVal = undef;
   if ( defined( $countryCode{"$alpha2"}{"gmsName"}) ) {
      $retVal = $countryCode{"$alpha2"}{"gmsName"};
   }
   elsif ( defined( $countryCode{"$alpha2"}{"country"}) ) {
      $retVal = $countryCode{"$alpha2"}{"country"};
   }
   
   return $retVal;
}

sub getAlpha2FromCountry {
   my $country = $_[0];
   foreach my $alpha2 ( keys(%countryCode) ) {
      if ( $countryCode{"$alpha2"}{"country"} eq $country ) {
         return $alpha2;
      }
   }
   return undef;
}


######################## Others ######################
sub getDrivingSide {
   my $in = $_[0];
   $in = lc($in);
   
   # get country if inparam is alpha2, check validity of country
   my $country = $in;
   if ( (length($in) == 2) and ($in ne "uk") ) {
      $country = getCountryFromAlpha2( $in );
   } else {
      if ( ! defined( getAlpha2FromCountry($in) ) ) {
         return undef;
      }
   }
   if ( ! defined($country) ) {
      return undef;
   }

   # get driving side
   foreach my $c ( @leftSideCountries ) {
      if ( $c eq $country ) {
         return "left";
      }
   }
   return "right";
}

my @countryOrder;
push @countryOrder, "sweden";
push @countryOrder, "norway";
push @countryOrder, "denmark";
push @countryOrder, "finland";
push @countryOrder, "germany";
push @countryOrder, "england";
push @countryOrder, "austria";
push @countryOrder, "switzerland";
push @countryOrder, "liechtenstein";
push @countryOrder, "belgium";
push @countryOrder, "netherlands";
push @countryOrder, "luxembourg";
push @countryOrder, "france";
push @countryOrder, "andorra";
push @countryOrder, "spain";
push @countryOrder, "monaco";
push @countryOrder, "italy";
push @countryOrder, "portugal";
push @countryOrder, "ireland";
push @countryOrder, "hungary";
push @countryOrder, "czech_republic";
push @countryOrder, "poland";
push @countryOrder, "greece";
push @countryOrder, "slovakia";
push @countryOrder, "russia";
push @countryOrder, "turkey";
push @countryOrder, "slovenia";
push @countryOrder, "croatia";
push @countryOrder, "usa";
push @countryOrder, "canada";
push @countryOrder, "uae";
push @countryOrder, "australia";
push @countryOrder, "afghanistan";
push @countryOrder, "antigua_and_barbuda";
push @countryOrder, "anguilla";
push @countryOrder, "albania";
push @countryOrder, "armenia";
push @countryOrder, "netherlands_antilles";
push @countryOrder, "angola";
push @countryOrder, "antarctica";
push @countryOrder, "argentina";
push @countryOrder, "american_samoa";
push @countryOrder, "aruba";
push @countryOrder, "azerbaijan";
push @countryOrder, "bosnia";
push @countryOrder, "barbados";
push @countryOrder, "bangladesh";
push @countryOrder, "burkina_faso";
push @countryOrder, "bulgaria";
push @countryOrder, "bahrain";
push @countryOrder, "burundi";
push @countryOrder, "benin";
push @countryOrder, "bermuda";
push @countryOrder, "brunei_darussalam";
push @countryOrder, "bolivia";
push @countryOrder, "brazil";
push @countryOrder, "bahamas";
push @countryOrder, "bhutan";
push @countryOrder, "botswana";
push @countryOrder, "belarus";
push @countryOrder, "belize";
push @countryOrder, "dr_congo";
push @countryOrder, "central_african_republic";
push @countryOrder, "congo";
push @countryOrder, "ivory_coast";
push @countryOrder, "cook_islands";
push @countryOrder, "chile";
push @countryOrder, "cameroon";
push @countryOrder, "china";
push @countryOrder, "colombia";
push @countryOrder, "costa_rica";
push @countryOrder, "serbia_montenegro";
push @countryOrder, "cuba";
push @countryOrder, "cape_verde";
push @countryOrder, "cyprus";
push @countryOrder, "djibouti";
push @countryOrder, "dominica";
push @countryOrder, "dominican_republic";
push @countryOrder, "algeria";
push @countryOrder, "ecuador";
push @countryOrder, "estonia";
push @countryOrder, "egypt";
push @countryOrder, "western_sahara";
push @countryOrder, "eritrea";
push @countryOrder, "ethiopia";
push @countryOrder, "fiji";
push @countryOrder, "falkland_islands";
push @countryOrder, "micronesia";
push @countryOrder, "faeroe_islands";
push @countryOrder, "gabon";
push @countryOrder, "grenada";
push @countryOrder, "georgia_country";
push @countryOrder, "french_guiana";
push @countryOrder, "ghana";
push @countryOrder, "greenland";
push @countryOrder, "gambia";
push @countryOrder, "guinea";
push @countryOrder, "guadeloupe";
push @countryOrder, "equatorial_guinea";
push @countryOrder, "guatemala";
push @countryOrder, "guam";
push @countryOrder, "guinea_bissau";
push @countryOrder, "guyana";
push @countryOrder, "hong_kong";
push @countryOrder, "honduras";
push @countryOrder, "haiti";
push @countryOrder, "indonesia";
push @countryOrder, "israel";
push @countryOrder, "india";
push @countryOrder, "iraq";
push @countryOrder, "iran";
push @countryOrder, "iceland";
push @countryOrder, "jamaica";
push @countryOrder, "jordan";
push @countryOrder, "japan";
push @countryOrder, "kenya";
push @countryOrder, "kyrgyzstan";
push @countryOrder, "cambodia";
push @countryOrder, "kiribati";
push @countryOrder, "comoros";
push @countryOrder, "saint_kitts_and_nevis";
push @countryOrder, "north_korea";
push @countryOrder, "south_korea";
push @countryOrder, "kuwait";
push @countryOrder, "cayman_islands";
push @countryOrder, "kazakhstan";
push @countryOrder, "laos";
push @countryOrder, "lebanon";
push @countryOrder, "saint_lucia";
push @countryOrder, "sri_lanka";
push @countryOrder, "liberia";
push @countryOrder, "lesotho";
push @countryOrder, "lithuania";
push @countryOrder, "latvia";
push @countryOrder, "libya";
push @countryOrder, "morocco";
push @countryOrder, "moldova";
push @countryOrder, "madagascar";
push @countryOrder, "marshall_islands";
push @countryOrder, "macedonia";
push @countryOrder, "mali";
push @countryOrder, "myanmar";
push @countryOrder, "mongolia";
push @countryOrder, "macao";
push @countryOrder, "northern_mariana_islands";
push @countryOrder, "martinique";
push @countryOrder, "mauritania";
push @countryOrder, "montserrat";
push @countryOrder, "malta";
push @countryOrder, "mauritius";
push @countryOrder, "maldives";
push @countryOrder, "malawi";
push @countryOrder, "mexico";
push @countryOrder, "malaysia";
push @countryOrder, "mozambique";
push @countryOrder, "namibia";
push @countryOrder, "new_caledonia";
push @countryOrder, "niger";
push @countryOrder, "nigeria";
push @countryOrder, "nicaragua";
push @countryOrder, "nepal";
push @countryOrder, "nauru";
push @countryOrder, "niue";
push @countryOrder, "new_zealand";
push @countryOrder, "oman";
push @countryOrder, "panama";
push @countryOrder, "peru";
push @countryOrder, "french_polynesia";
push @countryOrder, "papua_new_guinea";
push @countryOrder, "philippines";
push @countryOrder, "pakistan";
push @countryOrder, "saint_pierre_and_miquelon";
push @countryOrder, "pitcairn";
push @countryOrder, "occupied_palestinian_territory";
push @countryOrder, "palau";
push @countryOrder, "paraguay";
push @countryOrder, "qatar";
push @countryOrder, "reunion";
push @countryOrder, "romania";
push @countryOrder, "rwanda";
push @countryOrder, "saudi_arabia";
push @countryOrder, "solomon_islands";
push @countryOrder, "seychelles";
push @countryOrder, "sudan";
push @countryOrder, "singapore";
push @countryOrder, "saint_helena";
push @countryOrder, "svalbard_and_jan_mayen";
push @countryOrder, "sierra_leone";
push @countryOrder, "senegal";
push @countryOrder, "somalia";
push @countryOrder, "suriname";
push @countryOrder, "sao_tome_and_principe";
push @countryOrder, "el_salvador";
push @countryOrder, "syria";
push @countryOrder, "swaziland";
push @countryOrder, "turks_and_caicos_islands";
push @countryOrder, "chad";
push @countryOrder, "togo";
push @countryOrder, "thailand";
push @countryOrder, "tajikistan";
push @countryOrder, "tokelau";
push @countryOrder, "timor_leste";
push @countryOrder, "turkmenistan";
push @countryOrder, "tunisia";
push @countryOrder, "tonga";
push @countryOrder, "trinidad_and_tobago";
push @countryOrder, "tuvalu";
push @countryOrder, "taiwan";
push @countryOrder, "tanzania";
push @countryOrder, "ukraine";
push @countryOrder, "uganda";
push @countryOrder, "united_states_minor_outlying_islands";
push @countryOrder, "uruguay";
push @countryOrder, "uzbekistan";
push @countryOrder, "saint_vincent_and_the_grenadines";
push @countryOrder, "venezuela";
push @countryOrder, "british_virgin_islands";
push @countryOrder, "united_states_virgin_islands";
push @countryOrder, "vietnam";
push @countryOrder, "vanuatu";
push @countryOrder, "wallis_and_futuna_islands";
push @countryOrder, "samoa";
push @countryOrder, "yemen";
push @countryOrder, "mayotte";
push @countryOrder, "south_africa";
push @countryOrder, "zambia";
push @countryOrder, "zimbabwe";
push @countryOrder, "san_marino";

sub getCountryOrder {
    my %countriesToSort;

    # Looping in-parameters
    foreach my $countryDir (@_){
        my $country=$countryDir;
        $country=~ s/\/before_merge$//;
        $country=~ s/^.*\///;
        $countriesToSort{$country} = $countryDir;
        #print "PCC: $country: $countryDir\n";
    }

    my $result = "";
    foreach my $sortedCountry ( @countryOrder ) {
        if ( defined($countriesToSort{$sortedCountry} ) ) {
            $result = "$result $countriesToSort{$sortedCountry}\n";
        }
    }
    return $result;
}

1;

=head1 NAME 

PerlCountryCodes

Package with functions working on country codes.

=head1 USE

Include into your perl file with the combination of:
   use lib "${BASEGENFILESPATH}/script/perllib";
   use PerlCountryCodes;
pointing to the directory where the perl modules are stored

=head1 FUNCTIONS

getCountryFromAlpha2( $alpha2 )

getAlpha2FromCountry( $country )

getGmsNameFromAlpha2( $alpha2 )

getDrivingSide( $alpha2 or $country )


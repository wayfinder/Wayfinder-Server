#!/usr/bin/perl -w

use File::Basename;

# The test to run for NavigatorServer
# Add tests to @nsTestSet

sub addNS {
    my $test = shift(@_);
    my $testName = "ns_" . basename( $test, ".txt" );
    push @nsTestSet, [ $testName, "./NS/tools/anstest $test $ngpmakerprog $nsHost $nsPort" ];
}

addNS( "NS/combined_search_sweden_wayf.txt" );
addNS( "NS/combined_search_sweden_airport.txt" );
#Eniro tests dissabled as 1. Eniro changes output alot (and we don't filter) 2. Eniro Finland is not stable.
#addNS( "NS/combined_search_sweden_lund_bi_round_1.txt" );
#addNS( "NS/combined_search_finland_helsinki_bi_round_1.txt" );
#addNS( "NS/combined_search_norway_oslo_bi_round_1.txt" );
#addNS( "NS/combined_search_denmark_kopenhavn_bi_round_1.txt" );

addNS( "NS/combined_search_uk_london_palace_park.txt" );
addNS( "NS/combined_search_uk_london_doctor.txt" );
addNS( "NS/combined_search_uk_london_park_road.txt" );
addNS( "NS/combined_search_position_london.txt" );
addNS( "NS/combined_search_sweden_storgatan.txt");
addNS( "NS/combined_search_sweden_stockholm_kungsgatan_5.txt");
addNS( "NS/combined_search_sweden_liseberg.txt");
addNS( "NS/combined_search_sweden_sodra_sandby_ampersand_bar.txt");
addNS( "NS/combined_search_sweden_malmo_305.txt");
addNS( "NS/combined_search_sweden_malmo_sturup.txt");
addNS( "NS/combined_search_denmark_tivoli.txt");
addNS( "NS/combined_search_germany_berlin.txt");
addNS( "NS/combined_search_italy_font_di_tre.txt");
addNS( "NS/combined_search_italy_roma_font_di_tre.txt");
addNS( "NS/combined_search_italy_roma_fontana_di_trevi.txt");
addNS( "NS/local_category_tree_req_england.txt" );
addNS( "NS/local_category_tree_req_germany.txt" );
addNS( "NS/local_category_tree_req_sweden.txt" );
addNS( "NS/local_category_tree_req_france.txt" );
addNS( "NS/one_search.txt" );
addNS( "NS/one_search_lund.txt" );
addNS( "NS/one_search_address.txt" );
addNS( "NS/one_search_what_where.txt" );
addNS( "NS/poi_details.txt" );
addNS( "NS/poi_details_v2.txt" );
addNS( "NS/prompt_dl_new_version.txt" );
addNS( "NS/prompt_dl_new_version_force.txt" );
addNS( "NS/prompt_dl_new_version_no_new_version.txt" );

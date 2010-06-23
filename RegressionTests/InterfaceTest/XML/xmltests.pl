#!/usr/bin/perl -w

# The test to run for XMLServer
# Add tests to @xmlTestSet

if( 0 ) {
push @xmlTestSet, [ "xs_top_region_request", "./XML/top_region_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_top_region_request_french", "./XML/top_region_request_french.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_top_region_request_german", "./XML/top_region_request_german.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_top_region_request_norwegian", "./XML/top_region_request_norwegian.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_top_region_request_swedish", "./XML/top_region_request_swedish.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_user_login_case_insensitive_test_uppercase", "./XML/user_login_case_insensitive_test_uppercase.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_user_login_case_insensitive_test_glitchy_shift", "./XML/user_login_case_insensitive_test_glitchy_shift.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_user_login_invalid_login", "./XML/user_login_invalid_login.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_user_login_invalid_password", "./XML/user_login_invalid_password.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_user_login_white_space", "./XML/user_login_white_space.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_search_request_area", "./XML/search_request_area.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort" ];
push @xmlTestSet, [ "xs_search_request_item", "./XML/search_request_item.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\"" ];
push @xmlTestSet, [ "xs_route_request", "./XML/route_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" xs_route_request $interactive" ]; # Sets route_id to 0_0
# From MAS (hospital), Malmö to Wayfinder, Lund
push @xmlTestSet, [ "xs_route_request_se_malmo_se_lund", "./XML/route_request_orig_dest.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" xs_route_request_se_malmo_se_lund $interactive 663206860 155116323 664748449 157366099" ]; # Sets route_id to 0_0
# Roundabout test 1. Linköping, 2. Snogeröd -> Höör, 3. Magic Rb in Swindon
push @xmlTestSet, [ "xs_route_request_roundabout1", "./XML/route_request_orig_dest.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" xs_route_request_roundabout1 $interactive 696798876 185844746 696807727 185845183" ];
push @xmlTestSet, [ "xs_route_request_roundabout2", "./XML/route_request_orig_dest.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" xs_route_request_roundabout2 $interactive 666099001 160962642 667606762 161871407" ];
push @xmlTestSet, [ "xs_route_request_roundabout3", "./XML/route_request_orig_dest.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" xs_route_request_roundabout2 $interactive 615167502 -21125203 615170943 -21147922" ];
# Category search test
push @xmlTestSet, [ "xs_category_tree_request_eng_vicinity", "./XML/category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_category_tree_request_eng_vicinity $interactive noCrc eng vicinity" ];
push @xmlTestSet, [ "xs_category_tree_request_swe_vicinity", "./XML/category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_category_tree_request_swe_vicinity $interactive noCrc swe vicinity" ];
push @xmlTestSet, [ "xs_category_tree_request_eng_eventfinder", "./XML/category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_category_tree_request_eng_eventfinder $interactive noCrc eng eventfinder" ];
push @xmlTestSet, [ "xs_category_tree_request_eng_crcok", "./XML/category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_category_tree_request_eng_crcok $interactive ff4d2113 eng vicinity" ];
push @xmlTestSet, [ "xs_category_tree_request_eng_emptytype", "./XML/category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_category_tree_request_eng_emptytype $interactive coCrc eng emptytype"];
push @xmlTestSet, [ "xs_category_tree_request_eng_wrongtype", "./XML/category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_category_tree_request_eng_wrongtype $interactive noCrc eng wrongtype" ];
push @xmlTestSet, [ "xs_local_category_tree_request_eng", "./XML/local_category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_local_category_tree_request_eng $interactive noCrc english 665200001 -75046560" ];
push @xmlTestSet, [ "xs_local_category_tree_request_ger", "./XML/local_category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_local_category_tree_request_eng $interactive noCrc english 623613523 96083475" ];
push @xmlTestSet, [ "xs_local_category_tree_request_swe", "./XML/local_category_tree_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_local_category_tree_request_eng $interactive noCrc english 673582845 162222849" ];
}
#One search tests
@oneSearchTests = (
		   'category',
		   'mcd_markaryd',
		   'eniro_andersson',
		   'london_starbucks',
		   'hamburg',
		   'spain',
		   'spain_round1',
		   'portugal_lisboa',
		   'london_round0',
		   'address',
		   'what_where'
		   );

foreach $t (@oneSearchTests) {
    push @xmlTestSet, [ "xs_one_search_" . $t, "./XML/one_search_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $t" ];
    push @xmlTestSet, [ "xs_poi_detail_" . $t, "./XML/poi_detail_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $t" ];
}

push @xmlTestSet, [ "xs_one_search_invalid_input", "./XML/one_search_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort invalid_input" ];

# Server info tests
push @xmlTestSet, [ "xs_prompt_new_client_no_force", "./XML/server_info_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_prompt_new_client_no_force NoUse-UnitTest1-vf 4.63.0:4.9.0:0 " ];
push @xmlTestSet, [ "xs_prompt_new_client_no_upgrade", "./XML/server_info_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_prompt_new_client_no_upgrade NoUse-UnitTest1-vf 4.64.0:4.9.0:0 " ];
push @xmlTestSet, [ "xs_prompt_new_client_force", "./XML/server_info_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_prompt_new_client_force NoUse-UnitTest2-vf 4.63.0:4.9.0:0 " ];
push @xmlTestSet, [ "xs_prompt_new_client_unknown_client", "./XML/server_info_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir xs_prompt_new_client_unknown_client UnitTest1-Unknown 4.63.0:4.9.0:0 " ];

# Routes across Europe:
my %cities = (
   'de_berlin'     => [ '626629610', '160005465' ],
   'dk_copenhagen' => [ '664244065', '149943460' ],
   'es_madrid'     => [ '482190505', '-44181491' ],
   'it_rome'       => [ '499832373', '148919929' ],
   'fr_paris'      => [ '582882699', '28048368'  ],
   'hu_budapest'   => [ '566678027', '227165098' ],
);

foreach my $city_orig (keys %cities) {
   foreach my $city_dest (keys %cities) {
      next if ($city_dest eq $city_orig);
      my ($olat, $olon) = @{$cities{$city_orig}};
      my ($dlat, $dlon) = @{$cities{$city_dest}};
      my $test_name = 'xs_route_request_' . $city_orig . '_' . $city_dest;
      push @xmlTestSet, [ 
          $test_name,
          "./XML/route_request_orig_dest.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" $test_name $interactive $olat $olon $dlat $dlon"
      ];
   }
}

# Positions for expand_request:
my %expand_positions = (
   'sofiaparken'      => [ 'N 554305', 'E 131149' ],
   'outsidemap'       => [ 'N 000000', 'E 000000' ],
   'piazzaditrevi'    => [ 'N 415403', 'E 122859' ],
   'middleofnowhere'  => [ 'N 634903', 'E 191250' ],
   'newyork'          => [ 'N 404603', 'W 735749' ],
   'kyrgyzstan'       => [ 'N 404603', 'E 735749' ],
);

foreach my $expand_position (keys %expand_positions) {
    my ($lat, $lon) = @{$expand_positions{$expand_position}};
    my $test_name = 'expand_position_request_' . $expand_position;
    push @xmlTestSet, [
        $test_name,
	"./XML/expand_position_request.sh $xmlLogin $xmlPasswd $xmlHost $xmlPort $resultDir \"$javaProg\" $test_name $interactive \"$lat\" \"$lon\""
    ];
}

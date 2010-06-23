#!/bin/zsh

xml_login=$1
xml_passwd=$2
xml_host=$3
xml_port=$4
result_dir=$5
# not used
java_prog=$6
test_id=$7
get_image_uris=$8
orig_mc2_lat=$9
orig_mc2_lon=$10
dest_mc2_lat=$11
dest_mc2_lon=$12
reply_file="${test_id}_reply_$$.xml"
request_file="${test_id}_request_$$.xml"

./tools/replace.pl "%%%AUTH_USER%%%" $xml_login "%%%AUTH_PASSWD%%%" $xml_passwd "%%%ORIG_MC2_LAT%%%" "$orig_mc2_lat" "%%%ORIG_MC2_LON%%%" "$orig_mc2_lon" "%%%DEST_MC2_LAT%%%" "$dest_mc2_lat" "%%%DEST_MC2_LON%%%" "$dest_mc2_lon" < XML/route_request_orig_dest.xml > $request_file
./tools/httpfile $xml_host $xml_port $request_file | ./tools//striphttpheader.pl > $reply_file
if [[ $get_image_uris = "1" ]] {
    ./tools/extracturi.pl < $reply_file > ${test_id}_images
}
# Anonymize route id
./tools/replace.pl 'route_id=".*"' 'route_id="0_0"' '&amp;' '&' "&r=.*&" "&r=0_0&" < $reply_file
rm $request_file
rm $reply_file

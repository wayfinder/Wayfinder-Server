#!/bin/zsh

xml_login=$1
xml_passwd=$2
xml_host=$3
xml_port=$4
result_dir=$5
test_id=$6
get_image_uris=$7
xml_attr_crc=$8
xml_attr_language=$9
lat=$10
lon=$11
reply_file="${test_id}_reply_$$.xml"
request_file="${test_id}_request_$$.xml"

# Replace attribute values in request
./tools/replace.pl "%%%AUTH_USER%%%" $xml_login "%%%AUTH_PASSWD%%%" $xml_passwd "%%%ATTR_CRC%%%" "$xml_attr_crc" "%%%ATTR_LANGUAGE%%%" "$xml_attr_language" %%%LAT%%% $lat %%%LON%%% $lon< XML/local_category_tree.xml > $request_file
#
./tools/httpfile $xml_host $xml_port $request_file | ./tools//striphttpheader.pl > $reply_file

if [[ $get_image_uris = "1" ]] {
    ./tools/extracturi.pl < $reply_file > ${test_id}_images
}
# Anonymize &amp (keept from route request to get files at correct location)
./tools/replace.pl 'dummyString' 'dummyString2' '&amp;' '&' "&r=.*&" "&r=0_0&" < $reply_file
rm $request_file
rm $reply_file

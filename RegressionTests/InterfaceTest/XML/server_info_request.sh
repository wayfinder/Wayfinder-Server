#!/bin/zsh

xml_login=$1
xml_passwd=$2
xml_host=$3
xml_port=$4
result_dir=$5
test_id=$6
client_type=$7
client_version=$8
reply_file="${test_id}_reply_$$.xml"
request_file="${test_id}_request_$$.xml"

# Replace attribute values in request
./tools/replace.pl "%%%AUTH_USER%%%" $xml_login "%%%AUTH_PASSWD%%%" $xml_passwd "%%%CLIENT_TYPE%%%" $client_type "%%%CLIENT_VERSION%%%" $client_version < XML/server_info_request.xml > $request_file
#
./tools/httpfile $xml_host $xml_port $request_file | ./tools//striphttpheader.pl > $reply_file

# Anonymize &amp (keept from route request to get files at correct location)
./tools/replace.pl 'dummyString' 'dummyString2' '&amp;' '&' "&r=.*&" "&r=0_0&" < $reply_file
rm $request_file
rm $reply_file

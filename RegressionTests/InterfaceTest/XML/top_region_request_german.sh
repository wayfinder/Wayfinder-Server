#!/bin/zsh

./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%LANGUAGE%%%" "german" < XML/top_region_request.xml > top_region_request_$$.xml
./tools//httpfile $3 $4 top_region_request_$$.xml > top_region_reply_raw_$$.xml
./tools/striphttpheader.pl < top_region_reply_raw_$$.xml
rm top_region_request_$$.xml
rm top_region_reply_raw_$$.xml

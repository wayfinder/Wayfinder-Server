#!/bin/zsh
./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%POS_LAT%%%" $9 "%%%POS_LON%%%" $10 < XML/expand_position_request.xml > expand_position_request_$$.xml
./tools//httpfile $3 $4 expand_position_request_$$.xml > expand_position_reply_raw_$$.xml
./tools/striphttpheader.pl < expand_position_reply_raw_$$.xml
rm expand_position_request_$$.xml
rm expand_position_reply_raw_$$.xml

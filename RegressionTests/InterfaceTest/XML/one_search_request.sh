#!/bin/zsh

test_name=$5
request_file="XML/one_search_${test_name}.xml"
request_tmp="one_search_request_${test_name}_$$.xml"
reply_file="one_search_replay_${test_name}_raw_$$.xml"

./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%LANGUAGE%%%" "english" < $request_file > $request_tmp
./tools//httpfile $3 $4 $request_tmp > $reply_file
./tools/striphttpheader.pl < $reply_file
rm $request_tmp
rm $reply_file

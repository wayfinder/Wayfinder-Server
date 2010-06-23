#!/bin/zsh

test_name=$5
request_file="XML/xs_poi_detail_request.xml"
request_tmp="poi_detail_request_${test_name}_$$.xml"
reply_file="poi_detail_replay_${test_name}_raw_$$.xml"
one_search_reply="results/xs_one_search_${test_name}_result"


if [ $6 ]; then
    itemid="<itemid>$6</itemid>"
 else 
    itemid=$(grep --max-count=1 "<itemid>" $one_search_reply)    
fi


./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%LANGUAGE%%%" "english" "%%%ITEMID%%%" $itemid < $request_file > $request_tmp
./tools//httpfile $3 $4 $request_tmp > $reply_file
./tools/striphttpheader.pl < $reply_file
rm $request_tmp
rm $reply_file

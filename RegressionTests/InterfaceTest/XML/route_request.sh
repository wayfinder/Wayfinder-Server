#!/bin/zsh

SearchItem=`$6 -classpath ./tools/ServerCommunicator/ServerCommunicator.jar:./tools firstSearchItem $5/xs_search_request_item_result`
./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%SEARCH_ITEM%%%" "$SearchItem" < XML/route_request.xml > route_request_$$.xml
./tools/httpfile $3 $4 route_request_$$.xml > route_request_raw_$$.xml
./tools//striphttpheader.pl < route_request_raw_$$.xml > route_reply_raw_$$.xml
if [[ $8 = "1" ]] {
    # If interactive get image uris
    ./tools/extracturi.pl < route_reply_raw_$$.xml > ${7}_images
}
# Anonymize route id
./tools/replace.pl 'route_id=".*"' 'route_id="0_0"' '&amp;' '&' "&r=.*&" "&r=0_0&" < route_reply_raw_$$.xml
rm route_request_$$.xml
rm route_request_raw_$$.xml
rm route_reply_raw_$$.xml

#!/bin/zsh

SearchArea=`$6 -classpath ./tools/ServerCommunicator/ServerCommunicator.jar:./tools firstSearchArea $5/xs_search_request_area_result`
./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%SEARCH_AREA%%%" "$SearchArea" < XML/search_request_item.xml > search_request_item_$$.xml
./tools/httpfile $3 $4 search_request_item_$$.xml > search_request_item_raw_$$.xml
./tools/striphttpheader.pl < search_request_item_raw_$$.xml 
rm search_request_item_$$.xml
rm search_request_item_raw_$$.xml

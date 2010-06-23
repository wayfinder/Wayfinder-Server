#!/bin/zsh

./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 < XML/search_request_area.xml > search_request_area_$$.xml
./tools/httpfile $3 $4 search_request_area_$$.xml > search_request_area_raw_$$.xml
./tools/striphttpheader.pl < search_request_area_raw_$$.xml 
rm search_request_area_$$.xml
rm search_request_area_raw_$$.xml

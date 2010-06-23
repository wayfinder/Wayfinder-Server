#!/bin/zsh

fileName=user_login
./tools/replace.pl "%%%AUTH_USER%%%" $1 "%%%AUTH_PASSWD%%%" $2 "%%%LOGIN%%%" "DEMO" "%%%PASS%%%" "faillogin" < XML/${fileName}.xml > ${fileName}_$$.xml
./tools//httpfile $3 $4 ${fileName}_$$.xml > ${fileName}_raw_$$.xml
./tools/striphttpheader.pl < ${fileName}_raw_$$.xml
rm ${fileName}_$$.xml
rm ${fileName}_raw_$$.xml

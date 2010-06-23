#!/bin/zsh

# Anonymize route id
mv $1 $1.tmp
../tools/replace.pl 'route_id=".*"' 'route_id="0_0"' "&r=.*&" "&r=0_0&" < $1.tmp > $1

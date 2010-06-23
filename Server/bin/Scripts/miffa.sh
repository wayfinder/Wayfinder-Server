#!/bin/zsh

# Converts files from svg to mif for use in Series 60 third

MIFCONV=${HOME}/bin/mifconv

while [[ $1 != "" ]]
do
  ${MIFCONV} `basename $1 .svg` $1
  shift
done

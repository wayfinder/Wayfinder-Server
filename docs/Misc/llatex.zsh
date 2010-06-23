#!/bin/zsh

if [ $# = 0 ]; then
  echo "This script runs latex, and latex again if references changed."
  echo "Important output from latex is displayed."
  echo "Index and bibliography is also generated."
  echo ""
  echo "Usage $0 latex-filename-with-extension [more params]"
  exit 1
fi

# The filename
fn=`echo ${1} | cut -d"." -f1`

echo "params: "$@"."
# Run latex, and repeat if neccessary.
echo "Latexing..."
latex -interaction=nonstopmode -o "$@" >&tmplatexoutput
#res=$?
#if ( [ $res != 0 ] ) then
#  echo less ${fn}.log
#  less ${fn}.log |  egrep "\.tex|^\!|^l\." | egrep -B 1 -A 1 "^\!"
#  egrep -A 2 -i '^!'
#  exit 1
#fi

texfile=`basename $2 .tex`
# create index files for index
makeindex ${texfile}

# run bibtex
bibtex ${texfile}

# run latex an extra time to get references and indexes right
latex -interaction=nonstopmode -o "$@" >&/dev/null

#less ${fn}.log | egrep "LaTeX Warning.*changed.*Rerun to get"
latex -interaction=nonstopmode -o "$@" >&tmplatexoutput
res=$?
if ( [ $res != 0 ] ) then
  # first print some errors
  echo "Errors from latex:"
  less ${fn}.log | egrep -A ${LINES} "^\!" | fold --width=$COLUMNS | head -n $[ ${LINES} - 3 ]
  echo "<press ctrl-c>"
#  cat # should not be present when running as a cron job
  exit 1
fi

# Print important info from Latex
# (Overfull .box is not considered important if less than 10 points)
less ${fn}.log | egrep -i "LaTeX Warning|Overfull|Underfull|warning|error|^\!" | egrep -v "Overfull.*\([:0-9:]\.|\.eps" |& tee tmp
# cat tmp
echo
echo "Warning frequency table for " `basename $2 .tex`
less tmp | sed -e 's/[:0-9:]//g' | sort | uniq -c | cut -b 1-$COLUMNS | sort -gr
echo
echo "Errors from latex:"
less ${fn}.log | egrep -A ${LINES} "^\!" | fold --width=$COLUMNS | head -n $[ ${LINES} - 3 ]
rm tmp
echo "<end of error list>"


#!/bin/zsh
# Runs servertest.pl and checks status

tmpres="/tmp/`basename ${0}`_$$"

pushd `dirname $0` 2>& /dev/null
./servertest.pl --noninteractive &> $tmpres
res=$?
popd 2>& /dev/null
if ( [ $res != 0 ] ) then
   cat $tmpres | egrep ".*: *(OK|FAILED)$"
   echo ""; echo "Failed test results with diffs" ; echo ""
   cat $tmpres | egrep -v ".*: *OK$"
   rm -f $tmpres
   exit $res
else
   rm -f $tmpres
   exit 0
fi

#!/bin/sh -x
#
# This script should be used when filtering maps. Typically it is called
# from a makemaps or makemaps.mcgen script.
# It both takes backups and filters the maps.


# BASEGENFILESPATH 
# genfiles is the base directory where all setting files for map generation
# is stored
# Update it to point to the full path of where you create the BASEGENFILESPATH
#BASEGENFILESPATH="fullpath/genfiles";
BASEGENFILESPATH="."


source ${BASEGENFILESPATH}/script/mfunctions.sh

STOREPATH="$1";  # The directory to store backups in (sub dir is created).
                 #
GENPATH="$2";    # The directory where to find the maps to filter and the
                 # GenereateMapServer to filter with.
                 #
COMPUTER="$3";   # The computer to use for filtering.
                 #
                 # If this is only one computer, like "hostname" two
                 # processors are used from this computer.
                 #
                 # If this is more than one computer, 
                 # like "hostname1 hostname2" as many processors as the 
                 # computer name will be used.
                 #
NOFILTBKP="$4";  # If this option is not the empty string, no backups are
                 # taken.

LOGPATH="`pwd`/logpath"



   # Backup before_filter
if [ ! "${NOFILTBKP}" ]; then
    # Store the unfiltered maps.
    echo ""
    echo `date +"%Y%m%d %T"`" Backup of unfiltered maps to ${STOREPATH}/before_filter.";
    distBackup ${STOREPATH}/before_filter "*.mcm index.db" "$COMPUTER"
    chk_res
fi


  # Filter the maps.
if [ "`echo \"${COMPUTER}\" | sed -e 's/^[^\ ]*\ *//'`" ]; then
  # multiple computers
  FILTER_COMPUTERS="${COMPUTER}";
else 
  # single computer
  FILTER_COMPUTERS="${COMPUTER} ${COMPUTER}"
fi
echo ""
echo `date +"%Y%m%d %T"`" Filtering maps on ${FILTER_COMPUTERS}.";
mkdir -p ${LOGPATH};
chmod g+rw ${LOGPATH}

for map in `ls 09*.mcm 08*.mcm 00*.mcm`; do
echo "cd ${GENPATH}; ./GenerateMapServer --filterCoordinates ${map} > ${LOGPATH}/${map}.filt.log 2>&1"
done > filt_to_be_distributed

./distribute -e ${FILTER_COMPUTERS} < filt_to_be_distributed;
chk_res


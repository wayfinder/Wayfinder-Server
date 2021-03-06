#!/bin/sh

#PRINTDEBUG="yes"

# Define directories and file names
# Assuming this script is located in genfiles/countries/*/script/
BASEGENFILESPATH=`echo $(dirname $(readlink -f ${BASH_SOURCE[0]})) | sed -e "s/\/countries\/.*\/script$//"`

RELEASEDIR="${BASEGENFILESPATH}/SupplierMapData/TeleAtlas/2010_06_eu_shp";
CC_CODE="FR";
TACC_CODE="fraf2075_";
MIDMIFDIR="${RELEASEDIR}/midmif/$CC_CODE";

# normal items
ADMINALTNAMES="${MIDMIFDIR}/${TACC_CODE}an.txt";
MUNICIPALMID="${MIDMIFDIR}/${TACC_CODE}a8.mid";
MUNICIPALMIF="${MIDMIFDIR}/${TACC_CODE}a8.mif";
BUAMID="${MIDMIFDIR}/${TACC_CODE}bu.mid";
BUAMIF="${MIDMIFDIR}/${TACC_CODE}bu.mif";
CITYPARTMID="${MIDMIFDIR}/${TACC_CODE}a9.mid";
CITYPARTMIF="${MIDMIFDIR}/${TACC_CODE}a9.mif";

MANOUVERS="${MIDMIFDIR}/${TACC_CODE}mn.mid";
MANOUVERSPATH="${MIDMIFDIR}/${TACC_CODE}mp.txt";
NETWORKMID="${MIDMIFDIR}/${TACC_CODE}nw.mid";
NETWORKMIF="${MIDMIFDIR}/${TACC_CODE}nw.mif";
ELEMINAREA="${MIDMIFDIR}/${TACC_CODE}ta.txt";

LANDCOVERMID="${MIDMIFDIR}/${TACC_CODE}lc.mid";
LANDCOVERMIF="${MIDMIFDIR}/${TACC_CODE}lc.mif";
LANDUSEMID="${MIDMIFDIR}/${TACC_CODE}lu.mid";
LANDUSEMIF="${MIDMIFDIR}/${TACC_CODE}lu.mif";
RAILWAYMID="${MIDMIFDIR}/${TACC_CODE}rr.mid";
RAILWAYMIF="${MIDMIFDIR}/${TACC_CODE}rr.mif";
WATERAREAMID="${MIDMIFDIR}/${TACC_CODE}wa.mid";
WATERAREAMIF="${MIDMIFDIR}/${TACC_CODE}wa.mif";
WATERLINEMID="${MIDMIFDIR}/${TACC_CODE}wl.mid";
WATERLINEMIF="${MIDMIFDIR}/${TACC_CODE}wl.mif";

# pois
CITYCENTREMID="${MIDMIFDIR}/${TACC_CODE}sm.mid";
CITYCENTREMIF="${MIDMIFDIR}/${TACC_CODE}sm.mif";
POIMID="${MIDMIFDIR}/${TACC_CODE}pi.mid";
POIMIF="${MIDMIFDIR}/${TACC_CODE}pi.mif";
PIEA="${MIDMIFDIR}/${TACC_CODE}piea.txt";

# grep for Pline Multiple in mif files
#PLINEMULTIPLES="";

# Debug output
if [ "$PRINTDEBUG" = "yes" ]; then
	echo "BASEGENFILESPATH: $BASEGENFILESPATH"
	echo "RELEASEDIR:       $RELEASEDIR"
	echo "CC_CODE:          $CC_CODE"
	echo "TACC_CODE:        $TACC_CODE"
	echo "MIDMIFDIR:        $MIDMIFDIR"
	echo "ADMINALTNAMES:    $ADMINALTNAMES"
fi

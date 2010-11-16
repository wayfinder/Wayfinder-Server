#!/bin/sh
#
# Improvements
# * gz files should be unpacked automatically and only temporary (or piped to ogr2ogr if that is possible)
# * 


ORIG=""
DEST="."
PATH_DBF2CSV="$(dirname $(readlink -f ${BASH_SOURCE[0]}))/dbf2csv.pl"

print_usage()
{
echo "usage: $0 [-o directory] [-c filename] [-i directory]"
echo "
        -c dbf2csv_path
        Where to find the helper script dbf2csv.pl, used to convert the 
        database files to CVS files. Default value is dbf2csv.pl in the 
	same directory as this script. The perl script can be downloaded
	from http://www.burtonsys.com/downloads.html

        -o output_directory_path
        Where to write the converted files. Default value: .

	-i input_directory_path
	Mandatory parameter, setting the path to find the files to convert.
"
}

# Get orgin and destination directories from command line
while getopts ho:i:c: opt
do
    case "$opt" in
      o)  DEST="$OPTARG";;
      i)  ORIG="$OPTARG";;
      c)  PATH_DBF2CSV="$OPTARG";;
      h)  print_usage
          exit 1;;
      \?)               # unknown flag
          print_usage
          exit 1;;
    esac
done
shift `expr $OPTIND - 1`

if [ "$ORIG" == "" ]; then
   echo "Missing input directory"
   print_usage
   exit 1
fi

# Get orgin and destination directories from command line
echo "Converting data in $ORIG to files in $DEST"

# Create the necessary mid/mif files for all shp files in $ORIG
for p in "a0" "a1" "a8" "a9" "bu" "lc" "lu" "mn" "nw" "pi" "rr" "sm" "wa" "wl"
do
   i=`find $ORIG -maxdepth 1 -iname "*$p.shp"`
   # Replace multiple __ with a single underscore for convenience
   m="$DEST/`echo $i | sed -e "s/.*\///" |sed -e "s/\.shp$/.\mif/I" | sed -e "s/__*/_/"`"
   echo "ogr2ogr -f 'MapInfo File' $m $i"
   `ogr2ogr -f 'MapInfo File' $m $i`
done              

# Create csv files for some DBX files in $ORIG
for i in "an.dbf" "mp.dbf" "pc.dbf" "piea.dbf" "ta.dbf"
do
   f=`find $ORIG -maxdepth 1 -iname "*$i"`
   perl $PATH_DBF2CSV $f
   basePathFile=`echo $f | sed -e "s/\.dbf$//I"`
   destFile=`echo $basePathFile| sed -e "s/.*\///" | sed -e "s/__*/_/"`
   mv $basePathFile.csv $DEST/$destFile.txt
done

# Run dos2unix on all files to ensure the line breaks are as expected
dos2unix $DEST/*.*

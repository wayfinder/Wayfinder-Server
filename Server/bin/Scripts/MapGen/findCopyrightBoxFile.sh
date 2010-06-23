#!/bin/sh -x


# BASEGENFILESPATH 
# genfiles is the base directory where all setting files for map generation
# is stored
# Update it to point to the full path of where you create the BASEGENFILESPATH
#BASEGENFILESPATH="fullpath/genfiles";
BASEGENFILESPATH="."


source ${BASEGENFILESPATH}/script/mfunctions.sh

# Read input arguments.
argNbr=0;  # Index of non-flagged parameter.
pos=1;     # Corresponds to index of positional parameter.
flaggedArg=""; #false
for i; do 
   case $i in
      -h) echo "`basename $0` [OPTIONS] DIRTOSEARCH MAPVERBYCOUNTRYFILE"
          echo ""
          echo "OPTIONS"
          echo "-validateIdFiles"
          echo "Makes sure that only one file with the same map versions"
          echo "exist in the directory where the files are stored. No need"
          echo "to give MAPVERBYCOUNTRYFILE with this option."
          echo ""
          exit;;
       -validateIdFiles) validateIdFiles="true";;
###    -flagWithArg) eval theArgOfTheFlag=$`echo $[pos+1]`;
###           flaggedArg="true";;
      -*) echo "Invalid option $i"; exit;;
      *) if [ ${flaggedArg} ]; then
           flaggedArg=""; #false
         else
            eval arg${argNbr}="$i";
            argNbr=$[argNbr+1]
         fi;;
   esac
   pos=$[pos+1];
done

# Print the commandline to the log.
commandLine="$0";
for i; do
    commandLine="${commandLine} $i";
done
#Only foundFile as output from this script echo "Command line: ${commandLine}";


dirToSearch=${arg0};
mapVerByCountryFile=${arg1};

if [ "${validateIdFiles}" ]; then
    # Validation mode.

    # Validate input parameters
    if [ ! "${dirToSearch}" ]; then
        "Missing parameter DIRTOSEARCH, se -h";
        exit 1;
    fi

    result="";
    for file1 in `ls -1r $dirToSearch/*_crb.txt`; do
        for file2 in `ls -1r $dirToSearch/*_crb.txt`; do
            if [ "$file1" = "$file2" ]; then
                ok=$ok; #Nothing to do.
            else
                #echo "Checking $file1 $file2";
                noMatch="`compareMapVersioByCountrySuppliers $file1 $file2`";
                if [ ! "${noMatch}" ]; then
                    result="$result ($file1 == $file2)";
                fi
            fi
        done
    done
    if [ "$result" ]; then
        echo -"$result";
    fi
  
else
    # Normal mode.
    
    # Validate input parameters
    if [ ! "${dirToSearch}" ]; then
        "Missing parameter DIRTOSEARCH, se -h";
        exit 1;
    fi
    if [ ! "${mapVerByCountryFile}" ]; then
        "Missing parameter MAPVERBYCOUNTRYFILE, se -h";
        exit 1;
    fi


    foundFile="";
    for file in `ls -1r $dirToSearch/*_crb.txt`; do
        if [ ! "$foundFile" ]; then
            noMatch="`compareMapVersioByCountrySuppliers $mapVerByCountryFile $file`";
            if [ ! "$noMatch" ]; then
                foundFile="$file";
            fi
        fi
    done
    
    if [ "$foundFile" ]; then
        echo "$foundFile";
    fi
fi    



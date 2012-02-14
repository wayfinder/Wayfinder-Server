#!/bin/sh
# This file contains functions used by other shell scripts.


# BASEGENFILESPATH 
# genfiles is the base directory where all setting files for map generation
# is stored
# Update it to point to the full path of where you create the BASEGENFILESPATH
#BASEGENFILESPATH="fullpath/genfiles";
#BASEGENFILESPATH="."
#Set the BASEGENFILESPATH assuming this script is located in genfiles/script
if [ -z "$BASEGENFILESPATH" ] ; then
    BASEGENFILESPATH=`echo $(dirname $(readlink -f ${BASH_SOURCE[0]})) | sed -e "s/\/script$//"`
fi

# Change to something else if you don't want the temp log files here
# The directory must exist
TEMPLOGFILEPATH="."
#TEMPLOGFILEPATH="$something/tmpMapGenLogs"




# Translates hostnames to names possible to use with ssh.
# 
function mhostname()
{
    local tmpname;
 
    tmpname=`/bin/hostname`
    echo "${tmpname}"
}

function chk_res() {
   exitcode=$?
   if (test $exitcode != 0); then
      echo "Nonzero exitcode (${exitcode})!"
      echo "Exiting script!"
      exit $exitcode
   fi
}

function chk_res_no_exit() {
   exitcode=$?
   if (test $exitcode != 0); then
      echo "Nonzero exitcode (${exitcode})!"
   fi
}

function chk_log(){
   exitcode=$?
   noExit="$1";  # set parameter to anything to optaion no exit. E.g. "noExit".
   #echo "$kasdole_localLogFileName";
   #echo "$kasdole_realLogFileName";
   mv $loclog $kasdole_realLogFileName;


   if (test $exitcode != 0); then
       if ( test "$noExit" ); then
           echo "Nonzero exitcode (${exitcode})!"
           echo "Not exiting!";
       else
           echo "Nonzero exitcode (${exitcode})!"
           echo "Exiting script!"
           exit $exitcode
       fi
   fi

}

function preploclog() {
    kasdole_realLogFileName=$1; # Prefixed wiht kasdole to make the variable 
    # name uniq.
    logFileName=`basename $kasdole_realLogFileName`;

    localDir="${TEMPLOGFILEPATH}"; # temporary location for logs.
    mkdir -p $localDir;
    #export kasdole_localLogFileName=`mktemp ${localDir}/${logFileName}.XXXXXXXXX`;
    loclog=`mktemp ${localDir}/${logFileName}.XXXXXXX`;
    touch $loclog;
    #chgrp devel $loclog;
    chmod g+rw $loclog;

    echo "Find local log in "`hostname`":$loclog" > $kasdole_realLogFileName;
    #echo "$kasdole_localLogFileName";
}

# Returns the code that is sent in
function mexit() {
    return $1;
}

# Takes care of writing to log.
function writeLog() {
   logFile=$1;

   localDir="${TEMPLOGFILEPATH}";

   mkdir -p $localDir;

   logFileName=`basename $logFile`;
   localLogFile=`mktemp ${localDir}/${logFileName}.XXXXXXXXX`;

   cat > ${localLogFile};
   mv $localLogFile $logFile;
}


function countryOrder() {
    countryList="";
    echo "`${BASEGENFILESPATH}/script/maps_countryInfo.pl -o $*`";

}

function getDrivingSide() {
   if [ ! -e "${BASEGENFILESPATH}/script/maps_countryInfo.pl" ]; then
      echo "No maps_countryInfo.pl in genfiles/script"
      exit 1;
   fi
   country="$1";
   side="`${BASEGENFILESPATH}/script/maps_countryInfo.pl -d ${country}`";
   echo "${side}";
}

function getDatabaseCountryName() {
   country="$1";
   alpha2="`${BASEGENFILESPATH}/script/maps_countryInfo.pl -i ${country}`";
   if [ ! "$alpha2" ]; then
      echo ""
   else
      name="`${BASEGENFILESPATH}/script/maps_countryInfo.pl -e ${alpha2}`";
      echo "$name";
  fi
}



function extradataShIsUpToDate {
    fileAndPath="$1/extradata.sh"
    echo "`fileIsUpToDate $fileAndPath`";
}

function fileIsUpToDate {
    fileAndPath="$1";
    path="`dirname $fileAndPath`"
    file="`basename $fileAndPath`"
    cvsdir=$1;
    if [ ${path} ]; then
        pushd $path  > /dev/null
    else
        echo "Error in fileIsUpToDate path"
        echo "path: $path";
        exit 1;
    fi

    # Check if the file to check is among the incoming to this
    # repository ($path)
    hgIncoming="`hg -q incoming --template '{files}\n'|grep ${file}`";

    if [ -z "$hgIncoming" ]; then
      echo "true"
    fi 
    popd > /dev/null
}


# Returns a list of IDs with the right number of zeroes and in hex, from
# from to to. E.g. 00000001a. The list includes the to value.
#
# @param from The from ID. Can be on the form 0x0 for hexadecimal.
# @param to The to ID. Can be on the form 0x0 for hexadecimal.
#
function getMapIDList {
    from="$1";
    to="$2";

    i=$[from];
    max=$[to+1];
    while [ $i -lt $max ]; do 
        printf '%09x\n' $i;
        i=$[i+1]; 
    done;
} #getMapIDList




function countryToISO3166Code() {
    country=$1;

    alpha2="`${BASEGENFILESPATH}/script/maps_countryInfo.pl -i ${country}`";
    echo $alpha2;

} # countryToISO3166Code



# Returns a list of countries being neightbours to the country parameter.
#
# NB!
# The method is used for identifying borders between different map provider
# countries, so not all neighbours are present. Typically countries only 
# neighbouring by sea may be missing. E.g. Sweden and Denmark are neighbours
# but not included in this list because there is the sea between them so 
# they are not sharing a country border at land.
#
# @paramm country The country to get the neighbours of.
function getNeighbours() {
   country="$1";       

   # This is the list of neighbouring countries.
   TMPFILE1=`mktemp ${TEMPLOGFILEPATH}/getNeighboursFirstXXXXXXXXX`;
   TMPFILE2=`mktemp ${TEMPLOGFILEPATH}/getNeighboursSecondXXXXXXXXX`;
   echo "#afghanistan#china#
         #afghanistan#iran#
         #afghanistan#pakistan#
         #afghanistan#tajikistan#
         #afghanistan#turkmenistan#
         #afghanistan#uzbekistan#
         #albania#greece#
         #albania#macedonia#
         #albania#serbia_montenegro#
         #algeria#libya#
         #algeria#mali#
         #algeria#mauritania#
         #algeria#morocco#
         #algeria#niger#
         #algeria#tunisia#
         #andorra#spain#
         #angola#congo#
         #angola#dr_congo#
         #angola#namibia#
         #angola#zambia#
         #argentina#bolivia#
         #argentina#brazil#
         #argentina#chile#
         #argentina#paraguay#
         #argentina#uruguay#
         #armenia#azerbaijan#
         #armenia#georgia_country#
         #armenia#iran#
         #austria#czech_republic#
         #austria#germany#
         #austria#hungary#
         #austria#italy#
         #austria#liechtenstein#
         #austria#slovakia#
         #austria#slovenia#
         #austria#switzerland#
         #azerbaijan#georgia_country#
         #azerbaijan#iran#
         #azerbaijan#russia#
         #bangladesh#india#
         #bangladesh#myanmar#
         #belarus#latvia#
         #belarus#lithuania#
         #belarus#poland#
         #belarus#russia#
         #belarus#ukraine#
         #belgium#germany#
         #belgium#luxembourg#
         #belgium#netherlands#
         #belize#guatemala#
         #belize#mexico#
         #benin#niger#
         #benin#nigeria#
         #bhutan#china#
         #bhutan#india#
         #bolivia#brazil#
         #bolivia#chile#
         #bolivia#paraguay#
         #bolivia#peru#
         #bosnia#serbia_montenegro#
         #botswana#namibia#
         #botswana#south_africa#
         #botswana#zambia#
         #botswana#zimbabwe#
         #brazil#colombia#
         #brazil#french_guiana#
         #brazil#guyana#
         #brazil#paraguay#
         #brazil#peru#
         #brazil#suriname#
         #brazil#uruguay#
         #brazil#venezuela#
         #brunei_darussalam#malaysia#
         #bulgaria#macedonia#
         #bulgaria#romania#
         #bulgaria#serbia_montenegro#
         #burkina_faso#ghana#
         #burkina_faso#ivory_coast#
         #burkina_faso#mali#
         #burkina_faso#niger#
         #burkina_faso#togo#
         #burundi#dr_congo#
         #burundi#rwanda#
         #burundi#tanzania#
         #cambodia#laos#
         #cambodia#thailand#
         #cambodia#vietnam#
         #cameroon#central_african_republic#
         #cameroon#chad#
         #cameroon#congo#
         #cameroon#equatorial_guinea#
         #cameroon#gabon#
         #cameroon#nigeria#
         #canada#usa#
         #central_african_republic#chad#
         #central_african_republic#congo#
         #central_african_republic#dr_congo#
         #central_african_republic#sudan#
         #chad#libya#
         #chad#niger#
         #chad#nigeria#
         #chad#sudan#
         #chile#peru#
         #china#hong_kong#
         #china#india#
         #china#kazakhstan#
         #china#kyrgyzstan#
         #china#laos#
         #china#macao#
         #china#mongolia#
         #china#myanmar#
         #china#nepal#
         #china#north_korea#
         #china#pakistan#
         #china#russia#
         #china#tajikistan#
         #china#vietnam#
         #colombia#ecuador#
         #colombia#panama#
         #colombia#peru#
         #colombia#venezuela#
         #congo#dr_congo#
         #congo#gabon#
         #costa_rica#panama#
         #croatia#hungary#
         #croatia#serbia_montenegro#
         #croatia#slovenia#
         #czech_republic#germany#
         #czech_republic#poland#
         #czech_republic#slovakia#
         #denmark#germany#
         #denmark#sweden#
         #djibouti#eritrea#
         #djibouti#ethiopia#
         #djibouti#somalia#
         #dr_congo#rwanda#
         #dr_congo#sudan#
         #dr_congo#tanzania#
         #dr_congo#uganda#
         #dr_congo#zambia#
         #ecuador#peru#
         #egypt#israel#
         #egypt#libya#
         #egypt#occupied_palestinian_territory#
         #egypt#sudan#
         #el_salvador#guatemala#
         #el_salvador#honduras#
         #equatorial_guinea#gabon#
         #eritrea#ethiopia#
         #eritrea#sudan#
         #estonia#latvia#
         #estonia#russia#
         #ethiopia#kenya#
         #ethiopia#somalia#
         #ethiopia#sudan#
         #fiji#tonga#
         #finland#norway#
         #finland#russia#
         #finland#sweden#
         #french_guiana#suriname#
         #gambia#senegal#
         #georgia_country#russia#
         #germany#luxembourg#
         #germany#netherlands#
         #germany#poland#
         #germany#switzerland#
         #ghana#ivory_coast#
         #greece#macedonia#
         #guatemala#honduras#
         #guatemala#mexico#
         #guinea#guinea_bissau#
         #guinea#ivory_coast#
         #guinea#liberia#
         #guinea#mali#
         #guinea#sierra_leone#
         #guinea_bissau#senegal#
         #guyana#suriname#
         #guyana#venezuela#
         #honduras#nicaragua#
         #hungary#romania#
         #hungary#serbia_montenegro#
         #hungary#slovakia#
         #hungary#slovenia#
         #hungary#ukraine#
         #india#myanmar#
         #india#nepal#
         #india#pakistan#
         #indonesia#malaysia#
         #indonesia#papua_new_guinea#
         #indonesia#timor_leste#
         #iran#iraq#
         #iran#pakistan#
         #iran#turkmenistan#
         #iraq#jordan#
         #iraq#kuwait#
         #iraq#saudi_arabia#
         #iraq#syria#
         #israel#jordan#
         #israel#lebanon#
         #israel#occupied_palestinian_territory#
         #israel#syria#
         #italy#slovenia#
         #italy#switzerland#
         #ivory_coast#liberia#
         #ivory_coast#mali#
         #jordan#saudi_arabia#
         #jordan#syria#
         #kazakhstan#kyrgyzstan#
         #kazakhstan#russia#
         #kazakhstan#turkmenistan#
         #kazakhstan#uzbekistan#
         #kenya#sudan#
         #kenya#tanzania#
         #kenya#uganda#
         #kuwait#saudi_arabia#
         #kyrgyzstan#tajikistan#
         #kyrgyzstan#uzbekistan#
         #laos#myanmar#
         #laos#thailand#
         #laos#vietnam#
         #latvia#lithuania#
         #latvia#russia#
         #lebanon#syria#
         #liberia#sierra_leone#
         #libya#niger#
         #libya#sudan#
         #libya#tunisia#
         #liechtenstein#switzerland#
         #macedonia#serbia_montenegro#
         #malawi#mozambique#
         #malawi#tanzania#
         #malawi#zambia#
         #malaysia#thailand#
         #mali#mauritania#
         #mali#niger#
         #mali#senegal#
         #mauritania#senegal#
         #mauritania#western_sahara#
         #mexico#usa#
         #moldova#romania#
         #moldova#ukraine#
         #mongolia#russia#
         #mozambique#south_africa#
         #mozambique#swaziland#
         #mozambique#tanzania#
         #mozambique#zambia#
         #mozambique#zimbabwe#
         #myanmar#thailand#
         #namibia#south_africa#
         #namibia#zambia#
         #niger#nigeria#
         #north_korea#russia#
         #north_korea#south_korea#
         #norway#russia#
         #norway#sweden#
         #oman#saudi_arabia#
         #oman#uae#
         #oman#yemen#
         #poland#russia#
         #poland#slovakia#
         #portugal#spain#
         #qatar#saudi_arabia#
         #romania#serbia_montenegro#
         #romania#ukraine#
         #russia#ukraine#
         #rwanda#tanzania#
         #rwanda#uganda#
         #saudi_arabia#uae#
         #saudi_arabia#yemen#
         #south_africa#swaziland#
         #south_africa#zimbabwe#
         #sudan#uganda#
         #tajikistan#uzbekistan#
         #tanzania#uganda#
         #tanzania#zambia#
         #turkmenistan#uzbekistan#
         #zambia#zimbabwe#
         #algeria#western_sahara#
         #andorra#france#
         #armenia#turkey#
         #australia#new_zealand#
         #azerbaijan#turkey#
         #belgium#france#
         #benin#burkina_faso#
         #benin#togo#
         #bulgaria#greece#
         #bulgaria#turkey#
         #costa_rica#nicaragua#
         #croatia#bosnia#
         #dominican_republic#haiti#
         #france#spain#
         #france#germany#
         #france#italy#
         #france#monaco#
         #france#switzerland#
         #georgia_country#turkey#
         #greece#turkey#
         #iraq#turkey#
         #iran#turkey#
         #ireland#uk#
         #kenya#somalia#
         #lithuania#russia#
         #lithuania#poland#
         #morocco#spain#
         #morocco#western_sahara#
         #poland#ukraine#
         #senegal#guinea#
         #singapore#malaysia#
         #slovakia#ukraine#
         #syria#turkey#" > ${TMPFILE1}

   grep "#${country}#" ${TMPFILE1} | sed -e "s/#${country}#//" | \
         sed -e 's/#//' | sed -e 's/\[ ]//g' | sed -e 's/^/ /' > ${TMPFILE2}

   # Return value
   while read line; do
       echo -n " $line";
   done < ${TMPFILE2}
   echo "";
   

   rm ${TMPFILE1};
   rm ${TMPFILE2};


} # getNeighbours


function distBackup() {
    dirName=$1;
    shift;
    filePattern=$1;
    shift;
    computers="$*";

    mkdir -p $dirName
    chk_res
    for file in `echo $filePattern | xargs ls`; do
        echo "cd `pwd`;bzip2 -c ${file} > $dirName/${file}.bz2" 
        chk_res
    done > distributedBackup.sh
    ./distribute -e $computers < distributedBackup.sh
    chk_res
    
}


function createMapVersionByCountryFile(){
    typeset countriesDir="$1";
    shift;
    typeset countries="$*";
    TMPFILE1=`mktemp mapVerByCountry1_XXXXXXXXXX`;
    TMPFILE2=`mktemp ${TEMPLOGFILEPATH}/mapVerByCountry2_XXXXXXXXXX`;

    for country in $countries; do
        echo -n "$country ";
        cat $countriesDir/$country/before_merge/mapOrigin.txt
    done | sort > $TMPFILE1;

    # Check that the file created does not include any invalid map supplier
    # names.
    cat $TMPFILE1 | sed -e 's/\(^[^ ]* [^_]*_[^0-9]\)/Broken: \1/' > $TMPFILE2;
    diffResult="`diff $TMPFILE1 $TMPFILE2`";
    if [ "$diffResult" ]; then
        echo "Some problem with map supplier name, they may not include "
        echo "\"_\" sings. Look for \"Broken\" in $TMPFILE2";
        rm $TMPFILE1;
        exit 1;
    fi
    echo "`pwd`/$TMPFILE1";
    rm $TMPFILE2;
}


function compareMapVersioByCountrySuppliers(){
    typeset file1=$1;
    typeset file2=$2;
    TMPFILE1=`mktemp ${TEMPLOGFILEPATH}/mapVerByCountryOnlyMapSup1_XXXXXXXXXX`;
    TMPFILE2=`mktemp ${TEMPLOGFILEPATH}/mapVerByCountryOnlyMapSup2_XXXXXXXXXX`;
    
    # This comparison depends on that no _-sign is included in the 
    # map supplier name. This is checked when creating the file with
    # createMapVersionByCountryFile
    cat $file1 | sed -e 's/\(^[^ ]* [^_]*\).*/\1/' > $TMPFILE1;
    cat $file2 | sed -e 's/\(^[^ ]* [^_]*\).*/\1/' > $TMPFILE2;
    theDiff="`diff $TMPFILE1 $TMPFILE2`";
    echo "$theDiff";

    rm $TMPFILE1 $TMPFILE2;
} 


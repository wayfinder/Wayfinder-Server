

# BASEGENFILESPATH 
# genfiles is the base directory where all setting files for map generation
# is stored
# Update it to point to the full path of where you create the BASEGENFILESPATH
#BASEGENFILESPATH="fullpath/genfiles";
#BASEGENFILESPATH="."
if [ -z "$BASEGENFILESPATH" ] ; then
    BASEGENFILESPATH=`echo $(dirname $(readlink -f ${BASH_SOURCE[0]})) | sed -e "s/\/script$//"`
fi


# Change to something else if you don't want the temp log files here
# The directory must exist
TEMPLOGFILEPATH="."
#TEMPLOGFILEPATH="$something/tmpMapGenLogs"




# Determine what kind of exit code check to use.
#checkFunction="chk_res_no_exit";
checkFunction="chk_res"; # Now we want it to crash.


# Function for including extradata commands
# Arguments: country   The name of the country to include extra data for.
#            insertion The insertionpoint in the script. 
#                      Possible values: after_extitm   Extra item
#                                       after_intconn  Internal connections
#                                       after_trngen  Street generation 
#                                       after_setloc   Set location
#                                       before_merge   Backup used by merge.
#                                       after_extconn  External connections
#                                       after_ovrmaps  Overview maps
#                                       after_addreg   Add region
#            mapRelease The release of the maps. E.g. TA_2008_07.
#
# Some notes:
# - IMPORTANT: use typeset to declare all variables so they do not 
#   influence variables of the calling script.

function extradata()
{
   typeset COUNTRY=$1;
   typeset INSERTION=$2;
   typeset mapRelease=$3

   typeset GENFILESPATH="${BASEGENFILESPATH}/countries"
   # Country specific extradata. May be overridden below.
   typeset EXTRADATAPATH="${GENFILESPATH}/${COUNTRY}/extradata" 

   # Where logs are put.
   typeset LOGPATH="logpath"

   echo ""
   echo `date +"%Y%m%d %T"`" Start Extradata $COUNTRY $INSERTION.";

## Country template
# Xxx extradata
#   elif [ ${COUNTRY} = xxx ]; then
#      if [ ${INSERTION} = after_extitm ]; then #xxx
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #xxx
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #xxx
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #xxx
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #xxx
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #xxx
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #xxx
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #xxx
#         #NB! These extradata have no effect on merged maps.
#
#      fi




## Example of what can be inserted into the extradata.sh extradata-function
## to make corrections in the maps
#      if [ ${INSERTION} = after_intconn ]; then #france
#      # Fix geometry of N118 and A35 
#if [ "$mapRelease" = "TA_2006_04" ]; then
#      # remove faulte street segments with extradata file (map correction record)
#      nice time ./GenerateMapServer -x ${EXTRADATAPATH}/streetsegmentitems_TA_2006_04/removeStreetsegments.ext >& $LOGPATH/removessi.log
#      ${checkFunction}
#      # add new correct geometry with street segments from a file in wayfinder midmif format
#      nice time ./GenerateMapServer --useCoordToFindCorrectMap -l -r ${EXTRADATAPATH}/streetsegmentitems_TA_2006_04/streetsegmentitem >& $LOGPATH/addssi.log
#      ${checkFunction}
#fi
#      # Island items
#if [ "$mapRelease" = "TA_2009_09" ]; then
#      # add islands from a file in wayfinder midmif format
#      nice time ./GenerateMapServer --useCoordToFindCorrectMap -r ${EXTRADATAPATH}/islands/islanditemsTA2009_09 >& $LOGPATH/addislands.log
#      ${checkFunction}
#fi
#      fi # end after_intconn






# Afghanistan extradata
   if [ ${COUNTRY} = afghanistan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #afghanistan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #afghanistan
#      fi



# Albania extradata
   elif [ ${COUNTRY} = albania ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #albania
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #albania
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #albania
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #albania
#      fi
#      if [ ${INSERTION} = before_merge ]; then #albania
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #albania
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #albania
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #albania
#      fi



# Algeria extradata
   elif [ ${COUNTRY} = algeria ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #algeria
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #algeria
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #algeria
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #algeria
#      fi
#      if [ ${INSERTION} = before_merge ]; then #algeria
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #algeria
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #algeria
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #algeria
#      fi



# American Samoa extradata
   elif [ ${COUNTRY} = american_samoa ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = before_merge ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #american_samoa
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #american_samoa
#      fi



# Andorra extradata
   elif [ ${COUNTRY} = andorra ]; then
       ok=""; #Empty statment replacer
#      fi
#      if [ ${INSERTION} = after_extitm ]; then #andorra
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #andorra
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #andorra
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #andorra
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #andorra
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #andorra
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #andorra
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #andorra
#
#      fi





# Angola extradata
   elif [ ${COUNTRY} = angola ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #angola
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #angola
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #angola
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #angola
#      fi
#      if [ ${INSERTION} = before_merge ]; then #angola
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #angola
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #angola
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #angola
#      fi



# Anguilla extradata
   elif [ ${COUNTRY} = anguilla ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #anguilla
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #anguilla
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #anguilla
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #anguilla
#      fi
#      if [ ${INSERTION} = before_merge ]; then #anguilla
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #anguilla
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #anguilla
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #anguilla
#      fi



# Antarctica extradata
   elif [ ${COUNTRY} = antarctica ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #antarctica
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #antarctica
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #antarctica
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #antarctica
#      fi
#      if [ ${INSERTION} = before_merge ]; then #antarctica
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #antarctica
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #antarctica
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #antarctica
#      fi



# Antigua and Barbuda extradata
   elif [ ${COUNTRY} = antigua_and_barbuda ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = before_merge ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #antigua_and_barbuda
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #antigua_and_barbuda
#      fi



# Argentina extradata
   elif [ ${COUNTRY} = argentina ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #argentina
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #argentina
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #argentina
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #argentina
#      fi
#      if [ ${INSERTION} = before_merge ]; then #argentina
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #argentina
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #argentina
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #argentina
#      fi



# Armenia extradata
   elif [ ${COUNTRY} = armenia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #armenia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #armenia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #armenia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #armenia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #armenia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #armenia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #armenia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #armenia
#      fi



# Aruba extradata
   elif [ ${COUNTRY} = aruba ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #aruba
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #aruba
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #aruba
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #aruba
#      fi
#      if [ ${INSERTION} = before_merge ]; then #aruba
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #aruba
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #aruba
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #aruba
#      fi



# Australia extradata
   elif [ ${COUNTRY} = australia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #australia
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #australia
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #australia
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #australia
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #australia
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #australia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #australia
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #australia
#
#      fi





# Austria extradata
   elif [ ${COUNTRY} = austria ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #austria
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #austria
#
#      fi # end after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #austria
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #austria
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #austria
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #austria
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #austria
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #austria
#
#      fi



# Azerbaijan extradata
   elif [ ${COUNTRY} = azerbaijan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #azerbaijan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #azerbaijan
#      fi



# Bahamas extradata
   elif [ ${COUNTRY} = bahamas ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bahamas
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bahamas
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bahamas
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bahamas
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bahamas
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bahamas
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bahamas
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bahamas
#      fi



# Bahrain extradata
   elif [ ${COUNTRY} = bahrain ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bahrain
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bahrain
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bahrain
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bahrain
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bahrain
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bahrain
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bahrain
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bahrain
#
#      fi



# Bangladesh extradata
   elif [ ${COUNTRY} = bangladesh ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bangladesh
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bangladesh
#      fi



# Barbados extradata
   elif [ ${COUNTRY} = barbados ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #barbados
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #barbados
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #barbados
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #barbados
#      fi
#      if [ ${INSERTION} = before_merge ]; then #barbados
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #barbados
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #barbados
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #barbados
#      fi



# Belarus extradata
   elif [ ${COUNTRY} = belarus ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #belarus
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #belarus
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #belarus
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #belarus
#      fi
#      if [ ${INSERTION} = before_merge ]; then #belarus
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #belarus
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #belarus
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #belarus
#      fi



# Belgium extradata
   elif [ ${COUNTRY} = belgium ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #belgium
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #belgium
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #belgium
#      
#      fi  # after_trngen
#      if [ ${INSERTION} = after_setloc ]; then #belgium
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #belgium
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #belgium
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #belgium
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #belgium
#
#      fi



# Belize extradata
   elif [ ${COUNTRY} = belize ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #belize
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #belize
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #belize
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #belize
#      fi
#      if [ ${INSERTION} = before_merge ]; then #belize
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #belize
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #belize
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #belize
#      fi



# Benin extradata
   elif [ ${COUNTRY} = benin ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #benin
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #benin
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #benin
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #benin
#      fi
#      if [ ${INSERTION} = before_merge ]; then #benin
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #benin
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #benin
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #benin
#      fi



# Bermuda extradata
   elif [ ${COUNTRY} = bermuda ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bermuda
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bermuda
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bermuda
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bermuda
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bermuda
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bermuda
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bermuda
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bermuda
#      fi



# Bhutan extradata
   elif [ ${COUNTRY} = bhutan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bhutan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bhutan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bhutan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bhutan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bhutan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bhutan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bhutan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bhutan
#      fi



# Bolivia extradata
   elif [ ${COUNTRY} = bolivia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bolivia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bolivia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bolivia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bolivia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bolivia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bolivia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bolivia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bolivia
#      fi



# Bosnia and Herzegovina extradata
   elif [ ${COUNTRY} = bosnia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bosnia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bosnia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bosnia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bosnia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bosnia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bosnia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bosnia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bosnia
#      fi



# Botswana extradata
   elif [ ${COUNTRY} = botswana ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #botswana
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #botswana
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #botswana
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #botswana
#      fi
#      if [ ${INSERTION} = before_merge ]; then #botswana
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #botswana
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #botswana
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #botswana
#      fi



# Brazil extradata
   elif [ ${COUNTRY} = brazil ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #brazil
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #brazil
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #brazil
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #brazil
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #brazil
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #brazil
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #brazil
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #brazil
#
#      fi




# British Virgin Islands extradata
   elif [ ${COUNTRY} = british_virgin_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #british_virgin_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #british_virgin_islands
#      fi



# Brunei Darussalam extradata
   elif [ ${COUNTRY} = brunei_darussalam ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = before_merge ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #brunei_darussalam
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #brunei_darussalam
#      fi



# Bulgaria extradata
   elif [ ${COUNTRY} = bulgaria ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = before_merge ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #bulgaria
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #bulgaria
#      fi



# Burkina Faso extradata
   elif [ ${COUNTRY} = burkina_faso ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = before_merge ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #burkina_faso
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #burkina_faso
#      fi



# Burundi extradata
   elif [ ${COUNTRY} = burundi ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #burundi
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #burundi
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #burundi
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #burundi
#      fi
#      if [ ${INSERTION} = before_merge ]; then #burundi
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #burundi
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #burundi
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #burundi
#      fi



# Cambodia extradata
   elif [ ${COUNTRY} = cambodia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cambodia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cambodia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cambodia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cambodia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cambodia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cambodia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cambodia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cambodia
#      fi



# Cameroon extradata
   elif [ ${COUNTRY} = cameroon ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cameroon
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cameroon
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cameroon
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cameroon
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cameroon
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cameroon
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cameroon
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cameroon
#      fi



# Canada extradata
   elif [ ${COUNTRY} = canada ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #canada
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #canada
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #canada
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #canada
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #canada
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #canada
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #canada
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #canada
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# Cape Verde extradata
   elif [ ${COUNTRY} = cape_verde ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cape_verde
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cape_verde
#      fi



# Cayman Islands extradata
   elif [ ${COUNTRY} = cayman_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cayman_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cayman_islands
#      fi



# Central African Republic extradata
   elif [ ${COUNTRY} = central_african_republic ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = before_merge ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #central_african_republic
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #central_african_republic
#      fi



# Chad extradata
   elif [ ${COUNTRY} = chad ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #chad
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #chad
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #chad
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #chad
#      fi
#      if [ ${INSERTION} = before_merge ]; then #chad
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #chad
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #chad
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #chad
#      fi



# Chile extradata
   elif [ ${COUNTRY} = chile ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #chile
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #chile
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #chile
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #chile
#      fi
#      if [ ${INSERTION} = before_merge ]; then #chile
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #chile
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #chile
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #chile
#      fi



# China extradata
   elif [ ${COUNTRY} = china ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #china
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #china
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #china
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #china
#      fi
#      if [ ${INSERTION} = before_merge ]; then #china
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #china
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #china
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #china
#      fi



# Colombia extradata
   elif [ ${COUNTRY} = colombia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #colombia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #colombia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #colombia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #colombia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #colombia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #colombia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #colombia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #colombia
#      fi



# Comoros extradata
   elif [ ${COUNTRY} = comoros ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #comoros
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #comoros
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #comoros
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #comoros
#      fi
#      if [ ${INSERTION} = before_merge ]; then #comoros
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #comoros
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #comoros
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #comoros
#      fi



# Congo extradata
   elif [ ${COUNTRY} = congo ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #congo
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #congo
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #congo
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #congo
#      fi
#      if [ ${INSERTION} = before_merge ]; then #congo
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #congo
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #congo
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #congo
#      fi



# Cook Islands extradata
   elif [ ${COUNTRY} = cook_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cook_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cook_islands
#      fi



# Costa Rica extradata
   elif [ ${COUNTRY} = costa_rica ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = before_merge ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #costa_rica
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #costa_rica
#      fi



# Croatia extradata
   elif [ ${COUNTRY} = croatia ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #croatia
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #croatia
#      
#      fi #  end   after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #croatia
#       
#      fi # end after_trngen
#      if [ ${INSERTION} = after_setloc ]; then #croatia
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #croatia
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #croatia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #croatia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #croatia
#         #NB! These extradata have no effect on merged maps.
#
#      fi





# Cuba extradata
   elif [ ${COUNTRY} = cuba ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cuba
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cuba
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cuba
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cuba
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cuba
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cuba
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cuba
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cuba
#      fi



# Cyprus extradata
   elif [ ${COUNTRY} = cyprus ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #cyprus
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #cyprus
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #cyprus
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #cyprus
#      fi
#      if [ ${INSERTION} = before_merge ]; then #cyprus
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #cyprus
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #cyprus
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #cyprus
#      fi



# Czech Republic extradata
   elif [ ${COUNTRY} = czech_republic ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #czech_republic
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #czech_republic
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #czech_republic
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #czech_republic
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #czech_republic
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #czech_republic
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #czech_republic
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #czech_republic
#         #NB! These extradata have no effect on merged maps.
#
#     fi




# Denmark extradata
   elif [ ${COUNTRY} = denmark ]; then
       ok=""; #Empty statment replacer


#      if [ ${INSERTION} = after_extitm ]; then #denmark
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #denmark
#
#      fi # after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #denmark
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #denmark
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #denmark
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #denmark
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #denmark
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #denmark
#
#      fi






# Djibouti extradata
   elif [ ${COUNTRY} = djibouti ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #djibouti
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #djibouti
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #djibouti
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #djibouti
#      fi
#      if [ ${INSERTION} = before_merge ]; then #djibouti
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #djibouti
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #djibouti
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #djibouti
#      fi



# Dominica extradata
   elif [ ${COUNTRY} = dominica ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #dominica
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #dominica
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #dominica
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #dominica
#      fi
#      if [ ${INSERTION} = before_merge ]; then #dominica
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #dominica
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #dominica
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #dominica
#      fi



# Dominican Republic extradata
   elif [ ${COUNTRY} = dominican_republic ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = before_merge ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #dominican_republic
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #dominican_republic
#      fi



# D.R. Congo extradata
   elif [ ${COUNTRY} = dr_congo ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = before_merge ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #dr_congo
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #dr_congo
#      fi



# Ecuador extradata
   elif [ ${COUNTRY} = ecuador ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #ecuador
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #ecuador
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #ecuador
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #ecuador
#      fi
#      if [ ${INSERTION} = before_merge ]; then #ecuador
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #ecuador
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #ecuador
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #ecuador
#      fi



# Egypt extradata
   elif [ ${COUNTRY} = egypt ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #egypt
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #egypt
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #egypt
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #egypt
#      fi
#      if [ ${INSERTION} = before_merge ]; then #egypt
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #egypt
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #egypt
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #egypt
#      fi



# El Salvador extradata
   elif [ ${COUNTRY} = el_salvador ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = before_merge ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #el_salvador
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #el_salvador
#      fi



# Equatorial Guinea extradata
   elif [ ${COUNTRY} = equatorial_guinea ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = before_merge ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #equatorial_guinea
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #equatorial_guinea
#      fi



# Eritrea extradata
   elif [ ${COUNTRY} = eritrea ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #eritrea
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #eritrea
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #eritrea
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #eritrea
#      fi
#      if [ ${INSERTION} = before_merge ]; then #eritrea
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #eritrea
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #eritrea
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #eritrea
#      fi



# Estonia extradata
   elif [ ${COUNTRY} = estonia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #estonia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #estonia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #estonia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #estonia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #estonia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #estonia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #estonia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #estonia
#      fi



# Ethiopia extradata
   elif [ ${COUNTRY} = ethiopia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #ethiopia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #ethiopia
#      fi



# Faeroe Islands extradata
   elif [ ${COUNTRY} = faeroe_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #faeroe_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #faeroe_islands
#      fi



# Falkland Islands extradata
   elif [ ${COUNTRY} = falkland_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #falkland_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #falkland_islands
#      fi



# Fiji extradata
   elif [ ${COUNTRY} = fiji ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #fiji
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #fiji
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #fiji
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #fiji
#      fi
#      if [ ${INSERTION} = before_merge ]; then #fiji
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #fiji
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #fiji
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #fiji
#      fi



# Finland extradata
   elif [ ${COUNTRY} = finland ]; then
       ok=""; #Empty statment replacer

#      if [ ${INSERTION} = after_extitm ]; then #finland
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #finland
#
#      fi #  end   after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #finland
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #finland
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #finland
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #finland
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #finland
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #finland
#
#      fi









# France extradata
   elif [ ${COUNTRY} = france ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #france
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #france
# 
#      fi # end after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #france
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #france
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #france
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #france
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #france
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #france
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# French Guiana extradata
   elif [ ${COUNTRY} = french_guiana ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = before_merge ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #french_guiana
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #french_guiana
#      fi



# French Polynesia extradata
   elif [ ${COUNTRY} = french_polynesia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #french_polynesia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #french_polynesia
#      fi



# Gabon extradata
   elif [ ${COUNTRY} = gabon ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #gabon
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #gabon
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #gabon
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #gabon
#      fi
#      if [ ${INSERTION} = before_merge ]; then #gabon
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #gabon
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #gabon
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #gabon
#      fi



# Gambia extradata
   elif [ ${COUNTRY} = gambia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #gambia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #gambia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #gambia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #gambia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #gambia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #gambia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #gambia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #gambia
#      fi



# Georgia extradata
   elif [ ${COUNTRY} = georgia_country ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = before_merge ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #georgia_country
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #georgia_country
#      fi



# Germany extradata
   elif [ ${COUNTRY} = germany ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #germany
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #germany
#
#      fi # end after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #germany
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #germany
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #germany
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #germany
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #germany
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #germany
#         #NB! These extradata have no effect on merged maps.
#
#      fi


# Ghana extradata
   elif [ ${COUNTRY} = ghana ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #ghana
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #ghana
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #ghana
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #ghana
#      fi
#      if [ ${INSERTION} = before_merge ]; then #ghana
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #ghana
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #ghana
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #ghana
#      fi



# Greece extradata
   elif [ ${COUNTRY} = greece ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #greece
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #greece
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #greece
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #greece
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #greece
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #greece
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #greece
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #greece
#         #NB! These extradata have no effect on merged maps.
#
#     fi



# Greenland extradata
   elif [ ${COUNTRY} = greenland ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #greenland
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #greenland
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #greenland
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #greenland
#      fi
#      if [ ${INSERTION} = before_merge ]; then #greenland
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #greenland
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #greenland
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #greenland
#      fi



# Grenada extradata
   elif [ ${COUNTRY} = grenada ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #grenada
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #grenada
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #grenada
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #grenada
#      fi
#      if [ ${INSERTION} = before_merge ]; then #grenada
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #grenada
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #grenada
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #grenada
#      fi



# Guadeloupe extradata
   elif [ ${COUNTRY} = guadeloupe ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = before_merge ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #guadeloupe
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #guadeloupe
#      fi



# Guam extradata
   elif [ ${COUNTRY} = guam ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #guam
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #guam
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #guam
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #guam
#      fi
#      if [ ${INSERTION} = before_merge ]; then #guam
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #guam
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #guam
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #guam
#      fi



# Guatemala extradata
   elif [ ${COUNTRY} = guatemala ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #guatemala
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #guatemala
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #guatemala
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #guatemala
#      fi
#      if [ ${INSERTION} = before_merge ]; then #guatemala
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #guatemala
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #guatemala
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #guatemala
#      fi



# Guinea extradata
   elif [ ${COUNTRY} = guinea ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #guinea
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #guinea
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #guinea
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #guinea
#      fi
#      if [ ${INSERTION} = before_merge ]; then #guinea
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #guinea
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #guinea
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #guinea
#      fi



# Guinea-Bissau extradata
   elif [ ${COUNTRY} = guinea_bissau ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = before_merge ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #guinea_bissau
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #guinea_bissau
#      fi



# Guyana extradata
   elif [ ${COUNTRY} = guyana ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #guyana
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #guyana
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #guyana
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #guyana
#      fi
#      if [ ${INSERTION} = before_merge ]; then #guyana
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #guyana
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #guyana
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #guyana
#      fi



# Haiti extradata
   elif [ ${COUNTRY} = haiti ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #haiti
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #haiti
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #haiti
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #haiti
#      fi
#      if [ ${INSERTION} = before_merge ]; then #haiti
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #haiti
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #haiti
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #haiti
#      fi



# Honduras extradata
   elif [ ${COUNTRY} = honduras ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #honduras
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #honduras
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #honduras
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #honduras
#      fi
#      if [ ${INSERTION} = before_merge ]; then #honduras
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #honduras
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #honduras
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #honduras
#      fi



# Hong Kong extradata
   elif [ ${COUNTRY} = hong_kong ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #hong_kong
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #hong_kong
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #hong_kong
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #hong_kong
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #hong_kong
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #hong_kong
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #hong_kong
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #hong_kong
#
#      fi


# Hungary extradata
   elif [ ${COUNTRY} = hungary ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #hungary
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #hungary
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #hungary
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #hungary
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #hungary
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #hungary
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #hungary
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #hungary
#         #NB! These extradata have no effect on merged maps.
#
#     fi



# Iceland extradata
   elif [ ${COUNTRY} = iceland ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #iceland
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #iceland
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #iceland
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #iceland
#      fi
#      if [ ${INSERTION} = before_merge ]; then #iceland
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #iceland
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #iceland
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #iceland
#      fi



# India extradata
   elif [ ${COUNTRY} = india ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #india
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #india
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #india
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #india
#      fi
#      if [ ${INSERTION} = before_merge ]; then #india
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #india
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #india
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #india
#      fi



# Indonesia extradata
   elif [ ${COUNTRY} = indonesia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #indonesia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #indonesia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #indonesia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #indonesia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #indonesia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #indonesia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #indonesia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #indonesia
#      fi



# Iran extradata
   elif [ ${COUNTRY} = iran ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #iran
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #iran
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #iran
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #iran
#      fi
#      if [ ${INSERTION} = before_merge ]; then #iran
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #iran
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #iran
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #iran
#      fi



# Iraq extradata
   elif [ ${COUNTRY} = iraq ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #iraq
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #iraq
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #iraq
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #iraq
#      fi
#      if [ ${INSERTION} = before_merge ]; then #iraq
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #iraq
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #iraq
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #iraq
#      fi



# Ireland extradata.
   elif [ ${COUNTRY} = ireland ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #ireland
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #ireland
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #ireland
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #ireland
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #ireland
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #ireland
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #ireland
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #ireland
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# Israel extradata
   elif [ ${COUNTRY} = israel ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #israel
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #israel
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #israel
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #israel
#      fi
#      if [ ${INSERTION} = before_merge ]; then #israel
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #israel
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #israel
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #israel
#      fi



# Italy extradata
   elif [ ${COUNTRY} = italy ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #italy
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #italy
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #italy
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #italy
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #italy
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #italy
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #italy
#         #NB! These extradata have no effect on merged maps.#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #italy
#         #NB! These extradata have no effect on merged maps.
#
#      fi




# Ivory Coast extradata
   elif [ ${COUNTRY} = ivory_coast ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = before_merge ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #ivory_coast
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #ivory_coast
#      fi



# Jamaica extradata
   elif [ ${COUNTRY} = jamaica ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #jamaica
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #jamaica
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #jamaica
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #jamaica
#      fi
#      if [ ${INSERTION} = before_merge ]; then #jamaica
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #jamaica
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #jamaica
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #jamaica
#      fi



# Japan extradata
   elif [ ${COUNTRY} = japan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #japan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #japan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #japan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #japan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #japan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #japan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #japan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #japan
#      fi



# Jordan extradata
   elif [ ${COUNTRY} = jordan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #jordan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #jordan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #jordan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #jordan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #jordan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #jordan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #jordan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #jordan
#      fi



# Kazakhstan extradata
   elif [ ${COUNTRY} = kazakhstan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #kazakhstan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #kazakhstan
#      fi



# Kenya extradata
   elif [ ${COUNTRY} = kenya ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #kenya
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #kenya
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #kenya
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #kenya
#      fi
#      if [ ${INSERTION} = before_merge ]; then #kenya
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #kenya
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #kenya
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #kenya
#      fi



# Kiribati extradata
   elif [ ${COUNTRY} = kiribati ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #kiribati
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #kiribati
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #kiribati
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #kiribati
#      fi
#      if [ ${INSERTION} = before_merge ]; then #kiribati
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #kiribati
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #kiribati
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #kiribati
#      fi



# Kuwait extradata
   elif [ ${COUNTRY} = kuwait ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #kuwait
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #kuwait
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #kuwait
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #kuwait
#      fi
#      if [ ${INSERTION} = before_merge ]; then #kuwait
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #kuwait
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #kuwait
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #kuwait
#      fi



# Kyrgyzstan extradata
   elif [ ${COUNTRY} = kyrgyzstan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #kyrgyzstan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #kyrgyzstan
#      fi



# Laos extradata
   elif [ ${COUNTRY} = laos ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #laos
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #laos
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #laos
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #laos
#      fi
#      if [ ${INSERTION} = before_merge ]; then #laos
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #laos
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #laos
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #laos
#      fi



# Latvia extradata
   elif [ ${COUNTRY} = latvia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #latvia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #latvia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #latvia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #latvia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #latvia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #latvia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #latvia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #latvia
#      fi



# Lebanon extradata
   elif [ ${COUNTRY} = lebanon ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #lebanon
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #lebanon
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #lebanon
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #lebanon
#      fi
#      if [ ${INSERTION} = before_merge ]; then #lebanon
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #lebanon
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #lebanon
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #lebanon
#      fi



# Lesotho extradata
   elif [ ${COUNTRY} = lesotho ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #lesotho
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #lesotho
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #lesotho
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #lesotho
#      fi
#      if [ ${INSERTION} = before_merge ]; then #lesotho
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #lesotho
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #lesotho
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #lesotho
#      fi



# Liberia extradata
   elif [ ${COUNTRY} = liberia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #liberia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #liberia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #liberia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #liberia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #liberia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #liberia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #liberia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #liberia
#      fi



# Libya extradata
   elif [ ${COUNTRY} = libya ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #libya
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #libya
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #libya
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #libya
#      fi
#      if [ ${INSERTION} = before_merge ]; then #libya
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #libya
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #libya
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #libya
#      fi



# Liechtenstein extradata
   elif [ ${COUNTRY} = liechtenstein ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #liechtenstein
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #liechtenstein
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #liechtenstein
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #liechtenstein
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #liechtenstein
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #liechtenstein
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #liechtenstein
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #liechtenstein
#
#      fi





# Lithuania extradata
   elif [ ${COUNTRY} = lithuania ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #lithuania
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #lithuania
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #lithuania
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #lithuania
#      fi
#      if [ ${INSERTION} = before_merge ]; then #lithuania
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #lithuania
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #lithuania
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #lithuania
#      fi



# Luxembourg extradata
   elif [ ${COUNTRY} = luxembourg ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #luxembourg
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #luxembourg
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #luxembourg
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #luxembourg
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #luxembourg
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #luxembourg
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #luxembourg
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #luxembourg
#
#      fi





# Macao extradata
   elif [ ${COUNTRY} = macao ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #macao
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #macao
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #macao
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #macao
#      fi
#      if [ ${INSERTION} = before_merge ]; then #macao
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #macao
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #macao
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #macao
#      fi



# Macedonia extradata
   elif [ ${COUNTRY} = macedonia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #macedonia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #macedonia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #macedonia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #macedonia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #macedonia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #macedonia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #macedonia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #macedonia
#      fi



# Madagascar extradata
   elif [ ${COUNTRY} = madagascar ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #madagascar
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #madagascar
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #madagascar
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #madagascar
#      fi
#      if [ ${INSERTION} = before_merge ]; then #madagascar
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #madagascar
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #madagascar
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #madagascar
#      fi



# Malawi extradata
   elif [ ${COUNTRY} = malawi ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #malawi
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #malawi
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #malawi
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #malawi
#      fi
#      if [ ${INSERTION} = before_merge ]; then #malawi
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #malawi
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #malawi
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #malawi
#      fi



# Malaysia extradata
   elif [ ${COUNTRY} = malaysia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #malaysia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #malaysia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #malaysia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #malaysia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #malaysia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #malaysia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #malaysia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #malaysia
#      fi



# Maldives extradata
   elif [ ${COUNTRY} = maldives ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #maldives
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #maldives
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #maldives
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #maldives
#      fi
#      if [ ${INSERTION} = before_merge ]; then #maldives
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #maldives
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #maldives
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #maldives
#      fi



# Mali extradata
   elif [ ${COUNTRY} = mali ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mali
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mali
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mali
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mali
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mali
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mali
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mali
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mali
#      fi



# Malta extradata
   elif [ ${COUNTRY} = malta ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #malta
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #malta
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #malta
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #malta
#      fi
#      if [ ${INSERTION} = before_merge ]; then #malta
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #malta
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #malta
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #malta
#      fi



# Marshall Islands extradata
   elif [ ${COUNTRY} = marshall_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #marshall_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #marshall_islands
#      fi



# Martinique extradata
   elif [ ${COUNTRY} = martinique ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #martinique
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #martinique
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #martinique
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #martinique
#      fi
#      if [ ${INSERTION} = before_merge ]; then #martinique
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #martinique
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #martinique
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #martinique
#      fi



# Mauritania extradata
   elif [ ${COUNTRY} = mauritania ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mauritania
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mauritania
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mauritania
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mauritania
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mauritania
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mauritania
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mauritania
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mauritania
#      fi



# Mauritius extradata
   elif [ ${COUNTRY} = mauritius ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mauritius
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mauritius
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mauritius
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mauritius
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mauritius
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mauritius
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mauritius
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mauritius
#      fi



# Mayotte extradata
   elif [ ${COUNTRY} = mayotte ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mayotte
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mayotte
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mayotte
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mayotte
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mayotte
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mayotte
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mayotte
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mayotte
#      fi



# Mexico extradata
   elif [ ${COUNTRY} = mexico ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mexico
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mexico
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mexico
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mexico
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mexico
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mexico
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mexico
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mexico
#      fi



# Micronesia extradata
   elif [ ${COUNTRY} = micronesia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #micronesia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #micronesia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #micronesia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #micronesia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #micronesia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #micronesia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #micronesia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #micronesia
#      fi



# Moldova extradata
   elif [ ${COUNTRY} = moldova ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #moldova
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #moldova
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #moldova
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #moldova
#      fi
#      if [ ${INSERTION} = before_merge ]; then #moldova
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #moldova
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #moldova
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #moldova
#      fi



# Monaco extradata
   elif [ ${COUNTRY} = monaco ]; then
       ok=""; #Empty statment replacer
#      fi
#      if [ ${INSERTION} = after_extitm ]; then #monaco
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #monaco
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #monaco
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #monaco
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #monaco
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #monaco
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #monaco
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #monaco
#
#      fi



# Mongolia extradata
   elif [ ${COUNTRY} = mongolia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mongolia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mongolia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mongolia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mongolia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mongolia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mongolia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mongolia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mongolia
#      fi



# Montserrat extradata
   elif [ ${COUNTRY} = montserrat ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #montserrat
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #montserrat
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #montserrat
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #montserrat
#      fi
#      if [ ${INSERTION} = before_merge ]; then #montserrat
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #montserrat
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #montserrat
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #montserrat
#      fi



# Morocco extradata
   elif [ ${COUNTRY} = morocco ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #morocco
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #morocco
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #morocco
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #morocco
#      fi
#      if [ ${INSERTION} = before_merge ]; then #morocco
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #morocco
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #morocco
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #morocco
#      fi



# Mozambique extradata
   elif [ ${COUNTRY} = mozambique ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #mozambique
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #mozambique
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #mozambique
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #mozambique
#      fi
#      if [ ${INSERTION} = before_merge ]; then #mozambique
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #mozambique
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #mozambique
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #mozambique
#      fi



# Myanmar extradata
   elif [ ${COUNTRY} = myanmar ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #myanmar
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #myanmar
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #myanmar
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #myanmar
#      fi
#      if [ ${INSERTION} = before_merge ]; then #myanmar
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #myanmar
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #myanmar
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #myanmar
#      fi



# Namibia extradata
   elif [ ${COUNTRY} = namibia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #namibia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #namibia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #namibia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #namibia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #namibia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #namibia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #namibia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #namibia
#      fi



# Nauru extradata
   elif [ ${COUNTRY} = nauru ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #nauru
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #nauru
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #nauru
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #nauru
#      fi
#      if [ ${INSERTION} = before_merge ]; then #nauru
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #nauru
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #nauru
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #nauru
#      fi



# Nepal extradata
   elif [ ${COUNTRY} = nepal ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #nepal
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #nepal
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #nepal
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #nepal
#      fi
#      if [ ${INSERTION} = before_merge ]; then #nepal
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #nepal
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #nepal
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #nepal
#      fi



# Netherlands extradata
   elif [ ${COUNTRY} = netherlands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #netherlands
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #netherlands
#        
#      fi # end after_intconn
#      if [ ${INSERTION} = after_extconn ]; then #netherlands
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #netherlands
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #netherlands
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #netherlands
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #netherlands
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #netherlands
#
#      fi



# Netherlands Antilles extradata
   elif [ ${COUNTRY} = netherlands_antilles ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = before_merge ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #netherlands_antilles
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #netherlands_antilles
#      fi



# New Caledonia extradata
   elif [ ${COUNTRY} = new_caledonia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #new_caledonia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #new_caledonia
#      fi



# New Zealand extradata
   elif [ ${COUNTRY} = new_zealand ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = before_merge ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #new_zealand
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #new_zealand
#      fi



# Nicaragua extradata
   elif [ ${COUNTRY} = nicaragua ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = before_merge ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #nicaragua
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #nicaragua
#      fi



# Niger extradata
   elif [ ${COUNTRY} = niger ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #niger
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #niger
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #niger
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #niger
#      fi
#      if [ ${INSERTION} = before_merge ]; then #niger
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #niger
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #niger
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #niger
#      fi



# Nigeria extradata
   elif [ ${COUNTRY} = nigeria ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #nigeria
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #nigeria
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #nigeria
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #nigeria
#      fi
#      if [ ${INSERTION} = before_merge ]; then #nigeria
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #nigeria
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #nigeria
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #nigeria
#      fi



# Niue extradata
   elif [ ${COUNTRY} = niue ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #niue
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #niue
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #niue
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #niue
#      fi
#      if [ ${INSERTION} = before_merge ]; then #niue
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #niue
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #niue
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #niue
#      fi



# Northern Mariana Islands extradata
   elif [ ${COUNTRY} = northern_mariana_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #northern_mariana_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #northern_mariana_islands
#      fi



# North Korea extradata
   elif [ ${COUNTRY} = north_korea ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #north_korea
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #north_korea
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #north_korea
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #north_korea
#      fi
#      if [ ${INSERTION} = before_merge ]; then #north_korea
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #north_korea
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #north_korea
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #north_korea
#      fi




# Norway extradata
   elif [ ${COUNTRY} = norway ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #norway
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #norway
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #norway
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #norway
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #norway
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #norway
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #norway
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #norway
#
#      fi
#   fi



# Occupied Palestinian Territory extradata
   elif [ ${COUNTRY} = occupied_palestinian_territory ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = before_merge ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #occupied_palestinian_territory
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #occupied_palestinian_territory
#      fi



# Oman extradata
   elif [ ${COUNTRY} = oman ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #oman
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #oman
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #oman
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #oman
#      fi
#      if [ ${INSERTION} = before_merge ]; then #oman
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #oman
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #oman
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #oman
#      fi



# Pakistan extradata
   elif [ ${COUNTRY} = pakistan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #pakistan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #pakistan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #pakistan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #pakistan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #pakistan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #pakistan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #pakistan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #pakistan
#      fi



# Palau extradata
   elif [ ${COUNTRY} = palau ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #palau
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #palau
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #palau
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #palau
#      fi
#      if [ ${INSERTION} = before_merge ]; then #palau
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #palau
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #palau
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #palau
#      fi



# Panama extradata
   elif [ ${COUNTRY} = panama ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #panama
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #panama
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #panama
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #panama
#      fi
#      if [ ${INSERTION} = before_merge ]; then #panama
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #panama
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #panama
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #panama
#      fi



# Papua New Guinea extradata
   elif [ ${COUNTRY} = papua_new_guinea ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = before_merge ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #papua_new_guinea
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #papua_new_guinea
#      fi



# Paraguay extradata
   elif [ ${COUNTRY} = paraguay ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #paraguay
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #paraguay
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #paraguay
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #paraguay
#      fi
#      if [ ${INSERTION} = before_merge ]; then #paraguay
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #paraguay
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #paraguay
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #paraguay
#      fi



# Peru extradata
   elif [ ${COUNTRY} = peru ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #peru
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #peru
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #peru
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #peru
#      fi
#      if [ ${INSERTION} = before_merge ]; then #peru
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #peru
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #peru
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #peru
#      fi



# Philippines extradata
   elif [ ${COUNTRY} = philippines ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #philippines
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #philippines
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #philippines
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #philippines
#      fi
#      if [ ${INSERTION} = before_merge ]; then #philippines
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #philippines
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #philippines
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #philippines
#      fi



# Pitcairn extradata
   elif [ ${COUNTRY} = pitcairn ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = before_merge ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #pitcairn
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #pitcairn
#      fi



# Poland extradata
   elif [ ${COUNTRY} = poland ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #poland
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #poland
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #poland
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #poland
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #poland
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #poland
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #poland
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #poland
#         #NB! These extradata have no effect on merged maps.
#
#     fi


# Portugal extradata.
   elif [ ${COUNTRY} = portugal ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #portugal
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #portugal
#      
#      fi # end after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #portugal
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #portugal
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #portugal
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #portugal
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #portugal
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #portugal
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# Qatar extradata
   elif [ ${COUNTRY} = qatar ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #qatar
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #qatar
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #qatar
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #qatar
#      fi
#      if [ ${INSERTION} = before_merge ]; then #qatar
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #qatar
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #qatar
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #qatar
#      fi



# Reunion extradata
   elif [ ${COUNTRY} = reunion ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #reunion
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #reunion
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #reunion
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #reunion
#      fi
#      if [ ${INSERTION} = before_merge ]; then #reunion
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #reunion
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #reunion
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #reunion
#      fi



# Romania extradata
   elif [ ${COUNTRY} = romania ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #romania
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #romania
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #romania
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #romania
#      fi
#      if [ ${INSERTION} = before_merge ]; then #romania
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #romania
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #romania
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #romania
#      fi



# Russia extradata
   elif [ ${COUNTRY} = russia ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #russia
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #russia
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #russia
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #russia
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #russia
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #russia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #russia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #russia
#         #NB! These extradata have no effect on merged maps.
#
#      fi


# Rwanda extradata
   elif [ ${COUNTRY} = rwanda ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #rwanda
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #rwanda
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #rwanda
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #rwanda
#      fi
#      if [ ${INSERTION} = before_merge ]; then #rwanda
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #rwanda
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #rwanda
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #rwanda
#      fi



# Saint Helena extradata
   elif [ ${COUNTRY} = saint_helena ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = before_merge ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #saint_helena
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #saint_helena
#      fi



# Saint Kitts and Nevis extradata
   elif [ ${COUNTRY} = saint_kitts_and_nevis ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = before_merge ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #saint_kitts_and_nevis
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #saint_kitts_and_nevis
#      fi



# Saint Lucia extradata
   elif [ ${COUNTRY} = saint_lucia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #saint_lucia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #saint_lucia
#      fi



# Saint Pierre and Miquelon extradata
   elif [ ${COUNTRY} = saint_pierre_and_miquelon ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = before_merge ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #saint_pierre_and_miquelon
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #saint_pierre_and_miquelon
#      fi



# Saint Vincent and the Grenadines extradata
   elif [ ${COUNTRY} = saint_vincent_and_the_grenadines ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = before_merge ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #saint_vincent_and_the_grenadines
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #saint_vincent_and_the_grenadines
#      fi



# Samoa extradata
   elif [ ${COUNTRY} = samoa ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #samoa
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #samoa
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #samoa
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #samoa
#      fi
#      if [ ${INSERTION} = before_merge ]; then #samoa
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #samoa
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #samoa
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #samoa
#      fi



# Sao Tome and Principe extradata
   elif [ ${COUNTRY} = sao_tome_and_principe ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = before_merge ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #sao_tome_and_principe
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #sao_tome_and_principe
#      fi



# Saudi Arabia extradata
   elif [ ${COUNTRY} = saudi_arabia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #saudi_arabia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #saudi_arabia
#      fi



# Senegal extradata
   elif [ ${COUNTRY} = senegal ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #senegal
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #senegal
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #senegal
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #senegal
#      fi
#      if [ ${INSERTION} = before_merge ]; then #senegal
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #senegal
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #senegal
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #senegal
#      fi



# Serbia and Montenegro extradata
   elif [ ${COUNTRY} = serbia_montenegro ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = before_merge ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #serbia_montenegro
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #serbia_montenegro
#      fi



# Seychelles extradata
   elif [ ${COUNTRY} = seychelles ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #seychelles
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #seychelles
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #seychelles
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #seychelles
#      fi
#      if [ ${INSERTION} = before_merge ]; then #seychelles
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #seychelles
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #seychelles
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #seychelles
#      fi



# Sierra Leone extradata
   elif [ ${COUNTRY} = sierra_leone ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = before_merge ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #sierra_leone
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #sierra_leone
#      fi



# Singapore extradata
   elif [ ${COUNTRY} = singapore ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #singapore
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #singapore
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #singapore
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #singapore
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #singapore
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #singapore
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #singapore
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #singapore
#
#      fi



# Slovakia extradata
   elif [ ${COUNTRY} = slovakia ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #slovakia
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #slovakia
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #slovakia
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #slovakia
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #slovakia
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #slovakia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #slovakia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #slovakia
#         #NB! These extradata have no effect on merged maps.
#
#      fi


# Slovenia extradata
   elif [ ${COUNTRY} = slovenia ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #slovenia
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #slovenia
#       
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #slovenia
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #slovenia
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #slovenia
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #slovenia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #slovenia
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #slovenia
#         #NB! These extradata have no effect on merged maps.
#
#      fi




# Solomon Islands extradata
   elif [ ${COUNTRY} = solomon_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #solomon_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #solomon_islands
#      fi



# Somalia extradata
   elif [ ${COUNTRY} = somalia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #somalia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #somalia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #somalia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #somalia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #somalia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #somalia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #somalia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #somalia
#      fi



# South Africa extradata
   elif [ ${COUNTRY} = south_africa ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #south_africa
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #south_africa
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #south_africa
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #south_africa
#      fi
#      if [ ${INSERTION} = before_merge ]; then #south_africa
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #south_africa
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #south_africa
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #south_africa
#      fi



# South Korea extradata
   elif [ ${COUNTRY} = south_korea ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #south_korea
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #south_korea
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #south_korea
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #south_korea
#      fi
#      if [ ${INSERTION} = before_merge ]; then #south_korea
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #south_korea
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #south_korea
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #south_korea
#      fi



# Spain extradata
   elif [ ${COUNTRY} = spain ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #spain
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #spain
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #spain
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #spain
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #xxx
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #xxx
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #spain
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #spain
#
#      fi



# Sri Lanka extradata
   elif [ ${COUNTRY} = sri_lanka ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = before_merge ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #sri_lanka
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #sri_lanka
#      fi



# Sudan extradata
   elif [ ${COUNTRY} = sudan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #sudan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #sudan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #sudan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #sudan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #sudan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #sudan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #sudan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #sudan
#      fi



# Suriname extradata
   elif [ ${COUNTRY} = suriname ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #suriname
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #suriname
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #suriname
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #suriname
#      fi
#      if [ ${INSERTION} = before_merge ]; then #suriname
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #suriname
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #suriname
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #suriname
#      fi



# Svalbard and Jan Mayen extradata
   elif [ ${COUNTRY} = svalbard_and_jan_mayen ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = before_merge ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #svalbard_and_jan_mayen
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #svalbard_and_jan_mayen
#      fi



# Swaziland extradata
   elif [ ${COUNTRY} = swaziland ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #swaziland
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #swaziland
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #swaziland
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #swaziland
#      fi
#      if [ ${INSERTION} = before_merge ]; then #swaziland
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #swaziland
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #swaziland
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #swaziland
#      fi



# Sweden extradata
   elif [ ${COUNTRY} = sweden ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #sweden
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #sweden
#
#      fi    # end after_intconn
#      if [ ${INSERTION} = after_trngen ]; then  #sweden
#
#      fi    # end after_trngen
#      if [ ${INSERTION} = after_setloc ]; then #sweden
#
#      fi    # end after_setloc
#      if [ ${INSERTION} = before_merge ]; then #sweden
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #sweden
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #sweden
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #sweden
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# Switzerland extradata
   elif [ ${COUNTRY} = switzerland ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #switzerland
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #switzerland
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #switzerland
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #switzerland
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #switzerland
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #switzerland
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #switzerland
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #switzerland
#
#      fi



# Syria extradata
   elif [ ${COUNTRY} = syria ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #syria
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #syria
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #syria
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #syria
#      fi
#      if [ ${INSERTION} = before_merge ]; then #syria
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #syria
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #syria
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #syria
#      fi



# Taiwan Province of China extradata
   elif [ ${COUNTRY} = taiwan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #taiwan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #taiwan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #taiwan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #taiwan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #taiwan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #taiwan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #taiwan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #taiwan
#      fi



# Tajikistan extradata
   elif [ ${COUNTRY} = tajikistan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #tajikistan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #tajikistan
#      fi



# United Republic of Tanzania extradata
   elif [ ${COUNTRY} = tanzania ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #tanzania
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #tanzania
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #tanzania
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #tanzania
#      fi
#      if [ ${INSERTION} = before_merge ]; then #tanzania
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #tanzania
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #tanzania
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #tanzania
#      fi



# Thailand extradata
   elif [ ${COUNTRY} = thailand ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #thailand
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #thailand
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #thailand
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #thailand
#      fi
#      if [ ${INSERTION} = before_merge ]; then #thailand
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #thailand
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #thailand
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #thailand
#      fi



# Timor-Leste extradata
   elif [ ${COUNTRY} = timor_leste ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = before_merge ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #timor_leste
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #timor_leste
#      fi



# Togo extradata
   elif [ ${COUNTRY} = togo ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #togo
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #togo
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #togo
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #togo
#      fi
#      if [ ${INSERTION} = before_merge ]; then #togo
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #togo
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #togo
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #togo
#      fi



# Tokelau extradata
   elif [ ${COUNTRY} = tokelau ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #tokelau
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #tokelau
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #tokelau
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #tokelau
#      fi
#      if [ ${INSERTION} = before_merge ]; then #tokelau
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #tokelau
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #tokelau
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #tokelau
#      fi



# Tonga extradata
   elif [ ${COUNTRY} = tonga ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #tonga
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #tonga
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #tonga
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #tonga
#      fi
#      if [ ${INSERTION} = before_merge ]; then #tonga
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #tonga
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #tonga
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #tonga
#      fi



# Trinidad and Tobago extradata
   elif [ ${COUNTRY} = trinidad_and_tobago ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = before_merge ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #trinidad_and_tobago
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #trinidad_and_tobago
#      fi



# Tunisia extradata
   elif [ ${COUNTRY} = tunisia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #tunisia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #tunisia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #tunisia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #tunisia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #tunisia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #tunisia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #tunisia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #tunisia
#      fi



# Turkey extradata
   elif [ ${COUNTRY} = turkey ]; then
      ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #turkey
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #turkey
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #turkey
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #turkey
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #turkey
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #turkey
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #turkey
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #turkey
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# Turkmenistan extradata
   elif [ ${COUNTRY} = turkmenistan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #turkmenistan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #turkmenistan
#      fi



# Turks and Caicos Islands extradata
   elif [ ${COUNTRY} = turks_and_caicos_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #turks_and_caicos_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #turks_and_caicos_islands
#      fi



# Tuvalu extradata
   elif [ ${COUNTRY} = tuvalu ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = before_merge ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #tuvalu
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #tuvalu
#      fi



# UK extradata
   elif [ ${COUNTRY} = england -o ${COUNTRY} = uk ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #uk
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #uk
#    
#      fi # end after_intconn
#      if [ ${INSERTION} = after_trngen ]; then #uk
#      
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #uk
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #uk
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #uk
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #uk
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #uk
#
#      fi




# UAE extradata
   elif [ ${COUNTRY} = uae ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #uae
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #uae
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #uae
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #uae
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #uae
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #uae
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #uae
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #uae
#
#      fi




# USA extradata
   elif [ ${COUNTRY} = usa ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #usa
#
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #usa
#
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #usa
#
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #usa
#
#      fi
#      if [ ${INSERTION} = before_merge ]; then #usa
#
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #usa
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #usa
#         #NB! These extradata have no effect on merged maps.
#
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #usa
#         #NB! These extradata have no effect on merged maps.
#
#      fi



# Uganda extradata
   elif [ ${COUNTRY} = uganda ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #uganda
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #uganda
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #uganda
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #uganda
#      fi
#      if [ ${INSERTION} = before_merge ]; then #uganda
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #uganda
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #uganda
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #uganda
#      fi



# Ukraine extradata
   elif [ ${COUNTRY} = ukraine ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #ukraine
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #ukraine
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #ukraine
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #ukraine
#      fi
#      if [ ${INSERTION} = before_merge ]; then #ukraine
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #ukraine
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #ukraine
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #ukraine
#      fi



# United States Minor Outlying Islands extradata
   elif [ ${COUNTRY} = united_states_minor_outlying_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #united_states_minor_outlying_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #united_states_minor_outlying_islands
#      fi



# United States Virgin Islands extradata
   elif [ ${COUNTRY} = united_states_virgin_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #united_states_virgin_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #united_states_virgin_islands
#      fi



# Uruguay extradata
   elif [ ${COUNTRY} = uruguay ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #uruguay
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #uruguay
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #uruguay
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #uruguay
#      fi
#      if [ ${INSERTION} = before_merge ]; then #uruguay
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #uruguay
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #uruguay
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #uruguay
#      fi



# Uzbekistan extradata
   elif [ ${COUNTRY} = uzbekistan ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = before_merge ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #uzbekistan
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #uzbekistan
#      fi



# Vanuatu extradata
   elif [ ${COUNTRY} = vanuatu ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = before_merge ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #vanuatu
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #vanuatu
#      fi



# Venezuela extradata
   elif [ ${COUNTRY} = venezuela ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #venezuela
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #venezuela
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #venezuela
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #venezuela
#      fi
#      if [ ${INSERTION} = before_merge ]; then #venezuela
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #venezuela
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #venezuela
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #venezuela
#      fi



# Viet Nam extradata
   elif [ ${COUNTRY} = vietnam ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #vietnam
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #vietnam
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #vietnam
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #vietnam
#      fi
#      if [ ${INSERTION} = before_merge ]; then #vietnam
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #vietnam
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #vietnam
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #vietnam
#      fi



# Wallis and Futuna Islands extradata
   elif [ ${COUNTRY} = wallis_and_futuna_islands ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = before_merge ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #wallis_and_futuna_islands
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #wallis_and_futuna_islands
#      fi



# Western Sahara extradata
   elif [ ${COUNTRY} = western_sahara ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = before_merge ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #western_sahara
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #western_sahara
#      fi



# Yemen extradata
   elif [ ${COUNTRY} = yemen ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #yemen
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #yemen
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #yemen
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #yemen
#      fi
#      if [ ${INSERTION} = before_merge ]; then #yemen
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #yemen
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #yemen
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #yemen
#      fi



# Zambia extradata
   elif [ ${COUNTRY} = zambia ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #zambia
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #zambia
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #zambia
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #zambia
#      fi
#      if [ ${INSERTION} = before_merge ]; then #zambia
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #zambia
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #zambia
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #zambia
#      fi



# Zimbabwe extradata
   elif [ ${COUNTRY} = zimbabwe ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = before_merge ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #zimbabwe
#      fi


# San Marino extradata
   elif [ ${COUNTRY} = san_marino ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = before_merge ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #zimbabwe
#      fi


# Zimbabwe extradata
   elif [ ${COUNTRY} = san_marino ]; then
       ok=""; #Empty statment replacer
#      if [ ${INSERTION} = after_extitm ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_intconn ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_trngen ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_setloc ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = before_merge ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_extconn ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_ovrmaps ]; then #zimbabwe
#      fi
#      if [ ${INSERTION} = after_addreg ]; then #zimbabwe
#      fi



   else
       echo "No extra data for country: $COUNTRY"
       echo "exits"
       exit 33;

   fi
   echo `date +"%Y%m%d %T"`" Done! Extradata $COUNTRY $INSERTION.";


} #function extradata


function stitch()
{
    typeset allBorders="$1";
    typeset SSHCMD="$2"; # may be blank
    typeset EXTRADATAPATH="${BASEGENFILESPATH}/extradata/stitching";
    
    tmpDir="${TEMPLOGFILEPATH}";
    if [ "$tmpDir" = "." ]; then
      tmpDir="`pwd`";
      echo "Setting tmpDir to pwd = $tmpDir"
    fi
    if [ ! -d "$tmpDir" ]; then
        echo "stitch tmpDir not reachable:"
        echo "$tmpDir";
        exit 1;
    fi
    
# Before mid/mif extra data, such as remove item.
    echo `date +"%Y%m%d %T"`" Stitch: extradata A";
    tmpEdAFile=`mktemp $tmpDir/stitch.edA.ext.XXXXXXXXX`;
    rm ${tmpEdAFile} >& /dev/null
    for line in $allBorders; do
        typeset STITCHFILEBASE=`echo $line`

        EXTRA_A="${EXTRADATAPATH}/${STITCHFILEBASE}.A.ext";
        if [ -e ${EXTRA_A} ]; then
            ${SSHCMD} cat ${EXTRA_A} >> ${tmpEdAFile};
        fi
    done
    ${checkFunction}
    if [ -e "${tmpEdAFile}" ]; then
       dos2unix  ${tmpEdAFile} >& /dev/null
       chmod g+rw ${tmpEdAFile} >& /dev/null
       ${SSHCMD} nice time ./GenerateMapServer -x ${tmpEdAFile} >& $LOGPATH/stc_extradata.A.log
       ${checkFunction}
       ${SSHCMD} mv ${tmpEdAFile} .
    fi
    

# Road geometry in mid/mif .
    echo `date +"%Y%m%d %T"`" Stitch: mid/mif streetSegmentItems";
    tmpFileBase=`mktemp $tmpDir/stitch.streetSegmentItems.XXXXXXXXX`;
    tmpFileMid="${tmpFileBase}.mid";
    rm ${tmpFileMid} >& /dev/null
    tmpFileMif="${tmpFileBase}.mif";
    rm ${tmpFileMif} >& /dev/null
    for line in $allBorders; do
        typeset STITCHFILEBASE="$line"
        
        midfile="${EXTRADATAPATH}/${STITCHFILEBASE}.streetSegmentItems.mid"
        miffile="${EXTRADATAPATH}/${STITCHFILEBASE}.streetSegmentItems.mif"
        if [ -e "${midfile}" ]; then
            if [ -e "${miffile}" ]; then
                ${SSHCMD} cat ${midfile} >> ${tmpFileMid};
                ${SSHCMD} cat ${miffile} >> ${tmpFileMif};
            else
                echo "Mismatched mid/mif files, exits".
                echo "File: ${EXTRADATAPATH}/${STITCHFILEBASE}";
                exit 1;
            fi
        fi  
    done
    ${checkFunction}
    if [ -e "${tmpFileMid}" ]; then
        ${SSHCMD} nice time ./GenerateMapServer --useCoordToFindCorrectMap -t -r ${tmpFileBase} >& $LOGPATH/stc_SSI.midmif.log
        ${checkFunction}
        ${SSHCMD} mv ${tmpFileBase}.mi* .
    fi


# Ferry geometry in mid/mif .
    echo `date +"%Y%m%d %T"`" Stitch: mid/mif ferryItems";
    tmpFileBase=`mktemp $tmpDir/stitch.ferryItems.XXXXXXXXX`;
    tmpFileMid="${tmpFileBase}.mid";
    rm ${tmpFileMid} >& /dev/null
    tmpFileMif="${tmpFileBase}.mif";
    rm ${tmpFileMif} >& /dev/null
    for line in $allBorders; do
        typeset STITCHFILEBASE="$line"
        
        midfile="${EXTRADATAPATH}/${STITCHFILEBASE}.ferryItems.mid"
        miffile="${EXTRADATAPATH}/${STITCHFILEBASE}.ferryItems.mif"
        if [ -e "${midfile}" ]; then
            if [ -e "${miffile}" ]; then
                ${SSHCMD} cat ${midfile} >> ${tmpFileMid};
                ${SSHCMD} cat ${miffile} >> ${tmpFileMif};
            else
                echo "Mismatched mid/mif files, exits".
                echo "File: ${EXTRADATAPATH}/${STITCHFILEBASE}";
                exit 1;
            fi
        fi  
    done
    ${checkFunction}
    if [ -e "${tmpFileMid}" ]; then
        ${SSHCMD} nice time ./GenerateMapServer --useCoordToFindCorrectMap -t -r ${tmpFileBase} >& $LOGPATH/stc_ferry.midmif.log
        ${checkFunction}
        ${SSHCMD} mv ${tmpFileBase}.mi* .
    fi


# After mid/mif extra data, such as set turn description and name.
# (only for the turn desc not already handled in midmif addition)
    echo `date +"%Y%m%d %T"`" Stitch: extradata B";
    tmpEdBFile=`mktemp $tmpDir/stitch.edB.ext.XXXXXXXXX`;
    rm ${tmpEdBFile} >& /dev/null
    for line in $allBorders; do
        typeset STITCHFILEBASE=`echo $line`
        
        EXTRA_B="${EXTRADATAPATH}/${STITCHFILEBASE}.B.ext";
        if [ -e ${EXTRA_B} ]; then
            ${SSHCMD} cat ${EXTRA_B} >> ${tmpEdBFile};
        fi
    done
    ${checkFunction}
    if [ -e "${tmpEdBFile}" ]; then
       dos2unix  ${tmpEdBFile} >& /dev/null
       chmod g+rw ${tmpEdBFile} >& /dev/null
       ${SSHCMD} nice time ./GenerateMapServer -x ${tmpEdBFile} >& $LOGPATH/stc_extradata.B.log
       ${checkFunction}
       ${SSHCMD} mv ${tmpEdBFile} .
    fi

} #function stitching


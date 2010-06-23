/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTRADATAREADER_H
#define EXTRADATAREADER_H

#include "config.h"
#include "ItemTypes.h"
#include "OldItem.h"
#include <map>
#include "CoordinateTransformer.h"
#include <vector>
#include "GMSMap.h"
#include "STLStrComp.h"
#include "OldExtraDataUtility.h"


class OldNode;
class OldConnection;


/**
  *   Objects of this class will read an asciifile with extra data 
  *   (map correction records) that will be inserted into the mcm map
  *   after it is created from the map supplier data
  *   but before it is added to the running mc2-system.
  *   
  */
class ExtraDataReader {
   public:
      /**
        *   Constructor that intialize the members in this object. The
        *   actual parsing is done by the parseInto-method.
        *
        *   In order to get string matching fast, this constructor is
        *   a bit expensive. Hence, try to reuse!
        *   
        *   @param   filePath The location of the file containing the extra
        *                     data.
        */
      ExtraDataReader(const char* filePath = NULL);

      /**
        *   Deletes this objects and returns the used memory to the 
        *   operating system.
        */
      virtual ~ExtraDataReader();

      /**
        *   Set new filename.
        *   @param   filePath The location of the file containing the extra
        *                     data.
        */
      void setFilePath(const char* filePath);

      /**
        *   Parse into the given map.
        *   @param theMap     The map into which parsed data should be 
        *                     written.
        *   @param parseOnly  Set this parameter to true if the extrafile
        *                     onlys should be parsed (nothing will be
        *                     added to the map).
        *   @return  True upon success, false otherwise.
        */
      bool parseInto(GMSMap* theMap, bool parseOnly=false);

      /**
       *    Initiate the check table when checking extra data. Pre-reads 
       *    the extra data file and stores information about the records 
       *    in the table based on extra data record id.
       *    The table is further filled with info when running the 
       *    parseInto-method with the m_checkRecords variable set to true, 
       *    storing e.g. whether a record is found in the map or not, 
       *    still needed or not, etc.
       *
       *    The extra data types that can be checked includes:
       *    <ol>
       *       <li> ADD_NAME_TO_ITEM
       *       <li> ADD_SIGNPOST
       *       <li> REMOVE_ITEM
       *       <li> REMOVE_NAME_FROM_ITEM
       *       <li> REMOVE_POIS
       *       <li> REMOVE_SIGNPOST
       *       <li> SET_CONTROLLEDACCESS
       *       <li> SET_ENTRYRESTRICTIONS
       *       <li> SET_HOUSENUMBER
       *       <li> SET_LEVEL
       *       <li> SET_MULTIDIGITISED
       *       <li> SET_RAMP
       *       <li> SET_ROUNDABOUT
       *       <li> SET_ROUNDABOUTISH
       *       <li> SET_SPEEDLIMIT
       *       <li> SET_TOLLROAD
       *       <li> SET_TURNDIRECTION
       *       <li> SET_VEHICLERESTRICTIONS
       *       <li> SET_WATER_TYPE
       *       <li> UPDATE_NAME
       *    </ol>
       */
      bool initCheckTable();

      /**
       *    Print information from the extra data check table after running 
       *    the parseInto-method, resulting in different outfiles with:
       *    <ol>
       *       <li> extra data records that are not checked.
       *       <li> extra data records that are found in the map and 
       *            still needed for correction.
       *       <li> extra data records that are found in the map but 
       *            not needed for correction.
       *       <li> extra data records that are found in the map and
       *            where the attribute value does not match with 
       *            the original value given in the comment. Duplicates
       *            to these records exist in the other two found-files.
       *       <li> extra data records that are not found in any of the 
       *            looped maps. The main reason for the record to be 
       *            missing is a change in geometry. However for theese
       *            records a candidate map id is most likely found.
       *    </ol>
       *
       */
      bool printCheckTable();

      /**
       *    Set the member variable m_applyOnMergedMaps true. Then 
       *    when parsing extra data all items affected are stored in
       *    the affect-vectors for applying the changes to overview
       *    and country overview maps.
       */
      void initMergedMaps();

      /**
       *    Get the ids of all items that were affected when adding extra
       *    data from one extra data file to merged maps.
       */
      inline multimap<uint32, uint32> getItemsAffectedInUVMaps();

      /**
       *    Get the number of extradata records that have beed added/applied
       *    to the currently loaded mcm map.
       */
      inline uint32 getNbrRecordsAdded();

      /**
       *    Get the number of extradata records that exist in the parsed file.
       *    It only makes sense calling this function AFTER parsing the file,
       *    since it is the m_curLineNumber member that is returned.
       */
      inline uint32 getNbrRecordsInEDFile();

      /**
       *    Get the line numbers of the extradata records that have
       *    been added to the looped maps.
       */
      inline vector<int> getRecordsAddedToMaps();

   private:

      typedef CoordinateTransformer::format_t format_t;

      /**
        *   File path.
        */
      const char* m_path;
 
      /**
        *   Dictionary for item types.
        *   Is filled with the english names of all item types from the 
        *   MC2 stringtable and some pre-defined abbreviations etc for 
        *   some item types, defined in ExtraDataReaderTables.
        */
      map<MC2String, ItemTypes::itemType, strNoCaseCompareLess> m_itemTypes;

      /**
        *   Dictionary for name types.
        */
      map<MC2String, ItemTypes::name_t, strNoCaseCompareLess> m_nameTypes;

      /**
        *   Dictionary for languages.
        */
      map<MC2String, LangTypes::language_t, strNoCaseCompareLess> m_languages;
      
      /**
        *   Dictionary for point of interest types.
        */
      map<MC2String, ItemTypes::pointOfInterest_t, strNoCaseCompareLess> 
         m_POITypes;

      /**
        *   Number of times to generate random coordinates and check if
        *   they are inside a BuiltUpArea-item in the ADD_CITYPART-function.
        */
      static const uint32 NBRS_OF_CHECK_COORD_INSIDE_BUA;

      /**
       *    Set the turn-direction of the connection that leads between 
       *    the given nodes.
       *    @param fromNode   The node where the connection starts.
       *    @param toNode     The node where the connection ends.
       *    @param turnDir    The new value of the turn-direction.
       *    @param checkRecord If true the existing turn dir value is checked
       *                       against the record, and no update is made.
       *    @return A negative value upon error (e.g. conn not found), and
       *            a positive value if the turn dir is updated.
       *            If the checkRecord is set, positive if the record 
       *            is still usefull and 0 if the map is already correct.
       */
      int setTurndirection(OldNode* fromNode, OldNode* toNode,
                           ItemTypes::turndirection_t turnDir,
                           bool checkRecord);

      /**
        *   Get the type of the item described with a text. The known
        *   strings are (not case sensitive):
        *   \begin{tabular}{l|l}
        *   {\bf OldItem type}      & {\bf Known strings} \\ \hline
        *   StreetSegmentItem    & SSI \\
        *   MunicipalItem         & MUNICIPAL, MUN, CAI \\
        *   WaterItem            & WI \\
        *   ParkItem             & PI \\
        *   ForestItem           & FI \\
        *   BuildingItem         & BI \\
        *   RailwayItem          & RI \\
        *   StreetItem           & SI \\
        *   BuiltUpAreaItem      & BUA \\
        *   PointOfInterestItem  & POI \\
        *   \end{tabular}
        */
      ItemTypes::itemType getItemType(MC2String& str) const;
        
      /**
        *   Get the type of the name described with a text. The known
        *   strings are (not case sensitive):
        *   \begin{tabular}{l|l}
        *   {\bf Name type}      & {\bf Known strings} \\ \hline
        *   OfficialName         & ON \\
        *   AlternativeName      & AN \\
        *   RoadNumber           & RN \\
        *   AbbreviationName     & ABB, ABBREVIATION \\
        *   \end{tabular}
        */
      ItemTypes::name_t getNameType(MC2String& str) const;

      LangTypes::language_t getLanguage(MC2String& str) const;


      /**
        *   Get the type of the name described with a text. The known
        *   strings are (not case sensitive):
        *   \begin{tabular}{l|l}
        *   {\bf Name type}      & {\bf Known strings} \\ \hline
        *   Airport              & AIRPORT \\
        *   BusStation           & BUSSTATION \\
        *   Cinema               & CINEMA \\
        *   Company              & COMPANY \\
        *   FerryTerminal        & FERRYTERMINAL \\
        *   GolfCourse           & GOLFCOURSE \\
        *   Hospital             & HOSPITAL
        *   OpenParkingArea      & OPENPARKINGGARAGE \\
        *   ParkAndRide          & PARKANDRIDE \\
        *   ParkingGarage        & PARKINGGARAGE \\
        *   PoliceStation        & POLICESTATION \\
        *   PublicSportAirport   & PUBLICSPORTAIRPORT \\
        *   RailwayStation       & RAILWAYSTATION \\
        *   Theatre              & THEATRE \\
        *   TouristOffice        & TOURISTOFFICE \\
        *   \end{tabular}
        */
      ItemTypes::pointOfInterest_t getPOIType(MC2String& s) const;

      /**
        *    Converts the string to an unsigned integer.
        */
      uint32 getUint32(MC2String& str) const;
      
      /**
        *    Converts the string to an integer.
        *    @return    If succesful the number, otherwise MAX_INT32.
        */
      int getNumber(MC2String& str) const;

      /**
       *    Get a string from a boolean for output.
       *    @return  A string "true" or "false".
       */
      inline const char* getStringFromBool(bool boolVal) const;
      
      /**
       *    Get the connection from fromNode to toNode.
       *    @param fromNode The node that the connection leads from.
       *    @param toNode   The node that the connection leads to.
       *    @return The connection between fromNode and toNode. NULL will
       *            be returned if no such connection or if either of the
       *            nodes are NULL.
       */
      OldConnection* getConnection(OldNode* fromNode, OldNode* toNode);

      /**
        *   Lookup an index in the stringtable.
        *   @param   str   The string to look for
        *   @return  Index of str in the string table.
        */
      uint32 lookupStringIndex(GMSMap* theMap,
                               const MC2String& str) const;

      /**
       *    @name Handle the records.
       *    The methods that are called to handle the different recordtypes.
       *    Almost all methods have the exact same parameters.
       *    @param recStr     A vector with the tokens of this record 
       *                      in strings.
       *    @param parseOnly  If this parameter is set, only the syntax 
       *                      of the record will be checked (nothing will 
       *                      be changed on the map).
       *    @return True if the syntax of the record is correct 
       *            (parseOnly == true) or if the item is added to the
       *            map (parseOnly == false). False will be returned if
       *            the syntax of the record is wrong (parseOnly == true),
       *            or if the item not was added to the current map
       *            (parseOnly == false).
       */
      //@{
         /**
          *    Handle one COMMENT-record. When checking extra data, the 
          *    extra data record id and original value are extracted and
          *    saved in membervariables. When applying dynamic extra data,
          *    the extra data insert type and comment date-and-time are 
          *    likewise extracted and saved.
          *    Record defintion:
          *    <ol>
          *       <li> # Date and Time
          *       <li> Writer
          *       <li> Source
          *       <li> Comment
          *       <li> Original value
          *       <li> Ref id (map supplier map error ref id)
          *       <li> Extra data record id
          *       <li> Extra data insert type
          *       <li> OldMap supplier and release
          *       <li> Extra data group id (if any)
          *       <li> Country
          *       <li> OldMap id
          *    </ol>
          */
         bool handleCommentRecord(vector<MC2String>& recStr, bool parseOnly);
         
         /**
          *    Handle one UPDATE_NAME-record. Record defintion:
          *    <ol>
          *       <li> OldItemType
          *       <li> coorinateType
          *       <li> coordinate
          *       <li> oldName
          *       <li> oldNameType
          *       <li> oldNameLang
          *       <li> newName
          *    </ol>
          */
         bool handleUpdateNameRecord(vector<MC2String>& recStr,
                                     bool parseOnly);

         /**
          *    Handle one REMOVE_NAME_FROM_ITEM-record. Record defintion:
          *    <ol>
          *       <li> OldItemType
          *       <li> coordinateType
          *       <li> coordinate
          *       <li> name
          *       <li> nameType
          *       <li> nameLang
          *    </ol>
          */
         bool handleRemoveNameFromItemRecord(vector<MC2String>& recStr, 
                                             bool parseOnly);

         /**
          *    Handle one ADD_NAME_TO_ITEM-record. Record defintion:
          *    <ol>
          *       <li> itemType
          *       <li> coordinate type
          *       <li> Coordinate
          *       <li> old name
          *       <li> old name type (currently not used)
          *       <li> old name language (currently not used)
          *       <li> new name
          *       <li> new name type
          *       <li> new name language
          *    </ol>
          */
         bool handleAddNameToItemRecord(vector<MC2String>& rec,
                                        bool parseOnly);

         /**
          *    Handle one ADD_CITYPART-record. Record defintion:
          *    <ol>
          *       <li> mc2-coord
          *       <li> name type
          *       <li> language
          *       <li> name
          *       <li> number of coordinates
          *       <li> coordinates
          *    </ol>
          */
         bool handleAddCitypartRecord(vector<MC2String>& rec, bool parseOnly);

         /**
          *    Handle one SET_VEHICLERESTRICTIONS-record. Record defintion:
          *    <ol>
          *       <li> First coordinate of fromNode.
          *       <li> Second coordinate of fromNode.
          *       <li> First coordinate of toNode.
          *       <li> Second coordinate of toNode.
          *       <li> Vehicle restrictions when following connection
          *            from <tt>fromNode</tt> to <tt>toNode</tt>.
          *    </ol>
          */
         bool handleSetVehicleRestrictionsRecord(vector<MC2String>& rec, 
                                                 bool parseOnly);

         /**
          *    Handle one ADD_LANDMARK-record. Record definition:
          *    <ol>
          *       <li> itemType
          *       <li> POI-type
          *       <li> importance
          *       <li> coordinateType
          *       <li> coordinate
          *       <li> radius
          *       <li> crossings
          *       <li> coordinate of the "landmark-item"
          *       <li> name of the "landmark-item"
          *    </ol>
          */
         
         bool handleAddLandmarkRecord(vector<MC2String>& rec,
                                      bool parseOnly);
         /**
          *    Handle one ADD_SINGLE_LANDMARK-record. Adds a landmark to a 
          *    the connection from fromNode to toNode in the map. 
          *    Record definition:
          *    <ol>
          *       <li> itemType
          *       <li> POI-type
          *       <li> importance
          *       <li> coordinateType
          *       <li> coordinate of the "landmark-item"
          *       <li> name of the "landmark-item"
          *       <li> location of the landmark
          *       <li> side for the landmark
          *       <li> first coordinate of fromNode
          *       <li> second coordinate of fromNode
          *       <li> first coordinate of toNode
          *       <li> second coordinate of toNode
          *    </ol>
          */
         bool handleAddSingleLandmarkRecord(vector<MC2String>& rec,
                                            bool parseOnly);

         /**
          *    Handle one record that sets a boolean value to one item,
          *    identified one the node.
          *    Handles records of type SET_MULTIDIGITISED, SET_ROUNDABOUT,
          *    SET_ROUNDABOUTISH, SET_RAMP and SET_CONTROLLEDACCESS.
          *    Record defintion:
          *    <ol>
          *       <li> First coordinate
          *       <li> Second coordinate
          *       <li> The boolean value
          *    </ol>
          */
         bool handleSingleBooleanRecord(vector<MC2String>& rec,
                                        bool parseOnly);

         /**
          *    Handle one REMOVE_ITEM-record. Record defintion:
          *    <ol>
          *       <li> OldItemType
          *       <li> coordinateType
          *       <li> coordinate
          *       <li> name
          *    </ol>
          */
         bool handleRemoveItemRecord(vector<MC2String>& recStr,
                                     bool parseOnly);

         /**
          *    Handle one ADD_SIGNPOST-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) FromNode
          *       <li> (lat2,lon2) FromNode
          *       <li> (lat1,lon1) ToNode
          *       <li> (lat2,lon2) ToNode
          *       <li> signpost text
          *       <li> language
          *       <li> type
          *    </ol>
          */
         bool handleAddSignpostRecord(vector<MC2String>& rec,
                                      bool parseOnly);

         /**
          *    Handle one REMOVE_SIGNPOST-record. Exact same record 
          *    definition as for addSignpost:
          *    <ol>
          *       <li> (lat1,lon1) FromNode
          *       <li> (lat2,lon2) FromNode
          *       <li> (lat1,lon1) ToNode
          *       <li> (lat2,lon2) ToNode
          *       <li> signpost text
          *       <li> language
          *       <li> type
          *    </ol>
          */
         bool handleRemoveSignpostRecord(vector<MC2String>& rec,
                                         bool parseOnly);
         
         /**
          *    Handle one SET_HOUSENUMBER-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) OldItemCoord
          *       <li> (lat2,lon2) OldItemCoord
          *       <li> leftNbrStart
          *       <li> leftNbrEnd
          *       <li> rightNbrStart
          *       <li> rightNbrEnd
          *       <li> Housenumber type
          *    </ol>
          */
         bool handleSetHousenumberRecord(vector<MC2String>& rec,
                                         bool parseOnly);

         /**
          *    Handle one SET_ENTRYRESTRICTIONS-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) node coord
          *       <li> (lat2,lon2) node coord
          *       <li> value
          *    </ol>
          */
         bool handleSetEntryRestrictionsRecord(vector<MC2String>& rec,
                                         bool parseOnly);

         /**
          *    Handle one SET_SPEEDLIMIT-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) node coord
          *       <li> (lat2,lon2) node coord
          *       <li> value
          *    </ol>
          */
         bool handleSetSpeedLimitRecord(vector<MC2String>& rec,
                                        bool parseOnly);

         /**
          *    Handle one SET_TOLLROAD-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) node coord
          *       <li> (lat2,lon2) node coord
          *       <li> bool value (true/false)
          *    </ol>
          */
         bool handleSetTollRoadRecord(vector<MC2String>& rec,
                                      bool parseOnly);

         /**
          *    Handle one SET_LEVEL-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) node coord
          *       <li> (lat2,lon2) node coord
          *       <li> value
          *    </ol>
          */
         bool handleSetLevelRecord(vector<MC2String>& rec, bool parseOnly);

         /**
          *    Handle one SET_TURNDIRECTION-record. Record definition:
          *    <ol>
          *       <li> (lat1,lon1) FromNode
          *       <li> (lat2,lon2) FromNode
          *       <li> (lat1,lon1) ToNode
          *       <li> (lat2,lon2) ToNode
          *       <li> Turn direction when following connection
          *            from <tt>fromNode</tt> to <tt>toNode</tt>.
          *    </ol>
          */
         bool handleSetTurndirectionRecord(vector<MC2String>& rec,
                                         bool parseOnly);

         /**
          *    Handle one SET_WATER_TYPE-record. Record definition:
          *    <ol>
          *       <li> itemType
          *       <li> coordinate type
          *       <li> coordinate
          *       <li> name
          *       <li> name type
          *       <li> name language
          *       <li> new water type
          *    </ol>
          */
         bool handleSetWaterTypeRecord(vector<MC2String>& rec,
                                       bool parseOnly);
      //@}
     
      /**
       *    Adds a signpost text to a connection in the map.
       *    @param   fromNode    Where the connection comes from.
       *    @param   toNode      Where the connection leads to.
       *    @param   text        The signpost text.
       *    @param   lang        The language of the signpost text.
       *    @param   type        The name type of the signpost text.
       *    @param   checkRecord True if the record should be checked, 
       *                         fals eif the update should be done.
       *    @return  An integer: negative if the connection is not found,
       *                         0 if the signpost text is already present
       *                         in the connection, 1 if the signpost was 
       *                         added or checked and found to be needed.
       */
      int addSignpost(OldNode* fromNode, OldNode* toNode,
                      MC2String& text, LangTypes::language_t lang,
                      ItemTypes::name_t type, bool checkRecord);

      
      /*
       *    Print one record to mc2log.
       *    @param rec  The vector with the strings that represent this 
       *                record.
       */
      void printRecord(vector<MC2String>& rec);

      /**
       *    The map that is currently handled.
       */
      GMSMap* m_map;
      
      /**
       *    Will be set to true if the location of the city-parts should
       *    be updated after all records are read, false otherwise.
       */
      bool m_updateCityPartLocation;

      /**
       *    The number on the current line = extra data record 
       *    in the parsed file. After parsing, this will hold the 
       *    total number of records in the parsed file.
       */
      int m_curLineNumber;

      /**
       *    The number of extra data records that are added/applied 
       *    to the currently loaded map.
       */
      uint32 m_nbrRecordsAdded;

      /**
       *    The line numbers of the extra data records that are
       *    added/applied to all looped underview maps.
       */
      vector<int> m_recordsAddedToMaps;

      /**
       *    String for storing latest extra data date for m_map. Used when 
       *    applying dynamic extra data to decide if a record should be 
       *    applied or not (new or old compared to this extra data date).
       *    Format "yyyy-mm-dd hh:mm:ss".
       */
      MC2String m_mapEDdate;

      /**
       *    @name Attributes from COMMENT record.
       *    Attributes extraced from the extra data record comment, 
       *    used either when checking extra data validity or when applying 
       *    dynamic extra data.
       */
      //@{
         /**
          *    The id of the extra data record beeing processed,
          *    extracted from the COMMENT. To be used for information
          *    about which records fit which map when m_checkRecords
          *    is set to true.
          */
         uint32 m_recordId;

         /**
          *    The original value for the extra data record beeing
          *    processed, extracted from the COMMENT. To be used for
          *    e.g. vehicleRestrictions and entryRestrictions when
          *    checking the need of extra data records.
          */
         MC2String m_originalValStr;

         /**
          *    The extra data insert type for the extra data record beeing
          *    processed, extracted from the COMMENT. To be used for
          *    detecting not allowed records when applying dynamic extra 
          *    data.
          */
         MC2String m_edInsertType;

         /**
          *    The extra data record date-and-time for the extra data 
          *    record beeing processed, extracted from the COMMENT. Used
          *    in dynamic extra data to decide if the record should be 
          *    applied or if it is too old compared to the extra data 
          *    date of the map. Format "yyyy-mm-dd hh:mm:ss".
          */
         MC2String m_recordEDdate;
      //@}

      /**
       *    Tells if the extra data records should be checked.
       *    Is set to true when the m_checkRecordsTable is initated,
       *    false otherwise.
       */
      bool m_checkRecords;

      /**
       *    The struct that is used in the check table to hold attributes
       *    about one extra data record
       */
      struct recordnotices_t {
         uint32 recordId;
         MC2String record;
         vector<uint32> mapIds;  // fixme: set would be better, to avoid duplicates
         vector<uint32> mapIdCandidates;
         bool recordChecked;
         bool recordFound;
         bool recordNeeded;
         bool recordMapChanged;
         vector<MC2String> comment;
         OldExtraDataUtility::record_t recType;
      };

      typedef vector<recordnotices_t>::iterator recordNoticeIt;

      /**
       *    A table (vector) that holds attributes for all records in the 
       *    file beeing parsed, when checking the need of extra data records
       *    (e.g. the need of old ed-records in new map releases).
       */
      vector <recordnotices_t> m_checkRecordTable;
      
      /**
       *    @name Modify the check table.
       *    Methods for modifying attributes of records in the check table.
       *    Used when checking extra data validity (m_checkRecordTable).
       */
      //@{
         /**
          *    Get a pointer to a certain record.
          *    @return  A pointer to a record if recordId exists in the
          *             check table, else NULL.
          */
         recordnotices_t* getRecordWithId(uint32 recordId);

         /**
          *    Set the recordChecked param for a certain record.
          *    @return  True if everything ok, false if e.g. no
          *             record was found with the id.
          */
         bool setRecordChecked(bool checked, uint32 recId);
         
         /**
          *    Set the recordFound param for a certain record. If the
          *    param is true, also the mapid for the record is set
          *    from m_map.
          *    @return  True if everything ok, false if e.g. no
          *             record was found with the id.
          */
         bool setRecordFoundAndAddMapid(bool found, uint32 recId);
         
         /**
          *    Set the recordMapChanged param for a certain record.
          *    @return  True if everything ok, false if e.g. no
          *             record was found with the id.
          */
         bool setRecordMapChanged(bool changed, uint32 recId);
         
         /**
          *    Set the recordNeeded param for a certain record.
          *    @return  True if everything ok, false if e.g. no
          *             record was found with the id.
          */
         bool setRecordNeeded(bool needed, uint32 recId);
         
         /**
          *    Add a mapid for a certain record. Add to either
          *    the mapIds-vector or the mapIdCandidates-vector.
          *    @return  True if everything ok, false if e.g. no
          *             record was found with the id.
          */
         bool addMapId(uint32 mapId, uint32 recId, bool addToCandidatesVector);

         /**
          *    Check if this map is a candidate for a certain record.
          *    Used for finding map candidates for missing-records.
          *    If any item of itemtype it is close to the coordinate
          *    given (within 500 meters) the map is a candidate, and
          *    the mapID is added to the record's mapIdCandidates-vector.
          */
         void checkAndAddMapCandidate(ItemTypes::itemType it,
                              int32 lat, int32 lon, uint32 recId);
      //@}

      /**
       *    Tells if the extra data records are applied on underview maps
       *    of a merge, so called dynamic extra data. The meber variable
       *    is set to true in the initMergedMaps function, false otherwise.
       *
       *    When applying dynamic extra data, for each record the comment
       *    date is checked against the extra data date of the map, so no 
       *    old records are applied.
       *    IDs of all items affected by any record are saved in vectors 
       *    or other containers during the parsing. The changes are then
       *    by other methods applied also to overview- and country 
       *    overview maps, all levels.
       */
      bool m_applyOnMergedMaps;

      /**
       *    @name Vectors for dynamic extra data.
       *    Vectors used, when applying dynamic extra data to merged maps, 
       *    for saving node/item ids in the underview map beeing processed 
       *    for after-processing. The vectors are however filled 
       *    regardless of the m_applyOnMergedMaps attribute.
       */
      //@{
         /**
          *    m_affectsTurnDescr Contains nodeIds of the nodes where
          *                turn descriptions need to be re-generated.
          */
         vector<uint32> m_affectsTurnDescr;
         /**
          *    m_affectsStreets   Contains itemIds of those street segment
          *                items whose name(s) have been changed and require
          *                streets to be re-generated.
          */
         vector<uint32> m_affectsStreets;
         /**
          *    m_affectsMap    Contains itemIds of all other items that have
          *                    changed due to a map error correction.
          */
         vector<uint32> m_affectsMap;
      //@}

      /**
       *    When applying extra data to merged maps this method checks
       *    if turndescriptions or streets need to be re-generated
       *    using info in the affects-vectors.
       *    @return  True if at least one of turn descr or streets
       *             have been re-generated in the map, false otherwise.
       */
      bool turndirAndStreetsInMergedUnderviewMaps();

      /**
       *    Struct used when applying extra data to merged maps. Used
       *    for checking if turn description or crossing kind has changed
       *    for any connections and has to be tranferred to overview maps.
       */
      struct connInfo_t {
         uint32 toNodeId;
         ItemTypes::turndirection_t turndir;
         ItemTypes::crossingkind_t crossing;
      };
      
      /**
       *    Container (map) to hold info about turn description and 
       *    crossing kind for the connections of the map in process.
       */
      map<OldConnection*, connInfo_t> m_connectionInfo;
      
      /**
       *    Method that initiate the m_connectionInfo map with info
       *    from the map beeing processed.
       */
      inline void fillConnectionInfo();

      /**
       *    Container to hold info about connections in the map that 
       *    has been applied a setTurnDirection record. Used for 
       *    dynamic extra data, to make sure a setTurnDirection record 
       *    is never over-run if re-generating turn descriptions.
       */
      map<OldConnection*, connInfo_t> m_addedTurnDescrRecords;
      
      /**
       *    Container for holding all mapIDs.itemIds of items that
       *    are affected when applying extra data on merged maps.
       */
      multimap<uint32, uint32> m_itemsAffectedInUVMaps;

      /**
       *    Method that adds one itemId to the m_itemsAffectedInUVMaps
       *    multimap, meaning that the item has been affected when 
       *    applying extra data to merged maps.
       *    @return  True if the itemId was added to the multimap, false 
       *             if not (e.g. if the id already exist in the mutlimap).
       */
      inline bool addItemAsAffected(uint32 itemId);

      /**
       *    Method that adds one vector of itemIds to the 
       *    m_itemsAffectedInUVMaps multimap, meaningn that the items
       *    have been affected when applying extra data to merged maps.
       *    @param   affectVector   Vector with affected itemIds.
       *    @param   itemids        True if the vector hold itemIds, 
       *                            false if it is nodeIds.
       *    @return  Nbr of itemIds that are added to the multimap.
       */
      inline uint32 addItemsAsAffected(vector<uint32> affectVector,
                                       bool itemids = true); // else node ids

      /**
       *    Method to check if a itemId has already been added to the
       *    m_itemsAffectedInUVMaps multimap.
       *    return   True if the itemId was already present in the multimap,
       *             false if not.
       */
      inline bool itemAffected(uint32 itemId);

      /**
       *    @name Handle dates for dynamic extra data
       *    Methods for handling extra data dates when applying 
       *    dynamic extra data (m_applyOnMergedMaps).
       */
      //@{
         /**
          *    Method to get the latest extra data time from m_map and
          *    store it in m_mapEDdate. The latest of original creation
          *    time and dynamic extra data time is chosen.
          *    @return  True if m_map had a valid extra data time and the
          *             m_mapEDdate was set, false if not.
          */
         bool getExtradataTimeForMap();

         /**
          *    Method to extract the date and time from a extra data 
          *    record comment and store it in m_recordEDdate.
          *    @param   commentDateAndTime   The date-and-time part of a 
          *                extra data record comment (extracted from WASP),
          *                e.g. "# 2004-02-18 13:41:01".
          *    @return  True if date-and-time was extracted from the 
          *             record comment, false if not.
          */
         bool getExtradataRecordTime(const MC2String& commentDateAndTime);

         /**
          *    Find out if m_mapEDdate is less than m_recordEDdate, to 
          *    decide if a record should be applied when running dynamic 
          *    extra data. An extra data record that is older than the 
          *    m_mapEDdate was (most likely) applied in an earlier 
          *    dynamic extra data parsing, and should not be re-applied.
          *    @return  True if the record comment date is more resent
          *             than the map-date and should be applied, false
          *             if the record is old and should not be applied.
          */
         bool applyCurrentRecordDynamic();
      //@}
};


const char*
ExtraDataReader::getStringFromBool(bool boolVal) const
{
   if (boolVal)
      return "true";
   
   return "false";
}

inline uint32
ExtraDataReader::getNbrRecordsInEDFile()
{
   return m_curLineNumber;
}

inline uint32
ExtraDataReader::getNbrRecordsAdded()
{
   return m_nbrRecordsAdded;
}

vector<int>
ExtraDataReader::getRecordsAddedToMaps()
{
   return m_recordsAddedToMaps;
}

multimap<uint32, uint32>
ExtraDataReader::getItemsAffectedInUVMaps()
{
   return m_itemsAffectedInUVMaps;
}

bool
ExtraDataReader::addItemAsAffected(uint32 itemId)
{
   if (!itemAffected(itemId)) {
      m_itemsAffectedInUVMaps.insert(make_pair(m_map->getMapID(), itemId));
      return true;
   }
   return false;
}

uint32
ExtraDataReader::addItemsAsAffected(vector<uint32> affectVector,
                                   bool itemids)
{
   uint32 nbrAdded = 0;
   for (vector<uint32>::const_iterator it = affectVector.begin();
        it != affectVector.end(); it++) {

      // add to m_itemsAffectedInUVMaps
      uint32 id = (*it);
      if (!itemids)
         id &= 0x7fffffff;
      if (addItemAsAffected(id))
         nbrAdded++;
   }

   return nbrAdded;
}

bool
ExtraDataReader::itemAffected(uint32 itemId)
{
   bool itemAffected = false;

   uint32 mapId = m_map->getMapID();
   if (m_itemsAffectedInUVMaps.count(mapId) == 0)
      return false;
   
   // Loop the m_itemsAffectedInUVMaps mmap and check if itemid 
   // is already present in this map.
   bool cont = true;
   multimap<uint32, uint32>::const_iterator it = 
                              m_itemsAffectedInUVMaps.lower_bound(mapId);
   while (cont && !itemAffected &&
          (it != m_itemsAffectedInUVMaps.upper_bound(mapId))) {
      if (it->first != mapId)
         cont = false;
      else if (it->second == itemId)
         itemAffected = true;
      else
         it++;
   }

   return itemAffected;
}

#endif // EXTRADATAREADER_H


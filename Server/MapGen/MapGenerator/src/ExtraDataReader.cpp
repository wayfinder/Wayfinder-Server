/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "ExtraDataReader.h"
#include <fstream>
#include <map>
#include "StringUtility.h"
#include "OldMapHashTable.h"
#include "Utility.h"
#include "ItemTypes.h" 
#include <functional>
#include "GMSMap.h"
#include "GfxUtility.h"
#include "GMSGfxData.h"
#include "OldNode.h"
#include "OldConnection.h"

#include "ExtraDataReaderTables.h"

#include "OldCityPartItem.h"
#include "OldStreetSegmentItem.h"
#include "OldWaterItem.h"

#include "GMSPointOfInterestItem.h"

#include "OldBuiltUpAreaItem.h"
#include "TimeUtility.h"

const uint32
ExtraDataReader::NBRS_OF_CHECK_COORD_INSIDE_BUA = 100;


#define END_OF_RECORD "EndOfRecord"

ExtraDataReader::ExtraDataReader(const char* filePath)
   : m_path(filePath)
{
   // Fill in the lookup maps
   ExtraDataReaderTables::initItemTypes ( m_itemTypes );
   ExtraDataReaderTables::initLanguages ( m_languages );
   ExtraDataReaderTables::initNameTypes ( m_nameTypes );
   ExtraDataReaderTables::initPOITypes  ( m_POITypes  );
   // default is to add extradata (not check and not on merged maps)
   m_checkRecords = false;
   m_recordId = MAX_UINT32;
   m_applyOnMergedMaps = false;
   m_itemsAffectedInUVMaps.clear();
   m_mapEDdate = "";
   m_recordsAddedToMaps.clear();
}

ExtraDataReader::~ExtraDataReader()
{
}

void
ExtraDataReader::setFilePath(const char* filePath)
{
   m_path = filePath;
}

bool
ExtraDataReader::parseInto(GMSMap* theMap, bool parseOnly)
{
   // Check indata
   MC2_ASSERT(theMap != NULL);
   MC2_ASSERT(m_path != NULL);

   m_map = theMap;
   m_curLineNumber = 0;
   ifstream is(m_path);
   if (!is) {
      mc2log << error << "ExtraDataReader, file not found (\"" << m_path 
             << "\")" << endl;
      return false;
   }

   // Does the citypart location need to be updated afterwards?
   m_updateCityPartLocation = false;

   // Reset the affects-vectors etc since they store info per underview map
   m_affectsStreets.clear();
   m_affectsTurnDescr.clear();
   m_affectsMap.clear();
   m_connectionInfo.clear();
   m_addedTurnDescrRecords.clear();

   // Number records added to this map
   m_nbrRecordsAdded = 0;

   // If parsing dynamic extradata, get the lastest extradata-date
   // for this map, which will be used for detecting if a record should 
   // be applied or not.
   // Sets the m_mapEDdate
   if (m_applyOnMergedMaps) {
      if (getExtradataTimeForMap()) {
         mc2dbg2 << "Extra data date for map " << m_map->getMapID()
                 << " is \"" << m_mapEDdate << "\"" << endl;
      } else {
         mc2log << error << "EDR, no extradata time stored in map, "
                << "can't do dynamic extra data" << endl;
         return false;
      }
   }

   uint32 startTime = TimeUtility::getCurrentMicroTime();

   // Get the first record
   vector<MC2String> recordAsStrings;
   bool recordOK = OldExtraDataUtility::readNextRecordFromFile(
                                                   is, recordAsStrings);
   while ( recordOK && 
          (recordAsStrings.size() > 0) && 
          (recordAsStrings[0].size() > 0)) {
      bool status = false;
      m_curLineNumber++;

      DEBUG8(printRecord(recordAsStrings));

      // Switch on the record-type
      switch (OldExtraDataUtility::getRecordType(recordAsStrings[0])) {
         case OldExtraDataUtility::COMMENT:
            // Get the record id and original value if checking extra data.
            // Get ed insert type and date+time if doing dynamic extra data
            status = handleCommentRecord(recordAsStrings, parseOnly);
            mc2dbg4 << "Skipping comment" << endl;
            m_curLineNumber--;
            break;

         case OldExtraDataUtility::UPDATE_NAME:
            status = handleUpdateNameRecord(recordAsStrings, parseOnly);
            break;

         case OldExtraDataUtility::REMOVE_NAME_FROM_ITEM:
            status = handleRemoveNameFromItemRecord(
                                          recordAsStrings, parseOnly);
            break;

         case OldExtraDataUtility::ADD_NAME_TO_ITEM :
            status = handleAddNameToItemRecord(recordAsStrings,
                                               parseOnly);
            break;

         case OldExtraDataUtility::ADD_CITYPART :
            status = handleAddCitypartRecord(recordAsStrings, parseOnly);
            break;            

         case OldExtraDataUtility::SET_RAMP :
         case OldExtraDataUtility::SET_ROUNDABOUT :
         case OldExtraDataUtility::SET_ROUNDABOUTISH :
         case OldExtraDataUtility::SET_MULTIDIGITISED :
         case OldExtraDataUtility::SET_CONTROLLEDACCESS :
            status = handleSingleBooleanRecord(recordAsStrings, parseOnly);
            break;            

         case OldExtraDataUtility::SET_HOUSENUMBER : {
            status = handleSetHousenumberRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::SET_ENTRYRESTRICTIONS : {
            status = handleSetEntryRestrictionsRecord(recordAsStrings,
                                                      parseOnly);
            break;
         }

         case OldExtraDataUtility::SET_TOLLROAD : {
            status = handleSetTollRoadRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::SET_SPEEDLIMIT: {
            status = handleSetSpeedLimitRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::SET_LEVEL : {
            status = handleSetLevelRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::SET_TURNDIRECTION : {
            status = handleSetTurndirectionRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::ADD_SIGNPOST : {
            status = handleAddSignpostRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::REMOVE_SIGNPOST : {
            status = handleRemoveSignpostRecord(recordAsStrings, parseOnly);
            break;
         }

         case OldExtraDataUtility::SET_VEHICLERESTRICTIONS : 
            status = handleSetVehicleRestrictionsRecord(recordAsStrings, 
                                                        parseOnly);
            break;

         case OldExtraDataUtility::ADD_LANDMARK:
            status = handleAddLandmarkRecord(recordAsStrings,
                                             parseOnly);
            break;
            
         case OldExtraDataUtility::ADD_SINGLE_LANDMARK:
            status = handleAddSingleLandmarkRecord(recordAsStrings,
                                                   parseOnly);
            break;
         
         case OldExtraDataUtility::REMOVE_ITEM:
            status = handleRemoveItemRecord(recordAsStrings, parseOnly);
            break;
         
         case OldExtraDataUtility::SET_WATER_TYPE:
            status = handleSetWaterTypeRecord(recordAsStrings, parseOnly);
            break;
         
         case OldExtraDataUtility::NUMBER_OF_RECORD_TYPES :
            status = false;
            mc2log << error << "Unknown record-type \"" << recordAsStrings[0]
                   << "\" ";
            printRecord(recordAsStrings);
      }
      // Read next record
      recordOK = OldExtraDataUtility::readNextRecordFromFile(is, recordAsStrings);
      mc2dbg8 << " status=" << status << " recordOK=" << recordOK 
              << " rec[0] = \"" << recordAsStrings[0] << "\"" << endl;
   }
   mc2dbg4 << "Extradata for map " << m_map->getMapID() << " took "
           << (TimeUtility::getCurrentMicroTime() - startTime) /1000.0 
           << " ms" << endl;

   // Update citypart location if needed.
   if (m_updateCityPartLocation)
      m_map->setCityPartLocation();

   if (m_applyOnMergedMaps) {
      // Store all connections in the map in order to find out if any 
      // turn directions or crossing kinds change during after-processing
      fillConnectionInfo();
      mc2dbg4 << "Filled m_connectionInfo with info about "
              << m_connectionInfo.size() << " connections" << endl;

      // Set new turn directions and/or re-generate streets if
      // extra data affecting these was applied
      bool mapChanged = turndirAndStreetsInMergedUnderviewMaps();
      if (mapChanged)
         mc2dbg << "ApplyOnMergedMaps turn descr or streets "
                << "may have changed" << endl;
      else
         mc2dbg << "ApplyOnMergedMaps neither turn descr nor "
                << "streets changed" << endl;

      // Find out if any turn directions or crossing kinds were
      // affected by any records (needed for transfer to overview maps)
      if (mapChanged) {
         uint32 nbrChanged = 0;
         uint32 nbrNull = 0;
         map<OldConnection*, connInfo_t>::const_iterator it = 
                                       m_connectionInfo.begin();
         while (it != m_connectionInfo.end()) {
            OldConnection* conn = (*it).first;
            connInfo_t connInfo = (*it).second;
            if (conn != NULL) {
               if ((conn->getTurnDirection() != connInfo.turndir) ||
                   (conn->getCrossingKind() != connInfo.crossing)) {
                  nbrChanged++;

                  // Add the item to m_itemsAffectedInUVMaps
                  addItemAsAffected(connInfo.toNodeId & ITEMID_MASK);
               }
            }
            // else The conn was perhaps removed ??
            else {
               nbrNull++;
            }
            it++;
         }
         mc2dbg2 << "ApplyOnMergedMaps: nbr changed turndir or crossingkind " 
                 << nbrChanged << endl;
         mc2dbg4 << "ApplyOnMergedMaps: nbr null connections " 
                 << nbrNull << endl;
         mc2dbg1 << "ApplyOnMergedMaps: Totally affected after map "
                 << m_map->getMapID() << " step 1 = "
                 << m_itemsAffectedInUVMaps.size() << endl;

         // Transfer the affected ids to the "global" m_itemsAffectedInUVMaps
         uint32 nbrAdded = 0;
         nbrAdded = addItemsAsAffected(m_affectsTurnDescr, false); // node ids
         mc2dbg2 << "ApplyOnMergedMaps: added " << nbrAdded << " of " 
                 << m_affectsTurnDescr.size() << " from affectsTurnDescr" 
                 << endl;
         nbrAdded = addItemsAsAffected(m_affectsStreets);
         mc2dbg2 << "ApplyOnMergedMaps: added " << nbrAdded << " of " 
                 << m_affectsStreets.size() << " from affectsStreets" << endl;
         mc2dbg1 << "ApplyOnMergedMaps: Totally affected after map "
                 << m_map->getMapID() << " step 2 = "
                 << m_itemsAffectedInUVMaps.size() << endl;
      }
      // Transfer the rest of the affected ids
      uint32 nbrAdded = 0;
      nbrAdded = addItemsAsAffected(m_affectsMap);
      mc2dbg2 << "ApplyOnMergedMaps: added " << nbrAdded << " of " 
              << m_affectsMap.size() << " from affectsMap" << endl;
      mc2dbg1 << "ApplyOnMergedMaps: Totally affected after map "
              << m_map->getMapID() << " step 3 = "
              << m_itemsAffectedInUVMaps.size() << endl;
   }
   m_affectsStreets.clear();
   m_affectsTurnDescr.clear();
   m_affectsMap.clear();
   m_connectionInfo.clear();
   m_addedTurnDescrRecords.clear();
   return true;
}

bool
ExtraDataReader::initCheckTable()
{
   mc2dbg4 << "initCheckTable" << endl;

   // Check indata
   MC2_ASSERT(m_path != NULL);

   ifstream is(m_path);
   if (!is) {
      mc2log << error << "ExtraDataReader:initCheckTable "
             << "file not found (\"" << m_path << "\")" << endl;
      return false;
   }

   uint32 startTime = TimeUtility::getCurrentTime();

   // Set this variable to true so we know what to do in
   // handleCommentRecord, parseInto and printCheckTable
   m_checkRecords = true;
   m_curLineNumber = 0;

   // Get the first record
   vector<MC2String> recordAsStrings;
   bool recordOK = OldExtraDataUtility::readNextRecordFromFile(
                                                   is, recordAsStrings);
   uint32 recNbr = 0;
   while ( recordOK && 
          (recordAsStrings.size() > 0) && 
          (recordAsStrings[0].size() > 0)) {
      bool status = false;
      m_curLineNumber++;
      
      // Switch on the record-type
      switch (OldExtraDataUtility::getRecordType(recordAsStrings[0])) {
         case OldExtraDataUtility::COMMENT: {
            // Get the record id and original value.
            status = handleCommentRecord(recordAsStrings,false);
            m_curLineNumber--;

            if (m_recordId != MAX_UINT32) {
               // Create a record notice for this record id and 
               // add to the check table (initiate the attributes)
               
               recordnotices_t rec;
               rec.recordId = m_recordId;
               rec.comment = recordAsStrings;
               rec.mapIds.clear();
               rec.mapIdCandidates.clear();
               rec.recordChecked = false;
               rec.recordFound = false;
               rec.recordMapChanged = false;
               rec.recordNeeded = false;
               m_checkRecordTable.push_back( rec );
            } else{
               mc2log << error << "Record " << recNbr+1
                      << " has no valid extra data record id." << endl;
            }
         }
         break;
         
         case OldExtraDataUtility::NUMBER_OF_RECORD_TYPES : {
            // Record is incorrect
            m_recordId = MAX_UINT32;
            m_originalValStr = "";
            mc2log << error << "Record " << recNbr+1
                   << " has no valid extra data record type." << endl;
         }
         break;
            
         default : {
            // Find the rec for m_recordId and add the recordStr
            // Fill the check table with information about 
            // this extra data record
            recNbr++;

            recordnotices_t* rec = getRecordWithId(m_recordId);
            if (rec != NULL) {
               MC2String recordStr = 
                  OldExtraDataUtility::createEDFileString(recordAsStrings, true);
               rec->record = recordStr;

               rec->recType = 
                  OldExtraDataUtility::getRecordType(recordAsStrings[0]);
            }
            if (recNbr % 1000 == 0) {
               cout << "Adding record " << recNbr 
                    << " recId=" << m_recordId << endl;
            }
            
            // to detect if the extra data records are incorrect..
            if (recNbr != m_checkRecordTable.size()) {
               mc2log << error << "Number records does not match for record "
                      << recNbr << ", m_id=" << m_recordId << endl;
               return false;
            }
         }
         break;
      }
      // Read next record
      recordOK = OldExtraDataUtility::readNextRecordFromFile(is, recordAsStrings);
   }
   mc2dbg1 << "initCheckTable: m_checkRecordTable.size()=" 
           << m_checkRecordTable.size() << ", no check entry for " 
           << recNbr-m_checkRecordTable.size() << " records" << endl;
   
   // XXX sort the vector on recordIds ?

   mc2dbg1 << "Extradata initCheckTable for edfile '" << m_path << "' took " 
           << TimeUtility::getCurrentTime() - startTime << " ms" << endl;

   return true;
}

bool
ExtraDataReader::printCheckTable()
{
   mc2dbg2 << "printCheckTable()" << endl;
   
   if (!m_checkRecords) {
      mc2log << warn << "nothing to print from the checkRecordTable "
             << "(m_checkRecords is false)" << endl;
      return false;
   }
   // else
   uint32 startTime = TimeUtility::getCurrentTime();
  
   // strip m_path to contain no directories, just the filename..
   char* edfileName = new char[strlen(m_path)+1];
   char* tmpStr = StringUtility::strrchr(m_path, '/');
   if (tmpStr ==  NULL) {
      // no occurrence of '/' in the m_path.
      strcpy(edfileName, m_path);
   } else {
      strcpy(edfileName, tmpStr+1);
   }
   
   // create outfiles:
   // found_use_  found_nouse_  found_changed  missing_  nocheck_
   char* foundUsefileName = new char[strlen(edfileName) + 15];
   strcpy(foundUsefileName, "found_use_");
   strcat(foundUsefileName, edfileName);
   char* foundNousefileName = new char[strlen(edfileName) + 15];
   strcpy(foundNousefileName, "found_nouse_");
   strcat(foundNousefileName, edfileName);
   char* foundChangedfileName = new char[strlen(edfileName) + 15];
   strcpy(foundChangedfileName, "found_changed_");
   strcat(foundChangedfileName, edfileName);
   char* missingfileName = new char[strlen(edfileName) + 10];
   strcpy(missingfileName, "missing_");
   strcat(missingfileName, edfileName);
   char* missingNoPriofileName = new char[strlen(edfileName) + 20];
   strcpy(missingNoPriofileName, "missing_noprio_");
   strcat(missingNoPriofileName, edfileName);
   char* nocheckfileName = new char[strlen(edfileName) + 10];
   strcpy(nocheckfileName, "nocheck_");
   strcat(nocheckfileName, edfileName);
   mc2dbg1 << " m_path=" << m_path << " foundusefile=" << foundUsefileName
           << " foundnousefile=" << foundNousefileName
           << " missingfile=" << missingfileName 
           << " missingNoPriofile=" << missingNoPriofileName 
           << endl;

   ofstream foundUseFile(foundUsefileName);
   ofstream foundNouseFile(foundNousefileName);
   ofstream foundChangedFile(foundChangedfileName);
   ofstream missingFile(missingfileName);
   ofstream missingNoPrioFile(missingNoPriofileName);
   ofstream nocheckFile(nocheckfileName);
   delete [] foundUsefileName;
   delete [] foundNousefileName;
   delete [] foundChangedfileName;
   delete [] missingfileName;
   delete [] missingNoPriofileName;
   delete [] nocheckfileName;
   delete [] edfileName;
   
   // loop the table and print each record to correct file..
   mc2dbg << "printCheckTable: m_checkRecordTable.size=" 
           << m_checkRecordTable.size() << endl;
   uint32 recNbr = 0;
   for (recordNoticeIt it = m_checkRecordTable.begin();
        it < m_checkRecordTable.end(); it++) {

      recNbr++;
      recordnotices_t rec = (*it);
      
      // Check how many maps there are in 1) mapIds or 2) mapIdCandidates 
      // vectors and loop and print the record that many times.
      // Use mapid in the comment str!
      uint32 nbrMaps = 0;
      if (rec.recordFound)
         nbrMaps = rec.mapIds.size();
      else if (rec.recordChecked)
         nbrMaps = rec.mapIdCandidates.size();
      if (nbrMaps > 1) {
         mc2dbg << "There are several maps for ed record " 
                 << rec.recordId << "(nbrMaps=" << nbrMaps << endl;
      } else if (nbrMaps == 0) {
         mc2dbg << "There are no maps for ed record id "
                 << rec.recordId << endl;
      }

      for (uint32 r = 0; r < nbrMaps; r++) {
         
         uint32 mapId = MAX_UINT32;
         if (rec.recordFound)
            mapId = rec.mapIds[r];
         else if (rec.recordChecked)
            mapId = rec.mapIdCandidates[r];
         char mapStr[5];
         sprintf(mapStr, "%d", mapId);
         
         // Update field 11=mapId to be the mapStr from checktable
         vector<MC2String> newComments;
         uint32 field = 0;
         for (vector<MC2String>::const_iterator cit=rec.comment.begin(); 
               cit!=rec.comment.end(); cit++) {
            if (field == 11) {
               // print mapid from the checktable
               newComments.push_back( mapStr );
            } else {
               newComments.push_back(*cit);
            }
            field++;
         }
         // Create comment record
         MC2String commentStr =
            OldExtraDataUtility::createEDFileString( newComments, true );

         if (rec.recordId == MAX_UINT32) {
            // This record is incorrect..
            // (no such records should exist in the table)
            mc2log << warn << "printCheckTable: no valid recordId " 
                   << rec.recordId << " for record nbr " << recNbr 
                   << " '" << rec.record << "'" << endl;
            
         } else if (!rec.recordChecked) {
            // print to nocheckfile
            nocheckFile << endl << commentStr << endl << rec.record << endl;
            mc2dbg4 << "record not checked " << rec.recordId << " '"
                    << rec.record << "'" << endl;
            
         } else if (rec.recordFound) {
            // print to foundUsefile/foundNousefile - check recordNeeded
            if (rec.recordNeeded) {
               foundUseFile << endl << commentStr << endl 
                            << rec.record << endl;
            } else {
               foundNouseFile << endl << commentStr << endl
                              << rec.record << endl;
            }
            if (rec.recordMapChanged) {
               foundChangedFile << endl << commentStr << endl
                                << rec.record << endl;
            }
            mc2dbg4 << "record found " << rec.recordId << " (needed="
                    << rec.recordNeeded << " mapchanged="
                    << rec.recordMapChanged << ") nbrmaps:" 
                    << rec.mapIds.size() << endl;
            
         } else if (!rec.recordFound) {
            // print to missingfile - using the mapid candidates
            if ( rec.recType == OldExtraDataUtility::SET_SPEEDLIMIT ) {
               missingNoPrioFile << endl << commentStr << endl
                                 << rec.record << endl;
            } else {
               missingFile << endl << commentStr << endl
                           << rec.record << endl;
            }
            mc2dbg4 << "record missing " << rec.recordId 
                    << " nbrmaps:" << rec.mapIds.size() 
                    << " nbrcandmaps:" << rec.mapIdCandidates.size() << endl;

         } else {
            // should be none
            mc2log << warn << "printCheckTable: something is "
                   << "incorrect about record " << rec.recordId << endl;
         }
      }
      
      if (nbrMaps == 0) {
         // For this record, nothing was found.
         // The record was either not checked, or not found and not close 
         // enough to any map. The latter probably means that it is a record 
         // that belongs to a map that was not looped/tested at all.
         // Print to the missing file
         
         // Update field 11=mapId to be the empty string
         vector<MC2String> newComments;
         uint32 field = 0;
         for (vector<MC2String>::const_iterator cit=rec.comment.begin(); 
               cit!=rec.comment.end(); cit++) {
            if (field == 11) {
               // print mapid from the checktable
               newComments.push_back("");
            } else {
               newComments.push_back(*cit);
            }
            field++;
         }
         // Create comment record
         MC2String commentStr =
            OldExtraDataUtility::createEDFileString( newComments, true );

         if (rec.recordId == MAX_UINT32) {
            // This record is incorrect..
            // (no such records should exist in the table)
            mc2log << warn << "printCheckTable: no valid recordId " 
                   << rec.recordId << " for record nbr " << recNbr 
                   << " '" << rec.record << "'" << endl;
            
         } else if (!rec.recordChecked) {
            // print to nocheckfile
            nocheckFile << endl << commentStr << endl << rec.record << endl;
            mc2dbg4 << "record not checked " << rec.recordId << " '"
                    << rec.record << "'" << endl;
         } else {
            if ( rec.recType == OldExtraDataUtility::SET_SPEEDLIMIT ) {
               missingNoPrioFile << endl << commentStr << endl
                                 << rec.record << endl;
            } else {
               missingFile << endl << commentStr << endl
                           << rec.record << endl;
            }
         }
      }
   }

   mc2dbg1 << "Extradata printCheckTable for edfile '" << m_path 
           << "' took " << TimeUtility::getCurrentTime() - startTime
           << " ms" << endl;
   
   return true;
}

void
ExtraDataReader::initMergedMaps()
{
   m_applyOnMergedMaps = true;
}

bool
ExtraDataReader::turndirAndStreetsInMergedUnderviewMaps()
{
   bool mapChanged = false;
   vector<uint32>::iterator it;

   // Regenerate turndesriptions because of e.g. change of ramp, rbt,
   // entry restr, vehicle restr..
   if (m_affectsTurnDescr.size() > 0) {
      // The m_affectsTurnDescr vector may contain one nodeid several times
      Vector nodeids;
      for (it = m_affectsTurnDescr.begin(); 
           it != m_affectsTurnDescr.end(); it++) {
         nodeids.addLastIfUnique(*it);
      }
      mapChanged = true;
      mc2dbg2 << "To re-generate turn desc because of " 
              << m_affectsTurnDescr.size() << " (" << nodeids.getSize()
              << " unique ids) changes in merged maps." << endl;
      for (uint32 i=0; i < nodeids.getSize(); i++) {
         mc2dbg2 << "To re-gen turn descr for node " 
                 << nodeids.getElementAt(i) << endl;
         m_map->initTurnDescriptions(nodeids.getElementAt(i));
      }

      // Check if there are any setTurnDirection-records that now have
      // to be re-applied.
      if (m_addedTurnDescrRecords.size() > 0) {
         mc2dbg2 << "Check if to re-apply " << m_addedTurnDescrRecords.size()
                 << " setTurnDirection-records" << endl;
         uint32 nbrReApplied = 0;
         map<OldConnection*, connInfo_t>::const_iterator it = 
                                       m_addedTurnDescrRecords.begin();
         while (it != m_addedTurnDescrRecords.end()) {
            OldConnection* conn = (*it).first;
            connInfo_t connInfo = (*it).second;
            if (conn != NULL) {
               if (conn->getTurnDirection() != connInfo.turndir) {
                  nbrReApplied++;
                  conn->setTurnDirection(connInfo.turndir);
               }
            }
            // else The conn was perhaps removed ??
            it++;
         }
         mc2dbg2 << "Re-applied " << nbrReApplied 
                 << " setTurnDirection-records" << endl;
      }
   }

   // Regenerate streets because of e.g. addNameToItem, updateName or
   // removeNameFromItem for streets and street segments
   if (m_affectsStreets.size() > 0) {
      mapChanged = true;
      mc2dbg2 << "To re-generate streets because of " 
              << m_affectsStreets.size() << " changes in merged maps." << endl;
      uint32 nbrStreets = 0;
      for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z) {
         for(uint32 i = 0; i < m_map->getNbrItemsWithZoom(z); ++i) {
            OldItem* item = m_map->getItem(z, i);
            if ((item != NULL) &&
                (item->getItemType() == ItemTypes::streetItem)) {
               nbrStreets++;
            }
         }
      }
      mc2dbg2 << "Number streets in map = " << nbrStreets << endl;
      // First remove all streets from the map since we can not generate
      // new streets if old ones are present.
      nbrStreets = 0;
      for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z) {
         for(uint32 i = 0; i < m_map->getNbrItemsWithZoom(z); ++i) {
            OldItem* item = m_map->getItem(z, i);
            if ((item != NULL) &&
                (item->getItemType() == ItemTypes::streetItem)) {
               // Remove street, no need to re-build map hash table 
               // until all items are removed
               m_map->removeItem(item->getID(), false);
               nbrStreets++;
            }
         }
      }
      mc2dbg2 << "Removed nbr streets " << nbrStreets << endl;
      m_map->buildHashTable();
      // Then generate new streets
      m_map->generateStreetsFromStreetSegments();
      mc2dbg2 << "Streets generated" << endl;
      // Need to set location of these new streets.
      m_map->updateStreetLocationAll(ItemTypes::municipalItem);
      m_map->updateStreetLocationAll(ItemTypes::builtUpAreaItem);
      mc2dbg2 << "Location-groups set for streets" << endl;
   }

   return mapChanged;
}


OldConnection*
ExtraDataReader::getConnection(OldNode* fromNode, OldNode* toNode)
{
   OldConnection* con = NULL;
   if ((fromNode != NULL) && (toNode != NULL)){
      // Find the connection
      uint32 j=0;
      uint32 fromNodeID = fromNode->getNodeID();
      while ( (j < toNode->getNbrConnections()) && 
              (con == NULL) ) {
         OldConnection* tmpCon = toNode->getEntryConnection(j);
         if ((tmpCon != NULL) && (tmpCon->getFromNode() == fromNodeID)) {
            con = tmpCon;
         }
         j++;
      }
   }
   return (con);
}

int 
ExtraDataReader::setTurndirection(OldNode* fromNode, OldNode* toNode,
                                  ItemTypes::turndirection_t turnDir, bool checkRecord)
{
   OldConnection* con = getConnection(fromNode, toNode);
   if (con != NULL) {
      if (!checkRecord) {
         // update
         con->setTurnDirection(turnDir);
         mc2dbg4 << "turndirection-value updated" << endl;
         return 1;
      } else {
         // check if the values differ
         if (con->getTurnDirection() != turnDir)
            return 1;
         else
            return 0;
      }
   }

   // No connection found
   mc2dbg4 << "Failed to update turndirection-value" << endl;
   return -1;
}

int
ExtraDataReader::addSignpost(OldNode* fromNode, OldNode* toNode,
                             MC2String& text, LangTypes::language_t lang,
                             ItemTypes::name_t type, bool checkRecord)
{
   MC2_ASSERT(m_map != NULL);
   MC2_ASSERT(fromNode != NULL);
   MC2_ASSERT(toNode != NULL);

   OldConnection* con = getConnection(fromNode, toNode);
   if (con != NULL) {
      // Make sure that a connection exists between the nodes.

      // Compose a sign post element.
      GMSSignPostElm signPostElm;
      uint32 nameStringCode = m_map->addName(text.c_str(), lang, type);
      signPostElm.setTextStringCode(nameStringCode);
      GMSSignPostElm::elementType_t elmType = GMSSignPostElm::placeName;
      if ( type == ItemTypes::roadNumber ){
         elmType = GMSSignPostElm::routeNumber;
      }
      else if ( type == ItemTypes::exitNumber ){
         elmType = GMSSignPostElm::exitNumber;
      }
      signPostElm.setType(elmType);
      
      return
         m_map->getNonConstSignPostTable().addSingleElmSignPost(fromNode->getNodeID(),
                                                         toNode->getNodeID(),
                                                         signPostElm );
   }
   mc2dbg4 << "Failed to add signpost" << endl;
   return -1;
}


ItemTypes::itemType
ExtraDataReader::getItemType(MC2String& s) const
{
   typedef map<MC2String,ItemTypes::itemType,
               strNoCaseCompareLess>::const_iterator
      CI;
   
   CI i = m_itemTypes.find(s);
   if(i == m_itemTypes.end())
      return ItemTypes::nullItem;
   return i->second;
}

ItemTypes::name_t
ExtraDataReader::getNameType(MC2String& s) const
{
   typedef map<MC2String,ItemTypes::name_t,strNoCaseCompareLess>::const_iterator
      CI;

   CI i = m_nameTypes.find(s);
   if(i == m_nameTypes.end())
      return ItemTypes::invalidName;
   return i->second;
}

LangTypes::language_t
ExtraDataReader::getLanguage(MC2String& s) const
{
   LangTypes::language_t result = 
      LangTypes::getStringAsLanguage(s.c_str(), 
                                     false /*not full name*/ );
   if ( result != LangTypes::invalidLanguage ){
      return result;
   }
      
   // Only gets here if the language code was not found in LangTypes.
   typedef map<MC2String,LangTypes::language_t,
      strNoCaseCompareLess>::const_iterator CI;
   
   CI i = m_languages.find(s);
   if(i == m_languages.end())
      return LangTypes::invalidLanguage;
   return i->second;
}

ItemTypes::pointOfInterest_t
ExtraDataReader::getPOIType(MC2String& s) const
{
   typedef map<MC2String,
               ItemTypes::pointOfInterest_t,
               strNoCaseCompareLess>::const_iterator CI;

   CI i = m_POITypes.find(s);
   if(i == m_POITypes.end())
      return ItemTypes::unknownType;
   return i->second;
}

uint32
ExtraDataReader::getUint32(MC2String& str) const
{

   char* dest = NULL;
   uint32 value = 0;
   const char* startPtr = (const char*) str.c_str();
   if ( Utility::getUint32(startPtr, dest, value) )
      return (uint32) value;
   else
      return MAX_UINT32;
}

int
ExtraDataReader::getNumber(MC2String& str) const
{

   char* dest = NULL;
   int value = 0;
   const char* startPtr = (const char*) str.c_str();
   if ( Utility::getNumber(startPtr, dest, value) )
      return value;
   else
      return MAX_INT32;
}


uint32
ExtraDataReader::lookupStringIndex(GMSMap* theMap,
                                   const MC2String& str) const
{
   uint32 startMicro = TimeUtility::getCurrentMicroTime();
   uint32 stringIndex = theMap->getItemNames()->stringExist(str.c_str());
   uint32 endMicro = TimeUtility::getCurrentMicroTime();
   mc2dbg4 << "Linear search for \"" << str << "\", found at pos=" 
           << stringIndex << ", done in " << (endMicro-startMicro)/1000.0 
           << " ms" << endl;

   if (stringIndex == MAX_UINT32) {
      return 0;
   }
   return stringIndex;
}


bool
ExtraDataReader::handleCommentRecord(vector<MC2String>& rec, bool parseOnly)
{
   mc2dbg4 << "COMMENT:" << endl;
   // record-definition:
   // 0  # date and time
   // 1  writer
   // 2  source
   // 3  comment
   // 4  original value
   // 5  ref id (map supplier map error ref id)
   // 6  ed record id
   // 7  ed insert type
   // 8  map release
   // 9  extra data group id
   // 10 country
   // 11 map id

   if (!m_checkRecords) {
      m_recordId = MAX_UINT32;
      m_originalValStr = "";
   }
   else {
      // check record
      if ((rec.size() < 7) ||
          (rec[6].size() == 0) ||
          !StringUtility::onlyDigitsInString(rec[6].c_str())) {
         // not all ed records have a record id in the log comment,
         // so don't print
         // BUT records without record id cannot be checked!
         mc2dbg4 << "Error in COMMENT record, cannot extract "
                 << "extra data record id '" << rec[6] << "'" << endl;
         m_recordId = MAX_UINT32;

         // use the curLineNumber as a temporary recordId
         m_recordId = m_curLineNumber;
      }
      else {
         // get the recordId
         uint32 val = getUint32(rec[6]);
         m_recordId = val;
      }

      // get the originalVal if any exists
      if ((rec.size() >= 5) && (rec[4].size() > 0))
         m_originalValStr = rec[4];
      else
         m_originalValStr = "";
   }

   if (m_applyOnMergedMaps) {
      // Get and store the record date and time
      getExtradataRecordTime(rec[0]);
      // Get extra data insert type
      if ((rec.size() >= 8) && (rec[7].size() > 0))
         m_edInsertType = rec[7];
      else
         m_edInsertType = "";
   } else {
      m_edInsertType = "";
      m_recordEDdate = "";
   }

   return true;
}


bool
ExtraDataReader::handleUpdateNameRecord(vector<MC2String>& rec, bool parseOnly)
{
   mc2dbg4 << "UPDATE_NAME:" << endl;

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   ItemTypes::itemType it = getItemType(rec[1]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[2]);
   int32 lat, lon;
   bool coordOK = OldExtraDataUtility::strtolatlon(rec[3], lat, lon, coordType);
   ItemTypes::name_t nt = getNameType(rec[5]);
   LangTypes::language_t lang = getLanguage(rec[6]);
   // 4 = misspelled name
   // 7 = correct name

   // Check indata
   if ( (rec.size() != 8) ||
        (rec[4].size() == 0) ||
        (rec[7].size() == 0) ||
        (it == ItemTypes::nullItem) ||
        (nt == ItemTypes::invalidName) ||
        (coordType == CoordinateTransformer::nbrCoordinateTypes) ||
        (!coordOK) ) {
      mc2log << error << "Error in UPDATE_NAME record it="
             << StringTable::getString(ItemTypes::getItemTypeSC(it),
                                       StringTable::ENGLISH) << " nt="
             << ItemTypes::getNameTypeAsString(nt) << " lang="
             << LangTypes::getLanguageAsString(lang);
      if (coordType != CoordinateTransformer::nbrCoordinateTypes) 
         mc2log << " cordType OK";
      else 
         mc2log << " cordType NOT OK";
      if (coordOK) 
         mc2log << " cord OK" << endl;
      else 
         mc2log << " cord NOT OK" << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   if ( m_applyOnMergedMaps &&
        ((it == ItemTypes::municipalItem) ||
         (it == ItemTypes::builtUpAreaItem)) &&
        ((m_edInsertType.size() == 0) || 
         (strcasecmp(m_edInsertType.c_str(),
                     "beforeInternalConnections") == 0)) ) {
      // Admin areas affecting merging and/or location-groups setting
      // (i.e. bua and mun) are (for now) not handled when applying 
      // extra data to merged maps.
      mc2log << warn << "For merged maps, record " << m_curLineNumber 
             << " UPDATE_NAME (" << ItemTypes::getItemTypeAsString(it) 
             << ") not handled" << endl;
      return true;
   }
   
   // Make update
   vector<OldItem*> closeItems;
   OldItem* item = m_map->getItemFromCoordinate(it, lat, lon, closeItems,
                                             rec[4].c_str(), -1);
   if (item != NULL) {
      // item found
      if (m_checkRecords) {
         setRecordFoundAndAddMapid(true, m_recordId);
      }

      // If the new name already exists, don't update to it..
      uint32 foo;
      bool itemHasNewName = m_map->itemHasNameAndType(
               item, lang, nt, rec[7].c_str(), foo);
      
      if (itemHasNewName) {
         mc2log << warn << "In updateName: Item already has the new name "
                << rec[7] << endl;
         return false;
      }
   
      int counter = OldExtraDataUtility::updateName(
                                 m_map, item, it, rec[4],
                                 nt, lang, rec[7],
                                 m_affectsStreets);
      if (counter == -1) {
         mc2log << error << here << " updateName(" << m_map << ", "
                << (int)it << ", " << lat << ", " << ", " << lon << rec[4]
                << ", " << (int)nt << ", " << (int)lang << ", " << rec[7] 
                << ")" << endl;
         return false;
      } else if (counter > 0) {
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber
                << " Updated name '" << rec[4] << "' -> '" << rec[7]
                << "' to " << counter << " item(s) for item " << item->getID() 
                << "  in map " 
                << m_map->getMapID() << endl;
         if (m_applyOnMergedMaps) {
            m_affectsMap.push_back(item->getID());
         }
         if ( m_checkRecords ) {
            setRecordNeeded(true, m_recordId);
         }
      } else if (counter == 0 ) {
         mc2dbg4 << "Counter=0 for UN " << rec[4] << "->" << rec[7] << endl;
         // Something was wrong with the record, item was found, but record
         // was not applied. Maybe the lang name was not correct?
         // Write to missing file if checking ED
         if (m_checkRecords) {
            setRecordFoundAndAddMapid(false, m_recordId);
            setRecordNeeded(false, m_recordId);
            checkAndAddMapCandidate(it, lat, lon, m_recordId);
         }
      }
   } else {
      DEBUG4(
      MC2BoundingBox bbox;
      m_map->getGfxData()->getMC2BoundingBox(bbox);
      if (bbox.contains(lat, lon)) {
         mc2log << warn << here 
                << " Possible error: didn't find any item at (" << lat
                << "," << lon << ") called \"" << rec[4] << "\"" << endl;
      });
      if (m_checkRecords) {
         // if name is already correct, the record is found but not needed
         item =  m_map->getItemFromCoordinate(it, lat, lon, closeItems,
                                              rec[7].c_str(), -1);
         if (item != NULL) {
            setRecordFoundAndAddMapid(true, m_recordId);
            return true;
         } else {
            // check if the coordinate fits in this map
            checkAndAddMapCandidate(it, lat, lon, m_recordId);
         }
      }
   }

   return true;
}

bool
ExtraDataReader::handleRemoveNameFromItemRecord(
               vector<MC2String>& rec, bool parseOnly)
{
   mc2dbg4 << "REMOVE_NAME_FROM_ITEM:" << endl;

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   ItemTypes::itemType it = getItemType(rec[1]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[2]);
   int32 lat, lon;
   bool coordOK = OldExtraDataUtility::strtolatlon(rec[3], lat, lon, coordType);
   ItemTypes::name_t nt = getNameType(rec[5]);
   LangTypes::language_t lang = getLanguage(rec[6]);

   // Check indata
   if ( (rec.size() != 7) ||
        (rec[4].size() == 0) ||
        (it == ItemTypes::nullItem) ||
        (nt == ItemTypes::invalidName) ||
        (lang == LangTypes::nbrLanguages) ||
        (coordType == CoordinateTransformer::nbrCoordinateTypes) ||
        (!coordOK) ) {
      mc2log << error << "Error in REMOVE_NAME_FROM_ITEM record it="
             << StringTable::getString(ItemTypes::getItemTypeSC(it),
                                       StringTable::ENGLISH) << " nt="
             << ItemTypes::getNameTypeAsString(nt) << " lang="
             << LangTypes::getLanguageAsString(lang);
      if (coordType != CoordinateTransformer::nbrCoordinateTypes) 
         mc2log << " cordType OK";
      else 
         mc2log << " cordType NOT OK";
      if (coordOK) 
         mc2log << " cord OK" << endl;
      else 
         mc2log << " cord NOT OK" << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // Make update
   vector<OldItem*> closeItems;
   OldItem* item = m_map->getItemFromCoordinate(it, lat, lon, closeItems,
                                             rec[4].c_str(), -1);
   if (item != NULL) {

      if (m_checkRecords) {
         // the record is found
         setRecordFoundAndAddMapid(true, m_recordId);
      }
   
      int counter = OldExtraDataUtility::removeNameFromItem(
                                 m_map, item, it, rec[4], nt, lang,
                                 m_affectsStreets);
      if (counter == -1) {
         mc2log << error << here << " removeNameFromItem(" << m_map << ", "
                << (int)it << ", " << lat << ", " << ", " << lon << rec[4]
                << ", " << (int)nt << ", " << (int)lang << ")" << endl;
         return false;
      } else if (counter > 0) {
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber
                << " Removed name '" << rec[4] << "' from " << counter 
                << " item(s)" << endl;
         if (m_applyOnMergedMaps) {
            m_affectsMap.push_back(item->getID());
         }
         if (m_checkRecords) {
            // the record is needed
            setRecordNeeded(true, m_recordId);
         }
      } else if (counter == 0 ) {
         mc2dbg4 << "Counter=0 for " << rec[4] << endl;
         // Something was wrong with the record, item was found, but record
         // was not applied. Maybe the lang name was not correct?
         // Write to missing file if checking ED
         if (m_checkRecords) {
            setRecordFoundAndAddMapid(false, m_recordId);
            setRecordNeeded(false, m_recordId);
            checkAndAddMapCandidate(it, lat, lon, m_recordId);
         }
      }
   } else {
      MC2BoundingBox bbox;
      m_map->getGfxData()->getMC2BoundingBox(bbox);
      if (bbox.contains(lat, lon)) {
         mc2log << warn << here 
                << " Possible error: didn't find any item at (" << lat
                << "," << lon << ") called \"" << rec[4] << "\"" << endl;
      }
      if (m_checkRecords) {
         // if there is an item but not with this name,
         // the record is found but not needed
         item =  m_map->getItemFromCoordinate(it, lat, lon, closeItems,
                                              NULL, -1);
         if (item != NULL) {
            setRecordFoundAndAddMapid(true, m_recordId);
            return true;
         } else {
            // check if the coordinate fits this map
            checkAndAddMapCandidate(it, lat, lon, m_recordId);
         }
      }
   }

   return true;
}


bool
ExtraDataReader::handleAddSignpostRecord(vector<MC2String>& rec,
                                         bool parseOnly)
{
   mc2dbg4 << "ADD_SIGNPOST" << endl;
   // ADD_SIGNPOST-record definition:
   // 1. (lat1,lon1) FromNode
   // 2. (lat2,lon2) FromNode
   // 3. (lat1,lon1) ToNode
   // 4. (lat2,lon2) ToNode
   // 5. signpost text
   // 6. language
   // 7. type

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   OldNode* fromNode = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   OldNode* toNode = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[3], rec[4]);
   LangTypes::language_t lang = getLanguage(rec[6]);
   ItemTypes::name_t nt = getNameType(rec[7]);

   // Check the indata
   if ( (rec.size() != 8) || (rec[5].size() == 0)) {
      mc2log << error << "Error in ADD_SIGNPOST record, text="
             << rec[5] << " name_t="
             << ItemTypes::getNameTypeAsString(nt) << " lang_t="
             << LangTypes::getLanguageAsString(lang) << ": " ;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // On this map?!
   if ( (fromNode == NULL) || (toNode == NULL) ) {
      if (m_checkRecords) {
         // check if this map is a candidate, before returning
         if ((fromNode != NULL) || (toNode != NULL)) {
            addMapId(m_map->getMapID(), m_recordId, true);
         } else {
            // use coords from rec3 (=toNode)
            int32 lat, lon;
            if (OldExtraDataUtility::strtolatlon(
                     rec[3], lat, lon, CoordinateTransformer::mc2)) {
               checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                       lat, lon, m_recordId);
            }
         }
      }
      return false;
   }

   // Make update
   int counter = addSignpost(fromNode, toNode,
                             rec[5], lang, nt, m_checkRecords);
   if (counter == -1) {
      mc2log << error << "Failed to add signpost " << endl;
      if (m_checkRecords) {
         // counter -1 means that connection was not found
         // but the nodes exist in this map (mapcandidate),
         addMapId(m_map->getMapID(), m_recordId, true);
      }
      return false;
   } else if (counter == 0) {
      mc2dbg2 << "The signpost already exists '" << rec[5] << "'" << endl;
      if (m_checkRecords) {
         // record is found, but not needed
         setRecordFoundAndAddMapid(true, m_recordId);
      }
   } else if (counter > 0) {
      // conn found, sp "added"
      if (m_checkRecords) {
         // record is found and needed
         setRecordFoundAndAddMapid(true, m_recordId);
         setRecordNeeded(true, m_recordId);
      } else {
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber
                << " Added Signpost, '" << rec[5] << "' to map " 
                << m_map->getMapID() << endl;
         if (m_applyOnMergedMaps)
            m_affectsMap.push_back(toNode->getNodeID() & ITEMID_MASK);
      }
   }

   return true;
}

bool
ExtraDataReader::handleRemoveSignpostRecord(vector<MC2String>& rec, 
                                            bool parseOnly)
{
   mc2dbg4  << "REMOVE_SIGNPOST" << endl;
   // REMOVE_SIGNPOST-record definition:
   // 1. (lat1,lon1) FromNode
   // 2. (lat2,lon2) FromNode
   // 3. (lat1,lon1) ToNode
   // 4. (lat2,lon2) ToNode
   // 5. signpost text
   // 6. language
   // 7. type

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   OldNode* fromNode = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   OldNode* toNode = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[3], rec[4]);
   LangTypes::language_t lang = getLanguage(rec[6]);
   ItemTypes::name_t nt = getNameType(rec[7]);

   // Check the indata
   if ( (rec.size() != 8) || (rec[5].size() == 0)) {
      mc2log << error << "Error in REMOVE_SIGNPOST record, text="
             << rec[5] << " name_t="
             << ItemTypes::getNameTypeAsString(nt) << " lang_t="
             << LangTypes::getLanguageAsString(lang) << ": " ;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // On this map?!
   if ( (fromNode == NULL) || (toNode == NULL) ) {
      if (m_checkRecords) {
         // check if this map is a candidate, before returning
         if ((fromNode != NULL) || (toNode != NULL)) {
            addMapId(m_map->getMapID(), m_recordId, true);
         } else {
            // use coords from rec3 (=toNode)
            int32 lat, lon;
            if (OldExtraDataUtility::strtolatlon(
                     rec[3], lat, lon, CoordinateTransformer::mc2)) {
               checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                       lat, lon, m_recordId);
            }
         }
      }
      return false;
   }


   // Make update
   OldConnection* con = getConnection(fromNode, toNode);
   bool retVal = false;
   if (con != NULL) {
      char* spText = StringUtility::newStrDup(rec[5].c_str());


      if (toNode->removeSignPost(*m_map, con->getFromNode(), spText) ){
         if (m_checkRecords) {
            // here is a signpost to remove (record found and needed)
            setRecordFoundAndAddMapid(true, m_recordId);
            setRecordNeeded(true, m_recordId);
            retVal = true;
         } 
         else {
            m_nbrRecordsAdded += 1;
            m_recordsAddedToMaps.push_back(m_curLineNumber);
            mc2log << info << "EDR rec " << m_curLineNumber
                   << " Signpost " << rec[5] << " removed from map " 
                   << m_map->getMapID() << endl;
            retVal = true;
            if (m_applyOnMergedMaps)
               m_affectsMap.push_back(toNode->getNodeID() & ITEMID_MASK);
         }
      }
      if (m_checkRecords) {
         // there was no signPost to remove from this conn,
         // (record found but not needed)
         setRecordFoundAndAddMapid(true, m_recordId);
         retVal = true;
      }
   }
   else if (m_checkRecords) {
      // no connection found between the nodes, however the
      // nodes are in this map (mapcandidate)
      addMapId(m_map->getMapID(), m_recordId, true);
   }
   return retVal;
}


bool
ExtraDataReader::handleAddNameToItemRecord(
                        vector<MC2String>& rec, bool parseOnly)
{
   mc2dbg4 << "ADD_NAME_TO_ITEM" << endl;

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get indata
   ItemTypes::itemType it = getItemType(rec[1]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[2]);
   int32 lat, lon;
   bool coordOK = OldExtraDataUtility::strtolatlon(rec[3],lat, lon, coordType);
   ItemTypes::name_t newNameType = getNameType(rec[8]);
   LangTypes::language_t newNameLanguage = getLanguage(rec[9]);

   // Check the indata
   if ( (rec.size() != 10) ||
        (rec[7].size() == 0) ||
        (it == ItemTypes::nullItem) ||
        (coordType == CoordinateTransformer::nbrCoordinateTypes) ||
        (!coordOK) ) {
      mc2log << error << "Error in ADD_NAME_TO_ITEM record it="
             << StringTable::getString(ItemTypes::getItemTypeSC(it),
                                       StringTable::ENGLISH) << " nnt="
             << ItemTypes::getNameTypeAsString(newNameType) << " nlang="
             << LangTypes::getLanguageAsString(newNameLanguage);
      if (coordType != CoordinateTransformer::nbrCoordinateTypes) 
         mc2dbg1 << " cordType OK";
      else 
         mc2dbg1 << " cordType NOT OK";
      if (coordOK) 
         mc2dbg1 << " cord OK" << endl;
      else 
         mc2dbg1 << " cord NOT OK" << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }
   
   if ( m_applyOnMergedMaps &&
        ((it == ItemTypes::municipalItem) ||
         (it == ItemTypes::builtUpAreaItem)) &&
        ((m_edInsertType.size() == 0) || 
         (strcasecmp(m_edInsertType.c_str(),
                     "beforeInternalConnections") == 0)) ) {
      // Admin areas affecting merging and/or location-groups setting
      // (i.e. bua and mun) are (for now) not handled when applying 
      // extra data to merged maps.
      mc2log << warn << "For merged maps, record " << m_curLineNumber 
             << " ADD_NAME_TO_ITEM (" << ItemTypes::getItemTypeAsString(it) 
             << ") not handled" << endl;
      return true;
   }
   
   // Add the name to the item
   // First check whether the item in question has a name or if NULL
   // should be sent to the getItemFromCoordinate function.
   char* oldName = StringUtility::newStrDup(rec[4].c_str());
   if (strlen(oldName) == 0)  {
      oldName = NULL;
   }
  
   mc2dbg8 << "Tries to add name: " << rec[7] << " at (" << lat << "," 
           << lon << ")" << endl;
   
   vector<OldItem*> closeItems;
   OldItem* item = m_map->getItemFromCoordinate(it, lat, lon, closeItems,
                                             oldName, -1);
   delete [] oldName;
  
   
   if (item != NULL) {
      // item found
      if (m_checkRecords) {
         setRecordFoundAndAddMapid(true, m_recordId);
      }

      // If name already exists, don't add it again.
      uint32 strIdx;
      bool itemHasName = m_map->itemHasNameAndType(
               item, newNameLanguage, newNameType, rec[7].c_str(), strIdx);

      if ( itemHasName ) {
         mc2log << warn << "In addNameToItem: item already has this name: "
                << rec[7] << endl;
         
      } else {

         if ((newNameType == ItemTypes::officialName) &&
             (item->getNameWithType(
                       newNameType, newNameLanguage) != MAX_UINT32)) {
            // The item already has a name (another) with type=officialName
            mc2log << warn << here
                   << " Item already has a name with name type=officialName"
                   << " and language=" << rec[9] << ", trying to add '"
                   << rec[7] << "'" << endl;
            return false;
         }
         
         int counter = OldExtraDataUtility::addNameToItem(
                                       m_map, item, it,
                                       rec[7], newNameType, newNameLanguage,
                                       m_affectsStreets);
         if (counter == -1) {
            mc2log << error << here << " addNameToItem(" << m_map << ", "
                   << (int)it << ", " << lat << ", " << ", " << lon << rec[4]
                   << ", " << (int)newNameType << ", " << (int)newNameLanguage
                   << ", " << rec[7] << ")" << endl;
            return false;
         } else if (counter > 0) {
            m_nbrRecordsAdded += 1;
            m_recordsAddedToMaps.push_back(m_curLineNumber);
            mc2log << info << "EDR rec " << m_curLineNumber
                   << " Added name '" << rec[7] << "' to " << counter 
                   << " item(s)" << endl;
            mc2dbg4 << "LOC:" << m_map->getFirstItemName(
                    m_map->getRegionID( item, ItemTypes::municipalItem )) 
                    << ";"
                    << m_map->getFirstItemName(
                     m_map->getRegionID( item, ItemTypes::builtUpAreaItem ))
                    << ";"
                    << rec[7] << endl;
            if (m_applyOnMergedMaps) {
               m_affectsMap.push_back(item->getID());
            }
            if (m_checkRecords) {
               setRecordNeeded(true, m_recordId);
            }
         } else if (counter == 0 ) {
            mc2dbg4 << "Counter=0 for " << rec[7] << endl;
         }

      }
      
   } else {
      DEBUG4(
      MC2BoundingBox bbox;
      m_map->getGfxData()->getMC2BoundingBox(bbox);
      if (bbox.contains(lat, lon)) {
         mc2log << warn << here 
                << " Possible error, no item for ed rec "
                << m_curLineNumber << " at (" << lat
                << "," << lon << ") called \"" << rec[4] << "\"" << endl;
      });
      if (m_checkRecords) {
         // check if this map is a candidate for this record
         checkAndAddMapCandidate(it, lat, lon, m_recordId);
      }
   }

   return true;
}
 

bool
ExtraDataReader::handleAddCitypartRecord(vector<MC2String>& rec,
                                         bool parseOnly)
{
   mc2dbg4 << "ADD_CITYPART:" << endl;

   if (m_checkRecords) {
      // This record can not be checked (recordChecked = false)
      return true;
   }
   if (m_applyOnMergedMaps) {
      // This record type is not handled when applying extra data
      // to merged maps.
      mc2log << warn << "For merged maps, record " << m_curLineNumber 
             << " ADD_CITYPART not handled" << endl;
      return true;
   }

   // Get indata
   int32 lat, lon;
   bool coordinateOK = OldExtraDataUtility::strtolatlon(
                           rec[1], lat, lon, CoordinateTransformer::mc2);
   ItemTypes::name_t nt = getNameType(rec[2]);
   LangTypes::language_t lang = getLanguage(rec[3]);
   uint32 nbrCoords = getUint32(rec[5]);

   // Check indata
   if ( (nt == ItemTypes::invalidName) ||
        (lang == LangTypes::invalidLanguage) ||
        (nbrCoords < 2) ||
        (!coordinateOK)) {
      mc2log << error << "Error in ADD_CITYPART record nt="
             << ItemTypes::getNameTypeAsString(nt) << " nlang="
             << LangTypes::getLanguageAsString(lang);
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // Check if on this map
   if (m_map->getGfxData()->insidePolygon(lat, lon) == 0) {
      mc2dbg4 << "ADD_CITYPART returns false due to outsid current map" 
              << endl;
      return false;
   }
   
   // Create a "good" name (without any quote-characters).
   char* theNewName = StringUtility::newStrDup(rec[4].c_str());
   StringUtility::replaceChar(theNewName, '"', ' ');
   StringUtility::replaceChar(theNewName, '\'', ' ');
   StringUtility::trimEnd(theNewName);
   char* namePointer = StringUtility::trimStart(theNewName);

   // Check that the city part was not already added....
   vector<OldItem*> foo;
   OldItem* tmpItem = m_map->getItemFromCoordinate(
               ItemTypes::cityPartItem, lat, lon, foo, namePointer, 100);
   if (tmpItem != NULL) {
      mc2log <<  warn << "AddCityPart: city part " << namePointer
             << " already exists in map " << m_map->getMapID() << endl;
      return false;
   }
   
   
   // Create GfxData and fill it with coordinates
   GfxDataFull* curGfx = GMSGfxData::createNewGfxData(NULL, true);
   const int firstCoordPos = 6;
   for (uint32 i = 0; i < nbrCoords; i++){
      if (!OldExtraDataUtility::strtolatlon(rec[firstCoordPos+i], lat, lon,
                                         CoordinateTransformer::mc2)){
         mc2log << error << here << " Failed to read coordinate: " 
                << rec[firstCoordPos+i] << ", lat=" << lat 
                << ", lon=" << lon << endl;
         delete curGfx;
         return false;
      }
      if (!curGfx->addCoordinate(lat, lon)){
         mc2log << error << " Failed to add coordinate: lat=" << lat 
                << ", lon=" << lon << endl;
         delete curGfx;
         return false;
      }
   }
   curGfx->setClosed(0, true);
   curGfx->removeIdenticalCoordinates();
   curGfx->updateLength();


   // Find a bua for the city part, using the centre point of the cp gfxdata
   float64 sqMinDist = MAX_FLOAT64;
   OldBuiltUpAreaItem* curBuaItem = NULL;
   MC2BoundingBox cpBBox;
   curGfx->getMC2BoundingBox( cpBBox );
   int32 cpLat, cpLon;
   curGfx->getPolygonCentroid( 0, cpLat, cpLon );
   // Try to find closest bua
   for(uint32 i = 0; i < m_map->getNbrItemsWithZoom( 0 ); ++i) {
      OldBuiltUpAreaItem* bua = 
         dynamic_cast<OldBuiltUpAreaItem*> (m_map->getItem(0, i));
      if ( ( bua != NULL ) &&
           ( bua->getGfxData() != NULL ) ) {
         GfxData* buaGfx = bua->getGfxData();
         MC2BoundingBox buaBBox;
         buaGfx->getMC2BoundingBox( buaBBox );
         // Check that bboxes overlaps first
         if ( cpBBox.overlaps( buaBBox ) ) {
            mc2dbg8 << " trying bua " << m_map->getFirstItemName(bua) << endl;
            for ( uint32 j = 0; j < buaGfx->getNbrPolygons(); ++j ) {
               int32 buaLat, buaLon;
               // Check if distance between CoG of citypart and bua
               // is smallest so far.
               buaGfx->getPolygonCentroid( j, buaLat, buaLon );
               float64 sqDist = GfxUtility::squareP2Pdistance_linear(
                     cpLat, cpLon, buaLat, buaLon );
               if ( sqDist < sqMinDist ) {
                  // Update minimum distance.
                  sqMinDist = sqDist;
                  // Set this as the best bua so far.
                  curBuaItem = bua;
               }
            }
         }
      }
   }

   if (curBuaItem == NULL) {
      mc2dbg4 << "Did not find any builtUpArea (in this map)"
              << " for this city part" << endl;
      return false;
   }
   
   // Else, this is the correct map for the city part
   // Please add it
   
   // Create CityPart, use dummy-id to initialize
   OldCityPartItem* curCityPart = new OldCityPartItem(MAX_UINT32);
   // Add gfxdata to new OldItem
   curCityPart->setGfxData(curGfx);
   // Add city part to the map
   uint32 id = m_map->addItem(curCityPart, ItemTypes::zoomConstCPI);
   if (id == MAX_UINT32) {
      mc2log << error << "Failed to add citypart ed rec "
             << m_curLineNumber << endl;
      return false;
   }
   
   // Set name
   uint32 index = m_map->addNameToItem(curCityPart, namePointer, lang, nt);
   delete [] theNewName;
   theNewName = NULL;
   if (index == MAX_UINT32) {
      mc2log << error << "Failed to add name to citypart ed rec "
             << m_curLineNumber << endl;
      return false;
   }

   // Print debug info about the ed addition
   m_nbrRecordsAdded += 1;
   m_recordsAddedToMaps.push_back(m_curLineNumber);
   mc2log << info << "EDR rec " << m_curLineNumber << " Added city part "
          << m_map->getFirstItemName(curCityPart) 
          << " to map " << m_map->getMapID() << endl;

   
   // Add citypart to the bua, and the bua to the citypart
   mc2dbg4 << "BUA id: " << curBuaItem->getID() << endl;
   if (!curBuaItem->addItem(id)){
      mc2dbg2 << "Citypart not added to BUA" << endl;
      return false;
   } else {
      mc2log << info << "Citypart, " << rec[4] << ", added to BUA, " 
             << m_map->getFirstItemName(curBuaItem) << endl;
      DEBUG4(
         cout << "Citypart added to BUA ";
         m_map->printItemNames(curBuaItem->getID());
         cout << ", has " << (uint32) curBuaItem->getNbrItems()
              << " groups" << endl; 
      );
   }
   if (!m_map->addRegionToItem(curCityPart, curBuaItem)){
      mc2log << error << here << " BUA not added to citypart" << endl;
      return false;
   }

   
   // Indicate that the citypart location should be updated
   // afterwards.
   m_updateCityPartLocation = true;

   return true;
}



bool
ExtraDataReader::handleSetVehicleRestrictionsRecord(vector<MC2String>& rec, 
                                          bool parseOnly)
{
   // SET_VEHICLERESTRICTIONS-record definition:
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. (lat1,lon1)
   // 4. (lat2,lon2)
   // 5. value
   
   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   OldNode* fromNode = OldExtraDataUtility::getNodeFromStrings(
                        m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   OldNode* toNode = OldExtraDataUtility::getNodeFromStrings(
                        m_map, CoordinateTransformer::mc2, rec[3], rec[4]);
   char* s = StringUtility::newStrDup(rec[5].c_str());
   uint32 val = strtoul(s, NULL, 0);
   OldConnection* con = getConnection(fromNode, toNode);

   mc2dbg4 << "Got vehicle restrictions, s=\"" << s << "\"" << endl;
   if ( (fromNode != NULL) && (toNode != NULL) && (con != NULL) ) {
      if ( StringUtility::onlyDigitsInString(s) ||
           (strlen(s) <= 2) || (s[0] != '0') || 
           (toupper(s[1]) != 'X') ) {
         mc2log << error << "Error in SET_VEHICLERESTRICTIONS-record ";
         printRecord(rec);
         delete [] s;
         return false; 
      }
   }
   delete [] s;
  
   if (parseOnly)
      return true;
   
   bool retVal = false;
   if ( (fromNode != NULL) && (toNode != NULL) && (con != NULL) ) {
      mc2dbg4 << "Got vehicle restrictions, val=0x" << hex 
              << val << dec << endl;

      if (m_checkRecords) {
         // record found
         setRecordFoundAndAddMapid(true, m_recordId);
         // Check origval against the map
         if ((m_originalValStr != "") &&
             (strlen(m_originalValStr.c_str()) > 0)) {
            uint32 origVal = strtoul(m_originalValStr.c_str(), NULL, 0);
            mc2dbg4 << " " << m_recordId << ": origVR=0x" << hex << origVal 
                    << " (str=" << m_originalValStr << ") mapVR=0x" 
                    << con->getVehicleRestrictions() << " (edVR=0x" << val
                    << ")" << dec << endl;
            if (origVal != con->getVehicleRestrictions())
               setRecordMapChanged(true, m_recordId);
         }
      }
      if (con->getVehicleRestrictions() != val) {
         if (m_checkRecords) {
            // If the values differ the record is needed
            // (check only pedestrian an passengerCar since those are
            //  the ones used uptil now 20030813 + 
            //  if the map says noVR and the record says some VR)
            bool valPed = ( (uint32(ItemTypes::pedestrian) & val) != 0);
            bool valCar = ( (uint32(ItemTypes::passengerCar) & val) != 0);
            if ( (con->isVehicleAllowed(ItemTypes::pedestrian) != valPed) ||
                 (con->isVehicleAllowed(ItemTypes::passengerCar) != valCar) ||
                 ((val != 0xffffffff) && 
                  (con->getVehicleRestrictions() == 0xffffffff)) ) {
               // record needed
               setRecordNeeded(true, m_recordId);
            }
         }

         con->setVehicleRestrictions(val);
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber 
                   << " Vehicle restrictions set to 0x"
                   << hex << val << dec << " "
                   << m_map->getMapID() << ":" << fromNode->getNodeID()
                   << ":" << toNode->getNodeID() << endl;
         m_affectsTurnDescr.push_back(toNode->getNodeID());
         
      }
      
      retVal = true;
      
   } // else not this map (or changed geometry)
   else if (m_checkRecords){
      // check if this map is a candidate for this record
      if ((fromNode != NULL) || (toNode != NULL)) {
         addMapId(m_map->getMapID(), m_recordId, true);
      } else {
         // use coords from rec3 (=toNode)
         int32 lat, lon;
         if (OldExtraDataUtility::strtolatlon(
                  rec[3], lat, lon, CoordinateTransformer::mc2)) {
            checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                    lat, lon, m_recordId);
            // XXX ssi or ferry ??
         }
      }
   }
   
   return retVal;
}

bool
ExtraDataReader::handleAddLandmarkRecord(vector<MC2String>& rec,
                                         bool parseOnly)
{
   mc2dbg4 << "ADD_LANDMARK" << endl;

   if (m_checkRecords) {
      // This record can not be checked (recordChecked = false)
      return true;
   }
   if (m_applyOnMergedMaps) {
      // This record type is not handled when applying extra data
      // to merged maps.
      mc2log << warn << "For merged maps, record " << m_curLineNumber 
             << " ADD_LANDMARK not handled" << endl;
      return true;
   }
   
   // Get indata
   ItemTypes::itemType it = ItemTypes::getItemTypeFromString(rec[1].c_str());
   ItemTypes::pointOfInterest_t pt = getPOIType(rec[2]);
   uint32 importance = getUint32(rec[3]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[4]);
   int32 lat, lon, itemlat, itemlon;
   bool coordOK = 
      OldExtraDataUtility::strtolatlon(rec[5], lat, lon, coordType);
   uint32 radius = getUint32(rec[6]);
   OldExtraDataUtility::selectConn_t selConns = 
      OldExtraDataUtility::getSelectConns(rec[7]);
   bool itemcoordOK = 
      OldExtraDataUtility::strtolatlon(rec[8], itemlat, itemlon, coordType);
   

   // Check indata
   if (it == ItemTypes::numberOfItemTypes) {
      mc2log << error << "Wrong itemtype: " << rec[1] << endl;
      return false;
   }
   if ((it == ItemTypes::pointOfInterestItem) && 
       (pt == ItemTypes::unknownType)) {
      mc2log << error << "Wrong pointOfInterest type: " << rec[2] << endl;
      return false;
   }
   if (!coordOK) {
      mc2log << error << "Error in coordinate: " << rec[5] << endl;
      return false;
   }
   if (!itemcoordOK && (it != ItemTypes::railwayItem)) {
      mc2log << error << "Error in itemcoordinate: " << rec[8] << endl;
      return false;
   }
   if ((importance == MAX_UINT32) || (radius == MAX_UINT32)) {
      mc2log << error << "Error in importance: " << rec[3] 
                      << " or radius: " << rec[6] << endl;
      return false;
   }
   if (selConns == OldExtraDataUtility::NOCONN) {
      mc2log << error << "Error in connections selection: " 
             << rec[7] << endl;
      return false;
   }

   // Indata OK
   if (parseOnly) {
      return true;
   }

   // Add indata
   int counter = OldExtraDataUtility::addLandmark(m_map, it, pt, importance,
         lat, lon, radius, itemlat, itemlon, rec[9].c_str(), selConns);
   if (counter == -1) {
      return false;
   } else if (counter > 0) {
      m_nbrRecordsAdded += 1;
      m_recordsAddedToMaps.push_back(m_curLineNumber);
      mc2log << info << "EDR rec " << m_curLineNumber 
                     << " Added the landmark " << rec[9].c_str() << " to " 
                     << counter << " connections." << endl;
   }

   return true;
}

bool
ExtraDataReader::handleAddSingleLandmarkRecord(vector<MC2String>& rec, 
                                            bool parseOnly)
{
   mc2dbg4 << "ADD_SINGLE_LANDMARK" << endl;

   if (m_checkRecords) {
      // This record can not be checked (recordChecked = false)
      return true;
   }
   if (m_applyOnMergedMaps) {
      // This record type is not handled when applying extra data
      // to merged maps.
      mc2log << warn << "For merged maps, record " << m_curLineNumber 
             << " ADD_SINGLE_LANDMARK not handled" << endl;
      return true;
   }

   // Get indata
   ItemTypes::itemType it = ItemTypes::getItemTypeFromString(rec[1].c_str());
   ItemTypes::pointOfInterest_t pt = getPOIType(rec[2]);
   uint32 importance = getUint32(rec[4]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[5]);

   int32 itemlat, itemlon;
   bool itemcoordOK = 
      OldExtraDataUtility::strtolatlon(rec[6], itemlat, itemlon, coordType);

   int location = ItemTypes::getLandmarkLocationFromString(rec[7].c_str());
   int side = ItemTypes::getLandmarkSideFromString(rec[8].c_str());
   
   
   // Check indata
   if (it == ItemTypes::numberOfItemTypes) {
      mc2log << error << "Wrong itemtype: " << rec[1] << endl;
      return false;
   }
   if ((it == ItemTypes::pointOfInterestItem) && 
       (pt == ItemTypes::unknownType)) {
      mc2log << error << "Wrong pointOfInterest type: " << rec[2] << endl;
      return false;
   }
   if (!itemcoordOK ) {
      mc2log << error << "Error in itemcoordinate: " << rec[6] << endl;
      return false;
   }
   if (importance == MAX_UINT32) {
      mc2log << error << "Error in importance: " << rec[4] << endl;
      return false;
   }
   if (location == -1)
      mc2log << warn << "No location given: " << rec[7] << endl;
   if (side == -1)
      mc2log << warn << "No side given: " << rec[8] << endl;

   
   // Indata OK
   if (parseOnly) {
      return true;
   }
   
   // Get the nodes and the landmark item
   OldNode* fromNode = OldExtraDataUtility::getNodeFromStrings(
                        m_map, coordType, rec[9], rec[10]);
   OldNode* toNode = OldExtraDataUtility::getNodeFromStrings(
                        m_map, coordType, rec[11], rec[12]);
   OldConnection* conn = getConnection(fromNode, toNode);

   vector<OldItem*> closeItems;
   OldItem* lmItem = m_map->getItemFromCoordinate(
         it, itemlat, itemlon, closeItems, rec[3].c_str(), -1, pt);

   
   // If the nodes/connection is NULL, probably another map
   // If not NULL, check that the landmark item was found
   if ( (fromNode != NULL) && (toNode != NULL) && (conn != NULL) ) {
      if (lmItem == NULL) {
         mc2log << error << "The landmark item not found, " 
                         << rec[3].c_str() << endl;
         return false;
      }
   }
   
   // Add indata
   if ( (fromNode != NULL) && (toNode != NULL) && (conn != NULL) ) {
      bool added = OldExtraDataUtility::addSingleLandmark(
            m_map, lmItem, importance, location, side, fromNode, toNode);
      if (!added) {
         return false;
      } else {
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber 
                        << " Added the single landmark " << rec[3].c_str() 
                        << " to the map." << endl;
      }
   }

   return true;
}


bool
ExtraDataReader::handleSingleBooleanRecord(vector<MC2String>& rec, 
                                           bool parseOnly)
{
   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get indata
   OldNode* node = OldExtraDataUtility::getNodeFromStrings(
                        m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   bool val = false;
   if ( (rec[3][0] == 'T') || (rec[3][0] == 't'))
      val = true;
   else if ( (rec[3][0] == 'F') || (rec[3][0] == 'f'))
      val = false;
   else {
      mc2log << error << " Invalid boolean-value: " << rec[3] << endl;
      return false;
   }

   // Check indata
   if ( (rec.size() != 4) ||
        (rec[1].size() == 0) ||
        (rec[2].size() == 0) ) {
      mc2log << error << "Error in BOOLEAN-record ";
      printRecord(rec);
      return false; 
   }

   // Indata OK!
   if (parseOnly) {
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
                        rec[1], lat, lon, CoordinateTransformer::mc2))
         return OldExtraDataUtility::strtolatlon(rec[2], lat, lon,
                                              CoordinateTransformer::mc2);
      return false;
   }

   // Add to the node. If NULL, probably located on other map.
   if (node != NULL) {

      if (m_checkRecords) {
         setRecordFoundAndAddMapid(true, m_recordId);
      }
      
      OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
                  (m_map->itemLookup(node->getNodeID() & ITEMID_MASK));
      switch (OldExtraDataUtility::getRecordType(rec[0])) {
         
         case OldExtraDataUtility::SET_MULTIDIGITISED :
            if (m_checkRecords) {
               if (ssi->isMultiDigitised() != val) {
                  // record found and needed
                  setRecordNeeded(true, m_recordId);
               } else {
                  // record found but not needed and map changed
                  setRecordMapChanged(true, m_recordId);
               }
               return true;
            }
            if (ssi->isMultiDigitised() != val) {
               ssi->setMultiDigitisedValue(val);
               m_nbrRecordsAdded += 1;
               m_recordsAddedToMaps.push_back(m_curLineNumber);
               mc2log << info << "EDR rec " << m_curLineNumber 
                      << " Multi.dig val set to "
                      << getStringFromBool(val) << " for " 
                      << m_map->getMapID() << ":" << ssi->getID()
                      << " name=" << m_map->getFirstItemName(ssi) << endl;
               m_affectsTurnDescr.push_back(node->getNodeID());
            }
            return true;
            
         case OldExtraDataUtility::SET_ROUNDABOUT:
            if (m_checkRecords) {
               if ( (ssi->isRoundabout() != val) &&
                    !((val == true) && ssi->isRoundaboutish()) ) {
                  // don't use old records setRbt when it is roundaboutish
                  // record found and needed
                  setRecordNeeded(true, m_recordId);
               } else {
                  // record found but not needed and map changed
                  setRecordMapChanged(true, m_recordId);
               }
               return true;
            }
            if (ssi->isRoundabout() != val) {
               ssi->setRoundaboutValue(val);
               m_nbrRecordsAdded += 1;
               m_recordsAddedToMaps.push_back(m_curLineNumber);
               mc2log << info << "EDR rec " << m_curLineNumber 
                      << " Roundabout val set to "
                      << getStringFromBool(val) << " for " 
                      << m_map->getMapID() << ":" << ssi->getID()
                      << " name=" << m_map->getFirstItemName(ssi) << endl;
               m_affectsTurnDescr.push_back(node->getNodeID());
            }
            return true;
            
         case OldExtraDataUtility::SET_ROUNDABOUTISH:
            if (m_checkRecords) {
               if ( ssi->isRoundaboutish() != val ) {
                  // record found and needed
                  setRecordNeeded(true, m_recordId);
               } else {
                  // record found but not needed and map changed
                  setRecordMapChanged(true, m_recordId);
               }
               return true;
            }
            if (ssi->isRoundaboutish() != val) {
               ssi->setRoundaboutishValue(val);
               m_nbrRecordsAdded += 1;
               m_recordsAddedToMaps.push_back(m_curLineNumber);
               mc2log << info << "EDR rec " << m_curLineNumber 
                      << " Roundaboutish val set to "
                      << getStringFromBool(val) << " for " 
                      << m_map->getMapID() << ":" << ssi->getID()
                      << " name=" << m_map->getFirstItemName(ssi) << endl;
               m_affectsTurnDescr.push_back(node->getNodeID());
            }
            return true;
            
         case OldExtraDataUtility::SET_RAMP:
            if (m_checkRecords) {
               if (ssi->isRamp() != val) {
                  // record found and needed
                  setRecordNeeded(true, m_recordId);
               } else {
                  // record found but not needed and map changed
                  setRecordMapChanged(true, m_recordId);
               }
               return true;
            }
            if (ssi->isRamp() != val) {
               ssi->setRampValue(val);
               m_nbrRecordsAdded += 1;
               m_recordsAddedToMaps.push_back(m_curLineNumber);
               mc2log << info << "EDR rec " << m_curLineNumber 
                      << " Ramp val set to "
                      << getStringFromBool(val) << " for " 
                      << m_map->getMapID() << ":" << ssi->getID()
                      << " name=" << m_map->getFirstItemName(ssi) << endl;
               m_affectsTurnDescr.push_back(node->getNodeID());
            }
            return true;
            
         case OldExtraDataUtility::SET_CONTROLLEDACCESS:
            if (m_checkRecords) {
               if ( ssi->isControlledAccess() != val ) {
                  // record found and needed
                  setRecordNeeded(true, m_recordId);
               } else {
                  // record found but not needed and map changed
                  setRecordMapChanged(true, m_recordId);
               }
               return true;
            }
            if (ssi->isControlledAccess() != val) {
               ssi->setControlledAccessValue(val);
               m_nbrRecordsAdded += 1;
               m_recordsAddedToMaps.push_back(m_curLineNumber);
               mc2log << info << "EDR rec " << m_curLineNumber 
                      << " Controlled access val set to "
                      << getStringFromBool(val) << " for " 
                      << m_map->getMapID() << ":" << ssi->getID()
                      << " name=" << m_map->getFirstItemName(ssi) << endl;
               m_affectsTurnDescr.push_back(node->getNodeID());
            }
            return true;
            
         default :
            mc2log << error << here << " Record-type not handled" << endl;
      }
   }
   else if (m_checkRecords){
      // use coords from rec1 (=OldNode)
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
               rec[1], lat, lon, CoordinateTransformer::mc2)) {
         checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                 lat, lon, m_recordId);
      }
   }
   return false;
} // handleSingleBooleanRecord

bool
ExtraDataReader::handleRemoveItemRecord(vector<MC2String>& rec,
                                        bool parseOnly)
{
   mc2dbg4 << "REMOVE_ITEM" << endl;
   
   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   ItemTypes::itemType it = getItemType(rec[1]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[2]);
   int32 lat, lon, mapssilat, mapssilon;
   bool coordOK = OldExtraDataUtility::strtolatlon(rec[3], lat, lon, coordType);
   // A coordinate could be provided to 
   bool mapcoordOK = false;
   if (rec.size() == 6) {
      mapcoordOK = OldExtraDataUtility::strtolatlon(
            rec[5], mapssilat, mapssilon, coordType);
   }

   // Check indata
   if ( !((rec.size() == 5) || (rec.size() == 6)) ||
        (it == ItemTypes::nullItem) ||
        (coordType == CoordinateTransformer::nbrCoordinateTypes) ||
        (!coordOK) || 
        ((rec.size() == 6) && !mapcoordOK) ) {
      mc2log << error << "Error in REMOVE_ITEM record it="
             << StringTable::getString(ItemTypes::getItemTypeSC(it),
                                       StringTable::ENGLISH);
      if (coordType != CoordinateTransformer::nbrCoordinateTypes) 
         mc2log << ", cordType OK";
      else 
         mc2log << ", cordType NOT OK";
      if (coordOK) 
         mc2log << ", cord OK";
      else 
         mc2log << ", cord NOT OK";
      if (rec.size() == 6) {
         if (mapcoordOK)
            mc2log << ", mapssicoord OK";
         else
            mc2log << ", mapssicoord NOT OK";
      }
      mc2log << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // If a mapssi coordinate was given, check if this is the correct map
   if (mapcoordOK) {
      OldMapHashTable* mht = m_map->getHashTable();
      MC2_ASSERT(mht != NULL);
      mht->clearAllowedItemTypes();
      mht->addAllowedItemType(ItemTypes::streetSegmentItem);
      // Find closest street segment
      uint64 dist;
      uint32 closestSSI = mht->getClosest(mapssilon, mapssilat, dist);
      mc2dbg8 << "Found closest ssi " << closestSSI << " dist="
              << sqrt(dist)*GfxConstants::MC2SCALE_TO_METER 
              << " meters" << endl;

      if (sqrt(dist)*GfxConstants::MC2SCALE_TO_METER < 2) {
         // correct map
         mc2dbg8 << " -> correct map." << endl;
      } else {
         mc2dbg8 << " -> not the correct map." << endl;
         return true;
      }
   }
   
   // Check whether the item to be removed has a name or if NULL
   // should be sent to the getItemFromCoordinate function.
   char* name = StringUtility::newStrDup(rec[4].c_str());
   if (strlen(name) == 0)  {
      name = NULL;
   }
   
   vector<OldItem*> closeItems;
   OldItem* item = 
      m_map->getItemFromCoordinate(it, lat, lon, closeItems, name, -1);
   delete [] name;
   
   if (item != NULL) {
 
      if (m_checkRecords) {
         // record found
         setRecordFoundAndAddMapid(true, m_recordId);
      }

      // Ssi and street are (now) allowed when applying dynamic extra data
      if (m_applyOnMergedMaps &&
          (it == ItemTypes::streetSegmentItem) ) {
         mc2log << warn << "REMOVE_ITEM: Risk of removing POIs connected to "
                << "this " << ItemTypes::getItemTypeAsString(it) 
                << ", record " << m_curLineNumber << endl;
      }
   
      uint32 itemID = item->getID();
      bool removed = OldExtraDataUtility::removeItem(m_map, item);
      if (removed) {
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber 
                << " Removed item, id=" << itemID 
                << " name='" << rec[4].c_str() << "'" 
                << " from map=" << m_map->getMapID()
                << endl;
         if (m_applyOnMergedMaps) {
            m_affectsMap.push_back(item->getID());
            // does not have to add to the m_affectsStreets vector, since all
            // group references are taken care of in the removeItem method.
            // BUT! There might be empty streets left in the map when 
            // removing the last ssi in that street!
         }
         if (m_checkRecords) {
            // record needed
            setRecordNeeded(true, m_recordId);
         }
      }
      
   } else {
      MC2BoundingBox bbox;
      m_map->getGfxData()->getMC2BoundingBox(bbox);
      if (bbox.contains(lat, lon)) {
         mc2log << warn << here 
                << " Possible error: didn't find any item at (" << lat
                << "," << lon << ") called \"" << rec[4] << "\""
                << ", record " << m_curLineNumber << endl;
      }
      if (m_checkRecords) {
         // if mapssi coord was given, we are in the correct map (add cand)
         // else check if any item close to coordinate
         if (mapcoordOK) {
            addMapId(m_map->getMapID(), m_recordId, true);
         } else {
            checkAndAddMapCandidate(it, lat, lon, m_recordId);
         }
      }
   }

   return true;
}

bool
ExtraDataReader::handleSetHousenumberRecord(vector<MC2String>& rec,                                                         bool parseOnly)
{
   mc2dbg4 << "SET_HOUSENUMBER" << endl;
   // setHousenumber-record definition
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. leftNbrStart
   // 4. leftNbrEnd
   // 5. rightNbrStart
   // 6. rightNbrEnd
   // 7. Housenumber type
   
   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   OldNode* node = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   uint16 leftNbrStart = uint16(getUint32(rec[3]));
   uint16 leftNbrEnd = uint16(getUint32(rec[4]));
   uint16 rightNbrStart = uint16(getUint32(rec[5]));
   uint16 rightNbrEnd = uint16(getUint32(rec[6]));
   // Housenumbertype, also calculate from the numbers
   ItemTypes::streetNumberType type = 
      ItemTypes::streetNumberType(getUint32(rec[7]));

   // Check indata
   if ((rec[3].size() == 0) ||
       (rec[4].size() == 0) ||
       (rec[5].size() == 0) ||
       (rec[6].size() == 0) ||
       (int(type) < 0) || (int(type) > 3) ) {
      mc2log << error << "Error in SET_HOUSENUMBER record: ";
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // Update streetnumbers
   if (node != NULL) {
      // Is the streetnumbertype correct in the record?
      ItemTypes::streetNumberType calcType = 
         ItemTypes::getStreetNumberTypeFromHouseNumbering(
               leftNbrStart, leftNbrEnd, rightNbrStart, rightNbrEnd);
      if (type != calcType) {
         mc2log << warn << "streetNumberType does not match "
             << "record=" << int(type) << " calculated="
             << int(calcType) << " - will use calculated: ";
         printRecord(rec);
         type = calcType;
      }
      OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
         (m_map->itemLookup(node->getNodeID() & ITEMID_MASK));
      if (ssi != NULL) {
         if (!m_checkRecords) {
            if ((ssi->getLeftSideNbrStart() != leftNbrStart) ||
                (ssi->getLeftSideNbrEnd() != leftNbrEnd) ||
                (ssi->getRightSideNbrStart() != rightNbrStart) ||
                (ssi->getRightSideNbrEnd() != rightNbrEnd) ||
                (ssi->getStreetNumberType() != type) ) {
               ssi->setStreetNumberType(type);
               ssi->setLeftSideNumberStart(leftNbrStart);
               ssi->setLeftSideNumberEnd(leftNbrEnd);
               ssi->setRightSideNumberStart(rightNbrStart);
               ssi->setRightSideNumberEnd(rightNbrEnd);
               m_nbrRecordsAdded += 1;
               m_recordsAddedToMaps.push_back(m_curLineNumber);
               mc2log << info << "EDR rec " << m_curLineNumber 
                      << " House number type set to '"
                      << rec[7] << "' nbrs "
                      << leftNbrStart << "," << leftNbrEnd << ","
                      << rightNbrStart << "," << rightNbrEnd
                      << " item=" << m_map->getMapID() << ":" << ssi->getID()
                      << endl;
               if (m_applyOnMergedMaps)
                  m_affectsMap.push_back(ssi->getID());
            }
         } else {
            // record found
            setRecordFoundAndAddMapid(true, m_recordId);

            // Check original value against the map
            if ((m_originalValStr != "") &&
                (strlen(m_originalValStr.c_str()) > 0)) {
               // build the string exactly like the origValStr in MapEditor
               char mapVal[64];
               sprintf(mapVal, "%d,%d,%d,%d,%d", ssi->getLeftSideNbrStart(),
                     ssi->getLeftSideNbrEnd(), ssi->getRightSideNbrStart(),
                     ssi->getRightSideNbrEnd(),ssi->getStreetNumberType());
               if (m_originalValStr != mapVal)
                  setRecordMapChanged(true, m_recordId);
            }
            
            if ((ssi->getLeftSideNbrStart() != leftNbrStart) ||
                (ssi->getLeftSideNbrEnd() != leftNbrEnd) ||
                (ssi->getRightSideNbrStart() != rightNbrStart) ||
                (ssi->getRightSideNbrEnd() != rightNbrEnd)) {
               // record found and needed
               setRecordNeeded(true, m_recordId);
            }
            // else record found but not needed
         }
         return true;
      }
   }
   else if (m_checkRecords) {
      // use coords from rec1 (=OldNode)
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
               rec[1], lat, lon, CoordinateTransformer::mc2)) {
         checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                 lat, lon, m_recordId);
      }
   }
   
   return false;
}

bool
ExtraDataReader::handleSetEntryRestrictionsRecord(vector<MC2String>& rec,
                                        bool parseOnly)
{
   mc2dbg4 << "SET_ENTRYRESTRICTIONS" << endl;
   // setEntryRestrictions-record definition
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. value

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   OldNode* node = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   int val = ItemTypes::getEntryRestriction(rec[3].c_str(), 
                                            StringTable::ENGLISH);
   // Check the indata
   if (val < 0) {
      mc2log << error << "Error in SET_ENTRYRESTRICTIONS record, val=" 
             << val << " (str=" << rec[3] << ")" << endl;
      printRecord(rec);
      return false;
   }
   
   // Indata OK!
   if (parseOnly) {
      return true;
   }
   
   if (node != NULL) {
      if ( m_checkRecords ) {
         // record found
         setRecordFoundAndAddMapid(true, m_recordId);
         ItemTypes::entryrestriction_t entryVal = node->getEntryRestrictions();
         // Check origval against the map
         if ((m_originalValStr != "") &&
             (strlen(m_originalValStr.c_str()) > 0)) {
            int32 origVal = ItemTypes::getEntryRestriction(
                  m_originalValStr.c_str(), StringTable::ENGLISH);
            mc2dbg4 << " " << m_recordId << ": origER=" << origVal 
                    << " (str=" << m_originalValStr << ") mapER=" 
                    << entryVal << " (edER=" << val << ")" << endl;
            if (origVal != int(entryVal))
               setRecordMapChanged(true, m_recordId);
         }
      }
      if (node->getEntryRestrictions() != val) {
         node->setEntryRestrictions( ItemTypes::entryrestriction_t(val));
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber 
                << " Entry restriction set to '"
                << rec[3] << "' " << m_map->getMapID() << ":"
                << node->getNodeID() << endl;
         m_affectsTurnDescr.push_back(node->getNodeID());

         // If this node is on a routeable item connected to a virtual item
         // transfer the correction also to the virtual.
         vector<uint32> changedVirtuals;
         OldExtraDataUtility::transferChangeToVirtualNode(
                  m_map, node->getNodeID(), val, 
                  OldExtraDataUtility::SET_ENTRYRESTRICTIONS,
                  changedVirtuals);
         // insert affected node ids into the m_affectsTurnDescr vector
         for ( vector<uint32>::const_iterator it = changedVirtuals.begin();
               it != changedVirtuals.end(); it++ ) {
            m_affectsTurnDescr.push_back( *it );
         }
         if ( m_checkRecords ) {
            setRecordNeeded(true, m_recordId);
         }
      }
      return true;
   }  
   else if (m_checkRecords) {
      // use coords from rec1 (=OldNode)
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
               rec[1], lat, lon, CoordinateTransformer::mc2)) {
         checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                 lat, lon, m_recordId);
      }
   }
  
   return false;
}

bool
ExtraDataReader::handleSetTollRoadRecord(vector<MC2String>& rec,
                                         bool parseOnly)
{
   mc2dbg4 << "SET_TOLLROAD" << endl;
   // setTollRoad-record definition
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. value

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   

   // Get indata
   OldNode* node = OldExtraDataUtility::getNodeFromStrings(
                        m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   bool val = false;
   if ( (rec[3][0] == 'T') || (rec[3][0] == 't'))
      val = true;
   else if ( (rec[3][0] == 'F') || (rec[3][0] == 'f'))
      val = false;
   else {
      mc2log << error << " Invalid boolean-value: " << rec[3] << endl;
      return false;
   }

   // Check indata
   if ( (rec.size() != 4) ||
        (rec[1].size() == 0) ||
        (rec[2].size() == 0) ) {
      mc2log << error << "Error in SET_TOLLROAD record " << m_curLineNumber
             << endl;
      printRecord(rec);
      return false; 
   }

   // Indata OK!
   if (parseOnly) {
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
                        rec[1], lat, lon, CoordinateTransformer::mc2))
         return OldExtraDataUtility::strtolatlon(rec[2], lat, lon,
                                              CoordinateTransformer::mc2);
      return false;
   }

   
   if (node != NULL) { 
      if (m_checkRecords) {
         // record found
         setRecordFoundAndAddMapid(true, m_recordId);
         if ( node->hasRoadToll() != val) {
            // record found and needed
            setRecordNeeded(true, m_recordId);
         } else {
            // record found but not needed and map changed
            setRecordMapChanged(true, m_recordId);
         }
      }
      
      // do the correction
      if (node->hasRoadToll() != val) {
         node->setRoadToll(val);
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber 
                << " Toll road val set to "
                << getStringFromBool(val) << " for " 
                << m_map->getMapID() << ":" << node->getNodeID() << endl;
         if (m_applyOnMergedMaps)
            m_affectsMap.push_back(node->getNodeID() & ITEMID_MASK);

         // If this node is on a routeable item connected to a virtual item
         // transfer the correction also to the virtual.
         vector<uint32> changedVirtuals;
         OldExtraDataUtility::transferChangeToVirtualNode(
                  m_map, node->getNodeID(), int(val), 
                  OldExtraDataUtility::SET_TOLLROAD,
                  changedVirtuals);
         // insert affected virtual item ids into the m_affectsMap vector
         if (m_applyOnMergedMaps) {
            for (vector<uint32>::const_iterator it = changedVirtuals.begin();
                 it != changedVirtuals.end(); it++ ) {
               m_affectsMap.push_back( *it & ITEMID_MASK );
            }
         }
      }
      return true;
            
      return true;
   }
   else if (m_checkRecords) {
      // node is NULL, probably another map
      // use coords from rec1 (=OldNode)
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
               rec[1], lat, lon, CoordinateTransformer::mc2)) {
         checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                 lat, lon, m_recordId);
      }
   }

   return false;
}

bool
ExtraDataReader::handleSetSpeedLimitRecord(vector<MC2String>& rec,
                                           bool parseOnly)
{
   mc2dbg4 << "SET_SPEEDLIMIT" << endl;
   // setSpeedLimit-record definition
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. value

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   OldNode* node = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   uint32 val = getUint32(rec[3]);

   // Check indata
   if (val >= MAX_UINT32/2) {
      mc2log << error << "Error in SET_SPEEDLIMIT record, val=" << val
             << "strval=" << rec[3] << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }
   
   if (node != NULL) { 
      if (m_checkRecords) {
         // record found
         setRecordFoundAndAddMapid(true, m_recordId);
      }
      
      if (node->getSpeedLimit() != val) {
         node->setSpeedLimit(byte(val));
         m_nbrRecordsAdded += 1;
         m_recordsAddedToMaps.push_back(m_curLineNumber);
         mc2log << info << "EDR rec " << m_curLineNumber 
                << " Speed limit set to '" << rec[3] << "'" << endl;
         if (m_applyOnMergedMaps)
            m_affectsMap.push_back(node->getNodeID() & ITEMID_MASK);
         
         // If this node is on a routeable item connected to a virtual item
         // transfer the correction also to the virtual.
         vector<uint32> changedVirtuals;
         OldExtraDataUtility::transferChangeToVirtualNode(
                  m_map, node->getNodeID(), val, 
                  OldExtraDataUtility::SET_SPEEDLIMIT,
                  changedVirtuals);
         // insert affected virtual item ids into the m_affectsMap vector
         if (m_applyOnMergedMaps) {
            for (vector<uint32>::const_iterator it = changedVirtuals.begin();
                 it != changedVirtuals.end(); it++ ) {
               m_affectsMap.push_back( *it & ITEMID_MASK );
            }
         }
         
         if (m_checkRecords) {
            // record needed
            setRecordNeeded(true, m_recordId);
            // Check origval against the map
            if ((m_originalValStr != "") &&
                (strlen(m_originalValStr.c_str()) > 0)) {
               int32 origVal = strtoul(m_originalValStr.c_str(), NULL, 0);
               mc2dbg4 << " " << m_recordId << ": origSpeed=" << origVal 
                       << " (str=" << m_originalValStr << ") mapSpeed=" 
                       << int(node->getSpeedLimit()) 
                       << " (edSpeed=" << val << ")" << endl;
               if (origVal != int(node->getSpeedLimit()))
                  setRecordMapChanged(true, m_recordId);
            }
         }
      }
      
      return true;
   }
   else if (m_checkRecords) {
      // use coords from rec1 (=OldNode)
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
               rec[1], lat, lon, CoordinateTransformer::mc2)) {
         checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                 lat, lon, m_recordId);
      }
   }

   return false;
}

bool
ExtraDataReader::handleSetLevelRecord(vector<MC2String>& rec, bool parseOnly)
{
   mc2dbg4 << "SET_LEVEL" << endl;
   // setLevel-record definition
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. value

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   OldNode* node = OldExtraDataUtility::getNodeFromStrings(
                  m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   int32 val = getNumber(rec[3]);

   // Check indata
   if (val == MAX_INT32) {
      mc2log << error << "Error in SET_LEVEL record, val=" << val
             << " valstr=" << rec[3] << endl;
      printRecord(rec);
      return false;
   }
   
   // Indata OK!
   if (parseOnly) {
      return true;
   }
  
   if (node != NULL) {
      if (!m_checkRecords) {
         if (node->getLevel() != int(val)) {
            node->setLevel(int(val));
            m_nbrRecordsAdded += 1;
            m_recordsAddedToMaps.push_back(m_curLineNumber);
            mc2log << info << "EDR rec " << m_curLineNumber 
                   << " Level set to '" << rec[3] << "' "
                   << m_map->getMapID() << ":" << node->getNodeID() << endl;
            if (m_applyOnMergedMaps)
               m_affectsMap.push_back(node->getNodeID() & ITEMID_MASK);
         }
      } else {
         // record found
         setRecordFoundAndAddMapid(true, m_recordId);
         // Check origval against the map
         if ((m_originalValStr != "") &&
             (strlen(m_originalValStr.c_str()) > 0)) {
            int32 origVal = strtol(m_originalValStr.c_str(), NULL, 0);
            mc2dbg4 << " " << m_recordId << ": origLevel=" << origVal 
                    << " (str=" << m_originalValStr << ") mapNodeLevel=" 
                    << int(node->getLevel()) << " (edNodeLevel=" 
                    << val << ")" << endl;
            if (origVal != int(node->getLevel()))
               setRecordMapChanged(true, m_recordId);
         }
         if (node->getLevel() != int(val)) {
            // record found and needed
            setRecordNeeded(true, m_recordId);
         }
         // else record found but not needed
      }
      return true;
   }
   else if (m_checkRecords) {
      // use coords from rec1 (=OldNode)
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(
               rec[1], lat, lon, CoordinateTransformer::mc2)) {
         checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                 lat, lon, m_recordId);
      }
      else {
         mc2log << error << "Error in SET_LEVEL record m_recordId="
                << m_recordId;
         mc2log << " cord NOT OK" << endl;
         printRecord(rec);
         return false;
      }
   }

   return false;
}

bool
ExtraDataReader::handleSetTurndirectionRecord(vector<MC2String>& rec,
                                        bool parseOnly)
{
   mc2dbg4 << "SET_TURNDIRECTION" << endl;
   // SET_TURNDIRECTION-record definition:
   // 1. (lat1,lon1)
   // 2. (lat2,lon2)
   // 3. (lat1,lon1)
   // 4. (lat2,lon2)
   // 5. value

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }

   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   OldNode* fromNode = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[1], rec[2]);
   OldNode* toNode = OldExtraDataUtility::getNodeFromStrings(
               m_map, CoordinateTransformer::mc2, rec[3], rec[4]);
   int val = ItemTypes::getTurnDirection(rec[5].c_str(),
                                        StringTable::ENGLISH);
   // Check the indata
   if (val < 0) {
      mc2log << error << "Error in SET_TURNDIRECTION record, val=" 
             << val << " (str=" << rec[5] << ")" << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }
  
   if ((fromNode != NULL) && (toNode != NULL)) {
      int setTurn = setTurndirection(fromNode, toNode,
                     ItemTypes::turndirection_t(val),
                     m_checkRecords);
      if (!m_checkRecords) {
         if (setTurn < 0) {
            // no conn betwen the nodes
            mc2log << error << "Failed to set turndirection: ";
            printRecord(rec);
            return false;
         } else {
            m_nbrRecordsAdded += 1;
            m_recordsAddedToMaps.push_back(m_curLineNumber);
            mc2log << info << "EDR rec " << m_curLineNumber 
                   << " Turndirection set to '" << rec[5] << "' "
                      << m_map->getMapID() << ":" << fromNode->getNodeID()
                      << ":" << toNode->getNodeID() << endl;
            if (m_applyOnMergedMaps) {
               m_affectsMap.push_back(toNode->getNodeID() & ITEMID_MASK);
               // Also save this info if the turn dir needs to be
               // re-applied due to re-generated turndescr
               OldConnection* conn = getConnection(fromNode, toNode);
               connInfo_t connInfo;
               connInfo.toNodeId = toNode->getNodeID();
               connInfo.turndir = conn->getTurnDirection();
               connInfo.crossing = conn->getCrossingKind();
               m_addedTurnDescrRecords.insert(make_pair(conn, connInfo));
            }
         }
      }
      else {
         if (setTurn < 0) {
            // No conn, but nodes found - correct map (mapcandidate)
            addMapId(m_map->getMapID(), m_recordId, true);
         } else if (setTurn == 0) {
            // The values matched (record found but not needed)
            setRecordFoundAndAddMapid(true, m_recordId);
         } else {
            // The values differed (record found and needed)
            setRecordFoundAndAddMapid(true, m_recordId);
            setRecordNeeded(true, m_recordId);
         }
      }
      return true;
   }
   else if (m_checkRecords) {
      // check if this map is a candidate for this record
      if ((fromNode != NULL) || (toNode != NULL)) {
         addMapId(m_map->getMapID(), m_recordId, true);
      } else {
         // use coords from rec3 (=toNode)
         int32 lat, lon;
         if (OldExtraDataUtility::strtolatlon(
                  rec[3], lat, lon, CoordinateTransformer::mc2)) {
            checkAndAddMapCandidate(ItemTypes::streetSegmentItem, 
                                    lat, lon, m_recordId);
            // XXX ssi or ferry ??
         }
      }
   }
   
   return false;
}

bool
ExtraDataReader::handleSetWaterTypeRecord(vector<MC2String>& rec,
                                          bool parseOnly)
{
   mc2dbg4 << "SET_WATER_TYPE" << endl;
   // SET_WATER_TYPE-record definition:
   // 1. itemType
   // 2. coordinate type
   // 3. coordinate
   // 4. name
   // 5. name type
   // 6. name language
   // 7. new water type

   if (m_checkRecords) {
      // set recordChecked to true
      setRecordChecked(true, m_recordId);
   }
   
   if (m_applyOnMergedMaps && !applyCurrentRecordDynamic()) {
      // This extra data record has a date that is older than
      // the extra data time for the map, don't (re-)apply.
      return false;
   }
   
   // Get the indata
   ItemTypes::itemType it = getItemType(rec[1]);
   format_t coordType = CoordinateTransformer::getCoordinateType(rec[2]);
   int32 lat, lon;
   bool coordOK = OldExtraDataUtility::strtolatlon(rec[3], lat, lon, coordType);
   ItemTypes::name_t nt = getNameType(rec[5]);
   LangTypes::language_t lang = getLanguage(rec[6]);
   int newType = ItemTypes::getWaterTypeFromString(rec[7].c_str());

   // Check indata
   if ( (rec.size() != 8) ||
        (rec[7].size() == 0) ||
        (it != ItemTypes::waterItem) ||
        ((rec[4].size() > 0) &&
         (nt == ItemTypes::invalidName)) ||
        ((lang == LangTypes::invalidLanguage) && 
         (nt != ItemTypes::roadNumber) &&
         (nt != ItemTypes::invalidName)) ||
        (coordType == CoordinateTransformer::nbrCoordinateTypes) ||
        (!coordOK) ||
        (newType < 0) ) {
      mc2log << error << "Error in SET_WATER_TYPE record it="
             << StringTable::getString(ItemTypes::getItemTypeSC(it),
                                       StringTable::ENGLISH) << " nt="
             << ItemTypes::getNameTypeAsString(nt) << " lang="
             << LangTypes::getLanguageAsString(lang);
      if (coordType != CoordinateTransformer::nbrCoordinateTypes) 
         mc2log << " cordType OK";
      else 
         mc2log << " cordType NOT OK";
      if (coordOK) 
         mc2log << " cord OK";
      else 
         mc2log << " cord NOT OK";
      if (newType >= 0)
         mc2log << ", water type OK" << endl;
      else
         mc2log << ", water type NOT OK" << endl;
      printRecord(rec);
      return false;
   }

   // Indata OK!
   if (parseOnly) {
      return true;
   }

   // Check whether the item in question has a name or if NULL
   // should be sent to the getItemFromCoordinate function.
   char* name = StringUtility::newStrDup(rec[4].c_str());
   if (strlen(name) == 0)  {
      name = NULL;
   }
   vector<OldItem*> closeItems;
   OldItem* item = m_map->getItemFromCoordinate(it, lat, lon, closeItems,
                                             name, -1);
   delete [] name;
   
   if ( (item != NULL) && 
        (item->getItemType() == ItemTypes::waterItem) ) {

      OldWaterItem* waterItem = static_cast<OldWaterItem*>(item);
      ItemTypes::waterType newWaterType = ItemTypes::waterType(newType);

      // If the water item already has this type
      bool itemHasType = waterItem->getWaterType() == newWaterType;
      
      if (m_checkRecords) {
         // the record is found and needed
         setRecordFoundAndAddMapid(true, m_recordId);
         if (!itemHasType)
            setRecordNeeded(true, m_recordId);
         return true;
      }
      if (itemHasType) {
         mc2log << warn << "In setWaterType: OldItem already has the new type "
                << rec[7] << endl;
         return false;
      }

      // Make update
      waterItem->setWaterType(newWaterType);
      m_nbrRecordsAdded += 1;
      m_recordsAddedToMaps.push_back(m_curLineNumber);
      mc2log << info << "EDR rec " << m_curLineNumber 
             << " Water type set to '" << rec[7] << "' for water "
             << m_map->getMapID() << ":" << waterItem->getID() 
             << " " << rec[4] << endl;
      if (m_applyOnMergedMaps) {
         m_affectsMap.push_back(item->getID());
      }
      return true;
   
   } else if (m_checkRecords) {
      // check if the coordinate fits in this map
      checkAndAddMapCandidate(it, lat, lon, m_recordId);
   }

   return false;
}

void 
ExtraDataReader::printRecord(vector<MC2String>& rec) {
   
   MC2String recordStr = OldExtraDataUtility::createEDFileString( rec, true );
   mc2log << m_path << ":" << m_curLineNumber << " \"" << recordStr
          << "\"" << endl;
}

ExtraDataReader::recordnotices_t*
ExtraDataReader::getRecordWithId(uint32 recordId)
{
   recordnotices_t* retVal = NULL;
   if (recordId == MAX_UINT32) {
      return retVal;
   }

   bool found = false;
   recordNoticeIt it = m_checkRecordTable.begin();
   while (!found && it != m_checkRecordTable.end()){
      if ((*it).recordId == recordId) {
         found = true;
         retVal = &(*it);
      } else {
         it++;
      }
   }
   return retVal;
}

bool
ExtraDataReader::setRecordChecked(bool checked, uint32 recId)
{
   bool retVal = false;
   
   recordnotices_t* rec = getRecordWithId(recId);
   if (rec != NULL) {
      (*rec).recordChecked = checked;
      retVal = true;
   }
   return retVal;
}

bool
ExtraDataReader::setRecordFoundAndAddMapid(bool found, uint32 recId)
{
   bool retVal = false;
   
   recordnotices_t* rec = getRecordWithId(recId);
   if (rec != NULL) {
      (*rec).recordFound = found;
      if (found) {
         // the record is found in this map, add mapid
         (*rec).mapIds.push_back(m_map->getMapID());
      }
      retVal = true;
   }
   return retVal;
}

bool
ExtraDataReader::setRecordMapChanged(bool changed, uint32 recId)
{
   bool retVal = false;
   
   recordnotices_t* rec = getRecordWithId(recId);
   if (rec != NULL) {
      (*rec).recordMapChanged = changed;
      retVal = true;
   }
   return retVal;
}

bool
ExtraDataReader::setRecordNeeded(bool needed, uint32 recId)
{
   bool retVal = false;
   
   recordnotices_t* rec = getRecordWithId(recId);
   if (rec != NULL) {
      (*rec).recordNeeded = needed;
      retVal = true;
   }
   return retVal;
}

bool
ExtraDataReader::addMapId(uint32 mapId, uint32 recId, 
                          bool addToCandidatesVector)
{
   bool retVal = false;
   
   recordnotices_t* rec = getRecordWithId(recId);
   if (rec != NULL) {
      if (addToCandidatesVector)
         (*rec).mapIdCandidates.push_back(mapId);
      else
         (*rec).mapIds.push_back(mapId);
      retVal = true;
   }
   return retVal;
}

void
ExtraDataReader::checkAndAddMapCandidate(ItemTypes::itemType it,
                               int32 lat, int32 lon, uint32 recId)
{
   // check if there is an item of itemtype it in this map 
   // that is close to (lat,lon)

   MC2BoundingBox bbox;
   m_map->getGfxData()->getMC2BoundingBox(bbox);
   if (bbox.contains(lat, lon)) {
   
      uint64 dist;
      OldMapHashTable* mht = m_map->getHashTable();
      mht->clearAllowedItemTypes();
      mht->addAllowedItemType(it);
      uint32 closestID = mht->getClosest(lon, lat, dist);
      mc2dbg4 << " For " << m_recordId << " found closest item "
              << m_map->getFirstItemName(closestID) << " in "
              << sqrt(dist) * GfxConstants::MC2SCALE_TO_METER << " meters "
              << endl;
      if ((sqrt(dist) * GfxConstants::MC2SCALE_TO_METER) < 500) {
         // one item is close enough
         recordnotices_t* rec = getRecordWithId(recId);
         if ((rec != NULL)) {
            (*rec).mapIdCandidates.push_back(m_map->getMapID());
         }
      }
   }
}

bool
ExtraDataReader::getExtradataTimeForMap()
{
   time_t tmpOrigCreationTime = 
      static_cast<time_t>(m_map->getTrueCreationTime());
   uint32 origCreationTime = m_map->getTrueCreationTime();

   time_t dynamicTime = 
      static_cast<time_t>(m_map->getDynamicExtradataTime());

   mc2dbg4 << "origCreationTime=" << tmpOrigCreationTime << " = "
           << asctime(localtime( &tmpOrigCreationTime));
   mc2dbg4 << "dynamicEDTime=" << dynamicTime << " = "
           << asctime(localtime( &dynamicTime));
   
   // Check the dates and store the latest one in m_mapEDdate
   uint32 mapTime = dynamicTime;
   if (mapTime == MAX_UINT32) {
      if (origCreationTime == MAX_UINT32) {
         // no time stored in the map, can't apply dynamic ed.
         return false;
      } else {
         mapTime = origCreationTime;
      }
   }

   if (mapTime != MAX_UINT32) {
      char mapDate[126];
      time_t time = time_t(mapTime);
      if ( sprintf(mapDate,
                   "%04d-%02d-%02d %02d:%02d:%02d",
                   localtime(&time)->tm_year + 1900,
                   localtime(&time)->tm_mon + 1,
                   localtime(&time)->tm_mday,
                   localtime(&time)->tm_hour,
                   localtime(&time)->tm_min,
                   localtime(&time)->tm_sec) ) {
         // date string extracted
         m_mapEDdate = mapDate;
         return true;
      }
   }
   
   return false;
}

bool
ExtraDataReader::getExtradataRecordTime(const MC2String& commentDateAndTime)
{
   // Records extracted from WASP "# yyyy-mm-dd hh:mm:ss"
   // Records created in MapEditor "# Mon Feb  2 12:37:03 2009"
   // For dynamic extra data we always use records extracted from WASP
   MC2String newDate = MC2String(commentDateAndTime, 2);

   if ((newDate.size() == 19) && ( newDate[4] == '-') ) {
      m_recordEDdate = newDate;
      return true;
   } else {
      m_recordEDdate = "";
      return false;
   }

   return false;
}

bool
ExtraDataReader::applyCurrentRecordDynamic()
{
   if ((m_mapEDdate.size() != 19) || (m_recordEDdate.size() != 19))
      return false;

   return ( m_mapEDdate < m_recordEDdate );
}

void
ExtraDataReader::fillConnectionInfo()
{
   // First clear
   m_connectionInfo.clear();
   // Then loop all routeable items and fill connection info
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z) {
      for(uint32 i = 0; i < m_map->getNbrItemsWithZoom(z); ++i) {
         OldRouteableItem* item =
            static_cast<OldRouteableItem*>(m_map->getItem(z, i));
         if ((item != NULL) &&
             ((item->getItemType() == ItemTypes::streetSegmentItem) ||
              (item->getItemType() == ItemTypes::ferryItem))) {
            //nbrRI++;
            for (uint32 n = 0; n < 2; n++) {
               OldNode* node = item->getNode(n);
               if (node != NULL) {
                  for (uint16 c = 0; c < node->getNbrConnections(); c++) {
                     OldConnection* conn = node->getEntryConnection(c);
                     if (conn != NULL) {
                        connInfo_t connInfo;
                        connInfo.toNodeId = node->getNodeID();
                        connInfo.turndir = conn->getTurnDirection();
                        connInfo.crossing = conn->getCrossingKind();
                        m_connectionInfo.insert(
                              make_pair(conn, connInfo));
                     }
                  }
               }
            }
         }
      }
   }
}

#ifdef TEST_EXTRADATAREADER
int main() {

   ExtraDataReader edr("test_extradata");

   edr.parseInto(0);

   return 0;
}
#endif



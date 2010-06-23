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

#include "OldExtraDataUtility.h"
#include "GfxData.h"
#include "OldMapHashTable.h"
#include "StringUtility.h"
#include "CoordinateTransformer.h"

// Items
#include "OldStreetSegmentItem.h"
#include "OldRailwayItem.h"
#include "OldPointOfInterestItem.h"
#include "OldBuiltUpAreaItem.h"
#include "OldStreetItem.h"

#include "OldConnection.h"
#include "OldNode.h"

#include "OldGenericMap.h"
#include "OldExternalConnections.h"

#include "GfxUtility.h"
#include "GfxConstants.h"
#include "MC2MapGenUtil.h"
#include "ExtraDataUtility.h"

#define END_OF_RECORD "EndOfRecord"

bool OldExtraDataUtility::m_initiated = false;
OldExtraDataUtility::recordMap_t OldExtraDataUtility::m_recordTypes;
OldExtraDataUtility::selectConnMap_t OldExtraDataUtility::m_selectConns;

const MC2String OldExtraDataUtility::edFieldSep = MC2MapGenUtil::poiWaspFieldSep;

void
OldExtraDataUtility::init()
{
   if (!m_initiated) {
      m_recordTypes["addNameToItem"] = m_recordTypes["ai"] = ADD_NAME_TO_ITEM;
      m_recordTypes["addcitypart"] = m_recordTypes["acp"] = ADD_CITYPART;
      m_recordTypes["updatename"] = m_recordTypes["un"] = UPDATE_NAME;
      m_recordTypes["setRamp"] = SET_RAMP;
      m_recordTypes["setRoundabout"] = SET_ROUNDABOUT;
      m_recordTypes["setMultiDigitised"] = SET_MULTIDIGITISED;
      m_recordTypes["setEntryRestrictions"] = SET_ENTRYRESTRICTIONS;
      m_recordTypes["setSpeedLimit"] = SET_SPEEDLIMIT;
      m_recordTypes["setLevel"] = SET_LEVEL;
      m_recordTypes["setTurnDirection"] = SET_TURNDIRECTION;
      m_recordTypes["setVehicleRestrictions"] = SET_VEHICLERESTRICTIONS;
      m_recordTypes["setHousenumber"] = SET_HOUSENUMBER;
      m_recordTypes["addlandmark"] = ADD_LANDMARK;
      m_recordTypes["addsinglelandmark"] = ADD_SINGLE_LANDMARK;
      m_recordTypes["removeNameFromItem"] = REMOVE_NAME_FROM_ITEM;
      m_recordTypes["removeItem"] = REMOVE_ITEM;
      m_recordTypes["addSignpost"] = ADD_SIGNPOST;
      m_recordTypes["removeSignpost"] = REMOVE_SIGNPOST;
      m_recordTypes["setRoundaboutish"] = SET_ROUNDABOUTISH;
      m_recordTypes["setControlledAccess"] = SET_CONTROLLEDACCESS;
      m_recordTypes["setWaterType"] = SET_WATER_TYPE;
      m_recordTypes["setTollRoad"] = SET_TOLLROAD;
      m_recordTypes["%"] = m_recordTypes["#"] = COMMENT;

      m_selectConns["ALLCONN"]   = ALLCONN;
      m_selectConns["AHEADCONN"] = AHEADCONN;
      m_selectConns["TURNCONN"]  = TURNCONN;
      m_selectConns["NOCONN"]    = NOCONN;

      m_initiated = true;
   }
}



// This is the function called from ExtraDataReader and MapEditor
int
OldExtraDataUtility::updateName(OldGenericMap* theMap,
                       OldItem* item,
                       ItemTypes::itemType it,
                       MC2String ename,
                       ItemTypes::name_t nt,
                       LangTypes::language_t lang,
                       MC2String nname,
                       vector<uint32>& affectsStreets)
{  
   // The number of names affected by the change
   int counter = 0;
   
   Vector* matchingItems = new Vector(4, 4);
   uint32 enameIndex = OldExtraDataUtility::getAllItemsWithName(
                                    theMap, ename, matchingItems);
   // Return if no match
   if (enameIndex == MAX_UINT32) {
      mc2dbg1 << "Failed to find \"" << ename << "\" in the map " 
              << theMap->getMapID() << endl;
      return 0;
   }
   
   uint8 offset = getNameOffsetForItem(item, enameIndex, nt, lang);
   if ( offset == MAX_UINT8 ){
      mc2log << error << "Failed to update name \"" << ename
             << "\" to item:" << item->getID() 
             << " Could not find this name in the item" << endl; 
      return 0;
   }

   item->removeNameWithOffset( offset );
   uint32 newStringIndex = theMap->addNameToItem(item, nname.c_str(), lang, nt);
   
   if (newStringIndex != MAX_UINT32) {
      counter++;
      mc2dbg4 << info << "Name \"" << nname << "\" updated" << endl;
      if (it == ItemTypes::streetSegmentItem)
         affectsStreets.push_back(item->getID());
   } else {
      mc2dbg1 << error << "Failed to update name \"" << nname << "\" to item" 
             << endl;
      return false;
   }

   // If the name update should be done for a street item, all ssi:s
   // in this street should also be updated.
   if (it == ItemTypes::streetItem) {
      OldStreetItem* street = dynamic_cast<OldStreetItem*> (item);
      Vector* streetSegIDs = new Vector();
      if (street != NULL) {
         // Put all the street's street segments in a vector 
         for (uint32 j = 0; j < street->getNbrItemsInGroup(); j++) {
            streetSegIDs->addLastIfUnique(street->getItemNumber(j));
         }
         mc2dbg4 << "OEDU::updateName street " << street->getID()
              << " nbrSSi=" << streetSegIDs->getSize() << endl;
         for (uint32 i = 0; i < streetSegIDs->getSize(); i++) {
            OldItem* curItem = theMap->itemLookup(streetSegIDs->getElementAt(i));

            mc2dbg4 << "removing name for item with id "
                    << streetSegIDs->getElementAt(i) << endl;
            DEBUG4(
            for (uint32 n = 0; n < curItem->getNbrNames(); n++) {
               mc2dbg4 << "ssi name: "
                       << theMap->getName(curItem->getStringIndex(n))
                       << " ssi name type: " << int(curItem->getNameType(n))
                       << endl;
            });
            
            // Why not using nt instead of finding ssint?
            byte nameOffset = getNameOffsetForItem(
                       curItem, enameIndex, ItemTypes::maxDefinedName, lang);
            ItemTypes::name_t ssint = curItem->getNameType(nameOffset);
            DEBUG4(cout << "ssint= " << int(ssint) << endl;)
            
            // Remove the old name and add a new one for all items involved.
            curItem->removeNameWithOffset(nameOffset);
            uint32 newStringIndex = theMap->addNameToItem(curItem,
                                                          nname.c_str(),
                                                          lang, ssint);
            
            if (newStringIndex != MAX_UINT32) {
               counter++;
               mc2dbg4 << info << "Name \"" << nname << "\" updated" << endl;
               affectsStreets.push_back(curItem->getID());
            } else {
               mc2dbg1 << error << "Failed to update name \"" << nname 
                                << "\" to item" << endl;
               return 0;
            }                                           
         }
      }
      delete streetSegIDs;
   }
   
   // If updating a name for a ssi, make sure that the ssi is
   // removed from and added to street-groups?
   // No, not handled.
   
   return counter;

}   

int
OldExtraDataUtility::removeNameFromItem(OldGenericMap* theMap,
                       OldItem* item,
                       ItemTypes::itemType it,
                       MC2String& ename,
                       ItemTypes::name_t nt,
                       LangTypes::language_t lang,
                       vector<uint32>& affectsStreets)
{  
   // The number of names affected by the change
   int counter = 0;
   
   Vector* matchingItems = new Vector(4, 4);
   uint32 enameIndex = OldExtraDataUtility::getAllItemsWithName(
                                    theMap, ename, matchingItems);
   // Return if no match
   if (enameIndex == MAX_UINT32) {
      mc2dbg1 << "Failed to find \"" << ename << "\" in the map " 
              << theMap->getMapID() << endl;
      return 0;
   }
   
   byte offset = getNameOffsetForItem(item, enameIndex, nt, lang);
   if ( offset == MAX_BYTE ) {
      mc2dbg1 << "Failed to find \"" << ename 
              << "\" with correct type and language in map "
              << theMap->getMapID() << endl;
      return 0;
   }
   
   item->removeNameWithOffset( offset ); 
   counter++;
   if (it == ItemTypes::streetSegmentItem)
      affectsStreets.push_back(item->getID());
   

   // If the name removal should be done for a street item, all ssi:s
   // in this street is also affected.
   if (it == ItemTypes::streetItem) {
      OldStreetItem* street = dynamic_cast<OldStreetItem*> (item);
      Vector* streetSegIDs = new Vector();
      if (street != NULL) {
         // Put all the street's street segments in a vector 
         for (uint32 j = 0; j < street->getNbrItemsInGroup(); j++) {
            mc2dbg4 << info << "Number ssi:s i si: " 
                            << street->getNbrItemsInGroup() << endl;
            streetSegIDs->addLastIfUnique(street->getItemNumber(j));            
         }                     
         for (uint32 i = 0; i < streetSegIDs->getSize(); i++) {
            OldItem* curItem = theMap->itemLookup(streetSegIDs->getElementAt(i));

            mc2dbg4 << "removing name for item with id " 
                    << streetSegIDs->getElementAt(i) << endl;
            DEBUG4(
            uint32 ssiNbrNames = curItem->getNbrNames();
            for (uint32 n = 0; n < ssiNbrNames; n++) {
               cout << "ssi name: " 
                    << theMap->getName( curItem->getStringIndex(n))
                    << " ssi name type: " << int(curItem->getNameType(n))
                    << endl;
            });
            
            // Why use maxDefinedName instead of nt?
            byte nameOffset = getNameOffsetForItem(
                       curItem, enameIndex, ItemTypes::maxDefinedName, lang);
            
            // Remove the name
            curItem->removeNameWithOffset(nameOffset);
            counter++;
            affectsStreets.push_back(curItem->getID());
            
         }
      }
      delete streetSegIDs;
   }

   // If removing a name from an ssi, make sure that the ssi is
   // removed from any street-groups it belongs to.
   // If removing name from a street, the ssis perhaps should not belong
   // to this street anymore?
   // No, not handled.
   
   return counter;
}   



int
OldExtraDataUtility::addNameToItem(OldGenericMap* theMap,
                             OldItem* item,
                             ItemTypes::itemType it,
                             MC2String nname,
                             ItemTypes::name_t nt,
                             LangTypes::language_t lang,
                             vector<uint32>& affectsStreets)
{
   int counter = 0;
   
   uint32 newStringIndex = theMap->addNameToItem(
         item, nname.c_str(), lang, nt);
   if (newStringIndex != MAX_UINT32) {
      counter++;
      mc2dbg4 << info << "Name \"" << nname << "\" added to item "  
              << theMap->getFirstItemName(item) << "(has now " 
              << uint32(item->getNbrNames()) << " names)" << endl;
      if (it == ItemTypes::streetSegmentItem)
         affectsStreets.push_back(item->getID());
   } else {
      mc2dbg1 << error << "Failed to add name \"" << nname << "\" to item" 
             << endl;
      return false;
   }
  
   // If the name addition is to be done to a street item, all ssi:s
   // in this street should have this name too.
   if (it == ItemTypes::streetItem) {
      OldStreetItem* street = dynamic_cast<OldStreetItem*> (item);
      if (street != NULL) {
         for (uint32 j = 0; j < street->getNbrItemsInGroup(); j++) {
            uint32 count = theMap->addNameToItem(
                                 theMap->itemLookup(street->getItemNumber(j)),
                                 nname.c_str(), lang, nt);            
            if (count != MAX_UINT32) {
               counter++;
               mc2dbg4 << info << "Name \"" << nname << "\" added to item "  
                               << theMap->getFirstItemName(item) << "(has now " 
                               << uint32(item->getNbrNames()) 
                               << " names)" << endl;
               affectsStreets.push_back(street->getItemNumber(j));
            } else {
               mc2dbg1 << error << "Failed to add name \"" << nname 
                                << "\" to item" << endl;
               return false;
            }
         }
      }
   }

   // If adding name to a ssi, update street-groups!
   // No, not handled.

   return counter;
   
}   

bool
OldExtraDataUtility::hasItemName(OldItem* item, 
                        const uint32 rawStringIndex)
{
   for (uint32 i=0; i<item->getNbrNames(); i++) {
      if (item->getRawStringIndex(i) == rawStringIndex)
         return true;
   }
   return false;
}



byte
OldExtraDataUtility::getNameOffsetForItem(OldItem* item, 
                                 uint32 enameIndex,
                                 ItemTypes::name_t nt,
                                 LangTypes::language_t lang)
{
   byte offset = MAX_BYTE;
   uint32 i = 0;
   while ((i < item->getNbrNames()) && (offset == MAX_BYTE)) {
      if ((item->getStringIndex(i) == enameIndex) &&
          ((item->getNameType(i) == nt) ||
           (nt == ItemTypes::maxDefinedName)) &&
          (item->getNameLanguage(i) == lang)) {
         // Found the name with the correct name, type and language!
         offset = i;
      } else {
         i++;
      }
   }
   mc2dbg8 << here << " returning " << uint32(offset) << endl;
   return offset;
}

uint32
OldExtraDataUtility::getAllItemsWithName(OldGenericMap* theMap,
                                MC2String& str, 
                                Vector* result)
{
   // Get the OldItemNames and initiate some variables
   OldItemNames* itemNames = theMap->getItemNames();
   const char* searchStr = str.c_str();
   bool found = false;
   uint32 stringIndex = 0;
   const uint32 nbrStrings = itemNames->getNbrStrings();

   // Loop until found the string or all strings are checked
   while ((!found) && (stringIndex < nbrStrings)) {
      if (strcmp(itemNames->getString(stringIndex), searchStr) == 0) {
         mc2dbg4 << "Found matching string. SearchStr=" << searchStr 
                 << ", found=" << itemNames->getString(stringIndex) 
                 << ", index=" << stringIndex << endl;
         found = true;
      } else {
         stringIndex++;
      }
   }

   // If we found the string, get all the items
   if (found) {
   
      // Loop over all the items in the map and check the items that
      // have the name stringIndex
      getAllItemsWithName(theMap, stringIndex, result);

      // Return the string index
      return stringIndex;
   } else {
      // Return an error
      return MAX_UINT32;
   }

}

void
OldExtraDataUtility::getAllItemsWithName(OldGenericMap* theMap,
                                const uint32 stringIndex,
                                Vector* result)
{
   // Loop over all the items in the map and check the items that
   // have the name stringIndex
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      uint32 nbrIWZ = theMap->getNbrItemsWithZoom(z);
      for(uint32 i = 0; i < nbrIWZ; i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item != NULL) {
            uint32 nbrNames = item->getNbrNames();
            for (uint32 j = 0; j < nbrNames; ++j) {
               if (item->getStringIndex(j) == stringIndex) { 
                  // Found
                  result->addLast(item->getID());
                  mc2dbg2 << "Found item (" << item->getID() 
                          << ") with correct name (" << stringIndex << ")"
                          << endl;
               }
            }
         }
      }
   }
}

bool
OldExtraDataUtility::removeItem(OldGenericMap* theMap, OldItem* item)
{
   uint32 itemID = item->getID();
   const char* itemName = theMap->getFirstItemName(item);

   // remove the item from the map
   bool retVal = theMap->removeItem(itemID);
   // group references and pois on ssi: fixed in the removeItem method

   if (retVal) {
      // if street segment item, street item:
      // group references are taken care of in the removeItem method.
   
      // check if the item is used as a landmark
      OldGenericMap::landmarkTable_t* lmTable = &theMap->getLandmarkTable();
      uint32 nbrRemoved = 0;
      typedef OldGenericMap::landmarkTable_t::iterator LI;
      LI it = lmTable->begin();
      LI removeit;
      while (it != lmTable->end()) {
         if ((*it).second.itemID == itemID) {
            mc2dbg4 << " Removing " << itemName
                    << " from the lmTable (conn " 
                    << uint32 (((*it).first >> 32) & 0x00000000ffffffff) 
                    << " -> " 
                    << uint32 ((*it).first & 0x00000000ffffffff) 
                    << ")" << endl;
            removeit = it;
            it++;
            lmTable->erase(removeit);
            nbrRemoved++;
         } else {
            it++;
         }
      }
      if (nbrRemoved > 0) {
         mc2dbg1 << "Removed " << nbrRemoved << " landmarks for '"
                 << itemName << "'" << endl;
      }
   }

   return retVal;
}

bool
OldExtraDataUtility::transferChangeToVirtualNode (
         OldGenericMap* theMap, uint32 nodeID, int val,
         record_t edType, vector<uint32>& changedVirtuals)
{
   // Check all connections that lead to this node and the opposite to see 
   // if any of the other nodes belong to a virtual item = is in the 
   // boundry segments vector of the map.
   // 
   //               nodeID        opposingNode
   //    o---o       o-------------------o       o---o
   //   virtual              normal             virtual
   //
   
   OldBoundrySegmentsVector* bsegments = theMap->getBoundrySegments();
  
   // 1. Check connections to the node that was changed by extra data
   // 2. Check connections to its opposing node
   uint32 nodeIDs[2];
   nodeIDs[0] = nodeID;
   nodeIDs[1] = nodeID ^ 0x80000000;
  
   for ( uint32 checkNode = 0; checkNode < 2; checkNode++ ) {

      uint32 curNodeID = nodeIDs[checkNode];
      OldNode* curNode = theMap->nodeLookup(curNodeID);
   
      for (uint16 c = 0; c < curNode->getNbrConnections(); c++ ) {
         OldConnection* conn = curNode->getEntryConnection(c);
         uint32 otherNodeID = conn->getConnectFromNode();
         // if second node check, we need the opposite node to otherNode.
         if (checkNode == 1)
            otherNodeID = otherNodeID ^ 0x80000000;

         // if this node belongs to a virtual item, apply change
         if ( bsegments->getBoundrySegment(otherNodeID & ITEMID_MASK) != NULL) {
            OldNode* otherNode = theMap->nodeLookup(otherNodeID);
            bool changed = false;
            switch (edType) {
               case SET_SPEEDLIMIT :
                  otherNode->setSpeedLimit( byte(val) );
                  mc2dbg1 << "Speed transfered to virtual node " << otherNodeID
                          << " of item " << (otherNodeID & ITEMID_MASK) << endl;
                  changed = true;
                  break;
               case SET_ENTRYRESTRICTIONS :
                  otherNode->setEntryRestrictions( 
                              ItemTypes::entryrestriction_t(val));
                  mc2dbg1 << "Entry restriction transfered to virtual node " 
                          << otherNodeID << " of item " 
                          << (otherNodeID & ITEMID_MASK) << endl;
                  changed = true;
                  break;
               case SET_TOLLROAD : {
                  bool boolVal = bool (val);
                  otherNode->setRoadToll( boolVal );
                  mc2dbg1 << "Toll road transfered to virtual node " 
                          << otherNodeID << " of item " 
                          << (otherNodeID & ITEMID_MASK) 
                          << ", val=" << val << " boolVal=" << boolVal
                          << endl;
                  changed = true;
                  break;
                  }
               default :
                  break;
            }
            if (changed) {
               changedVirtuals.push_back(otherNodeID);
            }
         }
      }
   }

   if ( changedVirtuals.size() > 0 )
      return true;
   return false;
}

bool
OldExtraDataUtility::readNextRecordFromFile(istream& is,
                                   vector<MC2String>& recordAsStrings)
{
   // Get new row from extra data file
   char t[102400];
   is.getline( t, 102400 );
   MC2String recordString = t;
   // If this row in extra data file was empty, read next (recursive)
   if ( (recordString.size() == 0) &&  ! is.eof()  ) {
      readNextRecordFromFile( is, recordAsStrings );
      return true;
   }
   
   // Clear the outdata-array
   recordAsStrings.erase(recordAsStrings.begin(), recordAsStrings.end());

   // Read all tokens for this record.
   MC2String token;
   while ( (nextToken(recordString, token)) && (token != END_OF_RECORD)) {
      recordAsStrings.push_back(token);
   }
   
   return true;
}

bool
OldExtraDataUtility::nextToken( MC2String& src, MC2String& token )
{
   if ( src.length() == 0 )
      return false;
   
   // position in src where to find the first occurance of separator
   uint32 pos = src.find( edFieldSep.c_str() );
   if ( pos == MAX_UINT32 ) {
      // no separator in src string
      return false;
   }
   
   // extract token, and set src to point at after the first separator
   token = src.substr(0, pos);
   src = src.substr( pos + edFieldSep.length(), src.length() );
   
   return true;
}

OldNode*
OldExtraDataUtility::getNodeFromStrings(
                              OldGenericMap* theMap, 
                              CoordinateTransformer::format_t coordType,
                              const MC2String& coord1, const MC2String& coord2)

{
   int32 lat1, lon1, lat2, lon2;

   if (!strtolatlon(coord1, lat1, lon1, coordType))
      return NULL;
   if (!strtolatlon(coord2, lat2, lon2, coordType))
      return NULL;
   return theMap->getNodeFromCoordinates(lat1, lon1, lat2, lon2);
}


bool
OldExtraDataUtility::strtolatlon(const MC2String& s, int32& lat, int32& lon,
                              CoordinateTransformer::format_t coordType)
{
   return ExtraDataUtility::strtolatlon( s, lat, lon, coordType );
}

OldExtraDataUtility::record_t
OldExtraDataUtility::getRecordType(MC2String& s)
{
   // Initiate the dictionary (will only do anything the first time it 
   // is called).
   init();
   
   // Use the lookup-table
   recordMap_t::const_iterator i = m_recordTypes.find( s.c_str() );
   if (i == m_recordTypes.end()) {
      if ( (s.size() > 0) &&
           ( (s[0] == '#') || (s[0] == '%') ) ) {
         // Found a comment, return that
         return COMMENT;
      } else {
         // Unknown record-type
         return NUMBER_OF_RECORD_TYPES;
      }
   }

   return i->second;
}

OldExtraDataUtility::selectConn_t
OldExtraDataUtility::getSelectConns(MC2String& s)
{
   // Initiate the dictionary (will only do anything the first time it 
   // is called).
   init();
   
   // Use the lookup-table
   typedef selectConnMap_t::const_iterator CI;
   CI i = m_selectConns.find(s.c_str());
   if(i == m_selectConns.end()) {
      return OldExtraDataUtility::NOCONN;
   }
   return i->second;
   
}


int
OldExtraDataUtility::addLandmark(OldGenericMap* theMap, ItemTypes::itemType it,
         ItemTypes::pointOfInterest_t pt, uint32 importance,
         int32 lat, int32 lon, uint32 radius,
         int32 itemlat, int32 itemlon, const char* itemName,
         selectConn_t selConns)
{

   // The number of connections this landmark is tied to.
   int retVal = 0;

   // Check if the coordinates may be in this map or not.
   MC2BoundingBox bbox;
   theMap->getGfxData()->getMC2BoundingBox(bbox);
   if (!bbox.contains(lat, lon)) {
      mc2dbg8 << "Coordinate outside bbox" << endl;
      return 0;
   }
   // else, it may be in this map!

   int length = strlen(itemName);
   char* name = new char[length + 1];
   if (length > 0) {
      strcpy(name, itemName);
   } else {
      delete [] name;
      name = NULL;
   }
  
   switch(it) {
      case ItemTypes::railwayItem :
         retVal = addRailwayLandmark(theMap, it, importance,
               lat, lon, radius, selConns);
         break;
      case ItemTypes::parkItem :
      case ItemTypes::buildingItem :
      case ItemTypes::builtUpAreaItem :
      case ItemTypes::individualBuildingItem :
      case ItemTypes::pointOfInterestItem :
         retVal = addOtherLandmark(theMap, it, pt, importance,
               lat, lon, radius, itemlat, itemlon, name, selConns);
         break;
      default :
         mc2dbg << "OldItemtype (" << int(it) << ") not handled." << endl;
   }
   
  
   delete [] name;
   return retVal;
}


int
OldExtraDataUtility::addRailwayLandmark(
      OldGenericMap* theMap, ItemTypes::itemType it, uint32 importance, 
      int32 lat, int32 lon, uint32 radius, selectConn_t selConns)
{
   // The number of connections this landmark is tied to.
   int retVal = 0;
   
   // Get all close items, first railways and then ssi
   OldMapHashTable* mht = theMap->getHashTable();
   mht->clearAllowedItemTypes();
   mht->addAllowedItemType(it);
   bool shouldKill = false;
   Vector* tmpIDs = 
      mht->getAllWithinRadius_meter(lon, lat, radius, shouldKill);
   if (tmpIDs == NULL) {
      mc2dbg << "No close rail in map " << theMap->getMapID()
             << ", lat=" << lat << " lon=" << lon << endl;
      return 0;
   }
   Vector railIDs;
   // Copy from tmpIDs to railIDs
   for (uint32 j = 0; j < tmpIDs->getSize(); j++) {
      railIDs.addLast(tmpIDs->getElementAt(j));
   }
   if (shouldKill)
      delete tmpIDs;
   
   mht->clearAllowedItemTypes();
   mht->addAllowedItemType(ItemTypes::streetSegmentItem);
   tmpIDs = mht->getAllWithinRadius_meter(lon, lat, radius, shouldKill);
   if (tmpIDs == NULL) {
      mc2dbg << "No close ssi in map " << theMap->getMapID()
             << ", lat=" << lat << " lon=" << lon << endl;
      return 0;
   }
   Vector ssiIDs;
   for (uint32 j = 0; j < tmpIDs->getSize(); j++) {
      ssiIDs.addLast(tmpIDs->getElementAt(j));
   }
   if (shouldKill)
      delete tmpIDs;
   
   mc2dbg2 << "Got " << ssiIDs.getSize() << " street segments and "
           << railIDs.getSize() << " railways within the radius of " 
           << radius << " meters." << endl;
   // Return if any vector is empty
   if ( (railIDs.getSize() == 0) || (ssiIDs.getSize() == 0) )
      return 0;


   mc2dbg1 << "Landmark = railway intersection with " 
           << theMap->getFirstItemName(ssiIDs.getElementAt(0)) << endl;


   // Vector with checked ssiIDs 
   // (used if the rail is split up in the intersection, to make 
   // sure that the same ssi is not used twice.)
   Vector checkedssi;

   for (uint32 r = 0; r < railIDs.getSize(); r++) {
      uint32 riID = railIDs.getElementAt(r);
      mc2dbg2 << "railID " << riID << endl;
      OldRailwayItem* ri = dynamic_cast<OldRailwayItem*>(theMap->itemLookup(riID));
      for (uint32 s = 0; s < ssiIDs.getSize(); s++) {
         uint32 ssiID = ssiIDs.getElementAt(s);
         mc2dbg2 << "ssiID " << ssiID << ":" 
                 << theMap->getFirstItemName(ssiID) << endl;
         OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
            (theMap->itemLookup(ssiID));
         if ((ssi->getGfxData()->minSquareDistTo(ri->getGfxData()) == 0) &&
             (checkedssi.linearSearch(ssiID) == MAX_UINT32 )) {
            // We have an intersection, and the ssi has not already been
            // used for calculations
            checkedssi.addLast(ssiID);
            
            for (uint32 n = 0; n < 2; n++) {
               uint32 ssiNodeID = ssi->getNode(n)->getNodeID();
               OldNode* oppNode = ssi->getNode((n+1)%2);
               uint32 oppNodeID = oppNode->getNodeID();

               // If oppNode is in the intersection, the landmark should
               // not be tied to any of the connections from ssiNode.
               // Get the distance from oppNode to the intersection.
               int32 nodelat, nodelon;
               theMap->getNodeCoordinates(oppNodeID, nodelat, nodelon);
               float64 dist = sqrt(GfxUtility::squareP2Pdistance_linear(
                          lat, lon, nodelat, nodelon));
               mc2dbg4 << " dist oppnode(" << oppNodeID 
                       << ")-intersection: " << dist << endl;
               if (dist > 2) {
               //should be > 0, but since the coordinate in the candidate 
               //file so far is chosen manually...
                  for (uint16 c = 0; c < oppNode->getNbrConnections(); c++) {
                     OldConnection* oppConn = oppNode->getEntryConnection(c);
                     OldConnection* conn = 
                        theMap->getOpposingConnection(oppConn, oppNodeID);
                     uint32 toNodeID = 
                        oppConn->getConnectFromNode() ^ 0x80000000;
                     ItemTypes::turndirection_t turn = 
                        conn->getTurnDirection();
                     //fromNode == ssiNodeID
                     mc2dbg2 << " turn:" << int(turn) << " fromNode:" 
                             << conn->getConnectFromNode() 
                             << " toNode:" << toNodeID << endl;

                     // We have the correct connection, calculate lm descr
                     // If crossings, calculate for not undef, ahead, follow
                     if (( (selConns == ALLCONN) ||
                           (selConns == TURNCONN) ) &&
                         (turn != ItemTypes::UNDEFINED) &&
                         (turn != ItemTypes::FOLLOWROAD) &&
                         (turn != ItemTypes::AHEAD)) {
                        ItemTypes::lmdescription_t description;
                        description.itemID = riID;
                        description.importance = importance;
                        description.side = SearchTypes::undefined_side;
                        description.location = ItemTypes::after;
                        description.type = ItemTypes::railwayLM;
                        //add the description to the landmark table
                        uint64 key = (uint64(ssiNodeID) << 32) | 
                                      uint64(toNodeID);
                        theMap->getLandmarkTable().insert(
                              pair<uint64, ItemTypes::lmdescription_t>
                              (key, description));
                        retVal++;
                        mc2dbg2 << " --landmark added--" << endl;
                     }
                     //set pass for turns = ahead, followroad
                     if ((turn == ItemTypes::AHEAD) ||
                         (turn == ItemTypes::FOLLOWROAD)) {
                        ItemTypes::lmdescription_t description;
                        description.itemID = riID;
                        description.importance = importance;
                        description.side = SearchTypes::undefined_side;
                        description.location = ItemTypes::pass;
                        description.type = ItemTypes::railwayLM;

                        uint64 key = (uint64(ssiNodeID) << 32) |
                                      uint64(toNodeID);
                        theMap->getLandmarkTable().insert(
                              pair<uint64, ItemTypes::lmdescription_t>
                              (key, description));
                        retVal++;
                        mc2dbg2 << " --landmark added--" << endl;
                     }
                  }//for c
               }//if dist
            }//for n
            
         }//intersect and not used
         else {
            mc2dbg2 << "There is no intersection or the ssiID has "
                    << "already been used for calculations." << endl;
         }
      }//for s
   }//for r
   
   return retVal;

}


int
OldExtraDataUtility::addOtherLandmark(OldGenericMap* theMap,
      ItemTypes::itemType it, ItemTypes::pointOfInterest_t pt,
      uint32 importance, 
      int32 lat, int32 lon, uint32 radius, 
      int32 itemlat, int32 itemlon, const char* itemName,
      selectConn_t selConns)
{
   // The number of connections this landmark is tied to.
   int retVal = 0;
   
   // Get close street segment items
   OldMapHashTable* mht = theMap->getHashTable();
   mht->clearAllowedItemTypes();
   mht->addAllowedItemType(ItemTypes::streetSegmentItem);
   bool shouldKill = false;
   Vector* tmpIDs = 
      mht->getAllWithinRadius_meter(lon, lat, radius, shouldKill);
   //if vector is empty return
   if ((tmpIDs == NULL) || (tmpIDs->getSize() == 0)) {
      mc2dbg << "No close ssi in map " << theMap->getMapID()
             << " for " << itemName << ", lat=" << lat 
             << " lon=" << lon << endl;
      return 0;
   }
   // copy the IDs
   Vector ssiIDs;
   for (uint32 j = 0; j < tmpIDs->getSize(); j++) {
      ssiIDs.addLast(tmpIDs->getElementAt(j));
   }
   if (shouldKill)
      delete tmpIDs;

   mc2dbg2 << "Got " << ssiIDs.getSize() << " street segments within "
           << radius << " meters." << endl;

   
   //Get the item representing the landmark (bua, area, poi..)
   uint32 lmID = MAX_UINT32;
   if (it == ItemTypes::pointOfInterestItem) {
      vector<OldItem*> closeItems;
      OldItem* lmItem = theMap->getItemFromCoordinate(it, itemlat, itemlon,
            closeItems, itemName, 5, pt);
      if (lmItem == NULL) {
         mc2log << error << "No item (type " << int(it) 
                << ") found close to the itemcoord" << endl;
         return 0;
      }
      lmID = lmItem->getID();
   } else {
      mht->clearAllowedItemTypes();
      mht->addAllowedItemType(it);
      uint64 itemdist;
      lmID = mht->getClosest(itemlon, itemlat, itemdist);
      mc2dbg4 << "Distance from itemcoord to item "
              << ::sqrt(itemdist)*GfxConstants::MC2SCALE_TO_METER 
              << ", id:" << lmID << endl;
   }
   // Return if not found
   if (lmID == MAX_UINT32) {
      mc2log << error << "No item found close to the itemcoord" << endl;
      return 0;
   }
   
   
   // Some things if we have a bua landmark
   if (it == ItemTypes::builtUpAreaItem) {
      
      OldBuiltUpAreaItem* bua = 
         dynamic_cast<OldBuiltUpAreaItem*>(theMap->itemLookup(lmID));
      // How should the importance set in the candidate file be used???
      // If the importance in the file differ from the buaimportance, the 
      // file sure has been edited, and that importance should be used.

      // Do we have the correct bua? (e.g. Svedala/Sjödiken as well as 
      // Ljungbyhed/Spången share the bua boundry, so getClosest 
      // may return the wrong one)
      if (! bua->getGfxData()->insidePolygon(itemlat,itemlon)) {
         mc2log << error << "Wrong bua was found for itemcoord ("
                << itemlat << "," << itemlon << "). Found "
                << theMap->getFirstItemName(lmID) 
                << ", looking for " << itemName << endl;
         return 0;
      }

      //remove unwanted ssi's from the ssiIDs vector
      mht->clearAllowedItemTypes();
      mht->addAllowedItemType(ItemTypes::streetSegmentItem);
      uint64 ssidist;
      uint32 coordSsiID = mht->getClosest(lon,lat, ssidist);
      OldStreetSegmentItem* coordSsi = 
         dynamic_cast<OldStreetSegmentItem*>(theMap->itemLookup(coordSsiID));
      uint32 RC = coordSsi->getRoadClass();
      if (RC > 1) {
         mc2log << error << "Ssi with roadclass " << RC << " was found "
                << "closest to the ssi coordinate - returning 0" << endl;
         return 0;
      }
      mc2dbg4 << "wanted roadclass: " << RC 
              << " ssiID: " << coordSsiID << endl;
      for (uint32 s = 0; s < ssiIDs.getSize(); s++) {
         OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
            (theMap->itemLookup(ssiIDs.getElementAt(s)));
         DEBUG8(cout << " rc:" << int(ssi->getRoadClass()));
         if ((ssi->getRoadClass() != RC) ||
             (ssi->isRamp()) || (ssi->isRoundabout())) {
            ssiIDs.remove(ssiIDs.getElementAt(s));
            DEBUG8(cout << " removing" << endl);
            s--;
         }
      }
      ssiIDs.trimToSize();
      mc2dbg4 << "nbr ssi: " << ssiIDs.getSize() << endl;

      if ((radius == 25) && (ssiIDs.getSize() != 2)) {
         mc2log << warn << "The bua " << lmID << ":" << itemName 
                << " will be tied to " << ssiIDs.getSize() 
                << " street segments - should be 2" << endl;
      }

   }


   mc2dbg1 << "Landmark = " << theMap->getFirstItemName(lmID) << " tied to "
           << theMap->getFirstItemName(ssiIDs.getElementAt(0))
           << " (" << ssiIDs.getSize() << " close ssi's)" << endl;
   
   // The landmark type for this landmark
   ItemTypes::landmark_t lmtype = ItemTypes::areaLM;
   if (it == ItemTypes::builtUpAreaItem) {
      lmtype = ItemTypes::builtUpAreaLM;
   } else if (it == ItemTypes::pointOfInterestItem) {
      lmtype = ItemTypes::poiLM;
   }

   
   // Ssi coordinate (x) located on a street segment:
   // Tie the landmark to the street segment(s) in ssiIDs to the
   // connection that leads OVER the ssi.
   //
   //         <-------------- 
   //    -----o o------x-----o o-------
   //             -------------->
   //
   // If there are two IDs in ssiIDs we most likely have a freeway. 
   // It is no use to tie the landmark to any connections with noWay 
   // in the fromNode.
   //
   
   // Ssi coordinate (x) in a crossing:
   // 
   //         |                The landmark (*) will be tied to
   //         |                connections that leads from ssiNode
   //      *  o <---------     to nodes in the crossing.
   //   ----o x  o--------o     
   //         o           ^    
   //         |           ssiNode
   //         |
   //

   // When ssi coordinate (x) is on a street segment:
   //
   //           |  ssi coord (lat,lon)
   //           |     |
   //           o     v
   //     ----o   o---x---------o o----
   //           o ^             ^
   //  toNode / |  oppNode      ssiNode
   //           |
   
   
   // Find the connections, and add the landmark
   for (uint32 s = 0; s < ssiIDs.getSize(); s++) {
      OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
         (theMap->itemLookup(ssiIDs.getElementAt(s)));
      mc2dbg2 << "ssiID " << ssiIDs.getElementAt(s) << " "
              << theMap->getFirstItemName(ssiIDs.getElementAt(s)) << endl;
      for (uint32 n = 0; n < 2; n++) {
         OldNode* ssiNode = ssi->getNode(n);
         if (ssiNode->getEntryRestrictions() != ItemTypes::noWay) {
            // not in the wrong direction on a freeway
            OldNode* oppNode = ssi->getNode((n+1)%2);
            uint32 ssiNodeID = ssiNode->getNodeID();
            uint32 oppNodeID = oppNode->getNodeID();
            
            // If (lat, lon) is in a crossing and if ssiNode is in the 
            // crossing, the landmark should not be tied to any of its 
            // connections.
            // Get the distance from ssiNode to (lat, lon)
            int32 nodelat, nodelon;
            theMap->getNodeCoordinates(ssiNodeID, nodelat, nodelon);
            float64 dist = sqrt(GfxUtility::squareP2Pdistance_linear(
                       lat, lon, nodelat, nodelon));
            mc2dbg4 << n << "dist ssinode-(lat,lon): " << dist << endl;
            if (dist > 4) { // dist > 0
               // ssiNode is not in (near) a crossing given by (lat,lon)
               // oppNode might be
               
               for (uint16 c = 0; c < oppNode->getNbrConnections(); c++) {
                  OldConnection* oppConn = oppNode->getEntryConnection(c);
                  OldConnection* conn = 
                     theMap->getOpposingConnection(oppConn, oppNodeID);
                  uint32 toNodeID = 
                     oppConn->getConnectFromNode() ^ 0x80000000;
                 
                  // We have a connection leading from ssiNode to one node 
                  // in the crossing containing oppNode
                  // get the turndirection of conn
                  ItemTypes::turndirection_t turn = conn->getTurnDirection();
                  mc2dbg2 << "turn:" << int(turn) << " fromNode(=ssiNode):"
                          << ssiNodeID << " toNode:" << toNodeID << endl;

                  // calculate side and location
                  SearchTypes::side_t side = SearchTypes::undefined_side;
                  ItemTypes::landmarklocation_t location = 
                                          ItemTypes::undefinedlocation;
                  bool ok = true;
                  if (it != ItemTypes::builtUpAreaItem) {
                     ok = getLandmarkLocationAndSide(theMap,
                           conn, toNodeID, itemlat, itemlon, lmID,
                           lmtype, ssiIDs, location, side);
                  } else {
                     location = ItemTypes::pass;
                  }
                  mc2dbg2 << "location=" << int(location)
                          << " side=" << int(side) << endl;
                  
                  // Set the landmark descriptions and add to the LM table
                  if ( ok ) {
                     if ( ((selConns != AHEADCONN) &&
                           (it != ItemTypes::builtUpAreaItem) &&
                           (turn != ItemTypes::AHEAD) &&
                           (turn != ItemTypes::FOLLOWROAD))
                           ||
                          ((selConns != TURNCONN) &&
                           ((turn == ItemTypes::AHEAD) || 
                            (turn == ItemTypes::FOLLOWROAD))) ) {
                        
                        ItemTypes::lmdescription_t description;
                        description.itemID = lmID;
                        description.importance = importance;
                        description.side = side;
                        description.location = location;
                        description.type = lmtype;
                        // add the description to the landmark table
                        uint64 key = (uint64(ssiNodeID) << 32) | 
                                      uint64(toNodeID);
                        theMap->getLandmarkTable().insert(
                              pair<uint64, ItemTypes::lmdescription_t>
                              (key, description));
                        mc2dbg2 << " --landmark added--" << endl;
                        retVal++;

                     }
                  }
                  
               }//for c
            }//if dist == 0
         }
      }//for n
   }//for s


   return retVal;

}


bool
OldExtraDataUtility::getLandmarkLocationAndSide(
      OldGenericMap* theMap, OldConnection* conn, uint32 toNodeID,
      int32 itemlat, int32 itemlon, uint32 itemID, 
      ItemTypes::landmark_t lmType, Vector& ssiIDs,
      ItemTypes::landmarklocation_t& location, SearchTypes::side_t& side)
{
   mc2dbg4 << "getLandmarkLocationAndSide()" << endl;

   // Get and check crossingkind of this connection
   ItemTypes::crossingkind_t kind = conn->getCrossingKind();
   if ((kind != ItemTypes::NO_CROSSING) &&
       (kind != ItemTypes::CROSSING_3WAYS_T) &&
       (kind != ItemTypes::CROSSING_3WAYS_Y) &&
       (kind != ItemTypes::CROSSING_4WAYS) ) {
      return false;
   }
   mc2dbg4 << " crossingkind = " << int(kind) << endl;

   // Get and check turndirection of this connection
   ItemTypes::turndirection_t turn = conn->getTurnDirection();
   if (turn == ItemTypes::UNDEFINED) {
      return false;
   }

   // Get crossing coordinate (toNode in the crossing)
   int32 nodelat, nodelon;
   theMap->getNodeCoordinates(toNodeID, nodelat, nodelon);
 

   // Set location and side if the crossing is located IN the landmark
   if ( lmType != ItemTypes::poiLM) {
      if ( theMap->itemLookup(itemID)->
            getGfxData()->insidePolygon(nodelat, nodelon) == 2 ) {
         if ((turn == ItemTypes::AHEAD) || (turn == ItemTypes::FOLLOWROAD)) {
            location = ItemTypes::pass;
            side = SearchTypes::side_does_not_matter;
         } else {
            location = ItemTypes::in;
            side = SearchTypes::unknown_side; //side not needed
         }
         return true;
      }
   }

   
   // Get the fromSsi
   uint32 fromssiID = conn->getConnectFromNode() & 0x7fffffff;
   OldStreetSegmentItem* fromSsi = 
      dynamic_cast<OldStreetSegmentItem*>(theMap->itemLookup(fromssiID));
   uint32 fromNodeNbr = 1;
   if (theMap->nodeLookup(conn->getConnectFromNode())->isNode0())
      fromNodeNbr = 0;
   // Get the toSsi
   uint32 tossiID = toNodeID & 0x7fffffff;
   OldStreetSegmentItem* toSsi =
      dynamic_cast<OldStreetSegmentItem*>(theMap->itemLookup(tossiID));
   uint32 toNodeNbr = 1;
   if (theMap->nodeLookup(toNodeID)->isNode0())
      toNodeNbr = 0;
   mc2dbg8 << " fromssi " << fromssiID << " tossi " << tossiID << endl;

   // Check which ssi(s) the landmark is WITHIN and ON
   bool withinFrom = withinSsi(fromSsi, itemlat, itemlon, nodelat, nodelon);
   bool withinTo = withinSsi(toSsi, itemlat, itemlon, nodelat, nodelon);
   bool onFromSsi = onSsi(fromSsi, itemlat, itemlon);
   bool onToSsi = onSsi(toSsi, itemlat, itemlon);
   mc2dbg4 << "Within F:" << int(withinFrom) << " T:" << int(withinTo)
           << " OnSsi F:" << int(onFromSsi) << " T:" << int(onToSsi) << endl;

   
   // Set location and side for AHEAD and FOLLOWROAD
   if ((turn == ItemTypes::AHEAD) || (turn == ItemTypes::FOLLOWROAD)) {
      location = ItemTypes::pass;
      if (withinFrom) {
         side = getSide(fromSsi, fromNodeNbr, itemlat, itemlon);
      } else if (withinTo) {
         if ((lmType == ItemTypes::poiLM) && onToSsi)
            side = SearchTypes::unknown_side;
         else
            side = getSide(toSsi, toNodeNbr, itemlat, itemlon, false);
      } else {
         // within none of fromSsi and toSsi,
         //  - when a poi is not located on any of the fromSsi or toSsi
         side = getSide(fromSsi, fromNodeNbr, itemlat, itemlon, true, false);
      }

      return true;
   }

   // Set location and side for turns in 4-way crossing
   if (kind == ItemTypes::CROSSING_4WAYS) {
     
      //location
      if (withinFrom) {
         // crossing after the lm.
         location = ItemTypes::after;
      } else if ((lmType == ItemTypes::poiLM) &&
                 (!withinFrom && withinTo && onToSsi)) {
         // poi on the toSsi
         location = ItemTypes::at;
      } else if ((lmType == ItemTypes::poiLM) &&
                  (!withinFrom && !withinTo &&
                   getSide(fromSsi, fromNodeNbr, itemlat, itemlon, 
                          true, false) != SearchTypes::unknown_side) ) {
         // poi not just in front of fromSsi
         location = ItemTypes::at;
      } else {
         // crossing before the lm.
         location = ItemTypes::before;
      }

      //side
      if ((lmType == ItemTypes::poiLM) && (withinFrom && onFromSsi) ) { 
         // the poi is on the fromssi
         side = SearchTypes::unknown_side;
      } else if ((lmType == ItemTypes::poiLM) && !withinFrom && !withinTo) {
         // the poi is in front of or "beside" the crossingconn
         side = getSide(fromSsi, fromNodeNbr, itemlat, itemlon, true, false);
      } else {
         
         if (withinTo && (turn == ItemTypes::LEFT)) {
            side = SearchTypes::left_side;
         } else if (withinTo && (turn == ItemTypes::RIGHT)) {
            side = SearchTypes::right_side;
         } else if (!withinTo && (turn == ItemTypes::LEFT)) {
            side = SearchTypes::right_side;
         } else if (!withinTo && (turn == ItemTypes::RIGHT)) {
            side = SearchTypes::left_side;
         } else {
            // any other turn
            side = SearchTypes::unknown_side;
         }
      }

      return true;
   }
   
   // To set location and side for turns in 3-way T-crossing
   // - also for "T-like" Y-crossings...
   // First make sure we have all ssi's in the crossing
   Vector checkIDs;
   OldConnection* tmpConn = NULL;
   if ( (ssiIDs.getSize() == 1) ||   // 1 -> ssi coord on segment
        ((ssiIDs.getSize() == 2) &&  // 2 -> ssi coord on segment "freeway"
         (theMap->getConnection(ssiIDs.getElementAt(0),
                        ssiIDs.getElementAt(1), tmpConn) == MAX_UINT32) ) ) {
      mc2dbg4 << "Have to get ssiIDs again" << endl;
      //The ssi coordinate was on a ssi, not in a crossing,
      //get the ssi's in the crossing
      OldMapHashTable* mht = theMap->getHashTable();
      mht->clearAllowedItemTypes();
      mht->addAllowedItemType(ItemTypes::streetSegmentItem);
      bool shouldKill = false;
      Vector* tmpIDs =
         mht->getAllWithinRadius_meter(nodelon, nodelat, 1, shouldKill);
      for (uint32 j = 0; j < tmpIDs->getSize(); j++) {
         checkIDs.addLast(tmpIDs->getElementAt(j));
      }
      if (shouldKill)
         delete tmpIDs;
   } else {
      for (uint32 j = 0; j < ssiIDs.getSize(); j++) {
         checkIDs.addLast(ssiIDs.getElementAt(j));
      }
   }
   // Then check if 3-way Y-crossing is T-like.
   bool tlike = false;
   if (kind == ItemTypes::CROSSING_3WAYS_Y) {
      tlike = crossingTlike(theMap, checkIDs);
      if (tlike)
         mc2dbg4 << "T-LIKE 3way-Y crossing" << endl;
   }
   
   // Set location and side for turns in 3-way T-crossing
   // - also for "T-like" Y-crossings...
   if ((kind == ItemTypes::CROSSING_3WAYS_T) || tlike) {
      // This turn != (ahead and followroad and undefined)
      
      if (checkIDs.getSize() != 3) {
         mc2log << warn << "Checking 3WAYS crossing and have "
                << checkIDs.getSize() << " ssi's!" << endl;
      }
      
      // Get the other connection from fromSsi (not to toSsi)
      OldConnection* otherconn = NULL;
      bool onOther = false;
      uint32 s = 0;
      bool found = false;
      while (!found && (s < checkIDs.getSize())) {
         uint32 ID = checkIDs.getElementAt(s);
         if ( (ID != fromssiID) && (ID != tossiID)) {
            theMap->getConnection(fromssiID, ID, otherconn);
            found = true;
            onOther = onSsi(
                  static_cast<OldStreetSegmentItem*>(theMap->itemLookup(ID)),
                  itemlat, itemlon);
         } else {
            s++;
         }
      }
      MC2_ASSERT(otherconn != NULL);
      ItemTypes::turndirection_t otherturn = otherconn->getTurnDirection();
      
      // Distance from the landmark to the crossing (toNode = nodelat,nodelon)
      float64 dist = sqrt(GfxUtility::squareP2Pdistance_linear(
               itemlat, itemlon, nodelat, nodelon));
      SearchTypes::side_t tmpside = getSide(toSsi, toNodeNbr, 
            itemlat, itemlon, false, withinTo);
      mc2dbg4 << " dist toNode-landmarkitemlat,lon = " << dist
              << " tmpside= " << int(tmpside) << endl;

      // Location
      if ((lmType == ItemTypes::poiLM) &&
          (onToSsi || 
           (onOther &&
            ((otherturn != ItemTypes::AHEAD) &&
             (otherturn != ItemTypes::FOLLOWROAD))) ) ) {
         location = ItemTypes::at;
      } else if ((lmType == ItemTypes::poiLM) && onFromSsi) {
         location = ItemTypes::after;

      } else if ( (!withinTo) && 
           ((otherturn == ItemTypes::AHEAD) ||
            (otherturn == ItemTypes::FOLLOWROAD)) &&
           ((dist < 60) || (tmpside == SearchTypes::unknown_side)) ) {
         // conn comes from one of the "roofs" of the T-crossing and 
         // the landmark is located on top of the T close to the 
         // crossing (if the landmark is "just in front of" the T-leg,
         // distance should not matter) -> turn AT the landmark
         // How close is close???
         location = ItemTypes::at;
      } else if (withinFrom) {
         location = ItemTypes::after;
      } else if (!withinFrom) {
          if ( (otherturn != ItemTypes::AHEAD) &&
               (otherturn != ItemTypes::FOLLOWROAD) &&
               (otherturn != ItemTypes::UNDEFINED) && 
               (dist > 60)) {
             // conn cones from the leg of the T and the landmark is 
             // located on top of the T, and far away from the crossing
             // Don't set the location or side (side = unknown anyway)
             // How far is far???
             return false;
          } else {
             location = ItemTypes::before;
          }
      } else {
         // won't happen?
         location = ItemTypes::at;
      }

      // Side
      if ((lmType == ItemTypes::poiLM) && onFromSsi) {
         side = SearchTypes::unknown_side;
      } else if ((lmType == ItemTypes::poiLM) && onOther){
         if (otherturn == ItemTypes::LEFT)
            side = SearchTypes::left_side;
         else if (otherturn == ItemTypes::RIGHT)
            side = SearchTypes::right_side;
         else //ahead or followroad
            side = SearchTypes::unknown_side;
      } else if ((lmType == ItemTypes::poiLM) && onToSsi) {
         if (turn == ItemTypes::RIGHT)
            side = SearchTypes::right_side;
         else if (turn == ItemTypes::LEFT)
            side = SearchTypes::left_side;
         //else ahead already managed
      }
         
      else if (!withinFrom &&
          ( (otherturn != ItemTypes::AHEAD) &&
            (otherturn != ItemTypes::FOLLOWROAD) &&
            (otherturn != ItemTypes::UNDEFINED))) {
         // conn comes from the leg of the T-crossing and the landmark is 
         // located on top of the T -> side = unknown
         side = SearchTypes::unknown_side;
      } else if (withinTo && (turn == ItemTypes::LEFT)) {
         side = SearchTypes::left_side;
      } else if (withinTo && (turn == ItemTypes::RIGHT)) {
         side = SearchTypes::right_side;
      } else if (!withinTo && (turn == ItemTypes::LEFT)) {
         side = SearchTypes::right_side;
      } else if (!withinTo && (turn == ItemTypes::RIGHT)) {
         side = SearchTypes::left_side;
      } else {
         // any other turn
         side = SearchTypes::unknown_side;
      }

      return true;
   }

   // Set location and side for turns in 3-way Y-crossing
   if (kind == ItemTypes::CROSSING_3WAYS_Y) {
      location = ItemTypes::at;

      if (withinFrom) {
         side = getSide(fromSsi, fromNodeNbr, itemlat, itemlon);
      } else {
         side = SearchTypes::unknown_side;
      }

      return true;
   }


   return false;
}

bool
OldExtraDataUtility::withinSsi(
      OldStreetSegmentItem* ssi, int32 itemlat, int32 itemlon,
      int32 crossinglat, int32 crossinglon)
{
   // The item (itemlat,itemlon) is within the ssi if the coordinate
   // on the ssi, that is closest to the item coordinate, is not in 
   // the crossing coordinate.
   // To avoid problems with long and curved street segments, use only 
   // the subsegment of ssi that is closest to the crossing coordinate.
   // (Must be ok...)
   
   bool retVal = true;

   // get the coord on ssi subsegment, that is closest to the item coord.
   int32 startlat, startlon, endlat, endlon;
   MC2_ASSERT(ssi->getGfxData()->getNbrPolygons() == 1);
   uint32 nbrCoords = ssi->getGfxData()->getNbrCoordinates(0);
   startlat = ssi->getGfxData()->getLat(0,0);
   startlon = ssi->getGfxData()->getLon(0,0);
   endlat = ssi->getGfxData()->getLastLat(0);
   endlon = ssi->getGfxData()->getLastLon(0);

   if ((startlat == crossinglat) && (startlon == crossinglon)) {
      endlat = ssi->getGfxData()->getLat(0,1);
      endlon = ssi->getGfxData()->getLon(0,1);
   } else if ((endlat == crossinglat) && (endlon == crossinglon)) {
      startlat = ssi->getGfxData()->getLat(0,nbrCoords-2);
      startlon = ssi->getGfxData()->getLon(0,nbrCoords-2);
   }

   const float64 coslat = cos( (float64) (itemlat) * M_PI / 
                               ((float64) 0x80000000) );
   int32 closelat, closelon;
   GfxUtility::closestDistVectorToPoint(
         startlon, startlat, endlon, endlat, itemlon, itemlat,
         closelon, closelat, coslat);

   // if closelat,closelon is in the crossing return false
   if ((closelat == crossinglat) && (closelon == crossinglon)) {
      retVal = false;
   }

   mc2dbg8 << " start (" << startlat << "," << startlon 
           << ") end (" << endlat << "," << endlon << ")" << endl
           << " crossing (" << crossinglat << "," << crossinglon 
           << ") close (" << closelat << "," << closelon << endl;

   mc2dbg8 << "withinSsi returning=" << int (retVal) << endl;

   return retVal;
}

bool
OldExtraDataUtility::onSsi(OldStreetSegmentItem* ssi, int32 itemlat, int32 itemlon)
{
   bool retVal = false;
   
   //loop every cordinate pair of the ssi
   uint64 dist = MAX_UINT64;
   const float64 coslat = cos( (float64) (itemlat) * M_PI / 
                                  ((float64) 0x80000000) );
   MC2_ASSERT(ssi->getGfxData()->getNbrPolygons() == 1);
   for (uint32 coord = 0; 
        coord < ssi->getGfxData()->getNbrCoordinates(0)-1; coord ++) {
      int32 startlat, startlon, endlat, endlon;
      startlat = ssi->getGfxData()->getLat(0,coord);
      startlon = ssi->getGfxData()->getLon(0,coord);
      endlat = ssi->getGfxData()->getLat(0,coord+1);
      endlon = ssi->getGfxData()->getLon(0,coord+1);

      uint64 tmpdist = GfxUtility::closestDistVectorToPoint(
            startlon, startlat, endlon, endlat, itemlon, itemlat,
            coslat);
      if (tmpdist < dist)
         dist = tmpdist;
   }
   if (dist == 0) {
      retVal = true;
   }

   return retVal;
}

SearchTypes::side_t
OldExtraDataUtility::getSide(OldStreetSegmentItem* ssi,
      uint32 nodeNbr,int32 itemlat, int32 itemlon,
      bool fromSide, bool within)
{
   // value to return
   SearchTypes::side_t side = SearchTypes::unknown_side;

   // If the item coordinate is ON the fromSsi (e.g. a poi) there is no
   // side to calculate. Return unknown side.
   if (fromSide && onSsi(ssi, itemlat, itemlon)) {
      return side;
   }
      

   // Get start and end coordinates for the last/first segment of 
   //                      the ssi (calculating fromSide = the 
   //          | toSide    two last coordinates, calculating 
   //       --- -----      toSide = the two first coordinates). 
   //          |  -->      Direction always counted from nodeNbr.
   //    from  | |
   //    Side  | | 
   //          
   MC2_ASSERT(ssi->getGfxData()->getNbrPolygons() == 1);
   uint32 from, to;
   if (fromSide) {
      from = 1;
      to = 0;
      if (nodeNbr == 0) {
         from = ssi->getGfxData()->getNbrCoordinates(0) - 2;
         to = ssi->getGfxData()->getNbrCoordinates(0) - 1;
      }
   } else {
      from = 0;
      to = 1;
      if (nodeNbr == 1) {
         from = ssi->getGfxData()->getNbrCoordinates(0) - 1;
         to = ssi->getGfxData()->getNbrCoordinates(0) - 2;
      }
   }
    
   int32 fromlat = ssi->getGfxData()->getLat(0,from);
   int32 fromlon = ssi->getGfxData()->getLon(0,from);
   int32 tolat = ssi->getGfxData()->getLat(0,to);
   int32 tolon = ssi->getGfxData()->getLon(0,to);

   // Get angles for the ssi and the point (relative the start of the ssi)
   float64 ssiAngle = 
      GfxUtility::getAngleFromNorth(fromlat, fromlon, tolat, tolon);
   float64 pointAngle =
      GfxUtility::getAngleFromNorth(fromlat, fromlon, itemlat, itemlon);

   mc2dbg8 << " ssiAngle=" << ssiAngle << " pointAngle="
           << pointAngle << endl;

   // Compensate for big difference in angle
   float64 angleDiff = pointAngle - ssiAngle;
   if ((angleDiff) < -M_PI) {
      pointAngle = pointAngle + (2*M_PI);
   }
   if ((angleDiff) > M_PI) {
      ssiAngle = ssiAngle + (2*M_PI);
   }
   // update angle diff
   angleDiff = pointAngle - ssiAngle;

   // Now decide the side 
   // - if the point is not "within" the ssi and the difference in angle 
   // is very small (the point is located just in front of or right 
   // behind the ssi)  -> unknown side. How small is small???
   float64 smalldiff = 10*GfxConstants::degreeToRadianFactor;
   mc2dbg8 << "(" << angleDiff << ":" << smalldiff << "," 
           << M_PI+smalldiff << "," << M_PI-smalldiff << ")" << endl;
   if ( !within && 
       ((abs(angleDiff) < smalldiff) ||
        ((abs(angleDiff) < (M_PI + smalldiff)) && 
         (abs(angleDiff) > (M_PI - smalldiff)) ) ) ) {
      mc2dbg8 << " angle diff small, side = unknown" << endl;
      return (SearchTypes::unknown_side);
   }
   
   if (angleDiff > 0) { //pointAngle > ssiAngle
      side = SearchTypes::right_side;
   } else {             //pointAngle < ssiAngle
      side = SearchTypes::left_side;
   }

   mc2dbg8 << " side " << int(side) << endl;
   
   return side;
}

bool
OldExtraDataUtility::crossingTlike(OldGenericMap* theMap, Vector& ssiIDs)
{
   bool retVal = false;
   uint32 nbrSsi = ssiIDs.getSize();
   if (nbrSsi == 3) {

      // Collect all connections between the ssi's
      ItemTypes::turndirection_t turnvec[3][3];

      for (uint32 i = 0; i < nbrSsi; i++) {
         for (uint32 j = 0; j < nbrSsi; j++) {
            if (i != j) {
               OldConnection* conn = NULL;
               theMap->getConnection(ssiIDs.getElementAt(i), 
                                     ssiIDs.getElementAt(j),
                                     conn);
               turnvec[i][j] = conn->getTurnDirection();
            }
         }
      }

      // check if any pair of connection has turndesc ahead|followroad
      for (uint32 i = 0; i < nbrSsi; i++) {
         for (uint32 j = 0; j < nbrSsi; j++) {
            if (i != j) {
               if ( ((turnvec[i][j] == ItemTypes::AHEAD) || 
                     (turnvec[i][j] == ItemTypes::FOLLOWROAD)) &&
                    ((turnvec[j][i] == ItemTypes::AHEAD) ||
                     (turnvec[j][i] == ItemTypes::FOLLOWROAD)) ) {
                  return true;
               }
            }
         }
      }
   }


   return retVal;
}


bool
OldExtraDataUtility::addSingleLandmark(OldGenericMap* theMap, 
         OldItem* lmItem, uint32 importance,
         int location, int side, OldNode* fromNode, OldNode* toNode)
{

   // get nodeids
   uint32 fromNodeID = fromNode->getNodeID();
   uint32 toNodeID = toNode->getNodeID();
   
   // get location and side (types)
   ItemTypes::landmarklocation_t lmloc = ItemTypes::undefinedlocation;
   SearchTypes::side_t lmside = SearchTypes::undefined_side;
   if (location > -1)
      lmloc = ItemTypes::landmarklocation_t(location);
   if (side > -1)
      lmside = SearchTypes::side_t(side);

   // extract landmark type
   ItemTypes::itemType it = lmItem->getItemType();
   ItemTypes::landmark_t lmtype;
   switch (it) {
      case ItemTypes::builtUpAreaItem :
         lmtype = ItemTypes::builtUpAreaLM;
         break;
      case ItemTypes::railwayItem :
         lmtype = ItemTypes::railwayLM;
         break;
      case ItemTypes::parkItem :
      case ItemTypes::buildingItem :
      case ItemTypes::individualBuildingItem :
         lmtype = ItemTypes::areaLM;
         break;
      case ItemTypes::pointOfInterestItem :
         lmtype = ItemTypes::poiLM;
         break;
      default :
         mc2dbg << "OldItemtype (" << int(it) << ") not handled." << endl;
         return false;
   }

   // create the landmark description
   ItemTypes::lmdescription_t description;
   description.itemID = lmItem->getID();
   description.importance = importance;
   description.side = lmside;
   description.location = lmloc;
   description.type = lmtype;

   // create the key and insert into the lm table.
   uint64 key = (uint64(fromNodeID) << 32) | uint64(toNodeID);
   theMap->getLandmarkTable().insert(
         pair<uint64, ItemTypes::lmdescription_t>(key, description));
      
   return true;
}

MC2String
OldExtraDataUtility::createEDFileString( vector<MC2String>& subStrings,
                                      bool addEndOfRec /* = false */)
{
   MC2String result = "";

   // Append all sub strings to result
   for ( vector<MC2String>::iterator it = subStrings.begin();
         it != subStrings.end(); it++ ) {
      
      // append to result
      result += *it;
      result += edFieldSep;
      
   }

   if ( addEndOfRec ) {
      result += END_OF_RECORD;
      result += edFieldSep;
   }
   
   mc2dbg8 << "string: '" << result << "'" << endl;

   return result;
}


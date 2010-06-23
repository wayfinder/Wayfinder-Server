/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSStreetSegmentItem.h"
#include "MidMifData.h"
#include "OldGenericMap.h"
#include "GMSMidMifHandler.h"
#include "Math.h"

#include "Utility.h"

GMSStreetSegmentItem::GMSStreetSegmentItem()
   : OldStreetSegmentItem(MAX_UINT32),
     GMSItem(GMSItem::INVALID)
{
   initMembers();
   m_map = NULL;
}


GMSStreetSegmentItem::GMSStreetSegmentItem( OldGenericMap* map ) 
   : OldStreetSegmentItem(),
     GMSItem(GMSItem::INVALID)
{
   initMembers();
   m_map = map;
}

GMSStreetSegmentItem::GMSStreetSegmentItem(OldGenericMap* map, 
                                           GMSStreetSegmentItem* ssi)
   : OldStreetSegmentItem(),
     GMSItem(GMSItem::INVALID)
{
   initMembers();

   m_map = map;
   
   m_node0 = NULL;
   m_node1 = NULL;
   copyAttributes(ssi);
}


GMSStreetSegmentItem::GMSStreetSegmentItem( OldGenericMap *map, 
                                            uint32 id, 
                                            GMSItem::map_t mapType )
                     : OldStreetSegmentItem(),
                       GMSItem(mapType)
{
   // Note that we might experience some troubles with other programs 
   // using this constructor
   // The previous version called OldStreetSegmentItem(id).

   initMembers();

   m_map = map;
   m_localID = id;

   if ( m_map != NULL ) {
      m_node0 = static_cast<GMSNode*>(getNewNode(m_map));
      m_node0->setNodeID(id & 0x7FFFFFFF);
      m_node1 = static_cast<GMSNode*>(getNewNode(m_map));
      m_node1->setNodeID(id | 0x80000000);
   } else {
      m_node0 = new GMSNode(id & 0x7FFFFFFF);
      m_node1 = new GMSNode(id | 0x80000000);
   }
}

GMSStreetSegmentItem::GMSStreetSegmentItem(uint32 id, OldGenericMap* map)
   : OldStreetSegmentItem(id),
     GMSItem(GMSItem::INVALID)
{
   initMembers();
   m_map = map;
}

GMSStreetSegmentItem::GMSStreetSegmentItem(uint32 id)
   : OldStreetSegmentItem(id),
     GMSItem(GMSItem::INVALID)
{
   initMembers();
   m_map = NULL;
}

void
GMSStreetSegmentItem::initMembers()
{
   init(ItemTypes::streetSegmentItem);

   // Initialization copied from the OldStreetSegmentItem(id) constructor
   m_condition = 0;
   m_roadClass = 4;
   m_width = 0;
   m_streetNumberType = ItemTypes::noStreetNumbers;

   m_leftsideNumberStart = 0;
   m_leftsideNumberEnd = 0;

   m_rightsideNumberStart = 0;
   m_rightsideNumberEnd = 0;

   m_roundabout = false;
   m_roundaboutish = false;
   m_ramp = false;
   m_divided = false;
   m_multiDigitised = false;
   m_controlledAccess = false;
   m_tempRoadClass = MAX_UINT32;
   m_tempNet2Class = MAX_UINT32;
   m_tempFormOfWay = MAX_UINT32;
   m_tempSpecRestriction = ItemTypes::noRestrictions;
   m_tempStubble = false;
   m_tempBackroad = MAX_UINT32;
   m_tempConstructionStatus = MAX_UINT32;
   m_tempNoThroughTraffic = false;
   m_tempRoughRoad = false;
}


GMSStreetSegmentItem::~GMSStreetSegmentItem()
{

   vector< pair<const char*, const char*> >::const_iterator it = m_zips.begin();
   while (it != m_zips.end()) {
      delete (*it).first;
      delete (*it).second;
      ++it;
   }

}

bool
GMSStreetSegmentItem::createFromMidMif(ifstream& midFile, bool readRestOfLine)
{
   const uint32 maxLineLength = GMSMidMifHandler::maxMidLineLength;
   
   //create empty nodes
   if ( m_node0 == NULL ) {
      if ( m_map != NULL ) {
         m_node0 = static_cast<GMSNode*>(getNewNode(m_map));
      } else {
         m_node0 = new GMSNode();
      }
   }
   if ( m_node1 == NULL ) {
      if ( m_map != NULL ) {
         m_node1 = static_cast<GMSNode*>(getNewNode(m_map));
      } else {
         m_node1 = new GMSNode();
      }
   }
   
   char inbuffer[maxLineLength];
   char* tmpchr;

   MidMifData* midmifData = dynamic_cast<MidMifData*>(getMapData());
   if ( midmifData == NULL ) {
      mc2log << error << here 
             << " mapData is not a  midmifData or NULL." << endl;
      MC2_ASSERT(midmifData != NULL);
   }

   //roadClass
   //read past first ','
   midFile.getline(inbuffer,maxLineLength, ',');
   //read the roadclass value
   midFile.getline(inbuffer,maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int tmpRoadClass = strtol(inbuffer, &tmpchr, 10);
      if ( (tmpRoadClass > 4 ) || (tmpRoadClass < 0) ) {
         mc2log << error << "Incorrect road class " << tmpRoadClass
                << " (itemID=" << this->getID() << endl;
      }
      mc2dbg8 << "  RoadClass = " << tmpRoadClass << endl;
      m_roadClass = (byte) tmpRoadClass;
   }
   
   //posspeed
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int32 posSpeed = strtol(inbuffer, &tmpchr, 10);
      if (posSpeed < 0) {
         posSpeed = -1;
         ItemTypes::entryrestriction_t restriction = ItemTypes::noWay;
         getNode(0)->setEntryRestrictions(restriction);
         mc2dbg4 << " setting entryrestrictions to NoWay due to "
                     << "pos-speed " << posSpeed << endl;
      }
      mc2dbg8 << "  Posspeed = " << posSpeed << endl;
      getNode(0)->setSpeedLimit( (uint8) posSpeed);
   }
   
   //negspeed
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int32 negSpeed = strtol(inbuffer, &tmpchr, 10);
      if (negSpeed < 0) {
         negSpeed = -1;
         ItemTypes::entryrestriction_t restriction = ItemTypes::noWay;
         getNode(1)->setEntryRestrictions(restriction);
         mc2dbg4 << " setting entryrestrictions to NoWay due to "
                 << "neg-speed " << negSpeed << endl;
      }
      mc2dbg8 << "  Negspeed = " << negSpeed << endl;
      getNode(1)->setSpeedLimit( (uint8) negSpeed);
   }

   //pos entryrestrictions (entering from node 0)
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int tmpPosRes = strtol(inbuffer, &tmpchr, 10);
      if ( (tmpPosRes >= 0) && (tmpPosRes < 4)) {
         ItemTypes::entryrestriction_t restriction = 
            ItemTypes::entryrestriction_t (tmpPosRes);
         getNode(0)->setEntryRestrictions(restriction);
      }
   }

   //neg entryrestrictions (entering from node 1)
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int tmpNegRes = strtol(inbuffer, &tmpchr, 10);
      if ( (tmpNegRes >= 0) && (tmpNegRes < 4)) {
         ItemTypes::entryrestriction_t restriction = 
            ItemTypes::entryrestriction_t (tmpNegRes);
         getNode(1)->setEntryRestrictions(restriction);
      }
   }

   //nbrLanes in each direction
   midFile.getline(inbuffer, maxLineLength, ',');
   mc2dbg8 << "  Not using NbrLanes = " << inbuffer << endl;
 
   //width
   midFile.getline(inbuffer, maxLineLength, ',');
   if ( strlen(inbuffer) > 0 ) {
      int tmpWidth = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  Width = " << tmpWidth << endl;
      m_width = (uint8) tmpWidth;
   }
   
   //maxHeight
   midFile.getline(inbuffer, maxLineLength, ',');
   if ( strlen(inbuffer) > 0 ) {
      int tmpHeight = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  Height = " << tmpHeight << endl;
      getNode(0)->setMaximumHeight( (uint8) tmpHeight);
      getNode(1)->setMaximumHeight( (uint8) tmpHeight);
   }
      
   //maxWeight
   midFile.getline(inbuffer, maxLineLength, ',');
   if ( strlen(inbuffer) > 0 ) {
      int tmpWeight = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  Weight = " << tmpWeight << endl;
      getNode(0)->setMaximumWeight( (uint8) tmpWeight);
      getNode(1)->setMaximumWeight( (uint8) tmpWeight);
   }
   
   //leftsideNumberStart
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpLeftStart = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  From left = " << tmpLeftStart << endl;
      setLeftSideNumberStart( (uint16) tmpLeftStart);
   }
   
   //leftsideNumberEnd
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpLeftEnd = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  To left = " << tmpLeftEnd << endl;
      setLeftSideNumberEnd( (uint16) tmpLeftEnd);
   }
   
   //rightsideNumberStart
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpRightStart = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  From right = " << tmpRightStart << endl;
      setRightSideNumberStart( (uint16) tmpRightStart);
   }
   
   //rightsideNumberEnd
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpRightEnd = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  To right = " << tmpRightEnd << endl;
      setRightSideNumberEnd( (uint16) tmpRightEnd);
   }

   //set the streetNumberType
   //works even if no houseNumbers have been set (inital value for them = 0)
   ItemTypes::streetNumberType nbrType = 
      ItemTypes::getStreetNumberTypeFromHouseNumbering(
            getLeftSideNbrStart(),
            getLeftSideNbrEnd(),
            getRightSideNbrStart(),
            getRightSideNbrEnd());
   mc2dbg8 << "  setting StreetNumberType = " << (int) nbrType << endl;
   setStreetNumberType(nbrType);

   //paved
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer))
      setCondition(ItemTypes::pavedStreet);
   else
      setCondition(ItemTypes::unpavedStreet);

   //level node 0
   midFile.getline(inbuffer, maxLineLength, ',');
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpLevel = strtol(inbuffer, &tmpchr, 10);
      getNode(0)->setLevel(tmpLevel);
   }

   //level node 1
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpLevel = strtol(inbuffer, &tmpchr, 10);
      getNode(1)->setLevel(tmpLevel);
   }

   //roundabout
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer)) {
      setRoundaboutValue(true);
   }
 
   //ramp
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer)) {
      setRampValue(true);
   }
 
   //divided
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer)) {
      setDividedValue(true);
   }
 
   //multidigitalized
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer)) {
      setMultiDigitisedValue(true);
   }
 
   //roadToll
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer)) {
      getNode(0)->setRoadToll(true);
      getNode(1)->setRoadToll(true);
   }
 
   //controlled access
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   if (Utility::getBoolFromString(inbuffer)) {
      setControlledAccessValue(true);
   }

   // Check if we have any extra attributes to read,
   // don't count any map ssi coordinates
   uint32 nbrExtraAttributes = 
         GMSMidMifHandler::getNumberExtraAttributes( midFile, !readRestOfLine);

   // Read any extra item attributes
   // Extra: Roundaboutish
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if (Utility::getBoolFromString(inbuffer)) {
         setRoundaboutishValue(true);
      }
      nbrExtraAttributes--;
   }
   // Extra: left zip code
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if( strlen(inbuffer) > 0 ) {
         addTmpZipCode(inbuffer, NULL);
      }
      nbrExtraAttributes--;
   }
   // Extra: right zip code
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if( strlen(inbuffer) > 0 ) {
         addTmpZipCode(inbuffer, NULL);
      }
      nbrExtraAttributes--;
   }
   // Extra: left settlement
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, ',');
      midFile.getline(inbuffer, maxLineLength, ',');
      if ( strlen(inbuffer) > 0 ) {
         uint32 tmpLeftSettlement = strtoul(inbuffer, &tmpchr, 10);
         midmifData->setLeftSettlement( tmpLeftSettlement );
      }
      nbrExtraAttributes--;
   }
   // Extra: right settlement
   if ( nbrExtraAttributes > 0 ) {
      // If this is the last attribute in the mid row there is no more 
      // "," char to read
      if ( (nbrExtraAttributes == 1) && readRestOfLine ) {
         midFile.getline(inbuffer, maxLineLength);
         readRestOfLine = false;
      } else {
         midFile.getline(inbuffer, maxLineLength, ',');
      }
      if ( strlen(inbuffer) > 0 ) {
         uint32 tmpRightSettlement = strtoul(inbuffer, &tmpchr, 10);
         midmifData->setRightSettlement( tmpRightSettlement );
      }
      nbrExtraAttributes--;
   }
   // Extra: settlement order
   if ( nbrExtraAttributes > 0 ) {
      // If this is the last attribute in the mid row there is no more 
      // "," char to read
      if ( (nbrExtraAttributes == 1) && readRestOfLine ) {
         midFile.getline(inbuffer, maxLineLength);
         readRestOfLine = false;
      } else {
         midFile.getline(inbuffer, maxLineLength, ',');
      }
      if ( strlen(inbuffer) > 0 ) {
         uint32 tmpSettlementOrder = strtoul(inbuffer, &tmpchr, 10);
         midmifData->setSettlementOrder( tmpSettlementOrder );
      }
      nbrExtraAttributes--;
   }
   // Extra: node 0 border node
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if (Utility::getBoolFromString(inbuffer)) {
         mc2dbg8 << " node 0 border node: " << inbuffer << endl;
         getNode(0)->setJunctionType( ItemTypes::borderCrossing );
      }
      nbrExtraAttributes--;
   }
   // Extra: node 1 border node
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if (Utility::getBoolFromString(inbuffer)) {
         mc2dbg8 << " node 1 border node: " << inbuffer << endl;
         getNode(1)->setJunctionType( ItemTypes::borderCrossing );
      }
      nbrExtraAttributes--;
   }
   // Extra: road display class
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, ',');
      // If this is the last attribute in the mid row there is no more 
      // "," char to read
      if ( (nbrExtraAttributes == 1) && readRestOfLine ) {
         midFile.getline(inbuffer, maxLineLength);
         readRestOfLine = false;
      } else {
         midFile.getline(inbuffer, maxLineLength, ',');
      }
      if ( strlen(inbuffer) > 0 ) {
         int tmpDisplayClass = strtol(inbuffer, &tmpchr, 10);
         mc2dbg8 << "extra road display class inbuffer=" << inbuffer
                 << " -> tmpDisplayClass=" << tmpDisplayClass << endl;
         // Only collect the road display classes that are within
         // the enum range. Value -1 means invalid value, i.e. the street
         // segment has no special display class
         if ( (tmpDisplayClass >=0) && 
              (tmpDisplayClass < int(ItemTypes::nbrRoadDisplayClasses)) ) {
            midmifData->setTempRoadDisplayClass( tmpDisplayClass );
         }
      }
      nbrExtraAttributes--;
   }

   
   /*
   // Template for next item attribute
   // Note: if the attribute (and previous attribute) is surrounded 
   // by "-chars or not affects how to read inbuffer.
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      nbrExtraAttributes--;
   }*/

   if (nbrExtraAttributes != 0) {
      mc2log << error << here << "Unused extra attributes: "
             << nbrExtraAttributes << endl;
   }

   // Skip the rest of the mid line, nothing there
   if (readRestOfLine)
      midFile.getline(inbuffer, maxLineLength);
   // else
   // Don't read the rest of the mid line, we have a map ssi coordinate 
   // to use for checking which map is the correct one
 
   return true;
}

void
GMSStreetSegmentItem::
copyAttributes(GMSStreetSegmentItem* ssi)
{
   if (m_node0 == NULL)
      m_node0 = new GMSNode();
   if (m_node1 == NULL)
      m_node1 = new GMSNode();

   m_roadClass = ssi->getRoadClass();
   m_width = ssi->getWidth();
   m_condition = ssi->getRoadCondition();
   m_ramp = ssi->isRamp();
   m_roundabout = ssi->isRoundabout();
   m_roundaboutish = ssi->isRoundaboutish();
   m_divided = ssi->isDivided();
   m_multiDigitised = ssi->isMultiDigitised();
   m_controlledAccess = ssi->isControlledAccess();

   //housenumbers?

   //node0
   getNode(0)->setSpeedLimit(ssi->getNode(0)->getSpeedLimit());
   getNode(0)->setEntryRestrictions(ssi->getNode(0)->getEntryRestrictions());
   getNode(0)->setMaximumHeight(ssi->getNode(0)->getMaximumHeight());
   getNode(0)->setMaximumWeight(ssi->getNode(0)->getMaximumWeight());
   getNode(0)->setLevel(ssi->getNode(0)->getLevel());
   getNode(0)->setMajorRoad(ssi->getNode(0)->isMajorRoad());
   getNode(0)->setRoadToll(ssi->getNode(0)->hasRoadToll());

   //node1
   getNode(1)->setSpeedLimit(ssi->getNode(1)->getSpeedLimit());
   getNode(1)->setEntryRestrictions(ssi->getNode(1)->getEntryRestrictions());
   getNode(1)->setMaximumHeight(ssi->getNode(1)->getMaximumHeight());
   getNode(1)->setMaximumWeight(ssi->getNode(1)->getMaximumWeight());
   getNode(1)->setLevel(ssi->getNode(1)->getLevel());
   getNode(1)->setMajorRoad(ssi->getNode(1)->isMajorRoad());
   getNode(1)->setRoadToll(ssi->getNode(1)->hasRoadToll());
}


